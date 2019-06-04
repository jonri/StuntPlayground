/*
	Stunt Playground
	
	StuntPlaygroundApplication
*/
#include "CEGUI.h"
#include "Ogre.h"
#include "OgreNewt.h"
#include ".\StuntPlaygroundApplication.h"
#include ".\MainFrameListener.h"
#include ".\CEGUIFrameListener.h"
#include ".\StuntPlayground.h"
#include ".\StandardButton.h"
#include ".\PropListItem.h"
#include ".\StuntPlayground.h"
#include ".\Prop.h"
#include ".\PropManager.h"
#include ".\FollowCamera.h"
#include ".\DummyBody.h"
#include ".\CollisionCallbacks.h"
#include ".\vehicle.h"
#include ".\dialogs.h"
#include ".\WorldRecorder.h"
#include ".\SoundManager.h"
#include "Line3D.h"
#include "OgreHardwarePixelBuffer.h"

#include <tinyxml.h>

namespace StuntPlayground
{

Application& Application::getSingleton()
{
	static Application instance;
	return instance;
}

Application* Application::getSingletonPtr()
{
	static Application instance;
	return &instance;
}



Application::Application(void)
{
	//create Newton World.
	mWorld = new OgreNewt::World();

	mPane = NULL;
	mState = StuntPlayground::PS_WAITING;
	mSelectedProp = NULL;

	mFollowCam = NULL;

	mArenaBody = NULL;
	mPropBaseNode = NULL;

	mFrameListener = NULL;
	mCEGUIListener = NULL;

	mMatArena = NULL;
	mMatCar = NULL;
	mMatProp = NULL;
	mMatDummy = NULL;

	mMatPairArenaVsCar = NULL;
	mMatPairCarVsProp = NULL;

	mCarArenaCallback = NULL;
	mDummyCallback = NULL;
	mPropCallback = NULL;

	mCurrentVehicle = 0;

	mFollowCamOldDist = 5.0;

#ifdef _SP_DEBUG
	mDebugLines = NULL;
#endif	// _SP_DEBUG

	mCar = NULL;

	mDummyBody = NULL;
	mAABB = NULL;

	mIteratorRemoveProps = NULL;

	for (unsigned int i=0; i<StuntPlayground::CV_SIZE; i++)
	{
		CameraAngle test;
		test.setup( Ogre::Radian(0), Ogre::Radian(15), 10.0, Ogre::Vector3(0,0,0));

		mCameraAngles.push_back(test);
	}

	mCurrentCamAngle = StuntPlayground::CV_FOLLOW;


	mCurrentReflectCam = 0;
	mReflectionListener = NULL;
	mReflectCamEnabled = false;
}

Application::~Application(void)
{
	// delete things created in the constructor
	if (mCar)
	{
		delete mCar;
		mCar = NULL;
	}

	delete mWorld;
}



void Application::createScene()
{


#ifdef _SP_DEBUG_LOG
	// debug log.
	mDebugLog = Ogre::LogManager::getSingleton().createLog("DebugLog.txt");
	mDebugLog->logMessage("=============================================================");
	mDebugLog->logMessage("				Stunt Playgrond DEBUG LOG");
	mDebugLog->logMessage("=============================================================");
#endif	// _SP_DEBUG_LOG


	// PHYSICS SYSTEM SETTINGS
	mWorld->setSolverModel( 5 );
	mWorld->setFrictionModel( OgreNewt::World::FM_ADAPTIVE );

	///////////////////////////////////////////
	//  INIT SOUND MANAGER
	SoundManager::getSingleton().Init( SoundManager::MR_44100, 32 );

	SoundManager& sMgr = SoundManager::getSingleton();


	// ENCODE SOUNDS
	/*
	Ogre::FileInfoListPtr list = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo("Sounds", "*.wav");
	for (Ogre::FileInfoList::iterator it = list->begin(); it != list->end(); it++)
	{
		std::string outfile = it->filename.substr(0, it->filename.find_first_of("."));

		sMgr.encodeSound( "../../Media/Sounds/"+it->filename, "../../Media/Sounds/"+outfile+".sound" );
	}
	*/


	// LOAD SOUNDS
	Ogre::FileInfoListPtr sounds = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo("Sounds", "*.sound");
	for (Ogre::FileInfoList::iterator it = sounds->begin(); it != sounds->end(); it++)
	{
		std::string name;
		std::string filename;

		name = it->filename.substr(0, it->filename.find_first_of("."));
		filename = "../../Media/Sounds/"+it->filename;

		sMgr.addSound( name, filename );
	}

	
	
	mSceneMgr->setShadowTechnique( Ogre::SHADOWTYPE_STENCIL_MODULATIVE );
	

	/////////////////////////////////////////////
	// load the basic background.
	mSceneMgr->setSkyBox(true, "World/NoonSky");

	/////////////////////////////////////////////
	// base track level rigid body.
	Ogre::SceneNode* levelnode = mSceneMgr->getRootSceneNode()->createChildSceneNode("World/TrackNode");
	Ogre::Entity* ent = mSceneMgr->createEntity("World/TrackEntity", "track.mesh");
	levelnode->attachObject(ent);
	// quick check for material indexes...
	Ogre::MeshPtr mesh = ent->getMesh();
	for (unsigned short sub = 0; sub < mesh->getNumSubMeshes(); sub++)
	{
		if (mesh->getSubMesh(sub)->getMaterialName() == "TRACK_BASE/SOLID/road.psd")
			mArenaIDRoad = sub;

		if (mesh->getSubMesh(sub)->getMaterialName() == "TRACK_BASE/SOLID/road_checker.png")
			mArenaIDRoadChecker = sub;

		if (mesh->getSubMesh(sub)->getMaterialName() == "TRACK_BASE/SOLID/ground.psd")
			mArenaIDGround = sub;

#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage(" TRACK SUBMESHES: "+Ogre::StringConverter::toString(sub)+" Material: "+mesh->getSubMesh(sub)->getMaterialName());
#endif	// _SP_DEBUG_LOG
	}

	/////////////////////////////////////////////
	// setup the various Newton materials...
	createMaterials();

#ifdef _SP_DEBUG_LOG
	mDebugLog->logMessage("Arena IDs: road:"+Ogre::StringConverter::toString(mArenaIDRoad)+
		"road_checker: "+Ogre::StringConverter::toString(mArenaIDRoadChecker)+
		" ground:"+Ogre::StringConverter::toString(mArenaIDGround));
#endif	// _SP_DEBUG_LOG


	ent->setCastShadows(false);
	OgreNewt::Collision* col = new OgreNewt::CollisionPrimitives::TreeCollision( mWorld, levelnode, true );
	mArenaBody = new OgreNewt::Body( mWorld, col, (int)StuntPlayground::BT_ARENA );
	mArenaBody->attachToNode( levelnode );
	mArenaBody->setMaterialGroupID( mMatArena );
	mArenaBody->setType( StuntPlayground::BT_ARENA );
    delete col;

	// world limits temp object.
	ent = mSceneMgr->createEntity( "worldLimitsEntity", "world_limits.mesh" );
	Ogre::SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	node->attachObject( ent );

	col = new OgreNewt::CollisionPrimitives::TreeCollision( mWorld, node, false );
	mWorldLimitsBody = new OgreNewt::Body( mWorld, col, BT_WORLD_LIMIT );
	mWorldLimitsBody->setMaterialGroupID( mMatArena );
	Ogre::AxisAlignedBox aabb = col->getAABB();
	aabb.merge( aabb.getMaximum() * 1.1f );
	aabb.merge( aabb.getMinimum() * 1.1f );
	mWorld->setWorldSize( aabb );

	delete col;

	node->detachAllObjects();
	mSceneMgr->destroyEntity( ent );
	node->getParentSceneNode()->removeAndDestroyChild( node->getName() );


	/////////////////////////////////////////////
	// INIT PROP MANAGER
	PropManager::getSingleton().init( "../../Media/Props/props.xml", mWorld, mMatProp, mSceneMgr );


	/////////////////////////////////////////////
	// setup CEGUI!
	mGUIRenderer = new CEGUI::OgreCEGUIRenderer( mWindow, Ogre::RENDER_QUEUE_OVERLAY, false, 3000 );
	new CEGUI::System( mGUIRenderer );

	// load up CEGUI stuff...
	try
	{
		CEGUI::Logger::getSingleton().setLoggingLevel( CEGUI::Informative );

		CEGUI::SchemeManager::getSingleton().loadScheme((CEGUI::utf8*)"../../Media/GUI/schemes/WindowsLook.scheme");
		CEGUI::System::getSingleton().setDefaultMouseCursor((CEGUI::utf8*)"WindowsLook", (CEGUI::utf8*)"MouseArrow");
		CEGUI::System::getSingleton().setDefaultFont((CEGUI::utf8*)"Commonwealth-10");

		CEGUI::Window* sheet = CEGUI::WindowManager::getSingleton().createWindow( (CEGUI::utf8*)"DefaultWindow", (CEGUI::utf8*)"root_wnd" );
		CEGUI::System::getSingleton().setGUISheet( sheet );

		makeGUI();
		setupGUI();
	
	}
	catch (CEGUI::Exception)
	{}

	/////////////////////////////////////////////
	loadGameSettings();

	SoundManager::getSingleton().setMasterSFXVolume( mSettings.sfxVolume );

	/////////////////////////////////////////////
	// shadows!
	mSceneMgr->setShadowTechnique( mSettings.shadowTechnique );

	/////////////////////////////////////////////
	makeDummy();

	/////////////////////////////////////////////
	// sun light
	Ogre::Light* light = mSceneMgr->createLight("World/SunLight");
	light->setType(Ogre::Light::LT_DIRECTIONAL);
	light->setDirection( Ogre::Vector3(-100,-500,-300) );
	//light->setPosition(100,500,300);
	mSceneMgr->setAmbientLight( Ogre::ColourValue(0.6,0.6,0.6) );


	/////////////////////////////////////////////
	// dirt particle system
	mDirtParticles = Ogre::ParticleSystemManager::getSingleton().createSystem("DirtParticles", "WalaberDirtTest");
	mDirtParticlesNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mDirtParticlesNode->attachObject( mDirtParticles );
	mDirtParticles->getEmitter(0)->setEmissionRate(0.0);

	/////////////////////////////////////////////
	// spark particles system
	mSparkParticles = Ogre::ParticleSystemManager::getSingleton().createSystem("SparkParticles", "WalaberSparkTest");
	mSparkParticlesNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	mSparkParticlesNode->attachObject( mSparkParticles );
	mSparkParticles->getEmitter(0)->setEmissionRate(0.0);



	/////////////////////////////////////////////
	//temp camera setting
	mCamera->setPosition(-50,10,20);
	mCamera->lookAt(0,0,0);

	//temp camera setting
	mFollowCam = new FollowCamera( mCamera );
	mFollowCam->setGoalNode( mArenaBody->getOgreNode() );
	mFollowCam->setGoalDistance( 30.0f );
	mFollowCam->setJustFollow(true);
	mFollowCam->setGoalNode( mDummyBody->getNewtonBody()->getOgreNode() );


	/////////////////////////////////////////////
	// AABB visualizer.
	mAABB = new MyAABB("AABB/Visualizer", mSceneMgr, "box.mesh", "AABB/OK");


	/////////////////////////////////////////////
	// fill the list of vehicles.
	mVehicleList = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo( "Vehicles", "*.car" );

	for (int i=0; i<mVehicleList->size(); i++)
	{
		Ogre::FileInfo file;
		file = mVehicleList->at(i);

#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage("Vehicle found: "+ file.path+file.filename);
#endif
	}

	/////////////////////////////////////////////
	// setup iterators.
	OgreNewt::BodyIterator::getSingleton().Init( mWorld );

	/////////////////////////////////////////////
	// setup instant replay recorder
	StuntPlayground::WorldRecorder::getSingleton().Init( 15.0 );

	/////////////////////////////////////////////
	// default camera angles
	setDefaultCameraAngles();

	/////////////////////////////////////////////
	// reflection cameras.
	makeReflectionCameras();

	/////////////////////////////////////////////
	// load high scores.
	mScores.load( "../../Media/high.scores" );
	mGUI_JP_DISTt->setCaption( Ogre::StringConverter::toString( mScores.mDist ) );
	mGUI_JP_TIMEt->setCaption( Ogre::StringConverter::toString( mScores.mTime ) );
	mGUI_JP_FLIPSt->setCaption( Ogre::StringConverter::toString( mScores.mFlips ) );
	mGUI_JP_2WHt->setCaption( Ogre::StringConverter::toString( mScores.m2WH ) );


	/////////////////////////////////////////////
	// debug node
#ifdef _SP_DEBUG
	mDebugNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("DebugNode");

	// show the debug overlay...
	Ogre::Overlay* debugOverlay = Ogre::OverlayManager::getSingleton().getByName("Walaber/StandardDebug");
	debugOverlay->show();
#endif	// _SP_DEBUG


	/////////////////////////////////////////////
	setProgState( StuntPlayground::PS_LOGO );

	
}

void Application::makeDummy()
{
	mDummyBody = new DummyBody( mWorld, mMatDummy, Ogre::Vector3(3,3,3), mSceneMgr );
	mDummyBody->setSpringConst( 200.0 );
	mDummyBody->setSpringDamp( 30.0 );
	mDummyBody->setSize( Ogre::Vector3(1,1,1) );
	mDummyBody->getGoalPositionOrientation( Ogre::Vector3(0,5,0), Ogre::Quaternion(Ogre::Quaternion::IDENTITY) );
	
}

void Application::setDummyCallback()
{
	mDummyBody->getNewtonBody()->setCustomForceAndTorqueCallback( Application::dummyForceCallback );
}

void Application::setDummyVisible( bool setting )
{
	if (setting)
	{
		mDummyBody->getNewtonBody()->setAutoFreeze(false);
		mDummyBody->getNewtonBody()->unFreeze();
		mDummyBody->getNewtonBody()->getOgreNode()->setVisible(true);
	}
	else
	{
		mDummyBody->getNewtonBody()->setAutoFreeze(true);
		mDummyBody->getNewtonBody()->freeze();
		mDummyBody->getNewtonBody()->getOgreNode()->setVisible(false);

#ifdef _SP_DEBUG
		mDebugNode->detachAllObjects();
		if (mDebugLines)
			delete mDebugLines;
		mDebugLines = NULL;
#endif	// _SP_DEBUG

	}
}


//////////////////////////////////////////////////////////////////////////
//		CEGUI SETUP
//////////////////////////////////////////////////////////////////////////
void Application::makeGUI()
{
	using namespace CEGUI;

	WindowManager& wMgr = WindowManager::getSingleton();

	Window* root = wMgr.getWindow((utf8*)"root_wnd");

	// LOGO image
	mLOGO = (StaticImage*)wMgr.createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"Intro/Logo");
	root->addChildWindow( mLOGO );
	mLOGO->setSize( Size(0.5,0.7) );
	mLOGO->setPosition( Point(0.25,0.15) );
	mLOGO->setFrameEnabled(false);
	mLOGO->setImage((utf8*)"LOGO", (utf8*)"Logo");
	mLOGO->setBackgroundEnabled(false);

