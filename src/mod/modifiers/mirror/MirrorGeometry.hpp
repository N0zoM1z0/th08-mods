#ifndef TH_MOD_MODIFIERS_MIRROR_MIRROR_GEOMETRY_HPP
#define TH_MOD_MODIFIERS_MIRROR_MIRROR_GEOMETRY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace mirror
{

struct Point
{
    float x;
    float y;
};

inline float QuarterTurnScale(float width, float height)
{
    if (width <= 0.0f || height <= 0.0f)
    {
        return 1.0f;
    }
    const float widthRatio = width / height;
    const float heightRatio = height / width;
    return widthRatio < heightRatio ? widthRatio : heightRatio;
}

inline Point TransformPoint(uint32_t mode, float left, float top,
                            float width, float height, float x, float y)
{
    const float centerX = left + width * 0.5f;
    const float centerY = top + height * 0.5f;
    const float offsetX = x - centerX;
    const float offsetY = y - centerY;
    Point transformed = {x, y};

    switch (mode)
    {
    case TH_MOD_MIRROR_HORIZONTAL:
        transformed.x = centerX - offsetX;
        break;
    case TH_MOD_MIRROR_VERTICAL:
        transformed.y = centerY - offsetY;
        break;
    case TH_MOD_MIRROR_ROTATE_90:
    {
        const float scale = QuarterTurnScale(width, height);
        transformed.x = centerX - offsetY * scale;
        transformed.y = centerY + offsetX * scale;
        break;
    }
    case TH_MOD_MIRROR_ROTATE_180:
        transformed.x = centerX - offsetX;
        transformed.y = centerY - offsetY;
        break;
    case TH_MOD_MIRROR_ROTATE_270:
    {
        const float scale = QuarterTurnScale(width, height);
        transformed.x = centerX + offsetY * scale;
        transformed.y = centerY - offsetX * scale;
        break;
    }
    }
    return transformed;
}

} // namespace mirror
} // namespace th_mod

#endif
