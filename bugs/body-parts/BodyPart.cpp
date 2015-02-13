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
	removeFromParent();
	for (int i=0; i<nChildren_; i++)
		delete children_[i];
}

bool BodyPart::applyRecursive(std::function<bool(BodyPart* pCurrent)> pred) {
	if (pred(this))
		return true;
	for (int i=0; i<nChildren_; i++)
		if (children_[i]->applyRecursive(pred))
			return true;
	return false;
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
	if (a2p*a2n >= 0) {
		// both ends of span2 are on the same side of angle 1
		if (a2p > 0) // both on the positive side
			outMargin = min(a2p, a2n) - a1p;
		else // both on the negative side
			outMargin = a1n - max(a2n, a2p);
	} else { // ends of span2 are on different sides of angle 1
		float d1 = a2n-a1p, d2 = a1n-a2p;
		outMargin = angle2 > 0 ? d1 : d2; // the smallest distance between the spans (positive) or the greatest overlap (negative)
	}

	return outMargin < 0;
}

void BodyPart::pushBodyParts(int index, float delta) {
	if (delta == 0)
		return;
	int di = sign(delta);
	int initialIndex = index;
	while (true) {
		BodyPartInitializationData::angularEntry &entry = initialData_->circularBuffer[index];
		children_[index]->setAttachmentDirection(children_[index]->attachmentDirectionParent_ + delta);
		entry.gapBefore += delta;
		entry.gapAfter -= delta;
		if (di > 0) {
			if (entry.gapAfter >= 0)
				break;
		} else /* (di < 0)*/ {
			// delta is negative here
			if (entry.gapBefore >= 0)
				break;
		}
		index = di > 0 ? circularNext(index, nChildren_) : circularPrev(index, nChildren_);
		assert(index != initialIndex && "came around full circle; something's fucked up");
	}
}

float BodyPart::add(BodyPart* part, float angle) {
	assert(nChildren_ < MAX_CHILDREN && initialData_);
	// determine the position in the circular buffer:
	int bufferPos = 0;
	while (bufferPos < nChildren_ && angle >= children_[bufferPos]->attachmentDirectionParent_)
		bufferPos++;
	for (int i=nChildren_; i>bufferPos; i--) {
		initialData_->circularBuffer[i] = initialData_->circularBuffer[i-1];
		children_[i] = children_[i-1];
	}
	nChildren_++;
	children_[bufferPos] = part;
	part->parent_ = this;
	part->setAttachmentDirection(angle);
	part->onAddedToParent();
	constexpr float span = 2*PI/MAX_CHILDREN;
	float gapLeftBefore = 2*PI-span, gapLeftAfter = 2*PI-span; // initial values valid for no other children case
	bool overlapsNext = false, overlapsPrev = false;
	if (nChildren_ > 1) {
		int nextPos = circularNext(bufferPos, nChildren_);
		float nextAngle = children_[nextPos]->attachmentDirectionParent_;
		constexpr float nextSpan = span;
		overlapsNext = angleSpanOverlap(angle, span, nextAngle, nextSpan, gapLeftAfter);
		int prevPos = circularPrev(bufferPos, nChildren_);
		float prevAngle = children_[prevPos]->attachmentDirectionParent_;
		constexpr float prevSpan = span;
		overlapsPrev = angleSpanOverlap(angle, span, prevAngle, prevSpan, gapLeftBefore);
		if (nChildren_ == 2) {
			// next and previous elements are the same, must fix gaps
			overlapsNext = overlapsNext && bufferPos == 0;
			overlapsPrev = overlapsPrev && bufferPos == 1;
			if (!overlapsNext && overlapsPrev)
				gapLeftAfter = 2*PI - span - nextSpan - gapLeftBefore;
			if (!overlapsPrev && overlapsNext)
				gapLeftBefore = 2*PI - span - prevSpan - gapLeftAfter;
		}
		initialData_->circularBuffer[nextPos].gapBefore = gapLeftAfter;
		initialData_->circularBuffer[prevPos].gapAfter = gapLeftBefore;
	}
	initialData_->circularBuffer[bufferPos].set(gapLeftBefore, gapLeftAfter);
	// done
	if (overlapsNext || overlapsPrev)
		fixOverlaps(bufferPos);
	return children_[bufferPos]->attachmentDirectionParent_;
}

