#include <amxmodx>
#include <amxmisc>
#include <fakemeta>
#include <fakemeta_util>
#include <json>

#include <reapi>
#include <redm>


static g_mapName[MAX_MAPNAME_LENGTH]

static const g_spawnClassname[] = "view_spawn" 

static bool: g_editorEnabled = false

enum HullVacant_s {
    hull_Stand,
    hull_Duck,
    hull_Invalid,
}

enum EditorProps_s {
    ep_team,
    ep_focusEntity,
    ep_gravityPreset,
    ep_group[32]
}

static g_editorProps[MAX_PLAYERS + 1][EditorProps_s]

new JSON: g_arrSpawns = Invalid_JSON

static const g_spawnViewModels[_: TeamName - 1][] = {
    "models/player/vip/vip.mdl",
    "models/player/leet/leet.mdl",
    "models/player/gign/gign.mdl",
}

static const g_teamName[_: TeamName - 1][] = {
    "ANY",
    "T",
    "CT",
}

static Float: g_gravityValues[] = {
    1.0, 0.5, 0.25, 0.05
} 

// Determines whether players are to spawn. 0 = default; 1 = both teams; 2 = Terrorists; 3 = CTs.
new mp_randomspawn = 1

// If non-zero, a randomly spawning player will, if possible, not be spawned at a spawn point with direct line of sight to another player. (2 = check viewcone)
new mp_randomspawn_los = 2

// If greater than 0, a randomly spawning player will, if possible, not be spawned at a spawn point where the smallest distance to another player is smaller than the value of this ConVar.
new Float: mp_randomspawn_dist = 1500.0

new bool: mp_freeforall


public plugin_precache() {
    for (new i = 0; i < sizeof(g_spawnViewModels); i++) {
        precache_model(g_spawnViewModels[i])
    }
}

public plugin_init() {
    register_plugin("Spawns manager", VERSION, "Sergey Shorokhov")
    register_dictionary("common.txt")

    get_mapname(g_mapName, charsmax(g_mapName))
    GameDLLSpawnsCountFix()

    register_clcmd("enter_spawnGroup", "ClCmd_EnterSpawnGroup")

    redm_addstyle("preset", "SpawnPreset_DefaultPreset")
}

public plugin_cfg() {
    bind_pcvar_num(get_cvar_pointer("mp_freeforall"), mp_freeforall)
    // set_pcvar_bounds(get_cvar_pointer("mp_forcerespawn"), CvarBound_Lower, true, 0.1) // TODO

    bind_pcvar_num(create_cvar("mp_randomspawn", "1"), mp_randomspawn)
    bind_pcvar_num(create_cvar("mp_randomspawn_los", "2"), mp_randomspawn_los)
    bind_pcvar_float(create_cvar("mp_randomspawn_dist", "1500.0"), mp_randomspawn_dist)
    
    RegisterHookChain(RG_CBasePlayer_UseEmpty, "CBasePlayer_UseEmpty", .post = false)

    register_concmd("redm_edit_spawns", "ConCmd_EditSpawns",
        ADMIN_MAP,
        "Edits spawn configuration"
    )
    
    register_concmd("redm_convert_spawns", "ConCmd_ConvertOldSpawns",
        ADMIN_MAP,
        "Convert old spawns to new format"
    )

    Editor_ReloadSpawns()
}

public client_putinserver(player) {
    Editor_ResetProps(player)
}

public ConCmd_EditSpawns(const player, const level, const cid) {
    SetGlobalTransTarget(player)

    if (!cmd_access(player, level, cid, 1))
        return PLUGIN_HANDLED

    g_editorEnabled = !g_editorEnabled

    if (!g_editorEnabled) {
        Editor_Disable(player)
    } else {
        Editor_Enable(player)
    }

    console_print(player, " * Spawns editor `%s`",
        g_editorEnabled ? "enabled" : "disabled"
    )

    return PLUGIN_HANDLED
}

