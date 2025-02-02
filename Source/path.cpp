/**
 * @file path.cpp
 *
 * Implementation of the path finding algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

// #define DEBUG_PATH

/** Pre-allocated memory to store nodes used by the path finding algorithm. */
PATHNODE path_nodes[MAXPATHNODES];
/** The index of last used node in path_nodes. */
static unsigned gnLastNodeIdx;
/** Container for reconstructing the path after the A* search is done. */
static int8_t reversePathDirs[MAX_PATH_LENGTH];
/** A linked list of all visited nodes. */
static PATHNODE* pathVisitedNodes;
#ifdef DEBUG_PATH
/** A stack for recursively update nodes. */
static PATHNODE* pathUpdateStack[MAXPATHNODES];
#endif
/** A linked list of the A* frontier, sorted by distance. */
static PATHNODE* pathFrontNodes;
/** The target location. */
static int gnTx, gnTy;

/** For iterating over the 8 possible movement directions. */
//                       PDIR_N   W   E   S  NW  NE  SE  SW
const int8_t pathxdir[8] = { -1, -1,  1,  1, -1,  0,  1,  0 };
const int8_t pathydir[8] = { -1,  1, -1,  1,  0, -1,  0,  1 };
#ifdef DEBUG_PATH
static const int8_t stepcost[8] = { 3, 3, 3, 3, 2, 2, 2, 2 };
/** The target location. */
static int gnSx, gnSy;
#endif
/** Maps from facing direction to path-direction. */
const BYTE dir2pdir[NUM_DIRS] = { PDIR_S, PDIR_SW, PDIR_W, PDIR_NW, PDIR_N, PDIR_NE, PDIR_E, PDIR_SE };

/**
 * each step direction is assigned a number like this:
 *       dx
 *     -1 0 1
 *     +-----
 *   -1|5 1 6
 * dy 0|2 0 3
 *    1|8 4 7
 */
//int8_t path_directions[9] = { WALK_N, WALK_NE, WALK_E, WALK_NW, 0, WALK_SE, WALK_W, WALK_SW, WALK_S };
int8_t path_directions[9] = { DIR_N, DIR_NE, DIR_E, DIR_NW, DIR_NONE, DIR_SE, DIR_W, DIR_SW, DIR_S };

/**
 * @brief return a node for (dx,dy) on the frontier, or NULL if not found
 */
static PATHNODE* PathFrontNodeAt(int dx, int dy)
{
	PATHNODE* result = pathFrontNodes->NextNode;

	while (result != NULL) {
		if (result->x == dx && result->y == dy)
			break;
		result = result->NextNode;
	}
	return result;
}

/**
 * @brief return a node for (dx,dy) if it was visited, or NULL if not found
 */
static PATHNODE* PathVisitedNodeAt(int dx, int dy)
{
	PATHNODE* result = pathVisitedNodes;

	while (true) {
		if (result->x == dx && result->y == dy)
			break;
		result = result->NextNode;
		if (result == NULL)
			break;
	}
	return result;
}

/**
 * @brief insert pPath into the frontier (keeping the frontier sorted by total distance)
 */
static void PathAddNode(PATHNODE* pPath)
{
	PATHNODE *next, *current;

	current = pathFrontNodes;
	next = pathFrontNodes->NextNode;
	pPath->NextNode = next;
	current->NextNode = pPath;
}
#ifdef DEBUG_PATH
/**
 * @brief return 2 if (sx,sy) is horizontally/vertically aligned with (dx,dy), else 3
 *
 * This approximates that diagonal movement on a square grid should have a cost
 * of sqrt(2). That's approximately 1.5, so they multiply all step costs by 2,
 * except diagonal steps which are times 3
 */
static inline int PathStepCost(int sx, int sy, int dx, int dy)
{
	return (sx == dx || sy == dy) ? 2 : 3;
}

/**
 * @brief update all path costs using depth-first search starting at pPath
 */
