#include <amxmodx>
#include <hamsandwich>
#include <fakemeta>
#include <fakemeta_util>
#include <amxmisc>
#include <json>

#include <reapi>
#include <redm>

#tryinclude <msgstocks>

#include "ReDeathmatch/ReDM_config.inc"
#include "ReDeathmatch/ReDM_cvars_handler.inc"
#include "ReDeathmatch/ReDM_spawn_manager.inc"
#include "ReDeathmatch/ReDM_equip_manager.inc"
#include "ReDeathmatch/ReDM_features.inc"
#include "ReDeathmatch/ReDM_round_modes.inc"
#include "ReDeathmatch/ReDM_api.inc"


static bool: g_prevState

static const g_soundEffects[][] = {
    "fvox/blip.wav",
    "buttons/button9.wav",
    "buttons/bell1.wav",
}

static bool: redm_active

public plugin_init() {
    register_plugin("ReDeathmatch", REDM_VERSION, "Sergey Shorokhov")
    register_dictionary("redm/redm.txt")

    RegisterHookChain(RG_CSGameRules_PlayerKilled, "CSGameRules_PlayerKilled_Post", .post = true)

    create_cvar("redm_version", REDM_VERSION, (FCVAR_SERVER|FCVAR_SPONLY))

    bind_pcvar_num(
        create_cvar(
            "redm_active", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .description = "Controls the state of Re:DM. \
                Don't use into ReDM configs!"
        ),
        redm_active
    )
    hook_cvar_change(get_cvar_pointer("redm_active"), "CvarChange_redm_active")

    register_concmd("redm_enable", "ConCmd_redm_enable", ADMIN_MAP, "Enables Re:DM.")
    register_concmd("redm_disable", "ConCmd_redm_disable", ADMIN_MAP, "Disables Re:DM.")
    register_concmd("redm_status", "ConCmd_redm_status", ADMIN_MAP, "Get Re:DM status.")
    register_concmd("redm", "ConCmd_redm", ADMIN_ALL, "Get info.")

    ApiInit_Forwards()
}

public plugin_precache() {
    Features_Precache()

    for (new i; i < sizeof(g_soundEffects); i++)
        precache_sound(g_soundEffects[i])
}

public plugin_cfg() {
    CallApi_InitStart()
    {
        CvarsHandler_Init()
        Features_Init()
        SpawnManager_Init()
        RoundModes_Init()
        EquipManager_Init()
        Config_Init()

        SetActive(redm_active)
    }
    CallApi_Initialized()
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

public client_disconnected(player, bool: drop, message[], maxLen) {
    ModeVote_Disconnected(player)
}

public CSGameRules_PlayerKilled_Post(const victim, const killer, const inflictor) {
    if (!IsActive())
        return
    
    if (!killer || killer == victim)
        return
    
    Features_PlayerKilled(victim, killer)
    EquipManager_PlayerKilled(victim, killer)
}

public CvarChange_redm_active(const cvar, const oldValue[], const value[]) {
    SetActive(strtol(value) != 0)
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

    console_print(player, "[Re:DM] Version `%s`", REDM_VERSION)
    console_print(player, "[Re:DM] https://github.com/wopox1337/ReDeathmatch")
    console_print(player, "[Re:DM] Copyright (c) 2023 Sergey Shorokhov", REDM_VERSION)

    return PLUGIN_HANDLED
}

bool: IsActive() {
    return redm_active
}

SetActive(const bool: active) {
    CallApi_ChangeState(active)

    ApplyState(active)

    set_cvar_num("redm_active", active ? 1 : 0)
}

static ApplyState(const bool: active) {
    if (active) {
        ReloadConfig()
        set_member_game(m_bGameStarted, true)
    } else {
        RoundModes_ResetCurrentMode()
        RestoreAllCvars()
    }

    set_cvar_num("sv_restart", 1)
}
