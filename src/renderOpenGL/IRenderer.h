/*
 * IRenderer.h
 *
 *  Created on: Oct 30, 2014
 *      Author: bog
 */

#ifndef IRENDERER_H_
#define IRENDERER_H_

#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <functional>

class IRenderer
{
public:
	virtual ~IRenderer() {}

	typedef std::function<void()> Renderable;

	virtual void registerRenderable(Renderable r) = 0;
	virtual const glm::mat4& getMatViewProj() const = 0;

	virtual void setScreenSize(int width, int height) = 0;
	virtual float getZoomLevel() = 0;
	virtual void setZoomLevel(float zoom) = 0;
	virtual void moveCamera(glm::vec2 delta) = 0;
	virtual void moveCameraTo(glm::vec2 where) = 0;
	virtual glm::vec2 getCameraPos() = 0;
};

#endif /* IRENDERER_H_ */
