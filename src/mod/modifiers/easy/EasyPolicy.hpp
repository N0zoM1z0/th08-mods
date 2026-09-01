#ifndef TH_MOD_MODIFIERS_EASY_EASY_POLICY_HPP
#define TH_MOD_MODIFIERS_EASY_EASY_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace easy
{

enum { kRulesetVersion = 1 };

ThModDifficultyStateV1 GetState(const ThModRunConfigV1 &config,
                                bool runActive);

} // namespace easy
} // namespace th_mod

#endif
