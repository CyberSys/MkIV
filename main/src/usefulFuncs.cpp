#include "usefulFuncs.h"
#include <math.h>
using namespace std;

// strip a string out of another string from an index to a max size, including whitespace
void stripString(char* instr, int from, char* outstr, int max) {
	int instrlen = strlen(instr);	// get length of input string
	int outi = 0;	// index in output string
	for (int ini = from; ini < instrlen; ini++) { // scan through entire input string
		outstr[outi] = instr[ini];
		outi++;
		if (outi == max - 1) {	// prevent stack smashing of output string
			break;
		}
	}
	outstr[outi] = '\0';	// terminate output string
}

double mpusTokph(double mpus) {
	return mpus * 3600000.0;
}

void legaliseFilename(const char* input, char* output) {
	char temp[256];
	strcpy(temp, input); // make working copy of input string
	int length = strlen(temp);
	for (int i = 0; i < length; i++) {
		if ((temp[i] >= 65) && (temp[i] <= 90)) { // convert to lowercase (tidier on Unix)
			temp[i] = temp[i] + 32; 
		}
		if ((temp[i] < 97) || (temp[i] > 122)) { // replace all possible illegal characters with underscore
			temp[i] = '_';
		}
	}
	if (temp[0] == '_') { // make sure that the first char isn't one of our underscores (just in case)
		temp[0] = 'a';
	}
	if (length >= 19) { // limit filename to 20 chars (leaving 4 to add a .extension later )
		temp[19] = '\0';
	}
	strcpy(output, temp); // prepare output
}

// put a radian back into the 0-2pi range
void normaliseRadian(double& radian) {
	while (radian < 0.0) {
		radian += 2.0f * M_PI;
	}
	while (radian >= 2.0f * M_PI) {
		radian -= 2.0f * M_PI;
	}
}

// put a radian back into the 0-2pi range
void normaliseRadian(float& radian) {
	while (radian < 0.0) {
		radian += 2.0f * M_PI;
	}
	while (radian >= 2.0f * M_PI) {
		radian -= 2.0f * M_PI;
	}
}

double sqDistanceBetween(const double& xi, const double& zi, const double& xf, const double& zf) {
	double xDist = fabs(xi - xf); // Calculate 1D distance btwn points
	double zDist = fabs(zi - zf); // Calculate 1D distance btwn points
	double sqDistance = xDist * xDist + zDist * zDist; // 2D squared distance
	return sqDistance;
}

