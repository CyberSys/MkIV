#include "TsunamiGUI.h"
#include <string.h>
#include "../globals.h"	// needed to display 'VERSION' definition

// NOTE: don't need to load all panels into memory at start time

// allocate memory to GUI
bool TsunamiGUI::init(unsigned int width, unsigned int height) {
	mMouseCursor.init();	// init mouse cursor
	mMouseCursor.setImage("cursor_black.png");
	mMouseCursor.setVisible(true);
	mMouseCursor.setWindowDimensions(width, height);

	// reset chat strings
	for (int i =0; i < 5; i++) {
		strcpy(mChatMsg[i],"");
	}
	mLatestChatIndex = 4;
	
	mDebugOverlay = OverlayManager::getSingleton().getByName("mkiv/DebugOverlay");
	mChatOverlay = OverlayManager::getSingleton().getByName("mkiv/ChatOverlay");
	mChatEntryOverlay = OverlayManager::getSingleton().getByName("mkiv/ChatEntryOverlay");
	mClientMenuOverlay = OverlayManager::getSingleton().getByName("mkiv/ClientOverlay");
	mConnectingOverlay = OverlayManager::getSingleton().getByName("mkiv/ConnectingOverlay");
	mStartingOverlay = OverlayManager::getSingleton().getByName("mkiv/StartingOverlay");
	mSavingOverlay = OverlayManager::getSingleton().getByName("mkiv/SavingOverlay");
	mLobbyOverlay = OverlayManager::getSingleton().getByName("mkiv/Lobby");
	mSaveOverlay = OverlayManager::getSingleton().getByName("mkiv/SaveOverlay");
	// put version number on main menu
	OverlayElement* versionTxt = OverlayManager::getSingleton().getOverlayElement("mkiv/versionText");
	versionTxt->setCaption(VERSION);

	mShowingDebugPanel = false;
	mLobbySpecCount = 0;					// count of spectators names
	mLobbyF0Count = 0;						// count of force1 names
	mLobbyF1Count = 0;						// count of force2 names	
	
	return true;
}

// free memory
bool TsunamiGUI::free() {
	if (!mLobbyOverlay) {	// half-arsed validation
		return false;
	}
	// delete all the overlays
	delete mSaveOverlay;
	delete mLobbyOverlay;
	delete mStartingOverlay;
	delete mSavingOverlay;
	delete mConnectingOverlay;
	delete mClientMenuOverlay;
	delete mChatOverlay;
	delete mDebugOverlay;
	
	return true;
}

// show/hide the chat messages from other players panel
void TsunamiGUI::showChatMsgs(bool showchat) {
	if (showchat) {
		mChatOverlay->show();
	} else {
		mChatOverlay->hide();
	}	
}

// show/hide the chat entry dialogue
void TsunamiGUI::showChatEntry(bool showchat) {
	if (showchat) {
		mChatEntryOverlay->show();
	} else {
		mChatEntryOverlay->hide();
	}	
}

// show/hide the Debug panel (FPS counter etc.)
void TsunamiGUI::showDebug(bool showdebug) {
	if (showdebug) {
		mDebugOverlay->show();
		mShowingDebugPanel = true;
		OverlayManager::getSingleton().getByName("mkiv/GFSOverlay")->show();
	} else {
		mDebugOverlay->hide();
		mShowingDebugPanel = false;
		OverlayManager::getSingleton().getByName("mkiv/GFSOverlay")->hide();
	}	
}

// show/hide the Client's Server IP entering panel
void TsunamiGUI::showClientMenu(bool showmenu) {
	if (showmenu) {
		mClientMenuOverlay->show();
	} else {
		mClientMenuOverlay->hide();
	}	
}

// show/hide the Scenario Title entering panel
void TsunamiGUI::showSaveTitle(bool showmenu) {
	if (showmenu) {
		mSaveOverlay->show();
	} else {
		mSaveOverlay->hide();
	}	
}

// show/hide the Connecting to Server panel
void TsunamiGUI::showConnecting(bool showconn) {
	if (showconn) {
		mConnectingOverlay->show();
	} else {
		mConnectingOverlay->hide();
	}
}

// show/hide the Loading Scenario panel
void TsunamiGUI::showLoading(bool show) {
	if (show) {
		mStartingOverlay->show();
	} else {
		mStartingOverlay->hide();
	}
}

// show/hide the Saving Scenario panel
void TsunamiGUI::showSaving(bool show) {
	if (show) {
		mSavingOverlay->show();
	} else {
		mSavingOverlay->hide();
	}

}

