/*
 * BodyPart.cpp
 *
 *  Created on: Dec 10, 2014
 *      Author: bog
 */

#include "BodyPart.h"
#include "BodyConst.h"
#include "../math/box2glm.h"
#include "../renderOpenGL/RenderContext.h"
#include "../renderOpenGL/Shape2D.h"
#include "../math/math2D.h"
#include "../utils/log.h"
#include "../utils/assert.h"
#include "../genetics/GeneDefinitions.h"
#include <glm/gtx/rotate_vector.hpp>
#include <Box2D/Dynamics/b2Body.h>
#include <cassert>

BodyPartInitializationData::BodyPartInitializationData()
	: angleOffset(0)
	, lateralOffset(0)
	, size(BodyConst::initialBodyPartSize)
	, density(BodyConst::initialBodyPartDensity)
{
}

BodyPart::BodyPart(PART_TYPE type, std::shared_ptr<BodyPartInitializationData> initialData)
	: type_(type)
	, parent_(nullptr)
	, children_{nullptr}
	, nChildren_(0)
	, committed_(false)
	, dontCreateBody_(false)
	, geneValuesCached_(false)
	, attachmentDirectionParent_(0)
	, angleOffset_(0)
	, lateralOffset_(0)
	, size_(0.01f)
	, density_(1.f)
	, initialData_(initialData)
	, updateList_(nullptr)
	, lastCommitSize_inv_(0)
{
	assert (initialData != nullptr);

	registerAttribute(GENE_ATTRIB_LOCAL_ROTATION, initialData_->angleOffset);
	registerAttribute(GENE_ATTRIB_ATTACHMENT_OFFSET, initialData_->lateralOffset);
	registerAttribute(GENE_ATTRIB_SIZE, initialData_->size);
}

BodyPart::~BodyPart() {
	changeParent(nullptr);
	for (int i=0; i<nChildren_; i++)
		delete children_[i];
}

void BodyPart::applyRecursive(std::function<void(BodyPart* pCurrent)> pred) {
	pred(this);
	for (int i=0; i<nChildren_; i++)
		children_[i]->applyRecursive(pred);
}

void BodyPart::addMotorLine(int line) {
	motorLines_.push_back(line);
	if (parent_)
		parent_->addMotorLine(line);
}

void BodyPart::addSensorLine(int line) {
	sensorLines_.push_back(line);
	if (parent_)
		parent_->addSensorLine(line);
}

int circularPrev(int index, int n) {
	if (n == 0)
		return 0;
	return (index+n-1) % n;
}
int circularNext(int index, int n) {
	if (n == 0)
		return 0;
	return (index+1) % n;
}
bool angleSpanOverlap(float angle1, float span1, float angle2, float span2, float &outMargin) {
	// will set outMargin to negative if overlap, or positive shortest gap around element if no overlap
	// move angle1 in origin for convenience:
	angle2 = limitAngle(angle2 - angle1, PI);
	angle1 = 0;
	float a1p = span1*0.5f, a1n = -span1*0.5f;
	float a2p = angle2 + span2*0.5f, a2n = angle2 - span2*0.5f;
	if (a2p*a2n > 0) {
		// both ends of span2 are on the same side of angle 1
		if (a2p > 0) // both on the positive side
			outMargin = min(a2p, a2n) - a1p;
		else // both on the negative side
			outMargin = a1n - max(a2n, a2p);
	} else // ends of span2 are on different sides of angle 1
		outMargin = min(a2n-a1p, a1n-a2p);	// the smallest distance between the spans (positive) or the greatest overlap (negative)

	return outMargin < 0;
}

inline float getAngularSize(float r, float width) {
	return 2 * atanf(width/(2*r));
}

void BodyPart::pushBodyParts(int circularBufferIndex, float delta) {
	if (delta == 0)
		return;
	float gapNext = 0;
	int i=circularBufferIndex;
	int di = sign(delta);
	do {
		BodyPartInitializationData::angularEntry &entry = initialData_->circularBuffer[i];
		children_[entry.childIndex]->setAttachmentDirection(children_[entry.childIndex]->attachmentDirectionParent_ + delta);
		if (di > 0) {
			gapNext = entry.gapAfter - delta;
			if (gapNext < 0)
				entry.gapAfter = 0;
		} else /* (di < 0)*/ {
			gapNext = entry.gapBefore + delta;  // delta is negative here
			if (gapNext < 0)
				entry.gapBefore = 0;
		}
		i += di;
	} while (gapNext > 0);
}

