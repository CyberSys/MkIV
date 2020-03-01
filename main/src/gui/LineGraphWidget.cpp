#include "LineGraphWidget.h"
#include "../globals.h"

// auto-delete the whole line point by point
bool LineDataPoint::deleteNextPoints() {
	if (next == NULL) {
		return true; // send signal that we are the last point left and can be deleted
	}
	if (next->deleteNextPoints()) { // tell next point to delete all after it
		delete next; // delete the last point
		next = NULL; // set it back to null
	} else {
		return false;
	}
	return true; // signal that we are ready to be deleted
}

LineGraphWidget::LineGraphWidget() {
	mpOverlay = NULL;
	mpBaseElement = NULL;
	mpContainer = NULL;
	mpBorderPanel = NULL;

	strcpy(mUniqueName, "");

	mParentBoundTop = 0.0f;
	mParentBoundLeft = 0.0f;
	mParentBoundBottom = 480.0f;
	mParentBoundRight = 640.0f;

	mWidgetWidthPixels = 511.0f;
	mWidgetHeightPixels = 175.0f;
	mXProp = 0.25f;
	mYProp = 0.25f;
	mXPos = 100.0f;
	mYPos = 100.0f;
	mGraphingAreaWidth = 420.0f;
	mGraphingAreaHeight = 120.0f;
	mGraphingAreaLeftMargin = 50.0f;
	mGraphingAreaTopMargin = 25.0f;
	
	mXAxisMax = 100.0f;
	mXAxisMin = 0.0f;
	mYAxisMax = 100.0f;
	mYAxisMin = 0.0f;
	mXInterval = 0.0f;
	
	mNumLinesForGraph = 0;
	
	mXAxisScrolling = false;
	mXAxisPegged = false; // autoscale set to true by default
	mYAxisPegged = false; // autoscale set to true by default
}

bool LineGraphWidget::init(const char* uniqueName, float parentTop, float parentLeft, float parentBottom, float parentRight, float xProp, float yProp, SceneManager* sceneManager) {
	if (!sceneManager) { // validate params
		printf("ERROR: scene manager passed is not valid\n");
		return false;
	}
	
	mParentBoundTop = parentTop;
	mParentBoundLeft = parentLeft;
	mParentBoundBottom = parentBottom;
	mParentBoundRight = parentRight;
	mXProp = xProp;
	mYProp = yProp;
	strcpy(mUniqueName, uniqueName);
	mHiding = false;

	// initialise the overlay for the menu
	char tmp[256];
	sprintf(tmp, "%s_overlay", uniqueName);
	mpOverlay = OverlayManager::getSingleton().create(tmp);
	if (!mpOverlay) {
		printf("ERROR: Could not create LineGraphWidget overlay\n");
		return false;
	}
	mpOverlay->setZOrder(600);	// set render order (0-650 are valid)

	// create a container panel for the widget
	sprintf(tmp, "%s_FB", uniqueName);	// name of the panel
	mpBaseElement = OverlayManager::getSingleton().createOverlayElement("BorderPanel", tmp);
	mpContainer = static_cast<Ogre::OverlayContainer*>(mpBaseElement);
	if (!mpContainer) {
		printf("ERROR: Could not create LineGraphWidget container\n");
		return false;
	}
	// set up the border panel (with another horrible cast)
	mpBorderPanel = static_cast<Ogre::BorderPanelOverlayElement*>(mpBaseElement);

	mpBorderPanel->setMetricsMode(GMM_PIXELS);
	mpBorderPanel->setBorderSize(0.5f);
	mpBorderPanel->setMaterialName("mkiv/FileBrowserBackground");
	mpBorderPanel->setBorderMaterialName("mkiv/ButtonBorder");
	mpBorderPanel->setDimensions(mWidgetWidthPixels, mWidgetHeightPixels);
	mpOverlay->add2D(mpContainer);

	// provide boundaries and move widget into place
	defineParentBounds(mParentBoundTop, mParentBoundLeft, mParentBoundBottom, mParentBoundRight);

	// put a label on the graph
	sprintf(tmp, "%s_graphtitlelabel", uniqueName);
	mGraphTitleLabel.init(mpContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.5f, 0.0f, "Title of this graph");
	//mpContainer->addChild(mGraphTitleLabel.getElement());
	mGraphTitleLabel.centreText();
	mGraphTitleLabel.setFont("BlueHighway");
	mGraphTitleLabel.setSize(20);
	mGraphTitleLabel.show();
	// x-axis label
	sprintf(tmp, "%s_graphxaxislabel", uniqueName);
	mXAxisLabel.init(mpContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.5f, 0.90f, "Time (s)");
	//mpContainer->addChild(mXAxisLabel.getElement());
	mXAxisLabel.centreText();
	mXAxisLabel.setFont("BlueHighway");
	mXAxisLabel.setSize(15);
	mXAxisLabel.show();
	
	float leftXProp = mGraphingAreaLeftMargin / mWidgetWidthPixels; // x prop of the y axis (left of x axis)
	float rightXProp = mGraphingAreaWidth / mWidgetWidthPixels + leftXProp;
	float topYProp = mGraphingAreaTopMargin / mWidgetHeightPixels; // top y prop of y axis
	float bottomYProp = mGraphingAreaHeight / mWidgetHeightPixels + topYProp;
	
	// x-axis max value label
	sprintf(tmp, "%s_graphxaxismaxvaluelabel", uniqueName);
	mXAxisMaxLabel.init(mpContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, rightXProp, 0.9f, "units");
	//mpContainer->addChild(mXAxisMaxLabel.getElement());
	mXAxisMaxLabel.setFont("BlueHighway");
	mXAxisMaxLabel.setSize(15);
	mXAxisMaxLabel.show();
	// x-axis min value label
	sprintf(tmp, "%s_graphxaxisminvaluelabel", uniqueName);
	mXAxisMinLabel.init(mpContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, leftXProp, 0.9f, "units");
	//mpContainer->addChild(mXAxisMinLabel.getElement());
	mXAxisMinLabel.setFont("BlueHighway");
	mXAxisMinLabel.setSize(15);
	mXAxisMinLabel.show();
	// y-axis max value label
	sprintf(tmp, "%s_graphyaxismaxvaluelabel", uniqueName);
	mYAxisMaxLabel.init(mpContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.0125f, topYProp, "units");
	//mpContainer->addChild(mYAxisMaxLabel.getElement());
	mYAxisMaxLabel.setFont("BlueHighway");
	mYAxisMaxLabel.setSize(15);
	mYAxisMaxLabel.show();
	// y-axis min value label
	sprintf(tmp, "%s_graphyaxisminvaluelabel", uniqueName);
	mYAxisMinLabel.init(mpContainer, tmp, mXPos, mYPos, mXPos + mWidgetWidthPixels, mYPos + mWidgetHeightPixels, 0.0125f, bottomYProp, "units");
	//mpContainer->addChild(mYAxisMinLabel.getElement());
	mYAxisMinLabel.setFont("BlueHighway");
	mYAxisMinLabel.setSize(15);
	mYAxisMinLabel.show();
	
	// create axis lines
	if (!initAxisLines(sceneManager)) {
		printf("ERROR: could not init axis lines on graph\n");
		return false;
	}

	return true;
}

