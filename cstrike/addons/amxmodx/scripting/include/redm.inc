
#if defined _redm_included
	#endinput
#endif
#define _redm_included


#include <redm_version>

// AMXX 1.9.0 compatibility
#if !defined MAX_MAPNAME_LENGTH
    #define MAX_MAPNAME_LENGTH 64
#endif


/**
 * Returns whether Re:DM is active.
 * 
 * @return      True if Re:DM is active, false otherwise.
 */
native bool: redm_is_active()

/**
 * Sets Re:DM to active (do not set the cvar!).
 * 
 * @param active    True to set Re:DM active, false to deactivate it.
 * @return          True if Re:DM is successfully set to active or inactive, false otherwise.
 */
native bool: redm_set_active(const bool: active)

/**
 * Retrieves the main plugin's configuration as a JSON handle.
 * 
 * @return      Config JSON handle. Returns Invalid_JSON if an error occurs.
 */
native JSON: redm_get_config()

/**
 * Retrieves the equipment configuration from the main plugin as a JSON handle.
 * 
 * @return      Equip config JSON handle. Returns Invalid_JSON if an error occurs.
 */
native JSON: redm_get_equip_config()

/**
 * Gets the number of registered spawner styles.
 * 
 * @return      The number of registered spawner styles.
 */
native redm_spawnstyles()

/**
 * Retrieves spawn style information by index.
 * 
 * @param style_index   Style index.
 * @param name          Buffer to copy the name.
 * @param maxlength     Maximum size of the buffer.
 * 
 * @error               Throws an error if an invalid handle is provided.
 * 
 * @return              The number of characters copied into the 'name' buffer.
 */
native redm_styleinfo(const style_index, name[], const length)

/**
 * Adds a spawn style handler with the specified name and function.
 */
native redm_addstyle(const name[], const function[])

/**
 * Sets the current spawn style handler by name.
 * The handler registered to this name will be called after every spawn.
 * 
 * @param name     Style preset name.
 * @return         True if the style has been set, otherwise false.
 */
native bool: redm_setstyle(const name[])

/**
 * Returns the current style ID.
 * 
 * @return      The ID of the current spawn style.
 */
native redm_curstyle()

/**
 * Called before initialization starts.
 */
forward ReDM_ChangeState(const bool: active)

/**
 * Called before initialization starts.
 */
forward ReDM_InitStart()

/**
 * Called after full initialization.
 */
forward ReDM_Initialized()

/**
 * Forward for retrieving the config name.
 * 
 * @param config    Buffer to copy the config name.
 * @param len       Maximum length of the buffer.
 */
forward ReDM_GetConfigName(config[], const len)

/**
 * Forward for retrieving the config prefix name.
 * 
 * @param config    Buffer to copy the config prefix name.
 * @param len       Maximum length of the buffer.
 * @param mapPrefix Buffer containing the map prefix.
 */
forward ReDM_GetConfigPrefixName(config[], const len, const mapPrefix[])



enum LogLevel { Trace, Debug, Info, Warning, Error, Fatal }

/**
 * Logs a message with the specified log level.
 *
 * This function logs a message with the given log level along with a timestamp.
 *
 * @param level     The log level (e.g., Info, Warning, Error).
 * @param message   The message to log. You can use format specifiers as in printf.
 * @param ...       Additional parameters for message formatting.
 */
stock LogMessageEx(const LogLevel: level = Info, const message[], any: ...) {
    static buffer[4096]
    vformat(buffer, sizeof buffer, message, 3)

    if (level == Fatal) {
        format(buffer, sizeof buffer, "%s", buffer)
        set_fail_state("%s", buffer)

        return
    }

    static const logLevelString[LogLevel][] = { "TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL" }
    format(buffer, sizeof buffer, "[%.2f][%s] %s", get_gametime(), logLevelString[level], buffer)

    server_print("%s", buffer)
}

/**
 * Plays a sound effect for a player with specified volume and pitch.
 *
 * This function plays a sound effect for a player with the specified volume and pitch.
 *
 * @param player    The player to play the sound effect to.
 * @param sound     The name of the sound file to play.
 * @param vol       The volume of the sound (default is 0.7).
 * @param pitch     The pitch of the sound (default is PITCH_NORM).
 */
stock UTIL_PlaySoundEffect(const player, const sound[], const Float: vol = 0.7, const pitch = PITCH_NORM, const Float: attn = ATTN_NORM, const flags = 0) {
    emit_sound(
        player,
        CHAN_VOICE,
        sound,
        .vol = vol,
        .pitch = pitch,
        .att = attn,
        .flags = flags
    )
}

