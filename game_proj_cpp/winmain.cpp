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

struct win32_window_dimension {
	int width;
	int height;
};

win32_window_dimension
Win32GetWindowDimension(HWND window) 
{
	win32_window_dimension result;

	RECT rect;
	GetClientRect(window, &rect);
	result.width = rect.right - rect.left;
	result.height = rect.bottom - rect.top;

	return(result);
}

struct win32_offscreen_buffer
{
	 BITMAPINFO Info;
	 void *Memory;
	 int Width;
	 int Height;
	 int Pitch;
	 int bytesPerPixel;
};

global_var win32_offscreen_buffer globalBackbuffer;

internal void RenderWeirdGradient(win32_offscreen_buffer buffer, int xOffset, int yOffset)
{
	int width = buffer.Width;
	int height = buffer.Height;
	// Pitch is the amount we want to move the pointer, difference between a row and the next row
	// pointer aliasing, when compiler doesn't know if a pointer has been rewritten
	uint8 *row = (uint8 *)buffer.Memory;
	for (int y = 0; y < height; ++y) 
	{
		/*
			Pointer arithmetic - anytime you add or subtract something from a pointer to move it around in memory, C will silently multiply that movement by the size of the thing being pointed to
			16 bit - move by 2, every time. Don't want to do that for explicit arithmetic
			Little endian architecture - first byte in the lowest part
		*/
		uint32 *pixel = (uint32 *)row;
		for (int x = 0; x < width; ++x)
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

		row += buffer.Pitch;
	}
}

//device independent bitmap
internal void Win32ResizeDIBSection(win32_offscreen_buffer *buffer, int width, int height) 
{
    if (buffer->Memory) 
    {
        // committing changes, switching to windows, 17:01
        VirtualFree(buffer->Memory, 0, MEM_RELEASE);
    }

	buffer->Width = width;
	buffer->Height = height;
	buffer->bytesPerPixel = 4;

	/* 
		Error that happens everytime you run the program
		Error that you may not find before you ship
	*/

	buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
	buffer->Info.bmiHeader.biWidth = buffer->Width;
	buffer->Info.bmiHeader.biHeight = -buffer->Height;
	buffer->Info.bmiHeader.biPlanes = 1;
	buffer->Info.bmiHeader.biBitCount = 32;
	buffer->Info.bmiHeader.biCompression = BI_RGB;

	buffer->Pitch = width*buffer->bytesPerPixel;
	int bitmapMemorySize = (buffer->Width * buffer->Height) * buffer->bytesPerPixel;
	buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);


}

internal void Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth, int windowHeight, win32_offscreen_buffer buffer, int x, int y, int width, int height) 
{
	// Aspect Ratio - floating point or in integer
	// TODO: Aspect ratio correction
	StretchDIBits(deviceContext, 
		0, 0, windowWidth, windowHeight, 0, 0,buffer.Width, buffer.Height, buffer.Memory, &buffer.Info, DIB_RGB_COLORS, SRCCOPY);
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
		win32_window_dimension dimension = Win32GetWindowDimension(window);
		Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, globalBackbuffer, x, y, width, height);

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
	Win32ResizeDIBSection(&globalBackbuffer, 1280, 720);

	windowClass.style = CS_HREDRAW|CS_VREDRAW;
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
                // trying to be lexically scoped as close as possible
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
					if (message.message == WM_QUIT) {
						running = false;
					}
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				RenderWeirdGradient(globalBackbuffer, xOffset, yOffset);

				HDC deviceContext = GetDC(windowHandle);
				win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);

				Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, globalBackbuffer, 0, 0, dimension.width, dimension.height);
				ReleaseDC(windowHandle, deviceContext);
				
				++xOffset;
                yOffset += 2;
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
