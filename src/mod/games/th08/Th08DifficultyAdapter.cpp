#include "mod/games/th08/Th08DifficultyAdapter.hpp"

#include "ModApi.h"

namespace th_mod
{
namespace th08
{
namespace
{

float Scale(float value, uint32_t numerator, uint32_t denominator)
{
    return value * static_cast<float>(numerator) /
           static_cast<float>(denominator);
}

ThModDifficultyStateV1 ReadState()
{
    ThModDifficultyStateV1 state;
    th_mod_get_difficulty_state_v1(&state);
    return state;
}

} // namespace

float ScalePlayerSpeed(float value)
{
    const ThModDifficultyStateV1 state = ReadState();
    return Scale(value, state.player_speed_numerator,
                 state.player_speed_denominator);
}

float ScaleProjectileSpeed(float value)
{
    const ThModDifficultyStateV1 state = ReadState();
    return Scale(value, state.projectile_speed_numerator,
                 state.projectile_speed_denominator);
}

float ScalePlayerHitbox(float value)
{
    const ThModDifficultyStateV1 state = ReadState();
    return Scale(value, state.player_hitbox_numerator,
                 state.player_hitbox_denominator);
}

float ScalePlayerGraze(float value)
{
    const ThModDifficultyStateV1 state = ReadState();
    return Scale(value, state.player_graze_numerator,
                 state.player_graze_denominator);
}

float ScaleLaserGrazeMargin(float value)
{
    return ScalePlayerGraze(value);
}

} // namespace th08
} // namespace th_mod