static void PathUpdateCosts(PATHNODE* pPath)
{
	PATHNODE* PathOld;
	PATHNODE* PathAct;
	int updateStackSize;
	int i, newStepCost, newWalkCost;

	pathUpdateStack[0] = pPath;
	updateStackSize = 1;
	do {
		updateStackSize--;
		PathOld = pathUpdateStack[updateStackSize];
		for (i = 0; i < lengthof(PathOld->Child); i++) {
			PathAct = PathOld->Child[i];
			if (PathAct == NULL)
				break;

			newStepCost = PathStepCost(PathOld->x, PathOld->y, PathAct->x, PathAct->y);
			newWalkCost = PathOld->walkCost + newStepCost;
			if (newWalkCost < PathAct->walkCost /*&& PathWalkable(PathOld->x, PathOld->y, PathAct->x, PathAct->y)*/) {
				EventPlrMsg("Update walk 0 %d:%d cost%d:%d last%d to cost%d:%d", PathAct->x - gnSx, PathAct->y - gnSy, PathAct->totalCost, PathAct->walkCost, PathAct->lastStepCost, newWalkCost + PathAct->remainingCost, newWalkCost);
				PathAct->Parent = PathOld;
				PathAct->walkCost = newWalkCost;
				PathAct->lastStepCost = newStepCost;
				PathAct->totalCost = newWalkCost + PathAct->remainingCost; // PathAct->totalCost -= PathAct->walkCost - newWalkCost;
				pathUpdateStack[updateStackSize] = PathAct;
				updateStackSize++;
			} else if (newWalkCost == PathAct->walkCost /*&& PathWalkable(PathOld->x, PathOld->y, PathAct->x, PathAct->y)*/ && newStepCost > PathAct->lastStepCost) {
				EventPlrMsg("Update walk 1 %d:%d cost%d:%d last%d to cost%d:%d", PathAct->x - gnSx, PathAct->y - gnSy, PathAct->totalCost, PathAct->walkCost, PathAct->lastStepCost, newWalkCost + PathAct->remainingCost, newWalkCost);
				PathAct->Parent = PathOld;
				PathAct->lastStepCost = newStepCost;
			}
		}
	} while (updateStackSize != 0);
}
#endif
/**
 * @brief heuristic, estimated cost from (x,y) to (gnTx,gnTy)
 */
static int PathRemainingCost(int x, int y)
{
	int delta_x = abs(x - gnTx);
	int delta_y = abs(y - gnTy);

	// see PathStepCost for why this is times 2 and 3
	if (delta_x > delta_y) {
		return 2 * (delta_x - delta_y) + 3 * delta_y;
	} else {
		return 2 * (delta_y - delta_x) + 3 * delta_x;
	}
}

static inline void PathAppendChild(PATHNODE* parent, PATHNODE* child)
{
	int i;

	for (i = 0; i < lengthof(parent->Child); i++) {
		if (parent->Child[i] == NULL) {
			parent->Child[i] = child;
			break;
		}
	}
}

/**
 * @brief add a step from pPath to (dx,dy), return true if successful, and update the frontier/visited nodes accordingly
 *
 * @return true if step successfully added, false if we ran out of nodes to use
 */
static bool path_parent_path(PATHNODE* pPath, int dx, int dy, int stepCost)
{
	int nextWalkCost;
	// int stepCost;
	bool frontier;
	PATHNODE* dxdy;

	// stepCost = PathStepCost(pPath->x, pPath->y, dx, dy);
	nextWalkCost = pPath->walkCost + stepCost;

	// 3 cases to consider
	// case 1: (dx,dy) is already on the frontier
	dxdy = PathFrontNodeAt(dx, dy);
	frontier = dxdy != NULL;
	if (!frontier) {
		// case 2: (dx,dy) was already visited
		dxdy = PathVisitedNodeAt(dx, dy);
	}
	if (dxdy != NULL) {
		PathAppendChild(pPath, dxdy);
		// update the node if necessary
		if (nextWalkCost < dxdy->walkCost /*&& PathWalkable(pPath->x, pPath->y, dx, dy)*/) {
			// EventPlrMsg("Update %d path %d:%d cost%d:%d last%d to cost%d:%d last%d", frontier, dxdy->x - gnSx, dxdy->y - gnSy, dxdy->totalCost, dxdy->walkCost, dxdy->lastStepCost, nextWalkCost + dxdy->remainingCost, nextWalkCost, stepCost);
			dxdy->Parent = pPath;
			dxdy->lastStepCost = stepCost;
			dxdy->walkCost = nextWalkCost;
			dxdy->totalCost = nextWalkCost + dxdy->remainingCost; // dxdy->totalCost -= dxdy->walkCost - nextWalkCost;
#ifdef DEBUG_PATH
			if (!frontier) {
				// already explored, so re-update others starting from that node
				PathUpdateCosts(dxdy);
			}
#endif
		} else if (nextWalkCost == dxdy->walkCost /*&& PathWalkable(pPath->x, pPath->y, dx, dy)*/ && stepCost > dxdy->lastStepCost) {
			// EventPlrMsg("Update 2 path %d:%d cost%d:%d last%d to last%d", dxdy->x - gnSx, dxdy->y - gnSy, dxdy->totalCost, dxdy->walkCost, dxdy->lastStepCost, stepCost);
			dxdy->Parent = pPath;
			dxdy->lastStepCost = stepCost;
		} else {
			// LogErrorF("Ignoring path %d:%d cost%d vs. %d last%d vs. last%d", dxdy->x - gnSx, dxdy->y - gnSy, nextWalkCost, dxdy->walkCost, dxdy->lastStepCost, stepCost);
		}
	} else {
		// case 3: (dx,dy) is totally new
		if (gnLastNodeIdx == MAXPATHNODES - 1) {
			// EventPlrMsg("Not enough nodes %d:%d", pPath->x - gnSx, pPath->y - gnSy);
			return false;
		}
		dxdy = &path_nodes[++gnLastNodeIdx];
		memset(dxdy, 0, sizeof(PATHNODE));
		dxdy->Parent = pPath;
		PathAppendChild(pPath, dxdy);
		dxdy->x = dx;
		dxdy->y = dy;
		dxdy->remainingCost = PathRemainingCost(dx, dy);
		dxdy->walkCost = nextWalkCost;
		dxdy->lastStepCost = stepCost;
		dxdy->totalCost = nextWalkCost + dxdy->remainingCost;
		// add it to the frontier
		PathAddNode(dxdy);
	}
	return true;
}

