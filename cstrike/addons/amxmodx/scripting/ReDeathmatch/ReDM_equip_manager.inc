enum EquipType_e {
    et_Primary,
    et_Secondary,
}
static const g_equipSections[EquipType_e][] = {
    "primary",
    "secondary"
}

enum {
    EQUIP_NOT_CHOOSEN = -1,
}

static g_equip[EquipType_e]
static g_playerWeapons[MAX_PLAYERS + 1][EquipType_e] // = []{ EQUIP_NOT_CHOOSEN, ... }

static InventorySlotType: g_lastWeaponSlot[MAX_PLAYERS + 1] = { PRIMARY_WEAPON_SLOT, ... }
static bool: g_playerRandomWeapons[MAX_PLAYERS + 1] = { false, ... }
static WeaponState: g_weaponState[MAX_PLAYERS + 1]
static bool: g_inEquipMenu[MAX_PLAYERS + 1]

static redm_keep_weapon_slot = 1
static bool: redm_open_equip_menu = false

EquipManager_Init() {
    RegisterHookChain(RG_CBasePlayer_GiveDefaultItems, "CBasePlayer_GiveDefaultItems", .post = false)
    // RegisterHookChain(RG_CBasePlayer_GiveNamedItem, "CBasePlayer_GiveNamedItem", .post = false)
    RegisterHookChain(RG_CBasePlayer_AddPlayerItem, "CBasePlayer_AddPlayerItem", .post = false)

    bind_pcvar_num(
        create_cvar(
            "redm_keep_weapon_slot", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .description = "Restore weapon slot on respawn."
        ),
        redm_keep_weapon_slot
    )
    bind_pcvar_num(
        create_cvar(
            "redm_open_equip_menu", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .description = "Open weapon equip menu on spawn."
        ),
        redm_open_equip_menu
    )

    register_concmd("redm_dump_equip", "ConCmd_redm_dump_equip", ADMIN_MAP, "Dump loaded equipset from config to table in console.")

    register_clcmd("drop", "ClCmd_Drop")
    register_clcmd("say /guns", "ClCmd_Drop")
    register_clcmd("say !guns", "ClCmd_Drop")
    register_clcmd("guns", "ClCmd_Drop")
    register_clcmd("cl_autobuy", "ClCmd_cl_autobuy")
}

EquipManager_PutInServer(const player) {
    EquipManager_PlayerResetEquip(player)
    g_lastWeaponSlot[player] = PRIMARY_WEAPON_SLOT

    if (is_user_bot(player) || is_user_hltv(player))
        return

    set_task(5.0, "Message_YouCanChooseEquip", player)
}

EquipManager_LoadConfig(const JSON: objEquip) {
    for (new EquipType_e: section; section < EquipType_e; section++) {
        LoadConfigEquip(objEquip, section)
    }
}

static LoadConfigEquip(const JSON: objEquip, const EquipType_e: section) {
    if (g_equip[section] == _: Invalid_Array) {
        g_equip[section] = _: ArrayCreate(32)
    } 

    ArrayClear(Array: g_equip[section])

    if (!json_object_has_value(objEquip, g_equipSections[section]))
        return

    new JSON: equipWeapons = json_object_get_value(objEquip, g_equipSections[section])
    if (equipWeapons == Invalid_JSON) {
        set_fail_state("Can't read `%s` section from config", g_equipSections[section])
    }

    new count = json_array_get_count(equipWeapons)
    if (!count) {
        json_free(equipWeapons)
        return
    }

    for (new i; i < count; i++) {
        new weapon[32]
        json_array_get_string(equipWeapons, i, weapon, charsmax(weapon))

        new weaponId = rg_get_weapon_info(weapon, WI_ID)
        if (!weaponId) {
            LogMessageEx(Warning, "LoadConfigEquip(): WARNING! Weapon `%s` hasn't ID. Invalid weapon skipped.", 
                weapon
            )

            continue
        }

        if (ArrayFindString(Array: g_equip[section], weapon) != -1) {
            LogMessageEx(Warning, "LoadConfigEquip(): WARNING! Weapon `%s` already in equip `%s` list. Skipped.", 
                weapon, g_equipSections[section]
            )

            continue
        }

        ArrayPushString(Array: g_equip[section], weapon)
    }

    json_free(equipWeapons)
}

public ClCmd_cl_autobuy(const player) {
    if (!IsActive())
        return PLUGIN_CONTINUE

    Player_SwitchRandomWeapons(player)

    return PLUGIN_HANDLED
}

public ClCmd_Drop(const player) {
    if (!IsActive())
        return PLUGIN_CONTINUE

    Player_CallEquipMenu(player)

    return PLUGIN_HANDLED
}

static bool: Player_SwitchRandomWeapons(const player) {
    if (!IsActive())
        return false

    SetGlobalTransTarget(player)

    g_playerRandomWeapons[player] = !g_playerRandomWeapons[player]

    if (!g_playerRandomWeapons[player]) {
        EquipManager_PlayerResetEquip(player)
    }

    UTIL_PlaySoundEffect(player, "buttons/button9.wav", .pitch = g_playerRandomWeapons[player] ? PITCH_HIGH : PITCH_LOW)

    client_print_color(player, print_team_red, "[Re:DM] %l - ^3%l^1.",
        "RandomWeapons",
        g_playerRandomWeapons[player] ? "Enabled" : "Disabled"
    )

    return g_playerRandomWeapons[player]
}

