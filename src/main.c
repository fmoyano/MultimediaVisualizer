#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <wingdi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

static HWND window = 0;
static BITMAPINFO bmInfo = {0};
static void *bitmap_memory;
static HBITMAP bitmap_handle;
static HDC device_context;

static bool running = true;

typedef struct WindowSize
{
	int width;
	int height;
} WindowSize;

typedef unsigned char byte;

//Allocate a back buffer
static void Create_Init_Backbuffer(int width, int height)
{	
	if (bitmap_memory)
	{
		VirtualFree(bitmap_memory, 0, MEM_RELEASE);
	}

	BITMAPINFOHEADER bmHeader = {0};
	bmHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmHeader.biWidth = width;
	bmHeader.biHeight = -height; //top-down bitmap (origin is on the top-left corner)
	bmHeader.biPlanes = 1;
	bmHeader.biBitCount = 32;
	bmHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader = bmHeader;

	unsigned bytes_per_pixel = 4;
	size_t memory_size = bytes_per_pixel * width * height;
	bitmap_memory = VirtualAlloc(0, memory_size, MEM_COMMIT, PAGE_READWRITE);

	byte *row = (byte *)bitmap_memory;
	int pitch = bytes_per_pixel * width;
	for (int y = 0; y < height; ++y)
	{
		uint32_t* pixel = (uint32_t*) row;
		for (int x = 0; x < width; ++x)
		{
			//aRGB in little endian machine so 0x000000FF is Blue
			*pixel++ = 0x000000FF;
			//*pixel++ = ((x % 255) << 24);
			//*pixel++ = 0x00FF00FF;
		}

		row += pitch;
	}
}

static WindowSize GetWindowSize()
{
	WindowSize wSize = {0};

	RECT clientRect;
	GetClientRect(window, &clientRect);
	wSize.width = clientRect.right - clientRect.left;
	wSize.height = clientRect.bottom - clientRect.top;

	return wSize;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		WindowSize wSize = GetWindowSize();
		Create_Init_Backbuffer(wSize.width, wSize.height);
		break;

	case WM_DESTROY:
		running = false;
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			running = false;
			PostQuitMessage(0);
		}
		return 0;
	}	

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdSHow)
{
	//From stack overflow: fi is not needed, we only use it because it's a required parameter
	//The important code is that stdout is redirected to CONOUT$, which is the console output stream
	bool console_enabled = AllocConsole();
	if (console_enabled)
	{
		FILE* fi = 0;
		freopen_s(&fi, "CONOUT$", "w", stdout);
	}
	console_enabled = AttachConsole(ATTACH_PARENT_PROCESS);
	
	const char CLASS_NAME[] = "Multimedia Visualizer";

	WNDCLASS wc = {0};
	wc.style = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	window = CreateWindowExA(0, CLASS_NAME, "Multimedia Visualizer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
	if (window == NULL) return -1;

	ShowWindow(window, nCmdSHow);

	device_context = GetDC(window);
	
	MSG msg;
	while (running)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);		
		}

		WindowSize wSize = GetWindowSize();	
		StretchDIBits(device_context,
			0, 0, wSize.width, wSize.height, //destination
			0, 0, wSize.width, wSize.height, //source
			bitmap_memory,
			&bmInfo, // BITMAP INFO
			DIB_RGB_COLORS,
			SRCCOPY);
	}

	if (console_enabled)
	{
		FreeConsole();
	}

	return 0;
}