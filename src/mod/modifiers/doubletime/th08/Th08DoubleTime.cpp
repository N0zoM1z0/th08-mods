#include "mod/modifiers/doubletime/th08/Th08DoubleTime.hpp"

#include "GameManager.hpp"
#include "mod/api/ModApi.h"

namespace th08
{
namespace mods
{
namespace doubletime
{

u32 SimulationTicksForPresentation()
{
    if (!g_GameManager.flags.unk2)
    {
        th_mod_reset_simulation_clock_v1();
        return 1;
    }

    return th_mod_next_simulation_tick_count_v1();
}

} // namespace doubletime
} // namespace mods
} // namespace th08
