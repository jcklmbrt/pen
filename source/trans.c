#include <math.h>
#include <trans.h>
#include <input.h>
#include <immintrin.h>


float rsqrtf(float x)
{
	__m128 ss;
	ss = _mm_set_ss(x);
	ss = _mm_rsqrt_ss(ss);
	return _mm_cvtss_f32(ss);
}


#define MAX_SCALE  10.0f
#define MIN_SCALE  0.01f
#define MAX_PAN    10000.0f
#define MIN_PAN   -10000.0f


static float scale  = 1.0f;
static float pan[2] = { 0.0f, 0.0f };


/* a full 3x3 matrix is overkill, we just want panning and zooming */
/* ...but maybe we need one for opengl integration */
void w2s(float w[2], float s[2])
{
	float wx = w[0];
	float wy = w[1];
	s[0] = (wx - pan[0]) * scale;
	s[1] = (wy - pan[1]) * scale;
}


void s2w(float s[2], float w[2])
{
	float sx = s[0];
	float sy = s[1];
	w[0] = sx / scale + pan[0];
	w[1] = sy / scale + pan[1];
}


void transinput(union input *in, uint8_t flags)
{
	static float lst_mpos[2];
	switch(flags) {
	case IN_SCROLL | IN_DOWN:
		for(int i = 0; i < 2; i++) {
			float lst = in->mpos[i] /  scale         + pan[i];
			float cur = in->mpos[i] / (scale * 0.9f) + pan[i];
			if(pan[i] + lst - cur > MAX_PAN || pan[i] + lst - cur < MIN_PAN) {
				return;
			}
			pan[i] += lst - cur;
		}
		if(scale * 0.9f > MAX_SCALE) {
			break;
		}
		scale *= 0.9f;
		break;
	case IN_SCROLL | IN_UP:
		for(int i = 0; i < 2; i++) {
			float lst = in->mpos[i] /  scale         + pan[i];
			float cur = in->mpos[i] / (scale * 1.1f) + pan[i];
			if(pan[i] + lst - cur > MAX_PAN || pan[i] + lst - cur < MIN_PAN) {
				return;
			}
			pan[i] += lst - cur;
		}
		if(scale * 1.1f > MAX_SCALE) {
			break;
		}
		scale *= 1.1f;
		break;
	case IN_MOUSE | IN_DOWN | IN_M:
		lst_mpos[0] = in->mpos[0];
		lst_mpos[1] = in->mpos[1];
		break;
	case IN_MOUSE | IN_MOVE | IN_M:
		if(pan[0] - (in->mpos[0] - lst_mpos[0]) / scale > MAX_PAN) {
			break;
		}
		if(pan[1] - (in->mpos[1] - lst_mpos[1]) / scale > MAX_PAN) {
			break;
		}
		pan[0] -= (in->mpos[0] - lst_mpos[0]) / scale,
		pan[1] -= (in->mpos[1] - lst_mpos[1]) / scale;
		lst_mpos[0] = in->mpos[0];
		lst_mpos[1] = in->mpos[1];
		break;
	}
}


/* useful for calculating size, radius, etc... */
float gscale()
{
	return scale;
}


float lerp(float a, float b, float t)
{
	float nt = 1.0f - t;
	return a * nt + b * t;
}


/* unit vector */
void v2norm(float p[2])
{
	float invlen = rsqrtf(p[0] * p[0] + p[1] * p[1]);
	p[0] *= invlen;
	p[1] *= invlen;
}


/* clockwise perpendicular */
void v2perp(float p[2])
{
	float tmp = p[0];
	p[0] =  p[1];
	p[1] = -tmp;
}


/* |a - b| < lt */
/* useful for checking if mouse cursor is close to a point */
int v2dlt(float a[2], float b[2], float lt)
{
	float d, lensqr = 0.0f;
	for(int i = 0; i < 2; i++) {
		d = a[i] - b[i];
		lensqr += d * d;
	}
	return lensqr < lt * lt;
}


/* |a - b| */
float v2dlen(float a[2], float b[2])
{
	float d[2];
	for(int i = 0; i < 2; i++) {
		d[i] = a[i] - b[i];
	}
	return sqrtf(d[0] * d[0] + d[1] * d[1]);
}

