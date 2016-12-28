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
#include "physics/PhysContactListener.h"
#include "physics/PhysDestroyListener.h"
#include "physics/PhysicsDebugDraw.h"
#include "math/math2D.h"
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

#include "utils/log.h"
#include "utils/DrawList.h"
#include "utils/UpdateList.h"
#include "utils/rand.h"
#include "utils/ioModif.h"

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
#include <thread>
#include <chrono>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

bool skipRendering = true;
bool updatePaused = false;
bool slowMo = false;
bool captureFrame = false;
b2World *pPhysWld = nullptr;
PhysicsDebugDraw *pPhysicsDraw = nullptr;

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

std::string formatTime(uint64_t val, int mul=1) {
	static const char* suffix[] = {
		" ns", " us", " ms", " s"
	};
	std::stringstream str;
	if (val >= 1000000) {
		return formatTime(val / 1000, mul+1);
	} else if (val >= 1000) {
		str << val/1000 << "." << val%1000 << suffix[mul];
	} else {
		str << val << suffix[mul-1];
	}
	return str.str();
}


void printCallFrame(perf::sectionData const& s, bool flatMode=false) {
	bool missingInfo = s.getExclusiveNanosec() > s.getInclusiveNanosec() / 10; // more than 10% unknown
	if (s.getInclusiveNanosec() < 1e9)
		missingInfo = false; // this is not significant

	if (s.isDeadTime())
		std::cout << ioModif::BG_RGB(80,60,60) << ioModif::DARK;

	std::cout << ioModif::BOLD << ioModif::FG_LIGHT_YELLOW << s.getName() << ioModif::FG_DEFAULT << ioModif::NO_BOLD;
	if (s.isDeadTime())
		std::cout << ioModif::BG_RGB(80,60,60) << ioModif::DARK;
	std::cout << "    {"
		<< "calls " << s.getExecutionCount() << " | "
		<< "inc " << ioModif::FG_LIGHT_GREEN << formatTime(s.getInclusiveNanosec()) << ioModif::FG_DEFAULT << " | ";
	if (!flatMode)
		std::cout << "exc " << (missingInfo ? ioModif::FG_RED : ioModif::FG_DEFAULT)
				<< formatTime(s.getExclusiveNanosec()) << ioModif::FG_DEFAULT << " | ";
	std::cout << "avg-inc " << formatTime(s.getInclusiveNanosec() / s.getExecutionCount()) << " | ";
	if (!flatMode)
		std::cout << "avg-exc " << formatTime(s.getExclusiveNanosec() / s.getExecutionCount());
	std::cout << "}" << ioModif::RESET;
}

void printCallTree(std::vector<std::shared_ptr<perf::sectionData>> t, int level) {
	std::sort(t.begin(), t.end(), [](auto &x, auto &y) {
		return x->getInclusiveNanosec() > y->getInclusiveNanosec();
	});
	const auto tab = "    ";
	for (auto &s : t) {
		for (int i=0; i<level; i++) {
			std::cout<<"|" << tab;
		}
		std::cout << "|--";
		printCallFrame(*s);
		std::cout << "\n";
		printCallTree(s->getCallees(), level+1);
	}
}

void printTopHits(std::vector<perf::sectionData> data) {
	std::sort(data.begin(), data.end(), [](auto &x, auto &y) {
		return x.getInclusiveNanosec() > y.getInclusiveNanosec();
	});
	const size_t maxHits = 6;
	for (unsigned i=0; i<min(maxHits, data.size()); i++) {
		std::cout << i << ": ";
		printCallFrame(data[i], true);
		std::cout << "\n";
	}
}

void dumpFrameCaptureData(std::vector<perf::FrameCapture::frameData> data) {
	auto referenceTime = data.front().startTime_;
	// convert any time point into relative amount of nanoseconds since start of frame
	auto relativeNano = [referenceTime] (decltype(data[0].startTime_) &pt) -> int64_t {
		return std::chrono::nanoseconds(pt - referenceTime).count();
	};
	for (auto &f : data) {
		std::cout << "FRAME " << f.name_ << "\n\t" << "thread: " << f.threadIndex_ << "\tstart: "
				<< relativeNano(f.startTime_)/1000 << "\tend: " << relativeNano(f.endTime_)/1000 << "\n";
	}
}

