#include "modifiers/hidden/HiddenPolicy.hpp"

namespace th_mod
{
namespace hidden
{

uint32_t ComputeAlpha(const ThModRunConfigV1 &config, bool runActive,
                      uint32_t baseAlpha, uint32_t activeAgeTicks)
{
    const uint32_t clampedAlpha = baseAlpha > 255u ? 255u : baseAlpha;
    if (!runActive || (config.enabled_mods & TH_MOD_BUILTIN_HIDDEN) == 0)
    {
        return clampedAlpha;
    }

    if (activeAgeTicks < config.hidden_visible_ticks)
    {
        return clampedAlpha;
    }

    const uint32_t fadeAge = activeAgeTicks - config.hidden_visible_ticks;
    if (fadeAge >= config.hidden_fade_ticks)
    {
        return 0;
    }

    const uint32_t remainingTicks = config.hidden_fade_ticks - fadeAge;
    return clampedAlpha * remainingTicks / config.hidden_fade_ticks;
}

} // namespace hidden
} // namespace th_mod
