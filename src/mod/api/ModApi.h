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
    TH_MOD_RESULT_RUN_STATE = -6,
    TH_MOD_RESULT_BUFFER_SIZE = -7,
    TH_MOD_RESULT_CONFLICT = -8
} ThModResult;

typedef enum ThModBuiltinV1 {
    TH_MOD_BUILTIN_HIDDEN = 1u << 0,
    TH_MOD_BUILTIN_FLASHLIGHT = 1u << 1,
    TH_MOD_BUILTIN_AUTOSHOT = 1u << 2,
    TH_MOD_BUILTIN_MIRROR = 1u << 3
} ThModBuiltinV1;

typedef enum ThModMirrorModeV1 {
    TH_MOD_MIRROR_HORIZONTAL = 0,
    TH_MOD_MIRROR_VERTICAL = 1,
    TH_MOD_MIRROR_ROTATE_90 = 2,
    TH_MOD_MIRROR_ROTATE_180 = 3,
    TH_MOD_MIRROR_ROTATE_270 = 4
} ThModMirrorModeV1;

typedef enum ThModActionV1 {
    TH_MOD_ACTION_SHOOT = 1u << 0,
    TH_MOD_ACTION_BOMB = 1u << 1,
    TH_MOD_ACTION_FOCUS = 1u << 2,
    TH_MOD_ACTION_MENU = 1u << 3,
    TH_MOD_ACTION_UP = 1u << 4,
    TH_MOD_ACTION_DOWN = 1u << 5,
    TH_MOD_ACTION_LEFT = 1u << 6,
    TH_MOD_ACTION_RIGHT = 1u << 7
} ThModActionV1;

typedef enum ThModInputContextV1 {
    TH_MOD_INPUT_CONTEXT_GAMEPLAY = 1u << 0,
    TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK = 1u << 1,
    TH_MOD_INPUT_CONTEXT_UI_BLOCKED = 1u << 2,
    TH_MOD_INPUT_CONTEXT_DIALOGUE = 1u << 3
} ThModInputContextV1;

typedef struct ThModRunConfigV1 {
    uint32_t struct_size;
    uint32_t api_version;
    uint32_t enabled_mods;
    uint32_t hidden_visible_ticks;
    uint32_t hidden_fade_ticks;
    uint32_t flashlight_radius_pixels;
    uint32_t flashlight_opacity;
    uint32_t mirror_mode;
    uint32_t reserved[7];
} ThModRunConfigV1;

typedef struct ThModFlashlightStateV1 {
    uint32_t struct_size;
    uint32_t is_active;
    uint32_t radius_pixels;
    uint32_t opacity;
} ThModFlashlightStateV1;

typedef struct ThModMirrorStateV1 {
    uint32_t struct_size;
    uint32_t is_active;
    uint32_t mode;
    uint32_t reserved;
} ThModMirrorStateV1;

uint32_t th_mod_config_v1_size(void);
ThModResult th_mod_get_default_config_v1(ThModRunConfigV1 *out_config);
ThModResult th_mod_validate_config_v1(const ThModRunConfigV1 *config);
ThModResult th_mod_configure_v1(const ThModRunConfigV1 *config);
ThModResult th_mod_get_config_v1(ThModRunConfigV1 *out_config);
uint32_t th_mod_manifest_v1_size(void);
ThModResult th_mod_write_manifest_v1(char *out_manifest,
                                     uint32_t manifest_capacity);

ThModResult th_mod_begin_run(void);
ThModResult th_mod_end_run(void);
uint32_t th_mod_is_run_active(void);
uint32_t th_mod_filter_actions_v1(uint32_t actions, uint32_t context);
uint32_t th_mod_hidden_alpha_v1(uint32_t base_alpha,
                                uint32_t active_age_ticks);
ThModResult th_mod_get_flashlight_state_v1(
    ThModFlashlightStateV1 *out_state);
ThModResult th_mod_get_mirror_state_v1(ThModMirrorStateV1 *out_state);

#ifdef __cplusplus
}
#endif

#endif