// free the whole widget
bool LineGraphWidget::free(SceneManager* sceneManager) {
	if (!sceneManager) { // validate params
		printf("ERROR: scene manager passed is not valid\n");
		return false;
	}
	if (!mpOverlay) { // validate internal memory has been allocated
		printf("ERROR: LineGraphWidget overlay does not exist\n");
		return false;
	}
	if (!mpContainer) { // validate internal memory has been allocated
		printf("ERROR: No LineGraphWidget container allocated\n");
		return false;	// no memory allocated
	}

	if (!freeAxisLines(sceneManager)) { // free all the lines
		printf("ERROR: could not free axis lines from graph widget\n");
	}
	if (!freeLines(sceneManager)) { // free all the lines
		printf("ERROR: could not free lines from graph widget\n");
	}
	mYAxisMinLabel.free();
	mYAxisMaxLabel.free();
	mXAxisMinLabel.free();
	mXAxisMaxLabel.free();
	mXAxisLabel.free(); // free x axis label
	mGraphTitleLabel.free(); // free title
#ifndef HACK_NOGUIFREE
	OverlayManager::getSingleton().destroyOverlayElement(mpBaseElement);
	OverlayManager::getSingleton().destroy(mpOverlay);
#endif
	mpBaseElement = NULL;
	mpContainer = NULL;
	mpBorderPanel = NULL;
	mpOverlay = NULL;	
	return true;
}

// change size of widget, and to some extent squish the sub-window to fit in this new size
bool LineGraphWidget::setWidgetSize(float xpixels, float ypixels) {
	float graphAreaWidthProp = mGraphingAreaWidth / mWidgetWidthPixels;
	float graphAreaHeightProp = mGraphingAreaHeight / mWidgetHeightPixels;
	mWidgetWidthPixels = xpixels;
	mWidgetHeightPixels = ypixels;
	mGraphingAreaWidth = graphAreaWidthProp * mWidgetWidthPixels;
	mGraphingAreaHeight = graphAreaHeightProp * mWidgetHeightPixels;	
	
	return true;
}

// define boundaries (in pixels) of the widget that this one is nested in (so that proportions work properly)
bool LineGraphWidget::defineParentBounds(float top, float left, float bottom, float right) {
	mParentBoundTop = top;
	mParentBoundLeft = left;
	mParentBoundBottom = bottom;
	mParentBoundRight = right;

	if (!mpBaseElement) {
		printf("ERROR: No file LineGraphWidget base element allocated\n");
		return false;	// no memory allocated
	}
	if (!moveWidget(mXProp, mYProp)) {
		printf("ERROR: failed to move file LineGraphWidget\n");
		return false;	// no memory allocated
	}
	return true;
}

