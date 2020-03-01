#ifndef MARKIV_BUILDINGBROWSER_WIDGET_H_
#define MARKIV_BUILDINGBROWSER_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "EntryWidget.h"
#include "LabelWidget.h"
#include "../FileScanner.h"
#include "../scenery/BuildingDef.h"
#include "MeshBrowserWidget.h"

using namespace Ogre;

class BuildingBrowserWidget : public MeshBrowserWidget {
public:
	BuildingBrowserWidget();
	bool initDetails();
	bool freeDetails();
	bool displaySelectedDetails();
	BuildingDef* mpSelectedUnitDetails;
};

#endif

