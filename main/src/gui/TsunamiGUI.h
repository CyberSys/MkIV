#ifndef TSUNAMIGUI_H_
#define TSUNAMIGUI_H_

#include "Ogre.h"
#include "OgreStringConverter.h"
#include "OgreException.h"				// probably not actually used here
#include "MouseCursor.h"

using namespace Ogre;

/* 11/07 Most of the functionality class is being phased out and replaced with the widget system. Most of the functions are now redundant (except for the mouse cursor and some other gui management) */

/* This is TSUNAMI GUI
 * I have made it myself after an exhaustive study of GUI libraries
 * All of them were crap, and it turns out it's really easy to make a GUI in Ogre
 */
class TsunamiGUI {
public:
	bool init(unsigned int width, unsigned int height);
	bool free();

	void showChatMsgs(bool showchat = true);
	void showDebug(bool showdebug = true);
	void showClientMenu(bool showmenu = true);
	void showSaveTitle(bool showmenu = true);
	void showChatEntry(bool showchat = true);
	void showConnecting(bool showconn = true);
	void showLoading(bool show = true);
	void showSaving(bool show = true);
	void showPaintBrushes(bool show = true);
	void showLobby(bool show, int mode = 0); // level {0,1,2} = {solo,client,server}
	void hideAllPanels();

	bool updateStats(RenderWindow* window);
	void updateCameraText(const char* text);
	void updateCamAimText(const char* text);
	void updateMousePosition();
	void updateConnectingTimer(unsigned long time);
	void updateMeReadyText(bool ready);
	void updateReadyCounter(int ready, int notready);

	void clearChatMsgs(); // wipe all the chat msgs
	void clearAllLobbyLists();
	void clearLobbySpecList();
	void clearLobbyF0List();
	void clearLobbyF1List();

	void addChat(const char* msg);
	bool pushLobbySpecName(const char *name);
	bool pushLobbyF0Name(const char *name);
	bool pushLobbyF1Name(const char *name);
	void setLobbyScenTitle(const char *title);
	
	bool isShowingDebugPanel(); // query whether or not we are showing the FPS panel
private:
	MouseCursor	mMouseCursor; // mouse cursor graphics
	Overlay* mDebugOverlay; // panel with FPS etc. on it
	Overlay* mChatOverlay; // panel to display chat from other players
	Overlay* mClientMenuOverlay; // the 'enter IP address of server' overlay
	Overlay* mChatEntryOverlay; // the 'enter chat' overlay
	Overlay* mConnectingOverlay; // connecting to server...
	Overlay* mStartingOverlay; // loading scenario...
	Overlay* mSavingOverlay; // saving...
	Overlay* mLobbyOverlay; // lobby which doubles as single player scenario chooser
	Overlay* mSaveOverlay; // 'enter scenario title' overlay
	char mChatMsg[5][256]; // last 5 chat messages from other players
	int mLatestChatIndex; // index of the newest chat mesg.
	bool mShowingDebugPanel; // whether or not we are showing the FPS panel
	int mLobbySpecCount; // count of spectators names
	int mLobbyF0Count; // count of force1 names
	int mLobbyF1Count; // count of force2 names
};

#endif /*TSUNAMIGUI_H_*/
