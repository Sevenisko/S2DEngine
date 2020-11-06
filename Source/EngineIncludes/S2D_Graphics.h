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

    Purpose: Implementation of the Graphics subsystem
\************************************************************/

#ifndef S2D_GFX_INCLUDED
#define S2D_GFX_INCLUDED

#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <functional>

#if defined(S2D_INCLUDE_DX9)
#include <d3d9.h>
#include <d3dx9.h>
#else
typedef struct IDirect3DTexture9 IDirect3DTexture9;
#endif

#if defined(S2D_INCLUDE_SDL)
#include <SDL.h>
#else
typedef struct SDL_Window SDL_Window;
#endif

typedef uint8_t Uint8;

struct S2DColor
{
public:
	Uint8 r = 0, g = 0, b = 0, a = 0;

	S2DColor(Uint8 r, Uint8 g, Uint8 b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 255;
	}
	S2DColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	static S2DColor Lerp(S2DColor c1, S2DColor c2, float f)
	{
		float r1 = (float)c1.r;
		float g1 = (float)c1.g;
		float b1 = (float)c1.b;
		float a1 = (float)c1.a;

		float r2 = (float)c2.r;
		float g2 = (float)c2.g;
		float b2 = (float)c2.b;
		float a2 = (float)c2.a;

		float r3 = (1 - f) * r1 + f * r2;
		float g3 = (1 - f) * g1 + f * g2;
		float b3 = (1 - f) * b1 + f * b2;
		float a3 = (1 - f) * a1 + f * a2;

		OutputDebugStringA(("S2DColor: {" + std::to_string(r3) + ", " + std::to_string(g3) + ", " + std::to_string(b3) + ", " + std::to_string(a3) + "}\n").c_str());

		Uint8 r = (int)r3;
		Uint8 g = (int)g3;
		Uint8 b = (int)b3;
		Uint8 a = (int)a3;

		return S2DColor(r, g, b, a);
	}

	static S2DColor Black() { return S2DColor(0, 0, 0); }
	static S2DColor White() { return S2DColor(255, 255, 255); }
	static S2DColor Red() { return S2DColor(255, 0, 0); }
	static S2DColor Green() { return S2DColor(0, 255, 0); }
	static S2DColor Blue() { return S2DColor(0, 0, 255); }
	static S2DColor Yellow() { return S2DColor(255, 255, 0); }
	static S2DColor Aqua() { return S2DColor(0, 255, 255); }
	static S2DColor Pink() { return S2DColor(255, 0, 255); }
	static S2DColor Gray() { return S2DColor(125, 125, 125); }

	static S2DColor LightGray() { return S2DColor(175, 175, 175); }
	static S2DColor LightRed() { return S2DColor(255, 64, 64); }
	static S2DColor LightGreen() { return S2DColor(64, 255, 64); }
	static S2DColor LightBlue() { return S2DColor(64, 64, 255); }
	static S2DColor LightYellow() { return S2DColor(255, 255, 64); }
	static S2DColor LightAqua() { return S2DColor(64, 255, 255); }
	static S2DColor LightPink() { return S2DColor(255, 64, 255); }

	static S2DColor DarkRed() { return S2DColor(150, 0, 0); }
	static S2DColor DarkGreen() { return S2DColor(0, 150, 0); }
	static S2DColor DarkBlue() { return S2DColor(0, 0, 150); }
	static S2DColor DarkYellow() { return S2DColor(150, 150, 0); }
	static S2DColor DarkAqua() { return S2DColor(0, 150, 150); }
	static S2DColor DarkPink() { return S2DColor(150, 0, 150); }
	static S2DColor DarkGray() { return S2DColor(75, 75, 75); }

	S2DColor operator =(S2DColor a)
	{
		this->r = a.r;
		this->g = a.g;
		this->b = a.b;
		this->a = a.a;
		return *this;
	}

	bool operator==(S2DColor a) const
	{
		return (this->r == a.r && this->g == a.g && this->b == a.b && this->a == a.a);
	}

	bool operator!=(S2DColor a) const
	{
		return (this->r != a.r || this->g != a.g || this->b != a.b || this->a != a.a);
	}
};

struct Vec2
{
	float x = 0.0f, y = 0.0f;

public:
	Vec2()
	{
		this->x = this->y = 0;
	}
	Vec2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	static Vec2 Lerp(Vec2 v1, Vec2 v2, float f)
	{
		float x = v1.x + (v2.x - v1.x) * f;
		float y = v1.y + (v2.y - v1.y) * f;

		return Vec2(x, y);
	}

