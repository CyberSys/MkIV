#include "BuildingDef.h"
#include <string.h>
#include <fstream>
#include "../globals.h" 

using namespace std;

// init default values (in case something is missing from .def)
BuildingDef::BuildingDef() {
	// reset strings
	strcpy(mFilename,"");
	strcpy(mDesignation,"Unnamed Building");
	strcpy(mName,"Name");
	strcpy(mMeshFile,"");

	// set dimension defaults
	mBuildingLength = 0.1;
	mAutoScale = false;
	mIsBuildingLoaded = false;
	mBuildingWidth = 0.1;
}

// load definition from a .def file
bool BuildingDef::loadFromFile(const char* filename) {
	printf("loading %s def from file\n", filename);

	// some basic validation
	if (!filename) {
		printf("Error: BuildingDef not given a real filename\n");
		return false;
	}
	if (strlen(filename) > 99) {
		printf("Error: BuildingDef filename too long\n");
		return false;
	}
	// load file
	ifstream file;

	char defPath[256];
	strcpy(defPath,"data/buildings/specifications/");
	strcat(defPath,filename);

	// append path to file
	file.open(defPath);
	if (!file) {
		printf("Error: Could not open .def file %s!\n",filename);
		return false;
	}

	// remember what file we are
	strcpy(mFilename,filename);

	// parse file
	char defDetails[256];
	char key[30];
	char next[256];
	while (file.getline(defDetails,256)) {
		// get the first part (indicator code) and rest (details) of each line
		strcpy(key,"");	// reset these strings. for some reason, even though the definitions were in here....
		strcpy(next,"");
		sscanf(defDetails,"%s %s",key,next);

		if (key[0] == '#') {
			// ignore comments
		} else if (strcmp(key,"TYPE") == 0) {

		} else if (strcmp(key,"DESIGNATION") == 0) {
			// grab all of string after key (not just next token)
			StripString(defDetails,12,mDesignation,20);
		} else if (strcmp(key,"NAME") == 0) {
			// grab all of string after key (not just next token)
			StripString(defDetails,5,mName,20);
		} else if (strcmp(key,"MESH_FILE") == 0) {
			strcpy(mMeshFile,next);
		} else if (strcmp(key,"LENGTH_M") == 0) {
			sscanf(next,"%f",&mBuildingLength);
		} else if (strcmp(key,"AUTO_SCALE") == 0) {
			int as;
			sscanf(next,"%i",&as);
			if (as > 0) {
				mAutoScale = true;
			}
		} else if (strcmp(key,"WIDTH_M") == 0) {
			sscanf(next,"%f",&mBuildingWidth);
		} else if (strcmp(key,"") == 0) {
			// skip blank lines
		} else {
			printf("WARNING: Unrecognised line in %s file: [%s]\n",filename,defDetails);
		}
		
	}
	mIsBuildingLoaded = true;
	return true;
}

char* BuildingDef::getFilename() {
	return mFilename;
}

void BuildingDef::setFilename(const char* filename) {
	strcpy(mFilename,filename);
}

char* BuildingDef::getDesignation() {
	return mDesignation;
}

void BuildingDef::setDesignation(const char* desig) {
	strcpy(mDesignation,desig);
}

char* BuildingDef::getName() {
	return mName;
}

void BuildingDef::setName(const char* desig) {
	strcpy(mName,desig);
}

char* BuildingDef::getMeshFile() {
	return mMeshFile;
}

void BuildingDef::setMeshFile(const char* meshfile) {
	strcpy(mMeshFile,meshfile);
}

float BuildingDef::getBuildingLength() {
	return mBuildingLength;
}

void BuildingDef::setBuildingLength(float length) {
	mBuildingLength = length;
}

bool BuildingDef::isAutoScale() {
	return mAutoScale;
}

void BuildingDef::setAutoScale(bool autoscale) {
	mAutoScale = autoscale;
}

float BuildingDef::getBuildingWidth() {
	return mBuildingWidth;
}

void BuildingDef::setBuildingWidth(float width) {
	mBuildingWidth = width;
}

