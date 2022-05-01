#include "plrctrls.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include <cstdint>
#include <queue>

#include "controls/controller_motion.h"
#include "controls/game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

bool sgbControllerActive = false;
static POS32 speedspellscoords[50];
int speedspellcount = 0;

/**
 * Native game menu, controlled by simulating a keyboard.
 */
bool InGameMenu()
{
	return stextflag != STORE_NONE
	    || gbHelpflag
	    || gbTalkflag
	    || gbQtextflag
	    || gmenu_is_active()
	    || gbGamePaused
	    || myplr._pInvincible;
}

static int slot = SLOTXY_INV_FIRST;

/**
 * Number of angles to turn to face the coordinate
 * @param x Tile coordinates
 * @param y Tile coordinates
 * @return -1 == down
 */
static int GetRotaryDistance(int x, int y)
{
	int d, d1, d2;

	if (myplr._pfutx == x && myplr._pfuty == y)
		return -1;

	d1 = myplr._pdir;
	d2 = GetDirection(myplr._pfutx, myplr._pfuty, x, y);

	d = ((d2 - d1) + NUM_DIRS) & 7;
	if (d > 4)
		d = NUM_DIRS - d;

	return d;
}

/**
 * @brief Get the best case walking steps to coordinates
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 */
static int GetMinDistance(int dx, int dy)
{
	return std::max(abs(myplr._pfutx - dx), abs(myplr._pfuty - dy));
}

/**
 * @brief Get walking steps to coordinate
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 * @param maxDistance the max number of steps to search
 * @return number of steps, or -1 if not reachable
 */
static int GetDistance(int dx, int dy, int maxDistance)
{
	if (GetMinDistance(dx, dy) > maxDistance) {
		return 0;
	}

	char walkpath[MAX_PATH_LENGTH];
	int steps = FindPath(PosOkPlayer, mypnum, myplr._pfutx, myplr._pfuty, dx, dy, walkpath);
	if (steps > maxDistance)
		return -1;

	return steps;
}

/**
 * @brief Get distance to coordinate
 * @param dx Tile coordinates
 * @param dy Tile coordinates
 */
static int GetDistanceRanged(int dx, int dy)
{
	int a = myplr._pfutx - dx;
	int b = myplr._pfuty - dy;

	return sqrt(a * a + b * b);
}

static void FindItemOrObject()
{
	int mx = myplr._pfutx;
	int my = myplr._pfuty;
	int rotations = 5;

	// As the player can not stand on the edge fo the map this is safe from OOB
	for (int xx = -1; xx <= 1; xx++) {
		for (int yy = -1; yy <= 1; yy++) {
			int ii = dItem[mx + xx][my + yy];
			if (ii <= 0)
				continue;
			ii--;
			if (items[ii]._itype == ITYPE_NONE || items[ii]._iSelFlag == 0)
				continue;
			int newRotations = GetRotaryDistance(mx + xx, my + yy);
			if (rotations < newRotations)
				continue;
			if (GetDistance(mx + xx, my + yy, 1) < 0)
				continue;
			rotations = newRotations;
			pcursitem = ii;
			cursmx = mx + xx;
			cursmy = my + yy;
		}
	}

	if (currLvl._dType == DTYPE_TOWN || pcursitem != ITEM_NONE)
		return; // Don't look for objects in town

	for (int xx = -1; xx <= 1; xx++) {
		for (int yy = -1; yy <= 1; yy++) {
			int oi = dObject[mx + xx][my + yy];
			if (oi == 0)
				continue;
			oi = oi >= 0 ? oi - 1 : -(oi + 1);
			if (objects[oi]._oSelFlag == 0)
				continue;
			if (xx == 0 && yy == 0 && objects[oi]._oDoorFlag != ODT_NONE)
				continue; // Ignore doorway so we don't get stuck behind barrels
			int newRotations = GetRotaryDistance(mx + xx, my + yy);
			if (rotations < newRotations)
				continue;
			if (GetDistance(mx + xx, my + yy, 1) < 0)
				continue;
			rotations = newRotations;
			pcursobj = oi;
			cursmx = mx + xx;
			cursmy = my + yy;
		}
	}
}

static void CheckTownersNearby()
{
	for (int i = MAX_MINIONS; i < numtowners; i++) {
		int distance = GetDistance(monsters[i]._mx, monsters[i]._my, 2);
		if (distance < 0)
			continue;
		pcursmonst = i;
	}
}

