#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Shape2D.h"
#include "renderOpenGL/Renderer.h"
#include "renderOpenGL/Viewport.h"
#include "renderOpenGL/GLText.h"
#include "input/GLFWInput.h"
#include "input/InputEvent.h"
#include "input/operations/OperationsStack.h"
#include "input/operations/OperationPan.h"
#include "input/operations/OperationSpring.h"
#include "body-parts/Bone.h"
#include "body-parts/Joint.h"
#include "entities/food/FoodDispenser.h"
#include "World.h"
#include "PhysContactListener.h"
#include "PhysicsDebugDraw.h"
#include "math/math2D.h"
#include "utils/log.h"
#include "entities/Bug.h"
#include "utils/DrawList.h"
#include "utils/UpdateList.h"
#include "OSD/ScaleDisplay.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include <functional>

template<> void draw(b2World* wld, RenderContext const &ctx) {
	wld->DrawDebugData();
}

template<> void update(b2World* wld, float dt) {
	wld->Step(dt, 6, 2);
}

int main() {
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
				// | b2Draw::e_jointBit
				// | b2Draw::e_aabbBit
			);
	physWld.SetDebugDraw(&physicsDraw);

	PhysContactListener contactListener;
	physWld.SetContactListener(&contactListener);

	World::getInstance()->setPhysics(&physWld);

	OperationsStack opStack(&vp1, World::getInstance(), &physWld);
	GLFWInput::initialize(gltGetWindow());
	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));
	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));

	Bug* b1(Bug::newBasicBug(glm::vec2(0, 0)));
	World::getInstance()->takeOwnershipOf(b1);
	Bug* b2(Bug::newBasicBug(glm::vec2(0.4f, 0)));
	World::getInstance()->takeOwnershipOf(b2);
	Bug* b3(Bug::newBasicBug(glm::vec2(-0.4f, 0)));
	World::getInstance()->takeOwnershipOf(b3);
	Bug* b4(Bug::newBasicBug(glm::vec2(0, 0.4f)));
	World::getInstance()->takeOwnershipOf(b4);

	FoodDispenser* foodDisp1(new FoodDispenser(glm::vec2(-1, 0.5f), 0));
	World::getInstance()->takeOwnershipOf(foodDisp1);
	FoodDispenser* foodDisp2(new FoodDispenser(glm::vec2(+1, -0.5f), 0));
	World::getInstance()->takeOwnershipOf(foodDisp2);
	FoodDispenser* foodDisp3(new FoodDispenser(glm::vec2(0, -1.5f), 0));
	World::getInstance()->takeOwnershipOf(foodDisp3);

	DrawList drawList;
	drawList.add(World::getInstance());
	drawList.add(&physWld);
	ScaleDisplay scale(glm::vec2(15, 25), 300);
	drawList.add(&scale);

	UpdateList updateList;
	updateList.add(&opStack);
	updateList.add(&physWld);
	updateList.add(&contactListener);
	updateList.add(World::getInstance());

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;

		// override dt with fixed time step
		dt = 1.f / 60;

		if (dt > 0) {
			updateList.update(dt);
		}
		// wait until previous frame finishes rendering and show frame output:
		gltEnd();
		// draw builds the render queue for the current frame
		drawList.draw(renderContext);

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
