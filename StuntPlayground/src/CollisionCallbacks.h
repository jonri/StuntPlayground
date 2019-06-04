/*
	Stunt Playground
	
	Collision Callbacks
*/

#ifndef _STUNTPLAYGROUND_COLLISIONCALLBACKS_
#define _STUNTPLAYGROUND_COLLISIONCALLBACKS_

#include <OgreNewt.h>
#include <Ogre.h>

#include "SoundManager.h"


namespace StuntPlayground
{


	// collision calback for the DummyBody.
	class DummyCollisionCallback : public OgreNewt::ContactCallback
	{
		// user-defined functions.
		int userBegin();

		int userProcess();

		void userEnd() {}
	
	
	};


	class CarArenaCallback : public OgreNewt::ContactCallback
	{
	public:
		void setIDs( unsigned int road, unsigned int ground, unsigned int roadchecker ) { mRoadID = road; mGroundID = ground; mRoadCheckerID = roadchecker; }
		void setCollisionSound( std::string name ) { mCarSnd = SoundManager::getSingleton().getSound( name ); }
	private:
		// user defined functions
		int userBegin() { return 1; }

		int userProcess();

		void userEnd() {}

		Sound* mCarSnd;
		
		unsigned int mRoadID;
		unsigned int mRoadCheckerID;
		unsigned int mGroundID;

	};




	class PropCollision : public OgreNewt::ContactCallback
	{
		int userBegin() ;

		int userProcess();

		void userEnd();
		
		Ogre::Real mNormSpeed;
		Ogre::Real mTangSpeed;

		Ogre::Vector3 mPosition;
	};

	
	class CarPropCollision : public OgreNewt::ContactCallback
	{
		int userBegin();

		int userProcess();

		void userEnd();

		bool mBodyHit;

		Ogre::Real mNormSpeed;
		Ogre::Real mTangSpeed;

		Ogre::Vector3 mPosition;
	};




}	// end NAMESPACE StuntPlayground


#endif	// _STUNTPLAYGROUND_COLLISIONCALLBACKS_
