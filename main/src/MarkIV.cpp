#include "MarkIV.h"
#include "globals.h"
#include "ai/FuzzyController.h" // included only for line-graph construction which may be a temporary feature
#include "audio/AudioHandler.h"

MarkIV::MarkIV() {
	mpScenarioEditor = NULL;
	mCamCreated = false;
}

// initialise the program
bool MarkIV::init() {
	printf("Initialising...\n");
	mRoot = new Root();
	if (!mRoot) {
		printf("[FAILED]\nERROR: Could not allocate memory to Root object\n");
		return false;
	}
	mInChatMode = false;
	ConfigFile cf;
	cf.load("resources.cfg"); // Load resource paths from config file
	ConfigFile::SectionIterator seci = cf.getSectionIterator(); // Go through all sections & settings in the file
	String secName, typeName, archName;
	while (seci.hasMoreElements()) {
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i) {
			typeName = i->first;
			archName = i->second;
		    ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}
	if (!userConfigure()) { // run user config screen
		printf("[HALTED]\n");
		return false;
	}
printf("debug: finished userconfig\n");
	chooseSceneManager(); // get the appropriate scene manager
printf("debug: choose sm\n");
	if (!createCamera()) { // init camera
		printf("Camera init error\n");
		return false;
	}
	createViewports();
printf("debug: created viewps\n");
	TextureManager::getSingleton().setDefaultNumMipmaps(5); // Set default mipmap level (NB some APIs ignore this)
	g.mInputHandler.init(mWindow); // start user input capture
	g.mGUI.init((unsigned int)mVPWidth, (unsigned int)mVPHeight); // start tsunami gui
printf("debug: gui inited\n");
	if (!mMainMenu.init("mkiv/MainMenu", 0.0f, 0.0f, mVPWidth, mVPHeight, 0.0f, 0.0f)) { // start main menu
		printf("ERROR: MarkIV::init could not start mainmenu\n");
	}
printf("debug: menu inited\n");
	pLobbyMenu = NULL;
	if (!g.mNoAudio) {
		SoundManager::getSingleton().init();	// initialise audio handler
	}
printf("debug: audio inited\n");
	pScenarioDetails = NULL;
	mReallyQuitPopup.init("ReallyQuitMainMenuBox", 0, 0, mVPWidth, mVPHeight, 0.4f, 0.4f);
	mReallyQuitPopup.setQuestion("Bail out?");
	mReallyQuitPopup.hide();
	mChaseCamTargetVehicleIndex = -1;
	printf("Initialisation done.\n");
	return true;
}

// initialisation specific to server
bool MarkIV::startServer() {
	if (!g.mServer.init()) { // init server
		printf("ERROR: failed to initialise server\n");
		return false;
	}
	if (!g.mServer.start()) {
		printf("ERROR: failed to start server\n");
		return false;
	}
	g.mGUI.showChatMsgs(); // display chat messages box
	return true;
}

// shutdown specific to server
bool MarkIV::shutdownServer() {
	g.mGUI.clearChatMsgs(); // clear chat box (in case we want to start another session)
	g.mGUI.showChatMsgs(false);	// hide chat box
	g.mServer.stop();					// stop server
	g.mServer.free();					// free memory
	return true;
}

// initialisation specific to client // return 0 for crashed, 1 for connected, or 2 for go back to menu
int MarkIV::startClient() {
	// assuming IP is already set!
	if (!g.mClient.connect()) { // connect to Server
		printf("ERROR: failed to connect to server\n");
		return -1;
	}
	if (!connectingLoop()) { // wait for connection or until timeout or user presses esc
		printf("WARNING: connection timed out - returning to menu\n");
		return 0; // return to menu if returns false
	}
	g.mGUI.showChatMsgs(); // display chat messages box
	return 1; // connected properly
}

// shutdown specific to client
bool MarkIV::shutdownClient() {
	g.mGUI.clearChatMsgs(); // clear chat box (in case we want to start another session)
	g.mGUI.showChatMsgs(false);	// hide chat box
	if (!g.mClient.disconnect()) { // stop client
		printf("ERROR: Could not disconnect cleanly from server\n");
	}
	if (!g.mClient.free()) { // free memory
		printf("ERROR: Could not free client memory\n");
	}
	return true;
}

