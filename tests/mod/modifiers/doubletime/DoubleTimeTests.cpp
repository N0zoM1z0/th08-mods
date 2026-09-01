#include "modifiers/doubletime/DoubleTimeTests.hpp"

#include "ModApi.h"
#include "TestHarness.hpp"

namespace
{

ThModTimeScaleV1 ReadTimeScale()
{
    ThModTimeScaleV1 state;
    CHECK(th_mod_get_time_scale_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.struct_size == sizeof(state));
    return state;
}

void CheckInactiveTimeScale()
{
    const ThModTimeScaleV1 state = ReadTimeScale();
    CHECK(state.is_active == 0);
    CHECK(state.numerator == 1);
    CHECK(state.denominator == 1);
    CHECK(th_mod_next_simulation_tick_count_v1() == 1);
}

} // namespace

void TestDoubleTimePolicy()
{
    ResetRuntime();
    CHECK(th_mod_get_time_scale_v1(0) == TH_MOD_RESULT_NULL_ARGUMENT);
    CheckInactiveTimeScale();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_DOUBLE_TIME;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CheckInactiveTimeScale();

    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    const ThModTimeScaleV1 active = ReadTimeScale();
    CHECK(active.is_active == 1);
    CHECK(active.numerator == 3);
    CHECK(active.denominator == 2);

    const uint32_t expectedTicks[] = {1, 2, 1, 2, 1, 2, 1, 2};
    uint32_t totalTicks = 0;
    for (std::size_t index = 0;
         index < sizeof(expectedTicks) / sizeof(expectedTicks[0]); ++index)
    {
        const uint32_t ticks = th_mod_next_simulation_tick_count_v1();
        CHECK(ticks == expectedTicks[index]);
        totalTicks += ticks;
    }
    CHECK(totalTicks == 12);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    CheckInactiveTimeScale();
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_next_simulation_tick_count_v1() == 1);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);

    ResetRuntime();
}
