/************************************************************\
      _____ ___  _____    ______             _
     / ____|__ \|  __ \  |  ____|           (_)
    | (___    ) | |  | | | |__   _ __   __ _ _ _ __   ___
     \___ \  / /| |  | | |  __| | '_ \ / _` | | '_ \ / _ \
     ____) |/ /_| |__| | | |____| | | | (_| | | | | |  __/
    |_____/|____|_____/  |______|_| |_|\__, |_|_| |_|\___|
                                        __/ |
                                       |___/
    ======================================================
        S2D Engine - An Open-Source 2D Game Framework
                    Coded by Sevenisko

    Purpose: Graphics subsystem of S2D Engine
\************************************************************/

#include "EngineIncludes.h"
#include <thread>
#include <string>
#include <map>

#pragma region DirectX-Specific shit
typedef enum
{
    SDL_RENDERCMD_NO_OP,
    SDL_RENDERCMD_SETVIEWPORT,
    SDL_RENDERCMD_SETCLIPRECT,
    SDL_RENDERCMD_SETDRAWCOLOR,
    SDL_RENDERCMD_CLEAR,
    SDL_RENDERCMD_DRAW_POINTS,
    SDL_RENDERCMD_DRAW_LINES,
    SDL_RENDERCMD_FILL_RECTS,
    SDL_RENDERCMD_COPY,
    SDL_RENDERCMD_COPY_EX
} SDL_RenderCommandType;

typedef struct
{
    SDL_Rect viewport;
    SDL_bool viewport_dirty;
    SDL_Texture* texture;
    SDL_BlendMode blend;
    SDL_bool cliprect_enabled;
    SDL_bool cliprect_enabled_dirty;
    SDL_Rect cliprect;
    SDL_bool cliprect_dirty;
    SDL_bool is_copy_ex;
    LPDIRECT3DPIXELSHADER9 shader;
} D3DDrawStateCache;

typedef struct
{
    void* d3dDLL;
    IDirect3D9* d3d;
    IDirect3DDevice9* device;
    UINT adapter;
    D3DPRESENT_PARAMETERS pparams;
    SDL_bool updateSize;
    SDL_bool beginScene;
    SDL_bool enableSeparateAlphaBlend;
    D3DTEXTUREFILTERTYPE scaleMode[8];
    IDirect3DSurface9* defaultRenderTarget;
    IDirect3DSurface9* currentRenderTarget;
    void* d3dxDLL;
    LPDIRECT3DPIXELSHADER9 shaders[3];
    LPDIRECT3DVERTEXBUFFER9 vertexBuffers[8];
    size_t vertexBufferSize[8];
    int currentVertexBuffer;
    SDL_bool reportedVboProblem;
    D3DDrawStateCache drawstate;
} D3DRenderData;

typedef struct
{
    SDL_bool dirty;
    int w, h;
    DWORD usage;
    Uint32 format;
    D3DFORMAT d3dfmt;
    IDirect3DTexture9* texture;
    IDirect3DTexture9* staging;
} D3DTextureRep;

typedef struct
{
    D3DTextureRep texture;
    D3DTEXTUREFILTERTYPE scaleMode;

    /* YV12 texture support */
    SDL_bool yuv;
    D3DTextureRep utexture;
    D3DTextureRep vtexture;
    Uint8* pixels;
    int pitch;
    SDL_Rect locked_rect;
} D3DTextureData;

typedef struct SDL_RenderCommand
{
    SDL_RenderCommandType command;
    union {
        struct {
            size_t first;
            SDL_Rect rect;
        } viewport;
        struct {
            SDL_bool enabled;
            SDL_Rect rect;
        } cliprect;
        struct {
            size_t first;
            size_t count;
            Uint8 r, g, b, a;
            SDL_BlendMode blend;
            SDL_Texture* texture;
        } draw;
        struct {
            size_t first;
            Uint8 r, g, b, a;
        } color;
    } data;
    struct SDL_RenderCommand* next;
} SDL_RenderCommand;

struct SDL_Renderer
{
    const void* magic;

