#include "Unit.h"
#include "../usefulFuncs.h"
#include "../audio/AudioHandler.h"
#include <string.h>
#include <math.h>
using namespace std;

// default var values
Unit::Unit() {
	pTerrainHandler = NULL;	// pointer to the Terrain Handler
	pSpecPtr = NULL;
	mMesh = NULL;
	mNode = NULL;
	mAnimationState = NULL;
	for (int i = 0; i < 6; i++) {
		mDustSystem[i] = NULL;
	}
	mWakeSystem[0] = mWakeSystem[1] = NULL;
	mCurrentWaypoint.x = mCurrentWaypoint.y = mCurrentWaypoint.z = 0.0;
	mPrimaryTarget.x = mPrimaryTarget.y = mPrimaryTarget.z = 0.0;
	mMainGunElevation = 0.0;
	mCurrentDriveSpeed 	= 0.0;
	mCurrentSteerSpeed	= 0.0;
	mPreviousDriveSpeed = 0.0;
	mMaxAllowedCurrentTerrainSpeed = 0.0;
	mPosition[0] = mPosition[1] = mPosition[2] = 0.0;
	mPitch = mHeading = mRoll = 0.0;
	mPrevPitch = mPrevYaw = mPrevRoll = 0.0;
	mTurretOrientation = 0.0;
	mSquaredDistToObstacle = 0.0; // navigation properties
	mSquaredDistToNavTarget = 0.0; // navigation properties
	mAngleToNavTarget = 0.0; // navigation properties
	mAngleToObstacle = 0.0; // navigation properties
	mRandomNumber = rand() % STAGGERED_UPDATE_GROUPS; // spread over 1 second
	mUniqueIndexId = 0;
	mSmokeInVehicleDelay = 0;
	mAimMainCannonDelay = 3000000; // 3 seconds aiming time default
	mReloadMainGunDelay = 0;
	mStartDelay = 0;
	mPrimaryTargetIndex = -1;
	mForce = -1;
	mDriveSoundChannel = mIdleSoundChannel = -1;
	mDustSystemCount = 0;
	mWakeSystemCount = 0;
	mCollisions = 0;
	mWaypointCount = 0;
	mCurrentWaypointIndex = 0;
	mCurrentTrainingWPIndex = 0;
	mNumberOfCurrentOccupants = 0;
	mNumberOfLightVehiclesStowed = 0;
	mIndexOfUnitBeingRidden = -1;
	mNumberOfAuxTurretsAttached = 0;
	mInitialised = false;
	mGotNavigationDestination = false;
	mGotCurrentWaypoint = false;
	mGotPrimaryTarget = false;
	mSelected = false;
	mMainGunLoaded = true;
	mFiring = false;
	mHasNearestObstacle = false;
	mWasTurning = false;
	mDestroyed = false;
	mCanSeeTarget = false;
	mImmobilised = false;
	mIsRidingOtherUnit = false;
}

// create the vehicle from a specification file, and attached to a given node
bool Unit::init(TerrainHandler *terrHdlr, VehicleSpec* specptr, Entity* mesh, SceneNode* node,Vector3& pos, double boundRad, int force) {
	if (!terrHdlr) { // validate params
		printf("ERROR: Terrain Handler is NULL\n");
		return false;
	}
	if (!specptr) { // validate params
		printf("ERROR: supplied specification file is empty\n");
		return false;
	}
	if (!mesh) { // validate params
		printf("ERROR: supplied mesh is empty\n");
		return false;
	}
	if (!node) { // validate params
		printf("ERROR: supplied node is empty\n");
		return false;
	}
	pTerrainHandler = terrHdlr;	// pointer to the Terrain Handler
	mMesh = mesh; // init from parameters
	mForce = force; // init from parameters
	pSpecPtr = specptr; // init from parameters
	mNode = node; // init from parameters
	//printf("DEBUG: Unit::init ->setPosition(%lf, %lf, %lf)\n", pos.x, pos.y, pos.z);
	setPosition(pos.x, 1000.0, pos.z); // put in a position in the world
	if (pSpecPtr->mAutoScale) { // scale the vehicle to correct size in meters
		double length = pSpecPtr->mHull.length_m;
		double width = pSpecPtr->mHull.width_m;
		double longest = length;
		if (width > length) { // get longest edge (for the odd cases where vehicle is wider than it is long)
			longest = width;
		}
		double scaleFactor = 1.0 / (boundRad / longest); // get factor to scale by
		node->setScale(scaleFactor, scaleFactor, scaleFactor); // scale node and corresponding bits (equally on all axiis)
	}
	mStartDelay = rand() % 100000; // set up a start delay for the idle sound and animation
	if (pSpecPtr->mHasIdleAnimation) { // Set idle animation (if it has one)
		mAnimationState = mMesh->getAnimationState(pSpecPtr->mIdleAnimationName);
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
		mAnimationState->addTime(mStartDelay); // start at a random point so that groups of tanks look more natural
	}
	mStartDelay *= 10; // scale up a bit for audio (distributes vehicles over 1st second)
	mInitialised = true;
	if (!balance()) {	// balance on terrain at that position
		printf("ERROR: could not balance\n");
		return false;
	}
	printf("loading fuzzy set definitions from %s\n", pSpecPtr->mAISpec.mFuzzySetsFile);
	mFuzzyMod.clear(); // reset navigator
	mFuzzyMod.readSetsFromFile(pSpecPtr->mAISpec.mFuzzySetsFile); // load set file
	printf("loading fuzzy rules from %s\n", pSpecPtr->mAISpec.mFuzzyRulesFile);
	mFuzzyMod.readRulesFromFile(pSpecPtr->mAISpec.mFuzzyRulesFile); // load rules file
	if (pSpecPtr->mIsLandingCraft) {
		strcpy (mLandingCraft.mRampBoneName, pSpecPtr->mRampBoneName);
	}
	if (pSpecPtr->mAISpec.mIsInTraining) { // set logfile name for GA fitness log
		strcpy(mGALogger.mLogFileName, pSpecPtr->mAISpec.mFitnessLogFilename);
		mGALogger.mGeneration = mFuzzyMod.mRuleBase.mGeneration;
		mGALogger.mBatch = mFuzzyMod.mRuleBase.mBatch;
	}
	return true;
}

