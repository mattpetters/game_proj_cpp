#if !defined(HANDMADE_H)

void PlatformLoadFile(char *FileName);

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define Pi32 3.1415926539f
#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;
// TODO Services that the platform layer provides to the game

/* NOTE
Services that the game provides to the platform layer.
This may expand in the future - sound of separate thread, ect.
*/

// TODO In the future, rendering _specifically_ will become a three-tiered abstraction
struct game_offscreen_buffer {
	void *Memory;
	int Width;
	int Height;
	int Pitch;
};

struct game_sound_output_buffer {
	int SamplesPerSecond;
	int SampleCount;
	int16 *Samples;
};

// FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use
void GameUpdateAndRender(game_offscreen_buffer *Buffer, int XOffset, int YOffset, game_sound_output_buffer *SoundBuffer, int ToneHz);

#define HANDMADE_H
#endif