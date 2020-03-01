#include "Shrub.h"
#include <string.h>
#include "../globals.h"
#include <math.h>
using namespace std;

Shrub::Shrub() {
	mIsPagedGeometry = false;
}

// create the shrub from a definition file, and attached to a given node
bool Shrub::init(ShrubDef* defptr, Entity* mesh, Vector3& pos, float boundRad) {
	// init from parameters
	mMesh = mesh;
	mDefPtr = defptr;
	mPosition[0] = pos.x;
	mPosition[1] = pos.y;
	mPosition[2] = pos.z;

	// validate params
	if (!mesh) {
		printf("ERROR: supplied mesh is empty\n");
		return false;
	}
	if (!defptr) {
		printf("ERROR: supplied DEF file is empty\n");
		return false;
	}

	mSelected						= false;
	mScaleFactor				= 1.0f;
	mYaw								= 0.0f;

	// scale the shrub to correct size in meters
	if (mDefPtr->isAutoScale()) {
		float length = mDefPtr->getShrubLength();
		float width = mDefPtr->getShrubWidth();
		// get longest edge (for the odd cases where vehicle is wider than it is long)
		float longest = length;
		if (width > length) {
			longest = width;
		}
		// get factor to scale by
		mScaleFactor = 1 / (boundRad / longest);
	}
	mBoundRad = boundRad * mScaleFactor;
	
	return true;
}

// get the index of the definition file for this vehicle
ShrubDef* Shrub::getDefPtr() {
	return mDefPtr;
}

// set the index of the definition file for this vehicle
void Shrub::setDefPtr(ShrubDef* defptr) {
	mDefPtr = defptr;
}

// Returns minimum height of building
AxisAlignedBox Shrub::getBoundingBox() {
	return mMesh->getWorldBoundingBox(); // get the bounding box of the building
}

// returns shrubs x,y,z pos in meters
Vector3 Shrub::getPosition() {
	Ogre::Vector3 pos;
	pos.x = mPosition[0];
	pos.y = mPosition[1];
	pos.z = mPosition[2];
	return pos;
}

void Shrub::setPosition(const double& x, const double& y, const double& z) {
	mPosition[0] = x;
	mPosition[1] = y;
	mPosition[2] = z;
}

// changes the absolute y-axis rotation of the object
void Shrub::setYaw(float yRot) {
	// adjust so is between 0 and 2Pi
	while (yRot < 0.0f) {
		yRot += 2.0f * M_PI;
	}
	while (yRot >= 2.0f * M_PI) {
		yRot -= 2.0f * M_PI;
	}
	// set internal copy
	mYaw = yRot;
}

// returns the absolute y-axis rotation of the object
float	Shrub::getYaw() {
	return mYaw;
}

// check if vehicle is currently 'selected' by user
bool Shrub::isSelected() {
	return mSelected;
}

// set the vehicle as currently 'selected' by user and show graphical indicator
void Shrub::setSelected(bool selected) {
	mSelected = selected;
}

// get shrubs radius in meters (not in squared meters)
double Shrub::getRadiusMeters() {
	return mBoundRad;
}