public ConCmd_ConvertOldSpawns(const player, const level, const cid) {
    SetGlobalTransTarget(player)

    if (!cmd_access(player, level, cid, 1))
        return PLUGIN_HANDLED

    Editor_ConvertSpawns()

    return PLUGIN_HANDLED
}


static Editor_Enable(const player) {
    if (g_arrSpawns != Invalid_JSON)
        Editor_AddViewSpawns(g_arrSpawns)

    Menu_Editor(player)
}

static Editor_Disable(const player) {
    reset_menu(player)

    Editor_ResetProps(player)

    Editor_SaveSpawns()
    Editor_RemoveViewSpawns()

    Editor_ReloadSpawns()
}

static Editor_ReloadSpawns() {
    if (g_arrSpawns != Invalid_JSON)
        json_free(g_arrSpawns)

    g_arrSpawns = Editor_LoadSpawns()
}

static Editor_ResetProps(const player) {
    g_editorProps[player][ep_focusEntity] = FM_NULLENT
    g_editorProps[player][ep_team] = 0
    g_editorProps[player][ep_gravityPreset] = 0
    g_editorProps[player][ep_group][0] = EOS
}

public CBasePlayer_UseEmpty(const player) {
    if (!g_editorEnabled)
        return HC_CONTINUE

    Editor_Focus(player)
    Menu_Editor(player)

    return HC_SUPERCEDE
}

static Editor_Focus(const player) {
    new entity = FindEntityByAim(player)
    if (entity != FM_NULLENT) {
        if (g_editorProps[player][ep_focusEntity] != FM_NULLENT)
            Editor_ClearEntityFocus(player)

        Editor_SetEntityFocus(player, entity)

        return
    }

    entity = g_editorProps[player][ep_focusEntity]
    if (entity != FM_NULLENT) {
        Editor_ClearEntityFocus(player)
    }
}

FindEntityByAim(const player) {
    new entity = FM_NULLENT

    SetEntitysSolid(true)
    get_user_aiming(player, entity, .dist = 1000)
    SetEntitysSolid(false)

    if (fm_is_ent_classname(entity, g_spawnClassname))
        return entity

    return FM_NULLENT
}

SetEntitysSolid(const bool: solid) {
    new entity = NULLENT
    while ((entity = fm_find_ent_by_class(entity, g_spawnClassname))) {
        if (!solid) {
            engfunc(EngFunc_SetSize, entity, Float: {0.0, 0.0, 0.0}, Float: {0.0, 0.0, 0.0})
        } else {
            engfunc(EngFunc_SetSize, entity, Float: {-16.0, -16.0, -36.0}, Float: {16.0, 16.0, 36.0})
        }
    }
}

static bool: Editor_SetEntityFocus(const player, const entity = FM_NULLENT) {
    if (entity == FM_NULLENT)
        return false
    
    static const teamColors[_: TeamName - 1][] = {
        { 255, 255, 255 },
        { 255, 0, 0 },
        { 0, 0, 255 },
    }

    new bool: inDuck = bool: (pev(entity, pev_flags) & FL_DUCKING)
    fm_animate_entity(entity, inDuck ? ACT_FLY : ACT_RUN, 0.3)
    new team = pev(entity, pev_team)
    fm_set_rendering(
        entity,
        kRenderFxGlowShell,
        teamColors[team][0],
        teamColors[team][1],
        teamColors[team][2],
        .amount = 20
    )

    g_editorProps[player][ep_focusEntity] = entity
    g_editorProps[player][ep_team] = pev(entity, pev_team)

    pev(entity, pev_netname,
        g_editorProps[player][ep_group],
        charsmax(g_editorProps[][ep_group])
    )

    return true
}

static Editor_ClearEntityFocus(const player) {
    new entity = g_editorProps[player][ep_focusEntity]
    new bool: inDuck = bool: (pev(entity, pev_flags) & FL_DUCKING)
    
    fm_animate_entity(entity, inDuck ? ACT_CROUCHIDLE : ACT_IDLE)
    fm_set_rendering(entity)

    g_editorProps[player][ep_focusEntity] = FM_NULLENT
    g_editorProps[player][ep_group][0] = EOS
}

