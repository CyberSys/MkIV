#include "VehicleSpec.h"
#include "../globals.h"
#include "../usefulFuncs.h"
#include <string.h>
#include <math.h> // for PI constants
#include <fstream>
using namespace std;

AISpec::AISpec() {
	resetSpecs();
}

void AISpec::resetSpecs() {
	strcpy(mFitnessLogFilename, "");
	strcpy(mFuzzySetsFile, "ai/default-fuzzy-sets.txt"); // file to use to define fuzzy nav sets
	strcpy(mFuzzyRulesFile, "ai/default-fuzzy-rules.txt"); // file to use to define fuzzy nav rules
	mCountRunsCompleted = 0;
	mCurrentGenerationNumber = -1;
	mCurrentBatchNumber = -1;
	mTrainingWPS = 0;
	mIsInTraining = false;
}

Armour::Armour() {
	resetSpecs();
}

void Armour::resetSpecs() {
	mFront_mm = 1.0f;
	mFrontSlope_rad = 1.57f;
	mRear_mm = 1.0f;
	mRearSlope_rad = 1.57f;
	mSide_mm = 1.0f;
	mSideSlope_rad =1.57f;
	mTop_mm = 1.0f;
	mTopSlope_rad = 0.0f;
}

// default constructor
Performance::Performance() {
	resetSpecs();
}

void Performance::resetSpecs() {
	setFullTurnTimeS(30.0f); // set both turning speed measures
	setMaxRoadKPH(0.0f);
	setMaxOffRoadKPH(0.0f);
	setMaxWaterKPH(0.0f);
	setMaxSlopeDeg(15.0f);
	mZeroToTopSpeedS = 10.0f;
	mFloats = false;
	mWaterLineOffsetM = 0.0;
}

// derive radians / microsecond rate
void Performance::setFullTurnTimeS(float ftt_s) {
	mFullTurnTimeS = ftt_s;
	mTopSteerRadUS =  ((2.0f * M_PI) / mFullTurnTimeS) / 1000000.0f; // set speed defaults
}

void Performance::setMaxRoadKPH(float maxroad_kph) {
	mTopRoadKPH = maxroad_kph;
	mTopRoadMPUS = mTopRoadKPH / 3600000.0f; // set speed defaults
}

void Performance::setMaxOffRoadKPH(float maxoffroad_kph) {
	mTopCrossKPH = maxoffroad_kph; // top cross-country speed in km/h
	mTopCrossMPUS = mTopCrossKPH / 3600000.0f; // set speed defaults
}

void Performance::setMaxWaterKPH(float speed) {
	mTopWaterKPH = speed; // top water speed in km/h
	mTopWaterMPUS = mTopWaterKPH / 3600000.0f; // set speed defaults
}

void Performance::setMaxSlopeDeg(float slope_deg) {
	mMaxSlopeDeg = slope_deg;
	mMaxSlopeRad = (slope_deg / 360.0f) * (2 * M_PI); // normalise & convert
}

float Performance::getMaxAccelerationS(float currentTerrainMaxSpeed) { // get max change in speed per second (where speed is m/us)
	float maxAccel = (currentTerrainMaxSpeed * 1000000.0f) / mZeroToTopSpeedS;
	return maxAccel;
}

// default constructor
Hull::Hull() {
	resetSpecs();
}

void Hull::resetSpecs() {
	mArmour.resetSpecs();
	length_m = 0.1f;
	width_m = 0.1f;
}

// default constructor
SpecialEffects::SpecialEffects() {
	resetSpecs();
}

void SpecialEffects::resetSpecs() {
	for (int i = 0; i < MAX_DUST_EFFECTS; i++) {
		strcpy(mDustEmitterBoneName[i], ""); // reset string
		strcpy(mWakeEmitterBoneName[i], "");
	}
	mDustEmitterCount = mWakeEmitterCount = mExhaustEmitterCount = 0;
}

// default constructor
Cannon::Cannon() {
	resetSpecs();
}

