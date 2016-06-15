#pragma once

#include "BSP/BSPNode.h"
#include "Render/ISpatialResolver.h"
#include "Physics/ISpatialResolver.h"
#include "Physics/PhysicObject.h"
#include "utils/objectPool.h"
#include "utils/sharedMutex.h"
#include "WorldObject.h"

namespace lifeApplication
{
	using BSPNodeVector = std::vector<BSP::BSPNode*>;
	using WorldObjVector = std::vector<WorldObject*>;

	class BSPTree : 
		public Render::ISpatialResolver,
		public Physics::ISpatialResolver,
		public IPoolObjectLifeManager<BSPNodeVector>,
		public IPoolObjectLifeManager<WorldObjVector>
	{
	public:
		BSPTree();
		virtual ~BSPTree();

		// adds an object to the tree (also updates object's internal node references)
		// call this only once per object!
		//
		// [EXPLICIT THREAD SYNC]
		void addObject(WorldObject* pObject);

		// removes an object from the tree (also removes object's internal node references)
		// [EXPLICIT THREAD SYNC]
		void removeObject(WorldObject* pObject);

		// retrieves a list of objects (as IRenderable instances) that are found completely 
		// or partially inside the box defined by (x1,y1) & (x2,y2) in world space
		// this method only appends the objects to the vector, does not clear it before.
		// [EXPLICIT THREAD SYNC]
		virtual void Render::ISpatialResolver::retrieveObjectsInBox(AlignedBox &box, 
			vector<Render::IRenderable*> *pOutVec);

		// retrieves a list of objects (as PhysicObject instances) that are found at least partially
		// inside the specified circle (circle is in world space)
		// this method appends the objects to the vector without affecting its already existing contents
		// [EXPLICIT THREAD SYNC]
		virtual void Physics::ISpatialResolver::retrieveObjectsInCircle(Circle const &circle, 
			vector<Physics::PhysicObject*> &outVector);

		// retrieves a list of objects (as WorldObject instances) that are found completely 
		// or partially inside the box defined by (x1,y1) & (x2,y2) in world space
		// this method only appends the objects to the vector, does not clear it before.
		// [EXPLICIT THREAD SYNC]
		void retrieveWorldObjectsInBox(AlignedBox &box, std::vector<WorldObject*> *pOutVec);

		inline BSP::BSPNode* getRootNode() { return m_RootNode; }

		// optimizes the tree's internal structure; call at most once per frame, but less often is better
		void optimize();

	protected:
		BSP::BSPNode* m_RootNode;
		sharedMutex smtx_treeNodes; // mutex for all access to the tree nodes

		ObjectPool<BSPNodeVector> *poolNodeVectors;
		ObjectPool<WorldObjVector> *poolWorldObjVectors;
		
		virtual void createPoolObject(BSPNodeVector **pOut) { *pOut = new BSPNodeVector(); }
		virtual void destroyPoolObject(BSPNodeVector* pObj) { delete pObj; }
		virtual void createPoolObject(WorldObjVector **pOut) { *pOut = new WorldObjVector(); }
		virtual void destroyPoolObject(WorldObjVector* pObj) { delete pObj; }

		static void objectPosChanged(BSP::IPartitionable* pObj, void* pThis);
		void updateObjectRelation(WorldObject* pObj);
	};
}
