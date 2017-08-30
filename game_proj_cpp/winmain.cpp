#include <Windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_var static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


// TODO: global for now
global_var bool running;
global_var BITMAPINFO bitmapInfo;
global_var void *bitmapMemory;
global_var int bitmapWidth;
global_var int bitmapHeight;
global_var int bytesPerPixel = 4;

internal void
RenderWeirdGradient(int xOffset, int yOffset)
{
	int width = bitmapWidth;
	int height = bitmapHeight;
	// Pitch is the amount we want to move the pointer, difference between a row and the next row
	int pitch = width*bytesPerPixel;
	uint8 *row = (uint8 *)bitmapMemory;
	for (int y = 0; y < bitmapHeight; ++y) 
	{
		/*
			Pointer arithmetic - anytime you add or subtract something from a pointer to move it around in memory, C will silently multiply that movement by the size of the thing being pointed to
			16 bit - move by 2, every time. Don't want to do that for explicit arithmetic
			Little endian architecture - first byte in the lowest part
		*/
		uint32 *pixel = (uint32 *)row;
		for (int x = 0; x < bitmapWidth; ++x)
		{
			// Dereference operator, access the memory pointed to by this pixel
			/*
								pixel+0 pixel+1 pixel+2 pixel+3
				Pixel in memory: bb      gg      rr       xx
			*/
			uint8 blue = (x + xOffset);
			uint8 green = (y + yOffset);

			// 32 bit write
			/*
				Memory: BB GG RR xx

				Register: xx RR GG BB (little endian)

				Blue in the bottom 8 bits

				ORing the blue bits with the green bits

				If either are set, it sets it, composites them together
			*/
			// shift these values up and OR them together
			*pixel++ = ((green << 8) | blue);
		}

		row += pitch;
	}
}

//device independent bitmap
internal void Win32ResizeDIBSection(int width, int height) 
{
    if (bitmapMemory) 
    {
        // committing changes, switching to windows, 17:01
        VirtualFree(bitmapMemory, 0, MEM_RELEASE);
    }

	bitmapWidth = width;
	bitmapHeight = height;


	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = bitmapWidth;
	bitmapInfo.bmiHeader.biHeight = -bitmapHeight;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = (bitmapWidth * bitmapHeight) * bytesPerPixel;
	bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

}

internal void Win32UpdateWindow(HDC deviceContext, RECT *windowRect, int x, int y, int width, int height) 
{

	int windowWidth = windowRect->right - windowRect->left;
	int windowHeight = windowRect->bottom - windowRect->top;

	StretchDIBits(deviceContext, 
		0, 0, bitmapWidth, bitmapHeight, 0, 0, windowWidth, windowHeight, bitmapMemory, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
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
		RECT clientRect;
		GetClientRect(window, &clientRect);
		Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
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
			"Handmade Hero",
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

				int xOffset = 0;
				int yOffset = 0;
			while (running) {
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
					if (message.message == WM_QUIT) {
						running = false;
					}
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				RenderWeirdGradient(xOffset, yOffset);

				HDC deviceContext = GetDC(windowHandle);
				RECT windowRect;
				GetClientRect(windowHandle, &windowRect);
				int windowWidth = windowRect.right - windowRect.left;
				int windowHeight = windowRect.bottom - windowRect.top;

				Win32UpdateWindow(deviceContext, &windowRect, 0, 0, windowWidth, windowHeight);
				ReleaseDC(windowHandle, deviceContext);

				++xOffset;
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
