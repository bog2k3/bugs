#ifndef __glToolkit_h__
#define __glToolkit_h__

#define GLEW_NO_GLU
#include <GL/glew.h>
#include <functional>

typedef std::function<void (int key, bool pressed)> keyboardCallback;
typedef std::function<void (int x, int y, bool pressed)> mouseButtonCallback;
typedef std::function<void (float dx, float dy)> mouseMovedCallback;

// initializes openGL an' all
bool gltInit(unsigned windowWidth=512, unsigned windowHeight=512);

// sets a callback to be invoked when user presses/releases a key
void gltSetKeyboardCallback(keyboardCallback cb);

// sets a callback to be invoked when the left mouse is pressed/released
void gltSetLeftMouseButtonCallback(mouseButtonCallback cb);

// sets a callback to be invoked when the right mouse is pressed/released
void gltSetRightMouseButtonCallback(mouseButtonCallback cb);

// begins a frame
void gltBegin();

// finishes a frame and displays the result
void gltEnd();

// draws an image (sprite) in screen space
void gltDrawImg(int x, int y, unsigned width, unsigned height, GLenum format, GLenum type, const GLvoid * data);

// returns true if application should continue, and false if it should shut down (user closed window)
bool gltCheckInput();

// returns value of mouse wheel scroll since last gltCheckInput() call
double gltGetMouseScroll();

#endif //__glToolkit_h__
