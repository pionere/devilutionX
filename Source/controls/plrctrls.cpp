#include "plrctrls.h"

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#include "controller_motion.h"
#include "game_controls.h"

DEVILUTION_BEGIN_NAMESPACE

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
	    //|| gbDoomflag
	    || gmenu_is_active()
	    || gbGamePaused
	    || gbDeathflag;
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

	int8_t walkpath[MAX_PATH_LENGTH];
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

static void FindItem()
{
	int mx = myplr._pfutx;
	int my = myplr._pfuty;
	int rotations = 5;

	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "FindItem expects a large enough border.");
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
			pcurspos.x = mx + xx;
			pcurspos.y = my + yy;
		}
	}
}

static void FindObject()
{
	int mx = myplr._pfutx;
	int my = myplr._pfuty;
	int rotations = 5;

	static_assert(DBORDERX >= 1 && DBORDERY >= 1, "FindObject expects a large enough border.");
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
			pcurspos.x = mx + xx;
			pcurspos.y = my + yy;
		}
	}
}

static void FindTowner()
{
	for (int i = MAX_MINIONS; i < numtowners; i++) {
		int distance = GetDistance(monsters[i]._mx, monsters[i]._my, 2);
		if (distance < 0)
			continue;
		pcursmonst = i;
	}
}

static bool HasRangedSkill()
{
	if (myplr._pSkillFlags & SFLAG_RANGED)
		return true;

	int spl = myplr._pAltAtkSkill;
	if (spl == SPL_INVALID)
		spl = myplr._pAltMoveSkill;

	return spl != SPL_INVALID
	    && spl != SPL_TOWN
	    && spl != SPL_TELEPORT
	    && (spelldata[spl].sSkillFlags & SDFLAG_TARGETED)
	    && (spelldata[spl].sUseFlags & myplr._pSkillFlags) == spelldata[spl].sUseFlags;
}

/**
 * @brief Find a monster to target
 * @param mode: 0 - offensive, 1 - heal
 * @param ranged: whether the current player is melee or ranged
 */
static void FindMonster(int mode, bool ranged)
{
	int newDistance, rotations, distance = MAXDUNX + MAXDUNY, mnum, lastMon;
	bool canTalk = true;

	if (mode == 0) {
		mnum = MAX_MINIONS;
		lastMon = MAXMONSTERS;
	} else {
		mnum = 0;
		lastMon = MAX_MINIONS;
	}
	for ( ; mnum < lastMon; mnum++) {
		const MonsterStruct& mon = monsters[mnum];
		if (mon._mmode > MM_INGAME_LAST || mon._mmode == MM_DEATH)
			continue;
		if (mon._mFlags & MFLAG_HIDDEN)
			continue;
		if (!(dFlags[mon._mx][mon._my] & BFLAG_VISIBLE))
			continue;

		const int mx = mon._mfutx;
		const int my = mon._mfuty;
		if (ranged) {
			newDistance = GetDistanceRanged(mx, my);
		} else {
			newDistance = GetDistance(mx, my, distance);
			if (newDistance < 0)
				continue;
		}
		const int newRotations = GetRotaryDistance(mx, my);
		const bool newCanTalk = CanTalkToMonst(mnum);
		if (canTalk == newCanTalk) {
			if (distance < newDistance)
				continue;
			if (distance == newDistance && rotations < newRotations)
				continue;
		} else if (newCanTalk) {
			continue;
		}
		distance = newDistance;
		rotations = newRotations;
		canTalk = newCanTalk;
		pcursmonst = mnum;
	}
}

/**
 * @brief Find a player to target
 * @param mode: 0 - offensive, 1 - heal, 2 - dead
 * @param ranged: whether the current player is melee or ranged
 */
static void FindPlayer(int mode, bool ranged)
{
	int newDistance, rotations, distance = MAXDUNX + MAXDUNY, pnum;
	bool sameTeam = mode == 0;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (pnum == mypnum)
			continue;
		if (!plr._pActive || plr._pDunLevel != currLvl._dLevelIdx)
			continue;
		if ((mode == 2) != (plr._pHitPoints == 0))
			continue;
		if (!(dFlags[plr._px][plr._py] & BFLAG_VISIBLE))
			continue;

		const int mx = plr._pfutx;
		const int my = plr._pfuty;
		if (ranged) {
			newDistance = GetDistanceRanged(mx, my);
		} else {
			newDistance = GetDistance(mx, my, distance);
			if (newDistance < 0)
				continue;
		}
		const int newRotations = GetRotaryDistance(mx, my);
		const bool newSameTeam = plr._pTeam == myplr._pTeam;
		if (sameTeam == newSameTeam) {
			if (distance < newDistance)
				continue;
			if (distance == newDistance && rotations < newRotations)
				continue;
		} else {
			if (mode == 0) {
				if (newSameTeam)
					continue; // offensive mode -> prefer opponents
			} else {
				if (!newSameTeam)
					continue; // defensive mode -> prefer teammates
			}
		}

		distance = newDistance;
		rotations = newRotations;
		sameTeam = newSameTeam;
		pcursplr = pnum;
	}
}

