#include "modifiers/relax/RelaxTests.hpp"

#include "TestHarness.hpp"

void TestRelaxPolicy()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_RELAX;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    const uint32_t gameplay = TH_MOD_INPUT_CONTEXT_GAMEPLAY;
    const uint32_t assisted = TH_MOD_ACTION_SHOOT | TH_MOD_ACTION_FOCUS;
    CHECK(th_mod_filter_actions_v1(0, gameplay) == 0);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_filter_actions_v1(0, gameplay) == assisted);
    CHECK(th_mod_filter_actions_v1(TH_MOD_ACTION_BOMB, gameplay) ==
          (TH_MOD_ACTION_BOMB | assisted));
    CHECK(th_mod_filter_actions_v1(TH_MOD_ACTION_LEFT, gameplay) ==
          (TH_MOD_ACTION_LEFT | assisted));
    CHECK(th_mod_filter_actions_v1(0, 0) == 0);
    CHECK(th_mod_filter_actions_v1(
              0, gameplay | TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK) == 0);
    CHECK(th_mod_filter_actions_v1(
              0, gameplay | TH_MOD_INPUT_CONTEXT_UI_BLOCKED) == 0);
    CHECK(th_mod_filter_actions_v1(
              0, gameplay | TH_MOD_INPUT_CONTEXT_DIALOGUE) == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
}
