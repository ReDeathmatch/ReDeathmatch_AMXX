
// Players modes vote item.
enum {
    VOTE_NOTVOTED = -1,
}

enum _Vote_s {
    Item,
    VotesCount,
}

static Array: g_vote = Invalid_Array

static g_votes[MAX_PLAYERS + 1] = { VOTE_NOTVOTED, ... }
static Array: g_playersInVote = Invalid_Array

static g_modeExtendedTimes

new bool: g_voteStarted

// continue cur mode

static redm_mode_vote_time = 15
static bool: redm_mode_vote_enable = true
static redm_mode_vote_extend_times = 1

ModeVote_Init() {
    RegisterHookChain(RG_CBasePlayer_ResetMaxSpeed, "CBasePlayer_ResetMaxSpeed_Post", .post = true)

    bind_pcvar_num(
        create_cvar(
            "redm_mode_vote_enable", "1",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Enable voting for the next game mode."
        ),
        redm_mode_vote_enable
    )
    bind_pcvar_num(
        create_cvar(
            "redm_mode_vote_time", "15",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_INTEGER,
            .description = "Time to vote for the next mode (sec)."
        ),
        redm_mode_vote_time
    )
    bind_pcvar_num(
        create_cvar(
            "redm_mode_vote_extend_times", "1",
            .has_min = true, .min_val = 0.0,
            .flags = _FCVAR_INTEGER,
            .description = "How many times can the game mode be extended."
        ),
        redm_mode_vote_extend_times
    )
}
 
bool: ModeVote_RoundEnd_Post() {
    if (!redm_mode_vote_enable)
        return false

    if (!get_playersnum_ex(GetPlayers_ExcludeBots | GetPlayers_ExcludeHLTV))
        return false

    if (RoundModes_IsModeForced())
        return false

    VoteStart(redm_mode_vote_time)

    return true
}

ModeVote_OnSpawnEquip(const player) {
    if (!g_voteStarted)
        return

    // TODO: mb CBasePlayer::Spawn_Post() or smth
    set_task(0.4, "ApplyEffectsToPlayers", player)
}

ModeVote_Disconnected(const player) {
    if (!g_voteStarted)
        return

    Vote_RemovePlayerFromVoters(player)

    new votedModeIdx = g_votes[player]
    Vote_PlayerVote(player, votedModeIdx, -1)

    g_votes[player] = VOTE_NOTVOTED
}

ModeVote_RestartRound() {
    if (!g_voteStarted)
        return VOTE_NOTVOTED

    new votedModeIdx = VoteEnd()

    if (votedModeIdx == RoundModes_GetCurrentMode())
        ++g_modeExtendedTimes
    else g_modeExtendedTimes = 0

    return votedModeIdx
}

VoteStart(const voteTime = 15) {
    set_member_game(m_iRoundWinStatus, WINSTATUS_NONE)
    set_member_game(m_flRestartRoundTime, get_gametime() + float(voteTime))
    set_member_game(m_bRoundTerminating, false)
    set_member_game(m_bFreezePeriod, true)

    g_voteStarted = true

    arrayset(g_votes, VOTE_NOTVOTED, sizeof(g_votes))
    Vote_FillVoters()
    Vote_FillVote()

    ApplyEffectsToPlayers(0)
    VoteTimer_Counter()
    set_task_ex(1.0, "VoteTimer_Counter", .flags = SetTask_RepeatTimes, .repeat = voteTime - 1)
}

public VoteTimer_Counter() {
    Menu_VoteMode()
}

Vote_GetItemVotesCount(const item) {
    new nominee[_Vote_s]
    new foundIdx = FindVoteIdxByItem(item, nominee)

    if (foundIdx == -1)
        return 0

    return nominee[VotesCount]
}

Vote_PlayerVote(const player, const item, const count = 1) {
    new nominee[_Vote_s]
    new foundIdx = FindVoteIdxByItem(item, nominee)

    if (foundIdx == -1)
        return 0

    g_votes[player] = item

    nominee[VotesCount] += count
    ArraySetArray(g_vote, foundIdx, nominee, sizeof(nominee))

    return nominee[VotesCount]
}

FindVoteIdxByItem(const item, nominee[_Vote_s]) {
    for (new idx, l = ArraySize(g_vote); idx < l; idx++) {
        ArrayGetArray(g_vote, idx, nominee, sizeof(nominee))

        if (nominee[Item] != item)
            continue

        return idx
    }

    return -1
}

bool: Vote_IsPlayerVoted(const player) {
    return g_votes[player] != VOTE_NOTVOTED
}

Vote_PlayerGetVote(const player) {
    return g_votes[player]
}

VoteEnd() {
    reset_menu(0)

    new modeIdx = Vote_GetResult()

    g_voteStarted = false

    return modeIdx
}

