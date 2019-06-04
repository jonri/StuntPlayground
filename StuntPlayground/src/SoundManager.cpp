#include "StuntPlaygroundApplication.h"
#include "SoundManager.h"

#include <fstream>


namespace StuntPlayground
{

	//////////////////////////////////////////////////////////////////////////////
	//			SOUND FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////
	Sound::Sound( std::string name, std::string filename ) : mName(name)
	{
		// check the file extension
		std::string ext = "";
		size_t offset = filename.find_last_of(".");

		ext = filename.substr(offset);

		if (ext == ".sound")
		{
			//this is an encoded file, we need to decode it before loading.
			ifstream in(filename.c_str(), ios::binary);
			

			char buffer;
			std::vector<char> bigbuffer;

			while ( in.read(&buffer, 1) )
				bigbuffer.push_back(buffer);
			
			char* decoded = new char[bigbuffer.size()];

			int off = 0;
			for (int i=bigbuffer.size()-1; i >= 0; i--)
				decoded[off++] = bigbuffer[i];

			

			mPtr = FSOUND_Sample_Load( FSOUND_FREE, decoded, FSOUND_LOADMEMORY, 0, bigbuffer.size() );
		}
		else
		{
			mPtr = FSOUND_Sample_Load( FSOUND_FREE, filename.c_str(), 0, 0, 0);
		}

		if (!mPtr)
		{
			// ERROR!!!!
		}

		mState = PlayState::PS_STOPPED;
	}

	Sound::~Sound()
	{
		FSOUND_Sample_Free( mPtr );
	}

	void Sound::play()
	{
		if (mState != PlayState::PS_PAUSED)
		{
			mChannel = FSOUND_PlaySound( FSOUND_FREE, mPtr );
			mTimer.reset();
		}
		else
		{
			FSOUND_SetPaused( mChannel, false );
		}
		
		mState = PlayState::PS_PLAYING;
	}

	// overridden play with sets volume.
	void Sound::play( int vol )
	{
		play();
		setVolume( vol );
	}

	// overridden play which also sets position, velocity, and volume
	void Sound::play(int vol, const Ogre::Vector3& pos, const Ogre::Vector3& vel )
	{
		play();
		setPositionVelocity(pos, vel);
		setVolume(vol);
	}

	void Sound::pause()
	{
		if (isPlaying())
		{
			FSOUND_SetPaused( mChannel, true );
			mState = PlayState::PS_PAUSED;
		}
	}

	void Sound::stop()
	{
		if (isPlaying())
		{
			FSOUND_StopSound( mChannel );
			mChannel = -1;
		}
		mState = PlayState::PS_STOPPED;

	}

	bool Sound::isPlaying()
	{
		if ((mState == PlayState::PS_PLAYING))
		{
			unsigned char ret = FSOUND_IsPlaying( mChannel );

			if (ret == false)
			{
				if (!mState == PlayState::PS_PAUSED) { mState = PlayState::PS_STOPPED; }
			}

			return ret;
		}

		return false;
	}

	void Sound::setPositionVelocity( const Ogre::Vector3& pos, const Ogre::Vector3& vel )
	{
		if (isPlaying())
		{
			Ogre::Vector3 mPos = pos;
			Ogre::Vector3 mVel = vel;
			mPos.z = -mPos.z;
			mVel.z = -mVel.z;
			FSOUND_3D_SetAttributes( mChannel, &mPos.x, &mVel.x );
		}
	}

	void Sound::getPositionVelocity( Ogre::Vector3& pos, Ogre::Vector3& vel )
	{
		if (isPlaying())
		{
			FSOUND_3D_GetAttributes( mChannel, &pos.x, &vel.x );
			pos.z = -pos.z;
			vel.z = -vel.z;
		}
	}



	void Sound::setVolume( int vol )
	{
		if (isPlaying())
		{
			if (vol < 0) { vol = 0; }
			if (vol > 255) { vol = 255; }

			FSOUND_SetVolume( mChannel, vol );
		}
	}

	int Sound::getVolume()
	{
		if (isPlaying())
		{
			return FSOUND_GetVolume( mChannel );
		}

		return -1;
	}

	void Sound::setFrequency( int freq )
	{
		if (isPlaying())
		{
			FSOUND_SetFrequency( mChannel, freq );
		}
	}

	int Sound::getFrequency()
	{
		if (isPlaying())
		{
			return FSOUND_GetFrequency( mChannel );
		}

		return -1;
	}