    void (*WindowEvent) (SDL_Renderer* renderer, const SDL_WindowEvent* event);
    int (*GetOutputSize) (SDL_Renderer* renderer, int* w, int* h);
    SDL_bool(*SupportsBlendMode)(SDL_Renderer* renderer, SDL_BlendMode blendMode);
    int (*CreateTexture) (SDL_Renderer* renderer, SDL_Texture* texture);
    int (*QueueSetViewport) (SDL_Renderer* renderer, SDL_RenderCommand* cmd);
    int (*QueueSetDrawColor) (SDL_Renderer* renderer, SDL_RenderCommand* cmd);
    int (*QueueDrawPoints) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, const SDL_FPoint* points,
        int count);
    int (*QueueDrawLines) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, const SDL_FPoint* points,
        int count);
    int (*QueueFillRects) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, const SDL_FRect* rects,
        int count);
    int (*QueueCopy) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, SDL_Texture* texture,
        const SDL_Rect* srcrect, const SDL_FRect* dstrect);
    int (*QueueCopyEx) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, SDL_Texture* texture,
        const SDL_Rect* srcquad, const SDL_FRect* dstrect,
        const double angle, const SDL_FPoint* center, const SDL_RendererFlip flip);
    int (*RunCommandQueue) (SDL_Renderer* renderer, SDL_RenderCommand* cmd, void* vertices, size_t vertsize);
    int (*UpdateTexture) (SDL_Renderer* renderer, SDL_Texture* texture,
        const SDL_Rect* rect, const void* pixels,
        int pitch);
    int (*UpdateTextureYUV) (SDL_Renderer* renderer, SDL_Texture* texture,
        const SDL_Rect* rect,
        const Uint8* Yplane, int Ypitch,
        const Uint8* Uplane, int Upitch,
        const Uint8* Vplane, int Vpitch);
    int (*LockTexture) (SDL_Renderer* renderer, SDL_Texture* texture,
        const SDL_Rect* rect, void** pixels, int* pitch);
    void (*UnlockTexture) (SDL_Renderer* renderer, SDL_Texture* texture);
    void (*SetTextureScaleMode) (SDL_Renderer* renderer, SDL_Texture* texture, SDL_ScaleMode scaleMode);
    int (*SetRenderTarget) (SDL_Renderer* renderer, SDL_Texture* texture);
    int (*RenderReadPixels) (SDL_Renderer* renderer, const SDL_Rect* rect,
        Uint32 format, void* pixels, int pitch);
    void (*RenderPresent) (SDL_Renderer* renderer);
    void (*DestroyTexture) (SDL_Renderer* renderer, SDL_Texture* texture);

    void (*DestroyRenderer) (SDL_Renderer* renderer);

    int (*GL_BindTexture) (SDL_Renderer* renderer, SDL_Texture* texture, float* texw, float* texh);
    int (*GL_UnbindTexture) (SDL_Renderer* renderer, SDL_Texture* texture);

    void* (*GetMetalLayer) (SDL_Renderer* renderer);
    void* (*GetMetalCommandEncoder) (SDL_Renderer* renderer);

    /* The current renderer info */
    SDL_RendererInfo info;

    /* The window associated with the renderer */
    SDL_Window* window;
    SDL_bool hidden;

    /* The logical resolution for rendering */
    int logical_w;
    int logical_h;
    int logical_w_backup;
    int logical_h_backup;

    /* Whether or not to force the viewport to even integer intervals */
    SDL_bool integer_scale;

    /* The drawable area within the window */
    SDL_Rect viewport;
    SDL_Rect viewport_backup;

    /* The clip rectangle within the window */
    SDL_Rect clip_rect;
    SDL_Rect clip_rect_backup;

    /* Wether or not the clipping rectangle is used. */
    SDL_bool clipping_enabled;
    SDL_bool clipping_enabled_backup;

    /* The render output coordinate scale */
    SDL_FPoint scale;
    SDL_FPoint scale_backup;

    /* The pixel to point coordinate scale */
    SDL_FPoint dpi_scale;

    /* The list of textures */
    SDL_Texture* textures;
    SDL_Texture* target;
    SDL_mutex* target_mutex;

    Uint8 r, g, b, a;                   /**< Color for drawing operations values */
    SDL_BlendMode blendMode;            /**< The drawing blend mode */

    SDL_bool always_batch;
    SDL_bool batching;
    SDL_RenderCommand* render_commands;
    SDL_RenderCommand* render_commands_tail;
    SDL_RenderCommand* render_commands_pool;
    Uint32 render_command_generation;
    Uint32 last_queued_color;
    SDL_Rect last_queued_viewport;
    SDL_Rect last_queued_cliprect;
    SDL_bool last_queued_cliprect_enabled;
    SDL_bool color_queued;
    SDL_bool viewport_queued;
    SDL_bool cliprect_queued;

    void* vertex_data;
    size_t vertex_data_used;
    size_t vertex_data_allocation;

    void* driverdata;
};

typedef struct SDL_SW_YUVTexture SDL_SW_YUVTexture;

struct SDL_Texture
{
    const void* magic;
    Uint32 format;              /**< The pixel format of the texture */
    int access;                 /**< SDL_TextureAccess */
    int w;                      /**< The width of the texture */
    int h;                      /**< The height of the texture */
    int modMode;                /**< The texture modulation mode */
    SDL_BlendMode blendMode;    /**< The texture blend mode */
    SDL_ScaleMode scaleMode;    /**< The texture scale mode */
    Uint8 r, g, b, a;           /**< Texture modulation values */

