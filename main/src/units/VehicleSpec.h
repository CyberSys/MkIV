#ifndef VEHICLESPEC_H_
#define VEHICLESPEC_H_

#include "../ai/FuzzyController.h"
#define MAX_CANNONS 2
#define MAX_DUST_EFFECTS 6

#define MAX_TRAINING_RUNS 60 // start w/ a low number for a quick turn-over (see what happens to error bars) also consider if we are flicking over after low mileage then diff batches will be tested on diff parts of the environment - is this okay (probably not fair) - relies on the env/runs being fairly repetitive - maybe loading all the batches in at execution time and swapping them randomly after every mileage section would be better. if we had a 'training watcher' class it could do all this - and sporadically log how many runs of each batch had completed (in case of crash/interruption) e.g: "TIMESTAMP RUNS: 11 11 12 10 11 10 9 10 12 12 10 9 11 7 11 10 18 11 12"

struct AISpec {
	AISpec();
	void resetSpecs();
	
	char mDGARulesFilename[256];
	char mFitnessLogFilename[256];
	
	// NOTE: not used if mIsInTraining
	char mFuzzySetsFile[256]; // file to use to define fuzzy nav rules (if there is only one)
	char mFuzzyRulesFile[256]; // file to use to define fuzzy nav rules (if there is only one)
	
	unsigned long mCountRunsCompleted;
	double mTrainingWPX[3];
	double mTrainingWPZ[3];
	int mCurrentGenerationNumber;
	int mCurrentBatchNumber;
	int mTrainingWPS;
	bool mIsInTraining;
};

// small struct defining vehicle performance characteristics
struct Performance {
	Performance(); // default constructor
	void resetSpecs();
	
	void setFullTurnTimeS(float ftt_s); // derive radians / microsecond rate
	void setMaxRoadKPH(float maxroad_kph);
	void setMaxOffRoadKPH(float maxoffroad_kph);
	void setMaxWaterKPH(float speed);
	void setMaxSlopeDeg(float slope_deg);
	float getMaxAccelerationS(float currentTerrainMaxSpeed); // get max change in speed per second (where speed is m/us)
	float mTopRoadMPUS; // top road speed in m/us
	float mTopCrossMPUS; // top cross-country speed in m/us
	float mTopWaterMPUS;
	float mTopSteerRadUS; // top angular speed in radians/us
	float mTopRoadKPH; // top road speed in km/h
	float mTopCrossKPH; // top cross-country speed in km/h
	float mTopWaterKPH;
	float mFullTurnTimeS; // time to turn 360 degrees in seconds
	float mMaxSlopeDeg; // max slope can climb in degrees
	float mMaxSlopeRad; // in radians
	float mZeroToTopSpeedS; // seconds it takes to get from 0-top speed (max acceleration)
	bool mFloats;
	double mWaterLineOffsetM;
};

struct Armour {
	Armour();
	void resetSpecs();
	
	float mFront_mm;
	float mFrontSlope_rad;
	float mRear_mm;
	float mRearSlope_rad;
	float mSide_mm;
	float mSideSlope_rad;
	float mTop_mm;
	float mTopSlope_rad;
};

// small sub-section of Vehicle components (bits just on the hull)
struct Hull {
	Hull(); // default constructor
	void resetSpecs();
	
	Armour mArmour;
	float length_m;
	float width_m;
};

// struct containing special effects bits and pieces
struct SpecialEffects {
	SpecialEffects(); // default constructor
	void resetSpecs();
	
	char mDustEmitterBoneName[MAX_DUST_EFFECTS][64]; // names of bones that designate dust emission points
	char mExhaustEmitterBoneName[2][128];
	char mWakeEmitterBoneName[2][128];
	int mWakeEmitterCount;
	int mDustEmitterCount; // count of mDustEmitterBoneNames
	int mExhaustEmitterCount;
};

// struct defining a type of main gun
struct Cannon {
	Cannon(); // set default values
	void resetSpecs();
	
	char mFireAudio[256];
	char mFireAnimation[128];
	char mMuzzleBone[128];
	char mPivotBone[128];
	float mPenetration[7];
	float mAccuracy[7];
	long mReloadTimeUS;
	int mFireAudioIndex;
	bool mHasFireAnimation;
	bool mHasFireAudio;
	bool mHasPivotBone;
	bool mHasMuzzleBone;
};

// small struct for each turret's specific bits and pieces
struct Turret {
	Turret(); // default constructor
	void resetSpecs();
	
	Armour mArmour;
	char mTraverseBone[128];
	char mTraverseAudio[256];
	float mTraverseRadS;
};

struct Crew {
		Crew();
		int mMaxOccupants;
		int mMaxLightVehicles;
		int mMaxAuxTurrets;
};

// class specifying the characteristics of a particular type of vehicle
struct VehicleSpec {
	VehicleSpec(); // default constructor
	void resetSpecs(); // reset all specs to default (mostly zero) values
	bool loadFromXMLfile(char* specFile); // load from specific file
	Performance mPerformance; // vehicle performance characteristics
	Hull mHull; // size and armour of hull
	SpecialEffects mSpecialEffects; // special textures and marker bone names
	Cannon mCannon;
	Turret mTurret;
	AISpec mAISpec;
	Crew mCrew;
	char mFilename[256]; // the name of this file
	char mDesignation[128]; // official name of the vehicle
	char mNickName[128]; // informal or secondary name of the vehicle
	char mDescription[2056]; // lengthy description of vehicle
	char mMeshFile[256]; // name of mesh file
	char mIdleAnimationName[128]; // name of the animation to use when idling
	char mMoveAnimationName[128]; // name of the animation to use when driving
	char mDeathAnimationName[128]; // name of the animation to use when driving
	char mIdleSoundFile[256]; // name of the sound to use when driving
	char mMoveSoundFile[256]; // name of the sound to use when driving
	char mRampBoneName[128];
	int mIdleSoundIndex; // index of sound file used when driving
	int mMoveSoundIndex; // index of sound file used when driving
	bool mAutoScale; // whether or not to auto scale the mesh
	bool mHasIdleAnimation; // whether or not there is an idling animation to play
	bool mHasMoveAnimation; // whether or not there is a driving animation to play
	bool mHasDeathAnimation; // whether or not there is a driving animation to play
	bool mHasIdleSound; // whether or not there is an idling sound to play
	bool mHasMoveSound; // whether or not there is a driving sound to play
	bool mHasMainTurret;
	bool mIsInfantry;
	bool mIsLightVehicle;
	bool mIsAmphibian;
	bool mIsLandingCraft;
	bool mIsAuxTurret;
	bool mHasBeenLoadedFromFile;
};

#endif /*VEHICLESPEC_H_*/
