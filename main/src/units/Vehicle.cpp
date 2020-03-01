#include "Vehicle.h"
#include <string.h>
#include <math.h>
using namespace std;

Vector3 Vehicle::getWaypoint(int index) {
	Vector3 waypoint;
	if ((index < 0) || (index >= mWaypointCount)) {
		printf("ERROR: waypoint is not valid requested to Unit::getWaypoint()\n");
		return waypoint;
	}
	#ifdef _GO_GRIDS_
	g.mBrainGrid.calcRealPos(mXWaypoint[index], mYWaypoint[index], waypoint.x, waypoint.z);
	#endif
	return waypoint;
}

bool Vehicle::bobUpAndDown() {
	if (mImmobilised) {
		return true;
	}
	Vector3 pos;
	getPosition(pos.x, pos.y, pos.z); // 2. get X,Y,Z position of vehicle (in metres)
	pos.y = pTerrainHandler->mWaterManager.getCurrentWaterHeight();
	//printf("Vehicle::bobUpAndDown() setposition()\n");
	setPosition(pos.x, pos.y, pos.z);
	return true;
}

// balance the vehicle on the terrain
bool Vehicle::balance() {
	if (!mInitialised) {
		printf("ERROR: vehicle not initialised so can not balance\n");
		return false;
	}
	Vector3 pos;
	getPosition(pos.x, pos.y, pos.z); // 2. get X,Y,Z position of vehicle (in metres)
	double halfWheelBase = (pSpecPtr->mHull.length_m / 2.0f);
	//printf("DEBUG: halfWheelBase(%lf) = pSpecPtr->mHull.length_m(%lf) / 2.0f\n", halfWheelBase, pSpecPtr->mHull.length_m);
	// 3. work out where the point 2m ahead of the vehicle's centre is in absolute 3d space
	double frontX = sinf(mHeading) * (halfWheelBase);
	//printf("DEBUG: frontX(%lf) = sinf(mHeading(%lf))(%lf) * (halfWheelBase)(%lf)\n", frontX, mHeading, sinf(mHeading), halfWheelBase);
	if (isnan(frontX)) {
		printf("FATAL ERROR: unit[%i] has NAN frontX in Vehicle::balance()\n", mUniqueIndexId);
		exit(1);
	}
	double frontZ = -cosf(mHeading) * (halfWheelBase);
	if (isnan(frontZ)) {
		printf("FATAL ERROR: unit[%i] has NAN frontZ in Vehicle::balance()\n", mUniqueIndexId);
		exit(1);
	}
	double frontHeight = pTerrainHandler->getTerrainHeightAt(pos.x + frontX, pos.z - frontZ);
	//printf("DEBUG: frontHeight(%lf) = pos.x(%lf) + frontX(%lf), pos.z(%lf) - frontZ(%lf)\n", frontHeight, pos.x, frontX, pos.z, frontZ);
	if (isnan(frontHeight)) {
		printf("FATAL ERROR: unit[%i] has NAN frontHeight in Vehicle::balance()\n", mUniqueIndexId);
		exit(1);
	}

	if (pSpecPtr->mIsLandingCraft) { // specific over-water over-terrain movement for landing craft
		if (frontHeight >= pos.y) {
			mLandingCraft.mIsBeached = true;
			mImmobilised = true;
		}
		if (!mLandingCraft.mIsBeached) {
			bobUpAndDown();
			performRotation(); // use our wrapper functions to set pitch and roll.
		}
		return true;
	}
	
	// get more terrain heights around vehicle
	double halfAxelWidth = (pSpecPtr->mHull.width_m / 2.0f);
	double leftX = cosf(mHeading) * halfAxelWidth;
	double leftZ = sinf(mHeading) * halfAxelWidth;
	double rightX = -cosf(mHeading) * halfAxelWidth;
	double rightZ = -sinf(mHeading) * halfAxelWidth;
	double rearHeight = pTerrainHandler->getTerrainHeightAt(pos.x - frontX, pos.z + frontZ);
	double leftHeight = pTerrainHandler->getTerrainHeightAt(pos.x + leftX, pos.z - leftZ);
	double rightHeight = pTerrainHandler->getTerrainHeightAt(pos.x + rightX, pos.z - rightZ);
	
	// balance amphibians on terrain AND water
	double waterHeight = pTerrainHandler->mWaterManager.getCurrentWaterHeight();
	if (pSpecPtr->mIsAmphibian) { // specific over-water over-terrain movement
		mAmphibian.mIsSwimming = false;
		if (frontHeight < waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM) {
			frontHeight = waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM;
			mAmphibian.mIsSwimming = true;
			//printf("DB: WATERHEIGHT = %f, WATERLINEOS = %lf, FH = %f\n", waterHeight, pSpecPtr->mPerformance.mWaterLineOffsetM, frontHeight);
		}
		if (rearHeight < waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM) {
			rearHeight = waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM;
			mAmphibian.mIsSwimming = true;
		}
		if (leftHeight < waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM) {
			leftHeight = waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM;
			mAmphibian.mIsSwimming = true;
		}
		if (rightHeight < waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM) {
			rightHeight = waterHeight - pSpecPtr->mPerformance.mWaterLineOffsetM;
			mAmphibian.mIsSwimming = true;
		}
	}	
	
	// 5. get difference to work out pitch
	double pitch = atan2f((rearHeight - frontHeight), pSpecPtr->mHull.length_m);	// where 4 = adjacent side 2m+2m
	
	// 6. work out roll
	double roll = atan2f((leftHeight - rightHeight), pSpecPtr->mHull.width_m); // where 2 = adjacent side (1m+1m)
	
	mRoll = roll; // use our wrapper functions to set pitch and roll.
	mPitch = pitch; // use our wrapper functions to set pitch and roll.
	performRotation(); // use our wrapper functions to set pitch and roll.
	double FRDiff = (frontHeight - rearHeight) / 2; // average the heights
	double avgFR = rearHeight + FRDiff;
	double LRDiff = (leftHeight - rightHeight) / 2;
	double avgLR = rightHeight + LRDiff;
	double totalDiff = (avgFR - avgLR) / 2;
	//printf("DEBUG: totalAvg(%lf) = avgFR(%lf) - avgLR(%lf)}\n", totalDiff, avgFR, avgLR);
	double totalAvg = avgLR + totalDiff;
	//printf("DEBUG: Vehicle::balance() setposition() with {%lf, %lf, %lf}\n", pos.x, totalAvg, pos.z);
	setPosition(pos.x, totalAvg, pos.z);
	return true;
}

