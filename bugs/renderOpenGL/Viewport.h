#pragma once

#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

class Camera;

class Viewport
{
public:
	Viewport(int x, int y, int w, int h);
	virtual ~Viewport();
	double getScale() { return fScale; }
	void setScale(double scale);
	void pan(glm::vec2 deltaPixels);
	void centerOn(glm::vec2 worldPoint);
	glm::vec4 getBkColor() { return glm::vec4(0); }
	void setArea(int vpX, int vpY, int vpW, int vpH);
	Camera* getCamera();
	int getWidth() { return viewportArea.z; }
	int getHeight() { return viewportArea.w; }
	bool isEnabled() { return mEnabled; }
	void setEnabled(bool enabled) { mEnabled = enabled; }
	/**
	 * returned vector: x-X, y-Y, z-Width, w-Height
	 */
	glm::vec4 getScreenRect() {return viewportArea; }
	glm::vec2 project(glm::vec2 point);
	glm::vec2 unproject(glm::vec2 point);
	/*void queueOSDElement(IOSDElement* pElement, LayoutAnchorEnum anchor, int Xpos, int Ypos);
	void renderOSD(RenderContext* pRenderContext);
	void addPermanentOSDElement(IOSDElement* pElement, LayoutAnchorEnum anchor, int Xpos, int Ypos);
	void removePermanentOSDElement(IOSDElement* pElement);*/

	long getUserData() { return m_userData; }
	void setUserData(long data) { m_userData = data; }

protected:
	long m_userData;
	glm::vec4 viewportArea;
	Camera* pCamera;
	double fScale;
	bool mEnabled;
	/*std::vector<IOSDElement*> OSD_vec;
	std::vector<IOSDElement*> OSDPerm_vec;
	IRenderer* pRenderer;

	void refreshOSDLayout(int oldWidth, int oldHeight);*/
};
