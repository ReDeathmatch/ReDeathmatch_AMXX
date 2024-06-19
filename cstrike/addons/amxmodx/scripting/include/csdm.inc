/**
 * (C)2003-2006 David "BAILOPAN" Anderson
 * Counter-Strike Deathmatch (CSDM) 2.10 Module Includes
 */

#if defined _csdm_included
  #endinput
#endif
#define _csdm_included

#if AMXX_VERSION_NUM >= 175
 #pragma reqlib csdm
 #if !defined AMXMODX_NOAUTOLOAD
  #pragma loadlib csdm
 #endif
#else
 #pragma library csdm
#endif

#define	CSDM_VERSION	"2.1.1"

#define	CFG_READ		0		//Line is being read normally
#define	CFG_RELOAD		1		//Section is being parsed from the start
#define	CFG_DONE		2		//Section is done being parsed

#define DEFAULT_ARMOR		100

#define MAX_WEAPONS 		32 
#define MAX_SECONDARY 		8
#define MAX_PRIMARY 		18

#define CSDM_FFA_ENABLE		3
#define CSDM_FFA_DISABLE	2
#define CSDM_ENABLE			1
#define CSDM_DISABLE		0

#define CSDM_DROP_CONTINUE	0	//continue normally
#define CSDM_DROP_REMOVE	1	//remove the dropped weapon
#define CSDM_DROP_IGNORE	2	//ignore entirely

/************
 * FORWARDS *
 ************/
 
//Called when CSDM initializes.  It passes in a version string, or a blank
// string if some load failure occurred.
//Do pre-config stuff here.
forward csdm_Init(const version[]);

//Called when CSDM is ready to accept config hooks.
forward csdm_CfgInit();

//Called right before a death message is sent.  Return value has no effect.
forward csdm_PreDeath(killer, victim, headshot, const weapon[]);

//Called right after a death message is handled.  
//return PLUGIN_HANDLED will block CSDM from doing any physical respawning.
forward csdm_PostDeath(killer, victim, headshot, const weapon[]);

//Called after a player is physically respawned,
// but before the spawn handler is called.  Returning PLUGIN_HANDLED
// will block CSDM from calling the set spawn handler.
//Note! this will only be called if CSDM internally respawns or you use
// csdm_respawn().
forward csdm_PreSpawn(player, bool:fake);

//Called after a player is physically respawned, 
// and after all spawn handling is completed.
forward csdm_PostSpawn(player, bool:fake);

//Called right before round is restarted
//The post parameter is set to 1 if the event is post.
//If post is 0, the event is pre, and PLUGIN_HANDLED will block
// CSDM from doing round restart actions (respawning)
forward csdm_RoundRestart(post);

//Called when a CSDM state is set
forward csdm_StateChange(csdm_state);

/** 
 * Called when CSDM is about to remove a weapon.  Return PLUGIN_HANDLED to block.
 *
 * owner - The player who owned this weapon.
 * entity_id - The entity index of the weapon.
 * box_id - If the weapon is boxed, this will be set to the boxed entity index..
 */
forward csdm_RemoveWeapon(owner, entity_id, boxed_id);

//Called by the main plugin right now
//If death is true, the weapon was dropped on death
//if weapon is -1, the weapon is the shield.
forward csdm_HandleDrop(id, weapon, death);

/***********
 * NATIVES *
 ***********/
 
//To retrieve/set angles and v_angles, pass up to two more vectors
native csdm_getpos(player, Float:origin[3], ...);

native csdm_setpos(player, const Float:origin[3], ...);

//Returns the number of registered spawn styles
native csdm_spawnstyles();

//Retrieves the name of a spawn style by index (indices start at 0)
native csdm_styleinfo(style_index, name[], maxlength);

//Adds a spawn style handler
native csdm_addstyle(const name[], const function[]);

//Sets the current spawn style handler by name.
//The handler registered to this name will be called after every spawn.
native csdm_setstyle(const name[]);

//Returns the current style id
native csdm_curstyle();

//Respawns a player correctly under the CSDM spawn system.
native csdm_respawn(player);

//Forces a "fake" respawn on the player - sort of like a ResetHUD
native csdm_fakespawn(player);

//Registers a hook on the config reader
//The forward looks like this:
//forward my_cfg_reader(read_mode, const line[], const section[])
native csdm_reg_cfg(const sectionName[], const handler[]);

//Gives an item.  Same as give_item from fun.
native csdm_give_item(player, const item_name[]);

//Forces a player to drop a weapon
native csdm_force_drop(player, const weapon[], remove=1);

//Schedules a weapon for removal.  0 seconds means immediate
//delay specifies whether it should wait 0.1 seconds or not (0 for immediate lookup)
native csdm_remove_weapon(player, const weapon[], seconds, delay);

//Reloads the config file
//If you specify a named file, CSDM auto-prepends the configs dir.
// I.e., passing csdm\mapconfigs\de_dust2.cfg will turn into:
// addons\amxmodx\configs\csdm\mapconfigs\de_dust2.cfg
native csdm_reload_cfg(const file[]="");

