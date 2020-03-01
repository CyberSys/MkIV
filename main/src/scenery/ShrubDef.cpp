#include "ShrubDef.h"
#include <string.h>
#include <fstream>
#include "../globals.h" 

using namespace std;

// init default values (in case something is missing from .def)
ShrubDef::ShrubDef() {
	// reset strings
	strcpy(mFilename,"");
	strcpy(mDesignation,"Unnamed Shrub");
	strcpy(mName,"Name");
	strcpy(mMeshFile,"");

	// set dimension defaults
	mShrubLength = 0.1;
	mAutoScale = false;
	mIsShrubLoaded = false;
	mShrubWidth = 0.1;
}

// load definition from a .def file
bool ShrubDef::loadFromFile(const char* filename) {
	// some basic validation
	if (!filename) {
		printf("Error: ShrubDef not given a real filename\n");
		return false;
	}
	if (strlen(filename) > 99) {
		printf("Error: ShrubDef filename too long\n");
		return false;
	}
	// load file
	ifstream file;

	char defPath[256];
	strcpy(defPath,"data/shrubs/specifications/");
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
			sscanf(next,"%f",&mShrubLength);
		} else if (strcmp(key,"AUTO_SCALE") == 0) {
			int as;
			sscanf(next,"%i",&as);
			if (as > 0) {
				mAutoScale = true;
			}
		} else if (strcmp(key,"WIDTH_M") == 0) {
			sscanf(next,"%f",&mShrubWidth);
		} else if (strcmp(key,"") == 0) {
			// skip blank lines
		} else {
			printf("WARNING: Unrecognised line in %s file: [%s]\n",filename,defDetails);
		}
	}
	mIsShrubLoaded = true;
	return true;
}

char* ShrubDef::getFilename() {
	return mFilename;
}

void ShrubDef::setFilename(const char* filename) {
	strcpy(mFilename,filename);
}

char* ShrubDef::getDesignation() {
	return mDesignation;
}

void ShrubDef::setDesignation(const char* desig) {
	strcpy(mDesignation,desig);
}

char* ShrubDef::getName() {
	return mName;
}

void ShrubDef::setName(const char* desig) {
	strcpy(mName,desig);
}

char* ShrubDef::getMeshFile() {
	return mMeshFile;
}

void ShrubDef::setMeshFile(const char* meshfile) {
	strcpy(mMeshFile,meshfile);
}

float ShrubDef::getShrubLength() {
	return mShrubLength;
}

void ShrubDef::setShrubLength(float length) {
	mShrubLength = length;
}

bool ShrubDef::isAutoScale() {
	return mAutoScale;
}

void ShrubDef::setAutoScale(bool autoscale) {
	mAutoScale = autoscale;
}

float ShrubDef::getShrubWidth() {
	return mShrubWidth;
}

void ShrubDef::setShrubWidth(float width) {
	mShrubWidth = width;
}

