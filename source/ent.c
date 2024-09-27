#include <stdbool.h>
#include <polybezier.h>
#include <trans.h>
#include <ent.h>
#include <r.h>

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

	float r = 10.0f * gscale();

	rcircle(pos[0], pos[1], r + 2.0f, black);
	rcircle(pos[0], pos[1], r + 0.0f, blue);
	rprintf(TERMINUS_8x16, pos[0], pos[1], white, "e:%d", i);
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

	e->pos = 0.0f;
	e->vel = 1.0f;

	active[index] |= 1 << nbits;

	return true;
}
