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
	gt.functionID.set((int)transferFuncNames::FN_GATE);
	c.genes.push_back(gt);
	// neuron #2 gate threshold
	gnb.targetNeuron.set(2);
	gnb.value.set(0);
	c.genes.push_back(gnb);
	// neuron #2 gate switch steepness
	gnp.targetNeuron.set(2);
	gnp.value.set(10);
	c.genes.push_back(gnp);

	// neuron #3 transfer:
	gt.targetNeuron.set(3);
	gt.functionID.set((int)transferFuncNames::FN_GATE);
	c.genes.push_back(gt);
	// neuron #3 gate threshold
	gnb.targetNeuron.set(3);
	gnb.value.set(0);
	c.genes.push_back(gnb);
	// neuron #3 gate switch steepness
	gnp.targetNeuron.set(3);
	gnp.value.set(10);
	c.genes.push_back(gnp);

	// neuron #19 transfer:
	gt.targetNeuron.set(19);
	gt.functionID.set((int)transferFuncNames::FN_SIGMOID);
	c.genes.push_back(gt);
	// neuron #19 sigmoid steepness
	gnp.targetNeuron.set(19);
	gnp.value.set(10);
	c.genes.push_back(gnp);

	// neuron #4 transfer:
	gt.targetNeuron.set(4);
	gt.functionID.set((int)transferFuncNames::FN_POW);
	c.genes.push_back(gt);
	// neuron #4 param:
	gnp.targetNeuron.set(4);
	gnp.value.set(-1);
	c.genes.push_back(gnp);
	// neuron #4 bias (to avoid division by 0)
	gnb.targetNeuron.set(4);
	gnb.value.set(1.e-10f);
	c.genes.push_back(gnb);

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
	gs.weight.set(+1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 0 to 5
	gs.from.set(0);
	gs.to.set(5);
	gs.weight.set(-1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 0 to 19
	gs.from.set(0);
	gs.to.set(19);
	gs.weight.set(-1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 1 to 3
	gs.from.set(1);
	gs.to.set(3);
	gs.weight.set(+1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 1 to 5
	gs.from.set(1);
	gs.to.set(5);
	gs.weight.set(+1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 1 to 19
	gs.from.set(1);
	gs.to.set(19);
	gs.weight.set(+1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 19 to 2
	gs.from.set(19);
	gs.to.set(2);
	gs.weight.set(-1);
	gs.priority.set(100);	// #0 for gate
	c.genes.push_back(gs);

	// synapse 19 to 3
	gs.from.set(19);
	gs.to.set(3);
	gs.weight.set(+1);
	gs.priority.set(100);	// #0 for gate
	c.genes.push_back(gs);

	// synapse 2 to 4
	gs.from.set(2);
	gs.to.set(4);
	gs.weight.set(1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 3 to 4
	gs.from.set(3);
	gs.to.set(4);
	gs.weight.set(1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 4 to 5
	gs.from.set(4);
	gs.to.set(5);
	gs.weight.set(1);
	gs.priority.set(100);	// this is #0 for MUL
	c.genes.push_back(gs);

	// synapse 5 to 13
	gs.from.set(5);
	gs.to.set(13);
	gs.weight.set(+0.5f);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 5 to 17
	gs.from.set(5);
	gs.to.set(17);
	gs.weight.set(-0.5f);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 6 to 7
	gs.from.set(6);
	gs.to.set(7);
	gs.weight.set(2*PI/musclePeriod);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 6 to 9
	gs.from.set(6);
	gs.to.set(9);
	gs.weight.set(2*PI/musclePeriod);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 6 to 11
	gs.from.set(6);
	gs.to.set(11);
	gs.weight.set(2*PI/musclePeriod);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 6 to 12
	gs.from.set(6);
	gs.to.set(12);
	gs.weight.set(2*PI/musclePeriod);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 6 to 15
	gs.from.set(6);
	gs.to.set(15);
	gs.weight.set(2*PI/musclePeriod);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 6 to 16
	gs.from.set(6);
	gs.to.set(16);
	gs.weight.set(2*PI/musclePeriod);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 7 to 8
	gs.from.set(7);
	gs.to.set(8);
	gs.weight.set(1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 9 to 10
	gs.from.set(9);
	gs.to.set(10);
	gs.weight.set(1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 11 to 14
	gs.from.set(11);
	gs.to.set(14);
	gs.weight.set(1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 13 to 14
	gs.from.set(13);
	gs.to.set(14);
	gs.weight.set(1);
	gs.priority.set(100);	// this is #0 for MUL
	c.genes.push_back(gs);

	// synapse 15 to 18
	gs.from.set(15);
	gs.to.set(18);
	gs.weight.set(1);
	gs.priority.set(0);
	c.genes.push_back(gs);

	// synapse 17 to 18
	gs.from.set(17);
	gs.to.set(18);
	gs.weight.set(1);
	gs.priority.set(100);	// this is #0 for MUL
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

	// grow left nose:
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_H);	// W+
	c.genes.push_back(gp);

	go.targetSegment.set(1);
	INSERT_OFFSET(LEFT_NOSE)
	c.genes.push_back(go);

	// grow right nose:
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_H);	// W+
	c.genes.push_back(gp);

	go.targetSegment.set(15);
	INSERT_OFFSET(RIGHT_NOSE)
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

	PART_MARKER(LEFT_NOSE)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialNoseSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_SENSOR_OUTPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(leftNose_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(RIGHT_NOSE)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialNoseSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_SENSOR_OUTPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(rightNose_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(EGGLAYER)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(5);
	ga.maxDepth.set(5);
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

	gjo.minDepth.set(2);
	gjo.maxDepth.set(4);
	INSERT_JOFFSET(TORSO_JOINT8)
	c.genes.push_back(gjo);

	ga.minDepth.set(2);
	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.25f * 0.02f);
	c.genes.push_back(ga);

	ga.minDepth.set(4);
	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.0003f);
	c.genes.push_back(ga);

	ga.minDepth.set(2);
	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(0.4f);
	c.genes.push_back(ga);

	ga.minDepth.set(4);
	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_ASPECT_RATIO;
	ga.value.set(6.5f);
	c.genes.push_back(ga);

	// grow second bone:
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
	INSERT_OFFSET(TORSO_BONE8);
	c.genes.push_back(go);

	// grow Egg Layer (on second bone):
	gp.minDepth.set(4);
	gp.maxDepth.set(4);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_B);	// X+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(4);
	go.targetSegment.set(0);
	INSERT_OFFSET(EGGLAYER)
	c.genes.push_back(go);

	// grow right leg (2):
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
	gp.targetSegment.set(2);
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
	go.targetSegment.set(2);
	INSERT_OFFSET(RIGHT_LEG)
	c.genes.push_back(go);

	// grow right leg pull Muscle(3):
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
	gp.targetSegment.set(3);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(3);
	go.minDepth.set(2);
	go.maxDepth.set(2);
	INSERT_OFFSET(RIGHT_MUSCLE_PULL)
	c.genes.push_back(go);

	// grow right leg push Muscle (1):
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
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
	go.minDepth.set(2);
	go.maxDepth.set(2);
	INSERT_OFFSET(RIGHT_MUSCLE_PUSH)
	c.genes.push_back(go);

	// grow left leg (14):
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
	gp.targetSegment.set(14);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(14);
	go.minDepth.set(2);
	go.maxDepth.set(2);
	INSERT_OFFSET(LEFT_LEG)
	c.genes.push_back(go);

	// grow left leg pull Muscle(13):
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
	gp.targetSegment.set(13);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.targetSegment.set(13);
	go.minDepth.set(2);
	go.maxDepth.set(2);
	INSERT_OFFSET(LEFT_MUSCLE_PULL)
	c.genes.push_back(go);

	// grow left leg push Muscle (15):
	gp.minDepth.set(2);
	gp.maxDepth.set(2);
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
	go.minDepth.set(2);
	go.maxDepth.set(2);
	INSERT_OFFSET(LEFT_MUSCLE_PUSH)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(LEFT_LEG)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for first bone:
	gjo.minDepth.set(4);
	gjo.maxDepth.set(4);
	INSERT_JOFFSET(LEFT_LEG_JOINTS)
	c.genes.push_back(gjo);

	ga.minDepth.set(4);
	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	// attribs for second bone:
	gjo.minDepth.set(6);
	gjo.maxDepth.set(6);
	INSERT_JOFFSET(LEFT_LEG_JOINTS)
	c.genes.push_back(gjo);

	ga.maxDepth.set(6);
	ga.minDepth.set(6);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	// common attribs for 1st bone, 2nd bone
	ga.minDepth.set(4);
	ga.maxDepth.set(6);

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
	gp.minDepth.set(6);
	gp.maxDepth.set(6);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(6);
	go.maxDepth.set(6);
	go.targetSegment.set(0);
	INSERT_OFFSET(LEFT_GRIPPER)
	c.genes.push_back(go);

	// (on 1st bone) grow 2nd Bone(0);
	gp.minDepth.set(4);
	gp.maxDepth.set(4);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(4);
	go.targetSegment.set(0);
	INSERT_OFFSET(LEFT_LEG)	// loop
	c.genes.push_back(go);

	// (on both bones) grow push Muscles (1):
	gp.minDepth.set(4);
	gp.maxDepth.set(6);
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(6);
	go.targetSegment.set(1);
	INSERT_OFFSET(LEFT_MUSCLE_PUSH)
	c.genes.push_back(go);

	// (on both bones) grow pull Muscles (15):
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(6);
	go.targetSegment.set(15);
	INSERT_OFFSET(LEFT_MUSCLE_PULL)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(RIGHT_LEG)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for first bone:
	gjo.minDepth.set(4);
	gjo.maxDepth.set(4);
	INSERT_JOFFSET(RIGHT_LEG_JOINTS)
	c.genes.push_back(gjo);

	ga.minDepth.set(4);
	ga.maxDepth.set(4);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	// attribs for second bone:
	gjo.minDepth.set(6);
	gjo.maxDepth.set(6);
	INSERT_JOFFSET(RIGHT_LEG_JOINTS)
	c.genes.push_back(gjo);

	ga.maxDepth.set(6);
	ga.minDepth.set(6);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(0.08f * 0.01f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_LOCAL_ROTATION;
	ga.value.set(0);
	c.genes.push_back(ga);

	// common attribs for 1st bone, 2nd bone
	ga.minDepth.set(4);
	ga.maxDepth.set(6);

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
	gp.minDepth.set(6);
	gp.maxDepth.set(6);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(6);
	go.maxDepth.set(6);
	go.targetSegment.set(0);
	INSERT_OFFSET(RIGHT_GRIPPER)
	c.genes.push_back(go);

	// (on 1st bone) grow 2nd Bone(0);
	gp.minDepth.set(4);
	gp.maxDepth.set(4);
	gp.targetSegment.set(0);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_C);	// Y-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_E);	// Z-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(4);
	go.targetSegment.set(0);
	INSERT_OFFSET(RIGHT_LEG)	// loop
	c.genes.push_back(go);

	// (on both bones) grow push Muscles (15):
	gp.minDepth.set(4);
	gp.maxDepth.set(6);
	gp.targetSegment.set(15);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(6);
	go.targetSegment.set(15);
	INSERT_OFFSET(RIGHT_MUSCLE_PUSH)
	c.genes.push_back(go);

	// (on both bones) grow pull Muscles (1):
	gp.targetSegment.set(1);
	gp.protein.set(GENE_PROT_A);	// X-
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_D);	// Y+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_F);	// Z+
	c.genes.push_back(gp);
	gp.protein.set(GENE_PROT_G);	// W-
	c.genes.push_back(gp);

	go.minDepth.set(4);
	go.maxDepth.set(6);
	go.targetSegment.set(1);
	INSERT_OFFSET(RIGHT_MUSCLE_PULL)
	c.genes.push_back(go);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(TORSO_JOINT8)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(1);
	ga.maxDepth.set(1);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 3);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(-BodyConst::initialJointMaxPhi);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(-BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(LEFT_LEG_JOINTS)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st joint:
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
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
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 2);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi*0.5f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	// attribs for 3rd joint
	ga.minDepth.set(7);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 2);
	c.genes.push_back(ga);

	// common attribs for all joints:
	ga.minDepth.set(3);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(RIGHT_LEG_JOINTS)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st joint:
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
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
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 2);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_HIGH_LIMIT;
	ga.value.set(BodyConst::initialJointMaxPhi*0.5f);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_JOINT_LOW_LIMIT;
	ga.value.set(BodyConst::initialJointMinPhi);
	c.genes.push_back(ga);

	// attribs for 3rd joint
	ga.minDepth.set(7);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialJointSize * 2);
	c.genes.push_back(ga);

	// common attribs for all joints:
	ga.minDepth.set(3);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_JOINT_RESET_TORQUE;
	ga.value.set(BodyConst::initialJointResetTorque);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(LEFT_GRIPPER)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(8);
	ga.maxDepth.set(8);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	gjo.minDepth.set(8);
	gjo.maxDepth.set(8);
	INSERT_JOFFSET(LEFT_LEG_JOINTS)
	c.genes.push_back(gjo);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(leftGripper_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(RIGHT_GRIPPER)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	ga.minDepth.set(8);
	ga.maxDepth.set(8);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	ga.attribute = GENE_ATTRIB_ATTACHMENT_OFFSET;
	ga.value.set(0);
	c.genes.push_back(ga);

	gjo.minDepth.set(8);
	gjo.maxDepth.set(8);
	INSERT_JOFFSET(RIGHT_LEG_JOINTS)
	c.genes.push_back(gjo);

	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(rightGripper_VMScoord);
	c.genes.push_back(ga);

	c.genes.push_back(GeneStop());
	c.genes.push_back(GeneStop());

	PART_MARKER(LEFT_MUSCLE_PULL)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st left pull muscle
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(6 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 2nd left pull muscle
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 1st & 2nd left pull muscle:
	ga.minDepth.set(3);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(leftLeg_pull_VMScoord);
	c.genes.push_back(ga);

	// attribs for 3rd left pull muscle:
	ga.minDepth.set(7);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for all left pull muscles:
	ga.minDepth.set(3);
	ga.maxDepth.set(7);
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

	PART_MARKER(LEFT_MUSCLE_PUSH)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st left push muscle
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(3 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 2nd left push muscle
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 1st & 2nd left push muscles
	ga.minDepth.set(3);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(leftLeg_push_VMScoord);
	c.genes.push_back(ga);

	// attribs for 3rd left push muscle:
	ga.minDepth.set(7);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for all left push muscles:
	ga.minDepth.set(3);
	ga.maxDepth.set(7);
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

	PART_MARKER(RIGHT_MUSCLE_PULL)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st right pull muscle
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(6 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 2nd right pull muscle
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 1st & 2nd right pull muscle:
	ga.minDepth.set(3);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(rightLeg_pull_VMScoord);
	c.genes.push_back(ga);

	// attribs for 3rd right pull muscle:
	ga.minDepth.set(7);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for all right pull muscles:
	ga.minDepth.set(3);
	ga.maxDepth.set(7);
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

	PART_MARKER(RIGHT_MUSCLE_PUSH)

#ifdef ENABLE_START_MARKER_GENES
	c.genes.push_back(gsm);
	c.genes.push_back(gsm);
#endif

	// attribs for 1st right push muscle
	ga.minDepth.set(3);
	ga.maxDepth.set(3);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(3 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 2nd right push muscle
	ga.minDepth.set(5);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for 1st & 2nd right push muscles
	ga.minDepth.set(3);
	ga.maxDepth.set(5);
	ga.attribute = GENE_ATTRIB_MOTOR_INPUT_COORD;
	ga.attribIndex.set(0);
	ga.value.set(rightLeg_push_VMScoord);
	c.genes.push_back(ga);

	// attribs for 3rd right push muscle:
	ga.minDepth.set(7);
	ga.maxDepth.set(7);
	ga.attribute = GENE_ATTRIB_SIZE;
	ga.value.set(2 * BodyConst::initialBodyPartSize);
	c.genes.push_back(ga);

	// attribs for all right push muscles:
	ga.minDepth.set(3);
	ga.maxDepth.set(7);
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
			if (c.genes[i].type == gene_type::JOINT_OFFSET) {
				c.genes[i].data.gene_joint_offset.offset.set(c.genes[i].data.gene_joint_offset.offset * (padding+1));
			}
		}
	}

	return c;
}

