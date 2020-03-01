#include <stdio.h>
#include "Ogre.h"
#include "MarkIV.h"
#include "globals.h"
#include "audio/AudioHandler.h"
#include <string.h>

using namespace Ogre;

GlobalStruct g; // solidify globals

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	#define WIN32_LEAN_AND_MEAN
	#include "windows.h"
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT ) {
	g.reset(); // reset global variables
#else
int main(int argc, char **argv) {
	g.reset(); // reset global variables
	if (argc > 1) { // command line debug switches
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--noaudio") == 0) {	// turn off audio (only a linux development trick - not a practical audio off). Still tries (and fails) to play sounds during simulation anyway.
				g.mNoAudio = true;
				printf("MKIVENGINE: Not Loading Audio\n");
			} else if (strcmp(argv[i], "--fuzzyfile") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the file to load\n");
					return -1;
				}
				strcpy(g.mFuzzySetFile, argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "--fuzzyrules") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the file to load\n");
					return -1;
				}
				strcpy(g.mFuzzyRulesFile, argv[i + 1]);
				g.mUseGlobalFuzzyRules = true;
				i++;
			} else if (strcmp(argv[i], "--version") == 0) {
				printf("markiv engine %s\n", VERSION);
				return 1;
			} else if (strcmp(argv[i], "--nogfxprompt") == 0) {
				g.mSuppressUserGfxChooser = true;
			} else if (strcmp(argv[i], "--scenario") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the file to load\n");
					return -1;
				}
				strcpy(g.mScenario, argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "--settings") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the file to load\n");
					return -1;
				}
				strcpy(g.mSettingsFile, argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "--batchout") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the file to append to\n");
					return -1;
				}
				strcpy(g.mBatchOutputFileName, argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "--compositor") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the compositor name\n");
					return -1;
				}
				strcpy(g.mCompositorName, argv[i + 1]);
				g.mUseCompositor = true;
				i++;
			} else if (strcmp(argv[i], "--debug") == 0) {
				if (argc < i + 2) {
					printf("FAILED: no text given\n");
					return - 1;
				}
				strcpy(g.mDebugText, argv[i + 1]);
				i++;
			} else if (strcmp(argv[i], "--noshadows") == 0) {
				g.mEnableShadows = false;
			} else if (strcmp(argv[i], "--noshiny") == 0) {
				g.mEnableSpecularLighting = false;
			} else if (strcmp(argv[i], "--runid") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the run id (integer)\n");
					return -1;
				}
				sscanf(argv[i + 1], "%i", &g.mNavRunId);
				i++;
			} else if (strcmp(argv[i], "--timecomp") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the run id (decimal number)\n");
					return -1;
				}
				sscanf(argv[i + 1], "%lf", &g.mTimeCompressionMultiplier);
				i++;
			} else if (strcmp(argv[i], "--timeout") == 0) {
				if (argc < i + 2) {
					printf("FAILED: you must specify the time in minutes (integer)\n");
					return -1;
				}
				int timeout = 0;
				sscanf(argv[i + 1], "%i", &timeout); // get timeout in minutes
				g.mTimeoutUS = timeout * 60000000; // store timeout in microseconds
				i++;
			} else if (strcmp(argv[i], "--help") == 0) {
				printf("markiv [options]\n");
				printf(" --help \t\tthis text\n");
				printf(" --version \t\tprint the version number of this build\n");
				printf(" --settings [FILE] \tuse a custom settings file\n");
				printf(" --noaudio \t\tdisable audio\n");
				printf(" --nogfxprompt \t\tdon't display graphics chooser window\n");
				printf(" --noshadows \t\tdisable shadows for performance gain\n");
				printf(" --noshiny \t\tdisable specularity\n");
				printf(" --compositor [NAME] \tuse a compositor given by NAME\n");
				printf(" --fuzzyfile [FILE] \tload a file for fuzzy set definitions\n");
				printf(" --fuzzyrules [FILE] \tuse only this file for fuzzy rules\n");
				printf(" --scenario [FILE] \tload a specified scenario (sans path)\n");
				printf(" --runid [NUMBER] \tgive this simulation run a unique id\n");
				printf(" --timeout [MINUTES] \tend the simulation after MINUTES\n");
				printf(" --timecomp [NUMBER] \tscale time compression, default 1.0\n");
				printf(" --batchout [FILE] \tname of file used to append run results to\n");
				printf(" --debug [TEXT] \tsome text to use for testing various things.\n");
				return 1;
			} else {
				printf("ERROR: unrecognised argument \"%s\". Use --help for a list of valid arguments.\n", argv[i]);
				return -1;
			}
		}
	}
	#endif

	printf("Mark IV Engine v%s\n",VERSION);
	if (!g.loadSettingsFromFile(g.mSettingsFile)) {
		printf("FATAL ERROR: could not load settings.ini file\n");
		exit(1);
	}
	SoundManager* sm = new SoundManager(); // create sound manager (used as singleton)
	MarkIV markIV;
	if (!markIV.init()) {
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        	//MessageBox(NULL,"init function failed", "An error has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		#else
			fprintf(stderr, "Initialisation cancelled by user.\n");
		#endif
		return 1;
	}
	while (true) { // META MAIN loop;
		int mode = -1;
		//if (g.mNavTest) {
			mode = 1; // just start simulation
		//} else {
			printf("Starting Main Menu\n");
			mode = markIV.mainMenuMode(); // display main menu (where you choose simulator mode)
			printf("Main Menu closed\n");
		//}

		if (mode < 0) { // quit on error
			printf("Quit on error; attempting to shutdown\n");
			break;
		} else if (mode == 0) { // quit
			printf("User quit from main menu.\n");
			break;
		} else if (mode == 1) { // start simulation
			// initialise network (if any)
			if (g.mIsServer) {
				printf("Starting Server\n");
				if (!markIV.startServer()) { // start network server
					printf("FATAL ERROR: could not initialise server. Terminating\n");
					break;
				}
			} else if (g.mIsClient) {
				printf("Starting Client\n");
				int clientFlags = markIV.startClient(); // start network client
				if (clientFlags < 0) { // error handling
					printf("FATAL ERROR: could not initialise client. Terminating\n");
					break;
				}
				if (clientFlags == 0) {
					continue;	// return to menu (connection timeout)
				}
			}

			int lobbyResult = -1;

			printf("Launching Lobby\n");
			lobbyResult = markIV.lobbyLoop(); // start lobby area/scenario chooser loop
			if (lobbyResult < 0) {
				printf("FATAL ERROR: Lobby/scenario loader failed. Terminating\n");
				break;
			} else if (lobbyResult == 0) {
				printf("Closing lobby\n");
				// shutdown network (if any)
				if (g.mIsServer) {
					if (!markIV.shutdownServer()) {
						printf("FATAL ERROR: could not shutdown server.\n");
						break;
					}
				}
				if (g.mIsClient) {
					if (!markIV.shutdownClient()) {
						printf("FATAL ERROR: could not shutdown client.\n");
						break;
					}
				}
				continue;	// return to main menu
			}

			SoundManager::getSingleton().stopAllSounds(); // stop any menu sounds and music
			g.mMusicAlreadyPlaying = false;
			// show loading message
			printf("Loading...\n");
			markIV.showLoadingOverlay();

			// initialise simulation-specific stuff
			printf("Initialising Simulator\n");
			if (!markIV.initSimulator()) {
				printf("FATAL ERROR: Could not initialise simulator. Terminating\n");
				break;
			}

			// main simulation loop
			printf("Starting main loop\n");
			if (!markIV.mainGameLoop()) {
				printf("FATAL ERROR: simulator quit with an error. Terminating\n");
				break;
			}

			printf("shutdown sim\n");
			// shutdown network (if any)
			if (g.mIsServer) {
				if (!markIV.shutdownServer()) {
					printf("FATAL ERROR: could not shutdown server.\n");
					break;
				}
			}
			if (g.mIsClient) {
				if (!markIV.shutdownClient()) {
					printf("FATAL ERROR: could not shutdown client.\n");
					break;
				}
			}
			// free simulation-specific stuff
			if (!markIV.shutdownSimulator()) {
				printf("FATAL ERROR: could not free memory. Terminating\n");
				break;
			}

		// start unit editor
		} else if (mode == 2) {
			SoundManager::getSingleton().stopAllSounds();
			g.mMusicAlreadyPlaying = false;
			// initialise editor-specific stuff
			if (!markIV.initScenarioEditor()) {
				printf("FATAL ERROR: could not initialise. Terminating\n");
			}

			// main editor loop
			if (!markIV.mainEditorLoop()) {
				printf("FATAL ERROR: editor quit with an error. Terminating\n");
			}

			// free editor-specific stuff
			if (!markIV.shutdownScenarioEditor()) {
				printf("FATAL ERROR: could not properly shutdown editor. Terminating\n");
			}
		}
		g.reset(); // reset global variables
	} // end of META main loop
	if (!markIV.shutdown()) {
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        	MessageBox(NULL,"shutdown function failed", "An error has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		#else
			fprintf(stderr, "ERROR: shutdown function failed!\n");
		#endif
		return 1;
	}
	return 0;
}

