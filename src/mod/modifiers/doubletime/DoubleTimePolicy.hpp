#ifndef TH_MOD_MODIFIERS_DOUBLETIME_DOUBLE_TIME_POLICY_HPP
#define TH_MOD_MODIFIERS_DOUBLETIME_DOUBLE_TIME_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace doubletime
{

enum
{
    kRulesetVersion = 1,
    kRateNumerator = 3,
    kRateDenominator = 2,
};

ThModTimeScaleV1 GetTimeScale(const ThModRunConfigV1 &config,
                              bool runActive);
uint32_t AdvancePresentation(const ThModTimeScaleV1 &state,
                             uint32_t &accumulator);

} // namespace doubletime
} // namespace th_mod

#endif
