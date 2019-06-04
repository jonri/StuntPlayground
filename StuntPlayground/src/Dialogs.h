/*
	Stunt Playground
	
	StuntPlaygroundApplication
*/

#ifndef _STUNTPLAYGROUND_DIALOGS_
#define _STUNTPLAYGROUND_DIALOGS_


#include <Ogre.h>
#include "CEGUI.h"
#include "StandardButton.h"
#include "SoundManager.h"
#include "Settings.h"

namespace StuntPlayground
{
	////////////////////////////////////////////////////////////////////////
	// basic dialog!
	class BasicDialog
	{
	public:
		BasicDialog( CEGUI::Window* parentwin, Ogre::String& basename );

		virtual ~BasicDialog();

		virtual void show( Ogre::String& text )
		{
			mWasOkClicked = false;
			mWindow->setText( (std::string)text );
		
			mWindow->show();
		}

		virtual void hide()
		{
			mWindow->hide();
		}

		void setPlaySounds( bool setting ) { mPlaySounds = setting; }

		bool wasOkClicked() { return mWasOkClicked; }

		CEGUI::FrameWindow* getCEGUIWindow() { return mWindow; }

	private:
		bool okClicked( const CEGUI::EventArgs& e )
		{
			mWasOkClicked = true;
			hide();

			return true;
		}

		bool cancelClicked( const CEGUI::EventArgs& e )
		{
			hide();

			return true;
		}

		bool playClickSound( const CEGUI::EventArgs& e )
		{
			if (mPlaySounds)
				SoundManager::getSingleton().playSound("Click1");

			return true;
		}

	public:

		bool mWasOkClicked;
		bool mPlaySounds;
		CEGUI::FrameWindow* mWindow;
		StandardButton* mButtonOK;
		StandardButton* mButtonCancel;

		

	};



	////////////////////////////////////////////////////////////////////////
	class OpenListItem : public CEGUI::ListboxTextItem
	{
	public:
		OpenListItem( CEGUI::String text ) : ListboxTextItem(text)
		{
			using namespace CEGUI;
			// set default colors, etc.
			setTextColours( colour(0.8,0.8,0.8) );
			setSelectionBrushImage((utf8*)"WindowsLook",(utf8*)"Background");
			setSelectionColours( colour(1.0,0,0) );
		 }

		~OpenListItem() {}

	};


	class LoadDialog : public BasicDialog
	{
	public:
		enum LoadMode
		{
			LM_ARENA, LM_REPLAY
		};

		LoadDialog( CEGUI::Window* parentwindow );
		
		~LoadDialog();

		void setup( LoadMode mode );

		LoadMode getMode() { return mLM; }

		CEGUI::Listbox* getCEGUIListbox() { return mListbox; }
		LoadMode mLM;		


	private:

		CEGUI::Listbox* mListbox;
	};

	////////////////////////////////////////////////////////////////////////
	class SaveDialog : public BasicDialog
	{
	public:
		enum SaveMode
		{
			SM_ARENA, SM_REPLAY
		};

		SaveDialog( CEGUI::Window* parentwin );

		~SaveDialog() {}

		void setSM( SaveMode mode ) { mSM = mode; }
		SaveMode getSM() { return mSM; }

		Ogre::String getText()
		{
			CEGUI::String txt = mEditbox->getText();

			return Ogre::String( txt.c_str() );
		}

	private:
		CEGUI::Editbox* mEditbox;
		SaveMode mSM;

	};

	////////////////////////////////////////////////////////////////////////
	class ErrorDialog : public BasicDialog
	{
	public:

		ErrorDialog( CEGUI::Window* parentwin );

		~ErrorDialog() {}


		void show( Ogre::String& text, bool showCancel = true )
		{
			mWasOkClicked = false;
			mText->setText( (std::string)text );

			if (!showCancel)
				mButtonCancel->getButton()->hide();

			mWindow->show();
		}

	private:
		CEGUI::StaticText* mText;
	};



	////////////////////////////////////////////////////////////////////////
	class SettingDialog : public BasicDialog
	{
	public:
		enum RADIO_GROUPS { RG_SHADOWS, RG_REFLECTIONS, RG_MANUAL, RG_RESTORE };

		SettingDialog( CEGUI::Window* parentwin, Ogre::SceneManager* mgr );

