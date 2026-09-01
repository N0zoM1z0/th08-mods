#ifndef TH_MOD_GAMES_TH08_TH08_PROJECTILE_OBSERVATION_HPP
#define TH_MOD_GAMES_TH08_TH08_PROJECTILE_OBSERVATION_HPP

#include "ZunResult.hpp"

namespace th08
{
struct AnmVm;
struct Bullet;
struct Laser;

namespace mods
{
namespace projectile_observation
{

ZunResult DrawBullet(AnmVm &vm, const Bullet &bullet);
ZunResult DrawLaser(AnmVm &vm, const Laser &laser);

} // namespace projectile_observation
} // namespace mods
} // namespace th08

#endif
