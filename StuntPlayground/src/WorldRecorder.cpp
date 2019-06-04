/*
	Stunt Playground
	
	Worldrecorder
*/

#include "StuntPlaygroundApplication.h"
#include "WorldRecorder.h"

#include <tinyxml.h>

namespace StuntPlayground
{

	/////////////////////////////////////////////////////////////////////////////////
	TiXmlElement WorldRecorder::Keyframe::getXMLElement()
	{
		TiXmlElement elem("keyframe");
		elem.SetAttribute( "time", Ogre::StringConverter::toString(mTime) );
		elem.SetAttribute( "position", Ogre::StringConverter::toString(mPos) );
		elem.SetAttribute( "orient", Ogre::StringConverter::toString(mOrient) );

		return elem;
	}

	void WorldRecorder::Keyframe::fromXMLElement( const TiXmlElement* elem )
	{
		mTime = Ogre::StringConverter::parseReal( elem->Attribute("time") );
		mPos = Ogre::StringConverter::parseVector3( elem->Attribute("position") );
		mOrient = Ogre::StringConverter::parseQuaternion( elem->Attribute("orient") );
	}


	/////////////////////////////////////////////////////////////////////////////////
	void WorldRecorder::RecordableObject::init( Ogre::SceneNode* node, Ogre::Real minwait )
	{
		 mNode = node;
		 mMinWaitTime = minwait;
		 mLastKeyframe = 0;

		 // reserve enough memory to record!
		 mFrames.reserve( static_cast<int>(WorldRecorder::getSingleton().getMaxRecTime() * (10/mMinWaitTime)) );
	}


	void WorldRecorder::RecordableObject::addKeyframe()
	{

		// are we currently recording?
		if (!WorldRecorder::getSingleton().recording())
		{
			return;
		}

		// get the elapsed time from the singleton object.
		Ogre::Real time = WorldRecorder::getSingleton().getElapsed();

		if (!mFrames.empty())
		{

			Ogre::Real delta = time - mFrames.back().mTime;
			if (delta < mMinWaitTime)
			{
				return;
			}
				
		}

		if (time > WorldRecorder::getSingleton().getMaxRecTime())
		{
			// we've exceeded the maximum rec time, so stop recording!
			WorldRecorder::getSingleton().stopRecording();
			return;
		}

		// add the keyframe!
		WorldRecorder::Keyframe frame;
		frame.mTime = time;
		frame.mPos = mNode->getWorldPosition();
		frame.mOrient = mNode->getWorldOrientation();

		mFrames.push_back( frame );

	}

	void WorldRecorder::RecordableObject::removeAllKeyframes()
	{
		while (!mFrames.empty())
			mFrames.pop_back();

		mLastKeyframe = 0;
		mLastTime = 0.0;
	}

	void WorldRecorder::RecordableObject::update( Ogre::Real time )
	{
		unsigned int before = 0;
		unsigned int after = 0;

		bool ready = false;

		// clamp the time!
		if (time <= 0.0) { time = 0.0; before = 0; after = 0; ready = true; }
		if (time >= mFrames.back().mTime) { time = mFrames.back().mTime; after = mFrames.size()-1; before = after; ready = true; }

		if (!ready)
		{
			// find the first keyframe with a time lower than this time. start with the last keyframe...
			if (mFrames[mLastKeyframe].mTime < time)
			{
				// current time is after the last frame, do a quick loop to find first frame after the current time.
				after = mLastKeyframe;
				while (mFrames[after].mTime < time)
					after++;

				// now after should be the first frame after time. update before!
				before = after - 1;
				ready = true;
			}
			else
			{
				// in this case, current time is BEFORE the last frame.
				before = mLastKeyframe;
				while (mFrames[before].mTime > time)
					before--;

				//now before is first frame before time. update after.
				after = before + 1;
				ready = true;
			}
		}

		//okay, we have found the frames before and after the current time.
		Ogre::Real denom = (mFrames[after].mTime - mFrames[before].mTime);
		Ogre::Real deltat;

		if (denom > 0.0)
			deltat = (time - mFrames[before].mTime) / (mFrames[after].mTime - mFrames[before].mTime);
		else
			deltat = 0.0;

		Ogre::Vector3 pos = mFrames[before].mPos + ((mFrames[after].mPos - mFrames[before].mPos) * deltat);
		Ogre::Quaternion orient = mFrames[before].mOrient.Slerp(deltat, mFrames[before].mOrient, mFrames[after].mOrient, true );

		// position the node!
		mNode->setPosition( pos );
		mNode->setOrientation( orient );

		mLastKeyframe = before;
	}

