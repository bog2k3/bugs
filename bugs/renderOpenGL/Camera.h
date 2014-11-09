/*
 * Camera.h
 *
 *  Created on: Nov 9, 2014
 *      Author: bog
 */

#ifndef RENDEROPENGL_CAMERA_H_
#define RENDEROPENGL_CAMERA_H_

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

class Viewport;

class Camera {
public:
	Camera(Viewport* vp);
	virtual ~Camera();

	const glm::mat4& getMatViewProj() const { return matViewProj; }

	float getZoomLevel() { return zoomLevel; }
	void setZoomLevel(float zoom);
	void move(glm::vec2 delta);
	void moveTo(glm::vec2 where);
	glm::vec2 getPos() { return cameraPos; }

protected:
	Viewport* pViewport;
	glm::mat4 matViewProj;
	float zoomLevel;
	glm::vec2 cameraPos;

	void updateViewProj();
};

#endif /* RENDEROPENGL_CAMERA_H_ */
