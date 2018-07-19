/*
 * BugGenome.cpp
 *
 *  Created on: Dec 30, 2015
 *      Author: bog
 */

#include "Bug.h"
#include "../../genetics/Gene.h"
#include "../../genetics/GeneDefinitions.h"
#include "../../genetics/constants.h"
#include "../../neuralnet/functions.h"
#include "../../body-parts/BodyConst.h"

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
	constexpr float musclePeriod = 3.f; // seconds
	constexpr float gripper_signal_threshold = -0.55f;
	constexpr float gripper_signal_phase_offset = 1.0f * PI;
	constexpr float bodyLegMuscleIn_VMScoord = 40.f;
	constexpr float bodyLegMuscleOut_VMScoord = 30.f;
	constexpr float leg1MuscleIn_VMScoord = 50.f;
	constexpr float leg1MuscleOut_VMScoord = 60.f;
	constexpr float leg2MuscleIn_VMScoord = 70.f;
	constexpr float leg2MuscleOut_VMScoord = 90.f;
	constexpr float head_VMSoffset = 100.f;			// vms offset for cells #4a and #4b
	constexpr float body_VMSoffset = 200.f;			// vms offset for cells #3b and #3c
	constexpr float mouth_size_ratio = 0.25f;		// mouth to head ratio (M3/#3a)
	constexpr float nose_size_ratio = 0.1f;			// nose to head ratio (N5/#4a)
	constexpr float egglayer_size_ratio = 0.01f;	// egglayer to body ratio (E1/#1)
	constexpr float head_size_ratio = 0.2f;			// head to body ratio (#2b/#2a)
	constexpr float leg_size_ratio = 2.0f;			// leg to torso ratio (#5b/B5)
	constexpr float legJoint_mass_ratio = 0.03f;		// joint ratio to parent cell
	constexpr float legMuscle_mass_ratio = 0.1f;	// muscle mass ratio relative to parent cell
	constexpr float fat_torso_ratio = 0.3f;			// fat to torso ratio (#4f/#4c)

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

	GeneDivisionParam gdp;
	gdp.param = GENE_DIVISION_MIRROR;
	gdp.value.set(2*constants::FBOOL_true);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(2.f / egglayer_size_ratio - 1.f);
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
	gp.weight.set(+sfu);
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

	gp.protein = GENE_PROT_X;					// move mouth from +x to -x
	gp.weight.set(-2*sfu);
	gp.restriction = BranchRestriction("0v 0v");
	PUSH(gp);

	gp.protein = GENE_PROT_Y;					// and from -y to +y
	gp.weight.set(+2*sfu);
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
	ga.value.set(0.f);							// this will link to the default #0 neuron in egg-layer cell which is unused
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD2;		// egg-layer suppress release signal
	ga.value.set(0.f);							// this will link to the default #0 neuron in egg-layer cell which is unused
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
	gdp.value.set(0);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(-PI/3);
	gdp.restriction = BranchRestriction("*v *< 0v");
	PUSH(gdp);

	gdp.param = GENE_DIVISION_REORIENT;
	gdp.value.set(constants::FBOOL_false);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(1.f);
	gdp.restriction.clear();
	PUSH(gdp);

	gdp.param = GENE_DIVISION_RATIO;
	gdp.value.set(2*fat_torso_ratio - 1.f);
	gdp.restriction = BranchRestriction("*v *< 0v");
	PUSH(gdp);

	GeneVMSOffset gvo;
	gvo.value.set(body_VMSoffset);
	gvo.restriction.clear();
	PUSH(gvo);

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
	gdp.value.set(PI/2.5);
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
	gp.weight.set(-sfu);
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.restriction.clear();
	gp.weight.set(-sfu);
	PUSH(gp);

	PUSH(GeneStop());

	OFFSET_MARKER(C4e)	//------------------------------- MARKER ----------------

	gdp.param = GENE_DIVISION_ANGLE;
	gdp.value.set(PI/2);
	gdp.restriction.clear();
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

	gdp.param = GENE_DIVISION_BOND_BIAS;
	gdp.value.set(1.2f);
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
	//ga.value.set(PI/1.1);
	ga.value.set(PI);
	ga.restriction = BranchRestriction("0v 0v 0v 0v *-");
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
	gja.value.set(BodyConst::initialJointMaxPhi * 0.1f);
	gja.restriction.clear();
	PUSH(gja);

	gja.attrib = GENE_JOINT_ATTR_LOW_LIMIT;
	gja.value.set(BodyConst::initialJointMinPhi * 4.5f);
	gja.restriction.clear();
	PUSH(gja);

	gp.protein = GENE_PROT_X;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(-2*sfu);
	gp.restriction = BranchRestriction("0v 0v 0v 0v 0v 0v 0>");
	PUSH(gp);

	gp.protein = GENE_PROT_Z;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(1.8f * BodyConst::initialBoneAspectRatio);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(0.4f * BodyConst::initialBoneAspectRatio);
	ga.restriction = BranchRestriction("*v *v *v *v *<");
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_GENERIC1;
	ga.value.set(BodyConst::initialBoneDensity);
	ga.restriction.clear();
	PUSH(ga);

	GeneMuscleAttribute gma;

	gma.attrib = GENE_MUSCLE_ATTR_INSERT_OFFSET1;
	gma.side.set(0);
	gma.value.set(BodyConst::initialMuscleInsertOffset);
	gma.restriction.clear();
	PUSH(gma);

	gma.attrib = GENE_MUSCLE_ATTR_INSERT_OFFSET2;
	gma.side.set(0);
	gma.value.set(BodyConst::initialMuscleInsertOffset);
	gma.restriction.clear();
	PUSH(gma);

	gma.attrib = GENE_MUSCLE_ATTR_MASS_RATIO;
	gma.side.set(0);
	gma.value.set(legMuscle_mass_ratio);
	gma.restriction.clear();
	PUSH(gma);

	ga.attribute = GENE_ATTRIB_VMS_COORD1;		// left (inner muscle) from body to first leg segment
	ga.restriction = BranchRestriction("*v *v *v *v *<");
	ga.value.set(bodyLegMuscleIn_VMScoord);
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD1;		// left (inner muscle) from first leg segment to second
	ga.restriction = BranchRestriction("*v *v *v *v *> *<");
	ga.value.set(leg1MuscleIn_VMScoord);
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD1;		// left (inner muscle) from 2nd leg segment to gripper
	ga.restriction = BranchRestriction("*v *v *v *v *> *> *<");
	ga.value.set(leg2MuscleIn_VMScoord);
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD2;	// right (outer muscle)
	ga.restriction = BranchRestriction("*v *v *v *v *<");
	ga.value.set(bodyLegMuscleOut_VMScoord);
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD2;		// right (outer muscle) from first leg segment to second
	ga.restriction = BranchRestriction("*v *v *v *v *> *<");
	ga.value.set(leg1MuscleOut_VMScoord);
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_VMS_COORD2;		// right (outer muscle) from 2nd leg segment to gripper
	ga.restriction = BranchRestriction("*v *v *v *v *> *> *<");
	ga.value.set(leg2MuscleOut_VMScoord);
	PUSH(ga);

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

	gvo.value.set(head_VMSoffset);
	gvo.restriction.clear();
	PUSH(gvo);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI/4);
	ga.restriction = BranchRestriction("*v *v *v *v 0>");
	PUSH(ga);

	gp.protein = GENE_PROT_X;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Y;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_Z;
	gp.weight.set(+sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(-sfu);
	gp.restriction.clear();
	PUSH(gp);

	gp.protein = GENE_PROT_W;
	gp.weight.set(2*sfu);
	gp.restriction = BranchRestriction("*v *v *v *v *<");
	PUSH(gp);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialBoneAspectRatio);
	ga.restriction.clear();
	PUSH(ga);

	ga.attribute = GENE_ATTRIB_GENERIC1;
	ga.value.set(BodyConst::initialBoneDensity);
	ga.restriction.clear();
	PUSH(ga);

	BranchRestriction branchLRHeadBone = BranchRestriction("*v *v *v *v *<");

	// neuron #0 transfer function
	GeneTransferFunction gtf;
	gtf.restriction = branchLRHeadBone;
	gtf.neuronLocation.set(0);								// #0, #0' [+/-100]
	gtf.functionID.set((int)transferFuncNames::FN_ONE);
	PUSH(gtf);

	// neuron #0 bias
	GeneNeuralBias gnb;
	gnb.restriction.clear();
	gnb.neuronLocation.set(0);								// #0, #0' [+/-100]
	gnb.value.set(0);
	PUSH(gnb);

	GeneNeuron gn;
	gn.restriction = branchLRHeadBone;
	gn.neuronLocation.set(-10);								// #1, #1' [+/-110]
	PUSH(gn);

	// neuron #1/#1' transfer:
	gtf.restriction.clear();
	gtf.neuronLocation.set(-10);
	gtf.functionID.set((int)transferFuncNames::FN_SIGMOID);
	PUSH(gtf);

	// neuron #1/#1' sigmoid steepness
	GeneNeuralParam gnp;
	gnp.restriction.clear();
	gnp.neuronLocation.set(-10);
	gnp.value.set(10);
	PUSH(gnp);

	// neuron #1/#1' bias
	gnb.restriction.clear();
	gnb.neuronLocation.set(-10);
	gnb.value.set(0);
	PUSH(gnb);

	gn.restriction = branchLRHeadBone;
	gn.neuronLocation.set(-20);								// #2, #2' [+/-120]
	PUSH(gn);

	// neuron #2 transfer:
	gtf.restriction.clear();
	gtf.neuronLocation.set(-20);
	gtf.functionID.set((int)transferFuncNames::FN_GATE);
	PUSH(gtf);

	// neuron #2 gate threshold
	gnb.restriction.clear();
	gnb.neuronLocation.set(-20);
	gnb.value.set(0);
	PUSH(gnb);

	// neuron #2 gate switch steepness
	gnp.restriction.clear();
	gnp.neuronLocation.set(-20);
	gnp.value.set(10);
	PUSH(gnp);

	gn.restriction = branchLRHeadBone;
	gn.neuronLocation.set(-30);								// #3, #3' [+/-130]
	PUSH(gn);

	// neuron #3 transfer:
	gtf.restriction.clear();
	gtf.neuronLocation.set(-30);
	gtf.functionID.set((int)transferFuncNames::FN_GATE);
	PUSH(gtf);

	// neuron #3 gate threshold
	gnb.restriction.clear();
	gnb.neuronLocation.set(-30);
	gnb.value.set(0);
	PUSH(gnb);

	// neuron #3 gate switch steepness
	gnp.restriction.clear();
	gnp.neuronLocation.set(-30);
	gnp.value.set(10);
	PUSH(gnp);

	gn.restriction = branchLRHeadBone;
	gn.neuronLocation.set(-40);								// #4, #4' [+/-140]
	PUSH(gn);

	// neuron #4 transfer:
	gtf.restriction.clear();
	gtf.neuronLocation.set(-40);
	gtf.functionID.set((int)transferFuncNames::FN_POW);
	PUSH(gtf);

	// neuron #4 param:
	gnp.restriction.clear();
	gnp.neuronLocation.set(-40);
	gnp.value.set(-1);
	PUSH(gnp);

	// neuron #4 bias (to avoid division by 0)
	gnb.restriction.clear();
	gnb.neuronLocation.set(-40);
	gnb.value.set(1.e-10f);
	PUSH(gnb);

	gn.restriction = branchLRHeadBone;
	gn.neuronLocation.set(-50);								// #5, #5' [+/-150]
	PUSH(gn);

	// neuron #5 transfer:
	gtf.restriction.clear();
	gtf.neuronLocation.set(-50);
	gtf.functionID.set((int)transferFuncNames::FN_MODULATE);
	PUSH(gtf);

	// neuron #5 bias
	gnb.restriction.clear();
	gnb.neuronLocation.set(-50);
	gnb.value.set(0);
	PUSH(gnb);


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