// pass control of a dust emitter to the vehicle
bool Unit::addDustController(ParticleSystem* systemPtr) {
	if (!systemPtr) {
		printf("ERROR: in call addDustController(system) - system was NULL\n");
		return false;
	}
	if (mDustSystemCount >= MAX_DUST_EFFECTS) {
		printf("ERROR: vehicle was given more than the maximum (6) dust controllers\n");
		return false;
	}
	mDustSystem[mDustSystemCount] = systemPtr; // add system to internal array
	mDustSystem[mDustSystemCount]->getEmitter(0)->setEnabled(false); // start controller as 'disabled'
	mDustSystemCount++; // increment count
	return true;
}

// pass control of a wake emitter to the vehicle
bool Unit::addWakeController(ParticleSystem* systemPtr) {
	if (!systemPtr) {
		printf("ERROR: in call addWakeController(system) - system was NULL\n");
		return false;
	}
	if (mWakeSystemCount >= 2) {
		printf("ERROR: vehicle was given more than the maximum (2) wake controllers\n");
		return false;
	}
	mWakeSystem[mWakeSystemCount] = systemPtr; // add system to internal array
	mWakeSystem[mWakeSystemCount]->getEmitter(0)->setEnabled(false); // start controller as 'disabled'
	mWakeSystemCount++; // increment count
	return true;
}

void Unit::attack(unsigned long& elapsed) {
	if (!mInitialised) { // validate
		printf("FATAL ERROR: can not attack in Unit::attack - veh not initialised\n");
		exit(1);
	}
	if (!mCanSeeTarget) {
		stopDriving();
		stopAnimations();
		return; // just wait if can not see target
	}
	bool facingTarget = false;
	if (pSpecPtr->mHasMainTurret) { // job 2.1: turn towards target
		if (rotateTurretToFace(mPrimaryTarget, elapsed)) {
			facingTarget = true; // already on target
		} else {
			// <- play turret crank audio here
			stopDriving();
			stopAnimations();
			return;
		}
	} else {
		if (rotateHullToFace(mPrimaryTarget, elapsed)) { // make sure that hull faces target
			facingTarget = true; // already on target
			if (mWasTurning) { // run this only once after driving finished
				mWasTurning = false;
				stopDriving();
				stopAnimations();
			}
		} else {
			stopIdling();
			startDriving(); // play move anim and sound
			mWasTurning = true;
			return;
		}
	}
	if (mAimMainCannonDelay > 0) { // wait until gun aimed
		mAimMainCannonDelay -= elapsed;
		return;
	}
	if (mSmokeInVehicleDelay > 0) { // wait for smoke to clear
		mSmokeInVehicleDelay -= elapsed;
		return;
	}
	if (!aimCannon(0, mPrimaryTarget, elapsed)) { // job 2.2.3 now angle the gun...
		return;
	}
	if (!mMainGunLoaded) { // wait here until reloaded
		return;
	}
	fireMainGun(); // fire the main gun
	mSmokeInVehicleDelay = 3000000; // smoke fills the cabin
	mAimMainCannonDelay = 3000000; // reset aiming delay
	return;
}

