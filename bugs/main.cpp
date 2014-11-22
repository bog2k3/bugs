#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Shape2D.h"
#include "renderOpenGL/Text.h"
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
#include <sstream>

int main()
{
	std::cout << "bugs\n";

	if (!gltInit(800, 600, "Bugs"))
		return -1;

	Renderer renderer;
	Viewport vp1(0, 0, 800, 600);
	renderer.addViewport(&vp1);
	ObjectRenderContext renderContext(new Shape2D(&renderer), &vp1);
	GLText::initialize("data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ');

	World wld;
	wld.setRenderContext(renderContext);

	Physics physics(&wld);

	OperationsStack opStack(&vp1, &wld, &wld);
	GLFWInput::initialize(gltGetWindow());
	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));
	MouseObject mouse;
	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(&mouse, InputEvent::MB_LEFT)));

	Bone b = Bone(glm::vec2(0, 0), 0, 5.f, glm::vec2(0.6, 0.3f), glm::vec2(0), 0.f);
	Bone b1 = Bone(glm::vec2(0, -1), 0, 5.f, glm::vec2(0.3, 0.7f), glm::vec2(0), 0.f);
	Spring s(AttachPoint(b.getRigidBody(), glm::vec2(-0.2,-0.15)), AttachPoint(b1.getRigidBody(), glm::vec2(-0.1, 0.35)), 50, 0.1f);
	// Bone b2 = Bone(glm::vec2(0, -2), 0, 5.f, glm::vec2(0.3, 0.7f), glm::vec2(0), 0.f);
	// Spring s2(AttachPoint(b1.getRigidBody(), glm::vec2(-0.2,-0.15)), AttachPoint(b2.getRigidBody(), glm::vec2(-0.1, 0.35)), 50, 0.1f);
	wld.addObject(&b);
	//wld.addObject(&b1);
	// wld.addObject(&b2);
	//wld.addObject(new WorldObject(&s));
	// wld.addObject(new WorldObject(&s2));

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;

		opStack.update(dt);
		wld.updatePrePhysics(dt);
		physics.update(dt, true);
		wld.updatePostPhysics(dt);

		// draw builds the render queue
		wld.draw();

		// now we do the actual openGL render (which is independent of our world)
		gltBegin();
		renderer.render();
		std::stringstream ss;
		ss << "E(trans) = " << physics.getTranslationalEnergy() << ";  E(rot) = " << physics.getRotationalEnergy();
		GLText::print(ss.str().c_str(), 20, 20, 16);
		gltEnd();
	}

	delete renderContext.shape;

	return 0;
}
