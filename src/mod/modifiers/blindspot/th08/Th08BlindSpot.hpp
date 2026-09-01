#ifndef TH_MOD_MODIFIERS_BLINDSPOT_TH08_TH08_BLIND_SPOT_HPP
#define TH_MOD_MODIFIERS_BLINDSPOT_TH08_TH08_BLIND_SPOT_HPP

#include "Global.hpp"

namespace th08
{
struct Bullet;
struct Laser;

namespace mods
{
namespace blindspot
{

uint32_t BulletAlpha(uint32_t baseAlpha, const Bullet &bullet);
uint32_t LaserAlpha(uint32_t baseAlpha, const Laser &laser);

} // namespace blindspot
} // namespace mods
} // namespace th08

#endif
