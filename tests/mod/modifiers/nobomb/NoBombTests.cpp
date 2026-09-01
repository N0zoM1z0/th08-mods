#include "modifiers/nobomb/NoBombTests.hpp"

#include "TestHarness.hpp"

void TestNoBombPolicy()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_NO_BOMB;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    const uint32_t gameplay = TH_MOD_INPUT_CONTEXT_GAMEPLAY;
    const uint32_t actions = TH_MOD_ACTION_LEFT | TH_MOD_ACTION_SHOOT |
                             TH_MOD_ACTION_BOMB;
    const uint32_t withoutBomb = actions & ~TH_MOD_ACTION_BOMB;
    CHECK(th_mod_filter_actions_v1(actions, gameplay) == actions);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_filter_actions_v1(actions, gameplay) == withoutBomb);
    CHECK(th_mod_filter_actions_v1(
              actions, gameplay | TH_MOD_INPUT_CONTEXT_DIALOGUE) == actions);
    CHECK(th_mod_filter_actions_v1(
              actions, gameplay | TH_MOD_INPUT_CONTEXT_UI_BLOCKED) == actions);
    CHECK(th_mod_filter_actions_v1(
              actions, gameplay | TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK) ==
          actions);
    CHECK(th_mod_filter_actions_v1(actions, 0) == actions);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_RELAX | TH_MOD_BUILTIN_NO_BOMB;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_filter_actions_v1(TH_MOD_ACTION_BOMB, gameplay) ==
          (TH_MOD_ACTION_SHOOT | TH_MOD_ACTION_FOCUS));
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
}