static bool HasRangedSpell()
{
	int spl = myplr._pAltAtkSkill;
	if (spl == SPL_INVALID)
		spl = myplr._pAltMoveSkill;

	return spl != SPL_INVALID
	    && spl != SPL_TOWN
	    && spl != SPL_TELEPORT
	    && (spelldata[spl].sSkillFlags & SDFLAG_TARGETED)
	    && (spelldata[spl].sUseFlags & myplr._pSkillFlags) == spelldata[spl].sUseFlags;
}

static bool CanTargetMonster(int mnum)
{
	MonsterStruct* mon;

	// The first MAX_MINIONS monsters are reserved for players' golems.
	if (mnum < MAX_MINIONS)
		return false;

	mon = &monsters[mnum];
	if (mon->_mmode > MM_INGAME_LAST)
		return false;
	if (mon->_mFlags & MFLAG_HIDDEN)
		return false;
	if (mon->_mhitpoints < (1 << 6)) // dead
		return false;

	if (!(dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE))
		return false;

	return true;
}

static void FindRangedTarget()
{
	int rotations = NUM_DIRS, distance = MAXDUNX + MAXDUNY, mnum;
	bool canTalk = true;

	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		if (!CanTargetMonster(mnum))
			continue;
		const bool newCanTalk = CanTalkToMonst(mnum);
		if (!canTalk && newCanTalk)
			continue;
		const MonsterStruct &mon = monsters[mnum];
		const int mx = mon._mfutx;
		const int my = mon._mfuty;
		const int newDdistance = GetDistanceRanged(mx, my);
		const int newRotations = GetRotaryDistance(mx, my);
		if (canTalk == newCanTalk) {
			if (distance < newDdistance)
				continue;
			if (distance == newDdistance && rotations < newRotations)
				continue;
		}
		distance = newDdistance;
		rotations = newRotations;
		canTalk = newCanTalk;
		pcursmonst = mnum;
	}
}

