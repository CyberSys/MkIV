/* Adaption of 'The Brain' program to provide
   artificial intelligence processing for agents

   Comprises a number of general functions and
   a core cBrain class

   Depends on BrainGrids.h for system implementation
   ANTON 2005 */


/*	Notes on A* Pathfinding:
	
	Using Manhattan Distance for 'H' heuristic is not (complete)
	because it OVERESTIMATES the distance to goal
	(because we can move diagonally in actual game) - therefor it will NOT
	always find the best path, and may work a bit oddly.
	However - it is fast to compute (MUCH MUCH MUCH
	faster than even a simplified version of Pythagoras Theorum)

	Thinking about limiting AI to a 'near-zone' of operation. long distance
	orders are worked out by flagging 'extend search when at last waypoint'
	with coordinates of final goal
	final waypoint to search for is then just the blank space in zone closest
	actual goal
*/

#ifndef _ANTON_E3_THEBRAIN_H_
#define _ANTON_E3_THEBRAIN_H_



/*
	Notes;

		A* Search cheats on last move -
		dosn't bother establishing quickest way to last square,
		just takes first route to goal when it is put in open list

		May result in slightly longer  routes, but still avoids
		obsticles, and cuts down computation

*/

// Reset a brain grid
void clearmap(char Map[MAXSPAN][MAXSPAN]);





// Finds first valid adjacent square to xs,zs
// returns 'num' of next square to check
// passes back co-ordinates xn and zn and a cost

// A* Path finding function. Avoids obsticles and grid boundaries

#endif
