#include "collider.hpp"

Collider::Collider(
	std::vector<Vec2f> points_,
	SoundMaterial soundMaterial_,
	bool isCustomHitSound_,
	std::string customHitSoundName_,
	std::string physicsMaterial2DAssetPath_
) : points(points_), soundMaterial(soundMaterial_), isCustomHitSound(isCustomHitSound_), customHitSoundName(customHitSoundName_), physicsMaterial2DAssetPath(physicsMaterial2DAssetPath_), r(128), g(128), b(128) {}

std::vector<std::vector<Vec2f>> Collider::generateCollidersFromImage(const std::shared_ptr<IImageData>& image, Rect rect) {
	std::vector<std::vector<Vec2f>> paths;

	const float alphaThreshold = 0.5f;

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
							if (!image->checkBounds(x + mx, y + my)) {
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
					if (pointIsCorner) {
						paths.back().push_back(point);
					}
				}
			}
			if (exit) break;
		}
		if (exit) break;
	}
	return paths;
}
void Collider::simplifyPath(std::vector<Vec2f>& path) {

}
