#ifndef MARKIV_INFOBOX_WIDGET_H_
#define MARKIV_INFOBOX_WIDGET_H_

#include "Ogre.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"
#include "LabelWidget.h"

// a box panel with some text in it
class InfoBoxWidget {
public:
	InfoBoxWidget();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xProp, float yProp, float width, float height);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);
	bool show();
	bool hide();
	bool moveWidget(float xProp, float yProp);	// move to a position relative to size of parent (0.0-1.0) display if next/prev clicked
	bool isShowing();
	void setText(const char* text);
private:
	Overlay* pOverlay;
	OverlayElement* pBaseElement;
	OverlayContainer* pContainer;
	BorderPanelOverlayElement* pBorderPanel;

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
	bool mIsShowing;
	LabelWidget mText;
};

#endif

