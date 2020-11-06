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

ID3DXSprite* sprite;

IDirect3DDevice9** dev;

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

#pragma region S2DShader
S2DShader::S2DShader(const char* path)
{
    ID3DXBuffer* listing_p = NULL;
    ID3DXBuffer* listing_v = NULL;
    ID3DXBuffer* code_p = NULL;
    ID3DXBuffer* code_v = NULL;

    auto vert_ret = D3DXCompileShaderFromFileA(path, NULL, NULL, "MainVertex", "vs_3_0", 0, &code_v, &listing_v, VertexConstantTable);
    auto pix_ret = D3DXCompileShaderFromFileA(path, NULL, NULL, "MainPixel", "ps_3_0", 0, &code_p, &listing_p, PixelConstantTable);
    
    if (FAILED(vert_ret) || FAILED(pix_ret) || listing_v || listing_p)
    {
        OutputDebugStringA("HLSL Error:\n");
        if (listing_p)
        {
            OutputDebugStringA(("\tPixel:\n" + std::string((char*)listing_p->GetBufferPointer())).c_str());
        }
        if (listing_v)
        {
            OutputDebugStringA(("\tVertex:\n" + std::string((char*)listing_v->GetBufferPointer())).c_str());
        }
    }
    else
    {
        (*dev)->CreateVertexShader((const DWORD*)code_p->GetBufferPointer(), &VertexShader);
        (*dev)->CreatePixelShader((const DWORD*)code_v->GetBufferPointer(), &PixelShader);

        code_p->Release();
        code_v->Release();
    }

    listing_p->Release();
    listing_v->Release();
}

S2DShader::~S2DShader()
{
    ((ID3DXConstantTable*)VertexConstantTable)->Release();
    ((ID3DXConstantTable*)PixelConstantTable)->Release();
    PixelShader->Release();
    VertexShader->Release();
}
#pragma endregion

#pragma region S2DFont
S2DFont::S2DFont(const char* name, int size, bool bold)
{
    ID3DXFont* fnt;

    if (D3DXCreateFontA(*dev, size, 0, (bold ? FW_BOLD : FW_NORMAL), 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, (DEFAULT_PITCH | FF_DONTCARE), name, &fnt) != S_OK)
    {
        S2DFatalError("Failed to create font");
    }
    
    this->font = fnt;
}

S2DFont::~S2DFont()
{
    this->font->Release();
    this->font = nullptr;
}

Vec2 S2DFont::GetSize(const char* text)
{
    if (strlen(text) < 1 || this->font == nullptr) return Vec2();

    HDC fontDC = this->font->GetDC();

    SIZE size;
    GetTextExtentPoint32A(fontDC, text, strlen(text), &size);

    return Vec2(size.cx, size.cy);
}

void S2DFont::Render(float scale, const char* text, Vec2 pos, Vec2 center, float angle, S2DColor color)
{
    if (strlen(text) < 1 || sprite == nullptr || this->font == nullptr) return;

    D3DXVECTOR2 sc(scale, scale);
    D3DXVECTOR2 cent(center.x, center.y);
    D3DXMATRIX matrix;

    D3DXMatrixTransformation2D(&matrix, NULL, 0, &sc, &cent, angle, NULL);

    sprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
    sprite->SetTransform(&matrix);

    RECT rect;
    SetRect(&rect, pos.x * (1.0f / scale), pos.y * (1.0f / scale), 0, 0);
    
    this->font->DrawTextA(sprite, text, -1, &rect, DT_NOCLIP, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));

    sprite->End();
}
#pragma endregion

#pragma region S2DSprite
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
#pragma endregion

