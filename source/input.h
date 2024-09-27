#ifndef _INCLUDE_INPUT_H
#define _INCLUDE_INPUT_H

#include <stdbool.h>
#include <stdint.h>

union input {
	float        mpos[2];
	unsigned int keycode;
};

enum inflag {
	IN_NONE   = 0,
	/* fundamental */
	IN_QUIT   = 1,
	IN_MOUSE  = 2,
	IN_SCROLL = 3,
	IN_KEY    = 4,
	IN_BASE   = IN_QUIT   | 
	            IN_MOUSE  |
	            IN_SCROLL |
	            IN_KEY,
	/* mouse key modifiers */
	IN_L      = 1 << 3,
	IN_R      = 2 << 3,
	IN_M      = 3 << 3,
	/* mouse move modifier */
	IN_MOVE   = 1 << 5,
	/* generic modifiers */
	IN_DOWN   = 0, /* IN_DOWN is the "default" modifier e.g. if no bits are set, down is assumed */
	IN_UP     = 1 << 6,
};

bool     pollevent(union input *in, uint8_t *flags);
uint64_t getticks(void);

#endif
