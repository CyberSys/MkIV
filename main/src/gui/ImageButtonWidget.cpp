#include "ImageButtonWidget.h"
#include "../handyMacros.h"

// constructor - looks like buttonwidget constructor is also called, so no point adding more code
ImageButtonWidget::ImageButtonWidget() {
//	pOverlay = NULL;
//	pBaseElement = NULL;
//	pContainer = NULL;

//	pButtonText_OE = NULL;
//	pButtonText_TAOE = NULL;
	
	strcpy(mOverlayHandle, "");
	strcpy(mOverlayElementHandle, "");
	strcpy(mTextAreaHandle, "");

	mHighlighted = false;
	mClicked = false;
	mSelected = false;
	mToggleType = false;
	mActive = true;	// becomes false either by mode or if obscured

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;

	mXProp = -1.0f;
	mYProp = -1.0f;
	mWidgetWidth = 120.0f;
	mWidgetHeight = 140.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;

	strcpy(mNormalMaterial, "mkiv/Button_Back");
	strcpy(mSelectedMaterial, "mkiv/Button_Selected");
}

// initialise button widget's internal data
bool ImageButtonWidget::init(const char* uniqueName, float xProp, float yProp, float parentLeft, float parentTop, float parentRight, float parentBottom, bool toggleType, float widgetWidth, float widgetHeight) {
	// arrange bits and pieces a bit differently to button widget
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mToggleType = toggleType;
	mXProp = xProp;
	mYProp = yProp;
	mWidgetWidth = widgetWidth;
	mWidgetHeight = widgetHeight;

	// initialise the overlay for the menu
	sprintf(mOverlayHandle, "%s_overlay", uniqueName);
	Ogre::Overlay* overlay = OverlayManager::getSingleton().create(mOverlayHandle);
	if (!overlay) {
		printf("ERROR: Could not create widget overlay\n");
		return false;
	}
	overlay->setZOrder(625);	// set render order (0-650 are valid)

	// create a container panel for the button
	sprintf(mOverlayElementHandle, "%s_BP", uniqueName);	// name of the panel
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", mOverlayElementHandle);
	Ogre::OverlayContainer* overlayContainer = static_cast<Ogre::OverlayContainer*>(overlayElement); // this is a weakness in the Ogre code that it has to be cast like this just to get a container -very poor program design
	if (!overlayContainer) {
		printf("ERROR: Could not create image button container\n");
		return false;
	}

	// set up the border panel (with another horrible cast)
	Ogre::BorderPanelOverlayElement* borderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(overlayElement);
	
	borderPanel->setMetricsMode(GMM_PIXELS);
	borderPanel->setBorderSize(0.0f);	// the border indicates if button is highlighted or not
	borderPanel->setMaterialName(mNormalMaterial);	// this is the image used for the button
	borderPanel->setBorderMaterialName("mkiv/ButtonBorder");	// this is the border image
	borderPanel->setDimensions(mWidgetWidth, mWidgetHeight);
	overlay->add2D(overlayContainer);
	// start the other elements
	sprintf(mTextAreaHandle, " %s_TA", uniqueName);
	Ogre::OverlayElement* buttonElement = OverlayManager::getSingleton().createOverlayElement("TextArea", mTextAreaHandle);
	if (!buttonElement) {
		printf("ERROR: Could not create image button buttonElement\n");
		return false;
	}
	borderPanel->addChild(buttonElement);
	Ogre::TextAreaOverlayElement* buttonText_TAOE = static_cast<Ogre::TextAreaOverlayElement*>(buttonElement);

	buttonText_TAOE->setMetricsMode(GMM_PIXELS);
	buttonText_TAOE->setVerticalAlignment(GVA_BOTTOM);
	buttonText_TAOE->setHorizontalAlignment(GHA_CENTER);
	buttonText_TAOE->setCaption("");
	int capLen = strlen("");
	buttonText_TAOE->setLeft(capLen/2 * -8);
	buttonText_TAOE->setTop(0);
	buttonText_TAOE->setDimensions(mWidgetWidth, mWidgetHeight + 20.0f);
	buttonText_TAOE->setFontName("Gunplay");
	buttonText_TAOE->setCharHeight(20);
	buttonText_TAOE->setSpaceWidth(5);
	ColourValue cv(0.0, 0.2, 0.0, 1.0);
	buttonText_TAOE->setColour(cv);

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	return true;
}

// free internal data resources
bool ImageButtonWidget::free() {
#ifndef HACK_NOGUIFREE
	Ogre::Overlay* overlay = Ogre::OverlayManager::getSingleton().getByName(mOverlayHandle);
	if (!overlay) {
		printf("ERROR could not fetch overlay\n");
		printf(WHERESTR "\n", WHEREARG);
	}
	Ogre::OverlayElement* oe = Ogre::OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!oe) {
		printf("ERROR could not fetch overlayelement\n");
		printf(WHERESTR "\n", WHEREARG);
	}
	Ogre::OverlayManager::getSingleton().destroyOverlayElement(oe);
	Ogre::OverlayElement* oeta = Ogre::OverlayManager::getSingleton().getOverlayElement(mTextAreaHandle);
	if (!oeta) {
		printf("ERROR could not fetch overlayelement text area\n");
		printf(WHERESTR "\n", WHEREARG);
	}
	Ogre::OverlayManager::getSingleton().destroyOverlayElement(oeta);
	Ogre::OverlayManager::getSingleton().destroy(overlay);
