#pragma once

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class Camera;

class Viewport
{
public:
	Viewport(int x, int y, int w, int h);
	virtual ~Viewport();

	// how many meters per pixel?
	double getScale() const { return fScale; }
	glm::vec4 getBkColor() const { return glm::vec4(0); }
	Camera* getCamera() const { return pCamera; }
	int getWidth() const { return viewportArea.z; }
	int getHeight() const { return viewportArea.w; }
	bool isEnabled() const { return mEnabled; }
	bool containsPoint(glm::vec2 const&p) const;
	/**
	 * returned vector: x-X, y-Y, z-Width, w-Height
	 */
	glm::vec4 getScreenRect() const {return viewportArea; }
	glm::vec2 project(glm::vec2 point) const;
	glm::vec2 unproject(glm::vec2 point) const;

	void setEnabled(bool enabled) { mEnabled = enabled; }
	void setArea(int vpX, int vpY, int vpW, int vpH);
	void setScale(double scale);
	void pan(glm::vec2 deltaPixels);
	void centerOn(glm::vec2 worldPoint);

	long getUserData() { return m_userData; }
	void setUserData(long data) { m_userData = data; }

protected:
	long m_userData;
	glm::vec4 viewportArea;
	Camera* pCamera;
	double fScale;
	bool mEnabled;
};
