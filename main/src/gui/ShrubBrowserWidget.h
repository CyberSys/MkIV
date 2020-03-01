#ifndef MARKIV_SHRUBBROWSER_WIDGET_H_
#define MARKIV_SHRUBBROWSER_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "EntryWidget.h"
#include "LabelWidget.h"
#include "../FileScanner.h"
#include "../scenery/ShrubDef.h"
#include "MeshBrowserWidget.h"

using namespace Ogre;

class ShrubBrowserWidget : public MeshBrowserWidget {
public:
	ShrubBrowserWidget();
	bool initDetails();
	bool freeDetails();
	bool displaySelectedDetails();
	ShrubDef* mpSelectedUnitDetails;
};

#endif

