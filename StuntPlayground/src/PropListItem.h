/*
	Stunt Playground
	
	PropListItem
*/

#ifndef _STUNTPLAYGROUND_PROPLISTITEM_
#define _STUNTPLAYGROUND_PROPLISTITEM_

#include <CEGUI.h>
#include "elements/CEGUIListboxTextItem.h"

namespace StuntPlayground
{

	class PropListItem : public CEGUI::ListboxTextItem
	{
	private:
		Ogre::String mBOD_File;

		CEGUI::String mImageset;
		CEGUI::String mImageName;
		CEGUI::String mMaterial;

	public:
		PropListItem( Ogre::String bod, CEGUI::String imageset, CEGUI::String image, CEGUI::String text, CEGUI::String material ) : ListboxTextItem(text)
		{
			mBOD_File = bod;
			mImageset = imageset;
			mImageName = image;
			mMaterial = material;

			using namespace CEGUI;
			// set default colors, etc.
			setTextColours( colour(0.8,0.8,0.8) );
			setSelectionBrushImage((utf8*)"WindowsLook",(utf8*)"Background");
			setSelectionColours( colour(1.0,0,0) );
		 }

		~PropListItem() {}

		Ogre::String& getBODFile() { return mBOD_File; }
		CEGUI::String& getImageset() { return mImageset; }
		CEGUI::String& getImageName() { return mImageName; }
		CEGUI::String& getMaterial() { return mMaterial; }

	};




}	// end NAMESPACE StuntPlayground

#endif	// _STUNTPLAYGROUND_PROPLISTITEM_