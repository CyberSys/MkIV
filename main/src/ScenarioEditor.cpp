#include "ScenarioEditor.h"
#include "globals.h"

// set members to default values
ScenarioEditor::ScenarioEditor() {
	pSceneManager = NULL;
	pTerrainHandler = NULL;
	mpUnitHandler = NULL;
	pCamera = NULL;
	pWindow = NULL;

	mpRaySceneQuery = NULL;
	mpMyLoader = NULL;
	mpMySaver = NULL;

	mVPWidth = 640.0f;
	mVPHeight = 200.0f;
	mEditMode = 0;
	mBrushSize = 1;
	mShrubClumpSize = 0;
	mForceSelection = 0;
	mExclusiveControl = false;
	mLoaderOpen = false;
	mLoadScenario = false;
	mSaverOpen = false;
	mSaveScenario = false;
	mUnitBrowserOpen = false;
	mShrubBrowserOpen = false;
	mBuildingBrowserOpen = false;
	mSceneryItemSelected = false;
	mCharacterSelected = false;
}

// initialisation specific to scenario editor
bool ScenarioEditor::init(SceneManager* pscenemgr, Camera* pcamera, RenderWindow* pwindow, TerrainHandler *pterrainHandler, float vpWidth, float vpHeight) {
	printf("Initialising Scenario Editor\n");

	// validate passed pointers
	if (!pscenemgr) {
		printf("ERROR: scene manager not allocated\n");
		return false;
	}
	pSceneManager = pscenemgr;
	if (!pcamera) {
		printf("ERROR: camera not allocated\n");
		return false;
	}
	pCamera = pcamera;
	if (!pwindow) {
		printf("ERROR: window ptr not allocated\n");
		return false;
	}
	pWindow = pwindow;
	if (!pterrainHandler) {
		printf("ERROR: terrainHandler not allocated\n");
		return false;
	}
	pTerrainHandler = pterrainHandler;

	mVPWidth = vpWidth;
	mVPHeight = vpHeight;

	// start terrain
	if (!pTerrainHandler->initScene(pscenemgr, pwindow, pCamera, false)) {
		printf("ERROR: Could not init Terrain Scene defaults\n");
		return false;
	}
	if (!pTerrainHandler->initET(pcamera->getViewport()->getActualHeight())) {
		printf("ERROR: Could not load Deformable Terrain\n");
		return false;
	}
	if (!pTerrainHandler->initEditorScenery()) {
		printf("ERROR: Could not init Shrubs\n");
		return false;
	}
	if (!pTerrainHandler->createEditBrush()) { // load a brush to edit terrain
		printf("ERROR: Could not load painting Brush\n");
		return false;
	}
	printf("  creating ray\n");
	mpRaySceneQuery = pscenemgr->createRayQuery(Ray());	// 3d picking ability
	mpUnitHandler = new UnitHandler;
	if (!mpUnitHandler->init(pscenemgr, pTerrainHandler, mpRaySceneQuery)) {
		printf("ERROR: could not add unithandler to scenario editor init\n");
		return false;
	}
	mpMyLoader = new FileBrowserWidget;
	mpMySaver = new FileSaverWidget;
	mpUnitBrowser = new UnitBrowserWidget;
	mpShrubBrowser = new ShrubBrowserWidget;
	mpBuildingBrowser = new BuildingBrowserWidget;
	mInfoBox.init("scedGuiIB", 0.0f, 0.0f, mVPWidth, mVPHeight, 0.0f, 0.0f, 300.0f, 100.0f);
	mInfoBox.setText("Mode: Select\nClick on a building or tree to select it.\nRight click on terrain to move selection.\n'<' and '>' keys rotate selection.\n'Del' key deletes selection.");
	mInfoBox.show();
	float yp = (mVPHeight - 60) / mVPHeight;
	float xp = (mVPWidth - 150) / mVPWidth;
	mToolTip.init("scedGuiTT", 0.0f, 0.0f, mVPWidth, mVPHeight, xp, yp, 150.0f, 20.0f);
	mToolTip.hide();
	printf("  setting gui text\n");
	strcpy(mToolButtonTipText[EDITMODE_SELECT], "Selection Mode");
	strcpy(mToolButtonTipText[EDITMODE_TERRAIN], "Hills and Valleys");
	strcpy(mToolButtonTipText[EDITMODE_PAINT], "Terrain Painting");
	strcpy(mToolButtonTipText[EDITMODE_SHRUBS], "Tree Planting");
	strcpy(mToolButtonTipText[EDITMODE_BUILDINGS], "Building Placement");
	strcpy(mToolButtonTipText[EDITMODE_VICTORY], "Victory Locations");
	strcpy(mToolButtonTipText[EDITMODE_UNITS], "Select Units");
	strcpy(mToolButtonTipText[EDITMODE_CAMERA], "Player Camera Start");
	strcpy(mToolButtonTipText[EDITMODE_X], "Navigation Target");
	strcpy(mToolButtonTipText[BUTTON_WATER], "Water");
	strcpy(mToolButtonTipText[BUTTON_LOAD], "Load Scenario");
	strcpy(mToolButtonTipText[BUTTON_SAVE], "Save Scenario");
	strcpy(mToolButtonTipText[BUTTON_SMALL_BRUSH], "Option: Small Brush");
	strcpy(mToolButtonTipText[BUTTON_MEDIUM_BRUSH], "Option: Medium Brush");
	strcpy(mToolButtonTipText[BUTTON_LARGE_BRUSH], "Option: Large Brush");
	strcpy(mToolButtonTipText[BUTTON_SINGLE_SHRUB], "Option: Single shrub");
	strcpy(mToolButtonTipText[BUTTON_CLUMP_SHRUBS], "Option: Clump of shrubs");
	strcpy(mToolButtonTipText[BUTTON_FOREST_SHRUBS], "Option: Large forest");
	strcpy(mToolButtonTipText[BUTTON_FORCE_1], "Force Number: 1");
	strcpy(mToolButtonTipText[BUTTON_FORCE_2], "Force Number: 2");
	
	strcpy(mToolButtonInfoText[EDITMODE_SELECT], "Mode: Select\nClick on a building or tree to select it.\nRight click on terrain to move selection.\n'<' and '>' keys rotate selection.\n'Del' key deletes selection.");
	strcpy(mToolButtonInfoText[EDITMODE_TERRAIN], "Mode: Mountains & Hills.\nLeft-click and hold on terrain for hill.\nRight click and hold on terrain for valley.\nChange effect size by clicking on icons.");
	strcpy(mToolButtonInfoText[2], "Mode: Painting.\n1-6 keys choose texture.\nLeft-click and hold on terrain to paint.\nRight click and hold on terrain to wipe.");
	strcpy(mToolButtonInfoText[3], "Mode: Placing trees.\nClick on terrain to place tree.\nThe options buttons let you place a single tree\n, a clump, or a forest per-click.\nTrees are randomly rotated on placement.");
	strcpy(mToolButtonInfoText[4], "Mode: Placing buildings.\nClick on terrain to place building.");
	strcpy(mToolButtonInfoText[5], "Mode: Placing Victory Locations.\nUNDER CONSTRUCTION\nClick on the terrain to place a victory location.");
	strcpy(mToolButtonInfoText[7], "Mode: Placing Player Camera Start Locations.\n UNDER CONSTRUCTION.");
	strcpy(mToolButtonInfoText[8], "Mode: Placing Navigation Targets.\n UNDER CONSTRUCTION.");
	strcpy(mToolButtonInfoText[6], "Mode: Placing Units for Scenario\nWhen chosing a unit to place, select the\nappropriate force number using the option\nbuttons.\nClick on the terrain to place a unit.");
	strcpy(mToolButtonInfoText[9], "Mode: Setting up water.\n UNDER CONSTRUCTION.");
	for (int l = BUTTON_LOAD; l < NUM_BUTTONS; l++) {
		strcpy(mToolButtonInfoText[l], ""); // no text for load/save
	}
	
	strcpy(mToolButtonMaterials[EDITMODE_SELECT], "mkiv/icon_select");
	strcpy(mToolButtonMaterials[EDITMODE_TERRAIN], "mkiv/icon_deform");
	strcpy(mToolButtonMaterials[EDITMODE_PAINT], "mkiv/icon_paint");
	strcpy(mToolButtonMaterials[EDITMODE_SHRUBS], "mkiv/icon_plant");
	strcpy(mToolButtonMaterials[EDITMODE_BUILDINGS], "mkiv/icon_building");
	strcpy(mToolButtonMaterials[EDITMODE_VICTORY], "mkiv/icon_flag");
	strcpy(mToolButtonMaterials[EDITMODE_UNITS], "mkiv/icon_tank");
	strcpy(mToolButtonMaterials[EDITMODE_CAMERA], "mkiv/icon_camera");
	strcpy(mToolButtonMaterials[EDITMODE_X], "mkiv/icon_x");
	strcpy(mToolButtonMaterials[9], "mkiv/icon_water");
	strcpy(mToolButtonMaterials[BUTTON_LOAD], "mkiv/icon_load");
	strcpy(mToolButtonMaterials[BUTTON_SAVE], "mkiv/icon_save");
	strcpy(mToolButtonMaterials[12], "mkiv/icon_small");
	strcpy(mToolButtonMaterials[13], "mkiv/icon_med");
	strcpy(mToolButtonMaterials[BUTTON_LARGE_BRUSH], "mkiv/icon_large");
	strcpy(mToolButtonMaterials[15], "mkiv/icon_single_tree");
	strcpy(mToolButtonMaterials[16], "mkiv/icon_copse");
	strcpy(mToolButtonMaterials[17], "mkiv/icon_forest");
	strcpy(mToolButtonMaterials[18], "mkiv/icon_num1");
	strcpy(mToolButtonMaterials[19], "mkiv/icon_num2");
	printf("  loading gui buttons\n");
	char tmp[128];
	yp = (mVPHeight - 44.0f) / mVPHeight;
	for (int i = 0; i < NUM_BUTTONS; i++) {
		sprintf(tmp, "seToolButt%i", i);
		bool modeStaysOn = true;
		if (i < BUTTON_LOAD) { // first row buttons along left hand side
			xp = (24.0f + (float)i * (44.0f)) / mVPWidth;
		} else if (i < BUTTON_SMALL_BRUSH) { // first row buttons right hand side
			xp = (mVPWidth - 24.0f - (float)(i - 9) * 44.0f) / mVPWidth;
			modeStaysOn = false;
		} else if (i < BUTTON_SINGLE_SHRUB) { // switch for 2nd row buttons
			xp = (24.0f + (float)(i - 10) * (44.0f)) / mVPWidth;
			yp = (mVPHeight - 88.0f) / mVPHeight;
		} else if (i < BUTTON_FORCE_1) {
			xp = (24.0f + (float)(i - 13) * (44.0f)) / mVPWidth;
			yp = (mVPHeight - 88.0f) / mVPHeight;
		} else {
			xp = (24.0f + (float)(i - 16) * (44.0f)) / mVPWidth;
			yp = (mVPHeight - 88.0f) / mVPHeight;
		}
		if (!mToolButtons[i].init(tmp, xp, yp, 0.0f, 0.0f, mVPWidth, mVPHeight, modeStaysOn, 40.0f, 40.0f)) {
			printf("ERROR: mToolButtons[%i] init failed in scenarioeditor::init()\n", i);
			return false;
		}
		strcpy(tmp, mToolButtonMaterials[i]);
		strcat(tmp, "_yell");
		if (!mToolButtons[i].setMaterials(mToolButtonMaterials[i], tmp)) {
			printf("ERROR: mToolButtons[%i] setMaterials failed in scenarioeditor::init()\n", i);
			return false;
		}
		if (i < BUTTON_SMALL_BRUSH) {
			mToolButtons[i].show();
		}
	}
	mToolButtons[EDITMODE_SELECT].setSelected(true);
	mToolButtons[BUTTON_MEDIUM_BRUSH].setSelected(true);
	mToolButtons[BUTTON_SINGLE_SHRUB].setSelected(true);
	mToolButtons[BUTTON_FORCE_1].setSelected(true);
	mpMyLoader->init("mkiv/editorLoader", 0.0f, 0.0f, mVPWidth, mVPHeight, 0.15f, 0.15f);
	mpMySaver->init("mkiv/editorSaver", 0.0f, 0.0f, mVPWidth, mVPHeight, 0.05f, 0.05f);
	mpUnitBrowser->mWidgetWidthPixels = mVPWidth;
	mpUnitBrowser->mWidgetHeightPixels = mVPHeight - 50.0f;
	if (!mpUnitBrowser->init("SEunitBwsr", 0.0f, mVPWidth, 0.0f, mVPHeight, 0.0f, 0.0f, pscenemgr)) {
		printf("ERROR: ScenarioEditor::init could not init unit browser widget\n");
		return false;
	}
	mpShrubBrowser->mWidgetWidthPixels = mVPWidth;
	mpShrubBrowser->mWidgetHeightPixels = mVPHeight - 50.0f;
	if (!mpShrubBrowser->init("mkiv/shrubBrowser", 0.0f, mVPWidth, 0.0f, mVPHeight, 0.0f, 0.0f, pscenemgr)) {
		printf("ERROR: ScenarioEditor::init could not init shrub browser widget\n");
		return false;
	}
	mpBuildingBrowser->mWidgetWidthPixels = mVPWidth;
	mpBuildingBrowser->mWidgetHeightPixels = mVPHeight - 50.0f;
	if (!mpBuildingBrowser->init("mkiv/buildingBrowser", 0.0f, mVPWidth, 0.0f, mVPHeight, 0.0f, 0.0f, pscenemgr)) {
		printf("ERROR: ScenarioEditor::init could not init building browser widget\n");
		return false;
	}
	mWaterChooser.init("waterchooser", 0.0f, 0.0f, mVPWidth, mVPHeight, 0.5, 0.5);
	mWaterChooser.setQuestion("Use a water plane at -10m?");
	mWaterChooser.hide();
	pscenemgr->setAmbientLight(ColourValue(1.0f, 1.0f, 1.0f));
	printf("Scenario Editor initialised\n");
	return true;
}

