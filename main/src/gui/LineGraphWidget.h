#ifndef MARKIV_LINEGRAPH_WIDGET_H_
#define MARKIV_LINEGRAPH_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreBorderPanelOverlayElement.h"
#include "LabelWidget.h"
#include "../CSVPlotExporter.h"
//#include "../scenery/ScenarioDetails.h"

/* This is a widget that will draw a line graph onto the screen inside a semi-transparent panel */

using namespace Ogre;

#define MAXGRAPHDATAPOINTS 1000000
#define GRAPHLINESMAX 10

// a data point along a line (linked list based)
struct LineDataPoint {
	bool deleteNextPoints();
	LineDataPoint* next; // next point (or NULL)
	LineDataPoint* prev; // next point (or NULL)
	float x; // real x value (only floats at this stage are supported)
	float y; // real y value (only floats at this stage are supported)
};

// representation of a line on the graph (collection of variables associated with each line)
struct GraphLine {
	ManualObject* mpLineObject; // for making lines
	SceneNode* mpLineNode; // for making lines
	MaterialPtr mpLineMaterial; // for making lines
	LineDataPoint* mpLatestDataPoint; // linked list of data points
	LineDataPoint* mpEarliestDataPoint;
	char mLineName[256]; // name of this data plot
	int mCurrentDataPoints;
	int mMaximumDataPoints; // max nodes to store in memory (to prevent mem leak)
};

// a widget for line graphs - does static and dynamic (real-time scrolling monitor) graphs
class LineGraphWidget {
public:
	// general widget stuff
	LineGraphWidget(); // constructor sets default var values
	bool setWidgetSize(float xpixels, float ypixels); // change size of widget, and to some extent squish the sub-window to fit in this new size
	bool init(const char* uniqueName, float parentTop, float parentLeft, float parentBottom, float parentRight, float xProp, float yProp, SceneManager* sceneManager);
	bool free(SceneManager* sceneManager); // free allocated memory
	bool defineParentBounds(float top, float left, float bottom, float right);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool show(); // show widget on screen
	bool hide(); // hide widget from view
	bool moveWidget(float xProp, float yProp);	// move to a position relative to size of parent (0.0-1.0)
	bool update(unsigned int mousex, unsigned int mousey, bool lmb);  // changes display if next/prev clicked
	
	// specialised widget stuff
	bool freeLines(SceneManager* sceneManager); // free all lines from graph
	bool setGraphTitle(const char* title); // set the title text for the graph
	bool setXAxisTitle(const char* title); // set title for x axis
	void setXAxisMax(float xAxisMax); // set axis scale
	void setXAxisMin(float xAxisMin); // set axis scale
	void setXAxisScroll(bool scroll = true); // indicate the the x-axis should keep this scale but scroll for new values (like a CPU monitor)
	void setXAxisInterval(float interval);
	void setYAxisMax(float yAxisMax); // set axis scale
	void setYAxisMin(float yAxisMin); // set axis scale
	int createLine(SceneManager* sceneManager, float r = 1.0f, float g = 1.0f, float b = 1.0f); // create a new line with this colour
	bool addPointToLine(int lineIndex, float realXValue, float realYValue, bool respectXIntervals = false, bool optimisePoints = false); // add a new data point to a line
	bool removeAllPointsFromLine(int lineIndex, bool isAxisLineIndex = false); // delete a line's points

	bool savePlotToFile(int lineIndex, bool gnuformat = false); // save to file (to both csv and gnuplot formats)
private:
	bool initAxisLines(SceneManager* sceneManager);
	bool freeAxisLines(SceneManager* sceneManager);
	float getXAxisProportion(float realXValue); // return real value in terms of 0-1 place along the axis
	float getYAxisProportion(float realYValue); // return real value in terms of 0-1 place along the axis
	float getAdjustedScreenXProportion(float graphXProp); // given a proportion of graph width, return this as identity-matrix adjusted full screen proportion
	float getAdjustedScreenYProportion(float graphYProp); // given a proportion of graph width, return this as identity-matrix adjusted full screen proportion
	bool redrawLine(int lineIndex, bool isAxisLineIndex = false); // redraw the line whenever it changes
	
	Overlay* mpOverlay; // overlay panel for drawing background and holding widget bits (except lines)
	OverlayElement* mpBaseElement; // base overlay element
	OverlayContainer* mpContainer; // container for parts of overlay
	BorderPanelOverlayElement* mpBorderPanel; // graphical border around widget
	
	CSVPlotExporter mCSVExporter; // an exporter for saving to csv files
	GraphLine mGraphLine[GRAPHLINESMAX]; // lines to draw on graph (note: index -1 is x-axis and -2 is y-axis)
	GraphLine mAxisLine[2]; // axis lines

	LabelWidget mGraphTitleLabel; // main title of the graph
	LabelWidget mXAxisLabel; // title of x-axis
	LabelWidget mXAxisMaxLabel; // eg "100.00 ms^-1"
	LabelWidget mXAxisMinLabel; // eg "0.00 ms^-1"
	//LabelWidget mYAxisLabel; // title of y-axis	
	LabelWidget mYAxisMaxLabel;
	LabelWidget mYAxisMinLabel;

	char mUniqueName[256];
	float mParentBoundTop; // bound of parent window (i.e. main viewport)
	float mParentBoundLeft; // bound of parent window (i.e. main viewport)
	float mParentBoundBottom; // bound of parent window (i.e. main viewport)
	float mParentBoundRight; // bound of parent window (i.e. main viewport)

	float mWidgetWidthPixels; // in pixels
	float mWidgetHeightPixels; // in pixels
	float mXProp; // position relative to parent
	float mYProp; // position relative to parent
	float mXPos; // position in pixels (absolute)
	float mYPos; // position in pixels (absolute)
	float mGraphingAreaWidth; // sub-window bound of widget that is used for drawing lines
	float mGraphingAreaHeight; // sub-window bound of widget that is used for drawing lines
	float mGraphingAreaLeftMargin; // sub-window bound of widget that is used for drawing lines
	float mGraphingAreaTopMargin; // sub-window bound of widget that is used for drawing lines
	
	float mXAxisMax; // scale of x axis
	float mXAxisMin; // scale of x axis
	float mYAxisMax; // scale of y axis
	float mYAxisMin; // scale of y axis
	float mXInterval; // frequency to record each data point
	
	int mNumLinesForGraph; // number of lines being drawn on graph	
	bool mXAxisScrolling; // if graph scrolls along x-axis
	bool mXAxisPegged; // if graph can autoscale or not
	bool mYAxisPegged; // if graph can autoscale or not
	bool mHiding; // if overlay is hidden
	bool mUseInterval;
};

#endif

