#include "mod/games/th08/Th08InputAdapter.hpp"

#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "mod/api/ModApi.h"

namespace th08
{
extern u16 g_GuiMessageInputCurrent;

namespace mods
{
namespace input
{
namespace
{

ChainElem *g_inputHook;

uint32_t BuildContext()
{
    uint32_t context = 0;
    if (g_GameManager.flags.unk2)
    {
        context |= TH_MOD_INPUT_CONTEXT_GAMEPLAY;
    }
    if (g_GameManager.flags.isReplay)
    {
        context |= TH_MOD_INPUT_CONTEXT_REPLAY_PLAYBACK;
    }
    if ((context & TH_MOD_INPUT_CONTEXT_GAMEPLAY) != 0 &&
        (g_GameManager.isInGameMenu || g_GameManager.showRetryMenu ||
         g_Gui.IsDialogPresent()))
    {
        context |= TH_MOD_INPUT_CONTEXT_UI_BLOCKED;
    }
    return context;
}

uint32_t ToPortableActions(u16 nativeInput)
{
    uint32_t actions = 0;
    if ((nativeInput & TH_BUTTON_SHOOT) != 0)
        actions |= TH_MOD_ACTION_SHOOT;
    if ((nativeInput & TH_BUTTON_BOMB) != 0)
        actions |= TH_MOD_ACTION_BOMB;
    if ((nativeInput & TH_BUTTON_FOCUS) != 0)
        actions |= TH_MOD_ACTION_FOCUS;
    if ((nativeInput & TH_BUTTON_MENU) != 0)
        actions |= TH_MOD_ACTION_MENU;
    if ((nativeInput & TH_BUTTON_UP) != 0)
        actions |= TH_MOD_ACTION_UP;
    if ((nativeInput & TH_BUTTON_DOWN) != 0)
        actions |= TH_MOD_ACTION_DOWN;
    if ((nativeInput & TH_BUTTON_LEFT) != 0)
        actions |= TH_MOD_ACTION_LEFT;
    if ((nativeInput & TH_BUTTON_RIGHT) != 0)
        actions |= TH_MOD_ACTION_RIGHT;
    return actions;
}

u16 ToNativeActions(uint32_t actions)
{
    u16 nativeInput = 0;
    if ((actions & TH_MOD_ACTION_SHOOT) != 0)
        nativeInput |= TH_BUTTON_SHOOT;
    if ((actions & TH_MOD_ACTION_BOMB) != 0)
        nativeInput |= TH_BUTTON_BOMB;
    if ((actions & TH_MOD_ACTION_FOCUS) != 0)
        nativeInput |= TH_BUTTON_FOCUS;
    if ((actions & TH_MOD_ACTION_MENU) != 0)
        nativeInput |= TH_BUTTON_MENU;
    if ((actions & TH_MOD_ACTION_UP) != 0)
        nativeInput |= TH_BUTTON_UP;
    if ((actions & TH_MOD_ACTION_DOWN) != 0)
        nativeInput |= TH_BUTTON_DOWN;
    if ((actions & TH_MOD_ACTION_LEFT) != 0)
        nativeInput |= TH_BUTTON_LEFT;
    if ((actions & TH_MOD_ACTION_RIGHT) != 0)
        nativeInput |= TH_BUTTON_RIGHT;
    return nativeInput;
}

void ReplaceGameplayActions(u16 &nativeInput, uint32_t actions)
{
    const u16 actionMask = TH_BUTTON_SHOOT | TH_BUTTON_BOMB |
                           TH_BUTTON_FOCUS | TH_BUTTON_MENU |
                           TH_BUTTON_UP | TH_BUTTON_DOWN |
                           TH_BUTTON_LEFT | TH_BUTTON_RIGHT;
    nativeInput = static_cast<u16>((nativeInput & ~actionMask) |
                                   ToNativeActions(actions));
}

ChainCallbackResult FilterInput(void *)
{
    const uint32_t actions = ToPortableActions(g_GuiMessageInputCurrent);
    const uint32_t filtered =
        th_mod_filter_actions_v1(actions, BuildContext());

    // Player consumes the GUI snapshot. The replay recorder later consumes
    // g_CurFrameInput, so both receive the same effective actions. Playback
    // is excluded by the portable policy to prevent a second transformation.
    ReplaceGameplayActions(g_GuiMessageInputCurrent, filtered);
    ReplaceGameplayActions(g_CurFrameInput, filtered);
    return CHAIN_CALLBACK_RESULT_CONTINUE;
}

ZunResult InputHookDeleted(void *)
{
    g_inputHook = NULL;
    return ZUN_SUCCESS;
}

} // namespace

bool RegisterHook()
{
    if (g_inputHook != NULL)
    {
        return true;
    }

    g_inputHook = g_Chain.CreateElem(FilterInput);
    if (g_inputHook == NULL)
    {
        return false;
    }
    g_inputHook->deletedCallback = InputHookDeleted;

    // Replay playback is priority 6, RNG synchronization is 7, and Player is
    // 9. Priority 8 sees replay input before Player and precedes recording.
    if (g_Chain.AddToCalcChain(g_inputHook, 8) != 0)
    {
        g_Chain.Cut(g_inputHook);
        g_inputHook = NULL;
        return false;
    }
    return true;
}

} // namespace input
} // namespace mods
} // namespace th08
