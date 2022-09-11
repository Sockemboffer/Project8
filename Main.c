#include <stdio.h>
#pragma warning( push, 0 )
#include <windows.h>
#pragma warning( pop )
#include <stdint.h>
#include "Main.h" // My files come after windows.h (has important definitions that our .h needs)

HWND gGameWindow; // NULL or 0 by default
BOOL gGameIsRunning; // global vars are automaticaly initialized to 0, no need to initialize unless you want somthing other than 0
GAMEBITMAP gBackBuffer;
GAMEPERFDATA gPerformanceData;
// Windows is native Unicode OS
// L is Unicode string instead of ASCII (also called multi-byte sometimes)
// VS tip: C/C++ doesn't show up in properties until you have atleast one file of that type
// %i type things are token or format specifier
// handle is a memory address
// Use Ascii version of functions rather than it needing to go through define expansion
// basic things you need: Window class, Window, Window process, message loop
// Game is single threaded, thread can be scheduled across multiple cpus
// our game is  100% while looping takng up a single cpu going as fast as it can

int __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CommandShow) // Input params from the OS
{
    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(CommandLine);
    UNREFERENCED_PARAMETER(CommandShow);

    if (GameIsAlreadyRunning() == TRUE) {
        MessageBoxA(NULL, "Another instance of the game is already running.", "Error.", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    if (CreateMainGameWindow() != ERROR_SUCCESS) {
        goto Exit;
    }

    // Only needs to be called once
    QueryPerformanceFrequency(&gPerformanceData.PerfFrequency);

    gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);
    gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
    gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
    gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
    gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
    gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;
    // Allocating larger than 64Kb of data, use VirtualAlloc (Use HeapAlloc if only need like 2 or 3 bytes)
    // VirtualAlloc returns NULL if fails, need to handle
    if ((gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)) == NULL) {
        MessageBoxA(NULL, "Failed to allocate memory for drawing surface.", "Error.", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    MSG Message = { 0 };
    gGameIsRunning = TRUE;
    while (gGameIsRunning == TRUE) 
    {
        QueryPerformanceCounter(&gPerformanceData.FrameStart);

        while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) // GetMessageA is a blocking call, not PeekMessageA
        {
            DispatchMessageA(&Message);
        }

        ProcessPlayerInput();
        RenderFrameGraphics();
        QueryPerformanceCounter(&gPerformanceData.FrameEnd);
        gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart = gPerformanceData.FrameEnd.QuadPart - gPerformanceData.FrameStart.QuadPart;
        gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart *= 1000000;
        gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart /= gPerformanceData.PerfFrequency.QuadPart;
        Sleep(1); // will revisit and tune

        gPerformanceData.TotalFramesRendered++;
        if (gPerformanceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES == 0)
        {
            char str[64] = { 0 };
            _snprintf_s(str, _countof(str), _TRUNCATE, "Elapsed microseconds: %lli\n", gPerformanceData.ElapsedMicrosecondsPerFrame.QuadPart);
            OutputDebugStringA(str);
        }
    }
Exit:
	return 0;
}

// Handles all Windows messages
LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
    LRESULT Result = 0;
    switch (Message)
    {
        case WM_CLOSE:
        {
            gGameIsRunning = FALSE;
            PostQuitMessage(0);
            break;
        }
        default:
        {
            Result = DefWindowProcA(WindowHandle, Message, WParam, LParam);
        }
    }
    return Result;
}

DWORD CreateMainGameWindow(void)
{
    DWORD Result = ERROR_SUCCESS;
    // Everything in windows is a window when progrmming in Win32, buttons, dropdowns, etc
    WNDCLASSEXA WindowClass = { 0 }; // braces to initialize data structures
    // very common in windows data structures that the first thing is the size of that data structure

    WindowClass.cbSize = sizeof(WNDCLASSEXA); // 'cb' is counts in bytes of data structure
    WindowClass.style = 0;
    WindowClass.lpfnWndProc = MainWindowProc; // long pointer to a function
    WindowClass.cbClsExtra = 0;
    WindowClass.hInstance = GetModuleHandleA(NULL); // Assumes module? is within program and not another dll
    WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION); // IDI_APPLICATION built-in default icon
    WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255)); // color of window
    WindowClass.lpszMenuName = NULL;
    WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

    // Manifest is an XML document that is embedded in with the program
    //SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (RegisterClassExA(&WindowClass) == 0) {// RegisterClassExA designed to return 0 if fails
        Result = GetLastError();
        MessageBoxA(NULL, "Window registation failed.", "Error.", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gGameWindow = CreateWindowExA(0, WindowClass.lpszClassName, "Title",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480, NULL, NULL, WindowClass.hInstance, NULL);
    if (gGameWindow == NULL) {
        Result = GetLastError();
        MessageBoxA(NULL, "Window creation failed.", "Error.", MB_ICONEXCLAMATION | MB_OK);
        goto Exit;
    }

    gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

    // Returns 0 if fails, however if MS docs say there is no "GetLastError" then we won't know why it failed
    if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == 0) {
        Result = ERROR_MONITOR_NO_DESCRIPTOR; // choose error that seems like it may fit
        goto Exit;
    };

    gPerformanceData.MonitorWidth = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;
    gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

    if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0) {
        Result = GetLastError();
        goto Exit;
    };
    
    if (SetWindowPos(gGameWindow, HWND_TOP, gPerformanceData.MonitorInfo.rcMonitor.left, gPerformanceData.MonitorInfo.rcMonitor.top,
        gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight, SWP_FRAMECHANGED) == 0) {
        Result = GetLastError();
        goto Exit;
    };
Exit:
    return Result;
}
BOOL GameIsAlreadyRunning(void) 
{
    HANDLE Mutex = NULL; // Piece of memory used to gate access to resources (prevent multiple threads from writing to memory)
    // Once "I'm" done with mutex then you can have it
    // I have the seashell, so I can speak. When you have it, then you can speak.
    // Short for Mutual Exclusion (Mutex)
    Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return TRUE;
    } 
    else {
        return FALSE;
    }
}
void ProcessPlayerInput(void) 
{
    int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
    if (EscapeKeyIsDown)
    {
        SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
    }
}

void RenderFrameGraphics(void) 
{
    // Whenever you get a device context, always remember to release when finished
    HDC DeviceContext = GetDC(gGameWindow);
    // DI = device independence
    StretchDIBits(DeviceContext, 0, 0,
        gPerformanceData.MonitorWidth, gPerformanceData.MonitorHeight,
        0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT, gBackBuffer.Memory, 
        &gBackBuffer.BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(gGameWindow, DeviceContext);
}