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
#include "World.h"
#include "PhysContactListener.h"
#include "PhysicsDebugDraw.h"
#include "math/math2D.h"
#include "utils/log.h"
#include "entities/Bug.h"
#include "entities/food/FoodDispenser.h"
#include "entities/Wall.h"
#include "utils/DrawList.h"
#include "utils/UpdateList.h"
#include "OSD/ScaleDisplay.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include <functional>

bool skipRendering = false;

template<> void draw(b2World* wld, RenderContext const &ctx) {
	wld->DrawDebugData();
}

template<> void update(b2World* wld, float dt) {
	wld->Step(dt, 6, 2);
}

void onInputEventHandler(InputEvent& ev) {
	if (ev.key == GLFW_KEY_SPACE) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			skipRendering ^= true;
	}
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
	GLFWInput::onInputEvent.add(onInputEventHandler);
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));
	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));

	randSeed(1424118659);
	//randSeed(time(NULL));
	LOGLN("RAND seed: "<<rand_seed);

	float worldRadius = 5.f;

	Wall* w1 = new Wall(glm::vec2(-worldRadius, -worldRadius), glm::vec2(+worldRadius, -worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w1);
	Wall* w2 = new Wall(glm::vec2(-worldRadius, +worldRadius), glm::vec2(+worldRadius, +worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w2);
	Wall* w3 = new Wall(glm::vec2(-worldRadius, -worldRadius), glm::vec2(-worldRadius, +worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w3);
	Wall* w4 = new Wall(glm::vec2(+worldRadius, -worldRadius), glm::vec2(+worldRadius, +worldRadius), 0.2f);
	World::getInstance()->takeOwnershipOf(w4);

	for (int i=0; i<25; i++) {
		FoodDispenser* foodDisp = new FoodDispenser(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f)), 0);
		World::getInstance()->takeOwnershipOf(foodDisp);
	}

	for (int i=0; i<100; i++) {
		Bug* bug = Bug::newBasicMutantBug(glm::vec2(srandf()*(worldRadius-0.5f), srandf()*(worldRadius-0.5f)));
		//Bug* bug = Bug::newBasicBug(glm::vec2(0.f));
		World::getInstance()->takeOwnershipOf(bug);
	}

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

	float realTime = 0;							// [s]
	float simulationTime = 0;					// [s]
	float lastPrintedSimTime = 0;				// [s]
	constexpr float simTimePrintInterval = 10.f; // [s]

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float realDT = newTime - t;
		t = newTime;
		realTime += realDT;

		// fixed time step for simulation
		float simDT = 0.02f;

		simulationTime += simDT;
		if (simulationTime > lastPrintedSimTime+simTimePrintInterval) {
			LOGLN("SIMULATION TIME: " << simulationTime<<"\t real time: "<<realTime<<"\t instant ratio: "<<simDT/realDT<<"\t average ratio: "<<simulationTime/realTime);
			lastPrintedSimTime = simulationTime;
		}

		if (simDT > 0) {
			updateList.update(simDT);
		}

		if (!skipRendering) {
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
	}

	World::getInstance()->free();

	delete renderContext.shape;

	return 0;
}
