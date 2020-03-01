#include "Infantry.h"

Vector3 Infantry::getWaypoint(int index) {
	Vector3 waypoint;
	if ((index < 0) || (index >= mWaypointCount)) {
		printf("ERROR: waypoint is not valid requested to Unit::getWaypoint()\n");
		return waypoint;
	}
	#ifdef _GO_GRIDS_
	g.mSmallGrid.calcRealPos(mXWaypoint[index], mYWaypoint[index], waypoint.x, waypoint.z);
	#endif
	return waypoint;
}

// balance the soldier on the terrain
bool Infantry::balance() {
	if (!mInitialised) {
		printf("ERROR: infantry unit not initialised so can not balance\n");
		return false;
	}
	double pos[3];
	getPosition(pos[0], pos[1], pos[2]); // 2. get X,Y,Z position of vehicle (in metres)
	pos[1]	= pTerrainHandler->getTerrainHeightAt(pos[0], pos[2]); // get terrain height under
	setPosition(pos[0], pos[1], pos[2]);
	performRotation();
	return true;
}

// recalculate path to final destination
bool Infantry::recalculatePath() {
#ifdef _GO_GRIDS_
	int xi, yi; // 2d initial coords
	int xf, yf; // 2d final coords
	Vector3 ownPos;
	getPosition(pos.x, pos.y, pos.z); // 2. get X,Y,Z position of vehicle (in metres)
	g.mSmallGrid.calcGridPos(ownPos.x, ownPos.z, xi, yi); // get 2d initial coords
	g.mSmallGrid.calcGridPos(mFinalDestination.x, mFinalDestination.z, xf, yf); // get final 2d coords
	if ((xi == xf) && (yi == yf)) { // don't bother if target is current square
		return false;
	}
	clearAllWaypoints(); // reset waypoint information
	if (g.mSmallGrid.ManhattanDistance(xi, yi, xf, yf) > 100) { // get partial path for long-distance searches?
	} else { // Otherwise only accept full path
	 	if (!g.mSmallGrid.FindPath(xi, yi, xf, yf, g.mSmallGrid.mCellsAcross, g.mSmallGrid.mCellsDown, mXWaypoint, mYWaypoint, mWaypointCount)) {
	 		//printf("ERROR: could not find path!\n"); removed this printf cos it was clogging at dense crowds
	 		return false; // could not find path
	 	}
	}
	if (mWaypointCount >= MAX_WAYPOINTS) {
	 	printf("FATAL ERROR: maximum waypoints in path exceeded\n");
	 	exit(1);
	}
	mCurrentWaypointIndex = 0; // TODO is this really necessary??? double check vs evo3 code
	#endif
	return true;
}

// update various things - animations etc (things to do every frame).
bool Infantry::update(unsigned long& elapsed) {
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
	if (mGotNavigationDestination) { // job 3: drive
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
		if (!g.mSmallGrid.calcRealPos(mXWaypoint[mCurrentWaypointIndex], mYWaypoint[mCurrentWaypointIndex], mCurrentWaypoint.x, mCurrentWaypoint.z)) {
			printf("ERROR: calculating real pos for waypoint\n");
			return false;
		}
		#endif
		if (driveToCurrentWaypoint(elapsed)) {
			mCurrentWaypointIndex++;
			mCurrentDriveSpeed = 0.0f; // reset driving speed to zero
			mCurrentSteerSpeed = 0.0f; // reset steering
			mGotCurrentWaypoint = false;
			stopDriving();
			startIdling(elapsed); // start animations and sounds for idling
		} else {
			stopIdling(); // stop animations and sounds for idling
			startDriving();
			mGotCurrentWaypoint = true;
		}
	} else {
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
	return true;
}

// hacked 'rotate' function to get around Ogre's odd quarternion design
void Infantry::performRotation() {
	setHeading(mHeading);
}

void Infantry::startMovementParticleEffects() {
}

void Infantry::stopMovementParticleEffects() {
}