void BodyPart::fixOverlaps(int startIndex) {
	bool overlapsNext = initialData_->circularBuffer[startIndex].gapAfter < 0;
	bool overlapsPrev = initialData_->circularBuffer[startIndex].gapBefore < 0;
	float gapNeeded = (overlapsNext ? -initialData_->circularBuffer[startIndex].gapAfter : 0) +
					(overlapsPrev ? -initialData_->circularBuffer[startIndex].gapBefore : 0);
	constexpr float span = 2*PI/MAX_CHILDREN;
	// more iterations may be required, since the first gaps found may not be enough:
	while (gapNeeded > 0) {
		// walk the circular buffer and compute 'mass' and gap
		float MBefore = 0, gapBefore = 0; // compute push 'mass' and available gap before angle
		float MAfter = 0, gapAfter = 0; // compute push 'mass' and available gap after angle
		int firstNext = circularNext(startIndex, nChildren_), firstPrev = circularPrev(startIndex, nChildren_);
		bool wrapAround = false;
		if (!overlapsNext) {
			MAfter = span;
			gapAfter = initialData_->circularBuffer[startIndex].gapAfter;
		} else {
			// walk positive:
			int iNext = startIndex;
			do {
				iNext = circularNext(iNext, nChildren_);
				MAfter += span;
				gapAfter = initialData_->circularBuffer[iNext].gapAfter;
			} while (gapAfter < EPS && (nChildren_ == 2 || iNext != firstPrev));
			if (nChildren_ > 2 && iNext == firstPrev) {	// forward wrap around
				gapAfter = 0;
				wrapAround = true;
			}
		}
		if (!overlapsPrev) {
			MBefore = span;
			gapBefore = initialData_->circularBuffer[startIndex].gapBefore;
		} else {
			// walk negative:
			int iPrev = startIndex;
			do {
				iPrev = circularPrev(iPrev, nChildren_);
				MBefore += span;
				gapBefore = initialData_->circularBuffer[iPrev].gapBefore;
			} while (gapBefore < EPS && (nChildren_ == 2 || iPrev != firstNext));
			if (nChildren_ > 2 && iPrev == firstNext) {	// backward wrap around
				gapBefore = 0;
				wrapAround = true;
			}
		}
		assert(gapBefore + gapAfter > 0);	// there's always room for all children
		assert(MAfter*MBefore != 0);
		float MRatio = MAfter / MBefore;
		float pushBef = gapNeeded / (1 + MRatio);
		float pushAft = pushBef * MRatio;
		// check if there is a single gap:
		if (nChildren_ == 2 || wrapAround) {
			// distribute the semi-gaps proportional to the mass ratio
			float gapSum = gapBefore + gapAfter;
			gapAfter = pushAft / gapNeeded * gapSum;
			gapBefore = gapSum - gapAfter;
		}
		bool willOverlapNext = false, willOverlapPrev = false;
		if (pushBef > gapBefore) {
			pushAft *= gapBefore / pushBef;
			pushBef = gapBefore;
			willOverlapPrev = true;
		}
		if (pushAft > gapAfter) {
			pushBef *= gapAfter / pushAft;
			pushAft = gapAfter;
			willOverlapNext = true;
		}
		if (pushAft > 0) {
			pushBodyParts(overlapsNext ? firstNext : startIndex, pushAft);	// must alter the attachmentDirectionParent of the siblings
			initialData_->circularBuffer[startIndex].gapAfter += overlapsNext ? pushAft : -pushAft;
		}
		if (pushBef > 0) {
			pushBodyParts(overlapsPrev ? firstPrev : startIndex, -pushBef); // -||-
			initialData_->circularBuffer[startIndex].gapBefore += overlapsPrev ? pushBef : -pushBef;
		}
		gapNeeded -= pushBef + pushAft;
		if (overlapsNext && overlapsPrev)
			children_[startIndex]->attachmentDirectionParent_ += (pushAft - pushBef) * 0.5f; // adjust angle to the middle of the gap

		overlapsNext = willOverlapNext;
		overlapsPrev = willOverlapPrev;
	}
}

void BodyPart::removeFromParent() {
	if (parent_)
		parent_->remove(this);
	parent_ = nullptr;
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
		glm::vec2 pos = getParentSpacePosition(); // this will temporarily cache gene values as well
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
