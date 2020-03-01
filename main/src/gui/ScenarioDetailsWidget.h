#ifndef MARKIV_SCENDETAILSPANEL_H_
#define MARKIV_SCENDETAILSPANEL_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePanelOverlayElement.h"
#include "LabelWidget.h"
#include "../scenery/ScenarioDetails.h"

using namespace Ogre; 

class ScenarioDetailsWidget {
public:
	ScenarioDetailsWidget();
	bool init(const char* uniqueName, float parentLeft, float parentRight, float parentTop, float parentBottom, float xProp, float yProp);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool moveWidget(float xProp, float yProp);	// move to a position relative to size of parent (0.0-1.0)
	bool show();
	bool hide();
	bool updateDetails(ScenarioDetails* details);
private:
	Overlay* pOverlay;
	OverlayElement* pBaseElement;
	OverlayContainer* pContainer;
	PanelOverlayElement* pPanel;
	LabelWidget mTimeLimitLabel;
	LabelWidget mNumberOfForcesLabel;
	LabelWidget mForce1Label;
	LabelWidget mForce2Label;

	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	float mWidgetWidthPixels; // in pixels
	float mWidgetHeightPixels; // in pixels
	float mXProp; // position relative to parent
	float mYProp; // position relative to parent
	float mXPos; // position in pixels (absolute)
	float mYPos; // position in pixels (absolute)
};

#endif