/**
 * Reloads weapons for a player, optionally for the currently active weapon.
 *
 * This function reloads weapons for a player. If `currentWeapon` is true, it reloads the currently active weapon,
 * otherwise, it reloads all weapons.
 *
 * @param player          The player for whom to reload weapons.
 * @param currentWeapon   If true, reloads the currently active weapon; otherwise, reloads all weapons (default).
 */
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

/**
 * Sends a death message to a player or set of players.
 *
 * This function sends a death message to a specific player or set of players. A death message
 * typically includes information about the killer, victim, whether it was a headshot, and the
 * weapon used.
 *
 * @param dest       The destination of the death message (one or more players).
 * @param receiver   The receiver of the message (typically the player(s) who see the death message).
 * @param killer     The ID of the killer player (attacker).
 * @param victim     The ID of the victim player (the one who died).
 * @param headshot   A flag indicating whether the death was a headshot (1 for headshot, 0 for not).
 * @param weaponName The name of the weapon used in the kill.
 */
stock UTIL_DeathMsg(const dest, const receiver, const killer, const victim, const headshot, const weaponName[]) {
    static msg_deathMsg
    if (!msg_deathMsg)
        msg_deathMsg = get_user_msgid("DeathMsg")

    message_begin(dest, msg_deathMsg, _, receiver)
    write_byte(killer)
    write_byte(victim)
    write_byte(headshot)
    write_string(weaponName)
    message_end()
}

/**
 * Checks if a player is blinded.
 *
 * This function checks if a player is blinded based on the current game time.
 *
 * @param player    The player to check for blindness.
 * @return          Returns true if the player is blinded; otherwise, returns false.
 */
stock bool: CBasePlayer_IsBlind(const player) {
    return Float: get_member(player, m_blindUntilTime) > get_gametime()
}

stock bool: SV_IsPlayerIndex(const index) {
    return (index >= 1 && index <= MaxClients)
}

/**
 * Retrieves the round restart delay duration from a game mode CVAR.
 *
 * This function is used to retrieve the round restart delay duration from a game mode CVAR
 * (ConVar). It checks if the CVAR exists and returns its value as a floating-point number.
 * If the CVAR doesn't exist or cannot be retrieved, it returns a default value for the round
 * restart delay.
 *
 * @return The duration of the round restart delay as a floating-point number.
 */
stock Float: CHalfLifeMultiplay_GetRoundRestartDelay() {
    static pointer
    if (!pointer)
        pointer = get_cvar_pointer("mp_round_restart_delay")

    const Float: ROUND_BEGIN_DELAY = 5.0    // delay before beginning new round
    return pointer ? get_pcvar_float(pointer) : ROUND_BEGIN_DELAY
}

/**
 * Converts a floating-point value to a FixedUnsigned16 format.
 *
 * This function converts a floating-point value to the FixedUnsigned16 format,
 * clamping it within the valid range [0, 0xFFFF].
 *
 * @param value     The floating-point value to convert.
 * @param scale     The scaling factor (default is 1 << 12).
 * @return          Returns the FixedUnsigned16 representation of the value.
 */
stock FixedUnsigned16(Float: value, scale = (1 << 12)) {
    return clamp(floatround(value * scale), 0, 0xFFFF)
}

enum {
    FFADE_IN = 0x0000, // Just here so we don't pass 0 into the function
    FFADE_OUT = 0x0001, // Fade out (not in)
    FFADE_MODULATE = 0x0002, // Modulate (don't blend)
    FFADE_STAYOUT = 0x0004, // ignores the duration, stays faded out until new ScreenFade message received
}

/**
 * Initiates a screen fade effect for a player.
 *
 * This function initiates a screen fade effect for a player with specified parameters.
 *
 * @param player    The player to apply the screen fade effect to.
 * @param fxTime    The duration of the fade-in effect (default is 0.2 seconds).
 * @param holdTime  The duration to hold the fade (default is 0.2 seconds).
 * @param color     An array representing the RGBA color values for the fade effect (default is [0, 200, 0, 50]).
 * @param fadeFlags Flags to control the fade effect (default is FFADE_OUT).
 */
