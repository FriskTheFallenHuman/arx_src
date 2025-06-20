// Copyright (C) 2004 Id Software, Inc.
//

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"


/*
===============================================================================

  idTrigger
	
===============================================================================
*/

const idEventDef EV_Enable( "enable", NULL );
const idEventDef EV_Disable( "disable", NULL );

CLASS_DECLARATION( idEntity, idTrigger )
	EVENT( EV_Enable,	idTrigger::Event_Enable )
	EVENT( EV_Disable,	idTrigger::Event_Disable )
END_CLASS

/*
================
idTrigger::DrawDebugInfo
================
*/
void idTrigger::DrawDebugInfo( void ) {
	idMat3		axis = gameLocal.GetLocalPlayer()->viewAngles.ToMat3();
	idVec3		up = axis[ 2 ] * 5.0f;
	idBounds	viewTextBounds( gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin() );
	idBounds	viewBounds( gameLocal.GetLocalPlayer()->GetPhysics()->GetOrigin() );
	idBounds	box( idVec3( -4.0f, -4.0f, -4.0f ), idVec3( 4.0f, 4.0f, 4.0f ) );
	idEntity	*ent;
	idEntity	*target;
	int			i;
	bool		show;
	const function_t *func;

	viewTextBounds.ExpandSelf( 128.0f );
	viewBounds.ExpandSelf( 512.0f );
	for( ent = gameLocal.spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next() ) {
		if ( ent->GetPhysics()->GetContents() & ( CONTENTS_TRIGGER | CONTENTS_FLASHLIGHT_TRIGGER ) ) {
			show = viewBounds.IntersectsBounds( ent->GetPhysics()->GetAbsBounds() );
			if ( !show ) {
				for( i = 0; i < ent->targets.Num(); i++ ) {
					target = ent->targets[ i ].GetEntity();
					if ( target && viewBounds.IntersectsBounds( target->GetPhysics()->GetAbsBounds() ) ) {
						show = true;
						break;
					}
				}
			}

			if ( !show ) {
				continue;
			}

			gameRenderWorld->DebugBounds( colorOrange, ent->GetPhysics()->GetAbsBounds() );
			if ( viewTextBounds.IntersectsBounds( ent->GetPhysics()->GetAbsBounds() ) ) {
				gameRenderWorld->DrawText( ent->name.c_str(), ent->GetPhysics()->GetAbsBounds().GetCenter(), 0.1f, colorWhite, axis, 1 );
				gameRenderWorld->DrawText( ent->GetEntityDefName(), ent->GetPhysics()->GetAbsBounds().GetCenter() + up, 0.1f, colorWhite, axis, 1 );
				if ( ent->IsType( idTrigger::Type ) ) {
					func = static_cast<idTrigger *>( ent )->GetScriptFunction();
				} else {
					func = NULL;
				}

				if ( func ) {
					gameRenderWorld->DrawText( va( "call script '%s'", func->Name() ), ent->GetPhysics()->GetAbsBounds().GetCenter() - up, 0.1f, colorWhite, axis, 1 );
				}
			}

			for( i = 0; i < ent->targets.Num(); i++ ) {
				target = ent->targets[ i ].GetEntity();
				if ( target ) {
					gameRenderWorld->DebugArrow( colorYellow, ent->GetPhysics()->GetAbsBounds().GetCenter(), target->GetPhysics()->GetOrigin(), 10, 0 );
					gameRenderWorld->DebugBounds( colorGreen, box, target->GetPhysics()->GetOrigin() );
					if ( viewTextBounds.IntersectsBounds( target->GetPhysics()->GetAbsBounds() ) ) {
						gameRenderWorld->DrawText( target->name.c_str(), target->GetPhysics()->GetAbsBounds().GetCenter(), 0.1f, colorWhite, axis, 1 );
					}
				}
			}
		}
	}
}

/*
================
idTrigger::Enable
================
*/
void idTrigger::Enable( void ) {
	GetPhysics()->SetContents( CONTENTS_TRIGGER );
	GetPhysics()->EnableClip();

	// Solarsplace - Arx End Of Sun
	isEnabled = true;

	//REMOVEDx
	//gameLocal.Printf( "Trigger (%s) is enabled\n", this->name.c_str() );
}

/*
================
idTrigger::Disable
================
*/
void idTrigger::Disable( void ) {
	// we may be relinked if we're bound to another object, so clear the contents as well
	GetPhysics()->SetContents( 0 );
	GetPhysics()->DisableClip();

	// Solarsplace - Arx End Of Sun
	isEnabled = false;

	//REMOVEDx
	//gameLocal.Printf( "Trigger (%s) is disabled\n", this->name.c_str() );
}

/*
================
idTrigger::ArxIsEnabled
================
*/
bool idTrigger::ArxIsEnabled( void ) {
	// Solarsplace - Arx End Of Sun
	return isEnabled;
}

/*
================
idTrigger::CallScript
================
*/
void idTrigger::CallScript( void ) const {
	idThread *thread;

	if ( scriptFunction ) {
		thread = new idThread( scriptFunction );
		thread->DelayedStart( 0 );
	}
}

