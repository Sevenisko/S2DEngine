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

#include <SDL.h>
#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <functional>

struct Color
{
public:
	Uint8 r = 0, g = 0, b = 0, a = 0;

	Color(Uint8 r, Uint8 g, Uint8 b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 255;
	}
	Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	static Color Lerp(Color c1, Color c2, float f)
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

		OutputDebugStringA(("Color: {" + std::to_string(r3) + ", " + std::to_string(g3) + ", " + std::to_string(b3) + ", " + std::to_string(a3) + "}\n").c_str());

		Uint8 r = (int)r3;
		Uint8 g = (int)g3;
		Uint8 b = (int)b3;
		Uint8 a = (int)a3;

		return Color(r, g, b, a);
	}

	static Color Black() { return Color(0, 0, 0); }
	static Color White() { return Color(255, 255, 255); }
	static Color Red() { return Color(255, 0, 0); }
	static Color Green() { return Color(0, 255, 0); }
	static Color Blue() { return Color(0, 0, 255); }
	static Color Yellow() { return Color(255, 255, 0); }
	static Color Aqua() { return Color(0, 255, 255); }
	static Color Pink() { return Color(255, 0, 255); }
	static Color Gray() { return Color(125, 125, 125); }

	static Color LightGray() { return Color(175, 175, 175); }
	static Color LightRed() { return Color(255, 64, 64); }
	static Color LightGreen() { return Color(64, 255, 64); }
	static Color LightBlue() { return Color(64, 64, 255); }
	static Color LightYellow() { return Color(255, 255, 64); }
	static Color LightAqua() { return Color(64, 255, 255); }
	static Color LightPink() { return Color(255, 64, 255); }

	static Color DarkRed() { return Color(150, 0, 0); }
	static Color DarkGreen() { return Color(0, 150, 0); }
	static Color DarkBlue() { return Color(0, 0, 150); }
	static Color DarkYellow() { return Color(150, 150, 0); }
	static Color DarkAqua() { return Color(0, 150, 150); }
	static Color DarkPink() { return Color(150, 0, 150); }
	static Color DarkGray() { return Color(75, 75, 75); }

	Color operator =(Color a)
	{
		this->r = a.r;
		this->g = a.g;
		this->b = a.b;
		this->a = a.a;
		return *this;
	}

	bool operator==(Color a) const
	{
		return (this->r == a.r && this->g == a.g && this->b == a.b && this->a == a.a);
	}

	bool operator!=(Color a) const
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
    int screenNum;
    ScreenResolution* resolution;
    bool fullscreen;
};

#define S2DWorldPosToPixels(relX, relY, X, Y) Vec2Int scrSize = Graphics->GetCurrentWindowSize(); \
	X = (relX / 16.0f) * scrSize.x; \
	Y = (relY / 16.0f) * scrSize.y;

#define S2DPixelsToWorldPos(X, Y, relX, relY) Vec2Int scrSize = Graphics->GetCurrentWindowSize(); \
	relX = (float)(X / (scrSize.x - (scrSize.x / 2))) * 16.0f; \
	relY = (float)(Y / (scrSize.y - (scrSize.y / 2))) * 16.0f; 
	
#ifdef S2D_MAIN_INCLUDED
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif // S2D_MAIN_INCLUDED

class DllExport S2DCamera
{
public:
	Vec2 Position;
};

class DllExport S2DTexture
{
public:
    int width, height, textureID;
    SDL_Texture* GetSDLTexture() { return nativeTexture; }
	const char* GetPath() { return texPath; }

	S2DTexture(SDL_Texture* tex, const char* path)
	{
		width = height = 0;
		texPath = path;
		nativeTexture = tex;
		SDL_QueryTexture(tex, NULL, NULL, &width, &height);
	}
private:
	const char* texPath;
    SDL_Texture* nativeTexture;
};

class DllExport S2DFont
{
public:
	Vec2Int GetSize(int size, const char* text);

	void UpdateRenderer(SDL_Renderer* renderer);

	void Render(int size, const char* text, Vec2 pos, Vec2 center, float angle, TexFlipMode flip, Color color);

	S2DTexture* RenderToTexture(int size, const char* text, Color color);

	S2DFont() {}

	S2DFont(SDL_Renderer* renderer, const char* path);
private:
	std::string ttfFile;
	SDL_Renderer* myRenderer;
};

class DllExport S2DSprite
{
public:
    S2DTexture* GetTexture() { return texture; }
	void UpdateTexture(S2DTexture* tex) { texture = tex; }
	int GetCurFrame() { return curFrame; }
	int GetFrameRate() { return framerate; }
	std::vector<SDL_Rect> GetFrames() { return frames; }
	void SetFrame(int frame);
	void NextFrame();
	void PrevFrame();

	S2DSprite() {}
	S2DSprite(S2DTexture* tex, Vec2Int sprSize, int frameRate, int numFrames);
	S2DSprite(S2DTexture* tex, std::vector<Vec2Int> sprSizes, int frameRate);
	~S2DSprite();

private:
    std::vector<SDL_Rect> frames;
    S2DTexture* texture;
	int curFrame;
    int framerate;
};

// Class containing the Graphics Subsystem
class DllExport S2DGraphics
{
public:
	std::function<void()> OnRenderReload;

	// Get current screen resolution
	ScreenResolution* GetCurrentResolution() { return &CurrentResolution; }

	// Get current window size
	Vec2Int GetCurrentWindowSize();

	// Get current screen (useful when using more screens)
	int GetCurrentScreen() { return CurrentScreen; }

    // Get a SDL Window instance
    SDL_Window* GetWindow() { return EngineWindow; }
    // Get a SDL Renderer instance
    SDL_Renderer* GetRenderer() { return NativeRenderer; }
    // Get an Direct3D9 Graphics Device created by SDL Renderer
	IDirect3DDevice9* GetDX9Device();

	// Set screen for window (useful when using more screens)
	void SetScreen(int screen);

	// Set a screen resolution
    void SetResolution(ScreenResolution* resolution);
    void SetResolution(int w, int h);

	// Enable/Disable fullscreen
    void SetFullscreen(bool state);

	// Draws an filled box
	void RenderFilledBox(Vec2 pos, Vec2 size, Color color);

	// Draws an wireframe box
	void RenderBox(Vec2 pos, Vec2 size, Color color);

	// Drawns an basic line
	void RenderLine(Vec2 p1, Vec2 p2, Color color);

	// Draws an single point
	void RenderPoint(Vec2 position, Color color);

	// Draws an sprite
	void RenderSprite(S2DSprite* sprite, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color);

	// Draws an texture
	void RenderTexture(int textureID, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color);

	// Draws an texture
	void RenderTexture(S2DTexture* tex, Vec2 pos, Vec2 center, Vec2 size, float angle, TexFlipMode flip, Color color);

	// Loads an texture from file
	int LoadTexture(const char* fileName);
	// Loads an texture from file
	S2DTexture* LoadTextureRaw(const char* fileName);

	// Get an texture from textureID
	S2DTexture* GetTextureByID(int texID);
	
	// Unloads an specific texture
	bool UnloadTexture(int textureID);
	// Unloads an specific texture
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

    SDL_Window* EngineWindow;
    SDL_Renderer* NativeRenderer;
	ScreenResolution CurrentResolution;
	int CurrentScreen;
    bool Fullscreen;
};

#endif // !S2D_GFX_INCLUDED
