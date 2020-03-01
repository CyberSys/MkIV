#ifndef MARKIV_MESHBROWSER_WIDGET_H_
#define MARKIV_MESHBROWSER_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "EntryWidget.h"
#include "LabelWidget.h"
#include "../FileScanner.h"

using namespace Ogre;

class MeshBrowserWidget {
public:
	MeshBrowserWidget();
	bool init(const char* uniqueName, float parentLeft, float parentRight, float parentTop, float parentBottom, float xProp, float yProp, SceneManager* sm);
	virtual bool initDetails() = 0;
	bool free();
	virtual bool freeDetails() = 0;
	bool defineParentBounds(float left, float top, float right, float bottom);	// tell this widget what the parent's dimensions are. Needs to be called if parent moved.
	bool scan(const char* subdir, const char* extension, bool recursive = false);
	bool show();
	bool hide();
	bool moveWidget(float xProp, float yProp);	// move to a position relative to size of parent (0.0-1.0)
	bool update(unsigned int mousex, unsigned int mousey, bool lmb);  // changes display if next/prev clicked
	bool populatePage(int pageNumber);
	char* getSelectedEntryName();
	bool isOKClicked();
	float mParentBoundLeft;
	float mParentBoundRight;
	float mParentBoundTop;
	float mParentBoundBottom;
	float mWidgetWidthPixels; // in pixels
	float mWidgetHeightPixels; // in pixels
protected:
	virtual bool displaySelectedDetails() = 0;
	SceneManager* pSceneManager;
	Overlay* pOverlay;
	OverlayElement* pBaseElement;
	OverlayContainer* pContainer;
	BorderPanelOverlayElement* pBorderPanel;
	Entity* pGUImesh;
	char mUniqueName[256];
	float mXProp; // position relative to parent
	float mYProp; // position relative to parent
	float mXPos; // position in pixels (absolute)
	float mYPos; // position in pixels (absolute)
	
	ButtonWidget mOKButton;
	ButtonWidget mNextPageButton;
	ButtonWidget mPrevPageButton;
	EntryWidget mCurrentEntry[15];
	LabelWidget mDesignationLabel;
	int mNumberPages;
	int mCurrentPage;
	int mSelectedEntryIndex;
	int mSelectedEntryOnPage;
	int mEntityCount;
	FileScanner* pFileScanner;
	SceneNode* mpPreviewMeshNode;

	char mPrefix[256];
	char mPostfix[256];
	char mSelectedEntry[256];
	bool mGUImeshAllocated;
};

#endif
