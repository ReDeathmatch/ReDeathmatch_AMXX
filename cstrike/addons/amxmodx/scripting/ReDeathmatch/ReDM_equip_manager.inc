/**
 * Enumeration representing equipment types.
 */
enum EquipType_e {
    et_Primary,     /**< Primary equipment. */
    et_Secondary,   /**< Secondary equipment. */
}

/**
 * Arrays containing equipment section names.
 */
static const g_equipSections[EquipType_e][] = {
    "primary",
    "secondary"
}

/**
 * Represents an invalid equipment choice.
 *
 * This constant is used to indicate that no equipment choice has been made or that the choice is invalid.
 * It is typically used to initialize equipment choices for players or to indicate that a player has not chosen equipment.
 */
enum {
    EQUIP_NOT_CHOOSEN = -1, /**< Represents an invalid or unset equipment choice. */
}

/**
 * Arrays to store player equipment choices.
 */
static Array: g_equip[EquipType_e]
static g_playerWeapons[MAX_PLAYERS + 1][EquipType_e]

static Array: g_botWeapons[EquipType_e]

/**
 * Array to store the last weapon slot chosen by players.
 */
static InventorySlotType: g_lastWeaponSlot[MAX_PLAYERS + 1] = { PRIMARY_WEAPON_SLOT, ... }

static bool: g_playerRandomWeapons[MAX_PLAYERS + 1] = { false, ... }

/**
 * Array to store the weapon state of players.
 */
static WeaponState: g_weaponState[MAX_PLAYERS + 1]

/**
 * Array to store whether players are in the equipment menu.
 */
static bool: g_inEquipMenu[MAX_PLAYERS + 1]

static bool: redm_equip_manager
static redm_keep_weapon_slot
static bool: redm_open_equip_menu
static bool: redm_equip_menu_open_by_g


EquipManager_Init() {
    RegisterHookChain(RG_CBasePlayer_OnSpawnEquip, "CBasePlayer_OnSpawnEquip", .post = false)
    RegisterHookChain(RG_CBasePlayer_OnSpawnEquip, "CBasePlayer_OnSpawnEquip_Post", .post = true)
    // RegisterHookChain(RG_CBasePlayer_GiveNamedItem, "CBasePlayer_GiveNamedItem", .post = false)
    RegisterHookChain(RG_CBasePlayer_AddPlayerItem, "CBasePlayer_AddPlayerItem", .post = false)

    bind_pcvar_num(
        create_cvar(
            "redm_equip_manager", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Enable equip manager."
        ),
        redm_equip_manager
    )
    bind_pcvar_num(
        create_cvar(
            "redm_keep_weapon_slot", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Restore weapon slot on respawn."
        ),
        redm_keep_weapon_slot
    )
    bind_pcvar_num(
        create_cvar(
            "redm_open_equip_menu", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Open weapon equip menu on spawn."
        ),
        redm_open_equip_menu
    )
    bind_pcvar_num(
        create_cvar(
            "redm_equip_menu_open_by_g", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Open equip menu by pressing `G` (drop command)."
        ),
        redm_equip_menu_open_by_g
    )

    register_concmd(
        "redm_dump_equip", "ConCmd_redm_dump_equip",
        ADMIN_MAP,
        "Dump loaded equipset from config to table in console."
    )

    new const equipMenuCmds[][] = {
        "say /guns", "say !guns", "guns"
    }

    for (new i; i < sizeof equipMenuCmds; i++) {
        register_clcmd(equipMenuCmds[i], "ClCmd_Guns", .FlagManager = false)
    }

    register_clcmd("cl_autobuy", "ClCmd_cl_autobuy", .FlagManager = false)
    register_clcmd("drop", "ClCmd_Drop", .FlagManager = false)
}

/**
 * Puts a player into the server.
 *
 * @param player The player to put into the server.
 */
EquipManager_PutInServer(const player) {
    EquipManager_Reset(player)
    g_lastWeaponSlot[player] = PRIMARY_WEAPON_SLOT

    if (!redm_equip_manager)
        return

    if (is_user_bot(player) || is_user_hltv(player))
        return

    set_task(5.0, "Message_YouCanChooseEquip", player)
}

/**
 * Loads equipment configurations from a JSON object.
 *
 * @param objEquip JSON handle containing the equipment configurations.
 */
EquipManager_LoadConfig(const JSON: objEquip) {
    for (new EquipType_e: section; section < EquipType_e; section++) {
        LoadConfigEquip(objEquip, section, g_equip[section])
        LoadConfigBotEquip(objEquip, section)
    }
}

static LoadConfigEquip(const JSON: objEquip, const EquipType_e: section, &Array: equipArray) {
    if (equipArray == Invalid_Array)
        equipArray = ArrayCreate(32)

    ArrayClear(equipArray)

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

        if (ArrayFindString(equipArray, weapon) != -1) {
            LogMessageEx(Warning, "LoadConfigEquip(): WARNING! Weapon `%s` already in equip `%s` list. Skipped.", 
                weapon, g_equipSections[section]
            )

            continue
        }

        ArrayPushString(equipArray, weapon)
    }

    json_free(equipWeapons)
}

