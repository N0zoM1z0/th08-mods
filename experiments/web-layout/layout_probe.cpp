#include <stdint.h>
#include <stdio.h>

namespace
{
volatile uint32_t g_ordinaryGlobal = 0x13579bdf;
}

int main()
{
    // This address is representative of the low target-owned data arena used
    // by the reconstructed source. The Web build places ordinary Wasm globals
    // above 32 MiB so that these raw 32-bit accesses do not collide with them.
    volatile uint32_t *targetSlot = reinterpret_cast<volatile uint32_t *>(0x004c6c3c);
    *targetSlot = 0x2468ace0;

    uintptr_t ordinaryAddress = reinterpret_cast<uintptr_t>(&g_ordinaryGlobal);
    printf("target-slot=0x%08lx value=0x%08lx ordinary-global=0x%08lx\n",
           static_cast<unsigned long>(reinterpret_cast<uintptr_t>(targetSlot)),
           static_cast<unsigned long>(*targetSlot),
           static_cast<unsigned long>(ordinaryAddress));

    if (ordinaryAddress < 0x02000000 || *targetSlot != 0x2468ace0)
    {
        fprintf(stderr, "reserved target arena overlaps ordinary Wasm data\n");
        return 1;
    }
    return 0;
}
