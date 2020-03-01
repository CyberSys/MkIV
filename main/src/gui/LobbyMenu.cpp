#include "LobbyMenu.h"
#include "../globals.h"

#define WHERESTR "[file %s, func %s, line %d] "
#define WHEREARG __FILE__,__func__,__LINE__
// printf(WHERESTR ": hey, x=%d\n", WHEREARG, x);

LobbyMenu::LobbyMenu() {
	pOverlay = NULL;
	pBaseElement = NULL;							// administrative cast of the container panel
	pContainer = NULL;							// nesting cast of the container panel
	pPanel = NULL;							// display panel
	mType = 0;
	mBrowserOpen = false;

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

bool LobbyMenu::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, int type) {
	mType = type;
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mXProp = xprop;
	mYProp = yprop;

	// initialise the overlay for the menu
	char tmp[256];
	sprintf(tmp,"%s_overlay",uniqueName);
	pOverlay = OverlayManager::getSingleton().create(tmp);
	if (!pOverlay) {
		printf("ERROR: Could not create lobby menu overlay\n");
		return false;
	}
	pOverlay->setZOrder(500);	// set render order (0-650 are valid)

	// create the background (this can be semi transparent)
	sprintf(tmp,"%s_panel",uniqueName);
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("Panel", tmp);
	if (!pBaseElement) {
		printf("ERROR: Could not create lobby menu container\n");
		return false;
	}
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	// set up the border panel (with another horrible cast)
	pPanel = static_cast<Ogre::PanelOverlayElement*>(pBaseElement);
	pPanel->setMetricsMode(GMM_PIXELS);
	pPanel->setMaterialName("mkiv/Lobby_Background");
	mWidgetWidth = parentRight - parentLeft;
	mWidgetHeight = parentBottom - parentTop;
	pPanel->setDimensions(mWidgetWidth, mWidgetHeight);	// expand to fill whole of parent
	pOverlay->add2D(pContainer);

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	sprintf(tmp,"%s_btw_ScenTitle", uniqueName);
	if (!mTitleWidget.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, "Scenario Title")) {
		printf("ERROR: Could not init big title widget\n");
		return false;
	}
	pContainer->addChild(mTitleWidget.getElement());
	mTitleWidget.show();

	// init force shield widgets
	for (int i = 0; i < 5; i++) {
		sprintf(tmp,"%s_shield_%i", uniqueName, i);
		if (!mForceShield[i].init(tmp, 0.5f + ((i % 3) * 0.2f), (0.12f + 0.45 * (i / 3)), mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom)) {
			printf("ERROR: Could not init force shield %i\n", i);
			return false;
		}
		sprintf(tmp, "Force %i", i);
		mForceShield[i].setText(tmp);
		pContainer->addChild(mForceShield[i].getElement());
	}
	// set up some default shields
	for (int i = 0; i < 5; i++) {
		mForceShield[i].setMaterials("mkiv/EyeOfRa", "mkiv/EyeOfRa");
	}
	mForceShield[0].setSelected(true);

	sprintf(tmp,"%s_butt_Cancel", uniqueName);
	if (!mCancelButton.init(tmp,  mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.9f, 0.7f, "Disconnect")) {
		printf("ERROR: Could not init lobby button\n");
		return false;
	}
	if (type == 0) {
		mCancelButton.setText("Cancel");
	}
	pContainer->addChild(mCancelButton.getElement());
	
	if (type != 0) {	// network games but not solo
		sprintf(tmp,"%s_butt_Ready",uniqueName);
		if (!mReadyButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.9f, 0.6f, "Ready", true)) {
			printf("ERROR: Could not init lobby button\n");
			return false;
		}
		pContainer->addChild(mReadyButton.getElement());

		sprintf(tmp,"%s_label_readyCount",uniqueName);
		if (!mReadyCountLabel.init(pContainer, tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.9f, 0.9f, "Players Ready: X/Y")) {
			printf("ERROR: Could not init ready count label\n");
			return false;
		}
		//pContainer->addChild(mReadyCountLabel.getElement());
		mReadyCountLabel.setColour(1.0f,1.0f,0.7f,1.0f);
		mReadyCountLabel.setSize(20);
		mReadyCountLabel.setFont("BlueHighway");
		mReadyCountLabel.centreText();
	}

	if (type != 1) {	// solo and server only
		sprintf(tmp,"%s_butt_Start",uniqueName);
		if (!mStartButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.9f, 0.8f, "Start")) {
			printf("ERROR: Could not init lobby button\n");
			return false;
		}
		pContainer->addChild(mStartButton.getElement());

		sprintf(tmp,"%s_butt_ChooseScenario",uniqueName);
		if (!mChooseScenarioButton.init(tmp, mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.1f, 0.75f, "Scenario...")) {
			printf("ERROR: Could not init menu button\n");
			return false;
		}
		pContainer->addChild(mChooseScenarioButton.getElement());
	}

	if (!mFileBrowser.init("mkiv/LobbyFileBrowser", mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.2f, 0.3f)) {
		printf("ERROR: Could not init lobby file browser\n");
		return false;
	}
	// select a default scenario
	ScenarioDetails* scenDetails = mFileBrowser.getDetails();
	if (scenDetails == NULL) {
		printf("ERROR: Scenario Details returned were NULL\n");
		return false;
	}
	scenDetails->loadFromFile("dogsofwar.sce");
	printf("Loaded default scenario\n");

	// Display scenario details
	if (!mScenarioDetailsBox.init("mkiv/LobbyScenDetailsBox", mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom, 0.01f, 0.12f)) {
		printf("ERROR: Could not init lobby scenario details box\n");
	}
	mScenarioDetailsBox.show();

	if (!selectScenario()) {
		printf("ERROR: Failed to select scenario\n");
		return false;
	}

	return true;
}

