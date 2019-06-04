/*
	Stunt Playground
	
	DummyBody
*/

#ifndef _STUNTPLAYGROUND_DUMMYBODY_
#define _STUNTPLAYGROUND_DUMMYBODY_

#include <Ogre.h>
#include <OgreNewt.h>
#include <OgreNewt_Body.h>

namespace StuntPlayground
{

	/*
		Stunt Playground

		DummyBody class

		dummy rigid body, used for prop placement collision detection, etc.
	*/
	class DummyBody
	{

	private:
		//private member variables.

		// Rigid Body, the base of the class.
		OgreNewt::Body* m_Body;
		Ogre::Vector3 m_Size;

		Ogre::Vector3 mGoalPos;
		Ogre::Quaternion mGoalOrient;

		Ogre::Real mSpringConst;
		Ogre::Real mSpringDamp;
		
		const OgreNewt::MaterialID* m_MyMat;

		// attached body.
		OgreNewt::Body* m_Attached;
		const OgreNewt::MaterialID* m_AttachedMat;

		// possibly attached body.
		OgreNewt::Body* m_PossibleAttach;

		Ogre::SceneNode* mRotNode;

		void setCollision( const OgreNewt::Collision* newcollision, Ogre::Vector3 nodescale = Ogre::Vector3(1,1,1) );

	public:
		DummyBody( OgreNewt::World* world, const OgreNewt::MaterialID* mat, Ogre::Vector3& size, Ogre::SceneManager* mgr );
			
		~DummyBody()
		{
			// delete the body!
			delete m_Body;
		}

		// attach the dummy to a specific Body
		void attachBody( OgreNewt::Body* body, const OgreNewt::MaterialID* tempMat );

		// detach from the current attached body.
		void detach();

		OgreNewt::Body* getAttached() { return m_Attached; }

		void setSize( Ogre::Vector3& size );

		OgreNewt::Body* getNewtonBody() { return m_Body; }

		void setGoalPositionOrientation( Ogre::Vector3& pos, Ogre::Quaternion& orient );
		void getGoalPositionOrientation( Ogre::Vector3& pos, Ogre::Quaternion& orient );

		void setSpringConst( Ogre::Real constant ) { mSpringConst = constant; }
		Ogre::Real getSpringConst() { return mSpringConst; }

		void setSpringDamp( Ogre::Real damp ) { mSpringDamp = damp; }
		Ogre::Real getSpringDamp() { return mSpringDamp; }

		OgreNewt::Body* getPossibleAttach() { return m_PossibleAttach; }
		void setPossibleAttach( OgreNewt::Body* body ) { m_PossibleAttach = body; }

		void attachToCurrentPossible( const OgreNewt::MaterialID* material );

		void setRotRingOrient( StuntPlayground::RotAxis axis );


	};




}	// end NAMESPACE StuntPlayground


#endif _STUNTPLAYGROUND_DUMMYBODY_