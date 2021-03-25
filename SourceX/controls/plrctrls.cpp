#include "plrctrls.h"

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
#include <cstdint>
#include <list>

#include "controls/controller_motion.h"
#include "controls/game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

bool sgbControllerActive = false;
coords speedspellscoords[50];
int speedspellcount = 0;

/**
 * Native game menu, controlled by simulating a keyboard.
 */
bool InGameMenu()
{
	return stextflag > 0
	    || helpflag
	    || talkflag
	    || qtextflag
	    || gmenu_is_active()
	    || PauseMode == 2
	    || plr[myplr]._pInvincible;
}

namespace {

int slot = SLOTXY_INV_FIRST;

/**
 * Number of angles to turn to face the coordinate
 * @param x Tile coordinates
 * @param y Tile coordinates
 * @return -1 == down
 */
int GetRotaryDistance(int x, int y)
{
	int d, d1, d2;

	if (plr[myplr]._pfutx == x && plr[myplr]._pfuty == y)
		return -1;

	d1 = plr[myplr]._pdir;
	d2 = GetDirection(plr[myplr]._pfutx, plr[myplr]._pfuty, x, y);

	d = abs(d1 - d2);
	if (d > 4)
		return 4 - (d % 4);

	return d;
}

/**
 * @brief Get the best case walking steps to coordinates
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 */
int GetMinDistance(int dx, int dy)
{
	return std::max(abs(plr[myplr]._pfutx - dx), abs(plr[myplr]._pfuty - dy));
}

/**
 * @brief Get walking steps to coordinate
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 * @param maxDistance the max number of steps to search
 * @return number of steps, or 0 if not reachable
 */
int GetDistance(int dx, int dy, int maxDistance)
{
	if (GetMinDistance(dx, dy) > maxDistance) {
		return 0;
	}

	char walkpath[MAX_PATH_LENGTH];
	int steps = FindPath(PosOkPlayer, myplr, plr[myplr]._pfutx, plr[myplr]._pfuty, dx, dy, walkpath);
	if (steps > maxDistance)
		return 0;

	return steps;
}

/**
 * @brief Get distance to coordinate
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 */
int GetDistanceRanged(int dx, int dy)
{
	int a = plr[myplr]._pfutx - dx;
	int b = plr[myplr]._pfuty - dy;

	return sqrt(a * a + b * b);
}

void FindItemOrObject()
{
	int mx = plr[myplr]._pfutx;
	int my = plr[myplr]._pfuty;
	int rotations = 5;

	// As the player can not stand on the edge fo the map this is safe from OOB
	for (int xx = -1; xx <= 1; xx++) {
		for (int yy = -1; yy <= 1; yy++) {
			int i = dItem[mx + xx][my + yy];
			if (i <= 0)
				continue;
			i--;
			if (item[i]._itype == ITYPE_NONE || item[i]._iSelFlag == 0)
				continue;
			int newRotations = GetRotaryDistance(mx + xx, my + yy);
			if (rotations < newRotations)
				continue;
			if (xx != 0 && yy != 0 && GetDistance(mx + xx, my + yy, 1) == 0)
				continue;
			rotations = newRotations;
			pcursitem = i;
			cursmx = mx + xx;
			cursmy = my + yy;
		}
	}

	if (leveltype == DTYPE_TOWN || pcursitem != -1)
		return; // Don't look for objects in town

	for (int xx = -1; xx <= 1; xx++) {
		for (int yy = -1; yy <= 1; yy++) {
			int o = dObject[mx + xx][my + yy];
			if (o == 0)
				continue;
			o = o >= 0 ? o - 1 : -(o + 1);
			if (object[o]._oSelFlag == 0)
				continue;
			if (xx == 0 && yy == 0 && object[o]._oDoorFlag)
				continue; // Ignore doorway so we don't get stuck behind barrels
			int newRotations = GetRotaryDistance(mx + xx, my + yy);
			if (rotations < newRotations)
				continue;
			if (xx != 0 && yy != 0 && GetDistance(mx + xx, my + yy, 1) == 0)
				continue;
			rotations = newRotations;
			pcursobj = o;
			cursmx = mx + xx;
			cursmy = my + yy;
		}
	}
}

void CheckTownersNearby()
{
	for (int i = 0; i < NUM_TOWNERS; i++) {
		int distance = GetDistance(towner[i]._tx, towner[i]._ty, 2);
		if (distance == 0)
			continue;
		pcursmonst = i;
	}
}

bool HasRangedSpell()
{
	int spl = plr[myplr]._pRSpell;

	return spl != SPL_INVALID
	    && spl != SPL_TOWN
	    && spl != SPL_TELEPORT
	    && spelldata[spl].sTargeted
	    && !spelldata[spl].sTownSpell;
}

bool CanTargetMonster(int mi)
{
	if (mi < MAX_MINIONS)
		return false;

	const MonsterStruct &monst = monster[mi];
	if (monst._mFlags & MFLAG_HIDDEN)
		return false;
	if (monst._mhitpoints >> 6 <= 0) // dead
		return false;

	const int mx = monst._mx;
	const int my = monst._my;
	if (!(dFlags[mx][my] & BFLAG_LIT)) // not visible
		return false;
	if (dMonster[mx][my] == 0)
		return false;

	return true;
}

void FindRangedTarget()
{
	int rotations = 0;
	int distance = 0;
	bool canTalk = false;

	// The first MAX_PLRS monsters are reserved for players' golems.
	for (int mi = MAX_PLRS; mi < MAXMONSTERS; mi++) {
		if (!CanTargetMonster(mi))
			continue;
		const bool newCanTalk = CanTalkToMonst(mi);
		if (pcursmonst != -1 && !canTalk && newCanTalk)
			continue;
		const MonsterStruct &mon = monster[mi];
		const int mx = mon._mfutx;
		const int my = mon._mfuty;
		const int newDdistance = GetDistanceRanged(mx, my);
		const int newRotations = GetRotaryDistance(mx, my);
		if (pcursmonst != -1 && canTalk == newCanTalk) {
			if (distance < newDdistance)
				continue;
			if (distance == newDdistance && rotations < newRotations)
				continue;
		}
		distance = newDdistance;
		rotations = newRotations;
		canTalk = newCanTalk;
		pcursmonst = mi;
	}
}

void FindMeleeTarget()
{
	bool visited[MAXDUNX][MAXDUNY] = { { 0 } };
	int maxSteps = MAX_PATH_LENGTH; // Max steps for FindPath is 25
	int rotations = 0;
	bool canTalk = false;

	struct SearchNode {
		int x, y;
		int steps;
	};
	std::list<SearchNode> queue;

	{
		const int start_x = plr[myplr]._pfutx;
		const int start_y = plr[myplr]._pfuty;
		visited[start_x][start_y] = true;
		queue.push_back({ start_x, start_y, 0 });
	}

	while (!queue.empty()) {
		SearchNode node = queue.front();
		queue.pop_front();

		static_assert(lengthof(pathxdir) == lengthof(pathydir), "Mismatching pathdir tables.");
		for (int i = 0; i < lengthof(pathxdir); i++) {
			const int dx = node.x + pathxdir[i];
			const int dy = node.y + pathydir[i];

			if (visited[dx][dy])
				continue; // already visisted

			if (node.steps > maxSteps) {
				visited[dx][dy] = true;
				continue;
			}

			if (!PosOkPlayer(myplr, dx, dy)) {
				visited[dx][dy] = true;

				int mi = dMonster[dx][dy];
				if (mi != 0) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (CanTargetMonster(mi)) {
						const bool newCanTalk = CanTalkToMonst(mi);
						if (pcursmonst != -1 && !canTalk && newCanTalk)
							continue;
						const int newRotations = GetRotaryDistance(dx, dy);
						if (pcursmonst != -1 && canTalk == newCanTalk && rotations < newRotations)
							continue;
						rotations = newRotations;
						canTalk = newCanTalk;
						pcursmonst = mi;
						if (!canTalk)
							maxSteps = node.steps; // Monsters found, cap search to current steps
					}
				}

				continue;
			}

			PATHNODE pPath;
			pPath.x = node.x;
			pPath.y = node.y;

			if (path_solid_pieces(&pPath, dx, dy)) {
				queue.push_back({ dx, dy, node.steps + 1 });
				visited[dx][dy] = true;
			}
		}
	}
}

