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
	constexpr float muscle1_VMScoord = 5.f;
	constexpr float gripper_VMScoord = 10.f;
	constexpr float muscle2_VMScoord = 15.f;
	constexpr float musclePeriod = 3.f; // seconds
	constexpr float gripper_signal_threshold = -0.55f;
	constexpr float gripper_signal_phase_offset = PI/2.2f;

	GeneOffset go;
	GeneJointOffset gjo;

	PART_MARKER(TORSO)

	GeneStartMarker gsm;
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

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

	// neuron #0 transfer:
	GeneTransferFunction gt;
	gt.targetNeuron.set(0);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #0 input:
	GeneNeuronInputCoord gic;
	gic.destNeuronVirtIndex.set(0);
	gic.inCoord.set(50);
	c.genes.push_back(gic);

	// neuron #1 transfer:
	gt.targetNeuron.set(1);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #1 constant:
	GeneNeuralConstant gnc;
	gnc.targetNeuron.set(1);
	gnc.value.set(PI);
	c.genes.push_back(gnc);

	// neuron #2 transfer:
	gt.targetNeuron.set(2);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #2 output VMS coord
	GeneNeuronOutputCoord goc;
	goc.srcNeuronVirtIndex.set(2);
	goc.outCoord.set(muscle2_VMScoord);
	c.genes.push_back(goc);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);
	// neuron #3 output VMS coord
	goc.srcNeuronVirtIndex.set(3);
	goc.outCoord.set(muscle1_VMScoord);
	c.genes.push_back(goc);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #4 constant:
	gnc.targetNeuron.set(4);
	gnc.value.set(gripper_signal_threshold);
	c.genes.push_back(gnc);

	// neuron #5 transfer:
	gt.targetNeuron.set(5);
	gt.functionID.set((int)transferFuncNames::FN_ONE);
	c.genes.push_back(gt);
	// neuron #5 output VMS coord
	goc.srcNeuronVirtIndex.set(5);
	goc.outCoord.set(gripper_VMScoord);
	c.genes.push_back(goc);

	// neuron #6 transfer:
	gt.targetNeuron.set(6);
	gt.functionID.set((int)transferFuncNames::FN_CONSTANT);
	c.genes.push_back(gt);
	// neuron #6 constant
	gnc.targetNeuron.set(6);
	gnc.value.set(gripper_signal_phase_offset);
	c.genes.push_back(gnc);

	// neuron #7 transfer:
	gt.targetNeuron.set(7);
	gt.functionID.set((int)transferFuncNames::FN_SIN);
	c.genes.push_back(gt);

	GeneSynapse gs;

	// synapse 0 to 2
	gs.from.set(0);
	gs.to.set(2);
	gs.weight.set(2*PI / musclePeriod);
	c.genes.push_back(gs);

	// synapse 0 to 3
	gs.from.set(0);
	gs.to.set(3);
	gs.weight.set(2*PI / musclePeriod);
	c.genes.push_back(gs);

	// synapse 1 to 3
	gs.from.set(1);
	gs.to.set(3);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 4 to 5
	gs.from.set(4);
	gs.to.set(5);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 0 to 7
	gs.from.set(0);
	gs.to.set(7);
	gs.weight.set(2*PI / musclePeriod);
	c.genes.push_back(gs);

	// synapse 1 to 7
	gs.from.set(1);
	gs.to.set(7);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 6 to 7
	gs.from.set(6);
	gs.to.set(7);
	gs.weight.set(1.f);
	c.genes.push_back(gs);

	// synapse 7 to 5
	gs.from.set(7);
	gs.to.set(5);
	gs.weight.set(1.f);
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
	INSERT_OFFSET(TORSO_MUSCLE7)
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
	INSERT_OFFSET(TORSO_MUSCLE9)
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

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

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

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

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

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	gjo.maxDepth.set(2);
	INSERT_JOFFSET(TORSO_JOINT8)
	c.genes.push_back(gjo);

	ga.maxDepth.set(2);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(PI/8);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_DENSITY;
	ga.value.set(BodyConst::initialBoneDensity);
	c.genes.push_back(ga);

	// grow Bone(0)
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

	go.maxDepth.set(2);
	go.targetSegment.set(0);
	INSERT_OFFSET(TORSO_BONE8_BONE0)
	c.genes.push_back(go);

	// grow Muscle(1):
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(1);
	INSERT_OFFSET(TORSO_BONE8_MUSCLE1)
	c.genes.push_back(go);

	// grow Muscle(15):
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(15);
	INSERT_OFFSET(TORSO_BONE8_MUSCLE15)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_JOINT8)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 3);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(0.1f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_MUSCLE1)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle2_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_MUSCLE15)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(3 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle1_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_BONE0)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	gjo.maxDepth.set(4);
	INSERT_JOFFSET(TORSO_BONE8_JOINT0)
	c.genes.push_back(gjo);

	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(4);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_DENSITY;
	ga.value.set(BodyConst::initialBoneDensity);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	// grow Gripper(0):
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

	go.maxDepth.set(4);
	go.targetSegment.set(0);
	INSERT_OFFSET(TORSO_BONE8_BONE0_GRIPPER0)
	c.genes.push_back(go);

	// grow gripper muscle (1):
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(1);
	INSERT_OFFSET(TORSO_BONE8_BONE0_MUSCLE1)
	c.genes.push_back(go);

	// grow gripper muscle (15):
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(15);
	INSERT_OFFSET(TORSO_BONE8_BONE0_MUSCLE15)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_JOINT0)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi*0.5f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_BONE0_GRIPPER0)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.attribute = GENE_ATTRIB_SIZE;
	ga.minDepth.set(0);
	ga.maxDepth.set(6);
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(gripper_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_BONE8_BONE0_MUSCLE1)
	PART_MARKER(TORSO_BONE8_BONE0_MUSCLE15)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

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

	PART_MARKER(TORSO_MUSCLE7)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2.e-3f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle1_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_MUSCLE9)

	c.genes.push_back(gsm);
	c.genes.push_back(gsm);

	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(1.e-3f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(BodyConst::initialMuscleAspectRatio);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(muscle2_VMScoord);
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
	if (false) {
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