static Menu_Editor(const player/* , const level */) {
    SetGlobalTransTarget(player)

    if (!is_user_alive(player))
        return

    static callback
    if (!callback)
        callback = menu_makecallback("MenuCallback_Editor")

    new menu = menu_create("Spawns manager", "MenuHandler_Editor")

    menu_additem(
        menu,
        (g_editorProps[player][ep_focusEntity] != FM_NULLENT) ? "Update" : "Add",
        .callback = callback
    )

    new team = g_editorProps[player][ep_team]
    menu_additem(menu, fmt("Team: %s", g_teamName[team]))
    menu_additem(menu, "Teleport", .callback = callback)
    menu_additem(menu, "Delete", .callback = callback)
    new gravityPreset = g_editorProps[player][ep_gravityPreset]
    menu_additem(menu, fmt("Gravity: %.2f", g_gravityValues[gravityPreset]), .callback = callback)
    menu_additem(menu, fmt("Group: %s", g_editorProps[player][ep_group]), .callback = callback)

    new stats[_: TeamName - 1]
    new total = Editor_GetStats(stats)

    menu_addtext(menu,
        fmt("^nTotal:%i (ANY:%i, T:%i, CT:%i)",
            total, stats[0], stats[1], stats[2]
        ),
        .slot = false
    )

    menu_setprop(menu, MPROP_EXIT, MEXIT_NEVER)

    menu_display(player, menu)
}

public MenuCallback_Editor(const player, const menu, const item) {
    switch(item) {
        case 2, 3: {
            if (g_editorProps[player][ep_focusEntity] == FM_NULLENT)
                return ITEM_DISABLED
        }
    }

    return ITEM_IGNORE
}

public MenuHandler_Editor(const player, const menu, const item) {
    menu_destroy(menu)

    if (item < 0) {
        return PLUGIN_HANDLED
    }

    switch (item) {
        case 0: {
            new entity = g_editorProps[player][ep_focusEntity]
            if (entity != FM_NULLENT) {
                Spawn_Update(player, entity)
            } else {
                Spawn_Add(player)
            }
        }
        case 1: {
            ++g_editorProps[player][ep_team]
            g_editorProps[player][ep_team] %= sizeof(g_spawnViewModels)
            
            new entity = g_editorProps[player][ep_focusEntity]
            if (entity != FM_NULLENT)
                Spawn_SetTeam(player, entity, g_editorProps[player][ep_team])
        }
        case 2: {
            new entity = g_editorProps[player][ep_focusEntity]

            new Float: origin[3], Float: angle[3], Float: vAngle[3]
            Spawn_EntityGetPosition(entity, origin, angle, vAngle)
            Spawn_EntitySetPosition(player, origin, angle, vAngle)
        }
        case 3: {
            new entity = g_editorProps[player][ep_focusEntity]

            Spawn_Delete(entity)
            g_editorProps[player][ep_focusEntity] = FM_NULLENT
        }
        case 4: {
            ++g_editorProps[player][ep_gravityPreset]
            g_editorProps[player][ep_gravityPreset] %= sizeof(g_gravityValues)

            set_pev(
                player,
                pev_gravity,
                g_gravityValues[g_editorProps[player][ep_gravityPreset]]
            )
        }
        case 5: {
            client_cmd(player, "messagemode enter_spawnGroup")
        }
    }

    Menu_Editor(player)

    return PLUGIN_HANDLED
}

public ClCmd_EnterSpawnGroup(const player, const level, const cid) {
    if (!cmd_access(player, level, cid, 1))
        return PLUGIN_HANDLED

    static spawnGroup[32]
    read_argv(1, spawnGroup, charsmax(spawnGroup))

    if (!spawnGroup[0]) {
      Menu_Editor(player)

      return PLUGIN_HANDLED
    }

    copy(g_editorProps[player][ep_group], charsmax(g_editorProps[][ep_group]), spawnGroup)

    new entity = g_editorProps[player][ep_focusEntity]
    if (entity != FM_NULLENT)
        set_pev(entity, pev_netname, spawnGroup)

    Menu_Editor(player)
    return PLUGIN_HANDLED
}