void CheckMonstersNearby()
{
	if (plr[myplr]._pwtype == WT_RANGED || HasRangedSpell()) {
		FindRangedTarget();
		return;
	}

	FindMeleeTarget();
}

void CheckPlayerNearby()
{
	int newDdistance;
	int rotations = 0;
	int distance = 0;

	if (pcursmonst != -1)
		return;

	int spl = plr[myplr]._pRSpell;
	if (FriendlyMode && spl != SPL_RESURRECT && spl != SPL_HEALOTHER)
		return;

	for (int i = 0; i < MAX_PLRS; i++) {
		if (i == myplr)
			continue;
		const int mx = plr[i]._pfutx;
		const int my = plr[i]._pfuty;
		if (dPlayer[mx][my] == 0
		    || !(dFlags[mx][my] & BFLAG_LIT)
		    || (plr[i]._pHitPoints == 0 && spl != SPL_RESURRECT))
			continue;

		if (plr[myplr]._pwtype == WT_RANGED || HasRangedSpell() || spl == SPL_HEALOTHER) {
			newDdistance = GetDistanceRanged(mx, my);
		} else {
			newDdistance = GetDistance(mx, my, distance);
			if (newDdistance == 0)
				continue;
		}

		if (pcursplr != -1 && distance < newDdistance)
			continue;
		const int newRotations = GetRotaryDistance(mx, my);
		if (pcursplr != -1 && distance == newDdistance && rotations < newRotations)
			continue;

		distance = newDdistance;
		rotations = newRotations;
		pcursplr = i;
	}
}