bool LineGraphWidget::moveWidget(float xProp, float yProp) {
	mXProp = xProp;
	mYProp = yProp;
	if (!mpBaseElement) {
		printf("ERROR: No file LineGraphWidget element allocated\n");
		return false;	// no memory allocated
	}

	// set position - can't just change metrics mode because all the size attributes are in pixels	
	mXPos = mParentBoundLeft + (mParentBoundRight - mParentBoundLeft) * xProp;	// get screen-proportionate position
	mYPos = mParentBoundTop + (mParentBoundBottom - mParentBoundTop) * yProp;	// get screen-proportionate position
	mpBorderPanel->setLeft(mXPos);
	mpBorderPanel->setTop(mYPos);

	return true;
}

// update the widget - this function should be run every frame. returns false on error
bool LineGraphWidget::update(unsigned int mousex, unsigned int mousey, bool lmb) {

	return true;
}

// show the widget
bool LineGraphWidget::show() {
	if (!mpOverlay) {
		printf("ERROR: No LineGraphWidget Panel in memory to show\n");
		return false;
	}
	mHiding = false;
	mpOverlay->show();
	// hide the lines
	for (int i = 0; i < mNumLinesForGraph; i++) {
		redrawLine(i);
	}
	// hide axis
	for (int i = 0; i < 2; i++) {
		redrawLine(i, true);
	}
	return true;
}

// hide the widget
bool LineGraphWidget::hide() {
	if (!mpOverlay) {
		printf("ERROR: No LineGraphWidget in memory to hide\n");
		return false;
	}
	mHiding = true;
	mpOverlay->hide(); // hide the overlay
	// hide the lines
	for (int i = 0; i < mNumLinesForGraph; i++) {
		mGraphLine[i].mpLineObject->clear();
	}
	// hide axis
	for (int i = 0; i < 2; i++) {
		mAxisLine[i].mpLineObject->clear();
	}	
	
	return true;
}

// remove all the lines from the graph and free memory
bool LineGraphWidget::freeLines(SceneManager* sceneManager) {
	printf("Freeing Graph Lines...");
	if (!sceneManager) {
		printf("ERROR: no scene manager in LineGraphWidget::freeLines()\n");
		return false;
	}
	for (int i = 0; i < mNumLinesForGraph; i++) {
		if (!removeAllPointsFromLine(i)) { // remove all the data points and clean memory
			printf("ERROR:could not delete line %i\n", i);
			return false;
		}
		mGraphLine[i].mMaximumDataPoints = MAXGRAPHDATAPOINTS; // reset data allowance
		mGraphLine[i].mCurrentDataPoints = 0; // reset point count
		strcpy(mGraphLine[i].mLineName, ""); // reset name
		char tmp[256]; // handle holder
		sprintf(tmp, "%sLineMaterial%i", mUniqueName, i); // get material name
		MaterialManager::getSingleton().remove(tmp); // remove material
		if (!mGraphLine[i].mpLineNode) {
			printf("ERROR: line node %i not found to destroy\n", i);
		}
		mGraphLine[i].mpLineNode = NULL; // reset pointer to node but node isn't removed (problem?)
		sceneManager->destroyManualObject(mGraphLine[i].mpLineObject); // remove object
	}
	mNumLinesForGraph = 0;

	printf("[OK]\n");
	return true;
}

// set the title text for the graph
bool LineGraphWidget::setGraphTitle(const char* title) {
	if (!mGraphTitleLabel.setText(title)) {
		printf("ERROR: could not set title on labelwidget\n");
		return false;
	}
	if (!mGraphTitleLabel.centreText()) {
		printf("ERROR: could not centre text labelwidget\n");
		return false;
	}
	return true;
}

// set the title text for the x axis
bool LineGraphWidget::setXAxisTitle(const char* title) {
	if (!mXAxisLabel.setText(title)) {
		printf("ERROR: could not set title on labelwidget\n");
		return false;
	}
	if (!mXAxisLabel.centreText()) {
		printf("ERROR: could not centre text labelwidget\n");
		return false;
	}

	return true;
}

// set scale of axis
void LineGraphWidget::setXAxisMax(float xAxisMax) {
	mXAxisMax = xAxisMax; // update actual value used for extent
	char tmp[256];
	sprintf(tmp, "%.2f", mXAxisMax);
	mXAxisMaxLabel.setText(tmp); // change value	displayed on axis label
}

// set scale of axis
void LineGraphWidget::setXAxisMin(float xAxisMin) {
	mXAxisMin = xAxisMin;
	char tmp[256];
	sprintf(tmp, "%.2f", mXAxisMin);
	mXAxisMinLabel.setText(tmp); // change value	displayed on axis label
}

// set graph to scroll
void LineGraphWidget::setXAxisScroll(bool scroll) {
	mXAxisScrolling = scroll;
}

void LineGraphWidget::setXAxisInterval(float interval) {
	mXInterval = interval;
}

