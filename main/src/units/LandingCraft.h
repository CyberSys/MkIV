#ifndef _LANDINGCRAFT_
#define _LANDINGCRAFT_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
using namespace Ogre;

struct LandingCraft {
	char mRampBoneName[128];
	double mCurrentRampAngle;
	double mCurrentRampSpeed;
	double mMaxRampAngle;
	bool mIsBeached;
	bool mRampFullyLowered;
	LandingCraft();
	bool lowerRamp(unsigned long& elapsed, Entity* mMesh, double& roll_rad);
};

#endif

