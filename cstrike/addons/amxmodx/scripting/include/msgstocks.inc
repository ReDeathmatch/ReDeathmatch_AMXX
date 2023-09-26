// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// Message Stocks
//

#if defined _msgnatives_included
	#endinput
#endif

#if defined _msgstocks_included
	#endinput
#endif

#define _msgstocks_included

#define MSGSTOCKS_VERSION 1.1

/**
 * @section Normal message stocks
 */

/**
 * Temporarily draws HUD numerical ammo amount and corresponding ammo
 * HUD icon in the middle of the right side of the screen.
 *
 * @note Draw time depends on the hud_drawhistory_time client cvar value.
 *
 * @param id            Client index or 0 for all clients
 * @param ammoid        Ammunition id
 * @param amount        Ammunition amount
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock draw_ammo_pickup_icon(id, ammoid, amount, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_AmmoPickup;

	if(!MSG_AmmoPickup)
		MSG_AmmoPickup = get_user_msgid("AmmoPickup");

	message_begin(get_msg_destination(id, reliable), MSG_AmmoPickup, .player = id);
	write_byte(ammoid);
	write_byte(amount);
	message_end();

	return 1;
}

/**
 * Temporarily draws a corresponding item HUD icon in the middle of the
 * right side of the screen.
 *
 * @note This stock isn't available in Day of Defeat.
 * @note Draw time depends on the hud_drawhistory_time client cvar value.
 * @note A list of all icons and screenshots of them can be found here:
 *       http://doktor.haze.free.fr/counter-strike/sprites_screens/index.php
 *
 * @param id            Client index or 0 for all clients
 * @param itemname      Item name
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock draw_item_pickup_icon(id, itemname[], bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_ItemPickup;

	if(!MSG_ItemPickup)
		MSG_ItemPickup = get_user_msgid("ItemPickup");

	message_begin(get_msg_destination(id, reliable), MSG_ItemPickup, .player = id);
	write_string(itemname);
	message_end();

	return 1;
}

/**
 * Temporarily draws the corresponding weapon HUD icon in the middle of the
 * right side of the screen.
 *
 * @note Draw time depends on the hud_drawhistory_time client cvar value.
 *
 * @param id            Client index or 0 for all clients
 * @param weaponid      Weapon id
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock draw_weapon_pickup_icon(id, weaponid, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_WeapPickup;

	if(!MSG_WeapPickup)
		MSG_WeapPickup = get_user_msgid("WeapPickup");

	message_begin(get_msg_destination(id, reliable), MSG_WeapPickup, .player = id);
	write_byte(weaponid);
	message_end();

	return 1;
}

/**
 * Flags used in draw_status_icon()
 */
enum StatusIconFlags
{
	StatusIcon_Hide = 0,
	StatusIcon_Show,
	StatusIcon_Flash
}

/**
 * Draws a specified status HUD icon.
 *
 * @note This stock is available only in the following games:
 *         Counter-Strike
 *         Counter-Strike: Condition Zero
 *         Half-Life: Opposing Force
 *         Team Fortress Classic
 * @note A list of all icons and screenshots of them can be found here:
 *       http://doktor.haze.free.fr/counter-strike/sprites_screens/index.php
 *
 * @param id            Client index or 0 for all clients
 * @param sprite        Sprite name (valid names are listed in sprites/hud.txt)
 * @param status        Valid status values:
 *                        StatusIcon_Hide         - hides the status icon
 *                        StatusIcon_Show         - shows the status icon
 *                        StatusIcon_Flash        - flashes the status icon
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock draw_status_icon(id, sprite[] = "", StatusIconFlags:status = StatusIcon_Hide, r = 0, g = 0, b = 0, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_StatusIcon;

	if(!MSG_StatusIcon)
		MSG_StatusIcon = get_user_msgid("StatusIcon");

	message_begin(get_msg_destination(id, reliable), MSG_StatusIcon, .player = id);
	write_byte(_:status);

	if(status)
	{
		write_string(sprite);
		write_byte(r);
		write_byte(g);
		write_byte(b);
	}

	message_end();
	return 1;
}

/**
 * Train controls used in draw_train_controls()
 */
enum TrainControlFlags
{
	TrainControls_None = 0,
	TrainControls_Neutral,
	TrainControls_Slow,
	TrainControls_Medium,
	TrainControls_Maximum,
	TrainControls_Reverse
}

/**
 * Displays the speed bar used for controlling a train.
 *
 * @note This stock isn't available in Day of Defeat.
 *
 * @param id            Client index or 0 for all clients
 * @param speed         Train speed:
 *                        TrainControls_None
 *                        TrainControls_Neutral
 *                        TrainControls_Slow
 *                        TrainControls_Medium
 *                        TrainControls_Maximum
 *                        TrainControls_Reverse
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock draw_train_controls(id, TrainControlFlags:speed = TrainControls_None, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_Train;

	if(!MSG_Train)
		MSG_Train = get_user_msgid("Train");

	message_begin(get_msg_destination(id, reliable), MSG_Train, .player = id);
	write_byte(_:speed);
	message_end();

	return 1;
}

/**
 * Sends the geiger signal that notifies the player of nearby radiation level.
 *
 * @note This stock isn't available in Day of Defeat.
 *
 * @param id            Client index or 0 for all clients
 * @param distance      Signal distance
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock send_geiger_signal(id, distance, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_Geiger;

	if(!MSG_Geiger)
		MSG_Geiger = get_user_msgid("Geiger");

	message_begin(get_msg_destination(id, reliable), MSG_Geiger, .player = id);
	write_byte(distance);
	message_end();

	return 1;
}

/**
 * Flags used in hide_hud_elements()
 */
enum HideElemenentFlags (<<= 1)
{
	HideElement_None = 0,
	HideElement_Cross_Ammo_WPNList = 1,
	HideElement_Flashlight,
	HideElement_All,
	HideElement_Radar_Health_Armor,
	HideElement_Timer,
	HideElement_Money,
	HideElement_Crosshair
}

