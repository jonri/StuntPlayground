/*
	WalaberOgreFrameListener

	basic frame listener, with mouse/keyboard input.  nothing else :)
*/

#ifndef __WALABER_OGRE_FRAMELISTENER_H__
#define __WALABER_OGRE_FRAMELISTENER_H__

#include "Ogre.h"
#include "OgreKeyEvent.h"
#include "OgreEventListeners.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

using namespace Ogre;

class WalaberOgreFrameListener: public FrameListener, public KeyListener
{

public:
    // Basic contructor. 
    WalaberOgreFrameListener(RenderWindow* win)
    {
		mInputDevice = PlatformManager::getSingleton().createInputReader();
        mInputDevice->initialise(win,true, true);
        
        mWindow = win;		
    }

    virtual ~WalaberOgreFrameListener()
    {
		if (mInputDevice)
			PlatformManager::getSingleton().destroyInputReader( mInputDevice );
	}

 
    // basic frameStarted function. must be overwritten.
    bool frameStarted(const FrameEvent& evt)
    {
        if(mWindow->isClosed())
            return false;

		return true;
    }

    bool frameEnded(const FrameEvent& evt)
    {
        return true;
    }

	void keyClicked(KeyEvent* e) {}
	void keyPressed(KeyEvent* e) {}
	void keyReleased(KeyEvent* e) {}


protected:
    InputReader* mInputDevice;

    RenderWindow* mWindow;
    

};

#endif	// __WALABER_OGRE_FRAMELISTENER_H__
