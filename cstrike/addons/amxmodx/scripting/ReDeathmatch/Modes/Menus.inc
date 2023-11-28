static MENU_FLAG = ADMIN_MAP

ModesMenus_Init() {
    registerClCmd(
        "redm_modes_menu",
        "ClCmd_redm_modes_menu",
        .flags = MENU_FLAG,
        .info = "ReDM: Change game mode"
    )

    register_clcmd("say /modes", "ClCmd_redm_modes_menu", .flags = MENU_FLAG)
}

public ClCmd_redm_modes_menu(const player, const level, const commandId) {
    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED
    
    if (!IsActive())
        return PLUGIN_HANDLED

    Menu_Modes(player)

    return PLUGIN_HANDLED
}

static bool: Menu_Modes(const player) {
    new count
    new JSON: arrModes = RoundModes_GetModesArr(count)
    if (!count)
        return false

    new callback
    if (!callback)
        callback = menu_makecallback("MenuCallback_Modes")

    SetGlobalTransTarget(player)
    new menu = menu_create(fmt("%l", "Modes"), "MenuHandler_Modes")

    if (!RoundModes_IsModeForced()) {
        menu_additem(
            menu,
            fmt(
                "%l^n",
                "DisableForceMode"
                ),
            .callback = callback
        )
    } else {
        new modeName[64]
        RoundModes_GetModeInfo(RoundModes_GetCurrentMode(), modeName, charsmax(modeName))

        menu_additem(
            menu,
            fmt(
                HasLangKey(modeName) ? "%l^n  %l: \d%l\w^n" : "%l^n  %l: \d%s\w^n" ,
                "DisableForceMode",
                "Currently",
                modeName
            ),
            .callback = callback
        )
    }

    for (new idx; idx < count; idx++) {
        new JSON: objMode = json_array_get_value(arrModes, idx)
        
        new modeName[32]
        json_object_get_string(objMode, "name", modeName, charsmax(modeName))

        menu_additem(menu, fmt(HasLangKey(modeName) ? "%l" : "%s", modeName))

        json_free(objMode)
    }
    
    menu_setprop(menu, MPROP_BACKNAME, fmt("%l", "BACK"))
    menu_setprop(menu, MPROP_NEXTNAME, fmt("%l", "MORE"))
    menu_setprop(menu, MPROP_EXITNAME, fmt("%l", "EXIT"))
    menu_setprop(menu, MPROP_NUMBER_COLOR, "\y")

    menu_display(player, menu)

    return true
}

public MenuCallback_Modes(const player, const menu, const item) {
    // Disable only `force mode` item
    if (item != 0)
        return ITEM_IGNORE

    return RoundModes_IsModeForced() ? ITEM_IGNORE : ITEM_DISABLED
}

public MenuHandler_Modes(const player, const menu, const item) {
    menu_destroy(menu)
    
    if (!IsActive())
        return PLUGIN_HANDLED

    if(item < 0) {
        UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 30)

        return PLUGIN_HANDLED
    }

    new modeIdx = (item - 1)

    RoundModes_SetForcedMode(modeIdx)

    rg_round_end(
        CHalfLifeMultiplay_GetRoundRestartDelay(),
        WINSTATUS_DRAW,
        ROUND_END_DRAW,
        fmt("%L", LANG_SERVER, "ModeChanges"),
        .trigger = true
    )

    UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 80)

    return PLUGIN_HANDLED
}

Menu_GetVoteItemsCount() {
    new count
    RoundModes_GetModesArr(count)

    return count
}

bool: Menu_VoteMode(const player = 0) {
    new count
    new JSON: arrModes = RoundModes_GetModesArr(count)
    if (!count)
        return false

    new menu = menu_create("Choose", "MenuHandler_ChooseNextMode")

    static callback
    if (!callback)
        callback = menu_makecallback("MenuCallback_ChooseNextMode")

    for (new idx; idx < count; idx++) {
        new JSON: objMode = json_array_get_value(arrModes, idx)
        
        new modeName[32]
        json_object_get_string(objMode, "name", modeName, charsmax(modeName))

        menu_additem(menu, modeName, .callback = callback)

        json_free(objMode)
    }

    menu_setprop(menu, MPROP_BACKNAME, fmt("%l", "BACK"))
    menu_setprop(menu, MPROP_NEXTNAME, fmt("%l", "MORE"))
    menu_setprop(menu, MPROP_NUMBER_COLOR, "\y")
    menu_setprop(menu, MPROP_EXITNAME, fmt("%l", "EXIT"))
    menu_setprop(menu, MPROP_EXIT, MEXIT_NEVER)

    new players[MAX_PLAYERS], playersCount
    get_players(players, playersCount, "ch")

    for (new idx; idx < playersCount; idx++) {
        new p = player ? player : players[idx]

        SetGlobalTransTarget(p)

        menu_setprop(menu, MPROP_TITLE, fmt("%l %l", "Choose", "NextGameMode"))
        menu_display(p, menu, .time = 1)

        if (player)
            break
    }

    return true
}

public MenuCallback_ChooseNextMode(const player, const menu, const item) {
    new modeName[32]
    menu_item_getinfo(
        menu, item,
        .name = modeName,
        .namelen = charsmax(modeName)
    )

    SetGlobalTransTarget(player)

    new buffer[256]

    new bool: playerInVote = Vote_IsPlayerParticipatingInVote(player)

    strcat(buffer, fmt(!playerInVote ? "\d" : ""), charsmax(buffer))
    strcat(buffer, fmt(HasLangKey(modeName) ? "%l" : "%s", modeName), charsmax(buffer))

    new bool: isCurrentModeItem = (item == RoundModes_GetCurrentMode())
    strcat(buffer, fmt(isCurrentModeItem ? " \d(%l)" : "", "Extend"), charsmax(buffer))

    new votesCount = Vote_GetItemVotesCount(item)
    new playerVote = Vote_PlayerGetVote(player)

    strcat(buffer, (item == playerVote) ? "\r" : "\w", charsmax(buffer))
    
    new playersCount = Vote_GetPlayersCount()
    new Float: votedPercent = (votesCount * 100.0) / playersCount

    strcat(buffer, fmt("\R%.0f%%", votedPercent), charsmax(buffer))

    menu_item_setname(menu, item, buffer)

    if (!Vote_CanExtendCurrentMode() && isCurrentModeItem)
        return ITEM_DISABLED

    new bool: playerHasVote = Vote_IsPlayerVoted(player)
    if(playerHasVote) {
        return item != playerVote ? ITEM_DISABLED : ITEM_IGNORE
    }

    return ITEM_IGNORE
}

public MenuHandler_ChooseNextMode(const player, const menu, const item) {
    menu_destroy(menu)

    if (!IsActive())
        return PLUGIN_HANDLED
    
    if (!ModeVote_IsVoteStarted())
        return PLUGIN_HANDLED
    
    new bool: playerInVote = Vote_IsPlayerParticipatingInVote(player)
    if (Vote_IsPlayerVoted(player) || !playerInVote)
        return PLUGIN_HANDLED

    if (item < 0) {
        UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 30)

        return PLUGIN_HANDLED
    }

    UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 80)

    Vote_PlayerVote(player, item)

    Menu_VoteMode(player)

    return PLUGIN_HANDLED
}
