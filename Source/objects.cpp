/**
 * @file objects.cpp
 *
 * Implementation of object functionality, interaction, spawning, loading, etc.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define DOOR_CLOSED  0
#define DOOR_OPEN    1
#define DOOR_BLOCKED 2
#define TRAP_ACTIVE   0
#define TRAP_INACTIVE 1
#define FLAMETRAP_DIR_X 2
#define FLAMETRAP_DIR_Y 0
#define FLAMETRAP_FIRE_ACTIVE    1
#define FLAMETRAP_FIRE_INACTIVE  0
#define FLAMETRAP_ACTIVE_FRAME   1
#define FLAMETRAP_INACTIVE_FRAME 2
#define VILE_CIRCLE_TARGET_NONE   0
#define VILE_CIRCLE_TARGET_A      1
#define VILE_CIRCLE_TARGET_B      2
#define VILE_CIRCLE_TARGET_CENTER 4

int trapid;
BYTE *pObjCels[NUM_OFILE_TYPES] = { 0 };
int objectactive[MAXOBJECTS];
/** Specifies the number of active objects. */
int nobjects;
int leverid;
int objectavail[MAXOBJECTS];
ObjectStruct object[MAXOBJECTS];
//bool gbInitObjFlag;

/** Specifies the X-coordinate delta between barrels. */
const int bxadd[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
/** Specifies the Y-coordinate delta between barrels. */
const int byadd[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
/** Maps from shrine_id to shrine name. */
const char *const shrinestrs[NUM_SHRINETYPE] = {
	"Hidden",
	"Gloomy",
	"Weird",
	"Religious",
	"Magical",
	"Stone",
	"Creepy",
	"Thaumaturgic",
	"Fascinating",
	"Shimmering",
	"Cryptic",
	"Eldritch",
	"Eerie",
	"Spooky",
	"Quiet",
	"Divine",
	"Holy",
	"Sacred",
	"Ornate",
	"Spiritual",
	"Secluded",
	"Glimmering",
	"Tainted",
	"Glistening",
#ifdef HELLFIRE
	"Sparkling",
	"Solar",
	"Murphy's",
#endif
};
/**
 * Specifies the game type for which each shrine may appear.
 * SHRINETYPE_ANY - 0 - sp & mp
 * SHRINETYPE_SINGLE - 1 - sp only
 * SHRINETYPE_MULTI - 2 - mp only
 */
const BYTE shrineavail[NUM_SHRINETYPE] = {
	SHRINETYPE_ANY,    // SHRINE_HIDDEN
	SHRINETYPE_ANY,    // SHRINE_GLOOMY
	SHRINETYPE_ANY,    // SHRINE_WEIRD
	SHRINETYPE_ANY,    // SHRINE_RELIGIOUS
	SHRINETYPE_ANY,    // SHRINE_MAGICAL
	SHRINETYPE_ANY,    // SHRINE_STONE
	SHRINETYPE_ANY,    // SHRINE_CREEPY
	SHRINETYPE_SINGLE, // SHRINE_THAUMATURGIC
	SHRINETYPE_ANY,    // SHRINE_FASCINATING
	SHRINETYPE_ANY,    // SHRINE_SHIMMERING
	SHRINETYPE_ANY,    // SHRINE_CRYPTIC
	SHRINETYPE_ANY,    // SHRINE_ELDRITCH
	SHRINETYPE_MULTI,  // SHRINE_EERIE
	SHRINETYPE_MULTI,  // SHRINE_SPOOKY
	SHRINETYPE_MULTI,  // SHRINE_QUIET
	SHRINETYPE_ANY,    // SHRINE_DIVINE
	SHRINETYPE_ANY,    // SHRINE_HOLY
	SHRINETYPE_ANY,    // SHRINE_SACRED
	SHRINETYPE_ANY,    // SHRINE_ORNATE
	SHRINETYPE_ANY,    // SHRINE_SPIRITUAL
	SHRINETYPE_ANY,    // SHRINE_SECLUDED
	SHRINETYPE_ANY,    // SHRINE_GLIMMERING
	SHRINETYPE_MULTI,  // SHRINE_TAINTED
	SHRINETYPE_ANY,    // SHRINE_GLISTENING
#ifdef HELLFIRE
	SHRINETYPE_ANY,    // SHRINE_SPARKLING
	SHRINETYPE_ANY,    // SHRINE_SOLAR
	SHRINETYPE_ANY,    // SHRINE_MURPHYS
#endif
};
/** Maps from book_id to book name. */
const char StoryBookName[][28] = {
	"The Great Conflict",
	"The Wages of Sin are War",
	"The Tale of the Horadrim",
	"The Dark Exile",
	"The Sin War",
	"The Binding of the Three",
	"The Realms Beyond",
	"Tale of the Three",
	"The Black King",
#ifdef HELLFIRE
	"Journal: The Ensorcellment",
	"Journal: The Meeting",
	"Journal: The Tirade",
	"Journal: His Power Grows",
	"Journal: NA-KRUL",
	"Journal: The End",
	"A Spellbook",
#endif
};
/** Specifies the speech IDs of each dungeon type narrator book. */
const int StoryText[3][3] = {
	{ TEXT_BOOK11, TEXT_BOOK12, TEXT_BOOK13 },
	{ TEXT_BOOK21, TEXT_BOOK22, TEXT_BOOK23 },
	{ TEXT_BOOK31, TEXT_BOOK32, TEXT_BOOK33 }
};

const int flickers[1][32] = {
	{ 1, 1, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 1 },
	//{ 0, 0, 0, 0, 0, 0, 1, 1, 1 }
};

void InitObjectGFX()
{
	const ObjDataStruct *ods;
	bool themeload[NUM_THEMES];
	bool fileload[NUM_OFILE_TYPES];
	char filestr[32];
	const char* const *objlist;
	int i;

	static_assert(false == 0, "InitObjectGFX fills fileload and themeload with 0 instead of false values.");
	memset(fileload, 0, sizeof(fileload));
	memset(themeload, 0, sizeof(themeload));

	for (i = 0; i < numthemes; i++)
		themeload[themes[i].ttype] = true;

	BYTE lvlMask = 1 << currLvl._dDunType; // TODO use dType instead?
	for (i = 0; i < NUM_OBJECTS; i++) {
		ods = &AllObjects[i];
		if ((ods->oload == 1 && (ods->oLvlTypes & lvlMask))
		 || (ods->otheme != THEME_NONE && themeload[ods->otheme])
		 || (ods->oquest != Q_INVALID && QuestStatus(ods->oquest))) {
			fileload[ods->ofindex] = true;
		}
	}

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_NEST)
		objlist = ObjNestLoadList;
	else if (currLvl._dType == DTYPE_CRYPT)
		objlist = ObjCryptLoadList;
	else
#endif
		objlist = ObjMasterLoadList;

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		if (fileload[i]) {
			snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", objlist[i]);
			pObjCels[i] = LoadFileInMem(filestr);
		}
	}
}

void FreeObjectGFX()
{
	int i;

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		MemFreeDbg(pObjCels[i]);
	}
}

/**
 * Check the location if an object can be placed there in the init phase.
 * Must not consider the player's position, since it is already initialized
 * and messes up the pseudo-random generated dungeon.
 */
static bool RndLocOk(int xp, int yp)
{
	if ((dMonster[xp][yp] | /*dPlayer[xp][yp] |*/ dObject[xp][yp]
	 | nSolidTable[dPiece[xp][yp]] | (dFlags[xp][yp] & BFLAG_POPULATED)) != 0)
		return false;
	// TODO: use dType instead?
	if (currLvl._dDunType != DTYPE_CATHEDRAL || dPiece[xp][yp] <= 126 || dPiece[xp][yp] >= 144)
		return true;
	return false;
}

static bool RndLoc3x3(int *x, int *y)
{
	int xp, yp, i, j, tries;

	tries = 0;
	while (TRUE) {
		xp = random_(140, DSIZEX) + DBORDERX;
		yp = random_(140, DSIZEY) + DBORDERY;
		for (i = -1; i <= 1; i++) {
			for (j = -1; j <= 1; j++) {
				if (!RndLocOk(xp + i, yp + j))
					goto fail;
			}
		}
		*x = xp;
		*y = yp;
		return true;
fail:
		if (++tries > 20000)
			break;
	}
	return false;
}

static bool RndLoc3x4(int *x, int *y)
{
	int xp, yp, i, j, tries;

	tries = 0;
	while (TRUE) {
		xp = random_(140, DSIZEX) + DBORDERX;
		yp = random_(140, DSIZEY) + DBORDERY;
		for (i = -1; i <= 1; i++) {
			for (j = -2; j <= 1; j++) {
				if (!RndLocOk(xp + i, yp + j))
					goto fail;
			}
		}
		*x = xp;
		*y = yp;
		return true;
fail:
		if (++tries > 20000)
			break;
	}
	return false;
}

static bool RndLoc5x5(int *x, int *y)
{
	int xp, yp, i, j, tries;

	tries = 0;
	while (TRUE) {
		xp = random_(140, DSIZEX) + DBORDERX;
		yp = random_(140, DSIZEY) + DBORDERY;
		for (i = -2; i <= 2; i++) {
			for (j = -2; j <= 2; j++) {
				if (!RndLocOk(xp + i, yp + j))
					goto fail;
			}
		}
		*x = xp;
		*y = yp;
		return true;
fail:
		if (++tries > 20000)
			break;
	}
	return false;
}

static bool RndLoc7x5(int *x, int *y)
{
	int xp, yp, i, j, tries;

	tries = 0;
	while (TRUE) {
		xp = random_(140, DSIZEX) + DBORDERX;
		yp = random_(140, DSIZEY) + DBORDERY;
		for (i = -3; i <= 3; i++) {
			for (j = -2; j <= 2; j++) {
				if (!RndLocOk(xp + i, yp + j))
					goto fail;
			}
		}
		*x = xp;
		*y = yp;
		return true;
fail:
		if (++tries > 20000)
			break;
	}
	return false;
}

static bool RndLoc6x7(int *x, int *y)
{
	int xp, yp, i, j, tries;

	tries = 0;
	while (TRUE) {
		xp = random_(140, DSIZEX) + DBORDERX;
		yp = random_(140, DSIZEY) + DBORDERY;
		for (i = -2; i <= 3; i++) {
			for (j = -3; j <= 3; j++) {
				if (!RndLocOk(xp + i, yp + j))
					goto fail;
			}
		}
		*x = xp;
		*y = yp;
		return true;
fail:
		if (++tries > 20000)
			break;
	}
	return false;
}

static void InitRndLocObj(int min, int max, int objtype)
{
	int i, xp, yp, numobjs;

	numobjs = RandRange(min, max);
	for (i = 0; i < numobjs; i++) {
		if (!RndLoc3x3(&xp, &yp))
			break;
		AddObject(objtype, xp, yp);
	}
}

static void InitRndSarcs()
{
	int i, xp, yp, numobjs;

	numobjs = RandRange(10, 15);
	for (i = 0; i < numobjs; i++) {
		if (!RndLoc3x4(&xp, &yp))
			break;
		AddObject(OBJ_SARC, xp, yp);
	}
}

static void InitRndLocObj5x5(int objtype)
{
	int xp, yp;

	if (RndLoc5x5(&xp, &yp))
		AddObject(objtype, xp, yp);
}

static void ClrAllObjects()
{
	int i;

	memset(object, 0, sizeof(object));
	memset(objectactive, 0, sizeof(objectactive));

	nobjects = 0;
	for (i = 0; i < MAXOBJECTS; i++)
		objectavail[i] = i;

	trapid = 1;
	leverid = 1;
}

static void AddCandles()
{
	int tx, ty;

	tx = quests[Q_PWATER]._qtx;
	ty = quests[Q_PWATER]._qty;
	AddObject(OBJ_STORYCANDLE, tx - 2, ty + 1);
	AddObject(OBJ_STORYCANDLE, tx + 3, ty + 1);
	AddObject(OBJ_STORYCANDLE, tx - 1, ty + 2);
	AddObject(OBJ_STORYCANDLE, tx + 2, ty + 2);
}

static void AddBookLever(int type, int x, int y, int x1, int y1, int x2, int y2, int qn)
{
	int oi;

	if (x == -1 && !RndLoc5x5(&x, &y))
		return;

	oi = AddObject(type, x, y);
	SetObjMapRange(oi, x1, y1, x2, y2, leverid);
	leverid++;
	object[oi]._oVar6 = object[oi]._oAnimFrame + 1; // LEVER_BOOK_ANIM
	object[oi]._oVar7 = qn; // LEVER_BOOK_QUEST
}

static void InitRndBarrels()
{
	int xp, yp;
	_object_id o;
	int dir;
	int t; // number of tries of placing next barrel in current group
	int c; // number of barrels in current group
	int i;

	// generate i number of groups of barrels
	for (i = RandRange(3, 7); i != 0; i--) {
		do {
			xp = random_(143, DSIZEX) + DBORDERX;
			yp = random_(143, DSIZEY) + DBORDERY;
		} while (!RndLocOk(xp, yp));
		o = (random_(143, 4) != 0) ? OBJ_BARREL : OBJ_BARRELEX;
		AddObject(o, xp, yp);
		c = 1;
		do {
			for (t = 0; t < 3; t++) {
				dir = random_(143, 8);
				xp += bxadd[dir];
				yp += byadd[dir];
				if (RndLocOk(xp, yp))
					break;
			}
			if (t == 3)
				break;
			o = (random_(143, 5) != 0) ? OBJ_BARREL : OBJ_BARRELEX;
			AddObject(o, xp, yp);
			c++;
		} while (random_(143, c >> 1) == 0);
	}
}

void AddL1Objs(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = dPiece[i][j];
			if (pn == 270)
				AddObject(OBJ_L1LIGHT, i, j);
			if (pn == 44 || pn == 51 || pn == 214)
				AddObject(OBJ_L1LDOOR, i, j);
			if (pn == 46 || pn == 56)
				AddObject(OBJ_L1RDOOR, i, j);
		}
	}
}

#ifdef HELLFIRE
static void AddCryptObjs(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = dPiece[i][j];
			if (pn == 77)
				AddObject(OBJ_L1LDOOR, i, j);
			if (pn == 80)
				AddObject(OBJ_L1RDOOR, i, j);
		}
	}
}
#endif

void AddL2Objs(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = dPiece[i][j];
			if (pn == 13 || pn == 541)
				AddObject(OBJ_L2LDOOR, i, j);
			if (pn == 17 || pn == 542)
				AddObject(OBJ_L2RDOOR, i, j);
		}
	}
}

static void AddL3Objs(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j < y2; j++) {
		for (i = x1; i < x2; i++) {
			pn = dPiece[i][j];
			if (pn == 531)
				AddObject(OBJ_L3LDOOR, i, j);
			if (pn == 534)
				AddObject(OBJ_L3RDOOR, i, j);
		}
	}
}

static bool TorchLocOk(int xp, int yp)
{
	return (dFlags[xp][yp] & BFLAG_POPULATED) == 0;
}

static void AddL2Torches()
{
	int i, j, pn;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			if (!TorchLocOk(i, j))
				continue;

			pn = dPiece[i][j];
			if (pn == 1 && random_(145, 3) == 0)
				AddObject(OBJ_TORCHL2, i, j);

			if (pn == 5 && random_(145, 3) == 0)
				AddObject(OBJ_TORCHR2, i, j);

			if (pn == 37 && random_(145, 10) == 0 && dObject[i - 1][j] == 0)
				AddObject(OBJ_TORCHL, i - 1, j);

			if (pn == 41 && random_(145, 10) == 0 && dObject[i][j - 1] == 0)
				AddObject(OBJ_TORCHR, i, j - 1);
		}
	}
}

