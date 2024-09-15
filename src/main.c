#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <wingdi.h>
#include <stdbool.h>
#include <stdio.h>

static HWND window = 0;
static BITMAPINFO bmInfo = {0};
static void *bitmap_memory;
static HBITMAP bitmap_handle;

typedef struct WindowSize
{
	unsigned width;
	unsigned height;
} WindowSize;

//Allocate a back buffer (DIB Section)
static void InitOrResizeDIBSection(unsigned width, unsigned height)
{
	if (bitmap_handle)
	{
		DeleteObject(bitmap_handle);
	}

	BITMAPINFOHEADER bmHeader = {0};
	bmHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmHeader.biWidth = width;
	bmHeader.biHeight = height;
	bmHeader.biPlanes = 1;
	bmHeader.biBitCount = 32;
	bmHeader.biCompression = BI_RGB;

	bmInfo.bmiHeader = bmHeader;

	HDC device_context = CreateCompatibleDC(0);
	bitmap_handle = CreateDIBSection(
  		device_context,
  		&bmInfo,
  		DIB_RGB_COLORS,
  		&bitmap_memory,
  		0, 0);
	DeleteDC(device_context);
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
		InitOrResizeDIBSection(wSize.width, wSize.height);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
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
	
	const char CLASS_NAME[] = "Sample Window Class";

	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	window = CreateWindowExA(0, CLASS_NAME, "Multimedia Visualizer", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (window == NULL) return -1;

	ShowWindow(window, nCmdSHow);

	WindowSize wSize = GetWindowSize();
	
	//Create DIB
	//CreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT usage, void **ppvBits, HANDLE hSection, DWORD offset)
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		//TODO: fill in (how to get device context?)
		/*StretchDIBits(0, //device context
			0, 0, wSize.width, wSize.height, //destination
			0, 0, wSize.width, wSize.height, //source
			const void *lpBits, //image data as bytes
			const BITMAPINFO *lpbmi, // BITMAP INFO
			DIB_RGB_COLORS,
			SRCCOPY);*/
	}

	if (console_enabled)
	{
		FreeConsole();
	}

	return 0;
}