#include "mod/modifiers/blindspot/th08/Th08BlindSpot.hpp"

#include "BulletManager.hpp"
#include "Player.hpp"
#include "mod/api/ModApi.h"
#include "mod/games/th08/Th08ModAdapter.hpp"

#include <math.h>

namespace th08
{
namespace mods
{
namespace blindspot
{
namespace
{

uint32_t DistancePixels(f32 deltaX, f32 deltaY)
{
    const f64 distance = sqrt(
        static_cast<f64>(deltaX) * deltaX +
        static_cast<f64>(deltaY) * deltaY);
    return static_cast<uint32_t>(distance);
}

uint32_t DistanceToLaserPixels(const Laser &laser)
{
    const f32 directionX = static_cast<f32>(cos(laser.angle));
    const f32 directionY = static_cast<f32>(sin(laser.angle));
    const f32 playerDeltaX = g_Player.position.x - laser.position.x;
    const f32 playerDeltaY = g_Player.position.y - laser.position.y;
    f32 projection =
        playerDeltaX * directionX + playerDeltaY * directionY;
    f32 segmentStart = laser.startOffset;
    f32 segmentEnd = laser.endOffset;
    if (segmentStart > segmentEnd)
    {
        const f32 temporary = segmentStart;
        segmentStart = segmentEnd;
        segmentEnd = temporary;
    }
    if (projection < segmentStart)
    {
        projection = segmentStart;
    }
    else if (projection > segmentEnd)
    {
        projection = segmentEnd;
    }

    const f32 closestX = laser.position.x + directionX * projection;
    const f32 closestY = laser.position.y + directionY * projection;
    return DistancePixels(
        g_Player.position.x - closestX,
        g_Player.position.y - closestY);
}

} // namespace

uint32_t BulletAlpha(uint32_t baseAlpha, const Bullet &bullet)
{
    if (!th08::mods::IsEnabled(TH_MOD_BUILTIN_BLIND_SPOT))
    {
        return baseAlpha;
    }
    return th_mod_blind_spot_alpha_v1(
        baseAlpha,
        DistancePixels(g_Player.position.x - bullet.position0.x,
                       g_Player.position.y - bullet.position0.y));
}

uint32_t LaserAlpha(uint32_t baseAlpha, const Laser &laser)
{
    if (!th08::mods::IsEnabled(TH_MOD_BUILTIN_BLIND_SPOT))
    {
        return baseAlpha;
    }
    return th_mod_blind_spot_alpha_v1(
        baseAlpha, DistanceToLaserPixels(laser));
}

} // namespace blindspot
} // namespace mods
} // namespace th08
