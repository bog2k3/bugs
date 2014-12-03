#include "Ribosome.h"

#include "../neuralnet/functions.h"
#include "../math/tools.h"
#include "../log.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "Gene.h"
#include "GeneDefinitions.h"

#include "Genome.h"
using namespace std;

struct neuronOutputGenePair {
	Neuron* pNeuron;
	double output_gene_value;
	neuronOutputGenePair(Neuron* pNeuron, double output_gene_value)
		: pNeuron(pNeuron), output_gene_value(output_gene_value)
	{ }
};

Ribosome::Ribosome(Bug* bug)
	: bug(bug)
	, crtPosition(0)
{
}

bool Ribosome::step() {
	bug->genome
}
