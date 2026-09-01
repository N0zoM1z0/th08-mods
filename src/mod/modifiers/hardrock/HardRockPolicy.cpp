#include "modifiers/hardrock/HardRockPolicy.hpp"

namespace th_mod
{
namespace hardrock
{

ThModDifficultyStateV1 GetState(const ThModRunConfigV1 &config,
                                bool runActive)
{
    ThModDifficultyStateV1 state = {};
    state.struct_size = sizeof(state);
    state.projectile_speed_numerator = 1;
    state.projectile_speed_denominator = 1;
    state.player_speed_numerator = 1;
    state.player_speed_denominator = 1;
    state.player_hitbox_numerator = 1;
    state.player_hitbox_denominator = 1;
    state.player_graze_numerator = 1;
    state.player_graze_denominator = 1;
    state.spell_time_numerator = 1;
    state.spell_time_denominator = 1;

    if (runActive &&
        (config.enabled_mods & TH_MOD_BUILTIN_HARD_ROCK) != 0)
    {
        state.is_active = 1;
        state.projectile_speed_numerator = 23;
        state.projectile_speed_denominator = 20;
        state.player_speed_numerator = 9;
        state.player_speed_denominator = 10;
        state.player_hitbox_numerator = 5;
        state.player_hitbox_denominator = 4;
        state.player_graze_numerator = 4;
        state.player_graze_denominator = 5;
    }
    return state;
}

} // namespace hardrock
} // namespace th_mod
