#include "Ribosome.h"

#include "../neuralnet/functions.h"
#include "../math/tools.h"
#include "../Logger.h"
#include "../neuralnet/Network.h"
#include "../neuralnet/Neuron.h"
#include "../neuralnet/OutputSocket.h"
#include "Chromosome.h"
#include "Gene.h"

#include <assert.h>
#include <sstream>
#include <map>
using namespace std;

map<gene_type, string> mapGeneTypeNames;
int initGeneTypeNameMap() {
	mapGeneTypeNames[GENE_TYPE_INVALID] = "INVALID";
	mapGeneTypeNames[GENE_INPUT_SOURCE] = "INPUT_SOURCE";
	mapGeneTypeNames[GENE_INPUT_WEIGHT] = "INPUT_WEIGHT";
	mapGeneTypeNames[GENE_BIAS] = "BIAS";
	mapGeneTypeNames[GENE_TRANSFER_FUNCTION] = "TRANSFER";
	mapGeneTypeNames[GENE_TRANSFER_ARGUMENT] = "TRANSF_ARG";
	mapGeneTypeNames[GENE_OUTPUT] = "OUTPUT";
	return 0;
}
int dummy_to_init_map = initGeneTypeNameMap();

struct neuronOutputGenePair {
	Neuron* pNeuron;
	double output_gene_value;
	neuronOutputGenePair(Neuron* pNeuron, double output_gene_value)
		: pNeuron(pNeuron), output_gene_value(output_gene_value)
	{ }
};

void check_create_default_input(NeuralNet* net, unsigned int inputIndex) {
	while (inputIndex >= net->inputs.size()) {
		net->inputs.push_back(new OutputSocket());
	}
}

void Ribosome::decode_and_develop_network(NeuralNet* net) {
	/* Develops an embryonic NeuralNet by decoding its Genome sequence and building the NeuralNet from that.

	Each Chromosome in the Genome defines an individual Neuron.

	The number of default inputs is determined directly from how the neurons refer to the inputs.
	They are created as needed.
	There is no number of default outputs, they are taken from the last Neuron, then second-last and so on
	until all outputs are satisfied or all neurons are used, in which case the remaining outputs are set to zero.
	Each Neuron specifies its Input sources, thus the synapses are defined. If a negative number is used
	for Input source, then its modulus represents a default Input source index, else the number represents the index
	of a Neuron to be used as Input for the current one
	*/
	assert(net->neurons.empty() && L"Embryonic NeuralNet has an invalid state!");
	assert(net->inputs.empty() && L"Embryonic NeuralNet has an invalid state!");
	assert(net->outputs.empty() && L"Embryonic NeuralNet has an invalid state!");

	// create the raw neurons, then decode Genome and fix them
	for (int i=0, n=net->genome.size(); i<n; ++i) {
		net->neurons.push_back(new Neuron(i));
	}
	std::vector<neuronOutputGenePair> outputNeurons;

	for (int iCh=0, nCh=net->genome.size(); iCh<nCh; ++iCh) {
		// read each Chromosome and build a Neuron from it
		vector<Gene> &genes = net->genome[iCh].gene_list;
		vector<double> inputWeights;
		for (int iG=0, nG=genes.size(); iG<nG; ++iG) {
			switch (genes[iG].type) {
				case GENE_INPUT_SOURCE:
					if (genes[iG].value_int < 0) {
						// link this Neuron to one of the default inputs
						unsigned int inputIndex = -genes[iG].value_int - 1;
						if (inputIndex >= MAX_NETWORK_DEFAULT_INPUTS)
							continue; // ignore this Gene if it specifies an invalid value
						check_create_default_input(net, inputIndex);
						Input* inp = new Input(net->neurons[iCh]);
						net->neurons[iCh]->inputs.push_back(inp);
						net->neurons[iCh]->isInput = true;
						net->inputs[inputIndex]->addTarget(net->neurons[iCh]->inputs.back());
					} else {
						// link this Neuron's Input to another's output
						Neuron* pSrcNeuron = NULL;
						if (genes[iG].value_int < (int)net->neurons.size())
							pSrcNeuron = net->neurons[genes[iG].value_int];
						// if the Gene has a valid Neuron index value, then the Input is created and connected,
						// otherwise the Gene is ignored
						if (pSrcNeuron != NULL) {
							Input* inp = new Input(net->neurons[iCh]);
							net->neurons[iCh]->inputs.push_back(inp);
							pSrcNeuron->output.addTarget(net->neurons[iCh]->inputs.back());
						}
					}
					break;
				case GENE_INPUT_WEIGHT:
					inputWeights.push_back(genes[iG].value_double);
					break;
				case GENE_BIAS:
					net->neurons[iCh]->bias = genes[iG].value_double;
					break;
				case GENE_TRANSFER_FUNCTION:
					int fnIndex; fnIndex = genes[iG].value_int;
					if (fnIndex >= FN_MAXCOUNT || fnIndex == FN_INVAID) // if the Gene specifies an invalid function index,
						fnIndex = FN_SIGM;		// fall-back to default SIGM
					net->neurons[iCh]->transfFunc= mapTransferFunctions[(transferFuncNames)fnIndex];
					break;
				case GENE_TRANSFER_ARGUMENT:
					net->neurons[iCh]->transferConstant = genes[iG].value_double;
				case GENE_OUTPUT:
					// the output Gene says this Neuron should be linked to one of the NeuralNet's output sockets
					// we need to save all neurons of this kind into a list, then order them based on the 
					// output genes' values and link them to output sockets in that order.
					outputNeurons.push_back(neuronOutputGenePair(net->neurons[iCh], genes[iG].value_double));
					break;
				default:
					assert(false && "Invalid Gene type!");
			}
		} 
		// done looping genes
		// check if Neuron has been assigned a function, if not default to SUM
		if (net->neurons[iCh]->transfFunc == NULL)
			net->neurons[iCh]->transfFunc = mapTransferFunctions[FN_SIGM];
		// set Input weights for current Neuron
		for (unsigned int iI=0, nI=net->neurons[iCh]->inputs.size(); iI<nI; ++iI) {
			if (iI >= inputWeights.size())
				break;
			net->neurons[iCh]->inputs[iI]->weight = inputWeights[iI];
		}
	}

	// done looping neurons, now order by strongest first and link output neurons to NeuralNet outputs
	for (int i=0, n=outputNeurons.size(); i<n-1; ++i) {
		for (int j=i+1; j<n; ++j)
			if (outputNeurons[i].output_gene_value < outputNeurons[j].output_gene_value) {
				neuronOutputGenePair aux = outputNeurons[i];
				outputNeurons[i] = outputNeurons[j];
				outputNeurons[j] = aux;
			}
	}
	for (int i=0, n=outputNeurons.size(); i<n; ++i) {
		Input* pOut = new Input(NULL);
		outputNeurons[i].pNeuron->output.addTarget(pOut);
		outputNeurons[i].pNeuron->isOutput = true;
		net->outputs.push_back(pOut);
	}
}

