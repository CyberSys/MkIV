#ifndef MARKIV_ENTRY_WIDGET_H_
#define MARKIV_ENTRY_WIDGET_H_

#include "ButtonWidget.h"

using namespace Ogre;

// A small button widget used as a clickable list item for combo-box type widgets
class EntryWidget : public ButtonWidget {
public:
	EntryWidget();
	bool init(const char* uniqueName, float parentLeft, float parentTop, float parentRight, float parentBottom, float xprop, float yprop, const char* text, bool toggleType = true);
	bool setText(const char *text);
private:
	
}; 

#endif
