#include "EngineIncludes.h"
#include <shlwapi.h>

int musicVolume = 100;
int soundVolume = 100;

S2DMusicClip* currentMusic = nullptr;

S2DMusicClip::S2DMusicClip(const char* path)
{
    this->path = path;
}

S2DAudioClip::S2DAudioClip(const char* path)
{
    sndFile = Mix_LoadWAV(path);
}

void S2DAudio::PlayAudioClip(S2DAudioClip* clip)
{
    Mix_VolumeChunk(clip->GetHandle(), (int)(clip->VolumeMultiplier * 128));
    clip->PlayedChannelID = Mix_PlayChannel(-1, clip->GetHandle(), 0);
}

void S2DAudio::SetMusicClip(S2DMusicClip* clip, bool loop)
{
    if (currentMusic != nullptr)
    {
        Mix_FreeMusic(currentMusic->GetHandle());
        currentMusic->SetHandle(NULL);
    }

    if (clip == NULL)
    {
        if (currentMusic != nullptr)
            Mix_HaltMusic();

        currentMusic = nullptr;
    }
    else
    {
        clip->SetHandle(Mix_LoadMUS(clip->GetPath()));

        Mix_PlayMusic(clip->GetHandle(), loop ? -1 : 0);

        currentMusic = clip;
    }
}

void S2DAudio::SetMidiSoundFont(const char* path)
{
    Mix_SetSoundFonts(path);
}

void S2DAudio::SetAudioVolume(int volume)
{
    int vol = (int)(1.28 * (float)volume);

    // Basic Clamp
    if (vol > 128) vol = 128;
    else if (vol < 0) vol = 0;

    Mix_Volume(-1, vol);
}

void S2DAudio::SetAudioPos(S2DAudioClip* clip, SoundSide side, int distance)
{
    Mix_SetPosition(clip->PlayedChannelID, (Sint16)side, distance);
}

void S2DAudio::SetMusicVolume(int volume)
{
    int vol = (int)(1.28 * (float)volume);

    // Basic Clamp
    if (vol > 128) vol = 128;
    else if (vol < 0) vol = 0;

    Mix_VolumeMusic(vol);


}