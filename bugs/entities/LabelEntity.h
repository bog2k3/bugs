/*
 * LabelEntity.h
 *
 *  Created on: May 18, 2017
 *      Author: bog
 */

#ifndef ENTITIES_LABELENTITY_H_
#define ENTITIES_LABELENTITY_H_

#include "Entity.h"
#include "enttypes.h"
#include "../OSD/Label.h"

#include <memory>
#include <string>
#include <set>

class LabelEntity: public Entity {
public:
	LabelEntity(std::string text, float size, ViewportCoord pos, float z, glm::vec3 color={1.f,1.f,1.f}, std::set<std::string> viewportFilter = {});
	virtual ~LabelEntity();

	EntityType getEntityType() const override { return EntityType::LABEL; }
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE; }

	void draw(RenderContext const&) override;

private:
	std::unique_ptr<Label> label_;
	std::set<std::string> filter_;
};

#endif /* ENTITIES_LABELENTITY_H_ */
