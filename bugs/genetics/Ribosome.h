/*
 *	a ribosome decodes a sequence of chromosomes and builds a fully-functional neural network
 */

#ifndef __ribosome_h__
#define __ribosome_h__

#include "Genome.h"

class Bug;

/**
 * decodes the entity's genome and builds it step by step. When finished the entity will have its final
 * shape and preprogrammed functionality, but will be very small in size.
 */
class Ribosome {
public:
	Ribosome(Bug* the_bug);
	~Ribosome();

	/**
	 * develops the entity one more step. Returns true as long as the process is not finished.
	 */
	bool step();

private:
	Bug* bug;
	int crtPosition;
};

#endif //__ribosome_h__
