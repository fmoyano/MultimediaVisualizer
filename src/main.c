#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <wingdi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "img_loader.h"
#include "png_loader.h"

static HWND window = 0;
static BITMAPINFO bmInfo = {0};
static void *bitmap_memory;
static HDC device_context;

static bool running = true;

static int image_width = 0;
static int image_height = 0;
static int image_depth = 0;
static void* rgb_data = 0;

typedef struct WindowSize
{
	int width;
	int height;
} WindowSize;

typedef unsigned char byte;

//Allocate a back buffer
static void Create_Init_Backbuffer()
{	
	if (bitmap_memory)
	{
		VirtualFree(bitmap_memory, 0, MEM_RELEASE);
	}

	size_t memory_size = 3 * image_width * image_height;
	bitmap_memory = VirtualAlloc(0, memory_size, MEM_COMMIT, PAGE_READWRITE);

	if (rgb_data)
		memcpy(bitmap_memory, rgb_data, memory_size);
	/*byte *row = (byte *)bitmap_memory;
	byte* rgb_data_byte = (byte*) rgb_data;
	for (int i = 0; i < bytes_per_pixel * image_width * image_height; ++i)
	{
		*row++ = *rgb_data_byte++;
	}*/

	/*int pitch = bytes_per_pixel * width;
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
	}*/
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
		Create_Init_Backbuffer();
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
	
	printf("Command line: %S\n", pCmdLine);

	if (pCmdLine[0] == 0)
	{
		printf("Enter filename to visualize\n");
		//TODO: remove in release
		pCmdLine = L"data/lion.png";
		//return -1;
	}

	const char CLASS_NAME[] = "Multimedia Visualizer";

	char* file_name = malloc(20);
	size_t chars_converted = 0;
	wcstombs_s(&chars_converted, file_name, 20, pCmdLine, 20);
	printf("File name is %s\n", file_name);
	image_width = 0, image_height = 0, image_depth = 0;

	BITMAPINFOHEADER bmHeader = {0};
	png_loader_open(file_name);
	//rgb_data = img_loader_load(file_name, &image_width, &image_height, &image_depth);
	if (!rgb_data)
	{
		printf("Not valid file\n");
	}
	else
	{
		bmHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmHeader.biPlanes = 1;
		bmHeader.biCompression = BI_RGB;	
		bmHeader.biWidth = image_width;
		bmHeader.biHeight = image_height;
		bmHeader.biBitCount = image_depth;
		bmInfo.bmiHeader = bmHeader;
	}

	WNDCLASS wc = {0};
	wc.style = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	window = CreateWindowExA(0, CLASS_NAME, "Multimedia Visualizer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, hInstance, 0);
	if (window == 0) return -1;

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
			0, 0, image_width, image_height, //destination
			0, 0, image_width, image_height, //source
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