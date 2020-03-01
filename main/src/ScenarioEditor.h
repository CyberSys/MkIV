#ifndef MARKIV_SCENARIOEDITOR_H_
#define MARKIV_SCENARIOEDITOR_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "gui/FileBrowserWidget.h"
#include "gui/FileSaverWidget.h"
#include "gui/UnitBrowserWidget.h"
#include "gui/ShrubBrowserWidget.h"
#include "gui/BuildingBrowserWidget.h"
#include "gui/InfoBoxWidget.h"
#include "gui/ImageButtonWidget.h"
#include "gui/YesNoBoxWidget.h"
#include "scenery/ScenarioDetails.h"
#include "scenery/TerrainHandler.h"
#include "units/UnitHandler.h"

#define NUM_BUTTONS 20
#define EDITMODE_SELECT 0
#define EDITMODE_TERRAIN 1
#define EDITMODE_PAINT 2
#define EDITMODE_SHRUBS 3
#define EDITMODE_BUILDINGS 4
#define EDITMODE_VICTORY 5
#define EDITMODE_UNITS 6
#define EDITMODE_CAMERA 7
#define EDITMODE_X 8
#define BUTTON_WATER 9
#define BUTTON_LOAD 10
#define BUTTON_SAVE 11
#define BUTTON_SMALL_BRUSH 12
#define BUTTON_MEDIUM_BRUSH 13
#define BUTTON_LARGE_BRUSH 14
#define BUTTON_SINGLE_SHRUB 15
#define BUTTON_CLUMP_SHRUBS 16
#define BUTTON_FOREST_SHRUBS 17
#define BUTTON_FORCE_1 18
#define BUTTON_FORCE_2 19

using namespace Ogre; 

// contains all code required for building scenarios in 3d. could be a separate program, but it's not.
class ScenarioEditor {
public:
	ScenarioEditor();
	bool init(SceneManager* pscenemgr, Camera* pcamera, RenderWindow* pwindow, TerrainHandler *pterrainHandler, float vpWidth, float vpHeight);
	bool free(SceneManager* pscenemgr);
	bool update(Timer &timer, unsigned long &elapsed);
private:
	// private methods
	bool handleUserInput(unsigned long &elapsed, bool lmbClicked, bool &exclusiveControl);
	// pointers to external objects (NOTE: this class could really have its own TH)
	Ogre::SceneManager* pSceneManager;
	TerrainHandler* pTerrainHandler;
	UnitHandler* mpUnitHandler;
	Camera* pCamera;
	RenderWindow* pWindow;
	// internally allocated memory
	RaySceneQuery* mpRaySceneQuery;	// scene query for checking mouse clicks
	FileBrowserWidget* mpMyLoader;	// file browser widget
	FileSaverWidget* mpMySaver;	// file browser widget
	UnitBrowserWidget* mpUnitBrowser;
	ShrubBrowserWidget* mpShrubBrowser;
	BuildingBrowserWidget* mpBuildingBrowser;
	// member variables
	InfoBoxWidget mInfoBox;
	InfoBoxWidget mToolTip;
	YesNoBoxWidget mWaterChooser;
	ImageButtonWidget mToolButtons[NUM_BUTTONS];
	char mToolButtonTipText[NUM_BUTTONS][128];
	char mToolButtonInfoText[NUM_BUTTONS][256];
	char mToolButtonMaterials[NUM_BUTTONS][64];
	float mVPWidth;
	float mVPHeight;
	int mEditMode;
	int mBrushSize;
	int mShrubClumpSize;
	int mForceSelection;
	bool mExclusiveControl;	// whether or not a dialogue is open that demands control of all the buttons
	bool mLoaderOpen;	// if the file loader gui is open
	bool mSaverOpen;	// if the file saver gui is open
	bool mUnitBrowserOpen;
	bool mBuildingBrowserOpen;
	bool mShrubBrowserOpen;
	bool mLoadScenario;	// internal flag for loading scen in next update
	bool mSaveScenario; // internal flag for saving scen in next update
	bool mSceneryItemSelected;
	bool mCharacterSelected;
};

#endif

