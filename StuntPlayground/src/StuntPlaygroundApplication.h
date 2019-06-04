/*
	Stunt Playground
	
	StuntPlaygroundApplication
*/

#ifndef _STUNTPLAYGROUNDAPPLICATION_
#define _STUNTPLAYGROUNDAPPLICATION_

#include "Prereq.h"

#include "CEGUI.h"
#include "OgreCEGUIRenderer.h"
#include "OgreCEGUIResourceProvider.h"
#include "WalaberOgreApplication.h"
#include ".\CEGUIFrameListener.h"
#include "OgreNewt.h"
#include ".\StuntPlayground.h"
#include "CollisionCallbacks.h"
#include ".\Prop.h"
#include ".\FollowCamera.h"
#include ".\StandardButton.h"
#include ".\DummyBody.h"
#include ".\vehicle.h"
#include ".\BodyIterators.h"
#include ".\dialogs.h"
#include ".\AABB.h"
#include "Line3D.h"
#include "settings.h"
#include "ReflectionRenderTarget.h"
#include "HighScore.h"

#include <tinyxml.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////

namespace StuntPlayground
{

class Application :
	public WalaberOgreApplication
{
public:
	// class for camera angles
	class CameraAngle
	{
	public:
		Ogre::Radian mYawAngle;
		Ogre::Radian mPitchAngle;
		Ogre::Real mDist;
		Ogre::Vector3 mOffset;

		void setup( Ogre::Radian yaw, Ogre::Radian pitch, Ogre::Real dist, Ogre::Vector3 offset ) { mYawAngle = yaw; mPitchAngle = pitch; mDist = dist; mOffset = offset; }
	};

	// class for reflection cameras
	class ReflectCam
	{
	public:
		Ogre::Camera* mCAM;
		Ogre::RenderTarget* mRT;
	};


	static Application& getSingleton();
	static Application* getSingletonPtr();

	CEGUI::TabPane* getPane() { return mPane; }

	ReplayControls* getReplayControls() { return mReplayControls; }

	OgreNewt::World* getOgreNewtWorld() { return mWorld; }

	Application(void);
	~Application(void);

	void makeDummy();
	void setDummyCallback();
	StuntPlayground::DummyBody* getDummy() { return mDummyBody; }
	StuntPlayground::MyAABB* getAABB() { return mAABB; }

	StuntPlayground::ProgState getProgState() { return mState; }
	void setProgState( StuntPlayground::ProgState state ) { mState = state; }

	void nextCar();
	void prevCar();

	void removeCar();

	void goCamera();
	void nextCamera();
	void setCamera( CameraView ang );
	unsigned int getCameraView() { return mCurrentCamAngle; }

	Ogre::SceneManager* getSceneMgr() { return mSceneMgr; }

	Ogre::SceneNode* getDirtParticlesNode() { return mDirtParticlesNode; }
	Ogre::ParticleSystem* getDirtParticles() { return mDirtParticles; }

	Ogre::SceneNode* getSparkParticlesNode() { return mSparkParticlesNode; }
	Ogre::ParticleSystem* getSparkParticles() { return mSparkParticles; }

	void showLOGO() { mLOGO->show(); }
	void hideLOGO() { mLOGO->hide(); }

	void showGUI();
	void hideGUI();
	void showGUI_REC();
	void hideGUI_REC();
	void updateOldMessages( Ogre::String text );

	void possibleJumpRecord( Ogre::Real length, Ogre::Real time, Ogre::Real flips );
	void possible2WheelRecord( Ogre::Real time );

	void updateReflection();

	void disableSaveReplay();
	void enableSaveReplay();

	void reflectionsON();
	void reflectionsOFF();

	void setDummyVisible( bool setting );

	void restoreArena();
	
	bool closeDialog();

	bool addClicked( const CEGUI::EventArgs& e );

	void showHint(Ogre::String& message ) { mHint->setText( (std::string)message ); }
	void showStandardHint();

	// show an error message
	void showErrorMessage( Ogre::String msg, bool showCancel );


	static void _cdecl updateProps( OgreNewt::Body* body );
	

#ifdef _SP_DEBUG
	Line3D* mDebugLines;
	Ogre::SceneNode* mDebugNode;
#endif	// _SP_DEBUG

#ifdef _SP_DEBUG_LOG
	Ogre::Log* mDebugLog;
#endif	// _SP_DEBUG_LOG

	// various Newton materials;
	const OgreNewt::MaterialID* mMatArena;
	const OgreNewt::MaterialID* mMatCar;
	const OgreNewt::MaterialID* mMatProp;
	const OgreNewt::MaterialID* mMatDummy;
	const OgreNewt::MaterialID* mMatPlacer;
	
	StuntPlayground::Car* mCar;

	Ogre::Real mFollowCamOldDist;

	// XML node (for saving props)
	TiXmlElement* mXmlElement;

protected:
	void createFrameListener();
	void createScene();
	void createMaterials();

	void destroyScene();
	void destroyMaterials();

	void makeGUI();
	void makeOnScreenGUI();
	void setupGUI();

	// CEGUI functions!
	bool handleQuit( const CEGUI::EventArgs& e );
	bool ListboxChanged( const CEGUI::EventArgs& e );
	bool goClicked( const CEGUI::EventArgs& e );
	bool openClicked( const CEGUI::EventArgs& e );
	bool saveClicked( const CEGUI::EventArgs& e );
	bool newClicked( const CEGUI::EventArgs& e );
	bool settingsClicked( const CEGUI::EventArgs& e );
	
	bool loadReplayClicked( const CEGUI::EventArgs& e );
	bool saveReplayClicked( const CEGUI::EventArgs& e );

	bool clearScoresClicked( const CEGUI::EventArgs& e );

	bool disablePane( const CEGUI::EventArgs& e ) { mPane->setEnabled(false); mPane2->setEnabled(false); return true; }
	bool enablePane( const CEGUI::EventArgs& e ) { mPane->setEnabled(true); mPane2->setEnabled(true); return true; }
	bool backFromLoad( const CEGUI::EventArgs& e );
	bool backFromSave( const CEGUI::EventArgs& e );
	bool backFromSetting( const CEGUI::EventArgs& e );

	bool playClickSound( const CEGUI::EventArgs& e );

	// car choosing functions...
	void makeCar();

	// save / load the arena...
	void saveArena( Ogre::String filename );
	void loadArena( Ogre::String filename );

	// save / load replays
	void saveReplay( Ogre::String filename );
	void loadReplay( Ogre::String filename );

	//  load the default camera angles
	void setDefaultCameraAngles();

	// setup / init / update reflection cameras
	void makeReflectionCameras();
	void killReflectionCameras();

	void applyReflectionCameras();
	void removeReflectionCameras();

	// GameSettings!
	void loadGameSettings();
	void saveGameSettings();
	void applySettings();

	// newton callbacks...
	static void dummyForceCallback( OgreNewt::Body* me );
	static void clearProps( OgreNewt::Body* body );
	static void saveProps( OgreNewt::Body* body );

private:
	// Newton World!!
	OgreNewt::World* mWorld;

	// CEGUI Ogre Renderer
	CEGUI::OgreCEGUIRenderer* mGUIRenderer;

	// CEGUI Frame Listener
	CEGUIFrameListener* mCEGUIListener;


	// main logo image
	CEGUI::StaticImage* mLOGO;

	// UI parts
	Ogre::Overlay* mGUI;
	Ogre::OverlayElement* mGUI_CAMERA;
	Ogre::OverlayElement* mGUI_REC;
	Ogre::OverlayElement* mGUI_JP_DISTt;
	Ogre::OverlayElement* mGUI_JP_TIMEt;
	Ogre::OverlayElement* mGUI_JP_FLIPSt;
	Ogre::OverlayElement* mGUI_JP_2WHt;

	// records!
	HighScore mScores;

	// main GUI window.
	CEGUI::TabPane* mPane;
	CEGUI::TabPane* mPane2;
	CEGUI::StaticText* mHint;

	// main preview image
	CEGUI::StaticImage* mPrevImg;

	// buttons!
	StandardButton* mButAdd;
	StandardButton* mButGo;
	StandardButton* mButQuit;
	StandardButton* mButOpen;
	StandardButton* mButSave;
	StandardButton* mButNew;
	StandardButton* mButSettings;
	StandardButton* mButLoadReplay;
	StandardButton* mButSaveReplay;
	StandardButton* mButClearScores;

	// program state
	StuntPlayground::ProgState mState;

	// my custom camera class.
	FollowCamera* mFollowCam;

	// base level Rigid Body
	OgreNewt::Body* mArenaBody;
	OgreNewt::Body* mWorldLimitsBody;

	// dummy body for prop placement
	DummyBody* mDummyBody;

	// AABB visualizer
	MyAABB* mAABB;
	
	// base scene node that holds all the props in the level.
	Ogre::SceneNode* mPropBaseNode;
	iteratorRemoveProps* mIteratorRemoveProps;

	// pointer to current selected node.
	StuntPlayground::Prop* mSelectedProp;

	
	// material pairs.
	OgreNewt::MaterialPair* mMatPairArenaVsCar;
	OgreNewt::MaterialPair* mMatPairCarVsProp;

	// newton callbacks
	CarArenaCallback* mCarArenaCallback;
	DummyCollisionCallback* mDummyCallback;
	PropCollision* mPropCallback;
	CarPropCollision* mCarPropCallback;

	// vehicle list.
	Ogre::FileInfoListPtr mVehicleList;
	unsigned short mCurrentVehicle;

	// dialog boxes.
	LoadDialog* mLoadDialog;
	SaveDialog* mSaveDialog;
	ErrorDialog* mErrorDialog;
	SettingDialog* mSettingDialog;
	ReplayControls* mReplayControls;

	// face indexes for car/arena collision
	unsigned int mArenaIDRoad;
	unsigned int mArenaIDRoadChecker;
	unsigned int mArenaIDGround;

	// all of the camera angles available.
	std::vector<CameraAngle> mCameraAngles;

	unsigned int mCurrentCamAngle;

	Ogre::ParticleSystem* mDirtParticles;
	Ogre::SceneNode* mDirtParticlesNode;

	Ogre::ParticleSystem* mSparkParticles;
	Ogre::SceneNode* mSparkParticlesNode;


	// Reflection Cameras.
	unsigned int mCurrentReflectCam;
	std::vector<ReflectCam> mReflectCams;
	std::vector<Ogre::String> mOldTextures;
	ReflectionRenderTarget* mReflectionListener;
	bool mReflectCamEnabled;

	// game settings
	GameSettings mSettings;


	float buffer[50];


};



}	// end NAMESPACE StuntPlayground


#endif