/*
================
idTrigger::GetScriptFunction
================
*/
const function_t *idTrigger::GetScriptFunction( void ) const {
	return scriptFunction;
}

/*
================
idTrigger::Save
================
*/
void idTrigger::Save( idSaveGame *savefile ) const {
	if ( scriptFunction ) {
		savefile->WriteString( scriptFunction->Name() );
	} else {
		savefile->WriteString( "" );
	}
}

/*
================
idTrigger::Restore
================
*/
void idTrigger::Restore( idRestoreGame *savefile ) {
	idStr funcname;
	savefile->ReadString( funcname );

	if ( funcname.Length() ) {
		scriptFunction = gameLocal.program.FindFunction( funcname );
		if ( scriptFunction == NULL ) {
			gameLocal.Warning( "idTrigger_Multi '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
		}
	} else {
		scriptFunction = NULL;
	}
}

/*
================
idTrigger::Event_Enable
================
*/
void idTrigger::Event_Enable( void ) {
	Enable();
}

/*
================
idTrigger::Event_Disable
================
*/
void idTrigger::Event_Disable( void ) {
	Disable();
}

/*
================
idTrigger::idTrigger
================
*/
idTrigger::idTrigger() {
	scriptFunction = NULL;
}

/*
================
idTrigger::Spawn
================
*/
void idTrigger::Spawn( void ) {

	//REMOVEDx
	//gameLocal.Printf( "idTrigger::Spawn\n" );

	GetPhysics()->SetContents( CONTENTS_TRIGGER );

	idStr funcname = spawnArgs.GetString( "call", "" );
	if ( funcname.Length() ) {
		scriptFunction = gameLocal.program.FindFunction( funcname );
		if ( scriptFunction == NULL ) {
			gameLocal.Warning( "trigger '%s' at (%s) calls unknown function '%s'", name.c_str(), GetPhysics()->GetOrigin().ToString(0), funcname.c_str() );
		}
	} else {
		scriptFunction = NULL;
	}
}


/*
===============================================================================

  idTrigger_Multi
	
===============================================================================
*/

const idEventDef EV_TriggerAction( "<triggerAction>", "e" );

CLASS_DECLARATION( idTrigger, idTrigger_Multi )
	EVENT( EV_Touch,			idTrigger_Multi::Event_Touch )
	EVENT( EV_Activate,			idTrigger_Multi::Event_Trigger )
	EVENT( EV_TriggerAction,	idTrigger_Multi::Event_TriggerAction )
END_CLASS


/*
================
idTrigger_Multi::idTrigger_Multi
================
*/
idTrigger_Multi::idTrigger_Multi( void ) {
	wait = 0.0f;
	random = 0.0f;
	delay = 0.0f;
	random_delay = 0.0f;
	nextTriggerTime = 0;
	removeItem = 0;
	touchClient = false;
	touchOther = false;
	triggerFirst = false;
	triggerWithSelf = false;

	// Solarsplace - Arx End Of Sun
	requirementWeight = 0.0f;
	triggerEnabled = false;
	oldTriggerEnabled = false;
}

/*
================
idTrigger_Multi::Save
================
*/
void idTrigger_Multi::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( wait );
	savefile->WriteFloat( random );
	savefile->WriteFloat( delay );
	savefile->WriteFloat( random_delay );
	savefile->WriteInt( nextTriggerTime );
	savefile->WriteString( requires );
	savefile->WriteInt( removeItem );
	savefile->WriteBool( touchClient );
	savefile->WriteBool( touchOther );
	savefile->WriteBool( triggerFirst );
	savefile->WriteBool( triggerWithSelf );
	savefile->WriteFloat( requirementWeight );

	// Solarsplace - Arx End Of Sun
	savefile->WriteBool( triggerEnabled );
	savefile->WriteBool( oldTriggerEnabled );
	savefile->WriteBool( isEnabled );
}

/*
================
idTrigger_Multi::Restore
================
*/
void idTrigger_Multi::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( wait );
	savefile->ReadFloat( random );
	savefile->ReadFloat( delay );
	savefile->ReadFloat( random_delay );
	savefile->ReadInt( nextTriggerTime );
	savefile->ReadString( requires );
	savefile->ReadInt( removeItem );
	savefile->ReadBool( touchClient );
	savefile->ReadBool( touchOther );
	savefile->ReadBool( triggerFirst );
	savefile->ReadBool( triggerWithSelf );
	savefile->ReadFloat( requirementWeight );

	// Solarsplace - Arx End Of Sun
	savefile->ReadBool( triggerEnabled );
	savefile->ReadBool( oldTriggerEnabled );
	savefile->ReadBool( isEnabled );
}

