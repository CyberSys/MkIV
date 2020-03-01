#include "FuzzyController.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm> // for min() and max()
#include <string.h>
#include <fstream>
#include <math.h>

using namespace std;

#define PI 3.14159265358979323846 // TEMP value until globals can be included
#define TWO_PI 6.28318530717958647693 // TEMP value until globals can be included

FuzzyController::FuzzyController() {
	// reset target sets to 0
	mInTargetDistanceNear[0] = 0.0f;
	mInTargetDistanceNear[1] = 0.0f;
	for (int i = 0; i < 4; i++) {
		mInTargetDistanceFar[i] = 0.0f;
	}
	mInTargetDistanceVeryFar[0] = 0.0f;
	mInTargetDistanceVeryFar[1] = 0.0f;
	mInTargetAngleSmall[0] = 0.0f;
	mInTargetAngleSmall[1] = 0.0f;
	for (int i = 0; i < 4; i++) {
		mInTargetAngleMedium[i] = 0.0f;
	}
	mInTargetAngleLarge[0] = 0.0f;
	mInTargetAngleLarge[1] = 0.0f;
	mInTargetAngleThreshold = 0.0f;
	for (int i = 0; i < NUM_SPEED_SETS; i++) {
		mOutTargetSpeedMids[i] = 0.0f;
	}
	for (int i = 0; i < NUM_STEER_SETS; i++) {
		mOutTargetAngleMids[i] = 0.0f;
	}

	// reset obstacle sets to 0
	mInObstacleDistanceNear[0] = 0.0f;
	mInObstacleDistanceNear[1] = 0.0f;
	for (int i = 0; i < 4; i++) {
		mInObstacleDistanceFar[i] = 0.0f;
	}
	mInObstacleDistanceVeryFar[0] = 0.0f;
	mInObstacleDistanceVeryFar[1] = 0.0f;
	mInObstacleAngleSmall[0] = 0.0f;
	mInObstacleAngleSmall[1] = 0.0f;
	for (int i = 0; i < 4; i++) {
		mInObstacleAngleMedium[i] = 0.0f;
	}
	mInObstacleAngleLarge[0] = 0.0f;
	mInObstacleAngleLarge[1] = 0.0f;
	mInObstacleAngleThreshold = 0.0f;
	for (int i = 0; i < NUM_SPEED_SETS; i++) {
		mOutObstacleSpeedMids[i] = 0.0f;
	}
	for (int i = 0; i < NUM_STEER_SETS; i++) {
		mOutObstacleAngleMids[i] = 0.0f;
	}
	clear(); // clear outputs
}

