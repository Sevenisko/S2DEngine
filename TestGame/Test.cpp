#include <S2D_Misc.h>
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

class MyGame : public S2DGame
{
public:
    S2DFont font;
    S2DSprite doomguy;
    S2DSprite test;

    S2DAudioClip clip;

    S2DMusicClip music;

    S2DCamera cam;
    
    int sndVolume = 100, musVolume = 55;

    uintptr_t groundBox = -1, dynBox = -1;

    // Here goes all the initialization stuff
    void OnInit() override
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        cam.Position = Vec2(0, 0);

        ImGui::StyleColorsDark();

        /*S2DInput::ShowCursor(false);
        S2DInput::LockCursor(true);*/

        ImGui_ImplSDL2_InitForD3D(Graphics->GetWindow());
        ImGui_ImplDX9_Init(Graphics->GetDX9Device());

        font = S2DFont(Graphics->GetRenderer(), "Ubuntu.ttf");
        doomguy = S2DSprite(Graphics->LoadTextureRaw("doomguy.png"), { {41, 56}, {37, 56}, {38, 56}, {40, 55} }, 15);
        test = S2DSprite(Graphics->LoadTextureRaw("TestSprite.png"), { {128, 128} }, 15);

        groundBox = Physics->CreateStaticBox(Vec2(0, -8.0f), Vec2(8, 2), Vec2(0.5f, 0.5f), 0);

        dynBox = Physics->CreateDynamicBox(Vec2(0, 4.5f), Vec2(1, 1), Vec2(0.5f, 0.5f), 0);

        clip = S2DAudioClip("test2.wav");
        music = S2DMusicClip("de1m4.ogg");

