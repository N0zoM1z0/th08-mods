#include "mod/modifiers/doubletime/th08/Th08DoubleTime.hpp"

#include "GameManager.hpp"
#include "mod/api/ModApi.h"
#ifdef TH08_MODERN_LINUX
#include "modern/linux/audio_playback_rate.hpp"
#endif

namespace th08
{
namespace mods
{
namespace doubletime
{
namespace
{

u32 g_appliedAudioNumerator = 1;
u32 g_appliedAudioDenominator = 1;

void ApplyAudioRate(u32 numerator, u32 denominator)
{
    if (numerator == 0 || denominator == 0)
    {
        numerator = 1;
        denominator = 1;
    }
    if (numerator == g_appliedAudioNumerator &&
        denominator == g_appliedAudioDenominator)
    {
        return;
    }

#ifdef TH08_MODERN_LINUX
    modern::audio::SetPlaybackRate(numerator, denominator);
#endif
    g_appliedAudioNumerator = numerator;
    g_appliedAudioDenominator = denominator;
}

} // namespace

u32 SimulationTicksForPresentation()
{
    if (!g_GameManager.flags.unk2)
    {
        ResetPresentationState();
        return 1;
    }

    ThModTimeScaleV1 timeScale;
    if (th_mod_get_time_scale_v1(&timeScale) != TH_MOD_RESULT_OK ||
        !timeScale.is_active)
    {
        ApplyAudioRate(1, 1);
    }
    else
    {
        ApplyAudioRate(timeScale.numerator, timeScale.denominator);
    }

    return th_mod_next_simulation_tick_count_v1();
}

void ResetPresentationState()
{
    th_mod_reset_simulation_clock_v1();
    ApplyAudioRate(1, 1);
}

} // namespace doubletime
} // namespace mods
} // namespace th08
