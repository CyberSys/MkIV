#include "BigTitleWidget.h"

// set default variable values
BigTitleWidget::BigTitleWidget() {
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;

	pWidgetText_OE = NULL;
	pWidgetText_TAOE = NULL;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;

	mWidgetWidth = 630;
	mWidgetHeight = 45;
}

// initialise widget
bool BigTitleWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, const char* text) {
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;

	// create a container panel for the widget
	char tmp[256];
	sprintf(tmp,"%s_BTW",uniqueName);	// name of the panel
	// this is a weakness in the Ogre code that it has to be cast like this just to get a container -very poor program design
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	if (!pContainer) {
		printf("ERROR: Could not create big title widget container\n");
		return false;
	}

	// set up the border panel (with another horrible cast)
	pBorderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(pBaseElement);
	
	pBorderPanel->setMetricsMode(GMM_PIXELS);
	pBorderPanel->setBorderSize(4.0f);
	pBorderPanel->setMaterialName("mkiv/Button_Back");
	pBorderPanel->setBorderMaterialName("mkiv/ButtonBorder");
	pBorderPanel->setDimensions(mWidgetWidth, mWidgetHeight);

	// start the other elements
	sprintf(tmp,"%s_TA",uniqueName);
	pWidgetText_OE = OverlayManager::getSingleton().createOverlayElement("TextArea", tmp);
	if (!pWidgetText_OE) {
		printf("ERROR: Could not create big title widget text\n");
		return false;
	}
	pBorderPanel->addChild(pWidgetText_OE);
	pWidgetText_TAOE = static_cast<Ogre::TextAreaOverlayElement*>(pWidgetText_OE);

	pWidgetText_TAOE->setMetricsMode(GMM_PIXELS);
	pWidgetText_TAOE->setVerticalAlignment(GVA_TOP);
	pWidgetText_TAOE->setHorizontalAlignment(GHA_LEFT);
	pWidgetText_TAOE->setCaption(text);
	pWidgetText_TAOE->setLeft(5);
	pWidgetText_TAOE->setTop(7);
	pWidgetText_TAOE->setDimensions(mWidgetWidth, mWidgetHeight);
	pWidgetText_TAOE->setFontName("Deutsch");
	pWidgetText_TAOE->setCharHeight(40);
	pWidgetText_TAOE->setSpaceWidth(10);
	ColourValue cv(0.9f, 0.8f, 0.2f, 1.0f);
	pWidgetText_TAOE->setColour(cv);

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	return true;
}

// free resources of widget
bool BigTitleWidget::free() {
#ifndef HACK_NOGUIFREE
	if (!pContainer) {
		printf("ERROR: No big title widget container allocated\n");
		return false;	// no memory allocated
	}
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	OverlayManager::getSingleton().destroyOverlayElement(pWidgetText_OE);
#endif
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;
	pWidgetText_OE = NULL;
	pWidgetText_TAOE = NULL;

	return true;
}

// return a pointer to the main OGRE overlay element
OverlayElement* BigTitleWidget::getElement() {
	return pBaseElement;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool BigTitleWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No widget base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(5.0f, 5.0f)) {
		printf("ERROR: failed to move widget\n");
		return false;	// no memory allocated
	}
	return true;
}

// show the widget
bool BigTitleWidget::show() {
	if (!pContainer) {
		printf("ERROR: No panel exists to adjust\n");
		return false;
	}
	pContainer->show();

	return true;
}

// hide the widget
bool BigTitleWidget::hide() {
	if (!pContainer) {
		printf("ERROR: No panel exists to adjust\n");
		return false;
	}
	pContainer->hide();

	return true;
}

// change the text of the title
bool BigTitleWidget::setText(const char *text) {
	if (!pWidgetText_TAOE) {
		printf("ERROR: No text to edit\n");
		return false;	// no memory allocated
	}

	pWidgetText_TAOE->setCaption(text);

	return true;
}

// move the position of the button
bool BigTitleWidget::moveWidget(float pixelX, float pixelY) {
	if (!pBaseElement) {
		printf("ERROR: No widget element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	pBorderPanel->setLeft(mParentBoundLeft + pixelX);
	pBorderPanel->setTop(mParentBoundTop + pixelY);

	return true;
}
