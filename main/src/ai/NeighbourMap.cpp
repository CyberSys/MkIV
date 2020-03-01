#include "NeighbourMap.h"
#include <stdio.h>
using namespace std;

NeighbourMap::NeighbourMap() {
	mCellSize = 10.0f;
	mCellsAcross = 0;
	mCellsDown = 0;
	mMinX = 0.0f;
	mMaxX = 0.0f;
	mMinZ = 0.0f;
	mMaxZ = 0.0f;
	mDepth = 0.0f;
	mWidth = 0.0f;
}

/** initialise the grids. and provide extents and cell size in the same units as extents. */
void NeighbourMap::init(float minx, float maxx, float minz, float maxz, float cellsize) {
	mMinX = minx;
	mMaxX = maxx;
	mMinZ = minz;
	mMaxZ = maxz;
	mCellSize = cellsize;

	// Work out depth and width of map
	mDepth = mMaxZ - mMinZ;
	if (mDepth < 0.0) {
		mDepth *= -1;
	}
	mWidth = mMaxX - mMinX;
	if (mWidth < 0.0) {
		mWidth *= -1;
	}

	// Work out number of cells
	mCellsAcross = (int)(mWidth / mCellSize);
	if ((mWidth / mCellSize) > (float)mCellsAcross) {
		mCellsAcross++;
	}
	mCellsDown = (int)(mDepth / mCellSize);
	if ((mDepth / mCellSize) > (float)mCellsDown) {
		mCellsDown++;
	}
	printf("NeighbourMap Initialised\n");
}

/** print the obstacle map to stdout */
void NeighbourMap::printObstacleMap() {
	printf("Printing Obstacle Map:\n");
	for (int y = 0; y < mCellsDown; y++) {
		for (int x = 0; x < mCellsAcross; x++) {
			if (mGridCells[x][y].mCountOfUnitsInCell > 0) {
				printf("u");
			} else if (mGridCells[x][y].mCountOfBuildingsInCell > 0) {
				printf("b");
			} else if (mGridCells[x][y].mCountOfShrubsInCell > 0) {
				printf("s");
			} else {
				printf(".");
			}
		}
		printf("\n");
	}
}

void NeighbourMap::clearDynamicObstacles() {
	for (int y = 0; y <= mCellsDown; y++) {
		for (int x = 0; x <= mCellsAcross; x++) {
			mGridCells[x][y].clearOfDynamicObstacles();
		}
	}
}

void NeighbourMap::clearStaticObstacles() {
	for (int y = 0; y <= mCellsDown; y++) {
		for (int x = 0; x <= mCellsAcross; x++) {
			mGridCells[x][y].clearOfStaticObstacles();
		}
	}
}

bool NeighbourMap::countObstaclesAroundCell(int x, int y, int radius) {
	if ((x < 0) || (y < 0) || (x >= mCellsAcross - 1) || (y >= mCellsDown - 1)) {
		printf("ERROR: NeighbourMap::countShrubsAroundCell(int, int, int) given invalid input cell (%i, %i)\n", x, y);
		return false;
	}
	mLastScanResults.reset();
	int lhs = x - radius;
	int rhs = x + radius;
	if (lhs < 0) {
		lhs = 0;
	}
	if (rhs >= mCellsAcross - 1) {
		rhs = mCellsAcross - 1;
	}
	int top = y - radius;
	int bot = y + radius;
	if (top < 0) {
		top = 0;
	}
	if (bot > mCellsDown - 1) {
		bot = mCellsDown - 1;
	}
	for (int i = lhs; i <= rhs; i++) {
		for (int j = top; j <= bot; j++) {
			int cellShrubCount = mGridCells[i][j].mCountOfShrubsInCell;
			for (int k = 0; k < cellShrubCount; k++) {
				mLastScanResults.mFetchedShrubIndices[mLastScanResults.mCountShrubNeighbours] = mGridCells[i][j].mIndicesOfShrubsInCell[k]; // actually record each one as well (TODO avoid double-ups??)
				mLastScanResults.mCountShrubNeighbours++;
			}
			int cellBuildingCount = mGridCells[i][j].mCountOfBuildingsInCell;
			for (int k = 0; k < cellBuildingCount; k++) {
				mLastScanResults.mFetchedBuildingIndices[mLastScanResults.mCountBuildingNeighbours] = mGridCells[i][j].mIndicesOfBuildingsInCell[k]; // actually record each one as well (TODO avoid double-ups??)
				mLastScanResults.mCountBuildingNeighbours++;
			}
			int cellUnitCount = mGridCells[i][j].mCountOfUnitsInCell;
			for (int k = 0; k < cellUnitCount; k++) {
				mLastScanResults.mFetchedUnitIndices[mLastScanResults.mCountUnitNeighbours] = mGridCells[i][j].mIndicesOfUnitsInCell[k]; // actually record each one as well (TODO avoid double-ups??)
				mLastScanResults.mCountUnitNeighbours++;
			}
		}
	}
	return true;
}

