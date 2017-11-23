#include <iostream>

#include "renderOpenGL/glToolkit.h"
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
#include "physics/PhysContactListener.h"
#include "physics/PhysDestroyListener.h"
#include "physics/PhysicsDebugDraw.h"
#include "math/math3D.h"
#include "OSD/ScaleDisplay.h"
#include "OSD/SignalViewer.h"
#include "OSD/EntityLabeler.h"
#include "GUI/GuiSystem.h"
#include "GUI/Window.h"
#include "GUI/controls/Button.h"
#include "GUI/controls/TextField.h"
#include "serialization/Serializer.h"
#include "serialization/objectTypes.h"
#include "session/SessionManager.h"
#include "session/PopulationManager.h"
#include "Infrastructure.h"

#include "Prototype.h"

#include "utils/log.h"
#include "utils/DrawList.h"
#include "utils/UpdateList.h"
#include "utils/rand.h"

#include "perf/marker.h"
#include "perf/results.h"
#include "perf/frameCapture.h"

#ifdef DEBUG
#include "entities/Bug.h"
#include "body-parts/Torso.h"
#include "body-parts/sensors/Nose.h"
#include "neuralnet/OutputSocket.h"
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

#define ENABLE_PROTOTYPING false

bool skipRendering = true;
bool updatePaused = false;
bool slowMo = false;
bool captureFrame = false;
b2World *pPhysWld = nullptr;
PhysicsDebugDraw *pPhysicsDraw = nullptr;

Prototype prototype;

template<> void draw(b2World* wld, RenderContext const &ctx) {
	wld->DrawDebugData();
}

template<> void update(b2World* wld, float dt) {
	PERF_MARKER_FUNC;
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
	} else if (ev.key == GLFW_KEY_F1) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			captureFrame = true;
	} else if (ev.type == InputEvent::EV_KEY_DOWN)
		prototype.onKeyDown(ev.key);
	 else if (ev.type == InputEvent::EV_KEY_UP)
		prototype.onKeyUp(ev.key);
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