	static Vec2 DistanceVec(Vec2 v1, Vec2 v2)
	{
		float diffX = v1.x - v2.x;
		float diffY = v1.y - v2.y;

		return Vec2(diffX, diffY);
	}

	static float Distance(Vec2 v1, Vec2 v2)
	{
		float diffX = v1.x - v2.x;
		float diffY = v1.y - v2.y;

		return sqrtf((diffY * diffY) + (diffX * diffX));
	}

	Vec2 operator =(Vec2 a)
	{
		this->x = a.y;
		this->x = a.y;
		return *this;
	}

	bool operator==(Vec2 a) const
	{
		return (this->x == a.x && this->y == a.y);
	}
};

struct Vec2Int
{
	int x = 0, y = 0;

public:
	Vec2Int()
	{
		this->x = this->y = 0;
	}
	Vec2Int(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	static Vec2Int Lerp(Vec2Int v1, Vec2Int v2, float f)
	{
		float x = v1.x + (v2.x - v1.x) * f;
		float y = v1.y + (v2.y - v1.y) * f;

		return Vec2Int((int)x, (int)y);
	}

	Vec2Int operator =(Vec2Int a)
	{
		this->x = a.y;
		this->x = a.y;
		return *this;
	}

	bool operator==(Vec2Int a) const
	{
		return (this->x == a.x && this->y == a.y);
	}
};

enum class TexFlipMode
{
	None,
	Horizontal,
	Vertical
};

struct ScreenResolution
{
    int width, height;
};

extern std::vector<ScreenResolution> GetResolutions();

extern ScreenResolution* GetResolution(int width, int height);

struct EngineInitSettings
{
    const char* title;
    ScreenResolution* resolution;
    bool fullscreen, vsync;
};
	
#ifdef S2D_MAIN_INCLUDED
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif // S2D_MAIN_INCLUDED

struct Rect
{
	int x, y;
	int w, h;
};

struct FRect
{
	float x;
	float y;
	float w;
	float h;
};

class DllExport S2DCamera
{
public:
	Vec2 Position;
};

class DllExport S2DTexture
{
public:
    int width, height, textureID;
#if defined(S2D_INCLUDE_DX9)
	IDirect3DTexture9* GetDX9Texture() { return nativeTexture; }
	D3DFORMAT GetDX9TextureFormat() { return textureFormat; }
#endif
	const char* GetPath() { return texPath; }

	S2DTexture() {};

#if defined(S2D_INCLUDE_DX9)
	S2DTexture(IDirect3DTexture9* tex, const char* path)
	{
		width = height = 0;
		texPath = path;
		nativeTexture = tex;
		D3DSURFACE_DESC desc;
		tex->GetLevelDesc(0, &desc);
		width = desc.Width;
		height = desc.Height;
	}
#endif

private:
	const char* texPath;
#if defined(S2D_INCLUDE_DX9)
	IDirect3DTexture9* nativeTexture;
	D3DFORMAT textureFormat;
#endif
};

class DllExport S2DShader
{
private:
#if defined(S2D_INCLUDE_DX9)
	IDirect3DVertexShader9* VertexShader;
	IDirect3DPixelShader9* PixelShader;
	LPD3DXCONSTANTTABLE* VertexConstantTable;
	LPD3DXCONSTANTTABLE* PixelConstantTable;
#endif

public:
	S2DShader() {};
	S2DShader(const char* path);

	~S2DShader();
#if defined(S2D_INCLUDE_DX9)
	IDirect3DVertexShader9* GetVertexShader() { return VertexShader; }
	IDirect3DPixelShader9* GetPixelShader() { return PixelShader; }
#endif
};

class DllExport S2DFont
{
public:
	Vec2 GetSize(const char* text);

	void Render(float scale, const char* text, Vec2 pos, Vec2 center, float angle, S2DColor S2DColor);

	S2DFont() {}

	~S2DFont();

	S2DFont(const char* name, int size, bool bold);
private:
#if defined(S2D_INCLUDE_DX9)
	ID3DXFont* font;
#endif
};

class DllExport S2DSprite
{
public:
    S2DTexture* GetTexture() { return texture; }
	void UpdateTexture(S2DTexture* tex) { texture = tex; }
	int GetCurFrame() { return curFrame; }
	int GetFrameRate() { return framerate; }
	std::vector<Rect> GetFrames() { return frames; }
	void SetFrame(int frame);
	void NextFrame();
	void PrevFrame();

