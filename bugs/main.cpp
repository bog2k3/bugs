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
#include "objects/body-parts/Joint.h"
#include "World.h"
#include "PhysicsDebugDraw.h"
#include "math/math2D.h"
#include "log.h"
#include "entities/Bug.h"
#include "DrawList.h"
#include "UpdateList.h"
#include "objects/OSD/ScaleDisplay.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include "renderOpenGL/GLText.h"

int main()
{
	LOGGER("app_main");

	if (!gltInit(800, 600, "Bugs"))
		return -1;

	Renderer renderer;
	Viewport vp1(0, 0, 800, 600);
	renderer.addViewport(&vp1);
	RenderContext renderContext(
			&vp1,
			new Shape2D(&renderer),
			new GLText(&renderer, "data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ', 22));

	b2World physWld(b2Vec2_zero);
	PhysicsDebugDraw physicsDraw(renderContext);
	physicsDraw.SetFlags(
				  b2Draw::e_shapeBit
				//| b2Draw::e_centerOfMassBit
				| b2Draw::e_jointBit
				//| b2Draw::e_aabbBit
			);
	physWld.SetDebugDraw(&physicsDraw);

	World::getInstance()->setPhysics(&physWld);

	OperationsStack opStack(&vp1, World::getInstance(), World::getInstance(), &physWld);
	GLFWInput::initialize(gltGetWindow());
	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));
	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));

	Bug* b = Bug::newBasicBug(glm::vec2(0, 0));

	UpdateList updateList;
	updateList.add(b);

	DrawList drawList;
	drawList.add(World::getInstance());
	drawList.add(ScaleDisplay(glm::vec2(15, 25), 150));

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;

		if (dt > 0) {
			opStack.update(dt);
			physWld.Step(dt, 6, 2);
			updateList.update(dt);
		}
		// wait until previous frame finishes rendering and show frame output:
		gltEnd();

		// draw builds the render queue for the current frame
		drawList.draw(renderContext);
		physWld.DrawDebugData();

		std::stringstream ss;
		ss << "Salut Lume!\n[Powered by Box2D]";
		renderContext.text->print(ss.str().c_str(), 20, vp1.getHeight()-20, 16, glm::vec3(0.2f, 0.4, 1.0f));

		// do the actual openGL render for the previous frame (which is independent of our world)
		gltBegin();
		renderer.render();
		// now rendering is on-going, move on to the next update:
	}

	delete renderContext.shape;

	return 0;
}
