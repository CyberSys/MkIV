//#include <OgrePanelOverlayElement.h>

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

using namespace Ogre;

class MouseCursor {
public:
	void init();
	void setImage(const String& filename);
	void setWindowDimensions(unsigned int width, unsigned int height);
	void setVisible(bool visible);
	void updatePosition(int x, int y);
	float clamp(float a, float min, float max);
private:
	Overlay* mGuiOverlay;
	OverlayContainer* mCursorContainer;
	TexturePtr mTexture;
	MaterialPtr mMaterial;
	Real mWindowWidth;
	Real mWindowHeight;
}; 
