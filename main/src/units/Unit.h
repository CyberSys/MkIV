#ifndef UNIT_H_
#define UNIT_H_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
#include "VehicleSpec.h"
#include "Amphibian.h"
#include "LandingCraft.h"
#include "GeneticLogger.h"
#include "../scenery/TerrainHandler.h"
#include "../globals.h"
using namespace std;
using namespace Ogre;
#define STAGGERED_UPDATE_GROUPS 33
#define MAX_STEER_FRICTION_FACTOR 0.5 // at full turn max speed is 50%
//#define ENABLE_BRAKE_DAMPENING // TODO: enable this after experiments finished

// an abstract unit (anything that can be selected and controlled is a unit)
struct Unit {
	Unit(); // default constructor

	virtual bool balance() = 0; // balance the vehicle on the terrain
	virtual void performRotation() = 0; // auto orient
	virtual bool update(unsigned long &elapsed) = 0; // update position etc
	virtual Vector3 getWaypoint(int index) = 0;
	virtual bool recalculatePath() = 0; // recalculate path to final destination
	virtual void startMovementParticleEffects() = 0;
	virtual void stopMovementParticleEffects() = 0;

	bool init(TerrainHandler *terrHdlr, VehicleSpec* defptr, Entity* mesh, SceneNode* node, Vector3& pos, double boundRad, int force); // set up vehicle
	AxisAlignedBox getBoundingBox();

	SceneNode* getNodePtr();
	Vector3	getCurrentWaypoint();
	Vector3 getGunWorldOrientation(); // get the world-relative orientation of the main gun as a 3d vector
	Vector3 getMainGunMuzzleWorldPosition(); // get world position of muzzle of main gun
	void getPosition(double& x, double& y, double& z); // get position of hull
	void getPosition(Ogre::Real& x, Ogre::Real& y, Ogre::Real& z); // get position of hull

	double getCurrentDriveSpeedKPH();
	double getOwnSizeRadiusMeters();
	double getObstacleAngleThreshold(); // get the cut-off limit for considering obstacles to avoid

	bool driveToCurrentWaypoint(unsigned long &elapsed);
	bool rotateHullToFace(Vector3 &target, unsigned long &elapsed);
	bool fireMainGun();
	bool reloadMainGun(unsigned long &elapsed); // primary task of reloading gun. May involve 2 crew members
	bool addWakeController(ParticleSystem* systemPtr);
	bool addDustController(ParticleSystem* systemPtr);
	bool rotateTurretToFace(Vector3 &target, unsigned long &elapsed);
	bool aimCannon(int cannonIndex, const Vector3 &target, unsigned long &elapsed);
	bool isSelected();
	bool attachUnit(Ogre::SceneNode* crew, const int& withVehType);

	void attachCrewmen(Entity *crewComm, Entity *crewLoad, Entity *crewAuxL, Entity *crewAuxR, Entity *crewDriver);
	void attackVehicle(int vehicleID);
	void clearAllWaypoints();
	void clearCurrentWaypoint();
	void clearPrimaryTarget();
	void showSelected(bool show); // show box around vehicle

	void setCurrentWaypoint(Vector3& waypoint);
	void setFinalDestination(Vector3& destination);

	double getCurrentSteerSpeed();

	void setPosition(double x, double y, double z);
	//void setPosition(Vector3& position);
	void setPitch(double xRot);
	void setHeading(double yRot);
	void setRoll(double zRot);
	void setCurrentSteerSpeed(double steerSpeed);

	void startIdling(unsigned long &elapsed); // start idling sounds and animation
	void stopIdling(); // stop idling sounds and animation
	void startDriving(); // start driving sounds and animation
	void stopDriving(); // stop driving sounds and animation
	void hatchOpen(unsigned long &elapsed);
	void updateAnimations(unsigned long &elapsed); // continue updating animations
	void stopAnimations(); // stop any currently playing animations

	void setNearestObstacle(const Vector3& obstaclePosition, double squaredDistance, double radius);
	void setSelected(bool selected);
	void attack(unsigned long& elapsed);
	void die(); // kill the unit; stop all anims, start dying anim

	void limitSpeedByClimbPhysics();
	void limitSpeedBySteeringFriction(const unsigned long& elapsed);
	void limitSteeringByVehiclePhysics(const unsigned long& elapsed);
	void limitAccelerationByVehiclePhysics(const unsigned long& elapsed);

