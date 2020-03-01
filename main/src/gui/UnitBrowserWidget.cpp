#include "UnitBrowserWidget.h"

UnitBrowserWidget::UnitBrowserWidget() {
	mpSelectedUnitDetails = NULL;
}

bool UnitBrowserWidget::initDetails() {
	mpSelectedUnitDetails = new VehicleSpec;
	if (!mpSelectedUnitDetails) {
		printf("ERROR: UnitBrowserWidget::initDetails() could not alloc VehicleSpec\n");
		return false;
	}
	
	// add special buttons and thingies that aren't in the generic meshbrowserwidget
	if (!pContainer) {
		printf("ERROR: UnitBrowserWidget::initDetails() pContainer was NULL\n");
		return false;
	}
	char tmp[256];
	sprintf(tmp, "%s_descrLabel", mUniqueName);
	if (!mDescriptionLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.55f, 0.1f, "")) {
		printf("ERROR: UnitBrowserWidget::init Could not init mDescriptionLabel label\n");
		return false;
	}
	mDescriptionLabel.setSize(15);
	mDescriptionLabel.setColour(1.0, 1.0, 1.0, 1.0);
	
	sprintf(tmp, "%s_perfL", mUniqueName);
	if (!mTopSpeedCrossLabel.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.01f, 0.7f, "")) {
		printf("ERROR: UnitBrowserWidget::init Could not init mTopSpeedCrossLabel label\n");
		return false;
	}
	mTopSpeedCrossLabel.setSize(15);
	mTopSpeedCrossLabel.setColour(1.0, 1.0, 0.7, 1.0);
	
	sprintf(tmp, "%s_armourL", mUniqueName);
	mArmourDetails.init(pContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.25f, 0.7f, "");
	mArmourDetails.setSize(15);
	mArmourDetails.setColour(1.0, 1.0, 0.7, 1.0);
	
	return true;
}

bool UnitBrowserWidget::freeDetails() {
	if (!mpSelectedUnitDetails) {
		printf("ERROR UnitBrowserWidget mpSelectedUnitDetails not alloced so can't free\n");
		return false;
	}
	delete mpSelectedUnitDetails;
	mpSelectedUnitDetails = NULL;
	mArmourDetails.free();
	if (!mTopSpeedCrossLabel.free()) {
		printf("ERROR: UnitBrowser could not free mTopSpeedCrossLabel\n");
		return false;
	}
	if (!mDescriptionLabel.free()) {
		printf("ERROR: UnitBrowser could not free mDescriptionLabel\n");
		return false;
	}
	return true;
}