float BodyPart::add(BodyPart* part, float angle) {
	assert(nChildren_ < MAX_CHILDREN && initialData_);
LOGPREFIX("BodyPart::add");
LOGLN("enter type"<<this->type_<<"; angle:"<<angle);
	// determine the position in the circular buffer:
	int bufferPos = 0;
	while (bufferPos < nChildren_ && angle >= children_[initialData_->circularBuffer[bufferPos].childIndex]->attachmentDirectionParent_)
		bufferPos++;
	float r = glm::length(getChildAttachmentPoint(angle));
	float span = getAngularSize(r, part->getAttachmentWidth()) * 1.1f; // *1.1f = allow some margin
LOGLN("add at pos : "<<bufferPos<<"; span:"<<span);
#warning "getAttachmentWidth() will return default part size since it's just been created; must update layout when size changes"
	float gapNeeded = span;
	float gapLeftBefore = 2*PI-span, gapLeftAfter = 2*PI-span; // initial values valid for no other children case
	int nextIdx = circularNext(bufferPos, nChildren_);
	bool overlapsNext = false;
	if (nextIdx != bufferPos) {
		float nextAngle = children_[initialData_->circularBuffer[nextIdx].childIndex]->attachmentDirectionParent_;
		float nextSpan = initialData_->circularBuffer[nextIdx].angularSize;
		overlapsNext = angleSpanOverlap(angle, gapNeeded, nextAngle, nextSpan, gapLeftAfter);
	}
	int prevIdx = circularPrev(bufferPos, nChildren_);
	bool overlapsPrev = false;
	if (prevIdx != bufferPos) {
		float prevAngle = children_[initialData_->circularBuffer[prevIdx].childIndex]->attachmentDirectionParent_;
		float prevSpan = initialData_->circularBuffer[prevIdx].angularSize;
		overlapsPrev = angleSpanOverlap(angle, gapNeeded, prevAngle, prevSpan, gapLeftBefore);
	}
	if (!overlapsNext && !overlapsPrev)
		gapNeeded = 0;
	// more iterations may be required, since the first gaps found may not be enough:
	while (gapNeeded > 0) {
		// walk the circular buffer and compute 'mass' and gap
		float MBefore = 0, gapBefore = 0; // compute push 'mass' and available gap before angle
		float MAfter = 0, gapAfter = 0; // compute push 'mass' and available gap after angle
		if (!overlapsNext) {
			MAfter = span;
			gapAfter = gapLeftAfter;
		} else if (!overlapsPrev) {
			MBefore = span;
			gapBefore = gapLeftBefore;
		} else {
			// both overlap
			// walk positive:
			int iNext = bufferPos;
			do {
				iNext = circularNext(iNext, nChildren_);
				MAfter += getAngularSize(r, children_[initialData_->circularBuffer[iNext].childIndex]->getAttachmentWidth());
				gapAfter = initialData_->circularBuffer[iNext].gapAfter;
			} while (eqEps(gapAfter, 0) && iNext != bufferPos);
			// walk negative:
			int iPrev = circularNext(bufferPos, nChildren_);
			do {
				iPrev = circularPrev(iPrev, nChildren_);
				MAfter += getAngularSize(r, children_[initialData_->circularBuffer[iPrev].childIndex]->getAttachmentWidth());
				gapBefore = initialData_->circularBuffer[iPrev].gapBefore;
			} while (eqEps(gapBefore, 0) && iPrev != bufferPos);
		}
		if (gapBefore+gapAfter < span) {
			assert(false && "not enough room for the new part, must decrease the size of the new part or of its siblings... nasty.");
		}
		float MRatio = MAfter / MBefore;
		float pushBef = gapNeeded / (1 + MRatio);
		float pushAft = pushBef * MRatio;
		if (pushBef > gapBefore) {
			pushAft *= gapBefore / pushBef;
			pushBef = gapBefore;
		} else
			gapLeftBefore = gapBefore - pushBef;
		if (pushAft > gapAfter) {
			pushBef *= gapAfter / pushAft;
			pushAft = gapAfter;
		} else
			gapLeftAfter = gapAfter - pushAft;
		if (overlapsNext)
			pushBodyParts(circularNext(bufferPos, nChildren_), pushAft);	// must alter the attachmentDirectionParent of the siblings
		else
			angle += pushAft;
		if (overlapsPrev)
			pushBodyParts(bufferPos, -pushBef); // -||-
		else
			angle += -pushBef;
		gapNeeded -= pushBef + pushAft;
		if (overlapsNext && overlapsPrev)
			angle += (pushAft - pushBef) * 0.5f; // adjust angle to the middle of the gap
	}
	for (int i=bufferPos+1; i<=nChildren_; i++)
		initialData_->circularBuffer[i] = initialData_->circularBuffer[i-1];
	initialData_->circularBuffer[bufferPos].set(nChildren_, span, gapLeftBefore, gapLeftAfter);
	// todo update gaps for neighbors
	children_[nChildren_] = part;
	nChildren_++;
	part->setAttachmentDirection(angle);
	return angle;
}

