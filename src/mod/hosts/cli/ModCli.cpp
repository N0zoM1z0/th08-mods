#include "mod/hosts/cli/ModCli.hpp"

#include "ModApi.h"

#include <new>
#include <stddef.h>
#include <string.h>

namespace th_mod
{
namespace cli
{
namespace
{

bool TokenEquals(const char *token, size_t length, const char *expected)
{
    return strlen(expected) == length &&
           strncmp(token, expected, length) == 0;
}

bool ModifierBit(const char *token, size_t length, uint32_t &bit)
{
    if (TokenEquals(token, length, "HD") ||
        TokenEquals(token, length, "hidden"))
    {
        bit = TH_MOD_BUILTIN_HIDDEN;
        return true;
    }
    if (TokenEquals(token, length, "FL") ||
        TokenEquals(token, length, "flashlight"))
    {
        bit = TH_MOD_BUILTIN_FLASHLIGHT;
        return true;
    }
    if (TokenEquals(token, length, "AT") ||
        TokenEquals(token, length, "autoshot"))
    {
        bit = TH_MOD_BUILTIN_AUTOSHOT;
        return true;
    }
    if (TokenEquals(token, length, "MR") ||
        TokenEquals(token, length, "mirror"))
    {
        bit = TH_MOD_BUILTIN_MIRROR;
        return true;
    }
    if (TokenEquals(token, length, "NF") ||
        TokenEquals(token, length, "nofail") ||
        TokenEquals(token, length, "no-fail"))
    {
        bit = TH_MOD_BUILTIN_NO_FAIL;
        return true;
    }
    return false;
}

bool ParseMirrorMode(const char *value, uint32_t &mode)
{
    if (value == NULL || value[0] == '\0')
    {
        return false;
    }
    const size_t length = strlen(value);
    if (TokenEquals(value, length, "horizontal"))
    {
        mode = TH_MOD_MIRROR_HORIZONTAL;
    }
    else if (TokenEquals(value, length, "vertical"))
    {
        mode = TH_MOD_MIRROR_VERTICAL;
    }
    else if (TokenEquals(value, length, "90") ||
             TokenEquals(value, length, "rotate-90"))
    {
        mode = TH_MOD_MIRROR_ROTATE_90;
    }
    else if (TokenEquals(value, length, "180") ||
             TokenEquals(value, length, "rotate-180"))
    {
        mode = TH_MOD_MIRROR_ROTATE_180;
    }
    else if (TokenEquals(value, length, "270") ||
             TokenEquals(value, length, "rotate-270"))
    {
        mode = TH_MOD_MIRROR_ROTATE_270;
    }
    else
    {
        return false;
    }
    return true;
}

bool ParseModifierList(const char *value, uint32_t &mask)
{
    if (value == NULL || value[0] == '\0')
    {
        return false;
    }

    mask = 0;
    bool sawNone = false;
    const char *token = value;
    for (;;)
    {
        const char *end = strchr(token, ',');
        const size_t length = end != NULL ?
            static_cast<size_t>(end - token) : strlen(token);
        if (length == 0)
        {
            return false;
        }

        if (TokenEquals(token, length, "none"))
        {
            if (mask != 0 || sawNone)
            {
                return false;
            }
            sawNone = true;
        }
        else
        {
            uint32_t bit = 0;
            if (sawNone || !ModifierBit(token, length, bit))
            {
                return false;
            }
            mask |= bit;
        }

        if (end == NULL)
        {
            return true;
        }
        token = end + 1;
    }
}

void PrintHelp(FILE *output)
{
    fprintf(output,
            "TH08 modifier options:\n"
            "  --mods=HD,FL,AT,MR,NF  Enable a comma-separated modifier set.\n"
            "  --mods none         Run without modifiers.\n"
            "  --mirror=MODE       Select horizontal, vertical, 90, 180, or 270.\n"
            "  --mod-manifest      Print the normalized manifest and exit.\n"
            "  --mod-help          Print this help and exit.\n"
            "\n"
            "Names hidden, flashlight, autoshot, mirror, and no-fail are also "
            "accepted.\n"
            "Mirror defaults to horizontal and only applies when MR is enabled.\n");
}

Result PrintManifest(FILE *output, FILE *errors)
{
    const uint32_t size = th_mod_manifest_v1_size();
    char *manifest = new (std::nothrow) char[size];
    if (manifest == NULL)
    {
        fprintf(errors, "th08-mod: unable to allocate manifest output\n");
        return kExitFailure;
    }

    const ThModResult result = th_mod_write_manifest_v1(manifest, size);
    if (result != TH_MOD_RESULT_OK)
    {
        delete[] manifest;
        fprintf(errors, "th08-mod: unable to build manifest (%d)\n",
                static_cast<int>(result));
        return kExitFailure;
    }

    fprintf(output, "%s\n", manifest);
    delete[] manifest;
    return kExitSuccess;
}

} // namespace

Result ConfigureFromArguments(int argc, char *const *argv,
                              FILE *output, FILE *errors)
{
    for (int index = 1; index < argc; ++index)
    {
        if (strcmp(argv[index], "--mod-help") == 0)
        {
            PrintHelp(output);
            return kExitSuccess;
        }
    }

    const char *modifierList = NULL;
    const char *mirrorMode = NULL;
    bool printManifest = false;
    for (int index = 1; index < argc; ++index)
    {
        const char *argument = argv[index];
        if (strcmp(argument, "--mods") == 0)
        {
            if (modifierList != NULL || ++index >= argc)
            {
                fprintf(errors,
                        "th08-mod: --mods requires exactly one modifier list\n");
                return kExitFailure;
            }
            modifierList = argv[index];
        }
        else if (strncmp(argument, "--mods=", 7) == 0)
        {
            if (modifierList != NULL)
            {
                fprintf(errors, "th08-mod: --mods may only be specified once\n");
                return kExitFailure;
            }
            modifierList = argument + 7;
        }
        else if (strcmp(argument, "--mod-manifest") == 0)
        {
            printManifest = true;
        }
        else if (strcmp(argument, "--mirror") == 0)
        {
            if (mirrorMode != NULL || ++index >= argc)
            {
                fprintf(errors,
                        "th08-mod: --mirror requires exactly one mode\n");
                return kExitFailure;
            }
            mirrorMode = argv[index];
        }
        else if (strncmp(argument, "--mirror=", 9) == 0)
        {
            if (mirrorMode != NULL)
            {
                fprintf(errors,
                        "th08-mod: --mirror may only be specified once\n");
                return kExitFailure;
            }
            mirrorMode = argument + 9;
        }
    }

    if (modifierList != NULL || mirrorMode != NULL)
    {
        ThModRunConfigV1 config;
        ThModResult result = th_mod_get_default_config_v1(&config);
        if (result != TH_MOD_RESULT_OK)
        {
            fprintf(errors, "th08-mod: unable to load configuration defaults\n");
            return kExitFailure;
        }
        if (modifierList != NULL &&
            !ParseModifierList(modifierList, config.enabled_mods))
        {
            fprintf(errors,
                    "th08-mod: invalid modifier list; use HD, FL, AT, MR, NF, "
                    "or none\n");
            return kExitFailure;
        }
        if (mirrorMode != NULL &&
            !ParseMirrorMode(mirrorMode, config.mirror_mode))
        {
            fprintf(errors,
                    "th08-mod: invalid Mirror mode; use horizontal, vertical, "
                    "90, 180, or 270\n");
            return kExitFailure;
        }

        result = th_mod_configure_v1(&config);
        if (result != TH_MOD_RESULT_OK)
        {
            fprintf(errors, "th08-mod: configuration was rejected (%d)\n",
                    static_cast<int>(result));
            return kExitFailure;
        }
    }

    if (printManifest)
    {
        return PrintManifest(output, errors);
    }
    return kRunGame;
}

} // namespace cli
} // namespace th_mod
