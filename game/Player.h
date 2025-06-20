// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_PLAYER_H__
#define __GAME_PLAYER_H__

/*
===============================================================================

	Player entity.
	
===============================================================================
*/

// <---- *** Arx ***

const int ARX_NEXT_HUNGRY_DEFAULT		= SEC2MS( 60 ) * 10;	// 15 minutes default.
const int ARX_HUNGER_WARNING_TIME		= SEC2MS( 60 ) * 2;		// Start to warn 2 minutes before hunger damage.
const int ARX_HUNGER_DAMAGE_INC			= SEC2MS( 30 );			// Increase hunger damage every x seconds

const int ARX_FISTS_WEAPON				= 0;
const int ARX_MAGIC_WEAPON				= 10;					// Id of the magic casting weapon
const int ARX_MANA_WEAPON				= ARX_MAGIC_WEAPON;		// This weapon will need to be a weapon that uses mana in order to use this as a guage for the mana hud item.
const int ARX_MANA_TYPE					= 1;					// See entityDef ammo_types - "ammo_mana" "1"

const int ARX_DEFAULT_BLACKSMITH_SKILL = 94;

const int ARX_MAX_EQUIPED_ITEMS = 4;
const int ARX_MAX_PLAYER_LEVELS = 11; // Level 0 to 10
const int ARX_MAX_SKILLS = 9;
const int ARX_MAX_ATTRIBUTES = 4;
const int ARX_MAX_ATTRIBUTE_POINTS = 20; // _DT - max points for each attribute.
const int ARX_MAX_SKILL_POINTS = 100; // _DT - max points for each skill.
// const int ARX_SKILL_BASE_VALUE = 10; // _DT commented out

enum {
	ARX_WEAPON_TYPE_MELEE = 0,
	ARX_WEAPON_TYPE_PROJECTILE = 1
};

enum {
	ARX_DAMAGE_TYPE_NON_MAGIC = 0,
	ARX_DAMAGE_TYPE_MAGICAL = 1
};

enum {
	ARX_SPELL_INVIS = 0,
	ARX_SPELL_TELEKENESIS_DURATION,
	ARX_SPELL_TELEKENESIS_DISTANCE,
	ARX_SPELL_LEVITATE
};

enum {
	ARX_EQUIPED_RING_LEFT = 0,
	ARX_EQUIPED_RING_RIGHT,
	ARX_EQUIPED_SUIT,
	ARX_EQUIPED_WEAPON
};

// Arx - Attributes / SKills / Classes
enum {
	ARX_ATTR_STRENGTH = 0,
	ARX_ATTR_MENTAL,
	ARX_ATTR_DEXTERITY,
	ARX_ATTR_CONSTITUTION
};

enum {
	ARX_STAT_HEALTH = 0,
	ARX_STAT_MANA,
	ARX_STAT_STAMINA
};

enum {
	ARX_SKILL_STEALTH = 0,
	ARX_SKILL_TECHNICAL,
	ARX_SKILL_INTUITION,
	ARX_SKILL_ETHEREAL_LINK,
	arx_skill_object_knowledge,
	ARX_SKILL_CASTING,
	ARX_SKILL_CLOSE_COMBAT,
	ARX_SKILL_PROJECTILE,
	ARX_SKILL_DEFENSE
};

enum {
	ARX_BONUS_SPEED = 0,
	ARX_NORMAL_PROJECTILE_DAMAGE,
	ARX_MAGIC_PROJECTILE_DAMAGE,
	ARX_MELEE_DAMAGE,
	ARX_MELEE_DISTANCE
};

struct _arxLevelMaps {
	idStr mapFileSystemName;
	idStr mapName;
	idStr mapDescription;
	idStr mapImageFile;
};

// ----> *** Arx ***

extern const idEventDef EV_Player_GetButtons;
extern const idEventDef EV_Player_GetMove;
extern const idEventDef EV_Player_GetViewAngles;
extern const idEventDef EV_Player_EnableWeapon;
extern const idEventDef EV_Player_DisableWeapon;
extern const idEventDef EV_Player_ExitTeleporter;
extern const idEventDef EV_Player_SelectWeapon;
extern const idEventDef EV_SpectatorTouch;

const float THIRD_PERSON_FOCUS_DISTANCE	= 512.0f;
const int	LAND_DEFLECT_TIME = 150;
const int	LAND_RETURN_TIME = 300;
const int	FOCUS_TIME = 300;
const int	FOCUS_GUI_TIME = 500;

const int MAX_WEAPONS = 16;

const int DEAD_HEARTRATE = 0;			// fall to as you die
const int LOWHEALTH_HEARTRATE_ADJ = 20; // 
const int DYING_HEARTRATE = 30;			// used for volumen calc when dying/dead
const int BASE_HEARTRATE = 70;			// default
const int ZEROSTAMINA_HEARTRATE = 115;  // no stamina
const int MAX_HEARTRATE = 130;			// maximum
const int ZERO_VOLUME = -40;			// volume at zero
const int DMG_VOLUME = 5;				// volume when taking damage
const int DEATH_VOLUME = 15;			// volume at death

const int SAVING_THROW_TIME = 5000;		// maximum one "saving throw" every five seconds

const int ASYNC_PLAYER_INV_AMMO_BITS = idMath::BitsForInteger( 999 );	// 9 bits to cover the range [0, 999]
const int ASYNC_PLAYER_INV_CLIP_BITS = -7;								// -7 bits to cover the range [-1, 60]

struct idItemInfo {
	idStr name;
	idStr icon;
};

struct idObjectiveInfo {
	idStr title;
	idStr text;
	idStr screenshot;
};

struct idLevelTriggerInfo {
	idStr levelName;
	idStr triggerName;
};

// powerups - the "type" in item .def must match
enum {
	BERSERK = 0, 
	INVISIBILITY,
	MEGAHEALTH,
	ADRENALINE,
	MAX_POWERUPS
};

// powerup modifiers
enum {
	SPEED = 0,
	PROJECTILE_DAMAGE,
	MELEE_DAMAGE,
	MELEE_DISTANCE
};

