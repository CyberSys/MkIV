#ifndef MARKIV_LOBBYMENU_WIDGET_H_
#define MARKIV_LOBBYMENU_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgrePanelOverlayElement.h"
#include "BigTitleWidget.h"
#include "ButtonWidget.h"
#include "FileBrowserWidget.h"
#include "ImageButtonWidget.h"
#include "LabelWidget.h"
#include "ScenarioDetailsWidget.h"
#include "../scenery/ScenarioDetails.h"

using namespace Ogre;

class LobbyMenu {
public:
	LobbyMenu();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, int type);	// level {0,1,2} = {solo,client,server}
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool moveWidget(float xprop, float yprop);	// note xpos and ypos are 0.0-0.1
	OverlayElement* getElement();
	bool update(unsigned int mousex, unsigned int mousey, bool lmb);
	bool show();
	bool hide();
	bool isStartClicked();
	bool isCancelClicked();
	bool isReadyClicked();
	bool isChooseScenarioClicked();
	bool updateReadyCounter(int readyCount, int notReadyCount);	// update gui
	ScenarioDetails* getSelectedScenarioDetails();
	bool useGlobalScenario();
	bool closeAnyMenus();
private:
	bool selectScenario();
	Overlay* pOverlay;
	// overlay panel to draw a background
	OverlayElement* pBaseElement;							// administrative cast of the container panel
	OverlayContainer* pContainer;							// nesting cast of the container panel
	PanelOverlayElement* pPanel;							// display panel
	// nested widgets
	BigTitleWidget mTitleWidget;
	ButtonWidget mStartButton;
	ButtonWidget mCancelButton;
	ButtonWidget mReadyButton;
	ButtonWidget mChooseScenarioButton;
	ImageButtonWidget mForceShield[5];
	LabelWidget mReadyCountLabel;
	FileBrowserWidget mFileBrowser;
	ScenarioDetailsWidget mScenarioDetailsBox;

	int	mType;	// type of user {0 = solo, 1 = client, 2 = server}
	int mReadyCount;
	int mNotReadyCount;
	bool mBrowserOpen;

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

	// **TODO Shield Widgets
};

#endif
