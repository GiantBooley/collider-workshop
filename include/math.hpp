#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>


class Vec2f {
public:
	float x, y;
	Vec2f();
	Vec2f(float x2, float y2);

	static float distance(Vec2f p1, Vec2f p2);
	static float dot(Vec2f v1, Vec2f v2);
	static Vec2f lerp(Vec2f p1, Vec2f p2, float t);
};
float getAbsAngleFromThreePoints(Vec2f p1, Vec2f p2, Vec2f p3);
float distanceFromPointToLine(Vec2f point, Vec2f l1, Vec2f l2);
float getPointProgressAlongLine(Vec2f point, Vec2f l1, Vec2f l2);
bool lineIntersection(Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3, Vec2f* intersection);

/*class Vec2i {
public:
	int x, y;
	Vec2i();
	Vec2i(int x2, int y2);
};*/

#endif
