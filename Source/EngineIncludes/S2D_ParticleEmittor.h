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

    Purpose: Implementation of the Particle emittor
    Note: This code is NOT written by me! 
    Source: https://github.com/scarsty/SDL2-particles
\************************************************************/

#ifndef S2D_EMITTOR_INCLUDED
#define S2D_EMITTOR_INCLUDED

class ParticleEmittor : public ParticleSystem
{
public:
    ParticleEmittor() {}
    virtual ~ParticleEmittor() {}

    Vec2 Position;

    ParticleStyle style_ = ParticleStyle::None;
    void setStyle(ParticleStyle style);
    SDL_Texture* getDefaultTexture()
    {
        static SDL_Texture* t = IMG_LoadTexture(_renderer, "data\\particle.png");
        //printf(SDL_GetError());
        return t;
    }
};
#endif // !S2D_EMITTOR_INCLUDED