// move towards current waypoint. return true if already there
bool Unit::driveToCurrentWaypoint(unsigned long &elapsed) {
	if (mIsRidingOtherUnit) {
		return true;
	}

	// Set max speed based on current terrain
	mMaxAllowedCurrentTerrainSpeed = pSpecPtr->mPerformance.mTopCrossMPUS;
	if (pSpecPtr->mIsAmphibian) {
		if (mAmphibian.mIsSwimming) {
			mMaxAllowedCurrentTerrainSpeed = pSpecPtr->mPerformance.mTopWaterMPUS;
		}
	} else if (pSpecPtr->mIsLandingCraft) {
		mMaxAllowedCurrentTerrainSpeed = pSpecPtr->mPerformance.mTopWaterMPUS;
	}

	bool avoidingObstacle = false;
	bool seekingTarget = false;
	Vector3 ownPos;
	getPosition(ownPos.x, ownPos.y, ownPos.z); // get own position components
	double ownSize = mMesh->getBoundingRadius(); // get own size to subtract from distance
	double xDist = (double)fabs(mCurrentWaypoint.x - ownPos.x); // Calculate x Distance to Point
	double zDist = (double)fabs(mCurrentWaypoint.z - ownPos.z); // Calculate z Distance to Point
	mSquaredDistToNavTarget = xDist * xDist + zDist * zDist; // squared distance to nav target (quicker to calculate than actual distance, but can only be used for comparison with other squared distances)
	mSquaredDistToNavTarget = mSquaredDistToNavTarget - ownSize * ownSize; // shorten distance by own size
	xDist = (double)fabs(mFinalDestination.x - ownPos.x); // Calculate x Distance to Point
	zDist = (double)fabs(mFinalDestination.z - ownPos.z); // Calculate z Distance to Point
	double squaredDistToFinalNavTarget = xDist * xDist + zDist * zDist; // as above
	squaredDistToFinalNavTarget = squaredDistToFinalNavTarget - ownSize * ownSize; // as above
	if (squaredDistToFinalNavTarget < 2.0) { // check if close enough to target
		return true;
	}
	double opposite = mCurrentWaypoint.x - ownPos.x; // Calculate Angle to next waypoint
	double adjacent = mCurrentWaypoint.z - ownPos.z;
	double theta = atan2f(opposite, adjacent);
	normaliseRadian(theta);
	mAngleToNavTarget = mHeading - theta;
	normaliseRadian(mAngleToNavTarget);
	bool obstacleOnLeftSide = true;
	if (mHasNearestObstacle) { // Get Difference Between Own heading and angle to obstacle
		double opposite = mNearestObstaclePosition.x - ownPos.x;
		double adjacent = mNearestObstaclePosition.z - ownPos.z;
		double theta = atan2f(opposite, adjacent);
		normaliseRadian(theta);
		mAngleToObstacle = mHeading - theta;
		normaliseRadian(mAngleToObstacle);
		if (mAngleToObstacle < M_PI) { // Decide if on Left or Right of Tank
			obstacleOnLeftSide = false;
		}
	} // ==fire up the fuzzy navigation system==
	double obstAvoidanceSpeed = 0.0;
	double obstAvoidanceAngle = 0.0;
	if ((mHasNearestObstacle) && (mSquaredDistToObstacle < mSquaredDistToNavTarget) && ((mAngleToObstacle < mFuzzyMod.getObstacleAngleThreshold()) || (mAngleToObstacle > 6.28f - mFuzzyMod.getObstacleAngleThreshold()))) { // when to avoid obstacle
		//printf("DB: engaging OA\n");
		avoidingObstacle = true;
		mFuzzyMod.processObstacleAvoidance(mAngleToObstacle, mSquaredDistToObstacle);
		obstAvoidanceSpeed = mFuzzyMod.getObstacleAvoidanceSpeed() * mMaxAllowedCurrentTerrainSpeed;
		//printf("DEBUG: oAA(%lf) = mFuzzyMod.getObstacleAvoidanceAngle()(%lf) * pSpecPtr->mPerformance.mTopCrossMPUS(%lf)\n", mFuzzyMod.getObstacleAvoidanceAngle() * pSpecPtr->mPerformance.mTopCrossMPUS, mFuzzyMod.getObstacleAvoidanceAngle(), pSpecPtr->mPerformance.mTopCrossMPUS);
		obstAvoidanceAngle = mFuzzyMod.getObstacleAvoidanceAngle() * mMaxAllowedCurrentTerrainSpeed;
		if ((obstacleOnLeftSide) && (mAngleToObstacle < 6.2f)) {
			obstAvoidanceAngle *= -1.0; // TODO make sure that this is correct w/ a printout
		}
		mCurrentDriveSpeed = obstAvoidanceSpeed;
		//printf("DEBUG: unit[%i], Unit::driveToCurrentWaypoint setting steer speed\n", mUniqueIndexId);
		setCurrentSteerSpeed(obstAvoidanceAngle);
	} else {
		//printf("DB: TS only\n");
	}
	// target seeking
	double targSpeed = 0.0;
	double targAngle = 0.0;
	seekingTarget = true;
	mFuzzyMod.processTargetSeeking(mAngleToNavTarget, squaredDistToFinalNavTarget); // calculate inputs

	targSpeed = mFuzzyMod.getTargetSeekingSpeed() * mMaxAllowedCurrentTerrainSpeed; // get speed delta output for acceleration
	targAngle = mFuzzyMod.getTargetSeekingAngle() * mMaxAllowedCurrentTerrainSpeed; // get angle delta output for steering (it's also scaled against max driving speed)
	if (mAngleToNavTarget < M_PI) {
		targAngle *= -1; // invert side
	}
	if (avoidingObstacle) { // blend OA and TS
		mCurrentDriveSpeed = (obstAvoidanceSpeed + targSpeed) / 2.0;
		//printf("unit[%i], Unit::driveToCurrentWaypoint(b) setting steer speed\n", mUniqueIndexId);
		setCurrentSteerSpeed((obstAvoidanceAngle + targAngle) / 2.0);
	} else { // just ts
		mCurrentDriveSpeed = targSpeed;
		//printf("unit[%i], Unit::driveToCurrentWaypoint(c) setting steer speed\n", mUniqueIndexId);
		setCurrentSteerSpeed(targAngle);
	}

	limitSpeedByClimbPhysics(); // uphill physics approximation for slopes
	limitSteeringByVehiclePhysics(elapsed); // make sure that steering is within maximum steering limit
	limitAccelerationByVehiclePhysics(elapsed); // keep acceleration within vehicle performance spec limits
	limitSpeedBySteeringFriction(elapsed); // further reduce speed if vehicle is turning

	//printf("DEBUG: U::driveToCW mHeading(%lf) = mHeading(%lf) + mCurrentSteerSpeed(%lf) * elapsed(%li)\n", mHeading + mCurrentSteerSpeed * elapsed, mHeading, mCurrentSteerSpeed, elapsed);
	mHeading = mHeading + mCurrentSteerSpeed * elapsed;
	balance();
	double xVelocity = (sinf(mHeading) * mCurrentDriveSpeed) * elapsed; 	// calculate object x movement
	double zVelocity = (cosf(mHeading) * mCurrentDriveSpeed) * elapsed;	// calculate object z movement
	Vector3 previousPos = ownPos;
	ownPos.x = ownPos.x + xVelocity;
	if ((ownPos.x > 1500.0) || (ownPos.x < -1500.0)) {
		printf("WARNING: vehicle is attempting to drive out of bounds\n");
		ownPos.x = 0.0;
	}
	ownPos.z = ownPos.z + zVelocity;
	if ((ownPos.z > 1500.0) || (ownPos.z < -1500.0)) {
		printf("WARNING: vehicle is attempting to drive out of bounds\n");
		ownPos.z = 0.0;
	}

	if (pSpecPtr->mAISpec.mIsInTraining) { // update GA fitness logs
		double distance = previousPos.distance(ownPos);
		mGALogger.recordMileage(distance, mCurrentDriveSpeed, pSpecPtr->mPerformance.mTopCrossKPH); // update GA mileage
	}
	//printf("DEBUG: Unit::driveToCurrentWaypoint ->setPosition\n");
	setPosition(ownPos.x, ownPos.y, ownPos.z); // Move the actual object
	balance(); // Move On terrain
	mHasNearestObstacle = false; // reset obstacle until next round
	return false;
}

// uphill physics approximation for slopes (approximation)
void Unit::limitSpeedByClimbPhysics() {
	if (mPitch < 0.0) {
		double prop = 1.0 - mPitch / -pSpecPtr->mPerformance.mMaxSlopeRad * 2.0; // double maximum slope (not realistic but easier to deal with)
		double maxSpeed = mMaxAllowedCurrentTerrainSpeed * prop;
		if (maxSpeed < 0.10 * mMaxAllowedCurrentTerrainSpeed) {
			maxSpeed = 0.10 * mMaxAllowedCurrentTerrainSpeed;
		}
		if (mCurrentDriveSpeed > maxSpeed) {
			double changeProp = maxSpeed / mCurrentDriveSpeed;
			mCurrentDriveSpeed = (mCurrentDriveSpeed + maxSpeed) / 2.0;
			//printf("unit[%i], Unit::limitSpeedByClimbPhysics setting steer speed\n", mUniqueIndexId);
			setCurrentSteerSpeed(mCurrentSteerSpeed * changeProp);
		}
	}
}

