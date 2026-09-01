#include "modifiers/doubletime/DoubleTimePolicy.hpp"

namespace th_mod
{
namespace doubletime
{

ThModTimeScaleV1 GetTimeScale(const ThModRunConfigV1 &config,
                              bool runActive)
{
    ThModTimeScaleV1 state = {};
    state.struct_size = sizeof(state);
    state.numerator = 1;
    state.denominator = 1;

    if (runActive &&
        (config.enabled_mods & TH_MOD_BUILTIN_DOUBLE_TIME) != 0)
    {
        state.is_active = 1;
        state.numerator = kRateNumerator;
        state.denominator = kRateDenominator;
    }
    return state;
}

uint32_t AdvancePresentation(const ThModTimeScaleV1 &state,
                             uint32_t &accumulator)
{
    if (!state.is_active || state.denominator == 0)
    {
        accumulator = 0;
        return 1;
    }

    accumulator += state.numerator;
    const uint32_t ticks = accumulator / state.denominator;
    accumulator %= state.denominator;
    return ticks;
}

} // namespace doubletime
} // namespace th_mod
