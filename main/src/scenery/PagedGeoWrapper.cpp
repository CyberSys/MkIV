#include "PagedGeoWrapper.h"
#include "pagedgeometry/BatchPage.h"
#include "pagedgeometry/ImpostorPage.h"
using namespace Forests;
using namespace Ogre;

PagedGeoWrapper::PagedGeoWrapper() {
	mTrees = NULL;
	mTreeLoader = NULL;
	mInitialised = false;
}

bool PagedGeoWrapper::init(Camera* cam) {
	mTrees = new PagedGeometry();
	if (!mTrees) {
		printf("ERROR: could not alloc PagedGeometry in PagedGeoWrapper::init\n");
		return false;
	}
	mTrees->setCamera(cam); // Set the camera so PagedGeometry knows how to calculate LODs
	mTrees->setPageSize(50); // Set the size of each page of geometry
	mTrees->setInfinite(); // Use infinite paging mode
	mTrees->addDetailLevel<BatchPage>(500, 100); // Use batches up to 150 units away, and fade for 30 more units
	//mTrees->addDetailLevel<ImpostorPage>(1000, 100); // Use impostors up to 400 units, and for for 50 more units
	
	mTreeLoader = new TreeLoader3D(mTrees, TBounds(-2000.0, -2000.0, 2000.0, 2000.0)); // Create a new TreeLoader3D object first
	if (!mTreeLoader) {
		printf("ERROR: could not alloc mTreeLoader in PagedGeoWrapper::init\n");
		return false;
	}
	mTrees->setPageLoader(mTreeLoader); // Assign the "treeLoader" to be used to load geometry for the PagedGeometry instance
	
	mInitialised = true;
	return true;
}

bool PagedGeoWrapper::free() {
	if (!mInitialised) {
		printf("ERROR: failed to PagedGeoWrapper::free() as not init yet!\n");
		return false;
	}
	
	delete mTreeLoader;
	mTreeLoader = NULL;
	
	delete mTrees;
	mTrees = NULL;
	
	// TODO destroy each entity by name
	
	return true;
}

bool PagedGeoWrapper::doneAddingTrees() {
	if (!mInitialised) {
		printf("ERROR: failed to PagedGeoWrapper::doneAddingTrees() as not init yet!\n");
		return false;
	}
	
	if (!mTreeLoader) {
		return false;
	}
	//mTrees->setPageLoader(mTreeLoader);
	return true;
}

// update paging (call every frame)
bool PagedGeoWrapper::update() {
	if (!mInitialised) {
		printf("ERROR: failed to PagedGeoWrapper::update() as not init yet!\n");
		return false;
	}
  mTrees->update();
	
	return true;
}

