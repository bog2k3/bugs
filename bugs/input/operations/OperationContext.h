#pragma once

class Viewport;

class OperationsStack;
class Viewport;
class IOperationSpatialLocator;
class IWorldManager;

class OperationContext
{
public:
	Viewport* pViewport;
	OperationsStack* pStack;
	IOperationSpatialLocator* locator;
	IWorldManager* worldManager;

	OperationContext(Viewport* pViewport, OperationsStack* pStack, IOperationSpatialLocator* locator, IWorldManager* worldManager)
		: pViewport(pViewport)
		, pStack(pStack)
		, locator(locator)
		, worldManager(worldManager) {
	}
};
