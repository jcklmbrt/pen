#define _USE_MATH_DEFINES
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <polybezier.h>
#include <trans.h>
#include <ent.h>

#define UINT_WIDTH (sizeof(unsigned int) * 8)

/* entities moving along the bezier curve */
static struct ent entities[MAX_ENTITIES];
static unsigned int active[MAX_ENTITIES / UINT_WIDTH] = { 0 };


#define MAX_OBS  1024
#define MAX_GUNS  256


struct gun {
	float   pos[2];
	float   size;
	color_t color;
	float   angvel;
	float   bulvel;
};


struct lvl {
	const char *name;
	const char *desc;
	float start[2][2];
	float end[2][2];
	float obs[MAX_OBS][2][2];
	size_t num_obs;
	struct gun guns[MAX_GUNS];
	size_t num_guns;
};


struct lvl levels[] = {
	/* name  */ "Level 0",
	/* desc  */ "This is the first level",
	/* start */ 100.0f, 100.0f, 200.0f, 200.0f,
	/* end   */ 300.0f, 100.0f, 400.0f, 200.0f,
	/* obs   */ {200.0f, 100.0f, 300.0f, 200.0f},  1,
	/* guns  */ {{{400.0f, 400.0f}, 10.0f, {0, 0, 255, 255}, M_PI_4, 10.0f},
	             {{300.0f, 400.0f}, 10.0f, {255, 50, 50, 255}, M_PI_4, 10.0f}}, 2
};

static size_t curlvl = 0;

static bool entisactive(size_t i)
{
	size_t index = i / UINT_WIDTH;
	size_t nbits = i % UINT_WIDTH;

	return active[index] & 1 << nbits;
}


static void entkill(size_t i)
{
	size_t index = i / UINT_WIDTH;
	size_t nbits = i % UINT_WIDTH;

	active[index] &= ~(1 << nbits);
}


static struct ent *entspawn(size_t i)
{
	size_t index = i / UINT_WIDTH;
	size_t nbits = i % UINT_WIDTH;

	active[index] |= 1 << nbits;

	struct ent *e = &entities[i];

	e->color.r = rand() & 255;
	e->color.g = rand() & 255;
	e->color.b = rand() & 255;
	e->color.a = 255;

	e->vel  = ((float)rand() / RAND_MAX) * 10.0f + 1.0f;
	e->size = ((float)rand() / RAND_MAX) * 10.0f + 1.0f;
	e->pos  = 0.0f;
}


void entmv(float dt)
{
	float len = pblen();
	for(size_t i = 0; i < MAX_ENTITIES; i++) {
		if(!entisactive(i)) {
			continue;
		}

		struct ent *e = &entities[i];

		e->pos += e->vel * dt;

		if(e->pos > len) {
			entkill(i);
		}
	}
}


static bool enttarget(const float pos[2], float projspeed, float *outangle)
{
	float len       = pblen();
	float minlensqr = FLT_MAX;
	for(size_t i = 0; i < MAX_ENTITIES; i++) {
		if(!entisactive(i)) {
			continue;
		}

		struct ent *e = &entities[i];
		float lensqr, epos[2], delta[2];
		pbinterp(e->pos / len, epos);

		delta[0] = pos[0] - epos[0];
		delta[1] = pos[1] - epos[1];
				
		lensqr = delta[0] * delta[0] + 
		         delta[1] * delta[1];

		if(lensqr < minlensqr) {
			float steps = sqrtf(lensqr) / projspeed;
			float npos  = e->pos + steps * e->vel;

			/* try to aim ahead */
			float nextpos[2];
			pbinterp(npos / len, nextpos);

			delta[0] = pos[0] - nextpos[0];
			delta[1] = pos[1] - nextpos[1];

			if(npos < len) {
				minlensqr = lensqr;
			}

			*outangle = M_PI + atan2f(delta[1], delta[0]);
		}
	}

	return minlensqr != FLT_MAX;
}


void raabb(float aabb[2][2], color_t color)
{
	float mins[2];
	float maxs[2];

	w2s(aabb[0], mins);
	w2s(aabb[1], maxs);

	rrect(mins[0],  mins[1],
	      maxs[0] - mins[0],
	      maxs[1] - mins[1],
	      color);
}


