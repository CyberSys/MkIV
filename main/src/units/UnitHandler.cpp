#include "UnitHandler.h"
#include "Infantry.h"
#include "../globals.h"
#include "../usefulFuncs.h"
#include "../audio/AudioHandler.h"
#include <fstream>
#include <string.h>
#include <math.h>
using namespace std;

#define DISABLE_NEAR_VEH_THRESH
#define OBST_DIST_MAX 2000.0

UnitHandler::UnitHandler() {
	mVehicleDefHashEnd = 0;
}

// initialise the unit handler
bool UnitHandler::init(SceneManager *sceneMgr, TerrainHandler* terrainMgr, RaySceneQuery* rsq) {
	printf("Initialising Unit Handler...\n");
	if (!sceneMgr) {
		printf("ERROR: scene manager is NULL in UnitHandler::init()\n");
		return false;
	}
	if (!terrainMgr) {
		printf("ERROR: terrain manager is NULL in UnitHandler::init()\n");
		return false;
	}
	if (!rsq) {
		printf("ERROR: RaySceneQuery parameter is NULL in UnitHandler::init()\n");
		return false;
	}
	srand(time(NULL));

	mTankEntityCount = 0;
	pSceneManager = sceneMgr;
	pTerrainHandler = terrainMgr;
	pRSQ = rsq;
	mSelectedNode = NULL;
	mTargetedNode = NULL;
	// reset vehicle hash
	for (int i = 0; i < VEHICLE_HASH_MAX; i++) {
		mUnitHash[i] = NULL;
	}
	// reset projectile hash
	for (int i = 0; i < PROJECTILE_HASH_MAX; i++) {
		mpProjectileHash[i] = NULL;
	}
	mUnitHashEnd = 0;
	mProjectileHashEnd = 0;
	mSelectedVehicleIndex = -1;
	char filename[256];
	strcpy(filename, "data/scenarios/maps/");
	strcat(filename, pTerrainHandler->mScenarioDetails.mUnitListName);
	if (!loadUnitsFromFile(filename)) { // TODO remove this line and make it dynamic (user-selected)
		printf("ERROR: Could not load units from file\n");
		return false;
	}
	if (!mLinesManager.init(pSceneManager, "uh")) { // init 3D line drawing
		printf("ERROR: Could not start 3D lines\n");
		return false;
	}
	printf("creating 3D lines for targeting and nav\n");
	mLinesManager.constructLine(1, 0, 0, 0); // create a new 3d line
	mLinesManager.constructLine(1, 1, 0, 0); // create a new 3d line
	mLinesManager.constructLine(0, 1, 0, 0); // create 3d line for target green
	mLinesManager.constructLine(1, 0, 0, 0); // create 3d line for target red
	mLinesManager.constructLine(0, 0, 0, 0); // create 3d line for target black
	
	if (g.mEnableGFS) {
		printf("initialising genetic algorithm manager\n");
		char geneticini[256];
		strcpy(geneticini, "genetic.ini");
		if (!mGAManager.mGASettings.loadFromFile(geneticini)) {
			printf("ERROR: unithandler couldnt do this : mGAManager.mGASettings.loadFromFile()\n");
			return false;
		}
		if (!mGAManager.loadRunKeys()) {
			printf("ERROR: unithandler couldnt do this : mGAManager.loadRunKeys()\n");
			return false;
		}
	}
	
	printf("Initialising Unit Handler...DONE\n");
	return true;
}

// free resources held by UnitHandler
bool UnitHandler::free() {
	printf("Freeing Unit Handler\n");
	printf("about to free 3d lines\n");
	/*if (!mLinesManager.free()) { // free 3d lines
		printf("ERROR: Could not free 3D lines\n");
		return false;
	}*/
	printf("about to free vehicles\n");
	for (int i = 0; i < VEHICLE_HASH_MAX; i++) { // reset vehicle hash
		if (mUnitHash[i] != NULL) {
			delete mUnitHash[i];
			mUnitHash[i] = NULL;
		}
	}
	for (int i = 0; i < PROJECTILE_HASH_MAX; i++) { // reset projectile hash
		if (mpProjectileHash[i] != NULL) {
			delete mpProjectileHash[i];
			mpProjectileHash[i] = NULL;
		}
	}
	return true;
}

// update
bool UnitHandler::update(unsigned long &elapsed) {
	if (g.mIsClient) { // updates based on network first
		if (!updateClient()) {
			printf("ERROR: could not update client\n");
			return false;
		}
	}
	if (g.mIsServer) { // updates based on network first
		if (!updateServer()) {
			printf("ERROR: could not update server\n");
			return false;
		}
	}
	for (int i = 0; i < mProjectileHashEnd; i++) { // TEMP HACK iterate through projectile hash
		if (mpProjectileHash[i] == NULL) {
			continue; // skip blank entries
		}

		if (!mpProjectileHash[i]->update(elapsed)) { // Update projectile here
			printf("ERROR: could not update projectile %i\n" ,i);
			return false;
		}
	}
	mLinesManager.hide3DLine(mOALineIndex); // hide 3D lines
	mLinesManager.hide3DLine(mTSLineIndex); // hide 3D lines
	mLinesManager.hide3DLine(2);
	mLinesManager.hide3DLine(3);
	mLinesManager.hide3DLine(4);
	if (!updateVehicles(elapsed)) { // update all the vehicles
		printf("ERROR: could not update vehicles\n");
		return false;
	}
	return true;
}

bool UnitHandler::createCharacterAtPosition(char* specFile, Ogre::Vector3 pos, double yaw, int force) {
	int defIndex = getSpecIndexOfFileLoadingIfNecessary(specFile);
	if (!createTank(defIndex, pos, yaw, force)) { // create the unit as specified
		printf("ERROR: UnitHandler::createCharacterAtPosition Could not create unit\n");
		return false;
	}

	return true;
}

int UnitHandler::getSpecIndexOfFileLoadingIfNecessary(char* specFile) {
	// look for definition in memory
	int defIndex = -1;
	for (int i = 0; i < mVehicleDefHashEnd; i++) {
		if (strcmp(mVehicleDefHash[i].mFilename, specFile) == 0) {
			return i; // this is the def, and we already have a copy
		}
	}
	// didn't find in memory, so create a new spec and load in from file
	if (mVehicleDefHashEnd >= VEHICLE_DEF_MAX) {
		printf("FATAL ERROR: Too many vehicle specification files held in memory!\nUnitHandler::getSpecIndexOfFileLoadingIfNecessary()\n");
		exit(1);
	}
	printf("spec was not found pre-loaded in list(%i)\n", mVehicleDefHashEnd);
	if (!mVehicleDefHash[mVehicleDefHashEnd].loadFromXMLfile(specFile)) { // load the vehicle specification from a VEH file
		printf("FATAL ERROR: Could not load vehicle specification file file %s\nUnitHandler::getSpecIndexOfFileLoadingIfNecessary()\n",specFile);
		exit(1);
	}
	// load required stuff from DEF
	defIndex = mVehicleDefHashEnd;
	// 1. audio
	if (mVehicleDefHash[mVehicleDefHashEnd].mHasMoveSound) { // check if a driving sound is available
		char fullPath[256];
		strcpy(fullPath, "data/vehicles/audio/");
		strcat(fullPath, mVehicleDefHash[mVehicleDefHashEnd].mMoveSoundFile);
		int sound = SoundManager::getSingleton().createLoopedSound(fullPath);
		mVehicleDefHash[mVehicleDefHashEnd].mMoveSoundIndex = sound;
	}
	if (mVehicleDefHash[mVehicleDefHashEnd].mHasIdleSound) { // check if an idling sound is available
		char fullPath[256];
		strcpy(fullPath, "data/vehicles/audio/");
		strcat(fullPath, mVehicleDefHash[mVehicleDefHashEnd].mIdleSoundFile);
		int sound = SoundManager::getSingleton().createLoopedSound(fullPath);
		mVehicleDefHash[mVehicleDefHashEnd].mIdleSoundIndex = sound;
	}
	if (mVehicleDefHash[mVehicleDefHashEnd].mCannon.mHasFireAudio) { // check if various cannon sounds are available
		char fullPath[256];
		strcpy(fullPath, "data/vehicles/audio/");
		strcat(fullPath, mVehicleDefHash[mVehicleDefHashEnd].mCannon.mFireAudio);
		int sound = SoundManager::getSingleton().createSound(fullPath); // load the sound file into memory. also keep track of index
		mVehicleDefHash[mVehicleDefHashEnd].mCannon.mFireAudioIndex = sound; // set the sound index in the spec entry
	}
	mVehicleDefHashEnd++; // increment count of DEFs held in memory

	return defIndex;
}

bool UnitHandler::saveUnitsToFile(const char* unitFile) {
	printf("saving units to %s\n", unitFile);
	ofstream file;
	file.open(unitFile);
	if(!file) {
		printf("Error: UnitHandler::saveUnitsToFile Could not open unit file %s!\n", unitFile);
		return false;
	}

	for (int i = 0; i < mUnitHashEnd; i++) {
		char line[256];
		if (!mUnitHash[i]) {
			continue;
		}
		int ridingIndex = -1;
		if (mUnitHash[i]->mIsRidingOtherUnit) {
			ridingIndex = mUnitHash[i]->mIndexOfUnitBeingRidden;
		}
		sprintf(line, "%i %s {%.2lf,%.2lf,%.2lf}{%lf} RIDE:%i\n", mUnitHash[i]->mForce, mUnitHash[i]->pSpecPtr->mFilename, mUnitHash[i]->mPosition[0], mUnitHash[i]->mPosition[1], mUnitHash[i]->mPosition[2], mUnitHash[i]->mHeading, ridingIndex);
		file << line;
	}

	file.close();

	return true;
}

