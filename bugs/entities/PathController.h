/*
 * PathController.h
 *
 *  Created on: May 21, 2017
 *      Author: bogdan
 */

#ifndef PHYSICS_PATHCONTROLLER_H_
#define PHYSICS_PATHCONTROLLER_H_

#include "Entity.h"
#include "../utils/path-lerper.h"
#include "../utils/glmstream.h"

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace physics {
class DynamicBody;
}

class PathController : public Entity {
public:

	struct Vertex {
		glm::vec3 position;
		glm::fquat orientation;
		glm::vec3 scale;

		Vertex(glm::vec3 pos = glm::vec3{0}, glm::fquat orientation = glm::fquat{}, glm::vec3 scale = glm::vec3{1, 1, 1})
			: position(pos), orientation(orientation), scale(scale) {
		}

		float operator-(Vertex x) const {
			float d = glm::length(position - x.position);
			if (abs(d) < 1.e-5f) {
				float dr = 2 * acos(glm::cross(x.orientation, glm::inverse(orientation)).w);
				if (dr == 0) {
					if (x.scale == scale)
						return 0;
					else
						return glm::length(x.scale - scale);
				}
				else
					return dr;
			} else
				return d;
		}

		static Vertex lerp(Vertex const& v1, Vertex const& v2, float f) {
//			DEBUGLOGLN("mix f=" << f << "  v1=" << v1.orientation << "  v2=" << v2.orientation << "  res= " << glm::slerp(v1.orientation, v2.orientation, f));
			return Vertex {
				v1.position * (1-f) + v2.position * f,
				glm::slerp(v1.orientation, v2.orientation, f),
				v1.scale * (1-f) + v2.scale * f
			};
		}
	};

	EntityType getEntityType() const override { return EntityType::PATH_CONTROLLER; }
	FunctionalityFlags getFunctionalityFlags() const { return FunctionalityFlags::UPDATABLE; }

	PathController(physics::DynamicBody *body);
	virtual ~PathController();

	void addVertex(Vertex v);
	void addRedirect(int index);
	void start(float speed);

	void update(float dt);

	Vertex value() const { return lerper_.value(); }
	Vertex vertex(int index) const { return lerper_.vertex(index); }

private:
	PathLerper<Vertex, decltype(&PathController::Vertex::lerp)> lerper_{Vertex::lerp};
	physics::DynamicBody* body_;
};

#endif /* PHYSICS_PATHCONTROLLER_H_ */
