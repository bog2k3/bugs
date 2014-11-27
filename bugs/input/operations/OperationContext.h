#pragma once

class Viewport;

class OperationsStack;
class Viewport;
class IOperationSpatialLocator;
class IWorldManager;
class b2World;

class OperationContext
{
public:
	Viewport* pViewport;
	OperationsStack* pStack;
	IOperationSpatialLocator* locator;
	IWorldManager* worldManager;
	b2World* physics;

	OperationContext(Viewport* pViewport, OperationsStack* pStack, IOperationSpatialLocator* locator,
			IWorldManager* worldManager, b2World* physics)
		: pViewport(pViewport)
		, pStack(pStack)
		, locator(locator)
		, worldManager(worldManager)
		, physics(physics) {
	}
};
