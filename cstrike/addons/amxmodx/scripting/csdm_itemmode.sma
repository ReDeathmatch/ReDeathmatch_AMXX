/**
 * csdm_itemmode.sma
 * Allows for Counter-Strike to be played as DeathMatch.
 *
 * CSDM Item Mode - Spawns different types of items all over the map.
 *
 * (C)2003-2006 Borja "FALUCO" Ferrer
 * (C)2003-2006 David "BAILOPAN" Anderson
 *
 *  Give credit where due.
 *  Share the source - it sets you free
 *  http://www.opensource.org/
 *  http://www.gnu.org/
 */

#include <amxmodx>
#include <fakemeta>
#include <cstrike>
#include <csdm>

#define MAX_ITEMS	250
#define MAX_ENTS 	1400
#define MAX_PACKS	50
#define ITEMTYPES_NUM	42
#define SLOTS		12

#define ITEM_LONGJUMP	31
#define ITEM_MEDKIT	32
#define ITEM_BATTERY	33
#define ITEM_PISTOLAMMO	34
#define ITEM_RIFLEAMMO	35
#define ITEM_SHOTAMMO	36
#define ITEM_SMGAMMO	37
#define ITEM_AWPAMMO	38
#define ITEM_PARAAMMO	39
#define ITEM_FULLAMMO	40
#define ITEM_ARMOR	41
#define ITEM_PACK	(MAX_ITEMS + 1)

#define CWRAP(%1,%2) (containi(%1,%2) != -1)

// Config variables
new bool:g_Enabled = true
new bool:g_EnabledCfg = true
new bool:g_droppacks = true
new g_battery = 15
new g_medkit = 15
new Float:g_itemTime = 20.0
new Float:g_packTime = 20.0

