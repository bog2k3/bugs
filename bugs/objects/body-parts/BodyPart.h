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
#include <memory>

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

struct BodyPartInitializationData {
	virtual ~BodyPartInitializationData() = default;
	BodyPartInitializationData()
		: size(1.e-4f) {
	}
	PhysicsProperties cachedProps;

	CummulativeValue attachmentDirectionParent;		// the attachment direction in parent's space
	CummulativeValue angleOffset;					// rotation offset from the original attachment angle
	CummulativeValue lateralOffset;					// lateral (local OY axis) offset from the attachment point
	CummulativeValue size;							// surface area
};

class BodyPart : public WorldObject {
public:
	BodyPart(BodyPart* parent, PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData);
	virtual ~BodyPart() override;

	virtual void draw(RenderContext& ctx) override;

	PART_TYPE getType() { return type_; }

	void changeParent(BodyPart* newParent);

	/**
	 * return the attachment point for a child of the current part, in the specified direction
	 * (in current's part coordinate frame).
	 * This is usually the point where the ray from the center intersects the edge of the body part.
	 */
	virtual glm::vec2 getChildAttachmentPoint(float relativeAngle) const { return glm::vec2(0); }

	/**
	 * returns the attachment point for the current part in its parent's coordinate space.
	 */
	//glm::vec2 getUpstreamAttachmentPoint();

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

	static const int MAX_CHILDREN = 15;
	BodyPart* children_[MAX_CHILDREN];
	int nChildren_;

	bool committed_;
	bool keepInitializationData_;	// set to true to not delete the initialData_ after commit()
	bool dontCreateBody_;			// set to true to prevent creating an actual physics body

	void add(BodyPart* part);
	void remove(BodyPart* part);
	void registerAttribute(gene_attribute_type type, CummulativeValue& value);
	std::shared_ptr<BodyPartInitializationData> getInitializationData() const { return initialData_; }

private:
	void computeBodyPhysProps();
	glm::vec2 getUpstreamAttachmentPoint() const;
	std::map<gene_attribute_type, CummulativeValue*> mapAttributes_;
	std::shared_ptr<BodyPartInitializationData> initialData_;
};



#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
