#pragma once

class Viewport;

class LifeApp;
class BSPTree;
class OperationsStack;
class IViewport;

struct OperationContext
{
	BSPTree* pBSPTree;
	IViewport* pViewport;
	OperationsStack* pStack;

	OperationContext(IViewport* pViewport, BSPTree* pBSPTree, OperationsStack* pStack)
		: pBSPTree(pBSPTree)
		, pViewport(pViewport)
		, pStack(pStack) {
	}
};