void MarkIV::initLineGraphs() {
FuzzyController myFuzzyContr;
	myFuzzyContr.readSetsFromFile(g.mFuzzySetFile);
	// init graphs
	float graphColumn2 = 513.0f / mVPWidth; // 2nd column (511.0 with a 2 pixel gap)
	float graphRow2 = 177.0f / mVPHeight; // 2nd row
	float graphRow3 = 354.0f / mVPHeight; // 3rd row
	float graphRow4 = 531.0f / mVPHeight; // 4th row
	g.mFuzzyObstDistGraph.init("fuzzy_obst_dist_inputs", 0.0f, 0.0f, mVPHeight, mVPWidth, graphColumn2, 0.0f, mSceneMgr);
	g.mLineGraphWidgetB.init("fuzzy_targ_angle_inputs", 0.0f, 0.0f, mVPHeight, mVPWidth, 0.0f, graphRow2, mSceneMgr);
	g.mFuzzyObstAngleGraph.init("fuzzy_obst_angle_inputs", 0.0f, 0.0f, mVPHeight, mVPWidth, graphColumn2, graphRow2, mSceneMgr);
	g.mLineGraphWidgetC.init("test_line_graphC", 0.0f, 0.0f, mVPHeight, mVPWidth, 0.0f, graphRow3, mSceneMgr);
	g.mDefuzzifiedSteeringGraph.init("defuzzified_steering", 0.0f, 0.0f, mVPHeight, mVPWidth, graphColumn2, graphRow3, mSceneMgr);
	g.mCrashGraph.init("crashes", 0.0f, 0.0f, mVPHeight, mVPWidth, 0.0f, graphRow4, mSceneMgr);
	g.mLineGraphWidgetA.init("fuzzy_targ_dist_inputs", 0.0f, 0.0f, mVPHeight, mVPWidth, 0.0f, 0.0f, mSceneMgr);
	g.mLineGraphWidgetA.setGraphTitle("Fuzzy Input Set Membership - Distance to Target"); // graph titles
	g.mLineGraphWidgetA.setXAxisTitle("Distance^2 (m*m)"); // graph axes
	g.mLineGraphWidgetA.setXAxisMax(myFuzzyContr.mInTargetDistanceVeryFar[1]); // 1 minute
	g.mLineGraphWidgetA.setXAxisMin(0.0f); // start at 0 s
	g.mLineGraphWidgetA.setYAxisMax(1.0f); // set axis scale
	g.mLineGraphWidgetA.setYAxisMin(0.0f); // set axis scale
	g.mLineGraphWidgetA.createLine(mSceneMgr, 1.0f, 0.0f, 0.0f); // create first set line
	g.mLineGraphWidgetA.addPointToLine(0, 0.0f, 1.0f);
	g.mLineGraphWidgetA.addPointToLine(0, myFuzzyContr.mInTargetDistanceNear[0], 1.0f);
	g.mLineGraphWidgetA.addPointToLine(0, myFuzzyContr.mInTargetDistanceNear[1], 0.0f);
	g.mLineGraphWidgetA.createLine(mSceneMgr, 1.0f, 0.0f, 1.0f); // create 2nd set line
	g.mLineGraphWidgetA.addPointToLine(1, myFuzzyContr.mInTargetDistanceFar[0], 0.0f);
	g.mLineGraphWidgetA.addPointToLine(1, myFuzzyContr.mInTargetDistanceFar[1], 1.0f);
	g.mLineGraphWidgetA.addPointToLine(1, myFuzzyContr.mInTargetDistanceFar[2], 1.0f);
	g.mLineGraphWidgetA.addPointToLine(1, myFuzzyContr.mInTargetDistanceFar[3], 0.0f);
	g.mLineGraphWidgetA.createLine(mSceneMgr, 0.0f, 0.0f, 1.0f); // create 3rd set line
	g.mLineGraphWidgetA.addPointToLine(2, myFuzzyContr.mInTargetDistanceVeryFar[0], 0.0f);
	g.mLineGraphWidgetA.addPointToLine(2, myFuzzyContr.mInTargetDistanceVeryFar[1], 1.0f);
	g.mLineGraphWidgetA.createLine(mSceneMgr, 0.0f, 1.0f, 0.0f); // this will be the indicator for currently tracked targ dist
	g.mFuzzyObstDistGraph.setGraphTitle("Fuzzy Input Set Membership - Distance to Obstacle");
	g.mLineGraphWidgetB.setGraphTitle("Fuzzy Input Set Membership - Angle to Target");
	g.mFuzzyObstAngleGraph.setGraphTitle("Fuzzy Input Set Membership - Angle to Obstacle");
	g.mLineGraphWidgetC.setGraphTitle("Defuzzified Output - Intended Velocity");
	g.mDefuzzifiedSteeringGraph.setGraphTitle("Defuzzified Output - Steering Adjustment");
	g.mCrashGraph.setGraphTitle("Collisions");
	g.mFuzzyObstDistGraph.setXAxisTitle("Distance^2 (m*m)");
	g.mFuzzyObstDistGraph.setXAxisMax(myFuzzyContr.mInObstacleDistanceVeryFar[1]);
	g.mFuzzyObstDistGraph.setXAxisMin(0.0f);
	g.mFuzzyObstDistGraph.setYAxisMax(1.0f);
	g.mFuzzyObstDistGraph.setYAxisMin(0.0f);
	g.mLineGraphWidgetB.setXAxisTitle("Angle from Current Heading (rad)");
	g.mLineGraphWidgetB.setXAxisMax(myFuzzyContr.mInObstacleAngleLarge[1]); // 1 minute
	g.mLineGraphWidgetB.setXAxisMin(0.0f); // start at 0 s
	g.mLineGraphWidgetB.setYAxisMax(1.0f); // set axis scale
	g.mLineGraphWidgetB.setYAxisMin(0.0f); // set axis scale
	g.mFuzzyObstAngleGraph.setXAxisTitle("Angle from Current Heading (rad)");
	g.mFuzzyObstAngleGraph.setXAxisMax(myFuzzyContr.mInObstacleAngleLarge[1]);
	g.mFuzzyObstAngleGraph.setXAxisMin(0.0f);
	g.mFuzzyObstAngleGraph.setYAxisMax(1.0f);
	g.mFuzzyObstAngleGraph.setYAxisMin(0.0f);
	g.mLineGraphWidgetC.setXAxisTitle("Time (s)");
	g.mLineGraphWidgetC.setXAxisMax(30.0f); // 30 secs window
	g.mLineGraphWidgetC.setXAxisMin(0.0f); // start at 0 s
	g.mLineGraphWidgetC.setYAxisMax(100.0f); // 100 kph max
	g.mLineGraphWidgetC.setYAxisMin(0.0f); // start at 0kph
	g.mLineGraphWidgetC.createLine(mSceneMgr, 0.0f, 1.0f, 1.0f); // create first test line
	g.mLineGraphWidgetC.setXAxisScroll(); // scrolling graph
	g.mDefuzzifiedSteeringGraph.setXAxisTitle("Time (s)");
	g.mDefuzzifiedSteeringGraph.setXAxisMax(30.0f); // 30 secs window
	g.mDefuzzifiedSteeringGraph.setXAxisMin(0.0f); // start at 0 s
	g.mDefuzzifiedSteeringGraph.setYAxisMax(1.6f); // 1.6 radians
	g.mDefuzzifiedSteeringGraph.setYAxisMin(0.0f); // start at 0kph
	g.mDefuzzifiedSteeringGraph.createLine(mSceneMgr, 0.0f, 1.0f, 0.0f); // create first test line
	g.mDefuzzifiedSteeringGraph.setXAxisScroll(); // scrolling graph
	g.mCrashGraph.setXAxisTitle("Time (s)");
	g.mCrashGraph.setXAxisMax(30.0f);
	g.mCrashGraph.setXAxisMin(0.0f);
	g.mCrashGraph.setYAxisMin(0.0f);
	g.mCrashGraph.setYAxisMax(10.0f);
	g.mCrashGraph.setXAxisInterval(0.5f);
	g.mCrashGraph.createLine(mSceneMgr, 1.0f, 1.0f, 0.0f); // create first test line
	g.mFuzzyObstDistGraph.createLine(mSceneMgr, 1.0f, 0.0f, 0.0f);
	g.mFuzzyObstDistGraph.addPointToLine(0, 0.0f, 1.0f);
	g.mFuzzyObstDistGraph.addPointToLine(0, myFuzzyContr.mInObstacleDistanceNear[0], 1.0f);
	g.mFuzzyObstDistGraph.addPointToLine(0, myFuzzyContr.mInObstacleDistanceNear[1], 0.0f);
	g.mFuzzyObstDistGraph.createLine(mSceneMgr, 1.0f, 0.0f, 1.0f); // create 2nd set line
	g.mFuzzyObstDistGraph.addPointToLine(1, myFuzzyContr.mInObstacleDistanceFar[0], 0.0f);
	g.mFuzzyObstDistGraph.addPointToLine(1, myFuzzyContr.mInObstacleDistanceFar[1], 1.0f);
	g.mFuzzyObstDistGraph.addPointToLine(1, myFuzzyContr.mInObstacleDistanceFar[2], 1.0f);
	g.mFuzzyObstDistGraph.addPointToLine(1, myFuzzyContr.mInObstacleDistanceFar[3], 0.0f);
	g.mFuzzyObstDistGraph.createLine(mSceneMgr, 0.0f, 0.0f, 1.0f); // create 3rd set line
	g.mFuzzyObstDistGraph.addPointToLine(2, myFuzzyContr.mInObstacleDistanceVeryFar[0], 0.0f);
	g.mFuzzyObstDistGraph.addPointToLine(2, myFuzzyContr.mInObstacleDistanceVeryFar[1], 1.0f);
	g.mFuzzyObstDistGraph.createLine(mSceneMgr, 0.0f, 1.0f, 0.0f); // this will be the indicator for currently tracked targ dist
	g.mLineGraphWidgetB.createLine(mSceneMgr, 1.0f, 0.0f, 0.0f); // create first set line
	g.mLineGraphWidgetB.addPointToLine(0, 0.0f, 1.0f);
	g.mLineGraphWidgetB.addPointToLine(0, myFuzzyContr.mInTargetAngleSmall[0], 1.0f);
	g.mLineGraphWidgetB.addPointToLine(0, myFuzzyContr.mInTargetAngleSmall[1], 0.0f);
	g.mLineGraphWidgetB.createLine(mSceneMgr, 1.0f, 0.0f, 1.0f); // create 2nd set line
	g.mLineGraphWidgetB.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[0], 0.0f);
	g.mLineGraphWidgetB.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[1], 1.0f);
	g.mLineGraphWidgetB.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[2], 1.0f);
	g.mLineGraphWidgetB.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[3], 0.0f);
	g.mLineGraphWidgetB.createLine(mSceneMgr, 0.0f, 0.0f, 1.0f); // create 3rd set line
	g.mLineGraphWidgetB.addPointToLine(2, myFuzzyContr.mInTargetAngleLarge[0], 0.0f);
	g.mLineGraphWidgetB.addPointToLine(2, myFuzzyContr.mInTargetAngleLarge[1], 1.0f);
	g.mLineGraphWidgetB.createLine(mSceneMgr, 0.0f, 1.0f, 0.0f); // create first set line
	g.mFuzzyObstAngleGraph.createLine(mSceneMgr, 1.0f, 0.0f, 0.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(0, 0.0f, 1.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(0, myFuzzyContr.mInTargetAngleSmall[0], 1.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(0, myFuzzyContr.mInTargetAngleSmall[1], 0.0f);
	g.mFuzzyObstAngleGraph.createLine(mSceneMgr, 1.0f, 0.0f, 1.0f); // create 2nd set line
	g.mFuzzyObstAngleGraph.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[0], 0.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[1], 1.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[2], 1.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(1, myFuzzyContr.mInTargetAngleMedium[3], 0.0f);
	g.mFuzzyObstAngleGraph.createLine(mSceneMgr, 0.0f, 0.0f, 1.0f); // create 3rd set line
	g.mFuzzyObstAngleGraph.addPointToLine(2, myFuzzyContr.mInTargetAngleLarge[0], 0.0f);
	g.mFuzzyObstAngleGraph.addPointToLine(2, myFuzzyContr.mInTargetAngleLarge[1], 1.0f);
	g.mFuzzyObstAngleGraph.createLine(mSceneMgr, 0.0f, 1.0f, 0.0f); // this will be the indicator for currently tracked targ dist
	g.mLineGraphWidgetA.hide();
	g.mFuzzyObstDistGraph.hide();
	g.mLineGraphWidgetB.hide();
	g.mFuzzyObstAngleGraph.hide();
	g.mLineGraphWidgetC.hide();
	g.mDefuzzifiedSteeringGraph.hide();
	g.mCrashGraph.hide();
}

// initialisation specific to simulator
bool MarkIV::initSimulator() {
	printf("Initialising Simulator...\n");
	if (!mSceneMgr) {
		printf("ERROR: Initialising Sim - scene manager is NULL\n");
		return false;
	}
	mRaySceneQuery = mSceneMgr->createRayQuery(Ray());	// 3d picking ability
	if (!mCamCreated) {
		printf("ERROR: camera not created yet in MarkIV::initSimulator\n");
		return false;
	}
	if (!mTerrainHandler.initScene(mSceneMgr, mWindow, mCamera, g.mEnablePagedGeometry)) { // start terrain
		printf("ERROR: Could not init Terrain Scene defaults\n");
		return false;
	}
	if (!mTerrainHandler.initET(mCamera->getViewport()->getActualHeight())) {
		printf("ERROR: Could not load Deformable Terrain\n");
		return false;
	}
	if (!mTerrainHandler.initEditorScenery()) { // TODO replace with initSimShrubs()
		printf("ERROR: Could not init Shrubs\n");
		return false;
	}
	if (!mTerrainHandler.loadScenario(pScenarioDetails->mFilename)) { // load default scenario
		printf("ERROR: Could not load painting Brush\n");
		return false;
	}
	if (!mUnitHandler.init(mSceneMgr, &mTerrainHandler, mRaySceneQuery)) { // start unit handler
		printf("ERROR: Could not initialise UnitHandler\n");
		return false;
	}
	printf("Unit Handler initialised\n");
	initLineGraphs(); // start the graph widgets
	#ifdef _GO_GRIDS_
	if (!g.mGridOverlay.init(mSceneMgr, &mTerrainHandler, 5, 5, g.mBrainGrid.mCellSize, "biggrid", 0, 1, 0, 0)) {
		printf("ERROR: could not init grid overlay\n");
		return false;
	}
	if (!g.mSmallGridOverlay.init(mSceneMgr, &mTerrainHandler, 5, 5, g.mSmallGrid.mCellSize, "smallgrid", 0, 0.25, 0, 0)) {
		printf("ERROR: could not init mSmallGridOverlay\n");
		return false;
	}
	#endif
	int force = g.mOurForce;
	if (force < 0) {
		force = 0;
	}
	Vector3 camPos = pScenarioDetails->mForces[force].mInitialCameraPos; // just load the observer's camera position for scenario editor
	Vector3 camAim = pScenarioDetails->mForces[force].mInitialCameraAim; // just load the observer's camera aim for scenario editor
	mCamera->setPosition(camPos); // move cam into position
	mCamera->pitch(Radian(camAim.x));
	mCamera->yaw(Radian(camAim.y));
	mCamera->roll(Radian(camAim.z));
	mGameSummary.init("Simulation Summary", 0, 0, mVPWidth, mVPHeight, 0.4f, 0.4f);
	float xp = (mVPWidth - 110.0f) / mVPWidth;
	mTimeWidget.init("TimeWidget", 0, 0, mVPWidth, mVPHeight, xp, 0.0f, 110.0f, 16.0f);
	if (pScenarioDetails->mHasMusic) {
		int soundIndex = SoundManager::getSingleton().createLoopedStream(pScenarioDetails->mMusicTrack);
		int channelIndex = -1;
		SoundManager::getSingleton().playSoundB(soundIndex, NULL, &channelIndex);
	}
	return true;
}

// shutdown specific to simulator
bool MarkIV::shutdownSimulator() {
	printf("Shutting down Simulator\n");
	printf(" Stopping audio\n");
	if (!g.mNoAudio) {
		SoundManager::getSingleton().stopAllSounds();	// stop all audio
	}
	delete mRaySceneQuery;	// delete 3d-picker
	mGameSummary.free();
	mTimeWidget.free();
	#ifdef _GO_GRIDS_
	g.mSmallGridOverlay.free();
	g.mGridOverlay.free();
	#endif
	g.mCrashGraph.free(mSceneMgr);
	g.mDefuzzifiedSteeringGraph.free(mSceneMgr);
	g.mLineGraphWidgetC.free(mSceneMgr);
	g.mFuzzyObstAngleGraph.free(mSceneMgr);
	g.mLineGraphWidgetB.free(mSceneMgr);
	g.mFuzzyObstDistGraph.free(mSceneMgr);
	g.mLineGraphWidgetA.free(mSceneMgr);
	printf(" free unit handler\n");
	if (!mUnitHandler.free()) { // free anything allocated by unit handler
		printf("ERROR: unit freeing function did not work\n");
		return false;
	}
	printf(" free terrain handler\n");
	if (!mTerrainHandler.free()) { // free anything allocated by terrain handler
		printf("ERROR: could not free terrain resources\n");
		return false;
	}
	printf(" clearing scene\n");
	mSceneMgr->clearScene(); // clear scene
	printf("Simulator Shutdown done.\n");
	return true;
}

// shutdown the whole engine
bool MarkIV::shutdown() {
	printf("Shutting down Engine...\n");
	printf(" Stopping audio\n");
	if (!g.mNoAudio) {
		SoundManager::getSingleton().stopAllSounds();	// stop all audio
		delete SoundManager::getSingletonPtr(); // force deconstructor to shut off sys
	}
	mReallyQuitPopup.free();
	g.mGUI.free(); // free GUI resources
	g.mInputHandler.free(); // free input handler and key repeat
	printf("Shutdown done.\n");
	return true;
}

// display the main menu
int MarkIV::mainMenuMode() {
	mMainMenu.show(); // show GUI
	Timer myTimer; // timer to do a timeout
	myTimer.reset();
	printf("debug: entering menu loop\n");
	if (!g.mMusicAlreadyPlaying) {
		if (g.mEnableMenuMusic) {
			printf("Staring menu music...\n");
			int soundIndex = SoundManager::getSingleton().createLoopedStream(g.mMenuMusicTrack);
			int channelIndex = -1;
			SoundManager::getSingleton().playSoundB(soundIndex, NULL, &channelIndex);
			g.mMusicAlreadyPlaying = true;
			printf("Menu music started.\n");
		}
	}
	while (true) { // menu loop
		unsigned long elapsed = myTimer.getMicroseconds(); // make the loop wait if it is going too fast
		frameLimiter(elapsed);	// limit fps to 250
		elapsed = myTimer.getMicroseconds();
		myTimer.reset(); // reset the time
		g.mInputHandler.captureAll(elapsed);
		if (g.mInputHandler.isQuitKeyDown()) { // quit
			if (mReallyQuitPopup.isShowing()) {
				mReallyQuitPopup.hide(); // 'are you sure' dialogue
			} else {
				mReallyQuitPopup.show(); // 'are you sure' dialogue
			}
		}
		if (mReallyQuitPopup.isShowing()) {
			mReallyQuitPopup.update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), g.mInputHandler.isLMBDown());
			if (mReallyQuitPopup.isOKClicked()) {
				mReallyQuitPopup.hide();
				break; // quit
			}
			if (mReallyQuitPopup.isCancelClicked()) {
				mReallyQuitPopup.hide();
			}
		} else { // show main menu
			mMainMenu.update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), g.mInputHandler.isLMBDown());
			if (mMainMenu.isQuitClicked()) {
				mReallyQuitPopup.show();	// 'are you sure' dialogue
			} else if (mMainMenu.isStartSoloClicked()) {	// user clicked on 'start solo'
				mSceneMgr->clearScene();
				mMainMenu.hide();
				return 1;
			} else if (mMainMenu.isHostServerClicked()) {
				mSceneMgr->clearScene();
				mMainMenu.hide();
				g.mIsServer = true;
				return 1;
			} else if (mMainMenu.isJoinServerClicked()) { // user clicked on 'start client'
				mMainMenu.hide();
				g.mGUI.showClientMenu(); // show IP entry box // show new text capture widget
				g.mInputHandler.toggleTextCapture();	// let user enter text
				char IPText[256];
				bool goBack = false;
				while (true) { // collect IP
					g.mInputHandler.captureAll(elapsed); // update buttons pressed
					if (g.mInputHandler.isQuitKeyDown()) { // go back to main menu
						goBack = true;
						break;
					}
					if (g.mInputHandler.isEnterChatKeyDown()) {
						if (!g.mClient.init()) { // init Client
							printf("FATAL ERROR: failed to initialise client\n");
							SoundManager::getSingleton().stopAllSounds();
							return -1;
						}
						g.mClient.setIP(g.mInputHandler.getCapturedText()); // store IP in client
						break;
					}
					strcpy(IPText, g.mInputHandler.getCapturedText());
					OverlayElement* IPaddr = OverlayManager::getSingleton().getOverlayElement("mkiv/ClientIPInput");
					IPaddr->setCaption(IPText);
					g.mGUI.updateMousePosition(); // update GUI with mouse position etc
					if (!renderFrame()) { // render client menu
						printf("FATAL ERROR: Could not render client menu frame\n");
						SoundManager::getSingleton().stopAllSounds();
						return -2;
					}
				} // END OF TEXT CAPTURE
				g.mInputHandler.toggleTextCapture(); // cancel text capture
				g.mGUI.showClientMenu(false); // hide IP box
				if (goBack) {
					mMainMenu.show();
					continue;
				}
				mSceneMgr->clearScene();
				g.mIsClient = true;
				return 1;
			} else if (mMainMenu.isScenEditorClicked()) { // user clicked on 'scenario editor'
				mSceneMgr->clearScene();
				mMainMenu.hide(); // hide gui
				SoundManager::getSingleton().stopAllSounds();
				return 2;
			}
		}
		g.mGUI.updateMousePosition(); // update GUI with mouse position etc
		if (!renderFrame()) {
			printf("ERROR: Could not render menu frame\n");
			break;
		}
	}
	printf("debug: menu finished\n");
	mSceneMgr->clearScene(); // clear scene (terrain etc)
	mMainMenu.hide(); // hide GUI
	SoundManager::getSingleton().stopAllSounds();
	return 0;
}

