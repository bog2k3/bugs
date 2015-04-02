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
#include "input/operations/OperationGui.h"
#include "body-parts/Bone.h"
#include "body-parts/Joint.h"
#include "World.h"
#include "PhysContactListener.h"
#include "PhysDestroyListener.h"
#include "PhysicsDebugDraw.h"
#include "math/math2D.h"
#include "utils/log.h"
#include "utils/DrawList.h"
#include "utils/UpdateList.h"
#include "OSD/ScaleDisplay.h"
#include "GUI/GuiSystem.h"
#include "GUI/Window.h"
#include "GUI/controls/Button.h"
#include "GUI/controls/TextField.h"
#include "serialization/Serializer.h"
#include "session/SessionManager.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include <functional>
#include <stdexcept>

bool skipRendering = false;
b2World *pPhysWld = nullptr;
PhysicsDebugDraw *pPhysicsDraw = nullptr;

template<> void draw(b2World* wld, RenderContext const &ctx) {
	wld->DrawDebugData();
}

template<> void update(b2World* wld, float dt) {
	wld->Step(dt, 6, 2);
}

void onInputEventHandler(InputEvent& ev) {
	if (ev.isConsumed())
		return;
	if (ev.key == GLFW_KEY_SPACE) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			skipRendering ^= true;
			pPhysWld->SetDebugDraw(skipRendering ? nullptr : pPhysicsDraw);
		}
	}
}

void save(const std::string &path) {
	Serializer serializer;
	auto vecSer = World::getInstance()->getEntities(Entity::FF_SERIALIZABLE);
	for (auto e : vecSer)
		serializer.queueObject(e);
	serializer.serializeToFile(path);
}

void autosave() {
#warning TODO: cycle autosave files first
	save("autosave.bin");
}

int main() {
	LOGGER("app_main");

	Serializer::setDeserializationObjectMapping(SerializationObjectTypes::BUG, &Bug::deserialize);

	try {
		if (!gltInit(800, 600, "Bugs"))
			return -1;

		Renderer renderer;
		Viewport vp1(0, 0, 800, 600);
		renderer.addViewport(&vp1);
		auto shape2d = new Shape2D(&renderer);
		auto gltext = new GLText(&renderer, "data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ', 22);
		RenderContext renderContext( &vp1, shape2d, gltext);

		b2World physWld(b2Vec2_zero);
		pPhysWld = &physWld;
		PhysicsDebugDraw physicsDraw(renderContext);
		pPhysicsDraw = &physicsDraw;
		physicsDraw.SetFlags(
					  b2Draw::e_shapeBit
					//| b2Draw::e_centerOfMassBit
					//| b2Draw::e_jointBit
					//| b2Draw::e_aabbBit
				);
		physWld.SetDebugDraw(&physicsDraw);

		PhysContactListener contactListener;
		physWld.SetContactListener(&contactListener);

		PhysDestroyListener destroyListener;
		physWld.SetDestructionListener(&destroyListener);

		World::getInstance()->setPhysics(&physWld);
		World::getInstance()->setDestroyListener(&destroyListener);

		GuiSystem Gui;
		/*std::shared_ptr<Window> win1 = std::make_shared<Window>(glm::vec2(400, 10), glm::vec2(380, 580));
		std::shared_ptr<Window> win2 = std::make_shared<Window>(glm::vec2(300, 130), glm::vec2(350, 200));
		Gui.addElement(std::static_pointer_cast<IGuiElement>(win1));
		Gui.addElement(std::static_pointer_cast<IGuiElement>(win2));
		win1->addElement(std::make_shared<Button>(glm::vec2(100, 100), glm::vec2(60, 35), "buton1"));
		win1->addElement(std::make_shared<TextField>(glm::vec2(50, 170), glm::vec2(200, 40), "text"));*/

		OperationsStack opStack(&vp1, World::getInstance(), &physWld);
		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationPan(InputEvent::MB_RIGHT)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationGui(Gui)));

		randSeed(1424118659);
		//randSeed(time(NULL));
		LOGLN("RAND seed: "<<rand_seed);

		SessionManager sessionMgr;
		sessionMgr.startDefaultSession();

		autosave();
		sessionMgr.loadSessionFromFile("autosave.bin");

		DrawList drawList;
		drawList.add(World::getInstance());
		drawList.add(&physWld);
		ScaleDisplay scale(glm::vec3(15, 25, 0), 300);
		drawList.add(&scale);
		drawList.add(&Gui);

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
				renderContext.text->print(ss.str().c_str(), 20, vp1.getHeight()-20, 0, 16, glm::vec3(0.2f, 0.4, 1.0f));

				// do the actual openGL render for the previous frame (which is independent of our world)
				gltBegin();
				renderer.render();
				// now rendering is on-going, move on to the next update:
			}
		}

		World::getInstance()->free();

		delete renderContext.shape;
	} catch (std::runtime_error &e) {
		ERROR("EXCEPTION: " << e.what());
		throw e;
	} catch (...) {
		ERROR("EXCEPTION (unknown)");
		throw;
	}

	return 0;
}
