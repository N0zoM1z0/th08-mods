#ifndef TH_MOD_MODIFIERS_NOFAIL_NO_FAIL_POLICY_HPP
#define TH_MOD_MODIFIERS_NOFAIL_NO_FAIL_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace nofail
{

enum { kRulesetVersion = 1 };

ThModNoFailDecisionV1 DecideMiss(const ThModRunConfigV1 &config,
                                 bool runActive,
                                 int32_t livesRemaining);

} // namespace nofail
} // namespace th_mod

#endif
