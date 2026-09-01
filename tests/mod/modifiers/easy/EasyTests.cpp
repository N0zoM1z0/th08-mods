#include "modifiers/easy/EasyTests.hpp"

#include "ModApi.h"
#include "TestHarness.hpp"

namespace
{

ThModDifficultyStateV1 ReadState()
{
    ThModDifficultyStateV1 state;
    CHECK(th_mod_get_difficulty_state_v1(&state) == TH_MOD_RESULT_OK);
    CHECK(state.struct_size == sizeof(state));
    CHECK(state.reserved[0] == 0);
    CHECK(state.reserved[1] == 0);
    CHECK(state.reserved[2] == 0);
    return state;
}

void CheckNeutral(const ThModDifficultyStateV1 &state)
{
    CHECK(state.is_active == 0);
    CHECK(state.projectile_speed_numerator == 1);
    CHECK(state.projectile_speed_denominator == 1);
    CHECK(state.player_speed_numerator == 1);
    CHECK(state.player_speed_denominator == 1);
    CHECK(state.player_hitbox_numerator == 1);
    CHECK(state.player_hitbox_denominator == 1);
    CHECK(state.player_graze_numerator == 1);
    CHECK(state.player_graze_denominator == 1);
    CHECK(state.starting_bomb_delta == 0);
    CHECK(state.spell_time_numerator == 1);
    CHECK(state.spell_time_denominator == 1);
}

} // namespace

void TestEasyPolicy()
{
    ResetRuntime();
    CheckNeutral(ReadState());

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_EASY;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CheckNeutral(ReadState());

    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    const ThModDifficultyStateV1 state = ReadState();
    CHECK(state.is_active == 1);
    CHECK(state.projectile_speed_numerator == 17);
    CHECK(state.projectile_speed_denominator == 20);
    CHECK(state.player_speed_numerator == 1);
    CHECK(state.player_speed_denominator == 1);
    CHECK(state.player_hitbox_numerator == 3);
    CHECK(state.player_hitbox_denominator == 4);
    CHECK(state.player_graze_numerator == 5);
    CHECK(state.player_graze_denominator == 4);
    CHECK(state.starting_bomb_delta == 2);
    CHECK(state.spell_time_numerator == 5);
    CHECK(state.spell_time_denominator == 4);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    CheckNeutral(ReadState());
    ResetRuntime();
}
