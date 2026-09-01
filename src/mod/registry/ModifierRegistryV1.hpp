#ifndef TH_MOD_REGISTRY_MODIFIER_REGISTRY_V1_HPP
#define TH_MOD_REGISTRY_MODIFIER_REGISTRY_V1_HPP

#include "ModApi.h"

#include <cstddef>
#include <cstdint>

namespace th_mod
{
namespace registry
{

struct ModifierDescriptorV1
{
    uint32_t flag;
    const char *code;
    uint32_t ruleset_version;
    uint32_t conflicts;
};

const ModifierDescriptorV1 *BuiltinsV1();
std::size_t BuiltinCountV1();

uint32_t KnownMask(const ModifierDescriptorV1 *descriptors,
                   std::size_t count);
bool HasConflict(uint32_t enabledMods,
                 const ModifierDescriptorV1 *descriptors,
                 std::size_t count);
ThModResult ValidateSelectionV1(uint32_t enabledMods);

} // namespace registry
} // namespace th_mod

#endif
