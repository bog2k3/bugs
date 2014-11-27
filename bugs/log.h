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
#define LOG(X) std::cout << X;
#define LOGLN(X) LOG(X << std::endl);
#define ERROR(X) std::cerr << "[ERROR] " << X << std::endl;

#else
#define LOG(X)
#define LOGLN(X)
#define ERROR(X)
#endif

#endif /* LOG_H_ */