void alter_meta_gene(MetaGene* pMeta)
{
	pMeta->value += srandd() * pMeta->dynamic_variation;
	if (pMeta->value < 0)
		pMeta->value = 0;
}

void segregate_and_alter_meta_genes(vector<MetaGene*> &src1, vector<MetaGene*> &src2, vector<MetaGene*> &target)
{
	for (unsigned iMG=0; iMG<src1.size(); ++iMG) {
		double r = randd();	// roll the dice...
		if (r<0.5)
			*target[iMG] = *src1[iMG];	// 50% select #1
		else
			*target[iMG] = *src2[iMG];	// or 50% select #2

		// now alter the meta-Gene:
		alter_meta_gene(target[iMG]);
	}
}

// alters the Gene by mutating, transforming or deleting
// returns false if Gene has been deleted, and true otherwise
bool alter_gene(Gene *pGene, unsigned availableDefInputs, unsigned nTotalNeurons, logger &log)
{
	// update Gene's meta genes:
	for (unsigned iMG=0, nMG=pGene->metaGenes.size(); iMG<nMG; ++ iMG)
		alter_meta_gene(pGene->metaGenes[iMG]);

	if (randd() < pGene->chance_to_delete.value) {
		log << "Gene has been deleted!\n";
		// Gene is deleted
		return false;
	}

	if (randd() < pGene->chance_to_transform.value) {
		log << "Gene transforming... ";
		gene_type newType = (gene_type)(randi(GENE_INPUT_SOURCE, GENE_OUTPUT));
		gene_value_type newValueType = Gene::mapGeneValueTypes[newType];
		if (newValueType != pGene->value_type) {
			// value types differ, we must convert the data:
			switch (newValueType) {
				case GENE_VALUE_DOUBLE:
					pGene->value_double = pGene->value_int;
					break;
				case GENE_VALUE_INT:
					pGene->value_int = (int)pGene->value_double;
					break;
				default:
					assert(false && "Invalid Gene type!");
			}
		}
		pGene->type = newType;
		pGene->value_type = newValueType;
		log < "new type : " < mapGeneTypeNames[pGene->type] < "\n";
	}

	if (randd() < pGene->chance_to_mutate.value) {
		log << "Gene mutating...  ";
		switch (pGene->value_type) {
			case GENE_VALUE_INT:
				// int value genes must be treated in a special way, because they usually define indices
				// in some arrays or enums
				switch (pGene->type) {
				case GENE_INPUT_SOURCE:
					pGene->value_int = randi(-(int)availableDefInputs, nTotalNeurons - 1);
					log < "New Input index is : " < pGene->value_int < "\n";
					break;
				case GENE_TRANSFER_FUNCTION:
					pGene->value_int = randi(FN_SIN, FN_MAXCOUNT-1);
					log < "Transfer function changed to : " < mapTransferFunctionNames[(transferFuncNames)pGene->value_int] < "\n";
					break;
				default:
					break;
				}
				break;
			case GENE_VALUE_DOUBLE:
				pGene->value_double += srandd() * pGene->mutation_reference_value.value;
				log < "New dbl value is : " < pGene->value_double < "\n";
				break;
			default:
				assert(false && "Invalid Gene type!");
		}
	}

	// we don't treat Gene swapping here, that is taken care of at Chromosome level

	return true;
}

