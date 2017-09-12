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
#include <deque>
#include <ostream>
#include <string>
#include <mutex>
#include <atomic>

#define LOGPREFIX(PREF) logger_prefix logger_prefix_token(PREF);

#define LOGIMPL(X) {if (logger::instance().getLogStream()) {\
	std::lock_guard<std::mutex> lk(logger::getLogMutex());\
	logger::instance().writeprefix(*logger::instance().getLogStream());\
	*logger::instance().getLogStream() << X;\
}}

#ifdef DEBUG
#define LOG(X) { LOGIMPL(X)\
	/* also put the log on stdout in DEBUG mode */\
	if (logger::instance().getLogStream() != &std::cout) {\
		std::lock(logger::getLogMutex(), logger::getErrMutex());\
		std::lock_guard<std::mutex> lkL(logger::getLogMutex(), std::adopt_lock);\
		std::lock_guard<std::mutex> lkE(logger::getErrMutex(), std::adopt_lock);\
		logger::instance().writeprefix(std::cout);\
		std::cout << X;\
	}\
}
#else
#define LOG(X) LOGIMPL(X)
#endif

#define LOGNP(X) { if (*logger::instance().getLogStream()) {\
	std::lock_guard<std::mutex> lk(logger::getLogMutex());\
	*logger::instance().getLogStream() << X;\
	}\
}
#define LOGLN(X) {LOG(X << "\n")}
#define ERROR(X) {\
	std::lock_guard<std::mutex> lk(logger::getErrMutex());\
	for (auto stream : {&std::cerr, logger::instance().getErrStream()}) {\
		if (!stream)\
			continue;\
		*stream << "[ERROR]";\
		logger::instance().writeprefix(*stream);\
		*stream << X << "\n";\
	}\
}

#else
#define LOG(X)
#define LOGNP(X)
#define LOGLN(X)
#define ERROR(X)
#endif

#ifdef DEBUG
#define DEBUGLOG LOG
#define DEBUGLOGLN LOGLN
#else
#define DEBUGLOG(X)
#define DEBUGLOGLN(X)
#endif

#ifdef _ENABLE_LOGGING_

class logger {
public:
	void writeprefix(std::ostream &stream);

	// returns old stream
	static std::ostream* setLogStream(std::ostream* newStream) {
		std::lock_guard<std::mutex> lk(logMutex_);
		std::ostream* pOld = instance_.pLogStream_;
		instance_.pLogStream_.store(newStream);
		return pOld;
	}
	// returns old stream
	std::ostream* setAdditionalErrStream(std::ostream* newStream) {
		std::lock_guard<std::mutex> lk(errMutex_);
		std::ostream* pOld = instance_.pErrStream_;
		instance_.pErrStream_.store(newStream);
		return pOld;
	}

	std::ostream* getLogStream() { return pLogStream_; }
	std::ostream* getErrStream() { return pErrStream_; }

	static logger& instance() { return instance_; }

	static std::mutex& getLogMutex() { return logMutex_; }
	static std::mutex& getErrMutex() { return errMutex_; }

private:
	static std::atomic<std::ostream*> pLogStream_;
	static std::atomic<std::ostream*> pErrStream_;
	std::deque<std::string> prefix_;
	static thread_local logger& instance_;
	static std::mutex logMutex_;
	static std::mutex errMutex_;

	void push_prefix(std::string prefix) { prefix_.push_back(prefix); }
	void pop_prefix() { prefix_.pop_back(); }

	friend class logger_prefix;
};

class logger_prefix {
public:
	logger_prefix(std::string s) {
		logger::instance_.push_prefix(s);
	}
	~logger_prefix() {
		logger::instance_.pop_prefix();
	}
};

#endif // _ENABLE_LOGGING_

#endif /* LOG_H_ */
