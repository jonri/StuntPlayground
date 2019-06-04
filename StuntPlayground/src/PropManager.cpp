#include "StuntPlaygroundApplication.h"
#include "PropManager.h"
#include <tinyxml.h>

namespace StuntPlayground
{



PropManager& PropManager::getSingleton()
{
	static PropManager instance;
	return instance;
}

void PropManager::init( Ogre::String proplistfile, OgreNewt::World* world, const OgreNewt::MaterialID* propmat, Ogre::SceneManager* mgr )
{
	mWorld = world;
	mPropMat = propmat;
	mSceneMgr = mgr;

	////////////////////////////////////////////////////////////////////
	// SEARCH FOR PROPS, AND FILL THE LISTBOX
	////////////////////////////////////////////////////////////////////
	TiXmlDocument doc;

	doc.LoadFile( proplistfile );

	TiXmlElement* rootelement = doc.RootElement();

	if (!rootelement)
	{
#ifdef _SP_DEBUG_LOG
		Application::getSingletonPtr()->mDebugLog->logMessage("Cannot find props definition!!");
#endif

		return;
	}

	//loop through all of the props, making objects for them.
	for (TiXmlElement* prop = rootelement->FirstChildElement("prop"); prop; prop = prop->NextSiblingElement( "prop" ) )
	{
		std::string name = prop->Attribute( "name" );
		PropDefinition pd;

		pd.mBodyFile = prop->Attribute( "bodfile" );
		pd.mImageSet = prop->Attribute( "imageset" );
		pd.mImage = prop->Attribute( "image" );
		pd.mHitSound = prop->Attribute( "sound" );

		mPropDefinitions[ name ] = pd;		
	}

}

Prop* PropManager::addProp( Ogre::String name, Ogre::Vector3 pos, Ogre::Quaternion orient )
{
	Prop* prp = NULL;

#ifdef _SP_DEBUG_LOG
	Application::getSingleton().mDebugLog->logMessage(" -P- PropManager::load prop: "+name);
#endif	// _SP_DEBUG_LOG

	PropDefinitionIterator it = mPropDefinitions.find( (std::string)name );

	if (it == mPropDefinitions.end())
	{
		//no prop with this name!
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -P- PropManager::addProp - cannot find definition for prop name: "+name);
#endif	// _SP_DEBUG_LOG
		Application::getSingleton().showErrorMessage("Error! no prop with name "+name+" exists!", false);
		return prp;
	}

	prp = new Prop;
	prp->load( name, count++, mWorld, mPropMat, it->second.mBodyFile, orient, pos, mSceneMgr, it->second.mHitSound );

	mProps.push_back( prp );

	return mProps.back();
}

void PropManager::removeProp( Prop* prop )
{
	// find this prop in the list.
	bool found = false;

	
	for (PropListIterator it=mProps.begin(); it!= mProps.end(); ++it)
	{
		if (prop == (*it))
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		prop->remove( mSceneMgr );
		mProps.remove( prop );
		delete prop;
	}		

}

PropManager::PropDefinition PropManager::getPropDefinition( std::string name )
{
	PropDefinition pd;

	PropDefinitionIterator it = mPropDefinitions.find( name );

	if (it != mPropDefinitions.end())
	{
		//found the prop definition, return it.
		pd = it->second; 
	}

	return pd;

}




PropManager::PropManager()
{
	mSceneMgr = NULL;
	count = 0;
}

PropManager::~PropManager()
{
	for (PropListIterator it = mProps.begin(); it != mProps.end(); it++)
	{
		(*it)->remove( mSceneMgr );
		delete (Prop*)(*it);
	}

	mProps.clear();
}




}	// end NAMESPACE StuntPlayground
