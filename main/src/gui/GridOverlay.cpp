#include "GridOverlay.h"
#include "../ai/BrainGrids.h"

GridOverlay::GridOverlay() {
	mpSceneManager = NULL;
	mpTerrainHandler = NULL;
	strcpy(mUniqueName, "");
	mCellSize_m = 0.0f;
	mLineCount = mNumberOfSquares = 0;
	mCellsDown = mCellsAcross = 0;
	mOldCentreSquareX = mOldCentreSquareY = -1;
	mOldStartY = mOldEndY = mOldStartX = mOldEndX = -1;
	mIsShowing = false;
	for (int i = 0; i < MAXREDSQUARES; i++) {
		mSquares[mNumberOfSquares] = NULL;
	}
}

bool GridOverlay::init(SceneManager* sceneMgr, TerrainHandler* th, int cellsAcross, int cellsDown, float cellSize_m, const char* uniqueName, float r, float g, float b, float a) {
	printf("Initialising overlay grid %ix%i@%.2f\n", cellsAcross, cellsDown, cellSize_m);
	exit(1);
	if (!sceneMgr) {
		printf("ERROR: SceneManager NULL in GridOverlay::init()\n");
		return false;
	}
	if (!th) {
		printf("ERROR: TerrainHandler NULL in GridOverlay::init()\n");
		return false;
	}
	strcpy(mUniqueName, uniqueName);
	mpSceneManager = sceneMgr;
	mpTerrainHandler = th;
	mCellsDown = cellsDown;
	mCellsAcross = cellsAcross;
	mCellSize_m = cellSize_m;
	if (!mLineHandler.init(mpSceneManager, uniqueName)) {
		printf("ERROR: could not init line handler in GridOverlay::init()\n");
		return false;
	}
	for (int i = 0; i < mCellsAcross * 2 + 1; i++) {
		mLineHandler.constructLine(r, g, b, a);
		mLineCount++;
	}
	for (int i = 0; i < mCellsDown * 2 + 1; i++) {
		mLineHandler.constructLine(r, g, b, a);
		mLineCount++;
	}
	mMaterial = MaterialManager::getSingleton().create("redSquareMaterial", "Two");
	mMaterial->setLightingEnabled(false);
	mMaterial->getTechnique(0)->getPass(0)->setSceneBlending(SBT_TRANSPARENT_ALPHA);
	mMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(TVC_AMBIENT|TVC_DIFFUSE);
	mMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
	return true;
}

bool GridOverlay::update(const Vector3& startPosition, BrainGrids& bg) {
	int x = -1;
	int y = -1;
	if (!bg.calcGridPos(startPosition.x, startPosition.z, x, y)) {
		printf("ERROR: GridOverlay::update could not work out grid position for vehicle\n");
		return false;
	}
	if ((x != mOldCentreSquareX) || (y != mOldCentreSquareY)) { // redraw lines if req.
		mOldCentreSquareX = x;
		mOldCentreSquareY = y;
		mOldStartY = y - mCellsDown;
		if (mOldStartY < 0) {
			mOldStartY = 0;
		} else if (mOldStartY >= bg.mCellsDown) {
			mOldStartY = bg.mCellsDown - 1;
		}
		mOldEndY = y + mCellsDown;
		if (mOldEndY < 0) {
			mOldEndY = 0;
		} else if (mOldEndY >= bg.mCellsDown) {
			mOldEndY = bg.mCellsDown - 1;
		}		
		mOldStartX = x - mCellsAcross;
		if (mOldStartX < 0) {
			mOldStartX = 0;
		} else if (mOldStartX >= bg.mCellsAcross) {
			mOldStartX = bg.mCellsAcross - 1;
		}
		mOldEndX = x + mCellsAcross;
		if (mOldEndX < 0) {
			mOldEndX = 0;
		} else if (mOldEndX >= bg.mCellsAcross) {
			mOldEndX = bg.mCellsAcross - 1;
		}
		if (mIsShowing) {
			show(bg);
		}
	}
	int total = mNumberOfSquares;
	if (mNumberOfSquares >= MAXREDSQUARES) {
		total = MAXREDSQUARES;
	}
	for (int i = 0; i < total; i++) {
		mSquares[i]->clear();
	}
	for (int i = mOldStartX; i < mOldEndX; i++) {
		for (int j = mOldStartY + 1; j < mOldEndY + 1; j++) {
			if (bg.mGridCells[i][j].isObstructed()) {
				Vector3 pos;
				bg.calcRealPos(i, j, pos.x, pos.z);
				redSquare(pos);
			}
		}
	}
	return true;
}

