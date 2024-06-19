#include <amxmodx>
#include <csdm>

public csdm_Init(const version[]) {
    server_print("csdm_Init(%s)", version)
}

public csdm_CfgInit() {
    server_print("csdm_CfgInit()")
}

public csdm_PreDeath(killer, victim, headshot, const weapon[]) {
    server_print("csdm_PreDeath(%i, %i, %i, `%s`)",
        killer, victim, headshot, weapon
    )
}
public csdm_PostDeath(killer, victim, headshot, const weapon[]) {
    server_print("csdm_PreDeath(%i, %i, %i, `%s`)",
        killer, victim, headshot, weapon
    )
}

public csdm_PreSpawn(player, bool:fake) {
    server_print("csdm_PreSpawn(%i, %i)",
        player, fake
    )
}
public csdm_PostSpawn(player, bool:fake) {
    server_print("csdm_PostSpawn(%i, %i)",
        player, fake
    )
}

public csdm_RoundRestart(post) {
    server_print("csdm_RoundRestart(%i)",
        post
    )
}

public csdm_StateChange(csdm_state) {

}

public csdm_RemoveWeapon(owner, entity_id, boxed_id) {

}

public csdm_HandleDrop(id, weapon, death) {
    
}