new const g_mainDir[] = "redm"
static const g_extraConfigsDir[] = "extraconfigs"
static const g_mainConfigFile[] = "gamemode_deathmatch"

new JSON: g_configHandle = Invalid_JSON

Config_Init() {
    ParseConfig()

    register_concmd("redm_reload", "ConCmd_redm_reload", ADMIN_MAP, "<configname> - reloads Re:DM config")
}

static FindConfigFile(filePath[], len = PLATFORM_MAX_PATH) {
    // TODO: refactor it by a cycle

    new configsDir[PLATFORM_MAX_PATH]
    get_configsdir(configsDir, charsmax(configsDir))

    new configFile[PLATFORM_MAX_PATH]
    formatex(configFile, charsmax(configFile),
        "%s/%s/%s",
        configsDir, g_mainDir, g_extraConfigsDir
    )

    if (!dir_exists(configFile) && mkdir(configFile) == -1)
        LogMessageEx(Warning, "FindConfigFile: Can't create directory `%s`.", configFile)

    // Check configs/redm/extraconfigs/<mapname>.json file
    new mapName[MAX_MAPNAME_LENGTH]
    rh_get_mapname(mapName, charsmax(mapName))

    formatex(configFile, charsmax(configFile), "%s/%s/%s/%s.json",
        configsDir,  g_mainDir, g_extraConfigsDir, mapName
    )

    new fileName[PLATFORM_MAX_PATH]

    if (file_exists(configFile)) {
        remove_filepath(configFile, fileName, charsmax(fileName))
        LogMessageEx(Info, "FindConfigFile: Extra map config loaded for map `%s`.", fileName)
        copy(filePath, len, configFile)        

        return true
    }

    // Check configs/redm/extraconfigs/prefix_<mapPrefix>.json file
    new mapPrefix[32]
    copyc(mapPrefix, charsmax(mapPrefix), mapName, '_')
    formatex(configFile, charsmax(configFile), "%s/%s/%s/%s.json",
        configsDir,  g_mainDir, g_extraConfigsDir, GetMainConfigPrefixName(mapPrefix)
    )

    if (file_exists(configFile)) {
        remove_filepath(configFile, fileName, charsmax(fileName))
        LogMessageEx(Info, "FindConfigFile: Prefix map config `%s` loaded.", fileName)
        copy(filePath, len, configFile)        

        return true
    }

    // Check configs/redm/gamemode_deathmatch.json file
    formatex(configFile, charsmax(configFile), "%s/%s/%s.json",
        configsDir, g_mainDir, GetMainConfigName()
    )

    if (file_exists(configFile)) {
        remove_filepath(configFile, fileName, charsmax(fileName))
        LogMessageEx(Info, "FindConfigFile: Config `%s` loaded.", fileName)
        copy(filePath, len, configFile)        

        return true
    }

    LogMessageEx(Warning, "FindConfigFile: Can't find any config file!")

    return false
}

static GetMainConfigName() {
    new file[PLATFORM_MAX_PATH]
    copy(file, charsmax(file), g_mainConfigFile)

    CallApi_GetConfigName(file, charsmax(file))

    return file
}

static GetMainConfigPrefixName(const mapPrefix[]) {
    new file[PLATFORM_MAX_PATH]
    format(file, charsmax(file), "prefix_%s", mapPrefix)

    CallApi_GetConfigPrefixName(file, charsmax(file), mapPrefix)

    return file
}

static bool: ParseConfig(const file[] = "") {
    new configFile[PLATFORM_MAX_PATH]

    if (strlen(file) != 0) {
        copy(configFile, charsmax(configFile), file)
    } else if (!FindConfigFile(configFile, charsmax(configFile))) {
        return false
    }

    new JSON: config = json_parse(configFile, .is_file = true, .with_comments = true)
    if (config == Invalid_JSON) {
        LogMessageEx(Warning, "ParseConfig: Can't parse JSON from `%s` file!", configFile)

        return false
    }

    g_configHandle = config
    return true
}

ReloadConfig() {
    CvarsHandler_LoadConfig(Config_GetCurrent())

    new JSON: equipConfig = Config_GetModeEquip()
    if (equipConfig == Invalid_JSON)
        equipConfig = Config_GetMainEquip()

    EquipManager_LoadConfig(equipConfig)

    json_free(equipConfig)
}


public ConCmd_redm_reload(const player, level, cid) {
    SetGlobalTransTarget(player)
      
    if (!cmd_access(player, level, cid, 2))
        return PLUGIN_HANDLED

    new filePath[PLATFORM_MAX_PATH]
    if (read_argc() == 2) {
        read_argv(1, filePath, charsmax(filePath))
    }
    
    if (!Config_ReloadCfg(filePath))
        return PLUGIN_HANDLED

    RoundModes_ResetCurrentMode()
    console_print(player, "[Re:DM] Config file reloaded.")
    return PLUGIN_HANDLED
}

bool: Config_ReloadCfg(const file[] = "") {
    json_free(g_configHandle)

    if (strlen(file) == 0)
        return ParseConfig()

    new configsDir[PLATFORM_MAX_PATH]
    get_configsdir(configsDir, charsmax(configsDir))

    new configFile[PLATFORM_MAX_PATH]
    formatex(configFile, charsmax(configFile),
        "%s/%s/%s",
        configsDir, g_mainDir, file
    )

    if (!file_exists(configFile)) {
        LogMessageEx(Warning, "Config_ReloadCfg: Config file `%s` not exists!", configFile)

        return false
    }

    return ParseConfig(configFile)
}

JSON: Config_GetCurrent() {
    return g_configHandle
}

JSON: Config_GetModeEquip() {
    if (!json_object_has_value(Config_GetCurrent(), "modes")) {
        return Invalid_JSON
    }

    new JSON: objModes = json_object_get_value(Config_GetCurrent(), "modes")
    new count = json_object_get_count(objModes)
    if (!count) {
        json_free(objModes)

        return Invalid_JSON
    }

    new currentModeIdx = RoundModes_GetCurrentMode()
    if (currentModeIdx < 0) {
        json_free(objModes)

        return Invalid_JSON
    }


    new JSON: objMode = json_object_get_value_at(objModes, currentModeIdx)
    if (!json_object_has_value(objMode, "equip")) {
        json_free(objMode)
        json_free(objModes)

        return Invalid_JSON
    }

    new JSON: objEquip = json_object_get_value(objMode, "equip")
    new JSON: obj = json_deep_copy(objEquip)

    json_free(objEquip)
    json_free(objMode)
    json_free(objModes)

    return obj
}

JSON: Config_GetMainEquip() {
    if (!json_object_has_value(Config_GetCurrent(), "equip"))
        return Invalid_JSON

    new JSON: objEquip = json_object_get_value(Config_GetCurrent(), "equip")
    return objEquip
}