/*
 * LabelEntity.h
 *
 *  Created on: May 18, 2017
 *      Author: bog
 */

#ifndef ENTITIES_SIGVIEWERENTITY_H_
#define ENTITIES_SIGVIEWERENTITY_H_

#include "Entity.h"
#include "enttypes.h"
#include "../OSD/SignalViewer.h"

#include <memory>
#include <string>
#include <set>

class SigViewerEntity: public Entity {
public:
	SigViewerEntity(ViewportCoord pos, float z, ViewportCoord size, std::set<std::string> viewportFilter = {});
	virtual ~SigViewerEntity();

	EntityType getEntityType() const override { return EntityType::SIGNAL_VIEWER; }
	FunctionalityFlags getFunctionalityFlags() const override {
		return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE;
	}

	SignalViewer& get() { return *sigViewer_; }

	void draw(RenderContext const&) override;
	void update(float dt) override;

private:
	std::unique_ptr<SignalViewer> sigViewer_;
};

#endif /* ENTITIES_SIGVIEWERENTITY_H_ */
