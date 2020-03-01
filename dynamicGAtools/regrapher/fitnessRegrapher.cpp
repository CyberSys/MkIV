/* Redraws fitness graphs using HOF data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h> // for PI constants
using namespace std;

int main(int argc, char **argv) {
	printf("fitness ranker\n");
	// validate parameters
	if (argc < 3) {
		printf("Usage is ./regrapher INPUT.xy OUTPUT.xy [N]\nIf N is specified then regrapher will plot individuals on the x axis rather than generation");
		return -1;
	}
	
	// store parameters
	char inFile[256];
	strcpy(inFile, argv[1]);
	
	// open the file
	ifstream file;
	file.open(inFile);
	if (!file) { // validate file
		printf("Error: Could not open file %s!\n", inFile);
		return 1;
	}
	
	FILE* outFile = fopen(argv[2], "w");
	
	bool plotIndivs = false;
	long int indivCount = 0;
	if (argc == 4) {
		plotIndivs = true;
		sscanf(argv[3], "%li", &indivCount);
	}
	
	double prevFitness = 100.0;
	double prevYErr = 0.0;
	
	char lineText[256];
	while (file.getline(lineText, 256)) {
		if (lineText[0] == '#') { // ignore comments
		} else if (strcmp(lineText, "") == 0) { // ignore
		} else {
			// scan line
			long int xdp = -1;
			double ydp = 0.0;
			double yerr = 0.0;
			// 0 0.477478 0.126911 {X Y YERR}
			sscanf(lineText, "%li %lf %lf", &xdp, &ydp, &yerr);
			
			if (prevFitness < ydp) {
				// plot {xdp prevFitness prevYerr}
				if (plotIndivs) {
					fprintf(outFile, "%li %lf %lf\n", xdp*indivCount, prevFitness, prevYErr);
				} else {
					fprintf(outFile, "%li %lf %lf\n", xdp, prevFitness, prevYErr);
				}
			} else {
				// plot current line
				if (plotIndivs) {
					fprintf(outFile, "%li %lf %lf\n", xdp*indivCount, ydp, yerr);
				} else {
					fprintf(outFile, "%li %lf %lf\n", xdp, ydp, yerr);
				}
				prevFitness = ydp;
				prevYErr = yerr;
			}
		}
	}
	fclose(outFile);
	file.close(); // close the file
	
	return 0; // EXIT_SUCCESS
}

