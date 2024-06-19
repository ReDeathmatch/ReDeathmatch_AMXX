#if defined CSDM_VERSION
    #undef CSDM_VERSION
    #define	CSDM_VERSION REDM_VERSION
#endif

static g_fwdInit
static g_fwdCfgInit
static g_fwdPreDeath
static g_fwdPostDeath
static g_fwdPreSpawn
static g_fwdPostSpawn
static g_fwdRoundRestart
static g_fwdStateChange
static g_fwdRemoveWeapon
static g_fwdHandleDrop

APICompat_Init() {
    register_native("csdm_active", "@csdm_active")
    register_native("csdm_set_active", "@csdm_set_active")
    register_native("csdm_csdm_get_ffa", "@csdm_get_ffa")
    register_native("csdm_csdm_set_ffa", "@csdm_set_ffa")
    register_native("csdm_get_spawnwait", "@csdm_get_spawnwait")
    register_native("csdm_set_spawnwait", "@csdm_set_spawnwait")
    register_native("csdm_reload_cfg", "@csdm_reload_cfg")
    register_native("csdm_remove_weapon", "@csdm_remove_weapon")
    register_native("csdm_force_drop", "@csdm_force_drop")
    register_native("csdm_give_item", "@csdm_give_item")
    register_native("csdm_reg_cfg", "@csdm_reg_cfg")
    register_native("csdm_fakespawn", "@csdm_fakespawn")
    register_native("csdm_respawn", "@csdm_respawn")
    
    register_native("csdm_curstyle", "@csdm_curstyle")
    register_native("csdm_setstyle", "@csdm_setstyle")
    register_native("csdm_addstyle", "@csdm_addstyle")
    register_native("csdm_styleinfo", "@csdm_styleinfo")
    register_native("csdm_spawnstyles", "@csdm_spawnstyles")

    register_native("csdm_setpos", "@csdm_setpos")
    register_native("csdm_getpos", "@csdm_getpos")

    register_native("csdm_cache", "@csdm_cache")
    register_native("csdm_trace_hull", "@csdm_trace_hull")
    register_native("csdm_set_intromsg", "@csdm_set_intromsg")
    register_native("csdm_main_menu", "@csdm_main_menu")
    register_native("csdm_set_mainoption", "@csdm_set_mainoption")
    register_native("csdm_fwd_drop", "@csdm_fwd_drop")
}

APICompat_Forwards() {
    // g_fwdGetConfigPrefixName = CreateMultiForward("ReDM_GetConfigPrefixName", ET_IGNORE, FP_ARRAY, FP_CELL, FP_STRING)
    
    g_fwdInit = CreateMultiForward("csdm_Init", ET_IGNORE, FP_STRING)
    g_fwdCfgInit = CreateMultiForward("csdm_CfgInit", ET_IGNORE)
    g_fwdPreDeath = CreateMultiForward("csdm_PreDeath", ET_IGNORE, FP_CELL, FP_CELL, FP_CELL, FP_STRING)
    g_fwdPostDeath = CreateMultiForward("csdm_PostDeath", ET_IGNORE, FP_CELL, FP_CELL, FP_CELL, FP_STRING)
    g_fwdPreSpawn = CreateMultiForward("csdm_PreSpawn", ET_STOP, FP_CELL, FP_CELL)
    g_fwdPostSpawn = CreateMultiForward("csdm_PostSpawn", ET_STOP, FP_CELL, FP_CELL)
    g_fwdRoundRestart = CreateMultiForward("csdm_RoundRestart", ET_IGNORE, FP_CELL)
    g_fwdStateChange = CreateMultiForward("csdm_StateChange", ET_IGNORE, FP_CELL)
    g_fwdRemoveWeapon = CreateMultiForward("csdm_RemoveWeapon", ET_STOP, FP_CELL, FP_CELL, FP_CELL)
    g_fwdHandleDrop = CreateMultiForward("csdm_HandleDrop", ET_CONTINUE, FP_CELL, FP_CELL, FP_CELL)
}

public bool: @csdm_active(const plugin_id, const argc) {
    return native_redm_is_active(plugin_id, argc)
}

