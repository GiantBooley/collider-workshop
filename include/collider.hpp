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

class Collider {
public:
    std::vector<Vec2f> points;
    SoundMaterial soundMaterial;
    bool isCustomHitSound;
    std::string customHitSoundName;
    std::string physicsMaterial2DAssetPath;
    unsigned char r, g, b;

    Collider(
        std::vector<Vec2f> points_,
        SoundMaterial soundMaterial_,
        bool isCustomHitSound_,
        std::string customHitSoundName_,
        std::string physicsMaterial2DAssetPath_
    );


    enum class Side {
        top, right, bottom, left
    };

    static std::vector<std::vector<Vec2f>> generateCollidersFromImage(const std::shared_ptr<IImageData>& image, Rect rect);
    static void simplifyPath(std::vector<Vec2f>& path);
};


#endif
