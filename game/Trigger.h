// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __GAME_TRIGGER_H__
#define __GAME_TRIGGER_H__

extern const idEventDef EV_Enable;
extern const idEventDef EV_Disable;

/*
===============================================================================

  Trigger base.

===============================================================================
*/

class idTrigger : public idEntity {
public:
	CLASS_PROTOTYPE( idTrigger );

	static void			DrawDebugInfo( void );

						idTrigger();
	void				Spawn( void );

	const function_t *	GetScriptFunction( void ) const;

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	virtual void		Enable( void );
	virtual void		Disable( void );

	// Solarsplace - Arx End Of Sun
	bool				ArxIsEnabled( void );

protected:
	void				CallScript( void ) const;

	void				Event_Enable( void );
	void				Event_Disable( void );

	// Solarsplace - Arx End Of Sun
	bool				isEnabled;

	const function_t *	scriptFunction;
};


/*
===============================================================================

  Trigger which can be activated multiple times.

===============================================================================
*/

class idTrigger_Multi : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Multi );

						idTrigger_Multi( void );

	void				Spawn( void );

	// Solarsplace - Arx End Of Sun
	void				Think( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

private:
	float				wait;
	float				random;
	float				delay;
	float				random_delay;
	int					nextTriggerTime;
	idStr				requires;
	int					removeItem;
	bool				touchClient;
	bool				touchOther;
	bool				triggerFirst;
	bool				triggerWithSelf;

	bool				CheckFacing( idEntity *activator );
	void				TriggerAction( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
	void				Event_Trigger( idEntity *activator );
	void				Event_Touch( idEntity *other, trace_t *trace );

	// Solarsplace - Arx End Of Sun
	idClipModel *		clipModel;
	float				TouchingWeights( void );
	float				requirementWeight;
	bool				triggerEnabled;
	bool				oldTriggerEnabled;
};


/*
===============================================================================

  Trigger which can only be activated by an entity with a specific name.

===============================================================================
*/

class idTrigger_EntityName : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_EntityName );

						idTrigger_EntityName( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );

private:
	float				wait;
	float				random;
	float				delay;
	float				random_delay;
	int					nextTriggerTime;
	bool				triggerFirst;
	idStr				entityName;

	void				TriggerAction( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
	void				Event_Trigger( idEntity *activator );
	void				Event_Touch( idEntity *other, trace_t *trace );
};

/*
===============================================================================

  Trigger which repeatedly fires targets.

===============================================================================
*/

class idTrigger_Timer : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Timer );

						idTrigger_Timer( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );

	virtual void		Enable( void );
	virtual void		Disable( void );

private:
	float				random;
	float				wait;
	bool				on;
	float				delay;
	idStr				onName;
	idStr				offName;

	void				Event_Timer( void );
	void				Event_Use( idEntity *activator );
};


/*
===============================================================================

  Trigger which fires targets after being activated a specific number of times.

===============================================================================
*/

class idTrigger_Count : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Count );

						idTrigger_Count( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );

private:
	int					goal;
	int					count;
	float				delay;

	void				Event_Trigger( idEntity *activator );
	void				Event_TriggerAction( idEntity *activator );
};


/*
===============================================================================

  Trigger which hurts touching entities.

===============================================================================
*/

class idTrigger_Hurt : public idTrigger {
public:
	CLASS_PROTOTYPE( idTrigger_Hurt );

						idTrigger_Hurt( void );

	void				Save( idSaveGame *savefile ) const;
	void				Restore( idRestoreGame *savefile );

	void				Spawn( void );

private:
	bool				on;
	float				delay;
	int					nextTime;

	void				Event_Touch( idEntity *other, trace_t *trace );
	void				Event_Toggle( idEntity *activator );
};


/*
===============================================================================

  Trigger which fades the player view.

===============================================================================
*/

class idTrigger_Fade : public idTrigger {
public:

	CLASS_PROTOTYPE( idTrigger_Fade );

private:
	void				Event_Trigger( idEntity *activator );
};


/*
===============================================================================

  Trigger which continuously tests whether other entities are touching it.

===============================================================================
*/

class idTrigger_Touch : public idTrigger {
public:

	CLASS_PROTOTYPE( idTrigger_Touch );

						idTrigger_Touch( void );

	void				Spawn( void );
	virtual void		Think( void );

	void				Save( idSaveGame *savefile );
	void				Restore( idRestoreGame *savefile );

	virtual void		Enable( void );
	virtual void		Disable( void );

	void				TouchEntities( void );

private:
	idClipModel *		clipModel;

	void				Event_Trigger( idEntity *activator );
};

/*
===============================================================================

	Arx - End Of Sun

	Trigger which displays a full screen GUI over the view

===============================================================================
*/

class idTrigger_FullScreenMenuGUI : public idTrigger {
public:

	CLASS_PROTOTYPE( idTrigger_FullScreenMenuGUI );

						idTrigger_FullScreenMenuGUI( void );

	void				Spawn( void );
	virtual void		Think( void );

	void				Save( idSaveGame *savefile );
	void				Restore( idRestoreGame *savefile );

	void				Redraw( void );

	idUserInterface *	fullScreenGUIInterface;
	bool				fullScreenGUIInterfaceOpen;

	void				Trigger( void );

private:
	idClipModel *		clipModel;

	void				UpdateGUI( void );
	void				ToggleFullScreenGUIInterface( void );

	void				Event_Trigger( idEntity *activator );
};

#endif /* !__GAME_TRIGGER_H__ */
