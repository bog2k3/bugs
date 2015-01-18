#include <iostream>

#include "renderOpenGL/glToolkit.h"
#include "renderOpenGL/Shape2D.h"
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
#include "DrawList.h"
#include "UpdateList.h"
#include "objects/OSD/ScaleDisplay.h"

#include <GLFW/glfw3.h>
#include <Box2D/Box2D.h>

#include <sstream>
#include "renderOpenGL/GLText.h"

template<> void draw(b2World*& wld, RenderContext &ctx) {
	wld->DrawDebugData();
}

/*void statsHdr() {
	std::cout << "t,phi,omega,tau,e\n";
}

void stats(float t, float phi, float omega, float tau, float e) {
	std::cout<<t<<","<<phi<<","<<omega<<","<<tau<<","<<e<<"\n";
}*/

int main() {
	LOGGER("app_main");

	if (!gltInit(800, 600, "Bugs"))
		return -1;

	Renderer renderer;
	Viewport vp1(0, 0, 800, 600);
	renderer.addViewport(&vp1);
	RenderContext renderContext(
			&vp1,
			new Shape2D(&renderer),
			new GLText(&renderer, "data/fonts/DejaVuSansMono_256_16_8.png", 8, 16, ' ', 22));

	b2World physWld(b2Vec2_zero);
	PhysicsDebugDraw physicsDraw(renderContext);
	physicsDraw.SetFlags(
				  b2Draw::e_shapeBit
				//| b2Draw::e_centerOfMassBit
				// | b2Draw::e_jointBit
				//| b2Draw::e_aabbBit
			);
	physWld.SetDebugDraw(&physicsDraw);

	World::getInstance()->setPhysics(&physWld);

	OperationsStack opStack(&vp1, World::getInstance(), World::getInstance(), &physWld);
	GLFWInput::initialize(gltGetWindow());
	GLFWInput::setListener(std::bind(&OperationsStack::handleInputEvent, &opStack, std::placeholders::_1));
	opStack.pushOperation(std::unique_ptr<OperationPan>(new OperationPan(InputEvent::MB_RIGHT)));
	opStack.pushOperation(std::unique_ptr<IOperation>(new OperationSpring(InputEvent::MB_LEFT)));

	/*
	 * joint motor test:
	 *
	b2BodyDef bdef;
	bdef.angularDamping = bdef.linearDamping = 0.4f;
	bdef.type = b2_dynamicBody;
	bdef.position.Set(-1.f, 0.f);
	b2Body* b1 = physWld.CreateBody(&bdef);
	bdef.position.Set(+0.5f, 0.f);
	//bdef.angle = PI/2;
	b2Body* b2 = physWld.CreateBody(&bdef);

	b2FixtureDef fdef;
	fdef.density = 1.f;
	fdef.restitution = 0.6f;
	b2PolygonShape shp;
	shp.SetAsBox(0.9f, 0.2f);
	fdef.shape = &shp;
	b1->CreateFixture(&fdef);
	shp.SetAsBox(0.2f, 1.9f);
	b2->CreateFixture(&fdef);

	b2RevoluteJointDef jdef;
	jdef.Initialize(b1, b2, b2Vec2(0, 0));
	jdef.enableMotor = true;
	jdef.maxMotorTorque = 0.5f;
	jdef.motorSpeed = PI/2;
	b2RevoluteJoint* j = (b2RevoluteJoint*)physWld.CreateJoint(&jdef);

	float phi = j->GetJointAngle();
	float e = 0;
	statsHdr();
	stats(0, phi, 0, 0, e);
	//*/

	Bug* b1 = Bug::newBasicBug(glm::vec2(0, 0));
	Bug* b2 = Bug::newBasicBug(glm::vec2(0.4f, 0));
	Bug* b3 = Bug::newBasicBug(glm::vec2(-0.4f, 0));
	Bug* b4 = Bug::newBasicBug(glm::vec2(0, 0.4f));

	UpdateList updateList;
	updateList.add(b1);
	//updateList.add(b2);
	//updateList.add(b3);
	//updateList.add(b4);

	DrawList drawList;
	drawList.add(World::getInstance());
	drawList.add(&physWld);
	drawList.add(ScaleDisplay(glm::vec2(15, 25), 300));

	float t = glfwGetTime();
	while (GLFWInput::checkInput()) {
		float newTime = glfwGetTime();
		float dt = newTime - t;
		t = newTime;

		if (dt > 0) {
			opStack.update(dt);
			physWld.Step(dt, 6, 2);
			updateList.update(dt);
		}
		// wait until previous frame finishes rendering and show frame output:
		gltEnd();

		/*float lastPhi = phi;
		phi = j->GetJointAngle();
		float omega = (phi - lastPhi) / dt;
		float tau = j->GetMotorTorque(1.f/dt);
		e += tau * (phi-lastPhi);
		stats(t, phi, omega, tau, e);
		*/

		// draw builds the render queue for the current frame
		drawList.draw(renderContext);

		std::stringstream ss;
		ss << "Salut Lume!\n[Powered by Box2D]";
		renderContext.text->print(ss.str().c_str(), 20, vp1.getHeight()-20, 16, glm::vec3(0.2f, 0.4, 1.0f));

		// do the actual openGL render for the previous frame (which is independent of our world)
		gltBegin();
		renderer.render();
		// now rendering is on-going, move on to the next update:
	}

	delete renderContext.shape;

	return 0;
}