// set scale of axis
void LineGraphWidget::setYAxisMax(float yAxisMax) {
	mYAxisMax = yAxisMax;
	char tmp[256];
	sprintf(tmp, "%.2f", mYAxisMax);
	mYAxisMaxLabel.setText(tmp); // change value	displayed on axis label
}

// set scale of axis
void LineGraphWidget::setYAxisMin(float yAxisMin) {
	mYAxisMin = yAxisMin;
	char tmp[256];
	sprintf(tmp, "%.2f", mYAxisMin);
	mYAxisMinLabel.setText(tmp); // change value	displayed on axis label
}

// add a new line to the graph with this colour
int LineGraphWidget::createLine(SceneManager* sceneManager, float r, float g, float b) {
	printf("Creating Line for LineGraph\n");
	if (mNumLinesForGraph >= GRAPHLINESMAX) { // validate params
		printf("ERROR: maximum graph lines exceeded, can not create new line.\n");
		return false;
	}
	if (!sceneManager) { // validate params
		printf("ERROR: no scene manager in LineGraphWidget::createLine()\n");
		return false;
	}
	
	int ourLine = mNumLinesForGraph; // get line number
	mNumLinesForGraph++; // incr number of lines used
	
	// start actual line construction	
	char tmp[256];
	sprintf(tmp, "%sLineObject%i", mUniqueName, ourLine); // name line
	mGraphLine[ourLine].mpLineObject = sceneManager->createManualObject(tmp); // create line's manual object
		
	// Use identity view/projection matrices (presumably this means draw to screen coordinates rather than world coords)
	mGraphLine[ourLine].mpLineObject->setUseIdentityProjection(true);
	mGraphLine[ourLine].mpLineObject->setUseIdentityView(true);
		
	sprintf(tmp, "%sLineNode%i", mUniqueName, ourLine);
	mGraphLine[ourLine].mpLineNode = sceneManager->getRootSceneNode()->createChildSceneNode(tmp);
	sprintf(tmp, "%sLineMaterial%i", mUniqueName, ourLine);
	mGraphLine[ourLine].mpLineMaterial = MaterialManager::getSingleton().create(tmp, "Two"); // not sure if 'debugger' needs to be unique
	mGraphLine[ourLine].mpLineMaterial->setReceiveShadows(false);
	mGraphLine[ourLine].mpLineMaterial->getTechnique(0)->setLightingEnabled(true);
	
	// create colour for line (white only at this stage)
	mGraphLine[ourLine].mpLineMaterial->getTechnique(0)->getPass(0)->setDiffuse(r, g, b, 0);
	mGraphLine[ourLine].mpLineMaterial->getTechnique(0)->getPass(0)->setAmbient(r, g, b);
	mGraphLine[ourLine].mpLineMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(r, g, b);
	
	mGraphLine[ourLine].mpLineNode->attachObject(mGraphLine[ourLine].mpLineObject);
	mGraphLine[ourLine].mpLineObject ->setRenderQueueGroup(RENDER_QUEUE_OVERLAY + 1); // render just after overlays
	
	mGraphLine[ourLine].mpLatestDataPoint = NULL; // set points list to none
	mGraphLine[ourLine].mpEarliestDataPoint = NULL;
	strcpy(mGraphLine[ourLine].mLineName, "unnamed data set");
	mGraphLine[ourLine].mCurrentDataPoints = 0;
	mGraphLine[ourLine].mMaximumDataPoints = MAXGRAPHDATAPOINTS; // 100 thousand line max points?
	
	printf("Line created\n");
	return ourLine; // return index of this line
}