/**
 * Hides specific elements from the HUD.
 *
 * @note The symbol + means that an additional crosshair will be drawn.
 * This crosshair looks not like the regular one, but like the one that
 * is drawn in spectator mode. You can remove this crosshair by setting
 * the "noadd" argument to "true".
 *
 * @param id            Client index or 0 for all clients
 * @param elements      HUD elements to hide. The names are self-explanatory:
 *                        HideElement_Cross_Ammo_WPNList
 *                        HideElement_Flashlight (+)
 *                        HideElement_All
 *                        HideElement_Radar_Health_Armor (+)
 *                        HideElement_Timer (+)
 *                        HideElement_Money (+)
 *                        HideElement_Crosshair
 * @param noadd         If set to false and the chosen element names have
 *                      a "+" sign, an additional crosshair will be drawn.
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock hide_hud_elements(id, HideElemenentFlags:elements, bool:noadd = true, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_HideWeapon;

	if(!MSG_HideWeapon)
		MSG_HideWeapon = get_user_msgid("HideWeapon");

	new iDest = get_msg_destination(id, reliable);

	message_begin(iDest, MSG_HideWeapon, .player = id);
	write_byte(_:elements);
	message_end();

	if(noadd)
	{
		static MSG_Crosshair;

		if(!MSG_Crosshair)
			MSG_Crosshair = get_user_msgid("Crosshair");

		message_begin(iDest, MSG_Crosshair, .player = id);
		write_byte(0);
		message_end();
	}

	return 1;
}

/**
 * Flags used in fade_user_screen()
 */
enum ScreenFadeFlags
{
	ScreenFade_FadeIn = 0x0000,
	ScreenFade_FadeOut = 0x0001,
	ScreenFade_Modulate = 0x0002,
	ScreenFade_StayOut = 0x0004
}

/**
 * Fades the client's screen.
 *
 * @param id            Client index or 0 for all clients
 * @param duration      How long (in seconds) the fade is going to stay
 *                      on the screen (0 - 16)
 * @param fadetime      How many seconds is the fade going to fade in (0 - 16)
 * @param flags         Screen fade flags:
 *                        ScreenFade_FadeIn       - default
 *                        ScreenFade_FadeOut      - fade out (not in)
 *                        ScreenFade_Modulate     - modulate (don't blend)
 *                        ScreenFade_StayOut      - ignores the duration and stays faded out until a new ScreenFade messages is received
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Color alpha (brightness) (0 - 255)
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock fade_user_screen(id, Float:duration = 1.0, Float:fadetime = 1.0, ScreenFadeFlags:flags = ScreenFade_FadeIn, r = 0, g = 0, b = 255, a = 75, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_ScreenFade;

	if(!MSG_ScreenFade)
		MSG_ScreenFade = get_user_msgid("ScreenFade");

	message_begin(get_msg_destination(id, reliable), MSG_ScreenFade, .player = id);
	write_short(float_to_short(fadetime));
	write_short(float_to_short(duration));
	write_short(_:flags);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	message_end();

	return 1;
}

/**
 * Shakes the client's screen.
 *
 * @param id            Client index or 0 for all clients
 * @param amplitude     Shake amplitude (0 - 16)
 * @param duration      Shake duration (in seconds) (0 - 16)
 * @param frequency     Delay between each shake (0 - 16)
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock shake_user_screen(id, Float:amplitude = 3.0, Float:duration = 3.0, Float:frequency = 1.0, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_ScreenShake;

	if(!MSG_ScreenShake)
		MSG_ScreenShake = get_user_msgid("ScreenShake");

	message_begin(get_msg_destination(id, reliable), MSG_ScreenShake, .player = id);
	write_short(float_to_short(amplitude));
	write_short(float_to_short(duration));
	write_short(float_to_short(frequency));
	message_end();

	return 1;
}

/**
 * Changes the client's field of view (FOV).
 *
 * @note Setting the "fov" argument below 45 will also draw a sniper scope.
 *
 * @param id            Client index or 0 for all clients
 * @param fov           Field of view degrees (0 - 255)
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock set_user_fov(id, fov = 0, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_SetFOV;

	if(!MSG_SetFOV)
		MSG_SetFOV = get_user_msgid("SetFOV");

	message_begin(get_msg_destination(id, reliable), MSG_SetFOV, .player = id);
	write_byte(fov);
	message_end();

	return 1;
}

/**
 * @endsection
 */

/**
 * @section Counter-Strike message stocks
 */

