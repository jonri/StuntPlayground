#include "StuntPlaygroundApplication.h"
#include "Dialogs.h"




namespace StuntPlayground
{

	BasicDialog::BasicDialog(CEGUI::Window* parentwin, Ogre::String& basename)
	{
		using namespace CEGUI;
		mWindow = (FrameWindow*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/FrameWindow", basename+"/Window" );
		parentwin->addChildWindow( mWindow );
		mWindow->setCloseButtonEnabled(false);
		mWindow->setSizingEnabled(false);
		mWindow->setCaptionColour( CEGUI::colour(0.8,0.8,0.8) );
		mWindow->setPosition( Point(0.3,0.2) );
			
		mButtonOK = new StandardButton(basename+"/OK", "OK" );
		mWindow->addChildWindow( mButtonOK->getButton() );

		mButtonCancel = new StandardButton(basename+"/CANCEL", "Cancel" );
		mWindow->addChildWindow( mButtonCancel->getButton() );

		mButtonOK->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber(&BasicDialog::playClickSound, this) );
		mButtonOK->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber(&BasicDialog::okClicked, this) );

		mButtonCancel->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber(&BasicDialog::playClickSound, this) );
		mButtonCancel->getButton()->subscribeEvent( PushButton::EventClicked, Event::Subscriber(&BasicDialog::cancelClicked, this) );

		mPlaySounds = false;

	}

	BasicDialog::~BasicDialog()
	{
		// cleanup.
		using namespace CEGUI;

		delete mButtonOK;
		delete mButtonCancel;
		WindowManager::getSingleton().destroyWindow( mWindow );
	}

	LoadDialog::LoadDialog( CEGUI::Window* parentwindow ) : BasicDialog( parentwindow, Ogre::String("LoadDialog") )
	{
		using namespace CEGUI;
		mWindow->setSize( Size(0.4,0.4) );

		mButtonOK->getButton()->setSize( Size(0.3,0.1) );
		mButtonOK->getButton()->setPosition( Point(0.3,0.85) );

		mButtonCancel->getButton()->setSize( Size(0.3,0.1) );
		mButtonCancel->getButton()->setPosition( Point(0.65,0.85) );

		mListbox = (Listbox*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/Listbox", (utf8*)"LoadDialog/List" );
		mWindow->addChildWindow( mListbox );
		mListbox->setSize( Size(0.9,0.65) );
		mListbox->setPosition( Point(0.05,0.15) );
		mListbox->setMultiselectEnabled(false);

		setPlaySounds( true );

		mLM = LM_ARENA;
	}

	LoadDialog::~LoadDialog()
	{
		using namespace CEGUI;
			
		while (mListbox->getItemCount() > 0)
		{
			OpenListItem* rem = (OpenListItem*)mListbox->getListboxItemFromIndex(0);
			mListbox->removeItem( rem );
			delete rem;
		}
	}

	void LoadDialog::setup( LoadMode mode )
	{
		mLM = mode;

		// first clear out the existing list of files.
		while (mListbox->getItemCount() > 0)
		{
			OpenListItem* rem = (OpenListItem*)mListbox->getListboxItemFromIndex(0);
			mListbox->removeItem( rem );
			delete rem;
		}

		Ogre::FileInfoListPtr list;

		if (mLM == LM_ARENA)
		{
			list = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo( "Arenas", "*.arena" );
		}

		if (mLM == LM_REPLAY)
		{
			list = Ogre::ResourceGroupManager::getSingleton().findResourceFileInfo( "Replays", "*.replay" );
		}

		for( unsigned int i=0; i<list->size(); i++)
		{
			Ogre::FileInfo file;
			file = list->at(i);

			OpenListItem* item = new OpenListItem( file.filename );
			item->setAutoDeleted( false );
			mListbox->addItem( item );
		}
	}


	SaveDialog::SaveDialog( CEGUI::Window* parentwin ) : BasicDialog( parentwin, Ogre::String("SaveDialog") )
	{
		using namespace CEGUI;
		mWindow->setSize( Size(0.4,0.15) );

		mButtonOK->getButton()->setSize( Size(0.3,0.28) );
		mButtonOK->getButton()->setPosition( Point(0.3,0.65) );

		mButtonCancel->getButton()->setSize( Size(0.3,0.28) );
		mButtonCancel->getButton()->setPosition( Point(0.65,0.65) );

		mEditbox = (Editbox*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/Editbox", (utf8*)"SaveDialog/Editbox" );
		mWindow->addChildWindow( mEditbox );
		mEditbox->setSize( Size( 0.9, 0.25 ) );
		mEditbox->setPosition( Point(0.05,0.33) );
		mEditbox->setNormalTextColour( colour(1,1,1) );
		mEditbox->setNormalSelectBrushColour( colour(0.8,0,0) );

		setPlaySounds( true );

		mSM = SM_ARENA;
	}

	ErrorDialog::ErrorDialog( CEGUI::Window* parentwin ) : BasicDialog( parentwin, Ogre::String("ErrorDialog") )
	{
		using namespace CEGUI;
		mWindow->setSize( Size(0.4,0.15) );
		mWindow->setText( "Error!" );

		mButtonOK->getButton()->setSize( Size(0.3,0.28) );
		mButtonOK->getButton()->setPosition( Point(0.3,0.65) );

		mButtonCancel->getButton()->setSize( Size(0.3,0.28) );
		mButtonCancel->getButton()->setPosition( Point(0.65,0.65) );

		mText = (StaticText*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"ErrorDialog/Text" );
		mWindow->addChildWindow( mText );
		mText->setSize( Size( 0.9, 0.25 ) );
		mText->setPosition( Point(0.05,0.33) );
		mText->setTextColours( colour(1,1,1) );
		mText->setFrameEnabled(false);

		setPlaySounds( true );
	}

	SettingDialog::SettingDialog( CEGUI::Window* parentwin, Ogre::SceneManager* mgr ) : BasicDialog( parentwin, Ogre::String("SettingDialog") )
	{
		mSceneMgr = mgr;

		using namespace CEGUI;
		mWindow->setSize( Size(0.3,0.8) );
		mWindow->setPosition( Point(0.4,0.1) );
		mWindow->setText( "GAME SETTINGS" );

		mButtonOK->getButton()->setSize( Size(0.4,0.05) );
		mButtonOK->getButton()->setPosition( Point(0.5,0.93) );

		mButtonCancel->getButton()->hide();

		StaticText* Text = (StaticText*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"SettingDialog/SoundVolText" );
		mWindow->addChildWindow( Text );
		Text->setSize( Size( 0.9, 0.03 ) );
		Text->setPosition( Point(0.05,0.08) );
		Text->setTextColours( colour(1,1,1) );
		Text->setFrameEnabled(false);
		Text->setBackgroundColours( colour(0,0,0,0) );
		Text->setText((utf8*)"Sound Volume");

		mSoundSlider = (Slider*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/Slider", (utf8*)"SettingDialog/SoundSlider");
		mWindow->addChildWindow( mSoundSlider );
		mSoundSlider->setSize( Size(0.9,0.05) );
		mSoundSlider->setPosition( Point(0.05, 0.1) );
		mSoundSlider->setVisible(true);
		mSoundSlider->setMaxValue( 255.0 );
		mSoundSlider->setClickStep( 4.0 );

		mSoundSlider->subscribeEvent( Slider::EventValueChanged, Event::Subscriber( &SettingDialog::sliderMoved, this ) );

		Text = (StaticText*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"SettingDialog/ShadowText" );
		mWindow->addChildWindow( Text );
		Text->setSize( Size( 0.9, 0.03 ) );
		Text->setPosition( Point(0.05,0.2) );
		Text->setTextColours( colour(1,1,1) );
		Text->setFrameEnabled(false);
		Text->setBackgroundColours( colour(0,0,0,0) );
		Text->setText((utf8*)"Realtime Shadows");

		mRadShadON = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/ShadowRadON");
		mWindow->addChildWindow( mRadShadON );
		mRadShadON->setSize( Size(0.2,0.05) );
		mRadShadON->setPosition( Point(0.15,0.23) );
		setupRadio( mRadShadON );
		mRadShadON->setGroupID( RG_SHADOWS );
		mRadShadON->setText((utf8*)"ON");

		mRadShadOFF = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/ShadowRadOFF");
		mWindow->addChildWindow( mRadShadOFF );
		mRadShadOFF->setSize( Size(0.2,0.05) );
		mRadShadOFF->setPosition( Point(0.45,0.23) );
		setupRadio( mRadShadOFF );
		mRadShadOFF->setGroupID( RG_SHADOWS );
		mRadShadOFF->setText((utf8*)"OFF");

		mRadShadON->subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );
		mRadShadOFF->subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );


		Text = (StaticText*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"SettingDialog/ReflectText" );
		mWindow->addChildWindow( Text );
		Text->setSize( Size( 0.9, 0.03 ) );
		Text->setPosition( Point(0.05,0.3) );
		Text->setTextColours( colour(1,1,1) );
		Text->setFrameEnabled(false);
		Text->setBackgroundColours( colour(0,0,0,0) );
		Text->setText((utf8*)"Realtime Reflections");

		mRadReflON = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/ReflectRadON");
		mWindow->addChildWindow( mRadReflON );
		mRadReflON->setSize( Size(0.2,0.05) );
		mRadReflON->setPosition( Point(0.15,0.33) );
		setupRadio( mRadReflON );
		mRadReflON->setGroupID( RG_REFLECTIONS );
		mRadReflON->setText((utf8*)"ON");

		mRadReflOFF = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/ReflectRadOFF");
		mWindow->addChildWindow( mRadReflOFF );
		mRadReflOFF->setSize( Size(0.2,0.05) );
		mRadReflOFF->setPosition( Point(0.45,0.33) );
		setupRadio( mRadReflOFF );
		mRadReflOFF->setGroupID( RG_REFLECTIONS );
		mRadReflOFF->setText((utf8*)"OFF");

		mRadReflON->Window::subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );
		mRadReflOFF->Window::subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );


		Text = (StaticText*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"SettingDialog/ManualText" );
		mWindow->addChildWindow( Text );
		Text->setSize( Size( 0.9, 0.03 ) );
		Text->setPosition( Point(0.05,0.4) );
		Text->setTextColours( colour(1,1,1) );
		Text->setFrameEnabled(false);
		Text->setBackgroundColours( colour(0,0,0,0) );
		Text->setText((utf8*)"Manual Transmission");

		mRadManON = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/ManualRadON");
		mWindow->addChildWindow( mRadManON );
		mRadManON->setSize( Size(0.2,0.05) );
		mRadManON->setPosition( Point(0.15,0.43) );
		setupRadio( mRadManON );
		mRadManON->setGroupID( RG_MANUAL );
		mRadManON->setText((utf8*)"ON");

		mRadManOFF = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/ManualRadOFF");
		mWindow->addChildWindow( mRadManOFF );
		mRadManOFF->setSize( Size(0.2,0.05) );
		mRadManOFF->setPosition( Point(0.45,0.43) );
		setupRadio( mRadManOFF );
		mRadManOFF->setGroupID( RG_MANUAL );
		mRadManOFF->setText((utf8*)"OFF");

		mRadManON->Window::subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );
		mRadManOFF->Window::subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );


		Text = (StaticText*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticText", (utf8*)"SettingDialog/RestoreText" );
		mWindow->addChildWindow( Text );
		Text->setSize( Size( 0.9, 0.03 ) );
		Text->setPosition( Point(0.05,0.5) );
		Text->setTextColours( colour(1,1,1) );
		Text->setFrameEnabled(false);
		Text->setBackgroundColours( colour(0,0,0,0) );
		Text->setText((utf8*)"Restore arena after driving");

		mRadRestON = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/RestoreRadON");
		mWindow->addChildWindow( mRadRestON );
		mRadRestON->setSize( Size(0.2,0.05) );
		mRadRestON->setPosition( Point(0.15,0.53) );
		setupRadio( mRadRestON );
		mRadRestON->setGroupID( RG_RESTORE );
		mRadRestON->setText((utf8*)"YES");

		mRadRestOFF = (RadioButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/RadioButton", (utf8*)"SettingDialog/RestoreRadOFF");
		mWindow->addChildWindow( mRadRestOFF );
		mRadRestOFF->setSize( Size(0.2,0.05) );
		mRadRestOFF->setPosition( Point(0.45,0.53) );
		setupRadio( mRadRestOFF );
		mRadRestOFF->setGroupID( RG_RESTORE );
		mRadRestOFF->setText((utf8*)"NO");

		mRadRestON->Window::subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );
		mRadRestOFF->Window::subscribeEvent( RadioButton::EventSelectStateChanged, Event::Subscriber( &SettingDialog::radioClicked, this ) );

		setPlaySounds( true );

	}


	void SettingDialog::show( const GameSettings set )
	{
		mSettings = set;

		mWasOkClicked = false;

		// SHADOWS
		Ogre::ShadowTechnique tech = mSettings.shadowTechnique;
		if (tech == Ogre::SHADOWTYPE_STENCIL_MODULATIVE)
		{
			mRadShadON->setSelected(true);
		}
		else
		{
			mRadShadOFF->setSelected(true);
		}

		// REFLECTIONS
		if (mSettings.useRealtimeReflections)
		{
			mRadReflON->setSelected(true);
		}
		else
		{
			mRadReflOFF->setSelected(true);
		}

		// MANUAL
		if (mSettings.useManualTransmission)
		{
			mRadManON->setSelected(true);
		}
		else
		{
			mRadManOFF->setSelected(true);
		}

		// RESTORE
		if (mSettings.restoreArena)
		{
			mRadRestON->setSelected(true);
		}
		else
		{
			mRadRestOFF->setSelected(true);
		}

		// SFX VOLUME
		mSoundSlider->setCurrentValue( (float)mSettings.sfxVolume );
			
		mWindow->show();
	}

	bool SettingDialog::radioClicked( const CEGUI::EventArgs& e )
	{
		using namespace CEGUI;
		RadioButton* but;

		but = (RadioButton*)((const WindowEventArgs&)e).window;

		if (but->getGroupID() == RG_SHADOWS)
		{
			if (but->getSelectedButtonInGroup()->getText() == "ON")
			{
				mSceneMgr->setShadowTechnique( Ogre::SHADOWTYPE_STENCIL_MODULATIVE );
				mSettings.shadowTechnique = Ogre::SHADOWTYPE_STENCIL_MODULATIVE;
			}
			else
			{
				mSceneMgr->setShadowTechnique( Ogre::SHADOWTYPE_NONE );
				mSettings.shadowTechnique = Ogre::SHADOWTYPE_NONE;
			}
		}

		if (but->getGroupID() == RG_REFLECTIONS)
		{
			if (but->getSelectedButtonInGroup()->getText() == "ON")
			{
				mSettings.useRealtimeReflections = true;
				Application::getSingleton().reflectionsON();
			}
			else
			{
				mSettings.useRealtimeReflections = false;
				Application::getSingleton().reflectionsOFF();
			}
		}

		if (but->getGroupID() == RG_MANUAL)
		{
			if (but->getSelectedButtonInGroup()->getText() == "ON")
			{
				mSettings.useManualTransmission = true;
			}
			else
			{
				mSettings.useManualTransmission = false;
			}
		}

		if (but->getGroupID() == RG_RESTORE)
		{
			if (but->getSelectedButtonInGroup()->getText() == "YES")
			{
				mSettings.restoreArena = true;
			}
			else
			{
				mSettings.restoreArena = false;
			}
		}

		return true;
	}

	bool SettingDialog::sliderMoved( const CEGUI::EventArgs& e )
	{
		using namespace CEGUI;

		Slider* sl = (Slider*)((WindowEventArgs&)e).window;

		SoundManager::getSingleton().setMasterSFXVolume( (int)(sl->getCurrentValue()) );
		Sound* chimes = SoundManager::getSingleton().getSound("chimes");
		if (chimes)
		{
			if (!chimes->isPlaying()) { chimes->play(); }
		}

		return true;
	}


	ReplayControls::ReplayControls( CEGUI::Window* parentwin )
	{
		using namespace CEGUI;

		mPlaySounds = true;

		mPane = (TabPane*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/TabPane", (utf8*)"ReplayControls/Pane");
		parentwin->addChildWindow( mPane );
		mPane->setSize( Size(0.3,0.06) );
		mPane->setPosition( Point(1.05,0.9) );

		mImgTop = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/TOP");
		mPane->addChildWindow( mImgTop );
		mImgTop->setSize( Size(0.1,0.96) );
		mImgTop->setPosition( Point(0.01,0.02) );
		mImgTop->setImage((utf8*)"UI", (utf8*)"IR_Top");
		mImgTop->setID( RM_TOP );

		mImgRW = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/RW");
		mPane->addChildWindow( mImgRW );
		mImgRW->setSize( Size(0.1,0.96) );
		mImgRW->setPosition( Point(0.12,0.02) );
		mImgRW->setImage((utf8*)"UI", (utf8*)"IR_RW");
		mImgRW->setID( RM_REWIND );

		mImgRP = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/RP");
		mPane->addChildWindow( mImgRP );
		mImgRP->setSize( Size(0.1,0.96) );
		mImgRP->setPosition( Point(0.23,0.02) );
		mImgRP->setImage((utf8*)"UI", (utf8*)"IR_RP");
		mImgRP->setID( RM_REWPLAY );

		mImgRS = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/RS");
		mPane->addChildWindow( mImgRS );
		mImgRS->setSize( Size(0.1,0.96) );
		mImgRS->setPosition( Point(0.34,0.02) );
		mImgRS->setImage((utf8*)"UI", (utf8*)"IR_RS");
		mImgRS->setID( RM_REWSLOW );

		mImgPS = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/PS");
		mPane->addChildWindow( mImgPS );
		mImgPS->setSize( Size(0.1,0.96) );
		mImgPS->setPosition( Point(0.45,0.02) );
		mImgPS->setImage((utf8*)"UI", (utf8*)"IR_PS");
		mImgPS->setID( RM_PAUSE );

		mImgFS = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/FS");
		mPane->addChildWindow( mImgFS );
		mImgFS->setSize( Size(0.1,0.96) );
		mImgFS->setPosition( Point(0.56,0.02) );
		mImgFS->setImage((utf8*)"UI", (utf8*)"IR_FS");
		mImgFS->setID( RM_PLAYSLOW );

		mImgFP = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/FP");
		mPane->addChildWindow( mImgFP );
		mImgFP->setSize( Size(0.1,0.96) );
		mImgFP->setPosition( Point(0.67,0.02) );
		mImgFP->setImage((utf8*)"UI", (utf8*)"IR_FP");
		mImgFP->setID( RM_PLAY );

		mImgFF = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/FF");
		mPane->addChildWindow( mImgFF );
		mImgFF->setSize( Size(0.1,0.96) );
		mImgFF->setPosition( Point(0.78,0.02) );
		mImgFF->setImage((utf8*)"UI", (utf8*)"IR_FF");
		mImgFF->setID( RM_FASTFORWARD );

		mImgEnd = (StaticImage*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/StaticImage", (utf8*)"ReplayControls/END");
		mPane->addChildWindow( mImgEnd );
		mImgEnd->setSize( Size(0.1,0.96) );
		mImgEnd->setPosition( Point(0.89,0.02) );
		mImgEnd->setImage((utf8*)"UI", (utf8*)"IR_End");
		mImgEnd->setID( RM_END );


		wireImg( mImgTop );
		wireImg( mImgRW );
		wireImg( mImgRP );
		wireImg( mImgRS );
		wireImg( mImgPS );
		wireImg( mImgFS );
		wireImg( mImgFP );
		wireImg( mImgFF );
		wireImg( mImgEnd );

		setMode(ReplayControls::RM_PAUSE );

	}

	void ReplayControls::setMode( Mode m )
	{ 
		mMode = m;
	
		using namespace CEGUI;
		// loop through all buttons.
		for (unsigned int i=0; i<mPane->getChildCount(); i++)
		{
			StaticImage* child = (StaticImage*)mPane->getChild(i);
			if (child->getID() == m)
			{
				child->setFrameColours( colour(1,0.2,0.2) );
			}
			else
			{
				child->setFrameColours( colour(0.8,0.8,0.8) );
			}
		}
	}

















}	// end NAMESPACE StuntPlayground