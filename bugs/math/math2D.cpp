#include <algorithm>
#include "math2D.h"

bool AlignedBox::intersectsCircle(Circle const &circle) const
{
	bool circleBetweenUpDown = 
		circle.vCenter.y >= bottomLeft.y 
		&& circle.vCenter.y <= topRight.y;
	bool circleBetweenLeftRight = 
		circle.vCenter.x >= bottomLeft.x 
		&& circle.vCenter.x <= topRight.x;

	if (circleBetweenLeftRight) {
		if (circleBetweenUpDown)
			return true;
		else return abs(circle.vCenter.y - bottomLeft.y) < circle.Radius
			|| abs(circle.vCenter.y - topRight.y) < circle.Radius;
	} else if (circleBetweenUpDown)
		return abs(circle.vCenter.x-bottomLeft.x) < circle.Radius
		|| abs(circle.vCenter.x - topRight.x) < circle.Radius;
	else {
		// circle is not between any sides of the box, need to check
		// distance to box corners
		double cornerX, cornerY;
		cornerX = (circle.vCenter.x < bottomLeft.x)
			? bottomLeft.x : topRight.x;
		cornerY = (circle.vCenter.y < bottomLeft.y)
			? bottomLeft.y : topRight.y;
		return glm::distance(glm::vec2(cornerX,cornerY), circle.vCenter) <= circle.Radius;
	}
}

bool ArbitraryBox::intersectsBox(ArbitraryBox const &other) const
{
	// if one box's vertices are ALL on the negative side of at least one axis from the other box,
	// then the boxes don't intersect. Otherwise they do.
	const ArbitraryBox* boxes[2] = {this, &other};
	bool axis_found=false;
	for (int iBox=0; iBox<2; ++iBox) {
		// search axes from current box (iBox)
		for (int iAxis=0; iAxis<4; ++iAxis) {
			bool vertexInPositiveSide = false;
			// loop through other box's (1-iBox) vertices
			for (int iVertex=0; iVertex<4; ++iVertex)
				if (boxes[iBox]->axes[iAxis]->probePoint(boxes[1-iBox]->vertices[iVertex]) >= 0) {
					vertexInPositiveSide = true;
					break;
				}
			if (!vertexInPositiveSide) {
				axis_found = true;
				break;
			}
		}
		if (axis_found)
			break;
	}
	return !axis_found;
}

ArbitraryBox ArbitraryBox::empty(glm::vec2 const &position)
{
	glm::vec2 verts[4] = {
		position+glm::vec2(-1e-30, -1e-30),
		position+glm::vec2(+1e-30, -1e-30),
		position+glm::vec2(+1e-30, +1e-30),
		position+glm::vec2(-1e-30, +1e-30),
	};
	return ArbitraryBox(verts);
}

ArbitraryBox::ArbitraryBox(glm::vec2 vertices[4])
{
	std::copy(&vertices[0], &vertices[3], this->vertices);
	axes[0] = new Axis(Axis::fromPoints(vertices[0], vertices[1]));
	axes[1] = new Axis(Axis::fromPoints(vertices[1], vertices[2]));
	axes[2] = new Axis(Axis::fromPoints(vertices[2], vertices[3]));
	axes[3] = new Axis(Axis::fromPoints(vertices[3], vertices[0]));
}

ArbitraryBox ArbitraryBox::fromAlignedBox(AlignedBox const &box, float rotation)
{
	//TODO must fix this to take rotation into account
	glm::vec2 verts[4] = {
		box.bottomLeft,
		glm::vec2(box.topRight.x, box.bottomLeft.y),
		box.topRight,
		glm::vec2(box.bottomLeft.x, box.topRight.y),
	};
	return ArbitraryBox(verts);
}

ArbitraryBox ArbitraryBox::fromDirectionPointAndSize(glm::vec2 direction, glm::vec2 const &point, glm::vec2 const &size)
{
	assert(direction.x != 0 || direction.y != 0); // don't allow degenerate boxes

	direction = glm::normalize(direction);
	glm::vec2 verts[4] = {
		point,
		point + direction*size.x,
		point + direction*size.x + getNormalVector(direction)*size.y,
		point + getNormalVector(direction)*size.y,
	};
	return ArbitraryBox(verts);
}

ArbitraryBox::~ArbitraryBox()
{
	for (int i=0; i<4; ++i)
		delete axes[i];
}
