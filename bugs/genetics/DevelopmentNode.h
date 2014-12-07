/*
 * DevelopmentNode.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef GENETICS_DEVELOPMENTNODE_H_
#define GENETICS_DEVELOPMENTNODE_H_

#include <list>
#include <stdint.h>

class BodyPart;

class DevelopmentNode {
public:
	DevelopmentNode();
	virtual ~DevelopmentNode();

	DevelopmentNode* parent;
	DevelopmentNode* children[4];

	void matchLocation(uint64_t loc, std::list<DevelopmentNode*> *out);

protected:
	friend class Ribosome;

	BodyPart* bodyPart;
	std::list<int> motorLines; // a list of motor nerve lines that pass through this node
	std::list<int> sensorLines; // a list of sensor nerve lines -..-
	bool isJoint;
	int nChildren;
};

#endif /* GENETICS_DEVELOPMENTNODE_H_ */
