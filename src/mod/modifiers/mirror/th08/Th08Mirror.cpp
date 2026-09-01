#include "mod/modifiers/mirror/th08/Th08Mirror.hpp"

#include "GameManager.hpp"
#include "Global.hpp"
#include "ScreenEffect.hpp"
#include "mod/api/ModApi.h"
#include "mod/modifiers/mirror/MirrorGeometry.hpp"
#include "modern/linux/d3d8_internal.hpp"

namespace th08
{
namespace mods
{
namespace mirror
{
namespace
{

ChainElem *g_resetHook;
ChainElem *g_beginHook;
ChainElem *g_endHook;
bool g_transformedThisFrame;
ThModMirrorStateV1 g_state;
ZunRect g_playfield;

void DisableTransform()
{
    th08_linux_set_playfield_transform(
        false, TH_MOD_MIRROR_HORIZONTAL, 0.0f, 0.0f, 0.0f, 0.0f);
}

ChainCallbackResult ResetTransform(void *)
{
    DisableTransform();
    g_transformedThisFrame = false;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ChainCallbackResult BeginTransform(void *)
{
    g_transformedThisFrame = false;
    if (!g_GameManager.flags.unk2 ||
        th_mod_get_mirror_state_v1(&g_state) != TH_MOD_RESULT_OK ||
        !g_state.is_active)
    {
        DisableTransform();
        return CHAIN_CALLBACK_RESULT_CONTINUE;
    }

    g_playfield.left = g_GameManager.arcadeRegionTopLeftPos.x;
    g_playfield.top = g_GameManager.arcadeRegionTopLeftPos.y;
    g_playfield.right = g_playfield.left + g_GameManager.arcadeRegionSize.x;
    g_playfield.bottom = g_playfield.top + g_GameManager.arcadeRegionSize.y;
    th08_linux_set_playfield_transform(
        true, g_state.mode, g_playfield.left, g_playfield.top,
        g_GameManager.arcadeRegionSize.x, g_GameManager.arcadeRegionSize.y);
    g_transformedThisFrame = true;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

void DrawBlackSquare(float left, float top, float right, float bottom)
{
    if (right <= left || bottom <= top)
    {
        return;
    }
    ZunRect rectangle;
    rectangle.left = left;
    rectangle.top = top;
    rectangle.right = right;
    rectangle.bottom = bottom;
    ScreenEffect::DrawSquare(&rectangle, 0xff000000u);
}

void DrawQuarterTurnLetterbox()
{
    const float width = g_playfield.right - g_playfield.left;
    const float height = g_playfield.bottom - g_playfield.top;
    const float scale = th_mod::mirror::QuarterTurnScale(width, height);
    const float contentWidth = height * scale;
    const float contentHeight = width * scale;
    const float centerX = (g_playfield.left + g_playfield.right) * 0.5f;
    const float centerY = (g_playfield.top + g_playfield.bottom) * 0.5f;
    const float innerLeft = centerX - contentWidth * 0.5f;
    const float innerTop = centerY - contentHeight * 0.5f;
    const float innerRight = centerX + contentWidth * 0.5f;
    const float innerBottom = centerY + contentHeight * 0.5f;

    DrawBlackSquare(g_playfield.left, g_playfield.top,
                    innerLeft, g_playfield.bottom);
    DrawBlackSquare(innerRight, g_playfield.top,
                    g_playfield.right, g_playfield.bottom);
    DrawBlackSquare(innerLeft, g_playfield.top, innerRight, innerTop);
    DrawBlackSquare(innerLeft, innerBottom, innerRight, g_playfield.bottom);
}

ChainCallbackResult EndTransform(void *)
{
    DisableTransform();
    if (g_transformedThisFrame &&
        (g_state.mode == TH_MOD_MIRROR_ROTATE_90 ||
         g_state.mode == TH_MOD_MIRROR_ROTATE_270))
    {
        DrawQuarterTurnLetterbox();
    }
    g_transformedThisFrame = false;
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult HookDeleted(void *slot)
{
    *static_cast<ChainElem **>(slot) = NULL;
    DisableTransform();
    g_transformedThisFrame = false;
    return ZUN_SUCCESS;
}

bool AddDrawHook(ChainElem **slot, ChainCallback callback, int priority)
{
    *slot = g_Chain.CreateElem(callback);
    if (*slot == NULL)
    {
        return false;
    }
    (*slot)->arg = slot;
    (*slot)->deletedCallback = HookDeleted;
    if (g_Chain.AddToDrawChain(*slot, priority) != ZUN_SUCCESS)
    {
        g_Chain.Cut(*slot);
        *slot = NULL;
        return false;
    }
    return true;
}

void RemoveHook(ChainElem **slot)
{
    if (*slot != NULL)
    {
        g_Chain.Cut(*slot);
        *slot = NULL;
    }
}

} // namespace

bool RegisterHooks()
{
    if (g_resetHook != NULL && g_beginHook != NULL && g_endHook != NULL)
    {
        return true;
    }

    // Reset protects the renderer if a previous draw chain stopped early.
    // Registering before gameplay objects makes the priority-6 begin sentinel
    // run before Background, while priority 15 ends before Spellcard and HUD.
    if (!AddDrawHook(&g_resetHook, ResetTransform, 3) ||
        !AddDrawHook(&g_beginHook, BeginTransform, 6) ||
        !AddDrawHook(&g_endHook, EndTransform, 15))
    {
        RemoveHook(&g_resetHook);
        RemoveHook(&g_beginHook);
        RemoveHook(&g_endHook);
        return false;
    }
    return true;
}

} // namespace mirror
} // namespace mods
} // namespace th08
