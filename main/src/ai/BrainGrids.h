/*
A class to simplify a given environment mesh into a set of simple grids for
AI planning
ANTON 2005
*/

#ifndef _BRAINGRIDS_ERROR_H_
#define _BRAINGRIDS_ERROR_H_

// Globals for Brain Grids
#define CELLSIZE 15.0f
#define MAXSPAN 512
#define MAX_WAYPOINTS 256 // Max waypoints to allow in path
#define MAX_ASTAR 1500	// Max nodes visting during A* (depth limiter)

#define MAX_SHURBS_IN_CELL 20
#define MAX_BUILDINGS_IN_CELL 20
#define MAX_UNITS_IN_CELL 15
#define MAX_NEIGHBOURS 20

// Node for use in A* Algorithm
struct ASNode {
	int X;
	int Z;

	int G;
	int H;
	int F;
	int ParentX;
	int ParentZ;
	ASNode *Parent;
	ASNode *Next;
};

struct GridCell {
	// extra variables for path planning
	float mTerrainHeight_m;
	float mSlopeToAdjacent[8]; // clockwise from 12 o'clock
	int mIndicesOfShrubsInCell[MAX_SHURBS_IN_CELL];
	int mIndicesOfBuildingsInCell[MAX_BUILDINGS_IN_CELL];
	int mIndicesOfUnitsInCell[MAX_UNITS_IN_CELL];
	int mCountOfShrubsInCell;
	int mCountOfBuildingsInCell;
	int mCountOfUnitsInCell;
	bool mIsUnderWater;
	GridCell();
	void clearOfDynamicObstacles();
	void clearOfStaticObstacles();
	bool isObstructed();
};

struct NeighbourScanResults {
	int mCountShrubNeighbours;
	int mCountBuildingNeighbours;
	int mCountUnitNeighbours;
	int mFetchedShrubIndices[MAX_NEIGHBOURS];
	int mFetchedBuildingIndices[MAX_NEIGHBOURS];
	int mFetchedUnitIndices[MAX_NEIGHBOURS];
	NeighbourScanResults();
	void reset();
};

/** This class represents a high-level map and converts to-from map cells and real world co-ordinates */
class BrainGrids {
public:
	BrainGrids();
	void init(float minx, float maxx, float minz, float maxz, float cellsize); // initialise maps
	int ManhattanDistance(int xi, int zi, int xf, int zf); // Manhattan heuristic distance between 2 squares
	int NextAdjacent(int num, int MapXBound, int MapYBound,int xs, int zs, int &xn, int &zn, int &cost);
	bool countObstaclesAroundPos(float x, float z, float radius_m);
	bool countObstaclesAroundCell(int x, int y, int radius);
	bool FindPath(int xi, int zi, int xf, int zf, int MapXBound, int MapYBound, int xwps[], int zwps[], int &num);
	bool calcGridPos(float xpos, float zpos, int& cellacross, int& celldown); // work out map index for a real position
	bool calcRealPos(int cellacross, int celldown, float &midx, float &midz); // work out real position for a map index
	bool addShrub(float xmin, float zmin, float xmax, float zmax, int index);
	bool addBuilding(float xmin, float zmin, float xmax, float zmax, int index);
	bool addUnit(float xmin, float zmin, float xmax, float zmax, int index);
	bool addShrubToCell(int x, int y, int index);
	bool addBuildingToCell(int x, int y, int index);
	bool addUnitToCell(int x, int y, int index);
	void printObstacleMap(); // print maps to stdout
	void clearDynamicObstacles(); // clear obstacle map
	void clearStaticObstacles(); // clear obstacle map

	NeighbourScanResults mLastScanResults;
	GridCell mGridCells[MAXSPAN][MAXSPAN]; // a map of obstacles in each cell
	float mCellSize; // width of cells in real world distance i.e. map scale
	int mCellsAcross; // number of cells representing x-axis
	int mCellsDown; // number of cells representing z-axis
	float mMinX; // minimum x-axis real-world extent of map
	float mMaxX; // maximum x-axis real-world extent of map
	float mMinZ; // minimum z-axis real-world extent of map
	float mMaxZ; // maximum z-axis real-world extent of map
	float mDepth; // z-axis coverage of map
	float mWidth; // x-axis coverage of map
};
#endif