// these are defined in perfPrint.cpp
void printFrameCaptureData(std::vector<perf::FrameCapture::frameData> data);
void printTopHits(std::vector<perf::sectionData> data);
void printCallTree(std::vector<std::shared_ptr<perf::sectionData>> t, int level);

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
	perf::setCrtThreadName("main");
	do {
		PERF_MARKER_FUNC;
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

		// initialize stuff:
		int winW = 1024, winH = 768;
		if (!gltInit(winW, winH, "Bugs"))
			return -1;

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		Renderer renderer(winW, winH);
		auto vp = std::make_unique<Viewport>(0, 0, winW, winH);
		auto vp1 = vp.get();
		renderer.addViewport("main", std::move(vp));
		RenderContext renderContext;

		b2ThreadPool b2tp(6);
		b2World physWld(b2Vec2_zero, &b2tp);
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

		OperationsStack opStack(vp1, World::getInstance(), &physWld);
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationPan(InputEvent::MB_RIGHT)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationGui(Gui)));

		//randSeed(1424118659);
		randSeed(time(NULL));
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
		SignalViewer sigViewer(
				{24, 4, ViewportCoord::percent, ViewportCoord::top|ViewportCoord::right},	// position
				-0.1f, 																		// z
				{20, 10, ViewportCoord::percent}); 											// size

		DrawList drawList;
		drawList.add(World::getInstance());
		drawList.add(&physWld);
		drawList.add(&scale);
		drawList.add(&sigViewer);
		drawList.add(&Gui);
		drawList.add(&EntityLabeler::getInstance());
		drawList.add(&prototype);

		UpdateList continuousUpdateList;
		continuousUpdateList.add(&opStack);

		UpdateList updateList;
		updateList.add(&physWld);
		updateList.add(&contactListener);
		updateList.add(&sessionMgr.getPopulationManager());
		updateList.add(World::getInstance());
		updateList.add(&sigViewer);
		updateList.add(&prototype);

		float realTime = 0;							// [s]
		float simulationTime = 0;					// [s]
		float lastPrintedSimTime = 0;				// [s]
		float simDTAcc = 0; // [s] accumulated sim dt values since last status print
		float realDTAcc = 0; // [s] accumulated real dt values since last status print
		constexpr float simTimePrintInterval = 10.f; // [s]

		constexpr float autoSaveInterval = 600.f; // 10 minutes of real time
		float lastAutosaveTime = 0;

		float frameTime = 0;

		sigViewer.addSignal("frameTime", &frameTime,
				glm::vec3(1.f, 0.2f, 0.2f), 0.1f, 50, 0.1, 0, 3);
		sigViewer.addSignal("population", [&] { return sessionMgr.getPopulationManager().getPopulationCount();},
				glm::vec3(0.3f, 0.3f, 1.f), 30.f, 50, sessionMgr.getPopulationManager().getPopulationTarget()+1, 0, 0);

	#ifdef DEBUG
		// Bug* pB = dynamic_cast<Bug*>(World::getInstance()->getEntities(EntityType::BUG)[0]);
		// static constexpr float neuronUpdateTime = 0.05f;
	//	sigViewer.addSignal("NoseL", &nl_out, glm::vec3(0.2f, 1.f, 0.2f), neuronUpdateTime, 50, 1.f, 0.f);
	//	sigViewer.addSignal("NoseR", &nr_out, glm::vec3(0.2f, 1.f, 0.2f), neuronUpdateTime, 50, 1.f, 0.f);
	//	sigViewer.addSignal("R>L", &rgtl, glm::vec3(0.1, 0.3, 1.f), neuronUpdateTime, 50, 1.1f, -1.1f);
	//	sigViewer.addSignal("gateL", &gate2, glm::vec3(0.1, 1.0, 0.3f), neuronUpdateTime, 50, 1.0f, 0);
	//	sigViewer.addSignal("gateR", &gate3, glm::vec3(0.1, 1.0, 0.3f), neuronUpdateTime, 50, 1.0f, 0);
	//	sigViewer.addSignal("sigma", &sigma, glm::vec3(0.7f, 1.f, 0.f), neuronUpdateTime, 50, 1.f, -1.f);
		//sigViewer.addSignal("1/max", &invmax, glm::vec3(0.7f, 1.f, 0.f), neuronUpdateTime, 50, 1.f, -1.f);

		std::function<void(float)> debugValues_update = [&] (float dt) {
			// neuron values:
	//		sigma = pB->getNeuronData(2);
	//		rgtl = pB->getNeuronData(3);
	//		gate2 = pB->getNeuronData(1);
	//		gate3 = pB->getNeuronData(5);
	//		invmax = pB->getNeuronData(6);
	//		// nose values:
	//		Torso* t = pB->getBody();
	//		if (t && t->getChildrenCount() >= 3) {
	//			nl_out = ((Nose*)t->getChild(1))->getOutputSocket(0)->debugGetCachedValue();
	//			nr_out = ((Nose*)t->getChild(3))->getOutputSocket(0)->debugGetCachedValue();
	//		}
		};
		updateList.add(&debugValues_update);
	#endif

		prototype.enable(ENABLE_PROTOTYPING);

		// initial update:
		updateList.update(0);

		float t = glfwGetTime();
		while (GLFWInput::checkInput()) {
			if (captureFrame)
				perf::FrameCapture::start(perf::FrameCapture::AllThreads);
			/* frame context */
			{
				PERF_MARKER("frame");
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
					PERF_MARKER("frame-update");
					updateList.update(simDT);
				}

				if (!skipRendering) {
					PERF_MARKER("frame-draw");
					// wait until previous frame finishes rendering and show frame output:
					gltEnd();
					// draw builds the render queue for the current frame
					drawList.draw(renderContext);

					GLText::get()->print("Salut Lume!\n[Powered by Box2D]",
							{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
							0, 16, glm::vec3(0.2f, 0.4, 1.0f));

					if (updatePaused) {
						GLText::get()->print("PAUSED",
								{50, 50, ViewportCoord::percent},
								0, 32, glm::vec3(1.f, 0.8f, 0.2f));
					}
					if (slowMo) {
						GLText::get()->print("~~ Slow Motion ON ~~",
								{10, 45},
								0, 18, glm::vec3(1.f, 0.5f, 0.1f));
					}

					// do the actual openGL render for the previous frame (which is independent of our world)
					gltBegin();
					renderer.render(renderContext);
					// now rendering is on-going, move on to the next update:
				}
			} /* frame context */

			if (captureFrame) {
				captureFrame = false;
				perf::FrameCapture::stop();
				printFrameCaptureData(perf::FrameCapture::getResults());
				perf::FrameCapture::cleanup();
			}
		}

		renderer.unload();
		Infrastructure::shutDown();
	} while (0);

	for (uint i=0; i<perf::Results::getNumberOfThreads(); i++) {
		std::cout << "\n=============Call Tree for thread [" << perf::Results::getThreadName(i) << "]==========================================\n";
		printCallTree(perf::Results::getCallTrees(i), 0);
		std::cout << "\n------------ TOP HITS -------------\n";
		printTopHits(perf::Results::getFlatList(i));
		std::cout << "\n--------------- END -------------------------------\n";
	}

	std::cout << "\n\n";

	return 0;
}
