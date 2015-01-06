/*
 * DevelopmentNode.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef GENETICS_DEVELOPMENTNODE_H_
#define GENETICS_DEVELOPMENTNODE_H_

#include "Gene.h"
#include <vector>
#include <functional>
#include <stdint.h>

class BodyPart;

class DevelopmentNode {
public:
	DevelopmentNode(DevelopmentNode* parent, BodyPart* part);
	virtual ~DevelopmentNode();

	static constexpr int MAX_CHILDREN = 15;

	DevelopmentNode* parent;
	DevelopmentNode* children[MAX_CHILDREN];

	void matchLocation(const Atom<LocationLevelType>* location, int nLevel, std::vector<DevelopmentNode*> *out);
	void applyRecursive(std::function<void(DevelopmentNode* pCurrent)> pred);

protected:
	friend class Ribosome;

	BodyPart* bodyPart;
	std::vector<int> motorLines; // a list of motor nerve lines that pass through this node
	std::vector<int> sensorLines; // a list of sensor nerve lines -..-
	bool isJoint;
	int nChildren;
};

#endif /* GENETICS_DEVELOPMENTNODE_H_ */
