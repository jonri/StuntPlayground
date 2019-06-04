/*
	Stunt Playground
	
	Prop
*/

#ifndef _STUNTPLAYGROUND_PROP_
#define _STUNTPLAYGROUND_PROP_

#include <OgreNewt.h>
#include <Ogre.h>
#include "WorldRecorder.h"

namespace StuntPlayground
{

	/*
		Stunt Playground

		Prop class

		all in-game jumps and other physics props
	*/
	class Prop
	{
	private:
		// name
		Ogre::String m_Name;

		Ogre::String m_HitSound;
		
		// Rigid Body, the base of the class.
		OgreNewt::Body* m_Body;
	
	public:
		WorldRecorder::RecordableObject* mRecObj;

	private:
		Ogre::Real buffer[24];

	public:
		Prop();			// constructor
		~Prop();		// destructor

		void load( Ogre::String name, int count, OgreNewt::World* world, const OgreNewt::MaterialID* mat, Ogre::String& filename, 
			Ogre::Quaternion& orient, Ogre::Vector3& pos, Ogre::SceneManager* mgr, Ogre::String hitsound );

		void remove( Ogre::SceneManager* mgr );

		const Ogre::String& getName() { return m_Name; }
		OgreNewt::Body* getNewtonBody() { return m_Body; }
		
		void setupRecording( Ogre::Real minwait );

		Ogre::String getHitSound() { return m_HitSound; }


		static void transformCallback( OgreNewt::Body* me, const Ogre::Quaternion& orient, const Ogre::Vector3& pos );

	};




}	// end NAMESPACE StuntPlayground


#endif _STUNTPLAYGROUND_PROP_