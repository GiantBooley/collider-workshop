#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <cstdint>
#include <filesystem>
#include <memory>

#include "collider.hpp"
#include "ImageData.hpp"

class Sprite {
public:
    std::string objectPath;

    // texture
    std::filesystem::path texturePath;
    std::shared_ptr<IImageData> image;
    float pixelsPerUnit;
    Rect rect;

    // sprite position in world space
    Vec3f position;
    Vec3f rotation;
    Vec3f scale;
    int sortingOrder;

    std::vector<Collider> colliders;

    Sprite(
        std::string objectPath_,
        std::string texturePath_,
        float pixelsPerUnit_,
        Rect rect_,
        Vec3f position_,
        Vec3f rotation_,
        Vec3f scale_,
        int sortingOrder_
    );

    float getWidth() const;
    float getHeight() const;
};

#endif