// influence levels
enum {
	INFLUENCE_NONE = 0,			// none
	INFLUENCE_LEVEL1,			// no gun or hud
	INFLUENCE_LEVEL2,			// no gun, hud, movement
	INFLUENCE_LEVEL3,			// slow player movement
};

class idInventory {
public:
	int						maxHealth;
	int						weapons;
	int						powerups;
	int						armor;
	int						maxarmor;
	int						ammo[ AMMO_NUMTYPES ];
	int						clip[ MAX_WEAPONS ];
	int						powerupEndTime[ MAX_POWERUPS ];

	// Solarsplace - Arx EOS
	int						money;

	idStr					weaponUniqueName; // Store unique string name of current active weapon

	idStr					arx_equiped_items[ ARX_MAX_EQUIPED_ITEMS ];

	int						arx_snake_weapon; // The current magic weapon
	int						arx_last_melee_weapon; // The last melee weapon the player was using

	bool					arx_new_hero_created;

	bool					arx_player_level_up_in_progress;
	int						arx_player_level;
	int						arx_player_x_points;

	int						arx_attribute_points;
	int						arx_skill_points;

	int						arx_attr_strength;
	int						arx_attr_mental;
	int						arx_attr_dexterity;
	int						arx_attr_constitution;

	int						arx_skill_casting;
	int						arx_skill_close_combat;
	int						arx_skill_defense;
	int						arx_skill_ethereal_link;
	int						arx_skill_intuition;
	int						arx_skill_object_knowledge;
	int						arx_skill_projectile;
	int						arx_skill_stealth;
	int						arx_skill_technical;

	int						arx_class_armour_points;
	int						arx_class_health_points;
	int						arx_class_mana_points;
	int						arx_class_resistance_to_magic;
	int						arx_class_resistance_to_poison;
	int						arx_class_damage_points;

	// Base values that do not change
	int						arx_class_armour_points_base;
	int						arx_class_health_points_base;
	int						arx_class_mana_points_base;
	int						arx_class_resistance_to_magic_base;
	int						arx_class_resistance_to_poison_base;
	int						arx_class_damage_points_base;

	int						arx_stat_secrets_found;
	int						arx_stat_ai_kills_total;
	int						arx_stat_flowers_mana_found;
	int						arx_stat_flowers_health_found;

	int						tmp_arx_attribute_points;
	int						tmp_arx_skill_points;

	int						tmp_arx_attr_strength;
	int						tmp_arx_attr_mental;
	int						tmp_arx_attr_dexterity;
	int						tmp_arx_attr_constitution;

	int						tmp_arx_skill_casting;
	int						tmp_arx_skill_close_combat;
	int						tmp_arx_skill_defense;
	int						tmp_arx_skill_ethereal_link;
	int						tmp_arx_skill_intuition;
	int						tmp_arx_skill_object_knowledge;
	int						tmp_arx_skill_projectile;
	int						tmp_arx_skill_stealth;
	int						tmp_arx_skill_technical;

	int						tmp_arx_class_armour_points;
	int						tmp_arx_class_health_points;
	int						tmp_arx_class_mana_points;
	int						tmp_arx_class_resistance_to_magic;
	int						tmp_arx_class_resistance_to_poison;
	int						tmp_arx_class_damage_points;

	int						arx_timer_player_stats_update;
	int						arx_timer_player_poison;
	int						arx_timer_player_invisible;
	int						arx_timer_player_onfire;
	int						arx_timer_player_telekinesis;
	int						arx_timer_player_levitate;
	int						arx_timer_player_warmth;
	int						arx_timer_player_hungry;

	idList<_arxLevelMaps>	arxLevelMaps;

	void					ClearDownTimedAttributes( bool clearDown );		// solarsplace 24th Nov 2013

	// mp
	int						ammoPredictTime;

	int						deplete_armor;
	float					deplete_rate;
	int						deplete_ammount;
	int						nextArmorDepleteTime;

	int						pdasViewed[4]; // 128 bit flags for indicating if a pda has been viewed

	int						selPDA;
	int						selEMail;
	int						selVideo;
	int						selAudio;
	bool					pdaOpened;
	bool					turkeyScore;
	idList<idDict *>		items;
	idStrList				pdas;
	idStrList				pdaSecurity;
	idStrList				videos;
	idStrList				emails;

	bool					ammoPulse;
	bool					weaponPulse;
	bool					armorPulse;
	int						lastGiveTime;

	idList<idLevelTriggerInfo> levelTriggers;

							idInventory() { Clear(); }
							~idInventory() { Clear(); }

	// save games
	void					Save( idSaveGame *savefile ) const;					// archives object for save game file
	void					Restore( idRestoreGame *savefile );					// unarchives object from save game file

	void					Clear( void );
	void					GivePowerUp( idPlayer *player, int powerup, int msec );
	void					ClearPowerUps( void );
	void					GetPersistantData( idDict &dict );
	void					RestoreInventory( idPlayer *owner, const idDict &dict );
	bool					Give( idPlayer *owner, const idDict &spawnArgs, const char *statname, const char *value, int *idealWeapon, bool updateHud );
	void					Drop( const idDict &spawnArgs, const char *weapon_classname, int weapon_index );
	ammo_t					AmmoIndexForAmmoClass( const char *ammo_classname ) const;
	int						MaxAmmoForAmmoClass( idPlayer *owner, const char *ammo_classname ) const;
	int						WeaponIndexForAmmoClass( const idDict & spawnArgs, const char *ammo_classname ) const;
	ammo_t					AmmoIndexForWeaponClass( const char *weapon_classname, int *ammoRequired );
	const char *			AmmoPickupNameForIndex( ammo_t ammonum ) const;
	void					AddPickupName( const char *name, const char *icon );

	int						HasAmmo( ammo_t type, int amount );
	bool					UseAmmo( ammo_t type, int amount );
	int						HasAmmo( const char *weapon_classname );			// looks up the ammo information for the weapon class first

	void					UpdateArmor( void );

	int						nextItemPickup;
	int						nextItemNum;
	int						onePickupTime;
	idList<idItemInfo>		pickupItemNames;
	idList<idObjectiveInfo>	objectiveNames;
};

typedef struct {
	int		time;
	idVec3	dir;		// scaled larger for running
} loggedAccel_t;