static void AddObjTraps()
{
	int i, j, oi, tx, ty, on;
	int rndv;

	rndv = 10 + (currLvl._dLevel >> 1);
	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			oi = dObject[i][j];
			if (oi <= 0)
				continue;

			oi--;
			if (!AllObjects[object[oi]._otype].oTrapFlag)
				continue;

			if (random_(144, 100) >= rndv)
				continue;
			if (random_(144, 2) == 0) {
				tx = i - 1;
				while (!nSolidTable[dPiece[tx][j]])
					tx--;

				if (i - tx <= 1)
					continue;

				ty = j;
				on = OBJ_TRAPL;
			} else {
				ty = j - 1;
				while (!nSolidTable[dPiece[i][ty]])
					ty--;

				if (j - ty <= 1)
					continue;

				tx = i;
				on = OBJ_TRAPR;
			}
			if (dFlags[tx][ty] & BFLAG_POPULATED)
				continue;
			if (!(pieceFlags[dPiece[tx][ty]] & PFLAG_TRAP_SOURCE))
				continue;
			on = AddObject(on, tx, ty);
			if (on == -1)
				return;
			object[oi]._oTrapFlag = TRUE;
			object[on]._oVar1 = oi; // TRAP_OI_REF
		}
	}
}

static void AddChestTraps()
{
	int i, j, r;
	char oi;

	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			oi = dObject[i][j];
			if (oi > 0) {
				oi--;
				if (object[oi]._otype >= OBJ_CHEST1 && object[oi]._otype <= OBJ_CHEST3 && !object[oi]._oTrapFlag && random_(0, 100) < 10) {
					object[oi]._otype += OBJ_TCHEST1 - OBJ_CHEST1;
					object[oi]._oTrapFlag = TRUE;
					if (currLvl._dType == DTYPE_CATACOMBS) {
						r = random_(0, 2);
					} else {
#ifdef HELLFIRE
						r = random_(0, 4);
#else
						r = random_(0, 3);
#endif
					}

					switch (r) {
					case 0:
						r = MIS_ARROW;
						break;
					case 1:
						r = MIS_FIREBOLT;
						break;
					case 2:
						r = MIS_LIGHTNOVAC;
						break;
#ifdef HELLFIRE
					case 3:
						r = MIS_FIRERING;
						break;
					/*case 4:
						mtype = MIS_STEALPOTS;
						break;
					case 5:
						mtype = MIS_MANATRAP;
						break;*/
					default:
						ASSUME_UNREACHABLE;
#endif
					}
					object[oi]._oVar4 = r; // CHEST_TRAP_TYPE

				}
			}
		}
	}
}

static void LoadMapSetObjects(const char *map, int startx, int starty, int x1, int y1, int w, int h, int leveridx)
{
	BYTE *pMap = LoadFileInMem(map);
	int rw, rh, i, j, oi, x2, y2, mapoff;
	BYTE *lm;

	//gbInitObjFlag = true;

	rw = pMap[0];
	rh = pMap[2];
	mapoff = (rw * rh + 2) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += rw * 2 * rh * 2;
	lm = &pMap[mapoff];

	x2 = x1 + w;
	y2 = y1 + h;
	startx += DBORDERX;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				oi = AddObject(ObjTypeConv[*lm], i, j);
				SetObjMapRange(oi, x1, y1, x2, y2, leveridx);
			}
			lm += 2;
		}
	}
	//gbInitObjFlag = false;

	mem_free_dbg(pMap);
}

static void LoadMapSetObjs(const char *map)
{
	BYTE *pMap = LoadFileInMem(map);
	int startx = 2 * setpc_x, starty = 2 * setpc_y;
	int rw, rh;
	int i, j, mapoff;
	BYTE *lm;

	//gbInitObjFlag = true;

	rw = pMap[0];
	rh = pMap[2];
	mapoff = (rw * rh + 2) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += rw * 2 * rh * 2;
	lm = &pMap[mapoff];

	startx += DBORDERX;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				AddObject(ObjTypeConv[*lm], i, j);
			}
			lm += 2;
		}
	}
	//gbInitObjFlag = false;

	mem_free_dbg(pMap);
}

static void AddDiabObjs()
{
	LoadMapSetObjects("Levels\\L4Data\\diab1.DUN", 2 * DIAB_QUAD_1X, 2 * DIAB_QUAD_1Y, DIAB_QUAD_2X, DIAB_QUAD_2Y, 11, 12, 1);
	LoadMapSetObjects("Levels\\L4Data\\diab2a.DUN", 2 * DIAB_QUAD_2X, 2 * DIAB_QUAD_2Y, DIAB_QUAD_3X, DIAB_QUAD_3Y, 11, 11, 2);
	LoadMapSetObjects("Levels\\L4Data\\diab3a.DUN", 2 * DIAB_QUAD_3X, 2 * DIAB_QUAD_3Y, DIAB_QUAD_4X, DIAB_QUAD_4Y, 9, 9, 3);
}

#ifdef HELLFIRE
static void AddLvl2xBooks(int bookidx)
{
	int xp, yp;

	if (!RndLoc7x5(&xp, &yp))
		return;

	AddHBooks(bookidx, xp, yp);
	AddObject(OBJ_STORYCANDLE, xp - 2, yp + 1);
	AddObject(OBJ_STORYCANDLE, xp - 2, yp);
	AddObject(OBJ_STORYCANDLE, xp - 1, yp - 1);
	AddObject(OBJ_STORYCANDLE, xp + 1, yp - 1);
	AddObject(OBJ_STORYCANDLE, xp + 2, yp);
	AddObject(OBJ_STORYCANDLE, xp + 2, yp + 1);
}

static void AddUberLever()
{
	int oi;

	oi = AddObject(OBJ_LEVER, 2 * setpc_x + DBORDERX + 7, 2 * setpc_y + DBORDERY + 5);
	SetObjMapRange(oi, setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h, 1);
}

static void AddLvl24Books()
{
	BYTE books[4];

	AddUberLever();
	switch (random_(0, 6)) {
	case 0:
		books[0] = 5; books[1] = 6; books[2] = 7; books[3] = 0;
		break;
	case 1:
		books[0] = 5; books[1] = 7; books[2] = 6; books[3] = 0;
		break;
	case 2:
		books[0] = 6; books[1] = 5; books[2] = 7; books[3] = 0;
		break;
	case 3:
		books[0] = 6; books[1] = 7; books[2] = 5; books[3] = 0;
		break;
	case 4:
		books[0] = 7; books[1] = 6; books[2] = 5; books[3] = 0;
		break;
	case 5:
		books[0] = 7; books[1] = 5; books[2] = 6; books[3] = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	AddHBooks(books[0], 2 * setpc_x + DBORDERX + 7, 2 * setpc_y + DBORDERY + 6);
	AddHBooks(books[1], 2 * setpc_x + DBORDERX + 6, 2 * setpc_y + DBORDERY + 3);
	AddHBooks(books[2], 2 * setpc_x + DBORDERX + 6, 2 * setpc_y + DBORDERY + 8);
}

static int ProgressUberLever(int bookidx, int status)
{
	if (status >= 4)
		return status;

	switch (bookidx) {
	case 6:
		return 1;
	case 7:
		return status == 1 ? 2 : 0;
	case 8:
		return status == 2 ? 3 : 0;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return 0;
}
#endif

static void AddStoryBooks()
{
	int xp, yp;

	if (!RndLoc7x5(&xp, &yp))
		return;

	AddObject(OBJ_STORYBOOK, xp, yp);
	AddObject(OBJ_STORYCANDLE, xp - 2, yp + 1);
	AddObject(OBJ_STORYCANDLE, xp - 2, yp);
	AddObject(OBJ_STORYCANDLE, xp - 1, yp - 1);
	AddObject(OBJ_STORYCANDLE, xp + 1, yp - 1);
	AddObject(OBJ_STORYCANDLE, xp + 2, yp);
	AddObject(OBJ_STORYCANDLE, xp + 2, yp + 1);
}

static void AddHookedBodies(int freq)
{
	int i, j, ii, jj;

	for (j = 0; j < DMAXY; j++) {
		jj = DBORDERY + j * 2;
		for (i = 0; i < DMAXX; i++) {
			ii = DBORDERX + i * 2;
			if (dungeon[i][j] != 1 && dungeon[i][j] != 2)
				continue;
			if (random_(0, freq) != 0)
				continue;
			if (!SkipThemeRoom(i, j))
				continue;
			if (dungeon[i][j] == 1 && dungeon[i + 1][j] == 6) {
				switch (random_(0, 3)) {
				case 0:
					AddObject(OBJ_TORTURE1, ii + 1, jj);
					break;
				case 1:
					AddObject(OBJ_TORTURE2, ii + 1, jj);
					break;
				case 2:
					AddObject(OBJ_TORTURE5, ii + 1, jj);
					break;
				default:
					ASSUME_UNREACHABLE
					break;
				}
			} else if (dungeon[i][j] == 2 && dungeon[i][j + 1] == 6) {
				if (random_(0, 2) == 0) {
					AddObject(OBJ_TORTURE3, ii, jj);
				} else {
					AddObject(OBJ_TORTURE4, ii, jj);
				}
			}
		}
	}
}

static void AddL4Goodies()
{
	AddHookedBodies(6);
	InitRndLocObj(2, 6, OBJ_TNUDEM1);
	InitRndLocObj(2, 6, OBJ_TNUDEM2);
	InitRndLocObj(2, 6, OBJ_TNUDEM3);
	InitRndLocObj(2, 6, OBJ_TNUDEM4);
	InitRndLocObj(2, 6, OBJ_TNUDEW1);
	InitRndLocObj(2, 6, OBJ_TNUDEW2);
	InitRndLocObj(2, 6, OBJ_TNUDEW3);
	InitRndLocObj(2, 6, OBJ_DECAP);
	InitRndLocObj(1, 3, OBJ_CAULDRON);
}

static void AddLazStand()
{
	int xp, yp;

	if (gbMaxPlayers != 1) {
		AddObject(OBJ_ALTBOY, 2 * setpc_x + DBORDERX + 4, 2 * setpc_y + DBORDERY + 6);
		return;
	}
	if (!RndLoc6x7(&xp, &yp)) {
		InitRndLocObj(1, 1, OBJ_LAZSTAND);
		return;
	}
	AddObject(OBJ_LAZSTAND, xp, yp);
	AddObject(OBJ_TNUDEM2, xp, yp + 2);
	AddObject(OBJ_STORYCANDLE, xp + 1, yp + 2);
	AddObject(OBJ_TNUDEM3, xp + 2, yp + 2);
	AddObject(OBJ_TNUDEW1, xp, yp - 2);
	AddObject(OBJ_STORYCANDLE, xp + 1, yp - 2);
	AddObject(OBJ_TNUDEW2, xp + 2, yp - 2);
	AddObject(OBJ_STORYCANDLE, xp - 1, yp - 1);
	AddObject(OBJ_TNUDEW3, xp - 1, yp);
	AddObject(OBJ_STORYCANDLE, xp - 1, yp + 1);
}

void InitObjects()
{
	ClrAllObjects();

	//gbInitObjFlag = true;
	switch (currLvl._dType) {
	case DTYPE_CATHEDRAL:
		if (currLvl._dLevelIdx == DLV_CATHEDRAL4)
			AddStoryBooks();
		if (QuestStatus(Q_BUTCHER))
			LoadMapSetObjs("Levels\\L1Data\\Butcher.DUN");
		if (QuestStatus(Q_PWATER))
			AddCandles();
		if (QuestStatus(Q_LTBANNER))
			AddObject(OBJ_SIGNCHEST, 2 * setpc_x + DBORDERX + 10, 2 * setpc_y + DBORDERY + 3);
		InitRndSarcs();
		AddL1Objs(0, 0, MAXDUNX, MAXDUNY);
		break;
	case DTYPE_CATACOMBS:
		if (currLvl._dLevelIdx == DLV_CATACOMBS4)
			AddStoryBooks();
		if (QuestStatus(Q_ROCK))
			InitRndLocObj5x5(OBJ_STAND);
		if (QuestStatus(Q_SCHAMB)) {
			AddBookLever(OBJ_BOOK2R, -1, 0, setpc_x, setpc_y, setpc_w + setpc_x, setpc_h + setpc_y, Q_SCHAMB);
		}
		AddL2Objs(0, 0, MAXDUNX, MAXDUNY);
		AddL2Torches();
		if (QuestStatus(Q_BLIND)) {
			AddBookLever(OBJ_BLINDBOOK, -1, 0, setpc_x, setpc_y, setpc_w + setpc_x, setpc_h + setpc_y, Q_BLIND);
			LoadMapSetObjs("Levels\\L2Data\\Blind2.DUN");
		}
		if (QuestStatus(Q_BLOOD)) {
			AddBookLever(OBJ_BLOODBOOK, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 24, setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7, Q_BLOOD);
			AddObject(OBJ_PEDISTAL, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 16);
		}
		break;
	case DTYPE_CAVES:
		if (QuestStatus(Q_MUSHROOM))
			AddMushPatch();
		if (currLvl._dLevelIdx == DLV_CAVES1 && gbMaxPlayers == 1)
			AddSlainHero();
		else if (currLvl._dLevelIdx == DLV_CAVES4)
			AddStoryBooks();
#ifdef HELLFIRE
	case DTYPE_NEST:
#endif
		AddL3Objs(0, 0, MAXDUNX, MAXDUNY);
		break;
	case DTYPE_HELL:
		if (currLvl._dLevelIdx == DLV_HELL4) {
			AddDiabObjs();
			return;
		}
		if (QuestStatus(Q_WARLORD)) {
			AddBookLever(OBJ_STEELTOME, -1, 0, setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h, Q_WARLORD);
			LoadMapSetObjs("Levels\\L4Data\\Warlord.DUN");
		}
		if (QuestStatus(Q_BETRAYER))
			AddLazStand();
		AddL4Goodies();
	break;
#ifdef HELLFIRE
	case DTYPE_CRYPT:
		switch (currLvl._dLevelIdx) {
		case DLV_CRYPT1:
			AddLvl2xBooks(0);
			break;
		case DLV_CRYPT2:
			AddLvl2xBooks(1);
			AddLvl2xBooks(2);
			break;
		case DLV_CRYPT3:
			AddLvl2xBooks(3);
			AddLvl2xBooks(4);
			break;
		case DLV_CRYPT4:
			AddLvl24Books();
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		InitRndSarcs();
		AddCryptObjs(0, 0, MAXDUNX, MAXDUNY);
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
	InitRndBarrels();
	InitRndLocObj(5, 10, OBJ_CHEST1);
	InitRndLocObj(3, 6, OBJ_CHEST2);
	InitRndLocObj(1, 5, OBJ_CHEST3);
	// TODO: use dType instead?
	if (currLvl._dDunType != DTYPE_HELL && currLvl._dDunType != DTYPE_CAVES)
		AddObjTraps();
	// BUGFIX: TODO no traps in CRYPT?
	if (currLvl._dDunType != DTYPE_CATHEDRAL)
		AddChestTraps();
	//gbInitObjFlag = false;
}

void SetMapObjects(BYTE *pMap)
{
	int rw, rh;
	int i, j;
	BYTE *lm, *h;
	long mapoff;
	bool fileload[NUM_OFILE_TYPES];
	char filestr[32];

	ClrAllObjects();
	static_assert(false == 0, "SetMapObjects fills fileload with 0 instead of false values.");
	memset(fileload, 0, sizeof(fileload));
	//gbInitObjFlag = true;

	for (i = 0; i < NUM_OBJECTS; i++) { // TODO: use dType instead?
		if (AllObjects[i].oload == 1 && currLvl._dDunType == AllObjects[i].oSetLvlType)
			fileload[AllObjects[i].ofindex] = true;
	}

	lm = pMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	mapoff = (rw * rh + 1) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += 2 * rw * rh * 2;
	lm += mapoff;
	h = lm;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm != 0) {
				fileload[AllObjects[ObjTypeConv[*lm]].ofindex] = true;
			}
			lm += 2;
		}
	}

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		if (!fileload[i])
			continue;

		snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", ObjMasterLoadList[i]);
		pObjCels[i] = LoadFileInMem(filestr);
	}

	lm = h;
	rw += DBORDERX;
	rh += DBORDERY;
	for (j = DBORDERY; j < rh; j++) {
		for (i = DBORDERX; i < rw; i++) {
			if (*lm != 0)
				AddObject(ObjTypeConv[*lm], i, j);
			lm += 2;
		}
	}
	//gbInitObjFlag = false;
}

