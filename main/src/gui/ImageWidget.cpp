#include "ImageWidget.h"

ImageWidget::ImageWidget() {
	pBaseElement = NULL;
	pPanel = NULL;
	mXProp = 0.0f;
	mYProp = 0.0f;
	mPicWidth = 100;
	mPicHeight = 100;
	mXPos = 0.0f;
	mYPos = 0.0f;

	mParentBoundLeft = 0.0f;
	mParentBoundRight = 640.0f;
	mParentBoundTop = 0.0f;
	mParentBoundBottom = 480.0f;
}

bool ImageWidget::init(const char* uniqueName, const char* materialName, float picWidth, float picHeight, float xProp, float yProp, float parentLeft, float parentTop, float parentRight, float parentBottom) {
	mXProp = xProp;
	mYProp = yProp;
	mParentBoundLeft = parentLeft;
	mParentBoundRight = parentRight;
	mParentBoundTop = parentTop;
	mParentBoundBottom = parentBottom;
	mPicWidth = picWidth;
	mPicHeight = picHeight;

	// create the image (this can be semi transparent)
	char tmp[256];
	sprintf(tmp,"%s_panel",uniqueName);
	pBaseElement = OverlayManager::getSingleton().createOverlayElement("Panel", tmp);
	if (!pBaseElement) {
		printf("ERROR: Could not create menu container\n");
		return false;
	}
	pPanel = static_cast<Ogre::PanelOverlayElement*>(pBaseElement);
	pPanel->setMetricsMode(GMM_PIXELS);
	pPanel->setMaterialName(materialName);
	pPanel->setDimensions(picWidth,picHeight);
	defineParentBounds(mParentBoundLeft, mParentBoundTop, mParentBoundRight, mParentBoundBottom);

	return true;
}

bool ImageWidget::free() {
	OverlayManager::getSingleton().destroyOverlayElement(pBaseElement);
	pBaseElement = NULL;
	pPanel = NULL;

	return true;
}

OverlayElement* ImageWidget::getElement() {
	return pBaseElement;
}

bool ImageWidget::show() {
	if (!pPanel) {
		printf("ERROR: No panel exists to adjust\n");
		return false;
	}
	pPanel->show();

	return true;
}

bool ImageWidget::hide() {
	if (!pPanel) {
		printf("ERROR: No panel exists to adjust\n");
		return false;
	}
	pPanel->hide();

	return true;
}

bool ImageWidget::setAlignment(const char* top, const char* left) {
	// validation
	if (!pPanel) {
		printf("ERROR: No panel exists to adjust\n");
		return false;
	}

	// multi-dialect spell checking
	if (strcmp(top,"centre") == 0) {
		pPanel->setVerticalAlignment(GVA_CENTER);
	} else if (strcmp(top,"center") == 0) {
		pPanel->setVerticalAlignment(GVA_CENTER);
	} else if (strcmp(top,"bottom") == 0) {
		pPanel->setVerticalAlignment(GVA_BOTTOM);
	} else {
		pPanel->setVerticalAlignment(GVA_TOP);
	}
	if (strcmp(left,"centre") == 0) {
		pPanel->setHorizontalAlignment(GHA_CENTER);
	} else if (strcmp(left,"center") == 0) {
		pPanel->setHorizontalAlignment(GHA_CENTER);
	} else if (strcmp(left,"right") == 0) {
		pPanel->setHorizontalAlignment(GHA_RIGHT);
	} else {
		pPanel->setHorizontalAlignment(GHA_LEFT);
	}

	return true;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool ImageWidget::defineParentBounds(float left, float top, float right, float bottom) {
	mParentBoundLeft = left;
	mParentBoundRight = right;
	mParentBoundTop = top;
	mParentBoundBottom = bottom;

	if (!pBaseElement) {
		printf("ERROR: No image base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp,mYProp)) {
		printf("ERROR: failed to move image\n");
		return false;	// no memory allocated
	}
	return true;
}

bool ImageWidget::moveWidget(float xprop, float yprop) {
	mXProp = xprop;
	mYProp = yprop;
	if (!pBaseElement) {
		printf("ERROR: No image base element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	float pixelX = (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position
	mXPos = pixelX - mPicWidth / 2;
	pPanel->setLeft(mXPos - mParentBoundLeft);
	pPanel->setTop(mYPos - mParentBoundTop);

	return true;
}