void FindActor()
{
	if (leveltype != DTYPE_TOWN)
		CheckMonstersNearby();
	else
		CheckTownersNearby();

	if (gbMaxPlayers != 1)
		CheckPlayerNearby();
}

void FindTrigger()
{
	int rotations;
	int distance = 0;

	if (pcursitem != -1 || pcursobj != -1)
		return; // Prefer showing items/objects over triggers (use of cursm* conflicts)

	for (int i = 0; i < nummissiles; i++) {
		int mi = missileactive[i];
		if (missile[mi]._miType == MIS_TOWN || missile[mi]._miType == MIS_RPORTAL) {
			int mix = missile[mi]._mix;
			int miy = missile[mi]._miy;
			const int newDdistance = GetDistance(mix, miy, 2);
			if (newDdistance == 0)
				continue;
			if (pcurstrig != -1 && distance < newDdistance)
				continue;
			const int newRotations = GetRotaryDistance(mix, miy);
			if (pcurstrig != -1 && distance == newDdistance && rotations < newRotations)
				continue;
			cursmx = mix;
			cursmy = miy;
			pcurstrig = MAXTRIGGERS + mi + 1;
			distance = newDdistance;
			rotations = newRotations;
		}
	}

	if (pcurstrig == -1) {
		for (int i = 0; i < numtrigs; i++) {
			int tx = trigs[i]._tx;
			int ty = trigs[i]._ty;
			if (trigs[i]._tlvl == 13)
				ty -= 1;
			const int newDdistance = GetDistance(tx, ty, 2);
			if (newDdistance == 0)
				continue;
			cursmx = tx;
			cursmy = ty;
			pcurstrig = i;
		}

		if (pcurstrig == -1) {
			for (int i = 0; i < MAXQUESTS; i++) {
				if (i == Q_BETRAYER || currlevel != quests[i]._qlevel || quests[i]._qslvl == 0)
					continue;
				const int newDdistance = GetDistance(quests[i]._qtx, quests[i]._qty, 2);
				if (newDdistance == 0)
					continue;
				cursmx = quests[i]._qtx;
				cursmy = quests[i]._qty;
				pcurstrig = -2 - i;
			}
		}
	}

	if (pcursmonst != -1 || pcursplr != -1 || cursmx == -1 || cursmy == -1)
		return; // Prefer monster/player info text

	CheckTrigForce();
	CheckTownPortal();
}

void Interact()
{
	if (leveltype == DTYPE_TOWN && pcursmonst != -1) {
		NetSendCmdLocParam1(TRUE, CMD_TALKXY, towner[pcursmonst]._tx, towner[pcursmonst]._ty, pcursmonst);
	} else if (pcursmonst != -1) {
		if (plr[myplr]._pwtype != WT_RANGED || CanTalkToMonst(pcursmonst)) {
			NetSendCmdParam1(TRUE, CMD_ATTACKID, pcursmonst);
		} else {
			NetSendCmdParam1(TRUE, CMD_RATTACKID, pcursmonst);
		}
	} else if (leveltype != DTYPE_TOWN && pcursplr != -1 && !FriendlyMode) {
		NetSendCmdParam1(TRUE, plr[myplr]._pwtype == WT_RANGED ? CMD_RATTACKPID : CMD_ATTACKPID, pcursplr);
	}
}

void AttrIncBtnSnap(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.y == AxisDirectionY_NONE)
		return;

	if (chrbtnactive && plr[myplr]._pStatPts <= 0)
		return;

	// first, find our cursor location
	int slot = 0;
	for (int i = 0; i < lengthof(ChrBtnsRect); i++) {
		if (MouseX >= ChrBtnsRect[i].x
		 && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
		 && MouseY >= ChrBtnsRect[i].y
		 && MouseY <= ChrBtnsRect[i].h + ChrBtnsRect[i].y) {
			slot = i;
			break;
		}
	}

	if (dir.y == AxisDirectionY_UP) {
		if (slot > 0)
			--slot;
	} else if (dir.y == AxisDirectionY_DOWN) {
		if (slot < 3)
			++slot;
	}

	// move cursor to our new location
	int x = ChrBtnsRect[slot].x + (ChrBtnsRect[slot].w / 2);
	int y = ChrBtnsRect[slot].y + (ChrBtnsRect[slot].h / 2);
	SetCursorPos(x, y);
}

#define SELECT_INV_SLOT(s)									\
{															\
	x = InvRect[s].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2);\
	y = InvRect[s].Y - (INV_SLOT_SIZE_PX / 2);				\
}

#define SELECT_BELT_SLOT(s)										\
{																\
	x = InvRect[s].X + (INV_SLOT_SIZE_PX / 2);					\
	y = SCREEN_HEIGHT - InvRect[s].Y - (INV_SLOT_SIZE_PX / 2);	\
}

/**
 * Move the cursor around in our inventory
 * If mouse coords are at SLOTXY_CHEST_LAST, consider this center of equipment
 * small inventory squares are 29x29 (roughly)
 */
void InvMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater(/*min_interval_ms=*/100);
	dir = repeater.Get(dir);
	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE)
		return;

	int x = MouseX;
	int y = MouseY;
	int r;

	// check which inventory rectangle the mouse is in, if any
	// standard inventory
	for (r = 0; r < SLOTXY_BELT_FIRST; r++) {
		if (x >= InvRect[r].X + RIGHT_PANEL
		 && x <= InvRect[r].X + RIGHT_PANEL + INV_SLOT_SIZE_PX
		 && y >= InvRect[r].Y - INV_SLOT_SIZE_PX
		 && y <= InvRect[r].Y) {
			break;
		}
	}
	// belt
	if (r == SLOTXY_BELT_FIRST) {
		for ( ; r <= SLOTXY_BELT_LAST; r++) {
			if (x >= InvRect[r].X
			 && x <= InvRect[r].X + INV_SLOT_SIZE_PX
			 && y >= SCREEN_HEIGHT - InvRect[r].Y - INV_SLOT_SIZE_PX
			 && y <= SCREEN_HEIGHT - InvRect[r].Y) {
				break;
			}
		}
	}
	if (r > SLOTXY_BELT_LAST)
		r = SLOTXY_INV_FIRST;
	slot = r;

	// when item is on cursor, this is the real cursor XY
	if (dir.x == AxisDirectionX_LEFT) {
		switch (InvSlotTbl[slot]) {
		case SLOT_HEAD:			// head
			break;				// do nothing
		case SLOT_RING_LEFT:	// left ring
			break;				// do nothing
		case SLOT_RING_RIGHT:
			SELECT_INV_SLOT(SLOTXY_RING_LEFT)
			break;
		case SLOT_AMULET:
			x = InvRect[SLOTXY_HEAD_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HEAD_FIRST + 2].Y - INV_SLOT_SIZE_PX;
			break;
		case SLOT_HAND_LEFT:	// left hand
			break;				// do nothing
		case SLOT_HAND_RIGHT:
			x = InvRect[SLOTXY_CHEST_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_CHEST_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_CHEST:
			x = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_STORAGE:		// general inventory
			if (((slot - SLOTXY_INV_FIRST) % 10) != 0) {
				slot -= 1;
				SELECT_INV_SLOT(slot)
			}
		case SLOT_BELT: // belt
			if (slot < SLOTXY_BELT_FIRST + 4)
				break;
			slot -= 4;
			SELECT_BELT_SLOT(slot)
			break;
		default:
			ASSUME_UNREACHABLE
		}
	} else if (dir.x == AxisDirectionX_RIGHT) {
		switch (InvSlotTbl[slot]) {
		case SLOT_HEAD: // head to amulet
			SELECT_INV_SLOT(SLOTXY_AMULET)
			break;
		case SLOT_RING_LEFT:
			SELECT_INV_SLOT(SLOTXY_RING_RIGHT)
			break;
		case SLOT_RING_RIGHT:
			break;				// do nothing
		case SLOT_AMULET:
			break;				// do nothing
		case SLOT_HAND_LEFT:
			x = InvRect[SLOTXY_CHEST_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_CHEST_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_HAND_RIGHT:	// right hand
			break;				// do nothing
		case SLOT_CHEST:
			x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_STORAGE:		// general inventory
			if (((SLOTXY_INV_LAST - slot) % 10) != 0) {
				slot += 1;
				SELECT_INV_SLOT(slot)
			}
			break;
		case SLOT_BELT:			// belt
			if (slot >= SLOTXY_BELT_FIRST + 4) {
				SELECT_INV_SLOT(SLOTXY_INV_FIRST + 30)
			} else {
				slot += 4;
				SELECT_BELT_SLOT(slot)
			}
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}
	if (dir.y == AxisDirectionY_UP) {
		switch (InvSlotTbl[slot]) {
		case SLOT_HEAD:
			break;				// do nothing
		case SLOT_RING_LEFT: // left ring to left hand
			x = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HAND_LEFT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_RING_RIGHT:	// right ring to right hand
			x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_AMULET:
			break;				// do nothing
		case SLOT_HAND_LEFT:	// left hand to head
			x = InvRect[SLOTXY_HEAD_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HEAD_FIRST + 2].Y - INV_SLOT_SIZE_PX;
			break;
		case SLOT_HAND_RIGHT: // right hand to amulet
			SELECT_INV_SLOT(SLOTXY_AMULET)
			break;
		case SLOT_CHEST: // chest to head
			x = InvRect[SLOTXY_HEAD_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HEAD_FIRST + 2].Y - INV_SLOT_SIZE_PX;
			break;
		case SLOT_STORAGE:			// general inventory
			if (slot >= SLOTXY_INV_FIRST && slot <= SLOTXY_INV_FIRST + 2) { // first 3 general slots
				SELECT_INV_SLOT(SLOTXY_RING_LEFT)
			} else if (slot >= SLOTXY_INV_FIRST + 3 && slot <= SLOTXY_INV_FIRST + 6) { // middle 4 general slots
				x = InvRect[SLOTXY_CHEST_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
				y = InvRect[SLOTXY_CHEST_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			} else if (slot >= SLOTXY_INV_FIRST + 7 && slot <= SLOTXY_INV_FIRST + 9) { // last 3 general slots
				SELECT_INV_SLOT(SLOTXY_RING_RIGHT)
			} else {
				slot -= 10;
				SELECT_INV_SLOT(slot)
			}
			break;
		case SLOT_BELT:			// belt to general inventory
			if (slot == SLOTXY_BELT_FIRST || slot == SLOTXY_BELT_FIRST + 4) {
				SELECT_INV_SLOT(SLOTXY_INV_FIRST + 30)
			} else {
				slot -= 1;
				SELECT_BELT_SLOT(slot)
			}
			break;
		default:
			ASSUME_UNREACHABLE
		}
	} else if (dir.y == AxisDirectionY_DOWN) {
		switch (InvSlotTbl[slot]) {
		case SLOT_HEAD:
			x = InvRect[SLOTXY_CHEST_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_CHEST_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_RING_LEFT:
			SELECT_INV_SLOT(SLOTXY_INV_FIRST + 1)
			break;
		case SLOT_RING_RIGHT:
			SELECT_INV_SLOT(SLOTXY_INV_FIRST + 8)
			break;
		case SLOT_AMULET:
			x = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].X + RIGHT_PANEL + INV_SLOT_SIZE_PX;
			y = InvRect[SLOTXY_HAND_RIGHT_FIRST + 2].Y - (INV_SLOT_SIZE_PX / 2);
			break;
		case SLOT_HAND_LEFT:
			SELECT_INV_SLOT(SLOTXY_RING_LEFT)
			break;
		case SLOT_HAND_RIGHT:
			SELECT_INV_SLOT(SLOTXY_RING_RIGHT)
			break;
		case SLOT_CHEST:
			SELECT_INV_SLOT(SLOTXY_INV_FIRST + 5)
			break;
		case SLOT_STORAGE:
			if (slot <= (SLOTXY_INV_LAST - 10)) { // general inventory
				slot += 10;
				SELECT_INV_SLOT(slot)
			} else {
				SELECT_BELT_SLOT(SLOTXY_BELT_FIRST + 4)
			}
			break;
		case SLOT_BELT:
			if (slot == SLOTXY_BELT_LAST || slot == SLOTXY_BELT_LAST - 4)
				break;
			slot += 1;
			SELECT_BELT_SLOT(slot)
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}

	if (x == MouseX && y == MouseY) {
		return; // Avoid wobeling when scalled
	}

	if (pcurs > 1) {       // [3] Keep item in the same slot, don't jump it up
		if (x != MouseX) { // without this, the cursor keeps moving -10
			x -= 10;
			y -= 10;
		}
	}
	SetCursorPos(x, y);
}

/**
 * check if hot spell at X Y exists
 */
bool HSExists(int x, int y)
{
	for (int r = 0; r < speedspellcount; r++) {
		if (x >= speedspellscoords[r].x - SPLICONLENGTH / 2
		 && x <= speedspellscoords[r].x + SPLICONLENGTH / 2
		 && y >= speedspellscoords[r].y - SPLICONLENGTH / 2
		 && y <= speedspellscoords[r].y + SPLICONLENGTH / 2) {
			return true;
		}
	}
	return false;
}

void HotSpellMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE)
		return;

	int spbslot = plr[myplr]._pRSpell;
	for (int r = 0; r < speedspellcount; r++) {
		if (MouseX >= speedspellscoords[r].x - SPLICONLENGTH / 2
		 && MouseX <= speedspellscoords[r].x + SPLICONLENGTH / 2
		 && MouseY >= speedspellscoords[r].y - SPLICONLENGTH / 2
		 && MouseY <= speedspellscoords[r].y + SPLICONLENGTH / 2) {
			spbslot = r;
			break;
		}
	}

	int x = speedspellscoords[spbslot].x;
	int y = speedspellscoords[spbslot].y;

	if (dir.x == AxisDirectionX_LEFT) {
		if (spbslot < speedspellcount - 1) {
			x = speedspellscoords[spbslot + 1].x;
			y = speedspellscoords[spbslot + 1].y;
		}
	} else if (dir.x == AxisDirectionX_RIGHT) {
		if (spbslot > 0) {
			x = speedspellscoords[spbslot - 1].x;
			y = speedspellscoords[spbslot - 1].y;
		}
	}

	if (dir.y == AxisDirectionY_UP) {
		if (HSExists(x, y - SPLICONLENGTH)) {
			y -= SPLICONLENGTH;
		}
	} else if (dir.y == AxisDirectionY_DOWN) {
		if (HSExists(x, y + SPLICONLENGTH)) {
			y += SPLICONLENGTH;
		}
	}

	if (x != MouseX || y != MouseY) {
		SetCursorPos(x, y);
	}
}

void SpellBookMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);

	if (dir.x == AxisDirectionX_LEFT) {
		if (sbooktab > 0)
			sbooktab--;
	} else if (dir.x == AxisDirectionX_RIGHT) {
		if (sbooktab < SPLBOOKTABS - 1)
			sbooktab++;
	}
}

static const direction kFaceDir[3][3] = {
	// NONE      UP      DOWN
	{ DIR_OMNI, DIR_N, DIR_S }, // NONE
	{ DIR_W, DIR_NW, DIR_SW },  // LEFT
	{ DIR_E, DIR_NE, DIR_SE },  // RIGHT
};
static const int kOffsets[8][2] = {
	{ 1, 1 },   // DIR_S
	{ 0, 1 },   // DIR_SW
	{ -1, 1 },  // DIR_W
	{ -1, 0 },  // DIR_NW
	{ -1, -1 }, // DIR_N
	{ 0, -1 },  // DIR_NE
	{ 1, -1 },  // DIR_E
	{ 1, 0 },   // DIR_SE
};

/**
 * @brief check if stepping in direction (dir) from x, y is blocked.
 *
 * If you step from A to B, at leat one of the Xs need to be clear:
 *
 *  AX
 *  XB
 *
 *  @return true if step is blocked
 */
bool IsPathBlocked(int x, int y, int dir)
{
	int d1, d2, d1x, d1y, d2x, d2y;

	switch (dir) {
	case DIR_N:
		d1 = DIR_NW;
		d2 = DIR_NE;
		break;
	case DIR_E:
		d1 = DIR_NE;
		d2 = DIR_SE;
		break;
	case DIR_S:
		d1 = DIR_SE;
		d2 = DIR_SW;
		break;
	case DIR_W:
		d1 = DIR_SW;
		d2 = DIR_NW;
		break;
	default:
		return false;
	}

	d1x = x + kOffsets[d1][0];
	d1y = y + kOffsets[d1][1];
	d2x = x + kOffsets[d2][0];
	d2y = y + kOffsets[d2][1];

	if (!nSolidTable[dPiece[d1x][d1y]] && !nSolidTable[dPiece[d2x][d2y]])
		return false;

	return !PosOkPlayer(myplr, d1x, d1y) && !PosOkPlayer(myplr, d2x, d2y);
}

void WalkInDir(AxisDirection dir)
{
	const int x = plr[myplr]._pfutx;
	const int y = plr[myplr]._pfuty;

	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE) {
		if (sgbControllerActive && plr[myplr].walkpath[0] != WALK_NONE && plr[myplr].destAction == ACTION_NONE)
			NetSendCmdLoc(TRUE, CMD_WALKXY, x, y); // Stop walking
		return;
	}

	const int pdir = kFaceDir[static_cast<std::size_t>(dir.x)][static_cast<std::size_t>(dir.y)];
	const int dx = x + kOffsets[pdir][0];
	const int dy = y + kOffsets[pdir][1];
	plr[myplr]._pdir = pdir;

	if (PosOkPlayer(myplr, dx, dy) && IsPathBlocked(x, y, pdir))
		return; // Don't start backtrack around obstacles

	NetSendCmdLoc(TRUE, CMD_WALKXY, dx, dy);
}