	// GUI!
	makeOnScreenGUI();

	//////////////////////////////////////////////////////////////////////////////
	// Main window, which holds all of the tools.
	mPane = (TabPane*)wMgr.createWindow((utf8*)"WindowsLook/TabPane",(utf8*)"Main/Pane");
	root->addChildWindow( mPane );
	mPane->setSize( Size(0.2,0.8) );
	mPane->setPosition( Point(-0.2,0.01) );

	StaticImage* logo = (StaticImage*)wMgr.createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"Main/Logo");
	mPane->addChildWindow( logo );
	logo->setSize( Size(0.9,0.04) );
	logo->setPosition( Point(0.05,0.01) );
	logo->setImage((utf8*)"UI", (utf8*)"Logo" );
	logo->setBackgroundEnabled(false);

	mPrevImg = (StaticImage*)wMgr.createWindow((utf8*)"WindowsLook/StaticImage",(utf8*)"Main/Image");
	mPane->addChildWindow(mPrevImg);
	mPrevImg->setPosition( Point(0.05,0.05) );
	mPrevImg->setSize( Size(0.9,0.2) );
	mPrevImg->setImage((utf8*)"Jumps1", (utf8*)"Small_Jump1");
	mPrevImg->setFrameColours( colour(1.0,0,0) );

	Listbox* lbox = (Listbox*)wMgr.createWindow((utf8*)"WindowsLook/Listbox",(utf8*)"Main/Listbox");
	mPane->addChildWindow( lbox );
	lbox->setPosition( Point(0.05,0.255) );
	lbox->setSize( Size(0.9,0.23) );
	lbox->setMultiselectEnabled(false);
	lbox->subscribeEvent( Listbox::EventMouseClick, Event::Subscriber( &Application::playClickSound, this ) );
	lbox->setWantsMultiClickEvents( false );

	// ADD Button
	mButAdd = new StandardButton((utf8*)"Main/Add", (utf8*)"Add Prop");
	mPane->addChildWindow( mButAdd->getButton() );
	mButAdd->getButton()->setSize( Size(0.9,0.07) );
	mButAdd->getButton()->setPosition( Point(0.05,0.5) );
	mButAdd->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );

	// LOAD Button
	mButOpen = new StandardButton((utf8*)"Main/Open", (utf8*)"Load Arena");
	mPane->addChildWindow( mButOpen->getButton() );
	mButOpen->getButton()->setSize( Size(0.9,0.04) );
	mButOpen->getButton()->setPosition( Point(0.05,0.61) );
	mButOpen->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );

	// SAVE Button
	mButSave = new StandardButton((utf8*)"Main/Save", (utf8*)"Save Arena");
	mPane->addChildWindow( mButSave->getButton() );
	mButSave->getButton()->setSize( Size(0.9,0.04) );
	mButSave->getButton()->setPosition( Point(0.05,0.67) );
	mButSave->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );

	// NEW Button
	mButNew = new StandardButton((utf8*)"Main/New", (utf8*)"Clear Arena");
	mPane->addChildWindow( mButNew->getButton() );
	mButNew->getButton()->setSize( Size(0.9,0.04) );
	mButNew->getButton()->setPosition( Point(0.05,0.73) );
	mButNew->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );

	// GO button
	mButGo = new StandardButton((utf8*)"Main/GO", (utf8*)"GO->");
	mPane->addChildWindow( mButGo->getButton() );
	mButGo->getButton()->setSize( Size(0.9,0.1) );
	mButGo->getButton()->setPosition( Point(0.05,0.8) );
	mButGo->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );


	// quit button.
	mButQuit = new StandardButton((utf8*)"Main/Quit", (utf8*)"QUIT");
	mPane->addChildWindow( mButQuit->getButton() );
	mButQuit->getButton()->setSize( Size(0.9,0.05) );
	mButQuit->getButton()->setPosition( Point(0.05, 0.92) );

	mLoadDialog = new LoadDialog( root );
	mLoadDialog->hide();

	mSaveDialog = new SaveDialog( root );
	mSaveDialog->hide();

	mErrorDialog = new ErrorDialog( root );
	mErrorDialog->hide();

	mSettingDialog = new SettingDialog( root, mSceneMgr );
	mSettingDialog->hide();

	mReplayControls = new ReplayControls( root );


	/////////////////////////////
	// FILL IN PROP LISTBOX
	PropManager::PropDefinitionMap map = PropManager::getSingleton().getPropDefinitionMap();

	for (PropManager::PropDefinitionIterator it = map.begin(); it != map.end(); it++)
	{
		PropListItem* item = new PropListItem( it->second.mBodyFile, it->second.mImageSet, it->second.mImage, it->first, it->second.mHitSound );
		item->setAutoDeleted(false);
		lbox->addItem( item );
	}





	//////////////////////////////////////////////////////////////////////////
	// 2nd Pane with other buttons
	mPane2 = (TabPane*)wMgr.createWindow((utf8*)"WindowsLook/TabPane",(utf8*)"Main/Pane2");
	root->addChildWindow( mPane2 );
	mPane2->setSize( Size(0.2,0.2) );
	mPane2->setPosition( Point(1.0,0.01) );

	// Settings button
	mButSettings = new StandardButton((utf8*)"Main/Settings", (utf8*)"SETTINGS");
	mPane2->addChildWindow( mButSettings->getButton() );
	mButSettings->getButton()->setSize( Size(0.9,0.15) );
	mButSettings->getButton()->setPosition( Point(0.05,0.05) );
	mButSettings->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );

	// Load Replay Button
	mButLoadReplay = new StandardButton((utf8*)"Main/LoadReplay", (utf8*)"LOAD REPLAY");
	mPane2->addChildWindow( mButLoadReplay->getButton() );
	mButLoadReplay->getButton()->setSize( Size(0.9,0.15) );
	mButLoadReplay->getButton()->setPosition( Point(0.05, 0.25) );
	mButLoadReplay->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );

	// Save Replay Button
	mButSaveReplay = new StandardButton((utf8*)"Main/SaveReplay", (utf8*)"SAVE REPLAY");
	mPane2->addChildWindow( mButSaveReplay->getButton() );
	mButSaveReplay->getButton()->setSize( Size(0.9,0.15) );
	mButSaveReplay->getButton()->setPosition( Point(0.05, 0.45) );
	mButSaveReplay->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );
	disableSaveReplay();

	// Clear highscores Button
	mButClearScores = new StandardButton((utf8*)"Main/ClearScores", (utf8*)"CLEAR SCORES");
	mPane2->addChildWindow( mButClearScores->getButton() );
	mButClearScores->getButton()->setSize( Size(0.9,0.15) );
	mButClearScores->getButton()->setPosition( Point(0.05, 0.65) );
	mButClearScores->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber( &Application::playClickSound, this ) );
	


	//////////////////////////////////////
	// Hint Textbox
	mHint = (StaticText*)wMgr.createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"Main/Hint" );
	root->addChildWindow( mHint );
	mHint->setPosition( Point(0.0, 0.96 ) );
	mHint->setSize( Size(1, 0.03) );
	mHint->setTextColours( colour(1,1,1) );
	mHint->setFrameEnabled(false);
	showHint(Ogre::String(""));


	hideGUI();

}

