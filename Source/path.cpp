/**
 * @file path.cpp
 *
 * Implementation of the path finding algorithms.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Pre-allocated memory to store nodes used by the path finding algorithm. */
PATHNODE path_nodes[MAXPATHNODES];
/** The number of in-use nodes in path_nodes. */
static int gnCurNodes;
/** Container for reconstructing the path after the A* search is done. */
static char reversePathDirs[MAX_PATH_LENGTH];
/** A linked list of all visited nodes. */
static PATHNODE* pathVisitedNodes;
/** A stack for recursively update nodes. */
static PATHNODE* pathUpdateStack[MAXPATHNODES];
/** A linked list of the A* frontier, sorted by distance. */
static PATHNODE* pathFrontNodes;
/** The target location. */
static int gnTx, gnTy;

/** For iterating over the 8 possible movement directions. */
//                     PDIR_N   W   E   S  NW  NE  SE  SW
const char pathxdir[8] = { -1, -1,  1,  1, -1,  0,  1,  0 };
const char pathydir[8] = { -1,  1, -1,  1,  0, -1,  0,  1 };
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
//char path_directions[9] = { WALK_N, WALK_NE, WALK_E, WALK_NW, 0, WALK_SE, WALK_W, WALK_SW, WALK_S };
char path_directions[9] = { DIR_N, DIR_NE, DIR_E, DIR_NW, DIR_NONE, DIR_SE, DIR_W, DIR_SW, DIR_S };

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
	PATHNODE* result = pathVisitedNodes->NextNode;

	while (result != NULL) {
		if (result->x == dx && result->y == dy)
			break;
		result = result->NextNode;
	}
	return result;
}

/**
 * @brief insert pPath into the frontier (keeping the frontier sorted by total distance)
 */
static void PathAddNode(PATHNODE* pPath)
{
	PATHNODE *next, *current;
	BYTE currCost;

	current = pathFrontNodes;
	next = pathFrontNodes->NextNode;
	currCost = pPath->totalCost;
	while (next != NULL && next->totalCost < currCost) {
		current = next;
		next = next->NextNode;
	}
	pPath->NextNode = next;
	current->NextNode = pPath;
}

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
	BYTE i, newWalkCost;

	pathUpdateStack[0] = pPath;
	updateStackSize = 1;
	do {
		updateStackSize--;
		PathOld = pathUpdateStack[updateStackSize];
		for (i = 0; i < lengthof(PathOld->Child); i++) {
			PathAct = PathOld->Child[i];
			if (PathAct == NULL)
				break;

			newWalkCost = PathOld->walkCost + PathStepCost(PathOld->x, PathOld->y, PathAct->x, PathAct->y);
			if (newWalkCost < PathAct->walkCost /*&& PathWalkable(PathOld->x, PathOld->y, PathAct->x, PathAct->y)*/) {
				PathAct->Parent = PathOld;
				PathAct->walkCost = newWalkCost;
				PathAct->totalCost = newWalkCost + PathAct->remainingCost;
				pathUpdateStack[updateStackSize] = PathAct;
				updateStackSize++;
			}
		}
	} while (updateStackSize != 0);
}

/**
 * @brief zero one of the preallocated nodes and return a pointer to it, or NULL if none are available
 */
static PATHNODE* path_new_step()
{
	PATHNODE* new_node;

	if (gnCurNodes == MAXPATHNODES)
		return NULL;

	new_node = &path_nodes[gnCurNodes];
	gnCurNodes++;
	memset(new_node, 0, sizeof(PATHNODE));
	return new_node;
}

/**
 * @brief heuristic, estimated cost from (x,y) to (gnTx,gnTy)
 */
