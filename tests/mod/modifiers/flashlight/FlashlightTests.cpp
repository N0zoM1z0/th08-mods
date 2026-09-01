#include "modifiers/flashlight/FlashlightTests.hpp"

#include "TestHarness.hpp"

void TestFlashlightPolicy()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_FLASHLIGHT;
    config.flashlight_radius_pixels = 72;
    config.flashlight_opacity = 192;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    ThModFlashlightStateV1 state;
    CHECK(th_mod_get_flashlight_state_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.struct_size == sizeof(state));
    CHECK(state.is_active == 0);
    CHECK(state.radius_pixels == 72);
    CHECK(state.opacity == 192);

    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_get_flashlight_state_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.is_active == 1);
    CHECK(state.radius_pixels == 72);
    CHECK(state.opacity == 192);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_get_flashlight_state_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.is_active == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_get_flashlight_state_v1(0) == TH_MOD_RESULT_NULL_ARGUMENT);
}
