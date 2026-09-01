#include "modifiers/hidden/HiddenTests.hpp"

#include "TestHarness.hpp"

void TestHiddenPolicy()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_HIDDEN;
    config.hidden_visible_ticks = 2;
    config.hidden_fade_ticks = 4;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    CHECK(th_mod_hidden_alpha_v1(200, 20) == 200);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_hidden_alpha_v1(300, 0) == 255);
    CHECK(th_mod_hidden_alpha_v1(200, 0) == 200);
    CHECK(th_mod_hidden_alpha_v1(200, 1) == 200);
    CHECK(th_mod_hidden_alpha_v1(200, 2) == 200);
    CHECK(th_mod_hidden_alpha_v1(200, 3) == 150);
    CHECK(th_mod_hidden_alpha_v1(200, 4) == 100);
    CHECK(th_mod_hidden_alpha_v1(200, 5) == 50);
    CHECK(th_mod_hidden_alpha_v1(200, 6) == 0);
    CHECK(th_mod_hidden_alpha_v1(0, 3) == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_hidden_alpha_v1(200, 1000) == 200);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
}