static Editor_GetStats(stats[_: TeamName - 1]) {
    new total
    new entity = NULLENT
    while ((entity = fm_find_ent_by_class(entity, g_spawnClassname))) {
        new team = pev(entity, pev_team)
        switch(team) {
            case 1, 2: ++stats[team]
            default: ++stats[0]
        }

        ++total
    }

    return total
}

static bool: Spawn_Add(const player) {
    new Float: origin[3], Float: angle[3], Float: vAngle[3]
    Spawn_EntityGetPosition(player, origin, angle, vAngle)
    origin[2] += 0.1

    new team = g_editorProps[player][ep_team]

    return Add(origin, angle, vAngle, team, g_editorProps[player][ep_group])
}

static bool: Add(const Float: origin[3], const Float: angle[3], const Float: vAngle[3], const team, const group[32]) {
    new entity = Spawn_CreateEntity()
    if (!Spawn_EntitySetPosition(entity, origin, angle, vAngle)) {
        Spawn_Delete(entity)
        return false
    }

    set_pev(entity, pev_team, team)
    set_pev(entity, pev_netname, group)
    engfunc(EngFunc_SetModel, entity, g_spawnViewModels[team])

    return true
}

static Spawn_Update(const player, const entity) {
    new Float: origin[3], Float: angle[3], Float: vAngle[3]
    Spawn_EntityGetPosition(player, origin, angle, vAngle)

    Spawn_EntitySetPosition(entity, origin, angle, vAngle)
    Editor_SetEntityFocus(player, entity)
}

static Spawn_Delete(const entity) {
    engfunc(EngFunc_RemoveEntity, entity)
}

static Spawn_SetTeam(const player, const entity, const team) {
    set_pev(entity, pev_team, team)
    engfunc(EngFunc_SetModel, entity, g_spawnViewModels[team])

    Editor_SetEntityFocus(player, entity)
}

static Spawn_EntityGetPosition(const entity, Float: origin[3], Float: angle[3], Float: vAngle[3]) {
    pev(entity, pev_origin, origin)
    pev(entity, pev_angles, angle)
    pev(entity, pev_v_angle, vAngle)
}

static bool: Spawn_EntitySetPosition(const entity, const Float: origin[3], const Float: angle[3], const Float: vAngle[3]) {
    new HullVacant_s: res = CheckHullVacant(origin, .ignorePlayers = g_editorEnabled)
    if (res == hull_Invalid) {
        if (g_editorEnabled)
            client_print_color(is_user_connected(entity) ? entity : 0, print_team_red, "^3Invalid place!^1")

        return false
    }

    engfunc(EngFunc_SetOrigin, entity, origin)
    set_pev(entity, pev_v_angle, vAngle)
    set_pev(entity, pev_angles, is_user_alive(entity) ? vAngle : angle)
    set_pev(entity, pev_fixangle, 1)
    set_pev(entity, pev_velocity, Float: {0.0, 0.0, 0.0})
    set_pev(entity, pev_punchangle, Float: {0.0, 0.0, 0.0})
    set_pev(entity, pev_avelocity, Float: {0.0, 0.0, 0.0})

    if (res == hull_Duck) {
        set_pev(entity, pev_flags, pev(entity, pev_flags) | FL_DUCKING)
    } else {
        set_pev(entity, pev_flags, pev(entity, pev_flags) & ~FL_DUCKING)
    }

    fm_animate_entity(entity, (res == hull_Duck) ? ACT_CROUCHIDLE : ACT_IDLE)

    return true
}