void printFrameCaptureData(std::vector<perf::FrameCapture::frameData> data) {
	dumpFrameCaptureData(data);
	auto referenceTime = data.front().startTime_;
	// convert any time point into relative amount of nanoseconds since start of frame
	auto relativeNano = [referenceTime] (decltype(data[0].startTime_) &pt) -> int64_t {
		return std::chrono::nanoseconds(pt - referenceTime).count();
	};
	// compute metrics:
	int64_t timeSpan = relativeNano(data.back().endTime_);
	struct winsize sz;
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&sz);
	auto lineWidth = sz.ws_col - 10;
	if (lineWidth <= 0)
		lineWidth = 80; // asume default
	double cellsPerNanosec = (lineWidth-1.0) / timeSpan;

	// build visual representation
	struct threadData {
		std::vector<std::stringstream> str;
		std::vector<int> strOffs;
		std::map<std::string, uint> legend;
		std::vector<unsigned> callsEndTime;

		threadData() = default;
		threadData(threadData &&t) = default;
	};
	struct rgbColor {
		int r, g, b;
		operator ioModif::BG_RGB() {
			return ioModif::BG_RGB(r, g, b);
		}
		operator ioModif::FG_RGB() {
			return ioModif::FG_RGB(r*1.5, g*1.5, b*1.5);
		}
	} colors[] = {
		{50,50,150},
		{0,150,0},
		{150,150,0},
		{150,0,0},
		{150,0,150},
	};
	auto colorsCount = sizeof(colors)/sizeof(colors[0]);
	std::vector<threadData> threads;
	for (auto &f : data) {
		while (threads.size() <= f.threadIndex_)
			threads.push_back(threadData());
		threadData &td = threads[f.threadIndex_];
		// see if this frame appeared before in this thread:
		if (td.legend.find(f.name_) == td.legend.end())
			td.legend[f.name_] = td.legend.size();
		// check if need to pop a stack level
		if (td.callsEndTime.size() >= 2 && *(td.callsEndTime.end()-2) < relativeNano(f.startTime_))
			td.callsEndTime.pop_back();
		// check if this is a new level on the stack
		if (td.callsEndTime.empty() || relativeNano(f.startTime_) < td.callsEndTime.back())
			td.callsEndTime.push_back(relativeNano(f.endTime_));
		else {
			td.callsEndTime.back() = relativeNano(f.endTime_);
		}
		int frameID = td.legend[f.name_];
		while (td.str.size() < td.callsEndTime.size()) {
			td.str.push_back(std::stringstream());
			td.strOffs.push_back(0);
		}
		auto& crtStr = td.str[td.callsEndTime.size()-1];
		auto& crtStrOffs = td.strOffs[td.callsEndTime.size()-1];
		// add spaces before this call:
		int startOffs = relativeNano(f.startTime_) * cellsPerNanosec;
		int endOffs = relativeNano(f.endTime_) * cellsPerNanosec;
		int spaceCells = max(0, startOffs - crtStrOffs);
		crtStr << std::string(spaceCells, ' ');
		crtStrOffs += spaceCells;
		// write this call:
		if (endOffs > startOffs) {
			crtStr << ioModif::RESET << (((ioModif::BG_RGB)colors[f.threadIndex_ % colorsCount]) * (f.deadTime_? 0.5 : 1))
					<< ioModif::BOLD << (f.deadTime_ ? ioModif::FG_GRAY : ioModif::FG_WHITE)
					<< (char)('A' + frameID - 1);
			int callCells = max(0, (int)(endOffs - crtStrOffs - 1));
			crtStr << std::string(callCells, ' ') << ioModif::RESET;
			crtStrOffs += callCells + 1;
		}
	}

	// print stats
	for (unsigned i=0; i<threads.size(); i++) {
		auto &t = threads[i];
		std::cout << (ioModif::FG_RGB)colors[i % colorsCount];
		std::cout << ">>>>>>>>>>>>>>>>>>> Thread ["
				<< perf::FrameCapture::getThreadNameForIndex(i)
				<< "] >>>>>>>>>>>>>>>>>\n";
		// print calls:
		for (int i=t.str.size()-1; i>=0; --i)
			std::cout << t.str[i].str() << "\n";
	}
	// print legend
	std::cout << ioModif::RESET << "\n";
	for (unsigned i=0; i<threads.size(); i++) {
		auto &t = threads[i];
		std::cout << (ioModif::FG_RGB)colors[i % colorsCount];
		std::vector<std::string> legend;
		for (auto &p : t.legend) {
			while (legend.size() <= p.second)
				legend.push_back("");
			legend[p.second] = p.first;
		}
		for (unsigned i=1; i<legend.size(); i++)
			std::cout << ioModif::BOLD << (char)('A' + i - 1) << ioModif::NO_BOLD << " - " << legend[i] << "\n";
	}
	std::cout << ioModif::RESET << "\n\n";
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
		if (!gltInit(800, 600, "Bugs"))
			return -1;

		Renderer renderer;
		Viewport vp1(0, 0, 800, 600);
		renderer.addViewport(&vp1);
		auto shape2d = new Shape2D(&renderer);
		auto gltext = new GLText(&renderer, "data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ', 22);
		RenderContext renderContext( &vp1, shape2d, gltext);

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
		drawList.add(&EntityLabeler::getInstance());

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

		sigViewer.addSignal("frameTime", &frameTime,
				glm::vec3(1.f, 0.2f, 0.2f), 0.1f);
		sigViewer.addSignal("population", [&] { return sessionMgr.getPopulationManager().getPopulationCount();},
				glm::vec3(0.3f, 0.3f, 1.f), 5.f);

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

		// initial update:
		updateList.update(0);

		float t = glfwGetTime();
		while (GLFWInput::checkInput()) {
			if (captureFrame)
				perf::FrameCapture::start(perf::FrameCapture::ThisThreadOnly);
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
			} /* frame context */

			if (captureFrame) {
				captureFrame = false;
				perf::FrameCapture::stop();
				printFrameCaptureData(perf::FrameCapture::getResults());
				perf::FrameCapture::cleanup();
			}
		}

		delete renderContext.shape;
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