static LoadConfigBotEquip(const JSON: objEquip, const EquipType_e: section) {
    if (!json_object_has_value(objEquip, "botEquip"))
        return

    new JSON: equipWeapons = json_object_get_value(objEquip, "botEquip")
    if (equipWeapons == Invalid_JSON)
        set_fail_state("Can't read `%s` section from config", g_equipSections[section])

    if (g_botWeapons[section] == Invalid_Array)
        g_botWeapons[section] = ArrayCreate(32)

    ArrayClear(g_botWeapons[section])
    LoadConfigEquip(equipWeapons, section, g_botWeapons[section])

    json_free(equipWeapons)
}

/**
 * Handles the "cl_autobuy" client command (key F1).
 *
 * @param player The player who issued the command.
 */
public ClCmd_cl_autobuy(const player) {
    if (!IsActive())
        return PLUGIN_CONTINUE
    
    if (!redm_equip_manager)
        return PLUGIN_CONTINUE

    Player_SwitchRandomWeapons(player, .newState = !g_playerRandomWeapons[player])

    return PLUGIN_HANDLED
}

public ClCmd_Drop(const player) {
    if (!IsActive())
        return PLUGIN_CONTINUE

    if (!redm_equip_menu_open_by_g)
        return PLUGIN_CONTINUE

    return ClCmd_Guns(player)
}

/**
 * Handles the "/guns" client command.
 *
 * @param player The player who issued the command.
 */
public ClCmd_Guns(const player) {
    if (!IsActive())
        return PLUGIN_CONTINUE

    if (!redm_equip_manager)
        return PLUGIN_CONTINUE

    Player_CallEquipMenu(player)

    return PLUGIN_HANDLED
}

/**
 * Toggles random weapons mode for a player.
 *
 * This function allows a player to toggle between having random weapons assigned to them
 * or using a predefined equipment set. When random weapons mode is enabled, the player
 * will receive random weapons from the available pool of weapons. When disabled, the
 * player's equipment will be reset to the predefined set.
 *
 * @param player The player for whom to toggle random weapons mode.
 *
 * @return Returns true if random weapons mode is enabled after toggling, false otherwise.
 */
static bool: Player_SwitchRandomWeapons(const player, const bool: newState) {
    if (!IsActive())
        return false

    if (is_user_bot(player))
        return false

    SetGlobalTransTarget(player)

    g_playerRandomWeapons[player] = newState

    if (!g_playerRandomWeapons[player]) {
        EquipManager_Reset(player)
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
        EquipManager_Reset(player)
    }

    for (new EquipType_e: section; section < EquipType_e; section++) {
        if (!HasEquipItems(player, section))
            continue

        Menu_ChooseEquip(player, section)

        return true
    }

    return true
}

static Menu_ChooseEquip(const player, const EquipType_e: section) {
    SetGlobalTransTarget(player)

    new menuTitle[128]
    formatex(menuTitle, charsmax(menuTitle), "%l", section == et_Primary ? "PrimaryEquip" : "SecondaryEquip")

    if (redm_equip_menu_open_by_g)
        strcat(menuTitle, fmt("^n%l", "ToClosePressG"), charsmax(menuTitle))

    new menu = menu_create(menuTitle, "MenuHandler_ChooseEquip")

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
    
    if (!redm_equip_manager)
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

    if (g_playerRandomWeapons[player])
        Player_SwitchRandomWeapons(player, .newState = false)

    Player_GiveWeapon(player, section)

    UTIL_PlaySoundEffect(player, "fvox/blip.wav", .pitch = 80)

    new bool: hasNextSection = (++section % EquipType_e) != EquipType_e: 0
    if (!hasNextSection) {
        client_print_color(player, print_team_red, "[Re:DM] %l",
            "YouCanChooseEquip"
        )

        return PLUGIN_HANDLED
    }
    
    if (!HasEquipItems(player, section))
        return PLUGIN_HANDLED

    Menu_ChooseEquip(player, section)

    return PLUGIN_HANDLED
}

public CBasePlayer_OnSpawnEquip(const player, bool: addDefault, bool: equipGame) {
    if (!IsActive())
        return

    RoundModes_OnSpawnEquip(player)
    ModeVote_OnSpawnEquip(player)

    if (!redm_equip_manager)
        return

    SetHookChainArg(3, ATYPE_BOOL, (equipGame = false))

    set_member(player, m_bNotKilled, false)
    rg_set_user_armor(player, 0, ARMOR_NONE)

    if (redm_keep_weapon_slot)
        RequestFrame("Player_ForceSlotChoose", player)

    new bool: fullyEquipped = true
    for (new EquipType_e: section; section < EquipType_e; section++) {
        if (!HasEquipItems(player, section))
            continue

        if (g_playerRandomWeapons[player])
            g_playerWeapons[player][section] = random_num(0, ArraySize(GetEquipConfig(player, section)) - 1)

        if (g_playerWeapons[player][section] != EQUIP_NOT_CHOOSEN)
            continue

        fullyEquipped = false
    }

    if (fullyEquipped) {
        rg_give_item(player, "weapon_knife")

        SetHookChainArg(2, ATYPE_BOOL, (addDefault = false))
    }
}

