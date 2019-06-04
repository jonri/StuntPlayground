/*
	Stunt Playground
	
	StandardButton
*/

#ifndef _STUNTPLAYGROUND_STANDARDBUTTON_
#define _STUNTPLAYGROUND_STANDARDBUTTON_

#include <CEGUI.h>

namespace StuntPlayground
{



	class StandardButton
	{
		CEGUI::PushButton* mButton;

	public:
		StandardButton( CEGUI::String name, CEGUI::String text )
		{
			using namespace CEGUI;
			mButton = (PushButton*)WindowManager::getSingleton().createWindow((utf8*)"WindowsLook/Button", name );

			mButton->setText( text );
			mButton->setNormalTextColour( colour(0.8,0.8,0.8) );
			mButton->setHoverTextColour( colour(1,1,1) );
			mButton->setPushedTextColour( colour(1,0.3,0.3) );
		}

		~StandardButton()
		{
			using namespace CEGUI;
			WindowManager::getSingleton().destroyWindow( mButton );
		}

		CEGUI::PushButton* getButton() { return mButton; }


	};















}	// end NAMESPACE StuntPlayground

#endif // _STUNTPLAYGROUND_STANDARDBUTTON_