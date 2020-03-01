#include "Core_Global.h"	// Globals used throughout program
#include "AStar.h"

// Reset a brain grid
void clearmap(char Map[MAXSPAN][MAXSPAN]) {
	for (int i = 0; i < MAXSPAN; i++) {
		for (int j = 0; j < MAXSPAN; j++) {
			Map[i][j] = '.';
		}
	}
}

// Manhattan heuristic distance between 2 squares
int ManhattanDistance(int xi,int zi,int xf,int zf) {
	int xdif = xi-xf;
	if (xdif < 0) xdif *= -1;
	int zdif = zi-zf;
	if (zdif < 0) zdif *= -1;
	return xdif + zdif;
}

// Finds first valid adjacent square to xs,zs
// returns 'num' of next square to check
// passes back co-ordinates xn and zn and a cost
int NextAdjacent(int num, int MapXBound, int MapYBound,int xs, int zs, int &xn, int &zn, int &cost) {
	xn = -1; zn = -1;
	if (num == 0) {
		// check above
		if (xs - 1 >= 0) {
			// check for obsticles
			if (ObstacleMap[xs-1][zs] == '.') {
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
			// check for obsticles
			if (ObstacleMap[xs-1][zs+1] == '.') {
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
			if (ObstacleMap[xs][zs+1] == '.') {
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
			if (ObstacleMap[xs+1][zs+1] == '.') {
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
			if (ObstacleMap[xs+1][zs] == '.') {
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
			if (ObstacleMap[xs+1][zs-1] == '.') {
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
			if (ObstacleMap[xs][zs-1] == '.') {
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
			if (ObstacleMap[xs-1][zs-1] == '.') {
				xn = xs - 1;
				zn = zs - 1;
				cost = 14;
				return 8;			// should be -1??
			}
		}
		num = 8;

	}
	return -1;
}

// A* Path finding function. Avoids obsticles and grid boundaries
bool FindPath(int xi,int zi,int xf,int zf,int MapXBound,int MapYBound,int xwps[],int zwps[],int &num) {
	// Make sure target is actually on map board
	if ((xi < 0) || (zi < 0) || (xf >= MapXBound) || (zf >= MapYBound)) { return false; }
	// Make sure destination is not a tree or something
	if (ObstacleMap[xf][zf] != '.') {
		int xn,zn,cost;
		int num = NextAdjacent(0,MapXBound,MapYBound,xf,zf,xn,zn,cost);
		if (num == -1) { return false; } // blocked all around the target aswell
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
			return false;
		}

		ASNode *Current = OpenList;
		ASNode *Ptr = OpenList;

		// a) Find Lowest F-Cost Node
		while (Ptr != NULL) {
			if (Ptr->F < Current->F) {
				Current = Ptr;
			}
			Ptr=Ptr->Next;
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
					Ptr=Ptr->Next;
				}

				AddedTarget = true;
				Final = Current;
				break;
			}
			// This is a 'visit' increment k counter here
			k++;

			int xnext,znext;
			int cost;
			i = NextAdjacent(i,MapXBound,MapYBound,Current->X,Current->Z,xnext,znext,cost);
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

	int reversedx[MAXWPS];
	int reversedz[MAXWPS];
	int count = 0;
	while (Final != NULL) {
		reversedx[count] = Final->X;
		reversedz[count] = Final->Z;
		count++;
		Final = Final->Parent;
	}
	// reverse
	int j = count-1;
	for (int i = 0; i < count; i++) {
		xwps[i] = reversedx[j];
		zwps[i] = reversedz[j];
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

	return true;
}

