#include "mod/games/th08/Th08InputTimeline.hpp"

namespace th08
{
namespace mods
{
namespace input
{

ActionTimeline FilterActionTimeline(uint32_t guiActions,
                                    uint32_t frameActions,
                                    uint32_t context)
{
    ActionTimeline filtered;
    filtered.gui_actions = th_mod_filter_actions_v1(guiActions, context);
    filtered.frame_actions = th_mod_filter_actions_v1(frameActions, context);
    return filtered;
}

} // namespace input
} // namespace mods
} // namespace th08