static Spawn_CreateEntity() {
    new entity = fm_create_entity("info_target")

    set_pev(entity, pev_classname, g_spawnClassname)
    set_pev(entity, pev_solid, SOLID_BBOX)

    return entity
}

static stock fm_animate_entity(const entity, const Activity: sequence = ACT_IDLE, const Float: framerate = 0.0) {
    set_pev(entity, pev_sequence, sequence)
    set_pev(entity, pev_framerate, framerate)
}

stock HullVacant_s: CheckHullVacant(const Float: origin[3], const bool: ignorePlayers = false) {
    static hulls[] = { HULL_HUMAN, HULL_HEAD }

    new res
    for (new i; i < sizeof(hulls); i++) {
        engfunc(EngFunc_TraceHull, origin, origin, ignorePlayers ? IGNORE_MONSTERS : DONT_IGNORE_MONSTERS, hulls[i], 0, 0)

        if (!get_tr2(0, TR_StartSolid) /*&& !get_tr2(0, TR_AllSolid) && get_tr2(0, TR_InOpen) */) {
            return HullVacant_s: res
        }
        ++res
    }

    return HullVacant_s: res
}

static Editor_RemoveViewSpawns() {
    new entity = FM_NULLENT
    while ((entity = fm_find_ent_by_class(entity, g_spawnClassname))) {
        engfunc(EngFunc_RemoveEntity, entity)
    }
}

static Editor_AddViewSpawns(const JSON: arrSpawns) {
    if (arrSpawns == Invalid_JSON) {
        LogMessageEx(Debug, "Editor_AddViewSpawns: `arrSpawns` is inavlid!")
        return
    }

    for (new idx, size = json_array_get_count(arrSpawns); idx < size; idx++) {
        new JSON: spawn = json_array_get_value(arrSpawns, idx)
         
        new Float: origin[3], Float: angle[3], Float: vAngle[3]
        new team = json_object_get_number(spawn, "team")
        new JSON: arrOrigin = json_object_get_value(spawn, "origin")
        new JSON: arrAngle = json_object_get_value(spawn, "angle")
        new JSON: arrVAngle = json_object_get_value(spawn, "vAngle")

        new group[32]
        json_object_get_string(spawn, "group", group, charsmax(group))

        for (new i; i < sizeof(origin); i++) {
            origin[i] = json_array_get_real(arrOrigin, i)
            if (i < 2) {
                angle[i] = json_array_get_real(arrAngle, i)
                vAngle[i] = json_array_get_real(arrVAngle, i)
            }
        }

        if (!Add(origin, angle, vAngle, team, group)) {
            LogMessageEx(Warning, "Editor_AddViewSpawns: Can't add spawn `%i`!")
        }

        json_free(arrOrigin)
        json_free(arrAngle)
        json_free(arrVAngle)

        json_free(spawn)
    }
}

static Editor_SaveSpawns() {
    new JSON: arrSpawns = json_init_array()

    new entity = FM_NULLENT
    while ((entity = fm_find_ent_by_class(entity, g_spawnClassname))) {
        new JSON: spawn = json_init_object()

        new Float: origin[3], Float: angle[3], Float: vAngle[3]
        pev(entity, pev_origin, origin)
        pev(entity, pev_angles, angle)
        pev(entity, pev_v_angle, vAngle)
        new team = pev(entity, pev_team)
        new group[32]
        pev(entity, pev_netname, group, charsmax(group))

        json_object_set_number(spawn, "team", team)
        json_object_set_string(spawn, "group", group)

        new JSON: arrOrigin = json_init_array()
        new JSON: arrAngle = json_init_array()
        new JSON: arrVAngle = json_init_array()
 
        for (new i; i < 3; i++) {
            json_array_append_real(arrOrigin, origin[i])
            if (i < 2) {
                json_array_append_real(arrAngle, angle[i])
                json_array_append_real(arrVAngle, vAngle[i])
            }
        }

        json_object_set_value(spawn, "origin", arrOrigin)
        json_object_set_value(spawn, "angle", arrAngle)
        json_object_set_value(spawn, "vAngle", arrVAngle)

        json_free(arrOrigin)
        json_free(arrAngle)
        json_free(arrVAngle)
        
        json_array_append_value(arrSpawns, spawn)

        json_free(spawn)
    }

    new JSON: objSpawns = json_init_object()
    json_object_set_value(objSpawns, "spawns", arrSpawns)
    json_free(arrSpawns)

    new filePath[PLATFORM_MAX_PATH]
    get_datadir(filePath, charsmax(filePath))

    formatex(filePath, charsmax(filePath), "%s/redm", filePath)
    if (!dir_exists(filePath) && mkdir(filePath) == -1)
        set_fail_state("Can't create folder `%s`", filePath)

    formatex(filePath, charsmax(filePath), "%s/%s.spawns.json",
        filePath, g_mapName
    )

    if (!json_serial_to_file(objSpawns, filePath, true))
        set_fail_state("Can't create file `%s`", filePath)

    json_free(objSpawns)
}

