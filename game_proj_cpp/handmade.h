#if !defined(HANDMADE_H)

/*
	Takes the timing, controller input, the buffer to render output, the sound buffer to output
*/
struct game_offscreen_buffer {
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

void GameUpdateAndRender(game_offscreen_buffer *buffer, int blueOffset, int greenOffset);

#define HANDMADE_H
#endif

