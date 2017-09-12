#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>

class GLFWwindow;

// initializes openGL an' all
bool gltInit(unsigned windowWidth=512, unsigned windowHeight=512, const char windowTitle[]="Untitled");

// begins a frame
void gltBegin();

// finishes a frame and displays the result
void gltEnd();

// draws an image (sprite) in screen space
void gltDrawImg(int x, int y, unsigned width, unsigned height, GLenum format, GLenum type, const GLvoid * data);

GLFWwindow* gltGetWindow();

#endif //__glToolkit_h__
