#include <Windows.h>

#define internal static
#define local_persist static
#define global_var static

// TODO: global for now
global_var bool running;
global_var BITMAPINFO bitmapInfo;
global_var void *bitmapMemory;

//device independent bitmap
internal void Win32ResizeDIBSection(int width, int height) 
{
	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bytesPerPixel = 4;
	int bitmapMemorySize = (width * height) * bytesPerPixel;
	bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height) 
{
	// rectangle to rectangle copy, buffer and window
	// RGB buffer, 

	StretchDIBits(deviceContext, x, y, width, height, x, y, width, height, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}


LRESULT CALLBACK Win32MainWindowCallback(
	HWND   window,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (message) {
	case WM_SIZE:
	{
		RECT clientRect;
		GetClientRect(window, &clientRect);
		int width = clientRect.right - clientRect.left;
		int height = clientRect.bottom - clientRect.top;
		Win32ResizeDIBSection(width, height);
		OutputDebugStringA("WM_SIZE\n");
	} break;

	case WM_DESTROY:
	{
		// TODO: handle this as an error - recreate window
		running = false;
	} break;

	case WM_CLOSE:
	{
		// TODO: handle this as a message to the user
		running = false;
	} break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugStringA("WM_ACTIVATEAPP\n");
	
	} break;

	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC deviceContext = BeginPaint(window, &paint);
		int x = paint.rcPaint.left;
		int y = paint.rcPaint.top;
		int height = paint.rcPaint.bottom - paint.rcPaint.top;
		int width = paint.rcPaint.right - paint.rcPaint.left;
		Win32UpdateWindow(deviceContext, x, y, width, height);
		EndPaint(window, &paint);

	} break;

	default:
	{
		OutputDebugStringA("default\n");
		result = DefWindowProc(window, message, wParam, lParam);

	} break;

	}


	return(result);
}


int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
	WNDCLASS windowClass = {};
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	// Get module handle gets the current instance wherever you are
	windowClass.hInstance = instance;
	// windowClass.hIcon
	windowClass.lpszClassName = "GameWindowClass";
	if (RegisterClass(&windowClass)) {
		HWND windowHandle = CreateWindowEx(
			0,
			windowClass.lpszClassName,
			"Game Time Yeah",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			instance,
			0);
		if (windowHandle) 
		{
			running = true;
			// Windows sends messages to your window through a message queue
			while (running) {
				MSG message;
				BOOL MessageResult = GetMessage(&message, 0, 0, 0);
				if (MessageResult > 0) {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else {
					break;
				}
			}
		}
		else {
			// TODO: logging
		}
	}
	else {
	
	}

	return(0);
}