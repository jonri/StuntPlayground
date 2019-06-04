/*
	Stunt Playground
	
	MainFrameListener
*/
#include ".\StuntPlaygroundApplication.h"
#include "OgreNewt.h"
#include "OgreNewt_Debugger.h"
#include ".\mainframelistener.h"
#include ".\FollowCamera.h"
#include ".\WorldRecorder.h"
#include ".\SoundManager.h"
#include ".\PropManager.h"

namespace StuntPlayground
{


	MainFrameListener::MainFrameListener(RenderWindow* win, Camera* cam, SceneManager* mgr, FollowCamera* follow, OgreNewt::World* world, int desired_fps ) :
		WalaberOgreFrameListener(win)
{
	mCamera = cam;
	mSceneMgr = mgr;
	mFollowCam = follow;
	mWorld = world;

	desired_framerate = desired_fps;

	mRotDegree = 0;

	m_update = (Ogre::Real)(1.0f / (Ogre::Real)desired_framerate);
	m_elapsed = 0.0f;

	mLMB = mRMB = mKeySU = mKeySD = mESCAPE = mLEFT = mRIGHT = mC = mSPACE = mREC = false;
	mUpdateNewton = true;

	mMessage = Ogre::OverlayManager::getSingleton().getOverlayElement("MESSAGE2");

	
// #ifdef _SP_DEBUG
	// general OgreNewt debugger.
	OgreNewt::Debugger::getSingleton().init( mgr );
#ifdef _SP_DEBUG
	m_totalelapsed = 0.0;
	m_totalframes = m_totalupdates = 0;
#endif	// _SP_DEBUG

	// basic axis'
	Ogre::Vector3 temp;
	temp = Ogre::Vector3::UNIT_X;
	mBasicAxis.push_back( temp );

	temp = Ogre::Vector3::UNIT_Y;
	mBasicAxis.push_back( temp );

	temp = Ogre::Vector3::UNIT_Z;
	mBasicAxis.push_back( temp );

	mCurrentAxis = StuntPlayground::AXIS_Y;

	mMinCamDist = 1.5;




}

MainFrameListener::~MainFrameListener(void)
{

#ifdef _SP_DEBUG_LOG
	Ogre::Log* DebugLog = Application::getSingleton().mDebugLog;
	// debug averages, etc.
	DebugLog->logMessage("=============================================================");
	DebugLog->logMessage("				OVERALL AVERAGES");
	DebugLog->logMessage("=============================================================");
	DebugLog->logMessage(" TOTAL FRAMES:  "+Ogre::StringConverter::toString(m_totalframes) );
	DebugLog->logMessage(" TOTAL ELAPSED: "+Ogre::StringConverter::toString(m_totalelapsed) );
	DebugLog->logMessage(" TOTAL UPDATES: "+Ogre::StringConverter::toString(m_totalupdates) );
	DebugLog->logMessage(" ====================================---------===============");
	DebugLog->logMessage("  AVERAGE ELAPSED: "+Ogre::StringConverter::toString( (m_totalelapsed/m_totalframes) ) );
	DebugLog->logMessage("  AVERAGE FPS:     "+Ogre::StringConverter::toString( (m_totalframes/m_totalelapsed) ) );
	DebugLog->logMessage("  AVERAGE UPDATES: "+Ogre::StringConverter::toString( (m_totalupdates/m_totalframes) ) );
	DebugLog->logMessage("=============================================================");
#endif	// _SP_DEBUG_LOG
	
}

bool MainFrameListener::frameStarted(const FrameEvent &evt)
{

	// if way too much time has elapsed, skip one frame for stability of deltatime-based elements.
	if (evt.timeSinceLastFrame > 0.8f) { return true; }

	// Application singleton!
	Application& app = Application::getSingleton();

	// get the dummy body.
	DummyBody* dummy = app.getDummy();

	// update newton world.
	m_elapsed += evt.timeSinceLastFrame;

#ifdef _SP_DEBUG_LOG
	Ogre::Real elap = m_elapsed;

	m_totalelapsed += evt.timeSinceLastFrame;
#endif	// _SP_DEBUG_LOG

	int count = 0;

	if ((m_elapsed > m_update) && (m_elapsed < (0.1f)) )
	{
		// clear out the dummy object.
		if (dummy)
			dummy->setPossibleAttach( NULL );

		while (m_elapsed > m_update)
		{
			if (mUpdateNewton)
			{
				mWorld->update( m_update );
				WorldRecorder::getSingleton().incTime( m_update );
			}
			userControls( m_update );			
			m_elapsed -= m_update;
			count++;
		}
	}
	else
	{
		if (m_elapsed < (m_update))
		{
			// not enough time has passed this loop, so ignore for now.
		}
		else
		{
			// clear out the dummy object.
			if (dummy)
				dummy->setPossibleAttach( NULL );

			// in this case, too much time has passed to be properly handled, so perform 1 update, and clear the elapsed time.
			if (mUpdateNewton)
			{
				mWorld->update( m_update );
				WorldRecorder::getSingleton().incTime( m_update );
			}
			userControls( m_update );
			m_elapsed = 0.0;
			count++;
		}
	}
	mInputDevice->capture();

#ifdef _SP_DEBUG_LOG
	app.mDebugLog->logMessage(" Elapsed: "+Ogre::StringConverter::toString(elap)+"		Updates: "+
		Ogre::StringConverter::toString( count ), Ogre::LML_TRIVIAL );

	m_totalupdates += count;
	m_totalframes ++;
#endif	// _SP_DEBUG_LOG

	/////////////
	// basic mouse data
	long mx = mInputDevice->getMouseRelativeX();
	long my = mInputDevice->getMouseRelativeY();

	if ((mx != 0) && (my != 0)) { mMouseStill.reset(); }

	if (mx > 20) { mx = 20; }
	if (mx < -20) { mx = -20; }
	if (my > 20) { my = 20; }
	if (my < -20) { my = -20; }



	//////////////////////////////////////////////////////////////
	// OPENING LOGO
	if (app.getProgState() == StuntPlayground::PS_LOGO)
	{

		if (mInputDevice->getMouseButton(0) && (mLMB == false))
		{
			mLMB = true;
			app.setProgState( StuntPlayground::PS_PLACING );
			app.hideLOGO();

			app.setDummyCallback();
		}

		if (!mInputDevice->getMouseButton(0)) { mLMB = false; }
	}

	//////////////////////////////////////////////////////////////
	//	CHOOSING CARS
	if (app.getProgState() == StuntPlayground::PS_CHOOSING)
	{
		if ((mInputDevice->isKeyDown(Ogre::KC_LEFT) || mInputDevice->isKeyDown(Ogre::KC_A)) && (!mLEFT))
		{
			mLEFT = true;
			app.mCar->stopEngine();
			app.prevCar();
			app.mCar->playEngine();
		}

		if ((mInputDevice->isKeyDown(Ogre::KC_RIGHT) || mInputDevice->isKeyDown(Ogre::KC_D)) && (!mRIGHT))
		{
			mRIGHT = true;
			app.mCar->stopEngine();
			app.nextCar();
			app.mCar->playEngine();
		}

		if ((mInputDevice->isKeyDown(Ogre::KC_RETURN)) || (mInputDevice->isKeyDown(Ogre::KC_W)) || (mInputDevice->isKeyDown(Ogre::KC_UP)))
		{
			app.showGUI();

			// setup the Instant replay recorder!
			WorldRecorder::getSingleton().removeAllRecordableObjects();
			app.mCar->setupRecording( 0.012 );
			WorldRecorder::getSingleton().collectProps( 0.03 );

			app.setProgState( StuntPlayground::PS_PLAYING );
		}

		if ((mInputDevice->isKeyDown(Ogre::KC_ESCAPE)) && (!mESCAPE))
		{
			mESCAPE = true;

			app.removeCar();

			app.restoreArena();

			app.setDummyVisible( true );

			app.setProgState( StuntPlayground::PS_PLACING );
			app.disableSaveReplay();

		}


		if (!mInputDevice->isKeyDown(Ogre::KC_LEFT) && !mInputDevice->isKeyDown(Ogre::KC_A)) { mLEFT = false; }
		if (!mInputDevice->isKeyDown(Ogre::KC_RIGHT) && !mInputDevice->isKeyDown(Ogre::KC_D)) { mRIGHT = false; }

		app.updateReflection();

	}

	//////////////////////////////////////////////////////////////
	//	OBJECT PLACEMENT INTERFACE
	if (app.getProgState() == StuntPlayground::PS_PLACING)
	{
		bool DropOK = true;
		
		if (dummy->getAttached())
		{
			Ogre::Matrix3 rot;
			Ogre::Quaternion or;
			Ogre::Vector3 p;

			Ogre::Real mass;
			Ogre::Vector3 inertia;

			dummy->getAttached()->getMassMatrix(mass,inertia);

			if (mass == 0.0)
			{
				dummy->getNewtonBody()->getPositionOrientation(p,or);
				or.ToRotationMatrix( rot );

				if ((rot.GetColumn(1).y <= 0.999) || (p.y > -2.0))
					DropOK = false;
			}

			// does the user want to remove this prop?
			if (mInputDevice->isKeyDown(Ogre::KC_BACK))
			{
				OgreNewt::Body* bod = dummy->getAttached();
				dummy->detach();

				if (bod->getType() == StuntPlayground::BT_PROP)
				{
					PropManager::getSingleton().removeProp( (Prop*)bod->getUserData() );
				}
			}
				
		}



		// make sure the CTRL key isn't pushed down!
		if ((!mInputDevice->isKeyDown(Ogre::KC_LCONTROL)) && (!mInputDevice->isKeyDown(Ogre::KC_RCONTROL)))
		{
			// moving the object around!
			Ogre::Vector3 curpos;
			Ogre::Quaternion curorient;
			Ogre::Real mass;
			Ogre::Vector3 inertia;
			
			if (dummy->getAttached())
			{
				//something attached, get it's mass
				dummy->getAttached()->getMassMatrix(mass, inertia);
			}
			else
			{
				//nothing attached, get the mass of the dummy itself
				dummy->getNewtonBody()->getMassMatrix(mass,inertia);
			}


			dummy->getGoalPositionOrientation( curpos, curorient );
			
			Ogre::Vector3 temppos;
			Ogre::Quaternion temporient;
			dummy->getNewtonBody()->getPositionOrientation( temppos, temporient );

			if (mMouseStill.getMilliseconds() > 300) { curpos.x = temppos.x; curpos.z = temppos.z; }

			if (!mInputDevice->getMouseButton(1))
			{
				if (mRMB)
				{
					//right mouse button was just released
					mRMB = false;
					curpos.y = temppos.y;
				}

				CEGUI::Renderer* rend = CEGUI::System::getSingleton().getRenderer();

				Ogre::Ray camray = mCamera->getCameraToViewportRay(rend->getWidth()/2, rend->getHeight()/2);
				Ogre::Vector3 forward, side;

				forward = camray.getDirection();
				forward.y = 0.0;
				forward.normalise();

				side = Ogre::Vector3( forward.z, 0, -forward.x );

				if ((!mInputDevice->isKeyDown(Ogre::KC_LSHIFT)) && (!mInputDevice->isKeyDown(Ogre::KC_RSHIFT)))
				{
					forward *= mx * 0.05;
					side *= my * -0.05;

					curpos += forward  + side;
					}

				// zero-mass check!
				if (mass == 0.0)
				{
					curpos.y = -3.5;
				}

				
			}
			else
			{
				mRMB = true;

				
				if (!mass == 0.0)
				{
					Ogre::Vector3 up(0,1,0);
					up *= my * -0.05;
	
					curpos += up;
				}
			}

			if (mInputDevice->isKeyDown(Ogre::KC_Y)) { mCurrentAxis = StuntPlayground::AXIS_Y; }
			if (mInputDevice->isKeyDown(Ogre::KC_X)) { mCurrentAxis = StuntPlayground::AXIS_X; }
			if (mInputDevice->isKeyDown(Ogre::KC_Z)) { mCurrentAxis = StuntPlayground::AXIS_Z; }
			if (mass == 0.0) { mCurrentAxis = StuntPlayground::AXIS_Y; }

			if (mInputDevice->isKeyDown(Ogre::KC_TAB))
			{
				mCurrentAxis = StuntPlayground::AXIS_Y;
				curorient = Ogre::Quaternion::IDENTITY;
			}

			dummy->setRotRingOrient( mCurrentAxis );
			mRotDegree = Ogre::Degree(0);

			// rotation.		
			if ((!mInputDevice->isKeyDown(Ogre::KC_LSHIFT)) && (!mInputDevice->isKeyDown(Ogre::KC_RSHIFT)))
				mRotDegree += Ogre::Degree(mInputDevice->getMouseRelativeZ()*0.03);
			else
				mRotDegree += Ogre::Degree(mx * 0.5);

			if (mRotDegree.valueDegrees() > 360.0)
				mRotDegree = Ogre::Degree(mRotDegree.valueDegrees() - 360.0);

			if (mRotDegree.valueDegrees() < -360.0)
				mRotDegree = Ogre::Degree(mRotDegree.valueDegrees() + 360.0);

			Ogre::Quaternion change;
			change.FromAngleAxis( mRotDegree, mBasicAxis[mCurrentAxis] );

			curorient = curorient * change;
			dummy->setGoalPositionOrientation( curpos, curorient );

			//user wants to drop the object?
			if (mInputDevice->getMouseButton(0) && mLMB == false)
			{
				mLMB = true;
				// object attached?
				if (dummy->getAttached())
				{
					//drop the current object!
					if (DropOK)
						dummy->detach();
				}
				else
				{
					//nothing attached...
	
					if (dummy->getPossibleAttach())
					{
						//object can be picked up, let's do so!
						dummy->attachToCurrentPossible( app.mMatPlacer );
						Application::getSingleton().showHint(Ogre::String(" move with mouse | click to drop | mouse wheel or shift to rotate | XYZ keys change axis | bksp to remove | TAB to reset rotation") );
					}
					else
					{
						// switch program modes
						app.setProgState( StuntPlayground::PS_WAITING );

					}
				}
			}
		
			if (!mInputDevice->getMouseButton(0))
			{
				mLMB = false;
			}

			////////////////////////////////////////////////////////////////////////////
			// AABB visualizer.
			if (dummy->getAttached())
			{
				Ogre::Real mass;
				Ogre::Vector3 inertia;

				OgreNewt::Body* bod = dummy->getAttached();

				bod->getMassMatrix( mass, inertia );

				// something can be selected... so let's highlight it!
				Ogre::AxisAlignedBox box = bod->getOgreNode()->_getWorldAABB();

				app.getAABB()->setToAABB( box );
				app.getAABB()->setVisible(true);
				if (DropOK)
					app.getAABB()->setMaterial("AABB/OK");
				else
					app.getAABB()->setMaterial("AABB/CX");

				// adjust camera min distance value.
				Ogre::Vector3 temp = box.getMaximum() - box.getMinimum();
				Ogre::Real rad = temp.x;
				if (temp.y > rad) { rad = temp.y; }
				if (temp.z > rad) { rad = temp.z; }
				rad = rad + 0.5;
				mMinCamDist = rad;
				rad = mFollowCam->getGoalDistance();
				if (rad < mMinCamDist)
					mFollowCam->setGoalDistance( mMinCamDist );

			}
			else
			{
				if (dummy->getPossibleAttach())
				{
					// something can be selected... so let's highlight it!
					Ogre::AxisAlignedBox box = dummy->getPossibleAttach()->getOgreNode()->_getWorldAABB();
					app.getAABB()->setToAABB( box );
					app.getAABB()->setVisible(true);
					app.getAABB()->setMaterial("AABB/OK");

					app.showHint( Ogre::String(" click mouse to pick up prop") );
				}
				else
				{
					app.getAABB()->setVisible(false);

					app.showStandardHint();
				}

				mMinCamDist = 1.5;
				if (mFollowCam->getGoalDistance() < 1.5) { mFollowCam->setGoalDistance(mMinCamDist); }
			}


		}


		//////////////////////////////////////////////////////////////
		// PROP REPEAT FUNCTION
		if (mInputDevice->isKeyDown(Ogre::KC_R) && (!mR))
		{
			if (!dummy->getAttached())
			{
				mR = true;
				CEGUI::EventArgs temp;
			
				app.addClicked( temp );
			}
		}

		if (!mInputDevice->isKeyDown(Ogre::KC_R))
			mR = false;

		//////////////////////////////////////////////////////////////
		// CAMERA CONTROLS.
		if (mInputDevice->isKeyDown(Ogre::KC_LCONTROL) || mInputDevice->isKeyDown(Ogre::KC_RCONTROL))
		{
			
			if ((!mInputDevice->isKeyDown(Ogre::KC_LSHIFT)) && (!mInputDevice->isKeyDown(Ogre::KC_RSHIFT)))
			{
				mFollowCam->setGoalYawAngle( Ogre::Radian(mFollowCam->getGoalYawAngle().valueRadians() + (mInputDevice->getMouseRelativeX()*-0.005)) );
				mFollowCam->setGoalPitchAngle( Ogre::Radian(mFollowCam->getGoalPitchAngle().valueRadians() + (mInputDevice->getMouseRelativeY()*0.005)) );
			
				mFollowCam->setGoalDistance( mFollowCam->getGoalDistance() + (mInputDevice->getMouseRelativeZ()*0.01) );
			}
			else
			{
				mFollowCam->setGoalDistance( mFollowCam->getGoalDistance() + (mInputDevice->getMouseRelativeX()*0.05) );
			}

			if (mFollowCam->getGoalDistance() < mMinCamDist)
				mFollowCam->setGoalDistance( mMinCamDist );
		}

		// center the mouse in placing mode.
		CEGUI::Renderer* rend;
		rend = CEGUI::System::getSingleton().getRenderer();
		CEGUI::MouseCursor::getSingleton().setPosition( CEGUI::Point(rend->getWidth()/2.0, rend->getHeight()/2.0) );

		// in this mode, escape takes us back to waiting mode.
		if ((mInputDevice->isKeyDown(Ogre::KC_ESCAPE)) && (!mESCAPE))
		{
			mESCAPE = true;
			app.setProgState( StuntPlayground::PS_WAITING );

			if (dummy->getAttached())
			{
				OgreNewt::Body* bod = dummy->getAttached();
				dummy->detach();

				if (bod->getType() == StuntPlayground::BT_PROP)
				{
					PropManager::getSingleton().removeProp( (Prop*)bod->getUserData() );
				}
				
				app.getAABB()->setVisible(false);
			}

			app.disableSaveReplay();
		}
	}


	//////////////////////////////////////////////////////////////
	// WAITING MODE (GAME MENU MODE)
	if ( app.getProgState() == StuntPlayground::PS_WAITING )
	{
		if ((mInputDevice->isKeyDown(Ogre::KC_ESCAPE)) && (!mESCAPE))
		{
			mESCAPE = true;

			if (!app.closeDialog())
				app.setProgState( StuntPlayground::PS_PLACING );
		}
	}

	//////////////////////////////////////////////////////////////
	// REPLAYING MODE
	if ( app.getProgState() == StuntPlayground::PS_REPLAYING )
	{
		// SWITCH CAMERAS
		if ((mInputDevice->isKeyDown(Ogre::KC_C)) && (!mC))
		{
			mC = true;
			app.nextCamera();
		}

		if (!mInputDevice->isKeyDown(Ogre::KC_C)) { mC = false; }

		// CAMERA UPDATE
		if ((app.getCameraView() == StuntPlayground::CV_FREE) && (mInputDevice->getMouseButton(0)))
		{
			if ((!mInputDevice->isKeyDown(Ogre::KC_LSHIFT)) && (!mInputDevice->isKeyDown(Ogre::KC_RSHIFT)))
			{
				mFollowCam->setGoalYawAngle( Ogre::Radian(mFollowCam->getGoalYawAngle().valueRadians() + (mInputDevice->getMouseRelativeX()*-0.005)) );
				mFollowCam->setGoalPitchAngle( Ogre::Radian(mFollowCam->getGoalPitchAngle().valueRadians() + (mInputDevice->getMouseRelativeY()*0.005)) );
			
				mFollowCam->setGoalDistance( mFollowCam->getGoalDistance() + (mInputDevice->getMouseRelativeZ()*0.01) );
			}
			else
			{
				mFollowCam->setGoalDistance( mFollowCam->getGoalDistance() + (mInputDevice->getMouseRelativeX()*0.05) );
			}

			if (mFollowCam->getGoalDistance() < 1.5)
				mFollowCam->setGoalDistance( 1.5 );
		}

		if (((mInputDevice->isKeyDown(Ogre::KC_SPACE)) && (!mSPACE)) || ((mInputDevice->isKeyDown(Ogre::KC_ESCAPE)) && (!mESCAPE)))
		{
			// go back to gameplay mode!
			mSPACE = true;
			mESCAPE = true;

			mUpdateNewton = true;

			WorldRecorder::getSingleton().stopPlaying();
			OgreNewt::BodyIterator::getSingleton().go( Application::updateProps );

			app.showGUI();
			
			app.mCar->playEngine();

			app.getDirtParticlesNode()->setVisible(true);
			app.getSparkParticlesNode()->setVisible(true);

			app.setProgState( StuntPlayground::PS_PLAYING );
			return true;
		}

		if (!mInputDevice->isKeyDown(Ogre::KC_SPACE))
			mSPACE = false;

		switch (app.getReplayControls()->getMode())
		{
		case ReplayControls::RM_END:
			mPlaybackTime = WorldRecorder::getSingleton().getElapsed();
			break;

		case ReplayControls::RM_FASTFORWARD:
			mPlaybackTime += evt.timeSinceLastFrame * 3.0;
			break;

		case ReplayControls::RM_PAUSE:
			break;

		case ReplayControls::RM_PLAY:
			mPlaybackTime += evt.timeSinceLastFrame;
			break;

		case ReplayControls::RM_PLAYSLOW:
			mPlaybackTime += evt.timeSinceLastFrame * 0.15;
			break;

		case ReplayControls::RM_REWIND:
			mPlaybackTime -= evt.timeSinceLastFrame * 3.0;
			break;

		case ReplayControls::RM_REWPLAY:
			mPlaybackTime -= evt.timeSinceLastFrame;
			break;

		case ReplayControls::RM_REWSLOW:
			mPlaybackTime -= evt.timeSinceLastFrame * 0.15;
			break;

		case ReplayControls::RM_TOP:
			mPlaybackTime = 0.0;
			break;
		}

		if (mPlaybackTime < 0) { mPlaybackTime = 0.0; }
		if (mPlaybackTime > WorldRecorder::getSingleton().getElapsed()) { mPlaybackTime = WorldRecorder::getSingleton().getElapsed(); }
		WorldRecorder::getSingleton().setTime( mPlaybackTime );

		app.showHint( Ogre::String(" Press SPACE or ESC to return to driving | current time: "+
			Ogre::StringConverter::toString( mPlaybackTime )+" of "+
			Ogre::StringConverter::toString( WorldRecorder::getSingleton().getElapsed() )+"sec") ); 

		app.updateReflection();

#ifdef _SP_DEBUG
		Ogre::OverlayElement* temp = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug10");
		temp->setCaption( "TIME: "+Ogre::StringConverter::toString(mPlaybackTime) +
			" / TOTAL: "+Ogre::StringConverter::toString(WorldRecorder::getSingleton().getElapsed()));
#endif	// _SP_DEBUG

		
	}


	//////////////////////////////////////////////////////////////
	// DRIVING MODE (CALLED ONCE PER RENDER)
	if (app.getProgState() == StuntPlayground::PS_PLAYING)
	{
		// record an instant replay!
		if ((mInputDevice->isKeyDown(Ogre::KC_F7)) && (!mREC))
		{
			mREC = true;

			if (!WorldRecorder::getSingleton().recorded())
			{
				if (!WorldRecorder::getSingleton().recording())
				{
					WorldRecorder::getSingleton().startRecording();
				}
			}
			else
			{
				if (WorldRecorder::getSingleton().recording())
				{
					WorldRecorder::getSingleton().stopRecording();
				}
				else
				{
					WorldRecorder::getSingleton().removeAllKeyframes();
					{
						WorldRecorder::getSingleton().startRecording();
					}
				}
			}
		}
		
		if (!mInputDevice->isKeyDown(Ogre::KC_F7))
			mREC = false;

		// Quit driving!!
		if ((mInputDevice->isKeyDown(Ogre::KC_ESCAPE)) && (!mESCAPE))
		{
			mESCAPE = true;

			if (WorldRecorder::getSingleton().recording())
				WorldRecorder::getSingleton().stopRecording();

			if (WorldRecorder::getSingleton().recorded())
			{
				WorldRecorder::getSingleton().removeAllRecordableObjects();
			}

			app.mCar->stopEngine();
			app.removeCar();

			app.restoreArena();
			
			app.hideGUI();

			app.getDirtParticlesNode()->setVisible(false);
			app.getSparkParticlesNode()->setVisible(false);

			app.setDummyVisible( true );

			// set the game mode.
			app.setProgState( StuntPlayground::PS_PLACING );
			app.disableSaveReplay();
		}

		// SWITCH CAMERAS
		if ((mInputDevice->isKeyDown(Ogre::KC_C)) && (!mC))
		{
			mC = true;
			app.nextCamera();
		}

		if (!mInputDevice->isKeyDown(Ogre::KC_C)) { mC = false; }

		// CAMERA UPDATE
		if ((app.getCameraView() == StuntPlayground::CV_FREE) && (mInputDevice->getMouseButton(0)))
		{
			if ((!mInputDevice->isKeyDown(Ogre::KC_LSHIFT)) && (!mInputDevice->isKeyDown(Ogre::KC_RSHIFT)))
			{
				mFollowCam->setGoalYawAngle( Ogre::Radian(mFollowCam->getGoalYawAngle().valueRadians() + (mInputDevice->getMouseRelativeX()*-0.005)) );
				mFollowCam->setGoalPitchAngle( Ogre::Radian(mFollowCam->getGoalPitchAngle().valueRadians() + (mInputDevice->getMouseRelativeY()*0.005)) );
			
				mFollowCam->setGoalDistance( mFollowCam->getGoalDistance() + (mInputDevice->getMouseRelativeZ()*0.01) );
			}
			else
			{
				mFollowCam->setGoalDistance( mFollowCam->getGoalDistance() + (mInputDevice->getMouseRelativeX()*0.05) );
			}

			if (mFollowCam->getGoalDistance() < 1.5)
				mFollowCam->setGoalDistance( 1.5 );
		}

		// SWITCH TO REPLAY MODE
		if ((mInputDevice->isKeyDown(Ogre::KC_SPACE)) && (!mSPACE))
		{
			// go into instant-replay mode!
			mSPACE = true;
			app.enableSaveReplay();

			if (WorldRecorder::getSingleton().recorded())
			{

				mUpdateNewton = false;

				if (WorldRecorder::getSingleton().recording())
					WorldRecorder::getSingleton().stopRecording();

				WorldRecorder::getSingleton().startPlaying();

				app.hideGUI();

				app.setProgState( StuntPlayground::PS_REPLAYING );

				app.getReplayControls()->setMode( StuntPlayground::ReplayControls::RM_PAUSE );

				app.getDirtParticlesNode()->setVisible(false);
				app.getSparkParticlesNode()->setVisible(false);

				app.getDirtParticles()->getEmitter(0)->setEmissionRate(0);
				app.getSparkParticles()->getEmitter(0)->setEmissionRate(0);

				app.mCar->stopEngine();

				mPlaybackTime = 0.0;
			}
		}

		if (!mInputDevice->isKeyDown(Ogre::KC_SPACE))
			mSPACE = false;

		// RESET THE VEHICLE
		if (mInputDevice->isKeyDown(Ogre::KC_R))
		{
			Ogre::Vector3 vel = app.mCar->getChassisBody()->getVelocity();
			
			if ((vel.length() < 0.5) && (app.mCar->getState() != Car::VS_AIRBORNE))
			{
				//reset the vehicle position slightly.
				Ogre::Vector3 pos;
				Ogre::Quaternion orient;

				app.mCar->getChassisBody()->getPositionOrientation( pos, orient);

				Ogre::Matrix3 rot;
				orient.ToRotationMatrix(rot);
				Ogre::Degree theang;
				Ogre::Real y = 0.0;

				for (int i=0; i<=360; i++)
				{
					Ogre::Quaternion testorient = orient * Ogre::Quaternion( Ogre::Degree(i), Ogre::Vector3::UNIT_X );
					testorient.ToRotationMatrix(rot);
					if (rot.GetColumn(1).y > y) { y = rot.GetColumn(1).y; theang = i; }
				}

				orient = orient * Ogre::Quaternion( Ogre::Degree(theang), Ogre::Vector3::UNIT_X );
				pos = pos + Ogre::Vector3(0,0.2,0);

				app.mCar->getChassisBody()->setPositionOrientation( pos, orient );
				app.mCar->reset();
			}
		}

		app.updateReflection();

		// update message!
		updateMessage( evt.timeSinceLastFrame );
	}



	if (!mInputDevice->isKeyDown(Ogre::KC_ESCAPE)) { mESCAPE = false; }

	// update the game camera!
	if (count > 0)
	{
		for (int i=0; i<count; i++)
			mFollowCam->update( m_update );
	}


//#ifdef _SP_DEBUG
	//////////////////////////////////////////////////////////////
	// OgreNewt debugger
	if (mInputDevice->isKeyDown(Ogre::KC_F3))
		OgreNewt::Debugger::getSingleton().showLines( mWorld );
	else
		OgreNewt::Debugger::getSingleton().hideLines();

#ifdef _SP_DEBUG
	Ogre::Quaternion goalorient;
	Ogre::Vector3 goalpos;
	app.getDummy()->getGoalPositionOrientation( goalpos, goalorient );
	Ogre::OverlayElement* text = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug8");
	Ogre::String thetext( "Goal Pos:" + Ogre::StringConverter::toString( goalpos ) );
	text->setCaption( thetext );

	if (WorldRecorder::getSingleton().recording())
	{
		Ogre::OverlayElement* temp = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug9");
		temp->setCaption( "(REC)" );
	}
	else
	{
		Ogre::OverlayElement* temp = Ogre::OverlayManager::getSingleton().getOverlayElement("StandardDebug9");
		temp->setCaption( "(   )" );
	}

#endif	// _SP_DEBUG

	/*
	const RenderTarget::FrameStats& stats = mWindow->getStatistics();
	Ogre::OverlayElement* ui = Ogre::OverlayManager::getSingleton().getOverlayElement("FPS");
	ui->setCaption( "FPS:" + Ogre::StringConverter::toString( stats.avgFPS ) );
	*/

	return true;
}

void MainFrameListener::userControls( Ogre::Real timeelapsed )
{
	Application& app = Application::getSingleton();

	//////////////////////////////////////////////////////////////
	// CAR CONTROLS!!
	if (app.getProgState() == StuntPlayground::PS_PLAYING)
	{
		if (app.mCar)
		{

			Ogre::Real throttle = 0.0f;
			Ogre::Real steer = 0.0f;
			bool brakes = false;

			// car controls
			if ((mInputDevice->isKeyDown(Ogre::KC_W)) || (mInputDevice->isKeyDown(Ogre::KC_UP))) { throttle = 2.0; }
			if ((mInputDevice->isKeyDown(Ogre::KC_S)) || (mInputDevice->isKeyDown(Ogre::KC_DOWN)))  { brakes = true; }
			if ((mInputDevice->isKeyDown(Ogre::KC_A)) || (mInputDevice->isKeyDown(Ogre::KC_LEFT)))  { steer -= 1.0; }
			if ((mInputDevice->isKeyDown(Ogre::KC_D)) || (mInputDevice->isKeyDown(Ogre::KC_RIGHT)))  { steer += 1.0; }
			if ((mInputDevice->isKeyDown(Ogre::KC_Q)) && (!mKeySU))
			{ 
				if (app.mCar->getManualTransmission()) { mKeySU = true; app.mCar->shiftUp(); }
			}
			if ((mInputDevice->isKeyDown(Ogre::KC_Z)) && (!mKeySD)) 
			{ 
				if (app.mCar->getManualTransmission()) { mKeySD = true; app.mCar->shiftDown(); }
			}

			if (!mInputDevice->isKeyDown(Ogre::KC_Q)) { mKeySU = false; }
			if (!mInputDevice->isKeyDown(Ogre::KC_Z)) { mKeySD = false; }

			if (mInputDevice->isKeyDown(Ogre::KC_LSHIFT)) { throttle *= 2.0; }

			app.mCar->setThrottleSteering( throttle, steer );
			app.mCar->setBrakes( brakes );
		}

		
	}

}

void MainFrameListener::updateMessage( Ogre::Real timestep )
{
	// message active?

	if (mMessage->getCaption() != "")
	{
		if (mMessageText != mMessage->getCaption())
		{
			//message has changed!
			mMessageText = mMessage->getCaption();
			mMessage->setLeft( (mWindow->getWidth()/2.0)+5 );
			
		}

		// move message!
		Ogre::Real lf = mMessage->getLeft();

		Ogre::Real speed = mWindow->getWidth();

		if ((lf > (mWindow->getWidth()/-3.0)) && (lf < (mWindow->getWidth()/3.0)))
		{
			speed /= 4.0; 
		}

		lf -= timestep * speed;

		if (lf < ((mWindow->getWidth()/-2.0)-mMessage->getWidth()-5))
		{
			//reset
			mMessage->setCaption("");
			mMessageText = "";
			lf = (mWindow->getWidth()/2.0)+5;
		}

		mMessage->setLeft( lf );

	}
}


void MainFrameListener::startReplayPlayback( Ogre::Real time )
{
	mWorld->update( 0.01 );

	mUpdateNewton = false;
	mPlaybackTime = time;
}



}	// end NAMESPACE StuntPlayground