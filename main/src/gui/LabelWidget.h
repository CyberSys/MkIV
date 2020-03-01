#ifndef MARKIV_LABEL_WIDGET_H_
#define MARKIV_LABEL_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreTextAreaOverlayElement.h" 

using namespace Ogre;

class LabelWidget {
public:
	LabelWidget();
	bool init(OverlayContainer* pContainer, const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, const char* text);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool show();
	bool hide();
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool setAlignment(const char* top, const char* left);
	bool setColour(float r, float g, float b, float a);
	bool setSize(int charHeight);
	bool setFont(const char* fontName);
	bool centreText();
	bool setText(const char *text);
	char* getText();
private:
	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	float mXProp;
	float mYProp;
	float mXPos;
	float mYPos;
	char mElementName[256];
	char mText[2056];
	int mCharHeight;
	bool mInitialised;
};

#endif
