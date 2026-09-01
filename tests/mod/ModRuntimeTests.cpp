#include "ModApi.h"
#include "TestHarness.hpp"
#include "modifiers/autoshot/AutoshotTests.hpp"
#include "modifiers/flashlight/FlashlightTests.hpp"
#include "modifiers/hidden/HiddenTests.hpp"

#include <cstdint>
#include <iostream>

int g_failures = 0;

void ResetRuntime()
{
    if (th_mod_is_run_active() != 0)
    {
        CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    }

    ThModRunConfigV1 defaults;
    CHECK(th_mod_get_default_config_v1(&defaults) == TH_MOD_RESULT_OK);
    CHECK(th_mod_configure_v1(&defaults) == TH_MOD_RESULT_OK);
}

namespace {

void TestDefaults()
{
    ResetRuntime();

    CHECK(th_mod_config_v1_size() == sizeof(ThModRunConfigV1));
    ThModRunConfigV1 config;
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.struct_size == sizeof(config));
    CHECK(config.api_version == TH_MOD_API_VERSION_V1);
    CHECK(config.enabled_mods == 0);
    CHECK(config.hidden_visible_ticks == 45);
    CHECK(config.hidden_fade_ticks == 45);
    CHECK(config.flashlight_radius_pixels == 96);
    CHECK(config.flashlight_opacity == 224);
    for (std::size_t index = 0;
         index < sizeof(config.reserved) / sizeof(config.reserved[0]);
         ++index)
    {
        CHECK(config.reserved[index] == 0);
    }
}

void TestValidation()
{
    ResetRuntime();

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_validate_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_validate_config_v1(0) == TH_MOD_RESULT_NULL_ARGUMENT);

    config.struct_size = 0;
    CHECK(th_mod_validate_config_v1(&config) == TH_MOD_RESULT_STRUCT_SIZE);
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);

    config.api_version = TH_MOD_API_VERSION_V1 + 1;
    CHECK(th_mod_validate_config_v1(&config) == TH_MOD_RESULT_API_VERSION);
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);

    config.enabled_mods = 1u << 31;
    CHECK(th_mod_validate_config_v1(&config) ==
          TH_MOD_RESULT_UNKNOWN_MODIFIER);
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);

    config.hidden_fade_ticks = 0;
    CHECK(th_mod_validate_config_v1(&config) == TH_MOD_RESULT_INVALID_OPTION);
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);

    config.flashlight_opacity = 256;
    CHECK(th_mod_validate_config_v1(&config) == TH_MOD_RESULT_INVALID_OPTION);
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);

    config.reserved[3] = 1;
    CHECK(th_mod_validate_config_v1(&config) == TH_MOD_RESULT_INVALID_OPTION);
}

void TestConfigurationCopyAndFreeze()
{
    ResetRuntime();

    ThModRunConfigV1 configured;
    CHECK(th_mod_get_default_config_v1(&configured) == TH_MOD_RESULT_OK);
    configured.enabled_mods = TH_MOD_BUILTIN_HIDDEN |
                              TH_MOD_BUILTIN_FLASHLIGHT |
                              TH_MOD_BUILTIN_AUTOSHOT;
    configured.hidden_visible_ticks = 60;
    CHECK(th_mod_configure_v1(&configured) == TH_MOD_RESULT_OK);

    configured.hidden_visible_ticks = 1;
    ThModRunConfigV1 observed;
    CHECK(th_mod_get_config_v1(&observed) == TH_MOD_RESULT_OK);
    CHECK(observed.hidden_visible_ticks == 60);

    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_is_run_active() == 1);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_RUN_STATE);
    CHECK(th_mod_configure_v1(&configured) == TH_MOD_RESULT_RUN_STATE);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    CHECK(th_mod_is_run_active() == 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_RUN_STATE);
    CHECK(th_mod_configure_v1(&configured) == TH_MOD_RESULT_OK);
}

} // namespace

int main()
{
    TestDefaults();
    TestValidation();
    TestConfigurationCopyAndFreeze();
    TestAutoshotPolicy();
    TestFlashlightPolicy();
    TestHiddenPolicy();

    if (g_failures != 0)
    {
        std::cerr << g_failures << " mod core test(s) failed\n";
        return 1;
    }

    std::cout << "All mod core tests passed\n";
    return 0;
}
