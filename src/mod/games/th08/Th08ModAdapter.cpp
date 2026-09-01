#include "mod/games/th08/Th08ModAdapter.hpp"

#include "GameManager.hpp"
#include "Global.hpp"
#include "Gui.hpp"
#include <stdio.h>

namespace th08
{
extern u16 g_GuiMessageInputCurrent;

namespace mods
{
namespace
{

ThModRunConfigV1 g_runConfig;
bool g_initialized;
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
        // The player consumes the GUI input snapshot. The replay recorder later
        // consumes g_CurFrameInput, so update both with the same effective
        // action. Playback is excluded above to prevent double transforms.
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

bool Initialize()
{
    if (g_initialized)
    {
        return true;
    }

    ThModResult result = th_mod_get_config_v1(&g_runConfig);
    if (result == TH_MOD_RESULT_OK)
    {
        result = th_mod_validate_config_v1(&g_runConfig);
    }
    if (result == TH_MOD_RESULT_OK)
    {
        result = th_mod_begin_run();
    }
    if (result != TH_MOD_RESULT_OK)
    {
        fprintf(stderr, "th08-mod: runtime initialization failed (%d)\n",
                static_cast<int>(result));
        return false;
    }

    g_initialized = true;
    fprintf(stderr, "th08-mod: API v%lu initialized with mask 0x%08lx\n",
            static_cast<unsigned long>(g_runConfig.api_version),
            static_cast<unsigned long>(g_runConfig.enabled_mods));
    return true;
}

void Shutdown()
{
    if (!g_initialized)
    {
        return;
    }

    const ThModResult result = th_mod_end_run();
    if (result != TH_MOD_RESULT_OK)
    {
        fprintf(stderr, "th08-mod: runtime shutdown failed (%d)\n",
                static_cast<int>(result));
    }
    g_initialized = false;
}

bool RegisterHooks()
{
    if (!g_initialized)
    {
        return false;
    }
    if (g_inputHook != NULL || !IsEnabled(TH_MOD_BUILTIN_AUTOSHOT))
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
    // 9. Priority 8 therefore sees replay input and records one effective input
    // stream before Player consumes it.
    if (g_Chain.AddToCalcChain(g_inputHook, 8) != 0)
    {
        g_Chain.Cut(g_inputHook);
        g_inputHook = NULL;
        return false;
    }
    return true;
}

const ThModRunConfigV1 &GetRunConfig()
{
    return g_runConfig;
}

bool IsEnabled(ThModBuiltinV1 modifier)
{
    return (g_runConfig.enabled_mods & static_cast<uint32_t>(modifier)) != 0;
}

} // namespace mods
} // namespace th08
