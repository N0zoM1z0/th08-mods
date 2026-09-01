#include "mod/games/th08/Th08InputTimeline.hpp"

namespace th08
{
namespace mods
{
namespace input
{

ActionTimeline AdvanceLiveInputTimeline(uint32_t previousRecordedActions,
                                        uint32_t sampledActions,
                                        uint32_t context)
{
    ActionTimeline timeline;
    timeline.player_actions = previousRecordedActions;
    timeline.recorder_actions =
        th_mod_filter_actions_v1(sampledActions, context);
    return timeline;
}

} // namespace input
} // namespace mods
} // namespace th08
