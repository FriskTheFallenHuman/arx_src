// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __PHYSICS_AF_H__
#define __PHYSICS_AF_H__

/*
===================================================================================

	Articulated Figure physics

	Employs a constraint force based dynamic simulation using a lagrangian
	multiplier method to solve for the constraint forces.

===================================================================================
*/

class idAFConstraint;
class idAFConstraint_Fixed;
class idAFConstraint_BallAndSocketJoint;
class idAFConstraint_BallAndSocketJointFriction;
class idAFConstraint_UniversalJoint;
class idAFConstraint_UniversalJointFriction;
class idAFConstraint_CylindricalJoint;
class idAFConstraint_Hinge;
class idAFConstraint_HingeFriction;
class idAFConstraint_HingeSteering;
class idAFConstraint_Slider;
class idAFConstraint_Line;
class idAFConstraint_Plane;
class idAFConstraint_Spring;
class idAFConstraint_Contact;
class idAFConstraint_ContactFriction;
class idAFConstraint_ConeLimit;
class idAFConstraint_PyramidLimit;
class idAFConstraint_Suspension;
class idAFBody;
class idAFTree;
class idPhysics_AF;

typedef enum {
	CONSTRAINT_INVALID,
	CONSTRAINT_FIXED,
	CONSTRAINT_BALLANDSOCKETJOINT,
	CONSTRAINT_UNIVERSALJOINT,
	CONSTRAINT_HINGE,
	CONSTRAINT_HINGESTEERING,
	CONSTRAINT_SLIDER,
	CONSTRAINT_CYLINDRICALJOINT,
	CONSTRAINT_LINE,
	CONSTRAINT_PLANE,
	CONSTRAINT_SPRING,
	CONSTRAINT_CONTACT,
	CONSTRAINT_FRICTION,
	CONSTRAINT_CONELIMIT,
	CONSTRAINT_PYRAMIDLIMIT,
	CONSTRAINT_SUSPENSION
} constraintType_t;


//===============================================================
//
//	idAFConstraint
//
//===============================================================

// base class for all constraints
class idAFConstraint {

	friend class idPhysics_AF;
	friend class idAFTree;

public:
							idAFConstraint( void );
	virtual					~idAFConstraint( void );
	constraintType_t		GetType( void ) const { return type; }
	const idStr &			GetName( void ) const { return name; }
	idAFBody *				GetBody1( void ) const { return body1; }
	idAFBody *				GetBody2( void ) const { return body2; }
	void					SetPhysics( idPhysics_AF *p ) { physics = p; }
	const idVecX &			GetMultiplier( void );
	virtual void			SetBody1( idAFBody *body );
	virtual void			SetBody2( idAFBody *body );
	virtual void			DebugDraw( void );
	virtual void			GetForce( idAFBody *body, idVec6 &force );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	constraintType_t		type;						// constraint type
	idStr					name;						// name of constraint
	idAFBody *				body1;						// first constrained body
	idAFBody *				body2;						// second constrained body, NULL for world
	idPhysics_AF *			physics;					// for adding additional constraints like limits

							// simulation variables set by Evaluate
	idMatX					J1, J2;						// matrix with left hand side of constraint equations
	idVecX					c1, c2;						// right hand side of constraint equations
	idVecX					lo, hi, e;					// low and high bounds and lcp epsilon
	idAFConstraint *		boxConstraint;				// constraint the boxIndex refers to
	int						boxIndex[6];				// indexes for special box constrained variables

							// simulation variables used during calculations
	idMatX					invI;						// transformed inertia
	idMatX					J;							// transformed constraint matrix
	idVecX					s;							// temp solution
	idVecX					lm;							// lagrange multipliers
	int						firstIndex;					// index of the first constraint row in the lcp matrix

	struct constraintFlags_s {
		bool				allowPrimary		: 1;	// true if the constraint can be used as a primary constraint
		bool				frameConstraint		: 1;	// true if this constraint is added to the frame constraints
		bool				noCollision			: 1;	// true if body1 and body2 never collide with each other
		bool				isPrimary			: 1;	// true if this is a primary constraint
		bool				isZero				: 1;	// true if 's' is zero during calculations
	} fl;

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
	void					InitSize( int size );
};

// fixed or rigid joint which allows zero degrees of freedom
// constrains body1 to have a fixed position and orientation relative to body2
class idAFConstraint_Fixed : public idAFConstraint {

public:
							idAFConstraint_Fixed( const idStr &name, idAFBody *body1, idAFBody *body2 );
	void					SetRelativeOrigin( const idVec3 &origin ) { this->offset = origin; }
	void					SetRelativeAxis( const idMat3 &axis ) { this->relAxis = axis; }
	virtual void			SetBody1( idAFBody *body );
	virtual void			SetBody2( idAFBody *body );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					offset;						// offset of body1 relative to body2 in body2 space
	idMat3					relAxis;					// rotation of body1 relative to body2

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
	void					InitOffset( void );
};

