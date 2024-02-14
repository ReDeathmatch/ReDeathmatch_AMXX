SettingsMenu_Init() {
    register_concmd(
        "redm_settings_menu",
        "ConCmd_redm_settings_menu",
        ADMIN_MAP,
        "Open ReDM settings menu."
    )

    register_clcmd(
        "enter_ChangeSettingValue",
        "ClCmd_ChangeSettingValue",
        ADMIN_MAP,
        .FlagManager = false
    )

    register_clcmd("radio1", "ConCmd_redm_settings_menu")
}

public ConCmd_redm_settings_menu(const player, const level, const commandId) {
    SetGlobalTransTarget(player)

    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED

    if (!IsActive())
        return PLUGIN_HANDLED

    Menu_Settings(player)
    return PLUGIN_HANDLED
}

public ClCmd_ChangeSettingValue(const player, const level, const commandId) {
    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED

    static newValue[128]
    read_argv(1, newValue, charsmax(newValue))

    if (!newValue[0]) {
        Menu_ChangeSettingsItem(player, .cvarIndex = strtol(cvarIndexStr))

        return PLUGIN_HANDLED
    }

    // client_print_color(
    //     player,
    //     print_team_red,
    //     "%L %L (%s)",
    //     id,
    //     "Gag_prefix",
    //     id,
    //     "Gag_YouSetManual_Reason",
    //     g_adminTempData[id][gd_reason][r_name]
    // )

    Menu_ChangeSettingsItem(player, .cvarIndex = strtol(cvarIndexStr))
    return PLUGIN_HANDLED
}
static bool: Menu_Settings(const player) {
    if (!is_user_connected(player))
        return false

    SetGlobalTransTarget(player)

    new menuTitle[128]
    formatex(menuTitle, charsmax(menuTitle), "%l", "ReDeathmatchSettingsMenu")

    new menu = menu_create(menuTitle, "MenuHandler_Settings")

    for (new cvarIndex; cvarIndex < 100; cvarIndex++) {
        new name[32]
        new flags
        new pluginId
        new cvarHandle
        new description[256]

        new bool: found = get_plugins_cvar(
            cvarIndex,
            name, charsmax(name),
            flags,
            pluginId,
            cvarHandle,
            description, charsmax(description)
        ) != 0

        if (!found)
            continue

        if (flags & FCVAR_PROTECTED)
            continue

        new const prefix[] = "redm_"
        if (strncmp(name, prefix, strlen(prefix), .ignorecase = true) != 0)
            continue

        new valueStr[32]
        if (flags & _FCVAR_FLOAT)
            formatex(valueStr, charsmax(valueStr), "%.2f", get_pcvar_float(cvarHandle))
        else 
            get_pcvar_string(cvarHandle, valueStr, charsmax(valueStr))

        menu_additem(
            menu, 
            fmt("%s: \y%s\w", name, valueStr),
            .info = fmt("%i", cvarIndex)
        )
    }

    menu_setprop(menu, MPROP_BACKNAME, fmt("%l", "BACK"))
    menu_setprop(menu, MPROP_NEXTNAME, fmt("%l", "MORE"))
    menu_setprop(menu, MPROP_EXITNAME, fmt("%l", "EXIT"))
    menu_setprop(menu, MPROP_NUMBER_COLOR, "\y")

    menu_display(player, menu)

    return true
}

public MenuHandler_Settings(const player, const menu, const item) {
    new info[3]
    menu_item_getinfo(
        menu, item,
        .info = info,
        .infolen = charsmax(info)
    )

    menu_destroy(menu)
    
    if (!IsActive())
        return PLUGIN_HANDLED

    if (item < 0)
        return PLUGIN_HANDLED

    Menu_ChangeSettingsItem(player, .cvarIndex = strtol(info))

    return PLUGIN_HANDLED
}

