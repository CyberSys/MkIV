#ifndef INFANTRY_H_
#define INFANTRY_H_

#include "Unit.h"

// class defines instances of land vehicles
struct Infantry: public Unit {
	Vector3 getWaypoint(int index);
	bool balance(); // polymorphism
	bool recalculatePath(); // recalculate path to final destination
	bool update(unsigned long& elapsed); // update various things - animations etc (things to do every frame).
	void performRotation(); // auto orient
	void startMovementParticleEffects();
	void stopMovementParticleEffects();
};

#endif /*INFANTRY_H_*/

