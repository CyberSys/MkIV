#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <time.h>
#include <math.h> // for PI constants
using namespace std;

#define MAX_PARENTS 100
#define MAX_FUZZY_RULE_VALUE 5

char parentGenes[MAX_PARENTS][256];

int main(int argc, char **argv) {
	printf("breeding vat\n");
	
	if (argc < 10) { // validate parameters
		printf("Usage is \"./C-breedingvat G P N R M T HALLOFFAME_FILE GENEPOOL_FILE RUNCOUNT_FILE (where G is the number of the generation to breed now, P is number of parents to use, and N is the population size to generate, and R is the 'radiation level' or percent chance that each gene will be mutated, and M is the mutation level - or amount a gene can be mutated up to if irradiated, and T is the amount of training runs to assign for training each new individual)\n");
		return -1;
	}
	
	long int g = 0;
	long int p = 0;
	long int n = 0;
	int r = 0;
	int m = 0;
	long int t = 0;
	sscanf(argv[1], "%li", &g);
	sscanf(argv[2], "%li", &p);
	sscanf(argv[3], "%li", &n);
	sscanf(argv[4], "%i", &r);
	sscanf(argv[5], "%i", &m);
	sscanf(argv[6], "%li", &t);
	char halloffamefile[256];
	strcpy(halloffamefile, argv[7]);
	char genepoolfile[256];
	strcpy(genepoolfile, argv[8]);
	char runcountfile[256];
	strcpy(runcountfile, argv[9]);
	
	ifstream file; // open file
	file.open(halloffamefile);
	if (!file) { // validate file
		printf("Error: Could not open file HallOfFame.txt!\n");
		return 1;
	}
	
	// 1) read top P chromosome(s) from HallOfFame.txt
	int pScanned = 0;
	char lineText[256];
	long int lastGene = -1;
	long int lastBatch = -1;
	while (file.getline(lineText, 256)) {
		if (lineText[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore
		} else {
			double fitness = 0.0;
			double error = 0.0;
			long int gen = -1;
			long int batch = -1;
			long int runs = -1;
			char chromosome[256];
			sscanf(lineText, "%lf %lf G:%li B:%li R:%li %s", &fitness, &error, &gen, &batch, &runs, chromosome);
			
			// double-check that we don't have a double-up of the previous one
			if ((gen == lastGene) && (batch == lastBatch)) {
				printf("ERROR: double-up found in HOF, attempting to continue...\n");
				continue;
			}
			strcpy(parentGenes[pScanned], chromosome);
			lastGene = gen;
			lastBatch = batch;
			pScanned++;
			if (pScanned == p) {
				break;
			}
		}		
	}
	file.close(); // close the file
	
	// make sure that there are at least P parents in the HallOfFame.txt
	if (pScanned < p) {
		printf("ERROR: not enough entries in HOF to meet Parent criterea P\n");
		return 1;
	}
	
	srand(time(NULL));
	FILE* genePoolFile = fopen(genepoolfile, "w");
	FILE* runKeyFile = fopen(runcountfile, "w");
	
	for (long int i = 0; i < n; i++) { // keep breeding until this many children made
		// straight copy of best parent first
		char childChromosome[256];
		strcpy(childChromosome, parentGenes[0]);

		for (int ruleNumber = 0; ruleNumber < 36; ruleNumber++) {
			int ruleResultIndex = ruleNumber * 3 + 2; // work out string index of rule integer
				
			if (p == 2) {
				//printf("attempting crossover - at this stage only 2 parents with a 50/50 blend supported\n");
				// 2) recombine/crossover
				int coinflip = rand() % 2;
				if (coinflip > 0) { // use second parent gene (first is used by default)
					childChromosome[ruleResultIndex] = parentGenes[1][ruleResultIndex];
				}
			}
				
			// 3) create ~4-X random mixtures by mutating 1-M genes
			int diceRoll = rand() % 100;
			if (diceRoll < r) { // radiation (check if gene is mutated)
				int swampThingAmount = rand() % (m * 2 + 1);
				swampThingAmount -= m;
				int oldRuleAsInt = (int)childChromosome[ruleResultIndex] - 48;
				oldRuleAsInt += swampThingAmount; // mutate
				if (oldRuleAsInt < 0) { // check bounds
					oldRuleAsInt = 0;
				} else if (oldRuleAsInt > 5) { // check bounds
					oldRuleAsInt = 5;
				}
				childChromosome[ruleResultIndex] = oldRuleAsInt + 48; // store as char
			}
		}
		
		// add entry to gene pool for next simulation assignment
		fprintf(genePoolFile, "G:%li B:%li %s\n", g, i, childChromosome);
		
		// also print a RUNS required meta-data file
		fprintf(runKeyFile, "G:%li B:%li R:%li\n", g, i, t);
	}
	fclose(runKeyFile);
	fclose(genePoolFile);
	
	return 0;
}

