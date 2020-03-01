#ifndef USEFUL_FUNCTIONS_H_
#define USEFUL_FUNCTIONS_H_

#include "handyMacros.h" // some useful pre-processor stuff
#include <string.h>
using namespace std;

// strip a string out of another string from an index to a max size, including whitespace
void stripString(char* instr, int from, char* outstr, int max);
double mpusTokph(double mpus);
void legaliseFilename(const char* input, char* output);
void normaliseRadian(double& radian);
void normaliseRadian(float& radian);
//! get the squared distance between two 2D positions
double sqDistanceBetween(const double& xi, const double& zi, const double& xf, const double& zf);

#endif
