#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "input.h"
#include "r.h"
#include "polybezier.h"
#include "font.h"
#include "trans.h"


/* call every frame. */
int framerate(bool newframe, float *dt)
{
	static float    lastfps = 0.0f;
	static uint64_t numframes = 0;
	static uint64_t lastticks = 0;
	const  uint64_t sampleframes = 20;
	if(newframe) {
		numframes++;
	}
	if(numframes > sampleframes || (lastticks == 0 && newframe)) {
		uint64_t cur_ticks = getticks();
		float dtime = (float)(cur_ticks - lastticks);
		*dt = dtime / 1000.0f;
		float frametime = dtime / numframes;
		lastticks = cur_ticks;
		numframes = 0;
		lastfps   = 1000.0f / frametime;
	}
	return (int)lastfps;
}


static void rdbg(int fps)
{
	rprintf(TERMINUS_8x16, 25.0f, 25.0f + (16.0f * 0), white, "Frames Per Second: %d", fps);
	rprintf(TERMINUS_8x16, 25.05, 25.0f + (16.0f * 1), white, "PolyBezier Length: %.1f", pblen());
}


int main(int argc, char **argv)
{
	(void)argv;
	(void)argc;

	float dt;

	srand(time(NULL));

	if(!rinit("pen")) {
		goto end;
	}

	union input in;
	uint8_t     flags;

	for(;;) {
		int fps = framerate(1, &dt);
		while(pollevent(&in, &flags)) {
			switch(flags & IN_BASE) {
			case IN_QUIT:
				goto end;
			case IN_MOUSE:
			case IN_SCROLL:
				transinput(&in, flags);
				pbinput(&in, flags);
				break;
			}
		}
		rclear(gray);
		pbdraw();
#ifndef NDEBUG
		rdbg(fps);
#endif
		rpresent();
	}
end:
	pbfree();
	rfree();
	return 0;
}
