#include "serialization/objectTypes.h"
#include "session/SessionManager.h"
#include "session/PopulationManager.h"
#include "Prototype.h"
#include "configFile.h"

#include "OperationBreakJoint.h"

#include "generator/Researcher.h"

#ifdef DEBUG
#include "entities/Bug/Bug.h"
#include "body-parts/sensors/Nose.h"
#include "neuralnet/OutputSocket.h"
#endif

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/input/GLFWInput.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/input/operations/OperationsStack.h>
#include <boglfw/input/operations/OperationPan.h>
#include <boglfw/input/operations/OperationSpring.h>
#include <boglfw/input/operations/OperationGui.h>
#include <boglfw/World.h>
#include <boglfw/physics/PhysContactListener.h>
#include <boglfw/physics/PhysDestroyListener.h>
#include <boglfw/physics/PhysicsDebugDraw.h>
#include <boglfw/math/math3D.h>
#include <boglfw/OSD/ScaleDisplay.h>
#include <boglfw/OSD/SignalViewer.h>
#include <boglfw/OSD/EntityLabeler.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/GUI/Window.h>
#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/TextField.h>
#include <boglfw/serialization/Serializer.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/DrawList.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/rand.h>
#include <boglfw/perf/marker.h>
#include <boglfw/perf/results.h>
#include <boglfw/perf/frameCapture.h>


#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <cstdio>

#include <sys/stat.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

namespace configNames {
	static auto constexpr disableMipMaps = "DISABLE_MIP_MAPS";
	static auto constexpr screenWidth = "screen_width";
	static auto constexpr screenHeight = "screen_height";
	static auto constexpr disableMultithreading = "DISABLE_MULTITHREADING";

/*
 * EXAMPLE CONFIG FILE - save as ~/.bugs.conf

# set to non-zero to disable mip-map generation (to avoid crashing in virtualBox VM)
# this will make text rendering lower quality
DISABLE_MIP_MAPS = 0

# set to non-zero to disable parallel processing for debug purposes. Performance will be lowered
DISABLE_MULTITHREADING = 0

screen_width = 1024
screen_height = 720

 */
};

#define ENABLE_PROTOTYPING false

bool skipRendering = true;
bool updatePaused = false;
bool slowMo = false;
bool captureFrame = false;
b2World *pPhysWld = nullptr;
PhysicsDebugDraw *pPhysicsDraw = nullptr;

Prototype prototype;

template<> void draw(b2World* wld, Viewport* vp) {
	PERF_MARKER_FUNC;
	wld->DrawDebugData();
}

