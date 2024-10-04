
#ifndef _INCLUDE_RENDERER_H
#define _INCLUDE_RENDERER_H

#include <stdbool.h>

#include <font.h>

#define R_MAX_TEXT     1024
#define R_CIRCLE_STEPS 32


typedef struct color { unsigned char r, g, b, a; } color_t;


extern color_t red;
extern color_t green;
extern color_t blue;
extern color_t white;
extern color_t black;
extern color_t darkblue;
extern color_t ltblue;
extern color_t blacktrans;
extern color_t pink;
extern color_t gray;
extern color_t yellow;


/* platform specific functions defined in r<impl>.c */
bool rinit(const char *title);
void rfree(void);
void rsize(float *w, float *h);
void rclear(color_t color);
void rpresent(void);
void rtris(color_t color, float *positions, int num_positions, int *indices, int num_indices);
void rtextries(int texid, color_t color, float *positions, float *tex_coords,
               int num_vertices, int *indices, int num_indices);


/* primitives, defined in r.c */
void rrect(float x, float y, float w, float h, color_t color);
void rline(float x0, float y0, float x1, float y1, float thickness, color_t color);
void rquad(float quad[4][2], color_t color);
void rcircle(float x, float y, float r, color_t color);
int  rprintf(int fid, float x, float y, color_t color, const char *fmt, ...);


#endif
