#include "ScenarioDetails.h"

#include <stdio.h>
#include <string.h>
#include <fstream>
#include "../globals.h"

using namespace std;

Force::Force() {
	strcpy(mName, "unknown");
	strcpy(mShieldMaterial, "mkiv/EyeOfRa");
	mInitialCameraAim.x = mInitialCameraAim.y = mInitialCameraAim.z = 0.0;
	mInitialCameraPos.x = mInitialCameraPos.z = 0.0;
	mInitialCameraPos.y = 100.0;
}

// default constructor for scenario
ScenarioDetails::ScenarioDetails() {
	strcpy(mFilename, "Untitled.sce");
	strcpy(mTitle, "Untitled");
	strcpy(mMapName, "default");
	strcpy(mShrubListName, "default_shrubs");
	strcpy(mBuildingListName, "default_buildings");
	strcpy(mUnitListName, "defaultUnits.txt");
	strcpy(mMusicTrack, "nomusic.mp3");
	mWaterLevel = -10.0;
	mNumberOfForces = 2;
	mTimeLimitMins = 0;
	mWaterType = 0;
	mHasMusic = false;
}

bool ScenarioDetails::copyFrom(ScenarioDetails* otherSD) {
	if (!otherSD) {
		printf("ERROR: ptr passed to ScenarioDetails::copyFrom is NULL\n");
		return false;
	}
	strcpy(mFilename, otherSD->mFilename);
	strcpy(mTitle, otherSD->mTitle);
	strcpy(mMapName, otherSD->mMapName);
	strcpy(mShrubListName, otherSD->mShrubListName);
	strcpy(mBuildingListName, otherSD->mBuildingListName);
	strcpy(mUnitListName, otherSD->mUnitListName);
	strcpy(mMusicTrack, otherSD->mMusicTrack);
	mWaterLevel = otherSD->mWaterLevel;
	mNumberOfForces = otherSD->mNumberOfForces;
	mTimeLimitMins = otherSD->mTimeLimitMins;
	mHasMusic = otherSD->mHasMusic;
	mWaterType = otherSD->mWaterType;
	for (int i = 0; i < mNumberOfForces; i++) {
		strcpy(mForces[i].mName, otherSD->mForces[i].mName);
		strcpy(mForces[i].mShieldMaterial, otherSD->mForces[i].mShieldMaterial);
		mForces[i].mInitialCameraAim = otherSD->mForces[i].mInitialCameraAim;
		mForces[i].mInitialCameraPos = otherSD->mForces[i].mInitialCameraPos;
	}
	return true;
}