void Application::makeOnScreenGUI(  )
{
	mGUI = Ogre::OverlayManager::getSingleton().getByName("StuntPlayground/BasicInfo");
	mGUI_CAMERA = Ogre::OverlayManager::getSingleton().getOverlayElement("CAMERA");
	mGUI_REC = Ogre::OverlayManager::getSingleton().getOverlayElement("REC");
	mGUI_JP_DISTt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_DISTt");
	mGUI_JP_TIMEt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_TIMEt");
	mGUI_JP_FLIPSt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_FLIPSt");
	mGUI_JP_2WHt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_2WHt");
	
}
void Application::setupGUI()
{
	using namespace CEGUI;

	WindowManager::getSingleton().getWindow("Main/Quit")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::handleQuit, this) );

	WindowManager::getSingleton().getWindow("Main/Listbox")->
		subscribeEvent( Listbox::EventSelectionChanged, Event::Subscriber(&Application::ListboxChanged, this) );

	WindowManager::getSingleton().getWindow("Main/Add")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::addClicked, this) );

	WindowManager::getSingleton().getWindow("Main/GO")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::goClicked, this) );

	WindowManager::getSingleton().getWindow("Main/Open")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::openClicked, this) );

	WindowManager::getSingleton().getWindow("Main/Save")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::saveClicked, this) );

	WindowManager::getSingleton().getWindow("Main/New")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::newClicked, this) );

	WindowManager::getSingleton().getWindow("Main/Settings")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::settingsClicked, this) );

	WindowManager::getSingleton().getWindow("Main/LoadReplay")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::loadReplayClicked, this) );

	WindowManager::getSingleton().getWindow("Main/SaveReplay")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::saveReplayClicked, this) );

	WindowManager::getSingleton().getWindow("Main/ClearScores")->
		subscribeEvent( PushButton::EventClicked, Event::Subscriber(&Application::clearScoresClicked, this) );

	WindowManager::getSingleton().getWindow("LoadDialog/Window")->
		subscribeEvent( FrameWindow::EventShown, Event::Subscriber(&Application::disablePane, this) );

	WindowManager::getSingleton().getWindow("LoadDialog/Window")->
		subscribeEvent( FrameWindow::EventHidden, Event::Subscriber(&Application::backFromLoad, this) );

	WindowManager::getSingleton().getWindow("SaveDialog/Window")->
		subscribeEvent( FrameWindow::EventShown, Event::Subscriber(&Application::disablePane, this) );

	WindowManager::getSingleton().getWindow("SaveDialog/Window")->
		subscribeEvent( FrameWindow::EventHidden, Event::Subscriber(&Application::backFromSave, this) );

	WindowManager::getSingleton().getWindow("ErrorDialog/Window")->
		subscribeEvent( FrameWindow::EventShown, Event::Subscriber(&Application::disablePane, this) );

	WindowManager::getSingleton().getWindow("ErrorDialog/Window")->
		subscribeEvent( FrameWindow::EventHidden, Event::Subscriber(&Application::enablePane, this) );

	WindowManager::getSingleton().getWindow("SettingDialog/Window")->
		subscribeEvent( FrameWindow::EventShown, Event::Subscriber(&Application::disablePane, this) );

	WindowManager::getSingleton().getWindow("SettingDialog/Window")->
		subscribeEvent( FrameWindow::EventHidden, Event::Subscriber(&Application::backFromSetting, this) );
	

}

void Application::showGUI()
{
	mGUI->show();
	mGUI_CAMERA->hide();
	mGUI_REC->hide();

	mHint->hide();
}

void Application::hideGUI()
{
	mGUI->hide();

	mHint->show();
}

void Application::showGUI_REC()
{
	mGUI_CAMERA->show();
	mGUI_REC->show();

}

void Application::hideGUI_REC()
{
	mGUI_REC->hide();

}

void Application::updateOldMessages( Ogre::String text )
{
	for (int i=1; i<=5; i++)
	{
		Ogre::String cur = "OldMessage" + Ogre::StringConverter::toString(i);
		Ogre::String next = "OldMessage" + Ogre::StringConverter::toString(i+1);

		Ogre::OverlayElement* txt = Ogre::OverlayManager::getSingleton().getOverlayElement(cur);

		if (i<5)
		{
			txt->setCaption( Ogre::OverlayManager::getSingleton().getOverlayElement(next)->getCaption() );
		}
		else
		{
			txt->setCaption( text );
		}
	}

}

void Application::possibleJumpRecord( Ogre::Real dist, Ogre::Real time, Ogre::Real flips )
{
	if (mState == StuntPlayground::PS_CHOOSING)
		return;

	if (mScores.possibleJumpDist( dist ))
	{
		Ogre::OverlayElement* txt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_DISTt");
		txt->setCaption( Ogre::StringConverter::toString(dist)+"m" );
	}

	if (mScores.possibleJumpTime( time ))
	{
		Ogre::OverlayElement* txt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_TIMEt");
		txt->setCaption( Ogre::StringConverter::toString(time)+"sec" );
	}

	if (mScores.possibleFlips( flips ))
	{
		Ogre::OverlayElement* txt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_FLIPSt");
		txt->setCaption( Ogre::StringConverter::toString(flips) );
	}

}

void Application::possible2WheelRecord( Ogre::Real time )
{
	if (mState == StuntPlayground::PS_CHOOSING)
		return;

	if (mScores.possible2Wheels( time ))
	{
		Ogre::OverlayElement* txt = Ogre::OverlayManager::getSingleton().getOverlayElement("JP_2WHt");
		txt->setCaption( Ogre::StringConverter::toString(time)+"sec" );
	}
}

