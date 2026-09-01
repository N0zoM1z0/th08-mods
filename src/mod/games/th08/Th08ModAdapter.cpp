#include "mod/games/th08/Th08ModAdapter.hpp"

#include "mod/games/th08/Th08InputAdapter.hpp"
#include <stdio.h>

namespace th08
{
namespace mods
{
namespace
{

ThModRunConfigV1 g_runConfig;
bool g_initialized;

} // namespace

bool Initialize()
{
    if (g_initialized)
    {
        return true;
    }

    ThModResult result = th_mod_get_config_v1(&g_runConfig);
    if (result == TH_MOD_RESULT_OK)
    {
        result = th_mod_validate_config_v1(&g_runConfig);
    }
    if (result == TH_MOD_RESULT_OK)
    {
        result = th_mod_begin_run();
    }
    if (result != TH_MOD_RESULT_OK)
    {
        fprintf(stderr, "th08-mod: runtime initialization failed (%d)\n",
                static_cast<int>(result));
        return false;
    }

    g_initialized = true;
    fprintf(stderr, "th08-mod: API v%lu initialized with mask 0x%08lx\n",
            static_cast<unsigned long>(g_runConfig.api_version),
            static_cast<unsigned long>(g_runConfig.enabled_mods));
    return true;
}

void Shutdown()
{
    if (!g_initialized)
    {
        return;
    }

    const ThModResult result = th_mod_end_run();
    if (result != TH_MOD_RESULT_OK)
    {
        fprintf(stderr, "th08-mod: runtime shutdown failed (%d)\n",
                static_cast<int>(result));
    }
    g_initialized = false;
}

bool RegisterHooks()
{
    if (!g_initialized)
    {
        return false;
    }
    const bool filtersInput = IsEnabled(TH_MOD_BUILTIN_AUTOSHOT) ||
                              IsEnabled(TH_MOD_BUILTIN_MIRROR);
    if (filtersInput && !input::RegisterHook())
    {
        return false;
    }
    return true;
}

const ThModRunConfigV1 &GetRunConfig()
{
    return g_runConfig;
}

bool IsEnabled(ThModBuiltinV1 modifier)
{
    return (g_runConfig.enabled_mods & static_cast<uint32_t>(modifier)) != 0;
}

} // namespace mods
} // namespace th08