// show/hide the lobby GUI/menu
void TsunamiGUI::showLobby(bool show, int mode) {
	if (show) {
		mLobbyOverlay->show();

		OverlayContainer* readyCountText = mLobbyOverlay->getChild("mkiv/ReadyCountText");
		OverlayContainer* meReadyText = mLobbyOverlay->getChild("mkiv/MeReadyText");
		OverlayContainer* readyButtonPanel = mLobbyOverlay->getChild("mkiv/ReadyButtonPanel");
		OverlayContainer* changeSceneButton = mLobbyOverlay->getChild("mkiv/ChangeScenarioButtonPanel");
		OverlayContainer* startButton = mLobbyOverlay->getChild("mkiv/StartButtonPanel");

		// show/hide mode-specific parts of gui
		if (mode == 0) {	// solo mode
			readyCountText->hide();
			meReadyText->hide();
			readyButtonPanel->hide();
			changeSceneButton->show();
			startButton->show();
		} else if (mode == 1) {	// client mode
			readyCountText->show();
			meReadyText->show();
			readyButtonPanel->show();
			changeSceneButton->hide();
			startButton->hide();
		} else if (mode == 2) {	// server mode
			readyCountText->show();
			meReadyText->show();
			readyButtonPanel->show();
			changeSceneButton->show();
			startButton->show();
		}
	} else {
		mLobbyOverlay->hide();
	}
}

// hide all the overlays
void TsunamiGUI::hideAllPanels() {
	mLobbyOverlay->hide();
	mStartingOverlay->hide();
	mSavingOverlay->hide();
	mConnectingOverlay->hide();
	mDebugOverlay->hide();
	mChatEntryOverlay->hide();
	mChatOverlay->hide();
}

// update the mouse coords
void TsunamiGUI::updateMousePosition() {
	mMouseCursor.updatePosition(g.mInputHandler.getMouseX(), g.mInputHandler.getMouseY());
}

// change the countdown clock on the Connecting to Server panel
void TsunamiGUI::updateConnectingTimer(unsigned long time) {
	OverlayElement* connTitle = OverlayManager::getSingleton().getOverlayElement("mkiv/ConnectingTitle");
	connTitle->setCaption("Waiting for Server Response..." + StringConverter::toString(time));
}

// indicate if we have clicked ready or not
void TsunamiGUI::updateMeReadyText(bool ready) {
	OverlayElement* meReady = OverlayManager::getSingleton().getOverlayElement("mkiv/MeReadyText");
	if (ready) {
		meReady->setCaption("We are ready");
	} else {
		meReady->setCaption("We are NOT ready");
	}
}

// change the clients ready counter on the lobby gui
void TsunamiGUI::updateReadyCounter(int ready, int notready) {
	int total = ready + notready;
	OverlayElement* connTitle = OverlayManager::getSingleton().getOverlayElement("mkiv/ReadyCountText");
	connTitle->setCaption(StringConverter::toString(ready) + " / " + StringConverter::toString(total) + " Players Ready");
}

// clear all lists of players in lobby gui
void TsunamiGUI::clearAllLobbyLists() {
	clearLobbySpecList();
	clearLobbyF0List();
	clearLobbyF1List();
}

// clear list of spectators in lobby gui
void TsunamiGUI::clearLobbySpecList() {
	OverlayElement* line[4];
	line[0] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player0");
	line[1] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player1");
	line[2] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player2");
	line[3] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player3");
	line[0]->setCaption("");
	line[1]->setCaption("");
	line[2]->setCaption("");
	line[3]->setCaption("");
	mLobbySpecCount = 0;
}

// clear list of force0 players in lobby gui
void TsunamiGUI::clearLobbyF0List() {
	OverlayElement* line[4];
	line[0] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player0");
	line[1] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player1");
	line[2] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player2");
	line[3] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player3");
	line[0]->setCaption("");
	line[1]->setCaption("");
	line[2]->setCaption("");
	line[3]->setCaption("");
	mLobbyF0Count = 0;
}

// clear list of force1 players in lobby gui
void TsunamiGUI::clearLobbyF1List() {
	OverlayElement* line[4];
	line[0] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player0");
	line[1] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player1");
	line[2] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player2");
	line[3] = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player3");
	line[0]->setCaption("");
	line[1]->setCaption("");
	line[2]->setCaption("");
	line[3]->setCaption("");
	mLobbyF1Count = 0;
}

// add a player name to force list
bool TsunamiGUI::pushLobbySpecName(const char *name) {
	if (mLobbySpecCount >= 4) {
		return false;
	}
	OverlayElement* line = NULL;
	if (mLobbySpecCount == 0) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player0");
	} else if (mLobbySpecCount == 1) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player1");
	} else if (mLobbySpecCount == 2) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player2");
	} else if (mLobbySpecCount == 3) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield0Player3");
	}
	line->setCaption(name);
	mLobbySpecCount++;
	
	return true;
}

