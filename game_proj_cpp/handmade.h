#if !defined(HANDMADE_H)

/*
	Takes the timing, controller input, the buffer to render output, the sound buffer to output
*/
#include <Windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>
#include <math.h>

#define internal static
#define local_persist static
#define global_var static

#define Pi32 3.14159265359f


typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;

struct game_offscreen_buffer {
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

void GameUpdateAndRender(game_offscreen_buffer *buffer, int blueOffset, int greenOffset);

#define HANDMADE_H
#endif