/*
================
idTrigger_Multi::Spawn

"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"call" : Script function to call when triggered
"random"	wait variance, default is 0
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
================
*/
void idTrigger_Multi::Spawn( void ) {
	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetFloat( "random", "0", random );
	spawnArgs.GetFloat( "delay", "0", delay );
	spawnArgs.GetFloat( "random_delay", "0", random_delay );
	
	if ( random && ( random >= wait ) && ( wait >= 0 ) ) {
		random = wait - 1;
		gameLocal.Warning( "idTrigger_Multi '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( random_delay && ( random_delay >= delay ) && ( delay >= 0 ) ) {
		random_delay = delay - 1;
		gameLocal.Warning( "idTrigger_Multi '%s' at (%s) has random_delay >= delay", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	spawnArgs.GetString( "requires", "", requires );
	spawnArgs.GetInt( "removeItem", "0", removeItem );
	spawnArgs.GetBool( "triggerFirst", "0", triggerFirst );
	spawnArgs.GetBool( "triggerWithSelf", "0", triggerWithSelf );

	if ( spawnArgs.GetBool( "anyTouch" ) ) {
		touchClient = true;
		touchOther = true;
	} else if ( spawnArgs.GetBool( "noTouch" ) ) {
		touchClient = false;
		touchOther = false;
	} else if ( spawnArgs.GetBool( "noClient" ) ) {
		touchClient = false;
		touchOther = true;
	} else {
		touchClient = true;
		touchOther = false;
	}

	nextTriggerTime = 0;

	if ( spawnArgs.GetBool( "flashlight_trigger" ) ) {
		GetPhysics()->SetContents( CONTENTS_FLASHLIGHT_TRIGGER );
	} else {
		GetPhysics()->SetContents( CONTENTS_TRIGGER );
	}

	// Start -> Solarsplace - Arx End Of Sun
	triggerEnabled = true;

	isEnabled = true;

	spawnArgs.GetFloat( "requirementWeight", "0.0", requirementWeight );
	if ( requirementWeight > 0.0f ) {

		// Here we basically disable the 'touch' event and process touching via think instead.

		// get the clip model
		clipModel = new idClipModel( GetPhysics()->GetClipModel() );

		// remove the collision model from the physics object
		GetPhysics()->SetClipModel( NULL, 1.0f );

		triggerEnabled = true;
		oldTriggerEnabled = !triggerEnabled;

		// Start thinking
		BecomeActive( TH_THINK );
	}
	// End -> Solarsplace - Arx End Of Sun
}

/*
================
idTrigger_Multi::Think
================
*/
void idTrigger_Multi::Think( void ) {

	// Solarsplace - Arx End Of Sun
	if ( requirementWeight > 0.0f ) {
		Event_Touch( this, NULL );
	}
}

/*
================
idTrigger_Multi::CheckFacing
================
*/
bool idTrigger_Multi::CheckFacing( idEntity *activator ) {
	if ( spawnArgs.GetBool( "facing" ) ) {
		if ( !activator->IsType( idPlayer::Type ) ) {
			return true;
		}
		idPlayer *player = static_cast< idPlayer* >( activator );
		float dot = player->viewAngles.ToForward() * GetPhysics()->GetAxis()[0];
		float angle = RAD2DEG( idMath::ACos( dot ) );
		if ( angle  > spawnArgs.GetFloat( "angleLimit", "30" ) ) {
			return false;
		}
	}
	return true;
}


/*
================
idTrigger_Multi::TriggerAction
================
*/
void idTrigger_Multi::TriggerAction( idEntity *activator ) {
	ActivateTargets( triggerWithSelf ? this : activator );
	CallScript();

	// Solarsplace - Arx End Of Sun
	idPlayer * player = gameLocal.GetLocalPlayer();

	// Solarsplace - Arx End Of Sun - Cold World
	if ( activator == player ) {
		bool warmthTrigger = spawnArgs.GetBool( "arx_warmth_trigger", "0" );
		if ( warmthTrigger ) {
			const int WARM_DELAY = 2; // trigger_arx_warmth "wait" = "1"
			player->inventory.arx_timer_player_warmth = gameLocal.time + SEC2MS(WARM_DELAY); 
		}
	}

	if ( wait >= 0 ) {
		nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
	} else {

		// Solarsplace - Arx End Of Sun - Level transition related.
		gameLocal.GetLocalPlayer()->SaveTransitionInfoSpecific( this, false, true );

		// SP - 4th July 2013 - Secret area found?
		if ( activator == player ) {
			bool secretArea = spawnArgs.GetBool( "arx_secret_area", "0" );
			if ( secretArea ) {
				// Increment the player secrets found total
				player->inventory.arx_stat_secrets_found ++;
				const idSoundShader *shader = declManager->FindSound( spawnArgs.GetString( "snd_arx_secret_area" ) );
				player->StartSoundShader( shader, SND_CHANNEL_ANY, SSF_GLOBAL, false, NULL );
				player->ShowHudMessage( "#str_general_00010" );	// "You found a secret area"
				player->ModifyPlayerXPs( spawnArgs.GetInt( "arx_xp_value", "0"), false ); // Award XPs
			}
		}

		// we can't just remove (this) here, because this is a touch function
		// called while looping through area links...
		nextTriggerTime = gameLocal.time + 1;
		PostEventMS( &EV_Remove, 0 );
	}
}

/*
================
idTrigger_Multi::Event_TriggerAction
================
*/
void idTrigger_Multi::Event_TriggerAction( idEntity *activator ) {
	TriggerAction( activator );
}

/*
================
idTrigger_Multi::Event_Trigger

the trigger was just activated
activated should be the entity that originated the activation sequence (ie. the original target)
activator should be set to the activator so it can be held through a delay
so wait for the delay time before firing
================
*/
void idTrigger_Multi::Event_Trigger( idEntity *activator ) {
	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	// see if this trigger requires an item
	if ( !gameLocal.RequirementMet( activator, requires, removeItem ) ) {
		return;
	}

	if ( !CheckFacing( activator ) ) {
		return;
	}

	if ( triggerFirst ) {
		triggerFirst = false;
		return;
	}

	// don't allow it to trigger twice in a single frame
	nextTriggerTime = gameLocal.time + 1;

	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, activator );
	} else {
		TriggerAction( activator );
	}
}

/*
================
idTrigger_Multi::Event_Touch
================
*/
void idTrigger_Multi::Event_Touch( idEntity *other, trace_t *trace ) {

	if( triggerFirst ) {
		return;
	}

	bool player = other->IsType( idPlayer::Type );
	if ( player ) {
		if ( !touchClient ) {
			return;
		}
		if ( static_cast< idPlayer * >( other )->spectating ) {
			return;
		}
	} else if ( !touchOther ) {
		return;
	}

	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	// see if this trigger requires an item
	if ( !gameLocal.RequirementMet( other, requires, removeItem ) ) {
		return;
	}

	if ( !CheckFacing( other ) ) {
		return;
	}

	if ( spawnArgs.GetBool( "toggleTriggerFirst" ) ) {
		triggerFirst = true;
	}

	// Start -> Solarsplace - Arx End Of Sun
	if ( requirementWeight > 0.0f ) {

		if ( TouchingWeights() >= requirementWeight ) {
			triggerEnabled = true;
		} else {
			triggerEnabled = false;
		}

		if ( triggerEnabled == oldTriggerEnabled ) {
			return;
		} else {
			oldTriggerEnabled = triggerEnabled;
		}
	}
	// End -> Solarsplace - Arx End Of Sun

	nextTriggerTime = gameLocal.time + 1;
	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, other );
	} else {
		TriggerAction( other );
	}
}

