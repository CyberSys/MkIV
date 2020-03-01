#ifndef MARKIV_FILEBROWSER_WIDGET_H_
#define MARKIV_FILEBROWSER_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "EntryWidget.h"
#include "LabelWidget.h"
#include "../FileScanner.h"
#include "../scenery/ScenarioDetails.h"

using namespace Ogre;

class FileBrowserWidget {
public:
	FileBrowserWidget();
	bool init(const char* uniqueName, float parentLeft, float parentRight, float parentTop, float parentBottom, float xProp, float yProp);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool scan(const char* subdir, const char* extension, bool recursive = false);
	bool show();
	bool hide();
	bool moveWidget(float xProp, float yProp);	// move to a position relative to size of parent (0.0-1.0)
	bool update(unsigned int mousex, unsigned int mousey, bool lmb);  // changes display if next/prev clicked
	bool populatePage(int pageNumber);
	char* getSelectedEntryName();
	bool isOKClicked();
	bool isCancelClicked();
	ScenarioDetails* getDetails();
	
	ScenarioDetails* pSelectedScenarioDetails;
	char mSelectedEntry[256];
private:
	Overlay* pOverlay;
	OverlayElement* pBaseElement;
	OverlayContainer* pContainer;
	BorderPanelOverlayElement* pBorderPanel;

	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	float mWidgetWidthPixels; // in pixels
	float mWidgetHeightPixels; // in pixels
	float mXProp; // position relative to parent
	float mYProp; // position relative to parent
	float mXPos; // position in pixels (absolute)
	float mYPos; // position in pixels (absolute)
	
	ButtonWidget mOKButton;
	ButtonWidget mCancelButton;
	ButtonWidget mNextPageButton;
	ButtonWidget mPrevPageButton;
	EntryWidget mCurrentEntry[15];
	LabelWidget mScenarioTitleLabel;
	LabelWidget mTimeLimitLabel;
	LabelWidget mNumberOfForcesLabel;
	LabelWidget mForce1Label;
	LabelWidget mForce2Label;
	int mNumberPages;
	int mCurrentPage;
	int mSelectedEntryIndex;
	int mSelectedEntryOnPage;
	FileScanner* pFileScanner;

	char mPrefix[256];
	char mPostfix[256];
};

#endif
