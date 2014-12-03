#ifndef __serialization_h__
#define __serialization_h__

#include <string>
#include "../genetics/Genome.h"

class Serialization {
public:
	static bool saveGenome(Genome &theGenome, std::string filename);
	static bool loadGenome(std::string filename, Genome &outGenome);
};

#endif //__serialization_h__
