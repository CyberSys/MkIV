#include "TextEntryWidget.h"
#include "../globals.h"

// constructor
TextEntryWidget::TextEntryWidget() {
	mpBaseElement = NULL;
	mpContainer = NULL;
	mpBorderPanel = NULL;

	// child elements
	mpTitleText_OE = NULL;
	mpTitleText_TAOE = NULL;
	mpTypedText_OE = NULL;
	mpTypedText_TAOE = NULL;
	mpHighlight = NULL;

	// state variables etc
	strcpy(mTypedText, "...");
	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;

	mXProp = -1.0f;
	mYProp = -1.0f;
	mWidgetWidth = 250.0f;
	mWidgetHeight = 18.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;
	mHighlighted = false;
	mClicked = false;
	mSelected = false;
	mActive = true;	
}

// initialise text widget
bool TextEntryWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop) {
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mXProp = xprop;
	mYProp = yprop;

	// create a container panel for the widget
	char tmp[256];
	sprintf(tmp, "%s_BP", uniqueName);	// name of the panel
	// this is a weakness in the Ogre code that it has to be cast like this just to get a container -very poor program design
	mpBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	mpContainer = static_cast<Ogre::OverlayContainer*>(mpBaseElement);
	if (!mpContainer) {
		printf("ERROR: Could not create widget container\n");
		return false;
	}

	// set up the border panel (with another horrible cast)
	mpBorderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(mpBaseElement);
	
	mpBorderPanel->setMetricsMode(GMM_PIXELS);
	mpBorderPanel->setBorderSize(1.0f);
	mpBorderPanel->setMaterialName("mkiv/FileBrowserBackground");
	mpBorderPanel->setBorderMaterialName("mkiv/ButtonBorder");
	mpBorderPanel->setDimensions(mWidgetWidth, mWidgetHeight);

	// start the other elements
	sprintf(tmp, "%s_title", uniqueName);
	mpTitleText_OE = OverlayManager::getSingleton().createOverlayElement("TextArea", tmp);
	if (!mpTitleText_OE) {
		printf("ERROR: Could not create text overlay element\n");
		return false;
	}
	mpBorderPanel->addChild(mpTitleText_OE);
	mpTitleText_TAOE = static_cast<Ogre::TextAreaOverlayElement*>(mpTitleText_OE);

	mpTitleText_TAOE->setMetricsMode(GMM_PIXELS);
	mpTitleText_TAOE->setVerticalAlignment(GVA_CENTER);
	mpTitleText_TAOE->setHorizontalAlignment(GHA_LEFT);
	char title[256];
	strcpy(title, "Text Entry Widget");
	mpTitleText_TAOE->setCaption(tmp);
	mpTitleText_TAOE->setLeft(0);
	mpTitleText_TAOE->setTop(-20);
	mpTitleText_TAOE->setDimensions(mWidgetWidth, mWidgetHeight + 20.0f);
	mpTitleText_TAOE->setFontName("Gunplay");
	mpTitleText_TAOE->setCharHeight(15);
	mpTitleText_TAOE->setSpaceWidth(5);

	sprintf(tmp, "%s_typed", uniqueName);
	mpTypedText_OE = OverlayManager::getSingleton().createOverlayElement("TextArea", tmp);
	if (!mpTypedText_OE) {
		printf("ERROR: Could not create text overlay element\n");
		return false;
	}
	mpBorderPanel->addChild(mpTypedText_OE);
	mpTypedText_TAOE = static_cast<Ogre::TextAreaOverlayElement*>(mpTypedText_OE);

	mpTypedText_TAOE->setMetricsMode(GMM_PIXELS);
	mpTypedText_TAOE->setVerticalAlignment(GVA_CENTER);
	mpTypedText_TAOE->setHorizontalAlignment(GHA_LEFT);
	mpTypedText_TAOE->setCaption(mTypedText);
	mpTypedText_TAOE->setLeft(2);
	mpTypedText_TAOE->setTop(-6);
	mpTypedText_TAOE->setDimensions(mWidgetWidth, mWidgetHeight + 20.0f);
	mpTypedText_TAOE->setFontName("Gunplay");
	//mpTypedText_TAOE->setColour(ColourValue(0.5f, 1.0f, 0.0f, 1.0f));
	mpTypedText_TAOE->setColour(ColourValue(1.0f, 0.0f, 0.0f, 1.0f));
	mpTypedText_TAOE->setCharHeight(15);
	mpTypedText_TAOE->setSpaceWidth(5);
	
	sprintf(tmp, "%s_HP", uniqueName);
	mpHighlight = OverlayManager::getSingleton().createOverlayElement("Panel", tmp);
	if (!mpHighlight) {
		printf("ERROR: Could not create highlight panel\n");
		return false;
	}
	mpBorderPanel->addChild(mpHighlight);

	mpHighlight->setMetricsMode(GMM_PIXELS);
	mpHighlight->setDimensions(mWidgetWidth, mWidgetHeight);
	mpHighlight->setMaterialName("mkiv/Button_Lit");
	mpHighlight->hide();

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	return true;
}