static void FindMeleeTarget()
{
	bool visited[MAXDUNX][MAXDUNY] = { { 0 } };
	int maxSteps = MAX_PATH_LENGTH;
	int rotations = NUM_DIRS;
	bool canTalk = true;

	struct SearchNode {
		int x, y;
		int steps;
	};
	std::queue<SearchNode> nodes;
	{
		const int startX = myplr._pfutx;
		const int startY = myplr._pfuty;
		visited[startX][startY] = true;
		nodes.push({ startX, startY, 0 });
	}

	while (!nodes.empty()) {
		SearchNode node = nodes.front();
		nodes.pop();

		if (node.steps > maxSteps)
			break;
		static_assert(lengthof(pathxdir) == lengthof(pathydir), "Mismatching pathdir tables.");
		for (int i = 0; i < lengthof(pathxdir); i++) {
			const int dx = node.x + pathxdir[i];
			const int dy = node.y + pathydir[i];
			if (visited[dx][dy])
				continue; // already visited

			if (!PosOkPlayer(mypnum, dx, dy)) {
				visited[dx][dy] = true;

				int mi = dMonster[dx][dy];
				if (mi != 0) {
					mi = mi >= 0 ? mi - 1 : -(mi + 1);
					if (CanTargetMonster(mi)) {
						const bool newCanTalk = CanTalkToMonst(mi);
						if (!canTalk && newCanTalk)
							continue;
						const int newRotations = GetRotaryDistance(dx, dy);
						if (canTalk == newCanTalk && rotations < newRotations)
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

			if (PathWalkable(node.x, node.y, i)) {
				nodes.push({ dx, dy, node.steps + 1 });
				visited[dx][dy] = true;
			}
		}
	}
}

static void CheckMonstersNearby()
{
	if ((myplr._pSkillFlags & SFLAG_RANGED) || HasRangedSpell()) {
		FindRangedTarget();
		return;
	}

	FindMeleeTarget();
}

static void CheckPlayerNearby()
{
	int newDdistance;
	int rotations;
	int distance = MAXDUNX + MAXDUNY;

	if (pcursmonst != MON_NONE)
		return;

	int spl = myplr._pAltAtkSkill;
	if (spl == SPL_INVALID)
		spl = myplr._pAltMoveSkill;

	for (int i = 0; i < MAX_PLRS; i++) {
		if (i == mypnum)
			continue;
		if (spl != SPL_RESURRECT) {
			if (players[i]._pHitPoints == 0)
				continue;
			if (players[i]._pTeam == myplr._pTeam && spl != SPL_HEALOTHER)
				continue;
		}
		const int mx = players[i]._pfutx;
		const int my = players[i]._pfuty;
		if (!(dFlags[mx][my] & BFLAG_VISIBLE))
			continue;
		if ((myplr._pSkillFlags & SFLAG_RANGED) || HasRangedSpell()) {
			newDdistance = GetDistanceRanged(mx, my);
		} else {
			newDdistance = GetDistance(mx, my, distance);
			if (newDdistance < 0)
				continue;
		}

		if (distance < newDdistance)
			continue;
		const int newRotations = GetRotaryDistance(mx, my);
		if (distance == newDdistance && rotations < newRotations)
			continue;

		distance = newDdistance;
		rotations = newRotations;
		pcursplr = i;
	}
}

static void FindActor()
{
	if (currLvl._dType != DTYPE_TOWN)
		CheckMonstersNearby();
	else
		CheckTownersNearby();

	if (!IsLocalGame)
		CheckPlayerNearby();
}

static void FindTrigger()
{
	int rotations;
	int distance = 2 + 1;

	if (pcursitem != ITEM_NONE || pcursobj != OBJ_NONE)
		return; // Prefer showing items/objects over triggers (use of cursm* conflicts)

	for (int i = 0; i < nummissiles; i++) {
		int mi = missileactive[i];
		if (missile[mi]._miType == MIS_TOWN || missile[mi]._miType == MIS_RPORTAL) {
			int mix = missile[mi]._mix;
			int miy = missile[mi]._miy;
			const int newDdistance = GetDistance(mix, miy, 2);
			if (newDdistance < 0)
				continue;
			if (distance < newDdistance)
				continue;
			const int newRotations = GetRotaryDistance(mix, miy);
			if (distance == newDdistance && rotations < newRotations)
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
			const int newDdistance = GetDistance(tx, ty, 2);
			if (newDdistance < 0)
				continue;
			cursmx = tx;
			cursmy = ty;
			pcurstrig = i;
		}
	}

	if (pcursmonst != MON_NONE || pcursplr != PLR_NONE || cursmx == -1 || cursmy == -1)
		return; // Prefer monster/player info text

	CheckTrigForce();
	CheckTownPortal();
}

static void Interact()
{
	/*
	if (pcursmonst != MON_NONE && CanTalkToMonst(pcursmonst))
		NetSendCmdParam1(CMD_TALKXY, pcursmonst);

	if (currLvl._dType != DTYPE_TOWN) {
		int attack = myplr._pAtkSkill;
		bool melee = (myplr._pSkillFlags & SFLAG_MELEE) != 0;
		if (pcursmonst != MON_NONE)
			NetSendCmdMonstAttack(melee ? CMD_ATTACKID : CMD_RATTACKID, pcursmonst, attack);
		else if (pcursplr != PLR_NONE && myplr._pTeam != players[pcursplr]._pTeam)
			NetSendCmdPlrAttack(pcursplr, attack);
	}*/
	ActionBtnCmd(false);
}

static void AttrIncBtnSnap(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.y == AxisDirectionY_NONE)
		return;

	if (gbChrbtnactive && myplr._pStatPts <= 0)
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
static void InvMove(AxisDirection dir)
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
			break;
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
static bool HSExists(int x, int y)
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

static void HotSpellMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.x == AxisDirectionX_NONE && dir.y == AxisDirectionY_NONE)
		return;

	int spbslot = myplr._pAltAtkSkill;
	if (spbslot == SPL_INVALID)
		spbslot = myplr._pAltMoveSkill;
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

static void SpellBookMove(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);

	if (dir.x == AxisDirectionX_LEFT) {
		if (guBooktab > 0)
			guBooktab--;
	} else if (dir.x == AxisDirectionX_RIGHT) {
		if (guBooktab < SPLBOOKTABS - 1)
			guBooktab++;
	}
}

static const direction FaceDir[3][3] = {
	// NONE      UP      DOWN
	{ DIR_NONE, DIR_N, DIR_S }, // NONE
	{ DIR_W, DIR_NW, DIR_SW },  // LEFT
	{ DIR_E, DIR_NE, DIR_SE },  // RIGHT
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
static bool IsPathBlocked(int x, int y, int dir)
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
	case DIR_SW:
	case DIR_NW:
	case DIR_NE:
	case DIR_SE:
		return false;
	default:
		ASSUME_UNREACHABLE
	}

	d1x = x + offset_x[d1];
	d1y = y + offset_y[d1];
	d2x = x + offset_x[d2];
	d2y = y + offset_y[d2];

	if (!nSolidTable[dPiece[d1x][d1y]] && !nSolidTable[dPiece[d2x][d2y]])
		return false;

	return !PosOkPlayer(mypnum, d1x, d1y) && !PosOkPlayer(mypnum, d2x, d2y);
}