/*static void DeleteObject_(int oi, int idx)
{
	objectavail[MAXOBJECTS - nobjects] = oi;
	dObject[object[oi]._ox][object[oi]._oy] = 0;
	nobjects--;
	if (nobjects > 0 && idx != nobjects)
		objectactive[idx] = objectactive[nobjects];
}*/

static void SetupObject(int oi, int x, int y, int type)
{
	ObjectStruct *os;
	const ObjDataStruct *ods;

	os = &object[oi];
	os->_ox = x;
	os->_oy = y;
	os->_otype = type;
	ods = &AllObjects[type];
	os->_oAnimData = pObjCels[ods->ofindex];
	os->_oAnimFlag = ods->oAnimFlag;
	os->_oAnimFrameLen = ods->oAnimFrameLen;
	if (ods->oAnimFlag) {
		os->_oAnimCnt = random_(146, ods->oAnimFrameLen);
		os->_oAnimLen = ods->oAnimLen;
		os->_oAnimFrame = RandRange(1, ods->oAnimLen);
	} else {
		os->_oAnimCnt = 0;
		os->_oAnimLen = ods->oAnimLen;
		os->_oAnimFrame = ods->oAnimBaseFrame;
	}
	os->_oAnimWidth = ods->oAnimWidth;
	os->_oAnimXOffset = (os->_oAnimWidth - 64) >> 1;
	os->_oSolidFlag = ods->oSolidFlag;
	os->_oMissFlag = ods->oMissFlag;
	os->_oLight = ods->oLightFlag;
	// os->_oDelFlag = FALSE; - unused
	os->_oBreak = ods->oBreak;
	os->_oSelFlag = ods->oSelFlag;
	static_assert(FALSE == 0, "SetupObject expects the objects to be zero-filled and skips a few initialization steps.");
	//os->_oPreFlag = FALSE;
	//os->_oTrapFlag = FALSE;
	//os->_oDoorFlag = FALSE;
}

void SetObjMapRange(int oi, int x1, int y1, int x2, int y2, int v)
{
	ObjectStruct *os;

	os = &object[oi];
	// LEVER_EFFECT
	os->_oVar1 = x1;
	os->_oVar2 = y1;
	os->_oVar3 = x2;
	os->_oVar4 = y2;
	// LEVER_INDEX
	os->_oVar8 = v;
}

static void AddChest(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (random_(147, 2) == 0)
		os->_oAnimFrame += 3;
	os->_oRndSeed = GetRndSeed();
	// CHEST_ITEM_NUM
	switch (os->_otype) {
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		os->_oVar1 = currLvl._dSetLvl ? 1 : random_(147, 2);
		break;
	case OBJ_TCHEST2:
	case OBJ_CHEST2:
		os->_oVar1 = currLvl._dSetLvl ? 2 : random_(147, 3);
		break;
	case OBJ_TCHEST3:
	case OBJ_CHEST3:
		os->_oVar1 = currLvl._dSetLvl ? 3 : random_(147, 4);
		break;
	}
	// CHEST_ITEM_TYPE
	os->_oVar2 = random_(147, 8);
}

static void SyncL1Doors(int oi);
static void AddL1Door(int oi)
{
	ObjectStruct *os;
	int x, y;

	os = &object[oi];
	x = os->_ox;
	y = os->_oy;
	os->_oVar4 = DOOR_CLOSED;
	//os->_oPreFlag = FALSE;
	//os->_oSelFlag = 3;
	//os->_oMissFlag = FALSE;
	os->_oDoorFlag = TRUE;
	os->_oVar1 = dPiece[x][y];     // DOOR_PIECE_CLOSED
	// DOOR_BACK_PIECE_CLOSED
	if (os->_otype == OBJ_L1RDOOR)
		x--;
	else
		y--;
	os->_oVar2 = dPiece[x][y];
	//SyncL1Doors(oi);
}

static void SyncL2Doors(int oi);
static void AddL2Door(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oDoorFlag = TRUE;
	os->_oVar4 = dMonster[os->_ox][os->_oy] == 0 ? DOOR_CLOSED : DOOR_OPEN;
	if (os->_oVar4 == DOOR_OPEN) {
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
	}
	//os->_oPreFlag = FALSE;
	//os->_oSelFlag = 3;
	//os->_oMissFlag = FALSE;
	SyncL2Doors(oi);
}

static void SyncL3Doors(int oi);
static void AddL3Door(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oDoorFlag = TRUE;
	os->_oVar4 = dMonster[os->_ox][os->_oy] == 0 ? DOOR_CLOSED : DOOR_OPEN;
	if (os->_oVar4 == DOOR_OPEN) {
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
	}
	//os->_oPreFlag = FALSE;
	//os->_oSelFlag = 3;
	//os->_oMissFlag = FALSE;
	SyncL3Doors(oi);
}

static void AddSarc(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	dObject[os->_ox][os->_oy - 1] = -(oi + 1);
	os->_oVar1 = random_(153, 10);       // SARC_ITEM
	os->_oRndSeed = GetRndSeed();
	if (os->_oVar1 >= 8)
		os->_oVar2 = PreSpawnSkeleton(); // SARC_SKELE
}

static void AddFlameTrap(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = trapid; // FLAMETRAP_ID
	os->_oVar2 = TRAP_ACTIVE;
	os->_oVar3 = FLAMETRAP_DIR_Y;
	os->_oVar4 = FLAMETRAP_FIRE_INACTIVE;
}

static void AddFlameLever(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = trapid; // FLAMETRAP_ID
}

static void AddTrap(int oi)
{
	ObjectStruct *os;
	int mt;

	mt = currLvl._dLevel;
	mt = mt / 6 + 1;
	mt = random_(148, mt) & 3;
	os = &object[oi];
	// TRAP_MISTYPE
	static_assert(MIS_ARROW == 0, "AddTrap might have an 'undefined'(0) missile-type, which is expected to be a standard arrow.");
	// os->_oVar3 = MIS_ARROW;
	if (mt == 1)
		os->_oVar3 = MIS_FIREBOLT;
	else if (mt == 2)
		os->_oVar3 = MIS_LIGHTNINGC;
	os->_oVar4 = TRAP_ACTIVE;
}

static void AddObjLight(int oi, int diffr)
{
	ObjectStruct *os;

	os = &object[oi];
	//if (gbInitObjFlag) {
		if (diffr != 0)
			DoLighting(os->_ox, os->_oy, diffr, -1);
		os->_olid = -1;
	//}
}

static void AddBarrel(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	//os->_oVar1 = 0;
	os->_oRndSeed = GetRndSeed();
	os->_oVar2 = (os->_otype == OBJ_BARRELEX) ? 0 : random_(149, 10); // BARREL_ITEM
	os->_oVar3 = random_(149, 3);        // BARREL_ITEM_TYPE

	if (os->_oVar2 >= 8)
		os->_oVar4 = PreSpawnSkeleton(); // BARREL_SKELE
}

static int FindValidShrine(int filter)
{
	int rv;
	BYTE excl = gbMaxPlayers != 1 ? SHRINETYPE_SINGLE : SHRINETYPE_MULTI;

	while (TRUE) {
		rv = random_(0, NUM_SHRINETYPE);
		if (rv != filter && shrineavail[rv] != excl)
			break;
	}
	return rv;
}

static void AddShrine(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oPreFlag = TRUE;
	os->_oRndSeed = GetRndSeed();
	os->_oVar1 = FindValidShrine(NUM_SHRINETYPE); // SHRINE_TYPE
	if (random_(150, 2) != 0) {
		os->_oAnimFrame = 12;
		os->_oAnimLen = 22;
	}
}

static void AddBookcase(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
	os->_oPreFlag = TRUE;
}

static void ObjAddRndSeed(int oi)
{
	object[oi]._oRndSeed = GetRndSeed();
}

static void AddPurifyingFountain(int oi)
{
	int ox, oy;

	ox = object[oi]._ox;
	oy = object[oi]._oy;
	dObject[ox][oy - 1] = -(oi + 1);
	dObject[ox - 1][oy] = -(oi + 1);
	dObject[ox - 1][oy - 1] = -(oi + 1);
	//object[oi]._oRndSeed = GetRndSeed();
}

static void AddArmorStand(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
}

static void AddCauldronGoatShrine(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
	os->_oVar1 = FindValidShrine(SHRINE_THAUMATURGIC); // SHRINE_TYPE
}

static void AddMurkyFountain(int oi)
{
	int ox, oy;

	ox = object[oi]._ox;
	oy = object[oi]._oy;
	dObject[ox][oy - 1] = -(oi + 1);
	dObject[ox - 1][oy] = -(oi + 1);
	dObject[ox - 1][oy - 1] = -(oi + 1);
	//object[oi]._oRndSeed = GetRndSeed();
}

static void AddDecap(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
	os->_oAnimFrame = RandRange(1, 8);
	os->_oPreFlag = TRUE;
}

static void AddVileBook(int oi)
{
	if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		object[oi]._oAnimFrame = 4;
	}
}

static void AddMagicCircle(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	//os->_oRndSeed = GetRndSeed();
	os->_oPreFlag = TRUE;
	os->_oVar6 = VILE_CIRCLE_TARGET_NONE;
	os->_oVar5 = 1; // VILE_CIRCLE_PROGRESS
}

static void AddStoryBook(int oi)
{
	ObjectStruct *os;
	int bookframe;

	bookframe = quests[Q_DIABLO]._qvar1;

	os = &object[oi];
	os->_oVar1 = bookframe;
	// STORY_BOOK_MSG
	if (currLvl._dLevelIdx == DLV_CATHEDRAL4)
		os->_oVar2 = StoryText[bookframe][0];
	else if (currLvl._dLevelIdx == DLV_CATACOMBS4)
		os->_oVar2 = StoryText[bookframe][1];
	else if (currLvl._dLevelIdx == DLV_CAVES4)
		os->_oVar2 = StoryText[bookframe][2];
	os->_oVar3 = (currLvl._dLevelIdx >> 2) + 3 * bookframe - 1; // STORY_BOOK_NAME
	os->_oAnimFrame = 5 - 2 * bookframe;
	os->_oVar4 = os->_oAnimFrame + 1; // STORY_BOOK_ANIM_FRAME
}

static void AddWeaponRack(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
}

static void AddTorturedBody(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	//os->_oRndSeed = GetRndSeed();
	os->_oAnimFrame = RandRange(1, 4);
	os->_oPreFlag = TRUE;
}

void AddMushPatch()
{
	int oi;
	int xp, yp;

	if (!RndLoc5x5(&xp, &yp))
		return;
	oi = AddObject(OBJ_MUSHPATCH, xp, yp);
	oi = -(oi + 1);
	dObject[xp - 1][yp - 1] = oi;
	dObject[xp][yp - 1] = oi;
	dObject[xp - 1][yp] = oi;
}

void AddSlainHero()
{
	int xp, yp;

	if (RndLoc5x5(&xp, &yp))
		AddObject(OBJ_SLAINHERO, xp, yp);
}

#ifdef HELLFIRE
void AddHBooks(int bookidx, int ox, int oy)
{
	int oi;

	if (nobjects >= MAXOBJECTS)
		return;

	oi = objectavail[0];
	objectactive[nobjects] = oi;
	nobjects++;
	objectavail[0] = objectavail[MAXOBJECTS - nobjects];
	dObject[ox][oy] = oi + 1;
	SetupObject(oi, ox, oy, OBJ_STORYBOOK);
	SetupHBook(oi, bookidx);
}

void SetupHBook(int oi, int bookidx)
{
	ObjectStruct *os;
	int frame;

	os = &object[oi];
	os->_oVar1 = 1;
	frame = 2 * os->_oVar1;
	os->_oAnimFrame = 5 - frame;
	os->_oVar4 = os->_oAnimFrame + 1;       // STORY_BOOK_ANIM_FRAME
	if (bookidx >= 5) {
		os->_oVar2 = TEXT_BOOKA + bookidx - 5; // STORY_BOOK_MSG
		os->_oVar3 = 15;                    // STORY_BOOK_NAME
		os->_oVar8 = bookidx + 1;           // STORY_BOOK_NAKRUL_IDX
	} else {
		os->_oVar2 = TEXT_BOOK4 + bookidx;  // STORY_BOOK_MSG
		os->_oVar3 = bookidx + 10;          // STORY_BOOK_NAME
		os->_oVar8 = 0;                     // STORY_BOOK_NAKRUL_IDX
	}
}
#endif

int AddObject(int type, int ox, int oy)
{
	int oi;

	if (nobjects >= MAXOBJECTS)
		return -1;

	oi = objectavail[0];
	objectactive[nobjects] = oi;
	nobjects++;
	objectavail[0] = objectavail[MAXOBJECTS - nobjects];
	dObject[ox][oy] = oi + 1;
	SetupObject(oi, ox, oy, type);
	switch (type) {
	case OBJ_L1LIGHT:
		AddObjLight(oi, 0);
		break;
	case OBJ_SKFIRE:
	case OBJ_CANDLE1:
	case OBJ_CANDLE2:
	case OBJ_BOOKCANDLE:
		AddObjLight(oi, 5);
		break;
	case OBJ_STORYCANDLE:
		AddObjLight(oi, 3);
		break;
	case OBJ_TORCHL:
	case OBJ_TORCHR:
	case OBJ_TORCHL2:
	case OBJ_TORCHR2:
		AddObjLight(oi, 8);
		break;
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
		AddL1Door(oi);
		break;
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		AddL2Door(oi);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		AddL3Door(oi);
		break;
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
		AddChest(oi);
		break;
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		AddChest(oi);
		object[oi]._oTrapFlag = TRUE;
		object[oi]._oVar4 = random_(0, currLvl._dType == DTYPE_CATACOMBS ? 2 : 3); // CHEST_TRAP_TYPE
		break;
	case OBJ_SARC:
		AddSarc(oi);
		break;
	case OBJ_FLAMEHOLE:
		AddFlameTrap(oi);
		break;
	case OBJ_FLAMELVR:
		AddFlameLever(oi);
		break;
	case OBJ_WATER:
		object[oi]._oAnimFrame = 1;
		break;
	case OBJ_TRAPL:
	case OBJ_TRAPR:
		AddTrap(oi);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
		AddBarrel(oi);
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		AddShrine(oi);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		AddBookcase(oi);
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
	case OBJ_BLOODBOOK:
	case OBJ_PEDISTAL:
	case OBJ_SLAINHERO:
		ObjAddRndSeed(oi);
		break;
	//case OBJ_BLOODFTN:
	//	ObjAddRndSeed(oi);
	//	break;
	case OBJ_DECAP:
		AddDecap(oi);
		break;
	case OBJ_PURIFYINGFTN:
		AddPurifyingFountain(oi);
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR:
		AddArmorStand(oi);
		break;
	case OBJ_GOATSHRINE:
	case OBJ_CAULDRON:
		AddCauldronGoatShrine(oi);
		break;
	case OBJ_MURKYFTN:
		AddMurkyFountain(oi);
		break;
	//case OBJ_TEARFTN:
	//	ObjAddRndSeed(oi);
	//	break;
	case OBJ_BOOK2L:
		AddVileBook(oi);
		break;
	case OBJ_MCIRCLE1:
	case OBJ_MCIRCLE2:
		AddMagicCircle(oi);
		break;
	case OBJ_STORYBOOK:
		AddStoryBook(oi);
		break;
	case OBJ_BCROSS:
	case OBJ_TBCROSS:
		// ObjAddRndSeed(oi);
		AddObjLight(oi, 10);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACKL:
	case OBJ_WEAPONRACKR:
		AddWeaponRack(oi);
		break;
	case OBJ_TNUDEM2:
		AddTorturedBody(oi);
		break;
	}
	return oi;
}

