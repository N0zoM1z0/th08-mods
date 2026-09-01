#include "games/th08/Th08DifficultyAdapterTests.hpp"

#include "ModApi.h"
#include "TestHarness.hpp"
#include "mod/games/th08/Th08DifficultyAdapter.hpp"

#include <cmath>

namespace
{

void CheckNear(float actual, float expected)
{
    CHECK(std::fabs(actual - expected) < 0.0001f);
}

void ConfigureAndBegin(uint32_t modifier)
{
    ResetRuntime();
    ThModRunConfigV1 config;
    CHECK(th_mod_get_default_config_v1(&config) == TH_MOD_RESULT_OK);
    config.enabled_mods = modifier;
    CHECK(th_mod_configure_v1(&config) == TH_MOD_RESULT_OK);
    CHECK(th_mod_begin_run() == TH_MOD_RESULT_OK);
}

} // namespace

void TestTh08DifficultyAdapter()
{
    ResetRuntime();
    CheckNear(th_mod::th08::ScalePlayerSpeed(4.0f), 4.0f);
    CheckNear(th_mod::th08::ScalePlayerHitbox(2.0f), 2.0f);
    CheckNear(th_mod::th08::ScalePlayerGraze(16.0f), 16.0f);
    CheckNear(th_mod::th08::ScaleLaserGrazeMargin(48.0f), 48.0f);

    ConfigureAndBegin(TH_MOD_BUILTIN_HARD_ROCK);
    CheckNear(th_mod::th08::ScalePlayerSpeed(4.0f), 3.6f);
    CheckNear(th_mod::th08::ScalePlayerHitbox(2.0f), 2.5f);
    CheckNear(th_mod::th08::ScalePlayerGraze(20.0f), 16.0f);
    CheckNear(th_mod::th08::ScaleLaserGrazeMargin(48.0f), 38.4f);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    ConfigureAndBegin(TH_MOD_BUILTIN_EASY);
    CheckNear(th_mod::th08::ScalePlayerSpeed(4.0f), 4.0f);
    CheckNear(th_mod::th08::ScalePlayerHitbox(4.0f), 3.0f);
    CheckNear(th_mod::th08::ScalePlayerGraze(16.0f), 20.0f);
    CheckNear(th_mod::th08::ScaleLaserGrazeMargin(48.0f), 60.0f);

    CHECK(th_mod_end_run() == TH_MOD_RESULT_OK);
    ResetRuntime();
}
