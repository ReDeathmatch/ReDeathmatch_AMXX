static const Float: BARRIER_REDRAW_TIME = 5.0

static g_spriteLaserbeam
static g_spriteBubble

static bool: redm_aim_barrier

enum BarrierState_s {
    BARRIER_DISABLED,
    BARRIER_ALWAYS_VISIBLE,
    BARRIER_VISIBLE_ON_TOUCH,
}

static BarrierState_s: redm_aim_barrier_always_show

AimBarriers_Precache() {
    g_spriteLaserbeam = precache_model("sprites/laserbeam.spr")
    g_spriteBubble = precache_model("sprites/bubble.spr")
}

AimBarriers_Init() {
    bind_pcvar_num(
        create_cvar(
            "redm_aim_barrier", "0",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 1.0,
            .flags = _FCVAR_BOOLEAN,
            .description = "Enable a barrier in the middle of the map (`anti-rush' system).^n\
                It does not work on `de_`, `cs_` `css_` maps."
        ),
        redm_aim_barrier
    )
    bind_pcvar_num(
        create_cvar(
            "redm_aim_barrier_always_show", "2",
            .has_min = true, .min_val = 0.0,
            .has_max = true, .max_val = 2.0,
            .flags = _FCVAR_INTEGER,
            .description = "Barrier display mode. ^n\
                ^t`0` - disable, ^n\
                ^t`1` - display permanently (beta), ^n\
                ^t`2` - display only when touched."
        ),
        redm_aim_barrier_always_show
    )

    new const prefixBlocklist[][] = { "de_", "cs_", "css_" }
    if (!IsMapCompatible(prefixBlocklist))
        return

    CreateBarrier()

    RegisterHam(Ham_Touch, "trigger_push", "CTriggerPush_Touch", .Post = false)
}

static CreateBarrier() {
    new Float: middle[3], longerAxis
    CWorld_GetCenter(middle, longerAxis)

    new Float: mins[3] = { -10.0, -10.0, -8192.0 }
    new Float: maxs[3] = { 10.0, 10.0, 8192.0 }

    mins[!longerAxis] *= 500
    maxs[!longerAxis] *= 500

    new ent = CBarrier_Spawn(middle, mins, maxs, 2.0)
    set_task_ex(BARRIER_REDRAW_TIME, "BarrierRedraw", ent, .flags = SetTask_Repeat)

    return ent
}

static bool: GetAvgOrigin(const classname[], Float: out[3], bool: simple = false) {
    new count

    new ent = MaxClients
    while ((ent = fm_find_ent_by_class(ent, classname))) {
        new Float: origin[3]
        pev(ent, pev_origin, origin)

        xs_vec_add(out, origin, out)
        ++count

        if (simple)
            break
    }

    if (!count)
        return false

    for (new i; i < sizeof out; i++) {
        out[i] /= float(count)
    }

    return true
}

static CWorld_GetCenter(Float: out[3], &longerAxis) {
    new const startEntClassname[] = "info_player_deathmatch"
    new const endEntClassname[] = "info_player_start"

    new Float: start[3], Float: end[3]
    GetAvgOrigin(startEntClassname, start)
    GetAvgOrigin(endEntClassname, end)

    _xs_vec_lerp(start, end, out, 0.5)

    longerAxis = floatabs(out[XS_X] - start[XS_X]) < floatabs(out[XS_Y] - start[XS_Y])

    // engfunc(EngFunc_SetOrigin, 0, out) // TODO: hacky ?!
}

static CBarrier_Spawn(const Float: origin[3], const Float: mins[3], const Float: maxs[3], const Float: pushForce = 2.0) {
    new ent = fm_create_entity("trigger_push")
    set_pev(ent, pev_origin, origin)
    set_pev(ent, pev_speed, -pushForce)

    dllfunc(DLLFunc_Spawn, ent)
    engfunc(EngFunc_SetSize, ent, mins, maxs)

    return ent
}

public CTriggerPush_Touch(const entity, const other) {
    if (!IsActive())
        return HAM_SUPERCEDE

    if (pev_valid(other) != 2)
        return HAM_IGNORED

    return CBarrier_Touch(entity, other)
}

static CBarrier_Touch(const entity, const other) {
    if (!redm_aim_barrier)
        return HAM_SUPERCEDE

    static Float: velocity[3]
    pev(other, pev_velocity, velocity)

    set_pev(entity, pev_movedir, velocity)

    if (redm_aim_barrier_always_show == BARRIER_VISIBLE_ON_TOUCH) {
        new Float: gameTime = get_gametime()

        static Float: nextEffectsTime
        if (nextEffectsTime > gameTime)
            return HAM_IGNORED
        
        const Float: drawDelay = 0.5
        nextEffectsTime = gameTime + drawDelay

        CBarrier_Draw(entity, other, drawDelay)
    }

    return HAM_IGNORED
}

static CBarrier_Draw(const entity, const toucher, const Float: drawDelay) {
    static Float: origin[3]
    pev(entity, pev_origin, origin)

    static Float: tourcherOrigin[3]
    tourcherOrigin = origin
    if (toucher) {
        pev(toucher, pev_absmin, tourcherOrigin)
        tourcherOrigin[XS_Z] += 10
    }

    static Float: mins[3]
    pev(entity, pev_mins, mins)

    static Float: maxs[3]
    pev(entity, pev_maxs, maxs)

    static Float: start[3]
    xs_vec_copy(origin, start)

    static Float: end[3]
    xs_vec_copy(origin, end)
    
    new longerAxis = floatabs(origin[XS_X] - mins[XS_X]) > floatabs(origin[XS_Y] - mins[XS_Y])

    start[!longerAxis] -= mins[!longerAxis]
    end[!longerAxis] -= maxs[!longerAxis]
    start[XS_Z] = end[XS_Z] = ((tourcherOrigin[XS_Z]) + (mins[XS_Z] * 2))

    new s[3], e[3]
    FVecIVec(start, s)
    FVecIVec(end, e)

    const Float: overlap = 0.0 // -0.8
    new lifeTime = floatround((drawDelay + overlap) * 10, floatround_ceil)
    te_create_beam_between_points(
        s, e,
        g_spriteLaserbeam, 
        0,
        30,
        lifeTime,
        255,
        .r = 100, .g = 0, .b = 0,
        .speed = 3,
        .noise = 0
    )

    te_create_bubble_line(s, e, g_spriteBubble, 255, .height = 1000, .randomness = true)
}

public BarrierRedraw(const entity) {
    if (!IsActive())
        return

    if (redm_aim_barrier_always_show != BARRIER_ALWAYS_VISIBLE)
        return

    CBarrier_Draw(entity, 0, BARRIER_REDRAW_TIME)
}