/**
 * Draws a HUD progress bar which is filled from 0% to 100% for the given
 * amount of seconds. Once the bar is fully filled it will be removed from
 * the screen automatically.
 *
 * @note If the "startpercent" argument is greater than 0, the bar will be
 * filled from that amount of percentage instead of starting from 0%.
 *
 * @param id            Client index or 0 for all clients
 * @param duration      How long (in seconds) until the bar is fully filled
 * @param startpercent  Bar starting percentage (0-100)
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock cs_draw_progress_bar(id, duration, startpercent = 0, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	if(startpercent)
	{
		static MSG_BarTime2;

		if(!MSG_BarTime2)
			MSG_BarTime2 = get_user_msgid("BarTime2");

		message_begin(get_msg_destination(id, reliable), MSG_BarTime2, .player = id);
	}
	else
	{
		static MSG_BarTime;

		if(!MSG_BarTime)
			MSG_BarTime = get_user_msgid("BarTime");

		message_begin(get_msg_destination(id, reliable), MSG_BarTime, .player = id);
	}

	write_short(duration);

	if(startpercent)
		write_short(startpercent);

	message_end();
	return 1;
}

/**
 * Plays a generic reload sound.
 *
 * @param id            Client index or 0 for all clients
 * @param shotgun       If set to true, it will play "weapons/generic_shot_reload.wav",
 *                      otherwise it will play "weapons/generic_reload.wav".
 * @param volume        Volume amount (0 - 255)
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock cs_play_reload_sound(id, bool:shotgun = false, volume = 100, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_ReloadSound;

	if(!MSG_ReloadSound)
		MSG_ReloadSound = get_user_msgid("ReloadSound");

	message_begin(get_msg_destination(id, reliable), MSG_ReloadSound, .player = id);
	write_byte(volume);
	write_byte(!shotgun);
	message_end();

	return 1;
}

/**
 * Displays a sprite to the right side of the round timer.
 *
 * @note A list of all icons and screenshots of them can be found here:
 *       http://doktor.haze.free.fr/counter-strike/sprites_screens/index.php
 *
 * @param id            Client index or 0 for all clients
 * @param active        If set to 0, it will remove the scenario icon and
 *                      ignore all other arguments. Always set this to 1
 *                      if you want to use any of the other arguments
 * @param sprite        Sprite name (valid names are listed in sprites.hud.txt)
 * @param alpha         Sprite alpha (100-255)
 * @param flashrate     If nonzero, the sprite will flash from the given alpha
 *                      in the "alpha" argument to an alpha of 100, at a rate
 *                      set in this argument
 * @param flashdelay    Delay (in seconds) between each flash
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock cs_set_hud_icon(id, active = 0, sprite[] = "", alpha = 100, flashrate = 0, flashdelay = 0, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_Scenario;

	if(!MSG_Scenario)
		MSG_Scenario = get_user_msgid("Scenario");

	message_begin(get_msg_destination(id, reliable), MSG_Scenario, .player = id);
	write_byte(active);

	if(active)
	{
		write_string(sprite);
		write_byte(alpha);
	}

	if(flashrate)
	{
		write_short(flashrate);
		write_short(flashdelay);
	}

	message_end();
	return 1;
}

/**
 * Creates/Hides the shadow beneath players.
 *
 * @note This stock can't be used to set shadow to a specific player. It can
 *       only set the shadow that a specific player sees for all other players.
 *
 * @param id            Client index or 0 for all clients
 * @param shadowid      Sprite index or 0 to disable the shadow
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "id" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock cs_set_user_shadow(id, shadowid = 0, bool:reliable = true)
{
	if(id && !is_user_connected(id))
		return 0;

	static MSG_ShadowIdx;

	if(!MSG_ShadowIdx)
		MSG_ShadowIdx = get_user_msgid("ShadowIdx");

	message_begin(get_msg_destination(id, reliable), MSG_ShadowIdx, .player = id);
	write_long(shadowid);
	message_end();

	return 1;
}

/**
 * @endsection
 */

/**
 * @section Stocks using temporary entities (SVC_TEMPENTITY)
 */

