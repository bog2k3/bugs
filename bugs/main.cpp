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
#include "World.h"
#include "PhysContactListener.h"
#include "PhysDestroyListener.h"
#include "PhysicsDebugDraw.h"
#include "math/math2D.h"
#include "OSD/ScaleDisplay.h"
#include "OSD/SignalViewer.h"
#include "GUI/GuiSystem.h"
#include "GUI/Window.h"
#include "GUI/controls/Button.h"
#include "GUI/controls/TextField.h"
#include "serialization/Serializer.h"
#include "serialization/objectTypes.h"
#include "session/SessionManager.h"
#include "session/PopulationManager.h"
#include "utils/log.h"
#include "utils/DrawList.h"
#include "utils/UpdateList.h"
#include "utils/rand.h"

#ifdef DEBUG
#include "entities/Bug.h"
#endif

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <cstdio>

#include <sys/stat.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

bool skipRendering = true;
bool updatePaused = false;
bool slowMo = false;
b2World *pPhysWld = nullptr;
PhysicsDebugDraw *pPhysicsDraw = nullptr;

template<> void draw(b2World* wld, RenderContext const &ctx) {
	wld->DrawDebugData();
}

template<> void update(b2World* wld, float dt) {
	wld->Step(dt, 5, 2);
}

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

void onInputEventHandler(InputEvent& ev) {
	if (ev.isConsumed())
		return;
	if (ev.key == GLFW_KEY_R) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			skipRendering ^= true;
			pPhysWld->SetDebugDraw(skipRendering ? nullptr : pPhysicsDraw);
		}
	} else if (ev.key == GLFW_KEY_SPACE) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			updatePaused ^= true;
	} else if (ev.key == GLFW_KEY_S) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			slowMo ^= true;
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

#define FFMT(prec, X) std::fixed << std::setprecision(prec) << (X)
#define IFMT(spac, X) std::fixed << std::setprecision(0) << std::setw(spac) << (X)

void printStatus(float simulationTime, float realTime, float simDTAcc, float realDTAcc, int population, int generations) {
	LOGLN(	"SIM-TIME: " << IFMT(5, simulationTime)
			<< "\tREAL-time: "<< IFMT(5, realTime)
			<< "\tINST-MUL: " << FFMT(2, simDTAcc/realDTAcc)
			<< "\tAVG-MUL: " << FFMT(2, simulationTime/realTime)
			<< "\tPopulation: " << population
			<< "\tGenerations: " << generations);
}

int main(int argc, char* argv[]) {
	// parse command line parameters:
	std::string loadFilename;
	std::string saveFilename;
	bool loadSession = false;
	bool defaultSession = false;
	bool saveSession = false;
	bool enableAutosave = false;
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
		} else if (!strcmp(argv[i], "--enable-autosave")) {
			enableAutosave = true;
		} else {
			ERROR("Unknown argument " << argv[i]);
			return -1;
		}
	}
	if (!enableAutosave) {
		LOGLN("WARNING: Autosave is turned off! (use --enable-autosave to turn on)");
	}

#ifdef DEBUG
	updatePaused = true;
	skipRendering = false;
