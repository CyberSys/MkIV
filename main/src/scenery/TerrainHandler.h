#ifndef TERRAINHANDLER_H_
#define TERRAINHANDLER_H_

/* Terrain and Scenery Handler class for MarkIV engine
	 TODO specialise this class as just terrain management, put other stuff in other handlers, and create an overall scenery manager
 */

//#define NOPAGED

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "ScenarioDetails.h"
#include "ETM/ETTerrainManager.h"
#include "ETM/ETTerrainInfo.h"
#include "ETM/ETBrush.h"
#include "ETM/ETSplattingManager.h"
#include "ShrubDef.h"
#include "Shrub.h"
#include "BuildingDef.h"
#include "Building.h"
#ifndef NOPAGED
 #include "PagedGeoWrapper.h"
#endif
#include "WaterPlane.h"

using namespace Ogre;
using namespace std;

// maximum numbers of things allowed in one game (memory limit)
#define SHRUB_DEF_MAX 256
#define SHRUB_HASH_MAX 4096
#define BUILDING_DEF_MAX 256
#define BUILDING_HASH_MAX 2048

struct Fog {
	FogMode mMode;
	ColourValue mColour;
	double mDensity;
	double mLinearStart_m;
	double mLinearEnd_m;
	Fog();
};

class TerrainHandler {
public:
	TerrainHandler();
	bool initScene(Ogre::SceneManager* sceneMgr, Ogre::RenderWindow* window, Ogre::Camera* cam, bool paged);
	bool initET(unsigned int viewportHeight);
	bool initEditorScenery();
	bool free();
	bool freeAllShrubs();
	bool freeAllBuildings();
	bool update(const unsigned long& elapsed);
	bool updateEnvironment(const unsigned long& elapsed);
	bool deselectAll();
	bool saveScenario(const char *name);
	bool loadScenario(const char *name);
	bool saveShrubs(const char *name);
	bool loadShrubs(const char *name);
	bool saveBuildings(const char *name);
	bool loadBuildings(const char *name);
	void setPointerPosition(const Vector3 &pointer);
	bool createEditBrush(int size = 16);	// temporary brush-loading function
	bool setBrushSizeSmall();
	bool setBrushSizeMed();
	bool setBrushSizeLarge();
	void setSelectedNode(SceneNode* node);
	bool moveSelectionWithMouse();
	bool deleteSelection();
	bool posRotSelection(const long &elapsed);
	bool negRotSelection(const long &elapsed);
	bool getNearestObstacleTo(const Ogre::Vector3& origin, double currentHeading, double angleThresh, Vector3& obstacle, double& distanceSquared, double& sizeRadius, Ogre::AxisAlignedBox& boundingBox);
	bool storeTerrainHeightsInGrid();
	double getTerrainHeightAt(const double& xp, const double& zp);
	double getChangeInHeadingTo(const Ogre::Vector3& origin, const double& currentHeading, const Ogre::Vector3& target);
	bool checkMouseIntersect(Ray &mouseRay);
	Ogre::Vector3 getMousePosition();
	Ogre::Vector3 getTerrainMinExtents();
	Ogre::Vector3 getTerrainMaxExtents();
	bool deform(unsigned long elapsed, bool up);
	bool paint(unsigned long elapsed, bool positive = true);
	bool updateLightmap();
	void setCurrentPaint(int num);
	int createShrub(int defIndex, Vector3& pos, double yaw);
	int createShrub(const char* defFile, Vector3& pos, double yaw);
	int createShrubsAtPointer(const char* defFile, int clumpSize);
	int createBuilding(int defIndex, Vector3& pos, double yaw);
	int createBuilding(const char* defFile, Vector3& pos, double yaw);
	int createBuildingAtPointer(const char* defFile);
	bool moveShrubWithMouse(int shrubID);
	bool moveBuildingWithMouse(int buildingID);
	void updateShrubCulling(Vector3 camPos);
	void updateBuildingCulling(Vector3 camPos);
	void reBalanceShrubsNear(Vector3 pos, double range);
	bool reBalanceBuildingsNear(Vector3 pos, double range);
	bool balanceBuilding(int buildingIndex);
	bool balanceShrub(int shrubIndex);
	bool addAllToObstacleMap(); // new algorithm: get each corner; get grid cell at each corner, occupy all with a for loop
	bool isTerrainBetween(const Ogre::Vector3& start, const Ogre::Vector3& end, Ogre::Vector3& terrainPos);
	bool moveShrubTo(const int& index, const double& x, const double& y, const double& z);
	bool yawShrub(int index, double yaw);
	bool shrubSetVisible(int index, bool visible);
	
	WaterManager mWaterManager;
	ScenarioDetails mScenarioDetails;
	#ifndef NOPAGED
	PagedGeoWrapper mPagedGeo;
	#endif
	char mLastFileNameCreated[256];
private:
	Ogre::SceneManager* mSceneMgr; // pointer to the scene manager
	Ogre::Camera* pcam;
	
	Ogre::SceneNode* mSelectedNode; // currently selected node (via mouse click).
	Ogre::Vector3 mMinTerrainExtents; // size of terrain in meters
	Ogre::Vector3 mMaxTerrainExtents; // size of terrain in meters
	Ogre::Vector3 mPointer;
	
	ET::TerrainManager* mTerrainManager; // Terrain Manager from ET library
	ET::SplattingManager* mSplattingManager; // Splatting Manager from ET library
	ET::Brush mEditBrush; // 'brush' for editing terrain	
	
	Fog mFog;
	
	ShrubDef mShrubDefHash[SHRUB_DEF_MAX];
	BuildingDef	mBuildingDefHash[BUILDING_DEF_MAX];
	Shrub* mShrubHash[SHRUB_HASH_MAX];
	Building*	mBuildingHash[BUILDING_HASH_MAX];
	
	float mShrubCullDistance; // Distance from camera to cull trees from rendering
	float mBuildingCullDistance; // Distance from camera to cull trees from rendering
	float mShrubUnCullDistance; // Distance from camera to return trees to view
	float mBuildingUnCullDistance; // Distance from camera to return trees to view
	int	mMaxShrubsOnScreen;
	int	mShrubDefHashEnd;
	int	mCurrentPaintTexture;
	int mSelectedShrub;
	int mSelectedBuilding;
	int mShrubHashEnd;
	int mShrubEntityCount; // count of shrubs generated (for naming etc)
	int mCurrentShrubType; // type of shrub to plant next
	int mBuildingDefHashEnd;
	int mBuildingHashEnd;
	int mBuildingEntityCount; // count of Buildings generated (for naming etc)
	int mCurrentBuildingType; // type of Building to plant next
	int mMaxBuildingsOnScreen;
	bool mStaticAlreadyMapped;
	bool mPagedGeometryActivated;
};

#endif /*TERRAINHANDLER_H_*/

