#include "Building.h"
#include <string.h> 
#include "../globals.h"
#include "math.h"
using namespace std;

// create the Building from a definition file, and attached to a given node
bool Building::init(BuildingDef* defptr, Entity* mesh, SceneNode*	node, Vector3& pos, float boundRad) {
	// init from parameters
	mMesh = mesh;
	mDefPtr = defptr;
	mNode = node;

	// validate params
	if (!mesh) {
		printf("ERROR: supplied mesh is empty\n");
		return false;
	}
	if (!node) {
		printf("ERROR: supplied node is empty\n");
		return false;
	}
	if (!defptr) {
		printf("ERROR: supplied DEF file is empty\n");
		return false;
	}

	mSelected						= false;
	mScaleFactor				= 1.0f;
	mYaw								= 0.0f;

	node->setPosition(pos); // put in a position in the world

	// scale the vehicle to correct size in meters
	if (mDefPtr->isAutoScale()) {
		float length = mDefPtr->getBuildingLength();
		float width = mDefPtr->getBuildingWidth();
		// get longest edge (for the odd cases where vehicle is wider than it is long)
		float longest = length;
		if (width > length) {
			longest = width;
		}
		// get factor to scale by
		mScaleFactor = 1 / (boundRad / longest);
		// scale node and corresponding bits (equally on all axiis)
		node->setScale(mScaleFactor, mScaleFactor, mScaleFactor);
	}

	return true;	
}

// get the index of the definition file for this vehicle
BuildingDef* Building::getDefPtr() {
	return mDefPtr;
}

// set the index of the definition file for this vehicle
void Building::setDefPtr(BuildingDef* defptr) {
	mDefPtr = defptr;
}

// return ptr to main node
SceneNode* Building::getNode() {
	return mNode;
}

// returns Builing's x,y,z pos in meters
Vector3 Building::getPosition() {
	return mNode->getPosition();
}

// change the world x,y,z position of the object
void Building::setPosition(Vector3& pos) {
	// set X,Y,Z position of building (in metres)
	mNode->setPosition(pos);
}

// changes the absolute y-axis rotation of the object
void Building::setYaw(float yRot) {
	// adjust so is between 0 and 2Pi
	while (yRot < 0.0f) {
		yRot += 2.0f * M_PI;
	}
	while (yRot >= 2.0f * M_PI) {
		yRot -= 2.0f * M_PI;
	}
	// set internal copy
	mYaw = yRot;

	// perform orientation
	mNode->resetOrientation();
	mNode->yaw(Radian(mYaw));
}

// returns the absolute y-axis rotation of the object
float	Building::getYaw() {
	return mYaw;
}

// Returns minimum height of building 
AxisAlignedBox Building::getBoundingBox() {
	return mMesh->getWorldBoundingBox(); // get the bounding box of the building
}

// check if vehicle is currently 'selected' by user
bool Building::isSelected() {
	return mSelected;
}

// set the vehicle as currently 'selected' by user and show graphical indicator
void Building::setSelected(bool selected) {
	mSelected = selected;
	mNode->showBoundingBox(selected);
}

// hide or return the building to view
void Building::setVisible(bool visible) {
	mNode->setVisible(visible);
}

// get the radius of the building's bbox - note this is not hugely accurate for rectangular buildings
float Building::getRadiusMeters() {
	if (!mMesh) {
		printf("FATAL ERROR: getting radius - building mesh is empty\n");
		exit(1);
	}
	return mMesh->getBoundingRadius();
}