/*
================
idTrigger_Multi::TouchingWeights
================
*/
float idTrigger_Multi::TouchingWeights( void ) {

	int numClipModels, i;
	idBounds bounds;
	idClipModel *cm, *clipModelList[ MAX_GENTITIES ];
	float totalWeightInTrigger = 0.0f;

	if ( clipModel == NULL ) {
		return 0.0f;
	}

	bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );
	numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	for ( i = 0; i < numClipModels; i++ ) {
		cm = clipModelList[ i ];

		if ( !cm->IsTraceModel() ) {
			continue;
		}

		idEntity *entity = cm->GetEntity();

		if ( !entity ) {
			continue;
		}
		
		if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
									clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
			continue;
		}

		totalWeightInTrigger += entity->spawnArgs.GetFloat( "inv_arx_weight", "0.0" );
	}

	return totalWeightInTrigger;
}

/*
===============================================================================

  idTrigger_EntityName
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_EntityName )
	EVENT( EV_Touch,			idTrigger_EntityName::Event_Touch )
	EVENT( EV_Activate,			idTrigger_EntityName::Event_Trigger )
	EVENT( EV_TriggerAction,	idTrigger_EntityName::Event_TriggerAction )
END_CLASS

/*
================
idTrigger_EntityName::idTrigger_EntityName
================
*/
idTrigger_EntityName::idTrigger_EntityName( void ) {
	wait = 0.0f;
	random = 0.0f;
	delay = 0.0f;
	random_delay = 0.0f;
	nextTriggerTime = 0;
	triggerFirst = false;
}

/*
================
idTrigger_EntityName::Save
================
*/
void idTrigger_EntityName::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( wait );
	savefile->WriteFloat( random );
	savefile->WriteFloat( delay );
	savefile->WriteFloat( random_delay );
	savefile->WriteInt( nextTriggerTime );
	savefile->WriteBool( triggerFirst );
	savefile->WriteString( entityName );
}

/*
================
idTrigger_EntityName::Restore
================
*/
void idTrigger_EntityName::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( wait );
	savefile->ReadFloat( random );
	savefile->ReadFloat( delay );
	savefile->ReadFloat( random_delay );
	savefile->ReadInt( nextTriggerTime );
	savefile->ReadBool( triggerFirst );
	savefile->ReadString( entityName );
}

