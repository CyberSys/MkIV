#include "LandingCraft.h"

LandingCraft::LandingCraft() {
	strcpy(mRampBoneName, "");
	mCurrentRampAngle = 0.0;
	mCurrentRampSpeed = 0.00000010;
	mMaxRampAngle = 1.0;
	mIsBeached = false;
	mRampFullyLowered = false;
}

bool LandingCraft::lowerRamp(unsigned long& elapsed, Entity* mMesh, double& roll_rad) {
	if (!mMesh) {
		printf("ERROR: mesh is null in LandingCraft::lowerRamp\n");
		return false;
	}
	if (!mMesh->hasSkeleton()) { // make sure that the mesh has a skeleton
		printf("ERROR: andingCraft::lowerRamp the mesh does not have a skeleton\n");
		return false;
	}
	SkeletonInstance* skeleton = NULL;
	skeleton = mMesh->getSkeleton(); // get a pointer to the hull's skeleton
	Bone* bone = NULL;
	bone = skeleton->getBone(mRampBoneName);
	if (!bone) {
		printf("ERROR: failed to find bone named %s for controlling ramp in LandingCraft::lowerRamp\n", mRampBoneName);
		return false;
	}
	bone->setManuallyControlled(true);
	bone->resetOrientation();
	bone->roll(Radian(1.57));
	if (mCurrentRampAngle >= mMaxRampAngle) {
		bone->yaw(Radian(-mMaxRampAngle));
		mRampFullyLowered = true;
	} else {
		double pitchAmountThisFrame = (double)elapsed * mCurrentRampSpeed;
		mCurrentRampAngle += pitchAmountThisFrame;
		bone->yaw(Radian(-mCurrentRampAngle));
		mCurrentRampSpeed += (double)elapsed * 0.000000000010;
	}
	return true;
}

