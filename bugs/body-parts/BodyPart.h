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
#include <memory>
#include <atomic>

class Viewport;
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

	virtual void draw(Viewport* vp);

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

	/* scale the part by a factor; the part's fixtures and all the connecting joints are updated when a scale threshold is reached */
	virtual void applyScale(float scale);

	PhysicsBody const& getBody() { return physBody_; }

	// runs a given predicate on the current part and recursively on all of its neighbors and so on until the graph is completely covered,
	// or the evaluation is stopped by the predicate.
	// the traversal is thread-safe, concurrent calls on different threads are allowed
	// return false from the predicate to continue or true to stop evaluation.
	// the return value of the method indicates whether the evaluation was forcefully stopped by the predicate:
	//		- [true] means the evaluation was stopped by the predicate returning true.
	//		- [false] means the predicate was applied to the entire graph and no positive was detected.
	// UOID parameter is used internally, don't pass any value to it explicitly.
	bool applyPredicateGraph(std::function<bool(const BodyPart* pCurrent)> const& pred, uint32_t UOID=0) const;

	// the same as above except the predicates are allowed to modify the state of the object here
	// thus the thread-safety is compromised, caller must ensure the calls are synchronized.
	// also the method is slower because it has to take precautions to preserve the iterators even when neighbors are altered in the predicate
	bool applyPredicateGraphMutable(std::function<bool(BodyPart* pCurrent)> const& pred, uint32_t UOID=0);

	// adds the motor line id into this node and all nodes above it recursively
	// this id is the index of the nerve line from the neural network down to one of this motor's inputs
	void addMotorLine(int lineId);

	void addNeighbor(BodyPart* n);
	virtual void removeNeighbor(BodyPart* n);
	std::vector<BodyPart*> neighbors() const { return neighbours_; }

	// breaks all links with neighbors, disconnecting from both sides and removing joints if present
	void disconnectAllNeighbors();

	// dead body parts turn into food
	void die();

	inline bool isDead() const { return dead_.load(std::memory_order_acquire); }

	float getFoodValue() const { return foodValueLeft_; }
	void consumeFoodValue(float amount);

	Event<void(BodyPart* part)> onDied;

	float mass() const { return size_ * density_; }
	float size() const { return size_; }

	static Entity* getEntityFromBodyPartPhysBody(PhysicsBody const& body);

#ifdef DEBUG
	std::string getDebugName() const;
#endif

protected:
	BodyPartContext const& context_;
	PhysicsBody physBody_;
	BodyPartType type_;
	std::vector<BodyPart*> neighbours_;
	float size_;
	float density_;

	/*
	 * This is called after the decoding is finished and body structure is fully defined.
	 * At this point the physics fixtures must be created.
	 * The physicsProperties of the body are in world coordinates at this time;
	 */
	virtual void updateFixtures() = 0;

	virtual void destroyFixtures();

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

	bool destroyCalled() const { return destroyCalled_; }

private:
	float lastCommitSize_inv_ = 0;
	bool destroyCalled_ = false;
	std::atomic<bool> dead_ { false };
	float foodValueLeft_ = 0;

#ifdef DEBUG
	std::string divisionPath_;
#endif

	mutable ThreadLocalValue<uint32_t> UOID_ = 0; // Unique Operation IDentifier
};

#endif /* OBJECTS_BODY_PARTS_BODYPART_H_ */