public bool: @csdm_set_active(const plugin_id, const argc) {
    return native_redm_set_active(plugin_id, argc)
}

public bool: @csdm_get_ffa(const plugin_id, const argc) {
    return get_cvar_num("mp_freeforall") != 0
}

public bool: @csdm_set_ffa(const plugin_id, const argc) {
    new bool: active = get_param(1) != 0
    set_cvar_num("mp_freeforall", active)
}

public Float: @csdm_get_spawnwait(const plugin_id, const argc) {
    return get_cvar_float("mp_forcerespawn")
}

public @csdm_set_spawnwait(const plugin_id, const argc) {
    new Float: time = get_param_f(1)
    set_cvar_float("mp_forcerespawn", time)
}

public bool: @csdm_reload_cfg(const plugin_id, const argc) {
    new filePath[PLATFORM_MAX_PATH]
    get_string(1, filePath, charsmax(filePath))

    server_cmd("redm_reload ^"%s^"", filePath)
}

public bool: @csdm_remove_weapon(const plugin_id, const argc) {
    // TODO: Implement

    // new player = get_param(1)
    // new weapon[64]
    // get_string(2, weapon, charsmax(weapon))

    
    // new seconds = get_param(3)
    // new delay = get_param(4)
    
    // set_task(float(delay), "@RemoveWeapon")
}

public bool: @csdm_force_drop(const plugin_id, const argc) {
    // TODO: Implement

    // new player = get_param(1)
    // new weapon[64]
    // get_string(2, weapon, charsmax(weapon))

    return false
}

public bool: @csdm_give_item(const plugin_id, const argc) {
    // TODO: Implement
    // new player = get_param(1)
    // new weapon[64]
    // get_string(2, weapon, charsmax(weapon))

    return false
}

public bool: @csdm_reg_cfg(const plugin_id, const argc) {
    // TODO: Implement

    return false
}

public bool: @csdm_fakespawn(const plugin_id, const argc) {
    // TODO: Implement

    return false
}

public bool: @csdm_respawn(const plugin_id, const argc) {
    // TODO: Implement

    return false
}

public @csdm_curstyle(const plugin_id, const argc) {
    return native_redm_curstyle(plugin_id, argc)
}

public bool: @csdm_setstyle(const plugin_id, const argc) {
    return native_redm_setstyle(plugin_id, argc)
}

public @csdm_addstyle(const plugin_id, const argc) {
    return native_redm_addstyle(plugin_id, argc)
}

public @csdm_styleinfo(const plugin_id, const argc) {
    return native_redm_styleinfo(plugin_id, argc)
}

public @csdm_spawnstyles(const plugin_id, const argc) {
    return native_redm_spawnstyles(plugin_id, argc)
}

public @csdm_getpos(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_setpos(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_cache(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_trace_hull(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_set_intromsg(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_main_menu(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_set_mainoption(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

public @csdm_fwd_drop(const plugin_id, const argc) {
    // TODO: Implement
    return 0
}

CallCompatApi_Init() {
    ExecuteForward(g_fwdInit)
}

CallCompatApi_CfgInit() {
    ExecuteForward(g_fwdCfgInit)
}

CallCompatApi_Death_Pre() {
    ExecuteForward(g_fwdPreDeath, _)
}

CallCompatApi_Death_Post() {
    ExecuteForward(g_fwdPostDeath, _)
}

CallCompatApi_Spawn_Pre() {
    ExecuteForward(g_fwdPreSpawn, _)
}

CallCompatApi_Spawn_Post() {
    ExecuteForward(g_fwdPostSpawn, _)
}

CallCompatApi_RoundRestart_Pre() {
    ExecuteForward(g_fwdRoundRestart, _, false)
}

CallCompatApi_RoundRestart_Post() {
    ExecuteForward(g_fwdRoundRestart, _, true)
}

CallCompatApi_StateChange(const bool: active) {
    ExecuteForward(g_fwdStateChange, _, active)
}

CallCompatApi_RemoveWeapon() {
    ExecuteForward(g_fwdRemoveWeapon, _)
}

CallCompatApi_HandleDrop() {
    ExecuteForward(g_fwdHandleDrop, _)
}