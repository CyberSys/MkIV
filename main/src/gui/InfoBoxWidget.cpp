#include "InfoBoxWidget.h"

InfoBoxWidget::InfoBoxWidget() {
	pOverlay = NULL;
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
	mWidgetWidthPixels = 300.0f;
	mWidgetHeightPixels = 100.0f;
	mXProp = 0.5f;
	mYProp = 0.5f;
	mXPos = 0.0f;
	mYPos = 0.0f;
	mIsShowing = false;
}

bool InfoBoxWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xProp, float yProp, float width, float height) {
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mXProp = xProp;
	mYProp = yProp;
	mWidgetWidthPixels = width;
	mWidgetHeightPixels = height;

	// initialise the overlay for the menu
	char tmp[256];
	sprintf(tmp,"%s_overlay",uniqueName);
	pOverlay = OverlayManager::getSingleton().create(tmp);
	if (!pOverlay) {
		printf("ERROR: Could not create widget overlay\n");
		return false;
	}
	pOverlay->setZOrder(650);	// set render order (0-650 are valid)

	// create a container panel for the widget
	sprintf(tmp,"%s_FB",uniqueName);	// name of the panel
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	if (!pContainer) {
		printf("ERROR: Could not create browser container\n");
		return false;
	}
	// set up the border panel (with another horrible cast)
	pBorderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(pBaseElement);

	pBorderPanel->setMetricsMode(GMM_PIXELS);
	pBorderPanel->setBorderSize(1.0f);
	pBorderPanel->setMaterialName("mkiv/FileBrowserBackground");
	pBorderPanel->setBorderMaterialName("mkiv/ButtonBorder");
	pBorderPanel->setDimensions(mWidgetWidthPixels, mWidgetHeightPixels);
	pOverlay->add2D(pContainer);

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);
	sprintf(tmp, "%s_label_Text", uniqueName);
	if (!mText.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.05f, 0.05f, "Would you like a backrub?")) {
		printf("ERROR: Could not init Text label\n");
		return false;
	}
	// - mParentBoundTopmQuestionText.centreText();
	mText.setSize(15);
	mText.setColour(1.0f, 1.0f, 0.7f, 1.0f);
	//pContainer->addChild(mText.getElement());

	return true;
}

bool InfoBoxWidget::free() {
	if (!pOverlay) {
		printf("ERROR: InfoBoxWidget::free() overlay does not exist\n");
		return false;
	}
	if (!pContainer) {
		printf("ERROR: No browser container allocated\n");
		return false;	// no memory allocated
	}
	mText.free();
#ifndef HACK_NOGUIFREE
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	OverlayManager::getSingleton().destroy(pOverlay);
#endif
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;
	pOverlay = NULL;
	
	return true;
}

bool InfoBoxWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No file browser base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp, mYProp)) {
		printf("ERROR: failed to move file browser\n");
		return false;	// no memory allocated
	}
	return true;
}


bool InfoBoxWidget::show() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to show\n");
		return false;
	}
	pOverlay->show();
	mIsShowing = true;
	return true;
}

bool InfoBoxWidget::hide() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to hide\n");
		return false;
	}
	pOverlay->hide();
	mIsShowing = false;
	return true;
}

bool InfoBoxWidget::moveWidget(float xProp, float yProp) {
	mXProp = xProp;
	mYProp = yProp;
	if (!pBaseElement) {
		printf("ERROR: No file browser element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	mXPos = mParentBoundLeft + (mParentBoundRight - mParentBoundLeft) * xProp;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yProp;	// get screen-proportionate position
	pBorderPanel->setLeft(mXPos - mParentBoundLeft);
	pBorderPanel->setTop(mYPos - mParentBoundTop);

	return true;
}

bool InfoBoxWidget::isShowing() {
	return mIsShowing;
}

void InfoBoxWidget::setText(const char* text) {
	mText.setText(text);
}