/**
 * Creates a beam between two points.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=3s
 *
 * @param startpos      Starting coordinates of the beam
 * @param endpos        Ending coordinates of the beam
 * @param sprite        The sprite index to use in the beam
 * @param startframe    The frame to start with in the sprite (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_between_points(startpos[3], endpos[3], sprite, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMPOINTS);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates a beam between an entity and a point.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=20s
 *
 * @param startent      Primary entity id
 * @param endpos        Ending coordinates of the beam
 * @param sprite        The sprite index to use in the beam
 * @param startframe    The frame to start with in the sprite (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_from_entity(startent, endpos[3], sprite, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMENTPOINT);
	write_short(startent);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates a gunshot that consists of a particle effect and a ricochet sound.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=36s
 *
 * @param position      Gunshot coordinates
 * @param receiver      Client index that will be able to see the gunshot
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_gunshot(position[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_GUNSHOT);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	message_end();

	return 1;
}

/**
 * Creates an explosion.
 *
 * @note A common sprite to use is "sprites/zerogxplode.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=43s
 *
 * @param position      Position of the explosion
 * @param sprite        The additive sprite index to use in the explosion
 * @param scale         The scale of the sprite in the explosion (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param flags         Explosion flags:
 *                        TE_EXPLFLAG_NONE               - default Half-Life explosion
 *                        TE_EXPLFLAG_NOADDITIVE         - sprite will be drawn opaque
 *                        TE_EXPLFLAG_NODLIGHTS          - don't render the dynamic lights
 *                        TE_EXPLFLAG_NOSOUND            - don't play the explosion sound
 *                        TE_EXPLFLAG_NOPARTICLES        - don't draw the particles
 * @param receiver      Client index that will be able to see the explosion
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_explosion(position[3], sprite, scale = 10, framerate = 30, flags = TE_EXPLFLAG_NONE, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_EXPLOSION);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(sprite);
	write_byte(scale);
	write_byte(framerate);
	write_byte(flags);
	message_end();

	return 1;
}

/**
 * Creates the Quake "tarbaby" explosion with sound.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=55s
 *
 * @param position      Position of the explosion
 * @param receiver      Client index that will be able to see the explosion
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_tar_explosion(position[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_TAREXPLOSION);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	message_end();

	return 1;
}

/**
 * Creates smoke (a rising alphablend sprite at 30 pps).
 *
 * @note A common sprite to use is "sprites/steam1.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=1m2s
 *
 * @param position      Position of the smoke effect
 * @param sprite        The alphablend sprite index to use for the smoke
 * @param scale         The scale of the smoke (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param receiver      Client index that will be able to see the smoke
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_smoke(position[3], sprite, scale = 10, framerate = 30, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SMOKE);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(sprite);
	write_byte(scale);
	write_byte(framerate);
	message_end();

	return 1;
}

/**
 * Creates a tracer effect from one point to another.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=1m12s
 *
 * @param startpos      Starting position of the tracer
 * @param endpos        Ending position of the tracer
 * @param receiver      Client index that will be able to see the tracer
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_tracer(startpos[3], endpos[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_TRACER);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	message_end();

	return 1;
}

/**
 * Creates a beam between two entities.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=1m26s
 *
 * @param startent      Primary entity id
 * @param endent        Secondary entity id
 * @param sprite        The sprite index to use in the beam
 * @param startframe    The frame to start with in the sprite (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_between_entities(startent, endent, sprite, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMENTS);
	write_short(startent);
	write_short(endent);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates 8 random tracers with gravity and a ricochet sprite.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=1m41s
 *
 * @param position      Position of the effect
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_sparks(position[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SPARKS);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	message_end();

	return 1;
}

/**
 * Creates a Quake-style lava splash.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=1m49s
 *
 * @param position      Position of the effect
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_lava_splash(position[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_LAVASPLASH);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	message_end();

	return 1;
}

/**
 * Creates a Quake-style teleport splash.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=2m6s
 *
 * @param position      Position of the effect
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_teleport_splash(position[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_TELEPORT);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	message_end();

	return 1;
}

/**
 * Creates a Quake colormapped (base palette) particle explosion with sound.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=2m19s
 *
 * @param position      Position of the explosion
 * @param startcolor    Starting color (0 - 255)
 * @param numcolors     Number of colors (1 - 255)
 * @param receiver      Client index that will be able to see the explosion
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_colored_explosion(position[3], startcolor = 0, numcolors = 255, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_EXPLOSION2);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_byte(startcolor);
	write_byte(clamp(numcolors, 1)); // 0 will crash the game
	message_end();

	return 1;
}

/**
 * Places a decal from the .BSP file.
 *
 * @note Using a decal index that doesn't exist will crash the client.
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=2m30s
 *
 * @param position      Position of the decal (center of texture in world)
 * @param texture       Texture index of precached decal texture name
 * @param entity        Entity index or 0 for world
 * @param entabove      Model index of the entity above (only available if
 *                      the "entity" argument is non-zero)
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_place_decal_from_bsp_file(position[3], texture, entity = 0, entabove = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BSPDECAL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(texture);
	write_short(entity);

	if(entity)
		write_short(entabove);

	message_end();

	return 1;
}

/**
 * Creates tracers moving towards a point.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=2m44s
 *
 * @param position      Position of the implosion effect
 * @param radius        Implosion radius
 * @param count         Number of tracers to generate
 * @param life          The length of time the effect shall remain
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_implosion(position[3], radius = 64, count = 10, life = 3, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_IMPLOSION);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_byte(radius);
	write_byte(count);
	write_byte(life);
	message_end();

	return 1;
}

/**
 * Creates a line of moving glow sprites or models with gravity, fadeout,
 * and collisions.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=2m58s
 *
 * @param startpos      Starting position of the effect
 * @param endpos        Ending position of the effect
 * @param sprite        Sprite index
 * @param count         Number of models/sprites to generate
 * @param life          The length of time the effect shall remain
 * @param scale         Scale of the effect
 * @param velocity      Velocity along vector
 * @param randomness    Randomness of the velocity
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_model_trail(startpos[3], endpos[3], sprite, count = 1, life = 10, scale = 10, velocity = 10, randomness = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SPRITETRAIL);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_short(sprite);
	write_byte(count);
	write_byte(life);
	write_byte(scale);
	write_byte(velocity);
	write_byte(randomness);
	message_end();

	return 1;
}

/**
 * Displays an additive sprite that plays one cycle.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=3m7s
 *
 * @param position      Sprite position
 * @param sprite        Sprite index
 * @param scale         Scale of the sprite
 * @param brightness    Brightness of the sprite
 * @param receiver      Client index that will be able to see the sprite
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_display_additive_sprite(position[3], sprite, scale = 5, brightness = 255, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SPRITE);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(sprite);
	write_byte(scale);
	write_byte(brightness);
	message_end();

	return 1;
}

/**
 * Creates a beam with a sprite at the end of the beam.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=3m29s
 *
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param startpos      Starting position of the beam
 * @param endpos        Ending position of the beam
 * @param beamid        Sprite index for the beam body
 * @param endid         Sprite index for the end of the beam
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_sprite(startpos[3], endpos[3], beamid, endid, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMSPRITE);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_short(beamid);
	write_short(endid);
	message_end();

	return 1;
}

/**
 * Creates a screen aligned beam ring that expands to the maximum radius
 * over lifetime.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=3m40s
 *
 * @param position      Starting coordinates of the beam
 * @param sprite        The sprite index to use in the beam
 * @param axis          Beam axis
 * @param startframe    The frame to start with in the sprite
 * @param framerate     The frame rate to show the sprite at
 * @param life          The length of time the beam shall remain
 * @param width         The width of the beam
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_ring(position[3], sprite, axis[3] = {0, 0, 0}, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMTORUS);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(axis[0]);
	write_coord(axis[1]);
	write_coord(axis[2]);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates a beam disk that expands to the maximum radius over lifetime.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=3m58s
 *
 * @param position      Starting coordinates of the beam
 * @param sprite        The sprite index to use in the beam
 * @param axis          Beam axis
 * @param startframe    The frame to start with in the sprite (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_disk(position[3], sprite, axis[3] = {0, 0, 0}, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMDISK);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(axis[0]);
	write_coord(axis[1]);
	write_coord(axis[2]);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates a beam cylinder that expands to the maximum radius over lifetime.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=4m17s
 *
 * @param position      Starting coordinates of the beam
 * @param sprite        The sprite index to use in the beam
 * @param axis          Beam axis
 * @param startframe    The frame to start with in the sprite (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_cylinder(position[3], sprite, axis[3] = {0, 0, 0}, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMCYLINDER);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(axis[0]);
	write_coord(axis[1]);
	write_coord(axis[2]);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates a decaying beam that follows the entity until it stops moving.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note When the entity stops moving, the beam will become visible again
 *       once the entity starts moving.
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=4m31s
 *
 * @param entity        Entity that the beam will follow
 * @param sprite        The sprite index to use in the beam
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_following_beam(entity, sprite, life = 10, width = 10, r = 0, g = 0, b = 255, a = 75, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMFOLLOW);
	write_short(entity);
	write_short(sprite);
	write_byte(life);
	write_byte(width);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	message_end();

	return 1;
}

/**
 * Displays a glowing sprite.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=4m43s
 *
 * @param position      Sprite position
 * @param sprite        Sprite index
 * @param scale         Sprite scale (0 - 255)
 * @param size          Sprite size (0 - 255)
 * @param brightness    Sprite brightness (0 - 255)
 * @param receiver      Client index that will be able to see the sprite
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_display_glow_sprite(position[3], sprite, scale = 10, size = 10, brightness = 150, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_GLOWSPRITE);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(sprite);
	write_byte(scale);
	write_byte(size);
	write_byte(brightness);
	message_end();

	return 1;
}

/**
 * Creates a beam ring between two entities.
 *
 * @note A common sprite to use is "sprites/laserbeam.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=5m10s
 *
 * @param startent      Primary entity id
 * @param endent        Secondary entity id
 * @param sprite        The sprite index to use in the beam
 * @param startframe    The frame to start with in the sprite (0 - 255)
 * @param framerate     The frame rate to show the sprite at (0 - 255)
 * @param life          The length of time the beam shall remain (0 - 255)
 * @param width         The width of the beam (0 - 255)
 * @param noise         The noise amplitude of the beam, this controls
 *                      the distorsion of the beam (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param a             Beam brightness (alpha) (0 - 255)
 * @param speed         The scroll speed of the beam (0 - 255)
 * @param receiver      Client index that will be able to see the beam
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_beam_ring_between_ent(startent, endent, sprite, startframe = 0, framerate = 30, life = 10, width = 10, noise = 0, r = 0, g = 0, b = 255, a = 75, speed = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BEAMRING);
	write_short(startent);
	write_short(endent);
	write_short(sprite);
	write_byte(startframe);
	write_byte(framerate);
	write_byte(life);
	write_byte(width);
	write_byte(noise);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(a);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Creates an oriented shower of tracers.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=5m34s
 *
 * @param position      Position of the effect
 * @param direction     Effect direction
 * @param color         Effect color (https://wiki.alliedmods.net/images/Palette.png)
 * @param count         Number of tracers to create
 * @param speed         The scroll speed of the effect
 * @param velocity      Random velocity
 * @param receiver      Client index that will be able to see the tracers
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_tracer_shower(position[3], direction[3] = {0, 0, 0}, color = 12, count = 1, speed = 0, velocity = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_STREAK_SPLASH);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(direction[0]);
	write_coord(direction[1]);
	write_coord(direction[2]);
	write_byte(color);
	write_short(count);
	write_short(speed);
	write_short(velocity);
	message_end();

	return 1;
}

/**
 * Creates a dynamic light with a world effect.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=5m47s
 *
 * @param position      Position of the light
 * @param radius        Light radius (0 - 255)
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param life          The length of time the light shall remain (0 - 255)
 * @param decay         Light decay rate (0 - 255)
 * @param receiver      Client index that will be able to see the light
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_dynamic_light(position[3], radius = 10, r = 255, g = 255, b = 255, life = 10, decay = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_DLIGHT);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_byte(radius);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(life);
	write_byte(decay);
	message_end();

	return 1;
}

/**
 * Creates a point entity light with no world effect.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=6m7s
 *
 * @param entity        Entity or client to apply the light on
 * @param position      Position of the light
 * @param radius        Light radius
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param life          The length of time the light shall remain (0 - 255)
 * @param decay         Light decay rate
 * @param receiver      Client index that will be able to see the light
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_entity_light(entity, position[3] = {0, 0, 0}, radius = 50, r = 255, g = 255, b = 255, life = 10, decay = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_ELIGHT);
	write_short(entity);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(radius);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	write_byte(life);
	write_coord(decay);
	message_end();

	return 1;
}

/**
 * Draws a simple line.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=6m32s
 *
 * @param startpos      Starting position of the line
 * @param endpos        Ending position of the line
 * @param life          Line life
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param receiver      Client index that will be able to see the line
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_draw_line(startpos[3], endpos[3], life = 10, r = 0, g = 0, b = 255, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_LINE);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_short(life);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	message_end();

	return 1;
}

/**
 * Creates a simple box.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=6m45s
 *
 * @param startpos      Starting position of the box
 * @param endpos        Ending position of the box
 * @param life          Box life
 * @param r             Red color amount (0 - 255)
 * @param g             Green color amount (0 - 255)
 * @param b             Blue color amount (0 - 255)
 * @param receiver      Client index that will be able to see the box
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_box(startpos[3], endpos[3], life = 10, r = 0, g = 0, b = 255, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BOX);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_short(life);
	write_byte(r);
	write_byte(g);
	write_byte(b);
	message_end();

	return 1;
}

/**
 * Removes all beams attached to an entity.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=7m7s
 *
 * @param entity        Entity id to remove attached beams from
 * @param receiver      Client index that will be able to see the changes
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_remove_all_beams_from_entity(entity, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_KILLBEAM);
	write_short(entity);
	message_end();

	return 1;
}

/**
 * Flags used in te_create_large_funnel()
 */
