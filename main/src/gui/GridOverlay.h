#ifndef MARKIV_GRID_OVERLAY_H_
#define MARKIV_GRID_OVERLAY_H_

#include "Ogre.h"
#include "ThreeDeeLines.h"
#include "../scenery/TerrainHandler.h"
#include "../ai/BrainGrids.h"
using namespace Ogre;

#define MAXREDSQUARES 30

struct GridOverlay {
	GridOverlay();
	bool init(SceneManager* sceneMgr, TerrainHandler* th, int cellsAcross, int cellsDown, float cellSize_m, const char* uniqueName, float r, float g, float b, float a);
	bool free();
	bool update(const Vector3& startPosition, BrainGrids& bg);
	bool show(BrainGrids& bg);
	void hide();
	void redSquare(Vector3& pos);
	SceneManager* mpSceneManager;
	TerrainHandler* mpTerrainHandler;
	ManualObject* mSquares[MAXREDSQUARES];
	MaterialPtr mMaterial;
	ThreeDeeLines mLineHandler;
	char mUniqueName[128];
	float mCellSize_m;
	int mLineCount;
	int mCellsDown;
	int mCellsAcross;
	int mNumberOfSquares;
	int mOldCentreSquareX;
	int mOldCentreSquareY;
	int mOldStartY;
	int mOldEndY;
	int mOldStartX;
	int mOldEndX;
	bool mIsShowing;
};

#endif

