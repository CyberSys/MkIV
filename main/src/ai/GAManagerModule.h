#ifndef GAMANMOD_H_
#define GAMANMOD_H_

#define MAX_POPULATION 1024

// this is a little module keeping separate all the GA parameters and variables that are loaded from a file
struct GaSettings {
	GaSettings();
	void resetSettings();
	bool loadFromFile(char* settingsFile);
	char mGenePoolFile[256];
	char mParentsFile[256];
	char mChampsFile[256];
	char mRunCountDownFile[256];
	char mGALogFile[256];
	char mSummaryFile[256];
	char mHallOfFameFile[256];
	char mEvolutionGraphFile[256];
	char mNextSettingsFileInChain[256];
	char mArchiveDir[256];
	long int mRunsToDo;
	long int mPopulationSizeN;
	long int mNumerOfParentsP;
	long int mMaxGen;
	int mMutationLevelM;
	int mRadiationPercentageR;
	bool mEvolutionLimit;
	bool mLoadAnotherSettingsFileAfterThisOne; // if this is true then the GAManager will attempt to load another experiment and keep going
	bool mArchiveWhenDone; // if we should copy all the files used into a folder when finished
};

// this is a little struct holding current data for each member of the population (how many more runs to do for it etc.)
struct RunsEntry {
	RunsEntry();
	long int mCountOfUncompletedRuns; // ... to write back to run key file at end
	long int mGeneration;
	long int mBatch;
};

// this is the main GA manager class
struct GAManagerModule {
	GAManagerModule();
	void reset();
	bool loadRunKeys();
	bool saveRunKeys();
	int getIndexOfNextIndividual(long int runsRequested, long int& gen, long int &batch, long int& runsGranted);
	bool updateCompletedRuns(long int gen, long int batch, long int runsCompleted);
	
	GaSettings mGASettings;
	RunsEntry mRunEntries[MAX_POPULATION];
	int mCountOfEntries;
	int mBatchToAllocate;
	long int mTotalRunsRemaining;
	long int mLastGenLoaded;
};

#endif