template<typename T>
void shuffle_elements(vector<T> &vec, logger &log) {
	//use chance_to_swap from each element
	int prevSwap = -1;
	for (unsigned i=0, n=vec.size(); i<n; ++i) {
		if (randd() < vec[i].chance_to_swap.value) {
			if (prevSwap != -1) {
				// we found a match, swap them!
				log << "Swapping " < i < " and " < prevSwap < "\n";
				T aux = vec[prevSwap];
				vec[prevSwap] = vec[i];
				vec[i] = aux;
				// reset index:
				prevSwap = -1;
			} else
				prevSwap = i;
		}
	}
}

// alter the Chromosome by swapping genes' places, spawning new genes, splitting or deleting
// returns false if Chromosome has been deleted, and true otherwise
// if Chromosome is split, then in pOutSplit will be returned a new Chromosome, else NULL
bool alter_chromosome(Chromosome *pChromosome, Chromosome** pOutSplit, unsigned availDefInputs, unsigned nTotalNeurons, unsigned numSynapses, logger &log)
{
	if (numSynapses == 0)
		numSynapses = 1;
	// deletion (the more synapses a Chromosome forms, the less chance for it to be deleted):
	if (randd() < pChromosome->chance_to_delete.value / numSynapses) {
		// Chromosome is deleted
		return false;
	}

	// shuffle:
	log.push_prefix("shuffle_genes");
	shuffle_elements<Gene>(pChromosome->gene_list, log);
	log.pop_prefix();

	// spawn new Gene
	if (randd() < pChromosome->chance_to_spawn_gene.value) {
		//don't allow output genes!
		log << "New Gene spawned ! Altering (deletion suppressed)...\n";
		log.push_prefix("new_gene");
		Gene newGene((gene_type)(randi(GENE_INPUT_SOURCE, GENE_TYPE_END-1)), 0, randd());
		while (!alter_gene(&newGene, availDefInputs, nTotalNeurons, log)); // avoid deletion
		log.pop_prefix();
		pChromosome->add_gene(newGene);
	}

	// split:
	if (randd() < pChromosome->chance_to_split.value * pChromosome->gene_list.size()) {
		unsigned splitpoint = pChromosome->gene_list.size() / 2 + randi(pChromosome->gene_list.size()/2);
		if (splitpoint == pChromosome->gene_list.size())
			--splitpoint;
		log << "Splitting Chromosome at position " < splitpoint < "\n";
		*pOutSplit = new Chromosome(*pChromosome);
		(*pOutSplit)->gene_list = vector<Gene>((*pOutSplit)->gene_list.begin() + splitpoint, (*pOutSplit)->gene_list.end());
		// update new Chromosome's meta genes:
		for (unsigned iMG=0, nMG=(*pOutSplit)->metaGenes.size(); iMG<nMG; ++ iMG)
			alter_meta_gene((*pOutSplit)->metaGenes[iMG]);
		unsigned oldSize = pChromosome->gene_list.size();
		pChromosome->gene_list.resize(splitpoint);
		assert(pChromosome->gene_list.size() < oldSize);
	} else
		*pOutSplit = NULL;

	// update Chromosome's meta genes:
	for (unsigned iMG=0, nMG=pChromosome->metaGenes.size(); iMG<nMG; ++ iMG)
		alter_meta_gene(pChromosome->metaGenes[iMG]);

	return true;
}

