#ifndef UNITHANDLER_H_
#define UNITHANDLER_H_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "VehicleSpec.h"
#include "Vehicle.h"
#include "Projectile.h"
#include "../scenery/TerrainHandler.h"
#include "../gui/ThreeDeeLines.h"
#include "../ai/GAManagerModule.h"
#include <time.h>
using namespace std;
using namespace Ogre;

#define VEHICLE_DEF_MAX 128
#define VEHICLE_HASH_MAX 4112
#define PROJECTILE_HASH_MAX 99999 // TODO: fix this it doesn't reuse the old ones so it eventually crashes

class UnitHandler {
public:
	UnitHandler();
	bool init(SceneManager* sceneMgr, TerrainHandler* terrainMgr, RaySceneQuery* rsq); // initialise the unit handler
	bool free();

	bool update(unsigned long &elapsed);
	void editorSelectionUpdateLoop();

	int getUnitMatchingNode(Ogre::SceneNode* node);
	void setSelectedNode(SceneNode* node);
	void setTargetedNode(SceneNode* node);
	void deselectAll(bool editmode = false);

	void orderDestination(Vector3 &destination);
	void orderAttackVehicle(int vehicle);
	bool orderBoardVehicle(const int& riderIndex, const int& mountIndex);

	bool moveSelectedVehicleTo(const double& x, const double& y, const double& z);
	bool editorRotateSelectedVehicle(const double& amount);
	bool editorDeleteSelection();

	bool addAllToObstacleMap(); // add dynamic obstacles to map

	Unit* getVehicleAtIndex(int index); // get this vehicle or null

	bool createCharacterAtPosition(char* specFile, Ogre::Vector3 pos, double yaw, int force);
	bool saveUnitsToFile(const char* unitFile);
	bool loadUnitsFromFile(const char* unitFile); // load a bunch of units from an XML file

	int mSelectedVehicleIndex; // number of currently selected vehicles
private:
	bool updateClient();
	bool updateServer();

	bool updateVehicles(unsigned long &elapsed);
	bool updateSelectedVehicle(int index); // update guis and things for the vehicle given by index
	void updateSelection(int uhIndex, bool editorMode = false);

	int getSpecIndexOfFileLoadingIfNecessary(char* specFile);

	bool createTank(int defIndex, Vector3& pos, double yaw, int force);
	bool createProjectile(Vector3 pos, double muzzleVelocity, Vector3 gunOrientation);

	bool getNearestVehicleTo(int index, Vector3& obstacle, double& distanceSquared, double& sizeRadius, AxisAlignedBox& boundingBox); // get nearest obstacle to a given unit index
	//! Iterate through units to find nearest enemy
	int getNextNearestEnemy(const int& prevClosest, const int& ourUnit);

	bool resolveCombat(int attacker, int defender);
	bool calculateToHit(int firingVeh, int targetVeh); // pseduo-rand test to see if we will hit a target (t/f)
	bool destroyVehicle(int index, int killersIndex); // destroy a vehicle given by index
	bool canSeeTarget(int ourVeh, int targetVeh); // test if in direct line of sight of target (true/false)
	bool armourPenetrationTest(int firingVeh, int hitVeh); // test if projectile penetrates armour (true/false)
	bool ricochetTest(double effectivePen_mm, double effectiveThick_mm, double slope_rad, double glancing_rad);
	double interpolatePenetration_mm(double squaredDistance, int firingVehIndex);

	SceneManager* pSceneManager; // pointer to the scene manager
	TerrainHandler* pTerrainHandler; // pointer to the terrain manager
	RaySceneQuery* pRSQ;

	GAManagerModule mGAManager;
	ThreeDeeLines mLinesManager;

	SceneNode* mSelectedNode; // currently selected node (via mouse click).
	SceneNode* mTargetedNode; // currently targeted node (via RMB click)

	VehicleSpec mVehicleDefHash[VEHICLE_DEF_MAX]; // collection of 'definitions' of vehicles
	Unit* mUnitHash[VEHICLE_HASH_MAX]; // collection of active vehicles in memory
	Projectile* mpProjectileHash[PROJECTILE_HASH_MAX]; // collection of active projectiles in memory

	SceneNode* mNavTargetMarker; // node for positioning a marker for nav target (navmode)
	Entity* mpNavTargetMesh; // mesh representation of nav target (navmode)

	int mTankEntityCount; // count of vehicle entities (used for naming)
	int mProjectileEntityCount; // count of projectile entities (used for naming)
	int mVehicleDefHashEnd; // index at end of allocated vehicle definition hash
	int mUnitHashEnd; // index at end of allocated vehicle hash
	int mProjectileHashEnd; // index at end of allocated projectile hash

	int mOALineIndex; // red line for OA
	int mTSLineIndex; // yellow line for TS
};

#endif /*UNITHANDLER_H_*/
