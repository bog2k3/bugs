#include "stdafx.h"
#include "BSPTree.h"
#include "utils/ThreadSlotDispatcher.h"

namespace lifeApplication
{
	using Render::IRenderable;
	using BSP::BSPNode;
	using BSP::IPartitionable;

	void BSPTree::addObject(WorldObject* pObject)
	{
		smtx_treeNodes.acquireRead();

		pObject->setPositionChangedHandler(&objectPosChanged, this);
		vector<BSPNode*> *pTmpNodeVec = poolNodeVectors->borrow();
		pObject->retrieveBSPNodes(NULL, true); // make sure no nodes are stored in the object
		m_RootNode->placeObject(pObject, pTmpNodeVec);
		poolNodeVectors->release(pTmpNodeVec);

		smtx_treeNodes.releaseRead();
	}

	void BSPTree::removeObject(WorldObject* pObject)
	{
		smtx_treeNodes.acquireRead();

		vector<BSPNode*> *pTmpNodeVec = poolNodeVectors->borrow();
		pTmpNodeVec->clear();
		pObject->retrieveBSPNodes(pTmpNodeVec, true);
		for (int i=0, n=pTmpNodeVec->size(); i<n; ++i)
			(*pTmpNodeVec)[i]->removeObject(pObject);
		poolNodeVectors->release(pTmpNodeVec);

		smtx_treeNodes.releaseRead();
	}

	void BSPTree::objectPosChanged(IPartitionable* pObj, void* pThis)
	{
		((BSPTree*)pThis)->updateObjectRelation(dynamic_cast<WorldObject*>(pObj));
	}

	void BSPTree::updateObjectRelation(WorldObject* pObj)
	{
		vector<BSPNode*> *pTmpNodeVec = poolNodeVectors->borrow();
		pTmpNodeVec->clear();

		smtx_treeNodes.acquireRead();

		// step1: remove the object from all previous nodes:
		pObj->retrieveBSPNodes(pTmpNodeVec, true);
		for (int i=0, n=pTmpNodeVec->size(); i<n; ++i)
			(*pTmpNodeVec)[i]->removeObject(pObj);
		// step2: trace the object and add it to new nodes:
		pTmpNodeVec->clear();
		m_RootNode->placeObject(pObj, pTmpNodeVec);

		smtx_treeNodes.releaseRead();

		poolNodeVectors->release(pTmpNodeVec);
	}

	void BSPTree::retrieveWorldObjectsInBox(AlignedBox &box, std::vector<WorldObject*> *pOutVec)
	{
		vector<BSPNode*> *vecNodes = poolNodeVectors->borrow();
		vecNodes->clear();
		smtx_treeNodes.acquireRead();
		m_RootNode->traceBox(box, vecNodes);	// get nodes inside box
		int threadSlot = ThreadSlotDispatcher::acquireSlot();	// get a slot for tagged operation
		DWORD UIDTag = ThreadSlotDispatcher::generateUID();	// generate tag
		for (int i=0, nsize=vecNodes->size(); i<nsize; ++i) {	// go through nodes and 
			vector<IPartitionable*> const &vObjects = (*vecNodes)[i]->acquireObjectsVec();	// get contained objects
			// figure out which objects have not already been added, and add them
			for (int j=0, nobj=vObjects.size(); j<nobj; ++j) {
				if (vObjects[j]->setUIDTag(threadSlot, UIDTag)) {
					if (vObjects[j]->probeBox(box)) // check object against box
						pOutVec->push_back(dynamic_cast<WorldObject*>(vObjects[j]));
				}
			}
			(*vecNodes)[i]->releaseObjectsVec();
		}
		smtx_treeNodes.releaseRead();
		ThreadSlotDispatcher::releaseSlot(threadSlot);
		poolNodeVectors->release(vecNodes);
	}

	void BSPTree::retrieveObjectsInBox(AlignedBox &box, vector<IRenderable*> *pOutVec)
	{
		vector<WorldObject*> *pVecObj = poolWorldObjVectors->borrow();
		pVecObj->clear();
		retrieveWorldObjectsInBox(box, pVecObj);
		std::copy(pVecObj->begin(), pVecObj->end(), back_inserter(*pOutVec));
		poolWorldObjVectors->release(pVecObj);
	}

	void BSPTree::retrieveObjectsInCircle(Circle const &circle, vector<Physics::PhysicObject*> &outVector)
	{
		//TODO implement
	}

	void BSPTree::optimize()
	{
		smtx_treeNodes.acquireWrite();
		m_RootNode->updateAndOptimize();
		smtx_treeNodes.releaseWrite();
	}

	BSPTree::BSPTree()
		: m_RootNode(new BSPNode())
		, smtx_treeNodes()

	{
		poolNodeVectors = new ObjectPool<BSPNodeVector>(4,1,this);
		poolWorldObjVectors = new ObjectPool<WorldObjVector>(4,1,this);
	}

	BSPTree::~BSPTree()
	{
		delete m_RootNode;
		delete poolNodeVectors;
		delete poolWorldObjVectors;
	}
}