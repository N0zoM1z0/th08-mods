#include "mod/modifiers/hidden/th08/Th08Hidden.hpp"

#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "mod/api/ModApi.h"

namespace th08
{
namespace mods
{
namespace hidden
{
namespace
{

uint32_t NonNegativeTicks(i32 ticks)
{
    return ticks > 0 ? static_cast<uint32_t>(ticks) : 0u;
}

ZunResult DrawObserved(AnmVm &vm, uint32_t activeAgeTicks)
{
    const uint32_t baseColor = vm.color1.d3dColor;
    const uint32_t alpha = th_mod_hidden_alpha_v1(
        baseColor >> 24, activeAgeTicks);
    const uint32_t observedColor =
        (baseColor & 0x00ffffffu) | (alpha << 24);
    if (observedColor == baseColor)
    {
        return g_AnmManager->Draw2D(&vm);
    }

    // Color is presentation state shared with later frames. Restrict the
    // observation transform to this draw call and restore vanilla state.
    vm.color1.d3dColor = observedColor;
    const ZunResult result = g_AnmManager->Draw2D(&vm);
    vm.color1.d3dColor = baseColor;
    return result;
}

uint32_t LaserActiveAgeTicks(const Laser &laser)
{
    // TH08 resets the laser timer when start-up becomes active and again when
    // despawn begins. Reattach the completed phase lengths to get one age.
    uint32_t age = NonNegativeTicks(laser.timer.current);
    if (laser.state >= 1)
    {
        age += NonNegativeTicks(laser.startTime);
    }
    if (laser.state >= 2)
    {
        age += NonNegativeTicks(laser.duration);
    }
    return age;
}

} // namespace

ZunResult DrawBullet(AnmVm &vm, const Bullet &bullet)
{
    return DrawObserved(vm, NonNegativeTicks(bullet.timer0.current));
}

ZunResult DrawLaser(AnmVm &vm, const Laser &laser)
{
    return DrawObserved(vm, LaserActiveAgeTicks(laser));
}

} // namespace hidden
} // namespace mods
} // namespace th08