	TiXmlElement WorldRecorder::RecordableObject::getXMLElement()
	{
		TiXmlElement me("recordable_object");

		for (std::vector<WorldRecorder::Keyframe>::iterator it = mFrames.begin(); it != mFrames.end(); it++)
		{
			TiXmlElement key = it->getXMLElement();
			me.InsertEndChild( key );
		}

		return me;
	}

	void WorldRecorder::RecordableObject::fromXMLElement( const TiXmlElement* node )
	{
		// first, clear any existing keyframes.
		mFrames.clear();

#ifdef _SP_DEBUG_LOG
		Application::getSingleton().mDebugLog->logMessage(" -R- RecordableObject::fromXMLElement: adding keyframes...");
#endif	// _SP_DEBUG_LOG

		// now loop through all frames in the XML element.
		for (TiXmlElement* key = ((TiXmlElement*)node)->FirstChildElement("keyframe"); key; key = key->NextSiblingElement("keyframe"))
		{
			Keyframe kf;
			kf.fromXMLElement( key );

			mFrames.push_back( kf );
			WorldRecorder::getSingleton().bumpTotalElapsed( kf.mTime );

#ifdef _SP_DEBUG_LOG
			Application::getSingleton().mDebugLog->logMessage(" -R- RecordableObject::fromXMLElement: added keyframe: time:"+
				Ogre::StringConverter::toString(kf.mTime) );
#endif	// _SP_DEBUG_LOG
		}

		WorldRecorder::getSingleton().recordedFromXML();
	}


	/////////////////////////////////////////////////////////////////////////////////
	WorldRecorder::WorldRecorder() : mTotalElapsed(0.0), mMaxRecTime(0.0), mTempMinWait(0.0)
	{
		mRecording = false;
		mPlaying = false;
		mRecorded = false;
	}

	WorldRecorder::~WorldRecorder()
	{

	}

	WorldRecorder& WorldRecorder::getSingleton()
	{
		static WorldRecorder instance;
		return instance;
	}

	WorldRecorder::RecordableObject* WorldRecorder::addRecordableObject( Ogre::SceneNode* node, Ogre::Real minwait )
	{
		// make sure this node hasn't already been added!
		std::list<RecordableObject>::iterator it;

		for (it = mObjects.begin(); it != mObjects.end(); ++it)
		{
			if (it->getNode() == node)
			{
				// error!
#ifdef _SP_DEBUG_LOG
				Application::getSingleton().mDebugLog->logMessage(" -R- addRecordableObject: Node already exists!");
#endif	// _SP_DEBUG_LOG
				return NULL;
			}
		}

		//okay, make a new object.
		RecordableObject obj;
		obj.init( node, minwait );

		mObjects.push_back( obj );

		return &mObjects.back();
	}

	Ogre::Real WorldRecorder::getElapsed()
	{
		if (mRecording)
		{
			// update our timer.
			/*unsigned long elap = mTimer.getMilliseconds();
			
			Ogre::Real delta = (Ogre::Real(elap))/(1000.0);
			*/
			// in seconds.
			mTotalElapsed = mTimer; //delta;
		}

		return mTotalElapsed;
	}

