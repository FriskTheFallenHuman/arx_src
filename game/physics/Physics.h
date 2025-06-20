// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __PHYSICS_H__
#define __PHYSICS_H__

/*
===============================================================================

	Physics abstract class

	A physics object is a tool to manipulate the position and orientation of
	an entity. The physics object is a container for idClipModels used for
	collision detection. The physics deals with moving these collision models
	through the world according to the laws of physics or other rules.

	The mass of a clip model is the volume of the clip model times the density.
	An arbitrary mass can however be set for specific clip models or the
	whole physics object. The contents of a clip model is a set of bit flags
	that define the contents. The clip mask defines the contents a clip model
	collides with.

	The linear velocity of a physics object is a vector that defines the
	translation of the center of mass in units per second. The angular velocity
	of a physics object is a vector that passes through the center of mass. The
	direction of this vector defines the axis of rotation and the magnitude
	defines the rate of rotation about the axis in radians per second.
	The gravity is the change in velocity per second due to gravitational force.

	Entities update their visual position and orientation from the physics
	using GetOrigin() and GetAxis(). Direct origin and axis changes of
	entities should go through the physics. In other words the physics origin
	and axis are updated first and the entity updates it's visual position
	from the physics.

===============================================================================
*/

#define CONTACT_EPSILON			0.25f				// maximum contact seperation distance

class idEntity;
class idPhysics_Liquid;

typedef struct impactInfo_s {
	float						invMass;			// inverse mass
	idMat3						invInertiaTensor;	// inverse inertia tensor
	idVec3						position;			// impact position relative to center of mass
	idVec3						velocity;			// velocity at the impact position
} impactInfo_t;


class idPhysics : public idClass {

public:
	ABSTRACT_PROTOTYPE( idPhysics );

	virtual						~idPhysics( void );
	static int					SnapTimeToPhysicsFrame( int t );

