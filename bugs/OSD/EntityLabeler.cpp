/*
 * EntityLabeler.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: bog
 */

#include "EntityLabeler.h"
#include "../entities/Entity.h"
#include "../OSD/Label.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Viewport.h"
#include "../renderOpenGL/Camera.h"
#include "../math/math3D.h"
#include "../math/aabb.h"
#include <vector>

class DecoratorLayout {
public:
	DecoratorLayout(aabb const& box, int padding, float zoomLevel)
		: box_(box), padding_(padding), zoomLevel_(zoomLevel) {
		nextPosition_ = glm::vec2(box.vMax.x + padding / zoomLevel_, box.vMax.y);
	}
	glm::vec2 nextPosition(glm::vec2 decoratorSize) {
		glm::vec2 ret = nextPosition_;
		switch(side_) {
		case 0 /*right*/:
			nextPosition_.y -= (padding_ + decoratorSize.y) / zoomLevel_;
			if (nextPosition_.y < box_.vMin.y) {
				// switch to left side
				nextPosition_ = glm::vec2(box_.vMin.x - padding_ / zoomLevel_, box_.vMax.y);
				side_++;
			}
			break;
		case 1 /*left*/:
			ret.x -= decoratorSize.x / zoomLevel_;
			nextPosition_.y -= (padding_ + decoratorSize.y) / zoomLevel_;
			if (nextPosition_.y < box_.vMin.y) {
				// switch to bottom side
				nextPosition_ = glm::vec2(box_.vMin.x, box_.vMin.y - padding_ / zoomLevel_);
				side_++;
			}
			break;
		default:
			assertDbg(false);
			throw std::runtime_error("must implement!");
		}
		return ret;
	}

private:
	aabb box_;
	int padding_;
	float zoomLevel_;
	int side_ = 0;	// 0 is right, 1 is left, 2 is below, 3 is above
	glm::vec2 nextPosition_;
};

EntityLabeler::EntityLabeler() {
}

EntityLabeler& EntityLabeler::getInstance() {
	static EntityLabeler instance;
	return instance;
}

void EntityLabeler::draw(RenderContext const&) {
	std::vector<const Entity*> entsToRemove;
	for (auto const& p : labels_) {
		if (p.first->isZombie()) {
			entsToRemove.push_back(p.first);
			continue;
		}
		// TODO restore
		/*auto aabb = p.first->getAABB();
		DecoratorLayout layout(aabb, 5, ctx.viewport->getCamera()->getZoomLevel()); // 5 pixel padding
		for (auto const& lp : p.second) {
			auto &label = lp.second.label_;
			label->setPos(glm::vec3(layout.nextPosition(label->getBoxSize(ctx)), 0));
			label->draw(ctx);
		}*/
	}
	for (auto e : entsToRemove)
		labels_.erase(labels_.find(e));
}

void EntityLabeler::setEntityLabel(const Entity* ent, std::string const& name, std::string const& value, glm::vec3 rgb) {
	EntLabel& el = labels_[ent][name];
	el.rgb_ = rgb;
	el.value_ = value;
	if (!el.label_) {
		auto xc = [ent] (Viewport* vp) -> float {
			return 0; //vp->project(ent->position()).x;
		};
		auto yc = [ent] (Viewport* vp) -> float {
			return 0; //vp->project(ent->position()).y;
		};
		el.label_ = std::unique_ptr<Label>(new Label(el.value_, {xc, yc}, 0, LABEL_TEXT_SIZE, el.rgb_));
	} else {
		el.label_->setText("[" + name + "] " + el.value_);
		el.label_->setColor(el.rgb_);
	}
}

void EntityLabeler::removeEntityLabel(const Entity* ent, std::string const& name) {
	auto it = labels_[ent].find(name);
	if (it != labels_[ent].end()) {
		labels_[ent].erase(it);
	}
}
