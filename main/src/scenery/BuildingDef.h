#ifndef BUILDINGDEF_H_
#define BUILDINGDEF_H_

class BuildingDef {
public:
	BuildingDef();	// default constructor
	bool loadFromFile(const char* filename);
	//bool saveToFile(char* filename);

	// get and set methods
	char* getFilename();
	void setFilename(const char* filename);
	char* getDesignation();
	void setDesignation(const char* desig);
	char* getName();
	void setName(const char* desig);
	char* getMeshFile();
	void setMeshFile(const char* meshfile);

	float getBuildingLength();
	void setBuildingLength(float length);
	bool isAutoScale();
	void setAutoScale(bool autoscale = true);
	float getBuildingWidth();
	void setBuildingWidth(float width);
	char mFilename[100];		// the name of this file
	char mDesignation[20];	// name of the vehicle
	char mName[20];					// informal name
	char mMeshFile[100];	// relative path to mesh file
	float mBuildingLength;		// hull length in meters
	bool mAutoScale;		// whether or not to auto scale the mesh
	bool mIsBuildingLoaded;
	float	mBuildingWidth;		// hull width in meters
};

#endif
