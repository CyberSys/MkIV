#include "FileSaverWidget.h"
#include "../globals.h"
#include "../usefulFuncs.h"

#define COL_ONE 0.25 // xprop for first col of buttons

FileSaverWidget::FileSaverWidget() {
	pOverlay = NULL;
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
	mWidgetWidthPixels = 600.0f;
	mWidgetHeightPixels = 400.0f;
	mXProp = 0.25f;
	mYProp = 0.25f;
	mXPos = 100.0f;
	mYPos = 100.0f;

	pFileScanner = NULL;
	pSelectedScenarioDetails = NULL;

	strcpy(mPrefix,"");
	strcpy(mPostfix,"");

	mSelectedTextBox = -1;
	mCurrentlyTyping = false;
	mClickedToSave = false;
}

bool FileSaverWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xProp, float yProp) {
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mXProp = xProp;
	mYProp = yProp;

	// initialise the overlay for the menu
	char tmp[256];
	sprintf(tmp,"%s_overlay",uniqueName);
	pOverlay = OverlayManager::getSingleton().create(tmp);
	if (!pOverlay) {
		printf("ERROR: Could not create FileSaverWidget overlay\n");
		return false;
	}
	pOverlay->setZOrder(600);	// set render order (0-650 are valid)

	// create a container panel for the widget
	sprintf(tmp,"%s_FB",uniqueName);	// name of the panel
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	if (!pContainer) {
		printf("ERROR: Could not create Saver container\n");
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

	// allocate mem to the file scanner
	pFileScanner = new FileScanner;
	pSelectedScenarioDetails = new ScenarioDetails;

	sprintf(tmp, "%s_popup_Confirm", uniqueName);
	if (!mConfirmPopup.init(tmp, parentLeft, parentTop, parentRight, parentBottom, 0.5f, 0.5f)) { // this one is not a true child but can float around the whole screen (should really be a child of the scenario editor only but it was easier to logically control from here
		printf("ERROR: Could not init Confirm popup\n");
		return false;
	}
	mConfirmPopup.hide(); // start hidden

	sprintf(tmp,"%s_butt_OK",uniqueName);
	if (!mOKButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.87f, 0.94f, "Save")) {
		printf("ERROR: Could not init OK button\n");
		return false;
	}
	pContainer->addChild(mOKButton.getElement());

	sprintf(tmp,"%s_butt_Cancel",uniqueName);
	if (!mCancelButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.62f, 0.94f, "Cancel")) {
		printf("ERROR: Could not init OK button\n");
		return false;
	}
	pContainer->addChild(mCancelButton.getElement());

	sprintf(tmp,"%s_label_ScenTitle",uniqueName);
	if (!mScenarioTitleLabel.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, COL_ONE, 0.1f)) {
		printf("ERROR: Could not init Scen Title label\n");
		return false;
	}
	mScenarioTitleLabel.setTitleText("Scenario Title");
	mScenarioTitleLabel.setTypedText("...");
	pContainer->addChild(mScenarioTitleLabel.getElement());

	sprintf(tmp, "%s_label_TimeLimit", uniqueName);
	if (!mTimeLimitLabel.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, COL_ONE, 0.2f)) {
		printf("ERROR: Could not init time label\n");
		return false;
	}
	mTimeLimitLabel.setTitleText("Time Limit (min)");
	mTimeLimitLabel.setTypedText("60");
	pContainer->addChild(mTimeLimitLabel.getElement());

	sprintf(tmp, "%s_label_ForceCount", uniqueName);
	if (!mNumberOfForcesLabel.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, COL_ONE, 0.3f)) {
		printf("ERROR: Could not init forcecount label\n");
		return false;
	}
	mNumberOfForcesLabel.setTitleText("Number of Forces");
	mNumberOfForcesLabel.setTypedText("2");
	pContainer->addChild(mNumberOfForcesLabel.getElement());

	sprintf(tmp, "%s_label_Force1Name", uniqueName);
	if (!mForce1Label.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, COL_ONE, 0.4f)) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	mForce1Label.setTitleText("Force 1");
	mForce1Label.setTypedText("Blue");
	pContainer->addChild(mForce1Label.getElement());

	sprintf(tmp, "%s_label_Force2Name", uniqueName);
	if (!mForce2Label.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, COL_ONE, 0.5f)) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	mForce2Label.setTitleText("Force 2");
	mForce2Label.setTypedText("Red");
	pContainer->addChild(mForce2Label.getElement());

	sprintf(tmp, "%s_TEW_Filename", uniqueName);
	if (!mFilenameTEW.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, COL_ONE, 0.8f, "filename.sce")) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	//pContainer->addChild(mFilenameTEW.getElement());

	return true;
}

