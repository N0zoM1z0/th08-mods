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

    const th08::mods::input::ActionTimeline filtered =
        th08::mods::input::FilterActionTimeline(
            TH_MOD_ACTION_LEFT,
            TH_MOD_ACTION_UP | TH_MOD_ACTION_BOMB,
            TH_MOD_INPUT_CONTEXT_GAMEPLAY);
    CHECK(filtered.gui_actions == TH_MOD_ACTION_RIGHT);
    CHECK(filtered.frame_actions ==
          (TH_MOD_ACTION_UP | TH_MOD_ACTION_BOMB));

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    ResetRuntime();
}
