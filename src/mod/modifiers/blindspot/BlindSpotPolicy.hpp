#ifndef TH_MOD_MODIFIERS_BLINDSPOT_BLIND_SPOT_POLICY_HPP
#define TH_MOD_MODIFIERS_BLINDSPOT_BLIND_SPOT_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace blindspot
{

enum
{
    kRulesetVersion = 1,
    kInvisibleRadiusPixels = 48,
    kVisibleRadiusPixels = 128
};

uint32_t ComputeAlpha(const ThModRunConfigV1 &config, bool runActive,
                      uint32_t baseAlpha, uint32_t distancePixels);

} // namespace blindspot
} // namespace th_mod

#endif
