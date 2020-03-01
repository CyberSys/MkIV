#include "MeshBrowserWidget.h"

MeshBrowserWidget::MeshBrowserWidget() {
	pSceneManager = NULL;
	pGUImesh = NULL;
	pOverlay = NULL;
	pBaseElement = NULL;
	pContainer = NULL;
	pBorderPanel = NULL;
	pFileScanner = NULL;
	mpPreviewMeshNode = NULL;
	strcpy(mPrefix, "");
	strcpy(mPostfix, "");
	strcpy(mSelectedEntry, "");
	strcpy(mUniqueName, "");
	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
	mWidgetWidthPixels = 800.0f;
	mWidgetHeightPixels = 550.0f;
	mXProp = 0.5f;
	mYProp = 0.5f;
	mXPos = 100.0f;
	mYPos = 100.0f;
	mNumberPages = 0;
	mCurrentPage = 0;
	mSelectedEntryIndex = -1;
	mSelectedEntryOnPage = -1;
	mEntityCount = 0;
	mGUImeshAllocated = false;
}

bool MeshBrowserWidget::init(const char* uniqueName, float parentLeft, float parentRight, float parentTop, float parentBottom, float xProp, float yProp, SceneManager* sm) {
	if (!sm) {
		printf("ERROR SceneManager NULL given to MeshBrowserWidget::init\n");
		return false;
	}
	strcpy(mUniqueName, uniqueName);
	pSceneManager = sm;
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mXProp = xProp;
	mYProp = yProp;
	char tmp[256];
	sprintf(tmp, "%s_ovrlay", uniqueName);
	pOverlay = OverlayManager::getSingleton().create(tmp); // initialise the overlay for the menu
	if (!pOverlay) {
		printf("ERROR: Could not create MeshBrowserWidget::init overlay\n");
		return false;
	}
	pOverlay->setZOrder(600);	// set render order (0-650 are valid)
	sprintf(tmp, "%s_UB", uniqueName);	// name of the panel
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement); // create a container panel for the widget
	if (!pContainer) {
		printf("ERROR: MeshBrowserWidget::init Could not create browser container\n");
		return false;
	}
	pBorderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(pBaseElement); // set up the border panel (with another horrible cast)
	pBorderPanel->setMetricsMode(GMM_PIXELS);
	pBorderPanel->setBorderSize(1.0f);
	pBorderPanel->setMaterialName("mkiv/FileBrowserBackground");
	pBorderPanel->setBorderMaterialName("mkiv/ButtonBorder");
	pBorderPanel->setDimensions(mWidgetWidthPixels, mWidgetHeightPixels);
	pOverlay->add2D(pContainer);
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom); // provide boundaries and move widget into place
	pFileScanner = new FileScanner; // allocate mem to the file scanner
	sprintf(tmp, "%s_butt_OK", uniqueName);
	if (!mOKButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.87f, 0.94f, "OK")) {
		printf("ERROR: MeshBrowserWidget::init Could not init OK button\n");
		return false;
	}
	pContainer->addChild(mOKButton.getElement());
	sprintf(tmp, "%s_butt_NextPage", uniqueName);
	if (!mNextPageButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.08f, 0.5f, "Next Page >")) {
		printf("ERROR: MeshBrowserWidget::init Could not init NextPage button\n");
		return false;
	}
	pContainer->addChild(mNextPageButton.getElement());
	sprintf(tmp,"%s_butt_PrevPage",uniqueName);
	if (!mPrevPageButton.init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.08f, 0.04f, "< Prev Page")) {
		printf("ERROR: MeshBrowserWidget::init Could not init PrevPage button\n");
		return false;
	}
	pContainer->addChild(mPrevPageButton.getElement());
	sprintf(tmp,"%s_label_ScenTitle",uniqueName);
	if (!mDesignationLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.5f, 0.05f, "No Scenario Selected")) {
		printf("ERROR: MeshBrowserWidget::init Could not init mDesignationLabel label\n");
		return false;
	}
	mDesignationLabel.centreText();
	mDesignationLabel.setSize(25);
	mDesignationLabel.setColour(1.0f, 1.0f, 0.7f, 1.0f);

	for (int i = 0; i < 15; i++) { // start the loader entries
		sprintf(tmp, "%s_entry_%i", uniqueName, i);
		float increment = (float)i * 16.0f / parentBottom; // 0.055f;
		if (!mCurrentEntry[i].init(tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 70.0f / parentRight, 0.08f + increment, "Entry")) {
			printf("ERROR: MeshBrowserWidget::init Could not init Entry button\n");
			return false;
		}
		pContainer->addChild(mCurrentEntry[i].getElement());
	}
	sprintf(tmp, "%s_snode", uniqueName);
	mpPreviewMeshNode = new SceneNode(sm, tmp); // create scene node (not managed by scenemgr)
	if (!mpPreviewMeshNode) {
		printf("ERROR MeshBrowserWidget::init couldn't create SceneNode\n");
		return false;
	}
	pOverlay->add3D(mpPreviewMeshNode);
	if (!initDetails()) {
		printf("ERROR: MeshBrowserWidget::init couldn't init concrete module\n");
	}
	return true;
}

