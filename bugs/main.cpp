#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Rectangle.h"
#include "renderOpenGL/Renderer.h"
#include "renderOpenGL/Viewport.h"
#include "input/GLFWInput.h"
#include "input/InputEvent.h"
#include "input/operations/OperationsStack.h"
#include "input/operations/OperationPan.h"
#include <GLFW/glfw3.h>
#include <functional>

int main()
{
	std::cout << "bugs\n";

	if (!gltInit(800, 600, "Bugs"))
		return -1;

	Renderer renderer;
	Viewport vp1(0, 0, 800, 600);
	renderer.addViewport(&vp1);

	Rectangle* rc = new Rectangle(&renderer);
	renderer.registerRenderable(rc);

	GLFWInput::initialize(gltGetWindow());
	OperationsStack opStack(&vp1, nullptr);
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan()));

	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;
		opStack.update(dt);
		gltBegin();
		renderer.render();
		gltEnd();
	}

	delete rc;

	return 0;
}
