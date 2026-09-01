#include "registry/ModifierRegistryTests.hpp"

#include "TestHarness.hpp"
#include "registry/ModifierRegistryV1.hpp"

#include <cstring>

void TestModifierRegistryV1()
{
    const th_mod::registry::ModifierDescriptorV1 *builtins =
        th_mod::registry::BuiltinsV1();
    CHECK(th_mod::registry::BuiltinCountV1() == 8);

    const uint32_t expectedFlags[] = {
        TH_MOD_BUILTIN_HIDDEN,
        TH_MOD_BUILTIN_FLASHLIGHT,
        TH_MOD_BUILTIN_AUTOSHOT,
        TH_MOD_BUILTIN_MIRROR,
        TH_MOD_BUILTIN_NO_FAIL,
        TH_MOD_BUILTIN_DOUBLE_TIME,
        TH_MOD_BUILTIN_HARD_ROCK,
        TH_MOD_BUILTIN_EASY,
    };
    const char *expectedCodes[] = {
        "HD", "FL", "AT", "MR", "NF", "DT", "HR", "EZ",
    };
    const uint32_t expectedConflicts[] = {
        0, 0, 0, 0, 0, 0, TH_MOD_BUILTIN_EASY, TH_MOD_BUILTIN_HARD_ROCK,
    };
    for (std::size_t index = 0; index < 8; ++index)
    {
        CHECK(builtins[index].flag == expectedFlags[index]);
        CHECK(std::strcmp(builtins[index].code, expectedCodes[index]) == 0);
        CHECK(builtins[index].ruleset_version == 1);
        CHECK(builtins[index].conflicts == expectedConflicts[index]);
    }

    const uint32_t knownMask = TH_MOD_BUILTIN_HIDDEN |
                               TH_MOD_BUILTIN_FLASHLIGHT |
                               TH_MOD_BUILTIN_AUTOSHOT |
                               TH_MOD_BUILTIN_MIRROR |
                               TH_MOD_BUILTIN_NO_FAIL |
                               TH_MOD_BUILTIN_DOUBLE_TIME |
                               TH_MOD_BUILTIN_HARD_ROCK |
                               TH_MOD_BUILTIN_EASY;
    CHECK(th_mod::registry::KnownMask(builtins, 8) == knownMask);
    CHECK(th_mod::registry::ValidateSelectionV1(
              knownMask & ~TH_MOD_BUILTIN_EASY) == TH_MOD_RESULT_OK);
    CHECK(th_mod::registry::ValidateSelectionV1(
              knownMask & ~TH_MOD_BUILTIN_HARD_ROCK) == TH_MOD_RESULT_OK);
    CHECK(th_mod::registry::ValidateSelectionV1(knownMask) ==
          TH_MOD_RESULT_CONFLICT);
    CHECK(th_mod::registry::ValidateSelectionV1(1u << 31) ==
          TH_MOD_RESULT_UNKNOWN_MODIFIER);

    const th_mod::registry::ModifierDescriptorV1 conflicting[] = {
        {1u << 0, "AA", 1, 1u << 1},
        {1u << 1, "BB", 1, 1u << 0},
    };
    CHECK(!th_mod::registry::HasConflict(
        1u << 0, conflicting, 2));
    CHECK(th_mod::registry::HasConflict(
        (1u << 0) | (1u << 1), conflicting, 2));
}
