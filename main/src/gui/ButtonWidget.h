#ifndef MARKIV_BUTTON_WIDGET_H_
#define MARKIV_BUTTON_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"
#include "../globals.h"

using namespace Ogre;

// NOTE: we can add some functions for:
// * changing z-order
// * changing scale
// if required. If we add all these we may as well have an abstract widget class

// generic button. uses 3 default images {basic/mouseover/selected}
class ButtonWidget {
public:
	ButtonWidget();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, const char* text, bool toggleType = false);
	// note xpos and ypos are 0.0-0.1
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	OverlayElement* getElement();
	bool show();
	bool hide();
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool setText(const char *text);
	bool update(unsigned int mousex, unsigned int mousey, bool lmb); // change gfx if hover
	bool isHighlighted();
	bool isClicked();
	bool isSelected();
	void setSelected(bool selected);
	void deactivate();
	void activate();
protected:
	// this is why polymorphism sucks:
	OverlayElement* pBaseElement;							// administrative cast of the container panel
	OverlayContainer* pContainer;							// nesting cast of the container panel
	BorderPanelOverlayElement* pBorderPanel;	// display cast of the container panel

	// child elements
	OverlayElement* pButtonText_OE;						// administrative cast of text
	TextAreaOverlayElement* pButtonText_TAOE;	// display cast of text
	OverlayElement* pHighlight;								// display cast of highlight

	// state variables etc
	bool mHighlighted;
	bool mClicked;
	bool mSelected;
	float mXProp;
	float mYProp;
	float mButtonWidth;
	float mButtonHeight;
	float mXPos;
	float mYPos;
	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	bool mToggleType;
	bool mActive;
}; 

#endif
