/*
 * log.h
 *
 *  Created on: Nov 14, 2014
 *      Author: bogdan
 */

#ifndef LOG_H_
#define LOG_H_

#define _ENABLE_LOGGING_

#ifdef _ENABLE_LOGGING_

#include <iostream>
#include <string>
#include <ostream>

#define LOGPREFIX(PREF) logger_prefix logger_prefix_token(PREF);

#ifdef DEBUG
#define LOG(X) {\
	if (logger::getLogStream()) {\
		logger::writeprefix(*logger::getLogStream());\
		*logger::getLogStream() << X;\
	}\
	/* also put the log on stdout in DEBUG mode */\
	if (logger::getLogStream() != &std::cout) {\
		logger::writeprefix(std::cout);\
		std::cout << X;\
	}\
}
#else
#define LOG(X) {\
	if (logger::getLogStream()) {\
		logger::writeprefix(*logger::getLogStream());\
		*logger::getLogStream() << X;\
	}\
}
#endif

#define LOGNP(X) { if (*logger::getLogStream()) *logger::getLogStream() << X; }
#define LOGLN(X) LOG(X << "\n")
#define ERROR(X) {\
	for (auto stream : {&std::cerr, logger::getErrStream()}) {\
		if (!stream)\
			continue;\
		*stream << "[ERROR]";\
		logger::writeprefix(*stream);\
		*stream << X << "\n";\
	}\
}

#else
#define LOG(X)
#define LOGNP(X)
#define LOGLN(X)
#define ERROR(X)
#endif

#ifdef _ENABLE_LOGGING_

#include <deque>
#include <stack>

class logger {
public:
	static void writeprefix(std::ostream &stream);

	// returns old stream
	static std::ostream* setLogStream(std::ostream* newStream) {
		std::ostream* pOld = pLogStream_;
		pLogStream_ = newStream;
		return pOld;
	}
	// returns old stream
	static std::ostream* setAdditionalErrStream(std::ostream* newStream) {
		std::ostream* pOld = pErrStream_;
		pErrStream_ = newStream;
		return pOld;
	}

	static std::ostream* getLogStream() { return pLogStream_; }
	static std::ostream* getErrStream() { return pErrStream_; }

private:
	static std::ostream* pLogStream_;
	static std::ostream* pErrStream_;
	static std::deque<std::string> prefix_;
	static logger instance_;

	static void push_prefix(std::string prefix) { prefix_.push_back(prefix); }
	static void pop_prefix() { prefix_.pop_back(); }

	friend class logger_prefix;
};

class logger_prefix {
public:
	logger_prefix(std::string s) {
		logger::push_prefix(s);
	}
	~logger_prefix() {
		logger::pop_prefix();
	}
};

#endif // _ENABLE_LOGGING_

#endif /* LOG_H_ */
