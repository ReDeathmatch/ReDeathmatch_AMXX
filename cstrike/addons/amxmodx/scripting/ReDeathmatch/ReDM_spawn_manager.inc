static Array: g_spawnsManager = Invalid_Array
static g_currentSpawnMethodIdx = -1

enum _: SpawnStyle_s {
    ss_Name[32],
    ss_Callback,
}

static redm_spawn_preset[32]


SpawnManager_Init() {
    bind_pcvar_string(
        create_cvar(
            "redm_spawn_preset",
            "preset",
            .flags = _FCVAR_STRING,
            .description = "Method of work for the respawn manager."
        ),
        redm_spawn_preset,
        charsmax(redm_spawn_preset)
    )
    hook_cvar_change(get_cvar_pointer("redm_spawn_preset"), "CvarChange_redm_spawn_preset")

    RegisterHookChain(RG_CSGameRules_GetPlayerSpawnSpot, "CSGameRules_GetPlayerSpawnSpot", .post = false)

    redm_setstyle(redm_spawn_preset)
}

public CvarChange_redm_spawn_preset(const cvar, const oldValue[], const value[]) {
    new bool: result = redm_setstyle(value)
    if (result)
        return

    set_pcvar_string(cvar, oldValue)
    LogMessageEx(Warning, "CvarChange_redm_spawn_preset: Spawn preset style `%s` not found!", value)
}

public CSGameRules_GetPlayerSpawnSpot(const player) {
    if (!IsActive())
        return HC_CONTINUE

    if (get_member(player, m_bJustConnected))
        return HC_CONTINUE

    if (!SpawnManager_Spawn(player))
        return HC_CONTINUE

    SetHookChainReturn(ATYPE_INTEGER, 0)
    return HC_SUPERCEDE
}

SpawnManager_AddMethod(const name[], const callbackHandle) {
    if (g_spawnsManager == Invalid_Array) {
        g_spawnsManager = ArrayCreate(_: SpawnStyle_s)
    }

    new spawnStyle[SpawnStyle_s]
    copy(spawnStyle[ss_Name], charsmax(spawnStyle[ss_Name]), name)
    spawnStyle[ss_Callback] = callbackHandle

    ArrayPushArray(g_spawnsManager, spawnStyle)

    return (ArraySize(g_spawnsManager) - 1)
}

SpawnManager_Spawn(const player) {
    if (g_currentSpawnMethodIdx == -1)
        return false

    new spawnStyle[SpawnStyle_s]
    ArrayGetArray(
        g_spawnsManager,
        g_currentSpawnMethodIdx,
        spawnStyle,
        sizeof(spawnStyle)
    )

    new bool: ret = false
    ExecuteForward(spawnStyle[ss_Callback], ret, player)

    return ret
}

Array: SpawnManager_Get() {
    return g_spawnsManager
}

SpawnManager_GetCurrentMethodIdx() {
    return g_currentSpawnMethodIdx
}

SpawnManager_SetCurrentMethodIdx(const idx) {
    g_currentSpawnMethodIdx = idx
}