void BodyPart::changeParent(BodyPart* newParent) {
	if (parent_)
		parent_->remove(this);
	parent_ = newParent;
	if (parent_)
		parent_->add(this, attachmentDirectionParent_);
}

void BodyPart::remove(BodyPart* part) {
	for (int i=0; i<nChildren_; i++)
		if (children_[i] == part) {
			children_[i] = children_[--nChildren_];
			break;
		}
}

glm::vec2 BodyPart::getUpstreamAttachmentPoint() {
	if (!parent_)
		return glm::vec2(0);
	else {
		glm::vec2 point(parent_->getChildAttachmentPoint(attachmentDirectionParent_));
		assert(!std::isnan(point.x) && !std::isnan(point.y));
		return point;
	}
}

void BodyPart::commit_tree(float initialScale) {
	if (!committed_) {
		initialData_->size.changeRel(initialScale);
		cacheInitializationData();
		geneValuesCached_ = true;
		purge_initializationData();
		computeBodyPhysProps();
	} else
		reverseUpdateCachedProps();
	lastCommitSize_inv_ = 1.f / size_;
	// perform commit on local node:
	if (type_ != BODY_PART_JOINT) {
		if (!physBody_.b2Body_ && !dontCreateBody_)
			physBody_.create(cachedProps_);
		commit();
	}
	// perform recursive commit on all non-muscle children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ != BODY_PART_MUSCLE)
			children_[i]->commit_tree(initialScale);
	}
	// muscles go after all other children:
	for (int i=0; i<nChildren_; i++) {
		if (children_[i]->type_ == BODY_PART_MUSCLE)
			children_[i]->commit_tree(initialScale);
	}

	if (type_ == BODY_PART_JOINT) {
		commit();
	}
	committed_ = true;
}

void BodyPart::purge_initializationData() {
	initialData_.reset();
}

glm::vec2 BodyPart::getParentSpacePosition() {
	if (!geneValuesCached_)
		cacheInitializationData();
	glm::vec2 upstreamAttach = getUpstreamAttachmentPoint();
	glm::vec2 localOffset = getChildAttachmentPoint(PI - angleOffset_);
	assert(!std::isnan(localOffset.x) && !std::isnan(localOffset.y));
	float angle;
	angle = attachmentDirectionParent_ + angleOffset_;
	glm::vec2 ret(upstreamAttach - glm::rotate(localOffset, angle));
	assert(!std::isnan(ret.x) && !std::isnan(ret.y));
	return ret;
#warning "must take into account lateral offset"
}

void BodyPart::reverseUpdateCachedProps() {
	// reverse the magic here: get values from the physics engine and put them in our cached props
	// in order to facilitate a recommit with changed data.
	glm::vec3 worldTransform = getWorldTransformation();
	cachedProps_.angle = worldTransform.z;
	cachedProps_.position = vec3xy(worldTransform);
	if (physBody_.b2Body_) {
		cachedProps_.velocity = b2g(physBody_.b2Body_->GetLinearVelocity());
		cachedProps_.angularVelocity = physBody_.b2Body_->GetAngularVelocity();
	} else if (parent_) {
		cachedProps_.velocity = parent_->cachedProps_.velocity;
		cachedProps_.angularVelocity = parent_->cachedProps_.angularVelocity;
	}
}

void BodyPart::computeBodyPhysProps() {
	// do the magic here and update cachedProps from other positioning fields
	// cachedProps must be in world space
	// parent's cachedProps are assumed to be updated and in world space at this time
	PhysicsProperties parentProps = parent_ ? parent_->cachedProps_ : PhysicsProperties();
	cachedProps_.velocity = parentProps.velocity;
	cachedProps_.angularVelocity = parentProps.angularVelocity;
	// compute parent space position:
	glm::vec2 pos = getParentSpacePosition();
	assert(!std::isnan(pos.x) && !std::isnan(pos.y));
	// compute world space position:
	pos = parentProps.position + glm::rotate(pos, parentProps.angle);
	assert(!std::isnan(pos.x) && !std::isnan(pos.y));
	cachedProps_.position = pos;
	// compute world space angle:
	cachedProps_.angle = parentProps.angle + attachmentDirectionParent_ + angleOffset_;
	assert(!std::isnan(cachedProps_.angle));
}

