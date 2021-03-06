﻿#include <S2D_Misc.h>
#include <S2D_Graphics.h>
#include <S2D_Input.h>
#include <S2D_Physics.h>
#include <S2D_Audio.h>
#include <S2D_Core.h>
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_sdl.h"
#include "../ImGui/imgui_impl_dx9.h"
#include <string>
#include <ctgmath>

EngineInitSettings settings;

#define GetAbsolute(x) x < 0 ? -x : x

#define Clamp(x, min, max) if(x < min) x = min; else if (x > max) x = max;

float Lerp(float a, float b, float f)
{
    return (1 - f) * a + f * b;
}

float randomFloat(float a, float b)
{
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

struct FlappyBox
{
    float currentPos;
    float holePos;
    float holeSize;

    bool operator==(FlappyBox a) const
    {
        return (this->currentPos == a.currentPos && this->holePos == a.holePos && this->holeSize == a.holeSize);
    }
};

class MyGame : public S2DGame
{
public:
    S2DFont font;
    S2DSprite player;

    std::vector<FlappyBox*> pillars;

    S2DAudioClip jumpClip;
    S2DAudioClip hitClip;
    S2DAudioClip reachClip;

    S2DMusicClip music;

    S2DCamera cam;

    Vec2 playerPos = Vec2(0, 0);
    Vec2 playerVelocity = Vec2(0, 0);
    Vec2 playerVelocityLimit = Vec2(0, 0.5f);

    float desiredPlayerAngle = 0;

    float playerAngle = 0;

    float playerSpeed = 0.25f;

    bool playerAlive = true;
    
    int sndVolume = 100, musVolume = 55;

    // Here goes all the initialization stuff
    void OnInit() override
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        cam.Position = Vec2(0, 0);

        ImGui::StyleColorsDark();

        S2DInput::ShowCursor(false);
        S2DInput::LockCursor(true);

        ImGui_ImplSDL2_InitForD3D(Graphics->GetWindow());
        ImGui_ImplDX9_Init(Graphics->GetDX9Device());

        font = S2DFont(Graphics->GetRenderer(), "Ubuntu.ttf");

        player = S2DSprite(Graphics->LoadTextureRaw("data\\sprites\\player.png"), { {17, 18}, {17, 18}, {17, 18} }, 15);

        jumpClip = S2DAudioClip("data\\sounds\\Jump.wav");
        hitClip = S2DAudioClip("data\\sounds\\Hit.wav");
        reachClip = S2DAudioClip("data\\sounds\\Reached100.wav");

        S2DAudio::SetAudioVolume(sndVolume);
        S2DAudio::SetMusicVolume(musVolume);
    }

    // Here goes all the SDL Event stuff
    void OnSDLEvent(SDL_Event evnt) override
    {
        ImGui_ImplSDL2_ProcessEvent(&evnt);
    }

    // Here comes all the reload stuff
    void OnRenderReload() override
    {
        player = S2DSprite(Graphics->LoadTextureRaw("data\\sprites\\player.png"), { {17, 18}, {17, 18}, {17, 18} }, 15);
        font.UpdateRenderer(Graphics->GetRenderer());
    }

    // Here goes all the cleanup stuff
    void OnQuit() override
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }

    void OnFocusGained() override
    {
        ImGui_ImplDX9_Init(Graphics->GetDX9Device());
    }

    void OnFocusLost() override
    {
        ImGui_ImplDX9_Shutdown();
    }
    
    float pillarTimer = 0;

    // Here goes all the game update stuff
    void OnUpdate() override
    {
        pillarTimer += GetDeltaTime();

        if (pillarTimer >= 1.75f - (playerSpeed / 2) && playerAlive)
        {
            FlappyBox* box = new FlappyBox {12.5f, randomFloat(-4.5f, 4.25f), 3.35f};

            pillars.push_back(box);

            pillarTimer = 0;
        }

        if (playerAlive)
        {
            if (playerVelocity.y > -playerVelocityLimit.y)
                playerVelocity.y -= 0.05f;
        }
        else
        {
            if (playerVelocity.y > -playerVelocityLimit.y * 1.75f)
                playerVelocity.y -= 0.15f;
        }

        if (S2DInput::GetKeyDown(InputKey::Space) && playerAlive)
        {
            S2DAudio::PlayAudioClip(&jumpClip);
            playerVelocity.y = 0.5f;
        }

        if (S2DInput::GetKeyDown(InputKey::Tab))
        {
            S2DInput::LockCursor(!S2DInput::IsCursorLocked());
        }

        if (S2DInput::GetKeyDown(InputKey::R))
        {
            for (auto b : pillars)
            {
                delete b;
            }
            pillars.clear();

            player.SetFrame(0);
            playerAlive = true;
            playerSpeed = 0.25f;
            playerPos.y = 0;
            playerAngle = 0;
            playerVelocity.y = 0;
        }

        playerAngle = -(playerVelocity.y * 90.0f);

        if (S2DInput::GetKeyDown(InputKey::Escape))
        {
            Quit();
        }

        if (playerAlive)
        {
            for (auto b : pillars)
            {
                if (b->currentPos <= -12.5f)
                {
                    pillars.erase(std::remove(pillars.begin(), pillars.end(), b), pillars.end());
                    delete b;
                }
                else
                    b->currentPos -= 0.15f * playerSpeed;
            }
        }
        
        if (!isMovingPlayer)
        {
            playerPos.y -= playerVelocity.y;
            Clamp(playerPos.y, -7.5f, 7.5f);
        }
            
        if ((playerPos.y >= 7.5f || playerPos.y <= -7.5f) && playerAlive)
        {
            player.SetFrame(1);
            S2DAudio::PlayAudioClip(&hitClip);
            playerAlive = false;
        }

        time += GetDeltaTime();
        fpsTime += GetDeltaTime();

        if (playerAlive && playerSpeed < 2.15f)
            playerSpeed = Lerp(playerSpeed, playerSpeed + 0.005f, 1.25f * GetDeltaTime());
        else
            playerSpeed = 0;
    }

    float time = 0;
    float fpsTime = 0;

    char frameRateText[256];
    char playerMovingText[256];
    char pillarsText[256];

    char testBuildText[256];

    bool isMovingPlayer = false;

    // Here goes all the rendering stuff
    void OnRender() override
    {
        Graphics->RenderSprite(&cam, &player, playerPos, Vec2(0.5, 0.5), Vec2(64, 64), playerAngle, TexFlipMode::None, Color::White());

        for (auto b : pillars)
        {
            std::vector<FlappyBox*>::iterator it = std::find(pillars.begin(), pillars.end(), b);

            if (it != pillars.end())
                OutputDebugStringA(("#" + std::to_string(std::distance(pillars.begin(), it)) + " {" + std::to_string(b->currentPos) + "}\n").c_str());

            Graphics->RenderFilledBox(&cam, Vec2(b->currentPos, b->holePos), Vec2(0.5f, 1.0f), Vec2(150, 1024), Color::Green());
            Graphics->RenderFilledBox(&cam, Vec2(b->currentPos, b->holePos + b->holeSize), Vec2(0.5f, 0), Vec2(150, 1024), Color::Green());
        }

        if (fpsTime >= 0.075f)
        {
            sprintf(frameRateText, "%.0f fps (%.1f ms)", roundf(GetFrameRate()), GetFrameTime());
            fpsTime = 0;
        }

        sprintf(testBuildText, "This is a test project of Seven2D Game Engine");
        sprintf(playerMovingText, "Movement speed: %f", playerSpeed);
        sprintf(pillarsText, "Spawned pillars: %d", pillars.size());

        auto size = font.GetSize(20, frameRateText);

        auto textSize = font.GetSize(16, testBuildText);

        font.Render(20, frameRateText, Vec2(0, 0), Vec2(0, 0), 0, TexFlipMode::None, Color::White());
        font.Render(20, playerMovingText, Vec2(0, size.y + 6), Vec2(0, 0), 0, TexFlipMode::None, Color::White());
        font.Render(20, pillarsText, Vec2(0, size.y * 2 + 12), Vec2(0, 0), 0, TexFlipMode::None, Color::White());
        font.Render(16, testBuildText, Vec2(Graphics->GetCurrentWindowSize().x - textSize.x - 6, Graphics->GetCurrentWindowSize().y - textSize.y - 6), Vec2(0, 0), 0, TexFlipMode::None, Color::White());

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplSDL2_NewFrame(Graphics->GetWindow());
        ImGui::NewFrame();

        /*if (ImGui::Begin("Player", (bool*)0, ImGuiWindowFlags_AlwaysAutoResize))
        {  
            if (ImGui::IsWindowFocused())
            {
                isMovingPlayer = true;
            }
            else
            {
                isMovingPlayer = false;
            }

            std::vector<float*> pos = { &playerPos.x, &playerPos.y };

            std::vector<ImU32> colors = { 
                0xBB0000FF, // red
                0xBB00FF00, // green
                0xBBFF0000, // blue
                0xBBFFFFFF, // white for alpha?
            };

            ImGui::DragFloatN_Colored("Position", pos, colors, 0.01f, -15000.0f, 15000.0f, "%.3f", 1.0f);

            ImGui::End();
        }*/

        /*if (ImGui::Begin("About this Project", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Seven2D (S2D) Game Engine");
            ImGui::Text("Created by Sevenisko");
            ImGui::Separator();
            ImGui::Text("Version: %d.%d.%d\n", GetEngineVersion().minor, GetEngineVersion().minor, GetEngineVersion().build);
            ImGui::Text("Build date: %s", GetBuildDate());
            ImGui::Text("Build time: %s", GetBuildTime());
            ImGui::Separator();

            if (ImGui::TreeNode("Used 3rd-Party libraries"))
            {
                ImGui::Text("Actually, these links are clickable. :)");
                ImGui::NewLine();
                ImGui::Text("> SDL2");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://www.libsdl.org/", NULL, NULL, 0);
                }
                
                ImGui::Text("> SDL2_mixer");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://www.libsdl.org/projects/SDL_mixer/", NULL, NULL, 0);
                }

                ImGui::Text("> SDL2_ttf");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://www.libsdl.org/projects/SDL_ttf/", NULL, NULL, 0);
                }

                ImGui::Text("> SDL2_image");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://www.libsdl.org/projects/SDL_image/", NULL, NULL, 0);
                }

                ImGui::Text("> SDL2_net");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://www.libsdl.org/projects/SDL_net/", NULL, NULL, 0);
                }

                ImGui::Text("> Dear ImGui");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://github.com/ocornut/imgui", NULL, NULL, 0);
                }

                ImGui::Text("> box2d Physics Engine");
                if (ImGui::IsItemClicked())
                {
                    ShellExecuteA(NULL, "open", "https://box2d.org/", NULL, NULL, 0);
                }

                ImGui::TreePop();
            }
        }*/

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    MyGame() : S2DGame(&settings) {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    settings = { "Flappy Guy - S2D Game Test", 0, new ScreenResolution {1600, 900}, true };
    MyGame* game = new MyGame();
    game->Run(nullptr);
}