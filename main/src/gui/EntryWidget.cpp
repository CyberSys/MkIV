#include "EntryWidget.h"

EntryWidget::EntryWidget() {
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;

	pButtonText_OE = NULL;
	pButtonText_TAOE = NULL;
	pHighlight = NULL;

	mHighlighted = false;
	mClicked = false;
	mSelected = false;
	mToggleType = true;
	mActive = true;	// becomes false either by mode or if obscured

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;

	mXProp = -1.0f;
	mYProp = -1.0f;
	mButtonWidth = 120.0f;
	mButtonHeight = 15.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;
}

bool EntryWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, const char* text, bool toggleType) {
mToggleType = toggleType;
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
	pBorderPanel->setBorderSize(0.0f);
	pBorderPanel->setMaterialName("mkiv/Button_Back");
	pBorderPanel->setDimensions(mButtonWidth,mButtonHeight);
	pBorderPanel->setLeft(mXPos);
	pBorderPanel->setTop(mYPos);

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
	pButtonText_TAOE->setVerticalAlignment(GVA_TOP);
	pButtonText_TAOE->setHorizontalAlignment(GHA_LEFT);
	pButtonText_TAOE->setCaption(text);
	pButtonText_TAOE->setLeft(2.0f);
	pButtonText_TAOE->setTop(0);
	pButtonText_TAOE->setDimensions(mButtonWidth,mButtonHeight + 0.0f);
	pButtonText_TAOE->setFontName("Gunplay");
	pButtonText_TAOE->setCharHeight(15);
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

	moveWidget(mXProp,mYProp);

	return true;
}

bool EntryWidget::setText(const char *text) {
	if (!pButtonText_TAOE) {
		printf("ERROR: No text to edit\n");
		return false;	// no memory allocated
	}

	// shorten length to 19 chars
	char shortenedText[20];
	int index = 0;
	for (index = 0; index < 19; index++) {
		shortenedText[index] = text[index];
	}
	shortenedText[index] = '\0';

	pButtonText_TAOE->setCaption(shortenedText);

	return true;
}


