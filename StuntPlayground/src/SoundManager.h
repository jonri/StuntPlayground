/*
	Stunt Playground
	
	SoundManager
*/

#ifndef _STUNTPLAYGROUND_SOUNDMANAGER_
#define _STUNTPLAYGROUND_SOUNDMANAGER_

#include <Ogre.h>
#include <fmod.h>

namespace StuntPlayground
{
	// basic sound class, represents a short sound clip.
	class Sound
	{
	public:
		enum PlayState
		{
			PS_STOPPED, PS_PLAYING, PS_PAUSED
		};

		Sound( std::string name, std::string filename );
		~Sound();

		void play();
		void play( int vol );
		void play( int vol,const Ogre::Vector3& pos,const Ogre::Vector3& vel = Ogre::Vector3::ZERO );

		void stop();
		void pause();

		bool isPlaying();

		void setPositionVelocity( const Ogre::Vector3& pos, const Ogre::Vector3& vel = Ogre::Vector3::ZERO );
		void getPositionVelocity( Ogre::Vector3& pos, Ogre::Vector3& vel );

		void setVolume( int vol );
		int getVolume();

		void setFrequency( int freq );
		int getFrequency();

		void setLoop( bool onoff );


		PlayState getState() { return mState; }

		std::string& getName() { return mName; }

		Ogre::Real getTimeSinceLastPlay() { return (mTimer.getMilliseconds()/1000.0); }

	private:
		int mChannel;
		PlayState mState;

		std::string mName;

		FSOUND_SAMPLE* mPtr;

		Ogre::Timer mTimer;
	};

	// SoundManager singleton class!
	class SoundManager
	{
	public:
		enum MixRate
		{
			MR_22050 = 22050, MR_24000 = 24000, MR_44100 = 44100, MR_48000 = 48000
		};

		static SoundManager& getSingleton();

		void Init( MixRate rate, int numSoftChannels );

		Sound* addSound( std::string name, std::string filename );
		void removeSound( std::string name );

		void playSound( std::string name );

		Sound* getSound( std::string name );

		void encodeSound( std::string infile, std::string outfile );

		void setListenerPosition( Ogre::Vector3& pos, Ogre::Vector3& vel, Ogre::Vector3& fdir, Ogre::Vector3& updir );
		void setListenerPosition( Ogre::Camera* cam, Ogre::Vector3& vel );

		void update3DSound() { FSOUND_Update(); }
		void setMasterSFXVolume( int vol );

	private:
		typedef std::map<std::string, Sound*> SoundMap;
		typedef std::map<std::string, Sound*>::iterator SoundMapIterator;
		typedef std::pair<std::string, Sound*> SoundPair;

		SoundManager();
		~SoundManager();

		SoundMap mSounds;

	};





}	// end NAMESPACE StuntPlayground


#endif	// _STUNTPLAYGROUND_SOUNDMANAGER_