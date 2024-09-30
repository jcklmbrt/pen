#include <math.h>
#include <stdlib.h>
#include <float.h>

#include <font.h>
#include <trans.h>
#include <input.h>
#include <r.h>
#include <polybezier.h>


#define PB_CACHESIZE 64
#define PB_INVALID    0


enum pbflags {
	PB_SELECT_CTRL0 = 0,
	PB_SELECT_CTRL1 = 1 << 0,
	PB_SELECT_POINT = 1 << 1,
	PB_SELECT_MASK  = PB_SELECT_CTRL0 |
	                  PB_SELECT_CTRL1 |
	                  PB_SELECT_POINT,
	PB_MIRROR       = 1 << 2,
	PB_EDIT         = 1 << 3,
};


struct node {
	float point[2];
	float ctrl[2][2];
	float cache[PB_CACHESIZE][2];
	int   cachesize;
	float len;
	struct node *prev;
	struct node *next;
};


static struct node *head     = NULL;
static struct node *selected = NULL;
static unsigned int pbflags  = PB_EDIT;


static void cbinterp(float p[4][2], float t, float out[2])
{
	float nt       = 1.0f - t;
	float ntt      = nt * nt;
	float nttt     = nt * ntt;
	float tt       = t * t;
	float ttt      = t * tt;
	float _3_ntt_t = 3 * ntt * t;
	float _3_nt_tt = 3 * nt * tt;

	for(int i = 0; i < 2; i++) {
		out[i] = nttt     * p[0][i] +
		         _3_ntt_t * p[1][i] +
		         _3_nt_tt * p[2][i] +
		         ttt      * p[3][i];
	}
}


static void cbderiv(float p[4][2], float t, float out[2])
{
	float nt      = 1.0f - t;
	float _3_tt   = 3 * (t * t);
	float _3_ntt  = 3 * (nt * nt);
	float _6_nt_t = 6 * nt * t;

	for(int i = 0; i < 2; i++) {
		out[i] = _3_ntt  * (p[1][i] - p[0][i]) +
		         _6_nt_t * (p[2][i] - p[1][i]) +
		         _3_tt   * (p[3][i] - p[2][i]);
	}
}


static float cblen(float p[4][2])
{
	const float epsilon = 0.001f;
	float cur_point[2];
	float lst_point[2];
	float length = 0.0f;

	cbinterp(p, 0.0f, lst_point);

	for(float t = epsilon; t <= 1.0f; t += epsilon) {
		cbinterp(p, t, cur_point);
		length += v2dlen(cur_point, lst_point);
		lst_point[0] = cur_point[0];
		lst_point[1] = cur_point[1];
	}

	return length;
}


static float minlensqr(struct node *n, float point[2])
{
	float minls = FLT_MAX;
	if(n->cachesize != PB_INVALID) {
		for(int i = 0; i <= n->cachesize; i++) {
			float delta, lensqr = 0.0f;
			for(int j = 0; j < 2; j++) {
				delta = point[j] - n->cache[i][j];
				lensqr += delta * delta;
			}
			if(lensqr < minls) {
				minls = lensqr;
			}
		}
	} else {
		/* dont even bother */
		//mindist = delta_length_2d(point, n->point);
	}
	return minls;
}


int pbadd(float point[2])
{
	struct node *n;
	n = malloc(sizeof(struct node));
	if(n == NULL) {
		return -1;
	}
	n->prev = NULL;
	n->next = NULL;
	pbflags &= ~PB_SELECT_MASK;
	for(int i = 0; i < 2; i++) {
		n->point[i]   = point[i];
		n->ctrl[0][i] = point[i];
		n->ctrl[1][i] = point[i];
	}
	if(head == NULL) {
		head = n;
		selected = head;
	} else if(selected != NULL) {
		if(selected->next == NULL) {
			n->next = selected->next;
			n->prev = selected;
			selected->next = n;
			selected = n;
			pbflags |= PB_SELECT_CTRL0 | PB_MIRROR;
		} else if(selected->prev == NULL) {
			n->prev = selected->prev;
			n->next = selected;
			selected->prev = n;
			selected = n;
			head     = n; /* if prev is null then we are the head. */
			pbflags |= PB_SELECT_CTRL1 | PB_MIRROR;
		} else {
			float prevlensqr = minlensqr(selected->prev, point);
			float nextlensqr = minlensqr(selected,       point);
			if(prevlensqr < nextlensqr) {
				n->prev = selected->prev;
				n->next = selected;
				selected->prev->next = n;
				selected->prev = n;
				selected = n;
				pbflags |= PB_SELECT_CTRL1 | PB_MIRROR;
			} else {
				n->next = selected->next;
				n->prev = selected;
				selected->next->prev = n;
				selected->next = n;
				selected = n;
				pbflags |= PB_SELECT_CTRL0 | PB_MIRROR;
			}
		}
	}
	return 0;
}


void pbrm(struct node *n)
{
	/* make sure head is not a dangling pointer */
	if(n == head) {
		head = head->next;
		if(head != NULL) {
			head->prev = NULL;
		}
		free(n);
	} else if(n != NULL) {
		/* stitch up loose ends */
		if(n->prev != NULL) {
			n->prev->next = n->next;
		}
		if(n->next != NULL) {
			n->next->prev = n->prev;
		}
		free(n);
	}
}


