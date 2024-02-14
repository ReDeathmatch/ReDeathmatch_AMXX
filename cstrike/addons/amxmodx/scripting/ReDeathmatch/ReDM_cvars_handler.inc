static Trie: g_cvarValues = Invalid_Trie

CvarsHandler_Init() {
    register_concmd("redm_dump_cvars", "ConCmd_redm_dump_cvars", ADMIN_MAP, "Dump changed CVar to table.")
    register_concmd("redm_dump_settings", "ConCmd_redm_dump_settings", ADMIN_MAP, "Dump mode settings")
}

/**
 * Displays saved CVAR values to the specified player.
 *
 * @param player        The player to whom the CVAR values should be displayed.
 * @param level         The access level of the player executing the command.
 * @param commandId     The ID of the command.
 * @return              Returns PLUGIN_HANDLED.
 */
public ConCmd_redm_dump_cvars(const player, const level, const commandId) {
    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED

    DumpAllSavedCvars(player)

    return PLUGIN_HANDLED
}

/**
 * Displays mode settings to the specified player.
 *
 * @param player        The player to whom the CVAR values should be displayed.
 * @param level         The access level of the player executing the command.
 * @param commandId     The ID of the command.
 * @return              Returns PLUGIN_HANDLED.
 */
public ConCmd_redm_dump_settings(const player, const level, const commandId) {
    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED

    DumpAllSettings(player)

    return PLUGIN_HANDLED
}

/**
 * Loads CVAR settings from a Re:DM configuration.
 *
 * @param config        JSON handle representing the Re:DM configuration.
 */
CvarsHandler_Load(const JSON: config) {
    new JSON: cvars = json_object_get_value(config, "cvars")
    if (cvars == Invalid_JSON) {
        new buffer[512]
        json_serial_to_string(config, buffer, charsmax(buffer), .pretty = true)
        set_fail_state("Can't read `cvars` section from config = `%s`", buffer)
    }

    CvarsHandler_ApplyCvars(cvars)

    json_free(cvars)
}

/**
 * Applies CVAR settings from a JSON object.
 *
 * This function applies CVAR settings specified in a JSON object to their corresponding CVARs.
 * The JSON object is expected to have CVAR names as keys and their values as string values.
 *
 * @param objCvars      JSON handle containing the CVAR settings.
 */
CvarsHandler_ApplyCvars(const JSON: objCvars) {
    new cvarsCount = json_object_get_count(objCvars)
    if (!cvarsCount) {
        // set_fail_state("Section `cvars` hasn't settings in config")
        return
    }

    for (new i; i < cvarsCount; i++) {
        new JSON: cvarSetting = json_object_get_value_at(objCvars, i)

        new key[32], value[32]
        json_object_get_name(objCvars, i, key, charsmax(key))
        json_get_string(cvarSetting, value, charsmax(value))

        ChangeValue(key, value)

        json_free(cvarSetting)
    }
}

/**
 * Changes the value of a CVAR and stores its old value.
 *
 * @param cvar          The name of the CVAR to change.
 * @param value         The new value for the CVAR.
 */
static ChangeValue(const cvar[], const value[]) {
    if (!g_cvarValues) {
        g_cvarValues = TrieCreate()
    }

    new oldValue[32]
    get_cvar_string(cvar, oldValue, charsmax(oldValue))

    if (strcmp(oldValue, value) == 0)
        return

    if (!TrieKeyExists(g_cvarValues, cvar)) {
        // Store only original CVar value
        TrieSetString(g_cvarValues, cvar, oldValue)
    }

    set_cvar_string(cvar, value)
}

/**
 * Restores the value of a CVAR or all CVARs to their stored values.
 *
 * @param cvar          The name of the CVAR to restore. If empty, all CVARs are restored.
 * @return              Returns true if the CVAR(s) are successfully restored, false otherwise.
 *                      If a specific CVAR is provided and it has no stored value, a warning is logged, and the function returns false.
 */
bool: CvarsHandler_RestoreValue(const cvar[] = "") {
    if (strlen(cvar) == 0)
        return RestoreAllStoredValues()

    new storedValue[32]
    if (!TrieGetString(g_cvarValues, cvar, storedValue, charsmax(storedValue))) {
        LogMessageEx(Warning, "CvarsHandler_RestoreValue: WARNING! CVar `%s` hasn't stored value. Can't restore.", 
            cvar
        )

        return false
    }
    
    set_cvar_string(cvar, storedValue)

    return true
}