void QuestLogMove(AxisDirection move_dir)
{
	static AxisDirectionRepeater repeater;
	move_dir = repeater.Get(move_dir);
	if (move_dir.y == AxisDirectionY_UP)
		QuestlogUp();
	else if (move_dir.y == AxisDirectionY_DOWN)
		QuestlogDown();
}

void StoreMove(AxisDirection move_dir)
{
	static AxisDirectionRepeater repeater;
	move_dir = repeater.Get(move_dir);
	if (move_dir.y == AxisDirectionY_UP)
		STextUp();
	else if (move_dir.y == AxisDirectionY_DOWN)
		STextDown();
}

typedef void (*HandleLeftStickOrDPadFn)(dvl::AxisDirection);

HandleLeftStickOrDPadFn GetLeftStickOrDPadGameUIHandler()
{
	if (invflag) {
		return &InvMove;
	} else if (chrflag && plr[myplr]._pStatPts > 0) {
		return &AttrIncBtnSnap;
	} else if (spselflag) {
		return &HotSpellMove;
	} else if (sbookflag) {
		return &SpellBookMove;
	} else if (questlog) {
		return &QuestLogMove;
	} else if (stextflag != STORE_NONE) {
		return &StoreMove;
	}
	return NULL;
}

void ProcessLeftStickOrDPadGameUI() {
	HandleLeftStickOrDPadFn handler = GetLeftStickOrDPadGameUIHandler();
	if (handler != NULL)
		handler(controller.GetLeftStickOrDpadDirection(true));
}