    SDL_Renderer* renderer;

    /* Support for formats not supported directly by the renderer */
    SDL_Texture* native;
    SDL_SW_YUVTexture* yuv;
    void* pixels;
    int pitch;
    SDL_Rect locked_rect;
    SDL_Surface* locked_surface;  /**< Locked region exposed as a SDL surface */

    Uint32 last_command_generation; /* last command queue generation this texture was in. */

    void* driverdata;           /**< Driver specific texture representation */

    SDL_Texture* prev;
    SDL_Texture* next;
};

D3DFORMAT PixelFormatToD3DFMT(Uint32 format)
{
    switch (format) {
    case SDL_PIXELFORMAT_RGB565:
        return D3DFMT_R5G6B5;
    case SDL_PIXELFORMAT_RGB888:
        return D3DFMT_X8R8G8B8;
    case SDL_PIXELFORMAT_ARGB8888:
        return D3DFMT_A8R8G8B8;
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        return D3DFMT_L8;
    default:
        return D3DFMT_UNKNOWN;
    }
}

Uint32 D3DFMTToPixelFormat(D3DFORMAT format)
{
    switch (format) {
    case D3DFMT_R5G6B5:
        return SDL_PIXELFORMAT_RGB565;
    case D3DFMT_X8R8G8B8:
        return SDL_PIXELFORMAT_RGB888;
    case D3DFMT_A8R8G8B8:
        return SDL_PIXELFORMAT_ARGB8888;
    default:
        return SDL_PIXELFORMAT_UNKNOWN;
    }
}

int CreateD3DTextureRep(IDirect3DDevice9* device, D3DTextureRep* texture, DWORD usage, Uint32 format, D3DFORMAT d3dfmt, int w, int h)
{
    HRESULT result;

    texture->dirty = SDL_FALSE;
    texture->w = w;
    texture->h = h;
    texture->usage = usage;
    texture->format = format;
    texture->d3dfmt = d3dfmt;

    result = IDirect3DDevice9_CreateTexture(device, w, h, 1, usage,
        PixelFormatToD3DFMT(format),
        D3DPOOL_DEFAULT, &texture->texture, NULL);
    if (FAILED(result)) {
        return -1;
    }
    return 0;
}

int CreateD3DStagingTexture(IDirect3DDevice9* device, D3DTextureRep* texture)
{
    HRESULT result;

    if (texture->staging == NULL) {
        result = IDirect3DDevice9_CreateTexture(device, texture->w, texture->h, 1, 0,
            texture->d3dfmt, D3DPOOL_SYSTEMMEM, &texture->staging, NULL);
        if (FAILED(result)) {
            return -1;
        }
    }
    return 0;
}

int UpdateD3DTextureRep(IDirect3DDevice9* device, D3DTextureRep* texture, int x, int y, int w, int h, const void* pixels, int pitch)
{
    RECT d3drect;
    D3DLOCKED_RECT locked;
    const Uint8* src;
    Uint8* dst;
    int row, length;
    HRESULT result;

    if (CreateD3DStagingTexture(device, texture) < 0) {
        return -1;
    }

    d3drect.left = x;
    d3drect.right = x + w;
    d3drect.top = y;
    d3drect.bottom = y + h;

    result = IDirect3DTexture9_LockRect(texture->staging, 0, &locked, &d3drect, 0);
    if (FAILED(result)) {
        return -2;
    }

    src = (const Uint8*)pixels;
    dst = (Uint8*)locked.pBits;
    length = w * SDL_BYTESPERPIXEL(texture->format);
    if (length == pitch && length == locked.Pitch) {
        SDL_memcpy(dst, src, length * h);
    }
    else {
        if (length > pitch) {
            length = pitch;
        }
        if (length > locked.Pitch) {
            length = locked.Pitch;
        }
        for (row = 0; row < h; ++row) {
            SDL_memcpy(dst, src, length);
            src += pitch;
            dst += locked.Pitch;
        }
    }
    result = IDirect3DTexture9_UnlockRect(texture->staging, 0);
    if (FAILED(result)) {
        return -3;
    }
    texture->dirty = SDL_TRUE;

    return 0;
}

void DestroyD3DTextureRep(D3DTextureRep* texture)
{
    if (texture->texture) {
        IDirect3DTexture9_Release(texture->texture);
        texture->texture = NULL;
    }
    if (texture->staging) {
        IDirect3DTexture9_Release(texture->staging);
        texture->staging = NULL;
    }
}