static JSON: Editor_LoadSpawns() {
    new filePath[PLATFORM_MAX_PATH]
    get_datadir(filePath, charsmax(filePath))

    formatex(filePath, charsmax(filePath), "%s/redm/%s.spawns.json",
        filePath, g_mapName
    )

    if (!file_exists(filePath)) {
        LogMessageEx(Debug, "Editor_LoadSpawns: No spawns file found `%s`.", filePath)
        return Invalid_JSON
    }

    new JSON: objSpawns = json_parse(filePath, true, true)
    if (objSpawns == Invalid_JSON) {
        LogMessageEx(Debug, "Editor_LoadSpawns: Can't parse JSON file from `%s`.", filePath)
        return Invalid_JSON
    }

    new JSON: arrSpawns = json_object_get_value(objSpawns, "spawns")

    LogMessageEx(Debug, "Editor_LoadSpawns: Map `%s` total spawns: %i loaded.",
        g_mapName, json_array_get_count(arrSpawns)
    )

    return arrSpawns
}

static Editor_ConvertSpawns() {
    new configsDir[PLATFORM_MAX_PATH]
    get_configsdir(configsDir, charsmax(configsDir))

    new csdmSpawnsDir[PLATFORM_MAX_PATH]
    formatex(csdmSpawnsDir, charsmax(csdmSpawnsDir),
        "%s/csdm/spawns",
        configsDir
    )

    if (!dir_exists(csdmSpawnsDir)) {
        LogMessageEx(Warning, "Editor_ConvertSpawns: Directorory not exists `%s`.", csdmSpawnsDir)
        
        return false
    }

    new fileName[MAX_MAPNAME_LENGTH]
    new dir = open_dir(csdmSpawnsDir, fileName, charsmax(fileName))
    if (!dir) {
        LogMessageEx(Warning, "Editor_ConvertSpawns: Can't open directorory `%s`.", csdmSpawnsDir)

        return false
    }

    new count
    while (next_file(dir, fileName, charsmax(fileName))) {
        new bool: isSpawnsFile = contain(fileName, ".spawns.cfg") != -1
        if (!isSpawnsFile)
            continue

        ConvertOldSpawnsFile(fmt("%s/%s", csdmSpawnsDir, fileName))
        ++count
    }

    LogMessageEx(Info, "Editor_ConvertSpawns: Succefully convert `%i` old spawn files.", count)

    close_dir(dir)

    return true
}