static void Obj_Light(int oi)
{
	ObjectStruct *os;
	int ox, oy, dx, dy, pnum, tr;
	bool turnon;

	const int lr = 8;
	const int *flicker = flickers[0];

	os = &object[oi];
	ox = os->_ox;
	oy = os->_oy;
	tr = lr + 10;
	turnon = false;
#ifdef _DEBUG
	if (!lightflag)
#endif
	{
		for (pnum = 0; pnum < MAX_PLRS && !turnon; pnum++) {
			if (plr.plractive && currLvl._dLevelIdx == plr.plrlevel) {
				dx = abs(plr._px - ox);
				dy = abs(plr._py - oy);
				if (dx < tr && dy < tr)
					turnon = true;
			}
		}
	}
	if (turnon) {
		tr -= 10;
		tr += flicker[os->_oAnimFrame];
		if (os->_olid == -1)
			os->_olid = AddLight(ox, oy, tr);
		else {
			if (LightList[os->_olid]._lradius != tr)
				ChangeLight(os->_olid, ox, oy, tr);
		}
	} else {
		if (os->_olid != -1) {
			AddUnLight(os->_olid);
			os->_olid = -1;
		}
	}
}

static void GetVileMissPos(int *dx, int *dy)
{
	int xx, yy, k, j, i;

	i = dObject[*dx][*dy] - 1;
	assert(object[i]._otype == OBJ_MCIRCLE1 || object[i]._otype == OBJ_MCIRCLE2);

	for (k = 0; k < 50; k++) {
		for (j = -k; j <= k; j++) {
			yy = j + *dy;
			for (i = -k; i <= k; i++) {
				xx = i + *dx;
				if (PosOkPlayer(mypnum, xx, yy)) {
					*dx = xx;
					*dy = yy;
					return;
				}
			}
		}
	}
}

static void Obj_Circle(int oi)
{
	ObjectStruct *os;
	int ox, oy;

	os = &object[oi];
	ox = os->_ox;
	oy = os->_oy;
	if (myplr._px == ox && myplr._py == oy) {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oAnimFrame = 2;
		else {
			assert(os->_otype == OBJ_MCIRCLE2);
			os->_oAnimFrame = 4;
		}
		if (ox == DBORDERX + 29 && oy == DBORDERY + 31) {
			os->_oVar6 = VILE_CIRCLE_TARGET_B;
		} else if (ox == DBORDERX + 10 && oy == DBORDERY + 30) {
			os->_oVar6 = VILE_CIRCLE_TARGET_A;
		} else if (ox == DBORDERX + 19 && oy == DBORDERY + 20 && os->_oVar5 == 3) { // VILE_CIRCLE_PROGRESS
			os->_oVar6 = VILE_CIRCLE_TARGET_CENTER;
			ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4); // LEVER_EFFECT
			if (/*quests[Q_BETRAYER]._qactive == QUEST_ACTIVE &&*/ quests[Q_BETRAYER]._qvar1 < 4) // BUGFIX stepping on the circle again will break the quest state (fixed)
				quests[Q_BETRAYER]._qvar1 = 4;
			int dx = 0, dy = 0;
			//if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
			assert(currLvl._dLevelIdx == SL_VILEBETRAYER);
				dx = DBORDERX + 19; dy = DBORDERY + 30;
				GetVileMissPos(&dx, &dy);
			//}
			AddMissile(ox, oy, dx, dy, 0, MIS_RNDTELEPORT, -1, mypnum, 0, 0, 0);
			gbActionBtnDown = false;
			gbAltActionBtnDown = false;
			ClrPlrPath(mypnum);
			PlrStartStand(mypnum, 0);
		} else {
			os->_oVar6 = VILE_CIRCLE_TARGET_NONE;
		}
	} else {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oAnimFrame = 1;
		else {
			assert(os->_otype == OBJ_MCIRCLE2);
			os->_oAnimFrame = 3;
		}
		os->_oVar6 = VILE_CIRCLE_TARGET_NONE;
	}
}

static void Obj_StopAnim(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oAnimFrame == os->_oAnimLen) {
		os->_oAnimFlag = FALSE;
	}
}

static void Obj_Door(int oi)
{
	ObjectStruct *os;
	int dx, dy;
	bool dok;

	os = &object[oi];
	if (os->_oVar4 == DOOR_CLOSED)
		return;
	dx = os->_ox;
	dy = os->_oy;
	dok = (dMonster[dx][dy] | dItem[dx][dy] | dDead[dx][dy] | dPlayer[dx][dy]) == 0;
	os->_oVar4 = dok ? DOOR_OPEN : DOOR_BLOCKED;
}

static void ActivateTrapLine(int tid)
{
	ObjectStruct *os;
	int i;

	for (i = 0; i < nobjects; i++) {
		os = &object[objectactive[i]];
		if (os->_otype == OBJ_FLAMEHOLE && os->_oVar1 == tid) { // FLAMETRAP_ID
			os->_oVar4 = FLAMETRAP_FIRE_ACTIVE;
			os->_oAnimFlag = TRUE;
			//os->_oAnimFrameLen = 1;
			os->_olid = AddLight(os->_ox, os->_oy, 1);
		}
	}
}

static void Obj_FlameTrap(int oi)
{
	ObjectStruct *os;
	int x, y;
	int i;

	os = &object[oi];
	if (os->_oVar2 != TRAP_ACTIVE) {
		if (os->_oVar4 != FLAMETRAP_FIRE_INACTIVE) {
			os->_oAnimFrame--;
			if (os->_oAnimFrame == 1) {
				os->_oVar4 = FLAMETRAP_FIRE_INACTIVE;
				AddUnLight(os->_olid);
			} else if (os->_oAnimFrame <= 4) {
				ChangeLightRadius(os->_olid, os->_oAnimFrame);
			}
		}
	} else if (os->_oVar4 == FLAMETRAP_FIRE_INACTIVE) {
		if (os->_oVar3 == FLAMETRAP_DIR_X) {
			x = os->_ox - 2;
			y = os->_oy;
			for (i = 0; i < 5; i++) {
				if ((dPlayer[x][y] | dMonster[x][y]) != 0)
					os->_oVar4 = FLAMETRAP_FIRE_ACTIVE;
				x++;
			}
		} else {
			x = os->_ox;
			y = os->_oy - 2;
			for (i = 0; i < 5; i++) {
				if ((dPlayer[x][y] | dMonster[x][y]) != 0)
					os->_oVar4 = FLAMETRAP_FIRE_ACTIVE;
				y++;
			}
		}
		if (os->_oVar4 != FLAMETRAP_FIRE_INACTIVE)
			ActivateTrapLine(os->_oVar1); // FLAMETRAP_ID
	} else {
		if (os->_oAnimFrame == os->_oAnimLen)
			os->_oAnimFrame = 11;
		if (os->_oAnimFrame == 11)
			AddMissile(os->_ox, os->_oy, 0, 0, 0, MIS_FIRETRAP, -1, -1, 0, 0, 0);
		else if (os->_oAnimFrame <= 5)
			ChangeLightRadius(os->_olid, os->_oAnimFrame);
	}
}

static void Obj_Trap(int oi)
{
	ObjectStruct *os, *on;
	int i, dir, trigNum;
	const POS32 *trigArea;
	const POS32 baseTrigArea[] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 0 } , { -1, 1 }, { 0, -1 }, { -1, -1 }, { 1, -1 } };
	const POS32 sarcTrigArea[] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 0 } , { -1, 1 }, { 0, -2 }, { -1, -2 }, { 1, -2 }, { -1, -1 }, { 1, -1 } };

	int sx, sy, dx, dy, x, y;

	os = &object[oi];
	if (os->_oVar4 != TRAP_ACTIVE)
		return;

	trigNum = 0;
	on = &object[os->_oVar1]; // TRAP_OI_REF
	switch (on->_otype) {
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		if (on->_oVar4 != DOOR_CLOSED) {
			trigArea = baseTrigArea;
			trigNum = lengthof(baseTrigArea);
		}
		break;
	case OBJ_LEVER:
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_SWITCHSKL:
		if (on->_oSelFlag == 0) {
			trigArea = baseTrigArea;
			trigNum = lengthof(baseTrigArea);
		}
		break;
	case OBJ_SARC:
		if (on->_oSelFlag == 0) {
			trigArea = sarcTrigArea;
			trigNum = lengthof(sarcTrigArea);
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (trigNum == 0)
		return;

	os->_oVar4 = TRAP_INACTIVE;
	on->_oTrapFlag = FALSE;

	sx = on->_ox;
	sy = on->_oy;
	PlaySfxLoc(IS_TRAP, sx, sy);

	dx = sx;
	dy = sy;
	for (i = 0; i < trigNum; i++) {
		x = sx + trigArea[i].x;
		y = sy + trigArea[i].y;
		if (dPlayer[x][y] != 0) {
			dx = x;
			dy = y;
		}
	}

	sx = os->_ox;
	sy = os->_oy;
	dir = GetDirection(sx, sy, dx, dy);
	AddMissile(sx, sy, dx, dy, dir, os->_oVar3, 1, -1, 0, 0, 0); // TRAP_MISTYPE

	NetSendCmdParam1(false, CMD_TRAPDISABLE, oi);
}

static void Obj_BCrossDamage(int oi)
{
	PlayerStruct *p;
	int fire_resist, damage;

	p = &myplr;
	if (p->_pInvincible)
		return;
	if (p->_px != object[oi]._ox || p->_py != object[oi]._oy - 1)
		return;

	damage = 4 + (currLvl._dLevel >> 2);
	fire_resist = p->_pFireResist;
	if (fire_resist > 0)
		damage -= fire_resist * damage / 100;

	if (!PlrDecHp(mypnum, damage, DMGTYPE_NPC))
		PlaySfxLoc(sgSFXSets[SFXS_PLR_68][p->_pClass], p->_px, p->_py);
}

void ProcessObjects()
{
	int i, oi;

	for (i = 0; i < nobjects; ++i) {
		oi = objectactive[i];
		switch (object[oi]._otype) {
		case OBJ_L1LIGHT:
			Obj_Light(oi);
			break;
		/*case OBJ_SKFIRE:
		case OBJ_CANDLE1:
		case OBJ_CANDLE2:
		case OBJ_BOOKCANDLE:
			Obj_Light(oi, 5);
			break;
		case OBJ_STORYCANDLE:
			Obj_Light(oi, 3);
			break;*/
		case OBJ_CRUX1:
		case OBJ_CRUX2:
		case OBJ_CRUX3:
		case OBJ_SARC:
		case OBJ_BARREL:
		case OBJ_BARRELEX:
		case OBJ_SHRINEL:
		case OBJ_SHRINER:
			Obj_StopAnim(oi);
			break;
		case OBJ_L1LDOOR:
		case OBJ_L1RDOOR:
		case OBJ_L2LDOOR:
		case OBJ_L2RDOOR:
		case OBJ_L3LDOOR:
		case OBJ_L3RDOOR:
			Obj_Door(oi);
			break;
		/*case OBJ_TORCHL:
		case OBJ_TORCHR:
		case OBJ_TORCHL2:
		case OBJ_TORCHR2:
			Obj_Light(oi, 5, flickers[1]);
			break;*/
		case OBJ_FLAMEHOLE:
			Obj_FlameTrap(oi);
			break;
		case OBJ_TRAPL:
		case OBJ_TRAPR:
			Obj_Trap(oi);
			break;
		case OBJ_MCIRCLE1:
		case OBJ_MCIRCLE2:
			Obj_Circle(oi);
			break;
		case OBJ_BCROSS:
		case OBJ_TBCROSS:
			//Obj_Light(oi, 5);
			Obj_BCrossDamage(oi);
			break;
		}
		if (!object[oi]._oAnimFlag)
			continue;

		object[oi]._oAnimCnt++;

		if (object[oi]._oAnimCnt < object[oi]._oAnimFrameLen)
			continue;

		object[oi]._oAnimCnt = 0;
		object[oi]._oAnimFrame++;
		if (object[oi]._oAnimFrame > object[oi]._oAnimLen)
			object[oi]._oAnimFrame = 1;
	}
	/*for (i = 0; i < nobjects; ) {
		oi = objectactive[i];
		if (object[oi]._oDelFlag) {
			DeleteObject_(oi, i);
		} else {
			i++;
		}
	}*/
}

void ObjSetMicro(int dx, int dy, int pn)
{
	uint16_t *pPiece;
	MICROS *pMap;
	int i, blocks;

	// assert(pn != 0 && pn < MAXTILES);
	dPiece[dx][dy] = pn;
	pn--;
	pMap = &dpiece_defs_map_2[dx][dy];
	blocks = AllLevels[currLvl._dLevelIdx].dBlocks;
	pPiece = &pLevelPieces[blocks * pn];
	for (i = 0; i < blocks; i++) {
		pMap->mt[i] = SwapLE16(pPiece[(i & 1) + blocks - 2 - (i & 0xE)]);
	}
}

static void objects_set_door_piece(int dx, int dy)
{
	int pn, blocks;
	uint16_t *pPiece;
	MICROS *pMap;

	pn = dPiece[dx][dy] - 1;
	pMap = &dpiece_defs_map_2[dx][dy];
	blocks = AllLevels[currLvl._dLevelIdx].dBlocks;
	pPiece = &pLevelPieces[blocks * pn];
	pMap->mt[0] = SwapLE16(pPiece[8]);
	pMap->mt[1] = SwapLE16(pPiece[9]);
}

static void ObjSetMini(int x, int y, int v)
{
	int xx, yy;
	long v1, v2, v3, v4;
	uint16_t *MegaTiles;

	MegaTiles = (uint16_t *)&pMegaTiles[(v - 1) * 8];
	v1 = SwapLE16(*(MegaTiles + 0)) + 1;
	v2 = SwapLE16(*(MegaTiles + 1)) + 1;
	v3 = SwapLE16(*(MegaTiles + 2)) + 1;
	v4 = SwapLE16(*(MegaTiles + 3)) + 1;

	xx = 2 * x + DBORDERX;
	yy = 2 * y + DBORDERY;
	ObjSetMicro(xx, yy, v1);
	ObjSetMicro(xx + 1, yy, v2);
	ObjSetMicro(xx, yy + 1, v3);
	ObjSetMicro(xx + 1, yy + 1, v4);
}

static void ObjL1Special(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j <= y2; ++j) {
		for (i = x1; i <= x2; ++i) {
			pn = dPiece[i][j];
			if (pn == 12)
				pn = 1;
			else if (pn == 11)
				pn = 2;
			else if (pn == 71)
				pn = 1;
			else if (pn == 253)
				pn = 3;
			else if (pn == 267)
				pn = 6;
			else if (pn == 259)
				pn = 5;
			else if (pn == 249)
				pn = 2;
			else if (pn == 325)
				pn = 2;
			else if (pn == 321)
				pn = 1;
			else if (pn == 255)
				pn = 4;
			else if (pn == 211)
				pn = 1;
			else if (pn == 344)
				pn = 2;
			else if (pn == 341)
				pn = 1;
			else if (pn == 331)
				pn = 2;
			else if (pn == 418)
				pn = 1;
			else if (pn == 421)
				pn = 2;
			else
				pn = 0;
			dSpecial[i][j] = pn;
		}
	}
}

