#ifndef MARKIV_ABSTRACT_WIDGET_H_
#define MARKIV_ABSTRACT_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePanelOverlayElement.h"

using namespace Ogre;

// this class represents an abstract widget class with some generic widget functions
class AbstractWidget {
public:
	AbstractWidget();
	bool init(char* uniqueName, float parentTop, float parentLeft,  float parentBottom, float parentRight, float xprop, float yprop, char* text, bool toggleType);
	OverlayElement* getBaseElement();	// get a pointer to the main overlay element
	bool show();
	bool hide();
	void activate();
	void deactivate();
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool dragWidget(float xmove, float ymove); // e.g. if mouse drags
	bool resize(float top, float left, float bottom, float right);	// 'define parent bounds'
	bool isActive(); // etc etc state checking
	bool isShowing();
	char* getHorizontalAlignment();
	char* getVerticalAlignment();
protected:
	void resizeNestedWidgets();
	
	OverlayElement* mpBaseElement; // administrative cast of the container panel
	OverlayContainer* mpContainer; // nesting cast of the container panel
	float mParentBoundTop; // dimensions of parent widget or viewport
	float mParentBoundLeft; // dimensions of parent widget or viewport
	float mParentBoundBottom; // dimensions of parent widget or viewport
	float mParentBoundRight; // dimensions of parent widget or viewport
	float mXProp; // Position relative to viewport size 0.0-1.0
	float mYProp; // Position relative to viewport size 0.0-1.0
	float mXPos; // Absolute Position in pixels
	float mYPos; // Absolute Position in pixels
	float mWidgetWidth; // Width of widget in pixels
	float mWidgetHeight; // Width of widget in pixels
	bool mVisible; // if widget is currently visible
	bool mActive; // if widget is still active
};

#endif
