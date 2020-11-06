#include <S2D_Misc.h>
#include <S2D_Graphics.h>
#include <S2D_Input.h>
#include <S2D_Physics.h>
#include <S2D_Audio.h>
#include <S2D_Core.h>
/*#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_sdl.h"
#include "../ImGui/imgui_impl_dx9.h"*/
#include <string>
#include <ctgmath>
#include <time.h>

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

    IDirect3DTexture9* dxTex;

    S2DTexture* tex;

    S2DAudioClip jumpClip;
    S2DAudioClip hitClip;
    S2DAudioClip reachClip;

    S2DMusicClip music;

    S2DCamera* cam;

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
        /*IMGUI_CHECKVERSION();
        ImGui::CreateContext();*/

        cam = new S2DCamera();

        cam->Position = Vec2(0, 0);

        //ImGui::StyleS2DColorsDark();

        S2DInput::ShowCursor(false);
        S2DInput::LockCursor(true);

        /*ImGui_ImplSDL2_InitForD3D(Graphics->GetWindow());
        ImGui_ImplDX9_Init(Graphics->GetDX9Device());*/

        font = S2DFont("Arial", 16, false);

        player = S2DSprite(Graphics->LoadTexture("data\\sprites\\player.png"), { {17, 18}, {17, 18}, {17, 18} }, 15);

        tex = Graphics->LoadTexture("data\\testscreen.png");

        jumpClip = S2DAudioClip("data\\sounds\\Jump.wav");
        hitClip = S2DAudioClip("data\\sounds\\Hit.wav");
        reachClip = S2DAudioClip("data\\sounds\\Reached100.wav");

        S2DAudio::SetAudioVolume(sndVolume);
        S2DAudio::SetMusicVolume(musVolume);

        Graphics->SetCamera(cam);
    }

    // Here goes all the SDL Event stuff
    void OnSDLEvent(SDL_Event evnt) override
    {
        //ImGui_ImplSDL2_ProcessEvent(&evnt);
    }

    // Here goes all the cleanup stuff
    void OnQuit() override
    {
        /*ImGui_ImplDX9_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();*/
    }

    void OnFocusGained() override
    {
        //ImGui_ImplDX9_Init(Graphics->GetDX9Device());
    }

    void OnFocusLost() override
    {
        //ImGui_ImplDX9_Shutdown();
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

        /*if (playerAlive)
        {
            if (playerVelocity.y > -playerVelocityLimit.y)
                playerVelocity.y -= 0.05f;
        }
        else
        {
            if (playerVelocity.y > -playerVelocityLimit.y * 1.75f)
                playerVelocity.y -= 0.15f;
        }*/

        if (S2DInput::GetKeyDown(InputKey::Space) && playerAlive)
        {
            S2DAudio::PlayAudioClip(&jumpClip);
            playerVelocity.y = 0.5f;
        }

        if (S2DInput::GetKeyDown(InputKey::F12))
        {
            Graphics->Screenshot("TestGame");
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

        gtime += GetDeltaTime();
        fpsTime += GetDeltaTime();

        if (playerAlive && playerSpeed < 2.15f)
            playerSpeed = Lerp(playerSpeed, playerSpeed + 0.005f, 1.25f * GetDeltaTime());
        else
            playerSpeed = 0;
    }

    float gtime = 0;
    float fpsTime = 0;

    char frameRateText[256];
    char playerMovingText[256];
    char pillarsText[256];

    char testBuildText[256];

    bool isMovingPlayer = false;

    // Here goes all the rendering stuff
    void OnRender() override
    {
        Graphics->RenderSprite(&player, playerPos, Vec2(0.5, 0.5), Vec2(64, 64), playerAngle, TexFlipMode::None, S2DColor::White(), nullptr);

        for (auto b : pillars)
        {
            std::vector<FlappyBox*>::iterator it = std::find(pillars.begin(), pillars.end(), b);

            if (it != pillars.end())
                OutputDebugStringA(("#" + std::to_string(std::distance(pillars.begin(), it)) + " {" + std::to_string(b->currentPos) + "}\n").c_str());

            Graphics->RenderFilledBox(Vec2(b->currentPos, b->holePos), Vec2(0.5f, 1.0f), Vec2(150, 1024), S2DColor::Green());
            Graphics->RenderFilledBox(Vec2(b->currentPos, b->holePos + b->holeSize), Vec2(0.5f, 0), Vec2(150, 1024), S2DColor::Green());
        }

        if (fpsTime >= 0.075f)
        {
            sprintf(frameRateText, "%.0f fps (%.1f ms)", roundf(GetFrameRate()), GetFrameTime());
            fpsTime = 0;
        }

        sprintf(testBuildText, "This is a test project of Seven2D Game Engine");
        sprintf(playerMovingText, "Movement speed: %f", playerSpeed);
        sprintf(pillarsText, "Spawned pillars: %d", pillars.size());

        auto size = font.GetSize(frameRateText);

        auto textSize = font.GetSize(testBuildText);

        font.Render(20, frameRateText, Vec2(0, 0), Vec2(0, 0), 0, S2DColor::White());
        font.Render(20, playerMovingText, Vec2(0, size.y + 6), Vec2(0, 0), 0, S2DColor::White());
        font.Render(20, pillarsText, Vec2(0, size.y * 2 + 12), Vec2(0, 0), 0, S2DColor::White());
        font.Render(16, testBuildText, Vec2(Graphics->GetCurrentWindowSize().x - textSize.x - 6, Graphics->GetCurrentWindowSize().y - textSize.y - 6), Vec2(0, 0), 0, S2DColor::White());
    }

    void OnPostRender(S2DTexture* frame) override
    {
        Graphics->RenderScreenTexture(frame, Vec2Int(0, 0), Vec2Int(Graphics->GetCurrentResolution()->width, Graphics->GetCurrentResolution()->height), S2DColor::Red(), nullptr);
    }

    MyGame() : S2DGame(&settings) {}
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    settings = { "Flappy Guy - S2D Game Test", new ScreenResolution {1600, 900}, false, true };
    MyGame* game = new MyGame();
    game->Run(nullptr);
}