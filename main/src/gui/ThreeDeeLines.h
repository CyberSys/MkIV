#ifndef MARKIV_THREEDEE_LINES_H_
#define MARKIV_THREEDEE_LINES_H_

#include "Ogre.h"
#include "../scenery/TerrainHandler.h"
using namespace Ogre;

#define MAX_3D_LINES 1024

// a class for contstructing 3D lines
struct ThreeDeeLines {
	ThreeDeeLines();
	bool init(SceneManager* sceneMgr, const char* uniqueName);
	bool free();
	int constructLine(float r, float g, float b, float a); // create a new 3d line - returns index or -1
	bool start3DLineStrip(int lineIndex); // start drawing with a line
	bool addPointTo3DLineStrip(int lineIndex, Vector3 position); // draw a point with line
	bool end3DLineStrip(int lineIndex); // stop drawing with a line
	bool hide3DLine(int lineIndex); // stop drawing a line
	bool show3DLineFromHereToThere(int lineIndex, const Vector3 &here, const Vector3 &there);
	SceneManager* pSceneManager; // ptr to the scenemanager
	ManualObject* mp3DLineObject[MAX_3D_LINES]; // for making 3D lines
	SceneNode* mp3DLineNode[MAX_3D_LINES]; // for making 3D lines
	MaterialPtr mp3DLineMaterial[MAX_3D_LINES]; // for making 3D lines
	char mUniqueName[256]; // for naming things without interferring with other instances of this class
	int mNumberOfLines; // count of lines created
};

#endif

