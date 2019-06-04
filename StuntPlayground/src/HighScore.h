/*
	Stunt Playground
	
	MainFrameListener
*/
#ifndef _STUNTPLAYGROUND_HIGHSCORE_
#define _STUNTPLAYGROUND_HIGHSCORE_

#include <Ogre.h>

namespace StuntPlayground
{



	// simple class for storing high scores.
	class HighScore
	{
	public:
		void load( std::string filename );
		void save( std::string filename );

		bool possibleJumpTime( Ogre::Real time ) { if (time > mTime) { mTime = time; return true; } return false; }
		bool possibleJumpDist( Ogre::Real dist ) { if (dist > mDist) { mDist = dist; return true; } return false; }
		bool possible2Wheels( Ogre::Real time ) { if (time > m2WH) { m2WH = time; return true; } return false; }
		bool possibleFlips( Ogre::Real flips ) { if (flips > mFlips) { mFlips = flips; return true; } return false; }

		Ogre::Real mTime;
		Ogre::Real mDist;
		Ogre::Real m2WH;
		Ogre::Real mFlips;
	};


}	// end NAMESPACE StuntPlayground





#endif	// _STUNTPLAYGROUND_HIGHSCORE_