//////////////////////////////////////////////////////////////////////////
//		OGRENEWT MATERIALS
//////////////////////////////////////////////////////////////////////////
void Application::createMaterials()
{
	// make and setup all materials!!

	mMatArena = mWorld->getDefaultMaterialID();
	mMatCar = new OgreNewt::MaterialID( mWorld );
	mMatProp = new OgreNewt::MaterialID( mWorld );
	mMatDummy = new OgreNewt::MaterialID( mWorld );
	mMatPlacer = new OgreNewt::MaterialID( mWorld );

	mMatPairArenaVsCar = new OgreNewt::MaterialPair( mWorld, mMatArena, mMatCar );
	mMatPairCarVsProp = new OgreNewt::MaterialPair( mWorld, mMatCar, mMatProp );

	// temp material pair, doesn't need to be remembered.
	OgreNewt::MaterialPair* temp = NULL;

	// CAR vs ARENA interactions
	mCarArenaCallback = new CarArenaCallback();
	mMatPairArenaVsCar->setContactCallback( mCarArenaCallback );
	mCarArenaCallback->setIDs( mArenaIDRoad, mArenaIDGround, mArenaIDRoadChecker );
	mCarArenaCallback->setCollisionSound( "CarCrash" );

	// DUMMY vs DUMMY
	temp = new OgreNewt::MaterialPair( mWorld, mMatDummy, mMatDummy );
	temp->setDefaultCollidable( 0 );
	delete temp;

	// DUMMY vs ARENA
	temp = new OgreNewt::MaterialPair( mWorld, mMatDummy, mMatArena );
	temp->setDefaultFriction( 0.0, 0.0 );
	delete temp;

	// DUMMY vs PROP
	temp = new OgreNewt::MaterialPair( mWorld, mMatDummy, mMatProp );
	// set custom callback here...
	mDummyCallback = new DummyCollisionCallback();
	temp->setContactCallback( mDummyCallback );
	delete temp;

	// PLACER vs DUMMY
	temp = new OgreNewt::MaterialPair( mWorld, mMatPlacer, mMatDummy );
	temp->setDefaultCollidable( 0 );
	delete temp;

	// PLACER vs ARENA
	temp = new OgreNewt::MaterialPair( mWorld, mMatPlacer, mMatArena );
	temp->setDefaultFriction( 0.0, 0.0 );
	delete temp;

	// PROP vs PROP
	temp = new OgreNewt::MaterialPair( mWorld, mMatProp, mMatProp );
	mPropCallback = new PropCollision();
	temp->setContactCallback( mPropCallback );
	delete temp;

	// PROP vs ARENA
	temp = new OgreNewt::MaterialPair( mWorld, mMatProp, mMatArena );
	temp->setContactCallback( mPropCallback );
	delete temp;

	// CAR vs DUMMY
	temp = new OgreNewt::MaterialPair( mWorld, mMatCar, mMatDummy );
	temp->setDefaultCollidable( 0 );
	delete temp;

	// CAR vs PROP
	mCarPropCallback = new CarPropCollision();
	mMatPairCarVsProp->setContactCallback( mCarPropCallback );


}

//////////////////////////////////////////////////////////////////////////
//		CHOOSING CAR FUNCTIONS
//////////////////////////////////////////////////////////////////////////
void Application::makeCar()
{
	if (mCar)
	{
		removeReflectionCameras();
		delete mCar;
	}

	mCar = NULL;

	//for (int i=0;i<10;i++)
	//	mWorld->update( (1.0/200.0) );

	Ogre::FileInfo file = mVehicleList->at(mCurrentVehicle);
	
	mCar = new Car( mWorld, mSceneMgr->getRootSceneNode(), mSceneMgr, "../../Media/vehicles/"+file.filename , mMatCar );
	mCar->setManualTransmission( mSettings.useManualTransmission );

	Ogre::Quaternion orient;
	Ogre::Matrix3 rot;
	rot.FromEulerAnglesXYZ(Ogre::Degree(0), Ogre::Degree(90), Ogre::Degree(0));
	orient.FromRotationMatrix( rot );
	mCar->getChassisBody()->setPositionOrientation( Ogre::Vector3(-100,-2,45), orient );

	// set camera mode.
	mFollowCam->setJustFollow( false );
	mFollowCam->setGoalNode( mCar->getChassisBody()->getOgreNode() );
	mFollowCamOldDist = mFollowCam->getGoalDistance();
	mCamera->setPosition( Ogre::Vector3(100,100,-45) );
	mFollowCam->setLookNode( mCar->getLookNode() );
	
	goCamera();

	// REFLECTION CAMERAS
	applyReflectionCameras();


	//for (int i=0;i<10;i++)
	//	mWorld->update( (1.0/200.0) );

}

void Application::nextCar()
{
	mCurrentVehicle++;

	if (mCurrentVehicle > (mVehicleList->size()-1))
		mCurrentVehicle = 0;

	makeCar();
}

void Application::prevCar()
{
	if (mCurrentVehicle > 0)
	{
		mCurrentVehicle--;
	}
	else
	{
		mCurrentVehicle = mVehicleList->size()-1;
	}

	makeCar();
}


void Application::removeCar()
{
	// reset the camera.
	mFollowCam->setJustFollow( true );
	mFollowCam->setGoalNode( mDummyBody->getNewtonBody()->getOgreNode() );
	mFollowCam->setGoalDistance( mFollowCamOldDist );
	mFollowCam->setLookNode( NULL );

	// remove reflection cameras
	removeReflectionCameras();

	// remove the vehicle!
	delete mCar;
	mCar = NULL;
	
}


void Application::goCamera()
{
	if (mCurrentCamAngle != StuntPlayground::CV_FREE)
	{
		mFollowCam->setJustFollow(false);
		mFollowCam->setGoalYawAngle( mCameraAngles[mCurrentCamAngle].mYawAngle );
		mFollowCam->setGoalPitchAngle( mCameraAngles[mCurrentCamAngle].mPitchAngle );
		mFollowCam->setGoalDistance( mCameraAngles[mCurrentCamAngle].mDist );

		Ogre::Vector3 offset = mCameraAngles[mCurrentCamAngle].mOffset;
		((Ogre::SceneNode*)mFollowCam->getLookNode())->setPosition( offset );
	}
	else
	{
		mFollowCam->setJustFollow(true);
		mFollowCam->setGoalDistance( 8.0 );
		((Ogre::SceneNode*)mFollowCam->getLookNode())->setPosition( Ogre::Vector3::ZERO );
	}
}

void Application::nextCamera()
{
	mCurrentCamAngle++;

	if (mCurrentCamAngle >= StuntPlayground::CV_SIZE)
		mCurrentCamAngle = 0;

	goCamera();
}

void Application::setCamera( CameraView ang )
{
	mCurrentCamAngle = ang;
	goCamera();
}

//////////////////////////////////////////////////////////////////////////
//		GAME SETTINGS
//////////////////////////////////////////////////////////////////////////
void Application::loadGameSettings()
{
	mSettings.load( "../../Media/settings.xml" );

}


void Application::saveGameSettings()
{
	mSettings.save( "../../Media/settings.xml" );
}


//////////////////////////////////////////////////////////////////////////////////////////
//	GUI FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////
bool Application::handleQuit( const CEGUI::EventArgs& e )
{
	mRoot->queueEndRendering();

	return true;
}

bool Application::ListboxChanged( const CEGUI::EventArgs& e )
{
	using namespace CEGUI;
	
	Listbox* lbox = (Listbox*)((const WindowEventArgs&)e).window;

	PropListItem* item = (PropListItem*)lbox->getFirstSelectedItem();
	if (!item)
		return true;

	// set the static image.
	mPrevImg->setImage( item->getImageset(), item->getImageName() );

	return true;
}


bool Application::addClicked( const CEGUI::EventArgs& e )
{
	// add the current selected prop to the world!
	using namespace CEGUI;

	Listbox* lbox = (Listbox*)WindowManager::getSingleton().getWindow("Main/Listbox");
	PropListItem* item = (PropListItem*)lbox->getFirstSelectedItem();

	if (!item)
		return true;

	// make a new prop!
	Ogre::Vector3 pos;
	Ogre::Quaternion orient;
	Ogre::String name( item->getText().c_str() );

	mDummyBody->getNewtonBody()->getPositionOrientation( pos, orient );

#ifdef _SP_DEBUG_LOG
	mDebugLog->logMessage("  addClicked: adding prop: "+name+"  bodfile:"+item->getBODFile());
#endif

	Prop* prop = PropManager::getSingleton().addProp( name, pos, orient );

	// attach the dummy to the prop!
	mDummyBody->attachBody( prop->getNewtonBody(), mMatPlacer );

	//set progstate.
	setProgState( StuntPlayground::PS_PLACING );

	showHint(Ogre::String(" move with mouse | click to drop | mouse wheel or shift to rotate | XYZ keys change axis | bksp to remove | TAB to reset rotation") );

	return true;
}

bool Application::goClicked( const CEGUI::EventArgs& e )
{
	// play sound
	SoundManager::getSingleton().playSound("EngineStart");

	// save the arena to the temp file for restoration
	saveArena( "../../Media/arenas/_sp.temp" );

	// user wants to play.  create the car, and switch game modes.
	makeCar();
	mCar->playEngine();

	// hide the dummy!
	setDummyVisible( false );

	mAABB->setVisible(false);

	// show particles!
	mDirtParticlesNode->setVisible( true );
	mSparkParticlesNode->setVisible( true );

	setCamera( StuntPlayground::CV_FOLLOW );

	// change program mode.
	setProgState( StuntPlayground::PS_CHOOSING );

	showHint( Ogre::String(" choose car with A and D keys, press W or RETURN to start driving") );

	return true;
}

