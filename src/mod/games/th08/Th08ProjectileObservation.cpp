#include "mod/games/th08/Th08ProjectileObservation.hpp"

#include "AnmManager.hpp"
#include "BulletManager.hpp"
#include "mod/modifiers/blindspot/th08/Th08BlindSpot.hpp"
#include "mod/modifiers/hidden/th08/Th08Hidden.hpp"

namespace th08
{
namespace mods
{
namespace projectile_observation
{
namespace
{

ZunResult DrawObserved(AnmVm &vm, uint32_t observedAlpha)
{
    const uint32_t baseColor = vm.color1.d3dColor;
    const uint32_t observedColor =
        (baseColor & 0x00ffffffu) | (observedAlpha << 24);
    if (observedColor == baseColor)
    {
        return g_AnmManager->Draw2D(&vm);
    }

    // Color is presentation state shared with later frames. Restrict the
    // composed observation transform to this draw and restore vanilla state.
    vm.color1.d3dColor = observedColor;
    const ZunResult result = g_AnmManager->Draw2D(&vm);
    vm.color1.d3dColor = baseColor;
    return result;
}

} // namespace

ZunResult DrawBullet(AnmVm &vm, const Bullet &bullet)
{
    uint32_t alpha = hidden::BulletAlpha(vm.color1.d3dColor >> 24, bullet);
    alpha = blindspot::BulletAlpha(alpha, bullet);
    return DrawObserved(vm, alpha);
}

ZunResult DrawLaser(AnmVm &vm, const Laser &laser)
{
    uint32_t alpha = hidden::LaserAlpha(vm.color1.d3dColor >> 24, laser);
    alpha = blindspot::LaserAlpha(alpha, laser);
    return DrawObserved(vm, alpha);
}

} // namespace projectile_observation
} // namespace mods
} // namespace th08