// read in set function definitions from a file
bool FuzzyController::readSetsFromFile(const char* filename) {
	ifstream file;
	file.open(filename); // load file
	if (!file.is_open()) { // some basic validation
		printf("Error: Could not open fuzzy set file %s!\n",filename);
		return false; // fail
	}
	char line[256];
	char key[256];
	char rest[256];
	while (file.getline(line, 256)) { // parse file
		strcpy(key, ""); // reset these strings.
		strcpy(rest, ""); // reset these strings.
		sscanf(line, "%s %s", key, rest); // scan line from file
		if (key[0] == '#') { // ignore comments
		} else if (strcmp(line, "") == 0) { // ignore blank lines
		} else if (strcmp(key, "TS-DIST-NEA") == 0) { // target seeking sets
			sscanf(line,"%s %f %f", key, &mInTargetDistanceNear[0], &mInTargetDistanceNear[1]);
		} else if (strcmp(key, "TS-DIST-MED") == 0) {
			sscanf(line,"%s %f %f %f %f", key, &mInTargetDistanceFar[0], &mInTargetDistanceFar[1], &mInTargetDistanceFar[2], &mInTargetDistanceFar[3]);
		} else if (strcmp(key, "TS-DIST-FAR") == 0) {
			sscanf(line,"%s %f %f", key, &mInTargetDistanceVeryFar[0], &mInTargetDistanceVeryFar[1]);
		} else if (strcmp(key, "TS-ANGLE-NAR") == 0) {
			sscanf(line,"%s %f %f", key, &mInTargetAngleSmall[0], &mInTargetAngleSmall[1]);
		} else if (strcmp(key, "TS-ANGLE-MID") == 0) {
			sscanf(line,"%s %f %f %f %f", key, &mInTargetAngleMedium[0], &mInTargetAngleMedium[1], &mInTargetAngleMedium[2], &mInTargetAngleMedium[3]);
		} else if (strcmp(key, "TS-ANGLE-WID") == 0) {
			sscanf(line,"%s %f %f", key, &mInTargetAngleLarge[0], &mInTargetAngleLarge[1]);
		} else if (strcmp(key, "TS-ANGLE-THRESH") == 0) {
			sscanf(line,"%s %f", key, &mInTargetAngleThreshold);
		} else if (strcmp(key, "TS-SPEED-MIDS") == 0) {
			sscanf(line,"%s %f %f %f %f %f %f", key, &mOutTargetSpeedMids[0], &mOutTargetSpeedMids[1], &mOutTargetSpeedMids[2], &mOutTargetSpeedMids[3], &mOutTargetSpeedMids[4], &mOutTargetSpeedMids[5]);
		} else if (strcmp(key, "TS-ANGLE-MIDS") == 0) {
			sscanf(line,"%s %f %f %f %f %f %f %f", key, &mOutTargetAngleMids[0], &mOutTargetAngleMids[1], &mOutTargetAngleMids[2], &mOutTargetAngleMids[3], &mOutTargetAngleMids[4], &mOutTargetAngleMids[5], &mOutTargetAngleMids[6]);
		// obstacle avoidance sets
		} else if (strcmp(key, "OA-DIST-NEA") == 0) {
			sscanf(line,"%s %f %f", key, &mInObstacleDistanceNear[0], &mInObstacleDistanceNear[1]);
		} else if (strcmp(key, "OA-DIST-MED") == 0) {
			sscanf(line,"%s %f %f %f %f", key, &mInObstacleDistanceFar[0], &mInObstacleDistanceFar[1], &mInObstacleDistanceFar[2], &mInObstacleDistanceFar[3]);
		} else if (strcmp(key, "OA-DIST-FAR") == 0) {
			sscanf(line,"%s %f %f", key, &mInObstacleDistanceVeryFar[0], &mInObstacleDistanceVeryFar[1]);
		} else if (strcmp(key, "OA-ANGLE-NAR") == 0) {
			sscanf(line,"%s %f %f", key, &mInObstacleAngleSmall[0], &mInObstacleAngleSmall[1]);
		} else if (strcmp(key, "OA-ANGLE-MID") == 0) {
			sscanf(line,"%s %f %f %f %f", key, &mInObstacleAngleMedium[0], &mInObstacleAngleMedium[1], &mInObstacleAngleMedium[2], &mInObstacleAngleMedium[3]);
		} else if (strcmp(key, "OA-ANGLE-WID") == 0) {
			sscanf(line,"%s %f %f", key, &mInObstacleAngleLarge[0], &mInObstacleAngleLarge[1]);
		} else if (strcmp(key, "OA-ANGLE-THRESH") == 0) {
			sscanf(line,"%s %f", key, &mInObstacleAngleThreshold);
		} else if (strcmp(key, "OA-SPEED-MIDS") == 0) {
			sscanf(line,"%s %f %f %f %f %f %f", key, &mOutObstacleSpeedMids[0], &mOutObstacleSpeedMids[1], &mOutObstacleSpeedMids[2], &mOutObstacleSpeedMids[3], &mOutObstacleSpeedMids[4], &mOutObstacleSpeedMids[5]);
		} else if (strcmp(key, "OA-ANGLE-MIDS") == 0) {
			sscanf(line,"%s %f %f %f %f %f %f %f", key, &mOutObstacleAngleMids[0], &mOutObstacleAngleMids[1], &mOutObstacleAngleMids[2], &mOutObstacleAngleMids[3], &mOutObstacleAngleMids[4], &mOutObstacleAngleMids[5], &mOutObstacleAngleMids[6]);
		}
	}
	file.close();

	return true;
}

