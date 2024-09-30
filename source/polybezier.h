#ifndef _INCLUDE_POLYBEZIER_H
#define _INCLUDE_POLYBEZIER_H


#include <stdint.h>
#include <stdbool.h>


union input;


void  pbfree(void);
bool  pbinput(union input *in, uint8_t flags);
void  pbdraw(void);
bool  pbinterp(float t, float out[2]);
float pblen(void);


#endif