#define LF_FLOAT_DOWN 0
#define LF_FLOAT_UP 1

/**
 * Creates a large group of sprites or models accompanied by green dots
 * that float up or down until they reach the point set in the "position" argument.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=7m10s
 *
 * @param position      Effect position
 * @param sprite        Sprite index to use
 * @param flag          List of valid flags:
 *                        LF_FLOAT_DOWN           - float downwards and end in the point set in the "position" argument
 *                        LF_FLOAT_UP             - start from the point set in the "position" argument and float upwards
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_large_funnel(position[3], sprite, flag = LF_FLOAT_DOWN, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_LARGEFUNNEL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(sprite);
	write_short(flag);
	message_end();

	return 1;
}

/**
 * Creates dripping blood particles.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=7m35s
 *
 * @param position      Starting position of the blood
 * @param direction     Blood direction
 * @param color         Blood color (https://wiki.alliedmods.net/images/Palette.png)
 * @param count         Number of blood particles to generate (0 - 255)
 * @param receiver      Client index that will be able to see the blood
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_bloodstream(position[3], direction[3] = {0, 0, 0}, color = 78, count = 1, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BLOODSTREAM);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(direction[0]);
	write_coord(direction[1]);
	write_coord(direction[2]);
	write_byte(color);
	write_byte(count);
	message_end();

	return 1;
}

/**
 * Draws a line of blood particles spread 5 units from each other that
 * disappears after 30 seconds.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=7m53s
 *
 * @param startpos      Starting position of the line
 * @param endpos        Ending position of the line
 * @param receiver      Client index that will be able to see the line
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_draw_blood_line(startpos[3], endpos[3], receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SHOWLINE);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	message_end();

	return 1;
}

/**
 * Sprays blood particles from a given point.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=7m59s
 *
 * @param position      Position from where the blood will be sprayed
 * @param direction     Blood spraying direction
 * @param color         Blood color (https://wiki.alliedmods.net/images/Palette.png)
 * @param speed         Speed at which the blood particles will be sprayed (0 - 255)
 * @param receiver      Client index that will be able to see the particles
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_spray_blood(position[3], direction[3] = {0, 0, 0}, color = 78, speed = 30, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BLOOD);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(direction[0]);
	write_coord(direction[1]);
	write_coord(direction[2]);
	write_byte(color);
	write_byte(speed);
	message_end();

	return 1;
}

/**
 * Applies a decal to a brush entity (not the world).
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=8m12s
 *
 * @param position      Position of the decal (center of texture in world)
 * @param texture       Texture index of precached decal texture name (0 - 511)
 * @param entity        Entity index to apply the decal to
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_place_brush_decal(position[3], texture, entity = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(texture > 256 ? TE_DECALHIGH : TE_DECAL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_byte(texture > 256 ? texture - 256 : texture);
	write_short(entity);
	message_end();

	return 1;
}

/**
 * Bounce sound types used in te_create_bouncing_model()
 */
