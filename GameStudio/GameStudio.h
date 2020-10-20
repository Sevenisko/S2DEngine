#ifndef S2D_GAMESTUDIO_INCLUDED
#define S2D_GAMESTUDIO_INCLUDED

#include <S2D_Misc.h>
#include <S2D_Graphics.h>
#include <S2D_Input.h>
#include <S2D_Physics.h>
#include <S2D_Audio.h>
#include <S2D_Core.h>

#include "FontAwesome.h"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_sdl.h"
#include "../ImGui/imgui_impl_dx9.h"

extern EngineInitSettings* settings;

extern const char* projectPath;

struct GameStudioProject
{
	const char* Name;
	const char* Author;
	time_t lastModifiedTime;
	VersionInfo Version;
};

class GameStudioApp : public S2DGame
{
public:
	GameStudioApp() : S2DGame(settings) {}

	void OnRender() override;
	void OnUpdate() override;
	void OnFocusLost() override;
	void OnFocusGained() override;
	void OnQuit() override;
	void OnRenderReload() override;
	void OnSDLEvent(SDL_Event e) override;
	void OnInit() override;
};

#endif // !S2D_GAMESTUDIO_INCLUDED
