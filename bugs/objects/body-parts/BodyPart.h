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
#include "../../genetics/Gene.h"
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
	BODY_PART_MOUTH,
};

struct BodyPartInitializationData {
	virtual ~BodyPartInitializationData() = default;
	BodyPartInitializationData();
	PhysicsProperties cachedProps;

	CummulativeValue attachmentDirectionParent;		// the attachment direction in parent's space
	CummulativeValue angleOffset;					// rotation offset from the original attachment angle
	CummulativeValue lateralOffset;					// lateral (local OY axis) offset from the attachment point
	CummulativeValue size;							// surface area
	CummulativeValue density;
};

class UpdateList;

class BodyPart : public WorldObject {
public:
	BodyPart(BodyPart* parent, PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData);
	virtual ~BodyPart() override;

	virtual void draw(RenderContext& ctx) override;

	inline PART_TYPE getType() const { return type_; }

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
	 * Returns a pointer to a specific attribute value, or nullptr if the type of body part doesn't support the specific attribute.
	 */
	inline CummulativeValue* getAttribute(gene_part_attribute_type attrib) {
		return mapAttributes_[attrib];
	}

	/*
	 * this will commit recursively in the entire body tree
	 */
	void commit_tree();

	/* returns the mass of the part and its entire subtree */
	float getMass_tree();

	/* scale the part and all its children by a given amount */
	void applyScale_tree(float scale);

	// tells the entire hierarchy that the body died
	void die_tree();

	/**
	 * recursively free the initialization data from all body parts after committing the entire tree
	 */
	void purge_initializationData_tree();

	inline int getChildrenCount() const { return nChildren_; }
	inline BodyPart* getChild(int i) const { assert(i<nChildren_); return children_[i]; }
	inline std::shared_ptr<BodyPartInitializationData> getInitializationData() const { return initialData_; }
	void setUpdateList(UpdateList& lst) { updateList_ = &lst; }

	/** returns the default (rest) angle of this part relative to its parent
	 */
	float getDefaultAngle();

	void matchLocation(const Atom<LocationLevelType>* location, int nLevel, std::vector<BodyPart*> *out);
	void applyRecursive(std::function<void(BodyPart* pCurrent)> pred);
	void addMotorLine(int line);
	void addSensorLine(int line);

	static constexpr int MAX_CHILDREN = 15;

protected:
	PART_TYPE type_;
	BodyPart* parent_;

	BodyPart* children_[MAX_CHILDREN];
	int nChildren_;

	bool committed_;
	bool keepInitializationData_;	// set to true to not delete the initialData_ after commit()
	bool dontCreateBody_;			// set to true to prevent creating an actual physics body

	/*
	 * This is called after the body is completely developed and no more changes will occur on body parts
	 * except in rare circumstances.
	 * At this point the physics fixtures must be created and all temporary data purged.
	 * The physicsProperties of the body are transform to world coordinates before this method is called;
	 * The physicsProperties are deleted after the commit is finished.
	 */
	virtual void commit() = 0;
	virtual void consumeEnergy(float amount);
	virtual void die() {}

	void add(BodyPart* part);
	void remove(BodyPart* part);
	void registerAttribute(gene_part_attribute_type type, CummulativeValue& value);
	glm::vec2 getUpstreamAttachmentPoint() const;
	UpdateList* getUpdateList();

private:
	void computeBodyPhysProps();
	void reverseUpdateCachedProps();
	glm::vec2 getParentSpacePosition() const;
	bool applyScale_treeImpl(float scale, bool parentChanged);

	std::vector<int> motorLines_; // a list of motor nerve lines that pass through this node
	std::vector<int> sensorLines_; // a list of sensor nerve lines -..-
	std::map<gene_part_attribute_type, CummulativeValue*> mapAttributes_;
	std::shared_ptr<BodyPartInitializationData> initialData_;
	UpdateList* updateList_;
	float lastCommitSize_inv_;
};



#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
