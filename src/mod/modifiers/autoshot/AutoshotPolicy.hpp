#ifndef TH_MOD_MODIFIERS_AUTOSHOT_AUTOSHOT_POLICY_HPP
#define TH_MOD_MODIFIERS_AUTOSHOT_AUTOSHOT_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace autoshot
{

enum { kRulesetVersion = 1 };

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context);

} // namespace autoshot
} // namespace th_mod

#endif