Chromosome Ribosome::createRandomChromosome(unsigned maxInputs, unsigned maxDefaultInputs) {
	Chromosome c;
	c.add_gene(Gene(GENE_BIAS, 0, randd()));
	c.add_gene(Gene(GENE_TRANSFER_FUNCTION, randi(FN_SIN, FN_MAXCOUNT-1), 0));
	unsigned nInp = randi(maxInputs);
	vector<bool> vecAdded; vecAdded.assign(maxInputs, false);
	for (unsigned i=0; i<nInp; ++i) {
		int inputSrc = -1;
		while (inputSrc == -1 || vecAdded[inputSrc])
			inputSrc = randi(maxInputs-1);
		vecAdded[inputSrc] = true;
		c.add_gene(Gene(GENE_INPUT_SOURCE, inputSrc, 0));
		c.add_gene(Gene(GENE_INPUT_WEIGHT, 0, randd()));
	}
	nInp = randi(maxDefaultInputs);
	vecAdded.assign(maxDefaultInputs, false);
	for (unsigned i=0; i<nInp; ++i) {
		int inputSrc = -1;
		while (inputSrc == -1 || vecAdded[inputSrc])
			inputSrc = randi(maxDefaultInputs-1);
		vecAdded[inputSrc] = true;
		c.add_gene(Gene(GENE_INPUT_SOURCE, -inputSrc-1, 0));
		c.add_gene(Gene(GENE_INPUT_WEIGHT, 0, randd()));
	}
	return c;
}