	void Sound::setLoop( bool onoff )
	{
		bool wasplaying = false;

		if (isPlaying())
		{ 
			wasplaying = true; 
			pause(); 
		}
		else
		{
			play();
			pause();
		}

		if (onoff)
		{
			FSOUND_SetLoopMode( mChannel, FSOUND_LOOP_NORMAL );
		}
		else
		{
			FSOUND_SetLoopMode( mChannel, FSOUND_LOOP_OFF );
		}

		if (wasplaying) { play(); }
	}





	//////////////////////////////////////////////////////////////////////////////
	//			SOUNDMANAGER FUNCTIONS
	//////////////////////////////////////////////////////////////////////////////
	SoundManager::SoundManager()
	{
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -S- SoundManager:: constructor");
#endif	// _SP_DEBUG_LOG
	}

	SoundManager::~SoundManager()
	{
		for (SoundMapIterator it=mSounds.begin(); it != mSounds.end(); it++)
			delete it->second;

		mSounds.clear();

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -S- SoundManager:: destructor");
#endif	// _SP_DEBUG_LOG
	}


	SoundManager& SoundManager::getSingleton()
	{
		static SoundManager instance;
		return instance;
	}

	void SoundManager::Init( MixRate rate, int numSoftChannels )
	{
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -S- SoundManager:: init -> rate:"+Ogre::StringConverter::toString(rate)+
			" numsoftchanels: "+Ogre::StringConverter::toString(numSoftChannels));
#endif	// _SP_DEBUG_LOG

		signed char ret = FSOUND_Init( rate, numSoftChannels, 0 );
	}

	Sound* SoundManager::addSound( std::string name, std::string filename )
	{
		// does a sound with this name already exist?
		SoundMapIterator exist = mSounds.find(name);

		if (exist != mSounds.end())
		{

			// sound with this name already exists!
#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -S- SoundManager:: error creating sound "+
				name + ": a sound with that name already exists!");
#endif	// _SP_DEBUG_LOG			
			return NULL;
		}
		
		Sound* newsound = new Sound( name, filename );

		mSounds[name] = newsound;

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -S- SoundManager:: addSound -> added sound name: "+name+" from file: "+filename);
#endif	// _SP_DEBUG_LOG

		return newsound;
	}

	void SoundManager::removeSound( std::string name )
	{
		// sound with this name exists?
		SoundMapIterator sound = mSounds.find(name);

		if (sound != mSounds.end())
		{
			// delete the sound.
			Sound* ptr = sound->second;
			delete ptr;

			mSounds.erase( name );
		}
	}

	void SoundManager::playSound( std::string name )
	{
		SoundMapIterator sound = mSounds.find(name);

		if (sound != mSounds.end())
		{
			sound->second->play();
		}
	}

	Sound* SoundManager::getSound( std::string name )
	{
		SoundMapIterator sound = mSounds.find(name);

		if (sound != mSounds.end())
		{
			return sound->second;
		}

		return NULL;
	}

	void SoundManager::setListenerPosition( Ogre::Vector3& pos, Ogre::Vector3& vel, Ogre::Vector3& fdir, Ogre::Vector3& updir )
	{
		pos.z = -pos.z;
		vel.z = -vel.z;
		fdir.z = -fdir.z;
		updir.z = -updir.z;

		FSOUND_3D_Listener_SetAttributes( &pos.x, &vel.x, fdir.x, fdir.y, fdir.z, updir.x, updir.y, updir.z );
	}

	void SoundManager::setListenerPosition( Ogre::Camera* cam, Ogre::Vector3& vel )
	{
		Ogre::Vector3 pos, fdir, updir;
		Ogre::Quaternion orient;

		pos = cam->getPosition();
		orient = cam->getOrientation();

		fdir = orient * Ogre::Vector3::UNIT_Z;
		updir = Ogre::Vector3::UNIT_Y;

		setListenerPosition( pos, vel, fdir, updir );
	}

	void SoundManager::encodeSound( std::string infile, std::string outfile )
	{
	
		ifstream in(infile.c_str(), ios::binary);
		ofstream out(outfile.c_str(), ios::binary);

		if (!in.is_open())
			return;

		if (!out.is_open())
			return;

		char buffer;
		std::vector<char> bigbuffer;
		

		while( in.read( &buffer, 1 ) )
		{
			bigbuffer.push_back(buffer);
		}

		// now "encode" the buffer :)
		for (int i=bigbuffer.size()-1; i>=0; i--)
			out.write( &bigbuffer[i], 1 );

		
		
	}

	void SoundManager::setMasterSFXVolume( int vol )
	{
		if (vol < 0) { vol = 0; }
		if (vol > 255) { vol = 255; }

		FSOUND_SetSFXMasterVolume( vol );
	}

		



}	// end NAMESPACE StuntPlayground