# Seven2D (S2D) Game Engine
A Open-Source 2D Game Engine made entirely for 2D Game Development.

## 3rd-Party Libraries
* [SDL2](https://www.libsdl.org/)
* [SDL2_mixer](https://www.libsdl.org/projects/SDL_mixer/)
* [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/)
* [SDL2_image](https://www.libsdl.org/projects/SDL_image/)

## Engine Features
- [x] Core Engine API
	- [x] Base Engine Class (S2DGame)
	- [x] Engine Loop handled by library
	- [x] Fully functional Engine Structure   
- [x] Graphics Subsystem
	- [x] Creation of Window and Renderer
	- [x] 2D World Matrix
	- [x] Implementation of Rendering Features
		- [x] Rendering Primitives (Rect, Filled Rect, Line, Point)
		- [x] Sprite Renderer
		- [x] Font Renderer (Text Renderer)
		- [ ] Post-Processing Features
	- [x] Make the code crash-free    
- [x] Input Subsystem
	- [x] Keyboard support
	- [x] Mouse support
	- [X] Controller (Joystick) support 
- [x] Sound Subsystem
	- [x] Ability to play music
	- [x] Ability to play sound files
	- [x] Ability to set Music and Sound volume (MIDI volume control currently unavailable)
- [ ] Physics Subsystem 
  - **You need to implement physics by yourself at the moment...**

## How to use?
There are three ways to use S2D Engine:
1. Install the GameStudio (currently not available)
2. Install the Engine Library
3. Download the Source Code (and Build)

### 1. Install the GameStudio
This way is already unavailable, i need to finish the core first!

### 2. Install the Engine Library
In order to successfully compile your game with S2D Game SDK, you need to also implemement these **3rd-Party** libraries
1. Go to the website [here](https://seven2d.eu/downloads), and download the **S2D Game SDK** (this package contains only the Engine library).
2. Create a C++ project using Visual Studio (+ setup the project)
3. Add the dependencies (Includes and .lib files)
4. Write a code (you can take the Example code from below)
5. Compile and you're done

### 3. Download the Source Code (and Build)
1. Clone this repository with this command: `git clone https://github.com/Sevenisko/S2DEngine`
2. Open the solution file *Seven2DEngine.sln*
3. Pick the platform and configuration
4. Build, and you're done!

## Example Code
```cpp
#include <S2D_Misc.h>
#include <S2D_Graphics.h>
#include <S2D_Core.h>

class MyGame : public S2DGame
{
public:
	// Initialization stuff
	void OnInit() override
	{
	
	}
	
	// Stuff used for cleanup
	void OnQuit() override
	{
	
	}
	
	// This function is called every frame before rendering
	void OnUpdate() override
	{
	
	}
	
	// This function is called every frame, and is used for rendering
	void OnRender() override
	{
	
	}
};

/*
// Console version (Console Subsystem)
int main(int argc, char** argv)
{
	settings = { "S2D Test Game", 0, new ScreenResolution {1600, 900}, true };
    MyGame* game = new MyGame();
    game->Run();
}
*/

// Win32 version (Windows Subsystem)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    settings = { "S2D Test Game", 0, new ScreenResolution {1600, 900}, true };
    MyGame* game = new MyGame();
    game->Run();
}

```

## Contributing into this project
You can **freely** contribute into this project (adding new platforms, etc) with *Pull requests*, you can clone this repo and create your *extended* version under your *own branding*, but please, do **NOT** create *re-branded* exact copies of this engine.