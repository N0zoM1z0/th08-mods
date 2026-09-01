#include "modifiers/blindspot/BlindSpotPolicy.hpp"

namespace th_mod
{
namespace blindspot
{

uint32_t ComputeAlpha(const ThModRunConfigV1 &config, bool runActive,
                      uint32_t baseAlpha, uint32_t distancePixels)
{
    const uint32_t clampedAlpha = baseAlpha > 255u ? 255u : baseAlpha;
    if (!runActive ||
        (config.enabled_mods & TH_MOD_BUILTIN_BLIND_SPOT) == 0)
    {
        return clampedAlpha;
    }
    if (distancePixels <= kInvisibleRadiusPixels)
    {
        return 0;
    }
    if (distancePixels >= kVisibleRadiusPixels)
    {
        return clampedAlpha;
    }

    const uint32_t fadeWidth =
        kVisibleRadiusPixels - kInvisibleRadiusPixels;
    return clampedAlpha * (distancePixels - kInvisibleRadiusPixels) /
           fadeWidth;
}

} // namespace blindspot
} // namespace th_mod
