// #include <amxmodx>
// #include <fakemeta>
// #include <hamsandwich>

// #include <reapi>


// Can't use `trigger` as base class cuz didn't call Spawn() and InitTrigger()
static const g_pointBaseClassname[] = "info_target"
static const g_pointClassname[] = "trigger_capture_area"

const Float: AREA_THINK_TIME = 0.1

enum perTeamData_s {
    iNumRequiredToCap,
    iNumTouching,
    iBlockedTouching,
    bCanCap,
    iSpawnAdjust,
}

enum {
    _pev_lastReductionTime = pev_fuser1,
    _pev_timeRemaining = pev_fuser4,
    _pev_captureTime = pev_fuser3,

    _pev_capturingTeam = pev_team,
    _pev_isCapturing = pev_iuser2,
    _pev_teamInZone = pev_iuser3,
    _pev_owningTeam = pev_owner,
    _pev_pointIdx = pev_modelindex,
    _pev_bBlocked = pev_bInDuck,
    _pev_bActive = pev_controller_0,
    _pev_hPoint = pev_controller_1,
    
    // TODO: rework it later 
    _pev_teamData = pev_noise, // perTeamData_s[] 
}

static Float: g_sequenceRate = 0.8
static Float: g_renderAmount = 100.0

static Float: g_color[TeamName][] = {
    { 100.0, 100.0, 100.0 },
    { 255.0, 100.0, 0.0 },
    { 0.0, 100.0, 255.0 },

    { 0.0, 0.0, 0.0 } // TeamName compability
}

static const g_ControlPointsModels[TeamName][] = {
    "models/player/vip/vip.mdl",
    "models/player/leet/leet.mdl",
    "models/player/gign/gign.mdl",

    "" // TeamName compability
}




#define MAX_CAPTURE_TEAMS TeamName

static mp_simulatemultiplecappers = 1
static mp_capstyle = 0

static const Float: SPAWN_POINT_OFFSET = 10.0 

static Float: redm_capture_spawn_time
static redm_capture_spawn_bar
static Float: redm_capture_spawn_penalty
static Float: redm_capture_spawn_distance
static redm_capture_spawn_changelevel
static redm_capture_spawn_static


CapturePoints_Init() {
    //  RegisterHookChain(RG_CBasePlayer_TakeDamage, "CBasePlayer_TakeDamage", .post = true)
    // RegisterHam(Ham_StopSneaking, "info_target", "CBaseEntity_StopSneaking", .Post = true)

    RegisterHam(Ham_Touch, g_pointBaseClassname, "CTriggerAreaCapture__AreaTouch", .Post=true)
    RegisterHam(Ham_Think, g_pointBaseClassname, "CTriggerAreaCapture__CaptureThink", .Post=true)

    Create_ConVars()

#if 1
    server_cmd("mp_auto_join_team 1; humans_join_team t; redm_randomspawn 1; mp_freeforall 0; yb_chat 0; yb_quota 2")
#endif
}

static Create_ConVars() {
    bind_pcvar_float(
        create_cvar(

            "redm_capture_spawn_time", "1.0",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_FLOAT,
            .description = "Time to capture spawn point. (0 - instantly)"
        ),
        redm_capture_spawn_time
    )
    bind_pcvar_num(
        create_cvar(
            "redm_capture_spawn_bar", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Point capture status bar"
        ),
        redm_capture_spawn_bar
    )
    bind_pcvar_float(
        create_cvar(
            "redm_capture_spawn_penalty", "3",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_FLOAT,
            .description = "The time after taking damage that the player will not be able to use points (0 - no restrictions)"
        ),
        redm_capture_spawn_penalty
    )
    bind_pcvar_float(
        create_cvar(
            "redm_capture_spawn_distance", "64",
            .has_min = true, .min_val = 16.0,
            .has_max = true, .max_val = 256.0,
            .flags = _FCVAR_FLOAT,
            .description = "The interaction distance between the player and the spawn point"
        ),
        redm_capture_spawn_distance
    )
    bind_pcvar_num(
        create_cvar(
            "redm_capture_spawn_changelevel", "80",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 100.0,
            .flags = _FCVAR_INTEGER,
            .description = "How many percent do you need to capture spawns to finish the game?"
        ),
        redm_capture_spawn_changelevel
    )
    bind_pcvar_num(
        create_cvar(
            "redm_capture_spawn_static", "10",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 20.0,
            .flags = _FCVAR_INTEGER,
            .description = "The number of static spawns of commands that are not subject to capture"
        ),
        redm_capture_spawn_static
    )
}

