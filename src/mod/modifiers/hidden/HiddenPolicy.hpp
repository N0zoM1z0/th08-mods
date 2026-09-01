#ifndef TH_MOD_MODIFIERS_HIDDEN_HIDDEN_POLICY_HPP
#define TH_MOD_MODIFIERS_HIDDEN_HIDDEN_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace hidden
{

enum { kRulesetVersion = 1 };

uint32_t ComputeAlpha(const ThModRunConfigV1 &config, bool runActive,
                      uint32_t baseAlpha, uint32_t activeAgeTicks);

} // namespace hidden
} // namespace th_mod

#endif