#endif
	return true;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool ImageButtonWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move button\n");
		return false;	// no memory allocated
	}
	return true;
}


// return the element (so that it can be added to a container)
OverlayElement* ImageButtonWidget::getElement() {
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	return overlayElement;
}


bool ImageButtonWidget::show() {
	Ogre::Overlay* overlay = OverlayManager::getSingleton().getByName(mOverlayHandle);
	if (!overlay) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	overlay->show();
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	overlayElement->show();
	//pBaseElement->show();
	//pContainer->show();
	//pBorderPanel->show();
	return true;
}

bool ImageButtonWidget::hide() {
	Ogre::Overlay* overlay = OverlayManager::getSingleton().getByName(mOverlayHandle);
	if (!overlay) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	overlay->hide();
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	overlayElement->hide();
	//pContainer->hide();
	//pBorderPanel->hide();
	return true;
}

bool ImageButtonWidget::setText(const char *text) {
	Ogre::OverlayElement* buttonElement = OverlayManager::getSingleton().getOverlayElement(mTextAreaHandle);
	if (!buttonElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	Ogre::TextAreaOverlayElement* buttonText_TAOE = static_cast<Ogre::TextAreaOverlayElement*>(buttonElement);
	
	buttonText_TAOE->setCaption(text);
	int capLen = strlen(text);
	buttonText_TAOE->setLeft(capLen / 2 * -7.5);

	return true;
}

bool ImageButtonWidget::setMaterials(const char* normalMaterial, const char* selectedMaterial) {
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	Ogre::BorderPanelOverlayElement* borderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(overlayElement);
	strcpy(mNormalMaterial, normalMaterial);
	strcpy(mSelectedMaterial, selectedMaterial);
	if (mSelected) {
		borderPanel->setMaterialName(selectedMaterial);
	} else {
		borderPanel->setMaterialName(mNormalMaterial);	// this is the image used for the button
	}
	return true;
}

// move the position of the button
bool ImageButtonWidget::moveWidget(float xprop, float yprop) {
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	Ogre::BorderPanelOverlayElement* borderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(overlayElement);

	mXProp = xprop;
	mYProp = yprop;
	// set position - can't just change metrics mode because all the size attributes are in pixels	
	float pixelX = (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	float pixelY = (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	mXPos = mParentBoundLeft + pixelX - mWidgetWidth / 2;	// centre button on this position
	mYPos = mParentBoundTop + pixelY;	// centre button on this position
	borderPanel->setLeft(mXPos - mParentBoundLeft);	// panel is offsetting display of buttons as well
	borderPanel->setTop(mYPos - mParentBoundTop); // so counter this by subtracting this offset from logical position

	return true;
}

// change state to highlighted based on mouse over etc etc
bool ImageButtonWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	Ogre::BorderPanelOverlayElement* borderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(overlayElement);

	mClicked = false;

	if (!mActive) {
		if (mHighlighted) {
			mHighlighted = false;
			// remove highlight effect
			borderPanel->setBorderSize(0.0f);
		}
		return true;
	}

	// change gfx if mouse hover
	if ((((float)mousex > mXPos) && ((float)mousex < mXPos + mWidgetWidth)) && (((float)mousey > mYPos) && ((float)mousey < mYPos + mWidgetHeight))) {
		mHighlighted = true;
		// enable highlight effect
		borderPanel->setBorderSize(5.0f);
		if (lmb) {
			if (mToggleType) {
				mSelected = !mSelected;
				if (mSelected) {
					// switch material to the selected one
					borderPanel->setMaterialName(mSelectedMaterial);
				} else {
					borderPanel->setMaterialName(mNormalMaterial);
				}
			}
			mClicked = true;
		}
	} else {
		mHighlighted = false;
		// remove highlight effect
		if (!mSelected) {
			borderPanel->setBorderSize(0.0f);
		}
	}

	return true;
}

// return if button is being hovered over with the mouse
bool ImageButtonWidget::isHighlighted() {
	return mHighlighted;
}

// return if button is being clicked on
bool ImageButtonWidget::isClicked() {
	return mClicked;
}

// return if button is in selected state
bool ImageButtonWidget::isSelected() {
	return mSelected;
}

// set if selected or not and change graphics to suit
bool ImageButtonWidget::setSelected(bool selected) {
	Ogre::OverlayElement* overlayElement = OverlayManager::getSingleton().getOverlayElement(mOverlayElementHandle);
	if (!overlayElement) {
		printf("ERROR: ");printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	Ogre::BorderPanelOverlayElement* borderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(overlayElement);
	
	mSelected = selected;
	if (mSelected) {
		// switch material to the selected one
		borderPanel->setMaterialName(mSelectedMaterial);
		borderPanel->setBorderSize(5.0f);
	} else {
		borderPanel->setMaterialName(mNormalMaterial);
	}
	return true;
}

void ImageButtonWidget::deactivate() {
	mActive = false;
}

void ImageButtonWidget::activate() {
	mActive = true;
}