ControlPoints_SpawnsLoaded(&JSON: arrSpawns) {
    collectSpawnPoints(arrSpawns)
}

ControlPoints_IsSpawnAvailable(spawnIdx, TeamName: targetTeam) {
    // WTF: player without team can't use this spawn!
    if (targetTeam != TEAM_TERRORIST && targetTeam != TEAM_CT)
        return 1

    new controlPoint = ___GetPointBySpawn(spawnIdx)
    if (!is_entity(controlPoint))
        return 1

    // Point hasn't team
    new TeamName: owningTeam = TeamName: pev(controlPoint, _pev_owningTeam)
    // if (owningTeam != TEAM_TERRORIST && owningTeam != TEAM_CT)
        // return 1

    // Point has same team with wish player target
    if (owningTeam == targetTeam)
        return 1

    return 0
}

___GetPointBySpawn(const spawnIdx) {
    new entity = MaxClients
    while ((entity = fm_find_ent_by_class(entity, g_pointClassname))) {
        if (pev(entity, _pev_hPoint) != spawnIdx)
            continue

        return entity
    }

    return -1
}


static collectSpawnPoints(&JSON: arrSpawns) {
    new spawnCount = json_array_get_count(arrSpawns) 
    for (new i; i < spawnCount; i++) {
        new spawn[SpawnProps_s]
        GetSpawnFromObject(i, spawn)

        spawn[sp_origin][2] += SPAWN_POINT_OFFSET
        
        // UNDONE: другой метод
        // if ((spawnTeam == 1 && numTerroristSpawns++ < redm_capture_spawn_static) || (spawnTeam == 2 && numCTSpawns++ < redm_capture_spawn_static)) {
        //     continue
        // }
        
        CTriggerAreaCapture__Spawn(
            spawn[sp_origin],
            spawn[sp_angle],
            g_ControlPointsModels[spawn[sp_team]],
            spawn[sp_team]
        )
    }
}

CTriggerAreaCapture__Spawn(const Float: origin[3], const Float: angles[3], const model[], const TeamName: team) {
    new entity = fm_create_entity(g_pointBaseClassname)
    set_pev(entity, pev_classname, g_pointClassname)

    fm_set_rendering(entity, kRenderFxNone, 0, 0, 0, kRenderTransAdd, 255)
    fm_animate_entity(entity, ACT_IDLE)

    set_pev(entity, pev_origin, origin)
    set_pev(entity, pev_model, model)
    
    // engfunc(EngFunc_SetModel, entity, model)
    // engfunc(EngFunc_SetOrigin, entity, origin)

    dllfunc(DLLFunc_Spawn, entity)

    CTriggerAreaCapture__CaptureThink(entity)
    // set_pev(entity, pev_nextthink, get_gametime() + AREA_THINK_TIME)

    // Prevent CBaseTrigger::InitTrigger() actions
    set_pev(entity, pev_effects, pev(entity, pev_effects) & ~EF_NODRAW)
    set_pev(entity, pev_angles, angles)


    {
        set_pev(entity, _pev_owningTeam, team)
        set_pev(entity, _pev_captureTime, redm_capture_spawn_time)

        new m_TeamData[MAX_CAPTURE_TEAMS][perTeamData_s]
        for ( new TeamName: t = TEAM_UNASSIGNED; t < MAX_CAPTURE_TEAMS; t++ ) {
            if ( m_TeamData[t][iNumRequiredToCap] < 1)
                m_TeamData[t][iNumRequiredToCap] = 1
        }
        set_pev(entity, _pev_teamData, m_TeamData)
    }


    engfunc(EngFunc_SetSize, entity, Float: {-16.0, -16.0, -36.0}, Float: {16.0, 16.0, 36.0})

    return entity
}

#if 0
    CTriggerAreaCapture__KeyValue() {

    }

    CTriggerAreaCapture__Precache() {

    }

    CTriggerAreaCapture__IsActive() {
        // return !m_bDisabled;
    }
#endif

// static Float: lastTouch[MAX_PLAYERS + 1] = { 0.0, ... }


