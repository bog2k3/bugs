/*
 * perfPrint.cpp
 *
 *  Created on: Dec 30, 2016
 *      Author: bog
 */

#include "perf/section.h"
#include "perf/frameCapture.h"
#include "utils/ioModif.h"

#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <map>

#include <unistd.h>
#include <sys/ioctl.h>

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
	std::vector<ulong> frameTimes;
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

void printFrameCaptureStatistics(std::vector<perf::FrameCapture::frameData> data) {
	std::cout << "============= FRAME CAPTURE STATS ================\n";
	std::cout << "Total Frame time: " << formatTime((data.back().endTime_-data.front().startTime_).count()) << "\n";
	std::cout << data.size() << " frames total\n";
	std::map<int, int> framesPerThread;
	for (auto &f : data)
		framesPerThread[f.threadIndex_]++;
	std::cout << "Frames distribution:\n";
	for (auto &p : framesPerThread)
		std::cout << "\tThread " << p.first << ": " << p.second << " frames\n";
	std::cout << "Average frames per thread: " << std::accumulate(framesPerThread.begin(), framesPerThread.end(), 0, [] (int x, auto &p) {
		return x + p.second;
	}) / framesPerThread.size() << "\n";
}

void printFrameCaptureData(std::vector<perf::FrameCapture::frameData> data) {
	//dumpFrameCaptureData(data);
	printFrameCaptureStatistics(data);
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
		while (td.callsEndTime.size() >= 2 && *(td.callsEndTime.end()-2) < relativeNano(f.endTime_))
			td.callsEndTime.pop_back();
		// check if this is a new level on the stack
		if (td.callsEndTime.empty() || relativeNano(f.endTime_) < td.callsEndTime.back())
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

