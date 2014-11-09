#pragma once

#include <d3dx9math.h>
#include "../math/math.h"

namespace BSP
{
	class BSPNode;
	class IPartitionable;

	typedef void (*positionChangedDelegate)(IPartitionable* pObj, void* pEventContext);

	class IPartitionable abstract
	{
	public:
		virtual ~IPartitionable() {}

		// sets an UID tag on a specified slot; used for UID-controlled operations
		// each slot corresponds to one concurrent thread.
		// returns true if tag was different before, and false if tag was the same
		virtual bool setUIDTag(int threadSlot, DWORD UID)=0;

		// retrieves the world-space axis-aligned bounding box of the object
		virtual void getBoundingBox(AlignedBox *pOutBox)=0;

		// check the object against a world-space box; return true if they overlap
		virtual bool probeBox(AlignedBox &boxToProbe)=0;

		// save a reference inside the object to the BSP node given as param.
		// this is called when the object is being placed in the BSP, for the BSP leaf nodes
		// that the object overlaps.
		virtual void addBSPNodeRef(BSPNode* pNodeRef)=0;

		// retrieves the BSP nodes that the object overlaps; 
		// if purgeNodes is true, the internal list of nodes should be cleared upon return
		// the pOutNodeVec may be NULL when purging.
		virtual void retrieveBSPNodes(std::vector<BSPNode*>* pOutNodeVec, bool purgeNodes)=0;

		// This is called once per object to set a callback that the object *must* call
		// when it's position (more precisely bounding box) has changed. This is required
		// in order to update the object's relation with the BSP tree.
		// When invoking the delegate, pass back the same context as the one received here
		// Failing to invoke this delegate will result in a corrupted object's relation with the BSP tree.
		virtual void setPositionChangedHandler(positionChangedDelegate delegate, void* pEventContext)=0;
	};
}