public CTriggerAreaCapture__AreaTouch(const entity, const other) {
    static classname[32]; pev(entity, pev_classname, classname, charsmax(classname))
    if (strcmp(classname, g_pointClassname) != 0)
        return

    // server_print("-- CTriggerAreaCapture__AreaTouch(%i, %i)", entity, other)

    #if 0
        new Float: nextThink; pev(entity, pev_nextthink, nextThink)
        if (nextThink - get_gametime() < (AREA_THINK_TIME - 0.1))
            return
    #endif

    #if 0
        if (!IsActive())
            return
        
        if (!PointsMayBeCaptured())
            return
    #endif

    // dont touch for non-alive or non-players
    if (!ExecuteHamB(Ham_IsPlayer, other) || !is_user_alive(other))
        return

    new player = other

    new TeamName: owningTeam = TeamName: pev(entity, _pev_owningTeam)
    new TeamName: playerTeam = TeamName: get_ent_data(player, "CBasePlayer", "m_iTeam")
    if (playerTeam != owningTeam) {
        new m_TeamData[MAX_CAPTURE_TEAMS][perTeamData_s]
        pev(entity, _pev_teamData, m_TeamData, (_: MAX_CAPTURE_TEAMS * _: perTeamData_s))

        if (m_TeamData[playerTeam][bCanCap]) {
            #if 0
                DisplayCapHintTo(player)
            #endif
        }
    }
    
    #if 0
        new Float: diff = get_gametime() - lastTouch[player]
        server_print("diff:%f", diff)
        if (diff > AREA_THINK_TIME) {
            server_print("--->> StartTouch")
        }

        lastTouch[player] = get_gametime()
    #endif

    #if 1
    {
        set_hudmessage(
            255, 255, 255,
            0.6, 0.0,
            0, 0.0,
            AREA_THINK_TIME + 0.1,
            0.0,
            0.0,
            1
        )

        new Float: timeRemaining;       pev(entity, _pev_timeRemaining, timeRemaining)
        new Float: lastReductionTime;   pev(entity, _pev_lastReductionTime, lastReductionTime)
        new Float: captureTime;         pev(entity, _pev_captureTime, captureTime)

        new buffer[512]
        formatex(buffer, charsmax(buffer), "^n\
            (%.1f) touch `trigger_capture_area` ^n\
            %n -> %i ^n\
            ^n\
            %.1f | %-16.16s^n\
            %.1f | %-16.16s^n\
            %.1f | %-16.16s^n\
            %i   | %-16.16s^n\
            %i   | %-16.16s^n\
            %i   | %-16.16s^n\
            %i   | %-16.16s^n\
            ",
            get_gametime(), other, entity,
            timeRemaining, "timeRemaining",
            lastReductionTime, "lastReductionTime",
            captureTime, "captureTime",

            pev(entity, _pev_isCapturing), "isCapturing", 
            pev(entity, _pev_owningTeam), "owningTeam", 
            pev(entity, _pev_teamInZone), "teamInZone", 
            pev(entity, _pev_capturingTeam), "capturingTeam"
        )

        show_hudmessage(player, buffer)
        // server_print(buffer)
    }
    #endif
}

TeamplayGameRules__GetCaptureValueForPlayer(const player) {
    #pragma unused player
    return 1
}