#pragma region S2DGraphics
struct Vertex2D {
    float x, y, z, rhw;
    DWORD color;
};

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

    EngineWindow = SDL_CreateWindow(settings->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, settings->resolution->width, settings->resolution->height, flags);

    if (!EngineWindow)
        S2DFatalErrorFormatted("Cannot create window!\n%s", SDL_GetError());

    Interface = Direct3DCreate9(D3D_SDK_VERSION);

    if (!Interface)
    {
        S2DFatalError("Cannot create Direct3D interface!");
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(EngineWindow, &info);

    SDL_DisplayMode mode;
    
    SDL_GetWindowDisplayMode(EngineWindow, &mode);

    ZeroMemory(&Parameters, sizeof(Parameters)); 
    Parameters.Windowed = !settings->fullscreen; 
    Parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    Parameters.BackBufferCount = 1;
    Parameters.hDeviceWindow = info.info.win.window;
    Parameters.BackBufferWidth = settings->resolution->width; 
    Parameters.BackBufferHeight = settings->resolution->height;
    if (settings->fullscreen)
    {
        Parameters.BackBufferFormat = PixelFormatToD3DFMT(mode.format);
        Parameters.FullScreen_RefreshRateInHz = mode.refresh_rate;
    }
    else
    {
        Parameters.BackBufferFormat = D3DFMT_UNKNOWN;
        Parameters.FullScreen_RefreshRateInHz = 0;
    }

    if (settings->vsync)
    {
        Parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    }
    else
    {
        Parameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    }

    Parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
    Parameters.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;

    if (FAILED(Interface->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, info.info.win.window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &Parameters, &Device)))
    {
        S2DFatalError("Cannot create Direct3D device!");
    }
    else
    {
        dev = &Device;
    }

    Device->CreateRenderTarget(Parameters.BackBufferWidth, Parameters.BackBufferHeight, Parameters.BackBufferFormat, D3DMULTISAMPLE_NONE, 0, FALSE, &CustomRenderTarget, NULL);
    Device->GetRenderTarget(0, &DefaultRenderTarget);

    Running = true;
}

S2DGraphics::~S2DGraphics()
{
    CustomRenderTarget->Release();
    Device->Release();
    Interface->Release();
}

void S2DGraphics::SetCamera(S2DCamera* cam)
{
    currentCamera = cam;
}

void S2DGraphics::Screenshot(const char* name)
{
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d-%m-%Y_%H-%M-%S", timeinfo);

    char* time(buffer);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(EngineWindow, &info);

    IDirect3DSurface9* surface;
    D3DDISPLAYMODE mode;
    Device->GetDisplayMode(0, &mode);

    Device->CreateOffscreenPlainSurface(mode.Width, mode.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, NULL);
    if(Fullscreen)
    {
        if (Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) == D3D_OK)
        {
            D3DXSaveSurfaceToFileA((strlen(name) == 0 ? "" : (std::string(name) + "_") + std::string(time) + ".png").c_str(), D3DXIFF_PNG, surface, NULL, NULL);
        }
    }
    else
    {
        if (Device->GetFrontBufferData(0, surface) == D3D_OK)
        {
            RECT r;
            GetClientRect(info.info.win.window, &r); // hWnd is our window handle
            POINT p = { 0, 0 };
            ClientToScreen(info.info.win.window, &p);
            SetRect(&r, p.x, p.y, p.x + r.right, p.y + r.bottom);
            D3DXSaveSurfaceToFileA((strlen(name) == 0 ? "" : (std::string(name) + "_") + std::string(time) + ".png").c_str(), D3DXIFF_PNG, surface, NULL, &r);                
        }
    }
    
    surface->Release();
}

