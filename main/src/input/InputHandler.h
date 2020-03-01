/* header for the main "InputHandler" class of MARK IV engine
 * Anton Gerdelan 2007
 * <gerdelan AT gmail DOT com>
 * 
 * InputHandler is an interface for managing input library
 * in this case the back-end is the OIS (Object Oriented Input System)
 */

#ifndef INPUTHANDLER_H_
#define INPUTHANDLER_H_

#include "Ogre.h"
#include "OIS.h"

using namespace Ogre;

class InputHandler {
public:
	InputHandler();
	bool init(Ogre::RenderWindow *window);
	bool free();
	void setDefaultKeyDefs();
	void captureAll(unsigned long &elapsed);
	void toggleTextCapture();
	void startCapturingText();
	void stopCapturingText();
	bool isCapturingText();
	
	// checking for various keys (functions not OIS-dependent and redefinable)
	bool isDebugKeyDown();
	bool isQuitKeyDown();
	bool isPauseKeyDown();
	bool isEnterChatKeyDown();
	bool isTogglePointerKeyDown();
	bool isToggleDebugPanelKeyDown();
	bool isStatsModeKeyDown();
	bool isSavePlotKeyDown();
	
	// unit command keys
	bool isChaseCamKeyDown();

	// edit mode keys
	bool isEditModeNoneKeyDown();
	bool isEditMode1KeyDown();
	bool isEditMode2KeyDown();
	bool isEditMode3KeyDown();
	bool isEditMode4KeyDown();
	bool isEditMode5KeyDown();
	// painting
	bool isPaintModeNoneKeyDown();
	bool isPaintMode1KeyDown();
	bool isPaintMode2KeyDown();
	bool isPaintMode3KeyDown();
	bool isPaintMode4KeyDown();
	bool isPaintMode5KeyDown();
	bool isPaintMode6KeyDown();
	bool isPaintMode7KeyDown();
	// scenery editing
	bool isDeleteKeyDown();
	bool isRotatePosKeyDown();
	bool isRotateNegKeyDown();
	
	// move camera keys
	bool isForwardKeyDown();
	bool isBackwardKeyDown();
	bool isSlideLeftKeyDown();
	bool isSlideRightKeyDown();
	bool isFlyUpKeyDown();
	bool isFlyDownKeyDown();
	
	// rotate camera keys
	bool isTurnUpKeyDown();
	bool isTurnDownKeyDown();
	bool isTurnLeftKeyDown();
	bool isTurnRightKeyDown();
	
	// TODO Change to 'select unit key' etc.
	bool isLMBDown();	// TODO: change these to 'clicked'
	bool isRMBDown();
	bool isMMBDown();

	bool isLMBHeldDown();
	bool isRMBHeldDown();
	
	char* getCapturedText();
	int getMouseX();
	int getMouseY();
	int getMouseRelX();
	int getMouseRelY();
private:
	void captureText();
	Ogre::RenderWindow* mWindow;	// ptr to main game's render window
	OIS::InputManager* mInputManager;
	OIS::Mouse* mMouse;
	OIS::Keyboard* mKeyboard;
	
	bool mTogglePointerKeyLock;
	bool mToggleDebugKeyLock;
	bool mToggleQuitKeyLock;
	bool mTogglePauseKeyLock;
	bool mToggleEnterChatKeyLock;
	bool mToggleDebugPanelKeyLock;
	bool mStatsModeKeyLock;
	bool mSavePlotKeyLock;
	bool mLMBLock;
	bool mRMBLock;
	bool mChaseCamKeyLock;
	bool mEditModeNoneLock;
	bool mEditMode1Lock;
	bool mEditMode2Lock;
	bool mEditMode3Lock;
	bool mEditMode4Lock;
	bool mEditMode5Lock;
	bool mPaintModeNoneLock;
	bool mPaintMode1Lock;
	bool mPaintMode2Lock;
	bool mPaintMode3Lock;
	bool mPaintMode4Lock;
	bool mPaintMode5Lock;
	bool mPaintMode6Lock;
	bool mPaintMode7Lock;
	bool mDeleteKeyLock;
	
	bool mIsCapturingText;		// are we capturing user-entered text
	char mCapturedText[256];
	Timer mChatTimer;	// timer to limit chat key presses
	unsigned long mChatTimerElapsed;
	char mLastChatKey; // last key typed by user
	
	int mMouseX;
	int mMouseY;
	int mMouseRelX;
	int mMouseRelY;
	
	// key definitions (flexibile)
	enum OIS::KeyCode mDebugKey;
	enum OIS::KeyCode	mQuitKey;
	enum OIS::KeyCode	mPauseKey;
	enum OIS::KeyCode	mEnterChatKey;
	enum OIS::KeyCode	mTogglePointerKey;
	enum OIS::KeyCode	mToggleDebugPanelKey;
	enum OIS::KeyCode	mStatsModeKey;
	enum OIS::KeyCode	mSavePlotKey;
	
	enum OIS::KeyCode mChaseCamKey;

	enum OIS::KeyCode	mEditModeNoneKey;
	enum OIS::KeyCode	mEditMode1Key;
	enum OIS::KeyCode	mEditMode2Key;
	enum OIS::KeyCode	mEditMode3Key;
	enum OIS::KeyCode	mEditMode4Key;
	enum OIS::KeyCode	mEditMode5Key;
	enum OIS::KeyCode	mPaintModeNoneKey;
	enum OIS::KeyCode	mPaintMode1Key;
	enum OIS::KeyCode	mPaintMode2Key;
	enum OIS::KeyCode	mPaintMode3Key;
	enum OIS::KeyCode	mPaintMode4Key;
	enum OIS::KeyCode	mPaintMode5Key;
	enum OIS::KeyCode	mPaintMode6Key;
	enum OIS::KeyCode mPaintMode7Key;

	enum OIS::KeyCode	mDeleteKey;
	enum OIS::KeyCode	mRotatePosKey;
	enum OIS::KeyCode	mRotateNegKey;
	
	enum OIS::KeyCode	mForwardKey;
	enum OIS::KeyCode	mForwardKey2;
	enum OIS::KeyCode	mBackwardKey;
	enum OIS::KeyCode	mBackwardKey2;
	enum OIS::KeyCode	mSlideLeftKey;
	enum OIS::KeyCode	mSlideRightKey;
	enum OIS::KeyCode	mFlyUpKey;
	enum OIS::KeyCode	mFlyUpKey2;
	enum OIS::KeyCode	mFlyDownKey;
	enum OIS::KeyCode	mFlyDownKey2;
	
	enum OIS::KeyCode	mTurnUpKey;
	enum OIS::KeyCode	mTurnUpKey2;
	enum OIS::KeyCode	mTurnDownKey;
	enum OIS::KeyCode	mTurnDownKey2;
	enum OIS::KeyCode	mTurnLeftKey;
	enum OIS::KeyCode	mTurnRightKey;
};

#endif /*INPUTHANDLER_H_*/