	bool assignTraining(char* genePoolFile, long int gen, long int batch, long int runs);
	bool hasFreeSpace(const int& forUnitType);

	TerrainHandler* pTerrainHandler; // pointer to external terrain handler
	Entity* mMesh; // main mesh
	AnimationState* mAnimationState; // The current animation state of the main object
	VehicleSpec* pSpecPtr; // vehicle's specification file
	Amphibian mAmphibian;
	LandingCraft mLandingCraft;
	GeneticLogger mGALogger;
	FuzzyController mFuzzyMod; // fuzzy logic based navigation aid
	Vector3 mFinalDestination;
	Vector3 mPrimaryTarget; // world x,y,z position of main target in meters from world origin

	double mMaxAllowedCurrentTerrainSpeed;

	double mObstacleRadiusSize; // obstacle properties
	double mSquaredDistToNavTarget; // obstacle properties
	double mAngleToNavTarget; // obstacle properties
	double mAngleToObstacle; // obstacle properties
	double mMainGunElevation;
	double mCurrentDriveSpeed; // local forward speed of vehicle in meters per microsecond
	double mPreviousDriveSpeed; // speed at last update

	double mPosition[3]; // internal copy of position (because I don't trust Ogre's)
	double mPitch; // local hull pitch in radians in relation to terrain
	double mHeading; // local hull facing in radians in relation to terrain
	double mRoll; // local hull roll in radians in relation to terrain
	double mPrevPitch;
	double mPrevYaw; // for additive yaw() to fix aabb problem
	double mPrevRoll;

	double mTurretOrientation; // local turret facing in radians in relation to hull (radians from zero (forward) facing)
	double mSquaredDistToObstacle; // obstacle properties

	unsigned long mRandomNumber; // random number used for updating at intervals (for staggered mass optimisation)
	long mSmokeInVehicleDelay; // time takes til smoke clears from cabin
	long mAimMainCannonDelay; // time it will take to aim the main cannon
	long mReloadMainGunDelay; // time until main gun is reloaded in single man time
	long mStartDelay; // delay to start first animation and audio to make it look more natural

	int mUniqueIndexId;

	int mCollisions; // record number of times we have hit an obstacle
	int mPrimaryTargetIndex; // if type is 1-3 then this is its hash index/id
	int mForce;	// the force that the vehicle belongs to
	int mDriveSoundChannel; // index of unique sound channel to use to play the looping driving sound
	int mIdleSoundChannel; // index of sound channel to use to play the looping idling sound
	int mDustSystemCount;
	int mWakeSystemCount;
	int mWaypointCount;
	int mCurrentWaypointIndex;
	int mCurrentTrainingWPIndex;
	int mNumberOfCurrentOccupants;
	int mNumberOfLightVehiclesStowed;
	int mNumberOfAuxTurretsAttached;
	int mIndexOfUnitBeingRidden;

	#ifndef MAX_WAYPOINTS
	#define MAX_WAYPOINTS 1
	#endif
	int mXWaypoint[MAX_WAYPOINTS]; // list of waypoints (x index)
	int mYWaypoint[MAX_WAYPOINTS]; // list of waypoints (y index)

	bool mGotNavigationDestination;
	bool mGotCurrentWaypoint; // if we have a current waypoint
	bool mGotPrimaryTarget; // if we have a primary target
	bool mMainGunLoaded; // if the main gun is ready to fire or not
	bool mFiring; // if vehicle has just fired (this instant)
	bool mWasTurning;
	bool mDestroyed;
	bool mCanSeeTarget;
	bool mImmobilised;
	bool mIsRidingOtherUnit;

	protected:
	SceneNode* mNode; // node the model is attached to (protected so that it isn't accessed by a naughty function and setPosition or getPosition used that by-passes our own mPosition[3])
	ParticleSystem* mDustSystem[MAX_DUST_EFFECTS]; // particle systems for dust emission
	ParticleSystem* mWakeSystem[2];
	Vector3 mCurrentWaypoint; // world x,y,z position of current waypoint in meters from world origin
	Vector3 mNearestObstaclePosition; // world x,y,z position of nearest obstacle
	double mCurrentSteerSpeed; // local angular speed of vehicle in radians per microsecond
	bool mHasNearestObstacle; // if there is an obstruction close to the vehicle's path of movement
	bool mInitialised; // if vehicle has been init()
	bool mSelected; // if we are selected by the user
};

#endif /*UNIT_H_*/

