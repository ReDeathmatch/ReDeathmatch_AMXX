static g_fwdChangeState = -1
static g_fwdInitStart = -1
static g_fwdInitialized = -1
static g_fwdConfigLoaded = -1
static g_fwdGetConfigName = -1
static g_fwdGetConfigPrefixName = -1


public ApiInit_Natives() {
    register_native("redm_is_active", "native_redm_is_active")
    register_native("redm_set_active", "native_redm_set_active")
    register_native("redm_get_config", "native_redm_get_config")
    register_native("redm_get_equip_config", "native_redm_get_equip_config")
    register_native("redm_get_round_mode_id", "native_redm_get_round_mode_id")

    register_native("redm_spawnstyles", "native_redm_spawnstyles")
    register_native("redm_styleinfo", "native_redm_styleinfo")
    register_native("redm_addstyle", "native_redm_addstyle")
    register_native("redm_setstyle", "native_redm_setstyle")
    register_native("redm_curstyle", "native_redm_curstyle")
}

ApiInit_Forwards() {
    g_fwdChangeState = CreateMultiForward("ReDM_ChangeState", ET_IGNORE, FP_CELL)
    g_fwdInitStart = CreateMultiForward("ReDM_InitStart", ET_IGNORE)
    g_fwdInitialized = CreateMultiForward("ReDM_Initialized", ET_IGNORE)
    g_fwdConfigLoaded = CreateMultiForward("redm_config_loaded", ET_IGNORE)
    g_fwdGetConfigName = CreateMultiForward("ReDM_GetConfigName", ET_IGNORE, FP_ARRAY, FP_CELL)
    g_fwdGetConfigPrefixName = CreateMultiForward("ReDM_GetConfigPrefixName", ET_IGNORE, FP_ARRAY, FP_CELL, FP_STRING)
}

/**
 * Retrieves the current activation state of Re:DM.
 *
 * @return              Returns true if Re:DM is active, false otherwise.
 */
public bool: native_redm_is_active(const plugin_id, const argc) {
    return IsActive()
}

/**
 * Sets the activation state of Re:DM.
 *
 * @param active        If true, activates Re:DM; if false, deactivates it.
 * @return              Returns true if the state was successfully changed, false otherwise.
 */
public bool: native_redm_set_active(const plugin_id, const argc) {
    new bool: active = get_param(1) != 0

    if (IsActive() && active)
        return false
    
    if (!IsActive() && !active)
        return false

    SetActive(active)

    return true
}

/**
 * Retrieves the current Re:DM configuration in JSON format.
 *
 * @return              JSON handle representing the Re:DM configuration.
 */
public JSON: native_redm_get_config(const plugin_id, const argc) {
    return Config_GetCurrent()
}

/**
 * Retrieves the current Re:DM equipment configuration in JSON format. (TODO)
 *
 * @return              JSON handle representing the Re:DM equipment configuration.
 */
public JSON: native_redm_get_equip_config(const plugin_id, const argc) {
    return Invalid_JSON // TODO:
}

/**
 * Retrieves the ID of the current Re:DM round mode.
 *
 * @return              ID of the current round mode.
 */
public native_redm_get_round_mode_id(const plugin_id, const argc) {
    return RoundModes_GetCurrentMode()
}

/**
 * Retrieves the number of registered spawn styles.
 *
 * @return              Number of registered spawn styles.
 */
public native_redm_spawnstyles(const plugin_id, const argc) {
    return ArraySize(SpawnManager_Get())
}

/**
 * Retrieves information about a spawn style based on its index.
 *
 * @param idx           The index of the spawn style to retrieve.
 * @return              Returns the name of the spawn style at the specified index.
 */
public native_redm_styleinfo(const plugin_id, const argc) {
    new idx = get_param(1)
    if (idx < 0 || idx >= ArraySize(SpawnManager_Get())) {
        log_error(AMX_ERR_BOUNDS, "Invalid style index provided (%i)", idx)
    }

    new spawnStyle[SpawnStyle_s]
    ArrayGetArray(SpawnManager_Get(), idx, spawnStyle)

    return set_string(2, spawnStyle[ss_Name], get_param(3))
}

/**
 * Adds a spawn style handler to Re:DM.
 *
 * @param name          The name of the spawn style.
 * @param functionName  The name of the function to handle the spawn style.
 * @return              Returns a handle to the added spawn style handler.
 */
public native_redm_addstyle(const plugin_id, const argc) {
    new name[32]
    get_string(1, name, charsmax(name))

    new functionName[32]
    get_string(2, functionName, charsmax(functionName))

    new handle = CreateOneForward(plugin_id, functionName, FP_CELL)
    if (handle < 0) {
        log_error(AMX_ERR_NOTFOUND, "Callback function not found (`%s`)", functionName)
    }

    SpawnManager_AddMethod(name, handle)
    return handle
}

/**
 * Sets the current spawn style for Re:DM.
 *
 * @param name          The name of the spawn style to set as current, or "none" to disable a style.
 * @return              Returns true if the style was successfully set, false otherwise.
 */
public bool: native_redm_setstyle(const plugin_id, const argc) {
    new name[32]
    get_string(1, name, charsmax(name))

    if (strcmp(name, "none") == 0) {
        SpawnManager_SetCurrentMethodIdx(-1)

        return true
    }

    for (new i, size = ArraySize(SpawnManager_Get()); i < size; i++) {
        new spawnStyle[SpawnStyle_s]
        ArrayGetArray(SpawnManager_Get(), i, spawnStyle)

        if (strcmp(spawnStyle[ss_Name], name) == 0) {
            SpawnManager_SetCurrentMethodIdx(i)
            return true
        }
    }

    return false
}

/**
 * Retrieves the ID of the current spawn style in use.
 *
 * @return              ID of the current spawn style.
 */
public native_redm_curstyle(const plugin_id, const argc) {
    return SpawnManager_GetCurrentMethodIdx()
}

CallApi_InitStart() {
    ExecuteForward(g_fwdInitStart)
}

CallApi_Initialized() {
    ExecuteForward(g_fwdInitialized)
}

// TODO
stock CallApi_ConfigLoaded() {
    ExecuteForward(g_fwdConfigLoaded)
}

CallApi_GetConfigName(fileName[], len) {
    ExecuteForward(
        g_fwdGetConfigName, _,
		PrepareArray(
            fileName,
            len,
            .copyback = true
        ),
        len
    )
}

CallApi_GetConfigPrefixName(fileName[], len, const mapPrefix[]) {
    ExecuteForward(
        g_fwdGetConfigPrefixName, _,
		PrepareArray(
            fileName,
            len,
            .copyback = true
        ),
        len,
        mapPrefix
    )
}

CallApi_ChangeState(const bool: active) {
    ExecuteForward(g_fwdChangeState, _, active)
}
