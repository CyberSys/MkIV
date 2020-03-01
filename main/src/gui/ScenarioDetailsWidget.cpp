#include "ScenarioDetailsWidget.h" 

ScenarioDetailsWidget::ScenarioDetailsWidget() {
	pOverlay = NULL;
	pBaseElement = NULL;
	pContainer = NULL;
	pPanel = NULL;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
	mWidgetWidthPixels = 250.0f;
	mWidgetHeightPixels = 280.0f;
	mXProp = 0.5f;
	mYProp = 0.5f;
	mXPos = 100.0f;
	mYPos = 100.0f;
}

bool ScenarioDetailsWidget::init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xProp, float yProp) {
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
	pOverlay->setZOrder(510);	// set render order (0-650 are valid)

	// create a container panel for the widget
	sprintf(tmp,"%s_SD",uniqueName);	// name of the panel
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("Panel", tmp);
	pContainer = static_cast<Ogre::OverlayContainer*>(pBaseElement);
	if (!pContainer) {
		printf("ERROR: Could not create scen details container\n");
		return false;
	}
	// set up the border panel (with another horrible cast)
	pPanel = static_cast<Ogre::PanelOverlayElement*>(pBaseElement);

	pPanel->setMetricsMode(GMM_PIXELS);
	pPanel->setMaterialName("mkiv/FileBrowserBackground");
	pPanel->setDimensions(mWidgetWidthPixels, mWidgetHeightPixels);
	pOverlay->add2D(pContainer);

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);
	
	// scenario labels (details of selected scenario)
	sprintf(tmp,"%s_label_TimeLimit",uniqueName);
	if (!mTimeLimitLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.02f, 0.05f, "Time Limit ")) {
		printf("ERROR: Could not init time label\n");
		return false;
	}
	mTimeLimitLabel.setSize(20);
	mTimeLimitLabel.setColour(1.0f,1.0f,0.7f,1.0f);
	//pContainer->addChild(mTimeLimitLabel.getElement());

	sprintf(tmp,"%s_label_ForceCount",uniqueName);
	if (!mNumberOfForcesLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.02f, 0.15f, "Number of Forces")) {
		printf("ERROR: Could not init forcecount label\n");
		return false;
	}
	mNumberOfForcesLabel.setSize(20);
	mNumberOfForcesLabel.setColour(1.0f,1.0f,0.7f,1.0f);
	//pContainer->addChild(mNumberOfForcesLabel.getElement());

	sprintf(tmp,"%s_label_Force1Name",uniqueName);
	if (!mForce1Label.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.02f, 0.2f, "Force 1 ")) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	mForce1Label.setSize(20);
	mForce1Label.setColour(1.0f,1.0f,0.7f,1.0f);
	//pContainer->addChild(mForce1Label.getElement());

	sprintf(tmp,"%s_label_Force2Name",uniqueName);
	if (!mForce2Label.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.02f, 0.25f, "Force 2")) {
		printf("ERROR: Could not init force name label\n");
		return false;
	}
	mForce2Label.setSize(20);
	mForce2Label.setColour(1.0f,1.0f,0.7f,1.0f);
	//pContainer->addChild(mForce2Label.getElement());

	return true;
}

bool ScenarioDetailsWidget::free() {
	if (!pOverlay) {
		printf("ERROR: ScenarioDetailsWidget::free() overlay does not exist\n");
		return false;
	}
	if (!pContainer) {
		printf("ERROR: No browser container allocated\n");
		return false;	// no memory allocated
	}

	mTimeLimitLabel.free();
	mNumberOfForcesLabel.free();
	mForce1Label.free();
	mForce2Label.free();
#ifndef HACK_NOGUIFREE
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	OverlayManager::getSingleton().destroy(pOverlay);
#endif
	pBaseElement = NULL;
	pContainer = NULL;
	pPanel = NULL;
	
	pOverlay = NULL;

	return true;
}


// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool ScenarioDetailsWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No scen details base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move scen details\n");
		return false;	// no memory allocated
	}
	return true;
}

bool ScenarioDetailsWidget::moveWidget(float xProp, float yProp) {
	mXProp = xProp;
	mYProp = yProp;
	if (!pBaseElement) {
		printf("ERROR: No scen details element allocated\n");
		return false;	// no memory allocated
	}

	float pixelX = (mParentBoundRight - mParentBoundLeft) * xProp;	// get screen-proportionate position
	float pixelY = (mParentBoundBottom - mParentBoundTop) * yProp;	// get screen-proportionate position
	mXPos = mParentBoundLeft + pixelX;	// centre button on this position
	mYPos = mParentBoundTop + pixelY;	// centre button on this position
	printf("xp[%f], yp[%f], pbl[%f], pbt[%f], px[%f], py[%f]\n",mXPos,mYPos,mParentBoundLeft,mParentBoundTop,pixelX,pixelY);
	pPanel->setLeft(mXPos - mParentBoundLeft);	// panel is offsetting display of buttons as well
	pPanel->setTop(mYPos - mParentBoundTop); // so counter this by subtracting this offset from logical position

	return true;
}

bool ScenarioDetailsWidget::show() {
	if (!pOverlay) {
		printf("ERROR: no pOverlay allocated to SDWidget\n");
		return false;
	}
	pOverlay->show();
	return true;
}

bool ScenarioDetailsWidget::hide() {
	if (!pOverlay) {
		printf("ERROR: no pOverlay allocated to SDWidget\n");
		return false;
	}
	pOverlay->hide();
	return true;
}

bool ScenarioDetailsWidget::updateDetails(ScenarioDetails* details) {
	if (!details) {
		printf("ERROR: no details mem allocated\n");
		return false;
	}

	char tmp[256];
	sprintf(tmp, "Time Limit: %i", details->mTimeLimitMins);
	mTimeLimitLabel.setText(tmp);
	sprintf(tmp, "Number of Forces: %i", details->mNumberOfForces);
	mNumberOfForcesLabel.setText(tmp);
	sprintf(tmp, "Force 1: %s", details->mForces[0].mName);
	mForce1Label.setText(tmp);
	sprintf(tmp, "Force 2: %s", details->mForces[1].mName);
	mForce2Label.setText(tmp);

	return true;
}


