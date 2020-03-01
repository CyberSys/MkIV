#ifndef SCENARIODETAILS_H_
#define SCENARIODETAILS_H_ 

#include "Ogre.h"
using namespace Ogre;

#define MAX_FORCES 6 // maximum number of forces in simulation

struct ScenarioDetails;

struct Force {
	Force();
	
	Vector3 mInitialCameraAim; // 3D preset position to point camera at (for each force)
	Vector3 mInitialCameraPos; // 3D preset position of camera (for each force)
	char mName[128];
	char mShieldMaterial[128];	
};

// This class holds basic details of a scenario
struct ScenarioDetails {
	ScenarioDetails();	// set default values
	bool copyFrom(ScenarioDetails* otherSD);
	bool loadFromFile(const char *name);	// load scenario from a given file
	bool saveToFile();	// save scenario to its file
	
	Force mForces[MAX_FORCES];
	char mFilename[128]; // filename to use
	char mTitle[128];	// title of scenario
	char mMapName[128];	// map name (prefix of all map and terrain files name)
	char mShrubListName[128];
	char mBuildingListName[128];
	char mUnitListName[128];
	char mMusicTrack[128];
	float mWaterLevel;
	int mNumberOfForces; // number of involved factions	
	int mTimeLimitMins;	// time limit in minutes (0=no limit)
	int mWaterType;
	bool mHasMusic;
};

#endif