// read FAMM rules from file (returns true if successful)
bool FuzzyController::readRulesFromFile(const char* filename) {
	ifstream file; // C++ file stream
	file.open(filename); // load file
	if (!file.is_open()) { // some basic validation
		printf("ERROR: Could not open fuzzy rules file %s! FuzzyController::readRulesFromFile()\n",filename);
		return false; // failure
	}
	char line[256]; // buffer
	char key[256]; // buffer
	char outputType[256]; // buffer
	int angleRule;
	int distRule;
	int outRule;
	while (file.getline(line, 256)) { // parse file
		strcpy(key, "");	// reset these strings. for some reason, even though the definitions were in here....
		angleRule = -1; // reset
		distRule = -1; // reset
		outRule = -1; // reset
		// example: "TS STEER 1 & 0 = 5"
		sscanf(line, "%s %s %i & %i = %i", key, outputType, &distRule, &angleRule, &outRule); // scan the next line from the file into components
		if (key[0] == '#') { // ignore comments
		} else if (strcmp(line, "") == 0) { // ignore blank lines
		}	else { // valid lines
			if (strcmp(key, "TS") == 0) { // check if target-seeking rule
				if (strcmp(outputType, "SPEED") == 0) { // check if speed rule
					mRuleBase.mRuleBaseTSSpeed[distRule][angleRule] = outRule; // record rule
				} else if (strcmp(outputType, "STEER") == 0) { // check if steering rule
					mRuleBase.mRuleBaseTSSteer[distRule][angleRule] = outRule; // record rule
				} else { // complain that we don't recognise this rule
					printf("ERROR: unrecognised output type (%s) scanned from file: %s\n", outputType, filename);
				}
			} else if (strcmp(key, "OA") == 0) { // check if obstacle-avoidance rule
				if (strcmp(outputType, "SPEED") == 0) { // check if speed rule
					mRuleBase.mRuleBaseOASpeed[distRule][angleRule] = outRule; // record rule
				} else if (strcmp(outputType, "STEER") == 0) { // check if steering rule
					mRuleBase.mRuleBaseOASteer[distRule][angleRule] = outRule; // record rule
				} else { // complain that we don't recognise this rule
					printf("ERROR: unrecognised output type (%s) scanned from file: %s\n", outputType, filename);
				}
			} else { // complain that we don't recognise this rule
				printf("WARNING: unrecognised key (%s) scanned from file: %s\n", key, filename);
			}
		}
	}
	file.close(); // close file pointer!
	return true; // success
}

// Reset all internal variables
void FuzzyController::clear() {
	for (int i = 0; i < 3; i++) {
		mInAngleSetMem[i] = 0.0f; // Reset Fuzzy Set Memberships
		mInDistSetMem[i] = 0.0f; // Reset Fuzzy Set Memberships
	}
	for (int i = 0; i < NUM_SPEED_SETS; i++) {
		mOutSpeedSetMem[i] = 0.0f; // Reset Fuzzy Set Memberships
	}
	for (int i = 0; i < NUM_STEER_SETS; i++) {
		mOutSteerSetMem[i] = 0.0f; // Reset Fuzzy Set Memberships
	}
	mTargetSeeking_Speed = 0.0f; // Defuzzified Values
	mTargetSeeking_Angle = 0.0f; // Defuzzified Values
	mObstacleAvoidance_Speed = 0.0f; // Defuzzified Values
	mObstacleAvoidance_Angle = 0.0f; // Defuzzified Values
}

// Function takes angle and distance from target and stores speed and angle
void FuzzyController::processTargetSeeking(float in_targetangle, float in_targetdistance) {
	clear(); // Reset everything (just in case)
	while (in_targetangle < 0.0f) { // Make angle valid (don't want a negative angle)
		in_targetangle += TWO_PI;
	}
	while (in_targetangle > TWO_PI) {
		in_targetangle -= TWO_PI;
	}
	if (in_targetangle > PI) { // Convert angles bigger than Pi to a range of 0-pi
		in_targetangle = PI - (in_targetangle - PI);
	}
	UpdateTargetInputMemberships(in_targetangle, in_targetdistance);
	CalculateTargetOutputMemberships();
	if (in_targetangle > mInTargetAngleThreshold) { // Calculate Speed with C.O.G. function if angletotarget is not > threshold
		mTargetSeeking_Speed = 0.0f;
	} else {
		mTargetSeeking_Speed = CalculateTargetSpeed();
	}
	mTargetSeeking_Angle = CalculateTargetAngle(); // Calculate Angle with C.O.G.
}