bool MeshBrowserWidget::free() {
	if (!pSceneManager) {
		printf("ERROR: MeshBrowserWidget::free() pSceneManager is NULL\n");
		return false;
	}
	if (!pOverlay) {
		printf("ERROR: MeshBrowserWidget::free() overlay does not exist\n");
		return false;
	}
	if (!pContainer) {
		printf("ERROR: MeshBrowserWidget::free No browser container allocated\n");
		return false;	// no memory allocated
	}
	if (!pFileScanner) {
		printf("ERROR: MeshBrowserWidget::free No file scanner allocated\n");
		return false;
	}
	if (!freeDetails()) {
		printf("ERROR: MeshBrowserWidget::free could not freedetails()\n");
		return false;
	}
	if (mpPreviewMeshNode != NULL) {
		delete mpPreviewMeshNode;
		mpPreviewMeshNode = NULL;
	}
	delete pFileScanner;
	pFileScanner = NULL;
	for (int i = 0; i < 15; i++) {
		mCurrentEntry[i].free();
	}
	mDesignationLabel.free();
	mOKButton.free();
	mNextPageButton.free();
	mPrevPageButton.free();
	pOverlay->clear(); // clear of all attached items
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

bool MeshBrowserWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;
	if (!pBaseElement) {
		printf("ERROR: MeshBrowserWidget::defineParentBounds No file browser base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: MeshBrowserWidget::defineParentBounds failed to move file browser\n");
		return false;	// no memory allocated
	}
	return true;
}

bool MeshBrowserWidget::scan(const char* subdir, const char* extension, bool recursive) {
	if (!pFileScanner) { // start the scanner
		printf("ERROR: MeshBrowserWidget::scan no file scanner allocated\n");
		return false;
	}
	if (!pFileScanner->scan(subdir, extension, recursive)) {
		printf("ERROR: MeshBrowserWidget::scan file scanner failed\n");
		return false;
	}
	if (!pBaseElement) {
		printf("ERROR: MeshBrowserWidget::scan No file browser element allocated\n");
		return false;	// no memory allocated
	}
	strcpy(mPrefix, subdir);
	strcpy(mPostfix, extension);
	int fileMatchesCount = pFileScanner->getFileCount(); // count num of pages required
	mNumberPages = fileMatchesCount / 15; // count how many pages this fills
	printf("DB: there are %i filematches and this takes %i pages\n", fileMatchesCount, mNumberPages);
	if ((fileMatchesCount % 15) > 0) {
		mNumberPages++;
	}
	if (!populatePage(0)) { // populate first page
		printf("ERROR: MeshBrowserWidget::hide() populating File Browser page\n");
		return false;
	}
	return true;
}

bool MeshBrowserWidget::show() {
	if (!pOverlay) {
		printf("ERROR: MeshBrowserWidget::hide() No Panel in memory to show\n");
		return false;
	}
	pOverlay->show();
	pContainer->show();
	return true;
}

bool MeshBrowserWidget::hide() {
	if (!pOverlay) {
		printf("ERROR: MeshBrowserWidget::hide() No Panel in memory to hide\n");
		return false;
	}
	pOverlay->hide();
	return true;
}