bool UnitBrowserWidget::displaySelectedDetails() {
	if (!mpSelectedUnitDetails) {
		printf("ERROR: UnitBrowserWidget::displaySelectedDetails() called w/o being initialised properly\n");
		return false;
	}
	int scannerIndexOfEntry = mSelectedEntryOnPage * 15 + mSelectedEntryIndex;
	strcpy(mSelectedEntry, pFileScanner->getFileName(scannerIndexOfEntry));
	strcat(mSelectedEntry, mPostfix); // update selected entry name (internal record)
	mpSelectedUnitDetails->resetSpecs();
	mpSelectedUnitDetails->loadFromXMLfile(mSelectedEntry);
	if (!mDescriptionLabel.setText(mpSelectedUnitDetails->mDescription)) { // updateLabels() with selected item's details
		printf("ERROR: UnitBrowserWidget::displaySelectedDetails() could not set mDescriptionLabel text\n");
		return false;
	}
	char tmp[1024];
	strcpy(tmp, "blank");
	#define RADCONV 57.2957795
	sprintf(tmp, "Performance:\nTop road speed: %.1f k/h\nTop offroad speed %.1f k/h\nTop swim speed %.1f k/h", mpSelectedUnitDetails->mPerformance.mTopRoadKPH, mpSelectedUnitDetails->mPerformance.mTopCrossKPH, mpSelectedUnitDetails->mPerformance.mTopWaterKPH);
	mTopSpeedCrossLabel.setText(tmp);
	
	sprintf(tmp, "Hull armour:\nFront %.0f mm @ %.0f deg.\nSides %.0f mm @ %.0f deg.\nRear %.0f mm @ %.0f deg.\nTop %.0f mm @ %.0f deg.", mpSelectedUnitDetails->mHull.mArmour.mFront_mm, mpSelectedUnitDetails->mHull.mArmour.mFrontSlope_rad * RADCONV, mpSelectedUnitDetails->mHull.mArmour.mSide_mm, mpSelectedUnitDetails->mHull.mArmour.mSideSlope_rad * RADCONV, mpSelectedUnitDetails->mHull.mArmour.mRear_mm, mpSelectedUnitDetails->mHull.mArmour.mRearSlope_rad * RADCONV, mpSelectedUnitDetails->mHull.mArmour.mTop_mm, mpSelectedUnitDetails->mHull.mArmour.mTopSlope_rad * RADCONV);
	if (mpSelectedUnitDetails->mHasMainTurret) {
		char temp2[1024];
		sprintf(temp2, "\n\nMain Turret Armour:\nFront %.0f mm @ %.0f deg.\nSides %.0f mm @ %.0f deg.\nRear %.0f mm @ %.0f deg.\nTop %.0f mm @ %.0f deg.", mpSelectedUnitDetails->mTurret.mArmour.mFront_mm, mpSelectedUnitDetails->mTurret.mArmour.mFrontSlope_rad * RADCONV, mpSelectedUnitDetails->mTurret.mArmour.mSide_mm, mpSelectedUnitDetails->mTurret.mArmour.mSideSlope_rad * RADCONV, mpSelectedUnitDetails->mTurret.mArmour.mRear_mm, mpSelectedUnitDetails->mTurret.mArmour.mRearSlope_rad * RADCONV, mpSelectedUnitDetails->mTurret.mArmour.mTop_mm, mpSelectedUnitDetails->mTurret.mArmour.mTopSlope_rad * RADCONV);
		strcat(tmp, temp2);
	}
	mArmourDetails.setText(tmp);
	
	if (!mDesignationLabel.setText(mpSelectedUnitDetails->mDesignation)) { // updateLabels() with selected item's details
		printf("ERROR: UnitBrowserWidget::displaySelectedDetails() could not set mDesignationLabel text\n");
		return false;
	}
	if (strlen(mpSelectedUnitDetails->mMeshFile) > 3) { // load the mesh
		char nameTemp[128];
		if (mGUImeshAllocated) {
			mpPreviewMeshNode->detachAllObjects();
			sprintf(nameTemp, "%sEntity%i", mUniqueName, mEntityCount);
			pSceneManager->destroyEntity(nameTemp);
			pGUImesh = NULL;
		}
		sprintf(nameTemp, "%sEntity%i", mUniqueName, mEntityCount++);
		pGUImesh = pSceneManager->createEntity(nameTemp, mpSelectedUnitDetails->mMeshFile); // create the entity (blender mesh)
		if (!pGUImesh) {
			printf("ERROR: UnitBrowserWidget::displaySelectedDetails() could not create gui entity from mesh file\n");
			return false;
		}
		mGUImeshAllocated = true;
		if (mpSelectedUnitDetails->mAutoScale) { // scale the vehicle to correct size
			float length = mpSelectedUnitDetails->mHull.length_m;
			float width = mpSelectedUnitDetails->mHull.width_m;
			float longest = length;
			if (width > length) { // get longest edge (for the odd cases where vehicle is wider than it is long)
				longest = width;
			}
			float scaleFactor = 1.0 / (pGUImesh->getBoundingRadius() / longest); // get factor to scale by
			scaleFactor = scaleFactor * 0.05f;
			mpPreviewMeshNode->setScale(scaleFactor, scaleFactor, scaleFactor); // scale node and corresponding bits (equally on all axiis)
		}
	} else {
		printf("ERROR: UnitBrowserWidget::displaySelectedDetails() mesh file name was less than 3 chacacters long\n");
		return false;
	}
	return true;
}