//Returns whether CSDM is active
native csdm_active();
//Sets CSDM to active (do not set the cvar!)
native csdm_set_active(on_or_off);

//Returns/sets the current FFA (free for all) state
native csdm_get_ffa();
native csdm_set_ffa(on_or_off);

//Gets/sets the respawn wait time
native Float:csdm_get_spawnwait();
native csdm_set_spawnwait(Float:time);

//returns cache information into each array slot
// 0 - number of free items in spawn task cache
// 1 - number of free task in general task cache
// 2 - number of hot tasks (ones in use)
// 3 - number of cached weapon removals in table
// 4 - number of live weapon removals in table
// 5 - number of weapon find tasks in cache
// in general, 0+5+4 should about equal 1+2
// do not edit the vers parameter.
native csdm_cache(ar[6], vers=2)

//Quick hack to replace engine dependency and work around a FM bug
//returns 1 if a hull collision is found
native csdm_trace_hull(Float:origin[3], hull_no);

//Disables the CSDM welcome message
//This is useful if you only use CSDM as an engine
//Returns the old value.  Pass -1 to do nothing.
native csdm_set_intromsg(value=-1);

/**
 * Natives defined by csdm_main 
 */
 
//Returns the handle for the main CSDM menu, letting you modify it.
native csdm_main_menu();

#define CSDM_OPTION_SAYRESPAWN	1		//No extra parameters
#define CSDM_SET_DISABLED		-1
#define CSDM_SET_DEFAULT		0
#define CSDM_SET_ENABLED		1

//Sets an internal option.  Options listed above.
native csdm_set_mainoption(option, setting, ...);

//Runs through the weapon drop deletion algorithm.
//If weapon_id is -1, name should be "weapon_shield"
native csdm_fwd_drop(player, weapon_id, const name[]);

/*** ************ ***
 *** HELPER STUFF 
 ***/
 
#define	SLOT_PRIMARY	1
#define	SLOT_SECONDARY	2
#define	SLOT_KNIFE		3
#define	SLOT_GRENADE	4
#define	SLOT_C4			5

#define	_TEAM_T			1
#define	_TEAM_CT		2

//Weapon slot lookup table
stock g_WeaponSlots[] = {
		0,
		2,	//CSW_P228
		0,
		1,	//CSW_SCOUT
		4,	//CSW_HEGRENADE
		1,	//CSW_XM1014
		5,	//CSW_C4
		1,	//CSW_MAC10
		1,	//CSW_AUG
		4,	//CSW_SMOKEGRENADE
		2,	//CSW_ELITE
		2,	//CSW_FIVESEVEN
		1,	//CSW_UMP45
		1,	//CSW_SG550
		1,	//CSW_GALIL
		1,	//CSW_FAMAS
		2,	//CSW_USP
		2,	//CSW_GLOCK18
		1,	//CSW_AWP
		1,	//CSW_MP5NAVY
		1,	//CSW_M249
		1,	//CSW_M3
		1,	//CSW_M4A1
		1,	//CSW_TMP
		1,	//CSW_G3SG1
		4,	//CSW_FLASHBANG
		2,	//CSW_DEAGLE
		1,	//CSW_SG552
		1,	//CSW_AK47
		3,	//CSW_KNIFE
		1	//CSW_P90
	};
	
//Maximum backpack ammo lookup table
stock g_MaxBPAmmo[] = {
		0,
		52,	//CSW_P228
		0,
		90,	//CSW_SCOUT
		1,	//CSW_HEGRENADE
		32,	//CSW_XM1014
		1,	//CSW_C4
		100,//CSW_MAC10
		90,	//CSW_AUG
		1,	//CSW_SMOKEGRENADE
		120,//CSW_ELITE
		100,//CSW_FIVESEVEN
		100,//CSW_UMP45
		90,	//CSW_SG550
		90,	//CSW_GALIL
		90,	//CSW_FAMAS
		100,//CSW_USP
		120,//CSW_GLOCK18
		30,	//CSW_AWP
		120,//CSW_MP5NAVY
		200,//CSW_M249
		32,	//CSW_M3
		90,	//CSW_M4A1
		120,//CSW_TMP
		90,	//CSW_G3SG1
		2,	//CSW_FLASHBANG
		35,	//CSW_DEAGLE
		90,	//CSW_SG552
		90,	//CSW_AK47
		0,	//CSW_KNIFE
		100//CSW_P90
	};
	
#define getWepId(%1) get_weaponid(%1)

stock bool:IsValidTeam(team)
{
	if (team == 1 || team == 2)
		return true
	return false
}

stock drop_with_shield(player, weapon_id)
{
	new name[32]
	get_weaponname(weapon_id, name, 31)
	csdm_force_drop(player, "weapon_shield", 0)
	csdm_force_drop(player, name, 0)
	csdm_fwd_drop(player, -1, "weapon_shield")
	csdm_fwd_drop(player, weapon_id, name)
	csdm_give_item(player, "weapon_shield")
}
