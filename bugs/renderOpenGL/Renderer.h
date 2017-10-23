/*
 * Renderer.h
 *
 *  Created on: Nov 2, 2014
 *      Author: bog
 */

#ifndef RENDERER_H_
#define RENDERER_H_

#include "IRenderable.h"
#include "IViewportManager.h"
#include <vector>
#include <map>

class RenderContext;

class Renderer : public IViewportManager {
public:
	virtual ~Renderer();
	Renderer(int windowWidth, int windowHeight);

	void registerRenderable(IRenderable* r);

	int windowWidth() const override { return winW_; }
	int windowHeight() const override { return winH_; }
	void addViewport(std::string name, std::unique_ptr<Viewport> vp) override;
	Viewport* getViewport(std::string name) const override;
	std::vector<Viewport*> getViewports() const override;
	void deleteViewport(std::string const& name) override;
	void clearViewports() override { viewports_.clear(); }

	void render(RenderContext const& ctx);

	void unload();

protected:
	std::vector<IRenderable*> renderComponents_;
	std::map<std::string, std::unique_ptr<Viewport>> viewports_;

	int winW_ = 0;
	int winH_ = 0;
};

#endif /* RENDERER_H_ */