void Movement()
{
	if (InGameMenu()
	    || IsControllerButtonPressed(ControllerButton_BUTTON_START)
	    || IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return;

	AxisDirection move_dir = GetMoveDirection();
	if (move_dir.x != AxisDirectionX_NONE || move_dir.y != AxisDirectionY_NONE) {
		sgbControllerActive = true;
	}

	if (GetLeftStickOrDPadGameUIHandler() == NULL) {
		WalkInDir(move_dir);
	}
}

struct RightStickAccumulator {

	RightStickAccumulator()
	{
		lastTc = SDL_GetTicks();
		hiresDX = 0;
		hiresDY = 0;
	}

	void pool(int *x, int *y, int slowdown)
	{
		const Uint32 tc = SDL_GetTicks();
		const int dtc = tc - lastTc;
		hiresDX += controller.rightStickX * dtc;
		hiresDY += controller.rightStickY * dtc;
		const int dx = hiresDX / slowdown;
		const int dy = hiresDY / slowdown;
		*x += dx;
		*y -= dy;
		lastTc = tc;
		// keep track of remainder for sub-pixel motion
		hiresDX -= dx * slowdown;
		hiresDY -= dy * slowdown;
	}

	void clear()
	{
		lastTc = SDL_GetTicks();
	}

	DWORD lastTc;
	float hiresDX;
	float hiresDY;
};

} // namespace

