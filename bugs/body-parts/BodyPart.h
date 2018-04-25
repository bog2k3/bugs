/*
 * BodyPart.h
 *
 *  Created on: Dec 7, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_BODYPART_H_
#define OBJECTS_BODY_PARTS_BODYPART_H_

#include "../genetics/GeneDefinitions.h"
#include "../genetics/CumulativeValue.h"
#include "BodyPartTypes.h"
#include "BodyPartContext.h"

#include <boglfw/physics/PhysicsBody.h>
#include <boglfw/utils/ThreadLocalValue.h>

#include <glm/vec2.hpp>

#include <vector>
//#include <map>
#include <memory>
//#include <ostream>

class UpdateList;
class RenderContext;
class Bug;
class BodyCell;
class Entity;

class BodyPart {
public:
	// general ctor
	BodyPart(BodyPartType type, BodyPartContext const& context, BodyCell const& cell, bool suppressPhysicalBody=false);
	// ctor for ZygoteShell
	BodyPart(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass, BodyPartContext const& context);
	virtual ~BodyPart();

	// call this to destroy and delete the object. Never delete directly
	void destroy();

	// this will return the division-depth of the body-part
	int getDepth() const { return 0; } // TODO use branch length

	virtual void draw(RenderContext const& ctx);

	inline BodyPartType getType() const { return type_; }

	/**
	 * return the attachment point for a neighbor of the current part, in the specified direction
	 * (in current's part coordinate frame).
	 * This is usually the point where the ray from the center intersects the edge of the body part.
	 */
	virtual glm::vec2 getAttachmentPoint(float relativeAngle) { return glm::vec2(0); }

	// returns the world transformation as {x, y, rotation}
	virtual glm::vec3 getWorldTransformation() const;
	// transform a vector+angle from world coordinates to local coordinates; specify isDirection=true to not apply translation
	glm::vec3 worldToLocal(glm::vec3 tr, bool isDirection=false) const;
	// transform a vector+angle from local coordinates to world coordinates; specify isDirection=true to not apply translation
	glm::vec3 localToWorld(glm::vec3 const& tr, bool isDirection=false) const;
	// transform a vector from world coordinates to local coordinates; specify isDirection=true to not apply translation
	glm::vec2 worldToLocal(glm::vec2 tr, bool isDirection=false) const;
	// transform a vector from local coordinates to world coordinates; specify isDirection=true to not apply translation
	glm::vec2 localToWorld(glm::vec2 const& tr, bool isDirection=false) const;
	// returns the bounding box of the body part
	aabb getAABB() const;

	Bug* getOwner() const { return &context_.owner; }

	/*
	 * Returns a pointer to a specific attribute value, or nullptr if the type of body part doesn't support the specific attribute.
	 */
	/*inline CumulativeValue* getAttribute(gene_part_attribute_type attrib, unsigned index=0) {
		if (mapAttributes_.find(attrib) == mapAttributes_.end())
			return nullptr;
		auto &attrVec = mapAttributes_[attrib];
		if (attrVec.size() > index)
			return attrVec[index];
		else
			return attrVec[0];
	}*/

	/*
	 * this will commit recursively in the entire body tree
	 */
	//void commit_tree(float initialScale);

	/* returns the mass of the part and its entire subtree */
	//virtual float getMass_tree();

	/* scale the part and all its children by a given amount */
	//virtual void applyScale_tree(float scale);

	// draws the whole tree of body-parts
	//void draw_tree(RenderContext const& ctx);

	//inline int getChildrenCount() const { return nChildren_; }
	//inline BodyPart* getChild(int i) const { assertDbg(i<nChildren_); return children_[i]; }
	//inline std::shared_ptr<BodyPartInitializationData> getInitializationData() const { return initialData_; }
	//void setUpdateList(UpdateList& lst) { updateList_ = &lst; }
	PhysicsBody const& getBody() { return physBody_; }

	/** returns the default (rest) angle of this part relative to its parent
	 */
	//inline float getDefaultAngle() const { return /*attachmentDirectionParent_ +*/ localRotation_; }
	//inline float getLocalRotation() const { return localRotation_; }
	//inline float getAttachmentAngle() const { return 0/*attachmentDirectionParent_*/; }

	// runs a given predicate on the current part and recursively on all of its neighbors and so on until the graph is completely covered,
	// or the evaluation is stopped by the predicate.
	// return false from the predicate to continue or true to stop evaluation.
	// the return value of the method indicates whether the evaluation was forcefully stopped by the predicate:
	//		- [true] means the evaluation was stopped by the predicate returning true.
	//		- [false] means the predicate was applied to the entire graph and no positive was detected.
	// UOID parameter is used internally, don't pass any value to it explicitly.
	bool applyPredicateGraph(std::function<bool(BodyPart* pCurrent)> pred, uint32_t UOID=0);

	// adds the motor line id into this node and all nodes above it recursively
	// this id is the index of the nerve line from the neural network down to one of this motor's inputs
	void addMotorLine(int lineId);

	/*
	 * adds another body part as a child of this one, trying to fit it at the given relative angle.
	 * The part's angle may be slightly changed if it overlaps other siblings.
	 * returns the actual angle at which the part was inserted.
	 */
	//virtual float add(BodyPart* part, float angle);
	/*
	 * remove all links, to parent and children. Calling this makes you responsible for the children, make sure
	 * they don't get leaked.
	 */