// Entity arrays
new g_EntModels[][] = 
{
	"", 
	"models/w_p228.mdl", 
	"", 
	"models/w_scout.mdl", 
	"models/w_hegrenade.mdl", 
	"models/w_xm1014.mdl", 
	"", 
	"models/w_mac10.mdl", 
	"models/w_aug.mdl", 
	"models/w_smokegrenade.mdl", 
	"models/w_elite.mdl", 
	"models/w_fiveseven.mdl", 
	"models/w_ump45.mdl", 
	"models/w_sg550.mdl", 
	"models/w_galil.mdl", 
	"models/w_famas.mdl", 
	"models/w_usp.mdl", 
	"models/w_glock18.mdl", 
	"models/w_awp.mdl", 
	"models/w_mp5.mdl", 
	"models/w_m249.mdl", 
	"models/w_m3.mdl", 
	"models/w_m4a1.mdl", 
	"models/w_tmp.mdl", 
	"models/w_g3sg1.mdl", 
	"models/w_flashbang.mdl", 
	"models/w_deagle.mdl", 
	"models/w_sg552.mdl", 
	"models/w_ak47.mdl", 
	"", 
	"models/w_p90.mdl", 
	"models/w_longjump.mdl", 
	"models/w_medkit.mdl", 
	"models/w_battery.mdl", 
	"models/w_357ammobox.mdl", 
	"models/w_9mmarclip.mdl", 
	"models/w_shotbox.mdl", 
	"models/w_9mmclip.mdl", 
	"models/w_crossbow_clip.mdl", 
	"models/w_chainammo.mdl", 
	"models/w_isotopebox.mdl", 
	"models/w_assault.mdl"
}
new g_EntClass[][] = 
{
	"", 
	"csdmw_p228", 
	"", 
	"csdmw_scout", 
	"csdmw_hegrenade", 
	"csdmw_xm1014", 
	"", 
	"csdmw_mac10", 
	"csdmw_aug", 
	"csdmw_smokegrenade", 
	"csdmw_elite", 
	"csdmw_fiveseven", 
	"csdmw_ump45", 
	"csdmw_sg550", 
	"csdmw_galil", 
	"csdmw_famas", 
	"csdmw_usp", 
	"csdmw_glock18", 
	"csdmw_awp", 
	"csdmw_mp5navy", 
	"csdmw_m249", 
	"csdmw_m3", 
	"csdmw_m4a1", 
	"csdmw_tmp", 
	"csdmw_g3sg1", 
	"csdmw_flashbang", 
	"csdmw_deagle", 
	"csdmw_sg552", 
	"csdmw_ak47", 
	"", 
	"csdmw_p90", 
	"csdm_longjump", 
	"csdm_medkit", 
	"csdm_battery", 
	"csdm_pistolammo", 
	"csdm_rifleammo", 
	"csdm_shotammo", 
	"csdm_smgammo", 
	"csdm_awpammo", 
	"csdm_paraammo", 
	"csdm_fullammo", 
	"csdm_armor"
}
stock g_Weap2Ammo[] =
{
	0,
	ITEM_PISTOLAMMO,//CSW_P228
	0,
	ITEM_RIFLEAMMO,	//CSW_SCOUT
	0,		//CSW_HEGRENADE
	ITEM_SHOTAMMO,	//CSW_XM1014
	0,		//CSW_C4
	ITEM_SMGAMMO,	//CSW_MAC10
	ITEM_RIFLEAMMO,	//CSW_AUG
	0,		//CSW_SMOKEGRENADE
	ITEM_PISTOLAMMO,//CSW_ELITE
	ITEM_PISTOLAMMO,//CSW_FIVESEVEN
	ITEM_SMGAMMO,	//CSW_UMP45
	ITEM_RIFLEAMMO,	//CSW_SG550
	ITEM_RIFLEAMMO,	//CSW_GALIL
	ITEM_RIFLEAMMO,	//CSW_FAMAS
	ITEM_PISTOLAMMO,//CSW_USP
	ITEM_PISTOLAMMO,//CSW_GLOCK18
	ITEM_AWPAMMO,	//CSW_AWP
	ITEM_SMGAMMO,	//CSW_MP5NAVY
	ITEM_PARAAMMO,	//CSW_M249
	ITEM_SHOTAMMO,	//CSW_M3
	ITEM_RIFLEAMMO,	//CSW_M4A1
	ITEM_SMGAMMO,	//CSW_TMP
	ITEM_RIFLEAMMO,	//CSW_G3SG1
	0,		//CSW_FLASHBANG
	ITEM_PISTOLAMMO,//CSW_DEAGLE
	ITEM_RIFLEAMMO,	//CSW_SG552
	ITEM_RIFLEAMMO,	//CSW_AK47
	0,		//CSW_KNIFE
	ITEM_SMGAMMO	//CSW_P90
}
new g_EntTable[MAX_ENTS]			// Contains the item_id (1-250) in the entid position(0-1500)
new g_EntType[MAX_ITEMS]			// Contains the item type (ammos, armor, ...) in the file order
new g_EntCount					// Global item count in the map (deathpacks not included)
new g_EntVecs[MAX_ITEMS][3]			// Contains the item position in the file order
new g_EntId[MAX_ITEMS]				// Contains the entid (0-1500) in the file order
new bool:HasLongJump[33] = false
new bool:IsRestricted[ITEMTYPES_NUM] = false	// Contains if an item is restricted or not
new bool:g_PackID[MAX_PACKS] = {true, ...}	// If true the packid can be used, else it's being use by another pack
new g_PackContents[MAX_PACKS][SLOTS]		// Contains the pack contents in the packid position(1-64)
						// [0]=size [1]=entid [2]=packid [3]=longjump [4...]=weapons
new g_MaxPlayers
new g_AllocStr

//Tampering with the author and name lines can violate the copyrights
new PLUGINNAME[] = "CSDM Item Mode"
new VERSION[] = CSDM_VERSION
new AUTHORS[] = "FALUCO"

public csdm_Init(const version[])
{
	if (version[0] == 0)
	{
		set_fail_state("CSDM failed to load.")
		return
	}
}

public csdm_CfgInit()
{
	g_AllocStr = engfunc(EngFunc_AllocString, "info_target")
	csdm_reg_cfg("items", "cfgmain")
	csdm_reg_cfg("item_restrictions", "cfgrestricts")
}

public plugin_init()
{
	register_plugin(PLUGINNAME, VERSION, AUTHORS)
	register_cvar("itemmode_version", CSDM_VERSION, FCVAR_SERVER|FCVAR_SPONLY)

	register_forward(FM_Touch, "hook_touch")

	g_MaxPlayers = get_maxplayers()
}