// ball and socket or spherical joint which allows 3 degrees of freedom
// constrains body1 relative to body2 with a ball and socket joint
class idAFConstraint_BallAndSocketJoint : public idAFConstraint {

public:
							idAFConstraint_BallAndSocketJoint( const idStr &name, idAFBody *body1, idAFBody *body2 );
							~idAFConstraint_BallAndSocketJoint( void );
	void					SetAnchor( const idVec3 &worldPosition );
	idVec3					GetAnchor( void ) const;
	void					SetNoLimit( void );
	void					SetConeLimit( const idVec3 &coneAxis, const float coneAngle, const idVec3 &body1Axis );
	void					SetPyramidLimit( const idVec3 &pyramidAxis, const idVec3 &baseAxis,
											const float angle1, const float angle2, const idVec3 &body1Axis );
	void					SetLimitEpsilon( const float e );
	void					SetFriction( const float f ) { friction = f; }
	float					GetFriction( void ) const;
	virtual void			DebugDraw( void );
	virtual void			GetForce( idAFBody *body, idVec6 &force );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					anchor1;					// anchor in body1 space
	idVec3					anchor2;					// anchor in body2 space
	float					friction;					// joint friction
	idAFConstraint_ConeLimit *coneLimit;				// cone shaped limit
	idAFConstraint_PyramidLimit *pyramidLimit;			// pyramid shaped limit
	idAFConstraint_BallAndSocketJointFriction *fc;		// friction constraint

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// ball and socket joint friction
class idAFConstraint_BallAndSocketJointFriction : public idAFConstraint {

public:
							idAFConstraint_BallAndSocketJointFriction( void );
	void					Setup( idAFConstraint_BallAndSocketJoint *cc );
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:
	idAFConstraint_BallAndSocketJoint *joint;

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// universal, Cardan or Hooke joint which allows 2 degrees of freedom
// like a ball and socket joint but also constrains the rotation about the cardan shafts
class idAFConstraint_UniversalJoint : public idAFConstraint {

public:
							idAFConstraint_UniversalJoint( const idStr &name, idAFBody *body1, idAFBody *body2 );
							~idAFConstraint_UniversalJoint( void );
	void					SetAnchor( const idVec3 &worldPosition );
	idVec3					GetAnchor( void ) const;
	void					SetShafts( const idVec3 &cardanShaft1, const idVec3 &cardanShaft2 );
	void					GetShafts( idVec3 &cardanShaft1, idVec3 &cardanShaft2 ) { cardanShaft1 = shaft1; cardanShaft2 = shaft2; }
	void					SetNoLimit( void );
	void					SetConeLimit( const idVec3 &coneAxis, const float coneAngle );
	void					SetPyramidLimit( const idVec3 &pyramidAxis, const idVec3 &baseAxis,
											const float angle1, const float angle2 );
	void					SetLimitEpsilon( const float e );
	void					SetFriction( const float f ) { friction = f; }
	float					GetFriction( void ) const;
	virtual void			DebugDraw( void );
	virtual void			GetForce( idAFBody *body, idVec6 &force );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					anchor1;					// anchor in body1 space
	idVec3					anchor2;					// anchor in body2 space
	idVec3					shaft1;						// body1 cardan shaft in body1 space
	idVec3					shaft2;						// body2 cardan shaft in body2 space
	idVec3					axis1;						// cardan axis in body1 space
	idVec3					axis2;						// cardan axis in body2 space
	float					friction;					// joint friction
	idAFConstraint_ConeLimit *coneLimit;				// cone shaped limit
	idAFConstraint_PyramidLimit *pyramidLimit;			// pyramid shaped limit
	idAFConstraint_UniversalJointFriction *fc;			// friction constraint

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// universal joint friction
class idAFConstraint_UniversalJointFriction : public idAFConstraint {

public:
							idAFConstraint_UniversalJointFriction( void );
	void					Setup( idAFConstraint_UniversalJoint *cc );
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:
	idAFConstraint_UniversalJoint *joint;			// universal joint

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// cylindrical joint which allows 2 degrees of freedom
// constrains body1 to lie on a line relative to body2 and allows only translation along and rotation about the line
class idAFConstraint_CylindricalJoint : public idAFConstraint {

public:
							idAFConstraint_CylindricalJoint( const idStr &name, idAFBody *body1, idAFBody *body2 );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// hinge, revolute or pin joint which allows 1 degree of freedom
// constrains all motion of body1 relative to body2 except the rotation about the hinge axis
class idAFConstraint_Hinge : public idAFConstraint {

public:
							idAFConstraint_Hinge( const idStr &name, idAFBody *body1, idAFBody *body2 );
							~idAFConstraint_Hinge( void );
	void					SetAnchor( const idVec3 &worldPosition );
	idVec3					GetAnchor( void ) const;
	void					SetAxis( const idVec3 &axis );
	void					GetAxis( idVec3 &a1, idVec3 &a2 ) const { a1 = axis1; a2 = axis2; }
	idVec3					GetAxis( void ) const;
	void					SetNoLimit( void );
	void					SetLimit( const idVec3 &axis, const float angle, const idVec3 &body1Axis );
	void					SetLimitEpsilon( const float e );
	float					GetAngle( void ) const;
	void					SetSteerAngle( const float degrees );
	void					SetSteerSpeed( const float speed );
	void					SetFriction( const float f ) { friction = f; }
	float					GetFriction( void ) const;
	virtual void			DebugDraw( void );
	virtual void			GetForce( idAFBody *body, idVec6 &force );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					anchor1;					// anchor in body1 space
	idVec3					anchor2;					// anchor in body2 space
	idVec3					axis1;						// axis in body1 space
	idVec3					axis2;						// axis in body2 space
	idMat3					initialAxis;				// initial axis of body1 relative to body2
	float					friction;					// hinge friction
	idAFConstraint_ConeLimit *coneLimit;				// cone limit
	idAFConstraint_HingeSteering *steering;				// steering
	idAFConstraint_HingeFriction *fc;					// friction constraint

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// hinge joint friction
class idAFConstraint_HingeFriction : public idAFConstraint {

public:
							idAFConstraint_HingeFriction( void );
	void					Setup( idAFConstraint_Hinge *cc );
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:
	idAFConstraint_Hinge *	hinge;						// hinge

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// constrains two bodies attached to each other with a hinge to get a specified relative orientation
class idAFConstraint_HingeSteering : public idAFConstraint {

public:
							idAFConstraint_HingeSteering( void );
	void					Setup( idAFConstraint_Hinge *cc );
	void					SetSteerAngle( const float degrees ) { steerAngle = degrees; }
	void					SetSteerSpeed( const float speed ) { steerSpeed = speed; }
	void					SetEpsilon( const float e ) { epsilon = e; }
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idAFConstraint_Hinge *	hinge;						// hinge
	float					steerAngle;					// desired steer angle in degrees
	float					steerSpeed;					// steer speed
	float					epsilon;					// lcp epsilon

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// slider, prismatic or translational constraint which allows 1 degree of freedom
// constrains body1 to lie on a line relative to body2, the orientation is also fixed relative to body2
class idAFConstraint_Slider : public idAFConstraint {

public:
							idAFConstraint_Slider( const idStr &name, idAFBody *body1, idAFBody *body2 );
	void					SetAxis( const idVec3 &ax );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					axis;						// axis along which body1 slides in body2 space
	idVec3					offset;						// offset of body1 relative to body2
	idMat3					relAxis;					// rotation of body1 relative to body2

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// line constraint which allows 4 degrees of freedom
// constrains body1 to lie on a line relative to body2, does not constrain the orientation.
class idAFConstraint_Line : public idAFConstraint {

public:
							idAFConstraint_Line( const idStr &name, idAFBody *body1, idAFBody *body2 );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// plane constraint which allows 5 degrees of freedom
// constrains body1 to lie in a plane relative to body2, does not constrain the orientation.
class idAFConstraint_Plane : public idAFConstraint {

public:
							idAFConstraint_Plane( const idStr &name, idAFBody *body1, idAFBody *body2 );
	void					SetPlane( const idVec3 &normal, const idVec3 &anchor );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					anchor1;					// anchor in body1 space
	idVec3					anchor2;					// anchor in body2 space
	idVec3					planeNormal;				// plane normal in body2 space

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// spring constraint which allows 6 or 5 degrees of freedom based on the spring limits
// constrains body1 relative to body2 with a spring
class idAFConstraint_Spring : public idAFConstraint {

public:
							idAFConstraint_Spring( const idStr &name, idAFBody *body1, idAFBody *body2 );
	void					SetAnchor( const idVec3 &worldAnchor1, const idVec3 &worldAnchor2 );
	void					SetSpring( const float stretch, const float compress, const float damping, const float restLength );
	void					SetLimit( const float minLength, const float maxLength );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					anchor1;					// anchor in body1 space
	idVec3					anchor2;					// anchor in body2 space
	float					kstretch;					// spring constant when stretched
	float					kcompress;					// spring constant when compressed
	float					damping;					// spring damping
	float					restLength;					// rest length of spring
	float					minLength;					// minimum spring length
	float					maxLength;					// maximum spring length

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// constrains body1 to either be in contact with or move away from body2
class idAFConstraint_Contact : public idAFConstraint {

public:
							idAFConstraint_Contact( void );
							~idAFConstraint_Contact( void );
	void					Setup( idAFBody *b1, idAFBody *b2, contactInfo_t &c );
	const contactInfo_t &	GetContact( void ) const { return contact; }
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			GetCenter( idVec3 &center );

protected:
	contactInfo_t			contact;					// contact information
	idAFConstraint_ContactFriction *fc;					// contact friction

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// contact friction
class idAFConstraint_ContactFriction : public idAFConstraint {

public:
							idAFConstraint_ContactFriction( void );
	void					Setup( idAFConstraint_Contact *cc );
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:
	idAFConstraint_Contact *cc;							// contact constraint

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// constrains an axis attached to body1 to be inside a cone relative to body2
class idAFConstraint_ConeLimit : public idAFConstraint {

public:
							idAFConstraint_ConeLimit( void );
	void					Setup( idAFBody *b1, idAFBody *b2, const idVec3 &coneAnchor, const idVec3 &coneAxis,
									const float coneAngle, const idVec3 &body1Axis );
	void					SetAnchor( const idVec3 &coneAnchor );
	void					SetBody1Axis( const idVec3 &body1Axis );
	void					SetEpsilon( const float e ) { epsilon = e; }
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					coneAnchor;					// top of the cone in body2 space
	idVec3					coneAxis;					// cone axis in body2 space
	idVec3					body1Axis;					// axis in body1 space that should stay within the cone
	float					cosAngle;					// cos( coneAngle / 2 )
	float					sinHalfAngle;				// sin( coneAngle / 4 )
	float					cosHalfAngle;				// cos( coneAngle / 4 )
	float					epsilon;					// lcp epsilon

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// constrains an axis attached to body1 to be inside a pyramid relative to body2
class idAFConstraint_PyramidLimit : public idAFConstraint {

public:
							idAFConstraint_PyramidLimit( void );
	void					Setup( idAFBody *b1, idAFBody *b2, const idVec3 &pyramidAnchor,
									const idVec3 &pyramidAxis, const idVec3 &baseAxis,
									const float pyramidAngle1, const float pyramidAngle2, const idVec3 &body1Axis );
	void					SetAnchor( const idVec3 &pyramidAxis );
	void					SetBody1Axis( const idVec3 &body1Axis );
	void					SetEpsilon( const float e ) { epsilon = e; }
	bool					Add( idPhysics_AF *phys, float invTimeStep );
	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );
	virtual void			Save( idSaveGame *saveFile ) const;
	virtual void			Restore( idRestoreGame *saveFile );

protected:
	idVec3					pyramidAnchor;				// top of the pyramid in body2 space
	idMat3					pyramidBasis;				// pyramid basis in body2 space with base[2] being the pyramid axis
	idVec3					body1Axis;					// axis in body1 space that should stay within the cone
	float					cosAngle[2];				// cos( pyramidAngle / 2 )
	float					sinHalfAngle[2];			// sin( pyramidAngle / 4 )
	float					cosHalfAngle[2];			// cos( pyramidAngle / 4 )
	float					epsilon;					// lcp epsilon

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};

// vehicle suspension
class idAFConstraint_Suspension : public idAFConstraint {

public:
							idAFConstraint_Suspension( void );