#if defined SECOND_VOTE
Vote_GetResult() {
    // TODO: meh o/....
    ArraySortEx(g_vote, "CompareVoteItems", { Sort_Descending }, 1)

    new nominee[_Vote_s]
    ArrayGetArray(g_vote, 0, nominee, sizeof(nominee))

    new itemsSize = Menu_GetVoteItemsCount()

    new bestItem = nominee[Item]
    new bestResult = nominee[VotesCount]
    if (!bestResult)
        return random_num(0, itemsSize - 1)

    new nomineesCount = 1
    for (new i = 1; i < itemsSize; i++) {
        ArrayGetArray(g_vote, i, nominee, sizeof(nominee))

        if (nominee[VotesCount] == bestResult)
            ++nomineesCount
    }

    if (nomineesCount < 2)
        return bestItem

    ArrayGetArray(g_vote, random_num(0, nomineesCount - 1), nominee, sizeof(nominee))

    return nominee[Item]
}
#endif

Vote_GetResult() {
    new itemsSize = Menu_GetVoteItemsCount()
    new bestItem

    for (new i, bestResult; i < itemsSize; i++) {
        new nominee[_Vote_s]
        ArrayGetArray(g_vote, i, nominee, sizeof(nominee))

        new votes = nominee[VotesCount]
        if (votes > bestResult || (votes == bestResult) && random_num(0, 1) == 0) {
            bestResult = votes
            bestItem = i
        }
    }

    return bestItem
}


public CompareVoteItems(const Array: array, const item1[_Vote_s], item2[_Vote_s], const SortMethod: sortMethod[]) {
    switch (sortMethod[0]) {
        case Sort_Ascending:  return item1[VotesCount] - item2[VotesCount]
        case Sort_Descending: return item2[VotesCount] - item2[VotesCount]
    }

    return random_num(-1, 1)
}

public ApplyEffectsToPlayers(const player) {
    new players[MAX_PLAYERS], count
    get_players(players, count, "h")

    new Float: voteTimeRemaining = ModeVote_GetVoteTimeRemaining()

    for (new idx; idx < count; idx++) {
        new p = player ? player : players[idx]

        set_entvar(p, var_maxspeed, 1.0)
        set_member(p, m_bCanShoot, false)
        set_member(p, m_iHideHUD,
            (HIDEHUD_WEAPONS | HIDEHUD_HEALTH |
            HIDEHUD_MONEY | HIDEHUD_CROSSHAIR |
            HIDEHUD_OBSERVER_CROSSHAIR | HIDEHUD_FLASHLIGHT)
        )

        UTIL_RoundTime(p, floatround(voteTimeRemaining))
        UTIL_ScreenFade(p, .fxTime = 1.0, .holdTime = voteTimeRemaining, .color = {50, 70, 60, 255})

        if (player)
            break
    }
}

public CBasePlayer_ResetMaxSpeed_Post(const player) {
    if (!g_voteStarted)
        return

    set_entvar(player, var_maxspeed, 1.0)
}

stock Float: ModeVote_GetVoteTimeRemaining() {
    return Float: get_member_game(m_flRestartRoundTime) - get_gametime()
}

stock bool: ModeVote_IsVoteStarted() {
    return g_voteStarted
}

Vote_FillVoters() {
    if(g_playersInVote == Invalid_Array)
        g_playersInVote = ArrayCreate()

    ArrayClear(g_playersInVote)

    new players[MAX_PLAYERS], count
    get_players(players, count, "ch")

    for (new idx; idx < count; idx++) {
        new p = players[idx]

        ArrayPushCell(g_playersInVote, p)
    }
}

Vote_FillVote() {
    if (g_vote == Invalid_Array)
        g_vote = ArrayCreate(_: _Vote_s)

    ArrayClear(g_vote)

    // TODO: get menu items count
    new itemsCount = Menu_GetVoteItemsCount()
    for (new modeIdx; modeIdx < itemsCount; modeIdx++) {
        new nominee[_Vote_s]
        nominee[Item] = modeIdx

        ArrayPushArray(g_vote, nominee, sizeof(nominee))
    }

    return ArraySize(g_vote)
}

/* Vote_GetPlayersForVote() {
    new players[MAX_PLAYERS]

    new size = ArraySize(g_playersInVote)

    for (new idx; idx < size; idx++) {
        players[idx] = ArrayGetCell(g_playersInVote, idx)
    }

    return players
} */

Vote_GetPlayersCount() {
    return ArraySize(g_playersInVote)
}

bool: Vote_IsPlayerParticipatingInVote(const player) {
    return ArrayFindValue(g_playersInVote, player) != -1
}

bool: Vote_RemovePlayerFromVoters(const player) {
    if (g_playersInVote == Invalid_Array)
        return false
    
    new idx = ArrayFindValue(g_playersInVote, player)
    if (idx == -1)
        return false
    
    ArrayDeleteItem(g_playersInVote, idx)
    return true
}

bool: Vote_CanExtendCurrentMode() {
    return g_modeExtendedTimes < redm_mode_vote_extend_times
}
