/*
	Stunt Playground
	
	CEGUIFrameListener
*/
#include ".\StuntPlaygroundApplication.h"
#include ".\CEGUIFrameListener.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////

namespace StuntPlayground
{

bool CEGUIFrameListener::frameStarted( const FrameEvent &evt )
{
	Application& app = Application::getSingleton();

	if (mQuit)
		return false;
	
	using namespace CEGUI;

	CEGUI::Point pos = mPane->getPosition();
	CEGUI::Point pos2 = mPane2->getPosition();
	
	if (app.getProgState() == StuntPlayground::PS_WAITING)
	{
		//slid all the way out?
		if (pos.d_x < 0.0)
		{
			//slide out a bit.
			pos.d_x += 0.5 * evt.timeSinceLastFrame;
			if (pos.d_x > 0.0) { pos.d_x = 0.0; }
			mPane->setPosition( pos );
		}
	}
	else
	{
		//slid out?
		if (pos.d_x > -0.2)
		{
			//slide in a bit.
			pos.d_x -= 0.5 * evt.timeSinceLastFrame;
			if (pos.d_x < -0.2) { pos.d_x = -0.2; }
			mPane->setPosition( pos );
		}
	}

	if ((app.getProgState() == StuntPlayground::PS_WAITING) || (app.getProgState() == StuntPlayground::PS_REPLAYING))
	{
		if (pos2.d_x > 0.8)
		{
			//slide in a bit
			pos2.d_x -= 0.5 * evt.timeSinceLastFrame;
			if (pos2.d_x < 0.8) { pos2.d_x = 0.8; }
			mPane2->setPosition( pos2 );
		}
	}
	else
	{
		if (pos2.d_x < 1.0)
		{
			//slide out a bit
			pos2.d_x += 0.5 * evt.timeSinceLastFrame;
			if (pos2.d_x > 1.0) { pos2.d_x = 1.0; }
			mPane2->setPosition( pos2 );
		}
	}


	////////////////////////////////////////////////////////////////////////////

	TabPane* rc = app.getReplayControls()->getPane();
	pos = rc->getPosition();
	if (app.getProgState() == StuntPlayground::PS_REPLAYING)
	{
		// slid in the way out?
		if (pos.d_x > 0.65)
		{
			//slide in a bit.
			pos.d_x -= 0.5 * evt.timeSinceLastFrame;
			if (pos.d_x < 0.65) { pos.d_x = 0.65; }
			rc->setPosition( pos );
		}
	}
	else
	{
		// slid out?
		if (pos.d_x < 1.05)
		{
			//slide out a bit.
			pos.d_x += 0.5 * evt.timeSinceLastFrame;
			if (pos.d_x > 1.05) { pos.d_x = 1.05; }
			rc->setPosition( pos );
		}
	}



	return true;

}


}	// end NAMESPACE StuntPlayground