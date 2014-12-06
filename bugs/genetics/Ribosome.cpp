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

Ribosome::Ribosome(Bug* bug)
	: bug(bug)
	, crtPosition(0)
{
}

bool Ribosome::step() {
	// bug->genome.
}
