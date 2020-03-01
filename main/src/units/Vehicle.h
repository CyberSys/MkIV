#ifndef VEHICLE_H_
#define VEHICLE_H_

#include "Unit.h"

// class defines instances of land vehicles
struct Vehicle : public Unit {
	Vector3 getWaypoint(int index);
	bool bobUpAndDown();
	bool balance(); // balance the vehicle on the terrain
	bool recalculatePath(); // recalculate path to final destination
	bool update(unsigned long& elapsed); // update various things - animations etc (things to do every frame).
	void performRotation(); // auto orient vehicle
	void startMovementParticleEffects();
	void stopMovementParticleEffects();
};

#endif /*VEHICLE_H_*/

