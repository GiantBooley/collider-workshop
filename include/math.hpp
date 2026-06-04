#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>

int realMod(int a, int b);

class Vec2f {
public:
	float x, y;
	Vec2f();
	Vec2f(float x_, float y_);

	static float distance(Vec2f p1, Vec2f p2);
	static float dot(Vec2f v1, Vec2f v2);
	static Vec2f lerp(Vec2f p1, Vec2f p2, float t);
};
float getAbsAngleFromThreePoints(Vec2f p1, Vec2f p2, Vec2f p3);
float distanceFromPointToLine(Vec2f point, Vec2f l1, Vec2f l2);
float getPointProgressAlongLine(Vec2f point, Vec2f l1, Vec2f l2);
bool lineIntersection(Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3, Vec2f* intersection);

class Vec3f {
public:
	float x, y, z;

	Vec3f();
	Vec3f(float x_, float y_, float z_);
};


class Rect {
public:
	float minX, minY, maxX, maxY;

	Rect(float minX_, float minY_, float maxX_, float maxY_);

	float getWidth() const;
	float getHeight() const;
};

class Vec2i {
public:
	int x, y;
	Vec2i();
	Vec2i(int x_, int y_);

	Vec2i operator+(const Vec2i& other) const;
	Vec2i operator-(const Vec2i& other) const;
	Vec2i operator+(const float& other) const;
	Vec2i operator-(const float& other) const;
};

#endif
