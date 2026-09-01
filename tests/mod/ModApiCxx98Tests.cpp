#include "ModApi.h"

#include <string.h>

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

    config.enabled_mods = TH_MOD_BUILTIN_AUTOSHOT |
                          TH_MOD_BUILTIN_HIDDEN |
                          TH_MOD_BUILTIN_FLASHLIGHT |
                          TH_MOD_BUILTIN_MIRROR |
                          TH_MOD_BUILTIN_NO_FAIL |
                          TH_MOD_BUILTIN_DOUBLE_TIME;
    config.mirror_mode = TH_MOD_MIRROR_ROTATE_90;
    if (th_mod_configure_v1(&config) != TH_MOD_RESULT_OK)
    {
        return 4;
    }
    char manifest[192];
    if (th_mod_manifest_v1_size() > sizeof(manifest) ||
        th_mod_write_manifest_v1(manifest, sizeof(manifest)) !=
            TH_MOD_RESULT_OK ||
        strcmp(manifest,
               "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
               "api=1;mods=HD@1(45,45)+FL@1(96,224)+AT@1+"
               "MR@1(rotate-90)+NF@1+DT@1") != 0)
    {
        return 5;
    }
    if (th_mod_begin_run() != TH_MOD_RESULT_OK)
    {
        return 6;
    }
    if (th_mod_is_run_active() == 0)
    {
        return 7;
    }
    if (th_mod_filter_actions_v1(TH_MOD_ACTION_UP,
                                 TH_MOD_INPUT_CONTEXT_GAMEPLAY) !=
        (TH_MOD_ACTION_LEFT | TH_MOD_ACTION_SHOOT))
    {
        return 8;
    }
    if (th_mod_hidden_alpha_v1(255, 1000) != 0)
    {
        return 9;
    }
    ThModFlashlightStateV1 flashlight;
    if (th_mod_get_flashlight_state_v1(&flashlight) != TH_MOD_RESULT_OK ||
        flashlight.struct_size != sizeof(flashlight) ||
        flashlight.is_active != 1)
    {
        return 10;
    }
    ThModMirrorStateV1 mirror;
    if (th_mod_get_mirror_state_v1(&mirror) != TH_MOD_RESULT_OK ||
        mirror.struct_size != sizeof(mirror) ||
        mirror.is_active != 1 ||
        mirror.mode != TH_MOD_MIRROR_ROTATE_90)
    {
        return 11;
    }
    ThModNoFailDecisionV1 noFail;
    if (th_mod_nofail_decide_miss_v1(0, &noFail) != TH_MOD_RESULT_OK ||
        noFail.struct_size != sizeof(noFail) ||
        noFail.continue_run != 1 ||
        noFail.consume_life != 0 ||
        noFail.reserved != 0)
    {
        return 12;
    }
    ThModTimeScaleV1 timeScale;
    th_mod_reset_simulation_clock_v1();
    if (th_mod_get_time_scale_v1(&timeScale) != TH_MOD_RESULT_OK ||
        timeScale.struct_size != sizeof(timeScale) ||
        timeScale.is_active != 1 ||
        timeScale.numerator != 3 ||
        timeScale.denominator != 2 ||
        th_mod_next_simulation_tick_count_v1() != 1 ||
        th_mod_next_simulation_tick_count_v1() != 2)
    {
        return 13;
    }
    if (th_mod_end_run() != TH_MOD_RESULT_OK)
    {
        return 14;
    }

    return 0;
}
