#include "ReDeathmatch/Features/AimBarriers.inc"
#include "ReDeathmatch/Features/Tickets.inc"

static g_fwdPrecacheEvent = -1
static g_gunsEventsId
static g_oldGroupinfo[MAX_PLAYERS + 1]
static bool: g_protectionState[MAX_PLAYERS + 1]
static Float: g_nextPlayerChooseTeam[MAX_PLAYERS + 1]

static bool: redm_block_drop_weapon
static Float: redm_sounds_distance
static Float: redm_healer
static Float: redm_healer_hs
static redm_fade
static redm_fade_colors[32]
static redm_refill_ammo
static redm_hitsound
static mp_damage_headshot_only
static bool: redm_hide_other_deathnotice
static redm_protection_color_t[32]
static redm_protection_color_ct[32]
static bool: mp_respawn_immunity_effects
static bool: redm_changeteam_unlimited
static Float: redm_changeteam_freq
static bool: redm_hide_hostages

Features_Precache() {
    AimBarriers_Precache()

    g_fwdPrecacheEvent = register_forward(FM_PrecacheEvent , "PrecacheEvent_Post", ._post = true)
}

Features_Init() {
    register_message(get_user_msgid("HudTextArgs"), "MsgHook_HudTextArgs")
    register_message(get_user_msgid("DeathMsg"), "MsgHook_DeathMsg")

    unregister_forward(FM_PrecacheEvent, g_fwdPrecacheEvent, .post = true)
    register_forward(FM_PlaybackEvent, "PlaybackEvent")
    register_forward(FM_PlaybackEvent, "PlaybackEvent_Post", ._post = true)

    RegisterHam(Ham_Touch, "game_player_equip", "BlockMapAutoEquipEntites", .Post = false)
    RegisterHam(Ham_Use, "game_player_equip", "BlockMapAutoEquipEntites", .Post = false)
    RegisterHam(Ham_Use, "player_weaponstrip", "BlockMapAutoEquipEntites", .Post = false)

    RegisterHookChain(RG_CBasePlayer_TakeDamage, "CBasePlayer_TakeDamage_Post", .post = true)
    RegisterHookChain(RG_CBasePlayer_TraceAttack, "CBasePlayer_TraceAttack", .post = false)
    RegisterHookChain(RG_CBasePlayer_SetSpawnProtection, "CBasePlayer_SetSpawnProtection_Post", .post = true)
    RegisterHookChain(RG_CBasePlayer_RemoveSpawnProtection, "CBasePlayer_RemoveSpawnProtection", .post = true)
    RegisterHookChain(RG_CBasePlayer_DropPlayerItem, "CBasePlayer_DropPlayerItem", .post = false)
    RegisterHookChain(RG_ShowVGUIMenu, "ShowVGUIMenu_Pre", .post = false)
    RegisterHookChain(RG_HandleMenu_ChooseTeam, "HandleMenu_ChooseTeam_Pre", .post = false)
    RegisterHookChain(RG_HandleMenu_ChooseTeam, "HandleMenu_ChooseTeam", .post = true)
    RegisterHookChain(RG_CSGameRules_RestartRound, "CSGameRules_RestartRound_Post", .post = true)

    AimBarriers_Init()
    Tickets_Init()

    bind_pcvar_num(
        create_cvar(
            "redm_block_drop_weapon", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Block the ability to drop the weapon."
        ),
        redm_block_drop_weapon
    )
    bind_pcvar_float(
        create_cvar(
            "redm_healer", "10",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_FLOAT,
            .description = "How much to restore HP when killing an enemy."
        ),
        redm_healer
    )
    bind_pcvar_float(
        create_cvar(
            "redm_healer_hs", "20",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_FLOAT,
            .description = "How much to restore HP when killing with HS an enemy."
        ),
        redm_healer_hs
    )
    bind_pcvar_float(
        create_cvar(
            "redm_sounds_distance", "1500",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_FLOAT,
            .description = "Mute the sounds of other players at a distance."
        ),
        redm_sounds_distance
    )
    bind_pcvar_num(
        create_cvar(
            "redm_fade", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Highlight the display when you kill someone."
        ),
        redm_fade
    )
    bind_pcvar_string(
        create_cvar(
            "redm_fade_colors", "0 200 0 50",
            .flags = _FCVAR_STRING,
            .description = "Kill highlights color [RGBA]"
        ),
        redm_fade_colors, charsmax(redm_fade_colors)
    )
    bind_pcvar_num(
        create_cvar(
            "redm_refill_ammo", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Refill ammo on kills."
        ),
        redm_refill_ammo
    )
    bind_pcvar_num(
        create_cvar(
            "redm_hitsound", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Play a sound when an enemy is hit."
        ),
        redm_hitsound
    )
    bind_pcvar_num(
        create_cvar(
            "mp_damage_headshot_only", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Enable hits only in HS."
        ),
        mp_damage_headshot_only
    )
    bind_pcvar_num(
        create_cvar(
            "redm_hide_other_deathnotice", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Hide other players' killfeed."
        ),
        redm_hide_other_deathnotice
    )
    bind_pcvar_string(
        create_cvar(
            "redm_protection_color_t", "200 0 0 15",
            .flags = _FCVAR_STRING,
            .description = "On spawn T protection colors [RGBA]"
        ),
        redm_protection_color_t, charsmax(redm_protection_color_t)
    )
    bind_pcvar_string(
        create_cvar(
            "redm_protection_color_ct", "0 0 200 15",
            .flags = _FCVAR_STRING,
            .description = "On spawn CT protection colors [RGBA]"
        ),
        redm_protection_color_ct, charsmax(redm_protection_color_ct)
    )

    bind_pcvar_num(get_cvar_pointer("mp_respawn_immunity_effects"), mp_respawn_immunity_effects)

    bind_pcvar_num(
        create_cvar(
            "redm_changeteam_unlimited",
            "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Remove limits on player's team switching."
        ),
        redm_changeteam_unlimited
    )
    bind_pcvar_float(
        create_cvar(
            "redm_changeteam_freq",
            "2.0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 240.0,
            .flags = _FCVAR_FLOAT,
            .description = "Limit the frequency of team changes. ^n\
                In seconds."
        ),
        redm_changeteam_freq
    )
    bind_pcvar_num(
        create_cvar(
            "redm_hide_hostages",
            "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Hide hostages."
        ),
        redm_hide_hostages
    )
}