bool Application::openClicked( const CEGUI::EventArgs& e )
{
	mLoadDialog->setup( LoadDialog::LM_ARENA );
	mLoadDialog->show(Ogre::String("Load Arena..."));

	showHint( Ogre::String(" open new arena- choose filename from list") );
	return true;
}

bool Application::backFromLoad( const CEGUI::EventArgs& e )
{
	mPane->setEnabled(true);
	mPane2->setEnabled(true);

	showHint( Ogre::String("") );

	if (!mLoadDialog->wasOkClicked())
		return true;

	// load the file!
	Ogre::String filename;

	CEGUI::ListboxItem* item = (CEGUI::ListboxItem*)mLoadDialog->getCEGUIListbox()->getFirstSelectedItem();

	if (!item)
	{
		mErrorDialog->show( Ogre::String("No file selected!"), false);
		return true;
	}

	filename = Ogre::String(item->getText().c_str());

	// is this a replay??
	if (mLoadDialog->getMode() == LoadDialog::LM_REPLAY)
	{
		loadReplay( "../../Media/replays/" + filename );
		return true;
	}
	
	// no, this is an arena!
	setProgState( StuntPlayground::PS_PLACING );
	
	loadArena( filename );

	showHint( Ogre::String(" Loaded Arena" + filename) );

	return true;
}


bool Application::saveClicked( const CEGUI::EventArgs& e )
{
	mSaveDialog->setSM( SaveDialog::SM_ARENA );
	mSaveDialog->show( Ogre::String("Save Arena..."));

	showHint( Ogre::String("Save current arena- enter name and press OK") );
	return true;
}

bool Application::backFromSave( const CEGUI::EventArgs& e )
{
	mPane->setEnabled(true);
	mPane2->setEnabled(true);

	showHint( Ogre::String("") );

	if (mSaveDialog->getText() == "")
	{
		mErrorDialog->show(Ogre::String("Please enter a name for the file!"), false);
		setProgState( StuntPlayground::PS_WAITING );
		return true;
	}
	
	Ogre::String filename = mSaveDialog->getText();
	if (filename.find_first_of(".") != Ogre::String.npos)
	{
		filename = filename.substr(0, filename.find_first_of("."));
	}

	if (mSaveDialog->getSM() == SaveDialog::SM_REPLAY)
	{
		filename = "../../Media/replays/" + filename + ".replay";
		saveReplay( filename );
		return true;
	}
	else
	{
		filename =  "../../Media/Arenas/" + filename + ".arena";
		saveArena( filename );
	}


	setProgState( StuntPlayground::PS_PLACING );

	showHint( Ogre::String(" Arena saved to" + filename ) );

	return true;
}

bool Application::newClicked( const CEGUI::EventArgs& e )
{

	OgreNewt::BodyIterator::getSingleton().go( Application::clearProps );

	showHint( Ogre::String(" Arena cleared.") );

	return true;
}

bool Application::playClickSound( const CEGUI::EventArgs& e )
{
	SoundManager::getSingleton().playSound( "Click1" );
	return true;
}

bool Application::settingsClicked( const CEGUI::EventArgs& e )
{
	loadGameSettings();
	mSettingDialog->show( mSettings );

	showHint( Ogre::String(" Game Settings....") );
	return true;
}

bool Application::backFromSetting( const CEGUI::EventArgs& e )
{
	mPane->setEnabled( true );
	mPane2->setEnabled(true);

	mSettings = mSettingDialog->getSettings();
	saveGameSettings();

	applySettings();

	showHint( Ogre::String("") );


	return true;
}

bool Application::loadReplayClicked( const CEGUI::EventArgs& e )
{
	// load a replay!
	mLoadDialog->setup( LoadDialog::LM_REPLAY );
	mLoadDialog->show(Ogre::String("Load a Replay...") );

	showHint( Ogre::String(" Load Replay- choose replay from the list") );
	return true;
}

bool Application::saveReplayClicked( const CEGUI::EventArgs& e )
{
	// save the current replay!
	mSaveDialog->setSM( SaveDialog::SM_REPLAY );
	mSaveDialog->show(Ogre::String("Save Replay..."));
	showHint( Ogre::String(" Save Replay- enter a name for this replay") );

	return true;
}

void Application::applySettings()
{
	mSceneMgr->setShadowTechnique( mSettings.shadowTechnique );

}

bool Application::clearScoresClicked( const CEGUI::EventArgs& e )
{
	// clear scores...
	mScores.m2WH = 0.0f;
	mScores.mDist = 0.0f;
	mScores.mFlips = 0.0f;
	mScores.mTime = 0.0f;

	// save scores...
	mScores.save( "../../Media/high.scores" );
	
	// update display...
	mGUI_JP_DISTt->setCaption( Ogre::StringConverter::toString( mScores.mDist ) );
	mGUI_JP_TIMEt->setCaption( Ogre::StringConverter::toString( mScores.mTime ) );
	mGUI_JP_FLIPSt->setCaption( Ogre::StringConverter::toString( mScores.mFlips ) );
	mGUI_JP_2WHt->setCaption( Ogre::StringConverter::toString( mScores.m2WH ) );

	return true;

}

	

bool Application::closeDialog()
{
	// user wants to close a dialog (aka pressed ESC in WAITING MODE).
	// return TRUE is a dialog was closed, FALSE if no dialogs were open in the first place.

	if (mErrorDialog->getCEGUIWindow()->isVisible())
	{
		mErrorDialog->hide();
		return true;
	}

	if (mLoadDialog->getCEGUIWindow()->isVisible())
	{
		mLoadDialog->hide();
		return true;
	}

	if (mSaveDialog->getCEGUIWindow()->isVisible())
	{
		mSaveDialog->hide();
		return true;
	}

	if (mSettingDialog->getCEGUIWindow()->isVisible())
	{
		mSettingDialog->hide();
		return true;
	}

	// otherwise just return false!
	return false;
}

	

//////////////////////////////////////////////////////////////////////////////////////////
//	CREATE FRAME LISTENERS
//////////////////////////////////////////////////////////////////////////////////////////
void Application::createFrameListener()
{
	mFrameListener = new MainFrameListener( mWindow, mCamera, mSceneMgr, mFollowCam, mWorld, 90 );
	mRoot->addFrameListener( mFrameListener );

	mCEGUIListener = new StuntPlayground::CEGUIFrameListener(mWindow, mCamera, mPane, mPane2);
	mRoot->addFrameListener( (Ogre::FrameListener*)mCEGUIListener );

}

//////////////////////////////////////////////////////////////////////////////////////////
//	NEWTON CALLBACKS
//////////////////////////////////////////////////////////////////////////////////////////

