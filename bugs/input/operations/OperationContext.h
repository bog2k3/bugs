#pragma once

class Viewport;

class LifeApp;
class BSPTree;
class OperationsStack;
class Viewport;

class OperationContext
{
public:
	BSPTree* pBSPTree;
	Viewport* pViewport;
	OperationsStack* pStack;

	OperationContext(Viewport* pViewport, BSPTree* pBSPTree, OperationsStack* pStack)
		: pBSPTree(pBSPTree)
		, pViewport(pViewport)
		, pStack(pStack) {
	}
};