enum BounceSounds
{
	BounceSound_Null = 0,
	BounceSound_Shell,
	BounceSound_ShotShell
}

/**
 * Creates a moving model or sprite that bounces and makes a sound when it hits.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=8m16s
 *
 * @param position      Position of the model
 * @param modelid       Model index
 * @param velocity      Model velocity
 * @param yaw           Initial yaw
 * @param bouncesound   Bounce sound type:
 *                        BounceSound_Null
 *                        BounceSound_Shell
 *                        BounceSound_ShotShell
 * @param life          The length of time the model shall remain (0 - 255)
 * @param receiver      Client index that will be able to see the model
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_bouncing_model(position[3], modelid, velocity[3] = {0, 0, 0}, yaw = 0, BounceSounds:bouncesound = BounceSound_Null, life = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_MODEL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(velocity[0]);
	write_coord(velocity[1]);
	write_coord(velocity[2]);
	write_angle(yaw);
	write_short(modelid);
	write_byte(_:bouncesound);
	write_byte(life);
	message_end();

	return 1;
}

/**
 * Creates model or sprite with a blinking orange aura effect.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=8m28s
 *
 * @param position      Position of the model
 * @param modelid       Model index
 * @param speed         Model speed
 * @param count         Number of models to generate
 * @param life          The length of time the model shall remain (0 - 255)
 * @param receiver      Client index that will be able to see the model
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_explode_model(position[3], modelid, speed = 0, count = 1, life = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_EXPLODEMODEL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(speed);
	write_short(modelid);
	write_short(count);
	write_byte(life);
	message_end();

	return 1;
}

/**
 * Flags used in te_create_break_model()
 */
#define BreakModel_TypeMask                  0x4F
#define BreakModel_Glass                     0x01
#define BreakModel_Metal                     0x02
#define BreakModel_Flesh                     0x04
#define BreakModel_Wood                      0x08
#define BreakModel_Smoke                     0x10
#define BreakModel_Trans                     0x20
#define BreakModel_Concrete                  0x40
#define BreakModel_2                         0x80

/**
 * Creates a model or sprite entity that slowly disappears until it's gone.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ
 *
 * @param position      Position of the model
 * @param size          Size of the model
 * @param velocity      Model velocity
 * @param modelid       Model index
 * @param random        Random velocity (0 - 255)
 * @param count         Number of model pieces to generate (0 - 255)
 * @param life          The length of time the model shall remain (0 - 255)
 * @param flags         Break model flags:
 *                        BreakModel_TypeMask
 *                        BreakModel_Glass
 *                        BreakModel_Metal
 *                        BreakModel_Flesh
 *                        BreakModel_Wood
 *                        BreakModel_Smoke
 *                        BreakModel_Trans
 *                        BreakModel_Concrete
 *                        BreakModel_2
 * @param receiver      Client index that will be able to see the model
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_break_model(position[3], modelid, size[3] = {50, 50, 50}, velocity[3] = {0, 0, 0}, random = 0, count = 1, life = 10, flags = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BREAKMODEL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(size[0]);
	write_coord(size[1]);
	write_coord(size[2]);
	write_coord(velocity[0]);
	write_coord(velocity[1]);
	write_coord(velocity[2]);
	write_byte(random);
	write_short(modelid);
	write_byte(count);
	write_byte(life);
	write_byte(flags);
	message_end();

	return 1;
}

/**
 * Places a gunshot decal on an entity or the world and plays a ricochet sound.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=8m41s
 *
 * @param position      Position of the decal
 * @param decal         Decal index (0 - 255)
 * @param entity        Entity to apply the decal to or 0 for world
 * @param receiver      Client index that will be able to see the model
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @noreturn
 * @error               If "receiver" is non-zero and the client isn't
 *                      connected, an error will be thrown.
 */
stock te_place_gunshot_decal(position[3], decal = 41, entity = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_GUNSHOTDECAL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(entity);
	write_byte(decal);
	message_end();

	return 1;
}

