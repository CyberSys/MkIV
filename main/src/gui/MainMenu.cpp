#include "MainMenu.h"
#include "../globals.h"

// set defaults (mostly for later validation)
MainMenu::MainMenu() {
	pOverlay = NULL;
	pBaseElement = NULL;
	pContainer = NULL;
	pPanel = NULL;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;

	mXProp = -1.0f;
	mYProp = -1.0f;
	mWidgetWidth = 120.0f;
	mWidgetHeight = 30.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;
}

// init a new main menu - must have a unique name to gel with Ogre's retarded overlay system
bool MainMenu::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop) {
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mXProp = xprop;
	mYProp = yprop;
printf("debug: main menu init\n");
	// initialise the overlay for the menu
	char tmp[256];
	sprintf(tmp,"%s_overlay",uniqueName);
	pOverlay = OverlayManager::getSingleton().create(tmp);
	if (!pOverlay) {
		printf("ERROR: Could not create main menu overlay\n");
		return false;
	}
	pOverlay->setZOrder(500);	// set render order (0-650 are valid)

printf("debug: mainm olay\n");
	// create the background (this can be semi transparent)
	sprintf(tmp,"%s_panel",uniqueName);
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("Panel", tmp);
	if (!pBaseElement) {
		printf("ERROR: Could not create menu container\n");
		return false;
	}
printf("debug: mainm based\n");
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	// set up the border panel (with another horrible cast)
printf("debug: container\n");
	pPanel = static_cast<Ogre::PanelOverlayElement*>(pBaseElement);
printf("debug: static panel\n");
	pPanel->setMetricsMode(GMM_PIXELS);
printf("debug: metrics\n");
	pPanel->setMaterialName("mkiv/Intro_Background");
printf("debug: panel\n");
	mWidgetWidth = parentRight - parentLeft;
	mWidgetHeight = parentBottom - parentTop;
	pPanel->setDimensions(mWidgetWidth, mWidgetHeight);	// expand to fill whole of parent
	pOverlay->add2D(pContainer);

printf("debug: main menu add cont\n");
	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	sprintf(tmp,"%s_butt_Quit",uniqueName);
	if (!mQuitButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.8f, 0.8f, "Quit")) {
		printf("ERROR: Could not init menu button\n");
		return false;
	}
	pContainer->addChild(mQuitButton.getElement());

	sprintf(tmp,"%s_butt_Solo",uniqueName);
	if (!mSoloButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.2f, 0.2f, "Solo")) {
		printf("ERROR: Could not init menu button\n");
		return false;
	}
	pContainer->addChild(mSoloButton.getElement());

	sprintf(tmp,"%s_butt_Server",uniqueName);
	if (!mServerButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.2f, 0.3f, "Server")) {
		printf("ERROR: Could not init menu button\n");
		return false;
	}
	pContainer->addChild(mServerButton.getElement());

	sprintf(tmp,"%s_butt_Client",uniqueName);
	if (!mClientButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.2f, 0.4f, "Client")) {
		printf("ERROR: Could not init menu button\n");
		return false;
	}
	pContainer->addChild(mClientButton.getElement());

	sprintf(tmp,"%s_butt_ScenEditor",uniqueName);
	if (!mScenEditorButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.8f, 0.2f, "Editor")) {
		printf("ERROR: Could not init menu button\n");
		return false;
	}
	pContainer->addChild(mScenEditorButton.getElement());

	if (!mLogoImage.init("mkiv/logo", "mkiv/LogoMat", 400, 140, 0.0f, 0.0f, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom)) {
		printf("ERROR: Could not init logo image\n");
		return false;
	}
	mLogoImage.setAlignment("top","centre");
	pContainer->addChild(mLogoImage.getElement());

	if (!mVersionText.init(pContainer, "versionText", mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.0f, -0.03f, VERSION)) {
		printf("ERROR: Could not init version text\n");
		return false;
	}
	//pContainer->addChild(mVersionText.getElement());
	mVersionText.setAlignment("bottom","left");	
	mVersionText.setColour(0.8f,0.0f,0.0f,1.0f);

	if (!mTextEntryWidget.init("mkiv/IPEntry", mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.0f, -0.03f)) {
		printf("ERROR: Could not init text entry widget\n");
		return false;
	}
	pContainer->addChild(mTextEntryWidget.getElement());
	
printf("debug: end of main menu init\n");
	return true;
}

// free resources
bool MainMenu::free() {
	if (!pOverlay) {
		printf("ERROR: main menu overlay does not exist\n");
		return false;
	}
	mQuitButton.free();
	mSoloButton.free();
	mServerButton.free();
	mClientButton.free();
	mScenEditorButton.free();
	mVersionText.free();
	mTextEntryWidget.free();
#ifndef HACK_NOGUIFREE
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	OverlayManager::getSingleton().destroy(pOverlay);
#endif
	printf("DB: Main Menu freed\n");
	pBaseElement = NULL;
	pContainer = NULL;
	pPanel = NULL;
	pOverlay = NULL;

	return true;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool MainMenu::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No MainMenu base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move MainMenu\n");
		return false;	// no memory allocated
	}
	return true;
}

// move the position of the MainMenu
bool MainMenu::moveWidget(float xprop, float yprop) {
	mXProp = xprop;
	mYProp = yprop;
	if (!pBaseElement) {
		printf("ERROR: No MainMenu base element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	mXPos = mParentBoundLeft + (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	pPanel->setLeft(mXPos - mParentBoundLeft);
	pPanel->setTop(mYPos - mParentBoundTop);

	return true;
}

bool MainMenu::show() {
	if (!pOverlay) {
		printf("ERROR: main menu overlay does not exist\n");
		return false;
	}
	pOverlay->show();

	return true;
}

bool MainMenu::hide() {
	if (!pOverlay) {
		printf("ERROR: main menu overlay does not exist\n");
		return false;
	}
	pOverlay->hide();

	return true;
}

bool MainMenu::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	mQuitButton.update(mousex, mousey, lmb);
	mSoloButton.update(mousex, mousey, lmb);
	mServerButton.update(mousex, mousey, lmb);
	mClientButton.update(mousex, mousey, lmb);
	mScenEditorButton.update(mousex, mousey, lmb);

	return true;
}

bool MainMenu::isQuitClicked() {
	return mQuitButton.isClicked();
}

bool MainMenu::isStartSoloClicked() {
	return mSoloButton.isClicked();
}

bool MainMenu::isHostServerClicked() {
	return mServerButton.isClicked();
}

bool MainMenu::isJoinServerClicked() {
	return mClientButton.isClicked();
}

bool MainMenu::isScenEditorClicked() {
	return mScenEditorButton.isClicked();
}
