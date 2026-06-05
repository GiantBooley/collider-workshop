#include "collider.hpp"

#include <iostream>

std::string floatToString(float value) {
	std::string s = std::to_string(value);
	s.erase(s.find_last_not_of('0') + 1, std::string::npos);
	if (s.back() == '.') s.pop_back();
	return s;
}

PhysicsMaterial2D::PhysicsMaterial2D(float friction_, float bounciness_, std::string assetPath_) : friction(friction_), bounciness(bounciness_) {
	size_t slashPos = assetPath_.find_last_of('/');
	size_t dotPos = assetPath_.find_last_of('.');
	size_t substringStart = 0;
	size_t substringEnd = assetPath_.length();
	if (slashPos != std::string::npos) {
		substringStart = slashPos + 1;
	}
	if (dotPos != std::string::npos) {
		substringEnd = dotPos;
	}
	displayName = "(F:" + floatToString(friction) + " B:" + floatToString(bounciness) + ") " + assetPath_.substr(substringStart, substringEnd - substringStart);
}

Collider::Collider(
	std::vector<Vec2f> points_,
	std::string physicsMaterialAssetPath_,
	SoundMaterial soundMaterial_,
	bool isCustomHitSound_,
	std::string customHitSoundName_,
	uint8_t r_,
	uint8_t g_,
	uint8_t b_
) : points(points_), physicsMaterialAssetPath(physicsMaterialAssetPath_), soundMaterial(soundMaterial_), isCustomHitSound(isCustomHitSound_), customHitSoundName(customHitSoundName_), r(r_), g(g_), b(b_) {}

