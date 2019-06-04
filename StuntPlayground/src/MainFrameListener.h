/*
	Stunt Playground
	
	MainFrameListener
*/
#ifndef _MAINFRAMELISTENER_
#define _MAINFRAMELISTENER_


#include "WalaberOgreFrameListener.h"
#include ".\StuntPlayground.h"
#include ".\FollowCamera.h"

namespace StuntPlayground
{

class MainFrameListener :
	public WalaberOgreFrameListener
{
protected:

	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;
	FollowCamera* mFollowCam;
	OgreNewt::World* mWorld;

	bool mLMB;
	bool mRMB;
	bool mKeySU, mKeySD;
	bool mESCAPE;
	bool mLEFT, mRIGHT;
	bool mC;
	bool mSPACE;
	bool mREC;
	bool mR;

	Ogre::Timer mMouseStill;

	Ogre::Degree mRotDegree;
	std::vector<Ogre::Vector3> mBasicAxis;
	StuntPlayground::RotAxis mCurrentAxis;

	Ogre::Real mPlaybackTime;

	Ogre::OverlayElement* mMessage;
	Ogre::String mMessageText;
	Ogre::Real mMessageWidth;

	// should we update the physics?
	bool mUpdateNewton;

	int desired_framerate;
	Ogre::Real m_update, m_elapsed;

	Ogre::Real mMinCamDist;

	void userControls( Ogre::Real timeelapsed );

	void updateMessage( Ogre::Real timestep );

#ifdef _SP_DEBUG_LOG
	Ogre::Real m_totalelapsed;
	unsigned int m_totalframes;
	unsigned int m_totalupdates;
#endif	// _SP_DEBUG_LOG

public:
	MainFrameListener(RenderWindow* win, Camera* cam, SceneManager* mgr, FollowCamera* follow, OgreNewt::World* world, int desired_fps );
	~MainFrameListener(void);

	bool frameStarted(const FrameEvent &evt);

	void startReplayPlayback( Ogre::Real time );

};


}	// end NAMESPACE StuntPlayground


#endif	// _MAINFRAMELISTENER_ guard