// connecting to server.... returns true if connected or false if head back to menu for various reasons
bool MarkIV::connectingLoop() {
	g.mGUI.showConnecting(); // show "Connecting To Server..." overlay
	Timer myTimer; // timer to do a timeout
	myTimer.reset();
	unsigned long elapsed = 0;
	while (true) {
		elapsed = myTimer.getMicroseconds(); // capture time here
		frameLimiter(elapsed);	// limit fps to 250
		elapsed = myTimer.getMicroseconds();
		unsigned long countdown = 20 - (elapsed / 1000000);
		g.mGUI.updateConnectingTimer(countdown); // update countdown clock in panel
		if (elapsed > 20000000) {	// quit if timed out // 20s timeout
			printf("Failed to connect to server\n"); // TODO: add 'failed to connect' sequence here
			g.mGUI.showConnecting(false);	// hide the connecting... overlay
			return false;
		}
		if (!renderFrame()) { // render gui
			printf("FATAL ERROR: Could not render overlay frame\n");
			exit(1);
		}
		g.mGUI.updateMousePosition(); // update GUI with mouse position etc (so we now its still alive)
		g.mInputHandler.captureAll(elapsed);
		if (g.mInputHandler.isQuitKeyDown()) { // quit
			g.mGUI.showConnecting(false);	// hide the connecting... overlay
			return false;
		}
		if (!g.mClient.handleMail()) { 	// check for connection notice
			printf("FATAL ERROR: Client blew up when checking messages\n");
			exit(1);
		}
		if (g.mClient.isConnected()) { // if connected - break
			break;
		}
	}
	g.mGUI.showConnecting(false);	// hide the connecting... overlay
	return true;
}

