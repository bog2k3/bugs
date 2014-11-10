#pragma once

#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

#define EPS 1.e-30f
#define PI 3.1415926535897932384626433832795f

template<typename T> inline T sqr(T x) { return x*x; }
template<typename T> inline void xchg(T &x1, T &x2) { T aux = x1; x1 = x2; x2 = aux; }

inline glm::vec2 getNormalVector(glm::vec2 v) { return glm::vec2(-v.y, v.x); }

struct Circle
{
	glm::vec2 vCenter;
	float Radius;

	inline Circle(): vCenter(0), Radius(0) {}
	inline Circle(glm::vec2 vCenter, float Radius) : vCenter(vCenter), Radius(Radius) {}
	inline Circle(float x, float y, float Radius) : vCenter(x,y), Radius(Radius) {}

	inline bool containsPoint(glm::vec2 const &vPoint) const
	{
		return (vPoint - vCenter).length() <= Radius;
	}

	inline bool intersectsCircle(Circle const &other) const
	{
		return (other.vCenter - vCenter).length() <= Radius + other.Radius;
	}
};

/// represents an arbitrary axis that separates the world into two sub-spaces (+ and -)
/// the axis is always normalized
struct Axis
{
	/* the axis equation coefficients;
	 * the axis equation is:
	 *	a*x + b*y + c = N;
	 *	if N == 0, the point (x,y) is on the axis;
	 *	if N < 0, the point is on the negative side of the axis;
	 *	if N > 0, the point is on the positive side of the axis;
	 *	in all cases, N is the distance from the point to the axis
	 */
	float a, b, c;

	inline Axis(float a, float b, float c): a(a), b(b), c(c) { assert(a!=0 || b!=0);} // prevent degenerate axes
	inline Axis(const Axis &ax): a(ax.a), b(ax.b), c(ax.c) {}
	inline static Axis fromPoints(glm::vec2 const &point1, glm::vec2 const &point2)
	{
		return fromDirectionAndPoint(point2-point1, point1);
	}
	inline static Axis fromDirectionAndPoint(glm::vec2 direction, glm::vec2 const &point)
	{
		glm::normalize(direction);
		return Axis(-direction.y, direction.x, direction.y*point.x - direction.x*point.y);
	}

	inline glm::vec2 getNormalVector() const  { return glm::vec2(a, b); }
	inline glm::vec2 getDirectionVector() const { return glm::vec2(b, -a); }

	inline float probePoint(glm::vec2 const &v) const { return probePoint(v.x, v.y); }
	inline float probePoint(float x, float y) const { return a*x + b*y + c; }

	// returns true if axes intersect and fills the pPoint with the point of intersection;
	// returns false if axes are parallel.
	inline bool intersectAxis(Axis const &other, glm::vec2 *pOutPoint) const
	{
		float div = a*other.b - other.a*b;
		if (div == 0)
			return false;
		pOutPoint->x = (b*other.c - other.b*c) / div;
		pOutPoint->y = (-c - a*pOutPoint->x) / b;
		return true;
	}

	// converts an axis-relative coordinate into a point in world-space
	inline glm::vec2 coordToPoint(float coord) const
	{
		float x = (b*coord - a*c) / (sqr(a)+sqr(b));
		float y = (-c -a*x) / b;
		return glm::vec2(x,y);
	}

	// returns the point's coordinate along the axis (a point has zero coordinate if it lies
	// on the line perpedicular to the axis that passes through the origin)
	inline float getCoord(glm::vec2 const &point) const
	{
		return glm::dot(point, getDirectionVector());
	}

	inline bool intersectsCircle(Circle const &circ) const { return abs(a*circ.vCenter.x + b*circ.vCenter.y + c) <= circ.Radius; }
};

/// represents an axis aligned box
struct AlignedBox
{
	glm::vec2 bottomLeft;
	glm::vec2 topRight;

	inline AlignedBox(): bottomLeft(), topRight() { }
	inline AlignedBox(glm::vec2& vBottomLeft, glm::vec2& vTopRight): bottomLeft(vBottomLeft), topRight(vTopRight) { }
	inline AlignedBox(float x1, float y1, float x2, float y2): bottomLeft(x1,y1), topRight(x2,y2) { }
	inline AlignedBox(AlignedBox const& orig): bottomLeft(orig.bottomLeft), topRight(orig.topRight) { }

	inline bool intersectsBox(AlignedBox const &other) const { 
		return !(
			   bottomLeft.y > other.topRight.y
			|| topRight.y < other.bottomLeft.y
			|| bottomLeft.x > other.topRight.x
			|| topRight.x < other.bottomLeft.x
		); 
	}

	inline bool containsPoint(glm::vec2 const &v2) const {
		return v2.x >= bottomLeft.x && v2.x <= topRight.x
			&& v2.y >= bottomLeft.y && v2.y <= topRight.y;
	}

	bool intersectsCircle(Circle const &circle) const;
};

/// represents an arbitrary box which does not need to be aligned to the world axis.
struct ArbitraryBox
{
	~ArbitraryBox();

	// promotes an AlignedBox to an ArbitraryBox
	static ArbitraryBox fromAlignedBox(AlignedBox const &box);

	// creates an ArbitraryBox from a direction (representing the bottom axis direction),
	// a point (representing the bottom-left corner of the box) and the size of the box along the
	// direction (x) and normal to that direction (y)
	static ArbitraryBox fromDirectionPointAndSize(glm::vec2 direction, glm::vec2 const &point, glm::vec2 const &size);

	// creates an empty box (size near 0) located at the specified position
	static ArbitraryBox empty(glm::vec2 const &position);

	bool intersectsBox(ArbitraryBox const &other) const;

	// use the following indexes to refer to an individual axis:
	static const int AXIS_BOTTOM;
	static const int AXIS_RIGHT;
	static const int AXIS_TOP;
	static const int AXIS_LEFT;

protected:
	// the positive subspace of each axis is towards the interior of the box; 
	// therefore a point that is in the positive subspace of all axes, is inside the box.
	Axis* axes[4];

	glm::vec2 vertices[4]; // counter-clockwise, starting at bottom-left

	ArbitraryBox(glm::vec2 vertices[4]);
};
