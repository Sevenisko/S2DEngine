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

	Purpose: Input subsystem for S2D Engine
\************************************************************/

#include "EngineIncludes.h"

struct Joystick
{
	std::vector<bool> buttons;
	std::vector<bool> buttonsup;
	std::vector<bool> buttonsdown;
	std::vector<float> axis;
	std::vector<Uint8> hats;
};

namespace Input
{
	bool mouseLocked = false;

	bool keys[SDL_NUM_SCANCODES];
	bool keysup[SDL_NUM_SCANCODES];
	bool keysdown[SDL_NUM_SCANCODES];

	std::vector<Joystick*> joy_values;
	
	std::vector<SDL_Joystick*> joysticks;

	bool mouseKeys[3]{ false, false, false };
	bool mouseKeysDown[3]{ false, false, false };
	bool mouseKeysUp[3]{ false, false, false };

	Vec2Int* MousePosition = new Vec2Int(0, 0);
	Vec2Int* MouseDelta = new Vec2Int(0, 0);

	int NumJoysticks()
	{
		return SDL_NumJoysticks();
	}

	int NumButtons(int index)
	{
		return SDL_JoystickNumButtons(joysticks[index]);
	}

	int NumAxes(int index)
	{
		return SDL_JoystickNumAxes(joysticks[index]);
	}

	int NumHats(int index)
	{
		return SDL_JoystickNumHats(joysticks[index]);
	}

	bool OpenJoystick(int index)
	{
		SDL_Joystick* joy = SDL_JoystickOpen(index);
		if (!joy) return false;
		else
		{
			Joystick* values = new Joystick;
			for (int i = 0; i < NumAxes(index); i++)
			{
				values->axis.push_back(0);
			}

			for (int i = 0; i < NumButtons(index); i++)
			{
				values->buttons.push_back(0);
				values->buttonsdown.push_back(0);
				values->buttonsup.push_back(0);
			}

			for (int i = 0; i < NumHats(index); i++)
			{
				values->hats.push_back(0);
			}
			joysticks.push_back(joy);
			joy_values.push_back(values);
		}
	}

	void CloseJoystick(SDL_Joystick* joystick)
	{
		SDL_JoystickClose(joystick);

		std::vector<SDL_Joystick*>::iterator it = std::find(joysticks.begin(), joysticks.end(), joystick);

		int index = -1;

		if (it != joysticks.end())
			index = std::distance(joysticks.begin(), it);

		delete joy_values[index];
		joy_values.erase(joy_values.begin() + index);

		joysticks.erase(std::remove(joysticks.begin(), joysticks.end(), joystick), joysticks.end());
	}

	void ProcessJoystickAxis(int index, int axis, float value)
	{
		float a = 0;
		if (value > 1.0F)
			a = 1.0F;
		else if (value < -1.0F)
			a = -1.0F;
		else
			a = value;

		joy_values[index]->axis[axis] = a;
	}

	void ProcessJoystickHat(int index, int hat, int value)
	{
		joy_values[index]->hats[hat] = value;
	}

	void ProcessJoystickButton(int index, int button, bool state)
	{
		joy_values[index]->buttons[button] = state;

		if (state)
		{
			joy_values[index]->buttonsdown[button] = true;
			joy_values[index]->buttonsup[button] = false;
		}
		else
		{
			joy_values[index]->buttonsdown[button] = false;
			joy_values[index]->buttonsup[button] = true;
		}
	}

	void ProcessMouseButton(MouseButton button, bool state)
	{
		mouseKeys[(int)button] = state;

		if (state)
		{
			mouseKeysDown[(int)button] = true;
			mouseKeysUp[(int)button] = false;
		}
		else
		{
			mouseKeysUp[(int)button] = true;
			mouseKeysDown[(int)button] = false;
		}
	}

	void UpdateMouseDelta(int x, int y)
	{
		MouseDelta->x = x;
		MouseDelta->y = y;
	}

	void UpdateMousePos()
	{
		SDL_GetMouseState(&MousePosition->x, &MousePosition->y);
	}

	void UpdateMousePos(int x, int y)
	{
		MousePosition->x = x;
		MousePosition->y = y;
	}

	void ProcessKey(SDL_Scancode key, bool state)
	{
		keys[(SDL_Scancode)key] = state;

		if (state)
		{
			keysdown[key] = true;
			keysup[key] = false;
		}
		else
		{
			keysup[key] = true;
			keysdown[key] = false;
		}
	}
}

bool S2DInput::IsCursorShown()
{
	int query = SDL_ShowCursor(SDL_QUERY);

	if (query)
		return true;
	else return false;
}

bool S2DInput::IsCursorLocked()
{
	return Input::mouseLocked;
}

void S2DInput::ShowCursor(bool state)
{
	int lock = 0;

	if (state)
	{
		lock = SDL_ENABLE;
	}
	else
	{
		lock = SDL_DISABLE;
	}

	SDL_ShowCursor(lock);
}

void S2DInput::LockCursor(bool state)
{
	if (state)
	{
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}

	Input::mouseLocked = state;
}

void S2DInput::SetMousePosition(int x, int y)
{
	SDL_WarpMouseGlobal(x, y);
}

void S2DInput::SetMousePosition(Vec2Int pos)
{
	SDL_WarpMouseGlobal(pos.x, pos.y);
}

Vec2Int* S2DInput::GetMousePosition()
{
	return Input::MousePosition;
}

Vec2Int* S2DInput::GetMouseDelta()
{
	return Input::MouseDelta;
}

float S2DInput::GetJoystickAxis(int index, int axis)
{
	return Input::joy_values[index]->axis[axis];
}

bool S2DInput::GetJoystickButtonDown(int index, int button)
{
	bool isDown = Input::joy_values[index]->buttonsdown[button];

	if (isDown) Input::joy_values[index]->buttonsdown[button] = false;

	return isDown;
}

bool S2DInput::GetJoystickButton(int index, int button)
{
	return Input::joy_values[index]->buttons[button];
}

bool S2DInput::GetJoystickButtonUp(int index, int button)
{
	bool isUp = Input::joy_values[index]->buttonsup[button];

	if (isUp) Input::joy_values[index]->buttonsup[button] = false;

	return isUp;
}

bool S2DInput::GetMouseButtonDown(MouseButton button)
{
	bool isDown = Input::mouseKeysDown[(int)button];

	if (isDown) Input::mouseKeysDown[(int)button] = false;

	return isDown;
}

bool S2DInput::GetMouseButton(MouseButton button)
{
	return Input::mouseKeys[(int)button];
}

bool S2DInput::GetMouseButtonUp(MouseButton button)
{
	bool isUp = Input::mouseKeysUp[(int)button];

	if (isUp) Input::mouseKeysUp[(int)button] = false;

	return isUp;
}

bool S2DInput::GetKeyDown(InputKey key)
{
	bool isDown = Input::keysdown[(SDL_Scancode)key];

	if (isDown) Input::keysdown[(SDL_Scancode)key] = false;

	return isDown;
}

bool S2DInput::GetKey(InputKey key)
{
	return Input::keys[(SDL_Scancode)key];
}

bool S2DInput::GetKeyUp(InputKey key)
{
	bool isUp = Input::keysup[(SDL_Scancode)key];

	if (isUp) Input::keysup[(SDL_Scancode)key] = false;

	return isUp;
};