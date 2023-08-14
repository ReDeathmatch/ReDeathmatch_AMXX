#include <amxmodx>
#include <hamsandwich>
#include <fakemeta>
#include <fakemeta_util>
#include <amxmisc>
#include <json>

#include <reapi>
#include <redm>

#include "ReDeathmatch/ReDM_config.inc"
#include "ReDeathmatch/ReDM_cvars_handler.inc"
#include "ReDeathmatch/ReDM_spawn_manager.inc"
#include "ReDeathmatch/ReDM_equip_manager.inc"
#include "ReDeathmatch/ReDM_features.inc"
#include "ReDeathmatch/ReDM_round_modes.inc"
#include "ReDeathmatch/ReDM_api.inc"

static g_pcvar_redm_active

static bool: g_prevState

static const g_soundEffects[][] = {
    "fvox/blip.wav",
    "buttons/button9.wav",
    "buttons/bell1.wav",
}

public plugin_init() {
    register_plugin("ReDeathmatch", VERSION, "Sergey Shorokhov")
    register_dictionary("redm/redm.txt")

    create_cvar("redm_version", VERSION, (FCVAR_SERVER|FCVAR_SPONLY))
    
    ApiInit_Forwards()

    Config_Init()
}

public plugin_precache() {
    Features_Precache()

    for (new i; i < sizeof(g_soundEffects); i++)
        precache_sound(g_soundEffects[i])
}

public plugin_cfg() {
    CvarsHandler_Init()
    CallApi_InitStart()

    Features_Init()
    SpawnManager_Init()
    RoundModes_Init()
    EquipManager_Init()

    RegisterHookChain(RG_CSGameRules_PlayerKilled, "CSGameRules_PlayerKilled_Post", .post = true)

    g_pcvar_redm_active = create_cvar("redm_active", "0", (FCVAR_SERVER|FCVAR_SPONLY))

    register_concmd("redm_enable", "ConCmd_redm_enable", ADMIN_MAP, "Enables Re:DM.")
    register_concmd("redm_disable", "ConCmd_redm_disable", ADMIN_MAP, "Disables Re:DM.")
    register_concmd("redm_status", "ConCmd_redm_status", ADMIN_MAP, "Get Re:DM status.")
    register_concmd("redm", "ConCmd_redm", ADMIN_ALL, "Get info.")

    CallApi_Initialized()

    if (Config_GetCurrent() != Invalid_JSON)
        SetActive(true)
}

public plugin_end() {
    RestoreAllCvars()
}

public plugin_pause() {
    if (!IsActive())
        return

    g_prevState = true
    SetActive(false)
}

public plugin_unpause() {
    if (!g_prevState)
        return

    SetActive(true)
}

public client_putinserver(player) {
    EquipManager_PutInServer(player)
}

public CSGameRules_PlayerKilled_Post(const victim, const killer, const inflictor) {
    if (!IsActive())
        return
    
    if (!killer || killer == victim)
        return
    
    Features_PlayerKilled(victim, killer)
    EquipManager_PlayerKilled(victim, killer)
}

public ConCmd_redm_enable(const player, const level, const cid) {
    SetGlobalTransTarget(player)

    if (!cmd_access(player, level, cid, 1))
        return PLUGIN_HANDLED

    if (IsActive()) {
        console_print(player, " * ReDeathmatch %l - `%l`!",
            "Already", "Enabled"
        )

        return PLUGIN_HANDLED
    }

    SetActive(true)
    return PLUGIN_HANDLED
}

public ConCmd_redm_disable(const player, level, cid) {
    SetGlobalTransTarget(player)
    
    if (!cmd_access(player, level, cid, 1))
        return PLUGIN_HANDLED

    if (!IsActive()) {
        console_print(player, " * ReDeathmatch %l - `%l`!",
            "Already", "Disabled"
        )

        return PLUGIN_HANDLED
    }

    SetActive(false)
    return PLUGIN_HANDLED
}

public ConCmd_redm_status(const player, level, cid) {
    SetGlobalTransTarget(player)

    console_print(player, "* ReDeathmatch %l - `%l`.", 
        "Currently",
        IsActive() ? "Enabled" : "Disabled"
    )

    return PLUGIN_HANDLED
}

public ConCmd_redm(const player, level, cid) {
    SetGlobalTransTarget(player)

    console_print(player, "[Re:DM] Version `%s`", VERSION)
    console_print(player, "[Re:DM] https://github.com/wopox1337/ReDeathmatch")
    console_print(player, "[Re:DM] Copyright (c) 2023 Sergey Shorokhov", VERSION)

    return PLUGIN_HANDLED
}

bool: IsActive() {
    return get_pcvar_num(g_pcvar_redm_active) != 0
}

SetActive(const bool: active) {
    CallApi_ChangeState(active)

    ApplyState(active)

    set_pcvar_num(g_pcvar_redm_active, active ? 1 : 0)
}

static ApplyState(const bool: active) {
    if (active) {
        ReloadConfig()
    } else {
        RoundModes_ResetCurrentMode()
        RestoreAllCvars()
    }

    set_cvar_num("sv_restart", 1)
}
