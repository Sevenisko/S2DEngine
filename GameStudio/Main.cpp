#include "GameStudio.h"

EngineInitSettings* settings;

const char* projectPath;

bool FileExists(const std::string& name) {
    if (FILE* file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}

void WriteUserSettings(EngineInitSettings* data)
{
    FILE* file = fopen("C:\\ProgramData\\Sevenisko\\S2D Game Studio\\settings.dat", "w+");
    fwrite((void*)data->fullscreen, sizeof(bool), 1, file);
    fwrite((void*)data->resolution, sizeof(ScreenResolution), 1, file);
    fwrite((void*)data->screenNum, sizeof(int), 1, file);
    fwrite((void*)projectPath, sizeof(const char), strlen(projectPath), file);
    fclose(file);
}

EngineInitSettings* ReadUserSettings()
{
    if (FileExists("C:\\ProgramData\\Sevenisko\\S2D Game Studio\\settings.dat"))
    {
        FILE* file = fopen("C:\\ProgramData\\Sevenisko\\S2D Game Studio\\settings.dat", "r");
        EngineInitSettings data;
        data.title = "S2D Game Studio";
        fread(&data.fullscreen, sizeof(bool), 1, file);
        fread(data.resolution, sizeof(ScreenResolution), 1, file);
        fread(&data.screenNum, sizeof(int), 1, file);
        fread(&projectPath, sizeof(const char), strlen(projectPath), file);
        fclose(file);

        return &data;
    }
    else
    {
        return new EngineInitSettings { "S2D Game Studio", 0, new ScreenResolution {1600, 900}, false };
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    settings = ReadUserSettings();

    GameStudioApp app = GameStudioApp();

    GameSplashScreen screen = {"editor\\splashscreen.png", 5500};

    app.Run(&screen);
}