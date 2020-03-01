#include "InputHandler.h"
#include <string.h>

// default constr - default vals for vars
InputHandler::InputHandler() {
	mTogglePointerKeyLock = false;
	mToggleDebugKeyLock = false;
	mToggleQuitKeyLock = false;
	mTogglePauseKeyLock = false;
	mToggleEnterChatKeyLock = false;
	mToggleDebugPanelKeyLock = false;
	mStatsModeKeyLock = false;
	mSavePlotKeyLock = false;
	mLMBLock = false;
	mRMBLock = false;
	mChaseCamKeyLock = false;
	mEditModeNoneLock = false;
	mEditMode1Lock = false;
	mEditMode2Lock = false;
	mEditMode3Lock = false;
	mEditMode4Lock = false;
	mEditMode5Lock = false;
	mPaintModeNoneLock = false;
	mPaintMode1Lock = false;
	mPaintMode2Lock = false;
	mPaintMode3Lock = false;
	mPaintMode4Lock = false;
	mPaintMode5Lock = false;
	mPaintMode6Lock = false;
	mPaintMode7Lock = false;
	mDeleteKeyLock = false;

	mIsCapturingText = false;

	strcpy(mCapturedText,"");
	mChatTimerElapsed = 999;
	mLastChatKey = -1;
	
	mMouseX = 0;
	mMouseY = 0;
	mMouseRelX = 0;
	mMouseRelY = 0;

	// assign keys to various functional buttons
	setDefaultKeyDefs();
}

// initialise input handler
bool InputHandler::init(Ogre::RenderWindow *window) {
	if (window == NULL) { // validate that we have a window to render to
		printf("ERROR: window passed to input handler is NULL\n");
		return false;
	}
	
	mWindow = window; // should really be mpWindow or not at all
	
	// create a ParamList object. Used to pass parameters to classes.
	OIS::ParamList pl;
	// Create a variable used to retrieve window handle from OGRE render window
	size_t windowHnd = 0;
	// create a variable to turn size_t into a string.
	std::ostringstream windowHndStr;
	// populate size_t variable with window handle
	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	// turn handle into string
	windowHndStr << windowHnd;
	// insert string and WINDOW into plist
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

	// CREATE OIS SYSTEM:
	mInputManager = OIS::InputManager::createInputSystem(pl);
	
	/* Note: this is absolutely horrific coding style - thanks to OIS */
	// the 2nd param to createInputObject is buffered true/(false)=unbuffered.
	mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, false));
	mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, false));
	
	// confine mouse movement to inside render window
	unsigned int width, height, depth;
	int top, left;
	mWindow->getMetrics(width,height,depth,left,top);
	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
	
	return true;	
}

// shutdown OIS and clean up keys and turn keyboard repeat back on
bool InputHandler::free() {
	if (!mInputManager) {
		printf("ERROR: Non mem alloced to the mInputManager to free\n");
		return false;
	}
	// free input managers
	mInputManager->destroyInputObject(mMouse);
	mInputManager->destroyInputObject(mKeyboard);
	mInputManager->destroyInputSystem(mInputManager); // terrible code

	return true;
}

// reset all the keys to system defaults
void InputHandler::setDefaultKeyDefs() {
	mDebugKey = OIS::KC_F12;
	mQuitKey					= OIS::KC_ESCAPE;
	mPauseKey					= OIS::KC_PAUSE;
	mEnterChatKey			= OIS::KC_RETURN;
	mTogglePointerKey = OIS::KC_SPACE;
	mToggleDebugPanelKey = OIS::KC_F1;
	mStatsModeKey			= OIS::KC_F2;
	mSavePlotKey			= OIS::KC_F3;
	
	mChaseCamKey = OIS::KC_C;
	
	mEditModeNoneKey	= OIS::KC_F4;	// not sure if this is the '~' key
	mEditMode1Key			= OIS::KC_F5;
	mEditMode2Key			= OIS::KC_F6;
	mEditMode3Key			= OIS::KC_F7;
	mEditMode4Key			= OIS::KC_F8;
	mEditMode5Key			= OIS::KC_F9;

	mPaintModeNoneKey	= OIS::KC_GRAVE;	// not sure if this is the '~' key
	mPaintMode1Key		= OIS::KC_1;
	mPaintMode2Key		= OIS::KC_2;
	mPaintMode3Key		= OIS::KC_3;
	mPaintMode4Key		= OIS::KC_4;
	mPaintMode5Key		= OIS::KC_5;
	mPaintMode6Key		= OIS::KC_6;
	mPaintMode7Key		= OIS::KC_7;

	mDeleteKey				= OIS::KC_DELETE;
	mRotatePosKey			= OIS::KC_COMMA;
	mRotateNegKey			= OIS::KC_PERIOD;
	
	mForwardKey 			= OIS::KC_W;
	mForwardKey2 			= OIS::KC_W;
	mBackwardKey			= OIS::KC_S;
	mBackwardKey2			= OIS::KC_S;
	mSlideLeftKey 		= OIS::KC_A;
	mSlideRightKey 		= OIS::KC_D;
	mFlyUpKey					= OIS::KC_PGUP;
	mFlyDownKey				= OIS::KC_PGDOWN;
	mFlyUpKey2				= OIS::KC_Q;
	mFlyDownKey2			= OIS::KC_E;
	
	mTurnUpKey				= OIS::KC_HOME;
	mTurnUpKey2				= OIS::KC_UP;
	mTurnDownKey			= OIS::KC_END;
	mTurnDownKey2			= OIS::KC_DOWN;
	mTurnLeftKey 			= OIS::KC_LEFT;
	mTurnRightKey 		= OIS::KC_RIGHT;
}

