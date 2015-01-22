/*
 * log.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: bogdan
 */

#include "log.h"

std::stack<logger*> logger::loggers;

void logger::writeprefix(std::ostream &stream) const {
	stream << "[" << name;
	for (unsigned i=0, n=prefix.size(); i<n; i++)
		stream << "." << prefix[i];
	stream << "] ";
}
