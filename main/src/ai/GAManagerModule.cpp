#include "GAManagerModule.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

GaSettings::GaSettings() {
	resetSettings();
}

void GaSettings::resetSettings() {
	strcpy(mGenePoolFile, "./data/ai/genepool.txt");
	strcpy(mParentsFile, "./data/ai/parents.txt");
	strcpy(mChampsFile, "./data/ai/champs.txt");
	strcpy(mRunCountDownFile, "./data/ai/populationRunCountdown.txt");
	strcpy(mGALogFile, "./data/ai/logs/log.txt");
	strcpy(mSummaryFile, "./data/ai/logs/sum.txt");
	strcpy(mHallOfFameFile, "./data/ai/HallOfFame.txt");
	strcpy(mEvolutionGraphFile, "./data/ai/fitnessEvoGraph.xy");
	strcpy(mArchiveDir, "");
	mMutationLevelM = 4;
	mRunsToDo = 30;
	mPopulationSizeN = 4;
	mNumerOfParentsP = 2;
	mRadiationPercentageR = 20;
	mEvolutionLimit = false;
	mLoadAnotherSettingsFileAfterThisOne = false;
	mArchiveWhenDone = false;
}

bool GaSettings::loadFromFile(char* settingsFile) {
	resetSettings();
	printf("GaSettings - loading settings from %s\n", settingsFile);
	ifstream file; // open the file
	file.open(settingsFile);
	if (!file) { // validate file
		printf("Error: GaSettings::loadFromFile() Could not open file %s!\n", settingsFile);
		return false;
	}
	char lineText[256];
	char key[128];
	char next[256];
	while (file.getline(lineText, 256)) {
		strcpy(key, "");
		strcpy(next, "");
		sscanf(lineText, "%s %s", key, next);
		if (key[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore blank lines
		} else if (strcmp(key, "GENEPOOLFILE") == 0) {
			strcpy(mGenePoolFile, next);
		} else if (strcmp(key, "RUNCOUNTDOWNFILE") == 0) {
			strcpy(mRunCountDownFile, next);
		} else if (strcmp(key, "GALOGFILE") == 0) {
			strcpy(mGALogFile, next);
		} else if (strcmp(key, "SUMMARYFILE") == 0) {
			strcpy(mSummaryFile, next);
		} else if (strcmp(key, "HALLOFFAMEFILE") == 0) {
			strcpy(mHallOfFameFile, next);
		} else if (strcmp(key, "EVOGRAPHFILE") == 0) {
			strcpy(mEvolutionGraphFile, next);
		} else if (strcmp(key, "ARCHIVE_INTO_DIR") == 0) {
			mArchiveWhenDone = true;
			strcpy(mArchiveDir, next);
		} else if (strcmp(key, "NEXT_SETTINGS_FILE_IN_CHAIN") == 0) {
			strcpy(mNextSettingsFileInChain, next);
			mLoadAnotherSettingsFileAfterThisOne = true;
		} else if (strcmp(key, "MUTATION_LEVEL") == 0) {
			sscanf(next, "%i", &mMutationLevelM);
		} else if (strcmp(key, "RUNS_TO_DO") == 0) {
			sscanf(next, "%li", &mRunsToDo);
		} else if (strcmp(key, "POP_SIZE_N") == 0) {
			sscanf(next, "%li", &mPopulationSizeN);
		} else if (strcmp(key, "PARENTS_P") == 0) {
			sscanf(next, "%li", &mNumerOfParentsP);
		} else if (strcmp(key, "RADIATION_LEVEL") == 0) {
			sscanf(next, "%i", &mRadiationPercentageR);
		} else if (strcmp(key, "MAX_GEN") == 0) {
			sscanf(next, "%li", &mMaxGen);
			mEvolutionLimit = true;
		}
	}
	file.close();
	return true;
}

RunsEntry::RunsEntry() {
	mCountOfUncompletedRuns = 0;
	mGeneration = -1;
	mBatch = -1;
}

GAManagerModule::GAManagerModule() {
	reset();
}

void GAManagerModule::reset() {
	mCountOfEntries = 0;
	mTotalRunsRemaining = 0;
	mBatchToAllocate = 0;
	mLastGenLoaded = -1;
}

bool GAManagerModule::loadRunKeys() {
	printf("GAMan - loading a new generation...\n");
	mCountOfEntries = 0;
	mTotalRunsRemaining = 0;

	ifstream file; // open the file
	file.open(mGASettings.mRunCountDownFile);
	if (!file) { // validate file
		printf("Error: GAManagerModule::loadRunKeys() Could not open file %s!\n", mGASettings.mRunCountDownFile);
		return false;
	}
	char lineText[256];
	long int currentGen = -1;
	while (file.getline(lineText, 256)) {
		// G:2 B:1 R:500
		if (lineText[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore
		} else {
			long int gen = -1;
			long int batch = -1;
			long int runsRemaining = -1;
			sscanf(lineText, "G:%li B:%li R:%li", &gen, &batch, &runsRemaining);
			
			if (gen == mLastGenLoaded) {
				printf("WARNING: GAMAN waiting on new generation...not going to load the same one again\n");
				file.close();
				return true;
			}
			if (currentGen == -1) {
				currentGen = gen;
			} else if (gen != currentGen) {
				printf("WARNING: GAMAN - gen file has mixed entries! Attemping to skip...\n");
				continue;
			}
			
			if (mCountOfEntries >= MAX_POPULATION - 1) {
				printf("ERROR: GAManagerModule::loadRunKeys() too many individuals loaded for memory (population size max exceeded)\n");
				return false;
			}
			
			mRunEntries[mCountOfEntries].mGeneration = gen;
			mRunEntries[mCountOfEntries].mBatch = batch;
			mRunEntries[mCountOfEntries].mCountOfUncompletedRuns = runsRemaining;
			mTotalRunsRemaining += runsRemaining;
			mCountOfEntries++;
		}
	}
	printf("GAMAN %i run-key entries loaded\n", mCountOfEntries);
	mLastGenLoaded = currentGen;
	file.close();

	return true;
}

int GAManagerModule::getIndexOfNextIndividual(long int runsRequested, long int& gen, long int &batch, long int& runsGranted) {
	if (mGASettings.mEvolutionLimit) {
		if (mLastGenLoaded >= mGASettings.mMaxGen) {
			printf("Evolution limit reached.\n");
			if (mGASettings.mLoadAnotherSettingsFileAfterThisOne) {
				if (mGASettings.mArchiveWhenDone) {
					printf("archiving old files...\n");
					char command[256];
					int result = -1;
					sprintf(command, "mkdir %s", mGASettings.mArchiveDir);
					result = system(command);
					sprintf(command, "cp ./data/ai/logs/* %s", mGASettings.mArchiveDir);
					result = system(command);
					sprintf(command, "cp ./data/ai/* %s", mGASettings.mArchiveDir);
					result = system(command);
					sprintf(command, "rm ./data/ai/logs/*"); // must remove the old log file or the last experiments logs will be used (same file name)
					result = system(command);
				}
				printf("attempting to load next settings file in chain; %s\n", mGASettings.mNextSettingsFileInChain);
				if (!mGASettings.loadFromFile(mGASettings.mNextSettingsFileInChain)) {
					printf("FATAL ERROR: GAManagerModule::getIndexOfNextIndividual failed to load next settings file in chain\n");
					exit(1);
				}
				printf("success! resetting GAManager Variables...\n");
				reset();
				if (!loadRunKeys()) {
					printf("FATAL ERROR: GAManagerModule::getIndexOfNextIndividual could not load new run keys file (could not get new generation)\n");
					exit(1);
				}
			}
		}
	}
	if (mTotalRunsRemaining <= 0) {
		// attempt to load another generation
		printf("WARNING: ***Generation training finished*** GAManagerModule::getIndexOfNextIndividual()\n");
		
		char command[256];
		int commandResult = 0; // output of system() commands (usually > 0 if there was an error running a tool)
		for (int i = 0; i < mGASettings.mPopulationSizeN; i++) {
			sprintf(command, "./data/ai/a-logger %s %li %li %i %s", mGASettings.mGALogFile, mGASettings.mRunsToDo, mLastGenLoaded, i, mGASettings.mSummaryFile);
			commandResult = system(command);
			printf("-executed log compiler with system() return value = %i\n", commandResult);
			if (commandResult > 0) {
				exit(1);
			}
		}

		sprintf(command, "./data/ai/b-ranker %s %li %li %li %s %s %s %s 0", mGASettings.mSummaryFile, mLastGenLoaded, mGASettings.mPopulationSizeN, mGASettings.mNumerOfParentsP, mGASettings.mGenePoolFile, mGASettings.mParentsFile, mGASettings.mEvolutionGraphFile, mGASettings.mChampsFile);
		commandResult = system(command);
		printf("-executed fitness ranker with system() return value = %i\n", commandResult);
		if (commandResult > 0) {
			exit(1);
		}
		sprintf(command, "./data/ai/c-breeder %li %li %li %i %i %li 2 %s %s %s %s", mLastGenLoaded + 1, mGASettings.mNumerOfParentsP, mGASettings.mPopulationSizeN, mGASettings.mRadiationPercentageR, mGASettings.mMutationLevelM, mGASettings.mRunsToDo, mGASettings.mParentsFile, mGASettings.mGenePoolFile, mGASettings.mRunCountDownFile, mGASettings.mChampsFile);
		commandResult = system(command);
		printf("-executed breeding vat with system() return value = %i\n", commandResult);
		if (commandResult > 0) {
			exit(1);
		}
		
		if (!loadRunKeys()) {
			printf("WARNING: GAManagerModule::getIndexOfNextIndividual() could not load new run keys file (could not get new generation)\n");
			return -1;
		}
	} else {
		printf("GAMAN sticking to current gen as %li runs are remaining...\n", mTotalRunsRemaining);
	}
	if (mRunEntries[mBatchToAllocate].mCountOfUncompletedRuns > 0) {
		runsGranted = runsRequested;
		gen = mRunEntries[mBatchToAllocate].mGeneration;
		batch = mRunEntries[mBatchToAllocate].mBatch;
		int retVal = mBatchToAllocate;
		mBatchToAllocate++; // start allocating next individual
		mBatchToAllocate = mBatchToAllocate % mCountOfEntries; // loop back around to first individual when at end
		return retVal;
	} else {
		for (int i = 0; i < mCountOfEntries; i++) {
			if (mRunEntries[i].mCountOfUncompletedRuns > 0) { // mCountOfUncompletedRuns changed because some were just sitting there hogging runs
				runsGranted = runsRequested;
				gen = mRunEntries[i].mGeneration;
				batch = mRunEntries[i].mBatch;
			
				mBatchToAllocate++; // start allocating next individual
				mBatchToAllocate = mBatchToAllocate % mCountOfEntries; // loop back around to first individual when at end
				return i;
			}
		}
	}
	printf("WARNING: Individual with runs not found GAManagerModule::getIndexOfNextIndividual()\n");
	return -1;
}

bool GAManagerModule::updateCompletedRuns(long int gen, long int batch, long int runsCompleted) {
	for (int i = 0; i < mCountOfEntries; i++) { // first find matching entry
		if ((mRunEntries[i].mGeneration == gen) && (mRunEntries[i].mBatch == batch)) {
		
			if (mRunEntries[i].mCountOfUncompletedRuns > 0) { // subtract any valid runs from total remaining (ignore extras)
				if (runsCompleted < mRunEntries[i].mCountOfUncompletedRuns) {
					mTotalRunsRemaining -= runsCompleted;
					mRunEntries[i].mCountOfUncompletedRuns -= runsCompleted;
				} else {
					mTotalRunsRemaining -= mRunEntries[i].mCountOfUncompletedRuns;
					mRunEntries[i].mCountOfUncompletedRuns = 0;
				}
			}
			
			return true;
		}
	}
	//printf("ERROR: could not find matching invidual (G:%li B:%li) in memory to update %li completed runs for GAManagerModule::updateCompletedRuns\n", gen, batch, runsCompleted);
	return false;
}

bool GAManagerModule::saveRunKeys() {
	FILE* runFile = fopen(mGASettings.mRunCountDownFile, "w");
	for (int i = 0; i < mCountOfEntries; i++) {
		fprintf(runFile, "G:%li B:%li R:%li\n", mRunEntries[i].mGeneration, mRunEntries[i].mBatch, mRunEntries[i].mCountOfUncompletedRuns);
	}
	fclose(runFile);
	return true;
}