//	void removeAllLinks();

	void addNeighbor(BodyPart* n);

	void removeNeighbor(BodyPart* n);

	// breaks all links with neighbors, disconnecting from both sides and removing joints if present
	void disconnectAllNeighbors();

	// dead body parts turn into food
	void die();

	inline bool isDead() const { return dead_; }

	float getFoodValue() const { return foodValueLeft_; }
	void consumeFoodValue(float amount);

	Event<void(BodyPart* part)> onDied;

	float mass() const { return size_ * density_; }
	float size() const { return size_; }

	static Entity* getEntityFromBodyPartPhysBody(PhysicsBody const& body);

#ifdef DEBUG
	std::string getDebugName() const { throw std::runtime_error("Implement this!"); }
#endif

protected:
	BodyPartContext const& context_;
	// these are used when initializing the body and whenever a new commit is called.
	// they contain world-space values that are updated only prior to committing
	//PhysicsProperties cachedProps_;
	PhysicsBody physBody_;
	BodyPartType type_;

	std::vector<BodyPart*> neighbours_;

	//bool committed_;
	//bool noFixtures_ = false;
	// bool keepInitializationData_;	// set to true to not delete the initialData_ after commit()
	//bool dontCreateBody_;			// set to true to prevent creating an actual physics body
	/* this indicates if the values that come from genes (such as angleOffset_, size_ etc) have been cached
	 * into the object's variables.
	 * If not, one must sanitize and use directly the values from the initialData for whatever purposes.
	 */
	//bool geneValuesCached_;

	// final positioning and physical values:
	//float attachmentDirectionParent_;
	float size_;
	float density_;

	/**
	 * Lists of motor & sensor nerve lines that pass through this node.
	 * Each number represents the Nth motor/sensor input/output socket that has been created for this body.
	 * This correlates with the Nth CONNECTED output/input nerve in the brain
	 * (they are connected by VMS coordinates but kept in this order nonetheless)
	 */
	std::vector<unsigned> motorLines_;

	/*
	 * This is called after the decoding is finished and body structure is fully defined.
	 * At this point the physics fixtures must be created.
	 * The physicsProperties of the body are in world coordinates at this time;
	 */
	virtual void updateFixtures() = 0;
	//virtual void onAddedToParent() {}
	//virtual void onDetachedFromParent() {}


	//void registerAttribute(gene_part_attribute_type type, CumulativeValue& value);
	//void registerAttribute(gene_part_attribute_type type, unsigned index, CumulativeValue& value);
	// returns the attachment point for the current part in its parent's coordinate space.
	//glm::vec2 getUpstreamAttachmentPoint();
	//UpdateList* getUpdateList();
	// call this if the fixture changed for any reason:
	//void reattachChildren();
	//void computeBodyPhysProps();

//	friend class JointPivot;

	//virtual void detachMotorLines(std::vector<unsigned> const& lines);
	//virtual void hierarchyMassChanged();

	void buildDebugName(std::stringstream &out_stream) const;

	/*
	 * Adjusts a pair of values used for fixture size in order to avoid too small fixtures which would cause trouble with box2D.
	 * the values are assumed to be multiplied together (as width*length for example) yielding a fixture whose area is proportional to this product.
	 * If pair.second is 0.0, it is ignored and the first value is considered to be an area size and adjusted accordingly.
	 * outTotalRatio returns the final adjustment ratio (finalValuesMultiplied / initialValuesMultiplied).
	 * Use this ratio to adjust the fixture's density in order to keep the same mass.
	 */
	static std::pair<float, float> adjustFixtureValues(std::pair<float, float> const& val, float &outTotalRatio);

	// sets new values for size and density; Muscle should use this since it's size and density are not found in the Cell
	void overrideSizeAndDensity(float newSize, float newDensity);

private:
	//void reverseUpdateCachedProps();
	//glm::vec2 getParentSpacePosition();
	//bool applyScale_treeImpl(float scale, bool parentChanged);
	//void purge_initializationData();
	/** changes the attachment direction of this part to its parent. This doesn't take effect until commit is called */
	//inline void setAttachmentDirection(float angle) { attachmentDirectionParent_ = angle; }
	//void remove(BodyPart* part);

	//std::map<gene_part_attribute_type, std::vector<CumulativeValue*>> mapAttributes_;
	//std::shared_ptr<BodyPartInitializationData> initialData_;
	//UpdateList* updateList_;
	float lastCommitSize_inv_ = 0;
	bool destroyCalled_ = false;
	bool dead_ = false;
	float foodValueLeft_ = 0;

	ThreadLocalValue<uint32_t> UOID_ = 0; // Unique Operation IDentifier
};

#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
