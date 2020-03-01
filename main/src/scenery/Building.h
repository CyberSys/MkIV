#ifndef BUILDING_H_
#define BUILDING_H_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "BuildingDef.h"

using namespace Ogre;

class Building {
public:
	bool init(BuildingDef* defptr, Entity* mesh, SceneNode*	node, Vector3& pos, float boundRad);
//	bool update(unsigned long &elapsed);	// any updates (move in breeze etc)

	void showSelected(bool show); // show selected box around building
	bool isSelected();
	void setSelected(bool selected);
	void setVisible(bool visible);
	// definition file
	BuildingDef* getDefPtr();
	void setDefPtr(BuildingDef* defptr);
	SceneNode* getNode();

	Vector3 getPosition();
	void setPosition(Vector3& position);
	void setYaw(float yRot);
	float getYaw();	// ++	void performRotation()
	AxisAlignedBox getBoundingBox();
	float getRadiusMeters(); // get the radius of the building's bbox - note this is not hugely accurate for rectangular buildings
	Entity* mMesh;					// main mesh
	SceneNode* mNode;				// node the model is attached to
	float	mScaleFactor;		// factor to scale the mesh by
private:
	BuildingDef* mDefPtr;	// vehicle's definition file
	float	mYaw;
	bool mSelected;
};

#endif
