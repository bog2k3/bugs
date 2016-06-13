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

class Entity;

class EntityLabeler {
public:
	EntityLabeler();
	virtual ~EntityLabeler();

	// add or replace the named label on the entity
	void setEntityLabel(const Entity* ent, std::string const& name, std::string const& value, glm::vec3 rgb_);

private:
	struct EntLabel {
		std::string value_;
		glm::vec3 rgb_;
	};

	std::map<Entity*, std::map<std::string, EntLabel>> labels_;
};

#endif /* OSD_ENTITYLABELER_H_ */
