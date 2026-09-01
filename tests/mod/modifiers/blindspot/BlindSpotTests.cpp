#include "modifiers/blindspot/BlindSpotTests.hpp"

#include "TestHarness.hpp"

void TestBlindSpotPolicy()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_BLIND_SPOT;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    CHECK(th_mod_blind_spot_alpha_v1(200, 0) == 200);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_blind_spot_alpha_v1(300, 200) == 255);
    CHECK(th_mod_blind_spot_alpha_v1(200, 200) == 200);
    CHECK(th_mod_blind_spot_alpha_v1(200, 128) == 200);
    CHECK(th_mod_blind_spot_alpha_v1(200, 88) == 100);
    CHECK(th_mod_blind_spot_alpha_v1(200, 49) == 2);
    CHECK(th_mod_blind_spot_alpha_v1(200, 48) == 0);
    CHECK(th_mod_blind_spot_alpha_v1(200, 0) == 0);
    CHECK(th_mod_blind_spot_alpha_v1(0, 88) == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_blind_spot_alpha_v1(200, 0) == 200);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
}