// shutdown specific to scenario editor
bool ScenarioEditor::free(SceneManager* pscenemgr) {
	printf("Shutting down Scenario Editor\n");
	if (!pscenemgr) { // validate passed pointers
		printf("ERROR: scene manager not allocated\n");
		return false;
	}
	if (!mpUnitHandler->free()) {
		printf("ERROR: could not free unithandler in scenario editor\n");
		return false;
	}
	delete mpUnitHandler;
	mpUnitHandler = NULL;
	delete mpRaySceneQuery;
	mpRaySceneQuery = NULL;
	for (int i = 0; i < NUM_BUTTONS; i++) {
		mToolButtons[i].free();
	}
	mWaterChooser.free();
	mInfoBox.free();
	mToolTip.free();
	
	if (!pTerrainHandler->free()) { // free anything allocated by unit handler
		printf("ERROR: could not free terrain resources\n");
		return false;
	}
	pTerrainHandler = NULL;
	if (!mpBuildingBrowser->free()) { // free widgets
		printf("ERROR: ScenarioEditor::free could not free mpBuildingBrowser\n");
		return false;
	}
	delete mpBuildingBrowser;
	mpBuildingBrowser = NULL;
	if (!mpShrubBrowser->free()) { // free widgets
		printf("ERROR: ScenarioEditor::free could not free mpShrubBrowser\n");
		return false;
	}
	delete mpShrubBrowser;
	mpShrubBrowser = NULL;
	if (!mpUnitBrowser->free()) { // free widgets
		printf("ERROR: ScenarioEditor::free could not free unit browser widget\n");
		return false;
	}
	delete mpUnitBrowser;
	mpUnitBrowser = NULL;
	if (!mpMyLoader->free()) { // free widgets
		printf("ERROR: could not free mpMyLoader resources\n");
		return false;
	}
	delete mpMyLoader;
	mpMyLoader = NULL;
	if (!mpMySaver->free()) { // free widgets
		printf("ERROR: could not free mpMySaver resources\n");
		return false;
	}
	delete mpMySaver;
	mpMySaver = NULL;
	pCamera = NULL;
	pscenemgr->clearScene(); // clear scene
	return true;
}

