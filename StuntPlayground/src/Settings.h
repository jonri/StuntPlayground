/*
	Stunt Playground
	
	Settings
*/

#ifndef _STUNTPLAYGROUND_SETTINGS_
#define _STUNTPLAYGROUND_SETTINGS_

#include "Ogre.h"

namespace StuntPlayground
{

	class GameSettings
	{
	public:

		void load( const Ogre::String& filename );
		void save( const Ogre::String& filename );

		std::string boolToString( bool setting )
		{
			if (setting)
			{
				return std::string("on");
			}
			else
			{
				return std::string("off");
			}
		}


		bool useManualTransmission;
		bool useRealtimeReflections;
		Ogre::ShadowTechnique shadowTechnique;
		unsigned int sfxVolume;
		bool restoreArena;

	};






}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_SETTINGS_