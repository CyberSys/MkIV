#include "LabelWidget.h"

LabelWidget::LabelWidget() {
	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
	mXProp = -1.0f;
	mYProp = -1.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;
	mCharHeight = 16;
	strcpy(mText, "Label");
	strcpy(mElementName, "whyareyoutormentingme");
	mInitialised = false;
}

bool LabelWidget::init(OverlayContainer* pContainer, const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, const char* text) {
	if (!pContainer) {
		printf("ERROR: no parent container given to LabelWidget::init()\n");
		return false;
	}
	mXProp = xprop;
	mYProp = yprop;
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	// start the other elements
	sprintf(mElementName, "%s_TA", uniqueName);
	OverlayElement* baseElement = OverlayManager::getSingleton().createOverlayElement("TextArea", mElementName);
	if (!baseElement) {
		printf("ERROR: Could not create label text\n");
		return false;
	}
	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(baseElement);
	textPtr = static_cast<Ogre::TextAreaOverlayElement*>(baseElement);
	mInitialised = true;

	textPtr->setMetricsMode(GMM_PIXELS);
	strcpy(mText, text);
	textPtr->setCaption(text);
	textPtr->setFontName("Gunplay");
	textPtr->setCharHeight(mCharHeight);
	textPtr->setSpaceWidth(0.5 * mCharHeight);
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom); // provide boundaries and move widget into place
	pContainer->addChild(baseElement);

	return true;
}

// free allocated mem
bool LabelWidget::free() {
	if (!mInitialised) {
		printf("ERROR: LabelWidget not initialised so can not free()\n");
		return false;	// no memory allocated
	}

	OverlayManager::getSingleton().destroyOverlayElement(mElementName);
	strcpy(mElementName, "");
	strcpy(mText, "");
	mInitialised = false;

	return true;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool LabelWidget::defineParentBounds(float left, float top, float right, float bottom) {	
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!moveWidget(mXProp, mYProp)) {
		printf("ERROR: LabelWidget::defineParentBounds() failed to move label\n");
		return false;	// no memory allocated
	}
	
	return true;
}

bool LabelWidget::show() {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::show() widget not initialised\n");
		return false;
	}

	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	textPtr->show();

	return true;
}

bool LabelWidget::hide() {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::hide() widget not initialised\n");
		return false;
	}

	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	textPtr->hide();

	return true;
}

// move the position of the label
bool LabelWidget::moveWidget(float xprop, float yprop) {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::moveWidget() widget not initialised\n");
		return false;
	}

	mXProp = xprop;
	mYProp = yprop;
	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	mXPos = (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	mYPos = (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	textPtr->setLeft(mXPos);
	textPtr->setTop(mYPos);

	return true;
}

bool LabelWidget::setAlignment(const char* top, const char* left) {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::setAlignment() widget not initialised\n");
		return false;
	}

	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));

	// multi-dialect spell checking
	if (strcmp(top,"centre") == 0) {
		textPtr->setVerticalAlignment(GVA_CENTER);
	} else if (strcmp(top,"center") == 0) {
		textPtr->setVerticalAlignment(GVA_CENTER);
	} else if (strcmp(top,"bottom") == 0) {
		textPtr->setVerticalAlignment(GVA_BOTTOM);
	} else {
		textPtr->setVerticalAlignment(GVA_TOP);
	}
	if (strcmp(left,"centre") == 0) {
		textPtr->setHorizontalAlignment(GHA_CENTER);
	} else if (strcmp(left,"center") == 0) {
		textPtr->setHorizontalAlignment(GHA_CENTER);
	} else if (strcmp(left,"right") == 0) {
		textPtr->setHorizontalAlignment(GHA_RIGHT);
	} else {
		textPtr->setHorizontalAlignment(GHA_LEFT);
	}

	return true;
}

bool LabelWidget::setColour(float r, float g, float b, float a) {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::setAlignment() widget not initialised\n");
		return false;
	}

	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	
	ColourValue cv(r,g,b,a);
	textPtr->setColour(cv);

	return true;
}

bool LabelWidget::setFont(const char* fontName) {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::setAlignment() widget not initialised\n");
		return false;
	}

	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	textPtr->setFontName(fontName);
	return true;
}

bool LabelWidget::setSize(int charHeight) {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::setAlignment() widget not initialised\n");
		return false;
	}

	mCharHeight = charHeight;
	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	textPtr->setCharHeight(charHeight);
	return true;
}

bool LabelWidget::centreText() {
	if (!mInitialised) {
		printf("ERROR: LabelWidget::setAlignment() widget not initialised\n");
		return false;
	}

	int capLen = strlen(mText);
	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	textPtr->setLeft(mXPos + capLen/2 * (-0.5 * mCharHeight));
	return true;
}

bool LabelWidget::setText(const char *text) {
	if (!mInitialised) {
		printf("ERROR: LabelWidget not initialised, can not set text\n");
		return false;
	}
	TextAreaOverlayElement* textPtr = static_cast<Ogre::TextAreaOverlayElement*>(OverlayManager::getSingleton().getOverlayElement(mElementName));
	strcpy(mText, text);
	textPtr->setCaption(text);

	return true;
}

char* LabelWidget::getText() {
	return mText;
}