void Cannon::resetSpecs() {
	strcpy(mFireAudio, "");
	strcpy(mFireAnimation, "");
	strcpy(mMuzzleBone, "");
	strcpy(mPivotBone, "");
	for (int i = 0; i < 7; i++) {
		mPenetration[i] = 1.0f;
	}
	mAccuracy[0] = 70.0;
	mAccuracy[1] = 60.0;
	mAccuracy[2] = 48.0;
	mAccuracy[3] = 25.0;
	mAccuracy[4] = 10.0;
	mAccuracy[5] = 03.0;
	mAccuracy[6] = 01.0;
	mReloadTimeUS = 5000000; // 5 secs default
	mFireAudioIndex = -1;
	mHasFireAnimation = false;
	mHasFireAudio = false;
	mHasPivotBone = false;
	mHasMuzzleBone = false;
}

// default constructor
Turret::Turret() {
	resetSpecs();
}

void Turret::resetSpecs() {
	mArmour.resetSpecs();
	strcpy(mTraverseBone, "");
	strcpy(mTraverseAudio, "");
	mTraverseRadS = 1.0f;
}

Crew::Crew() {
		mMaxOccupants = 0;
		mMaxLightVehicles = 0;
		mMaxAuxTurrets = 0;
}

// init default values (in case something is missing from .spec)
VehicleSpec::VehicleSpec() {
	resetSpecs();
}

void VehicleSpec::resetSpecs() {
	mPerformance.resetSpecs();
	mHull.resetSpecs();
	mSpecialEffects.resetSpecs();
	mCannon.resetSpecs();
	mTurret.resetSpecs();
	mAISpec.resetSpecs();
	strcpy(mFilename, ""); // set regular attributes to defaults
	strcpy(mDesignation, "");
	strcpy(mDescription, "Empty description");
	strcpy(mNickName, "");
	strcpy(mMeshFile, "");
	strcpy(mIdleAnimationName, "idle"); // animation details
	strcpy(mMoveAnimationName, "move"); // animation details
	strcpy(mDeathAnimationName, "death"); // anim details
	strcpy(mIdleSoundFile,""); // audio details
	strcpy(mMoveSoundFile,""); // audio details
	strcpy(mRampBoneName, "");
	mIdleSoundIndex = -1;
	mMoveSoundIndex = -1;
	mAutoScale = false;
	mHasDeathAnimation = false;
	mHasIdleAnimation = false; // animation details
	mHasMoveAnimation = false; // animation details
	mHasIdleSound = false;
	mHasMoveSound = false;
	mHasMainTurret = false;
	mIsInfantry = false;
	mIsLightVehicle = false;
	mIsAmphibian = false;
	mIsLandingCraft = false;
	mIsAuxTurret = false;
	mHasBeenLoadedFromFile = false;
}

