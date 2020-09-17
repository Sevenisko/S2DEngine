#include "EngineIncludes.h"

namespace Input
{
	bool mouseLocked = false;

	bool keys[SDL_NUM_SCANCODES];
	bool keysup[SDL_NUM_SCANCODES];
	bool keysdown[SDL_NUM_SCANCODES];

	bool mouseKeys[3]{ false, false, false };
	bool mouseKeysDown[3]{ false, false, false };
	bool mouseKeysUp[3]{ false, false, false };

	Vec2Int* MousePosition = new Vec2Int(0, 0);
	Vec2Int* MouseDelta = new Vec2Int(0, 0);

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