template<> void update(b2World* wld, float dt) {
	PERF_MARKER_FUNC;
	wld->Step(dt, 5, 2);
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

void registerEventHandlers(World &world) {
	world.registerEventHandler("pauseRequested", [](int val) {
		updatePaused = val != 0;
	});
	world.registerEventHandler("slowMoRequested", [](int val) {
		slowMo = val != 0;
	});
}

void researchRun(std::string researchPath) {
	constexpr int targetPopulation = 50;
	constexpr float recombinationRatio = 0.25f;
	constexpr float renewRatio = 0.025f;
	constexpr int motorSampleFrames = 500;
	constexpr int randomGenomeLength = 200;
	constexpr float timeStep = 0.02f;

	LOGLN("Running in research mode....");
	LOGLN("[press ENTER to stop]");

	Researcher r(researchPath);
	r.initialize(targetPopulation, recombinationRatio, renewRatio, motorSampleFrames, randomGenomeLength);

	std::atomic_bool stop {false};
	Infrastructure::getThreadPool().queueTask([&] {
		while (std::cin.get() != '\n');
		stop.store(true, std::memory_order_release);
		LOGLN("Stop requested.");
	});

	while(!stop.load(std::memory_order_acquire)) {
		LOGPREFIX("Researcher")
		r.iterate(timeStep);
	}

	LOGLN("waiting for pool threads...");
	Infrastructure::getThreadPool().wait();

	LOGLN("Saving genomes...");
	r.saveGenomes();

	LOGLN("Session stats:");
	r.printStatistics();

	LOGLN("Done, exiting.");
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
#ifdef _ENABLE_LOGGING_
	logger::setLogLevel(LOG_LEVEL_DEBUG);
#endif
	do {
		PERF_MARKER_FUNC;
		// parse command line parameters:
		std::string loadFilename;
		std::string saveFilename;
		bool loadSession = false;
		bool defaultSession = false;
		bool saveSession = false;
		bool enableAutosave = false;
		bool runInResearchMode = false;
		std::string researchPath;
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
				// must save session
				saveSession = true;
				saveFilename = argv[i+1];
				i++;
			} else if (!strcmp(argv[i], "--enable-autosave")) {
				enableAutosave = true;
			} else if (!strcmp(argv[i], "--research")){
				runInResearchMode = true;
				if (i == argc-1) {
					ERROR("Expected path after --research");
					return -1;
				}
				researchPath = argv[++i];
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

		std::map<std::string, std::string> configOpts {
			{configNames::disableMipMaps, "0"},
			{configNames::screenWidth, "1024"},
			{configNames::screenHeight, "720"},
			{configNames::disableMultithreading, "0"},
		};
		// read config options from file:
		const char* pHomeDir = getenv("HOME");
		if (!pHomeDir) {
			ERROR("Could not access HOME directory! Config file will not be read!");
		} else {
			if (!parseConfigFile(std::string(pHomeDir) + "/.bugs.conf", configOpts, {})) {
				ERROR("Config file could not be read.");
			}
		}

		// initialize world and physics:
		b2ThreadPool b2tp(6);
		b2World physWld(b2Vec2_zero, &b2tp);
		pPhysWld = &physWld;
		PhysicsDebugDraw physicsDraw;
		if (!runInResearchMode) {
			pPhysicsDraw = &physicsDraw;
			physicsDraw.SetFlags(
						  b2Draw::e_shapeBit
						//| b2Draw::e_centerOfMassBit
						//| b2Draw::e_jointBit
						//| b2Draw::e_aabbBit
					);
			physWld.SetDebugDraw(&physicsDraw);
		}

		PhysContactListener contactListener;
		physWld.SetContactListener(&contactListener);

		PhysDestroyListener destroyListener;
		physWld.SetDestructionListener(&destroyListener);

		WorldConfig cfg;
		cfg.disableParallelProcessing = std::stoi(configOpts[configNames::disableMultithreading]) != 0;
		cfg.disableUserEvents = runInResearchMode;
		World::setConfig(cfg);
		World &world = World::getInstance();
		registerEventHandlers(world);

		world.setPhysics(&physWld);
		world.setDestroyListener(&destroyListener);

		randSeed(1517598343);
		//randSeed(time(NULL));
		LOGLN("RAND seed: "<<rand_seed);

		if (runInResearchMode) {
#ifdef _ENABLE_LOGGING_
			logger::setLogLevel(LOG_LEVEL_INFO);
#endif
			researchRun(researchPath);
			world.reset();
			Infrastructure::shutDown();
			return 0;
		}

		// initialize window and rendering:
		int winW = std::stoi(configOpts[configNames::screenWidth]);
		int winH = std::stoi(configOpts[configNames::screenHeight]);
		bool disableMipMaps = std::stoi(configOpts[configNames::disableMipMaps]) != 0;
		if (!gltInit(winW, winH, "Bugs"))
			return -1;

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		GLText::disableMipMaps(disableMipMaps);

		Renderer renderer(winW, winH);
		auto vp = std::make_unique<Viewport>(0, 0, winW, winH);
		auto vp1 = vp.get();
		renderer.addViewport("main", std::move(vp));

		// initialize GUI
		GuiSystem Gui;
		/*std::shared_ptr<Window> win1 = std::make_shared<Window>(glm::vec2(400, 10), glm::vec2(380, 580));
		std::shared_ptr<Window> win2 = std::make_shared<Window>(glm::vec2(300, 130), glm::vec2(350, 200));
		Gui.addElement(std::static_pointer_cast<IGuiElement>(win1));
		Gui.addElement(std::static_pointer_cast<IGuiElement>(win2));
		win1->addElement(std::make_shared<Button>(glm::vec2(100, 100), glm::vec2(60, 35), "buton1"));
		win1->addElement(std::make_shared<TextField>(glm::vec2(50, 170), glm::vec2(200, 40), "text"));*/

		OperationsStack opStack(vp1, &world, &physWld);
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationPan(InputEvent::MB_RIGHT)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationGui(Gui)));
		opStack.pushOperation(std::unique_ptr<IOperation>(new OperationBreakJoint(InputEvent::MB_MIDDLE)));

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

		ScaleDisplay scale(glm::vec2(15, 25), 0, 300);
		SignalViewer sigViewer(
				{24, 4, ViewportCoord::percent, ViewportCoord::top|ViewportCoord::right},	// position
				-1.f, 																		// z
				{20, 10, ViewportCoord::percent}); 											// size

		DrawList drawList;
		drawList.add(&world);
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
		updateList.add(&world);
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
		std::vector<Entity*> ents;
//		world.getEntities(ents, EntityType::BUG);
//		Bug* pB = dynamic_cast<Bug*>(ents[0]);
		static constexpr float metabUpdateTime = 1.f;
		static constexpr float quickMetabUpdateTime = 0.1f;
//		sigViewer.addSignal("cachedLeanMass", [pB]{return pB->getDebugValue("cachedLeanMass");}, {0.2f, 1.f, 0.2f}, metabUpdateTime, 50);
//		sigViewer.addSignal("growthMassBuffer", [pB]{return pB->getDebugValue("growthMassBuffer");}, {0.2f, 1.f, 0.2f}, quickMetabUpdateTime, 50, pB->getDebugValue("maxGrowthMassBuffer"));
//		sigViewer.addSignal("actualGrowthSpeed", [pB]{return pB->getDebugValue("actualGrowthSpeed");}, {0.2f, 1.f, 0.2f}, quickMetabUpdateTime, 50, 1.e-3f);
//		sigViewer.addSignal("eggGrowthSpeed", [pB]{return pB->getDebugValue("eggGrowthSpeed");}, {0.2f, 1.f, 0.2f}, metabUpdateTime, 50);
//		sigViewer.addSignal("fatGrowthSpeed", [pB]{return pB->getDebugValue("fatGrowthSpeed");}, {0.2f, 1.f, 0.2f}, quickMetabUpdateTime, 50);
//		sigViewer.addSignal("fatMass", [pB]{return pB->getDebugValue("fatMass");}, {0.2f, 1.f, 0.2f}, metabUpdateTime, 50);
//		sigViewer.addSignal("frameFoodProcessed", [pB]{return pB->getDebugValue("frameFoodProcessed");}, {0.2f, 1.f, 0.2f}, quickMetabUpdateTime, 50);
//		sigViewer.addSignal("frameEnergyUsed", [pB]{return pB->getDebugValue("frameEnergyUsed");}, {0.2f, 1.f, 0.2f}, quickMetabUpdateTime, 50);
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
		if (ENABLE_PROTOTYPING)
			prototype.initialize();

		auto infoTexts = [&] (Viewport*) {
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
		};
		drawList.add(&infoTexts);

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
					{
						PERF_MARKER_BLOCKED("GPU Sync Wait");
						// wait until previous frame finishes rendering and show frame output:
						gltEnd();
					}
					// do the drawing
					gltBegin();
					renderer.render(drawList);
					// now hardware rendering is on-going, move on to the next update:
				}
			} /* frame context */

			if (captureFrame) {
				captureFrame = false;
				perf::FrameCapture::stop();
				printFrameCaptureData(perf::FrameCapture::getResults());
				perf::FrameCapture::cleanup();
			}
		}

		if (ENABLE_PROTOTYPING)
			prototype.terminate();

		world.reset();
		renderer.unload();
		Infrastructure::shutDown();
	} while (0);

	for (unsigned i=0; i<perf::Results::getNumberOfThreads(); i++) {
		std::cout << "\n=============Call Tree for thread [" << perf::Results::getThreadName(i) << "]==========================================\n";
		printCallTree(perf::Results::getCallTrees(i), 0);
		std::cout << "\n------------ TOP HITS -------------\n";
		printTopHits(perf::Results::getFlatList(i));
		std::cout << "\n--------------- END -------------------------------\n";
	}

	std::cout << "\n\n";

	return 0;
}
