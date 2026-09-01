#ifndef TH08_MODERN_LINUX_AUDIO_PLAYBACK_RATE_HPP
#define TH08_MODERN_LINUX_AUDIO_PLAYBACK_RATE_HPP

#include "inttypes.hpp"

namespace th08
{
namespace modern
{
namespace audio
{

void SetPlaybackRate(u32 numerator, u32 denominator);

} // namespace audio
} // namespace modern
} // namespace th08

#endif