typedef struct {
	int		areaNum;
	idVec3	pos;
} aasLocation_t;

class idPlayer : public idActor {
public:
	enum {
		EVENT_IMPULSE = idEntity::EVENT_MAXEVENTS,
		EVENT_EXIT_TELEPORTER,
		EVENT_ABORT_TELEPORTER,
		EVENT_POWERUP,
		EVENT_SPECTATE,
		EVENT_MAXEVENTS
	};

	usercmd_t				usercmd;

	class idPlayerView		playerView;			// handles damage kicks and effects

	bool					noclip;
#ifdef _DT // levitate spell
	bool					levitate;
#endif
	bool					godmode;

	bool					spawnAnglesSet;		// on first usercmd, we must set deltaAngles
	idAngles				spawnAngles;
	idAngles				viewAngles;			// player view angles
	idAngles				cmdAngles;			// player cmd angles

	int						buttonMask;
	int						oldButtons;
	int						oldFlags;

	int						lastHitTime;			// last time projectile fired by player hit target
	int						lastSndHitTime;			// MP hit sound - != lastHitTime because we throttle
	int						lastSavingThrowTime;	// for the "free miss" effect

	idScriptBool			AI_FORWARD;
	idScriptBool			AI_BACKWARD;
	idScriptBool			AI_STRAFE_LEFT;
	idScriptBool			AI_STRAFE_RIGHT;
	idScriptBool			AI_ATTACK_HELD;
	idScriptBool			AI_WEAPON_FIRED;
	idScriptBool			AI_JUMP;
	idScriptBool			AI_CROUCH;
	idScriptBool			AI_ONGROUND;
	idScriptBool			AI_ONLADDER;
	idScriptBool			AI_DEAD;
	idScriptBool			AI_RUN;
	idScriptBool			AI_PAIN;
	idScriptBool			AI_HARDLANDING;
	idScriptBool			AI_SOFTLANDING;
	idScriptBool			AI_RELOAD;
	idScriptBool			AI_TELEPORT;
	idScriptBool			AI_TURN_LEFT;
	idScriptBool			AI_TURN_RIGHT;

	// inventory
	idInventory				inventory;

	idEntityPtr<idWeapon>	weapon;
	idUserInterface *		hud;				// MP: is NULL if not local player
	idUserInterface *		objectiveSystem;
	bool					objectiveSystemOpen;

	/*
	*****************************************************************************************************
	*****************************************************************************************************
	*****************************************************************************************************
	*** BEGIN - Solarsplace - Arx EOS - PUBLIC
	*/

	/* All the Arx GUI's are

		inventorySystem
		journalSystem
		readableSystem
		conversationSystem
		shoppingSystem

	*/

	idStr					fullScreenMenuGUIId;

	// Solarsplace 10th April 2010 - Inventory related
	idUserInterface *		inventorySystem;
	bool					inventorySystemOpen;

	// Solarsplace 6th May 2010 - Journal GUI related
	idUserInterface *		journalSystem;
	bool					journalSystemOpen;

	// Solarsplace 6th May 2010 - Readable GUI related
	idUserInterface *		readableSystem;
	bool					readableSystemOpen;
	idEntity *				lastReadableEntity;
	int						lastReadablePage;

	// Solarsplace 2nd Nov 2011 - NPC GUI related
	idUserInterface *		conversationSystem;
	bool					conversationSystemOpen;
	idStr					conversationWindowQuestId;

	// Solarsplace 6th Nov 2011 - Shopping GUI related
	idUserInterface *		shoppingSystem;
	bool					shoppingSystemOpen;
	idEntity *				lastShopEntity;

	// Solarsplace 1st Apr 2016 - Teleporter GUI related
	idUserInterface *		teleporterSystem;
	bool					teleporterSystemOpen;

	// Solarsplace - Water related
	int						waterScreenFinishTime;
	bool					playerUnderWater;

	// Solarsplace - Magic related
	bool					magicModeActive;
	bool					lastMagicModeActive;

	// AI related
	void					AlertAI( bool playerVisible, float alertRadius, int aiTeam, int teamAlertOptions );
	idVec3					lastPlayerAlertOrigin;					

	// Skills & stats related
	void					ModifyPlayerXPs( int XPs, bool showMessage );
	float					ArxSkillGetAlertDistance( float defaultDistance );
	int						ArxCalculateOwnWeaponDamage( int baseDamageAmount, int weaponSkillType );
	float					ArxCalculateD3GameBonuses( float baseValue, int bonusType );
	bool					ArxCalculateHeroChance( idStr chanceDescription );					// solarsplace 15th Mar 2013
	void					ArxPlayerLevelUp( void );
	void					ArxHandleRunesGUI( int selectedSpellIndex );

	/*
	*** END - Solarsplace - Arx EOS - PUBLIC
	*****************************************************************************************************
	*****************************************************************************************************
	*****************************************************************************************************
	*/

	int						weapon_soulcube;
	int						weapon_pda;
	int						weapon_fists;

	int						heartRate;
	idInterpolate<float>	heartInfo;
	int						lastHeartAdjust;
	int						lastHeartBeat;
	int						lastDmgTime;
	int						deathClearContentsTime;
	bool					doingDeathSkin;
	int						lastArmorPulse;		// lastDmgTime if we had armor at time of hit
	float					stamina;
	float					healthPool;			// amount of health to give over time
	int						nextHealthPulse;
	bool					healthPulse;
	bool					healthTake;
	int						nextHealthTake;


	bool					hiddenWeapon;		// if the weapon is hidden ( in noWeapons maps )
	idEntityPtr<idProjectile> soulCubeProjectile;

	// mp stuff
	static idVec3			colorBarTable[ 5 ];
	int						spectator;
	idVec3					colorBar;			// used for scoreboard and hud display
	int						colorBarIndex;
	bool					scoreBoardOpen;
	bool					forceScoreBoard;
	bool					forceRespawn;
	bool					spectating;
	int						lastSpectateTeleport;
	bool					lastHitToggle;
	bool					forcedReady;
	bool					wantSpectate;		// from userInfo
	bool					weaponGone;			// force stop firing
	bool					useInitialSpawns;	// toggled by a map restart to be active for the first game spawn
	int						latchedTeam;		// need to track when team gets changed
	int						tourneyRank;		// for tourney cycling - the higher, the more likely to play next - server
	int						tourneyLine;		// client side - our spot in the wait line. 0 means no info.
	int						spawnedTime;		// when client first enters the game

