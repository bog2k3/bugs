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
	: parent_{parent}
	, children_{nullptr}
	, bodyPart_{part}
	, nChildren_{0}
{
}

DevelopmentNode::~DevelopmentNode() {
	// TODO Auto-generated destructor stub
}

void DevelopmentNode::matchLocation(const Atom<LocationLevelType>* location, int nLevel, std::vector<DevelopmentNode*> *out) {
	assert(nLevel >= 0);
	if (*location & (1<<15)) {
		out->push_back(this);
	}
	for (int i=0; i<nChildren_; i++) {
		if (*location & (1 << i))
			children_[i]->matchLocation(location+1, nLevel-1, out);
	}
}

void DevelopmentNode::applyRecursive(std::function<void(DevelopmentNode* pCurrent)> pred) {
	pred(this);
	for (int i=0; i<nChildren_; i++)
		children_[i]->applyRecursive(pred);
}

void DevelopmentNode::addMotorLine(int line) {
	motorLines_.push_back(line);
	if (parent_)
		parent_->addMotorLine(line);
}
