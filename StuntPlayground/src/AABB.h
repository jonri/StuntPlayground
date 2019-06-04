/*
	Stunt Playground
	
	DummyBody
*/

#ifndef _STUNTPLAYGROUND_AABB_
#define _STUNTPLAYGROUND_AABB_

#include <Ogre.h>

namespace StuntPlayground
{

	/*
		Stunt Playground

		AABB class

		simple AABB - visualizer, used to highliting objects, etc.
	*/
	class MyAABB
	{
		Ogre::SceneNode* mNode;
		Ogre::SceneManager* mSceneMgr;

		Ogre::AxisAlignedBox mBox;
		Ogre::String mMatName;


	public:
		MyAABB( Ogre::String name, Ogre::SceneManager* mgr, Ogre::String boxmesh, Ogre::String materialname )
		{
			mSceneMgr = mgr;

			// create the entity, etc.
			Ogre::Entity* ent = mgr->createEntity(name, boxmesh);
			mNode = mgr->getRootSceneNode()->createChildSceneNode();
			mNode->attachObject(ent);
			ent->setCastShadows(false);
			ent->setMaterialName( materialname );
			mNode->setVisible(false);

			mMatName = "";
			mBox = Ogre::AxisAlignedBox(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
		}

		~MyAABB() 
		{
			// delete the scene node and entity.
			Ogre::Entity* ent = (Ogre::Entity*)mNode->detachObject(static_cast<unsigned short>(0));

			mSceneMgr->SceneManager::destroyEntity( ent->getName() );

			mNode->getParentSceneNode()->removeAndDestroyChild( mNode->getName() );	
		}

		void setSize( Ogre::Vector3 size ) { mNode->setScale( size ); }
		Ogre::Vector3 getSize() { return mNode->getScale(); }

		void setMaterial( Ogre::String matname ) 
		{ 
			if (matname == mMatName)
				return;
			
			mMatName = matname;
			((Ogre::Entity*)mNode->getAttachedObject(0))->setMaterialName(matname); 
		}

		void setVisible( bool setting ) { mNode->setVisible(setting); }

		void setToAABB( Ogre::AxisAlignedBox box )
		{
			Ogre::Vector3 center;
			Ogre::Vector3 size;

			if ((box.getMaximum() == mBox.getMaximum()) || (box.getMinimum() == mBox.getMinimum()))
				return;

			size = box.getMaximum() - box.getMinimum();

			center = box.getMinimum() + (size/2.0);

			mNode->setPosition( center );
			setSize( size );
		}

	};


}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_AABB_