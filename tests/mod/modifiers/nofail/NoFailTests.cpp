#include "modifiers/nofail/NoFailTests.hpp"

#include "ModApi.h"
#include "TestHarness.hpp"

namespace
{

ThModNoFailDecisionV1 Decide(int32_t livesRemaining)
{
    ThModNoFailDecisionV1 decision;
    CHECK(th_mod_nofail_decide_miss_v1(livesRemaining, &decision) ==
          TH_MOD_RESULT_OK);
    CHECK(decision.struct_size == sizeof(decision));
    CHECK(decision.reserved == 0);
    return decision;
}

void CheckDecision(int32_t livesRemaining,
                   uint32_t continueRun,
                   uint32_t consumeLife)
{
    const ThModNoFailDecisionV1 decision = Decide(livesRemaining);
    CHECK(decision.continue_run == continueRun);
    CHECK(decision.consume_life == consumeLife);
}

} // namespace

void TestNoFailPolicy()
{
    ResetRuntime();
    CHECK(th_mod_nofail_decide_miss_v1(0, 0) ==
          TH_MOD_RESULT_NULL_ARGUMENT);

    CheckDecision(3, 1, 1);
    CheckDecision(1, 1, 1);
    CheckDecision(0, 0, 0);
    CheckDecision(-1, 0, 0);

    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = TH_MOD_BUILTIN_NO_FAIL;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);

    CheckDecision(0, 0, 0);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
    CheckDecision(3, 1, 1);
    CheckDecision(1, 1, 1);
    CheckDecision(0, 1, 0);
    CheckDecision(-1, 1, 0);
    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    CheckDecision(0, 0, 0);

    ResetRuntime();
}
