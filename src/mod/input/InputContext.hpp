#ifndef TH_MOD_INPUT_INPUT_CONTEXT_HPP
#define TH_MOD_INPUT_INPUT_CONTEXT_HPP

#include "ModApi.h"

namespace th_mod
{
namespace input
{

inline bool CanTransformGameplayInput(uint32_t context)
{
    return (context & TH_MOD_INPUT_CONTEXT_GAMEPLAY) != 0 &&
           (context & TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK) == 0 &&
           (context & TH_MOD_INPUT_CONTEXT_UI_BLOCKED) == 0;
}

inline bool CanInjectGameplayAction(uint32_t context)
{
    return CanTransformGameplayInput(context) &&
           (context & TH_MOD_INPUT_CONTEXT_DIALOGUE) == 0;
}

} // namespace input
} // namespace th_mod

#endif