bool FileSaverWidget::free() {
	if (!pOverlay) {
		printf("ERROR: Saver overlay does not exist\n");
		return false;
	}
	if (!pContainer) {
		printf("ERROR: No Saver container allocated\n");
		return false;	// no memory allocated
	}

	if (!pFileScanner) {
		printf("ERROR: No file scanner allocated\n");
		return false;
	}
	delete pFileScanner;
	pFileScanner = NULL;

	mConfirmPopup.free();
	mFilenameTEW.free();
	mTimeLimitLabel.free();
	mNumberOfForcesLabel.free();
	mForce1Label.free();
	mForce2Label.free();
	mScenarioTitleLabel.free();
	mOKButton.free();
	mCancelButton.free();
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

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool FileSaverWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No file Saver base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move file Saver\n");
		return false;	// no memory allocated
	}
	return true;
}

bool FileSaverWidget::moveWidget(float xProp, float yProp) {
	mXProp = xProp;
	mYProp = yProp;
	if (!pBaseElement) {
		printf("ERROR: No file Saver element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels
	mXPos = mParentBoundLeft + (mParentBoundRight - mParentBoundLeft) * xProp;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yProp;	// get screen-proportionate position
	pBorderPanel->setLeft(mXPos - mParentBoundLeft);
	pBorderPanel->setTop(mYPos - mParentBoundTop);

	return true;
}

// scan a new directory for a type of file, and populate entries
bool FileSaverWidget::scan(const char* subdir, const char* extension, bool recursive) {
	// start the scanner
	if (!pFileScanner) {
		printf("ERROR: no file scanner allocated\n");
		return false;
	}
	if (!pFileScanner->scan(subdir, extension, recursive)) {
		printf("ERROR: file scanner failed\n");
		return false;
	}
	if (!pBaseElement) {
		printf("ERROR: No file Saver element allocated\n");
		return false;	// no memory allocated
	}

	strcpy(mPrefix, subdir);
	strcpy(mPostfix, extension);

	return true;
}

// update the widget - this function should be run every frame. returns false on error
bool FileSaverWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	// validation (that initialisation has already succeeded)
	if (!pSelectedScenarioDetails) {
		printf("ERROR: updating file Saver -no scenario details alloc\n");
		return false;
	}

	// if confirm-to-save dialogue is up then only update this
	if (mConfirmPopup.isShowing()) {
		mConfirmPopup.update(mousex, mousey, lmb); // update the popup
		if (mConfirmPopup.isOKClicked()) { // check if "Yes" clicked
			mConfirmPopup.hide(); // close popup
			mClickedToSave = true; // flag to say that whole process finished and is ready to save
		}
		if (mConfirmPopup.isCancelClicked()) { // check if "Cancel" clicked
			g.mInputHandler.startCapturingText(); // resume text capturing from previous dialogue
			mConfirmPopup.hide(); // close popup and back to file saver details menu
		}
		return true;
	}
	
	// update the mapname label
	char temp[256];
	legaliseFilename(mScenarioTitleLabel.getTypedText(), temp); // replace spaces in 'name' with underscores
	mFilenameTEW.setText(temp);

	// update other buttons and fields
	mOKButton.update(mousex, mousey, lmb); // "Save" button
	if (mOKButton.isClicked()) { // check to see if user clicked to save
		g.mInputHandler.stopCapturingText(); // stop text capturing from previous dialogue
		// make sure that we put everything in the def prior to closing
		char mapname[128];
		strcpy(mapname, mFilenameTEW.getText());
		strcpy(pSelectedScenarioDetails->mMapName, mapname); // mapname
		strcpy(pSelectedScenarioDetails->mTitle, mScenarioTitleLabel.getTypedText()); // scen title
		int time; // turn time from string to integer
		sscanf(mTimeLimitLabel.getTypedText(),"%i", &time); // time limit in minutes
		pSelectedScenarioDetails->mTimeLimitMins = time; // time limit in minutes
		int forces; // turn from string to integer
		sscanf(mNumberOfForcesLabel.getTypedText(),"%i", &forces); // number of involved forces
		pSelectedScenarioDetails->mNumberOfForces = forces; // number of involved forces
		strcpy(pSelectedScenarioDetails->mForces[0].mName, mForce1Label.getTypedText()); // TODO dynamicalise for more than 2 forces
		strcpy(pSelectedScenarioDetails->mForces[1].mName, mForce2Label.getTypedText()); // TODO dynamicalise for more than 2 forces
		if (pFileScanner->doesFileExist(mFilenameTEW.getText())) { // if given filename already exists popup confirmation
			char tmp[256];
			sprintf(tmp, "%s.sce already exists. \nAre you sure that you want to\noverwrite the existing file?", mapname);
			mConfirmPopup.setQuestion(tmp); // ask user about file in question
			mConfirmPopup.show(); // show popup
		} else {
			mClickedToSave = true; // otherwise just save to file
		}
	}

	mCancelButton.update(mousex, mousey, lmb); // "Cancel" button
	if (mCancelButton.isClicked()) {
		g.mInputHandler.stopCapturingText(); // stop text capturing from this widget when clicked
	}

	bool clicked = false; // checks if a new text field has been clicked into (text entry focus change)
	mScenarioTitleLabel.update(mousex, mousey, lmb); // 0
	if (mScenarioTitleLabel.isClicked()) {
		if (mSelectedTextBox != 0) {
			mSelectedTextBox = 0;
			clicked = true;
		}
	}
	mTimeLimitLabel.update(mousex, mousey, lmb); // 1
	if (mTimeLimitLabel.isClicked()) {
		if (mSelectedTextBox != 1) {
			mSelectedTextBox = 1;
			clicked = true;
		}
	}
	mNumberOfForcesLabel.update(mousex, mousey, lmb); // 2
	if (mNumberOfForcesLabel.isClicked()) {
		if (mSelectedTextBox != 2) {
			mSelectedTextBox = 2;
			clicked = true;
		}
	}
	mForce1Label.update(mousex, mousey, lmb); // 3
	if (mForce1Label.isClicked()) {
		if (mSelectedTextBox != 3) {
			mSelectedTextBox = 3;
			clicked = true;
		}
	}
	mForce2Label.update(mousex, mousey, lmb); // 4
	if (mForce2Label.isClicked()) {
		if (mSelectedTextBox != 4) {
			mSelectedTextBox = 4;
			clicked = true;
		}
	}

	if (clicked) { // clicked on a new text field
		if (mCurrentlyTyping) { // stop the current text capture so can start again for new field
			g.mInputHandler.stopCapturingText();
		}
		// clear all previous selections
		mScenarioTitleLabel.setSelected(false);
		mTimeLimitLabel.setSelected(false);
		mNumberOfForcesLabel.setSelected(false);
		mForce1Label.setSelected(false);
		mForce2Label.setSelected(false);

		g.mInputHandler.startCapturingText(); // start text capture for field
		mCurrentlyTyping = true; // indicate that we are typing (so can stop driving the camera etc).
		// apply selection
		if (mSelectedTextBox == 0) {
			mScenarioTitleLabel.setSelected(true);
		} else if (mSelectedTextBox == 1) {
			mTimeLimitLabel.setSelected(true);
		} else if (mSelectedTextBox == 2) {
			mNumberOfForcesLabel.setSelected(true);
		} else if (mSelectedTextBox == 3) {
			mForce1Label.setSelected(true);
		} else if (mSelectedTextBox == 4) {
			mForce2Label.setSelected(true);
		}
	}

	return true;
}

// show the widget
bool FileSaverWidget::show() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to show\n");
		return false;
	}
	pOverlay->show();
	return true;
}

