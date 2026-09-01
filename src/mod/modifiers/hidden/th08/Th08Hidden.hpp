#ifndef TH_MOD_MODIFIERS_HIDDEN_TH08_TH08_HIDDEN_HPP
#define TH_MOD_MODIFIERS_HIDDEN_TH08_TH08_HIDDEN_HPP

#include "ZunResult.hpp"

namespace th08
{
struct AnmVm;
struct Bullet;
struct Laser;

namespace mods
{
namespace hidden
{

ZunResult DrawBullet(AnmVm &vm, const Bullet &bullet);
ZunResult DrawLaser(AnmVm &vm, const Laser &laser);

} // namespace hidden
} // namespace mods
} // namespace th08

#endif
