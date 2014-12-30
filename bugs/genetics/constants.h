#ifndef __constants_h__
#define __constants_h__

namespace constants {
// chance that even a stable gene will be affected by mutation/deletion

	const double global_alteration_override_chance	=	0.0001;
	const double global_chance_new_chromosome		=	0.005;

	// initial values for chromosomes' meta-genes:
	const double initial_chromosome_delete			=	0.001;
	const double initial_chromosome_split			=	0.0005;
	const double initial_chromosome_swap			=	0.005;
	const double initial_chromosome_spawn_gene		=	0.01;

	// dynamic alteration amplitudes for chromosomes' meta-genes:
	const double change_chromosome_delete			=	0.0001;
	const double change_chromosome_split			=	0.0002;
	const double change_chromosome_swap				=	0.0003;
	const double change_chromosome_spawn_gene		=	0.002;

	// initial values for genes' meta-genes:
	const double initial_gene_mutate				=	0.02;
	const double initial_gene_transform				=	0.005;
	const double initial_gene_delete				=	0.0005;
	const double initial_gene_swap					=	0.01;
	const double initial_gene_mutation_value		=	0.01;

	// dynamic alteration amplitudes for genes' meta-genes:
	const double change_gene_mutate					=	0.005;
	const double change_gene_transform				=	0.0005;
	const double change_gene_delete					=	0.0001;
	const double change_gene_swap					=	0.002;
	const double change_gene_mutation_value			=	0.005;

	const unsigned MAX_GROWTH_DEPTH					=	12;

}

#endif  // __constants_h__
