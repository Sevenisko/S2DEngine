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

    Purpose: Plugin instances for S2D Engine
\************************************************************/

typedef void(__stdcall* OnPLoad)();
typedef void(__stdcall* OnPUnload)();
typedef void(__stdcall* OnPUpdate)();
typedef void(__stdcall* OnPRender)();
typedef void(__stdcall* OnPRenderUpdate)();
typedef void(__stdcall* OnPSDLEvent)(SDL_Event evnt);

class S2DPlugin
{
public:
    S2DPlugin(const char* dllFile)
    {
        HINSTANCE dllInstance = LoadLibraryA(dllFile);

        if (!dllInstance) S2DFatalError("Failed to load plugin DLL file!");

        OnLoad = (OnPLoad)GetProcAddress(dllInstance, "OnPluginLoad");
        OnUnload = (OnPUnload)GetProcAddress(dllInstance, "OnPluginUnload");
        OnUpdate = (OnPUpdate)GetProcAddress(dllInstance, "OnPluginUpdate");
        OnRender = (OnPRender)GetProcAddress(dllInstance, "OnPluginRender");
        OnRenderUpdate = (OnPRenderUpdate)GetProcAddress(dllInstance, "OnPluginRenderUpdate");
        OnSDLEvent = (OnPSDLEvent)GetProcAddress(dllInstance, "OnPluginSDLEvent");
    }

    OnPLoad OnLoad;
    OnPUnload OnUnload;
    OnPUpdate OnUpdate;
    OnPRender OnRender;
    OnPRenderUpdate OnRenderUpdate;
    OnPSDLEvent OnSDLEvent;
};