	idEntityPtr<idEntity>	teleportEntity;		// while being teleported, this is set to the entity we'll use for exit
	int						teleportKiller;		// entity number of an entity killing us at teleporter exit
	bool					lastManOver;		// can't respawn in last man anymore (srv only)
	bool					lastManPlayAgain;	// play again when end game delay is cancelled out before expiring (srv only)
	bool					lastManPresent;		// true when player was in when game started (spectators can't join a running LMS)
	bool					isLagged;			// replicated from server, true if packets haven't been received from client.
	bool					isChatting;			// replicated from server, true if the player is chatting.

	// timers
	int						minRespawnTime;		// can respawn when time > this, force after g_forcerespawn
	int						maxRespawnTime;		// force respawn after this time

	// the first person view values are always calculated, even
	// if a third person view is used
	idVec3					firstPersonViewOrigin;
	idMat3					firstPersonViewAxis;

#ifdef _DT	// head anim
	idMat3					firstPersonViewWeaponAxis;
#endif

	idDragEntity			dragEntity;

public:
	CLASS_PROTOTYPE( idPlayer );

							idPlayer();
	virtual					~idPlayer();

	void					Spawn( void );
	void					Think( void );

	// save games
	void					Save( idSaveGame *savefile ) const;					// archives object for save game file
	void					Restore( idRestoreGame *savefile );					// unarchives object from save game file

	virtual void			Hide( void );
	virtual void			Show( void );

	void					Init( void );
	void					PrepareForRestart( void );
	virtual void			Restart( void );
	void					LinkScriptVariables( void );
	void					SetupWeaponEntity( void );
	void					SelectInitialSpawnPoint( idVec3 &origin, idAngles &angles );
	void					SpawnFromSpawnSpot( void );
	void					SpawnToPoint( const idVec3	&spawn_origin, const idAngles &spawn_angles );
	void					SetClipModel( void );	// spectator mode uses a different bbox size

	void					SavePersistantInfo( void );
	void					RestorePersistantInfo( void );
	void					SetLevelTrigger( const char *levelName, const char *triggerName );

	bool					UserInfoChanged( bool canModify );
	idDict *				GetUserInfo( void );
	bool					BalanceTDM( void );

	void					CacheWeapons( void );

	void					EnterCinematic( void );
	void					ExitCinematic( void );
	bool					HandleESC( void );
	bool					SkipCinematic( void );

	void					UpdateConditions( void );
	void					SetViewAngles( const idAngles &angles );

							// delta view angles to allow movers to rotate the view of the player
	void					UpdateDeltaViewAngles( const idAngles &angles );

	virtual bool			Collide( const trace_t &collision, const idVec3 &velocity );

	virtual void			GetAASLocation( idAAS *aas, idVec3 &pos, int &areaNum ) const;
	virtual void			GetAIAimTargets( const idVec3 &lastSightPos, idVec3 &headPos, idVec3 &chestPos );
	virtual void			DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage );
	void					CalcDamagePoints(  idEntity *inflictor, idEntity *attacker, const idDict *damageDef,
							   const float damageScale, const int location, int *health, int *armor );
	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir, const char *damageDefName, const float damageScale, const int location );

							// use exitEntityNum to specify a teleport with private camera view and delayed exit
	virtual void			Teleport( const idVec3 &origin, const idAngles &angles, idEntity *destination );

	void					Kill( bool delayRespawn, bool nodamage );
	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
	void					StartFxOnBone(const char *fx, const char *bone);

	renderView_t *			GetRenderView( void );
	void					CalculateRenderView( void );	// called every tic by player code
	void					CalculateFirstPersonView( void );

	void					DrawHUD( idUserInterface *hud );

	void					WeaponFireFeedback( const idDict *weaponDef );

	float					DefaultFov( void ) const;
	float					CalcFov( bool honorZoom );
	void					CalculateViewWeaponPos( idVec3 &origin, idMat3 &axis );
	idVec3					GetEyePosition( void ) const;
	void					GetViewPos( idVec3 &origin, idMat3 &axis ) const;

#ifdef _DT	// head anim
	void					GetViewWeaponAxis( idMat3 &axis ) const;