public CTriggerAreaCapture__CaptureThink(const entity) {
    static classname[32]; pev(entity, pev_classname, classname, charsmax(classname))
    if (strcmp(classname, g_pointClassname) != 0)
        return

    // server_print("-- (%.2f) CTriggerAreaCapture__CaptureThink(%i)", get_gametime(), entity)
#if 1
    // go through our list of players
    new iNumPlayers[MAX_CAPTURE_TEAMS]
    new iNumBlockablePlayers[MAX_CAPTURE_TEAMS] // Players in the zone who can't cap, but can block / pause caps
    new pFirstPlayerTouching[MAX_CAPTURE_TEAMS]

    // Loop through the entities we're touching, and find players
    // new m_hTouchingEntities[MAX_PLAYERS + 1]
    for (new p = 1; p <= MaxClients; p++) {
        if (!is_user_alive(p))
            continue
        
        new Float: distance = fm_entity_range(entity, p)
        if (distance > redm_capture_spawn_distance)
            continue

        new TeamName: team = TeamName: get_member(p, m_iTeam)

        dllfunc(DLLFunc_Touch, entity, p)

        #if 0
            // If a team's not allowed to cap a point, don't count players in it at all
            if ( !TeamplayGameRules()->TeamMayCapturePoint( team, m_hPoint->GetPointIndex() ) )
                continue;

            if ( !TeamplayGameRules()->PlayerMayCapturePoint( pPlayer, m_hPoint->GetPointIndex() ) ) {
                if ( TeamplayGameRules()->PlayerMayBlockPoint( pPlayer, m_hPoint->GetPointIndex() ) )
                {
                    if ( iNumPlayers[iTeam] == 0 && iNumBlockablePlayers[iTeam] == 0 )
                    {
                        pFirstPlayerTouching[iTeam] = pPlayer
                    }

                    iNumBlockablePlayers[iTeam] += TeamplayGameRules()->GetCaptureValueForPlayer( pPlayer )
                }
                continue;
            }
        #endif

        if (team == TEAM_TERRORIST || team == TEAM_CT) {
            if (iNumPlayers[team] == 0 && iNumBlockablePlayers[team] == 0) {
                pFirstPlayerTouching[team] = p
            }

            iNumPlayers[team] += TeamplayGameRules__GetCaptureValueForPlayer( p )
        }

    }

    new iTeamsInZone = 0
    // new bool: bUpdatePlayers = false
    set_pev(entity, _pev_teamInZone, TEAM_UNASSIGNED)

    new m_TeamData[MAX_CAPTURE_TEAMS][perTeamData_s]
    pev(entity, _pev_teamData, m_TeamData, (_: MAX_CAPTURE_TEAMS * _: perTeamData_s))

    for (new TeamName: t = TEAM_TERRORIST; t <= TEAM_CT; t++) {
        iNumPlayers[t] *= mp_simulatemultiplecappers

        if (m_TeamData[t][iNumTouching] != iNumPlayers[t]) {
            m_TeamData[t][iNumTouching] = iNumPlayers[t]

            // bUpdatePlayers = true
        }

        m_TeamData[t][iBlockedTouching] = m_TeamData[t][iNumTouching]
        if (m_TeamData[t][iNumTouching]) {
            iTeamsInZone++

            set_pev(entity, _pev_teamInZone, t)
        }

        set_pev(entity, _pev_teamData, m_TeamData)
    }

    if (iTeamsInZone > 1) {
        set_pev(entity, _pev_teamInZone, TEAM_UNASSIGNED)
    } else {
        // If we've got non-cappable, yet blockable players here for the team that's defending, they 
        // need to block the cap. This catches cases like the TF invulnerability, which needs to block
        // caps, but isn't allowed to contribute to a cap.
        for (new TeamName: t = TEAM_TERRORIST; t <= TEAM_CT; t++) {
            if (!iNumBlockablePlayers[t] || TeamName: pev(entity, _pev_teamInZone) == t)
                continue

            set_pev(entity, _pev_teamInZone, pev(entity, _pev_teamInZone) + 1)
        }
    }

    #if 0
        UpdateTeamInZone()
    #endif

    // server_print("iNumPlayers:[%i|%i], iTeamsInZone:%i, m_TeamData[][iNumTouching]:[%i, %i] _pev_teamInZone:%i", 
    //     iNumPlayers[TEAM_TERRORIST], iNumPlayers[TEAM_CT],
    //     iTeamsInZone, 
    //     m_TeamData[TEAM_TERRORIST][iNumTouching], m_TeamData[TEAM_CT][iNumTouching],
    //     pev(entity, _pev_teamInZone)
    // )

#endif




    new Float: gametime = get_gametime()
    set_pev(entity, pev_nextthink, gametime + AREA_THINK_TIME)

    // When a player blocks, tell them the cap index and attempt number
    // only give successive blocks to them if the attempt number is different
    if (pev(entity, _pev_isCapturing)) {
        new TeamName: capturingTeam = TeamName: pev(entity, _pev_capturingTeam)
        new TeamName: teamInZone = TeamName: pev(entity, _pev_teamInZone)
        new TeamName: owningTeam = TeamName: pev(entity, _pev_owningTeam)

        new Float: lastReductionTime; pev(entity, _pev_lastReductionTime, lastReductionTime)
        new Float: timeRemaining; pev(entity, _pev_timeRemaining, timeRemaining)
        new Float: captureTime; pev(entity, _pev_captureTime, captureTime)

        // Calculate the amount of modification to the cap time
        new Float: flTimeDelta = (gametime - lastReductionTime)

        #if 0
            new Float: old; pev(entity, pev_fov, old)
            server_print("delta:%f, old:%f, new:%f", gametime-old, old, gametime)
            set_pev(entity, pev_fov, (old = gametime))
        #endif


        set_pev(entity, _pev_lastReductionTime, (lastReductionTime = gametime))
        
        new Float: flReduction = flTimeDelta
        new Float: flTotalTimeToCap = captureTime

        // Now remove the reduction amount after we've determined there's only 1 team in the area
        if (capturingTeam == teamInZone) {
            CTriggerAreaCapture__SetCapTimeRemaining(entity, timeRemaining - flReduction)
        } else if (owningTeam == TEAM_UNASSIGNED && teamInZone != TEAM_UNASSIGNED ) {
            CTriggerAreaCapture__SetCapTimeRemaining(entity, timeRemaining + flReduction)
        } else {
            // Caps deteriorate over time
            #if 1
                if ( /* TeamplayRoundBasedRules() && m_hPoint && TeamplayRoundBasedRules()->TeamMayCapturePoint(m_nCapturingTeam,m_hPoint->GetPointIndex()) */ true ) {
                    static Float: mp_capdeteriorate_time = 15.0

                    new Float: flDecrease = (flTotalTimeToCap / mp_capdeteriorate_time) * flTimeDelta
                    if ( /* TeamplayRoundBasedRules() && TeamplayRoundBasedRules()->InOvertime() */ false ) {
                        flDecrease *= 6
                    }
                    // server_print("22222222 (%.2f + %.2f):%.2f", 
                    //     timeRemaining, flDecrease,
                    //     timeRemaining + flDecrease
                    // )
    
                    CTriggerAreaCapture__SetCapTimeRemaining(entity, timeRemaining + flDecrease )
                }
                else
            #endif
            {
                // server_print("111111111")
                CTriggerAreaCapture__SetCapTimeRemaining(entity, flTotalTimeToCap)
            }
        }

        pev(entity, _pev_timeRemaining, timeRemaining)

        #if 1
            { // TODO: team_control_point WORK
                new Float: renderColor[3]

                // new Float: percentage = ((captureTime - timeRemaining) / captureTime) * 100
                new Float: remainingPercentage = timeRemaining / captureTime

                // server_print("progress: %f, remainingPercentage:%f, seqRate: %f",
                //     percentage, remainingPercentage, (g_sequenceRate * remainingPercentage)
                // )

                xs_vec_mul_scalar(g_color[owningTeam], remainingPercentage, renderColor)

                set_pev(entity, pev_rendercolor, renderColor)
                set_pev(entity, pev_renderamt, g_renderAmount * remainingPercentage)

                // set_pev(entity, pev_framerate, (g_sequenceRate * remainingPercentage))
            }
        #endif

        // if the cap is done
        if (timeRemaining <= 0.0) {
            CTriggerAreaCapture__EndCapture(entity, capturingTeam)
            return  //we're done
        } else {
            // server_print("    :: Think:  (diff:%f) timeRemaining:%f, flTotalTimeToCap:%f",
            //     (timeRemaining - flTotalTimeToCap), timeRemaining, flTotalTimeToCap
            // )

            if (timeRemaining >= flTotalTimeToCap) {
				CTriggerAreaCapture__BreakCapture(entity, false )
				return
			}
        }
    } else {
        // If there are any teams in the zone that aren't the owner, try to start capping
        new TeamName: teamInZone = TeamName: pev(entity, _pev_teamInZone)
        if ((teamInZone != TEAM_TERRORIST && teamInZone != TEAM_CT))
            return

        new TeamName: owningTeam = TeamName: pev(entity, _pev_owningTeam)
        
        for (new TeamName: t = TEAM_TERRORIST; t <= TEAM_CT; t++) {
            if (owningTeam == t)
                continue

            // if (!m_TeamData[t][bCanCap])
            //     continue

            // server_print("^n^nowningTeam:%i == t:%i, m_TeamData[][iNumTouching]:%i",
            //     owningTeam, t,
            //     m_TeamData[t][iNumTouching]
            // )
            if (m_TeamData[t][iNumTouching] == 0)
                continue

            if (mp_capstyle == 0 && m_TeamData[t][iNumTouching] < m_TeamData[t][iNumRequiredToCap])
                continue

            CTriggerAreaCapture__StartCapture(entity, t)
            break
        }
    }
}

