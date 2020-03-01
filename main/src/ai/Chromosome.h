#ifndef MKIV_CHROM_H_
#define MKIV_CHROM_H_

#include <stdio.h>
using namespace std;

struct Chromosome {
	Chromosome();
	bool loadRBFromString(char* chromString);
	bool loadRBFromGenePoolToMatch(char* genePoolFile, long int g, long int b);
	
	char mChromosomeString[256];
	long int mGeneration;
	long int mBatch;
	int mRuleBaseTSSpeed[3][3];
	int mRuleBaseOASpeed[3][3];
	int mRuleBaseTSSteer[3][3];
	int mRuleBaseOASteer[3][3];
};

#endif