#endif

	void					OffsetThirdPersonView( float angle, float range, float height, bool clip );

	bool					Give( const char *statname, const char *value );
	bool					GiveItem( idItem *item );
	void					GiveItem( const char *name );
	void					GiveHealthPool( float amt );
	
	bool					GiveInventoryItem( idDict *item );
	void					RemoveInventoryItem( idDict *item );
	bool					GiveInventoryItem( const char *name );
	void					RemoveInventoryItem( const char *name );
	idDict *				FindInventoryItem( const char *name );

	// Solarsplace - 2nd July 2010 - Needed a slightly modified version of the above function
	int						FindInventoryItemIndex( const char *name );
	int						FindInventoryItemCount( const char *name );
	int						FindInventoryItemIndexUniqueName( const char *uniqueName );
	int						FindInventoryWeaponIndex( int playerWeaponDefNumber, bool checkHealth );
	bool					UpdateInventoryItem( const char *uniqueItemName, const char *dictKey, const char *dictValue );
	bool					UpdateInventoryItem_health( int newWeaponHealth );
	bool					UpdateInventoryItem_health_max( int newWeaponHealthMax ); // Think of it as max health / durability an item can be repaired to
	const char *			GetInventoryItemHealthIcon( int health, int health_max, const idDict itemDict );
	const char *			GetInventoryEquippedItemHealthIcon( int health, int health_max );
	idStr					GetInventoryItemString( const char *uniqueItemName, const char *dictKey );
	bool					UpdateWeaponHealth( void );

	void					GivePDA( const char *pdaName, idDict *item );
	void					GiveVideo( const char *videoName, idDict *item );
	void					GiveEmail( const char *emailName );
	void					GiveSecurity( const char *security );
	void					GiveObjective( const char *title, const char *text, const char *screenshot );
	void					CompleteObjective( const char *title );

	bool					GivePowerUp( int powerup, int time );
	void					ClearPowerUps( void );
	bool					PowerUpActive( int powerup ) const;
	float					PowerUpModifier( int type );

	int						SlotForWeapon( const char *weaponName );
	void					Reload( void );
	void					NextWeapon( void );
	void					NextBestWeapon( void );
	void					PrevWeapon( void );
	void					SelectWeapon( int num, bool force );
	void					DropWeapon( bool died ) ;
	void					StealWeapon( idPlayer *player );
	void					AddProjectilesFired( int count );
	void					AddProjectileHits( int count );
	void					SetLastHitTime( int time );
	void					LowerWeapon( void );
	void					RaiseWeapon( void );
	void					WeaponLoweringCallback( void );
	void					WeaponRisingCallback( void );
	void					RemoveWeapon( const char *weap );
	bool					CanShowWeaponViewmodel( void ) const;

	void					AddAIKill( void );
	void					SetSoulCubeProjectile( idProjectile *projectile );

	void					AdjustHeartRate( int target, float timeInSecs, float delay, bool force );
	void					SetCurrentHeartRate( void );
	int						GetBaseHeartRate( void );
	void					UpdateAir( void );

	virtual bool			HandleSingleGuiCommand( idEntity *entityGui, idLexer *src );
	bool					GuiActive( void ) { return focusGUIent != NULL; }

	void					PerformImpulse( int impulse );
	void					Spectate( bool spectate );
	void					TogglePDA( void );

	/*
	*****************************************************************************************************
	*****************************************************************************************************
	*****************************************************************************************************
	*** BEGIN - Solarsplace - Arx EOS - Public
	*/

	// Inventory related
	void					ToggleInventorySystem( void );			// Solarsplace 10th Apr 2010
	void					UpdateInventoryGUI( void );				// solarsplace 14th Apr 2010

	// Journal related
	void					ToggleJournalSystem( void );			// Solarsplace 6th May 2010
	void					UpdateJournalGUI( void );				// solarsplace 6th May 2010

	// Readable related
	void					ToggleReadableSystem( void );			// Solarsplace 6th May 2010
	void					UpdateReadableGUI( void );				// solarsplace 6th May 2010

	// NPG GUI related
	void					ToggleConversationSystem( void );		// Solarsplace 2nd Nov 2011
	void					UpdateConversationSystem( void );		// Solarsplace 2nd Nov 2011

	// Shop GUI related
	void					ToggleShoppingSystem( void );		// Solarsplace 2nd Nov 2011
	void					UpdateShoppingSystem( void );		// Solarsplace 2nd Nov 2011

	// Teleporter GUI related
	void					ToggleTeleporterSystem( void );		// Solarsplace 1st Apr 2016
	void					UpdateTeleporterSystem( void );		// Solarsplace 1st Apr 2016
	void					ProcessTeleportation( void );		// Solarsplace 1st Apr 2016

	// Magic related
	void					ToggleMagicMode( void );				// Solarsplace 29th Apr 2010

	// Level Transition related
	void					SetMapEntryPoint( idStr entityName );
	idStr					GetMapEntryPoint( void );
	void					SaveTransitionInfoSpecific( idEntity *ent, bool spawnedItem, bool hiddenItem );

	// HUD messages
	void					ShowHudMessage( idStr message );		// Solarsplace 25th Sep 2011

	/*
	*** END - Solarsplace - Arx EOS - Public
	*****************************************************************************************************
	*****************************************************************************************************
	*****************************************************************************************************
	*/

	void					ToggleScoreboard( void );
	void					RouteGuiMouse( idUserInterface *gui );
	void					UpdateHud( void );
	const idDeclPDA *		GetPDA( void ) const;
	const idDeclVideo *		GetVideo( int index );
	void					SetInfluenceFov( float fov );
	void					SetInfluenceView( const char *mtr, const char *skinname, float radius, idEntity *ent );
	void					SetInfluenceLevel( int level );
	int						GetInfluenceLevel( void ) { return influenceActive; };
	void					SetPrivateCameraView( idCamera *camView );
	idCamera *				GetPrivateCameraView( void ) const { return privateCameraView; }
	void					StartFxFov( float duration  );
	void					UpdateHudWeapon( bool flashWeapon = true );
	void					UpdateHudStats( idUserInterface *hud );



	void					UpdateHudAmmo( idUserInterface *hud );
	void					Event_StopAudioLog( void );
	void					StartAudioLog( void );
	void					StopAudioLog( void );
	void					ShowTip( const char *title, const char *tip, bool autoHide );
	void					HideTip( void );
	bool					IsTipVisible( void ) { return tipUp; };
	void					ShowObjective( const char *obj );
	void					HideObjective( void );

	virtual void			ClientPredictionThink( void );
	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );
	void					WritePlayerStateToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadPlayerStateFromSnapshot( const idBitMsgDelta &msg );

	virtual bool			ServerReceiveEvent( int event, int time, const idBitMsg &msg );

	virtual bool			GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis );
	virtual bool			GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis );

	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );
	bool					IsReady( void );
	bool					IsRespawning( void );
	bool					IsInTeleport( void );

	idEntity				*GetInfluenceEntity( void ) { return influenceEntity; };
	const idMaterial		*GetInfluenceMaterial( void ) { return influenceMaterial; };
	float					GetInfluenceRadius( void ) { return influenceRadius; };

	// server side work for in/out of spectate. takes care of spawning it into the world as well
	void					ServerSpectate( bool spectate );
	// for very specific usage. != GetPhysics()
	idPhysics				*GetPlayerPhysics( void );
	void					TeleportDeath( int killer );
	void					SetLeader( bool lead );
	bool					IsLeader( void );

	void					UpdateSkinSetup( bool restart );

	bool					OnLadder( void ) const;

	virtual	void			UpdatePlayerIcons( void );
	virtual	void			DrawPlayerIcons( void );
	virtual	void			HidePlayerIcons( void );
	bool					NeedsIcon( void );

	bool					SelfSmooth( void );
	void					SetSelfSmooth( bool b );

#ifdef _DT // levitate spell
	void					Event_LevitateStart( void );
	void					Event_LevitateStop( void );
