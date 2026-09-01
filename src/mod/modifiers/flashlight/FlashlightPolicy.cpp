#include "modifiers/flashlight/FlashlightPolicy.hpp"

namespace th_mod
{
namespace flashlight
{

ThModFlashlightStateV1 GetState(const ThModRunConfigV1 &config,
                                bool runActive)
{
    ThModFlashlightStateV1 state = {};
    state.struct_size = sizeof(state);
    state.is_active =
        runActive && (config.enabled_mods & TH_MOD_BUILTIN_FLASHLIGHT) != 0;
    state.radius_pixels = config.flashlight_radius_pixels;
    state.opacity = config.flashlight_opacity;
    return state;
}

} // namespace flashlight
} // namespace th_mod
