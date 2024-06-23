static g_respawnsCount[any: TeamName]

static redm_tickets
static Float: redm_tickets_hud_update_freq
static Float: redm_tickets_hud_blink_percent
static redm_tickets_hud_type
static Float: redm_tickets_hud_x
static Float: redm_tickets_hud_y
static redm_tickets_hud_gap

Tickets_Init() {
    RegisterHookChain(RG_CBasePlayer_Killed, "CBasePlayer_Killed", .post = true)
    RegisterHookChain(RG_CSGameRules_FPlayerCanRespawn, "CSGameRules_FPlayerCanRespawn", .post = true)

    bind_pcvar_num(
        create_cvar(
            "redm_tickets", "0",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_INTEGER,
            .description = "Number of times a team can ^n\
                have players respawn before they stop ^n\
                being able to respawn. ^n\
                `0` - disabled"
        ),
        redm_tickets
    )
    hook_cvar_change(get_cvar_pointer("redm_tickets"), "CvarChange_redm_tickets")

    bind_pcvar_float(
        create_cvar(
            "redm_tickets_hud_update_freq", "1.0",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_FLOAT,
            .description = "Tickets HUD update frequency."
        ),
        redm_tickets_hud_update_freq
    )
    bind_pcvar_float(
        create_cvar(
            "redm_tickets_hud_blink_percent", "10",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 100.0,
            .flags = _FCVAR_FLOAT,
            .description = "Minimum percentage of tickets to start blinking."
        ),
        redm_tickets_hud_blink_percent
    )
    bind_pcvar_num(
        create_cvar(
            "redm_tickets_hud_type", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_INTEGER,
            .description = "Ticket HUD display type. ^n\
                0 - without colors and effects; ^n\
                1 - color, with effects."
        ),
        redm_tickets_hud_type
    )
    bind_pcvar_float(
        create_cvar(
            "redm_tickets_hud_x", "0.5",
            .has_min = true, .min_val = -1.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_FLOAT,
            .description = "Tickets HUD X position."
        ),
        redm_tickets_hud_x
    )
    bind_pcvar_float(
        create_cvar(
            "redm_tickets_hud_y", "0.12",
            .has_min = true, .min_val = -1.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_FLOAT,
            .description = "Tickets HUD Y position."
        ),
        redm_tickets_hud_y
    )
    bind_pcvar_num(
        create_cvar(
            "redm_tickets_hud_gap", "10",
            .flags = _FCVAR_INTEGER,
            .description = "Space between tickets for teams (when type = 2)."
        ),
        redm_tickets_hud_gap
    )

    CTickets_UpdateHUD()
}

/**
 * CvarChange_redm_tickets function
 * Called when the value of the "redm_tickets" configuration variable changes
 * 
 * @param cvar      Name of the configuration variable whose value has changed
 * @param oldValue  Previous value of the configuration variable
 * @param value     New value of the configuration variable
 * 
 * The function performs the following actions:
 * 1. Updates the information about the number of tickets on the HUD
 * 2. For each connected player:
 *    - Retrieves the team the player belongs to
 *    - If the player's team has 0 tickets left, skips this player
 *    - Allows the player to spawn, if possible
 */
public CvarChange_redm_tickets(const cvar, const oldValue[], const value[]) {
    CTickets_UpdateHUD()
    for (new player = 1; player <= MaxClients; player++) {
        if (!is_user_connected(player))
            continue
        
        new TeamName: team = get_member(player, m_iTeam)
        if (CTickets_TeamTicketsLeft(team) == 0)
            continue

        CTickets_PreventPlayerSpawning(player, .isSpawnAvailable = true)
    }
}

Tickets_RestartRound() {
    arrayset(g_respawnsCount, 0, sizeof(g_respawnsCount))

    CTickets_UpdateHUD()
}

public CBasePlayer_Killed(const player, const killer, const gib) {
    if (!IsActive())
        return

    if (redm_tickets <= 0)
        return

    new TeamName: team = get_member(player, m_iTeam)
    g_respawnsCount[team]++

    if (CTickets_TeamTicketsLeft(team) != 0)
        return

    CTickets_PreventPlayerSpawning(player)
}

/**
 * Determines whether a player is allowed to respawn
 * 
 * @param player    The player who wants to respawn
 */
public CSGameRules_FPlayerCanRespawn(const player) {
    if (!IsActive())
        return HC_CONTINUE

    if (redm_tickets <= 0)
        return HC_CONTINUE

    new TeamName: team = get_member(player, m_iTeam)

    SetHookChainReturn(ATYPE_INTEGER, CTickets_TeamTicketsLeft(team) != 0)
    return HC_SUPERCEDE
}

public CTickets_UpdateHUD() {
    if (!IsActive())
        return

    if (redm_tickets <= 0)
        return

    CTickets_ShowPlayerTicketsHUD(.player = 0)

    const taskId = 9999
    remove_task(taskId)
    set_task_ex(redm_tickets_hud_update_freq, "CTickets_UpdateHUD", .id = taskId)
}