// toggle text capture on/off
void InputHandler::toggleTextCapture() {
	mIsCapturingText = !mIsCapturingText;
	if (!mIsCapturingText) {
		strcpy(mCapturedText,"");
	}
}

void InputHandler::startCapturingText() {
	mIsCapturingText = true;
}

void InputHandler::stopCapturingText() {
	mIsCapturingText = false;
	strcpy(mCapturedText,"");
}

// capture input (update input)
void InputHandler::captureAll(unsigned long &elapsed) {
	mKeyboard->capture();
	mMouse->capture();	
	
	// check if user is entering text into a text-box
	if (mIsCapturingText) {
		captureText();
	}
	
	// reset locks if applicable
	if (mLMBLock) {
		if (!mMouse->getMouseState().buttonDown(OIS::MB_Left)) {
			mLMBLock = false;	// unlock button because mouse button has been released
		}
	}
	
	if (mRMBLock) {
		if (!mMouse->getMouseState().buttonDown(OIS::MB_Right)) {
			mRMBLock = false;	// unlock button because mouse button has been released
		}
	}
	
	if (mTogglePointerKeyLock) {
		if (!mKeyboard->isKeyDown(mTogglePointerKey)) {
			mTogglePointerKeyLock = false;
		}
	}
	
	if (mToggleDebugPanelKeyLock) {
		if (!mKeyboard->isKeyDown(mToggleDebugPanelKey)) {
			mToggleDebugPanelKeyLock = false;
		}
	}

	if (mStatsModeKeyLock) { // unlock stats key if key is not being pressed any more
		if (!mKeyboard->isKeyDown(mStatsModeKey)) {
			mStatsModeKeyLock = false;
		}
	}
	
	if (mSavePlotKeyLock) { // unlock stats key if key is not being pressed any more
		if (!mKeyboard->isKeyDown(mSavePlotKey)) {
			mSavePlotKeyLock = false;
		}
	}
	
	if (mDeleteKeyLock) {
		if (!mKeyboard->isKeyDown(mDeleteKey)) {
			mDeleteKeyLock = false;
		}
	}
	
	if (mChaseCamKeyLock) {
		if (!mKeyboard->isKeyDown(mChaseCamKey)) {
			mChaseCamKeyLock = false;
		}
	}


	if (mEditModeNoneLock) {
		if (!mKeyboard->isKeyDown(mEditModeNoneKey)) {
			mEditModeNoneLock = false;
		}
	}

	if (mEditMode1Lock) {
		if (!mKeyboard->isKeyDown(mEditMode1Key)) {
			mEditMode1Lock = false;
		}
	}
	
	if (mEditMode2Lock) {
		if (!mKeyboard->isKeyDown(mEditMode2Key)) {
			mEditMode2Lock = false;
		}
	}

	if (mEditMode3Lock) {
		if (!mKeyboard->isKeyDown(mEditMode3Key)) {
			mEditMode3Lock = false;
		}
	}

	if (mEditMode4Lock) {
		if (!mKeyboard->isKeyDown(mEditMode4Key)) {
			mEditMode4Lock = false;
		}
	}

	if (mEditMode5Lock) {
		if (!mKeyboard->isKeyDown(mEditMode5Key)) {
			mEditMode5Lock = false;
		}
	}

	if (mPaintModeNoneLock) {
		if (!mKeyboard->isKeyDown(mPaintModeNoneKey)) {
			mPaintModeNoneLock = false;
		}
	}

	if (mPaintMode1Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode1Key)) {
			mPaintMode1Lock = false;
		}
	}
	
	if (mPaintMode2Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode2Key)) {
			mPaintMode2Lock = false;
		}
	}

	if (mPaintMode3Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode3Key)) {
			mPaintMode3Lock = false;
		}
	}

	if (mPaintMode4Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode4Key)) {
			mPaintMode4Lock = false;
		}
	}

	if (mPaintMode5Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode5Key)) {
			mPaintMode5Lock = false;
		}
	}

	if (mPaintMode6Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode6Key)) {
			mPaintMode6Lock = false;
		}
	}

	if (mPaintMode7Lock) {
		if (!mKeyboard->isKeyDown(mPaintMode7Key)) {
			mPaintMode7Lock = false;
		}
	}
	if (mToggleDebugKeyLock) {
		if (!mKeyboard->isKeyDown(mDebugKey)) {
			mToggleDebugKeyLock = false;
		}
	}
	if (mToggleQuitKeyLock) {
		if (!mKeyboard->isKeyDown(mQuitKey)) {
			mToggleQuitKeyLock = false;
		}
	}

	if (mTogglePauseKeyLock) {
		if (!mKeyboard->isKeyDown(mPauseKey)) {
			mTogglePauseKeyLock = false;
		}
	}
	
	if (mToggleEnterChatKeyLock) {
		if (!mKeyboard->isKeyDown(mEnterChatKey)) {
			mToggleEnterChatKeyLock = false;
		}
	}
	
	// record mouse position
	OIS::MouseState ms = mMouse->getMouseState();
	
	mMouseX = ms.X.abs;
	mMouseY = ms.Y.abs;
	
	mMouseRelX = ms.X.rel;
	mMouseRelY = ms.Y.rel;
}