#endif

	int						nScreenFrostAlpha;	// sikk - Screen Frost

	/*
	int						nShowHudTimer;		// sikk - Dynamic hud system - Used to say when to show the hud as well as fade it in/out (just for health/armor/ammo/weapon changes)
	*/

// sikk---> Manual Item Pickup
	/*
	idItem*					focusItem;
	int						itemPickupTime;
	*/
// <---sikk

// sikk---> Searchable Corpses
	/*
	void					SearchCorpse( idAFEntity_Gibbable* corpse );
	idAFEntity_Gibbable*	focusCorpse;
	int						searchTimer;
	*/
// <---sikk

// sikk---> Object Manipulation
	/*
	idGrabEntity			grabEntity;
	idEntity*				focusMoveable;
	int						focusMoveableId;
	int						focusMoveableTimer;
	*/
// <---sikk

// sikk---> Adrenaline Pack System
	/*
	void					UseAdrenaline( void );
	int						adrenalineAmount;
	*/
// <---sikk

// sikk---> Health Management System
	/*
	void					UseHealthPack( void );
	int						healthPackAmount;
	int						healthPackTimer;
	int						nextHealthRegen;
	int						prevHeatlh;			// sikk - holds player health after Health station has been used
	*/
// <---sikk

// sikk---> Crosshair Positioning
	int						GetCurrentWeapon( void ) { return currentWeapon; };
	//idVec3				v3CrosshairPos;
// <---sikk

// sikk---> Weapon Management: Awareness
	/*
	bool					GetWeaponAwareness( void );
	bool					bWATrace;
	bool					bWAIsSprinting;
	bool					bWAUseHideDist;
	float					fSpreadModifier;
	idEntity*				entChainsawed;
	*/
// <---sikk

// sikk---> Depth Render
	void					ToggleSuppression( bool bSuppress );
	bool					bViewModelsModified;
// <---sikk

// sikk---> Depth of Field PostProcess
	int						GetTalkCursor( void ) { return talkCursor; };	// used to check if character has focus
	bool					bIsZoomed;
	float					focusDistance;
// <---sikk

// sikk---> Global Ambient Light
	void					ToggleAmbientLight( bool bOn );
	bool					bAmbientLightOn;
	idStr					szAmbientLightColor;
	idStr					szAmbientLightRadius;
// <---sikk

// sikk---> Infrared Goggles/Headlight Mod
	/*
	void					UpdateBattery( void );
	void					ToggleIRGoggles( void );
	void					ToggleHeadlight( void );

	bool					bIRGogglesOn;
	bool					bHeadlightOn;
	int						nIRGogglesTime;
	int						nHeadlightTime;
	int						nBattery;
	float					fIntensity;
	float					fIRBloomParms[ 7 ];
	*/
// <---sikk

private:
	jointHandle_t			hipJoint;
	jointHandle_t			chestJoint;
	jointHandle_t			headJoint;

	idPhysics_Player		physicsObj;			// player physics

	idList<aasLocation_t>	aasLocation;		// for AI tracking the player

	int						bobFoot;
	float					bobFrac;
	float					bobfracsin;
	int						bobCycle;			// for view bobbing and footstep generation
	float					xyspeed;
	int						stepUpTime;
	float					stepUpDelta;
	float					idealLegsYaw;
	float					legsYaw;
	bool					legsForward;
	float					oldViewYaw;
	idAngles				viewBobAngles;
	idVec3					viewBob;
	int						landChange;
	int						landTime;

	int						currentWeapon;
	int						idealWeapon;
	int						previousWeapon;
	int						weaponSwitchTime;
	bool					weaponEnabled;
	bool					showWeaponViewModel;

	const idDeclSkin *		skin;
	const idDeclSkin *		powerUpSkin;
	idStr					baseSkinName;

	int						numProjectilesFired;	// number of projectiles fired
	int						numProjectileHits;		// number of hits on mobs

	bool					airless;
	int						airTics;				// set to pm_airTics at start, drops in vacuum
	int						lastAirDamage;

	bool					gibDeath;
	bool					gibsLaunched;
	idVec3					gibsDir;

	idInterpolate<float>	zoomFov;
	idInterpolate<float>	centerView;
	bool					fxFov;
#ifdef _DT
	bool					isRunning;
	bool					changeLevelOK;	// _DT - Use this to skip 'UpdateHeroStats' in player Think, if changing level.
											// We need to skip it since it modifies some timed attributes, and that should not happen
											// when changing map. 'ClearDownTimedAttributes' is called before that method,
											// and these timed attributes should not be modified after attribute cleardown,
											// otherwise we would save wrong values.
											// Added this bool check so we can keep the same order in method call - changing
											// method call order in Think could be very dangerous, so we should avoid it.
	// gasp bubble particle -->
	const idDeclParticle *	smokeGasp;
	int						smokeGaspTime;
	// gasp bubble particle <--