static bool: Player_CallEquipMenu(const player) {
    if (!IsActive())
        return false

    new menuId, newMenuId
    player_menu_info(player, menuId, newMenuId)

    if (newMenuId == -1) {
        g_inEquipMenu[player] = false
    }

    if (g_inEquipMenu[player] || newMenuId != -1) {
        UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 30)
        reset_menu(player)

        return false
    }
    
    if (Player_HasChosenEquip(player)) {
        EquipManager_PlayerResetEquip(player)
    }

    for (new EquipType_e: section; section < EquipType_e; section++) {
        if (!HasEquipItems(section))
            continue

        Menu_ChooseEquip(player, section)

        return true
    }

    return true
}

static Menu_ChooseEquip(const player, const EquipType_e: section) {
    SetGlobalTransTarget(player)

    new menu = menu_create(
        fmt("%l^n%l",
            section == et_Primary ? "PrimaryEquip" : "SecondaryEquip",
            "ToClosePressG"
        ),
        "MenuHandler_ChooseEquip"
    )

    static callback
    if(!callback)
        callback = menu_makecallback("MenuCallback_Primary")

    for (new i, size = ArraySize(Array: g_equip[section]); i < size; i++) {
        new equipName[32]
        ArrayGetString(Array: g_equip[section], i, equipName, charsmax(equipName))

        new weaponId = rg_get_weapon_info(equipName, WI_ID)

        new weaponName[32]
        rg_get_weapon_info(weaponId, WI_NAME, weaponName, charsmax(weaponName))

        menu_additem(menu, weaponName, .info = fmt("%i", section), .callback = callback)
    }

    menu_setprop(menu, MPROP_BACKNAME, fmt("%l", "BACK"))
    menu_setprop(menu, MPROP_NEXTNAME, fmt("%l", "MORE"))
    menu_setprop(menu, MPROP_EXITNAME, fmt("%l", "EXIT"))
    menu_setprop(menu, MPROP_NUMBER_COLOR, "\y")

    g_inEquipMenu[player] = true
    menu_display(player, menu)

    return PLUGIN_HANDLED
}

public MenuCallback_Primary(const player, const menu, const item) {
    new name[32]
    menu_item_getinfo(
        menu, item,
        .name = name,
        .namelen = charsmax(name)
    )

    if (strncmp(name, "weapon_", 7) == 0) {
        replace(name, charsmax(name), "weapon_", "")
        strtoupper(name)
    }

    menu_item_setname(menu, item, name)

    return ITEM_IGNORE
}

public MenuHandler_ChooseEquip(const player, const menu, const item) {
    if (!IsActive())
        return PLUGIN_HANDLED

    new info[2]
    menu_item_getinfo(
        menu, item,
        .info = info,
        .infolen = charsmax(info)
    )

    g_inEquipMenu[player] = false
    menu_destroy(menu)

    if(item < 0) {
        UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 30)

        return PLUGIN_HANDLED
    }

    new EquipType_e: section = EquipType_e: strtol(info)
    g_playerWeapons[player][section] = item

    Player_GiveWeapon(player, section)

    UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 80)

    new bool: hasNextSection = (++section % EquipType_e) != EquipType_e: 0
    if (!hasNextSection) {
        client_print_color(player, print_team_red, "[Re:DM] %l",
            "YouCanChooseEquip"
        )

        return PLUGIN_HANDLED
    }
    
    if (!HasEquipItems(section))
        return PLUGIN_HANDLED

    Menu_ChooseEquip(player, section)

    return PLUGIN_HANDLED
}

public CBasePlayer_GiveDefaultItems(const player) {
    if (!IsActive())
        return HC_CONTINUE

    ModeVote_GiveDefaultItems(player)

    if (is_user_bot(player))
        return HC_CONTINUE

    if (redm_keep_weapon_slot)
        RequestFrame("Player_ForceSlotChoose", player)

    rg_give_item(player, "weapon_knife")

    for (new EquipType_e: section; section < EquipType_e; section++) {
        if (!HasEquipItems(section))
            continue
        
        if (g_playerRandomWeapons[player]) {
            g_playerWeapons[player][section] = random_num(0, ArraySize(Array: g_equip[section]) - 1)
        }

        if (g_playerWeapons[player][section] == EQUIP_NOT_CHOOSEN) {
            if (redm_open_equip_menu)
                Menu_ChooseEquip(player, section)

            return HC_CONTINUE
        }

        Player_GiveWeapon(player, section)
    }

    return HC_SUPERCEDE
}

public CBasePlayer_AddPlayerItem(const player, const item) {
    if (!IsActive())
        return

    // Restore weaponState
    set_member(item, m_Weapon_iWeaponState, g_weaponState[player])
}

