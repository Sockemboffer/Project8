#pragma once
#define GAME_NAME "Project8"
// divisible by 8, 16x16 tiles
#define GAME_RES_WIDTH 384
#define GAME_RES_HEIGHT 240
#define GAME_BPP		32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP/8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 100
#define TARGET_MICROSECONDS_PER_FRAME 16667

#pragma warning(disable: 4820) // warning about structure padding
#pragma warning(disable: 5045) // warning about spectors
// Structure needs to be a factor of Structure Size to be perfectly alligned in memory
typedef struct GAMEBITMAP {
	BITMAPINFO BitmapInfo; // 44 bytes
	// 4 bytes padding (warning issue to make factor of 8)
	void* Memory; // 8 bytes (4 on 32bit machine) // total 52 bytes (not a factor of 8)
} GAMEBITMAP;

typedef struct PIXEL32 {
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} PIXEL32;

typedef struct GAMEPERFDATA {
	uint64_t TotalFramesRendered;
	float RawFPSAverage;
	float CookedFPSAverage;
	int64_t PerfFrequency;
	MONITORINFO MonitorInfo;
	int32_t MonitorWidth;
	int32_t MonitorHeight;
} GAMEPERFDATA;

// Declarations
LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam);
DWORD CreateMainGameWindow(void); // _In_ helps compiler watch for consistency of usage
BOOL GameIsAlreadyRunning(void);
void ProcessPlayerInput(void);
void RenderFrameGraphics(void);