void pbfree(void)
{
	struct node *n, *tmp = NULL;
	for(n = head; n != NULL; n = tmp) {
		tmp = n->next;
		pbrm(n);
	}
}


static void cbnorm(float p[4][2], float t, float out[2])
{
	cbderiv(p, t, out);
	v2norm(out);
	v2perp(out);
}


static int cbseg(float p[4][2], float t[2], float targetlen, float out[2])
{
	const float epsilon = 0.01f;
	float point[2];
	float start[2];
	float lo = t[0];
	float hi = t[1];
	cbinterp(p, t[0], start);
	while(lo < hi) {
		float mid = lo + (hi - lo) / 2;
		cbinterp(p, mid,  point);
		float len = v2dlen(start, point);
		if(fabsf(len - targetlen) < epsilon) {
			t[0]   = mid;
			out[0] = point[0];
			out[1] = point[1];
			return 0;
		}
		if(len > targetlen) {
			hi = mid;
		} else {
			lo = mid;
		}
	}
	return 1;
}


/* split curve into n equal pieces */
static int split(float p[4][2], float length, float out[][2], int n)
{
	float step = length / n;
	float t[2] = { 0.0f, 1.0f };
	int i = 0;
	while(t[0] < t[1] && i < n) {
		if(cbseg(p, t, step, out[i++])) {
			break;
		}
	}
	return i;
}


static int pb2cb(struct node *n, float p[4][2])
{
	if(n->next != NULL) {
		for(int i = 0; i < 2; i++) {
			p[0][i] = n->point[i];
			p[1][i] = n->ctrl[0][i];
			p[2][i] = n->next->ctrl[1][i];
			p[3][i] = n->next->point[i];
		}
	}
	return n->next != NULL;
}

/* cache path to next node */
static void cachenext(struct node *n)
{
	float p[4][2];
	if(pb2cb(n, p)) {
		/* excl. first and last */
		n->len       = cblen(p);
		n->cachesize = split(p, n->len, &n->cache[1], PB_CACHESIZE - 2);
		/* add first and last points */
		n->cache[0][0] = p[0][0];
		n->cache[0][1] = p[0][1];
		n->cache[n->cachesize][0] = p[3][0];
		n->cache[n->cachesize][1] = p[3][1];
	} else {
		n->cachesize = PB_INVALID;
	}
}

/* cache path to prev and next nodes */
static void cachenode(struct node *n)
{
	if(n != NULL) {
		cachenext(n);
		if(n->prev != NULL) {
			cachenext(n->prev);
		}
	}
}


float pblen(void)
{
	float length = 0.0f;
	struct node *n;
	for(n = head; n != NULL; n = n->next) {
		if(n->cachesize != PB_INVALID) {
			length += n->len;
		} else {
			cachenext(n);
			if(n->cachesize != PB_INVALID) {
				length += n->len;
			}
		}
	}
	return length;
}


bool pbinterp(float t, float out[2])
{
	struct node *n;
	float targetlen = t * pblen();
	float curlen    = 0.0f;
	for(n = head; n != NULL; n = n->next) {
		if(n->cachesize != PB_INVALID) {
			if(targetlen < curlen + n->len) {
				float nt = (targetlen - curlen) / n->len;
				int i = (int)floorf(n->cachesize * nt);
				float frac = n->cachesize * nt - (float)i;
				out[0] = lerp(n->cache[i][0], n->cache[i + 1][0], frac);
				out[1] = lerp(n->cache[i][1], n->cache[i + 1][1], frac);
				return true;
			}
			curlen += n->len;
		}
	}
	return false;
}


void cbplot(float p[4][2], float thickness, color_t color)
{
	const float epsilon = 0.01f; /* smallest possible step */
	const float mindp   = 0.99f; /* smallest dot product beteen 2 normals */
	const float start   = 0.0f + FLT_EPSILON;
	const float end     = 1.0f - FLT_EPSILON;
	float t = start;
	float cur_point[2],  lst_point[2];
	float cur_normal[2], lst_normal[2];
	while(t <= end) {
		cbinterp(p, t, cur_point);
		cbnorm(p, t, cur_normal);
		if(t != start) {
			float quad[4][2];
			float scale = thickness * 0.5f;
			for(int i = 0; i < 2; i++) {
				quad[0][i] = cur_point[i] - cur_normal[i] * scale;
				quad[1][i] = cur_point[i] + cur_normal[i] * scale;
				quad[2][i] = lst_point[i] - lst_normal[i] * scale;
				quad[3][i] = lst_point[i] + lst_normal[i] * scale;
			}
			rquad(quad, color);
		}
		/* last iter, step no further */
		if(t >= end) {
			break;
		}
		for(int i = 0; i < 2; i++) {
			lst_point[i]  = cur_point[i];
			lst_normal[i] = cur_normal[i];
		}
		float delta = (1.0f - t) * 0.5f;
		while(t + delta <= end) {
			cbnorm(p, t + delta, cur_normal);
			if(cur_normal[0] * lst_normal[0] +
			   cur_normal[1] * lst_normal[1] > mindp) {
				break;
			}
			if(delta <= epsilon) {
				break;
			}
			delta *= 0.5f;
		}
		t += delta;
	}
}