	// Must not be virtual
	void						Save( idSaveGame *savefile ) const;
	void						Restore( idRestoreGame *savefile );

public:	// common physics interface
								// set pointer to entity using physics
	virtual void				SetSelf( idEntity *e ) = 0;
								// clip models
	virtual void				SetClipModel( idClipModel *model, float density, int id = 0, bool freeOld = true ) = 0;
	virtual void				SetClipBox( const idBounds &bounds, float density );
	virtual idClipModel *		GetClipModel( int id = 0 ) const = 0;
	virtual int					GetNumClipModels( void ) const = 0;
								// get/set the mass of a specific clip model or the whole physics object
	virtual void				SetMass( float mass, int id = -1 ) = 0;
	virtual float				GetMass( int id = -1 ) const = 0;
								// get/set the contents of a specific clip model or the whole physics object
	virtual void				SetContents( int contents, int id = -1 ) = 0;
	virtual int					GetContents( int id = -1 ) const = 0;
								// get/set the contents a specific clip model or the whole physics object collides with
	virtual void				SetClipMask( int mask, int id = -1 ) = 0;
	virtual int					GetClipMask( int id = -1 ) const = 0;
								// get the bounds of a specific clip model or the whole physics object
	virtual const idBounds &	GetBounds( int id = -1 ) const = 0;
	virtual const idBounds &	GetAbsBounds( int id = -1 ) const = 0;
								// evaluate the physics with the given time step, returns true if the object moved
	virtual bool				Evaluate( int timeStepMSec, int endTimeMSec ) = 0;
								// update the time without moving
	virtual void				UpdateTime( int endTimeMSec ) = 0;
								// get the last physics update time
	virtual int					GetTime( void ) const = 0;
								// collision interaction between different physics objects
	virtual void				GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const = 0;
	virtual void				ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse ) = 0;
	virtual void				AddForce( const int id, const idVec3 &point, const idVec3 &force ) = 0;
	virtual void				Activate( void ) = 0;
	virtual void				PutToRest( void ) = 0;
	virtual bool				IsAtRest( void ) const = 0;
	virtual int					GetRestStartTime( void ) const = 0;
	virtual bool				IsPushable( void ) const = 0;
								// save and restore the physics state
	virtual void				SaveState( void ) = 0;
	virtual void				RestoreState( void ) = 0;
								// set the position and orientation in master space or world space if no master set
	virtual void				SetOrigin( const idVec3 &newOrigin, int id = -1 ) = 0;
	virtual void				SetAxis( const idMat3 &newAxis, int id = -1 ) = 0;
								// translate or rotate the physics object in world space
	virtual void				Translate( const idVec3 &translation, int id = -1 ) = 0;
	virtual void				Rotate( const idRotation &rotation, int id = -1 ) = 0;
								// get the position and orientation in world space
	virtual const idVec3 &		GetOrigin( int id = 0 ) const = 0;
	virtual const idMat3 &		GetAxis( int id = 0 ) const = 0;
								// set linear and angular velocity
	virtual void				SetLinearVelocity( const idVec3 &newLinearVelocity, int id = 0 ) = 0;
	virtual void				SetAngularVelocity( const idVec3 &newAngularVelocity, int id = 0 ) = 0;
								// get linear and angular velocity
	virtual const idVec3 &		GetLinearVelocity( int id = 0 ) const = 0;
	virtual const idVec3 &		GetAngularVelocity( int id = 0 ) const = 0;
								// gravity
	virtual void				SetGravity( const idVec3 &newGravity ) = 0;
	virtual const idVec3 &		GetGravity( void ) const = 0;
	virtual const idVec3 &		GetGravityNormal( void ) const = 0;
								// get first collision when translating or rotating this physics object
	virtual void				ClipTranslation( trace_t &results, const idVec3 &translation, const idClipModel *model ) const = 0;
	virtual void				ClipRotation( trace_t &results, const idRotation &rotation, const idClipModel *model ) const = 0;
	virtual int					ClipContents( const idClipModel *model ) const = 0;
								// disable/enable the clip models contained by this physics object
	virtual void				DisableClip( void ) = 0;
	virtual void				EnableClip( void ) = 0;
								// link/unlink the clip models contained by this physics object
	virtual void				UnlinkClip( void ) = 0;
	virtual void				LinkClip( void ) = 0;
								// contacts
	virtual bool				EvaluateContacts( void ) = 0;
	virtual int					GetNumContacts( void ) const = 0;
	virtual const contactInfo_t &GetContact( int num ) const = 0;
	virtual void				ClearContacts( void ) = 0;
	virtual void				AddContactEntity( idEntity *e ) = 0;
	virtual void 				RemoveContactEntity( idEntity *e ) = 0;
								// ground contacts
	virtual bool				HasGroundContacts( void ) const = 0;
	virtual bool				IsGroundEntity( int entityNum ) const = 0;
	virtual bool				IsGroundClipModel( int entityNum, int id ) const = 0;
								// set the master entity for objects bound to a master
	virtual void				SetMaster( idEntity *master, const bool orientated = true ) = 0;
								// set pushed state
	virtual void				SetPushed( int deltaTime ) = 0;
	virtual const idVec3 &		GetPushedLinearVelocity( const int id = 0 ) const = 0;
	virtual const idVec3 &		GetPushedAngularVelocity( const int id = 0 ) const = 0;
								// get blocking info, returns NULL if the object is not blocked
	virtual const trace_t *		GetBlockingInfo( void ) const = 0;
	virtual idEntity *			GetBlockingEntity( void ) const = 0;
								// movement end times in msec for reached events at the end of predefined motion
	virtual int					GetLinearEndTime( void ) const = 0;
	virtual int					GetAngularEndTime( void ) const = 0;
								// networking
	virtual void				WriteToSnapshot( idBitMsgDelta &msg ) const = 0;
	virtual void				ReadFromSnapshot( const idBitMsgDelta &msg ) = 0;

								// gets/sets the water
								// these should be pure virtual but I would've had to change 10 or so other classes
								// so this was a better solution
	virtual idPhysics_Liquid	*GetWater() { return NULL; }
	virtual void				SetWater( idPhysics_Liquid *e ) {}
};

#endif /* !__PHYSICS_H__ */
