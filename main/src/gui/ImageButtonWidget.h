#ifndef MARKIV_IMAGE_BUTTON_WIDGET_H_
#define MARKIV_IMAGE_BUTTON_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreBorderPanelOverlayElement.h"
#include "OgreTextAreaOverlayElement.h"

using namespace Ogre;

// generic button. uses 3 default images {basic/mouseover/selected}
class ImageButtonWidget {
public:
	ImageButtonWidget();
	bool init(const char* uniqueName, float xProp, float yProp, float parentLeft, float parentTop, float parentRight, float parentBottom, bool toggleType = false, float widgetWidth = 120.0f, float widgetHeight = 140.0f);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	OverlayElement* getElement();
	bool show();
	bool hide();
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool setText(const char *text);
	bool setMaterials(const char* normalMaterial, const char* selectedMaterial);
	bool update(unsigned int mousex, unsigned int mousey, bool lmb); // change gfx if hover
	bool isHighlighted();
	bool isClicked();
	bool isSelected();
	bool setSelected(bool selected);
	void deactivate();
	void activate();
protected:
	// this is why polymorphism sucks:
	char mOverlayHandle[128];
	char mOverlayElementHandle[128];
	char mTextAreaHandle[128];
//	Overlay* pOverlay;
//	OverlayElement* pBaseElement;							// administrative cast of the container panel
//	OverlayContainer* pContainer;							// nesting cast of the container panel
	//BorderPanelOverlayElement* pBorderPanel;	// display cast of the container panel

	// child elements
//	OverlayElement* pButtonText_OE;						// administrative cast of text
//	TextAreaOverlayElement* pButtonText_TAOE;	// display cast of text

	// state variables etc
	bool mHighlighted;
	bool mClicked;
	bool mSelected;
	float mXProp;
	float mYProp;
	float mWidgetWidth;
	float mWidgetHeight;
	float mXPos;
	float mYPos;
	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	bool mToggleType;
	bool mActive;

	char mNormalMaterial[100];
	char mSelectedMaterial[100];
};

#endif