public plugin_precache()
{
	precache_model("models/w_medkit.mdl")
	precache_model("models/w_battery.mdl")
	precache_model("models/w_357ammobox.mdl")		//Pistol Ammo
	precache_model("models/w_shotbox.mdl")			//Shotgun Ammo
	precache_model("models/w_9mmclip.mdl")			//SMG Ammo
	precache_model("models/w_9mmarclip.mdl")		//Rifle Ammo
	precache_model("models/w_crossbow_clip.mdl")		//Awp Ammo
	precache_model("models/w_isotopebox.mdl")		//Full ammo
	precache_model("models/w_isotopeboxt.mdl")		//Full ammo
	precache_model("models/w_chainammo.mdl")		//Full ammo
	precache_model("models/w_weaponbox.mdl")		//Drop pack
	precache_model("models/w_assault.mdl")			//assaultsuit
	precache_model("models/w_longjump.mdl")			//longjump - thanks asskicr
	precache_model("models/w_longjumpt.mdl")		//"
	precache_sound("items/smallmedkit1.wav")
	precache_sound("items/gunpickup2.wav")
	precache_sound("items/suitchargeok1.wav")
	precache_sound("items/ammopickup2.wav")
	precache_sound("items/clipinsert1.wav")

	return PLUGIN_CONTINUE
}

public csdm_StateChange(csdm_state)
{
	if (g_Enabled && csdm_state == CSDM_DISABLE)
	{
		destroyAllItems()
		destroyAllPacks()
		g_Enabled = false
	}

	if (g_EnabledCfg && csdm_state == CSDM_ENABLE)
	{
		g_Enabled = true
		ReadFile()
		SetEnts()
	}
}

public cfgmain(readAction, line[], section[])
{
	if (!csdm_active())
		return
	
	if (readAction == CFG_READ)
	{
		new setting[24], sign[3], value[32]

		parse(line, setting, 23, sign, 2, value, 31)
		
		if (equali(setting, "enabled"))
		{
			g_Enabled = str_to_num(value) ? true : false
			g_EnabledCfg = g_Enabled
		}
		else if (equali(setting, "drop_packs"))
		{
			g_droppacks = str_to_num(value) ? true : false
		}
		else if (equali(setting, "battery"))
		{
			g_battery = str_to_num(value)
		}
		else if (equali(setting, "medkit"))
		{
			g_medkit = str_to_num(value)
		}
		else if (equali(setting, "item_time"))
		{
			g_itemTime = str_to_float(value)
		}
		else if (equali(setting, "item_time"))
		{
			g_itemTime = str_to_float(value)
			g_itemTime = (g_itemTime > 30.0) ? 30.0 : g_itemTime
		}
	}
}

public cfgrestricts(readAction, line[], section[])
{
	if (!csdm_active())
		return

	if (readAction == CFG_READ)
	{
		new itemname[24]
		parse(line, itemname, 23)
		
		if (equali(itemname, "longjump"))
		{
			IsRestricted[ITEM_LONGJUMP] = true
		}
		else if (equali(itemname, "medkit"))
		{
			IsRestricted[ITEM_MEDKIT] = true
		}
		else if (equali(itemname, "battery"))
		{
			IsRestricted[ITEM_BATTERY] = true
		}
		else if (equali(itemname, "pistolammo"))
		{
			IsRestricted[ITEM_PISTOLAMMO] = true
		}
		else if (equali(itemname, "rifleammo"))
		{
			IsRestricted[ITEM_RIFLEAMMO] = true
		}
		else if (equali(itemname, "shotammo"))
		{
			IsRestricted[ITEM_SHOTAMMO] = true
		}
		else if (equali(itemname, "smgammo"))
		{
			IsRestricted[ITEM_SMGAMMO] = true
		}
		else if (equali(itemname, "awpammo"))
		{
			IsRestricted[ITEM_AWPAMMO] = true
		}
		else if (equali(itemname, "paraammo"))
		{
			IsRestricted[ITEM_PARAAMMO] = true
		}
		else if (equali(itemname, "fullammo"))
		{
			IsRestricted[ITEM_FULLAMMO] = true
		}
		else if (equali(itemname, "armor"))
		{
			IsRestricted[ITEM_ARMOR] = true
		} else {
			new weapname[24], weaptype
			
			format(weapname, 23, "weapon_%s", itemname)
			weaptype = getWeapId(weapname)
			
			if (weaptype != 0)
				IsRestricted[weaptype] = true
			else
				log_amx("^"%s^" is not a valid name. Check your restrictions for item mode.", itemname)
		}
	}

	if (readAction == CFG_RELOAD)
	{
		// Reset all restrictions
		arrayset(IsRestricted, false, ITEMTYPES_NUM)
	}

	if (readAction == CFG_DONE)
	{
		destroyAllItems()
		destroyAllPacks()

		if (g_Enabled)
		{
			ReadFile()
			SetEnts()
		}
	}
}

