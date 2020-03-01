#include "ButtonWidget.h"
#include "../audio/AudioHandler.h"

// set defaults
ButtonWidget::ButtonWidget() {
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;

	pButtonText_OE = NULL;
	pButtonText_TAOE = NULL;
	pHighlight = NULL;

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
	mButtonWidth = 120.0f;
	mButtonHeight = 30.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;
}

// allocate memory for the button
bool ButtonWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, const char* text, bool toggleType) {
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mToggleType = toggleType;
	mXProp = xprop;
	mYProp = yprop;

	// create a container panel for the button
	char tmp[256];
	sprintf(tmp,"%s_BP",uniqueName);	// name of the panel
	// this is a weakness in the Ogre code that it has to be cast like this just to get a container -very poor program design
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	if (!pContainer) {
		printf("ERROR: Could not create button container\n");
		return false;
	}

	// set up the border panel (with another horrible cast)
	pBorderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(pBaseElement);
	
	pBorderPanel->setMetricsMode(GMM_PIXELS);
	pBorderPanel->setBorderSize(2.0f);
	pBorderPanel->setMaterialName("mkiv/Button_Back");
	pBorderPanel->setBorderMaterialName("mkiv/ButtonBorder");
	pBorderPanel->setDimensions(mButtonWidth,mButtonHeight);

	// start the other elements
	sprintf(tmp,"%s_TA",uniqueName);
	pButtonText_OE = OverlayManager::getSingleton().createOverlayElement("TextArea", tmp);
	if (!pButtonText_OE) {
		printf("ERROR: Could not create button text\n");
		return false;
	}
	pBorderPanel->addChild(pButtonText_OE);
	pButtonText_TAOE = static_cast<Ogre::TextAreaOverlayElement*>(pButtonText_OE);

	pButtonText_TAOE->setMetricsMode(GMM_PIXELS);
	pButtonText_TAOE->setVerticalAlignment(GVA_CENTER);
	pButtonText_TAOE->setHorizontalAlignment(GHA_CENTER);
	pButtonText_TAOE->setCaption(text);
	int capLen = strlen(text);
	pButtonText_TAOE->setLeft(capLen/2 * -7.5);
	pButtonText_TAOE->setTop(-10);
	pButtonText_TAOE->setDimensions(mButtonWidth,mButtonHeight + 20.0f);
	pButtonText_TAOE->setFontName("Gunplay");
	pButtonText_TAOE->setCharHeight(20);
	pButtonText_TAOE->setSpaceWidth(5);
	
	sprintf(tmp,"%s_HP",uniqueName);
	pHighlight = OverlayManager::getSingleton().createOverlayElement("Panel", tmp);
	if (!pHighlight) {
		printf("ERROR: Could not create highlight panel\n");
		return false;
	}
	pBorderPanel->addChild(pHighlight);

	pHighlight->setMetricsMode(GMM_PIXELS);
	pHighlight->setDimensions(mButtonWidth,mButtonHeight);
	pHighlight->setMaterialName("mkiv/Button_Lit");
	pHighlight->hide();

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	return true;
}

// free allocated mem
bool ButtonWidget::free() {
#ifndef HACK_NOGUIFREE
	if (!pContainer) {
		printf("ERROR: No button container allocated\n");
		return false;	// no memory allocated
	}
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	OverlayManager::getSingleton().destroyOverlayElement(pButtonText_OE);
	OverlayManager::getSingleton().destroyOverlayElement(pHighlight);
#endif	
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;
	pButtonText_OE = NULL;
	pButtonText_TAOE = NULL;
	pHighlight = NULL;

	return true;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool ButtonWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No button base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp, mYProp)) {
		printf("ERROR: failed to move button\n");
		return false;	// no memory allocated
	}
	return true;
}

// return the element (so that it can be added to a container)
OverlayElement* ButtonWidget::getElement() {
	return pBaseElement;
}

bool ButtonWidget::show() {
	if (!pContainer) {
		printf("ERROR: No button container allocated\n");
		return false;	// no memory allocated
	}
	pContainer->show();
	return true;
}

bool ButtonWidget::hide() {
	if (!pContainer) {
		printf("ERROR: No button container allocated\n");
		return false;	// no memory allocated
	}
	pContainer->hide();
	return true;
}

bool ButtonWidget::setText(const char *text) {
	if (!pButtonText_TAOE) {
		printf("ERROR: No text to edit\n");
		return false;	// no memory allocated
	}

	pButtonText_TAOE->setCaption(text);
	int capLen = strlen(text);
	pButtonText_TAOE->setLeft(capLen/2 * -7.5);

	return true;
}

// move the position of the button
bool ButtonWidget::moveWidget(float xprop, float yprop) {
	mXProp = xprop;
	mYProp = yprop;
	if (!pBaseElement) {
		printf("ERROR: No button element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	float pixelX = (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	float pixelY = (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	mXPos = mParentBoundLeft + pixelX - mButtonWidth / 2;	// centre button on this position
	mYPos = mParentBoundTop + pixelY - mButtonHeight / 2;	// centre button on this position
	pBorderPanel->setLeft(mXPos - mParentBoundLeft); // panel is offsetting display of buttons as well
	pBorderPanel->setTop(mYPos - mParentBoundTop); // so counter this by subtracting this offset from logical position

	return true;
}

// change state to highlighted based on mouse over etc etc
bool ButtonWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	if (!pBaseElement) {
		printf("ERROR: No button element allocated\n");
		return false;	// no memory allocated
	}

	mClicked = false;

	if (!mActive) {
		if (mHighlighted) {
			mHighlighted = false;
			// remove highlight effect
			pHighlight->hide();
		}
		return true;
	}

	// change gfx if mouse hover
	if ((((float)mousex > mXPos) && ((float)mousex < mXPos + mButtonWidth)) && (((float)mousey > mYPos) && ((float)mousey < mYPos + mButtonHeight))) {
		mHighlighted = true;
		// enable highlight effect
		pHighlight->show();
		if (lmb) {
			if (mToggleType) {
				mSelected = !mSelected;
				if (mSelected) {
					// switch material to the selected one
					pBorderPanel->setMaterialName("mkiv/Button_Selected");
				} else {
					pBorderPanel->setMaterialName("mkiv/Button_Back");
				}
			}
			mClicked = true;
			int schannel = -1;
			SoundManager::getSingleton().playSoundB(SoundManager::getSingleton().mButtonClickSoundIndex, NULL, &schannel);
		} else {
			/* TODO the Already Playing function doesn't seem to work properly
			SoundManager::getSingleton().PlaySoundB(soundIndex, Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->getRootSceneNode(), &channelIndex);*/
		}
	} else {
		if (mHighlighted) {
			mHighlighted = false;
			// remove highlight effect
			pHighlight->hide();
		}
	}

	return true;
}

// return if button is being hovered over with the mouse
bool ButtonWidget::isHighlighted() {
	return mHighlighted;
}

// return if button is being clicked on
bool ButtonWidget::isClicked() {
	return mClicked;
}

// return if button is in selected state
bool ButtonWidget::isSelected() {
	return mSelected;
}

// set if selected or not and change graphics to suit
void ButtonWidget::setSelected(bool selected) {
	if (!mToggleType) {
		return;
	}
	mSelected = selected;
	if (mSelected) {
		// switch material to the selected one
		pBorderPanel->setMaterialName("mkiv/Button_Selected");
	} else {
		pBorderPanel->setMaterialName("mkiv/Button_Back");
	}
}

void ButtonWidget::deactivate() {
	mActive = false;
}

void ButtonWidget::activate() {
	mActive = true;
}

