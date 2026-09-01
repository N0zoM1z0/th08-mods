#include "mod/modifiers/hidden/th08/Th08Hidden.hpp"

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

uint32_t BulletAlpha(uint32_t baseAlpha, const Bullet &bullet)
{
    return th_mod_hidden_alpha_v1(
        baseAlpha, NonNegativeTicks(bullet.timer0.current));
}

uint32_t LaserAlpha(uint32_t baseAlpha, const Laser &laser)
{
    return th_mod_hidden_alpha_v1(baseAlpha, LaserActiveAgeTicks(laser));
}

} // namespace hidden
} // namespace mods
} // namespace th08