static int PathRemainingCost(int x, int y)
{
	int delta_x = abs(x - gnTx);
	int delta_y = abs(y - gnTy);

	// see PathStepCost for why this is times 2
	return 2 * (delta_x + delta_y);
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
static bool path_parent_path(PATHNODE* pPath, int dx, int dy)
{
	BYTE nextWalkCost;
	PATHNODE* dxdy;

	nextWalkCost = pPath->walkCost + PathStepCost(pPath->x, pPath->y, dx, dy);

	// 3 cases to consider
	// case 1: (dx,dy) is already on the frontier
	dxdy = PathFrontNodeAt(dx, dy);
	if (dxdy != NULL) {
		PathAppendChild(pPath, dxdy);
		if (nextWalkCost < dxdy->walkCost /*&& PathWalkable(pPath->x, pPath->y, dx, dy)*/) {
			// we'll explore it later, just update
			dxdy->Parent = pPath;
			dxdy->walkCost = nextWalkCost;
			dxdy->totalCost = nextWalkCost + dxdy->remainingCost;
		}
	} else {
		// case 2: (dx,dy) was already visited
		dxdy = PathVisitedNodeAt(dx, dy);
		if (dxdy != NULL) {
			PathAppendChild(pPath, dxdy);
			if (nextWalkCost < dxdy->walkCost /*&& PathWalkable(pPath->x, pPath->y, dx, dy)*/) {
				// update the node
				dxdy->Parent = pPath;
				dxdy->walkCost = nextWalkCost;
				dxdy->totalCost = nextWalkCost + dxdy->remainingCost;
				// already explored, so re-update others starting from that node
				PathUpdateCosts(dxdy);
			}
		} else {
			// case 3: (dx,dy) is totally new
			dxdy = path_new_step();
			if (dxdy == NULL)
				return false;
			dxdy->Parent = pPath;
			PathAppendChild(pPath, dxdy);
			dxdy->x = dx;
			dxdy->y = dy;
			dxdy->remainingCost = PathRemainingCost(dx, dy);
			dxdy->walkCost = nextWalkCost;
			dxdy->totalCost = nextWalkCost + dxdy->remainingCost;
			// add it to the frontier
			PathAddNode(dxdy);
		}
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
	int dx, dy;
	int i;
	bool ok;

	static_assert(lengthof(pathxdir) == lengthof(pathydir), "Mismatching pathdir tables.");
	for (i = 0; i < lengthof(pathxdir); i++) {
		dx = pPath->x + pathxdir[i];
		dy = pPath->y + pathydir[i];
		ok = PosOk(PosOkArg, dx, dy);
		if ((ok && PathWalkable(pPath->x, pPath->y, i)) || (!ok && dx == gnTx && dy == gnTy)) {
			if (!path_parent_path(pPath, dx, dy))
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
	PATHNODE* result;

	result = pathFrontNodes->NextNode;
	if (result != NULL) {
		pathFrontNodes->NextNode = result->NextNode;
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
int FindPath(bool (*PosOk)(int, int, int), int PosOkArg, int sx, int sy, int dx, int dy, char* path)
{
	PATHNODE* currNode;
	int path_length, i;

	// clear all nodes, create root nodes for the visited/frontier linked lists
	gnCurNodes = 0;
	pathFrontNodes = path_new_step();
	pathVisitedNodes = path_new_step();
	gnTx = dx;
	gnTy = dy;
	currNode = path_new_step();
	currNode->x = sx;
	currNode->y = sy;
	currNode->remainingCost = PathRemainingCost(sx, sy);
	currNode->walkCost = 0;
	currNode->totalCost = currNode->remainingCost + currNode->walkCost;
	pathFrontNodes->NextNode = currNode;
	// A* search until we find (dx,dy) or fail
	while ((currNode = PathPopNode())) {
		// reached the end, success!
		if (currNode->x == gnTx && currNode->y == gnTy) {
			path_length = 0;
			while (currNode->Parent != NULL) {
				if (path_length == MAX_PATH_LENGTH)
					return -1; // path does not fit to the destination, abort!
				reversePathDirs[path_length++] = path_directions[3 * (currNode->y - currNode->Parent->y) - currNode->Parent->x + 4 + currNode->x];
				currNode = currNode->Parent;
			}
			for (i = 0; i < path_length; i++)
				path[i] = reversePathDirs[path_length - i - 1];
			return i;
		}
		// ran out of nodes, abort!
		if (!path_get_path(PosOk, PosOkArg, currNode))
			return -1;
	}
	// frontier is empty, no path!
	return -1;
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
