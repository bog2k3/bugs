#ifndef __constants_h__
#define __constants_h__

namespace constants {

	// chance that even a stable gene will be affected by mutation/deletion
	constexpr float global_alteration_override_chance	=	0.0001f;
	constexpr float global_chance_to_spawn_gene			=	0.01f;

	// initial values for genes' meta-genes:
	constexpr float initial_gene_mutate					=	0.015f;		// chance to mutate gene
	constexpr float initial_gene_delete					=	0.002f;		// chance to delete gene
	constexpr float initial_gene_swap					=	0.01f;		// chance to swap gene
	constexpr float initial_gene_mutation_value			=	0.01f;		// max value +/- by which a gene mutates (for ints it's always +/-1)

	constexpr float small_gene_value					=	initial_gene_mutation_value * 0.5;	// an small enough initial value for a gene comparable to its mutation deviation
	constexpr float FBOOL_true							=	small_gene_value;	// true value for float-encoded bool genes
	constexpr float FBOOL_false							=	-FBOOL_true;		// false value -||-

	// dynamic alteration amplitudes for genes' meta-genes:
	constexpr float change_gene_mutate					=	0.005f;		// how the mutate meta gene changes
	constexpr float change_gene_delete					=	0.0001f;	// ...
	constexpr float change_gene_swap					=	0.002f;
	constexpr float change_gene_mutation_value			=	0.005f;

	constexpr unsigned MAX_DIVISION_DEPTH				=	16;			// maximum times a cell can divide

}

#endif  // __constants_h__
