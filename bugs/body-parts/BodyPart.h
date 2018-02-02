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
	BodyPart(BodyPartType type, BodyPartContext const& context, BodyCell const& cell, bool suppressPhysicalBody=false);
	BodyPart(glm::vec2 position, float angle, glm::vec2 velocity, float angularVelocity, float mass, BodyPartContext const& context);
	virtual ~BodyPart();

	// call this to destroy and delete the object. Never delete directly
	void destroy();

	// this will return the division-depth of the body-part
	int getDepth() const { return 0; }

	virtual void draw(RenderContext const& ctx);

	inline BodyPartType getType() const { return type_; }

	// detaches this body part along with all its children from the parent part, breaking all neural connections.
	// this part and its children may die as a result of this operation if the parameter is true
	virtual void detach(bool die);

	/**
	 * return the attachment point for a neighbour of the current part, in the specified direction
	 * (in current's part coordinate frame).
	 * This is usually the point where the ray from the center intersects the edge of the body part.
	 */
	virtual glm::vec2 getAttachmentPoint(float relativeAngle) { return glm::vec2(0); }

	virtual glm::vec3 getWorldTransformation() const;

	aabb getAABB() const;

	// must return the actual amount deduced from mass argument
	virtual float addFood(float mass) { throw std::runtime_error("not implemented"); /*if (parent_) return parent_->addFood(mass); else return 0;*/ }

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

	// tells the entire hierarchy that the body died
	//void die_tree();

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

	// return false from the predicate to continue or true to break out; the ORed return value is passed back to the caller as method return
	//bool applyRecursive(std::function<bool(BodyPart* pCurrent)> pred);

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
	void removeAllLinks();

	inline bool isDead() const { return dead_; }

	float getFoodValue() const { return foodValueLeft_; }
	void consumeFoodValue(float amount);

	Event<void(BodyPart* part)> onDied;

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
	virtual void consumeEnergy(float amount);
	virtual void die() {}
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

	friend class JointPivot;

	virtual void detachMotorLines(std::vector<unsigned> const& lines);
	//virtual void hierarchyMassChanged();

	void buildDebugName(std::stringstream &out_stream) const;

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
};

#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