static void ObjL2Special(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			dSpecial[i][j] = 0;
			pn = dPiece[i][j];
			if (pn == 541)
				dSpecial[i][j] = 5;
			if (pn == 178)
				dSpecial[i][j] = 5;
			if (pn == 551)
				dSpecial[i][j] = 5;
			if (pn == 542)
				dSpecial[i][j] = 6;
			if (pn == 553)
				dSpecial[i][j] = 6;
		}
	}
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			pn = dPiece[i][j];
			if (pn == 132) {
				dSpecial[i][j + 1] = 2;
				dSpecial[i][j + 2] = 1;
			}
			if (pn == 135 || pn == 139) {
				dSpecial[i + 1][j] = 3;
				dSpecial[i + 2][j] = 4;
			}
		}
	}
}

static void DoorSet(int oi, int dx, int dy)
{
	int pn;

	pn = dPiece[dx][dy];
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		if (pn == 75)
			ObjSetMicro(dx, dy, 204);
		if (pn == 79)
			ObjSetMicro(dx, dy, 208);
		if (pn == 86)
			ObjSetMicro(dx, dy, object[oi]._otype == OBJ_L1LDOOR ? 232 : 234);
		if (pn == 91)
			ObjSetMicro(dx, dy, 215);
		if (pn == 93)
			ObjSetMicro(dx, dy, 218);
		if (pn == 99)
			ObjSetMicro(dx, dy, 220);
		if (pn == 111)
			ObjSetMicro(dx, dy, 222);
		if (pn == 113)
			ObjSetMicro(dx, dy, 224);
		if (pn == 115)
			ObjSetMicro(dx, dy, 226);
		if (pn == 117)
			ObjSetMicro(dx, dy, 228);
		if (pn == 119)
			ObjSetMicro(dx, dy, 230);
		if (pn == 232)
			ObjSetMicro(dx, dy, 212);
		if (pn == 234)
			ObjSetMicro(dx, dy, 212);
		return;
	}
#endif
	if (pn == 43)
		ObjSetMicro(dx, dy, 392);
	if (pn == 45)
		ObjSetMicro(dx, dy, 394);
	if (pn == 50)
		ObjSetMicro(dx, dy, object[oi]._otype == OBJ_L1LDOOR ? 411 : 412);
	if (pn == 54)
		ObjSetMicro(dx, dy, 397);
	if (pn == 55)
		ObjSetMicro(dx, dy, 398);
	if (pn == 61)
		ObjSetMicro(dx, dy, 399);
	if (pn == 67)
		ObjSetMicro(dx, dy, 400);
	if (pn == 68)
		ObjSetMicro(dx, dy, 401);
	if (pn == 69)
		ObjSetMicro(dx, dy, 403);
	if (pn == 70)
		ObjSetMicro(dx, dy, 404);
	if (pn == 72)
		ObjSetMicro(dx, dy, 406);
	if (pn == 212)
		ObjSetMicro(dx, dy, 407);
	if (pn == 354)
		ObjSetMicro(dx, dy, 409);
	if (pn == 355)
		ObjSetMicro(dx, dy, 410);
	if (pn == 411)
		ObjSetMicro(dx, dy, 396);
	if (pn == 412)
		ObjSetMicro(dx, dy, 396);
}

void RedoPlayerVision()
{
	int pnum;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr.plractive && currLvl._dLevelIdx == plr.plrlevel) {
			ChangeVisionXY(plr._pvid, plr._px, plr._py);
		}
	}
}

static void OperateL1RDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;
	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) > 1 || abs(yp - y) != 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOOROPEN, oi);
#ifdef HELLFIRE
		if (!deltaload)
			PlaySfxLoc(currLvl._dType != DTYPE_CRYPT ? IS_DOOROPEN : IS_CROPEN, xp, yp);
#else
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
#endif
		os->_oVar4 = DOOR_OPEN;
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
		SyncL1Doors(oi);
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (os->_oVar4 == DOOR_BLOCKED) {
		if (!deltaload)
			PlaySfxLoc(IS_DOORCLOS, xp, yp);
		return;
	}
	if (!deltaload)
#ifdef HELLFIRE
		PlaySfxLoc(currLvl._dType != DTYPE_CRYPT ? IS_DOORCLOS : IS_CRCLOS, xp, yp);
#else
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
#endif
	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOORCLOSE, oi);
		os->_oVar4 = DOOR_CLOSED;
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		SyncL1Doors(oi);
		RedoPlayerVision();
	} else {
		os->_oVar4 = DOOR_BLOCKED;
	}
}

static void OperateL1LDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;

	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) != 1 || abs(yp - y) > 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOOROPEN, oi);
#ifdef HELLFIRE
		if (!deltaload)
			PlaySfxLoc(currLvl._dType != DTYPE_CRYPT ? IS_DOOROPEN : IS_CROPEN, xp, yp);
#else
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
#endif
		os->_oVar4 = DOOR_OPEN;
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
		SyncL1Doors(oi);
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (os->_oVar4 == DOOR_BLOCKED) {
		if (!deltaload)
			PlaySfxLoc(IS_DOORCLOS, xp, yp);
		return;
	}
	if (!deltaload)
#ifdef HELLFIRE
		PlaySfxLoc(currLvl._dType != DTYPE_CRYPT ? IS_DOORCLOS : IS_CRCLOS, xp, yp);
#else
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
#endif
	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOORCLOSE, oi);
		os->_oVar4 = DOOR_CLOSED;
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		SyncL1Doors(oi);
		RedoPlayerVision();
	} else {
		os->_oVar4 = DOOR_BLOCKED;
	}
}

static void OperateL2RDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;
	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) > 1 || abs(yp - y) != 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOOROPEN, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		os->_oVar4 = DOOR_OPEN;
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
		SyncL2Doors(oi);
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOORCLOSE, oi);
		os->_oVar4 = DOOR_CLOSED;
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		SyncL2Doors(oi);
		RedoPlayerVision();
	} else {
		os->_oVar4 = DOOR_BLOCKED;
	}
}

static void OperateL2LDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;
	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) != 1 || abs(yp - y) > 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOOROPEN, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		os->_oVar4 = DOOR_OPEN;
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
		SyncL2Doors(oi);
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOORCLOSE, oi);
		os->_oVar4 = DOOR_CLOSED;
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		SyncL2Doors(oi);
		RedoPlayerVision();
	} else {
		os->_oVar4 = DOOR_BLOCKED;
	}
}

static void OperateL3RDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;
	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) != 1 || abs(yp - y) > 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOOROPEN, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		os->_oVar4 = DOOR_OPEN;
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
		SyncL3Doors(oi);
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOORCLOSE, oi);
		os->_oVar4 = DOOR_CLOSED;
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		SyncL3Doors(oi);
		RedoPlayerVision();
	} else {
		os->_oVar4 = DOOR_BLOCKED;
	}
}

static void OperateL3LDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;
	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) > 1 || abs(yp - y) != 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOOROPEN, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		os->_oVar4 = DOOR_OPEN;
		os->_oPreFlag = TRUE;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
		os->_oAnimFrame += 2;
		SyncL3Doors(oi);
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_DOORCLOSE, oi);
		os->_oVar4 = DOOR_CLOSED;
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		SyncL3Doors(oi);
		RedoPlayerVision();
	} else {
		os->_oVar4 = DOOR_BLOCKED;
	}
}

void MonstCheckDoors(int mx, int my)
{
	int i, j, oi;

	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++) {
			oi = dObject[mx + i][my + j];
			if (oi != 0) {
				oi = oi >= 0 ? oi - 1 : -(oi + 1);
				if (object[oi]._oVar4 != DOOR_CLOSED)
					continue;
				if (object[oi]._otype == OBJ_L1LDOOR) {
					OperateL1LDoor(mx, my, oi, true);
				} else if (object[oi]._otype == OBJ_L1RDOOR) {
					OperateL1RDoor(mx, my, oi, true);
				} else if (object[oi]._otype == OBJ_L2LDOOR) {
					OperateL2LDoor(mx, my, oi, true);
				} else if (object[oi]._otype == OBJ_L2RDOOR) {
					OperateL2RDoor(mx, my, oi, true);
				} else if (object[oi]._otype == OBJ_L3LDOOR) {
					OperateL3LDoor(mx, my, oi, true);
				} else if (object[oi]._otype == OBJ_L3RDOOR) {
					OperateL3RDoor(mx, my, oi, true);
				}
			}
		}
}

void ObjChangeMap(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			ObjSetMini(i, j, pdungeon[i][j]);
			dungeon[i][j] = pdungeon[i][j];
		}
	}
	x1 = 2 * x1 + DBORDERX;
	y1 = 2 * y1 + DBORDERY;
	x2 = 2 * x2 + DBORDERX + 1;
	y2 = 2 * y2 + DBORDERY + 1;
	if (currLvl._dType == DTYPE_CATHEDRAL) {
		ObjL1Special(x1, y1, x2, y2);
		AddL1Objs(x1, y1, x2, y2);
	} else if (currLvl._dType == DTYPE_CATACOMBS) {
		ObjL2Special(x1, y1, x2, y2);
		AddL2Objs(x1, y1, x2, y2);
	}
}

void ObjChangeMapResync(int x1, int y1, int x2, int y2)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			ObjSetMini(i, j, pdungeon[i][j]);
			dungeon[i][j] = pdungeon[i][j];
		}
	}
	x1 = 2 * x1 + DBORDERX;
	y1 = 2 * y1 + DBORDERY;
	x2 = 2 * x2 + DBORDERX + 1;
	y2 = 2 * y2 + DBORDERY + 1;
	if (currLvl._dType == DTYPE_CATHEDRAL) {
		ObjL1Special(x1, y1, x2, y2);
	} else if (currLvl._dType == DTYPE_CATACOMBS) {
		ObjL2Special(x1, y1, x2, y2);
	}
}

static void OperateLever(int oi, bool sendmsg)
{
	ObjectStruct *os, *on;
	int i;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
	if (currLvl._dLevelIdx == DLV_HELL4) {
		for (i = 0; i < nobjects; i++) {
			on = &object[objectactive[i]]; //         LEVER_INDEX
			if (on->_otype == OBJ_SWITCHSKL && os->_oVar8 == on->_oVar8 && on->_oSelFlag != 0) {
				return;
			}
		}
#ifdef HELLFIRE
	} else if (currLvl._dLevelIdx == DLV_CRYPT4 && !deltaload) {
		if (quests[Q_NAKRUL]._qactive == QUEST_DONE)
			return;
		PlaySfxLoc(IS_CROPEN, os->_ox - 3, os->_oy + 1);
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
#endif
	}
	ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4); // LEVER_EFFECT
}

static void OperateVileBook(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os, *on;
	int i;
	int dx, dy;
	bool missile_added;

	assert(currLvl._dSetLvl);

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		missile_added = false;
		for (i = 0; i < nobjects; i++) {
			on = &object[objectactive[i]];
			if (on->_otype != OBJ_MCIRCLE2)
				continue;
			if (on->_oVar6 == VILE_CIRCLE_TARGET_A) {
				dx = DBORDERX + 11;
				dy = DBORDERY + 13;
			} else if (on->_oVar6 == VILE_CIRCLE_TARGET_B) {
				dx = DBORDERX + 27;
				dy = DBORDERY + 13;
			} else {
				continue;
			}
			on->_oVar6 = VILE_CIRCLE_TARGET_CENTER;
			object[dObject[DBORDERX + 19][DBORDERY + 20] - 1]._oVar5++; // VILE_CIRCLE_PROGRESS
			GetVileMissPos(&dx, &dy);
			AddMissile(plr._px, plr._py, dx, dy, 0, MIS_RNDTELEPORT, -1, pnum, 0, 0, 0);
			missile_added = true;
		}
		if (!missile_added)
			return;
	}
	os->_oSelFlag = 0;
	os->_oAnimFrame++;

	if (currLvl._dLevelIdx == SL_BONECHAMB) {
		if (deltaload)
			return;
		if (plr._pSkillLvl[SPL_GUARDIAN] == 0) {
			plr._pSkillLvl[SPL_GUARDIAN] = 1;
			plr._pSkillExp[SPL_GUARDIAN] = SkillExpLvlsTbl[0];
			plr._pMemSkills |= SPELL_MASK(SPL_GUARDIAN);
		}
		PlaySfxLoc(IS_QUESTDN, os->_ox, os->_oy);
		if (pnum == mypnum)
			InitDiabloMsg(EMSG_BONECHAMB);
		AddMissile(
		    plr._px,
		    plr._py,
		    os->_ox - 2,
		    os->_oy - 4,
		    plr._pdir,
		    MIS_GUARDIAN,
		    0,
		    pnum,
		    0,
		    0,
		    0);
		quests[Q_SCHAMB]._qactive = QUEST_DONE;
		if (sendmsg) {
			NetSendCmdQuest(Q_SCHAMB, true); // recipient should not matter
			NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
		}
	} else if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4); // LEVER_EFFECT
		//for (i = 0; i < nobjects; i++)
		//	SyncObjectAnim(objectactive[i]);
	}
}

static void OperateBookLever(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int qn;

	if (numitems >= MAXITEMS && !deltaload) {
		return;
	}
	os = &object[oi];
	// assert(os->_oSelFlag != 0);
	qn = os->_oVar7;     // LEVER_BOOK_QUEST
	if (os->_oAnimFrame != os->_oVar6) { // LEVER_BOOK_ANIM
		os->_oAnimFrame = os->_oVar6; // LEVER_BOOK_ANIM
		if (qn != Q_BLOOD)
			ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);    // LEVER_EFFECT
		if (qn == Q_BLIND) {
			if (!deltaload)
				SpawnUnique(UITEM_OPTAMULET, 2 * os->_oVar1 + DBORDERX + 5, 2 * os->_oVar2 + DBORDERY + 5, sendmsg, false);
			//int tv = dTransVal[2 * os->_oVar1 + DBORDERX + 1][2 * os->_oVar2 + DBORDERY + 1];
			//DRLG_MRectTrans(os->_oVar1 + 2, os->_oVar2 + 2, os->_oVar1 + 4, os->_oVar2 + 4, tv); // LEVER_EFFECT
			//DRLG_MRectTrans(os->_oVar1 + 6, os->_oVar2 + 6, os->_oVar1 + 8, os->_oVar2 + 8, tv); // LEVER_EFFECT
		}
	}
	if (deltaload)
		return;
	if (pnum == mypnum)
		InitQTextMsg(questlist[qn]._qdmsg);
	if (quests[qn]._qvar1 == 0) {
		quests[qn]._qvar1 = 1;
		quests[qn]._qactive = QUEST_ACTIVE;
		quests[qn]._qlog = TRUE;
		if (qn == Q_BLOOD) {
			SetRndSeed(os->_oRndSeed);
			SpawnQuestItemAt(IDI_BLDSTONE, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 17, sendmsg, false);
		}
		if (sendmsg) {
			NetSendCmdQuest(qn, true);
			NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
		}
	}
}

