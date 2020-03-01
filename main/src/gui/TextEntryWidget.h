#ifndef MARKIV_TEXTENTRY_WIDGET_H_
#define MARKIV_TEXTENTRY_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"

using namespace Ogre;

// This is a widget for capturing user-input text
class TextEntryWidget {
public: // in a real gui this would extend some overcomplicated abstract class, but it isn't a real gui system
	TextEntryWidget(); // set defaults
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop);
	bool free(); // free allocated memory
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	OverlayElement* getElement();
	bool show();
	bool hide();
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool setTitleText(const char *text);
	bool setTypedText(const char *text); // call this method every time a user enters another character
	char* getTypedText(); // get the user-typed text
	bool update(unsigned int mousex, unsigned int mousey, bool lmb); // lets widget know if user is clicking to type or clicking to stop
	bool isHighlighted();
	bool isClicked();
	bool isSelected();
	void setSelected(bool selected);
	void deactivate();
	void activate();
protected:
	// this is why polymorphism sucks:
	OverlayElement* mpBaseElement;							// administrative cast of the container panel
	OverlayContainer* mpContainer;							// nesting cast of the container panel
	BorderPanelOverlayElement* mpBorderPanel;	// display cast of the container panel

	// child elements
	OverlayElement* mpTitleText_OE;						// administrative cast of text
	TextAreaOverlayElement* mpTitleText_TAOE;	// display cast of text
	OverlayElement* mpTypedText_OE;						// administrative cast of text
	TextAreaOverlayElement* mpTypedText_TAOE;	// display cast of text
	OverlayElement* mpHighlight;								// display cast of highlight

	// state variables etc
	char mTypedText[1024]; // user-typed text (hold up to 1kB of text)
	float mXProp;
	float mYProp;
	float mWidgetWidth;
	float mWidgetHeight;
	float mXPos;
	float mYPos;
	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	bool mHighlighted;
	bool mClicked;
	bool mSelected;
	bool mActive;
}; 

#endif