public MsgHook_HudTextArgs() {
    return IsActive() ? PLUGIN_HANDLED : PLUGIN_CONTINUE
}

public MsgHook_DeathMsg(msgid, dest, receiver) {
    enum { arg_killer = 1, arg_victim, arg_headshot, arg_weapon_name }

    if (!IsActive())
        return PLUGIN_CONTINUE

    if (!redm_hide_other_deathnotice)
        return PLUGIN_CONTINUE

    new killer = get_msg_arg_int(arg_killer)
    new victim = get_msg_arg_int(arg_victim)
    new headshot = get_msg_arg_int(arg_headshot)
    new killerWeaponName[64]
    get_msg_arg_string(arg_weapon_name, killerWeaponName, charsmax(killerWeaponName))

    for(new p = 1; p <= MaxClients; p++) {
        // check player settings

        if (p != killer && p != victim)
            continue

        if (!is_user_connected(p) || is_user_bot(p))
            continue

        UTIL_DeathMsg(MSG_ONE, p, killer, victim, headshot, killerWeaponName)
    }

    new observers[MAX_PLAYERS - 1]
    new count = GetPlayerObservers(killer, observers)
    for (new o; o < count; o++) {
        UTIL_DeathMsg(MSG_ONE, observers[o], killer, victim, headshot, killerWeaponName)
    }

    count = GetPlayerObservers(victim, observers)
    for (new o; o < count; o++) {
        UTIL_DeathMsg(MSG_ONE, observers[o], killer, victim, headshot, killerWeaponName)
    }

    return PLUGIN_HANDLED
}

/**
 * Retrieves the list of players who are currently observing a specified player.
 *
 * This function identifies and returns a list of players who are currently observing the given player.
 * Observers are typically other players who are spectating the target player in first-person mode.
 * The list of observers is stored in the 'observers' array.
 *
 * @param player     The player for whom to retrieve the list of observers.
 * @param observers  An array to store the list of observers' player IDs. It should be pre-allocated and have a maximum size of (MAX_PLAYERS - 1).
 *
 * @return The number of observers found for the specified player.
 *
 * @remark To use this function, make sure to pre-allocate the 'observers' array with enough space
 *         to store the potential maximum number of observers, which is (MAX_PLAYERS - 1).
 */
static GetPlayerObservers(const player, observers[MAX_PLAYERS - 1]) {
    new count

    for (new o = 1; o <= MaxClients; o++) {
        if (o == player)
            continue

        if (!is_user_connected(o) || is_user_bot(o))
            continue

        if (get_entvar(o, var_iuser1) != OBS_IN_EYE)
            continue
        
        new observerTarget = get_member(o, m_hObserverTarget)
        if (observerTarget != player)
            continue

        observers[count++] = o
    }

    return count
}

