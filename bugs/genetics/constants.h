#ifndef __constants_h__
#define __constants_h__

namespace constants {
	// chance that even a stable gene will be affected by mutation/deletion
	extern const double global_alteration_override_chance;

	// chance that a new chromosome will be spawned in the new genome (filled with random genes)
	extern const double global_chance_new_chromosome;

	// initial values for chromosomes' meta-genes:
	extern const double initial_chromosome_delete;
	extern const double initial_chromosome_split;
	extern const double initial_chromosome_swap;
	extern const double initial_chromosome_spawn_gene;

	// dynamic alteration amplitudes for chromosomes' meta-genes:
	extern const double change_chromosome_delete;
	extern const double change_chromosome_split;
	extern const double change_chromosome_swap;
	extern const double change_chromosome_spawn_gene;

	// initial values for genes' meta-genes:
	extern const double initial_gene_mutate;
	extern const double initial_gene_transform;
	extern const double initial_gene_delete;
	extern const double initial_gene_swap;
	extern const double initial_gene_mutation_value;

	// dynamic alteration amplitudes for genes' meta-genes:
	extern const double change_gene_mutate;
	extern const double change_gene_transform;
	extern const double change_gene_delete;
	extern const double change_gene_swap;
	extern const double change_gene_mutation_value;

}

#endif  // __constants_h__
