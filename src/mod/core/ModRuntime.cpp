#include "ModApi.h"
#include "manifest/ManifestV1.hpp"
#include "modifiers/autoshot/AutoshotPolicy.hpp"
#include "modifiers/doubletime/DoubleTimePolicy.hpp"
#include "modifiers/easy/EasyPolicy.hpp"
#include "modifiers/flashlight/FlashlightPolicy.hpp"
#include "modifiers/hardrock/HardRockPolicy.hpp"
#include "modifiers/hidden/HiddenPolicy.hpp"
#include "modifiers/mirror/MirrorPolicy.hpp"
#include "modifiers/nofail/NoFailPolicy.hpp"
#include "registry/ModifierRegistryV1.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

constexpr uint32_t kMaximumTickOption = 60u * 60u * 10u;
constexpr uint32_t kMaximumFlashlightRadius = 4096u;

static_assert(sizeof(uint32_t) == 4, "The mod ABI requires 32-bit uint32_t.");
static_assert(sizeof(ThModRunConfigV1) == 60,
              "ThModRunConfigV1 must retain its version 1 ABI size.");
static_assert(sizeof(ThModFlashlightStateV1) == 16,
              "ThModFlashlightStateV1 must retain its version 1 ABI size.");
static_assert(sizeof(ThModMirrorStateV1) == 16,
              "ThModMirrorStateV1 must retain its version 1 ABI size.");
static_assert(sizeof(ThModNoFailDecisionV1) == 16,
              "ThModNoFailDecisionV1 must retain its version 1 ABI size.");
static_assert(sizeof(ThModTimeScaleV1) == 16,
              "ThModTimeScaleV1 must retain its version 1 ABI size.");
static_assert(sizeof(ThModDifficultyStateV1) == 64,
              "ThModDifficultyStateV1 must retain its version 1 ABI size.");

ThModRunConfigV1 MakeDefaultConfig()
{
    ThModRunConfigV1 config = {};
    config.struct_size = sizeof(config);
    config.api_version = TH_MOD_API_VERSION_V1;
    config.hidden_visible_ticks = 45;
    config.hidden_fade_ticks = 45;
    config.flashlight_radius_pixels = 96;
    config.flashlight_opacity = 224;
    return config;
}

struct RuntimeState {
    ThModRunConfigV1 config;
    bool run_active;
    uint32_t time_accumulator;
};

RuntimeState g_runtime = {MakeDefaultConfig(), false, 0};

bool ReservedFieldsAreZero(const ThModRunConfigV1 &config)
{
    for (std::size_t index = 0;
         index < sizeof(config.reserved) / sizeof(config.reserved[0]);
         ++index)
    {
        if (config.reserved[index] != 0)
        {
            return false;
        }
    }
    return true;
}

} // namespace

extern "C" uint32_t th_mod_config_v1_size(void)
{
    return sizeof(ThModRunConfigV1);
}

