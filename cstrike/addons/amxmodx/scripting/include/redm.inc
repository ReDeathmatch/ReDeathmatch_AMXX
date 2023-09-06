
#if defined _redm_included
	#endinput
#endif
#define _redm_included


new const VERSION[] = "3711549" 

// AMXX 1.9.0 compatibility
#if !defined MAX_MAPNAME_LENGTH
    #define MAX_MAPNAME_LENGTH 64
#endif


/**
 * Returns whether Re:DM is active
 * 
 * @return      True if active, false overwise.
 */
native bool: redm_active()

/**
 * Sets Re:DM to active (do not set the cvar!)
 * 
 * @return      True if active, false overwise.
 */
native bool: redm_set_active(const bool: active)

/**
 * Getting config from main plugin.
 * 
 * @return      Config JSON handle, Invalid_JSON if error occurred.
 */
native JSON: redm_get_config()

/**
 * Getting equip config from main plugin.
 * 
 * @return      Config JSON handle, Invalid_JSON if error occurred.
 */
native JSON: redm_get_equip_config()

/**
 * Get the number of registered spawner styles.
 * 
 * @return      Number of styles.
 */
native redm_spawnstyles()

/**
 * Get a spawn style info by index (indices start at 0).
 * 
 * @param style_index   Style index.
 * @param name          Buffer to copy the name.
 * @param maxlength     Maximum size of buffer.
 * 
 * @error               If an invalid handle is provided
 *                      an error will be thrown.
 * 
 * @return      Number of cells copied from buffer.
 */
native redm_styleinfo(const style_index, name[], const length)

/**
 * Adds a spawn style handler
 */
native redm_addstyle(const name[], const function[])

/**
 * Sets the current spawn style handler by name.
 * The handler registered to this name will be called after every spawn.
 */
native redm_setstyle(const name[])

/**
 * Returns the current style id
 */
native redm_curstyle()

/**
 * Called before initialization start.
 */
forward ReDM_ChangeState(const bool: active)

/**
 * Called before initialization start.
 */
forward ReDM_InitStart()

/**
 * Called after fully initialized.
 */
forward ReDM_Initialized()

/**
 * 
 */
forward ReDM_GetConfigName(config[], const len)

/**
 * 
 */
forward ReDM_GetConfigPrefixName(config[], const len, const mapPrefix[])



