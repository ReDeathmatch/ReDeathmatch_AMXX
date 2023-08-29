static Trie: g_cvarValues = Invalid_Trie


CvarsHandler_Init() {
    register_concmd("redm_dump_cvars", "ConCmd_redm_dump_cvars", ADMIN_MAP, "Dump changed CVar to table.")
}

public ConCmd_redm_dump_cvars(const player) {
    DumpAllSavedCvars(player)

    return PLUGIN_HANDLED
}

CvarsHandler_LoadConfig(const JSON: config) {
    new JSON: cvars = json_object_get_value(config, "cvars")
    if (cvars == Invalid_JSON) {
        new buffer[512]
        json_serial_to_string(config, buffer, charsmax(buffer), .pretty = true)
        set_fail_state("Can't read `cvars` section from config = `%s`", buffer)
    }

    CvarsHandler_LoadCvars(cvars)

    json_free(cvars)
}

CvarsHandler_LoadCvars(const JSON: objCvars) {
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

        CvarChangeValue(key, value)

        json_free(cvarSetting)
    }
}

static CvarChangeValue(const cvar[], const value[]) {
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

static stock bool: CvarRestoreValue(const cvar[]) {
    new storedValue[32]
    if (!TrieGetString(g_cvarValues, cvar, storedValue, charsmax(storedValue))) {
        LogMessageEx(Warning, "CvarRestoreValue: WARNING! CVar `%s` hasn't stored value. Can't restore.", 
            cvar, storedValue, value
        )

        return false
    }
    
    set_cvar_string(cvar, storedValue)

    return true
}

RestoreAllCvars() {
    if (g_cvarValues == Invalid_Trie) {
        LogMessageEx(Debug, "RestoreAllCvars(): WARNING! CVars not saved. Can't restore.")
        return
    }

    new TrieIter: iter = TrieIterCreate(g_cvarValues)

    while(!TrieIterEnded(iter)) {
        new key[32]
        TrieIterGetKey(iter, key, charsmax(key))

        new value[32]
        TrieIterGetString(iter, value, charsmax(value))

        set_cvar_string(key, value)

        TrieIterNext(iter)
    }

    TrieIterDestroy(iter)
    TrieClear(g_cvarValues)
}

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