inline int MIN(int a, int b) { return a>b?b:a; }

 Genome Ribosome::recombine_offspring(Genome &genome1, Genome &genome2, int requiredOutputs, int availableInputs)
{
	// 1. find the minimum number of genome from parent1 and parent2;
	// 2. for the remaining genome (present in only one parent) there's a 50-50 chance that each will
	//		be inherited by the offspring; some of the genes of the Chromosome (if it's inherited) may be mutated or deleted.
	// 3. for the genome that are present in both parents, a new Chromosome is created by randomly selecting one Gene from
	//		each pair formed by the parents' genome. Genes that don't have an equivalent in the other Chromosome, have
	//		a 50-50 chance to be segregated or not.
	// 4. at this step each Gene may be mutated or deleted. New genes may appear in the Chromosome.
	// 5. if a Chromosome grows too big, the chance that it will split into two smaller genome increases.
	// 
	// there's a small chance that each Chromosome may be deleted from the Genome (inverse proportional to the
	// number of synapses it forms

	logger log("recombine_offspring");
	log << "START recombination\n";

	Genome newGenome; // this represents the output Genome
	vector<unsigned> synapses; synapses.assign(max(genome1.size(),genome2.size())+1, 0); // holds the synapse count that will
	// be created at each Neuron; a Chromosome that defines a Neuron with many synapses tends to be more stable
	vector<bool> isOutput; isOutput.assign(max(genome1.size(),genome2.size())+1, false);

	int outputCount = 0;

	unsigned nCommonChroms = genome1.size();
	Genome *pRemainingChromosomes = &genome2;

	// compute the approximate size of the new Genome - will need it when altering Input genes:
	unsigned approxGenomeSize = nCommonChroms + (pRemainingChromosomes->size() - nCommonChroms) / 2;

	if (genome2.size() < nCommonChroms) {
		nCommonChroms = genome2.size();
		pRemainingChromosomes = &genome1;
	}

	for (unsigned i=0; i<nCommonChroms; ++i) {
		ostringstream str; str << "chromosome_" << i;
		log.push_prefix(str.str());

		// create i-th Chromosome from the two parent genome
		Chromosome chr; // the new Chromosome
		Chromosome& pC1 = genome1[i]; // parent Chromosome 1
		Chromosome& pC2 = genome2[i]; // parent Chromosome 2

		bool isCrtOut = false;

		// segregate the meta-genes from the two genome into the child Chromosome:
		segregate_and_alter_meta_genes(pC1.metaGenes, pC2.metaGenes, chr.metaGenes);

		// segregate genes:
		unsigned nCommonGenes = pC1.gene_list.size();
		if (pC2.gene_list.size() < nCommonGenes)
			nCommonGenes = pC2.gene_list.size();
		for (unsigned iG = 0; iG<nCommonGenes; ++iG) {
			Gene selectedGene;
			// random selection
			if (randd() > 0.5) {
				selectedGene = pC1.gene_list[iG];
			} else {
				selectedGene = pC2.gene_list[iG];
			}

			// alter Gene and check if it should be deleted:
			str.str(""); str << "gene_" << iG;
			log.push_prefix(str.str());
			if (alter_gene(&selectedGene, availableInputs, approxGenomeSize, log)) {
				chr.add_gene(selectedGene);
				if (selectedGene.type == GENE_INPUT_SOURCE) {
					synapses[i]++;	// target Neuron
					if (selectedGene.value_int >= 0)
						synapses[selectedGene.value_int]++; // source Neuron
				}
				if (selectedGene.type == GENE_OUTPUT && !isCrtOut) {
					outputCount++;
					isOutput[i] = true;
					isCrtOut = true;
				}
			}
			log.pop_prefix();
		}
		// copy the remaining genes:
		vector<Gene> *vecRemaining = NULL;
		if (pC1.gene_list.size() > nCommonGenes)
			vecRemaining = &pC1.gene_list;
		else
			if (pC2.gene_list.size() > nCommonGenes)
				vecRemaining = &pC2.gene_list;
		if (vecRemaining != NULL) {
			log.push_prefix("remaining_genes");
			for (unsigned iG=nCommonGenes, nG=vecRemaining->size(); iG<nG; ++iG) {
				str.str(""); str << "gene_" << iG; log.push_prefix(str.str());
				Gene g = (*vecRemaining)[iG];
				// bool isOut = g.type == GENE_OUTPUT;
				if (randd() < 0.5 && alter_gene(&g, availableInputs, approxGenomeSize, log)) {
					chr.add_gene(g);
					if (g.type == GENE_OUTPUT && !isCrtOut) {
						outputCount++;
						isOutput[i] = true;
						isCrtOut = true;
					}
				}
				log.pop_prefix();
			}
			log.pop_prefix();
		}

		newGenome.push_back(chr);
		log.pop_prefix();
	}

	// now treat remaining genome which are segregated at Chromosome level
	for (unsigned i=nCommonChroms, n=pRemainingChromosomes->size(); i<n; ++i) {
		if (randd() < 0.5) {
			newGenome.push_back((*pRemainingChromosomes)[i]);
			bool isCrtOut = false;
			// count synapses:
			for (unsigned jG=0; jG<(*pRemainingChromosomes)[i].gene_list.size(); jG++) {
				if ((*pRemainingChromosomes)[i].gene_list[jG].type == GENE_INPUT_SOURCE) {
					synapses[i]++;	// target Neuron
					if ((*pRemainingChromosomes)[i].gene_list[jG].value_int >= 0)
						synapses[(*pRemainingChromosomes)[i].gene_list[jG].value_int]++; // source Neuron
				}
				if ((*pRemainingChromosomes)[i].gene_list[jG].type == GENE_OUTPUT && !isCrtOut) {
					outputCount++;
					isOutput[i] = true;
					isCrtOut = true;
				}
			}
		}
	}

	// alter genome:
	for (unsigned i=0; i<newGenome.size(); i++) {
		ostringstream str; str << "chromosome_" << i; log.push_prefix(str.str());
		Chromosome* pAux = NULL;
		if (alter_chromosome(&newGenome[i], &pAux, availableInputs, newGenome.size(), synapses[i], log)) {
			
			if (pAux) { // Chromosome has been split
				
				if (isOutput[i]) {
					// Chromosome had output, must check if it still persists after split
					bool outFound = false;
					for (unsigned jG=0; jG<newGenome[i].gene_list.size(); jG++) {
						if (newGenome[i].gene_list[jG].type == GENE_OUTPUT) {
							outFound = true;
							break;
						}
					}
					if (!outFound) {
						isOutput[i] = false;
						outputCount--;
					}
				}

				newGenome.push_back(*pAux);
				synapses.push_back(0);
				isOutput.push_back(false);
				// count synapses & check for output:
				for (unsigned jG=0; jG<pAux->gene_list.size(); jG++) {
					if (pAux->gene_list[jG].type == GENE_INPUT_SOURCE) {
						synapses[newGenome.size()-1]++;	// target Neuron
					}
					if (pAux->gene_list[jG].type == GENE_OUTPUT && !isOutput[newGenome.size()-1]) {
						isOutput[newGenome.size()-1] = true;
						outputCount++;
					}
				}
				synapses[i] -= synapses[newGenome.size()-1];
				delete pAux;
			}
			// newGenome[i].chance_to_swap.value /= synapses[i] ? synapses[i] : 1; // prepare for swapping
		} else {
			log << "Chromosome deleted!\n";
			// check if it was output:
			if (isOutput[i])
				outputCount--;
			newGenome.erase(newGenome.begin()+i);
			synapses.erase(synapses.begin()+i);
			isOutput.erase(isOutput.begin()+i);
			i--;
		}
		log.pop_prefix();
	}

	// now check if there are enough output genome and if not, add some output genes to make up the required number
	for (; outputCount<MIN(requiredOutputs, newGenome.size()); outputCount++) {
		int index = -1;
		// search for a non-output Chromosome and add an output Gene to it
		while (index == -1 || isOutput[index])
			index = randi(newGenome.size()-1);
		newGenome[index].add_gene(Gene(GENE_OUTPUT, 0, randd()));
	}

	// shuffle genome:
	// divided all genome chance to shuffle by the number of their synapses, do the shuffling then multiply back
	log.push_prefix("shuffling_chromosomes");
	for (unsigned i=0, n=newGenome.size(); i<n; i++) {
		newGenome[i].chance_to_swap.value /= synapses[i] ? synapses[i] : 1;
	}
	shuffle_elements<Chromosome>(newGenome, log);
	log.pop_prefix();
	for (unsigned i=0, n=newGenome.size(); i<n; i++) {
		newGenome[i].chance_to_swap.value *= synapses[i] ? synapses[i] : 1;
	}

	// check if we should create new genome:
	if (randd() < constants::global_chance_new_chromosome) {
		log << "Created new Chromosome!\n";
		newGenome.push_back(createRandomChromosome(newGenome.size(), availableInputs));
		// create several jump-start Input synapses from other neurons to this new one
		unsigned nSyn = randi(1, 4);
		if (nSyn > newGenome.size()-1)
			nSyn = newGenome.size()-1;
		vector<bool> vecVisited; vecVisited.assign(newGenome.size() - 1, false);
		for (unsigned i=0; i<nSyn; ++i) {
			int index = -1;
			while (index == -1 || vecVisited[index]) 
				index = randi(newGenome.size() - 2);
			vecVisited[index] = true;
			newGenome[index].add_gene(Gene(GENE_INPUT_SOURCE, newGenome.size()-1, 0));
			newGenome[index].add_gene(Gene(GENE_INPUT_WEIGHT, 0, randd()));
		}
	}

	log << "END recombination.\n";

	return newGenome;
}

