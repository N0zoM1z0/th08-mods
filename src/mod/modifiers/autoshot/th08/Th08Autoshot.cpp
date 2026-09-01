#include "mod/modifiers/autoshot/th08/Th08Autoshot.hpp"

#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include "mod/api/ModApi.h"

namespace th08
{
extern u16 g_GuiMessageInputCurrent;

namespace mods
{
namespace autoshot
{
namespace
{

ChainElem *g_inputHook;

ChainCallbackResult FilterInput(void *)
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

    uint32_t actions = 0;
    if ((g_GuiMessageInputCurrent & TH_BUTTON_SHOOT) != 0)
    {
        actions |= TH_MOD_ACTION_SHOOT;
    }

    const uint32_t filtered = th_mod_filter_actions_v1(actions, context);
    if ((filtered & TH_MOD_ACTION_SHOOT) != 0)
    {
        // Player consumes the GUI snapshot. The replay recorder later consumes
        // g_CurFrameInput, so both receive the same effective action. Playback
        // is excluded above to prevent a second transformation.
        g_GuiMessageInputCurrent |= TH_BUTTON_SHOOT;
        g_CurFrameInput |= TH_BUTTON_SHOOT;
    }

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

} // namespace autoshot
} // namespace mods
} // namespace th08