#endif

	float					influenceFov;
	int						influenceActive;		// level of influence.. 1 == no gun or hud .. 2 == 1 + no movement
	idEntity *				influenceEntity;
	const idMaterial *		influenceMaterial;
	float					influenceRadius;
	const idDeclSkin *		influenceSkin;

	idCamera *				privateCameraView;

	static const int		NUM_LOGGED_VIEW_ANGLES = 64;		// for weapon turning angle offsets
	idAngles				loggedViewAngles[NUM_LOGGED_VIEW_ANGLES];	// [gameLocal.framenum&(LOGGED_VIEW_ANGLES-1)]
	static const int		NUM_LOGGED_ACCELS = 16;			// for weapon turning angle offsets
	loggedAccel_t			loggedAccel[NUM_LOGGED_ACCELS];	// [currentLoggedAccel & (NUM_LOGGED_ACCELS-1)]
	int						currentLoggedAccel;

	// if there is a focusGUIent, the attack button will be changed into mouse clicks
	idEntity *				focusGUIent;
	idUserInterface *		focusUI;				// focusGUIent->renderEntity.gui, gui2, or gui3
	idAI *					focusCharacter;
	int						talkCursor;				// show the state of the focusCharacter (0 == can't talk/dead, 1 == ready to talk, 2 == busy talking)
	int						focusTime;
	idAFEntity_Vehicle *	focusVehicle;
	idUserInterface *		cursor;
	
	// full screen guis track mouse movements directly
	int						oldMouseX;
	int						oldMouseY;

	idStr					pdaAudio;
	idStr					pdaVideo;
	idStr					pdaVideoWave;

	bool					tipUp;
	bool					objectiveUp;

	int						lastDamageDef;
	idVec3					lastDamageDir;
	int						lastDamageLocation;
	int						smoothedFrame;
	bool					smoothedOriginUpdated;
	idVec3					smoothedOrigin;
	idAngles				smoothedAngles;

	// mp
	bool					ready;					// from userInfo
	bool					respawning;				// set to true while in SpawnToPoint for telefrag checks
	bool					leader;					// for sudden death situations
	int						lastSpectateChange;
	int						lastTeleFX;
	unsigned int			lastSnapshotSequence;	// track state hitches on clients
	bool					weaponCatchup;			// raise up the weapon silently ( state catchups )
	int						MPAim;					// player num in aim
	int						lastMPAim;
	int						lastMPAimTime;			// last time the aim changed
	int						MPAimFadeTime;			// for GUI fade
	bool					MPAimHighlight;
	bool					isTelefragged;			// proper obituaries

	idPlayerIcon			playerIcon;

	bool					selfSmooth;

	idEntityPtr<idActor>	friendsCommonEnemy; //ivan

	void					LookAtKiller( idEntity *inflictor, idEntity *attacker );

	void					StopFiring( void );
	void					FireWeapon( void );
	void					Weapon_Combat( void );
	void					Weapon_NPC( void );
	void					Weapon_GUI( void );
	void					UpdateWeapon( void );
	void					UpdateSpectating( void );
	void					SpectateFreeFly( bool force );	// ignore the timeout to force when followed spec is no longer valid
	void					SpectateCycle( void );
	idAngles				GunTurningOffset( void );
	idVec3					GunAcceleratingOffset( void );

	void					UseObjects( void );
	void					CrashLand( const idVec3 &oldOrigin, const idVec3 &oldVelocity );
	void					BobCycle( const idVec3 &pushVelocity );
	void					UpdateViewAngles( void );
	void					EvaluateControls( void );
	void					AdjustSpeed( void );
	void					AdjustBodyAngles( void );
	void					InitAASLocation( void );
	void					SetAASLocation( void );
	void					Move( void );
	void					UpdatePowerUps( void );
	void					UpdateDeathSkin( bool state_hitch );
	void					ClearPowerup( int i );
	void					SetSpectateOrigin( void );

	void					ClearFocus( void );
	void					UpdateFocus( void );
	void					UpdateLocation( void );
	idUserInterface *		ActiveGui( void );
	void					UpdatePDAInfo( bool updatePDASel );
	int						AddGuiPDAData( const declType_t dataType, const char *listName, const idDeclPDA *src, idUserInterface *gui );
	void					ExtractEmailInfo( const idStr &email, const char *scan, idStr &out );
	void					UpdateObjectiveInfo( void );

	void					UseVehicle( void );

	void					Event_GetButtons( void );
	void					Event_GetMove( void );
	void					Event_GetViewAngles( void );
	void					Event_StopFxFov( void );
	void					Event_EnableWeapon( void );
	void					Event_DisableWeapon( void );
	void					Event_GetCurrentWeapon( void );
	void					Event_GetPreviousWeapon( void );
	void					Event_SelectWeapon( const char *weaponName );
	void					Event_GetWeaponEntity( void );
	void					Event_OpenPDA( void );
	void					Event_PDAAvailable( void );
	void					Event_InPDA( void );
	void					Event_ExitTeleporter( void );
	void					Event_HideTip( void );
	void					Event_LevelTrigger( void );
	void					Event_Gibbed( void );
	void					Event_GetIdealWeapon( void );

	//ivan start
	void					Event_ForceUpdateNpcStatus( void ); 
	void					Event_SetCommonEnemy( idEntity *enemy ); 
	void					Event_GetCommonEnemy( void );
	//ivan end

	/*
	*****************************************************************************************************
	*****************************************************************************************************
	*****************************************************************************************************
	*** BEGIN - Solarsplace - Arx EOS - Private
	*/

	// EVENTS
	void					Event_InventoryContainsItem( const char *itemName );				// 2nd Jan 2009 - Solarsplace
	void					Event_LevelTransitionSpawnPoint( const char *spawnPointEntName );	// 3rd Oct 2010 - Solarsplace
	void					Event_HudMessage( const char *message );							// 25th Sep 2011 - Solarsplace
	void					Event_SetFloatHUDParm( const char *key, float value );				// 6th Sep 2012 - Solarsplace		
	void					Event_PlayerMoney( int amount );									// 15th Oct 2011 - Solarsplace
	void					Event_OpenCloseShop( const char *newState );						// 30th Nov 2011 - Solarsplace
	void					Event_RemoveInventoryItem( const char *name );						// 29th Dec 2011 - Solarsplace
	bool					Event_GiveInventoryItem( const char *name );						// 29th Dec 2011 - Solarsplace // // _DT - modified return type from 'void' to 'bool'.
	void					Event_FindInventoryItemCount( const char *name );					// 2nd Jan 2011 - Solarsplace
	void					Event_GiveJournal( const char *name );								// 13th Sep 2012 - Solarsplace
	void					Event_GetMapName( void );											// 22nd Sep 2012 - Solarsplace
	void					Event_ModifyPlayerXPs( int XPs );
	void					Event_GetWeaponChargeTime( float baseTime );
	void					Event_GetWaterLevel( void );
	void					Event_ArxCheckPlayerInventoryFull( void );
	void					Event_HasGotJournal( const char *name );
#ifdef _DT
	void					Event_GetStaminaPercentage( void );
