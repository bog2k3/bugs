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
#include <boglfw/math/constants.h>

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

#define OFFSET_MARKER(name) { offsetMarkers[#name] = c.genes.size(); }
#define INSERT_OFFSET(targetMarker, sourceMarker) { insertions.push_back(offsetInsertion(offsetMarkers[#sourceMarker], c.genes.size(), #targetMarker)); }
#define PUSH(g) c.genes.push_back(g)

#warning "make sure inserted offsets are relative to parent cell's offset"

//  ------- constants -------------------------

	constexpr float body_min_fat_ratio = 0.1f;
	constexpr float body_adult_lean_mass = 4; // kg
	constexpr float nose_VMScoord = 10;
	constexpr float rightNose_VMSoffs = 20;
	constexpr float time_VMScoord = Bug::defaultConstants::lifetimeSensor_vmsCoord;
	constexpr float leg_pull_VMScoord = 100;
	constexpr float leg_push_VMScoord = 120;
	constexpr float gripper_VMScoord = 150;
	constexpr float rightLeg_VMSoffs = 70;
	constexpr float egglayer_sig1_VMScoord = 500;
	constexpr float egglayer_sig2_VMScoord = 530;
	constexpr float musclePeriod = 3.f; // seconds
	constexpr float gripper_signal_threshold = -0.55f;
	constexpr float gripper_signal_phase_offset = 1.0f * PI;
	constexpr float mouth_size_ratio = 0.25f;		// mouth to head ratio (M3/#3a)
	constexpr float nose_size_ratio = 0.1f;			// nose to head ratio (N5/#4a)
	constexpr float egglayer_size_ratio = 0.005f;	// egglayer to body ratio (E1/#1)
	constexpr float head_size_ratio = 0.2f;			// head to body ratio (#2b/#2a)
	constexpr float leg_size_ratio = 2.5f;			// leg to torso ratio (#5b/B5)
	constexpr float legJoint_mass_ratio = 0.1f;		// joint ratio to parent cell
	constexpr float legMuscle_mass_ratio = 0.1f;	// muscle mass ratio relative to parent cell

	constexpr float sfu = constants::small_gene_value;	// small float unit

