#include <stdio.h>
#include <assert.h>
#include <SDL2/SDL.h>


#include <font.h>
#include <r.h>


static SDL_Renderer *renderer = NULL;
static SDL_Window   *window   = NULL;
static SDL_Texture  *fontimpl[FONTID_NUM];


void rsize(float *w, float *h)
{
	int iw, ih;
	SDL_GetWindowSize(window, &iw, &ih);
	*w = (float)iw;
	*h = (float)ih;
}


void rtris(color_t color, float *positions, int num_positions,
    int *indices, int num_indices)
{
	SDL_RenderGeometryRaw(renderer, NULL, positions,
	    sizeof(float) * 2, (SDL_Color *)&color, 0, NULL, 0, num_positions,
	    indices, num_indices, 4);
}

void rtextries(int texid,
    color_t color, float *positions, float *tex_coords,
    int num_vertices, int *indices, int num_indices)
{
	SDL_Texture *t = fontimpl[texid];
	SDL_RenderGeometryRaw(renderer, t,
		positions, sizeof(float) * 2,
		(SDL_Color *)&color, 0,
		tex_coords, sizeof(float) * 2,
		num_vertices, indices, num_indices, 4);
}

void rclear(color_t color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(renderer);
}

void rpresent(void)
{
	SDL_RenderPresent(renderer);
}

int rinit(const char *title)
{
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		goto bad_sdl;
	}
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	window = SDL_CreateWindow(title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		1280,
		720,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if(window == NULL) {
		goto bad_sdl;
	}

	renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED |
		SDL_RENDERER_PRESENTVSYNC
	);

	if(renderer == NULL) {
		goto bad_sdl;
	}

	if(SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) != 0) {
		fprintf(stdout, "Failed to enable alpha channel\n");
	}

	for(int fid = 0; fid < FONTID_NUM; fid++) {
		struct font *f = &fontdata[fid];
		unsigned char *rgba = malloc(f->size * 8 * 4);
		if(rgba == NULL) {
			fprintf(stderr, "Failed to allocate memory for font: %dx%d, %8p", (int)f->width, (int)f->height, f->data);
			goto bad;
		}
		unsigned int range = f->flags & FONTRANGE_MASK;
		for(int i = 0; i < f->size; i++) {
			for(int j = 0; j < 8; j++) {
				rgba[(i * f->width + j) * 4 + 0] = 0xFF;
				rgba[(i * f->width + j) * 4 + 1] = 0xFF;
				rgba[(i * f->width + j) * 4 + 2] = 0xFF;
				if(f->data[i] & 1 << (8 - j)) {
					rgba[(i * f->width + j) * 4 + 3] = 0xFF;
				} else {
					rgba[(i * f->width + j) * 4 + 3] = 0x00;
				}
			}
		}
		SDL_Texture *t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, f->width, f->height * range);
		SDL_UpdateTexture(t, NULL, rgba, f->width * 4);
		SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
		fontimpl[fid] = t;
		free(rgba);
	}
	return 0;
bad_sdl:
	fprintf(stderr, "\n%s\n", SDL_GetError());
bad:
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return -1;
}

void rfree(void)
{
	for(int fid = 0; fid < FONTID_NUM; fid++) {
		SDL_DestroyTexture(fontimpl[fid]);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
