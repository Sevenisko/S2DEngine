#include "EngineIncludes.h"
#include <string>
#include <thread>

EngineInitSettings* defaultSettings = new EngineInitSettings{ "S2D Game Engine", 0, new ScreenResolution {1280, 720}, false };

bool IsConsoleApp()
{
    PIMAGE_NT_HEADERS header = ImageNtHeader((PVOID)GetModuleHandle(NULL));
    if (header->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI)
    {
        return true;
    }

    return false;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

void PrintModifiers(Uint16 mod) {
    OutputDebugStringA("Modifers: ");

    if (mod == KMOD_NONE) {
        OutputDebugStringA("None\n");
        return;
    }

    if (mod & KMOD_NUM) OutputDebugStringA("NUMLOCK ");
    if (mod & KMOD_CAPS) OutputDebugStringA("CAPSLOCK ");
    if (mod & KMOD_LCTRL) OutputDebugStringA("LCTRL ");
    if (mod & KMOD_RCTRL) OutputDebugStringA("RCTRL ");
    if (mod & KMOD_RSHIFT) OutputDebugStringA("RSHIFT ");
    if (mod & KMOD_LSHIFT) OutputDebugStringA("LSHIFT ");
    if (mod & KMOD_RALT) OutputDebugStringA("RALT ");
    if (mod & KMOD_LALT) OutputDebugStringA("LALT ");
    if (mod & KMOD_CTRL) OutputDebugStringA("CTRL ");
    if (mod & KMOD_SHIFT) OutputDebugStringA("SHIFT ");
    if (mod & KMOD_ALT) OutputDebugStringA("ALT ");
    OutputDebugStringA("\n");
}

void PrintKeyInfo(SDL_KeyboardEvent* key) {
    /* Is it a release or a press? */
    if (key->type == SDL_KEYUP)
        OutputDebugStringA("Release:- ");
    else
        OutputDebugStringA("Press:- ");

    /* Print the hardware scancode first */
    OutputDebugStringA(("Scancode: " + std::to_string(key->keysym.scancode)).c_str());
    /* Print the name of the key */
    OutputDebugStringA((", Name: " + std::string(SDL_GetKeyName(key->keysym.sym))).c_str());
    OutputDebugStringA("\n");
    /* Print modifier info */
    PrintModifiers(key->keysym.mod);
}

void S2DGame::HandleEvents(SDL_Event& e)
{  
    switch (e.type)
    {
    case SDL_QUIT:
        Quit();
        break;

    case SDL_WINDOWEVENT:
        switch (e.window.event)
        {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            WindowFocused = true;
            OnFocusGained();
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            WindowFocused = false;
            OnFocusLost();
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            WindowFocused = false;
            OnFocusLost();
            break;
        case SDL_WINDOWEVENT_RESTORED:
            WindowFocused = true;
            break;
        case SDL_WINDOWEVENT_CLOSE:
            Quit();
            break;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        if(WindowFocused) Input::ProcessMouseButton((MouseButton)(e.button.button - 1), true);
        break;

    case SDL_MOUSEBUTTONUP:
        if (WindowFocused) Input::ProcessMouseButton((MouseButton)(e.button.button - 1), false);
        break;

    case SDL_KEYUP:
        if (WindowFocused) Input::ProcessKey(e.key.keysym.scancode, false);
        break;

    case SDL_KEYDOWN:
        if ((e.key.keysym.mod & KMOD_RALT || e.key.keysym.mod & KMOD_LALT) && e.key.keysym.sym == SDLK_RETURN && WindowFocused)
        {
            OutputDebugStringA("Toggle Fullscreen\n");

            bool enabled = Graphics->GetFullscreen();

            if (enabled)
            {
                WindowFocused = false;
                OnFocusLost();
                Graphics->SetFullscreen(false);
                WindowFocused = true;
                OnFocusGained();
            }
            else
            {
                WindowFocused = false;
                OnFocusLost();
                Graphics->SetFullscreen(true);
                WindowFocused = true;
                OnFocusGained();
            }
        }
        if (WindowFocused) Input::ProcessKey(e.key.keysym.scancode, true);
        break;
    }

    OnSDLEvent(e);
}

VersionInfo S2DGame::GetEngineVersion()
{
    return { S2D_VER_MAJOR, S2D_VER_MINOR, S2D_VER_BUILD };
}

const char* S2DGame::GetBuildDate()
{
    return __DATE__;
}

const char* S2DGame::GetBuildTime()
{
    return __TIME__;
}

float S2DGame::GetDeltaTime()
{
    return Deltatime;
}

float S2DGame::GetFrameTime()
{
    return Frametime;
}

float S2DGame::GetFrameRate()
{
    return Framerate;
}

void S2DGame::Run()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(NULL);
    Mix_Init(NULL);
    TTF_Init();

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 8, 4096) < 0)
        S2DFatalErrorFormatted("Cannot initalizate sound system!\n\t%s", Mix_GetError());

    Graphics = new S2DGraphics(CurrentSettings);
    Physics = new S2DPhysics();
    SDL_RaiseWindow(Graphics->GetWindow());
    WindowFocused = true;

    OnInit();

    Graphics->OnRenderReload = [&]() { OnRenderReload(); };

    Uint64 g_Time = 0;

    float timeStep = 1.0f / 60.0f;

    for (;;)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            HandleEvents(event);
        }

        if (WindowFocused && Graphics->IsRunning())
        {
            Vec2Int delta;

            SDL_GetRelativeMouseState(&delta.x, &delta.y);
            
            Input::UpdateMouseDelta(delta.x, delta.y);

            Vec2Int size = Graphics->GetCurrentWindowSize();

            if (Input::mouseLocked)
                Input::UpdateMousePos(size.x / 2, size.y / 2);
            else
                Input::UpdateMousePos();

            Physics->Update(timeStep);

            OnUpdate();

            Graphics->BeginFrame();
            OnRender();
            Graphics->EndFrame();

            // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
            Uint64 frequency = SDL_GetPerformanceFrequency();
            Uint64 current_time = SDL_GetPerformanceCounter();
            Deltatime = g_Time > 0 ? (float)((double)(current_time - g_Time) / frequency) : (float)(1.0f / 60.0f);
            Framerate = 1.0f / Deltatime;
            Frametime = Deltatime * 1000.0f;
            g_Time = current_time;

            if (Input::mouseLocked)
            {
                SDL_WarpMouseGlobal(size.x / 2, size.y / 2);
            }

            //OutputDebugStringA(("Framerate: " + std::to_string(Framerate) + "(" + std::to_string(Frametime) + " ms)\n").c_str());
        }
        else
        {
            SDL_Delay(1);
        }
    }
}

void S2DGame::Quit()
{
    OnQuit();
    SDL_SetRelativeMouseMode(SDL_FALSE);
    S2DInput::ShowCursor(true);
    S2DInput::LockCursor(false);
    SDL_Quit();
    IMG_Quit();
    Mix_Quit();
    TTF_Quit();
    ExitProcess(0);
}

S2DGame::S2DGame(EngineInitSettings* settings)
{
    CurrentSettings = settings;
    std::string mutexName = ReplaceAll(std::string(settings->title), " ", "").c_str();
	MutexHandle = CreateMutexA(NULL, TRUE, ("S2DGame_" + mutexName).c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        S2DFatalError("An another application instance is already running!");
}

S2DGame::~S2DGame()
{
    ReleaseMutex(MutexHandle);
}