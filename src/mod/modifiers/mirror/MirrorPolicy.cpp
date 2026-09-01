#include "modifiers/mirror/MirrorPolicy.hpp"

#include "input/InputContext.hpp"

namespace th_mod
{
namespace mirror
{
namespace
{

uint32_t MapDirections(uint32_t directions, uint32_t mode)
{
    uint32_t mapped = 0;
    if (mode == TH_MOD_MIRROR_HORIZONTAL)
    {
        if (directions & TH_MOD_ACTION_UP) mapped |= TH_MOD_ACTION_UP;
        if (directions & TH_MOD_ACTION_DOWN) mapped |= TH_MOD_ACTION_DOWN;
        if (directions & TH_MOD_ACTION_LEFT) mapped |= TH_MOD_ACTION_RIGHT;
        if (directions & TH_MOD_ACTION_RIGHT) mapped |= TH_MOD_ACTION_LEFT;
    }
    else if (mode == TH_MOD_MIRROR_VERTICAL)
    {
        if (directions & TH_MOD_ACTION_UP) mapped |= TH_MOD_ACTION_DOWN;
        if (directions & TH_MOD_ACTION_DOWN) mapped |= TH_MOD_ACTION_UP;
        if (directions & TH_MOD_ACTION_LEFT) mapped |= TH_MOD_ACTION_LEFT;
        if (directions & TH_MOD_ACTION_RIGHT) mapped |= TH_MOD_ACTION_RIGHT;
    }
    else if (mode == TH_MOD_MIRROR_ROTATE_90)
    {
        if (directions & TH_MOD_ACTION_UP) mapped |= TH_MOD_ACTION_LEFT;
        if (directions & TH_MOD_ACTION_RIGHT) mapped |= TH_MOD_ACTION_UP;
        if (directions & TH_MOD_ACTION_DOWN) mapped |= TH_MOD_ACTION_RIGHT;
        if (directions & TH_MOD_ACTION_LEFT) mapped |= TH_MOD_ACTION_DOWN;
    }
    else if (mode == TH_MOD_MIRROR_ROTATE_180)
    {
        if (directions & TH_MOD_ACTION_UP) mapped |= TH_MOD_ACTION_DOWN;
        if (directions & TH_MOD_ACTION_DOWN) mapped |= TH_MOD_ACTION_UP;
        if (directions & TH_MOD_ACTION_LEFT) mapped |= TH_MOD_ACTION_RIGHT;
        if (directions & TH_MOD_ACTION_RIGHT) mapped |= TH_MOD_ACTION_LEFT;
    }
    else if (mode == TH_MOD_MIRROR_ROTATE_270)
    {
        if (directions & TH_MOD_ACTION_UP) mapped |= TH_MOD_ACTION_RIGHT;
        if (directions & TH_MOD_ACTION_RIGHT) mapped |= TH_MOD_ACTION_DOWN;
        if (directions & TH_MOD_ACTION_DOWN) mapped |= TH_MOD_ACTION_LEFT;
        if (directions & TH_MOD_ACTION_LEFT) mapped |= TH_MOD_ACTION_UP;
    }
    return mapped;
}

} // namespace

uint32_t FilterActions(const ThModRunConfigV1 &config, bool runActive,
                       uint32_t actions, uint32_t context)
{
    if (!runActive || !input::CanTransformGameplayInput(context) ||
        (config.enabled_mods & TH_MOD_BUILTIN_MIRROR) == 0)
    {
        return actions;
    }

    const uint32_t directionMask = TH_MOD_ACTION_UP | TH_MOD_ACTION_DOWN |
                                   TH_MOD_ACTION_LEFT | TH_MOD_ACTION_RIGHT;
    return (actions & ~directionMask) |
           MapDirections(actions & directionMask, config.mirror_mode);
}

ThModMirrorStateV1 GetState(const ThModRunConfigV1 &config, bool runActive)
{
    ThModMirrorStateV1 state = {};
    state.struct_size = sizeof(state);
    state.is_active =
        runActive && (config.enabled_mods & TH_MOD_BUILTIN_MIRROR) != 0;
    state.mode = config.mirror_mode;
    return state;
}

const char *ModeIdentifier(uint32_t mode)
{
    switch (mode)
    {
    case TH_MOD_MIRROR_HORIZONTAL: return "horizontal";
    case TH_MOD_MIRROR_VERTICAL: return "vertical";
    case TH_MOD_MIRROR_ROTATE_90: return "rotate-90";
    case TH_MOD_MIRROR_ROTATE_180: return "rotate-180";
    case TH_MOD_MIRROR_ROTATE_270: return "rotate-270";
    default: return "invalid";
    }
}

} // namespace mirror
} // namespace th_mod