int CreateD3DTexture(SDL_Renderer* renderer, SDL_Texture* texture)
{
    D3DRenderData* data = (D3DRenderData*)renderer->driverdata;
    D3DTextureData* texturedata;
    DWORD usage;

    texturedata = (D3DTextureData*)SDL_calloc(1, sizeof(*texturedata));
    if (!texturedata) {
        return SDL_OutOfMemory();
    }
    texturedata->scaleMode = (texture->scaleMode == SDL_ScaleModeNearest) ? D3DTEXF_POINT : D3DTEXF_LINEAR;

    texture->driverdata = texturedata;

    if (texture->access == SDL_TEXTUREACCESS_TARGET) {
        usage = D3DUSAGE_RENDERTARGET;
    }
    else {
        usage = 0;
    }

    if (CreateD3DTextureRep(data->device, &texturedata->texture, usage, texture->format, PixelFormatToD3DFMT(texture->format), texture->w, texture->h) < 0) {
        return -1;
    }

    if (texture->format == SDL_PIXELFORMAT_YV12 ||
        texture->format == SDL_PIXELFORMAT_IYUV) {
        texturedata->yuv = SDL_TRUE;

        if (CreateD3DTextureRep(data->device, &texturedata->utexture, usage, texture->format, PixelFormatToD3DFMT(texture->format), (texture->w + 1) / 2, (texture->h + 1) / 2) < 0) {
            return -1;
        }

        if (CreateD3DTextureRep(data->device, &texturedata->vtexture, usage, texture->format, PixelFormatToD3DFMT(texture->format), (texture->w + 1) / 2, (texture->h + 1) / 2) < 0) {
            return -1;
        }
    }
    return 0;
}

int UpdateD3DTexture(SDL_Renderer* renderer, SDL_Texture* texture,
    const SDL_Rect* rect, const void* pixels, int pitch)
{
    D3DRenderData* data = (D3DRenderData*)renderer->driverdata;
    D3DTextureData* texturedata = (D3DTextureData*)texture->driverdata;

    if (!texturedata) {
        SDL_SetError("Texture is not currently available");
        return -1;
    }

    if (UpdateD3DTextureRep(data->device, &texturedata->texture, rect->x, rect->y, rect->w, rect->h, pixels, pitch) < 0) {
        return -1;
    }

    if (texturedata->yuv) {
        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + rect->h * pitch);

        if (UpdateD3DTextureRep(data->device, texture->format == SDL_PIXELFORMAT_YV12 ? &texturedata->vtexture : &texturedata->utexture, rect->x / 2, rect->y / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, pixels, (pitch + 1) / 2) < 0) {
            return -1;
        }

        /* Skip to the correct offset into the next texture */
        pixels = (const void*)((const Uint8*)pixels + ((rect->h + 1) / 2) * ((pitch + 1) / 2));
        if (UpdateD3DTextureRep(data->device, texture->format == SDL_PIXELFORMAT_YV12 ? &texturedata->utexture : &texturedata->vtexture, rect->x / 2, (rect->y + 1) / 2, (rect->w + 1) / 2, (rect->h + 1) / 2, pixels, (pitch + 1) / 2) < 0) {
            return -1;
        }
    }
    return 0;
}

void DestroyD3DTexture(SDL_Renderer* renderer, SDL_Texture* texture)
{
    D3DRenderData* renderdata = (D3DRenderData*)renderer->driverdata;
    D3DTextureData* data = (D3DTextureData*)texture->driverdata;

    if (renderdata->drawstate.texture == texture) {
        renderdata->drawstate.texture = NULL;
        renderdata->drawstate.shader = NULL;
        IDirect3DDevice9_SetPixelShader(renderdata->device, NULL);
        IDirect3DDevice9_SetTexture(renderdata->device, 0, NULL);
        if (data->yuv) {
            IDirect3DDevice9_SetTexture(renderdata->device, 1, NULL);
            IDirect3DDevice9_SetTexture(renderdata->device, 2, NULL);
        }
    }

    if (!data) {
        return;
    }

    DestroyD3DTextureRep(&data->texture);
    DestroyD3DTextureRep(&data->utexture);
    DestroyD3DTextureRep(&data->vtexture);
    SDL_free(data->pixels);
    SDL_free(data);
    texture->driverdata = NULL;
}
#pragma endregion

S2DTexture* rtex;

std::vector<S2DFont*> LoadedFonts;

std::vector<ScreenResolution> GetResolutions()
{
    std::vector<ScreenResolution> resolutions;

    DEVMODE dm = { 0 };
    dm.dmSize = sizeof(dm);
    for (int iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; iModeNum++) {
        if (dm.dmDisplayFrequency != 60) continue;

        resolutions.push_back({ (int)dm.dmPelsWidth, (int)dm.dmPelsHeight });
    }

    return resolutions;
}

