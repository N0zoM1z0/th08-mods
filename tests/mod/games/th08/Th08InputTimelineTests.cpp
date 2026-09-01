#include "games/th08/Th08InputTimelineTests.hpp"

#include "TestHarness.hpp"
#include "mod/games/th08/Th08InputTimeline.hpp"

void TestIndependentInputTimelines()
{
    ResetRuntime();
    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_MIRROR;
    config.mirror_mode = TH_MOD_MIRROR_HORIZONTAL;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);

    const th08::mods::input::ActionTimeline firstFrame =
        th08::mods::input::AdvanceLiveInputTimeline(
            TH_MOD_ACTION_LEFT,
            TH_MOD_ACTION_UP | TH_MOD_ACTION_BOMB,
            TH_MOD_INPUT_CONTEXT_GAMEPLAY);
    CHECK(firstFrame.player_actions == TH_MOD_ACTION_LEFT);
    CHECK(firstFrame.recorder_actions ==
          (TH_MOD_ACTION_UP | TH_MOD_ACTION_BOMB));

    const th08::mods::input::ActionTimeline secondFrame =
        th08::mods::input::AdvanceLiveInputTimeline(
            TH_MOD_ACTION_LEFT,
            TH_MOD_ACTION_RIGHT,
            TH_MOD_INPUT_CONTEXT_GAMEPLAY);
    CHECK(secondFrame.player_actions == TH_MOD_ACTION_LEFT);
    CHECK(secondFrame.recorder_actions == TH_MOD_ACTION_LEFT);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    ResetRuntime();

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods =
        TH_MOD_BUILTIN_AUTOSHOT | TH_MOD_BUILTIN_MIRROR;
    config.mirror_mode = TH_MOD_MIRROR_HORIZONTAL;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);

    const th08::mods::input::ActionTimeline gameplay =
        th08::mods::input::AdvanceLiveInputTimeline(
            0,
            TH_MOD_ACTION_LEFT,
            TH_MOD_INPUT_CONTEXT_GAMEPLAY);
    CHECK(gameplay.player_actions == 0);
    CHECK(gameplay.recorder_actions ==
          (TH_MOD_ACTION_RIGHT | TH_MOD_ACTION_SHOOT));

    const th08::mods::input::ActionTimeline dialogue =
        th08::mods::input::AdvanceLiveInputTimeline(
            TH_MOD_ACTION_RIGHT,
            TH_MOD_ACTION_LEFT,
            TH_MOD_INPUT_CONTEXT_GAMEPLAY |
                TH_MOD_INPUT_CONTEXT_DIALOGUE);
    CHECK(dialogue.player_actions == TH_MOD_ACTION_RIGHT);
    CHECK(dialogue.recorder_actions == TH_MOD_ACTION_RIGHT);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    ResetRuntime();
}
