#include "FileBrowserWidget.h"

FileBrowserWidget::FileBrowserWidget() {
	pOverlay = NULL;
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
	mWidgetWidthPixels = 500.0f;
	mWidgetHeightPixels = 300.0f;
	mXProp = 0.5f;
	mYProp = 0.5f;
	mXPos = 100.0f;
	mYPos = 100.0f;
	mNumberPages = 0;
	mCurrentPage = 0;
	mSelectedEntryIndex = -1;
	mSelectedEntryOnPage = -1;

	pFileScanner = NULL;
	pSelectedScenarioDetails = NULL;

	strcpy(mSelectedEntry,"");
	strcpy(mPrefix,"");
	strcpy(mPostfix,"");
}

bool FileBrowserWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xProp, float yProp) {
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
		printf("ERROR: Could not create FileBrowserWidget overlay\n");
		return false;
	}
	pOverlay->setZOrder(600);	// set render order (0-650 are valid)

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

	// allocate mem to the file scanner
	pFileScanner = new FileScanner;
	pSelectedScenarioDetails = new ScenarioDetails;

	sprintf(tmp, "%s_butt_OK", uniqueName);
	if (!mOKButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.87f, 0.94f, "OK")) {
		printf("ERROR: Could not init OK button\n");
		return false;
	}
	pContainer->addChild(mOKButton.getElement());

	sprintf(tmp, "%s_butt_Cancel", uniqueName);
	if (!mCancelButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.62f, 0.94f, "Cancel")) {
		printf("ERROR: Could not init OK button\n");
		return false;
	}
	pContainer->addChild(mCancelButton.getElement());

	sprintf(tmp, "%s_butt_NextPage", uniqueName);
	if (!mNextPageButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.38f, 0.94f, "Next Page >")) {
		printf("ERROR: Could not init NextPage button\n");
		return false;
	}
	pContainer->addChild(mNextPageButton.getElement());

	sprintf(tmp,"%s_butt_PrevPage",uniqueName);
	if (!mPrevPageButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.13f, 0.94f, "< Prev Page")) {
		printf("ERROR: Could not init PrevPage button\n");
		return false;
	}
	pContainer->addChild(mPrevPageButton.getElement());

	sprintf(tmp,"%s_label_ScenTitle",uniqueName);
	if (!mScenarioTitleLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.6f, 0.05f, "No Scenario Selected")) {
		printf("ERROR: Could not init Scen Title label\n");
		return false;
	}
	mScenarioTitleLabel.centreText();
	mScenarioTitleLabel.setSize(25);
	mScenarioTitleLabel.setColour(1.0f, 1.0f, 0.7f, 1.0f);
	//pContainer->addChild(mScenarioTitleLabel.getElement());

	sprintf(tmp,"%s_label_TimeLimit",uniqueName);
	if (!mTimeLimitLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.3f, 0.15f, "Time Limit ")) {
		printf("ERROR: Could not init time label\n");
		return false;
	}
	//pContainer->addChild(mTimeLimitLabel.getElement());
	sprintf(tmp,"%s_label_ForceCount",uniqueName);
	if (!mNumberOfForcesLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.3f, 0.25f, "Number of Forces")) {
		printf("ERROR: Could not init forcecount label\n");
		return false;
	}
	//pContainer->addChild(mNumberOfForcesLabel.getElement());
	sprintf(tmp,"%s_label_Force1Name",uniqueName);
	if (!mForce1Label.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.3f, 0.35f, "Force 1 ")) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	//pContainer->addChild(mForce1Label.getElement());
	sprintf(tmp,"%s_label_Force2Name",uniqueName);
	if (!mForce2Label.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.3f, 0.45f, "Force 2")) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	//pContainer->addChild(mForce2Label.getElement());

	// start the loader entries
	for (int i = 0; i < 15; i++) {
		sprintf(tmp,"%s_entry_%i",uniqueName,i);
		float increment = (float)i * 0.055f;
		if (!mCurrentEntry[i].init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.15f, 0.08f + increment, "Entry")) {
			printf("ERROR: Could not init Entry button\n");
			return false;
		}
		pContainer->addChild(mCurrentEntry[i].getElement());
	}

	return true;
}

