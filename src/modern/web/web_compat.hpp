#pragma once

#ifndef TH08_MODERN_WEB
#error "This header is only for the modern Web build."
#endif

// VC7 and the desktop GCC build expose the C allocation declarations through
// transitive headers. Clang's wasm32 sysroot does not, so make that platform
// dependency explicit at the forced-include boundary.
#include <stdlib.h>

#include "modern/linux/linux_compat.hpp"