S2DTexture* S2DGraphics::GetCurrentFrame()
{
    IDirect3DTexture9* texture;
    IDirect3DSurface9* surface;

    D3DXCreateTexture(Device, Parameters.BackBufferWidth, Parameters.BackBufferHeight, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture);

    texture->GetSurfaceLevel(0, &surface);
    Device->StretchRect(CustomRenderTarget, NULL, surface, NULL, D3DTEXF_NONE);

    return new S2DTexture(texture, "curframe");
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

void S2DGraphics::SetScreen(int screen)
{
    SDL_SetWindowPosition(EngineWindow, SDL_WINDOWPOS_CENTERED_DISPLAY(screen), SDL_WINDOWPOS_CENTERED_DISPLAY(screen));
    CurrentScreen = screen;
}

void S2DGraphics::SetResolution(ScreenResolution* res)
{
    SDL_SetWindowSize(EngineWindow, res->width, res->height);

    Parameters.BackBufferWidth = res->width;
    Parameters.BackBufferHeight = res->height;

    Device->Reset(&Parameters);

    CustomRenderTarget->Release();

    Device->CreateRenderTarget(Parameters.BackBufferWidth, Parameters.BackBufferHeight, Parameters.BackBufferFormat, D3DMULTISAMPLE_NONE, 0, FALSE, &CustomRenderTarget, NULL);
    Device->GetRenderTarget(0, &DefaultRenderTarget);

    CurrentResolution = *res;
}

void S2DGraphics::SetResolution(int w, int h)
{
    SDL_SetWindowSize(EngineWindow, w, h);

    Parameters.BackBufferWidth = w;
    Parameters.BackBufferHeight = h;

    Device->Reset(&Parameters);

    CustomRenderTarget->Release();

    Device->CreateRenderTarget(Parameters.BackBufferWidth, Parameters.BackBufferHeight, Parameters.BackBufferFormat, D3DMULTISAMPLE_NONE, 0, FALSE, &CustomRenderTarget, NULL);
    Device->GetRenderTarget(0, &DefaultRenderTarget);

    CurrentResolution = { w, h };
}

void S2DGraphics::SetFullscreen(bool state)
{
    Fullscreen = state;

    SDL_SetWindowFullscreen(EngineWindow, state ? SDL_WINDOW_FULLSCREEN_DESKTOP : NULL);

    int Width = 0, Height = 0;

    if (state)
    {
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(GetCurrentScreen(), &DM);
        Width = DM.w;
        Height = DM.h;
    }
    else
    {
        SDL_GetWindowSize(EngineWindow, &Width, &Height);
    }

    SDL_SetWindowFullscreen(EngineWindow, state ? SDL_WINDOW_FULLSCREEN_DESKTOP : NULL);

    Parameters.BackBufferWidth = Width;
    Parameters.BackBufferHeight = Height;
    Parameters.Windowed = !state;

    Device->Reset(&Parameters);

    CustomRenderTarget->Release();

    Device->CreateRenderTarget(Parameters.BackBufferWidth, Parameters.BackBufferHeight, Parameters.BackBufferFormat, D3DMULTISAMPLE_NONE, 0, FALSE, &CustomRenderTarget, NULL);
    Device->GetRenderTarget(0, &DefaultRenderTarget);
}

void S2DGraphics::EnableRenderTarget(bool state)
{
    if (state)
    {
        Device->SetRenderTarget(0, CustomRenderTarget);
    }
    else
    {
        Device->SetRenderTarget(0, DefaultRenderTarget);
    }

    renderTargetEnabled = state;
}

void S2DGraphics::RenderBox(Vec2 pos, Vec2 center, Vec2 size, S2DColor color)
{
    if (Device)
    {
        Vec2 pixPos = Vec2();
        Vec2 camPixPos = Vec2();

        Vec2Int scrSize = GetCurrentWindowSize();
        pixPos.x = (pos.x / 16.0f) * scrSize.x;
        pixPos.y = (pos.y / 16.0f) * scrSize.y;

        camPixPos.x = (currentCamera->Position.x / 16.0f) * scrSize.x;
        camPixPos.y = (currentCamera->Position.y / 16.0f) * scrSize.y;

        Vec2 v1(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2));
        Vec2 v2(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x, pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2));
        Vec2 v3(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x, pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y);
        Vec2 v4(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y);

        RenderLineOnScreen(v1, v2, 3, color);
        RenderLineOnScreen(v2, v3, 3, color);
        RenderLineOnScreen(v3, v4, 3, color);
        RenderLineOnScreen(v4, v1, 3, color);
    }
}

void S2DGraphics::RenderFilledBox(Vec2 pos, Vec2 center, Vec2 size, S2DColor color)
{
    if (Device)
    {
        Vec2 pixPos = Vec2();
        Vec2 camPixPos = Vec2();

        Vec2Int scrSize = GetCurrentWindowSize();
        pixPos.x = (pos.x / 16.0f) * scrSize.x;
        pixPos.y = (pos.y / 16.0f) * scrSize.y;

        camPixPos.x = (currentCamera->Position.x / 16.0f) * scrSize.x;
        camPixPos.y = (currentCamera->Position.y / 16.0f) * scrSize.y;

        const Vertex2D rect[] = {
            { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2),			pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2),			0, 1.0f,	D3DCOLOR_ARGB(color.a, color.r, color.g, color.b) },
            { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x,	pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2),			0, 1.0f,	D3DCOLOR_ARGB(color.a, color.r, color.g, color.b) },
            { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2),			pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y, 0, 1.0f,	D3DCOLOR_ARGB(color.a, color.r, color.g, color.b) },
            { pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x,	pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y, 0, 1.0f,	D3DCOLOR_ARGB(color.a, color.r, color.g, color.b) },
        };

        Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        Device->SetPixelShader(NULL);
        Device->SetVertexShader(NULL);
        Device->SetRenderState(D3DRS_ZENABLE, TRUE);
        Device->SetTexture(0, NULL);
        Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &rect, sizeof(Vertex2D));
    }
}

void S2DGraphics::RenderLineOnScreen(Vec2 p1, Vec2 p2, float width, S2DColor color)
{
    ID3DXLine* line;

    D3DXCreateLine(Device, &line);

    line->SetWidth(width);
    line->SetPattern(D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));

    D3DXVECTOR2 points[2];

    points[0] = D3DXVECTOR2(p1.x, p1.y);
    points[1] = D3DXVECTOR2(p2.x, p2.y);

    line->Draw(points, 2, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));

    line->Release();
}

