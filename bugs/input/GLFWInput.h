/*
 * GLFWInput.h
 *
 *  Created on: Nov 7, 2014
 *      Author: bog
 */

#ifndef INPUT_GLFWINPUT_H_
#define INPUT_GLFWINPUT_H_

#include <functional>
#include <glm/vec2.hpp>

class InputEvent;
class GLFWwindow;

class GLFWInput {
public:
	static void initialize(GLFWwindow* window);

	// returns true if application should continue, and false if it should shut down (user closed window)
	static bool checkInput();
	static void setListener(std::function<void(InputEvent)> listener) {
		GLFWInput::listener = listener;
	}

private:
	static void glfwMouseScroll(GLFWwindow* win,double x, double y);

	static std::function<void(InputEvent)> listener;
	static GLFWwindow *window;

	static bool lastLeftDown;
	static bool lastRightDown;
	static glm::vec2 lastMousePos;
};

#endif /* INPUT_GLFWINPUT_H_ */
