#pragma once
#define GAME_NAME "Project8"
// divisible by 8, 16x16 tiles
#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP		32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP/8))

typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;
	void* Memory;
} GAMEBITMAP;

// Declarations
LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);
DWORD CreateMainGameWindow(void); // _In_ helps compiler watch for consistency of usage
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);