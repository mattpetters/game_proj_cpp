#include "handmade.h"

global_var bool running;
global_var LPDIRECTSOUNDBUFFER globalSecondaryBuffer;


struct win32_window_dimension {
	int width;
	int height;
};


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice,  LPDIRECTSOUND *ppDS,  LPUNKNOWN pUnkOuter);
typedef DIRECT_SOUND_CREATE(direct_sound_create);


#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_var x_input_get_state *XInputGetState_ = XInputGetStateStub;

// initializing function pointers so that program doesn't crash

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub) 
{
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_var x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void)
{
	HMODULE xInputLibrary = LoadLibrary("xinput1_4.dll");

	if (!xInputLibrary) {
	HMODULE xInputLibrary = LoadLibrary("xinput1_3.dll");
	}
	if (xInputLibrary) {
		XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
	}
}


internal void
Win32InitDirectSound(HWND window, int32 bufferSize, int32 samplesPerSecond) 
{
	//load the library
	HMODULE DirectSoundLibrary = LoadLibraryA("dsound.dll");
	if (DirectSoundLibrary) {
		direct_sound_create *DirectSoundCreate = (direct_sound_create*)GetProcAddress(DirectSoundLibrary, "DirectSoundCreate");
		WAVEFORMATEX waveFormat = {};
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nChannels = 2;
		waveFormat.nSamplesPerSec = samplesPerSecond;
		waveFormat.wBitsPerSample = 16;
		waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
		waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign*waveFormat.nSamplesPerSec;
		waveFormat.cbSize = 0;

		LPDIRECTSOUND directSound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &directSound, 0))) {
			if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC bufferDescription = { sizeof(bufferDescription) };
				bufferDescription.dwSize = sizeof(bufferDescription);
				bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

				LPDIRECTSOUNDBUFFER primaryBuffer;
				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0))) {

					if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat))) {
						OutputDebugStringA("Primary buffer format was set \n");
					} else {
						//TODO: logging
					}
				}
				else {
					//TODO: logging
				}


			}
			else {
				// TODO: logging
			}

			DSBUFFERDESC bufferDescription = { sizeof(bufferDescription) };
			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwFlags = 0;
			bufferDescription.dwBufferBytes = bufferSize;
			bufferDescription.lpwfxFormat = &waveFormat;

			HRESULT error = directSound->CreateSoundBuffer(&bufferDescription, &globalSecondaryBuffer, 0);
			if (SUCCEEDED(error)) 
			{
					OutputDebugStringA("Secondary buffer format was set \n");
			}
			else {
				//TODO: logging
			}
		}
		else {
			//todo: logging
		}

	}
	else {
			//todo: logging
	}


	//get a directsound object
	//create a primary buffer
	//create a secondary buffer
	//start it playing
}

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

internal void Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth, int windowHeight, win32_offscreen_buffer *buffer, int x, int y, int width, int height) 
{
	// Aspect Ratio - floating point or in integer
	// TODO: Aspect ratio correction
	StretchDIBits(deviceContext, 
		0, 0, windowWidth, windowHeight, 0, 0,buffer->Width, buffer->Height, buffer->Memory, &buffer->Info, DIB_RGB_COLORS, SRCCOPY);
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

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		uint32 virtualKeyCode = wParam;
		bool WasDown = ((lParam & (1 << 30)) != 0);
		bool IsDown = ((lParam & (1 << 31)) == 0);
		if (IsDown != WasDown) {
			if (virtualKeyCode == 'W') {
			}
			else if (virtualKeyCode == 'A') {
			}
			else if (virtualKeyCode == 'S') {
			}
			else if (virtualKeyCode == 'D') {
			}
			else if (virtualKeyCode == 'Q') {
			}
			else if (virtualKeyCode == 'E') {
			}
			else if (virtualKeyCode == VK_UP) {
			}
			else if (virtualKeyCode == VK_DOWN) {
			}
			else if (virtualKeyCode == VK_LEFT) {
			}
			else if (virtualKeyCode == VK_RIGHT) {
			}
			else if (virtualKeyCode == VK_ESCAPE) {
			}
			else if (virtualKeyCode == VK_SPACE) {
				OutputDebugStringA("Space:");
				if (IsDown) {
				OutputDebugStringA("IsDown\n");
				}
				else if (WasDown) {
				OutputDebugStringA("WasDown\n");
				}
			};
		}
		
		bool AltKeyWasDown = ((lParam & (1 << 29)) != 0);
		if ((virtualKeyCode == VK_F4) && AltKeyWasDown){
			running = false;
		}

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
		Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, &globalBackbuffer, x, y, width, height);

		EndPaint(window, &paint);

	} break;

	default:
	{
		result = DefWindowProc(window, message, wParam, lParam);

	} break;

	}


	return(result);
}