/** calculate the cell index of a given (x,y) position. returns false if not within map extents */
bool NeighbourMap::calcGridPos(float xpos, float zpos, int& cellacross, int& celldown) {
	float across = xpos; // Calculate 2d 'X' position
	across = across - mMinX; //	1. Subtract Minimum Map Bound
	across = across / mCellSize; //	2. Divide by Cellsize
	int indexA = (int)across; //	3. Truncate
	float down = zpos; // Calculate 3d 'Y' position NOTE: 3d Axis is reverse positive to 2d axis
	down = mMaxZ - down; // 1. Subtract actual 3d value FROM MAXIMUM Map Bound. This inverts our value
	down = down / mCellSize; //	2. Divide by cellsize
	int indexD = (int)down; //	3. truncate
	if (indexA >= MAXSPAN) { // Validiate
		printf("ERROR in NeighbourMap::calcGridPos(): IndexAcross >= MAXSPAN!: %i >= %i\n", indexA, MAXSPAN);
		return false;
	}
	if (indexD >= MAXSPAN) {
		printf("ERROR in NeighbourMap::calcGridPos(): IndexDown >= MAXSPAN!: %i >= %i\n", indexD, MAXSPAN);
		return false;
		}
	if (indexA < 0) {
		printf("ERROR in NeighbourMap::calcGridPos(): IndexAcross < 0!: %i < 0\n", indexA);
		return false;
	}
	if (indexD < 0) {
		printf("ERROR in NeighbourMap::calcGridPos(): IndexDown < 0!: %i < 0\n", indexD);
		return false;
	}
	cellacross = indexA;
	celldown = indexD;
	return true;
}

/** calculate the real (x,y) position from the center of a given grid index. returns false if not within map extents */
bool NeighbourMap::calcRealPos(int cellacross, int celldown, float &midx, float &midz) {
	float threedeeZ; // Calculate threedeeZ pos in middle of square 2dY
	threedeeZ = (float)celldown; //	1. cast as float
	threedeeZ = threedeeZ * mCellSize; //	2. Scale by cellsize
	threedeeZ = mMaxZ - threedeeZ; //	3. Relative to (threedeeMaxBound = 2d0)
	threedeeZ = threedeeZ - mCellSize / 2; //  4. Put in middle of square
	float threedeeX;
	threedeeX = (float)cellacross;
	threedeeX = threedeeX * mCellSize;
	threedeeX = mMinX + threedeeX;
	threedeeX = threedeeX + mCellSize / 2;
	midx = threedeeX;
	midz = threedeeZ;
	return true;
}