bool GridOverlay::show(BrainGrids& bg) {
	mIsShowing = true;
	int currentLine = 0;
	for (int i = mOldStartX; i < mOldEndX + 1; i++) {
		mLineHandler.start3DLineStrip(currentLine);
		for (int j = mOldStartY; j < mOldEndY + 1; j++) {
			Vector3 position;
			if (!bg.calcRealPos(i, j, position.x, position.z)) {
				printf("ERROR: GridOverlay::show could not calc real pos\n");
				return false;
			}
			position.x -= mCellSize_m / 2.0f;
			position.z -= mCellSize_m / 2.0f;
			position.y = mpTerrainHandler->getTerrainHeightAt(position.x, position.z) + 1.0f; // move above ground 1 meter
			mLineHandler.addPointTo3DLineStrip(currentLine, position);
		}
		mLineHandler.end3DLineStrip(currentLine);
		currentLine++;
	}
	for (int i = mOldStartY; i < mOldEndY + 1; i++) {
		mLineHandler.start3DLineStrip(currentLine);
		for (int j = mOldStartX; j < mOldEndX + 1; j++) {
			Vector3 position;
			if (!bg.calcRealPos(j, i, position.x, position.z)) {
				printf("ERROR: GridOverlay::show could not calc real pos\n");
				return false;
			}
			position.x -= mCellSize_m / 2.0f;
			position.z -= mCellSize_m / 2.0f;
			position.y = mpTerrainHandler->getTerrainHeightAt(position.x, position.z) + 1.0f; // move above ground 1 meter
			mLineHandler.addPointTo3DLineStrip(currentLine, position);
		}
		mLineHandler.end3DLineStrip(currentLine);
		currentLine++;
	}
	return true;
}

void GridOverlay::hide() {
	mIsShowing = false;
	for (int i = 0; i < mLineCount; i++) {
		mLineHandler.hide3DLine(i);
	}
	int total = mNumberOfSquares;
	if (mNumberOfSquares >= MAXREDSQUARES) {
		total = MAXREDSQUARES;
	}
	for (int i = 0; i < total; i++) {
		mSquares[i]->clear();
	}
}

void GridOverlay::redSquare(Vector3& pos) {
	if (!mIsShowing) {
		return;
	}
	int square = mNumberOfSquares % MAXREDSQUARES; // loop around if used too many
	if (!mSquares[square]) {
		char tmp[128];
		sprintf(tmp, "%sgridSquare%i", mUniqueName, square);
		mSquares[square] = mpSceneManager->createManualObject(tmp);
		mSquares[square]->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1); // render just after overlays
		mpSceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(mSquares[square]);
	}
	mSquares[square]->clear();
	mSquares[square]->begin("redSquareMaterial", RenderOperation::OT_TRIANGLE_STRIP);
	pos.y = mpTerrainHandler->getTerrainHeightAt(pos.x - mCellSize_m / 2.0f, pos.z + mCellSize_m / 2.0f);
	mSquares[square]->position(pos.x - mCellSize_m / 2.0f, pos.y + 1.0f, pos.z + mCellSize_m / 2.0f);
	mSquares[square]->colour(1, 0, 0, 0.3);
	pos.y = mpTerrainHandler->getTerrainHeightAt(pos.x + mCellSize_m / 2.0f, pos.z + mCellSize_m / 2.0f);
  mSquares[square]->position(pos.x + mCellSize_m / 2.0f, pos.y + 1.0f, pos.z + mCellSize_m / 2.0f);
	mSquares[square]->colour(1, 0, 0, 0.3);
	pos.y = mpTerrainHandler->getTerrainHeightAt(pos.x - mCellSize_m / 2.0f, pos.z - mCellSize_m / 2.0f);
  mSquares[square]->position(pos.x - mCellSize_m / 2.0f, pos.y + 1.0f, pos.z - mCellSize_m / 2.0f);
	mSquares[square]->colour(1, 0, 0, 0.3);
	pos.y = mpTerrainHandler->getTerrainHeightAt(pos.x + mCellSize_m / 2.0f, pos.z - mCellSize_m / 2.0f);
  mSquares[square]->position(pos.x + mCellSize_m / 2.0f, pos.y + 1.0f, pos.z - mCellSize_m / 2.0f);
	mSquares[square]->colour(1, 0, 0, 0.3);
	mSquares[square]->end();
	mNumberOfSquares++;
}

bool GridOverlay::free() {
	printf("GridOverlay::free()\n");
	mLineHandler.free();
	for (int i = 0; i < mNumberOfSquares; i++) {
		char tmp[256];
		strcpy(tmp, "redSquareMaterial");
		MaterialManager::getSingleton().remove(tmp); // remove material
		//mpSceneManager->destroyManualObject(mSquares[i]); // remove object // maybe clearing the scene removes this anyway?
	}
	mIsShowing = false;
	mCellsAcross = mCellsDown = mLineCount = mNumberOfSquares = 0;
	mCellSize_m = 0.0f;
	return true;
}