static bool: Menu_ChangeSettingsItem(const player, const cvarIndex) {
    new name[32], flags, cvarHandle, description[328]

    new bool: found = get_plugins_cvar(
        cvarIndex, 
        name, charsmax(name),
        flags,
        .pcvar_handle = cvarHandle,
        .description = description,
        .desc_len = charsmax(description)
    ) != 0

    if (!found)
        return false

    new valueStr[32]
    if (flags & _FCVAR_FLOAT)
        formatex(valueStr, charsmax(valueStr), "%.2f", get_pcvar_float(cvarHandle))
    else 
        get_pcvar_string(cvarHandle, valueStr, charsmax(valueStr))

    new bounds[CvarBounds][Bounds_s]
    for (new CvarBounds: boundType; boundType < CvarBounds; boundType++) {
        bounds[boundType][b_hasBound] = get_pcvar_bounds(cvarHandle, boundType, bounds[boundType][b_value])
    }

    SetGlobalTransTarget(player)

    new menuTitle[128]
    formatex(menuTitle, charsmax(menuTitle), "%l", "ReDeathmatchSettingsMenu")

    new menu = menu_create(menuTitle, "MenuHandler_SettingsItem")
    menu_additem(menu, fmt("Change value?"), .info = fmt("%i", cvarIndex))

    new buffer[256]
    formatex(buffer, charsmax(buffer), "^n\y%s\w: \r%s\w", name, valueStr)

    strcat(buffer,
        fmt("^n^nType: \r%s\w", GetCvarTypeStr(flags)),
        charsmax(buffer)
    )

    new bool: hasBounds = (bounds[CvarBound_Lower][b_hasBound] || bounds[CvarBound_Upper][b_hasBound])
    strcat(buffer,
        hasBounds ? 
            fmt("^nBounds: \r%s\w - \r%s\w",
                bounds[CvarBound_Lower][b_hasBound] ? GetBoundByType(flags, bounds[CvarBound_Lower][b_value]) : " - ",
                bounds[CvarBound_Upper][b_hasBound] ? GetBoundByType(flags, bounds[CvarBound_Upper][b_value]) : " - "
            )
            :
            "^nBounds: \dnone\w",
        charsmax(buffer)
    )

    strcat(buffer, 
        fmt("^nDesc: ^n^t\d%s\w", description),
        charsmax(buffer)
    )

    menu_addtext(menu, buffer, .slot = true)

    menu_setprop(menu, MPROP_EXITNAME, fmt("%l", "EXIT"))
    menu_setprop(menu, MPROP_NUMBER_COLOR, "\y")

    menu_display(player, menu)
    return true
}

GetBoundByType(const flags, const Float: bound) {
    new buff[10]
    if (flags & _FCVAR_FLOAT)
        formatex(buff, charsmax(buff), "%.1f", bound)

    if (flags & _FCVAR_BOOLEAN)
        formatex(buff, charsmax(buff), "%s", floatround(bound) != 0 ? "true" : "false")

    if (flags & _FCVAR_INTEGER)
        formatex(buff, charsmax(buff), "%i", floatround(bound))

    return buff
}

public MenuHandler_SettingsItem(const player, const menu, const item) {
    new info[3]
    menu_item_getinfo(
        menu, item,
        .info = info,
        .infolen = charsmax(info)
    )

    menu_destroy(menu)
    
    if (!IsActive())
        return PLUGIN_HANDLED

    if (item < 0) {
        Menu_Settings(player)

        return PLUGIN_HANDLED
    }

    new cvarIndex = strtol(info)

    new name[32], flags, description[256]

    new bool: found = get_plugins_cvar(
        cvarIndex, 
        name, charsmax(name),
        flags,
        .description = description,
        .desc_len = charsmax(description)
    ) != 0

    if (!found)
        return PLUGIN_HANDLED

    server_print("CHANGE settings value: %i", cvarIndex)

    client_cmd(player, "messagemode enter_ChangeSettingValue", cvarIndex)
    Menu_ChangeSettingsItem(player, cvarIndex)

    return PLUGIN_HANDLED
}
