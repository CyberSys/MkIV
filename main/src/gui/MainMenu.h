#ifndef MARKIV_MAINMENU_WIDGET_H_
#define MARKIV_MAINMENU_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "OgrePanelOverlayElement.h"
#include "ImageWidget.h"
#include "LabelWidget.h"
#include "TextEntryWidget.h"

using namespace Ogre;

// This is a class controlling the main menu GUI and pathways through the program
class MainMenu {
public:
	MainMenu();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	bool update(unsigned int mousex, unsigned int mousey, bool lmb);
	bool show();
	bool hide();
	bool isQuitClicked();
	bool isStartSoloClicked();
	bool isHostServerClicked();
	bool isJoinServerClicked();
	bool isScenEditorClicked();
private:
	Overlay* pOverlay;
	// overlay panel to draw a background
	OverlayElement* pBaseElement;							// administrative cast of the container panel
	OverlayContainer* pContainer;							// nesting cast of the container panel
	PanelOverlayElement* pPanel;							// display panel

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

	// nested widgets
	ButtonWidget mQuitButton;
	ButtonWidget mSoloButton;
	ButtonWidget mServerButton;
	ButtonWidget mClientButton;
	ButtonWidget mScenEditorButton;
	ImageWidget mLogoImage; // logo image
	LabelWidget mVersionText; // version info text
	TextEntryWidget mTextEntryWidget; // a text-entry box to enter a server IP
};

#endif
