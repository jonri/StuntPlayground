/*
	Stunt Playground
	
	FollowCamera
*/

#include "FollowCamera.h"
#include "SoundManager.h"

namespace StuntPlayground
{


	void FollowCamera::update( Ogre::Real deltat )
	{
		// first, get the ideal goal position.
		Ogre::Vector3 mypos;
		Ogre::Vector3 goalpos;
		Ogre::Vector3 togoal;

		// if no goal node is assigned, we can`t move.
		if (!mGoalNode)
			return;

		mypos = mCamera->getPosition();

		goalpos = mGoalNode->getWorldPosition();

		//first get the Y offset and the XZ plane offset from the pitch angle.
		Ogre::Real x, y, z, xz;
		y = sin( mPitch.valueRadians() ) * mDist;
		xz = cos( mPitch.valueRadians() ) * mDist;

		// now get the x any z values from the Yaw angle.
		x = sin( mYaw.valueRadians() ) * xz;
		z = cos( mYaw.valueRadians() ) * xz;
		

		if (mJustFollow)
		{
			// in this case, don't worry about the orientation of the GoalNode.
			goalpos += Ogre::Vector3(x,y,z);
		}
		else
		{
			//take the orientation of the GoalNode into account!
			Ogre::Quaternion orient = mGoalNode->getWorldOrientation();
			Ogre::Matrix3 mat;
			orient.ToRotationMatrix( mat );
			Ogre::Vector3 xdir = mat.GetColumn(0);

			xdir.y = 0.0;
			xdir.normalise();

			mat.FromEulerAnglesXYZ(Ogre::Radian(0), Ogre::Math::ATan2(-xdir.z, xdir.x), Ogre::Radian(0) );

			orient.FromRotationMatrix( mat );
			goalpos += (orient * Ogre::Vector3(x,y,z));
		}

		
		// move toward the goal position!
		if (deltat > mMaxDelta) { deltat = mMaxDelta; }

		togoal = (goalpos - mypos).normalisedCopy() * (goalpos - mypos).length() * 10 * deltat;
		if (togoal.squaredLength() > (goalpos - mypos).squaredLength())
			togoal = (goalpos - mypos);

		mypos += togoal;

		if (mypos.y < -3.5 ) { mypos.y = -3.5; }

		mCamera->setPosition( mypos );

		// look
		Ogre::Vector3 goallook;

		if (mLookNode)
			goallook = mLookNode->getWorldPosition();
		else
			goallook = mGoalNode->getWorldPosition();

		Ogre::Vector3 tolook = (goallook - mLookVec) * (10*deltat);

		mLookVec += tolook;

		mCamera->lookAt( mLookVec );

		/////////////////
		// SOUND LISTENER SETTING
		//SoundManager::getSingleton().setListenerPosition( mCamera, togoal * deltat );

	}
		


}	// end NAMESPACE StuntPlayground