#include "mod/modifiers/flashlight/th08/Th08Flashlight.hpp"

#include "AnmManager.hpp"
#include "GameManager.hpp"
#include "Player.hpp"
#include "ScreenEffect.hpp"
#include "mod/api/ModApi.h"

namespace th08
{
#ifdef TH08_MODERN_WEB
extern AnmLoaded *&g_AsciiManagerDemoAnm0577EB4;
#else
DIFFABLE_EXTERN(AnmLoaded *, g_AsciiManagerDemoAnm0577EB4);
#endif

namespace mods
{
namespace flashlight
{
namespace
{

AnmVm g_maskVm;

void DrawSquare(f32 left, f32 top, f32 right, f32 bottom,
                D3DCOLOR color)
{
    if (right <= left || bottom <= top)
    {
        return;
    }

    ZunRect rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    ScreenEffect::DrawSquare(&rect, color);
}

} // namespace

void DrawOverlay()
{
    if (!g_GameManager.flags.unk2)
    {
        return;
    }

    ThModFlashlightStateV1 state;
    if (th_mod_get_flashlight_state_v1(&state) != TH_MOD_RESULT_OK ||
        !state.is_active || state.opacity == 0 ||
        g_AsciiManagerDemoAnm0577EB4 == NULL)
    {
        return;
    }

    const f32 playfieldLeft = g_GameManager.arcadeRegionTopLeftPos.x;
    const f32 playfieldTop = g_GameManager.arcadeRegionTopLeftPos.y;
    const f32 playfieldRight =
        playfieldLeft + g_GameManager.arcadeRegionSize.x;
    const f32 playfieldBottom =
        playfieldTop + g_GameManager.arcadeRegionSize.y;
    const f32 radius = static_cast<f32>(state.radius_pixels);
    const f32 centerX = playfieldLeft + g_Player.position.x +
                        g_AnmManager->screenShakeOffset.x;
    const f32 centerY = playfieldTop + g_Player.position.y +
                        g_AnmManager->screenShakeOffset.y;
    const D3DCOLOR color = state.opacity << 24;

    DrawSquare(playfieldLeft, playfieldTop, centerX - radius,
               playfieldBottom, color);
    DrawSquare(centerX + radius, playfieldTop, playfieldRight,
               playfieldBottom, color);

    f32 innerLeft = centerX - radius;
    if (innerLeft < playfieldLeft)
    {
        innerLeft = playfieldLeft;
    }
    f32 innerRight = centerX + radius;
    if (innerRight > playfieldRight)
    {
        innerRight = playfieldRight;
    }
    DrawSquare(innerLeft, playfieldTop, innerRight, centerY - radius, color);
    DrawSquare(innerLeft, centerY + radius, innerRight, playfieldBottom, color);

    // Script 105 supplies the feathered circular edge used by TH08's native
    // Stage 2 darkness. A separate VM lets both overlays alpha-compose without
    // overwriting the native effect's presentation state.
    g_AsciiManagerDemoAnm0577EB4->SetAndExecuteScriptIdx(&g_maskVm, 105);
    g_maskVm.scale.x = radius / 63.0f;
    g_maskVm.scale.y = g_maskVm.scale.x;
    g_maskVm.pos = g_Player.position;
    g_maskVm.pos.x += playfieldLeft;
    g_maskVm.pos.y += playfieldTop;
    g_maskVm.color1.a = static_cast<u8>(state.opacity);
    g_AnmManager->DrawNoRotation(&g_maskVm);
}

} // namespace flashlight
} // namespace mods
} // namespace th08
