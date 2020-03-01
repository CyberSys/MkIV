/* Reads a given fitness logfile up to the first 'L' lines and averages them.
 * !!!NOTE: might be worthwhile compiling crash data as well as overall fitness
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

// #define MULTI_FITNESS_COLLECTION // remove this line if want to collect multiple fitnesses from one thing

#define MAXRUNS 100000

char outFile[256];
int l; // this is the number of lines (particles) to stick together for each run
// initially i'm using 10 100m samples i.e. 1km of motion (maybe 1.5 or 2 would be more akin to prev runs)
double fitnessArray[MAXRUNS]; // holds the samples read so far from the file
long int mGlobalGen;
long int mGlobalBatch;

bool compileFitness(int arrayStartIndex) {
	// sum and mean
	double sum = 0.0f;
	int arrayEndIndex = arrayStartIndex + l;
	for (int i = arrayStartIndex; i < arrayEndIndex; i++) {
		sum = sum + fitnessArray[i];
	}
	printf("sum of fitness scores = %lf\n", sum);
	double mean = sum / (double)l;
	printf("mean fitness score = %lf\n", mean);
	
	// std dev
	double sumOfSqDiffs = 0.0f;
	for (int i = arrayStartIndex; i < arrayEndIndex; i++) {
		double diffFromMean = fitnessArray[i] - mean;
		double dfmSquared = diffFromMean * diffFromMean;
		sumOfSqDiffs = sumOfSqDiffs + dfmSquared;
	}
	double stdDev = sqrt(sumOfSqDiffs / l);
	printf("stdev = %lf\n", stdDev);
	
	// std error
	double stderr = stdDev / sqrt(l);
	printf("stderr = %lf\n", stderr);
	
	// append results to file
	FILE* secondFile = fopen(outFile, "a+");
	fprintf(secondFile, "GEN:%li BATCH:%li MEANFITNESS:%lf STDERR:%lf RUNS:%i\n", mGlobalGen, mGlobalBatch, mean, stderr, l);
	printf("output to file: %lf %lf\n", mean, stderr);
	fclose(secondFile);
	
	return true;
}

int main(int argc, char **argv) {
	printf("fitness logfile compiler\n");
	
	if (argc < 6) { // validate parameters
		printf("Usage is \"./summfitlog INFILE R G B OUTFILE\"\n");
		return -1;
	}
	
/* // TODO check for LOCK file
	if (lock exists) {
		abort process
	} else {
		write lock file
	} */
	
	// store parameters
	char inFile[256];
	
	strcpy(inFile, argv[1]);
	l = 0;
	sscanf(argv[2], "%i", &l);
	printf("runs (lines) to read in = %i\n", l);	
	sscanf(argv[3], "%li", &mGlobalGen);
	printf("mGlobalGen = %li\n", mGlobalGen);
	sscanf(argv[4], "%li", &mGlobalBatch);
	printf("mGlobalBatch = %li\n", mGlobalBatch);
	strcpy(outFile, argv[5]);
	printf("File in: %s, File out: %s\n", inFile, outFile);
	
	
	// open the file
	ifstream file;
	file.open(inFile);
	if (!file) { // validate file
		printf("Error: Could not open file %s!\n", inFile);
		return 1;
	}
	
	// stats
	int n = 0;
	int countOfRunsCompiled = 0;
	
	float fpvs[MAXRUNS];
	
	// parse file and get count 'n'
	char lineText[256];
	while (file.getline(lineText, 256)) {
		if (lineText[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore
		} else {
			double mileage = 0.0;
			double crashRating = 0.0;
			double speedRating = 0.0;
			double speedInstance = 0.0;
			double topSpeed = 0.0;
			double fitness = 0.0;
			long int gen = 0;
			long int batch = 0;
			
			sscanf(lineText, "G:%li B:%li MILEAGE: %lf and CRASH-RATING: %lf + SPEED-RATING: %lf (1.0 - %lf / %lf) = FITNESS: %lf", &gen, &batch, &mileage, &crashRating, &speedRating, &speedInstance, &topSpeed, &fitness);
			
			if ((gen != mGlobalGen) || (batch != mGlobalBatch)) {
				//printf("ERROR: found a gen/batch (should be g:%li b:%li) mismatch in log - ignoring. line was: %s\n", mGlobalGen, mGlobalBatch, lineText);
				continue;
			}
			
			fitnessArray[n] = fitness;
			n++; // add to count of runs
			if (n  % l == 0) {
				compileFitness(n - l); // compile collected runs into a fitness function
				countOfRunsCompiled++;
				break; // TODO - remove this line if want to collect multiple fitnesses from one thing
			}
		}
	}
	file.close(); // close the file
	printf("numer of run particles read = %i\n", n);
	printf("numer of runs compiled = %i\n", countOfRunsCompiled);
	if (n < l) {
		printf("FAILURE: line count of %i was less than required sample size of %i.", n, l);
	} else {
	#ifdef MULTI_FITNESS_COLLECTION
		char archiveName[256];
		strcpy(archiveName, "archive/");
		strcat(archiveName, inFile);
		char systemString[256];
		sprintf(systemString, "mv %s %s", inFile, archiveName);
		system(systemString);
		sprintf(systemString, "rm %s", inFile);
		system(systemString);
	#endif
	}	
	
	/* // TODO remove LOCK file */
	
	return 0; // EXIT_SUCCESS
}

