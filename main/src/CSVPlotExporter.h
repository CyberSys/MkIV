#ifndef MARKIV_PLOTEXPORTERCSV_H_
#define MARKIV_PLOTEXPORTERCSV_H_

#include <fstream>

/* little class to export a set of data plots into CSV format for reading into spreadsheets and things
 * file is called "TIMESTAMP_PLOTNAME.csv"
 * can also be set to save in gnuplot format
 */
class CSVPlotExporter {
public:
	CSVPlotExporter();
	bool startCSVFile(const char* fileSuffix, bool gnuformat = false);
	bool writePlotTitle(const char* title, bool gnuformat = false);
	bool writeXTitle(const char* title, bool gnuformat = false);
	bool writeYTitle(const char* title, bool gnuformat = false);
	bool add2dPoint(float x, float y, bool gnuformat = false);
	bool closeCSVFile();
private:
	FILE* mpFile;
};

#endif

