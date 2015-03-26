// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"

/*
===============================================================================

	Player control of the Doom Marine.
	This object handles all player movement and world interaction.

===============================================================================
*/

// distance between ladder rungs (actually is half that distance, but this sounds better)
const int LADDER_RUNG_DISTANCE = 32;

// amount of health per dose from the health station
const int HEALTH_PER_DOSE = 10;

// time before a weapon dropped to the floor disappears
const int WEAPON_DROP_TIME = 20 * 1000;

// time before a next or prev weapon switch happens
const int WEAPON_SWITCH_DELAY = 150;

// how many units to raise spectator above default view height so it's in the head of someone
const int SPECTATE_RAISE = 25;

const int HEALTHPULSE_TIME = 333;

// minimum speed to bob and play run/walk animations at
const float MIN_BOB_SPEED = 5.0f;

// #ifdef _DT // levitate spell // set through cvar
// push velocity when start levitating
// const int LEVITATE_PUSH_VELOCITY = 50;

// max step height while levitating
// const int LEVITATE_STEP_HEIGHT = 37;
// #endif

const idEventDef EV_Player_GetButtons( "getButtons", NULL, 'd' );
const idEventDef EV_Player_GetMove( "getMove", NULL, 'v' );
const idEventDef EV_Player_GetViewAngles( "getViewAngles", NULL, 'v' );
const idEventDef EV_Player_StopFxFov( "stopFxFov" );
const idEventDef EV_Player_EnableWeapon( "enableWeapon" );
const idEventDef EV_Player_DisableWeapon( "disableWeapon" );
const idEventDef EV_Player_GetCurrentWeapon( "getCurrentWeapon", NULL, 's' );
const idEventDef EV_Player_GetPreviousWeapon( "getPreviousWeapon", NULL, 's' );
const idEventDef EV_Player_SelectWeapon( "selectWeapon", "s" );
const idEventDef EV_Player_GetWeaponEntity( "getWeaponEntity", NULL, 'e' );
const idEventDef EV_Player_OpenPDA( "openPDA" );
const idEventDef EV_Player_InPDA( "inPDA", NULL, 'd' );
#ifdef _DT // levitate spell
const idEventDef EV_Player_LevitateStart( "levitateStart" );
const idEventDef EV_Player_LevitateStop( "levitateStop" );
#endif
const idEventDef EV_Player_ExitTeleporter( "exitTeleporter" );
const idEventDef EV_Player_StopAudioLog( "stopAudioLog" );
const idEventDef EV_Player_HideTip( "hideTip" );
const idEventDef EV_Player_LevelTrigger( "levelTrigger" );
const idEventDef EV_SpectatorTouch( "spectatorTouch", "et" );
const idEventDef EV_Player_GetIdealWeapon( "getIdealWeapon", NULL, 's' );

//ivan start
const idEventDef EV_Player_ForceUpdateNpcStatus( "forceUpdateNpcStatus" );
const idEventDef EV_Player_SetCommonEnemy( "setCommonEnemy", "E" );
const idEventDef EV_Player_GetCommonEnemy( "getCommonEnemy", NULL, 'e' );
//ivan end

//*****************************************************************
//*****************************************************************
// Arx EOS Events
const idEventDef EV_Player_InventoryContainsItem( "inventoryContainsItem", "s", 'f' );
const idEventDef EV_Player_LevelTransitionSpawnPoint( "levelTransitionSpawnPoint", "s", NULL );
const idEventDef EV_HudMessage( "HudMessage", "s" );
const idEventDef EV_SetFloatHUDParm( "SetFloatHUDParm", "sf", NULL );
const idEventDef EV_PlayerMoney( "PlayerMoney", "d", 'd' );
const idEventDef EV_OpenCloseShop( "OpenCloseShop", "s", NULL );
const idEventDef EV_RemoveInventoryItem( "RemoveInventoryItem", "s", NULL );
const idEventDef EV_GiveInventoryItem( "GiveInventoryItem", "s", NULL );
const idEventDef EV_FindInventoryItemCount( "FindInventoryItemCount", "s", 'f' );
const idEventDef EV_GiveJournal( "GiveJournal", "s", NULL );
const idEventDef EV_GetMapName( "GetMapName", NULL, 's' );
const idEventDef EV_ModifyPlayerXPs( "modifyPlayerXPs", "d", NULL );

//*****************************************************************
//*****************************************************************

CLASS_DECLARATION( idActor, idPlayer )
	EVENT( EV_Player_GetButtons,			idPlayer::Event_GetButtons )
	EVENT( EV_Player_GetMove,				idPlayer::Event_GetMove )
	EVENT( EV_Player_GetViewAngles,			idPlayer::Event_GetViewAngles )
	EVENT( EV_Player_StopFxFov,				idPlayer::Event_StopFxFov )
	EVENT( EV_Player_EnableWeapon,			idPlayer::Event_EnableWeapon )
	EVENT( EV_Player_DisableWeapon,			idPlayer::Event_DisableWeapon )
	EVENT( EV_Player_GetCurrentWeapon,		idPlayer::Event_GetCurrentWeapon )
	EVENT( EV_Player_GetPreviousWeapon,		idPlayer::Event_GetPreviousWeapon )
	EVENT( EV_Player_SelectWeapon,			idPlayer::Event_SelectWeapon )
	EVENT( EV_Player_GetWeaponEntity,		idPlayer::Event_GetWeaponEntity )
	EVENT( EV_Player_OpenPDA,				idPlayer::Event_OpenPDA )
	EVENT( EV_Player_InPDA,					idPlayer::Event_InPDA )
#ifdef _DT // levitate spell
	EVENT( EV_Player_LevitateStart,			idPlayer::Event_LevitateStart )
	EVENT( EV_Player_LevitateStop,			idPlayer::Event_LevitateStop )
#endif
	EVENT( EV_Player_ExitTeleporter,		idPlayer::Event_ExitTeleporter )
	EVENT( EV_Player_StopAudioLog,			idPlayer::Event_StopAudioLog )
	EVENT( EV_Player_HideTip,				idPlayer::Event_HideTip )
	EVENT( EV_Player_LevelTrigger,			idPlayer::Event_LevelTrigger )
	EVENT( EV_Gibbed,						idPlayer::Event_Gibbed )
	EVENT( EV_Player_GetIdealWeapon,		idPlayer::Event_GetIdealWeapon )
	//ivan start
	EVENT( EV_Player_ForceUpdateNpcStatus,  idPlayer::Event_ForceUpdateNpcStatus)
	EVENT( EV_Player_SetCommonEnemy,		idPlayer::Event_SetCommonEnemy)
	EVENT( EV_Player_GetCommonEnemy,		idPlayer::Event_GetCommonEnemy) 
	//ivan end
	//*****************************************************************
	//*****************************************************************
	// Arx EOS Events
	EVENT( EV_Player_InventoryContainsItem,		idPlayer::Event_InventoryContainsItem )
	EVENT( EV_Player_LevelTransitionSpawnPoint,	idPlayer::Event_LevelTransitionSpawnPoint )
	EVENT( EV_HudMessage,						idPlayer::Event_HudMessage )
	EVENT( EV_SetFloatHUDParm,					idPlayer::Event_SetFloatHUDParm )
	EVENT( EV_PlayerMoney,						idPlayer::Event_PlayerMoney )
	EVENT( EV_OpenCloseShop,					idPlayer::Event_OpenCloseShop )
	EVENT( EV_RemoveInventoryItem,				idPlayer::Event_RemoveInventoryItem )
	EVENT( EV_GiveInventoryItem,				idPlayer::Event_GiveInventoryItem )
	EVENT( EV_FindInventoryItemCount,			idPlayer::Event_FindInventoryItemCount )
	EVENT( EV_GiveJournal,						idPlayer::Event_GiveJournal )
	EVENT( EV_GetMapName,						idPlayer::Event_GetMapName )
	EVENT( EV_ModifyPlayerXPs,					idPlayer::Event_ModifyPlayerXPs )
	//*****************************************************************
	//*****************************************************************

END_CLASS

const int MAX_RESPAWN_TIME = 10000;
const int RAGDOLL_DEATH_TIME = 3000;
const int MAX_PDAS = 256;							// Solarsplace - 15th JUne 2012 increased from 64
const int MAX_PDA_ITEMS = 128;
const int STEPUP_TIME = 200;
const int MAX_INVENTORY_ITEMS = 512;				// Solarsplace - 5th July 2013 - Inventory related - Increaced to 512 to expand inventory capacity a lot

const int ARX_FISTS_WEAPON = 0;
const int ARX_MAGIC_WEAPON = 10;					// Solarsplace - 13th May 2010 - The id for the empty magic weapon.
const int ARX_MANA_WEAPON = ARX_MAGIC_WEAPON;		// Solarsplace - 26th May 2010 - This weapon will need to be a weapon that uses mana in order to use this as a guage for the mana hud item.
const int ARX_MANA_TYPE = 1;						// Solarsplace - 2nd June 2010 - See entityDef ammo_types - "ammo_mana" "1"
const int ARX_MANA_BASE_COST = 10;					// Solarsplace - 6th June 2010 - Every spell consumes at least 10 mana. Add additional cost in arx_magic_spells.def
const int ARX_INVIS_TIME = 60 * 1000;				// Solarsplace - 6th June 2010 - Time invis magic lasts 
const int ARX_TELEKENESIS_TIME = 30 * 1000;			// Solarsplace - 15th June 2012 - Time telekenesis magic lasts 
const int ARX_LEVITATE_TIME = 30 * 1000;			// Solarsplace - 3rd July 2014 - Time levitate magic lasts 

const int ARX_MAX_PLAYER_LEVELS = 14;

const int ARX_DEFAULT_BLACKSMITH_SKILL = 94;

const int ARX_SKILL_BASE_VALUE = 10;

//*****************************************************************
//*****************************************************************
// Solarsplace 2nd Sep 2010 - Arx - Level transition related
const int ARX_LVL_MAPNAME = 1;
const int ARX_LVL_RECORDTYPE = 2;
const int ARX_LVL_ENTNAME = 3;
const int ARX_LVL_ENTCLASSNAME = 3;
const int ARX_LVL_ENTPROPERTY = 4;

const idStr ARX_REC_SEP = "<@@@ARX@@@>";
const idStr ARX_REC_NEW = "ARX_ENTITY_NEW";
const idStr ARX_REC_CHANGED = "ARX_ENTITY_CHANGED";

const idStr ARX_PROP_ORIGIN = "ARX_ORIGIN";
const idStr ARX_PROP_AXIS = "ARX_AXIS";
const idStr ARX_PROP_HIDDEN = "ARX_HIDDEN";
const idStr ARX_PROP_INV_NAME = "ARX_INV_NAME";
const idStr ARX_PROP_INV_HEALTH = "ARX_INV_HEALTH";
const idStr ARX_PROP_INV_HEALTH_MAX = "ARX_INV_HEALTH_MAX";
const idStr ARX_PROP_LOCKED = "ARX_LOCKED";
const idStr ARX_PROP_CLASSNAME = "ARX_CLASSNAME";

const idStr ARX_PROP_MAPENTRYPOINT = "ARX_MAPENTRYPOINT";
const idStr ARX_PROP_MAP_ANY = "ARX_MAP_ANY";
const idStr ARX_PROP_ENT_ANY = "ARX_ENT_ANY";

// ***** MIRRORED IN arx_quest_base.script *****
const idStr ARX_CHAR_QUEST_WINDOW = "ARX_C_Q_WINDOW";
const idStr ARX_QUEST_STATE = "ARX_QUEST_STATE"; // Must mirror in scripts too.

const int ARX_ENTITY_STATE_NOTSTORED = 0; // This is the default return value if no persistent value has been stored.
const int ARX_ENTITY_STATE_USED = 1;
const int ARX_ENTITY_STATE_UNLOCKED = 666;
const int ARX_ENTITY_STATE_LOCKED = 999;
const int ARX_ENTITY_STATE_DOUSE = 666;
const int ARX_ENTITY_STATE_IGNITE = 999;

const int ARX_QUEST_STATE_INITIAL = 0;
const int ARX_QUEST_STATE_COMPLETED = 999;

//*****************************************************************
//*****************************************************************

const float ARX_MAX_ITEM_PICKUP_DISTANCE = 92.0f;		// Solarsplace 7th June 2010 - The max trace distance for a pickup item.
const float ARX_MAX_ITEM_PICKUP_DISTANCE_TELE = 364;	// Solarsplace 15th June 2012  - The max trace distance for a pickup item with telekinesis 

//*****************************************************************
//*****************************************************************

idVec3 idPlayer::colorBarTable[ 5 ] = {
	idVec3( 0.25f, 0.25f, 0.25f ),
	idVec3( 1.00f, 0.00f, 0.00f ),
	idVec3( 0.00f, 0.80f, 0.10f ),
	idVec3( 0.20f, 0.50f, 0.80f ),
	idVec3( 1.00f, 0.80f, 0.10f )
};

/*
==============
idInventory::Clear
==============
*/
void idInventory::Clear( void ) {
	maxHealth		= 0;
	weapons			= 0;
	powerups		= 0;
	armor			= 0;
	maxarmor		= 0;
	deplete_armor	= 0;
	deplete_rate	= 0.0f;
	deplete_ammount	= 0;
	nextArmorDepleteTime = 0;

	// ****************************************************
	// ****************************************************
	// ****************************************************

	// Solarsplace - Arx EOS
	money							= 0;
	weaponUniqueName				= "";

	int i;
	for ( i = 0; i < ARX_EQUIPED_ITEMS_MAX; i++ ) {
		arx_equiped_items[ i ] = "";
	}

	arx_snake_weapon				= ARX_MAGIC_WEAPON;

	arx_player_level				= 0;

	arx_player_x_points				= 0;

	arx_attribute_points			= 0;
	arx_skill_points				= 0;

	arx_attr_strength				= 0;
	arx_attr_mental					= 0;
	arx_attr_dexterity				= 0;
	arx_attr_constitution			= 0;

	arx_skill_casting				= 0;
	arx_skill_close_combat			= 0;
	arx_skill_defense				= 0;
	arx_skill_ethereal_link			= 0;
	arx_skill_intuition				= 0;
	arx_skill_intelligence			= 0;
	arx_skill_projectile			= 0;
	arx_skill_stealth				= 0;
	arx_skill_technical				= 0;

	arx_class_armour_points			= 0;
	arx_class_health_points			= 0;
	arx_class_mana_points			= 0;
	arx_class_resistance_to_magic	= 0;
	arx_class_resistance_to_poison	= 0;
	arx_class_damage_points			= 0;
	arx_stat_secrets_found			= 0;

	arx_timer_player_stats_update	= 0;

	// Init to non 0 to prevent being equal to gameLocal time at start.
	arx_timer_player_poison			= -1;
	arx_timer_player_invisible		= -1;
	arx_timer_player_onfire			= -1;
	arx_timer_player_telekinesis	= -1;
	arx_timer_player_levitate		= -1;
	arx_timer_player_warmth			= -1;

	// ****************************************************
	// ****************************************************
	// ****************************************************

	memset( ammo, 0, sizeof( ammo ) );

	ClearPowerUps();

	// set to -1 so that the gun knows to have a full clip the first time we get it and at the start of the level
	memset( clip, -1, sizeof( clip ) );
	
	items.DeleteContents( true );
	memset(pdasViewed, 0, 4 * sizeof( pdasViewed[0] ) );
	pdas.Clear();
	videos.Clear();
	emails.Clear();
	selVideo = 0;
	selEMail = 0;
	selPDA = 0;
	selAudio = 0;
	pdaOpened = false;
	turkeyScore = false;

	levelTriggers.Clear();

	nextItemPickup = 0;
	nextItemNum = 1;
	onePickupTime = 0;
	pickupItemNames.Clear();
	objectiveNames.Clear();

	ammoPredictTime = 0;

	lastGiveTime = 0;

	ammoPulse	= false;
	weaponPulse	= false;
	armorPulse	= false;
}

/*
==============
idInventory::GivePowerUp
==============
*/
void idInventory::GivePowerUp( idPlayer *player, int powerup, int msec ) {
	if ( !msec ) {
		// get the duration from the .def files
		const idDeclEntityDef *def = NULL;
		switch ( powerup ) {
			case BERSERK:
				def = gameLocal.FindEntityDef( "powerup_berserk", false );
				break;
			case INVISIBILITY:
				def = gameLocal.FindEntityDef( "powerup_invisibility", false );
				break;
			case MEGAHEALTH:
				def = gameLocal.FindEntityDef( "powerup_megahealth", false );
				break;
			case ADRENALINE: 
				def = gameLocal.FindEntityDef( "powerup_adrenaline", false );
				break;
		}
		assert( def );
		msec = def->dict.GetInt( "time" ) * 1000;
	}
	powerups |= 1 << powerup;
	powerupEndTime[ powerup ] = gameLocal.time + msec;
}

/*
==============
idInventory::ClearPowerUps
==============
*/
void idInventory::ClearPowerUps( void ) {
	int i;
	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		powerupEndTime[ i ] = 0;
	}
	powerups = 0;
}

/*
==============
idInventory::GetPersistantData
==============
*/
void idInventory::GetPersistantData( idDict &dict ) {
	int		i;
	int		num;
	idDict	*item;
	idStr	key;
	const idKeyValue *kv;
	const char *name;

	// armor
	dict.SetInt( "armor", armor );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	// Solarsplace - Arx EOS
	dict.SetInt( "money", money );
	dict.Set( "weaponUniqueName", weaponUniqueName );

	/*
	for ( i = 0; i < arx_equipt_items.Num(); i++ ) {
		sprintf( key, "arx_equipt_items_%i", i );
		dict.Set( key, arx_equipt_items[ i ] );
	}
	dict.SetInt( "arx_equipt_items_num", arx_equipt_items.Num() );
	*/

	dict.SetInt( "arx_snake_weapon", arx_snake_weapon );

	dict.SetInt( "arx_player_level", arx_player_level );

	dict.SetInt( "arx_player_x_points", arx_player_x_points );
	
	dict.SetInt( "arx_attribute_points", arx_attribute_points );
	dict.SetInt( "arx_skill_points", arx_skill_points );

	dict.SetInt( "arx_attr_strength", arx_attr_strength );
	dict.SetInt( "arx_attr_mental", arx_attr_mental );
	dict.SetInt( "arx_attr_dexterity", arx_attr_dexterity );
	dict.SetInt( "arx_attr_constitution", arx_attr_constitution );

	dict.SetInt( "arx_skill_casting", arx_skill_casting );
	dict.SetInt( "arx_skill_close_combat", arx_skill_close_combat );
	dict.SetInt( "arx_skill_defense", arx_skill_defense );
	dict.SetInt( "arx_skill_ethereal_link", arx_skill_ethereal_link );
	dict.SetInt( "arx_skill_intuition", arx_skill_intuition );
	dict.SetInt( "arx_skill_intelligence", arx_skill_intelligence );
	dict.SetInt( "arx_skill_projectile", arx_skill_projectile );
	dict.SetInt( "arx_skill_stealth", arx_skill_stealth );
	dict.SetInt( "arx_skill_technical", arx_skill_technical );

	dict.SetInt( "arx_class_armour_points", arx_class_armour_points );
	dict.SetInt( "arx_class_health_points", arx_class_health_points );
	dict.SetInt( "arx_class_mana_points", arx_class_mana_points );
	dict.SetInt( "arx_class_resistance_to_magic", arx_class_resistance_to_magic );
	dict.SetInt( "arx_class_resistance_to_poison", arx_class_resistance_to_poison );
	dict.SetInt( "arx_class_damage_points", arx_class_damage_points );
    dict.SetInt( "arx_stat_secrets_found", arx_stat_secrets_found );

	dict.SetInt( "arx_timer_player_stats_update", arx_timer_player_stats_update );
	dict.SetInt( "arx_timer_player_poison", arx_timer_player_poison );
	dict.SetInt( "arx_timer_player_invisible", arx_timer_player_invisible );
	dict.SetInt( "arx_timer_player_onfire", arx_timer_player_onfire );
	dict.SetInt( "arx_timer_player_telekinesis", arx_timer_player_telekinesis );
	dict.SetInt( "arx_timer_player_levitate", arx_timer_player_levitate );
	dict.SetInt( "arx_timer_player_warmth", arx_timer_player_warmth );

	// ****************************************************
	// ****************************************************
	// ****************************************************


    // don't bother with powerups, maxhealth, maxarmor, or the clip

	// ammo
	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		name = idWeapon::GetAmmoNameForNum( ( ammo_t )i );
		if ( name ) {
			dict.SetInt( name, ammo[ i ] );
		}
	}

	// items
	num = 0;
	for( i = 0; i < items.Num(); i++ ) {
		item = items[ i ];

		// copy all keys with "inv_"
		kv = item->MatchPrefix( "inv_" );
		if ( kv ) {
			while( kv ) {
				sprintf( key, "item_%i %s", num, kv->GetKey().c_str() );
				dict.Set( key, kv->GetValue() );

				//gameLocal.Printf("kv = (%s) - (%s)\n", key.c_str(), kv->GetValue().c_str() );

				kv = item->MatchPrefix( "inv_", kv );

			}
			num++;
		}

		/*
		// Solarsplace - TEST - Level transition related
		kv = item->MatchPrefix( "classname" );
		if ( kv ) {
			while( kv ) {
				sprintf( key, "item_%i %s", num, kv->GetKey().c_str() );
				dict.Set( key, kv->GetValue() );
				kv = item->MatchPrefix( "inv_", kv );
			}
		}
		*/

	}
	dict.SetInt( "items", num );

	// pdas viewed
	for ( i = 0; i < 4; i++ ) {
		dict.SetInt( va("pdasViewed_%i", i), pdasViewed[i] );
	}

	dict.SetInt( "selPDA", selPDA );
	dict.SetInt( "selVideo", selVideo );
	dict.SetInt( "selEmail", selEMail );
	dict.SetInt( "selAudio", selAudio );
	dict.SetInt( "pdaOpened", pdaOpened );
	dict.SetInt( "turkeyScore", turkeyScore );

	// pdas
	for ( i = 0; i < pdas.Num(); i++ ) {
		sprintf( key, "pda_%i", i );
		dict.Set( key, pdas[ i ] );
	}
	dict.SetInt( "pdas", pdas.Num() );

	// video cds
	for ( i = 0; i < videos.Num(); i++ ) {
		sprintf( key, "video_%i", i );
		dict.Set( key, videos[ i ].c_str() );
	}
	dict.SetInt( "videos", videos.Num() );

	// emails
	for ( i = 0; i < emails.Num(); i++ ) {
		sprintf( key, "email_%i", i );
		dict.Set( key, emails[ i ].c_str() );
	}
	dict.SetInt( "emails", emails.Num() );

	// weapons
	dict.SetInt( "weapon_bits", weapons );

	dict.SetInt( "levelTriggers", levelTriggers.Num() );
	for ( i = 0; i < levelTriggers.Num(); i++ ) {
		sprintf( key, "levelTrigger_Level_%i", i );
		dict.Set( key, levelTriggers[i].levelName );
		sprintf( key, "levelTrigger_Trigger_%i", i );
		dict.Set( key, levelTriggers[i].triggerName );
	}
}

/*
==============
idInventory::RestoreInventory
==============
*/
void idInventory::RestoreInventory( idPlayer *owner, const idDict &dict ) {
	int			i;
	int			num;
	idDict		*item;
	idStr		key;
	idStr		itemname;
	const idKeyValue *kv;
	const char	*name;

	Clear();

	// health/armor
	maxHealth		= dict.GetInt( "maxhealth", "100" );
	armor			= dict.GetInt( "armor", "50" );
	maxarmor		= dict.GetInt( "maxarmor", "100" );
	deplete_armor	= dict.GetInt( "deplete_armor", "0" );
	deplete_rate	= dict.GetFloat( "deplete_rate", "2.0" );
	deplete_ammount	= dict.GetInt( "deplete_ammount", "1" );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	// Solarsplace - Arx EOS
	money							= dict.GetInt( "money", "0" );
	weaponUniqueName				= dict.GetString( "weaponUniqueName", "" );

	/*
	num = dict.GetInt( "arx_equipt_items_num" );
	arx_equipt_items.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "arx_equipt_items_%i", i );
		arx_equipt_items[i] = dict.GetString( itemname, "" );
	}
	*/

	arx_snake_weapon				= dict.GetInt( "arx_snake_weapon", idStr( ARX_MAGIC_WEAPON ) );

	arx_player_level				= dict.GetInt( "arx_player_level", "0" );

	arx_player_x_points				= dict.GetInt( "arx_player_x_points", "0" );

	arx_attribute_points			= dict.GetInt( "arx_attribute_points", "0" );
	arx_skill_points				= dict.GetInt( "arx_skill_points", "0" );

	arx_attr_strength				= dict.GetInt( "arx_attr_strength", "0" );
	arx_attr_mental					= dict.GetInt( "arx_attr_mental", "0" );
	arx_attr_dexterity				= dict.GetInt( "arx_attr_dexterity", "0" );
	arx_attr_constitution			= dict.GetInt( "arx_attr_constitution", "0" );

	arx_skill_casting				= dict.GetInt( "arx_skill_casting", "0" );
	arx_skill_close_combat			= dict.GetInt( "arx_skill_close_combat", "0" );
	arx_skill_defense				= dict.GetInt( "arx_skill_defense", "0" );
	arx_skill_ethereal_link			= dict.GetInt( "arx_skill_ethereal_link", "0" );
	arx_skill_intuition				= dict.GetInt( "arx_skill_intuition", "0" );
	arx_skill_intelligence			= dict.GetInt( "arx_skill_intelligence", "0" );
	arx_skill_projectile			= dict.GetInt( "arx_skill_projectile", "0" );
	arx_skill_stealth				= dict.GetInt( "arx_skill_stealth", "0" );
	arx_skill_technical				= dict.GetInt( "arx_skill_technical", "0" );

	arx_class_armour_points			= dict.GetInt( "arx_class_armour_points", "0" );
	arx_class_health_points			= dict.GetInt( "arx_class_health_points", "0" );
	arx_class_mana_points			= dict.GetInt( "arx_class_mana_points", "0" );
	arx_class_resistance_to_magic	= dict.GetInt( "arx_class_resistance_to_magic", "0" );
	arx_class_resistance_to_poison	= dict.GetInt( "arx_class_resistance_to_poison", "0" );
	arx_class_damage_points			= dict.GetInt( "arx_class_damage_points", "0" );
	arx_stat_secrets_found			= dict.GetInt( "arx_stat_secrets_found", "0" );

	arx_timer_player_stats_update	= dict.GetInt( "arx_timer_player_stats_update", "0" );

	// Init to non 0 to prevent being equal to gameLocal time at start.
	arx_timer_player_poison			= dict.GetInt( "arx_timer_player_poison", "-1" );
	arx_timer_player_invisible		= dict.GetInt( "arx_timer_player_invisible", "-1" );
	arx_timer_player_onfire			= dict.GetInt( "arx_timer_player_onfire", "-1" );
	arx_timer_player_telekinesis	= dict.GetInt( "arx_timer_player_telekinesis", "-1" );
	arx_timer_player_levitate		= dict.GetInt( "arx_timer_player_levitate", "-1" );
	arx_timer_player_warmth			= dict.GetInt( "arx_timer_player_warmth", "-1" );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	// the clip and powerups aren't restored

	// ammo
	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		name = idWeapon::GetAmmoNameForNum( ( ammo_t )i );
		if ( name ) {
			ammo[ i ] = dict.GetInt( name );
		}
	}

	// items
	num = dict.GetInt( "items" );
	items.SetNum( num );
	for( i = 0; i < num; i++ ) {
		item = new idDict();
		items[ i ] = item;
		sprintf( itemname, "item_%i ", i );
		kv = dict.MatchPrefix( itemname );
		while( kv ) {
			key = kv->GetKey();
			key.Strip( itemname );
			item->Set( key, kv->GetValue() );
			kv = dict.MatchPrefix( itemname, kv );
		}
	}

	// pdas viewed
	for ( i = 0; i < 4; i++ ) {
		pdasViewed[i] = dict.GetInt(va("pdasViewed_%i", i));
	}

	selPDA = dict.GetInt( "selPDA" );
	selEMail = dict.GetInt( "selEmail" );
	selVideo = dict.GetInt( "selVideo" );
	selAudio = dict.GetInt( "selAudio" );
	pdaOpened = dict.GetBool( "pdaOpened" );
	turkeyScore = dict.GetBool( "turkeyScore" );

	// pdas
	num = dict.GetInt( "pdas" );
	pdas.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "pda_%i", i );
		pdas[i] = dict.GetString( itemname, "default" );
	}

	// videos
	num = dict.GetInt( "videos" );
	videos.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "video_%i", i );
		videos[i] = dict.GetString( itemname, "default" );
	}

	// emails
	num = dict.GetInt( "emails" );
	emails.SetNum( num );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "email_%i", i );
		emails[i] = dict.GetString( itemname, "default" );
	}

	// weapons are stored as a number for persistant data, but as strings in the entityDef
	weapons	= dict.GetInt( "weapon_bits", "0" );

#ifdef ID_DEMO_BUILD
		Give( owner, dict, "weapon", dict.GetString( "weapon" ), NULL, false );
#else
	if ( g_skill.GetInteger() >= 3 ) {
		Give( owner, dict, "weapon", dict.GetString( "weapon_nightmare" ), NULL, false );
	} else {
		Give( owner, dict, "weapon", dict.GetString( "weapon" ), NULL, false );
	}
#endif

	num = dict.GetInt( "levelTriggers" );
	for ( i = 0; i < num; i++ ) {
		sprintf( itemname, "levelTrigger_Level_%i", i );
		idLevelTriggerInfo lti;
		lti.levelName = dict.GetString( itemname );
		sprintf( itemname, "levelTrigger_Trigger_%i", i );
		lti.triggerName = dict.GetString( itemname );
		levelTriggers.Append( lti );
	}

}

/*
==============
idInventory::Save
==============
*/
void idInventory::Save( idSaveGame *savefile ) const {
	int i;

	savefile->WriteInt( maxHealth );
	savefile->WriteInt( weapons );
	savefile->WriteInt( powerups );
	savefile->WriteInt( armor );
	savefile->WriteInt( maxarmor );
	savefile->WriteInt( ammoPredictTime );
	savefile->WriteInt( deplete_armor );
	savefile->WriteFloat( deplete_rate );
	savefile->WriteInt( deplete_ammount );
	savefile->WriteInt( nextArmorDepleteTime );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	// Solarsplace - Arx EOS
	savefile->WriteInt( money );
	savefile->WriteString( weaponUniqueName );

	/*
	savefile->WriteInt( arx_equipt_items.Num() );
	for( i = 0; i < arx_equipt_items.Num(); i++ ) {
		savefile->WriteString( arx_equipt_items[ i ].unique_name );
		savefile->WriteInt( arx_equipt_items[ i ].equipment_position );
	}
	*/

	savefile->WriteInt( arx_snake_weapon );

	savefile->WriteInt( arx_player_level );

	savefile->WriteInt( arx_player_x_points );
	
	savefile->WriteInt( arx_attribute_points );
	savefile->WriteInt( arx_skill_points );

	savefile->WriteInt( arx_attr_strength );
	savefile->WriteInt( arx_attr_mental );
	savefile->WriteInt( arx_attr_dexterity );
	savefile->WriteInt( arx_attr_constitution );

	savefile->WriteInt( arx_skill_casting );
	savefile->WriteInt( arx_skill_close_combat );
	savefile->WriteInt( arx_skill_defense );
	savefile->WriteInt( arx_skill_ethereal_link );
	savefile->WriteInt( arx_skill_intuition );
	savefile->WriteInt( arx_skill_intelligence );
	savefile->WriteInt( arx_skill_projectile );
	savefile->WriteInt( arx_skill_stealth );
	savefile->WriteInt( arx_skill_technical );

	savefile->WriteInt( arx_class_armour_points );
	savefile->WriteInt( arx_class_health_points );
	savefile->WriteInt( arx_class_mana_points );
	savefile->WriteInt( arx_class_resistance_to_magic );
	savefile->WriteInt( arx_class_resistance_to_poison );
	savefile->WriteInt( arx_class_damage_points );
	savefile->WriteInt( arx_stat_secrets_found );

	savefile->WriteInt( arx_timer_player_stats_update );
	savefile->WriteInt( arx_timer_player_poison );
	savefile->WriteInt( arx_timer_player_invisible );
	savefile->WriteInt( arx_timer_player_onfire );
	savefile->WriteInt( arx_timer_player_telekinesis );
	savefile->WriteInt( arx_timer_player_levitate );
	savefile->WriteInt( arx_timer_player_warmth );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		savefile->WriteInt( ammo[ i ] );
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		savefile->WriteInt( clip[ i ] );
	}
	for( i = 0; i < MAX_POWERUPS; i++ ) {
		savefile->WriteInt( powerupEndTime[ i ] );
	}

	savefile->WriteInt( items.Num() );
	for( i = 0; i < items.Num(); i++ ) {
		savefile->WriteDict( items[ i ] );
	}

	savefile->WriteInt( pdasViewed[0] );
	savefile->WriteInt( pdasViewed[1] );
	savefile->WriteInt( pdasViewed[2] );
	savefile->WriteInt( pdasViewed[3] );
	
	savefile->WriteInt( selPDA );
	savefile->WriteInt( selVideo );
	savefile->WriteInt( selEMail );
	savefile->WriteInt( selAudio );
	savefile->WriteBool( pdaOpened );
	savefile->WriteBool( turkeyScore );

	savefile->WriteInt( pdas.Num() );
	for( i = 0; i < pdas.Num(); i++ ) {
		savefile->WriteString( pdas[ i ] );
	}

	savefile->WriteInt( pdaSecurity.Num() );
	for( i=0; i < pdaSecurity.Num(); i++ ) {
		savefile->WriteString( pdaSecurity[ i ] );
	}

	savefile->WriteInt( videos.Num() );
	for( i = 0; i < videos.Num(); i++ ) {
		savefile->WriteString( videos[ i ] );
	}

	savefile->WriteInt( emails.Num() );
	for ( i = 0; i < emails.Num(); i++ ) {
		savefile->WriteString( emails[ i ] );
	}

	savefile->WriteInt( nextItemPickup );
	savefile->WriteInt( nextItemNum );
	savefile->WriteInt( onePickupTime );

	savefile->WriteInt( pickupItemNames.Num() );
	for( i = 0; i < pickupItemNames.Num(); i++ ) {
		savefile->WriteString( pickupItemNames[i].icon );
		savefile->WriteString( pickupItemNames[i].name );
	}

	savefile->WriteInt( objectiveNames.Num() );
	for( i = 0; i < objectiveNames.Num(); i++ ) {
		savefile->WriteString( objectiveNames[i].screenshot );
		savefile->WriteString( objectiveNames[i].text );
		savefile->WriteString( objectiveNames[i].title );
	}

	savefile->WriteInt( levelTriggers.Num() );
	for ( i = 0; i < levelTriggers.Num(); i++ ) {
		savefile->WriteString( levelTriggers[i].levelName );
		savefile->WriteString( levelTriggers[i].triggerName );
	}

	savefile->WriteBool( ammoPulse );
	savefile->WriteBool( weaponPulse );
	savefile->WriteBool( armorPulse );

	savefile->WriteInt( lastGiveTime );
}

/*
==============
idInventory::Restore
==============
*/
void idInventory::Restore( idRestoreGame *savefile ) {
	int i, num;

	savefile->ReadInt( maxHealth );
	savefile->ReadInt( weapons );
	savefile->ReadInt( powerups );
	savefile->ReadInt( armor );
	savefile->ReadInt( maxarmor );
	savefile->ReadInt( ammoPredictTime );
	savefile->ReadInt( deplete_armor );
	savefile->ReadFloat( deplete_rate );
	savefile->ReadInt( deplete_ammount );
	savefile->ReadInt( nextArmorDepleteTime );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	// Solarsplace - Arx EOS
	savefile->ReadInt( money );
	savefile->ReadString( weaponUniqueName );

	/*
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr equipedItem;
		savefile->ReadString( equipedItem );
		arx_equipt_items.Append( equipedItem );
	}
	*/

	savefile->ReadInt( arx_snake_weapon );

	savefile->ReadInt( arx_player_level );

	savefile->ReadInt( arx_player_x_points );

	savefile->ReadInt( arx_attribute_points );
	savefile->ReadInt( arx_skill_points );

	savefile->ReadInt( arx_attr_strength );
	savefile->ReadInt( arx_attr_mental );
	savefile->ReadInt( arx_attr_dexterity );
	savefile->ReadInt( arx_attr_constitution );

	savefile->ReadInt( arx_skill_casting );
	savefile->ReadInt( arx_skill_close_combat );
	savefile->ReadInt( arx_skill_defense );
	savefile->ReadInt( arx_skill_ethereal_link );
	savefile->ReadInt( arx_skill_intuition );
	savefile->ReadInt( arx_skill_intelligence );
	savefile->ReadInt( arx_skill_projectile );
	savefile->ReadInt( arx_skill_stealth );
	savefile->ReadInt( arx_skill_technical );

	savefile->ReadInt( arx_class_armour_points );
	savefile->ReadInt( arx_class_health_points );
	savefile->ReadInt( arx_class_mana_points );
	savefile->ReadInt( arx_class_resistance_to_magic );
	savefile->ReadInt( arx_class_resistance_to_poison );
	savefile->ReadInt( arx_class_damage_points );
	savefile->ReadInt( arx_stat_secrets_found );

	savefile->ReadInt( arx_timer_player_stats_update );
	savefile->ReadInt( arx_timer_player_poison );
	savefile->ReadInt( arx_timer_player_invisible );
	savefile->ReadInt( arx_timer_player_onfire );
	savefile->ReadInt( arx_timer_player_telekinesis );
	savefile->ReadInt( arx_timer_player_levitate );
	savefile->ReadInt( arx_timer_player_warmth );

	// ****************************************************
	// ****************************************************
	// ****************************************************

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		savefile->ReadInt( ammo[ i ] );
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		savefile->ReadInt( clip[ i ] );
	}
	for( i = 0; i < MAX_POWERUPS; i++ ) {
		savefile->ReadInt( powerupEndTime[ i ] );
	}

	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idDict *itemdict = new idDict;

		savefile->ReadDict( itemdict );
		items.Append( itemdict );
	}

	// pdas
	savefile->ReadInt( pdasViewed[0] );
	savefile->ReadInt( pdasViewed[1] );
	savefile->ReadInt( pdasViewed[2] );
	savefile->ReadInt( pdasViewed[3] );
	
	savefile->ReadInt( selPDA );
	savefile->ReadInt( selVideo );
	savefile->ReadInt( selEMail );
	savefile->ReadInt( selAudio );
	savefile->ReadBool( pdaOpened );
	savefile->ReadBool( turkeyScore );

	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr strPda;
		savefile->ReadString( strPda );
		pdas.Append( strPda );
	}

	// pda security clearances
	savefile->ReadInt( num );
	for ( i = 0; i < num; i++ ) {
		idStr invName;
		savefile->ReadString( invName );
		pdaSecurity.Append( invName );
	}

	// videos
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr strVideo;
		savefile->ReadString( strVideo );
		videos.Append( strVideo );
	}

	// email
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idStr strEmail;
		savefile->ReadString( strEmail );
		emails.Append( strEmail );
	}

	savefile->ReadInt( nextItemPickup );
	savefile->ReadInt( nextItemNum );
	savefile->ReadInt( onePickupTime );
	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idItemInfo info;

		savefile->ReadString( info.icon );
		savefile->ReadString( info.name );

		pickupItemNames.Append( info );
	}

	savefile->ReadInt( num );
	for( i = 0; i < num; i++ ) {
		idObjectiveInfo obj;

		savefile->ReadString( obj.screenshot );
		savefile->ReadString( obj.text );
		savefile->ReadString( obj.title );

		objectiveNames.Append( obj );
	}

	savefile->ReadInt( num );
	for ( i = 0; i < num; i++ ) {
		idLevelTriggerInfo lti;
		savefile->ReadString( lti.levelName );
		savefile->ReadString( lti.triggerName );
		levelTriggers.Append( lti );
	}

	savefile->ReadBool( ammoPulse );
	savefile->ReadBool( weaponPulse );
	savefile->ReadBool( armorPulse );

	savefile->ReadInt( lastGiveTime );
}

/*
==============
idInventory::AmmoIndexForAmmoClass
==============
*/
ammo_t idInventory::AmmoIndexForAmmoClass( const char *ammo_classname ) const {
	return idWeapon::GetAmmoNumForName( ammo_classname );
}

/*
==============
idInventory::AmmoIndexForAmmoClass
==============
*/
int idInventory::MaxAmmoForAmmoClass( idPlayer *owner, const char *ammo_classname ) const {
	return owner->spawnArgs.GetInt( va( "max_%s", ammo_classname ), "0" );
}

/*
==============
idInventory::AmmoPickupNameForIndex
==============
*/
const char *idInventory::AmmoPickupNameForIndex( ammo_t ammonum ) const {
	return idWeapon::GetAmmoPickupNameForNum( ammonum );
}

/*
==============
idInventory::WeaponIndexForAmmoClass
mapping could be prepared in the constructor
==============
*/
int idInventory::WeaponIndexForAmmoClass( const idDict & spawnArgs, const char *ammo_classname ) const {
	int i;
	const char *weapon_classname;
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if ( !weapon_classname ) {
			continue;
		}
		const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname, false );
		if ( !decl ) {
			continue;
		}
		if ( !idStr::Icmp( ammo_classname, decl->dict.GetString( "ammoType" ) ) ) {
			return i;
		}
	}
	return -1;
}

/*
==============
idInventory::AmmoIndexForWeaponClass
==============
*/
ammo_t idInventory::AmmoIndexForWeaponClass( const char *weapon_classname, int *ammoRequired ) {
	const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname, false );
	if ( !decl ) {
		gameLocal.Error( "Unknown weapon in decl '%s'", weapon_classname );
	}
	if ( ammoRequired ) {
		*ammoRequired = decl->dict.GetInt( "ammoRequired" );
	}
	ammo_t ammo_i = AmmoIndexForAmmoClass( decl->dict.GetString( "ammoType" ) );
	return ammo_i;
}

/*
==============
idInventory::AddPickupName
==============
*/
void idInventory::AddPickupName( const char *name, const char *icon ) {
	int num;

	num = pickupItemNames.Num();
	if ( ( num == 0 ) || ( pickupItemNames[ num - 1 ].name.Icmp( name ) != 0 ) ) {
		idItemInfo &info = pickupItemNames.Alloc();

		if ( idStr::Cmpn( name, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) {
			info.name = common->GetLanguageDict()->GetString( name );
		} else {
			info.name = name;
		}
		info.icon = icon;
	} 
}

/*
==============
idInventory::Give
==============
*/
bool idInventory::Give( idPlayer *owner, const idDict &spawnArgs, const char *statname, const char *value, int *idealWeapon, bool updateHud ) {
	int						i;
	const char				*pos;
	const char				*end;
	int						len;
	idStr					weaponString;
	int						max;
	const idDeclEntityDef	*weaponDecl;
	bool					tookWeapon;
	int						amount;
	idItemInfo				info;
	const char				*name;

	if ( !idStr::Icmpn( statname, "ammo_", 5 ) ) {
		i = AmmoIndexForAmmoClass( statname );
		max = MaxAmmoForAmmoClass( owner, statname );
		if ( ammo[ i ] >= max ) {
			return false;
		}
		amount = atoi( value );
		if ( amount ) {			
			ammo[ i ] += amount;
			if ( ( max > 0 ) && ( ammo[ i ] > max ) ) {
				ammo[ i ] = max;
			}
			ammoPulse = true;

			name = AmmoPickupNameForIndex( i );
			if ( idStr::Length( name ) ) {
				AddPickupName( name, "" );
			}
		}
	} else if ( !idStr::Icmp( statname, "armor" ) ) {
		if ( armor >= maxarmor ) {
			return false;	// can't hold any more, so leave the item
		}
		amount = atoi( value );
		if ( amount ) {
			armor += amount;
			if ( armor > maxarmor ) {
				armor = maxarmor;
			}
			nextArmorDepleteTime = 0;
			armorPulse = true;
		}
	} else if ( idStr::FindText( statname, "inclip_" ) == 0 ) {
		i = WeaponIndexForAmmoClass( spawnArgs, statname + 7 );
		if ( i != -1 ) {
			// set, don't add. not going over the clip size limit.
			clip[ i ] = atoi( value );
		}
	} else if ( !idStr::Icmp( statname, "berserk" ) ) {
		GivePowerUp( owner, BERSERK, SEC2MS( atof( value ) ) );
	} else if ( !idStr::Icmp( statname, "mega" ) ) {
		GivePowerUp( owner, MEGAHEALTH, SEC2MS( atof( value ) ) );
	} else if ( !idStr::Icmp( statname, "weapon" ) ) {
		tookWeapon = false;
		for( pos = value; pos != NULL; pos = end ) {
			end = strchr( pos, ',' );
			if ( end ) {
				len = end - pos;
				end++;
			} else {
				len = strlen( pos );
			}

			idStr weaponName( pos, 0, len );

			// find the number of the matching weapon name
			for( i = 0; i < MAX_WEAPONS; i++ ) {
				if ( weaponName == spawnArgs.GetString( va( "def_weapon%d", i ) ) ) {
					break;
				}
			}

			if ( i >= MAX_WEAPONS ) {
				gameLocal.Error( "Unknown weapon '%s'", weaponName.c_str() );
			}

			// cache the media for this weapon
			weaponDecl = gameLocal.FindEntityDef( weaponName, false );

			// don't pickup "no ammo" weapon types twice
			// not for D3 SP .. there is only one case in the game where you can get a no ammo
			// weapon when you might already have it, in that case it is more conistent to pick it up
			if ( gameLocal.isMultiplayer && weaponDecl && ( weapons & ( 1 << i ) ) && !weaponDecl->dict.GetInt( "ammoRequired" ) ) {
				continue;
			}

			if ( !gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || ( weaponName == "weapon_fists" ) || ( weaponName == "weapon_soulcube" ) ) {
				if ( ( weapons & ( 1 << i ) ) == 0 || gameLocal.isMultiplayer ) {
					if ( owner->GetUserInfo()->GetBool( "ui_autoSwitch" ) && idealWeapon ) {
						assert( !gameLocal.isClient );
						*idealWeapon = i;
					} 
					if ( owner->hud && updateHud && lastGiveTime + 1000 < gameLocal.time ) {
						owner->hud->SetStateInt( "newWeapon", i );
						owner->hud->HandleNamedEvent( "newWeapon" );
						lastGiveTime = gameLocal.time;
					}
					weaponPulse = true;
					weapons |= ( 1 << i );
					tookWeapon = true;
				}
			}
		}
		return tookWeapon;
	} else if ( !idStr::Icmp( statname, "item" ) || !idStr::Icmp( statname, "icon" ) || !idStr::Icmp( statname, "name" ) ) {
		// ignore these as they're handled elsewhere
		return false;

	// Solarsplace - Arx End Of Sun - Inventory related. Note the inv_ is stripped
	} else if (!idStr::Icmp(statname, "arx_inventory_item" ) 
			|| !idStr::Icmp( statname, "arx_item_attribute" )
			|| !idStr::Icmp( statname, "shop_item_value" )
			|| !idStr::Icmp( statname, "classname" )
			|| !idStr::Icmp( statname, "weapon_def" ))
	{
		// ignore these as they are at this time not important here, but are necessary to enable the item to be re-picked up.
		return false;

	} else {
		// unknown item
		gameLocal.Warning( "Unknown stat '%s' added to player's inventory", statname );
		return false;
	}

	return true;
}

/*
===============
idInventoy::Drop
===============
*/
void idInventory::Drop( const idDict &spawnArgs, const char *weapon_classname, int weapon_index ) {
	// remove the weapon bit
	// also remove the ammo associated with the weapon as we pushed it in the item
	assert( weapon_index != -1 || weapon_classname );
	if ( weapon_index == -1 ) {
		for( weapon_index = 0; weapon_index < MAX_WEAPONS; weapon_index++ ) {
			if ( !idStr::Icmp( weapon_classname, spawnArgs.GetString( va( "def_weapon%d", weapon_index ) ) ) ) {
				break;
			}
		}
		if ( weapon_index >= MAX_WEAPONS ) {
			gameLocal.Error( "Unknown weapon '%s'", weapon_classname );
		}
	} else if ( !weapon_classname ) {
		weapon_classname = spawnArgs.GetString( va( "def_weapon%d", weapon_index ) );
	}
	weapons &= ( 0xffffffff ^ ( 1 << weapon_index ) );
	ammo_t ammo_i = AmmoIndexForWeaponClass( weapon_classname, NULL );
	if ( ammo_i ) {
		clip[ weapon_index ] = -1;
		ammo[ ammo_i ] = 0;
	}
}

/*
===============
idInventory::HasAmmo
===============
*/
int idInventory::HasAmmo( ammo_t type, int amount ) {
	if ( ( type == 0 ) || !amount ) {
		// always allow weapons that don't use ammo to fire
		return -1;
	}

	// check if we have infinite ammo
	if ( ammo[ type ] < 0 ) {
		return -1;
	}

	// return how many shots we can fire
	return ammo[ type ] / amount;
}

/*
===============
idInventory::HasAmmo
===============
*/
int idInventory::HasAmmo( const char *weapon_classname ) {
	int ammoRequired;
	ammo_t ammo_i = AmmoIndexForWeaponClass( weapon_classname, &ammoRequired );
	return HasAmmo( ammo_i, ammoRequired );
}

/*
===============
idInventory::UseAmmo
===============
*/
bool idInventory::UseAmmo( ammo_t type, int amount ) {
	if ( !HasAmmo( type, amount ) ) {
		return false;
	}

	// take an ammo away if not infinite
	if ( ammo[ type ] >= 0 ) {
		ammo[ type ] -= amount;
		ammoPredictTime = gameLocal.time; // mp client: we predict this. mark time so we're not confused by snapshots
	}

	return true;
}

/*
===============
idInventory::UpdateArmor
===============
*/
void idInventory::UpdateArmor( void ) {
	if ( deplete_armor != 0.0f && deplete_armor < armor ) {
		if ( !nextArmorDepleteTime ) {
			nextArmorDepleteTime = gameLocal.time + deplete_rate * 1000;
		} else if ( gameLocal.time > nextArmorDepleteTime ) {
			armor -= deplete_ammount;
			if ( armor < deplete_armor ) {
				armor = deplete_armor;
			}
			nextArmorDepleteTime = gameLocal.time + deplete_rate * 1000;
		}
	}
}

/*
==============
idPlayer::idPlayer
==============
*/
idPlayer::idPlayer() {

	// *********************************************************************************
	// *********************************************************************************
	// *********************************************************************************
	// Solarsplace - Arx End Of Sun

	magicWand				= NULL;					// Spell casting related
	magicWandTrail			= NULL;					// Spell casting related
	magicModeActive			= false;				// Spell casting related
	lastMagicModeActive		= false;				// Spell casting related
	magicAttackInProgress	= false;				// Spell casting related
	magicDoingPreCastSpellProjectile	= false;	// Spell casting related			

	invItemGroupCount			= new idDict();
	invItemGroupPointer			= new idDict();

	waterScreenFinishTime		= 0;
	playerUnderWater			= false;

	lastPlayerAlertOrigin		= vec3_zero;

	// *********************************************************************************
	// *********************************************************************************
	// *********************************************************************************


	memset( &usercmd, 0, sizeof( usercmd ) );

	noclip					= false;
#ifdef _DT // levitate spell
	levitate				= false;
#endif
	godmode					= false;

	spawnAnglesSet			= false;
	spawnAngles				= ang_zero;
	viewAngles				= ang_zero;
	cmdAngles				= ang_zero;

	oldButtons				= 0;
	buttonMask				= 0;
	oldFlags				= 0;

	lastHitTime				= 0;
	lastSndHitTime			= 0;
	lastSavingThrowTime		= 0;

	weapon					= NULL;

	hud						= NULL;
	objectiveSystem			= NULL;
	objectiveSystemOpen		= false;

	heartRate				= BASE_HEARTRATE;
	heartInfo.Init( 0, 0, 0, 0 );
	lastHeartAdjust			= 0;
	lastHeartBeat			= 0;
	lastDmgTime				= 0;
	deathClearContentsTime	= 0;
	lastArmorPulse			= -10000;
	stamina					= 0.0f;
	healthPool				= 0.0f;
	nextHealthPulse			= 0;
	healthPulse				= false;
	nextHealthTake			= 0;
	healthTake				= false;

	scoreBoardOpen			= false;
	forceScoreBoard			= false;
	forceRespawn			= false;
	spectating				= false;
	spectator				= 0;
	colorBar				= vec3_zero;
	colorBarIndex			= 0;
	forcedReady				= false;
	wantSpectate			= false;

	lastHitToggle			= false;

	minRespawnTime			= 0;
	maxRespawnTime			= 0;

	firstPersonViewOrigin	= vec3_zero;
	firstPersonViewAxis		= mat3_identity;

#ifdef _DT	// head anim
	firstPersonViewWeaponAxis		= mat3_identity;
#endif

	hipJoint				= INVALID_JOINT;
	chestJoint				= INVALID_JOINT;
	headJoint				= INVALID_JOINT;

	bobFoot					= 0;
	bobFrac					= 0.0f;
	bobfracsin				= 0.0f;
	bobCycle				= 0;
	xyspeed					= 0.0f;
	stepUpTime				= 0;
	stepUpDelta				= 0.0f;
	idealLegsYaw			= 0.0f;
	legsYaw					= 0.0f;
	legsForward				= true;
	oldViewYaw				= 0.0f;
	viewBobAngles			= ang_zero;
	viewBob					= vec3_zero;
	landChange				= 0;
	landTime				= 0;

	currentWeapon			= -1;
	idealWeapon				= -1;
	previousWeapon			= -1;
	weaponSwitchTime		=  0;
	weaponEnabled			= true;
	weapon_soulcube			= -1;
	weapon_pda				= -1;
	weapon_fists			= -1;
	showWeaponViewModel		= true;

	skin					= NULL;
	powerUpSkin				= NULL;
	baseSkinName			= "";

	numProjectilesFired		= 0;
	numProjectileHits		= 0;

	airless					= false;
	airTics					= 0;
	lastAirDamage			= 0;

	gibDeath				= false;
	gibsLaunched			= false;
	gibsDir					= vec3_zero;

	zoomFov.Init( 0, 0, 0, 0 );
	centerView.Init( 0, 0, 0, 0 );
	fxFov					= false;
#ifdef _DT
	isRunning				= false;
#endif

	influenceFov			= 0;
	influenceActive			= 0;
	influenceRadius			= 0.0f;
	influenceEntity			= NULL;
	influenceMaterial		= NULL;
	influenceSkin			= NULL;

	privateCameraView		= NULL;

	memset( loggedViewAngles, 0, sizeof( loggedViewAngles ) );
	memset( loggedAccel, 0, sizeof( loggedAccel ) );
	currentLoggedAccel	= 0;

	focusTime				= 0;
	focusGUIent				= NULL;
	focusUI					= NULL;
	focusCharacter			= NULL;
	talkCursor				= 0;
	focusVehicle			= NULL;
	cursor					= NULL;
	
	oldMouseX				= 0;
	oldMouseY				= 0;

	pdaAudio				= "";
	pdaVideo				= "";
	pdaVideoWave			= "";

	lastDamageDef			= 0;
	lastDamageDir			= vec3_zero;
	lastDamageLocation		= 0;
	smoothedFrame			= 0;
	smoothedOriginUpdated	= false;
	smoothedOrigin			= vec3_zero;
	smoothedAngles			= ang_zero;

	fl.networkSync			= true;

	latchedTeam				= -1;
	doingDeathSkin			= false;
	weaponGone				= false;
	useInitialSpawns		= false;
	tourneyRank				= 0;
	lastSpectateTeleport	= 0;
	tourneyLine				= 0;
	hiddenWeapon			= false;
	tipUp					= false;
	objectiveUp				= false;
	teleportEntity			= NULL;
	teleportKiller			= -1;
	respawning				= false;
	ready					= false;
	leader					= false;
	lastSpectateChange		= 0;
	lastTeleFX				= -9999;
	weaponCatchup			= false;
	lastSnapshotSequence	= 0;

	MPAim					= -1;
	lastMPAim				= -1;
	lastMPAimTime			= 0;
	MPAimFadeTime			= 0;
	MPAimHighlight			= false;

	spawnedTime				= 0;
	lastManOver				= false;
	lastManPlayAgain		= false;
	lastManPresent			= false;

	isTelefragged			= false;

	isLagged				= false;
	isChatting				= false;

	selfSmooth				= false;

	//fSpreadModifier			= 0.0f;		// sikk - Weapon Management: Handling

	//focusMoveableTimer		= 0;		// sikk - Object Manipulation

	//focusItem				= NULL;		// sikk - Manual Item Pickup

	//searchTimer				= 0;		// sikk - Searchable Corpses

	//adrenalineAmount		= 0;		// sikk - Adrenaline Pack System

	bViewModelsModified		= false;	// sikk - Depth Render
	
	//v3CrosshairPos.Zero();				// sikk - Crosshair Positioning

	bAmbientLightOn			= false;	// sikk - Global Ambient Light

	nScreenFrostAlpha		= 0;		// sikk - Screen Frost

// sikk---> Depth of Field PostProcess
	bIsZoomed				= false;
	focusDistance			= 0.0f;
// <---sikk

// sikk---> Health Management System
	/*
	healthPackAmount		= 0;
	healthPackTimer			= 0;
	nextHealthRegen			= 0;
	prevHeatlh				= health;
	*/
// <---sikk

// sikk--> Infrared Goggles/Headlight Mod
	/*
	bIRGogglesOn			= false;
	bHeadlightOn			= false;
	nIRGogglesTime			= 0;
	nHeadlightTime			= 0;
	nBattery				= 100;
	fIntensity				= 1.0f;
	*/
// <---sikk
}

/*
==============
idPlayer::LinkScriptVariables

set up conditions for animation
==============
*/
void idPlayer::LinkScriptVariables( void ) {
	AI_FORWARD.LinkTo(			scriptObject, "AI_FORWARD" );
	AI_BACKWARD.LinkTo(			scriptObject, "AI_BACKWARD" );
	AI_STRAFE_LEFT.LinkTo(		scriptObject, "AI_STRAFE_LEFT" );
	AI_STRAFE_RIGHT.LinkTo(		scriptObject, "AI_STRAFE_RIGHT" );
	AI_ATTACK_HELD.LinkTo(		scriptObject, "AI_ATTACK_HELD" );
	AI_WEAPON_FIRED.LinkTo(		scriptObject, "AI_WEAPON_FIRED" );
	AI_JUMP.LinkTo(				scriptObject, "AI_JUMP" );
	AI_DEAD.LinkTo(				scriptObject, "AI_DEAD" );
	AI_CROUCH.LinkTo(			scriptObject, "AI_CROUCH" );
	AI_ONGROUND.LinkTo(			scriptObject, "AI_ONGROUND" );
	AI_ONLADDER.LinkTo(			scriptObject, "AI_ONLADDER" );
	AI_HARDLANDING.LinkTo(		scriptObject, "AI_HARDLANDING" );
	AI_SOFTLANDING.LinkTo(		scriptObject, "AI_SOFTLANDING" );
	AI_RUN.LinkTo(				scriptObject, "AI_RUN" );
	AI_PAIN.LinkTo(				scriptObject, "AI_PAIN" );
	AI_RELOAD.LinkTo(			scriptObject, "AI_RELOAD" );
	AI_TELEPORT.LinkTo(			scriptObject, "AI_TELEPORT" );
	AI_TURN_LEFT.LinkTo(		scriptObject, "AI_TURN_LEFT" );
	AI_TURN_RIGHT.LinkTo(		scriptObject, "AI_TURN_RIGHT" );
}

/*
==============
idPlayer::SetupWeaponEntity
==============
*/
void idPlayer::SetupWeaponEntity( void ) {
	int w;
	const char *weap;

	if ( weapon.GetEntity() ) {
		// get rid of old weapon
		weapon.GetEntity()->Clear();
		currentWeapon = -1;
	} else if ( !gameLocal.isClient ) {
		weapon = static_cast<idWeapon *>( gameLocal.SpawnEntityType( idWeapon::Type, NULL ) );
		weapon.GetEntity()->SetOwner( this );
		currentWeapon = -1;
	}

	for( w = 0; w < MAX_WEAPONS; w++ ) {
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( weap && *weap ) {
			idWeapon::CacheWeapon( weap );
		}
	}
}

/*
==============
idPlayer::Init
==============
*/
void idPlayer::Init( void ) {

	const char			*value;
	const idKeyValue	*kv;

	noclip					= false;
#ifdef _DT // levitate spell
	levitate				= false;
#endif
	godmode					= false;

	oldButtons				= 0;
	oldFlags				= 0;

	currentWeapon			= -1;
	idealWeapon				= -1;
	previousWeapon			= -1;
	weaponSwitchTime		= 0;
	weaponEnabled			= true;
	weapon_soulcube			= SlotForWeapon( "weapon_soulcube" );
	weapon_pda				= SlotForWeapon( "weapon_pda" );
	weapon_fists			= SlotForWeapon( "weapon_fists" );
	showWeaponViewModel		= GetUserInfo()->GetBool( "ui_showGun" );


	lastDmgTime				= 0;
	lastArmorPulse			= -10000;
	lastHeartAdjust			= 0;
	lastHeartBeat			= 0;
	heartInfo.Init( 0, 0, 0, 0 );

	bobCycle				= 0;
	bobFrac					= 0.0f;
	landChange				= 0;
	landTime				= 0;
	zoomFov.Init( 0, 0, 0, 0 );
	centerView.Init( 0, 0, 0, 0 );
	fxFov					= false;
#ifdef _DT
	isRunning				= false;
#endif

	influenceFov			= 0;
	influenceActive			= 0;
	influenceRadius			= 0.0f;
	influenceEntity			= NULL;
	influenceMaterial		= NULL;
	influenceSkin			= NULL;

	currentLoggedAccel		= 0;

	focusTime				= 0;
	focusGUIent				= NULL;
	focusUI					= NULL;
	focusCharacter			= NULL;
	talkCursor				= 0;
	focusVehicle			= NULL;

	//fSpreadModifier			= 0.0f;		// sikk - Weapon Management: Handling

	//focusMoveableTimer		= 0;		// sikk - Object Manipulation

	//focusItem				= NULL;		// sikk - Manual Item Pickup

	//searchTimer				= 0;		// sikk - Searchable Corpses

	//adrenalineAmount		= 0;		// sikk - Adrenaline Pack System

	bViewModelsModified		= false;	// sikk - Depth Render

	//v3CrosshairPos.Zero();				// sikk - Crosshair Positioning

	bAmbientLightOn			= false;	// sikk - Global Ambient Light

	nScreenFrostAlpha		= 0;		// sikk - Screen Frost

// sikk---> Depth of Field PostProcess
	bIsZoomed				= false;
	focusDistance			= 0.0f;
// <---sikk

	// sikk---> Health Management System
	/*
	healthPackAmount		= 0;
	healthPackTimer			= 0;
	nextHealthRegen			= 0;
	prevHeatlh				= health;
	*/
// <---sikk

// sikk--> Infrared Goggles/Headlight PostProcess
	/*
	bIRGogglesOn			= false;
	bHeadlightOn			= false;
	nIRGogglesTime			= 0;
	nHeadlightTime			= 0;
	nBattery				= 100;
	fIntensity				= 1.0f;
	*/
// <---sikk

	// remove any damage effects
	playerView.ClearEffects();

	// damage values
	fl.takedamage			= true;
	ClearPain();

	// restore persistent data
	RestorePersistantInfo();

	bobCycle		= 0;
	stamina			= 0.0f;
	healthPool		= 0.0f;
	nextHealthPulse = 0;
	healthPulse		= false;
	nextHealthTake	= 0;
	healthTake		= false;

	SetupWeaponEntity();
	currentWeapon = -1;
	previousWeapon = -1;

	heartRate = BASE_HEARTRATE;
	AdjustHeartRate( BASE_HEARTRATE, 0.0f, 0.0f, true );

	idealLegsYaw = 0.0f;
	legsYaw = 0.0f;
	legsForward	= true;
	oldViewYaw = 0.0f;

	// set the pm_ cvars
	if ( !gameLocal.isMultiplayer || gameLocal.isServer ) {
		kv = spawnArgs.MatchPrefix( "pm_", NULL );
		while( kv ) {
			cvarSystem->SetCVarString( kv->GetKey(), kv->GetValue() );
			kv = spawnArgs.MatchPrefix( "pm_", kv );
		}
	}

	// disable stamina on hell levels
	if ( gameLocal.world && gameLocal.world->spawnArgs.GetBool( "no_stamina" ) ) {
		pm_stamina.SetFloat( 0.0f );
	}

	// stamina always initialized to maximum
	stamina = pm_stamina.GetFloat();

	// air always initialized to maximum too
	airTics = pm_airTics.GetFloat();
	airless = false;

	gibDeath = false;
	gibsLaunched = false;
	gibsDir.Zero();

	// set the gravity
	physicsObj.SetGravity( gameLocal.GetGravity() );

	// start out standing
	SetEyeHeight( pm_normalviewheight.GetFloat() );

	stepUpTime = 0;
	stepUpDelta = 0.0f;
	viewBobAngles.Zero();
	viewBob.Zero();

	value = spawnArgs.GetString( "model" );
	if ( value && ( *value != 0 ) ) {
		SetModel( value );
	}

	if ( cursor ) {
		cursor->SetStateInt( "talkcursor", 0 );
		cursor->SetStateString( "combatcursor", "1" );
		cursor->SetStateString( "itemcursor", "0" );
		cursor->SetStateString( "guicursor", "0" );
	}

	if ( ( gameLocal.isMultiplayer || g_testDeath.GetBool() ) && skin ) {
		SetSkin( skin );
		renderEntity.shaderParms[6] = 0.0f;
	} else if ( spawnArgs.GetString( "spawn_skin", NULL, &value ) ) {
		skin = declManager->FindSkin( value );
		SetSkin( skin );
		renderEntity.shaderParms[6] = 0.0f;
	}

	value = spawnArgs.GetString( "bone_hips", "" );
	hipJoint = animator.GetJointHandle( value );
	if ( hipJoint == INVALID_JOINT ) {
		//gameLocal.Error( "Joint '%s' not found for 'bone_hips' on '%s'", value, name.c_str() ); // Arx EOS - Minimal Assets Mod
		gameLocal.Warning( "Joint '%s' not found for 'bone_hips' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_chest", "" );
	chestJoint = animator.GetJointHandle( value );
	if ( chestJoint == INVALID_JOINT ) {
		//gameLocal.Error( "Joint '%s' not found for 'bone_chest' on '%s'", value, name.c_str() ); // Arx EOS - Minimal Assets Mod
		gameLocal.Warning( "Joint '%s' not found for 'bone_chest' on '%s'", value, name.c_str() );
	}

	value = spawnArgs.GetString( "bone_head", "" );
	headJoint = animator.GetJointHandle( value );
	if ( headJoint == INVALID_JOINT ) {
		//gameLocal.Error( "Joint '%s' not found for 'bone_head' on '%s'", value, name.c_str() ); // Arx EOS - Minimal Assets Mod
		gameLocal.Warning( "Joint '%s' not found for 'bone_head' on '%s'", value, name.c_str() );
	}

	// initialize the script variables
	AI_FORWARD		= false;
	AI_BACKWARD		= false;
	AI_STRAFE_LEFT	= false;
	AI_STRAFE_RIGHT	= false;
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED	= false;
	AI_JUMP			= false;
	AI_DEAD			= false;
	AI_CROUCH		= false;
	AI_ONGROUND		= true;
	AI_ONLADDER		= false;
	AI_HARDLANDING	= false;
	AI_SOFTLANDING	= false;
	AI_RUN			= false;
	AI_PAIN			= false;
	AI_RELOAD		= false;
	AI_TELEPORT		= false;
	AI_TURN_LEFT	= false;
	AI_TURN_RIGHT	= false;

	// reset the script object
	ConstructScriptObject();

	// execute the script so the script object's constructor takes effect immediately
	scriptThread->Execute();
	
	forceScoreBoard		= false;
	forcedReady			= false;

	privateCameraView	= NULL;

	lastSpectateChange	= 0;
	lastTeleFX			= -9999;

	hiddenWeapon		= false;
	tipUp				= false;
	objectiveUp			= false;
	teleportEntity		= NULL;
	teleportKiller		= -1;
	leader				= false;

	SetPrivateCameraView( NULL );

	lastSnapshotSequence	= 0;

	MPAim				= -1;
	lastMPAim			= -1;
	lastMPAimTime		= 0;
	MPAimFadeTime		= 0;
	MPAimHighlight		= false;

	if ( hud ) {
		hud->HandleNamedEvent( "aim_clear" );
	}

	cvarSystem->SetCVarBool( "ui_chat", false );
}

/*
==============
idPlayer::Spawn

Prepare any resources used by the player.
==============
*/
void idPlayer::Spawn( void ) {

	// Solarsplace - Start



	// Solarsplace - End

	idStr		temp;
	idBounds	bounds;

	if ( entityNumber >= MAX_CLIENTS ) {
		gameLocal.Error( "entityNum > MAX_CLIENTS for player.  Player may only be spawned with a client." );
	}

	// allow thinking during cinematics
	cinematic = true;

	if ( gameLocal.isMultiplayer ) {
		// always start in spectating state waiting to be spawned in
		// do this before SetClipModel to get the right bounding box
		spectating = true;
	}

	// set our collision model
	physicsObj.SetSelf( this );
	SetClipModel();
	physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );
	physicsObj.SetContents( CONTENTS_BODY );
	physicsObj.SetClipMask( MASK_PLAYERSOLID );
	SetPhysics( &physicsObj );
	InitAASLocation();

	skin = renderEntity.customSkin;

	// only the local player needs guis
	if ( !gameLocal.isMultiplayer || entityNumber == gameLocal.localClientNum ) {

		// load HUD
		if ( gameLocal.isMultiplayer ) {
			hud = uiManager->FindGui( "guis/mphud.gui", true, false, true );
		} else if ( spawnArgs.GetString( "hud", "", temp ) ) {
			hud = uiManager->FindGui( temp, true, false, true );
		}
		if ( hud ) {
			hud->Activate( true, gameLocal.time );
		}

		// load cursor
		if ( spawnArgs.GetString( "cursor", "", temp ) ) {
			cursor = uiManager->FindGui( temp, true, gameLocal.isMultiplayer, gameLocal.isMultiplayer );
		}
		if ( cursor ) {
			cursor->Activate( true, gameLocal.time );
		}

		objectiveSystem = uiManager->FindGui( "guis/arx_journal.gui", true, false, true );
		objectiveSystemOpen = false;

		// Solarsplace 6th Nov 2011 - Shop GUI related
		shoppingSystem = uiManager->FindGui( "guis/arx_inventory_and_shop.gui", true, false, true );
		shoppingSystemOpen = false;

		// Solarsplace 2nd Nov 2011 - NPC GUI related
		conversationSystem = uiManager->FindGui("guis/placeholder.gui", true, false, true );
		conversationSystemOpen = false;
		conversationWindowQuestId = "";

		// Solarsplace 11th June 2010 - Readable related
		readableSystem = uiManager->FindGui( "guis/placeholder.gui", true, false, true );
		readableSystemOpen = false;

		// Solarsplace 11th April 2010 - Inventory related
		inventorySystem = uiManager->FindGui( "guis/arx_inventory.gui", true, false, true );
		inventorySystemOpen = false;

		// Solarsplace 6th May 2010 - Journal related
		// Solarsplace 18th May 2012 - Removed from use ATM
		journalSystem = uiManager->FindGui( "guis/placeholder.gui", true, false, true );
		journalSystemOpen = false;

	}

	SetLastHitTime( 0 );

	// load the armor sound feedback
	declManager->FindSound( "player_sounds_hitArmor" );

	// set up conditions for animation
	LinkScriptVariables();

	animator.RemoveOriginOffset( true );

	// initialize user info related settings
	// on server, we wait for the userinfo broadcast, as this controls when the player is initially spawned in game
	if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
		UserInfoChanged( false );
	}

	// create combat collision hull for exact collision detection
	SetCombatModel();

	// init the damage effects
	playerView.SetPlayerEntity( this );

	// supress model in non-player views, but allow it in mirrors and remote views
	renderEntity.suppressSurfaceInViewID = entityNumber+1;

	// don't project shadow on self or weapon
	renderEntity.noSelfShadow = true;

	idAFAttachment *headEnt = head.GetEntity();
	if ( headEnt ) {
		headEnt->GetRenderEntity()->suppressSurfaceInViewID = entityNumber+1;
		headEnt->GetRenderEntity()->noSelfShadow = true;
	}

	if ( gameLocal.isMultiplayer ) {
		Init();
		Hide();	// properly hidden if starting as a spectator
		if ( !gameLocal.isClient ) {
			// set yourself ready to spawn. idMultiplayerGame will decide when/if appropriate and call SpawnFromSpawnSpot
			SetupWeaponEntity();
			SpawnFromSpawnSpot();
			forceRespawn = true;
			assert( spectating );
		}
	} else {
		SetupWeaponEntity();
		SpawnFromSpawnSpot();
	}

	// trigger playtesting item gives, if we didn't get here from a previous level
	// the devmap key will be set on the first devmap, but cleared on any level
	// transitions
	if ( !gameLocal.isMultiplayer && gameLocal.serverInfo.FindKey( "devmap" ) ) {
		// fire a trigger with the name "devmap"
		idEntity *ent = gameLocal.FindEntity( "devmap" );
		if ( ent ) {
			ent->ActivateTargets( this );
		}
	}
	if ( hud ) {
		// We can spawn with a full soul cube, so we need to make sure the hud knows this
		if ( weapon_soulcube > 0 && ( inventory.weapons & ( 1 << weapon_soulcube ) ) ) {
			int max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
			if ( inventory.ammo[ idWeapon::GetAmmoNumForName( "ammo_souls" ) ] >= max_souls ) {
				hud->HandleNamedEvent( "soulCubeReady" );
			}
		}
		hud->HandleNamedEvent( "itemPickup" );
	}

	if ( GetPDA() ) {
		// Add any emails from the inventory
		for ( int i = 0; i < inventory.emails.Num(); i++ ) {
			GetPDA()->AddEmail( inventory.emails[i] );
		}
		GetPDA()->SetSecurity( common->GetLanguageDict()->GetString( "#str_00066" ) );
	}

	if ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
		hiddenWeapon = true;
		if ( weapon.GetEntity() ) {
			weapon.GetEntity()->LowerWeapon();
		}
		idealWeapon = 0;
	} else {
		hiddenWeapon = false;
	}
	
	if ( hud ) {
		UpdateHudWeapon();
		hud->StateChanged( gameLocal.time );
	}

	tipUp = false;
	objectiveUp = false;

	if ( inventory.levelTriggers.Num() ) {
		PostEventMS( &EV_Player_LevelTrigger, 0 );
	}

	inventory.pdaOpened = false;
	inventory.selPDA = 0;

	if ( !gameLocal.isMultiplayer ) {
		if ( g_skill.GetInteger() < 2 ) {
			if ( health < 25 ) {
				health = 25;
			}
			if ( g_useDynamicProtection.GetBool() ) {
				g_damageScale.SetFloat( 1.0f );
			}
		} else {
			g_damageScale.SetFloat( 1.0f );
			g_armorProtection.SetFloat( ( g_skill.GetInteger() < 2 ) ? 0.4f : 0.2f );
#ifndef ID_DEMO_BUILD
			if ( g_skill.GetInteger() == 3 ) {
				healthTake = true;
				nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
			}
#endif
		}
	}
}

/*
==============
idPlayer::~idPlayer()

Release any resources used by the player.
==============
*/
idPlayer::~idPlayer() {
	delete weapon.GetEntity();
	weapon = NULL;
}

/*
===========
idPlayer::Save
===========
*/
void idPlayer::Save( idSaveGame *savefile ) const {
	int i;

	savefile->WriteUsercmd( usercmd );
	playerView.Save( savefile );

	savefile->WriteBool( noclip );
#ifdef _DT // levitate spell
	savefile->WriteBool( levitate );
#endif
	savefile->WriteBool( godmode );

	// don't save spawnAnglesSet, since we'll have to reset them after loading the savegame
	savefile->WriteAngles( spawnAngles );
	savefile->WriteAngles( viewAngles );
	savefile->WriteAngles( cmdAngles );

	savefile->WriteInt( buttonMask );
	savefile->WriteInt( oldButtons );
	savefile->WriteInt( oldFlags );

	savefile->WriteInt( lastHitTime );
	savefile->WriteInt( lastSndHitTime );
	savefile->WriteInt( lastSavingThrowTime );

	// idBoolFields don't need to be saved, just re-linked in Restore

	inventory.Save( savefile );
	weapon.Save( savefile );

	savefile->WriteUserInterface( hud, false );
	savefile->WriteUserInterface( objectiveSystem, false );
	savefile->WriteBool( objectiveSystemOpen );

	// Start - Solarsplace - 15th May 2010  - Save Arx EOS user interfaces
	savefile->WriteUserInterface( inventorySystem, false );
	savefile->WriteBool( inventorySystemOpen );

	savefile->WriteUserInterface( journalSystem, false );
	savefile->WriteBool( journalSystemOpen );

	savefile->WriteUserInterface( readableSystem, false );
	savefile->WriteBool( readableSystemOpen );

	savefile->WriteUserInterface( conversationSystem, false );
	savefile->WriteBool( conversationSystemOpen );
	savefile->WriteString( conversationWindowQuestId );

	savefile->WriteUserInterface( shoppingSystem, false );
	savefile->WriteBool( shoppingSystemOpen );
	// End - Solarsplace - 15th May 2010  - Save Arx EOS user interfaces

	savefile->WriteInt( weapon_soulcube );
	savefile->WriteInt( weapon_pda );
	savefile->WriteInt( weapon_fists );

	savefile->WriteInt( heartRate );

	savefile->WriteFloat( heartInfo.GetStartTime() );
	savefile->WriteFloat( heartInfo.GetDuration() );
	savefile->WriteFloat( heartInfo.GetStartValue() );
	savefile->WriteFloat( heartInfo.GetEndValue() );

	savefile->WriteInt( lastHeartAdjust );
	savefile->WriteInt( lastHeartBeat );
	savefile->WriteInt( lastDmgTime );
	savefile->WriteInt( deathClearContentsTime );
	savefile->WriteBool( doingDeathSkin );
	savefile->WriteInt( lastArmorPulse );
	savefile->WriteFloat( stamina );
	savefile->WriteFloat( healthPool );
	savefile->WriteInt( nextHealthPulse );
	savefile->WriteBool( healthPulse );
	savefile->WriteInt( nextHealthTake );
	savefile->WriteBool( healthTake );

	savefile->WriteBool( hiddenWeapon );
	soulCubeProjectile.Save( savefile );

	savefile->WriteInt( spectator );
	savefile->WriteVec3( colorBar );
	savefile->WriteInt( colorBarIndex );
	savefile->WriteBool( scoreBoardOpen );
	savefile->WriteBool( forceScoreBoard );
	savefile->WriteBool( forceRespawn );
	savefile->WriteBool( spectating );
	savefile->WriteInt( lastSpectateTeleport );
	savefile->WriteBool( lastHitToggle );
	savefile->WriteBool( forcedReady );
	savefile->WriteBool( wantSpectate );
	savefile->WriteBool( weaponGone );
	savefile->WriteBool( useInitialSpawns );
	savefile->WriteInt( latchedTeam );
	savefile->WriteInt( tourneyRank );
	savefile->WriteInt( tourneyLine );

	teleportEntity.Save( savefile );
	savefile->WriteInt( teleportKiller );

	savefile->WriteInt( minRespawnTime );
	savefile->WriteInt( maxRespawnTime );

	savefile->WriteVec3( firstPersonViewOrigin );
	savefile->WriteMat3( firstPersonViewAxis );

#ifdef _DT	// head anim
	savefile->WriteMat3( firstPersonViewWeaponAxis );
#endif

	// don't bother saving dragEntity since it's a dev tool

	savefile->WriteJoint( hipJoint );
	savefile->WriteJoint( chestJoint );
	savefile->WriteJoint( headJoint );

	savefile->WriteStaticObject( physicsObj );

	savefile->WriteInt( aasLocation.Num() );
	for( i = 0; i < aasLocation.Num(); i++ ) {
		savefile->WriteInt( aasLocation[ i ].areaNum );
		savefile->WriteVec3( aasLocation[ i ].pos );
	}

	savefile->WriteInt( bobFoot );
	savefile->WriteFloat( bobFrac );
	savefile->WriteFloat( bobfracsin );
	savefile->WriteInt( bobCycle );
	savefile->WriteFloat( xyspeed );
	savefile->WriteInt( stepUpTime );
	savefile->WriteFloat( stepUpDelta );
	savefile->WriteFloat( idealLegsYaw );
	savefile->WriteFloat( legsYaw );
	savefile->WriteBool( legsForward );
	savefile->WriteFloat( oldViewYaw );
	savefile->WriteAngles( viewBobAngles );
	savefile->WriteVec3( viewBob );
	savefile->WriteInt( landChange );
	savefile->WriteInt( landTime );

	savefile->WriteInt( currentWeapon );
	savefile->WriteInt( idealWeapon );
	savefile->WriteInt( previousWeapon );
	savefile->WriteInt( weaponSwitchTime );
	savefile->WriteBool( weaponEnabled );
	savefile->WriteBool( showWeaponViewModel );

	savefile->WriteSkin( skin );
	savefile->WriteSkin( powerUpSkin );
	savefile->WriteString( baseSkinName );

	savefile->WriteInt( numProjectilesFired );
	savefile->WriteInt( numProjectileHits );

	savefile->WriteBool( airless );
	savefile->WriteInt( airTics );
	savefile->WriteInt( lastAirDamage );

	savefile->WriteBool( gibDeath );
	savefile->WriteBool( gibsLaunched );
	savefile->WriteVec3( gibsDir );

	savefile->WriteFloat( zoomFov.GetStartTime() );
	savefile->WriteFloat( zoomFov.GetDuration() );
	savefile->WriteFloat( zoomFov.GetStartValue() );
	savefile->WriteFloat( zoomFov.GetEndValue() );

	savefile->WriteFloat( centerView.GetStartTime() );
	savefile->WriteFloat( centerView.GetDuration() );
	savefile->WriteFloat( centerView.GetStartValue() );
	savefile->WriteFloat( centerView.GetEndValue() );

	savefile->WriteBool( fxFov );
#ifdef _DT
	savefile->WriteBool( isRunning );
#endif

	savefile->WriteFloat( influenceFov );
	savefile->WriteInt( influenceActive );
	savefile->WriteFloat( influenceRadius );
	savefile->WriteObject( influenceEntity );
	savefile->WriteMaterial( influenceMaterial );
	savefile->WriteSkin( influenceSkin );

	savefile->WriteObject( privateCameraView );

	for( i = 0; i < NUM_LOGGED_VIEW_ANGLES; i++ ) {
		savefile->WriteAngles( loggedViewAngles[ i ] );
	}
	for( i = 0; i < NUM_LOGGED_ACCELS; i++ ) {
		savefile->WriteInt( loggedAccel[ i ].time );
		savefile->WriteVec3( loggedAccel[ i ].dir );
	}
	savefile->WriteInt( currentLoggedAccel );

	savefile->WriteObject( focusGUIent );
	// can't save focusUI
	savefile->WriteObject( focusCharacter );
	savefile->WriteInt( talkCursor );
	savefile->WriteInt( focusTime );
	savefile->WriteObject( focusVehicle );
	savefile->WriteUserInterface( cursor, false );

	savefile->WriteInt( oldMouseX );
	savefile->WriteInt( oldMouseY );

	savefile->WriteString( pdaAudio );
	savefile->WriteString( pdaVideo );
	savefile->WriteString( pdaVideoWave );

	savefile->WriteBool( tipUp );
	savefile->WriteBool( objectiveUp );

	savefile->WriteInt( lastDamageDef );
	savefile->WriteVec3( lastDamageDir );
	savefile->WriteInt( lastDamageLocation );
	savefile->WriteInt( smoothedFrame );
	savefile->WriteBool( smoothedOriginUpdated );
	savefile->WriteVec3( smoothedOrigin );
	savefile->WriteAngles( smoothedAngles );

	savefile->WriteBool( ready );
	savefile->WriteBool( respawning );
	savefile->WriteBool( leader );
	savefile->WriteInt( lastSpectateChange );
	savefile->WriteInt( lastTeleFX );

	savefile->WriteFloat( pm_stamina.GetFloat() );

	//*****************************************************************************
	//*****************************************************************************
	//*****************************************************************************
	// Begin - Solarsplace - Arx End Of Sun

	// Magic related
	savefile->WriteBool( magicModeActive );
	savefile->WriteBool( lastMagicModeActive );
	savefile->WriteObject( magicWand );
	savefile->WriteObject( magicWandTrail );

	// SmartAI
	friendsCommonEnemy.Save( savefile );

	// End - Solarsplace - Arx End Of Sun
	//*****************************************************************************
	//*****************************************************************************
	//*****************************************************************************

	savefile->WriteInt( nScreenFrostAlpha );	// sikk - Screen Frost

	//savefile->WriteInt( adrenalineAmount );		// sikk - Adrenaline Pack System

// sikk---> Health Management System
	/*
	savefile->WriteInt( healthPackAmount );
	savefile->WriteInt( nextHealthRegen );
	savefile->WriteInt( prevHeatlh );
	*/
// <---sikk

// sikk--> Infrared Goggles/Headlight/Global Ambient light
	savefile->WriteBool( bAmbientLightOn );
	/*
	savefile->WriteBool( bIRGogglesOn );
	savefile->WriteBool( bHeadlightOn );
	savefile->WriteFloat( fIntensity );	
	savefile->WriteInt( nBattery );
	*/
// <---sikk

	if ( hud ) {
		hud->SetStateString( "message", common->GetLanguageDict()->GetString( "#str_02916" ) );
		hud->HandleNamedEvent( "Message" );
	}
}

/*
===========
idPlayer::Restore
===========
*/
void idPlayer::Restore( idRestoreGame *savefile ) {
	int	  i;
	int	  num;
	float set;

	// REMOVEME
	gameLocal.Printf( " idPlayer::Restore\n" );

	savefile->ReadUsercmd( usercmd );
	playerView.Restore( savefile );

	savefile->ReadBool( noclip );
#ifdef _DT // levitate spell
	savefile->ReadBool( levitate );
#endif
	savefile->ReadBool( godmode );

	savefile->ReadAngles( spawnAngles );
	savefile->ReadAngles( viewAngles );
	savefile->ReadAngles( cmdAngles );

	memset( usercmd.angles, 0, sizeof( usercmd.angles ) );
	SetViewAngles( viewAngles );
	spawnAnglesSet = true;

	savefile->ReadInt( buttonMask );
	savefile->ReadInt( oldButtons );
	savefile->ReadInt( oldFlags );

	usercmd.flags = 0;
	oldFlags = 0;

	savefile->ReadInt( lastHitTime );
	savefile->ReadInt( lastSndHitTime );
	savefile->ReadInt( lastSavingThrowTime );

	// Re-link idBoolFields to the scriptObject, values will be restored in scriptObject's restore
	LinkScriptVariables();

	inventory.Restore( savefile );
	weapon.Restore( savefile );

	for ( i = 0; i < inventory.emails.Num(); i++ ) {
		GetPDA()->AddEmail( inventory.emails[i] );
	}

	savefile->ReadUserInterface( hud );
	savefile->ReadUserInterface( objectiveSystem );
	savefile->ReadBool( objectiveSystemOpen );

	// Start - Solarsplace - 15th May 2010  - Load Arx EOS user interfaces
	savefile->ReadUserInterface( inventorySystem );
	savefile->ReadBool( inventorySystemOpen );

	savefile->ReadUserInterface( journalSystem );
	savefile->ReadBool( journalSystemOpen );

	savefile->ReadUserInterface( readableSystem );
	savefile->ReadBool( readableSystemOpen );

	savefile->ReadUserInterface( conversationSystem );
	savefile->ReadBool( conversationSystemOpen );
	savefile->ReadString( conversationWindowQuestId );

	savefile->ReadUserInterface( shoppingSystem );
	savefile->ReadBool( shoppingSystemOpen );
	// End - Solarsplace - 15th May 2010  - Load Arx EOS user interfaces

	savefile->ReadInt( weapon_soulcube );
	savefile->ReadInt( weapon_pda );
	savefile->ReadInt( weapon_fists );

	savefile->ReadInt( heartRate );

	savefile->ReadFloat( set );
	heartInfo.SetStartTime( set );
	savefile->ReadFloat( set );
	heartInfo.SetDuration( set );
	savefile->ReadFloat( set );
	heartInfo.SetStartValue( set );
	savefile->ReadFloat( set );
	heartInfo.SetEndValue( set );

	savefile->ReadInt( lastHeartAdjust );
	savefile->ReadInt( lastHeartBeat );
	savefile->ReadInt( lastDmgTime );
	savefile->ReadInt( deathClearContentsTime );
	savefile->ReadBool( doingDeathSkin );
	savefile->ReadInt( lastArmorPulse );
	savefile->ReadFloat( stamina );
	savefile->ReadFloat( healthPool );
	savefile->ReadInt( nextHealthPulse );
	savefile->ReadBool( healthPulse );
	savefile->ReadInt( nextHealthTake );
	savefile->ReadBool( healthTake );

	savefile->ReadBool( hiddenWeapon );
	soulCubeProjectile.Restore( savefile );

	savefile->ReadInt( spectator );
	savefile->ReadVec3( colorBar );
	savefile->ReadInt( colorBarIndex );
	savefile->ReadBool( scoreBoardOpen );
	savefile->ReadBool( forceScoreBoard );
	savefile->ReadBool( forceRespawn );
	savefile->ReadBool( spectating );
	savefile->ReadInt( lastSpectateTeleport );
	savefile->ReadBool( lastHitToggle );
	savefile->ReadBool( forcedReady );
	savefile->ReadBool( wantSpectate );
	savefile->ReadBool( weaponGone );
	savefile->ReadBool( useInitialSpawns );
	savefile->ReadInt( latchedTeam );
	savefile->ReadInt( tourneyRank );
	savefile->ReadInt( tourneyLine );

	teleportEntity.Restore( savefile );
	savefile->ReadInt( teleportKiller );

	savefile->ReadInt( minRespawnTime );
	savefile->ReadInt( maxRespawnTime );

	savefile->ReadVec3( firstPersonViewOrigin );
	savefile->ReadMat3( firstPersonViewAxis );

#ifdef _DT	// head anim
	savefile->ReadMat3( firstPersonViewWeaponAxis );
#endif

	// don't bother saving dragEntity since it's a dev tool
	dragEntity.Clear();

	savefile->ReadJoint( hipJoint );
	savefile->ReadJoint( chestJoint );
	savefile->ReadJoint( headJoint );

	savefile->ReadStaticObject( physicsObj );
	RestorePhysics( &physicsObj );

	savefile->ReadInt( num );
	aasLocation.SetGranularity( 1 );
	aasLocation.SetNum( num );
	for( i = 0; i < num; i++ ) {
		savefile->ReadInt( aasLocation[ i ].areaNum );
		savefile->ReadVec3( aasLocation[ i ].pos );
	}

	savefile->ReadInt( bobFoot );
	savefile->ReadFloat( bobFrac );
	savefile->ReadFloat( bobfracsin );
	savefile->ReadInt( bobCycle );
	savefile->ReadFloat( xyspeed );
	savefile->ReadInt( stepUpTime );
	savefile->ReadFloat( stepUpDelta );
	savefile->ReadFloat( idealLegsYaw );
	savefile->ReadFloat( legsYaw );
	savefile->ReadBool( legsForward );
	savefile->ReadFloat( oldViewYaw );
	savefile->ReadAngles( viewBobAngles );
	savefile->ReadVec3( viewBob );
	savefile->ReadInt( landChange );
	savefile->ReadInt( landTime );

	savefile->ReadInt( currentWeapon );
	savefile->ReadInt( idealWeapon );
	savefile->ReadInt( previousWeapon );
	savefile->ReadInt( weaponSwitchTime );
	savefile->ReadBool( weaponEnabled );
	savefile->ReadBool( showWeaponViewModel );

	savefile->ReadSkin( skin );
	savefile->ReadSkin( powerUpSkin );
	savefile->ReadString( baseSkinName );

	savefile->ReadInt( numProjectilesFired );
	savefile->ReadInt( numProjectileHits );

	savefile->ReadBool( airless );
	savefile->ReadInt( airTics );
	savefile->ReadInt( lastAirDamage );

	savefile->ReadBool( gibDeath );
	savefile->ReadBool( gibsLaunched );
	savefile->ReadVec3( gibsDir );

	savefile->ReadFloat( set );
	zoomFov.SetStartTime( set );
	savefile->ReadFloat( set );
	zoomFov.SetDuration( set );
	savefile->ReadFloat( set );
	zoomFov.SetStartValue( set );
	savefile->ReadFloat( set );
	zoomFov.SetEndValue( set );

	savefile->ReadFloat( set );
	centerView.SetStartTime( set );
	savefile->ReadFloat( set );
	centerView.SetDuration( set );
	savefile->ReadFloat( set );
	centerView.SetStartValue( set );
	savefile->ReadFloat( set );
	centerView.SetEndValue( set );

	savefile->ReadBool( fxFov );
#ifdef _DT
	savefile->ReadBool( isRunning );
#endif

	savefile->ReadFloat( influenceFov );
	savefile->ReadInt( influenceActive );
	savefile->ReadFloat( influenceRadius );
	savefile->ReadObject( reinterpret_cast<idClass *&>( influenceEntity ) );
	savefile->ReadMaterial( influenceMaterial );
	savefile->ReadSkin( influenceSkin );

	savefile->ReadObject( reinterpret_cast<idClass *&>( privateCameraView ) );

	for( i = 0; i < NUM_LOGGED_VIEW_ANGLES; i++ ) {
		savefile->ReadAngles( loggedViewAngles[ i ] );
	}
	for( i = 0; i < NUM_LOGGED_ACCELS; i++ ) {
		savefile->ReadInt( loggedAccel[ i ].time );
		savefile->ReadVec3( loggedAccel[ i ].dir );
	}
	savefile->ReadInt( currentLoggedAccel );

	savefile->ReadObject( reinterpret_cast<idClass *&>( focusGUIent ) );
	// can't save focusUI
	focusUI = NULL;
	savefile->ReadObject( reinterpret_cast<idClass *&>( focusCharacter ) );
	savefile->ReadInt( talkCursor );
	savefile->ReadInt( focusTime );
	savefile->ReadObject( reinterpret_cast<idClass *&>( focusVehicle ) );
	savefile->ReadUserInterface( cursor );

	savefile->ReadInt( oldMouseX );
	savefile->ReadInt( oldMouseY );

	savefile->ReadString( pdaAudio );
	savefile->ReadString( pdaVideo );
	savefile->ReadString( pdaVideoWave );

	savefile->ReadBool( tipUp );
	savefile->ReadBool( objectiveUp );

	savefile->ReadInt( lastDamageDef );
	savefile->ReadVec3( lastDamageDir );
	savefile->ReadInt( lastDamageLocation );
	savefile->ReadInt( smoothedFrame );
	savefile->ReadBool( smoothedOriginUpdated );
	savefile->ReadVec3( smoothedOrigin );
	savefile->ReadAngles( smoothedAngles );

	savefile->ReadBool( ready );
	savefile->ReadBool( respawning );
	savefile->ReadBool( leader );
	savefile->ReadInt( lastSpectateChange );
	savefile->ReadInt( lastTeleFX );

	// set the pm_ cvars
	const idKeyValue	*kv;
	kv = spawnArgs.MatchPrefix( "pm_", NULL );
	while( kv ) {
		cvarSystem->SetCVarString( kv->GetKey(), kv->GetValue() );
		kv = spawnArgs.MatchPrefix( "pm_", kv );
	}

	savefile->ReadFloat( set );
	pm_stamina.SetFloat( set );

	//*****************************************************************************
	//*****************************************************************************
	//*****************************************************************************
	// Begin - Solarsplace - Arx End Of Sun

	// Magic related
	savefile->ReadBool( magicModeActive );
	savefile->ReadBool( lastMagicModeActive );
	savefile->ReadObject( reinterpret_cast<idClass *&>( magicWand ) );
	savefile->ReadObject( reinterpret_cast<idClass *&>( magicWandTrail ) );

	// SmartAI
	friendsCommonEnemy.Restore( savefile );

	// End - Solarsplace - Arx End Of Sun
	//*****************************************************************************
	//*****************************************************************************
	//*****************************************************************************

	savefile->ReadInt( nScreenFrostAlpha );	// sikk - Screen Frost

	//savefile->ReadInt( adrenalineAmount );	// sikk - Adrenaline Pack System

// sikk---> Health Management System
	/*
	savefile->ReadInt( healthPackAmount );
	savefile->ReadInt( nextHealthRegen );
	savefile->ReadInt( prevHeatlh );
	*/
// <---sikk

// sikk--> Infrared Goggles/Headlight Mod/Global Ambient Light
	savefile->ReadBool( bAmbientLightOn );
	/*
	savefile->ReadBool( bIRGogglesOn );
	savefile->ReadBool( bHeadlightOn );
	savefile->ReadFloat( fIntensity );
	savefile->ReadInt( nBattery );
	*/
// <---sikk

	// create combat collision hull for exact collision detection
	SetCombatModel();
}

/*
===============
idPlayer::PrepareForRestart
================
*/
void idPlayer::PrepareForRestart( void ) {
	ClearPowerUps();
	Spectate( true );
	forceRespawn = true;
	
	// we will be restarting program, clear the client entities from program-related things first
	ShutdownThreads();

	// the sound world is going to be cleared, don't keep references to emitters
	FreeSoundEmitter( false );
}

/*
===============
idPlayer::Restart
================
*/
void idPlayer::Restart( void ) {
	idActor::Restart();
	
	// client needs to setup the animation script object again
	if ( gameLocal.isClient ) {
		Init();
	} else {
		// choose a random spot and prepare the point of view in case player is left spectating
		assert( spectating );
		SpawnFromSpawnSpot();
	}

	useInitialSpawns = true;
	UpdateSkinSetup( true );
}

/*
===============
idPlayer::ServerSpectate
================
*/
void idPlayer::ServerSpectate( bool spectate ) {
	assert( !gameLocal.isClient );

	if ( spectating != spectate ) {
		Spectate( spectate );
		if ( spectate ) {
			SetSpectateOrigin();
		} else {
			if ( gameLocal.gameType == GAME_DM ) {
				// make sure the scores are reset so you can't exploit by spectating and entering the game back
				// other game types don't matter, as you either can't join back, or it's team scores
				gameLocal.mpGame.ClearFrags( entityNumber );
			}
		}
	}
	if ( !spectate ) {
		SpawnFromSpawnSpot();
	}
}

/*
===========
idPlayer::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void idPlayer::SelectInitialSpawnPoint( idVec3 &origin, idAngles &angles ) {
	idEntity *spot;
	idStr skin;

	spot = gameLocal.SelectInitialSpawnPoint( this );

	// set the player skin from the spawn location
	if ( spot->spawnArgs.GetString( "skin", NULL, skin ) ) {
		spawnArgs.Set( "spawn_skin", skin );
	}

	// activate the spawn locations targets
	spot->PostEventMS( &EV_ActivateTargets, 0, this );

	origin = spot->GetPhysics()->GetOrigin();
	origin[2] += 4.0f + CM_BOX_EPSILON;		// move up to make sure the player is at least an epsilon above the floor
	angles = spot->GetPhysics()->GetAxis().ToAngles();
}

/*
===========
idPlayer::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void idPlayer::SpawnFromSpawnSpot( void ) {
	idVec3		spawn_origin;
	idAngles	spawn_angles;
	
	SelectInitialSpawnPoint( spawn_origin, spawn_angles );
	SpawnToPoint( spawn_origin, spawn_angles );
}

/*
===========
idPlayer::SpawnToPoint

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState

when called here with spectating set to true, just place yourself and init
============
*/
void idPlayer::SpawnToPoint( const idVec3 &spawn_origin, const idAngles &spawn_angles ) {
	idVec3 spec_origin;

	assert( !gameLocal.isClient );

	respawning = true;

	Init();

	fl.noknockback = false;

	// stop any ragdolls being used
	StopRagdoll();

	// set back the player physics
	SetPhysics( &physicsObj );

	physicsObj.SetClipModelAxis();
	physicsObj.EnableClip();

	if ( !spectating ) {
		SetCombatContents( true );
	}

	physicsObj.SetLinearVelocity( vec3_origin );

	// setup our initial view
	if ( !spectating ) {
		SetOrigin( spawn_origin );
	} else {
		spec_origin = spawn_origin;
		spec_origin[ 2 ] += pm_normalheight.GetFloat();
		spec_origin[ 2 ] += SPECTATE_RAISE;
		SetOrigin( spec_origin );
	}

	// if this is the first spawn of the map, we don't have a usercmd yet,
	// so the delta angles won't be correct.  This will be fixed on the first think.
	viewAngles = ang_zero;
	SetDeltaViewAngles( ang_zero );
	SetViewAngles( spawn_angles );
	spawnAngles = spawn_angles;
	spawnAnglesSet = false;

	legsForward = true;
	legsYaw = 0.0f;
	idealLegsYaw = 0.0f;
	oldViewYaw = viewAngles.yaw;

	if ( spectating ) {
		Hide();
	} else {
		Show();
	}

	if ( gameLocal.isMultiplayer ) {
		if ( !spectating ) {
			// we may be called twice in a row in some situations. avoid a double fx and 'fly to the roof'
			if ( lastTeleFX < gameLocal.time - 1000 ) {
				idEntityFx::StartFx( spawnArgs.GetString( "fx_spawn" ), &spawn_origin, NULL, this, true );
				lastTeleFX = gameLocal.time;
			}
		}
		AI_TELEPORT = true;
	} else {
		AI_TELEPORT = false;
	}

	// kill anything at the new position
	if ( !spectating ) {
		physicsObj.SetClipMask( MASK_PLAYERSOLID ); // the clip mask is usually maintained in Move(), but KillBox requires it
		gameLocal.KillBox( this );
	}

	// don't allow full run speed for a bit
	physicsObj.SetKnockBack( 100 );

	// set our respawn time and buttons so that if we're killed we don't respawn immediately
	minRespawnTime = gameLocal.time;
	maxRespawnTime = gameLocal.time;
	if ( !spectating ) {
		forceRespawn = false;
	}

	privateCameraView = NULL;

	BecomeActive( TH_THINK );

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	Think();

	//**************************************************
	//<--- Arx - EOS - Solarsplace 

	LoadTransitionInfo();

	if ( inventory.pdas.Num() == 0 ) {
		GivePDA( "arx_default", NULL );
	}

	//---> Arx - EOS - Solarsplace
	//**************************************************

	respawning			= false;
	lastManOver			= false;
	lastManPlayAgain	= false;
	isTelefragged		= false;
}

/*
===============
idPlayer::SavePersistantInfo

Saves any inventory and player stats when changing levels.
===============
*/
void idPlayer::SavePersistantInfo( void ) {

	idDict &playerInfo = gameLocal.persistentPlayerInfo[entityNumber];

	playerInfo.Clear();

	inventory.GetPersistantData( playerInfo );

	playerInfo.SetInt( "health", health );

	playerInfo.SetInt( "current_weapon", currentWeapon );
}

/*
===============
idPlayer::RestorePersistantInfo

Restores any inventory and player stats when changing levels.
===============
*/
void idPlayer::RestorePersistantInfo( void ) {

	if ( gameLocal.isMultiplayer ) {
		gameLocal.persistentPlayerInfo[entityNumber].Clear();
	}

	spawnArgs.Copy( gameLocal.persistentPlayerInfo[entityNumber] );

	inventory.RestoreInventory( this, spawnArgs );

	health = spawnArgs.GetInt( "health", "100" );

	if ( !gameLocal.isClient ) {

		idealWeapon = spawnArgs.GetInt( "current_weapon", "1" );

		// Solarsplace - Arx End Of Sun - 26th Nov 2013
		UpdateWeaponHealth();
	}
}

/*
================
idPlayer::GetUserInfo
================
*/
idDict *idPlayer::GetUserInfo( void ) {
	return &gameLocal.userInfo[ entityNumber ];
}

/*
==============
idPlayer::UpdateSkinSetup
==============
*/
void idPlayer::UpdateSkinSetup( bool restart ) {
	if ( restart ) {
		team = ( idStr::Icmp( GetUserInfo()->GetString( "ui_team" ), "Blue" ) == 0 );
	}
	if ( gameLocal.gameType == GAME_TDM ) {
		if ( team ) {
			baseSkinName = "skins/characters/player/marine_mp_blue";
		} else {
			baseSkinName = "skins/characters/player/marine_mp_red";
		}
		if ( !gameLocal.isClient && team != latchedTeam ) {
			gameLocal.mpGame.SwitchToTeam( entityNumber, latchedTeam, team );
		}
		latchedTeam = team;
	} else {
		baseSkinName = GetUserInfo()->GetString( "ui_skin" );
	}
	if ( !baseSkinName.Length() ) {
		baseSkinName = "skins/characters/player/marine_mp";
	}
	skin = declManager->FindSkin( baseSkinName, false );
	assert( skin );
	// match the skin to a color band for scoreboard
	if ( baseSkinName.Find( "red" ) != -1 ) {
		colorBarIndex = 1;
	} else if ( baseSkinName.Find( "green" ) != -1 ) {
		colorBarIndex = 2;
	} else if ( baseSkinName.Find( "blue" ) != -1 ) {
		colorBarIndex = 3;
	} else if ( baseSkinName.Find( "yellow" ) != -1 ) {
		colorBarIndex = 4;
	} else {
		colorBarIndex = 0;
	}
	colorBar = colorBarTable[ colorBarIndex ];
	if ( PowerUpActive( BERSERK ) ) {
		powerUpSkin = declManager->FindSkin( baseSkinName + "_berserk" );
	}
}

/*
==============
idPlayer::BalanceTDM
==============
*/
bool idPlayer::BalanceTDM( void ) {
	int			i, balanceTeam, teamCount[2];
	idEntity	*ent;

	teamCount[ 0 ] = teamCount[ 1 ] = 0;
	for( i = 0; i < gameLocal.numClients; i++ ) {
		ent = gameLocal.entities[ i ];
		if ( ent && ent->IsType( idPlayer::Type ) ) {
			teamCount[ static_cast< idPlayer * >( ent )->team ]++;
		}
	}
	balanceTeam = -1;
	if ( teamCount[ 0 ] < teamCount[ 1 ] ) {
		balanceTeam = 0;
	} else if ( teamCount[ 0 ] > teamCount[ 1 ] ) {
		balanceTeam = 1;
	}
	if ( balanceTeam != -1 && team != balanceTeam ) {
		common->DPrintf( "team balance: forcing player %d to %s team\n", entityNumber, balanceTeam ? "blue" : "red" );
		team = balanceTeam;
		GetUserInfo()->Set( "ui_team", team ? "Blue" : "Red" );
		return true;
	}
	return false;
}

/*
==============
idPlayer::UserInfoChanged
==============
*/
bool idPlayer::UserInfoChanged( bool canModify ) {
	idDict	*userInfo;
	bool	modifiedInfo;
	bool	spec;
	bool	newready;

	userInfo = GetUserInfo();
	showWeaponViewModel = userInfo->GetBool( "ui_showGun" );

	if ( !gameLocal.isMultiplayer ) {
		return false;
	}

	modifiedInfo = false;

	spec = ( idStr::Icmp( userInfo->GetString( "ui_spectate" ), "Spectate" ) == 0 );
	if ( gameLocal.serverInfo.GetBool( "si_spectators" ) ) {
		// never let spectators go back to game while sudden death is on
		if ( canModify && gameLocal.mpGame.GetGameState() == idMultiplayerGame::SUDDENDEATH && !spec && wantSpectate == true ) {
			userInfo->Set( "ui_spectate", "Spectate" );
			modifiedInfo |= true;
		} else {
			if ( spec != wantSpectate && !spec ) {
				// returning from spectate, set forceRespawn so we don't get stuck in spectate forever
				forceRespawn = true;
			}
			wantSpectate = spec;
		}
	} else {
		if ( canModify && spec ) {
			userInfo->Set( "ui_spectate", "Play" );
			modifiedInfo |= true;
		} else if ( spectating ) {  
			// allow player to leaving spectator mode if they were in it when si_spectators got turned off
			forceRespawn = true;
		}
		wantSpectate = false;
	}

	newready = ( idStr::Icmp( userInfo->GetString( "ui_ready" ), "Ready" ) == 0 );
	if ( ready != newready && gameLocal.mpGame.GetGameState() == idMultiplayerGame::WARMUP && !wantSpectate ) {
		gameLocal.mpGame.AddChatLine( common->GetLanguageDict()->GetString( "#str_07180" ), userInfo->GetString( "ui_name" ), newready ? common->GetLanguageDict()->GetString( "#str_04300" ) : common->GetLanguageDict()->GetString( "#str_04301" ) );
	}
	ready = newready;
	team = ( idStr::Icmp( userInfo->GetString( "ui_team" ), "Blue" ) == 0 );
	// server maintains TDM balance
	if ( canModify && gameLocal.gameType == GAME_TDM && !gameLocal.mpGame.IsInGame( entityNumber ) && g_balanceTDM.GetBool() ) {
		modifiedInfo |= BalanceTDM( );
	}
	UpdateSkinSetup( false );
	
	isChatting = userInfo->GetBool( "ui_chat", "0" );
	if ( canModify && isChatting && AI_DEAD ) {
		// if dead, always force chat icon off.
		isChatting = false;
		userInfo->SetBool( "ui_chat", false );
		modifiedInfo |= true;
	}

	return modifiedInfo;
}

/*
===============
idPlayer::GetPlayerManaAmount
===============
*/
int idPlayer::GetPlayerManaAmount( void )
{
	// Created by Solarsplace 28th Feb 2010

	const char *weap;

	// This assumes that the weapon in slot 6 takes mana as ammo.
	weap = spawnArgs.GetString( va( "def_weapon%d", ARX_MANA_WEAPON ) );

	if ( inventory.HasAmmo( weap ) )
	{ return inventory.HasAmmo( weap ); }
	else
	{ return 0; }

}

/*
===============
idPlayer::UpdateHudAmmo
===============
*/
void idPlayer::UpdateHudAmmo( idUserInterface *_hud ) {
	int inclip;
	int ammoamount;
	int i;

	// Solarsplace - Arx End Of Sun
	int totalMana;
	bool hasChargeAttack;
	ArxTraceAIHealthHUD();

	assert( weapon.GetEntity() );
	assert( _hud );

	inclip		= weapon.GetEntity()->AmmoInClip();
	ammoamount	= weapon.GetEntity()->AmmoAvailable();

	// Solarsplace - Arx End Of Sun
	hasChargeAttack = weapon.GetEntity()->HasChargeAttack();
	_hud->SetStateBool( "arx_has_charge_attack", hasChargeAttack );

	if ( ammoamount < 0 || !weapon.GetEntity()->IsReady() ) {
		// show infinite ammo
		_hud->SetStateString( "player_ammo", "" );
		_hud->SetStateString( "player_totalammo", "" );
	} else { 
		// show remaining ammo
		_hud->SetStateString( "player_totalammo", va( "%i", ammoamount - inclip ) );
		_hud->SetStateString( "player_ammo", weapon.GetEntity()->ClipSize() ? va( "%i", inclip ) : "--" );		// how much in the current clip
		_hud->SetStateString( "player_clips", weapon.GetEntity()->ClipSize() ? va( "%i", ammoamount / weapon.GetEntity()->ClipSize() ) : "--" );
		_hud->SetStateString( "player_allammo", va( "%i/%i", inclip, ammoamount - inclip ) );
	} 

	// Solarsplace 28th Feb 2010
	// Show the mana total for the player
	totalMana = GetPlayerManaAmount();
	_hud->SetStateString( "player_totalmana", va( "%i", totalMana ) );

	// Solarsplace 4th Sep 2013
	// Hide the mana meter unless the player has the snake compass
	bool hasSnakeCompass = false;
	if ( FindInventoryItemCount( "#str_item_00099" ) > 0 ) {
		hasSnakeCompass = true;
	}
	_hud->SetStateBool( "player_mana_visible", ( hasSnakeCompass ) );

	// Solarsplace 17th May 2010 - Poison related
	if ( inventory.arx_timer_player_poison >= gameLocal.time )
	{ _hud->SetStateString( "poisoned", "1" ); }
	else
	{ _hud->SetStateString( "poisoned", "0" ); }
	
	_hud->SetStateBool( "player_ammo_empty", ( ammoamount == 0 ) );
	_hud->SetStateBool( "player_clip_empty", ( weapon.GetEntity()->ClipSize() ? inclip == 0 : false ) );
	_hud->SetStateBool( "player_clip_low", ( weapon.GetEntity()->ClipSize() ? inclip <= weapon.GetEntity()->LowAmmo() : false ) );

	_hud->HandleNamedEvent( "updateAmmo" );

	// Quickspells
	for ( i = 0; i < 3; i++ )
	{
		// Runes -- Not sure if this is efficient? suspect not.... Don't see the game doing it anywhere :(
		const char *result;

		gameLocal.persistentLevelInfo.GetString( va( "magic_aam_yok_taar_%i", i ), "0", &result );
		_hud->SetStateString( va( "magic_aam_yok_taar_%i", i ), result );

		gameLocal.persistentLevelInfo.GetString( va( "magic_aam_folgora_taar_%i", i ), "0", &result );
		_hud->SetStateString( va( "magic_aam_folgora_taar_%i", i ), result );
	}
}

/*
===============
idPlayer::UpdateHudStats
===============
*/
void idPlayer::UpdateHudStats( idUserInterface *_hud ) {
	int staminapercentage;
	float max_stamina;

	assert( _hud );

	max_stamina = pm_stamina.GetFloat();
	if ( !max_stamina ) {
		// stamina disabled, so show full stamina bar
		staminapercentage = 100.0f;
	} else {
		staminapercentage = idMath::FtoiFast( 100.0f * stamina / max_stamina );
	}

	_hud->SetStateInt( "player_health", health );
	_hud->SetStateInt( "player_stamina", staminapercentage );
	_hud->SetStateInt( "player_armor", inventory.armor );
	_hud->SetStateInt( "player_hr", heartRate );
	_hud->SetStateInt( "player_nostamina", ( max_stamina == 0 ) ? 1 : 0 );

	_hud->HandleNamedEvent( "updateArmorHealthAir" );

	if ( healthPulse ) {
		_hud->HandleNamedEvent( "healthPulse" );
		StartSound( "snd_healthpulse", SND_CHANNEL_ITEM, 0, false, NULL );
		healthPulse = false;
	}

	if ( healthTake ) {
		_hud->HandleNamedEvent( "healthPulse" );
		StartSound( "snd_healthtake", SND_CHANNEL_ITEM, 0, false, NULL );
		healthTake = false;
	}

	if ( inventory.ammoPulse ) { 
		_hud->HandleNamedEvent( "ammoPulse" );
		inventory.ammoPulse = false;
	}
	if ( inventory.weaponPulse ) {
		// We need to update the weapon hud manually, but not
		// the armor/ammo/health because they are updated every
		// frame no matter what
		UpdateHudWeapon();
		_hud->HandleNamedEvent( "weaponPulse" );
		inventory.weaponPulse = false;
	}
	if ( inventory.armorPulse ) { 
		_hud->HandleNamedEvent( "armorPulse" );
		inventory.armorPulse = false;
	}

	UpdateHudAmmo( _hud );
}

/*
===============
idPlayer::UpdateHudWeapon
===============
*/
void idPlayer::UpdateHudWeapon( bool flashWeapon ) {
	idUserInterface *hud = idPlayer::hud;

	// if updating the hud of a followed client
	if ( gameLocal.localClientNum >= 0 && gameLocal.entities[ gameLocal.localClientNum ] && gameLocal.entities[ gameLocal.localClientNum ]->IsType( idPlayer::Type ) ) {
		idPlayer *p = static_cast< idPlayer * >( gameLocal.entities[ gameLocal.localClientNum ] );
		if ( p->spectating && p->spectator == entityNumber ) {
			assert( p->hud );
			hud = p->hud;
		}
	}

	if ( !hud ) {
		return;
	}

	for ( int i = 0; i < MAX_WEAPONS; i++ ) {
		const char *weapnum = va( "def_weapon%d", i );
		const char *hudWeap = va( "weapon%d", i );
		int weapstate = 0;
		if ( inventory.weapons & ( 1 << i ) ) {
			const char *weap = spawnArgs.GetString( weapnum );
			if ( weap && *weap ) {
				weapstate++;
			}
			if ( idealWeapon == i ) {
				weapstate++;
			}
		}
		hud->SetStateInt( hudWeap, weapstate );
	}
	if ( flashWeapon ) {
		hud->HandleNamedEvent( "weaponChange" );
	}
}

/*
===============
idPlayer::DrawHUD
===============
*/
void idPlayer::DrawHUD( idUserInterface *_hud ) {

	if ( !weapon.GetEntity() || influenceActive != INFLUENCE_NONE || privateCameraView || gameLocal.GetCamera() || !_hud || !g_showHud.GetBool() ) {
		return;
	}

	UpdateHudStats( _hud );

	_hud->SetStateString( "weapicon", weapon.GetEntity()->Icon() );

	// FIXME: this is temp to allow the sound meter to show up in the hud
	// it should be commented out before shipping but the code can remain
	// for mod developers to enable for the same functionality
	_hud->SetStateInt( "s_debug", cvarSystem->GetCVarInteger( "s_showLevelMeter" ) );

	weapon.GetEntity()->UpdateGUI();

	_hud->Redraw( gameLocal.realClientTime );

	// weapon targeting crosshair
	if ( !GuiActive() ) {
		if ( cursor && weapon.GetEntity()->ShowCrosshair() ) {
			cursor->Redraw( gameLocal.realClientTime );
		}
	}
}

/*
===============
idPlayer::EnterCinematic
===============
*/
void idPlayer::EnterCinematic( void ) {
	Hide();
	StopAudioLog();
	StopSound( SND_CHANNEL_PDA, false );
	if ( hud ) {
		hud->HandleNamedEvent( "radioChatterDown" );
	}
	
	physicsObj.SetLinearVelocity( vec3_origin );
	
	SetState( "EnterCinematic" );
	UpdateScript();

	if ( weaponEnabled && weapon.GetEntity() ) {
		weapon.GetEntity()->EnterCinematic();
	}

	AI_FORWARD		= false;
	AI_BACKWARD		= false;
	AI_STRAFE_LEFT	= false;
	AI_STRAFE_RIGHT	= false;
	AI_RUN			= false;
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED	= false;
	AI_JUMP			= false;
	AI_CROUCH		= false;
	AI_ONGROUND		= true;
	AI_ONLADDER		= false;
	AI_DEAD			= ( health <= 0 );
	AI_RUN			= false;
	AI_PAIN			= false;
	AI_HARDLANDING	= false;
	AI_SOFTLANDING	= false;
	AI_RELOAD		= false;
	AI_TELEPORT		= false;
	AI_TURN_LEFT	= false;
	AI_TURN_RIGHT	= false;
}

/*
===============
idPlayer::ExitCinematic
===============
*/
void idPlayer::ExitCinematic( void ) {
	Show();

	if ( weaponEnabled && weapon.GetEntity() ) {
		weapon.GetEntity()->ExitCinematic();
	}

	SetState( "ExitCinematic" );
	UpdateScript();
}

/*
=====================
idPlayer::UpdateConditions
=====================
*/
void idPlayer::UpdateConditions( void ) {
	idVec3	velocity;
	float	fallspeed;
	float	forwardspeed;
	float	sidespeed;

	// minus the push velocity to avoid playing the walking animation and sounds when riding a mover
	velocity = physicsObj.GetLinearVelocity() - physicsObj.GetPushedLinearVelocity();
	fallspeed = velocity * physicsObj.GetGravityNormal();

	if ( influenceActive ) {
		AI_FORWARD		= false;
		AI_BACKWARD		= false;
		AI_STRAFE_LEFT	= false;
		AI_STRAFE_RIGHT	= false;
	} else if ( gameLocal.time - lastDmgTime < 500 ) {
		forwardspeed = velocity * viewAxis[ 0 ];
		sidespeed = velocity * viewAxis[ 1 ];
		AI_FORWARD		= AI_ONGROUND && ( forwardspeed > 20.01f );
		AI_BACKWARD		= AI_ONGROUND && ( forwardspeed < -20.01f );
		AI_STRAFE_LEFT	= AI_ONGROUND && ( sidespeed > 20.01f );
		AI_STRAFE_RIGHT	= AI_ONGROUND && ( sidespeed < -20.01f );
	} else if ( xyspeed > MIN_BOB_SPEED ) {
		AI_FORWARD		= AI_ONGROUND && ( usercmd.forwardmove > 0 );
		AI_BACKWARD		= AI_ONGROUND && ( usercmd.forwardmove < 0 );
		AI_STRAFE_LEFT	= AI_ONGROUND && ( usercmd.rightmove < 0 );
		AI_STRAFE_RIGHT	= AI_ONGROUND && ( usercmd.rightmove > 0 );
	} else {
		AI_FORWARD		= false;
		AI_BACKWARD		= false;
		AI_STRAFE_LEFT	= false;
		AI_STRAFE_RIGHT	= false;
	}

	AI_RUN			= ( usercmd.buttons & BUTTON_RUN ) && ( ( !pm_stamina.GetFloat() ) || ( stamina > pm_staminathreshold.GetFloat() ) );
	AI_DEAD			= ( health <= 0 );
}

/*
==================
WeaponFireFeedback

Called when a weapon fires, generates head twitches, etc
==================
*/
void idPlayer::WeaponFireFeedback( const idDict *weaponDef ) {
	// force a blink
	blink_time = 0;

	// play the fire animation
	AI_WEAPON_FIRED = true;

	// update view feedback
	playerView.WeaponFireFeedback( weaponDef );
}

/*
===============
idPlayer::StopFiring
===============
*/
void idPlayer::StopFiring( void ) {
	AI_ATTACK_HELD	= false;
	AI_WEAPON_FIRED = false;
	AI_RELOAD		= false;
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->EndAttack();
	}
}

/*
===============
idPlayer::FireWeapon
===============
*/
void idPlayer::FireWeapon( void ) {

	// Solarsplace - 27th April 2010 - Magic related.
	// Do not allow the player to fire any weapon when in magic casting mode.
	if ( magicModeActive )
	{ return; }

	// Solarsplace - 22nd May 2010 - Magic related.
	// Never allow the player to manually fire the magic weapon.
	// Solarsplace - 18th March 2015 - Changed how this works. The magic weapons are now script controlled normal weapons.
	/*
	if ( currentWeapon == ARX_MAGIC_WEAPON )
	{ return; }
	*/

	idMat3 axis;
	idVec3 muzzle;

	if ( privateCameraView ) {
		return;
	}

	if ( g_editEntityMode.GetInteger() ) {
		GetViewPos( muzzle, axis );
		if ( gameLocal.editEntities->SelectEntity( muzzle, axis[0], this ) ) {
			return;
		}
	}

	if ( !hiddenWeapon && weapon.GetEntity()->IsReady() ) {
		if ( weapon.GetEntity()->AmmoInClip() || weapon.GetEntity()->AmmoAvailable() ) {
			AI_ATTACK_HELD = true;
			weapon.GetEntity()->BeginAttack();
			if ( ( weapon_soulcube >= 0 ) && ( currentWeapon == weapon_soulcube ) ) {
				if ( hud ) {
					hud->HandleNamedEvent( "soulCubeNotReady" );
				}
				SelectWeapon( previousWeapon, false );
			}
		} else {
			NextBestWeapon();
		}
	}

	if ( hud ) {
		if ( tipUp ) {
			HideTip();
		}
		// may want to track with with a bool as well
		// keep from looking up named events so often
		if ( objectiveUp ) {
			HideObjective();
		}
	}
}

/*
===============
idPlayer::CacheWeapons
===============
*/
void idPlayer::CacheWeapons( void ) {
	idStr	weap;
	int		w;

	// check if we have any weapons
	if ( !inventory.weapons ) {
		return;
	}
	
	for( w = 0; w < MAX_WEAPONS; w++ ) {
		if ( inventory.weapons & ( 1 << w ) ) {
			weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
			if ( weap != "" ) {
				idWeapon::CacheWeapon( weap );
			} else {
				inventory.weapons &= ~( 1 << w );
			}
		}
	}
}

/*
===============
idPlayer::Give
===============
*/
bool idPlayer::Give( const char *statname, const char *value ) {
	int amount;

	if ( AI_DEAD ) {
		return false;
	}

	if ( !idStr::Icmp( statname, "health" ) ) {
		if ( health >= inventory.maxHealth ) {
			// Arx EOS
			ShowHudMessage( "#str_general_00017" ); // "Your health is already at maximum"
			return false;
		}
		amount = atoi( value );
		if ( amount ) {
			health += amount;
			if ( health > inventory.maxHealth ) {
				health = inventory.maxHealth;
			}
			if ( hud ) {
				hud->HandleNamedEvent( "healthPulse" );
			}
		}

	} else if ( !idStr::Icmp( statname, "stamina" ) ) {
		if ( stamina >= 100 ) {
			return false;
		}
		stamina += atof( value );
		if ( stamina > 100 ) {
			stamina = 100;
		}

	} else if ( !idStr::Icmp( statname, "heartRate" ) ) {
		heartRate += atoi( value );
		if ( heartRate > MAX_HEARTRATE ) {
			heartRate = MAX_HEARTRATE;
		}

	} else if ( !idStr::Icmp( statname, "air" ) ) {
		if ( airTics >= pm_airTics.GetInteger() ) {
			return false;
		}
		airTics += atoi( value ) / 100.0 * pm_airTics.GetInteger();
		if ( airTics > pm_airTics.GetInteger() ) {
			airTics = pm_airTics.GetInteger();
		}
	} else {
		return inventory.Give( this, spawnArgs, statname, value, &idealWeapon, true );
	}
	return true;
}


/*
===============
idPlayer::GiveHealthPool

adds health to the player health pool
===============
*/
void idPlayer::GiveHealthPool( float amt ) {
	
	if ( AI_DEAD ) {
		return;
	}

	if ( health > 0 ) {
		healthPool += amt;
		if ( healthPool > inventory.maxHealth - health ) {
			healthPool = inventory.maxHealth - health;
		}
		nextHealthPulse = gameLocal.time;
	}
}

/*
===============
idPlayer::GiveItem

Returns false if the item shouldn't be picked up
===============
*/
bool idPlayer::GiveItem( idItem *item ) {

	//REMOVED
	//gameLocal.Printf( "Entered idPlayer::GiveItem( idItem *item )\n" );

	int					i;
	const idKeyValue	*arg;
	idDict				attr;
	bool				gave;
	int					numPickup;

	if ( gameLocal.isMultiplayer && spectating ) {
		return false;
	}

	item->GetAttributes( attr );
	
	gave = false;
	numPickup = inventory.pickupItemNames.Num();
	for( i = 0; i < attr.GetNumKeyVals(); i++ ) {
		arg = attr.GetKeyVal( i );
		if ( Give( arg->GetKey(), arg->GetValue() ) ) {
			gave = true;
		}
	}

	arg = item->spawnArgs.MatchPrefix( "inv_weapon", NULL );
	if ( arg && hud ) {
		// We need to update the weapon hud manually, but not
		// the armor/ammo/health because they are updated every
		// frame no matter what
		UpdateHudWeapon( false );
		hud->HandleNamedEvent( "weaponPulse" );
	}

	// display the pickup feedback on the hud
	if ( gave && ( numPickup == inventory.pickupItemNames.Num() ) ) {
		inventory.AddPickupName( item->spawnArgs.GetString( "inv_name" ), item->spawnArgs.GetString( "inv_icon" ) );
	}

	return gave;
}

/*
===============
idPlayer::PowerUpModifier
===============
*/
float idPlayer::PowerUpModifier( int type ) {
	float mod = 1.0f;

	if ( PowerUpActive( BERSERK ) ) {
		switch( type ) {
			case SPEED: {
				mod *= 1.7f;
				break;
			}
			case PROJECTILE_DAMAGE: {
				mod *= 2.0f;
				break;
			}
			case MELEE_DAMAGE: {
				mod *= 30.0f;
				break;
			}
			case MELEE_DISTANCE: {
				mod *= 2.0f;
				break;
			}
		}
	}

	if ( gameLocal.isMultiplayer && !gameLocal.isClient ) {
		if ( PowerUpActive( MEGAHEALTH ) ) {
			if ( healthPool <= 0 ) {
				GiveHealthPool( 100 );
			}
		} else {
			healthPool = 0;
		}
	}

	return mod;
}

/*
===============
idPlayer::PowerUpActive
===============
*/
bool idPlayer::PowerUpActive( int powerup ) const {
	return ( inventory.powerups & ( 1 << powerup ) ) != 0;
}

/*
===============
idPlayer::GivePowerUp
===============
*/
bool idPlayer::GivePowerUp( int powerup, int time ) {
	const char *sound;
	const char *skin;

	if ( powerup >= 0 && powerup < MAX_POWERUPS ) {

		if ( gameLocal.isServer ) {
			idBitMsg	msg;
			byte		msgBuf[MAX_EVENT_PARAM_SIZE];

			msg.Init( msgBuf, sizeof( msgBuf ) );
			msg.WriteShort( powerup );
			msg.WriteBits( 1, 1 );
			ServerSendEvent( EVENT_POWERUP, &msg, false, -1 );
		}

		if ( powerup != MEGAHEALTH ) {
			inventory.GivePowerUp( this, powerup, time );
		}

		const idDeclEntityDef *def = NULL;

		switch( powerup ) {
			case BERSERK: {
				if ( spawnArgs.GetString( "snd_berserk_third", "", &sound ) ) {
					StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_DEMONIC, 0, false, NULL );
				}
				if ( baseSkinName.Length() ) {
					powerUpSkin = declManager->FindSkin( baseSkinName + "_berserk" );
				}
				if ( !gameLocal.isClient ) {
					idealWeapon = 0;
				}
				break;
			}
			case INVISIBILITY: {
				spawnArgs.GetString( "skin_invisibility", "", &skin );
				powerUpSkin = declManager->FindSkin( skin );
				// remove any decals from the model
				if ( modelDefHandle != -1 ) {
					gameRenderWorld->RemoveDecals( modelDefHandle );
				}
				if ( weapon.GetEntity() ) {
					weapon.GetEntity()->UpdateSkin();
				}
				if ( spawnArgs.GetString( "snd_invisibility", "", &sound ) ) {
					StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
				}
				break;
			}
			case ADRENALINE: {
				stamina = 100.0f;
				break;
			 }
			case MEGAHEALTH: {
				if ( spawnArgs.GetString( "snd_megahealth", "", &sound ) ) {
					StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
				}
				def = gameLocal.FindEntityDef( "powerup_megahealth", false );
				if ( def ) {
					health = def->dict.GetInt( "inv_health" );
				}
				break;
			 }
		}

		if ( hud ) {
			hud->HandleNamedEvent( "itemPickup" );
		}

		return true;
	} else {
		gameLocal.Warning( "Player given power up %i\n which is out of range", powerup );
	}
	return false;
}

/*
==============
idPlayer::ClearPowerup
==============
*/
void idPlayer::ClearPowerup( int i ) {

	if ( gameLocal.isServer ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteShort( i );
		msg.WriteBits( 0, 1 );
		ServerSendEvent( EVENT_POWERUP, &msg, false, -1 );
	}

	powerUpSkin = NULL;
	inventory.powerups &= ~( 1 << i );
	inventory.powerupEndTime[ i ] = 0;
	switch( i ) {
		case BERSERK: {
			StopSound( SND_CHANNEL_DEMONIC, false );
			break;
		}
		case INVISIBILITY: {
			if ( weapon.GetEntity() ) {
				weapon.GetEntity()->UpdateSkin();
			}
			break;
		}
	}
}

/*
==============
idPlayer::UpdatePowerUps
==============
*/
void idPlayer::UpdatePowerUps( void ) {
	int i;

	if ( !gameLocal.isClient ) {
		for ( i = 0; i < MAX_POWERUPS; i++ ) {
			if ( PowerUpActive( i ) && inventory.powerupEndTime[i] <= gameLocal.time ) {
				ClearPowerup( i );
			}
		}
	}

	if ( health > 0 ) {
		if ( powerUpSkin ) {
			renderEntity.customSkin = powerUpSkin;
		} else {
			renderEntity.customSkin = skin;
		}
	}

	if ( healthPool && gameLocal.time > nextHealthPulse && !AI_DEAD && health > 0 ) {
		assert( !gameLocal.isClient );	// healthPool never be set on client
		int amt = ( healthPool > 5 ) ? 5 : healthPool;
		health += amt;
		if ( health > inventory.maxHealth ) {
			health = inventory.maxHealth;
			healthPool = 0;
		} else {
			healthPool -= amt;
		}
		nextHealthPulse = gameLocal.time + HEALTHPULSE_TIME;
		healthPulse = true;
	}
#ifndef ID_DEMO_BUILD
	if ( !gameLocal.inCinematic && influenceActive == 0 && g_skill.GetInteger() == 3 && gameLocal.time > nextHealthTake && !AI_DEAD && health > g_healthTakeLimit.GetInteger() ) {
		assert( !gameLocal.isClient );	// healthPool never be set on client
		health -= g_healthTakeAmt.GetInteger();
		if ( health < g_healthTakeLimit.GetInteger() ) {
			health = g_healthTakeLimit.GetInteger();
		}
		nextHealthTake = gameLocal.time + g_healthTakeTime.GetInteger() * 1000;
		healthTake = true;
	}
#endif
}

/*
===============
idPlayer::ClearPowerUps
===============
*/
void idPlayer::ClearPowerUps( void ) {
	int i;
	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		if ( PowerUpActive( i ) ) {
			ClearPowerup( i );
		}
	}
	inventory.ClearPowerUps();
}

/*
===============
idPlayer::GiveInventoryItem
===============
*/
bool idPlayer::GiveInventoryItem( idDict *item ) {

	if ( gameLocal.isMultiplayer && spectating ) {
		return false;
	}

	// Solarsplace - 7th Dec 2012
	// Note! - This method needs enhancing. For instance adding the drop item version to inventory rather than pickup version....

	// Solarsplace - 7th Dec 2012 - Hack to handle runes differently.
	if ( item->GetBool( "player_persistent_rune", "0" ) ) {

		gameLocal.persistentLevelInfo.SetBool( item->GetString( "persistent_rune_name" ), true );

		// Update persistent information for the journal spells that the player can cast. - 22nd Jul 2010 for Nuro
		MagicUpdateJournalSpells();

	} else {

		// SP - Arx - 21st Feb 2013 - Weapon health related
		// Now create a "reasonably" unique name for everything added to the inventory. Chances of this being duplicated are (i hope) very very very small. This is a bodge really.
		idStr fairlyUniqueName;
		sprintf( fairlyUniqueName, "<ARX_NAME_START>_%i_%i_%i_<ARX_NAME_END>", gameLocal.random.RandomInt(), gameLocal.random.RandomInt(), gameLocal.random.RandomInt() );
		item->Set( "inv_unique_name", fairlyUniqueName );
		//gameLocal.Printf( "Adding unique name %s to dictionary\n", fairlyUniqueName.c_str() );

		// Start -> Original code path
		inventory.items.Append( new idDict( *item ) );
		idItemInfo info;
		const char* itemName = item->GetString( "inv_name" );
		if ( idStr::Cmpn( itemName, STRTABLE_ID, STRTABLE_ID_LENGTH ) == 0 ) {
			info.name = common->GetLanguageDict()->GetString( itemName );
		} else {
			info.name = itemName;
		}
		info.icon = item->GetString( "inv_icon" );
		inventory.pickupItemNames.Append( info );
		if ( hud ) {
			hud->SetStateString( "itemicon", info.icon );
			hud->HandleNamedEvent( "invPickup" );
		}
		// End <- Original code path
	}

	return true;
}

/*
==============
idPlayer::UpdateObjectiveInfo
==============
 */
void idPlayer::UpdateObjectiveInfo( void ) {
	if ( objectiveSystem == NULL ) {
		return;
	}
	objectiveSystem->SetStateString( "objective1", "" );
	objectiveSystem->SetStateString( "objective2", "" );
	objectiveSystem->SetStateString( "objective3", "" );
	for ( int i = 0; i < inventory.objectiveNames.Num(); i++ ) {
		objectiveSystem->SetStateString( va( "objective%i", i+1 ), "1" );
		objectiveSystem->SetStateString( va( "objectivetitle%i", i+1 ), inventory.objectiveNames[i].title.c_str() );
		objectiveSystem->SetStateString( va( "objectivetext%i", i+1 ), inventory.objectiveNames[i].text.c_str() );
		objectiveSystem->SetStateString( va( "objectiveshot%i", i+1 ), inventory.objectiveNames[i].screenshot.c_str() );
	}
	objectiveSystem->StateChanged( gameLocal.time );
}

/*
===============
idPlayer::GiveObjective
===============
*/
void idPlayer::GiveObjective( const char *title, const char *text, const char *screenshot ) {
	idObjectiveInfo info;
	info.title = title;
	info.text = text;
	info.screenshot = screenshot;
	inventory.objectiveNames.Append( info );
	ShowObjective( "newObjective" );
	if ( hud ) {
		hud->HandleNamedEvent( "newObjective" );
	}
}

/*
===============
idPlayer::CompleteObjective
===============
*/
void idPlayer::CompleteObjective( const char *title ) {
	int c = inventory.objectiveNames.Num();
	for ( int i = 0;  i < c; i++ ) {
		if ( idStr::Icmp(inventory.objectiveNames[i].title, title) == 0 ) {
			inventory.objectiveNames.RemoveIndex( i );
			break;
		}
	}
	ShowObjective( "newObjectiveComplete" );

	if ( hud ) {
		hud->HandleNamedEvent( "newObjectiveComplete" );
	}
}

/*
===============
idPlayer::GiveVideo
===============
*/
void idPlayer::GiveVideo( const char *videoName, idDict *item ) {

	if ( videoName == NULL || *videoName == NULL ) {
		return;
	}

	inventory.videos.AddUnique( videoName );

	if ( item ) {
		idItemInfo info;
		info.name = item->GetString( "inv_name" );
		info.icon = item->GetString( "inv_icon" );
		inventory.pickupItemNames.Append( info );
	}
	if ( hud ) {
		hud->HandleNamedEvent( "videoPickup" );
	}
}

/*
===============
idPlayer::GiveSecurity
===============
*/
void idPlayer::GiveSecurity( const char *security ) {
	GetPDA()->SetSecurity( security );
	if ( hud ) {
		hud->SetStateString( "pda_security", "1" );
		hud->HandleNamedEvent( "securityPickup" );
	}
}

/*
===============
idPlayer::GiveEmail
===============
*/
void idPlayer::GiveEmail( const char *emailName ) {

	if ( emailName == NULL || *emailName == NULL ) {
		return;
	}

	inventory.emails.AddUnique( emailName );
	GetPDA()->AddEmail( emailName );

	if ( hud ) {
		hud->HandleNamedEvent( "emailPickup" );
	}
}

/*
===============
idPlayer::GivePDA
===============
*/
void idPlayer::GivePDA( const char *pdaName, idDict *item )
{
	if ( gameLocal.isMultiplayer && spectating ) {
		return;
	}

	// Solarsplace - Arx End Of Sun - Don't update the journal / play sounds if player already has this journal.
	if ( inventory.pdas.Find( pdaName ) )
	{ return; }

	if ( item ) {
		inventory.pdaSecurity.AddUnique( item->GetString( "inv_name" ) );
	}

	if ( pdaName == NULL || *pdaName == NULL ) {
		pdaName = "personal";
	}

	const idDeclPDA *pda = static_cast< const idDeclPDA* >( declManager->FindType( DECL_PDA, pdaName ) );

	inventory.pdas.AddUnique( pdaName );

	// Copy any videos over
	for ( int i = 0; i < pda->GetNumVideos(); i++ ) {
		const idDeclVideo *video = pda->GetVideoByIndex( i );
		if ( video ) {
			inventory.videos.AddUnique( video->GetName() );
		}
	}

	// This is kind of a hack, but it works nicely
	// We don't want to display the 'you got a new pda' message during a map load
	if ( gameLocal.GetFrameNum() > 10 ) {
		if ( pda && hud ) {
			idStr pdaName = pda->GetPdaName();
			pdaName.RemoveColors();
			hud->SetStateString( "pda", "1" );
			hud->SetStateString( "pda_text", pdaName );
			const char *sec = pda->GetSecurity();
			hud->SetStateString( "pda_security", ( sec && *sec ) ? "1" : "0" );
			hud->HandleNamedEvent( "pdaPickup" );
		}

		if ( inventory.pdas.Num() == 1 ) {
			GetPDA()->RemoveAddedEmailsAndVideos();
			if ( !objectiveSystemOpen ) {
				TogglePDA();
			}
			objectiveSystem->HandleNamedEvent( "showPDATip" );
			//ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_firstPDA" ), true );
		}

		if ( inventory.pdas.Num() > 1 && pda->GetNumVideos() > 0 && hud ) {
			hud->HandleNamedEvent( "videoPickup" );
		}
	}

	// Arx - EOS - Solarsplace - 19th May 2012
	// Don't show the 'your journal was updated message' when we give the player his first PDA in code.
	if ( inventory.pdas.Num() > 1 ) {
		
		ShowHudMessage( "#str_general_00001" );

		// Play a sound to indicate journal updated. Cached in player.def
		StartSound( "snd_arx_journal_updated", SND_CHANNEL_ANY, 0, false, NULL );
	}
}

/*
===============
idPlayer::FindInventoryItem
===============
*/
idDict *idPlayer::FindInventoryItem( const char *name ) {
	for ( int i = 0; i < inventory.items.Num(); i++ ) {
		const char *iname = inventory.items[i]->GetString( "inv_name" );
		if ( iname && *iname ) {
			if ( idStr::Icmp( name, iname ) == 0 ) {
				return inventory.items[i];
			}
		}
	}
	return NULL;
}

/*
===============
idPlayer::FindInventoryItemCount
===============
*/
int idPlayer::FindInventoryItemCount( const char *name ) {

	int itemCount = 0;
	idStr name_idstr;
	idStr iname_idstr;

	// SP - 6th Sep 2013 - Make names and string conversions consistent.
	if ( idStr::FindText( name, "#str_" ) == 0 ) {
		name_idstr = common->GetLanguageDict()->GetString( name );
	} else {
		// SP - 7th June 2014 - Corrected bug where non "#str_" would not be matched.
		name_idstr = idStr( name );
	}

	for ( int i = 0; i < inventory.items.Num(); i++ ) {
		const char *iname = inventory.items[i]->GetString( "inv_name" );
		if ( iname && *iname ) {
		
			// SP - 6th Sep 2013 - Make names and string conversions consistent.
			if ( idStr::FindText( iname, "#str_" ) == 0 ) {
				iname_idstr = common->GetLanguageDict()->GetString( iname );
			} else {
				// SP - 31st Oct 2014 - Corrected bug where non "#str_" would not be matched.
				iname_idstr = idStr( iname );
			}

			if ( idStr::Icmp( name_idstr, iname_idstr ) == 0 ) {
				itemCount ++;
			}

		}
	}
	return itemCount;
}

/*
===============
idPlayer::FindInventoryItemIndex - Solarsplace - 2nd July 2010
===============
*/
int idPlayer::FindInventoryItemIndex( const char *name ) {

	for ( int i = 0; i < inventory.items.Num(); i++ ) {
		const char *iname = inventory.items[i]->GetString( "inv_name" );
		if ( iname && *iname ) {
			if ( idStr::Icmp( name, iname ) == 0 ) {
				return i;
			}
		}
	}
	return -1;
}

/*
===============
idPlayer::FindInventoryItemIndexUniqueName - Solarsplace - 26th Nov 2013
===============
*/
int idPlayer::FindInventoryItemIndexUniqueName( const char *uniqueName ) {

	for ( int i = 0; i < inventory.items.Num(); i++ ) {
		const char *iuniqueName = inventory.items[i]->GetString( "inv_unique_name" );
		if ( iuniqueName && *iuniqueName ) {
			if ( idStr::Icmp( iuniqueName, uniqueName ) == 0 ) {
				return i;
			}
		}
	}
	return -1;
}

/*
===============
idPlayer::UpdateWeaponHealth - Solarsplace - 26th Nov 2013
===============
*/
bool idPlayer::UpdateWeaponHealth( void ) {

	//REMOVEME
	gameLocal.Printf( "Entered idPlayer::UpdateWeaponHealth\n" );

	idStr currentWeaponUniqueName;
	int invItemIndex;

	int itemHealth;
	int itemHealthMax;

	// Pull out from the inventory the unique name of the current & last selected weapon.
	currentWeaponUniqueName = inventory.weaponUniqueName;

	// Using the unique name, attempt to find the inventory item number / id.
	invItemIndex = FindInventoryItemIndexUniqueName( currentWeaponUniqueName.c_str() );

	//REMOVEME
	gameLocal.Printf( "idPlayer::UpdateWeaponHealth - currentWeaponUniqueName = %s\n", currentWeaponUniqueName.c_str() );
	gameLocal.Printf( "idPlayer::UpdateWeaponHealth - invItemIndex = %d\n", invItemIndex );

	if ( invItemIndex >= 0 ) {

		// Get the current & max health of the current weapon.
		inventory.items[invItemIndex]->GetInt( "inv_health", "0", itemHealth );
		inventory.items[invItemIndex]->GetInt( "inv_health_max", "100", itemHealthMax );

		// Now we update the weapon class properties.

		//REMOVEME
		gameLocal.Printf( "idPlayer::UpdateWeaponHealth - inv_health = %d\ns", itemHealth );
		gameLocal.Printf( "idPlayer::UpdateWeaponHealth - inv_health_max = %d\n", itemHealthMax );

		// Set the weapon health
		weapon.GetEntity()->health = itemHealth;

		// Set the weapon max health
		weapon.GetEntity()->health_max = itemHealthMax;

		return true;
	} else {
		// The item number / id was not found base on the unique name. Most un-expected :(
		return false;
	}
}

/*
===============
idPlayer::FindInventoryWeaponIndex - Solarsplace - 6nd Aug 2012
===============
*/
int idPlayer::FindInventoryWeaponIndex( int playerWeaponDefNumber, bool checkHealth ) {

	int iWeapon = -1;
	int weaponHealth;

	for ( int i = 0; i < inventory.items.Num(); i++ ) {

		iWeapon = inventory.items[i]->GetInt( "inv_weapon_def" );

		if ( playerWeaponDefNumber == iWeapon ) {

			// SP - 6th Mar 2013 - Optional check the weapon is not broken.
			inventory.items[i]->GetInt( "inv_health", "0", weaponHealth );
			if ( checkHealth && weaponHealth <= 0 ) { continue; }

			return i;
		}
	}
	return -1;
}

/*
===============
idPlayer::UpdateInventoryItem_health - Solarsplace - 22nd Feb 2013
===============
*/
bool idPlayer::UpdateInventoryItem_health( int newWeaponHealth ) {

	idStr weaponHealth;
	idStr weaponUniqueName;

	sprintf( weaponHealth, "%d", newWeaponHealth );
	weaponUniqueName = inventory.weaponUniqueName;

	//gameLocal.Printf( "idPlayer::UpdateInventoryItem_health '%s', 'inv_health', '%s'\n", weaponUniqueName.c_str(), weaponHealth.c_str() );

	return UpdateInventoryItem( weaponUniqueName.c_str() , "inv_health", weaponHealth.c_str() );

}

/*
===============
idPlayer::UpdateInventoryItem_health_max - Solarsplace - 11th Mar 2013
===============
*/
bool idPlayer::UpdateInventoryItem_health_max( int newWeaponHealthMax ) {

	idStr weaponHealth;
	idStr weaponUniqueName;

	sprintf( weaponHealth, "%d", newWeaponHealthMax );
	weaponUniqueName = inventory.weaponUniqueName;

	//gameLocal.Printf( "idPlayer::UpdateInventoryItem_health '%s', 'inv_health_max', '%s'\n", weaponUniqueName.c_str(), weaponHealth.c_str() );

	return UpdateInventoryItem( weaponUniqueName.c_str() , "inv_health_max", weaponHealth.c_str() );

}

/*
===============
idPlayer::GetInventoryItemHealthIcon - Solarsplace - 11th Mar 2013
===============
*/
const char *idPlayer::GetInventoryItemHealthIcon( int health, int health_max, const idDict itemDict  ) {

	int healthPercentage;
	const char *assetName;

	// Prevent divide by 0 exception set health_max to 100 if it is 0.
	if ( health_max == 0 ) { health_max = 100; }

	healthPercentage = ( (float)health / (float)health_max ) * 100; // Just in case health_max is some how 0.

	//gameLocal.Printf( "idPlayer::GetInventoryItemHealthIcon healthPercentage(%d) health(%d) health_max(%d)\n", healthPercentage, health, health_max );

	if ( healthPercentage >= 81 ) {
		assetName = itemDict.GetString( "inv_icon", "" );
	} else if ( healthPercentage >= 61 && healthPercentage <= 80 ) {
		assetName = itemDict.GetString( "inv_icon_damage_1", "" );
	} else if ( healthPercentage >= 41 && healthPercentage <= 60 ) {
		assetName = itemDict.GetString( "inv_icon_damage_2", "" );
	} else if ( healthPercentage >= 21 && healthPercentage <= 40 ) {
		assetName = itemDict.GetString( "inv_icon_damage_3", "" );
	} else if ( healthPercentage >= 1 && healthPercentage <= 20 ) {
		assetName = itemDict.GetString( "inv_icon_damage_4", "" );
	} else if ( healthPercentage <= 0 ) {
		assetName = itemDict.GetString( "inv_icon_damage_5", "" );
	}

	if ( assetName && *assetName ) {
		return assetName;
	} else {
		return itemDict.GetString( "inv_icon", "guis/assets/icons/404_icon.tga" ); // All normal inventory items hit this path.
	}
}

/*
===============
idPlayer::UpdateInventoryItem - Solarsplace - 22nd Feb 2013
===============
*/
bool idPlayer::UpdateInventoryItem( const char *uniqueItemName, const char *dictKey, const char *dictValue) {

	// Used to update inventory item key vals.
	// The inventory item is usually identified via uniqueItemName = inv_unique_name key

	bool updated = false;

	// Loop through all inventory items
	for ( int i = 0; i < inventory.items.Num(); i++ ) {

		// Get the unique name for this item
		const char *inv_uniqueName = inventory.items[i]->GetString( "inv_unique_name" );

		//gameLocal.Printf( "idPlayer::UpdateInventoryItem inventory.items[%d] unique name is '%s' updated\n", i, inv_uniqueName );

		// Do we have a unique inventory name key?
		if ( inv_uniqueName && *inv_uniqueName ) {

			// Does the unique inventory item name match the unique name we are looking for?
			if ( idStr::Icmp( uniqueItemName, inv_uniqueName ) == 0 ) {

				// Add new or update original key val
				inventory.items[i]->Set( dictKey, dictValue );

				//gameLocal.Printf( "idPlayer::UpdateInventoryItem '%s', '%s', '%s' updated\n", uniqueItemName, dictKey, dictValue );

				updated = true;
			}
		}
	}

	return updated;
}

/*
===============
idPlayer::GetInventoryItemValue - Solarsplace - 31st Mar 2013
===============
*/
idStr idPlayer::GetInventoryItemString( const char *uniqueItemName, const char *dictKey ) {

	// The inventory item is usually identified via uniqueItemName = inv_unique_name key

	idStr returnValue = "";

	// Loop through all inventory items
	for ( int i = 0; i < inventory.items.Num(); i++ ) {

		// Get the unique name for this item
		const char *inv_uniqueName = inventory.items[i]->GetString( "inv_unique_name" );

		// Do we have a unique inventory name key?
		if ( inv_uniqueName && *inv_uniqueName ) {

			// Does the unique inventory item name match the unique name we are looking for?
			if ( idStr::Icmp( uniqueItemName, inv_uniqueName ) == 0 ) {

				inventory.items[i]->GetString( idStr(dictKey), "", returnValue );
				break;
			}
		}
	}

	return returnValue;
}

/*
===============
idPlayer::RemoveInventoryItem
===============
*/
void idPlayer::RemoveInventoryItem( const char *name ) {

	idDict *item;

	if ( idStr::FindText( name, "#str_" ) == 0 ) {
		item = FindInventoryItem( common->GetLanguageDict()->GetString( name ) );
	} else {
		item = FindInventoryItem( name );
	}
	
	if ( item ) {
		RemoveInventoryItem( item );
	}
}

// Solarsplace - 29th Dec 2011 - Arx - End Of Sun
void idPlayer::Event_RemoveInventoryItem( const char *name ) {

	idStr message;

	if ( idStr::FindText( name, "#str_" ) == 0 ) {
		sprintf( message, "Inventory item %s removed", common->GetLanguageDict()->GetString( name ) );
	} else {
		sprintf( message, "Inventory item %s removed", name );
	}

	ShowHudMessage( message );

	RemoveInventoryItem( name );
}

/*
===============
idPlayer::RemoveInventoryItem
===============
*/
void idPlayer::RemoveInventoryItem( idDict *item ) {
	inventory.items.Remove( item );
	delete item;
}

/*
===============
idPlayer::GiveItem
===============
*/
void idPlayer::GiveItem( const char *itemname ) {

	// Solarsplace - 29th Dec 2011 - Commented out.

	/*
	idDict args;

	args.Set( "classname", itemname );
	args.Set( "owner", name.c_str() );
	gameLocal.SpawnEntityDef( args );
	if ( hud ) {
		hud->HandleNamedEvent( "itemPickup" );
	}
	*/
}

// Solarsplace - 29th Dec 2011 - Arx - End Of Sun
void idPlayer::Event_GiveInventoryItem( const char *name ) {

	const idDeclEntityDef *shopItemDef = NULL;
	idDict args;
	idStr invName;

	shopItemDef = gameLocal.FindEntityDef( name, false );
	args = shopItemDef->dict;

	if ( args.GetString( "def_dropItem" ) )
	{ args.Set( "inv_classname", args.GetString( "def_dropItem" ) ); }
	else
	{ args.Set( "inv_classname", args.GetString( "classname" ) ); }

	// To be consistent with the GetEntityByViewRay we need to decode the inv_name
	// when an item is picked up from the world inv_name is no longer #str_XXXX
	args.GetString( "inv_name", "", invName );
	if ( idStr::FindText( invName, "#str_" ) == 0 ) {
		invName = common->GetLanguageDict()->GetString( invName );
		args.Set( "inv_name", invName );
	}

	GiveInventoryItem( &args );

	if ( !gameLocal.isNewFrame )
	{ return; } // don't play the sound, but don't report an error

	// Play pickup sound effect
	const idSoundShader *shader;
	const char *sound;
	if ( args.GetString( "snd_acquire", "", &sound ) ) {
		if ( !sound[0] == '\0' ) {
			shader = declManager->FindSound( sound );
			StartSoundShader( shader, SND_CHANNEL_ITEM, 0, false, NULL );
		}
	}

	if ( hud ) {
		hud->HandleNamedEvent( "invPickup" );
		ShowHudMessage( "Item " + invName + " received" );	
	}
}

/*
==================
idPlayer::SlotForWeapon
==================
*/
int idPlayer::SlotForWeapon( const char *weaponName ) {
	int i;

	for( i = 0; i < MAX_WEAPONS; i++ ) {
		const char *weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
		if ( !idStr::Cmp( weap, weaponName ) ) {
			return i;
		}
	}

	// not found
	return -1;
}

/*
===============
idPlayer::Reload
===============
*/
void idPlayer::Reload( void ) {
	if ( gameLocal.isClient ) {
		return;
	}

	if ( spectating || gameLocal.inCinematic || influenceActive ) {
		return;
	}

	if ( weapon.GetEntity() && weapon.GetEntity()->IsLinked() ) {
		weapon.GetEntity()->Reload();
	}
}

/*
===============
idPlayer::NextBestWeapon
===============
*/
void idPlayer::NextBestWeapon( void ) {
	const char *weap;
	int w = MAX_WEAPONS;

	if ( gameLocal.isClient || !weaponEnabled ) {
		return;
	}

	while ( w > 0 ) {
		w--;
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !weap[ 0 ] || ( ( inventory.weapons & ( 1 << w ) ) == 0 ) || ( !inventory.HasAmmo( weap ) ) ) {
			continue;
		}
		if ( !spawnArgs.GetBool( va( "weapon%d_best", w ) ) ) {
			continue;
		}
		break;
	}
	idealWeapon = w;
	weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
	UpdateHudWeapon();
}

/*
===============
idPlayer::NextWeapon
===============
*/
void idPlayer::NextWeapon( void ) {
	const char *weap;
	int w;

	if ( !weaponEnabled || spectating || hiddenWeapon || gameLocal.inCinematic || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || health < 0 ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	// check if we have any weapons
	if ( !inventory.weapons ) {
		return;
	}
	
	w = idealWeapon;
	while( 1 ) {
		w++;
		if ( w >= MAX_WEAPONS ) {
			w = 0;
		} 
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) ) {
			continue;
		}
		if ( !weap[ 0 ] ) {
			continue;
		}
		if ( ( inventory.weapons & ( 1 << w ) ) == 0 ) {
			continue;
		}
		if ( inventory.HasAmmo( weap ) ) {
			break;
		}
	}

	if ( ( w != currentWeapon ) && ( w != idealWeapon ) ) {
		idealWeapon = w;
		weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
		UpdateHudWeapon();
	}
}

/*
===============
idPlayer::PrevWeapon
===============
*/
void idPlayer::PrevWeapon( void ) {
	const char *weap;
	int w;

	if ( !weaponEnabled || spectating || hiddenWeapon || gameLocal.inCinematic || gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) || health < 0 ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	// check if we have any weapons
	if ( !inventory.weapons ) {
		return;
	}

	w = idealWeapon;
	while( 1 ) {
		w--;
		if ( w < 0 ) {
			w = MAX_WEAPONS - 1;
		}
		weap = spawnArgs.GetString( va( "def_weapon%d", w ) );
		if ( !spawnArgs.GetBool( va( "weapon%d_cycle", w ) ) ) {
			continue;
		}
		if ( !weap[ 0 ] ) {
			continue;
		}
		if ( ( inventory.weapons & ( 1 << w ) ) == 0 ) {
			continue;
		}
		if ( inventory.HasAmmo( weap ) ) {
			break;
		}
	}

	if ( ( w != currentWeapon ) && ( w != idealWeapon ) ) {
		idealWeapon = w;
		weaponSwitchTime = gameLocal.time + WEAPON_SWITCH_DELAY;
		UpdateHudWeapon();
	}
}

/*
===============
idPlayer::SelectWeapon
===============
*/
void idPlayer::SelectWeapon( int num, bool force ) {

	// REMOVED
	//gameLocal.Printf("Arx debug -> entered -> idPlayer::SelectWeapon( %i )\n", num);

	const char *weap;

	if ( !weaponEnabled || spectating || gameLocal.inCinematic || health < 0 ) {
		return;
	}

	if ( ( num < 0 ) || ( num >= MAX_WEAPONS ) ) {
		return;
	}

	if ( gameLocal.isClient ) {
		return;
	}

	if ( ( num != weapon_pda ) && gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
		num = weapon_fists;
		hiddenWeapon ^= 1;
		if ( hiddenWeapon && weapon.GetEntity() ) {
			weapon.GetEntity()->LowerWeapon();
		} else {
				weapon.GetEntity()->RaiseWeapon();
		}
	}	

	weap = spawnArgs.GetString( va( "def_weapon%d", num ) );
	if ( !weap[ 0 ] ) {
		gameLocal.Printf( "Invalid weapon\n" );
		return;
	}

	if ( force || ( inventory.weapons & ( 1 << num ) ) ) {

		if ( !inventory.HasAmmo( weap ) && !spawnArgs.GetBool( va( "weapon%d_allowempty", num ) ) ) {
			return;
		}

		if ( ( previousWeapon >= 0 ) && ( idealWeapon == num ) && ( spawnArgs.GetBool( va( "weapon%d_toggle", num ) ) ) ) {

			weap = spawnArgs.GetString( va( "def_weapon%d", previousWeapon ) );
			if ( !inventory.HasAmmo( weap ) && !spawnArgs.GetBool( va( "weapon%d_allowempty", previousWeapon ) ) ) {
				return;
			}

			// Solarsplace - 6th Sep 2012 - Make sure the player has not dropped previous weapon from inventory.
			// If after the drop we no longer have any weapons of this type in the inventory, then select the fists.
			// Solarsplace - 6th Mar 2013 - Added weapon health check so that we do not select any broken weapons. 
			if ( FindInventoryWeaponIndex( previousWeapon, true ) != -1 )
			{
				idealWeapon = previousWeapon;

			} else {
				idealWeapon = 0; // Fists
			}
			

		} else if ( ( weapon_pda >= 0 ) && ( num == weapon_pda ) && ( inventory.pdas.Num() == 0 ) ) {

			// 1st Jan 2010 - Solarsplace - Prevent this message window from being displayed.
			//ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_noPDA" ), true );
			return;

		} else {

			idealWeapon = num;
		}

		// Solarsplace - Arx End Of Sun - 26th Nov 2013
		UpdateWeaponHealth();

		UpdateHudWeapon();
	}

	//REMOVED
	//gameLocal.Printf("Arx debug -> left -> idPlayer::SelectWeapon( %i )\n", num);
}

/*
=================
idPlayer::DropWeapon
=================
*/
void idPlayer::DropWeapon( bool died ) {
	idVec3 forward, up;
	int inclip, ammoavailable;

	assert( !gameLocal.isClient );
	
	if ( spectating || weaponGone || weapon.GetEntity() == NULL ) {
		return;
	}
	
	if ( ( !died && !weapon.GetEntity()->IsReady() ) || weapon.GetEntity()->IsReloading() ) {
		return;
	}
	// ammoavailable is how many shots we can fire
	// inclip is which amount is in clip right now
	ammoavailable = weapon.GetEntity()->AmmoAvailable();
	inclip = weapon.GetEntity()->AmmoInClip();
	
	// don't drop a grenade if we have none left
	if ( !idStr::Icmp( idWeapon::GetAmmoNameForNum( weapon.GetEntity()->GetAmmoType() ), "ammo_grenades" ) && ( ammoavailable - inclip <= 0 ) ) {
		return;
	}

	// expect an ammo setup that makes sense before doing any dropping
	// ammoavailable is -1 for infinite ammo, and weapons like chainsaw
	// a bad ammo config usually indicates a bad weapon state, so we should not drop
	// used to be an assertion check, but it still happens in edge cases
	if ( ( ammoavailable != -1 ) && ( ammoavailable - inclip < 0 ) ) {
		common->DPrintf( "idPlayer::DropWeapon: bad ammo setup\n" );
		return;
	}
	idEntity *item = NULL;
	if ( died ) {
		// ain't gonna throw you no weapon if I'm dead
		item = weapon.GetEntity()->DropItem( vec3_origin, 0, WEAPON_DROP_TIME, died );
	} else {
		viewAngles.ToVectors( &forward, NULL, &up );
		item = weapon.GetEntity()->DropItem( 250.0f * forward + 150.0f * up, 500, WEAPON_DROP_TIME, died );
	}
	if ( !item ) {
		return;
	}
	// set the appropriate ammo in the dropped object
	const idKeyValue * keyval = item->spawnArgs.MatchPrefix( "inv_ammo_" );
	if ( keyval ) {
		item->spawnArgs.SetInt( keyval->GetKey(), ammoavailable );
		idStr inclipKey = keyval->GetKey();
		inclipKey.Insert( "inclip_", 4 );
		item->spawnArgs.SetInt( inclipKey, inclip );
	}
	if ( !died ) {
		// remove from our local inventory completely
		inventory.Drop( spawnArgs, item->spawnArgs.GetString( "inv_weapon" ), -1 );
		weapon.GetEntity()->ResetAmmoClip();
		NextWeapon();
		weapon.GetEntity()->WeaponStolen();
		weaponGone = true;
	}
}

/*
=================
idPlayer::StealWeapon
steal the target player's current weapon
=================
*/
void idPlayer::StealWeapon( idPlayer *player ) {
	assert( !gameLocal.isClient );

	// make sure there's something to steal
	idWeapon *player_weapon = static_cast< idWeapon * >( player->weapon.GetEntity() );
	if ( !player_weapon || !player_weapon->CanDrop() || weaponGone ) {
		return;
	}
	// steal - we need to effectively force the other player to abandon his weapon
	int newweap = player->currentWeapon;
	if ( newweap == -1 ) {
		return;
	}
	// might be just dropped - check inventory
	if ( ! ( player->inventory.weapons & ( 1 << newweap ) ) ) {
		return;
	}
	const char *weapon_classname = spawnArgs.GetString( va( "def_weapon%d", newweap ) );
	assert( weapon_classname );
	int ammoavailable = player->weapon.GetEntity()->AmmoAvailable();
	int inclip = player->weapon.GetEntity()->AmmoInClip();
	if ( ( ammoavailable != -1 ) && ( ammoavailable - inclip < 0 ) ) {
		// see DropWeapon
		common->DPrintf( "idPlayer::StealWeapon: bad ammo setup\n" );
		// we still steal the weapon, so let's use the default ammo levels
		inclip = -1;
		const idDeclEntityDef *decl = gameLocal.FindEntityDef( weapon_classname );
		assert( decl );
		const idKeyValue *keypair = decl->dict.MatchPrefix( "inv_ammo_" );
		assert( keypair );
		ammoavailable = atoi( keypair->GetValue() );
	}

	player->weapon.GetEntity()->WeaponStolen();
	player->inventory.Drop( player->spawnArgs, NULL, newweap );
	player->SelectWeapon( weapon_fists, false );
	// in case the robbed player is firing rounds with a continuous fire weapon like the chaingun/plasma etc.
	// this will ensure the firing actually stops
	player->weaponGone = true;

	// give weapon, setup the ammo count
	Give( "weapon", weapon_classname );
	ammo_t ammo_i = player->inventory.AmmoIndexForWeaponClass( weapon_classname, NULL );
	idealWeapon = newweap;
	inventory.ammo[ ammo_i ] += ammoavailable;
	inventory.clip[ newweap ] = inclip;
}

/*
===============
idPlayer::ActiveGui
===============
*/
idUserInterface *idPlayer::ActiveGui( void ) {
	if ( objectiveSystemOpen ) {
		return objectiveSystem;
	}

	// Solarsplace 11th April 2010 - Inventory related
	if ( inventorySystemOpen )
	{
		return inventorySystem;
	}

	// Solarsplace 6th May 2010 - Journal related
	if ( journalSystemOpen )
	{
		return journalSystem;
	}

	// Solarsplace 11th June 2010 - Readable related
	if ( readableSystemOpen )
	{
		return readableSystem;
	}

	// Solarsplace 2nd Nov 2011 - NPC GUI related
	if ( conversationSystemOpen )
	{
		return conversationSystem;
	}

	// Solarsplace 6th Nov 2011 - Shop GUI related
	if ( shoppingSystemOpen )
	{
		return shoppingSystem;
	}


	return focusUI;
}

/*
===============
idPlayer::Weapon_Combat
===============
*/
void idPlayer::Weapon_Combat( void ) {
	if ( influenceActive || !weaponEnabled || gameLocal.inCinematic || privateCameraView ) {
		return;
	}

	weapon.GetEntity()->RaiseWeapon();
	if ( weapon.GetEntity()->IsReloading() ) {
		if ( !AI_RELOAD ) {
			AI_RELOAD = true;
			SetState( "ReloadWeapon" );
			UpdateScript();
		}
	} else {
		AI_RELOAD = false;
	}

	if ( idealWeapon == weapon_soulcube && soulCubeProjectile.GetEntity() != NULL ) {
		idealWeapon = currentWeapon;
	}

	if ( idealWeapon != currentWeapon ) {
		if ( weaponCatchup ) {
			assert( gameLocal.isClient );

			currentWeapon = idealWeapon;
			weaponGone = false;
			animPrefix = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
			weapon.GetEntity()->GetWeaponDef( animPrefix, inventory.clip[ currentWeapon ] );
			animPrefix.Strip( "weapon_" );

			weapon.GetEntity()->NetCatchup();
			const function_t *newstate = GetScriptFunction( "NetCatchup" );
			if ( newstate ) {
				SetState( newstate );
				UpdateScript();
			}
			weaponCatchup = false;			
		} else {
			if ( weapon.GetEntity()->IsReady() ) {
				weapon.GetEntity()->PutAway();
			}

			if ( weapon.GetEntity()->IsHolstered() ) {
				assert( idealWeapon >= 0 );
				assert( idealWeapon < MAX_WEAPONS );

				if ( currentWeapon != weapon_pda && !spawnArgs.GetBool( va( "weapon%d_toggle", currentWeapon ) ) ) {
					previousWeapon = currentWeapon;
				}
				currentWeapon = idealWeapon;
				weaponGone = false;
				animPrefix = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
				weapon.GetEntity()->GetWeaponDef( animPrefix, inventory.clip[ currentWeapon ] );
				animPrefix.Strip( "weapon_" );

				weapon.GetEntity()->Raise();
			}
		}
	} else {
		weaponGone = false;	// if you drop and re-get weap, you may miss the = false above 
		if ( weapon.GetEntity()->IsHolstered() ) {
			if ( !weapon.GetEntity()->AmmoAvailable() ) {
				// weapons can switch automatically if they have no more ammo
				NextBestWeapon();
			} else {
				weapon.GetEntity()->Raise();
				state = GetScriptFunction( "RaiseWeapon" );
				if ( state ) {
					SetState( state );
				}
			}
		}
	}

	// check for attack
	AI_WEAPON_FIRED = false;
	if ( !influenceActive ) {
		if ( ( usercmd.buttons & BUTTON_ATTACK ) && !weaponGone ) {
			FireWeapon();
		} else if ( oldButtons & BUTTON_ATTACK ) {
			AI_ATTACK_HELD = false;

			// Start - Solarsplace 15th May 2010 - Magic related - Arx EOS
			// This condition is ESSENTIAL to prevent the the magic attack being instantly stopped because it was not called via a button or impulse.
			if ( !magicAttackInProgress )
			{ weapon.GetEntity()->EndAttack(); }
			// End - Solarsplace 15th May 2010 - Magic related - Arx EOS
		}
	}

	// update our ammo clip in our inventory
	if ( ( currentWeapon >= 0 ) && ( currentWeapon < MAX_WEAPONS ) ) {
		inventory.clip[ currentWeapon ] = weapon.GetEntity()->AmmoInClip();
		if ( hud && ( currentWeapon == idealWeapon ) ) {
			UpdateHudAmmo( hud );
		}
	}
}

/*
===============
idPlayer::Weapon_NPC
===============
*/
void idPlayer::Weapon_NPC( void ) {

	// Solarsplace - Arx End Of Sun - 31st May 2012
	// Alter code so that 'use' button instigates NPC GUI and not fire button so we
	// can still easily attack NPC's with GUI's if we want to

	/*
	if ( idealWeapon != currentWeapon ) {
		Weapon_Combat();
	}
	StopFiring();
	weapon.GetEntity()->LowerWeapon();

	if ( ( usercmd.buttons & BUTTON_ATTACK ) && !( oldButtons & BUTTON_ATTACK ) ) {
		buttonMask |= BUTTON_ATTACK;
	*/

		// SP - Arx EOS - Prevent talking to an AI that has an enemy (may be attacking player?)
		if ( focusCharacter->GetEnemy() )
		{
			return;
		}

		// SP - Arx EOS - NPC GUI
		idStr charactersGui = focusCharacter->spawnArgs.GetString( "characters_gui", "" );
		if ( charactersGui != "" )
		{
			conversationSystem = uiManager->FindGui( focusCharacter->spawnArgs.GetString( "characters_gui" ), true, false, true );

			// Get this characters quest id if they have one and store the id in variable conversationWindowQuestId
			focusCharacter->spawnArgs.GetString( "character_quest_id", "0", conversationWindowQuestId );

			ToggleConversationSystem();
		}

		focusCharacter->TalkTo( this );
	//}
}

/*
===============
idPlayer::LowerWeapon
===============
*/
void idPlayer::LowerWeapon( void ) {
	if ( weapon.GetEntity() && !weapon.GetEntity()->IsHidden() ) {
		weapon.GetEntity()->LowerWeapon();
	}
}

/*
===============
idPlayer::RaiseWeapon
===============
*/
void idPlayer::RaiseWeapon( void ) {
	if ( weapon.GetEntity() && weapon.GetEntity()->IsHidden() ) {
		weapon.GetEntity()->RaiseWeapon();
	}
}

/*
===============
idPlayer::WeaponLoweringCallback
===============
*/
void idPlayer::WeaponLoweringCallback( void ) {
	SetState( "LowerWeapon" );
	UpdateScript();
}

/*
===============
idPlayer::WeaponRisingCallback
===============
*/
void idPlayer::WeaponRisingCallback( void ) {
	SetState( "RaiseWeapon" );
	UpdateScript();
}

/*
===============
idPlayer::Weapon_GUI
===============
*/
void idPlayer::Weapon_GUI( void ) {

	if ( !objectiveSystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// Solarsplace 11th April 2010 - Inventory related
	if ( !inventorySystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// Solarsplace 6th May 2010 - Journal related
	if ( !journalSystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// Solarsplace 6th May 2010 - Readable related
	if ( !readableSystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// Solarsplace 2nd Nov 2011 - NPC GUI related
	if ( !conversationSystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// Solarsplace 2nd Nov 2011 - NPC GUI related
	if ( !shoppingSystemOpen ) {
		if ( idealWeapon != currentWeapon ) {
			Weapon_Combat();
		}
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
	}

	// disable click prediction for the GUIs. handy to check the state sync does the right thing
	if ( gameLocal.isClient && !net_clientPredictGUI.GetBool() ) {
		return;
	}

	if ( ( oldButtons ^ usercmd.buttons ) & BUTTON_ATTACK ) {
		sysEvent_t ev;
		const char *command = NULL;
		bool updateVisuals = false;

		idUserInterface *ui = ActiveGui();
		if ( ui ) {
			ev = sys->GenerateMouseButtonEvent( 1, ( usercmd.buttons & BUTTON_ATTACK ) != 0 );
			command = ui->HandleEvent( &ev, gameLocal.time, &updateVisuals );
			if ( updateVisuals && focusGUIent && ui == focusUI ) {
				focusGUIent->UpdateVisuals();
			}
		}
		if ( gameLocal.isClient ) {
			// we predict enough, but don't want to execute commands
			return;
		}
		if ( focusGUIent ) {
			HandleGuiCommands( focusGUIent, command );
		} else {
			HandleGuiCommands( this, command );
		}
	}
}

/*
===============
idPlayer::UpdateWeapon
===============
*/
void idPlayer::UpdateWeapon( void ) {
	if ( health <= 0 ) {
		return;
	}

	assert( !spectating );

	if ( gameLocal.isClient ) {
		// clients need to wait till the weapon and it's world model entity
		// are present and synchronized ( weapon.worldModel idEntityPtr to idAnimatedEntity )
		if ( !weapon.GetEntity()->IsWorldModelReady() ) {
			return;
		}
	}

	// always make sure the weapon is correctly setup before accessing it
	if ( !weapon.GetEntity()->IsLinked() ) {
		if ( idealWeapon != -1 ) {
			animPrefix = spawnArgs.GetString( va( "def_weapon%d", idealWeapon ) );
			weapon.GetEntity()->GetWeaponDef( animPrefix, inventory.clip[ idealWeapon ] );
			assert( weapon.GetEntity()->IsLinked() );
		} else {
			return;
		}
	}

	if ( hiddenWeapon && tipUp && usercmd.buttons & BUTTON_ATTACK ) {
		HideTip();
	}
	
	if ( g_dragEntity.GetBool() ) {
		StopFiring();
		weapon.GetEntity()->LowerWeapon();
		dragEntity.Update( this );
	} else if ( ActiveGui() ) {
		// gui handling overrides weapon use
		Weapon_GUI();

	// Solarsplace - Arx End Of Sun - 31st May 2012
	// Functionality requires the 'use' key now.
	/*
	} else 	if ( focusCharacter && ( focusCharacter->health > 0 ) ) {
		Weapon_NPC();
	*/

	} else {
		Weapon_Combat();
	}
	
	if ( hiddenWeapon ) {
		weapon.GetEntity()->LowerWeapon();
	}

	// update weapon state, particles, dlights, etc
	weapon.GetEntity()->PresentWeapon( showWeaponViewModel );
}

/*
===============
idPlayer::SpectateFreeFly
===============
*/
void idPlayer::SpectateFreeFly( bool force ) {
	idPlayer	*player;
	idVec3		newOrig;
	idVec3		spawn_origin;
	idAngles	spawn_angles;

	player = gameLocal.GetClientByNum( spectator );
	if ( force || gameLocal.time > lastSpectateChange ) {
		spectator = entityNumber;
		if ( player && player != this && !player->spectating && !player->IsInTeleport() ) {
			newOrig = player->GetPhysics()->GetOrigin();
			if ( player->physicsObj.IsCrouching() ) {
				newOrig[ 2 ] += pm_crouchviewheight.GetFloat();
			} else {
				newOrig[ 2 ] += pm_normalviewheight.GetFloat();
			}
			newOrig[ 2 ] += SPECTATE_RAISE;
			idBounds b = idBounds( vec3_origin ).Expand( pm_spectatebbox.GetFloat() * 0.5f );
			idVec3 start = player->GetPhysics()->GetOrigin();
			start[2] += pm_spectatebbox.GetFloat() * 0.5f;
			trace_t t;
			// assuming spectate bbox is inside stand or crouch box
			gameLocal.clip.TraceBounds( t, start, newOrig, b, MASK_PLAYERSOLID, player );
			newOrig.Lerp( start, newOrig, t.fraction );
			SetOrigin( newOrig );
			idAngles angle = player->viewAngles;
			angle[ 2 ] = 0;
			SetViewAngles( angle );
		} else {	
			SelectInitialSpawnPoint( spawn_origin, spawn_angles );
			spawn_origin[ 2 ] += pm_normalviewheight.GetFloat();
			spawn_origin[ 2 ] += SPECTATE_RAISE;
			SetOrigin( spawn_origin );
			SetViewAngles( spawn_angles );
		}
		lastSpectateChange = gameLocal.time + 500;
	}
}

/*
===============
idPlayer::SpectateCycle
===============
*/
void idPlayer::SpectateCycle( void ) {
	idPlayer *player;

	if ( gameLocal.time > lastSpectateChange ) {
		int latchedSpectator = spectator;
		spectator = gameLocal.GetNextClientNum( spectator );
		player = gameLocal.GetClientByNum( spectator );
		assert( player ); // never call here when the current spectator is wrong
		// ignore other spectators
		while ( latchedSpectator != spectator && player->spectating ) {
			spectator = gameLocal.GetNextClientNum( spectator );
			player = gameLocal.GetClientByNum( spectator );
		}
		lastSpectateChange = gameLocal.time + 500;
	}
}

/*
===============
idPlayer::UpdateSpectating
===============
*/
void idPlayer::UpdateSpectating( void ) {
	assert( spectating );
	assert( !gameLocal.isClient );
	assert( IsHidden() );
	idPlayer *player;
	if ( !gameLocal.isMultiplayer ) {
		return;
	}
	player = gameLocal.GetClientByNum( spectator );
	if ( !player || ( player->spectating && player != this ) ) {
		SpectateFreeFly( true );
	} else if ( usercmd.upmove > 0 ) {
		SpectateFreeFly( false );
	} else if ( usercmd.buttons & BUTTON_ATTACK ) {
		SpectateCycle();
	}
}

/*
===============
idPlayer::HandleSingleGuiCommand
===============
*/
bool idPlayer::HandleSingleGuiCommand( idEntity *entityGui, idLexer *src ) {
	idToken token;

	// Solarsplace 14th April 2010 - Inventory related
	idToken token2;

	if ( !src->ReadToken( &token ) ) {
		return false;
	}

	if ( token == ";" ) {
		return false;
	}

	if ( token.Icmp( "addhealth" ) == 0 ) {
		if ( entityGui && health < 100 ) {
			int _health = entityGui->spawnArgs.GetInt( "gui_parm1" );
			int amt = ( _health >= HEALTH_PER_DOSE ) ? HEALTH_PER_DOSE : _health;
			_health -= amt;
			entityGui->spawnArgs.SetInt( "gui_parm1", _health );
			if ( entityGui->GetRenderEntity() && entityGui->GetRenderEntity()->gui[ 0 ] ) {
				entityGui->GetRenderEntity()->gui[ 0 ]->SetStateInt( "gui_parm1", _health );
			}
			health += amt;
			if ( health > 100 ) {
				health = 100;
			}
		}
		return true;
	}

	if ( token.Icmp( "ready" ) == 0 ) {
		PerformImpulse( IMPULSE_17 );
		return true;
	}

	if ( token.Icmp( "updatepda" ) == 0 ) {
		UpdatePDAInfo( true );
		return true;
	}

	if ( token.Icmp( "updatepda2" ) == 0 ) {
		UpdatePDAInfo( false );
		return true;
	}

	if ( token.Icmp( "stoppdavideo" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaVideoWave.Length() > 0 ) {
			StopSound( SND_CHANNEL_PDA, false );
		}
		return true;
	}

	// Solarsplace 11th April 2010 - Inventory related
	// The token 'close' DOES NOT make it through to here!
	// Wrote a new token to handle this just below here 'shutinventory' :

	if ( token.Icmp( "close" ) == 0 ) {

		if ( objectiveSystem && objectiveSystemOpen ) {
			TogglePDA();
		}
	}

	if ( token.Icmp( "shutinventory" ) == 0 ) {

		// Solarsplace 11th April 2010 - Inventory related
		if ( inventorySystemOpen ) {
			ToggleInventorySystem();
		}
	}

	if ( token.Icmp( "shutjournal" ) == 0 ) {

		// Solarsplace 6th May 2010 - Journal related
		if ( journalSystemOpen ) {
			ToggleJournalSystem();
		}
	}

	if ( token.Icmp( "shutreadable" ) == 0 ) {

		// Solarsplace 6th May 2010 - Journal related
		if ( readableSystemOpen ) {
			ToggleReadableSystem();
		}
	}

	if ( token.Icmp( "shutnpcgui" ) == 0 ) {

		// Solarsplace 2nd Nov 2011 - NPC GUI related
		if ( conversationSystemOpen ) {
			ToggleConversationSystem();
		}
	}

	if ( token.Icmp( "shutshopgui" ) == 0 ) {

		// Solarsplace 6th Nov 2011 - Shop GUI related
		if ( shoppingSystemOpen ) {

			//ToggleShoppingSystem();

			// If there is an associated chest (door) then shut it
			if ( lastShopEntity ) {
				lastShopEntity->ActivateTargets( gameLocal.GetLocalPlayer() ); // If the shop is a door / chest then shut it!
			}
		}
	}

	if ( token.Icmp( "playpdavideo" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaVideo.Length() > 0 ) {
			const idMaterial *mat = declManager->FindMaterial( pdaVideo );
			if ( mat ) {
				int c = mat->GetNumStages();
				for ( int i = 0; i < c; i++ ) {
					const shaderStage_t *stage = mat->GetStage(i);
					if ( stage && stage->texture.cinematic ) {
						stage->texture.cinematic->ResetTime( gameLocal.time );
					}
				}
				if ( pdaVideoWave.Length() ) {
					const idSoundShader *shader = declManager->FindSound( pdaVideoWave );
					StartSoundShader( shader, SND_CHANNEL_PDA, 0, false, NULL );
				}
			}
		}
	}

	if ( token.Icmp( "playpdaaudio" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaAudio.Length() > 0 ) {
			const idSoundShader *shader = declManager->FindSound( pdaAudio );
			int ms;
			StartSoundShader( shader, SND_CHANNEL_PDA, 0, false, &ms );
			StartAudioLog();
			CancelEvents( &EV_Player_StopAudioLog );
			PostEventMS( &EV_Player_StopAudioLog, ms + 150 );
		}
		return true;
	}

	if ( token.Icmp( "stoppdaaudio" ) == 0 ) {
		if ( objectiveSystem && objectiveSystemOpen && pdaAudio.Length() > 0 ) {
			// idSoundShader *shader = declManager->FindSound( pdaAudio );
			StopAudioLog();
			StopSound( SND_CHANNEL_PDA, false );
		}
		return true;
	}

	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/

	const idKeyValue *kv;

	// Solarsplace 24th Jun 2014 - Shop related
	if ( token.Icmp( "shop_sellitem" ) == 0 ) {

		//REMOVEME
		gameLocal.Printf( "idPlayer::HandleSingleGuiCommand - shop_sellitem\n" );

		if ( src->ReadToken( &token2 ) ) {

			kv = invItemGroupPointer->GetKeyVal( atoi( token2 ) );

			if ( kv ) {

				int invItemIndex = atoi( kv->GetValue() );

				// Items which the player cannot sell from their inventory
				bool noSell = inventory.items[invItemIndex]->GetBool( "inv_arx_noinvdrop", "0" );
				if ( noSell ) {
					ShowHudMessage( "#str_general_00012" ); // "This inventory item can not be sold"
					StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
					return true;
				}

				// Populate dictionary with key vals of selling item
				idDict *sellingItem = inventory.items[invItemIndex];

				// Get the class name of the item we are selling
				idStr sellingClassName = inventory.items[invItemIndex]->GetString( "inv_classname" );

				// Get the base value of the item we are selling
				float itemValue = sellingItem->GetFloat( "inv_shop_item_value", "0" );

				// Get the health of the item we are selling	
				int tempHealth = inventory.items[invItemIndex]->GetInt( "inv_health", "100" );
				int tempHealthMax = inventory.items[invItemIndex]->GetInt( "inv_health_max", "100" );
				if ( tempHealthMax == 0 ) { tempHealthMax = 100; } // Safety just in case it somehow gets set 0 to avoid divide by zero errors...

				float durabilityRatio = ( (float)tempHealth / (float)tempHealthMax ) * 100;

				//TODO - Abort sale if item too broken?

				int buyFromPlayerPrice = ShopGetBuyFromPlayerPrice( itemValue, durabilityRatio, arxShopFunctions.ratioBuyFromPlayer );

				// Add the item we are selling to the current open shop
				if (arxShopFunctions.AddShopItem( sellingClassName.c_str() ) ) {

					// Selling the weapon the player is currently using?
					if ( strcmp( inventory.weaponUniqueName, sellingItem->GetString( "inv_unique_name" ) ) == 0 ) {

							// Clear the inventory / weapon unique name
							inventory.weaponUniqueName = "";

							// Now select the fists
							SelectWeapon( ARX_FISTS_WEAPON, true );
					}

					// Un-equip the item if equiped (if any match)
					int i;
					for ( i = 0; i < ARX_EQUIPED_ITEMS_MAX; i++ ) {
						if ( inventory.arx_equiped_items[ i ] == sellingItem->GetString( "inv_unique_name" ) ) {
							inventory.arx_equiped_items[ i ] == "";
						}
					}

					// Now remove the item from the players inventory
					RemoveInventoryItem( sellingItem ); 

					// Now pay the player
					inventory.money += buyFromPlayerPrice; //itemValue;
					StartSound( "snd_shop_success", SND_CHANNEL_ANY, 0, false, NULL );
				}
			}
		}
		return true;

	}

	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/

	// Solarsplace 17th Nov 2011 - Shop related
	if ( token.Icmp( "shop_buyitem" ) == 0 ) {

		//gameLocal.Printf( "idPlayer::HandleSingleGuiCommand - shop_buyitem\n" );

		//*** This is for inventory sell
		//if ( arxShopFunctions.totalUsedShopSlots < MAX_INVENTORY_ITEMS )
		//{

			if ( src->ReadToken( &token2 ) ) {

				int itemPrice = atoi( arxShopFunctions.shopSlotItem_Dict->GetString( va( "inv_shop_item_value_%i", atoi( token2 ) ), "") );

				//gameLocal.Printf( "itemPrice = %i\n", itemPrice );

				if ( inventory.money - itemPrice >= 0 )
				{
					// !!! Must do the steps in this order !!!

					// Add the item to the players inventory
					const idDeclEntityDef *shopItemDef = NULL;
					shopItemDef = gameLocal.FindEntityDef( arxShopFunctions.shopSlotItem_Dict->GetString( va( "shop_item_class_%i", atoi( token2 ) ), ""), false );

					if ( !shopItemDef  ) {
						// Nothing to buy / sell
						StartSound( "snd_shop_fail", SND_CHANNEL_ANY, 0, false, NULL );
						return true;
					}

					idDict args = shopItemDef->dict;

					// Solarsplace 9th Oct 2011 - If we specify a drop item use that.
					// This is because the item we may 'get' is static, but it must be a moveable variant to be dropped from the inventory on most occasions.
					if ( args.GetString( "def_dropItem" ) )
					{ args.Set( "inv_classname", args.GetString( "def_dropItem" ) ); }
					else
					{ args.Set( "inv_classname", args.GetString( "classname" ) ); }

					GiveInventoryItem( &args );

					// Spend money and remove the item from the shop
					int sellToPlayerPrice = ShopGetSellToPlayerPrice( itemPrice, 100.0f, arxShopFunctions.ratioSellToPlayer );

					inventory.money -= sellToPlayerPrice; //itemPrice;
					arxShopFunctions.RemoveShopItem( atoi( token2 )  );
					StartSound( "snd_shop_success", SND_CHANNEL_ANY, 0, false, NULL );
				}
				else
				{ StartSound( "snd_shop_fail", SND_CHANNEL_ANY, 0, false, NULL ); }
			}

		//}

		return true;
	}

	// Solarsplace 11th April 2010 - Inventory related
	if ( token.Icmp( "inventoryitemuse" ) == 0 ) {

		//gameLocal.Printf( "idPlayer::HandleSingleGuiCommand - inventoryitemuse\n" );

		if ( src->ReadToken( &token2 ) ) {

			kv = invItemGroupPointer->GetKeyVal( atoi( token2 ) );

			if ( kv ) {
				//gameLocal.Printf( "&token2 = %s\n", token2.c_str() );
				ConsumeInventoryItem( atoi( kv->GetValue() ) );
			}
		}
		return true;
	}

	// Solarsplace 15th April 2010 - Inventory related
	if ( token.Icmp( "inventoryitemdrop" ) == 0 ) {

		//gameLocal.Printf( "idPlayer::HandleSingleGuiCommand - inventoryitemdrop\n" );

		if ( src->ReadToken( &token2 ) ) {

			kv = invItemGroupPointer->GetKeyVal( atoi( token2 ) );

			if ( kv ) {
				//REMOVED
				//gameLocal.Printf( "&token2 = %s\n", token2.c_str() );
				DropInventoryItem( atoi( kv->GetValue() ) );
			}
		}
		return true;
	}

	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	// *** Journal

	//gameLocal.Printf( "Journal updated at time %d\n", gameLocal.time );
	//gameLocal.Printf( "Journal token is %s\n", token.c_str() );

	// *** Decrement values
	if ( token.Icmp( "arx_attr_strength_dec" ) == 0 ) {
		inventory.tmp_arx_attribute_points ++;
		inventory.tmp_arx_attr_strength --;
		return true;
	}

	if ( token.Icmp( "arx_attr_mental_dec" ) == 0 ) {
		inventory.tmp_arx_attribute_points ++;
		inventory.tmp_arx_attr_mental --;
		return true;
	}

	if ( token.Icmp( "arx_attr_dexterity_dec" ) == 0 ) {
		inventory.tmp_arx_attribute_points ++;
		inventory.tmp_arx_attr_dexterity --;
		return true;
	}

	if ( token.Icmp( "arx_attr_constitution_dec" ) == 0 ) {
		inventory.tmp_arx_attribute_points ++;
		inventory.tmp_arx_attr_constitution --;
		return true;
	}

	if ( token.Icmp( "arx_skill_casting_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_casting --;
		return true;
	}

	if ( token.Icmp( "arx_skill_close_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_close_combat --;
		return true;
	}

	if ( token.Icmp( "arx_skill_defense_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_defense --;
		return true;
	}

	if ( token.Icmp( "arx_skill_ethereal_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_ethereal_link --;
		return true;
	}

	if ( token.Icmp( "arx_skill_intuition_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_intuition --;
		return true;
	}

	if ( token.Icmp( "arx_skill_intelligence_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_intelligence --;
		return true;
	}

	if ( token.Icmp( "arx_skill_projectile_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_projectile --;
		return true;
	}

	if ( token.Icmp( "arx_skill_stealth_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_stealth --;
		return true;
	}

	if ( token.Icmp( "arx_skill_technical_dec" ) == 0 ) {
		inventory.tmp_arx_skill_points ++;
		inventory.tmp_arx_skill_technical --;
		return true;
	}

	// *** Increment values
	if ( token.Icmp( "arx_attr_strength_inc" ) == 0 ) {
		inventory.tmp_arx_attribute_points --;
		inventory.tmp_arx_attr_strength ++;
		return true;
	}

	if ( token.Icmp( "arx_attr_mental_inc" ) == 0 ) {
		inventory.tmp_arx_attribute_points --;
		inventory.tmp_arx_attr_mental ++;
		return true;
	}

	if ( token.Icmp( "arx_attr_dexterity_inc" ) == 0 ) {
		inventory.tmp_arx_attribute_points --;
		inventory.tmp_arx_attr_dexterity ++;
		return true;
	}

	if ( token.Icmp( "arx_attr_constitution_inc" ) == 0 ) {
		inventory.tmp_arx_attribute_points --;
		inventory.tmp_arx_attr_constitution ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_casting_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_casting ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_close_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_close_combat ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_defense_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_defense ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_ethereal_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_ethereal_link ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_intuition_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_intuition ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_intelligence_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_intelligence ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_projectile_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_projectile ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_stealth_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_stealth ++;
		return true;
	}

	if ( token.Icmp( "arx_skill_technical_inc" ) == 0 ) {
		inventory.tmp_arx_skill_points --;
		inventory.tmp_arx_skill_technical ++;
		return true;
	}

	// *** Apply values
	if ( token.Icmp( "arx_apply_attr_skill_points" ) == 0 ) {

		inventory.arx_attribute_points = 0;
		inventory.tmp_arx_attribute_points = 0;

		inventory.arx_skill_points = 0;
		inventory.tmp_arx_skill_points = 0;

		inventory.arx_attr_strength = inventory.tmp_arx_attr_strength;
		inventory.arx_attr_mental = inventory.tmp_arx_attr_mental;
		inventory.arx_attr_dexterity = inventory.tmp_arx_attr_dexterity;
		inventory.arx_attr_constitution = inventory.tmp_arx_attr_constitution;

		inventory.arx_skill_casting = inventory.tmp_arx_skill_casting;
		inventory.arx_skill_close_combat = inventory.tmp_arx_skill_close_combat;
		inventory.arx_skill_defense = inventory.tmp_arx_skill_defense;
		inventory.arx_skill_ethereal_link = inventory.tmp_arx_skill_ethereal_link;
		inventory.arx_skill_intuition = inventory.tmp_arx_skill_intuition;
		inventory.arx_skill_intelligence = inventory.tmp_arx_skill_intelligence;
		inventory.arx_skill_projectile = inventory.tmp_arx_skill_projectile;
		inventory.arx_skill_stealth = inventory.tmp_arx_skill_stealth;
		inventory.arx_skill_technical = inventory.tmp_arx_skill_technical;

		ShowHudMessage( "#str_book_00005" ); // "Your points have been applied!"

		return true;
	}

	// *** Unequip items
	if ( token.Icmp( "unequip_left_ring" ) == 0 ) {
		inventory.arx_equiped_items[ ARX_EQUIPED_RING_LEFT ] = "";
		return true;
	}

	if ( token.Icmp( "unequip_right_ring" ) == 0 ) {
		inventory.arx_equiped_items[ ARX_EQUIPED_RING_RIGHT ] = "";
		return true;
	}

	// *************************************************
	// Start - Solarsplace - Arx End Of Sun - Blacksmith

	int blackSmithRepairCost = 0;
	int selectedListKey = -1;
	int repairListValue = -1;
	int inventoryItemId = 0;

	if ( token.Icmp( "arx_select_repair_item" ) == 0 ) {

		// Get the selected display item
		selectedListKey = conversationSystem->State().GetInt( "listRepairItems_sel_0", "0" );
		if ( selectedListKey == -1 ) {
			return true; // Nothing selected in list def
		}

		// Using the visible list def selected item, programatically select the same row in the hidden list def's
		conversationSystem->SetStateInt( "listRepairItemsHidden_sel_0", selectedListKey ); // The hidden list is not 0, 1, 2, 3 etc it could be 1, 9, 14 which is the inventory id.
		conversationSystem->SetStateInt( "listRepairItemsHiddenCost_item_", selectedListKey );

		blackSmithRepairCost = atoi( conversationSystem->State().GetString( va( "listRepairItemsHiddenCost_item_%i", selectedListKey ), "-1" ) );

		conversationSystem->SetStateInt( "blackSmithCost", blackSmithRepairCost );

		return true;
	}

	if ( token.Icmp( "arx_perform_repair_item" ) == 0 ) {

		selectedListKey = conversationSystem->State().GetInt( "listRepairItems_sel_0", "0" );

		if ( selectedListKey == -1 ) {
			return true; // Nothing selected in list def
		}

		// Get the selected display item
		selectedListKey = conversationSystem->State().GetInt( "listRepairItems_sel_0", "0" );
		if ( selectedListKey == -1 ) {
			selectedListKey = 0;
		}

		// Using the visible list def selected item, programatically select the same row in the hidden list def's
		conversationSystem->SetStateInt( "listRepairItemsHidden_sel_0", selectedListKey ); // The hidden list is not 0, 1, 2, 3 etc it could be 1, 9, 14 which is the inventory id.
		conversationSystem->SetStateInt( "listRepairItemsHiddenCost_item_", selectedListKey );

		blackSmithRepairCost = atoi( conversationSystem->State().GetString( va( "listRepairItemsHiddenCost_item_%i", selectedListKey ), "-1" ) );	
		inventoryItemId = atoi( conversationSystem->State().GetString( va( "listRepairItemsHidden_item_%i", selectedListKey ), "-1" ) );

		if ( inventory.money - blackSmithRepairCost >= 0 ) {

			inventory.money -= blackSmithRepairCost;

			int tempHealth = 0;
			int tempHealthMax = 0;
			float tempHealthPercent = 0;
			int blackSmithSkill = 0;
			int repairedHealth = 0;
							
			tempHealth = inventory.items[inventoryItemId]->GetInt( "inv_health", "0" );
			tempHealthMax = inventory.items[inventoryItemId]->GetInt( "inv_health_max", "100" );
			if ( tempHealthMax == 0 ) { tempHealthMax = 100; } // Safety just in case it somehow gets set 0 to avoid divide by zero errors...

			tempHealthPercent = ( (float)tempHealth / (float)tempHealthMax ) * 100;

			if ( focusCharacter ) {
				blackSmithSkill = focusCharacter->spawnArgs.GetInt( "blacksmith_skill", idStr(ARX_DEFAULT_BLACKSMITH_SKILL) );
			} else {
				// This should never happen, but just in case.
				blackSmithSkill = ARX_DEFAULT_BLACKSMITH_SKILL;
			}

			repairedHealth = ( (float)tempHealthMax /  100 ) * blackSmithSkill;

			inventory.items[inventoryItemId]->Set( "inv_health", idStr( repairedHealth ) );

			StartSound( "snd_arx_blacksmith_vo_restored", SND_CHANNEL_VOICE, 0, false, NULL );
			StartSound( "snd_arx_blacksmith_repair_success", SND_CHANNEL_WEAPON, 0, false, NULL );

			// Now after repair clear the selections
			conversationSystem->SetStateInt( "listRepairItems_sel_0", -1 );
			conversationSystem->SetStateInt( "listRepairItemsHidden_sel_0", -1 );
			conversationSystem->SetStateInt( "listRepairItemsHiddenCost_sel_0", -1 );
			conversationSystem->SetStateInt( "blackSmithCost", 0 );

		} else {
			ShowHudMessage( "#str_blacksmith_00004" ); // "Not enough money to pay for repair"
			StartSound( "snd_arx_blacksmith_repair_fail", SND_CHANNEL_WEAPON, 0, false, NULL );
		}

		return true;
	}

	
	// End - Solarsplace - Arx End Of Sun - Blacksmith
	// ***********************************************


	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/
	/*****************************************************************************************/

	src->UnreadToken( &token );
	return false;
}

/*
==============
idPlayer::Collide
==============
*/
bool idPlayer::Collide( const trace_t &collision, const idVec3 &velocity ) {
	idEntity *other;

	if ( gameLocal.isClient ) {
		return false;
	}

	other = gameLocal.entities[ collision.c.entityNum ];
	if ( other ) {
		other->Signal( SIG_TOUCH );
		if ( !spectating ) {
			if ( other->RespondsTo( EV_Touch ) ) {
				other->ProcessEvent( &EV_Touch, this, &collision );
			}
		} else {
			if ( other->RespondsTo( EV_SpectatorTouch ) ) {
				other->ProcessEvent( &EV_SpectatorTouch, this, &collision );
			}
		}
	}
	return false;
}


/*
================
idPlayer::UpdateLocation

Searches nearby locations 
================
*/
void idPlayer::UpdateLocation( void ) {
	if ( hud ) {
		idLocationEntity *locationEntity = gameLocal.LocationForPoint( GetEyePosition() );
		if ( locationEntity ) {
			hud->SetStateString( "location", locationEntity->GetLocation() );
		} else {
			hud->SetStateString( "location", common->GetLanguageDict()->GetString( "#str_02911" ) );
		}
	}
}

/*
================
idPlayer::ClearFocus

Clears the focus cursor
================
*/
void idPlayer::ClearFocus( void ) {
	focusCharacter	= NULL;
	focusGUIent		= NULL;
	focusUI			= NULL;
	focusVehicle	= NULL;
	talkCursor		= 0;
}

/*
================
idPlayer::UpdateFocus

Searches nearby entities for interactive guis, possibly making one of them
the focus and sending it a mouse move event
================
*/
void idPlayer::UpdateFocus( void ) {
	idClipModel *clipModelList[ MAX_GENTITIES ];
	idClipModel *clip;
	int			listedClipModels;
	idEntity	*oldFocus;
	idEntity	*ent;
	idUserInterface *oldUI;
	idAI		*oldChar;
	int			oldTalkCursor;
	idAFEntity_Vehicle *oldVehicle;
	int			i, j;
	idVec3		start, end;
	bool		allowFocus;
	const char *command;
	trace_t		trace;
	guiPoint_t	pt;
	const idKeyValue *kv;
	sysEvent_t	ev;
	idUserInterface *ui;

	if ( gameLocal.inCinematic ) {
		return;
	}

	// only update the focus character when attack button isn't pressed so players
	// can still chainsaw NPC's
	//if ( gameLocal.isMultiplayer || ( !focusCharacter && ( usercmd.buttons & BUTTON_ATTACK ) ) ) {
	if ( gameLocal.isMultiplayer ) {
		allowFocus = false;
	} else {
		allowFocus = true;
	}

	oldFocus		= focusGUIent;
	oldUI			= focusUI;
	oldChar			= focusCharacter;
	oldTalkCursor	= talkCursor;
	oldVehicle		= focusVehicle;

	if ( focusTime <= gameLocal.time ) {
		ClearFocus();
	}

	// don't let spectators interact with GUIs
	if ( spectating ) {
		return;
	}

	start = GetEyePosition();
	end = start + viewAngles.ToForward() * 80.0f;

	// player identification -> names to the hud
	if ( gameLocal.isMultiplayer && entityNumber == gameLocal.localClientNum ) {
		idVec3 end = start + viewAngles.ToForward() * 768.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_BOUNDINGBOX, this );
		int iclient = -1;
		if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum < MAX_CLIENTS ) ) {
			iclient = trace.c.entityNum;
		}
		if ( MPAim != iclient ) {
			lastMPAim = MPAim;
			MPAim = iclient;
			lastMPAimTime = gameLocal.realClientTime;
		}
	}

	idBounds bounds( start );
	bounds.AddPoint( end );

	listedClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	// no pretense at sorting here, just assume that there will only be one active
	// gui within range along the trace
	for ( i = 0; i < listedClipModels; i++ ) {
		clip = clipModelList[ i ];
		ent = clip->GetEntity();

		if ( ent->IsHidden() ) {
			continue;
		}

		if ( allowFocus ) {
			if ( ent->IsType( idAFAttachment::Type ) ) {
				idEntity *body = static_cast<idAFAttachment *>( ent )->GetBody();
				if ( body && body->IsType( idAI::Type ) && ( static_cast<idAI *>( body )->GetTalkState() >= TALK_OK ) ) {
					gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
					if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum == ent->entityNumber ) ) {
						ClearFocus();
						focusCharacter = static_cast<idAI *>( body );
						talkCursor = 1;
						focusTime = gameLocal.time + FOCUS_TIME;
						break;
					}
				}
				continue;
			}

			if ( ent->IsType( idAI::Type ) ) {
				if ( static_cast<idAI *>( ent )->GetTalkState() >= TALK_OK ) {
					gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
					if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum == ent->entityNumber ) ) {
						ClearFocus();
						focusCharacter = static_cast<idAI *>( ent );
						talkCursor = 1;
						focusTime = gameLocal.time + FOCUS_TIME;
						break;
					}
				}
				continue;
			}

			if ( ent->IsType( idAFEntity_Vehicle::Type ) ) {
				gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
				if ( ( trace.fraction < 1.0f ) && ( trace.c.entityNum == ent->entityNumber ) ) {
					ClearFocus();
					focusVehicle = static_cast<idAFEntity_Vehicle *>( ent );
					focusTime = gameLocal.time + FOCUS_TIME;
					break;
				}
				continue;
			}
		}

		if ( !ent->GetRenderEntity() || !ent->GetRenderEntity()->gui[ 0 ] || !ent->GetRenderEntity()->gui[ 0 ]->IsInteractive() ) {
			continue;
		}

		if ( ent->spawnArgs.GetBool( "inv_item" ) ) {
			// don't allow guis on pickup items focus
			continue;
		}

		pt = gameRenderWorld->GuiTrace( ent->GetModelDefHandle(), start, end );
		if ( pt.x != -1 ) {
			// we have a hit
			renderEntity_t *focusGUIrenderEntity = ent->GetRenderEntity();
			if ( !focusGUIrenderEntity ) {
				continue;
			}

			if ( pt.guiId == 1 ) {
				ui = focusGUIrenderEntity->gui[ 0 ];
			} else if ( pt.guiId == 2 ) {
				ui = focusGUIrenderEntity->gui[ 1 ];
			} else {
				ui = focusGUIrenderEntity->gui[ 2 ];
			}
			
			if ( ui == NULL ) {
				continue;
			}

			ClearFocus();
			focusGUIent = ent;
			focusUI = ui;

			if ( oldFocus != ent ) {
				// new activation
				// going to see if we have anything in inventory a gui might be interested in
				// need to enumerate inventory items

				focusUI->SetStateInt( "inv_count", inventory.items.Num() );
				for ( j = 0; j < inventory.items.Num(); j++ ) {
					idDict *item = inventory.items[ j ];
					const char *iname = item->GetString( "inv_name" );
					const char *iicon = item->GetString( "inv_icon" );
					const char *itext = item->GetString( "inv_text" );

					focusUI->SetStateString( va( "inv_name_%i", j), iname );
					focusUI->SetStateString( va( "inv_icon_%i", j), iicon );
					focusUI->SetStateString( va( "inv_text_%i", j), itext );
					kv = item->MatchPrefix("inv_id", NULL);
					if ( kv ) {
						focusUI->SetStateString( va( "inv_id_%i", j ), kv->GetValue() );
					}
					focusUI->SetStateInt( iname, 1 );
				}


				for( j = 0; j < inventory.pdaSecurity.Num(); j++ ) {
					const char *p = inventory.pdaSecurity[ j ];
					if ( p && *p ) {
						focusUI->SetStateInt( p, 1 );
					}
				}

				int staminapercentage = ( int )( 100.0f * stamina / pm_stamina.GetFloat() );
				focusUI->SetStateString( "player_health", va("%i", health ) );
				focusUI->SetStateString( "player_stamina", va( "%i%%", staminapercentage ) );
				focusUI->SetStateString( "player_armor", va( "%i%%", inventory.armor ) );

				kv = focusGUIent->spawnArgs.MatchPrefix( "gui_parm", NULL );
				while ( kv ) {
					focusUI->SetStateString( kv->GetKey(), kv->GetValue() );
					kv = focusGUIent->spawnArgs.MatchPrefix( "gui_parm", kv );
				}
			}

			// clamp the mouse to the corner
			ev = sys->GenerateMouseMoveEvent( -2000, -2000 );
			command = focusUI->HandleEvent( &ev, gameLocal.time );
 			HandleGuiCommands( focusGUIent, command );

			// move to an absolute position
			ev = sys->GenerateMouseMoveEvent( pt.x * SCREEN_WIDTH, pt.y * SCREEN_HEIGHT );
			command = focusUI->HandleEvent( &ev, gameLocal.time );
			HandleGuiCommands( focusGUIent, command );
			focusTime = gameLocal.time + FOCUS_GUI_TIME;
			break;
		}
	}

	if ( focusGUIent && focusUI ) {
		if ( !oldFocus || oldFocus != focusGUIent ) {
			command = focusUI->Activate( true, gameLocal.time );
			HandleGuiCommands( focusGUIent, command );
			StartSound( "snd_guienter", SND_CHANNEL_ANY, 0, false, NULL );
			// HideTip();
			// HideObjective();
		}
	} else if ( oldFocus && oldUI ) {
		command = oldUI->Activate( false, gameLocal.time );
		HandleGuiCommands( oldFocus, command );
		StartSound( "snd_guiexit", SND_CHANNEL_ANY, 0, false, NULL );
	}

	if ( cursor && ( oldTalkCursor != talkCursor ) ) {
		cursor->SetStateInt( "talkcursor", talkCursor );
	}

	if ( oldChar != focusCharacter && hud ) {
		if ( focusCharacter ) {

			//Ivan start

			if(focusCharacter->spawnArgs.GetBool( "showStatus", "0" )){  //ff1.1
				hud->SetStateString( "npc", "Status:" );
				hud->SetStateString( "npc_action", focusCharacter->spawnArgs.GetString( "shownState", "Inactive" ) );
			}else{
				hud->SetStateString( "npc", focusCharacter->spawnArgs.GetString( "npc_name", "404" ) );

				// Solarsplace - Arx End Of Sun
				// Don't show option to talk to AI that has an enemy or has no talk GUI.
				if ( !focusCharacter->GetEnemy() ) {

					idStr charactersGui = focusCharacter->spawnArgs.GetString( "characters_gui", "" );
					if ( charactersGui != "" ) {
						hud->SetStateString( "npc_action", common->GetLanguageDict()->GetString( "#str_02036" ) ); // Talk
					}
				}
			}

			//Ivan end

			hud->HandleNamedEvent( "showNPC" );

			// HideTip();
			// HideObjective();

		} else {

			hud->SetStateString( "npc", "" );
			hud->SetStateString( "npc_action", "" );
			hud->HandleNamedEvent( "hideNPC" );

			// Solarsplace - Arx EOS - 31st May 2012
			// Do to script code changes we now need to set script AI_TALK
			// to false on the NPC the player was previously speaking to
			oldChar->TalkTo( NULL );

			// SP - Arx EOS - NPC GUI
			if (conversationSystemOpen)
			{
				// If the conversation GUI is open and we no longer have a focus AI then shut the GUI.
				ToggleConversationSystem();
			}
		}
	}
}

/*
=================
idPlayer::CrashLand

Check for hard landings that generate sound events
=================
*/
void idPlayer::CrashLand( const idVec3 &oldOrigin, const idVec3 &oldVelocity ) {
	idVec3		origin, velocity;
	idVec3		gravityVector, gravityNormal;
	float		delta;
	float		hardDelta, fatalDelta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
	waterLevel_t waterLevel;
	bool		noDamage;

	AI_SOFTLANDING = false;
	AI_HARDLANDING = false;

	// if the player is not on the ground
	if ( !physicsObj.HasGroundContacts() ) {
		return;
	}

	gravityNormal = physicsObj.GetGravityNormal();

	// if the player wasn't going down
	if ( ( oldVelocity * -gravityNormal ) >= 0.0f ) {
		return;
	}

	waterLevel = physicsObj.GetWaterLevel();

	// never take falling damage if completely underwater
	if ( waterLevel == WATERLEVEL_HEAD ) {
		return;
	}

	// no falling damage if touching a nodamage surface
	noDamage = false;
	for ( int i = 0; i < physicsObj.GetNumContacts(); i++ ) {
		const contactInfo_t &contact = physicsObj.GetContact( i );
		if ( contact.material->GetSurfaceFlags() & SURF_NODAMAGE ) {
			noDamage = true;
			StartSound( "snd_land_hard", SND_CHANNEL_ANY, 0, false, NULL );
			break;
		}
	}

	origin = GetPhysics()->GetOrigin();
	gravityVector = physicsObj.GetGravity();

	// calculate the exact velocity on landing
	dist = ( origin - oldOrigin ) * -gravityNormal;
	vel = oldVelocity * -gravityNormal;
	acc = -gravityVector.Length();

	a = acc / 2.0f;
	b = vel;
	c = -dist;

	den = b * b - 4.0f * a * c;
	if ( den < 0 ) {
		return;
	}
	t = ( -b - idMath::Sqrt( den ) ) / ( 2.0f * a );

	delta = vel + t * acc;
	delta = delta * delta * 0.0001;

	// reduce falling damage if there is standing water
	if ( waterLevel == WATERLEVEL_WAIST ) {
		delta *= 0.25f;
	}
	if ( waterLevel == WATERLEVEL_FEET ) {
		delta *= 0.5f;
	}

	if ( delta < 1.0f ) {
		return;
	}

	// allow falling a bit further for multiplayer
	if ( gameLocal.isMultiplayer ) {
		fatalDelta	= 75.0f;
		hardDelta	= 50.0f;
	} else {
		fatalDelta	= 65.0f;
		hardDelta	= 45.0f;
	}

	if ( delta > fatalDelta ) {
		AI_HARDLANDING = true;
		landChange = -32;
		landTime = gameLocal.time;
		if ( !noDamage ) {
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_fatalfall", 1.0f, 0 );
		}
	} else if ( delta > hardDelta ) {
		AI_HARDLANDING = true;
		landChange	= -24;
		landTime	= gameLocal.time;
		if ( !noDamage ) {
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_hardfall", 1.0f, 0 );
		}
	} else if ( delta > 30 ) {
		AI_HARDLANDING = true;
		landChange	= -16;
		landTime	= gameLocal.time;
		if ( !noDamage ) {
			pain_debounce_time = gameLocal.time + pain_delay + 1;  // ignore pain since we'll play our landing anim
			Damage( NULL, NULL, idVec3( 0, 0, -1 ), "damage_softfall", 1.0f, 0 );
		}
	} else if ( delta > 7 ) {
		AI_SOFTLANDING = true;
		landChange	= -8;
		landTime	= gameLocal.time;
	} else if ( delta > 3 ) {
		// just walk on
	}
}

/*
===============
idPlayer::BobCycle
===============
*/
void idPlayer::BobCycle( const idVec3 &pushVelocity ) {
	float		bobmove;
	int			old, deltaTime;
	idVec3		vel, gravityDir, velocity;
	idMat3		viewaxis;
	float		bob;
	float		delta;
	float		speed;
	float		f;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	velocity = physicsObj.GetLinearVelocity() - pushVelocity;

	gravityDir = physicsObj.GetGravityNormal();
	vel = velocity - ( velocity * gravityDir ) * gravityDir;
	xyspeed = vel.LengthFast();

	// do not evaluate the bob for other clients
	// when doing a spectate follow, don't do any weapon bobbing
	if ( gameLocal.isClient && entityNumber != gameLocal.localClientNum ) {
		viewBobAngles.Zero();
		viewBob.Zero();
		return;
	}

	if ( !physicsObj.HasGroundContacts() || influenceActive == INFLUENCE_LEVEL2 || ( gameLocal.isMultiplayer && spectating ) ) {
		// airborne
		bobCycle = 0;
		bobFoot = 0;
		bobfracsin = 0;
	} else if ( ( !usercmd.forwardmove && !usercmd.rightmove ) || ( xyspeed <= MIN_BOB_SPEED ) ) {
		// start at beginning of cycle again
		bobCycle = 0;
		bobFoot = 0;
		bobfracsin = 0;
	} else {
		if ( physicsObj.IsCrouching() ) {
			bobmove = pm_crouchbob.GetFloat();
			// ducked characters never play footsteps
		} else {
			// vary the bobbing based on the speed of the player
			bobmove = pm_walkbob.GetFloat() * ( 1.0f - bobFrac ) + pm_runbob.GetFloat() * bobFrac;
		}

		// check for footstep / splash sounds
		old = bobCycle;
		bobCycle = (int)( old + bobmove * gameLocal.msec ) & 255;
		bobFoot = ( bobCycle & 128 ) >> 7;
		bobfracsin = idMath::Fabs( sin( ( bobCycle & 127 ) / 127.0 * idMath::PI ) );
	}

	// calculate angles for view bobbing
	viewBobAngles.Zero();

	viewaxis = viewAngles.ToMat3() * physicsObj.GetGravityAxis();

	// add angles based on velocity
	delta = velocity * viewaxis[0];
	viewBobAngles.pitch += delta * pm_runpitch.GetFloat();
	
	delta = velocity * viewaxis[1];
	viewBobAngles.roll -= delta * pm_runroll.GetFloat();

	// add angles based on bob
	// make sure the bob is visible even at low speeds
	speed = xyspeed > 200 ? xyspeed : 200;

	delta = bobfracsin * pm_bobpitch.GetFloat() * speed;
	if ( physicsObj.IsCrouching() ) {
		delta *= 3;		// crouching
	}
	viewBobAngles.pitch += delta;
	delta = bobfracsin * pm_bobroll.GetFloat() * speed;
	if ( physicsObj.IsCrouching() ) {
		delta *= 3;		// crouching accentuates roll
	}
	if ( bobFoot & 1 ) {
		delta = -delta;
	}
	viewBobAngles.roll += delta;

	// calculate position for view bobbing
	viewBob.Zero();

	if ( physicsObj.HasSteppedUp() ) {

		// check for stepping up before a previous step is completed
		deltaTime = gameLocal.time - stepUpTime;
		if ( deltaTime < STEPUP_TIME ) {
			stepUpDelta = stepUpDelta * ( STEPUP_TIME - deltaTime ) / STEPUP_TIME + physicsObj.GetStepUp();
		} else {
			stepUpDelta = physicsObj.GetStepUp();
		}
		if ( stepUpDelta > 2.0f * pm_stepsize.GetFloat() ) {
			stepUpDelta = 2.0f * pm_stepsize.GetFloat();
		}
		stepUpTime = gameLocal.time;
	}

	idVec3 gravity = physicsObj.GetGravityNormal();

	// if the player stepped up recently
	deltaTime = gameLocal.time - stepUpTime;
	if ( deltaTime < STEPUP_TIME ) {
		viewBob += gravity * ( stepUpDelta * ( STEPUP_TIME - deltaTime ) / STEPUP_TIME );
	}

	// add bob height after any movement smoothing
	bob = bobfracsin * xyspeed * pm_bobup.GetFloat();
	if ( bob > 6 ) {
		bob = 6;
	}
	viewBob[2] += bob;

	// add fall height
	delta = gameLocal.time - landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		f = delta / LAND_DEFLECT_TIME;
		viewBob -= gravity * ( landChange * f );
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - ( delta / LAND_RETURN_TIME );
		viewBob -= gravity * ( landChange * f );
	}
}

/*
================
idPlayer::UpdateDeltaViewAngles
================
*/
void idPlayer::UpdateDeltaViewAngles( const idAngles &angles ) {
	// set the delta angle
	idAngles delta;
	for( int i = 0; i < 3; i++ ) {
		delta[ i ] = angles[ i ] - SHORT2ANGLE( usercmd.angles[ i ] );
	}
	SetDeltaViewAngles( delta );
}

/*
================
idPlayer::SetViewAngles
================
*/
void idPlayer::SetViewAngles( const idAngles &angles ) {
	UpdateDeltaViewAngles( angles );
	viewAngles = angles;
}

/*
================
idPlayer::TraceUsables
================
*/
void idPlayer::TraceUsables()
{
	//******************************************************************************************
	//******************************************************************************************
	//*** Solarsplace 7th June 2010

	// 14th Nov 2013 - Cannot do this if dead...
	if ( health <= 0 ) { return; }

	trace_t trace;
	idEntity * target;
	float pickupDistance;

	if ( inventory.arx_timer_player_telekinesis > gameLocal.GetTime() ) {
		pickupDistance = ARX_MAX_ITEM_PICKUP_DISTANCE_TELE;
	} else {
		pickupDistance = ARX_MAX_ITEM_PICKUP_DISTANCE;
	}

	/*
	idPlayer * player = gameLocal.GetLocalPlayer();
	idVec3 startPosition = player->GetEyePosition();
	idVec3 endPosition = startPosition + player->viewAngles.ToForward() * ARX_MAX_ITEM_PICKUP_DISTANCE;
	gameLocal.clip.TracePoint( trace, startPosition, endPosition, MASK_ALL, player ); // Make sure we ignore the player.
	*/

	// New trace code based on weapon melee - Solars - 25th Mar 2012
	idVec3 start = firstPersonViewOrigin;
	idVec3 end = start + firstPersonViewAxis[0] * ( pickupDistance );

		// Solarsplace 10th May 2012 - Changed mask type to custom for Arx
	gameLocal.clip.TracePoint( trace, start, end, MASK_ARX_LEVEL_USE_TRIG, gameLocal.GetLocalPlayer() ); // MASK_SHOT_RENDERMODEL | MASK_ALL

	// Solarsplace 10th May 2012
	if( ( trace.fraction < 1.0f ) && ( trace.c.entityNum != ENTITYNUM_NONE ) )
	{
		target = gameLocal.entities[ trace.c.entityNum ];

		if ( target->IsType( idTrigger::Type ) )
		{
			// If idEntity is a trigger that does not have the bool spawn arg arx_usable_item set
			// then we repeat the trace again with different masks so we see through the trigger
			// this is so we can pickup up food from within a fire damage trigger for example.
			if ( !target->spawnArgs.GetBool( "arx_usable_item", "0" ) )
			{
				gameLocal.clip.TracePoint( trace, start, end, MASK_ARX_LEVEL_USE_NOTRIG, gameLocal.GetLocalPlayer() );
			}
		}
	}

	if( ( trace.fraction < 1.0f ) && ( trace.c.entityNum != ENTITYNUM_NONE ) )
	{
		target = gameLocal.entities[ trace.c.entityNum ];

		if ( strcmp( target->name.c_str(), lastUsableName ) == 0 )
		{ return; }
		else
		{
			lastUsableName = target->name.c_str();
			lastUsableTraceWasNothing = false;
		}

		if ( hud ) // Solarsplace 3rd Aug 2010 
		{
			hud->SetStateString( "playerLookingAt_invItem_inv_name", "" );
			hud->HandleNamedEvent( "playerLookingAt_nothing" );
		}

		/******************************************************************************************
		*******************************************************************************************/
		if ( target->spawnArgs.GetBool( "arx_searchable_corpse" ) && !target->IsHidden() )
		{
			//NOWREMOVED
			//gameLocal.Printf( "Looking at inv_arx_inventory_item - %s\n", target->spawnArgs.GetString( "inv_name" ) );

			bool searchOk = false;
			if ( target && target->IsType( idAFEntity_Gibbable::Type ) ) {
				if ( static_cast<idAFEntity_Gibbable *>( target )->searchable && !static_cast<idAFEntity_Gibbable *>( target )->IsGibbed() ) {
					if ( target->IsType( idAI::Type ) ) {
						if ( static_cast<idAI *>( target )->IsDead() ) {
							searchOk = true;
						}
					} else if ( static_cast<idAFEntity_Gibbable *>( target )->IsAtRest() ) {
						searchOk = true;
					}
				}
			}

			if ( hud && searchOk )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", gameLocal.GetSafeLanguageMessage( target->spawnArgs.GetString( "inv_name" ) ) );
				hud->HandleNamedEvent( "playerLookingAt_invItem" );
			}	
		}
		/******************************************************************************************
		*******************************************************************************************/
		if ( target->spawnArgs.GetBool( "arx_searchable_container" ) && !target->IsHidden() )
		{
			//NOWREMOVED
			//gameLocal.Printf( "Looking at inv_arx_inventory_item - %s\n", target->spawnArgs.GetString( "inv_name" ) );

			if ( hud )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", gameLocal.GetSafeLanguageMessage( target->spawnArgs.GetString( "inv_name" ) ) );
				hud->HandleNamedEvent( "playerLookingAt_invItem" );
			}	
		}
		/******************************************************************************************
		*******************************************************************************************/
		if ( target->spawnArgs.GetBool( "inv_arx_inventory_item" ) && !target->IsHidden() )
		{
			//NOWREMOVED
			//gameLocal.Printf( "Looking at inv_arx_inventory_item - %s\n", target->spawnArgs.GetString( "inv_name" ) );

			if ( hud )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", gameLocal.GetSafeLanguageMessage( target->spawnArgs.GetString( "inv_name" ) ) );
				hud->HandleNamedEvent( "playerLookingAt_invItem" );
			}	
		}
		/******************************************************************************************
		*******************************************************************************************/
		if ( target->spawnArgs.GetBool( "arx_usable_item" ) && !target->IsHidden() )
		{
			//NOWREMOVED
			//gameLocal.Printf( "Looking at arx_usable_item - %s\n", target->name.c_str() );

			if ( hud )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", gameLocal.GetSafeLanguageMessage( target->spawnArgs.GetString( "arx_usable_item_helptext" ) ) );
				hud->HandleNamedEvent( "playerLookingAt_usableItem" );
			}
		}
		/******************************************************************************************
		*******************************************************************************************/
		if ( ( target->spawnArgs.GetBool( "arx_readable_item" ) || target->spawnArgs.GetBool( "arx_journal_item" ) ) && !target->IsHidden() )
		{
			//NOWREMOVED
			//gameLocal.Printf( "Looking at arx_usable_item - %s\n", target->name.c_str() );

			if ( hud )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", "" ) ; //TODO names for doors, chests etc.
				hud->HandleNamedEvent( "playerLookingAt_readableItem" );
			}
		}
		/******************************************************************************************
		*******************************************************************************************/
		if ( target->spawnArgs.GetBool( "arx_level_change" ) && !target->IsHidden() )
		{
			//NOWREMOVED
			//gameLocal.Printf( "Looking at arx_usable_item - %s\n", target->name.c_str() );

			if ( hud )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", "" ) ; //TODO names for doors, chests etc.
				hud->HandleNamedEvent( "playerLookingAt_levelChangeItem" );
			}
		}
		/******************************************************************************************
		*******************************************************************************************/
	}
	else
	{
		// If the player has moved out of reading distance of a readable item
		// and the readable item GUI is open, then close the readable GUI.
		if ( readableSystemOpen )
		{ ToggleReadableSystem(); }

		// Apply the same logic to the shop
		if ( shoppingSystemOpen )
		{
			// If there is an associated chest (door) then shut it
			if ( lastShopEntity ) {
				lastShopEntity->ActivateTargets( gameLocal.GetLocalPlayer() ); // If the shop is a door / chest then shut it!
			}
			
			//ToggleShoppingSystem();
		}

		if ( lastUsableTraceWasNothing == false )
		{ 
			lastUsableTraceWasNothing = true;

			// Solarsplace - 13th June 2010 - Corrected small bug, where same object
			// could not be indicated more than once.
			lastUsableName = "";

			//NOWREMOVED
			//gameLocal.Printf( "lastUsableTraceWasNothing event fired.\n" );

			if ( hud )
			{
				hud->SetStateString( "playerLookingAt_invItem_inv_name", "" );
				hud->HandleNamedEvent( "playerLookingAt_nothing" );
			}
		}
	}
}

/*
================
idPlayer::ProcessMagic
================
*/
void idPlayer::ProcessMagic()
{
	// Solarsplace 22nd May 2010 - Magic related

	if (magicCompassSequence == "")
	{ return; }

	idVec3 forward, up, playerOrigin;

	const idDeclEntityDef *def = NULL;
	const idDeclEntityDef *runeDef = NULL;
	const idDeclEntityDef *magicSpellCombo = NULL;
	const idDeclEntityDef *customSpellsDef = NULL;
	const char *currentRune;
	const char *currentRuneSound;
	const char *projectileName;
	const char *customMagicName;
	const char *customMagicFX;
	const char *customMagicSound;
	const char *customMagicSpell;
	const char *customMagicScriptActionWorld;
	idStr scriptAction;
	float alertRadius;
	const char *tmpValue;
	int spellManaCost;

	def = gameLocal.FindEntityDef( "arx_magic_rune_guestures", false );

	/*
	entityDef arx_magic_rune_guestures {
	"EE"		"aam"		// Create
	"SENEEE"	"cetrius"	// Poison
	.....
	}*/
	
	if ( def )
	{

		// Do we have a match to the directions drawn?
		if ( def->dict.GetString( magicCompassSequence ) != "" )
		{
			// We DO have a direction sequence matched to a rune.
			currentRune = def->dict.GetString( magicCompassSequence );

			// We need to now check if we have this rune and exit if we don't
			if ( !gameLocal.persistentLevelInfo.GetBool( currentRune ) )
			{ return; }

			// Find out what sound to play for this rune and play it.
			runeDef = gameLocal.FindEntityDef( va( "arx_magic_rune_%s", currentRune ), false );
			if ( runeDef )
			{
				currentRuneSound = runeDef->dict.GetString( "snd_cast" );
				StartSoundShader( declManager->FindSound( currentRuneSound ), SND_CHANNEL_ANY, 0, false, NULL );
			}

			// Need to concatonate a drawn sequence of runes and match to a whole spell.
			idStr tmpConcat;
			sprintf( tmpConcat, "_%s", currentRune );
			magicRuneSequence += tmpConcat;

			// OK, here we see if we have a complete spell.
			magicSpellCombo = gameLocal.FindEntityDef( "magic" + magicRuneSequence, false );
			if ( magicSpellCombo )
			{
				// Some spells spawn things. Get the players position etc.
				playerOrigin = GetPhysics()->GetOrigin();
				viewAngles.ToVectors( &forward, NULL, &up );

				// ****************************************************************************************
				// Do we have enough mana to cast the spell? if not then leave

				int playerCurrentMana = GetPlayerManaAmount();
				spellManaCost = magicSpellCombo->dict.GetInt( "mana_cost", "0" );

				if ( playerCurrentMana < ( spellManaCost ) ) //+ ARX_MANA_BASE_COST ) )
				{
					StartSoundShader( declManager->FindSound( "arx_magic_drawing_fizzle" ), SND_CHANNEL_ANY, 0, false, NULL );
					return;
				}
				// ****************************************************************************************

				projectileName = magicSpellCombo->dict.GetString( "magic_projectile" );
				if ( projectileName != "" )
				{
					// Solarsplace - 18th March 2015 - Old code. Changing the system to use standard script based D3 weapons.
					/*
					if ( usercmd.buttons & BUTTON_6 )
					{
						// Store pre-cast spell
						magicSaveSpell( spellManaCost, projectileName, magicSpellCombo->GetName() );
					}
					else
					{
						FireMagicWeapon( projectileName, 4, spellManaCost ); // This will require >= 1 mana and will use 1 mana
					}
					*/

					int tmp_snake_weapon = magicSpellCombo->dict.GetInt( "inv_weapon_def", idStr( ARX_MAGIC_WEAPON ) );

					// If the current weapon == tmp_snake_weapon don't re-select it to prevent 'toggling'
					if ( currentWeapon != tmp_snake_weapon ) {
						inventory.arx_snake_weapon = magicSpellCombo->dict.GetInt( "inv_weapon_def", idStr( ARX_MAGIC_WEAPON ) );
						SelectWeapon( inventory.arx_snake_weapon, true );
					}

					// Clear the buffer, so we can do another spell this magic session.
					magicRuneSequence = "";

				} else {

					/****************************************************************************************
					*****************************************************************************************
					Solarsplace - 1st Aug 2010 - Now perform any custom spell actions. */

					// Use appropriate mana
					int ammo_mana = idWeapon::GetAmmoNumForName( "ammo_mana" );
					inventory.ammo[ ammo_mana ] = inventory.ammo[ ammo_mana ] - spellManaCost;

					// Get custom spell effects and actions
					customMagicSpell = magicSpellCombo->dict.GetString( "magic_spell" );
					customMagicFX = magicSpellCombo->dict.GetString( "fx_magic_cast" );
					customMagicSound = magicSpellCombo->dict.GetString( "snd_magic_cast" );
					customMagicScriptActionWorld = magicSpellCombo->dict.GetString( "magic_world_script_action" );

					// ***********************************************************
					// ***********************************************************
					// Display visual spell effects around player etc.
					if ( !strcmp( customMagicFX, "" ) == 0 ) {
						idEntityFx::StartFx( customMagicFX, &playerOrigin, NULL, this, true );
					}

					// ***********************************************************
					// ***********************************************************
					// Play sound spell effects around player etc.
					if ( !strcmp( customMagicSound, "" ) == 0 ) {
						StartSoundShader( declManager->FindSound( customMagicSound ), SND_CHANNEL_DEMONIC, 0, false, NULL );
					}

					// *************************
					// *************************
					// *** level 1

					//Activate portal

					//Douse
						// === Handled below in "Script calls"

					//Ignite
						// === Handled below in "Script calls"

					//Magic missile

					//Night vision

					// *************************
					// *************************
					// *** level 2

					//Armor

					//Detect trap

					//Harm
					if ( strcmp( customMagicSpell, "add_harm" ) == 0 ) {
						idVec3 org;
						org = physicsObj.GetOrigin();

						// Do damage of 50 with a 128 unit radius - TODO - Increase with skills
						gameLocal.RadiusDamage( org, this, this, this, this, "damage_arx_harm_base" );
					}

					//Heal

					//Lower armor

					// *************************
					// *************************
					// *** level 3

					//Feed

					//Fireball
						// === Handled above in projectiles

					//Ice projection

					//Reveal

					//Speed

					// *************************
					// *************************
					// *** level 4

					//Bless

					//Curse

					//Dispel field
						// === Handled below in "Script calls"

					//Protection from cold

					//Protection from fire

					//Telekinesis
					if ( strcmp( customMagicSpell, "add_telekinesis" ) == 0 ) {
						inventory.arx_timer_player_telekinesis = gameLocal.time + ARX_TELEKENESIS_TIME;
					}

					// *************************
					// *************************
					// *** level 5

					//Cure effects of poison
					if ( strcmp( customMagicSpell, "remove_poison" ) == 0 ) {
						inventory.UseAmmo( ARX_MANA_TYPE, spellManaCost );
						inventory.arx_timer_player_poison = gameLocal.time;
					}

					//Levitate
					if ( strcmp( customMagicSpell, "add_levitate" ) == 0 ) {
						inventory.arx_timer_player_levitate = gameLocal.time + ARX_LEVITATE_TIME;
						Event_LevitateStart();
					}

					//Poison projection

					//Repel undead

					//Trap

					// *************************
					// *************************
					// *** level 6

					//Create field
					if ( strcmp( customMagicSpell, "add_field" ) == 0 ) {

						// Spawn the timed field entity
						idDict argsField;
						idEntity *spawnedField;
						argsField.Set( "classname", "func_arx_forcefield_animated_spawned" );
						gameLocal.SpawnEntityDef( argsField, &spawnedField );

						// Move the field entity origin
						idVec3 fieldOrigin = playerOrigin;
						fieldOrigin.z += 64.0f; // Forcefield origin is in the middle of the model
						spawnedField->GetPhysics()->SetOrigin( fieldOrigin + ( forward * 128.0f ) );
					}

					//Disable trap

					//Paralyze

					//Raise dead

					//Slow down

					// *************************
					// *************************
					// *** level 7

					//Confuse

					//Fire field

					//Flying eye

					//Ice field

					//Lightning projection
						// === Handled above in projectiles

					// *************************
					// *************************
					// *** level 8

					//Chaos

					//Enchant object

					//Invisibility
					if ( strcmp( customMagicSpell, "add_invisibility" ) == 0 )
					{
						inventory.arx_timer_player_invisible = gameLocal.time + ARX_INVIS_TIME;
						GivePowerUp( 1, ARX_INVIS_TIME );
					}

					//Life drain

					//Mana drain

					// *************************
					// *************************
					// *** level 9

					//Incinerate

					//Mass paralyze

					//Negate magic

					//Summon

					// *************************
					// *************************
					// *** level 10

					//Control demon

					//Mass incinerate
					if ( strcmp( customMagicSpell, "add_mass_incinerate" ) == 0 ) {
						idVec3 org;
						org = physicsObj.GetOrigin();

						// Do damage of 60 with a 256 unit radius - TODO - Increase with skills
						gameLocal.RadiusDamage( org, this, this, this, this, "damage_arx_mass_incinerate_base" );
					}

					//Mass lightning projection

					//Slow time

					// ***********************************************************
					// ***********************************************************
					// Script calls
					if ( !strcmp( customMagicScriptActionWorld, "" ) == 0 ) {
						magicSpellCombo->dict.GetFloat( "spell_radius", "256", alertRadius );
						inventory.UseAmmo( ARX_MANA_TYPE, spellManaCost );
						RadiusSpell( customMagicScriptActionWorld, alertRadius );
					}
					// ***********************************************************
					// ***********************************************************

				} // projectileName != ""

			} // if ( magicSpellCombo )

		} // if ( def->dict.GetString( magicCompassSequence ) != "" )

	} // if ( def )

	// Clear the current direction sequence
	magicCompassSequence = "";
}

/*
================
idPlayer::UpdateViewAngles
================
*/
void idPlayer::UpdateViewAngles( void ) {

	/****************************************************************************************
	****************************************************************************************
	Solarsplace 7th June 2010 - Inventory related */

	TraceUsables();

	/****************************************************************************************
	****************************************************************************************/

	int i;
	idAngles delta;

	// Solarsplace - 2nd May 2010 - Magic related
	idVec2 magicDirectionVec;
	idVec3 start, end, trailEnd;
	int magicCompassDir;

	// Solarsplace 11th April 2010 - Inventory related - Added inventorySystem to condition
	// Solarsplace 6th May 2010 - Journal related - Added journalSystem to condition
	// Solarsplace 2nd Nov 2011 - Added NPC GUI
	if ( !noclip && ( gameLocal.inCinematic || privateCameraView || gameLocal.GetCamera() || influenceActive == INFLUENCE_LEVEL2 || objectiveSystemOpen || inventorySystemOpen || journalSystemOpen || readableSystemOpen || conversationSystemOpen || shoppingSystemOpen) ) {
		// no view changes at all, but we still want to update the deltas or else when
		// we get out of this mode, our view will snap to a kind of random angle
		UpdateDeltaViewAngles( viewAngles );
		return;
	}

	// orient the model towards the direction we're looking
	SetAngles( idAngles( 0, viewAngles.yaw, 0 ) );

	// save in the log for analyzing weapon angle offsets
	loggedViewAngles[ gameLocal.framenum & (NUM_LOGGED_VIEW_ANGLES-1) ] = viewAngles;

	/*******************************************************************************************
	 *******************************************************************************************
	 *******************************************************************************************
	 *******************************************************************************************/

	// Bool magic mode toggle
	if ( magicModeActive )
	{
		// Spawned entities
		if ( magicWand && magicWandTrail )
		{
			// Solarsplace - 27th April 2010 - Turn on magic trail to indicate spell sampling
			if ( usercmd.buttons & BUTTON_ATTACK )
			{ magicWandTrail->Show(); }
			else
			{ magicWandTrail->Hide(); }

			float	movementAmount = 0.1f;
			float	magicMaxTranslationVertical = 240.0f;
			float	magicMinTranslationVertical = -240.0f;
			float	magicMaxTranslationHorizontal = 320.0f;
			float	magicMinTranslationHorizontal = -320.0f;
			bool	magicChange = false;

			start = GetEyePosition();
			end = start + ( viewAngles.ToForward() * 40.0f );
			trailEnd = start + ( viewAngles.ToForward() * 40.1f );

			if ( usercmd.my != magicLastYPos )
			{
				magicMoveAmountVertical += ( ( usercmd.my - magicLastYPos ) * -1 );
				magicChange = true;
			}

			if ( usercmd.mx != magicLastXPos )
			{
				magicMoveAmountHorizontal += ( ( usercmd.mx - magicLastXPos ) * -1 );
				magicChange = true;
			}

			// Solarsplace 29th April 2010 - Magic related - Need to clamp translation to viewable area in front of player
			if ( magicMoveAmountVertical > magicMaxTranslationVertical )
			{ magicMoveAmountVertical = magicMaxTranslationVertical; }

			if ( magicMoveAmountVertical < magicMinTranslationVertical )
			{ magicMoveAmountVertical = magicMinTranslationVertical; }

			if ( magicMoveAmountHorizontal > magicMaxTranslationHorizontal )
			{ magicMoveAmountHorizontal = magicMaxTranslationHorizontal; }

			if ( magicMoveAmountHorizontal < magicMinTranslationHorizontal )
			{ magicMoveAmountHorizontal = magicMinTranslationHorizontal; }

			if ( magicChange == true )
			{
				// Translate along the up & right axis
				end += ( firstPersonViewAxis[ 2 ] * ( magicMoveAmountVertical * movementAmount ) ) + ( firstPersonViewAxis[ 1 ] * ( magicMoveAmountHorizontal * movementAmount ) );
				trailEnd += ( firstPersonViewAxis[ 2 ] * ( magicMoveAmountVertical * movementAmount ) ) + ( firstPersonViewAxis[ 1 ] * ( magicMoveAmountHorizontal * movementAmount ) );

				// Now position the magic artifact
				magicWand->SetOrigin( end );
				magicWandTrail->SetOrigin( trailEnd );

				// Update the comparison variables
				magicLastXPos = usercmd.mx;
				magicLastYPos = usercmd.my;
			} 
			float dampingFactor = 0.01f;
			float vecX = (magicMoveAmountHorizontal * dampingFactor )  * -1;
			float vecY = (magicMoveAmountVertical * dampingFactor );

			if ( usercmd.buttons & BUTTON_ATTACK )
			{
				if ( magicChange == true )
				{
					// Movement is not idle so, update the time.
					magicIdleTime = gameLocal.time;

					// Set the end positional vector to the new cursor position.
					// This is happening constantly as the use moves the mouse around.
					magicEndVec = idVec2( vecX, vecY );
		
					magicDirectionVec = magicEndVec - magicStartVec; // Get the direction vector from the start and end positional vectors

					float vecLen =  magicDirectionVec.Length(); // // Find the length if the directional vector

					// Not sure if needed in the long run
					if ( magicStartVec != magicEndVec )
					{

						if ( vecLen > 1.0f ) // Highly related to damping factor above. Arrived at by experimentation.
						{
							// Get the compass direction
							magicCompassDir = arxMiscFunctions.vectorToCompassQuadrant( magicDirectionVec );

							if ( magicLastCompassDir != magicCompassDir )
							{
								// Here is where we would process the spell
								
								// Keep it really simple obvious code while we get this working.

								idStr currentMagicDirection = "";
								idStr feedbackMagicDirection = "";
								if ( magicCompassDir == 0 )
									{
										currentMagicDirection = "NN";
										feedbackMagicDirection = "North";
									}
								else if ( magicCompassDir == 1 )
									{
										currentMagicDirection = "NE";
										feedbackMagicDirection = "North-East";
									}
								else if ( magicCompassDir == 2 )
									{
										currentMagicDirection = "EE";
										feedbackMagicDirection = "East";
									}
								else if ( magicCompassDir == 3 )
									{
										currentMagicDirection = "SE";
										feedbackMagicDirection = "South-East";
									}
								else if ( magicCompassDir == 4 )
									{
										currentMagicDirection = "SS";
										feedbackMagicDirection = "South";
									}
								else if ( magicCompassDir == 5 )
									{
										currentMagicDirection = "SW";
										feedbackMagicDirection = "South-West";
									}
								else if ( magicCompassDir == 6 )
									{
										currentMagicDirection = "WW";
										feedbackMagicDirection = "West";
									}
								else if ( magicCompassDir == 7 )
									{
										currentMagicDirection = "NW";
										feedbackMagicDirection = "North-West";
									}
	
								// Here we add up the directions drawn so far
								//sprintf( magicCompassSequence, "%s", currentMagicDirection.c_str() );

								magicCompassSequence += currentMagicDirection;

								//REMOVEME - HUD debugging
								idStr strHUDMessage;
								sprintf( strHUDMessage, "%s ", feedbackMagicDirection.c_str() );
								hudMagicHelp += strHUDMessage;
								//hudMagicHelp = strHUDMessage;
								idPlayer *pPlayer = static_cast<idPlayer *>( this );
								if ( pPlayer && pPlayer->hud )
								{
									//pPlayer->hud->SetStateString( "message", feedbackMagicDirection.c_str() );
									//pPlayer->hud->HandleNamedEvent( "Message" );
									ShowHudMessage( feedbackMagicDirection );
								}
							}

							// Update poitional vectors
							magicStartVec = idVec2( vecX, vecY );

							magicLastCompassDir = magicCompassDir;	

						} // if ( vecLen > x.xxf )
						
					} // if ( magicStartVec != magicEndVec )

				} // if ( magicChange = true )
				else
				{
					if ( ( gameLocal.time - magicIdleTime ) >= 100 ) // Milliseconds
					{
						// Update poitional vectors
						magicStartVec = idVec2( vecX, vecY );
					}
				}

			} // if ( usercmd.buttons & BUTTON_ATTACK )
			else
			{
				ProcessMagic();

				// Update positional vectors
				hudMagicHelp = "";
				magicStartVec = idVec2( vecX, vecY );
				magicEndVec = idVec2( vecX, vecY );
				magicLastCompassDir = -1;
			}

			// no view changes at all, but we still want to update the deltas or else when
			// we get out of this mode, our view will snap to a kind of random angle
			UpdateDeltaViewAngles( viewAngles );

			return;
		}
	}

	/*******************************************************************************************
	 *******************************************************************************************
	 *******************************************************************************************
	 *******************************************************************************************/

	// if dead
	if ( health <= 0 ) {
		if ( pm_thirdPersonDeath.GetBool() ) {
			viewAngles.roll = 0.0f;
			viewAngles.pitch = 30.0f;
		} else {
			viewAngles.roll = 40.0f;
			viewAngles.pitch = -15.0f;
		}
		return;
	}

	// circularly clamp the angles with deltas
	for ( i = 0; i < 3; i++ ) {
		cmdAngles[i] = SHORT2ANGLE( usercmd.angles[i] );
		if ( influenceActive == INFLUENCE_LEVEL3 ) {
			viewAngles[i] += idMath::ClampFloat( -1.0f, 1.0f, idMath::AngleDelta( idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i]) + deltaViewAngles[i] ) , viewAngles[i] ) );
		} else {
			viewAngles[i] = idMath::AngleNormalize180( SHORT2ANGLE( usercmd.angles[i]) + deltaViewAngles[i] );
		}
	}
	if ( !centerView.IsDone( gameLocal.time ) ) {
		viewAngles.pitch = centerView.GetCurrentValue(gameLocal.time);
	}

	// clamp the pitch
	if ( noclip ) {
		if ( viewAngles.pitch > 89.0f ) {
			// don't let the player look down more than 89 degrees while noclipping
			viewAngles.pitch = 89.0f;
		} else if ( viewAngles.pitch < -89.0f ) {
			// don't let the player look up more than 89 degrees while noclipping
			viewAngles.pitch = -89.0f;
		}
	} else {
		if ( viewAngles.pitch > pm_maxviewpitch.GetFloat() ) {
			// don't let the player look down enough to see the shadow of his (non-existant) feet
			viewAngles.pitch = pm_maxviewpitch.GetFloat();
		} else if ( viewAngles.pitch < pm_minviewpitch.GetFloat() ) {
			// don't let the player look up more than 89 degrees
			viewAngles.pitch = pm_minviewpitch.GetFloat();
		}
	}

	UpdateDeltaViewAngles( viewAngles );
}



/*
==============
idPlayer::AdjustHeartRate

Player heartrate works as follows

DEF_HEARTRATE is resting heartrate

Taking damage when health is above 75 adjusts heart rate by 1 beat per second
Taking damage when health is below 75 adjusts heart rate by 5 beats per second
Maximum heartrate from damage is MAX_HEARTRATE

Firing a weapon adds 1 beat per second up to a maximum of COMBAT_HEARTRATE

Being at less than 25% stamina adds 5 beats per second up to ZEROSTAMINA_HEARTRATE

All heartrates are target rates.. the heart rate will start falling as soon as there have been no adjustments for 5 seconds
Once it starts falling it always tries to get to DEF_HEARTRATE

The exception to the above rule is upon death at which point the rate is set to DYING_HEARTRATE and starts falling 
immediately to zero

Heart rate volumes go from zero ( -40 db for DEF_HEARTRATE to 5 db for MAX_HEARTRATE ) the volume is 
scaled linearly based on the actual rate

Exception to the above rule is once the player is dead, the dying heart rate starts at either the current volume if
it is audible or -10db and scales to 8db on the last few beats
==============
*/
void idPlayer::AdjustHeartRate( int target, float timeInSecs, float delay, bool force ) {

	if ( heartInfo.GetEndValue() == target ) {
		return;
	}

	if ( AI_DEAD && !force ) {
		return;
	}

    lastHeartAdjust = gameLocal.time;

	heartInfo.Init( gameLocal.time + delay * 1000, timeInSecs * 1000, heartRate, target );
}

/*
==============
idPlayer::GetBaseHeartRate
==============
*/
int idPlayer::GetBaseHeartRate( void ) {
	int base = idMath::FtoiFast( ( BASE_HEARTRATE + LOWHEALTH_HEARTRATE_ADJ ) - ( (float)health / 100.0f ) * LOWHEALTH_HEARTRATE_ADJ );
	int rate = idMath::FtoiFast( base + ( ZEROSTAMINA_HEARTRATE - base ) * ( 1.0f - stamina / pm_stamina.GetFloat() ) );
	int diff = ( lastDmgTime ) ? gameLocal.time - lastDmgTime : 99999;
	rate += ( diff < 5000 ) ? ( diff < 2500 ) ? ( diff < 1000 ) ? 15 : 10 : 5 : 0;
	return rate;
}

/*
==============
idPlayer::SetCurrentHeartRate
==============
*/
void idPlayer::SetCurrentHeartRate( void ) {

	int base = idMath::FtoiFast( ( BASE_HEARTRATE + LOWHEALTH_HEARTRATE_ADJ ) - ( (float) health / 100.0f ) * LOWHEALTH_HEARTRATE_ADJ );

	if ( PowerUpActive( ADRENALINE )) {
		heartRate = 135;
	} else {
		heartRate = idMath::FtoiFast( heartInfo.GetCurrentValue( gameLocal.time ) );
		int currentRate = GetBaseHeartRate();
		if ( health >= 0 && gameLocal.time > lastHeartAdjust + 2500 ) {
			AdjustHeartRate( currentRate, 2.5f, 0.0f, false );
		}
	}

	int bps = idMath::FtoiFast( 60.0f / heartRate * 1000.0f );
	if ( gameLocal.time - lastHeartBeat > bps ) {
		int dmgVol = DMG_VOLUME;
		int deathVol = DEATH_VOLUME;
		int zeroVol = ZERO_VOLUME;
		float pct = 0.0;
		if ( heartRate > BASE_HEARTRATE && health > 0 ) {
			pct = (float)(heartRate - base) / (MAX_HEARTRATE - base);
			pct *= ((float)dmgVol - (float)zeroVol);
		} else if ( health <= 0 ) {
			pct = (float)(heartRate - DYING_HEARTRATE) / (BASE_HEARTRATE - DYING_HEARTRATE);
			if ( pct > 1.0f ) {
				pct = 1.0f;
			} else if (pct < 0.0f) {
				pct = 0.0f;
			}
			pct *= ((float)deathVol - (float)zeroVol);
		} 

		pct += (float)zeroVol;

		if ( pct != zeroVol ) {
			StartSound( "snd_heartbeat", SND_CHANNEL_HEART, SSF_PRIVATE_SOUND, false, NULL );
			// modify just this channel to a custom volume
			soundShaderParms_t	parms;
			memset( &parms, 0, sizeof( parms ) );
			parms.volume = pct;
			refSound.referenceSound->ModifySound( SND_CHANNEL_HEART, &parms );
		}

		lastHeartBeat = gameLocal.time;
	}
}

/*
==============
idPlayer::UpdateAir
==============
*/
void idPlayer::UpdateAir( void ) {

	// Modified: Solarsplace 2nd April 2010 - Added underwater sounds

	if ( health <= 0 ) {
		return;
	}

	idPhysics_Player *phys = dynamic_cast<idPhysics_Player *>(this->GetPhysics());
	
	// see if the player is connected to the info_vacuum
	bool	newAirless = false;

	if ( gameLocal.vacuumAreaNum != -1 ) {
		int	num = GetNumPVSAreas();
		if ( num > 0 ) {
			int		areaNum;

			// if the player box spans multiple areas, get the area from the origin point instead,
			// otherwise a rotating player box may poke into an outside area
			if ( num == 1 ) {
				const int	*pvsAreas = GetPVSAreas();
				areaNum = pvsAreas[0];
			} else {
				areaNum = gameRenderWorld->PointInArea( this->GetPhysics()->GetOrigin() );
			}
			newAirless = gameRenderWorld->AreasAreConnected( gameLocal.vacuumAreaNum, areaNum, PS_BLOCK_AIR );
		}
	}

	// check if the player is in water
	if( phys != NULL && phys->GetWaterLevel() >= WATERLEVEL_HEAD )
		newAirless = true;

	if ( newAirless )
	{
		if ( !airless )
		{
			// Solarsplace
			if (phys->GetWaterLevel() == WATERLEVEL_HEAD)
			{

				//gameLocal.Printf( "!airless snd_footstep_underwater\n" );
				
				playerUnderWater = true;

				StartSound( "snd_footstep_underwater", SND_CHANNEL_BODY2, 0, false, NULL );
			}
			else
			{
				//gameLocal.Printf( "snd_decompress\n" );

				// Original D3 sounds
				StartSound( "snd_decompress", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
				StartSound( "snd_noAir", SND_CHANNEL_BODY2, 0, false, NULL );
			}
			
			if ( hud )
			{
				hud->HandleNamedEvent( "noAir" );
			}
		}
		
		airTics--;
		if ( airTics < 0 )
		{
			airTics = 0;
			// check for damage
			const idDict *damageDef = gameLocal.FindEntityDefDict( "damage_noair", false );
			int dmgTiming = 1000 * ((damageDef) ? damageDef->GetFloat( "delay", "3.0" ) : 3.0f );
			if ( gameLocal.time > lastAirDamage + dmgTiming ) {
				Damage( NULL, NULL, vec3_origin, "damage_noair", 1.0f, 0 );
				lastAirDamage = gameLocal.time;
			}
		}
		
	}
	else
	{
		if ( airless ) {

			// Solarsplace
			if (phys->GetWaterLevel() == WATERLEVEL_FEET || phys->GetWaterLevel() == WATERLEVEL_WAIST || phys->GetWaterLevel() == WATERLEVEL_NONE)
			{
				waterScreenFinishTime = gameLocal.time + 3000;
				playerUnderWater = false;

				StopSound( SND_CHANNEL_BODY2, false );
			}
			else
			{
				//gameLocal.Printf( "snd_recompress\n" );

				StartSound( "snd_recompress", SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
				StopSound( SND_CHANNEL_BODY2, false );
			}

			if ( hud ) {
				hud->HandleNamedEvent( "Air" );
			}
		}

		airTics+=2;	// regain twice as fast as lose
		if ( airTics > pm_airTics.GetInteger() )
		{
			airTics = pm_airTics.GetInteger();
		}
	}

	airless = newAirless;

	if ( hud ) {
		hud->SetStateInt( "player_air", 100 * airTics / pm_airTics.GetInteger() );
	}
}

/*
==============
idPlayer::AddGuiPDAData
==============
 */
int idPlayer::AddGuiPDAData( const declType_t dataType, const char *listName, const idDeclPDA *src, idUserInterface *gui ) {
	int c, i;
	idStr work;
	if ( dataType == DECL_EMAIL ) {
		c = src->GetNumEmails();
		for ( i = 0; i < c; i++ ) {
			const idDeclEmail *email = src->GetEmailByIndex( i );
			if ( email == NULL ) {
				work = va( "-\tEmail %d not found\t-", i );
			} else {
				work = email->GetFrom();
				work += "\t";
				work += email->GetSubject();
				work += "\t";
				work += email->GetDate();
			}
			gui->SetStateString( va( "%s_item_%i", listName, i ), work );
		}
		return c;
	} else if ( dataType == DECL_AUDIO ) {
		c = src->GetNumAudios();
		for ( i = 0; i < c; i++ ) {
			const idDeclAudio *audio = src->GetAudioByIndex( i );
			if ( audio == NULL ) {
				work = va( "Audio Log %d not found", i );
			} else {
				work = audio->GetAudioName();
			}
			gui->SetStateString( va( "%s_item_%i", listName, i ), work );
		}
		return c;
	} else if ( dataType == DECL_VIDEO ) {
		c = inventory.videos.Num();
		for ( i = 0; i < c; i++ ) {
			const idDeclVideo *video = GetVideo( i );
			if ( video == NULL ) {
				work = va( "Video CD %s not found", inventory.videos[i].c_str() );
			} else {
				work = video->GetVideoName();
			}
			gui->SetStateString( va( "%s_item_%i", listName, i ), work );
		}
		return c;
	}
	return 0;
}

/*
==============
idPlayer::GetPDA
==============
 */
const idDeclPDA *idPlayer::GetPDA( void ) const {
	if ( inventory.pdas.Num() ) {
		return static_cast< const idDeclPDA* >( declManager->FindType( DECL_PDA, inventory.pdas[ 0 ] ) );
	} else {
		return NULL;
	}
}


/*
==============
idPlayer::GetVideo
==============
*/
const idDeclVideo *idPlayer::GetVideo( int index ) {
	if ( index >= 0 && index < inventory.videos.Num() ) {
		return static_cast< const idDeclVideo* >( declManager->FindType( DECL_VIDEO, inventory.videos[index], false ) );
	}
	return NULL;
}


/*
==============
idPlayer::UpdatePDAInfo
==============
*/
void idPlayer::UpdatePDAInfo( bool updatePDASel ) {
	int j, sel;

	if ( objectiveSystem == NULL ) {
		return;
	}

	assert( hud );

	int currentPDA = objectiveSystem->State().GetInt( "listPDA_sel_0", "0" );
	if ( currentPDA == -1 ) {
		currentPDA = 0;
	}

	if ( updatePDASel ) {
		objectiveSystem->SetStateInt( "listPDAVideo_sel_0", 0 );
		objectiveSystem->SetStateInt( "listPDAEmail_sel_0", 0 );
		objectiveSystem->SetStateInt( "listPDAAudio_sel_0", 0 );
	}

	if ( currentPDA > 0 ) {
		currentPDA = inventory.pdas.Num() - currentPDA;
	}

	// Mark in the bit array that this pda has been read
	if ( currentPDA < 128 ) {
		inventory.pdasViewed[currentPDA >> 5] |= 1 << (currentPDA & 31);
	}

	pdaAudio = "";
	pdaVideo = "";
	pdaVideoWave = "";
	idStr name, data, preview, info, wave;
	for ( j = 0; j < MAX_PDAS; j++ ) {
		objectiveSystem->SetStateString( va( "listPDA_item_%i", j ), "" );
	}
	for ( j = 0; j < MAX_PDA_ITEMS; j++ ) {
		objectiveSystem->SetStateString( va( "listPDAVideo_item_%i", j ), "" );
		objectiveSystem->SetStateString( va( "listPDAAudio_item_%i", j ), "" );
		objectiveSystem->SetStateString( va( "listPDAEmail_item_%i", j ), "" );
		objectiveSystem->SetStateString( va( "listPDASecurity_item_%i", j ), "" );
	}
	for ( j = 0; j < inventory.pdas.Num(); j++ ) {

		const idDeclPDA *pda = static_cast< const idDeclPDA* >( declManager->FindType( DECL_PDA, inventory.pdas[j], false ) );

		if ( pda == NULL ) {
			continue;
		}

		int index = inventory.pdas.Num() - j;
		if ( j == 0 ) {
			// Special case for the first PDA
			index = 0;
		}

		// Arx - Show completed quests in journal?
		if ( !g_showCompletedQuests.GetBool() ) {
			if ( GetQuestState( pda->GetID() ) ) { // Arx quest object string id stored in "ID" field.
				continue;
			}
		}

		if ( j != currentPDA && j < 128 && inventory.pdasViewed[j >> 5] & (1 << (j & 31)) ) {

			// This pda has been read already.

			if ( GetQuestState( pda->GetID() ) ) { // Arx quest object string id stored in "ID" field.
				objectiveSystem->SetStateString( va( "listPDA_item_%i", index), va(S_COLOR_BLACK "%s", pda->GetPdaName()) );
			} else {

				if ( idStr::Icmp( pda->GetSecurity(), "arx_side_quest" ) == 0 ) { // Arx quest type stored in "Security" field.
					objectiveSystem->SetStateString( va( "listPDA_item_%i", index), va(S_COLOR_CYAN "%s", pda->GetPdaName()) );
				} else {
					objectiveSystem->SetStateString( va( "listPDA_item_%i", index), va(S_COLOR_BLUE "%s", pda->GetPdaName()) );
				}
			}

		} else {

			// This pda has not been read yet
			objectiveSystem->SetStateString( va( "listPDA_item_%i", index), va(S_COLOR_GREEN "%s", pda->GetPdaName()) );
		}

		const char *security = pda->GetSecurity();
		if ( j == currentPDA || (currentPDA == 0 && security && *security ) ) {
			if ( *security == NULL ) {
				security = common->GetLanguageDict()->GetString( "#str_00066" );
			}
			objectiveSystem->SetStateString( "PDASecurityClearance", security );
		}

		if ( j == currentPDA ) {

			objectiveSystem->SetStateString( "pda_icon", pda->GetIcon() );
			objectiveSystem->SetStateString( "pda_id", pda->GetID() );
			objectiveSystem->SetStateString( "pda_title", pda->GetTitle() );

			if ( j == 0 ) {
				// Selected, personal pda
				// Add videos
				if ( updatePDASel || !inventory.pdaOpened ) {
				objectiveSystem->HandleNamedEvent( "playerPDAActive" );
				objectiveSystem->SetStateString( "pda_personal", "1" );
					inventory.pdaOpened = true;
				}
				objectiveSystem->SetStateString( "pda_location", hud->State().GetString("location") );
				objectiveSystem->SetStateString( "pda_name", cvarSystem->GetCVarString( "ui_name") );
				AddGuiPDAData( DECL_VIDEO, "listPDAVideo", pda, objectiveSystem );
				sel = objectiveSystem->State().GetInt( "listPDAVideo_sel_0", "0" );
				const idDeclVideo *vid = NULL;
				if ( sel >= 0 && sel < inventory.videos.Num() ) {
					vid = static_cast< const idDeclVideo * >( declManager->FindType( DECL_VIDEO, inventory.videos[ sel ], false ) );
				}
				if ( vid ) {
					pdaVideo = vid->GetRoq();
					pdaVideoWave = vid->GetWave();
					objectiveSystem->SetStateString( "PDAVideoTitle", vid->GetVideoName() );
					objectiveSystem->SetStateString( "PDAVideoVid", vid->GetRoq() );
					objectiveSystem->SetStateString( "PDAVideoIcon", vid->GetPreview() );
					objectiveSystem->SetStateString( "PDAVideoInfo", vid->GetInfo() );
				} else {
					//FIXME: need to precache these in the player def
					objectiveSystem->SetStateString( "PDAVideoVid", "sound/vo/video/welcome.tga" );
					objectiveSystem->SetStateString( "PDAVideoIcon", "sound/vo/video/welcome.tga" );
					objectiveSystem->SetStateString( "PDAVideoTitle", "" );
					objectiveSystem->SetStateString( "PDAVideoInfo", "" );
				}
			} else {
				// Selected, non-personal pda
				// Add audio logs
				if ( updatePDASel ) {
				objectiveSystem->HandleNamedEvent( "playerPDANotActive" );
				objectiveSystem->SetStateString( "pda_personal", "0" );
					inventory.pdaOpened = true;
				}
				objectiveSystem->SetStateString( "pda_location", pda->GetPost() );
				objectiveSystem->SetStateString( "pda_name", pda->GetFullName() );
				int audioCount = AddGuiPDAData( DECL_AUDIO, "listPDAAudio", pda, objectiveSystem );
				objectiveSystem->SetStateInt( "audioLogCount", audioCount );
				sel = objectiveSystem->State().GetInt( "listPDAAudio_sel_0", "0" );
				const idDeclAudio *aud = NULL;
				if ( sel >= 0 ) {
					aud = pda->GetAudioByIndex( sel );
				}
				if ( aud ) {
					pdaAudio = aud->GetWave();
					objectiveSystem->SetStateString( "PDAAudioTitle", aud->GetAudioName() );
					objectiveSystem->SetStateString( "PDAAudioIcon", aud->GetPreview() );
					objectiveSystem->SetStateString( "PDAAudioInfo", aud->GetInfo() );
				} else {
					objectiveSystem->SetStateString( "PDAAudioIcon", "sound/vo/video/welcome.tga" );
					objectiveSystem->SetStateString( "PDAAutioTitle", "" );
					objectiveSystem->SetStateString( "PDAAudioInfo", "" );
				}
			}
			// add emails
			name = "";
			data = "";
			int numEmails = pda->GetNumEmails();
			if ( numEmails > 0 ) {
				AddGuiPDAData( DECL_EMAIL, "listPDAEmail", pda, objectiveSystem );
				sel = objectiveSystem->State().GetInt( "listPDAEmail_sel_0", "-1" );
				if ( sel >= 0 && sel < numEmails ) {
					const idDeclEmail *email = pda->GetEmailByIndex( sel );
					name = email->GetSubject();
					data = email->GetBody();
				}
			}
			objectiveSystem->SetStateString( "PDAEmailTitle", name );
			objectiveSystem->SetStateString( "PDAEmailText", data );
		}
	}
	if ( objectiveSystem->State().GetInt( "listPDA_sel_0", "-1" ) == -1 ) {
		objectiveSystem->SetStateInt( "listPDA_sel_0", 0 );
	}
	objectiveSystem->StateChanged( gameLocal.time );
}

/*
==============
idPlayer::ToggleInventorySystem
==============
*/
void idPlayer::ToggleInventorySystem(void)
{
	// Solarsplace 11th April 2010 - Inventory related

	if(!inventorySystemOpen)
	{
		inventorySystem->Activate( true, gameLocal.time );
		inventorySystemOpen = true;
	}
	else
	{
		inventorySystem->Activate( false, gameLocal.time );
		inventorySystemOpen = false;
	}
}

/*
==============
idPlayer::ToggleJournalSystem
==============
*/
void idPlayer::ToggleJournalSystem(void)
{
	// Solarsplace 6th May 2010 - Journal related

	if( !journalSystemOpen )
	{
		journalSystem->Activate( true, gameLocal.time );
		journalSystemOpen = true;
	}
	else
	{
		journalSystem->Activate( false, gameLocal.time );
		journalSystemOpen = false;
	}
}

/*
==============
idPlayer::ToggleReadableSystem
==============
*/
void idPlayer::ToggleReadableSystem(void)
{
	// Solarsplace 6th May 2010 - Journal related
	if( !readableSystemOpen )
	{
		readableSystem->Activate( true, gameLocal.time );
		readableSystemOpen = true;
	}
	else
	{
		readableSystem->Activate( false, gameLocal.time );
		readableSystemOpen = false;
	}
}

/*
==============
idPlayer::ToggleConversationSystem
==============
*/
void idPlayer::ToggleConversationSystem(void)
{
	// Solarsplace 2nd Nov 2011 - NPC GUI Related
	if( !conversationSystemOpen )
	{
		conversationSystem->Activate( true, gameLocal.time );

		// Select fists if talking to a blacksmith. Makes updating the current weapons health easier as there is none selected.
		if ( focusCharacter ) {
			if ( focusCharacter->spawnArgs.GetInt( "blacksmith_skill", "0" ) > 0 ) {
				SelectWeapon( ARX_FISTS_WEAPON, true );
			}
		}

		conversationSystem->SetStateInt( "listRepairItems_sel_0", -1 );
		conversationSystem->SetStateInt( "listRepairItemsHidden_sel_0", -1 );
		conversationSystem->SetStateInt( "listRepairItemsHiddenCost_sel_0", -1 );
		conversationSystem->SetStateInt( "blackSmithSkill", 0 );
		conversationSystem->SetStateInt( "blackSmithCost", 0 );

		conversationSystem->StateChanged( gameLocal.time, false );

		conversationSystemOpen = true;
	}
	else
	{
		conversationSystem->Activate( false, gameLocal.time );
		conversationSystemOpen = false;
	}
}

/*
==============
idPlayer::ToggleShoppingSystem
==============
*/
void idPlayer::ToggleShoppingSystem(void)
{

	// Solarsplace 6th Nov 2011 - Shop GUI Related
	if( !shoppingSystemOpen )
	{
		gameLocal.Printf("ToggleShoppingSystem = opening\n" ); //REMOVEME

		shoppingSystem->Activate( true, gameLocal.time );
		shoppingSystemOpen = true;
	}
	else
	{
		gameLocal.Printf("ToggleShoppingSystem = closing\n" ); //REMOVEME

		shoppingSystem->Activate( false, gameLocal.time );
		shoppingSystemOpen = false;
	}
}

/*
==============
idPlayer::ToggleMagicMode
==============
*/
void idPlayer::ToggleMagicMode(void)
{
	// Solarsplace 27th April 2010 - Magic related

	idDict args, argsTrail;
	idVec3 start, end, trailEnd;

	if( magicModeActive )
	{
		// Need to initalise...
		magicStartVec = vec2_origin;
		magicEndVec = vec2_origin;
		magicInitVectors = true;
		magicCompassSequence = "";
		magicLastCompassDir = -1;
		magicRuneSequence = "";
	
		//playerView.Flash( colorBlue, 500 );

		if ( currentWeapon != inventory.arx_snake_weapon ) // If the current weapon is not the magic weapon, select it. Must check the current weapon to prevent 'toggling'
		{ SelectWeapon( inventory.arx_snake_weapon, true ); }

		StartSoundShader( declManager->FindSound( "arx_magic_drawing_ambient" ), SND_CHANNEL_BODY2, 0, false, NULL );
		
		args.Set( "classname", "func_arx_magic_wand" );
		argsTrail.Set( "classname", "func_arx_magic_wand_trail" );

		if ( !magicWand && !magicWandTrail)
		{
			gameLocal.SpawnEntityDef( args, &magicWand );
			gameLocal.SpawnEntityDef( argsTrail, &magicWandTrail );
		}

		if ( magicWand && magicWandTrail )
		{
			magicMoveAmountVertical = 0.0f;
			magicMoveAmountHorizontal = 0.0f;
			magicLastYPos = usercmd.my;
			magicLastXPos = usercmd.mx;

			start = GetEyePosition();
			end = start + viewAngles.ToForward() * 40.0f;
			trailEnd = start + viewAngles.ToForward() * 40.1f;

			magicWand->SetOrigin( end );
			magicWandTrail->SetOrigin( trailEnd );

			// May have been hidden during previous toggle. But the trail is not visible unless BUTTON_ATTACK is pressed.
			magicWand->Show();
			magicWandTrail->Hide();
		}
	}
	else
	{
		//playerView.Flash( colorCyan, 500 );

		StopSound( SND_CHANNEL_BODY2, false );
		StartSoundShader( declManager->FindSound( "arx_magic_drawing_fizzle" ), SND_CHANNEL_ANY, 0, false, NULL );

		if ( magicWand && magicWandTrail )
		{
			magicWand->Hide();
			magicWandTrail->Hide();
		}
	}
}

/*
==============
idPlayer::TogglePDA
==============
*/
void idPlayer::TogglePDA( void ) {

	if ( objectiveSystem == NULL ) {
		return;
	}

	if ( inventory.pdas.Num() == 0 ) {

		// 1st Jan 2010 - Solarsplace - Prevent this message window from being displayed.
		//ShowTip( spawnArgs.GetString( "text_infoTitle" ), spawnArgs.GetString( "text_noPDA" ), true );
		return;
	}

	assert( hud );

	if ( !objectiveSystemOpen ) {
		int j, c = inventory.items.Num();
		objectiveSystem->SetStateInt( "inv_count", c );
		for ( j = 0; j < MAX_INVENTORY_ITEMS; j++ ) {
			objectiveSystem->SetStateString( va( "inv_name_%i", j ), "" );
			objectiveSystem->SetStateString( va( "inv_icon_%i", j ), "" );
			objectiveSystem->SetStateString( va( "inv_text_%i", j ), "" );
		}
		for ( j = 0; j < c; j++ ) {
			idDict *item = inventory.items[j];
			if ( !item->GetBool( "inv_pda" ) ) {
				const char *iname = item->GetString( "inv_name" );
				const char *iicon = item->GetString( "inv_icon" );
				const char *itext = item->GetString( "inv_text" );
				objectiveSystem->SetStateString( va( "inv_name_%i", j ), iname );
				objectiveSystem->SetStateString( va( "inv_icon_%i", j ), iicon );
				objectiveSystem->SetStateString( va( "inv_text_%i", j ), itext );
				const idKeyValue *kv = item->MatchPrefix( "inv_id", NULL );
				if ( kv ) {
					objectiveSystem->SetStateString( va( "inv_id_%i", j ), kv->GetValue() );
				}
			}
		}

		for ( j = 0; j < MAX_WEAPONS; j++ ) {
			const char *weapnum = va( "def_weapon%d", j );
			const char *hudWeap = va( "weapon%d", j );
			int weapstate = 0;
			if ( inventory.weapons & ( 1 << j ) ) {
				const char *weap = spawnArgs.GetString( weapnum );
				if ( weap && *weap ) {
					weapstate++;
				}
			}
			objectiveSystem->SetStateInt( hudWeap, weapstate );
		}

		objectiveSystem->SetStateInt( "listPDA_sel_0", inventory.selPDA );
		objectiveSystem->SetStateInt( "listPDAVideo_sel_0", inventory.selVideo );
		objectiveSystem->SetStateInt( "listPDAAudio_sel_0", inventory.selAudio );
		objectiveSystem->SetStateInt( "listPDAEmail_sel_0", inventory.selEMail );
		UpdatePDAInfo( false );
		UpdateObjectiveInfo();

		// *** Start - Solarsplace - Arx End Of Sun

		// Init any variables
		inventory.tmp_arx_attribute_points = inventory.arx_attribute_points;
		inventory.tmp_arx_skill_points = inventory.arx_skill_points;

		inventory.tmp_arx_attr_strength = inventory.arx_attr_strength;
		inventory.tmp_arx_attr_mental = inventory.arx_attr_mental;
		inventory.tmp_arx_attr_dexterity = inventory.arx_attr_dexterity;
		inventory.tmp_arx_attr_constitution = inventory.arx_attr_constitution;

		inventory.tmp_arx_skill_casting = inventory.arx_skill_casting;
		inventory.tmp_arx_skill_close_combat = inventory.arx_skill_close_combat;
		inventory.tmp_arx_skill_defense = inventory.arx_skill_defense;
		inventory.tmp_arx_skill_ethereal_link = inventory.arx_skill_ethereal_link;
		inventory.tmp_arx_skill_intuition = inventory.arx_skill_intuition;
		inventory.tmp_arx_skill_intelligence = inventory.arx_skill_intelligence;
		inventory.tmp_arx_skill_projectile = inventory.arx_skill_projectile;
		inventory.tmp_arx_skill_stealth = inventory.arx_skill_stealth;
		inventory.tmp_arx_skill_technical = inventory.arx_skill_technical;

		// *** End - Solarsplace - Arx End Of Sun

		objectiveSystem->Activate( true, gameLocal.time );
		hud->HandleNamedEvent( "pdaPickupHide" );
		hud->HandleNamedEvent( "videoPickupHide" );
	} else {
		inventory.selPDA = objectiveSystem->State().GetInt( "listPDA_sel_0" );
		inventory.selVideo = objectiveSystem->State().GetInt( "listPDAVideo_sel_0" );
		inventory.selAudio = objectiveSystem->State().GetInt( "listPDAAudio_sel_0" );
		inventory.selEMail = objectiveSystem->State().GetInt( "listPDAEmail_sel_0" );

		objectiveSystem->Activate( false, gameLocal.time );
	}
	objectiveSystemOpen ^= 1;
}

/*
==============
idPlayer::ToggleScoreboard
==============
*/
void idPlayer::ToggleScoreboard( void ) {
	scoreBoardOpen ^= 1;
}

/*
==============
idPlayer::Spectate
==============
*/
void idPlayer::Spectate( bool spectate ) {
	idBitMsg	msg;
	byte		msgBuf[MAX_EVENT_PARAM_SIZE];

	// track invisible player bug
	// all hiding and showing should be performed through Spectate calls
	// except for the private camera view, which is used for teleports
	assert( ( teleportEntity.GetEntity() != NULL ) || ( IsHidden() == spectating ) );

	if ( spectating == spectate ) {
		return;
	}

	spectating = spectate;

	if ( gameLocal.isServer ) {
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.WriteBits( spectating, 1 );
		ServerSendEvent( EVENT_SPECTATE, &msg, false, -1 );
	}

	if ( spectating ) {
		// join the spectators
		ClearPowerUps();
		spectator = this->entityNumber;
		Init();
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.DisableClip();
		Hide();
		Event_DisableWeapon();
		if ( hud ) {
			hud->HandleNamedEvent( "aim_clear" );
			MPAimFadeTime = 0;
		}
	} else {
		// put everything back together again
		currentWeapon = -1;	// to make sure the def will be loaded if necessary
		Show();
		Event_EnableWeapon();
	}
	SetClipModel();
}

/*
==============
idPlayer::SetClipModel
==============
*/
void idPlayer::SetClipModel( void ) {
	idBounds bounds;

	if ( spectating ) {
		bounds = idBounds( vec3_origin ).Expand( pm_spectatebbox.GetFloat() * 0.5f );
	} else {
		bounds[0].Set( -pm_bboxwidth.GetFloat() * 0.5f, -pm_bboxwidth.GetFloat() * 0.5f, 0 );
		bounds[1].Set( pm_bboxwidth.GetFloat() * 0.5f, pm_bboxwidth.GetFloat() * 0.5f, pm_normalheight.GetFloat() );
	}
	// the origin of the clip model needs to be set before calling SetClipModel
	// otherwise our physics object's current origin value gets reset to 0
	idClipModel *newClip;
	if ( pm_usecylinder.GetBool() ) {
		newClip = new idClipModel( idTraceModel( bounds, 8 ) );
		newClip->Translate( physicsObj.PlayerGetOrigin() );
		physicsObj.SetClipModel( newClip, 1.0f );
	} else {
		newClip = new idClipModel( idTraceModel( bounds ) );
		newClip->Translate( physicsObj.PlayerGetOrigin() );
		physicsObj.SetClipModel( newClip, 1.0f );
	}
}

/*
==============
idPlayer::UseVehicle
==============
*/
void idPlayer::UseVehicle( void ) {
	trace_t	trace;
	idVec3 start, end;
	idEntity *ent;

	if ( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) ) {
		Show();
		static_cast<idAFEntity_Vehicle*>(GetBindMaster())->Use( this );
	} else {
		start = GetEyePosition();
		end = start + viewAngles.ToForward() * 80.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
		if ( trace.fraction < 1.0f ) {
			ent = gameLocal.entities[ trace.c.entityNum ];
			if ( ent && ent->IsType( idAFEntity_Vehicle::Type ) ) {
				Hide();
				static_cast<idAFEntity_Vehicle*>(ent)->Use( this );
			}
		}
	}
}

/*
==============
idPlayer::PerformImpulse
==============
*/
void idPlayer::PerformImpulse( int impulse ) {

	if ( gameLocal.isClient ) {
		idBitMsg	msg;
		byte		msgBuf[MAX_EVENT_PARAM_SIZE];

		assert( entityNumber == gameLocal.localClientNum );
		msg.Init( msgBuf, sizeof( msgBuf ) );
		msg.BeginWriting();
		msg.WriteBits( impulse, 6 );
		ClientSendEvent( EVENT_IMPULSE, &msg );
	}

	// Solarsplace 27th April 2010 - Disable almost everything if in magic mode. - Magic related
	if ( !magicModeActive || magicDoingPreCastSpellProjectile)
	{
		// Solarsplace - 14th Aug 2010 - Can no longer select weapons via impulses!
		/*
		if ( impulse >= IMPULSE_0 && impulse <= IMPULSE_12 ) {
			SelectWeapon( impulse, false );
			return;
		}
		*/

		switch( impulse )
		{
			//REMOVEME
			case IMPULSE_13: {

				/***********************************************************************/
				/***********************************************************************/
				/***********************************************************************/
				gameLocal.Printf("IMPULSE_13\n");

				//RadiusSpell();

				/*
				if ( inventory.arx_timer_player_invisible > gameLocal.time ) // Do not alert AI or set enemy if player invisible
				{ break; };

				int			e;
				idEntity *	ent;
				idEntity *	entityList[ MAX_GENTITIES ];
				int			numListedEntities;
				idBounds	bounds;
				//idActor	*	actor;

				bounds = idBounds( GetPhysics()->GetOrigin() ).Expand( 1024 );

				const idActor *actor = static_cast<const idActor *>( this );

				// get all entities touching the bounds
				numListedEntities = gameLocal.clip.EntitiesTouchingBounds( bounds, -1, entityList, MAX_GENTITIES );

				for ( e = 0; e < numListedEntities; e++ ) {

					ent = entityList[ e ];
						
					if ( ent->IsType( idAI::Type ) ) {

						gameLocal.Printf("idAI is %s\n", ent->name.c_str());

						if ( static_cast<idActor *>( ent )->CanSee(this, true) )
						{
							gameLocal.Printf("The idAI %s can see the player\n", ent->name.c_str());
						}
						else
						{
							gameLocal.Printf("The idAI %s can NOT see the player\n", ent->name.c_str());
						}

						gameLocal.AlertAI( this );

						static_cast<idAI *>( ent )->SetEnemy( static_cast<idActor *>( this ) );// TouchedByFlashlight( actor );
						
					}
				}
				*/
				/***********************************************************************/
				/***********************************************************************/
				/***********************************************************************/


				//Reload();
				break;
			}

			// Solarsplace 14th Aug 2010 - Don't need this stuff
			/*
			case IMPULSE_14: {
				NextWeapon();
				break;
			}
			case IMPULSE_15: {
				PrevWeapon();
				break;
			}
			*/

			/* Solarsplace - Removed
			case IMPULSE_17: {
				if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
					gameLocal.mpGame.ToggleReady();
				}
				break;
			}
			*/
			case IMPULSE_18: {
				centerView.Init(gameLocal.time, 200, viewAngles.pitch, 0);
				break;
			}
			/* Solarsplace - Removed
			case IMPULSE_19: {
				// when we're not in single player, IMPULSE_19 is used for showScores
				// otherwise it opens the pda
				if ( !gameLocal.isMultiplayer ) {
					if ( objectiveSystemOpen ) {
						TogglePDA();
					} else if ( weapon_pda >= 0 ) {
						SelectWeapon( weapon_pda, true );
					}
				}
				break;
			}
			*/
			/* Solarsplace - Removed
			case IMPULSE_20: {
				if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
					gameLocal.mpGame.ToggleTeam();
				}
				break;
			}
			*/
			/* Solarsplace - Removed
			case IMPULSE_22: {
				if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
					gameLocal.mpGame.ToggleSpectate();
				}
				break;
			}
			*/

			// Solarsplace 14th Aug 2010 - Don't need this stuff
			/*
			case IMPULSE_28: {
				if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
					gameLocal.mpGame.CastVote( gameLocal.localClientNum, true );
				}
				break;
			}
			case IMPULSE_29: {
				if ( gameLocal.isClient || entityNumber == gameLocal.localClientNum ) {
					gameLocal.mpGame.CastVote( gameLocal.localClientNum, false );
				}
				break;
			}
			case IMPULSE_40: {
				UseVehicle();
				break;
			}
			*/
		}
	}


	//****************************************************************************
	//****************************************************************************
	//****************************************************************************
	//*** Start - Arx EOS - Solarsplace

	switch( impulse )
	{
		case IMPULSE_16: { // Arx - Quick health
			ConsumeInventoryItem( FindInventoryItemIndex ( "#str_item_00061" ) ); // "A life potion"
			break;
		}

		case IMPULSE_17: { // Arx - Quick Mana
			ConsumeInventoryItem( FindInventoryItemIndex ( "#str_item_00059" ) ); // "A mana potion"
			break;
		}

		case IMPULSE_19: { // Arx - Use command
			GetEntityByViewRay();
			break;
		}

		case IMPULSE_20: { // Arx - Inventory

			if (
				//!inventorySystemOpen &&	// 1
				!journalSystemOpen &&		// 2
				!readableSystemOpen &&		// 3
				!conversationSystemOpen &&	// 4
				!shoppingSystemOpen &&		// 5
				!objectiveSystemOpen		// 6
				)

				{ ToggleInventorySystem(); }

			break;
		}

		case IMPULSE_21: { // Arx - Journal

			if (
				!inventorySystemOpen &&		// 1
				!journalSystemOpen &&		// 2
				!readableSystemOpen &&		// 3
				!conversationSystemOpen &&	// 4
				!shoppingSystemOpen			// 5
				//!objectiveSystemOpen		// 6
				)

				{ TogglePDA(); }

			/*
			if ( !inventorySystemOpen && !readableSystemOpen && !conversationSystemOpen && !shoppingSystemOpen && !objectiveSystemOpen)
				{ ToggleJournalSystem(); }
			*/

			break;
		}
		
		case IMPULSE_22: { // Arx - Quick spell 1

			if (
				!inventorySystemOpen &&		// 1
				!journalSystemOpen &&		// 2
				!readableSystemOpen &&		// 3
				!conversationSystemOpen &&	// 4
				!shoppingSystemOpen			// 5
				//!objectiveSystemOpen		// 6
				)

				{ 
					if ( !magicModeActive )
					{ FireMagicWeapon( "", 0, 0 ); }
				}

			break;
		}

		case IMPULSE_23: { // Arx - Quick spell 2

			if (
				!inventorySystemOpen &&		// 1
				!journalSystemOpen &&		// 2
				!readableSystemOpen &&		// 3
				!conversationSystemOpen &&	// 4
				!shoppingSystemOpen			// 5
				//!objectiveSystemOpen		// 6
				)

				{ 
					if ( !magicModeActive )
					{ FireMagicWeapon( "", 1, 0 ); }
				}

			break;
		}

		case IMPULSE_24: { // Arx - Quick spell 3

			if (
				!inventorySystemOpen &&		// 1
				!journalSystemOpen &&		// 2
				!readableSystemOpen &&		// 3
				!conversationSystemOpen &&	// 4
				!shoppingSystemOpen			// 5
				//!objectiveSystemOpen		// 6
				)

				{ 
					if ( !magicModeActive )
					{ FireMagicWeapon( "", 2, 0 ); }
				}

			break;
		}

		case IMPULSE_25: { // Arx - Weapon Empty (Magic Weapon)

			if (
				!inventorySystemOpen &&		// 1
				!journalSystemOpen &&		// 2
				!readableSystemOpen &&		// 3
				!conversationSystemOpen &&	// 4
				!shoppingSystemOpen	&&		// 5
				!objectiveSystemOpen		// 6
				)

				{ 
					SelectWeapon( inventory.arx_snake_weapon, true );
				}

			break;
		}

		case IMPULSE_26: { // Arx - Test - Save pinfo
			SaveTransitionInfo();
			break;
		}

		case IMPULSE_27: { // Arx - Test - Load pinfo
			LoadTransitionInfo();
			break;
		}

	}

	//*** End - Arx EOS
	//****************************************************************************
	//****************************************************************************
	//****************************************************************************
}

/*
*****************************************************************************************************
*****************************************************************************************************
*****************************************************************************************************
*** BEGIN - Functions created by Solarsplace for the Arx End Of Sun project 
*/


void idPlayer::SetMapEntryPoint( idStr entityName )
{
	gameLocal.persistentLevelInfo.Set
		( ARX_PROP_MAP_ANY + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ARX_PROP_ENT_ANY + ARX_REC_SEP + ARX_PROP_MAPENTRYPOINT + ARX_REC_SEP, entityName );
}

idStr idPlayer::GetMapEntryPoint( void )
{
	idStr entryPoint;

	entryPoint = gameLocal.persistentLevelInfo.GetString
		( ARX_PROP_MAP_ANY + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ARX_PROP_ENT_ANY + ARX_REC_SEP + ARX_PROP_MAPENTRYPOINT + ARX_REC_SEP, "info_player_start" );

	return entryPoint;
}

void idPlayer::DeleteTransitionInfoSpecific( idStr recordType, idStr entityName )
{
	// Solarsplace 3rd Sep 2010
	// Simple routine to delete all the common save properties.

	const char *mapName;

	mapName = gameLocal.GetMapName();

	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_ORIGIN + ARX_REC_SEP );
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_AXIS + ARX_REC_SEP );
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_HIDDEN + ARX_REC_SEP );
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_INV_NAME + ARX_REC_SEP );
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_INV_HEALTH + ARX_REC_SEP );
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_INV_HEALTH_MAX + ARX_REC_SEP );
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_LOCKED + ARX_REC_SEP ); // Don't think this is used. Leave it for now.
	gameLocal.persistentLevelInfo.Delete( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_CLASSNAME + ARX_REC_SEP );
}

int idPlayer::GetTransitionKeyIndex( idStr recordType, idStr entityName )
{
	idStr myKey;
	const char *mapName;
	int indexKey;

	mapName = gameLocal.GetMapName();
	
	indexKey = gameLocal.persistentLevelInfo.FindKeyIndex( mapName + ARX_REC_SEP + recordType + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_ORIGIN + ARX_REC_SEP );

	return indexKey;
}

void idPlayer::SaveTransitionInfoSpecific( idEntity *ent, bool spawnedItem, bool hiddenItem )
{
	const char *mapName;
	idStr myKey;
	idStr itemType;
	idVec3 entityOrigin;
	idMat3 entityAxis;
	int keyIndex;

	mapName = gameLocal.GetMapName();
	entityOrigin = ent->GetPhysics()->GetOrigin();
	entityAxis = ent->GetPhysics()->GetAxis();

	if ( spawnedItem )
	{ itemType = ARX_REC_NEW; } 
	else
	{ itemType = ARX_REC_CHANGED; }

	// Only need to save minimal information here. This is for dropped inventory items only atm.
	gameLocal.persistentLevelInfo.SetVector( mapName + ARX_REC_SEP + itemType + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_ORIGIN + ARX_REC_SEP, entityOrigin );
	gameLocal.persistentLevelInfo.SetBool( mapName + ARX_REC_SEP + itemType + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_HIDDEN + ARX_REC_SEP, hiddenItem );
}

void idPlayer::SaveTransitionInfo( void )
{
	// This happens on level change.

	/*
	const int ARX_LVL_MAPNAME = 1;
	const int ARX_LVL_RECORDTYPE = 2;
	const int ARX_LVL_ENTNAME = 3;
	const int ARX_LVL_ENTPROPERTY = 4;

	const idStr ARX_REC_SEP = "<@@@ARX@@@>";
	const idStr ARX_REC_NEW = "ARX_ENTITY_NEW";
	const idStr ARX_REC_CHANGED = "ARX_ENTITY_CHANGED";

	const idStr ARX_PROP_ORIGIN = "ARX_ORIGIN";
	const idStr ARX_PROP_AXIS = "ARX_AXIS";
	const idStr ARX_PROP_HIDDEN = "ARX_HIDDEN";
	const idStr ARX_PROP_INV_NAME = "ARX_INV_NAME";
	*/

	idEntity *ent;
	const char *mapName;
	char *persistKey;
	idStr myKey;
	idVec3 entityOrigin;
	idMat3 entityAxis;
	bool entityHidden;
	idStr entityInventoryName;
	int entityHealth;
	int entityHealthMax;
	idStr entityClassName;

	mapName = gameLocal.GetMapName();

	for ( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() )
	{
		// Here we only detect changes in entities of this type
		if ( ent->name.Left( 13 ) ==  "moveable_item" ||
			ent->name.Left( 24 ) ==  "idMoveable_moveable_item" ) // Items which are dropped (spawned) from the inventory have this prefix added to them by the game.
		{
			// Get the current entity origin & axis and store it locally.
			entityOrigin = ent->GetPhysics()->GetOrigin();
			entityAxis = ent->GetPhysics()->GetAxis();
			entityHidden = ent->IsHidden();
			ent->spawnArgs.GetString( "inv_name", "", entityInventoryName );
			ent->spawnArgs.GetInt( "inv_health", "", entityHealth );
			ent->spawnArgs.GetInt( "inv_health_max", "", entityHealthMax );
			ent->spawnArgs.GetString( "classname", "", entityClassName );

			//******************************************************************************************************
			//******************************************************************************************************
			//******************************************************************************************************
			//***** Saving of non-native level items

			if ( GetTransitionKeyIndex( ARX_REC_NEW, ent->name ) != -1 )
			{
				// If the non-native entity is hidden, then delete its records. No point carrying them about, this entity will never be used again.
				// Although its name may be recycled by a new entity.
				if ( gameLocal.persistentLevelInfo.GetBool( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_HIDDEN ) )
				{
					DeleteTransitionInfoSpecific( ARX_REC_NEW, ent->name );
					
				}
				else
				{
					//gameLocal.Printf( "Arx: Saving ARX_REC_NEW (%s) (%s) (%s)\n", ent->name.c_str(), entityInventoryName.c_str(), entityClassName.c_str() );

					gameLocal.persistentLevelInfo.SetVector( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_ORIGIN + ARX_REC_SEP, entityOrigin );
					gameLocal.persistentLevelInfo.SetMatrix( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_AXIS + ARX_REC_SEP, entityAxis );
					gameLocal.persistentLevelInfo.SetBool( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_HIDDEN + ARX_REC_SEP, entityHidden );

					if ( idStr::Icmp( entityInventoryName, "" ) != 0 )
					{ gameLocal.persistentLevelInfo.Set( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_INV_NAME + ARX_REC_SEP, entityInventoryName ); }

					gameLocal.persistentLevelInfo.SetInt( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_INV_HEALTH + ARX_REC_SEP, entityHealth );
					gameLocal.persistentLevelInfo.SetInt( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_INV_HEALTH_MAX + ARX_REC_SEP, entityHealthMax );
					gameLocal.persistentLevelInfo.Set( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_CLASSNAME + ARX_REC_SEP, entityClassName );
				}

				continue;
			}
			
			//******************************************************************************************************
			//******************************************************************************************************
			//******************************************************************************************************
			//***** Saving of native level items

			// ATM this just saves new positions and angles of things that have moved around by knocking for example

			if ( ent->originalOrigin != entityOrigin )
			{
				//gameLocal.Printf( "Arx: Changed ENT: %s\n", ent->name.c_str() );

				gameLocal.persistentLevelInfo.SetVector( mapName + ARX_REC_SEP + ARX_REC_CHANGED + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_ORIGIN + ARX_REC_SEP, entityOrigin );
				gameLocal.persistentLevelInfo.SetMatrix( mapName + ARX_REC_SEP + ARX_REC_CHANGED + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_AXIS + ARX_REC_SEP, entityAxis );
				gameLocal.persistentLevelInfo.SetBool( mapName + ARX_REC_SEP + ARX_REC_CHANGED + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_HIDDEN + ARX_REC_SEP, entityHidden );

				// SP - 21st Feb 2013 - This is actually probably not needed here at all. We don't restore this property in 'SpawnTransitionEntity' ! - Comment out for now.
				//if ( idStr::Icmp( entityInventoryName, "" ) != 0 )
				//{ gameLocal.persistentLevelInfo.Set( mapName + ARX_REC_SEP + ARX_REC_CHANGED + ARX_REC_SEP + ent->name + ARX_REC_SEP + ARX_PROP_INV_NAME + ARX_REC_SEP, entityInventoryName ); }	
			}
		}
	}
}

idStr idPlayer::SplitStrings( idStr inputString, int requiredPart )
{
	int cutPosition = 0;
	idStr outputString;

	for (int i = 0; i < requiredPart; i++ )
	{
		cutPosition = inputString.Find( ARX_REC_SEP, 0, -1 );

		outputString = inputString.Left( cutPosition );

		inputString = inputString.Right( inputString.Length() - ( cutPosition + 11 ) );
	}

	//gameLocal.Printf( "SplitStrings( %s, %i )\n", outputString.c_str(), requiredPart );

	return outputString;
}


void idPlayer::SpawnTransitionEntity( idStr entityName )
{
	const char *mapName;
	idVec3 entityOrigin;
	idMat3 entityAxis;
	idStr entityInvName;
	int entityHealth;
	int entityHealthMax;
	idStr entityClassName;
	idDict args;
	idEntity *spawnedItem;

	mapName = gameLocal.GetMapName();
	entityOrigin = gameLocal.persistentLevelInfo.GetVector( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_ORIGIN + ARX_REC_SEP);
	entityAxis = gameLocal.persistentLevelInfo.GetMatrix( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_AXIS + ARX_REC_SEP );
	entityInvName = gameLocal.persistentLevelInfo.GetString( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_INV_NAME + ARX_REC_SEP );
	entityHealth = gameLocal.persistentLevelInfo.GetInt( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_INV_HEALTH + ARX_REC_SEP );
	entityHealthMax = gameLocal.persistentLevelInfo.GetInt( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_INV_HEALTH_MAX + ARX_REC_SEP );
	entityClassName = gameLocal.persistentLevelInfo.GetString( mapName + ARX_REC_SEP + ARX_REC_NEW + ARX_REC_SEP + entityName + ARX_REC_SEP + ARX_PROP_CLASSNAME + ARX_REC_SEP );

	args.Set( "classname", entityClassName );
	args.Set( "dropped", "1" );
	args.Set( "nodrop", "1" ); // we sometimes drop idMoveables here, so set 'nodrop' to 1 so that it doesn't get put on the floor

	gameLocal.SpawnEntityDef( args, &spawnedItem );

	if ( spawnedItem )
	{
		spawnedItem->GetPhysics()->SetOrigin( entityOrigin );
		spawnedItem->GetPhysics()->SetAxis( entityAxis );
		spawnedItem->spawnArgs.Set ( "inv_name", entityInvName );
		spawnedItem->spawnArgs.SetInt ( "inv_health", entityHealth );
		spawnedItem->spawnArgs.SetInt ( "inv_health_max", entityHealthMax );
	}
}

void idPlayer::LoadTransitionInfo( void )
{
	idEntity *ent;
	const char *mapName;
	const idKeyValue *arg;
	int maxPerstItems;

	idStr keyText;
	idStr keyMapName;
	idStr keyEntityNewChanged;
	idStr keyEntityName;

	idVec3 entityOrigin;
	idMat3 entityAxis;

	// Get the name of the current loaded map
	mapName = gameLocal.GetMapName(); // maps/mapname.map

	// Get count of all the saved persistent key vals
	maxPerstItems = gameLocal.persistentLevelInfo.GetNumKeyVals();

	for ( int i = 0; i < maxPerstItems; i++ )
	{
		arg = gameLocal.persistentLevelInfo.GetKeyVal( i );
		keyText = arg->GetKey();
		keyMapName = SplitStrings( keyText, ARX_LVL_MAPNAME );

		if ( idStr::Icmp( mapName, keyMapName ) == 0 )
		{
			//******************************************************************************************************
			//******************************************************************************************************
			//******************************************************************************************************
			//***** Loading  of non-native level items

			if ( idStr::Icmp( ARX_REC_NEW, SplitStrings( keyText, ARX_LVL_RECORDTYPE ) ) == 0 )
			{
				// Wait until the key is the origin key, then grab all the other properties and spawn the entity once only.
				if ( idStr::Icmp( ARX_PROP_ORIGIN, SplitStrings( keyText, ARX_LVL_ENTPROPERTY ) ) == 0 )
				{
					SpawnTransitionEntity( SplitStrings( keyText, ARX_LVL_ENTNAME ) );
				}
			}

			//******************************************************************************************************
			//******************************************************************************************************
			//******************************************************************************************************
			//***** Loading of native level items

			if ( idStr::Icmp( ARX_REC_CHANGED, SplitStrings( keyText, ARX_LVL_RECORDTYPE ) ) == 0 )
			{
				keyEntityName = SplitStrings( keyText, ARX_LVL_ENTNAME );

				ent = gameLocal.FindEntity( keyEntityName.c_str() );

				if ( !ent ) { continue; }

				/****************************************************************************
				****************************************************************************/
				//  Set origin

				if ( idStr::Icmp( SplitStrings( keyText, ARX_LVL_ENTPROPERTY ) , ARX_PROP_ORIGIN ) == 0 )
				{
					entityOrigin = gameLocal.persistentLevelInfo.GetVector( keyText );
					ent->GetPhysics()->SetOrigin( entityOrigin );
				}

				/****************************************************************************
				****************************************************************************/
				//  Set axis
				else if ( idStr::Icmp( SplitStrings( keyText, ARX_LVL_ENTPROPERTY ) , ARX_PROP_AXIS ) == 0 )
				{
					entityAxis = gameLocal.persistentLevelInfo.GetMatrix( keyText );
					ent->GetPhysics()->SetAxis( entityAxis );
				}

				/****************************************************************************
				****************************************************************************/
				//  Set hidden - atually remove the entity
				else if ( idStr::Icmp( SplitStrings( keyText, ARX_LVL_ENTPROPERTY ) , ARX_PROP_HIDDEN ) == 0 )
				{
					if ( gameLocal.persistentLevelInfo.GetBool( keyText ) )
					{
						ent->PostEventMS( &EV_Remove, 0 );
					}
				}

				/****************************************************************************
				****************************************************************************/

			}
		}
	}
}



void idPlayer::FireMagicWeapon( const char *projectileName, int quickSpellIndex, int manaCost )
{
	// **************************************************************
	// **************************************************************
	// Solarsplace - 18th March 2015 - This is old retired code.
	// Keeping here for reference.
	// I was never happy with this implementation, it always felt clumsy and a hack although it did mostly work.

	// Solarsplace - 13th May 2010 - Magic related
	// Solarsplace - 03rd July 2010 - Modified

	if ( !magicAttackInProgress )
	{
		if ( quickSpellIndex == 4 )
		{
			//**********************************************************************
			//**********************************************************************
			//***** Real time magic

			//REMOVEMEx
			//gameLocal.Printf( "Perfoming real time magic.\n" );

			// Flags & timings
			magicAttackInProgress = true;
			magicAttackTime = gameLocal.time;

			weapon.GetEntity()->magicChangeProjectileDef( projectileName );
			inventory.UseAmmo( ARX_MANA_TYPE, manaCost );
			weapon.GetEntity()->BeginAttack();

		}
		else
		{
			//**********************************************************************
			//**********************************************************************
			//***** Pre-cast magic

			//TODO
			// Check enough mana available to fire spell!

			//REMOVEMEx
			//gameLocal.Printf( "Perfoming pre-cast magic.\n" );

			if ( idStr::Icmp( gameLocal.persistentLevelInfo.GetString( va( "projectileName_%i", quickSpellIndex ), "" ), "" ) == 0 )
			{
				//REMOVEMEx
				//gameLocal.Printf( "No magic saved in slot %i aborting.\n", quickSpellIndex );
				return;
			}

			// Flags & timings
			magicPreDelay = gameLocal.time; // TODO - may remove this shit
			magicDoingPreCastSpellProjectile = true;

			if ( currentWeapon != ARX_MAGIC_WEAPON ) // If the current weapon is not the magic weapon, select it. Must check the current weapon to prevent 'toggling'
			{ SelectWeapon( ARX_MAGIC_WEAPON, false ); }

			//REMOVEMEx
			//gameLocal.Printf( "gameLocal.persistentLevelInfo.GetInt( %s )\n", va( "manaCost_%i", quickSpellIndex ) );
			//gameLocal.Printf( "gameLocal.persistentLevelInfo.GetString( %s )\n", va( "projectileName_%i", quickSpellIndex ) );
			
			// Set the global variable containg the pre-cast magic's projectile def
			magicLatestProjectileDefName = gameLocal.persistentLevelInfo.GetString( va( "projectileName_%i", quickSpellIndex ), "" );
			inventory.UseAmmo( ARX_MANA_TYPE, gameLocal.persistentLevelInfo.GetInt( va( "manaCost_%i", quickSpellIndex ), 0 ) );

			// Now clear the saved spell.

			//REMOVEMEx
			//gameLocal.Printf( "projectileName_%i = '%s'\n", quickSpellIndex, gameLocal.persistentLevelInfo.GetString( va( "projectileName_%i", quickSpellIndex ), "" )  );
			//gameLocal.Printf( "Clearing persistent key '%s'.\n", gameLocal.persistentLevelInfo.GetString( va( "spellNameKey_%i", quickSpellIndex ), "" ));

			gameLocal.persistentLevelInfo.SetInt( va( "manaCost_%i", quickSpellIndex ), 0 ); // Mana
			gameLocal.persistentLevelInfo.Set( va( "projectileName_%i", quickSpellIndex ), "" ); // Projectile
			
			// Example: Sets key = spellName_0 with value = magic_aam_folgora_taar_0
			const char *skey = va( "spellNameKey_%i", quickSpellIndex );

			gameLocal.persistentLevelInfo.Set( gameLocal.persistentLevelInfo.GetString( skey , "0" ), "" ); // HUD spell key

			// The attack is now started and coordinated from the think function.
		}	
	}
}

void::idPlayer::magicSaveSpell( int manaCost, const char *projectileName, const char *spellName )
{
	int i;
	bool savedSpell = false;

	for ( i = 0; i < 3; i++ )
	{
		if ( idStr::Icmp( gameLocal.persistentLevelInfo.GetString( va( "projectileName_%i", i ) ), "" ) == 0 )
		{
			// Save the spell in this free slot.
			savedSpell = true;
			gameLocal.persistentLevelInfo.SetInt( va( "manaCost_%i", i ), manaCost ); // Mana
			gameLocal.persistentLevelInfo.Set( va( "projectileName_%i", i ), projectileName ); // Projectile

			// Example: Sets key = spellName_0 with value = magic_aam_folgora_taar_0
			const char *skey = va( "spellNameKey_%i", i );
			const char *sVal =  va( "%s_%i", spellName, i );

			//REMOVEMEx
			//gameLocal.Printf( "Save spell at position %i - ( %s, %s)\n", i, skey, sVal );
			gameLocal.persistentLevelInfo.Set( skey, sVal ); // Hud spell key

			// Example: Sets key = magic_aam_folgora_taar_0 with value = 1
			gameLocal.persistentLevelInfo.Set( va( "%s_%i", spellName, i ), "1" ); // Spell combo

			break;
		}
	}

	if ( savedSpell )
	{ // TODO - Sucess sound
	}
	else
	{ StartSoundShader( declManager->FindSound( "arx_magic_drawing_fizzle" ), SND_CHANNEL_ANY, 0, false, NULL ); }
}

void idPlayer::DropInventoryItem( int invItemIndex )
{
	// Solarsplace - 1st May 2010 - Inventory related

	// Solarsplace 17th Mar 2010
	// Don't bother trying any of this if there is nothing in the inventory.
	if ( inventory.items.Num() <= 0 )
	{ return; }

	// Solarsplace 17th Mar 2010
	// Do not attempt to drop an inventory item whose index is greater than the number of inventory items held.
	if ( invItemIndex > inventory.items.Num() )
	{ return; }

	// Solarsplace 4th July 2013
	// Items which the player cannot drop from their inventory
	bool noInvDrop = inventory.items[invItemIndex]->GetBool( "inv_arx_noinvdrop", "0" );
	if ( noInvDrop ) {
		ShowHudMessage( "#str_general_00011" ); // "This item can not be dropped"
		StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
		return;
	}

	idVec3 forward, up, playerOrigin, throwVector, dropPoint;
	idDict args;
	idEntity *spawnedItem;
	
	float playerItemDropFromHeight = 50.0f;		// You need this so, one can see when standing straight the item being dropped.
	float playerItemDropForwardForce = 50.0f;	// This seems good. One can drop stuff on tables OK.
	float playerItemDropUpwardForce = 120.0f;	// Seems OK too.

	playerOrigin = GetPhysics()->GetOrigin(); // Get the players current world origin
	viewAngles.ToVectors( &forward, NULL, &up ); // Get players forward and up view direction

	// Solarsplace 24th Jan 2012 - Try to make sure we do not drop an item into into a brick wall or out of the level or something equally silly.
	trace_t trace;
	idPlayer * player = gameLocal.GetLocalPlayer();
	idVec3 startPosition = player->GetEyePosition();
	idVec3 endPosition = playerOrigin + (playerItemDropFromHeight * up) + ( forward * 24.0f ); // Make it a bit bigger than the 64.0f below to account for origin in middle of model.
	
	gameLocal.clip.TracePoint( trace, startPosition, endPosition, MASK_PLAYERSOLID, player ); // Make sure we ignore the player.

	if( ( trace.fraction < 1.0f ) && ( trace.c.entityNum != ENTITYNUM_NONE ) )
	{
		// Play a sound to indicate drop not possible here
		StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
		return;
	}

	// Solarsplace - 4th Oct 2010 - Level Transition related - changed from 'classname' to 'inv_classname' which is persisted through level changes.
	args.Set( "classname", inventory.items[invItemIndex]->GetString( "inv_classname" ) );
	args.Set( "inv_health", inventory.items[invItemIndex]->GetString( "inv_health" ) ); // SP - 6th Mar 2013 - Added for breakable weapons / items. Must be done before spawn!
	args.Set( "inv_health_max", inventory.items[invItemIndex]->GetString( "inv_health_max" ) ); // SP - 13th Mar 2013 - Added for breakable weapons / items. Must be done before spawn!
	args.Set( "dropped", "1" ); // Probably not used
	args.Set( "nodrop", "1" ); // We sometimes drop idMoveables here, so set 'nodrop' to 1 so that it doesn't get put on the floor

	gameLocal.SpawnEntityDef( args, &spawnedItem );

	if ( spawnedItem )
	{
		// Force, with which to launch object
		throwVector = (playerItemDropForwardForce * forward) + (playerItemDropUpwardForce * up); // Solarsplace - TBH, throwVector = experimental
		spawnedItem->GetPhysics()->SetOrigin( playerOrigin + (playerItemDropFromHeight * up) + ( forward * 16.0f ) );
		spawnedItem->GetPhysics()->SetAxis( spawnedItem->GetPhysics()->GetAxis() ); // Solarsplace - TBH, not 100% on this at the moment, but seems to be working OK.
		spawnedItem->GetPhysics()->SetLinearVelocity( throwVector );

		// *************************************************************************************

		// Get "inv_XXX" key vals from the item in the players inventory that is being dropped
		// and put them into the spawn args of the dropped item.
		// This needs to be done for things like keys that have unique names set in inv_name or
		// for items such as tools or weapons that have a health value

		// *** CRITICAL - Update SaveTransitionInfo() and associated methods to persist these values through level changes ***
		spawnedItem->spawnArgs.Set ( "inv_name", inventory.items[invItemIndex]->GetString( "inv_name" ) );
		
		// *************************************************************************************

		// Populate dictionary with key vals of dropping item
		idDict *droppingItem = inventory.items[invItemIndex];

		// Dropping the weapon the player is currently using?
		if ( strcmp( inventory.weaponUniqueName, droppingItem->GetString( "inv_unique_name" ) ) == 0 ) {

				// Clear the inventory / weapon unique name
				inventory.weaponUniqueName = "";

				// Now select the fists
				SelectWeapon( ARX_FISTS_WEAPON, true );
		}

		// Un-equip the item if equiped (if any match)
		int i;
		for ( i = 0; i < ARX_EQUIPED_ITEMS_MAX; i++ ) {
			if ( inventory.arx_equiped_items[ i ] == droppingItem->GetString( "inv_unique_name" ) ) {
				inventory.arx_equiped_items[ i ] == "";
			}
		}

		// Now remove the item from the players inventory
		RemoveInventoryItem( droppingItem ); 

		// Save persistent info
		SaveTransitionInfoSpecific( spawnedItem, true, false );
	}
	else
	{
		gameLocal.Printf( "Item spawned NO\n" );
	}
}

void idPlayer::UpdateShoppingSystem( void )
{
	if ( shoppingSystem && shoppingSystemOpen )
	{
		/*********************************************************************************************************/
		/*********************************************************************************************************/
		/*********************************************************************************************************/
		/*** From UpdateInventoryGUI ***/ 

		int totalMana; // Solarsplace 26th April 2010 - Inventory related

		// Solarsplace 15th Oct 2011 - Money
		shoppingSystem->SetStateString( "player_money", va( "%i", inventory.money ) );

		// Solarsplace - 16th May 2010 - Poison related
		if ( inventory.arx_timer_player_poison >= gameLocal.time )
		{ shoppingSystem->SetStateString( "poisoned", "1" ); }
		else
		{ shoppingSystem->SetStateString( "poisoned", "0" ); }

		// Solarsplace 26th April 2010 - Inventory related
		// Show the mana total for the player
		totalMana = GetPlayerManaAmount();
		shoppingSystem->SetStateString( "player_totalmana", va( "%i", totalMana ) );

		// Solarsplace 26th April 2010 - Inventory related
		// Show the player health
		shoppingSystem->SetStateInt( "player_health", health );

		// This clears out the hud strings - not 100% sure if needed.
		int j, c = inventory.items.Num();
		shoppingSystem->SetStateInt( "inv_count", c );
		for ( j = 0; j < MAX_INVENTORY_ITEMS; j++ ) {
			shoppingSystem->SetStateString( va( "inv_name_%i", j ), "" );
			shoppingSystem->SetStateString( va( "inv_icon_%i", j ), "" );
			shoppingSystem->SetStateString( va( "inv_text_%i", j ), "" );
			shoppingSystem->SetStateString( va( "inv_group_count_%i", j ), "0" );
			shoppingSystem->SetStateString( va( "inv_shop_item_value_%i", j ), "" );
		}

		const idKeyValue *argPointer;
		const idKeyValue *argGroupCount;

		invItemGroupCount->Clear();
		invItemGroupPointer->Clear();

		bool hasInventoryKeyRing = false;
		if ( FindInventoryItemCount( "#str_item_00450" ) > 0 ) {
			hasInventoryKeyRing = true;
		}

		int itemGroupCount;

		for ( j = 0; j < c; j++ ) {

			idDict *item = inventory.items[j];

			if ( !item->GetBool( "inv_pda" ) ) {

				// Solarsplace - 5th July 2013 - Hide inventory keys if the player has a key ring.
				if ( hasInventoryKeyRing ) {
					if ( item->GetBool( "inv_arx_key", "0" ) ) {
						continue;
					}
				}

				// Solarsplace - 20th March 2015 - Only sell matching item types to matching shops
				int arx_item_flags = item->GetInt( "inv_arx_shop_flags", idStr( ARX_SHOP_ALL ) );
				if ( !arxShopFunctions.MatchShopFlags( arx_item_flags ) ) {
					continue;
				}

				const char *iname = common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) );
				const char *iicon = item->GetString( "inv_icon" );
				//const char *itext = item->GetString( "inv_text" );

				idStr n1 = iname;
				idStr n2 = va( "_%i", j );

				if ( item->GetBool( "inventory_nostack", "0" ) )
				{
					invItemGroupPointer->SetInt( va( "inventoryitem_%i", j ), j ); //invItemGroupPointer->SetInt( iname, j );
					invItemGroupCount->SetInt( va( "inventoryitem_%i", j ), 1 ); // invItemGroupCount->SetInt( iname, 1 );
				}
				else
				{
					if ( !invItemGroupPointer->FindKey( iname ) )
					{
						// Add 1 new item
						invItemGroupPointer->SetInt( iname, j );
						invItemGroupCount->SetInt( iname, 1 );
					}
					else
					{
						// We have this inv_name in the dictionary already. So update its quantity count.
						itemGroupCount = invItemGroupCount->GetInt( iname, "0" ) + 1;
						invItemGroupCount->SetInt( iname, itemGroupCount );
					}
				}
			}
		}

		c = invItemGroupPointer->GetNumKeyVals();

		for ( j = 0; j < c; j++ ) {

			argPointer = invItemGroupPointer->GetKeyVal( j );
			argGroupCount = invItemGroupCount->GetKeyVal( j );

			idDict *item = inventory.items[ atoi( argPointer->GetValue() ) ];

			const char *iname = common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) );
			const char *iicon = item->GetString( "inv_icon" );
			//const char *itext = item->GetString( "inv_text" );
			const char *ivalue = item->GetString( "inv_shop_item_value" );

			shoppingSystem->SetStateString( va( "inv_name_%i", j ), iname );
			shoppingSystem->SetStateString( va( "inv_icon_%i", j ), iicon );
			//shoppingSystem->SetStateString( va( "inv_text_%i", j ), itext );
			shoppingSystem->SetStateString( va( "inv_value_%i", j ), ivalue );

			shoppingSystem->SetStateString( va( "inv_group_count_%i", j ), argGroupCount->GetValue() );
		}

		/*********************************************************************************************************/
		/*********************************************************************************************************/
		/*********************************************************************************************************/
		/*** Shop ***/

		shoppingSystem->SetStateInt( "shop_inv_count", 999 ); // GUI just checks > 0

		for ( j = 0; j < MAX_INVENTORY_ITEMS; j++ ) {

			shoppingSystem->SetStateString( va( "shop_inv_icon_%i", j ), "" );
			shoppingSystem->SetStateString( va( "shop_inv_name_%i", j ), "" );
			shoppingSystem->SetStateString( va( "shop_inv_value_%i", j ), "" );
			shoppingSystem->SetStateString( va( "shop_inv_group_count_%i", j ), "0" ); // Note: Named different from dictionary shop_item_count_
		}

		for ( j = 0; j < MAX_INVENTORY_ITEMS; j++ ) {

			const char *sicon = arxShopFunctions.shopSlotItem_Dict->GetString( va( "shop_item_icon_%i", j ), "");
			const char *sname = common->GetLanguageDict()->GetString( arxShopFunctions.shopSlotItem_Dict->GetString( va( "shop_item_name_%i", j ), "") );
			const char *svalue = arxShopFunctions.shopSlotItem_Dict->GetString( va( "inv_shop_item_value_%i", j ), "");
			const char *scount = arxShopFunctions.shopSlotItem_Dict->GetString( va( "shop_item_count_%i", j ), "0");

			if ( atoi(scount) > 0 ) {

				shoppingSystem->SetStateString( va( "shop_inv_icon_%i", j ), sicon );
				shoppingSystem->SetStateString( va( "shop_inv_name_%i", j ), sname );
				shoppingSystem->SetStateString( va( "shop_inv_value_%i", j ), svalue );
				shoppingSystem->SetStateString( va( "shop_inv_group_count_%i", j ), scount );
			}
		}

		// !!! Critical - MUST DO THIS !!!
		shoppingSystem->StateChanged( gameLocal.time );
	}
}

void idPlayer::UpdateConversationSystem( void )
{
	int j = 0;
	int c = 0;
	int tempHealth = 0;
	int tempHealthMax = 0;
	int tempItemValue = 0;
	int counter = 0;
	float tempHealthPercent = 0;
	int blackSmithSkill = 0;
	int blackSmithCost = 0;
	float blackSmithMultiply = 0;
	const int roundUpHack = 1;
	idStr tempInvName;

	const int MAX_BLACKSMITH_REPAIR_ITEMS = 255; // This value set to a high level that should never be exceeded by the inventory items for repair total.
	const float defaultblackSmithMultiply = 0.9;

	if ( conversationSystem && conversationSystemOpen )
	{
		idStr questWindow;
		gameLocal.persistentLevelInfo.GetString( ARX_CHAR_QUEST_WINDOW + ARX_REC_SEP + conversationWindowQuestId, "0", questWindow );

		conversationSystem->SetStateString( "quest_visible_window", questWindow );

		// ***********************************************************************************************************
		// ***********************************************************************************************************
		// ***********************************************************************************************************
		// Blacksmith

		// Display the blacksmith skill
		if ( focusCharacter ) {
			blackSmithSkill = focusCharacter->spawnArgs.GetInt( "blacksmith_skill", idStr(ARX_DEFAULT_BLACKSMITH_SKILL) );
			blackSmithMultiply = focusCharacter->spawnArgs.GetFloat( "blacksmith_multiply", idStr(defaultblackSmithMultiply) );
		} else {
			// This should never happen, but just in case.
			blackSmithSkill = ARX_DEFAULT_BLACKSMITH_SKILL;
			blackSmithMultiply = defaultblackSmithMultiply;
		}
		conversationSystem->SetStateInt( "blackSmithSkill", blackSmithSkill );

		// Need to clear out the list incase anything has been repaired / removed.
		for ( j = 0; j < MAX_BLACKSMITH_REPAIR_ITEMS; j++ ) {
			conversationSystem->SetStateString( va( "listRepairItems_item_%i", j ), "" );
			conversationSystem->SetStateString( va( "listRepairItemsHidden_item_%i", j ), "" );
		}

		c = inventory.items.Num();
		counter = 0;
		for ( j = 0; j < c; j++ ) {

			idDict *item = inventory.items[j];

			if ( item->GetBool( "inv_allow_blacksmith", "0" ) ) {
			
				// Display current health / max health in repair list box
				tempHealth = item->GetInt( "inv_health", "0" );
				tempHealthMax = item->GetInt( "inv_health_max", "100" );
				if ( tempHealthMax == 0 ) { tempHealthMax = 100; } // Safety just in case it somehow gets set 0 to avoid divide by zero errors...

				tempHealthPercent = ( (float)tempHealth / (float)tempHealthMax ) * 100.0f;

				tempItemValue = item->GetInt( "inv_shop_item_value", "999999" ); // Set very high to make it obvious no proper value set

				if ( ( (int)tempHealthPercent + roundUpHack ) >= blackSmithSkill ) { continue; }

				blackSmithCost = BlackSmithRepairComputeCost( tempHealthMax, tempHealth, tempItemValue, blackSmithMultiply );

				// e.g. Iron Sword (25/100) - 245 Gold
				sprintf( tempInvName, "%s (%d/%d) - %d %s", common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) ),
					tempHealth, tempHealthMax, tempItemValue, common->GetLanguageDict()->GetString( "#str_item_00010" ) );

				conversationSystem->SetStateString( va( "listRepairItems_item_%i", counter ), tempInvName );
				conversationSystem->SetStateString( va( "listRepairItemsHidden_item_%i", counter ), idStr( j ) );
				conversationSystem->SetStateString( va( "listRepairItemsHiddenCost_item_%i", counter ), idStr( blackSmithCost ) );
			
				counter ++;
			}
		}

		// Players money
		conversationSystem->SetStateString( "player_money", va( "%i", inventory.money ) );

		// ***********************************************************************************************************
		// ***********************************************************************************************************
		// ***********************************************************************************************************

		// !!! Critical - MUST DO THIS !!!
		conversationSystem->StateChanged( gameLocal.time );
	}
}

int idPlayer::BlackSmithRepairComputeCost( int maxHealth, int currentHealth, int itemValue, float blackSmithMultiply )
{
	float repairRatio = 0;
	float repairCost = 0;

	repairRatio = ( (float)maxHealth - (float)currentHealth ) / (float)maxHealth;
	repairCost = (float)itemValue * repairRatio;

	if ( blackSmithMultiply != 0.f ) {
		repairCost *= blackSmithMultiply;
	}

	if ( ( repairCost > 0.f ) && ( repairCost < 1.f ) ) {
		repairCost = 1.f;
	}
	
	return (int)repairCost;
}

void idPlayer::UpdateInventoryGUI( void )
{
	// Solarsplace 14th April 2010 - Inventory related

	if ( inventorySystem && inventorySystemOpen )
	{
		int totalMana; // Solarsplace 26th April 2010 - Inventory related

		/* Solarsplace 24th Sep 2011 - This seems not to be used anymore in the gui's
		int x;
		for( x = 0; x < MAX_INVENTORY_ITEMS; x++ ) {
			inventorySystem->SetStateString( va( "inv_has_item_at_%i", x ), "0" );
			if ( x <= inventory.items.Num() ) {
				inventorySystem->SetStateString( va( "inv_has_item_at_%i", x ), "1" );
			}
		}
		*/

		// Solarsplace 15th Oct 2011 - Money
		inventorySystem->SetStateString( "player_money", va( "%i", inventory.money ) );

		// Solarsplace - 16th May 2010 - Poison related
		if ( inventory.arx_timer_player_poison >= gameLocal.time )
		{ inventorySystem->SetStateString( "poisoned", "1" ); }
		else
		{ inventorySystem->SetStateString( "poisoned", "0" ); }

		// Solarsplace 26th April 2010 - Inventory related
		// Show the mana total for the player
		totalMana = GetPlayerManaAmount();
		inventorySystem->SetStateString( "player_totalmana", va( "%i", totalMana ) );

		// Solarsplace 26th April 2010 - Inventory related
		// Show the player health
		inventorySystem->SetStateInt( "player_health", health );

		// This clears out the hud strings - not 100% sure if needed.
		int j, c = inventory.items.Num();
		inventorySystem->SetStateInt( "inv_count", c );
		for ( j = 0; j < MAX_INVENTORY_ITEMS; j++ ) {
			inventorySystem->SetStateString( va( "inv_name_%i", j ), "" );
			inventorySystem->SetStateString( va( "inv_icon_%i", j ), "" );
			inventorySystem->SetStateString( va( "inv_text_%i", j ), "" );
			inventorySystem->SetStateString( va( "inv_group_count_%i", j ), "0" ); // Solarsplace 24th Sep 2011 - Reset item groupings totals.
		}

		const idKeyValue *argPointer;
		const idKeyValue *argGroupCount;

		invItemGroupCount->Clear();
		invItemGroupPointer->Clear();

		int itemGroupCount;

		for ( j = 0; j < c; j++ ) {

			idDict *item = inventory.items[j];

			if ( !item->GetBool( "inv_pda" ) ) {

				const char *iname = common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) );

				if ( item->GetBool( "inventory_nostack", "0" ) ) {
					invItemGroupPointer->SetInt( va( "inventoryitem_%i", j ), j );
					invItemGroupCount->SetInt( va( "inventoryitem_%i", j ), 1 );
				}
				else
				{
					if ( !invItemGroupPointer->FindKey( iname ) ) {
						// Add 1 new item
						invItemGroupPointer->SetInt( iname, j );
						invItemGroupCount->SetInt( iname, 1 );
					}
					else {
						// We have this inv_name in the dictionary already. So update its quantity count.
						itemGroupCount = invItemGroupCount->GetInt( iname, "0" ) + 1;
						invItemGroupCount->SetInt( iname, itemGroupCount );
					}
				}
			}
		}

		c = invItemGroupPointer->GetNumKeyVals();

		for ( j = 0; j < c; j++ ) {

			argPointer = invItemGroupPointer->GetKeyVal( j );
			argGroupCount = invItemGroupCount->GetKeyVal( j );

			idDict *item = inventory.items[ atoi( argPointer->GetValue() ) ];

			//const char *iname = common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) );
			int tempHealth = item->GetInt( "inv_health", "0" );
			int tempHealthMax = item->GetInt( "inv_health_max", "100" );
			const char *iname;

			if ( !item->GetBool( "inv_allow_weapon_damage", "0" ) ) {
				iname = common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) );
			} else {
				// Display current health / max health in inventory
				idStr tempInvName;
				sprintf( tempInvName, "%s (%d/%d)", common->GetLanguageDict()->GetString( item->GetString( "inv_name" ) ), tempHealth, tempHealthMax );
				iname = tempInvName.c_str();
			}

			//const char *iicon = item->GetString( "inv_icon" );
			const char *iicon = GetInventoryItemHealthIcon( tempHealth, tempHealthMax, *item );

			const char *itext = item->GetString( "inv_text" );

			inventorySystem->SetStateString( va( "inv_name_%i", j ), iname );
			inventorySystem->SetStateString( va( "inv_icon_%i", j ), iicon );
			inventorySystem->SetStateString( va( "inv_text_%i", j ), itext );

			inventorySystem->SetStateString( va( "inv_group_count_%i", j ), argGroupCount->GetValue() );
		}

	}
}

void idPlayer::UpdateJournalGUI( void )
{
	// Solarsplace - Arx End Of Sun

	if ( objectiveSystem && objectiveSystemOpen )
	{
		// *****************************************************************
		// *****************************************************************
		// *****************************************************************
		// *** Book skills

		// *** Equipped armour
		// LOGIC TODO
		const int ARX_PLAYER_ARMOUR_NONE = 0;
		const int ARX_PLAYER_ARMOUR_GOBINSKINS = 1;
		const int ARX_PLAYER_ARMOUR_HUMANGUARD = 2;
		const int ARX_PLAYER_ARMOUR_TRAVELERS = 3;
		objectiveSystem->SetStateInt( "arx_player_armour", ARX_PLAYER_ARMOUR_GOBINSKINS );

		idStr equipedItemIcon;

		// *** Left ring equipment
		equipedItemIcon = GetInventoryItemString( inventory.arx_equiped_items[ ARX_EQUIPED_RING_LEFT ], "inv_icon" );
		if ( strcmp( equipedItemIcon, "" ) == 0 ) {
			objectiveSystem->SetStateBool( "ring_left_equipped", false );
			objectiveSystem->SetStateString( "ring_left_icon", "" );
		} else {
			objectiveSystem->SetStateBool( "ring_left_equipped", true );
			objectiveSystem->SetStateString( "ring_left_icon", equipedItemIcon.c_str() );
		}

		// *** Right ring equipment
		equipedItemIcon = GetInventoryItemString( inventory.arx_equiped_items[ ARX_EQUIPED_RING_RIGHT ], "inv_icon" );
		if ( strcmp( equipedItemIcon, "" ) == 0 ) {
			objectiveSystem->SetStateBool( "ring_right_equipped", false );
			objectiveSystem->SetStateString( "ring_right_icon", "" );
		} else {
			objectiveSystem->SetStateBool( "ring_right_equipped", true );
			objectiveSystem->SetStateString( "ring_right_icon", equipedItemIcon.c_str() );
		}

		// Level and XP's
		objectiveSystem->SetStateInt( "arx_player_level", inventory.arx_player_level );
		objectiveSystem->SetStateInt( "arx_player_x_points", inventory.arx_player_x_points );

		bool hasAttributePointsToSpend = false;
		if ( inventory.arx_attribute_points > 0 ) {
			hasAttributePointsToSpend = true;
		}

		bool hasSkillPointsToSpend = false;
		if ( inventory.arx_skill_points > 0 ) {
			hasSkillPointsToSpend = true;
		}

		/*
		inventory.tmp_arx_attribute_points = inventory.arx_attribute_points;
		inventory.tmp_arx_skill_points = inventory.arx_skill_points;

		inventory.tmp_arx_attr_strength = inventory.arx_attr_strength;
		inventory.tmp_arx_attr_mental = inventory.arx_attr_mental;
		inventory.tmp_arx_attr_dexterity = inventory.arx_attr_dexterity;
		inventory.tmp_arx_attr_constitution = inventory.arx_attr_constitution;

		inventory.tmp_arx_skill_casting = inventory.arx_skill_casting;
		inventory.tmp_arx_skill_close_combat = inventory.arx_skill_close_combat;
		inventory.tmp_arx_skill_defense = inventory.arx_skill_defense;
		inventory.tmp_arx_skill_ethereal_link = inventory.arx_skill_ethereal_link;
		inventory.tmp_arx_skill_intuition = inventory.arx_skill_intuition;
		inventory.tmp_arx_skill_intelligence = inventory.arx_skill_intelligence;
		inventory.tmp_arx_skill_projectile = inventory.arx_skill_projectile;
		inventory.tmp_arx_skill_stealth = inventory.arx_skill_stealth;
		inventory.tmp_arx_skill_technical = inventory.arx_skill_technical;
		*/

		// *** Start - Only show increment button if their are appropriate points to spend
		objectiveSystem->SetStateBool( "arx_attr_strength_inc_visible", hasAttributePointsToSpend );
		objectiveSystem->SetStateBool( "arx_attr_mental_inc_visible", hasAttributePointsToSpend );
		objectiveSystem->SetStateBool( "arx_attr_dexterity_inc_visible", hasAttributePointsToSpend );
		objectiveSystem->SetStateBool( "arx_attr_constitution_inc_visible", hasAttributePointsToSpend );

		objectiveSystem->SetStateBool( "arx_skill_casting_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_close_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_defense_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_ethereal_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_intuition_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_intelligence_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_projectile_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_stealth_inc_visible", hasSkillPointsToSpend );
		objectiveSystem->SetStateBool( "arx_skill_technical_inc_visible", hasSkillPointsToSpend );
		// *** End - Only show increment button if their are appropriate points to spend

		// *** Start - Only allow increment if attr or skill points to spend
		bool allowDecrement;
		allowDecrement = ( inventory.tmp_arx_attr_strength > inventory.arx_attr_strength );
		objectiveSystem->SetStateBool( "arx_attr_strength_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_attr_mental > inventory.arx_attr_mental );
		objectiveSystem->SetStateBool( "arx_attr_mental_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_attr_dexterity > inventory.arx_attr_dexterity );
		objectiveSystem->SetStateBool( "arx_attr_dexterity_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_attr_constitution > inventory.arx_attr_constitution );
		objectiveSystem->SetStateBool( "arx_attr_constitution_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_casting > inventory.arx_skill_casting );
		objectiveSystem->SetStateBool( "arx_skill_casting_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_close_combat > inventory.arx_skill_close_combat );
		objectiveSystem->SetStateBool( "arx_skill_close_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_defense > inventory.arx_skill_defense );
		objectiveSystem->SetStateBool( "arx_skill_defense_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_ethereal_link > inventory.arx_skill_ethereal_link );
		objectiveSystem->SetStateBool( "arx_skill_ethereal_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_intuition > inventory.arx_skill_intuition );
		objectiveSystem->SetStateBool( "arx_skill_intuition_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_intelligence > inventory.arx_skill_intelligence );
		objectiveSystem->SetStateBool( "arx_skill_intelligence_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_projectile > inventory.arx_skill_projectile );
		objectiveSystem->SetStateBool( "arx_skill_projectile_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_stealth > inventory.arx_skill_stealth );
		objectiveSystem->SetStateBool( "arx_skill_stealth_dec_visible", allowDecrement );

		allowDecrement = ( inventory.tmp_arx_skill_technical > inventory.arx_skill_technical );
		objectiveSystem->SetStateBool( "arx_skill_technical_dec_visible", allowDecrement );
		// *** End - dont allow decrement lower than current value

		// Attributes & attribute points
		objectiveSystem->SetStateBool( "arx_attribute_points_visible", hasAttributePointsToSpend );
		objectiveSystem->SetStateInt( "arx_attribute_points", inventory.tmp_arx_attribute_points );
		objectiveSystem->SetStateInt( "arx_attr_constitution", inventory.tmp_arx_attr_constitution ); // 4
		objectiveSystem->SetStateInt( "arx_attr_dexterity", inventory.tmp_arx_attr_dexterity ); // 3
		objectiveSystem->SetStateInt( "arx_attr_mental", inventory.tmp_arx_attr_mental ); // 2
		objectiveSystem->SetStateInt( "arx_attr_strength", inventory.tmp_arx_attr_strength ); // 1

		// Skills & skill points
		objectiveSystem->SetStateBool( "arx_skill_points_visible", hasSkillPointsToSpend && !hasAttributePointsToSpend );
		objectiveSystem->SetStateInt( "arx_skill_points", inventory.tmp_arx_skill_points );
		objectiveSystem->SetStateInt( "arx_skill_casting", inventory.tmp_arx_skill_casting ); // 10
		objectiveSystem->SetStateInt( "arx_skill_close_combat", inventory.tmp_arx_skill_close_combat ); // 11
		objectiveSystem->SetStateInt( "arx_skill_defense", inventory.tmp_arx_skill_defense ); // 13
		objectiveSystem->SetStateInt( "arx_skill_ethereal_link", inventory.tmp_arx_skill_ethereal_link ); // 8
		objectiveSystem->SetStateInt( "arx_skill_intelligence", inventory.tmp_arx_skill_intelligence ); // 9
		objectiveSystem->SetStateInt( "arx_skill_intuition", inventory.tmp_arx_skill_intuition ); // 7
		objectiveSystem->SetStateInt( "arx_skill_projectile", inventory.tmp_arx_skill_projectile ); // 12
		objectiveSystem->SetStateInt( "arx_skill_stealth", inventory.tmp_arx_skill_stealth ); // 5
		objectiveSystem->SetStateInt( "arx_skill_technical", inventory.tmp_arx_skill_technical ); // 6

		// Apply button for skills and attributes
		bool allowStatsAttrApply = ( inventory.arx_attribute_points > 0 && inventory.arx_skill_points > 0 && inventory.tmp_arx_attribute_points == 0 && inventory.tmp_arx_skill_points == 0 );
		objectiveSystem->SetStateBool( "arx_points_apply_button_visible", allowStatsAttrApply ); // LOGIC = TODO

		// Player class totals
		objectiveSystem->SetStateInt( "arx_class_armour_points", inventory.arx_class_armour_points ); // 1
		objectiveSystem->SetStateInt( "arx_class_damage_points", inventory.arx_class_damage_points ); // 6
		objectiveSystem->SetStateInt( "arx_class_health_points", inventory.arx_class_health_points ); // 2
		objectiveSystem->SetStateInt( "arx_class_mana_points", inventory.arx_class_mana_points ); // 4
		objectiveSystem->SetStateInt( "arx_class_resistance_to_magic", inventory.arx_class_resistance_to_magic ); // 3
		objectiveSystem->SetStateInt( "arx_class_resistance_to_poison", inventory.arx_class_resistance_to_poison ); // 5

		// Secrets found
		objectiveSystem->SetStateInt( "arx_stat_secrets_found", inventory.arx_stat_secrets_found );

		// *****************************************************************
		// *****************************************************************
		// *****************************************************************

		int totalMana; // Solarsplace 16th May 2010 - Journal related

		// Solarsplace - 16th May 2010 - Poison related
		if ( inventory.arx_timer_player_poison >= gameLocal.time )
		{ objectiveSystem->SetStateString( "poisoned", "1" ); }
		else
		{ objectiveSystem->SetStateString( "poisoned", "0" ); }

		// Solarsplace 26th April 2010 - Inventory related
		// Show the mana total for the player
		totalMana = GetPlayerManaAmount();
		objectiveSystem->SetStateString( "player_totalmana", va( "%i", totalMana ) );

		// Solarsplace 26th April 2010 - Inventory related
		// Show the player health
		objectiveSystem->SetStateInt( "player_health", health );

		// Runes -- Not sure if this is efficient? suspect not.... Don't see the game doing it anywhere :(
		const char *result;

		gameLocal.persistentLevelInfo.GetString( "aam", "0", &result );
		objectiveSystem->SetStateString( "amm", result );

		gameLocal.persistentLevelInfo.GetString( "nhi", "0", &result );
		objectiveSystem->SetStateString( "nhi", result );

		gameLocal.persistentLevelInfo.GetString( "mega", "0", &result );
		objectiveSystem->SetStateString( "mega", result );

		gameLocal.persistentLevelInfo.GetString( "yok", "0", &result );
		objectiveSystem->SetStateString( "yok", result );

		gameLocal.persistentLevelInfo.GetString( "taar", "0", &result );
		objectiveSystem->SetStateString( "taar", result );

		gameLocal.persistentLevelInfo.GetString( "kaom", "0", &result );
		objectiveSystem->SetStateString( "kaom", result );

		gameLocal.persistentLevelInfo.GetString( "vitae", "0", &result );
		objectiveSystem->SetStateString( "vitae", result );

		gameLocal.persistentLevelInfo.GetString( "vista", "0", &result );
		objectiveSystem->SetStateString( "vista", result );

		gameLocal.persistentLevelInfo.GetString( "stregum", "0", &result );
		objectiveSystem->SetStateString( "stregum", result );

		gameLocal.persistentLevelInfo.GetString( "morte", "0", &result );
		objectiveSystem->SetStateString( "morte", result );

		gameLocal.persistentLevelInfo.GetString( "cosum", "0", &result );
		objectiveSystem->SetStateString( "cosum", result );

		gameLocal.persistentLevelInfo.GetString( "comunicatum", "0", &result );
		objectiveSystem->SetStateString( "comunicatum", result );

		gameLocal.persistentLevelInfo.GetString( "movis", "0", &result );
		objectiveSystem->SetStateString( "movis", result );

		gameLocal.persistentLevelInfo.GetString( "tempus", "0", &result );
		objectiveSystem->SetStateString( "tempus", result );

		gameLocal.persistentLevelInfo.GetString( "folgora", "0", &result );
		objectiveSystem->SetStateString( "folgora", result );

		gameLocal.persistentLevelInfo.GetString( "spacium", "0", &result );
		objectiveSystem->SetStateString( "spacium", result );

		gameLocal.persistentLevelInfo.GetString( "tera", "0", &result );
		objectiveSystem->SetStateString( "tera", result );

		gameLocal.persistentLevelInfo.GetString( "cetrius", "0", &result );
		objectiveSystem->SetStateString( "cetrius", result );

		gameLocal.persistentLevelInfo.GetString( "rhaa", "0", &result );
		objectiveSystem->SetStateString( "rhaa", result );

		gameLocal.persistentLevelInfo.GetString( "fridd", "0", &result );
		objectiveSystem->SetStateString( "fridd", result );


		/*****************************************************************************
		 *****************************************************************************
		 *****************************************************************************
		 *****************************************************************************/

		gameLocal.persistentLevelInfo.GetString( "spell_mega_vista", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_vista", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_taar", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_taar", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_yok", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_yok", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_yok", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_yok", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_spacium", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_spacium", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_morte_cosum_vista", "0", &result );
		objectiveSystem->SetStateString( "spell_morte_cosum_vista", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_kaom", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_kaom", result );

		gameLocal.persistentLevelInfo.GetString( "spell_rhaa_kaom", "0", &result );
		objectiveSystem->SetStateString( "spell_rhaa_kaom", result );

		gameLocal.persistentLevelInfo.GetString( "spell_rhaa_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_rhaa_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_stregum_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_stregum_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_yok_taar", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_yok_taar", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_cosum_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_cosum_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_yok_fridd", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_yok_fridd", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_stregum_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_stregum_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_spacium", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_spacium", result );

		gameLocal.persistentLevelInfo.GetString( "spell_yok_kaom", "0", &result );
		objectiveSystem->SetStateString( "spell_yok_kaom", result );

		gameLocal.persistentLevelInfo.GetString( "spell_spacium_comunicatum", "0", &result );
		objectiveSystem->SetStateString( "spell_spacium_comunicatum", result );

		gameLocal.persistentLevelInfo.GetString( "spell_rhaa_stregum_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_rhaa_stregum_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_fridd_kaom", "0", &result );
		objectiveSystem->SetStateString( "spell_fridd_kaom", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_morte_cosum", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_morte_cosum", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_spacium_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_spacium_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_cetrius", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_cetrius", result );

		gameLocal.persistentLevelInfo.GetString( "spell_morte_kaom", "0", &result );
		objectiveSystem->SetStateString( "spell_morte_kaom", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_taar_cetrius", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_taar_cetrius", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_morte_vitae", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_morte_vitae", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_kaom_spacium", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_kaom_spacium", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_morte_cosum", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_morte_cosum", result );

		gameLocal.persistentLevelInfo.GetString( "spell_rhaa_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_rhaa_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_vista_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_vista_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_yok_spacium", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_yok_spacium", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_folgora_taar", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_folgora_taar", result );

		gameLocal.persistentLevelInfo.GetString( "spell_rhaa_vista", "0", &result );
		objectiveSystem->SetStateString( "spell_rhaa_vista", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_fridd_spacium", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_fridd_spacium", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_vista", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_vista", result );

		gameLocal.persistentLevelInfo.GetString( "spell_stregum_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_stregum_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_aam_yok", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_aam_yok", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_stregum_cosum", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_stregum_cosum", result );

		gameLocal.persistentLevelInfo.GetString( "spell_vitae_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_vitae_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_vitae_tera", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_vitae_tera", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_stregum_spacium", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_stregum_spacium", result );

		gameLocal.persistentLevelInfo.GetString( "spell_aam_mega_yok", "0", &result );
		objectiveSystem->SetStateString( "spell_aam_mega_yok", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_nhi_movis", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_nhi_movis", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_aam_taar_folgora", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_aam_taar_folgora", result );

		gameLocal.persistentLevelInfo.GetString( "spell_movis_comunicatum", "0", &result );
		objectiveSystem->SetStateString( "spell_movis_comunicatum", result );

		gameLocal.persistentLevelInfo.GetString( "spell_nhi_tempus", "0", &result );
		objectiveSystem->SetStateString( "spell_nhi_tempus", result );

		gameLocal.persistentLevelInfo.GetString( "spell_mega_aam_mega_yok", "0", &result );
		objectiveSystem->SetStateString( "spell_mega_aam_mega_yok", result );

		/*****************************************************************************
		 *****************************************************************************
		 *****************************************************************************
		 *****************************************************************************/

	}
}

/*
===============
idPlayerConsumeInventoryItem

Returns false if the item shouldn't be consumed
===============
*/
bool idPlayer::ConsumeInventoryItem( int invItemIndex ) {

	// Solarsplace - Arx End Of Sun

	//  Inventory safety
	if ( inventory.items.Num() <= 0 || invItemIndex < 0 || invItemIndex > inventory.items.Num() )
	{ return false; }

	// 14th Nov 2013 - Cannot do this if dead...
	if ( health <= 0 ) { return false; }

	// Common D3 properties and variables
	int					i;
	const idKeyValue	*arg;
	bool				gave = false;
	const char			*sound;
	const char			*itemAttribute;

	// Common Arx properties and variables
	bool	processedItem = false;
	const	char *iname;
	idStr	equipType;
	idStr	uniqueName;
	int		itemHealth;
	int		itemHealthMax;

	// Get the inv_name of the item and use this to put its spawn args in item
	iname = inventory.items[invItemIndex]->GetString( "inv_name" );
	idDict *item = FindInventoryItem( iname );

	// Gather common information
	inventory.items[invItemIndex]->GetInt( "inv_health", "0", itemHealth );
	inventory.items[invItemIndex]->GetInt( "inv_health_max", "100", itemHealthMax );
	inventory.items[invItemIndex]->GetString( "inv_arx_equipable_type", "", equipType );
	inventory.items[invItemIndex]->GetString( "inv_unique_name", "", uniqueName );

	sound = gameLocal.GetStringFromEntityDef( inventory.items[invItemIndex]->GetString( "inv_classname", "" ), "snd_consume" );

	// ********************************************************************************************
	// ********************************************************************************************
	// ********************************************************************************************
	// *** WEAPONS

	if ( !strcmp( item->GetString( "inv_weapon", "" ), "" ) == 0 )
	{

		if ( itemHealth > 0 ) {

			// Clear any previous equiped weapon 
			inventory.arx_equiped_items[ ARX_EQUIPED_WEAPON ] = "";

			// Switch to the inventory selected weapon
			int weaponId = inventory.items[invItemIndex]->GetInt( "inv_weapon_def" );
			SelectWeapon( weaponId, false );

			// Set the unique name of this weapon: This is basically used to tie the weapon class with the current weapon
			inventory.weaponUniqueName = uniqueName;

			// Set the weapon health
			weapon.GetEntity()->health = itemHealth;

			// Set the weapon max health
			weapon.GetEntity()->health_max = itemHealthMax;

			// Optional, may wish to play an equip sound.
			if ( sound )
			{ StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL ); }

		}

		processedItem = true;
	}

	// ********************************************************************************************
	// ********************************************************************************************
	// ********************************************************************************************
	// *** EQUIPABLE ITEMS

	if ( item->GetBool( "inv_arx_equipable_item", "0" ) ) {

		bool itemEquiped = false;

		if ( equipType == "ring" ) {

			bool leftRingFull = false;
			bool rightRingFull = false;

			if ( inventory.arx_equiped_items[ ARX_EQUIPED_RING_LEFT ] != "" ) { leftRingFull = true; }
			if ( inventory.arx_equiped_items[ ARX_EQUIPED_RING_RIGHT ] != "" ) { rightRingFull = true; }

			// If left and right rings are already equiped swap the left one.
			if ( leftRingFull && rightRingFull ) {
				inventory.arx_equiped_items[ ARX_EQUIPED_RING_LEFT ] = uniqueName;
				itemEquiped = true;
			} else {
				// Else file the empty ring slot
				if ( !leftRingFull ) {
					inventory.arx_equiped_items[ ARX_EQUIPED_RING_LEFT ] = uniqueName;
					itemEquiped = true;
				} else {
					inventory.arx_equiped_items[ ARX_EQUIPED_RING_RIGHT ] = uniqueName;
					itemEquiped = true;
				}
			}

		} else if ( equipType == "weapon" ) {

			inventory.arx_equiped_items[ ARX_EQUIPED_WEAPON ] = uniqueName;
			itemEquiped = true;
		}

		if ( itemEquiped ) {
			// Optional, may wish to play an equip sound.
			if ( sound )
			{ StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL ); }
		}

		processedItem = true;
	}

	// SP - Arx EOS - Return if item handled above.
	if ( processedItem ) {
		return true;
	}

	if ( item )
	{
		gave = false;

		for( i = 0; i < item->GetNumKeyVals(); i++ ) {

			arg = item->GetKeyVal( i );
			if ( arg->GetKey().Left( 4 ) == "inv_" ) {
				if ( Give( arg->GetKey().Right( arg->GetKey().Length() - 4 ), arg->GetValue() ) )
				{ gave = true; }
			}

			//*********************************************************************************
			//*********************************************************************************
			//*********************************************************************************
			// Solarsplace - Arx End Of Sun - Special case functions.

			if ( arg->GetKey() == "inv_arx_item_attribute" )
			{
				itemAttribute = arg->GetValue();

				// Invisibility
				if ( strcmp( itemAttribute, "add_invisibility" ) == 0 )
				{
					inventory.arx_timer_player_invisible = gameLocal.time + ARX_INVIS_TIME;
					GivePowerUp( 1, ARX_INVIS_TIME );
					gave = true;
				}

				// Telekenesis
				if ( strcmp( itemAttribute, "add_telekinesis" ) == 0 )
				{
					inventory.arx_timer_player_telekinesis = gameLocal.time + ARX_TELEKENESIS_TIME;
					gave = true;
				}

				// Levitate
				if ( strcmp( itemAttribute, "add_levitate" ) == 0 )
				{
					Event_LevitateStart();
					inventory.arx_timer_player_levitate = gameLocal.time + ARX_LEVITATE_TIME;
					gave = true;
				}

				// Cure poison
				if ( strcmp( itemAttribute, "remove_poison" ) == 0 )
				{
					inventory.arx_timer_player_poison = gameLocal.time;
					gave = true;
				}

				// Drink Wine
				if ( strcmp( itemAttribute, "add_wine" ) == 0 )
				{
					// Health gain handled above in original 'Give'
					Damage( this, this, vec3_origin, "damage_arx_drunk", 1.0f, INVALID_JOINT );
				}

				// Ignight Wooden flame torch
				if ( strcmp( itemAttribute, "add_flametorch" ) == 0 )
				{
					// Spawn the hidden light entitiy. The entitiy script object will do the rest.
					idDict args;
					idEntity *spawnedItem;
					args.Set( "classname", "arx_light_hidden_torch_timed" );
					gameLocal.SpawnEntityDef( args, &spawnedItem );
					gave = true;
				}
			}

			//*********************************************************************************
			//*********************************************************************************
			//*********************************************************************************
		}
	}
	else
	{
		// Play a sound to indicate nothing to pickup.
		StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
	}

	if ( gave )
	{
		RemoveInventoryItem( item );

		if ( sound ) {
			StartSoundShader( declManager->FindSound( sound ), SND_CHANNEL_ANY, 0, false, NULL );
		}
	}

	return gave;
}

/*
==============
idPlayer::AlertAI
==============
*/
void idPlayer::AlertAI( bool playerVisible, float alertRadius, int aiTeam, int teamAlertOptions ) {

	// Solarsplace - Arx End Of Sun

	int			e;
	idEntity *	ent;
	idEntity *	entityList[ MAX_GENTITIES ];
	int			numListedEntities;
	idBounds	bounds;

	if ( inventory.arx_timer_player_invisible > gameLocal.time ) // Do not alert AI or set enemy if player is invisible.
	{ return; };

	bounds = idBounds( GetPhysics()->GetOrigin() ).Expand( alertRadius );

	const idActor *actor = static_cast<const idActor *>( this );

	// Get all entities touching the bounds
	numListedEntities = gameLocal.clip.EntitiesTouchingBounds( bounds, -1, entityList, MAX_GENTITIES );

	for ( e = 0; e < numListedEntities; e++ ) {

		ent = entityList[ e ];

		if ( ent->IsType( idAI::Type ) ) {

			if ( playerVisible ) {
				if ( !static_cast<idActor *>( ent )->CanSee(this, true) ) { continue; }
			}

			// Do not alert this actor. This actor will not respond to help signals from other attacked AI.
			if ( static_cast<idActor *>( ent )->spawnArgs.GetBool( "arx_alertai_ignore", "0" ) ) { continue; }

			/*
			0 = Any team
			1 = Equal team only
			2 = Equal or greater team only
			*/

			if ( teamAlertOptions == 1 ) {
				if ( ent->spawnArgs.GetInt( "team", "0" ) != aiTeam ) {
					continue;
				}
			}

			if ( teamAlertOptions == 2 ) {
				if ( ent->spawnArgs.GetInt( "team", "0" ) < aiTeam ) {
					continue;
				}
			}

			gameLocal.AlertAI( this );
			static_cast<idAI *>( ent )->SetEnemy( static_cast<idActor *>( this ) );
						
		}
	}
}

/*
==============
idPlayer::RadiusSpell
==============
*/
void idPlayer::RadiusSpell( idStr scriptAction, float alertRadius ) {

	// Solarsplace - Arx End Of Sun - 31st May 2012

	int			e;
	idEntity *	ent;
	idEntity *	entityList[ MAX_GENTITIES ];
	int			numListedEntities;
	idBounds	bounds;

	// Script functions
	const function_t	*func;
	idThread			*thread;

	bounds = idBounds( GetPhysics()->GetOrigin() ).Expand( alertRadius );

	// Get all entities touching the bounds
	numListedEntities = gameLocal.clip.EntitiesTouchingBounds( bounds, -1, entityList, MAX_GENTITIES );

	for ( e = 0; e < numListedEntities; e++ ) {

		ent = entityList[ e ];

		func = ent->scriptObject.GetFunction( scriptAction.c_str() );

		if ( func )
		{
			// create a thread and call the function
			thread = new idThread();
			thread->CallFunction( ent, func, true );
			thread->Start();
		}
	}
}

/*
==============
idPlayer::GetEntityByViewRay
==============
*/

void idPlayer::GetEntityByViewRay( void )
{
	// Solarsplace - Arx End Of Sun

	// 14th Nov 2013 - Cannot do this if dead...
	if ( health <= 0 ) { return; }

	// Require 'use' key to instigate NPC GUI
	if ( focusCharacter && ( focusCharacter->health > 0 ) )
	{
		Weapon_NPC();
		return;
	}

	trace_t trace;
	idPlayer * player = gameLocal.GetLocalPlayer();
	idEntity * target;
	float pickupDistance;
	idDict *inventoryItem;
	idStr entityClassName;
	idStr requiredItemInvName;

	if ( inventory.arx_timer_player_telekinesis >= gameLocal.GetTime() ) {
		pickupDistance = ARX_MAX_ITEM_PICKUP_DISTANCE_TELE;
	} else {
		pickupDistance = ARX_MAX_ITEM_PICKUP_DISTANCE;
	}

	idVec3 start = firstPersonViewOrigin;
	idVec3 end = start + firstPersonViewAxis[0] * ( pickupDistance );

	// Solarsplace 10th May 2012 - Changed mask type to custom for Arx
	gameLocal.clip.TracePoint( trace, start, end, MASK_ARX_LEVEL_USE_TRIG, player );

	// Solarsplace 10th May 2012
	if( ( trace.fraction < 1.0f ) && ( trace.c.entityNum != ENTITYNUM_NONE ) )
	{
		target = gameLocal.entities[ trace.c.entityNum ];

		if ( target->IsType( idTrigger::Type ) )
		{
			// If idEntity is a trigger that does not have the bool spawn arg arx_usable_item set
			// then we repeat the trace again with different masks so we see through the trigger
			// this is so we can pickup up food from within a fire damage trigger for example.
			if ( !target->spawnArgs.GetBool( "arx_usable_item", "0" ) )
			{
				gameLocal.clip.TracePoint( trace, start, end, MASK_ARX_LEVEL_USE_NOTRIG, gameLocal.GetLocalPlayer() );
			}
		}
	}

	if( ( trace.fraction < 1.0f ) && ( trace.c.entityNum != ENTITYNUM_NONE ) )
	{
		target = gameLocal.entities[ trace.c.entityNum ];

		// Solarsplace 2nd April 2010 - It is critical that the inv_ remains
		if ( target->spawnArgs.GetBool( "inv_arx_inventory_item" ) && !target->IsHidden() )
		{

			// Solarsplace 9th Oct 2011 - Does this item require another inventory item before it can be picked up?
			requiredItemInvName = target->spawnArgs.GetString( "requires_inv_item", "" );
			if ( idStr::Icmp( requiredItemInvName, "" ) != 0 ) // Updated to be in line with the level change code below 20th Nov 2010
			{
				inventoryItem = FindInventoryItem( target->spawnArgs.GetString( "requires_inv_item" ) );
				if ( !inventoryItem )
				{
					// Play a sound to indicate nothing to pickup.
					StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
					return;
				}
			}

			bool arx_no_pickup = target->spawnArgs.GetBool( "arx_no_pickup", "0" );
			if ( arx_no_pickup ) {
				// Play a sound to indicate nothing to pickup.
				StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
				ShowHudMessage(  target->spawnArgs.GetString( "arx_no_pickup_message", common->GetLanguageDict()->GetString( "#str_general_00015" ) ) ); // Default: This object cannot be picked up
				return;
			}

			//*** Start - Solarsplace 13th Oct 2011 - Should we alert AI?
			bool alertai;
			bool alertaifov;
			float alertradius;

			target->spawnArgs.GetBool( "alertai", "0", alertai );
			target->spawnArgs.GetBool( "alertfov", "0", alertaifov );
			target->spawnArgs.GetFloat( "alerteffectradius", "1024", alertradius );

			if ( alertai ) {
				AlertAI( alertaifov, alertradius, 0, 0 );
			}
			//*** End - Solarsplace 13th Oct 2011 - Should we alert AI?

			// Solarsplace 25th Sep 2011
			if ( target->spawnArgs.GetString( "inv_name" ) )
			{ ShowHudMessage( target->spawnArgs.GetString( "inv_name" ) ); }

			// Solarsplace 4th Oct 2010 - Level transition related
			SaveTransitionInfoSpecific( target, false, true );

			// hide the model
			target->Hide();

			//REMOVEMEx
			//gameLocal.Printf( "idPlayer::GetEntityByViewRay name: %s\n ", target->name.c_str() );
			//gameLocal.Printf( "idPlayer::GetEntityByViewRay inv_name: %s\n ", target->spawnArgs.GetString( "inv_name" ) );

			idDict &args = target->spawnArgs;

			// Solarsplace 4th Oct 2010 - Level transition related
			// Need to save 'classname' args in the inventory args as 'inv_classname' so it is persisted through level transitions

			// Solarsplace 9th Oct 2011 - If we specify a drop item use that.
			if ( target->spawnArgs.GetString( "def_dropItem" ) )
			{
				entityClassName = target->spawnArgs.GetString( "def_dropItem" );
			}
			else
			{
				entityClassName = target->spawnArgs.GetString( "classname" );
			}
			args.Set( "inv_classname", entityClassName );

			// trigger our targets
			ActivateTargets( this );

			// clear our contents so the object isn't picked up twice
			GetPhysics()->SetContents( 0 );

			PlaySpecificEntitySoundShader( target, "snd_acquire" );

			//************************************************************************************************
			//************************************************************************************************
			//************************************************************************************************
			if ( target->spawnArgs.GetBool( "player_persistent_rune" ) )
			{
				gameLocal.persistentLevelInfo.SetBool( target->spawnArgs.GetString( "persistent_rune_name" ), true );

				// Need to tell the HUD we picked up a rune
				if ( hud ) { hud->HandleNamedEvent( "NewRune" ); } // Changed to 'NewRune' - 6th June 2010 for Nuro

				// Update persistent information for the journal spells that the player can cast. - 22nd Jul 2010 for Nuro
				MagicUpdateJournalSpells();

			}
			//************************************************************************************************
			//************************************************************************************************
			//************************************************************************************************
			else if ( target->spawnArgs.GetBool( "player_money_gold" ) )
			{
				// Need to tell the HUD we got more money
				if ( hud ) { hud->HandleNamedEvent( "invPickup" ); }

				int moneyAmount = atoi( target->spawnArgs.GetString( "player_money_gold_amount", "0" ) );
				if ( moneyAmount > 0) {
					inventory.money += moneyAmount;
				}
			}
			//************************************************************************************************
			//************************************************************************************************
			//************************************************************************************************
			else if ( target->spawnArgs.GetBool( "player_inventory_weapon" ) )
			{
				// Need to tell the HUD we got a weapon
				if ( hud ) { hud->HandleNamedEvent( "invPickup" ); }
				GiveInventoryItem( &args );
			}
			//************************************************************************************************
			//************************************************************************************************
			//************************************************************************************************
			else
			{
				GiveInventoryItem( &args );
			}

			// Solarsplace 22nd Nov 2011 - Finally remove the model and anything bound to it such as food steam etc.
			target->PostEventMS( &EV_Remove, 0 );
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_readable_item" ) && !target->IsHidden() )
		{
			if ( target->spawnArgs.GetString( "target_gui" ) != "" )
			{

				// Solarsplace 13th June 2012 - Readable may add a journal / PDA
				idStr journalDefTarget;
				journalDefTarget = target->spawnArgs.GetString( "journal_pda_def", "" );

				if ( journalDefTarget != "" )
				{
					idDict journalArgs;
					const idDeclEntityDef *journalDef = gameLocal.FindEntityDef( journalDefTarget, false );
					journalArgs = journalDef->dict;
					const char *str = journalDef->dict.GetString( "pda_name" );
					player->GivePDA( str, &journalArgs );
				}

				readableSystem = uiManager->FindGui( target->spawnArgs.GetString( "target_gui" ), true, false, true );
				ToggleReadableSystem();
			}
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_usable_item_door" ) && !target->IsHidden() )
		{

			// Solarsplace -> BUT! - remember in this case target is a script created fucking trigger!
			// We need to find the door entity name or master door if teamed from the barstard triggers spawn args.
			idEntity * actualDoorMaster = gameLocal.FindEntity( target->spawnArgs.GetString( "target" ) );

			if ( actualDoorMaster->spawnArgs.GetBool( "lock_status", "0" ) == true )
			{
				requiredItemInvName = actualDoorMaster->spawnArgs.GetString( "requires_inv_item", "" );

				// The door is currently locked.
				if ( idStr::Icmp( requiredItemInvName, "" ) != 0 ) // Updated to be in line with the level change code below 20th Nov 2010
				{
					inventoryItem = FindInventoryItem( actualDoorMaster->spawnArgs.GetString( "requires_inv_item" ) );
					if ( inventoryItem )
					{
						PlaySpecificEntitySoundShader( actualDoorMaster, "snd_unlocked" ); 
						actualDoorMaster->spawnArgs.Set( "lock_status", "0" );
					}
					else
					{
						// Solarsplace -> Remember target the trigger here and NOT the real door entity.
						// Call this to make sure the door script plays the locked sound.
						target->ActivateTargets( player );

						// SP - 5th July 2013 - Display on hud what the locked object requires to make it easier for the player.
						idStr lockedMessage = common->GetLanguageDict()->GetString( "#str_general_00013" ); // "Object is locked and requires "
						lockedMessage += requiredItemInvName;
						ShowHudMessage( lockedMessage );

					}
				}
				// If no required item is set the just unlock it, other wise it can never be unlocked. Set the required item to something
				// that does not exist if you do not want the door to ever be opened.
				else
				{
					PlaySpecificEntitySoundShader( actualDoorMaster, "snd_unlocked" ); 
					actualDoorMaster->spawnArgs.Set( "lock_status", "0" );
				}
			}
			else
			{
				// Solarsplace -> Remember target the trigger here and NOT the real door entity.
				target->ActivateTargets( player );

				// Shopping
				if ( actualDoorMaster->spawnArgs.GetBool( "arx_shop" ) && !actualDoorMaster->IsHidden() )
				{
					if ( shoppingSystemOpen == false )
					{
						arxShopFunctions.LoadActiveShop( actualDoorMaster );
					
						// If any other GUI's are open then shut them
						if ( inventorySystemOpen ) { ToggleInventorySystem(); }
						if ( readableSystemOpen ) { ToggleReadableSystem(); }
						if ( journalSystemOpen ) { ToggleJournalSystem(); }
						if ( conversationSystemOpen ) { ToggleConversationSystem(); }
					}

					lastShopEntity = target; // Needed so we can close the door when we move away from, or stop looking at the door!

					//ToggleShoppingSystem();

				}
			}
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_shop" ) && !target->IsHidden() )
		{
			/* ALL TESTS SO FAR BASED ON SHOPS FROM CHESTS THAT USE func_rotatingdoor !!! */
			/* This needs to be implemented if we need it                                 */
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_level_change" ) && !target->IsHidden() )
		{
			// Solarsplace - Added and tested on 20th Nov 2010
			// Updated 25th Nov 2013

			idStr nextMap;
			idStr nextMapGameLocalFormat;
			idStr levelTransitionSpawnPoint;
			boolean changeLevelOK = false;

			// Check pre-requisites
			if ( !target->spawnArgs.GetString( "nextMap", "", nextMap ) ) {
				gameLocal.Printf( "idPlayer::GetEntityByViewRay: arx_level_change (%s) has no nextMap key. Can not change level.\n", target->name.c_str() );
				return;
			}

			// Does this level change require that the player has something in their inventory?
			requiredItemInvName = target->spawnArgs.GetString( "requires_inv_item", "" );
			if ( idStr::Icmp( requiredItemInvName, "" ) != 0 ) {
				inventoryItem = FindInventoryItem( target->spawnArgs.GetString( "requires_inv_item" ) );
				if ( inventoryItem ) {
					changeLevelOK = true;
				}
			}
			else {
				changeLevelOK = true;
			}

			if ( changeLevelOK ) {

				// Carry over timed attributes such as magic and damages.
				inventory.ClearDownTimedAttributes( false );

				// Active any targets of the change level entity.
				target->ActivateTargets( this );

				// Save level transition data just before we send a session command to change levels.
				SaveTransitionInfo();

				// SP - 12th June 1013 - Convert nextMap variable contents ( folder/mapname ) to gameLocal.GetMapName() format ( maps/folder/mapname.map )
				levelTransitionSpawnPoint = target->spawnArgs.GetString( "levelTransitionSpawnPoint", "" );
				nextMapGameLocalFormat = "maps/" + nextMap + ".map" + levelTransitionSpawnPoint;
				SetMapEntryPoint( nextMapGameLocalFormat );

				gameLocal.sessionCommand = "map " + nextMap;
			}
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_trigger_script" ) && !target->IsHidden() )
		{
			// Solarsplace 15th Oct 2011 - trigger scripts of entities in game

			target->Signal( SIG_TRIGGER );
			return;
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_use_script" ) && !target->IsHidden() )
		{
			const function_t *func;
			idThread *thread;

			func = target->scriptObject.GetFunction( target->spawnArgs.GetString( "arx_use_script_call", "use" ) );

			if ( func )
			{
				// create a thread and call the function
				thread = new idThread();
				thread->CallFunction( target, func, true );
				thread->Start();
			}
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "inv_pda" ) && !target->IsHidden() ) 
		{
			// Solarsplace 13th June 2012 - Pickup journal PDA's

			const char *str = target->spawnArgs.GetString( "pda_name" );
			idDict &journalArgs = target->spawnArgs;
			player->GivePDA( str, &journalArgs );

		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_searchable_corpse" ) )
		{
			bool searchOk = false;
			if ( target && target->IsType( idAFEntity_Gibbable::Type ) ) {
				if ( static_cast<idAFEntity_Gibbable *>( target )->searchable && !static_cast<idAFEntity_Gibbable *>( target )->IsGibbed() ) {
					if ( target->IsType( idAI::Type ) ) {
						if ( static_cast<idAI *>( target )->IsDead() ) {
							searchOk = true;
						}
					} else if ( static_cast<idAFEntity_Gibbable *>( target )->IsAtRest() ) {
						searchOk = true;
					}
				}
			}

			if ( searchOk ) {

				if ( GiveSearchItem( target ) )
				{
					// Script functions
					const function_t	*func;
					idThread			*thread;

					func = target->scriptObject.GetFunction( "SavePersistentState" );

					if ( func )
					{
						// create a thread and call the function
						thread = new idThread();
						thread->CallFunction( target, func, true );
						thread->Start();
					}
				}
			}
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else if ( target->spawnArgs.GetBool( "arx_searchable_container" ) )
		{
			if ( GiveSearchItem( target ) )
			{
				// Script functions
				const function_t	*func;
				idThread			*thread;

				func = target->scriptObject.GetFunction( "SavePersistentState" );

				if ( func )
				{
					// create a thread and call the function
					thread = new idThread();
					thread->CallFunction( target, func, true );
					thread->Start();
				}
			}
		}
		//************************************************************************************************
		//************************************************************************************************
		//************************************************************************************************
		else
		{
			// Play a sound to indicate nothing to pickup.
			StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}
	//************************************************************************************************
	//************************************************************************************************
	//************************************************************************************************
	else
	{
		// Play a sound to indicate nothing to pickup.
		StartSound( "snd_arx_pickup_fail", SND_CHANNEL_ANY, 0, false, NULL );
	}
}

bool idPlayer::GiveSearchItem( idEntity *target )
{
	// SP - Arx EOS

	bool gave = false;
	idStr defaultItem = "item_arx_goldcoins_stack"; // 10 gold default find item

	idStr fixedGiveItem = target->spawnArgs.GetString( "arx_searchable_find_fixed", "" ); // Get a specified single find item

	if ( fixedGiveItem.Length() ) {
		// Give the player a specified single item
		Event_GiveInventoryItem( fixedGiveItem );
		gave = true;
	} else if ( target->spawnArgs.GetBool( "player_money_gold", "0" ) ) {

		int fixedGold = target->spawnArgs.GetInt( "player_money_gold_amount", "0" );
		if ( fixedGold > 0 )
		{
			// Give the player a specified amount of gold
			Event_PlayerMoney( fixedGold );
			gave = true;
		} else {
			// Give the player a random amount of gold
			bool randomGold = target->spawnArgs.GetInt( "player_money_gold_amount_random", "0" );
			if ( randomGold ) {

				int randomGoldMin = target->spawnArgs.GetInt( "player_money_gold_amount_min", "1" );
				int randomGoldMax = target->spawnArgs.GetInt( "player_money_gold_amount_max", "20" );
				int randomGoldAmount = randomGoldMin + gameLocal.random.RandomInt( randomGoldMax - randomGoldMin );

				Event_PlayerMoney( randomGoldAmount );
				gave = true;
			}
		}
	} else {
		// Give the player a random item from the list
		const int MAX_CHOICE = 12;
		int randomItemNumber = gameLocal.random.RandomInt( MAX_CHOICE );

		idStr randomGiveItem = target->spawnArgs.GetString( va( "arx_searchable_find_%i", randomItemNumber ),  "" ); // Get a specified single find item
		if ( randomGiveItem.Length() ) {
			// Give the player the specified single item
			Event_GiveInventoryItem( randomGiveItem );
			gave = true;
		}
	}

	if ( gave ) {
		return true;
	} else {
		return false;
	}
}

void idPlayer::MagicUpdateJournalSpells( void )
{

	bool aam = gameLocal.persistentLevelInfo.GetBool("aam", "0");
	bool nhi = gameLocal.persistentLevelInfo.GetBool("nhi", "0");
	bool mega = gameLocal.persistentLevelInfo.GetBool("mega", "0");
	bool yok = gameLocal.persistentLevelInfo.GetBool("yok", "0");
	bool taar = gameLocal.persistentLevelInfo.GetBool("taar", "0");
	bool kaom = gameLocal.persistentLevelInfo.GetBool("kaom", "0");
	bool vitae = gameLocal.persistentLevelInfo.GetBool("vitae", "0");
	bool vista = gameLocal.persistentLevelInfo.GetBool("vista", "0");
	bool stregum = gameLocal.persistentLevelInfo.GetBool("stregum", "0");
	bool morte = gameLocal.persistentLevelInfo.GetBool("morte", "0");
	bool cosum = gameLocal.persistentLevelInfo.GetBool("cosum", "0");
	bool comunicatum = gameLocal.persistentLevelInfo.GetBool("comunicatum", "0");
	bool movis = gameLocal.persistentLevelInfo.GetBool("movis", "0");
	bool tempus = gameLocal.persistentLevelInfo.GetBool("tempus", "0");
	bool folgora = gameLocal.persistentLevelInfo.GetBool("folgora", "0");
	bool spacium = gameLocal.persistentLevelInfo.GetBool("spacium", "0");
	bool tera = gameLocal.persistentLevelInfo.GetBool("tera", "0");
	bool cetrius = gameLocal.persistentLevelInfo.GetBool("cetrius", "0");
	bool rhaa = gameLocal.persistentLevelInfo.GetBool("rhaa", "0");
	bool fridd = gameLocal.persistentLevelInfo.GetBool("fridd", "0");

	if ( mega && vista )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_vista", "1"); }

	if ( aam && taar )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_taar", "1"); }
	
	if ( aam && yok )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_yok", "1"); }

	if ( nhi && yok )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_yok", "1"); }

	if ( mega && spacium )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_spacium", "1"); }

	if ( mega && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_vitae", "1"); }

	if ( morte && cosum && vista )
	{ gameLocal.persistentLevelInfo.Set("spell_morte_cosum_vista", "1"); }

	if ( mega && kaom )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_kaom", "1"); }

	if ( rhaa && kaom )
	{ gameLocal.persistentLevelInfo.Set("spell_rhaa_kaom", "1"); }

	if ( rhaa && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_rhaa_vitae", "1"); }

	if ( mega && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_movis", "1"); }

	if ( nhi && stregum && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_stregum_vitae", "1"); }

	if ( aam && yok && taar )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_yok_taar", "1"); }

	if ( aam && cosum && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_cosum_vitae", "1"); }

	if ( aam && yok && fridd )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_yok_fridd", "1"); }

	if ( mega && stregum && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_stregum_vitae", "1"); }

	if ( nhi && spacium )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_spacium", "1"); }

	if ( yok && kaom )
	{ gameLocal.persistentLevelInfo.Set("spell_yok_kaom", "1"); }

	if ( spacium && comunicatum )
	{ gameLocal.persistentLevelInfo.Set("spell_spacium_comunicatum", "1"); }

	if ( rhaa && stregum && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_rhaa_stregum_vitae", "1"); }

	if ( fridd && kaom )
	{ gameLocal.persistentLevelInfo.Set("spell_fridd_kaom", "1"); }

	if ( aam && morte && cosum )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_morte_cosum", "1"); }

	if ( mega && spacium && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_spacium_movis", "1"); }

	if ( nhi && cetrius )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_cetrius", "1"); }

	if ( morte && kaom )
	{ gameLocal.persistentLevelInfo.Set("spell_morte_kaom", "1"); }

	if ( aam && taar && cetrius )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_taar_cetrius", "1"); }

	if ( aam && morte && vitae )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_morte_vitae", "1"); }

	if ( nhi && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_movis", "1"); }

	if ( aam && kaom && spacium )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_kaom_spacium", "1"); }

	if ( nhi && morte && cosum )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_morte_cosum", "1"); }

	if ( rhaa && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_rhaa_movis", "1"); }

	if ( vista && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_vista_movis", "1"); }

	if ( aam && yok && spacium )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_yok_spacium", "1"); }

	if ( aam && folgora && taar )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_folgora_taar", "1"); }

	if ( rhaa && vista )
	{ gameLocal.persistentLevelInfo.Set("spell_rhaa_vista", "1"); }

	if ( aam && fridd && spacium )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_fridd_spacium", "1"); }

	if ( nhi && vista )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_vista", "1"); }

	if ( stregum && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_stregum_movis", "1"); }

	if ( mega && aam && yok )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_aam_yok", "1"); }

	if ( mega && stregum && cosum )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_stregum_cosum", "1"); }

	if ( vitae && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_vitae_movis", "1"); }

	if ( aam && vitae && tera )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_vitae_tera", "1"); }

	if ( nhi && stregum && spacium )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_stregum_spacium", "1"); }

	if ( aam && mega && yok )
	{ gameLocal.persistentLevelInfo.Set("spell_aam_mega_yok", "1"); }

	if ( mega && nhi && movis )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_nhi_movis", "1"); }

	if ( mega && aam && taar && folgora )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_aam_taar_folgora", "1"); }

	if ( movis && comunicatum )
	{ gameLocal.persistentLevelInfo.Set("spell_movis_comunicatum", "1"); }

	if ( nhi && tempus )
	{ gameLocal.persistentLevelInfo.Set("spell_nhi_tempus", "1"); }

	if ( mega && aam && mega && yok )
	{ gameLocal.persistentLevelInfo.Set("spell_mega_aam_mega_yok", "1"); }

}

void idPlayer::PlaySpecificEntitySoundShader( idEntity *ent, const char *sndShaderName )
{	
	// Solarsplace 4th Mar 2010

	const idSoundShader *shader;
	const char *sound;

	if ( !ent->spawnArgs.GetString( sndShaderName, "", &sound ) ) {
		gameLocal.Printf( "idPlayer::PlaySpecificEntitySoundShader: Could not find sound shader %s in spawn args\n", sndShaderName );
		return;
	}

	if ( sound[0] == '\0' ) { // (2)
		gameLocal.Printf( "idPlayer::PlaySpecificEntitySoundShader: Sound was empty for %s\n", sndShaderName );
		return;
	}

	if ( !gameLocal.isNewFrame )
	{ return; } // don't play the sound, but don't report an error
	
	shader = declManager->FindSound( sound );
	StartSoundShader( shader, SND_CHANNEL_ITEM, 0, false, NULL );
}	

/*
*** END - Solarsplace 1st Mar 2010
*****************************************************************************************************
*****************************************************************************************************
*****************************************************************************************************
*/

bool idPlayer::HandleESC( void ) {
	if ( gameLocal.inCinematic ) {
		return SkipCinematic();
	}

	if ( objectiveSystemOpen ) {
		TogglePDA();
		return true;
	}

	// Solarsplace 11th April 2010 - Inventory related
	if ( inventorySystemOpen ) {
		ToggleInventorySystem();
		return true;
	}

	// Solarsplace 6th May 2010 - Journal related
	if ( journalSystemOpen ) {
		ToggleJournalSystem();
		return true;
	}

	// Solarsplace 22nd Nov 2011 - Readable related
	if ( readableSystemOpen ) {
		ToggleReadableSystem();
		return true;
	}

	// Solarsplace 22nd Nov 2011 - Readable related
	if ( shoppingSystemOpen ) {
		
		//ToggleShoppingSystem();

		if ( lastShopEntity ) {
			lastShopEntity->ActivateTargets( gameLocal.GetLocalPlayer() ); // If the shop is a door / chest then shut it!
		}
		
		return true;
	}

	return false;
}

bool idPlayer::SkipCinematic( void ) {
	StartSound( "snd_skipcinematic", SND_CHANNEL_ANY, 0, false, NULL );
	return gameLocal.SkipCinematic();
}

/*
==============
idPlayer::EvaluateControls
==============
*/
void idPlayer::EvaluateControls( void ) {
	// check for respawning
	if ( health <= 0 ) {
		if ( ( gameLocal.time > minRespawnTime ) && ( usercmd.buttons & BUTTON_ATTACK ) ) {
			forceRespawn = true;
		} else if ( gameLocal.time > maxRespawnTime ) {
			forceRespawn = true;
		}
	}

	// in MP, idMultiplayerGame decides spawns
	if ( forceRespawn && !gameLocal.isMultiplayer && !g_testDeath.GetBool() ) {
		// in single player, we let the session handle restarting the level or loading a game
		gameLocal.sessionCommand = "died";
	}

	if ( ( usercmd.flags & UCF_IMPULSE_SEQUENCE ) != ( oldFlags & UCF_IMPULSE_SEQUENCE ) ) {
		PerformImpulse( usercmd.impulse );
	}

	scoreBoardOpen = ( ( usercmd.buttons & BUTTON_SCORES ) != 0 || forceScoreBoard );

	oldFlags = usercmd.flags;

	AdjustSpeed();

	// update the viewangles
	UpdateViewAngles();
}

/*
==============
idPlayer::AdjustSpeed
==============
*/
void idPlayer::AdjustSpeed( void ) {
	float speed;
	float rate;

	if ( spectating ) {
		speed = pm_spectatespeed.GetFloat();
		bobFrac = 0.0f;
	} else if ( noclip ) {
		speed = pm_noclipspeed.GetFloat();
		bobFrac = 0.0f;
#ifdef _DT // levitate spell
	} else if ( levitate ) {
		speed = pm_levitatespeed.GetFloat();
		bobFrac = 0.0f;
#endif
	} else if ( !physicsObj.OnLadder() && ( usercmd.buttons & BUTTON_RUN ) && ( usercmd.forwardmove || usercmd.rightmove ) && ( usercmd.upmove >= 0 ) ) {
		if ( !gameLocal.isMultiplayer && !physicsObj.IsCrouching() && !PowerUpActive( ADRENALINE ) ) {
			stamina -= MS2SEC( gameLocal.msec );
		}
		if ( stamina < 0 ) {
			stamina = 0;
		}
		if ( ( !pm_stamina.GetFloat() ) || ( stamina > pm_staminathreshold.GetFloat() ) ) {
#ifdef _DT
			if ( !isRunning && !usercmd.rightmove ) {
				zoomFov.Init( gameLocal.time + 500, 300.0f, CalcFov( false ), DefaultFov() + 10.0f );
				isRunning = true;
			}
#endif
			bobFrac = 1.0f;
		} else if ( pm_staminathreshold.GetFloat() <= 0.0001f ) {
			bobFrac = 0.0f;
		} else {
#ifdef _DT
			if ( isRunning ) {
				zoomFov.Init( gameLocal.time, 300.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
				isRunning = false;
			}
#endif
			bobFrac = stamina / pm_staminathreshold.GetFloat();
		}
		speed = pm_walkspeed.GetFloat() * ( 1.0f - bobFrac ) + pm_runspeed.GetFloat() * bobFrac;
	} else {
#ifdef _DT
		if ( isRunning ) {
			zoomFov.Init( gameLocal.time, 200.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
			isRunning = false;
		}
#endif
		rate = pm_staminarate.GetFloat();
		
		// increase 25% faster when not moving
		if ( ( usercmd.forwardmove == 0 ) && ( usercmd.rightmove == 0 ) && ( !physicsObj.OnLadder() || ( usercmd.upmove == 0 ) ) ) {
			 rate *= 1.25f;
		}

		stamina += rate * MS2SEC( gameLocal.msec );
		if ( stamina > pm_stamina.GetFloat() ) {
			stamina = pm_stamina.GetFloat();
		}
		speed = pm_walkspeed.GetFloat();
		bobFrac = 0.0f;
	}

	// Solarsplace - Arx EOS - Clamp movement speed when wading or swimming.
	if (physicsObj.GetWaterLevel() >= WATERLEVEL_WAIST)
	{
		speed = idMath::ClampFloat(0, pm_walkspeed.GetFloat(), speed);
	}

	speed *= PowerUpModifier(SPEED);

	if ( influenceActive == INFLUENCE_LEVEL3 ) {
		speed *= 0.33f;
	}

	physicsObj.SetSpeed( speed, pm_crouchspeed.GetFloat() );
}

/*
==============
idPlayer::AdjustBodyAngles
==============
*/
void idPlayer::AdjustBodyAngles( void ) {
	idMat3	lookAxis;
	idMat3	legsAxis;
	bool	blend;
	float	diff;
	float	frac;
	float	upBlend;
	float	forwardBlend;
	float	downBlend;

	if ( health < 0 ) {
		return;
	}

	blend = true;

	if ( !physicsObj.HasGroundContacts() ) {
		idealLegsYaw = 0.0f;
		legsForward = true;
	} else if ( usercmd.forwardmove < 0 ) {
		idealLegsYaw = idMath::AngleNormalize180( idVec3( -usercmd.forwardmove, usercmd.rightmove, 0.0f ).ToYaw() );
		legsForward = false;
	} else if ( usercmd.forwardmove > 0 ) {
		idealLegsYaw = idMath::AngleNormalize180( idVec3( usercmd.forwardmove, -usercmd.rightmove, 0.0f ).ToYaw() );
		legsForward = true;
	} else if ( ( usercmd.rightmove != 0 ) && physicsObj.IsCrouching() ) {
		if ( !legsForward ) {
			idealLegsYaw = idMath::AngleNormalize180( idVec3( idMath::Abs( usercmd.rightmove ), usercmd.rightmove, 0.0f ).ToYaw() );
		} else {
			idealLegsYaw = idMath::AngleNormalize180( idVec3( idMath::Abs( usercmd.rightmove ), -usercmd.rightmove, 0.0f ).ToYaw() );
		}
	} else if ( usercmd.rightmove != 0 ) {
		idealLegsYaw = 0.0f;
		legsForward = true;
	} else {
		legsForward = true;
		diff = idMath::Fabs( idealLegsYaw - legsYaw );
		idealLegsYaw = idealLegsYaw - idMath::AngleNormalize180( viewAngles.yaw - oldViewYaw );
		if ( diff < 0.1f ) {
			legsYaw = idealLegsYaw;
			blend = false;
		}
	}

	if ( !physicsObj.IsCrouching() ) {
		legsForward = true;
	}

	oldViewYaw = viewAngles.yaw;

	AI_TURN_LEFT = false;
	AI_TURN_RIGHT = false;
	if ( idealLegsYaw < -45.0f ) {
		idealLegsYaw = 0;
		AI_TURN_RIGHT = true;
		blend = true;
	} else if ( idealLegsYaw > 45.0f ) {
		idealLegsYaw = 0;
		AI_TURN_LEFT = true;
		blend = true;
	}

	if ( blend ) {
		legsYaw = legsYaw * 0.9f + idealLegsYaw * 0.1f;
	}
	legsAxis = idAngles( 0.0f, legsYaw, 0.0f ).ToMat3();
	animator.SetJointAxis( hipJoint, JOINTMOD_WORLD, legsAxis );

	// calculate the blending between down, straight, and up
	frac = viewAngles.pitch / 90.0f;
	if ( frac > 0.0f ) {
		downBlend		= frac;
		forwardBlend	= 1.0f - frac;
		upBlend			= 0.0f;
	} else {
		downBlend		= 0.0f;
		forwardBlend	= 1.0f + frac;
		upBlend			= -frac;
	}

    animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 0, downBlend );
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 1, forwardBlend );
	animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 2, upBlend );

	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 0, downBlend );
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 1, forwardBlend );
	animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 2, upBlend );
}

/*
==============
idPlayer::InitAASLocation
==============
*/
void idPlayer::InitAASLocation( void ) {
	int		i;
	int		num;
	idVec3	size;
	idBounds bounds;
	idAAS	*aas;
	idVec3	origin;

	GetFloorPos( 64.0f, origin );

	num = gameLocal.NumAAS();
	aasLocation.SetGranularity( 1 );
	aasLocation.SetNum( num );	
	for( i = 0; i < aasLocation.Num(); i++ ) {
		aasLocation[ i ].areaNum = 0;
		aasLocation[ i ].pos = origin;
		aas = gameLocal.GetAAS( i );
		if ( aas && aas->GetSettings() ) {
			size = aas->GetSettings()->boundingBoxes[0][1];
			bounds[0] = -size;
			size.z = 32.0f;
			bounds[1] = size;

			aasLocation[ i ].areaNum = aas->PointReachableAreaNum( origin, bounds, AREA_REACHABLE_WALK );
		}
	}
}

/*
==============
idPlayer::SetAASLocation
==============
*/
void idPlayer::SetAASLocation( void ) {
	int		i;
	int		areaNum;
	idVec3	size;
	idBounds bounds;
	idAAS	*aas;
	idVec3	origin;

	if ( !GetFloorPos( 64.0f, origin ) ) {
		return;
	}
	
	for( i = 0; i < aasLocation.Num(); i++ ) {
		aas = gameLocal.GetAAS( i );
		if ( !aas ) {
			continue;
		}

		size = aas->GetSettings()->boundingBoxes[0][1];
		bounds[0] = -size;
		size.z = 32.0f;
		bounds[1] = size;

		areaNum = aas->PointReachableAreaNum( origin, bounds, AREA_REACHABLE_WALK );
		if ( areaNum ) {
			aasLocation[ i ].pos = origin;
			aasLocation[ i ].areaNum = areaNum;
		}
	}
}

/*
==============
idPlayer::GetAASLocation
==============
*/
void idPlayer::GetAASLocation( idAAS *aas, idVec3 &pos, int &areaNum ) const {
	int i;

	if ( aas != NULL ) {
		for( i = 0; i < aasLocation.Num(); i++ ) {
			if ( aas == gameLocal.GetAAS( i ) ) {
				areaNum = aasLocation[ i ].areaNum;
				pos = aasLocation[ i ].pos;
				return;
			}
		}
	}

	areaNum = 0;
	pos = physicsObj.GetOrigin();
}

/*
==============
idPlayer::Move
==============
*/
void idPlayer::Move( void ) {
	float newEyeOffset;
	idVec3 oldOrigin;
	idVec3 oldVelocity;
	idVec3 pushVelocity;

	// save old origin and velocity for crashlanding
	oldOrigin = physicsObj.GetOrigin();
	oldVelocity = physicsObj.GetLinearVelocity();
	pushVelocity = physicsObj.GetPushedLinearVelocity();

	// set physics variables
#ifdef _DT // levitate spell
	if ( levitate ) {
		physicsObj.SetMaxStepHeight( pm_levitateStepSize.GetFloat() );
	} else {
		physicsObj.SetMaxStepHeight( pm_stepsize.GetFloat() );
	}
#else
	physicsObj.SetMaxStepHeight( pm_stepsize.GetFloat() );
#endif
	physicsObj.SetMaxJumpHeight( pm_jumpheight.GetFloat() );

	if ( noclip ) {
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_NOCLIP );
#ifdef _DT // levitate spell
	} else if ( levitate ) {
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_LEVITATE );
#endif
	} else if ( spectating ) {
		physicsObj.SetContents( 0 );
		physicsObj.SetMovementType( PM_SPECTATOR );
	} else if ( health <= 0 ) {
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
		physicsObj.SetMovementType( PM_DEAD );
	} else if ( gameLocal.inCinematic || gameLocal.GetCamera() || privateCameraView || ( influenceActive == INFLUENCE_LEVEL2 ) ) {
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_FREEZE );
	} else {
		physicsObj.SetContents( CONTENTS_BODY );
		physicsObj.SetMovementType( PM_NORMAL );
	}

	if ( spectating ) {
		physicsObj.SetClipMask( MASK_DEADSOLID );
	} else if ( health <= 0 ) {
		physicsObj.SetClipMask( MASK_DEADSOLID );
	} else {
		physicsObj.SetClipMask( MASK_PLAYERSOLID );
	}

	physicsObj.SetDebugLevel( g_debugMove.GetBool() );
	physicsObj.SetPlayerInput( usercmd, viewAngles );

	// FIXME: physics gets disabled somehow
	BecomeActive( TH_PHYSICS );
	RunPhysics();

	// update our last valid AAS location for the AI
	SetAASLocation();

	if ( spectating ) {
		newEyeOffset = 0.0f;
	} else if ( health <= 0 ) {
		newEyeOffset = pm_deadviewheight.GetFloat();
	} else if ( physicsObj.IsCrouching() ) {
		newEyeOffset = pm_crouchviewheight.GetFloat();
	} else if ( GetBindMaster() && GetBindMaster()->IsType( idAFEntity_Vehicle::Type ) ) {
		newEyeOffset = 0.0f;
	} else {
		newEyeOffset = pm_normalviewheight.GetFloat();
	}

	if ( EyeHeight() != newEyeOffset ) {
		if ( spectating ) {
			SetEyeHeight( newEyeOffset );
		} else {
			// smooth out duck height changes
			SetEyeHeight( EyeHeight() * pm_crouchrate.GetFloat() + newEyeOffset * ( 1.0f - pm_crouchrate.GetFloat() ) );
		}
	}
#ifdef _DT // levitate spell
	if ( noclip || gameLocal.inCinematic || ( influenceActive == INFLUENCE_LEVEL2 ) || levitate ) {
#else
	if ( noclip || gameLocal.inCinematic || ( influenceActive == INFLUENCE_LEVEL2 ) ) {
#endif
		AI_CROUCH	= false;
		AI_ONGROUND	= ( influenceActive == INFLUENCE_LEVEL2 );
		AI_ONLADDER	= false;
		AI_JUMP		= false;
	} else {
		AI_CROUCH	= physicsObj.IsCrouching();
		AI_ONGROUND	= physicsObj.HasGroundContacts();
		AI_ONLADDER	= physicsObj.OnLadder();
		AI_JUMP		= physicsObj.HasJumped();

		// check if we're standing on top of a monster and give a push if we are
		idEntity *groundEnt = physicsObj.GetGroundEntity();
		if ( groundEnt && groundEnt->IsType( idAI::Type ) ) {
			idVec3 vel = physicsObj.GetLinearVelocity();
			if ( vel.ToVec2().LengthSqr() < 0.1f ) {
				vel.ToVec2() = physicsObj.GetOrigin().ToVec2() - groundEnt->GetPhysics()->GetAbsBounds().GetCenter().ToVec2();
				vel.ToVec2().NormalizeFast();
				vel.ToVec2() *= pm_walkspeed.GetFloat();
			} else {
				// give em a push in the direction they're going
				vel *= 1.1f;
			}
			physicsObj.SetLinearVelocity( vel );
		}
	}

	if ( AI_JUMP ) {
		// bounce the view weapon
 		loggedAccel_t	*acc = &loggedAccel[currentLoggedAccel&(NUM_LOGGED_ACCELS-1)];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[2] = 200;
		acc->dir[0] = acc->dir[1] = 0;
	}

	if ( AI_ONLADDER ) {
		int old_rung = oldOrigin.z / LADDER_RUNG_DISTANCE;
		int new_rung = physicsObj.GetOrigin().z / LADDER_RUNG_DISTANCE;

		if ( old_rung != new_rung ) {
			StartSound( "snd_stepladder", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}

	BobCycle( pushVelocity );
	CrashLand( oldOrigin, oldVelocity );
}

/*
==============
idPlayer::UpdateHud
==============
*/
void idPlayer::UpdateHud( void ) {
	idPlayer *aimed;

	if ( !hud ) {
		return;
	}

	if ( entityNumber != gameLocal.localClientNum ) {
		return;
	}

	int c = inventory.pickupItemNames.Num();
	if ( c > 0 ) {
		if ( gameLocal.time > inventory.nextItemPickup ) {
			if ( inventory.nextItemPickup && gameLocal.time - inventory.nextItemPickup > 2000 ) {
				inventory.nextItemNum = 1;
			}
			int i;
			for ( i = 0; i < 5, i < c; i++ ) {
				hud->SetStateString( va( "itemtext%i", inventory.nextItemNum ), inventory.pickupItemNames[0].name );
				hud->SetStateString( va( "itemicon%i", inventory.nextItemNum ), inventory.pickupItemNames[0].icon );
				hud->HandleNamedEvent( va( "itemPickup%i", inventory.nextItemNum++ ) );
				inventory.pickupItemNames.RemoveIndex( 0 );
				if (inventory.nextItemNum == 1 ) {
					inventory.onePickupTime = gameLocal.time;
				} else 	if ( inventory.nextItemNum > 5 ) {
					inventory.nextItemNum = 1;
					inventory.nextItemPickup = inventory.onePickupTime + 2000;
				} else {
					inventory.nextItemPickup = gameLocal.time + 400;
				}
			}
		}
	}

	if ( gameLocal.realClientTime == lastMPAimTime ) {
		if ( MPAim != -1 && gameLocal.gameType == GAME_TDM
			&& gameLocal.entities[ MPAim ] && gameLocal.entities[ MPAim ]->IsType( idPlayer::Type )
			&& static_cast< idPlayer * >( gameLocal.entities[ MPAim ] )->team == team ) {
				aimed = static_cast< idPlayer * >( gameLocal.entities[ MPAim ] );
				hud->SetStateString( "aim_text", gameLocal.userInfo[ MPAim ].GetString( "ui_name" ) );
				hud->SetStateFloat( "aim_color", aimed->colorBarIndex );
				hud->HandleNamedEvent( "aim_flash" );
				MPAimHighlight = true;
				MPAimFadeTime = 0;	// no fade till loosing focus
		} else if ( MPAimHighlight ) {
			hud->HandleNamedEvent( "aim_fade" );
			MPAimFadeTime = gameLocal.realClientTime;
			MPAimHighlight = false;
		}
	}
	if ( MPAimFadeTime ) {
		assert( !MPAimHighlight );
		if ( gameLocal.realClientTime - MPAimFadeTime > 2000 ) {
			MPAimFadeTime = 0;
		}
	}

	hud->SetStateInt( "g_showProjectilePct", g_showProjectilePct.GetInteger() );
	if ( numProjectilesFired ) {
		hud->SetStateString( "projectilepct", va( "Hit %% %.1f", ( (float) numProjectileHits / numProjectilesFired ) * 100 ) );
	} else {
		hud->SetStateString( "projectilepct", "Hit % 0.0" );
	}

	if ( isLagged && gameLocal.isMultiplayer && gameLocal.localClientNum == entityNumber ) {
		hud->SetStateString( "hudLag", "1" );
	} else {
		hud->SetStateString( "hudLag", "0" );
	}
}

/*
==============
idPlayer::UpdateDeathSkin
==============
*/
void idPlayer::UpdateDeathSkin( bool state_hitch ) {
	if ( !( gameLocal.isMultiplayer || g_testDeath.GetBool() ) ) {
		return;
	}
	if ( health <= 0 ) {
		if ( !doingDeathSkin ) {
			deathClearContentsTime = spawnArgs.GetInt( "deathSkinTime" );
			doingDeathSkin = true;
			renderEntity.noShadow = true;
			if ( state_hitch ) {
				renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f - 2.0f;
			} else {
				renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f;
			}
			UpdateVisuals();
		}

		// wait a bit before switching off the content
		if ( deathClearContentsTime && gameLocal.time > deathClearContentsTime ) {
			SetCombatContents( false );
			deathClearContentsTime = 0;
		}
	} else {
		renderEntity.noShadow = false;
		renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = 0.0f;
		UpdateVisuals();
		doingDeathSkin = false;
	}
}

/*
==============
idPlayer::StartFxOnBone
==============
*/
void idPlayer::StartFxOnBone( const char *fx, const char *bone ) {
	idVec3 offset;
	idMat3 axis;
	jointHandle_t jointHandle = GetAnimator()->GetJointHandle( bone );

	if ( jointHandle == INVALID_JOINT ) {
		gameLocal.Printf( "Cannot find bone %s\n", bone );
		return;
	}

	if ( GetAnimator()->GetJointTransform( jointHandle, gameLocal.time, offset, axis ) ) {
		offset = GetPhysics()->GetOrigin() + offset * GetPhysics()->GetAxis();
		axis = axis * GetPhysics()->GetAxis();
	}

	idEntityFx::StartFx( fx, &offset, &axis, this, true );
}

/*
==============
idPlayer::Think

Called every tic for each player
==============
*/
void idPlayer::Think( void ) {

	bool allowAttack = false;
	renderEntity_t *headRenderEnt;
	UpdatePlayerIcons();

	// latch button actions
	oldButtons = usercmd.buttons;

	// grab out usercmd
	usercmd_t oldCmd = usercmd;
	usercmd = gameLocal.usercmds[ entityNumber ];
	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;

	// Solarsplace - 4th Sep 2013 - http://bugs.thedarkmod.com/print_bug_page.php?bug_id=2424
	if ( ! (gameLocal.mainMenuExitHasDisabledAttack && ( usercmd.buttons & BUTTON_ATTACK )) )
	{
		allowAttack = true;
		gameLocal.mainMenuExitHasDisabledAttack = false;
	}

	if ( gameLocal.inCinematic && gameLocal.skipCinematic ) {
		return;
	}

	// clear the ik before we do anything else so the skeleton doesn't get updated twice
	walkIK.ClearJointMods();
	
	// if this is the very first frame of the map, set the delta view angles
	// based on the usercmd angles
	if ( !spawnAnglesSet && ( gameLocal.GameState() != GAMESTATE_STARTUP ) ) {
		spawnAnglesSet = true;
		SetViewAngles( spawnAngles );
		oldFlags = usercmd.flags;
	}

	if ( objectiveSystemOpen || gameLocal.inCinematic || influenceActive ) {
		if ( objectiveSystemOpen && AI_PAIN ) {
			TogglePDA();
		}

		// Solarsplace - Arx - 18th May 2012 - Allow movement to be consistent with other new Arx GUI's and like the original game.
		/*
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.upmove = 0;
		*/
	}

	// log movement changes for weapon bobbing effects
	if ( usercmd.forwardmove != oldCmd.forwardmove ) {
		loggedAccel_t	*acc = &loggedAccel[currentLoggedAccel&(NUM_LOGGED_ACCELS-1)];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[0] = usercmd.forwardmove - oldCmd.forwardmove;
		acc->dir[1] = acc->dir[2] = 0;
	}

	if ( usercmd.rightmove != oldCmd.rightmove ) {
		loggedAccel_t	*acc = &loggedAccel[currentLoggedAccel&(NUM_LOGGED_ACCELS-1)];
		currentLoggedAccel++;
		acc->time = gameLocal.time;
		acc->dir[1] = usercmd.rightmove - oldCmd.rightmove;
		acc->dir[0] = acc->dir[2] = 0;
	}

	// freelook centering
	if ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_MLOOK ) {
		centerView.Init( gameLocal.time, 200, viewAngles.pitch, 0 );
	}

	// zooming
#ifdef _DT
	if ( ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_ZOOM ) && !isRunning ) {
#else
	if ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_ZOOM ) {
#endif
		if ( ( usercmd.buttons & BUTTON_ZOOM ) && weapon.GetEntity() ) {
			zoomFov.Init( gameLocal.time, 200.0f, CalcFov( false ), weapon.GetEntity()->GetZoomFov() );
		} else {
			zoomFov.Init( gameLocal.time, 200.0f, zoomFov.GetCurrentValue( gameLocal.time ), DefaultFov() );
		}
	}

	// if we have an active gui, we will unrotate the view angles as
	// we turn the mouse movements into gui events
	idUserInterface *gui = ActiveGui();
	if ( gui && gui != focusUI ) {
		RouteGuiMouse( gui );
	}

	// set the push velocity on the weapon before running the physics
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->SetPushVelocity( physicsObj.GetPushedLinearVelocity() );
	}

	EvaluateControls();

	if ( !af.IsActive() ) {
		AdjustBodyAngles();
		CopyJointsFromBodyToHead();
	}

	Move();

	if ( !g_stopTime.GetBool() ) {

		if ( !noclip && !spectating && ( health > 0 ) && !IsHidden() ) {
			TouchTriggers();
		}

		// not done on clients for various reasons. don't do it on server and save the sound channel for other things
		if ( !gameLocal.isMultiplayer ) {
			SetCurrentHeartRate();
			float scale = g_damageScale.GetFloat();
			if ( g_useDynamicProtection.GetBool() && scale < 1.0f && gameLocal.time - lastDmgTime > 500 ) {
				if ( scale < 1.0f ) {
					scale += 0.05f;
				}
				if ( scale > 1.0f ) {
					scale = 1.0f;
				}
				g_damageScale.SetFloat( scale );
			}
		}

		// update GUIs, Items, and character interactions
		UpdateFocus();
		
		UpdateLocation();

		// update player script
		UpdateScript();

		// service animations
		if ( !spectating && !af.IsActive() && !gameLocal.inCinematic ) {
    		UpdateConditions();
			UpdateAnimState();
			CheckBlink();
		}

		// clear out our pain flag so we can tell if we recieve any damage between now and the next time we think
		AI_PAIN = false;
	}

	// calculate the exact bobbed view position, which is used to
	// position the view weapon, among other things
	CalculateFirstPersonView();

	// this may use firstPersonView, or a thirdPeroson / camera view
	CalculateRenderView();

	inventory.UpdateArmor();

	if ( spectating ) {
		UpdateSpectating();
	} else if ( health > 0  && allowAttack ) {
		UpdateWeapon();
	}

	// sikk---> Global Ambient Light
	if ( g_useAmbientLight.GetBool() )
		ToggleAmbientLight( true );
	else
		ToggleAmbientLight( false );
	// <---sikk

	UpdateAir();
	
	UpdateHud();

	UpdatePowerUps();

	UpdateDeathSkin( false );

	// **************************************************************************
	// **************************************************************************
	// **************************************************************************
	// Solarsplace - Start

	// GUI's
	UpdateJournalGUI();
	UpdateInventoryGUI();
	UpdateShoppingSystem();
	UpdateConversationSystem();

	// This code is ONLY used for pre-cast magic projectiles.
	if ( magicDoingPreCastSpellProjectile )
	{
		// OK - Listen up!
		// This feels a little hacky, however you must have a delay here in order for the weapon class
		// to switch reload the new weapon def, and then for us to forcable override the projecile dict of the def.
		if ( ( gameLocal.time - magicPreDelay ) >= 400)
		{
			//REMOVEMEx
			//gameLocal.Printf ("Entered magicDoingPreCastSpellProjectile\n" );
			
			// Flags & timings
			magicDoingPreCastSpellProjectile = false;
			magicAttackInProgress = true;
			magicAttackTime = gameLocal.time;

			// Solarsplace - 4th July 2010
			// It is critical to override the projectile def very late otherwise
			// there are many places in the code where the weapon def will be reloaded
			// thus deleting the custom magic projectile def.
			weapon.GetEntity()->magicChangeProjectileDef( magicLatestProjectileDefName );
			weapon.GetEntity()->BeginAttack();
			
		}
	}

	// This code will end the magic attack for real time magic & pre-cast magic too.
	if ( magicAttackInProgress )
	{
		if ( ( gameLocal.time - magicAttackTime ) >= 400 ) // This is the number of milliseconds that the magic weapon takes to complete its fire cycle.
		{
			// Flags
			magicAttackInProgress = false;
			magicDoingPreCastSpellProjectile = false;

			// Actions
			weapon.GetEntity()->EndAttack();
		}
	}

	// Solarsplace - 2nd July 2010 - Now magic mode is BUTTON_5 not an IMPULSE

	// Note: We cannot use the following original D3 hack because it always enters each condition twice!
	// if ( ( usercmd.buttons ^ oldCmd.buttons ) & BUTTON_5 ) {

	// Solarsplace - 21st Dec 2012 - Updated list of GUIS
	if ( objectiveSystemOpen || inventorySystemOpen || journalSystemOpen || readableSystemOpen || conversationSystemOpen || shoppingSystemOpen )
	{
		// Do nothing - other GUI's are open.
	}
	else
	{ 
		if ( usercmd.buttons & BUTTON_5 )
		{
			// The player IS holding down the magic mode button.
			magicModeActive = true;
			if ( magicModeActive != lastMagicModeActive )
			{
				lastMagicModeActive = magicModeActive;

				// SP - 4th Sep 2013 - Only allow magic mode if has snake compass
				bool hasSnakeCompass = false;
				if ( FindInventoryItemCount( "#str_item_00099" ) > 0 ) {
					hasSnakeCompass = true;
				} else {
					ShowHudMessage( "#str_general_00016" ); // "You cannot yet speak the words of power"
					//StartSoundShader( declManager->FindSound( "arx_magic_drawing_fizzle" ), SND_CHANNEL_ANY, 0, false, NULL );
				}

				if ( hasSnakeCompass ) {
					ToggleMagicMode();
				}
			}
		}
		else
		{
			// The player is NOT holding down the magic mode button.
			magicModeActive = false;
			if ( magicModeActive != lastMagicModeActive )
			{
				lastMagicModeActive = magicModeActive;
				ToggleMagicMode();
			}
		}
	}

	// 15th Mar 2013 - Process bonuses and player stats etc
	UpdateHeroStats();

	// Solarsplace - End
	// **************************************************************************
	// **************************************************************************
	// **************************************************************************

	if ( gameLocal.isMultiplayer ) {
		DrawPlayerIcons();
	}

	if ( head.GetEntity() ) {
		headRenderEnt = head.GetEntity()->GetRenderEntity();
	} else {
		headRenderEnt = NULL;
	}

	if ( headRenderEnt ) {
		if ( influenceSkin ) {
			headRenderEnt->customSkin = influenceSkin;
		} else {
			headRenderEnt->customSkin = NULL;
		}
	}

	if ( gameLocal.isMultiplayer || g_showPlayerShadow.GetBool() ) {
		renderEntity.suppressShadowInViewID	= 0;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = 0;
		}
	} else {
		renderEntity.suppressShadowInViewID	= entityNumber+1;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = entityNumber+1;
		}
	}
	// never cast shadows from our first-person muzzle flashes
	renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	if ( headRenderEnt ) {
		headRenderEnt->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	}

	if ( !g_stopTime.GetBool() ) {
		UpdateAnimation();

        Present();

		UpdateDamageEffects();

		LinkCombat();

		playerView.CalculateShake();
	}

	if ( !( thinkFlags & TH_THINK ) ) {
		gameLocal.Printf( "player %d not thinking?\n", entityNumber );
	}

	if ( g_showEnemies.GetBool() ) {
		idActor *ent;
		int num = 0;
		for( ent = enemyList.Next(); ent != NULL; ent = ent->enemyNode.Next() ) {
			gameLocal.Printf( "enemy (%d)'%s'\n", ent->entityNumber, ent->name.c_str() );
			gameRenderWorld->DebugBounds( colorRed, ent->GetPhysics()->GetBounds().Expand( 2 ), ent->GetPhysics()->GetOrigin() );
			num++;
		}
		gameLocal.Printf( "%d: enemies\n", num );
	}

	//neuro start    
	// determine if portal sky is in pvs
	gameLocal.portalSkyActive = gameLocal.pvs.CheckAreasForPortalSky( gameLocal.GetPlayerPVS(), GetPhysics()->GetOrigin() );
	//neuro end
}

/*
=================
idPlayer::RouteGuiMouse
=================
*/
void idPlayer::RouteGuiMouse( idUserInterface *gui ) {
	sysEvent_t ev;
	const char *command;

	if ( usercmd.mx != oldMouseX || usercmd.my != oldMouseY ) {
		ev = sys->GenerateMouseMoveEvent( usercmd.mx - oldMouseX, usercmd.my - oldMouseY );
		command = gui->HandleEvent( &ev, gameLocal.time );
		oldMouseX = usercmd.mx;
		oldMouseY = usercmd.my;
	}
}

/*
==================
idPlayer::LookAtKiller
==================
*/
void idPlayer::LookAtKiller( idEntity *inflictor, idEntity *attacker ) {
	idVec3 dir;
	
	if ( attacker && attacker != this ) {
		dir = attacker->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	} else if ( inflictor && inflictor != this ) {
		dir = inflictor->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin();
	} else {
		dir = viewAxis[ 0 ];
	}

	idAngles ang( 0, dir.ToYaw(), 0 );
	SetViewAngles( ang );
}

/*
==============
idPlayer::Kill
==============
*/
void idPlayer::Kill( bool delayRespawn, bool nodamage ) {
	if ( spectating ) {
		SpectateFreeFly( false );
	} else if ( health > 0 ) {
		godmode = false;
		if ( nodamage ) {
			ServerSpectate( true );
			forceRespawn = true;
		} else {
			Damage( this, this, vec3_origin, "damage_suicide", 1.0f, INVALID_JOINT );
			if ( delayRespawn ) {
				forceRespawn = false;
				int delay = spawnArgs.GetFloat( "respawn_delay" );
				minRespawnTime = gameLocal.time + SEC2MS( delay );
				maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
			}
		}
	}
}

/*
==================
idPlayer::Killed
==================
*/
void idPlayer::Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location ) {
	float delay;

	assert( !gameLocal.isClient );

	// stop taking knockback once dead
	fl.noknockback = true;
	if ( health < -999 ) {
		health = -999;
	}

	if ( AI_DEAD ) {
		AI_PAIN = true;
		return;
	}

	heartInfo.Init( 0, 0, 0, BASE_HEARTRATE );
	AdjustHeartRate( DEAD_HEARTRATE, 10.0f, 0.0f, true );

	if ( !g_testDeath.GetBool() ) {
		playerView.Fade( colorBlack, 12000 );
	}

	AI_DEAD = true;
	SetAnimState( ANIMCHANNEL_LEGS, "Legs_Death", 4 );
	SetAnimState( ANIMCHANNEL_TORSO, "Torso_Death", 4 );
	SetWaitState( "" );

	animator.ClearAllJoints();

	if ( StartRagdoll() ) {
		pm_modelView.SetInteger( 0 );
		minRespawnTime = gameLocal.time + RAGDOLL_DEATH_TIME;
		maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
	} else {
		// don't allow respawn until the death anim is done
		// g_forcerespawn may force spawning at some later time
		delay = spawnArgs.GetFloat( "respawn_delay" );
		minRespawnTime = gameLocal.time + SEC2MS( delay );
		maxRespawnTime = minRespawnTime + MAX_RESPAWN_TIME;
	}

	physicsObj.SetMovementType( PM_DEAD );
	StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
	StopSound( SND_CHANNEL_BODY2, false );

	fl.takedamage = true;		// can still be gibbed

	// get rid of weapon
	weapon.GetEntity()->OwnerDied();

	// drop the weapon as an item
	DropWeapon( true );

	if ( !g_testDeath.GetBool() ) {
		LookAtKiller( inflictor, attacker );
	}

	if ( gameLocal.isMultiplayer || g_testDeath.GetBool() ) {
		idPlayer *killer = NULL;
		// no gibbing in MP. Event_Gib will early out in MP
		if ( attacker->IsType( idPlayer::Type ) ) {
			killer = static_cast<idPlayer*>(attacker);
			if ( health < -20 || killer->PowerUpActive( BERSERK ) ) {
				gibDeath = true;
				gibsDir = dir;
				gibsLaunched = false;
			}
		}
		gameLocal.mpGame.PlayerDeath( this, killer, isTelefragged );
	} else {
		physicsObj.SetContents( CONTENTS_CORPSE | CONTENTS_MONSTERCLIP );
	}

	ClearPowerUps();

	UpdateVisuals();

	isChatting = false;
}

/*
=====================
idPlayer::GetAIAimTargets

Returns positions for the AI to aim at.
=====================
*/
void idPlayer::GetAIAimTargets( const idVec3 &lastSightPos, idVec3 &headPos, idVec3 &chestPos ) {
	idVec3 offset;
	idMat3 axis;
	idVec3 origin;
	
	origin = lastSightPos - physicsObj.GetOrigin();

	GetJointWorldTransform( chestJoint, gameLocal.time, offset, axis );
	headPos = offset + origin;

	GetJointWorldTransform( headJoint, gameLocal.time, offset, axis );
	chestPos = offset + origin;
}

/*
================
idPlayer::DamageFeedback

callback function for when another entity received damage from this entity.  damage can be adjusted and returned to the caller.
================
*/
void idPlayer::DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage ) {
	assert( !gameLocal.isClient );
	damage *= PowerUpModifier( BERSERK );
	if ( damage && ( victim != this ) && victim->IsType( idActor::Type ) ) {
		SetLastHitTime( gameLocal.time );
	}
}

/*
=================
idPlayer::CalcDamagePoints

Calculates how many health and armor points will be inflicted, but
doesn't actually do anything with them.  This is used to tell when an attack
would have killed the player, possibly allowing a "saving throw"
=================
*/
void idPlayer::CalcDamagePoints( idEntity *inflictor, idEntity *attacker, const idDict *damageDef,
							   const float damageScale, const int location, int *health, int *armor ) {
	int		damage;
	int		armorSave;

	damageDef->GetInt( "damage", "20", damage );
	damage = GetDamageForLocation( damage, location );

	idPlayer *player = attacker->IsType( idPlayer::Type ) ? static_cast<idPlayer*>(attacker) : NULL;
	if ( !gameLocal.isMultiplayer ) {
		if ( inflictor != gameLocal.world ) {
			switch ( g_skill.GetInteger() ) {
				case 0: 
					damage *= 0.80f;
					if ( damage < 1 ) {
						damage = 1;
					}
					break;
				case 2:
					damage *= 1.70f;
					break;
				case 3:
					damage *= 3.5f;
					break;
				default:
					break;
			}
		}
	}

	damage *= damageScale;

	// always give half damage if hurting self
	if ( attacker == this ) {
		if ( gameLocal.isMultiplayer ) {
			// only do this in mp so single player plasma and rocket splash is very dangerous in close quarters
			damage *= damageDef->GetFloat( "selfDamageScale", "0.5" );
		} else {
			damage *= damageDef->GetFloat( "selfDamageScale", "1" );
		}
	}

	// check for completely getting out of the damage
	if ( !damageDef->GetBool( "noGod" ) ) {
		// check for godmode
		if ( godmode ) {
			damage = 0;
		}
	}

	// inform the attacker that they hit someone
	attacker->DamageFeedback( this, inflictor, damage );

	// save some from armor
	if ( !damageDef->GetBool( "noArmor" ) ) {
		float armor_protection;

		armor_protection = ( gameLocal.isMultiplayer ) ? g_armorProtectionMP.GetFloat() : g_armorProtection.GetFloat();

		armorSave = ceil( damage * armor_protection );
		if ( armorSave >= inventory.armor ) {
			armorSave = inventory.armor;
		}

		if ( !damage ) {
			armorSave = 0;
		} else if ( armorSave >= damage ) {
			armorSave = damage - 1;
			damage = 1;
		} else {
			damage -= armorSave;
		}
	} else {
		armorSave = 0;
	}

	// check for team damage
	if ( gameLocal.gameType == GAME_TDM
		&& !gameLocal.serverInfo.GetBool( "si_teamDamage" )
		&& !damageDef->GetBool( "noTeam" )
		&& player
		&& player != this		// you get self damage no matter what
		&& player->team == team ) {
			damage = 0;
	}

	*health = damage;
	*armor = armorSave;
}

/*
============
Damage

this		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: this=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback in global space

damageDef	an idDict with all the options for damage effects

inflictor, attacker, dir, and point can be NULL for environmental effects
============
*/
void idPlayer::Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir,
					   const char *damageDefName, const float damageScale, const int location ) {
	idVec3		kick;
	int			damage;
	int			armorSave;
	int			knockback;
	idVec3		damage_from;
	idVec3		localDamageVector;	
	float		attackerPushScale;
	float		playerDamageScale; //ivan

	// damage is only processed on server
	if ( gameLocal.isClient ) {
		return;
	}
	
	if ( !fl.takedamage || noclip || spectating || gameLocal.inCinematic ) {
		return;
	}

	if ( !inflictor ) {
		inflictor = gameLocal.world;
	}
	if ( !attacker ) {
		attacker = gameLocal.world;
	}

	if ( attacker->IsType( idAI::Type ) ) {
		if ( PowerUpActive( BERSERK ) ) {
			return;
		}
		// don't take damage from monsters during influences
		if ( influenceActive != 0 ) {
			return;
		}
	}

	const idDeclEntityDef *damageDef = gameLocal.FindEntityDef( damageDefName, false );
	if ( !damageDef ) {
		gameLocal.Warning( "Unknown damageDef '%s'", damageDefName );
		return;
	}

	if ( damageDef->dict.GetBool( "ignore_player" ) ) {
		return;
	}

	//ivan start
	if(damageDef->dict.GetBool( "ignore_friends" )){
		if(team == attacker->spawnArgs.GetInt("team","0")){
			return;
		}
	}

	playerDamageScale = damageDef->dict.GetFloat( "playerDamageScale","1");
	//ivan end

	CalcDamagePoints( inflictor, attacker, &damageDef->dict, damageScale*playerDamageScale , location, &damage, &armorSave ); //ivan: added *playerDamageScale

	// determine knockback
	damageDef->dict.GetInt( "knockback", "20", knockback );

	if ( knockback != 0 && !fl.noknockback ) {
		if ( attacker == this ) {
			damageDef->dict.GetFloat( "attackerPushScale", "0", attackerPushScale );
		} else {
			attackerPushScale = 1.0f;
		}

		kick = dir;
		kick.Normalize();
		kick *= g_knockback.GetFloat() * knockback * attackerPushScale / 200.0f;
		physicsObj.SetLinearVelocity( physicsObj.GetLinearVelocity() + kick );

		// set the timer so that the player can't cancel out the movement immediately
		physicsObj.SetKnockBack( idMath::ClampInt( 50, 200, knockback * 2 ) );
	}

	// give feedback on the player view and audibly when armor is helping
	if ( armorSave ) {
		inventory.armor -= armorSave;

		if ( gameLocal.time > lastArmorPulse + 200 ) {
			StartSound( "snd_hitArmor", SND_CHANNEL_ITEM, 0, false, NULL );
		}
		lastArmorPulse = gameLocal.time;
	}
	
	// Solarsplace - 16th May 2010 - Poision related
	int poisonTime = damageDef->dict.GetInt( "poisoned", "0" );
	if ( poisonTime > 0 )
	{
		// SP - Updated 15th Mar 2013 - Player skills related
		if ( CalculateHeroChance( "add_poison" ) ) {
			inventory.arx_timer_player_poison += gameLocal.time + SEC2MS( poisonTime );
		}
	}

	if ( damageDef->dict.GetBool( "burn" ) ) {
		StartSound( "snd_burn", SND_CHANNEL_BODY3, 0, false, NULL );
	} else if ( damageDef->dict.GetBool( "no_air" ) ) {
		if ( !armorSave && health > 0 ) {
			StartSound( "snd_airGasp", SND_CHANNEL_ITEM, 0, false, NULL );
		}
	}

	if ( g_debugDamage.GetInteger() ) {
		gameLocal.Printf( "client:%i health:%i damage:%i armor:%i\n", 
			entityNumber, health, damage, armorSave );
	}

	// move the world direction vector to local coordinates
	damage_from = dir;
	damage_from.Normalize();
	
	viewAxis.ProjectVector( damage_from, localDamageVector );

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( health > 0 ) {
		playerView.DamageImpulse( localDamageVector, &damageDef->dict );
	}

	// do the damage
	if ( damage > 0 ) {

		if ( !gameLocal.isMultiplayer ) {
			float scale = g_damageScale.GetFloat();
			if ( g_useDynamicProtection.GetBool() && g_skill.GetInteger() < 2 ) {
				if ( gameLocal.time > lastDmgTime + 500 && scale > 0.25f ) {
					scale -= 0.05f;
					g_damageScale.SetFloat( scale );
				}
			}

			if ( scale > 0.0f ) {
				damage *= scale;
			}
		}

		if ( damage < 1 ) {
			damage = 1;
		}

		int oldHealth = health;
		health -= damage;

		if ( health <= 0 ) {

			if ( health < -999 ) {
				health = -999;
			}

			isTelefragged = damageDef->dict.GetBool( "telefrag" );

			lastDmgTime = gameLocal.time;
			Killed( inflictor, attacker, damage, dir, location );

		} else {
			// force a blink
			blink_time = 0;

			// let the anim script know we took damage
			AI_PAIN = Pain( inflictor, attacker, damage, dir, location );
			if ( !g_testDeath.GetBool() ) {
				lastDmgTime = gameLocal.time;
			}
		}
	} else {
		// don't accumulate impulses
		if ( af.IsLoaded() ) {
			// clear impacts
			af.Rest();

			// physics is turned off by calling af.Rest()
			BecomeActive( TH_PHYSICS );
		}
	}

	// Solarsplace - 14th Nov 2013 - Fire related
	if ( damageDef->dict.GetBool( "onFire" ) ) {
		int fireDamageDuration = (int)( ( (float)damage / 6.0f ) + 0.5f ); // Duration based on damage. Should never be less than 1 second.
		inventory.arx_timer_player_onfire = gameLocal.time + SEC2MS( fireDamageDuration );
	}

	lastDamageDef = damageDef->Index();
	lastDamageDir = damage_from;
	lastDamageLocation = location;
}

/*
===========
idPlayer::Teleport
============
*/
void idPlayer::Teleport( const idVec3 &origin, const idAngles &angles, idEntity *destination ) {
	idVec3 org;

	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->LowerWeapon();
	}

	SetOrigin( origin + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	if ( !gameLocal.isMultiplayer && GetFloorPos( 16.0f, org ) ) {
		SetOrigin( org );
	}

	// clear the ik heights so model doesn't appear in the wrong place
	walkIK.EnableAll();

	GetPhysics()->SetLinearVelocity( vec3_origin );

	SetViewAngles( angles );

	legsYaw = 0.0f;
	idealLegsYaw = 0.0f;
	oldViewYaw = viewAngles.yaw;

	if ( gameLocal.isMultiplayer ) {
		playerView.Flash( colorWhite, 140 );
	}

	UpdateVisuals();

	teleportEntity = destination;

	if ( !gameLocal.isClient && !noclip ) {
		if ( gameLocal.isMultiplayer ) {
			// kill anything at the new position or mark for kill depending on immediate or delayed teleport
			gameLocal.KillBox( this, destination != NULL );
		} else {
			// kill anything at the new position
			gameLocal.KillBox( this, true );
		}
	}
}

/*
====================
idPlayer::SetPrivateCameraView
====================
*/
void idPlayer::SetPrivateCameraView( idCamera *camView ) {
	privateCameraView = camView;
	if ( camView ) {
		StopFiring();
		Hide();
	} else {
		if ( !spectating ) {
			Show();
		}
	}
}

/*
====================
idPlayer::DefaultFov

Returns the base FOV
====================
*/
float idPlayer::DefaultFov( void ) const {
	float fov;

	fov = g_fov.GetFloat();
	if ( gameLocal.isMultiplayer ) {
		if ( fov < 90.0f ) {
			return 90.0f;
		} else if ( fov > 110.0f ) {
			return 110.0f;
		}
	}

	return fov;
}

/*
====================
idPlayer::CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
float idPlayer::CalcFov( bool honorZoom ) {
	float fov;

	if ( fxFov ) {
		return DefaultFov() + 10.0f + cos( ( gameLocal.time + 2000 ) * 0.01 ) * 10.0f;
	}

	if ( influenceFov ) {
		return influenceFov;
	}

	if ( zoomFov.IsDone( gameLocal.time ) ) {
#ifdef _DT
		if ( !isRunning && ( honorZoom && usercmd.buttons & BUTTON_ZOOM ) && weapon.GetEntity() ) {
			fov = weapon.GetEntity()->GetZoomFov();
		} else if ( isRunning ) {
			fov = DefaultFov() + 10.0f;
		} else {
			fov = DefaultFov();
		}
#else
		fov = ( honorZoom && usercmd.buttons & BUTTON_ZOOM ) && weapon.GetEntity() ? weapon.GetEntity()->GetZoomFov() : DefaultFov();
#endif
	} else {
		fov = zoomFov.GetCurrentValue( gameLocal.time );
	}

	// bound normal viewsize
	if ( fov < 1 ) {
		fov = 1;
	} else if ( fov > 179 ) {
		fov = 179;
	}

	return fov;
}

/*
==============
idPlayer::GunTurningOffset

generate a rotational offset for the gun based on the view angle
history in loggedViewAngles
==============
*/
idAngles idPlayer::GunTurningOffset( void ) {
	idAngles	a;

	a.Zero();

	if ( gameLocal.framenum < NUM_LOGGED_VIEW_ANGLES ) {
		return a;
	}

	idAngles current = loggedViewAngles[ gameLocal.framenum & (NUM_LOGGED_VIEW_ANGLES-1) ];

	idAngles	av, base;
	int weaponAngleOffsetAverages;
	float weaponAngleOffsetScale, weaponAngleOffsetMax;

	weapon.GetEntity()->GetWeaponAngleOffsets( &weaponAngleOffsetAverages, &weaponAngleOffsetScale, &weaponAngleOffsetMax );

	av = current;

	// calcualte this so the wrap arounds work properly
	for ( int j = 1 ; j < weaponAngleOffsetAverages ; j++ ) {
		idAngles a2 = loggedViewAngles[ ( gameLocal.framenum - j ) & (NUM_LOGGED_VIEW_ANGLES-1) ];

		idAngles delta = a2 - current;

		if ( delta[1] > 180 ) {
			delta[1] -= 360;
		} else if ( delta[1] < -180 ) {
			delta[1] += 360;
		}

		av += delta * ( 1.0f / weaponAngleOffsetAverages );
	}

	a = ( av - current ) * weaponAngleOffsetScale;

	for ( int i = 0 ; i < 3 ; i++ ) {
		if ( a[i] < -weaponAngleOffsetMax ) {
			a[i] = -weaponAngleOffsetMax;
		} else if ( a[i] > weaponAngleOffsetMax ) {
			a[i] = weaponAngleOffsetMax;
		}
	}

	return a;
}

/*
==============
idPlayer::GunAcceleratingOffset

generate a positional offset for the gun based on the movement
history in loggedAccelerations
==============
*/
idVec3	idPlayer::GunAcceleratingOffset( void ) {
	idVec3	ofs;

	float weaponOffsetTime, weaponOffsetScale;

	ofs.Zero();

	weapon.GetEntity()->GetWeaponTimeOffsets( &weaponOffsetTime, &weaponOffsetScale );

	int stop = currentLoggedAccel - NUM_LOGGED_ACCELS;
	if ( stop < 0 ) {
		stop = 0;
	}
	for ( int i = currentLoggedAccel-1 ; i > stop ; i-- ) {
		loggedAccel_t	*acc = &loggedAccel[i&(NUM_LOGGED_ACCELS-1)];

		float	f;
		float	t = gameLocal.time - acc->time;
		if ( t >= weaponOffsetTime ) {
			break;	// remainder are too old to care about
		}

		f = t / weaponOffsetTime;
		f = ( cos( f * 2.0f * idMath::PI ) - 1.0f ) * 0.5f;
		ofs += f * weaponOffsetScale * acc->dir;
	}

	return ofs;
}

/*
==============
idPlayer::CalculateViewWeaponPos

Calculate the bobbing position of the view weapon
==============
*/
void idPlayer::CalculateViewWeaponPos( idVec3 &origin, idMat3 &axis ) {
	float		scale;
	float		fracsin;
	idAngles	angles;
	int			delta;

	// CalculateRenderView must have been called first
	const idVec3 &viewOrigin = firstPersonViewOrigin;

#ifdef _DT	// head anim
	// Actually, if we use "firstPersonViewAxis", the weapon will follow the player view pos while animating the bone "head".
	// Since this is not good, we use "firstPersonViewWeaponAxis", which doesn't take care of the head orientation.
	const idMat3 &viewAxis = firstPersonViewWeaponAxis;
#else
	const idMat3 &viewAxis = firstPersonViewAxis;	
#endif

	// these cvars are just for hand tweaking before moving a value to the weapon def
	idVec3	gunpos( g_gun_x.GetFloat(), g_gun_y.GetFloat(), g_gun_z.GetFloat() );

	// as the player changes direction, the gun will take a small lag
	idVec3	gunOfs = GunAcceleratingOffset();
	origin = viewOrigin + ( gunpos + gunOfs ) * viewAxis;

	// on odd legs, invert some angles
	if ( bobCycle & 128 ) {
		scale = -xyspeed;
	} else {
		scale = xyspeed;
	}

	// gun angles from bobbing
	angles.roll		= scale * bobfracsin * 0.005f;
	angles.yaw		= scale * bobfracsin * 0.01f;
	angles.pitch	= xyspeed * bobfracsin * 0.005f;

	// gun angles from turning
	if ( gameLocal.isMultiplayer ) {
		idAngles offset = GunTurningOffset();
		offset *= g_mpWeaponAngleScale.GetFloat();
		angles += offset;
	} else {
		angles += GunTurningOffset();
	}

	idVec3 gravity = physicsObj.GetGravityNormal();

	// drop the weapon when landing after a jump / fall
	delta = gameLocal.time - landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		origin -= gravity * ( landChange*0.25f * delta / LAND_DEFLECT_TIME );
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		origin -= gravity * ( landChange*0.25f * (LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME );
	}

	// speed sensitive idle drift
	scale = xyspeed + 40.0f;
	fracsin = scale * sin( MS2SEC( gameLocal.time ) ) * 0.01f;
	angles.roll		+= fracsin;
	angles.yaw		+= fracsin;
	angles.pitch	+= fracsin;

	axis = angles.ToMat3() * viewAxis;
}

/*
===============
idPlayer::OffsetThirdPersonView
===============
*/
void idPlayer::OffsetThirdPersonView( float angle, float range, float height, bool clip ) {
	idVec3			view;
	idVec3			focusAngles;
	trace_t			trace;
	idVec3			focusPoint;
	float			focusDist;
	float			forwardScale, sideScale;
	idVec3			origin;
	idAngles		angles;
	idMat3			axis;
	idBounds		bounds;

	angles = viewAngles;
	GetViewPos( origin, axis );

	if ( angle ) {
		angles.pitch = 0.0f;
	}

	if ( angles.pitch > 45.0f ) {
		angles.pitch = 45.0f;		// don't go too far overhead
	}

	focusPoint = origin + angles.ToForward() * THIRD_PERSON_FOCUS_DISTANCE;
	focusPoint.z += height;
	view = origin;
	view.z += 8 + height;

	angles.pitch *= 0.5f;
	renderView->viewaxis = angles.ToMat3() * physicsObj.GetGravityAxis();

	idMath::SinCos( DEG2RAD( angle ), sideScale, forwardScale );
	view -= range * forwardScale * renderView->viewaxis[ 0 ];
	view += range * sideScale * renderView->viewaxis[ 1 ];

	if ( clip ) {
		// trace a ray from the origin to the viewpoint to make sure the view isn't
		// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything
		bounds = idBounds( idVec3( -4, -4, -4 ), idVec3( 4, 4, 4 ) );
		gameLocal.clip.TraceBounds( trace, origin, view, bounds, MASK_SOLID, this );
		if ( trace.fraction != 1.0f ) {
			view = trace.endpos;
			view.z += ( 1.0f - trace.fraction ) * 32.0f;

			// try another trace to this position, because a tunnel may have the ceiling
			// close enough that this is poking out
			gameLocal.clip.TraceBounds( trace, origin, view, bounds, MASK_SOLID, this );
			view = trace.endpos;
		}
	}

	// select pitch to look at focus point from vieword
	focusPoint -= view;
	focusDist = idMath::Sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1.0f ) {
		focusDist = 1.0f;	// should never happen
	}

	angles.pitch = - RAD2DEG( atan2( focusPoint.z, focusDist ) );
	angles.yaw -= angle;

	renderView->vieworg = view;
	renderView->viewaxis = angles.ToMat3() * physicsObj.GetGravityAxis();
	renderView->viewID = 0;
}

/*
===============
idPlayer::GetEyePosition
===============
*/
idVec3 idPlayer::GetEyePosition( void ) const {
	idVec3 org;
 
	// use the smoothed origin if spectating another player in multiplayer
	if ( gameLocal.isClient && entityNumber != gameLocal.localClientNum ) {
		org = smoothedOrigin;
	} else {
		org = GetPhysics()->GetOrigin();
	}
	return org + ( GetPhysics()->GetGravityNormal() * -eyeOffset.z );
}

/*
===============
idPlayer::GetViewPos
===============
*/
void idPlayer::GetViewPos( idVec3 &origin, idMat3 &axis ) const {
	idAngles angles;

#ifdef _DT	// head anim
	idAngles headAnimAngle;
#endif

	// if dead, fix the angle and don't add any kick
	if ( health <= 0 ) {
		angles.yaw = viewAngles.yaw;
		angles.roll = 40;
		angles.pitch = -15;
		axis = angles.ToMat3();
		origin = GetEyePosition();
	} else {
		origin = GetEyePosition() + viewBob;

#ifdef _DT	// head anim
		if ( weapon.GetEntity()->HasHeadJoint() ) {	// Check in the viewmodel if the bone "head" is present, if true take care of its orientation.
			headAnimAngle = weapon.GetEntity()->GetHeadAngle();
			angles = viewAngles + viewBobAngles + headAnimAngle + playerView.AngleOffset();
			//gameLocal.Printf( "calculate head anim \n" ); // debug
		} else {
			angles = viewAngles + viewBobAngles + playerView.AngleOffset();
			//gameLocal.Printf( "do not calculate head anim \n" ); // debug
		}		
#else
		angles = viewAngles + viewBobAngles + playerView.AngleOffset();
#endif

		axis = angles.ToMat3() * physicsObj.GetGravityAxis();

		// adjust the origin based on the camera nodal distance (eye distance from neck)
		origin += physicsObj.GetGravityNormal() * g_viewNodalZ.GetFloat();
		origin += axis[0] * g_viewNodalX.GetFloat() + axis[2] * g_viewNodalZ.GetFloat();
	}
}

#ifdef _DT	// head anim
/*
===============
idPlayer::GetViewWeaponAxis
===============
*/
void idPlayer::GetViewWeaponAxis( idMat3 &axis ) const {
	idAngles angles;
	
	// if dead, fix the angle and don't add any kick
	if ( health <= 0 ) {
		angles.yaw = viewAngles.yaw;
		angles.roll = 40;
		angles.pitch = -15;
		axis = angles.ToMat3();
		
	} else {
		
		angles = viewAngles + viewBobAngles + playerView.AngleOffset();

		axis = angles.ToMat3() * physicsObj.GetGravityAxis();

		
	}
}
#endif

/*
===============
idPlayer::CalculateFirstPersonView
===============
*/
void idPlayer::CalculateFirstPersonView( void ) {
	if ( ( pm_modelView.GetInteger() == 1 ) || ( ( pm_modelView.GetInteger() == 2 ) && ( health <= 0 ) ) ) {
		//	Displays the view from the point of view of the "camera" joint in the player model

		idMat3 axis;
		idVec3 origin;
		idAngles ang;

		ang = viewBobAngles + playerView.AngleOffset();
		ang.yaw += viewAxis[ 0 ].ToYaw();
		
		jointHandle_t joint = animator.GetJointHandle( "camera" );
		animator.GetJointTransform( joint, gameLocal.time, origin, axis );
		firstPersonViewOrigin = ( origin + modelOffset ) * ( viewAxis * physicsObj.GetGravityAxis() ) + physicsObj.GetOrigin() + viewBob;
		firstPersonViewAxis = axis * ang.ToMat3() * physicsObj.GetGravityAxis();
	} else {
		// offset for local bobbing and kicks
		GetViewPos( firstPersonViewOrigin, firstPersonViewAxis );
#if 0
		// shakefrom sound stuff only happens in first person
		firstPersonViewAxis = firstPersonViewAxis * playerView.ShakeAxis();
#endif
	}

#ifdef _DT	// head anim
	GetViewWeaponAxis( firstPersonViewWeaponAxis );
#endif
}

/*
==================
idPlayer::GetRenderView

Returns the renderView that was calculated for this tic
==================
*/
renderView_t *idPlayer::GetRenderView( void ) {
	return renderView;
}

/*
==================
idPlayer::CalculateRenderView

create the renderView for the current tic
==================
*/
void idPlayer::CalculateRenderView( void ) {
	int i;
	float range;

	if ( !renderView ) {
		renderView = new renderView_t;
	}
	memset( renderView, 0, sizeof( *renderView ) );

	// copy global shader parms
	for( i = 0; i < MAX_GLOBAL_SHADER_PARMS; i++ ) {
		renderView->shaderParms[ i ] = gameLocal.globalShaderParms[ i ];
	}
	renderView->globalMaterial = gameLocal.GetGlobalMaterial();
	renderView->time = gameLocal.time;

	// calculate size of 3D view
	renderView->x = 0;
	renderView->y = 0;
	renderView->width = SCREEN_WIDTH;
	renderView->height = SCREEN_HEIGHT;
	renderView->viewID = 0;

	// check if we should be drawing from a camera's POV
	if ( !noclip && (gameLocal.GetCamera() || privateCameraView) ) {
		// get origin, axis, and fov
		if ( privateCameraView ) {
			privateCameraView->GetViewParms( renderView );
		} else {
			gameLocal.GetCamera()->GetViewParms( renderView );
		}
	} else {
		if ( g_stopTime.GetBool() ) {
			renderView->vieworg = firstPersonViewOrigin;
			renderView->viewaxis = firstPersonViewAxis;

			if ( !pm_thirdPerson.GetBool() ) {
				// set the viewID to the clientNum + 1, so we can suppress the right player bodies and
				// allow the right player view weapons
				renderView->viewID = entityNumber + 1;
			}
		} else if ( pm_thirdPerson.GetBool() ) {
			OffsetThirdPersonView( pm_thirdPersonAngle.GetFloat(), pm_thirdPersonRange.GetFloat(), pm_thirdPersonHeight.GetFloat(), pm_thirdPersonClip.GetBool() );
		} else if ( pm_thirdPersonDeath.GetBool() ) {
			range = gameLocal.time < minRespawnTime ? ( gameLocal.time + RAGDOLL_DEATH_TIME - minRespawnTime ) * ( 120.0f / RAGDOLL_DEATH_TIME ) : 120.0f;
			OffsetThirdPersonView( 0.0f, 20.0f + range, 0.0f, false );
		} else {
			renderView->vieworg = firstPersonViewOrigin;
			renderView->viewaxis = firstPersonViewAxis;

			// set the viewID to the clientNum + 1, so we can suppress the right player bodies and
			// allow the right player view weapons
			renderView->viewID = entityNumber + 1;
		}
		
		// field of view
		gameLocal.CalcFov( CalcFov( true ), renderView->fov_x, renderView->fov_y );
	}

	if ( renderView->fov_y == 0 ) {
		common->Error( "renderView->fov_y == 0" );
	}

	if ( g_showviewpos.GetBool() ) {
		gameLocal.Printf( "%s : %s\n", renderView->vieworg.ToString(), renderView->viewaxis.ToAngles().ToString() );
	}
}

/*
=============
idPlayer::AddAIKill
=============
*/
void idPlayer::AddAIKill( void ) {
	int max_souls;
	int ammo_souls;

	if ( ( weapon_soulcube < 0 ) || ( inventory.weapons & ( 1 << weapon_soulcube ) ) == 0 ) {
		return;
	}

	assert( hud );

	ammo_souls = idWeapon::GetAmmoNumForName( "ammo_souls" );
	max_souls = inventory.MaxAmmoForAmmoClass( this, "ammo_souls" );
	if ( inventory.ammo[ ammo_souls ] < max_souls ) {
		inventory.ammo[ ammo_souls ]++;
		if ( inventory.ammo[ ammo_souls ] >= max_souls ) {
			hud->HandleNamedEvent( "soulCubeReady" );
			StartSound( "snd_soulcube_ready", SND_CHANNEL_ANY, 0, false, NULL );
		}
	}
}

/*
=============
idPlayer::SetSoulCubeProjectile
=============
*/
void idPlayer::SetSoulCubeProjectile( idProjectile *projectile ) {
	soulCubeProjectile = projectile;
}

/*
=============
idPlayer::AddProjectilesFired
=============
*/
void idPlayer::AddProjectilesFired( int count ) {
	numProjectilesFired += count;
}

/*
=============
idPlayer::AddProjectileHites
=============
*/
void idPlayer::AddProjectileHits( int count ) {
	numProjectileHits += count;
}

/*
=============
idPlayer::SetLastHitTime
=============
*/
void idPlayer::SetLastHitTime( int time ) {
	idPlayer *aimed = NULL;

	if ( time && lastHitTime != time ) {
		lastHitToggle ^= 1;
	}
	lastHitTime = time;
	if ( !time ) {
		// level start and inits
		return;
	}
	if ( gameLocal.isMultiplayer && ( time - lastSndHitTime ) > 10 ) {
		lastSndHitTime = time;
		StartSound( "snd_hit_feedback", SND_CHANNEL_ANY, SSF_PRIVATE_SOUND, false, NULL );
	}
	if ( cursor ) {
		cursor->HandleNamedEvent( "hitTime" );
	}
	if ( hud ) {
		if ( MPAim != -1 ) {
			if ( gameLocal.entities[ MPAim ] && gameLocal.entities[ MPAim ]->IsType( idPlayer::Type ) ) {
				aimed = static_cast< idPlayer * >( gameLocal.entities[ MPAim ] );
			}
			assert( aimed );
			// full highlight, no fade till loosing aim
			hud->SetStateString( "aim_text", gameLocal.userInfo[ MPAim ].GetString( "ui_name" ) );
			if ( aimed ) {
				hud->SetStateFloat( "aim_color", aimed->colorBarIndex );
			}
			hud->HandleNamedEvent( "aim_flash" );
			MPAimHighlight = true;
			MPAimFadeTime = 0;
		} else if ( lastMPAim != -1 ) {
			if ( gameLocal.entities[ lastMPAim ] && gameLocal.entities[ lastMPAim ]->IsType( idPlayer::Type ) ) {
				aimed = static_cast< idPlayer * >( gameLocal.entities[ lastMPAim ] );
			}
			assert( aimed );
			// start fading right away
			hud->SetStateString( "aim_text", gameLocal.userInfo[ lastMPAim ].GetString( "ui_name" ) );
			if ( aimed ) {
				hud->SetStateFloat( "aim_color", aimed->colorBarIndex );
			}
			hud->HandleNamedEvent( "aim_flash" );
			hud->HandleNamedEvent( "aim_fade" );
			MPAimHighlight = false;
			MPAimFadeTime = gameLocal.realClientTime;
		}
	}
}

/*
=============
idPlayer::SetInfluenceLevel
=============
*/
void idPlayer::SetInfluenceLevel( int level ) {
	if ( level != influenceActive ) {
		if ( level ) {
			for ( idEntity *ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
				if ( ent->IsType( idProjectile::Type ) ) {
					// remove all projectiles
					ent->PostEventMS( &EV_Remove, 0 );
				}
			}
			if ( weaponEnabled && weapon.GetEntity() ) {
				weapon.GetEntity()->EnterCinematic();
			}
		} else {
			physicsObj.SetLinearVelocity( vec3_origin );
			if ( weaponEnabled && weapon.GetEntity() ) {
				weapon.GetEntity()->ExitCinematic();
			}
		}
		influenceActive = level;
	}
}

/*
=============
idPlayer::SetInfluenceView
=============
*/
void idPlayer::SetInfluenceView( const char *mtr, const char *skinname, float radius, idEntity *ent ) {
	influenceMaterial = NULL;
	influenceEntity = NULL;
	influenceSkin = NULL;
	if ( mtr && *mtr ) {
		influenceMaterial = declManager->FindMaterial( mtr );
	}
	if ( skinname && *skinname ) {
		influenceSkin = declManager->FindSkin( skinname );
		if ( head.GetEntity() ) {
			head.GetEntity()->GetRenderEntity()->shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		}
		UpdateVisuals();
	}
	influenceRadius = radius;
	if ( radius > 0.0f ) {
		influenceEntity = ent;
	}
}

/*
=============
idPlayer::SetInfluenceFov
=============
*/
void idPlayer::SetInfluenceFov( float fov ) {
	influenceFov = fov;
}

/*
================
idPlayer::OnLadder
================
*/
bool idPlayer::OnLadder( void ) const {
	return physicsObj.OnLadder();
}

/*
==================
idPlayer::Event_GetButtons
==================
*/
void idPlayer::Event_GetButtons( void ) {
	idThread::ReturnInt( usercmd.buttons );
}

/*
==================
idPlayer::Event_GetMove
==================
*/
void idPlayer::Event_GetMove( void ) {
	idVec3 move( usercmd.forwardmove, usercmd.rightmove, usercmd.upmove );
	idThread::ReturnVector( move );
}

/*
================
idPlayer::Event_GetViewAngles
================
*/
void idPlayer::Event_GetViewAngles( void ) {
	idThread::ReturnVector( idVec3( viewAngles[0], viewAngles[1], viewAngles[2] ) );
}

/*
==================
idPlayer::Event_StopFxFov
==================
*/
void idPlayer::Event_StopFxFov( void ) {
	fxFov = false;
}

/*
==================
idPlayer::StartFxFov 
==================
*/
void idPlayer::StartFxFov( float duration ) { 
	fxFov = true;
	PostEventSec( &EV_Player_StopFxFov, duration );
}

/*
==================
idPlayer::Event_EnableWeapon 
==================
*/
void idPlayer::Event_EnableWeapon( void ) {
	hiddenWeapon = gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
	weaponEnabled = true;
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->ExitCinematic();
	}
}

/*
==================
idPlayer::Event_DisableWeapon
==================
*/
void idPlayer::Event_DisableWeapon( void ) {
	hiddenWeapon = gameLocal.world->spawnArgs.GetBool( "no_Weapons" );
	weaponEnabled = false;
	if ( weapon.GetEntity() ) {
		weapon.GetEntity()->EnterCinematic();
	}
}

/*
==================
idPlayer::Event_GetCurrentWeapon
==================
*/
void idPlayer::Event_GetCurrentWeapon( void ) {
	const char *weapon;

	if ( currentWeapon >= 0 ) {
		weapon = spawnArgs.GetString( va( "def_weapon%d", currentWeapon ) );
		idThread::ReturnString( weapon );
	} else {
		idThread::ReturnString( "" );
	}
}

/*
==================
idPlayer::Event_GetPreviousWeapon
==================
*/
void idPlayer::Event_GetPreviousWeapon( void ) {
	const char *weapon;

	if ( previousWeapon >= 0 ) {
		int pw = ( gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) ? 0 : previousWeapon;
		weapon = spawnArgs.GetString( va( "def_weapon%d", pw) );
		idThread::ReturnString( weapon );
	} else {
		idThread::ReturnString( spawnArgs.GetString( "def_weapon0" ) );
	}
}

/*
==================
idPlayer::Event_SelectWeapon
==================
*/
void idPlayer::Event_SelectWeapon( const char *weaponName ) {
	int i;
	int weaponNum;

	if ( gameLocal.isClient ) {
		gameLocal.Warning( "Cannot switch weapons from script in multiplayer" );
		return;
	}

	if ( hiddenWeapon && gameLocal.world->spawnArgs.GetBool( "no_Weapons" ) ) {
		idealWeapon = weapon_fists;
		weapon.GetEntity()->HideWeapon();
		return;
	}

	weaponNum = -1;
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( inventory.weapons & ( 1 << i ) ) {
			const char *weap = spawnArgs.GetString( va( "def_weapon%d", i ) );
			if ( !idStr::Cmp( weap, weaponName ) ) {
				weaponNum = i;
				break;
			}
		}
	}

	if ( weaponNum < 0 ) {
		gameLocal.Warning( "%s is not carrying weapon '%s'", name.c_str(), weaponName );
		return;
	}

	hiddenWeapon = false;
	idealWeapon = weaponNum;

	UpdateHudWeapon();
}

/*
==================
idPlayer::Event_GetWeaponEntity
==================
*/
void idPlayer::Event_GetWeaponEntity( void ) {
	idThread::ReturnEntity( weapon.GetEntity() );
}

/*
==================
idPlayer::Event_OpenPDA
==================
*/
void idPlayer::Event_OpenPDA( void ) {
	if ( !gameLocal.isMultiplayer ) {
		TogglePDA();
	}
}

/*
==================
idPlayer::Event_InPDA
==================
*/
void idPlayer::Event_InPDA( void ) {
	idThread::ReturnInt( objectiveSystemOpen );
}

#ifdef _DT // levitate spell
/*
==================
idPlayer::Event_LevitateStart
==================
*/
void idPlayer::Event_LevitateStart( void ) {

	if ( !levitate ) {

		idVec3 levitateVelocity( 0, 0, pm_levitatePushVelocity.GetFloat() );
		idVec3 curent_vel = GetPhysics()->GetLinearVelocity();

		curent_vel += levitateVelocity;
		
		GetPhysics()->SetLinearVelocity( curent_vel );

		levitate = true;
	}
}

/*
==================
idPlayer::Event_LevitateStop
==================
*/
void idPlayer::Event_LevitateStop( void ) {
	gameLocal.GetLocalPlayer()->inventory.arx_timer_player_levitate = gameLocal.time;
	levitate = false;
}
#endif

/*
==================
idPlayer::TeleportDeath
==================
*/
void idPlayer::TeleportDeath( int killer ) {
	teleportKiller = killer;
}

/*
==================
idPlayer::Event_ExitTeleporter
==================
*/
void idPlayer::Event_ExitTeleporter( void ) {
	idEntity	*exitEnt;
	float		pushVel;

	// verify and setup
	exitEnt = teleportEntity.GetEntity();
	if ( !exitEnt ) {
		common->DPrintf( "Event_ExitTeleporter player %d while not being teleported\n", entityNumber );
		return;
	}

	pushVel = exitEnt->spawnArgs.GetFloat( "push", "300" );

	if ( gameLocal.isServer ) {
		ServerSendEvent( EVENT_EXIT_TELEPORTER, NULL, false, -1 );
	}

	SetPrivateCameraView( NULL );
	// setup origin and push according to the exit target
	SetOrigin( exitEnt->GetPhysics()->GetOrigin() + idVec3( 0, 0, CM_CLIP_EPSILON ) );
	SetViewAngles( exitEnt->GetPhysics()->GetAxis().ToAngles() );
	physicsObj.SetLinearVelocity( exitEnt->GetPhysics()->GetAxis()[ 0 ] * pushVel );
	physicsObj.ClearPushedVelocity();
	// teleport fx
	playerView.Flash( colorWhite, 120 );

	// clear the ik heights so model doesn't appear in the wrong place
	walkIK.EnableAll();

	UpdateVisuals();

	StartSound( "snd_teleport_exit", SND_CHANNEL_ANY, 0, false, NULL );

	if ( teleportKiller != -1 ) {
		// we got killed while being teleported
		Damage( gameLocal.entities[ teleportKiller ], gameLocal.entities[ teleportKiller ], vec3_origin, "damage_telefrag", 1.0f, INVALID_JOINT );
		teleportKiller = -1;
	} else {
		// kill anything that would have waited at teleport exit
		gameLocal.KillBox( this );
	}
	teleportEntity = NULL;
}

/*
================
idPlayer::ClientPredictionThink
================
*/
void idPlayer::ClientPredictionThink( void ) {
	renderEntity_t *headRenderEnt;

	oldFlags = usercmd.flags;
	oldButtons = usercmd.buttons;

	usercmd = gameLocal.usercmds[ entityNumber ];

	if ( entityNumber != gameLocal.localClientNum ) {
		// ignore attack button of other clients. that's no good for predictions
		usercmd.buttons &= ~BUTTON_ATTACK;
	}

	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;

	if ( objectiveSystemOpen ) {
		usercmd.forwardmove = 0;
		usercmd.rightmove = 0;
		usercmd.upmove = 0;
	}

	// clear the ik before we do anything else so the skeleton doesn't get updated twice
	walkIK.ClearJointMods();

	if ( gameLocal.isNewFrame ) {
		if ( ( usercmd.flags & UCF_IMPULSE_SEQUENCE ) != ( oldFlags & UCF_IMPULSE_SEQUENCE ) ) {
			PerformImpulse( usercmd.impulse );
		}
	}

	scoreBoardOpen = ( ( usercmd.buttons & BUTTON_SCORES ) != 0 || forceScoreBoard );

	AdjustSpeed();

	UpdateViewAngles();

	// update the smoothed view angles
	if ( gameLocal.framenum >= smoothedFrame && entityNumber != gameLocal.localClientNum ) {
		idAngles anglesDiff = viewAngles - smoothedAngles;
		anglesDiff.Normalize180();
		if ( idMath::Fabs( anglesDiff.yaw ) < 90.0f && idMath::Fabs( anglesDiff.pitch ) < 90.0f ) {
			// smoothen by pushing back to the previous angles
			viewAngles -= gameLocal.clientSmoothing * anglesDiff;
			viewAngles.Normalize180();
		}
		smoothedAngles = viewAngles;
	}
	smoothedOriginUpdated = false;

	if ( !af.IsActive() ) {
		AdjustBodyAngles();
	}

	if ( !isLagged ) {
		// don't allow client to move when lagged
		Move();
	} 

	// update GUIs, Items, and character interactions
	UpdateFocus();

	// service animations
	if ( !spectating && !af.IsActive() ) {
    	UpdateConditions();
		UpdateAnimState();
		CheckBlink();
	}

	// clear out our pain flag so we can tell if we recieve any damage between now and the next time we think
	AI_PAIN = false;

	// calculate the exact bobbed view position, which is used to
	// position the view weapon, among other things
	CalculateFirstPersonView();

	// this may use firstPersonView, or a thirdPerson / camera view
	CalculateRenderView();

	if ( !gameLocal.inCinematic && weapon.GetEntity() && ( health > 0 ) && !( gameLocal.isMultiplayer && spectating ) ) {
		UpdateWeapon();
	}

	UpdateHud();

	if ( gameLocal.isNewFrame ) {
		UpdatePowerUps();
	}

	UpdateDeathSkin( false );

	if ( head.GetEntity() ) {
		headRenderEnt = head.GetEntity()->GetRenderEntity();
	} else {
		headRenderEnt = NULL;
	}

	if ( headRenderEnt ) {
		if ( influenceSkin ) {
			headRenderEnt->customSkin = influenceSkin;
		} else {
			headRenderEnt->customSkin = NULL;
		}
	}

	if ( gameLocal.isMultiplayer || g_showPlayerShadow.GetBool() ) {
		renderEntity.suppressShadowInViewID	= 0;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = 0;
		}
	} else {
		renderEntity.suppressShadowInViewID	= entityNumber+1;
		if ( headRenderEnt ) {
			headRenderEnt->suppressShadowInViewID = entityNumber+1;
		}
	}
	// never cast shadows from our first-person muzzle flashes
	renderEntity.suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	if ( headRenderEnt ) {
		headRenderEnt->suppressShadowInLightID = LIGHTID_VIEW_MUZZLE_FLASH + entityNumber;
	}

	if ( !gameLocal.inCinematic ) {
		UpdateAnimation();
	}

	if ( gameLocal.isMultiplayer ) {
		DrawPlayerIcons();
	}

	Present();

	UpdateDamageEffects();

	LinkCombat();

	if ( gameLocal.isNewFrame && entityNumber == gameLocal.localClientNum ) {
		playerView.CalculateShake();
	}

	//neuro start
	// determine if portal sky is in pvs
	pvsHandle_t	clientPVS = gameLocal.pvs.SetupCurrentPVS( GetPVSAreas(), GetNumPVSAreas() );
	gameLocal.portalSkyActive = gameLocal.pvs.CheckAreasForPortalSky( clientPVS, GetPhysics()->GetOrigin() );
	gameLocal.pvs.FreeCurrentPVS( clientPVS );
}

/*
================
idPlayer::GetPhysicsToVisualTransform
================
*/
bool idPlayer::GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis ) {
	if ( af.IsActive() ) {
		af.GetPhysicsToVisualTransform( origin, axis );
		return true;
	}

	// smoothen the rendered origin and angles of other clients
	// smooth self origin if snapshots are telling us prediction is off
	if ( gameLocal.isClient && gameLocal.framenum >= smoothedFrame && ( entityNumber != gameLocal.localClientNum || selfSmooth ) ) {
		// render origin and axis
		idMat3 renderAxis = viewAxis * GetPhysics()->GetAxis();
		idVec3 renderOrigin = GetPhysics()->GetOrigin() + modelOffset * renderAxis;

		// update the smoothed origin
		if ( !smoothedOriginUpdated ) {
			idVec2 originDiff = renderOrigin.ToVec2() - smoothedOrigin.ToVec2();
			if ( originDiff.LengthSqr() < Square( 100.0f ) ) {
				// smoothen by pushing back to the previous position
				if ( selfSmooth ) {
					assert( entityNumber == gameLocal.localClientNum );
					renderOrigin.ToVec2() -= net_clientSelfSmoothing.GetFloat() * originDiff;
				} else {
					renderOrigin.ToVec2() -= gameLocal.clientSmoothing * originDiff;
				}
			}
			smoothedOrigin = renderOrigin;

			smoothedFrame = gameLocal.framenum;
			smoothedOriginUpdated = true;
		}

		axis = idAngles( 0.0f, smoothedAngles.yaw, 0.0f ).ToMat3();
		origin = ( smoothedOrigin - GetPhysics()->GetOrigin() ) * axis.Transpose();

	} else {

		axis = viewAxis;
		origin = modelOffset;
	}
	return true;
}

/*
================
idPlayer::GetPhysicsToSoundTransform
================
*/
bool idPlayer::GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis ) {
	idCamera *camera;

	if ( privateCameraView ) {
		camera = privateCameraView;
	} else {
		camera = gameLocal.GetCamera();
	}

	if ( camera ) {
		renderView_t view;

		memset( &view, 0, sizeof( view ) );
		camera->GetViewParms( &view );
		origin = view.vieworg;
		axis = view.viewaxis;
		return true;
	} else {
		return idActor::GetPhysicsToSoundTransform( origin, axis );
	}
}

/*
================
idPlayer::WriteToSnapshot
================
*/
void idPlayer::WriteToSnapshot( idBitMsgDelta &msg ) const {
	physicsObj.WriteToSnapshot( msg );
	WriteBindToSnapshot( msg );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[0] );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[1] );
	msg.WriteDeltaFloat( 0.0f, deltaViewAngles[2] );
	msg.WriteShort( health );
	msg.WriteBits( gameLocal.ServerRemapDecl( -1, DECL_ENTITYDEF, lastDamageDef ), gameLocal.entityDefBits );
	msg.WriteDir( lastDamageDir, 9 );
	msg.WriteShort( lastDamageLocation );
	msg.WriteBits( idealWeapon, idMath::BitsForInteger( MAX_WEAPONS ) );
	msg.WriteBits( inventory.weapons, MAX_WEAPONS );
	msg.WriteBits( weapon.GetSpawnId(), 32 );
	msg.WriteBits( spectator, idMath::BitsForInteger( MAX_CLIENTS ) );
	msg.WriteBits( lastHitToggle, 1 );
	msg.WriteBits( weaponGone, 1 );
	msg.WriteBits( isLagged, 1 );
	msg.WriteBits( isChatting, 1 );
}

/*
================
idPlayer::ReadFromSnapshot
================
*/
void idPlayer::ReadFromSnapshot( const idBitMsgDelta &msg ) {
	int		i, oldHealth, newIdealWeapon, weaponSpawnId;
	bool	newHitToggle, stateHitch;

	if ( snapshotSequence - lastSnapshotSequence > 1 ) {
		stateHitch = true;
	} else {
		stateHitch = false;
	}
	lastSnapshotSequence = snapshotSequence;

	oldHealth = health;

	physicsObj.ReadFromSnapshot( msg );
	ReadBindFromSnapshot( msg );
	deltaViewAngles[0] = msg.ReadDeltaFloat( 0.0f );
	deltaViewAngles[1] = msg.ReadDeltaFloat( 0.0f );
	deltaViewAngles[2] = msg.ReadDeltaFloat( 0.0f );
	health = msg.ReadShort();
	lastDamageDef = gameLocal.ClientRemapDecl( DECL_ENTITYDEF, msg.ReadBits( gameLocal.entityDefBits ) );
	lastDamageDir = msg.ReadDir( 9 );
	lastDamageLocation = msg.ReadShort();
	newIdealWeapon = msg.ReadBits( idMath::BitsForInteger( MAX_WEAPONS ) );
	inventory.weapons = msg.ReadBits( MAX_WEAPONS );
	weaponSpawnId = msg.ReadBits( 32 );
	spectator = msg.ReadBits( idMath::BitsForInteger( MAX_CLIENTS ) );
	newHitToggle = msg.ReadBits( 1 ) != 0;
	weaponGone = msg.ReadBits( 1 ) != 0;
	isLagged = msg.ReadBits( 1 ) != 0;
	isChatting = msg.ReadBits( 1 ) != 0;

	// no msg reading below this

	if ( weapon.SetSpawnId( weaponSpawnId ) ) {
		if ( weapon.GetEntity() ) {
			// maintain ownership locally
			weapon.GetEntity()->SetOwner( this );
		}
		currentWeapon = -1;
	}
	// if not a local client assume the client has all ammo types
	if ( entityNumber != gameLocal.localClientNum ) {
		for( i = 0; i < AMMO_NUMTYPES; i++ ) {
			inventory.ammo[ i ] = 999;
		}
	}

	if ( oldHealth > 0 && health <= 0 ) {
		if ( stateHitch ) {
			// so we just hide and don't show a death skin
			UpdateDeathSkin( true );
		}
		// die
		AI_DEAD = true;
		ClearPowerUps();
		SetAnimState( ANIMCHANNEL_LEGS, "Legs_Death", 4 );
		SetAnimState( ANIMCHANNEL_TORSO, "Torso_Death", 4 );
		SetWaitState( "" );
		animator.ClearAllJoints();
		if ( entityNumber == gameLocal.localClientNum ) {
			playerView.Fade( colorBlack, 12000 );
		}
		StartRagdoll();
		physicsObj.SetMovementType( PM_DEAD );
		if ( !stateHitch ) {
			StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
		}
		if ( weapon.GetEntity() ) {
			weapon.GetEntity()->OwnerDied();
		}
	} else if ( oldHealth <= 0 && health > 0 ) {
		// respawn
		Init();
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.EnableClip();
		SetCombatContents( true );
	} else if ( health < oldHealth && health > 0 ) {
		if ( stateHitch ) {
			lastDmgTime = gameLocal.time;
		} else {
			// damage feedback
			const idDeclEntityDef *def = static_cast<const idDeclEntityDef *>( declManager->DeclByIndex( DECL_ENTITYDEF, lastDamageDef, false ) );
			if ( def ) {
				playerView.DamageImpulse( lastDamageDir * viewAxis.Transpose(), &def->dict );
				AI_PAIN = Pain( NULL, NULL, oldHealth - health, lastDamageDir, lastDamageLocation );
				lastDmgTime = gameLocal.time;
			} else {
				common->Warning( "NET: no damage def for damage feedback '%d'\n", lastDamageDef );
			}
		}
	} else if ( health > oldHealth && PowerUpActive( MEGAHEALTH ) && !stateHitch ) {
		// just pulse, for any health raise
		healthPulse = true;
	}

	// If the player is alive, restore proper physics object
	if ( health > 0 && IsActiveAF() ) {
		StopRagdoll();
		SetPhysics( &physicsObj );
		physicsObj.EnableClip();
		SetCombatContents( true );
	}

	if ( idealWeapon != newIdealWeapon ) {
		if ( stateHitch ) {
			weaponCatchup = true;
		}
		idealWeapon = newIdealWeapon;
		UpdateHudWeapon();
	}

	if ( lastHitToggle != newHitToggle ) {
		SetLastHitTime( gameLocal.realClientTime );
	}

	if ( msg.HasChanged() ) {
		UpdateVisuals();
	}
}

/*
================
idPlayer::WritePlayerStateToSnapshot
================
*/
void idPlayer::WritePlayerStateToSnapshot( idBitMsgDelta &msg ) const {
	int i;

	msg.WriteByte( bobCycle );
	msg.WriteLong( stepUpTime );
	msg.WriteFloat( stepUpDelta );
	msg.WriteShort( inventory.weapons );
	msg.WriteByte( inventory.armor );

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		msg.WriteBits( inventory.ammo[i], ASYNC_PLAYER_INV_AMMO_BITS );
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		msg.WriteBits( inventory.clip[i], ASYNC_PLAYER_INV_CLIP_BITS );
	}
}

/*
================
idPlayer::ReadPlayerStateFromSnapshot
================
*/
void idPlayer::ReadPlayerStateFromSnapshot( const idBitMsgDelta &msg ) {
	int i, ammo;

	bobCycle = msg.ReadByte();
	stepUpTime = msg.ReadLong();
	stepUpDelta = msg.ReadFloat();
	inventory.weapons = msg.ReadShort();
	inventory.armor = msg.ReadByte();

	for( i = 0; i < AMMO_NUMTYPES; i++ ) {
		ammo = msg.ReadBits( ASYNC_PLAYER_INV_AMMO_BITS );
		if ( gameLocal.time >= inventory.ammoPredictTime ) {
			inventory.ammo[ i ] = ammo;
		}
	}
	for( i = 0; i < MAX_WEAPONS; i++ ) {
		inventory.clip[i] = msg.ReadBits( ASYNC_PLAYER_INV_CLIP_BITS );
	}
}

/*
================
idPlayer::ServerReceiveEvent
================
*/
bool idPlayer::ServerReceiveEvent( int event, int time, const idBitMsg &msg ) {

	if ( idEntity::ServerReceiveEvent( event, time, msg ) ) {
		return true;
	}

	// client->server events
	switch( event ) {
		case EVENT_IMPULSE: {
			PerformImpulse( msg.ReadBits( 6 ) );
			return true;
		}
		default: {
			return false;
		}
	}
}

/*
================
idPlayer::ClientReceiveEvent
================
*/
bool idPlayer::ClientReceiveEvent( int event, int time, const idBitMsg &msg ) {
	int powerup;
	bool start;

	switch ( event ) {
		case EVENT_EXIT_TELEPORTER:
			Event_ExitTeleporter();
			return true;
		case EVENT_ABORT_TELEPORTER:
			SetPrivateCameraView( NULL );
			return true;
		case EVENT_POWERUP: {
			powerup = msg.ReadShort();
			start = msg.ReadBits( 1 ) != 0;
			if ( start ) {
				GivePowerUp( powerup, 0 );
			} else {
				ClearPowerup( powerup );
			}	
			return true;
		}
		case EVENT_SPECTATE: {
			bool spectate = ( msg.ReadBits( 1 ) != 0 );
			Spectate( spectate );
			return true;
		}
		case EVENT_ADD_DAMAGE_EFFECT: {
			if ( spectating ) {
				// if we're spectating, ignore
				// happens if the event and the spectate change are written on the server during the same frame (fraglimit)
				return true;
			}
			return idActor::ClientReceiveEvent( event, time, msg );
		}
		default: {
			return idActor::ClientReceiveEvent( event, time, msg );
		}
	}
	return false;
}

/*
================
idPlayer::Hide
================
*/
void idPlayer::Hide( void ) {
	idWeapon *weap;

	idActor::Hide();
	weap = weapon.GetEntity();
	if ( weap ) {
		weap->HideWorldModel();
	}
}

/*
================
idPlayer::Show
================
*/
void idPlayer::Show( void ) {
	idWeapon *weap;
	
	idActor::Show();
	weap = weapon.GetEntity();
	if ( weap ) {
		weap->ShowWorldModel();
	}
}

/*
===============
idPlayer::StartAudioLog
===============
*/
void idPlayer::StartAudioLog( void ) {
	if ( hud ) {
		hud->HandleNamedEvent( "audioLogUp" );
	}
}

/*
===============
idPlayer::StopAudioLog
===============
*/
void idPlayer::StopAudioLog( void ) {
	if ( hud ) {
		hud->HandleNamedEvent( "audioLogDown" );
	}
}

/*
===============
idPlayer::ShowTip
===============
*/
void idPlayer::ShowTip( const char *title, const char *tip, bool autoHide ) {
	if ( tipUp ) {
		return;
	}
	hud->SetStateString( "tip", tip );
	hud->SetStateString( "tiptitle", title );
	hud->HandleNamedEvent( "tipWindowUp" ); 
	if ( autoHide ) {
		PostEventSec( &EV_Player_HideTip, 5.0f );
	}
	tipUp = true;
}

/*
===============
idPlayer::HideTip
===============
*/
void idPlayer::HideTip( void ) {
	hud->HandleNamedEvent( "tipWindowDown" ); 
	tipUp = false;
}

/*
===============
idPlayer::Event_HideTip
===============
*/
void idPlayer::Event_HideTip( void ) {
	HideTip();
}

/*
===============
idPlayer::ShowObjective
===============
*/
void idPlayer::ShowObjective( const char *obj ) {
	hud->HandleNamedEvent( obj );
	objectiveUp = true;
}

/*
===============
idPlayer::HideObjective
===============
*/
void idPlayer::HideObjective( void ) {
	hud->HandleNamedEvent( "closeObjective" );
	objectiveUp = false;
}

/*
===============
idPlayer::Event_StopAudioLog
===============
*/
void idPlayer::Event_StopAudioLog( void ) {
	StopAudioLog();
}

/*
===============
idPlayer::SetSpectateOrigin
===============
*/
void idPlayer::SetSpectateOrigin( void ) {
	idVec3 neworig;

	neworig = GetPhysics()->GetOrigin();
	neworig[ 2 ] += EyeHeight();
	neworig[ 2 ] += 25;
	SetOrigin( neworig );
}

/*
===============
idPlayer::RemoveWeapon
===============
*/
void idPlayer::RemoveWeapon( const char *weap ) {
	if ( weap && *weap ) {
		inventory.Drop( spawnArgs, spawnArgs.GetString( weap ), -1 );
	}
}

/*
===============
idPlayer::CanShowWeaponViewmodel
===============
*/
bool idPlayer::CanShowWeaponViewmodel( void ) const {
	return showWeaponViewModel;
}

/*
===============
idPlayer::SetLevelTrigger
===============
*/
void idPlayer::SetLevelTrigger( const char *levelName, const char *triggerName ) {
	if ( levelName && *levelName && triggerName && *triggerName ) {
		idLevelTriggerInfo lti;
		lti.levelName = levelName;
		lti.triggerName = triggerName;
		inventory.levelTriggers.Append( lti );
	}
}

/*
===============
idPlayer::Event_LevelTrigger
===============
*/
void idPlayer::Event_LevelTrigger( void ) {
	idStr mapName = gameLocal.GetMapName();
	mapName.StripPath();
	mapName.StripFileExtension();
	for ( int i = inventory.levelTriggers.Num() - 1; i >= 0; i-- ) {
		if ( idStr::Icmp( mapName, inventory.levelTriggers[i].levelName) == 0 ){
			idEntity *ent = gameLocal.FindEntity( inventory.levelTriggers[i].triggerName );
			if ( ent ) {
				ent->PostEventMS( &EV_Activate, 1, this );
			}
		}
	}
}

/*
===============
idPlayer::Event_Gibbed
===============
*/
void idPlayer::Event_Gibbed( void ) {
}

/*
==================
idPlayer::Event_GetIdealWeapon 
==================
*/
void idPlayer::Event_GetIdealWeapon( void ) {
	const char *weapon;

	if ( idealWeapon >= 0 ) {
		weapon = spawnArgs.GetString( va( "def_weapon%d", idealWeapon ) );
		idThread::ReturnString( weapon );
	} else {
		idThread::ReturnString( "" );
	}
}

/*
===============
idPlayer::UpdatePlayerIcons
===============
*/
void idPlayer::UpdatePlayerIcons( void ) {
	int time = networkSystem->ServerGetClientTimeSinceLastPacket( entityNumber );
	if ( time > cvarSystem->GetCVarInteger( "net_clientMaxPrediction" ) ) {
		isLagged = true;
	} else {
		isLagged = false;
	}
}

/*
===============
idPlayer::DrawPlayerIcons
===============
*/
void idPlayer::DrawPlayerIcons( void ) {
	if ( !NeedsIcon() ) {
		playerIcon.FreeIcon();
		return;
	}
	playerIcon.Draw( this, headJoint );
}

/*
===============
idPlayer::HidePlayerIcons
===============
*/
void idPlayer::HidePlayerIcons( void ) {
	playerIcon.FreeIcon();
}

/*
===============
idPlayer::NeedsIcon
==============
*/
bool idPlayer::NeedsIcon( void ) {
	// local clients don't render their own icons... they're only info for other clients
	return entityNumber != gameLocal.localClientNum && ( isLagged || isChatting );
}

/*
================
2nd Jan 2010 - Solarsplace
idPlayer::Event_InventoryContainsItem
================
*/
void idPlayer::Event_InventoryContainsItem( const char *itemName )
{
	idDict *item = FindInventoryItem(itemName);

	if ( item )
	{
		idThread::ReturnFloat( 1 );
	}
	else
	{
		idThread::ReturnFloat( 0 );
	} 
}

void idPlayer::Event_LevelTransitionSpawnPoint( const char *levelTransitionSpawnPoint )
{
	SaveTransitionInfo();
	SetMapEntryPoint( levelTransitionSpawnPoint );
}


void idPlayer::Event_HudMessage( const char *message )
{
	if ( hud )
	{
		if ( idStr::FindText( message, "#str_" ) == 0 )
		{
			hud->SetStateString( "arx_MainMessageText", common->GetLanguageDict()->GetString( message ) );
			gameLocal.Printf("%s\n", common->GetLanguageDict()->GetString( message ) );
		} else {
			hud->SetStateString( "arx_MainMessageText", message );
			gameLocal.Printf("%s\n", message);
		}
		hud->HandleNamedEvent( "arx_ShowMainMessage" );
	}
	
}

void idPlayer::Event_SetFloatHUDParm( const char *key, float value )
{
	if ( hud )
	{
		hud->SetStateFloat( key, value );
		hud->StateChanged( gameLocal.time );
	}
}

void idPlayer::Event_FindInventoryItemCount( const char *name )
{
	int result = FindInventoryItemCount( name );
	idThread::ReturnInt( result );
}


void idPlayer::ShowHudMessage( idStr message )
{
	Event_HudMessage( message.c_str() );
}

void idPlayer::Event_PlayerMoney( int amount )
{
	if ( amount > 0 )
	{
		inventory.money += amount;
		ShowHudMessage( common->GetLanguageDict()->GetString( "#str_general_00004" ) ); // "Your gold increased"
		StartSound( "snd_shop_success", SND_CHANNEL_ANY, 0, false, NULL );
		idThread::ReturnInt( 1 );
	}
	else
	{
		// Don't show hud message for gold decreased as the mesage will be item recieved from a shop or weapon repaired etc.

		if ( inventory.money + amount >= 0 )
		{
			inventory.money += amount;
			StartSound( "snd_shop_success", SND_CHANNEL_ANY, 0, false, NULL );
			idThread::ReturnInt( 1 );
		}
		else
		{
			idThread::ReturnInt( 0 );
		}
	}
}

void idPlayer::Event_OpenCloseShop( const char *newState )
{
	gameLocal.Printf( "Event_OpenCloseShop\n" ); //REMOVEME
	ToggleShoppingSystem();
}

void idPlayer::Event_GiveJournal( const char *name )
{
	if ( name && *name ) {
		GivePDA( name, NULL );
	}
}

void idPlayer::Event_GetMapName( void ) {

	return idThread::ReturnString( gameLocal.GetMapName() );
}

//ivan start
/*
===============
idPlayer::Event_ForceUpdateNpcStatus
==============
*/
void idPlayer::Event_ForceUpdateNpcStatus( void ) { //ff1.1
	if ( focusCharacter && hud ) {
		if(focusCharacter->spawnArgs.GetBool( "showStatus", "0" )){  
			hud->SetStateString( "npc", "Status:" );
			hud->SetStateString( "npc_action", focusCharacter->spawnArgs.GetString( "shownState", "Inactive" ) );
		}
	}
}

/*
===============
idPlayer::Event_SetCommonEnemy
==============
*/
void idPlayer::Event_SetCommonEnemy( idEntity *enemy ) { 
	if ( enemy && enemy->IsType( idActor::Type ) ) {
		friendsCommonEnemy = static_cast<idActor *>( enemy );
	}else{
		friendsCommonEnemy = NULL;
	}
}

/*
===============
idPlayer::Event_GetCommonEnemy
==============
*/
void idPlayer::Event_GetCommonEnemy( void ) { 
	idActor *ent = friendsCommonEnemy.GetEntity();
	if ( ent && ent->health <= 0 ) {
		friendsCommonEnemy = NULL;
		ent = NULL;

		//gameLocal.Printf("CommonEnemy Health <= 0!\n");
	}

	/*
	else if ( !ent ){ //test only
		gameLocal.Printf("CommonEnemy not valid!\n");
	}
	*/

	idThread::ReturnEntity( ent );
}

//ivan end


// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// ****************************************************************************************************************************
// Arx - End Of Sun


// ==================================================================
// ==================================================================
// Skill, stats etc...

/*
=================
idPlayer::CalculateHeroChance
=================
*/
bool idPlayer::CalculateHeroChance( idStr chanceDescription ) {

	// Use the format add_xxxx or remove_xxxx

	bool returnChance = false;

	// Chance of getting poisoned
	if ( strcmp( chanceDescription, "add_poison" ) == 0 ) {

		if ( godmode ) { return false; }

		//REMOVEME
		int poison_resistance_chance = 20; // 20%

		if ( gameLocal.random.RandomFloat() * 100 > poison_resistance_chance ) {
			returnChance = true;
		}

	}

	// Chance of getting burnt
	if ( strcmp( chanceDescription, "add_fire" ) == 0 ) {

		if ( godmode ) { return false; }

		//REMOVEME
		int fire_resistance_chance = 20; // 20%

		if ( gameLocal.random.RandomFloat() * 100 > fire_resistance_chance ) {
			returnChance = true;
		}

	}

	// Chance of getting cold damage
	if ( strcmp( chanceDescription, "add_cold" ) == 0 ) {

		if ( godmode ) { return false; }

		//REMOVEME
		int cold_resistance_chance = 20; // 20%

		if ( gameLocal.random.RandomFloat() * 100 > cold_resistance_chance ) {
			returnChance = true;
		}

	}

	

	return returnChance;
}

/*
=================
idPlayer::UpdateEquipedItems
=================
*/
void idPlayer::UpdateEquipedItems( void ) {

	// ****************************************************
	// ****************************************************
	idStr invUniqueName = "";
	int invItemIndex = 0;
	int itemHealth = 0;

	// ****************************************************
	// ****************************************************
	
	int items_arx_power_health = 0;
	int items_arx_power_mana = 0;
	int items_arx_power_armour = 0;

	// SP - Not implementing ATM too much work.
	/*
	int tmp_arx_attr_strength = 0;
	int tmp_arx_attr_mental = 0;
	int tmp_arx_attr_dexterity = 0;
	int tmp_arx_attr_constitution = 0;

	int tmp_arx_skill_casting = 0;
	int tmp_arx_skill_close_combat = 0;
	int tmp_arx_skill_defense = 0;
	int tmp_arx_skill_ethereal_link = 0;
	int tmp_arx_skill_intuition = 0;
	int tmp_arx_skill_intelligence = 0;
	int tmp_arx_skill_projectile = 0;
	int tmp_arx_skill_stealth = 0;
	int tmp_arx_skill_technical = 0;
	*/

	// ****************************************************
	// ****************************************************
	
	int total_arx_power_health = 0;
	int total_arx_power_mana = 0;
	int total_arx_power_armour = 0;

	// SP - Not implementing ATM too much work.
	/*
	int total_arx_attr_strength = 0;
	int total_arx_attr_mental = 0;
	int total_arx_attr_dexterity = 0;
	int total_arx_attr_constitution = 0;

	int total_arx_skill_casting = 0;
	int total_arx_skill_close_combat = 0;
	int total_arx_skill_defense = 0;
	int total_arx_skill_ethereal_link = 0;
	int total_arx_skill_intuition = 0;
	int total_arx_skill_intelligence = 0;
	int total_arx_skill_projectile = 0;
	int total_arx_skill_stealth = 0;
	int total_arx_skill_technical = 0;
	*/

	// *******************************
	// *******************************
	// *******************************
	// TODO - Link with skills

	// Get current player health statistics
	int tmp_current_player_health = this->health;
	int tmp_current_player_max_health = this->health_max;

	// Get current player mana statistics
	int ammo_mana_index = idWeapon::GetAmmoNumForName( "ammo_mana" );
	int tmp_current_player_mana = inventory.ammo[ ammo_mana_index ];
	int tmp_current_player_max_mana = inventory.MaxAmmoForAmmoClass( this, "ammo_mana" );

	// Get current player armour statistics
	int tmp_current_player_armour = this->inventory.armor;
	int tmp_current_player_max_armour = this->inventory.maxarmor;

	// ****************************************************
	// ****************************************************

	// ****************************************************
	// ****************************************************
	// Process negative attribute items
	// These items reduce the attributes of health, mana and aromour

	int x = 0;
	for ( x = 0; x < ARX_EQUIPED_ITEMS_MAX; x++ ) {

		invUniqueName = inventory.arx_equiped_items[ x ];

		if ( idStr::Icmp( "", invUniqueName ) ) // Returns 0 if the text is equal
		{
			invItemIndex = FindInventoryItemIndexUniqueName( invUniqueName.c_str() );

			if ( invItemIndex >= 0 ) {

				// If the item has run out of health then do not process it.
				inventory.items[invItemIndex]->GetInt( "inv_health", "0", itemHealth );

				if ( itemHealth > 0 )
				{
					// ****************************************************
					// Get item attributes into temp variables

					inventory.items[invItemIndex]->GetInt( "arx_attr_health", "0", items_arx_power_health );
					inventory.items[invItemIndex]->GetInt( "arx_attr_mana", "0", items_arx_power_mana );
					inventory.items[invItemIndex]->GetInt( "arx_attr_armour", "0", items_arx_power_armour );

					// Process health
					if ( items_arx_power_health < 0 ) {
						tmp_current_player_health += items_arx_power_health;
					}

					// Process mana
					if ( items_arx_power_mana < 0 ) {
						tmp_current_player_mana += items_arx_power_mana;
					}

					// Process armour
					if ( items_arx_power_armour < 0 ) {
						tmp_current_player_armour += items_arx_power_armour;
					}
				}
			}
		}
	}

	// ****************************************************
	// ****************************************************
	// Process positive attribute items
	// These items increase the attributes of health, mana and aromour

	int i = 0;
	for ( i = 0; i < ARX_EQUIPED_ITEMS_MAX; i++ ) {

		invUniqueName = inventory.arx_equiped_items[ i ];

		if ( idStr::Icmp( "", invUniqueName ) ) // Returns 0 if the text is equal
		{
			invItemIndex = FindInventoryItemIndexUniqueName( invUniqueName.c_str() );

			if ( invItemIndex >= 0 ) {

				// If the item has run out of health then do not process it.
				inventory.items[invItemIndex]->GetInt( "inv_health", "0", itemHealth );

				if ( itemHealth > 0 )
				{

					// ****************************************************
					// Get item attributes into temp variables

					inventory.items[invItemIndex]->GetInt( "arx_attr_health", "0", items_arx_power_health );
					inventory.items[invItemIndex]->GetInt( "arx_attr_mana", "0", items_arx_power_mana );
					inventory.items[invItemIndex]->GetInt( "arx_attr_armour", "0", items_arx_power_armour );

					// SP - Not implementing ATM too much work.]
					/*
					inventory.items[invItemIndex]->GetInt( "arx_attr_strength", "0",		tmp_arx_attr_strength );
					inventory.items[invItemIndex]->GetInt( "arx_attr_mental", "0",			tmp_arx_attr_mental );
					inventory.items[invItemIndex]->GetInt( "arx_attr_dexterity", "0",		tmp_arx_attr_dexterity );
					inventory.items[invItemIndex]->GetInt( "arx_attr_constitution", "0",	tmp_arx_attr_constitution );

					inventory.items[invItemIndex]->GetInt( "arx_skill_casting", "0",		tmp_arx_skill_casting );
					inventory.items[invItemIndex]->GetInt( "arx_skill_close_combat", "0",	tmp_arx_skill_close_combat );
					inventory.items[invItemIndex]->GetInt( "arx_skill_defense", "0",		tmp_arx_skill_defense );
					inventory.items[invItemIndex]->GetInt( "arx_skill_ethereal_link", "0",	tmp_arx_skill_ethereal_link );
					inventory.items[invItemIndex]->GetInt( "arx_skill_intuition", "0",		tmp_arx_skill_intuition );
					inventory.items[invItemIndex]->GetInt( "arx_skill_intelligence", "0",	tmp_arx_skill_intelligence );
					inventory.items[invItemIndex]->GetInt( "arx_skill_projectile", "0",		tmp_arx_skill_projectile );
					inventory.items[invItemIndex]->GetInt( "arx_skill_stealth", "0",		tmp_arx_skill_stealth );
					inventory.items[invItemIndex]->GetInt( "arx_skill_technical", "0",		tmp_arx_skill_technical );
					*/

					// *********************************************
					// Update running totals from temp variables

					/*
					total_arx_power_health += items_arx_power_health;
					total_arx_power_mana += items_arx_power_mana;
					total_arx_power_armour += items_arx_power_armour;
					*/

					// SP - Not implementing ATM too much work.
					/*
					total_arx_attr_strength += tmp_arx_attr_strength;
					total_arx_attr_mental += tmp_arx_attr_mental;
					total_arx_attr_dexterity += tmp_arx_attr_dexterity;
					total_arx_attr_constitution += tmp_arx_attr_constitution;

					total_arx_skill_casting += tmp_arx_skill_casting;
					total_arx_skill_close_combat += tmp_arx_skill_close_combat;
					total_arx_skill_defense += tmp_arx_skill_defense;
					total_arx_skill_ethereal_link += tmp_arx_skill_ethereal_link;
					total_arx_skill_intuition += tmp_arx_skill_intuition;
					total_arx_skill_intelligence += tmp_arx_skill_intelligence;
					total_arx_skill_projectile += tmp_arx_skill_projectile;
					total_arx_skill_stealth += tmp_arx_skill_stealth;
					total_arx_skill_technical += tmp_arx_skill_technical;
					*/

					// *************************************************
					// *************************************************
					// Process health

					int needed_Health = tmp_current_player_max_health - tmp_current_player_health;

					// If health needed and item gives health
					if ( needed_Health > 0 && items_arx_power_health > 0 ) {

						// Item can give more than needed
						if ( needed_Health < items_arx_power_health ) {
							tmp_current_player_health += needed_Health;
							inventory.items[invItemIndex]->SetInt( "inv_health", itemHealth - needed_Health );
						} else {
							tmp_current_player_health += items_arx_power_health;
							inventory.items[invItemIndex]->SetInt( "inv_health", itemHealth - items_arx_power_health );
						}
					}

					// *************************************************
					// *************************************************
					// Process mana
					int needed_Mana = tmp_current_player_max_mana - tmp_current_player_mana;

					// If mana needed and item gives mana
					if ( needed_Mana > 0 && items_arx_power_mana > 0 ) {

						// Item can give more than needed
						if ( needed_Mana < items_arx_power_mana ) {
							tmp_current_player_mana += needed_Mana;
							inventory.items[invItemIndex]->SetInt( "inv_health", itemHealth - needed_Mana );
						} else {
							tmp_current_player_mana += items_arx_power_mana;
							inventory.items[invItemIndex]->SetInt( "inv_health", itemHealth - items_arx_power_mana );
						}
					}

					// *************************************************
					// *************************************************
					// Process armour
					int needed_Armour = tmp_current_player_max_armour - tmp_current_player_armour;

					// If mana needed and item gives mana
					if ( needed_Armour > 0 && items_arx_power_armour > 0 ) {

						// Item can give more than needed
						if ( needed_Armour < items_arx_power_armour ) {
							tmp_current_player_armour += needed_Armour;
							inventory.items[invItemIndex]->SetInt( "inv_health", itemHealth - needed_Armour );
						} else {
							tmp_current_player_armour += items_arx_power_armour;
							inventory.items[invItemIndex]->SetInt( "inv_health", itemHealth - items_arx_power_armour );
						}
					}
				}
			}
		}
	}

	// *************************************************
	// *************************************************
	// Process health
	const int MAX_HEALTH_DAMAGE = 10;
	int healthResult = this->health - tmp_current_player_health;

	if ( healthResult > 0 ) {
		if ( healthResult > MAX_HEALTH_DAMAGE ) { healthResult = MAX_HEALTH_DAMAGE; }
		Damage( NULL, NULL, vec3_origin, va( "damage_arx_general_%i", healthResult ), 1.0f, INVALID_JOINT );
	} else {
		this->health = tmp_current_player_health;
	}

	// *************************************************
	// *************************************************
	// Process mana
	const int MAX_MANA_DAMAGE = 10;
	int manaResult = inventory.ammo[ ammo_mana_index ] - tmp_current_player_mana;

	if ( manaResult > 0 ) {
		if ( manaResult > MAX_MANA_DAMAGE ) { manaResult = MAX_MANA_DAMAGE; }
		inventory.ammo[ ammo_mana_index ] = inventory.ammo[ ammo_mana_index ] - manaResult;
	} else {
		inventory.ammo[ ammo_mana_index ] = tmp_current_player_mana;
	}

	// *************************************************
	// *************************************************
	// Process armour
	const int MAX_ARMOUR_DAMAGE = 10;
	int armourResult = this->inventory.armor - tmp_current_player_armour;

	if ( armourResult > 0 ) {
		if ( armourResult > MAX_ARMOUR_DAMAGE ) { armourResult = MAX_ARMOUR_DAMAGE; }
		this->inventory.armor = this->inventory.armor - armourResult;
	} else {
		this->inventory.armor = tmp_current_player_armour;
	}
}

/*
=================
idPlayer::UpdateHeroStats
=================
*/
void idPlayer::UpdateHeroStats( void ) {

	//TODO
	const int ARX_HERO_UPDATE_RATE = 2000;
	const int ARX_MANA_LEVITATE_RATE = 2;
	const int ARX_POISON_DAMAGE_BASE = 5;
	const int ARX_COLD_DAMAGE_BASE = 3;
	const int ARX_COLD_DAMAGE_START_RATE = 20; // Start to take damage after 20 seconds of cold exposure.
	
	int now = gameLocal.time;
	int ammo_mana = idWeapon::GetAmmoNumForName( "ammo_mana" );
	int max_mana = inventory.MaxAmmoForAmmoClass( this, "ammo_mana" );
	int currentManaLevel = inventory.ammo[ ammo_mana ];

	// *******************************************************************************
	// *******************************************************************************
	// Solarsplace - Arx End Of Sun - cold system - Thanks Sikkmod
	// We do this every frame to make the visual effects smooth.
	// However pain is only processed every 2 seconds.
	bool enableColdWorld = gameLocal.world->spawnArgs.GetBool( "arx_cold_world", "0" );
	if ( enableColdWorld ) {
	
		if ( gameLocal.time > inventory.arx_timer_player_warmth ) {

			if ( g_screenFrostTime.GetInteger() ) {
				int n = g_screenFrostTime.GetInteger() * 60;
				nScreenFrostAlpha++;
				nScreenFrostAlpha = ( nScreenFrostAlpha > n ) ? n : nScreenFrostAlpha;
			}
		} else {

			if ( g_screenFrostTime.GetInteger() ) {
				nScreenFrostAlpha -= 3; // SP was 2 set to 3 to warm up a little quicker
				nScreenFrostAlpha = ( nScreenFrostAlpha < 0 ) ? 0 : nScreenFrostAlpha;
			}
		}
	} else {
		nScreenFrostAlpha = 0;
	}
	// *******************************************************************************
	// *******************************************************************************

	if ( now >= inventory.arx_timer_player_stats_update ) {

		inventory.arx_timer_player_stats_update = now + ARX_HERO_UPDATE_RATE;

		UpdateEquipedItems();

		// *****************************
		// *****************************
		// Spells
		
		// Levitate
		if ( levitate ) {
			// Stop levitate if time or mana has run out
			if ( inventory.arx_timer_player_levitate <= now || currentManaLevel <= 0 ) {
				Event_LevitateStop();
			} else {
				// Use mana to maintain levitate
				inventory.ammo[ ammo_mana ] = inventory.ammo[ ammo_mana ] - ARX_MANA_LEVITATE_RATE; // TODO - Add skill factor
			}
		}

		// *****************************
		// *****************************
		// Damages
		if ( health > 0 ) {

			// Poison damage
			if ( inventory.arx_timer_player_poison >= gameLocal.time ) {
				if ( CalculateHeroChance( "add_poison" ) ) {
                    Damage( NULL, NULL, vec3_origin, va( "damage_arx_general_%i", ARX_POISON_DAMAGE_BASE ), 1.0f, INVALID_JOINT );
				}
			}

			// Fire damage
			if ( inventory.arx_timer_player_onfire >= gameLocal.time ) {
				if ( CalculateHeroChance( "add_fire" ) ) {
					Damage( NULL, NULL, vec3_origin, "damage_arx_fire_interval", 1.0f, INVALID_JOINT );
				}
			}
			
			// Frost damage
			if ( nScreenFrostAlpha > 0 ) {

				float n = (float)g_screenFrostTime.GetInteger() * 60.0f;
				if ( n > 0 ) {
					n = n * 0.5f; // Start doing cold damage after 50% if full frost time
				}

				if ( (float)nScreenFrostAlpha > n ) {
					if ( CalculateHeroChance( "add_cold" ) ) {
						Damage( NULL, NULL, vec3_origin, va( "damage_arx_general_%i", ARX_COLD_DAMAGE_BASE ), 1.0f, INVALID_JOINT );
					}
				}
			}
		}
	}

	

	/*

	int healthDecrementAmount = 5;
	int healthDecrementDelaySecs = 5;

	if ( playerPoisoned )
	{
		if ( health > 0 )
		{
			if ( gameLocal.time >= healthNextDecreaseTime )
			{
				health -= healthDecrementAmount;
				healthNextDecreaseTime = gameLocal.time + SEC2MS( healthDecrementAmount);
			}
		}
	}


	*/


	/*

	*** Mana

	int ammo_mana;
	int max_mana;

	ammo_mana = idWeapon::GetAmmoNumForName( "ammo_mana" );
	max_mana = inventory.MaxAmmoForAmmoClass( this, "ammo_mana" );

	if ( inventory.ammo[ ammo_mana ] < 100 )
	{
		inventory.ammo[ ammo_mana ] = inventory.ammo[ ammo_mana ] + manaIncrementAmount;
	}

	*/


}

float idPlayer::ArxSkillGetAlertDistance( float defaultDistance ) {

	float returnValue = 0.0f;
	int skillValue = ( ARX_SKILL_BASE_VALUE - inventory.arx_skill_stealth ) * 128;
	returnValue = defaultDistance - (float)skillValue;

	//REMOVEME
	//gameLocal.Printf( "ArxSkillGetAlertDistance = %f\n", returnValue );

	return returnValue;
}

/*
=================
idPlayer::GetRequiredXPForLevel
=================
*/
int	idPlayer::GetRequiredXPForLevel( int level ) {

	switch ( level )
	{
		case 0:
			return 0;
			break;
		case 1:
			return 2000;
			break;
		case 2:
			return 4000;
			break;
		case 3:
			return 6000;
			break;
		case 4:
			return 10000;
			break;
		case 5:
			return 16000;
			break;
		case 6:
			return 26000;
			break;
		case 7:
			return 42000;
			break;
		case 8:
			return 68000;
			break;
		case 9:
			return 110000;
			break;
		case 10:
			return 178000;
			break;
		case 11:
			return 300000;
			break;
		case 12:
			return 450000;
			break;
		case 13:
			return 600000;
			break;
		case 14:
			return 750000;
			break;
		default:
			return level * 60000;
	}

	return LONG_MAX;
}

/*
================
2nd Jan 2010 - Solarsplace
idPlayer::Event_InventoryContainsItem
================
*/
void idPlayer::Event_ModifyPlayerXPs( int XPs )
{
	ModifyPlayerXPs( XPs, true );
}

/*
=================
idPlayer::ModifyPlayerXPs
=================
*/
void idPlayer::ModifyPlayerXPs( int XPs, bool showMessage )
{
	if ( XPs <= 0 ) { return; }

	int levelUp = 0;

	inventory.arx_player_x_points += XPs;				

	for ( int i = 1; i < ARX_MAX_PLAYER_LEVELS; i++ )
	{
		if ( i > inventory.arx_player_level )
		{
			if ( ( inventory.arx_skill_points >= GetRequiredXPForLevel( i ) ) )
			{ levelUp = 1; }

			if ( levelUp )
			{ ArxPlayerLevelUp(); }
		}
	}

	if ( !levelUp ) {
		// Don't wipe out the 'level up' message. Only show this if we did not level up.
		if ( showMessage ) {
			ShowHudMessage( "#str_general_00009" );	// "You gained XPs"
		}
	}
}

/*
=================
idPlayer::ArxPlayerLevelUp
=================
*/
void idPlayer::ArxPlayerLevelUp( void ) {

	ShowHudMessage( "##str_general_00014" );	// "!!! You Level Up !!!"

	// Play a sound to level up. Cached in player.def
	StartSound( "snd_arx_level_up", SND_CHANNEL_ANY, 0, false, NULL );

	inventory.arx_player_level ++;

	inventory.arx_attribute_points = spawnArgs.GetInt( "arx_levelup_attribute_points", "1" );
	inventory.arx_skill_points = spawnArgs.GetInt( "arx_levelup_skill_points", "15" );

	// Give max health
	health = inventory.maxHealth;

	// Give max mana
	int ammo_mana = idWeapon::GetAmmoNumForName( "ammo_mana" );
	int max_mana = inventory.MaxAmmoForAmmoClass( this, "ammo_mana" );
	inventory.ammo[ ammo_mana ] = max_mana;

	// Compute player stats here

	/*
	player.Old_Skill_Stealth			=	player.Skill_Stealth;
	player.Old_Skill_Mecanism			=	player.Skill_Mecanism;
	player.Old_Skill_Intuition			=	player.Skill_Intuition;
	player.Old_Skill_Etheral_Link		=	player.Skill_Etheral_Link;
	player.Old_Skill_Object_Knowledge	=	player.Skill_Object_Knowledge;
	player.Old_Skill_Casting			=	player.Skill_Casting;
	player.Old_Skill_Projectile			=	player.Skill_Projectile;
	player.Old_Skill_Close_Combat		=	player.Skill_Close_Combat;
	player.Old_Skill_Defense			=	player.Skill_Defense;
	*/

}

/*
=================
idPlayer::ClearDownTimedAttributes
=================
*/
void idInventory::ClearDownTimedAttributes( bool clearDown ) {

	// Solarsplace - 25th Nov 2013

	const int RESET_TIME = -9999;

	if ( clearDown ) {

		// This code path is not used at the moment. Retained in case of future use.
		return;

		arx_timer_player_stats_update		= RESET_TIME;
		arx_timer_player_poison				= RESET_TIME;
		arx_timer_player_invisible			= RESET_TIME;
		arx_timer_player_onfire				= RESET_TIME;
		arx_timer_player_telekinesis		= RESET_TIME;
		arx_timer_player_levitate			= RESET_TIME;
		arx_timer_player_warmth				= RESET_TIME;

	} else {

		// On level change gameLocal.time resets to 0
		// Adjust timed attribues to carry over so that player
		// cannot cheat by changing levels back and forth.

		// Update time
		if ( arx_timer_player_stats_update > gameLocal.time ) {
			arx_timer_player_stats_update = arx_timer_player_stats_update - gameLocal.time;
		} else {
			arx_timer_player_stats_update = RESET_TIME;
		}

		// Poisoned
		if ( arx_timer_player_poison > gameLocal.time ) {
			arx_timer_player_poison = arx_timer_player_poison - gameLocal.time;
		} else {
			arx_timer_player_poison = RESET_TIME;
		}

		// Invisible
		if ( arx_timer_player_invisible > gameLocal.time ) {
			arx_timer_player_invisible = arx_timer_player_invisible - gameLocal.time;
		} else {
			arx_timer_player_invisible = RESET_TIME;
		}

		// Fire damage
		if ( arx_timer_player_onfire > gameLocal.time ) {
			arx_timer_player_onfire = arx_timer_player_onfire - gameLocal.time;
		} else {
			arx_timer_player_onfire = RESET_TIME;
		}

		// Telekinesis
		if ( arx_timer_player_telekinesis > gameLocal.time ) {
			arx_timer_player_telekinesis = arx_timer_player_telekinesis - gameLocal.time;
		} else {
			arx_timer_player_telekinesis = RESET_TIME;
		}

		// Levitate
		if ( arx_timer_player_levitate > gameLocal.time ) {
			arx_timer_player_levitate = arx_timer_player_levitate - gameLocal.time;
		} else {
			arx_timer_player_levitate = RESET_TIME;
		}
	}
}

/*
=================
idPlayer::GetQuestState
=================
*/
bool idPlayer::GetQuestState( idStr questObjectQuestName )
{
	idStr questId = ARX_QUEST_STATE + ARX_REC_SEP + questObjectQuestName;

	float returnValue = gameLocal.persistentLevelInfo.GetFloat( questId );

	if ( returnValue == ARX_QUEST_STATE_INITIAL ) {
		return false;
	}

	if ( returnValue == ARX_QUEST_STATE_COMPLETED ) {
		return true;
	}

	return false;
}

/*
=================
idPlayer::ShopGetBuyFromPlayerPrice
=================
*/
int idPlayer::ShopGetBuyFromPlayerPrice( float baseValue, float durabilityRatio, float shopRatio ) {
	return (int)baseValue * shopRatio * durabilityRatio; //TODO add skill bonus
}

/*
=================
idPlayer::ShopGetSellToPlayerPrice
=================
*/
int idPlayer::ShopGetSellToPlayerPrice( float baseValue, float durabilityRatio, float shopRatio ) {
	// Assumes all things shop sells are 100% durability
	return (int)baseValue * shopRatio; //TODO add skill bonus
}


void idPlayer::ArxTraceAIHealthHUD( void ) {

	// SP - Arx End Of Sun

	// Update the hud with AI health information. Distance dependent on player skill

	if ( !hud ) {
		return;
	}

	trace_t trace;
	idVec3 start;
	idVec3 end;

	bool noDamage = false;
	int healthCurrent = 0;
	int healthMax = 0;
	float healthUnit = 0.0f;
	int team = 0;
	idStr strHealth;

	// Start the traceline at our eyes
	start = GetEyePosition( );

	// End the traceline 768 units ahead in the direction we're viewing
	end = start + viewAngles.ToForward( ) * 768.0f;

	gameLocal.clip.TracePoint( trace, start, end, MASK_MONSTERSOLID, this );

	// trace.fraction is the fraction of the traceline that was travelled
	// if trace.fraction is less than one then we hit something
	if( ( trace.fraction < 1.0f ) && ( trace.c.entityNum != ENTITYNUM_NONE ) )
	{
		idEntity *ent = gameLocal.GetTraceEntity( trace );

		if ( ent->IsType( idActor::Type ) ) {

			noDamage = ent->spawnArgs.GetBool( "noDamage", "0" );
			team = ent->spawnArgs.GetInt( "team", "-1" );

			if ( noDamage ) {
				healthCurrent = 100;
				healthMax = 100;
				healthUnit = 1.0f;
			} else {
				healthCurrent = ent->health;
				healthMax = ent->health_max;
				healthUnit = (float)healthCurrent / (float)healthMax ;
			}

			sprintf( strHealth, "%d / %d", healthCurrent, healthMax );

			hud->HandleNamedEvent( "AIHealth" );
			hud->SetStateString( "arx_ai_health_string", strHealth.c_str() );
			hud->SetStateFloat( "arx_ai_health_unit", healthUnit );
			hud->SetStateInt( "arx_ai_health_team", team );

		} else {
			hud->HandleNamedEvent( "noAIHealth" );
		}
	} else {
		hud->HandleNamedEvent( "noAIHealth" );
	}
}

// sikk---> Depth Render
/*
==================
idPlayer::ToggleSuppression
==================
*/
void idPlayer::ToggleSuppression( bool bSuppress ) {

	// Solarsplace - Added entity checks. This crashes if there is no player model when testing to go stand alone.
	return;

	int headHandle = 0;
	int weaponHandle = 0;
	int weaponWorldHandle = 0;
	renderEntity_t *headRenderEntity;
	renderEntity_t *weaponRenderEntity;
	renderEntity_t *weaponWorldRenderEntity;

	if ( head.GetEntity() ) {
		headHandle = head.GetEntity()->GetModelDefHandle();
		headRenderEntity = head.GetEntity()->GetRenderEntity();
	}

	if ( weapon.GetEntity() )
	{
		weaponHandle = weapon.GetEntity()->GetModelDefHandle();
		weaponWorldHandle = weapon.GetEntity()->GetWorldModel()->GetEntity()->GetModelDefHandle();
		weaponRenderEntity = weapon.GetEntity()->GetRenderEntity();
		weaponWorldRenderEntity	= weapon.GetEntity()->GetWorldModel()->GetEntity()->GetRenderEntity();
	}

	if ( bSuppress ) {
		if ( modelDefHandle != -1 ) {
// sikk---> First Person Body
			/*
			if ( !g_showFirstPersonBody.GetBool() ) {
				// suppress body in DoF render view
				renderEntity.suppressSurfaceInViewID = -8;
				gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
			}
			*/
// <---sikk
			if ( head.GetEntity() && headHandle != -1 ) {
				// suppress head in DoF render view
				headRenderEntity->suppressSurfaceInViewID = -8;
				gameRenderWorld->UpdateEntityDef( headHandle, headRenderEntity );
			}

			if ( weaponEnabled && weapon.GetEntity() && weaponHandle != -1 ) {
				// allow weapon view model in DoF render view
				weaponRenderEntity->allowSurfaceInViewID = -8;
				gameRenderWorld->UpdateEntityDef( weaponHandle, weaponRenderEntity );

				if ( weaponWorldHandle != -1 ) {
					// suppress weapon world model in DoF render view
					weaponWorldRenderEntity->suppressSurfaceInViewID = -8;
					gameRenderWorld->UpdateEntityDef( weaponWorldHandle, weaponWorldRenderEntity );
				}
			}
		}

		bViewModelsModified = true;
	} else {
		if ( modelDefHandle != -1 ) {
			// restore suppression of body
			renderEntity.suppressSurfaceInViewID = entityNumber + 1;
			gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );

			if ( head.GetEntity() && headHandle != -1 ) {
				// restore suppression of head
				headRenderEntity->suppressSurfaceInViewID = entityNumber + 1;
				gameRenderWorld->UpdateEntityDef( headHandle, headRenderEntity );
			}

			if ( weaponEnabled && weapon.GetEntity() && weaponHandle != -1  ) {
				// restore allowance of weapon view model
				weaponRenderEntity->allowSurfaceInViewID = entityNumber + 1;
				gameRenderWorld->UpdateEntityDef( weaponHandle, weaponRenderEntity );
				
				if ( weaponWorldHandle != -1 ) {
					// restore suppression of weapon world model
					weaponWorldRenderEntity->suppressSurfaceInViewID = entityNumber + 1;
					gameRenderWorld->UpdateEntityDef( weaponWorldHandle, weaponWorldRenderEntity );
				}
			}
		}

		bViewModelsModified = false;
	}
}
// <---sikk

// sikk---> Infrared Goggles/Headlight/Ambient light Mod
/*
==================
idPlayer::UpdateBattery
==================
*/
/*
void idPlayer::UpdateBattery() {
	// update ir goggle's battery usage
	if ( bIRGogglesOn ) {
		if ( nIRGogglesTime <= gameLocal.time ) {
			nBattery--;
			nIRGogglesTime = ( g_batteryLife.GetInteger() * 10 ) + gameLocal.time;
		}
		if ( nBattery <= 0 ) {
			nBattery = 0;
			ToggleIRGoggles();
		} else if ( GetCurrentWeapon() == 12 || gameLocal.inCinematic || PowerUpActive( BERSERK ) || ( GetInfluenceMaterial() || GetInfluenceEntity() ) ) {
			ToggleIRGoggles();
		}
	}

	// update headlight's battery usage
	if ( bHeadlightOn ) {
		idEntity *ent = gameLocal.FindEntity( name + "_headlight" );

		if ( nHeadlightTime <= gameLocal.time ) {
			nHeadlightTime = ( g_batteryLife.GetInteger() * 10 ) + gameLocal.time;
			nBattery--;
		}

		// update headlight angle and intensity
		if ( ent ) {
			ent->SetAngles( viewAngles );

			if ( nBattery > 5 )
				fIntensity = 1.0f;
			else {
				fIntensity = fIntensity * 0.95 + ( nBattery * 0.2 ) * 0.05;
			}

			static_cast<idLight*>( ent )->SetLightParms( 1.0f, 1.0f, 1.0f, fIntensity );
		}

		if ( nBattery <= 0 ) {
			nBattery = 0;
			ToggleHeadlight();
		} else if ( GetCurrentWeapon() >= 11 || gameLocal.inCinematic ) {
			ToggleHeadlight();
		}
	} 
	
	if ( nBattery < 100 && !bIRGogglesOn && !bHeadlightOn ) {
		if ( nHeadlightTime <= gameLocal.time && nIRGogglesTime <= gameLocal.time ) {
			nHeadlightTime = nIRGogglesTime = ( g_batteryRechargeRate.GetInteger() * 10 ) + gameLocal.time;
			nBattery++;
		}
		if ( nBattery >= 100 )
			nBattery = 100;
	}

	// update hud element
	if ( hud ) {
		hud->SetStateInt( "player_battery", nBattery );
		hud->HandleNamedEvent( "UpdateBattery" );
	}
}
*/

/*
==================
idPlayer::ToggleIRGoggles
==================
*/
/*
void idPlayer::ToggleIRGoggles() {
	if ( !gameLocal.GetLocalPlayer() )
		return;

	bIRGogglesOn = !bIRGogglesOn;

	if ( bIRGogglesOn ) {
		idEntity *ent;
		idDict args;

		// play sound
		StartSoundShader( declManager->FindSound( "player_sounds_irgoggles_on" ), SND_CHANNEL_VOICE, 0, false, NULL );

		if ( !g_goggleType.GetBool() ) {
			// spawn ir ambient light
			args.Set( "classname", "light" );
			args.Set( "name", name + "_irlight" );					// light name
			args.Set( "texture", "lights/ambientLight" );			// light texture
			args.Set( "_color", "0.0625 0.0625 0.0625" );			// light color
			args.Set( "light_radius", "512 512 512" );				// light radius
			args.Set( "light_center", "-256 0 0" );					// light center
			gameLocal.SpawnEntityDef( args, &ent );
			ent->Bind( this, true );								// light bind parent
			ent->SetOrigin( idVec3( 256, 0, 48 ) );					// light origin
		}

		// save current bloom parms
		fIRBloomParms[ 0 ] = r_useBloom.GetBool();
		fIRBloomParms[ 1 ] = r_bloomBufferSize.GetInteger();
		fIRBloomParms[ 2 ] = r_bloomBlurIterations.GetInteger();
		fIRBloomParms[ 3 ] = r_bloomBlurScaleX.GetFloat();
		fIRBloomParms[ 4 ] = r_bloomBlurScaleY.GetFloat();
		fIRBloomParms[ 5 ] = r_bloomScale.GetFloat();
		fIRBloomParms[ 6 ] = r_bloomGamma.GetFloat();

		if ( !g_goggleType.GetBool() ) {
			// set specific bloom parms for ir
			r_useBloom.SetBool( true );
			r_bloomBufferSize.SetInteger( 2 );
			r_bloomBlurIterations.SetInteger( 1 );
			r_bloomBlurScaleX.SetFloat( 1.0f );
			r_bloomBlurScaleY.SetFloat( 1.0f );
			r_bloomScale.SetFloat( 1.25f );
			r_bloomGamma.SetFloat( 1.25f );
		}
	} else {
		// play sound
		StartSoundShader( declManager->FindSound( "player_sounds_irgoggles_off" ), SND_CHANNEL_VOICE, 0, false, NULL );

		// reset bloom parms
		r_useBloom.SetInteger( fIRBloomParms[ 0 ] );
		r_bloomBufferSize.SetInteger( (int)fIRBloomParms[ 1 ] );
		r_bloomBlurIterations.SetInteger( (int)fIRBloomParms[ 2 ] );
		r_bloomBlurScaleX.SetFloat( (int)fIRBloomParms[ 3 ] );
		r_bloomBlurScaleY.SetFloat( fIRBloomParms[ 4 ] );
		r_bloomScale.SetFloat( fIRBloomParms[ 5 ] );
		r_bloomGamma.SetFloat( fIRBloomParms[ 6 ] );

		// find and remove ir ambient light
		idEntity *ent = gameLocal.FindEntity( name + "_irlight" );
		if ( !ent )
			return;
		delete ent;
	}
}
*/

/*
==================
idPlayer::ToggleHeadlight
==================
*/
/*
void idPlayer::ToggleHeadlight() {
	if ( !gameLocal.GetLocalPlayer() )
		return;

	bHeadlightOn = !bHeadlightOn;

	if ( bHeadlightOn ) {
		idEntity *ent;
		idDict args;

		// play sound
		StartSoundShader( declManager->FindSound( "player_sounds_headlight_on" ), SND_CHANNEL_VOICE, 0, false, NULL );

		// spawn headlight light
		args.Set( "classname", "light" );
		args.Set( "name", name + "_headlight" );	// light name
		args.Set( "texture", "lights/headlight" );	// light texture
		args.Set( "light_target", "768 0 0" );		// light cone direction
 		args.Set( "light_up", "0 0 192" );			// light cone height
 		args.Set( "light_right", "0 -192 0" );		// light cone width
		gameLocal.SpawnEntityDef( args, &ent );
		ent->BindToJoint( this, "head", 0.0f );		// light bind parent joint
		ent->SetOrigin( idVec3( 8, -12, 4 ) );		// light origin
		ent->SetAngles( viewAngles );				// light angle
	} else {
		// play sound
		StartSoundShader( declManager->FindSound( "player_sounds_headlight_off" ), SND_CHANNEL_VOICE, 0, false, NULL );

		// find and remove headlight light
		idEntity *ent = gameLocal.FindEntity( name + "_headlight" );
		if ( !ent )
			return;
		delete ent;
	}
}
*/

/*
==================
idPlayer::ToggleAmbientLight
==================
*/
void idPlayer::ToggleAmbientLight( bool bOn ) {
	if ( !gameLocal.GetLocalPlayer() )
		return;

	idEntity *ent;
	renderView_t *rv = GetRenderView();

	// this is so we update on any changes to color or radius
	if ( bOn && bAmbientLightOn ) {
		ent = gameLocal.FindEntity( name + "_ambientlight" );
		if ( !ent ) {
			bAmbientLightOn = false;
			return;
		}

		// update light origin & angle
		ent->SetOrigin( rv->vieworg + ( rv->viewaxis.ToAngles().ToForward() * 256.0f ) );	// light origin
//		ent->SetAngles( rv->viewaxis.ToAngles() );				// light angle

		// this is so we update on any changes to color or radius
		// we remove it and spawn it again below
		if ( szAmbientLightColor != g_ambientLightColor.GetString() ||
			 szAmbientLightRadius != g_ambientLightRadius.GetString() ) {
			// remove ambient light
			delete ent;
			bAmbientLightOn = false;
		}
	}

	szAmbientLightColor = g_ambientLightColor.GetString();
	szAmbientLightRadius = g_ambientLightRadius.GetString();

	if ( bOn && !bAmbientLightOn ) {
		idEntity *ent;
		idDict args;

		// spawn ambient light
		args.Set( "classname", "light" );
		args.Set( "name", name + "_ambientlight" );				// light name
		args.Set( "texture", "lights/ambientLight" );			// light texture
		args.Set( "_color", szAmbientLightColor );				// light color
		args.Set( "light_radius", szAmbientLightRadius );		// light radius
		gameLocal.SpawnEntityDef( args, &ent );

		ent->SetOrigin( rv->vieworg + ( rv->viewaxis.ToAngles().ToForward() * 256.0f ) );	// light origin

		bAmbientLightOn = true;
	} else if ( !bOn && bAmbientLightOn ) {
		// find and remove ambient light
		idEntity *ent = gameLocal.FindEntity( name + "_ambientlight" );
		if ( !ent ) {
			bAmbientLightOn = false;
			return;
		}
		delete ent;
		bAmbientLightOn = false;
	}
}
// <---sikk

// sikk---> Health Management System (Health Pack)
/*
==================
idPlayer::UseHealthPack
==================
*/
/*
void idPlayer::UseHealthPack() {
	if ( ( health > 0 ) && ( health < inventory.maxHealth ) && healthPackAmount ) {
		int oldhealth = health;
		int oldhealthPackAmount = healthPackAmount;

		healthPackAmount -= g_healthPackTotal.GetInteger() / g_healthPackUses.GetInteger();
		healthPackAmount = ( healthPackAmount < 0 ) ? 0 : healthPackAmount;

		health += ( oldhealthPackAmount - healthPackAmount );
		if ( health > inventory.maxHealth )
			health = inventory.maxHealth;
		if ( hud )
			hud->HandleNamedEvent( "healthPulse" );

		int time = ( health - oldhealth ) * g_healthPackTime.GetInteger() * 10;
		time = ( time < 1000 ) ? 1000 : time;
		healthPackTimer = gameLocal.time + time;
		StartSoundShader( declManager->FindSound( "pack_pickup" ), SND_CHANNEL_ITEM, 0, false, NULL );
	}
}
*/
// <---sikk

// sikk---> Adrenaline Pack System
/*
==================
idPlayer::UseAdrenaline
==================
*/
/*
void idPlayer::UseAdrenaline() {
	if ( adrenalineAmount ) {
		inventory.GivePowerUp( this, ADRENALINE, 0 );
		StartSoundShader( declManager->FindSound( "pickup_adrenaline" ), SND_CHANNEL_ITEM, 0, false, NULL );
		stamina = 100.0f;
		adrenalineAmount = 0;
	}
}
*/
// <---sikk

// sikk---> Searchable Corpses
/*
==================
idPlayer::SearchCorpse
==================
*/
/*
void idPlayer::SearchCorpse( idAFEntity_Gibbable* corpse ) {
	StartSoundShader( declManager->FindSound( "use_search" ), SND_CHANNEL_VOICE, 0, false, NULL );
	searchTimer = gameLocal.time + 1500;
	corpse->searchable = false;

	if ( ( gameLocal.random.RandomFloat() * 0.99999f ) < g_itemSearchFactor.GetFloat() ) {
		idEntity *ent;
		idDict args;
		idVec3 itemOrg = GetEyePosition() + viewAngles.ToForward() * 32.0f;

		float random = gameLocal.random.RandomFloat();
		const char* defItem = corpse->spawnArgs.GetString( "def_searchItem" );
		if ( !idStr::Icmp( defItem, "" ) ) {
			if ( random <= 0.1 )
				defItem = "powerup_adrenaline";
			else if ( random <= 0.5 )
				defItem = "item_medkit_small";
			else
				defItem = "item_armor_shard";
		} else {
			if ( random <= 0.05 )
				defItem = "powerup_adrenaline";
			else if ( random <= 0.33333333 )
				defItem = "item_medkit_small";
			else if ( random <= 0.66666666 )
				defItem = "item_armor_shard";
		}

		args.Set( "classname", defItem );
		args.Set( "removeable", "0" );
		args.SetVector( "origin", itemOrg );
		gameLocal.SpawnEntityDef( args, &ent );
	}
}
*/
// <---sikk

// sikk---> Weapon Management: Awareness
/*
==================
idPlayer::GetWeaponAwareness
==================
*/
/*
bool idPlayer::GetWeaponAwareness() {
	if ( g_weaponAwareness.GetBool() ) {
		idEntity *ent;
		trace_t trace;
		idVec3 start = GetEyePosition();
		idVec3 end = start + viewAngles.ToForward() * 32.0f;
		gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_RENDERMODEL, this );
		bWATrace = false;
		
		if ( ( trace.fraction < 1.0f ) && ( GetCurrentWeapon() > 0 && GetCurrentWeapon() < 9 ) ) {
			ent = gameLocal.entities[ trace.c.entityNum ];
			if ( ent && !ent->IsType( idAI::Type ) ) 
				bWATrace = true;
			if ( ent->GetBindMaster() && ent->GetBindMaster()->IsType( idAI::Type ) )
				bWATrace = false;
		}

		bWAIsSprinting = ( ( GetCurrentWeapon() > 0 ) && AI_RUN && ( AI_FORWARD || AI_BACKWARD || AI_STRAFE_LEFT || AI_STRAFE_RIGHT ) );

		if ( bWATrace || bWAIsSprinting || OnLadder() )
			return true;
	}

	return false;
}
*/
// <---sikk