/*
 * Renderer.h
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include "IRenderer.h"
#include <glm/mat4x4.hpp>
#include <vector>

class Renderer : public IRenderer{
public:
	virtual ~Renderer();
	Renderer();

	virtual void registerRenderable(Renderable r);
	virtual const glm::mat4& getMatViewProj() const { return matViewProj; }

	virtual void setScreenSize(int width, int height);
	virtual float getZoomLevel() { return zoomLevel; }
	virtual void setZoomLevel(float zoom);
	virtual void moveCamera(glm::vec2 delta);
	virtual void moveCameraTo(glm::vec2 where);
	virtual glm::vec2 getCameraPos() { return cameraPos; }

	void render();

protected:
	std::vector<IRenderer::Renderable> renderComponents;
	glm::mat4 matViewProj;
	float zoomLevel;
	int screenWidth;
	int screenHeight;
	glm::vec2 cameraPos;

	void updateViewProj();
};

#endif /* RENDERER_H_ */
