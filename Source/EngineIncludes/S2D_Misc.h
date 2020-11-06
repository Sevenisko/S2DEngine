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

    Purpose: Implementation of misc features of S2D Engine
\************************************************************/

#ifndef S2D_MISC_INCLUDED
#define S2D_MISC_INCLUDED

#include <Windows.h>
#include <ImageHlp.h>

#ifdef S2D_MAIN_INCLUDED
#define DllExport __declspec(dllexport)
#else
#define DllExport
#endif // S2D_MAIN_INCLUDED

#define S2DAssert(expression) if(!expression) { if(IsConsoleApp()) {printf("File: %s\nLine: %d\n\t %s", __FILE__, __LINE__, #expression); abort(); } else { char msg[128]; sprintf(msg, "File: %s\nLine: %d\n\t %s", __FILE__, __LINE__, #expression);  MessageBoxA(NULL, msg, "S2D Assert Failure", MB_OK | MB_ICONERROR); ExitProcess(1); } }

#define S2DFatalError(message) if(IsConsoleApp()) {printf("Fatal Error: %s", message); abort(); } else { MessageBoxA(NULL, message, "Fatal Error", MB_OK | MB_ICONERROR); ExitProcess(1); }

#define S2DFatalErrorFormatted(message, ...) if(IsConsoleApp()) {char msg[256]; sprintf(msg, message, ##__VA_ARGS__); printf("Fatal Error: %s", msg); abort(); } else { char msg[256]; sprintf(msg, message, ##__VA_ARGS__); MessageBoxA(NULL, msg, "Fatal Error", MB_OK | MB_ICONERROR); ExitProcess(1); }

#define S2DMsgBox(message, ...) \
char msg[256];\
sprintf(msg, message, ##__VA_ARGS__);\
MessageBoxA(NULL, msg, "S2D Game Engine", MB_OK | MB_ICONINFORMATION);

extern bool IsConsoleApp();

#endif