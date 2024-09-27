#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include <font.h>
#include <r.h>


GLFWwindow *window = NULL;
static GLuint fontimpl[FONTID_NUM];


static void framebuffer_size_callback(GLFWwindow *w, int width, int height)
{
	(void)w;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, (GLdouble)width, (GLdouble)height, 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void rsize(float *w, float *h)
{
	int iw, ih;
	glfwGetWindowSize(window, &iw, &ih);
	*w = (float)iw;
	*h = (float)ih;
}


void rtris(color_t color, float *positions, int num_positions,
	int *indices, int num_indices)
{
	(void)num_positions;
	glDisable(GL_TEXTURE_2D);
	glColor4ub(color.r, color.g, color.b, color.a);
	glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, positions);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);

}

void rtextries(int texid,
	color_t color, float *positions, float *tex_coords,
	int num_vertices, int *indices, int num_indices)
{
	(void)num_vertices;
	GLuint t = fontimpl[texid];
	glEnable(GL_TEXTURE_2D);
	glColor4ub(color.r, color.g, color.b, color.a);
	glVertexPointer(2, GL_FLOAT, sizeof(float) * 2, positions);
	glTexCoordPointer(2, GL_FLOAT, sizeof(float) * 2, tex_coords);
	glBindTexture(GL_TEXTURE_2D, t);
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, indices);
}

void rclear(color_t color)
{
	GLclampf r = (GLfloat)color.r / 255.0f;
	GLclampf g = (GLfloat)color.g / 255.0f;
	GLclampf b = (GLfloat)color.b / 255.0f;
	GLclampf a = (GLfloat)color.a / 255.0f;

	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void rpresent(void)
{
	glfwSwapBuffers(window);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

int rinit(const char *title)
{
	if(glfwInit() != GLFW_TRUE) {
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	/* I'm not sure if this will work w/o OpenGL extensions */
	glfwWindowHint(GLFW_SAMPLES, 16);

	window = glfwCreateWindow(1280, 720, title, NULL, NULL);
	if(window == NULL) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	/* set callbacks. TODO move to inglfw.c */
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	framebuffer_size_callback(window, 1280, 720);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_SCISSOR_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel(GL_SMOOTH);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glGenTextures(FONTID_NUM, fontimpl);

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
				}
				else {
					rgba[(i * f->width + j) * 4 + 3] = 0x00;
				}
			}
		}

		glBindTexture(GL_TEXTURE_2D, fontimpl[fid]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, f->width, f->height * range, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

		free(rgba);
	}
	return 0;
bad:
	glfwDestroyWindow(window);
	glfwTerminate();
	return -1;
}

void rfree(void)
{
	glDeleteTextures(FONTID_NUM, fontimpl);
	glfwDestroyWindow(window);
	glfwTerminate();
}
