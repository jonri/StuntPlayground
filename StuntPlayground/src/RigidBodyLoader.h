/*
	Stunt Playground
	
	RigidBodyLoader function.
*/

#ifndef _STUNTPLAYGROUND_RIGIDBODYLOADER
#define _STUNTPLAYGROUND_RIGIDBODYLOADER

#include <OgreNewt.h>
#include <Ogre.h>
#include "StuntPlayground.h"

namespace StuntPlayground
{

	OgreNewt::Body* loadRigidBody( Ogre::String& filename, Ogre::String& entityname, OgreNewt::World* world, Ogre::SceneNode* parentnode,
		Ogre::SceneManager* mgr, int colID = 0 );


}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_RIGIDBODYLOADER
