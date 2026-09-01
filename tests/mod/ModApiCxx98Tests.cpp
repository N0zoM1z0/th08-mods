#include "ModApi.h"

int main()
{
    ThModRunConfigV1 config;
    if (th_mod_get_default_config_v1(&config) != TH_MOD_RESULT_OK)
    {
        return 1;
    }
    if (config.struct_size != sizeof(config) ||
        config.api_version != TH_MOD_API_VERSION_V1)
    {
        return 2;
    }

    config.enabled_mods = TH_MOD_BUILTIN_AUTOSHOT;
    if (th_mod_configure_v1(&config) != TH_MOD_RESULT_OK)
    {
        return 3;
    }
    if (th_mod_begin_run() != TH_MOD_RESULT_OK)
    {
        return 4;
    }
    if (th_mod_is_run_active() == 0)
    {
        return 5;
    }
    if (th_mod_end_run() != TH_MOD_RESULT_OK)
    {
        return 6;
    }

    return 0;
}