Genome Ribosome::createRandomGenome(unsigned maxChromosomes, unsigned nDefaultInputs, unsigned requiredOutputs)
{
	unsigned minChromosomes = nDefaultInputs;
	if (minChromosomes < requiredOutputs)
		minChromosomes = requiredOutputs;
	if (maxChromosomes < minChromosomes)
		maxChromosomes = minChromosomes;
	unsigned nChroms = minChromosomes + randi(maxChromosomes - minChromosomes);
	
	Genome randGenome;
	// create the perceptrons:
	for (unsigned i=0; i<nDefaultInputs; ++i) {
		randGenome.push_back(createRandomChromosome(0, 0));
		randGenome[i].add_gene(Gene(GENE_INPUT_SOURCE, -(int)i-1, 0));
		randGenome[i].add_gene(Gene(GENE_INPUT_WEIGHT, 0, randd()));
	}
	// create the rest of the genome:
	for (unsigned i=nDefaultInputs; i<nChroms; ++i) {
		randGenome.push_back(createRandomChromosome(nChroms, 0));
	}

	// add exactly requiredOutputs output genes to different genome:
	vector<bool> vecOutputs; vecOutputs.assign(nChroms, false);
	for (unsigned i=0; i<requiredOutputs; ++i) {
		int outIndex = -1;
		while (outIndex == -1 || vecOutputs[outIndex])
			outIndex = randi(nChroms-1);
		vecOutputs[outIndex] = true;
		randGenome[outIndex].add_gene(Gene(GENE_OUTPUT, 0, 0));
	}
	return randGenome;
}