static void OperateChest(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int i, mdir;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;

	os->_oSelFlag = 0;
	os->_oAnimFrame += 2;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	if (currLvl._dSetLvl) {
		for (i = os->_oVar1; i > 0; i--) { // CHEST_ITEM_NUM
			CreateRndItem(os->_ox, os->_oy, true, sendmsg, false);
		}
	} else {
		for (i = os->_oVar1; i > 0; i--) { // CHEST_ITEM_NUM
			if (os->_oVar2 != 0)           // CHEST_ITEM_TYPE
				CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
			else
				CreateRndUseful(os->_ox, os->_oy, sendmsg, false);
		}
	}
	if (os->_otype >= OBJ_TCHEST1 && os->_otype <= OBJ_TCHEST3 && os->_oTrapFlag) {
		os->_oTrapFlag = FALSE;
		mdir = GetDirection(os->_ox, os->_oy, plr._px, plr._py);
		AddMissile(os->_ox, os->_oy, plr._px, plr._py, mdir, os->_oVar4, 1, -1, 0, 0, 0); // CHEST_TRAP_TYPE
	}
}

static void OperateMushPatch(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	if (numitems >= MAXITEMS) {
		return;
	}

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAround(IDI_MUSHROOM, os->_ox, os->_oy, sendmsg, false);
}

static void OperateInnSignChest(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	if (numitems >= MAXITEMS) {
		return;
	}

	if (quests[Q_LTBANNER]._qvar1 != 1) {
		if (!deltaload && pnum == mypnum) {
			PlaySFX(sgSFXSets[SFXS_PLR_24][plr._pClass]);
		}
		return;
	}
	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame += 2;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAround(IDI_BANNER, os->_ox, os->_oy, sendmsg, false);
}

static void OperateSlainHero(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;
	BYTE pc;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	pc = plr._pClass;
	const int typeCurs[NUM_CLASSES][2] = {
		{ ITYPE_SWORD, ICURS_BASTARD_SWORD },
		{ ITYPE_BOW, ICURS_LONG_WAR_BOW },
		{ ITYPE_STAFF, ICURS_LONG_STAFF },
#ifdef HELLFIRE
		{ ITYPE_STAFF, ICURS_WAR_STAFF },
		{ ITYPE_SWORD, ICURS_KATAR }, // TODO: better ICURS?
		{ ITYPE_AXE, ICURS_BATTLE_AXE },
#endif
	};
	SetRndSeed(os->_oRndSeed);
	CreateMagicItem(typeCurs[pc][0], typeCurs[pc][1], os->_ox, os->_oy, sendmsg);
	PlaySfxLoc(sgSFXSets[SFXS_PLR_09][pc], plr._px, plr._py);
}

static void OperateFlameTrapLever(int oi, bool sendmsg)
{
	ObjectStruct *os, *on;
	int i;
	bool disable;

	os = &object[oi];
	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);

	disable = os->_oAnimFrame == FLAMETRAP_ACTIVE_FRAME;
	os->_oAnimFrame = disable ? FLAMETRAP_INACTIVE_FRAME : FLAMETRAP_ACTIVE_FRAME;

	if (sendmsg)
		NetSendCmdParam1(true, disable ? CMD_TRAPCLOSE : CMD_TRAPOPEN, oi);
	for (i = 0; i < nobjects; i++) {
		on = &object[objectactive[i]]; //         FLAMETRAP_ID
		if (on->_otype == OBJ_FLAMEHOLE && on->_oVar1 == os->_oVar1) {
			on->_oVar2 = disable ? TRAP_INACTIVE : TRAP_ACTIVE;
		}
	}
}

static void OperateSarc(int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		return;
	}

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_SARC, os->_ox, os->_oy);

	os->_oAnimFlag = TRUE;
	//os->_oAnimFrameLen = 3;
	SetRndSeed(os->_oRndSeed);
	if (os->_oVar1 <= 2) // SARC_ITEM
		CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
	if (os->_oVar1 >= 8) // SARC_SKELE
		SpawnSkeleton(os->_oVar2, os->_ox, os->_oy, DIR_NONE);
}

/**
 * Handle the using the pedistal of Q_BLOOD-quest.
 */
static void OperatePedistal(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int iv;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	if (!deltaload && pnum != -1) { // TODO: possible desync of player-items?
		if (numitems >= MAXITEMS)
			return;
		if (!PlrHasItem(pnum, IDI_BLDSTONE, &iv))
			return;
		quests[Q_BLOOD]._qvar1++;
		if (sendmsg) {
			// assert(pnum == mypnum);
			PlrInvItemRemove(pnum, iv);
			NetSendCmdQuest(Q_BLOOD, false);
			NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
		}
	}

	os->_oAnimFrame = quests[Q_BLOOD]._qvar1;
	switch (quests[Q_BLOOD]._qvar1) {
	case 1:
		break;
	case 3:
		ObjChangeMap(setpc_x + 6, setpc_y + 3, setpc_x + setpc_w, setpc_y + 7);
	case 2:
		ObjChangeMap(setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7);
		break;
	case 4:
		ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
		LoadMapSetObjs("Levels\\L2Data\\Blood2.DUN");
		os->_oSelFlag = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (deltaload)
		return;
	switch (quests[Q_BLOOD]._qvar1) {
	case 1:
		break; // should not really happen
	case 2:
		SetRndSeed(os->_oRndSeed);
		SpawnQuestItemAt(IDI_BLDSTONE, 2 * setpc_x + DBORDERX + 3, 2 * setpc_y + DBORDERY + 10, sendmsg, false);
		break;
	case 3:
		SetRndSeed(os->_oRndSeed + 1);
		SpawnQuestItemAt(IDI_BLDSTONE, 2 * setpc_x + DBORDERX + 15, 2 * setpc_y + DBORDERY + 10, sendmsg, false);
		break;
	case 4:
		// SetRndSeed(os->_oRndSeed + 2);
		SpawnUnique(UITEM_ARMOFVAL, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 3, sendmsg, false);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	PlaySfxLoc(quests[Q_BLOOD]._qvar1 == 4 ? LS_BLODSTAR : LS_PUDDLE, os->_ox, os->_oy);
}

void DisarmObject(int pnum, int oi)
{
	ObjectStruct *os, *on;
	int i, trapdisper;

	if (pnum == mypnum)
		NewCursor(CURSOR_HAND);
	os = &object[oi];
	if (os->_oTrapFlag) {
		trapdisper = 2 * plr._pDexterity - 8 * currLvl._dLevel;
		if (random_(154, 100) <= trapdisper) {
			for (i = 0; i < nobjects; i++) {
				on = &object[objectactive[i]];
				if ((on->_otype == OBJ_TRAPL || on->_otype == OBJ_TRAPR)
				 && dObject[on->_oVar1][on->_oVar2] - 1 == oi) {
					on->_oVar4 = TRAP_INACTIVE;
					os->_oTrapFlag = FALSE;
				}
			}
#ifndef HELLFIRE
			if (os->_otype >= OBJ_TCHEST1 && os->_otype <= OBJ_TCHEST3)
				os->_oTrapFlag = FALSE;
#endif
		}
	}
}

static void CloseChest(int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag != 0)
		return;
	os->_oSelFlag = 1;
	os->_oAnimFrame -= 2;

	SetRndSeed(os->_oRndSeed);
	os->_oRndSeed = GetRndSeed();

	if (sendmsg)
		NetSendCmdParam1(false, CMD_CHESTCLOSE, oi);
}

/** Raise the skill of the given player for the price of 10% of their maximum mana
*/
static void AddRaiseSkill(int pnum, int sn)
{
	int v1, v2;
	unsigned t;
	// add/raise the skill
	plr._pMemSkills |= SPELL_MASK(sn);
	if (plr._pSkillLvl[sn] < MAXSPLLEVEL) {
		plr._pSkillExp[sn] = SkillExpLvlsTbl[plr._pSkillLvl[sn]];
		plr._pSkillLvl[sn]++;
	}
	// reduce maximum mana
	t = plr._pMaxManaBase / 10;
	v1 = plr._pMana - plr._pManaBase;
	v2 = plr._pMaxMana - plr._pMaxManaBase;

	plr._pManaBase -= t;
	plr._pMana -= t;
	plr._pMaxMana -= t;
	plr._pMaxManaBase -= t;
	if (plr._pMana >> 6 <= 0) {
		plr._pManaBase = 0;
		plr._pMana = v1;
	}
	if (plr._pMaxMana >> 6 <= 0) {
		plr._pMaxManaBase = 0;
		plr._pMaxMana = v2;
	}
}

static void ConvertPotion(ItemStruct *pi)
{
	if (pi->_itype != ITYPE_MISC)
		return;
	if (pi->_iMiscId == IMISC_FULLHEAL || pi->_iMiscId == IMISC_FULLMANA) {
		CreateBaseItem(pi, IDI_FULLREJUV);
	} else if (pi->_iMiscId == IMISC_HEAL || pi->_iMiscId == IMISC_MANA) {
		CreateBaseItem(pi, IDI_REJUV);
	}
}

static void OperateShrine(int pnum, int psfx, int psfxCnt, int oi, bool sendmsg)
{
	ObjectStruct *os;
	ItemStruct *pi;
	int i, r, cnt;
#ifdef HELLFIRE
	int xx, yy;
#endif
	assert((unsigned)oi < MAXOBJECTS);

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;

	if (deltaload) {
		os->_oAnimFlag = FALSE;
		os->_oAnimFrame = os->_oAnimLen;
		return;
	}

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	SetRndSeed(os->_oRndSeed);

	PlaySfxLoc(psfx, os->_ox, os->_oy, psfxCnt);
	os->_oAnimFlag = TRUE;
	//os->_oAnimFrameLen = 1;

	switch (os->_oVar1) { // SHRINE_TYPE
	case SHRINE_HIDDEN:
		cnt = 0;
		pi = plr.InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype != ITYPE_NONE
			 && pi->_iMaxDur != DUR_INDESTRUCTIBLE
			 && pi->_iMaxDur != 0)
				cnt++;
		}
		if (cnt != 0) {
			r = random_(0, cnt);
			pi = plr.InvBody;
			for (i = NUM_INVLOC; i != 0; i--, pi++) {
				if (pi->_itype != ITYPE_NONE
				 && pi->_iMaxDur != DUR_INDESTRUCTIBLE
				 && pi->_iMaxDur != 0) {
					if (r == 0) {
						pi->_iMaxDur = pi->_iMaxDur > 10 ? pi->_iMaxDur - 10 : 1;
						pi->_iDurability = pi->_iDurability > 10 ? pi->_iDurability - 10 : 1;
					} else {
						pi->_iMaxDur = std::min(pi->_iMaxDur + 10, DUR_INDESTRUCTIBLE - 1);
						pi->_iDurability = std::min(pi->_iDurability + 10, pi->_iMaxDur);
					}
					r--;
				}
			}
		}
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_HIDDEN);
		break;
	case SHRINE_GLOOMY:
		pi = &plr.InvBody[INVLOC_HAND_LEFT];
		pi->_iDurability = pi->_iMaxDur;

		pi = &plr.InvBody[INVLOC_HAND_RIGHT];
		if (pi->_iClass == ICLASS_WEAPON)
			pi->_iDurability = pi->_iMaxDur;

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_GLOOMY);
		break;
	case SHRINE_WEIRD:
		pi = &plr.InvBody[INVLOC_HEAD];
		pi->_iDurability = pi->_iMaxDur;
		
		pi = &plr.InvBody[INVLOC_CHEST];
		pi->_iDurability = std::max(1, pi->_iDurability >> 1);

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_WEIRD);
		break;
	case SHRINE_RELIGIOUS:
		pi = plr.InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur;
		pi = plr.InvList;
		for (i = plr._pNumInv; i > 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur;
		pi = plr.SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur; // belt items don't have durability?

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_RELIGIOUS);
		break;
	case SHRINE_MAGICAL:
		AddMissile(
		    0,
		    0,
		    0,
		    0,
		    0,
		    MIS_MANASHIELD,
		    -1,
		    pnum,
		    0,
		    0,
		    (1 + currLvl._dLevel) >> 1);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_MAGICAL);
		break;
	case SHRINE_STONE:
		pi = plr.InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges;
		pi = plr.InvList;
		for (i = plr._pNumInv; i > 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges;
		pi = plr.SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges; // belt items don't have charges?

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_STONE);
		break;
	case SHRINE_CREEPY:
		pi = &plr.InvBody[INVLOC_HAND_LEFT];
		pi->_iCharges = pi->_iMaxCharges;

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_CREEPY);
		break;
	case SHRINE_THAUMATURGIC:
		for (i = 0; i < nobjects; i++) {
			os = &object[objectactive[i]];
			if (os->_otype == OBJ_CHEST1
			 || os->_otype == OBJ_CHEST2
			 || os->_otype == OBJ_CHEST3) {
				CloseChest(objectactive[i], sendmsg);
			}
		}
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_THAUMATURGIC);
		break;
	case SHRINE_FASCINATING:
		AddRaiseSkill(pnum, SPL_FIREBOLT);

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_FASCINATING);
		break;
	case SHRINE_SHIMMERING:
		PlrFillMana(pnum);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_SHIMMERING);
		break;
	case SHRINE_CRYPTIC:
		AddMissile(
		    os->_ox,
		    os->_oy,
		    0,
		    0,
		    0,
		    MIS_LIGHTNOVAC,
		    0,
		    -1,
		    0,
		    0,
		    0);
		PlrFillMana(pnum);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_CRYPTIC);
		break;
	case SHRINE_ELDRITCH:
		pi = plr.InvList;
		for (i = plr._pNumInv; i > 0; i--, pi++)
			ConvertPotion(pi);
		pi = plr.SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			ConvertPotion(pi);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_ELDRITCH);
		break;
	case SHRINE_EERIE:
		if (pnum == mypnum) {
			InitDiabloMsg(EMSG_SHRINE_EERIE1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_EERIE2);

			PlrFillMana(mypnum);
		}
		break;
	case SHRINE_SPOOKY:
		if (pnum == mypnum) {
			InitDiabloMsg(EMSG_SHRINE_SPOOKY1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_SPOOKY2);
			PlrFillHp(mypnum);
			PlrFillMana(mypnum);
		}
		break;
	case SHRINE_QUIET:
		if (pnum == mypnum) {
			InitDiabloMsg(EMSG_SHRINE_QUIET1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_QUIET2);

			PlrFillHp(mypnum);
		}
		break;
	case SHRINE_DIVINE:
		if (currLvl._dLevelIdx <= DLV_CATHEDRAL3) {
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLMANA, sendmsg, false);
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLHEAL, sendmsg, false);
		} else {
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLREJUV, sendmsg, false);
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLREJUV, sendmsg, false);
		}
		PlrFillHp(pnum);
		PlrFillMana(pnum);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_DIVINE);
		break;
	case SHRINE_HOLY:
		AddMissile(plr._px, plr._py, 0, 0, 0, MIS_RNDTELEPORT, -1, pnum, 0, 0, 0);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_HOLY);
		break;
	case SHRINE_SACRED:
		AddRaiseSkill(pnum, SPL_CBOLT);

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_SACRED);
		break;
	case SHRINE_ORNATE:
		AddRaiseSkill(pnum, SPL_HBOLT);

		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_ORNATE);
		break;
	case SHRINE_SPIRITUAL:
		for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
			if (plr.InvGrid[i] == 0) {
				r = currLvl._dLevel + random_(160, 2 * currLvl._dLevel);
				plr._pGold += r;
				SetGoldItemValue(&golditem, r);
				golditem._iSeed = GetRndSeed();
				cnt = plr._pNumInv;
				NetSendCmdChItem(&golditem, INVITEM_INV_FIRST + cnt);
				copy_pod(plr.InvList[cnt], golditem);
				plr._pNumInv++;
				plr.InvGrid[i] = plr._pNumInv;
			}
		}
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_SPIRITUAL);
		break;
	case SHRINE_SECLUDED:
		if (pnum != mypnum)
			return;
		static_assert(sizeof(automapview) == DMAXY * DMAXX, "Linear traverse of automapview does not work in OperateShrine.");
		memset(automapview, TRUE, DMAXX * DMAXY);
		InitDiabloMsg(EMSG_SHRINE_SECLUDED);
		break;
	case SHRINE_GLIMMERING:
		pi = plr.InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE;
		pi = plr.InvList;
		for (i = plr._pNumInv; i > 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE;
		pi = plr.SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE; // belt items can't be magical?
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_GLIMMERING);
		break;
	case SHRINE_TAINTED:
		if (MINION_NR_INACTIVE(mypnum)) {
			AddMissile(
				myplr._px,
				myplr._py,
				myplr._px,
				myplr._py,
				0,
				MIS_GOLEM,
				0,
				mypnum,
				0,
				0,
				currLvl._dLevel >> 1);
		}
		//if (pnum != mypnum)
		//	return;
		InitDiabloMsg(EMSG_SHRINE_TAINTED);
		break;
	case SHRINE_GLISTENING:
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_GLISTENING);
		AddMissile(
		    os->_ox,
		    os->_oy,
		    plr._px,
		    plr._py,
		    plr._pdir,
		    MIS_TOWN,
		    1,
		    pnum,
		    0,
		    0,
		    0);
		break;
