/*
 * GLFWInput.cpp
 *
 *  Created on: Nov 7, 2014
 *      Author: bog
 */

#include "GLFWInput.h"
#include "InputEvent.h"
#include <GLFW/glfw3.h>

std::function<void(InputEvent)> GLFWInput::listener = nullptr;
GLFWwindow* GLFWInput::window = nullptr;
bool GLFWInput::lastLeftDown = false;
bool GLFWInput::lastRightDown = false;
glm::vec2 GLFWInput::lastMousePos;


void GLFWInput::initialize(GLFWwindow* pWindow) {
	window = pWindow;
	glfwSetScrollCallback(window, &GLFWInput::glfwMouseScroll);
}

bool GLFWInput::checkInput() {
	InputEvent* ev = nullptr;

	/*
	 if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (keyboardCB != NULL)
			keyboardCB(GLFW_KEY_SPACE, true);
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
		if (!lastLeftDown) {
			lastLeftDown = true;
			if (mouseLeftCB != NULL) {
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				mouseLeftCB(x, y, glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
			}
		}
	} else
		lastLeftDown = false;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
		if (!lastRightDown) {
			lastRightDown = true;
			if (mouseRightCB != NULL) {
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				mouseRightCB(x, y, glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
			}
		}
	} else
		lastRightDown = false;

	scrollVal = 0;

	// Check if the ESC key was pressed or the window was closed
	*/
	glfwPollEvents();
	return glfwWindowShouldClose(window) == 0;
}

void GLFWInput::glfwMouseScroll(GLFWwindow* win, double x, double y) {

}
