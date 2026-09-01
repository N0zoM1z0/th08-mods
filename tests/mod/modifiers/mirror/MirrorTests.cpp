#include "modifiers/mirror/MirrorTests.hpp"

#include "ModApi.h"
#include "TestHarness.hpp"
#include "modifiers/mirror/MirrorGeometry.hpp"

#include <cmath>

namespace
{

uint32_t FilterDirection(uint32_t mode, uint32_t action,
                         uint32_t context = TH_MOD_INPUT_CONTEXT_GAMEPLAY)
{
    ThModRunConfigV1 config;
    if (th_mod_is_run_active() != 0)
    {
        CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    }
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_MIRROR;
    config.mirror_mode = mode;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    return th_mod_filter_actions_v1(action, context);
}

void CheckCardinalMapping(uint32_t mode,
                          uint32_t up, uint32_t right,
                          uint32_t down, uint32_t left)
{
    CHECK(FilterDirection(mode, TH_MOD_ACTION_UP) == up);
    CHECK(FilterDirection(mode, TH_MOD_ACTION_RIGHT) == right);
    CHECK(FilterDirection(mode, TH_MOD_ACTION_DOWN) == down);
    CHECK(FilterDirection(mode, TH_MOD_ACTION_LEFT) == left);
}

void CheckPoint(const th_mod::mirror::Point &point, float x, float y)
{
    CHECK(std::fabs(point.x - x) < 0.001f);
    CHECK(std::fabs(point.y - y) < 0.001f);
}

void TestGeometry()
{
    const float left = 32.0f;
    const float top = 16.0f;
    const float width = 384.0f;
    const float height = 448.0f;
    CHECK(std::fabs(th_mod::mirror::QuarterTurnScale(width, height) -
                    6.0f / 7.0f) < 0.001f);
    CheckPoint(th_mod::mirror::TransformPoint(
                   TH_MOD_MIRROR_HORIZONTAL, left, top, width, height,
                   left, top),
               416.0f, 16.0f);
    CheckPoint(th_mod::mirror::TransformPoint(
                   TH_MOD_MIRROR_VERTICAL, left, top, width, height,
                   left, top),
               32.0f, 464.0f);
    CheckPoint(th_mod::mirror::TransformPoint(
                   TH_MOD_MIRROR_ROTATE_90, left, top, width, height,
                   left, top),
               416.0f, 75.42857f);
    CheckPoint(th_mod::mirror::TransformPoint(
                   TH_MOD_MIRROR_ROTATE_180, left, top, width, height,
                   left, top),
               416.0f, 464.0f);
    CheckPoint(th_mod::mirror::TransformPoint(
                   TH_MOD_MIRROR_ROTATE_270, left, top, width, height,
                   left, top),
               32.0f, 404.57143f);
    CheckPoint(th_mod::mirror::TransformPoint(
                   TH_MOD_MIRROR_ROTATE_90, left, top, width, height,
                   224.0f, 240.0f),
               224.0f, 240.0f);
}

} // namespace

void TestMirrorPolicy()
{
    ResetRuntime();
    TestGeometry();

    CheckCardinalMapping(TH_MOD_MIRROR_HORIZONTAL,
                         TH_MOD_ACTION_UP, TH_MOD_ACTION_LEFT,
                         TH_MOD_ACTION_DOWN, TH_MOD_ACTION_RIGHT);
    CheckCardinalMapping(TH_MOD_MIRROR_VERTICAL,
                         TH_MOD_ACTION_DOWN, TH_MOD_ACTION_RIGHT,
                         TH_MOD_ACTION_UP, TH_MOD_ACTION_LEFT);
    CheckCardinalMapping(TH_MOD_MIRROR_ROTATE_90,
                         TH_MOD_ACTION_LEFT, TH_MOD_ACTION_UP,
                         TH_MOD_ACTION_RIGHT, TH_MOD_ACTION_DOWN);
    CheckCardinalMapping(TH_MOD_MIRROR_ROTATE_180,
                         TH_MOD_ACTION_DOWN, TH_MOD_ACTION_LEFT,
                         TH_MOD_ACTION_UP, TH_MOD_ACTION_RIGHT);
    CheckCardinalMapping(TH_MOD_MIRROR_ROTATE_270,
                         TH_MOD_ACTION_RIGHT, TH_MOD_ACTION_DOWN,
                         TH_MOD_ACTION_LEFT, TH_MOD_ACTION_UP);

    const uint32_t diagonal = TH_MOD_ACTION_UP | TH_MOD_ACTION_LEFT;
    const uint32_t nonDirection = TH_MOD_ACTION_SHOOT | TH_MOD_ACTION_FOCUS;
    CHECK(FilterDirection(TH_MOD_MIRROR_ROTATE_90,
                          diagonal | nonDirection) ==
          (TH_MOD_ACTION_LEFT | TH_MOD_ACTION_DOWN | nonDirection));

    CHECK(FilterDirection(TH_MOD_MIRROR_HORIZONTAL, TH_MOD_ACTION_LEFT,
                          TH_MOD_INPUT_CONTEXT_GAMEPLAY |
                              TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK) ==
          TH_MOD_ACTION_LEFT);
    CHECK(FilterDirection(TH_MOD_MIRROR_HORIZONTAL, TH_MOD_ACTION_LEFT,
                          TH_MOD_INPUT_CONTEXT_GAMEPLAY |
                              TH_MOD_INPUT_CONTEXT_UI_BLOCKED) ==
          TH_MOD_ACTION_LEFT);

    ThModMirrorStateV1 state;
    CHECK(th_mod_get_mirror_state_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.struct_size == sizeof(state));
    CHECK(state.is_active == 1);
    CHECK(state.mode == TH_MOD_MIRROR_HORIZONTAL);
    CHECK(state.reserved == 0);
    CHECK(th_mod_get_mirror_state_v1(0) == TH_MOD_RESULT_NULL_ARGUMENT);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_get_mirror_state_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.is_active == 0);

    ResetRuntime();
}