// recalculate path to final destination
bool Vehicle::recalculatePath() {
#ifdef _GO_GRIDS_
	int xi, yi; // 2d initial coords
	int xf, yf; // 2d final coords
	Vector3 ownPos;
	getPosition(ownPos.x, ownPos.y, ownPos.z);
	g.mBrainGrid.calcGridPos(ownPos.x, ownPos.z, xi, yi); // get 2d initial coords
	g.mBrainGrid.calcGridPos(mFinalDestination.x, mFinalDestination.z, xf, yf); // get final 2d coords
	if ((xi == xf) && (yi == yf)) { // don't bother if target is current square
		return false;
	}
	clearAllWaypoints(); // reset waypoint information
	if (!g.mBrainGrid.FindPath(xi, yi, xf, yf, g.mBrainGrid.mCellsAcross, g.mBrainGrid.mCellsDown, mXWaypoint, mYWaypoint, mWaypointCount)) {
		printf("DEBUG: WARNING: could not find path!Vehicle::recalculatePath()\n"); // removed due to excessive clogging during crowd scenes
		return false; // could not find path
	}
	if (mWaypointCount >= MAX_WAYPOINTS) {
	 	printf("FATAL ERROR: maximum waypoints in path exceeded\n");
	 	exit(1);
	}
	mCurrentWaypointIndex = 0; // is this really necessary???
#endif
	return true;
}