static bool CanChangeDirection()
{
	PlayerStruct *p = &myplr;

	if (p->_pmode == PM_STAND)
		return true;
	if (p->_pmode == PM_ATTACK && p->_pAnimFrame > p->_pAFNum)
		return true;
	if (p->_pmode == PM_RATTACK && p->_pAnimFrame > p->_pAFNum)
		return true;
	if (p->_pmode == PM_SPELL && p->_pAnimFrame > p->_pSFNum)
		return true;
	return false;
}

static void WalkInDir(AxisDirection dir)
{
	const int x = myplr._pfutx;
	const int y = myplr._pfuty;

	const int pdir = FaceDir[dir.x][dir.y];
	if (pdir == DIR_NONE) {
		if (sgbControllerActive && myplr.walkpath[0] != DIR_NONE && myplr.destAction == ACTION_NONE)
			NetSendCmdLoc(CMD_WALKXY, x, y); // Stop walking
		return;
	}

	const int dx = x + offset_x[pdir];
	const int dy = y + offset_y[pdir];
	if (CanChangeDirection())
		myplr._pdir = pdir;

	if (PosOkPlayer(mypnum, dx, dy) && IsPathBlocked(x, y, pdir))
		return; // Don't start backtrack around obstacles

	NetSendCmdLoc(CMD_WALKXY, dx, dy);
}

static void QuestLogMove(AxisDirection moveDir)
{
	static AxisDirectionRepeater repeater;
	moveDir = repeater.Get(moveDir);
	if (moveDir.y == AxisDirectionY_UP)
		QuestlogUp();
	else if (moveDir.y == AxisDirectionY_DOWN)
		QuestlogDown();
}

static void StoreMove(AxisDirection moveDir)
{
	static AxisDirectionRepeater repeater;
	moveDir = repeater.Get(moveDir);
	if (moveDir.y == AxisDirectionY_UP)
		STextUp();
	else if (moveDir.y == AxisDirectionY_DOWN)
		STextDown();
}

typedef void (*HandleLeftStickOrDPadFn)(dvl::AxisDirection);

static HandleLeftStickOrDPadFn GetLeftStickOrDPadGameUIHandler()
{
	if (gbInvflag) {
		return &InvMove;
	} else if (gbChrflag && myplr._pStatPts > 0) {
		return &AttrIncBtnSnap;
	} else if (gbSkillListFlag) {
		return &HotSpellMove;
	} else if (gbSbookflag) {
		return &SpellBookMove;
	} else if (gbQuestlog) {
		return &QuestLogMove;
	} else if (stextflag != STORE_NONE) {
		return &StoreMove;
	}
	return NULL;
}

static void ProcessLeftStickOrDPadGameUI()
{
	HandleLeftStickOrDPadFn handler = GetLeftStickOrDPadGameUIHandler();
	if (handler != NULL)
		handler(GetLeftStickOrDpadDirection(true));
}

static void Movement()
{
	if (InGameMenu()
	    || IsControllerButtonPressed(ControllerButton_BUTTON_START)
	    || IsControllerButtonPressed(ControllerButton_BUTTON_BACK))
		return;

	AxisDirection moveDir = GetMoveDirection();
	if (moveDir.x != AxisDirectionX_NONE || moveDir.y != AxisDirectionY_NONE) {
		sgbControllerActive = true;
	}

	if (GetLeftStickOrDPadGameUIHandler() == NULL) {
		WalkInDir(moveDir);
	}
}

struct RightStickAccumulator {

	RightStickAccumulator()
	{
		lastTc = SDL_GetTicks();
		hiresDX = 0;
		hiresDY = 0;
	}