stock UTIL_ScreenFade(const player, const Float: fxTime = 0.2, const Float: holdTime = 0.2, const color[4] = {0, 200, 0, 50}, const fadeFlags = FFADE_OUT) {
    if (CBasePlayer_IsBlind(player))
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

/**
 * Dumps a JSON object to the server log.
 *
 * This function serializes a JSON object to a string and logs it to the server log.
 *
 * @param obj   The JSON object to dump.
 * @param msg   A message prefix to include in the log entry.
 */
stock UTIL_DumpJSON(const JSON: obj, const bool: pretty = false) {
    new buffer[8192]
    json_serial_to_string(obj, buffer, charsmax(buffer), pretty)

    for (new i, len = strlen(buffer); i < len; i+=255) {
        server_print(fmt("%-255s", buffer[i]))
    }
}

/**
 * Checks if a language translation key exists.
 *
 * This function checks if a language translation key exists for a given string.
 *
 * @param string    The string to check for a translation key.
 * @return          Returns true if a translation key exists; otherwise, returns false.
 */
stock bool: HasLangKey(const string[]) {
    return (GetLangTransKey(string) != TransKey_Bad)
}

/**
 * Parses a color buffer into an array of RGBA values.
 *
 * This function parses a color buffer (e.g., "255 0 0 128") into an array of RGBA values.
 *
 * @param buffer    The color buffer to parse.
 * @return          Returns an array of RGBA values [R, G, B, A].
 */
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

    if (is_plugin_loaded("Menus Front-End"))
        AddMenuItem(info, clientCmd, flags, "ReDeathmatch")

    return clCmdId
}

/**
 * Sends a "BarTime" message to the specified player indicating a time duration.
 *
 * This function sends a message to the specified player to display a time duration in a progress bar.
 *
 * @param player    The player ID to send the message to. Defaults to 0 (all players).
 * @param duration  The duration to display in the progress bar.
 */
stock UTIL_BarTime(const player = 0, const duration) {
    static msgId_BarTime
    if (!msgId_BarTime) {
        msgId_BarTime = get_user_msgid("BarTime")
    }

    message_begin(MSG_ONE, msgId_BarTime, .player = player)
    write_short(duration)
    message_end()
}

/**
 * Sends a "RoundTime" message to the specified player indicating remaining time for the round.
 *
 * This function sends a message to the specified player to display the remaining time for the round.
 *
 * @param player      The player ID to send the message to. Defaults to 0 (all players).
 * @param tmRemaining The remaining time for the round to display.
 */
stock UTIL_RoundTime(const player = 0, const tmRemaining = 0) {
    static msgId_RoundTime
    if (!msgId_RoundTime) {
        msgId_RoundTime = get_user_msgid("RoundTime")
    }

    message_begin(MSG_ONE, msgId_RoundTime, .player = player)
    write_short(tmRemaining + 1)
    message_end()
}


/**
 * Checks if the current map's name is compatible with a list of prefix blocklists.
 *
 * This function compares the current map's name with a list of prefixes specified in `prefixBlocklist`.
 * It returns true if the map's name does not contain any of the specified prefixes, and false otherwise.
 *
 * @param prefixBlocklist   An array of strings representing the prefixes that should not be present in the map's name.
 * @param size              The size of the `prefixBlocklist` array.
 *
 * @return                  Returns true if the current map's name is compatible with the prefix blocklist,
 *                          meaning it does not contain any of the specified prefixes.
 *                          Returns false if the current map's name contains any of the specified prefixes.
 */
stock bool: IsMapCompatible(const prefixBlocklist[][], const size = sizeof(prefixBlocklist)) {
    new mapName[32]
    get_mapname(mapName, charsmax(mapName))

    for (new i; i < size; i++) {
        if (containi(mapName, prefixBlocklist[i]) == -1)
            continue
        
        return false
    }

    return true
}

/**
 * Linearly interpolates between two values.
 *
 * Linearly interpolates between a and b by t.
 * When t = 0 returns a.
 * When t = 1 return b.
 * When t = 0.5 returns the midpoint of a and b.
 *
 * @param a         Start value, returned when t = 0.
 * @param b         End value, returned when t = 1.
 * @param t         Value used to interpolate between a and b.
 *
 * @return          The interpolated float result between the two float values.
 */
stock Float: _xs_lerp(const Float: a, const Float: b, const Float: t) {
    return a + (b - a) * t;
}

/**
 * Determines where a value lies between two points.
 * The a and b values define the start and end of a linear numeric range.
 * The "value" parameter you supply represents a value which might lie somewhere within that range.
 * This method calculates where, within the specified range, the "value" parameter falls.
 * If the "value" parameter is within the range, _xs_inverse_lerp() returns a value between zero and one,
 * proportional to the value's position within the range.
 * If the "value" parameter falls outside of the range, _xs_inverse_lerp() returns either zero or one,
 * depending on whether it falls before the start of the range or after the end of the range.
 *
 * @param a         The a of the range.
 * @param b         The b of the range.
 * @param t         The point within the range you want to calculate.
 *
 * @return          A value between zero and one,
 *                  representing where the "value" parameter
 *                  falls within the range defined by `a` and `b`.
 */
