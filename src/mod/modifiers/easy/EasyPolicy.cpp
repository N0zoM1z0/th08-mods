#include "modifiers/easy/EasyPolicy.hpp"

namespace th_mod
{
namespace easy
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

    if (runActive && (config.enabled_mods & TH_MOD_BUILTIN_EASY) != 0)
    {
        state.is_active = 1;
        state.projectile_speed_numerator = 17;
        state.projectile_speed_denominator = 20;
        state.player_hitbox_numerator = 3;
        state.player_hitbox_denominator = 4;
        state.player_graze_numerator = 5;
        state.player_graze_denominator = 4;
        state.starting_bomb_delta = 2;
        state.spell_time_numerator = 5;
        state.spell_time_denominator = 4;
    }
    return state;
}

} // namespace easy
} // namespace th_mod