// load a bunch of units from a file
bool UnitHandler::loadUnitsFromFile(const char* unitFile) {
	ifstream file;
	file.open(unitFile); // load file containing unit list
	if(!file) {
		printf("Error: UnitHandler::loadUnitsFromFile Could not open unit file %s!\n", unitFile);
		return false;
	}

	char unitDetails[256];
	while (file.getline(unitDetails, 256)) {
		if ((unitDetails[0] == '#') || (strcmp(unitDetails, "") == 0)) {
			continue;
		}
		// scan unit details
		double x, y, z, u;
		x = y = z = u = 0.0;
		int force = -1;
		int ride = -1;
		char defFile[256];
		sscanf(unitDetails,"%i %s {%lf,%lf,%lf}{%lf} RIDE:%i\n", &force, defFile, &x, &y, &z, &u, &ride);
		if (force < 10) {
			g.mScores[force].mUnitsStart++;
			g.mScores[force].mUnitsRemaining++;
		}
		Vector3 pos;
		pos.x = x;
		pos.y = y;
		pos.z = z;

		if (!createCharacterAtPosition(defFile, pos, u, force)) { // create the unit as specified
			printf("ERROR: UnitHandler::loadUnitsFromFile Could not create unit\n");
			return false;
		}
		printf(" mounting unit on vehicle %i...\n", ride);
		if (ride > -1) {
			orderBoardVehicle(mUnitHashEnd - 1, ride);
		}
	}
	printf("%i animated characters loaded\n", mTankEntityCount);
	file.close();
	return true;
}

// creates a unit from named mesh type at specified position
bool UnitHandler::createTank(int defIndex, Vector3& pos, double yaw, int force) {
	if (!pSceneManager) {
		printf("ERROR: UnitHandler::createTank SceneManager = NULL\n");
		return false;
	}
	char tankEntityName[64];
	sprintf(tankEntityName, "vehicleEntity%i", mTankEntityCount);
	printf("creating tank from defIndex %i at position {%f,%f,%f}\n", defIndex, pos.x, pos.y, pos.z);
	printf("mesh file is [%s]\n", mVehicleDefHash[defIndex].mMeshFile);
	Entity* ent = NULL;
	ent = pSceneManager->createEntity(tankEntityName, mVehicleDefHash[defIndex].mMeshFile); // create the entity (blender mesh)
	if (!ent) {
		printf("ERROR: could not create hull entity from mesh file\n");
		return false;
	}
	ent->setCastShadows(true); // set to cast shadows
	Ogre::Vector3 size = ent->getBoundingBox().getSize();
	double boundRad = size.x;
	if (size.y > boundRad) {
		boundRad = size.y;
	}
	if (size.z > boundRad) {
		boundRad = size.z;
	}
	
	SceneNode* tempNode = pSceneManager->getRootSceneNode()->createChildSceneNode(String(tankEntityName) + "Node"); // create a node for the mesh to attach to
	if (!tempNode) {
		printf("Error: could not create hull nodes\n");
		return false;
	}
	printf("attaching object to node...\n");
	tempNode->attachObject(ent);
	if (mUnitHashEnd >= VEHICLE_HASH_MAX) { // create the tempNode as a vehicle
		printf("ERROR: Too many vehicles, could not create any more.\n");
		return false;	// too many vehicles
	}
	int slotToUse = mUnitHashEnd; // default slot is at the end
	for (int i = 0; i < mUnitHashEnd; i++) { // first look for empty slots
		if (mUnitHash[i] == NULL) {
			// found a free spot - make new tank here
			slotToUse = i;
			break;
		}
	}
	printf("polymorphing...\n");
	// infantry or vehicle polymorphism
	if (mVehicleDefHash[defIndex].mIsInfantry) {
		mUnitHash[slotToUse] = new Infantry;
	} else {
		mUnitHash[slotToUse] = new Vehicle;
	}
	if (!mUnitHash[slotToUse]) {
		printf("Fatal Error: not enough free memory to create vehicle\n");
		return false;
	}
	if (!mUnitHash[slotToUse]->init(pTerrainHandler, &mVehicleDefHash[defIndex], ent, tempNode, pos, boundRad, force)) { // initialise vehicle
		printf("Error: failed to create vehicle\n");
		return false;
	}
	mUnitHash[slotToUse]->mUniqueIndexId = slotToUse; // let the unit know what its own index is
	mUnitHash[slotToUse]->setHeading(yaw);

	for (int i = 0; i < mVehicleDefHash[defIndex].mSpecialEffects.mDustEmitterCount; i++) { // attach dust emitters to pre-designated dust emission points.
		char tempPartName[64];
		sprintf(tempPartName, "%iEffectNode%i", mTankEntityCount, i);
		SceneNode* dustNode = tempNode->createChildSceneNode(String(tempPartName) + "Node"); // create a new node for each one
		ParticleSystem* dustSystem = pSceneManager->createParticleSystem(tempPartName, "Particle/Dust");
		dustNode->attachObject(dustSystem);
		dustNode->setInheritOrientation(false);
		Bone* dustBonePtr = ent->getSkeleton()->getBone(mVehicleDefHash[defIndex].mSpecialEffects.mDustEmitterBoneName[i]);
		Vector3 bonePos = dustBonePtr->_getDerivedPosition();// get the position of the bone from the armature
		dustNode->setPosition(bonePos); // put the node on this position
		printf(" adding dust controller %i to vehicle\n", i);
		if (!mUnitHash[slotToUse]->addDustController(dustSystem)) {
			printf("ERROR: could not add dust controller to vehicle\n");
			return false;
		}
	}
	for (int i = 0; i < mVehicleDefHash[defIndex].mSpecialEffects.mExhaustEmitterCount; i++) { // attach Exhaust emitters to pre-designated Exhaust emission points.
		char tempPartName[64];
		sprintf(tempPartName, "%iEffectNodeExh%i", mTankEntityCount, i);
		SceneNode* exhaustNode = tempNode->createChildSceneNode(String(tempPartName) + "Node"); // create a new node for each one
		ParticleSystem* exhaustSystem = pSceneManager->createParticleSystem(tempPartName, "Particle/Exhaust");
		exhaustNode->setInheritOrientation(false);
		exhaustNode->attachObject(exhaustSystem);
		Bone* exhaustBonePtr = ent->getSkeleton()->getBone(mVehicleDefHash[defIndex].mSpecialEffects.mExhaustEmitterBoneName[i]);
		Vector3 bonePos = exhaustBonePtr->_getDerivedPosition();
		exhaustNode->setPosition(bonePos); // put the node on this position
		if (!mUnitHash[slotToUse]->addDustController(exhaustSystem)) {
			printf("ERROR: could not add dust controller to vehicle\n");
			return false;
		}
	}
	for (int i = 0; i < mVehicleDefHash[defIndex].mSpecialEffects.mWakeEmitterCount; i++) { // attach
		char tempPartName[64];
		sprintf(tempPartName, "%iEffectNodeWak%i", mTankEntityCount, i);
		SceneNode* wakeNode = tempNode->createChildSceneNode(String(tempPartName) + "Node"); // create a new node for each one
		ParticleSystem* wakeSystem = pSceneManager->createParticleSystem(tempPartName, "Particle/ShipWake");
		wakeNode->setInheritOrientation(true);
		wakeNode->attachObject(wakeSystem);
		Bone* wakeBonePtr = ent->getSkeleton()->getBone(mVehicleDefHash[defIndex].mSpecialEffects.mWakeEmitterBoneName[i]);
		Vector3 bonePos = wakeBonePtr->_getDerivedPosition();
		wakeNode->setPosition(bonePos); // put the node on this position
		if (!mUnitHash[slotToUse]->addWakeController(wakeSystem)) {
			printf("ERROR: could not add wake particle controller to vehicle\n");
			return false;
		}
	}
	mTankEntityCount++; // increment counter of entities
	if (slotToUse == mUnitHashEnd) { // increment hash slots and counters
		mUnitHashEnd++;
	}
	printf("tank created\n");
	return true;
}

// creates a projectile at specified position
bool UnitHandler::createProjectile(Vector3 pos, double muzzleVelocity, Vector3 gunOrientation) {
	if (!pSceneManager) {
		printf("ERROR: SceneManager = NULL\n");
		return false;
	}
	// create the entity (blender mesh)
	char projectileEntityName[64];
	sprintf(projectileEntityName, "projectileEntity%i", mProjectileEntityCount);
	Entity* ent = pSceneManager->createEntity(projectileEntityName, "SmallShell.mesh");
	if (!ent) {
		printf("ERROR: could not create hull entity from mesh file\n");
		return false;
	}
	ent->setCastShadows(false); // no shadows thanks
	SceneNode* tempNode = pSceneManager->getRootSceneNode()->createChildSceneNode(String(projectileEntityName) + "Node");
	if (!tempNode) {
		printf("ERROR: could not create projectile nodes\n");
		return false;
	}
	tempNode->attachObject(ent);
	mProjectileEntityCount++;

	// create the tempNode as a projectile
	if (mProjectileHashEnd >= PROJECTILE_HASH_MAX) {
		printf("ERROR: Too many projectiles (%i), could not create any more.\n", PROJECTILE_HASH_MAX);
		return false; // too many projectiles
	}

	int slotToUse = mProjectileHashEnd; // default slot is at the end
	// first look for empty slots
	for (int i = 0; i < mProjectileHashEnd; i++) {
		if (mpProjectileHash[i] == NULL) {
			// found a free spot - make new tank here
			slotToUse = i;
			break;
		}
	}

	mpProjectileHash[slotToUse] = new Projectile;
	if (!mpProjectileHash[slotToUse]) {
		printf("ERROR: not enough free memory to create projectile\n");
		return false;
	}

	// initialise projectile
	if(!mpProjectileHash[slotToUse]->init(pTerrainHandler, tempNode, pos, muzzleVelocity, gunOrientation)) {
		printf("ERROR: failed to init projectile\n");
		return false;
	}

	// create a particle system for explosions
	char tempPartName[64];
	sprintf(tempPartName, "%iEffectNode", mProjectileEntityCount);
	SceneNode* effectNode = tempNode->createChildSceneNode(String(tempPartName) + "Node"); // create a new node for each one
	ParticleSystem* explosionSystem = pSceneManager->createParticleSystem(tempPartName, "Particle/DirtSplash");
	effectNode->attachObject(explosionSystem);
	effectNode->setInheritOrientation(false);
	// add these particle systems to projectile so that it can turn them on and off
		if(!mpProjectileHash[slotToUse]->setExplosionController(explosionSystem)) {
		printf("ERROR: could not add effect controller to projectile\n");
		return false;
	}

	if (slotToUse == mProjectileHashEnd) {
		mProjectileHashEnd++;
	}

	return true;
}

