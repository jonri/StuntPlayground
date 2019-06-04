/*
	Stunt Playground

	main.cpp
*/
#ifdef _DEBUG
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif // _DEBUG

#include "StuntPlaygroundApplication.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>



INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
{
#ifdef _DEBUG
		_CrtMemState s1, s2, s3;

		_CrtMemCheckpoint( &s1 );
#endif	// _DEBUG

    // Create application object
		StuntPlayground::Application* myapp = new StuntPlayground::Application;

    try {
		StuntPlayground::Application::getSingleton().go();
    } catch( Ogre::Exception& e ) {
		printf( "An Error HAS OCCURED: %s", e.getFullDescription() );
    }

	delete myapp;

#ifdef _DEBUG
	_CrtMemCheckpoint( &s2 );

	if ( _CrtMemDifference( &s3, &s1, &s2) )
		_CrtMemDumpStatistics( &s3 );
#endif	// _DEBUG

    return 0;
}