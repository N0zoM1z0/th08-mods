#ifndef TH08_MOD_GAMES_TH08_INPUT_TIMELINE_HPP
#define TH08_MOD_GAMES_TH08_INPUT_TIMELINE_HPP

#include "ModApi.h"

namespace th08
{
namespace mods
{
namespace input
{

struct ActionTimeline
{
    uint32_t player_actions;
    uint32_t recorder_actions;
};

ActionTimeline AdvanceLiveInputTimeline(uint32_t previousRecordedActions,
                                        uint32_t sampledActions,
                                        uint32_t context);

} // namespace input
} // namespace mods
} // namespace th08

#endif