// add a new data point to a line
bool LineGraphWidget::addPointToLine(int lineIndex, float realXValue, float realYValue, bool respectXIntervals, bool optimisePoints) {
	if ((lineIndex < 0) || (lineIndex >= mNumLinesForGraph)) { // validate
		printf("ERROR: That line is not in the graph\n");
		return false;
	}
	
	// make sure line hasn't got more than maximum allowed data points allocated on it
	if (mGraphLine[lineIndex].mCurrentDataPoints >= mGraphLine[lineIndex].mMaximumDataPoints) {
		printf("WARNING: too many points (%i) added to line#%i. LineGraphWidget::addPointToLine()\n", mGraphLine[lineIndex].mCurrentDataPoints, lineIndex);
		return false;
	}
	LineDataPoint* currentPoint = mGraphLine[lineIndex].mpLatestDataPoint; // get end of list
	LineDataPoint* previousPoint = NULL;
	
	// only collect data at set x point (or near to)
	if (respectXIntervals) {
		if (mXInterval == 0.0f) {
			printf("ERROR: LineGraphWidget::addPointToLine with respect intervals on; will not work as you have not setXinterval()\n");
			return false;
		}
		unsigned long  upcomingInterval = 0;
		if (currentPoint != NULL) {
			unsigned long adjustedValue = (unsigned long)(realXValue  * 1000000.0f);
			unsigned long adjustedInterval = (unsigned long)(mXInterval  * 1000000.0f);
			unsigned long adjustedCurrentX = (unsigned long)(currentPoint->x * 1000000.0f);
			upcomingInterval = adjustedCurrentX + adjustedInterval;
			if (adjustedValue < upcomingInterval) {
			return true; // don't need this point, ignore
			}
			while (adjustedValue > upcomingInterval + adjustedInterval) { // make sure we didn't skip an interval
				upcomingInterval = upcomingInterval + adjustedInterval;
			}
		}
		realXValue = (float)(upcomingInterval) / 1000000.0f; // stick the measurement to the data point
		// NOTE - can collect x-axis error at this point as well if desired
	}
	
	previousPoint = currentPoint; // keep track of end of list
	currentPoint = new LineDataPoint; // make new list item
	if (!currentPoint) { // validate new memory
		printf("ERROR: mem overflow creating new data plot point\n");
		return false; // failure
	}
	currentPoint->x = realXValue; // set data
	currentPoint->y = realYValue; // set data
	currentPoint->prev = previousPoint; // point to previous end of list
	currentPoint->next = NULL;
	if (previousPoint != NULL) {
		previousPoint->next = currentPoint;
	} else {
		mGraphLine[lineIndex].mpEarliestDataPoint = currentPoint;
	}
	mGraphLine[lineIndex].mpLatestDataPoint = currentPoint; // tell line about this new end point
	mGraphLine[lineIndex].mCurrentDataPoints++;
	
	// check if data y values are 3-in-row or more and can wipe intermediate values for space (good for scrolling display, not for graphing results with error bars)
	if (optimisePoints) {
		if (previousPoint != NULL) {
			if (previousPoint->prev != NULL) {
				float firstPoint = previousPoint->prev->y; 
				int truncated = (int)(firstPoint * 100.0f); // reduce accuracy of instruments to 2 d.p
				firstPoint = (float)truncated / 100.0f; // reduce accuracy of instruments to 2 d.p
				float secondPoint = previousPoint->y; // reduce accuracy of instruments to 2 d.p
				truncated = (int)(secondPoint * 100.0f); // reduce accuracy of instruments to 2 d.p
				secondPoint = (float)truncated / 100.0f; // reduce accuracy of instruments to 2 d.p
				float thirdPoint = realYValue; // reduce accuracy of instruments to 2 d.p
				truncated = (int)(thirdPoint * 100.0f); // reduce accuracy of instruments to 2 d.p
				thirdPoint = (float)truncated / 100.0f; // reduce accuracy of instruments to 2 d.p
				if ((firstPoint == secondPoint) && (firstPoint == thirdPoint)) {
					currentPoint->prev = previousPoint->prev; // cut out 'second' point
					delete previousPoint;
					previousPoint = currentPoint->prev; // change this as well just in case we want it later
					if (previousPoint != NULL) {
						previousPoint->next = currentPoint;
					}
					mGraphLine[lineIndex].mCurrentDataPoints--;
				}
			}
		}
	}
	
	// check if need to adjust graph scale (if this point is outside current graph bounds)
	bool rescaled = false;
	if (realXValue < mXAxisMin) {
		if (!mXAxisPegged) { // expand graph
			setXAxisMin(realXValue);
			rescaled = true;
		}
	}
	if (realXValue > mXAxisMax) {
		if (mXAxisScrolling) {
			float axisRange = mXAxisMax - mXAxisMin;
			setXAxisMax(realXValue);
			setXAxisMin(mXAxisMax - axisRange);
			rescaled = true;
		} else if (!mXAxisPegged) {
			setXAxisMax(realXValue); // auto-scale the graph
			rescaled = true;
		}
	}
	if (realYValue < mYAxisMin) {
		if (!mYAxisPegged) {
			setYAxisMin(realYValue);
			rescaled = true;
		}
	}
	if (realYValue > mYAxisMax) {
		if (!mYAxisPegged) {
			setYAxisMax(realYValue);
			rescaled = true;
		}
	}
	if (!mHiding) {
		if (rescaled) {
			for (int i = 0; i < mNumLinesForGraph; i++) { // redraw all lines on graph to new scale
				if (!redrawLine(i)) {
					printf("ERROR: could not redraw line\n");
					return false;
				}
			}
		} else {
			if (!redrawLine(lineIndex)) { // just redraw our line
				printf("ERROR: could not redraw line\n");
				return false;
			}
		}
	}
	
	return true;
}

 // delete a line's points (freeing memory)
