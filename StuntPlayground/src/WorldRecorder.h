/*
	Stunt Playground
	
	WorldRecorder
*/

#ifndef _STUNTPLAYGROUND_WORLDRECORDER_
#define _STUNTPLAYGROUND_WORLDRECORDER_

#include <Ogre.h>
#include <OgreNewt.h>
#include <OgreNewt_Body.h>

#include <tinyxml.h>

namespace StuntPlayground
{

	/*
		Stunt Playground

		WorldREcorder class

		Singleton class for recording the movements of bodies in the world, for playback later.
	*/
	class WorldRecorder
	{
		// basic keyframe class, holds all information for a single time/frame.
		class Keyframe
		{
		public:
			Keyframe() : mTime(0.0), mPos(0,0,0), mOrient(Ogre::Quaternion::IDENTITY) {}
			~Keyframe() {}

			//tinyXML export function
			TiXmlElement getXMLElement();

			//tinyXML import function
			void fromXMLElement( const TiXmlElement* elem );

			// membe variables
			Ogre::Real mTime;
			Ogre::Vector3 mPos;
			Ogre::Quaternion mOrient;	
		};

	public:
		// basic class for redordable objects
		class RecordableObject
		{
			Ogre::SceneNode* mNode;

			Ogre::Real mLastTime;
			Ogre::Real mMinWaitTime;

			unsigned int mLastKeyframe;
			std::vector<Keyframe> mFrames;
		
		public:
			// these should not be called by the user, use the "add" function in the WorldRecorder
			RecordableObject() : mNode(NULL), mLastTime(0.0), mMinWaitTime(0) {}
			~RecordableObject() {}

			void init( Ogre::SceneNode* node, Ogre::Real minwait );

			// main function used to add a keyframe to the object.
			void addKeyframe();

			// update the position, given a time value.
			void update( Ogre::Real time );

			// remove all keyframes!
			void removeAllKeyframes();

			Ogre::SceneNode* getNode() { return mNode; }

			// tinyXML export function.
			TiXmlElement getXMLElement();

			// tinyXML import function.
			void fromXMLElement( const TiXmlElement* node );
		};


	private:
		WorldRecorder();
		~WorldRecorder();

		Ogre::Real mMaxRecTime;

		Ogre::Real mTimer;
		Ogre::Real mTotalElapsed;

		Ogre::Real mTempMinWait;

		std::list<RecordableObject> mObjects;

		bool mRecording;
		bool mPlaying;

		bool mRecorded;


	public:

		static WorldRecorder& getSingleton();

		void Init( Ogre::Real maxrectime ) { mMaxRecTime = maxrectime; }

		RecordableObject* addRecordableObject( Ogre::SceneNode* node, Ogre::Real minwait );

		void removeRecordableObject( RecordableObject* obj );

		void removeAllRecordableObjects();
		void removeAllKeyframes();

		void incTime( Ogre::Real deltat ) { mTimer += deltat; }

		void startRecording();
		void stopRecording();

		void startPlaying();
		void stopPlaying();

		void setTime( Ogre::Real time );

		Ogre::Real getElapsed();

		bool recording() { return mRecording; }
		bool playing() { return mPlaying; }
		bool recorded() { return mRecorded; }

		Ogre::Real getMaxRecTime() { return mMaxRecTime; }

		void collectProps( Ogre::Real minwait );

		void saveToXML( std::string filename );
		void bumpTotalElapsed( Ogre::Real time ) { if (time > mTotalElapsed) { mTotalElapsed = time; } }
		void recordedFromXML() { mRecorded = true; }

		static void collectPropsCallback( OgreNewt::Body* body );

	};

}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_WORLDRECORDER_