/**
 * @brief perform a single step of A* bread-first search by trying to step in every possible direction from pPath with goal (x,y). Check each step with PosOk
 *
 * @return FALSE if we ran out of preallocated nodes to use, else TRUE
 */
static bool path_get_path(bool (*PosOk)(int, int, int), int PosOkArg, PATHNODE* pPath)
{
	int sx, sy;
	int dx, dy;
	int i;
	bool ok;

	sx = pPath->x;
	sy = pPath->y;
	static_assert(lengthof(pathxdir) == lengthof(pathydir), "Mismatching pathdir tables.");
	for (i = 0; i < lengthof(pathxdir); i++) {
		dx = sx + pathxdir[i];
		dy = sy + pathydir[i];
		ok = PosOk(PosOkArg, dx, dy);
		if ((ok && PathWalkable(sx, sy, i)) || (!ok && dx == gnTx && dy == gnTy)) {
#ifdef DEBUG_PATH
			assert(stepcost[i] == (i < 4 ? 3 : 2));
#endif
			if (!path_parent_path(pPath, dx, dy, i < 4 ? 3 : 2))
				return false;
		}
	}

	return true;
}

/**
 * @brief get the next node on the A* frontier to explore (estimated to be closest to the goal), mark it as visited, and return it
 */
static PATHNODE* PathPopNode()
{
	PATHNODE* prevNode = pathFrontNodes;
	PATHNODE* result = pathFrontNodes->NextNode;

	if (result != NULL) {
		PATHNODE* res = result;
		PATHNODE* pNode;
		while (true) {
			pNode = res;
			res = res->NextNode;
			if (res == NULL) {
				break;
			}

			if (res->totalCost < result->totalCost
			// || (res->totalCost == result->totalCost && (res->walkCost < result->walkCost || (res->walkCost == result->walkCost && res->lastStepCost > result->lastStepCost)))) {
			 || (res->totalCost == result->totalCost && result->remainingCost == 0)) {
				result = res;
				prevNode = pNode;
			}
		}

		prevNode->NextNode = result->NextNode;
		result->NextNode = pathVisitedNodes->NextNode;
		pathVisitedNodes->NextNode = result;
	}
	return result;
}

/**
 * find the shortest path from (sx,sy) to (dx,dy), using PosOk(PosOkArg,x,y) to
 * check that each step is a valid position. Store the step directions (see
 * path_directions) in path, which must have room for MAX_PATH_LENGTH steps
 * @return the length of the path or -1 if there is none
 */