// handle GUI button logic for editor TODO: widgetise - returns true if a button is clicked on
bool ScenarioEditor::handleUserInput(unsigned long &elapsed, bool lmbClicked, bool &exclusiveControl) {
	// validation
	if (!pTerrainHandler) {
		// this is a fatal error because this func can not return error code
		printf("FATAL ERROR: TerrainHandler not allocated\n");
		exit(1);
	}
	int mouseX = g.mInputHandler.getMouseX();
	int mouseY = g.mInputHandler.getMouseY();
	
	if (mWaterChooser.isShowing()) {
		mWaterChooser.update(mouseX, mouseY, lmbClicked);
		if (mWaterChooser.isOKClicked()) {
			mWaterChooser.hide();
			pTerrainHandler->mWaterManager.createWaterPlane(-10.0);
			pTerrainHandler->mScenarioDetails.mWaterType = 1;
			pTerrainHandler->mScenarioDetails.mWaterLevel = -10.0;
		}
		if (mWaterChooser.isCancelClicked()) {
			mWaterChooser.hide();
			pTerrainHandler->mWaterManager.freeWaterPlane();
			pTerrainHandler->mScenarioDetails.mWaterType = 0;
		}
	}
	
	mToolTip.hide();
	for (int i = 0; i < NUM_BUTTONS; i++) {
		if (mEditMode == EDITMODE_SHRUBS) {
			if (i > BUTTON_SAVE && i < BUTTON_SINGLE_SHRUB) {
				continue;
			}
			if (i > BUTTON_FOREST_SHRUBS) {
				continue;
			}
		}
		if (mEditMode == EDITMODE_UNITS) {
			if (i > BUTTON_SAVE && i < BUTTON_FORCE_1) {
				continue;
			}
		}
		mToolButtons[i].update(mouseX, mouseY, lmbClicked);
		if (mToolButtons[i].isClicked()) {
			mInfoBox.show();
			if (i < BUTTON_LOAD) { // stops at load/save and they aren't edit modes
				mInfoBox.setText(mToolButtonInfoText[i]);
				mToolButtons[mEditMode].setSelected(false);
				mToolButtons[i].setSelected(true);
				mEditMode = i;
				
				if ((i == EDITMODE_TERRAIN) || (i == EDITMODE_PAINT)) {
					mToolButtons[12].show();
					mToolButtons[13].show();
					mToolButtons[14].show();
				} else {
					mToolButtons[12].hide();
					mToolButtons[13].hide();
					mToolButtons[14].hide();
				}
				
				if (i == EDITMODE_SHRUBS) {
					mpShrubBrowser->scan("data/shrubs/specifications/", ".txt"); // rescan
					mpShrubBrowser->show();
					mShrubBrowserOpen = true;
					mInfoBox.hide();
					exclusiveControl = !exclusiveControl;	// disable other buttons when menu is open
					mToolButtons[15].show();
					mToolButtons[16].show();
					mToolButtons[17].show();
				} else {
					mToolButtons[15].hide();
					mToolButtons[16].hide();
					mToolButtons[17].hide();
				} 
				
				if (i == EDITMODE_BUILDINGS) {
					mpBuildingBrowser->scan("data/buildings/specifications/", ".txt"); // rescan
					mpBuildingBrowser->show();
					mBuildingBrowserOpen = true;
					mInfoBox.hide();
					exclusiveControl = !exclusiveControl;	// disable other buttons when menu is open
				}
				
				if (i == EDITMODE_UNITS) {
					mpUnitBrowser->scan("data/vehicles/specifications/", ".spec"); // rescan
					mpUnitBrowser->show();
					mUnitBrowserOpen = true;
					mInfoBox.hide();
					exclusiveControl = !exclusiveControl;	// disable other buttons
					mToolButtons[18].show();
					mToolButtons[19].show();
				} else {
					mToolButtons[18].hide();
					mToolButtons[19].hide();
				}
				
				if (i == BUTTON_WATER) {
					mWaterChooser.show();
				}
				
			} else if (i == BUTTON_LOAD) {
				mpMyLoader->scan("data/scenarios/", ".sce"); //rescan
				mpMyLoader->show();
				mLoaderOpen = true;
				mInfoBox.hide();
				exclusiveControl = !exclusiveControl;	// disable other buttons when menu is open
			} else if (i == BUTTON_SAVE) {
				mpMySaver->scan("data/scenarios/", ".sce"); //rescan
				mpMySaver->setDetails(pTerrainHandler->mScenarioDetails);
				mpMySaver->show();
				mSaverOpen = true;
				mInfoBox.hide();
				exclusiveControl = !exclusiveControl;	// disable other buttons when menu is open
			} else if (i < BUTTON_SINGLE_SHRUB) {
				mToolButtons[mBrushSize + 12].setSelected(false);
				mBrushSize = i - 12;
				mToolButtons[i].setSelected(true);
				if (mBrushSize == 0) {
					pTerrainHandler->setBrushSizeSmall();
				} else if (mBrushSize == 1) {
					pTerrainHandler->setBrushSizeMed();
				} else {
					pTerrainHandler->setBrushSizeLarge();
				}
			} else if (i < BUTTON_FORCE_1) { // set the amount of shrubs to be placed {single=0, copse=1, forest=2}
				mToolButtons[mShrubClumpSize + BUTTON_SINGLE_SHRUB].setSelected(false);
				mShrubClumpSize = i - BUTTON_SINGLE_SHRUB;
				mToolButtons[i].setSelected(true);
			} else {
				mToolButtons[mForceSelection + BUTTON_FORCE_1].setSelected(false);
				mForceSelection = i - BUTTON_FORCE_1;
				mToolButtons[i].setSelected(true);
			}
			return true;
		} else if (mToolButtons[i].isHighlighted()) {
			if ((i < BUTTON_SMALL_BRUSH) || (mEditMode == EDITMODE_TERRAIN) || (mEditMode == EDITMODE_PAINT) || (mEditMode == EDITMODE_SHRUBS) || (mEditMode == EDITMODE_UNITS)) {
				mToolTip.setText(mToolButtonTipText[i]);
				int mx = g.mInputHandler.getMouseX();
				int my = g.mInputHandler.getMouseY();
				float mxp = (float)mx / mVPWidth;
				float myp = (float)(my - 5) / mVPHeight;
				if (mxp > 0.5) {
					mxp = (float)(mx - 150) / mVPWidth;
				}
				mToolTip.moveWidget(mxp, myp);
				mToolTip.show();
				return true;
			}
		}
	}
	return false;
}