public PrecacheEvent_Post(type, const name[]) {
    new const gunsEvents[][] = {
        "events/awp.sc", "events/g3sg1.sc", "events/ak47.sc",
        "events/scout.sc", "events/m249.sc", "events/m4a1.sc",
        "events/sg552.sc", "events/aug.sc", "events/sg550.sc",
        "events/m3.sc", "events/xm1014.sc", "events/usp.sc",
        "events/mac10.sc", "events/ump45.sc", "events/fiveseven.sc",
        "events/p90.sc", "events/deagle.sc", "events/p228.sc",
        "events/glock18.sc", "events/mp5n.sc", "events/tmp.sc",
        "events/elite_left.sc", "events/elite_right.sc", "events/galil.sc",
        "events/famas.sc"
    }

    for (new i = 0; i < sizeof(gunsEvents); i++) {
        if (strcmp(gunsEvents[i], name))
            continue
        
        g_gunsEventsId |= (1 << get_orig_retval())
        return FMRES_HANDLED
    }

    return FMRES_IGNORED
}

public PlaybackEvent(flags, invoker, eventid, Float: delay, Float: Origin[3], Float: Angles[3], Float: fparam1, Float: fparam2, iparam1, iparam2, bparam1, bparam2) {
    if (!IsActive())
        return

    if (redm_sounds_distance <= 0.0)
        return   

    if (!IsGunsEvent(eventid))
        return

    if (invoker < 1 || invoker > MaxClients)
        return

    g_oldGroupinfo[invoker] = pev(invoker, pev_groupinfo)

    set_pev(invoker, pev_groupinfo, 1)

    for (new i = 1; i <= MaxClients; i++) {
        if (i == invoker)
            continue

        if (fm_entity_range(i, invoker) < redm_sounds_distance)
            continue

        g_oldGroupinfo[i] = pev(i, pev_groupinfo)
        set_pev(i, pev_groupinfo, 2)
    }
}

public PlaybackEvent_Post(flags, invoker, eventid, Float: delay, Float: Origin[3], Float: Angles[3], Float: fparam1, Float: fparam2, iparam1, iparam2, bparam1, bparam2) {
    if (!IsActive())
        return

    if (redm_sounds_distance <= 0.0)
        return

    if (!IsGunsEvent(eventid))
        return

    if (invoker < 1 || invoker > MaxClients)
        return

    // TODO: refactor that shit
    set_pev(invoker, pev_groupinfo, g_oldGroupinfo[invoker])

    for (new i = 1; i <= MaxClients; i++) {
        if (i == invoker)
            continue

        if (fm_entity_range(i, invoker) < redm_sounds_distance)
            continue

        set_pev(i, pev_groupinfo,  g_oldGroupinfo[i])
    }
}

static bool: IsGunsEvent(const eventId) {
    return (g_gunsEventsId & (1 << eventId)) != 0
}

public BlockMapAutoEquipEntites() {
    return IsActive() ? HAM_SUPERCEDE : HAM_IGNORED
}

public CBasePlayer_TakeDamage_Post(const victim, const inflictor, const attacker, const Float: damage, const bitsDamageType) {
    if (!IsActive())
        return

    if (!attacker || attacker == victim)
        return

    if (!rg_is_player_can_takedamage(victim, attacker))
        return

    if (is_user_bot(attacker))
        return

    if (redm_hitsound) {
        UTIL_PlaySoundEffect(
            attacker,
            "buttons/bell1.wav",
            0.6,
            get_member(victim, m_LastHitGroup) == HITGROUP_HEAD ? 240 : 200 
        )
    }
}

Features_PlayerKilled(const victim, const killer) {
    if (redm_healer + redm_healer_hs > 0.0) {
        new bool: isHeadshot = bool: get_member(victim, m_bHeadshotKilled)
        ExecuteHamB(Ham_TakeHealth, killer, isHeadshot ? redm_healer_hs : redm_healer, DMG_GENERIC)
    }

    if (!is_user_connected(killer))
        return

    if (redm_refill_ammo) {
        UTIL_ReloadWeapons(killer, .currentWeapon = (redm_refill_ammo == 1))
    }

    if (is_user_bot(killer))
        return

    if (redm_fade) {
        UTIL_ScreenFade(killer, .color = ParseColors4(redm_fade_colors), .fadeFlags = FFADE_IN)
    }
}

