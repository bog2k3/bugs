#pragma once

class Viewport;

class OperationsStack;
class Viewport;
class IOperationSpatialLocator;
class b2World;

class OperationContext
{
public:
	Viewport* pViewport;
	OperationsStack* pStack;
	IOperationSpatialLocator* locator;
	b2World* physics;

	OperationContext(Viewport* pViewport, OperationsStack* pStack, IOperationSpatialLocator* locator, b2World* physics)
		: pViewport(pViewport)
		, pStack(pStack)
		, locator(locator)
		, physics(physics) {
	}
};
