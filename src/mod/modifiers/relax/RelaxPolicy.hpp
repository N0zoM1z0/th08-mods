#ifndef TH_MOD_MODIFIERS_RELAX_RELAX_POLICY_HPP
#define TH_MOD_MODIFIERS_RELAX_RELAX_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace relax
{

enum { kRulesetVersion = 1 };

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context);

} // namespace relax
} // namespace th_mod

#endif
