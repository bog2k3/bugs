/*
 *	a ribosome decodes a sequence of chromosomes and builds a fully-functional neural network
 */

#ifndef __ribosome_h__
#define __ribosome_h__

#include "Genome.h"
#include "Gene.h"

class Bug;
class DevelopmentNode;

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
	unsigned crtPosition;
	DevelopmentNode* root;

	void decodeDevelopCommand(GeneCommand const& g);
	void decodeDevelopGrowth(GeneCommand const& g, std::vector<DevelopmentNode*> const& nodes);
	void decodeDevelopSplit(GeneCommand const& g, std::vector<DevelopmentNode*> const& nodes);
	void decodePartAttrib(GeneLocalAttribute const& g);
	void decodeGeneralAttrib(GeneGeneralAttribute const& g);
	void decodeSynapse(GeneSynapse const& g);
	void decodeTransferFn(GeneTransferFunction const& g);
	void decodeMuscleCommand(GeneMuscleCommand const& g);
	bool partMustGenerateJoint(int part_type);
};

#endif //__ribosome_h__
