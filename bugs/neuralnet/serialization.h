#ifndef __serialization_h__
#define __serialization_h__

#include <string>
#include "Chromosome.h"

namespace _libNeurons {

	struct chromosome;

	class serialization {
	public:

		static bool saveGenome(genome &theGenome, std::string filename);

		static bool loadGenome(std::string filename, genome &outGenome);
	};

} // namespace

#endif //__serialization_h__