public CBasePlayer_OnSpawnEquip_Post(const player, bool: addDefault, bool: equipGame) {
    if (!IsActive())
        return

    if (!redm_equip_manager)
        return

    new hasConfiguredEquip = false
    for (new EquipType_e: section; section < EquipType_e; section++) {
        if (g_playerWeapons[player][section] == EQUIP_NOT_CHOOSEN)
            continue

        hasConfiguredEquip = true
        Player_GiveWeapon(player, section)
    }

    if (hasConfiguredEquip)
        return

    // TODO: smells like a shit. need to rework later
    if (!hasConfiguredEquip && redm_open_equip_menu) {
        if (!g_inEquipMenu[player]) {
            new EquipType_e: section = (g_playerWeapons[player][et_Primary] == EQUIP_NOT_CHOOSEN) ? et_Primary : et_Secondary

            Menu_ChooseEquip(player, section)
        }
    }
}

public CBasePlayer_AddPlayerItem(const player, const item) {
    if (!IsActive())
        return

    if (!redm_equip_manager)
        return

    // Restore weaponState
    set_member(item, m_Weapon_iWeaponState, g_weaponState[player])
}

/**
 * Gets the weapon slot of a given weapon.
 *
 * @param weaponId The ID of the weapon.
 * @return         The weapon slot type.
 */
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

EquipManager_PlayerKilled(const victim) {
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

/**
 * Gives a player a specific type of equipment.
 *
 * @param player  The player to receive the equipment.
 * @param section The type of equipment (primary or secondary).
 * @return        True if equipment was given, false otherwise.
 */
static bool: Player_GiveWeapon(const player, const EquipType_e: section) {
    if (g_playerWeapons[player][section] == EQUIP_NOT_CHOOSEN)
        return false

    new weaponName[32]
    ArrayGetString(
        GetEquipConfig(player, section),
        g_playerWeapons[player][section],
        weaponName,
        charsmax(weaponName)
    )

    new item = rg_give_item(player, weaponName, .type = GT_REPLACE)

    // Reset last item
    set_member(player, m_pLastItem, item)

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

/**
 * Resets a player's equipment choices to the default state.
 *
 * This function resets a player's equipment choices for both primary and secondary weapons
 * to the default state, allowing them to choose their equipment again.
 *
 * @param player The player for whom to reset equipment choices.
 */
EquipManager_Reset(const player) {
    for (new EquipType_e: section; section < EquipType_e; section++) {
        g_playerWeapons[player][section] = EQUIP_NOT_CHOOSEN
    }

    g_playerRandomWeapons[player] = bool: is_user_bot(player)
}

/**
 * Checks if a given equipment type section has available items.
 *
 * This function checks if a specific equipment type section (primary or secondary) has any
 * available items in its equipment list.
 *
 * @param section The equipment type section to check (et_Primary or et_Secondary).
 *
 * @return Returns true if the specified section has available items, false otherwise.
 */
static bool: HasEquipItems(const player, const EquipType_e: section) {
    return ArraySize(GetEquipConfig(player, section)) != 0
}

static Array: GetEquipConfig(const player, const EquipType_e: section) {
    if (!is_user_bot(player))
        return g_equip[section]

    if (g_botWeapons[section] == Invalid_Array || ArraySize(g_botWeapons[section]) == 0)
        return g_equip[section]

    return g_botWeapons[section]
}

public ConCmd_redm_dump_equip(const player, const level, const commandId) {
    if (!cmd_access(player, level, commandId, 1))
        return PLUGIN_HANDLED

    DumpEquip(player)

    return PLUGIN_HANDLED
}

/**
 * Dumps the current equipment set to the console.
 *
 * This function displays the current equipment set, including primary and secondary weapons,
 * in a tabular format in the player's console.
 *
 * @param player The player for whom to display the equipment set. If set to 0, it will be displayed in the server console.
 *
 * @remark This function is typically used for debugging and informational purposes.
 */
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

/**
 * Sends a message to a player indicating they can choose their equipment.
 *
 * This function sends a message to a player indicating that they can choose their equipment.
 * It typically notifies players when they are allowed to select their primary and secondary weapons.
 *
 * @param player The player to whom the message should be sent.
 */
public Message_YouCanChooseEquip(const player) {
    SetGlobalTransTarget(player)

    client_print_color(player, print_team_red, "[Re:DM] %l", "YouCanChooseEquip")
}
