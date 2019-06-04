/*
	Stunt Playground
	
	Vehicle
*/

#ifndef _STUNTPLAYGROUND_VEHICLE
#define _STUNTPLAYGROUND_VEHICLE

#include <OgreNewt.h>
#include <Ogre.h>
#include "StuntPlayground.h"
#include "WorldRecorder.h"
#include "SoundManager.h"

namespace StuntPlayground
{




	/* Car class!  this is the basis for the entire vehicle system for Stunt Playground. */
	class Car : public OgreNewt::Vehicle
	{
	public:
		enum VehicleState
		{
			VS_4ONFLOOR, VS_2WHEELS, VS_AIRBORNE, VS_UPSIDEDOWN
		};

		enum TireState
		{
			TS_ONROAD, TS_ONDIRT, TS_ONPROP
		};

		class MyTire : public OgreNewt::Vehicle::Tire
		{
		public:
			MyTire( Ogre::SceneManager* mgr, Ogre::SceneNode* parentnode, OgreNewt::Vehicle* vehicle, Ogre::Quaternion localorient, Ogre::Vector3 localpos, Ogre::Vector3 pin,
				Ogre::Real mass, Ogre::Real width, Ogre::Real radius, Ogre::Real susShock, Ogre::Real susSpring, Ogre::Real susLength, int colID = 0);

			~MyTire();

			void setSteer( int steer ) { mSteer = steer; }
			int getSteer() { return mSteer; }

			void setDrive( int drive ) { mDrive = drive; }
			int getDrive() { return mDrive; }

			void setGrip( Ogre::Real grip ) { mGrip = grip; }
			Ogre::Real getGrip() { return mGrip; }

			Ogre::Real getRadius() { return mRadius; }

			WorldRecorder::RecordableObject* mRecObj;

		private:

			Ogre::SceneManager* mSceneMgr;
			int mSteer;
			int mDrive;
			Ogre::Real mGrip;
			Ogre::Real mRadius;
		};

		struct TorquePoint
		{
			Ogre::Real rpm;
			Ogre::Real torque;
		};


		Car();
		Car( OgreNewt::World* world, Ogre::SceneNode* parentnode, Ogre::SceneManager* mgr, Ogre::String filename, const OgreNewt::MaterialID* mat );
		~Car();
		// my init function.
		void _init( OgreNewt::World* world, Ogre::SceneNode* parentnode, Ogre::SceneManager* mgr, Ogre::String filename, const OgreNewt::MaterialID* mat );

		void setup();

		void userCallback();

		void setThrottleSteering( Ogre::Real throttle, Ogre::Real steering ) { mThrottle = throttle; mSteering = steering; }
		void shiftUp() { mCurrentGear++; mJustShifted = true;  if (mCurrentGear > mGears.size()-1) { mCurrentGear = mGears.size()-1; mJustShifted = false; } }
		void shiftDown() { mCurrentGear--; mJustShifted = true;  if (mCurrentGear <= 0) { if (mLastOmega <= 5.0) { mCurrentGear = 0; mJustShifted = false;  } else { mCurrentGear = 1; } } }
		void setBrakes( bool onoff ) { mBrakesOn = onoff; }

		Ogre::SceneNode* getLookNode() { return mLookNode; }

		static void Transform( OgreNewt::Body* me, const Ogre::Quaternion& orient, const Ogre::Vector3& pos );

		void setupRecording( Ogre::Real minwait );

		WorldRecorder::RecordableObject* mRecObject;

		VehicleState getState() { return mState; }

		void incCollisions();

		void setTireState( TireState state ) { mTireState = state; }
		TireState getTireState() { return mTireState; }

		Ogre::String getFilename() { return mFilename; }

		void setManualTransmission( bool setting ) { mManualTransmission = setting; }
		bool getManualTransmission() { return mManualTransmission; }

		void playEngine();
		void stopEngine();


	private:
		Ogre::SceneManager* mSceneMgr;

		Ogre::String mFilename;
		Ogre::String mName;

		std::vector<Ogre::Real> mGears;
		int mCurrentGear;

		Ogre::Real mShiftDelay;
		Ogre::Real mShiftTime;

		std::vector<TorquePoint> mTorqueCurve;

		Ogre::Real mBrakeStrength;
		Ogre::Real mDifferentialRatio;
		Ogre::Real mTransmissionEfficiency;
		Ogre::Real mDragCoefficient;
		Ogre::Real mEngineMass;

		StuntPlayground::Sound* mEngineSnd;
		StuntPlayground::Sound* mScreechSnd;

		Ogre::Real mMinRPM;
		Ogre::Real mMaxRPM;
		Ogre::Real mRPMRange;
		Ogre::Real mEngineBaseRPM;
		Ogre::Real mEngineScale;

		Ogre::Radian mSteerMaxAngle;
		Ogre::Radian mSteerSpeed;
		Ogre::Real mSteerLossPercent;
		Ogre::Real mSteerFromSpeed;
		Ogre::Real mSteerToSpeed;
		Ogre::Real mSteerSpeedRange;

		Ogre::Real mThrottle;
		Ogre::Real mSteering;

		Ogre::Radian mSteerAngle;

		bool mBrakesOn;

		Ogre::Real mLastOmega;

		Ogre::SceneNode* mLookNode;

		VehicleState mState;
		VehicleState mOldState;

		// Ogre::Timer mTimer;
		Ogre::Real mTimer;
		Ogre::Real mChangeTimer;
		bool mCheckingChange;
		Ogre::Real mShiftTimer;
		Ogre::Real mJustShiftedTimer;


		bool mJustShifted;
		

		Ogre::Vector3 mTakeOffPos;
		int mNumFlips;
		Ogre::Vector3 mLastY;
		unsigned int mCollisions;

		TireState mTireState;

		bool mManualTransmission;


	};



}	// end NAMESPACE StuntPlayground



#endif	// _STUNTPLAYGROUND_VEHICLE