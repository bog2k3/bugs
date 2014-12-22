/*
 * DevelopmentNode.cpp
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#include "DevelopmentNode.h"
#include "../objects/body-parts/BodyPart.h"
#include <assert.h>

DevelopmentNode::DevelopmentNode(DevelopmentNode* parent, BodyPart* part)
	: parent{parent}
	, children{nullptr}
	, bodyPart{part}
	, isJoint{part->getType() == BODY_PART_JOINT}
	, nChildren{0}
{
}

DevelopmentNode::~DevelopmentNode() {
	// TODO Auto-generated destructor stub
}

void DevelopmentNode::matchLocation(uint64_t loc, std::vector<DevelopmentNode*> *out) {
	if (loc == 0 || nChildren == 0) {
		out->push_back(this);
	} else {
		if (isJoint) {
			if (loc == 1)
				out->push_back(children[0]);
			else
				children[0]->matchLocation(loc >> 1, out);
		} else {
			for (int i=0; i<nChildren; i++) {
				if (loc & (1 << i))
					children[i]->matchLocation(loc >> 4, out);
			}
		}
	}
}

void DevelopmentNode::applyRecursive(std::function<void(DevelopmentNode* pCurrent)> pred) {
	pred(this);
	for (int i=0; i<nChildren; i++)
		children[i]->applyRecursive(pred);
}