bool FileBrowserWidget::free() {
	if (!pOverlay) {
		printf("ERROR: FileBrowserWidget::free() overlay does not exist\n");
		return false;
	}
	if (!pContainer) {
		printf("ERROR: No browser container allocated\n");
		return false;	// no memory allocated
	}

	if (!pFileScanner) {
		printf("ERROR: No file scanner allocated\n");
		return false;
	}
	delete pFileScanner;
	pFileScanner = NULL;

	for (int i = 0; i < 15; i++) {
		mCurrentEntry[i].free();
	}
	mTimeLimitLabel.free();
	mNumberOfForcesLabel.free();
	mForce1Label.free();
	mForce2Label.free();
	mScenarioTitleLabel.free();
	mOKButton.free();
	mCancelButton.free();
	mNextPageButton.free();
	mPrevPageButton.free();
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
bool FileBrowserWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No file browser base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move file browser\n");
		return false;	// no memory allocated
	}
	return true;
}

bool FileBrowserWidget::moveWidget(float xProp, float yProp) {
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

// scan a new directory for a type of file, and populate entries
bool FileBrowserWidget::scan(const char* subdir, const char* extension, bool recursive) {
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
		printf("ERROR: No file browser element allocated\n");
		return false;	// no memory allocated
	}

	strcpy(mPrefix, subdir);
	strcpy(mPostfix, extension);

	// count num of pages required
	int fileMatchesCount = pFileScanner->getFileCount();
	// count how many pages this fills
	mNumberPages = fileMatchesCount / 15;
	if ((fileMatchesCount % 15) > 0) {
		mNumberPages++;
	}

	// populate first page
	if (!populatePage(0)) {
		printf("ERROR: populating File Browser page\n");
		return false;
	}

	return true;
}

bool FileBrowserWidget::populatePage(int pageNumber) {
	if (!pBaseElement) {
		printf("ERROR: No file browser element allocated\n");
		return false;	// no memory allocated
	}
	if (!pFileScanner) {
		printf("ERROR: no file scanner allocated\n");
		return false;
	}
	if (mNumberPages < pageNumber) {
		printf("ERROR: Page number out of range\n");
		return false;
	}

	mCurrentPage = pageNumber;
	int pageStart = 0 + (15 * pageNumber);
	int pageEnd = 15 * (pageNumber + 1);
	int currentEntryNum = 0;
	int numEntriesThisPage = 15;

	// find out if partially-filled page
	if (pageNumber == mNumberPages - 1) {
		int fileMatchesCount = pFileScanner->getFileCount();
		numEntriesThisPage = fileMatchesCount % 15;
		if ((numEntriesThisPage == 0) && (fileMatchesCount > 0)) {
			numEntriesThisPage = 15;
		}
	}

	for (int i = pageStart; i < pageEnd; i++) {
		// update text
		char* entryString;
		entryString = pFileScanner->getFileName(i);
		if (!mCurrentEntry[currentEntryNum].setText(entryString)) {
			printf("ERROR: Could not set entry text\n");
			return false;
		}

		// make sure not selected (unless was previously selected, of course)
		if ((mSelectedEntryOnPage != pageNumber) || (mSelectedEntryIndex != currentEntryNum)) {
			mCurrentEntry[currentEntryNum].setSelected(false);
		} else {
			mCurrentEntry[currentEntryNum].setSelected(true);
		}

		// check if have put all entries for this page down already
		currentEntryNum++;
		if (numEntriesThisPage == currentEntryNum) {
			break;
		}
	}

	// hide unused entries
	for (int i = 0; i < 15; i++) {
		if (i < numEntriesThisPage) {
			mCurrentEntry[i].show();
			mCurrentEntry[i].activate();
		} else {
			mCurrentEntry[i].hide();
			mCurrentEntry[i].deactivate();
		}
	}

	// deal with next and prev page buttons
	if (mCurrentPage < mNumberPages - 1) {
		mNextPageButton.show();
		mNextPageButton.activate();
	} else {
		mNextPageButton.hide();
		mNextPageButton.deactivate();
	}

	if (mCurrentPage > 0) {
		mPrevPageButton.show();
		mPrevPageButton.activate();
	} else {
		mPrevPageButton.hide();
		mPrevPageButton.deactivate();
	}

	return true;
}

