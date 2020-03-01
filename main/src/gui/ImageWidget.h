#ifndef MARKIV_IMAGE_WIDGET_H_
#define MARKIV_IMAGE_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePanelOverlayElement.h"

using namespace Ogre;

class ImageWidget {
public:
	ImageWidget();
	bool init(const char* uniqueName, const char* materialName, float picWidth, float picHeight, float xProp, float yProp, float parentLeft, float parentTop, float parentRight, float parentBottom);
	bool free();
	OverlayElement* getElement();
	bool show();
	bool hide();
	bool setAlignment(const char* top, const char* left);
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool defineParentBounds(float left, float top, float right, float bottom); // tell this widget what the parent's dimensions are. Needs to be called if parent moved.
private:
	OverlayElement* pBaseElement;							// administrative cast of the container panel
	PanelOverlayElement* pPanel;							// display panel
	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	float mXProp;
	float mYProp;
	float mXPos;
	float mYPos;
	float mPicWidth;
	float mPicHeight;
};

#endif