static void FindTrigger()
{
	int rotations;
	int distance = 2 + 1;

	if (pcursitem != ITEM_NONE || pcursobj != OBJ_NONE)
		return; // Prefer showing items/objects over triggers (use of cursm* conflicts)

	for (int i = 0; i < numtrigs; i++) {
		int tx = trigs[i]._tx;
		int ty = trigs[i]._ty;
		const int newDistance = GetDistance(tx, ty, 2);
		if (newDistance < 0)
			continue;
		pcurspos.x = tx;
		pcurspos.y = ty;
		pcurstrig = i;
	}

	for (int i = 0; i < nummissiles; i++) {
		int mi = missileactive[i];
		if (missile[mi]._miType == MIS_TOWN || missile[mi]._miType == MIS_RPORTAL) {
			int mix = missile[mi]._mix;
			int miy = missile[mi]._miy;
			const int newDistance = GetDistance(mix, miy, 2);
			if (newDistance < 0)
				continue;
			if (distance < newDistance)
				continue;
			const int newRotations = GetRotaryDistance(mix, miy);
			if (distance == newDistance && rotations < newRotations)
				continue;
			pcurspos.x = mix;
			pcurspos.y = miy;
			pcurstrig = MAXTRIGGERS + mi + 1;
			distance = newDistance;
			rotations = newRotations;
		}
	}

	/* commented out because it would just set the pcurspos.x/y and pcurstrig fields again
	if (pcursmonst != MON_NONE || pcursplr != PLR_NONE || pcurstrig == -1)
		return; // Prefer monster/player info text

	CheckTrigForce();
	CheckTownPortal();*/
}

static void AttrIncBtnSnap(AxisDirection dir)
{
	static AxisDirectionRepeater repeater;
	dir = repeater.Get(dir);
	if (dir.y == AxisDirectionY_NONE)
		return;

	// find the current slot based on the mouse position
	int slot = -1;
	int sy = MousePos.y - (gnWndCharY + CHRBTN_TOP(0));
	while (sy >= 0) {
		slot++;
		sy -= CHRBTN_TOP(slot + 1) - CHRBTN_TOP(slot);
	}
	// step in the desired direction
	if (dir.y == AxisDirectionY_UP) {
		slot--;
	} else {
		// assert(dir.y == AxisDirectionY_DOWN);
		slot++;
	}
	// limit the slot to the available ones
	if (slot < 0)
		slot = 0;
	else if (slot >= NUM_ATTRIBS)
		slot = NUM_ATTRIBS - 1;
	// move cursor to our new location
	int x = gnWndCharX + ((SDL_GetModState() & KMOD_ALT) != 0 ? CHRBTN_ALT : CHRBTN_LEFT) + (CHRBTN_WIDTH / 2);
	int y = gnWndCharY + CHRBTN_TOP(slot) + (CHRBTN_HEIGHT / 2);
	if (abs(MousePos.x - x) >= CHRBTN_WIDTH / 2 || abs(MousePos.y - y) >= CHRBTN_HEIGHT / 2) // Avoid wobbling when scaled
		SetCursorPos(x, y);
}

#define SELECT_INV_SLOT(s)                                     \
	{                                                          \
		slot = s;                                              \
		x = gnWndInvX + InvRect[s].X + (INV_SLOT_SIZE_PX / 2); \
		y = gnWndInvY + InvRect[s].Y - (INV_SLOT_SIZE_PX / 2); \
	}