bool LobbyMenu::free() {
	hide();
	return true;/*
	printf(WHERESTR "\n", WHEREARG);
	if (!pOverlay) {
		printf("ERROR: main menu overlay does not exist\n");
		return false;
	}
	printf(WHERESTR "\n", WHEREARG);
	if (mType != 1) {
		mStartButton.free();
		mChooseScenarioButton.free();
	}
	printf(WHERESTR "\n", WHEREARG);
	// free force shields
	for (int i = 0; i < 5; i++) {
		mForceShield[i].free();
	}
	printf(WHERESTR "\n", WHEREARG);
	mFileBrowser.free();
	mCancelButton.free();
	if (mType != 0) {
		mReadyButton.free();
		mReadyCountLabel.free();
	}	
	printf(WHERESTR "\n", WHEREARG);
	mScenarioDetailsBox.free();
	mTitleWidget.free();
#ifndef HACK_NOGUIFREE
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	OverlayManager::getSingleton().destroy(pOverlay);
#endif
	pBaseElement = NULL;
	pContainer = NULL;
	pPanel = NULL;	
	pOverlay = NULL;
	printf(WHERESTR "\n", WHEREARG);
	return true;*/
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool LobbyMenu::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No LobbyMenu base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move LobbyMenu\n");
		return false;	// no memory allocated
	}
	return true;
}