extern "C" ThModResult th_mod_get_default_config_v1(
    ThModRunConfigV1 *out_config)
{
    if (out_config == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_config = MakeDefaultConfig();
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_validate_config_v1(
    const ThModRunConfigV1 *config)
{
    if (config == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }
    if (config->struct_size != sizeof(*config))
    {
        return TH_MOD_RESULT_STRUCT_SIZE;
    }
    if (config->api_version != TH_MOD_API_VERSION_V1)
    {
        return TH_MOD_RESULT_API_VERSION;
    }
    const ThModResult selectionValidation =
        th_mod::registry::ValidateSelectionV1(config->enabled_mods);
    if (selectionValidation != TH_MOD_RESULT_OK)
    {
        return selectionValidation;
    }
    if (config->hidden_visible_ticks > kMaximumTickOption ||
        config->hidden_fade_ticks == 0 ||
        config->hidden_fade_ticks > kMaximumTickOption ||
        config->flashlight_radius_pixels == 0 ||
        config->flashlight_radius_pixels > kMaximumFlashlightRadius ||
        config->flashlight_opacity > 255 ||
        config->mirror_mode > TH_MOD_MIRROR_ROTATE_270 ||
        !ReservedFieldsAreZero(*config))
    {
        return TH_MOD_RESULT_INVALID_OPTION;
    }

    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_configure_v1(
    const ThModRunConfigV1 *config)
{
    if (g_runtime.run_active)
    {
        return TH_MOD_RESULT_RUN_STATE;
    }

    const ThModResult validation = th_mod_validate_config_v1(config);
    if (validation != TH_MOD_RESULT_OK)
    {
        return validation;
    }

    g_runtime.config = *config;
    g_runtime.time_accumulator = 0;
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_get_config_v1(ThModRunConfigV1 *out_config)
{
    if (out_config == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_config = g_runtime.config;
    return TH_MOD_RESULT_OK;
}

extern "C" uint32_t th_mod_manifest_v1_size(void)
{
    const std::string manifest = th_mod::manifest::BuildV1(g_runtime.config);
    return static_cast<uint32_t>(manifest.size() + 1);
}

extern "C" ThModResult th_mod_write_manifest_v1(
    char *out_manifest, uint32_t manifest_capacity)
{
    if (out_manifest == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    const std::string manifest = th_mod::manifest::BuildV1(g_runtime.config);
    const std::size_t requiredSize = manifest.size() + 1;
    if (manifest_capacity < requiredSize)
    {
        return TH_MOD_RESULT_BUFFER_SIZE;
    }

    std::memcpy(out_manifest, manifest.c_str(), requiredSize);
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_begin_run(void)
{
    if (g_runtime.run_active)
    {
        return TH_MOD_RESULT_RUN_STATE;
    }

    g_runtime.run_active = true;
    g_runtime.time_accumulator = 0;
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_end_run(void)
{
    if (!g_runtime.run_active)
    {
        return TH_MOD_RESULT_RUN_STATE;
    }

    g_runtime.run_active = false;
    g_runtime.time_accumulator = 0;
    return TH_MOD_RESULT_OK;
}

extern "C" uint32_t th_mod_is_run_active(void)
{
    return g_runtime.run_active ? 1u : 0u;
}

extern "C" uint32_t th_mod_filter_actions_v1(uint32_t actions,
                                                uint32_t context)
{
    // V1 replay identity fixes geometry transforms before action injection.
    actions = th_mod::mirror::FilterActions(
        g_runtime.config, g_runtime.run_active, actions, context);
    return th_mod::autoshot::FilterActions(
        g_runtime.config, g_runtime.run_active, actions, context);
}

extern "C" uint32_t th_mod_hidden_alpha_v1(uint32_t base_alpha,
                                             uint32_t active_age_ticks)
{
    return th_mod::hidden::ComputeAlpha(
        g_runtime.config, g_runtime.run_active, base_alpha, active_age_ticks);
}

extern "C" ThModResult th_mod_get_flashlight_state_v1(
    ThModFlashlightStateV1 *out_state)
{
    if (out_state == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_state = th_mod::flashlight::GetState(
        g_runtime.config, g_runtime.run_active);
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_get_mirror_state_v1(
    ThModMirrorStateV1 *out_state)
{
    if (out_state == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_state = th_mod::mirror::GetState(
        g_runtime.config, g_runtime.run_active);
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_nofail_decide_miss_v1(
    int32_t lives_remaining, ThModNoFailDecisionV1 *out_decision)
{
    if (out_decision == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_decision = th_mod::nofail::DecideMiss(
        g_runtime.config, g_runtime.run_active, lives_remaining);
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_get_time_scale_v1(
    ThModTimeScaleV1 *out_state)
{
    if (out_state == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_state = th_mod::doubletime::GetTimeScale(
        g_runtime.config, g_runtime.run_active);
    return TH_MOD_RESULT_OK;
}

extern "C" ThModResult th_mod_get_difficulty_state_v1(
    ThModDifficultyStateV1 *out_state)
{
    if (out_state == 0)
    {
        return TH_MOD_RESULT_NULL_ARGUMENT;
    }

    *out_state = th_mod::hardrock::GetState(
        g_runtime.config, g_runtime.run_active);
    if (out_state->is_active == 0)
    {
        *out_state = th_mod::easy::GetState(
            g_runtime.config, g_runtime.run_active);
    }
    return TH_MOD_RESULT_OK;
}

extern "C" void th_mod_reset_simulation_clock_v1(void)
{
    g_runtime.time_accumulator = 0;
}

extern "C" uint32_t th_mod_next_simulation_tick_count_v1(void)
{
    const ThModTimeScaleV1 state = th_mod::doubletime::GetTimeScale(
        g_runtime.config, g_runtime.run_active);
    return th_mod::doubletime::AdvancePresentation(
        state, g_runtime.time_accumulator);
}