#ifdef HELLFIRE
	case SHRINE_SPARKLING:
		AddPlrExperience(pnum, plr._pLevel, 500 * currLvl._dLevel);
		AddMissile(
		    os->_ox,
		    os->_oy,
		    0,
		    0,
		    0,
		    MIS_FLASH,
		    0,
		    -1,
		    0,
		    0,
		    0);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_SPARKLING);
		break;
	case SHRINE_SOLAR: { // BUGFIX: missiles are not added/handled by the delta info...
		static_assert(MIS_RUNEFIRE + 1 == MIS_RUNELIGHT, "SHRINE_SOLAR expects runes in a given order I.");
		static_assert(MIS_RUNEFIRE + 2 == MIS_RUNENOVA, "SHRINE_SOLAR expects runes in a given order II.");
		static_assert(MIS_RUNEFIRE + 3 == MIS_RUNEIMMOLAT, "SHRINE_SOLAR expects runes in a given order III.");
		const char *cr = &CrawlTable[CrawlNum[3]];
		for (i = (BYTE)*cr; i > 0; i--) {
			xx = plr._px + *++cr;
			yy = plr._py + *++cr;
			if (!ItemSpaceOk(xx, yy))
				continue;
			if (random_(0, 3) == 0)
				AddMissile(xx, yy, xx, yy, 0, MIS_RUNEFIRE + random_(0, 4), -1, -1, 0, 0, currLvl._dLevel >> 1);
			else
				CreateTypeItem(xx, yy, false, ITYPE_MISC, IMISC_RUNE, sendmsg, false);
		}
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_SOLAR);
	} break;
	case SHRINE_MURPHYS:
		pi = plr.InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype == ITYPE_NONE || random_(0, 3) != 0)
				continue;
			r = pi->_iDurability;
			if (r != DUR_INDESTRUCTIBLE && r != 0) {
				r /= 2;
				if (r == 0) {
					if (pnum == mypnum)
						NetSendCmdDelItem(i);
					pi->_itype = ITYPE_NONE;
				}
				else
					pi->_iDurability = r;
				break;
			}
		}
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_MURPHYS);
		// BUGFIX: move this up if TakePlrsMoney works with any pnum
		if (i == 0) {
			TakePlrsMoney(plr._pGold / 3);
		}
		break;
#endif
	default:
		ASSUME_UNREACHABLE
	}

	CalcPlrInv(pnum, true);
	gbRedrawFlags = REDRAW_ALL;
}

static void OperateSkelBook(int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame += 2;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC,
		random_(161, 5) != 0 ? IMISC_SCROLL : IMISC_BOOK, sendmsg, false);
}

static void OperateBookCase(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame -= 2;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_BOOK, sendmsg, false);
	if (QuestStatus(Q_ZHAR) && quests[Q_ZHAR]._qactive != QUEST_DONE
	 && dTransVal[monster[MAX_MINIONS]._mx][monster[MAX_MINIONS]._my] == dTransVal[plr._px][plr._py]
	 && monster[MAX_MINIONS].mtalkmsg == TEXT_ZHAR1) {
		assert((monster[MAX_MINIONS]._uniqtype - 1) == UMT_ZHAR);
		monster[MAX_MINIONS].mtalkmsg = TEXT_ZHAR2;
		//MonStartStand(MAX_MINIONS, monster[MAX_MINIONS]._mdir);
		//monster[MAX_MINIONS]._mgoal = MGOAL_ATTACK2;
		monster[MAX_MINIONS]._mmode = MM_TALK;
		//monster[MAX_MINIONS]._mVar8 = 0; // MON_TIMER
		quests[Q_ZHAR]._qvar1 = 2;
		if (sendmsg)
			NetSendCmdQuest(Q_ZHAR, true);
	}
}

static void OperateDecap(int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	SetRndSeed(os->_oRndSeed);
	CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
}

static void OperateArmorStand(int oi, bool sendmsg)
{
	ObjectStruct *os;
	int itype;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	SetRndSeed(os->_oRndSeed);
	static_assert(ITYPE_LARMOR + 1 == ITYPE_MARMOR, "OperateArmorStand expects an ordered ITYPE_ for armors I.");
	static_assert(ITYPE_MARMOR + 1 == ITYPE_HARMOR, "OperateArmorStand expects an ordered ITYPE_ for armors II.");
	itype = ITYPE_LARMOR + random_(0, currLvl._dLevel >= 24 ? 3 : (currLvl._dLevel >= 10 ? 2 : 1));
	CreateTypeItem(os->_ox, os->_oy, true, itype, IMISC_NONE, sendmsg, false);
}

static void OperateGoatShrine(int pnum, int oi, bool sendmsg)
{
	OperateShrine(pnum, LS_GSHRINE, 1, oi, sendmsg);
	object[oi]._oAnimFlag = TRUE;
}

static void OperateCauldron(int pnum, int oi, bool sendmsg)
{
	OperateShrine(pnum, LS_CALDRON, 1, oi, sendmsg);
	object[oi]._oAnimFrame = 3;
	object[oi]._oAnimFlag = FALSE;
}

static void OperateFountains(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	// SetRndSeed(os->_oRndSeed);
	switch (os->_otype) {
	case OBJ_BLOODFTN:
		if (deltaload)
			return;
		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		PlrIncHp(pnum, 64);
		break;
	case OBJ_PURIFYINGFTN:
		if (deltaload)
			return;
		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		PlrIncMana(pnum, 64);
		break;
	case OBJ_MURKYFTN:
		if (os->_oSelFlag == 0)
			return;
		os->_oSelFlag = 0;
		if (deltaload)
			return;
		if (sendmsg)
			NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		AddMissile(
		    0,
		    0,
		    0,
		    0,
		    0,
		    MIS_INFRA,
		    -1,
		    pnum,
		    0,
		    0,
		    currLvl._dLevel >> 1);
		break;
	case OBJ_TEARFTN:
		if (deltaload)
			return;

		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		PlrIncHp(pnum, 64);
		if (plr._pMana >= 64)
			PlrDecMana(pnum, 64);
		break;
	}
	if (pnum == mypnum)
		gbRedrawFlags = REDRAW_ALL;
}

static void OperateWeaponRack(int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;

	os->_oSelFlag = 0;
	os->_oAnimFrame++;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	assert(currLvl._dLevel >= 4); // must have an appropriate item for each type
	static_assert(ITYPE_SWORD + 1 == ITYPE_AXE, "OperateWeaponRack expects an ordered ITYPE_ for weapons I.");
	static_assert(ITYPE_AXE + 1 == ITYPE_BOW, "OperateWeaponRack expects an ordered ITYPE_ for weapons II.");
	static_assert(ITYPE_BOW + 1 == ITYPE_MACE, "OperateWeaponRack expects an ordered ITYPE_ for weapons III.");
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, true, ITYPE_SWORD + random_(0, 4),	IMISC_NONE, sendmsg, false);
}

/**
 * Handle the reading of story books in the dungeon.
 */
static void OperateStoryBook(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	// assert(os->_oSelFlag != 0);

	os->_oAnimFrame = os->_oVar4; // STORY_BOOK_ANIM_FRAME
	if (deltaload) {
#ifdef HELLFIRE
		if (currLvl._dLevelIdx == DLV_CRYPT4 && os->_oVar8 == 8 && quests[Q_NAKRUL]._qvar1 >= 4) {
			if (quests[Q_NAKRUL]._qvar1 == 4)
				WakeUberDiablo();
			OpenUberRoom();
		}
#endif
		return;
	}
	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT && quests[Q_NAKRUL]._qactive != QUEST_DONE) {
		if (os->_oVar8 != 0) { // STORY_BOOK_NAKRUL_IDX
			assert(currLvl._dLevelIdx == DLV_CRYPT4);
			if (pnum != -1) {
				quests[Q_NAKRUL]._qvar1 = ProgressUberLever(os->_oVar8, quests[Q_NAKRUL]._qvar1);
				if (sendmsg)
					NetSendCmdQuest(Q_NAKRUL, true);
			}
			if (quests[Q_NAKRUL]._qvar1 == 3) {
				quests[Q_NAKRUL]._qvar1 = 4;
				quests[Q_NAKRUL]._qactive = QUEST_DONE;
				if (sendmsg)
					NetSendCmdQuest(Q_NAKRUL, true);
				OpenUberRoom();
				WakeUberDiablo();
				return;
			}
		} else {
			quests[Q_NAKRUL]._qactive = QUEST_ACTIVE;
			quests[Q_NAKRUL]._qlog = TRUE;
			quests[Q_NAKRUL]._qmsg = os->_oVar2; // STORY_BOOK_MSG
		}
	}
#endif
	if (pnum == mypnum)
		InitQTextMsg(os->_oVar2); // STORY_BOOK_MSG
}

static void OperateLazStand(int oi, bool sendmsg)
{
	ObjectStruct *os = &object[oi];

	if (numitems >= MAXITEMS) {
		return;
	}
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	SpawnQuestItemAround(IDI_LAZSTAFF, os->_ox, os->_oy, sendmsg, false);
}

static void OperateCrux(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os, *on;
	int i;
	bool triggered;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFlag = TRUE;
	os->_oAnimFrame = 1;
	//os->_oAnimFrameLen = 1;
	os->_oSolidFlag = TRUE;
	os->_oMissFlag = TRUE;
	os->_oBreak = -1;

	triggered = true;
	for (i = 0; i < nobjects; i++) {
		on = &object[objectactive[i]];
		if (on->_otype != OBJ_CRUX1 && on->_otype != OBJ_CRUX2 && on->_otype != OBJ_CRUX3)
			continue;
		if (os->_oVar8 != on->_oVar8 || on->_oBreak == -1)
			continue;
		triggered = false;
	}
	if (triggered)
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);

	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		//os->_oAnimCnt = 0;
		//os->_oAnimFrameLen = 1000;
		return;
	}

	if (sendmsg || pnum == -1) // send message if the crux was destroyed by a missile
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);

	PlaySfxLoc(LS_BONESP, os->_ox, os->_oy);

	if (triggered)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
}

static void OperateBarrel(bool forcebreak, int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os = &object[oi];
	int mpo;
	int xp, yp;

	if (os->_oSelFlag == 0)
		return;
	if (!forcebreak) {
		if (pnum != mypnum) {
			if (deltaload)
				return;

			PlaySfxLoc(IS_IBOW, os->_ox, os->_oy);
			return;
		}
	}

	// os->_oVar1 = 0;
	os->_oAnimFlag = TRUE;
	os->_oAnimFrame = 1;
	//os->_oAnimFrameLen = 1;
	os->_oSolidFlag = FALSE;
	os->_oMissFlag = TRUE;
	os->_oBreak = -1;
	os->_oSelFlag = 0;
	os->_oPreFlag = TRUE;
	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		//os->_oAnimCnt = 0;
		//os->_oAnimFrameLen = 1000;
		return;
	}

	if (os->_otype == OBJ_BARRELEX) {
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_CRYPT)
			PlaySfxLoc(IS_POPPOP3, os->_ox, os->_oy);
		else if (currLvl._dType == DTYPE_NEST)
			PlaySfxLoc(IS_POPPOP8, os->_ox, os->_oy);
		else
#endif
			PlaySfxLoc(IS_BARLFIRE, os->_ox, os->_oy);
		for (yp = os->_oy - 1; yp <= os->_oy + 1; yp++) {
			for (xp = os->_ox - 1; xp <= os->_ox + 1; xp++) {
				AddMissile(xp, yp, 0, 0, 0, MIS_BARRELEX, -1, -1, 0, 0, 0);
				mpo = dObject[xp][yp];
				if (mpo > 0) {
					mpo--;
					if (object[mpo]._otype == OBJ_BARRELEX && object[mpo]._oBreak != -1)
						OperateBarrel(true, pnum, mpo, sendmsg);
				}
			}
		}
	} else {
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_CRYPT)
			PlaySfxLoc(IS_POPPOP2, os->_ox, os->_oy);
		else if (currLvl._dType == DTYPE_NEST)
			PlaySfxLoc(IS_POPPOP5, os->_ox, os->_oy);
		else
#endif
			PlaySfxLoc(IS_BARREL, os->_ox, os->_oy);
		SetRndSeed(os->_oRndSeed);
		if (os->_oVar2 <= 1) {    // BARREL_ITEM
			if (os->_oVar3 == 0)  // BARREL_ITEM_TYPE
				CreateRndUseful(os->_ox, os->_oy, sendmsg, false);
			else
				CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
		} else if (os->_oVar2 >= 8)
			SpawnSkeleton(os->_oVar4, os->_ox, os->_oy, DIR_NONE); // BARREL_SKELE
	}
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