	void					Setup( const char *name, idAFBody *body, const idVec3 &origin, const idMat3 &axis, idClipModel *clipModel );
	void					SetSuspension( const float up, const float down, const float k, const float d, const float f );

	void					SetSteerAngle( const float degrees ) { steerAngle = degrees; }
	void					EnableMotor( const bool enable ) { motorEnabled = enable; }
	void					SetMotorForce( const float force ) { motorForce = force; }
	void					SetMotorVelocity( const float vel ) { motorVelocity = vel; }
	void					SetEpsilon( const float e ) { epsilon = e; }
	const idVec3			GetWheelOrigin( void ) const;

	virtual void			DebugDraw( void );
	virtual void			Translate( const idVec3 &translation );
	virtual void			Rotate( const idRotation &rotation );

protected:
	idVec3					localOrigin;				// position of suspension relative to body1
	idMat3					localAxis;					// orientation of suspension relative to body1
	float					suspensionUp;				// suspension up movement
	float					suspensionDown;				// suspension down movement
	float					suspensionKCompress;		// spring compress constant
	float					suspensionDamping;			// spring damping
	float					steerAngle;					// desired steer angle in degrees
	float					friction;					// friction
	bool					motorEnabled;				// whether the motor is enabled or not
	float					motorForce;					// motor force
	float					motorVelocity;				// desired velocity
	idClipModel *			wheelModel;					// wheel model
	idVec3					wheelOffset;				// wheel position relative to body1
	trace_t					trace;						// contact point with the ground
	float					epsilon;					// lcp epsilon

protected:
	virtual void			Evaluate( float invTimeStep );
	virtual void			ApplyFriction( float invTimeStep );
};


//===============================================================
//
//	idAFBody
//
//===============================================================

typedef struct AFBodyPState_s {
	idVec3					worldOrigin;				// position in world space
	idMat3					worldAxis;					// axis at worldOrigin
	idVec6					spatialVelocity;			// linear and rotational velocity of body
	idVec6					externalForce;				// external force and torque applied to body
} AFBodyPState_t;


class idAFBody {

