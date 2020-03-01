#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

#define MAXFILES 100

int ruleValue[36];

void resetRules() {
	ruleValue[0] = 1;
	ruleValue[1] = 1;
	ruleValue[2] = 1;
	ruleValue[3] = 4;
	ruleValue[4] = 3;
	ruleValue[5] = 2;
	ruleValue[6] = 5;
	ruleValue[7] = 4;
	ruleValue[8] = 3;

	ruleValue[9] = 0;
	ruleValue[10] = 1;
	ruleValue[11] = 2;
	ruleValue[12] = 0;
	ruleValue[13] = 2;
	ruleValue[14] = 3;
	ruleValue[15] = 0;
	ruleValue[16] = 3;
	ruleValue[17] = 4;

	ruleValue[18] = 5;
	ruleValue[19] = 4;
	ruleValue[20] = 3;
	ruleValue[21] = 4;
	ruleValue[22] = 3;
	ruleValue[23] = 2;
	ruleValue[24] = 1;
	ruleValue[25] = 2;
	ruleValue[26] = 1;

	ruleValue[27] = 5;
	ruleValue[28] = 4;
	ruleValue[29] = 2;
	ruleValue[30] = 4;
	ruleValue[31] = 3;
	ruleValue[32] = 1;
	ruleValue[33] = 2;
	ruleValue[34] = 1;
	ruleValue[35] = 0;
}

int main(int argc, char **argv) {
	// take a given "best" member (or 2-3) and run all the genetic operators on them - so far we only do mass mutation.
	
	// 0. fire out a large range of disparate sets to try?? or just tune an initial one
	
	// 1. preselect best sets:
	// a. collect all aggregated logs.
	// b. put in db file and rank
	// c. do selection of best $2$
	
	// 2. do some crossover of the best sets (50% from one and 50% from the other) -- only if there are relatively high scoring, else just take the superior one. create ~4 random mixtures
	
	// 3. mutate by 1 on X random genes
	
	for (int iteration = 0; iteration < 72; iteration++) {
		int ruleNumber = iteration % 36;
		resetRules();
		if (iteration < 36) {
			ruleValue[ruleNumber]--;
			if (ruleValue[ruleNumber] < 0) {
				ruleValue[ruleNumber] = 0;
			}
		} else {
			ruleValue[ruleNumber]++;
			if (ruleValue[ruleNumber] > 6) {
				ruleValue[ruleNumber] = 6;
			}
		}
		char outFileName[256];
		sprintf(outFileName, "brute-rules%i.txt", iteration);
		FILE* outFilePtr = fopen(outFileName, "w");
		int dist = 0;
		int angle = 0;
		for (int i = 0; i < 9; i++) {
			fprintf(outFilePtr, "TS SPEED %i & %i = %i\n", dist, angle, ruleValue[i]);
			angle++;
			if (angle == 3) {
				angle = 0;
				dist++;
			}
		}
		dist = angle = 0;
		for (int i = 9; i < 18; i++) {
			fprintf(outFilePtr, "TS STEER %i & %i = %i\n", dist, angle, ruleValue[i]);
			angle++;
			if (angle == 3) {
				angle = 0;
				dist++;
			}
		}
		dist = angle = 0;
		for (int i = 18; i < 27; i++) {
			fprintf(outFilePtr, "OA SPEED %i & %i = %i\n", dist, angle, ruleValue[i]);
			angle++;
			if (angle == 3) {
				angle = 0;
				dist++;
			}
		}
		dist = angle = 0;
		for (int i = 27; i < 36; i++) {
			fprintf(outFilePtr, "OA STEER %i & %i = %i\n", dist, angle, ruleValue[i]);
			angle++;
			if (angle == 3) {
				angle = 0;
				dist++;
			}
		}
		fclose(outFilePtr);
	}
	// # {TS, OA} {SPEED, STEER} [dist] & [angle] = [output]
	return 0;
}

