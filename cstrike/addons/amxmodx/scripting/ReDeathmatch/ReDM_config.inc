static const g_mainDir[] = "redm"
static const g_extraConfigsDir[] = "extraconfigs"
static const g_mainConfigFile[] = "gamemode_deathmatch"

static JSON: g_configHandle = Invalid_JSON

Config_Init() {
    ParseConfig()

    register_concmd("redm_reload", "ConCmd_redm_reload", ADMIN_MAP, "<configname> - reloads Re:DM config")
}

/**
 * Searches for and loads a Re:DM configuration file.
 *
 * This function attempts to find and load a Re:DM configuration file based on
 * the current map name, map prefix, and default configuration. It searches for
 * configuration files in the following order of priority:
 * 1. Map-specific configuration file: `configs/redm/extraconfigs/<mapname>.json`
 * 2. Map prefix-specific configuration file: `configs/redm/extraconfigs/prefix_<mapPrefix>.json`
 * 3. Default game mode configuration file: `configs/redm/gamemode_deathmatch.json`
 *
 * If a matching configuration file is found, it is loaded, and its path is copied to `filePath`.
 *
 * @param filePath   Buffer to store the path of the loaded configuration file.
 * @param len        Maximum length of the `filePath` buffer.
 *
 * @return           True if a configuration file is found and loaded successfully, false otherwise.
 *                   If no suitable configuration file is found, a warning is logged, and the function returns false.
 */
static bool: FindConfigFile(filePath[], len = PLATFORM_MAX_PATH) {
    new configsDir[PLATFORM_MAX_PATH]
    get_configsdir(configsDir, charsmax(configsDir))

    new mapName[MAX_MAPNAME_LENGTH]
    get_mapname(mapName, charsmax(mapName))

    new mapPrefix[32]
    copyc(mapPrefix, charsmax(mapPrefix), mapName, '_')

    new configFiles[3][PLATFORM_MAX_PATH]

    formatex(configFiles[0], charsmax(configFiles[]), "%s/%s/%s/%s.json", configsDir, g_mainDir, g_extraConfigsDir, mapName)
    formatex(configFiles[1], charsmax(configFiles[]), "%s/%s/%s/%s.json", configsDir, g_mainDir, g_extraConfigsDir, GetMainConfigPrefixName(mapPrefix))
    formatex(configFiles[2], charsmax(configFiles[]), "%s/%s/%s.json", configsDir, g_mainDir, GetMainConfigName())

    new fileName[PLATFORM_MAX_PATH]
    for (new i = 0; i < sizeof(configFiles); i++) {
        if (file_exists(configFiles[i][0])) {
            remove_filepath(configFiles[i][0], fileName, charsmax(fileName))
            copy(filePath, len, configFiles[i][0])

            LogMessageEx(Info, "FindConfigFile: Config `%s` loaded.", fileName)
            return true
        }
    }

    LogMessageEx(Fatal, "^n    Can't find any config file in `%s/%s/**/*.json`!", configsDir, g_mainDir)

    return false
}

/**
 * Gets the name of the main configuration file.
 *
 * @return      The name of the main configuration file as a string.
 */
static GetMainConfigName() {
    new file[PLATFORM_MAX_PATH]
    copy(file, charsmax(file), g_mainConfigFile)

    CallApi_GetConfigName(file, charsmax(file))

    return file
}

/**
 * Gets the name of the configuration file with a prefix based on the map prefix.
 *
 * @param mapPrefix     The map prefix used to create the file name.
 *
 * @return              The name of the configuration file with the specified prefix as a string.
 */
static GetMainConfigPrefixName(const mapPrefix[]) {
    new file[PLATFORM_MAX_PATH]
    format(file, charsmax(file), "prefix_%s", mapPrefix)

    CallApi_GetConfigPrefixName(file, charsmax(file), mapPrefix)

    return file
}