public CBasePlayer_TraceAttack(victim, attacker, Float: damage, Float: vecDir[3], tracehandle, bitsDamageType) {
    if (!IsActive())
        return HC_CONTINUE

    if (!mp_damage_headshot_only)
        return HC_CONTINUE

    if (!(bitsDamageType & DMG_BULLET))
        return HC_CONTINUE

    if (GetCurrentWeapon(attacker) == WEAPON_KNIFE)
        return HC_CONTINUE

    new bool: hitHead = get_tr2(tracehandle, TR_iHitgroup) == HIT_HEAD
    return hitHead ? HC_CONTINUE : HC_SUPERCEDE
}

public CBasePlayer_SetSpawnProtection_Post(const player, const Float: time) {
    if (!IsActive())
        return

    if (!mp_respawn_immunity_effects)
        return

    new TeamName: team = get_member(player, m_iTeam)
    if (team != TEAM_CT && team != TEAM_TERRORIST)
        return

    new colors[4]
    colors = ParseColors4(team == TEAM_CT ? redm_protection_color_ct : redm_protection_color_t)
  
    fm_set_rendering(
        player,
        .r = colors[0],
        .g = colors[1],
        .b = colors[2],
        .amount = colors[3],
        .fx = kRenderFxGlowShell,
        .render = kRenderNormal
    )

    g_protectionState[player] = true
}

public CBasePlayer_RemoveSpawnProtection(const player) {
    if (!g_protectionState[player])
        return

    g_protectionState[player] = false
    fm_set_rendering(player)
}

public CBasePlayer_DropPlayerItem(const player, const itemName[]) {
    if (!IsActive())
        return HC_CONTINUE

    if (!redm_block_drop_weapon)
        return HC_CONTINUE

    client_printex(player, print_center, "#Weapon_Cannot_Be_Dropped")

    SetHookChainReturn(ATYPE_INTEGER, 0)
    return HC_SUPERCEDE
}

public ShowVGUIMenu_Pre(const player, VGUIMenu: menuType, bitsSlots, oldMenu[])
{
    if (!redm_changeteam_unlimited)
        return

    if (menuType != VGUI_Menu_Team)
        return

    // Force `old menu` style for better control
    set_member(player, m_bForceShowMenu, true)

    // Force `6. Spectate` case
    SetHookChainArg(3, ATYPE_INTEGER, (MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_5 | MENU_KEY_6 | MENU_KEY_0))
    SetHookChainArg(4, ATYPE_STRING, "#IG_Team_Select_Spect")
}

public HandleMenu_ChooseTeam_Pre(const player, const MenuChooseTeam: slot) {
    if (!redm_changeteam_unlimited)
        return HC_CONTINUE

    if (get_gametime() < g_nextPlayerChooseTeam[player]) {
        client_printex(player, print_center, "#Only_1_Team_Change")

        new bool: shouldMenuClose = false
        SetHookChainReturn(ATYPE_INTEGER, shouldMenuClose)
        return HC_SUPERCEDE
    }

    // Remove gamedll restrictions
    GameDLL_Hack(player, true)

    return HC_CONTINUE
}
public HandleMenu_ChooseTeam(const player, const MenuChooseTeam: slot) {
    if (!redm_changeteam_unlimited)
        return

    // Revert gamedll workflow
    GameDLL_Hack(player, false)

    new bool: teamChanged = GetHookChainReturn(ATYPE_INTEGER)
    if (!teamChanged)
        return

    if (!get_member(player, m_bJustConnected))
        g_nextPlayerChooseTeam[player] = get_gametime() + redm_changeteam_freq
}

static GameDLL_Hack(const player, const bool: apply) {
    static bool: old_freezePeriod
    if (apply) {
        old_freezePeriod = get_member_game(m_bFreezePeriod)
        set_member_game(m_bFreezePeriod, true)
    } else {
        set_member_game(m_bFreezePeriod, old_freezePeriod)
        set_member(player, m_bTeamChanged, false)
    }
}

stock WeaponIdType: GetCurrentWeapon(const player) {
    new activeItem = get_member(player, m_pActiveItem)
    if (!activeItem)
        return WEAPON_NONE

    return WeaponIdType: get_member(activeItem, m_iId)
}

public CSGameRules_RestartRound_Post() {
    if (!IsActive())
        return

    if (redm_hide_hostages)
        HideHostages()
}

HideHostages() {
    new entity = NULLENT
    while ((entity = fm_find_ent_by_class(entity, "hostage_entity"))) {
        new Float: origin[3]
        get_entvar(entity, var_origin, origin)
        xs_vec_add(origin, Float: {0.0, 0.0, -8192.0}, origin)
        set_entvar(entity, var_origin, origin)
    }
}