// strip a string out of another string from an index to a max size, including whitespace
void StripString(char* instr, int from, char* outstr, int max) {
	int instrlen = strlen(instr);	// get length of input string
	int outi = 0;	// index in output string
	// scan through entire input string
	for (int ini = from; ini < instrlen; ini++) {
		outstr[outi] = instr[ini];
		outi++;
		if (outi == max - 1) {	// prevent stack smashing of output string
			break;
		}
	}
	outstr[outi] = '\0';	// terminate output string
}

// limit frame rate to 250fps to avoid cpu overloads
void frameLimiter(unsigned long &elapsed) {
	if (elapsed < 4000) {	// 4000 *micro* seconds
		#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			Sleep((4000 - elapsed) / 1000);	// this is 2ms
		#else
			usleep(4000 - elapsed);	// microsleep
		#endif
	}
}

bool GlobalStruct::loadSettingsFromFile(char* filename) {
	ifstream file;
	file.open(filename);
	if(!file) {
		printf("Error: MarkIV::loadSettingsINIFile Could not open ini file\n");
		return false;
	}
	char iniDetail[256];
	char key[128];
	char next[256];
	while (file.getline(iniDetail, 256)) {
		strcpy(key, "");
		strcpy(next, "");
		sscanf(iniDetail, "%s %s", key, next);
		if (key[0] == '#') { // ignore comments
		} else if (strcmp(iniDetail, "") == 0) { // ignore blank lines
		} else if (strcmp(key, "ENABLE_GFX_PROMPT") == 0) {
			int result = 0;
			sscanf(next, "%i", &result);
			if (result == 0) {
				mSuppressUserGfxChooser = true;
			}
		} else if (strcmp(key, "ENABLE_AUDIO") == 0) {
			int result = 0;
			sscanf(next, "%i", &result);
			if (result == 0) {
				mNoAudio = true;
			}
		} else if (strcmp(key, "ENABLE_MENU_MUSIC") == 0) {
			strcpy(mMenuMusicTrack, next);
			mEnableMenuMusic = true;
		} else if (strcmp(key, "ENABLE_SHADOWS") == 0) {
			int result = 0;
			sscanf(next, "%i", &result);
			if (result == 1) {
				mEnableShadows = true;
			} else {
				mEnableShadows = false;
			}
		} else if (strcmp(key, "ENABLE_SPECULAR_LIGHTING") == 0) {
			int result = 0;
			sscanf(next, "%i", &result);
			if (result == 0) {
				mEnableSpecularLighting = false;
			} else {
				mEnableSpecularLighting = true;
			}
		} else if (strcmp(key, "ENABLE_PAGED_GEOMETRY") == 0) {
			int result = 0;
			sscanf(next, "%i", &result);
			if (result == 0) {
				mEnablePagedGeometry = false;
			} else {
				mEnablePagedGeometry = true;
			}
		} else if (strcmp(key, "SET_TIMOUT_US") == 0) {
			sscanf(next, "%li", &mTimeoutUS);
		} else if (strcmp(key, "USE_USERNAME") == 0) {
			strcpy(mUserName, next);
		} else if (strcmp(key, "USE_COMPOSITOR") == 0) {
			strcpy(mCompositorName, next);
			mUseCompositor = true;
		} else if (strcmp(key, "USE_SCENARIO") == 0) {
			strcpy(mScenario, next);
		} else if (strcmp(key, "USE_FUZZYSETS") == 0) {
			strcpy(mFuzzySetFile, next);
		} else if (strcmp(key, "USE_FUZZYRULES") == 0) {
			mUseGlobalFuzzyRules = true;
			strcpy(mFuzzyRulesFile, next);
		} else if (strcmp(key, "USE_DEBUG_TEXT") == 0) {
			strcpy(mDebugText, next);
		} else if (strcmp(key, "CAMERA_FARCLIP") == 0) {
			sscanf(next, "%lf", &mCamFarClip);
		} else if (strcmp(key, "CAMERA_NEARCLIP") == 0) {
			sscanf(next, "%lf", &mCamNearClip);
		} else if (strcmp(key, "SCALE_TIME_COMPRESSION") == 0) {
			sscanf(next, "%lf", &mTimeCompressionMultiplier);
		} else if (strcmp(key, "CAMERA_POLYGONMODE") == 0) {
			sscanf(next, "%i", &mCamPolygonMode);
		} else if (strcmp(key, "RANDOM_START_DISTANCE") == 0) {
			mUsingRandomStartLocation = true;
			sscanf(next, "%lf", &mRandomStartDistance);
		} else if (strcmp(key, "BATCHOUT_FILE") == 0) {
			strcpy(mBatchOutputFileName, next);
		} else if (strcmp(key, "ENABLE_GFS") == 0) {
			int result = 0;
			sscanf(next, "%i", &result);
			if (result == 0) {
				mEnableGFS = false;
			} else {
				mEnableGFS = true;
			}
		}
	}
	file.close();
	return true;
}