glm::vec3 BodyPart::getWorldTransformation() {
	if (physBody_.b2Body_) {
		return glm::vec3(b2g(physBody_.b2Body_->GetPosition()), physBody_.b2Body_->GetAngle());
	} else {
		// if not committed yet, must compute these values on the fly
		glm::vec3 parentTransform(parent_ ? parent_->getWorldTransformation() : glm::vec3(0));
		glm::vec2 pos = getParentSpacePosition(); // this will cache gene values as well
		return parentTransform + glm::vec3(
				glm::rotate(pos, parentTransform.z),
				attachmentDirectionParent_ + angleOffset_);
	}
}

void BodyPart::draw(RenderContext const& ctx) {
	if (committed_)
		return;
	glm::vec3 trans = getWorldTransformation();
	glm::vec2 pos(trans.x, trans.y);
	ctx.shape->drawLine(pos + glm::vec2(-0.01f, 0), pos + glm::vec2(0.01f, 0), 0, glm::vec3(0.2f, 0.2f, 1.f));
	ctx.shape->drawLine(pos + glm::vec2(0, -0.01f), pos + glm::vec2(0, 0.01f), 0, glm::vec3(1.f, 0.2f, 0.2f));
}

void BodyPart::registerAttribute(gene_part_attribute_type type, CummulativeValue& value) {
	mapAttributes_[type] = &value;
}

UpdateList* BodyPart::getUpdateList() {
	if (updateList_)
		return updateList_;
	else if (parent_) {
		updateList_ = parent_->getUpdateList();
		return updateList_;
	} else
		return nullptr;
}

float BodyPart::getMass_tree() {
	float mass = size_ * density_;

	for (int i=0; i<nChildren_; i++)
		mass += children_[i]->getMass_tree();
	return mass;

#warning "muscle mass must somehow count toward total body mass - maybe add it to torso?"
}

void BodyPart::applyScale_tree(float scale) {
	applyScale_treeImpl(scale, false);
}

bool BodyPart::applyScale_treeImpl(float scale, bool parentChanged) {
	size_ *= scale;
	bool committed_now = false, should_commit_joint = false;
	if (committed_) {
		if (size_ * lastCommitSize_inv_ > BodyConst::SizeThresholdToCommit
				|| size_ * lastCommitSize_inv_ < BodyConst::SizeThresholdToCommit_inv)
		{
			lastCommitSize_inv_ = 1.f / size_;
			if (type_ != BODY_PART_JOINT) {
				commit();
				committed_now = true;
			} else
				should_commit_joint = true;
		}
	}
	bool child_changed = false;
	for (int i=0; i<nChildren_; i++) {
		child_changed |= children_[i]->applyScale_treeImpl(scale, committed_now);
	}
	if (type_ == BODY_PART_JOINT && committed_ && (should_commit_joint || parentChanged || child_changed)) {
		// must commit a joint whenever the threshold is reached, or parent or child has committed
		commit();
		committed_now = true;
	}

	return committed_now;
}

void BodyPart::consumeEnergy(float amount) {
	if (parent_)
		parent_->consumeEnergy(amount);
}

void BodyPart::die_tree() {
	die();
	for (int i=0; i<nChildren_; i++)
		children_[i]->die_tree();
}

void BodyPart::reattachChildren() {
	if (committed_) {
		for (int i=0; i<nChildren_; i++) {
			children_[i]->commit();
		}
	}
}

void BodyPart::draw_tree(RenderContext const& ctx) {
	draw(ctx);
	for (int i=0; i<nChildren_; i++)
		children_[i]->draw_tree(ctx);
}

void BodyPart::cacheInitializationData() {
	angleOffset_ = limitAngle(initialData_->angleOffset, 2*PI);
	lateralOffset_ = initialData_->lateralOffset;
	size_ = initialData_->size.clamp(BodyConst::MinBodyPartSize, 1.e10f);
	density_ = initialData_->density.clamp(BodyConst::MinBodyPartDensity, BodyConst::MaxBodyPartDensity);
}