static bool: ConvertOldSpawnsFile(const file[]) {
    new fileName[PLATFORM_MAX_PATH]
    remove_filepath(file, fileName, charsmax(fileName))

    new map[PLATFORM_MAX_PATH]
    split_string(fileName, ".spawns.cfg", map, charsmax(map))

    new f = fopen(file, "r")
    if (!f) {
        LogMessageEx(Info, "ConvertOldSpawnsFile: Can't open file `%s`.", file)

        return false
    }

    new JSON: arrSpawns = json_init_array()
    while (!feof(f)) {
        new data[1024]
        fgets(f, data, charsmax(data))
        trim(data)

        new strOrigin[3][8], strAngles[3][8], strTeam[3], strVAngles[3][8]
        new argC = parse(
            data,
            strOrigin[0], charsmax(strOrigin[]),
            strOrigin[1], charsmax(strOrigin[]),
            strOrigin[2], charsmax(strOrigin[]),
            strAngles[0], charsmax(strAngles[]),
            strAngles[1], charsmax(strAngles[]),
            strAngles[2], charsmax(strAngles[]),
            strTeam, charsmax(strTeam),
            strVAngles[0], charsmax(strVAngles[]),
            strVAngles[1], charsmax(strVAngles[]),
            strVAngles[2], charsmax(strVAngles[])
        )

        if (argC != 10)
            continue
        
        {
            new JSON: spawn = json_init_object()

            json_object_set_number(spawn, "team", strtol(strTeam))

            new JSON: arrOrigin = json_init_array()
            new JSON: arrAngle = json_init_array()
            new JSON: arrVAngle = json_init_array()
    
            for (new i; i < 3; i++) {
                json_array_append_real(arrOrigin, strtof(strOrigin[i]))
                if (i < 2) {
                    json_array_append_real(arrAngle, strtof(strAngles[i]))
                    json_array_append_real(arrVAngle, strtof(strVAngles[i]))
                }
            }

            json_object_set_value(spawn, "origin", arrOrigin)
            json_object_set_value(spawn, "angle", arrAngle)
            json_object_set_value(spawn, "vAngle", arrVAngle)

            json_free(arrOrigin)
            json_free(arrAngle)
            json_free(arrVAngle)

            json_array_append_value(arrSpawns, spawn)

            json_free(spawn)
        }
    }

    fclose(f)

    new JSON: objSpawns = json_init_object()
    json_object_set_value(objSpawns, "spawns", arrSpawns)
    json_free(arrSpawns)

    new filePath[PLATFORM_MAX_PATH]
    get_datadir(filePath, charsmax(filePath))

    formatex(filePath, charsmax(filePath), "%s/redm/converted", filePath)
    if (!dir_exists(filePath) && mkdir(filePath) == -1)
        set_fail_state("Can't create folder `%s`", filePath)

    formatex(filePath, charsmax(filePath), "%s/%s.spawns.json",
        filePath, map
    )

    if (!json_serial_to_file(objSpawns, filePath, true))
        set_fail_state("Can't create file `%s`", filePath)

    json_free(objSpawns)
    return true
}

static GameDLLSpawnsCountFix() {
    set_member_game(m_bLevelInitialized, true)
    set_member_game(m_iSpawnPointCount_CT, 32)
    set_member_game(m_iSpawnPointCount_Terrorist, 32)
}

public bool: SpawnPreset_DefaultPreset(const player) {
    if (g_arrSpawns == Invalid_JSON)
        return false
    
    new count = json_array_get_count(g_arrSpawns)
    if (!count)
        return false

    new team = get_member(player, m_iTeam)
    switch(mp_randomspawn) {
        case 0:
            return false
        case 2: {
            if (team != _: TEAM_TERRORIST)
                return false
        }
        case 3: {
            if (team != _: TEAM_CT)
                return false
        }
    }

    return Player_MoveToSpawn(player, count)
}

