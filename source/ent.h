#ifndef _INCLUDE_ENT_H
#define _INCLUDE_ENT_H

#include <r.h>

#define MAX_ENTITIES 1024

struct ent {
	/* position and velocity are scalar
	   as we are moving along a line */
	float   pos;
	float   vel;
	float   size;
	color_t color;
};


void entmv(float dt);
void lvlmv(float dt);
void lvldraw(void);
void entdraw(void);
bool entadd(void);


#endif