// move the position of the MainMenu
bool LobbyMenu::moveWidget(float xprop, float yprop) {
	mXProp = xprop;
	mYProp = yprop;
	if (!pBaseElement) {
		printf("ERROR: No LobbyMenu base element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	mXPos = mParentBoundLeft + (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	pPanel->setLeft(mXPos - mParentBoundLeft);
	pPanel->setTop(mYPos - mParentBoundTop);

	return true;
}

bool LobbyMenu::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	mCancelButton.update(mousex, mousey, lmb);
	if ((mType == 0) || (mType == 2)) {
		mStartButton.update(mousex, mousey, lmb);
		mChooseScenarioButton.update(mousex, mousey, lmb);
		if (mChooseScenarioButton.isClicked()) {
			mFileBrowser.scan("data/scenarios/", ".sce");
			mFileBrowser.show();
				// prevent background buttons from working while browser is open
			mChooseScenarioButton.deactivate();
			mCancelButton.deactivate();
			mReadyButton.deactivate();
			mStartButton.deactivate();
			mBrowserOpen = true;
		}
	}
	if ((mType == 1) || (mType == 2)) {
		mReadyButton.update(mousex, mousey, lmb);
	}
	if (mBrowserOpen) {
		mFileBrowser.update(mousex, mousey, lmb);	// check for changes of state in file browser
		if (mFileBrowser.isCancelClicked()) {
			// close browser, don't accept changes.
			mFileBrowser.hide();	// hide the sce chooser window
			mBrowserOpen = false;	// flag browser as closed

			// re-enable disabled buttons
			mChooseScenarioButton.activate();
			mCancelButton.activate();
			mReadyButton.activate();
			mStartButton.activate();

		} else if (mFileBrowser.isOKClicked()) {
			// close browser, and accept changes.
			mFileBrowser.hide();	// hide the sce chooser window
			mBrowserOpen = false;	// flag browser as closed
		
			// re-enable disabled buttons
			mChooseScenarioButton.activate();
			mCancelButton.activate();
			mReadyButton.activate();
			mStartButton.activate();
			if (!selectScenario()) {
				printf("ERROR: Failed to select scenario\n");
				return false;
			}
		}
	} else {
		// update force shields
		for (int i = 0; i < 5; i++) {
			mForceShield[i].update(mousex, mousey, lmb);
			if (mForceShield[i].isClicked()) {
				for (int j = 0; j < 5; j++) {
					mForceShield[j].setSelected(false);
				}
				mForceShield[i].setSelected(true);
				// TODO: notify change of force to server and to globals
				// TODO change observer to 0?
				g.mOurForce = i - 1;
			}
		}
	}

	return true;
}


			// 1. check if a force selected
/*			if ((mouseY > 100) && (mouseY < 200)) {	// top row of shields
				if ((mouseX > mVPWidth - 115) && (mouseX < mVPWidth - 15)) {	// shield 0
					if (g.mIsServer) {
						g.mOurForce = -1;
						g.mServer.updateForceLists();
					} else if (g.mIsClient) {
						g.mClient.sendRequestForce(-1);
					} else {
						g.mGUI.clearAllLobbyLists();	// change our name on GUI
						g.mGUI.pushLobbySpecName(g.mUserName);
						g.mOurForce = -1;
					}
				} else if ((mouseX > mVPWidth - 230) && (mouseX < mVPWidth - 130)) {	// shield 1
					if (g.mIsServer) {
						g.mOurForce = 0;
						g.mServer.updateForceLists();
					} else if (g.mIsClient) {
						g.mClient.sendRequestForce(0);
					} else {
						g.mGUI.clearAllLobbyLists();	// change our name on GUI
						g.mGUI.pushLobbyF0Name(g.mUserName);
						g.mOurForce = 0;
					}
				} else if ((mouseX > mVPWidth - 345) && (mouseX < mVPWidth - 245)) {	// shield 2
					if (g.mIsServer) {
						g.mOurForce = 1;
						g.mServer.updateForceLists();
					} else if (g.mIsClient) {
						g.mClient.sendRequestForce(1);
					} else {
						g.mGUI.clearAllLobbyLists();	// change our name on GUI
						g.mGUI.pushLobbyF1Name(g.mUserName);
						g.mOurForce = 1;
					}
				}
			}*/
///////

bool LobbyMenu::show() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to show\n");
		return false;
	}
	pOverlay->show();
	return true;
}