bool LineGraphWidget::removeAllPointsFromLine(int lineIndex, bool isAxisLineIndex) {
	if (!isAxisLineIndex) {
		if ((lineIndex < 0) || (lineIndex >= mNumLinesForGraph)) { // validate
			printf("ERROR: That line is not in the graph\n");
			return false;
		}
	} else {
		if ((lineIndex < 0) || (lineIndex >= 2)) { // validate
			printf("ERROR: That line is not in the graph\n");
			return false;
		}
	}
	LineDataPoint* currentPoint = NULL;
	if (!isAxisLineIndex) {
		currentPoint = mGraphLine[lineIndex].mpLatestDataPoint; // get end of list
	} else {
		currentPoint = mAxisLine[lineIndex].mpLatestDataPoint; // get end of list
	}
	if (currentPoint == NULL) {
		return true; // line was already empty so return true
	}
	if (!currentPoint->deleteNextPoints()) { // auto-delete line point by point
		printf("ERROR: could not delete line - malformed line\n");
		return false;
	}
	delete currentPoint; // delete final point
	if (!isAxisLineIndex) {
		mGraphLine[lineIndex].mpLatestDataPoint = NULL; // and set it to NULL
		mGraphLine[lineIndex].mpEarliestDataPoint = NULL;
	} else {
		mAxisLine[lineIndex].mpLatestDataPoint = NULL; // and set it to NULL
		mAxisLine[lineIndex].mpEarliestDataPoint = NULL;
	}

	return true;
}

// save to file
bool LineGraphWidget::savePlotToFile(int lineIndex, bool gnuformat) {
	if ((lineIndex < 0) || (lineIndex >= mNumLinesForGraph)) { // validate
		printf("ERROR: That line is not in the graph\n");
		return false;
	}
	// go through a loop of all points
	LineDataPoint* currentPoint = NULL;
	currentPoint = mGraphLine[lineIndex].mpEarliestDataPoint; // get end of list
	if (currentPoint == NULL) {
		return true; // line was already empty so return true
	}
	char tmp[256];
	sprintf(tmp, "%s_%i-runid %i", mUniqueName, lineIndex, g.mNavRunId);
	if (!mCSVExporter.startCSVFile(tmp, gnuformat)) { // open file
		printf("ERROR: saving csv plot\n");
		return false;
	}
	if (!gnuformat) { // write titles and things
		if (!mCSVExporter.writePlotTitle(mGraphTitleLabel.getText(), gnuformat)) {
			printf("ERROR: saving csv plot\n");
			return false;
		}
		if (!mCSVExporter.writeXTitle(mXAxisLabel.getText(), gnuformat)) {
			printf("ERROR: saving csv plot\n");
			return false;
		}
		if (!mCSVExporter.writeYTitle("", gnuformat)) { // TEMPORARY until actually have a title for y axis
			printf("ERROR: saving csv plot\n");
			return false;
		}
	}
	
	while (currentPoint != NULL) { // loop thru points printing them
		if (!mCSVExporter.add2dPoint(currentPoint->x, currentPoint->y, gnuformat)) { // write point
			printf("ERROR: file corrupted - csv plot could not be saved\n");
			return false;
		}
		currentPoint = currentPoint->next; // next point
	}
	if (!mCSVExporter.closeCSVFile()) { // close and done
		printf("ERROR: file corrupted - csv plot could not be saved\n");
		return false;
	}
	
	return true;
}

