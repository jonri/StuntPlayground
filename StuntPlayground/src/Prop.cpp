/*
	Stunt Playground
	
	Prop
*/
#include ".\StuntPlaygroundApplication.h"
#include "Prop.h"
#include "StuntPlayground.h"
#include "RigidBodyLoader.h"
#include "WorldRecorder.h"
#include <OgreNewt.h>

namespace StuntPlayground
{

	Prop::Prop()
	{
		m_Name = "";

		mRecObj = NULL;

		m_HitSound = "";

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P- Prop constructor.");
#endif	// _SP_DEBUG_LOG

	}

	Prop::~Prop()
	{
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P- Prop Destructor.");
#endif	// _SP_DEBUG_LOG

	}



	void Prop::load( Ogre::String name, int count, OgreNewt::World* world, const OgreNewt::MaterialID* mat, Ogre::String& filename, 
		Ogre::Quaternion& proporient, Ogre::Vector3& proppos, Ogre::SceneManager* mgr, Ogre::String hitsound)
	{
		m_Name = name;

		m_Body = StuntPlayground::loadRigidBody( filename, name+"_Entity"+Ogre::StringConverter::toString(count), world, mgr->getRootSceneNode(), mgr );

		m_Body->setPositionOrientation( proppos, proporient );
		m_Body->setMaterialGroupID( mat );
		m_Body->setType( (int)StuntPlayground::BT_PROP );
		m_Body->setUserData(this);

		m_Body->setCustomTransformCallback( Prop::transformCallback );

		m_HitSound = hitsound;


#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P- Prop Loaded. name:"+name+" filename:"+filename);
#endif	// _SP_DEBUG_LOG

	}

	void Prop::remove( Ogre::SceneManager* mgr )
	{
		
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P- Prop::remove.");
#endif	// _SP_DEBUG_LOG

		// first, get the node for this prop.
		Ogre::SceneNode* node = m_Body->getOgreNode();

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P-  deleting movable objects body...");
#endif	// _SP_DEBUG_LOG

		// now get rid of the mesh and the scene node itself.
		while (node->numAttachedObjects() > 0)
		{
			Ogre::MovableObject* obj = node->detachObject(static_cast<unsigned short>(0));

#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -P- removing entity :"+obj->getName());
#endif	// _SP_DEBUG_LOG

			mgr->destroyEntity( (Ogre::Entity*)obj );

		}

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P-  deleting scene node...");
#endif	// _SP_DEBUG_LOG

		Ogre::SceneNode* todestroy = (Ogre::SceneNode*)node->getParent()->removeChild( node->getName() );
		mgr->destroySceneNode( todestroy->getName() );

		m_Body->attachToNode(NULL);

		const OgreNewt::Collision* col = m_Body->getCollision();
		delete col;

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P-  deleting rigid body...");
#endif	// _SP_DEBUG_LOG

		// now destroy the rigid body itself.
		delete m_Body;
		m_Body = NULL;


#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P- Prop Removed." );
#endif	// _SP_DEBUG_LOG
		
	}

	void Prop::transformCallback( OgreNewt::Body* me, const Ogre::Quaternion& orient, const Ogre::Vector3& pos )
	{
		me->getOgreNode()->setPosition( pos );
		me->getOgreNode()->setOrientation( orient );

		Prop* prop = (Prop*)me->getUserData();

		if (prop)
		{
			if (prop->mRecObj)
				prop->mRecObj->addKeyframe();
		}
	}

	void Prop::setupRecording( Ogre::Real minwait )
	{
		mRecObj = WorldRecorder::getSingleton().addRecordableObject( m_Body->getOgreNode(), minwait );
	}





}	// end NAMESPACE StuntPlayground