bool MarkIV::showLoadingOverlay() {
	g.mGUI.showLoading(true);	// show loading scenario overlay
	if (!renderFrame()) { // render gui
		printf("FATAL ERROR: Could not render overlay frame\n");
		exit(1);
	}
	g.mGUI.showLoading(false);		// hide loading scenario overlay
	return true;
}

// start the lobby/scenario chooser
int MarkIV::lobbyLoop() {
	int type = 0;
	if (g.mIsClient) { // load some panels
		type = 1;
		g.mGUI.clearAllLobbyLists();
	} else if (g.mIsServer) {
		type = 2;
		if (!g.mServer.updateForceLists()) {
			printf("ERROR: updating force lists from server\n");
		}
	} else {
		g.mGUI.clearAllLobbyLists();
		g.mGUI.pushLobbySpecName(g.mUserName);
	}
	pLobbyMenu = new LobbyMenu; // load and start the lobby menu
	if (!pLobbyMenu->init("mkiv/LobbyMenu", 0.0f, 0.0f, mVPWidth, mVPHeight, 0.0f, 0.0f, type)) {
		printf("ERROR: Could not load lobby menu.\n");
		return -2;
	}
	if (!pLobbyMenu->show()) {
		printf("ERROR: Could not display lobby menu.\n");
		return -2;
	}
	Timer myTimer; // timer to do a timeout
	Timer serverUpdateTimer;
	myTimer.reset();
	serverUpdateTimer.reset();
	unsigned long elapsed = 0;
	int readyCount = 0;
	int notReadyCount = 0;
	while (true) { // do a loop
		elapsed = myTimer.getMicroseconds(); // capture time here
		frameLimiter(elapsed);	// limit fps to 250
		elapsed = myTimer.getMicroseconds();
		myTimer.reset(); // reset the time
		g.mInputHandler.captureAll(elapsed); // update buttons pressed
		if (g.mInputHandler.isQuitKeyDown()) { // quit
			if (mInChatMode) {
				handleChatEntry();	// stop capturing of chat
			} else if (!pLobbyMenu->closeAnyMenus()) {
				pLobbyMenu->free();
				delete pLobbyMenu;
				pLobbyMenu = NULL;
				return 0;	// return to main menu
			}
		}
		if ((g.mIsServer) || (g.mIsClient)) {	 // check if chat mode button pressed (if networking)
			if (g.mInputHandler.isEnterChatKeyDown()) {
				handleChatEntry();	// toggle capturing of chat
			}
		}
		OverlayElement* chatEntry = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatEntryInput");
		chatEntry->setCaption(g.mInputHandler.getCapturedText()); // assign chat to chat entry thingy
		if (pLobbyMenu->isStartClicked()) { // check if 'start' selected (only on server or solo)
			if (g.mIsServer) {
				pScenarioDetails = pLobbyMenu->getSelectedScenarioDetails();
				g.mTimeoutUS = pScenarioDetails->mTimeLimitMins * 60000000;
				g.mServer.sendStart();	// send start message to all clients
				pLobbyMenu->free();
				delete pLobbyMenu;
				pLobbyMenu = NULL;
				return 1;	// start
			} else if (g.mIsClient) {
				if (pScenarioDetails == NULL) {
					pScenarioDetails = new ScenarioDetails;
				}
				pScenarioDetails->loadFromFile(g.mScenario);
				g.mTimeoutUS = pScenarioDetails->mTimeLimitMins * 60000000;
				pLobbyMenu->free();
				delete pLobbyMenu;
				pLobbyMenu = NULL;
				return 1;	// start
			} else {
				pScenarioDetails = pLobbyMenu->getSelectedScenarioDetails();
				g.mTimeoutUS = pScenarioDetails->mTimeLimitMins * 60000000;
				pLobbyMenu->free();
				delete pLobbyMenu;
				pLobbyMenu = NULL;
				return 1;	// start loading simulator
			}
		}
		if (pLobbyMenu->isReadyClicked()) {
			if (g.mIsServer) {
				g.mServer.toggleReadyStatus();
			} else if (g.mIsClient) {
				g.mClient.toggleReadyStatus();
			}
		}
		if (pLobbyMenu->isCancelClicked()) {
			pLobbyMenu->free();
			delete pLobbyMenu;
			pLobbyMenu = NULL;
			return 0;	// return to main menu
		}
		if (g.mIsServer) { // update networking
			if (serverUpdateTimer.getMicroseconds() > 500000) {	// intermitantly ask the clients if they are ready // every 0.5 seconds
				serverUpdateTimer.reset();	// reset server send timer
				readyCount = g.mServer.getReadyCount();
				notReadyCount = g.mServer.getNotReadyCount();
				pLobbyMenu->updateReadyCounter(readyCount, notReadyCount);	// update gui
				g.mServer.sendCounter();	// send ready counter to clients
				g.mServer.queryClientsReady();	// ask the clients to tell us if they are ready or not
				myTimer.reset();
			}
			g.mServer.handleMail(); // check mail
		}
		if (g.mIsClient) {
			g.mClient.handleMail(); // check messages
			readyCount = g.mClient.getReadyCount(); // update count of ready players as reported by server
			notReadyCount = g.mClient.getNotReadyCount();
			pLobbyMenu->updateReadyCounter(readyCount, notReadyCount);	// update gui
			if (g.mNeedToUpdateScenario) {
				if (!pLobbyMenu->useGlobalScenario()) {
					// TODO a popup that says "MISSING SCENARIO"
					printf("WARNING: Missing scenario %s required by server\n", g.mScenario);
				}
				g.mNeedToUpdateScenario = false;
			}
			if (g.mClient.gotStartMessage()) { // start when ordered by server
				pScenarioDetails = pLobbyMenu->getSelectedScenarioDetails();
				g.mTimeoutUS = pScenarioDetails->mTimeLimitMins * 60000000;
				pLobbyMenu->free();
				delete pLobbyMenu;
				pLobbyMenu = NULL;
				return 1;
			}
		}
		g.mGUI.updateMousePosition(); // update GUI with mouse position etc
		pLobbyMenu->update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), g.mInputHandler.isLMBDown());
		if (!renderFrame()) {
			printf("ERROR: Could not render menu frame\n");
			pLobbyMenu->free();
			delete pLobbyMenu;
			pLobbyMenu = NULL;
			return -1;
		}
	} // TODO: update panels based on other players status
	printf("DEBUG:::::: LOADING SCENARIO FROM LOBBY %s\n",pLobbyMenu->getSelectedScenarioDetails()->mFilename);
	pScenarioDetails = pLobbyMenu->getSelectedScenarioDetails();
	g.mTimeoutUS = pScenarioDetails->mTimeLimitMins * 60000000;
	pLobbyMenu->free();
	delete pLobbyMenu;
	pLobbyMenu = NULL;
	return 1;	// start loading simulator
}