        S2DAudio::SetMusicClip(&music, true);
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
        doomguy = S2DSprite(Graphics->LoadTextureRaw("doomguy.png"), { {41, 56}, {37, 56}, {38, 56}, {40, 55} }, 15);
    }

    // Here goes all the cleanup stuff
    void OnQuit() override
    {
        Physics->DeleteBox(groundBox);
        Physics->DeleteBox(dynBox);

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

    float curX1 = 0;
    float curY1 = 0;

    float curX2 = 0;
    float curY2 = 0;

    float curX3 = 0;
    float curY3 = 0;

    Vec2 physPos = Vec2();
    float physAngle;

    Color myColor = Color::White();

    bool cursorEnabled = true;

    // Here goes all the game update stuff
    void OnUpdate() override
    {
        if (!cursorEnabled)
        {
            curX3 += S2DInput::GetMouseDelta()->x / 32.0f;
            curY3 += S2DInput::GetMouseDelta()->y / 32.0f;

            /*curX3 = S2DInput::GetMousePosition()->x;
            curY3 = S2DInput::GetMousePosition()->y;*/

            if (S2DInput::GetMouseButtonDown(MouseButton::Left))
            {
                myColor = Color::Red();
            }
            else if (S2DInput::GetMouseButtonDown(MouseButton::Middle))
            {
                myColor = Color::Green();
            }
            else if (S2DInput::GetMouseButtonDown(MouseButton::Right))
            {
                myColor = Color::Blue();
            }

            if (S2DInput::GetKey(InputKey::W))
            {
                curY1 -= 0.25f;
            }
            if (S2DInput::GetKey(InputKey::S))
            {
                curY1 += 0.25f;
            }
            if (S2DInput::GetKey(InputKey::A))
            {
                curX1 -= 0.25f;
            }
            if (S2DInput::GetKey(InputKey::D))
            {
                curX1 += 0.25f;
            }

            if (S2DInput::GetKey(InputKey::Up))
            {
                curY2 += 0.25f;
            }
            if (S2DInput::GetKey(InputKey::Down))
            {
                curY2 -= 0.25f;
            }
            if (S2DInput::GetKey(InputKey::Left))
            {
                curX2 -= 0.25f;
            }
            if (S2DInput::GetKey(InputKey::Right))
            {
                curX2 += 0.25f;
            }
        }
        
        if (S2DInput::GetKeyDown(InputKey::Space))
        {
            S2DAudio::PlayAudioClip(&clip);
        }

        if (S2DInput::GetKeyDown(InputKey::Escape))
        {
            Quit();
        }

        if (S2DInput::GetKeyDown(InputKey::Tab))
        {
            S2DInput::LockCursor(!S2DInput::IsCursorLocked());
            cursorEnabled = !cursorEnabled;
        }

        cam.Position.x = curX2;
        cam.Position.y = curY2;

        physPos = Physics->GetBoxPos(dynBox);
        physAngle = Physics->GetBoxAngle(dynBox);

        time += GetDeltaTime();
        fpsTime += GetDeltaTime();
    }

    float time = 0;
    float fpsTime = 0;

    char frameRateText[256];

    char cursorText[256];

    char posText[256];
    char physText[256];

    // Here goes all the rendering stuff
    void OnRender() override
    {
        if (time >= 0.15f)
        {
            doomguy.NextFrame();
            time = 0;
        }

        Graphics->RenderSprite(&cam, &test, physPos, Vec2(0.5f, 0.5f), Vec2(16, 16), physAngle, TexFlipMode::None, Color::Red());
        Graphics->RenderFilledBox(&cam, Vec2(0, -8.0f), Vec2(0.5f, 0.5f), Vec2(128, 16), Color::Green());

        Graphics->RenderSprite(&cam ,&doomguy, Vec2(curX1, curY1), Vec2(0.5, 1), Vec2(76, 112), 0, TexFlipMode::None, Color::White());
        //Graphics->RenderFilledBox(Vec2(curX1, curY1), Vec2(64, 64), Color::Red());
        //Graphics->RenderFilledBox(&cam, Vec2(curX2, curY2), Vec2(2.5, 2.5), Vec2(64, 64), Color::Green());
        Graphics->RenderFilledBox(&cam, Vec2(curX3, curY3), Vec2(0.5, 0.5), Vec2(32, 32), myColor);

        if (fpsTime >= 0.075f)
        {
            sprintf(frameRateText, "%.0f fps (%.1f ms)", roundf(GetFrameRate()), GetFrameTime());
            fpsTime = 0;
        }

        sprintf(cursorText, "Cursor enabled: %s, Cursor position: [%d, %d]", cursorEnabled ? "Yes" : "No", S2DInput::GetMousePosition()->x, S2DInput::GetMousePosition()->y);
        sprintf(posText, "Player position: [%.2f, %.2f], Box position: [%.2f, %.2f], Camera position: [%.2f, %.2f]", curX1, curY1, curX3, curY3, curX2, curY2);

        sprintf(physText, "Physics position: [%.2f, %.2f], Physics angle: %.2f", physPos.x, physPos.y, physAngle);

        auto size = font.GetSize(20, cursorText);

        font.Render(20, frameRateText, Vec2(0, 0), Vec2(0, 0), 0, TexFlipMode::None, Color::White());
        font.Render(20, cursorText, Vec2(0, size.y + 6), Vec2(0, 0), 0, TexFlipMode::None, Color::White());
        font.Render(20, posText, Vec2(0, (size.y * 2) + 6), Vec2(0, 0), 0, TexFlipMode::None, Color::White());
        font.Render(20, physText, Vec2(0, (size.y * 3) + 6), Vec2(0, 0), 0, TexFlipMode::None, Color::White());

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplSDL2_NewFrame(Graphics->GetWindow());
        ImGui::NewFrame();

        if (ImGui::Begin("Audio Settings", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::SliderInt("SFX Volume", &sndVolume, 0, 100, "%d%%"))
                S2DAudio::SetAudioVolume(sndVolume);
            if (ImGui::SliderInt("Music Volume", &musVolume, 0, 100, "%d%%"))
                S2DAudio::SetMusicVolume(musVolume);
            
        }

        if (ImGui::Begin("About this Project", NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
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
        }

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    MyGame() : S2DGame(&settings) {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    settings = { "S2D Test Game", 0, new ScreenResolution {1600, 900}, true };
    MyGame* game = new MyGame();
    game->Run();
}