// update various things - animations etc (things to do every frame).
bool Vehicle::update(unsigned long& elapsed) {
	// update GFS stats
	if (mSelected) {
		long int gen = mGALogger.mGeneration;
		char txt[256];
		sprintf(txt, "Gen: %li", gen);
		OverlayManager::getSingleton().getOverlayElement("mkiv/GFSGen")->setCaption(txt);
		long int ind = mGALogger.mBatch;
		sprintf(txt, "Indiv: %li", ind);
		OverlayManager::getSingleton().getOverlayElement("mkiv/GFSIndi")->setCaption(txt);
		double crash = mGALogger.mCollisionHeuristic;
		sprintf(txt, "Crash: %.2lf", crash);
		OverlayManager::getSingleton().getOverlayElement("mkiv/GFSCrash")->setCaption(txt);
		double speed = 0.0;
		if (mGALogger.mNextDataPoint > 0) {
			for (int i = 0; i < mGALogger.mNextDataPoint; i++) {
				speed += mGALogger.mSpeedDataPoint_kph[i];
			}
			speed = speed / (double)mGALogger.mNextDataPoint;
			speed = speed / pSpecPtr->mPerformance.mTopCrossKPH;
		}
		sprintf(txt, "Av.Speed: %.2lf", speed);
		OverlayManager::getSingleton().getOverlayElement("mkiv/GFSSpeed")->setCaption(txt);
		double miles = 1000.0 - mGALogger.mMileage_m;
		sprintf(txt, "left: %.0lfm", miles);
		OverlayManager::getSingleton().getOverlayElement("mkiv/GFSMiles")->setCaption(txt);
	}

	updateAnimations(elapsed); // continue updating animations
	if (mDestroyed) {
		return true;
	}
	if (mGotNavigationDestination) { // job 0: plan path
		unsigned long netTime = g.mSimTimer.getMicroseconds() - g.mTotalTimePaused;
		unsigned long remainder = netTime % STAGGERED_UPDATE_GROUPS;
		if (remainder == mRandomNumber) { // only recalculate path every second
			recalculatePath();
		}
	}
	mMainGunLoaded = reloadMainGun(elapsed); // job 1: reload
	mFiring = false;
	if (mGotPrimaryTarget) { // job 2: attack
		attack(elapsed);
		return true;
	}
	if (mGotNavigationDestination && !mImmobilised) { // job 3: drive
		if (mCurrentWaypointIndex == 0) { // Skip the square we start in unless it is also the target square
			if (mWaypointCount != 1) {
				mCurrentWaypointIndex++;
			}
		}
		if (mCurrentWaypointIndex >= mWaypointCount) { // end of path arrived at
			mGotNavigationDestination = false;
			mGotCurrentWaypoint = false;
				return true;
		}
		#ifdef _GO_GRIDS_
		if (!g.mBrainGrid.calcRealPos(mXWaypoint[mCurrentWaypointIndex], mYWaypoint[mCurrentWaypointIndex], mCurrentWaypoint.x, mCurrentWaypoint.z)) {
			printf("ERROR: calculating real pos for waypoint\n");
			return false;
		}
		#endif
		if (driveToCurrentWaypoint(elapsed)) {
			mCurrentWaypointIndex++;
			mCurrentDriveSpeed = 0.0; // reset driving speed to zero
			//printf("unit[%i], Vehicle::update setting steer speed\n", mUniqueIndexId);
			setCurrentSteerSpeed(0.0); // reset steering
			mGotCurrentWaypoint = false;
			stopDriving();
			startIdling(elapsed); // start animations and sounds for idling
		} else {
			stopIdling(); // stop animations and sounds for idling
			startDriving();
			mGotCurrentWaypoint = true;
		}
	} else {
		if (pSpecPtr->mPerformance.mFloats) {
			bobUpAndDown();
		} else if (mAmphibian.mIsSwimming) {
			balance();
		}
		if (!mImmobilised) {
			if (pSpecPtr->mAISpec.mIsInTraining) {
				if (pSpecPtr->mAISpec.mTrainingWPS > 0) {
				
					Vector3 waypoint;
					waypoint.x = pSpecPtr->mAISpec.mTrainingWPX[mCurrentTrainingWPIndex];
					waypoint.y = 0.0;
					waypoint.z = pSpecPtr->mAISpec.mTrainingWPZ[mCurrentTrainingWPIndex];
					setFinalDestination(waypoint); // give nav destination to unit
				
					mCurrentTrainingWPIndex++;
					mCurrentTrainingWPIndex = mCurrentTrainingWPIndex % 3;
				}
			}
			startIdling(elapsed); // start animations and sounds for idling
		} else if (pSpecPtr->mIsLandingCraft) {
			stopDriving();
			double roll = (double)mRoll;
			if (!mLandingCraft.lowerRamp(elapsed, mMesh, roll)) {
				printf("ERROR lowering ramp in vehicle::update\n");
				return false;
			}
		}
	}
	return true;
}

/*
I discovered the same problem (crashing with an assert error). I'm not doing any direct AAB usage but I was resetting the orientation of the nodes and doing a pitch() yaw() and roll(). The problem started when I took out the pitch and roll and was just doing something like this:

Code: Select all
    resetOrientation();
    yaw(mHeading);


It took me a while to figure out what the problem was (it's not obvious) - but that kind of code must be causing some kind of problem at certain values of rotation. I scrapped that and replaced it with additive yaw (rather than trying to reset and do the whole yaw each time):

Code: Select all
    yaw(addYaw);


That seemed to fix the problem. Not sure if it was because I was no longer doing the pitch and roll as well, or if my yaw was using bad values, but definitely seemed to be the source of the problem.

*/

// hacked 'rotate' function to get around Ogre's odd quarternion design
void Vehicle::performRotation() {
	mNode->resetOrientation();
	mNode->yaw(Radian(mHeading));
	mNode->pitch(Radian(mPitch));
	mNode->roll(Radian(mRoll));/*
	setHeading(mHeading);
	setPitch(mPitch);
	setRoll(mRoll);*/
}

void Vehicle::startMovementParticleEffects() {
	if (((pSpecPtr->mIsAmphibian) && (mAmphibian.mIsSwimming)) || (pSpecPtr->mIsLandingCraft)) {
		for (int i = 0; i < mDustSystemCount; i++) { // turn off dust
			mDustSystem[i]->getEmitter(0)->setEnabled(false);
		}
		for (int i = 0; i < mWakeSystemCount; i++) {
			mWakeSystem[i]->getEmitter(0)->setEnabled(true);
		}
	} else {
		for (int i = 0; i < mWakeSystemCount; i++) {
			mWakeSystem[i]->getEmitter(0)->setEnabled(false);
		}
		for (int i = 0; i < mDustSystemCount; i++) { // turn on dust
			mDustSystem[i]->getEmitter(0)->setEnabled(true);
		}
	}
}

void Vehicle::stopMovementParticleEffects() {
	for (int i = 0; i < mDustSystemCount; i++) { // turn off dust
		mDustSystem[i]->getEmitter(0)->setEnabled(false);
	}
	for (int i = 0; i < mWakeSystemCount; i++) {
		mWakeSystem[i]->getEmitter(0)->setEnabled(false);
	}
}


