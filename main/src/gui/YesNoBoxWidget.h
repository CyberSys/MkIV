#ifndef MARKIV_YESNOBOX_WIDGET_H_
#define MARKIV_YESNOBOX_WIDGET_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "ButtonWidget.h"
#include "LabelWidget.h"

/* A box that pops up with "Do you really want to blah blah blah Yes or Cancel"
*/

class YesNoBoxWidget {
public:
	YesNoBoxWidget();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xProp, float yProp);
	bool free();
	bool defineParentBounds(float left, float top, float right, float bottom);
	bool show();
	bool hide();
	bool moveWidget(float xProp, float yProp);	// move to a position relative to size of parent (0.0-1.0)
	bool update(unsigned int mousex, unsigned int mousey, bool lmb);  // changes display if next/prev clicked
	bool isOKClicked();
	bool isCancelClicked();
	bool isShowing();
	void setQuestion(const char* questionText);
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
	bool mIsShowing;	

	ButtonWidget mOKButton;
	ButtonWidget mCancelButton;
	LabelWidget mQuestionText;
};

#endif