ScreenResolution* GetResolution(int width, int height)
{
    for (auto res : GetResolutions())
    {
        if (res.width == width && res.height == height) return &res;
        else return nullptr;
    }
}

S2DFont::S2DFont(SDL_Renderer* renderer, const char* path)
{
    myRenderer = renderer;
    ttfFile = std::string(path);

    TTF_Font* fnt = TTF_OpenFont(path, 36);

    if (!fnt)
        S2DFatalErrorFormatted("%s", TTF_GetError());

    LoadedFonts.push_back(this);

    TTF_CloseFont(fnt);
}

Vec2Int S2DFont::GetSize(int size, const char* text)
{
    TTF_Font* fnt = TTF_OpenFont(ttfFile.c_str(), size);
    SDL_Surface* sur = TTF_RenderText_Blended(fnt, text, { 255, 255, 255, 255 });
    SDL_Texture* tex = SDL_CreateTextureFromSurface(myRenderer, sur);

    Vec2Int s;

    SDL_QueryTexture(tex, NULL, NULL, &s.x, &s.y);

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(sur);
    TTF_CloseFont(fnt);

    return s;
}

void S2DFont::Render(int size, const char* text, Vec2 pos, Vec2 center, float angle, TexFlipMode flip, Color color)
{
    TTF_Font* fnt = TTF_OpenFont(ttfFile.c_str(), size);
    SDL_Surface* sur = TTF_RenderText_Blended(fnt, text, { color.r, color.g, color.b, color.a });
    SDL_Texture* tex = SDL_CreateTextureFromSurface(myRenderer, sur);

    Vec2Int isize = GetSize(size, text);

    Vec2 vsize = Vec2(isize.x, isize.y);

    SDL_FRect rect = { pos.x - (vsize.x * center.x), pos.y - (vsize.y * center.y), vsize.x, vsize.y };
    SDL_SetTextureColorMod(tex, color.r, color.g, color.b);

    SDL_RenderCopyExF(myRenderer, tex, NULL, &rect, angle, NULL, (SDL_RendererFlip)flip);

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(sur);
    TTF_CloseFont(fnt);
}

S2DTexture* S2DFont::RenderToTexture(int size, const char* text, Color color)
{
    TTF_Font* fnt = TTF_OpenFont(ttfFile.c_str(), size);
    SDL_Surface* sur = TTF_RenderText_Blended(fnt, text, { color.r, color.g, color.b, color.a });
    SDL_Texture* tex = SDL_CreateTextureFromSurface(myRenderer, sur);

    SDL_FreeSurface(sur);
    TTF_CloseFont(fnt);

    return new S2DTexture(tex, "");
}

void S2DFont::UpdateRenderer(SDL_Renderer* renderer)
{
    myRenderer = renderer;
}

S2DSprite::S2DSprite(S2DTexture* tex, Vec2Int sprSize, int frameRate, int numFrames)
{
    this->texture = tex;

    for (int i = 1; i <= numFrames; i++)
    {
        frames.push_back({numFrames * i, 0, sprSize.x, sprSize.y});
        this->framerate = frameRate;
        this->curFrame = 0;
    }
}

S2DSprite::S2DSprite(S2DTexture* tex, std::vector<Vec2Int> sprSizes, int frameRate)
{
    int curX = 0;

    this->texture = tex;

    for (int i = 0; i < sprSizes.size(); i++)
    {
        frames.push_back({ curX, 0, sprSizes[i].x, sprSizes[i].y });
        this->framerate = frameRate;
        this->curFrame = 0;
        curX += sprSizes[i].x;
    }
}

S2DSprite::~S2DSprite()
{
    /*std::vector<S2DSprite*>::iterator position = std::find(LoadedSprites.begin(), LoadedSprites.end(), this);
    if (position != LoadedSprites.end())
        LoadedSprites.erase(position);*/
}

void S2DSprite::NextFrame()
{
    if (curFrame >= frames.size() - 1)
        curFrame = 0;
    else
        curFrame++;
}

void S2DSprite::SetFrame(int frame)
{
    if (frame > frames.size() - 1 && frame < 0)
        curFrame = 0;
    else
        curFrame = frame;
}

void S2DSprite::PrevFrame()
{
    if (curFrame <= 0)
        curFrame = frames.size() - 1;
    else
        curFrame--;
}

IDirect3DDevice9* S2DGraphics::GetDX9Device()
{
    auto dev = SDL_RenderGetD3D9Device(NativeRenderer);

    S2DAssert(dev != NULL);

    return dev;
}

