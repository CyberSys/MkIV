#ifndef SHRUBDEF_H_
#define SHRUBDEF_H_

class ShrubDef {
public:
	ShrubDef();	// default constructor
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

	float getShrubLength();
	void setShrubLength(float length);
	bool isAutoScale();
	void setAutoScale(bool autoscale = true);
	float getShrubWidth();
	void setShrubWidth(float width);
	char mFilename[128];		// the name of this file
	char mDesignation[128];	// name of the vehicle
	char mName[128];					// informal name
	char mMeshFile[128];	// relative path to mesh file
	float mShrubLength;		// hull length in meters
	bool mAutoScale;		// whether or not to auto scale the mesh
	float	mShrubWidth;		// hull width in meters
	bool mIsShrubLoaded;
};

#endif