	friend class idPhysics_AF;
	friend class idAFTree;

public:
							idAFBody( void );
							idAFBody( const idStr &name, idClipModel *clipModel, float density );
							~idAFBody( void );

	void					Init( void );
	const idStr &			GetName( void ) const { return name; }
	const idVec3 &			GetWorldOrigin( void ) const { return current->worldOrigin; }
	const idMat3 &			GetWorldAxis( void ) const { return current->worldAxis; }
	const idVec3 &			GetLinearVelocity( void ) const { return current->spatialVelocity.SubVec3(0); }
	const idVec3 &			GetAngularVelocity( void ) const { return current->spatialVelocity.SubVec3(1); }
	idVec3					GetPointVelocity( const idVec3 &point ) const;
	const idVec3 &			GetCenterOfMass( void ) const { return centerOfMass; }
	void					SetClipModel( idClipModel *clipModel );
	idClipModel *			GetClipModel( void ) const { return clipModel; }
	void					SetClipMask( const int mask ) { clipMask = mask; fl.clipMaskSet = true; }
	int						GetClipMask( void ) const { return clipMask; }
	void					SetSelfCollision( const bool enable ) { fl.selfCollision = enable; }
	void					SetWorldOrigin( const idVec3 &origin ) { current->worldOrigin = origin; }
	void					SetWorldAxis( const idMat3 &axis ) { current->worldAxis = axis; }
	void					SetLinearVelocity( const idVec3 &linear ) const { current->spatialVelocity.SubVec3(0) = linear; }
	void					SetAngularVelocity( const idVec3 &angular ) const { current->spatialVelocity.SubVec3(1) = angular; }
	void					SetFriction( float linear, float angular, float contact );
	float					GetContactFriction( void ) const { return contactFriction; }
	void					SetBouncyness( float bounce );
	float					GetBouncyness( void ) const { return bouncyness; }
	float					GetVolume( void ) const { return volume; }
	void					SetDensity( float density, const idMat3 &inertiaScale = mat3_identity );
	float					GetInverseMass( void ) const { return invMass; }
	idMat3					GetInverseWorldInertia( void ) const { return current->worldAxis.Transpose() * inverseInertiaTensor * current->worldAxis; }

