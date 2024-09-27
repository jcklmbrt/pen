#ifndef _INCLUDE_POLYBEZIER_H
#define _INCLUDE_POLYBEZIER_H


#include <stdint.h>


union input;


void  pbfree(void);
int   pbinput(union input *in, uint8_t flags);
void  pbdraw(void);
int   pbinterp(float t, float out[2]);
float pblen(void);


#endif
