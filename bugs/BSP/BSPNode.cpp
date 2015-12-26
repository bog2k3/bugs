#if(0)
#include "stdafx.h"
#include "BSPNode.h"
#include "IPartitionable.h"

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

namespace BSP
{
	ObjectPool<BSPNode>* BSPNode::nodePool(NULL);
	ObjectPool<Portal>* BSPNode::portalPool(NULL);
	int BSPNode::shared_resource_counter(0);

	BSPNode::BSPNode()
		: verticalSplit(true)
		, splitCoordinate(0.0)
		, pNegativeChild(NULL)
		, pPositiveChild(NULL)
		, smtx_objects()
	{
		if (shared_resource_counter++ == 0) {
			nodePool = new ObjectPool<BSPNode>(1024, 128, this);
			portalPool = new ObjectPool<Portal>(1024, 128, this);
		}
	}

	BSPNode::~BSPNode()
	{
#ifdef DEBUG
		smtx_objects.acquireRead();
		assert(m_objects.empty()); // make sure all objects have been removed from the node prior to deletion
		smtx_objects.releaseRead();
#endif

		if (--shared_resource_counter == 0) {
			delete nodePool;
			delete portalPool;
		}
	}

	void BSPNode::updateAndOptimize()
	{
		//todo - bsp adaptiv:
		//	- daca un nod are copii, dar nr total de obj < threshold, stergem copii, unificam totul in nod
		//	- daca un nod e frunza si are mai mult de thresh obj, atunci facem split dinamic
		//	- pornim cu un singur nod root care se splituie dinamic
		
		//	poolNodes->borrow(negativeChild)
	}

	const BSPNode* BSPNode::tracePoint(double x, double y) const
	{
		if (pNegativeChild) {
			// we're not a leaf
			double searchCoord;
			if (verticalSplit)
				searchCoord = x;
			else // horizontal split
				searchCoord = y;

			if (searchCoord >= splitCoordinate)
				return pPositiveChild->tracePoint(x, y);
			else
				return pNegativeChild->tracePoint(x, y);
		} else
			return this; // we're a leaf
	}

	void BSPNode::traceBox(AlignedBox const &box, std::vector<BSPNode*>* pOut) const
	{
		assert(pOut != NULL);

		if (!pNegativeChild) { // if we're a leaf...
			pOut->push_back(const_cast<BSPNode*>(this));
			return;
		}
		// else we have children
		double searchC1, searchC2; // search coordinates - based on split direction
		if (verticalSplit) {
			searchC1 = box.bottomLeft.x; searchC2 = box.topRight.x;
		} else {
			searchC1 = box.bottomLeft.y; searchC2 = box.topRight.y;
		}
		if (searchC1 >= splitCoordinate || searchC2 >= splitCoordinate) {
			// at least part of the box is on the positive side, propagate to positive child
			pPositiveChild->traceBox(box, pOut);
		}
		if (searchC1 < splitCoordinate || searchC2 < splitCoordinate) {
			// at least part of the box is on the negative side, propagate to negative child
			pNegativeChild->traceBox(box, pOut);
		}
	}

	void BSPNode::placeObject(IPartitionable* pObj, vector<BSPNode*>* pTmpNodeVector)
	{
		assert(pObj != NULL);
		//pObj->retrieveBSPNodes(NULL, true); // purge previous node list

		bool local_vector = false;
		AlignedBox objectBBox;
		pObj->getBoundingBox(&objectBBox);
		if (pTmpNodeVector==NULL) {
			pTmpNodeVector = new vector<BSPNode*>;
			local_vector = true;
		} else
			pTmpNodeVector->clear();
		traceBox(objectBBox, pTmpNodeVector);

		for (int i=0, n=pTmpNodeVector->size(); i<n; ++i) {
			(*pTmpNodeVector)[i]->addObjectRef(pObj);
			pObj->addBSPNodeRef((*pTmpNodeVector)[i]);
		}
		if (local_vector)
			delete pTmpNodeVector;
	}

	void BSPNode::addObjectRef(IPartitionable* pObj)
	{
		smtx_objects.acquireWrite();
#ifdef DEBUG
		//TODO - replace with set so this search won't be necessary
		// check the object doesn't already exist in our list:
		assert(std::find(m_objects.begin(), m_objects.end(), pObj) == m_objects.end());
#endif
		m_objects.push_back(pObj);
		smtx_objects.releaseWrite();
	}

	void BSPNode::releaseChildren()
	{
		if (!pNegativeChild)
			return;

		pNegativeChild->reset();
		pNegativeChild = NULL;
		nodePool->release(pNegativeChild);

		pPositiveChild->reset();
		pPositiveChild = NULL;
		nodePool->release(pPositiveChild);
	}

	// called before return to pool
	void BSPNode::reset()
	{
		releaseChildren();
		m_portals.clear();
		smtx_objects.acquireWrite();
		m_objects.clear();
		smtx_objects.releaseWrite();
	}

	void BSPNode::removeObject(const IPartitionable *pObj)
	{
		smtx_objects.acquireWrite();
		m_objects.erase(
			std::remove(m_objects.begin(), m_objects.end(), pObj), 
			m_objects.end()
		);
		smtx_objects.releaseWrite();
	}
}
#endif
