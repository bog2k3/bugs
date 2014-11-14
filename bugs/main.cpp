#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Shape2D.h"
#include "renderOpenGL/Renderer.h"
#include "renderOpenGL/Viewport.h"
#include "input/GLFWInput.h"
#include "input/InputEvent.h"
#include "input/operations/OperationsStack.h"
#include "input/operations/OperationPan.h"
#include "input/operations/OperationSpring.h"
#include "objects/body-parts/Bone.h"
#include "objects/MouseObject.h"
#include "physics/Spring.h"
#include "physics/Physics.h"
#include "World.h"
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

	ObjectRenderContext renderContext(new Shape2D(&renderer), &vp1);

	GLFWInput::initialize(gltGetWindow());
	OperationsStack opStack(&vp1, nullptr);
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));

	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));

	World wld;
	wld.setRenderContext(renderContext);

	Physics physics(&wld);

	Bone b = Bone(glm::vec2(0, 0), 0, 5.f, glm::vec2(1, 0.3f), glm::vec2(0), 0.f);
	wld.addObject(&b);

	MouseObject mouse;
	/*Spring s(
			AttachPoint(b.getRigidBody(),
				glm::vec2(0.5f, 0.15f)
			),
			AttachPoint(&mouse, glm::vec2(0)),
			10.f, // k
			0.01f // initialLength
			);
	wld.addObject(new WorldObject(&s));*/

	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(&mouse, InputEvent::MB_LEFT)));

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;

		opStack.update(dt);
		wld.updatePrePhysics(dt);
		physics.update(dt);
		wld.updatePostPhysics(dt);

		// draw builds the render queue
		wld.draw();

		// now we do the actual openGL render (which is independent of our world)
		gltBegin();
		renderer.render();
		gltEnd();
	}

	delete renderContext.shape;

	return 0;
}