// the main function of the simulator
bool MarkIV::mainGameLoop() {
	Timer realWorldTimer; // used for real-world time (for moving the camera) in visualisation layer
	Timer simulationTimer; // used for simulation time (for moving tanks) and can be paused and compressed
	Timer pausedTimer; // collects time pause has been enabled
	g.mSimTimer.reset(); // timer for entire sim run time (accumulates over all loops)
	realWorldTimer.reset(); // start timer
	simulationTimer.reset(); // start timer
	unsigned long elapsedRealTime = 0; // counter that is used as the 'current' time slice variable
	unsigned long elapsedSimulationTime = 0; // counter that is used as the 'current' time slice variable
	g.mTotalTimePaused = 0; // get ready to accumulate paused time
	while (true) { // === Main Simulation Loop ===
		// update timers
		elapsedRealTime = realWorldTimer.getMicroseconds(); // tentatively update time slice
		frameLimiter(elapsedRealTime); // limit visualisation time to 250 frames per second (wait if time slice too short)
		elapsedRealTime = realWorldTimer.getMicroseconds(); // get adjusted time for slice (which is constant throughout slice).
		realWorldTimer.reset(); // start the real world timer again
		if (!g.mIsPaused) { // update simulation time only if not paused
			// capture time here
			elapsedSimulationTime = simulationTimer.getMicroseconds(); // tentatively update time slice
			frameLimiter(elapsedSimulationTime); // limit fps to 250 (wait if time slice too short)
			elapsedSimulationTime = simulationTimer.getMicroseconds(); // get adjusted time for slice (which is constant throughout slice).
		} else {
			elapsedSimulationTime = 0; // simulation time not passing
		}
		double adjustedSimTime = (double)elapsedSimulationTime * g.mTimeCompressionMultiplier; // eval time compression
		elapsedSimulationTime = (unsigned long)adjustedSimTime; // truncate time (hopefully this works)
		simulationTimer.reset(); // reset the timer so that we can time how long this frame took to process
		// check game end conditions
		unsigned long totalTime = g.mSimTimer.getMicroseconds() - g.mTotalTimePaused; // total sim time
		if (g.mTimeoutUS > 0) { // if there is a simulation time limit...
			char text[128];
			unsigned long timeMins = (g.mTimeoutUS - totalTime) / 60000000;
			double seconds = (double)(g.mTimeoutUS - totalTime) / 1000000.0;
			seconds = seconds - timeMins * 60;
			sprintf(text, "Time left %li:%.0f\n", timeMins, seconds);
			mTimeWidget.setText(text);
			mTimeWidget.show();
			if (totalTime >= g.mTimeoutUS) { // check for timeout (simulation time exceeded)
				g.mGameOver = true; // flag game over
			}
		}
		if (g.mGameOver) { // flag to shutdown simulation
			mTimeWidget.hide();
			char summary[1024];
			sprintf(summary, "Our force:\nUnits at start: %i\nUnits at end: %i\nKills: %i", g.mScores[g.mOurForce].mUnitsStart, g.mScores[g.mOurForce].mUnitsRemaining, g.mScores[g.mOurForce].mUnitKills);
			mGameSummary.setQuestion(summary);
			mGameSummary.show();
			g.mIsPaused = true;
		}
		char txt[256];
		unsigned long mins = totalTime / 60000000;
		double secs = (double)totalTime / 1000000.0;
		secs = secs - mins * (double)60.0;
		sprintf(txt, "Time: %li:%.0lf", mins, secs);
		OverlayManager::getSingleton().getOverlayElement("mkiv/GFSTime")->setCaption(txt);
		g.mInputHandler.captureAll(elapsedRealTime); // update buttons pressed
		if (g.mInputHandler.isQuitKeyDown()) { // quit
			if (mReallyQuitPopup.isShowing()) {
				mReallyQuitPopup.hide();
			} else {
				mReallyQuitPopup.show();
			}
		}
		if (mReallyQuitPopup.isShowing()) {
			mReallyQuitPopup.update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), g.mInputHandler.isLMBDown());
			if (mReallyQuitPopup.isOKClicked()) {
				mReallyQuitPopup.hide();
				break;
			} else if (mReallyQuitPopup.isCancelClicked()) {
				mReallyQuitPopup.hide();
			}
		}
		if (mGameSummary.isShowing()) {
			mGameSummary.update(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY(), g.mInputHandler.isLMBDown());
			if (mGameSummary.isOKClicked()) {
				mGameSummary.hide();
				return true; // quit
			}
		}
		if (g.mInputHandler.isChaseCamKeyDown()) {
			if (mChaseCamTargetVehicleIndex < 0) {
				mChaseCamTargetVehicleIndex = mUnitHandler.mSelectedVehicleIndex;
			} else {
				mChaseCamTargetVehicleIndex = -1;
			}
		}

		if (g.mInputHandler.isPauseKeyDown()) { // pause/unpause game
			if (!g.mIsPaused) {
				pausedTimer.reset(); // start a timer
			} else {
				g.mTotalTimePaused += pausedTimer.getMicroseconds(); // add time
			}
			g.mIsPaused = !g.mIsPaused;
			printf("pause key down!\n");
			// TODO network message here (request server for pause or instruct clients to pause)
		}
		if (g.mInputHandler.isStatsModeKeyDown()) { // hide/show graphs
			if (g.mNavGraphsShowing) {
				g.mLineGraphWidgetA.hide();
				g.mFuzzyObstDistGraph.hide();
				g.mLineGraphWidgetB.hide();
				g.mFuzzyObstAngleGraph.hide();
				g.mLineGraphWidgetC.hide();
				g.mDefuzzifiedSteeringGraph.hide();
				g.mCrashGraph.hide();
				#ifdef _GO_GRIDS_
				g.mGridOverlay.hide();
				g.mSmallGridOverlay.hide();
				#endif
				g.mNavGraphsShowing = false;
			} else {
				g.mLineGraphWidgetA.show();
				g.mFuzzyObstDistGraph.show();
				g.mLineGraphWidgetB.show();
				g.mFuzzyObstAngleGraph.show();
				g.mLineGraphWidgetC.show();
				g.mDefuzzifiedSteeringGraph.show();
				g.mCrashGraph.show();
				//g.mGridOverlay.show(g.mBrainGrid);
				//g.mSmallGridOverlay.show(g.mSmallGrid);
				g.mNavGraphsShowing = true;
			}
		}
		if (g.mInputHandler.isSavePlotKeyDown()) { // save graph points to file
			// save all lines from all graphs (graphmanager class?)
			g.mLineGraphWidgetC.savePlotToFile(0, false); // csv format
			g.mLineGraphWidgetC.savePlotToFile(0, true); // gnuplot format
			g.mDefuzzifiedSteeringGraph.savePlotToFile(0, false); // csv format
			g.mDefuzzifiedSteeringGraph.savePlotToFile(0, true); // csv format
			g.mCrashGraph.savePlotToFile(0, false);
			g.mCrashGraph.savePlotToFile(0, true);
		}
		if (g.mInputHandler.isEnterChatKeyDown()) { // toggle capturing of chat
			handleChatEntry(); // check if chat mode button pressed
		}
		OverlayElement* chatEntry = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatEntryInput");
		chatEntry->setCaption(g.mInputHandler.getCapturedText()); // assign chat to chat entry thingy
		if(!mInChatMode) {
			if (g.mInputHandler.isTogglePointerKeyDown()) { // set global modes
				g.mPointerMode = !g.mPointerMode;
			}
			if (g.mInputHandler.isToggleDebugPanelKeyDown()) { // display debug panel here if key down
				if (g.mGUI.isShowingDebugPanel()) {
					g.mGUI.showDebug(false);
				} else {
					g.mGUI.showDebug(true);
				}
			}
			if (!updateCamera(elapsedRealTime)) { // update the camera
				printf("FATAL ERROR: updating camera!\n");
				return false;	// return false quit and clean up resources
			}
		}
		g.mGUI.updateMousePosition(); // update GUI with mouse position etc
		SoundManager::getSingleton().update(mCamera, elapsedRealTime); // update audio based on camera movement (doppler)
		if (!update(elapsedSimulationTime)) {
			printf("ERROR: running update function\n");
			break;	// break will try again
		}
		if (!renderFrame()) { // run message pump and then render
			printf("FATAL ERROR: running update function\n");
			break; // break will try again
		}
	}
	g.mGUI.hideAllPanels(); // hide all the overlays
	return true;
}