		~SettingDialog() {}


		void show( const GameSettings set );

		void hide()
		{
			mSettings.sfxVolume = mSoundSlider->getCurrentValue();	

			mWindow->hide();
		}

		GameSettings getSettings()
		{
			return mSettings;
		}


	private:
		void setupRadio( CEGUI::RadioButton* rad )
		{
			using namespace CEGUI;

			rad->setNormalTextColour( colour(0.8,0.8,0.8) );
			rad->setPushedTextColour( colour(1,0.2,0.2) );
			rad->setDisabledTextColour( colour(0.5,0.5,0.5) );
			rad->setHoverTextColour( colour(1,1,1) );
		}

		bool radioClicked( const CEGUI::EventArgs& e );
		bool sliderMoved( const CEGUI::EventArgs& e );

		Ogre::SceneManager* mSceneMgr;
		CEGUI::Slider* mSoundSlider;
		CEGUI::RadioButton* mRadShadON;
		CEGUI::RadioButton* mRadShadOFF;
		CEGUI::RadioButton* mRadReflON;
		CEGUI::RadioButton* mRadReflOFF;
		CEGUI::RadioButton* mRadManON;
		CEGUI::RadioButton* mRadManOFF;
		CEGUI::RadioButton* mRadRestON;
		CEGUI::RadioButton* mRadRestOFF;

		GameSettings mSettings;
	};


	class ReplayControls 
	{
	public:
		enum Mode
		{
			RM_TOP, RM_REWIND, RM_REWPLAY, RM_REWSLOW, RM_PAUSE, RM_PLAYSLOW, RM_PLAY, RM_FASTFORWARD, RM_END
		};


		ReplayControls( CEGUI::Window* parentwin );

		~ReplayControls()
		{
			using namespace CEGUI;

			WindowManager::getSingleton().destroyWindow( mPane );
		}

		void show()
		{
			mPane->show();
		}

		void hide()
		{
			mPane->hide();
		}

		void setMode( Mode m );

		Mode getMode() { return mMode; }

		CEGUI::TabPane* getPane() { return mPane; }

		void setPlaySounds( bool setting ) { mPlaySounds = setting; }
	private:

		void wireImg( CEGUI::StaticImage* img )
		{
			using namespace CEGUI;
			img->subscribeEvent( StaticImage::EventMouseClick, Event::Subscriber( &ReplayControls::imgClicked, this ) );
			img->subscribeEvent( StaticImage::EventMouseEnters, Event::Subscriber( &ReplayControls::imgEnter, this ) );
			img->subscribeEvent( StaticImage::EventMouseLeaves, Event::Subscriber( &ReplayControls::imgLeave, this ) );
		}


		bool imgClicked( const CEGUI::EventArgs& e)
		{
			using namespace CEGUI;
			StaticImage* clicked = (StaticImage*)(((const CEGUI::WindowEventArgs&)e).window);

			setMode( (Mode)clicked->getID() );

			if (mPlaySounds)
				SoundManager::getSingleton().playSound( "click" );

			return true;
		}

		bool imgEnter( const CEGUI::EventArgs& e)
		{
			using namespace CEGUI;
			StaticImage* img = (StaticImage*)(((const CEGUI::WindowEventArgs&)e).window);
			if (mMode == img->getID()) { return true; }
			img->setFrameColours( colour(1,1,1) );
			return true;
		}

		bool imgLeave( const CEGUI::EventArgs& e)
		{
			using namespace CEGUI;
			StaticImage* img = (StaticImage*)(((const CEGUI::WindowEventArgs&)e).window);
			if (mMode == img->getID()) { return true; }
			img->setFrameColours( colour(0.8,0.8,0.8) );
			return true;
		}


		Mode mMode;

		bool mPlaySounds;

		CEGUI::TabPane* mPane;
		CEGUI::StaticImage* mImgTop;
		CEGUI::StaticImage* mImgRW;
		CEGUI::StaticImage* mImgRP;
		CEGUI::StaticImage* mImgRS;
		CEGUI::StaticImage* mImgPS;
		CEGUI::StaticImage* mImgFS;
		CEGUI::StaticImage* mImgFP;
		CEGUI::StaticImage* mImgFF;
		CEGUI::StaticImage* mImgEnd;
	};



		





}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_DIALOGS_
