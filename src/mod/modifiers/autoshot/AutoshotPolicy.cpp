#include "modifiers/autoshot/AutoshotPolicy.hpp"

#include "input/InputContext.hpp"

namespace th_mod
{
namespace autoshot
{

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context)
{
    if (runActive && input::CanTransformGameplayInput(context) &&
        (config.enabled_mods & TH_MOD_BUILTIN_AUTOSHOT) != 0)
    {
        actions |= TH_MOD_ACTION_SHOOT;
    }

    return actions;
}

} // namespace autoshot
} // namespace th_mod
