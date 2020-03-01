#include "Chromosome.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

Chromosome::Chromosome() {
	mGeneration = 0;
	mBatch = 0;
	for (int a = 0; a < 3; a++) {
		for (int d = 0; d < 3; d++) {
			mRuleBaseTSSpeed[d][a] = 0;
			mRuleBaseOASpeed[d][a] = 0;
			mRuleBaseTSSteer[d][a] = 0;
			mRuleBaseOASteer[d][a] = 0;
		}
	}
}

bool Chromosome::loadRBFromString(char* chromString) {
	int length = strlen(chromString);
	if (length != 108) {
		printf("ERROR Chromosome::loadRBFromString: chromosome corrupted: %i/108 digits long. Chromosome = [%s]\n", length, chromString);
		return false;
	}
	for (int i = 0; i < 9; i++) {
		int inputDistanceIndex = 0 + i * 3;
		int inputAngleIndex = 1 + i * 3;
		int ruleIndex = 2 + i * 3;
		int inputA = (int)chromString[inputDistanceIndex] - 48;
		int inputB = (int)chromString[inputAngleIndex] - 48;
		mRuleBaseTSSpeed[inputA][inputB] = (int)chromString[ruleIndex] - 48;
	}
	for (int i = 9; i < 18; i++) {
		int inputDistanceIndex = 0 + i * 3;
		int inputAngleIndex = 1 + i * 3;
		int ruleIndex = 2 + i * 3;
		int inputA = (int)chromString[inputDistanceIndex] - 48;
		int inputB = (int)chromString[inputAngleIndex] - 48;
		mRuleBaseOASpeed[inputA][inputB] = (int)chromString[ruleIndex] - 48;
	}
	for (int i = 18; i < 24; i++) {
		int inputDistanceIndex = 0 + i * 3;
		int inputAngleIndex = 1 + i * 3;
		int ruleIndex = 2 + i * 3;
		int inputA = (int)chromString[inputDistanceIndex] - 48;
		int inputB = (int)chromString[inputAngleIndex] - 48;
		mRuleBaseTSSteer[inputA][inputB] = (int)chromString[ruleIndex] - 48;
	}
	for (int i = 24; i < 36; i++) {
		int inputDistanceIndex = 0 + i * 3;
		int inputAngleIndex = 1 + i * 3;
		int ruleIndex = 2 + i * 3;
		int inputA = (int)chromString[inputDistanceIndex] - 48;
		int inputB = (int)chromString[inputAngleIndex] - 48;
		mRuleBaseOASteer[inputA][inputB] = (int)chromString[ruleIndex] - 48;
	}
	strcpy(mChromosomeString, chromString);
	return true;
}

bool Chromosome::loadRBFromGenePoolToMatch(char* genePoolFile, long int g, long int b) {
	ifstream genePool;
	genePool.open(genePoolFile);
	if (!genePool) { // validate file
		printf("Error: Could not open %s in Chromosome::loadRBFromGenePoolToMatch()\n", genePoolFile);
		return false;
	}
	bool found = false;
	char chromosome[256];
	char lineText[256];
	while (genePool.getline(lineText, 256)) {
		long int gpg = -1;
		long int gpb = -1;
		sscanf(lineText, "G:%li B:%li %s", &gpg, &gpb, chromosome);
		if ((gpg == g) && (gpb == b)) {
			found = true;
			if (!loadRBFromString(chromosome)) {
				printf("Error: Could not use the chrom. that we found from file. Chromosome::loadRBFromGenePoolToMatch()\n");
				return false;
			}
			mGeneration = g;
			mBatch = b;
			break;
		}
	}
	genePool.close();
	if (!found) {
		printf("ERROR: could not find matching chromosome in genepool.txt Chromosome::loadRBFromGenePoolToMatch()\n");
		return false;
	}
	return true;
}

