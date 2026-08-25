#include "th_pch.h"

#include "EclManager.hpp"
#ifdef TH08_MODERN_WEB
#include "AsciiManager.hpp"
#include "Background.hpp"
#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "Spellcard.hpp"
#include "Supervisor.hpp"
#endif
#include "EclOperands.hpp"

namespace th08
{

namespace EclRunLowProposal
{
struct InterpolationSlot;
void __fastcall InterpolateLinear(EclOperands::EnemyOverlay *enemy, InterpolationSlot *slot, f32 t);
void __fastcall InterpolateHermite(EclOperands::EnemyOverlay *enemy, InterpolationSlot *slot, f32 t);

DIFFABLE_STATIC_ARRAY_ASSIGN(void *, 8, g_EclInterpolatorCallbacks) = {
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateLinear),
    reinterpret_cast<void *>(&InterpolateHermite),
};
} // namespace EclRunLowProposal

struct EclExInstruction;
namespace EclExIns
{
void __fastcall ReisenFreezeBullets(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall MokouResurrection(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
}

void __fastcall FUN_00423390(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004233d0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423400(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423530(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423a60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424130(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424170(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004241e0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004244f0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424730(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004246e0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424820(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424e00(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424c40(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424e50(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424f60(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424f90(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424fc0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423db0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00423e20(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425020(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425040(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424910(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425070(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004250d0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_004251b0(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425290(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00424a00(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);
void __fastcall FUN_00425390(EclOperands::EnemyOverlay *enemy, EclExInstruction *instruction);

DIFFABLE_STATIC_ARRAY_ASSIGN(void *, 32, g_EclExInsn) = {
    reinterpret_cast<void *>(&FUN_00423390),
    reinterpret_cast<void *>(&FUN_004233d0),
    reinterpret_cast<void *>(&FUN_00423400),
    reinterpret_cast<void *>(&FUN_00423530),
    reinterpret_cast<void *>(&FUN_00423a60),
    reinterpret_cast<void *>(&FUN_00424130),
    reinterpret_cast<void *>(&FUN_00424170),
    reinterpret_cast<void *>(&FUN_004241e0),
    reinterpret_cast<void *>(&FUN_004244f0),
    reinterpret_cast<void *>(&FUN_00424730),
    reinterpret_cast<void *>(&FUN_004246e0),
    reinterpret_cast<void *>(&FUN_00424820),
    reinterpret_cast<void *>(&EclExIns::ReisenFreezeBullets),
    reinterpret_cast<void *>(&FUN_00424e00),
    reinterpret_cast<void *>(&FUN_00424c40),
    reinterpret_cast<void *>(&FUN_00424e20),
    reinterpret_cast<void *>(&FUN_00424e50),
    reinterpret_cast<void *>(&FUN_00424f60),
    reinterpret_cast<void *>(&FUN_00424f90),
    reinterpret_cast<void *>(&FUN_00424fc0),
    reinterpret_cast<void *>(&FUN_00423db0),
    reinterpret_cast<void *>(&FUN_00423e20),
    reinterpret_cast<void *>(&EclExIns::MokouResurrection),
    reinterpret_cast<void *>(&FUN_00425020),
    reinterpret_cast<void *>(&FUN_00425040),
    reinterpret_cast<void *>(&FUN_00424910),
    reinterpret_cast<void *>(&FUN_00425070),
    reinterpret_cast<void *>(&FUN_004250d0),
    reinterpret_cast<void *>(&FUN_004251b0),
    reinterpret_cast<void *>(&FUN_00425290),
    reinterpret_cast<void *>(&FUN_00424a00),
    reinterpret_cast<void *>(&FUN_00425390),
};

#ifdef TH08_MODERN_WEB
// The retail linker gives these names overlapping views into their aggregate
// owners. WebAssembly globals are relocatable, so model those views directly.
i32 &g_EclCallbackPublishedEnemyField24 = g_AsciiManager.unk_16f08;
i32 &g_EclCallbackPublishedEnemyField56 =
    *reinterpret_cast<i32 *>(&g_AsciiManager.unk_16f04);
i32 &g_EclGlobal004EA290 = g_Background.unk6260;
i32 &g_EclGlobal004ECCA8 = g_Spellcard.scoreLimit;
i8 &g_EclScriptedGlobalUpdateFreeze = g_GameManager.unk2C;
f32 &g_EclGameTimeScale = g_Supervisor.framerateMultiplier;
u32 &g_EclGameTimeScaleFlags = *reinterpret_cast<u32 *>(&g_Supervisor.flags);
#else
DIFFABLE_STATIC(i32, g_EclCallbackPublishedEnemyField24);
DIFFABLE_STATIC(i32, g_EclCallbackPublishedEnemyField56);
DIFFABLE_STATIC(i32, g_EclGlobal004EA290);
DIFFABLE_STATIC(i32, g_EclGlobal004ECCA8);
DIFFABLE_STATIC(i8, g_EclScriptedGlobalUpdateFreeze);
DIFFABLE_STATIC(f32, g_EclGameTimeScale);
DIFFABLE_STATIC(u32, g_EclGameTimeScaleFlags);
#endif
DIFFABLE_STATIC(EclManager, g_EclManager);

namespace EclRunLowProposal
{
DIFFABLE_STATIC(EclCallParameterCopy, g_EclCallParameters);
#ifdef TH08_MODERN_WEB
EclOperands::EnemyOverlay *(&g_EclEnemyTableF54CC0)[92] =
    *reinterpret_cast<EclOperands::EnemyOverlay *(*)[92]>(
        reinterpret_cast<u8 *>(&g_EnemyManager) + 0x9DCDA0);
#else
DIFFABLE_STATIC_ARRAY(EclOperands::EnemyOverlay *, 92, g_EclEnemyTableF54CC0);
#endif
} // namespace EclRunLowProposal

} // namespace th08
