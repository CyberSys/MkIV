#ifndef MARKIV_H_
#define MARKIV_H_

#include "Ogre.h"
#include "ScenarioEditor.h"
#include "units/UnitHandler.h"
#include "gui/MainMenu.h"
#include "gui/LobbyMenu.h"
#include "gui/YesNoBoxWidget.h"
#include "scenery/ScenarioDetails.h"
#include "gui/InfoBoxWidget.h"

using namespace Ogre;

// This is the main application class
class MarkIV {
public:
	MarkIV();
	bool init();
	bool shutdown();
	int  mainMenuMode();
	int lobbyLoop();
	bool connectingLoop();
	bool showLoadingOverlay();
	bool startServer();
	bool shutdownServer();
	int startClient();
	bool shutdownClient();
	bool initSimulator();
	bool shutdownSimulator();
	bool mainGameLoop();
	bool initScenarioEditor();
	bool shutdownScenarioEditor();
	bool mainEditorLoop();
	bool setScenarioToLoad(const char* scenfile);
private:
	bool userConfigure();
	bool chooseSceneManager();
	bool createCamera();
	bool createViewports();
	bool update(unsigned long &elapsed);
	bool renderFrame();
	bool updateCamera(unsigned long &elapsed);
	bool chaseCam(int vehicleIndex);
	void checkMouseClicks();
	void handleChatEntry();
	void initLineGraphs();
	bool loadSettingsINIFile();
	Root* mRoot;
	SceneManager* mSceneMgr;
	RenderWindow* mWindow;
	Camera* mCamera;
	RaySceneQuery* mRaySceneQuery;	// scene query for checking mouse clicks
	MainMenu mMainMenu;
	LobbyMenu* pLobbyMenu;
	UnitHandler mUnitHandler;
	TerrainHandler mTerrainHandler;
	ScenarioEditor* mpScenarioEditor;
	ScenarioDetails* pScenarioDetails;
	Vector3 mCamTranslateVector;
	Degree mCamRotScale;
	Degree mCamRotX;
	Degree mCamRotY;
	YesNoBoxWidget mReallyQuitPopup;
	YesNoBoxWidget mGameSummary;
	InfoBoxWidget mTimeWidget;
	float mVPWidth;	// viewport (display) dimensions
	float mVPHeight;
	double mCamRotateSpeed;
	double mCamMoveSpeed;
	double mCamMoveScale;
	int mChaseCamTargetVehicleIndex;
	bool mInChatMode;
	bool mCamCreated;
};

#endif /*MARKIV_H_*/