bool LobbyMenu::hide() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to hide\n");
		return false;
	}
	pOverlay->hide();
	mTitleWidget.hide();
	mStartButton.hide();
	mCancelButton.hide();
	mReadyButton.hide();
	mChooseScenarioButton.hide();
	for (int i = 0; i < 5; i ++) {
		mForceShield[i].hide();
	}	
	mReadyCountLabel.hide();
	mFileBrowser.hide();
	mScenarioDetailsBox.hide();
	return true;
}

bool LobbyMenu::isCancelClicked() {
	return mCancelButton.isClicked();
}

bool LobbyMenu::isStartClicked() {
	return mStartButton.isClicked();
}

bool LobbyMenu::isReadyClicked() {
	return mReadyButton.isClicked();
}

bool LobbyMenu::isChooseScenarioClicked() {
	return mChooseScenarioButton.isClicked();
}

bool LobbyMenu::updateReadyCounter(int readyCount, int notReadyCount) {
	mReadyCount = readyCount;
	mNotReadyCount = notReadyCount;
	char tmp[100];
	sprintf(tmp,"Players Ready: %i/%i", readyCount, notReadyCount + readyCount);
	if (!mReadyCountLabel.setText(tmp)) {
			printf("ERROR: Could not init ready count label\n");
			return false;
	}

	return true;
}

ScenarioDetails* LobbyMenu::getSelectedScenarioDetails() {
	return mFileBrowser.getDetails();
}

// closes top-level open dialogue. If none open then returns false.
bool LobbyMenu::closeAnyMenus() {
	if (mBrowserOpen) {
		// close browser, don't accept changes.
		mFileBrowser.hide();	// hide the sce chooser window
		mBrowserOpen = false;	// flag browser as closed
		// re-enable disabled buttons
		mChooseScenarioButton.activate();
		mCancelButton.activate();
		mReadyButton.activate();
		mStartButton.activate();

		return true;
	}	

	return false;
}

// get scenario from filebrowser as selected
bool LobbyMenu::selectScenario() {
	// TODO get details before and check if null for validation to prevent segfault
	ScenarioDetails* scenDetails = mFileBrowser.getDetails();
	if (!scenDetails) {
		printf("ERROR: scenario details fetched were null\n");
		return false;
	}
	mTitleWidget.setText(scenDetails->mTitle);	// change the main title to reflect new scenario
	mScenarioDetailsBox.updateDetails(scenDetails);	// update details box w/ new scenario details
	for (int i = 0; i < 5; i++) { //rename forces and change images for force shields
		if (i == 0) {	// spectator force
			mForceShield[i].setText("Observers\n");
			mForceShield[i].activate();
			mForceShield[i].show();
		} else if (i <= scenDetails->mNumberOfForces) {
			if (i == 4) {	// used forces
				printf("WARNING: only 4 forces currently supported in lobby\n");
				break;
			}
			if (!mForceShield[i].setText(scenDetails->mForces[i - 1].mName)) {
				printf("ERROR: Could not set name for force shield %i\n", i);
				return false;
			}
			mForceShield[i].setMaterials(scenDetails->mForces[i - 1].mShieldMaterial, scenDetails->mForces[i - 1].mShieldMaterial);
			mForceShield[i].activate();
			mForceShield[i].show();
		} else {	// unused forces
			mForceShield[i].deactivate();
			mForceShield[i].hide();
		}
	}
	
	// send new scenario to clients
	if (g.mIsServer) {
		if (!g.mServer.sendScenarioDetailsToAll(scenDetails->mFilename)) {
			printf("ERROR: Could not send scenario details to clients\n");
			return false;
		}
	}
	return true;
}

bool LobbyMenu::useGlobalScenario() {
	if (!mFileBrowser.pSelectedScenarioDetails->loadFromFile(g.mScenario)) {
		printf("ERROR: LobbyMenu::useGlobalScenario() could not set scenario\n");
		return false;
	}
	if (!selectScenario()) {
		printf("ERROR: LobbyMenu::useGlobalScenario() could update gui with scen details\n");
		return false;
	}
	return true;
}

