#include "BuildingBrowserWidget.h"

BuildingBrowserWidget::BuildingBrowserWidget() {
	mpSelectedUnitDetails = NULL;
}

bool BuildingBrowserWidget::initDetails() {
	mpSelectedUnitDetails = new BuildingDef;
	if (!mpSelectedUnitDetails) {
		printf("ERROR: BuildingBrowserWidget::initDetails() could not alloc BuildingDef\n");
		return false;
	}
	return true;
}

bool BuildingBrowserWidget::freeDetails() {
	if (!mpSelectedUnitDetails) {
		printf("ERROR: BuildingBrowserWidget cannot free details as specptr not allocated yet\n");
		return false;
	}
	delete mpSelectedUnitDetails;
	mpSelectedUnitDetails = NULL;
	return true;
}

bool BuildingBrowserWidget::displaySelectedDetails() {
	if (!mpSelectedUnitDetails) {
		printf("ERROR: BuildingBrowserWidget::displaySelectedDetails() called w/o being initialised properly\n");
		return false;
	}
	int scannerIndexOfEntry = mSelectedEntryOnPage * 15 + mSelectedEntryIndex;
	strcpy(mSelectedEntry, pFileScanner->getFileName(scannerIndexOfEntry));
	strcat(mSelectedEntry, mPostfix); // update selected entry name (internal record)
	printf("BuildingBrowserWidget: loading %s\n", mSelectedEntry);
	mpSelectedUnitDetails->loadFromFile(mSelectedEntry);
	printf("BuildingBrowserWidget: design %s\n", mpSelectedUnitDetails->mDesignation);
	if (!mDesignationLabel.setText(mpSelectedUnitDetails->mDesignation)) { // updateLabels() with selected item's details
		printf("ERROR: BuildingBrowserWidget::displaySelectedDetails() could not set label text\n");
		return false;
	}
	if (strlen(mpSelectedUnitDetails->mMeshFile) > 3) { // load the mesh
		char tmp[128];
		if (mGUImeshAllocated) {
			printf("UB: removing prev objects\n");
			mpPreviewMeshNode->detachAllObjects();
			sprintf(tmp, "%sEntity%i", mUniqueName, mEntityCount);
			pSceneManager->destroyEntity(tmp);
			pGUImesh = NULL;
		}
		sprintf(tmp, "%sEntity%i", mUniqueName, mEntityCount++);
		printf("UB: creating gui ent from mesh file[%s]\n", mpSelectedUnitDetails->mMeshFile);
		pGUImesh = pSceneManager->createEntity(tmp, mpSelectedUnitDetails->mMeshFile); // create the entity (blender mesh)
		if (!pGUImesh) {
			printf("ERROR: BuildingBrowserWidget::displaySelectedDetails() could not create gui entity from mesh file\n");
			return false;
		}
		mGUImeshAllocated = true;
		if (mpSelectedUnitDetails->mAutoScale) { // scale the Building to correct size
			float length = mpSelectedUnitDetails->mBuildingLength;
			float width = mpSelectedUnitDetails->mBuildingWidth;
			float longest = length;
			if (width > length) { // get longest edge (for the odd cases where vehicle is wider than it is long)
				longest = width;
			}
			float scaleFactor = 1.0f / (pGUImesh->getBoundingRadius() / longest); // get factor to scale by
			scaleFactor = scaleFactor * 0.025f;
			mpPreviewMeshNode->setScale(scaleFactor, scaleFactor, scaleFactor); // scale node and corresponding bits (equally on all axiis)
		}
	} else {
		printf("ERROR: BuildingBrowserWidget::displaySelectedDetails() mesh file name was less than 3 chacacters long\n");
		return false;
	}
	return true;
}