// hide the widget
bool FileSaverWidget::hide() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to hide\n");
		return false;
	}
	mConfirmPopup.hide();
	mClickedToSave = false;
	mSelectedTextBox = -1;
	pOverlay->hide();
	return true;
}

// tell watcher that we are ready to save details (that "save" has been clicked and confirmed if necessary as well)
bool FileSaverWidget::isOKClicked() {
	return mClickedToSave;
}

// user has decided to close widget
bool FileSaverWidget::isCancelClicked() {
	return mCancelButton.isClicked();
}

// give the user-entered details back
ScenarioDetails* FileSaverWidget::getDetails() {
	return pSelectedScenarioDetails;
}

// use these details for text fields (i.e. if already loaded a scenario)
void FileSaverWidget::setDetails(const ScenarioDetails& details) {
	mScenarioTitleLabel.setTypedText(details.mTitle); // scen title
	char tmp[256];
	sprintf(tmp, "%i", details.mTimeLimitMins);
	mTimeLimitLabel.setTypedText(tmp);
	sprintf(tmp, "%i", details.mNumberOfForces);
	mNumberOfForcesLabel.setTypedText(tmp);
	mForce1Label.setTypedText(details.mForces[0].mName); // TODO dynamicalise for more than 2 forces
	mForce2Label.setTypedText(details.mForces[1].mName); // TODO dynamicalise for more than 2 forces
}