void OperateObject(int pnum, int oi, bool TeleFlag)
{
	bool sendmsg;

	sendmsg = (pnum == mypnum);
	switch (object[oi]._otype) {
	case OBJ_L1LDOOR:
		if (TeleFlag)
			OperateL1LDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == mypnum
			OperateL1LDoor(plr._px, plr._py, oi, sendmsg);
		break;
	case OBJ_L1RDOOR:
		if (TeleFlag)
			OperateL1RDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == mypnum
			OperateL1RDoor(plr._px, plr._py, oi, sendmsg);
		break;
	case OBJ_L2LDOOR:
		if (TeleFlag)
			OperateL2LDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == mypnum
			OperateL2LDoor(plr._px, plr._py, oi, sendmsg);
		break;
	case OBJ_L2RDOOR:
		if (TeleFlag)
			OperateL2RDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == mypnum
			OperateL2RDoor(plr._px, plr._py, oi, sendmsg);
		break;
	case OBJ_L3LDOOR:
		if (TeleFlag)
			OperateL3LDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == mypnum
			OperateL3LDoor(plr._px, plr._py, oi, sendmsg);
		break;
	case OBJ_L3RDOOR:
		if (TeleFlag)
			OperateL3RDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == mypnum
			OperateL3RDoor(plr._px, plr._py, oi, sendmsg);
		break;
	case OBJ_LEVER:
	case OBJ_SWITCHSKL:
		OperateLever(oi, sendmsg);
		break;
	case OBJ_BOOK2L:
		OperateVileBook(pnum, oi, sendmsg);
		break;
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		OperateChest(pnum, oi, sendmsg);
		break;
	case OBJ_SARC:
		OperateSarc(oi, sendmsg);
		break;
	case OBJ_FLAMELVR:
		OperateFlameTrapLever(oi, sendmsg);
		break;
	case OBJ_BLINDBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_STEELTOME:
	//case OBJ_BOOKLVR:
	case OBJ_BOOK2R:
		OperateBookLever(pnum, oi, sendmsg);
		break;
	case OBJ_CRUX1:
	case OBJ_CRUX2:
	case OBJ_CRUX3:
		OperateCrux(pnum, oi, sendmsg);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
		OperateBarrel(false, pnum, oi, sendmsg);
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		OperateShrine(pnum, IS_MAGIC, 2, oi, sendmsg);
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		OperateSkelBook(oi, sendmsg);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		OperateBookCase(pnum, oi, sendmsg);
		break;
	case OBJ_DECAP:
		OperateDecap(oi, sendmsg);
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR:
		OperateArmorStand(oi, sendmsg);
		break;
	case OBJ_GOATSHRINE:
		OperateGoatShrine(pnum, oi, sendmsg);
		break;
	case OBJ_CAULDRON:
		OperateCauldron(pnum, oi, sendmsg);
		break;
	case OBJ_BLOODFTN:
	case OBJ_PURIFYINGFTN:
	case OBJ_MURKYFTN:
	case OBJ_TEARFTN:
		OperateFountains(pnum, oi, sendmsg);
		break;
	case OBJ_STORYBOOK:
		OperateStoryBook(pnum, oi, sendmsg);
		break;
	case OBJ_PEDISTAL:
		OperatePedistal(pnum, oi, sendmsg);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACKL:
	case OBJ_WEAPONRACKR:
		OperateWeaponRack(oi, sendmsg);
		break;
	case OBJ_MUSHPATCH:
		OperateMushPatch(pnum, oi, sendmsg);
		break;
	case OBJ_LAZSTAND:
		OperateLazStand(oi, sendmsg);
		break;
	case OBJ_SLAINHERO:
		OperateSlainHero(pnum, oi, sendmsg);
		break;
	case OBJ_SIGNCHEST:
		OperateInnSignChest(pnum, oi, sendmsg);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void SyncDoorOpen(int oi)
{
	if (object[oi]._oVar4 == DOOR_CLOSED)
		SyncOpObject(-1, oi);
}
void SyncDoorClose(int oi)
{
	if (object[oi]._oVar4 == DOOR_OPEN)
		SyncOpObject(-1, oi);
}

void SyncTrapDisable(int oi)
{
	object[oi]._oVar4 = TRAP_INACTIVE;
}

void SyncTrapOpen(int oi)
{
	if (object[oi]._oAnimFrame == FLAMETRAP_INACTIVE_FRAME)
		SyncOpObject(-1, oi);
}
void SyncTrapClose(int oi)
{
	if (object[oi]._oAnimFrame == FLAMETRAP_ACTIVE_FRAME)
		SyncOpObject(-1, oi);
}

/**
 * Re-Close chest during delta-load.
 * Does NOT work as a standard sync, because it needs to force the new oRndSeed.
 * (to achive this, OperateChest is called).
 */
void SyncChestClose(int oi)
{
	// assert(deltaload);
	OperateChest(-1, oi, false);
	CloseChest(oi, false);
}

void SyncOpObject(int pnum, int oi)
{
	switch (object[oi]._otype) {
	case OBJ_L1LDOOR:
		OperateL1LDoor(-1, -1, oi, false);
		break;
	case OBJ_L1RDOOR:
		OperateL1RDoor(-1, -1, oi, false);
		break;
	case OBJ_L2LDOOR:
		OperateL2LDoor(-1, -1, oi, false);
		break;
	case OBJ_L2RDOOR:
		OperateL2RDoor(-1, -1, oi, false);
		break;
	case OBJ_L3LDOOR:
		OperateL3LDoor(-1, -1, oi, false);
		break;
	case OBJ_L3RDOOR:
		OperateL3RDoor(-1, -1, oi, false);
		break;
	case OBJ_LEVER:
	case OBJ_SWITCHSKL:
		OperateLever(oi, false);
		break;
	case OBJ_BOOK2L:
		OperateVileBook(pnum, oi, false);
		break;
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		OperateChest(pnum, oi, false);
		break;
	case OBJ_SARC:
		OperateSarc(oi, false);
		break;
	case OBJ_FLAMELVR:
		OperateFlameTrapLever(oi, false);
		break;
	case OBJ_BLINDBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_STEELTOME:
	//case OBJ_BOOKLVR:
	case OBJ_BOOK2R:
		OperateBookLever(pnum, oi, false);
		break;
	case OBJ_CRUX1:
	case OBJ_CRUX2:
	case OBJ_CRUX3:
		OperateCrux(pnum, oi, false);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
		OperateBarrel(true, pnum, oi, false);
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		OperateShrine(pnum, IS_MAGIC, 2, oi, false);
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		OperateSkelBook(oi, false);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		OperateBookCase(pnum, oi, false);
		break;
	case OBJ_DECAP:
		OperateDecap(oi, false);
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR:
		OperateArmorStand(oi, false);
		break;
	case OBJ_GOATSHRINE:
		OperateGoatShrine(pnum, oi, false);
		break;
	case OBJ_CAULDRON:
		OperateCauldron(pnum, oi, false);
		break;
	case OBJ_MURKYFTN:
	case OBJ_TEARFTN:
		OperateFountains(pnum, oi, false);
		break;
	case OBJ_STORYBOOK:
		OperateStoryBook(-1, oi, false);
		break;
	case OBJ_PEDISTAL:
		OperatePedistal(-1, oi, false);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACKL:
	case OBJ_WEAPONRACKR:
		OperateWeaponRack(oi, false);
		break;
	case OBJ_MUSHPATCH:
		OperateMushPatch(pnum, oi, false);
		break;
	case OBJ_LAZSTAND:
		OperateLazStand(oi, false);
		break;
	case OBJ_SLAINHERO:
		OperateSlainHero(pnum, oi, false);
		break;
	case OBJ_SIGNCHEST:
		OperateInnSignChest(pnum, oi, false);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void SyncLever(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4); // LEVER_EFFECT
}

static void SyncBookLever(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oAnimFrame == os->_oVar6) { // LEVER_BOOK_ANIM
		ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4); // LEVER_EFFECT
		//if (os->_otype == OBJ_BLINDBOOK) {
			//int tv = dTransVal[2 * os->_oVar1 + DBORDERX + 1][2 * os->_oVar2 + DBORDERY + 1];
			//DRLG_MRectTrans(os->_oVar1 + 2, os->_oVar2 + 2, os->_oVar1 + 4, os->_oVar2 + 4, tv); // LEVER_EFFECT
			//DRLG_MRectTrans(os->_oVar1 + 6, os->_oVar2 + 6, os->_oVar1 + 8, os->_oVar2 + 8, tv); // LEVER_EFFECT
		//}
	}
}

static void SyncPedistal(int oi)
{
	switch (quests[Q_BLOOD]._qvar1) {
	case 0:
	case 1:
		break;
	case 3:
		ObjChangeMapResync(setpc_x + 6, setpc_y + 3, setpc_x + setpc_w, setpc_y + 7);
	case 2:
		ObjChangeMapResync(setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7);
		break;
	case 4:
		ObjChangeMapResync(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
		LoadMapSetObjs("Levels\\L2Data\\Blood2.DUN");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void SyncL1Doors(int oi)
{
	ObjectStruct *os;
	int x, y, pn;

	os = &object[oi];

	x = os->_ox;
	y = os->_oy;
	if (os->_oVar4 == DOOR_CLOSED) {
		ObjSetMicro(x, y, os->_oVar1); // DOOR_PIECE_CLOSED
		dSpecial[x][y] = 0;

		pn = os->_oVar2;                 // DOOR_BACK_PIECE_CLOSED

		if (os->_otype == OBJ_L1RDOOR)
			x--;
		else
			y--;
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_CRYPT) {
			if (pn == 86 && dPiece[x][y] == 210)
				pn = os->_otype == OBJ_L1RDOOR ? 232 : 234;
		} else
#endif
		{
			if (pn == 50 && dPiece[x][y] == 396)
				pn = os->_otype == OBJ_L1RDOOR ? 411 : 412;
		}
		ObjSetMicro(x, y, pn);
		return;
	}

#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT) {
		if (os->_otype == OBJ_L1LDOOR) {
			ObjSetMicro(x, y, 206);
			dSpecial[x][y] = 1;
			objects_set_door_piece(x - 1, y);
			y--;
		} else {
			ObjSetMicro(x, y, 209);
			dSpecial[x][y] = 2;
			objects_set_door_piece(x, y - 1);
			x--;
		}
	} else
#endif
	{
		if (os->_otype == OBJ_L1LDOOR) {
			ObjSetMicro(x, y, os->_oVar1 == 214 ? 408 : 393); // DOOR_PIECE_CLOSED
			dSpecial[x][y] = 7;
			objects_set_door_piece(x - 1, y);
			y--;
		} else {
			ObjSetMicro(x, y, 395);
			dSpecial[x][y] = 8;
			objects_set_door_piece(x, y - 1);
			x--;
		}
	}
	DoorSet(oi, x, y);
}

static void SyncL2Doors(int oi)
{
	ObjectStruct *os;
	int x, y;

	os = &object[oi];
	x = os->_ox;
	y = os->_oy;
	if (os->_otype == OBJ_L2LDOOR) {
		if (os->_oVar4 == DOOR_CLOSED) {
			ObjSetMicro(x, y, 538);
			dSpecial[x][y] = 0;
		} else { // if (os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED) {
			ObjSetMicro(x, y, 13);
			dSpecial[x][y] = 5;
		}
	} else { // if (os->_otype == OBJ_L2RDOOR) {
		if (os->_oVar4 == DOOR_CLOSED) {
			ObjSetMicro(x, y, 540);
			dSpecial[x][y] = 0;
		} else { // if (os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED) {
			ObjSetMicro(x, y, 17);
			dSpecial[x][y] = 6;
		}
	}
}

static void SyncL3Doors(int oi)
{
	ObjectStruct *os;
	int x, y;

	os = &object[oi];
	x = os->_ox;
	y = os->_oy;
	if (os->_otype == OBJ_L3LDOOR) {
		if (os->_oVar4 == DOOR_CLOSED)
			ObjSetMicro(x, y, 531);
		else // if (os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED)
			ObjSetMicro(x, y, 538);
	} else { // if (os->_otype == OBJ_L3RDOOR) {
		if (os->_oVar4 == DOOR_CLOSED)
			ObjSetMicro(x, y, 534);
		else // if (os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED)
			ObjSetMicro(x, y, 541);
	}
}

void SyncObjectAnim(int oi)
{
	int type;

	type = object[oi]._otype;
	object[oi]._oAnimData = pObjCels[AllObjects[type].ofindex];
	object[oi]._oAnimFrameLen = AllObjects[type].oAnimFrameLen;
	switch (type) {
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
		SyncL1Doors(oi);
		break;
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		SyncL2Doors(oi);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		SyncL3Doors(oi);
		break;
	case OBJ_LEVER:
	case OBJ_BOOK2L:
	case OBJ_SWITCHSKL:
		SyncLever(oi);
		break;
	case OBJ_BOOK2R:
	case OBJ_BLINDBOOK:
	case OBJ_STEELTOME:
		SyncBookLever(oi);
		break;
	case OBJ_PEDISTAL:
		SyncPedistal(oi);
		break;
	}
}

void GetObjectStr(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	switch (os->_otype) {
	case OBJ_LEVER:
	case OBJ_FLAMELVR:
		copy_cstr(infostr, "Lever");
		break;
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		copy_cstr(infostr, "Small Chest");
		break;
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		if (os->_oVar4 == DOOR_OPEN)
			copy_cstr(infostr, "Open Door");
		else if (os->_oVar4 == DOOR_CLOSED)
			copy_cstr(infostr, "Closed Door");
		else // if (os->_oVar4 == DOOR_BLOCKED)
			copy_cstr(infostr, "Blocked Door");
		break;
	case OBJ_BOOK2L:
		if (currLvl._dLevelIdx == SL_BONECHAMB)
			copy_cstr(infostr, "Ancient Tome");
		else if (currLvl._dLevelIdx == SL_VILEBETRAYER)
			copy_cstr(infostr, "Book of Vileness");
		break;
	case OBJ_SWITCHSKL:
		copy_cstr(infostr, "Skull Lever");
		break;
	case OBJ_BOOK2R:
		copy_cstr(infostr, "Mythical Book");
		break;

	case OBJ_CHEST2:
	case OBJ_TCHEST2:
		copy_cstr(infostr, "Chest");
		break;
	case OBJ_CHEST3:
	case OBJ_TCHEST3:
	case OBJ_SIGNCHEST:
		copy_cstr(infostr, "Large Chest");
		break;
	case OBJ_CRUX1:
	case OBJ_CRUX2:
	case OBJ_CRUX3:
		copy_cstr(infostr, "Crucified Skeleton");
		break;
	case OBJ_SARC:
		copy_cstr(infostr, "Sarcophagus");
		break;
	case OBJ_BOOKSHELF:
		copy_cstr(infostr, "Bookshelf");
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_NEST)       // for nest levels
			copy_cstr(infostr, "Pod");          //Then a barrel is called a pod
		else if (currLvl._dType == DTYPE_CRYPT) // for crypt levels
			copy_cstr(infostr, "Urn");          //Then a barrel is called an urn
		else
#endif
			copy_cstr(infostr, "Barrel");
		break;
	case OBJ_SKELBOOK:
		copy_cstr(infostr, "Skeleton Tome");
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		snprintf(infostr, sizeof(infostr), "%s Shrine", shrinestrs[os->_oVar1]); // SHRINE_TYPE
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		copy_cstr(infostr, "Bookcase");
		break;
	case OBJ_BOOKSTAND:
		copy_cstr(infostr, "Library Book");
		break;
	case OBJ_BLOODFTN:
		copy_cstr(infostr, "Blood Fountain");
		break;
	case OBJ_DECAP:
		copy_cstr(infostr, "Decapitated Body");
		break;
	case OBJ_BLINDBOOK:
		copy_cstr(infostr, "Book of the Blind");
		break;
	case OBJ_BLOODBOOK:
		copy_cstr(infostr, "Book of Blood");
		break;
	case OBJ_PEDISTAL:
		copy_cstr(infostr, "Pedestal of Blood");
		break;
	case OBJ_PURIFYINGFTN:
		copy_cstr(infostr, "Purifying Spring");
		break;
	case OBJ_ARMORSTAND:
	case OBJ_WARARMOR:
		copy_cstr(infostr, "Armor");
		break;
	case OBJ_GOATSHRINE:
		copy_cstr(infostr, "Goat Shrine");
		break;
	case OBJ_CAULDRON:
		copy_cstr(infostr, "Cauldron");
		break;
	case OBJ_MURKYFTN:
		copy_cstr(infostr, "Murky Pool");
		break;
	case OBJ_TEARFTN:
		copy_cstr(infostr, "Fountain of Tears");
		break;
	case OBJ_STEELTOME:
		copy_cstr(infostr, "Steel Tome");
		break;
	case OBJ_STORYBOOK:
		copy_cstr(infostr, StoryBookName[os->_oVar3]); // STORY_BOOK_NAME
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACKL:
	case OBJ_WEAPONRACKR:
		copy_cstr(infostr, "Weapon Rack");
		break;
	case OBJ_MUSHPATCH:
		copy_cstr(infostr, "Mushroom Patch");
		break;
	case OBJ_LAZSTAND:
		copy_cstr(infostr, "Vile Stand");
		break;
	case OBJ_SLAINHERO:
		copy_cstr(infostr, "Slain Hero");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	infoclr = COL_WHITE;
	if (myplr._pClass == PC_ROGUE) {
		if (os->_oTrapFlag) {
			snprintf(tempstr, sizeof(tempstr), "Trapped %s", infostr);
			copy_str(infostr, tempstr);
			infoclr = COL_RED;
		}
	}
}

#ifdef HELLFIRE
void OpenUberRoom()
{
	ObjChangeMapResync(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h);
}
#endif

DEVILUTION_END_NAMESPACE