// look for alpha numeric chars
void InputHandler::captureText() {
	// this is a massively simplified wrapper of OIS
	
	// 1. check all the keys to see if pressed
	// 2. stop at first one found
	// 3. start a timer to block input for a set amount of time
	// 4. append any keys found to end of string
	
	// update timer
	mChatTimerElapsed = mChatTimer.getMilliseconds();

	int length = strlen(mCapturedText);

	// check if string too long
	if (length > 252) {
		return;
	}
	
	char charTyped = -1;	// character typed by user
	bool shiftHeld = false;	// if shift is held by user (for capitals)	

	// check for text-altering keys
	if ((mKeyboard->isKeyDown(OIS::KC_LSHIFT)) || (mKeyboard->isKeyDown(OIS::KC_RSHIFT))) {
		shiftHeld = true;
	}

	// backspace
	if (mKeyboard->isKeyDown(OIS::KC_BACK)) {
		if (mLastChatKey == -2) {
			// check a timer to see if we can continue text capture
			if (mChatTimerElapsed < 175) {
				return;
			}
		}
		mLastChatKey = -2;	// set last chat key to -2 (backspace)
		
		if (length > 0) {
			// remove trailing character
			mCapturedText[length-1] = '\0';
			mChatTimer.reset();	// start timer
		}
	}
	
	// check if numbers pressed
	if ((mKeyboard->isKeyDown(OIS::KC_1)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD1))) {
		if (shiftHeld) {
			charTyped = '!';	// ascii 33
		} else {
			charTyped = '1';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_2)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD2))) {
		if (shiftHeld) {
			charTyped = '@';
		} else {
			charTyped = '2';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_3)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD3))) {
		if (shiftHeld) {
			charTyped = '#';
		} else {
			charTyped = '3';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_4)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD4))) {
		if (shiftHeld) {
			charTyped = '$';
		} else {
			charTyped = '4';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_5)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD5))) {
		if (shiftHeld) {
			charTyped = '%';
		} else {
			charTyped = '5';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_6)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD6))) {
		if (shiftHeld) {
			charTyped = '^';
		} else {
			charTyped = '6';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_7)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD7))) {
		if (shiftHeld) {
			charTyped = '&';
		} else {
			charTyped = '7';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_8)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD8))) {
		if (shiftHeld) {
			charTyped = '*';
		} else {
			charTyped = '8';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_9)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD9))) {
		if (shiftHeld) {
			charTyped = '(';
		} else {
			charTyped = '9';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_0)) || (mKeyboard->isKeyDown(OIS::KC_NUMPAD0))) {
		if (shiftHeld) {
			charTyped = ')';
		} else {
			charTyped = '0';
		}

	// check for punctuation
	} else if (mKeyboard->isKeyDown(OIS::KC_MINUS)) {
		if (shiftHeld) {
			charTyped = '_';
		} else {
			charTyped = '-';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_EQUALS)) {
		if (shiftHeld) {
			charTyped = '+';
		} else {
			charTyped = '=';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_TAB)) {
		charTyped = '\t';
	} else if (mKeyboard->isKeyDown(OIS::KC_LBRACKET)) {
		if (shiftHeld) {
			charTyped = '{';
		} else {
			charTyped = '[';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_RBRACKET)) {
		if (shiftHeld) {
			charTyped = '}';
		} else {
			charTyped = ']';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_SEMICOLON)) {
		if (shiftHeld) {
			charTyped = ':';
		} else {
			charTyped = ';';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_APOSTROPHE)) {
		if (shiftHeld) {
			charTyped = '"';
		} else {
			charTyped = '\'';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_GRAVE)) {
		if (shiftHeld) {
			charTyped = '~';
		} else {
			charTyped = '`';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_COMMA)) {
		if (shiftHeld) {
			charTyped = '<';
		} else {
			charTyped = ',';
		}
	} else if ((mKeyboard->isKeyDown(OIS::KC_PERIOD)) || (mKeyboard->isKeyDown(OIS::KC_SUBTRACT))) {
		if (shiftHeld) {
			charTyped = '>';
		} else {
			charTyped = '.';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_SLASH)) {
		if (shiftHeld) {
			charTyped = '?';
		} else {
			charTyped = '/';
		}
	} else if (mKeyboard->isKeyDown(OIS::KC_MULTIPLY)) {
		charTyped = '*';
	} else if (mKeyboard->isKeyDown(OIS::KC_SPACE)) {
		charTyped = ' ';
	}

	// check for alphabet characters
	else if (mKeyboard->isKeyDown(OIS::KC_A)) {
		charTyped = 'a';
	} else if (mKeyboard->isKeyDown(OIS::KC_B)) {
		charTyped = 'b';
	} else if (mKeyboard->isKeyDown(OIS::KC_C)) {
		charTyped = 'c';
	} else if (mKeyboard->isKeyDown(OIS::KC_D)) {
		charTyped = 'd';
	} else if (mKeyboard->isKeyDown(OIS::KC_E)) {
		charTyped = 'e';
	} else if (mKeyboard->isKeyDown(OIS::KC_F)) {
		charTyped = 'f';
	} else if (mKeyboard->isKeyDown(OIS::KC_G)) {
		charTyped = 'g';
	} else if (mKeyboard->isKeyDown(OIS::KC_H)) {
		charTyped = 'h';
	} else if (mKeyboard->isKeyDown(OIS::KC_I)) {
		charTyped = 'i';
	} else if (mKeyboard->isKeyDown(OIS::KC_J)) {
		charTyped = 'j';
	} else if (mKeyboard->isKeyDown(OIS::KC_K)) {
		charTyped = 'k';
	} else if (mKeyboard->isKeyDown(OIS::KC_L)) {
		charTyped = 'l';
	} else if (mKeyboard->isKeyDown(OIS::KC_M)) {
		charTyped = 'm';
	} else if (mKeyboard->isKeyDown(OIS::KC_N)) {
		charTyped = 'n';
	} else if (mKeyboard->isKeyDown(OIS::KC_O)) {
		charTyped = 'o';
	} else if (mKeyboard->isKeyDown(OIS::KC_P)) {
		charTyped = 'p';
	} else if (mKeyboard->isKeyDown(OIS::KC_Q)) {
		charTyped = 'q';
	} else if (mKeyboard->isKeyDown(OIS::KC_R)) {
		charTyped = 'r';
	} else if (mKeyboard->isKeyDown(OIS::KC_S)) {
		charTyped = 's';
	} else if (mKeyboard->isKeyDown(OIS::KC_T)) {
		charTyped = 't';
	} else if (mKeyboard->isKeyDown(OIS::KC_U)) {
		charTyped = 'u';
	} else if (mKeyboard->isKeyDown(OIS::KC_V)) {
		charTyped = 'v';
	} else if (mKeyboard->isKeyDown(OIS::KC_W)) {
		charTyped = 'w';
	} else if (mKeyboard->isKeyDown(OIS::KC_X)) {
		charTyped = 'x';
	} else if (mKeyboard->isKeyDown(OIS::KC_Y)) {
		charTyped = 'y';
	} else if (mKeyboard->isKeyDown(OIS::KC_Z)) {
		charTyped = 'z';
	}

	// change to capitals if shift held
	if ((charTyped >= 'a') && (charTyped <= 'z')) {
		if (shiftHeld) {
			charTyped -= 32;
		}
	}
	
	// append captured string with the new character
	if (charTyped != -1) {
		if (charTyped == mLastChatKey) {
			// check a timer to see if we can continue text capture
			if (mChatTimerElapsed < 175) {
				return;
			}
		}
		char tmp[4];
		sprintf(tmp,"%c",charTyped);
		strcat(mCapturedText,tmp);
		mLastChatKey = charTyped;
		mChatTimer.reset();	// start timer
	}
}