// free allocated mem
bool TextEntryWidget::free() {
	if (!mpContainer) {
		printf("ERROR: No widget container allocated\n");
		return false;	// no memory allocated
	}
#ifndef HACK_NOGUIFREE
	OverlayManager::getSingleton().destroyOverlayElement(mpBaseElement);
	OverlayManager::getSingleton().destroyOverlayElement(mpTypedText_OE);
	OverlayManager::getSingleton().destroyOverlayElement(mpTitleText_OE);
	OverlayManager::getSingleton().destroyOverlayElement(mpHighlight);
#endif
	mpBaseElement = NULL;
	mpContainer = NULL;
	mpBorderPanel = NULL;
	mpTypedText_OE = NULL;
	mpTypedText_TAOE = NULL;
	mpTitleText_OE = NULL;
	mpTitleText_TAOE = NULL;
	mpHighlight = NULL;

	return true;
}


// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool TextEntryWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!mpBaseElement) {
		printf("ERROR: No widget base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move widget\n");
		return false;	// no memory allocated
	}
	return true;
}

// return the element (so that it can be added to a container)
OverlayElement* TextEntryWidget::getElement() {
	return mpBaseElement;
}

bool TextEntryWidget::show() {
	if (!mpContainer) {
		printf("ERROR: No widget container allocated\n");
		return false;	// no memory allocated
	}
	mpContainer->show();
	return true;
}

bool TextEntryWidget::hide() {
	if (!mpContainer) {
		printf("ERROR: No widget container allocated\n");
		return false;	// no memory allocated
	}
	mpContainer->hide();
	return true;
}

bool TextEntryWidget::setTitleText(const char *text) {
	if (!mpTitleText_TAOE) {
		printf("ERROR: No text to edit\n");
		return false;	// no memory allocated
	}

	mpTitleText_TAOE->setCaption(text);
	mpTitleText_TAOE->setLeft(0);

	return true;
}

bool TextEntryWidget::setTypedText(const char *text) {
	if (!mpTypedText_TAOE) {
		printf("ERROR: No text to edit\n");
		return false;	// no memory allocated
	}

	strcpy(mTypedText, text);
	mpTypedText_TAOE->setCaption(mTypedText);
	mpTypedText_TAOE->setLeft(2);

	return true;
}

char* TextEntryWidget::getTypedText() {
	return mTypedText;
}


// move the position of the widget
bool TextEntryWidget::moveWidget(float xprop, float yprop) {
	mXProp = xprop;
	mYProp = yprop;
	if (!mpBaseElement) {
		printf("ERROR: No widget element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	float pixelX = (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	float pixelY = (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	mXPos = mParentBoundLeft + pixelX - mWidgetWidth /2;	// centre Widget on this position
	mYPos = mParentBoundTop + pixelY - mWidgetHeight /2;	// centre Widget on this position
	mpBorderPanel->setLeft(mXPos - mParentBoundLeft);	// panel is offsetting display of Widgets as well
	mpBorderPanel->setTop(mYPos - mParentBoundTop); // so counter this by subtracting this offset from logical position

	return true;
}

// change state to highlighted based on mouse over etc etc
bool TextEntryWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	if (!mpBaseElement) {
		printf("ERROR: No widget element allocated\n");
		return false;	// no memory allocated
	}

	mClicked = false;

	if (!mActive) {
		if (mHighlighted) {
			mHighlighted = false;
			// remove highlight effect
			mpHighlight->hide();
		}
		return true;
	}

	// change gfx if mouse hover
	if ((((float)mousex > mXPos) && ((float)mousex < mXPos + mWidgetWidth)) && (((float)mousey > mYPos) && ((float)mousey < mYPos + mWidgetHeight))) {
		mHighlighted = true;
		// enable highlight effect
		mpHighlight->show();
		if (lmb) {
			mClicked = true;
		}
	} else {
		if (mHighlighted) {
			mHighlighted = false;
			// remove highlight effect
			mpHighlight->hide();
		}
	}

	// if being typed into capture and represent text
	if (mSelected) {
		strcpy(mTypedText, g.mInputHandler.getCapturedText());
		strcat(mTypedText, "\0"); // make sure captured text is properly terminated as well
		// add cursor indicating that we are typing here
		char tmp[1024];
		strcpy(tmp, mTypedText);
		strcat(tmp, "_");
		mpTypedText_TAOE->setCaption(tmp);
	}

	return true;
}

// return if widget is being hovered over with the mouse
bool TextEntryWidget::isHighlighted() {
	return mHighlighted;
}

// return if widget is being clicked on
bool TextEntryWidget::isClicked() {
	return mClicked;
}

// return if widget is in selected state
bool TextEntryWidget::isSelected() {
	return mSelected;
}

// set if selected or not and change graphics to suit
void TextEntryWidget::setSelected(bool selected) {
	mSelected = selected;
	if (!mSelected) {
		mpTypedText_TAOE->setCaption(mTypedText);
	}
}

void TextEntryWidget::deactivate() {
	mActive = false;
}

void TextEntryWidget::activate() {
	mActive = true;
}

