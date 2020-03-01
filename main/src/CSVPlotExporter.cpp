#include "CSVPlotExporter.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace std;

// defaults
CSVPlotExporter::CSVPlotExporter() {
	mpFile = NULL;
}

// open a file, ready for data points to be added
bool CSVPlotExporter::startCSVFile(const char* fileSuffix, bool gnuformat) {
	time_t seconds = time(NULL);
    
	char filename[256];
	if (!gnuformat) {
		sprintf(filename, "graphplots/%ld_%s.csv", seconds, fileSuffix); // work out a filename based on the fileSuffix and the time
	} else {
		sprintf(filename, "graphplots/%ld_%s.gnuplot", seconds, fileSuffix); // work out a filename based on the fileSuffix and the time
	}

	mpFile = fopen(filename, "w"); // Open file
	if (!mpFile) { // validate
		printf("ERROR: Could not open csv file %s for writing\n", filename);
		return false;
	}

	return true;
}

bool CSVPlotExporter::writePlotTitle(const char* title, bool gnuformat) {
if (!mpFile) { // validate
		printf("ERROR: a csv has not been opened for writing\n");
		return false;
	}
	if (!gnuformat) {
		fprintf(mpFile, "\"%s\",\n", title); // print
	} else {
		fprintf(mpFile, "%s\n", title); // print
	}

	return true;
}

bool CSVPlotExporter::writeXTitle(const char* title, bool gnuformat) {
	if (!mpFile) { // validate
		printf("ERROR: a csv has not been opened for writing\n");
		return false;
	}
	if (!gnuformat) {
		fprintf(mpFile, "\"%s\",", title); // print
	} else {
		fprintf(mpFile, "%s ", title); // print
	}
	return true;
}

bool CSVPlotExporter::writeYTitle(const char* title, bool gnuformat) {
	if (!mpFile) { // validate
		printf("ERROR: a csv has not been opened for writing\n");
		return false;
	}
	if (!gnuformat) {
		fprintf(mpFile, "\"%s\",\n", title); // print
	} else {
		fprintf(mpFile, "%s\n", title); // print
	}
	return true;
}

bool CSVPlotExporter::add2dPoint(float x, float y, bool gnuformat) {
	if (!mpFile) { // validate
		printf("ERROR: a csv has not been opened for writing\n");
		return false;
	}
	if (!gnuformat) {
		fprintf(mpFile, "%f,%f,\n", x, y); // print
	} else {
		fprintf(mpFile, "%f %f\n", x, y); // print
	}
	return true;
}

bool CSVPlotExporter::closeCSVFile() {
	if (!mpFile) { // validate
		printf("ERROR: can't close file: a csv has not been opened for writing\n");
		return false;
	}
	fclose(mpFile);	// close file (temp)
	mpFile = NULL;
	return true;
}

