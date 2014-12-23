/*
 * BodyPart.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BODYPART_H_
#define OBJECTS_BODY_PARTS_BODYPART_H_

#include "../WorldObject.h"
#include "../../genetics/GeneDefinitions.h"
#include "../../genetics/CummulativeValue.h"
#include <vector>
#include <map>

enum PART_TYPE {
	BODY_PART_INVALID,

	BODY_PART_TORSO,
	BODY_PART_BONE,
	BODY_PART_MUSCLE,
	BODY_PART_JOINT,
	BODY_PART_GRIPPER,
	BODY_PART_ZYGOTE_SHELL,
	BODY_PART_SENSOR,
};

class BodyPart : public WorldObject {
public:
	// the position and rotation in props are relative to the parent
	BodyPart(BodyPart* parent, PART_TYPE type, PhysicsProperties props);
	virtual ~BodyPart() override;

	virtual void draw(RenderContext& ctx) override;

	PART_TYPE getType() { return type_; }

	void changeParent(BodyPart* newParent);

	/**
	 * return the attachment point for a child of the current part, in the specified direction
	 * (in current's part coordinate frame).
	 * This is usually the point where the ray from the center intersects the edge of the body part.
	 */
	virtual glm::vec2 getChildAttachmentPoint(float relativeAngle) { return glm::vec2(0); }

	/**
	 * returns the attachment point for the current part in its parent's coordinate space.
	 */
	glm::vec2 getUpstreamAttachmentPoint();

	virtual glm::vec3 getWorldTransformation() const;

	/*
	 * This is called after the body is completely developed and no more changes will occur on body parts
	 * except in rare circumstances.
	 * At this point the physics fixtures must be created and all temporary data purged.
	 * The physicsProperties of the body are transform to world coordinates before this method is called;
	 * The physicsProperties are deleted after the commit is finished.
	 */
	virtual void commit() = 0;

	/*
	 * Returns a pointer to a specific attribute value, or nullptr if the type of body part doesn't support the specific attribute.
	 */
	CummulativeValue* getAttribute(gene_attribute_type attrib);

	/*
	 * this will commit recursively in the entire body tree
	 */
	void commit_tree();

protected:
	PART_TYPE type_;
	BodyPart* parent_;
	static const int MAX_CHILDREN = 4;
	BodyPart* children_[MAX_CHILDREN];
	int nChildren_;
	bool committed_;
	bool keepInitializationData_;
	bool dontCommit_;

	void add(BodyPart* part);
	void remove(BodyPart* part);
	void registerAttribute(gene_attribute_type type, CummulativeValue& value);
	glm::vec2 getFinalPrecommitPosition();

private:
	bool coordinates_local_;
	void transform_position_and_angle();
	void commit_tree(std::vector<BodyPart*> &out_joints);
	std::map<gene_attribute_type, CummulativeValue*> mapAttributes_;
};



#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
