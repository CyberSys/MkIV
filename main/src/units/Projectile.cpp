#include "Projectile.h"
#include "../globals.h"
#include <math.h>
using namespace std;

// default constructor - set vals to defaults
Projectile::Projectile() {
	pTerrainHandler = NULL;
	pNode = NULL;
	pDirtSplash = NULL;
	mScaleFactor = 1.0f;
	mCurrentAirVelocity.x = 0.0f;
	mCurrentAirVelocity.y = 0.0f;
	mCurrentAirVelocity.z = 0.0f;
	mOrientation.x = 0.0f;
	mOrientation.y = 0.0f;
	mOrientation.z = 0.0f;
	mLifetime = 16000000; // set lifetime to 4 seconds (2kms for a 500m/s projectile)
	mExplosionCountdown = 250000; // 1/4 second explosion
	mIsDead = false;
	mExploding = false;
}

// initialise projectile with some basic values
bool Projectile::init(TerrainHandler *terrHdlr, SceneNode* node, Vector3 pos, float muzzVel, Vector3 gunOrientation) {
	if (!terrHdlr) {
		printf("ERROR: projectile could not be init due to null terrHdlr\n");
		return false;
	}
	if (!node) {
		printf("ERROR: projectile could not be init due to null scene node\n");
		return false;
	}
	// init from parameters
	pTerrainHandler = terrHdlr;	// pointer to the Terrain Handler
	pNode = node;	// pointer to scene node

//	pos.y = pos.y + 5.0f;	// HACK add 5m height until can get actual muzzle velocity

	pNode->setPosition(pos); // put in a position in the world
	mCurrentAirVelocity = gunOrientation * muzzVel;	// set initial velocity

	// orient the mesh by getting it's node to 'look at' a position in the direction of travel
	Vector3 currentDestination = pos + mCurrentAirVelocity * 1000.0f; // after testing the * 1000 factor makes more accurate
	pNode->lookAt(currentDestination, Node::TS_WORLD);

	return true;
}

// give ptr to a particle system to the projectile
bool Projectile::setExplosionController(ParticleSystem* systemPtr) {
	if (!systemPtr) {
		printf("ERROR: in call setExplosionController(system) - system was NULL\n");
		return false;
	}
	pDirtSplash = systemPtr;
	pDirtSplash->getEmitter(0)->setEnabled(false);

	return true;
}

// update various things
bool Projectile::update(unsigned long &elapsed) {
	if (!pNode) {
		printf("ERROR: Node uninitialised, can not update projectile\n");
		return false;
	}

	// subtract time from explosion
	if (mExploding) {
		if (mExplosionCountdown <= 0) {
			mExploding = false;
			pDirtSplash->getEmitter(0)->setEnabled(false);
		} else {
			mExplosionCountdown -= elapsed;
		}
	}

	if (mIsDead) {
		return true; // TODO ACTUALLY kill projectile instead
	}

	// update lifetime
	if (mLifetime <= 0) {
		// TODO kill particle and remove node from render queue
		mIsDead = true;
		return true;
	}
	mLifetime = mLifetime - elapsed;

	// keep moving the Projectile
	Vector3 ownPos = pNode->getPosition(); // get own position components (straight path)
	
	// change downward speed due to gravity (parabolic path)
	// Assuming gravity = -10m/s/s, which is -10m/1,000,000us every 1,000,000us
	// this is equivalent to -0.00001m/us every 1,000,000us which is equivalent to:
//	mCurrentAirVelocity.y = mCurrentAirVelocity.y - 0.00001f * (elapsed / 1000000.0f);
	
	// TODO: change x and z velocity components due to air friction (dented parabolic path)
	// TODO: add wind speed to velocity (3-dimensional dented parabolic path)
	// NOTE: local wind speeds would make this an (irregular 3d dented parabolic path)

	ownPos.x = ownPos.x + mCurrentAirVelocity.x * elapsed; // advance position of projectile
	ownPos.y = ownPos.y + mCurrentAirVelocity.y * elapsed; // advance position of projectile
	ownPos.z = ownPos.z + mCurrentAirVelocity.z * elapsed; // advance position of projectile
	pNode->setPosition(ownPos);

	// orient the mesh by getting it's node to 'look at' a position in the direction of travel
	Vector3 currentDestination = ownPos + mCurrentAirVelocity * 1000.0f;
	pNode->lookAt(currentDestination, Node::TS_WORLD);

	// check against terrain for collision
	Vector3 pos = pNode->getPosition(); // get position of projectile
	float terrainHeight = pTerrainHandler->getTerrainHeightAt(pos.x, pos.z); // get height of terrain at pos of projectile
	if (terrainHeight > pos.y) { // check if projectile under earth i.e. has collided
			// TODO self destruct
			mIsDead = true;
			// start particles for dirt explosion
			if (pDirtSplash != NULL) {
				pDirtSplash->getEmitter(0)->setEnabled(true);
				mExploding = true;
				mExplosionCountdown = 250000;
			}

			// deform terrain
			pTerrainHandler->setPointerPosition(pos);
			// do larger hill first to make a crater rim
			pTerrainHandler->createEditBrush(2); // set size of crater (TODO although this loads a new image file, which is dumb).
			pTerrainHandler->deform(20000, true); // crater of depth 1000
			// now do the hole in the middle
			pTerrainHandler->createEditBrush(1); // set size of crater (TODO although this loads a new image file, which is dumb).
			pTerrainHandler->deform(20000, false); // crater of depth 1000
			// paint scorch marks
			pTerrainHandler->createEditBrush(2);
			pTerrainHandler->setCurrentPaint(6);
			pTerrainHandler->paint(500000, true);
	}

	return true;
}

// get the world x,y,z position of the object
Vector3 Projectile::getPosition() {
	if (!pNode) {
		printf("ERROR: Node uninitialised, can not update projectile\n");
	}
	// get X,Y,Z position of vehicle (in metres)
	return pNode->getPosition();
}

// change the world x,y,z position of the object
void Projectile::setPosition(Vector3& pos) {
	if (!pNode) {
		printf("ERROR: Node uninitialised, can not update projectile\n");
	}
	// set X,Y,Z position of vehicle (in metres)
	pNode->setPosition(pos);
}


// changes the absolute x-axis rotation of the object
void Projectile::setPitch(float xRot) {
	// adjust so is between 0 and 2Pi
	while (xRot < 0.0f) {
		xRot += 2.0f * M_PI;
	}
	while (xRot >= 2.0f * M_PI) {
		xRot -= 2.0f * M_PI;
	}
	// set internal copy
	mOrientation.x = xRot;
}

// returns the absolute x-axisi rotation of the object
float Projectile::getPitch() {
	return mOrientation.x;
}

// changes the absolute y-axis rotation of the object
void Projectile::setHeading(float yRot) {
	// adjust so is between 0 and 2Pi
	while (yRot < 0.0f) {
		yRot += 2.0f * M_PI;
	}
	while (yRot >= 2.0f * M_PI) {
		yRot -= 2.0f * M_PI;
	}
	// set internal copy
	mOrientation.y = yRot;
}

// returns the absolute y-axis rotation of the object
float	Projectile::getHeading() {
	return mOrientation.y;
}

// get a pointer to the 3d object
SceneNode* Projectile::getNode() {
	return pNode;
}

