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
    uint32_t gui_actions;
    uint32_t frame_actions;
};

ActionTimeline FilterActionTimeline(uint32_t guiActions,
                                    uint32_t frameActions,
                                    uint32_t context);

} // namespace input
} // namespace mods
} // namespace th08

#endif