void UnitHandler::setSelectedNode(SceneNode* node) {
	mSelectedNode = node;
}

void UnitHandler::setTargetedNode(SceneNode* node) {
	mTargetedNode = node;
}

void UnitHandler::deselectAll(bool editmode) {
	if (mUnitHash[mSelectedVehicleIndex] != NULL) { // deselect the selected vehicle
		mUnitHash[mSelectedVehicleIndex]->setSelected(false);
	}
	mSelectedVehicleIndex = -1;
	if (editmode) {
		mLinesManager.hide3DLine(0); // stop displaying 3D lines
		mLinesManager.hide3DLine(1); // stop displaying 3D lines
		mLinesManager.hide3DLine(2); // stop displaying 3D lines
		mLinesManager.hide3DLine(3); // stop displaying 3D lines
		mLinesManager.hide3DLine(4);
	}
}

// set destination for all selected units
void UnitHandler::orderDestination(Vector3 &destination) {
	if (mUnitHash[mSelectedVehicleIndex] == NULL) {
		return; // skip empty entries
	}
	if (mUnitHash[mSelectedVehicleIndex]->mForce != g.mOurForce) { // Check if is actually our unit, otherwise do nothing
		return;
	}
	if (mUnitHash[mSelectedVehicleIndex]->mIsRidingOtherUnit) {
		return; // do nothing as in transit
	}
	if (g.mIsServer) { // send order based on network setup (to server or to all clients or none)
		Vector3 pos;
		mUnitHash[mSelectedVehicleIndex]->getPosition(pos.x, pos.y, pos.z); // get some server-side details to pack into message
		double yr = mUnitHash[mSelectedVehicleIndex]->mHeading;
		g.mServer.sendOrderMove(mSelectedVehicleIndex, pos.x, pos.y, pos.z, yr, destination.x, destination.y, destination.z); // send out to all clients (including the one that sent it)
		mUnitHash[mSelectedVehicleIndex]->setFinalDestination(destination); // order unit
	} else if (g.mIsClient) { // send order based on network setup (to server or to all clients or none)
		g.mClient.sendOrderMove(mSelectedVehicleIndex, destination.x, destination.y, destination.z); // send order REQUEST to server only
	} else { // send order based on network setup (to server or to all clients or none)
		mUnitHash[mSelectedVehicleIndex]->setFinalDestination(destination); // order unit
	}
}

// set target for all selected units
void UnitHandler::orderAttackVehicle(int vehicle) {
	if (mUnitHash[mSelectedVehicleIndex] == NULL) {
		return;		// skip empty entries
	}
	if (mUnitHash[mSelectedVehicleIndex]->mForce != g.mOurForce) {
		return;			// Check if is actually our unit, otherwise do nothing
	}
	if (mUnitHash[mSelectedVehicleIndex]->mIsRidingOtherUnit) {
		return; // do nothing as in transit
	}

	if (g.mIsServer) { // send order based on network setup (to server or to all clients or none)
		Vector3 pos;
		mUnitHash[mSelectedVehicleIndex]->getPosition(pos.x, pos.y, pos.z); // get some server-side details to pack into message
		double yr = mUnitHash[mSelectedVehicleIndex]->mHeading;
		g.mServer.sendOrderAttackVehicle(mSelectedVehicleIndex,pos.x,pos.y,pos.z,yr,vehicle); // send out to all clients
		// order unit
		mUnitHash[mSelectedVehicleIndex]->attackVehicle(vehicle);
	} else if (g.mIsClient) { // send order REQUEST to server only
		g.mClient.sendOrderAttackVehicle(mSelectedVehicleIndex,vehicle);
	} else { // order unit
		mUnitHash[mSelectedVehicleIndex]->attackVehicle(vehicle);
	}
}

bool UnitHandler::orderBoardVehicle(const int& riderIndex, const int& mountIndex) {
	printf("db: order board vehicle\n");
	if (!mUnitHash[riderIndex]) {
		printf("ERROR: rider unithash index NULL\n");
		printf(WHERESTR "\n", WHEREARG);
	}
	if (mUnitHash[riderIndex]->mIsRidingOtherUnit) {
		printf("WARNING - trying to board unit whilst in transit!\n");
		return false; // do nothing as in transit
	}
	int vehType = -1;
	if (mUnitHash[riderIndex]->pSpecPtr->mIsInfantry) {
		vehType = 0;
	} else if (mUnitHash[riderIndex]->pSpecPtr->mIsLightVehicle) {
		vehType = 1;
	} else if (mUnitHash[riderIndex]->pSpecPtr->mIsAuxTurret) {
		vehType = 69;
	}
	if (!mUnitHash[mountIndex]->hasFreeSpace(vehType)) {
		printf("WARNING: No space left to board vehicle\n");
		return false;
	}
	// TODO the whole move to code and attach when "atVehicle" = true
	if (!mUnitHash[mountIndex]->attachUnit(mUnitHash[riderIndex]->getNodePtr(), vehType)) {
		printf("ERROR: could not attach unit in UnitHandler::orderBoardVehicle\n");
		return false;
	}
	mUnitHash[riderIndex]->mIsRidingOtherUnit = true;
	mUnitHash[riderIndex]->mIndexOfUnitBeingRidden = mountIndex;
	return true;
}


// get nearest obstacle to given indexed unit. false in no obstacles. also provides radius of object (~size of obstacle) TODO: {3D distances, dynamic obstacles}
bool UnitHandler::getNearestVehicleTo(int index, Vector3& obstacle, double& distanceSquared, double& sizeRadius, AxisAlignedBox& boundingBox) {
	if ((index < 0) || (index >= mUnitHashEnd)) { // validation
		printf("FATAL ERROR: vehicle does not exist in UnitHandler::getNearestVehicleTo()\n");
		exit(1);
	}
	if (!mUnitHash[index]) { // validation
		printf("FATAL ERROR: vehicle does not exist in UnitHandler::getNearestVehicleTo()\n");
		exit(1);
	}
	if (!pTerrainHandler) { // validation
		printf("FATAL ERROR: pTerrainHandler not initialised in UnitHandler::getNearestVehicleTo()\n");
		exit(1);
	}
	Vector3 origin;
	mUnitHash[index]->getPosition(origin.x, origin.y, origin.z); // get own position
	double currentHeading = mUnitHash[index]->mHeading; // get own heading
	double angleThresh = mUnitHash[index]->getObstacleAngleThreshold(); // get our obstacle threshold (how far from front of us to continue taking obstacle into account)
	Vector3 nearestObstaclePosition;
	nearestObstaclePosition.x = nearestObstaclePosition.y = nearestObstaclePosition.z = 0.0f; // reset
	double nearestObstacleDistance = 9876543210.987654210f; // "current" nearest obst (none) is hugely far away, so that the first thing that we find is better
	double nearestObstacleSizeRadius = 0.0f;
	int nearestObstacleIndex = -1; // -1 means none found yet

	// look through all vehicles TODO: optimise this to only vehicles within a reference zone
	for (int i = 0; i < mUnitHashEnd; i++) {
		if (i == index) { // skip own vehicle
			continue;
		}
		if (!mUnitHash[i]) { // skip empty entries
			continue;
		}
		Vector3 obstPosition;
		mUnitHash[i]->getPosition(obstPosition.x, obstPosition.y, obstPosition.z); // find out where this obstacle is geographically
		double xDist = abs(obstPosition.x - origin.x); // Calculate 2d Distance btwn points
		double zDist = abs(obstPosition.z - origin.z); // Calculate 2d Distance btwn points
		double squaredDistance = xDist * xDist + zDist * zDist; // squared distance in meters to centre of obstacle
		double radius = mUnitHash[i]->getOwnSizeRadiusMeters(); // radius size of obstacle (not squared)
		double sqDistInclSqRadius = squaredDistance - (radius * radius); // include radius (size) in nearest obstacle calculation
		// find out if this is a new closest obstacle
		if (sqDistInclSqRadius < nearestObstacleDistance) { // compare sqDist - rad^2
			#ifndef DISABLE_NEAR_VEH_THRESH
			double chInHeading = pTerrainHandler->getChangeInHeadingTo(origin, currentHeading, obstPosition); // check if obst is in front of us
			if (chInHeading < angleThresh) { // only consider obst in front of us
			#endif
				nearestObstaclePosition = obstPosition;
				nearestObstacleDistance = squaredDistance; // set sqDist and radius separately
				nearestObstacleSizeRadius = radius; // set sqDist and radius separately
				nearestObstacleIndex = i; // set this as nearest obst so far
			#ifndef DISABLE_NEAR_VEH_THRESH
			}
			#endif
		}
	}
	if (nearestObstacleIndex > -1) { // set required details
		obstacle = nearestObstaclePosition;
		distanceSquared = nearestObstacleDistance;
		sizeRadius = nearestObstacleSizeRadius;
		boundingBox = mUnitHash[nearestObstacleIndex]->getBoundingBox();
		return true; // return success (that we have found a closest obstacle)
	}
	return false; // return failure (that no closest obstacle found)
}