void Application::dummyForceCallback( OgreNewt::Body* me )
{
	StuntPlayground::DummyBody* dummy = (DummyBody*)me->getUserData();

	// perform a basic spring force for movement.
	Ogre::Vector3 curpos, goalpos;				// current position and orientation of the rigid body.
	Ogre::Quaternion curorient, goalorient;		// goal position and orientation of the rigid body.

	// fill the variables with the actual data.
	me->getPositionOrientation( curpos, curorient );
	dummy->getGoalPositionOrientation( goalpos, goalorient );

	
	Ogre::Matrix3 current, goal;

	curorient.ToRotationMatrix( current );
	goalorient.ToRotationMatrix( goal );

	Ogre::Vector3 togoal, dir;

	//get the goal position of each axis element.
	std::vector<Ogre::Vector3> goalaxis;
	std::vector<Ogre::Vector3> curaxis;

	for (int i=0;i<3;i++)
	{
		Ogre::Vector3 temp;
		temp = goal.GetColumn(i) * 5.0;
		goalaxis.push_back(temp);

		temp = current.GetColumn(i) * 3.0;
		curaxis.push_back(temp);
	}

	// velocity and omega of the rigid body.
	Ogre::Vector3 omega = me->getOmega();
	Ogre::Vector3 velocity = me->getVelocity();


	Ogre::Vector3 from, to;
	Ogre::Vector3 force;

	// debug lines!
#ifdef _SP_DEBUG
	Application::getSingleton().mDebugNode->detachAllObjects();
	if (Application::getSingleton().mDebugLines)
		delete Application::getSingleton().mDebugLines;
	Application::getSingleton().mDebugLines = NULL;

	Application::getSingleton().mDebugLines = new Line3D();
#endif	// _SP_DEBUG

	for (int i=0; i<3; i++)
	{
		// find the instantaneous velocity of the point on the body.
		Ogre::Vector3 instvel;
		instvel = (velocity) + (omega.crossProduct(curaxis[i]));

		// per-axis forces.
		from = (curaxis[i]) + curpos;
		to = (goalaxis[i]) + goalpos + (goalaxis[(i==2)?0:i+1]*0.2);
		togoal = (to-from);
		dir = togoal.normalisedCopy();

#ifdef _SP_DEBUG	
		Application::getSingleton().mDebugLines->addPoint(from);
		Application::getSingleton().mDebugLines->addPoint(to);
#endif	// _SP_DEBUG

		force = (((togoal.length()) * dummy->getSpringConst()) * dir) - (instvel * dummy->getSpringDamp());
		me->addGlobalForce( force, from );

		from = (curaxis[i]) + curpos;
		to = (goalaxis[i]) + goalpos - (goalaxis[(i==2)?0:i+1]*0.2);
		togoal = (to-from);
		dir = togoal.normalisedCopy();

#ifdef _SP_DEBUG
		Application::getSingleton().mDebugLines->addPoint(from);
		Application::getSingleton().mDebugLines->addPoint(to);
#endif // _SP_DEBUG

		force = (((togoal.length()) * dummy->getSpringConst()) * dir) - (instvel * dummy->getSpringDamp());
		me->addGlobalForce( force, from );


		//////////////////////////////////////////////////////
		from = (-curaxis[i]) + curpos;
		to = (-goalaxis[i]) + goalpos + (goalaxis[(i==2)?0:i+1]*0.2);
		togoal = (to-from);
		dir = togoal.normalisedCopy();

#ifdef _SP_DEBUG
		Application::getSingleton().mDebugLines->addPoint(from);
		Application::getSingleton().mDebugLines->addPoint(to);
#endif	// _SP_DEBUG

		force = (((togoal.length()) * dummy->getSpringConst()) * dir) - (instvel * dummy->getSpringDamp());
		me->addGlobalForce( force, from );

		from = (-curaxis[i]) + curpos;
		to = (-goalaxis[i]) + goalpos - (goalaxis[(i==2)?0:i+1]*0.2);
		togoal = (to-from);
		dir = togoal.normalisedCopy();

#ifdef _SP_DEBUG
		Application::getSingleton().mDebugLines->addPoint(from);
		Application::getSingleton().mDebugLines->addPoint(to);
#endif	// _SP_DEBUG

		force = (((togoal.length()) * dummy->getSpringConst()) * dir) - (instvel * dummy->getSpringDamp());
		me->addGlobalForce( force, from );
		
	}

#ifdef _SP_DEBUG
	Application::getSingleton().mDebugLines->drawLines();
	Application::getSingleton().mDebugNode->attachObject( Application::getSingleton().mDebugLines );
#endif	// _SP_DEBUG

	// finally, add a damper for torque!
	dir = omega.normalisedCopy();
	Ogre::Real invmass;
	Ogre::Vector3 invinertia;
	me->getInvMass( invmass, invinertia );

	me->addTorque( -dir * (omega.length()*200) );
	
	// finally, also update the attached body's position (if exists)
	OgreNewt::Body* attached = dummy->getAttached();

	if (attached)
	{
		attached->setPositionOrientation(curpos,curorient);
	}
}


void Application::clearProps( OgreNewt::Body* body )
{

	if (body->getType() == StuntPlayground::BT_PROP)
	{
		Prop* prp = (Prop*)body->getUserData();

		PropManager::getSingleton().removeProp( prp );
	}
}

void Application::saveProps( OgreNewt::Body* body )
{

	if (body->getType() == StuntPlayground::BT_PROP)
	{
		Prop* prp = (Prop*)body->getUserData();

		TiXmlElement prop( "Prop" );
		prop.SetAttribute("name", prp->getName() );

		Ogre::Vector3 rot, pos;
		Ogre::Matrix3 rotmat;
		Ogre::Quaternion orient;
		Ogre::Radian x,y,z;

		body->getPositionOrientation( pos, orient );
		orient.ToRotationMatrix( rotmat );
		rotmat.ToEulerAnglesXYZ( x,y,z );
		rot = Ogre::Vector3(x.valueDegrees(),y.valueDegrees(),z.valueDegrees());

		prop.SetAttribute("position", (std::string)Ogre::StringConverter::toString(pos) );
		prop.SetAttribute("rotation", (std::string)Ogre::StringConverter::toString(rot) );

		Application::getSingleton().mXmlElement->InsertEndChild( prop );
	}
}

