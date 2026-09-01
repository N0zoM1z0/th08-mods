#ifndef TH_MOD_MANIFEST_MANIFEST_V1_HPP
#define TH_MOD_MANIFEST_MANIFEST_V1_HPP

#include "ModApi.h"

#include <string>

namespace th_mod
{
namespace manifest
{

std::string BuildV1(const ThModRunConfigV1 &config);

} // namespace manifest
} // namespace th_mod

#endif
