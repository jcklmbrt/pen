#ifndef _INCLUDE_ENT_H
#define _INCLUDE_ENT_H

#include <r.h>

#define MAX_ENTITIES 64


struct ent {
	/* position and velocity are scalar 
	   as we are moving along a line */
	float   pos;
	float   vel;
	float   size;
	color_t color;
};


struct lvl {
	float start[2][2];
	float end[2][2];
};


void entmv(float dt);
bool entadd(void);
void lvldraw(void);
void entdraw(void);


#endif
