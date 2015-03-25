/*
 * GuiHelper.h
 *
 *  Created on: Mar 25, 2015
 *      Author: bog
 */

#ifndef GUI_GUIHELPER_H_
#define GUI_GUIHELPER_H_

#include <memory>
#include <vector>
#include <glm/vec2.hpp>

class IGuiElement;
class GuiBasicElement;

class GuiHelper {
public:
	template <class T>
	static std::shared_ptr<T> getTopElementAtPosition(std::vector<std::shared_ptr<T>> collection, float x, float y) {
		decltype(collection) vec;
		for (auto &e : collection) {
			glm::vec2 min, max;
			e->getBoundingBox(min, max);
			if (x >= min.x && y >= min.y && x <= max.x && y <= max.y)
				vec.push_back(e);
		}
		std::shared_ptr<T> top = nullptr;
		float topZ = 0.f;
		for (auto &e : vec)
			if (e->getZValue() >= topZ) {
				topZ = e->getZValue();
				top = e;
			}
		return top;
	}
};

#endif /* GUI_GUIHELPER_H_ */
