#include "serialization.h"
#include "chromosome.h"
#include <fstream>

namespace _libNeurons {

	static const unsigned MAGIC_GENOME_HEADER = 0x6E9043AB;
	static const unsigned MAGIC_CHROMOSOME_HEADER = 0xC3904020;

	void writeMetaGenes(std::vector<meta_gene*> &mg, std::ofstream *fs) {
		unsigned size =mg.size();
		fs->write((const char*)&size, sizeof(size));
		for (unsigned i=0; i<size; ++i) {
			fs->write((const char*)&mg[i]->value, sizeof(mg[i]->value));
			fs->write((const char*)&mg[i]->dynamic_variation, sizeof(mg[i]->dynamic_variation));
		}
	}

	void readMetaGenes(std::vector<meta_gene*> &mg, std::ifstream *fs) {
		unsigned size;
		fs->read((char*)&size, sizeof(size));
		for (unsigned i=0; i<size; ++i) {
			unsigned index = i;
			if (index >= mg.size())
				index = mg.size() - 1; // if there are more values in the file than there are 
			// meta genes in the list, continue overwriting the last one, to avoid vector overflow
			fs->read((char*)&mg[index]->value, sizeof(mg[index]->value));
			fs->read((char*)&mg[index]->dynamic_variation, sizeof(mg[index]->dynamic_variation));
		}
	}

	void writeGene(gene &g, std::ofstream *fs) {
		fs->write((const char*)&g.type, sizeof(g.type));
		fs->write((const char*)&g.value_type, sizeof(g.value_type));
		fs->write((const char*)&g.value_int, sizeof(g.value_int));
		fs->write((const char*)&g.value_double, sizeof(g.value_double));
		writeMetaGenes(g.meta_genes, fs);
	}

	void readGene(std::vector<gene> &genes, std::ifstream *fs) {
		gene g;
		fs->read((char*)&g.type, sizeof(g.type));
		fs->read((char*)&g.value_type, sizeof(g.value_type));
		fs->read((char*)&g.value_int, sizeof(g.value_int));
		fs->read((char*)&g.value_double, sizeof(g.value_double));
		readMetaGenes(g.meta_genes, fs);
		genes.push_back(g);
	}

	void writeChromosome(chromosome &c, std::ofstream *fs) {
		fs->write((const char*)&MAGIC_CHROMOSOME_HEADER, sizeof(MAGIC_CHROMOSOME_HEADER));
		unsigned geneCount = c.gene_list.size();
		fs->write((const char*)&geneCount, sizeof(geneCount));
		for (unsigned i=0; i<geneCount; ++i) {
			writeGene(c.gene_list[i], fs);
		}
		writeMetaGenes(c.meta_genes, fs);
	}

	void readChromosome(genome &outGenome, std::ifstream *fs) {
		unsigned magic;
		fs->read((char*)&magic, sizeof(magic));
		if (magic != MAGIC_CHROMOSOME_HEADER)
			throw "we fucked up the data in the file...";
		chromosome c;
		unsigned geneCount;
		fs->read((char*)&geneCount, sizeof(geneCount));
		for (unsigned i=0; i<geneCount; ++i) {
			readGene(c.gene_list, fs);
		}
		readMetaGenes(c.meta_genes, fs);
		outGenome.push_back(c);
	}

	bool serialization::saveGenome(genome &theGenome, std::string filename)
	{
		std::ofstream fs;
		try {
			fs.open(filename.c_str(), std::ios_base::out|std::ios_base::binary);
			fs.write((const char*)&MAGIC_GENOME_HEADER, sizeof(MAGIC_GENOME_HEADER));
			unsigned genomeSize = theGenome.size();
			fs.write((const char*)&genomeSize, sizeof(genomeSize));
			for (unsigned i=0; i<genomeSize; ++i) {
				writeChromosome(theGenome[i], &fs);
			}
			fs.close();
		} catch (...) {
			return false;
		}
		return true;
	}

	bool serialization::loadGenome(std::string filename, genome &outGenome)
	{
		std::ifstream fs;
		try {
			outGenome.clear();
			fs.open(filename.c_str(), std::ios_base::in|std::ios_base::binary);
			unsigned magic;
			fs.read((char*)&magic, sizeof(magic));
			if (magic != MAGIC_GENOME_HEADER)
				return false;
			unsigned genomeSize;
			fs.read((char*)&genomeSize, sizeof(genomeSize));
			for (unsigned i=0; i<genomeSize; ++i) {
				readChromosome(outGenome, &fs);
			}
			fs.close();
		} catch (...) {
			return false;
		}
		return true;
	}

}