struct win32_sound_output {
	int samplesPerSecond;
	int toneHz;
	int16 toneVolume;
	uint32 runningSampleIndex;
	int wavePeriod;
	int bytesPerSample;
	int secondaryBufferSize;
};


void
Win32FillSoundBuffer(win32_sound_output *soundOutput, DWORD byteToLock, DWORD bytesToWrite)
{
	VOID *region1;
	DWORD region1Size;
	VOID *region2;
	DWORD region2Size;


	if (SUCCEEDED(globalSecondaryBuffer->Lock(byteToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0))) 
	{
			// assert that region1Size is valid

			int16 *sampleOut = (int16 *)region1;
			DWORD region1SampleCount = region1Size / soundOutput->bytesPerSample;

			for (DWORD sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
			{
				real32 t = (2.0f*Pi32*(real32)soundOutput->runningSampleIndex) / (real32)soundOutput->wavePeriod;
				real32 sineValue = sinf(t);
				int16 sampleValue = (int16)(sineValue* soundOutput->toneVolume);
				*sampleOut++ = sampleValue;
				*sampleOut++ = sampleValue;

				++soundOutput->runningSampleIndex;
			}

			sampleOut = (int16 *)region2;
			DWORD region2SampleCount = region2Size / soundOutput->bytesPerSample;
			for (DWORD sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
			{
				real32 t = (2.0f*Pi32*(real32)soundOutput->runningSampleIndex) / (real32)soundOutput->wavePeriod;
				real32 sineValue = sinf(t);
				int16 sampleValue = (int16)(sineValue* soundOutput->toneVolume);
				*sampleOut++ = sampleValue;
				*sampleOut++ = sampleValue;

				++soundOutput->runningSampleIndex;
			}
			globalSecondaryBuffer->Unlock(region1, region1Size, region2, region2Size);
		}
}

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
	LARGE_INTEGER performanceCountFrequencyResult;
	QueryPerformanceFrequency(&performanceCountFrequencyResult);
	int64 performanceCountFrequency = performanceCountFrequencyResult.QuadPart;

	Win32LoadXInput();
	WNDCLASSA windowClass = {};
	Win32ResizeDIBSection(&globalBackbuffer, 1280, 720);

	windowClass.style = CS_HREDRAW|CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	// Get module handle gets the current instance wherever you are
	windowClass.hInstance = instance;
	// windowClass.hIcon
	windowClass.lpszClassName = "GameWindowClass";

	if (RegisterClass(&windowClass)) 
	{
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
			HDC deviceContext = GetDC(windowHandle);
			// Windows sends messages to your window through a message queue
			
				int xOffset = 0;
				int yOffset = 0;

				win32_sound_output soundOutput = {};
				soundOutput.samplesPerSecond = 48000;
				soundOutput.toneHz = 256;
				soundOutput.toneVolume = 3000;
				soundOutput.runningSampleIndex = 0;
				soundOutput.wavePeriod = soundOutput.samplesPerSecond/soundOutput.toneHz;
				soundOutput.bytesPerSample = sizeof(int16) * 2;
				soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond*soundOutput.bytesPerSample;
				Win32InitDirectSound(windowHandle, soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);
				Win32FillSoundBuffer(&soundOutput, 0, soundOutput.secondaryBufferSize);
				globalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
				
				LARGE_INTEGER lastCounter;
				QueryPerformanceCounter(&lastCounter);
				int64 lastCycleCount = __rdtsc();

				running = true;
				while (running) 
				{


					MSG message;
					while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) 
					{
						if (message.message == WM_QUIT) {
							running = false;
						}
						TranslateMessage(&message);
						DispatchMessage(&message);
					}

					// X-input is a polling based API, it only gives us the current state of the controller when we ask for it
					// interrupt and polling input
					// interrupt, whenever device needs to tell you something happened, it sends you the data
					// old days, interrupts on the CPU
					// polling usually, or packet networking buffering thing

					for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; ++controllerIndex) 
					{
						XINPUT_STATE controllerState;
						if (XInputGetState(controllerIndex, &controllerState) == ERROR_SUCCESS) 
						{
							// gotta love things like error_success
							// controller is plugged in
							XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

							bool Up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
							bool Down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
							bool Left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
							bool Right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

							bool Start = (pad->wButtons & XINPUT_GAMEPAD_START);
							bool Back = (pad->wButtons & XINPUT_GAMEPAD_BACK);

							bool LeftShoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
							bool RightShoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

							bool AButton = (pad->wButtons & XINPUT_GAMEPAD_A);
							bool BButton = (pad->wButtons & XINPUT_GAMEPAD_B);
							bool XButton = (pad->wButtons & XINPUT_GAMEPAD_X);
							bool YButton = (pad->wButtons & XINPUT_GAMEPAD_Y);

							int16 StickX = pad->sThumbLX;
							int16 StickY = pad->sThumbLY;

							if (AButton) {
								yOffset += 2;
								soundOutput.toneHz = 512;
								soundOutput.wavePeriod = soundOutput.samplesPerSecond / soundOutput.toneHz;
							}

							xOffset += StickY >> 12;
							yOffset += StickX >> 12;

						}
						else {
							// controller is not available
						}
					}

                    int16 Samples[(48000/30) * 2];

                    game_sound_output_buffer soundBuffer = {};
                    soundBuffer.samplesPerSecond = soundOutput.samplesPerSecond;
                    soundBuffer.sampleCount = soundBuffer.samplesPerSecond / 30;
                    soundBuffer.samples = Samples;
                    game_offscreen_buffer Buffer = {};
                    Buffer.Memory = globalBackbuffer.Memory;
                    Buffer.Width = globalBackbuffer.Width;
                    Buffer.Height = globalBackbuffer.Height;
                    Buffer.Pitch = globalBackbuffer.Pitch;

					GameUpdateAndRender(&Buffer, xOffset, yOffset, &soundBuffer);

					//DirectSound test
					DWORD playCursor;
					DWORD writeCursor;
					if (SUCCEEDED(globalSecondaryBuffer->GetCurrentPosition(&playCursor, &writeCursor))) 
					{

						DWORD byteToLock = ((soundOutput.runningSampleIndex*soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize);
						DWORD bytesToWrite;

						if (byteToLock > playCursor) 
						{
							bytesToWrite = (soundOutput.secondaryBufferSize - byteToLock);
							bytesToWrite += playCursor;
						}
						else 
						{
							bytesToWrite = playCursor - byteToLock;
						}

						Win32FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite);
					}

				win32_window_dimension dimension = Win32GetWindowDimension(windowHandle);
				Win32DisplayBufferInWindow(deviceContext, dimension.width, dimension.height, &globalBackbuffer, 0, 0, dimension.width, dimension.height);

				int64 endCycleCount = __rdtsc();
				LARGE_INTEGER endCounter;
				QueryPerformanceCounter(&endCounter);

				// MainLoop();
				//display the value here
				int64 cyclesElapsed = endCycleCount - lastCycleCount;
				int64 counterElapsed = endCounter.QuadPart - lastCounter.QuadPart; //
				// 1 / counts per frame
				int32 milliseconds = (int32)(1000 * counterElapsed); //
				int32 msPerFrame = (int32)((milliseconds) / performanceCountFrequency); //ms per frame
				int32 framesPerSecond = (int32)performanceCountFrequency / (int32) counterElapsed;
				int32 megacyclesPerFrame = (int32)(cyclesElapsed / (1000 * 1000));
#if 0
				char buffer[256];
				wsprintf(buffer, "%dms/f,  %df/s,  %dmc/s \n", msPerFrame, framesPerSecond, megacyclesPerFrame);
				OutputDebugStringA(buffer);
#endif
				lastCounter = endCounter;
				lastCycleCount = endCycleCount;
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