// capture user-entered chat
void MarkIV::handleChatEntry() {
	mInChatMode = !mInChatMode;	// toggle chat mode
	g.mGUI.showChatEntry(mInChatMode); // toggle text entry box display
	if (!mInChatMode) {
		char* capturedText;
		capturedText = g.mInputHandler.getCapturedText(); // get captured text (now that it is completed)
		char tmp[50]; // parse to make sure there is actually something in there
		sscanf(capturedText,"%s",tmp); // use sscanf to make sure we don't count whitespace as content
		if (strlen(tmp) == 0) { // do nothing
		} else if (strncmp(capturedText,"/setName",8) == 0) {// parse for system commands
			char name[15];
			StripString(capturedText,9,name,15);
			strcpy(g.mUserName,name);	// just set name locally
			if (g.mIsClient) {
				g.mClient.login(name);	// tell server what our name is
			} if (g.mIsServer) {
				g.mServer.updateForceLists();
			}
		} else if (strncmp(capturedText,"/ping",5) == 0) {
			if (g.mIsServer) {
				int target = 0;
				sscanf(capturedText,"/ping %i",&target);
				g.mServer.ping(target);
			} else if (g.mIsClient) {
				g.mClient.ping();
			}
		} else if (strncmp(capturedText,"/setReady",9) == 0) {
			if (g.mIsServer) {
				g.mServer.setReadyStatus(true);
			} else if (g.mIsClient) {
				g.mClient.setReadyStatus(true);
			}
		} else if (strncmp(capturedText,"/setNotReady",12) == 0) {
			if (g.mIsServer) {
				g.mServer.setReadyStatus(false);
			} else if (g.mIsClient) {
				g.mClient.setReadyStatus(false);
			}
		} else if (g.mIsClient) { // send captured text if !inChatMode
			g.mClient.sendMsg(capturedText);
			char tmp[256];
			sprintf(tmp,"%s: %s",g.mUserName,capturedText);
			g.mGUI.addChat(tmp); // print the message so we can see it
		} else if (g.mIsServer) {
			g.mServer.sendChat(capturedText);
			char tmp[256];
			sprintf(tmp,"%s: %s",g.mUserName,capturedText);
			g.mGUI.addChat(tmp);
		}
	}
	g.mInputHandler.toggleTextCapture();	// toggle text entry ability
}

