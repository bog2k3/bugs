#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Shape2D.h"
#include "renderOpenGL/Text.h"
#include "renderOpenGL/Renderer.h"
#include "renderOpenGL/Viewport.h"
#include "input/GLFWInput.h"
#include "input/InputEvent.h"
#include "input/operations/OperationsStack.h"
#include "input/operations/OperationPan.h"
#include "input/operations/OperationSpring.h"
#include "objects/body-parts/Bone.h"
#include "objects/body-parts/Joint.h"
#include "World.h"
#include "PhysicsDebugDraw.h"
#include "math/math2D.h"
#include "log.h"
#include "entities/Bug.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>

int main()
{
	LOGGER("app_main");

	if (!gltInit(800, 600, "Bugs"))
		return -1;

	Renderer renderer;
	Viewport vp1(0, 0, 800, 600);
	renderer.addViewport(&vp1);
	ObjectRenderContext renderContext(new Shape2D(&renderer), &vp1);
	GLText::initialize("data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ');

	b2World physWld(b2Vec2_zero);
	PhysicsDebugDraw physicsDraw(renderContext);
	physicsDraw.SetFlags(
				  b2Draw::e_shapeBit
				//| b2Draw::e_centerOfMassBit
				| b2Draw::e_jointBit
				//| b2Draw::e_aabbBit
			);
	physWld.SetDebugDraw(&physicsDraw);

	World::getInstance()->setPhysics(&physWld);
	World::getInstance()->setRenderContext(renderContext);

	OperationsStack opStack(&vp1, World::getInstance(), World::getInstance(), &physWld);
	GLFWInput::initialize(gltGetWindow());
	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));
	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));

	Bug* b = Bug::newBasicBug(glm::vec2(0, 0));
	World::getInstance()->addUpdatable(b);

	b2BodyDef def1;
	def1.type = b2_dynamicBody;
	def1.angle = 0;
	def1.position.Set(0,0);
	b2BodyDef def2(def1);
	def2.angle = PI;
	def2.position.Set(-0.02f, 0);
	b2Body* b1 = physWld.CreateBody(&def1);
	b2Body* b2 = physWld.CreateBody(&def2);
	b2RevoluteJointDef def;
	def.Initialize(b1, b2, b2Vec2(-0.01f, 0));
	// physProps_.position must be in world space at this step:
	def.enableLimit = true;
	def.lowerAngle = -PI/8;
	def.upperAngle = PI/1.2f;

	b2RevoluteJoint* physJoint_ = (b2RevoluteJoint*)World::getInstance()->getPhysics()->CreateJoint(&def);

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {

		// do the actual openGL render (which is independent of our world)
		gltBegin();
		renderer.render();
		std::stringstream ss;
		ss << "Salut Lume!\n[Powered by Box2D]";
		GLText::print(ss.str().c_str(), 20, 20, 16);
		// now rendering is on-going, get on with other stuff:

		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;

		if (dt > 0) {
			opStack.update(dt);
			physWld.Step(dt, 6, 2);
			World::getInstance()->update(dt);
		}

		// draw builds the render queue
		World::getInstance()->draw();
		physWld.DrawDebugData();

		// wait until rendering is done and show frame output:
		gltEnd();
	}

	delete renderContext.shape;

	return 0;
}
