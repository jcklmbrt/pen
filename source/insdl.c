#include <input.h>
#include <SDL2/SDL.h>


uint64_t getticks(void)
{
	return SDL_GetTicks64();
}


bool pollevent(union input *in, uint8_t *flags)
{
	int ret;
	SDL_Event sdlevent;
	*flags = IN_NONE;
	if((ret = SDL_PollEvent(&sdlevent))) {
		switch(sdlevent.type) {
		case SDL_QUIT:
			*flags = IN_QUIT;
			break;
		case SDL_MOUSEBUTTONUP:
			*flags = IN_MOUSE | IN_UP;
			/* fallthrough */
		case SDL_MOUSEBUTTONDOWN:
			/* accumulate flags */
			/* IN_MOUSE_UP | IN_MOUSE_DOWN = IN_MOUSE_UP */
			/* IN_QUIT/0   | IN_MOUSE_DOWN = IN_MOUSE_DOWN */
			*flags |= IN_MOUSE | IN_DOWN;
			in->mpos[0] = (float)sdlevent.button.x;
			in->mpos[1] = (float)sdlevent.button.y;
			if(sdlevent.button.button == SDL_BUTTON_LEFT) {
				*flags |= IN_L;
			} else if(sdlevent.button.button == SDL_BUTTON_RIGHT) {
				*flags |= IN_R;
			} else if(sdlevent.button.button == SDL_BUTTON_MIDDLE) {
				*flags |= IN_M;
			}
			break;
		case SDL_MOUSEMOTION:
			*flags = IN_MOUSE | IN_MOVE;
			in->mpos[0] = (float)sdlevent.motion.x;
			in->mpos[1] = (float)sdlevent.motion.y;
			if(sdlevent.motion.state & SDL_BUTTON_LMASK) {
				*flags |= IN_L;
			} else if(sdlevent.motion.state & SDL_BUTTON_RMASK) {
				*flags |= IN_R;
			} else if(sdlevent.motion.state & SDL_BUTTON_MMASK) {
				*flags |= IN_M;
			}
			break;
		case SDL_MOUSEWHEEL:
			*flags = IN_SCROLL | IN_DOWN;
			in->mpos[0] = (float)sdlevent.wheel.mouseX;
			in->mpos[1] = (float)sdlevent.wheel.mouseY;
			if(sdlevent.wheel.y > 0) {
				*flags |= IN_UP;
			}
			break;
		case SDL_KEYUP:
			*flags = IN_KEY | IN_UP;
			/* fallthrough */
		case SDL_KEYDOWN:
			/* same as mouse up/down */
			*flags |= IN_KEY | IN_DOWN;
			in->keycode = sdlevent.key.keysym.sym;
			break;
		default:
			break;
		}
	}
	return ret == 1;
}
