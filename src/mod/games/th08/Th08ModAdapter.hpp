#ifndef TH08_MOD_GAMES_TH08_MOD_ADAPTER_HPP
#define TH08_MOD_GAMES_TH08_MOD_ADAPTER_HPP

#include "mod/api/ModApi.h"

namespace th08
{
namespace mods
{

bool Initialize();
void Shutdown();
bool RegisterHooks();
const ThModRunConfigV1 &GetRunConfig();
bool IsEnabled(ThModBuiltinV1 modifier);

} // namespace mods
} // namespace th08

#endif
