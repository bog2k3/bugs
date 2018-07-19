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

template<class C>
SeparatedTextOutStream& operator << (SeparatedTextOutStream &s, C const& c) {
	s.str_ << c << "\n";
	return s;
}

#endif /* BUGS_SERIALIZATION_SEPARATEDTEXTOUTSTREAM_H_ */
