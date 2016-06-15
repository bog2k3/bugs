/*
 * SignalViewer.h
 *
 *  Created on: Jan 24, 2016
 *      Author: bog
 */

#ifndef OSD_SIGNALVIEWER_H_
#define OSD_SIGNALVIEWER_H_

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <vector>

class RenderContext;
class SignalDataSource;

class SignalViewer {
public:

	struct DataInfo {
		std::unique_ptr<SignalDataSource> source_;
		std::string name_;
		glm::vec3 color_;
		float lastMinValue_ = 0.f;
		float lastMaxValue_ = 0.f;
		float minUpperY_ = -1e20f;
		float maxLowerY_ = 1e20f;

		DataInfo() = default;
		DataInfo(DataInfo &&x) = default;
		DataInfo(std::unique_ptr<SignalDataSource> && source, std::string const& name, glm::vec3 const& color, float minUpperY, float maxLowerY)
			: source_(std::move(source)), name_(name), color_(color),
			  minUpperY_(minUpperY), maxLowerY_(maxLowerY) {
		}
	};

	SignalViewer(glm::vec3 const& uniformPos, glm::vec2 const& uniformSize);
	virtual ~SignalViewer();

	void addSignal(std::string const& name, float* pValue, glm::vec3 const& rgb, float sampleInterval, int maxSamples = 50, float minUpperY = -1e20f, float maxLowerY = 1e20f);

	void update(float dt);
	void draw(RenderContext const& ctx);

private:
	std::vector<DataInfo> sourceInfo_;
	glm::vec3 uPos_;
	glm::vec2 uSize_;
};

class SignalDataSource {
public:
	SignalDataSource(float* pValue, int maxSamples, float sampleInterval);
	virtual ~SignalDataSource();
	void update(float dt);
	inline float getSample(uint i) { return samples_[(i+zero_)%n_]; }
	inline uint getNumSamples() { return n_; }
	inline uint getCapacity() { return capacity_; }
private:
	float* pValue_ = nullptr;
	float* samples_ = nullptr;	// circular buffer
	uint n_ = 0;
	uint zero_ = 0;
	uint capacity_ = 0;
	float sampleInterval_ = 0;
	float timeSinceLastSample_ = 0;
};

#endif /* OSD_SIGNALVIEWER_H_ */
