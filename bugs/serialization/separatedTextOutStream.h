/*
 * separatedTextOutStream.h
 *
 *  Created on: Jul 19, 2018
 *      Author: bog
 */

#ifndef BUGS_SERIALIZATION_SEPARATEDTEXTOUTSTREAM_H_
#define BUGS_SERIALIZATION_SEPARATEDTEXTOUTSTREAM_H_

#include <ostream>

class SeparatedTextOutStream {
public:
	SeparatedTextOutStream(std::ostream &str)
		: str_(str) {
	}

	~SeparatedTextOutStream() {}

	std::ostream &str_;
};

SeparatedTextOutStream& operator << (SeparatedTextOutStream &s, int const& c) {
	s.str_ << c << "\n";
	return s;
}

SeparatedTextOutStream& operator << (SeparatedTextOutStream &s, unsigned const& c) {
	s.str_ << c << "\n";
	return s;
}

SeparatedTextOutStream& operator << (SeparatedTextOutStream &s, float const& c) {
	s.str_ << c << "\n";
	return s;
}

SeparatedTextOutStream& operator << (SeparatedTextOutStream &s, double const& c) {
	s.str_ << c << "\n";
	return s;
}

#endif /* BUGS_SERIALIZATION_SEPARATEDTEXTOUTSTREAM_H_ */
