#include "modifiers/nofail/NoFailPolicy.hpp"

namespace th_mod
{
namespace nofail
{

ThModNoFailDecisionV1 DecideMiss(const ThModRunConfigV1 &config,
                                 bool runActive,
                                 int32_t livesRemaining)
{
    ThModNoFailDecisionV1 decision = {};
    decision.struct_size = sizeof(decision);
    decision.consume_life = livesRemaining > 0 ? 1u : 0u;
    decision.continue_run = decision.consume_life;

    if (runActive &&
        (config.enabled_mods & TH_MOD_BUILTIN_NO_FAIL) != 0)
    {
        decision.continue_run = 1;
    }
    return decision;
}

} // namespace nofail
} // namespace th_mod
