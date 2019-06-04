/*
	Stunt Playground
	
	FollowCamera
*/

#ifndef _STUNTPLAYGROUND_FOLLOWCAMERA_
#define _STUNTPLAYGROUND_FOLLOWCAMERA_

#include <Ogre.h>

namespace StuntPlayground
{

	/* my custom follow camera class.  this class lets you "assign" a node to the camera, and it will
	   follow it.  it also has 2 follow modes.  "JustFollow" simply follows a moving node, trying to
	   maintain a certain distance.  if justfollow is set to false, the camera maintains a certain angle,
	   as defined by the Yaw and Pitch angles. */
	class FollowCamera
	{
	private:
		// the camera!
		Ogre::Camera* mCamera;

		// the node we want to follow.
		const Ogre::Node* mGoalNode;
		// the (optional) node we want to look at
		const Ogre::Node* mLookNode;

		// goal distance we want to maintain from the node.
		Ogre::Real mDist;
		Ogre::Real mJustFollowHeight;

		// optional angles.
		Ogre::Radian mYaw;
		Ogre::Radian mPitch;

		// whether or not to use angles
		bool mJustFollow;

		// look position!
		Ogre::Vector3 mLookVec;

		// maximum delta to maintain stability.
		Ogre::Real mMaxDelta;

	public:
		FollowCamera( Ogre::Camera* cam ) : mDist(0), mYaw(0), mPitch(0), mJustFollowHeight(0), mMaxDelta(0.1f), mJustFollow(true) { mCamera = cam;  mGoalNode = NULL; mLookNode = NULL; mLookVec = Ogre::Vector3::ZERO; }
		~FollowCamera() { }

		// set the max delta time.
		void setMaxDelta( Ogre::Real val ) { mMaxDelta = val; }

		// main update command, this should be called each frame...
		void update( Ogre::Real deltat );

		// set the goal node.
		void setGoalNode( const Ogre::Node* node ) { mGoalNode = node; }

		// set the goal distance.
		void setGoalDistance( Ogre::Real dist ) { mDist = dist; }

		// set the goal Yaw angle
		void setGoalYawAngle( Ogre::Radian angle ) { mYaw = angle; }

		// set the goal Pitch angle
		void setGoalPitchAngle( Ogre::Radian angle ) { mPitch = angle; }

		// just follow?  false = respect angles as well.  true = just follow.
		void setJustFollow( bool setting ) { mJustFollow = setting; }

		// get goal node.
		const Ogre::Node* getGoalNode() { return mGoalNode; }

		// get goal distance.
		Ogre::Real getGoalDistance() { return mDist; }

		// get goal Yaw angle
		Ogre::Radian getGoalYawAngle() { return mYaw; }

		// get goal Pitch angle
		Ogre::Radian getGoalPitchAngle() { return mPitch; }

		// get follow mode
		bool getJustFollow() { return mJustFollow; }

		// set the (optional) look node.
		void setLookNode( const Ogre::Node* node ) { mLookNode = node; }

		// remove the (optional look node)
		void removeLookNode() { mLookNode = NULL; }

		// get look node.
		const Ogre::Node* getLookNode() { return mLookNode; }

		// set a Y-offset for just follow mode.
		void setJustFollowHeight( Ogre::Real offset ) { mJustFollowHeight = offset; }

	};






}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_FOLLOWCAMERA_