// add a player name to force list
bool TsunamiGUI::pushLobbyF0Name(const char *name) {
	if (mLobbyF0Count >= 4) {
		return false;
	}
	OverlayElement* line = NULL;
	if (mLobbyF0Count == 0) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player0");
	} else if (mLobbyF0Count == 1) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player1");
	} else if (mLobbyF0Count == 2) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player2");
	} else if (mLobbyF0Count == 3) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield1Player3");
	}
	line->setCaption(name);
	mLobbyF0Count++;
	
	return true;
}

// add a player name to force list
bool TsunamiGUI::pushLobbyF1Name(const char *name) {
	if (mLobbyF1Count >= 4) {
		return false;
	}
	OverlayElement* line = NULL;
	if (mLobbyF1Count == 0) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player0");
	} else if (mLobbyF1Count == 1) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player1");
	} else if (mLobbyF1Count == 2) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player2");
	} else if (mLobbyF1Count == 3) {
		line = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyShield2Player3");
	}
	line->setCaption(name);
	mLobbyF1Count++;
	
	return true;
}

// change the name of the selected scenario in the lobby
void TsunamiGUI::setLobbyScenTitle(const char *title) {
	// TODO will also need to inform clients when scenario changed
	OverlayElement* guititle = OverlayManager::getSingleton().getOverlayElement("mkiv/LobbyTitleText");
	guititle->setCaption(title);
}

// update content
bool TsunamiGUI::updateStats(RenderWindow* window) {
  static String currFps = "Current FPS: ";
  static String avgFps = "Average FPS: ";
  static String bestFps = "Best FPS: ";
  static String worstFps = "Worst FPS: ";
  static String tris = "Triangle Count: ";

  // update stats when necessary
  OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("mkiv/AverageFps");
  OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("mkiv/CurrFps");
  OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("mkiv/BestFps");
  OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("mkiv/WorstFps");

  const RenderTarget::FrameStats& stats = window->getStatistics();

  guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
  guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
  guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
      +" "+StringConverter::toString(stats.bestFrameTime)+" ms");
  guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
      +" "+StringConverter::toString(stats.worstFrameTime)+" ms");

  OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("mkiv/NumTris");
  guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

  OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("mkiv/DebugText");
	guiDbg->setCaption("THIS IS A TEXT TEST");
	
	return true;	
}

void TsunamiGUI::updateCameraText(const char* text) {
	OverlayElement* cameraText = OverlayManager::getSingleton().getOverlayElement("mkiv/CameraText");
	cameraText->setCaption(text);
}

void TsunamiGUI::updateCamAimText(const char* text) {
	OverlayElement* cameraText = OverlayManager::getSingleton().getOverlayElement("mkiv/CamAim");
	cameraText->setCaption(text);
}

// add some chat message input to display
void TsunamiGUI::addChat(const char* msg) {
	mLatestChatIndex = (mLatestChatIndex + 1) % 5;
	
	strcpy(mChatMsg[mLatestChatIndex],msg);	// replace oldest string
	// update chat strings in panel
	OverlayElement* string0 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine0");
	OverlayElement* string1 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine1");
	OverlayElement* string2 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine2");
	OverlayElement* string3 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine3");
	OverlayElement* string4 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine4");
	
	// put onto display in time order
	string0->setCaption(mChatMsg[mLatestChatIndex]);
	string4->setCaption(mChatMsg[(mLatestChatIndex + 1) % 5]);
	string3->setCaption(mChatMsg[(mLatestChatIndex + 2) % 5]);
	string2->setCaption(mChatMsg[(mLatestChatIndex + 3) % 5]);
	string1->setCaption(mChatMsg[(mLatestChatIndex + 4) % 5]);
}

// return true if the debug panel (FPS etc) is being shown
bool TsunamiGUI::isShowingDebugPanel() {
	return mShowingDebugPanel;
}

// clear out the chat messages box
void TsunamiGUI::clearChatMsgs() {
	OverlayElement* string0 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine0");
	OverlayElement* string1 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine1");
	OverlayElement* string2 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine2");
	OverlayElement* string3 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine3");
	OverlayElement* string4 = OverlayManager::getSingleton().getOverlayElement("mkiv/ChatLine4");
	string0->setCaption("");
	string4->setCaption("");
	string3->setCaption("");
	string2->setCaption("");
	string1->setCaption("");
	for (int i = 0; i < 5; i++) {
		mChatMsg[i][0] = '\0';
	}
}

