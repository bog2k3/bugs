#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Rectangle.h"
#include "renderOpenGL/Renderer.h"
#include "input/OperationPan.h"

OperationPan *opPan = nullptr;

void onMouseLeftButton(bool pressed) {
	InputEvent ev(pressed ? InputEvent::EV_MOUSE_DOWN : InputEvent::EV_MOUSE_UP,
			0, 0, 0, 0, ....
	opPan->handleInput(ev);
}

int main()
{
	std::cout << "bugs\n";

	if (!gltInit(800, 600))
		return -1;
	Renderer renderer;
	renderer.setScreenSize(800, 600);
	Rectangle::initialize(&renderer);

	float t = 0.f;
	while (gltCheckInput()) {
		gltBegin();
		Rectangle::draw(0.5f, 0.5f, 0, 1.5f, 1.5f, t, 1, 0, 0);
		t += 0.05f;
		renderer.render();
		gltEnd();
	}

	return 0;
}
