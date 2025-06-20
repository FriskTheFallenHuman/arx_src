// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_ACTOR_H__
#define __GAME_ACTOR_H__

/*
===============================================================================

	idActor

===============================================================================
*/

extern const idEventDef AI_EnableEyeFocus;
extern const idEventDef AI_DisableEyeFocus;
extern const idEventDef EV_Footstep;
extern const idEventDef EV_FootstepLeft;
extern const idEventDef EV_FootstepRight;
extern const idEventDef EV_EnableWalkIK;
extern const idEventDef EV_DisableWalkIK;
extern const idEventDef EV_EnableLegIK;
extern const idEventDef EV_DisableLegIK;
extern const idEventDef AI_SetAnimPrefix;
extern const idEventDef AI_PlayAnim;
extern const idEventDef AI_PlayCycle;
extern const idEventDef AI_AnimDone;
extern const idEventDef AI_SetBlendFrames;
extern const idEventDef AI_GetBlendFrames;
extern const idEventDef AI_SpawnItem;	// Solarsplace 18th July 2011 - Spawn things a few seconds after death
extern const idEventDef AI_SpawnItemCooked;	// Solarsplace 18th July 2011 - Spawn cooked body parts a few seconds after death
extern const idEventDef AI_ArxUpdateTeam; // SP - 25th July 2015

class idDeclParticle;

class idAnimState {
public:
	bool					idleAnim;
	idStr					state;
	int						animBlendFrames;
	int						lastAnimBlendFrames;		// allows override anims to blend based on the last transition time

public:
							idAnimState();
							~idAnimState();

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	void					Init( idActor *owner, idAnimator *_animator, int animchannel );
	void					Shutdown( void );
	void					SetState( const char *name, int blendFrames );
	void					StopAnim( int frames );
	void					PlayAnim( int anim );
	void					CycleAnim( int anim );
	void					BecomeIdle( void );
	bool					UpdateState( void );
	bool					Disabled( void ) const;
	void					Enable( int blendFrames );
	void					Disable( void );
	bool					AnimDone( int blendFrames ) const;
	bool					IsIdle( void ) const;
	animFlags_t				GetAnimFlags( void ) const;

private:
	idActor *				self;
	idAnimator *			animator;
	idThread *				thread;
	int						channel;
	bool					disabled;
};

class idAttachInfo {
public:
	idEntityPtr<idEntity>	ent;
	int						channel;
};

typedef struct {
	jointModTransform_t		mod;
	jointHandle_t			from;
	jointHandle_t			to;
} copyJoints_t;

class idActor : public idAFEntity_Gibbable {
public:
	CLASS_PROTOTYPE( idActor );

	int						team;
	int						rank;				// monsters don't fight back if the attacker's rank is higher
	idMat3					viewAxis;			// view axis of the actor

	idLinkList<idActor>		enemyNode;			// node linked into an entity's enemy list for quick lookups of who is attacking him
	idLinkList<idActor>		enemyList;			// list of characters that have targeted the player as their enemy

public:
							idActor( void );
	virtual					~idActor( void );

	void					Spawn( void );
	virtual void			Restart( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Hide( void );
	virtual void			Show( void );
	virtual int				GetDefaultSurfaceType( void ) const;
#ifdef _DT // decal angle
	virtual void			ProjectOverlay( const idVec3 &origin, const idVec3 &dir, float size, const char *material, float angle = 0 );
#else
	virtual void			ProjectOverlay( const idVec3 &origin, const idVec3 &dir, float size, const char *material );
#endif

	virtual bool			LoadAF( void );
	void					SetupBody( void );

	void					CheckBlink( void );

	virtual bool			GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis );
	virtual bool			GetPhysicsToSoundTransform( idVec3 &origin, idMat3 &axis );

							// script state management
	void					ShutdownThreads( void );
	virtual bool			ShouldConstructScriptObjectAtSpawn( void ) const;
	virtual idThread *		ConstructScriptObject( void );
	void					UpdateScript( void );
	const function_t		*GetScriptFunction( const char *funcname );
	void					SetState( const function_t *newState );
	void					SetState( const char *statename );

							// vision testing
	void					SetEyeHeight( float height );
	float					EyeHeight( void ) const;
	idVec3					EyeOffset( void ) const;
	idVec3					GetEyePosition( void ) const;
	virtual void			GetViewPos( idVec3 &origin, idMat3 &axis ) const;
	void					SetFOV( float fov );
	bool					CheckFOV( const idVec3 &pos ) const;
	bool					CanSee( idEntity *ent, bool useFOV ) const;
	bool					PointVisible( const idVec3 &point ) const;
	virtual void			GetAIAimTargets( const idVec3 &lastSightPos, idVec3 &headPos, idVec3 &chestPos );

							// damage
	void					SetupDamageGroups( void );
	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, const idVec3 &dir, const char *damageDefName, const float damageScale, const int location );
	int						GetDamageForLocation( int damage, int location );
	const char *			GetDamageGroup( int location );
	void					ClearPain( void );
	virtual bool			Pain( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );

							// model/combat model/ragdoll
	void					SetCombatModel( void );
	idClipModel *			GetCombatModel( void ) const;
	virtual void			LinkCombat( void );
	virtual void			UnlinkCombat( void );
	bool					StartRagdoll( void );
	void					StopRagdoll( void );
	virtual bool			UpdateAnimationControllers( void );

							// delta view angles to allow movers to rotate the view of the actor
	const idAngles &		GetDeltaViewAngles( void ) const;
	void					SetDeltaViewAngles( const idAngles &delta );

	bool					HasEnemies( void ) const;
	idActor *				ClosestEnemyToPoint( const idVec3 &pos );
	idActor *				EnemyWithMostHealth();

	virtual bool			OnLadder( void ) const;

	virtual void			GetAASLocation( idAAS *aas, idVec3 &pos, int &areaNum ) const;

	void					Attach( idEntity *ent );

	virtual void			Teleport( const idVec3 &origin, const idAngles &angles, idEntity *destination );

	virtual	renderView_t *	GetRenderView();	
	
							// animation state control
	int						GetAnim( int channel, const char *name );
	void					UpdateAnimState( void );
	void					SetAnimState( int channel, const char *name, int blendFrames );
	const char *			GetAnimState( int channel ) const;
	bool					InAnimState( int channel, const char *name ) const;
	const char *			WaitState( void ) const;
	void					SetWaitState( const char *_waitstate );
	bool					AnimDone( int channel, int blendFrames ) const;
	virtual void			SpawnGibs( const idVec3 &dir, const char *damageDefName );

