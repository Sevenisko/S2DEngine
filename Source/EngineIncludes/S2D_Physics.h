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

    Purpose: Implementation of box2D Physics Engine
\************************************************************/

#ifndef S2D_PHYSICS_INCLUDED
#define S2D_PHYSICS_INCLUDED

#define S2D_DEGREES_TO_RADIANS(a) (a * M_PI) / 180
#define S2D_RADIANS_TO_DEGREES(a) (a * 180) / M_PI

class DllExport S2DPhysics
{
public:
    S2DPhysics();

    S2DPhysics(Vec2 gravity);

    ~S2DPhysics();

    void Update(float timeStep);

    uintptr_t CreateStaticBox(Vec2 position, Vec2 size, Vec2 center, float angle);
    uintptr_t CreateDynamicBox(Vec2 position, Vec2 size, Vec2 center, float angle);

    Vec2 GetBoxPos(uintptr_t pointer);
    float GetBoxAngle(uintptr_t pointer);

    void DeleteBox(uintptr_t pointer);

private:
    Vec2 Gravity = Vec2(0, -10.0f);

    int VelocityIterations = 6;
    int PositionIterations = 2;
};

#endif // !S2D_PHYSICS_INCLUDED