// the main function of the editor - returns false if needs to break loop
bool ScenarioEditor::update(Timer &timer, unsigned long &elapsed) {
	if (!pCamera) {
		printf("FATAL ERROR: No camera allocated\n");
		exit(1);
	}
	elapsed = timer.getMicroseconds(); // capture time here
	frameLimiter(elapsed); // limit fps to 250
	elapsed = timer.getMicroseconds(); // limit fps to 250
	timer.reset(); // reset the time
	g.mInputHandler.captureAll(elapsed);// update buttons pressed
	if (g.mInputHandler.isQuitKeyDown()) { // quit but use this to close dialogs first
		if (mLoaderOpen) { // close browser, don't accept changes.
			mpMyLoader->hide();	// hide the sce chooser window
			mLoaderOpen = false;	// flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
		} else if (mSaverOpen) { // close browser, don't accept changes.
			mpMySaver->hide();	// hide the sce chooser window
			mSaverOpen = false;	// flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
		} else if (mShrubBrowserOpen) {
			mpShrubBrowser->hide();
			mShrubBrowserOpen = false;
			mExclusiveControl = false; // re-enable disabled buttons
		} else if (mBuildingBrowserOpen) {
			mpBuildingBrowser->hide();
			mBuildingBrowserOpen = false;
			mExclusiveControl = false; // re-enable disabled buttons
		} else if (mUnitBrowserOpen) {
			mpUnitBrowser->hide();
			mUnitBrowserOpen = false;
			mExclusiveControl = false; // re-enable disabled buttons
		} else { // TODO: add an "are you sure you want to quit" menu
			return false; // quit
		}
	}
	mpUnitHandler->editorSelectionUpdateLoop();
	if (mLoadScenario) { // load scenario if flagged to
		pSceneManager->clearScene();
		if (mpUnitHandler) {
			mpUnitHandler->free();
			printf("unit handler freed\n");
			delete mpUnitHandler;
			mpUnitHandler = NULL;
		}
		mpUnitHandler = new UnitHandler;
		if (!mpUnitHandler->init(pSceneManager, pTerrainHandler, mpRaySceneQuery)) {
			printf("ERROR: could not add unithandler to scenario editor init\n");
			return false;
		}

		if (!pTerrainHandler->loadScenario(mpMyLoader->getDetails()->mFilename)) {
			printf("ERROR: Could not Load Scenario\n");
			return false;
		}
		char ulistName[256];
		strcpy(ulistName, "data/scenarios/maps/");
		strcat(ulistName, mpMyLoader->getDetails()->mUnitListName);
		if (!mpUnitHandler->loadUnitsFromFile(ulistName)) {
			printf("ERROR: Could not Load Scenario units in scenarioeditor\n");
			return false;
		}
		g.mGUI.showLoading(false);	// hide loading scenario overlay
		mLoadScenario = false;
		// reset camera
		Vector3 camPos = mpMyLoader->getDetails()->mForces[0].mInitialCameraPos; // just load the observer's camera position for scenario editor
		Vector3 camAim = mpMyLoader->getDetails()->mForces[0].mInitialCameraAim; // just load the observer's camera aim for scenario editor
		pCamera->setPosition(camPos); // move cam into position
		pCamera->pitch(Radian(camAim.x));
		pCamera->yaw(Radian(camAim.y));
		pCamera->roll(Radian(camAim.z));
		return true;
	}
	if (mSaveScenario) { // save scenario if flagged to
		//pTerrainHandler->mScenarioDetails.copyFrom(mpMySaver->getDetails()); // TODO - why are we copying all this?
		if (!pTerrainHandler->saveScenario(mpMySaver->getDetails()->mTitle)) {
			printf("ERROR: Could not create Scenario\n");
			return false;
		}
		char temp[256];
		strcpy(temp, "data/scenarios/maps/");
		strcat(temp, pTerrainHandler->mLastFileNameCreated);
		strcat(temp, "_units.txt");
		if (!mpUnitHandler->saveUnitsToFile(temp)) {
			printf("ERROR: Could not save units to file ScenarioEditor::update\n");
			return false;
		} // save vehicles to file here
		g.mGUI.showSaving(false);	// hide saving scenario overlay
		mSaveScenario = false;
		return true;
	}

	// check mouse clicks on buttons
	bool lmbClicked = g.mInputHandler.isLMBDown();
	bool rmbHeldDown = g.mInputHandler.isRMBHeldDown();
	bool buttonArea = false;	// if mouse is over a button (i.e. can not edit terrain)
	if (!mExclusiveControl) {
		buttonArea = handleUserInput(elapsed, lmbClicked, mExclusiveControl);
	}

// TODO: put each edit mode in a subfunction

	if (!mExclusiveControl) {
		if (mEditMode == EDITMODE_SELECT) {
			Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX() / mVPWidth,g.mInputHandler.getMouseY() / mVPHeight); // Setup the ray scene query
			mpRaySceneQuery->setRay(mouseRay);
			mpRaySceneQuery->setSortByDistance(true);
			RaySceneQueryResult &result = mpRaySceneQuery->execute(); // Execute query
			RaySceneQueryResult::iterator itr = result.begin();
			bool hitTerrain = false;
			pTerrainHandler->setSelectedNode(NULL);
			mpUnitHandler->setSelectedNode(NULL);
			for (itr = result.begin(); itr != result.end(); itr++) { // Get results, create a node/entity on the position
				if ((itr->movable && itr->movable->getName().substr(0, 5) == "shrub") || (itr->movable && itr->movable->getName().substr(0,5) == "build")) { // differentiates between units and scenery
					if (lmbClicked) {
						pTerrainHandler->setSelectedNode(itr->movable->getParentSceneNode()); // tell the unit handler which node was selected (so it can match it to a vehicle)
						hitTerrain = false;
						mSceneryItemSelected = true;
						mCharacterSelected = false;
						mpUnitHandler->deselectAll();
					}
					break;
				} else if (itr->movable && itr->movable->getName().substr(0, 5) == "vehic") {
					if (lmbClicked) {
						mpUnitHandler->setSelectedNode(itr->movable->getParentSceneNode());
						mSceneryItemSelected = false;
						mCharacterSelected = true;
						pTerrainHandler->deselectAll();
					} else if (rmbHeldDown) {
						int targetUnit = mpUnitHandler->getUnitMatchingNode(itr->movable->getParentSceneNode());
						if (targetUnit > -1) {
							mpUnitHandler->orderBoardVehicle(mpUnitHandler->mSelectedVehicleIndex, targetUnit);
						}
					}
					break;
				} else if (pTerrainHandler->checkMouseIntersect(mouseRay)) { // otherwise check if hit terrain
						if (lmbClicked) {
							hitTerrain = true;
							mpUnitHandler->deselectAll();
							pTerrainHandler->deselectAll();
							mSceneryItemSelected = false;
							mCharacterSelected = false;
						} else if (rmbHeldDown) {
							if (mSceneryItemSelected) {
								pTerrainHandler->moveSelectionWithMouse();
							} else if (mCharacterSelected) {
								mpUnitHandler->moveSelectedVehicleTo(pTerrainHandler->getMousePosition().x, pTerrainHandler->getMousePosition().y, pTerrainHandler->getMousePosition().z);
							}
						}
					}
			}
			if (g.mInputHandler.isDeleteKeyDown()) {
				if (mSceneryItemSelected) {
					pTerrainHandler->deleteSelection();
				} else if (mCharacterSelected) {
					mpUnitHandler->editorDeleteSelection();
				}
			}
			if (g.mInputHandler.isRotatePosKeyDown()) {
				if (mSceneryItemSelected) {
					pTerrainHandler->posRotSelection(elapsed);
				} else if (mCharacterSelected) {
					mpUnitHandler->editorRotateSelectedVehicle(0.01);
				}
			} else if (g.mInputHandler.isRotateNegKeyDown()) {
				if (mSceneryItemSelected) {
					pTerrainHandler->negRotSelection(elapsed);
				} else if (mCharacterSelected) {
					mpUnitHandler->editorRotateSelectedVehicle(-0.01);
				}
			}
		} else if (mEditMode == EDITMODE_TERRAIN) {
			// LMB = terrain mountain maker
			if (!buttonArea && g.mInputHandler.isLMBHeldDown()) {
				// get 3D ray from 2D mouse coords
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX()/mVPWidth,g.mInputHandler.getMouseY()/mVPHeight);
				// go ahead if ray intersects terrain
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					pTerrainHandler->deform(elapsed, true);
					pTerrainHandler->updateLightmap();
				}
			} else if (g.mInputHandler.isRMBHeldDown()) {// RMB = terrain valley maker
				// get 3D ray from 2D mouse coords
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX()/mVPWidth,g.mInputHandler.getMouseY()/mVPHeight);
				// go ahead if ray intersects terrain
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					pTerrainHandler->deform(elapsed, false);
					pTerrainHandler->updateLightmap();
				}
			}
		} else if (mEditMode == EDITMODE_PAINT) { // paint
			if (g.mInputHandler.isPaintModeNoneKeyDown()) { // change paint mode based on user input
				pTerrainHandler->setCurrentPaint(-1);
			} else if (g.mInputHandler.isPaintMode1KeyDown()) {
				pTerrainHandler->setCurrentPaint(0);
			} else if (g.mInputHandler.isPaintMode2KeyDown()) {
				pTerrainHandler->setCurrentPaint(1);
			} else if (g.mInputHandler.isPaintMode3KeyDown()) {
				pTerrainHandler->setCurrentPaint(2);
			} else if (g.mInputHandler.isPaintMode4KeyDown()) {
				pTerrainHandler->setCurrentPaint(3);
			} else if (g.mInputHandler.isPaintMode5KeyDown()) {
				pTerrainHandler->setCurrentPaint(4);
			} else if (g.mInputHandler.isPaintMode6KeyDown()) {
				pTerrainHandler->setCurrentPaint(5);
			} else if (g.mInputHandler.isPaintMode7KeyDown()) {
				pTerrainHandler->setCurrentPaint(6);
			}

			// terrain editing
			if (!buttonArea && g.mInputHandler.isLMBHeldDown()) {
				// get 3D ray from 2D mouse coords
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX()/mVPWidth,g.mInputHandler.getMouseY()/mVPHeight);
				// go ahead if ray intersects terrain
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					pTerrainHandler->paint(elapsed, true);
				}
			} else if (g.mInputHandler.isRMBHeldDown()) {	// negative intensity paint, whatever that does
				// get 3D ray from 2D mouse coords
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX()/mVPWidth,g.mInputHandler.getMouseY()/mVPHeight);
				// go ahead if ray intersects terrain
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					pTerrainHandler->paint(elapsed, false);
				}
			}

		} else if (mEditMode == EDITMODE_SHRUBS) {
			// planting trees. tell the editor to use a tree as a mouse pointer
			// if LMB plant a new tree
			if (!buttonArea && lmbClicked) {
				// get 3D ray from 2D mouse coords
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX() / mVPWidth, g.mInputHandler.getMouseY() / mVPHeight);
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					if (mpShrubBrowser->mpSelectedUnitDetails->mIsShrubLoaded) {
						if (pTerrainHandler->createShrubsAtPointer(mpShrubBrowser->mpSelectedUnitDetails->getFilename(), mShrubClumpSize) < 0) {
							// TODO: catch too many shrubs (-4) and display info box instead
							printf("FATAL ERROR: Failed to create shrub at mouse\n");
							exit(1);
						}
					}
				}
			}
		} else if (mEditMode == EDITMODE_BUILDINGS) {
			if (!buttonArea && lmbClicked) { // if LMB plant a new tree
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX() / mVPWidth,g.mInputHandler.getMouseY() / mVPHeight); // get 3D ray from 2D mouse coords
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					if (mpBuildingBrowser->mpSelectedUnitDetails->mIsBuildingLoaded) {
						if (pTerrainHandler->createBuildingAtPointer(mpBuildingBrowser->mpSelectedUnitDetails->getFilename()) < 0) {
							// TODO: catch too many bldgs (-4) and display info box instead
							printf("FATAL ERROR: Failed to create building at mouse\n");
							exit(1);
						}
					}
				}
			}
		} else if (mEditMode == EDITMODE_VICTORY) {
			if (!buttonArea && lmbClicked) { // if LMB plant a new flag
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX() / mVPWidth,g.mInputHandler.getMouseY() / mVPHeight); // get 3D ray from 2D mouse coords
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					if (pTerrainHandler->createBuildingAtPointer(mpBuildingBrowser->mpSelectedUnitDetails->getFilename()) < 0) {
						printf("FATAL ERROR: Failed to create VL at mouse\n");
						exit(1);
					}
				}
			}
		} else if (mEditMode == EDITMODE_UNITS) { // placing vehicles and characters
			if (!buttonArea && lmbClicked) { // if LMB place a new character
				Ray mouseRay = pCamera->getCameraToViewportRay(g.mInputHandler.getMouseX() / mVPWidth,g.mInputHandler.getMouseY() / mVPHeight); // get 3D ray from 2D mouse coords
				if (pTerrainHandler->checkMouseIntersect(mouseRay)) {
					if (mpUnitBrowser->mpSelectedUnitDetails->mHasBeenLoadedFromFile) {
						double yaw = 0.0;
						int force = mForceSelection;
						if (mForceSelection == 1) {
						 yaw = 3.14;
						}
						if (!mpUnitHandler->createCharacterAtPosition(mpUnitBrowser->mpSelectedUnitDetails->mFilename, pTerrainHandler->getMousePosition(), yaw, force)) {
							printf("ERROR: ScenarioEditor::update could not create vehicle at mouse position\n");
							return false;
						}
					}
				}
			}
		}
	}

	if (!mLoaderOpen && !mSaverOpen && !mUnitBrowserOpen && !mShrubBrowserOpen && !mBuildingBrowserOpen) {
		// set global modes
		if (g.mInputHandler.isTogglePointerKeyDown()) {
			g.mPointerMode = !g.mPointerMode;
		}
		// display debug panel here if key down
		if (g.mInputHandler.isToggleDebugPanelKeyDown()) {
			if (g.mGUI.isShowingDebugPanel()) {
				g.mGUI.showDebug(false);
			} else {
				g.mGUI.showDebug(true);
			}
		}
		pTerrainHandler->update(elapsed); // update scenery handler
	}
	if (g.mGUI.isShowingDebugPanel()) { // update debug panel
		g.mGUI.updateStats(pWindow);
	}
	g.mGUI.updateMousePosition(); // update GUI with mouse position etc
	if (mLoaderOpen) {
		mpMyLoader->update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), lmbClicked);
		if (mpMyLoader->isCancelClicked()) { // close browser, don't accept changes.
			mpMyLoader->hide();	// hide the sce chooser window
			mLoaderOpen = false; // flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
		} else if (mpMyLoader->isOKClicked()) { // close browser, and accept changes.
			mpMyLoader->hide(); // hide the sce chooser window
			mLoaderOpen = false; // flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
			g.mGUI.showLoading(true);	// show loading scenario overlay
			mLoadScenario = true; // Load the scenario
		}
	}
	if (mSaverOpen) {
		mpMySaver->update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), lmbClicked);
		if (mpMySaver->isCancelClicked()) {
			g.mInputHandler.stopCapturingText();
			// close browser, don't accept changes.
			mpMySaver->hide();	// hide the sce chooser window
			mSaverOpen = false;	// flag browser as closed
			// re-enable disabled buttons
			mExclusiveControl = false;
		} else if (mpMySaver->isOKClicked()) {
			g.mInputHandler.stopCapturingText();
			// close browser, and accept changes.
			mpMySaver->hide();	// hide the sce chooser window
			mSaverOpen = false;	// flag browser as closed
			// re-enable disabled buttons
			mExclusiveControl = false;

			// Show 'Saving Scenario' text
			g.mGUI.showSaving(true);	// show loading scenario overlay
			// Save the scenario
			mSaveScenario = true;
		}
	}
	if (mUnitBrowserOpen) {
		mpUnitBrowser->update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), lmbClicked);
		if (mpUnitBrowser->isOKClicked()) { // close browser, and accept changes.
			g.mInputHandler.stopCapturingText();
			mpUnitBrowser->hide();	// hide the sce chooser window
			mUnitBrowserOpen = false;	// flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
			mInfoBox.show();
		}
	} else if (mShrubBrowserOpen) {
		mpShrubBrowser->update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), lmbClicked);
		if (mpShrubBrowser->isOKClicked()) { // close browser, and accept changes.
			g.mInputHandler.stopCapturingText();
			mpShrubBrowser->hide();	// hide the sce chooser window
			mShrubBrowserOpen = false;	// flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
			mInfoBox.show();
		}
	} else if (mBuildingBrowserOpen) {
		mpBuildingBrowser->update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), lmbClicked);
		if (mpBuildingBrowser->isOKClicked()) { // close browser, and accept changes.
			g.mInputHandler.stopCapturingText();
			mpBuildingBrowser->hide();	// hide the sce chooser window
			mBuildingBrowserOpen = false;	// flag browser as closed
			mExclusiveControl = false; // re-enable disabled buttons
			mInfoBox.show();
		}
	}
	return true;
}