protected:
	friend class			idAnimState;

	float					fovDot;				// cos( fovDegrees )
	idVec3					eyeOffset;			// offset of eye relative to physics origin
	idVec3					modelOffset;		// offset of visual model relative to the physics origin
	idVec3					mHeadModelOffset;	// offset for the head - SP 14th Sep 2011 Arx EOS - From TDM

	idAngles				deltaViewAngles;	// delta angles relative to view input angles

	int						pain_debounce_time;	// next time the actor can show pain
	int						pain_delay;			// time between playing pain sound
	int						pain_threshold;		// how much damage monster can take at any one time before playing pain animation

	idStrList				damageGroups;		// body damage groups
	idList<float>			damageScale;		// damage scale per damage gruop

	bool						use_combat_bbox;	// whether to use the bounding box for combat collision
	idEntityPtr<idAFAttachment>	head;
	idList<copyJoints_t>		copyJoints;			// copied from the body animation to the head model

	// state variables
	const function_t		*state;
	const function_t		*idealState;

	// joint handles
	jointHandle_t			leftEyeJoint;
	jointHandle_t			rightEyeJoint;
	jointHandle_t			soundJoint;

	idIK_Walk				walkIK;

	idStr					animPrefix;
	idStr					painAnim;

	// blinking
	int						blink_anim;
	int						blink_time;
	int						blink_min;
	int						blink_max;

	// script variables
	idThread *				scriptThread;
	idStr					waitState;
	idAnimState				headAnim;
	idAnimState				torsoAnim;
	idAnimState				legsAnim;

	bool					allowPain;
	bool					allowEyeFocus;
	bool					finalBoss;

	int						painTime;

	idList<idAttachInfo>	attachments;

	virtual void			Gib( const idVec3 &dir, const char *damageDefName );

							// removes attachments with "remove" set for when character dies
	void					RemoveAttachments( void );

							// copies animation from body to head joints
	void					CopyJointsFromBodyToHead( void );

							// Arx - End Of Sun
	idDict					allSounds;					

private:

	// Solarslace
	bool					spashSoundChannel;
	int						spashSoundTime;

	void					SyncAnimChannels( int channel, int syncToChannel, int blendFrames );
	void					FinishSetup( void );
	void					SetupHead( void );
	void					PlayFootStepSound( void );

	void					Event_EnableEyeFocus( void );
	void					Event_DisableEyeFocus( void );
	void					Event_Footstep( void );
	void					Event_EnableWalkIK( void );
	void					Event_DisableWalkIK( void );
	void					Event_EnableLegIK( int num );
	void					Event_DisableLegIK( int num );
	void					Event_SetAnimPrefix( const char *name );
	void					Event_LookAtEntity( idEntity *ent, float duration );
	void					Event_PreventPain( float duration );
	void					Event_DisablePain( void );
	void					Event_EnablePain( void );
	void					Event_GetPainAnim( void );
	void					Event_StopAnim( int channel, int frames );
	void					Event_PlayAnim( int channel, const char *name );
	void					Event_PlayCycle( int channel, const char *name );
	void					Event_IdleAnim( int channel, const char *name );
	void					Event_SetSyncedAnimWeight( int channel, int anim, float weight );
	void					Event_OverrideAnim( int channel );
	void					Event_EnableAnim( int channel, int blendFrames );
	void					Event_SetBlendFrames( int channel, int blendFrames );
	void					Event_GetBlendFrames( int channel );
	void					Event_AnimState( int channel, const char *name, int blendFrames );
	void					Event_GetAnimState( int channel );
	void					Event_InAnimState( int channel, const char *name );
	void					Event_FinishAction( const char *name );
	void					Event_AnimDone( int channel, int blendFrames );
	void					Event_HasAnim( int channel, const char *name );
	void					Event_CheckAnim( int channel, const char *animname );
	void					Event_ChooseAnim( int channel, const char *animname );
	void					Event_AnimLength( int channel, const char *animname );
	void					Event_AnimDistance( int channel, const char *animname );
	void					Event_HasEnemies( void );
	void					Event_NextEnemy( idEntity *ent );
	void					Event_ClosestEnemyToPoint( const idVec3 &pos );
	void					Event_StopSound( int channel, int netsync );
	void					Event_SetNextState( const char *name );
	void					Event_SetState( const char *name );
	void					Event_GetState( void );
	void					Event_GetHead( void );
	void					Event_SpawnItem( void );	// Solarsplace 18th July 2011 - Spawn things a few seconds after death
	void					Event_SpawnItemCooked( const char *name );	// Solarsplace 18th July 2011 - Spawn cooked body parts a few seconds after death
	void					Event_ArxUpdateTeam( int newTeam ); // SP 27th July 2015
};

#endif /* !__GAME_ACTOR_H__ */