	void					SetFrictionDirection( const idVec3 &dir );
	bool					GetFrictionDirection( idVec3 &dir ) const;

	// returns the depth of the object in the water
	// 0.0f if out of water
	float					GetWaterLevel() const;
	float					SetWaterLevel( idPhysics_Liquid *l, const idVec3 &gravityNormal, bool fixedDensityBuoyancy );

	void					SetContactMotorDirection( const idVec3 &dir );
	bool					GetContactMotorDirection( idVec3 &dir ) const;
	void					SetContactMotorVelocity( float vel ) { contactMotorVelocity = vel; }
	float					GetContactMotorVelocity( void ) const { return contactMotorVelocity; }
	void					SetContactMotorForce( float force ) { contactMotorForce = force; }
	float					GetContactMotorForce( void ) const { return contactMotorForce; }

	void					AddForce( const idVec3 &point, const idVec3 &force );
	void					InverseWorldSpatialInertiaMultiply( idVecX &dst, const float *v ) const;
	idVec6 &				GetResponseForce( int index ) { return reinterpret_cast<idVec6 &>(response[ index * 8 ]); }

	void					Save( idSaveGame *saveFile );
	void					Restore( idRestoreGame *saveFile );

private:
							// properties
	idStr					name;						// name of body
	idAFBody *				parent;						// parent of this body
	idList<idAFBody *>		children;					// children of this body
	idClipModel *			clipModel;					// model used for collision detection
	idAFConstraint *		primaryConstraint;			// primary constraint (this->constraint->body1 = this)
	idList<idAFConstraint *>constraints;				// all constraints attached to this body
	idAFTree *				tree;						// tree structure this body is part of
	float					linearFriction;				// translational friction
	float					angularFriction;			// rotational friction
	float					contactFriction;			// friction with contact surfaces
	float					bouncyness;					// bounce
	float					volume;						// volume of body
	int						clipMask;					// contents this body collides with
	idVec3					frictionDir;				// specifies a single direction of friction in body space
	idVec3					contactMotorDir;			// contact motor direction
	float					contactMotorVelocity;		// contact motor velocity
	float					contactMotorForce;			// maximum force applied to reach the motor velocity