/**
 * Creates a spray of alpha sprites or models.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=8m52s
 *
 * @param position      Position of the effect
 * @param sprite        Sprite index
 * @param velocity      Spray velocity
 * @param count         Number of sprays to generate (0 - 255)
 * @param speed         The scroll speed of the effect (0 - 255)
 * @param noise         The noise amplitude of the effect - this
 *                      controls the distorsion of the effect (0 - 255)
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_sprite_spray(position[3], sprite, velocity[3] = {0, 0, 0}, count = 1, speed = 0, noise = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SPRITE_SPRAY);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(velocity[0]);
	write_coord(velocity[1]);
	write_coord(velocity[2]);
	write_short(sprite);
	write_byte(count);
	write_byte(speed);
	write_byte(noise);
	message_end();

	return 1;
}

/**
 * Creates a quick spray sprite with a ricochet sound.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=9m3s
 *
 * @param position      Position of the effect
 * @param scale         Scale of the effect (0 - 255)
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_armor_ricochet(position[3], scale = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_ARMOR_RICOCHET);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_byte(scale);
	message_end();

	return 1;
}

/**
 * Places a player spray on an entity or the world.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=9m16s
 *
 * @param position      Position of the decal
 * @param decal         Decal index (0 - 255)
 * @param entity        Entity to apply the decal to or 0 for world
 * @param receiver      Client index that will be able to see the model
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @noreturn
 * @error               If "receiver" is non-zero and the client isn't
 *                      connected, an error will be thrown.
 */
stock te_place_player_spray(position[3], player, spray = 0, entity = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_PLAYERDECAL);
	write_byte(player);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(entity);
	write_byte(spray);
	message_end();

	return 1;
}

/**
 * Creates alpha sprites or models inside of a box that float upwards.
 *
 * @note A common sprite to use is "sprites/bubble.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=9m22s
 *
 * @param startpos      Start position
 * @param endpos        End position
 * @param sprite        Sprite index
 * @param count         Number of sprites to generate (0 - 255)
 * @param randomness    Randoness of the floating direction
 * @param height        Float height
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_bubble_box(startpos[3], endpos[3], sprite, count = 3, randomness = 0, height = 50, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BUBBLES);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_coord(height);
	write_short(sprite);
	write_byte(count);
	write_coord(randomness);
	message_end();

	return 1;
}

/**
 * Creates alpha sprites or models along a line that float upwards.
 *
 * @note A common sprite to use is "sprites/bubble.spr"
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=9m37s
 *
 * @param startpos      Start position
 * @param endpos        End position
 * @param sprite        Sprite index
 * @param count         Number of sprites to generate (0 - 255)
 * @param randomness    Randoness of the floating direction
 * @param height        Float height
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_bubble_line(startpos[3], endpos[3], sprite, count = 3, randomness = 0, height = 50, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BUBBLETRAIL);
	write_coord(startpos[0]);
	write_coord(startpos[1]);
	write_coord(startpos[2]);
	write_coord(endpos[0]);
	write_coord(endpos[1]);
	write_coord(endpos[2]);
	write_coord(height);
	write_short(sprite);
	write_byte(count);
	write_coord(randomness);
	message_end();

	return 1;
}

/**
 * Creates an spray of opaque sprites or models that fall to another sprite or model.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=9m44s
 *
 * @param position      Effect position
 * @param sprite1id     Primary sprite index
 * @param sprite2id     Secondary sprite index
 * @param color         Sprite color (https://wiki.alliedmods.net/images/Palette.png)
 * @param scale         Sprite scale (0 - 255)
 * @param receiver      Client index that will be able to see the sprite
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_display_falling_sprite(position[3], sprite1id, sprite2id, color = 78, scale = 10, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_BLOODSPRITE);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(sprite1id);
	write_short(sprite2id);
	write_byte(color);
	write_byte(scale);
	message_end();

	return 1;
}

/**
 * Applies a decal to the world brush.
 *
 * @note Using a decal index that doesn't exist will crash the client.
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=9m56s
 *
 * @param position      Decal position (center of texture in world)
 * @param texture       Texture index of precached decal texture name (0 - 511)
 * @param receiver      Client index that will be able to see the decal
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_place_world_decal(position[3], texture, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(texture > 256 ? TE_WORLDDECALHIGH : TE_WORLDDECAL);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_byte(texture > 256 ? texture - 256 : texture);
	message_end();

	return 1;
}

/**
 * Creates a nail-like projectile.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=10m12s
 *
 * @param position      Projectile position
 * @param velocity      Projectile velocity
 * @param model         Model index that will be used for the projectile
 * @param life          The length of time the projectile shall remain (0 - 255)
 * @param owner         The projectile won't collide with the owner, if set to 0,
 *                      the projectile will hit any client
 * @param receiver      Client index that will be able to see the projectile
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_projectile(position[3], velocity[3], model, life = 10, owner = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_PROJECTILE);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(velocity[0]);
	write_coord(velocity[1]);
	write_coord(velocity[2]);
	write_short(model);
	write_byte(life);
	write_byte(owner);
	message_end();

	return 1;
}

/**
 * Creates a shower of sprites or models.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=10m27s
 *
 * @param position      Effect position
 * @param model         Model index that will be used for the effect
 * @param direction     Effect direction
 * @param count         Number of sprites/models to generate (0 - 255)
 * @param speed         The scroll speed of the effect (0 - 255)
 * @param noise         The noise amplitude of the effect - this
 *                      controls the distorsion of the effect (0 - 255)
 * @param rendermode    Render mode - one of kRender* constants
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_sprite_shower(position[3], model, direction[3] = {0, 0, 0}, count = 1, speed = 0, noise = 0, rendermode = kRenderNormal, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_SPRAY);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(direction[0]);
	write_coord(direction[1]);
	write_coord(direction[2]);
	write_short(model);
	write_byte(count);
	write_byte(speed);
	write_byte(noise);
	write_byte(rendermode);
	message_end();

	return 1;
}

/**
 * Emits sprites or models from a player's bounding box.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=10m39s
 *
 * @param player        Client index to emit the sprites from (can't be 0)
 * @param sprite        Sprite index
 * @param count         Number of sprites to generate (0 - 255)
 * @param variance      Variance in size (0 = no variance; 10 = 10% variance) (0 - 255)
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_emit_sprite_from_player(player, sprite, count = 1, variance = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_PLAYERSPRITES);
	write_short(player);
	write_short(sprite);
	write_byte(count);
	write_byte(variance);
	message_end();

	return 1;
}

/**
 * Creates a particle burst similar to te_create_lava_splash.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=10m55s
 *
 * @param position      Effect position
 * @param radius        Effect radius
 * @param color         Particle color (https://wiki.alliedmods.net/images/Palette.png)
 * @param duration      Duration of the effect (will be randomized a bit) (0 - 255)
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_particle_burst(position[3], radius = 30, color = 106, duration = 5, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_PARTICLEBURST);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(radius);
	write_byte(color);
	write_byte(duration);
	message_end();

	return 1;
}

/**
 * Creates a field of fire.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=11m7s
 *
 * @param position      Effect position
 * @param sprite        Sprite or model index to use for the fire
 * @param radius        Effect radius
 * @param count         Number of fields to generate (0 - 255)
 * @param duration      Duration of the effect (will be randomized a bit) (0 - 255)
 * @param flags         Available flags:
 *                        TEFIRE_FLAG_ALLFLOAT    - all sprites will drift upwards as they animate
 *                        TEFIRE_FLAG_SOMEFLOAT   - some of the sprites will drift upwards (50% chance)
 *                        TEFIRE_FLAG_LOOP        - if set, sprite plays at 15 fps, otherwise plays at whatever rate stretches the animation over the sprite's duration
 *                        TEFIRE_FLAG_ALPHA       - if set, sprite is rendered alpha blended at 50% else, opaque
 *                        TEFIRE_FLAG_PLANAR      - if set, all fire sprites have same initial Z instead of randomly filling a cube
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_fire_field(position[3], sprite, radius = 5, count = 1, duration = 10, flags = TEFIRE_FLAG_ALLFLOAT, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_FIREFIELD);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_short(radius);
	write_short(sprite);
	write_byte(count);
	write_byte(flags);
	write_byte(duration);
	message_end();

	return 1;
}

/**
 * Attaches a temporary entity model to a client.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=11m16s
 *
 * @param player        Client index to attach the model to
 * @param model         Model index
 * @param offset        Vertical offset (attachment origin.z = player origin.z + vertical offset)
 * @param life          The length of time the model shall remain
 * @param receiver      Client index that will be able to see the model
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_attach_model_to_player(player, model, offset = 0, life = 5, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_PLAYERATTACHMENT);
	write_byte(player);
	write_coord(offset);
	write_short(model);
	write_short(life);
	message_end();

	return 1;
}

/**
 * Kills all temporary entity models attached to a client.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=11m24s
 *
 * @param player        Client index to remove the attachments from
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @noreturn
 * @error               If "receiver" is non-zero and the client isn't
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_remove_player_attachments(player, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_KILLPLAYERATTACHMENTS);
	write_byte(player);
	message_end();

	return 1;
}

/**
 * Much more compact shotgun shot stock.
 *
 * @note This stock is used to make a client approximate a 'spray' of gunfire.
 *       Any weapon that fires more than one bullet per frame and fires in a bit
 *       of a spread is a good candidate for MULTIGUNSHOT use. (shotguns)
 * @note This effect makes the client do traces for each bullet, these client
 *       traces ignore entities that have studio models. Traces are 4096 long.
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=11m27s
 *
 * @param position      Gunshot position
 * @param direction     Gunshot direction
 * @param decal         Bullethole decal texture index (0 - 255)
 * @param count         Number of bulletholes to generate (0 - 255)
 * @param noise_x       X noise multiplied by 100
 * @param noise_y       Y noise multiplied by 100
 * @param receiver      Client index that will be able to see the Gunshot
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_multi_gunshot(position[3], direction[3], decal = 105, count = 1, noise_x = 0, noise_y = 0, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_MULTIGUNSHOT);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(direction[0]);
	write_coord(direction[1]);
	write_coord(direction[2]);
	write_coord(noise_x);
	write_coord(noise_y);
	write_byte(count);
	write_byte(decal);
	message_end();

	return 1;
}

/**
 * Creates a tracer effect and allows more customization than te_create_tracer.
 *
 * @note Video preview of this and all other te_ stocks can be found here:
 *       https://youtu.be/szW-bSMPuyQ?t=11m36s
 *
 * @param position      Effect position
 * @param velocity      Effect velocity
 * @param life          The length of time the effect shall remain, multiplied by 10 (0 - 255)
 * @param color         Tracer color (0 - 12)
 * @param length        Length of the tracer (0 - 255)
 * @param receiver      Client index that will be able to see the effect
 *                      or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              0 if "receiver" is non-zero and the client isn't connected,
 *                      1 otherwise
 */