	S2DSprite() {}
	S2DSprite(S2DTexture* tex, Vec2Int sprSize, int frameRate, int numFrames);
	S2DSprite(S2DTexture* tex, std::vector<Vec2Int> sprSizes, int frameRate);
	~S2DSprite() {}

private:
    std::vector<Rect> frames;
    S2DTexture* texture;
	int curFrame;
    int framerate;
};

// Class containing the Graphics Subsystem
class DllExport S2DGraphics
{
public:
	// Get current screen resolution
	ScreenResolution* GetCurrentResolution() { return &CurrentResolution; }

	// Get current window size
	Vec2Int GetCurrentWindowSize();

	// Get current screen (useful when using more screens)
	int GetCurrentScreen() { return CurrentScreen; }

#if defined (S2D_INCLUDE_SDL)
    // Get the SDL Window instance
    SDL_Window* GetWindow() { return EngineWindow; }
#endif

#if defined (S2D_INCLUDE_DX9)
	// Get the Direct3D9 Interface
	IDirect3D9* GetDX9Interface() { return Interface; }

	// Get the Direct3D9 Graphics Device
	IDirect3DDevice9* GetDX9Device() { return Device; }
#endif

	// Set screen for window (useful when using more screens)
	void SetScreen(int screen);

	// Set a screen resolution
    void SetResolution(ScreenResolution* resolution);
    void SetResolution(int w, int h);

	// Enable/Disable fullscreen
    void SetFullscreen(bool state);

	// Set the Engine's camera
	void SetCamera(S2DCamera* cam);

	// Take and save the screenshot
	void Screenshot(const char* name);

	// Enable rendering into secondary render target
	void EnableRenderTarget(bool state);

	bool RenderTargetEnabled() { return renderTargetEnabled; }

	// Take the screenshot and convert it into the texture
	S2DTexture* GetCurrentFrame();

	// Get the current Engine's camera
	S2DCamera* GetCurrentCamera() { return currentCamera; }

	// Draw filled box
	void RenderFilledBox(Vec2 pos, Vec2 center, Vec2 size, S2DColor S2DColor);

	// Draw wireframe box
	void RenderBox(Vec2 pos, Vec2 center, Vec2 size, S2DColor S2DColor);

	// Draw basic line
	void RenderLine(Vec2 p1, Vec2 p2, float width, S2DColor S2DColor);

	// Draw basic line using screen coordinates
	void RenderLineOnScreen(Vec2 p1, Vec2 p2, float width, S2DColor color);

	// Draw single point
	void RenderPoint(Vec2 position, float size, S2DColor S2DColor);

	// Draw a sprite
	void RenderSprite(S2DSprite* sprite, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, S2DColor S2DColor, S2DShader* shader);
	
	// Draw texture
	void RenderTexture(S2DTexture* tex, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, S2DColor S2DColor, S2DShader* shader);

	// Draw texture on screen space
	void RenderScreenTexture(S2DTexture* tex, Vec2Int pos, Vec2Int size, S2DColor S2DColor, S2DShader* shader);

	// Load a texture from file
	S2DTexture* LoadTexture(const char* fileName);

	// Get an texture from textureID
	S2DTexture* GetTextureByID(int texID);
	
	// Unloads a specific texture
	bool UnloadTexture(S2DTexture* texture);

	// Get all paths to loaded textures for reload
	std::map<int, const char*> GetReloadTextures();

	// Unload all the loaded textures
	void ClearTextures();

    // Starts the rendering frame sequence
    void BeginFrame();

    // Ends the rendering frame sequence and draws the buffer to screen
    void EndFrame();

    S2DGraphics() {};

    // Creates an instance of 2D Rendering Engine
    S2DGraphics(EngineInitSettings* settings);

    // Destroys the current instance of 2D Rendering Engine
    ~S2DGraphics();

    bool GetFullscreen() { return Fullscreen; }

	bool IsRunning() { return Running; }

private:
	std::vector<S2DTexture*> LoadedTextures;

	bool Running;

#if defined (S2D_INCLUDE_SDL)
    SDL_Window* EngineWindow;
#endif
#if defined (S2D_INCLUDE_DX9)
	IDirect3D9* Interface;
	D3DPRESENT_PARAMETERS Parameters;
    IDirect3DDevice9* Device;

	IDirect3DSurface9* DefaultRenderTarget;
	IDirect3DSurface9* CustomRenderTarget;
#endif

	bool renderTargetEnabled;

	S2DCamera* currentCamera;

	ScreenResolution CurrentResolution;
	int CurrentScreen;
    bool Fullscreen;
};

#endif // !S2D_GFX_INCLUDED
