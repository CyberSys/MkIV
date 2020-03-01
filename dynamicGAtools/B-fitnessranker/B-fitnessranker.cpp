/* Reads a given compiled fitness results file (a whole generation) up to the first 'C' children and ranks them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

#define MAX_POP 10000

struct Fitness;

struct Fitness {
	Fitness* next;
	Fitness* prev;
	long int generationNum;
	long int batchNum;
	long int runs;
	double meanFitness;
	double stdErr;
	Fitness() {
		next = NULL;
		prev = NULL;
		generationNum = 0;
		batchNum = 0;
		runs = 0;
		meanFitness = 0.0;
		stdErr = 0.0;
	}
};

int main(int argc, char **argv) {
	printf("fitness ranker\n");
	// validate parameters
	if (argc < 8) {
		printf("Usage is \"./B-fitnessranker FITNESS-SUMMARY-FILE G N P GENEPOOL_FILE HOF_FILE OUTGRAPH_FILE (where G is the generation number to look for, N is the number of individuals in the generation and P is the number of parents to select for breeding)\"\n");
		return -1;
	}
	
	Fitness* topFitness = NULL;
	bool scannedPop[MAX_POP];
	for (int i = 0; i < MAX_POP; i++) {
		scannedPop[i] = false;
	}
	
	// store parameters
	char inFile[256];
	strcpy(inFile, argv[1]);
	long int generationNumber = -1;
	sscanf(argv[2], "%li", &generationNumber);
	printf("* generation number = %li\n", generationNumber);
	int c = 0;
	sscanf(argv[3], "%i", &c);
	printf("* lines (children) to read in = %i\n", c);
	int p = 0;
	sscanf(argv[4], "%i", &p);
	printf("* parents to select = %i\n", p);
	char genepoolfile[256];
	strcpy(genepoolfile, argv[5]);
	printf("File in: %s, File out: fitnessEvoGraph.xy\n", inFile);
	char hoffile[256];
	strcpy(hoffile, argv[6]);
	char graphfile[256];
	strcpy(graphfile, argv[7]);
	
	// open the file
	ifstream file;
	file.open(inFile);
	if (!file) { // validate file
		printf("Error: Could not open file %s!\n", inFile);
		return 1;
	}
	
	// stats
	int n = 0;
	
	// parse file and get count 'n'
	char lineText[256];
	while (file.getline(lineText, 256)) {
		if (lineText[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore
		} else {
			// scan line
			long int tempGen = -1;
			long int tempBatch = -1;
			double tempFit = 666.0;
			double tempErr = 666.0;
			long int tempRuns = -1;
			sscanf(lineText, "GEN:%li BATCH:%li MEANFITNESS:%lf STDERR:%lf RUNS:%li", &tempGen, &tempBatch, &tempFit, &tempErr, &tempRuns);
			if (tempGen != generationNumber) { // check generation
				continue; // not ours so skip
			}
			if (scannedPop[tempBatch]) {
				printf("WARNING: double-entry found for G:%li B:%li, skipping entries after the first\n", tempGen, tempBatch);
				continue; // double-up so skip
			}
			
			// create new entry
			Fitness* currentFitness = new Fitness;
			currentFitness->generationNum = tempGen;
			currentFitness->batchNum = tempBatch;
			currentFitness->meanFitness = tempFit;
			currentFitness->stdErr = tempErr;
			currentFitness->runs = tempRuns;
			
			if (n == 0) { // put at start of list
				topFitness = currentFitness;
			} else if (n == 1) { // one item in list
				if (currentFitness->meanFitness < topFitness->meanFitness) {
					currentFitness->next = topFitness;
					topFitness->prev = currentFitness;
					topFitness = currentFitness;
				} else {
					currentFitness->prev = topFitness;
					topFitness->next = currentFitness;
				}
			} else { // two or more items in list
				Fitness* ptr = topFitness;
				while (ptr != NULL) { // put in-between two existing items
					if (currentFitness->meanFitness < ptr->meanFitness) {
						currentFitness->next = ptr;
						currentFitness->prev = ptr->prev;
						if (!ptr->prev) {
							topFitness = currentFitness;
						} else {
							ptr->prev->next = currentFitness;
						}
						ptr->prev = currentFitness;
						break;
					}
					if (!ptr->next) { // put at end of list
						ptr->next = currentFitness;
						currentFitness->prev = ptr;
						break;
					}
					ptr = ptr->next;
				}
			}
			
			n++; // add to count of runs			
			if (n == c) {
				break;
			}
		}
	}
	file.close(); // close the file
	printf("n = %i\n", n);
	if (n < c) {
		printf("FAILURE: line count of %i was less than required sample size of %i\nnot writing results.", n, c);
		return 1; // EXIT_FAILURE
	}
	
	// SELECT winners
	if ((n > 1) && (n >= p)) {
		int parentsAdded = 0;
		printf("Top ranking individuals are:\n");
		Fitness* winningFitness = topFitness;
		
		FILE* hof = fopen(hoffile, "a+");
		while (parentsAdded < p) {
			if (winningFitness == NULL) {
				printf("ERROR: not enough parents available, exit\n");
				return 1;
			}

			printf("%lf %lf G:%li B:%li R:%li\n", winningFitness->meanFitness, winningFitness->stdErr, winningFitness->generationNum, winningFitness->batchNum, winningFitness->runs);
			
			// find chromosome for this individual
			printf("looking up chromosome for this individual...\n");
			// open the file
			ifstream genePool;
			genePool.open(genepoolfile);
			if (!genePool) { // validate file
				printf("Error: Could not open %s\n", genepoolfile);
				return 1;
			}
			bool found = false;
			char chromosome[256];
			while (genePool.getline(lineText, 256)) {
				long int gpg = -1;
				long int gpb = -1;
				sscanf(lineText, "G:%li B:%li %s", &gpg, &gpb, chromosome);
				if ((gpg == winningFitness->generationNum) && (gpb == winningFitness->batchNum)) {
					printf("found chromosome: %s\n", chromosome);
					found = true;
					printf("adding the parent to Hall Of Fame...\n");
					fprintf(hof, "%lf %lf G:%li B:%li R:%li %s\n", winningFitness->meanFitness, winningFitness->stdErr, winningFitness->generationNum, winningFitness->batchNum, winningFitness->runs, chromosome);
					break;
				}
			}
			genePool.close();
			if (!found) {
				printf("ERROR: could not find matching chromosome in genepool.txt\n");
				return 1;
			}
		
			winningFitness = winningFitness->next;
			parentsAdded++;
		}
		fclose(hof);
		
		printf("Re-Sorting Hall Of Fame\n");
		char tmp[256];
		sprintf(tmp, "sort -g %s > hof2.txt", hoffile);
		system(tmp); // "man sort" or "info sort"
		sprintf(tmp, "cp hof2.txt %s", hoffile);
		system(tmp);
		system("rm hof2.txt");
	} else {
		printf("SELECTION FAILED: not enough children in database\n");
	}
	
	// append the top score from this generation to a .xy file for graphing
	FILE* secondFile = fopen(graphfile, "a+");
	// GEN# TOPFITNESS STDERR
	fprintf(secondFile, "%li %lf %lf\n", topFitness->generationNum, topFitness->meanFitness, topFitness->stdErr);
	printf("output to graph file: %li %lf %lf\n", topFitness->generationNum, topFitness->meanFitness, topFitness->stdErr);
	fclose(secondFile);
	
	return 0; // EXIT_SUCCESS
}

