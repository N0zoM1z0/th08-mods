#ifndef TH_MOD_MODIFIERS_MIRROR_MIRROR_POLICY_HPP
#define TH_MOD_MODIFIERS_MIRROR_MIRROR_POLICY_HPP

#include "ModApi.h"

namespace th_mod
{
namespace mirror
{

enum { kRulesetVersion = 1 };

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context);
ThModMirrorStateV1 GetState(const ThModRunConfigV1 &config, bool runActive);
const char *ModeIdentifier(uint32_t mode);

} // namespace mirror
} // namespace th_mod

#endif
