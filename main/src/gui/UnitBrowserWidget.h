#ifndef MARKIV_UNITBROWSER_WIDGET_H_
#define MARKIV_UNITBROWSER_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "EntryWidget.h"
#include "LabelWidget.h"
#include "../FileScanner.h"
#include "../units/VehicleSpec.h"
#include "MeshBrowserWidget.h"

using namespace Ogre;

class UnitBrowserWidget : public MeshBrowserWidget {
public:
	UnitBrowserWidget();
	bool initDetails();
	bool freeDetails();
	bool displaySelectedDetails();
	VehicleSpec* mpSelectedUnitDetails;
private:
	LabelWidget mDescriptionLabel;
	LabelWidget mTopSpeedCrossLabel;
	LabelWidget mArmourDetails;
};

#endif

