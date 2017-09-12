/*
 * CameraController.h
 *
 *  Created on: May 7, 2017
 *      Author: bog
 */

#ifndef ENTITIES_CAMERACONTROLLER_H_
#define ENTITIES_CAMERACONTROLLER_H_

#include "Entity.h"
#include "enttypes.h"

#include "../utils/path-lerper.h"

#include <glm/vec3.hpp>

#include <vector>

class Camera;

class CameraController: public Entity {
public:
	CameraController(Camera* target);
	virtual ~CameraController();

	EntityType getEntityType() const override { return EntityType::CAMERA_CTRL; }
	// these flags MUST NOT change during the life time of the object, or else UNDEFINED BEHAVIOUR
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::UPDATABLE; }

	void update(float dt) override;

	// starts a back-and-forth movement between p1 and p2. starts from the current position toward p1 first.
	void startBackAndForth(glm::vec3 p1, glm::vec3 p2, glm::vec3 lookAtTarget, float speed);

	struct CameraNode {
		glm::vec3 position;
		glm::vec3 lookAtTarget;

		CameraNode operator*(float factor) const {
			return CameraNode {
				position * factor, lookAtTarget * factor
			};
		}

		CameraNode operator+(CameraNode v) const {
			return CameraNode {
				position + v.position, lookAtTarget + v.lookAtTarget
			};
		}

		// distance function
		float operator-(CameraNode v) const {
			return (position - v.position).length();
		}
	};

	PathLerper<CameraNode>& pathLerper() { return pathLerper_; }

private:
	Camera* camera_ = nullptr;
	PathLerper<CameraNode> pathLerper_;
};

#endif /* ENTITIES_CAMERACONTROLLER_H_ */
