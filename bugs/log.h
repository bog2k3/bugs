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

#define LOGGER(NAME) logger logger_token(NAME)
#define LOGPREFIX(PREF) logger_prefix logger_prefix_token(PREF);

#define LOG(X) logger::getCurrent()->writeprefix(std::cout); std::cout << X
#define LOGNP(X) std::cout << X
#define LOGLN(X) logger::getCurrent()->writeprefix(std::cout); std::cout << X << std::endl
#define ERROR(X) std::cerr << "[ERROR]"; logger::getCurrent()->writeprefix(std::cerr); std::cerr << X << std::endl

#else
#define LOG(X)
#define LOGNP(X)
#define LOGLN(X)
#define ERROR(X)
#endif

#include <string>
#include <iostream>
#include <deque>
#include <stack>

class logger {
public:
	logger(std::string name) : name(name) { loggers.push(this); }
	~logger() { loggers.pop(); }

	static logger* getCurrent() { return loggers.empty() ? nullptr : loggers.top(); }
	void writeprefix(std::ostream &stream) const;

private:
	std::string name;
	std::deque<std::string> prefix;
	static std::stack<logger*> loggers;

	void push_prefix(std::string prefix) { this->prefix.push_back(prefix); }
	void pop_prefix() { this->prefix.pop_back(); }

	friend class logger_prefix;
};

class logger_prefix {
public:
	logger_prefix(std::string s) {
		logger::getCurrent()->push_prefix(s);
	}
	~logger_prefix() {
		logger::getCurrent()->pop_prefix();
	}
};

#endif /* LOG_H_ */
