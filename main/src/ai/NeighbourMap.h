/*
A map of obstacles in a low-res grid so that it's quick to find the nearest neighbour
Anton Gerdelan. First version 29 June 2009
*/

#ifndef _NEIGHBOURHOOD_GRID_H_
#define _NEIGHBOURHOOD_GRID_H_

#define NG_MAXSPAN 512
#define NG_MAX_SHURBS_IN_CELL 20
#define NG_MAX_BUILDINGS_IN_CELL 20
#define NG_MAX_UNITS_IN_CELL 15
#define NG_MAX_NEIGHBOURS 50

struct NeighbourMapScanResults {
	int mCountShrubNeighbours;
	int mCountBuildingNeighbours;
	int mCountUnitNeighbours;
	int mFetchedShrubIndices[NG_MAX_NEIGHBOURS];
	int mFetchedBuildingIndices[NG_MAX_NEIGHBOURS];
	int mFetchedUnitIndices[NG_MAX_NEIGHBOURS];
	NeighbourMapScanResults();
	void reset();
};

struct NeighbourMapCell {
	int mIndicesOfShrubsInCell[NG_MAX_SHURBS_IN_CELL];
	int mIndicesOfBuildingsInCell[NG_MAX_BUILDINGS_IN_CELL];
	int mIndicesOfUnitsInCell[NG_MAX_UNITS_IN_CELL];
	int mCountOfShrubsInCell;
	int mCountOfBuildingsInCell;
	int mCountOfUnitsInCell;
	NGGridCell();
	void clearOfDynamicObstacles();
	void clearOfStaticObstacles();
};

/** This class represents a high-level map and converts to-from map cells and real world co-ordinates */
class NeighbourMap {
public:
	NeighbourMap();
	void init(float minx, float maxx, float minz, float maxz, float cellsize); // initialise maps
	bool countObstaclesAroundCell(int x, int y, int radius = 1);
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

	NeighbourMapScanResults mLastScanResults;
	NeighbourMapCell mGridCells[NG_MAXSPAN][NG_MAXSPAN]; // a map of obstacles in each cell
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