#endif
	void					Event_GiveLevelMap( const char *name );
	void					Event_GetEyePos( void );
	void					Event_GetSpellTelekinesisEndTime( void );
	void					Event_GetSpellTelekinesisDistance( void );

	// Inventory related
	void					TraceUsables( void );											// solarsplace 7th June 2010
	idStr					lastUsableName;													// solarsplace 7th June 2010
	bool					lastUsableTraceWasNothing;										// solarsplace 7th June 2010
	void					GetEntityByViewRay( void );										// solarsplace 1st Mar 2010
	void					PlaySpecificEntitySoundShader( idEntity *ent, const char *sndShaderName );	// solarsplace 1st Mar 2010
	bool					ConsumeInventoryItem( int invItemIndex );						// solarsplace 15th Apr 2010
	void					DropInventoryItem( int invItemIndex );							// solarsplace 15th Apr 2010
	idDict					*invItemGroupCount;												// solarsplace 24th Sep 2011
	idDict					*invItemGroupPointer;											// solarsplace 24th Sep 2011
	bool					ArxCheckPlayerInventoryFull( void );							// solarsplace 29th Sep 2015
	bool					ArxCheckPlayerInventoryFull( idDict &itemDict );				// doomtrinity 17th May 2016
	bool					ArxIsItemStackedInInventory( idDict &itemDict );				// doomtrinity 17th May 2016

	// Skills related
	void					CreateNewHero( void );											// solarsplace 10th Apr 2015
	void					ArxLoadSkillSpawnArgsIntoInventory( void );									// solarsplace 10th Apr 2015
	void					LoadCurrentSkillsIntoTemp( void );								// solarsplace 24th Apr 2015
	float					ArxGetAttributeSkillMatrix( int ArxAttribute, int ArxSkill );	// solarsplace 24th Apr 2015
	bool					ArxHasUnspentPoints( void );									// solarsplace 1st May 2015
	int						GetSpellBonus( int spell );										// solarsplace 24th Apr 2015
public: // _DT - need to access GetPercentageBonus outside player
	float					GetPercentageBonus( float BaseValue, float BonusPercentage );	// solarsplace 24th Apr 2015
private:	
	void					ArxUpdateHeroSkills( void );										// solarsplace 10th Apr 2015
	void					UpdateHeroStats( void );										// solarsplace 15th Mar 2013
	void					UpdateEquipedItems( void );										// solarsplace 08th Oct 2014
	int						GetRequiredXPForLevel( int level );
	int						ArxCalculatePlayerDamage( int baseDamageAmount, int damageType );
	float					ArxGetStatAsPercentage( int statType );

	// Magic related
	int						GetPlayerManaAmount( void );									// Solarsplace 28th Feb 2010
	//void					FireMagicWeapon( const char *projectileName, int quickSpellIndex, int manaCost );
	int						magicAttackTime;												// solarsplace 15th May 2010
	//bool					magicAttackInProgress;											// solarsplace 15th May 2010
	void					ProcessMagic();													// solarsplace 22nd May 2010 
	void					MagicUpdateJournalSpells( void );								// solarsplace 22nd Jul 2010
	void					ArxTraceAIHealthHUD( void );									// solarsplace 24th Sep 2014
	void					ArxSpawnMiscFoodItemIntoWorld( void );
	void					ArxGiveNewLevelMap( const char *mapFileSystemName, const char *mapName, const char *mapDescription, const char *mapImageFile );
	void					ArxTelekinesisEffect( void );

	// Shop related
	idArxShop				arxShopFunctions;
	int						ShopGetSellToPlayerPrice( float baseValue, float durabilityRatio, float shopRatio );	// solarsplace 14nd Jun 2014
	int						ShopGetBuyFromPlayerPrice( float baseValue, float durabilityRatio, float shopRatio );	// solarsplace 14nd Jun 2014

	// Spell casting related
	idEntity *				magicWand;
	idEntity *				magicWandTrail;
	idArxMisc				arxMiscFunctions;
	short					magicLastYPos;
	short					magicLastXPos;
	float					magicMoveAmountVertical;
	float					magicMoveAmountHorizontal;
	idVec2					magicStartVec;
	idVec2					magicEndVec;
	bool					magicInitVectors;
	idStr					magicCompassSequence;
	idStr					magicRuneSequence;
	int						magicLastCompassDir;
	int						magicIdleTime;
	idStr					hudMagicHelp;
	idStr					magicProjectileDef;
	int						magicPreDelay;
	//bool					magicDoingPreCastSpellProjectile;
	const char				*magicLatestProjectileDefName;
	//void					magicSaveSpell( int manaCost, const char *projectileName, const char *spellName );

	// Level transition related
	idStr					SplitStrings( idStr inputString, int requiredPart );
	void					SaveTransitionInfo( void );
	void					LoadTransitionInfo( void );
	void					DeleteTransitionInfoSpecific( idStr recordType, idStr entityName );
	int						GetTransitionKeyIndex( idStr recordType, idStr entityName );
	void					SpawnTransitionEntity( idStr entityName );

	// Spell related
	void					RadiusSpell( idStr scriptAction, float alertRadius );

	// Blacksmith related
	int						BlackSmithRepairComputeCost( int maxHealth, int currentHealth, int itemValue, float blackSmithMultiply );

	// Quest related
	bool					GetQuestState( idStr questObjectQuestName );

	// Searchable
	bool					GiveSearchItem( idEntity *searchTarget );

	// Misc
	void					ArxProcessTimedEvents( void );
	int						ArxNextProcessEvent;
	int						arxPlayingFreezing;

	/*
	*** END - Solarsplace - Arx EOS - Private
	*****************************************************************************************************
	*****************************************************************************************************
	*****************************************************************************************************
	*/


};

ID_INLINE bool idPlayer::IsReady( void ) {
	return ready || forcedReady;
}

ID_INLINE bool idPlayer::IsRespawning( void ) {
	return respawning;
}

ID_INLINE idPhysics* idPlayer::GetPlayerPhysics( void ) {
	return &physicsObj;
}

ID_INLINE bool idPlayer::IsInTeleport( void ) {
	return ( teleportEntity.GetEntity() != NULL );
}

ID_INLINE void idPlayer::SetLeader( bool lead ) {
	leader = lead;
}

ID_INLINE bool idPlayer::IsLeader( void ) {
	return leader;
}

ID_INLINE bool idPlayer::SelfSmooth( void ) {
	return selfSmooth;
}

ID_INLINE void idPlayer::SetSelfSmooth( bool b ) {
	selfSmooth = b;
}

#endif /* !__GAME_PLAYER_H__ */

