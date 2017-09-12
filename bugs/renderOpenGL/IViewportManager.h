/*
 * IViewportManager.h
 *
 *  Created on: May 6, 2017
 *      Author: bog
 */

#ifndef RENDEROPENGL_IVIEWPORTMANAGER_H_
#define RENDEROPENGL_IVIEWPORTMANAGER_H_

#include <memory>
#include <vector>
#include <string>

class Viewport;

class IViewportManager {
public:
	virtual ~IViewportManager() {}

	virtual int windowWidth() const = 0;
	virtual int windowHeight() const = 0;

	virtual void addViewport(std::string name, std::unique_ptr<Viewport> vp) = 0;
	virtual Viewport* getViewport(std::string name) const = 0;
	virtual std::vector<Viewport*> getViewports() const = 0;
	virtual void deleteViewport(std::string const& name) = 0;
	virtual void clearViewports() = 0;
};

#endif /* RENDEROPENGL_IVIEWPORTMANAGER_H_ */