// load scenario from a given file
bool ScenarioDetails::loadFromFile(const char *name) {
	if (!name) { // some basic validation
		printf("Error: scenario loader not given a real filename [%s]\n", name);
		return false;
	}
	if (strlen(name) > 99) { // some basic validation
		printf("Error: scenario loader given a filename that is too long [%s]\n", name);
		return false;
	}
	ifstream file;
	char scenarioPath[256];
	strcpy(scenarioPath, "data/scenarios/");
	strcat(scenarioPath, name); // append path to file
	file.open(scenarioPath); // load file
	if (!file.is_open()) {
		printf("Error: Could not open .sce file %s!\n",name);
		return false;
	}
	strcpy(mFilename, name); // remember what file we are
	char sceDetails[256];
	char key[30];
	char next[256];
	while (file.getline(sceDetails, 256)) { // parse file
		strcpy(key,"");	// reset these strings. for some reason, even though the definitions were in here....
		strcpy(next,"");
		sscanf(sceDetails,"%s %s", key, next); // get the first part (indicator code) and rest (details) of each line
		if (key[0] == '#') { // ignore comments
		} else if (strcmp(sceDetails,"") == 0) { // ignore blank lines
		} else if (strcmp(key, "TITLE") == 0) {
			StripString(sceDetails, 6, mTitle, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "MAP_NAME") == 0) {
			StripString(sceDetails, 9, mMapName, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "SHRUB_LIST_NAME") == 0) {
			StripString(sceDetails, 16, mShrubListName, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "BUILDING_LIST_NAME") == 0) {
			StripString(sceDetails,19,mBuildingListName, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "UNIT_LIST_NAME") == 0) { 
			StripString(sceDetails, 15, mUnitListName, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "TIME_LIMIT_MINS") == 0) {
			sscanf(next, "%i", &mTimeLimitMins);
		} else if (strcmp(key, "WATER_TYPE") == 0) {
			if (strcmp(next, "NONE") == 0) {
				mWaterType = 0;
			} else if (strcmp(next, "PLANE") == 0) {
				mWaterType = 1;
			} else if (strcmp(next, "HYDRAX") == 0) {
				mWaterType = 2;
			}
		} else if (strcmp(key, "WATER_LEVEL") == 0) {
			sscanf(next, "%f", &mWaterLevel);
		} else if (strcmp(key, "NUMBER_OF_FORCES") == 0) {
			sscanf(next, "%i", &mNumberOfForces);
		} else if (strcmp(key, "FORCE_NAME") == 0) {
			int force = -1;
			char tmp[256];
			sscanf(sceDetails, "FORCE_NAME %i %s", &force, tmp);
			int charPos = 13; // where to chop the string
			if (force > 9) { // if the integer takes up two spaces then chop from string index 14
				charPos = 14;
			}
			StripString(sceDetails, charPos, mForces[force].mName, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key,"FORCE_SHIELD") == 0) {
			int force = -1;
			char tmp[256];
			sscanf(sceDetails, "FORCE_SHIELD %i %s", &force, tmp);
			strcpy(mForces[force].mShieldMaterial, tmp);
		} else if (strcmp(key, "CAM_POS_FORCE") == 0) { // camera position presets
			char tmp[256]; // sub-string buffer
			StripString(sceDetails, 14, tmp, 256); // grab all of string after key (not just next token)
			Vector3 tempPos; // vector to grab camera position
			tempPos.x = tempPos.y = tempPos.z = 0.0f; // clear to keep compiler happy
			int forceIndex = 69; // integer to grab force index
			sscanf(tmp, "%i {%f,%f,%f}", &forceIndex, &tempPos.x, &tempPos.y, &tempPos.z);
			if (forceIndex >= MAX_FORCES) { // validate range
				printf("ERROR: Scanning camera preset from scenario - force number (%i) is too high (max %i)\n", forceIndex, MAX_FORCES);
				printf(" original line segment was [%s]\n", tmp);
				return false;
			}
			 mForces[forceIndex].mInitialCameraPos = tempPos; // assign position by force
		} else if (strcmp(key, "CAM_AIM_FORCE") == 0) { // camera aiming presets
			char tmp[256]; // sub-string buffer
			StripString(sceDetails, 14, tmp, 256); // grab all of string after key (not just next token)
			Vector3 tempPos; // vector to grab camera position
			int forceIndex; // integer to grab force index
			sscanf(tmp, "%i {%f,%f,%f}", &forceIndex, &tempPos.x, &tempPos.y, &tempPos.z);
			if (forceIndex >= MAX_FORCES) { // validate range
				printf("ERROR: Scanning camera preset from scenario - force number is too high (max %i)\n", MAX_FORCES);
				return false;
			}
			mForces[forceIndex].mInitialCameraAim = tempPos; // assign position by force
		} else if (strcmp(key,"MUSIC_TRACK") == 0) {
			strcpy(mMusicTrack, next);
			mHasMusic = true;
		} else {
			printf("WARNING: Unrecognised line in %s file: %s\n",name,sceDetails);
		}
	}
	file.close(); // close file
	return true; // success
}

// save scenario to its file
bool ScenarioDetails::saveToFile() {
	// Open scenario file and setup data members
	FILE* fDataFile;
	fDataFile = fopen(mFilename, "w");
	if (!fDataFile) {
		printf("ERROR: Could not open scenario file %s for writing\n",mFilename);
		return false;
	}

	// write all details to file
	fprintf(fDataFile, "TITLE %s\n", mTitle);
	fprintf(fDataFile, "MAP_NAME %s\n", mMapName);
	fprintf(fDataFile, "SHRUB_LIST_NAME %s\n", mShrubListName);
	fprintf(fDataFile, "BUILDING_LIST_NAME %s\n", mBuildingListName);
	fprintf(fDataFile, "UNIT_LIST_NAME %s\n", mUnitListName);
	fprintf(fDataFile, "TIME_LIMIT_MINS %i\n" , mTimeLimitMins);
	fprintf(fDataFile, "NUMBER_OF_FORCES %i\n", mNumberOfForces);
	for (int i = 0; i < mNumberOfForces; i++) {
		fprintf(fDataFile, "FORCE_NAME %i %s\n", i, mForces[i].mName);
		fprintf(fDataFile, "FORCE_SHIELD %i %s\n", i, mForces[i].mShieldMaterial);
		fprintf(fDataFile, "CAM_POS_FORCE %i {%.2f,%.2f,%.2f}\n", i, mForces[i].mInitialCameraPos.x, mForces[i].mInitialCameraPos.y, mForces[i].mInitialCameraPos.z);
		fprintf(fDataFile, "CAM_AIM_FORCE %i {%.2f,%.2f,%.2f}\n", i, mForces[i].mInitialCameraAim.x, mForces[i].mInitialCameraAim.y, mForces[i].mInitialCameraAim.z);
	}
	if (mHasMusic) {
		fprintf(fDataFile, "MUSIC_TRACK %s\n", mMusicTrack);
	}
	if (mWaterType == 0) {
		fprintf(fDataFile, "WATER_TYPE NONE\n");
	} else if (mWaterType == 1) {
		fprintf(fDataFile, "WATER_TYPE PLANE\n");
	} else if (mWaterType == 2) {
		fprintf(fDataFile, "WATER_TYPE HYDRAX\n");
	}
	fprintf(fDataFile, "WATER_LEVEL %.2f\n", mWaterLevel);

	fclose(fDataFile); // close file
	return true; // success
}

