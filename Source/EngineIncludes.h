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

    Purpose: Implementation of all Engine includes into
             internal Engine code
\************************************************************/

#ifndef S2D_MAIN_INCLUDED
#define S2D_MAIN_INCLUDED
    #include "EngineVersion.h"
    #include "EngineIncludes/S2D_Misc.h"
    #include "EngineIncludes/S2D_Graphics.h"
    #include "EngineIncludes/S2D_Input.h"
    #include "EngineIncludes/S2D_Core.h"
    #include "EngineIncludes/S2D_Physics.h"
    #include "EngineIncludes/S2D_Audio.h"

    #include <SDL_mixer.h> 
    #include <SDL_image.h> 
    #include <SDL_ttf.h> 
    #include <SDL_net.h> 

    #include <box2d/box2d.h>

namespace Input
{
    extern void ProcessKey(SDL_Scancode key, bool state);
    extern void ProcessMouseButton(MouseButton button, bool state);
    extern void UpdateMousePos();
    extern void UpdateMousePos(int x, int y);
    extern void UpdateMouseDelta(int x, int y);

    extern bool mouseLocked;
}
#endif // !S2D_MAIN_INCLUDED