// load from specific file
bool VehicleSpec::loadFromXMLfile(char* specFile) {
	printf("Loading %s from spec file...\n", specFile);
	char specificationPath[256];
	strcpy(specificationPath, "data/vehicles/specifications/");
	strcat(specificationPath, specFile); // add default path to file name
	ifstream file;
	file.open(specificationPath); // open the file
	if (!file) { // validate file
		printf("Error: Could not open spec file %s!\n", specFile);
		return false;
	}
	strcpy(mFilename, specFile); // record filename (for comparative id of this def)
	char key[256];
	char next[256];
	char specDetails[256];
	bool inDescriptionSection = false;
	while (file.getline(specDetails, 256)) { // parse file
		strcpy(key,"");	// reset these strings.
		strcpy(next,""); // reset these strings.
		sscanf(specDetails, "%s %s", key, next);
		if (inDescriptionSection) {
			if (strcmp(key, "DESCRIPTION_END") == 0) {
				inDescriptionSection = false;
			} else {
				strcat(mDescription, specDetails);
				strcat(mDescription, "\n");
			}
		}
		if (key[0] == '#') { // ignore comments
		} else if (strcmp(specDetails, "") == 0) { // ignore blank lines
		} else if (strcmp(key, "DESIGNATION") == 0) {
			stripString(specDetails, strlen(key) + 1, mDesignation, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "DESCRIPTION_START") == 0) {
			strcpy(mDescription, ""); // reset in case of being over-written
			inDescriptionSection = true;
		} else if (strcmp(key, "NICKNAME") == 0) {
			stripString(specDetails, strlen(key) + 1, mNickName, 128); // grab all of string after key (not just next token)
		} else if (strcmp(key, "MESHFILE") == 0) {
			strcpy(mMeshFile, next);
		} else if (strcmp(key, "ANIM_IDLE") == 0) {
			strcpy(mIdleAnimationName, next);
			mHasIdleAnimation = true;
		} else if (strcmp(key, "ANIM_MOVE") == 0) {
			strcpy(mMoveAnimationName, next);
			mHasMoveAnimation = true;
		} else if (strcmp(key, "ANIM_DEATH") == 0) {
			strcpy(mDeathAnimationName, next);
			mHasDeathAnimation = true;
		} else if (strcmp(key, "SOUND_IDLE") == 0) {
			strcpy(mIdleSoundFile, next);
			mHasIdleSound = true;
		} else if (strcmp(key, "SOUND_MOVE") == 0) {
			strcpy(mMoveSoundFile, next);
			mHasMoveSound = true;
		} else if (strcmp(key, "AUTO_SCALE") == 0) {
			int as = 0;
			sscanf(next, "%i", &as);
			if (as == 1) {
				mAutoScale = true;
			}
		} else if (strcmp(key, "FULL_TURN_TIME_S") == 0) {
			float ftt_s = 0.0f;
			sscanf(next, "%f", &ftt_s);
			mPerformance.setFullTurnTimeS(ftt_s);
		} else if (strcmp(key, "FLOATS") == 0) {
			int f = 0;
			sscanf(next, "%i", &f);
			if (f == 1) {
				mPerformance.mFloats = true;
			}
		} else if (strcmp(key, "MAX_OCCUPANTS") == 0) {
			sscanf(next, "%i", &mCrew.mMaxOccupants);
		} else if (strcmp(key, "MAX_LIGHT_VEHICLES_STOWED") == 0) {
			sscanf(next, "%i", &mCrew.mMaxLightVehicles);
		} else if (strcmp(key, "MAX_AUX_TURRETS") == 0) {
			sscanf(next, "%i", &mCrew.mMaxAuxTurrets);
		} else if (strcmp(key, "ZERO_TO_TOP_SPEED_S") == 0) {
			sscanf(next, "%f", &mPerformance.mZeroToTopSpeedS);
		} else if (strcmp(key, "MAX_ROAD_KPH") == 0) {
			float maxroad_kph = 0.0f;
			sscanf(next, "%f", &maxroad_kph);
			mPerformance.setMaxRoadKPH(maxroad_kph);
		} else if (strcmp(key, "MAX_OFFROAD_KPH") == 0) {
			float maxoffroad_kph = 0.0f;
			sscanf(next, "%f", &maxoffroad_kph);
			mPerformance.setMaxOffRoadKPH(maxoffroad_kph);
		} else if (strcmp(key, "MAX_WATER_KPH") == 0) {
			float speed = 0.0f;
			sscanf(next, "%f", &speed);
			mPerformance.setMaxWaterKPH(speed);
		} else if (strcmp(key, "HULL_LENGTH_M") == 0) {
			sscanf(next, "%f", &mHull.length_m);
		} else if (strcmp(key, "HULL_WIDTH_M") == 0) {
			sscanf(next, "%f", &mHull.width_m);
		} else if (strcmp(key, "HULL_ARMOUR") == 0) {
			sscanf(specDetails, "HULL_ARMOUR %f@%f %f@%f %f@%f %f@%f", &mHull.mArmour.mFront_mm, &mHull.mArmour.mFrontSlope_rad, &mHull.mArmour.mRear_mm, &mHull.mArmour.mRearSlope_rad, &mHull.mArmour.mSide_mm, &mHull.mArmour.mSideSlope_rad, &mHull.mArmour.mTop_mm, &mHull.mArmour.mTopSlope_rad);
			mHull.mArmour.mFrontSlope_rad = (mHull.mArmour.mFrontSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
			mHull.mArmour.mRearSlope_rad = (mHull.mArmour.mRearSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
			mHull.mArmour.mSideSlope_rad = (mHull.mArmour.mSideSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
			mHull.mArmour.mTopSlope_rad = (mHull.mArmour.mTopSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
		} else if (strcmp(key, "MAX_SLOPE_DEG") == 0) {
			float maxslope_deg = 0.0f;
			sscanf(next, "%f", &maxslope_deg);
			mPerformance.setMaxSlopeDeg(maxslope_deg);
		} else if (strcmp(key, "DUST_EMITTER") == 0) {
			if (mSpecialEffects.mDustEmitterCount >= MAX_DUST_EFFECTS) {
				printf("ERROR: too many dust effects given in spec file %s\n", specFile);
			} else {
				strcpy(mSpecialEffects.mDustEmitterBoneName[mSpecialEffects.mDustEmitterCount], next);
				printf(" adding dust emitter %i to spec\n", mSpecialEffects.mDustEmitterCount);
				mSpecialEffects.mDustEmitterCount++;
			}
		} else if (strcmp(key, "EXHAUST_EMITTER") == 0) {
			if (mSpecialEffects.mExhaustEmitterCount >= 2) {
				printf("ERROR: too many exhaust effects given in spec file %s\n", specFile);
			} else {
				strcpy(mSpecialEffects.mExhaustEmitterBoneName[mSpecialEffects.mExhaustEmitterCount], next);
				mSpecialEffects.mExhaustEmitterCount++;
			}
		} else if (strcmp(key, "WAKE_EMITTER") == 0) {
			if (mSpecialEffects.mWakeEmitterCount >= 2) {
				printf("ERROR: too many wake effects given in spec file %s\n", specFile);
			} else {
				strcpy(mSpecialEffects.mWakeEmitterBoneName[mSpecialEffects.mWakeEmitterCount], next);
				mSpecialEffects.mWakeEmitterCount++;
			}
		} else if (strcmp(key, "CANNON_SOUND_FIRE") == 0) {
			strcpy(mCannon.mFireAudio, next);
			mCannon.mHasFireAudio = true;
		} else if (strcmp(key, "CANNON_ANIM_FIRE") == 0) {
			strcpy(mCannon.mFireAnimation, next);
			mCannon.mHasFireAnimation = true;
		} else if (strcmp(key, "CANNON_BONE_MUZZLE") == 0) {
			strcpy(mCannon.mMuzzleBone, next);
			mCannon.mHasMuzzleBone = true;
		} else if (strcmp(key, "CANNON_BONE_PIVOT") == 0) {
			strcpy(mCannon.mPivotBone, next);
			mCannon.mHasPivotBone = true;
		} else if (strcmp(key, "CANNON_RELOAD_TIME_S") == 0) {
			float seconds = 0;
			sscanf(next, "%f", &seconds);
			mCannon.mReloadTimeUS = (unsigned long)(seconds * 1000000.0f);
		} else if (strcmp(key, "PENETRATION_mm") == 0) {
			sscanf(specDetails, "PENETRATION_mm %f %f %f %f %f %f %f", &mCannon.mPenetration[0], &mCannon.mPenetration[1], &mCannon.mPenetration[2], &mCannon.mPenetration[3], &mCannon.mPenetration[4], &mCannon.mPenetration[5], &mCannon.mPenetration[6]);
		} else if (strcmp(key, "ACCURACY") == 0) {
			sscanf(specDetails, "ACCURACY %f %f %f %f %f %f %f", &mCannon.mAccuracy[0], &mCannon.mAccuracy[1], &mCannon.mAccuracy[2], &mCannon.mAccuracy[3], &mCannon.mAccuracy[4], &mCannon.mAccuracy[5], &mCannon.mAccuracy[6]);
		} else if (strcmp(key, "TURRET_TRAVERSE_BONE") == 0) {
			strcpy(mTurret.mTraverseBone, next);
			mHasMainTurret = true;
		} else if (strcmp(key, "TURRET_TIME_TO_TRAVERSE_360_S") == 0) {
			float seconds = 0;
			sscanf(next, "%f", &seconds);
			mTurret.mTraverseRadS = (2.0f * M_PI) / seconds; // convert to radians/second
			mHasMainTurret = true;
		} else if (strcmp(key, "TURRET_ARMOUR") == 0) {
			sscanf(specDetails, "TURRET_ARMOUR %f@%f %f@%f %f@%f %f@%f", &mTurret.mArmour.mFront_mm, &mTurret.mArmour.mFrontSlope_rad, &mTurret.mArmour.mRear_mm, &mTurret.mArmour.mRearSlope_rad, &mTurret.mArmour.mSide_mm, &mTurret.mArmour.mSideSlope_rad, &mTurret.mArmour.mTop_mm, &mTurret.mArmour.mTopSlope_rad);
			mTurret.mArmour.mFrontSlope_rad = (mTurret.mArmour.mFrontSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
			mTurret.mArmour.mRearSlope_rad = (mTurret.mArmour.mRearSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
			mTurret.mArmour.mSideSlope_rad = (mTurret.mArmour.mSideSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
			mTurret.mArmour.mTopSlope_rad = (mTurret.mArmour.mTopSlope_rad / 360.0f) * (2 * M_PI); // normalise & convert
		} else if (strcmp(key, "UNIT_TYPE") == 0) {
			if (strcmp(next, "INFANTRY") == 0) {
				mIsInfantry = true;
			}
			if (strcmp(next, "LIGHT_VEHICLE") == 0) {
				mIsLightVehicle = true;
			}
			if (strcmp(next, "AMPHIBIAN") == 0) {
				mIsAmphibian = true;
			}
			if (strcmp(next, "LANDINGCRAFT") == 0) {
				mIsLandingCraft = true;
			}
			if (strcmp(next, "AUX_TURRET") == 0) {
				mIsLandingCraft = true;
			}
		} else if (strcmp(key, "RAMP_BONE") == 0) {
			strcpy(mRampBoneName, next);
		} else if (strcmp(key, "WATERLINE_OFFSET_M") == 0) {
			sscanf(next, "%lf", &mPerformance.mWaterLineOffsetM);
			printf("DB: found water os %lf\n", mPerformance.mWaterLineOffsetM);
		} else if (strcmp(key, "FUZZY_SETS") == 0) {
			strcpy(mAISpec.mFuzzySetsFile, next);
		} else if (strcmp(key, "FUZZY_RULES") == 0) {
			strcpy(mAISpec.mFuzzyRulesFile, next);
		} else if (strcmp(key, "LOG_FITNESS_TO_FILE") == 0) {
			strcpy(mAISpec.mFitnessLogFilename, next);
			mAISpec.mIsInTraining = true;
		} else if (strcmp(key, "TRAINING_WPS") == 0) {
			mAISpec.mTrainingWPS = 3;
			sscanf(specDetails, "TRAINING_WPS {%lf,%lf} {%lf,%lf} {%lf,%lf}", &mAISpec.mTrainingWPX[0], &mAISpec.mTrainingWPZ[0], &mAISpec.mTrainingWPX[1], &mAISpec.mTrainingWPZ[1], &mAISpec.mTrainingWPX[2], &mAISpec.mTrainingWPZ[2]);
			printf("* using training wps {%lf,%lf} {%lf,%lf} {%lf,%lf}\n", mAISpec.mTrainingWPX[0], mAISpec.mTrainingWPZ[0], mAISpec.mTrainingWPX[1], mAISpec.mTrainingWPZ[1], mAISpec.mTrainingWPX[2], mAISpec.mTrainingWPZ[2]);
		}
	}
	if (g.mUseGlobalFuzzyRules) {
		strcpy(mAISpec.mFuzzyRulesFile, g.mFuzzyRulesFile);
	}
	file.close(); // close the file
	mHasBeenLoadedFromFile = true;
	
	return true;
}

