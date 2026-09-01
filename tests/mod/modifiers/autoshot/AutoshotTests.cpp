#include "modifiers/autoshot/AutoshotTests.hpp"

#include "TestHarness.hpp"

void TestAutoshotPolicy()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_AUTOSHOT;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    const uint32_t gameplay = TH_MOD_INPUT_CONTEXT_GAMEPLAY;
    CHECK(th_mod_filter_actions_v1(0, gameplay) == 0);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_filter_actions_v1(0, gameplay) == TH_MOD_ACTION_SHOOT);
    CHECK(th_mod_filter_actions_v1(TH_MOD_ACTION_BOMB, gameplay) ==
          (TH_MOD_ACTION_BOMB | TH_MOD_ACTION_SHOOT));
    CHECK(th_mod_filter_actions_v1(0, 0) == 0);
    CHECK(th_mod_filter_actions_v1(
              0, gameplay | TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK) == 0);
    CHECK(th_mod_filter_actions_v1(
              0, gameplay | TH_MOD_INPUT_CONTEXT_UI_BLOCKED) == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_filter_actions_v1(0, gameplay) == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
}
