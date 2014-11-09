 #pragma once

/* TODO 
 * http://www.codeproject.com/Articles/17745/Checked-Iterators
 * pentru optimizare iterator la vector
 */

#include "../utils/objectPool.h"
#include "../utils/sharedMutex.h"
#include "../math/math.h"

namespace BSP
{
	class IPartitionable;
	class BSPNode;
	
	struct Portal
	{
		double x1, y1, x2, y2;
		BSPNode* target;
	};

	class BSPNode : 
		public IPoolObjectLifeManager<BSPNode>, 
		public IPoolObjectLifeManager<Portal>
	{
	public:
		BSPNode();
		~BSPNode();

		// optimizes the tree; call this at the end of the frame.
		// 
		// !!! CALLER MUST ENSURE NO OTHER TREE OPEARTIONS ARE TAKING PLACE DURING THIS CALL !!!
		// 
		// [NOT SYNCHRONIZED]
		void updateAndOptimize();

		// traces the specified point down the tree and returns the leaf node that contains it
		// [NOT SYNCHRONIZED]
		const BSPNode* tracePoint(double x, double y) const;
		
		// traces a box down the tree and populates the pOut vector with all the nodes that
		// intersect the box;
		// 
		// [NOT SYNCHRONIZED for node structure; EXPLICIT SYNCHRO for objects]
		void traceBox(AlignedBox const &box, std::vector<BSPNode*>* pOut) const;
		void traceBox(ArbitraryBox const &box, std::vector<BSPNode*>* pOut) const;

		// acquires the object vector locking it, and returns a reference to it.
		// as long as the vector is locked it cannot be modified by an outside source, so 
		// iterating through it is guaranteed to be safe.
		// call releaseObjectsVec() when done to unlock it
		inline const std::vector<IPartitionable*> & acquireObjectsVec() { smtx_objects.acquireRead(); return m_objects; }

		// releases the lock on the objects vector, making it available to other consumers.
		inline void releaseObjectsVec() { smtx_objects.releaseRead(); }

		// traces the object against the tree and adds it to the leaf nodes it overlaps
		// in the second parameter pass an optional vector for performance optimization
		// or NULL, in which case a vector will be created internally.
		// 
		// [EXPLICIT SYNCHRO]
		void placeObject(IPartitionable* pObj, vector<BSPNode*>* pTmpNodeVector);

		// removes an object from THIS node if it exists (does not trace down the tree)
		// 
		// [EXPLICIT SYNCHRO]
		void removeObject(const IPartitionable* pObj);

	protected:
		bool verticalSplit;
		double splitCoordinate;
		BSPNode *pNegativeChild;
		BSPNode *pPositiveChild;
		sharedMutex smtx_objects; // mutex for accessing the objects within a node

		std::vector<Portal*> m_portals;
		std::vector<IPartitionable*> m_objects; //TODO: replace vector with set (faster binary lookup & uniqueness guaranteed - no need for UID)

		inline void setSplitDirection(bool vertical) { verticalSplit = vertical; }
		void reset(); // called before return to pool
		void releaseChildren();
		void addObjectRef(IPartitionable* pObj);

		static ObjectPool<BSPNode>* nodePool;
		static ObjectPool<Portal>* portalPool;
		static int shared_resource_counter;

		virtual void createPoolObject(BSPNode **pOut) { *pOut = new BSPNode(); }
		virtual void destroyPoolObject(BSPNode *pObj) { delete pObj; }
		virtual void createPoolObject(Portal **pOut) { *pOut = new Portal(); }
		virtual void destroyPoolObject(Portal *pObj) { delete pObj; }
	};
}
