#include "MouseCursor.h"

void MouseCursor::init() {
	mMaterial = MaterialManager::getSingleton().create("MouseCursor/default", "General");
	mCursorContainer = (OverlayContainer*) OverlayManager::getSingletonPtr()->createOverlayElement("Panel", "MouseCursor");
	mCursorContainer->setMaterialName(mMaterial->getName());
	mCursorContainer->setPosition(0, 0);
	mGuiOverlay = OverlayManager::getSingletonPtr()->create("MouseCursor");
	mGuiOverlay->setZOrder(649);
	mGuiOverlay->add2D(mCursorContainer);
	mGuiOverlay->show();
}

void MouseCursor::setImage(const String& filename) {
	mTexture = TextureManager::getSingleton().load(filename, "General");
	TextureUnitState *pTexState;
	if(mMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates()) {
		pTexState = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
	}	else {
		pTexState = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState(mTexture->getName());
	}
	pTexState->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
	mMaterial->getTechnique(0)->getPass(0)->setSceneBlending(SBT_TRANSPARENT_ALPHA);
} 

void MouseCursor::setWindowDimensions(unsigned int width, unsigned int height) {
	mWindowWidth = (width > 0) ? width : 1;
	mWindowHeight = (height > 0) ? height : 1;
	mCursorContainer->setWidth(mTexture->getWidth() / mWindowWidth);
	mCursorContainer->setHeight(mTexture->getHeight() / mWindowHeight);
}

void MouseCursor::setVisible(bool visible) {
	if(visible) {
		mCursorContainer->show();
	} else {
		mCursorContainer->hide();
	}
}

void MouseCursor::updatePosition(int x, int y) {
	float rx = x / mWindowWidth;
	float ry = y / mWindowHeight;
	mCursorContainer->setPosition(clamp(rx, 0.0f, 1.0f), clamp(ry, 0.0f, 1.0f));
}

float MouseCursor::clamp(float a, float min, float max) {
	if (a < min) {
		return min;
	}
	if (a > max) {
		return max;
	} 
	return a;
}
