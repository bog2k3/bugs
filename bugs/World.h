/*
 * World.h
 *
 *  Created on: Nov 12, 2014
 *      Author: bogdan
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "objects/WorldObject.h"
#include "input/operations/IOperationSpatialLocator.h"
#include "renderOpenGL/RenderContext.h"
#include "drawable.h"
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <vector>
#include <deque>

class b2World;
class b2Body;

class World : public IOperationSpatialLocator, public b2QueryCallback {
public:
	static World* getInstance();
	virtual ~World();

	b2Body* getBodyAtPos(glm::vec2 pos) override;
	void getObjectsInBox(glm::vec2 bottomLeft, glm::vec2 topRight, std::vector<WorldObject*> &outVec) override;

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	bool ReportFixture(b2Fixture* fixture) override;

	void setPhysics(b2World* physWld);
	b2World* getPhysics() { return physWld; }
	b2Body* getGroundBody() { return groundBody; }

	// Factory methods:
	template<typename T, typename TP1>
	std::shared_ptr<T> createObject(TP1 const &p1) {
		auto ptr = std::make_shared<T>(p1);
		addObject(ptr);
		return ptr;
	}
	template<typename T, typename TP1, typename TP2>
	std::shared_ptr<T> createObject(TP1 const &p1, TP2 const &p2) {
		auto ptr = std::make_shared<T>(p1, p2);
		addObject(ptr);
		return ptr;
	}
	template<typename T, typename TP1, typename TP2, typename TP3>
	std::shared_ptr<T> createObject(TP1 &p1, TP2 &p2, TP3 &p3) {
		auto ptr = std::make_shared<T>(p1, p2, p3);
		addObject(ptr);
		return ptr;
	}
	template<typename T, typename TP1, typename TP2, typename TP3, typename TP4>
	std::shared_ptr<T> createObject(TP1 &p1, TP2 &p2, TP3 &p3, TP4 &p4) {
		auto ptr = std::make_shared<T>(p1, p2, p3, p4);
		addObject(ptr);
		return ptr;
	}
	template<typename T, typename TP1, typename TP2, typename TP3, typename TP4, typename TP5>
	std::shared_ptr<T> createObject(TP1 const &p1, TP2 const &p2, TP3 const &p3, TP4 const &p4, TP5 const &p5) {
		auto ptr = std::make_shared<T>(p1, p2, p3, p4, p5);
		addObject(ptr);
		return ptr;
	}

protected:
	World();
	b2World* physWld;
	b2Body* groundBody;
	std::vector<std::shared_ptr<WorldObject>> objects;
	std::deque<b2Fixture*> b2QueryResult;

	void addObject(std::shared_ptr<WorldObject> obj);
	void removeObject(std::shared_ptr<WorldObject> obj);

	friend class WorldObject;
	friend void draw<World*>(World*& wld, RenderContext& ctx);
};

template<> void draw(World*& wld, RenderContext& ctx);

#endif /* WORLD_H_ */