CTriggerAreaCapture__StartCapture(const entity, const TeamName: team /* , const capmode */ ) {
    // server_print("^n^n--> CTriggerAreaCapture__StartCapture(e:%i, t:%i)", entity, team)


    #if 1
    { // TODO: team_control_point WORK
        fm_animate_entity(entity, ACT_WALK, g_sequenceRate)
        // set_pev(entity, pev_avelocity, Float: {0.0, 0.0, 0.0})

        set_pev(entity, pev_rendermode, kRenderTransAdd)
        set_pev(entity, pev_renderamt, 80.0)

        set_pev(entity, pev_renderfx, kRenderFxGlowShell)
        set_pev(entity, pev_rendercolor, g_color[TeamName: pev(entity, _pev_owningTeam)])

    }
    #endif

    set_pev(entity, _pev_capturingTeam, team)
    new Float: captureTime; pev(entity, _pev_captureTime, captureTime)

    if (mp_capstyle == 1) {
        // CTriggerAreaCapture__SetCapTimeRemaining( ((m_flCapTime * 2) * m_TeamData[team].iNumRequiredToCap) );
    } else {
        CTriggerAreaCapture__SetCapTimeRemaining(entity, captureTime)
    }

    set_pev(entity, _pev_isCapturing, true)
    // m_bBlocked = false

    set_pev(entity, _pev_lastReductionTime, get_gametime())
}