static bool: Player_MoveToSpawn(const player, const count) {
    new team = mp_freeforall ? 0 : get_member(player, m_iTeam)

    new bestSpawnIdx = -1
    for (new attempt; attempt <= 2 && bestSpawnIdx == -1; attempt++) {
        new spawnsCount = count

        // Set random offset for starting search
        new maxSpawn = count - 1
        new potentialSpawnIdx = random_num(0, maxSpawn)
        while(spawnsCount--) {
            // Reset iterator
            if (++potentialSpawnIdx > maxSpawn) {
                potentialSpawnIdx = 0
            }

            // TODO: optimize that!
            if (Spawn_CheckConditions(player, team, potentialSpawnIdx, attempt)) {
                bestSpawnIdx = potentialSpawnIdx
                break
            } else if (attempt == 2) {
                break
            }
        }
    }

    if (bestSpawnIdx == -1) {
        LogMessageEx(Warning, "Player %n can't found good spawn point",
            player
        )

        return false
    }

    new Float: origin[3], Float: angle[3], Float: vAngle[3]
    GetSpawnFromObject(bestSpawnIdx, origin, angle, vAngle)
    new bool: res = Spawn_EntitySetPosition(player, origin, angle, vAngle)
    if (!res) {
        LogMessageEx(Debug, "Player %n can't use a spawn point %i[%.2f,%.2f,%.2f]",
            player, bestSpawnIdx,
            origin[0], origin[1], origin[2]
        )
    }

    return res
}

static bool: Spawn_CheckConditions(const target, const targetTeam, const spawnIdx, const attempt) {
    new Float: spawnOrigin[3], Float: spawnAngle[3], Float: spawnVAngle[3], spawnTeam, spawnGroup[32]
    GetSpawnFromObject(spawnIdx, spawnOrigin, spawnAngle, spawnVAngle, spawnTeam, spawnGroup)

    // Doesn't match because of the spawn team
    if (targetTeam && spawnTeam && (spawnTeam != targetTeam))
        return false

    // TODO: implement spawn group check
    // if (spawnGroup[0] != EOS && spawnGroup[0] == 'A')
    //     return false

    for (new i = 1; i <= MaxClients; i++) {
        if (target == i)
            continue

        if (!is_user_alive(i))
            continue

        // Exclude teammates if need it
        if (targetTeam && targetTeam == pev(i, pev_team)) 
            continue

        new Float: enemyOrigin[3]
        pev(i, pev_origin, enemyOrigin)

        new Float: disatanceToEnemy = get_distance_f(spawnOrigin, enemyOrigin)
        new Float: searchDistance = mp_randomspawn_dist / (attempt + 1)

        if (disatanceToEnemy < 200.0)
            return false

        if (disatanceToEnemy > searchDistance)
            continue

        spawnOrigin[2] += 17.0 // check the head
        if (mp_randomspawn_los) {
            if (/* fm_is_in_viewcone(i, spawnOrigin) && */ fm_is_visible(i, spawnOrigin, true)) {
                return false
            }
        } 
    }

    return true
}

static GetSpawnFromObject(const spawnIdx, Float: origin[3], Float: angle[3], Float: vAngle[3], &team = 0, spawnGroup[] = "") {
    new JSON: spawn = json_array_get_value(g_arrSpawns, spawnIdx)
    team = json_object_get_number(spawn, "team")
    new JSON: arrOrigin = json_object_get_value(spawn, "origin")
    new JSON: arrAngle = json_object_get_value(spawn, "angle")
    new JSON: arrVAngle = json_object_get_value(spawn, "vAngle")
    json_object_get_string(spawn, "group", spawnGroup, 31)

    for (new i; i < sizeof(origin); i++) {
        origin[i] = json_array_get_real(arrOrigin, i)
        if (i < 2) {
            angle[i] = json_array_get_real(arrAngle, i)
            vAngle[i] = json_array_get_real(arrVAngle, i)
        }
    }

    json_free(arrOrigin)
    json_free(arrAngle)
    json_free(arrVAngle)
    json_free(spawn)
}

// TODO: for optimize
static stock GetPlayersOrigin(const startPlayer, Float: playersOrigin[MAX_PLAYERS + 1][3]) {
    for (new i = 1; i <= MaxClients; i++) {
        if (i != startPlayer && !is_user_alive(i))
            continue

        new Float: origin[3]
        pev(i, pev_origin, origin)

        new Float: view_ofs[3]
        pev(i, pev_view_ofs, view_ofs)

        xs_vec_add(playersOrigin[i], origin, view_ofs)
    }
}