ReadFile()
{
	new Map[32], config[32], File[64]

	get_mapname(Map, 31)
	get_localinfo("amxx_configsdir", config, 31)

	format(File, 63, "%s\csdm\items\ents_%s.cfg", config, Map)
	g_EntCount = 1

	if (file_exists(File))
	{
		new Data[51], len, EntName[25]
		new line = 0
		new pos[3][8]

		while ((g_EntCount < MAX_ITEMS) && ((line = read_file(File, line, Data, 50, len)) != 0))
		{
			if (strlen(Data) < 2)
				continue

			parse(Data, EntName, 24, pos[0], 7, pos[1], 7, pos[2], 7)

			g_EntVecs[g_EntCount][0] = str_to_num(pos[0])
			g_EntVecs[g_EntCount][1] = str_to_num(pos[1])
			g_EntVecs[g_EntCount][2] = str_to_num(pos[2])

			if (CWRAP(EntName, "item_longjump") && !(IsRestricted[ITEM_LONGJUMP]))
				g_EntType[g_EntCount] = ITEM_LONGJUMP
			else if (CWRAP(EntName, "item_healthkit") && !(IsRestricted[ITEM_MEDKIT]))
				g_EntType[g_EntCount] = ITEM_MEDKIT
			else if (CWRAP(EntName, "item_battery") && !(IsRestricted[ITEM_BATTERY]))
				g_EntType[g_EntCount] = ITEM_BATTERY
			else if (CWRAP(EntName, "pistol_ammo") && !(IsRestricted[ITEM_PISTOLAMMO]))
				g_EntType[g_EntCount] = ITEM_PISTOLAMMO
			else if (CWRAP(EntName, "rifle_ammo") && !(IsRestricted[ITEM_RIFLEAMMO]))
				g_EntType[g_EntCount] = ITEM_RIFLEAMMO
			else if (CWRAP(EntName, "shotgun_ammo") && !(IsRestricted[ITEM_SHOTAMMO]))
				g_EntType[g_EntCount] = ITEM_SHOTAMMO
			else if (CWRAP(EntName, "smg_ammo") && !(IsRestricted[ITEM_SMGAMMO]))
				g_EntType[g_EntCount] = ITEM_SMGAMMO
			else if (CWRAP(EntName, "full_ammo") && !(IsRestricted[ITEM_FULLAMMO]))
				g_EntType[g_EntCount] = ITEM_FULLAMMO
			else if (CWRAP(EntName, "armor") && !(IsRestricted[ITEM_ARMOR]))
				g_EntType[g_EntCount] = ITEM_ARMOR
			else if (CWRAP(EntName, "awp_ammo") && !(IsRestricted[ITEM_AWPAMMO]))
				g_EntType[g_EntCount] = ITEM_AWPAMMO
			else if (CWRAP(EntName, "para_ammo") && !(IsRestricted[ITEM_PARAAMMO]))
				g_EntType[g_EntCount] = ITEM_PARAAMMO
			else
			{
				new weaptype = getWeapId(EntName)

				if (weaptype != 0 && !(IsRestricted[weaptype]))
					g_EntType[g_EntCount] = weaptype
			}

			g_EntCount++
		}

		log_amx("Loaded %d items for map %s.", g_EntCount, Map)
	} else {
		log_amx("No items file found (%s)", File)
		g_Enabled = false	
	}
}

SetEnts()
{
	if (g_EntCount > 1)
		for (new i = 1; i < g_EntCount; i++)
			g_EntId[i] = MakeEnt(i)
}