std::vector<std::vector<Vec2f>> Collider::generateCollidersFromImage(const std::shared_ptr<IImageData>& image, Rect rect, float alphaThreshold) {
	std::vector<std::vector<Vec2f>> paths;

	bool exit = false;
	// loop over every pixel, starting from top left
	for (int y = rect.minY; y < rect.maxY; y++) {
		for (int x = rect.minX; x < rect.maxX; x++) {
			// if pixel is opaque then start collider generation
			float alpha;
			image->getNormalizedPixelA(x, y, &alpha);
			if (alpha > alphaThreshold) {
				paths.push_back({});
				exit = true;
				x--;
				Vec2i startPos{x, y}; // used to check if loop is completed

				Side side = Side::right;
				bool matrix[9]; // 0,0: top left

				// move clockwise around image (+y down)
				for (int i = 0; i < 100000; i++) {
					if (i != 0 && x == startPos.x && y == startPos.y && side == Side::right) break; // when full loop is completed, exit

					// get surroundings of current spot
					for (int my = -1; my <= 1; my++) {
						for (int mx = -1; mx <= 1; mx++) {
							if ( // check if pixel is in bounds
								!image->checkBounds(x + mx, y + my) ||
								static_cast<float>(x + mx) < rect.minX ||
								static_cast<float>(x + mx) >= rect.maxX ||
								static_cast<float>(y + my) < rect.minY ||
								static_cast<float>(y + my) >= rect.maxY
							) {
								matrix[(my + 1) * 3 + (mx + 1)] = false;
								continue;
							}
							float checkAlpha;
							image->getNormalizedPixelA(x + mx, y + my, &checkAlpha);
							matrix[(my + 1) * 3 + (mx + 1)] = checkAlpha > alphaThreshold;
						}
					}

					Vec2f point; // set point to most clockwise point of edge
					bool pointIsCorner = false;
					switch (side) {
					case Side::top: { // checks: top left, middle left
						point = Vec2f((float)x, (float)y);
						if (!matrix[0*3+0] && !matrix[1*3+0]) { // top left empty, middle left empty
							x--;
							y--;
							side = Side::right;
							pointIsCorner = true;
						}
						if (matrix[0*3+0] && !matrix[1*3+0]) { // top left full, middle left empty
							x--;
						}
						if (matrix[1*3+0]) { // middle left full
							side = Side::left;
							pointIsCorner = true;
						}
						break;
					}
					case Side::right: { // checks: top right, top middle
						point = Vec2f((float)x + 1.f, (float)y);
						if (!matrix[0*3+2] && !matrix[0*3+1]) { // top right empty, top middle empty
							x++;
							y--;
							side = Side::bottom;
							pointIsCorner = true;
						}
						if (matrix[0*3+2] && !matrix[0*3+1]) { // top right full, top middle empty
							y--;
						}
						if (matrix[0*3+1]) { // top middle full
							side = Side::top;
							pointIsCorner = true;
						}
						break;
					}
					case Side::bottom: { // checks: bottom right, middle right
						point = Vec2f((float)x + 1.f, (float)y + 1.f);
						if (!matrix[2*3+2] && !matrix[1*3+2]) { // bottom right empty, middle right empty
							x++;
							y++;
							side = Side::left;
							pointIsCorner = true;
						}
						if (matrix[2*3+2] && !matrix[1*3+2]) { // bottom right full, middle right empty
							x++;
						}
						if (matrix[1*3+2]) { // middle right full
							side = Side::right;
							pointIsCorner = true;
						}
						break;
					}
					case Side::left: { // checks: bottom left, bottom middle
						point = Vec2f((float)x, (float)y + 1.f);
						if (!matrix[2*3+0] && !matrix[2*3+1]) { // bottom left empty, bottom middle empty
							x--;
							y++;
							side = Side::top;
							pointIsCorner = true;
						}
						if (matrix[2*3+0] && !matrix[2*3+1]) { // bottom left full, bottom middle empty
							y++;
						}
						if (matrix[2*3+1]) { // bottom middle full
							side = Side::bottom;
							pointIsCorner = true;
						}
						break;
					}
					}
					if (!pointIsCorner || pointIsCorner) {
						paths.back().push_back(Vec2f(point.x - rect.minX, point.y - rect.minY));
					}
				}
			}
			if (exit) break;
		}
		if (exit) break;
	}
	return paths;
}
void Collider::simplifyPath(std::vector<Vec2f>& path, float douglasPeuckerEpsilon, int smoothSteps) {
	//const float minAngle = 0.01f;

	// change phase by 0.5
	for (int i = 0; i < smoothSteps; i++) {
		blurPath(path);
	}
	path = douglasPeucker(path, douglasPeuckerEpsilon);

	// planar simplification
	/*pointsRemoved = 0;
	std::vector<Vec2f> newSimplifiedPath;
	for (int i = 0; i < path.size(); i++) {
		Vec2f previous = path[realMod(i - 1, path.size())];
		Vec2f current = path[i];
		Vec2f next = path[realMod(i + 1, path.size())];
		float angle = getAbsAngleFromThreePoints(current, previous, next);
		if (angle >= minAngle) {
			newSimplifiedPath.push_back(current);
		} else {
			pointsRemoved++;
		}
	}
	path = newSimplifiedPath;
	std::cout << "points removed: " << pointsRemoved << std::endl;*/
}

void Collider::blurPath(std::vector<Vec2f>& path) {
	int length = path.size();
	Vec2f firstPoint{};
	if (length > 0) firstPoint = path[0];
	for (int i = 0; i < length; i++) {
		Vec2f current = path[i];
		Vec2f next = (i == length - 1) ? firstPoint : path[i + 1];
		path[i] = Vec2f((current.x + next.x) * 0.5f, (current.y + next.y) * 0.5f);
	}
}
std::vector<Vec2f> Collider::douglasPeucker(const std::vector<Vec2f>& path, float epsilon) {
	float dmax = 0.f;
	int index = -1;
	int end = path.size() - 1;
	for (int i = 1; i < end; i++) {
		float d = distanceFromPointToLine(path[i], path[0], path[end]);
		if (d > dmax) {
			index = i;
			dmax = d;
		}
	}

	std::vector<Vec2f> resultPath;

	if (dmax > epsilon) {
		std::vector<Vec2f>recResults1 = douglasPeucker({path.begin()        , path.begin() + index + 1}, epsilon); // include 0 and index
		std::vector<Vec2f>recResults2 = douglasPeucker({path.begin() + index, path.end()              }, epsilon); // include index and end

		resultPath.insert(resultPath.end(), recResults1.begin(), recResults1.end() - 1);
		resultPath.insert(resultPath.end(), recResults2.begin(), recResults2.end());
	} else {
		resultPath = {path[0], path[end]};
	}

	return resultPath;
}