/*! get the next nearest enemy unit using 2D (birds-eye) distances
@param prevClosest index of previous 'closest' unit. or -1 to ignore
@param ourUnit index of our unit
@return index of enemy or -1 if none left */
int UnitHandler::getNextNearestEnemy(const int& prevClosest, const int& ourUnit) {
	int enemyIndex = -1;
	double prevDistance = 9999999999.0;
	double minDistance = 0.0;
	if (prevClosest >= 0) {
		minDistance = sqDistanceBetween(mUnitHash[ourUnit]->mPosition[0], mUnitHash[ourUnit]->mPosition[2], mUnitHash[prevClosest]->mPosition[0], mUnitHash[prevClosest]->mPosition[2]);
	}
	//printf("db: prevdist was %lf\n", prevDistance);
	
	for (int i = 0; i < mUnitHashEnd; i++) {
		if (!mUnitHash[i]) { // skip empty entries
			continue;
		}
		if (mUnitHash[i]->mForce == mUnitHash[ourUnit]->mForce) {
			continue; // ignore friends
		}
		if (mUnitHash[i]->mDestroyed) {
			continue; // ignore dead enemies
		}
		double distance = sqDistanceBetween(mUnitHash[ourUnit]->mPosition[0], mUnitHash[ourUnit]->mPosition[2], mUnitHash[i]->mPosition[0], mUnitHash[i]->mPosition[2]);
		if (distance <= minDistance) {
			continue; // ignore those closer than 
		}
		if (i == prevClosest) {
			continue; // ignore the last target
		}
		//printf("db: newdist was %lf\n", distance);
		if (distance < prevDistance) {
			prevDistance = distance;
			enemyIndex = i;
		}
	}
	return enemyIndex;
}

// get this vehicle or null
Unit* UnitHandler::getVehicleAtIndex(int index) {
	if ((index < 0) || (index >= mUnitHashEnd)) { // check if out of memory bounds
		return NULL;
	}
	if (!mUnitHash[index]) { // check if that vehicle exists
		return NULL;
	}
	return mUnitHash[index]; // return vehicle
}

