#ifndef SHRUB_H_
#define SHRUB_H_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "ShrubDef.h"

using namespace Ogre;

struct Shrub {
	Shrub();
	bool init(ShrubDef* defptr, Entity* mesh, Vector3& pos, float boundRad);
	ShrubDef* getDefPtr(); // definition file
	AxisAlignedBox getBoundingBox();
	Vector3 getPosition();
	float getYaw();	// ++	void performRotation()
	double getRadiusMeters(); // get shrubs radius in meters (not in squared meters)
	bool isSelected();
	void setSelected(bool selected);
	void showSelected(bool show); // show selected box around shrub
	void setDefPtr(ShrubDef* defptr);
	void setPosition(const double& x, const double& y, const double& z);
	void setYaw(float yRot);
	
	Entity* mMesh; // main mesh
	char mNodeName[128];
	double mPosition[3];
	float	mScaleFactor; // factor to scale the mesh by
	double mBoundRad;
	ShrubDef* mDefPtr; // vehicle's definition file
	float	mYaw;
	bool mSelected;
	bool mIsPagedGeometry;
};

#endif
