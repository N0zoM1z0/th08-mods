#include "manifest/ManifestV1.hpp"

#include "modifiers/autoshot/AutoshotPolicy.hpp"
#include "modifiers/flashlight/FlashlightPolicy.hpp"
#include "modifiers/hidden/HiddenPolicy.hpp"

#include <cstdint>

namespace th_mod
{
namespace manifest
{
namespace
{

void AppendUnsigned(std::string &output, uint32_t value)
{
    char reversed[10];
    unsigned int count = 0;
    do
    {
        reversed[count++] = static_cast<char>('0' + value % 10);
        value /= 10;
    } while (value != 0);

    while (count != 0)
    {
        output += reversed[--count];
    }
}

void BeginModifier(std::string &output, bool &hasModifier,
                   const char *code, uint32_t version)
{
    if (hasModifier)
    {
        output += '+';
    }
    hasModifier = true;
    output += code;
    output += '@';
    AppendUnsigned(output, version);
}

} // namespace

std::string BuildV1(const ThModRunConfigV1 &config)
{
    std::string output =
        "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;api=";
    AppendUnsigned(output, config.api_version);
    output += ";mods=";

    bool hasModifier = false;
    if ((config.enabled_mods & TH_MOD_BUILTIN_HIDDEN) != 0)
    {
        BeginModifier(output, hasModifier, "HD", hidden::kRulesetVersion);
        output += '(';
        AppendUnsigned(output, config.hidden_visible_ticks);
        output += ',';
        AppendUnsigned(output, config.hidden_fade_ticks);
        output += ')';
    }
    if ((config.enabled_mods & TH_MOD_BUILTIN_FLASHLIGHT) != 0)
    {
        BeginModifier(output, hasModifier, "FL", flashlight::kRulesetVersion);
        output += '(';
        AppendUnsigned(output, config.flashlight_radius_pixels);
        output += ',';
        AppendUnsigned(output, config.flashlight_opacity);
        output += ')';
    }
    if ((config.enabled_mods & TH_MOD_BUILTIN_AUTOSHOT) != 0)
    {
        BeginModifier(output, hasModifier, "AT", autoshot::kRulesetVersion);
    }

    if (!hasModifier)
    {
        output += "none";
    }
    return output;
}

} // namespace manifest
} // namespace th_mod