	void Pool(int *x, int *y, int slowdown)
	{
		const Uint32 tc = SDL_GetTicks();
		const int dtc = tc - lastTc;
		hiresDX += rightStickX * dtc;
		hiresDY += rightStickY * dtc;
		const int dx = hiresDX / slowdown;
		const int dy = hiresDY / slowdown;
		*x += dx;
		*y -= dy;
		lastTc = tc;
		// keep track of remainder for sub-pixel motion
		hiresDX -= dx * slowdown;
		hiresDY -= dy * slowdown;
	}

	void Clear()
	{
		lastTc = SDL_GetTicks();
	}

	Uint32 lastTc;
	float hiresDX;
	float hiresDY;
};

void StoreSpellCoords()
{
	const int START_X = PANEL_LEFT + 12 + SPLICONLENGTH / 2;
	const int END_X = START_X + SPLICONLENGTH * SPLROWICONLS;
	const int END_Y = PANEL_BOTTOM - (128 + 17) - SPLICONLENGTH / 2;
	speedspellcount = 0;
	int xo = END_X;
	int yo = END_Y;
	for (int i = 0; i < 4; i++) {
		std::uint64_t spells;
		switch (i) {
		case RSPLTYPE_ABILITY:
			spells = myplr._pAblSkills;
			break;
		case RSPLTYPE_SPELL:
			spells = myplr._pMemSkills;
			break;
		case RSPLTYPE_SCROLL:
			spells = myplr._pScrlSkills;
			break;
		case RSPLTYPE_CHARGES:
			spells = myplr._pISpells;
			break;
		default:
			continue;
		}
		std::uint64_t spell = 1;
		for (int j = 1; j < NUM_SPELLS; j++) {
			if ((spell & spells) != 0) {
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
		if (spells != 0 && xo != END_X)
			xo -= SPLICONLENGTH;
		if (xo < START_X) {
			xo = END_X;
			yo -= SPLICONLENGTH;
		}
	}
}

bool IsAutomapActive()
{
	return gbAutomapflag && currLvl._dType != DTYPE_TOWN;
}

bool IsMovingMouseCursorWithController()
{
	return rightStickX != 0 || rightStickY != 0;
}

// Moves the map if active, the cursor otherwise.
static void HandleRightStickMotion()
{
	static RightStickAccumulator acc;
	// deadzone is handled in ScaleJoystickAxes() already
	if (rightStickX == 0 && rightStickY == 0) {
		acc.Clear();
		return;
	}

	if (IsAutomapActive()) { // move map
		int dx = 0, dy = 0;
		acc.Pool(&dx, &dy, 32);
		AutoMapXOfs += dy + dx;
		AutoMapYOfs += dy - dx;
		return;
	}

	{ // move cursor
		sgbControllerActive = false;
		int x = MouseX;
		int y = MouseY;
		acc.Pool(&x, &y, 2);
		x = std::min(std::max(x, 0), SCREEN_WIDTH - 1);
		y = std::min(std::max(y, 0), SCREEN_HEIGHT - 1);

		// We avoid calling `SetCursorPos` within the same SDL tick because
		// that can cause all stick motion events to arrive before all
		// cursor position events.
		static int lastMouseSetTick = 0;
		const int now = SDL_GetTicks();
		if (now - lastMouseSetTick > 0) {
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
		pcursplr = PLR_NONE;
		pcursmonst = MON_NONE;
		pcursitem = ITEM_NONE;
		pcursobj = OBJ_NONE;
		pcurstrig = -1;
		cursmx = -1;
		cursmy = -1;
		if (myplr._pInvincible) {
			return;
		}
		if (gbDoomflag) {
			return;
		}
		if (!gbInvflag) {
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

	pi = myplr._pSpdList;
	for (int i = 0; i < MAXBELTITEMS; i++, pi++) {
		const int id = AllItemsList[pi->_iIdx].iMiscId;
		const int spellId = AllItemsList[pi->_iIdx].iSpell;
		if ((type == BLT_HEALING && (id == IMISC_HEAL || id == IMISC_FULLHEAL || (id == IMISC_SCROLL && spellId == SPL_HEAL)))
		    || (type == BLT_MANA && (id == IMISC_MANA || id == IMISC_FULLMANA))
		    || id == IMISC_REJUV || id == IMISC_FULLREJUV) {
			if (InvUseItem(INVITEM_BELT_FIRST + i))
				break;
		}
	}
}

void PerformPrimaryAction()
{
	if (DoPanBtn())
		return;

	if (TryIconCurs(false))
		return;

	if (MouseX <= InvRect[SLOTXY_BELT_LAST].X + INV_SLOT_SIZE_PX && MouseY >= SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST].Y - INV_SLOT_SIZE_PX) {
		// in belt
		// assert(!DoPanBtn());
		CheckBeltClick(false);
		return;
	}

	if (gbInvflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		// in inventory
		CheckInvClick(false);
		return;
	}

	if (gbSbookflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		SelectBookSkill(false, false);
		return;
	}

	if (gbTeamFlag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		// in team panel
		CheckTeamClick(false);
		return;
	}

	if (gbSkillListFlag) {
		SetSkill(false, false);
		return;
	}

	if (gbChrflag && CheckChrBtns()) {
		ReleaseChrBtns();
		return;
	}

	Interact();
}

static bool SpellHasActorTarget()
{
	int spl = myplr._pAltAtkSkill;
	if (spl == SPL_INVALID)
		spl = myplr._pAltMoveSkill;
	if (spl == SPL_TOWN || spl == SPL_TELEPORT)
		return false;

	if (spl == SPL_FIREWALL && pcursmonst != MON_NONE) {
		cursmx = monsters[pcursmonst]._mx;
		cursmy = monsters[pcursmonst]._my;
	}

	return pcursplr != PLR_NONE || pcursmonst != MON_NONE;
}

static void UpdateSpellTarget()
{
	if (SpellHasActorTarget())
		return;

	pcursplr = PLR_NONE;
	pcursmonst = MON_NONE;

	const PlayerStruct &player = myplr;

	int range = 1;
	if (myplr._pAltMoveSkill == SPL_TELEPORT)
		range = 4;

	cursmx = player._pfutx + offset_x[player._pdir] * range;
	cursmy = player._pfuty + offset_y[player._pdir] * range;
}

/**
 * @brief Try dropping item in all 9 possible places
 */
static void TryDropItem()
{
	cursmx = myplr._pfutx + 1;
	cursmy = myplr._pfuty;
	DropItem();
}

void PerformSpellAction()
{
	if (InGameMenu() || gbQuestlog || gbSbookflag || gbTeamFlag)
		return;

	if (gbSkillListFlag) {
		SetSkill(false, true);
		return;
	}

	if (TryIconCurs(false))
		return;
	if (pcurs >= CURSOR_FIRSTITEM) {
		TryDropItem();
		return;
	}

	int spl = myplr._pAltAtkSkill;
	if (spl == SPL_INVALID)
		spl = myplr._pAltMoveSkill;
	if ((pcursplr == PLR_NONE && (spl == SPL_RESURRECT || spl == SPL_HEALOTHER))
	    || (pcursobj == OBJ_NONE && spl == SPL_DISARM)) {
		PlaySFX(sgSFXSets[SFXS_PLR_27][myplr._pClass]);
		return;
	}

	UpdateSpellTarget();
	AltActionBtnCmd(false);
}

static void CtrlUseInvItem()
{
	ItemStruct *is;

	if (pcursinvitem == INVITEM_NONE)
		return;

	is = PlrItem(mypnum, pcursinvitem);

	if (is->_iMiscId == IMISC_SCROLL && (spelldata[is->_iSpell].sSkillFlags & SDFLAG_TARGETED)) {
		return;
	}

	InvUseItem(pcursinvitem);
}

void PerformSecondaryAction()
{
	int dx, dy;

	if (gbInvflag) {
		CtrlUseInvItem();
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM) {
		TryDropItem();
		return;
	}
	if (pcurs > CURSOR_HAND)
		NewCursor(CURSOR_HAND);

	if (pcursitem != ITEM_NONE) {
		NetSendCmdLocParam1(CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
	} else if (pcursobj != OBJ_NONE) {
		NetSendCmdLocParam1(CMD_OPOBJXY, cursmx, cursmy, pcursobj);
	} else if (pcurstrig != -1) {
		if (pcurstrig >= MAXTRIGGERS + 1) {
			// portal
			int mi = pcurstrig - (MAXTRIGGERS + 1);
			dx = missile[mi]._mix;
			dy = missile[mi]._miy;
		} else {
			// standard trigger
			dx = trigs[pcurstrig]._tx;
			dy = trigs[pcurstrig]._ty;
		}
		MakePlrPath(mypnum, dx, dy, true);
		myplr.destAction = ACTION_WALK;
	}
}

DEVILUTION_END_NAMESPACE

#endif
