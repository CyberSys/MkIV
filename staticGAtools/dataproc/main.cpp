#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

#define MAXRUNS 10000

int main(int argc, char **argv) {
	// validate parameters
	if (argc < 4) {
		printf("Usage is \"stats INFILE OUTFILE X\"\n");
		return -1;
	}
	
	// store parameters
	char inFile[256];
	char outFile[256];
	strcpy(inFile, argv[1]);
	strcpy(outFile, argv[2]);
	printf("File in: %s, File out: %s\n", inFile, outFile);
	float x;
	sscanf(argv[3], "%f", &x);
	printf("x = %f\n", x);
	
	// open the file
	ifstream file;
	file.open(inFile);
	if (!file) { // validate file
		printf("Error: Could not open file %s!\n", inFile);
		return -1;
	}
	
	// stats
	int n = 0;
	double scores[MAXRUNS];
	float fpvs[MAXRUNS];
	
	// parse file and get count 'n'
	char lineText[256];
	while (file.getline(lineText, 256)) {
		if (lineText[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore
		} else {
			int runid, timelimit, timetaken, score; // dummy vars
			double collisionHeuristic = 0.0f;
			double fitness = 0.0f; // dummy
			float fps = 0.0f; // dummy
			float fpvs = 0.0f;
			sscanf(lineText, "%i %i %i %lf %lf %i %f %f", &runid, &timelimit, &timetaken, &collisionHeuristic, &fitness, &score, &fps, &fpvs);
			scores[n] = score;
			n++; // add to count of runs
		}
	}
	file.close(); // close the file
	printf("n = %i\n", n);
	
	// sum and mean
	double sum = 0.0f;
	for (int i = 0; i < n; i++) {
		sum = sum + scores[i];
	}
	printf("sum = %lf\n", sum);
	double mean = sum / (double)n;
	printf("mean = %lf\n", mean);
	
	// std dev
	double sumOfSqDiffs = 0.0f;
	for (int i = 0; i < n; i++) {
		double diffFromMean = scores[i] - mean;
		double dfmSquared = diffFromMean * diffFromMean;
		sumOfSqDiffs = sumOfSqDiffs + dfmSquared;
	}
	double stdDev = sqrt(sumOfSqDiffs / n);
	printf("stdev = %lf\n", stdDev);
	
	// std error
	double stderr = stdDev / sqrt(n);
	printf("stderr = %lf\n", stderr);
	
	// append results to file
	FILE* secondFile = fopen(outFile, "a+");
	fprintf(secondFile, "%f %lf %lf\n", x, mean, stderr);
	printf("output to file: %f %lf %lf\n", x, mean, stderr);
	return 0;
}