void Application::updateProps( OgreNewt::Body* body )
{
	if (body->getType() == StuntPlayground::BT_PROP)
	{
		Ogre::Vector3 pos;
		Ogre::Quaternion orient;

		body->getPositionOrientation( pos, orient );
		body->getOgreNode()->setPosition( pos );
		body->getOgreNode()->setOrientation( orient );
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//		SAVE / LOAD / NEW FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////
void Application::loadArena( Ogre::String filename )
{
	// load an arena!
	OgreNewt::BodyIterator::getSingleton().go( Application::clearProps );

	TiXmlDocument arenafile;

	arenafile.LoadFile( "../../Media/Arenas/"+filename );

	TiXmlElement* arena = arenafile.RootElement();

	if (!arena)
	{
		mErrorDialog->show( Ogre::String("Error in arena file!"), false );
		setProgState( StuntPlayground::PS_WAITING );
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage("ERROR!  invalid Arena file "+filename+": cannot find root.");
#endif	// _SP_DEBUG_LOG
		return;
	}

	TiXmlElement* prop = arena->FirstChildElement("Prop");

	if (!prop)
	{
		//mErrorDialog->show( Ogre::String("No props in this arena file!"), false );
		setProgState( StuntPlayground::PS_WAITING );
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage("ERROR!  no props found in arena file "+filename+": cannot find any props!" );
#endif	// _SP_DEBUG_LOG
		return;
	}

	while (prop)
	{
		Ogre::Quaternion orient;
		Ogre::Vector3 rot, pos;
		Ogre::Matrix3 rotmat;

		Ogre::String name;
		
		name = (Ogre::String)prop->Attribute("name");
		
		
		rot = Ogre::StringConverter::parseVector3( (Ogre::String)prop->Attribute("rotation") );
		pos = Ogre::StringConverter::parseVector3( (Ogre::String)prop->Attribute("position") );

		rotmat.FromEulerAnglesXYZ( Ogre::Degree(rot.x), Ogre::Degree(rot.y), Ogre::Degree(rot.z) );
		orient.FromRotationMatrix( rotmat );

		Prop* prp = PropManager::getSingleton().addProp(name, pos, orient );
		if (!prp)
		{
			break;
		}
		prp->getNewtonBody()->setStandardForceCallback();

		// get next prop.
		prop = prop->NextSiblingElement("Prop");
	}
    

}

void Application::saveArena( Ogre::String filename )
{
	TiXmlDocument arenafile;

	TiXmlDeclaration dec;
	arenafile.InsertEndChild( dec );

	TiXmlComment com;
	com.SetValue( "Stunt Playground Arena file" );

	arenafile.InsertEndChild( com );

	TiXmlElement arena("Arena");

	mXmlElement = &arena;

	// add props here!
	OgreNewt::BodyIterator::getSingleton().go( Application::saveProps );

	arenafile.InsertEndChild( arena );

	arenafile.SaveFile((std::string)filename );

}

void Application::restoreArena()
{
	if (mSettings.restoreArena)
		loadArena( "../../Media/arenas/_sp.temp" );
}


void Application::setDefaultCameraAngles()
{
	// open the camera angle .xml file.
	TiXmlDocument file;

	file.LoadFile("../../Media/cameras.xml");

	TiXmlElement* root = file.RootElement();

	if (!root)
	{
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage(" ERROR! Cannot find Camera angles!!!");
#endif	// _SP_DEBUG_LOG

		mErrorDialog->show(Ogre::String("Error loading Camera angles!!"), false );
		return;
	}

	// loop through camera angles.
	TiXmlElement* angle = root->FirstChildElement("Angle");

	if (!angle)
	{
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage(" ERROR! No Angles found in camera.xml file!!!");
#endif	// _SP_DEBUG_LOG

		mErrorDialog->show(Ogre::String("Error loading Camera angles!!"), false );
		return;
	}

	while (angle)
	{
		Ogre::String name = angle->Attribute("name");
		Ogre::Real yaw = Ogre::StringConverter::parseReal(angle->Attribute("yaw"));
		Ogre::Real pitch = Ogre::StringConverter::parseReal(angle->Attribute("pitch"));
		Ogre::Real dist = Ogre::StringConverter::parseReal(angle->Attribute("dist"));
		Ogre::Vector3 offset = Ogre::StringConverter::parseVector3(angle->Attribute("lookoff"));

		int off = 0;

		if (name == "FOLLOW") { off = StuntPlayground::CV_FOLLOW; }
		if (name == "FRONT") { off = StuntPlayground::CV_FRONT; }
		if (name == "LDOOR") { off = StuntPlayground::CV_LDOOR; }
		if (name == "RDOOR") { off = StuntPlayground::CV_RDOOR; }
		if (name == "LSIDE") { off = StuntPlayground::CV_LSIDE; }
		if (name == "RSIDE") { off = StuntPlayground::CV_RSIDE; }
		if (name == "WIDE_FOLLOW") { off = StuntPlayground::CV_WIDE_FOLLOW; }
		if (name == "WIDE_LSIDE") { off = StuntPlayground::CV_WIDE_LSIDE; }
		if (name == "WIDE_RSIDE") { off = StuntPlayground::CV_WIDE_RSIDE; }

		mCameraAngles[off].setup( Ogre::Degree(yaw), Ogre::Degree(pitch), dist, offset );

		// get next angle.
		angle = angle->NextSiblingElement("Angle");
	}
}


/////// REPALAYS ////////
void Application::saveReplay( Ogre::String filename )
{
	// save out the existing replay to an xml file.
	TiXmlDocument doc;

	TiXmlComment com;
	com.SetValue("Stunt Playground Replay File");
	doc.InsertEndChild( com );

	TiXmlElement replay("replay");

	// first export the vehicle.
	if (mCar)
	{
		TiXmlElement vehicle("vehicle");
		vehicle.SetAttribute("filename", mCar->getFilename() );

		// chassis
		TiXmlElement chassis = mCar->mRecObject->getXMLElement();
		chassis.SetValue( "chassis" );

		vehicle.InsertEndChild( chassis );

		// tires.
		for (Car::MyTire* tire = (Car::MyTire*)mCar->getFirstTire(); tire; tire = (Car::MyTire*)mCar->getNextTire(tire))
		{
			TiXmlElement tire_elem = tire->mRecObj->getXMLElement();
			tire_elem.SetValue( "tire" );
			vehicle.InsertEndChild( tire_elem );
		}

		replay.InsertEndChild( vehicle );
	}

	// props.
	PropManager& pm = PropManager::getSingleton();

	TiXmlElement props("props");

	for (PropManager::PropListIterator it = pm.getPropListIteratorBegin(); it!= pm.getPropListIteratorEnd(); it++)
	{
		TiXmlElement prop("prop");
		prop.SetAttribute("name", (*it)->getName());
		
		TiXmlElement key = (*it)->mRecObj->getXMLElement();

		prop.InsertEndChild( key );

		props.InsertEndChild( prop );
	}

	replay.InsertEndChild( props );
		

	doc.InsertEndChild( replay );
		
	

	doc.SaveFile( filename );
}


void Application::loadReplay( Ogre::String filename )
{
	// load an existing replay saved to an xml file!
	if (!mLoadDialog->wasOkClicked())
		return;

	if (getProgState() == StuntPlayground::PS_REPLAYING)
	{
		WorldRecorder::getSingleton().stopPlaying();
	}

	// clear any existing replays
	WorldRecorder::getSingleton().removeAllRecordableObjects();

	// first, clear any existing arena!
	OgreNewt::BodyIterator::getSingleton().go( Application::clearProps );
	

	// remove a vehicle if we have one.
	if (mCar)
	{
		removeReflectionCameras();
		delete mCar;
		mCar = NULL;
	}

	// okay, now start loading the replay!
	TiXmlDocument doc;
	doc.LoadFile( filename );

	TiXmlElement* root = doc.RootElement();

	if (!root)
	{
		//error!
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage(" -R- loadReplay: cannot find root element in xml file: "+filename);
#endif	// _SP_DEBUG_LOG
		return;
	}

	// get the vehicle.
	TiXmlElement* vehicle = root->FirstChildElement("vehicle");

	if (!vehicle)
	{
		//error!
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage(" -R- loadReplay: cannot find vehicle element in xml file: "+filename);
#endif	// _SP_DEBUG_LOG
		return;
	}

	Ogre::String vehiclefile = vehicle->Attribute("filename");

	// load the vehicle
#ifdef _SP_DEBUG_LOG
	mDebugLog->logMessage(" -R- loadReplay:: about to load vehicle "+vehiclefile );
#endif	// _SP_DEBUG_LOG

	mCar = new Car( mWorld, mSceneMgr->getRootSceneNode(), mSceneMgr, vehiclefile , mMatCar );
	
	if (!mCar->getChassisBody())
	{
		return;
	}

	mCar->setManualTransmission( mSettings.useManualTransmission );

	mCar->mRecObject = WorldRecorder::getSingleton().addRecordableObject( mCar->getChassisBody()->getOgreNode(), 0.1 );
	mCar->mRecObject->fromXMLElement( vehicle->FirstChildElement("chassis") );

	Ogre::Quaternion carorient;
	Ogre::Matrix3 carrot;
	carrot.FromEulerAnglesXYZ(Ogre::Degree(0), Ogre::Degree(90), Ogre::Degree(0));
	carorient.FromRotationMatrix( carrot );
	mCar->getChassisBody()->setPositionOrientation( Ogre::Vector3(-100,-2,45), carorient );

	// REFLECTION CAMERAS
	applyReflectionCameras();


#ifdef _SP_DEBUG_LOG
	mDebugLog->logMessage(" -R- loadReplay:: vehicle loaded! ");
#endif // _SP_DEBUG_LOG

	//now set the rec objects for the tires.
	TiXmlElement* tire_elem = vehicle->FirstChildElement("tire");

	for (Car::MyTire* tire = (Car::MyTire*)mCar->getFirstTire(); tire; tire = (Car::MyTire*)mCar->getNextTire(tire))
	{
		tire->mRecObj = WorldRecorder::getSingleton().addRecordableObject( tire->getOgreNode(), 0.1 );
		tire->mRecObj->fromXMLElement( tire_elem );

		//next element.
		tire_elem = tire_elem->NextSiblingElement("tire");
	}


	// NOW load props!!!
	TiXmlElement* props = root->FirstChildElement("props");

	if (!props)
	{
		//error!
#ifdef _SP_DEBUG_LOG
		mDebugLog->logMessage(" -R- loadReplay:: cannot find props element in xml file "+filename);
#endif	// _SP_DEBUG_LOG
	}
	else
	{

		for (TiXmlElement* prop = props->FirstChildElement("prop"); prop; prop = prop->NextSiblingElement("prop"))
		{
			Ogre::String propname = prop->Attribute("name");

			TiXmlElement* recobj = prop->FirstChildElement("recordable_object");

			TiXmlElement* firstkey = recobj->FirstChildElement("keyframe");

			// add the prop.
			Ogre::Vector3 pos = Ogre::StringConverter::parseVector3( firstkey->Attribute("position") );
			Ogre::Quaternion orient = Ogre::StringConverter::parseQuaternion( firstkey->Attribute("orient") );

#ifdef _SP_DEBUG_LOG
			mDebugLog->logMessage(" -R- loadReplay: adding prop: "+propname );
#endif	// _SP_DEBUG_LOG

			Prop* prp = PropManager::getSingleton().addProp( propname, pos, orient );
			prp->getNewtonBody()->setStandardForceCallback();

			// recordable object.
			prp->mRecObj = WorldRecorder::getSingleton().addRecordableObject( prp->getNewtonBody()->getOgreNode(), 0.1 );
			prp->mRecObj->fromXMLElement( recobj );
		}
	}



	// save the arena to the temp file for restoration
	saveArena( "../../Media/arenas/_sp.temp" );


	// finally, set ourselves in replay mode!!
	setProgState( StuntPlayground::PS_REPLAYING );
	WorldRecorder::getSingleton().startPlaying();
	enableSaveReplay();

	hideGUI();

	mReplayControls->setMode( StuntPlayground::ReplayControls::RM_PAUSE );

	((MainFrameListener*)mFrameListener)->startReplayPlayback(0.0);

	mDirtParticlesNode->setVisible(false);
	mSparkParticlesNode->setVisible(false);

	mFollowCam->setJustFollow( false );
	mFollowCam->setGoalNode( mCar->getChassisBody()->getOgreNode() );
	mFollowCamOldDist = mFollowCam->getGoalDistance();
	mCamera->setPosition( Ogre::Vector3(100,100,-45) );
	mFollowCam->setLookNode( mCar->getLookNode() );
	
	goCamera();

	setDummyVisible( false );
	

	mAABB->setVisible(false);

	// reflection cameras
	applyReflectionCameras();

	mGUI_CAMERA->show();
	mGUI_REC->hide();

}

void Application::enableSaveReplay()
{
	mButSaveReplay->getButton()->setEnabled(true);
}

void Application::disableSaveReplay()
{
	mButSaveReplay->getButton()->setEnabled(false);
}


//////////////////////////////////////////////////////////////////////////////////////////
//		REFLECTION CAMERA FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////
void Application::makeReflectionCameras()
{
	

	////////////////////////////////
	
	if (!mReflectionListener)
		mReflectionListener = new ReflectionRenderTarget( mSceneMgr, mGUI );


	Ogre::RenderSystem* rend = mRoot->getRenderSystem();
	Ogre::TextureManager& tm = Ogre::TextureManager::getSingleton();

	ReflectCam temp;
	Ogre::Viewport* vp;

	// make the base render texture, to which we will render the 6 cameras.
	Ogre::TexturePtr cubetex = tm.TextureManager::createManual("reflection_tex", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_CUBE_MAP, 
																	256, 256, 0, PF_R8G8B8, TU_RENDERTARGET);


	//+X (0), -X (1), +Y (2), -Y (3), +Z (4), -Z (5)

	for (int face=0; face<6;face++)
	{
		
		temp.mCAM = mSceneMgr->createCamera("Reflect_"+Ogre::StringConverter::toString(face));
		temp.mCAM->setAspectRatio(1.0f);
		temp.mCAM->setFOVy( Ogre::Degree(90) );
		temp.mCAM->setNearClipDistance( 0.1 );

		temp.mRT = cubetex->getBuffer(face)->getRenderTarget();
		vp = temp.mRT->addViewport(temp.mCAM);
		temp.mRT->setAutoUpdated(false);
		temp.mRT->addListener( mReflectionListener );

		temp.mCAM->setPosition(Ogre::Vector3::ZERO);

		Ogre::Vector3 lookAt(0,0,0);
		Ogre::Vector3 up(0,0,0);
		Ogre::Vector3 right(0,0,0);

		switch(face)
		{
		case 0 : 
			lookAt.x = -1; up.y = 1; right.z = 1; break;
        case 1: //-X 
            lookAt.x = 1; up.y = 1; right.z = -1; break;
        case 2: //+Y
            lookAt.y = -1; up.z = 1; right.x = 1; break;
        case 3: //-Y
            lookAt.y = 1; up.z = -1; right.x = 1; break;
        case 4: //+Z
            lookAt.z = 1; up.y = 1; right.x = -1; break;
        case 5: //-Z
            lookAt.z = -1; up.y = 1; right.x = -1; break;
        }

		Ogre::Quaternion orient( right, up, lookAt );
	
		temp.mCAM->setOrientation( orient );

		mReflectCams.push_back(temp);

	}

	
#ifdef _SP_DEBUG_LOG
	/*
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName("DEBUG/PANEL1");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("reflect_fr");
	
	mat = Ogre::MaterialManager::getSingleton().getByName("DEBUG/PANEL2");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("reflect_bk.");

	mat = Ogre::MaterialManager::getSingleton().getByName("DEBUG/PANEL3");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("reflect_lf.");

	mat = Ogre::MaterialManager::getSingleton().getByName("DEBUG/PANEL4");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("reflect_rt.");

	mat = Ogre::MaterialManager::getSingleton().getByName("DEBUG/PANEL5");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("reflect_up.");

	mat = Ogre::MaterialManager::getSingleton().getByName("DEBUG/PANEL6");
	mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("reflect_dn.");
	*/
#endif	// _SP_DEBUG_LOG


}

void Application::killReflectionCameras()
{
	//return;

	
	while (mReflectCams.size() > 0)
	{
		ReflectCam& it = mReflectCams.back();

		mSceneMgr->destroyCamera( it.mCAM->getName() );
		mRoot->getRenderSystem()->destroyRenderTexture( it.mRT->getName() );

		mReflectCams.pop_back();
	}

	if (mReflectionListener)
	{
		delete mReflectionListener;
		mReflectionListener = NULL;
	}
	


}

void Application::reflectionsON()
{
	applyReflectionCameras();
}

void Application::reflectionsOFF()
{
	removeReflectionCameras();
}

void Application::applyReflectionCameras()
{
	if (!mSettings.useRealtimeReflections)
	{
		mReflectCamEnabled = false;
		return;
	}

	if (mReflectCamEnabled == true)
		return;
	
	if (mCar)
	{
		
		Ogre::MeshPtr mesh = ((Entity*)mCar->getChassisBody()->getOgreNode()->getAttachedObject(0))->getMesh();

		for (unsigned i=0; i<mesh->getNumSubMeshes(); i++)
		{
			Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName( mesh->getSubMesh(i)->getMaterialName() );

			if (mat.isNull())
				continue;
			
			if (mat->getTechnique(0)->getPass(0)->getNumTextureUnitStates() >= 2)
			{
				Ogre::TextureUnitState* tex = mat->getTechnique(0)->getPass(0)->getTextureUnitState(1);
				
				// save the old texture.
				Ogre::String texname = tex->getTextureName();
				Ogre::String ext = "";

				size_t tounder = texname.find_first_of("_");
				size_t todot = texname.find_first_of(".");

				if (todot != Ogre::String::npos)
				{
					ext = texname.substr( todot );
					texname = texname.substr( 0, todot );
				}

				if (tounder != Ogre::String::npos)
					texname = texname.substr(0, tounder);
				
				texname = texname + ext;
					

				mOldTextures.push_back( texname );

				tex->setTextureName( "reflection_tex" );
				tex->setEnvironmentMap( true, Ogre::TextureUnitState::ENV_REFLECTION );
				
			}
		}

		

		// now get the nodes to hide!
		mReflectionListener->addHideNode( mCar->getChassisBody()->getOgreNode() );

		//loop through tires.
		for(Car::MyTire* tire = (Car::MyTire*)mCar->getFirstTire(); tire; tire = (Car::MyTire*)mCar->getNextTire(tire))
		{
			mReflectionListener->addHideNode( tire->getOgreNode() );
		}

		mReflectionListener->addHideParticle( mDirtParticles );
		
		mReflectionListener->addHideParticle( mSparkParticles );

		mReflectCamEnabled = true;
	}
	
}

void Application::removeReflectionCameras()
{
	if (mReflectCamEnabled == false)
		return;
	
	if (mCar)
	{
		Ogre::MeshPtr mesh = ((Entity*)mCar->getChassisBody()->getOgreNode()->getAttachedObject(0))->getMesh();

		Ogre::MaterialPtr mat;

		
		for (unsigned int i=0; i<mesh->getNumSubMeshes(); i++)
		{
			mat = Ogre::MaterialManager::getSingleton().getByName( mesh->getSubMesh(i)->getMaterialName() );

			if (mat.isNull())
				continue;

			if (mat->getTechnique(0)->getPass(0)->getNumTextureUnitStates() < 2)
				continue;
			
			Ogre::TextureUnitState* tex = mat->getTechnique(0)->getPass(0)->getTextureUnitState(1);

			if (tex)
			{
				tex->TextureUnitState::setCubicTextureName( mOldTextures.back() );
				tex->setTextureScale( 1.0, 1.0 );
				mOldTextures.pop_back();
			}
		}

		mReflectionListener->clearHideNodes();

		mReflectCamEnabled = false;
	}
	
}


void Application::updateReflection()
{
	if (!mSettings.useRealtimeReflections)
		return;

	
	mCurrentReflectCam++;
	if (mCurrentReflectCam > 5) { mCurrentReflectCam = 0; }

	ReflectCam& now = mReflectCams.at(mCurrentReflectCam);


	if (mCar)
	{
		now.mCAM->setPosition( mCar->getChassisBody()->getOgreNode()->getWorldPosition() );

		now.mRT->update();
	}
	

}


void Application::showStandardHint()
{
	showHint( Ogre::String(" move cursor with mouse | rotate with mouse wheel or shift | hold CTRL to change camera angle, zoom with mouse wheel") );
}


void Application::showErrorMessage( Ogre::String msg, bool showCancel )
{
	mErrorDialog->show( msg, showCancel );
}

//////////////////////////////////////////////////////////////////////////////////////////
//	CLEANUP FUNCTIONS
//////////////////////////////////////////////////////////////////////////////////////////
void Application::destroyScene()
{

	// save high scores.
	mScores.save( "../../Media/high.scores" );
	
	// destroy items in listbox
	using namespace CEGUI;

	Listbox* lbox = (Listbox*)WindowManager::getSingleton().getWindow((utf8*)"Main/Listbox");

	while (lbox->getItemCount() > 0)
	{
		PropListItem* del = (PropListItem*)lbox->getListboxItemFromIndex(0);
		lbox->removeItem( del );
		delete del;
	}

	// destroy my follow camera
	delete mFollowCam;

	// reflection camera listener
	if (mReflectionListener)
		delete mReflectionListener;

	// destroy buttons
	delete mButAdd;
	delete mButGo;
	delete mButQuit;
	delete mButOpen;
	delete mButSave;
	delete mButNew;
	delete mButSettings;
	delete mButLoadReplay;
	delete mButSaveReplay;

	// destroy dialog boxes
	delete mLoadDialog;
	delete mSaveDialog;
	delete mErrorDialog;
	delete mSettingDialog;
	delete mReplayControls;

	// destroy newton materials
	destroyMaterials();

	// destroy mAABB object
	delete mAABB;

	// destroy dummy body
	delete mDummyBody;

	// debug lines
#ifdef _SP_DEBUG
	if (mDebugLines)
		delete mDebugLines;
#endif	// _SP_DEBUG

	// destroy frame listeners
	if (mFrameListener)
	{
		mRoot->removeFrameListener( mFrameListener );
		delete mFrameListener;
		mFrameListener = NULL;
	}

	if (mCEGUIListener)
	{
		mRoot->removeFrameListener( mCEGUIListener );
		delete mCEGUIListener;
		mCEGUIListener = NULL;
	}


	CEGUI::System* sys = CEGUI::System::getSingletonPtr();
	delete sys;

	// CEGUI Cleanup
	delete mGUIRenderer;
}

void Application::destroyMaterials()
{
	// mateial pairs
	if (mMatPairArenaVsCar)
	{
		delete mMatPairArenaVsCar;
		mMatPairArenaVsCar = NULL;
	}

	if (mMatPairCarVsProp)
	{
		delete mMatPairCarVsProp;
		mMatPairCarVsProp = NULL;
	}

	if (mMatCar)
	{
		delete mMatCar;
		mMatCar = NULL;
	}

	if (mMatProp)
	{
		delete mMatProp;
		mMatProp = NULL;
	}

	if (mMatDummy)
	{
		delete mMatDummy;
		mMatDummy = NULL;
	}

	if (mMatPlacer)
	{
		delete mMatPlacer;
		mMatPlacer = NULL;
	}

	// callbacks
	if (mCarArenaCallback)
	{
		delete mCarArenaCallback;
		mCarArenaCallback = NULL;
	}

	if (mDummyCallback)
	{
		delete mDummyCallback;
		mDummyCallback = NULL;
	}

	if (mPropCallback)
	{
		delete mPropCallback;
		mPropCallback = NULL;
	}

	if (mCarPropCallback)
	{
		delete mCarPropCallback;
		mCarPropCallback = NULL;
	}

}




}	// end NAMESPACE StuntPlayground
