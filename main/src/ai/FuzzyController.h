#ifndef _ANT_FUZZY_CONTROLLER_H_
#define _ANT_FUZZY_CONTROLLER_H_

/*
	Functionality to control Fuzzy Target-Seeking	and Fuzzy Obstacle-Avoidance.
    C++ Adaption of algorithm and VB code from
	"Unmanned30.4-Scaled_Waypoint-DynamicObstacles-NZ"

	Original VB code - Aug.4,2004-Dec.2,2005 by Dr. Napoleon Reyes
	First C++ revision - December 2005 by Anton Gerdelan for Evo3 project
	Second C++ revision - April 2008 by Anton Gerdelan for MarkIV project
	Traffic simulation adaptation - March 2009 by Anton Gerdelan for Metropolis project
*/

//********************************//NOTES
// For other types of vehicle:
// Output is the SAME
// each vehicle has a top speed and top rot speed
// if either of these is exceeded by o.a then we have an OA percentage
// if either is exceeded by t.s. then we have a TS percentage (modifier of output speeds)
// apply to both o/p angle AND o/p Speed (so turn radius is the same)

// Definitions of Fuzzy Input Set Trapezoids
// a,b for vertices of left or right side trapezoids and
// a,b,c,d for central trapezoids

/* Notes on scale conversion:
	1. I assumed Napoleon's top speed of 5.5cm/time was equiv to my 0.15cm/time
	2. Therefore Time constant Nap->Anton is:
		0.027273
	3. Conversion of degrees to radians constant is
		0.017453
*/

// Values adjusted for % of top speeds

/****************************************************************************
NOTE if can not use same i/p set definitions for obstacle and target then
just use class twice
****************************************************************************/

#include "Chromosome.h"
#define NUM_SPEED_SETS 6
#define NUM_STEER_SETS 7

class FuzzyController {
public:
	FuzzyController();
	bool readSetsFromFile(const char* filename);
	bool readRulesFromFile(const char* filename); // read FAMM rules from file (returns true if successful)
	void clear();
	void processTargetSeeking(float in_targetangle, float in_targetdistance);
	void processObstacleAvoidance(float in_obstangle, float in_obstdistance);
	float getTargetSeekingSpeed();
	float getTargetSeekingAngle();
	float getObstacleAvoidanceSpeed();
	float getObstacleAvoidanceAngle();
	float getObstacleAngleThreshold();
	void getAngleInputMemberships(float& small, float& medium, float& large);
	void getDistInputMemberships(float& near, float& far, float& vfar);
	void getSpeedOutputMemberships(float& vslow, float& slow, float& med, float& fast, float& vfast, float& wfast);
	void getSteerOutputMemberships(float& zer, float& vli, float& lig, float& mod, float& sha, float& vsh, float& max);

	// set definitions
	float mInTargetDistanceNear[2];
	float mInTargetDistanceFar[4];
	float mInTargetDistanceVeryFar[2];
	float mInTargetAngleSmall[2];
	float mInTargetAngleMedium[4];
	float mInTargetAngleLarge[2];
	float mInTargetAngleThreshold;
	float mOutTargetSpeedMids[6];
	float mOutTargetAngleMids[7];

	float mInObstacleDistanceNear[2];
	float mInObstacleDistanceFar[4];
	float mInObstacleDistanceVeryFar[2];
	float mInObstacleAngleSmall[2];
	float mInObstacleAngleMedium[4];
	float mInObstacleAngleLarge[2];
	float mInObstacleAngleThreshold;
	float mOutObstacleSpeedMids[6];
	float mOutObstacleAngleMids[7];
	
	Chromosome mRuleBase;
private:
	bool UpdateTargetInputMemberships(float mIn_targetangle,float mIn_targetdistance);
	bool UpdateObstacleInputMemberships(float mIn_obstangle,float mIn_obstdistance);
	bool CalculateTargetOutputMemberships(); // Internal Sub-Functions
	bool CalculateObstacleOutputMemberships(); // Internal Sub-Functions
	float CalculateTargetSpeed(); // Internal Sub-Functions
	float CalculateTargetAngle(); // Internal Sub-Functions
	float CalculateObstacleSpeed(); // Internal Sub-Functions
	float CalculateObstacleAngle(); // Internal Sub-Functions
	
	float mInAngleSetMem[3]; // Fuzzy Input Set Memberships
	float mInDistSetMem[3]; // Fuzzy Input Set Memberships
	float mOutSpeedSetMem[NUM_SPEED_SETS]; // Fuzzy Output Set Memberships
	float mOutSteerSetMem[NUM_STEER_SETS]; // Fuzzy Output Set Memberships
	float mTargetSeeking_Speed; // Defuzzified Values
	float mTargetSeeking_Angle; // Defuzzified Values
	float mObstacleAvoidance_Speed; // Defuzzified Values
	float mObstacleAvoidance_Angle; // Defuzzified Values
};

#endif
