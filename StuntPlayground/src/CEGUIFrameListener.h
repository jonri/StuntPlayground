/*
	Stunt Playground
	
	CEGUIFrameListener
*/
#ifndef _STUNTPLAYGROUND_CEGUIFRAMELISTENER_
#define _STUNTPLAYGROUND_CEGUIFRAMELISTENER_

#include "CEGUI.h"
#include "WalaberOgreFrameListener.h"
#include "OgreEventQueue.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////

namespace StuntPlayground
{


class CEGUIFrameListener : public WalaberOgreFrameListener, Ogre::MouseMotionListener, Ogre::MouseListener
{
public:
	CEGUIFrameListener(RenderWindow* win, Camera* cam, CEGUI::TabPane* pane, CEGUI::TabPane* pane2) : WalaberOgreFrameListener(win)
	{
		mInputDevice2 = mInputDevice;

		mEventProcessor = new EventProcessor();
		mEventProcessor->initialise(win);
		mEventProcessor->startProcessingEvents();
		mEventProcessor->addKeyListener(this);
		mEventProcessor->addMouseMotionListener(this);
		mEventProcessor->addMouseListener(this);
		mInputDevice = mEventProcessor->getInputReader();

		mQuit = false;
		mSkipCount = 0;
		mUpdateFreq = 0;

		mPane = pane;
		mPane2 = pane2;
	}

	~CEGUIFrameListener()
	{
		mEventProcessor->stopProcessingEvents();
		mEventProcessor->removeKeyListener(this);
		mEventProcessor->removeMouseMotionListener(this);
		mEventProcessor->removeMouseListener(this);
		delete mEventProcessor;

		mInputDevice = mInputDevice2;
	}


	virtual void 	mouseMoved (MouseEvent *e)
	{
		CEGUI::Renderer* rend = CEGUI::System::getSingleton().getRenderer();
		CEGUI::System::getSingleton().injectMouseMove(e->getRelX() * rend->getWidth(), e->getRelY() * rend->getHeight());
		e->consume();
	}


	virtual void 	mouseDragged (MouseEvent *e)
	{
		mouseMoved(e);
	}


	virtual void 	keyPressed (KeyEvent *e)
	{
		/* // give 'quitting' priority
		if (e->getKey() == KC_ESCAPE)
		{
			mQuit = true;
			e->consume();
			return;
		} */

		// do event injection
		CEGUI::System& cegui = CEGUI::System::getSingleton();

		// key down
		cegui.injectKeyDown(e->getKey());

		// now character
		cegui.injectChar(e->getKeyChar());

		e->consume();
	}


	virtual void 	keyReleased (KeyEvent *e)
	{
		CEGUI::System::getSingleton().injectKeyUp(e->getKey());
	}



	virtual void 	mousePressed (MouseEvent *e)
	{
		CEGUI::System::getSingleton().injectMouseButtonDown(convertOgreButtonToCegui(e->getButtonID()));
		e->consume();
	}


	virtual void 	mouseReleased (MouseEvent *e)
	{
		CEGUI::System::getSingleton().injectMouseButtonUp(convertOgreButtonToCegui(e->getButtonID()));
		e->consume();
	}

	// do-nothing events
	virtual void 	keyClicked (KeyEvent *e) {}
	virtual void 	mouseClicked (MouseEvent *e) {}
	virtual void 	mouseEntered (MouseEvent *e) {}
	virtual void 	mouseExited (MouseEvent *e) {}


	bool frameStarted(const FrameEvent& evt);

protected:
	CEGUI::MouseButton convertOgreButtonToCegui(int ogre_button_id)
	{
		switch (ogre_button_id)
		{
		case MouseEvent::BUTTON0_MASK:
			return CEGUI::LeftButton;
			break;

		case MouseEvent::BUTTON1_MASK:
			return CEGUI::RightButton;
			break;

		case MouseEvent::BUTTON2_MASK:
			return CEGUI::MiddleButton;
			break;

		case MouseEvent::BUTTON3_MASK:
			return CEGUI::X1Button;
			break;

		default:
			return CEGUI::LeftButton;
			break;
		}

	}

protected:
	Ogre::EventProcessor* mEventProcessor;

	float	mSkipCount;
	float	mUpdateFreq;

	Ogre::InputReader* mInputDevice2;
	CEGUI::TabPane* mPane;
	CEGUI::TabPane* mPane2;

	bool mQuit;
};



}	// end NAMESPACE StuntPlayground




#endif	// _STUNTPLAYGROUND_CEGUIFRAMELISTENER_