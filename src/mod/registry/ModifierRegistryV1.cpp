#include "registry/ModifierRegistryV1.hpp"

#include "modifiers/autoshot/AutoshotPolicy.hpp"
#include "modifiers/doubletime/DoubleTimePolicy.hpp"
#include "modifiers/easy/EasyPolicy.hpp"
#include "modifiers/flashlight/FlashlightPolicy.hpp"
#include "modifiers/hidden/HiddenPolicy.hpp"
#include "modifiers/hardrock/HardRockPolicy.hpp"
#include "modifiers/mirror/MirrorPolicy.hpp"
#include "modifiers/nofail/NoFailPolicy.hpp"

namespace th_mod
{
namespace registry
{
namespace
{

const ModifierDescriptorV1 kBuiltins[] = {
    {TH_MOD_BUILTIN_HIDDEN, "HD", hidden::kRulesetVersion, 0},
    {TH_MOD_BUILTIN_FLASHLIGHT, "FL", flashlight::kRulesetVersion, 0},
    {TH_MOD_BUILTIN_AUTOSHOT, "AT", autoshot::kRulesetVersion, 0},
    {TH_MOD_BUILTIN_MIRROR, "MR", mirror::kRulesetVersion, 0},
    {TH_MOD_BUILTIN_NO_FAIL, "NF", nofail::kRulesetVersion, 0},
    {TH_MOD_BUILTIN_DOUBLE_TIME, "DT", doubletime::kRulesetVersion, 0},
    {TH_MOD_BUILTIN_HARD_ROCK, "HR", hardrock::kRulesetVersion,
     TH_MOD_BUILTIN_EASY},
    {TH_MOD_BUILTIN_EASY, "EZ", easy::kRulesetVersion,
     TH_MOD_BUILTIN_HARD_ROCK},
};

} // namespace

const ModifierDescriptorV1 *BuiltinsV1()
{
    return kBuiltins;
}

std::size_t BuiltinCountV1()
{
    return sizeof(kBuiltins) / sizeof(kBuiltins[0]);
}

uint32_t KnownMask(const ModifierDescriptorV1 *descriptors,
                   std::size_t count)
{
    uint32_t mask = 0;
    for (std::size_t index = 0; index < count; ++index)
    {
        mask |= descriptors[index].flag;
    }
    return mask;
}

bool HasConflict(uint32_t enabledMods,
                 const ModifierDescriptorV1 *descriptors,
                 std::size_t count)
{
    for (std::size_t index = 0; index < count; ++index)
    {
        const ModifierDescriptorV1 &descriptor = descriptors[index];
        if ((enabledMods & descriptor.flag) != 0 &&
            (enabledMods & descriptor.conflicts) != 0)
        {
            return true;
        }
    }
    return false;
}

ThModResult ValidateSelectionV1(uint32_t enabledMods)
{
    const ModifierDescriptorV1 *descriptors = BuiltinsV1();
    const std::size_t count = BuiltinCountV1();
    if ((enabledMods & ~KnownMask(descriptors, count)) != 0)
    {
        return TH_MOD_RESULT_UNKNOWN_MODIFIER;
    }
    if (HasConflict(enabledMods, descriptors, count))
    {
        return TH_MOD_RESULT_CONFLICT;
    }
    return TH_MOD_RESULT_OK;
}

} // namespace registry
} // namespace th_mod