static stock InventorySlotType: GetWeaponSlot(const weaponId) {
    // TODO: ReAPI update
    static const weaponSlotInfo[] = {0,2,0,1,4,1,5,1,1,4,2,2,1,1,1,1,2,2,1,1,1,1,1,1,1,4,2,1,1,3,1}

    if (weaponId < 0 || weaponId >= sizeof(weaponSlotInfo))
        return NONE_SLOT
    
    return InventorySlotType: weaponSlotInfo[weaponId]
}

public CBasePlayer_GiveNamedItem(const player, const weaponName[]) {
    // TODO: doesn't work now, need ReGameDLL fixes 
    /* 
    if (!Player_HasChosenEquip(player))
        return HC_CONTINUE

    new weaponId = rg_get_weapon_info(weaponName, WI_ID)
    new EquipType_e: receiveWeaponSlot = EquipType_e: (GetWeaponSlot(weaponId) - InventorySlotType: 1)

    if (receiveWeaponSlot < et_Primary || receiveWeaponSlot >= EquipType_e)
        return HC_CONTINUE

    if (g_playerWeapons[player][receiveWeaponSlot] != EQUIP_NOT_CHOOSEN) {
        Player_GiveWeapon(player, receiveWeaponSlot)
        return HC_SUPERCEDE
    }

    return HC_CONTINUE
    */
}

public EquipManager_PlayerKilled(const victim, const killer) {
    new item = get_member(victim, m_pActiveItem)
    if (!is_entity(item))
        return

    Player_SaveActiveSlot(victim, item)
    Player_SaveWeaponState(victim, item)
}

static Player_SaveActiveSlot(const player, const item) {
    new InventorySlotType: slot = InventorySlotType: (rg_get_iteminfo(item, ItemInfo_iSlot) + 1)
    if (slot >= KNIFE_SLOT)
        return

    g_lastWeaponSlot[player] = slot
}

static Player_SaveWeaponState(const player, const item) {
    g_weaponState[player] = get_member(item, m_Weapon_iWeaponState)
}

public Player_ForceSlotChoose(const player) {
    if (!is_user_alive(player))
        return

    new item = get_member(player, m_rgpPlayerItems, g_lastWeaponSlot[player])
    if (!is_entity(item))
        return

    new weaponName[32]
    rg_get_weapon_info(get_member(item, m_iId), WI_NAME, weaponName, charsmax(weaponName))

    // Restore activeItem
    rg_internal_cmd(player, weaponName)
}

static bool: Player_GiveWeapon(const player, const EquipType_e: section) {
    if (g_playerWeapons[player][section] == EQUIP_NOT_CHOOSEN)
        return false

    new weaponName[32]
    ArrayGetString(
        Array: g_equip[section],
        g_playerWeapons[player][section],
        weaponName,
        charsmax(weaponName)
    )

    rg_give_item(player, weaponName, .type = GT_REPLACE)
    return true
}

static bool: Player_HasChosenEquip(const player) {
    for (new EquipType_e: section; section < EquipType_e; section++) {
        if (g_playerWeapons[player][section] != EQUIP_NOT_CHOOSEN) {
            return true
        }
    }

    return false
}

EquipManager_PlayerResetEquip(const player) {
    for (new EquipType_e: section; section < EquipType_e; section++) {
        g_playerWeapons[player][section] = EQUIP_NOT_CHOOSEN
    }
}

static bool: HasEquipItems(const EquipType_e: section) {
    return ArraySize(Array: g_equip[section]) != 0
}

public ConCmd_redm_dump_equip(const player) {
    DumpEquip(player)

    return PLUGIN_HANDLED
}

static DumpEquip(const player = 0) {
    if (Array: g_equip[et_Primary] == Invalid_Array && Array: g_equip[et_Secondary] == Invalid_Array) {
        console_print(player, "DumpEquip(): WARNING! Equip not itinialized!")
        return
    }

    console_print(player, "^nDump current equipset:")
    console_print(player, "| %-2s | %-16s | %-16s |",
        "#", "Primary", "Secondary"
    )

    new const template[] = "| %-2i | %-16s | %-16s |"
    console_print(player, "| -- | ---------------- | ---------------- |")
    
    new primaryCount = ArraySize(Array: g_equip[et_Primary])
    new secondaryCount = ArraySize(Array: g_equip[et_Secondary])
    new size = max(primaryCount, secondaryCount)

    for (new i; i < size; i++) {
        new primaryWeaponName[32], secondaryWeaponName[32]

        if (i < primaryCount)
            ArrayGetString(Array: g_equip[et_Primary], i, primaryWeaponName, charsmax(primaryWeaponName))
        
        if (i < secondaryCount)
            ArrayGetString(Array: g_equip[et_Secondary], i, secondaryWeaponName, charsmax(secondaryWeaponName))

        console_print(
            player,
            template,
            i + 1,
            primaryWeaponName,
            secondaryWeaponName
        )
    }

    console_print(player, "^n")
}

public Message_YouCanChooseEquip(const player) {
    SetGlobalTransTarget(player)

    client_print_color(player, print_team_red, "[Re:DM] %l", "YouCanChooseEquip")
}
