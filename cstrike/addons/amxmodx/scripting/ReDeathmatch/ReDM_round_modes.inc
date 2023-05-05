static g_currentRoundModeIdx = -1
static redm_modes_switch[32]

RoundModes_Init() {
    register_dictionary("redm/modes.txt")

    RegisterHookChain(RG_RoundEnd, "RoundEnd", .post = false)
    RegisterHookChain(RG_CSGameRules_RestartRound, "CSGameRules_RestartRound", .post = false)
    RegisterHookChain(RG_CBasePlayer_OnSpawnEquip, "CBasePlayer_OnSpawnEquip", .post = false)

    bind_pcvar_string(
        create_cvar("redm_modes_switch", "sequentially"),
        redm_modes_switch,
        charsmax(redm_modes_switch)
    )
}

public RoundEnd(WinStatus: status, ScenarioEventEndRound: event, Float: tmDelay) {
    if (strcmp(redm_modes_switch, "disable") == 0)
        return

    new nextModeIdx = GetNextMode()
    if (nextModeIdx == -1)
        return

    // TODO:
    // SetHookChainArg(1, ATYPE_INTEGER, WINSTATUS_NONE)
    // SetHookChainArg(2, ATYPE_INTEGER, ROUND_TARGET_SAVED)

    new modeName[32]
    GetModeInfo(nextModeIdx, modeName, charsmax(modeName))

    client_print_color(0, print_team_red,
        HasLangKey(modeName) ? "[Re:DM] %l ^3%l^1" : "[Re:DM] %l ^3%s^1",
        "NextMode",
        modeName
    )

    g_currentRoundModeIdx = nextModeIdx
}

public CSGameRules_RestartRound() {
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
    set_member(player, m_bNotKilled, false)
    rg_set_user_armor(player, 0, ARMOR_NONE)

    if (g_currentRoundModeIdx == -1)
        return

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
        HasLangKey(modeName) ? "%l: %l" : "%l: %s",
        "CurrentMode",
        modeName
    )
}


static GetModeInfo(const index, name[] = "", len = 0) {
    if (!json_object_has_value(Config_GetCurrent(), "modes"))
        return false

    new JSON: arrModes = json_object_get_value(Config_GetCurrent(), "modes")

    new count = json_array_get_count(arrModes)
    if (!count) {
        json_free(arrModes)
        return false
    }
    
    new JSON: objMode = json_array_get_value(arrModes, index)

    if (index < 0 || index >= count) {
        json_free(arrModes)
        json_free(objMode)
        return false
    }

    json_object_get_string(objMode, "name", name, len)

    json_free(arrModes)
    json_free(objMode)

    return true
}

static GetNextMode() {
    if (!json_object_has_value(Config_GetCurrent(), "modes"))
        return -1

    new JSON: arrModes = json_object_get_value(Config_GetCurrent(), "modes")

    new count = json_array_get_count(arrModes)
    if (!count) {
        json_free(arrModes)
        return -1
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
    if (index < 0)
        return false

    ReloadConfig()

    if (!json_object_has_value(Config_GetCurrent(), "modes"))
        return false

    new JSON: arrModes = json_object_get_value(Config_GetCurrent(), "modes")

    new count = json_array_get_count(arrModes)
    if (!count) {
        json_free(arrModes)
        return false
    }

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
    return g_currentRoundModeIdx    
}

RoundModes_ResetCurrentMode() {
    g_currentRoundModeIdx = -1    
}