bool InputHandler::isCapturingText() {
	return mIsCapturingText;
}

bool InputHandler::isDebugKeyDown() {
	if (mToggleDebugKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mDebugKey)) {
		mToggleDebugKeyLock = true;
		return true;
	}
	return false;
}

// check if the 'quit' button is pushed
bool InputHandler::isQuitKeyDown() {
	if (mToggleQuitKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mQuitKey)) {
		mToggleQuitKeyLock = true;
		return true;
	}
	return false;
}

// check if the 'pause' button is pushed
bool InputHandler::isPauseKeyDown() {
	if (mTogglePauseKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPauseKey)) {
		mTogglePauseKeyLock = true;
		return true;
	}
	return false;
}

// check if the 'enter chat (talk)' button is pushed
bool InputHandler::isEnterChatKeyDown() {
		if (mToggleEnterChatKeyLock) {
			return false;
		}
		if (mKeyboard->isKeyDown(mEnterChatKey)) {
			mToggleEnterChatKeyLock = true;
			return true;
		}	
		return false;
}

// check if 'toggle mouse pointer' button is pushed
bool InputHandler::isTogglePointerKeyDown() {
	if (mTogglePointerKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mTogglePointerKey)) {
		mTogglePointerKeyLock = true;
		return true;
	}
	return false;
}