/**
 * Restores all CVARs to their stored values.
 *
 * @return              Returns true if all CVARs are successfully restored, false otherwise.
 */
static bool: RestoreAllStoredValues() {
    if (g_cvarValues == Invalid_Trie) {
        LogMessageEx(Debug, "RestoreAllStoredValues(): WARNING! CVars not saved. Can't restore.")
        return false
    }

    new TrieIter: iter = TrieIterCreate(g_cvarValues)

    while (!TrieIterEnded(iter)) {
        new key[32]
        TrieIterGetKey(iter, key, charsmax(key))

        new value[32]
        TrieIterGetString(iter, value, charsmax(value))

        set_cvar_string(key, value)

        TrieIterNext(iter)
    }

    TrieIterDestroy(iter)
    TrieClear(g_cvarValues)

    return true
}

/**
 * Dumps all saved CVars to the console or a specific player.
 *
 * This function is used to display all saved CVars to the console or a specific player. It retrieves
 * the saved CVar values from the global Trie data structure and compares them with the current
 * values of the CVars, showing the old and current values.
 *
 * @param player The player to whom the CVar information will be displayed (default is console).
 */
stock DumpAllSavedCvars(const player = 0) {
    if (g_cvarValues == Invalid_Trie) {
        LogMessageEx(Debug, "DumpAllSavedCvars(): WARNING! CVars not saved. Can't restore.")
        return
    }

    if (!TrieGetSize(g_cvarValues)) {
        LogMessageEx(Debug, "DumpAllSavedCvars(): WARNING!  Hasn't saved CVars.")
        return
    }

    new idx
    new TrieIter: iter = TrieIterCreate(g_cvarValues)
    console_print(player, "Dump saved CVars:")

    new const template[] = "| %-2i | %-32s | %-8s | %-8s |"
    console_print(player, "| %-2s | %-32s | %-8s | %-8s |",
        "#", "CVar", "old", "current"
    )
    console_print(player, "| -- | -------------------------------- | -------- | -------- |")
    while (!TrieIterEnded(iter)) {
        new key[32]
        TrieIterGetKey(iter, key, charsmax(key))

        new value[32]
        TrieIterGetString(iter, value, charsmax(value))

        new current[32]
        get_cvar_string(key, current, charsmax(current))

        console_print(player, template, ++idx, key, value, current)

        TrieIterNext(iter)
    }

    TrieIterDestroy(iter)
}

DumpAllSettings(const player = 0, const prefix[] = "redm_") {
    new const template[] = "| %-2i | %-32.32s | %-7s | %-16s | %10s |"
    console_print(player, "| %-2s | %-32.32s | %-7s | %-16s | %10s |",
        "#", "ConVar name", "type", "value", "bounds"
    )
    console_print(player,
        "| -- | -------------------------------- | ------- | ---------------- | ---------- |"
    )

    for (new cvarIndex, idx; cvarIndex < 100; cvarIndex++) {
        new name[32]
        new flags
        new pluginId
        new cvarHandle
        new description[192]

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

        #if 0
        // TODO: Does it need to be?
        static currentPluginId = -1
        if (currentPluginId == -1)
            currentPluginId = get_plugin(-1)

        if (pluginId != currentPluginId)
            continue
        #endif

        if (strncmp(name, prefix, strlen(prefix), .ignorecase = true) != 0)
            continue

        new valueStr[32]
        if (flags & _FCVAR_FLOAT)
            formatex(valueStr, charsmax(valueStr), "%.2f", get_pcvar_float(cvarHandle))
        else 
            get_pcvar_string(cvarHandle, valueStr, charsmax(valueStr))
        
        new bounds[CvarBounds][Bounds_s]

        for (new CvarBounds: boundType; boundType < CvarBounds; boundType++) {
            bounds[boundType][b_hasBound] = get_pcvar_bounds(cvarHandle, boundType, bounds[boundType][b_value])
        }

        new boundsBuffer[32]
        if (bounds[CvarBound_Lower][b_hasBound] || bounds[CvarBound_Upper][b_hasBound]) {
            formatex(boundsBuffer, charsmax(boundsBuffer), "%s .. %s",
                bounds[CvarBound_Lower][b_hasBound] ? fmt("%.1f", bounds[CvarBound_Lower][b_value]) : " - ",
                bounds[CvarBound_Upper][b_hasBound] ? fmt("%.1f", bounds[CvarBound_Upper][b_value]) : " - "
            )
        }

        console_print(player, template,
            ++idx,
            name,
            GetCvarTypeStr(flags),
            valueStr,
            boundsBuffer
        )
    }
}