static const float point_radius = 10.0f;
static const float ctrl_radius  = 10.0f;

static bool lmousedown(float pos[2])
{
	struct node *n;
	for(n = head; n != NULL; n = n->next) {
		if(selected == n) {
			for(int i = 0; i < 2; i++) {
				if(v2dlt(n->ctrl[i], pos, ctrl_radius / gscale())) {
					selected = n;
					pbflags &= ~PB_SELECT_MASK;
					pbflags |= i & PB_SELECT_MASK;
					return true;
				}
			}
		}
		if(v2dlt(n->point, pos, point_radius / gscale())) {
			selected = n;
			return true;
		}
	}
	/* pbadd will set selected to the new node. */
	if(pbadd(pos) == 0) {
		cachenode(selected);
		return true;
	}
	return false;
}


static bool rmousedown(float pos[2])
{
	struct node *n;
	for(n = head; n != NULL; n = n->next) {
		if(v2dlt(n->point, pos, point_radius / gscale())) {
			selected = NULL;
			struct node *prev = n->prev;
			pbrm(n);
			if(prev != NULL) {
				cachenode(prev);
			}
			return true;
		}
	}
	return false;
}


static bool lmousemove(float pos[2])
{
	if(selected == NULL) {
		return false;
	}
	float d[2];
	unsigned int i;
	switch(pbflags & PB_SELECT_MASK) {
	case PB_SELECT_CTRL1:
	case PB_SELECT_CTRL0:
		i = pbflags & PB_SELECT_MASK;
		selected->ctrl[i][0] = pos[0];
		selected->ctrl[i][1] = pos[1];
		if(pbflags & PB_MIRROR) {
			d[0] = selected->point[0] - selected->ctrl[i][0];
			d[1] = selected->point[1] - selected->ctrl[i][1];
			selected->ctrl[!i][0] = selected->point[0] + d[0];
			selected->ctrl[!i][1] = selected->point[1] + d[1];
		}
		break;
	case PB_SELECT_POINT:
		d[0] = pos[0] - selected->point[0];
		d[1] = pos[1] - selected->point[1];
		selected->point[0] = pos[0];
		selected->point[1] = pos[1];
		for(int j = 0; j < 2; j++) {
			selected->ctrl[0][j] += d[j];
			selected->ctrl[1][j] += d[j];
		}
		break;
	}
	cachenode(selected);
	return true;
}


bool pbinput(union input *in, uint8_t flags)
{
	if(!(pbflags & PB_EDIT) || !(flags & IN_MOUSE)) {
		return false;
	}
	float wp[2];
	s2w(in->mpos, wp);
	switch(flags)
	{
	case IN_MOUSE | IN_DOWN | IN_L:
		return lmousedown(wp);
	case IN_MOUSE | IN_DOWN | IN_R:
		return rmousedown(wp);
	case IN_MOUSE | IN_UP | IN_L:
		pbflags |= PB_SELECT_POINT;
		pbflags &= ~(PB_SELECT_CTRL0 | PB_SELECT_CTRL1 | PB_MIRROR);
		return 1;
	case IN_MOUSE | IN_MOVE | IN_L:
		return lmousemove(wp);
	default:
		break;
	}
	return false;
}


void pbdraw(void)
{
	struct node *n;
	/* draw ctrl points */
	if(pbflags & PB_EDIT) {
		for(n = head; n != NULL; n = n->next) {
			float sp[2], sctrl[2][2];
			w2s(n->point, sp);
			w2s(n->ctrl[0], sctrl[0]);
			w2s(n->ctrl[1], sctrl[1]);
			if(n == selected) {
				for(int i = 0; i < 2; i++) {
					rline(sp[0], sp[1], sctrl[i][0], sctrl[i][1], 1.0f, black);
					rcircle(sctrl[i][0], sctrl[i][1], 4.0f, black);
					//rcircle(sctrl[i][0], sctrl[i][1], 2.0f, black);
				}
			}
		}
	}
	/* draw line */
	for(n = head; n != NULL; n = n->next) {
		float wp[4][2], sp[4][2];
		if(pb2cb(n, wp)) {
			for(int i = 0; i < 4; i++) {
				w2s(wp[i], sp[i]);
			}
			if(pbflags & PB_EDIT) {
				//cbplot(sp, 3.0f, blacktrans);
				cbplot(sp, 2.0f, black);
			} else {
				cbplot(sp, 5.0f, blacktrans);
				cbplot(sp, 4.0f, white);
			}
		}
	}
	/* draw points */
	if(pbflags & PB_EDIT) {
		for(n = head; n != NULL; n = n->next) {
			float sp[2];
			w2s(n->point, sp);
			rrect(sp[0] - 4.0f, sp[1] - 4.0f, 8.0f, 8.0f, black);
			//
			if(n == selected) {
				//
			} else {
				//
			}
		}
	}
}
