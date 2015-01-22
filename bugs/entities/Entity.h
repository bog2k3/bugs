/*
 * Entity.h
 *
 *  Created on: Jan 21, 2015
 *      Author: bog
 */

#ifndef ENTITIES_ENTITY_H_
#define ENTITIES_ENTITY_H_

class RenderContext;

class Entity {
public:
	virtual ~Entity();

	typedef unsigned FunctionalityFlags;
	static constexpr FunctionalityFlags FF_NONE			= 0;
	static constexpr FunctionalityFlags FF_DRAWABLE		= 1;
	static constexpr FunctionalityFlags FF_UPDATABLE	= 2;

	// these flags MUST NOT change during the life time of the object, or else UNDEFINED BEHAVIOUR
	virtual FunctionalityFlags getFunctionalityFlags() { return FF_NONE; }

	virtual void update(float dt) {}
	virtual void draw(RenderContext const& ctx) {}

	void destroy();

private:
	bool markedForDeletion_ = false;
	friend class World;
};

#endif /* ENTITIES_ENTITY_H_ */