void Unit::limitSpeedBySteeringFriction(const unsigned long& elapsed) {
// TODO redo this. also look at the limitSteeringByVehiclePhysics() func * elapsed as may have similar issues
	/*double proportionOfSteering = (fabs(mCurrentSteerSpeed) * elapsed) / (pSpecPtr->mPerformance.mTopSteerRadUS);
	double proportionOfSpeed = 1.0 - MAX_STEER_FRICTION_FACTOR * proportionOfSteering;
	mCurrentDriveSpeed = mCurrentDriveSpeed * proportionOfSpeed;
	printf("DB: steering (%lf) friction reducing speed to %lf of original\n", proportionOfSteering, proportionOfSpeed);*/
}

// make sure that steering is within maximum steering limit (approximation)
void Unit::limitSteeringByVehiclePhysics(const unsigned long& elapsed) {
	if (mCurrentSteerSpeed * elapsed > pSpecPtr->mPerformance.mTopSteerRadUS) {
		//printf("unit[%i], Unit::limitSteeringByVehiclePhysics setting steer speed\n", mUniqueIndexId);
		setCurrentSteerSpeed(pSpecPtr->mPerformance.mTopSteerRadUS); // limit yaw to vehicle's maximum steering
	} else if (mCurrentSteerSpeed * elapsed < -pSpecPtr->mPerformance.mTopSteerRadUS) {
		//printf("unit[%i], Unit::limitSteeringByVehiclePhysics(b) setting steer speed\n", mUniqueIndexId);
		setCurrentSteerSpeed(-pSpecPtr->mPerformance.mTopSteerRadUS);
	}
}

// keep acceleration within vehicle performance spec limits
void Unit::limitAccelerationByVehiclePhysics(const unsigned long& elapsed) {
	double accelLimit_s = pSpecPtr->mPerformance.getMaxAccelerationS(mMaxAllowedCurrentTerrainSpeed);
	double differenceInSpeed_frame = (mCurrentDriveSpeed - mPreviousDriveSpeed) * 1000000.0;
	double secondsPassed = (double)elapsed / 1000000.0;
	double differenceInSpeed_s = differenceInSpeed_frame / secondsPassed;
	if (differenceInSpeed_s > accelLimit_s) {
		mCurrentDriveSpeed = mPreviousDriveSpeed + (accelLimit_s / 1000000.0 * secondsPassed); // limit acceleration
	}
#ifdef ENABLE_BRAKE_DAMPENING
	if (differenceInSpeed_s < 0.0) { // braking
		double brakingLimit_s = -accelLimit_s * 2.0;
		if (differenceInSpeed_s < accelLimit_s) {
			mCurrentDriveSpeed = mPreviousDriveSpeed - (brakingLimit_s / 1000000.0 * secondsPassed); // limit braking
		}
	}
#endif
	mPreviousDriveSpeed = mCurrentDriveSpeed;
}

// rotate the hull to face a particular spot
bool Unit::rotateHullToFace(Vector3 &target, unsigned long &elapsed) {
	if (!mInitialised) {
		printf("FATAL ERROR: vehicle was not initialised in Unit::rotateHullToFace()\n");
		exit(1);
	}
	bool rotateNegatively = false;
	Vector3 ownPos;
	getPosition(ownPos.x, ownPos.y, ownPos.z); // get own position components
	double opposite = target.x - ownPos.x; // check if facing right direction
	double adjacent = target.z - ownPos.z;  // check if facing right direction
	double theta = atan2f(opposite, adjacent); // Calculate Angle to next waypoint
	normaliseRadian(theta);
	normaliseRadian(mHeading);
	double range = mHeading - theta;
	double angleOppositeYRot = mHeading + M_PI; // Get the opposite angle to YRot
	normaliseRadian(angleOppositeYRot);
	if ((double)fabs(range) <= (double)fabs(pSpecPtr->mPerformance.mTopSteerRadUS * elapsed)) { // Check if already on target
		mHeading = theta;
		return true;
	}
	if (mHeading < M_PI) { // Check if it's quicker to rotate the other way around
		if ((theta < mHeading) && (theta < angleOppositeYRot)) {
			rotateNegatively = true;
		} else if ((theta > mHeading) && (theta > angleOppositeYRot) ) {
			rotateNegatively = true;
		}
	} else {
		if ((theta < mHeading) && (theta > angleOppositeYRot)) {
			rotateNegatively = true;
		}
	}
	double yaw = 0.0;
	if (rotateNegatively) { // Rotate incrementally
		yaw -= pSpecPtr->mPerformance.mTopSteerRadUS * elapsed;
	} else {
		yaw += pSpecPtr->mPerformance.mTopSteerRadUS * elapsed;
	}
	mHeading = mHeading + yaw;
	balance();
	return false;

}
SceneNode* Unit::getNodePtr() {
	return mNode;
}

// get the world-relative orientation of the main gun as a 3d vector
Vector3 Unit::getGunWorldOrientation() {
	SkeletonInstance* skeleton = NULL;
	skeleton = mMesh->getSkeleton(); // get a pointer to the skeleton
	if (!skeleton) { // double check skeleton (probably unnecessary)
		printf("FATAL ERROR: failed to access the skeleton in the mesh\n");
		exit(1);
	}
	// get the bone
	Bone* muzzleBone = NULL; // try and get the armature from the cannon
	muzzleBone = skeleton->getBone(pSpecPtr->mCannon.mMuzzleBone); // get a pointer to attaching bone in hull
	if (!muzzleBone) { // validate the new armature pointer was successful
		printf("ERROR: failed to find the bone [%s] specified in .spec file.\n", pSpecPtr->mCannon.mMuzzleBone);
	}
	Quaternion fullOrientation = mMesh->getParentSceneNode()->_getDerivedOrientation() * muzzleBone->_getDerivedOrientation(); // get the bone's world rotation by cross producting it with the entity's (mesh's) world orientation
	Vector3 forwardBoneVector = fullOrientation.yAxis();
	return forwardBoneVector;
}

