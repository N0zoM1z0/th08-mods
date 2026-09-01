#ifndef TH08_MOD_API_MOD_API_H
#define TH08_MOD_API_MOD_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TH_MOD_API_VERSION_V1 1u

typedef enum ThModResult {
    TH_MOD_RESULT_OK = 0,
    TH_MOD_RESULT_NULL_ARGUMENT = -1,
    TH_MOD_RESULT_STRUCT_SIZE = -2,
    TH_MOD_RESULT_API_VERSION = -3,
    TH_MOD_RESULT_UNKNOWN_MODIFIER = -4,
    TH_MOD_RESULT_INVALID_OPTION = -5,
    TH_MOD_RESULT_RUN_STATE = -6
} ThModResult;

typedef enum ThModBuiltinV1 {
    TH_MOD_BUILTIN_HIDDEN = 1u << 0,
    TH_MOD_BUILTIN_FLASHLIGHT = 1u << 1,
    TH_MOD_BUILTIN_AUTOSHOT = 1u << 2
} ThModBuiltinV1;

typedef struct ThModRunConfigV1 {
    uint32_t struct_size;
    uint32_t api_version;
    uint32_t enabled_mods;
    uint32_t hidden_visible_ticks;
    uint32_t hidden_fade_ticks;
    uint32_t flashlight_radius_pixels;
    uint32_t flashlight_opacity;
    uint32_t reserved[8];
} ThModRunConfigV1;

ThModResult th_mod_get_default_config_v1(ThModRunConfigV1 *out_config);
ThModResult th_mod_validate_config_v1(const ThModRunConfigV1 *config);
ThModResult th_mod_configure_v1(const ThModRunConfigV1 *config);
ThModResult th_mod_get_config_v1(ThModRunConfigV1 *out_config);

ThModResult th_mod_begin_run(void);
ThModResult th_mod_end_run(void);
uint32_t th_mod_is_run_active(void);

#ifdef __cplusplus
}
#endif

#endif