CTriggerAreaCapture__SetCapTimeRemaining(const entity, Float: time)
{
    // server_print("     :: SetCapTimeRemaining(%i, %f)", entity, time)

    set_pev(entity, _pev_timeRemaining, time)
   

#if 0
    new Float: flCapPercentage
    
    new TeamName: capturingTeam = TeamName: pev(entity, _pev_capturingTeam)
    new Float: captureTime; pev(entity, _pev_captureTime, captureTime)

    new m_TeamData[MAX_CAPTURE_TEAMS][perTeamData_s]
    pev(entity, _pev_teamData, m_TeamData, (_: MAX_CAPTURE_TEAMS * _: perTeamData_s))


    if (capturingTeam) {
        flCapPercentage = time / captureTime
        if (mp_capstyle == 1) {
            flCapPercentage = time / ((captureTime * 2) * m_TeamData[capturingTeam][iNumRequiredToCap])
        }
    }
#endif
}


CTriggerAreaCapture__EndCapture(const entity, const TeamName: team) {
    set_pev(entity, _pev_owningTeam, team)

    set_pev(entity, _pev_isCapturing, false)
    set_pev(entity, _pev_capturingTeam, TEAM_UNASSIGNED)

    CTriggerAreaCapture__SetCapTimeRemaining(entity, 0.0)

    #if 1
    { // TODO: team_control_point WORK
        fm_animate_entity(entity, ACT_IDLE, 0.0)
        fm_set_rendering(entity, kRenderFxNone, 0, 0, 0, kRenderTransAdd, 0)

        
        set_pev(entity, pev_rendermode, kRenderTransAdd)
        set_pev(entity, pev_renderamt, 80.0)

        new TeamName: owningTeam = TeamName: pev(entity, _pev_owningTeam)
        engfunc(EngFunc_SetModel, entity, g_ControlPointsModels[owningTeam])
    }
    #endif

    // server_print("<-- CTriggerAreaCapture__EndCapture(e:%i, t:%i)", entity, team)
}

CTriggerAreaCapture__BreakCapture(const entity, const bool: notEnoughPlayers) {
    #pragma unused notEnoughPlayers
    #if 1
    { // TODO: team_control_point WORK
        fm_animate_entity(entity, ACT_IDLE, 0.0)
        fm_set_rendering(entity, kRenderFxNone, 0, 0, 0, kRenderTransAdd, 0)

        
        set_pev(entity, pev_rendermode, kRenderTransAdd)
        set_pev(entity, pev_renderamt, 80.0)
    }
    #endif

    if (pev(entity, _pev_isCapturing)) {
        set_pev(entity, _pev_isCapturing, false) 
        set_pev(entity, _pev_capturingTeam, TEAM_UNASSIGNED)

        CTriggerAreaCapture__SetCapTimeRemaining(entity, 0.0)
    }

    // server_print("<-- CTriggerAreaCapture__BreakCapture(e:%i)", entity)
}
