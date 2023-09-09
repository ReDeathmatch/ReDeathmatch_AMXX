static g_forcedModeIdx = -1
static g_currentRoundModeIdx = -1
static redm_modes_switch[32]

#include "ReDeathmatch/Modes/Menus.inc"
#include "ReDeathmatch/Modes/Vote.inc"

RoundModes_Init() {
    register_dictionary("redm/modes.txt")

    RegisterHookChain(RG_RoundEnd, "RoundEnd_Post", .post = true)
    RegisterHookChain(RG_CSGameRules_RestartRound, "CSGameRules_RestartRound", .post = false)
    RegisterHookChain(RG_CBasePlayer_OnSpawnEquip, "CBasePlayer_OnSpawnEquip", .post = false)

    bind_pcvar_string(
        create_cvar(
            "redm_modes_switch", "sequentially",
            .description = "Game mode change mode. \
                `0`, `disable` - disable modes, \
                `random` - random change, \
                `sequentially` - sequential mode change."
        ),
        redm_modes_switch,
        charsmax(redm_modes_switch)
    )

    ModesMenus_Init()
    ModeVote_Init()
}

public RoundEnd_Post(WinStatus: status, ScenarioEventEndRound: event, Float: tmDelay) {
    if (!IsActive())
        return

    new bool: voteStarted = ModeVote_RoundEnd_Post()

    if (voteStarted) {
        client_print(0, print_center, "%l %l.", "Selecting", "NextGameMode")

        return
    }

    if (strcmp(redm_modes_switch, "disable") == 0)
        return

    new nextModeIdx = GetNextMode()
    if (nextModeIdx == -1)
        return

    g_currentRoundModeIdx = nextModeIdx

    new modeName[32]
    GetModeInfo(g_currentRoundModeIdx, modeName, charsmax(modeName))

    client_print_color(0, print_team_red,
        HasLangKey(modeName) ? "[Re:DM] %l ^3%l^1" : "[Re:DM] %l ^3%s^1",
        "NextMode",
        modeName
    )

    // TODO: better will be hook EndRoundMessage()
    client_print(0, print_center, "%l", "ModeChanges")
}

public CSGameRules_RestartRound() {
    if (!IsActive())
        return

    new votedModeIdx = ModeVote_RestartRound()
    if (votedModeIdx >= 0)
        g_currentRoundModeIdx = votedModeIdx

    if (g_currentRoundModeIdx == -1)
        return

    if (!GetModeInfo(g_currentRoundModeIdx))
        return

    ApplyMode(g_currentRoundModeIdx)

    for (new p = 1; p <= MaxClients; p++) {
        EquipManager_PlayerResetEquip(p)
    }
}

public CBasePlayer_OnSpawnEquip(const player, bool: addDefault, bool: equipGame) {
    if (!IsActive())
        return

    set_member(player, m_bNotKilled, false)
    rg_set_user_armor(player, 0, ARMOR_NONE)

    if (g_currentRoundModeIdx == -1)
        return

    set_task(0.3, "RoundInfoMessage", player)
}

public RoundInfoMessage(const player) {
    new modeName[32]
    GetModeInfo(g_currentRoundModeIdx, modeName, charsmax(modeName))

    set_dhudmessage(
        .red = 200,
        .green = 200,
        .blue = 200,
        .y = 0.85,
        .holdtime = 4.0
    )

    SetGlobalTransTarget(player)

    show_dhudmessage(
        player,
        HasLangKey(modeName) ? "%l %l" : "%l %s",
        "CurrentMode",
        modeName
    )
}

JSON: GetModesArr(& count = 0) {
    if (!json_object_has_value(Config_GetCurrent(), "modes"))
        return Invalid_JSON

    new JSON: arrModes = json_object_get_value(Config_GetCurrent(), "modes")

    count = json_array_get_count(arrModes)
    if (!count) {
        json_free(arrModes)

        return Invalid_JSON
    }
    
    return arrModes
}

bool: GetModeInfo(const index, name[] = "", len = 0) {
    new count
    new JSON: arrModes = GetModesArr(count)
    if (arrModes == Invalid_JSON)
        return false

    if (index < 0 || index >= count) {
        json_free(arrModes)

        return false
    }

    new JSON: objMode = json_array_get_value(arrModes, index)
    json_object_get_string(objMode, "name", name, len)

    json_free(arrModes)
    json_free(objMode)

    return true
}

static GetNextMode() {
    new count
    new JSON: arrModes = GetModesArr(count)
    if (arrModes == Invalid_JSON)
        return -1

    if (IsModeForced()) {
        json_free(arrModes)

        return g_forcedModeIdx
    }

    new currentIdx = g_currentRoundModeIdx

    if (strcmp(redm_modes_switch, "random") == 0) {
        while (currentIdx == g_currentRoundModeIdx) {
            currentIdx = random_num(0, count - 1)
        }
    } else if (strcmp(redm_modes_switch, "sequentially") == 0) {
        ++currentIdx
        currentIdx %= count
    }

    return currentIdx
}


bool: ApplyMode(const index) {
    if (!IsActive())
        return false

    if (index < 0)
        return false

    ReloadConfig()

    new JSON: arrModes = GetModesArr()
    if (arrModes == Invalid_JSON)
        return false

    new JSON: objMode = json_array_get_value(arrModes, index)
    if (json_object_has_value(objMode, "cvars")) {
        new JSON: objCvars = json_object_get_value(objMode, "cvars")

        CvarsHandler_LoadCvars(objCvars)

        json_free(objCvars)
    }

    json_free(objMode)
    json_free(arrModes)

    return true
}

RoundModes_GetCurrentMode() {
    return g_forcedModeIdx > -1 ? g_forcedModeIdx : g_currentRoundModeIdx    
}

RoundModes_SetCurrentMode(const modeIdx) {
    g_currentRoundModeIdx = modeIdx
}

RoundModes_ResetCurrentMode() {
    g_currentRoundModeIdx = -1    
}

bool: IsModeForced() {
    // TODO: find better
    return g_forcedModeIdx > -1
}

Float: CHalfLifeMultiplay__GetRoundRestartDelay() {
	return get_cvar_float("mp_round_restart_delay")
}
