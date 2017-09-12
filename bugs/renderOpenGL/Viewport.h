#pragma once

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <string>

class Camera;

class Viewport
{
public:
	Viewport(int x, int y, int w, int h);
	virtual ~Viewport();

	glm::vec3 bkColor() const { return backgroundColor_; }
	void setBkColor(glm::vec3 c) { backgroundColor_ = c; }
	Camera* camera() const { return pCamera; }
	int width() const { return viewportArea.z; }
	int height() const { return viewportArea.w; }
	float aspect() const { return (float)width() / height(); }
	glm::vec2 position() const { return glm::vec2(viewportArea.x, viewportArea.y); }
	bool isEnabled() const { return mEnabled; }
	bool containsPoint(glm::vec2 const&p) const;
	/**
	 * returned vector: x-X, y-Y, z-Width, w-Height
	 */
	glm::vec4 screenRect() const {return viewportArea; }
	glm::vec3 project(glm::vec3 point) const;
	glm::vec3 unproject(glm::vec3 point) const;

	void setEnabled(bool enabled) { mEnabled = enabled; }
	void setArea(int vpX, int vpY, int vpW, int vpH);

	long userData() { return m_userData; }
	void setUserData(long data) { m_userData = data; }

	std::string name() const { return name_; }

protected:
	long m_userData = 0;
	glm::vec4 viewportArea;
	Camera* pCamera = nullptr;
	bool mEnabled = true;
	glm::vec3 backgroundColor_;
	std::string name_ {"unnamed"};

	void setName(std::string name) { name_ = name; }

	friend class Renderer;
};