int FindPath(bool (*PosOk)(int, int, int), int PosOkArg, int sx, int sy, int dx, int dy, int8_t* path)
{
	PATHNODE* currNode;
	int path_length, i;
#ifdef DEBUG_PATH
	// EventPlrMsg("Find path from %d:%d to %d:%d", sx, sy, dx, dy);
	gnSx = sx;
	gnSy = sy;
#endif
	gnTx = dx;
	gnTy = dy;
	// create root nodes for the visited/frontier linked lists
	pathFrontNodes = &path_nodes[0];
	currNode = &path_nodes[1];
	gnLastNodeIdx = 1;
	memset(&path_nodes[0], 0, 2 * sizeof(PATHNODE));
	currNode->x = sx;
	currNode->y = sy;
	currNode->remainingCost = PathRemainingCost(sx, sy);
	currNode->walkCost = 0;
	currNode->totalCost = currNode->remainingCost + currNode->walkCost;
	pathVisitedNodes = currNode;
	// A* search until we find (dx,dy) or fail
	while (true) {
		// LogErrorF("Eval path from %d:%d", currNode->x - gnSx, currNode->y - gnSy);
		// reached the end, success!
		if (currNode->x == gnTx && currNode->y == gnTy) {
			path_length = 0;
			while (currNode->Parent != NULL) {
				if (path_length == MAX_PATH_LENGTH) {
					// EventPlrMsg("Found path from %d:%d to %d:%d -- too long", sx - gnSx, sy - gnSy, dx, dy - gnSy);
					return -1; // path does not fit to the destination, abort!
				}
				reversePathDirs[path_length++] = 3 * (currNode->y - currNode->Parent->y) - currNode->Parent->x + 4 + currNode->x;
				currNode = currNode->Parent;
			}
			for (i = 0; i < path_length; i++)
				path[i] = path_directions[reversePathDirs[path_length - i - 1]];
			// EventPlrMsg("Found path from %d:%d to %d:%d: %d", sx - gnSx, sy - gnSy, dx - gnSx, dy - gnSy, path_length);
			return i;
		}
		if (currNode->totalCost > 3 * MAX_PATH_LENGTH) {
			// EventPlrMsg("No path from %d:%d to %d:%d -- too long", sx - gnSx, sy - gnSy, dx, dy - gnSy);
			return -1; // path is hopeless
		}
		if (!path_get_path(PosOk, PosOkArg, currNode)) {
			// EventPlrMsg("No path from %d:%d to %d:%d -- too many options", sx - gnSx, sy - gnSy, dx, dy - gnSy);
			return -1; // ran out of nodes, abort!
		}
		currNode = PathPopNode();
		if (currNode == NULL) {
			// EventPlrMsg("No path from %d:%d to %d:%d -- not at all", sx - gnSx, sy - gnSy, dx, dy - gnSy);
			return -1; // frontier is empty, no path!
		}
	}
}

/**
 * @brief check if stepping from (sx,sy) to (dx,dy) cuts a corner.
 *
 * If you step from A to B, both Xs need to be clear:
 *
 *  AX
 *  XB
 *
 *  @return true if step is allowed
 */
/*bool PathWalkable(int sx, int sy, int dx, int dy)
{
	bool rv = true;

	switch (path_directions[3 * (dy - sy) + 3 - sx + 1 + dx]) {
	case WALK_N:
		rv = !nSolidTable[dPiece[dx][dy + 1]] && !nSolidTable[dPiece[dx + 1][dy]];
		break;
	case WALK_E:
		rv = !nSolidTable[dPiece[dx][dy + 1]] && !nSolidTable[dPiece[dx - 1][dy]];
		break;
	case WALK_S:
		rv = !nSolidTable[dPiece[dx][dy - 1]] && !nSolidTable[dPiece[dx - 1][dy]];
		break;
	case WALK_W:
		rv = !nSolidTable[dPiece[dx + 1][dy]] && !nSolidTable[dPiece[dx][dy - 1]];
		break;
	}
	return rv;
}*/
bool PathWalkable(int sx, int sy, int pdir)
{
	bool rv = true;
	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "PathWalkable expects a large enough border.");
	switch (pdir) {
	case PDIR_N:
		rv = !(nSolidTable[dPiece[sx - 1][sy]] | nSolidTable[dPiece[sx][sy - 1]]);
		break;
	case PDIR_E:
		rv = !(nSolidTable[dPiece[sx + 1][sy]] | nSolidTable[dPiece[sx][sy - 1]]);
		break;
	case PDIR_S:
		rv = !(nSolidTable[dPiece[sx + 1][sy]] | nSolidTable[dPiece[sx][sy + 1]]);
		break;
	case PDIR_W:
		rv = !(nSolidTable[dPiece[sx][sy + 1]] | nSolidTable[dPiece[sx - 1][sy]]);
		break;
	}
	return rv;
}

DEVILUTION_END_NAMESPACE