// update game components per-frame
bool MarkIV::update(unsigned long &elapsed) {
	if (g.mGUI.isShowingDebugPanel()) { // update debug panel
		g.mGUI.updateStats(mWindow);
	}
	checkMouseClicks();
	if (g.mIsPaused) {
		return true;
	} // Remake Dynamic Obstacle Map
	#ifdef _GO_GRIDS_
	g.mBrainGrid.clearDynamicObstacles(); // clear map
	g.mSmallGrid.clearDynamicObstacles(); // clear map
	#endif
	if (!mUnitHandler.addAllToObstacleMap()) { // add dynamic obsts (separate from static to save CPU)
		printf("ERROR: could not record dynamic obstacles. MarkIV::update()\n");
		return false;
	}
	if (!mTerrainHandler.updateEnvironment(elapsed)) {
		printf("ERROR: could not update environment in MarkIV::update()\n");
		return false;
	}
	if (!mUnitHandler.update(elapsed)) {
		return false;
	}
	if (g.mIsServer) { // update networking
		g.mServer.handleMail();
	}
	if (g.mIsClient) { // update networking
		g.mClient.handleMail();
	}
	return true;
}

bool MarkIV::renderFrame() {
	WindowEventUtilities::messagePump(); // run message pump
	if (!mRoot->renderOneFrame()) { // render frame
		return false;
	}
	return true;
}

// Show the configuration dialog and initialise the system
bool MarkIV::userConfigure() {
	if (g.mSuppressUserGfxChooser) {
		mRoot->restoreConfig();
		mWindow = mRoot->initialise(true);
		return true;
	}
	if (mRoot->showConfigDialog()) { // If returned true, user clicked OK so initialise
		mWindow = mRoot->initialise(true); // Here we choose to let the system create a default rendering window by passing 'true'
		return true;
	}
	return false;
}

// Create the SceneManager, in this case an exterior one
bool MarkIV::chooseSceneManager() {
printf("db: choose sm\n");
	mSceneMgr = mRoot->createSceneManager(ST_GENERIC, "mkivSceneManager"); // Create the SceneManager, in this case the "generic" option which loads octtree if the Plugin_OctreeSceneManager is loaded. recommended for ETM
printf("db: choose rggo\n");
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups(); // to allow access to extra textures
printf("db: choose rg init\n");
	if (g.mEnableShadows) {
		mSceneMgr->setShadowColour(ColourValue(0.6, 0.6, 0.6));
		mSceneMgr->setShadowTechnique(SHADOWTYPE_STENCIL_MODULATIVE); // enable shadows
		mSceneMgr->setShadowFarDistance(250.0f);
		mSceneMgr->setShadowUseInfiniteFarPlane(false); // to avoid odd shadows with camera farclip
	}
	return true;
}

// Create the camera
bool MarkIV::createCamera() {
	mCamera = mSceneMgr->createCamera("PlayerCam"); // Create the camera
	mCamera->setPosition(Vector3(400,150,600)); // Position it at 500 in Z direction
	mCamera->pitch(Degree(-25));
	mCamera->yaw(Degree(-35)); // Look back along -Z
	mCamera->setNearClipDistance(g.mCamNearClip);
	mCamera->setFarClipDistance(g.mCamFarClip);
	mCamera->setPolygonMode(Ogre::PolygonMode(g.mCamPolygonMode));
	mCamRotateSpeed = 0.00007;
	mCamMoveSpeed = 0.0001; // 360km/h
	mCamMoveScale = 0.0f;
	mCamRotScale = 0.0f;
	mCamTranslateVector = Vector3::ZERO;
	mCamCreated = true;
	return true;
}

// Create one viewport, entire window
bool MarkIV::createViewports() {
    Viewport* vp = mWindow->addViewport(mCamera); // Create one viewport, entire window
    vp->setBackgroundColour(ColourValue(0,0,0));
    mVPWidth = vp->getActualWidth();
    mVPHeight = vp->getActualHeight();
    mCamera->setAspectRatio(Real(mVPWidth) / Real(mVPHeight)); // Alter the camera aspect ratio to match the viewport
		if (g.mUseCompositor) {
			CompositorManager::getSingleton().addCompositor(vp, g.mCompositorName);
			CompositorManager::getSingleton().setCompositorEnabled(vp, g.mCompositorName, true);
		}
    return true;
}

// move the camera
bool MarkIV::updateCamera(unsigned long &elapsed) {
	if (mChaseCamTargetVehicleIndex > -1) {
		chaseCam(mChaseCamTargetVehicleIndex);
		Vector3 camPos = mCamera->getPosition();
		//mTerrainHandler.updateShrubCulling(camPos); // update scenery handler
		char text[100];
		sprintf(text, "CamPos[%.0f] [%.0f] [%.0f]", camPos.x, camPos.y, camPos.z);
		g.mGUI.updateCameraText(text); // update camera debug text
		return true;
	}
	bool camMoved = false;
	mCamTranslateVector = Vector3::ZERO; // reset translation vector
	mCamMoveScale = mCamMoveSpeed * elapsed; // Move about 100 units per second,
	mCamRotScale = mCamRotateSpeed * elapsed; // Take about 10 seconds for full rotation
	if (g.mInputHandler.isForwardKeyDown()) { // move cam fwd
		mCamTranslateVector.z = -mCamMoveScale;
		camMoved = true;
	}
	if (g.mInputHandler.isBackwardKeyDown()) { // move cam bkwd
		mCamTranslateVector.z = mCamMoveScale;
		camMoved = true;
	}
	if (g.mInputHandler.isSlideLeftKeyDown()) { // move cam left
		mCamTranslateVector.x = -mCamMoveScale;
		camMoved = true;
	}
	if (g.mInputHandler.isSlideRightKeyDown()) { // move cam right
		mCamTranslateVector.x = mCamMoveScale;
		camMoved = true;
	}
	if (g.mInputHandler.isFlyUpKeyDown()) { // move cam up
		mCamTranslateVector.y = mCamMoveScale;
		camMoved = true;
	}
	if (g.mInputHandler.isFlyDownKeyDown()) { // move cam down
		mCamTranslateVector.y = -mCamMoveScale;
		camMoved = true;
	}
	if (g.mInputHandler.isTurnUpKeyDown()) { // rotate cam up
		mCamera->pitch(mCamRotScale);
	}
	if (g.mInputHandler.isTurnDownKeyDown()) { // rotate cam down
		mCamera->pitch(-mCamRotScale);
	}
	if (g.mInputHandler.isTurnLeftKeyDown()) { // rotate cam left
		mCamera->yaw(mCamRotScale);
	}
	if (g.mInputHandler.isTurnRightKeyDown()) { // rotate cam right
		mCamera->yaw(-mCamRotScale);
	}
	if (!g.mPointerMode) { // handle mouse and rotation if in appropriate mode
		mCamRotX = Degree(-g.mInputHandler.getMouseRelX() * 0.3);
    mCamRotY = Degree(-g.mInputHandler.getMouseRelY() * 0.3);
		mCamera->yaw(mCamRotX);
		mCamera->pitch(mCamRotY);
	}
	if (camMoved) { // update camera position
    mCamera->moveRelative(mCamTranslateVector);
		Vector3 camPos = mCamera->getPosition();
		double terrainHeight = mTerrainHandler.getTerrainHeightAt(camPos.x,camPos.z); // make sure don't fall through terrain by using Ray vector (scene query)
		if (camPos.y < terrainHeight + 2.0f) {
			mCamera->setPosition(camPos.x, terrainHeight + 2.0f, camPos.z);
		}
		//mTerrainHandler.updateShrubCulling(mCamera->getPosition()); // update scenery handler
		char text[100];
		sprintf(text, "CamPos[%.0f] [%.0f] [%.0f]", camPos.x, camPos.y, camPos.z);
		g.mGUI.updateCameraText(text); // update camera debug text
	}
	return true;
}

