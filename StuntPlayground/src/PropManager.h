/*
	Stunt Playground
	
	PropManager
*/

#ifndef _STUNTPLAYGROUND_PROPMANAGER_
#define _STUNTPLAYGROUND_PROPMANAGER_

#include "Prop.h"

namespace StuntPlayground
{


	// Prop manager - for adding/removing/managing props in the arena.
	class PropManager
	{
	public:
		struct PropDefinition
		{
			std::string mBodyFile;
			std::string mImageSet;
			std::string mImage;
			std::string mHitSound;
		};

		typedef std::map<std::string, PropDefinition> PropDefinitionMap;
		typedef std::map<std::string, PropDefinition>::iterator PropDefinitionIterator;
		typedef std::list<Prop*>::iterator PropListIterator;

		static PropManager& getSingleton();

		void init( Ogre::String proplisfile, OgreNewt::World* world, const OgreNewt::MaterialID* propmat, Ogre::SceneManager* mgr );


		Prop* addProp( Ogre::String name, Ogre::Vector3 pos, Ogre::Quaternion orient );
		void removeProp( Prop* prop );

		PropDefinition getPropDefinition( std::string name );
		PropDefinitionMap getPropDefinitionMap() { return mPropDefinitions; }

		PropListIterator getPropListIteratorBegin() { return mProps.begin(); }
		PropListIterator getPropListIteratorEnd() { return mProps.end(); }

	private:

		typedef std::list<Prop*> PropList;
		

		PropManager();
		~PropManager();

		PropList mProps;
		
		PropDefinitionMap mPropDefinitions;

		Ogre::SceneManager* mSceneMgr;
		OgreNewt::World* mWorld;
		const OgreNewt::MaterialID* mPropMat;

		int count;

	};








}	// end NAMESPACE StuntPlayground



#endif	// _STUNTPLAYGROUND_PROPMANAGER_
