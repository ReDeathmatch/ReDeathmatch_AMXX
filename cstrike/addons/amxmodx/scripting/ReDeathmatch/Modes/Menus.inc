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

public ClCmd_redm_modes_menu(const player, const level, const cid) {
    if (!cmd_access(player, level, cid, 1))
        return PLUGIN_HANDLED
    
    if (!IsActive())
        return PLUGIN_HANDLED

    Menu_Modes(player)

    return PLUGIN_HANDLED
}

static bool: Menu_Modes(const player) {
    SetGlobalTransTarget(player)

    if (!json_object_has_value(Config_GetCurrent(), "modes"))
        return false

    new JSON: arrModes = json_object_get_value(Config_GetCurrent(), "modes")

    new count = json_array_get_count(arrModes)
    if (!count) {
        json_free(arrModes)
        return false
    }

    new callback
    if (!callback)
        callback = menu_makecallback("MenuCallback_Modes")

    new menu = menu_create(fmt("%l", "Modes"), "MenuHandler_Modes")

    if (!IsModeForced()) {
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
        GetModeInfo(g_forcedModeIdx, modeName, charsmax(modeName))

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

    return IsModeForced() ? ITEM_IGNORE : ITEM_DISABLED
}

public MenuHandler_Modes(const player, const menu, const item) {
    if (!IsActive())
        return PLUGIN_HANDLED

    menu_destroy(menu)

    if(item < 0) {
        UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 30)

        return PLUGIN_HANDLED
    }

    new modeIdx = (item - 1)

    g_forcedModeIdx = modeIdx

    rg_round_end(
        CHalfLifeMultiplay__GetRoundRestartDelay(),
        WINSTATUS_DRAW,
        ROUND_END_DRAW,
        fmt("%L", LANG_SERVER, "ModeChanges"),
        .trigger = true
    )

    UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 80)

    return PLUGIN_HANDLED
}
