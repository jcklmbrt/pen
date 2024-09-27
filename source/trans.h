#ifndef _INCLUDE_TRANS_H
#define _INCLUDE_TRANS_H

#include <input.h>

/* actual trans API */
void  w2s(float w[2], float s[2]);
void  s2w(float s[2], float w[2]);
float gscale();
void  transinput(union input *in, uint8_t flags);
/* misc math functions TODO: move */
float lerp(float a, float b, float t);
void  v2norm(float p[2]);
void  v2perp(float p[2]);
int   v2dlt(float a[2], float b[2], float lt);
float v2dlen(float a[2], float b[2]);

#endif