bool NeighbourMap::addShrub(float xmin, float zmin, float xmax, float zmax, int index) {
	int minCellX = -1;
	int minCellY = -1;
	int maxCellX = -1;
	int maxCellY = -1;
	if (!calcGridPos(xmin, zmin, minCellX, minCellY)) { // get grid cell min
		printf("ERROR: NeighbourMap::addShrub() - SHRUB %i AABB min position (%.2f, %.2f) not in valid grid location\n", index, xmin, zmin);
		return false;
	}
	if (!calcGridPos(xmax, zmax, maxCellX, maxCellY)) { // get grid cell min
		printf("ERROR: NeighbourMap::addShrub() - SHRUB %i AABB max position (%.2f, %.2f) not in valid grid location\n", index, xmax, zmax);
		return false;
	}
	for (int j = minCellX; j <= maxCellX; j++) {
		for (int k = maxCellY; k <= minCellY; k++) {
			if (!addShrubToCell(j, k, index)) {
				printf("ERROR: could not add shrub to cell in NeighbourMap::addShrub()\n");
				return false;
			}
		}
	}
	return true;
}

bool NeighbourMap::addBuilding(float xmin, float zmin, float xmax, float zmax, int index) {
	int minCellX = -1;
	int minCellY = -1;
	int maxCellX = -1;
	int maxCellY = -1;
	if (!calcGridPos(xmin, zmin, minCellX, minCellY)) { // get grid cell min
		printf("ERROR: NeighbourMap::addBuilding() - BUILDING %i AABB min position (%.2f, %.2f) not in valid grid location\n", index, xmin, zmin);
		return false;
	}
	if (!calcGridPos(xmax, zmax, maxCellX, maxCellY)) { // get grid cell min
		printf("ERROR: NeighbourMap::addBuilding() - BUILDING %i AABB max position (%.2f, %.2f) not in valid grid location\n", index, xmax, zmax);
		return false;
	}
	for (int j = minCellX; j <= maxCellX; j++) {
		for (int k = maxCellY; k <= minCellY; k++) {
			if (!addBuildingToCell(j, k, index)) {
				printf("ERROR: could not add building to cell in NeighbourMap::addBuilding()\n");
				return false;
			}
		}
	}
	return true;
}

bool NeighbourMap::addUnit(float xmin, float zmin, float xmax, float zmax, int index) {
	int minCellX = -1;
	int minCellY = -1;
	int maxCellX = -1;
	int maxCellY = -1;
	if (!calcGridPos(xmin, zmin, minCellX, minCellY)) { // get grid cell min
		printf("ERROR: NeighbourMap::addUnit() - UNIT %i AABB min position (%.2f, %.2f) not in valid grid location\n", index, xmin, zmin);
		return false;
	}
	if (!calcGridPos(xmax, zmax, maxCellX, maxCellY)) { // get grid cell min
		printf("ERROR: NeighbourMap::addUnit() - UNIT %i AABB max position (%.2f, %.2f) not in valid grid location\n", index, xmax, zmax);
		return false;
	}
	for (int j = minCellX; j <= maxCellX; j++) {
		for (int k = maxCellY; k <= minCellY; k++) {
			if (!addUnitToCell(j, k, index)) {
				printf("ERROR: could not add unit to cell in NeighbourMap::addUnit()\n");
				return false;
			}
		}
	}
	return true;
}

bool NeighbourMap::addShrubToCell(int x, int y, int index) {
	if ((x < 0) || (y < 0) || (x > mCellsAcross) || (y > mCellsDown)) {
		printf("ERROR: cell [%i][%i] given to NeighbourMap::addShrubToCell() is outside grid bounds (%i, %i).\n", x, y, mCellsAcross, mCellsDown);
		return false;
	}
	if (mGridCells[x][y].mCountOfShrubsInCell >= NG_MAX_SHURBS_IN_CELL) {
		printf("WARNING: max shrubs allowed in cell (%i) exceeded in cell [%i][%i]. In NeighbourMap::addShrubToCell()\n", MNG_AX_SHURBS_IN_CELL, x, y);
		return true; // don't bother recording index
	}
	mGridCells[x][y].mIndicesOfShrubsInCell[mGridCells[x][y].mCountOfShrubsInCell] = index;
	mGridCells[x][y].mCountOfShrubsInCell++;
	//printf("DB: added shrub to cell [%i][%i], count is now %i\n", x, y, mGridCells[x][y].mCountOfShrubsInCell);
	return true;
}

