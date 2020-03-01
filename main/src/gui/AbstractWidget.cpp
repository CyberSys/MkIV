#include "AbstractWidget.h"

// set default values
AbstractWidget::AbstractWidget() {
	mpBaseElement = NULL;
	mpContainer = NULL;
	mParentBoundTop = 0.0f;
	mParentBoundLeft = 0.0f;
	mParentBoundBottom = 480.0f;
	mParentBoundRight = 640.0f;	
	mXProp = -1.0f;
	mYProp = -1.0f;
	mXPos = 100.0f;
	mYPos = 100.0f;
	mWidgetWidth = 100.0f;
	mWidgetHeight = 100.0f;
	mVisible = true;
	mActive = true;
}

// return the element (so that it can be added to a container)
OverlayElement* AbstractWidget::getBaseElement() {
	return mpBaseElement;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool AbstractWidget::resize(float top, float left, float bottom, float right) {
	mParentBoundTop = top;
	mParentBoundLeft = left;
	mParentBoundBottom = bottom;
	mParentBoundRight = right;
	
	if (!mpBaseElement) {
		printf("ERROR: No widget base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp, mYProp)) {
		printf("ERROR: failed to move widget\n");
		return false;	// no memory allocated
	}
	return true;
}

// move the position of the widget
bool AbstractWidget::moveWidget(float xprop, float yprop) {
	mXProp = xprop;
	mYProp = yprop;
	if (!mpBaseElement) {
		printf("ERROR: No widget base element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	mXPos = (mParentBoundRight - mParentBoundLeft) * xprop;	// get screen-proportionate position
	mYPos = (mParentBoundBottom - mParentBoundTop) * yprop;	// get screen-proportionate position

	return true;
}

