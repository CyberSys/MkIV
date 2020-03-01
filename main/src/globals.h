#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "scenery/TerrainHandler.h"
#include "input/InputHandler.h"
#include "network/Server.h"
#include "network/Client.h"
#include "gui/TsunamiGUI.h"
#include "gui/LineGraphWidget.h" // for scientific overlays

//#define _GO_GRIDS_ // Turn Grids on/off
#ifdef _GO_GRIDS_
#include "ai/BrainGrids.h"
#include "gui/GridOverlay.h"
#endif

#define VERSION "Alpha 10.01.18 (Mjothvitnir)"

//#define HACK_NOGUIFREE // a hack to remove the freeing of Overlay elements (due to Ogre segfault)

/*
 * 9. Then sought the gods their assembly-seats,
The holy ones, and council held,
To find who should raise the race of dwarfs
Out of  Brimir's blood and the legs of Blain.
10. There was Motsognir the mightiest made
Of all the dwarfs, and Durin next;
Many a likeness of men they made,
The dwarfs in the earth, as Durin said.
11. Nyi and Nithi,  Northri and Suthri,
 Austri and Vestri,  Althjof, Dvalin,
Nar and  Nain, Niping, Dain,
 Bifur, Bofur, Bombur,  Nori,
An and Onar,  Ai, Mjothvitnir,
12. Vigg and  Gandalf, Vindalf, Thrain,
 Thekk and Thorin, Thror,  Vit and Lit,
 Nyr and Nyrath,
Regin and  Rathvith — now have I told the list aright.
13. Fili, Kili, Fundin,  Nali,
Heptifili,  Hannar, Sviur,
 Frar, Hornbori,  Fræg and Loni,
 Aurvang, Jari, Eikinskjaldi.
14. The race of the dwarfs in Dvalin's throng
Down to Lofar the list must I tell;
The rocks they left, and through the wet lands
They sought a home in the fields of sand.
15. There were Draupnir and Dolgthrasir ,
Hor,  Haugspori, Hlevang, Gloin,
 Dori, Ori, Duf,  Andvari,
Skirfir,  Virfir, Skafith,  Ai.
16. Alf and Yngvi, Eikinskjaldi;
Fjalar and Frosti,  Fith and Ginnar;
So for all time shall the tale be known,
The list of all the forbears of Lofar.
 */

struct Score {
	int mUnitKills;
	int mUnitsStart;
	int mUnitsRemaining;
};

// global (useful) functions
// Anton's String Stripper
// grabs all of string after index 'from' and stores in a new string
void StripString(char* instr, int from, char* outstr, int max);	// max is the length of the output string
void frameLimiter(unsigned long &elapsed);	// limit frame rate to 250fps

