
#include "math.hpp"

int realMod(int a, int b) {
	if (a >= 0) return a % b; else return (b >= 0 ? b : -b) - 1 + (a + 1) % b;
}
float mapRange(float value, float oldMin, float oldMax, float newMin, float newMax) {
	float t = (value - oldMin) / (oldMax - oldMin);
	return (newMax - newMin) * t + newMin;
}

// Vec2f
Vec2f::Vec2f() : x(0.f), y(0.f) {}
Vec2f::Vec2f(float x_, float y_) : x(x_), y(y_) {}

float Vec2f::distance(Vec2f p1, Vec2f p2) {
	return std::sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}
float Vec2f::dot(Vec2f v1, Vec2f v2) {
	return v1.x * v2.x + v1.y * v2.y;
}
Vec2f Vec2f::lerp(Vec2f p1, Vec2f p2, float t) {
	return Vec2f(std::lerp(p1.x, p2.x, t), std::lerp(p1.y, p2.y, t));
}

Vec2f Vec2f::operator+(const Vec2f& other) const {
	return Vec2f(x + other.x, y + other.y);
}
Vec2f Vec2f::operator-(const Vec2f& other) const {
	return Vec2f(x - other.x, y - other.y);
}
Vec2f Vec2f::operator+(const float& other) const {
	return Vec2f(x + other, y + other);
}
Vec2f Vec2f::operator-(const float& other) const {
	return Vec2f(x - other, y - other);
}

float getAbsAngleFromThreePoints(Vec2f p1, Vec2f p2, Vec2f p3) {// p1 is vertex
	return std::abs(std::atan((p3.y - p1.y) / (p3.x - p1.x)) - std::atan((p2.y - p1.y) / (p2.x - p1.x)));
}
float distanceFromPointToLine(Vec2f point, Vec2f l1, Vec2f l2) {
	if (l1.x == l2.x) return std::abs(point.x - l1.x); // vertical line case
	float slope = (l2.y - l1.y) / (l2.x - l1.x);
	return std::abs(slope * (point.x - l1.x) - point.y + l1.y) / std::sqrt(slope * slope + 1.f);
}
float getPointProgressAlongLine(Vec2f point, Vec2f l1, Vec2f l2) {
	if (l1.x == l2.x) return (point.y - l1.y) / (l2.y - l1.y); // vertical line case
	float slope = (l2.y - l1.y) / (l2.x - l1.x);
	return ((point.x + slope * (point.y - l1.y + slope * l1.x)) / (slope * slope + 1.f) - l1.x) / (l2.x - l1.x);
}
bool lineIntersection(Vec2f p0, Vec2f p1, Vec2f p2, Vec2f p3, Vec2f* intersection) {
	Vec2f s1, s2;
	s1.x = p1.x - p0.x; s1.y = p1.y - p0.y;
	s2.x = p3.x - p2.x; s2.y = p3.y - p2.y;

	float s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x * s1.y + s1.x * s2.y);
	float t = ( s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x * s1.y + s1.x * s2.y);

	if (s >= 0.f && s <= 1.f && t >= 0.f && t <= 1.f) {
		intersection->x = p0.x + (t * s1.x);
		intersection->y = p0.y + (t * s1.y);
		return true;
	}

	return false;
}

// Vec3f
Vec3f::Vec3f() : x(0.f), y(0.f), z(0.f) {}
Vec3f::Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

// Rect
Rect::Rect(float minX_, float minY_, float maxX_, float maxY_) : minX(minX_), minY(minY_), maxX(maxX_), maxY(maxY_) {}

float Rect::getWidth() const {
	return maxX - minX;
}
float Rect::getHeight() const {
	return maxY - minY;
}

// Vec2i
Vec2i::Vec2i() : x(0), y(0) {}
Vec2i::Vec2i(int x_, int y_) : x(x_), y(y_) {}
Vec2i Vec2i::operator+(const Vec2i& other) const {
	return Vec2i(x + other.x, y + other.y);
}
Vec2i Vec2i::operator-(const Vec2i& other) const {
	return Vec2i(x - other.x, y - other.y);
}
Vec2i Vec2i::operator+(const int& other) const {
	return Vec2i(x + other, y + other);
}
Vec2i Vec2i::operator-(const int& other) const {
	return Vec2i(x - other, y - other);
}