enum LogLevel { Trace, Debug, Info, Warning, Error, Fatal }
stock LogMessageEx(const LogLevel: level = Info, const message[], any: ...) {
	static const logLevelString[LogLevel][] = { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" }
	static buffer[4096]
	vformat(buffer, sizeof buffer, message, 3)
	format(buffer, sizeof buffer,"[%.2f][%s] %s", get_gametime(), logLevelString[level], buffer)

	if (level == Fatal)
		set_fail_state("%s", buffer)

	server_print("%s", buffer)
}

stock UTIL_PlaySoundEffect(const player, const sound[], const Float: vol = 0.7, const pitch = PITCH_NORM) {
    rh_emit_sound2(
        player,
        player,
        CHAN_VOICE,
        sound,
        .vol = vol,
        .pitch = pitch
    )
}

stock UTIL_ReloadWeapons(const player, bool: currentWeapon = false) {
    if (!currentWeapon) {
        rg_instant_reload_weapons(player)
        return
    }

    new activeItem = get_member(player, m_pActiveItem)
    if (!is_nullent(activeItem)) {
        rg_instant_reload_weapons(player, activeItem)
    }
}

stock bool: IsBlind(const player) {
    return Float: get_member(player, m_blindUntilTime) > get_gametime()
}

stock FixedUnsigned16(Float: value, scale = (1 << 12)) {
    return clamp(floatround(value * scale), 0, 0xFFFF)
}

enum {
    FFADE_IN = 0x0000, // Just here so we don't pass 0 into the function
    FFADE_OUT = 0x0001, // Fade out (not in)
    FFADE_MODULATE = 0x0002, // Modulate (don't blend)
    FFADE_STAYOUT = 0x0004, // ignores the duration, stays faded out until new ScreenFade message received
}

stock UTIL_ScreenFade(const player, const Float: fxTime = 0.2, const Float: holdTime = 0.2, const color[4] = {0, 200, 0, 50}, const fadeFlags = FFADE_OUT) {
    if (IsBlind(player))
        return

    static msgId_ScreenFade
    if (!msgId_ScreenFade) {
        msgId_ScreenFade = get_user_msgid("ScreenFade")
    }

    message_begin(MSG_ONE_UNRELIABLE, msgId_ScreenFade, .player = player)
    write_short(FixedUnsigned16(fxTime))
    write_short(FixedUnsigned16(holdTime))
    write_short(fadeFlags)
    write_byte(color[0])
    write_byte(color[1])
    write_byte(color[2])
    write_byte(color[3])
    message_end()
}

stock UTIL_DumpJSON(JSON: obj, msg[]) {
    new buffer[1024]
    json_serial_to_string(obj, buffer, charsmax(buffer))

    server_print("%s:`%s`", msg, buffer)
}

stock bool: HasLangKey(const string[]) {
    return (GetLangTransKey(string) != TransKey_Bad)
}

stock ParseColors4(const buffer[]) {
    new stringParsed[4][4]
    new colors[4]

    if (!strlen(buffer))
        return colors
    
    new parsedArgs = parse(
        buffer,
        stringParsed[0], charsmax(stringParsed[]),  // R
        stringParsed[1], charsmax(stringParsed[]),  // G
        stringParsed[2], charsmax(stringParsed[]),  // B
        stringParsed[3], charsmax(stringParsed[])   // A
    )

    if (parsedArgs < 3)
        return colors

    for (new i; i < sizeof(stringParsed); i++)
        colors[i] = strtol(stringParsed[i])

    return colors
}

/**
 * Registers a callback and add to `amxmodmenu` 
 * to be called when the client executes a command from the
 * console.
 *
 * @note For a list of possible access flags, see the ADMIN_* constants in
 *       amxconst.inc
 * @note Opting in to FlagManager enables the admin privileges to be overwritten
 *       by the end user via the cmdaccess.ini config file.
 * @note Automatic detection for FlagManager will only include a command if it
 *       has required privileges (flags is not -1) and it is not a command
 *       starting with "say".
 *
 * @param client_cmd    Command to register
 * @param function      Callback function
 * @param flags         Admin privilege flags required
 * @param info          Command description
 * @param FlagManager   0 opts out of flag manager, 1 opts in, -1 selects
 *                      automatically
 * @param info_ml       If true, the parameter "info" will be looked up as multilingual key
 *
 * @return              Command id, 0 on failure
 * @error               If an invalid callback function is specified, an error
 *                      will be thrown.
 */
stock registerClCmd(const clientCmd[], const function[], flags = -1, const info[] = "", FlagManager = -1, bool: infoMl = false) {
    new clCmdId = register_clcmd(
        clientCmd,
        function,
        flags,
        info,
        FlagManager,
        infoMl
    )

    AddMenuItem(info, clientCmd, flags, "ReDeathmatch")

    return clCmdId
}

stock UTIL_BarTime(const player = 0, const duration) {
    static msgId_BarTime
    if (!msgId_BarTime) {
        msgId_BarTime = get_user_msgid("BarTime")
    }

    message_begin(MSG_ONE, msgId_BarTime, .player = player)
    write_short(duration)
    message_end()
}

stock UTIL_RoundTime(const player = 0, const tmRemaining = 0) {
    static msgId_RoundTime
    if (!msgId_RoundTime) {
        msgId_RoundTime = get_user_msgid("RoundTime")
    }

    message_begin(MSG_ONE, msgId_RoundTime, .player = player)
    write_short(tmRemaining + 1)
    message_end()
}
