/*
 * Muscle.h
 *
 *  Created on: Dec 23, 2014
 *      Author: bog
 */

#ifndef OBJECTS_BODY_PARTS_MUSCLE_H_
#define OBJECTS_BODY_PARTS_MUSCLE_H_

#include "BodyPart.h"

class Muscle: public BodyPart {
public:
	Muscle(BodyPart* parent, PhysicsProperties props); // the position and rotation in props are relative to the parent
	virtual ~Muscle() override;

	void commit() override {} // NO COMMIT since our muscles are not really physical constructs, only theoretical
	void draw(ObjectRenderContext* ctx) override;
	glm::vec2 getChildAttachmentPoint(float relativeAngle) override;

	float getSize() { return size_; }
	float getAspectRatio() { return aspectRatio_; }
	float getInsertionOffset() { return insertionOffset_; }


protected:
	CummulativeValue size_;
	CummulativeValue aspectRatio_;
	CummulativeValue insertionOffset_;	// % of target bone' length
};

#endif /* OBJECTS_BODY_PARTS_MUSCLE_H_ */
