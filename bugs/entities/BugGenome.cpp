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
#include "../utils/log.h"

#include <map>

Chromosome Bug::createBasicChromosome() {
	Chromosome c;

	struct offsetInsertion {
		unsigned partOffset;
		unsigned geneIndex;
		std::string markerName;
		bool jointOffs;
		offsetInsertion(unsigned partOffs, unsigned geneIndex, std::string const& markerName, bool jointOffs)
			: partOffset(partOffs), geneIndex(geneIndex), markerName(markerName), jointOffs(jointOffs) {
		}
	};
	unsigned crtOffset;
	std::map<std::string, unsigned> partMarkers;
	std::vector<offsetInsertion> insertions;

#define PART_MARKER(name) { crtOffset = c.genes.size(); partMarkers[#name] = crtOffset; }
#define INSERT_OFFSET(targetMarker) { insertions.push_back(offsetInsertion(crtOffset, c.genes.size(), #targetMarker, false)); }
#define INSERT_JOFFSET(targetMarker) { insertions.push_back(offsetInsertion(crtOffset, c.genes.size(), #targetMarker, true)); }

	constexpr float body_size = 0.1f * 0.1f; // sq meters
	constexpr float body_init_fat_ratio = 0.5f;
	constexpr float body_min_fat_ratio = 0.1f;
	constexpr float body_adult_lean_mass = 4; // kg
	constexpr float leftNose_VMScoord = 10;
	constexpr float rightNose_VMScoord = 20;
	constexpr float time_VMScoord = Bug::defaultConstants::lifetimeSensor_vmsCoord;
	constexpr float leftLeg_pull_VMScoord = 5;
	constexpr float leftLeg_push_VMScoord = 10;
	constexpr float rightLeg_pull_VMScoord = 15;
	constexpr float rightLeg_push_VMScoord = 20;
	constexpr float leftGripper_VMScoord = 25;
	constexpr float rightGripper_VMScoord = 30;
	constexpr float musclePeriod = 3.f; // seconds
	constexpr float gripper_signal_threshold = -0.55f;
	constexpr float gripper_signal_phase_offset = 0.9f * PI;

	GeneOffset go;
	GeneJointOffset gjo;

	PART_MARKER(TORSO)

#ifdef ENABLE_START_MARKER_GENES
	GeneStartMarker gsm;
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// body size (sq meters)
	GeneAttribute ga;
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(body_size);
	c.genes.push_back(ga);

	//body attributes
	GeneBodyAttribute gba;
	gba.attribute = GENE_BODY_ATTRIB_INITIAL_FAT_MASS_RATIO;
	gba.value.set(body_init_fat_ratio);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_MIN_FAT_MASS_RATIO;
	gba.value.set(body_min_fat_ratio);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_ADULT_LEAN_MASS;
	gba.value.set(body_adult_lean_mass);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_GROWTH_SPEED;
	gba.value.set(BodyConst::initialGrowthSpeed);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_EGG_MASS;
	gba.value.set(BodyConst::initialEggMass);
	c.genes.push_back(gba);

	gba.attribute = GENE_BODY_ATTRIB_REPRODUCTIVE_MASS_RATIO;
	gba.value.set(BodyConst::initialReproductiveMassRatio);
	c.genes.push_back(gba);

	// neural system
	GeneTransferFunction gt;
	GeneNeuronInputCoord gic;
	GeneNeuronOutputCoord goc;
	GeneNeuralBias gnb;
	GeneNeuralParam gnp;
	GeneSynapse gs;

	// neuron #0 transfer:
	gt.targetNeuron.set(0);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #0 input:
	gic.destNeuronVirtIndex.set(0);
	gic.inCoord.set(leftNose_VMScoord);
	c.genes.push_back(gic);

	// neuron #1 transfer:
	gt.targetNeuron.set(1);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #1 input:
	gic.destNeuronVirtIndex.set(1);
	gic.inCoord.set(rightNose_VMScoord);
	c.genes.push_back(gic);

	// neuron #2 transfer:
	gt.targetNeuron.set(2);
	gt.functionID.set((int)transferFuncNames::FN_THRESHOLD);
	c.genes.push_back(gt);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set((int)transferFuncNames::FN_THRESHOLD);
	c.genes.push_back(gt);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set((int)transferFuncNames::FN_POW);
	c.genes.push_back(gt);
	// neuron #4 param:
	gnp.targetNeuron.set(4);
	gnp.value.set(-1);
	c.genes.push_back(gnp);

	// neuron #5 transfer:
	gt.targetNeuron.set(5);
	gt.functionID.set((int)transferFuncNames::FN_MODULATE);
	c.genes.push_back(gt);

	// neuron #6 transfer:
	gt.targetNeuron.set(6);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #6 input:
	gic.destNeuronVirtIndex.set(6);
	gic.inCoord.set(time_VMScoord);
	c.genes.push_back(gic);

	// neuron #7 transfer:
	gt.targetNeuron.set(7);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #7 bias:
	gnb.targetNeuron.set(7);
	gnb.value.set(gripper_signal_phase_offset);
	c.genes.push_back(gnb);

	// neuron #8 transfer:
	gt.targetNeuron.set(8);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #8 bias:
	gnb.targetNeuron.set(8);
	gnb.value.set(gripper_signal_threshold);
	c.genes.push_back(gnb);
	// neuron #8 output:
	goc.srcNeuronVirtIndex.set(8);
	goc.outCoord.set(leftGripper_VMScoord);
	c.genes.push_back(goc);

	// neuron #9 transfer:
	gt.targetNeuron.set(9);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #9 bias:
	gnb.targetNeuron.set(9);
	gnb.value.set(gripper_signal_phase_offset);
	c.genes.push_back(gnb);

	// neuron #10 transfer:
	gt.targetNeuron.set(10);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #10 bias:
	gnb.targetNeuron.set(10);
	gnb.value.set(gripper_signal_threshold);
	c.genes.push_back(gnb);
	// neuron #10 output:
	goc.srcNeuronVirtIndex.set(10);
	goc.outCoord.set(rightGripper_VMScoord);
	c.genes.push_back(goc);

	// neuron #11 transfer:
	gt.targetNeuron.set(11);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #11 bias:
	gnb.targetNeuron.set(11);
	gnb.value.set(PI);
	c.genes.push_back(gnb);

	// neuron #12 transfer:
	gt.targetNeuron.set(12);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #12 output:
	goc.srcNeuronVirtIndex.set(12);
	goc.outCoord.set(leftLeg_pull_VMScoord);
	c.genes.push_back(goc);

	// neuron #13 transfer:
	gt.targetNeuron.set(13);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #13 bias:
	gnb.targetNeuron.set(13);
	gnb.value.set(0.5f);
	c.genes.push_back(gnb);

	// neuron #14 transfer:
	gt.targetNeuron.set(14);
	gt.functionID.set((int)transferFuncNames::FN_MODULATE);
	c.genes.push_back(gt);
	// neuron #14 output:
	goc.srcNeuronVirtIndex.set(14);
	goc.outCoord.set(leftLeg_push_VMScoord);
	c.genes.push_back(goc);

	// neuron #15 transfer:
	gt.targetNeuron.set(15);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #15 bias:
	gnb.targetNeuron.set(15);
	gnb.value.set(PI);
	c.genes.push_back(gnb);

	// neuron #16 transfer:
	gt.targetNeuron.set(16);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #16 output:
	goc.srcNeuronVirtIndex.set(16);
	goc.outCoord.set(rightLeg_pull_VMScoord);
	c.genes.push_back(goc);

	// neuron #17 transfer:
	gt.targetNeuron.set(17);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #17 bias:
	gnb.targetNeuron.set(17);
	gnb.value.set(0.5f);
	c.genes.push_back(gnb);

	// neuron #18 transfer:
	gt.targetNeuron.set(18);
	gt.functionID.set((int)transferFuncNames::FN_MODULATE);
	c.genes.push_back(gt);
	// neuron #18 output:
	goc.srcNeuronVirtIndex.set(18);
	goc.outCoord.set(rightLeg_push_VMScoord);
	c.genes.push_back(goc);

	// synapse 0 to 2
	gs.from.set(0);
	gs.to.set(2);
	gs.weight.set(2*PI / musclePeriod);
	c.genes.push_back(gs);



	// grow Mouth:
	GeneProtein gp;
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_B);	// X+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(0);
	INSERT_OFFSET(MOUTH)
	c.genes.push_back(go);

	// grow Bone(8):
	gp.targetSegment.set(8);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(8);
	INSERT_OFFSET(TORSO_BONE8)
	c.genes.push_back(go);

	// grow Muscle(7):
	gp.targetSegment.set(7);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(7);
	INSERT_OFFSET(TORSO_MUSCLE_LEFT)
	c.genes.push_back(go);

	// grow Muscle(9):
	gp.targetSegment.set(9);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(9);
	INSERT_OFFSET(TORSO_MUSCLE_RIGHT)
	c.genes.push_back(go);

	// grow Egg Layer:
	gp.targetSegment.set(5);
	gp.protein.set(GENE_PROT_B);	// X+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(5);
	INSERT_OFFSET(EGGLAYER)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(MOUTH)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialMouthSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMouthAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(EGGLAYER)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_EGG_EJECT_SPEED;
	ga.value.set(BodyConst::initialEggEjectSpeed);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for first bone:
	gjo.minDepth.set(2);
	gjo.maxDepth.set(2);
	INSERT_JOFFSET(TORSO_JOINT8)
	c.genes.push_back(gjo);

	ga.minDepth.set(2);
	ga.maxDepth.set(2);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI/8);
	c.genes.push_back(ga);

	// attribs for second bone:
	gjo.minDepth.set(4);
	gjo.maxDepth.set(4);
	INSERT_JOFFSET(TORSO_JOINT8)
	c.genes.push_back(gjo);

	ga.maxDepth.set(4);
	ga.minDepth.set(4);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	// common attribs for 1st bone, 2nd bone
	ga.minDepth.set(2);
	ga.maxDepth.set(4);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_DENSITY;
	ga.value.set(BodyConst::initialBoneDensity);
	c.genes.push_back(ga);

	// (on 2nd bone) grow Gripper(0):
	gp.minDepth.set(4);
	gp.maxDepth.set(4);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(4);
	go.targetSegment.set(0);
	INSERT_OFFSET(TORSO_BONE8_BONE0_GRIPPER0)
	c.genes.push_back(go);

	// (on 1st bone) grow 2nd Bone(0);
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(2);
	go.maxDepth.set(2);
	go.targetSegment.set(0);
	INSERT_OFFSET(TORSO_BONE8)	// loop
	c.genes.push_back(go);

	// (on both bones) grow Muscle(1):
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(2);
	go.maxDepth.set(4);
	go.targetSegment.set(1);
	INSERT_OFFSET(TORSO_MUSCLE_RIGHT)
	c.genes.push_back(go);

	// (on both bones) grow Muscle(15):
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(2);
	go.maxDepth.set(4);
	go.targetSegment.set(15);
	INSERT_OFFSET(TORSO_MUSCLE_LEFT)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_JOINT8)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st joint:
	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 3);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(0.1f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(4 * BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	// attribs for 2nd joint:
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 2);
	c.genes.push_back(ga);

	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi*0.5f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	// attribs for 3rd joint
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 2);
	c.genes.push_back(ga);

	// common attribs for all joints:
	ga.minDepth.set(1);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_BONE0_GRIPPER0)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(6);
	ga.maxDepth.set(6);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	gjo.minDepth.set(6);
	gjo.maxDepth.set(6);
	INSERT_JOFFSET(TORSO_JOINT8)
	c.genes.push_back(gjo);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(gripper_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_MUSCLE_LEFT)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st left muscle
	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(6 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle1_VMScoord);
	c.genes.push_back(ga);

	// attribs for 2nd left muscle
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle1_VMScoord);
	c.genes.push_back(ga);

	// attribs for 3rd left muscle:
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for all left muscles:
	ga.minDepth.set(1);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_MUSCLE_RIGHT)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(3 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle2_VMScoord);
	c.genes.push_back(ga);

	// attribs for 2nd right muscle
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle2_VMScoord);
	c.genes.push_back(ga);

	// attribs for 3rd right muscle:
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for all right muscles:
	ga.minDepth.set(1);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	// finished with adding genes.

	// generate and insert offsets:
	for (offsetInsertion &i : insertions) {
		assertDbg(partMarkers.find(i.markerName) != partMarkers.end());	// if this jumps, a marker referred by an offset gene can't be found
#ifdef DEBUG
//		LOGLN("Marker offset [" << i.markerName << "] = " << partMarkers[i.markerName]);
#endif
		Atom<int> *pVal = &c.genes[i.geneIndex].data.gene_offset.offset;
		if (i.jointOffs)
			pVal = &c.genes[i.geneIndex].data.gene_joint_offset.offset;
		pVal->set(partMarkers[i.markerName] - i.partOffset);
	}

	// now we need to add some redundancy in between genes:
	if (true) {
		int padding = 2;
		for (uint i=0; i<c.genes.size(); i+=padding+1) {
			for (int k=0; k<padding; k++)
				c.genes.insert(c.genes.begin()+i+1, GeneNoOp());
			if (c.genes[i].type == GENE_TYPE_SKIP) {
				c.genes[i].data.gene_skip.count.set(c.genes[i].data.gene_skip.count * (padding+1));
			}
			if (c.genes[i].type == GENE_TYPE_OFFSET) {
				c.genes[i].data.gene_offset.offset.set(c.genes[i].data.gene_offset.offset * (padding+1));
			}
			if (c.genes[i].type == GENE_TYPE_JOINT_OFFSET) {
				c.genes[i].data.gene_joint_offset.offset.set(c.genes[i].data.gene_joint_offset.offset * (padding+1));
			}
		}
	}

	return c;
}
