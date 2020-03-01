#ifndef MARKIV_BIGTITLE_WIDGET_H_
#define MARKIV_BIGTITLE_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"

using namespace Ogre;

class BigTitleWidget {
public:
	BigTitleWidget();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, const char* text);
	// note xpos and ypos are 0.0-0.1
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	OverlayElement* getElement();
	bool show();
	bool hide();
	bool moveWidget(float pixelX, float pixelY);	// note xpos and ypos are 0.0-0.1
	bool setText(const char *text);
private:
	// this is why polymorphism sucks:
	OverlayElement* pBaseElement;							// administrative cast of the container panel
	OverlayContainer* pContainer;							// nesting cast of the container panel
	BorderPanelOverlayElement* pBorderPanel;	// display cast of the container panel

	// child elements
	OverlayElement* pWidgetText_OE;						// administrative cast of text
	TextAreaOverlayElement* pWidgetText_TAOE;	// display cast of text
	OverlayElement* pHighlight;								// display cast of highlight

	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	float mWidgetWidth;
	float mWidgetHeight;
};

#endif