/*
================
idTrigger_EntityName::Spawn
================
*/
void idTrigger_EntityName::Spawn( void ) {
	spawnArgs.GetFloat( "wait", "0.5", wait );
	spawnArgs.GetFloat( "random", "0", random );
	spawnArgs.GetFloat( "delay", "0", delay );
	spawnArgs.GetFloat( "random_delay", "0", random_delay );
	
	if ( random && ( random >= wait ) && ( wait >= 0 ) ) {
		random = wait - 1;
		gameLocal.Warning( "idTrigger_EntityName '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( random_delay && ( random_delay >= delay ) && ( delay >= 0 ) ) {
		random_delay = delay - 1;
		gameLocal.Warning( "idTrigger_EntityName '%s' at (%s) has random_delay >= delay", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	spawnArgs.GetBool( "triggerFirst", "0", triggerFirst );

	entityName = spawnArgs.GetString( "entityname" );
	if ( !entityName.Length() ) {
		gameLocal.Error( "idTrigger_EntityName '%s' at (%s) doesn't have 'entityname' key specified", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	nextTriggerTime = 0;

	if ( !spawnArgs.GetBool( "noTouch" ) ) {
		GetPhysics()->SetContents( CONTENTS_TRIGGER );
	}
}

/*
================
idTrigger_EntityName::TriggerAction
================
*/
void idTrigger_EntityName::TriggerAction( idEntity *activator ) {
	ActivateTargets( activator );
	CallScript();

	if ( wait >= 0 ) {
		nextTriggerTime = gameLocal.time + SEC2MS( wait + random * gameLocal.random.CRandomFloat() );
	} else {

		// Solarsplace - Arx End Of Sun - Level transition related.
		gameLocal.GetLocalPlayer()->SaveTransitionInfoSpecific( this, false, true );

		// we can't just remove (this) here, because this is a touch function
		// called while looping through area links...
		nextTriggerTime = gameLocal.time + 1;
		PostEventMS( &EV_Remove, 0 );
	}
}

/*
================
idTrigger_EntityName::Event_TriggerAction
================
*/
void idTrigger_EntityName::Event_TriggerAction( idEntity *activator ) {
	TriggerAction( activator );
}

/*
================
idTrigger_EntityName::Event_Trigger

the trigger was just activated
activated should be the entity that originated the activation sequence (ie. the original target)
activator should be set to the activator so it can be held through a delay
so wait for the delay time before firing
================
*/
void idTrigger_EntityName::Event_Trigger( idEntity *activator ) {
	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	if ( !activator || ( activator->name != entityName ) ) {
		return;
	}

	if ( triggerFirst ) {
		triggerFirst = false;
		return;
	}

	// don't allow it to trigger twice in a single frame
	nextTriggerTime = gameLocal.time + 1;

	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, activator );
	} else {
		TriggerAction( activator );
	}
}

/*
================
idTrigger_EntityName::Event_Touch
================
*/
void idTrigger_EntityName::Event_Touch( idEntity *other, trace_t *trace ) {
	if( triggerFirst ) {
		return;
	}

	if ( nextTriggerTime > gameLocal.time ) {
		// can't retrigger until the wait is over
		return;
	}

	if ( !other || ( other->name != entityName ) ) {
		return;
	}

	nextTriggerTime = gameLocal.time + 1;
	if ( delay > 0 ) {
		// don't allow it to trigger again until our delay has passed
		nextTriggerTime += SEC2MS( delay + random_delay * gameLocal.random.CRandomFloat() );
		PostEventSec( &EV_TriggerAction, delay, other );
	} else {
		TriggerAction( other );
	}
}

/*
===============================================================================

  idTrigger_Timer
	
===============================================================================
*/

const idEventDef EV_Timer( "<timer>", NULL );

CLASS_DECLARATION( idTrigger, idTrigger_Timer )
	EVENT( EV_Timer,		idTrigger_Timer::Event_Timer )
	EVENT( EV_Activate,		idTrigger_Timer::Event_Use )
END_CLASS

/*
================
idTrigger_Timer::idTrigger_Timer
================
*/
idTrigger_Timer::idTrigger_Timer( void ) {
	random = 0.0f;
	wait = 0.0f;
	on = false;
	delay = 0.0f;
}

/*
================
idTrigger_Timer::Save
================
*/
void idTrigger_Timer::Save( idSaveGame *savefile ) const {
	savefile->WriteFloat( random );
	savefile->WriteFloat( wait );
	savefile->WriteBool( on );
	savefile->WriteFloat( delay );
	savefile->WriteString( onName );
	savefile->WriteString( offName );
}

/*
================
idTrigger_Timer::Restore
================
*/
void idTrigger_Timer::Restore( idRestoreGame *savefile ) {
	savefile->ReadFloat( random );
	savefile->ReadFloat( wait );
	savefile->ReadBool( on );
	savefile->ReadFloat( delay );
	savefile->ReadString( onName );
	savefile->ReadString( offName );
}

/*
================
idTrigger_Timer::Spawn

Repeatedly fires its targets.
Can be turned on or off by using.
================
*/
void idTrigger_Timer::Spawn( void ) {

	spawnArgs.GetFloat( "random", "1", random );
	spawnArgs.GetFloat( "wait", "1", wait );
	spawnArgs.GetBool( "start_on", "0", on );
	spawnArgs.GetFloat( "delay", "0", delay );
	onName = spawnArgs.GetString( "onName" );
	offName = spawnArgs.GetString( "offName" );

	if ( random >= wait && wait >= 0 ) {
		random = wait - 0.001;
		gameLocal.Warning( "idTrigger_Timer '%s' at (%s) has random >= wait", name.c_str(), GetPhysics()->GetOrigin().ToString(0) );
	}

	if ( on ) {
		PostEventSec( &EV_Timer, delay );
	}
}

/*
================
idTrigger_Timer::Enable
================
*/
void idTrigger_Timer::Enable( void ) {
	// if off, turn it on
	if ( !on ) {
		on = true;
		PostEventSec( &EV_Timer, delay );
	}
}

/*
================
idTrigger_Timer::Disable
================
*/
void idTrigger_Timer::Disable( void ) {
	// if on, turn it off
	if ( on ) {
		on = false;
		CancelEvents( &EV_Timer );
	}
}

/*
================
idTrigger_Timer::Event_Timer
================
*/
void idTrigger_Timer::Event_Timer( void ) {
	ActivateTargets( this );

	// Solarsplace - Arx End Of Sun
	// Bug in id's code. The timer trigger did not call script functions.
	CallScript();

	// set time before next firing
	if ( wait >= 0.0f ) {
		PostEventSec( &EV_Timer, wait + gameLocal.random.CRandomFloat() * random );
	}
}

/*
================
idTrigger_Timer::Event_Use
================
*/
void idTrigger_Timer::Event_Use( idEntity *activator ) {
	// if on, turn it off
	if ( on ) {
		if ( offName.Length() && offName.Icmp( activator->GetName() ) ) {
			return;
		}
		on = false;
		CancelEvents( &EV_Timer );
	} else {
		// turn it on
		if ( onName.Length() && onName.Icmp( activator->GetName() ) ) {
			return;
		}
		on = true;
		PostEventSec( &EV_Timer, delay );
	}
}

/*
===============================================================================

  idTrigger_Count
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Count )
	EVENT( EV_Activate,	idTrigger_Count::Event_Trigger )
	EVENT( EV_TriggerAction,	idTrigger_Count::Event_TriggerAction )
END_CLASS

/*
================
idTrigger_Count::idTrigger_Count
================
*/
idTrigger_Count::idTrigger_Count( void ) {
	goal = 0;
	count = 0;
	delay = 0.0f;
}

/*
================
idTrigger_Count::Save
================
*/
void idTrigger_Count::Save( idSaveGame *savefile ) const {
	savefile->WriteInt( goal );
	savefile->WriteInt( count );
	savefile->WriteFloat( delay );
}

/*
================
idTrigger_Count::Restore
================
*/
void idTrigger_Count::Restore( idRestoreGame *savefile ) {
	savefile->ReadInt( goal );
	savefile->ReadInt( count );
	savefile->ReadFloat( delay );
}

/*
================
idTrigger_Count::Spawn
================
*/
void idTrigger_Count::Spawn( void ) {
	spawnArgs.GetInt( "count", "1", goal );
	spawnArgs.GetFloat( "delay", "0", delay );
	count = 0;
}

/*
================
idTrigger_Count::Event_Trigger
================
*/
void idTrigger_Count::Event_Trigger( idEntity *activator ) {
	// goal of -1 means trigger has been exhausted
	if (goal >= 0) {
		count++;
		if ( count >= goal ) {
			if (spawnArgs.GetBool("repeat")) {
				count = 0;
			} else {
				goal = -1;
			}
			PostEventSec( &EV_TriggerAction, delay, activator );
		}
	}
}

/*
================
idTrigger_Count::Event_TriggerAction
================
*/
void idTrigger_Count::Event_TriggerAction( idEntity *activator ) {
	ActivateTargets( activator );
	CallScript();
	if ( goal == -1 ) {

		// Solarsplace - Arx End Of Sun - Level transition related.
		gameLocal.GetLocalPlayer()->SaveTransitionInfoSpecific( this, false, true );

		PostEventMS( &EV_Remove, 0 );
	}
}

/*
===============================================================================

  idTrigger_Hurt
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Hurt )
	EVENT( EV_Touch,		idTrigger_Hurt::Event_Touch )
	EVENT( EV_Activate,		idTrigger_Hurt::Event_Toggle )
END_CLASS


/*
================
idTrigger_Hurt::idTrigger_Hurt
================
*/
idTrigger_Hurt::idTrigger_Hurt( void ) {
	on = false;
	delay = 0.0f;
	nextTime = 0;
}

/*
================
idTrigger_Hurt::Save
================
*/
void idTrigger_Hurt::Save( idSaveGame *savefile ) const {
	savefile->WriteBool( on );
	savefile->WriteFloat( delay );
	savefile->WriteInt( nextTime );
}

/*
================
idTrigger_Hurt::Restore
================
*/
void idTrigger_Hurt::Restore( idRestoreGame *savefile ) {
	savefile->ReadBool( on );
	savefile->ReadFloat( delay );
	savefile->ReadInt( nextTime );
}

/*
================
idTrigger_Hurt::Spawn

	Damages activator
	Can be turned on or off by using.
================
*/
void idTrigger_Hurt::Spawn( void ) {
	spawnArgs.GetBool( "on", "1", on );
	spawnArgs.GetFloat( "delay", "1.0", delay );
	nextTime = gameLocal.time;
	Enable();
}

/*
================
idTrigger_Hurt::Event_Touch
================
*/
void idTrigger_Hurt::Event_Touch( idEntity *other, trace_t *trace ) {
	const char *damage;

	if ( on && other && gameLocal.time >= nextTime ) {
		damage = spawnArgs.GetString( "def_damage", "damage_painTrigger" );
		other->Damage( NULL, NULL, vec3_origin, damage, 1.0f, INVALID_JOINT );

		ActivateTargets( other );
		CallScript();

		nextTime = gameLocal.time + SEC2MS( delay );
	}
}

/*
================
idTrigger_Hurt::Event_Toggle
================
*/
void idTrigger_Hurt::Event_Toggle( idEntity *activator ) {
	on = !on;
}


/*
===============================================================================

  idTrigger_Fade

===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Fade )
	EVENT( EV_Activate,		idTrigger_Fade::Event_Trigger )
END_CLASS

/*
================
idTrigger_Fade::Event_Trigger
================
*/
void idTrigger_Fade::Event_Trigger( idEntity *activator ) {
	idVec4		fadeColor;
	int			fadeTime;
	idPlayer	*player;

	player = gameLocal.GetLocalPlayer();
	if ( player ) {
		fadeColor = spawnArgs.GetVec4( "fadeColor", "0, 0, 0, 1" );
		fadeTime = SEC2MS( spawnArgs.GetFloat( "fadeTime", "0.5" ) );
		player->playerView.Fade( fadeColor, fadeTime );
		PostEventMS( &EV_ActivateTargets, fadeTime, activator );
	}
}

/*
===============================================================================

  idTrigger_Touch
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_Touch )
	EVENT( EV_Activate,		idTrigger_Touch::Event_Trigger )
END_CLASS


/*
================
idTrigger_Touch::idTrigger_Touch
================
*/
idTrigger_Touch::idTrigger_Touch( void ) {
	clipModel = NULL;
}

/*
================
idTrigger_Touch::Spawn
================
*/
void idTrigger_Touch::Spawn( void ) {
	// get the clip model
	clipModel = new idClipModel( GetPhysics()->GetClipModel() );

	// remove the collision model from the physics object
	GetPhysics()->SetClipModel( NULL, 1.0f );

	if ( spawnArgs.GetBool( "start_on" ) ) {
		BecomeActive( TH_THINK );
	}
}

/*
================
idTrigger_Touch::Save
================
*/
void idTrigger_Touch::Save( idSaveGame *savefile ) {
	savefile->WriteClipModel( clipModel );
}

/*
================
idTrigger_Touch::Restore
================
*/
void idTrigger_Touch::Restore( idRestoreGame *savefile ) {
	savefile->ReadClipModel( clipModel );
}

/*
================
idTrigger_Touch::TouchEntities
================
*/
void idTrigger_Touch::TouchEntities( void ) {
	int numClipModels, i;
	idBounds bounds;
	idClipModel *cm, *clipModelList[ MAX_GENTITIES ];

	if ( clipModel == NULL || scriptFunction == NULL ) {
		return;
	}

	bounds.FromTransformedBounds( clipModel->GetBounds(), clipModel->GetOrigin(), clipModel->GetAxis() );
	numClipModels = gameLocal.clip.ClipModelsTouchingBounds( bounds, -1, clipModelList, MAX_GENTITIES );

	for ( i = 0; i < numClipModels; i++ ) {
		cm = clipModelList[ i ];

		if ( !cm->IsTraceModel() ) {
			continue;
		}

		idEntity *entity = cm->GetEntity();

		if ( !entity ) {
			continue;
		}
		
		if ( !gameLocal.clip.ContentsModel( cm->GetOrigin(), cm, cm->GetAxis(), -1,
									clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis() ) ) {
			continue;
		}

		ActivateTargets( entity );

		idThread *thread = new idThread();
		thread->CallFunction( entity, scriptFunction, false );
		thread->DelayedStart( 0 );
	}
}

/*
================
idTrigger_Touch::Think
================
*/
void idTrigger_Touch::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		TouchEntities();
	}
	idEntity::Think();
}

/*
================
idTrigger_Touch::Event_Trigger
================
*/
void idTrigger_Touch::Event_Trigger( idEntity *activator ) {
	if ( thinkFlags & TH_THINK ) {
		BecomeInactive( TH_THINK );
	} else {
		BecomeActive( TH_THINK );
	}
}

/*
================
idTrigger_Touch::Enable
================
*/
void idTrigger_Touch::Enable( void ) {
	BecomeActive( TH_THINK );
}

/*
================
idTrigger_Touch::Disable
================
*/
void idTrigger_Touch::Disable( void ) {
	BecomeInactive( TH_THINK );
}

/*
===============================================================================

	Arx - End Of Sun

	idTrigger_FullScreenMenuGUI
	
===============================================================================
*/

CLASS_DECLARATION( idTrigger, idTrigger_FullScreenMenuGUI )
	EVENT( EV_Activate,		idTrigger_FullScreenMenuGUI::Event_Trigger )
END_CLASS


/*
================
idTrigger_FullScreenMenuGUI::idTrigger_FullScreenMenuGUI
================
*/
idTrigger_FullScreenMenuGUI::idTrigger_FullScreenMenuGUI( void ) {
	clipModel = NULL;
}

/*
================
idTrigger_FullScreenMenuGUI::Spawn
================
*/
void idTrigger_FullScreenMenuGUI::Spawn( void ) {
	// get the clip model
	clipModel = new idClipModel( GetPhysics()->GetClipModel() );

	// remove the collision model from the physics object
	GetPhysics()->SetClipModel( NULL, 1.0f );

	//TODO - Add safety checks
	idStr interfaceGUI = this->spawnArgs.GetString( "gui_fullScreenMenu", "" );
	fullScreenGUIInterface = uiManager->FindGui( interfaceGUI, true, false, true );

	fullScreenGUIInterfaceOpen = false;

	/*
	if ( spawnArgs.GetBool( "start_on" ) ) {
		BecomeActive( TH_THINK );
	}
	*/
}

/*
================
idTrigger_FullScreenMenuGUI::Save
================
*/
void idTrigger_FullScreenMenuGUI::Save( idSaveGame *savefile ) {
	savefile->WriteClipModel( clipModel );
	savefile->WriteUserInterface( fullScreenGUIInterface, false );
	savefile->WriteBool( fullScreenGUIInterfaceOpen );
}

/*
================
idTrigger_FullScreenMenuGUI::Restore
================
*/
void idTrigger_FullScreenMenuGUI::Restore( idRestoreGame *savefile ) {
	savefile->ReadClipModel( clipModel );
	savefile->ReadUserInterface( fullScreenGUIInterface );
	savefile->ReadBool( fullScreenGUIInterfaceOpen );
}

/*
================
idTrigger_FullScreenMenuGUI::Redraw
================
*/
void idTrigger_FullScreenMenuGUI::Redraw( void ) {
	if ( fullScreenGUIInterface && fullScreenGUIInterfaceOpen ) {
		fullScreenGUIInterface->Redraw( gameLocal.time );
	}
}

/*
================
idTrigger_FullScreenMenuGUI::Think
================
*/
void idTrigger_FullScreenMenuGUI::Think( void ) {
	if ( thinkFlags & TH_THINK ) {
		UpdateGUI();
	}
	idEntity::Think();
}


/*
================
idTrigger_FullScreenMenuGUI::UpdateGUI
================
*/
void idTrigger_FullScreenMenuGUI::UpdateGUI( void ) {

	// This is probably not needed at the moment.

	if ( fullScreenGUIInterface && fullScreenGUIInterfaceOpen ) {

		fullScreenGUIInterface->StateChanged( gameLocal.time );
	}
}

/*
================
idTrigger_FullScreenMenuGUI::ToggleFullScreenGUIInterface
================
*/
void idTrigger_FullScreenMenuGUI::ToggleFullScreenGUIInterface( void ) {

	if( !fullScreenGUIInterfaceOpen )
	{
		//gameLocal.Printf("ToggleFullScreenGUIInterface = opening\n" );

		fullScreenGUIInterface->Activate( true, gameLocal.time );
		fullScreenGUIInterfaceOpen = true;

		gameLocal.GetLocalPlayer()->fullScreenMenuGUIId = this->name;
		gameLocal.GetLocalPlayer()->SetInfluenceLevel( INFLUENCE_LEVEL2 );
	}
	else
	{
		//gameLocal.Printf("ToggleFullScreenGUIInterface = closing\n" );

		fullScreenGUIInterface->Activate( false, gameLocal.time );
		fullScreenGUIInterfaceOpen = false;

		gameLocal.GetLocalPlayer()->fullScreenMenuGUIId = "";
		gameLocal.GetLocalPlayer()->SetInfluenceLevel( INFLUENCE_NONE );
	}
}

/*
================
idTrigger_FullScreenMenuGUI::Trigger
================
*/
void idTrigger_FullScreenMenuGUI::Trigger( void )
{
	Event_Trigger( this );
}

/*
================
idTrigger_FullScreenMenuGUI::Event_Trigger
================
*/
void idTrigger_FullScreenMenuGUI::Event_Trigger( idEntity *activator ) {

	ToggleFullScreenGUIInterface();

	if ( thinkFlags & TH_THINK ) {
		BecomeActive( TH_THINK );
	} else {
		BecomeInactive( TH_THINK );
	}
}