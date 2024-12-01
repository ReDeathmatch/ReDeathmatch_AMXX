#include <amxmodx>
#include <hamsandwich>
#include <fakemeta>
#include <fakemeta_util>
#include <amxmisc>
#include <json>
#include <easy_http>

#include <reapi>
#include <redm>

#tryinclude <msgstocks>

#pragma dynamic (8192 + 4096)

#include "ReDeathmatch/ReDM_config.inc"
#include "ReDeathmatch/ReDM_cvars_handler.inc"
#include "ReDeathmatch/ReDM_spawn_manager.inc"
#include "ReDeathmatch/ReDM_equip_manager.inc"
#include "ReDeathmatch/ReDM_features.inc"
#include "ReDeathmatch/ReDM_round_modes.inc"
#include "ReDeathmatch/ReDM_api.inc"
#include "ReDeathmatch/ReDM_updater.inc"


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

    create_cvar("redm_version", REDM_VERSION, FCVAR_SERVER)

    if (!is_regamedll()) {
        LogMessageEx(Fatal, "^n    ReGameDLL not found!")
        return
    }

    RegisterHookChain(RG_RoundEnd, "RoundEnd_Post", .post = true)
    RegisterHookChain(RG_CSGameRules_RestartRound, "CSGameRules_RestartRound", .post = false)
    RegisterHookChain(RG_CSGameRules_PlayerKilled, "CSGameRules_PlayerKilled_Post", .post = true)

    bind_pcvar_num(
        create_cvar(
            "redm_active", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Controls the state of Re:DM.^n\
                Don't use into ReDM configs!"
        ),
        redm_active
    )
    hook_cvar_change(get_cvar_pointer("redm_active"), "CvarChange_redm_active")

    register_concmd("redm_enable", "ConCmd_redm_enable", ADMIN_MAP, "Enables Re:DM.")
    register_concmd("redm_disable", "ConCmd_redm_disable", ADMIN_MAP, "Disables Re:DM.")
    register_concmd("redm_status", "ConCmd_redm_status", ADMIN_MAP, "Get Re:DM status.")
    register_concmd("redm", "ConCmd_redm", ADMIN_MAP, "Get info.", .FlagManager = false)

    ApiInit_Forwards()
    Updater_Init()
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
        EquipManager_Init()
        Config_Init()
        RoundModes_Init()

        SetActive(redm_active)
    }
    CallApi_Initialized()
    Updater_Check()
}

public plugin_end() {
    if (!IsActive())
        return

    CvarsHandler_RestoreValue()
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

public plugin_natives() {
    ApiInit_Natives()
    Updater_Natives()
}

public client_putinserver(player) {
    if (!IsActive())
        return

    EquipManager_PutInServer(player)
}

public client_disconnected(player, bool: drop, message[], maxLen) {
    if (!IsActive())
        return

    ModeVote_Disconnected(player)
}

public RoundEnd_Post(WinStatus: status, ScenarioEventEndRound: event, Float: tmDelay) {
    if (!IsActive())
        return
    
    RoundModes_RoundEnd()
}

public CSGameRules_RestartRound() {
    if (!IsActive())
        return

    RoundModes_RestartRound()
    Tickets_RestartRound()
}

public CSGameRules_PlayerKilled_Post(const victim, const killer, const inflictor) {
    if (!IsActive())
        return
    
    if (!SV_IsPlayerIndex(killer))
        return

    EquipManager_PlayerKilled(victim)
    if (killer == victim)
        return

    Features_PlayerKilled(victim, killer)
}

public CvarChange_redm_active(const cvar, const oldValue[], const value[]) {
    SetActive(strtol(value) != 0)
}

public ConCmd_redm_enable(const player, const level, const commandId) {
    SetGlobalTransTarget(player)

    if (!cmd_access(player, level, commandId, 1))
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

public ConCmd_redm_disable(const player, const level, const commandId) {
    SetGlobalTransTarget(player)
    
    if (!cmd_access(player, level, commandId, 1))
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

public ConCmd_redm_status(const player, const level, const commandId) {
    SetGlobalTransTarget(player)

    console_print(player, "* ReDeathmatch %l - `%l`.", 
        "Currently",
        IsActive() ? "Enabled" : "Disabled"
    )

    return PLUGIN_HANDLED
}

public ConCmd_redm(const player, const level, const commandId) {
    SetGlobalTransTarget(player)

    console_print(player, "[Re:DM] Version `%s`", REDM_VERSION)
    console_print(player, "[Re:DM] https://ReDeathmatch.github.io")

    new bool: isManualBuild = (strfind(REDM_VERSION, "manual") != -1)
    if (!isManualBuild) {
        new bool: isRelease = (strlen(REDM_VERSION_PATCH) < 4)

        console_print(player,
            "[Re:DM] https://github.com/ReDeathmatch/ReDeathmatch_AMXX/%s/%s",
            isRelease ? "releases/tag" : "commit",
            isRelease ? REDM_VERSION : REDM_VERSION_PATCH
        )
    }

    console_print(player, "[Re:DM] Copyright (c) 2024 Sergey Shorokhov")

    if (!cmd_access(player, level, commandId, 1, .accesssilent = true))
        return PLUGIN_HANDLED

    console_print(player, "Compilation info:")
    console_print(player, "  AMXX version: `%s`", AMXX_VERSION_STR)
    console_print(player, "  ReAPI version: `%i.%i`", REAPI_VERSION_MAJOR, REAPI_VERSION_MINOR)
    console_print(player, "  Time: `%s %s (GMT)`", __DATE__, __TIME__)

    return PLUGIN_HANDLED
}

/**
 * Checks if the Re:DM mode is currently active.
 *
 * @return              Returns true if Re:DM is active, false otherwise.
 */
bool: IsActive() {
    return redm_active
}

/**
 * Sets the Re:DM mode to active or inactive.
 *
 * @param active        If true, activates Re:DM; if false, deactivates it.
 */
SetActive(const bool: active) {
    CallApi_ChangeState(active)

    ApplyState(active)

    set_cvar_num("redm_active", active ? 1 : 0)
}

/**
 * Applies the Re:DM mode state, either activating or deactivating it.
 *
 * @param active        If true, activates Re:DM; if false, deactivates it.
 */
static ApplyState(const bool: active) {
    if (active) {
        Config_Reload()
        set_member_game(m_bGameStarted, true)
    } else {
        RoundModes_ResetCurrentMode()
        CvarsHandler_RestoreValue()
    }

    set_cvar_num("sv_restart", 1)
}
