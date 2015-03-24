/*
 * GLFWInput.h
 *
 *  Created on: Nov 7, 2014
 *      Author: bog
 */

#ifndef INPUT_GLFWINPUT_H_
#define INPUT_GLFWINPUT_H_

#include "../utils/Event.h"
#include <functional>
#include <glm/vec2.hpp>
#include <vector>

class InputEvent;
class GLFWwindow;

class GLFWInput {
public:
	static void initialize(GLFWwindow* window);

	// returns true if application should continue, and false if it should shut down (user closed window)
	static bool checkInput();

	static Event<void(InputEvent&)> onInputEvent;

private:
	static void glfwMouseScroll(GLFWwindow* win,double x, double y);
	static void glfwMouseButton(GLFWwindow* win, int button, int action, int mods);
	static void glfwMousePos(GLFWwindow* win, double x, double y);
	static void glfwKey(GLFWwindow* win, int key, int scancode, int action, int mods);
	static void glfwChar(GLFWwindow* win, unsigned int ch);

	static std::vector<InputEvent> eventQueue;
	static GLFWwindow *window;
	static glm::vec2 lastMousePos;
};

#endif /* INPUT_GLFWINPUT_H_ */
