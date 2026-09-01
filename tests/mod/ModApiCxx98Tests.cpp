#include "ModApi.h"

int main()
{
    if (th_mod_config_v1_size() != sizeof(ThModRunConfigV1))
    {
        return 1;
    }

    ThModRunConfigV1 config;
    if (th_mod_get_default_config_v1(&config) != TH_MOD_RESULT_OK)
    {
        return 2;
    }
    if (config.struct_size != sizeof(config) ||
        config.api_version != TH_MOD_API_VERSION_V1)
    {
        return 3;
    }

    config.enabled_mods = TH_MOD_BUILTIN_AUTOSHOT | TH_MOD_BUILTIN_HIDDEN;
    if (th_mod_configure_v1(&config) != TH_MOD_RESULT_OK)
    {
        return 4;
    }
    if (th_mod_begin_run() != TH_MOD_RESULT_OK)
    {
        return 5;
    }
    if (th_mod_is_run_active() == 0)
    {
        return 6;
    }
    if (th_mod_filter_actions_v1(0, TH_MOD_INPUT_CONTEXT_GAMEPLAY) !=
        TH_MOD_ACTION_SHOOT)
    {
        return 7;
    }
    if (th_mod_hidden_alpha_v1(255, 1000) != 0)
    {
        return 8;
    }
    if (th_mod_end_run() != TH_MOD_RESULT_OK)
    {
        return 9;
    }

    return 0;
}
