/*
 * BodyPart.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BODYPART_H_
#define OBJECTS_BODY_PARTS_BODYPART_H_

#include "../genetics/GeneDefinitions.h"
#include "../genetics/CummulativeValue.h"
#include "../genetics/Gene.h"
#include "../PhysicsBody.h"
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

// inherit this struct and put in it all the CummulativeValues that are changed by the genes.
// after the genome is completely decoded, this data will be cached into real floats and this struct will be destroyed.
struct BodyPartInitializationData {
	virtual ~BodyPartInitializationData() = default;
	BodyPartInitializationData();
	/* this is called before commit() in order to sanitize all the initialData members that are set up by genes.
	 * it's important to do this because some values may be broken (ex zero or negative for size, or other values that
	 * don't make sense).
	 * The common members are sanitized by the base class's implementation, so call this as well from the overridden method
	 */

	CummulativeValue attachmentDirectionParent;		// the attachment direction in parent's space
	CummulativeValue angleOffset;					// rotation offset from the original attachment angle
	CummulativeValue lateralOffset;					// lateral (local OY axis) offset from the attachment point
	CummulativeValue size;							// surface area
	CummulativeValue density;
};

class UpdateList;
class RenderContext;

class BodyPart {
public:
	BodyPart(BodyPart* parent, PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData);
	virtual ~BodyPart();

	virtual void draw(RenderContext const& ctx);

	inline PART_TYPE getType() const { return type_; }

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
	//glm::vec2 getUpstreamAttachmentPoint();

	virtual glm::vec3 getWorldTransformation();

	virtual void addProcessedFood(float mass) { if (parent_) parent_->addProcessedFood(mass); }

	/*
	 * Returns a pointer to a specific attribute value, or nullptr if the type of body part doesn't support the specific attribute.
	 */
	inline CummulativeValue* getAttribute(gene_part_attribute_type attrib) {
		return mapAttributes_[attrib];
	}

	/*
	 * this will commit recursively in the entire body tree
	 */
	void commit_tree(float initialScale);

	/* returns the mass of the part and its entire subtree */
	virtual float getMass_tree();

	/* scale the part and all its children by a given amount */
	virtual void applyScale_tree(float scale);

	// tells the entire hierarchy that the body died
	void die_tree();

	// draws the whole tree of body-parts
	void draw_tree(RenderContext const& ctx);

	inline int getChildrenCount() const { return nChildren_; }
	inline BodyPart* getChild(int i) const { assert(i<nChildren_); return children_[i]; }
	inline std::shared_ptr<BodyPartInitializationData> getInitializationData() const { return initialData_; }
	void setUpdateList(UpdateList& lst) { updateList_ = &lst; }
	PhysicsBody const& getBody() { return physBody_; }

	/** returns the default (rest) angle of this part relative to its parent
	 */
	inline float getDefaultAngle() { return attachmentDirectionParent_ + angleOffset_; }
	inline float getAngleOffset() { return angleOffset_; }

	void matchLocation(const Atom<LocationLevelType>* location, int nLevel, std::vector<BodyPart*> *out);
	void applyRecursive(std::function<void(BodyPart* pCurrent)> pred);
	void addMotorLine(int line);
	void addSensorLine(int line);

	static constexpr int MAX_CHILDREN = 15;

protected:
	// these are used when initializing the body and whenever a new commit is called.
	// they contain world-space values that are updated only prior to committing
	PhysicsProperties cachedProps_;
	PhysicsBody physBody_;
	PART_TYPE type_;
	BodyPart* parent_;

	BodyPart* children_[MAX_CHILDREN];
	int nChildren_;

	bool committed_;
	// bool keepInitializationData_;	// set to true to not delete the initialData_ after commit()
	bool dontCreateBody_;			// set to true to prevent creating an actual physics body
	/* this indicates if the values that come from genes (such as angleOffset_, size_ etc) have been cached
	 * into the object's variables.
	 * If not, one must sanitize and use directly the values from the initialData for whatever purposes.
	 */
	bool geneValuesCached_;

	// final positioning and physical values:
	float attachmentDirectionParent_;
	float angleOffset_;
	float lateralOffset_;
	float size_;
	float density_;

	/**
	 * called after genome decoding finished, just before initializationData will be destroyed.
	 * Here you get the chance to cache and sanitize the initialization values into your member variables.
	 *
	 * SANITIZE all values, don't trust genes !!!
	 */
	virtual void cacheInitializationData();

	/*
	 * This is called after the body is completely developed and no more changes will occur on body parts
	 * except in rare circumstances.
	 * At this point the physics fixtures must be created and all temporary data purged.
	 * The physicsProperties of the body are transform to world coordinates before this method is called;
	 */
	virtual void commit() = 0;
	virtual void consumeEnergy(float amount);
	virtual void die() {}

	void add(BodyPart* part);
	void remove(BodyPart* part);
	void registerAttribute(gene_part_attribute_type type, CummulativeValue& value);
	glm::vec2 getUpstreamAttachmentPoint();
	UpdateList* getUpdateList();
	// call this if the fixture changed for any reason:
	void reattachChildren();

private:
	void computeBodyPhysProps();
	void reverseUpdateCachedProps();
	glm::vec2 getParentSpacePosition();
	bool applyScale_treeImpl(float scale, bool parentChanged);
	void purge_initializationData();

	std::vector<int> motorLines_; // a list of motor nerve lines that pass through this node
	std::vector<int> sensorLines_; // a list of sensor nerve lines -..-
	std::map<gene_part_attribute_type, CummulativeValue*> mapAttributes_;
	std::shared_ptr<BodyPartInitializationData> initialData_;
	UpdateList* updateList_;
	float lastCommitSize_inv_;
};



#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