#define SELECT_BELT_SLOT(s)                                     \
	{                                                           \
		slot = s;                                               \
		x = gnWndBeltX + InvRect[s].X + (INV_SLOT_SIZE_PX / 2); \
		y = gnWndBeltY + InvRect[s].Y - (INV_SLOT_SIZE_PX / 2); \
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

	int x = MousePos.x;
	int y = MousePos.y;
	int r;

	// check which inventory rectangle the mouse is in, if any
	// standard inventory
	for (r = 0; r < SLOTXY_BELT_FIRST; r++) {
		if (POS_IN_RECT(x, y,
			gnWndInvX + InvRect[r].X, gnWndInvY + InvRect[r].Y - INV_SLOT_SIZE_PX,
			INV_SLOT_SIZE_PX + 1, INV_SLOT_SIZE_PX + 1)) {
			break;
		}
	}
	// belt
	if (r == SLOTXY_BELT_FIRST) {
		for ( ; r <= SLOTXY_BELT_LAST; r++) {
			if (POS_IN_RECT(x, y,
				gnWndBeltX + InvRect[r].X, gnWndBeltY + InvRect[r].Y - INV_SLOT_SIZE_PX,
				INV_SLOT_SIZE_PX + 1, INV_SLOT_SIZE_PX + 1)) {
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
		case SLOT_HEAD:      // head
			break;           // do nothing
		case SLOT_RING_LEFT: // left ring
			break;           // do nothing
		case SLOT_RING_RIGHT:
			SELECT_INV_SLOT(SLOTXY_RING_LEFT)
			break;
		case SLOT_AMULET:
			SELECT_INV_SLOT(SLOTXY_HEAD_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			y -= INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_HAND_LEFT: // left hand
			break;           // do nothing
		case SLOT_HAND_RIGHT:
			SELECT_INV_SLOT(SLOTXY_CHEST_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_CHEST:
			SELECT_INV_SLOT(SLOTXY_HAND_LEFT_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_STORAGE: // general inventory
			if (((slot - SLOTXY_INV_FIRST) % 10) != 0) {
				SELECT_INV_SLOT(slot - 1)
			}
			break;
		case SLOT_BELT: // belt
			if (slot < SLOTXY_BELT_FIRST + 4)
				break;
			SELECT_BELT_SLOT(slot - 4)
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
		case SLOT_RING_RIGHT: // rigth ring
			break;            // do nothing
		case SLOT_AMULET:     // amu
			break;            // do nothing
		case SLOT_HAND_LEFT:
			SELECT_INV_SLOT(SLOTXY_CHEST_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_HAND_RIGHT: // right hand
			break;            // do nothing
		case SLOT_CHEST:
			SELECT_INV_SLOT(SLOTXY_HAND_RIGHT_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_STORAGE: // general inventory
			if (((SLOTXY_INV_LAST - slot) % 10) != 0) {
				SELECT_INV_SLOT(slot + 1)
			}
			break;
		case SLOT_BELT: // belt
			if (slot >= SLOTXY_BELT_FIRST + 4) {
				SELECT_INV_SLOT(SLOTXY_INV_FIRST + 30)
			} else {
				SELECT_BELT_SLOT(slot + 4)
			}
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}
	if (dir.y == AxisDirectionY_UP) {
		switch (InvSlotTbl[slot]) {
		case SLOT_HEAD:
			break;           // do nothing
		case SLOT_RING_LEFT: // left ring to left hand
			SELECT_INV_SLOT(SLOTXY_HAND_LEFT_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_RING_RIGHT: // right ring to right hand
			SELECT_INV_SLOT(SLOTXY_HAND_RIGHT_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_AMULET:
			break;           // do nothing
		case SLOT_HAND_LEFT: // left hand to head
			SELECT_INV_SLOT(SLOTXY_HEAD_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			y -= INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_HAND_RIGHT: // right hand to amulet
			SELECT_INV_SLOT(SLOTXY_AMULET)
			break;
		case SLOT_CHEST: // chest to head
			SELECT_INV_SLOT(SLOTXY_HEAD_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			y -= INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_STORAGE: // general inventory
			if (slot >= SLOTXY_INV_FIRST && slot <= SLOTXY_INV_FIRST + 2) { // first 3 general slots
				SELECT_INV_SLOT(SLOTXY_RING_LEFT)
			} else if (slot >= SLOTXY_INV_FIRST + 3 && slot <= SLOTXY_INV_FIRST + 6) { // middle 4 general slots
				SELECT_INV_SLOT(SLOTXY_CHEST_FIRST + 2)
				x += INV_SLOT_SIZE_PX / 2;
			} else if (slot >= SLOTXY_INV_FIRST + 7 && slot <= SLOTXY_INV_FIRST + 9) { // last 3 general slots
				SELECT_INV_SLOT(SLOTXY_RING_RIGHT)
			} else {
				SELECT_INV_SLOT(slot - 10)
			}
			break;
		case SLOT_BELT: // belt to general inventory
			if (slot == SLOTXY_BELT_FIRST || slot == SLOTXY_BELT_FIRST + 4) {
				SELECT_INV_SLOT(SLOTXY_INV_FIRST + 30)
			} else {
				SELECT_BELT_SLOT(slot - 1)
			}
			break;
		default:
			ASSUME_UNREACHABLE
		}
	} else if (dir.y == AxisDirectionY_DOWN) {
		switch (InvSlotTbl[slot]) {
		case SLOT_HEAD:
			SELECT_INV_SLOT(SLOTXY_CHEST_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
			break;
		case SLOT_RING_LEFT:
			SELECT_INV_SLOT(SLOTXY_INV_FIRST + 1)
			break;
		case SLOT_RING_RIGHT:
			SELECT_INV_SLOT(SLOTXY_INV_FIRST + 8)
			break;
		case SLOT_AMULET:
			SELECT_INV_SLOT(SLOTXY_HAND_RIGHT_FIRST + 2)
			x += INV_SLOT_SIZE_PX / 2;
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
				SELECT_INV_SLOT(slot + 10)
			} else {
				SELECT_BELT_SLOT(SLOTXY_BELT_FIRST + 4)
			}
			break;
		case SLOT_BELT:
			if (slot == SLOTXY_BELT_LAST || slot == SLOTXY_BELT_LAST - 4)
				break;
			SELECT_BELT_SLOT(slot + 1)
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}

	if (x == MousePos.x && y == MousePos.y) {
		return; // Avoid wobbling when scaled
	}

	if (pcursicon > CURSOR_HAND) { // [3] Keep item in the same slot, don't jump it up
		if (x != MousePos.x) {     // without this, the cursor keeps moving -10
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
		if (POS_IN_RECT(x, y,
			speedspellscoords[r].x - SPLICON_WIDTH / 2, speedspellscoords[r].y - SPLICON_HEIGHT / 2,
			SPLICON_WIDTH, SPLICON_HEIGHT)) {
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
		if (POS_IN_RECT(MousePos.x, MousePos.y,
			speedspellscoords[r].x - SPLICON_WIDTH / 2, speedspellscoords[r].y - SPLICON_HEIGHT / 2,
			SPLICON_WIDTH, SPLICON_HEIGHT)) {
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
		if (HSExists(x, y - SPLICON_HEIGHT)) {
			y -= SPLICON_HEIGHT;
		}
	} else if (dir.y == AxisDirectionY_DOWN) {
		if (HSExists(x, y + SPLICON_HEIGHT)) {
			y += SPLICON_HEIGHT;
		}
	}

	if (x != MousePos.x || y != MousePos.y) {
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
	PlayerStruct* p = &myplr;

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
		if (sgbControllerActive && myplr._pWalkpath[0] != DIR_NONE && myplr._pDestAction == ACTION_NONE)
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
	else if (moveDir.x == AxisDirectionX_LEFT)
		STextLeft();
	else if (moveDir.x == AxisDirectionX_RIGHT)
		STextRight();
}

typedef void (*HandleLeftStickOrDPadFn)(dvl::AxisDirection);

static HandleLeftStickOrDPadFn GetLeftStickOrDPadGameUIHandler()
{
	if (stextflag != STORE_NONE) {
		return &StoreMove;
	}
	if (gbSkillListFlag) {
		return &HotSpellMove;
	}
	if (gnNumActiveWindows != 0) {
		switch (gaActiveWindows[gnNumActiveWindows - 1]) {
		case WND_INV:   return &InvMove;
		case WND_CHAR:  return myplr._pStatPts > 0 ? &AttrIncBtnSnap : NULL;
		case WND_BOOK:  return &SpellBookMove;
		case WND_TEAM:  return NULL;
		case WND_QUEST: return &QuestLogMove;
		default: ASSUME_UNREACHABLE; break;
		}
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

	void Pool(POS32& pos, int slowdown)
	{
		const Uint32 tc = SDL_GetTicks();
		const int dtc = tc - lastTc;
		hiresDX += rightStickX * dtc;
		hiresDY += rightStickY * dtc;
		const int dx = hiresDX / slowdown;
		const int dy = hiresDY / slowdown;
		pos.x = dx;
		pos.y = dy;
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
	const int START_X = PANEL_MIDX(SPLICON_WIDTH * SPLROWICONLS) + SPLICON_WIDTH / 2;
	const int END_X = START_X + SPLICON_WIDTH * SPLROWICONLS;
	const int END_Y = PANEL_BOTTOM - (128 + 17) - SPLICON_HEIGHT / 2;
	speedspellcount = 0;
	int xo = END_X;
	int yo = END_Y;
	static_assert(RSPLTYPE_ABILITY == 0, "Looping over the spell-types in StoreSpellCoords relies on ordered, indexed enum values 1.");
	static_assert(RSPLTYPE_SPELL == 1, "Looping over the spell-types in StoreSpellCoords relies on ordered, indexed enum values 2.");
	static_assert(RSPLTYPE_INV == 2, "Looping over the spell-types in StoreSpellCoords relies on ordered, indexed enum values 3.");
	static_assert(RSPLTYPE_CHARGES == 3, "Looping over the spell-types in StoreSpellCoords relies on ordered, indexed enum values 4.");
	for (int i = 0; i < 4; i++) {
		std::uint64_t spells;
		switch (i) {
		case RSPLTYPE_ABILITY:
			spells = myplr._pAblSkills;
			break;
		case RSPLTYPE_SPELL:
			spells = myplr._pMemSkills;
			break;
		case RSPLTYPE_INV:
			spells = myplr._pInvSkills;
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
				xo -= SPLICON_WIDTH;
				if (xo < START_X) {
					xo = END_X;
					yo -= SPLICON_HEIGHT;
				}
			}
			spell <<= 1;
		}
		if (spells != 0 && xo != END_X)
			xo -= SPLICON_WIDTH;
		if (xo < START_X) {
			xo = END_X;
			yo -= SPLICON_HEIGHT;
		}
	}
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
		POS32 pos;
		acc.Pool(pos, 32);
		AutoMapXOfs += pos.y + pos.x;
		AutoMapYOfs += pos.y - pos.x;
		return;
	}

	{ // move cursor
		sgbControllerActive = false;
		POS32 pos;
		acc.Pool(pos, 2);
		pos.x += MousePos.x;
		pos.y += MousePos.y;
		pos.x = std::min(std::max(pos.x, 0), SCREEN_WIDTH - 1);
		pos.y = std::min(std::max(pos.y, 0), SCREEN_HEIGHT - 1);

		// We avoid calling `SetCursorPos` within the same SDL tick because
		// that can cause all stick motion events to arrive before all
		// cursor position events.
		static int lastMouseSetTick = 0;
		const int now = SDL_GetTicks();
		if (now - lastMouseSetTick > 0) {
			SetCursorPos(pos.x, pos.y);
			lastMouseSetTick = now;
		}
	}
}

/**
 * @brief Moves the mouse to the first inventory slot.
 */
void FocusOnInventory()
{
	SetCursorPos(gnWndInvX + InvRect[SLOTXY_INV_FIRST].X + (INV_SLOT_SIZE_PX / 2), gnWndInvY + InvRect[SLOTXY_INV_FIRST].Y - (INV_SLOT_SIZE_PX / 2));
}

// Moves the mouse to the first attribute "+" button.
void FocusOnCharInfo()
{
	if (gbInvflag || myplr._pStatPts <= 0)
		return;

	// Jump to the first incrementable stat.
	SetCursorPos(gnWndCharX + CHRBTN_LEFT + (CHRBTN_WIDTH / 2), gnWndCharY + CHRBTN_TOP(0) + (CHRBTN_HEIGHT / 2));
}

void plrctrls_after_check_curs_move()
{
	// check for monsters first, then items, then towners.
	if (sgbControllerActive) {
		// Clear focus set by cursor
		pcursplr = PLR_NONE;
		pcursmonst = MON_NONE;
		pcursitem = ITEM_NONE;
		pcursobj = OBJ_NONE;
		pcurstrig = -1;
		pcurspos.x = -1;
		pcurspos.y = -1;
		static_assert(MDM_ALIVE == 0, "BitOr optimization of plrctrls_after_check_curs_move expects MDM_ALIVE to be zero.");
		static_assert(STORE_NONE == 0, "BitOr optimization of plrctrls_after_check_curs_move expects STORE_NONE to be zero.");
		if (gbDeathflag /*| gbDoomflag*/ | gbSkillListFlag | gbQtextflag | stextflag) {
			return;
		}
		if (!gbInvflag) {
			*infostr = '\0';
			bool ranged = HasRangedSkill();

			switch (pcurstgt) {
			case TGT_NORMAL:
				if (currLvl._dType != DTYPE_TOWN)
					FindMonster(0, ranged);
				else
					FindTowner();
				if (pcursmonst == MON_NONE)
					FindPlayer(0, ranged);
				FindItem();
				if (pcursitem == ITEM_NONE)
					FindObject();
				FindTrigger();
				break;
			case TGT_ITEM:
				FindItem();
				break;
			case TGT_OBJECT:
				FindObject();
				break;
			case TGT_OTHER:
				assert(ranged);
				FindPlayer(1, true);
				if (pcursplr == PLR_NONE)
					FindMonster(1, true);
				break;
			case TGT_DEAD:
				assert(ranged);
				FindPlayer(2, true);
				break;
			case TGT_NONE:
				break;
			default:
				ASSUME_UNREACHABLE
			}
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

void UseBeltItem(bool manaItem)
{
	ItemStruct* pi;

	pi = myplr._pSpdList;
	for (int i = 0; i < MAXBELTITEMS; i++, pi++) {
		const int id = pi->_iMiscId;
		const int spellId = pi->_iSpell;
		if ((!manaItem && (id == IMISC_HEAL || id == IMISC_FULLHEAL || (id == IMISC_SCROLL && spellId == SPL_HEAL)))
		    || (manaItem && (id == IMISC_MANA || id == IMISC_FULLMANA))
		    || id == IMISC_REJUV || id == IMISC_FULLREJUV) {
			if (InvUseItem(INVITEM_BELT_FIRST + i))
				break;
		}
	}
}

static bool SpellHasActorTarget()
{
	int spl = myplr._pAltAtkSkill;
	if (spl == SPL_INVALID)
		spl = myplr._pAltMoveSkill;
	if (spl == SPL_TOWN || spl == SPL_TELEPORT)
		return false;

	if (spl == SPL_FIREWALL && pcursmonst != MON_NONE) {
		pcurspos.x = monsters[pcursmonst]._mx;
		pcurspos.y = monsters[pcursmonst]._my;
	}

	return pcursplr != PLR_NONE || pcursmonst != MON_NONE;
}

static void UpdateSpellTarget()
{
	if (SpellHasActorTarget())
		return;

	pcursplr = PLR_NONE;
	pcursmonst = MON_NONE;

	const PlayerStruct& player = myplr;

	int range = 1;
	if (player._pAltMoveSkill == SPL_TELEPORT)
		range = 4;

	pcurspos.x = player._pfutx + offset_x[player._pdir] * range;
	pcurspos.y = player._pfuty + offset_y[player._pdir] * range;
}

/**
 * @brief Try dropping item in all 9 possible places
 */
static void TryDropItem()
{
	pcurspos.x = myplr._pfutx + 1;
	pcurspos.y = myplr._pfuty;
	DropItem();
}

void PerformSpellAction()
{
	if (InGameMenu())
		return;

	if (gbSkillListFlag) {
		SetSkill(false, true);
		return;
	}

	if (TryIconCurs(false))
		return;
	if (pcursicon >= CURSOR_FIRSTITEM) {
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
	ItemStruct* is;

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
	if (InGameMenu())
		return;

	if (pcursicon >= CURSOR_FIRSTITEM) {
		TryDropItem();
		return;
	}
	if (pcursicon > CURSOR_HAND)
		NewCursor(CURSOR_HAND);

	if (gbInvflag) {
		CtrlUseInvItem();
		return;
	}

	if (pcurswnd == WND_BOOK) {
		CheckBookClick(false, true);
		return;
	}

	if (pcurswnd == WND_TEAM) {
		CheckTeamClick(false);
		return;
	}

	if (pcursitem != ITEM_NONE) {
		NetSendCmdLocParam1(CMD_GOTOAGETITEM, pcurspos.x, pcurspos.y, pcursitem);
	} else if (pcursobj != OBJ_NONE) {
		NetSendCmdLocParam1(CMD_OPOBJXY, pcurspos.x, pcurspos.y, pcursobj);
	} else if (pcurstrig != -1) {
		NetSendCmdLoc(CMD_WALKXY, pcurspos.x, pcurspos.y);
	}
}

DEVILUTION_END_NAMESPACE
#endif // HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
