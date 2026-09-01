#ifndef TH_MOD_MODIFIERS_HARDROCK_HARD_ROCK_POLICY_HPP
#define TH_MOD_MODIFIERS_HARDROCK_HARD_ROCK_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace hardrock
{

enum { kRulesetVersion = 1 };

ThModDifficultyStateV1 GetState(const ThModRunConfigV1 &config,
                                bool runActive);

} // namespace hardrock
} // namespace th_mod

#endif
