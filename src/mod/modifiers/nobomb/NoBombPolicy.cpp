#include "modifiers/nobomb/NoBombPolicy.hpp"

#include "input/InputContext.hpp"

namespace th_mod
{
namespace nobomb
{

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context)
{
    if (runActive && input::CanModifyGameplayActions(context) &&
        (config.enabled_mods & TH_MOD_BUILTIN_NO_BOMB) != 0)
    {
        actions &= ~static_cast<uint32_t>(TH_MOD_ACTION_BOMB);
    }
    return actions;
}

} // namespace nobomb
} // namespace th_mod
