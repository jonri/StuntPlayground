/*
	Stunt Playground
	
	StuntPlayground:: BodyIterators
*/

#ifndef _STUNTPLAYGROUND_BODYITERATORS_
#define _STUNTPLAYGROUND_BODYITERATORS_

#include "OgreNewt.h"
#include "OgreNewt_Body.h"

namespace StuntPlayground
{

	class iteratorRemoveProps : public OgreNewt::BodyIterator
	{
	public:
		iteratorRemoveProps() : BodyIterator() {}
		~iteratorRemoveProps() {}

		void userIterator( OgreNewt::Body* body )
		{
			if (body->getType() == StuntPlayground::BT_PROP)
			{
				Ogre::LogManager::getSingleton().logMessage(" ITERATOR - found prop");
			}
		}
	};


}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_BODYITERATORS_
