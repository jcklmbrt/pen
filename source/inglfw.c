#include <input.h>
#include <GLFW/glfw3.h>

extern GLFWwindow *window;

/* prob overkill */
#define MAX_EVENTS 128


static union input q_in[MAX_EVENTS];
static uint8_t     q_fl[MAX_EVENTS];
static uint8_t     head = 0;
static uint8_t     tail = 0;


static void pushevent(union input in, uint8_t flags)
{
	uint8_t next = (head + 1) & (MAX_EVENTS - 1);

	if(next != tail) {
		q_in[head] = in;
		q_fl[head] = flags;
		head = next;
	}
}


static int popevent(union input *in, uint8_t *flags)
{
	if(head != tail) {
		*in    = q_in[tail];
		*flags = q_fl[tail];
		tail   = (tail + 1) & (MAX_EVENTS - 1);
		return 1;

	} else {
		return 0;
	}
}


static void setmpos(float mpos[2])
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	mpos[0] = (float)x;
	mpos[1] = (float)y;
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void)window;
	(void)scancode;
	(void)mods;

	union input in;
	uint8_t flags = IN_KEY | IN_DOWN;

	in.keycode = key;
	if(action == GLFW_RELEASE) {
		flags |= IN_UP;
	} else if(action != GLFW_PRESS) {
		return;
	}

	pushevent(in, flags);
}


void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	(void)window;
	(void)mods;

	union input  in;
	enum  inflag flags = IN_MOUSE | IN_DOWN;

	setmpos(in.mpos);

	if(action == GLFW_RELEASE) {
		flags |= IN_UP;
	} else if (action != GLFW_PRESS) {
		return;
	}

	if(button == GLFW_MOUSE_BUTTON_LEFT) {
		flags |= IN_L;
	} else if(button == GLFW_MOUSE_BUTTON_RIGHT) {
		flags |= IN_R;
	} else if(button == GLFW_MOUSE_BUTTON_MIDDLE) {
		flags |= IN_M;
	}

	pushevent(in, flags);
}


void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	(void)window;
	(void)xoffset;

	union input  in;
	enum  inflag flags = IN_SCROLL | IN_DOWN;

	setmpos(in.mpos);

	if(yoffset > 0) {
		flags |= IN_UP;
	}

	pushevent(in, flags);
}


void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
	union input in;
	uint8_t flags = IN_MOUSE | IN_MOVE;

	in.mpos[0] = (float)xpos;
	in.mpos[1] = (float)ypos;

	/* Button callback doesn't give you the position
	   and position callback doesn't give you the button.
	   What a shit API! 
	   I'm also just pissed off in general because I have to implement my own queue */
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		flags |= IN_L;
	} else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		flags |= IN_R;
	} else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		flags |= IN_M;
	}

	if((flags & IN_M) == 0) {
		return;
	}

	pushevent(in, flags);
}


uint64_t getticks(void)
{
	return (uint64_t)(glfwGetTime() * 1000.0f);
}


int pollevent(union input *in, uint8_t *flags)
{
	(void)in;

	glfwPollEvents();

	if(glfwWindowShouldClose(window)) {
		*flags = IN_QUIT;
		return 1;
	}

	return popevent(in, flags);
}
