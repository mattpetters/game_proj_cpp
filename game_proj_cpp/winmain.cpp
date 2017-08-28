#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS windowClass = {};
	// windowClass.style = ;
	// windowClass.lpfnWndProc = ;
	// Get module handle gets the current instance wherever you are
	windowClass.hInstance = hInstance;
	// windowClass.hIcon
	windowClass.lpszClassName = "GameWindowClass";
	//stopped at 16:43 - ep 2

	return(0);
}