Vec2Int S2DGraphics::GetCurrentWindowSize()
{
    if (Fullscreen)
    {
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(GetCurrentScreen(), &DM);
        auto Width = DM.w;
        auto Height = DM.h;

        return Vec2Int(Width, Height);
    }
    else
    {
        int Width = 0, Height = 0;
        SDL_GetWindowSize(EngineWindow, &Width, &Height);
        return Vec2Int(Width, Height);
    }
}

void S2DGraphics::SetResolution(ScreenResolution* res)
{
    SDL_SetWindowSize(EngineWindow, res->width, res->height);

    CurrentResolution = *res;
}

void S2DGraphics::SetResolution(int w, int h)
{
    SDL_SetWindowSize(EngineWindow, w, h);

    CurrentResolution = { w, h };
}

void S2DGraphics::SetScreen(int screen)
{
    SDL_SetWindowPosition(EngineWindow, SDL_WINDOWPOS_CENTERED_DISPLAY(screen), SDL_WINDOWPOS_CENTERED_DISPLAY(screen));
    CurrentScreen = screen;
}

void S2DGraphics::SetFullscreen(bool state)
{
    Fullscreen = state;

    Running = false;

    std::map<int, const char*> reloadTexs = GetReloadTextures();
    //ClearTextures();
    SDL_DestroyRenderer(NativeRenderer);

    int width, height;

    SDL_GetWindowSize(EngineWindow, &width, &height);
    SDL_Surface* surface = SDL_GetWindowSurface(EngineWindow);
    Uint32 skyblue = SDL_MapRGB(surface->format, 0, 0, 0);
    SDL_FillRect(surface, NULL, skyblue);
    SDL_UpdateWindowSurface(EngineWindow);
    SDL_SetWindowFullscreen(EngineWindow, state ? SDL_WINDOW_FULLSCREEN_DESKTOP : NULL);
    NativeRenderer = SDL_CreateRenderer(EngineWindow, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    for (auto font : LoadedFonts)
    {
        font->UpdateRenderer(NativeRenderer);
    }

    for (auto t : reloadTexs)
    {
        LoadTexture(t.second);
    }

    OnRenderReload();

    Running = true;
}

S2DTexture* S2DGraphics::GetTextureByID(int texID)
{
    return LoadedTextures[texID];
}

void S2DGraphics::RenderSprite(S2DCamera* cam, S2DSprite* sprite, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color)
{
    if (sprite->GetTexture()->GetPath() == NULL) return;

    SDL_Rect crop = sprite->GetFrames()[sprite->GetCurFrame()];

    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (cam->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (cam->Position.y / 16.0f) * scrSize.y;

    SDL_FRect rect = { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), size.x, size.y };

    Vec2 dist = Vec2::DistanceVec(pixPos, camPixPos);

    if (dist.x > ((scrSize.x / 2) + size.x) || dist.y > ((scrSize.y / 2) + size.y)) return;

    SDL_SetTextureColorMod(sprite->GetTexture()->GetSDLTexture(), color.r, color.g, color.b);

    SDL_RenderCopyExF(NativeRenderer, sprite->GetTexture()->GetSDLTexture(), &crop, &rect, angle, NULL, (SDL_RendererFlip)flip);
}

void S2DGraphics::RenderFilledBox(S2DCamera* cam, Vec2 pos, Vec2 center, Vec2 size, Color color)
{
    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (cam->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (cam->Position.y / 16.0f) * scrSize.y;

    SDL_FRect rect = { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), size.x, size.y };

    Vec2 dist = Vec2::DistanceVec(pixPos, camPixPos);

    if (dist.x > ((scrSize.x / 2) + size.x) || dist.y > ((scrSize.y / 2) + size.y)) return;

    SDL_SetRenderDrawColor(NativeRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRectF(NativeRenderer, &rect);
}

void S2DGraphics::RenderBox(S2DCamera* cam, Vec2 pos, Vec2 center, Vec2 size, Color color)
{
    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (cam->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (cam->Position.y / 16.0f) * scrSize.y;

    SDL_FRect rect = { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), size.x, size.y };

    Vec2 dist = Vec2::DistanceVec(pixPos, camPixPos);

    if (dist.x > ((scrSize.x / 2) + size.x) || dist.y > ((scrSize.y / 2) + size.y)) return;
    
    SDL_SetRenderDrawColor(NativeRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRectF(NativeRenderer, &rect);
}

void S2DGraphics::RenderLine(S2DCamera* cam, Vec2 p1, Vec2 p2, Color color)
{
    Vec2 pixPos1 = Vec2();
    Vec2 pixPos2 = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos1.x = (p1.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos1.y = (p2.y / 16.0f) * scrSize.y + (scrSize.x / 2);
    pixPos2.x = (p1.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos2.y = (p2.y / 16.0f) * scrSize.y + (scrSize.x / 2);

    SDL_SetRenderDrawColor(NativeRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLineF(NativeRenderer, p1.x, p1.y, p2.x, p2.y);
}

void S2DGraphics::RenderPoint(S2DCamera* cam, Vec2 position, Color color)
{
    SDL_SetRenderDrawColor(NativeRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPointF(NativeRenderer, position.x, position.y);
}

S2DTexture* S2DGraphics::LoadTextureRaw(const char* fileName)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto tex = IMG_LoadTexture(NativeRenderer, fileName);
    if (!tex) return nullptr;

    S2DTexture* t = new S2DTexture(tex, fileName);

    t->textureID = -1;

    return t;
}

int S2DGraphics::LoadTexture(const char* fileName)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto tex = IMG_LoadTexture(NativeRenderer, fileName);
    if (!tex) return -1;

    S2DTexture* t = new S2DTexture(tex, fileName);

    size_t index = LoadedTextures.size();

    t->textureID = index;

    LoadedTextures.push_back(t);

    size_t size = LoadedTextures.size();

    return index;
}

bool S2DGraphics::UnloadTexture(S2DTexture* texture)
{
    auto tex = texture->GetSDLTexture();
    if (tex)
    {
        SDL_DestroyTexture(tex);
       
        int index = 0;
        std::vector<S2DTexture*>::iterator it = std::find(LoadedTextures.begin(), LoadedTextures.end(), texture);

        delete texture;

        if (it != LoadedTextures.end())
            index = std::distance(LoadedTextures.begin(), it);

        LoadedTextures.erase(LoadedTextures.begin() + index);
    }

    return false;
}

bool S2DGraphics::UnloadTexture(int textureID)
{
    auto tex = LoadedTextures[textureID]->GetSDLTexture();
    if (tex)
    {
        SDL_DestroyTexture(LoadedTextures[textureID]->GetSDLTexture());
        delete LoadedTextures[textureID];
        LoadedTextures.erase(LoadedTextures.begin() + textureID);
    }

    return false;
}

std::map<int, const char*> S2DGraphics::GetReloadTextures()
{
    std::map<int, const char*> reloadTexs;

    for (auto tex : LoadedTextures)
    {
        OutputDebugStringA(("Reloading texture: " + std::string(tex->GetPath()) + "\n").c_str());

        reloadTexs.insert(std::pair<int, const char*>(tex->textureID, tex->GetPath()));
        SDL_DestroyTexture(tex->GetSDLTexture());
        delete tex;
        std::vector<S2DTexture*>::iterator position = std::find(LoadedTextures.begin(), LoadedTextures.end(), tex);
        if (position != LoadedTextures.end())
            LoadedTextures.erase(position);
    }

    return reloadTexs;
}

void S2DGraphics::ClearTextures()
{
    for (auto tex : LoadedTextures)
    {
        SDL_DestroyTexture(tex->GetSDLTexture());
        delete tex;
        std::vector<S2DTexture*>::iterator position = std::find(LoadedTextures.begin(), LoadedTextures.end(), tex);
        if (position != LoadedTextures.end())
            LoadedTextures.erase(position);
    }
}

#define texture_height 1080.0f
#define texture_width 1920.0f

void S2DGraphics::RenderFullscreenD3DTexture(IDirect3DTexture9* tex, Color color)
{
    LPD3DXSPRITE sprite;

    D3DXCreateSprite(GetDX9Device(), &sprite);

    sprite->Begin(NULL);

    float screen_x = (float)GetCurrentResolution()->width;
    float screen_y = (float)GetCurrentResolution()->height;
    float scale_x = screen_y / texture_height;

    D3DXMATRIX loading_screen_transform;
    D3DXVECTOR2 scale = { scale_x, scale_x };
    D3DXVECTOR2 position = { ((float)GetCurrentResolution()->width / 2.0f) - ((texture_width / 2.0f) * scale_x), 0.0f };

    D3DXMatrixTransformation2D(&loading_screen_transform, NULL, 0.0, &scale, NULL, NULL, &position);
    sprite->SetTransform(&loading_screen_transform);
    sprite->Draw(tex, NULL, NULL, NULL, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));
    sprite->End();
}

void S2DGraphics::LoadAndRenderFullscreenD3DTexture(const char* path, Color color)
{
    LPD3DXSPRITE sprite;

    IDirect3DTexture9* tex;
    D3DXCreateTextureFromFileA(GetDX9Device(), path, &tex);

    D3DXCreateSprite(GetDX9Device(), &sprite);

    sprite->Begin(NULL);

    float screen_x = (float)GetCurrentResolution()->width;
    float screen_y = (float)GetCurrentResolution()->height;
    float scale_x = screen_y;

    D3DXMATRIX loading_screen_transform;
    D3DXVECTOR2 scale = { scale_x, scale_x };
    D3DXVECTOR2 position = { ((float)GetCurrentResolution()->width / 2.0f) - scale_x, 0.0f };

    D3DXMatrixTransformation2D(&loading_screen_transform, NULL, 0.0, &scale, NULL, NULL, &position);
    sprite->SetTransform(&loading_screen_transform);
    sprite->Draw(tex, NULL, NULL, NULL, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));
    sprite->End();
}

void S2DGraphics::RenderTexture(S2DCamera* cam, int textureID, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color)
{
    auto tex = LoadedTextures[textureID];
    if (!tex->GetSDLTexture()) return;

    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (cam->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (cam->Position.y / 16.0f) * scrSize.y;

    SDL_FRect rect = { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), size.x, size.y };

    Vec2 dist = Vec2::DistanceVec(pixPos, camPixPos);

    if (dist.x > ((scrSize.x / 2) + size.x) || dist.y > ((scrSize.y / 2) + size.y)) return;

    SDL_SetTextureColorMod(tex->GetSDLTexture(), color.r, color.g, color.b);

    SDL_RenderCopyExF(NativeRenderer, tex->GetSDLTexture(), NULL, &rect, angle, NULL, (SDL_RendererFlip)flip);
}

void S2DGraphics::RenderTexture(S2DCamera* cam, S2DTexture* tex, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color)
{
    if (!tex->GetSDLTexture()) return;

    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (cam->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (cam->Position.y / 16.0f) * scrSize.y;

    SDL_FRect rect = { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), size.x, size.y };

    Vec2 dist = Vec2::DistanceVec(pixPos, camPixPos);

    if (dist.x > ((scrSize.x / 2) + size.x) || dist.y > ((scrSize.y / 2) + size.y)) return;

    SDL_SetTextureColorMod(tex->GetSDLTexture(), color.r, color.g, color.b);

    SDL_RenderCopyExF(NativeRenderer, tex->GetSDLTexture(), NULL, &rect, angle, NULL, (SDL_RendererFlip)flip);
}

void S2DGraphics::BeginFrame(bool toTexture)
{
    if (toTexture)
    {
        ScreenResolution* res = GetCurrentResolution();

        rtex = new S2DTexture(SDL_CreateTexture(GetRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res->width, res->height), "rendertex");

        SDL_SetRenderTarget(GetRenderer(), rtex->GetSDLTexture());
    }

    SDL_SetRenderDrawColor(NativeRenderer, 0, 0, 0, 255);
    SDL_RenderClear(NativeRenderer);
}

IDirect3DTexture9* S2DGraphics::GetD3DTexture(S2DTexture* tex)
{
    auto data = (D3DTextureData*)tex->GetSDLTexture()->driverdata;

    return data->texture.texture;
}

void S2DGraphics::EndFrame()
{
    if (rtex)
    {
        SDL_SetRenderTarget(GetRenderer(), NULL);

        delete rtex;
    }
    else
    {
        SDL_RenderPresent(NativeRenderer);
    }
}

S2DGraphics::S2DGraphics(EngineInitSettings* settings)
{
    auto flags = 0;

    if (settings->fullscreen)
    {
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        Fullscreen = true;
    }
    else
    {
        Fullscreen = false;
    }

    flags |= SDL_WINDOW_OPENGL;

    CurrentResolution = *settings->resolution;
    CurrentScreen = settings->screenNum;

    // Create a window
    EngineWindow = SDL_CreateWindow(settings->title, SDL_WINDOWPOS_CENTERED_DISPLAY(settings->screenNum), SDL_WINDOWPOS_CENTERED_DISPLAY(settings->screenNum), settings->resolution->width, settings->resolution->height, flags);

    if (!EngineWindow) // If the window wasn't created
        S2DFatalErrorFormatted("Cannot create window!\n%s", SDL_GetError());

    // Create an SDL Renderer using DX9
    NativeRenderer = SDL_CreateRenderer(EngineWindow, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
    
    if (!NativeRenderer) // If the Renderer creation failed
        S2DFatalErrorFormatted("Cannot create renderer!\n%s", SDL_GetError());

    Running = true;
}

S2DGraphics::~S2DGraphics()
{
    // Free up the resources - Destroy the renderer and window
    SDL_DestroyRenderer(NativeRenderer);
    SDL_DestroyWindow(EngineWindow);
}