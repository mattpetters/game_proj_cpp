#include <Windows.h>


LRESULT CALLBACK MainWindowCallback(
	HWND   window,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (message) {
	case WM_SIZE:
	{
		OutputDebugStringA("WM_SIZE\n");
	} break;

	case WM_DESTROY:
	{
		OutputDebugStringA("WM_DESTROY\n");
	} break;

	case WM_CLOSE:
	{
		OutputDebugStringA("WM_CLOSE\n");
	
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
		// Very bad idea, global scope, however local scope lexically
		static DWORD operation = WHITENESS;
		PatBlt(deviceContext, x, y, width, height, operation);
		if (operation == WHITENESS) {
			operation = BLACKNESS;
		}
		else {
			operation = WHITENESS;
		}
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
	windowClass.lpfnWndProc = MainWindowCallback;
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
			// Windows sends messages to your window through a message queue
			for (;;) {
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