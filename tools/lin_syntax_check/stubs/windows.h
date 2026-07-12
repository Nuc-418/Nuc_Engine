// Minimal Win32 stub so Windows-only translation units can be syntax-checked
// on Linux with g++ -fsyntax-only. Never used by the real (MSVC) build.
#pragma once
typedef struct { long x, y; } POINT;
typedef void* HWND;
int GetCursorPos(POINT*);
int SetCursorPos(int, int);
int SetWindowPos(HWND, int, int, int, int, int, unsigned);
HWND GetConsoleWindow();
int ShowCursor(int);
#define SWP_NOSIZE 1