/**
 * Displays the player's team tickets HUD.
 *
 * @param player    The player index to display the HUD for. If 0, it will be displayed for all players.
 */
static CTickets_ShowPlayerTicketsHUD(const player = 0) {
    new ticketsT = CTickets_TeamTicketsLeft(TEAM_TERRORIST) 
    new ticketsCT = CTickets_TeamTicketsLeft(TEAM_CT)
    new digitsCount = UTIL_getDigitsCount(redm_tickets)

    /*  0.0093 = 16:9
        0.0167 = 4:3 */
    const Float: charSize =  0.0167
    const Float: overlapTime = 0.1

    static digitsTemplate[32]
    formatex(digitsTemplate, charsmax(digitsTemplate),
        "%%" + "0%i" + "d", digitsCount
    )

    if (redm_tickets_hud_type == 0) {
        new buffer[256]
        formatex(buffer, charsmax(buffer), "%l",
            "TicketsTemplateType1",
            digitsTemplate,
            digitsTemplate
        )

        formatex(buffer, charsmax(buffer), buffer,
            ticketsT,
            ticketsCT    
        )

        /*  0.72 = 4:3
            0.442 = 16:9 */
        const Float: charsFactor = 0.72
        new Float: stringWidth = strlen(buffer) * charSize * charsFactor
        new Float: centerOffset = stringWidth / -2.0

        set_dhudmessage(
            .red = 200,
            .green = 200,
            .blue = 200,
            .x = (redm_tickets_hud_x + centerOffset),
            .y = redm_tickets_hud_y,
            .effects = 0,
            .fadeintime = 0.0,
            .fadeouttime = 0.0,
            .holdtime = (redm_tickets_hud_update_freq + overlapTime)
        )
        show_dhudmessage(player, "%s", buffer)

        return
    }

    new TeamName: looserTeam = TEAM_UNASSIGNED
    if (ticketsCT < ticketsT)
        looserTeam = TEAM_CT
    else if (ticketsCT > ticketsT)
        looserTeam = TEAM_TERRORIST

    new bool: shouldBlink = CTickets_TeamTicketsLeft(looserTeam) <= (redm_tickets * (redm_tickets_hud_blink_percent / 100))
    if (!shouldBlink)
        looserTeam = TEAM_UNASSIGNED

    new Float: fadeoutTime = (redm_tickets_hud_update_freq * 0.5)
    new Float: holdTimeBlink = (redm_tickets_hud_update_freq - fadeoutTime)

    // Position calculating
    new Float: digitsWidth = digitsCount * charSize
    new Float: centerOffset = digitsWidth / -2.0

    /*  200 = 16:9
        120 = 4:3 */
    const Float: paddingFactor = 120.0
    new Float: padding = (digitsCount / paddingFactor) + ((redm_tickets_hud_gap * charSize) / 2)

    // T tickets
    set_dhudmessage(
        .red = 255,
        .green = 100,
        .blue = 0,
        .x = (redm_tickets_hud_x + centerOffset) - padding,
        .y = redm_tickets_hud_y,
        .effects = 0,
        .fadeintime = 0.0,
        .fadeouttime = (looserTeam == TEAM_TERRORIST) ? fadeoutTime : 0.0,
        .holdtime = (looserTeam == TEAM_TERRORIST) ? holdTimeBlink : (redm_tickets_hud_update_freq + overlapTime)
    )
    show_dhudmessage(player, digitsTemplate, ticketsT)

    // CT tickets
    set_dhudmessage(
        .red = 0,
        .green = 100,
        .blue = 255,
        .x = (redm_tickets_hud_x + centerOffset) + padding,
        .y = redm_tickets_hud_y,
        .effects = 0,
        .fadeintime = 0.0,
        .fadeouttime = (looserTeam == TEAM_CT) ? fadeoutTime : 0.0,
        .holdtime = (looserTeam == TEAM_CT) ? holdTimeBlink : (redm_tickets_hud_update_freq + overlapTime)
    )

    show_dhudmessage(player, digitsTemplate, ticketsCT)
}

/**
 * Prevents or allows a player to spawn based on the remaining team tickets.
 *
 * @param player            The player index.
 * @param isSpawnAvailable  Indicates whether the player should be allowed to spawn.
 * @return  The player's respawn pending time.
 */
static Float: CTickets_PreventPlayerSpawning(const player, const bool: isSpawnAvailable = false) {
    set_member(player, m_flRespawnPending, isSpawnAvailable ? get_gametime() : 9999999.0)

    return get_member(player, m_flRespawnPending)
}

/**
 * Calculates the number of tickets left for the given team.
 *
 * @param team  The team to check.
 * @return The number of tickets left for the team.
 */
static CTickets_TeamTicketsLeft(const TeamName: team) {
    return max(0, redm_tickets - g_respawnsCount[team])
}