							// derived properties
	float					mass;						// mass of body
	float					invMass;					// inverse mass
	float					liquidMass;					// mass of object in a liquid
	float					invLiquidMass;				// inverse liquid mass
	float					waterLevel;					// percent of body in water
	idVec3					centerOfMass;				// center of mass of body
	idMat3					inertiaTensor;				// inertia tensor
	idMat3					inverseInertiaTensor;		// inverse inertia tensor

							// physics state
	AFBodyPState_t			state[2];
	AFBodyPState_t *		current;					// current physics state
	AFBodyPState_t *		next;						// next physics state
	AFBodyPState_t			saved;						// saved physics state
	idVec3					atRestOrigin;				// origin at rest
	idMat3					atRestAxis;					// axis at rest

							// simulation variables used during calculations
	idMatX					inverseWorldSpatialInertia;	// inverse spatial inertia in world space
	idMatX					I, invI;					// transformed inertia
	idMatX					J;							// transformed constraint matrix
	idVecX					s;							// temp solution
	idVecX					totalForce;					// total force acting on body
	idVecX					auxForce;					// force from auxiliary constraints
	idVecX					acceleration;				// acceleration
	float *					response;					// forces on body in response to auxiliary constraint forces
	int *					responseIndex;				// index to response forces
	int						numResponses;				// number of response forces
	int						maxAuxiliaryIndex;			// largest index of an auxiliary constraint constraining this body
	int						maxSubTreeAuxiliaryIndex;	// largest index of an auxiliary constraint constraining this body or one of it's children

	struct bodyFlags_s {
		bool				clipMaskSet			: 1;	// true if this body has a clip mask set
		bool				selfCollision		: 1;	// true if this body can collide with other bodies of this AF
		bool				spatialInertiaSparse: 1;	// true if the spatial inertia matrix is sparse
		bool				useFrictionDir		: 1;	// true if a single friction direction should be used
		bool				useContactMotorDir	: 1;	// true if a contact motor should be used
		bool				isZero				: 1;	// true if 's' is zero during calculations
	} fl;
};


//===============================================================
//
//	idAFTree
//
//===============================================================

class idAFTree {
	friend class idPhysics_AF;

public:
	void					Factor( void ) const;
	void					Solve( int auxiliaryIndex = 0 ) const;
	void					Response( const idAFConstraint *constraint, int row, int auxiliaryIndex ) const;
	void					CalculateForces( float timeStep ) const;
	void					SetMaxSubTreeAuxiliaryIndex( void );
	void					SortBodies( void );
	void					SortBodies_r( idList<idAFBody*>&sortedList, idAFBody *body );
	void					DebugDraw( const idVec4 &color ) const;

private:
	idList<idAFBody *>		sortedBodies;
};


//===============================================================
//
//	idPhysics_AF
//
//===============================================================

typedef struct AFPState_s {
	int						atRest;						// >= 0 if articulated figure is at rest
	float					noMoveTime;					// time the articulated figure is hardly moving
	float					activateTime;				// time since last activation
	float					lastTimeStep;				// last time step
	idVec6					pushVelocity;				// velocity with which the af is pushed
} AFPState_t;

typedef struct AFCollision_s {
	trace_t					trace;
	idAFBody *				body;
} AFCollision_t;


class idPhysics_AF : public idPhysics_Base {

public:
	CLASS_PROTOTYPE( idPhysics_AF );