// global variables (handlers that need to be available to everything)
struct GlobalStruct {
	InputHandler mInputHandler;
	Server mServer; // server interface
	Client mClient; // client interface
	TsunamiGUI mGUI; // Tsunami gui
//	SoundManager mAudioHandler; // audio with fmod
	LineGraphWidget mLineGraphWidgetA; // a panel displaying a line graph (test implementation)
	LineGraphWidget mLineGraphWidgetB; // a panel displaying a line graph (test implementation)
	LineGraphWidget mLineGraphWidgetC; // a panel displaying a line graph (test implementation)
	LineGraphWidget mFuzzyObstDistGraph; // a panel displaying fuzzy input set graph for selected vehicle
	LineGraphWidget mFuzzyObstAngleGraph; // a panel displaying fuzzy input set graph for selected vehicle
	LineGraphWidget mDefuzzifiedSteeringGraph; // panel displaying steering instructions as resultant from FAMM defuzzification
	LineGraphWidget mCrashGraph; // graph for tracking total crashes
	#ifdef _GO_GRIDS_
	BrainGrids mBrainGrid; // system for computing map of terrain in discreet format
	BrainGrids mSmallGrid; // high-res grid used by small vehicles and infantry
	GridOverlay mGridOverlay;
	GridOverlay mSmallGridOverlay;
	#endif
	Timer mSimTimer; // total time elapsed since sim start, including time paused
	Score mScores[10];
	double mTimeCompressionMultiplier; // time compression multiplier for simulation time
	double mCrashHeuristic; // a heuristic for total crash severity in simulation
	double mCamFarClip;
	double mCamNearClip;
	double mRandomStartDistance;
	char mDebugText[256];
	char mUserName[15]; // our user name
	char mFuzzySetFile[256];
	char mFuzzyRulesFile[256];
	char mCompositorName[256]; // name of graphics post-process to use
	char mScenario[256];
	char mSettingsFile[256];
	char mBatchOutputFileName[256];
	char mMenuMusicTrack[256];
	unsigned long mTotalTimePaused; // cumulative total of time that sim has been paused
	unsigned long mTimeoutUS; // timeout in microseconds
	int mOurForce; // number of force that we control
	int mNavRunId; // id number of run for simulation batches
	int mCamPolygonMode;
	bool mNoAudio; // global audio-startup flag
	bool mEnableMenuMusic;
	bool mPointerMode; // true=ShowMousePointer false=MouseRotateCamera
	bool mIsServer;
	bool mIsClient;
	bool mIsPaused; // if simulation time is currently paused
	bool mButtonHatchesOrderGiven;
	bool mSuppressUserGfxChooser; // suppress user graphics chooser
	bool mUseCompositor;
	bool mGameOver;
	bool mEnableShadows;
	bool mEnableSpecularLighting;
	bool mEnablePagedGeometry;
	bool mNeedToUpdateScenario;
	bool mNavGraphsShowing;
	bool mUsingRandomStartLocation;
	bool mUseGlobalFuzzyRules;
	bool mMusicAlreadyPlaying;
	bool mEnableGFS;
	GlobalStruct() {
		reset(); // run-time globals (others are pre-configured)
		strcpy(mUserName, "UnknownSoldier");
		strcpy(mDebugText, "Particle/Exhaust");
		strcpy(mFuzzySetFile, "ai/default-fuzzy-sets.txt"); // set default file
		strcpy(mFuzzyRulesFile, "ai/default-fuzzy-rules.txt"); // set default file
		strcpy(mCompositorName, "");
		strcpy(mScenario, "dogsofwar.sce");
		strcpy(mSettingsFile, "settings.ini");
		strcpy(mBatchOutputFileName, "batch-results-nav-out.txt");
		strcpy(mMenuMusicTrack, "");
		mTimeCompressionMultiplier = 1.0f;
		mCamFarClip = 500.0f;
		mCamNearClip = 1.0f;
		mCamPolygonMode = 3; // default to full-textured rendering
		mNoAudio = false;
		mEnableMenuMusic = false;
		mSuppressUserGfxChooser = false;
		mEnableShadows = false;
		mEnableSpecularLighting = true;
		mEnablePagedGeometry = true;
		mUsingRandomStartLocation = false;
		mUseGlobalFuzzyRules = false;
		mMusicAlreadyPlaying = false;
		mEnableGFS = false;
	}
	void reset() {
		for (int i = 0; i < 10; i++) {
			mScores[i].mUnitKills = mScores[i].mUnitsStart = mScores[i].mUnitsRemaining = 0;
		}
		mPointerMode = true;
		mIsServer = false;
		mIsClient = false;
		mIsPaused = false;
		mButtonHatchesOrderGiven = false;
		mGameOver = false;
		mNeedToUpdateScenario = false;
		mNavGraphsShowing = false;
		mCrashHeuristic = 0.0f;
		mTotalTimePaused = 0;
		mTimeoutUS = 0;
		mOurForce = -1;	// start as an observer
		mNavRunId = -1;
   }
	bool loadSettingsFromFile(char* filename);
};

extern GlobalStruct g;

#endif /*GLOBALS_H_*/