//  ------- genome begins -------------------------

	OFFSET_MARKER(C0)	//------------------------------- MARKER ----------------

	//body attributes
	GeneBodyAttribute gba;
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

	// neural genes start here --------------------------------------
	// todo
	// neural genes end here --------------------------------------

	GeneDivisionParam gdp;
	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(2*constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(1.f / egglayer_size_ratio);
	gdp.restriction.clear();
	PUSH(gdp);

	GeneOffset go;
	go.side.set(constants::FBOOL_true);	// true (positive) means left
	go.restriction.clear();
	INSERT_OFFSET(C1, C0);
	PUSH(go);

	go.side.set(constants::FBOOL_false);	// false (negative) means right
	go.restriction.clear();
	INSERT_OFFSET(EGG_LAYER, C0);
	PUSH(go);

	OFFSET_MARKER(C1)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(3*PI/2);
	gdp.restriction = BranchRestriction("0v L-");
	PUSH(gdp);

	go.side.set(constants::FBOOL_true);
	go.restriction = BranchRestriction("0v");	// don't apply to C0
	INSERT_OFFSET(C2a, C1);
	PUSH(go);

	go.side.set(constants::FBOOL_false);
	go.restriction = BranchRestriction("0v");	// don't apply to C0
	INSERT_OFFSET(C2b, C1);
	PUSH(go);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.restriction = BranchRestriction("0v");	// don't apply to C0
	gdp.value.set(1.f / head_size_ratio);
	PUSH(gdp);

	OFFSET_MARKER(C2b)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(-PI/2);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_SEPARATE;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(mouth_size_ratio);
	gdp.restriction = BranchRestriction("0v 0v");	// don't apply to C0 and C1
	PUSH(gdp);

	GeneAttribute ga;
	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	ga.restriction.clear();
	PUSH(ga);

	go.side.set(constants::FBOOL_true);
	go.restriction = BranchRestriction("0v 0v");	// don't apply to C0 and C1
	INSERT_OFFSET(MOUTH, C2b);
	PUSH(go);

	go.side.set(constants::FBOOL_false);
	go.restriction = BranchRestriction("0v 0v");	// don't apply to C0 and C1
	INSERT_OFFSET(C3a, C2b);
	PUSH(go);

	PUSH(GeneStop());

	GeneProtein gp;

	OFFSET_MARKER(EGG_LAYER)	//------------------------------- MARKER ----------------
	OFFSET_MARKER(MOUTH)		//------------------------------- MARKER ----------------

	gp.protein = GENE_PROT_X;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Z;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Z;					// move mouth from +z to -z
	gp.weight.set(-2*sfu);
	gp.restriction = BranchRestriction("0v 0v");
	PUSH(gp);

	gp.protein = GENE_PROT_W;					// and from +w to -w
	gp.weight.set(-2*sfu);
	gp.restriction = BranchRestriction("0v 0v");
	PUSH(gp);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_GENERIC1;
	ga.value.set(BodyConst::initialEggEjectSpeed);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD1;		// egg-layer suppress growth signal
	ga.value.set(egglayer_sig1_VMScoord);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD2;		// egg-layer suppress release signal
	ga.value.set(egglayer_sig2_VMScoord);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMouthAspectRatio);
	ga.restriction.clear();
	PUSH(ga);

	PUSH(GeneStop());

	OFFSET_MARKER(C2a)	//------------------------------- MARKER ----------------
	OFFSET_MARKER(C3a)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(2*constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	go.side.set(0);
	go.restriction = BranchRestriction("0v 0<");	// don't apply to C0, C1 and right subtree of C1
	INSERT_OFFSET(C3bc, C2a);
	PUSH(go);

	go.side.set(0);
	go.restriction = BranchRestriction("0v 0>");	// don't apply to C0, C1 and left subtree of C1
	INSERT_OFFSET(C4ab, C3a);
	PUSH(go);

	OFFSET_MARKER(C3bc)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(0.f);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(0.5f);
	gdp.restriction.clear();
	PUSH(gdp);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	ga.restriction.clear();
	PUSH(ga);

	go.side.set(constants::FBOOL_true);
	go.restriction = BranchRestriction("0v 0< 0v");	// don't apply to C0, C1, right subtree of C1 and C2a
	INSERT_OFFSET(C4f, C3bc);
	PUSH(go);

	go.side.set(constants::FBOOL_false);
	go.restriction = BranchRestriction("0v 0< 0v");	// don't apply to C0, C1, right subtree of C1 and C2a
	INSERT_OFFSET(C4e, C3bc);
	PUSH(go);

	PUSH(GeneStop());

	OFFSET_MARKER(C4f)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(PI/2);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction = BranchRestriction("0v 0v 0v 0v 0v");	// only apply to levels 5+
	PUSH(gdp);

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(1.f);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_SEPARATE;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	ga.restriction.clear();
	PUSH(ga);

	gp.protein = GENE_PROT_X;
	gp.restriction.clear();
	gp.weight.set(-sfu);
	PUSH(gp);

	gp.protein = GENE_PROT_Y;
	gp.restriction.clear();
	gp.weight.set(-sfu);
	PUSH(gp);

	gp.protein = GENE_PROT_Z;
	gp.restriction.clear();
	gp.weight.set(+sfu);
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.restriction.clear();
	gp.weight.set(-sfu);
	PUSH(gp);

	PUSH(GeneStop());

	OFFSET_MARKER(C4e)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(PI/4);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(3*PI/4);
	gdp.restriction = BranchRestriction("0v 0v 0v 0v 0v");	// 3PI/4 averaged with the above value of PI/4 yield PI/2
	PUSH(gdp);

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction = BranchRestriction("0v 0v 0v 0v 0v Lv Lv");
	PUSH(gdp);

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(leg_size_ratio);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_SEPARATE;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI);
	ga.restriction = BranchRestriction("0v 0v 0v 0v *-");
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(-PI/4);
	ga.restriction = BranchRestriction("0v 0v 0v 0v 0v R-");
	PUSH(ga);

	GeneJointAttribute gja;
	gja.attrib = GENE_JOINT_ATTR_TYPE;
	gja.value.set(constants::FBOOL_true);
	gja.restriction.clear();
	PUSH(gja);

	gja.attrib = GENE_JOINT_ATTR_MASS_RATIO;
	gja.value.set(legJoint_mass_ratio);
	gja.restriction.clear();
	PUSH(gja);

	gja.attrib = GENE_JOINT_ATTR_RESET_TORQUE;
	gja.value.set(BodyConst::initialJointResetTorque);
	gja.restriction.clear();
	PUSH(gja);

	gja.attrib = GENE_JOINT_ATTR_DENSITY;
	gja.value.set(BodyConst::initialJointDensity);
	gja.restriction.clear();
	PUSH(gja);

	gja.attrib = GENE_JOINT_ATTR_HIGH_LIMIT;
	gja.value.set(BodyConst::initialJointMaxPhi);
	gja.restriction.clear();
	PUSH(gja);

	gja.attrib = GENE_JOINT_ATTR_LOW_LIMIT;
	gja.value.set(BodyConst::initialJointMinPhi);
	gja.restriction.clear();
	PUSH(gja);

	gp.protein = GENE_PROT_X;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gdp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(sfu);
	gp.restriction.clear();
	PUSH(gdp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(-2*sfu);
	gp.restriction = BranchRestriction("0v 0v 0v 0v 0v 0v 0>");
	PUSH(gdp);

	gp.protein = GENE_PROT_Z;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gdp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gdp);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI);
	ga.restriction = BranchRestriction("0v 0v 0v 0v R-");
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(-PI/4);
	ga.restriction = BranchRestriction("0v 0v 0v 0v 0> R-");
	PUSH(ga);

	GeneMuscleAttribute gma;
	gma.attrib = GENE_MUSCLE_ATTR_ASPECT_RATIO;
	gma.side.set(0);
	gma.value.set(BodyConst::initialMuscleAspectRatio);
	gma.restriction.clear();
	PUSH(gma);

	gma.attrib = GENE_MUSCLE_ATTR_INSERT_OFFSET;
	gma.side.set(0);
	gma.value.set(BodyConst::initialMuscleInsertOffset);
	gma.restriction.clear();
	PUSH(gma);

	gma.attrib = GENE_MUSCLE_ATTR_MASS_RATIO;
	gma.side.set(0);
	gma.value.set(legMuscle_mass_ratio);
	gma.restriction.clear();
	PUSH(gma);

	gma.attrib = GENE_MUSCLE_ATTR_INPUT_COORD;
	gma.side.set(sfu);	// left (inner muscle)
	gma.restriction.clear();
	gma.value.set(leg_push_VMScoord);
	PUSH(gma);

	gma.attrib = GENE_MUSCLE_ATTR_INPUT_COORD;
	gma.side.set(-sfu);	// right (outer muscle)
	gma.restriction.clear();
	gma.value.set(leg_pull_VMScoord);
	PUSH(gma);

	GeneVMSOffset gvo;
	gvo.restriction = BranchRestriction("0v 0v 0v 0>");
	gvo.value.set(rightLeg_VMSoffs);
	PUSH(gvo);

	PUSH(GeneStop());

	OFFSET_MARKER(C4ab)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(3*PI/4);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_AFFINITY;
	gdp.value.set(constants::FBOOL_true);
	gdp.restriction = BranchRestriction("*v *v *v *v *-");
	PUSH(gdp);

	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(1.f / nose_size_ratio);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_SEPARATE;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI/4);
	ga.restriction = BranchRestriction("*v *v *v *v 0>");
	PUSH(ga);

	gvo.restriction = BranchRestriction("*v *v *v *>");
	gvo.value.set(rightNose_VMSoffs);
	PUSH(gvo);

	gp.protein = GENE_PROT_X;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Z;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Z;
	gp.weight.set(2*sfu);
	gp.restriction = BranchRestriction("*v *v *v *v *<");
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(2*sfu);
	gp.restriction = BranchRestriction("*v *v *v *v *<");
	PUSH(gp);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialBoneAspectRatio);
	ga.restriction.clear();

	ga.attribute = GENE_ATTRIB_GENERIC1;
	ga.value.set(BodyConst::initialBoneDensity);
	ga.restriction.clear();

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