bool NeighbourMap::addBuildingToCell(int x, int y, int index) {
	if ((x < 0) || (y < 0) || (x > mCellsAcross) || (y > mCellsDown)) {
		printf("ERROR: cell [%i][%i] given to NeighbourMap::addBuildingToCell() is outside grid bounds (%i, %i).\n", x, y, mCellsAcross, mCellsDown);
		return false;
	}
	if (mGridCells[x][y].mCountOfBuildingsInCell >= NG_MAX_BUILDINGS_IN_CELL) {
		printf("WARNING: max buildings allowed in cell (%i) exceeded in cell [%i][%i]. In NeighbourMap::addBuildingToCell()\n", NG_MAX_BUILDINGS_IN_CELL, x, y);
		return true; // don't bother recording index
	}
	mGridCells[x][y].mIndicesOfBuildingsInCell[mGridCells[x][y].mCountOfBuildingsInCell] = index;
	mGridCells[x][y].mCountOfBuildingsInCell++;
	return true;
}

bool NeighbourMap::addUnitToCell(int x, int y, int index) {
	if ((x < 0) || (y < 0) || (x > mCellsAcross) || (y > mCellsDown)) {
		printf("ERROR: cell [%i][%i] given to NeighbourMap::addUnitToCell() is outside grid bounds (%i, %i).\n", x, y, mCellsAcross, mCellsDown);
		return false;
	}
	if (mGridCells[x][y].mCountOfUnitsInCell >= NG_MAX_UNITS_IN_CELL) {
		printf("WARNING: max units allowed in cell (%i) exceeded in cell [%i][%i]. In NeighbourMap::addUnitToCell()\n", NG_MAX_UNITS_IN_CELL, x, y);
		return true; // don't bother recording index
	}
	mGridCells[x][y].mIndicesOfUnitsInCell[mGridCells[x][y].mCountOfUnitsInCell] = index;
	mGridCells[x][y].mCountOfUnitsInCell++;
	return true;
}

NeighbourScanResults::NeighbourScanResults() {
	for (int i = 0; i < NG_MAX_NEIGHBOURS; i++) {
		mFetchedShrubIndices[i] = 0;
		mFetchedBuildingIndices[i] = 0;
		mFetchedUnitIndices[i] = 0;
	}
	mCountShrubNeighbours = 0;
	mCountBuildingNeighbours = 0;
	mCountUnitNeighbours = 0;
}

void NeighbourScanResults::reset() {
	mCountShrubNeighbours = 0;
	mCountBuildingNeighbours = 0;
	mCountUnitNeighbours = 0;
}

NeighbourMapCell::NeighbourMapCell() {
	for (int i = 0; i < NG_MAX_SHURBS_IN_CELL; i++) {
		mIndicesOfShrubsInCell[i] = -1;
	}
	for (int i = 0; i < NG_MAX_BUILDINGS_IN_CELL; i++) {
		mIndicesOfBuildingsInCell[i] = -1;
	}
	for (int i = 0; i < NG_MAX_UNITS_IN_CELL; i++) {
		mIndicesOfUnitsInCell[i] = -1;
	}
	mCountOfShrubsInCell = 0;
	mCountOfBuildingsInCell = 0;
	mCountOfUnitsInCell = 0;
}

void NeighbourMapCell::clearOfDynamicObstacles() {
	mCountOfUnitsInCell = 0;
}

void NeighbourMapCell::clearOfStaticObstacles() {
	mCountOfShrubsInCell = 0;
	mCountOfBuildingsInCell = 0;
}

bool NeighbourMapCell::isObstructed() {
	if ((mCountOfShrubsInCell > 0) || (mCountOfBuildingsInCell > 0) || (mCountOfUnitsInCell > 0)) {
		return true;
	}
	return false;
}