							idPhysics_AF( void );
							~idPhysics_AF( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

							// initialisation
	int						AddBody( idAFBody *body );	// returns body id
	void					AddConstraint( idAFConstraint *constraint );
	void					AddFrameConstraint( idAFConstraint *constraint );
							// force a body to have a certain id
	void					ForceBodyId( idAFBody *body, int newId );
							// get body or constraint id
	int						GetBodyId( idAFBody *body ) const;
	int						GetBodyId( const char *bodyName ) const;
	int						GetConstraintId( idAFConstraint *constraint ) const;
	int						GetConstraintId( const char *constraintName ) const;
							// number of bodies and constraints
	int						GetNumBodies( void ) const;
	int						GetNumConstraints( void ) const;
							// retrieve body or constraint
	idAFBody *				GetBody( const char *bodyName ) const;
	idAFBody *				GetBody( const int id ) const;
	idAFBody *				GetMasterBody( void ) const { return masterBody; }
	idAFConstraint *		GetConstraint( const char *constraintName ) const;
	idAFConstraint *		GetConstraint( const int id ) const;
							// delete body or constraint
	void					DeleteBody( const char *bodyName );
	void					DeleteBody( const int id );
	void					DeleteConstraint( const char *constraintName );
	void					DeleteConstraint( const int id );
							// get all the contact constraints acting on the body
	int						GetBodyContactConstraints( const int id, idAFConstraint_Contact *contacts[], int maxContacts ) const;
							// set the default friction for bodies
	void					SetDefaultFriction( float linear, float angular, float contact );
							// suspend settings
	void					SetSuspendSpeed( const idVec2 &velocity, const idVec2 &acceleration );
							// set the time and tolerances used to determine if the simulation can be suspended when the figure hardly moves for a while
	void					SetSuspendTolerance( const float noMoveTime, const float translationTolerance, const float rotationTolerance );
							// set minimum and maximum simulation time in seconds
	void					SetSuspendTime( const float minTime, const float maxTime );
							// set the time scale value
	void					SetTimeScale( const float ts ) { timeScale = ts; }
							// set time scale ramp
	void					SetTimeScaleRamp( const float start, const float end );
							// set the joint friction scale
	void					SetJointFrictionScale( const float scale ) { jointFrictionScale = scale; }
							// set joint friction dent
	void					SetJointFrictionDent( const float dent, const float start, const float end );
							// get the current joint friction scale
	float					GetJointFrictionScale( void ) const;
							// set the contact friction scale
	void					SetContactFrictionScale( const float scale ) { contactFrictionScale = scale; }
							// set contact friction dent
	void					SetContactFrictionDent( const float dent, const float start, const float end );
							// get the current contact friction scale
	float					GetContactFrictionScale( void ) const;
							// enable or disable collision detection
	void					SetCollision( const bool enable ) { enableCollision = enable; }
							// enable or disable self collision
	void					SetSelfCollision( const bool enable ) { selfCollision = enable; }
							// enable or disable coming to a dead stop
	void					SetComeToRest( bool enable ) { comeToRest = enable; }
							// call when structure of articulated figure changes
	void					SetChanged( void ) { changedAF = true; }
							// enable/disable activation by impact
	void					EnableImpact( void );
	void					DisableImpact( void );
							// lock of unlock the world constraints
	void					LockWorldConstraints( const bool lock ) { worldConstraintsLocked = lock; }
							// set force pushable
	void					SetForcePushable( const bool enable ) { forcePushable = enable; }
							// update the clip model positions
	void					UpdateClipModels( void );

		// buoyancy stuff
	void					SetLiquidDensity( float density );
	float					GetLiquidDensity() const;

							// this will reset liquidDensity so be careful when using it
	void					SetFixedDensityBuoyancy( bool fixed );
	bool					GetFixedDensityBuoyancy() const;

public:	// common physics interface
	void					SetClipModel( idClipModel *model, float density, int id = 0, bool freeOld = true );
	idClipModel *			GetClipModel( int id = 0 ) const;
	int						GetNumClipModels( void ) const;

	void					SetMass( float mass, int id = -1 );
	float					GetMass( int id = -1 ) const;

	void					SetContents( int contents, int id = -1 );
	int						GetContents( int id = -1 ) const;

	const idBounds &		GetBounds( int id = -1 ) const;
	const idBounds &		GetAbsBounds( int id = -1 ) const;

	bool					Evaluate( int timeStepMSec, int endTimeMSec );
	void					UpdateTime( int endTimeMSec );
	int						GetTime( void ) const;

	void					GetImpactInfo( const int id, const idVec3 &point, impactInfo_t *info ) const;
	void					ApplyImpulse( const int id, const idVec3 &point, const idVec3 &impulse );
	void					AddForce( const int id, const idVec3 &point, const idVec3 &force );
	bool					IsAtRest( void ) const;
	int						GetRestStartTime( void ) const;
	void					Activate( void );
	void					PutToRest( void );
	bool					IsPushable( void ) const;

	void					SaveState( void );
	void					RestoreState( void );

	void					SetOrigin( const idVec3 &newOrigin, int id = -1 );
	void					SetAxis( const idMat3 &newAxis, int id = -1 );

	void					Translate( const idVec3 &translation, int id = -1 );
	void					Rotate( const idRotation &rotation, int id = -1 );

	const idVec3 &			GetOrigin( int id = 0 ) const;
	const idMat3 &			GetAxis( int id = 0 ) const;

	void					SetLinearVelocity( const idVec3 &newLinearVelocity, int id = 0 );
	void					SetAngularVelocity( const idVec3 &newAngularVelocity, int id = 0 );

	const idVec3 &			GetLinearVelocity( int id = 0 ) const;
	const idVec3 &			GetAngularVelocity( int id = 0 ) const;

	void					ClipTranslation( trace_t &results, const idVec3 &translation, const idClipModel *model ) const;
	void					ClipRotation( trace_t &results, const idRotation &rotation, const idClipModel *model ) const;
	int						ClipContents( const idClipModel *model ) const;

	void					DisableClip( void );
	void					EnableClip( void );

	void					UnlinkClip( void );
	void					LinkClip( void );

	bool					EvaluateContacts( void );

	void					SetPushed( int deltaTime );
	const idVec3 &			GetPushedLinearVelocity( const int id = 0 ) const;
	const idVec3 &			GetPushedAngularVelocity( const int id = 0 ) const;

	void					SetMaster( idEntity *master, const bool orientated = true );

	void					WriteToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadFromSnapshot( const idBitMsgDelta &msg );

private:
							// articulated figure
	idList<idAFTree *>		trees;							// tree structures
	idList<idAFBody *>		bodies;							// all bodies
	idList<idAFConstraint *>constraints;					// all frame independent constraints
	idList<idAFConstraint *>primaryConstraints;				// list with primary constraints
	idList<idAFConstraint *>auxiliaryConstraints;			// list with auxiliary constraints
	idList<idAFConstraint *>frameConstraints;				// constraints that only live one frame
	idList<idAFConstraint_Contact *>contactConstraints;		// contact constraints
	idList<int>				contactBodies;					// body id for each contact
	idList<AFCollision_t>	collisions;						// collisions
	bool					changedAF;						// true when the articulated figure just changed

							// properties
	float					linearFriction;					// default translational friction
	float					angularFriction;				// default rotational friction
	float					contactFriction;				// default friction with contact surfaces
	float					bouncyness;						// default bouncyness
	float					totalMass;						// total mass of articulated figure
	float					forceTotalMass;					// force this total mass

	idVec2					suspendVelocity;				// simulation may not be suspended if a body has more velocity
	idVec2					suspendAcceleration;			// simulation may not be suspended if a body has more acceleration
	float					noMoveTime;						// suspend simulation if hardly any movement for this many seconds
	float					noMoveTranslation;				// maximum translation considered no movement
	float					noMoveRotation;					// maximum rotation considered no movement
	float					minMoveTime;					// if > 0 the simulation is never suspended before running this many seconds
	float					maxMoveTime;					// if > 0 the simulation is always suspeded after running this many seconds
	float					impulseThreshold;				// threshold below which impulses are ignored to avoid continuous activation

	float					timeScale;						// the time is scaled with this value for slow motion effects
	float					timeScaleRampStart;				// start of time scale change
	float					timeScaleRampEnd;				// end of time scale change

	float					jointFrictionScale;				// joint friction scale
	float					jointFrictionDent;				// joint friction dives from 1 to this value and goes up again
	float					jointFrictionDentStart;			// start time of joint friction dent
	float					jointFrictionDentEnd;			// end time of joint friction dent
	float					jointFrictionDentScale;			// dent scale

	float					contactFrictionScale;			// contact friction scale
	float					contactFrictionDent;			// contact friction dives from 1 to this value and goes up again
	float					contactFrictionDentStart;		// start time of contact friction dent
	float					contactFrictionDentEnd;			// end time of contact friction dent
	float					contactFrictionDentScale;		// dent scale

	bool					enableCollision;				// if true collision detection is enabled
	bool					selfCollision;					// if true the self collision is allowed
	bool					comeToRest;						// if true the figure can come to rest
	bool					linearTime;						// if true use the linear time algorithm
	bool					noImpact;						// if true do not activate when another object collides
	bool					worldConstraintsLocked;			// if true world constraints cannot be moved
	bool					forcePushable;					// if true can be pushed even when bound to a master

							// physics state
	AFPState_t				current;
	AFPState_t				saved;
	bool					fixedDensityBuoyancy;			// treats liquid Density as THE density for each body when the AF is in liquid.
															// otherwise liquidDensity is just a gravity scalar for the AF in any liquid.
	float					liquidDensity;					// explained above.

	idAFBody *				masterBody;						// master body
	idLCP *					lcp;							// linear complementarity problem solver

private:
	void					BuildTrees( void );
	bool					IsClosedLoop( const idAFBody *body1, const idAFBody *body2 ) const;
	void					PrimaryFactor( void );
	void					EvaluateBodies( float timeStep );
	void					EvaluateConstraints( float timeStep );
	void					AddFrameConstraints( void );
	void					RemoveFrameConstraints( void );
	void					ApplyFriction( float timeStep, float endTimeMSec );
	void					PrimaryForces( float timeStep  );
	void					AuxiliaryForces( float timeStep );
	void					VerifyContactConstraints( void );
	void					SetupContactConstraints( void );
	void					ApplyContactForces( void );
	void					Evolve( float timeStep );
	idEntity *				SetupCollisionForBody( idAFBody *body ) const;
	bool					CollisionImpulse( float timeStep, idAFBody *body, trace_t &collision );
	bool					ApplyCollisions( float timeStep );
	void					CheckForCollisions( float timeStep );
	void					ClearExternalForce( void );
	void					AddGravity( void );
	void					SwapStates( void );
	bool					TestIfAtRest( float timeStep );
	void					Rest( void );
	void					AddPushVelocity( const idVec6 &pushVelocity );
	void					DebugDraw( void );
};

#endif /* !__PHYSICS_AF_H__ */
