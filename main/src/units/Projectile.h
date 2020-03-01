#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "../scenery/TerrainHandler.h"

// class to represent artillery shells etc.
class Projectile {
public:
	Projectile();
	bool init(TerrainHandler *terrHdlr, SceneNode* node, Vector3 pos, float muzzVel, Vector3 gunOrientation);
	bool update(unsigned long &elapsed);

	SceneNode* getNode();
	Vector3 getPosition();
	float getPitch();
	float getHeading();
	bool setExplosionController(ParticleSystem* systemPtr);
	void setPosition(Vector3& position);
	void setPitch(float xRot);
	void setHeading(float yRot);
private:
	TerrainHandler *pTerrainHandler; // pointer to terrain (so we can check if we have hit it)
	SceneNode* pNode; // node the model is attached to
	ParticleSystem* pDirtSplash; // particle system for explosion

	// dynamic projectile data
	Vector3 mCurrentAirVelocity;
	Vector3 mOrientation;
	float mScaleFactor; // factor to scale the mesh by
	long mLifetime;
	long mExplosionCountdown;
	bool mIsDead; // TODO TEMP var.
	bool mExploding; // exploding
};

#endif /* PROJECTILE_H */
