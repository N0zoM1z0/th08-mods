#include "modifiers/relax/RelaxPolicy.hpp"

#include "input/InputContext.hpp"

namespace th_mod
{
namespace relax
{

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context)
{
    if (runActive && input::CanInjectGameplayAction(context) &&
        (config.enabled_mods & TH_MOD_BUILTIN_RELAX) != 0)
    {
        actions |= TH_MOD_ACTION_SHOOT | TH_MOD_ACTION_FOCUS;
    }

    return actions;
}

} // namespace relax
} // namespace th_mod