// Function takes angle and distance from nearest obstacle and stores speed and angle
void FuzzyController::processObstacleAvoidance(float in_obstangle, float in_obstdistance) {
	clear(); // Reset everything (just in case)
	while (in_obstangle < 0.0f) { // Make angle valid (don't want a negative angle)
		in_obstangle += TWO_PI;
	}
	while (in_obstangle > TWO_PI) {
		in_obstangle -= TWO_PI;
	}
	if (in_obstangle > PI) { // Convert angles bigger than Pi to a range of 0-pi
		in_obstangle = PI - (in_obstangle - PI);
	}
	UpdateObstacleInputMemberships(in_obstangle, in_obstdistance);
	CalculateObstacleOutputMemberships();
	mObstacleAvoidance_Speed = CalculateObstacleSpeed();
	mObstacleAvoidance_Angle = CalculateObstacleAngle(); // Calculate Angle with C.O.G.
}

// Function takes real inputs and converts to fuzzy set memberships returns false on error
bool FuzzyController::UpdateTargetInputMemberships(float in_targetangle, float in_targetdistance) {
	if (in_targetdistance > mInTargetDistanceVeryFar[1]) { // Work out distance fuzzy set memberships
		mInDistSetMem[2] = 1.0; // Definite FAR case
	} else if (in_targetdistance < mInTargetDistanceNear[0]) {
		mInDistSetMem[0] = 1.0; // Definate NEA case
	} else if ((in_targetdistance > mInTargetDistanceFar[1]) && (in_targetdistance < mInTargetDistanceFar[2])) {
		mInDistSetMem[1] = 1.0; // Definate MED case
	} else if (in_targetdistance >= mInTargetDistanceFar[2]) { // MED-FAR
		float divisor = mInTargetDistanceFar[3] - mInTargetDistanceFar[2];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[1] = 1.0f - (in_targetdistance - mInTargetDistanceFar[2]) / divisor;
		divisor = mInTargetDistanceVeryFar[1] - mInTargetDistanceVeryFar[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[2] = (in_targetdistance - mInTargetDistanceVeryFar[0]) / divisor;
	} else { // NEA-MED
		float divisor = mInTargetDistanceNear[1] - mInTargetDistanceNear[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[0] = 1.0f - (in_targetdistance - mInTargetDistanceNear[0]) / divisor;
		divisor = mInTargetDistanceFar[1] - mInTargetDistanceFar[0];
		if (divisor == 0.0f) {
			printf("/0 error\n");
		}
		mInDistSetMem[1] = (in_targetdistance - mInTargetDistanceFar[0]) / divisor;
	}
	if (in_targetangle > mInTargetAngleLarge[1]) { // Work out fuzzy angle set memberships
		mInAngleSetMem[2] = 1.0; // Definite WID case
	} else if (in_targetangle < mInTargetAngleSmall[0]) {
		mInAngleSetMem[0] = 1.0; // Definate NAR case
	} else if ((in_targetangle > mInTargetAngleMedium[1]) && (in_targetangle < mInTargetAngleMedium[2])) {
		mInAngleSetMem[1] = 1.0; // Definate MID case
	} else if (in_targetangle >= mInTargetAngleMedium[2]) {
		float divisor = mInTargetAngleMedium[3] - mInTargetAngleMedium[2];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[1] = 1.0f - (in_targetangle - mInTargetAngleMedium[2]) / divisor;
		divisor = mInTargetAngleLarge[1] - mInTargetAngleLarge[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[2] = (in_targetangle - mInTargetAngleLarge[0]) / divisor;
	} else {
		float divisor = mInTargetAngleSmall[1] - mInTargetAngleSmall[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[0] = 1.0f - (in_targetangle - mInTargetAngleSmall[0]) / divisor;
		divisor = mInTargetAngleMedium[1] - mInTargetAngleMedium[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[1] = (in_targetangle - mInTargetAngleMedium[0]) / divisor;
	}
	return true; // success
}

// Function takes real inputs and converts to fuzzy set memberships returns false on error
bool FuzzyController::UpdateObstacleInputMemberships(float in_obstangle, float in_obstdistance) {
	// Work out distance fuzzy set memberships
	if (in_obstdistance > mInObstacleDistanceVeryFar[1]) {
		mInDistSetMem[2] = 1.0; // Definite FAR case
	} else if (in_obstdistance < mInObstacleDistanceNear[0]) {
		mInDistSetMem[0] = 1.0; // Definate NEA case
	} else if ((in_obstdistance > mInObstacleDistanceFar[1]) && (in_obstdistance < mInObstacleDistanceFar[2])) {
		mInDistSetMem[1] = 1.0; // Definate MED case
	} else if (in_obstdistance >= mInObstacleDistanceFar[2]) { // MED-FAR
		float divisor = mInObstacleDistanceFar[3] - mInObstacleDistanceFar[2];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[1] = 1.0f - (in_obstdistance - mInObstacleDistanceFar[2]) / divisor;
		divisor = mInObstacleDistanceVeryFar[1] - mInObstacleDistanceVeryFar[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[2] = (in_obstdistance - mInObstacleDistanceVeryFar[0]) / divisor;
	} else { // NEA-MED
		float divisor = mInObstacleDistanceNear[1] - mInObstacleDistanceNear[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[0] = 1.0f - (in_obstdistance - mInObstacleDistanceNear[0]) / divisor;
		divisor = mInObstacleDistanceFar[1] - mInObstacleDistanceFar[0];
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInDistSetMem[1] = (in_obstdistance - mInObstacleDistanceFar[0]) / divisor;
	}
	// Work out fuzzy angle set memberships
	if (in_obstangle > mInObstacleAngleLarge[1]) {
		mInAngleSetMem[2] = 1.0; // Definite WID case
	} else if (in_obstangle < mInObstacleAngleSmall[0]) {
		mInAngleSetMem[0] = 1.0; // Definate NAR case
	} else if ((in_obstangle > mInObstacleAngleMedium[1]) && (in_obstangle < mInObstacleAngleMedium[2])) {
		mInAngleSetMem[1] = 1.0; // Definate MID case
	} else if (in_obstangle >= mInObstacleAngleMedium[2]) {
		float divisor = (mInObstacleAngleMedium[3] - mInObstacleAngleMedium[2]);
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[1] = 1.0f - ((in_obstangle - mInObstacleAngleMedium[2]) / divisor);
		divisor = (mInObstacleAngleLarge[1] - mInObstacleAngleLarge[0]);
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[2] = (in_obstangle - mInObstacleAngleLarge[0]) / divisor;
	} else {
		float divisor = (mInObstacleAngleSmall[1] - mInObstacleAngleSmall[0]);
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[0] = 1.0f - ((in_obstangle - mInObstacleAngleSmall[0]) / divisor);
		divisor = (mInObstacleAngleMedium[1] - mInObstacleAngleMedium[0]);
		if (divisor == 0.0f) {
			divisor = 0.00001f;
		}
		mInAngleSetMem[1] = (in_obstangle - mInObstacleAngleMedium[0]) / divisor;
	}
	return true;
}

// uses member variables for input set membership and calculates output set memberships
bool FuzzyController::CalculateTargetOutputMemberships() {
	for (int i = 0; i < 3; i++) { // angle in loop
		for (int j = 0; j < 3; j++) { // dist in loop
			if (mInAngleSetMem[i] > 0.0) {
				if (mInDistSetMem[j] > 0.0) {
					int outSpeedSetIndex = mRuleBase.mRuleBaseTSSpeed[j][i]; // rule lookup
					int outSteerSetIndex = mRuleBase.mRuleBaseTSSteer[j][i]; // rule lookup
					float tempMemValue = min(mInAngleSetMem[i], mInDistSetMem[j]);
					mOutSpeedSetMem[outSpeedSetIndex] = max(mOutSpeedSetMem[outSpeedSetIndex], tempMemValue); // choose new output membership
					mOutSteerSetMem[outSteerSetIndex] = max(mOutSteerSetMem[outSteerSetIndex], tempMemValue); // choose new output membership
				}
			}
		}
	}
	return true; // success
}

// uses member variables for input set membership and calculates output set memberships
bool FuzzyController::CalculateObstacleOutputMemberships() {
	for (int i = 0; i < 3; i++) { // angle in loop
		for (int j = 0; j < 3; j++) { // dist in loop
			if (mInAngleSetMem[i] > 0.0) {
				if (mInDistSetMem[j] > 0.0) {
					int outSpeedSetIndex = mRuleBase.mRuleBaseOASpeed[j][i]; // rule lookup
					int outSteerSetIndex = mRuleBase.mRuleBaseOASteer[j][i]; // rule lookup
					float tempMemValue = min(mInAngleSetMem[i], mInDistSetMem[j]);
					mOutSpeedSetMem[outSpeedSetIndex] = max(mOutSpeedSetMem[outSpeedSetIndex], tempMemValue); // choose new output membership
					mOutSteerSetMem[outSteerSetIndex] = max(mOutSteerSetMem[outSteerSetIndex], tempMemValue); // choose new output membership
				}
			}
		}
	}
	return true; // success
}

// Function to perform centre of gravity calculation of fuzzy outputs and get real output
float FuzzyController::CalculateTargetSpeed() {
	float numeratorSpeed = (
		mOutTargetSpeedMids[0] * mOutSpeedSetMem[0] +
		mOutTargetSpeedMids[1] * mOutSpeedSetMem[1] +
		mOutTargetSpeedMids[2] * mOutSpeedSetMem[2] +
		mOutTargetSpeedMids[3] * mOutSpeedSetMem[3] +
		mOutTargetSpeedMids[4] * mOutSpeedSetMem[4] +
		mOutTargetSpeedMids[5] * mOutSpeedSetMem[5]
	);
	float denominatorSpeed = mOutSpeedSetMem[0] + mOutSpeedSetMem[1] + mOutSpeedSetMem[2] + mOutSpeedSetMem[3] + mOutSpeedSetMem[4] + mOutSpeedSetMem[5];
	if (denominatorSpeed == 0.0f) {
		denominatorSpeed = 0.00001f; // avoid /zero errors
	}
	return numeratorSpeed / denominatorSpeed;
}

// Function to perform centre of gravity calculation of fuzzy outputs and get real output
float FuzzyController::CalculateTargetAngle() {
	float numeratorSpeed = (
		mOutTargetAngleMids[0] * mOutSteerSetMem[0] +
		mOutTargetAngleMids[1] * mOutSteerSetMem[1] +
		mOutTargetAngleMids[2] * mOutSteerSetMem[2] +
		mOutTargetAngleMids[3] * mOutSteerSetMem[3] +
		mOutTargetAngleMids[4] * mOutSteerSetMem[4] +
		mOutTargetAngleMids[5] * mOutSteerSetMem[5] +
		mOutTargetAngleMids[6] * mOutSteerSetMem[6]
	);
	float denominatorSpeed = mOutSteerSetMem[0] + mOutSteerSetMem[1] + mOutSteerSetMem[2] + mOutSteerSetMem[3] + mOutSteerSetMem[4] + mOutSteerSetMem[5] + mOutSteerSetMem[6];
	if (denominatorSpeed == 0.0f) {
		denominatorSpeed = 0.00001f; // avoid /zero errors
	}
	return numeratorSpeed / denominatorSpeed;
}

// Function to perform centre of gravity calculation of fuzzy outputs and get real output
float FuzzyController::CalculateObstacleSpeed() {
	float numeratorSpeed = (
		mOutObstacleSpeedMids[0] * mOutSpeedSetMem[0] +
		mOutObstacleSpeedMids[1] * mOutSpeedSetMem[1] +
		mOutObstacleSpeedMids[2] * mOutSpeedSetMem[2] +
		mOutObstacleSpeedMids[3] * mOutSpeedSetMem[3] +
		mOutObstacleSpeedMids[4] * mOutSpeedSetMem[4] +
		mOutObstacleSpeedMids[5] * mOutSpeedSetMem[5]
	);
	float denominatorSpeed = mOutSpeedSetMem[0] + mOutSpeedSetMem[1] + mOutSpeedSetMem[2] + mOutSpeedSetMem[3] + mOutSpeedSetMem[4] + mOutSpeedSetMem[5];
	if (denominatorSpeed == 0.0f) {
		denominatorSpeed = 0.00001f; // avoid /zero errors
	}
	return numeratorSpeed / denominatorSpeed;
}

// Function to perform centre of gravity calculation of fuzzy outputs and get real output
float FuzzyController::CalculateObstacleAngle() {
	float numeratorSpeed = (
		mOutObstacleAngleMids[0] * mOutSteerSetMem[0] +
		mOutObstacleAngleMids[1] * mOutSteerSetMem[1] +
		mOutObstacleAngleMids[2] * mOutSteerSetMem[2] +
		mOutObstacleAngleMids[3] * mOutSteerSetMem[3] +
		mOutObstacleAngleMids[4] * mOutSteerSetMem[4] +
		mOutObstacleAngleMids[5] * mOutSteerSetMem[5] +
		mOutObstacleAngleMids[6] * mOutSteerSetMem[6]
	);

	float denominatorSpeed = mOutSteerSetMem[0] + mOutSteerSetMem[1] + mOutSteerSetMem[2] + mOutSteerSetMem[3] + mOutSteerSetMem[4] + mOutSteerSetMem[5]  + mOutSteerSetMem[6];
	if (denominatorSpeed == 0.0f) {
		denominatorSpeed = 0.00001f; // avoid /zero errors
	}
	float result = numeratorSpeed / denominatorSpeed;
	if (isnan(result)) {
		printf("FATAL ERROR: result is NAN in FuzzyController::CalculateObstacleSpeed() where float result(%f) = numeratorSpeed(%f) / denominatorSpeed(%f)\n", result, numeratorSpeed, denominatorSpeed);
		printf("and where float numeratorSpeed = mOutObstacleAngleMids[0-6] * mOutSteerSetMem[0-6] as follows:\n");
		for (int i = 0; i < NUM_STEER_SETS; i++) {
			printf("mOutObstacleAngleMids[%i](%f) * mOutSteerSetMem[%i](%f)\n", i, mOutObstacleAngleMids[i], i, mOutSteerSetMem[i]);
		}
		printf("and where float denominatorSpeed = mOutSteerSetMem[0](%f) + mOutSteerSetMem[1](%f) + mOutSteerSetMem[2](%f) + mOutSteerSetMem[3](%f) + mOutSteerSetMem[4](%f) + mOutSteerSetMem[5](%f) + mOutSteerSetMem[6](%f);\n", mOutSteerSetMem[0], mOutSteerSetMem[1], mOutSteerSetMem[2], mOutSteerSetMem[3], mOutSteerSetMem[4], mOutSteerSetMem[5], mOutSteerSetMem[6]);
		exit(1);
	}
	return result;
}

// return target seeking speed
float FuzzyController::getTargetSeekingSpeed() {
	return mTargetSeeking_Speed;
}

// return target seeking angle
float FuzzyController::getTargetSeekingAngle() {
	return mTargetSeeking_Angle;
}

// return obstacle avoidance speed
float FuzzyController::getObstacleAvoidanceSpeed() {
	return mObstacleAvoidance_Speed;
}

// return obstacle avoidance angle
float FuzzyController::getObstacleAvoidanceAngle() {
	return mObstacleAvoidance_Angle;
}

float FuzzyController::getObstacleAngleThreshold() {
	return mInObstacleAngleThreshold;
}

void FuzzyController::getAngleInputMemberships(float& small, float& medium, float& large) {
	small = mInAngleSetMem[0];
	medium = mInAngleSetMem[1];
	large = mInAngleSetMem[2];
}

void FuzzyController::getDistInputMemberships(float& near, float& far, float& vfar) {
	near = mInDistSetMem[0];
	far = mInDistSetMem[1];
	vfar = mInDistSetMem[2];
}

void FuzzyController::getSpeedOutputMemberships(float& vslow, float& slow, float& med, float& fast, float& vfast, float& wfast) {
	vslow = mOutSpeedSetMem[0];
	slow = mOutSpeedSetMem[1];
	med = mOutSpeedSetMem[2];
	fast = mOutSpeedSetMem[3];
	vfast = mOutSpeedSetMem[4];
	wfast = mOutSpeedSetMem[5];
}

void FuzzyController::getSteerOutputMemberships(float& zer, float& vli, float& lig, float& mod, float& sha, float& vsh, float& max) {
	zer = mOutSteerSetMem[0];
	vli = mOutSteerSetMem[1];
	lig = mOutSteerSetMem[2];
	mod = mOutSteerSetMem[3];
	sha = mOutSteerSetMem[4];
	vsh = mOutSteerSetMem[5];
	max = mOutSteerSetMem[6];
}