/**
 * Parses a configuration file.
 *
 * @param file          The name of the configuration file to parse. If empty (default), it searches for the file.
 *
 * @return              Returns true if the configuration file is successfully parsed, false otherwise.
 */
static bool: ParseConfig(const file[] = "") {
    new configFile[PLATFORM_MAX_PATH]

    if (strlen(file) != 0) {
        copy(configFile, charsmax(configFile), file)
    } else if (!FindConfigFile(configFile, charsmax(configFile))) {
        return false
    }

    new JSON: config = json_parse(configFile, .is_file = true, .with_comments = true)
    if (config == Invalid_JSON) {
        LogMessageEx(Fatal, "ParseConfig: Can't parse JSON from `%s` file!", configFile)

        return false
    }

    g_configHandle = config
    return true
}

/**
 * Reloads the configuration file and updates related settings.
 */
Config_Reload() {
    CvarsHandler_Load(Config_GetCurrent())

    new JSON: equipConfig = Config_GetModeEquip()
    if (equipConfig == Invalid_JSON)
        equipConfig = Config_GetMainEquip()

    EquipManager_LoadConfig(equipConfig)
    RoundModes_ReadModes()

    json_free(equipConfig)
}

/**
 * Console command handler for reloading the Re:DM configuration file.
 *
 * @param player        The player executing the command.
 * @param level         The admin level of the player.
 * @param commandId     The unique command ID assigned to this command.
 *
 * @return              Returns PLUGIN_HANDLED to indicate that the command was handled.
 */
public ConCmd_redm_reload(const player, const level, const commandId) {
    SetGlobalTransTarget(player)
      
    if (!cmd_access(player, level, commandId, 2))
        return PLUGIN_HANDLED

    new filePath[PLATFORM_MAX_PATH]
    if (read_argc() == 2) {
        read_argv(1, filePath, charsmax(filePath))
    }
    
    if (!LoadFile(filePath))
        return PLUGIN_HANDLED

    RoundModes_ResetCurrentMode()
    console_print(player, "[Re:DM] Config file reloaded.")
    return PLUGIN_HANDLED
}

/**
 * Loads a configuration file, updating the current configuration.
 *
 * @param file          The name of the configuration file to load. If empty (default), it loads the main configuration.
 *
 * @return              Returns true if the file is successfully loaded and parsed, false otherwise.
 */
static bool: LoadFile(const file[] = "") {
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
        LogMessageEx(Warning, "LoadFile: Config file `%s` not exists!", configFile)

        return false
    }

    return ParseConfig(configFile)
}

/**
 * Gets the current configuration object.
 *
 * @return              The current configuration as a JSON object.
 */
JSON: Config_GetCurrent() {
    return g_configHandle
}

/**
 * Gets the equipment object for the current game mode.
 *
 * @return              The equipment configuration for the current game mode as a JSON object.
 */
JSON: Config_GetModeEquip() {
    new count
    new JSON: arrModes = RoundModes_GetModesArr(count)
    if (!count)
        return Invalid_JSON

    new currentModeIdx = RoundModes_GetCurrentMode()
    if (currentModeIdx < 0)
        return Invalid_JSON

    new JSON: objMode = json_array_get_value(arrModes, currentModeIdx)
    if (!json_object_has_value(objMode, "equip")) {
        json_free(objMode)

        return Invalid_JSON
    }

    new JSON: objEquip = json_object_get_value(objMode, "equip")
    new JSON: obj = json_deep_copy(objEquip)

    json_free(objEquip)
    json_free(objMode)

    return obj
}

/**
 * Gets the equipment object for the main configuration.
 *
 * @return              The equipment configuration for the main configuration as a JSON object.
 */
JSON: Config_GetMainEquip() {
    if (!json_object_has_value(Config_GetCurrent(), "equip"))
        return Invalid_JSON

    new JSON: objEquip = json_object_get_value(Config_GetCurrent(), "equip")
    return objEquip
}
