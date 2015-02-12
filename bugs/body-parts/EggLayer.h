/*
 * EggLayer.h
 *
 *  Created on: Feb 10, 2015
 *      Author: bogdan
 */

#ifndef BODY_PARTS_EGGLAYER_H_
#define BODY_PARTS_EGGLAYER_H_

#include "BodyPart.h"

#define DEBUG_DRAW_EGGLAYER

class EggLayer: public BodyPart {
public:
	EggLayer();
	virtual ~EggLayer() override;

	void draw(RenderContext const& ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;
	void update(float dt);

protected:
	void commit() override;
	void onAddedToParent() override;
};

#endif /* BODY_PARTS_EGGLAYER_H_ */
