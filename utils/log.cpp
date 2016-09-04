/*
 * log.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: bogdan
 */

#include "log.h"

#ifdef _ENABLE_LOGGING_

#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <deque>

thread_local logger theInstance;
thread_local logger& logger::instance_ { theInstance };
std::atomic<std::ostream*> logger::pLogStream_ { &std::cout };
std::atomic<std::ostream*> logger::pErrStream_ { nullptr };
std::mutex logger::logMutex_;
std::mutex logger::errMutex_;

std::string formatCrtDateTime() {
	time_t t = time(0);   // get time now
	struct tm * now = localtime( & t );
	// output date & time in this format: "yyyy-mm-dd hh:mm:ss"
	std::stringstream s;
	s << 1900+now->tm_year << "-";
	s << std::setw(2) << std::setfill('0') << 1 + now->tm_mon << "-";
	s << std::setw(2) << std::setfill('0') << now->tm_mday << " ";
	s << std::setw(2) << std::setfill('0') << now->tm_hour << ":";
	s << std::setw(2) << std::setfill('0') << now->tm_min << ":";
	s << std::setw(2) << std::setfill('0') << now->tm_sec;

	return s.str();
}

void logger::writeprefix(std::ostream &stream) {
	// 1. write timestamp
	stream << "{" << formatCrtDateTime() << "} ";

	// 2. write logger name:
	if (prefix_.size())
		stream << "[";
	for (unsigned i=0, n=prefix_.size(); i<n; i++)
		stream << (i==0 ? "" : "::") << prefix_[i];
	if (prefix_.size())
		stream << "] ";
}

#endif // _ENABLE_LOGGING_