bool MeshBrowserWidget::moveWidget(float xProp, float yProp) {
	mXProp = xProp;
	mYProp = yProp;
	if (!pBaseElement) {
		printf("ERROR: No file browser element allocated\n");
		return false;	// no memory allocated
	}
	mXPos = mParentBoundLeft + (mParentBoundRight - mParentBoundLeft) * xProp;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yProp;	// get screen-proportionate position
	pBorderPanel->setLeft(mXPos - mParentBoundLeft);
	pBorderPanel->setTop(mYPos - mParentBoundTop);
	return true;
}

bool MeshBrowserWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {
	mOKButton.update(mousex,mousey,lmb);
	mNextPageButton.update(mousex,mousey,lmb);
	if (mNextPageButton.isClicked()) { // changes display if next/prev clicked
		populatePage(mCurrentPage + 1);		
	}
	mPrevPageButton.update(mousex,mousey,lmb);
	if (mPrevPageButton.isClicked()) { // changes display if next/prev clicked
		populatePage(mCurrentPage - 1);		
	}
	mpPreviewMeshNode->yaw(Radian(0.001f));
	for (int i = 0; i < 15; i++) {
		mCurrentEntry[i].update(mousex,mousey,lmb);
		if (mCurrentEntry[i].isClicked()) {
			if (mCurrentEntry[i].isSelected()) {
				if ((mSelectedEntryIndex != -1) && (mSelectedEntryIndex != i)){ // deselect older selection
					mCurrentEntry[mSelectedEntryIndex].setSelected(false);
				}
				mSelectedEntryIndex = i; // indicate current selection
				mSelectedEntryOnPage = mCurrentPage;
				if (!displaySelectedDetails()) {
					printf("ERROR: MeshBrowserWidget::update could not display selected item's details\n");
					return false;
				}
				pGUImesh->setCastShadows(false); // set to cast shadows
				pGUImesh->setRenderQueueGroup(RENDER_QUEUE_OVERLAY + 1);
				pGUImesh->setVisible(true);
				mpPreviewMeshNode->setPosition(-0.3, 0, -2); // put it in front of the camera		
				// OGRE 1.70: doesn't have this function
				//mpPreviewMeshNode->getMaterial()->setDepthCheckEnabled(false); 
				mpPreviewMeshNode->setVisible(true);
				mpPreviewMeshNode->_update(true, true);
				mpPreviewMeshNode->attachObject(pGUImesh);
			} else { // disallow from deselecting
				mCurrentEntry[mSelectedEntryIndex].setSelected(true);
			}
		}
	}
	return true;
}

bool MeshBrowserWidget::populatePage(int pageNumber) {
	printf("DB: populating page %i\n", pageNumber);
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
	if (pageNumber == mNumberPages - 1) { // find out if partially-filled page
		int fileMatchesCount = pFileScanner->getFileCount();
		numEntriesThisPage = fileMatchesCount % 15;
		if ((numEntriesThisPage == 0) && (fileMatchesCount > 0)) {
			numEntriesThisPage = 15;
		}
	}
	printf("DB: pagestart %i pageEnd %i\n", pageStart, pageEnd);
	for (int i = pageStart; i < pageEnd; i++) { // update text
		char* entryString;
		entryString = pFileScanner->getFileName(i);
		if (!mCurrentEntry[currentEntryNum].setText(entryString)) {
			printf("ERROR: Could not set entry text\n");
			return false;
		}
		if ((mSelectedEntryOnPage != pageNumber) || (mSelectedEntryIndex != currentEntryNum)) { // make sure not selected (unless was previously selected, of course)
			mCurrentEntry[currentEntryNum].setSelected(false);
		} else {
			mCurrentEntry[currentEntryNum].setSelected(true);
		}
		currentEntryNum++; // check if have put all entries for this page down already
		if (numEntriesThisPage == currentEntryNum) {
			break;
		}
	}
	for (int i = 0; i < 15; i++) { // hide unused entries
		if (i < numEntriesThisPage) {
			printf("DB: showing button %i///\n", i);
			mCurrentEntry[i].show();
			mCurrentEntry[i].activate();
		} else {
			mCurrentEntry[i].hide();
			mCurrentEntry[i].deactivate();
		}
	}
	if (mCurrentPage < mNumberPages - 1) { // deal with next and prev page buttons
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

char* MeshBrowserWidget::getSelectedEntryName() {
	return mSelectedEntry;
}

bool MeshBrowserWidget::isOKClicked() {
	return mOKButton.isClicked();
}

