static g_fwdPrecacheEvent = -1
static g_gunsEventsId
static g_oldGroupinfo[MAX_PLAYERS + 1]

static Float: redm_sounds_distance
static Float: redm_healer
static Float: redm_healer_hs
static redm_fade
static redm_fade_colors[32]
static redm_refill_ammo
static redm_hitsound
static mp_damage_headshot_only
static bool: redm_hide_other_deathnotice


Features_Precache() {
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
    // RegisterHookChain(RH_SV_StartSound, "SV_StartSound", .post = false)

    bind_pcvar_float(create_cvar("redm_healer", "10.0"), redm_healer)
    bind_pcvar_float(create_cvar("redm_healer_hs", "20.0"), redm_healer_hs)
    bind_pcvar_float(create_cvar("redm_sounds_distance", "1500.0"), redm_sounds_distance)
    bind_pcvar_num(create_cvar("redm_fade", "1"), redm_fade)
    bind_pcvar_string(create_cvar("redm_fade_colors", "0 200 0 50"), redm_fade_colors, charsmax(redm_fade_colors))
    bind_pcvar_num(create_cvar("redm_refill_ammo", "1"), redm_refill_ammo)
    bind_pcvar_num(create_cvar("redm_hitsound", "1"), redm_hitsound)
    bind_pcvar_num(create_cvar("mp_damage_headshot_only", "0"), mp_damage_headshot_only)
    bind_pcvar_num(create_cvar("redm_hide_other_deathnotice", "1"), redm_hide_other_deathnotice)
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

public Features_PlayerKilled(const victim, const killer) {
    if (redm_healer + redm_healer_hs > 0.0) {
        new bool: isHeadshot = bool: get_member(victim, m_bHeadshotKilled)
        ExecuteHamB(Ham_TakeHealth, killer, isHeadshot ? redm_healer_hs : redm_healer, DMG_GENERIC)
    }

    if (redm_refill_ammo) {
        UTIL_ReloadWeapons(killer, .currentWeapon = (redm_refill_ammo == 1))
    }

    if (is_user_bot(killer))
        return

    if (redm_fade) {
        UTIL_ScreenFade(killer, .color = ParseColors4(redm_fade_colors))
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

stock WeaponIdType: GetCurrentWeapon(const player) {
    new activeItem = get_member(player, m_pActiveItem)
    if (!activeItem)
        return WEAPON_NONE

    return WeaponIdType: get_member(activeItem, m_iId)
}

public SV_StartSound(const recipients, const entity, const channel, const sample[], const volume, Float:attenuation, const fFlags, const pitch) {
    /* server_print("s:`%s`, r:%i, att:%.2f, pitch:%i",
        sample, recipients, attenuation, pitch
    ) */
}

static stock UTIL_DeathMsg(const dest, const receiver, const killer, const victim, const headshot, const weaponName[]) {
  static msg_deathMsg
  if(!msg_deathMsg)
    msg_deathMsg = get_user_msgid("DeathMsg")

  message_begin(dest, msg_deathMsg, _, receiver)
  write_byte(killer)
  write_byte(victim)
  write_byte(headshot)
  write_string(weaponName)
  message_end()
}

