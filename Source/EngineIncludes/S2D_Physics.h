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

    Purpose: Implementation of box2D physics system
\************************************************************/

#ifndef S2D_PHYSICS_INCLUDED
#define S2D_PHYSICS_INCLUDED



class DllExport S2DPhysics
{
public:
    Vec2 Gravity = Vec2(0, -10.0f);

    S2DPhysics();
};

#endif // !S2D_PHYSICS_INCLUDED