#endif

	try {
		// initialize stuff:
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

		ScaleDisplay scale(glm::vec3(15, 25, 0), 300);
		SignalViewer sigViewer(glm::vec3(0.75f, 0.1f, 1.f), glm::vec2(0.2f, 0.1f));

		DrawList drawList;
		drawList.add(World::getInstance());
		drawList.add(&physWld);
		drawList.add(&scale);
		drawList.add(&sigViewer);
		drawList.add(&Gui);

		UpdateList continuousUpdateList;
		continuousUpdateList.add(&opStack);
		UpdateList updateList;
		updateList.add(&physWld);
		updateList.add(&contactListener);
		updateList.add(&sessionMgr.getPopulationManager());
		updateList.add(World::getInstance());
		updateList.add(&sigViewer);

		float realTime = 0;							// [s]
		float simulationTime = 0;					// [s]
		float lastPrintedSimTime = 0;				// [s]
		float simDTAcc = 0; // [s] accumulated sim dt values since last status print
		float realDTAcc = 0; // [s] accumulated real dt values since last status print
		constexpr float simTimePrintInterval = 10.f; // [s]

		constexpr float autoSaveInterval = 600.f; // 10 minutes of real time
		float lastAutosaveTime = 0;

		float frameTime = 0;

		sigViewer.addSignal("frameTime", &frameTime, 50, 0.1f, glm::vec3(1.f, 0.2f, 0.2f));

#ifdef DEBUG
		Bug* pB = dynamic_cast<Bug*>(World::getInstance()->getEntities(EntityType::BUG)[0]);
		static constexpr float neuronUpdateTime = 0.05f;
		float n14_out = 0;
		float n14_i0=0, n14_i1=0;
		sigViewer.addSignal("N14#0", &n14_i0, 50, neuronUpdateTime, glm::vec3(0.2f, 1.f, 0.2f));
		sigViewer.addSignal("N14#1", &n14_i1, 50, neuronUpdateTime, glm::vec3(0.2f, 1.f, 0.2f));

		std::function<void(float)> debugNeurons_update = [&] (float dt) {
			n14_out = pB->getNeuronData(17);
			n14_i0 = pB->getNeuronData(6);
			n14_i1 = pB->getNeuronData(11);
		};
		updateList.add(&debugNeurons_update);
#endif

		float t = glfwGetTime();
		while (GLFWInput::checkInput()) {
			float newTime = glfwGetTime();
			float realDT = newTime - t;
			frameTime = realDT;
			realDTAcc += realDT;
			t = newTime;
			realTime += realDT;

			if (enableAutosave && realTime - lastAutosaveTime > autoSaveInterval) {
				LOGLN("Autosaving...");
				if (autosave(sessionMgr)) {
					LOGLN("Autosave successful.");
					lastAutosaveTime = realTime;
				} else {
					LOGLN("Autosave FAILED. Retrying in 10 seconds...");
					lastAutosaveTime = realTime - autoSaveInterval + 10;
				}
			}

			// fixed time step for simulation (unless slowMo is on)
			float simDT = updatePaused ? 0 : 0.02f;
			if (slowMo) {
				// use same fixed timestep in order to avoid breaking physics, but
				// only update once every n frames to slow down
				static float frameCounter = 0;
				constexpr float cycleLength = 10; // frames
				if (++frameCounter == cycleLength) {
					frameCounter = 0;
				} else
					simDT = 0;
			}

			simulationTime += simDT;
			simDTAcc += simDT;

			if (simulationTime > lastPrintedSimTime+simTimePrintInterval) {
				int population = sessionMgr.getPopulationManager().getPopulationCount();
				int maxGeneration = sessionMgr.getPopulationManager().getMaxGeneration();
				printStatus(simulationTime, realTime, simDTAcc, realDTAcc, population, maxGeneration);
				simDTAcc = realDTAcc = 0;
				lastPrintedSimTime = simulationTime;
			}

			continuousUpdateList.update(realDT);
			if (simDT > 0) {
				updateList.update(simDT);
			}

			if (!skipRendering) {
				// wait until previous frame finishes rendering and show frame output:
				gltEnd();
				// draw builds the render queue for the current frame
				drawList.draw(renderContext);

				renderContext.text->print("Salut Lume!\n[Powered by Box2D]", 20, vp1.getHeight()-20, 0, 16, glm::vec3(0.2f, 0.4, 1.0f));

				if (updatePaused) {
					renderContext.text->print("PAUSED", vp1.getWidth() / 2, vp1.getHeight() / 2, 0, 32, glm::vec3(1.f, 0.8f, 0.2f));
				}
				if (slowMo) {
					renderContext.text->print("~~ Slow Motion ON ~~", 10, 45, 0, 18, glm::vec3(1.f, 0.5f, 0.1f));
				}

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
