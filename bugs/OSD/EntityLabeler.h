/*
 * EntityLabeler.h
 *
 *  Created on: Jun 13, 2016
 *      Author: bog
 */

#ifndef OSD_ENTITYLABELER_H_
#define OSD_ENTITYLABELER_H_

#include <glm/vec3.hpp>
#include <map>
#include <memory>

class Entity;
class Label;
class RenderContext;

class EntityLabeler {
public:
	static EntityLabeler& getInstance();

	// add or replace the named label on the entity
	void setEntityLabel(const Entity* ent, std::string const& name, std::string const& value, glm::vec3 rgb);

	// remove a named label
	void removeEntityLabel(const Entity* ent, std::string const& name);

	void draw(RenderContext const&);

private:
	EntityLabeler();

	static constexpr int LABEL_TEXT_SIZE = 14;

	struct EntLabel {
		std::string value_;
		glm::vec3 rgb_;
		std::unique_ptr<Label> label_;
	};

	std::map<const Entity*, std::map<std::string, EntLabel>> labels_;
};

#endif /* OSD_ENTITYLABELER_H_ */
