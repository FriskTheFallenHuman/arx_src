
#ifndef   __PLAYER_ARXMISC_H__
#define   __PLAYER_ARXMISC_H__

/*
Created on 23rd April 2010 by Solarsplace
*/

enum compass_quadrant {
   North,		// 0
   NorthEast,	// 1
   East,		// 2
   SouthEast,	// 3
   South,		// 4
   SouthWest,	// 5
   West,		// 6
   NorthWest	// 7
};

class idArxMisc {
	public:
	idArxMisc();
	~idArxMisc();
	//void   Draw( const idVec3 &origin,const idMat3 &axis,const char *material );

public:
	//renderEntity_t      renderEnt;
	//qhandle_t         cursorHandle;
	//bool            created;

public:
	compass_quadrant vectorToCompassQuadrant( const idVec2 v );
	//void   FreeCursor( void );
	//bool   CreateCursor( idPlayer* player , const idVec3 &origin, const idMat3 &axis,const char *material);
	//void   UpdateCursor( idPlayer* player ,const idVec3 &origin, const idMat3 &axis);


};
#endif   /* !_PLAYER_ARXMISC_H_ */
