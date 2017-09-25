#include "handmade.h"


void GameOutputSound(game_sound_output_buffer *SoundBuffer)
{
    local_persist real32 tSine;
    int16 toneVolume = 3000;
    int16 *sampleOut = SoundBuffer->samples;
    int toneHz = 256;
    int wavePeriod = SoundBuffer->samplesPerSecond/toneHz;
    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        real32 sineValue = sinf(t);
        int16 sampleValue = (int16)(sineValue * toneVolume);
        *sampleOut++ = sampleValue;
        *sampleOut++ = sampleValue;

        tSine += 2.0f*Pi32*1.0f/(real32)wavePeriod;
    }
}

void RenderWeirdGradient(game_offscreen_buffer *buffer, int xOffset, int yOffset)
{
	int width = buffer->Width;
	int height = buffer->Height;
	// Pitch is the amount we want to move the pointer, difference between a row and the next row
	// pointer aliasing, when compiler doesn't know if a pointer has been rewritten
	uint8 *row = (uint8 *)buffer->Memory;
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

		row += buffer->Pitch;
	}
}

void GameUpdateAndRender(game_offscreen_buffer *buffer, int blueOffset, int greenOffset, game_sound_output_buffer *SoundBuffer)
{
    //TODO: Allow sample offsets for more robust platform options
    GameOutputSound(SoundBuffer);
    RenderWeirdGradient(buffer, blueOffset, greenOffset);
}