// get world position of muzzle of main gun
Vector3 Unit::getMainGunMuzzleWorldPosition() {
	if (!mInitialised) { // validate
		printf("FATAL ERROR: Unit::getMainGunMuzzleWorldPosition - Unit not initialised\n");
		exit(1);
	}
	if (!pSpecPtr) {
		printf("FATAL ERROR: No pSpecPtr allocated [getMainGunMuzzleWorldPosition()]\n");
			exit(1);
	}
	if (!mMesh->hasSkeleton()) { // make sure that the mesh has a skeleton
		printf("FATAL ERROR: the mesh does not have a skeleton\n");
		exit(1);
	}
	SkeletonInstance* skeleton = NULL;
	skeleton = mMesh->getSkeleton(); // get a pointer to the skeleton
	if (!skeleton) { // double check skeleton (probably unnecessary)
		printf("FATAL ERROR: failed to access the skeleton in the mesh\n");
		exit(1);
	}
	Bone* muzzleBone = NULL; // try and get the armature from the cannon
	muzzleBone = skeleton->getBone(pSpecPtr->mCannon.mMuzzleBone); // get a pointer to attaching bone in hull
	if (!muzzleBone) { // validate the new armature pointer was successful
		printf("ERROR: failed to find the bone [%s] specified in .spec file.\n", pSpecPtr->mCannon.mMuzzleBone);
	}
//	Vector3 boneOffsetPos = muzzleBone->_getDerivedPosition(); // get world position of bone
	Vector3 muzzlePos = mMesh->_getParentNodeFullTransform() * muzzleBone->_getDerivedPosition(); // get the 'actual' world pos of bone

	return muzzlePos; // return the resolved world bone position
}

/* New idea for aiming - "line the gun up with the target"

  a = angle between muzzle and target
  b = angle between breech and muzzle (i.e along barrel)
  c = any angle adjustment for gravity's effect on projectile
  while (b != a + c) {
		* check if rotating turret cw or acw increases or decreases the difference between angle
    * same with pitch of gun
	}

  - Alternative idea (possibly more realistic) -
  1. check slope of terrain - if pitch or roll > some threshold then no cannon fire possible
  2. otherwise traverse turret until facing (see above method)
  3. then pitch gun until correct angle reached
  4. then swivel the gun itself (up to 15 degrees either side?) to make any correction or tweaks due to terrain
*/

// aim the/a turret // TODO totally revise this function
bool Unit::rotateTurretToFace(Vector3 &target, unsigned long &elapsed) {
	/* TODO
	* rename new method 'traverseTurret(int turretIndex, Vector3 &target, unsigned long &elapsed);'
	* first validate turret, and ask spec for bone
	* do all rotatation based on the position and orientation of the weapon that we are aiming.
	  this way we can aim forward and rear guns equally, and use a more realistic aiming scheme
	  although we may have to also pre-respect the (potential) gun (sub) traverse and pitch as well
	*/
	// get turret bone
	if (!mMesh->hasSkeleton()) { // make sure that the mesh has a skeleton
		printf("FATAL ERROR: the mesh does not have a skeleton\n");
		exit(1);
	}
	SkeletonInstance* skeleton = NULL;
	skeleton = mMesh->getSkeleton(); // get a pointer to the hull's skeleton
	Bone* turretBone = NULL;
	turretBone = skeleton->getBone(pSpecPtr->mTurret.mTraverseBone); // get the name of the traverse bone for this turret
	if (!turretBone) {
		printf("FATAL ERROR: failed to find the turret joint\n");
		exit(1);
	}
	turretBone->setManuallyControlled(true);

	// ------------------ start of old traversal function ------------------
	bool rotateNegatively = false;
	// get own position components
	Vector3 ownPos;
	getPosition(ownPos.x, ownPos.y, ownPos.z); // TODO this is wrong - get the muzzle bone position
	// check if facing right direction
	double opposite = target.x - ownPos.x; // Calculate Angle to target
	double adjacent = target.z - ownPos.z;
	double theta = atan2f(opposite,adjacent);
	normaliseRadian(theta);
	// get absolute angle of turret
	double absFacing = mHeading + mTurretOrientation; // TODO wrong - get muzzle bone orientation
	normaliseRadian(absFacing);
	double range = absFacing - theta;
	double angleOppositeYRot = absFacing + M_PI; // Get the opposite angle to YRot
	normaliseRadian(angleOppositeYRot);
	double traverseSpeed = pSpecPtr->mTurret.mTraverseRadS / 1000000.0; // get traverse speed from spec file and convert to museconds
	if ((double)fabs(range) <= (double)fabs(traverseSpeed * elapsed)) { // Check if already on target
		mTurretOrientation = theta - mHeading;
		turretBone->resetOrientation();
		turretBone->yaw(Radian(mTurretOrientation));
		return true;
	}
	// Check if it's quicker to rotate the other way around
	if (absFacing < M_PI) {
		if ((theta < absFacing) && (theta < angleOppositeYRot)) {
			rotateNegatively = true;
		} else if ((theta > absFacing) && (theta > angleOppositeYRot) ) {
			rotateNegatively = true;
		}
	} else {
		if ((theta < absFacing) && (theta > angleOppositeYRot)) {
			rotateNegatively = true;
		}
	}
	double yaw = mTurretOrientation;
	if (rotateNegatively) { // Rotate incrementally
		yaw -= traverseSpeed * elapsed;
	} else {
		yaw += traverseSpeed * elapsed;
	}
	mTurretOrientation = yaw;
	normaliseRadian(mTurretOrientation);
	turretBone->resetOrientation();
	turretBone->yaw(Radian(mTurretOrientation));
	return false;
}

