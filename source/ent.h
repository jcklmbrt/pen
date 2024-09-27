#ifndef _INCLUDE_ENT_H
#define _INCLUDE_ENT_H


#define MAX_ENTITIES 64


struct ent {
	/* position and velocity are scalar 
	   as we are moving along a line */
	float pos;
	float vel;
};


struct obs {
	/* AABB */
	float min[2];
	float max[2];
};


void entmv(float dt);
bool entadd(void);
void entdraw(void);


#endif
