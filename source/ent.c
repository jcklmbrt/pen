#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

#include <polybezier.h>
#include <trans.h>
#include <ent.h>

#define UINT_WIDTH (sizeof(unsigned int) * 8)


static struct ent entities[MAX_ENTITIES];
static unsigned int active[MAX_ENTITIES / UINT_WIDTH] = { 0 };


static void entkill(size_t i)
{
	size_t index = i / UINT_WIDTH;
	size_t nbits = i % UINT_WIDTH;

	active[index] &= ~(1 << nbits);
}


static bool nextinactive(size_t *index, size_t *nbit)
{
	for(size_t i = 0; i < MAX_ENTITIES / UINT_WIDTH; i++) {
		for(size_t j = 0; j < UINT_WIDTH; j++) {
			if((active[i] & 1 << j) == 0) {
				*nbit  = j;
				*index = i;
				return true;
			}
		}
	}
	return false;
}


static void mv(size_t i, float len, float dt)
{
	struct ent *e = &entities[i];

	e->pos += e->vel * dt;

	if(e->pos > len) {
		entkill(i);
	}
}


static void draw(size_t i, float len)
{
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


void entmv(float dt)
{
	float len = pblen();
	for(size_t i = 0; i < MAX_ENTITIES / UINT_WIDTH; i++) {
		for(size_t j = 0; j < UINT_WIDTH; j++) {
			if(active[i] & 1 << j) {
				mv(i * UINT_WIDTH + j, len, dt);
			}	
		}
	}
}


void lvldraw()
{
	float start[2][2];
	float end[2][2];

	start[0][0] = 100.0f;
	start[0][1] = 100.0f;
	start[1][0] = 300.0f;
	start[1][1] = 300.0f;

	end[0][0] = 400.0f;
	end[0][1] = 400.0f;
	end[1][0] = 500.0f;
	end[1][1] = 500.0f;

	float mins[2];
	float maxs[2];

	w2s(start[0], mins);
	w2s(start[1], maxs);

	rrect(mins[0],  mins[1], 
	      maxs[0] - mins[0], 
	      maxs[1] - mins[1], 
	      green);

	w2s(end[0], mins);
	w2s(end[1], maxs);

	rrect(mins[0],  mins[1], 
	      maxs[0] - mins[0], 
	      maxs[1] - mins[1], 
	      red);
}


void entdraw(void)
{
	float len = pblen();
	for(size_t i = 0; i < MAX_ENTITIES / UINT_WIDTH; i++) {
		for(size_t j = 0; j < UINT_WIDTH; j++) {
			if(active[i] & 1 << j) {
				draw(i * UINT_WIDTH + j, len);
			}
		}
	}
}


bool entadd(void)
{
	size_t index, nbits;

	if(!nextinactive(&index, &nbits)) {
		return false;
	}

	size_t i = index * UINT_WIDTH + nbits;

	struct ent *e = &entities[i];

	e->color.r = rand() & 255;
	e->color.g = rand() & 255;
	e->color.b = rand() & 255;
	e->color.a = 255;

	e->vel  = ((float)rand() / RAND_MAX) * 10.0f + 1.0f;
	e->size = ((float)rand() / RAND_MAX) * 10.0f + 1.0f;
	e->pos  = 0.0f;

	active[index] |= 1 << nbits;

	return true;
}