/** aim one of the vehicle's cannon using a process emulating actual artillery calculations done by crew
* this is a per-time unit process so needs to be called in iterations  */
bool Unit::aimCannon(int cannonIndex, const Vector3 &target, unsigned long &elapsed) {
	if (!pSpecPtr->mCannon.mHasPivotBone) { // 4. make sure that the cannon has a pivot joint in its specification
		return true;
	}
	if (!mMesh->hasSkeleton()) { // 3. make sure that the mesh has a skeleton
		printf("FATAL ERROR: the mesh does not have a skeleton [Unit::aimCannon()]\n");
		exit(1);
	}
	SkeletonInstance* skeleton = NULL; // set up a pointer for the skeleton
	skeleton = mMesh->getSkeleton(); // get a pointer to the hull's skeleton
	if (!skeleton) {
		printf("FATAL ERROR: failed to find the skeleton [Unit::aimCannon()]\n");
		exit(1);
	}
	// 2. get a reference to the bone
	Bone* mainCannonPivotBone = NULL; // set up a pointer for the bone
	mainCannonPivotBone = skeleton->getBone(pSpecPtr->mCannon.mPivotBone); // get a pointer the cannon pivot joint
	if (!mainCannonPivotBone) {
		printf("FATAL ERROR: failed to find the mainCannonPivotBone joint [Unit::aimCannon()]\n");
		exit(1);
	}
	// 3. take hard control of the pivot joint
	mainCannonPivotBone->setManuallyControlled(true); // take control of the cannon pitch

// == PART THREE - OBTAIN REQUIRED VARIABLES ==
// bool calculateAimForCannon(int cannonIndex, const Vector3 &target, isPrimaryCannon = false);
	// 1. Get the (should be estimated) distance between us and the target
	// 1.1 Get own position (actually the position of the cannon pivot to be extra-accurate)
	Vector3 ownPos = mMesh->_getParentNodeFullTransform() * mainCannonPivotBone->_getDerivedPosition(); // get the world pos of bone
	// 1.2 get the distance between the gun pivot and the target
	double xDistanceToTarget = fabs(ownPos.x - target.x);
	double zDistanceToTarget = fabs(ownPos.z - target.z);
	double horizDistanceToTarget = sqrt(xDistanceToTarget * xDistanceToTarget + zDistanceToTarget * zDistanceToTarget);
	// TODO: remove sqrt() and work out as a squard distance
	// 1.3 TODO factor in any inaccuracies due to equipment and crew rating here

	// 2. Get muzzle velocity
	// 2.1 TODO Find out what type of ammunition is loaded into the cannon
	// 2.2 Get from ammunition specification TODO fixup to match 2.1
	// double muzzleVelocity = cannon->mAmmoType[0]->mMuzzleVelocity; // at the moment the ammo is defaulted to type '0'
	// 3. Get height difference
	double heightDifference = target.y - ownPos.y;	// a positive difference here means that the target is higher up

	// 4. Derive arc of pitch between self and target (easy to get this wrong)
	// 4.1 derive trig
	double theta = atan(heightDifference / horizDistanceToTarget); // work out angle. Note that if target is higher up angle should be -ve

// == PART FOUR - CONTINUE PITCHING THE CANNON ==
// pitchCannon(int cannonIndex, const Vector3 &target,unsigned long &elapsed);
// in here we call the function for PART THREE but only when necessary - not every frame
// **** UNDER (MESSY) CONSTRUCTION ****

	// X. automatically set pitch instantly to correct angle assuming that the tank is on flat terrain
	mainCannonPivotBone->setOrientation(mainCannonPivotBone->getInitialOrientation()); // reset orientation by an unusual method because
	mainCannonPivotBone->yaw(Radian(-theta)); // perform rotation negatively as aiming 'positively' means down in this engine

	return true; // report that we are on target and cleared to fire
}

// get the current speed of motion forwards in k/h
double Unit::getCurrentDriveSpeedKPH() {
	double metersPerHour = mCurrentDriveSpeed * 3600000000.0;
	double kmPerHour = metersPerHour / 1000.0;
	return kmPerHour;
}

double Unit::getCurrentSteerSpeed() {
	return mCurrentSteerSpeed;
}

void Unit::getPosition(double& x, double& y, double& z) { // get position of hull
	x = mPosition[0];
	y = mPosition[1];
	z = mPosition[2];
}

void Unit::getPosition(Ogre::Real& x, Ogre::Real& y, Ogre::Real& z) { // get position of hull
	x = (Ogre::Real)mPosition[0];
	y = (Ogre::Real)mPosition[1];
	z = (Ogre::Real)mPosition[2];
}

/*
// get the world x,y,z position of the object
Vector3 Unit::getPosition() {
	if (!mInitialised) { // validate
		printf("FATAL ERROR: Unit::getPosition - Unit not initialised\n");
		exit(1);
	}
	return mNode->getPosition(); // get X,Y,Z position of vehicle (in metres)
}*/

void Unit::setPosition(double x, double y, double z) {
	//printf("DEBUG: Unit::setPosition(%lf, %lf, %lf)\n", x, y, z);
	if (isnan(x) || isnan(y) || isnan(z)) {
		printf("FATAL ERROR: unit[%i] is trying to set position with a nan in Unit::setPosition(%lf, %lf, %lf)\n", mUniqueIndexId, x, y, z);
		exit(1);
	}
	mPosition[0] = x;
	mPosition[1] = y;
	mPosition[2] = z;
	mNode->setPosition(mPosition[0], mPosition[1], mPosition[2]);
}
/*
// change the world x,y,z position of the object
void Unit::setPosition(Vector3& pos) {
	if (!mInitialised) { // validate
		printf("FATAL ERROR: Unit::setPosition - Unit not initialised\n");
		exit(1);
	}
	printf("DEBUG: Unit::setPosition ->setPosition\n");
	mNode->setPosition(pos); // set X,Y,Z position of vehicle (in metres)
}*/

// change x-axis rotation of vehicle. additive pitch() to fix Ogre AABB complaints
void Unit::setPitch(double xRot) {
	normaliseRadian(xRot); // make sure rotation in range 0 to 2pi
	mPitch = xRot; // set internal copy
	double addPitch = mPitch - mPrevPitch;
	mPrevPitch = mPitch;
	//printf("DEBUG: Unit::setPitch\n");
	mNode->pitch(Radian(addPitch));
}

// change y-axis rotation of vehicle. additive yaw() to fix Ogre AABB complaints
void Unit::setHeading(double yRot) {
	normaliseRadian(yRot); // make sure rotation in range 0 to 2pi
	mHeading = yRot; // set internal copy
	double addYaw = mHeading - mPrevYaw;
	mPrevYaw = mHeading;
	//printf("DEBUG: Unit::setHeading\n");
	mNode->yaw(Radian(addYaw));
}

// change z-axis rotation of vehicle. additive roll() to fix Ogre AABB complaints
void Unit::setRoll(double zRot) {
	normaliseRadian(zRot); // make sure rotation in range 0 to 2pi
	mRoll = zRot; // set internal copy
	double addRoll = mRoll - mPrevRoll;
	mPrevRoll = mRoll;
	//printf("DEBUG: Unit::setRoll\n");
	mNode->roll(Radian(addRoll));
}

void Unit::setCurrentSteerSpeed(double steerSpeed) {
	if (isnan(steerSpeed)) {
		printf("FATAL ERROR: Unit[%i]::setCurrentSteerSpeed is nan\n", mUniqueIndexId);
		exit(1);
	}
	mCurrentSteerSpeed = steerSpeed;
}

// continue reloading the MAIN gun (other weapons have lower priority and are done separatey)
bool Unit::reloadMainGun(unsigned long &elapsed) {
	if (mReloadMainGunDelay <= 0) { // load main gun (this could be hull or turret-based)
		return true;
	}
	mReloadMainGunDelay -= elapsed;
	return false; // not finished loading
}