void StoreSpellCoords()
{
	const int START_X = PANEL_LEFT + 12 + SPLICONLENGTH / 2;
	const int END_X = START_X + SPLICONLENGTH * SPLROWICONLS;
	const int END_Y = PANEL_TOP - 17 - SPLICONLENGTH / 2;
	speedspellcount = 0;
	int xo = END_X;
	int yo = END_Y;
	for (int i = 0; i < 4; i++) {
		std::uint64_t spells;
		switch (i) {
		case RSPLTYPE_SKILL:
			spells = plr[myplr]._pAblSpells;
			break;
		case RSPLTYPE_SPELL:
			spells = plr[myplr]._pMemSpells;
			break;
		case RSPLTYPE_SCROLL:
			spells = plr[myplr]._pScrlSpells;
			break;
		case RSPLTYPE_CHARGES:
			spells = plr[myplr]._pISpells;
			break;
		default:
			continue;
		}
		std::uint64_t spell = 1;
		for (int j = 1; j < NUM_SPELLS; j++) {
			if ((spell & spells)) {
				speedspellscoords[speedspellcount] = { xo, yo };
				++speedspellcount;
				xo -= SPLICONLENGTH;
				if (xo < START_X) {
					xo = END_X;
					yo -= SPLICONLENGTH;
				}
			}
			spell <<= 1;
		}
		if (spells && xo != END_X)
			xo -= SPLICONLENGTH;
		if (xo < START_X) {
			xo = END_X;
			yo -= SPLICONLENGTH;
		}
	}
}

bool IsAutomapActive()
{
	return automapflag && leveltype != DTYPE_TOWN;
}

bool IsMovingMouseCursorWithController()
{
	return controller.rightStickX != 0 || controller.rightStickY != 0;
}

void HandleRightStickMotion()
{
	static RightStickAccumulator acc;
	// deadzone is handled in ScaleJoystickAxes() already
	if (controller.rightStickX == 0 && controller.rightStickY == 0) {
		acc.clear();
		return;
	}

	if (IsAutomapActive()) { // move map
		int dx = 0, dy = 0;
		acc.pool(&dx, &dy, 32);
		AutoMapXOfs += dy + dx;
		AutoMapYOfs += dy - dx;
		return;
	}

	{ // move cursor
		sgbControllerActive = false;
		int x = MouseX;
		int y = MouseY;
		acc.pool(&x, &y, 2);
		x = std::min(std::max(x, 0), SCREEN_WIDTH - 1);
		y = std::min(std::max(y, 0), SCREEN_HEIGHT - 1);

		// We avoid calling `SetCursorPos` within the same SDL tick because
		// that can cause all stick motion events to arrive before all
		// cursor position events.
		static int lastMouseSetTick = 0;
		const int now = SDL_GetTicks();
		if (now - lastMouseSetTick > 0)
		{
			SetCursorPos(x, y);
			lastMouseSetTick = now;
		}
	}
}

/**
 * @brief Moves the mouse to the first inventory slot.
 */
void FocusOnInventory()
{
	SetCursorPos(InvRect[SLOTXY_INV_FIRST].X + RIGHT_PANEL + (INV_SLOT_SIZE_PX / 2), InvRect[SLOTXY_INV_FIRST].Y - (INV_SLOT_SIZE_PX / 2));
}

void plrctrls_after_check_curs_move()
{
	// check for monsters first, then items, then towners.
	if (sgbControllerActive) {
		// Clear focuse set by cursor
		pcursplr = -1;
		pcursmonst = -1;
		pcursitem = -1;
		pcursobj = -1;
		pcurstrig = -1;
		cursmx = -1;
		cursmy = -1;
		if (!invflag) {
			*infostr = '\0';
			FindActor();
			FindItemOrObject();
			FindTrigger();
		}
	}
}

void plrctrls_every_frame()
{
	ProcessLeftStickOrDPadGameUI();
	HandleRightStickMotion();
}

void plrctrls_after_game_logic()
{
	Movement();
}