stock Float: _xs_inverse_lerp(const Float: a, const Float: b, const Float: value) {
    return (value - a) / (b - a)
}

/**
 * Linearly interpolates between two vectors.
 *
 * @param a         Start value, returned when t = 0.
 * @param b         End value, returned when t = 1.
 * @param out       The output vector. Can be one of the input vectors.
 * @param t         Value used to interpolate between a and b.
 *
 * @noreturn
 */
stock _xs_vec_lerp(Float: a[], Float: b[], Float: out[], const Float: t, const size = sizeof(out)) {
    for (new i; i < size; i++) {
        out[i] = a[i] + (b[i] - a[i]) * t;
    }
}

/**
 * Linearly interpolates between two angles.
 * This method returns the shortest path between the specified angles.
 * This method wraps around values that are outside the range [-180, 180].
 * For example, _xs_vec_lerp_angle(1.0, 190.0, 1.0) returns -170.0.
 * To find the longest path use xs_lerp().
 *
 * @param a         The a angle. A float expressed in degrees.
 * @param b         The b angle. A float expressed in degrees.
 * @param t         Value used to interpolate between a and b.
 *
 * @return          Returns the interpolated float result between angle `a` and angle `b`,
 *                  based on the interpolation value `t`.
 */
stock Float: _xs_vec_lerp_angle(Float: a, Float: b, const Float: t) {
    while (a < -180.0)
        a += 360.0;
    while (a > 180.0)
        a -= 360.0;

    while (b < -180.0)
        b += 360.0;
    while (b > 180.0)
        b -= 360.0;

    new Float: angleDiff = b - a;

    if (angleDiff > 180.0)
        angleDiff -= 360.0;
    else if (angleDiff < -180.0)
        angleDiff += 360.0;

    new Float: result = a + t * angleDiff;

    while (result < -180.0)
        result += 360.0;
    while (result > 180.0)
        result -= 360.0;

    return result;
}

/**
 * Converts a vector represented as an array of floats to a formatted string.
 *
 * @param vec       The vector represented as an array of floats.
 * @param output    The output string where the formatted vector will be stored.
 * @param len       The maximum length of the output string.
 * @param precision The number of decimal places for each vector component.
 * @param size      The size of the vector array.
 *
 * @return          Returns the number of characters written to the output string.
 *                  It is the length of the formatted vector string.
 */
stock _xs_vec_to_string(const Float: vec[], output[], const len, const precision = 1, const size = sizeof(vec)) {
    new _len = copy(output, len, "(");

    for (new i = 0; i < size && _len < len; ++i) {
        _len += format(output[_len], (len - _len),
            fmt("%%.%if%s", precision, (i < size - 1) ? ", " : ")"),
            vec[i]
        );
    }

    return _len;
}

stock const XS_X = 0;
stock const XS_Y = 1;
stock const XS_Z = 2;

// Extend ConVar flags.
enum _: ConVar_ExtendedFlags (<<= 1) {
    _FCVAR_BOOLEAN = (FCVAR_NOEXTRAWHITEPACE << 1), /**< Can contain boolean value */
    _FCVAR_STRING,                                  /**< Can contain string value */
    _FCVAR_FLOAT,                                   /**< Can contain float value */
    _FCVAR_INTEGER,                                 /**< Can contain integer value */
}

stock GetCvarTypeStr(const /* ConVar_ExtendedFlags: */ flags) {
    new buffer[32]

    if (flags & _FCVAR_BOOLEAN)
        copy(buffer, charsmax(buffer), "boolean")

    if (flags & _FCVAR_STRING)
        copy(buffer, charsmax(buffer), "string")

    if (flags & _FCVAR_FLOAT)
        copy(buffer, charsmax(buffer), "float")
    
    if (flags & _FCVAR_INTEGER)
        copy(buffer, charsmax(buffer), "integer")
    
    return buffer
}

enum Bounds_s {
    bool: b_hasBound,
    Float: b_value
}

/**
 * Gets the number of digits in a given number.
 *
 * @param number   The number to get the digit count for.
 * @return         The number of digits in the given number.
 */
stock UTIL_getDigitsCount(number) {
    return 1 + floatround(
        floatlog(
            floatabs(
                float(number)
            )
        ),
        floatround_floor
    )
}