	void WorldRecorder::removeAllRecordableObjects()
	{
		// just pop off everything in the list!
		if (mRecording)
		{
			// error!
			return;
		}

		if (mPlaying)
		{
			// error!
			return;
		}

		while (!mObjects.empty())
			mObjects.pop_back();

		// reset!
		mRecorded = false;
		mRecording = false;
		mPlaying = false;

	}

	void WorldRecorder::removeAllKeyframes()
	{
		if (mRecording)
		{
			// error!
			return;
		}

		if (mPlaying)
		{
			// error!
			return;
		}

		std::list<RecordableObject>::iterator obj;

		for (obj = mObjects.begin() ; obj!=mObjects.end(); ++obj)
		{
			obj->removeAllKeyframes();
		}

		mRecorded = false;
		mRecording = false;
		mPlaying = false;
	}

	void WorldRecorder::startRecording()
	{
		Application::getSingleton().showGUI_REC();

		if (mPlaying)
		{
			// error!
			return;
		}

		if (mRecorded)
		{
			// error!
			return;
		}

		mRecording = true;
		// reset the timer.
		mTimer = 0.0; //.reset();

		mTotalElapsed = 0.0;

		std::list<RecordableObject>::iterator obj;

		// set an initial keyframe for all objects.
		for (obj = mObjects.begin(); obj != mObjects.end(); ++obj)
		{
			obj->addKeyframe();
		}

		mRecorded = true;
	}


	void WorldRecorder::stopRecording()
	{
		Application::getSingleton().hideGUI_REC();

		if (mPlaying)
		{
			// error!
			return;
		}

		if (!mRecording)
		{
			// error!
			return;
		}

		mRecording = false;

		// set a final keyframe for all objects.
		std::list<RecordableObject>::iterator obj;

		for (obj = mObjects.begin(); obj != mObjects.end(); ++obj)
		{
			obj->addKeyframe();
		}
	}

	void WorldRecorder::startPlaying()
	{
		if (mRecording)
		{
			// error!
			return;
		}

		if (mPlaying)
		{
			// error!
			return;
		}

		if (!mRecorded)
		{
			// error!
			return;
		}

		mPlaying = true;

		//set time to 0.
		setTime( 0.0 );
	}


	void WorldRecorder::stopPlaying()
	{
		if (mRecording)
		{
			// error!
			return;
		}

		if (!mPlaying)
		{
			// error!
			return;
		}

		// reset to last keyframe.
		//setTime( mTotalElapsed );

		mPlaying = false;

	}

	void WorldRecorder::setTime( Ogre::Real time )
	{
		// loop through and update all objects based in this time.
		if (time > mTotalElapsed) { time = mTotalElapsed; }
		if (time < 0 ) { time = 0.0; }

		std::list<RecordableObject>::iterator obj;

		for (obj = mObjects.begin(); obj != mObjects.end(); ++obj)
			obj->update( time );
	}

	void WorldRecorder::collectProps( Ogre::Real minwait )
	{
		mTempMinWait = minwait;

		OgreNewt::BodyIterator::getSingleton().go( WorldRecorder::collectPropsCallback );

		mTempMinWait = 0.0;
	}


	void WorldRecorder::collectPropsCallback( OgreNewt::Body* body )
	{
		if (body->getType() == StuntPlayground::BT_PROP)
		{
			Prop* prp = (Prop*)body->getUserData();
			prp->setupRecording( WorldRecorder::getSingleton().mTempMinWait );
		}
	}


	void WorldRecorder::saveToXML( std::string filename )
	{
		TiXmlDocument doc;

		TiXmlElement root("WorldRecording");

		// loop through all recordable objects!
		for (std::list<RecordableObject>::iterator it = mObjects.begin(); it != mObjects.end(); it++)
		{
			TiXmlElement obj = it->getXMLElement();

			root.InsertEndChild( obj );
		}

		doc.InsertEndChild( root );

		doc.SaveFile( filename );
	}
			


}	// end NAMESPACE StuntPlayground