stock te_create_user_tracer(position[3], velocity[3], life = 1, color = 106, length = 1, receiver = 0, bool:reliable = true)
{
	if(receiver && !is_user_connected(receiver))
		return 0;

	message_begin(get_msg_destination(receiver, reliable), SVC_TEMPENTITY, .player = receiver);
	write_byte(TE_USERTRACER);
	write_coord(position[0]);
	write_coord(position[1]);
	write_coord(position[2]);
	write_coord(velocity[0]);
	write_coord(velocity[1]);
	write_coord(velocity[2]);
	write_byte(life);
	write_byte(clamp(color, 0, 12));
	write_byte(length);
	message_end();

	return 1;
}

/**
 * @endsection
 */

/**
 * Used with message stocks. Returns whether or not to use the reliable or
 * unreliable channel when sending a message according to the params used.
 *
 * @param id            Client index or 0 for all clients
 * @param reliable      If true, the message will be sent via the reliable
 *                      channel, otherwise it will use the unreliable one
 *
 * @return              MSG_ONE if "id" is non-zero and "reliable" is true,
 *                      MSG_ONE_UNRELIABLE if "id" is non-zero and "reliable" is false,
 *                      MSG_ALL if "id" is zero and "reliable" is true,
 *                      MSG_BROADCAST if "id" is zero and "reliable" is false.
 */
stock get_msg_destination(id, bool:reliable)
{
	if(id)
		return reliable ? MSG_ONE : MSG_ONE_UNRELIABLE;

	return reliable ? MSG_ALL : MSG_BROADCAST;
}

/**
 * Converts a float value into a short.
 *
 * @param value         Float value to convert
 *
 * @return              Value converted to short
 */
stock float_to_short(Float:value)
	return clamp(floatround(value * (1<<12)), 0, 0xFFFF);