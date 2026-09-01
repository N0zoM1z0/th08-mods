#ifndef TH_MOD_MODIFIERS_FLASHLIGHT_FLASHLIGHT_POLICY_HPP
#define TH_MOD_MODIFIERS_FLASHLIGHT_FLASHLIGHT_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace flashlight
{

enum { kRulesetVersion = 1 };

ThModFlashlightStateV1 GetState(const ThModRunConfigV1 &config,
                                bool runActive);

} // namespace flashlight
} // namespace th_mod

#endif
