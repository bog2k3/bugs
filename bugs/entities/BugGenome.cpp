/*
 * BugGenome.cpp
 *
 *  Created on: Dec 30, 2015
 *      Author: bog
 */

#include "Bug.h"
#include "../genetics/Gene.h"
#include "../genetics/GeneDefinitions.h"
#include "../genetics/constants.h"
#include "../neuralnet/functions.h"
#include "../body-parts/BodyConst.h"

#include <boglfw/utils/log.h>

#include <map>

Chromosome Bug::createBasicChromosome() {
//  ------- helpers ------------------
	Chromosome c;

	struct offsetInsertion {
		unsigned partOffset;
		unsigned geneIndex;
		std::string markerName;
		offsetInsertion(unsigned partOffs, unsigned geneIndex, std::string const& markerName)
			: partOffset(partOffs), geneIndex(geneIndex), markerName(markerName) {
		}
	};

	unsigned crtOffset;
	std::map<std::string, unsigned> offsetMarkers;
	std::vector<offsetInsertion> insertions;

#define OFFSET_MARKER(name) { crtOffset = c.genes.size(); offsetMarkers[#name] = crtOffset; }
#define INSERT_OFFSET(targetMarker) { insertions.push_back(offsetInsertion(crtOffset, c.genes.size(), #targetMarker)); }
#define PUSH(g) c.genes.push_back(g)

//  ------- constants -------------------------

	constexpr float body_min_fat_ratio = 0.1f;
	constexpr float body_adult_lean_mass = 4; // kg
	constexpr float leftNose_VMScoord = 100;
	constexpr float rightNose_VMScoord = 200;
	constexpr float time_VMScoord = Bug::defaultConstants::lifetimeSensor_vmsCoord;
	constexpr float leftLeg_pull_VMScoord = 50;
	constexpr float leftLeg_push_VMScoord = 100;
	constexpr float rightLeg_pull_VMScoord = 150;
	constexpr float rightLeg_push_VMScoord = 200;
	constexpr float leftGripper_VMScoord = 250;
	constexpr float rightGripper_VMScoord = 300;
	constexpr float musclePeriod = 3.f; // seconds
	constexpr float gripper_signal_threshold = -0.55f;
	constexpr float gripper_signal_phase_offset = 1.0f * PI;
	constexpr float mouth_size_ratio = 0.05f;

//  ------- genome begins -------------------------

	OFFSET_MARKER(C0)	//------------------------------- MARKER ----------------

	//body attributes
	GeneBodyAttribute gba;
	gba.attribute = GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO;
	gba.value.set(body_init_fat_ratio);
	PUSH(gba);

	gba.attribute = GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO;
	gba.value.set(body_min_fat_ratio);
	PUSH(gba);

	gba.attribute = GENE_BODY_ATTRIB_ADULT_LEAN_MASS;
	gba.value.set(body_adult_lean_mass);
	PUSH(gba);

	gba.attribute = GENE_BODY_ATTRIB_GROWTH_SPEED;
	gba.value.set(BodyConst::initialGrowthSpeed);
	PUSH(gba);

	gba.attribute = GENE_BODY_ATTRIB_EGG_MASS;
	gba.value.set(BodyConst::initialEggMass);
	PUSH(gba);

	gba.attribute = GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO;
	gba.value.set(BodyConst::initialReproductiveMassRatio);
	PUSH(gba);

	GeneDivisionParam gdp;
	gdp.param = GENE_DIVISION_AFFINITY;
	//gdp.restriction = BranchRestriction("*- L- *- *- *- 0< RX");
	gdp.value.set(constants::FBOOL_true);
	PUSH(gdp);

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(-PI/2);
	PUSH(gdp);

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(constants::FBOOL_true);
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_false);
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(1.f - mouth_size_ratio);
	PUSH(gdp);

	gdp.param = GENE_DIVISION_SEPARATE;
	gdp.value.set(constants::FBOOL_false);
	PUSH(gdp);

	GeneOffset go;
	go.side = constants::FBOOL_true;	// true (positive) means left
	INSERT_OFFSET(C1L);
	PUSH(go);

	go.side = constants::FBOOL_false;	// false (negative) means right
	INSERT_OFFSET(EGG_LAYER);
	PUSH(go);

	PUSH(GeneStop());

	OFFSET_MARKER(EGG_LAYER)	//------------------------------- MARKER ----------------

	GeneProtein gp;
	gp.weight.set(1);
	gp.protein.set(GENE_PROT_)


	PUSH(GeneStop());
	PUSH(GeneStop());

//  ------- finished genome; house-keeping from here on -----------------

	// generate and insert offsets:
	for (offsetInsertion &i : insertions) {
		assertDbg(offsetMarkers.find(i.markerName) != offsetMarkers.end());	// if this jumps, a marker referred by an offset gene can't be found
#ifdef DEBUG
//		LOGLN("Marker offset [" << i.markerName << "] = " << offsetMarkers[i.markerName]);
#endif
		Atom<int> *pVal = &c.genes[i.geneIndex].data.gene_offset.offset;
//		if (i.jointOffs)
//			pVal = &c.genes[i.geneIndex].data.gene_joint_offset.offset;
		pVal->set(offsetMarkers[i.markerName] - i.partOffset);
	}

	// now we need to add some redundancy in between genes:
	if (false) {
		int padding = 2;
		for (uint i=0; i<c.genes.size(); i+=padding+1) {
			for (int k=0; k<padding; k++)
				c.genes.insert(c.genes.begin()+i+1, GeneNoOp());
			if (c.genes[i].type == gene_type::SKIP) {
				c.genes[i].data.gene_skip.count.set(c.genes[i].data.gene_skip.count * (padding+1));
			}
			if (c.genes[i].type == gene_type::OFFSET) {
				c.genes[i].data.gene_offset.offset.set(c.genes[i].data.gene_offset.offset * (padding+1));
			}
//			if (c.genes[i].type == gene_type::JOINT_OFFSET) {
//				c.genes[i].data.gene_joint_offset.offset.set(c.genes[i].data.gene_joint_offset.offset * (padding+1));
//			}
		}
	}

	return c;
}

