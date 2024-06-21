#include <amxmodx>
#include <csdm>

public csdm_Init(const version[]) {
    server_print(" -- %.2f csdm_Init(`%s`)", get_gametime(), version)
}

public csdm_CfgInit() {
    server_print(" -- %.2f csdm_CfgInit()", get_gametime())
}

public csdm_PreDeath(killer, victim, headshot, const weapon[]) {
    server_print(" -- %.2f csdm_PreDeath(%i, %i, %i, `%s`)", get_gametime(),
        killer, victim, headshot, weapon
    )
}
public csdm_PostDeath(killer, victim, headshot, const weapon[]) {
    server_print(" -- %.2f csdm_PostDeath(%i, %i, %i, `%s`)", get_gametime(),
        killer, victim, headshot, weapon
    )
}

public csdm_PreSpawn(player, bool:fake) {
    server_print(" -- %.2f csdm_PreSpawn(%i, %i)", get_gametime(),
        player, fake
    )

    return PLUGIN_HANDLED
}
public csdm_PostSpawn(player, bool:fake) {
    server_print(" -- %.2f csdm_PostSpawn(%i, %i)", get_gametime(),
        player, fake
    )
}

public csdm_RoundRestart(post) {
    server_print(" -- %.2f csdm_RoundRestart(%i)", get_gametime(),
        post
    )
}

public csdm_StateChange(csdm_state) {
    server_print(" -- %.2f csdm_StateChange(%i)", get_gametime(),
        csdm_state
    )
}

public csdm_RemoveWeapon(owner, entity_id, boxed_id) {
    server_print(" -- %.2f csdm_RemoveWeapon(%i, %i, %i)", get_gametime(),
        owner, entity_id, boxed_id
    )
}

public csdm_HandleDrop(id, weapon, death) {
    server_print(" -- %.2f csdm_HandleDrop(%i, %i, %i)", get_gametime(),
        id, weapon, death
    )
}
