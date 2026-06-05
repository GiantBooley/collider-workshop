#ifndef COLLIDER_HPP
#define COLLIDER_HPP

#include <vector>
#include <string>

#include "math.hpp"
#include "IImageData.hpp"

enum class SoundMaterial {
	rock,
	wood,
	metal,
	plastic,
	furniture,
	snow,
	cardboard,
	none,
	snake,
	solidmetal
};
inline const std::string soundMaterialNames[10] = {
	"Rock",
	"Wood",
	"Metal",
	"Plastic",
	"Furniture",
	"Snow",
	"Cardboard",
	"None",
	"Snake",
	"SolidMetal"
};
class PhysicsMaterial2D {
public:
	float friction;
	float bounciness;
	std::string displayName;

	PhysicsMaterial2D(float friction_, float bounciness_, std::string assetPath_);
};

class Collider {
public:
	std::vector<Vec2f> points;
	std::string physicsMaterialAssetPath;
	SoundMaterial soundMaterial;
	bool isCustomHitSound;
	std::string customHitSoundName;
	uint8_t r, g, b;

	Collider(
		std::vector<Vec2f> points_,
		std::string physicsMaterialAssetPath_,
		SoundMaterial soundMaterial_,
		bool isCustomHitSound_,
		std::string customHitSoundName_,
		uint8_t r_,
		uint8_t g_,
		uint8_t b_
	);


	enum class Side {
		top, right, bottom, left
	};

	static std::vector<std::vector<Vec2f>> generateCollidersFromImage(const std::shared_ptr<IImageData>& image, Rect rect, float alphaThreshold);
	static void simplifyPath(std::vector<Vec2f>& path, float douglasPeuckerEpsilon, int smoothSteps);
	static void blurPath(std::vector<Vec2f>& path);
	static std::vector<Vec2f> douglasPeucker(const std::vector<Vec2f>& path, float epsilon);
};


#endif
