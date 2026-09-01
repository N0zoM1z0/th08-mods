#include "modifiers/autoshot/AutoshotPolicy.hpp"

namespace th_mod
{
namespace autoshot
{

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context)
{
    const bool activeGameplay =
        (context & TH_MOD_INPUT_CONTEXT_GAMEPLAY) != 0;
    const bool playback =
        (context & TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK) != 0;
    const bool uiBlocked =
        (context & TH_MOD_INPUT_CONTEXT_UI_BLOCKED) != 0;

    if (runActive && activeGameplay && !playback && !uiBlocked &&
        (config.enabled_mods & TH_MOD_BUILTIN_AUTOSHOT) != 0)
    {
        actions |= TH_MOD_ACTION_SHOOT;
    }

    return actions;
}

} // namespace autoshot
} // namespace th_mod