#define MAX_BULLETS 256
static float bullets[MAX_BULLETS][2];
static float bullet_angles[MAX_BULLETS];
static int num_bullets = 0;
const float bulletspeed = 10.0f;

static float theta[MAX_GUNS];
static float cooldown[MAX_GUNS];

const float speed     = 0.5f;
const float cannonlen = 20.0f;

void lvlmv(float dt)
{
	struct lvl *lvl = &levels[curlvl];

	for(size_t i = 0; i < lvl->num_guns; i++) {
		struct gun *g = &lvl->guns[i];
		float closest;
		if(enttarget(g->pos, bulletspeed, &closest)) {
			float step = speed * dt;
			float diff = remainderf(closest - theta[i], M_PI * 2.0f);

			diff *= 2.0f;

			if(fabsf(diff) <= step) {
				theta[i] = closest;
				/* shoot */
				if(cooldown[i] <= 0.0f) {
					//bullets[num_bullets][0] = g->pos[0] + cosf(closest) * cannonlen;
					//bullets[num_bullets][1] = g->pos[1] + sinf(closest) * cannonlen;
					bullets[num_bullets][0] = g->pos[0];
					bullets[num_bullets][1] = g->pos[1];
					bullet_angles[num_bullets] = closest;
					num_bullets = (num_bullets + 1) & (MAX_BULLETS - 1);
					cooldown[i] = 5.0f;
				}
			}
			else if(diff > 0) {
				theta[i] += step;
			}
			else {
				theta[i] -= step;
			}
		}
		cooldown[i] -= dt;
	}


	for(int i = 0; i < num_bullets; i++) {
		bullets[i][0] += cosf(bullet_angles[i]) * dt * bulletspeed;
		bullets[i][1] += sinf(bullet_angles[i]) * dt * bulletspeed;
	}
}


void lvldraw(void)
{
	struct lvl *lvl = &levels[curlvl];

	for(int i = 0; i < num_bullets; i++) {
		float bpos[2];
		w2s(bullets[i], bpos);
		rcircle(bpos[0], bpos[1], 2.0f * gscale(), red);
	}

	for(size_t i = 0; i < lvl->num_guns; i++) {
		struct gun *g = &lvl->guns[i];
		float r = g->size * gscale();
		float pos[2];
		w2s(g->pos, pos);
		rcircle(pos[0], pos[1], r + 1.0f, black);
		rcircle(pos[0], pos[1], r + 0.0f, g->color);

		float cannon[2];
		cannon[0] = g->pos[0] + cosf(theta[i]) * cannonlen;
		cannon[1] = g->pos[1] + sinf(theta[i]) * cannonlen;

		w2s(cannon, cannon);
		rline(pos[0], pos[1], cannon[0], cannon[1], 5.0f * gscale() + 2.0f, black);
		rline(pos[0], pos[1], cannon[0], cannon[1], 5.0f * gscale(), gray);
	}

	raabb(lvl->start, green);
	raabb(lvl->end, red);

	for(int i = 0; i < lvl->num_obs; i++) {
		raabb(lvl->obs[i], ltblue);
	}

	//rrect(80.0f, 80.0f, 200.0f, 80.0f, blacktrans);
	//rprintf(TERMINUS_8x16, 100.0f, 100.0f, white, "%s", lvl->name);
	//rprintf(TERMINUS_6x12, 100.0f, 116.0f, white, "%s", lvl->desc);
}


void entdraw(void)
{
	float len = pblen();
	for(size_t i = 0; i < MAX_ENTITIES; i++) {
		if(!entisactive(i)) {
			continue;
		}
		if(len == 0.0f) {
			return;
		}

		struct ent *e = &entities[i];

		float pos[2];
		pbinterp(e->pos / len, pos);

		w2s(pos, pos);

		float r = e->size * gscale();

		rcircle(pos[0], pos[1], r + 2.0f, black);
		rcircle(pos[0], pos[1], r + 0.0f, e->color);
	}
}


bool entadd(void)
{
	for(size_t i = 0; i < MAX_ENTITIES; i++) {
		if(!entisactive(i)) {
			entspawn(i);
			return true;
		}
	}

	return false;
}