void UseBeltItem(int type)
{
	ItemStruct *pi;

	pi = plr[myplr].SpdList;
	for (int i = 0; i < MAXBELTITEMS; i++, pi++) {
		const int id = AllItemsList[pi->_iIdx].iMiscId;
		const int spellId = AllItemsList[pi->_iIdx].iSpell;
		if ((type == BLT_HEALING && (id == IMISC_HEAL || id == IMISC_FULLHEAL || (id == IMISC_SCROLL && spellId == SPL_HEAL)))
		    || (type == BLT_MANA && (id == IMISC_MANA || id == IMISC_FULLMANA))
		    || id == IMISC_REJUV || id == IMISC_FULLREJUV) {
			if (UseInvItem(INVITEM_BELT_FIRST + i))
				break;
		}
	}
}

void PerformPrimaryAction()
{
	if (DoPanBtn())
		return;

	if (TryIconCurs(FALSE))
		return;

	if (MouseX <= InvRect[SLOTXY_BELT_LAST].X + INV_SLOT_SIZE_PX && MouseY >= SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST].Y - INV_SLOT_SIZE_PX) {
		// in belt
		// assert(!DoPanBtn());
		CheckBeltClick();
		return;
	}

	if (invflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		// in inventory
		CheckInvClick();
		return;
	}

	if (spselflag) {
		SetSpell();
		return;
	}

	if (chrflag && CheckChrBtns()) {
		ReleaseChrBtns();
		return;
	}

	Interact();
}

static bool SpellHasActorTarget()
{
	int spl = plr[myplr]._pRSpell;
	if (spl == SPL_TOWN || spl == SPL_TELEPORT)
		return false;

	if (spl == SPL_FIREWALL && pcursmonst != -1) {
		cursmx = monster[pcursmonst]._mx;
		cursmy = monster[pcursmonst]._my;
	}

	return pcursplr != -1 || pcursmonst != -1;
}

static void UpdateSpellTarget()
{
	if (SpellHasActorTarget())
		return;

	pcursplr = -1;
	pcursmonst = -1;

	const PlayerStruct &player = plr[myplr];

	int range = 1;
	if (plr[myplr]._pRSpell == SPL_TELEPORT)
		range = 4;

	cursmx = player._pfutx + kOffsets[player._pdir][0] * range;
	cursmy = player._pfuty + kOffsets[player._pdir][1] * range;
}

/**
 * @brief Try dropping item in all 9 possible places
 */
bool TryDropItem()
{
	cursmx = plr[myplr]._pfutx + 1;
	cursmy = plr[myplr]._pfuty;
	return DropItem();
}

void PerformSpellAction()
{
	if (InGameMenu() || questlog || sbookflag)
		return;

	if (spselflag) {
		SetSpell();
		return;
	}

	if (TryIconCurs(FALSE))
		return;
	if (pcurs >= CURSOR_FIRSTITEM) {
		TryDropItem();
		return;
	}

	int spl = plr[myplr]._pRSpell;
	if ((pcursplr == -1 && (spl == SPL_RESURRECT || spl == SPL_HEALOTHER))
	    || (pcursobj == -1 && spl == SPL_DISARM)) {
		PlaySFX(sgSFXSets[SFXS_PLR_27][plr[myplr]._pClass]);
		return;
	}

	UpdateSpellTarget();
	AltActionBtnCmd(FALSE);
}

static void CtrlUseInvItem()
{
	ItemStruct *is;

	if (pcursinvitem == -1)
		return;

	if (pcursinvitem <= INVITEM_INV_LAST)
		is = &plr[myplr].InvList[pcursinvitem - INVITEM_INV_FIRST];
	else
		is = &plr[myplr].SpdList[pcursinvitem - INVITEM_BELT_FIRST];

	if (is->_iMiscId == IMISC_SCROLL && spelldata[is->_iSpell].sTargeted) {
		return;
	}

	UseInvItem(pcursinvitem);
}

void PerformSecondaryAction()
{
	if (invflag) {
		CtrlUseInvItem();
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM && !TryDropItem())
		return;
	if (pcurs > CURSOR_HAND)
		NewCursor(CURSOR_HAND);

	if (pcursitem != -1) {
		NetSendCmdLocParam1(TRUE, CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
	} else if (pcursobj != -1) {
		NetSendCmdLocParam1(TRUE, CMD_OPOBJXY, cursmx, cursmy, pcursobj);
	} else if (pcurstrig != -1) {
		if (pcurstrig >= MAXTRIGGERS + 1) {
			int mi = pcurstrig - (MAXTRIGGERS + 1);
			MakePlrPath(myplr, missile[mi]._mix, missile[mi]._miy, TRUE);
		} else if (pcurstrig >= 0) {
			MakePlrPath(myplr, trigs[pcurstrig]._tx, trigs[pcurstrig]._ty, TRUE);
		} else {
			int qn = -2 - pcurstrig;
			MakePlrPath(myplr, quests[qn]._qtx, quests[qn]._qty, TRUE);
		}
		plr[myplr].destAction = ACTION_WALK;
	}
}

DEVILUTION_END_NAMESPACE

#endif