// set the current target destination of the vehicle
void Unit::setCurrentWaypoint(Vector3& waypoint) {
	clearPrimaryTarget();	// override attack instruction
	mCurrentWaypoint = waypoint;
	mGotCurrentWaypoint = true;
}

void Unit::setFinalDestination(Vector3& destination) {
	mFinalDestination = destination;
	mGotCurrentWaypoint = true;
	mGotNavigationDestination = true;
	clearPrimaryTarget(); // stop attacking
	#ifndef _GO_GRIDS_
	mCurrentWaypoint = destination;
	mWaypointCount = 2;
	mCurrentWaypointIndex = 0;
	#endif
	recalculatePath(); // recalculate path of movement immediately
}

// get rid of all waypoints
void Unit::clearAllWaypoints() {
	mGotCurrentWaypoint = false;
	mWaypointCount = 0;
	// TODO: get rid of the whole stack here too
	// also stop driving and start idling
}

// set internal memory for vehicle we are attacking
void Unit::attackVehicle(int vehicleID) {
	clearAllWaypoints(); // cancel other movement orders (it is a primary target)
	mPrimaryTargetIndex = vehicleID;
	mGotPrimaryTarget = true;
}

// remove primary target
void Unit::clearPrimaryTarget() {
	mGotPrimaryTarget = false;
	mPrimaryTargetIndex = -1;
}

// fire the main gun at primary target
// TODO: revise this system completely; piece with main gun on it (be it turret or hull) faces target. all guns decide when to fire by themselves
bool Unit::fireMainGun() {
	if (!pSpecPtr) { // validate - does it even have a main gun?
		return false;
	}
	mFiring = true;
	mMainGunLoaded = false; // unload main gun
	mReloadMainGunDelay = pSpecPtr->mCannon.mReloadTimeUS;
	int mainGunSoundIndex = pSpecPtr->mCannon.mFireAudioIndex;
	if (mainGunSoundIndex != -1) { // play pre-defined firing sound (if it has one)
		int channelUsed = -1; // use a temporary channel because it's not looping so we can 'fire and forget'
		SoundManager::getSingleton().playSoundB(mainGunSoundIndex, mNode, &channelUsed);
	}
	if (pSpecPtr->mCannon.mHasFireAnimation) { // ask main gun if it has a firing animation
		mAnimationState = mMesh->getAnimationState(pSpecPtr->mCannon.mFireAnimation); // make sure that we have stopped any current animation
		mAnimationState->setLoop(false);
		mAnimationState->setEnabled(true);
		mAnimationState->setTimePosition(0.0);
	}
	return true;
}

// check if vehicle is currently 'selected' by user
bool Unit::isSelected() {
	return mSelected;
}

// set the vehicle as currently 'selected' by user and show graphical indicator
void Unit::setSelected(bool selected) {
	mSelected = selected;
	mNode->showBoundingBox(selected);
}

// get bounding box
AxisAlignedBox Unit::getBoundingBox() {
	return mMesh->getWorldBoundingBox(); // get the bounding box of the vehicle
}

// update animation (if one is being used)
void Unit::updateAnimations(unsigned long &elapsed) {
	double time = (double)elapsed / 1000000; // convert into Real seconds
	if (mAnimationState != NULL) {
		mAnimationState->addTime(time);
	}
}

// start idling sounds and animation
void Unit::startIdling(unsigned long &elapsed) {
	if (mStartDelay > 0) { // make sure start delay has run down
		mStartDelay = mStartDelay - elapsed;
	} else { // update idling sound
		if (pSpecPtr->mIdleSoundIndex != -1) {
			if (mIdleSoundChannel == -1) {
				SoundManager::getSingleton().playSoundB(pSpecPtr->mIdleSoundIndex, mNode, &mIdleSoundChannel);
			}
		}
		if (pSpecPtr->mHasIdleAnimation) { // run the 'idle' animation
			mAnimationState = mMesh->getAnimationState(pSpecPtr->mIdleAnimationName);
			mAnimationState->setLoop(true);
			mAnimationState->setEnabled(true);
		}
	}
}

// stop idling sounds and animation
void Unit::stopIdling() {
	if ((pSpecPtr->mIdleSoundIndex != -1) && (mIdleSoundChannel != -1)) { // stop playing idle sound
		SoundManager::getSingleton().stopSound(&mIdleSoundChannel);
		mIdleSoundChannel = -1;	// reset channel info so we know that it's not still playing
	}
}

// start driving sounds and animation
void Unit::startDriving() {
	if (pSpecPtr->mMoveSoundIndex != -1) { // continue (or start) playing driving sound (if it has one)
		if (mDriveSoundChannel == -1) {
			SoundManager::getSingleton().playSoundB(pSpecPtr->mMoveSoundIndex, mNode, &mDriveSoundChannel);
		}
	}
	if (pSpecPtr->mHasMoveAnimation) { // run the 'moving' animation
		mAnimationState = mMesh->getAnimationState(pSpecPtr->mMoveAnimationName);
		mAnimationState->setLoop(true);
		mAnimationState->setEnabled(true);
	}
	startMovementParticleEffects();
}

// stop driving sounds and animation
void Unit::stopDriving() {
	if (mAnimationState != NULL) {
		mAnimationState->setEnabled(false);
	}
	if (mDriveSoundChannel != -1) { // stop playing drive sound
		SoundManager::getSingleton().stopSound(&mDriveSoundChannel);
		mDriveSoundChannel = -1;	// reset channel info so we know it's not still playing
	}
	stopMovementParticleEffects();
}

// tell the vehicle about its nearest obstruction
void Unit::setNearestObstacle(const Vector3& obstaclePosition, double squaredDistance, double radius) {
	mHasNearestObstacle = true; // flag that we know of an obstacle that we should consider avoisding
	mNearestObstaclePosition = obstaclePosition; // retain position of nearest obstacle
	double ownRadius = mMesh->getBoundingRadius(); // get own size to subtract from distance
	mSquaredDistToObstacle = squaredDistance - (radius * radius) - (ownRadius * ownRadius); // add squared radius size of obstacle to distance calculation.
}

double Unit::getOwnSizeRadiusMeters() {
	if (!mMesh) {
		printf("FATAL ERROR: accessing radius of un-unit vehicle mesh\n");
		exit(1);
	}
	return mMesh->getBoundingRadius();
}

