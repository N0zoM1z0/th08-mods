#include "mod/modifiers/nofail/th08/Th08NoFail.hpp"

#include "GameManager.hpp"
#include "mod/api/ModApi.h"

namespace th08
{
namespace mods
{
namespace nofail
{
namespace
{

ThModNoFailDecisionV1 DecideMiss()
{
    const i32 livesRemaining = g_GameManager.GetLives();
    ThModNoFailDecisionV1 decision;
    decision.struct_size = sizeof(decision);
    decision.continue_run = livesRemaining > 0 ? 1u : 0u;
    decision.consume_life = decision.continue_run;
    decision.reserved = 0;

    // Keep vanilla behavior as a safe fallback if the portable runtime ever
    // rejects the query. Runtime initialization normally makes this path
    // unreachable.
    th_mod_nofail_decide_miss_v1(livesRemaining, &decision);
    return decision;
}

} // namespace

bool ShouldContinueAfterMiss()
{
    return DecideMiss().continue_run != 0;
}

void ConsumeLifeIfAvailable()
{
    if (DecideMiss().consume_life != 0)
    {
        g_GameManager.AddLives(-1);
    }
}

} // namespace nofail
} // namespace mods
} // namespace th08