// update the widget - this function should be run every frame. returns false on error
bool FileBrowserWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	if (!pSelectedScenarioDetails) {
		printf("ERROR: updating file browser -no scenario details alloc\n");
		return false;
	}
 // changes display if next/prev clicked
	mOKButton.update(mousex,mousey,lmb);
	mCancelButton.update(mousex,mousey,lmb);
	mNextPageButton.update(mousex,mousey,lmb);
	if (mNextPageButton.isClicked()) {
		populatePage(mCurrentPage + 1);		
	}
	mPrevPageButton.update(mousex,mousey,lmb);
	if (mPrevPageButton.isClicked()) {
		populatePage(mCurrentPage - 1);		
	}
	for (int i = 0; i < 15; i++) {
		mCurrentEntry[i].update(mousex,mousey,lmb);
		if (mCurrentEntry[i].isClicked()) {
			if (mCurrentEntry[i].isSelected()) {
				// deselect older selection
				if ((mSelectedEntryIndex != -1) && (mSelectedEntryIndex != i)){
					mCurrentEntry[mSelectedEntryIndex].setSelected(false);
				}

				// indicate current selection
				mSelectedEntryIndex = i;
				mSelectedEntryOnPage = mCurrentPage;

				// update selected entry name (internal record)
				int scannerIndexOfEntry = mSelectedEntryOnPage * 15 + mSelectedEntryIndex;
				strcpy(mSelectedEntry, pFileScanner->getFileName(scannerIndexOfEntry));
				strcat(mSelectedEntry, mPostfix);

				// updateLabels() with selected item's details
				pSelectedScenarioDetails->loadFromFile(mSelectedEntry);
				if (!mScenarioTitleLabel.setText(pSelectedScenarioDetails->mTitle)) {
					printf("ERROR: could not set label text\n");
					return false;
				}
				int time = pSelectedScenarioDetails->mTimeLimitMins;
				char tmp[100];
				sprintf(tmp,"Time Limit: %i",time);
				if (!mTimeLimitLabel.setText(tmp)) {
					printf("ERROR: could not set label text\n");
					return false;
				}
				int numForces = pSelectedScenarioDetails->mNumberOfForces;
				sprintf(tmp,"Number of Forces: %i",numForces);
				if (!mNumberOfForcesLabel.setText(tmp)) {
					printf("ERROR: could not set label text\n");
					return false;
				}
				sprintf(tmp,"Force 1: %s", pSelectedScenarioDetails->mForces[0].mName);
				if (!mForce1Label.setText(tmp)) {
					printf("ERROR: could not set label text\n");
					return false;
				}
				sprintf(tmp,"Force 2: %s", pSelectedScenarioDetails->mForces[1].mName);
				if (!mForce2Label.setText(tmp)) {
					printf("ERROR: could not set label text\n");
					return false;
				}
			} else {
				// disallow from deselecting
				mCurrentEntry[mSelectedEntryIndex].setSelected(true);
			}
		}
	}

	return true;
}

char* FileBrowserWidget::getSelectedEntryName(){
	return mSelectedEntry;
}

bool FileBrowserWidget::show() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to show\n");
		return false;
	}
	pOverlay->show();
	return true;
}

bool FileBrowserWidget::hide() {
	if (!pOverlay) {
		printf("ERROR: No Panel in memory to hide\n");
		return false;
	}
	pOverlay->hide();
	return true;
}

bool FileBrowserWidget::isOKClicked() {
	return mOKButton.isClicked();
}

bool FileBrowserWidget::isCancelClicked() {
	return mCancelButton.isClicked();
}

ScenarioDetails* FileBrowserWidget::getDetails() {
	return pSelectedScenarioDetails;
}