// check if 'toggle debug panel' button is pushed
bool InputHandler::isToggleDebugPanelKeyDown() {
	if (mToggleDebugPanelKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mToggleDebugPanelKey)) {
		mToggleDebugPanelKeyLock = true;
		return true;
	}
	return false;
}

// check if 'stats' button is pushed
bool InputHandler::isStatsModeKeyDown() {
	if (mStatsModeKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mStatsModeKey)) {
		mStatsModeKeyLock = true;
		return true;
	}
	return false;
}

// check if 'save plot' button is pushed
bool InputHandler::isSavePlotKeyDown() {
	if (mSavePlotKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mSavePlotKey)) {
		mSavePlotKeyLock = true;
		return true;
	}
	return false;
}

bool InputHandler::isChaseCamKeyDown() {
	if (mChaseCamKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mChaseCamKey)) {
		mChaseCamKeyLock = true;
		return true;
	}
	return false;
}

bool InputHandler::isEditModeNoneKeyDown() {
	if (mEditModeNoneLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mEditModeNoneKey)) {
		mEditModeNoneLock = true;
		return true;
	}
	return false;
}

bool InputHandler::isEditMode1KeyDown() {
	if (mEditMode1Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mEditMode1Key)) {
		mEditMode1Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isEditMode2KeyDown() {
	if (mEditMode2Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mEditMode2Key)) {
		mEditMode2Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isEditMode3KeyDown() {
	if (mEditMode3Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mEditMode3Key)) {
		mEditMode3Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isEditMode4KeyDown() {
	if (mEditMode4Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mEditMode4Key)) {
		mEditMode4Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isEditMode5KeyDown() {
	if (mEditMode5Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mEditMode5Key)) {
		mEditMode5Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintModeNoneKeyDown() {
	if (mPaintModeNoneLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintModeNoneKey)) {
		mPaintModeNoneLock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode1KeyDown() {
	if (mPaintMode1Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode1Key)) {
		mPaintMode1Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode2KeyDown() {
	if (mPaintMode2Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode2Key)) {
		mPaintMode2Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode3KeyDown() {
	if (mPaintMode3Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode3Key)) {
		mPaintMode3Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode4KeyDown() {
	if (mPaintMode4Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode4Key)) {
		mPaintMode4Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode5KeyDown() {
	if (mPaintMode5Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode5Key)) {
		mPaintMode5Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode6KeyDown() {
	if (mPaintMode6Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode6Key)) {
		mPaintMode6Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isPaintMode7KeyDown() {
	if (mPaintMode7Lock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mPaintMode7Key)) {
		mPaintMode7Lock = true;
		return true;
	}
	return false;
}

bool InputHandler::isDeleteKeyDown() {
	if (mDeleteKeyLock) {
		return false;
	}
	if (mKeyboard->isKeyDown(mDeleteKey)) {
		mDeleteKeyLock = true;
		return true;
	}
	return false;
}

bool InputHandler::isRotatePosKeyDown() {
	if (mKeyboard->isKeyDown(mRotatePosKey)) {
		return true;
	}
	return false;
}

bool InputHandler::isRotateNegKeyDown() {
	if (mKeyboard->isKeyDown(mRotateNegKey)) {
		return true;
	}
	return false;
}

bool InputHandler::isForwardKeyDown() {
	if (mKeyboard->isKeyDown(mForwardKey)) {
		return true;
	}
	if (mKeyboard->isKeyDown(mForwardKey2)) {
		return true;
	}
	return false;
}

bool InputHandler::isBackwardKeyDown() {
	if (mKeyboard->isKeyDown(mBackwardKey)) {
		return true;
	}
	if (mKeyboard->isKeyDown(mBackwardKey2)) {
		return true;
	}
	return false;
}

bool InputHandler::isSlideLeftKeyDown() {
	if (mKeyboard->isKeyDown(mSlideLeftKey)) {
		return true;
	}
	return false;
}

bool InputHandler::isSlideRightKeyDown() {
	if (mKeyboard->isKeyDown(mSlideRightKey)) {
		return true;
	}
	return false;
}

bool InputHandler::isFlyUpKeyDown() {
	if (mKeyboard->isKeyDown(mFlyUpKey)) {
		return true;
	}
	if (mKeyboard->isKeyDown(mFlyUpKey2)) {
		return true;
	}
	return false;
}

bool InputHandler::isFlyDownKeyDown() {
	if (mKeyboard->isKeyDown(mFlyDownKey)) {
		return true;
	}
	if (mKeyboard->isKeyDown(mFlyDownKey2)) {
		return true;
	}
	return false;
}

bool InputHandler::isTurnUpKeyDown() {
	if (mKeyboard->isKeyDown(mTurnUpKey)) {
		return true;
	}
	if (mKeyboard->isKeyDown(mTurnUpKey2)) {
		return true;
	}
	return false;
}

bool InputHandler::isTurnDownKeyDown() {
	if (mKeyboard->isKeyDown(mTurnDownKey)) {
		return true;
	}
	if (mKeyboard->isKeyDown(mTurnDownKey2)) {
		return true;
	}
	return false;
}

bool InputHandler::isTurnLeftKeyDown() {
	if (mKeyboard->isKeyDown(mTurnLeftKey)) {
		return true;
	}
	return false;
}

char* InputHandler::getCapturedText() {
	return mCapturedText;
}

bool InputHandler::isTurnRightKeyDown() {
	if (mKeyboard->isKeyDown(mTurnRightKey)) {
		return true;
	}
	return false;
}

int InputHandler::getMouseX() {
	return mMouseX;	
}

int InputHandler::getMouseY() {
	return mMouseY;	
}

int InputHandler::getMouseRelX() {
	return mMouseRelX;
}

int InputHandler::getMouseRelY() {
	return mMouseRelY;
}

bool InputHandler::isLMBDown() {
	// don't even check if the mouse is still locked from last time
	if (mLMBLock) {
		return false;
	}
	
	// check if button is down
	if (mMouse->getMouseState().buttonDown(OIS::MB_Left)) {
		mLMBLock = true;	// lock button (so we only get 1 click)
		return true;
	}
	return false;
}

bool InputHandler::isRMBDown() {
	// don't even check if the mouse is still locked from last time
	if (mRMBLock) {
		return false;
	}
	
	if (mMouse->getMouseState().buttonDown(OIS::MB_Right)) {
		mRMBLock = true;	// lock button (so we only get 1 click)
		return true;
	}
	return false;	
}

bool InputHandler::isMMBDown() {
	if (mMouse->getMouseState().buttonDown(OIS::MB_Middle)) {
		return true;
	}
	return false;	
}

bool InputHandler::isLMBHeldDown() {
	return mMouse->getMouseState().buttonDown(OIS::MB_Left);
}

bool InputHandler::isRMBHeldDown() {
	return mMouse->getMouseState().buttonDown(OIS::MB_Right);
}

