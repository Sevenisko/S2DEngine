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
#include <Shlwapi.h>
#include <direct.h>
#include <time.h>
#include <d3d9.h>
#include <d3dx9.h>

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

SDL_Renderer* render;

std::vector<S2DFont*> LoadedFonts;

std::vector<ParticleEmittor*> Emittors;

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

    return new S2DTexture(tex, "", SDL_PIXELFORMAT_ARGB8888);
}

void S2DFont::UpdateRenderer(SDL_Renderer* renderer)
{
    myRenderer = renderer;
}

IDirect3DTexture9* S2DTexture::ToDX9Texture()
{
    IDirect3DTexture9* dtex;

    D3DLOCKED_RECT locked;

    SDL_RenderGetD3D9Device(render)->CreateTexture(width, height, 1, 0, PixelFormatToD3DFMT(format), D3DPOOL_MANAGED, &dtex, NULL);

    dtex->LockRect(0, &locked, NULL, NULL);

    void* pixels;
    int pitch;

    SDL_LockTexture(GetSDLTexture(), NULL, &pixels, &pitch);

    memcpy(locked.pBits, pixels, pitch * height);

    SDL_UnlockTexture(GetSDLTexture());

    dtex->UnlockRect(0);

    return dtex;
}

S2DSprite::S2DSprite(S2DTexture* tex, Vec2Int sprSize, int frameRate, int numFrames)
{
    this->texture = tex;

    for (int i = 1; i <= numFrames; i++)
    {
        frames.push_back({ numFrames * i, 0, sprSize.x, sprSize.y });
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

    render = NativeRenderer;

    for (auto emittor : Emittors)
    {
        emittor->setRenderer(NativeRenderer);
        emittor->setTexture(emittor->getDefaultTexture());
    }

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

void S2DGraphics::RenderSprite(S2DSprite* sprite, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color)
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

void S2DGraphics::RenderFilledBox(Vec2 pos, Vec2 center, Vec2 size, Color color)
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

void S2DGraphics::RenderBox(Vec2 pos, Vec2 center, Vec2 size, Color color)
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

void S2DGraphics::RenderLine(Vec2 p1, Vec2 p2, Color color)
{
    Vec2 pixPos1 = Vec2();
    Vec2 pixPos2 = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos1.x = (p1.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos1.y = (p2.y / 16.0f) * scrSize.y + (scrSize.x / 2);
    pixPos2.x = (p1.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos2.y = (p2.y / 16.0f) * scrSize.y + (scrSize.x / 2);

    camPixPos.x = (cam->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (cam->Position.y / 16.0f) * scrSize.y;

    Vec2 p1f = Vec2(pixPos1.x - camPixPos.x + (scrSize.x / 2), pixPos1.y + camPixPos.y + (scrSize.y / 2));
    Vec2 p2f = Vec2(pixPos2.x - camPixPos.x + (scrSize.x / 2), pixPos2.y + camPixPos.y + (scrSize.y / 2));

    SDL_SetRenderDrawColor(NativeRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLineF(NativeRenderer, p1f.x, p1f.y, p2f.x, p2f.y);
}

void S2DGraphics::RenderPoint(Vec2 position, Color color)
{
    SDL_SetRenderDrawColor(NativeRenderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPointF(NativeRenderer, position.x, position.y);
}
 
int S2DGraphics::CreateParticleEmittor(Vec2 pos, ParticleStyle style)
{
    auto emittor = new ParticleEmittor();

    emittor->Position = pos;

    emittor->setRenderer(NativeRenderer);
    emittor->setStyle(style);
    emittor->setStartSpin(0);
    emittor->setStartSpinVar(90);
    emittor->setEndSpin(90);
    emittor->setStartSpinVar(90);

    size_t index = Emittors.size();

    Emittors.push_back(emittor);

    return index;
}

bool S2DGraphics::ResetParticleEmittor(int emitID)
{
    auto emittor = Emittors[emitID];
    if (!emittor) return false;

    emittor->resetSystem();

    return true;
}

bool S2DGraphics::RenderParticleEmittor(int emitID, Vec2 pos, float angle)
{
    auto emittor = Emittors[emitID];
    if (!emittor) return false;

    Vec2 pixPos;

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos.y = (pos.y / 16.0f) * scrSize.y + (scrSize.y / 2);

    emittor->setPosition(pixPos.x, pixPos.y);

    emittor->setAngle(angle);
    emittor->setAngleVar(angle);

    emittor->draw();

    return true;
}

bool S2DGraphics::SetParticleEmittorStyle(int emitID, ParticleStyle style)
{
    auto emittor = Emittors[emitID];
    if (!emittor) return false;

    emittor->setStyle(style);

    return true;
}

void S2DGraphics::DestroyParticleEmittor(int emitID)
{
    delete Emittors[emitID];
    Emittors.erase(Emittors.begin() + emitID);
}

S2DTexture* S2DGraphics::LoadTextureRaw(const char* fileName)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto sur = IMG_Load(fileName);
    if (!sur) return NULL;

    auto tex = SDL_CreateTexture(NativeRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, sur->w, sur->h);

    void* pixels;
    int pitch;

    SDL_LockTexture(tex, NULL, &pixels, &pitch);

    SDL_Surface* sur2 = SDL_ConvertSurfaceFormat(sur, SDL_PIXELFORMAT_RGBA8888, 0);

    memcpy(pixels, sur2->pixels, sur2->pitch * sur2->h);

    SDL_UnlockTexture(tex);

    SDL_FreeSurface(sur2);
    SDL_FreeSurface(sur);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    S2DTexture* t = new S2DTexture(tex, fileName, SDL_PIXELFORMAT_RGBA8888);

    t->textureID = -1;

    return t;
}

int S2DGraphics::LoadTexture(const char* fileName)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto sur = IMG_Load(fileName);
    if (!sur) return -1;

    auto tex = SDL_CreateTexture(NativeRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, sur->w, sur->h);

    void* pixels;
    int pitch;

    SDL_LockTexture(tex, NULL, &pixels, &pitch);

    SDL_Surface* sur2 = SDL_ConvertSurfaceFormat(sur, SDL_PIXELFORMAT_RGBA8888, 0);

    memcpy(pixels, sur2->pixels, sur2->pitch * sur2->h);

    SDL_UnlockTexture(tex);

    SDL_FreeSurface(sur2);
    SDL_FreeSurface(sur);

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    S2DTexture* t = new S2DTexture(tex, fileName, SDL_PIXELFORMAT_RGBA8888);

    size_t index = LoadedTextures.size();

    t->textureID = index;

    LoadedTextures.push_back(t);

    size_t size = LoadedTextures.size();

    return index;
}

S2DRenderTexture* S2DGraphics::CreateRenderTexture(int width, int height)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    auto tex = SDL_CreateTexture(NativeRenderer, SDL_GetWindowPixelFormat(EngineWindow), SDL_TEXTUREACCESS_TARGET, width, height);

    S2DRenderTexture* t = new S2DRenderTexture(tex, SDL_GetWindowPixelFormat(EngineWindow));

    return t;
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

        return true;
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

        return true;
    }

    return false;
}

bool S2DGraphics::DestroyRenderTexture(S2DRenderTexture* tex)
{
    if (curRenderTarget == tex) return false;

    if (tex)
    {
        SDL_DestroyTexture(tex->GetSDLTexture());
        delete tex;
        return true;
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

void S2DGraphics::RenderTexture(int textureID, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color)
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

void S2DGraphics::RenderTexture(S2DTexture* tex, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color)
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

void S2DGraphics::RenderTextureOnScreen(S2DTexture* tex, Vec2Int pos, Vec2Int size, float angle, TexFlipMode flip, Color color)
{
    SDL_Rect rect = { pos.x, pos.y, size.x, size.y };

    SDL_SetTextureColorMod(tex->GetSDLTexture(), color.r, color.g, color.b);

    SDL_RenderCopyEx(NativeRenderer, tex->GetSDLTexture(), NULL, &rect, angle, NULL, (SDL_RendererFlip)flip);
}

void S2DGraphics::SetRenderTexture(S2DRenderTexture* tex)
{
    if (!tex)
    {
        SDL_SetRenderTarget(GetRenderer(), NULL);
        curRenderTarget = NULL;
    }
    else
    {
        SDL_SetRenderTarget(GetRenderer(), tex->GetSDLTexture());
        curRenderTarget = tex;
    }
}

bool S2DGraphics::Screenshot(const char* file)
{
    // Used temporary variables
    SDL_Rect _viewport;
    SDL_Surface* _surface = NULL;

    SDL_RenderGetViewport(NativeRenderer, &_viewport);

    _surface = SDL_CreateRGBSurface(0, _viewport.w, _viewport.h, 32, 0, 0, 0, 0);

    if (!_surface) {
        S2DFatalErrorFormatted("Cannot create SDL_Surface for screenshot\n\t%s", SDL_GetError());
        return false;
    }

    if (SDL_RenderReadPixels(NativeRenderer, NULL, _surface->format->format, _surface->pixels, _surface->pitch) != 0) {
        S2DFatalErrorFormatted("Cannot read data from renderer\n\t%s", SDL_GetError());

        // Don't forget to free memory
        SDL_FreeSurface(_surface);
        return false;
    }

    if (IMG_SavePNG(_surface, file) != 0) {
        S2DFatalErrorFormatted("Cannot save screenshot\n\t%s", SDL_GetError());

        SDL_FreeSurface(_surface);
        return false;
    }

    SDL_FreeSurface(_surface);
    return true;
}

void S2DGraphics::BeginFrame()
{
    SDL_SetRenderDrawColor(NativeRenderer, 0, 0, 0, 255);
    SDL_RenderClear(NativeRenderer);
}

void S2DGraphics::EndFrame()
{
    SDL_RenderPresent(NativeRenderer);
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

    render = NativeRenderer;

    Running = true;
}

S2DGraphics::~S2DGraphics()
{
    // Free up the resources - Destroy the renderer and window
    SDL_DestroyRenderer(NativeRenderer);
    SDL_DestroyWindow(EngineWindow);
}