void S2DGraphics::RenderLine(Vec2 p1, Vec2 p2, float width, S2DColor color)
{
    ID3DXLine* line;

    D3DXCreateLine(Device, &line);

    line->SetWidth(width);
    line->SetPattern(D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));

    D3DXVECTOR2 points[2];

    Vec2 pixPos1 = Vec2();
    Vec2 pixPos2 = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos1.x = (p1.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos1.y = (p2.y / 16.0f) * scrSize.y + (scrSize.x / 2);
    pixPos2.x = (p1.x / 16.0f) * scrSize.x + (scrSize.x / 2);
    pixPos2.y = (p2.y / 16.0f) * scrSize.y + (scrSize.x / 2);

    points[0] = D3DXVECTOR2(pixPos1.x, pixPos1.y);
    points[1] = D3DXVECTOR2(pixPos2.x, pixPos2.y);

    line->Draw(points, 2, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b));

    line->Release();
}

#define CIRCLE_RES 64

void S2DGraphics::RenderPoint(Vec2 position, float size, S2DColor color)
{
    Vertex2D verts[CIRCLE_RES + 1];

    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (position.x / 16.0f) * scrSize.x;
    pixPos.y = (position.y / 16.0f) * scrSize.y;

    camPixPos.x = (currentCamera->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (currentCamera->Position.y / 16.0f) * scrSize.y;

    for (int i = 0; i < CIRCLE_RES + 1; i++)
    {
        verts[i].x = camPixPos.x + size * cos(D3DX_PI * (i / (CIRCLE_RES / 2.0f)));
        verts[i].y = camPixPos.y + size * sin(D3DX_PI * (i / (CIRCLE_RES / 2.0f)));
        verts[i].z = 0;
        verts[i].rhw = 1;
        verts[i].color = D3DCOLOR_ARGB(color.a, color.r, color.g, color.b);
    }

    Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    Device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, CIRCLE_RES - 1, &verts, sizeof(Vertex2D));
}

void S2DGraphics::RenderSprite(S2DSprite* sprite, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, S2DColor color, S2DShader* shader)
{
    LPDIRECT3DSTATEBLOCK9 pStateBlock = NULL;
    Device->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
    pStateBlock->Capture();

    struct TexVertex {
        float x, y, z;
        float rhw;
        DWORD color;
        float tu, tv;
    };

    Rect crop = sprite->GetFrames()[sprite->GetCurFrame()];

    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (currentCamera->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (currentCamera->Position.y / 16.0f) * scrSize.y;

    TexVertex g_square_vertices[] = {
        { (float)pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), (float)pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), (float)(crop.x / sprite->GetTexture()->width), (float)(crop.y / sprite->GetTexture()->height) },
        { (float)(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x), (float)pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), (float)((crop.x + crop.w) / sprite->GetTexture()->width), (float)(crop.y / sprite->GetTexture()->height) },
        { (float)pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), (float)(pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), (float)(crop.x / sprite->GetTexture()->width), (float)((crop.y + crop.h) / sprite->GetTexture()->height) },
        { (float)(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x), (float)(pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), (float)((crop.x + crop.w) / sprite->GetTexture()->width), (float)((crop.y + crop.h) / sprite->GetTexture()->height) }
    };

    Device->SetRenderState(D3DRS_ZENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHAREF, 0x08);
    Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    Device->SetRenderState(D3DRS_CLIPPING, TRUE);
    Device->SetRenderState(D3DRS_VERTEXBLEND, FALSE);
    Device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    Device->SetRenderState(D3DRS_FOGENABLE, FALSE);

    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

    Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
    Device->SetTexture(0, sprite->GetTexture()->GetDX9Texture());
    
    if (shader)
    {
        Device->SetPixelShader(shader->GetPixelShader());
        Device->SetVertexShader(shader->GetVertexShader());
    }
    else
    {
        Device->SetPixelShader(NULL);
        Device->SetVertexShader(NULL);
    }

    Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &g_square_vertices, sizeof(TexVertex));

    if (pStateBlock) {
        pStateBlock->Apply();
        pStateBlock->Release();
        pStateBlock = nullptr;
    }
}

