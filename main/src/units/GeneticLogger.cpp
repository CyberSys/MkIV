#include "GeneticLogger.h"
#include "../usefulFuncs.h"

GeneticLogger::GeneticLogger() {
	strcpy(mLogFileName, "unnamed-ga.log");
	mDataPointSeparation = GA_MILEAGE_STOP / NUM_SPEED_DATAPOINTS;
	resetCollectedData();
	mGeneration = -1;
	mBatch = -1;
	mRunsLeft = 0;
	mRunsCompleted = 0;
}

void GeneticLogger::resetCollectedData() {
	mMileage_m = 0.0;
	mCollisionHeuristic = 0.0;
	mAverageSpeed_kph = 0.0;
	for (int i = 0; i < NUM_SPEED_DATAPOINTS; i++) {
		mSpeedDataPoint_kph[i] = 0.0;
	}
	mNextDataPoint = 0;
	mStuckCount = 0;
}

void GeneticLogger::recordMileage(double dist_m, double speed_mpus, const double& maxspeed) {
	mMileage_m += dist_m; // add mileage to total
	// decide if far enough to record a data point
	if (mMileage_m > mNextDataPoint * mDataPointSeparation) {
		double kph = mpusTokph(speed_mpus); // convert speed to kph
		mSpeedDataPoint_kph[mNextDataPoint] = kph; // record speed as a data point
		mNextDataPoint++;
	} else if (speed_mpus < 0.0000001) {
		mStuckCount++;
	}
	updateLogs(maxspeed);	
}

bool GeneticLogger::updateLogs(const double& maxspeed) {
	if (mRunsLeft <= 0) {
		//printf("WARNING: no runs left to log in GeneticLogger::updateLogs. is the GAManager out of juice?\n");
		return true;
	}

	if (mMileage_m >= GA_MILEAGE_STOP) { // check if time to log
	
		// work out average speed
		double sum = 0.0;
		for (int i = 0; i < mNextDataPoint; i++) {
			sum += mSpeedDataPoint_kph[i];
		}
		mAverageSpeed_kph = sum / (double)mNextDataPoint;
		double speedFitness = 1.0 - (mAverageSpeed_kph / maxspeed); // puts this in the range of 0-1 (top speed is 0, stopped is 1)
		double crashFitness = mCollisionHeuristic / MAX_CRASH; // hopefully in the 0-1 range (no crash is 0, biggest crash is 1)
		double fitness = crashFitness * CRASH_WEIGHT + speedFitness * SPEED_WEIGHT;
		
		// write to file
		FILE* outFilePtr = NULL;
		outFilePtr = fopen(mLogFileName, "a+"); // open file
		if (!outFilePtr) { // validate file
			printf("Error: Could not open file %s!\n", mLogFileName);
			return false;
		}
		fprintf(outFilePtr, "G:%li B:%li MILEAGE: %lf and CRASH-RATING: %lf + SPEED-RATING: %lf (1.0 - %lf / %lf) = FITNESS: %lf\n", mGeneration, mBatch, mMileage_m, crashFitness, speedFitness, mAverageSpeed_kph, maxspeed, fitness); // append
		fclose(outFilePtr); // close file
		
		mRunsLeft--;
		mRunsCompleted++;
		
		// reset logger's data
		resetCollectedData(); // clear
	} else if (mStuckCount >= STUCK_LIMIT) { // timeout
		printf("WARNING: vehicle stuck too many times in a row; cancelling this run with avg speed fitness set to 20.0\n");
		mAverageSpeed_kph = 0.0;
		double speedFitness = 20.0;
		double crashFitness = mCollisionHeuristic / MAX_CRASH;
		double fitness = crashFitness * CRASH_WEIGHT + speedFitness * SPEED_WEIGHT;
		// write to file
		FILE* outFilePtr = NULL;
		outFilePtr = fopen(mLogFileName, "a+"); // open file
		if (!outFilePtr) { // validate file
			printf("Error: Could not open file %s!\n", mLogFileName);
			return false;
		}
		fprintf(outFilePtr, "G:%li B:%li MILEAGE: %lf and CRASH-RATING: %lf + SPEED-RATING: %lf (1.0 - %lf / %lf) = FITNESS: %lf\n", mGeneration, mBatch, mMileage_m, crashFitness, speedFitness, mAverageSpeed_kph, maxspeed, fitness); // append
		fclose(outFilePtr); // close file
		
		mRunsLeft--;
		mRunsCompleted++;
		
		// reset logger's data
		resetCollectedData(); // clear
	}
	
	return true;
}

