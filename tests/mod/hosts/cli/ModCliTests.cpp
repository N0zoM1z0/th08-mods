#include "ModApi.h"
#include "TestHarness.hpp"
#include "mod/hosts/cli/ModCli.hpp"

#include <stdio.h>
#include <string>

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

namespace
{

std::string ReadOutput(FILE *file)
{
    fflush(file);
    rewind(file);
    std::string output;
    char chunk[256];
    while (fgets(chunk, sizeof(chunk), file) != NULL)
    {
        output += chunk;
    }
    return output;
}

th_mod::cli::Result RunCli(int argc, char *const *argv,
                           std::string *outputText)
{
    FILE *output = tmpfile();
    FILE *errors = tmpfile();
    CHECK(output != NULL);
    CHECK(errors != NULL);
    if (output == NULL || errors == NULL)
    {
        if (output != NULL) fclose(output);
        if (errors != NULL) fclose(errors);
        return th_mod::cli::kExitFailure;
    }

    const th_mod::cli::Result result =
        th_mod::cli::ConfigureFromArguments(argc, argv, output, errors);
    if (outputText != NULL)
    {
        *outputText = ReadOutput(output);
    }
    fclose(output);
    fclose(errors);
    return result;
}

void TestUnrelatedArgumentsLeaveConfigurationAlone()
{
    ResetRuntime();
    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_AUTOSHOT;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    char executable[] = "th08-modern";
    char dataOption[] = "--data-dir=/game";
    char *arguments[] = {executable, dataOption};
    CHECK(RunCli(2, arguments, NULL) == th_mod::cli::kRunGame);
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.enabled_mods == TH_MOD_BUILTIN_AUTOSHOT);
}

void TestModifierSelection()
{
    ResetRuntime();
    char executable[] = "th08-modern";
    char option[] = "--mods=flashlight,HD,AT,mirror,no-fail,double-time";
    char *arguments[] = {executable, option};
    CHECK(RunCli(2, arguments, NULL) == th_mod::cli::kRunGame);

    ThModRunConfigV1 config;
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.enabled_mods == (TH_MOD_BUILTIN_HIDDEN |
                                  TH_MOD_BUILTIN_FLASHLIGHT |
                                  TH_MOD_BUILTIN_AUTOSHOT |
                                  TH_MOD_BUILTIN_MIRROR |
                                  TH_MOD_BUILTIN_NO_FAIL |
                                  TH_MOD_BUILTIN_DOUBLE_TIME));
    CHECK(config.mirror_mode == TH_MOD_MIRROR_HORIZONTAL);

    char mirrorOption[] = "--mods=MR";
    char mirrorMode[] = "--mirror=90";
    char *mirrorArguments[] = {executable, mirrorOption, mirrorMode};
    CHECK(RunCli(3, mirrorArguments, NULL) == th_mod::cli::kRunGame);
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.enabled_mods == TH_MOD_BUILTIN_MIRROR);
    CHECK(config.mirror_mode == TH_MOD_MIRROR_ROTATE_90);

    char noFailOption[] = "--mods=nofail";
    char *noFailArguments[] = {executable, noFailOption};
    CHECK(RunCli(2, noFailArguments, NULL) == th_mod::cli::kRunGame);
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.enabled_mods == TH_MOD_BUILTIN_NO_FAIL);

    char doubleTimeOption[] = "--mods=doubletime";
    char *doubleTimeArguments[] = {executable, doubleTimeOption};
    CHECK(RunCli(2, doubleTimeArguments, NULL) == th_mod::cli::kRunGame);
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.enabled_mods == TH_MOD_BUILTIN_DOUBLE_TIME);

    char modsOption[] = "--mods";
    char none[] = "none";
    char *noneArguments[] = {executable, modsOption, none};
    CHECK(RunCli(3, noneArguments, NULL) == th_mod::cli::kRunGame);
    CHECK(th_mod_get_config_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(config.enabled_mods == 0);
}

void TestInvalidSelections()
{
    ResetRuntime();
    char executable[] = "th08-modern";
    char empty[] = "--mods=";
    char *emptyArguments[] = {executable, empty};
    CHECK(RunCli(2, emptyArguments, NULL) == th_mod::cli::kExitFailure);

    char unknown[] = "--mods=HR";
    char *unknownArguments[] = {executable, unknown};
    CHECK(RunCli(2, unknownArguments, NULL) == th_mod::cli::kExitFailure);

    char mixed[] = "--mods=none,HD";
    char *mixedArguments[] = {executable, mixed};
    CHECK(RunCli(2, mixedArguments, NULL) == th_mod::cli::kExitFailure);

    char first[] = "--mods=HD";
    char second[] = "--mods=FL";
    char *duplicateArguments[] = {executable, first, second};
    CHECK(RunCli(3, duplicateArguments, NULL) ==
          th_mod::cli::kExitFailure);

    char invalidMirror[] = "--mirror=diagonal";
    char *invalidMirrorArguments[] = {executable, invalidMirror};
    CHECK(RunCli(2, invalidMirrorArguments, NULL) ==
          th_mod::cli::kExitFailure);

    char mirror[] = "--mirror";
    char vertical[] = "vertical";
    char duplicateMirror[] = "--mirror=180";
    char *duplicateMirrorArguments[] = {
        executable, mirror, vertical, duplicateMirror};
    CHECK(RunCli(4, duplicateMirrorArguments, NULL) ==
          th_mod::cli::kExitFailure);
}

void TestHelpAndManifestOutput()
{
    ResetRuntime();
    char executable[] = "th08-modern";
    char help[] = "--mod-help";
    char *helpArguments[] = {executable, help};
    std::string output;
    CHECK(RunCli(2, helpArguments, &output) == th_mod::cli::kExitSuccess);
    CHECK(output.find("--mods=HD,FL,AT,MR,NF,DT") != std::string::npos);
    CHECK(output.find("--mirror=MODE") != std::string::npos);

    char mods[] = "--mods=FL";
    char manifest[] = "--mod-manifest";
    char *manifestArguments[] = {executable, mods, manifest};
    CHECK(RunCli(3, manifestArguments, &output) ==
          th_mod::cli::kExitSuccess);
    CHECK(output ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=FL@1(96,224)\n");

    char mirrorMods[] = "--mods=MR";
    char rotate[] = "--mirror=270";
    char *mirrorManifestArguments[] = {
        executable, mirrorMods, rotate, manifest};
    CHECK(RunCli(4, mirrorManifestArguments, &output) ==
          th_mod::cli::kExitSuccess);
    CHECK(output ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=MR@1(rotate-270)\n");

    char noFailMods[] = "--mods=NF";
    char *noFailManifestArguments[] = {
        executable, noFailMods, manifest};
    CHECK(RunCli(3, noFailManifestArguments, &output) ==
          th_mod::cli::kExitSuccess);
    CHECK(output ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=NF@1\n");

    char doubleTimeMods[] = "--mods=DT";
    char *doubleTimeManifestArguments[] = {
        executable, doubleTimeMods, manifest};
    CHECK(RunCli(3, doubleTimeManifestArguments, &output) ==
          th_mod::cli::kExitSuccess);
    CHECK(output ==
          "game=th08@1.00d;engine=th08-mods@1;base=th08-web@3f926db;"
          "api=1;mods=DT@1\n");
}

} // namespace

int main()
{
    TestUnrelatedArgumentsLeaveConfigurationAlone();
    TestModifierSelection();
    TestInvalidSelections();
    TestHelpAndManifestOutput();

    if (g_failures != 0)
    {
        return 1;
    }
    return 0;
}
