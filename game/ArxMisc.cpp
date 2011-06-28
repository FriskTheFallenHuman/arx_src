// 23rd April 2010 - Solarsplace - Arx End Of Sun

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "ArxMisc.h"

//const float	idArxMisc::PI				= 3.14159265358979323846f;

/*
===============
idPlayerCursor::idPlayerCursor
===============
*/
idArxMisc::idArxMisc() {
   //cursorHandle   = -1;
   //created = false;
}

/*
===============
idPlayerCursor::~idPlayerCursor
===============
*/
idArxMisc::~idArxMisc() {
   //FreeCursor();
}

compass_quadrant idArxMisc::vectorToCompassQuadrant( idVec2 v )
{
	// Credits: http://www.idevgames.com/forum/showthread.php?t=10652

	v.Normalize();

	// you could cache this as a literal ( 0.866025 )
	const float cos30  = cosf( 30.0f * idMath::PI / 180.0f );

	if ( v.x >= 0 && v.y >= 0 )
	{
		if ( v.x > cos30 ) return East;
		if ( v.y > cos30 ) return North;
		return NorthEast;
	}

	if ( v.x >= 0 && v.y <= 0 )
		{
		if ( v.x > cos30 ) return East;
		if ( v.y < -cos30 ) return South;
		return SouthEast;      
	}

	if ( v.x <= 0 && v.y <= 0 )
		{
		if ( v.x < -cos30 ) return West;
		if ( v.y < -cos30 ) return South;
		return SouthWest;
	}

	if ( v.x < -cos30 ) return West;
	if ( v.y > cos30 ) return North;
	return NorthWest;
}