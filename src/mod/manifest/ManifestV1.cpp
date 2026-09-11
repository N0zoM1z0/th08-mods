#include "manifest/ManifestV1.hpp"

#include "modifiers/mirror/MirrorPolicy.hpp"
#include "registry/ModifierRegistryV1.hpp"

#include <cstddef>
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
        "game=th08@1.00d;engine=th08-mods@1;base=th08-web@e3485ab;api=";
    AppendUnsigned(output, config.api_version);
    output += ";mods=";

    bool hasModifier = false;
    const registry::ModifierDescriptorV1 *descriptors =
        registry::BuiltinsV1();
    for (std::size_t index = 0; index < registry::BuiltinCountV1(); ++index)
    {
        const registry::ModifierDescriptorV1 &descriptor = descriptors[index];
        if ((config.enabled_mods & descriptor.flag) == 0)
        {
            continue;
        }

        BeginModifier(output, hasModifier, descriptor.code,
                      descriptor.ruleset_version);
        switch (descriptor.flag)
        {
        case TH_MOD_BUILTIN_HIDDEN:
            output += '(';
            AppendUnsigned(output, config.hidden_visible_ticks);
            output += ',';
            AppendUnsigned(output, config.hidden_fade_ticks);
            output += ')';
            break;
        case TH_MOD_BUILTIN_FLASHLIGHT:
            output += '(';
            AppendUnsigned(output, config.flashlight_radius_pixels);
            output += ',';
            AppendUnsigned(output, config.flashlight_opacity);
            output += ')';
            break;
        case TH_MOD_BUILTIN_MIRROR:
            output += '(';
            output += mirror::ModeIdentifier(config.mirror_mode);
            output += ')';
            break;
        default:
            break;
        }
    }

    if (!hasModifier)
    {
        output += "none";
    }
    return output;
}

} // namespace manifest
} // namespace th_mod
