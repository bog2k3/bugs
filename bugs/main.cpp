#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Rectangle.h"
#include "renderOpenGL/Renderer.h"
#include "input/GLFWInput.h"
#include "input/InputEvent.h"
#include "input/OperationPan.h"
#include <functional>

OperationPan *opPan = nullptr;

int main()
{
	std::cout << "bugs\n";

	if (!gltInit(800, 600, "Bugs"))
		return -1;
	Renderer renderer;
	renderer.setScreenSize(800, 600);
	Rectangle::initialize(&renderer);

	GLFWInput::initialize(gltGetWindow());
	opPan = new OperationPan(&renderer);
	GLFWInput::setListener(std::bind(&OperationPan::handleInput, opPan, std::placeholders::_1));

	float t = 0.f;
	while (GLFWInput::checkInput()) {
		gltBegin();
		Rectangle::draw(0.5f, 0.5f, 0, 1.5f, 1.5f, t, 1, 0, 0);
		t += 0.05f;
		renderer.render();
		gltEnd();
	}

	return 0;
}
