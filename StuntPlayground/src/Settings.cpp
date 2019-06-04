#include "StuntPlaygroundApplication.h"
#include "Settings.h"

#include <tinyxml.h>


namespace StuntPlayground
{


	void GameSettings::load( const Ogre::String& filename  )
	{
		TiXmlDocument doc;

		doc.LoadFile( (std::string)filename );


		TiXmlElement* root = doc.RootElement();

		if (!root)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -S- Settings::load - cannot find 'root' in "+filename);
#endif	// _SP_DEBUG_LOG
			return;
		}

		// load settings...
		TiXmlElement* elem;

		// SHADOW TECHNIQUE
		// set default
		shadowTechnique = Ogre::SHADOWTYPE_STENCIL_MODULATIVE;

		elem = root->FirstChildElement( "shadows" );

		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -S- Settings::load - cannot find shadow settings");
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			if (elem->Attribute("setting") == boolToString(false))
			{ 
				shadowTechnique = Ogre::SHADOWTYPE_NONE; 
			}
		}

		// REALTIME REFLECTIONS
		// set default
		useRealtimeReflections = true;

		elem = root->FirstChildElement( "reflections" );

		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -S- Settings::load - cannot find reflection settings");
#endif	// _SP_DEBUG_LOG
		}
		else
		{
			if (elem->Attribute("setting") == boolToString(false)) 
			{ 
				useRealtimeReflections = false; 
			}
		}

		// MANUAL TRANSMISSION SETTINGS
		// set default
		useManualTransmission = false;

		elem = root->FirstChildElement("manualtransmission");

		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -S- Settings::load - cannot find manual transmission settings");
#endif	//_SP_DEBUG_LOG
		}
		else
		{
			if (elem->Attribute("setting") == boolToString(true)) 
			{ 
				useManualTransmission = true; 
			}
		}


		// SOUND EFFECTS MASTER VOLUME
		// set default
		sfxVolume = 128;

		elem = root->FirstChildElement("soundvolume");

		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -S- Settings::load - cannot find sound volume settings");
#endif	//_SP_DEBUG_LOG
		}
		else
		{
			int vol = Ogre::StringConverter::parseInt( elem->Attribute("setting") );
			if (vol<0) { vol = 0; }
			if (vol > 255) { vol = 255; }

			sfxVolume = (unsigned int)vol;
		}


		// RESTORE ARENA SETTINGS
		// set default
		restoreArena = true;

		elem = root->FirstChildElement("restorearena");

		if (!elem)
		{
#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -S- Settings::load - cannot find restore arena settings");
#endif	//_SP_DEBUG_LOG
		}
		else
		{
			if (elem->Attribute("setting") == boolToString(false)) 
			{ 
				restoreArena = false; 
			}
		}

	}



	void GameSettings::save( const Ogre::String& filename )
	{
		TiXmlDocument doc;

		// root element.
		TiXmlElement root( "GameSettings" );


		// various settings.
		TiXmlElement shadows( "shadows" );
		if (shadowTechnique == Ogre::SHADOWTYPE_STENCIL_MODULATIVE)
		{
			shadows.SetAttribute( "setting", "on" );
		}
		else
		{
			shadows.SetAttribute( "setting", "off" );
		}

		root.InsertEndChild( shadows );


		TiXmlElement reflections("reflections");
		reflections.SetAttribute("setting", boolToString(useRealtimeReflections));

		root.InsertEndChild( reflections );


		TiXmlElement manual("manualtransmission");
		manual.SetAttribute("setting", boolToString(useManualTransmission));

		root.InsertEndChild( manual );


		TiXmlElement vol("soundvolume");
		vol.SetAttribute("setting", (std::string)Ogre::String( Ogre::StringConverter::toString(sfxVolume) ) );

		root.InsertEndChild( vol );


		TiXmlElement restore("restorearena");
		restore.SetAttribute("setting", boolToString(restoreArena));

		root.InsertEndChild( restore );


		// declaration
		doc.InsertEndChild( root );

		doc.SaveFile( (std::string)filename );

	}



}	// end NAMESPACE StuntPlayground

