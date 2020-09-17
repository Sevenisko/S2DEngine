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

    Purpose: Implementation of input system of S2D
\************************************************************/

#ifndef S2D_INPUT_INCLUDED
#define S2D_INPUT_INCLUDED
enum class InputKey
{
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z, 

    Alpha1, Alpha2, Alpha3, Alpha4, Alpha5, Alpha6, Alpha7, Alpha8, Alpha9, Alpha0,
    Return, Escape, Backspace, Tab, Space, Minus, Equals, LeftBracket, RightBracked, Backslash,
    Semicolon = 51, Apostrophe, Grave, Comma, Period, Slash, 

    F1 = 58, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    PrintScreen, Pause = 72, Insert, Home, PageUp, Delete, End, PageDown, 
    Right, Left, Down, Up,

    NumDivide = 84, NumMultiply, NumMinus, NumPlus, NumEnter, 
    Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
    NumPeriod,

    LeftCtrl = 224, LeftShift, LeftAlt, RightCtrl = 228, RightShift, RightAlt
};

enum class MouseButton
{
    Left, Middle, Right
};

class DllExport S2DInput
{
public:
    static void ShowCursor(bool state);
    static bool IsCursorShown();
    static void LockCursor(bool state);
    static bool IsCursorLocked();
    static void SetMousePosition(int x, int y);
    static void SetMousePosition(Vec2Int pos);
    static Vec2Int* GetMousePosition();
    static Vec2Int* GetMouseDelta();
    static bool GetMouseButtonDown(MouseButton button);
    static bool GetMouseButtonUp(MouseButton button);
    static bool GetMouseButton(MouseButton button);
    static bool GetKeyDown(InputKey key);
    static bool GetKeyUp(InputKey key);
    static bool GetKey(InputKey key);
};
#endif