#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>

#include <font.h>
#include <trans.h>
#include <r.h>


color_t red        = { 0xFF, 0x00, 0x00, 0xFF };
color_t green      = { 0x00, 0xFF, 0x00, 0xFF };
color_t blue       = { 0x00, 0x00, 0xFF, 0xFF };
color_t white      = { 0xFF, 0xFF, 0xFF, 0xFF };
color_t black      = { 0x00, 0x00, 0x00, 0xFF };
color_t darkblue   = { 0x00, 0x00, 0x77, 0xFF };
color_t ltblue     = { 0x33, 0x33, 0xFF, 0xFF };
color_t blacktrans = { 0x00, 0x00, 0x00, 0x33 };
color_t pink       = { 0xFF, 0x77, 0x77, 0xFF };
color_t gray       = { 0x77, 0x77, 0x77, 0xFF };
color_t yellow     = { 0x77, 0x77, 0x00, 0xFF };


static void rect2quad(float x, float y, float w, float h, float quad[4][2])
{
	quad[0][0] = x;
	quad[0][1] = y;
	quad[1][0] = x + w;
	quad[1][1] = y;
	quad[2][0] = x;
	quad[2][1] = y + h;
	quad[3][0] = x + w;
	quad[3][1] = y + h;
}


void rquad(float quad[4][2], color_t color)
{
	int idx[6] = {
		0, 1, 2,
		1, 3, 2
	};
	rtris(color, (float *)quad, 4, idx, 6);
}


void rrect(float x, float y, float w, float h, color_t color)
{
	float quad[4][2];
	rect2quad(x, y, w, h, quad);
	rquad(quad, color);
}


void rline(float x0, float y0, float x1, float y1, float thickness, color_t color)
{
	float normal[2];
	float quad[4][2];
	normal[0] = x0 - x1;
	normal[1] = y0 - y1;
	v2norm(normal);
	v2perp(normal);
	normal[0] *= thickness * 0.5f;
	normal[1] *= thickness * 0.5f;
	quad[0][0] = x0 - normal[0];
	quad[0][1] = y0 - normal[1];
	quad[1][0] = x0 + normal[0];
	quad[1][1] = y0 + normal[1];
	quad[2][0] = x1 - normal[0];
	quad[2][1] = y1 - normal[1];
	quad[3][0] = x1 + normal[0];
	quad[3][1] = y1 + normal[1];
	rquad(quad, color);
}


void rcircle(float x, float y, float r, color_t color)
{
	const int   steps = R_CIRCLE_STEPS;
	const float step  = ((float)M_PI * 2.0f) / steps;
	float vtx[R_CIRCLE_STEPS + 1][2];
	int   idx[R_CIRCLE_STEPS][3];
	/* center point */
	vtx[R_CIRCLE_STEPS][0] = x;
	vtx[R_CIRCLE_STEPS][1] = y;
	for(int i = 0; i < steps; i++) {
		vtx[i][0] = x + sinf(i * step) * r;
		vtx[i][1] = y + cosf(i * step) * r;
		/* think of pizza slices */
		idx[i][0] = steps;
		idx[i][1] = i;
		idx[i][2] = (i + 1) % steps;
	}
	rtris(color, (float *)vtx, steps + 1, (int *)idx, steps * 3);
}


static void rtext(int fid, float x, float y, color_t color, const char *msg, int len)
{
	float vtx[R_MAX_TEXT][4][2];
	float uvs[R_MAX_TEXT][4][2];
	int   idx[R_MAX_TEXT][6];
	struct font *f = &fontdata[fid];
	for(int i = 0; i < len; i++) {
		float range = (float)(f->flags & FONTRANGE_MASK);
		float ch0   = ((float)msg[i]) / range;
		rect2quad(0.0f, ch0, 1.0f, 1.0f / range, uvs[i]);
		rect2quad(x, y, f->width, f->height, vtx[i]);
		idx[i][0] = i * 4 + 0;
		idx[i][1] = i * 4 + 1;
		idx[i][2] = i * 4 + 2;
		idx[i][3] = i * 4 + 1;
		idx[i][4] = i * 4 + 3;
		idx[i][5] = i * 4 + 2;
		x += f->width;
	}
	rtextries(fid, color, (float *)vtx, (float *)uvs, len * 4, (int *)idx, len * 6);
}


int rprintf(int fid, float x, float y, color_t color, const char *fmt, ...)
{
	int  len;
	char buf[R_MAX_TEXT];
	va_list  args;
	va_start(args, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	if(len < 0) {
		return len;
	}
	x = floorf(x); 
	y = floorf(y);
	struct font *f = &fontdata[fid];
	switch(f->flags & FONTSTYLE_MASK)
	{
	case FONTSTYLE_OUTLINE:
		for(int j = 0; j < 8; j++) {
			float dx = (float)(j % 3 - 1);
			float dy = (float)(j / 3 - 1);
			rtext(fid, x + dx, y + dy, black, buf, len);
			/* i=8 is identical to the shadow routine */
			/* simply fallthrough at that point */
		}
		/* fallthrough */
	case FONTSTYLE_SHADOW:
		rtext(fid, x + 1, y + 1, black, buf, len);
		/* fallthrough */
	default:
		rtext(fid, x, y, color, buf, len);
	}
	return len;
}