// set up lines on axis
bool LineGraphWidget::initAxisLines(SceneManager* sceneManager) {
	if (!sceneManager) { // validate
		printf("ERROR: no scene manager in LineGraphWidget::initLines()\n");
		return false;
	}
	
	// start actual line construction	
	char tmp[256];
	for (int i = 0; i < 2; i++) {
		sprintf(tmp, "%sAxisLineObject%i", mUniqueName, i); // name line
		mAxisLine[i].mpLineObject = sceneManager->createManualObject(tmp); // create line's manual object
		// Use identity view/projection matrices (presumably this means draw to screen coordinates rather than world coords)
		mAxisLine[i].mpLineObject->setUseIdentityProjection(true);
		mAxisLine[i].mpLineObject->setUseIdentityView(true);
		sprintf(tmp, "%sAxisLineNode%i", mUniqueName, i);
		mAxisLine[i].mpLineNode = sceneManager->getRootSceneNode()->createChildSceneNode(tmp); // attach line to scengraph
		sprintf(tmp, "%sAxisLineMaterial%i", mUniqueName, i);
		mAxisLine[i].mpLineMaterial = MaterialManager::getSingleton().create(tmp, "Two"); // not sure if 'debugger' needs to be unique
		mAxisLine[i].mpLineMaterial->setReceiveShadows(false);
		mAxisLine[i].mpLineMaterial->getTechnique(0)->setLightingEnabled(true);
		// create colour for line (white only at this stage)
		mAxisLine[i].mpLineMaterial->getTechnique(0)->getPass(0)->setDiffuse(1.0f, 1.0f, 1.0f, 0); // white lines
		mAxisLine[i].mpLineMaterial->getTechnique(0)->getPass(0)->setAmbient(1.0f, 1.0f, 1.0f);
		mAxisLine[i].mpLineMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1.0f, 1.0f, 1.0f);
		mAxisLine[i].mpLineNode->attachObject(mAxisLine[i].mpLineObject);
		mAxisLine[i].mpLineObject ->setRenderQueueGroup(RENDER_QUEUE_OVERLAY + 1); // render just after overlays
		mAxisLine[i].mpLatestDataPoint = NULL; // set points list to none
		mAxisLine[i].mpEarliestDataPoint = NULL; // set points list to none
		strcpy(mAxisLine[i].mLineName, "axis_line");
		mAxisLine[i].mCurrentDataPoints = 0;
		mAxisLine[i].mMaximumDataPoints = MAXGRAPHDATAPOINTS; // 100 line max points?

		// create points along line
		LineDataPoint* currentPoint = NULL;
		LineDataPoint* previousPoint = NULL;
		float notchX = 0.0f;
		float notchY = 0.0f;
		float notchInterval = 0.10f; // ten notches (10% of graph each)
		float notchHeight = 0.05f; // 5% of graph height high
		float notchWidth = 0.01f;
		for (int j = 0; j < 11; j++) { // create 10 notches along line
			// first create line at bottom of notch
			previousPoint = currentPoint; // remember existing end point if there is one
			currentPoint = new LineDataPoint; // make new list item
			currentPoint->x = notchX; // set data
			currentPoint->y = notchY; // set data
			currentPoint->prev = previousPoint; // point to previous end of list
			currentPoint->next = NULL;
			mAxisLine[i].mpLatestDataPoint = currentPoint; // tell line about this new end point
			if (mAxisLine[i].mpEarliestDataPoint == NULL) {
				mAxisLine[i].mpEarliestDataPoint = currentPoint;
			}
			// second create line at top of notch
			if (i == 0) {
				notchY = notchHeight;
			} else {
				notchX = notchWidth;
			}
			previousPoint = currentPoint; // remember existing end point if there is one
			currentPoint = new LineDataPoint; // make new list item
			currentPoint->x = notchX; // set data
			currentPoint->y = notchY; // set data
			currentPoint->prev = previousPoint; // point to previous end of list
			currentPoint->next = NULL;
			mAxisLine[i].mpLatestDataPoint = currentPoint; // tell line about this new end point
			// third go back to the bottom of the notch
			if (i == 0) {
				notchY = 0.0f;
			} else {
				notchX = 0.0f;
			}
			previousPoint = currentPoint; // remember existing end point if there is one
			currentPoint = new LineDataPoint; // make new list item
			currentPoint->x = notchX; // set data
			currentPoint->y = notchY; // set data
			currentPoint->prev = previousPoint; // point to previous end of list
			currentPoint->next = NULL;
			mAxisLine[i].mpLatestDataPoint = currentPoint; // tell line about this new end point
			// move on to next notch
			if (i == 0) {
				notchX = notchX + notchInterval; // go to next notch along
			} else {
				notchY = notchY + notchInterval;
			}
		}
		if (!redrawLine(i, true)) {
			printf("ERROR: drawing axis line\n");
			return false;
		}
	}
	
	return true;
}

// remove axis lines
bool LineGraphWidget::freeAxisLines(SceneManager* sceneManager) {
	if (!sceneManager) { // validate
		printf("ERROR: no scene manager in LineGraphWidget::freeAxisLines()\n");
		return false;
	}
	for (int i = 0; i < 2; i++) {
		if (!removeAllPointsFromLine(i, true)) { // remove all the data points and clean memory
			printf("ERROR:could not delete line %i\n", i);
			return false;
		}
		mAxisLine[i].mMaximumDataPoints = MAXGRAPHDATAPOINTS; // reset data allowance
		mAxisLine[i].mCurrentDataPoints = 0; // reset point count
		strcpy(mAxisLine[i].mLineName, ""); // reset name
		char tmp[256]; // handle holder
		sprintf(tmp, "%sAxisLineMaterial%i", mUniqueName, i); // get material name
		MaterialManager::getSingleton().remove(tmp); // remove material
		if (!mAxisLine[i].mpLineNode) {
			printf("ERROR: line node %i not found to destroy\n", i);
		}
		mAxisLine[i].mpLineNode = NULL; // reset pointer to node but node isn't removed (problem?)
		sceneManager->destroyManualObject(mAxisLine[i].mpLineObject); // remove object
	}
	return true;
}

// return real value in terms of 0-1 place along the axis
float LineGraphWidget::getXAxisProportion(float realXValue) {
	// assuming here that realXValue is in valid range already
	float axisRange = mXAxisMax - mXAxisMin;
	return (realXValue - mXAxisMin) / axisRange;
}

// return real value in terms of 0-1 place along the axis
float LineGraphWidget::getYAxisProportion(float realYValue) {
	// assuming here that realXValue is in valid range already
	float axisRange = mYAxisMax - mYAxisMin;
	return (realYValue - mYAxisMin) / axisRange;
}

// given a proportion of widget width, return this as identity-matrix adjusted full screen proportion
float LineGraphWidget::getAdjustedScreenXProportion(float graphXProp) {
	float graphLeftSide = mXProp * mParentBoundRight + mGraphingAreaLeftMargin; // adjust these to fit in graph sub-area of widget
	float numerator = graphLeftSide + graphXProp * mGraphingAreaWidth; // get x pixels along screen
	float screenProportion = numerator / mParentBoundRight; // assuming mParentBoundRight is SCREEN right
	float identityAdjustedProportion = screenProportion * 2.0f - 1.0f; // put in the -1.0f-1.0f range
	
	return identityAdjustedProportion;
}

