#ifndef GUI_UNITPANL_H_
#define GUI_UNITPANL_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

using namespace Ogre;

class UnitInfoPanel {
public:
	UnitInfoPanel(); // set defaults
	// META CONTROLS
	bool init(); // initialise memory
	bool free(); // free memory
	bool show(); // start drawing
	bool hide(); // stop drawing
	void setBasicMode(); // mode 0
	void setFuzzyNavMode(); // mode 1
	void setBallisticsMode(); // mode 2
	void setCrewMode(); // mode 3
	void cycleMode();

	// BASIC DETAILS (mode 0)
	bool updateUnitDetails(const char *desig, const char *name, int force, bool target, int tid);
	bool updateUnitOrder(int orders);

	// FUZZY NAV (mode 1)
	bool setNavInputs(float targSqD, float targRad, float obstSqD, float obstRad, bool obstOnLeft);
	bool setNavModes(bool tsMode, bool oaMode);
	bool setFuzzyInputMemsAngle(float small, float medium, float large);
	bool setFuzzyInputMemsDist(float near, float far, float vfar);
	bool setFuzzyOutputMemsSteer(float a, float b, float c, float d, float e);
	bool setFuzzyOutputMemsSpeed(float a, float b, float c, float d, float e, float f);
	bool setDefuzzifiedValues(float speed, float steering);

	// BALLISTICS (mode 2)
	bool updateUnitAimingDetails(float horizDist, float yDist, float pitchArc, float muzzleVelocity); // legacy update function

	// CREW (mode 3)
	bool updateCrewAction(int crewIndex, int action);
	bool resetCrewActions();
private:
	Overlay* mpUnitPanel; // panel displaying selected unit details
	int mPanelMode; // 'mode' of panel (which info set is being displayed)
};

#endif
