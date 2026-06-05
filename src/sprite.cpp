#include "sprite.hpp"

Sprite::Sprite(
	std::string objectPath_,
	std::string texturePath_,
	float pixelsPerUnit_,
	Rect rect_,
	Vec3f position_,
	Vec3f rotation_,
	Vec3f scale_,
	int sortingOrder_
) : objectPath(objectPath_), texturePath(texturePath_), pixelsPerUnit(pixelsPerUnit_), rect(rect_), position(position_), rotation(rotation_), scale(scale_), sortingOrder(sortingOrder_), colliders() {
	colliders.push_back({
		{
			Vec2f(-getWidth() / 2.f, -getHeight() / 2.f),
						Vec2f(-getWidth() / 2.f,  getHeight() / 2.f),
						Vec2f( getWidth() / 2.f,  getHeight() / 2.f),
						Vec2f( getWidth() / 2.f, -getHeight() / 2.f)
		}, // points
		"", // physicsMaterial2DAssetPath
		SoundMaterial::rock, // soundMaterial
		false, // isCustomHitSound
		"", // customHitSoundName
		128, 128, 128
	});
}

float Sprite::getWidth() const {
	return rect.getWidth() / pixelsPerUnit;
}
float Sprite::getHeight() const {
	return rect.getHeight() / pixelsPerUnit;
}