bool MarkIV::chaseCam(int vehicleIndex) {
	Unit* veh = NULL;
	veh = mUnitHandler.getVehicleAtIndex(vehicleIndex);
	if (!veh) {
		printf("ERROR: chase cam failed because vehicle index %i not valid. MarkIV::chaseCam()\n", vehicleIndex);
		return false;
	}
	mCamera->detachFromParent();
	Vector3 pos;
	veh->getPosition(pos.x, pos.y, pos.z);
	pos.y = pos.y + 7.0f; // 7m above ground
	double xOffset	= sinf(veh->mHeading) * (4.0f);
	double zOffset	= -cosf(veh->mHeading) * (4.0f);
	pos.x = pos.x - xOffset;
	pos.z = pos.z + zOffset;
	mCamera->setPosition(pos); // move cam into position
	if (!g.mPointerMode) { // handle mouse and rotation if in appropriate mode
		mCamRotX = Degree(-g.mInputHandler.getMouseRelX() * 0.3);
    mCamRotY = Degree(-g.mInputHandler.getMouseRelY() * 0.3);
		mCamera->yaw(mCamRotX);
		mCamera->pitch(mCamRotY);
	}
	return true;
}

// check if user clicked on vehicles or terrain
void MarkIV::checkMouseClicks() {
	mUnitHandler.setSelectedNode(NULL);
	mUnitHandler.setTargetedNode(NULL);
	if (g.mInputHandler.isLMBDown()) { // Left mouse button down
		Ray mouseRay = mCamera->getCameraToViewportRay(g.mInputHandler.getMouseX()/mVPWidth,g.mInputHandler.getMouseY()/mVPHeight); // Setup the ray scene query
		mRaySceneQuery->setRay( mouseRay );
		mRaySceneQuery->setSortByDistance( true );
		RaySceneQueryResult &result = mRaySceneQuery->execute(); // Execute query
		RaySceneQueryResult::iterator itr = result.begin();
		bool hitTerrain = false;
		for (itr = result.begin(); itr != result.end(); itr++) { // Get results, create a node/entity on the position
			if (itr->movable && itr->movable->getName().substr(0,5) == "vehic") {	// differentiates between units and scenery
				mUnitHandler.setSelectedNode(itr->movable->getParentSceneNode()); // tell the unit handler which node was selected (so it can match it to a vehicle)
				return;
			} else if (mTerrainHandler.checkMouseIntersect(mouseRay)) { // otherwise check if hit terrain
				hitTerrain = true;
			}
		}
		if (hitTerrain) { // if hit terrain but not unit then deselect
			mUnitHandler.deselectAll(); // deselect all units
		}
	} else if (g.mInputHandler.isRMBDown()) {
		Ray mouseRay = mCamera->getCameraToViewportRay(g.mInputHandler.getMouseX()/mVPWidth,g.mInputHandler.getMouseY()/mVPHeight); // Setup the ray scene query
		mRaySceneQuery->setRay( mouseRay );
		mRaySceneQuery->setSortByDistance( true );
		RaySceneQueryResult &result = mRaySceneQuery->execute(); 	// Execute query
		RaySceneQueryResult::iterator itr = result.begin();
		bool hitTerrain = false;
		Vector3 destination;
		for (itr = result.begin(); itr != result.end(); itr++) { // Get results, create a node/entity on the position
			if (itr->movable && itr->movable->getName().substr(0,5) == "vehic") { // check if found a moveable object (excluding terrain tiles)
				mUnitHandler.setTargetedNode(itr->movable->getParentSceneNode());	//target
				return;
			} else if (mTerrainHandler.checkMouseIntersect(mouseRay)) { // otherwise check if hit terrain
				hitTerrain = true;
				destination = mTerrainHandler.getMousePosition();
			}
		}
		if (hitTerrain) {
			mUnitHandler.orderDestination(destination); // send destination as order
		}
	}
}

bool MarkIV::initScenarioEditor() {
	mpScenarioEditor = new ScenarioEditor;
	if (!mpScenarioEditor) {
		printf("ERROR: Could not alloc scen editor\n");
		return false;
	}
	if (!mpScenarioEditor->init(mSceneMgr, mCamera, mWindow, &mTerrainHandler, mVPWidth, mVPHeight)) {
		printf("ERROR: Could not init scen editor\n");
		return false;
	}
	return true;
}

bool MarkIV::shutdownScenarioEditor() {
	if (!mpScenarioEditor) {
		printf("ERROR:  scen editor not alloced\n");
		return false;
	}
	if (!mpScenarioEditor->free(mSceneMgr)) {
		printf("ERROR: Could not init scen editor\n");
		return false;
	}
	return true;
}

bool MarkIV::mainEditorLoop() {
	if (!mpScenarioEditor) {
		printf("ERROR:  scen editor not alloced\n");
		return false;
	}
	unsigned long elapsed = 0;
	Timer myTimer;
	myTimer.reset();
	while (mpScenarioEditor->update(myTimer, elapsed)) {
		if (!g.mInputHandler.isCapturingText()) {
			if (!updateCamera(elapsed)) { // update the camera unless user is typing 'w's and 's's
				printf("FATAL ERROR: updating camera!\n");
				return false;	// return false quit and clean up resources
			}
		}
		if (!renderFrame()) { // render
			printf("FATAL ERROR: running update function\n");
			break; // break will try again
		}
	} // end while
	g.mGUI.hideAllPanels();	// hide all the overlays
	return true;
}

bool MarkIV::setScenarioToLoad(const char* scenfile) {
	if (pScenarioDetails != NULL) {
		printf("ERROR: already loaded a scenario file into memory\n");
		return false;
	}
	pScenarioDetails = new ScenarioDetails;
	if (!pScenarioDetails) {
		printf("ERROR: memory overflow\n");
		return false;
	}
	pScenarioDetails->loadFromFile(scenfile);
	g.mTimeoutUS = pScenarioDetails->mTimeLimitMins * 60000000;
	return true;
}