void S2DGraphics::RenderScreenTexture(S2DTexture* tex, Vec2Int pos, Vec2Int size, S2DColor color, S2DShader* shader)
{
    LPDIRECT3DSTATEBLOCK9 pStateBlock = NULL;
    Device->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
    pStateBlock->Capture();

    struct texturedVertex {
        float x, y, z;
        float rhw;
        DWORD color;
        float tu, tv;
    };

    texturedVertex g_square_vertices[] = {
        { (float)pos.x, (float)pos.y, 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 0.0f, 0.0f },
        { (float)(pos.x + size.x), (float)pos.y, 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 1.0f, 0.0f },
        { (float)pos.x, (float)(pos.y + size.y), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 0.0f, 1.0f },
        { (float)(pos.x + size.x), (float)(pos.y + size.y), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 1.0f, 1.0f }
    };

    Device->SetRenderState(D3DRS_ZENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHAREF, 0x08);
    Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    Device->SetRenderState(D3DRS_CLIPPING, TRUE);
    Device->SetRenderState(D3DRS_VERTEXBLEND, FALSE);
    Device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    Device->SetRenderState(D3DRS_FOGENABLE, FALSE);

    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

    Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
    Device->SetTexture(0, tex->GetDX9Texture());
    
    if (shader)
    {
        Device->SetPixelShader(shader->GetPixelShader());
        Device->SetVertexShader(shader->GetVertexShader());
    }
    else
    {
        Device->SetPixelShader(NULL);
        Device->SetVertexShader(NULL);
    }

    Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &g_square_vertices, sizeof(texturedVertex));

    if (pStateBlock) {
        pStateBlock->Apply();
        pStateBlock->Release();
        pStateBlock = nullptr;
    }
}

void S2DGraphics::RenderTexture(S2DTexture* tex, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, S2DColor color, S2DShader* shader)
{
    LPDIRECT3DSTATEBLOCK9 pStateBlock = NULL;
    Device->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
    pStateBlock->Capture();

    struct TexVertex {
        float x, y, z;
        float rhw;
        DWORD color;
        float tu, tv;
    };

    Vec2 pixPos = Vec2();
    Vec2 camPixPos = Vec2();

    Vec2Int scrSize = GetCurrentWindowSize();
    pixPos.x = (pos.x / 16.0f) * scrSize.x;
    pixPos.y = (pos.y / 16.0f) * scrSize.y;

    camPixPos.x = (currentCamera->Position.x / 16.0f) * scrSize.x;
    camPixPos.y = (currentCamera->Position.y / 16.0f) * scrSize.y;

    TexVertex g_square_vertices[] = {
        { (float)pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), (float)pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 0.0f, 0.0f },
        { (float)(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x), (float)pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 1.0f, 0.0f },
        { (float)pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2), (float)(pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 0.0f, 1.0f },
        { (float)(pixPos.x - (size.x * center.x) - camPixPos.x + (scrSize.x / 2) + size.x), (float)(pixPos.y - (size.y * center.y) + camPixPos.y + (scrSize.y / 2) + size.y), 0, 1.0f, D3DCOLOR_ARGB(color.a, color.r, color.g, color.b), 1.0f, 1.0f }
    };

    Device->SetRenderState(D3DRS_ZENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHAREF, 0x08);
    Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    Device->SetRenderState(D3DRS_CLIPPING, TRUE);
    Device->SetRenderState(D3DRS_VERTEXBLEND, FALSE);
    Device->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
    Device->SetRenderState(D3DRS_FOGENABLE, FALSE);

    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

    Device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE);
    Device->SetTexture(0, tex->GetDX9Texture());

    if (shader)
    {
        Device->SetPixelShader(shader->GetPixelShader());
        Device->SetVertexShader(shader->GetVertexShader());
    }
    else
    {
        Device->SetPixelShader(NULL);
        Device->SetVertexShader(NULL);
    }
    
    Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &g_square_vertices, sizeof(TexVertex));

    if (pStateBlock) {
        pStateBlock->Apply();
        pStateBlock->Release();
        pStateBlock = nullptr;
    }
}

S2DTexture* S2DGraphics::LoadTexture(const char* fileName)
{
    IDirect3DTexture9* tex;

    if (FAILED(D3DXCreateTextureFromFileA(Device, fileName, &tex)))
    {
        S2DFatalErrorFormatted("Failed to load texture:\n%s", fileName);
    }

    return new S2DTexture(tex, fileName);
}

bool S2DGraphics::UnloadTexture(S2DTexture* texture)
{
    auto tex = texture->GetDX9Texture();

    if (tex)
    {
        tex->Release();

        delete texture;

        return true;
    }

    return false;
}

void S2DGraphics::BeginFrame()
{
    Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0);

    Device->BeginScene();
}

void S2DGraphics::EndFrame()
{
    Device->EndScene();
    Device->Present(NULL, NULL, NULL, NULL);
}
#pragma endregion