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

    Purpose: Implementation of audio system into S2D Engine
\************************************************************/

#ifndef S2D_AUDIO_INCLUDED
#define S2D_AUDIO_INCLUDED

enum class SoundSide
{
    Front = 0,
    Right = 90,
    Behind = 180,
    Left = 270
};

#ifndef S2D_MAIN_INCLUDED
typedef struct Mix_Music Mix_Music;
typedef struct Mix_Chunk Mix_Chunk;
#endif // !S2D_MAIN_INCLUDED

class DllExport S2DMusicClip
{
public:
    S2DMusicClip() {};
    S2DMusicClip(const char* path);

    const char* GetPath() { return path; }

    Mix_Music* GetHandle() { return musFile; }

    void SetHandle(Mix_Music* handle) { musFile = handle; }

    float VolumeMultiplier = 1.0f;
private:
    const char* path;
    Mix_Music* musFile;
};

class DllExport S2DAudioClip
{
public:
    S2DAudioClip() {};
    S2DAudioClip(const char* path);

    SoundSide side = SoundSide::Front;

    int PlayedChannelID;

    const char* GetPath() { return path; }

    Mix_Chunk* GetHandle() { return sndFile; }

    float VolumeMultiplier = 1.0f;
private:
    const char* path;
    Mix_Chunk* sndFile;
};

class DllExport S2DAudio
{
public:
    static void PlayAudioClip(S2DAudioClip* clip);
    static void SetMusicClip(S2DMusicClip* clip, bool loop);
    static void SetAudioPos(S2DAudioClip* clip, SoundSide side, int distance);
    static void SetAudioVolume(int volume);
    static void SetMusicVolume(int volume);
    static void SetMidiSoundFont(const char* path);
};

#endif // !S2D_AUDIO_INCLUDED