MakeEnt(item_id)
{
	new entid = CreateEntId()
	if (!entid)
		return 0

	new Float:Vec[3]
	IVecFVec(g_EntVecs[item_id], Vec)
	new type = g_EntType[item_id]

	g_EntTable[entid] = item_id

	set_pev(entid, pev_classname, g_EntClass[type])
	engfunc(EngFunc_SetModel, entid, g_EntModels[type])
	set_pev(entid, pev_origin, Vec)
	set_pev(entid, pev_solid, 1)
	set_pev(entid, pev_movetype, 6)

	if (type <= 30)	// Is it a Weapon?
	{
		set_pev(entid, pev_angles, Float:{0.0, 0.0, 0.0})
		set_pev(entid, pev_velocity, Float:{0.0, 0.0, 0.0})
	}

	return entid
}

CreateEntId()
{
	return engfunc(EngFunc_CreateNamedEntity, g_AllocStr)
}

public hook_touch(ptr, ptd)
{
	if (!g_Enabled)
		return FMRES_HANDLED

	if (!g_EntTable[ptr])
		return FMRES_HANDLED

	if (ptd < 1 || ptd > g_MaxPlayers)
		return FMRES_HANDLED

	new item_id[1], item_type

	item_id[0] = g_EntTable[ptr]
	item_type = (g_EntTable[ptr] < MAX_ITEMS) ? g_EntType[g_EntTable[ptr]] : g_EntTable[ptr]

	// Death Pack
	if (item_type > ITEM_PACK)
	{
		new packid = item_type - ITEM_PACK
		new maxiter = g_PackContents[packid][0] + 4
		new weap, weapname[24]

		new value = get_user_armor(ptd)
		if (value + 15 < 100)
			set_pev(ptd, pev_armorvalue, float(value + 15))
		else
			set_pev(ptd, pev_armorvalue, float(100))


		value = get_user_health(ptd)
		if (value + 15 < 100)
			set_pev(ptd, pev_health, float(value + 15))
		else
			set_pev(ptd, pev_health, float(100))

		emit_sound(ptd, CHAN_ITEM, "items/ammopickup2.wav", 0.85, ATTN_NORM, 0, 150)

		for (new i = 4; i < maxiter; i++)
		{
			weap = g_PackContents[packid][i]

			if (weap == CSW_C4)
				continue

			if (CanGetWeapon(ptd, weap))
			{
				get_weaponname(weap, weapname, 23)
				csdm_give_item(ptd, weapname)
			}
			GiveAmmo(ptd, g_Weap2Ammo[weap])
		}

		if (g_PackContents[packid][3] == ITEM_LONGJUMP && !HasLongJump[ptd])
		{
			csdm_give_item(ptd, "item_longjump")
			HasLongJump[ptd] = true
			client_print(ptd, print_chat, "[CSDM] To use LongJump, press DUCK+JUMP while moving FORWARD!")
		}

		remove_task(packid)

		engfunc(EngFunc_RemoveEntity, ptr)
		g_EntTable[ptr] = 0
		g_PackID[packid] = true
		ZeroPack(packid)

		return FMRES_HANDLED
	}	

	// Ammo
	if ((item_type >= 34) && (item_type <= 39))
	{
		if (CanGetAmmo(ptd, item_type))
		{
			GiveAmmo(ptd, item_type)
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
		}
		return FMRES_HANDLED
	}

	// Weapon
	if ((item_type >= 1) && (item_type <= 30))
	{
		if (CanGetWeapon(ptd, item_type))
		{
			new weapname[24]

			get_weaponname(item_type, weapname, 23)
			csdm_give_item(ptd, weapname)
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
		}
		return FMRES_HANDLED
	}

	// Battery
	if (item_type == ITEM_BATTERY)
	{
		new armor = get_user_armor(ptd)
		if (armor < 100)
		{
			if (armor + g_battery < 100)
				set_pev(ptd, pev_armorvalue, float(armor + g_battery))
			else
				set_pev(ptd, pev_armorvalue, float(100))

			emit_sound(ptr, CHAN_ITEM, "items/gunpickup2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM)
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
		}
		return FMRES_HANDLED
	}

	// Medkit
	if (item_type == ITEM_MEDKIT)
	{
		new health = get_user_health(ptd)
		if (health < 100)
		{
			if (health + g_medkit < 100)
				set_pev(ptd, pev_health, float(health + g_medkit))
			else
				set_pev(ptd, pev_health, float(100))

			emit_sound(ptr, CHAN_ITEM, "items/smallmedkit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM)
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
		}
		return FMRES_HANDLED
	}

	// Armor
	if (item_type == ITEM_ARMOR)
	{
		if (get_user_armor(ptd) < 100)
		{
			csdm_give_item(ptd, "item_assaultsuit")
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
		}
		return FMRES_HANDLED
	}


	// Full Ammo
	if (item_type == ITEM_FULLAMMO)
	{
		if (CanGetAmmo(ptd, ITEM_PISTOLAMMO) || CanGetAmmo(ptd, ITEM_RIFLEAMMO) || CanGetAmmo(ptd, ITEM_SHOTAMMO) || CanGetAmmo(ptd, ITEM_SMGAMMO)
			|| CanGetAmmo(ptd, ITEM_AWPAMMO) || CanGetAmmo(ptd, ITEM_PARAAMMO))
		{
			GiveAmmo(ptd, ITEM_PISTOLAMMO)
			GiveAmmo(ptd, ITEM_RIFLEAMMO)
			GiveAmmo(ptd, ITEM_SHOTAMMO)
			GiveAmmo(ptd, ITEM_SMGAMMO)
			GiveAmmo(ptd, ITEM_AWPAMMO)
			GiveAmmo(ptd, ITEM_PARAAMMO)
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
		}
		return FMRES_HANDLED
	}

	// Longjump
	if (item_type == ITEM_LONGJUMP)
	{
		if (!HasLongJump[ptd])
		{
			csdm_give_item(ptd, "item_longjump")
			HasLongJump[ptd] = true
			emit_sound(ptr, CHAN_ITEM, "items/clipinsert1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM)
			engfunc(EngFunc_RemoveEntity, ptr)
			g_EntTable[ptr] = 0
			g_EntId[item_id[0]] = 0
			set_task(g_itemTime, "Replenish", 0, item_id, 1)
			client_print(ptd, print_chat, "[CSDM] To use LongJump, press DUCK+JUMP while moving FORWARD!")
		}
		return FMRES_HANDLED
	}

	return FMRES_HANDLED
}

public Replenish(item_id[])
{
	g_EntId[item_id[0]] = MakeEnt(item_id[0])
	emit_sound(g_EntId[item_id[0]], CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM, 0, 150)
}

public csdm_PostDeath(killer, victim, headshot, const weapon[])
{
	if (!g_Enabled)
		return

	if (HasLongJump[victim])
		HasLongJump[victim] = false
}

public csdm_RoundRestart()
{
	if (!g_Enabled)
		return

	new players[32], num, player

	get_players(players, num)

	for (new i = 0; i < num; i++)
	{
		player = players[i]
		
		if (HasLongJump[player] && is_user_alive(player))
			csdm_give_item(player, "item_longjump")
	}

	destroyAllPacks()
	destroyAllItems(false)
	SetEnts()
}

public csdm_PreDeath(killer, victim, headshot, const weapon[])
{
	if (!g_Enabled || !g_droppacks)
		return

	new packid = GetPackId()
	if (packid < 0)
		return

	new entid = CreateEntId()
	if (!entid) 
	{
		g_PackID[packid] = true
		return
	}

	new Float:Pos[3], Orig[3]

	get_user_origin(victim, Orig)
	IVecFVec(Orig, Pos)
	g_EntTable[entid] = ITEM_PACK + packid

	set_pev(entid, pev_classname, "csdm_deathpack")
	engfunc(EngFunc_SetModel, entid, "models/w_weaponbox.mdl")
	set_pev(entid, pev_origin, Pos)
	set_pev(entid, pev_solid, 1)
	set_pev(entid, pev_movetype, 6)
	set_pev(entid, pev_owner, victim)

	new Weapons[32], weapnum = 0
	get_user_weapons(victim, Weapons, weapnum)

	for (new i = 0; i < weapnum; i++)
	{
		g_PackContents[packid][i + 4] = Weapons[i]
	}

	if (HasLongJump[victim])
	{
		g_PackContents[packid][3] = ITEM_LONGJUMP
	}

	g_PackContents[packid][0] = weapnum
	g_PackContents[packid][1] = entid
	g_PackContents[packid][2] = packid

	new info[2]
	info[0] = entid
	info[1] = packid
	set_task(g_packTime, "DeletePack", packid, info, 2)
}

public DeletePack(pack_info[])
{
	new packid = pack_info[1]
	new entid = pack_info[0]

	engfunc(EngFunc_RemoveEntity, entid)
	g_PackID[packid] = true
	g_EntTable[entid] = 0
	ZeroPack(packid)
}

GetPackId()
{
	for (new i = 1; i < MAX_PACKS; i++)
	{
		if (g_PackID[i])
		{
			g_PackID[i] = false
			return i
		}
	}

	return -1
}

ZeroPack(packid)
{
	arrayset(g_PackContents[packid], 0, SLOTS)
}

public client_connect(id)
{
	if (!g_Enabled)
		return

	HasLongJump[id] = false
}

public csdm_HandleDrop(id, weapon, death)
{
	return (g_Enabled && g_droppacks) ? CSDM_DROP_REMOVE : CSDM_DROP_CONTINUE
}

bool:CanGetAmmo(id, ammotype)
{
	switch (ammotype)
	{
		case ITEM_PISTOLAMMO:
		{
			if (cs_get_user_bpammo(id, CSW_DEAGLE) < g_MaxBPAmmo[CSW_DEAGLE])
				return true
			if (cs_get_user_bpammo(id, CSW_P228) < g_MaxBPAmmo[CSW_P228])
				return true
			if (cs_get_user_bpammo(id, CSW_USP) < g_MaxBPAmmo[CSW_USP])
				return true
			if (cs_get_user_bpammo(id, CSW_GLOCK18) < g_MaxBPAmmo[CSW_GLOCK18])
				return true
			if (cs_get_user_bpammo(id, CSW_FIVESEVEN) < g_MaxBPAmmo[CSW_FIVESEVEN])
				return true
		}
		case ITEM_SHOTAMMO:
		{
			if (cs_get_user_bpammo(id, CSW_XM1014) < g_MaxBPAmmo[CSW_XM1014])
				return true
		}
		case ITEM_RIFLEAMMO:
		{
			if (cs_get_user_bpammo(id, CSW_M4A1) < g_MaxBPAmmo[CSW_M4A1])
				return true
			if (cs_get_user_bpammo(id, CSW_AK47) < g_MaxBPAmmo[CSW_AK47])
				return true
		}
		case ITEM_SMGAMMO:
		{
			if (cs_get_user_bpammo(id, CSW_MP5NAVY) < g_MaxBPAmmo[CSW_MP5NAVY])
				return true
			if (cs_get_user_bpammo(id, CSW_MAC10) < g_MaxBPAmmo[CSW_MAC10])
				return true
		}
		case ITEM_AWPAMMO:
		{
			if (cs_get_user_bpammo(id, CSW_AWP) < g_MaxBPAmmo[CSW_AWP])
				return true
		}
		case ITEM_PARAAMMO:
		{
			if (cs_get_user_bpammo(id, CSW_M249) < g_MaxBPAmmo[CSW_M249])
				return true
		}
	}
	
	return false
}

GiveAmmo(id, ammotype)
{
	switch (ammotype)
	{
		case ITEM_PISTOLAMMO:
		{
			csdm_give_item(id, "ammo_357sig")
			csdm_give_item(id, "ammo_57mm")
			csdm_give_item(id, "ammo_45acp")
			csdm_give_item(id, "ammo_50ae")
			csdm_give_item(id, "ammo_9mm")
		}
		case ITEM_SHOTAMMO:
		{
			csdm_give_item(id, "ammo_buckshot")
		}
		case ITEM_RIFLEAMMO:
		{
			csdm_give_item(id, "ammo_762nato")
			csdm_give_item(id, "ammo_556nato")
		}
		case ITEM_SMGAMMO:
		{
			csdm_give_item(id, "ammo_9mm")
			csdm_give_item(id, "ammo_45acp")
		}
		case ITEM_AWPAMMO:
		{
			csdm_give_item(id, "ammo_338magnum")
		}
		case ITEM_PARAAMMO:
		{
			csdm_give_item(id, "ammo_556natobox")
		}
	}
}

bool:CanGetWeapon(id, wid)
{
	new Weapons[MAX_WEAPONS], num, slot = g_WeaponSlots[wid]

	get_user_weapons(id, Weapons, num)

	for (new i = 0; i < num; i++)
	{
		if (g_WeaponSlots[Weapons[i]] == slot && (slot != 4))
			return false
	}

	// check if it's a grenade (we can have 2 flashbangs)
	if ((slot == 4) && (cs_get_user_bpammo(id, wid) >= g_MaxBPAmmo[wid]))
		return false

	return true
}

destroyAllItems(bool:mapchange = true)
{
	new iter, entid

	// abort any respawn task to avoid double respawns on plugin reload
	if (task_exists())
		remove_task()

	// destroy all items
	for (iter = 0; iter < MAX_ITEMS; iter++)
	{
		entid = g_EntId[iter]
		if (pev_valid(entid))
			engfunc(EngFunc_RemoveEntity, entid)
	}

	arrayset(g_EntId, 0, MAX_ITEMS)
	arrayset(g_EntTable, 0, MAX_ENTS)
	if (mapchange)
		arrayset(g_EntType, 0, MAX_ITEMS)
}

destroyAllPacks()
{
	new packid, entid	

	for (new i = 1; i < MAX_PACKS; i++)
	{
		if (g_PackContents[i][0])
		{
			entid = g_PackContents[i][1]
			packid = g_PackContents[i][2]

			if (pev_valid(entid))
				engfunc(EngFunc_RemoveEntity, entid)

			remove_task(packid)

			ZeroPack(packid)
		}
	}

	arrayset(g_PackID, true, MAX_PACKS)
}

stock getWeapId(wp[])
{
	if (equali(wp, "weapon_p228")) {
		return CSW_P228
	} else if (equali(wp, "weapon_scout")) {
		return CSW_SCOUT
	} else if (equali(wp, "weapon_hegrenade")) {
		return CSW_HEGRENADE
	} else if (equali(wp, "weapon_xm1014")) {
		return CSW_XM1014
	} else if (equali(wp, "weapon_c4")) {
		return CSW_C4
	} else if (equali(wp, "weapon_mac10")) {
		return CSW_MAC10
	} else if (equali(wp, "weapon_aug")) {
		return CSW_AUG
	} else if (equali(wp, "weapon_smokegrenade")) {
		return CSW_SMOKEGRENADE
	} else if (equali(wp, "weapon_elite")) {
		return CSW_ELITE
	} else if (equali(wp, "weapon_fiveseven")) {
		return CSW_FIVESEVEN
	} else if (equali(wp, "weapon_ump45")) {
		return CSW_UMP45
	} else if (equali(wp, "weapon_sg550")) {
		return CSW_SG550
	} else if (equali(wp, "weapon_galil")) {
		return CSW_GALIL
	} else if (equali(wp, "weapon_famas")) {
		return CSW_FAMAS
	} else if (equali(wp, "weapon_usp")) {
		return CSW_USP
	} else if (equali(wp, "weapon_glock18")) {
		return CSW_GLOCK18
	} else if (equali(wp, "weapon_awp")) {
		return CSW_AWP
	} else if (equali(wp, "weapon_mp5navy")) {
		return CSW_MP5NAVY
	} else if (equali(wp, "weapon_m249")) {
		return CSW_M249
	} else if (equali(wp, "weapon_m3")) {
		return CSW_M3
	} else if (equali(wp, "weapon_m4a1")) {
		return CSW_M4A1
	} else if (equali(wp, "weapon_tmp")) {
		return CSW_TMP
	} else if (equali(wp, "weapon_g3sg1")) {
		return CSW_G3SG1
	} else if (equali(wp, "weapon_flashbang")) {
		return CSW_FLASHBANG
	} else if (equali(wp, "weapon_deagle")) {
		return CSW_DEAGLE
	} else if (equali(wp, "weapon_sg552")) {
		return CSW_SG552
	} else if (equali(wp, "weapon_ak47")) {
		return CSW_AK47
	} else if (equali(wp, "weapon_knife")) {
		return CSW_KNIFE
	} else if (equali(wp, "weapon_p90")) {
		return CSW_P90
	}
	
	return 0
}
