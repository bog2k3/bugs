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
#include "serialization/objectTypes.h"
#include "session/SessionManager.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include <functional>
#include <stdexcept>
#include <cstdio>

#include <sys/stat.h>

bool skipRendering = true;
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

bool autosave(SessionManager &sessionMgr) {
	struct stat buffer;
	bool primary_exists(stat("autosave.bin", &buffer) == 0);
	bool secondary_exists(stat("autosave.prev.bin", &buffer) == 0);

	// cycle files to always keep the last two autosaves
	if (secondary_exists)
		std::rename("autosave.prev.bin", "autosave.prev.bin.0");
	if (primary_exists)
		std::rename("autosave.bin", "autosave.prev.bin");

	if (sessionMgr.saveSessionToFile("autosave.bin")) {
		// success, remove old backup file:
		if (secondary_exists)
			std::remove("autosave.prev.bin.0");
		return true;
	} else {
		// failure, put back backup files
		std::remove("autosave.bin");
		std::rename("autosave.prev.bin", "autosave.bin");
		if (secondary_exists)
			std::rename("autosave.prev.bin.0", "autosave.prev.bin");
		return false;
	}
}

int main(int argc, char* argv[]) {
	LOGGER("app_main");
	// parse command line parameters:
	std::string loadFilename;
	std::string saveFilename;
	bool loadSession = false;
	bool defaultSession = false;
	bool saveSession = false;
	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i], "--load")) {
			if (defaultSession) {
				ERROR("--default and --load cannot be used together.");
				return -1;
			}
			if (i == argc-1) {
				ERROR("Expected filename after --load");
				return -1;
			}
			// must load session
			loadSession = true;
			loadFilename = argv[i+1];
			i++;
		} else if (!strcmp(argv[i], "--default")) {
			if (loadSession) {
				ERROR("--default and --load cannot be used together.");
				return -1;
			}
			defaultSession = true;
		} else if (!strcmp(argv[i], "--save")) {
			if (i == argc-1) {
				ERROR("Expected filename after --save");
				return -1;
			}
			// must load session
			saveSession = true;
			saveFilename = argv[i+1];
			i++;
		} else {
			ERROR("Unknown argument " << argv[i]);
			return -1;
		}
	}
	// initialize stuff:
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

		World world;

		world.setPhysics(&physWld);
		world.setDestroyListener(&destroyListener);

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

		if (defaultSession)
			sessionMgr.startDefaultSession();
		else if (loadSession) {
			if (!sessionMgr.loadSessionFromFile(loadFilename)) {
				ERROR("Could not load session from file \""<<loadFilename<<"\"");
				return -1;
			}
		}
		else {
			LOGLN("No parameters specified. Starting with empty session.");
		}

		if (saveSession) {
			if (!sessionMgr.saveSessionToFile(saveFilename))
				ERROR("Could not save session to file \"" << saveFilename << "\"");
		}

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

		constexpr float autoSaveInterval = 120.f; // 2 minutes of sym time
		float lastAutosaveTime = 0;

		float t = glfwGetTime();
		while (GLFWInput::checkInput()) {
			float newTime = glfwGetTime();
			float realDT = newTime - t;
			t = newTime;
			realTime += realDT;

			if (simulationTime - lastAutosaveTime > autoSaveInterval) {
				LOGLN("Autosaving...");
				if (autosave(sessionMgr)) {
					LOGLN("Autosave successful.");
					lastAutosaveTime = simulationTime;
				} else {
					LOGLN("Autosave FAILED. Retrying in 10 seconds...");
					lastAutosaveTime = simulationTime - autoSaveInterval + 10;
				}
			}

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
