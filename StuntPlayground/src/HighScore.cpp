#include "StuntPlaygroundApplication.h"

#include "HighScore.h"

#include <tinyxml.h>


namespace StuntPlayground
{


	void HighScore::save( std::string filename )
	{
		TiXmlDocument doc;

		TiXmlElement root("high_scores");

		TiXmlElement airtime("airtime");
		airtime.SetAttribute("value", Ogre::StringConverter::toString(mTime) );
		root.InsertEndChild( airtime );

		TiXmlElement dist("dist");
		dist.SetAttribute("value", Ogre::StringConverter::toString(mDist) );
		root.InsertEndChild( dist );

		TiXmlElement twowheels("two_wheels");
		twowheels.SetAttribute("value", Ogre::StringConverter::toString(m2WH) );
		root.InsertEndChild( twowheels );

		TiXmlElement flips("flips");
		flips.SetAttribute("value", Ogre::StringConverter::toString(mFlips) );
		root.InsertEndChild( flips );

		doc.InsertEndChild(root);

		doc.SaveFile( filename );
	}


	void HighScore::load( std::string filename )
	{
		TiXmlDocument doc;

		doc.LoadFile( filename );

		TiXmlElement* root = doc.RootElement();

		if (!root)
		{
			// error!
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -H- HighScore::load - cannot find 'root' in "+filename );
#endif
			return;
		}

		// airtime record.
		TiXmlElement* elem = root->FirstChildElement("airtime");
		if (elem)
		{
			mTime = Ogre::StringConverter::parseReal( elem->Attribute("value") );
		}

		// distance record.
		elem = root->FirstChildElement("dist");
		if (elem)
		{
			mDist = Ogre::StringConverter::parseReal( elem->Attribute("value") );
		}

		// 2-wheels record.
		elem = root->FirstChildElement("two_wheels");
		if (elem)
		{
			m2WH = Ogre::StringConverter::parseReal( elem->Attribute("value") );
		}

		// flips record.
		elem = root->FirstChildElement("flips");
		if (elem)
		{
			mFlips = Ogre::StringConverter::parseReal( elem->Attribute("value") );
		}

	}






}	// end NAMESPACE StuntPlayground