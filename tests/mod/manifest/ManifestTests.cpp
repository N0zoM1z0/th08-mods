#include "manifest/ManifestTests.hpp"

#include "ModApi.h"
#include "TestHarness.hpp"

#include <string>
#include <vector>

namespace
{

std::string ReadManifest()
{
    const uint32_t size = th_mod_manifest_v1_size();
    std::vector<char> buffer(size);
    CHECK(th_mod_write_manifest_v1(buffer.data(), size) == TH_MOD_RESULT_OK);
    return std::string(buffer.data());
}

} // namespace

void TestManifestV1()
{
    ResetRuntime();

    const std::string defaultManifest =
        "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
        "api=1;mods=none";
    CHECK(ReadManifest() == defaultManifest);
    CHECK(th_mod_manifest_v1_size() == defaultManifest.size() + 1);
    CHECK(th_mod_write_manifest_v1(0, 100) == TH_MOD_RESULT_NULL_ARGUMENT);

    std::vector<char> shortBuffer(defaultManifest.size());
    CHECK(th_mod_write_manifest_v1(shortBuffer.data(), shortBuffer.size()) ==
          TH_MOD_RESULT_BUFFER_SIZE);

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_AUTOSHOT |
                          TH_MOD_BUILTIN_FLASHLIGHT |
                          TH_MOD_BUILTIN_HIDDEN |
                          TH_MOD_BUILTIN_MIRROR |
                          TH_MOD_BUILTIN_NO_FAIL |
                          TH_MOD_BUILTIN_DOUBLE_TIME |
                          TH_MOD_BUILTIN_HARD_ROCK;
    config.hidden_visible_ticks = 60;
    config.hidden_fade_ticks = 30;
    config.flashlight_radius_pixels = 120;
    config.flashlight_opacity = 192;
    config.mirror_mode = TH_MOD_MIRROR_ROTATE_90;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(ReadManifest() ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=HD@1(60,30)+FL@1(120,192)+AT@1+MR@1(rotate-90)+"
          "NF@1+DT@1+HR@1");

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_EASY;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(ReadManifest() ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=EZ@1");

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_RELAX;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(ReadManifest() ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=RX@1");

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_BLIND_SPOT;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(ReadManifest() ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=BS@1");

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_NO_BOMB;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(ReadManifest() ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=NB@1");

    const uint32_t modes[] = {
        TH_MOD_MIRROR_HORIZONTAL,
        TH_MOD_MIRROR_VERTICAL,
        TH_MOD_MIRROR_ROTATE_90,
        TH_MOD_MIRROR_ROTATE_180,
        TH_MOD_MIRROR_ROTATE_270,
    };
    const char *identifiers[] = {
        "horizontal", "vertical", "rotate-90", "rotate-180", "rotate-270",
    };
    for (std::size_t index = 0; index < 5; ++index)
    {
        CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
        config.enabled_mods = TH_MOD_BUILTIN_MIRROR;
        config.mirror_mode = modes[index];
        CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
        CHECK(ReadManifest() ==
              std::string("game=th08@1.00d;engine=th08-mods@1;"
                          "base=th08-web@3f926db;api=1;mods=MR@1(") +
                  identifiers[index] + ")");
    }

    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.hidden_visible_ticks = 999;
    config.flashlight_opacity = 1;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(ReadManifest() == defaultManifest);
}
