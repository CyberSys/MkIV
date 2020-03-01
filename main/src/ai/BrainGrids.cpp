#include "BrainGrids.h"
#include <stdio.h>

using namespace std;

BrainGrids::BrainGrids() {
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
void BrainGrids::init(float minx, float maxx, float minz, float maxz, float cellsize) {
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
	printf("Brain Grids Initialised\n");
}

/** print the obstacle map to stdout */
void BrainGrids::printObstacleMap() {
	printf("Printing Obsticle Map:\n");
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

void BrainGrids::clearDynamicObstacles() {
	for (int y = 0; y <= mCellsDown; y++) {
		for (int x = 0; x <= mCellsAcross; x++) {
			mGridCells[x][y].clearOfDynamicObstacles();
		}
	}
}

void BrainGrids::clearStaticObstacles() {
	for (int y = 0; y <= mCellsDown; y++) {
		for (int x = 0; x <= mCellsAcross; x++) {
			mGridCells[x][y].clearOfStaticObstacles();
		}
	}
}

bool BrainGrids::countObstaclesAroundPos(float x, float z, float radius_m) {
	int a = 0;
	int d = 0;
	int cellradius = (int)(radius_m / mCellSize) + 1;
	//printf("cellradius for scan is %i\n", cellradius);
	if (!calcGridPos(x, z, a, d)) {
		printf("ERROR: could not calc grid pos in BrainGrids::countObstaclesAroundPos()\n");
		return false;
	}
	if (!countObstaclesAroundCell(a, d, cellradius)) {
		printf("ERROR: could not scan cells in BrainGrids::countObstaclesAroundPos()\n");
		return false;
	}
	return true;
}

bool BrainGrids::countObstaclesAroundCell(int x, int y, int radius) {
	if ((x < 0) || (y < 0) || (x >= mCellsAcross - 1) || (y >= mCellsDown - 1)) {
		printf("ERROR: BrainGrids::countShrubsAroundCell(int, int, int) given invalid input cell (%i, %i)\n", x, y);
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
bool BrainGrids::calcGridPos(float xpos, float zpos, int& cellacross, int& celldown) {
	// Calculate 2d 'X' position
	float across = xpos;
	//	1. Subtract Minimum Map Bound
	across = across - mMinX;
	//	2. Divide by Cellsize
	across = across / mCellSize;
	//	3. Truncate
	int indexA = (int)across;
	// Calculate 3d 'Y' position NOTE: 3d Axis is reverse positive to 2d axis
	float down = zpos;
	//	1. Subtract actual 3d value FROM MAXIMUM Map Bound
	down = mMaxZ - down;	// This inverts our value
	//	2. Divide by cellsize
	down = down / mCellSize;
	//	3. truncate
	int indexD = (int)down;

	// Validiate
	if (indexA >= MAXSPAN) {
		printf("ERROR in BrainGrids::calcGridPos(): IndexAcross >= MAXSPAN!: %i >= %i\n", indexA, MAXSPAN);
		return false;
	}
	if (indexD >= MAXSPAN) {
		printf("ERROR in BrainGrids::calcGridPos(): IndexDown >= MAXSPAN!: %i >= %i\n", indexD, MAXSPAN);
		return false;
		}
	if (indexA < 0) {
		printf("ERROR in BrainGrids::calcGridPos(): IndexAcross < 0!: %i < 0\n", indexA);
		return false;
	}
	if (indexD < 0) {
		printf("ERROR in BrainGrids::calcGridPos(): IndexDown < 0!: %i < 0\n", indexD);
		return false;
	}

	cellacross = indexA;
	celldown = indexD;

	return true;
}

/** calculate the real (x,y) position from the center of a given grid index. returns false if not within map extents */
bool BrainGrids::calcRealPos(int cellacross, int celldown, float &midx, float &midz) {
	// Calculate threedeeZ pos in middle of square 2dY
	float threedeeZ;
	//	1. cast as float
	threedeeZ = (float)celldown;
	//	2. Scale by cellsize
	threedeeZ = threedeeZ * mCellSize;
	//	3. Relative to (threedeeMaxBound = 2d0)
	threedeeZ = mMaxZ - threedeeZ;
	//  4. Put in middle of square
	threedeeZ = threedeeZ - mCellSize / 2;

	float threedeeX;
	threedeeX = (float)cellacross;
	threedeeX = threedeeX * mCellSize;
	threedeeX = mMinX + threedeeX;
	threedeeX = threedeeX + mCellSize / 2;

	// **Outstanding problem with overlapping. Centre of edge squares not on map
	/*
		if (threedeeX > MaxX) { printf("ERROR in BRAIN: CellAcross > Max X!"); return false; }
		if (threedeeZ > MaxZ) { printf("ERROR in BRAIN: CellDown > Max Z!"); return false; }
		if (threedeeX < MinX) { printf("ERROR in BRAIN: CellAcross < Min X!"); return false; }
		if (threedeeZ < MinZ) { printf("ERROR in BRAIN: CellDown < Min Z!"); return false; }
	*/

	midx = threedeeX;
	midz = threedeeZ;

	return true;
}

bool BrainGrids::addShrub(float xmin, float zmin, float xmax, float zmax, int index) {
	int minCellX = -1;
	int minCellY = -1;
	int maxCellX = -1;
	int maxCellY = -1;
	if (!calcGridPos(xmin, zmin, minCellX, minCellY)) { // get grid cell min
		printf("ERROR: BrainGrids::addShrub() - SHRUB %i AABB min position (%.2f, %.2f) not in valid grid location\n", index, xmin, zmin);
		return false;
	}
	if (!calcGridPos(xmax, zmax, maxCellX, maxCellY)) { // get grid cell min
		printf("ERROR: BrainGrids::addShrub() - SHRUB %i AABB max position (%.2f, %.2f) not in valid grid location\n", index, xmax, zmax);
		return false;
	}
	for (int j = minCellX; j <= maxCellX; j++) {
		for (int k = maxCellY; k <= minCellY; k++) {
			if (!addShrubToCell(j, k, index)) {
				printf("ERROR: could not add shrub to cell in BrainGrids::addShrub()\n");
				return false;
			}
		}
	}
	return true;
}

bool BrainGrids::addBuilding(float xmin, float zmin, float xmax, float zmax, int index) {
	int minCellX = -1;
	int minCellY = -1;
	int maxCellX = -1;
	int maxCellY = -1;
	if (!calcGridPos(xmin, zmin, minCellX, minCellY)) { // get grid cell min
		printf("ERROR: BrainGrids::addBuilding() - BUILDING %i AABB min position (%.2f, %.2f) not in valid grid location\n", index, xmin, zmin);
		return false;
	}
	if (!calcGridPos(xmax, zmax, maxCellX, maxCellY)) { // get grid cell min
		printf("ERROR: BrainGrids::addBuilding() - BUILDING %i AABB max position (%.2f, %.2f) not in valid grid location\n", index, xmax, zmax);
		return false;
	}
	for (int j = minCellX; j <= maxCellX; j++) {
		for (int k = maxCellY; k <= minCellY; k++) {
			if (!addBuildingToCell(j, k, index)) {
				printf("ERROR: could not add building to cell in BrainGrids::addBuilding()\n");
				return false;
			}
		}
	}
	return true;
}

bool BrainGrids::addUnit(float xmin, float zmin, float xmax, float zmax, int index) {
	int minCellX = -1;
	int minCellY = -1;
	int maxCellX = -1;
	int maxCellY = -1;
	if (!calcGridPos(xmin, zmin, minCellX, minCellY)) { // get grid cell min
		printf("ERROR: BrainGrids::addUnit() - UNIT %i AABB min position (%.2f, %.2f) not in valid grid location\n", index, xmin, zmin);
		return false;
	}
	if (!calcGridPos(xmax, zmax, maxCellX, maxCellY)) { // get grid cell min
		printf("ERROR: BrainGrids::addUnit() - UNIT %i AABB max position (%.2f, %.2f) not in valid grid location\n", index, xmax, zmax);
		return false;
	}
	for (int j = minCellX; j <= maxCellX; j++) {
		for (int k = maxCellY; k <= minCellY; k++) {
			if (!addUnitToCell(j, k, index)) {
				printf("ERROR: could not add unit to cell in BrainGrids::addUnit()\n");
				return false;
			}
		}
	}
	return true;
}

bool BrainGrids::addShrubToCell(int x, int y, int index) {
	if ((x < 0) || (y < 0) || (x > mCellsAcross) || (y > mCellsDown)) {
		printf("ERROR: cell [%i][%i] given to BrainGrids::addShrubToCell() is outside grid bounds (%i, %i).\n", x, y, mCellsAcross, mCellsDown);
		return false;
	}
	if (mGridCells[x][y].mCountOfShrubsInCell >= MAX_SHURBS_IN_CELL) {
		printf("WARNING: max shrubs allowed in cell (%i) exceeded in cell [%i][%i]. In BrainGrids::addShrubToCell()\n", MAX_SHURBS_IN_CELL, x, y);
		return true; // don't bother recording index
	}
	mGridCells[x][y].mIndicesOfShrubsInCell[mGridCells[x][y].mCountOfShrubsInCell] = index;
	mGridCells[x][y].mCountOfShrubsInCell++;
	//printf("DB: added shrub to cell [%i][%i], count is now %i\n", x, y, mGridCells[x][y].mCountOfShrubsInCell);
	return true;
}

bool BrainGrids::addBuildingToCell(int x, int y, int index) {
	if ((x < 0) || (y < 0) || (x > mCellsAcross) || (y > mCellsDown)) {
		printf("ERROR: cell [%i][%i] given to BrainGrids::addBuildingToCell() is outside grid bounds (%i, %i).\n", x, y, mCellsAcross, mCellsDown);
		return false;
	}
	if (mGridCells[x][y].mCountOfBuildingsInCell >= MAX_BUILDINGS_IN_CELL) {
		printf("WARNING: max buildings allowed in cell (%i) exceeded in cell [%i][%i]. In BrainGrids::addBuildingToCell()\n", MAX_BUILDINGS_IN_CELL, x, y);
		return true; // don't bother recording index
	}
	mGridCells[x][y].mIndicesOfBuildingsInCell[mGridCells[x][y].mCountOfBuildingsInCell] = index;
	mGridCells[x][y].mCountOfBuildingsInCell++;
	return true;
}

bool BrainGrids::addUnitToCell(int x, int y, int index) {
	if ((x < 0) || (y < 0) || (x > mCellsAcross) || (y > mCellsDown)) {
		printf("ERROR: cell [%i][%i] given to BrainGrids::addUnitToCell() is outside grid bounds (%i, %i).\n", x, y, mCellsAcross, mCellsDown);
		return false;
	}
	if (mGridCells[x][y].mCountOfUnitsInCell >= MAX_UNITS_IN_CELL) {
		printf("WARNING: max units allowed in cell (%i) exceeded in cell [%i][%i]. In BrainGrids::addUnitToCell()\n", MAX_UNITS_IN_CELL, x, y);
		return true; // don't bother recording index
	}
	mGridCells[x][y].mIndicesOfUnitsInCell[mGridCells[x][y].mCountOfUnitsInCell] = index;
	mGridCells[x][y].mCountOfUnitsInCell++;
	return true;
}

// Manhattan heuristic distance between 2 squares
int BrainGrids::ManhattanDistance(int xi, int zi, int xf,int zf) {
	int xdif = xi-xf;
	if (xdif < 0) xdif *= -1;
	int zdif = zi-zf;
	if (zdif < 0) zdif *= -1;
	return xdif + zdif;
}

// checks all adjacent 8 grid cells to see if free, starting from 'num'. returns number of first free cell checked.
// must supply map bounds and also current location as a grid reference. passes back grid ref of free cell and also heuristic cost to this cell
int BrainGrids::NextAdjacent(int num, int MapXBound, int MapYBound, int xs, int zs, int& xn, int& zn, int& cost) {
	xn = zn = -1;
	if (num == 0) {
		// check above
		if (xs - 1 >= 0) {
			// check for obstacles
			if (!mGridCells[xs - 1][zs].isObstructed()) {
				xn = xs - 1;
				zn = zs;
				cost = 10;
				return 1;
			}
		}
		num = 1;
	}
	if (num == 1) {
		// above right
		if ((xs - 1 >= 0) && (zs + 1 < MapYBound)) {
			// check for obstacles
			if (!mGridCells[xs - 1][zs + 1].isObstructed()) {
				xn = xs - 1;
				zn = zs + 1;
				cost = 14;
				return 2;
			}
		}
		num = 2;
	}
	if (num == 2) {
		// right
		if (zs + 1 < MapYBound) {
			// check for obsticles
			if (!mGridCells[xs][zs+1].isObstructed()) {
				xn = xs;
				zn = zs + 1;
				cost = 10;
				return 3;
			}
		}
		num = 3;
	}
	if (num == 3) {
		// below right
		if ((xs + 1 < MapXBound) && (zs + 1 < MapYBound)) {
			// check for obsticles
			if (!mGridCells[xs+1][zs+1].isObstructed()) {
				xn = xs + 1;
				zn = zs + 1;
				cost = 14;
				return 4;
			}

		}
		num = 4;
	}
	if (num == 4) {
		// below
		if (xs + 1 < MapXBound) {
			// check for obsticles
			if (!mGridCells[xs+1][zs].isObstructed()) {
				xn = xs + 1;
				zn = zs;
				cost = 10;
				return 5;
			}
		}
		num = 5;

	}
	if (num == 5) {
		// below left
		if ((xs + 1 < MapXBound) && (zs - 1 >= 0)) {
			// check for obsticles
			if (!mGridCells[xs+1][zs-1].isObstructed()) {
				xn = xs + 1;
				zn = zs - 1;
				cost = 14;
				return 6;
			}
		}
		num = 6;
	}
	if (num == 6) {
		// left
		if (zs - 1 >= 0) {
			// check for obsticles
			if (!mGridCells[xs][zs-1].isObstructed()) {
				xn = xs;
				zn = zs - 1;
				cost = 10;
				return 7;
			}
		}
		num = 7;
	}
	if (num == 7) {
		// above left
			if ((xs - 1 >= 0) && (zs - 1 >= 0)) {
			// check for obsticles
			if (!mGridCells[xs - 1][zs - 1].isObstructed()) {
				xn = xs - 1;
				zn = zs - 1;
				cost = 14;
				return 8; // should be -1??
			}
		}
		num = 8;

	}
	return -1;
}

bool BrainGrids::FindPath(int xi, int zi, int xf, int zf,int MapXBound, int MapYBound, int xwps[], int zwps[], int& num) {
	if ((xi < 0) || (zi < 0) || (xf >= MapXBound) || (zf >= MapYBound)) { // Make sure target is actually on map board
		return false;
	}
	if (mGridCells[xf][zf].isObstructed()) { // Make sure destination is not a tree or something
		int xn, zn, cost;
		num = NextAdjacent(0, MapXBound, MapYBound, xf, zf, xn, zn, cost);
		if (num == -1) { // check if blocked all around the target as well
			//printf("DEBUG: ERROR: could not find next adjacent\n");
			return false;
		}
		xf = xn;
		zf = zn;
	}
	int k = 0; // Counter to depth-limit A*
	ASNode *OpenList = NULL;
	ASNode *ClosedList = NULL;
	bool AddedTarget = false;
	ASNode *Final = NULL;

	// 1.Add start node to open list
	OpenList = new ASNode;
	OpenList->X = xi;
	OpenList->Z = zi;
	OpenList->G = 0;
	OpenList->H = ManhattanDistance(xi,zi,xf,zf);
	OpenList->F = OpenList->H + OpenList->G;
	OpenList->ParentX = -1;
	OpenList->ParentZ = -1;
	OpenList->Parent = NULL;
	OpenList->Next = NULL;

	// 2. Repeating Loop
	while (true) {
		// There is no possible path = fail
		if (OpenList == NULL) {
			// free memory (added 2006)
			ASNode *cursor = OpenList;
			ASNode *term = NULL;
			while (cursor != NULL) {
				term = cursor;
				cursor = cursor->Next;
				delete term;
			}
			cursor = ClosedList;
			while (cursor != NULL) {
				term = cursor;
				cursor = cursor->Next;
				delete term;
			}
			//printf("DEBUG: ERROR: there is no possible path\n");
			return false;
		}

		ASNode *Current = OpenList;
		ASNode *Ptr = OpenList;

		// a) Find Lowest F-Cost Node
		while (Ptr != NULL) {
			if (Ptr->F < Current->F) {
				Current = Ptr;
			}
			Ptr = Ptr->Next;
		}

		// b) Remove from open list
		if (OpenList == Current) {
			OpenList = Current->Next;
		} else {
			Ptr = OpenList;
			while (Ptr != NULL) {
				if (Ptr->Next == Current) {
					Ptr->Next = Current->Next;
					break;
				}
				Ptr = Ptr->Next;
			}
		}

		// b) and Add to closed list
		Current->Next = ClosedList;
		ClosedList = Current;

		// c) for all adjacent squares...
		int i = 0;
		while (true) {
			// escape hatch to save CPU
			if (k > MAX_ASTAR) {
				// a) Find Lowest H-Cost Node (to add as the CARROT - dummy target node)
				ASNode *Current = ClosedList;
				ASNode *Ptr = ClosedList;
				while (Ptr != NULL) {
					if (Ptr->H < Current->H) {
						Current = Ptr;
					}
					Ptr = Ptr->Next;
				}

				AddedTarget = true;
				Final = Current;
				break;
			}
			// This is a 'visit' increment k counter here
			k++;

			int xnext, znext;
			int cost;
			i = NextAdjacent(i, MapXBound, MapYBound, Current->X, Current->Z, xnext, znext, cost);
			if (i == -1) break; // We have found all adjacent squares

			// check if in closed list
			ASNode *Tmp = ClosedList;
			bool inClosed = false;
			while (Tmp != NULL) {
				//printf("Checking Closed List\n");
				if ((Tmp->X == xnext) && (Tmp->Z == znext)) {
					//printf("In Closed\n");
					inClosed = true;
					break;
				}
				Tmp = Tmp->Next;
			}
			if (inClosed) continue;

			// check if in open list
			Tmp = OpenList;
			bool inOpen = false;
			while (Tmp != NULL) {
				if ((Tmp->X == xnext) && (Tmp->Z == znext)) {
					inOpen = true;
					break;
				}
				Tmp = Tmp->Next;
			}
			if (inOpen == false) {
				// add to open list
				ASNode *AdjNode = new ASNode;
				AdjNode->X = xnext;
				AdjNode->Z = znext;
				AdjNode->G = cost + Current->G;
				AdjNode->H = ManhattanDistance(xnext,znext,xf,zf);
				AdjNode->F = AdjNode->G + AdjNode->H;
				AdjNode->ParentX = Current->X;
				AdjNode->ParentZ = Current->Z;
				AdjNode->Parent = Current;
				AdjNode->Next = OpenList;
				OpenList = AdjNode;

				// check if this is target node
				if ((AdjNode->X == xf) && (AdjNode->Z == zf)) {
					AddedTarget = true;
					Final = AdjNode;
					break;
				}
			} else {
				// update path to this node
				int G = cost + Current->G;
				if (G < Tmp->G) {
					Tmp->ParentX = Current->X;
					Tmp->ParentZ = Current->Z;
					Tmp->Parent = Current;
					Tmp->G = G;
					Tmp->F = Tmp->G + Tmp->H;
				}
			}
		}
		if (AddedTarget == true) {break; }
	}

	int reversedx[MAX_WAYPOINTS];
	int reversedz[MAX_WAYPOINTS];
	int count = 0;
	while (Final != NULL) {
		reversedx[count] = Final->X;
		reversedz[count] = Final->Z;
		count++;
		Final = Final->Parent;
	}
	// reverse
	int j = count - 1;
	for (int i = 0; i < count; i++) {
		xwps[i] = reversedx[j];
		zwps[i] = reversedz[j];

		//mObstacleMap[xwps[i]][zwps[i]] = i + 48; // TEMP draw this waypoto screen
		//printObstacleMap(); // TEMP draw this waypoto screen

		j--;
	}
	num = count;

	// free memory (added 2006)
	ASNode *cursor = OpenList;
	ASNode *term = NULL;
	while (cursor != NULL) {
		term = cursor;
		cursor = cursor->Next;
		delete term;

	}
	cursor = ClosedList;
	while (cursor != NULL) {
		term = cursor;
		cursor = cursor->Next;
		delete term;

	}

	//printf("DEBUG: found path\n");
	return true;
}

NeighbourScanResults::NeighbourScanResults() {
	for (int i = 0; i < MAX_NEIGHBOURS; i++) {
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

GridCell::GridCell() {
	mTerrainHeight_m = 0.0f;
	for (int i = 0; i < 8; i++) {
		mSlopeToAdjacent[i] = 0.0f;
	}
	for (int i = 0; i < MAX_SHURBS_IN_CELL; i++) {
		mIndicesOfShrubsInCell[i] = -1;
	}
	for (int i = 0; i < MAX_BUILDINGS_IN_CELL; i++) {
		mIndicesOfBuildingsInCell[i] = -1;
	}
	for (int i = 0; i < MAX_UNITS_IN_CELL; i++) {
		mIndicesOfUnitsInCell[i] = -1;
	}
	mCountOfShrubsInCell = 0;
	mCountOfBuildingsInCell = 0;
	mCountOfUnitsInCell = 0;
	mIsUnderWater = false;
}

void GridCell::clearOfDynamicObstacles() {
	mCountOfUnitsInCell = 0;
}

void GridCell::clearOfStaticObstacles() {
	mCountOfShrubsInCell = 0;
	mCountOfBuildingsInCell = 0;
}

bool GridCell::isObstructed() {
	if ((mCountOfShrubsInCell > 0) || (mCountOfBuildingsInCell > 0) || (mCountOfUnitsInCell > 0)) {
		return true;
	}
	return false;
}

