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
        
    Purpose: Implementation of core features of S2D Engine
\************************************************************/

#ifndef S2D_CORE_INCLUDED
#define S2D_CORE_INCLUDED

#include <Windows.h>
#include <SDL.h>

typedef struct IDirect3DTexture9 IDirect3DTexture9;

struct VersionInfo
{
    int major;
    int minor;
    int build;
};

struct GameSplashScreen
{
    const char* imagePath;
    int time;
};

// Default Engine Settings
extern EngineInitSettings* defaultSettings;

// Base class for manipulation with S2D Engine
class DllExport S2DGame
{
public:
    S2DGraphics* Graphics;
    //S2DPhysics* Physics;

    float GetDeltaTime();

    float GetFrameRate();

    float GetFrameTime();

    VersionInfo GetEngineVersion();
    const char* GetBuildDate();
    const char* GetBuildTime();

    // This function is called on initialization
    virtual void OnInit() {}

    // This function is called when application quits
    virtual void OnQuit() {}

    // This function is called when SDL event is called
    virtual void OnSDLEvent(SDL_Event event) {}

    // This function is called when fucus is gained
    virtual void OnFocusGained() {}

    // This function is called when focus is lost
    virtual void OnFocusLost() {}

    // This function is called every frame (Pre-Render) and updates the game logic
    virtual void OnUpdate() {}

    // This function is called every frame and draws things into the buffer
    virtual void OnRender() {}

    virtual void OnPostRender(S2DTexture* frame) {}

    // Starts the game engine
    void Run(GameSplashScreen* splash);

    // Quits the game engine
    void Quit();

    S2DGame() {};

    S2DGame(EngineInitSettings* settings);

    ~S2DGame();

    Vec2 Gravity = Vec2(0, -10.0f);

private:
    HANDLE MutexHandle;

    EngineInitSettings* CurrentSettings;

    bool WindowFocused = false;

    void HandleEvents(SDL_Event& e);

    float Framerate, Frametime, Deltatime;
};

#endif // !S2D_CORE_INCLUDED
