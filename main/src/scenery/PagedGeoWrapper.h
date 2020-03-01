#ifndef _MKIV_PAGEDGEOWRAPPER_H_
#define _MKIV_PAGEDGEOWRAPPER_H_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "pagedgeometry/PagedGeometry.h"
#include "pagedgeometry/TreeLoader3D.h"
using namespace Ogre;
using namespace Forests;

struct PagedGeoWrapper {
	PagedGeoWrapper();
	bool init(Ogre::Camera* cam);
	bool free();
	bool doneAddingTrees();
	bool update(); // update paging (call every frame)
	
	PagedGeometry* mTrees;
	TreeLoader3D* mTreeLoader;
	bool mInitialised;
};

#endif

// to add a tree: mTreeLoader->addTree(myEntity, position, yaw, scale);
// to finish adding trees: 