bool UnitHandler::updateClient() {
	if (g.mClient.gotOrderString()) {
		char rxString[100];
		strcpy(rxString,g.mClient.getOrderString());
		// got an order
		if (strncmp(rxString,"/order mov",10) == 0) {
			int unitID;	// tmp variables to store stuff from mesg
			double xp,yp,zp;
			double yr;
			double xd,yd,zd;
			// clients receive the position and orientation as well (to keep in sync)
			sscanf(rxString,"/order mov [%i] {%lf,%lf,%lf} {%lf} {%lf,%lf,%lf}", &unitID, &xp, &yp, &zp, &yr, &xd, &yd, &zd);

			// validate codes
			if ((unitID < 0) || (unitID >= mUnitHashEnd)) {
				printf("ERROR: Client received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			if (mUnitHash[unitID] == NULL) {
				printf("ERROR: Client received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}

			Vector3 destVec(xd,yd,zd);	// same but for destination
			printf("DEBUG: Unithandler::updateClient\n");
			mUnitHash[unitID]->setPosition(xp, yp, zp);	// set position as per message
			mUnitHash[unitID]->mHeading = yr;				// set heading as per message
			mUnitHash[unitID]->balance();	// pre-balance vehicle just in case orient changed but not moving
			mUnitHash[unitID]->setFinalDestination(destVec);	// set current waypoint as per message
		} else if (strncmp(rxString,"/order atv", 10) == 0) {
			int unitID;	// tmp variables to store stuff from mesg
			double xp,yp,zp;
			double yr;
			int targetID;
			sscanf(rxString,"/order atv [%i] {%lf,%lf,%lf} {%lf} [%i]", &unitID, &xp, &yp, &zp, &yr, &targetID); // clients receive the position and orientation as well (to keep in sync)
			if ((unitID < 0) || (unitID >= mUnitHashEnd)) { // validate codes
				printf("ERROR: UnitHandler::updateClient() Client received bad atv order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			if ((targetID < 0) || (targetID >= mUnitHashEnd)) {
				printf("ERROR: Client received bad atv order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			if ((mUnitHash[unitID] == NULL) || (mUnitHash[targetID] == NULL)) {
				printf("ERROR: Client received bad atv order; unit does not exist.\n %s\n",rxString);
				return false;
			}

			mUnitHash[unitID]->setPosition(xp,yp,zp);	// set position as per message
			mUnitHash[unitID]->mHeading = yr;				// set heading as per message
			mUnitHash[unitID]->balance();	// pre-balance vehicle just in case orient changed but not moving
			mUnitHash[unitID]->attackVehicle(targetID);
		} else if (strncmp(rxString,"/order kil", 10) == 0) { // destroy unit
			int attacker, defender;	// tmp variables to store stuff from mesg
			double xp, yp, zp, yr;
			sscanf(rxString,"/order kil [%i][%i] {%lf, %lf, %lf} {%lf}", &defender, &attacker, &xp, &yp, &zp, &yr); // clients receive the position and orientation as well (to keep in sync)
			mUnitHash[defender]->setPosition(xp,yp,zp);	// set position as per message
			mUnitHash[defender]->mHeading = yr;				// set heading as per message
			mUnitHash[defender]->balance();	// pre-balance vehicle just in case orient changed but not moving
			if (!destroyVehicle(defender, attacker)) {
				printf("ERROR: UnitHandler::updateClient() could not destroy vehicle\n");
				return false;
			}
		} else if (strncmp(rxString,"/order cpj", 10) == 0) {
			double muzzleVel = 0.0005; // HACK exchange hardcoded velocity with dynamic
			Vector3 direction;
			direction.x = direction.z = 0.0f;
			direction.y = -1.0f;
			double xp, yp, zp;
			sscanf(rxString,"/order cpj {%lf, %lf, %lf}", &xp, &yp, &zp);
			Vector3 posVec(xp, yp, zp);	// create a vector for current pos from message
			if (!createProjectile(posVec, muzzleVel, direction)) {
				printf("ERROR: UnitHandler::updateClient Could not create projectile\n");
				return false;
			}
		} else if (strncmp(rxString,"/order ric", 10) == 0) {
			int unitId;
			double xp, yp, zp, yr;
			sscanf(rxString,"/order ric [%i] {%lf, %lf, %lf} {%lf}", &unitId, &xp, &yp, &zp, &yr);
			// TODO play ricochet sound for client at this vehicle's location
		}
	}
	return true;
}

bool UnitHandler::updateServer() {
	if (g.mServer.gotOrderString()) {
		char rxString[100];
		strcpy(rxString,g.mServer.getOrderString());
		// got an order
		if (strncmp(rxString,"/order mov",10) == 0) {
			int unitID;
			double xd,yd,zd;
			sscanf(rxString,"/order mov [%i] {%lf,%lf,%lf}",&unitID,&xd,&yd,&zd);

			// validate codes
			if ((unitID < 0) || (unitID >= mUnitHashEnd)) {
				printf("ERROR: Server received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			if (mUnitHash[unitID] == NULL) {
				printf("ERROR: Server received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}

			// get some server-side details to pack into message
			Vector3 pos;
			mUnitHash[unitID]->getPosition(pos.x, pos.y, pos.z);
			double yr = mUnitHash[unitID]->mHeading;
			// send out to all clients (including the one that sent it)
			g.mServer.sendOrderMove(unitID,pos.x,pos.y,pos.z,yr,xd,yd,zd);
			// carry out order locally
			Vector3 destVec(xd,yd,zd);
			mUnitHash[unitID]->setFinalDestination(destVec);
		} else if (strncmp(rxString,"/order atv",10) == 0) {
			int unitID;
			int targetID;
			sscanf(rxString,"/order atv [%i] [%i]",&unitID,&targetID);

			// validate codes
			if ((unitID < 0) || (unitID >= mUnitHashEnd)) {
				printf("ERROR: Server received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			if ((targetID < 0) || (targetID >= mUnitHashEnd)) {
				printf("ERROR: Server received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			if ((mUnitHash[unitID] == NULL) || (mUnitHash[targetID] == NULL)) {
				printf("ERROR: Server received bad order; unit does not exist.\n %s\n",rxString);
				return false;
			}
			Vector3 pos;
			mUnitHash[unitID]->getPosition(pos.x, pos.y, pos.z); // get some server-side details to pack into message
			double yr = mUnitHash[unitID]->mHeading;
			g.mServer.sendOrderAttackVehicle(unitID,pos.x,pos.y,pos.z,yr,targetID); // send out to all clients (including the one that sent it)
			mUnitHash[unitID]->attackVehicle(targetID); // carry out order locally
		}
	}
	return true;
}

// add dynamic obstacles to map
bool UnitHandler::addAllToObstacleMap() {/*
	for (int i = 0; i < mUnitHashEnd; i++) {
		if (mUnitHash[i] == NULL) {
			continue; // skip blank entries
		}
		// update obstacle occupancy map (remove vehicle-occupied nodes from navigation search domain)
		int obstx = -1;
		int obsty = -1;
		if (!g.mBrainGrid.calcGridPos(mUnitHash[i]->getPosition().x, mUnitHash[i]->getPosition().z, obstx, obsty)) { // work out grid location of vehicle
			printf("ERROR: vehicle at invalid location from grid map\n");
			return false;
		}
		g.mBrainGrid.addUnitToCell(obstx, obsty, i); // add obstacle to map
		if (!g.mSmallGrid.calcGridPos(mUnitHash[i]->getPosition().x, mUnitHash[i]->getPosition().z, obstx, obsty)) { // work out grid location of vehicle
			printf("ERROR: vehicle at invalid location from grid map\n");
			return false;
		}
		g.mSmallGrid.addUnitToCell(obstx, obsty, i); // add obstacle to map
	}*/
	return true;
}

bool UnitHandler::updateVehicles(unsigned long &elapsed) {
	for (int i = 0; i < mUnitHashEnd; i++) { // iterate through entire vehicle hash
		if (mUnitHash[i] == NULL) {
			continue; // skip blank entries
		}
		
		// reset things that need to be re-tested
		bool alreadySpottedTarget = false;
		
		// AI team assign targets
/*
		if (mUnitHash[i]->mForce != g.mOurForce) {
			bool validtargetFound = false;
			int enemy = -1;
			while (validtargetFound == false) {
				enemy = getNextNearestEnemy(enemy, i); // at this stage wait until it comes into sight rather than attacking nearest visible enemy
				//printf("db: enemy is %i\n", enemy);
				if (enemy < 0) {
					//printf("db: no valid target\n");
					break; // no targets left - wait until one appears
				}
				
				// check for LOS
				if (canSeeTarget(i, enemy)) { // check for line of fire
					mUnitHash[i]->mCanSeeTarget = true;
					alreadySpottedTarget = true;
					validtargetFound = true;
					mUnitHash[i]->attackVehicle(enemy);
					mUnitHash[i]->mGotPrimaryTarget = true;
				}
			}
		} else if (mUnitHash[i]->mGotPrimaryTarget) {
			if (mUnitHash[mUnitHash[i]->mPrimaryTargetIndex] != NULL) {
				if (mUnitHash[mUnitHash[i]->mPrimaryTargetIndex]->mDestroyed) {
					mUnitHash[i]->mPrimaryTargetIndex = -1; // reset target when destroyed
					mUnitHash[i]->mGotPrimaryTarget = false;
				}
			}
		}
*/
		
		// GFS
		if (g.mEnableGFS) {
			if (mUnitHash[i]->pSpecPtr->mAISpec.mIsInTraining) {
				if (mUnitHash[i]->mGALogger.mRunsLeft <= 0) {
					// first check how many runs it completed, and tell the manager how many were done
					if (mUnitHash[i]->mGALogger.mRunsCompleted > 0) {
						if (!mGAManager.updateCompletedRuns(mUnitHash[i]->mGALogger.mGeneration, mUnitHash[i]->mGALogger.mBatch, mUnitHash[i]->mGALogger.mRunsCompleted)) {
							// could complain here - but probably just waiting on the GA pipeline to spit out a new generation
						}
					}
					long int runsRequested = 5;
					long int gen = 0;
					long int batch = 0;
					long int runsGranted = 0;
					int index = mGAManager.getIndexOfNextIndividual(runsRequested, gen, batch, runsGranted);
					if (index > -1) {
						printf("DEBUG: assigning training to unit[%i]. gen(%li), bat(%li), runs(%li)\n", i, gen, batch, runsGranted);
						mUnitHash[i]->assignTraining(mGAManager.mGASettings.mGenePoolFile, gen, batch, runsGranted);
					} else {
						printf("WARNING: error requesting training runs...maybe training generation finished? unithandler::updatevehicles()\n");
					}
				}
			}
		}
		
		if (!mUnitHash[i]->update(elapsed)) { // 3.) Update vehicle here
			printf("ERROR: could not update vehicle %i\n",i);
			return false;
		}
		if (mUnitHash[i]->mDestroyed) {
			continue;
		}
		// 1.) work out the nearest obstacle to this vehicle if it is driving and tell the vehicle about the obstacle
		if (mUnitHash[i]->mGotCurrentWaypoint) {
			mLinesManager.hide3DLine(2); // stop displaying 3D lines
			mLinesManager.hide3DLine(3); // stop displaying 3D lines
			mLinesManager.hide3DLine(4);
			AxisAlignedBox obstBoundingBoxDynamic; // look for nearest dynamic obstacle
			Vector3 nearestDynamicObstaclePos;
			nearestDynamicObstaclePos.x = nearestDynamicObstaclePos.y = nearestDynamicObstaclePos.z = 0.0f; // reset
			double distSquaredDynamic = 0.0f;
			double sizeRadiusDynamic = 0.0f;
			bool foundDynObst = false;

			// tell brain grid to scan nearby cells for obstacles of all types
			#ifdef _GO_GRIDS_
			if (mUnitHash[i]->pSpecPtr->mIsInfantry) {
				g.mBrainGrid.countObstaclesAroundPos(mUnitHash[i]->getPosition().x, mUnitHash[i]->getPosition().z, 8.0f);
			} else {
				g.mBrainGrid.countObstaclesAroundPos(mUnitHash[i]->getPosition().x, mUnitHash[i]->getPosition().z, 40.0f);
			}
			#endif

			foundDynObst = getNearestVehicleTo(i, nearestDynamicObstaclePos, distSquaredDynamic, sizeRadiusDynamic, obstBoundingBoxDynamic); // dynamic obstacle search
			double distSqDynInclRadius =  distSquaredDynamic - sizeRadiusDynamic;
			// look for nearest static obstacle
			AxisAlignedBox obstBoundingBoxStatic;
			Vector3 nearestStaticObstaclePos;
			nearestStaticObstaclePos.x = nearestStaticObstaclePos.y = nearestStaticObstaclePos.z = 0.0f; // reset
			double distSquaredStatic = 0.0;
			double sizeRadiusStatic = 0.0;
			bool foundStaticObst = false;
			Vector3 ownPos;
			mUnitHash[i]->getPosition(ownPos.x, ownPos.y, ownPos.z);
			double ownHeading = mUnitHash[i]->mHeading;
			double obstAngleThresh = mUnitHash[i]->getObstacleAngleThreshold(); // get obst consideration angle cut-off limit
			foundStaticObst = pTerrainHandler->getNearestObstacleTo(ownPos, ownHeading, obstAngleThresh, nearestStaticObstaclePos, distSquaredStatic, sizeRadiusStatic, obstBoundingBoxStatic); // static obstacle search
			double distSqStatInclRadius = distSquaredStatic - sizeRadiusStatic;
			if (foundStaticObst && foundDynObst) { // compare obstacles to find closest of either type
				if (distSqDynInclRadius < distSqStatInclRadius) {
					foundStaticObst = false; // dynamic is closer so ignore static
				} else {
					foundDynObst = false; // turn off the other one
				}
			}
			AxisAlignedBox obstBoundingBox;
			Vector3 nearestObstaclePos;
			nearestObstaclePos.x = nearestObstaclePos.y = nearestObstaclePos.z = 0.0f; // reset
			double distSquared = 0.0;
			double sizeRadius = 0.0;
			if (foundStaticObst) {
				nearestObstaclePos = nearestStaticObstaclePos;
				distSquared = distSquaredStatic;
				sizeRadius = sizeRadiusStatic;
				obstBoundingBox = obstBoundingBoxStatic;
			}
			if (foundDynObst) {
				nearestObstaclePos = nearestDynamicObstaclePos;
				distSquared = distSquaredDynamic;
				sizeRadius = sizeRadiusDynamic;
				obstBoundingBox = obstBoundingBoxDynamic;
			}
			if ((foundStaticObst || foundDynObst) && (distSquared < OBST_DIST_MAX)) {
				mUnitHash[i]->setNearestObstacle(nearestObstaclePos, distSquared, sizeRadius); // tell the vehicle about the obstacle
				if (mUnitHash[i]->isSelected()) { // draw a 3D line to the obstacle if the vehicle is selected
					Vector3 startPosition;
					mUnitHash[i]->getPosition(startPosition.x, startPosition.y, startPosition.z);
					startPosition.y = startPosition.y + 5.0f; // raise the start 5m so that it doesn't disappear under the ground
					Vector3 adjustedEndPosition = nearestObstaclePos;
					adjustedEndPosition.y = adjustedEndPosition.y + 5.0f; // raise the end 5m so that it doesn't disappear under the ground
					if (!mLinesManager.show3DLineFromHereToThere(0, startPosition,  adjustedEndPosition)) {
						printf("ERROR: could not show 3D line in UnitHandler::updateVehicles\n");
						return false;
					}
					mLinesManager.start3DLineStrip(1); // start the yellow path line
					int wpCount = mUnitHash[i]->mWaypointCount;
					for (int j = 0; j < wpCount; j++) {
						Vector3 position = mUnitHash[i]->getWaypoint(j);
						position.y = pTerrainHandler->getTerrainHeightAt(position.x, position.z) + 2.5f; // move above ground 5 meters
						mLinesManager.addPointTo3DLineStrip(1, position);
					}
					mLinesManager.end3DLineStrip(1);
					#ifdef _GO_GRIDS_
					if (g.mNavGraphsShowing) {
						if (mUnitHash[i]->pSpecPtr->mIsInfantry) {
							g.mGridOverlay.hide();
							g.mSmallGridOverlay.show(g.mSmallGrid);
							g.mSmallGridOverlay.update(startPosition, g.mSmallGrid);
						} else {
							g.mSmallGridOverlay.hide();
							g.mGridOverlay.show(g.mBrainGrid);
							g.mGridOverlay.update(startPosition, g.mBrainGrid);
						}
					}
					#endif
				}
				//if (mUnitHash[i]->getBoundingBox().intersects(obstBoundingBox)) { // work out if we have just crashed - Bounding box intersection check ****! ERROR. this wasn't working for scenery. because of paging library???
					double sumOfRadiiSquard = mUnitHash[i]->getOwnSizeRadiusMeters() * mUnitHash[i]->getOwnSizeRadiusMeters() + sizeRadius * sizeRadius;
					double sqMetersOfPenetration = sumOfRadiiSquard - distSquared; // crash severity heuristic
					if (sqMetersOfPenetration > 0.0) {
						//g.mCrashGraph.addPointToLine(0, ((float)(g.mSimTimer.getMicroseconds() - g.mTotalTimePaused)) / 1000000.0f, sqMetersOfPenetration, true); // draw crash on graph with desensitise mode on the graph
						g.mCrashHeuristic += sqMetersOfPenetration * (float)elapsed; // keep track of total crashes per time unit so far
						mUnitHash[i]->mCollisions++; // bang!
						mUnitHash[i]->mGALogger.mCollisionHeuristic += sqrt(sqMetersOfPenetration) * ((double)elapsed / 1000000.0);
					}
				//}
			}
		}
		if (mUnitHash[i]->isSelected()) { // 2.) update gui if this vehicle is selected
			updateSelectedVehicle(i);
		}
		if (mTargetedNode != NULL) { // check if targeted node is a unit
			if (mUnitHash[i]->getNodePtr() == mTargetedNode) {
				if (mUnitHash[i]->mForce != g.mOurForce) { // order selected units to attack this unit
					orderAttackVehicle(i);
				} else if (mSelectedVehicleIndex >= 0) { // order selected units to board or follow this unit
					if (mUnitHash[mSelectedVehicleIndex] != NULL) {
						if (!orderBoardVehicle(mSelectedVehicleIndex, i)) {
							// TODO: orderFollowVehicle(i);
						}
					}
				}
				mTargetedNode = NULL;	// reset targeted node
			}
		}
		if (mUnitHash[i]->mGotPrimaryTarget) {
			mLinesManager.hide3DLine(0); // stop displaying 3D lines
			mLinesManager.hide3DLine(1); // stop displaying 3D lines
			int targetIndex = mUnitHash[i]->mPrimaryTargetIndex;
			if (!alreadySpottedTarget) {
				mUnitHash[i]->mCanSeeTarget = canSeeTarget(i, targetIndex);
			}
			
			if (mUnitHash[i]->mCanSeeTarget) { // check for line of fire
				Vector3 location;
				mUnitHash[targetIndex]->getPosition(location.x, location.y, location.z); // update the position of the target
				mUnitHash[i]->mPrimaryTarget = location; // update the position of the target
				if (mUnitHash[i]->mFiring) {
					if (!resolveCombat(i, targetIndex)) {
						printf("ERROR:  UnitHandler::updateVehicles() could not resolve combat\n");
						return false;
					}
				}
			} else {
				//printf("db: NOT firing at target\n");
			}
		}
		updateSelection(i); // 4.) check if it might have been selected/deselected
	} // end of big unitHash loop
	return true;
}

void UnitHandler::editorSelectionUpdateLoop() {
	for (int i = 0; i < mUnitHashEnd; i++) {
		updateSelection(i, true);
	}
}

int UnitHandler::getUnitMatchingNode(Ogre::SceneNode* node) {
	for (int i = 0; i < mUnitHashEnd; i++) {
		if (mUnitHash[i] == NULL) {
			continue;
		}
		if (mUnitHash[i]->getNodePtr() == node) {
			return i;
		}
	}
	return -1;
}

void UnitHandler::updateSelection(int uhIndex, bool editorMode) {
	if (mSelectedNode != NULL) {
		if (mUnitHash[uhIndex]->getNodePtr() == mSelectedNode) { // it's a match - so invert the selection
			bool selected = mUnitHash[uhIndex]->isSelected();
			if (selected == false) {
				mSelectedNode = NULL; // this line is redundant
				deselectAll();
				mUnitHash[uhIndex]->setSelected(true);
				mSelectedVehicleIndex = uhIndex;
			} else { // this means we clicked on this unit but it was already selected - so deselect it (it was prev. 'selected)
				mUnitHash[uhIndex]->setSelected(false);
				mSelectedVehicleIndex = -1;
				if (!editorMode) {
					mLinesManager.hide3DLine(0); // stop displaying 3D lines
					mLinesManager.hide3DLine(1); // stop displaying 3D lines
					mLinesManager.hide3DLine(2); // stop displaying 3D lines
					mLinesManager.hide3DLine(3); // stop displaying 3D lines
					mLinesManager.hide3DLine(4);
				}
			}
		}
	}
}

// update guis and things for the vehicle given by index
bool UnitHandler::updateSelectedVehicle(int index) {
	if (g.mIsPaused) {
		return true;
	}
	g.mLineGraphWidgetA.removeAllPointsFromLine(3); // clear old indicator line
	g.mLineGraphWidgetA.setXAxisMax(1600.0f);
	g.mLineGraphWidgetA.addPointToLine(3, mUnitHash[index]->mSquaredDistToNavTarget, 0.0f); // get distance to target from selected vehicle
	g.mLineGraphWidgetA.addPointToLine(3, mUnitHash[index]->mSquaredDistToNavTarget, 1.0f); // get distance to target from selected vehicle
	g.mFuzzyObstDistGraph.removeAllPointsFromLine(3); // clear old indicator line
	g.mFuzzyObstDistGraph.setXAxisMax(1600.0f);
	g.mFuzzyObstDistGraph.addPointToLine(3, mUnitHash[index]->mSquaredDistToObstacle, 0.0f); // get distance to obstacle from selected vehicle
	g.mFuzzyObstDistGraph.addPointToLine(3, mUnitHash[index]->mSquaredDistToObstacle, 1.0f);
	g.mLineGraphWidgetB.removeAllPointsFromLine(3); // clear old indicator line
	g.mLineGraphWidgetB.setXAxisMax(0.67f);
	double angle = mUnitHash[index]->mAngleToNavTarget;
	if (angle > M_PI) { // convert left angles to right equivalents
		angle = M_PI - (angle - M_PI);
	}
	g.mLineGraphWidgetB.addPointToLine(3, angle, 0.0f); // get distance to target from selected vehicle
	g.mLineGraphWidgetB.addPointToLine(3, angle, 1.0f); // get distance to target from selected vehicle
	g.mFuzzyObstAngleGraph.removeAllPointsFromLine(3); // clear old indicator line
	g.mFuzzyObstAngleGraph.setXAxisMax(0.67f);
	angle = mUnitHash[index]->mAngleToObstacle;
	if (angle > M_PI) { // convert left angles to right equivalents
		angle = M_PI - (angle - M_PI);
	}
	g.mFuzzyObstAngleGraph.addPointToLine(3, angle, 0.0f); // get distance to obstacle from selected vehicle
	g.mFuzzyObstAngleGraph.addPointToLine(3, angle, 1.0f); // get distance to obstacle from selected vehicle
	g.mLineGraphWidgetC.addPointToLine(0, ((float)(g.mSimTimer.getMicroseconds() - g.mTotalTimePaused)) / 1000000.0f, mUnitHash[index]->getCurrentDriveSpeedKPH(), false, true);
	g.mDefuzzifiedSteeringGraph.addPointToLine(0, ((float)(g.mSimTimer.getMicroseconds() - g.mTotalTimePaused)) / 1000000.0f, mUnitHash[index]->mHeading, false, true);
	return true;
}

// test if in direct line of sight of target (true/false)
bool UnitHandler::canSeeTarget(int ourVeh, int targetVeh) {
	if ((ourVeh < 0) || (ourVeh >= mUnitHashEnd) || (targetVeh < 0) || (targetVeh >= mUnitHashEnd)) {
		printf("FATAL ERROR: vehicle indices out of bounds in UnitHandler::canSeeTarget()\n");
		exit(1);
	}
	Vector3 ourPos;
	mUnitHash[ourVeh]->getPosition(ourPos.x, ourPos.y, ourPos.z);
	Vector3 targetPos;
	mUnitHash[targetVeh]->getPosition(targetPos.x, targetPos.y, targetPos.z);
	AxisAlignedBox waabb = mUnitHash[ourVeh]->getNodePtr()->_getWorldAABB();
	Vector3 max = waabb.getMaximum();
	ourPos.y = max.y;
	targetPos.y += 3.0f; // add 2 meters
	Vector3 terrainPos;
	terrainPos = targetPos;
	if (pTerrainHandler->isTerrainBetween(ourPos, targetPos, terrainPos)) { // check if terrain is in way
		if (mUnitHash[ourVeh]->isSelected()) {
			mLinesManager.hide3DLine(2); // hide 3D lines
			mLinesManager.show3DLineFromHereToThere(3, ourPos, terrainPos);
			mLinesManager.show3DLineFromHereToThere(4, ourPos, targetPos);
		}
		return false;
	}
	Vector3 direction;
	direction.x = targetPos.x - ourPos.x;
	direction.y = targetPos.y - ourPos.y;
	direction.z = targetPos.z - ourPos.z;
	direction.normalise();
	Ray ourRay(ourPos, direction); // create a ray
	pRSQ->setRay(ourRay);
	RaySceneQueryResult &result = pRSQ->execute(); // execute query
	RaySceneQueryResult::iterator itr = result.begin();
	for (itr = result.begin(); itr != result.end(); itr++) { // Get results
		if ((itr->movable && itr->movable->getName().substr(0,5) == "shrub") || (itr->movable && itr->movable->getName().substr(0,5) == "build")) { // building or tree in the way
			Vector3 thingyPos = itr->movable->getParentSceneNode()->getPosition();
			if (ourPos.distance(targetPos) > ourPos.distance(thingyPos)) {
				if (mUnitHash[ourVeh]->isSelected()) {
					mLinesManager.hide3DLine(2); // hide 3D lines
					mLinesManager.show3DLineFromHereToThere(3, ourPos, thingyPos);
					mLinesManager.show3DLineFromHereToThere(4, ourPos, targetPos);
				}
				return false;
			}
		}
		if (itr->movable && itr->movable->getName().substr(0,5) == "vehic") { // check if other vehicles are in way
			if ((itr->movable->getParentSceneNode() != mUnitHash[ourVeh]->getNodePtr()) && (itr->movable->getParentSceneNode() != mUnitHash[targetVeh]->getNodePtr())) {
				Vector3 thingyPos = itr->movable->getParentSceneNode()->getPosition();
				if (ourPos.distance(targetPos) > ourPos.distance(thingyPos)) {
					if (mUnitHash[ourVeh]->isSelected()) {
						mLinesManager.hide3DLine(2); // hide 3D lines
						mLinesManager.show3DLineFromHereToThere(3, ourPos, thingyPos);
						mLinesManager.show3DLineFromHereToThere(4, ourPos, targetPos);
					}
					return false;
				}
			}
		}
	}
	if (mUnitHash[ourVeh]->isSelected()) {
		mLinesManager.hide3DLine(3); // hide 3D lines
		mLinesManager.hide3DLine(4); // hide 3D lines
		mLinesManager.show3DLineFromHereToThere(2, ourPos, targetPos);
	}
	return true;
}

bool UnitHandler::resolveCombat(int attacker, int defender) {
	if (g.mIsClient) {
		return true; // go away and wait for instructions
	}
	if (calculateToHit(attacker, defender)) { // hit target - do something fantastic
		if (armourPenetrationTest(attacker, defender)) {
			//printf("UnitHandler DB: target hit, and penetrated\n");
			if (!destroyVehicle(defender, attacker)) { // BOOM!
				printf("ERROR: UnitHandler::resolveCombat Could not destroy vehicle\n");
				return false;
			}
			if (g.mIsServer) {
				Vector3 pos;
				mUnitHash[defender]->getPosition(pos.x, pos.y, pos.z);
				double yr = mUnitHash[defender]->mHeading;
				g.mServer.sendOrderDestroyVehicle(defender, attacker, pos.x, pos.y, pos.z, yr);
			}
		} else {
			//printf("UnitHandler DB: target hit, but armour NOT penetrated\n");
			// TODO play sound (and in client update)
			if (g.mIsServer) {
				Vector3 pos;
				mUnitHash[defender]->getPosition(pos.x, pos.y, pos.z);
				double yr = mUnitHash[defender]->mHeading;
				g.mServer.sendOrderRicochet(defender, pos.x, pos.y, pos.z, yr);
			}
		}
	} else { // missed
		Vector3 missLocation;
		mUnitHash[defender]->getPosition(missLocation.x, missLocation.y, missLocation.z);
		missLocation.x = missLocation.x - 10.0f + (double)(rand() % 20);
		missLocation.y = missLocation.y + 10.0f;
		missLocation.z = missLocation.z - 10.0f + (double)(rand() % 20);
		double muzzleVel = 0.0005; // HACK exchange hardcoded velocity with dynamic
		Vector3 direction;
		direction.x = direction.z = 0.0f;
		direction.y = -1.0f;
		if (!createProjectile(missLocation, muzzleVel, direction)) {
			printf("ERROR: UnitHandler::resolveCombat Could not create projectile upon firing\n");
			return false;
		}
		if (g.mIsServer) {
			Vector3 pos;
			mUnitHash[defender]->getPosition(pos.x, pos.y, pos.z);
			g.mServer.sendOrderCreateProjectile(pos.x, pos.y, pos.z);
		}
	}
	return true;
}

// pseduo-rand test to see if we will hit a target. returns true if hit
bool UnitHandler::calculateToHit(int firingVeh, int targetVeh) {
	if ((firingVeh < 0) || (firingVeh >= mUnitHashEnd) || (targetVeh < 0) || (targetVeh >= mUnitHashEnd)) { // validate params
		printf("FATAL ERROR: vehicle indices out of bounds in UnitHandler::calculateToHit()\n");
		exit(1);
	}
	Vector3 start;
	mUnitHash[firingVeh]->getPosition(start.x, start.y, start.z);
	Vector3 end;
	mUnitHash[targetVeh]->getPosition(end.x, end.y, end.z);
	double squaredDistance = start.squaredDistance(end); // get sqd distance
	double baseAccuracy = 0.0f;
	if (squaredDistance < 10000.0f) {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[0];
	} else if (squaredDistance < 250000.0f) {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[1];
	} else if (squaredDistance < 1000000.0f) {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[2];
	} else if (squaredDistance < 2250000.0f) {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[3];
	} else if (squaredDistance < 4000000.0f) {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[4];
	} else if (squaredDistance < 6250000.0f) {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[5];
	} else {
		baseAccuracy = mUnitHash[firingVeh]->pSpecPtr->mCannon.mAccuracy[6];
	}
//	printf("DEBUG: sqd dist to target vehicle = %.4f\n", squaredDistance);
//	printf("DEBUG: base accuracy = %.2f\n", baseAccuracy);
	double coinflip = (double)(rand() % 100);
	//printf("DEBUG: coinflip = %f\n", coinflip);
	double sizeMod = mUnitHash[targetVeh]->pSpecPtr->mHull.width_m / 2.5f;
	// TODO make an area in stead - using width or length depending on facing, and waabb max.y
	double adjustedAccuracy = baseAccuracy * sizeMod;
	//printf("DEBUG: size adjusted accuracy = %.2f\n", adjustedAccuracy);
	double speedMod = mUnitHash[targetVeh]->getCurrentDriveSpeedKPH() / 20.0f;
	adjustedAccuracy = adjustedAccuracy * (1 - speedMod);
	//printf("DEBUG: speed adjusted accuracy = %.2f\n", adjustedAccuracy);
	if (coinflip < adjustedAccuracy) {
		return true; // hit
	}
	return false; // miss
}

// test if projectile penetrates armour (true/false)
bool UnitHandler::armourPenetrationTest(int firingVeh, int hitVeh) {
	if ((firingVeh < 0) || (firingVeh >= mUnitHashEnd) || (hitVeh < 0) || (hitVeh >= mUnitHashEnd)) {
		printf("FATAL ERROR: vehicle indices out of bounds in UnitHandler::armourPenetrationTest()\n");
		exit(1);
	}
	VehicleSpec* hitSpec = mUnitHash[hitVeh]->pSpecPtr; // get specs
	Vector3 start;
	mUnitHash[firingVeh]->getPosition(start.x, start.y, start.z); // get firing location
	Vector3 end;
	mUnitHash[hitVeh]->getPosition(end.x, end.y, end.z); // get firing location
	double squaredDistance = start.squaredDistance(end); // get sqd distance
	double basePenetration = interpolatePenetration_mm(squaredDistance, firingVeh);
	//printf("DB: statedPenat60: %.2f\n", basePenetration);
	basePenetration = basePenetration / sinf(0.33333333f * M_PI);
	//printf("DB: calced to flat: %.2f\n", basePenetration);
	double adjustedPenetration = basePenetration - 0.10f * basePenetration; // subtract 10% so we can do+-10%
	double coinflip = (double)(rand() % 20) / 100.0f; // execute randomness
	adjustedPenetration = adjustedPenetration + coinflip * basePenetration; // 20% random
	double hitVehHeading = mUnitHash[hitVeh]->mHeading; // get which way target was facing
	// work out angle tank hit would have to turn to face firing tank
	double opposite = start.x - end.x; // check facing
	double adjacent = start.z - end.z;  // check facing
	double theta = atan2f(opposite, adjacent); // Calculate Angle from hit tank to firing tank
	normaliseRadian(theta);
	normaliseRadian(hitVehHeading);
	double range = hitVehHeading - theta;
	normaliseRadian(range);
	//printf("DB: angle between (ANT) = %.2f\n", range);
	// work out which quadrant of tank was hit
	double quadrantAngle = atan2f(hitSpec->mHull.width_m, hitSpec->mHull.length_m); // get angles of each side
	double armour = 1.0f;
	double slope = 0.0f;
	if ((range < quadrantAngle) || (range > (2.0f * M_PI - quadrantAngle))) {
		armour = hitSpec->mHull.mArmour.mFront_mm;
	//	printf("DB: HIT FRONT w %.2fmm%.2frad\n", armour, hitSpec->mHull.mArmour.mFrontSlope_rad);
		slope = hitSpec->mHull.mArmour.mFrontSlope_rad;
		armour = armour / sinf(slope); // work out effective mm
		//printf("DB: effective thickness = %.2fmm\n", armour);
	} else if (range < M_PI - quadrantAngle) {
		armour = hitSpec->mHull.mArmour.mSide_mm;
		//printf("DB: HIT RIGHT SIDE w %.2fmm%.2frad\n", armour, hitSpec->mHull.mArmour.mSideSlope_rad);
		slope = hitSpec->mHull.mArmour.mSideSlope_rad;
		armour = armour / sinf(slope); // work out effective mm
		//printf("DB: effective thickness = %.2fmm\n", armour);
	} else if (range < M_PI + quadrantAngle) {
		armour = hitSpec->mHull.mArmour.mRear_mm;
		//printf("DB: HIT REAR w %.2fmm%.2frad\n", armour, hitSpec->mHull.mArmour.mRearSlope_rad);
		slope = hitSpec->mHull.mArmour.mRearSlope_rad;
		armour = armour / sinf(slope); // work out effective mm
		//printf("DB: effective thickness = %.2fmm\n", armour);
	} else {
		armour = hitSpec->mHull.mArmour.mSide_mm;
		//printf("DB: HIT LEFT SIDE w %.2fmm@%.2frad\n", armour, hitSpec->mHull.mArmour.mSideSlope_rad);
		slope = hitSpec->mHull.mArmour.mSideSlope_rad;
		armour = armour / sinf(slope); // work out effective mm
		//printf("DB: effective thickness = %.2fmm\n", armour);
	}
	//printf("DB: range: %.2f, basePen: %.2f, coin: %.2f, adjPen: %.2f, arm: %.2f\n", squaredDistance, basePenetration, coinflip, adjustedPenetration, armour);
	if (adjustedPenetration > armour) {
		//printf("DB: penetration YES\n");
		if (ricochetTest(adjustedPenetration, armour, slope, range)) {
			//printf("DB: But ricoche\n");
			return false;
		}
		return true;
	}
	//printf("DB: penetration NO\n");
	return false; // no penetration
}

// test if projectile ricochets off sloped armour
bool UnitHandler::ricochetTest(double effectivePen_mm, double effectiveThick_mm, double slope_rad, double glancing_rad) {
	double ninetyDeg = M_PI / 2.0f; // 90 deg in rads (was sick of typing this)
	double slopeTest = (ninetyDeg - slope_rad) / ninetyDeg; // 0 deg = 100% chance, 90 deg = 0% chance
	double glanceFrac = fabs(sinf(2 * glancing_rad)); // 0-1
	double angleCombined = (slopeTest * 1.0f + glanceFrac * 0.5) / 2.0f; // weighted avg. glance is * 0.5 cos its only equiv in range to 90-45degrees
	double penetrationFactor = effectivePen_mm / effectiveThick_mm; // number of plates this size punched
	double chance = angleCombined * (1.0f / penetrationFactor) * 100.0f; // 0-1 / 0-x
	double coinflip = (double)(rand() % 100); // execute randomness
	//printf("DB: pen %.2f, thi %.2f, slo %.2f\n", effectivePen_mm, effectiveThick_mm, slope_rad);
	//printf("DB: slopeT %.2f, glanceF %.2f, combinedA %.2f, penF %.2f\n", slopeTest, glanceFrac, angleCombined, penetrationFactor);
	//printf("DB: coinflip %.2f, chance %.2f\n", coinflip, chance);
	if (coinflip < chance) {
		return true; // zoooppeeeoooooowww..........
	}
	return false; // bung...arrgghhH!!!
}

// work out effective mm of armour pen based on given stats and some averaging
double UnitHandler::interpolatePenetration_mm(double squaredDistance, int firingVehIndex) {
	if ((firingVehIndex < 0) || (firingVehIndex >= mUnitHashEnd)) {
		printf("FATAL ERROR: vehicle indices out of bounds in UnitHandler::interpolatePenetration()\n");
		exit(1);
	}
	VehicleSpec* fireSpec = mUnitHash[firingVehIndex]->pSpecPtr; // get specs
	double basePenetration = 0.0f;
	if (squaredDistance < 10000.0f) { // look up table for penetration at this range
		basePenetration = fireSpec->mCannon.mPenetration[0]; // constant if < 100m
	} else if (squaredDistance < 250000.0f) { // 500m
		double rangeBracket = 250000.0f - 10000.0f;
		double topPen = fireSpec->mCannon.mPenetration[1];
		double bottomPen = fireSpec->mCannon.mPenetration[0];
		double penDiff = topPen - bottomPen;
		double fracRange = (squaredDistance - 10000.0f) / rangeBracket;
		basePenetration = topPen - fracRange * penDiff;
	} else if (squaredDistance < 1000000.0f) { // 1km
		double rangeBracket = 1000000.0f - 250000.0f;
		double topPen = fireSpec->mCannon.mPenetration[2];
		double bottomPen = fireSpec->mCannon.mPenetration[1];
		double penDiff = topPen - bottomPen;
		double fracRange = (squaredDistance - 250000.0f) / rangeBracket;
		basePenetration = topPen - fracRange * penDiff;
	} else if (squaredDistance < 2250000.0f) { // 1.5km
		double rangeBracket = 2250000.0f - 1000000.0f;
		double topPen = fireSpec->mCannon.mPenetration[3];
		double bottomPen = fireSpec->mCannon.mPenetration[2];
		double penDiff = topPen - bottomPen;
		double fracRange = (squaredDistance - 1000000.0f) / rangeBracket;
		basePenetration = topPen - fracRange * penDiff;
	} else if (squaredDistance < 4000000.0f) { // 2km
		double rangeBracket = 4000000.0f - 2250000.0f;
		double topPen = fireSpec->mCannon.mPenetration[4];
		double bottomPen = fireSpec->mCannon.mPenetration[3];
		double penDiff = topPen - bottomPen;
		double fracRange = (squaredDistance - 2250000.0f) / rangeBracket;
		basePenetration = topPen - fracRange * penDiff;
	} else if (squaredDistance < 6250000.0f) {
		double rangeBracket = 6250000.0f - 4000000.0f;
		double topPen = fireSpec->mCannon.mPenetration[5];
		double bottomPen = fireSpec->mCannon.mPenetration[4];
		double penDiff = topPen - bottomPen;
		double fracRange = (squaredDistance - 4000000.0f) / rangeBracket;
		basePenetration = topPen - fracRange * penDiff;
	} else {
		basePenetration = fireSpec->mCannon.mPenetration[6];
	}
	return basePenetration;
}

// destroy a vehicle given by index
bool UnitHandler::destroyVehicle(int index, int killersIndex) {
	if ((index < 0) || (index >= mUnitHashEnd)) {
		printf("ERROR: vehicle indices out of bounds in UnitHandler::destroyVehicle()\n");
		return false;
	}
	if (mUnitHash[index]->mDestroyed) {
		return true;
	}
	mUnitHash[index]->die(); // tell the unit to kill itself
	if (mUnitHash[index]->mForce < 10) {
		g.mScores[mUnitHash[index]->mForce].mUnitsRemaining--;
		//printf("db: unit dest, units rem %i\n", g.mScores[mUnitHash[index]->mForce].mUnitsRemaining);
		if (g.mScores[mUnitHash[index]->mForce].mUnitsRemaining == 0) {
			g.mGameOver = true;
		}
	}
	char tempPartName[128];
	sprintf(tempPartName, "%iEffectNodeExp", index);
	SceneNode* effectNode = mUnitHash[index]->getNodePtr()->createChildSceneNode(String(tempPartName) + "Node"); // create a new node for each one
	ParticleSystem* particleSystem = pSceneManager->createParticleSystem(tempPartName, "particle/tank_explosion2");
	effectNode->attachObject(particleSystem);
	effectNode->setInheritOrientation(false);
	particleSystem->getEmitter(0)->setEnabled(true);
	sprintf(tempPartName, "%iEffectNodeBSm", index);
	particleSystem = pSceneManager->createParticleSystem(tempPartName, "Particle/BurningSmoke");
	effectNode->attachObject(particleSystem);
	effectNode->setInheritOrientation(false);
	particleSystem->getEmitter(0)->setEnabled(true);
	if (mUnitHash[killersIndex]->mForce < 10) {
		g.mScores[mUnitHash[killersIndex]->mForce].mUnitKills++;
	}
	return true;
}

bool UnitHandler::moveSelectedVehicleTo(const double& x, const double& y, const double& z) {
	if (!mUnitHash[mSelectedVehicleIndex]) {
		printf("ERROR selected vehicle index does not exist - UnitHandler::moveSelectedVehicleTo\n");
		return false;
	}
	if (!mUnitHash[mSelectedVehicleIndex]->mIsRidingOtherUnit) {
		mUnitHash[mSelectedVehicleIndex]->setPosition(x, y, z);
	}
	return true;
}

bool UnitHandler::editorRotateSelectedVehicle(const double& amount) {
	if (!mUnitHash[mSelectedVehicleIndex]) {
		printf("ERROR selected vehicle index does not exist - UnitHandler::editorRotateSelectedVehicle\n");
		return false;
	}
	mUnitHash[mSelectedVehicleIndex]->setHeading(mUnitHash[mSelectedVehicleIndex]->mHeading + amount);
	mUnitHash[mSelectedVehicleIndex]->balance();
	return true;
}

bool UnitHandler::editorDeleteSelection() {
	if (!mUnitHash[mSelectedVehicleIndex]) {
		printf("ERROR selected vehicle index does not exist - UnitHandler::editorDeleteSelection\n");
		return false;
	}
	int index = mSelectedVehicleIndex;
	deselectAll();
	Ogre::SceneNode* sn = NULL;
	sn = mUnitHash[index]->getNodePtr();
	if (!sn) {
		printf("ERROR: selected vehicle sn is NULL does not exist - UnitHandler::editorDeleteSelection\n");
		return false;
	}
	sn->setVisible(false);
	mUnitHash[index]->setPosition(10000.0, 10000.0, 10000.0);
	pSceneManager->destroySceneNode(sn);
	/*delete mUnitHash[index];
	mUnitHash[index] = NULL;
	if (index == mUnitHashEnd -1) {
		mUnitHashEnd--;
	}*/
	return true;
}