// get the cut-off limit for considering obstacles to avoid
double Unit::getObstacleAngleThreshold() {
	if (!pSpecPtr) {
		printf("FATAL ERROR: pSpecPtr NULL in Unit::getObstacleAngleThreshold()\n");
		exit(1);
	}
	return mFuzzyMod.getObstacleAngleThreshold();
}

void Unit::attachCrewmen(Entity *crewComm, Entity *crewLoad, Entity *crewAuxL, Entity *crewAuxR, Entity *crewDriver) {
	AnimationState* animationState = NULL;
	if (crewComm != NULL) {
		mMesh->attachObjectToBone("commander", crewComm);
		animationState = crewComm->getAnimationState("idle");
		animationState->setLoop(true);
		animationState->setEnabled(true);
	}
	if (crewLoad != NULL) {
		mMesh->attachObjectToBone("loader", crewLoad);
		animationState = crewLoad->getAnimationState("idle");
		animationState->setLoop(true);
		animationState->setEnabled(true);
	}
	if (crewAuxL != NULL) {
		mMesh->attachObjectToBone("left_gunner", crewAuxL);
		animationState = crewAuxL->getAnimationState("idle");
		animationState->setLoop(true);
		animationState->setEnabled(true);
	}
	if (crewAuxR != NULL) {
		mMesh->attachObjectToBone("right_gunner", crewAuxR);
		animationState = crewAuxR->getAnimationState("idle");
		animationState->setLoop(true);
		animationState->setEnabled(true);
	}
	if (crewDriver != NULL) {
		mMesh->attachObjectToBone("driver", crewDriver);
		animationState = crewDriver->getAnimationState("idle");
		animationState->setLoop(true);
		animationState->setEnabled(true);
	}
}

bool Unit::attachUnit(Ogre::SceneNode* crew, const int& withVehType) {
	printf("DB: Unit::attachUnit\n");
	if (!crew) {
		printf("ERROR: trying to attach NULL crew in Unit::attachUnit\n");
		return false;
	}
	char boneName[256];
	if (withVehType == 0) { // infantry
		if (mNumberOfCurrentOccupants >= pSpecPtr->mCrew.mMaxOccupants) {
			return true;
		}
		sprintf(boneName, "occupant%i", mNumberOfCurrentOccupants);
		mNumberOfCurrentOccupants++;
	} else if (withVehType == 1) {
		if (mNumberOfLightVehiclesStowed >= pSpecPtr->mCrew.mMaxLightVehicles) {
			return true;
		}
		sprintf(boneName, "lightvehicle%i", mNumberOfLightVehiclesStowed);
		mNumberOfLightVehiclesStowed++;
	} else if (withVehType == 69) {
		if (mNumberOfAuxTurretsAttached >= pSpecPtr->mCrew.mMaxAuxTurrets) {
			return true;
		}
		printf("DB: attaching aux turret\n");
		sprintf(boneName, "aux_turret%i", mNumberOfAuxTurretsAttached);
		mNumberOfAuxTurretsAttached++;
	} else {
		printf("WARNING Unknown vehicle type - can not attach Unit::attachUnit\n");
		return true;
	}
	SceneNode* pSn = NULL;
	pSn = crew->getParentSceneNode();
	if (!pSn) {
		printf("ERROR: could not get parent scene node in Unit::attachUnit\n");
		return false;
	}
	pSn->removeChild(crew);
	crew->setInheritScale(false);
	crew->resetOrientation();
	mNode->addChild(crew);
	if (!mMesh->hasSkeleton()) {
		printf("ERROR: mesh has no skeleton so can not board vehicle! Unit::attachUnit()\n");
		return false;
	}
	Ogre::SkeletonInstance* skeleton = mMesh->getSkeleton();
	if (!skeleton->hasBone(boneName)) {
		printf("ERROR: mesh's skeleton has no bone called %s in Unit::attachUnit()\n", boneName);
		return false;
	}
	Ogre::Bone* bone = skeleton->getBone(boneName);
	crew->setPosition(bone->getPosition());
	// TODO - set crewman as "attached" so he cant be moved around
	return true;
}

// stop any currently playing animations
void Unit::stopAnimations() {
	if (mAnimationState != NULL) {
		mAnimationState->setEnabled(false);
	}
}

// kill the unit; stop all anims, start dying anim
void Unit::die() {
	stopAnimations();
	mDestroyed = true;
	stopDriving();
	stopIdling();
	if (pSpecPtr->mHasDeathAnimation) { // run the 'death' animation
		mAnimationState = mMesh->getAnimationState(pSpecPtr->mDeathAnimationName);
		mAnimationState->setLoop(false);
		mAnimationState->setEnabled(true);
	}
}

bool Unit::assignTraining(char* genePoolFile, long int gen, long int batch, long int runs) {
	if ((gen != mGALogger.mGeneration) || (batch != mGALogger.mBatch)) { // check if need to load new chromosome
		if (!mFuzzyMod.mRuleBase.loadRBFromGenePoolToMatch(genePoolFile, gen, batch)) {
			printf("ERROR: could not load new rule base in Unit::assignTraining()\n");
			return false;
		}
		mGALogger.mGeneration = gen;
		mGALogger.mBatch = batch;
	}
	mGALogger.mRunsLeft = runs;
	mGALogger.mRunsCompleted = 0;
	return true;
}

bool Unit::hasFreeSpace(const int& forUnitType) {
	printf("DB: Unit::hasFreeSpace\n");
	if (!pSpecPtr) {
		printf("WARNING: vehicle has no free space for occupants as specptr = NULL Unit::hasFreeSpace()\n");
		return false;
	}
	if (forUnitType == 0) { // infantry
		printf("DB: UT INF HFS\n");
		if (mNumberOfCurrentOccupants < pSpecPtr->mCrew.mMaxOccupants) {
			return true;
		}
	} else if (forUnitType == 1) { // jeeps etc
		printf("DB: UT LV HFS\n");
		if (mNumberOfLightVehiclesStowed < pSpecPtr->mCrew.mMaxLightVehicles) {
			return true;
		}
	} else if (forUnitType == 69) { // aux turrets
		printf("DB: UT AUXT HFS\n");
		if (mNumberOfAuxTurretsAttached < pSpecPtr->mCrew.mMaxAuxTurrets) {
			return true;
		}
	}
	printf("DB: no free space - not attachable\n");
	return false;
}

