#ifndef _GENETIC_LOGGER_
#define _GENETIC_LOGGER_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"
using namespace Ogre;
#include <stdio.h>
#include <string.h>
#include <fstream>
using namespace std;

#define GA_MILEAGE_STOP 1000.0 // append to log after 100m
#define NUM_SPEED_DATAPOINTS 100 // number of speed data points to collect to compute average
#define MAX_CRASH 30.0
#define CRASH_WEIGHT 1.0
#define SPEED_WEIGHT 1.0
#define STUCK_LIMIT 1000 // this many frames is max to be moving < 0.5kph

struct GeneticLogger {
	char mLogFileName[256];
	double mCollisionHeuristic; // time (s) * penetration depth (m)
	double mMileage_m;
	double mSpeedDataPoint_kph[NUM_SPEED_DATAPOINTS];
	double mAverageSpeed_kph;
	double mDataPointSeparation;
	long int mGeneration;
	long int mBatch;
	long int mRunsLeft;
	long int mRunsCompleted;
	int mNextDataPoint;
	int mStuckCount;
	GeneticLogger();
	void resetCollectedData();
	void recordMileage(double dist_m, double speed_mpus, const double& maxspeed);
	bool updateLogs(const double& maxspeed);
};

#endif