// given a proportion of widget height, return this as identity-matrix adjusted full screen proportion
// note that in this case the reverse of normal proportions is true so 0.0 is the BOTTOM of the graph
float LineGraphWidget::getAdjustedScreenYProportion(float graphYProp) {
	float graphTopSide = mYProp * mParentBoundBottom + mGraphingAreaTopMargin; // adjust these to fit in graph sub-area of widget
	float numerator = graphTopSide + (1.0f - graphYProp) * mGraphingAreaHeight; // get y pixels along screen
	float screenProportion = numerator / mParentBoundBottom; // assuming mParentBoundBottom is SCREEN bottom
	float identityAdjustedProportion = (screenProportion * 2.0f - 1.0f) * -1.0f; // put in the -1.0f-1.0f range
	
	return identityAdjustedProportion;
}

// draw a line on the graph using a set of points
bool LineGraphWidget::redrawLine(int lineIndex, bool isAxisLineIndex) {
	if (!isAxisLineIndex) {
		if ((lineIndex < 0) || (lineIndex >= mNumLinesForGraph)) { // validate
			printf("ERROR: That line is not in the graph\n");
			return false;
		}
	} else {
		if ((lineIndex < 0) || (lineIndex >= 2)) { // validate
			printf("ERROR: That line is not in the graph\n");
			return false;
		}
	}
	LineDataPoint* currentPoint = NULL;
	if (!isAxisLineIndex) {
		currentPoint = mGraphLine[lineIndex].mpLatestDataPoint; // get the first point
	} else {
		currentPoint = mAxisLine[lineIndex].mpLatestDataPoint; // get the first point
	}
	char tmp[256]; // name for material
	if (!isAxisLineIndex) {
		sprintf(tmp, "%sLineMaterial%i", mUniqueName, lineIndex); // use material that we set up earlier
		mGraphLine[lineIndex].mpLineObject->clear();
	} else {
		sprintf(tmp, "%sAxisLineMaterial%i", mUniqueName, lineIndex); // use material that we set up earlier
		mAxisLine[lineIndex].mpLineObject->clear();
	}
	
	// Use infinite AAB to always stay visible
	AxisAlignedBox aabInf;
	aabInf.setInfinite();
	if (!isAxisLineIndex) {
		mGraphLine[lineIndex].mpLineObject->setBoundingBox(aabInf);
		mGraphLine[lineIndex].mpLineObject->begin(tmp, RenderOperation::OT_LINE_STRIP); // signal start of line-draw process
	} else {
		mAxisLine[lineIndex].mpLineObject->setBoundingBox(aabInf);
		mAxisLine[lineIndex].mpLineObject->begin(tmp, RenderOperation::OT_LINE_STRIP); // signal start of line-draw process
	}
	while (currentPoint != NULL) { // draw all points until at end of list
		float xProp = 0.0f;
		float yProp = 0.0f;
		if (!isAxisLineIndex) {
			xProp = getXAxisProportion(currentPoint->x); // get x value as 0-1 of current axis scale
			yProp = getYAxisProportion(currentPoint->y); // get y value as 0-1 of current axis scale
		} else {
			xProp = currentPoint->x; // axis points are already in the form 0-1 of graph width
			yProp = currentPoint->y; // axis points are already in the form 0-1 of graph height
		}
		float screenXPlacement = getAdjustedScreenXProportion(xProp); // get adjusted x pos for point
		float screenYPlacement = getAdjustedScreenYProportion(yProp); // get adjusted y pos for point
		if (!isAxisLineIndex) {
			bool doNotDraw = false; // whether or not to draw the point
			//if (mXAxisPegged) { // if graph is not resizing then check if point is outside bounds
				if ((currentPoint->x > mXAxisMax) || (currentPoint->x < mXAxisMin)) {
					doNotDraw = true;
				}
			//}
			//if (mYAxisPegged) { // if graph is not resizing then check if point is outside bounds
				if ((currentPoint->y > mYAxisMax) || (currentPoint->y < mYAxisMin)) {
					doNotDraw = true;
				}
			//}
			if (!doNotDraw) { // draw point unless it is outside graph bounds (and graph was not resized)
				mGraphLine[lineIndex].mpLineObject->position(screenXPlacement, screenYPlacement, 0.0f); // add draw point
			}
		} else {
			mAxisLine[lineIndex].mpLineObject->position(screenXPlacement, screenYPlacement, 0.0f); // add draw point
		}		
		currentPoint = currentPoint->prev; // get next point
	}
	if (!isAxisLineIndex) {
		mGraphLine[lineIndex].mpLineObject->end(); // signal finish drawing line
	} else {
		mAxisLine[lineIndex].mpLineObject->end(); // signal finish drawing line
	}
	
	return true;
}

