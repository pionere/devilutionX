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

#define FLAMETRAP_DIR_X          2
#define FLAMETRAP_DIR_Y          0
#define FLAMETRAP_FIRE_ACTIVE    1
#define FLAMETRAP_FIRE_INACTIVE  0
#define FLAMETRAP_ACTIVE_FRAME   1
#define FLAMETRAP_INACTIVE_FRAME 2

int trapid;
static BYTE* objanimdata[NUM_OFILE_TYPES] = { 0 };
// int objectactive[MAXOBJECTS];
/** Specifies the number of active objects. */
int numobjects;
int leverid;
//int objectavail[MAXOBJECTS];
ObjectStruct objects[MAXOBJECTS];
//bool gbInitObjFlag;

/** Specifies the X-coordinate delta between barrels. */
const int bxadd[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
/** Specifies the Y-coordinate delta between barrels. */
const int byadd[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
/** Maps from shrine_id to shrine name. */
const char* const shrinestrs[NUM_SHRINETYPE] = {
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
	"Sparkling",
	"Murphy's",
#ifdef HELLFIRE
	"Solar",
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
	SHRINETYPE_ANY,    // SHRINE_SPARKLING
	SHRINETYPE_ANY,    // SHRINE_MURPHYS
#ifdef HELLFIRE
	SHRINETYPE_ANY,    // SHRINE_SOLAR
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
	//"Journal: The Ensorcellment",
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
#if FLICKER_LIGHT
const int flickers[32] = {
	1, 1, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 1
	//{ 0, 0, 0, 0, 0, 0, 1, 1, 1 }
};
#endif

void AddObjectType(int ofindex)
{
	char filestr[DATA_ARCHIVE_MAX_PATH];

	if (objanimdata[ofindex] != NULL) {
		return;
	}

	snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", objfiledata[ofindex].ofName);
	objanimdata[ofindex] = LoadFileInMem(filestr);
}

void InitObjectGFX()
{
	const ObjectData* ods;
	bool themeload[NUM_THEMES];
	bool fileload[NUM_OFILE_TYPES];
	int i;

	static_assert(false == 0, "InitObjectGFX fills fileload and themeload with 0 instead of false values.");
	memset(fileload, 0, sizeof(fileload));
	memset(themeload, 0, sizeof(themeload));

	for (i = 0; i < numthemes; i++)
		themeload[themes[i].ttype] = true;

	BYTE lvlMask = 1 << currLvl._dType;
	for (i = 0; i < NUM_OBJECTS; i++) {
		ods = &objectdata[i];
		if (!(ods->oLvlTypes & lvlMask)
		 && (ods->otheme == THEME_NONE || !themeload[ods->otheme])
		 && (ods->oquest == Q_INVALID || !QuestStatus(ods->oquest))) {
			continue;
		}
		if (fileload[ods->ofindex]) {
			continue;
		}
		fileload[ods->ofindex] = true;
		AddObjectType(ods->ofindex);
	}
}

void FreeObjectGFX()
{
	int i;

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		MemFreeDbg(objanimdata[i]);
	}
}

/**
 * Check the location if an object can be placed there in the init phase.
 * Must not consider the player's position, since it could change the dungeon
 * when a player re-enters the dungeon.
 */
static bool RndLocOk(int xp, int yp)
{
	if ((dMonster[xp][yp] | /*dPlayer[xp][yp] |*/ dObject[xp][yp]
	 | nSolidTable[dPiece[xp][yp]] | (dFlags[xp][yp] & BFLAG_POPULATED)) != 0)
		return false;
	// should be covered by Freeupstairs.
	//if (currLvl._dDunType != DTYPE_CATHEDRAL || dPiece[xp][yp] <= 126 || dPiece[xp][yp] >= 144)
		return true;
	//return false;
}

static POS32 RndLoc3x3()
{
	int xp, yp, i, j, tries;
	static_assert(DBORDERX != 0, "RndLoc3x3 returns 0;0 position as a failed location.");
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
		return { xp, yp };
fail:
		if (++tries > 20000)
			break;
	}
	return { 0, 0 };
}

static POS32 RndLoc3x4()
{
	int xp, yp, i, j, tries;
	static_assert(DBORDERX != 0, "RndLoc3x4 returns 0;0 position as a failed location.");
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
		return { xp, yp };
fail:
		if (++tries > 20000)
			break;
	}
	return { 0, 0 };
}

static POS32 RndLoc5x5()
{
	int xp, yp, i, j, tries;
	static_assert(DBORDERX != 0, "RndLoc5x5 returns 0;0 position as a failed location.");
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
		return { xp, yp };
fail:
		if (++tries > 20000)
			break;
	}
	return { 0, 0 };
}

static POS32 RndLoc7x5()
{
	int xp, yp, i, j, tries;
	static_assert(DBORDERX != 0, "RndLoc7x5 returns 0;0 position as a failed location.");
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
		return { xp, yp };
fail:
		if (++tries > 20000)
			break;
	}
	return { 0, 0 };
}

static POS32 RndLoc6x7()
{
	int xp, yp, i, j, tries;
	static_assert(DBORDERX != 0, "RndLoc6x7 returns 0;0 position as a failed location.");
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
		return { xp, yp };
fail:
		if (++tries > 20000)
			break;
	}
	return { 0, 0 };
}

static void InitRndLocObj(int min, int max, int objtype)
{
	int i, numobjs;
	POS32 pos;

	//assert(max >= min);
	//assert(max - min < 0x7FFF);
	numobjs = RandRangeLow(min, max);
	for (i = 0; i < numobjs; i++) {
		pos = RndLoc3x3();
		if (pos.x == 0)
			break;
		AddObject(objtype, pos.x, pos.y);
	}
}

static void InitRndSarcs(int objtype)
{
	int i, numobjs;
	POS32 pos;

	numobjs = RandRange(10, 15);
	for (i = 0; i < numobjs; i++) {
		pos = RndLoc3x4();
		if (pos.x == 0)
			break;
		AddObject(objtype, pos.x, pos.y);
	}
}

static void InitRndLocObj5x5(int objtype)
{
	POS32 pos = RndLoc5x5();

	if (pos.x != 0)
		AddObject(objtype, pos.x, pos.y);
}

void InitLevelObjects()
{
//	int i;

	numobjects = 0;

	//memset(objects, 0, sizeof(objects));
	//memset(objectactive, 0, sizeof(objectactive));

//	for (i = 0; i < MAXOBJECTS; i++)
//		objectavail[i] = i;

	trapid = 1;
	leverid = 1;
}

static void AddCandles()
{
	int tx, ty;

	tx = pWarps[DWARP_SIDE]._wx + 1;
	ty = pWarps[DWARP_SIDE]._wy;
	AddObject(OBJ_STORYCANDLE, tx - 2, ty + 1);
	AddObject(OBJ_STORYCANDLE, tx + 3, ty + 1);
	AddObject(OBJ_STORYCANDLE, tx - 1, ty + 2);
	AddObject(OBJ_STORYCANDLE, tx + 2, ty + 2);
}

static void AddBookLever(int type, int x1, int y1, int x2, int y2, int qn)
{
	int oi;
	POS32 pos;

	pos = RndLoc5x5();
	if (pos.x == 0)
		return;

	oi = AddObject(type, pos.x, pos.y);
	SetObjMapRange(oi, x1, y1, x2, y2, leverid);
	leverid++;
	objects[oi]._oVar6 = objects[oi]._oAnimFrame + 1; // LEVER_BOOK_ANIM
	objects[oi]._oVar7 = qn; // LEVER_BOOK_QUEST
}

static void InitRndBarrels(int otype)
{
	int i, xp, yp;
	int dir;
	int t; // number of tries of placing next barrel in current group
	int c; // number of barrels in current group

	static_assert((int)OBJ_BARREL + 1 == (int)OBJ_BARRELEX, "InitRndBarrels expects ordered BARREL enum I.");
#ifdef HELLFIRE
	static_assert((int)OBJ_URN + 1 == (int)OBJ_URNEX, "InitRndBarrels expects ordered BARREL enum II.");
	static_assert((int)OBJ_POD + 1 == (int)OBJ_PODEX, "InitRndBarrels expects ordered BARREL enum III.");
#endif

	// generate i number of groups of barrels
	for (i = RandRange(3, 7); i != 0; i--) {
		do {
			xp = random_(143, DSIZEX) + DBORDERX;
			yp = random_(143, DSIZEY) + DBORDERY;
		} while (!RndLocOk(xp, yp));
		AddObject(otype + (random_(143, 4) == 0 ? 1 : 0), xp, yp);
		c = 1;
		do {
			for (t = 0; t < 3; t++) {
				dir = random_(143, NUM_DIRS);
				xp += bxadd[dir];
				yp += byadd[dir];
				if (RndLocOk(xp, yp))
					break;
			}
			if (t == 3)
				break;
			AddObject(otype + (random_(143, 5) == 0 ? 1 : 0), xp, yp);
			c++;
		} while (random_low(143, c >> 1) == 0);
	}
}

static void AddDunObjs(int x1, int y1, int x2, int y2)
{
	int i, j, pn;

	assert((objectdata[OBJ_L1LDOOR].oLvlTypes & DTM_CATHEDRAL) && (objectdata[OBJ_L1RDOOR].oLvlTypes & DTM_CATHEDRAL) && (objectdata[OBJ_L1LIGHT].oLvlTypes & DTM_CATHEDRAL));
	assert(objectdata[OBJ_L1LDOOR].oSetLvlType == DTYPE_CATHEDRAL && objectdata[OBJ_L1RDOOR].oSetLvlType == DTYPE_CATHEDRAL && objectdata[OBJ_L1LIGHT].oSetLvlType == DTYPE_CATHEDRAL);
	assert((objectdata[OBJ_L2LDOOR].oLvlTypes & DTM_CATACOMBS) && (objectdata[OBJ_L2RDOOR].oLvlTypes & DTM_CATACOMBS));
	assert(objectdata[OBJ_L2LDOOR].oSetLvlType == DTYPE_CATACOMBS && objectdata[OBJ_L2RDOOR].oSetLvlType == DTYPE_CATACOMBS);
	assert((objectdata[OBJ_L3LDOOR].oLvlTypes & DTM_CAVES) && (objectdata[OBJ_L3RDOOR].oLvlTypes & DTM_CAVES));
	assert(objectdata[OBJ_L3LDOOR].oSetLvlType == DTYPE_CAVES && objectdata[OBJ_L3RDOOR].oSetLvlType == DTYPE_CAVES);
#ifdef HELLFIRE
	assert((objectdata[OBJ_L5LDOOR].oLvlTypes & DTM_CRYPT) && (objectdata[OBJ_L5RDOOR].oLvlTypes & DTM_CRYPT));
	assert(objectdata[OBJ_L5LDOOR].oSetLvlType == DTYPE_CRYPT && objectdata[OBJ_L5RDOOR].oSetLvlType == DTYPE_CRYPT);
#endif
	switch (currLvl._dType) {
	case DTYPE_CATHEDRAL:
		for (j = y1; j <= y2; j++) {
			for (i = x1; i <= x2; i++) {
				pn = dPiece[i][j];
				if (pn == 270)
					AddObject(OBJ_L1LIGHT, i, j);
				// these pieces are closed doors which are placed directly
				assert(pn != 51 && pn != 56);
				if (pn == 44 || /*pn == 51 ||*/ pn == 214)
					AddObject(OBJ_L1LDOOR, i, j);
				if (pn == 46 /*|| pn == 56*/)
					AddObject(OBJ_L1RDOOR, i, j);
			}
		}
		break;
	case DTYPE_CATACOMBS:
		for (j = y1; j <= y2; j++) {
			for (i = x1; i <= x2; i++) {
				pn = dPiece[i][j];
				// 13 and 17 pieces are open doors and not handled at the moment
				// 541 and 542 are doorways which are no longer handled as doors
				// 538 and 540 pieces are closed doors
				if (/*pn == 13 ||*/ pn == 538 /*|| pn == 541*/)
					AddObject(OBJ_L2LDOOR, i, j);
				if (/*pn == 17 ||*/ pn == 540 /*|| pn == 542*/)
					AddObject(OBJ_L2RDOOR, i, j);
			}
		}
		break;
	case DTYPE_CAVES:
		for (j = y1; j <= y2; j++) {
			for (i = x1; i <= x2; i++) {
				pn = dPiece[i][j];
				// 531 and 534 pieces are closed doors which are placed directly
				if (pn == 534)
					AddObject(OBJ_L3LDOOR, i, j);
				if (pn == 531)
					AddObject(OBJ_L3RDOOR, i, j);
			}
		}
		break;
	case DTYPE_HELL:
		break;
#ifdef HELLFIRE
	case DTYPE_CRYPT:
		for (j = y1; j <= y2; j++) {
			for (i = x1; i <= x2; i++) {
				pn = dPiece[i][j];
				// 77 and 80 pieces are closed doors which are placed directly
				if (pn == 77)
					AddObject(OBJ_L5LDOOR, i, j);
				if (pn == 80)
					AddObject(OBJ_L5RDOOR, i, j);
			}
		}
		break;
	case DTYPE_NEST:
		break;
#endif
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void AddL2Torches()
{
	int i, j;
	// place torches on NE->SW walls
	for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
		for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
			// skip setmap pieces
			if (dFlags[i][j] & BFLAG_POPULATED)
				continue;
			// select 'trapable' position
			if (nTrapTable[dPiece[i][j]] != PTT_LEFT)
				continue;
			if (random_(145, 32) != 0)
				continue;
			// assert(nSolidTable[dPiece[i][j - 1]] | nSolidTable[dPiece[i][j + 1]]);
			if (!nSolidTable[dPiece[i + 1][j]]) {
				AddObject(OBJ_TORCHL1, i, j);
			} else {
				AddObject(OBJ_TORCHL2, i - 1, j);
			}
			// skip a few tiles to prevent close placement
			j += 4;
		}
	}
	// place torches on NW->SE walls
	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			// skip setmap pieces
			if (dFlags[i][j] & BFLAG_POPULATED)
				continue;
			// select 'trapable' position
			if (nTrapTable[dPiece[i][j]] != PTT_RIGHT)
				continue;
			if (random_(145, 32) != 0)
				continue;
			// assert(nSolidTable[dPiece[i - 1][j]] | nSolidTable[dPiece[i + 1][j]]);
			if (!nSolidTable[dPiece[i][j + 1]]) {
				AddObject(OBJ_TORCHR1, i, j);
			} else {
				if (dObject[i][j - 1] == 0)
					AddObject(OBJ_TORCHR2, i, j - 1);
			}
			// skip a few tiles to prevent close placement
			i += 4;
		}
	}
}

static void AddObjTraps()
{
	int i, j, tx, ty, on, rndv;
	int8_t oi;

	rndv = 10 + (currLvl._dLevel >> 1);
	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			oi = dObject[i][j];
			oi--;
			if (oi < 0)
				continue;
			if (!objectdata[objects[oi]._otype].oTrapFlag)
				continue;

			if (random_(144, 128) >= rndv)
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
			if (dObject[tx][ty] != 0)
				continue;
			if (nTrapTable[dPiece[tx][ty]] == PTT_NONE)
				continue;
			on = AddObject(on, tx, ty);
			if (on == -1)
				return;
			objects[on]._oVar1 = oi; // TRAP_OI_REF
			objects[oi]._oTrapChance = RandRange(1, 64);
			objects[oi]._oVar5 = on + 1; // TRAP_OI_BACKREF
		}
	}
}

static void AddChestTraps()
{
	int i, j;
	int8_t oi;

	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			oi = dObject[i][j];
			if (oi > 0) {
				oi--;
				if (objects[oi]._otype >= OBJ_CHEST1 && objects[oi]._otype <= OBJ_CHEST3 && objects[oi]._oTrapChance == 0 && random_(0, 100) < 10) {
					objects[oi]._otype += OBJ_TCHEST1 - OBJ_CHEST1;
					objects[oi]._oTrapChance = RandRange(1, 64);
					objects[oi]._oVar5 = 0; // TRAP_OI_BACKREF
				}
			}
		}
	}
}

static void LoadMapSetObjects(const BYTE* map, int startx, int starty)
{
	const BYTE* pMap = map;
	int i, j;
	uint16_t rw, rh, *lm, oidx;

	//gbInitObjFlag = true;
	lm = (uint16_t*)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	lm += rw * rh; // skip dun
	rw <<= 1;
	rh <<= 1;
	lm += 2 * rw * rh; // skip items?, monsters

	startx += DBORDERX;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				oidx = SwapLE16(*lm); // index of ObjConvTbl
				assert(oidx < lengthof(ObjConvTbl));
				oidx = ObjConvTbl[oidx]; // index of objectdata
				assert(oidx != 0);
				AddObjectType(objectdata[oidx].ofindex);
				AddObject(oidx, i, j);
			}
			lm++;
		}
	}
	//gbInitObjFlag = false; -- setpieces, setmap levers?
}

static void LoadMapSetObjs(const BYTE* map)
{
	LoadMapSetObjects(map, 2 * pSetPieces[0]._spx, 2 * pSetPieces[0]._spy);
}

static void SetupObject(int oi, int type)
{
	ObjectStruct* os;
	const ObjectData* ods;
	const ObjFileData* ofd;

	os = &objects[oi];
	os->_otype = type;
	ods = &objectdata[type];
	os->_oSelFlag = ods->oSelFlag;
	os->_oDoorFlag = ods->oDoorFlag;
	os->_oProc = ods->oProc;
	os->_oModeFlags = ods->oModeFlags;
	os->_oAnimFrame = ods->oAnimBaseFrame;
	os->_oAnimData = objanimdata[ods->ofindex];
	ofd = &objfiledata[ods->ofindex];
	os->_oSFX = ofd->oSFX;
	os->_oSFXCnt = ofd->oSFXCnt;
	os->_oAnimFlag = ofd->oAnimFlag;
	os->_oAnimFrameLen = ofd->oAnimFrameLen;
	os->_oAnimLen = ofd->oAnimLen;
	//os->_oAnimCnt = 0;
	if (ofd->oAnimFlag != OAM_NONE) {
		os->_oAnimCnt = random_low(146, os->_oAnimFrameLen);
		os->_oAnimFrame = RandRangeLow(1, os->_oAnimLen);
	}
	os->_oAnimWidth = ofd->oAnimWidth * ASSET_MPL;
	os->_oAnimXOffset = (os->_oAnimWidth - TILE_WIDTH) >> 1;
	os->_oSolidFlag = ofd->oSolidFlag;
	os->_oMissFlag = ofd->oMissFlag;
	os->_oLightFlag = ofd->oLightFlag;
	os->_oBreak = ofd->oBreak;
	// os->_oDelFlag = FALSE; - unused
	os->_oPreFlag = FALSE;
	os->_oTrapChance = 0;
}

static int ObjIndex(int x, int y)
{
	int oi = dObject[x][y];
#if DEBUG_MODE
	if (oi == 0) {
		app_fatal("ObjIndex: Active object not found at (%d,%d)", x, y);
	}
#endif
	oi = oi >= 0 ? oi - 1 : -(oi + 1);
	return oi;
}

static void AddDiabObjs()
{
	LoadMapSetObjects(pSetPieces[0]._spData, 2 * pSetPieces[0]._spx, 2 * pSetPieces[0]._spy);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[0]._spx + 5, DBORDERY + 2 * pSetPieces[0]._spy + 5), pSetPieces[1]._spx, pSetPieces[1]._spy, pSetPieces[1]._spx + 11, pSetPieces[1]._spy + 12, 1);
	LoadMapSetObjects(pSetPieces[1]._spData, 2 * pSetPieces[1]._spx, 2 * pSetPieces[1]._spy);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[1]._spx + 13, DBORDERY + 2 * pSetPieces[1]._spy + 10), pSetPieces[2]._spx, pSetPieces[2]._spy, pSetPieces[2]._spx + 11, pSetPieces[2]._spy + 11, 2);
	LoadMapSetObjects(pSetPieces[2]._spData, 2 * pSetPieces[2]._spx, 2 * pSetPieces[2]._spy);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[2]._spx + 8, DBORDERY + 2 * pSetPieces[2]._spy + 2), pSetPieces[3]._spx, pSetPieces[3]._spy, pSetPieces[3]._spx + 9, pSetPieces[3]._spy + 9, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[2]._spx + 8, DBORDERY + 2 * pSetPieces[2]._spy + 14), pSetPieces[3]._spx, pSetPieces[3]._spy, pSetPieces[3]._spx + 9, pSetPieces[3]._spy + 9, 3);
}

#ifdef HELLFIRE
static void AddHBooks(int bookidx, int ox, int oy)
{
	ObjectStruct* os;
	constexpr int bookframe = 1;
	int oi = AddObject(OBJ_L5BOOK, ox, oy);

	if (oi == -1)
		return;

	os = &objects[oi];
	// os->_oVar1 = bookframe;
	os->_oAnimFrame = 5 - 2 * bookframe;
	os->_oVar4 = os->_oAnimFrame + 1; // STORY_BOOK_READ_FRAME
	if (bookidx >= QNB_BOOK_A) {
		os->_oVar2 = TEXT_BOOKA + bookidx - QNB_BOOK_A; // STORY_BOOK_MSG
		os->_oVar3 = 14;                                // STORY_BOOK_NAME
		os->_oVar8 = bookidx;                           // STORY_BOOK_NAKRUL_IDX
	} else {
		os->_oVar2 = TEXT_BOOK4 + bookidx; // STORY_BOOK_MSG
		os->_oVar3 = bookidx + 9;          // STORY_BOOK_NAME
		os->_oVar8 = 0;                    // STORY_BOOK_NAKRUL_IDX
	}
}

static void AddLvl2xBooks(int bookidx)
{
	POS32 pos = RndLoc7x5();

	if (pos.x == 0)
		return;

	AddHBooks(bookidx, pos.x, pos.y);
	AddObject(OBJ_L5CANDLE, pos.x - 2, pos.y + 1);
	AddObject(OBJ_L5CANDLE, pos.x - 2, pos.y);
	AddObject(OBJ_L5CANDLE, pos.x - 1, pos.y - 1);
	AddObject(OBJ_L5CANDLE, pos.x + 1, pos.y - 1);
	AddObject(OBJ_L5CANDLE, pos.x + 2, pos.y);
	AddObject(OBJ_L5CANDLE, pos.x + 2, pos.y + 1);
}

static void AddUberLever()
{
	int oi;

	oi = AddObject(OBJ_L5LEVER, 2 * pSetPieces[0]._spx + DBORDERX + 7, 2 * pSetPieces[0]._spy + DBORDERY + 5);
	SetObjMapRange(oi, pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 2, pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 3, 1);
}

static void AddLvl24Books()
{
	BYTE books[4];

	AddUberLever();
	switch (random_(0, 6)) {
	case 0:
		books[0] = QNB_BOOK_A; books[1] = QNB_BOOK_B; books[2] = QNB_BOOK_C; books[3] = 0;
		break;
	case 1:
		books[0] = QNB_BOOK_A; books[1] = QNB_BOOK_C; books[2] = QNB_BOOK_B; books[3] = 0;
		break;
	case 2:
		books[0] = QNB_BOOK_B; books[1] = QNB_BOOK_A; books[2] = QNB_BOOK_C; books[3] = 0;
		break;
	case 3:
		books[0] = QNB_BOOK_B; books[1] = QNB_BOOK_C; books[2] = QNB_BOOK_A; books[3] = 0;
		break;
	case 4:
		books[0] = QNB_BOOK_C; books[1] = QNB_BOOK_B; books[2] = QNB_BOOK_A; books[3] = 0;
		break;
	case 5:
		books[0] = QNB_BOOK_C; books[1] = QNB_BOOK_A; books[2] = QNB_BOOK_B; books[3] = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	AddHBooks(books[0], 2 * pSetPieces[0]._spx + DBORDERX + 7, 2 * pSetPieces[0]._spy + DBORDERY + 6);
	AddHBooks(books[1], 2 * pSetPieces[0]._spx + DBORDERX + 6, 2 * pSetPieces[0]._spy + DBORDERY + 3);
	AddHBooks(books[2], 2 * pSetPieces[0]._spx + DBORDERX + 6, 2 * pSetPieces[0]._spy + DBORDERY + 8);
}

static int ProgressUberLever(int bookidx, int status)
{
	static_assert((int)QV_NAKRUL_BOOKOPEN < (int)QV_NAKRUL_LEVEROPEN, "ProgressUberLever might reset QV_NAKRUL_LEVEROPEN.");
	if (status >= QV_NAKRUL_BOOKOPEN)
		return status;

	switch (bookidx) {
	case QNB_BOOK_A:
		return QV_NAKRUL_BOOKA;
	case QNB_BOOK_B:
		return status == QV_NAKRUL_BOOKA ? QV_NAKRUL_BOOKB : QV_INIT;
	case QNB_BOOK_C:
		return status == QV_NAKRUL_BOOKB ? QV_NAKRUL_BOOKC : QV_INIT;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return 0;
}
#endif

static void Alloc2x2Obj(int oi)
{
	int ox, oy;

	ox = objects[oi]._ox;
	oy = objects[oi]._oy;
	oi = -(oi + 1);
	dObject[ox][oy - 1] = oi;
	dObject[ox - 1][oy] = oi;
	dObject[ox - 1][oy - 1] = oi;
}

static void AddStoryBook()
{
	POS32 pos = RndLoc7x5();

	if (pos.x == 0)
		return;

	AddObject(OBJ_STORYBOOK, pos.x, pos.y);
	AddObject(OBJ_STORYCANDLE, pos.x - 2, pos.y + 1);
	AddObject(OBJ_STORYCANDLE, pos.x - 2, pos.y);
	AddObject(OBJ_STORYCANDLE, pos.x - 1, pos.y - 1);
	AddObject(OBJ_STORYCANDLE, pos.x + 1, pos.y - 1);
	AddObject(OBJ_STORYCANDLE, pos.x + 2, pos.y);
	AddObject(OBJ_STORYCANDLE, pos.x + 2, pos.y + 1);
}

static void AddHookedBodies()
{
	int i, j, ttv, type;
	// TODO: straight loop (in dlrgs)?
	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			ttv = nTrapTable[dPiece[i][j]];
			if (ttv == PTT_NONE)
				continue;
			if (dFlags[i][j] & BFLAG_POPULATED)
				continue;
			type = random_(0, 32);
			if (type >= 3)
				continue;
			if (ttv == PTT_LEFT) {
				type = OBJ_TORTUREL1 + type;
			} else {
				// assert(ttv == PTT_RIGHT);
				type = OBJ_TORTURER1 + type;
			}
			AddObject(type, i, j);
		}
	}
}

static void AddL4Goodies()
{
	AddHookedBodies();
	InitRndLocObj(2 * 4, 6 * 4, OBJ_TNUDEM);
	InitRndLocObj(2 * 3, 6 * 3, OBJ_TNUDEW);
	InitRndLocObj(2, 6, OBJ_DECAP);
	InitRndLocObj(1, 3, OBJ_CAULDRON);
}

static void AddLazStand()
{
	POS32 pos;

	if (IsMultiGame) {
		AddObject(OBJ_ALTBOY, 2 * pSetPieces[0]._spx + DBORDERX + 4, 2 * pSetPieces[0]._spy + DBORDERY + 6);
		return;
	}
	pos = RndLoc6x7();
	if (pos.x == 0) {
		InitRndLocObj(1, 1, OBJ_LAZSTAND);
		return;
	}
	AddObject(OBJ_LAZSTAND, pos.x, pos.y);
	AddObject(OBJ_TNUDEM, pos.x, pos.y + 2);
	AddObject(OBJ_STORYCANDLE, pos.x + 1, pos.y + 2);
	AddObject(OBJ_TNUDEM, pos.x + 2, pos.y + 2);
	AddObject(OBJ_TNUDEW, pos.x, pos.y - 2);
	AddObject(OBJ_STORYCANDLE, pos.x + 1, pos.y - 2);
	AddObject(OBJ_TNUDEW, pos.x + 2, pos.y - 2);
	AddObject(OBJ_STORYCANDLE, pos.x - 1, pos.y - 1);
	AddObject(OBJ_TNUDEW, pos.x - 1, pos.y);
	AddObject(OBJ_STORYCANDLE, pos.x - 1, pos.y + 1);
}

void InitObjects()
{
	//gbInitObjFlag = true;
	if (QuestStatus(Q_ROCK)) // place first to make the life of PlaceRock easier
		InitRndLocObj5x5(OBJ_STAND);
	if (QuestStatus(Q_PWATER))
		AddCandles();
	if (QuestStatus(Q_MUSHROOM))
		InitRndLocObj5x5(OBJ_MUSHPATCH);
	if (pSetPieces[0]._sptype == SPT_BUTCHER) // QuestStatus(Q_BUTCHER)
		LoadMapSetObjs(pSetPieces[0]._spData);
	if (pSetPieces[0]._sptype == SPT_BANNER) { // QuestStatus(Q_BANNER)
		if (pSetPieces[0]._spData != NULL) {
		// patch set-piece to add objects - Banner2.DUN
		uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
		// add chest
		lm[2 + 8 * 8 + 8 * 8 * 2 * 2 + 8 * 8 * 2 * 2 + 10 + 3 * 8 * 2] = 90;
		}
		LoadMapSetObjs(pSetPieces[0]._spData);
	}
	if (pSetPieces[0]._sptype == SPT_BCHAMB) { // QuestStatus(Q_BCHAMB)
		AddBookLever(OBJ_BOOK2R, pSetPieces[0]._spx, pSetPieces[0]._spy, pSetPieces[0]._spx + 5, pSetPieces[0]._spy + 5, Q_BCHAMB);
	}
	if (pSetPieces[0]._sptype == SPT_BLIND) { // QuestStatus(Q_BLIND)
		AddBookLever(OBJ_BLINDBOOK, pSetPieces[0]._spx, pSetPieces[0]._spy + 1, pSetPieces[0]._spx + 11, pSetPieces[0]._spy + 10, Q_BLIND);
	}
	if (pSetPieces[0]._sptype == SPT_BLOOD) { // QuestStatus(Q_BLOOD)
		if (pSetPieces[0]._spData != NULL) {
		// patch set-piece to add objects - Blood2.DUN
		uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
		// add book and pedistal
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 9 + 24 * 10 * 2] = 15;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 9 + 16 * 10 * 2] = 91;
		// remove torches
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 8 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 10 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 12 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 8 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 10 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 12 * 10 * 2] = 0;
		}
		LoadMapSetObjs(pSetPieces[0]._spData);
	}
	if (pSetPieces[0]._sptype == SPT_WARLORD) { // QuestStatus(Q_WARLORD)
		AddBookLever(OBJ_STEELTOME, pSetPieces[0]._spx + 7, pSetPieces[0]._spy + 1, pSetPieces[0]._spx + 7, pSetPieces[0]._spy + 5, Q_WARLORD);
		// patch set-piece to add objects - Warlord2.DUN
		uint16_t* lm = (uint16_t*)pSetPieces[0]._spData;
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 2 + 3 * 8 * 2] = 108;
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 2 + 9 * 8 * 2] = 108;
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 5 + 2 * 8 * 2] = 109;
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 + 2 * 8 * 2] = 109;
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 5 + 10 * 8 * 2] = 109;
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 + 10 * 8 * 2] = 109;
		LoadMapSetObjs(pSetPieces[0]._spData);
	}
	switch (currLvl._dLevelIdx) {
	case DLV_CATHEDRAL4:
		AddStoryBook();
		break;
	case DLV_CATACOMBS4:
		AddStoryBook();
		break;
	case DLV_CAVES1:
		if (!IsMultiGame)
			InitRndLocObj5x5(OBJ_SLAINHERO);
		break;
	case DLV_CAVES4:
		AddStoryBook();
		break;
	case DLV_HELL4:
		AddDiabObjs();
		return;
	case DLV_HELL3: // QuestStatus(Q_BETRAYER) / setpc_type == SPT_BETRAYER (single?)
		AddLazStand();
		break;
#ifdef HELLFIRE
	case DLV_CRYPT1:
		AddLvl2xBooks(QNB_BOOK_1);
		break;
	case DLV_CRYPT2:
		AddLvl2xBooks(QNB_BOOK_2);
		AddLvl2xBooks(QNB_BOOK_3);
		break;
	case DLV_CRYPT3:
		AddLvl2xBooks(QNB_BOOK_4);
		AddLvl2xBooks(QNB_BOOK_5);
		break;
	case DLV_CRYPT4:
		AddLvl24Books();
		break;
#endif
	}
	AddDunObjs(DBORDERX, DBORDERY, MAXDUNX - DBORDERX - 1, MAXDUNY - DBORDERY - 1);
	BYTE lvlMask = 1 << currLvl._dType;
	if (lvlMask & objectdata[OBJ_SARC].oLvlTypes) {
		InitRndSarcs(OBJ_SARC);
	}
#ifdef HELLFIRE
	if (lvlMask & objectdata[OBJ_L5SARC].oLvlTypes) {
		InitRndSarcs(OBJ_L5SARC);
	}
#endif
	assert(objectdata[OBJ_TORCHL1].oLvlTypes == objectdata[OBJ_TORCHL2].oLvlTypes && objectdata[OBJ_TORCHL1].oLvlTypes == objectdata[OBJ_TORCHR1].oLvlTypes && objectdata[OBJ_TORCHR1].oLvlTypes == objectdata[OBJ_TORCHR2].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TORCHL1].oLvlTypes) {
		AddL2Torches();
	}
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TNUDEW].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_DECAP].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_CAULDRON].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TORTUREL1].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TORTUREL2].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TORTUREL3].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TORTURER1].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TORTURER2].oLvlTypes);
	assert(objectdata[OBJ_TNUDEM].oLvlTypes == objectdata[OBJ_TORTURER3].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TNUDEM].oLvlTypes) {
		AddL4Goodies();
	}
	assert(objectdata[OBJ_BARREL].oLvlTypes == objectdata[OBJ_BARRELEX].oLvlTypes);
	if (lvlMask & objectdata[OBJ_BARREL].oLvlTypes) {
		InitRndBarrels(OBJ_BARREL);
	}
#ifdef HELLFIRE
	assert(objectdata[OBJ_URN].oLvlTypes == objectdata[OBJ_URNEX].oLvlTypes);
	if (lvlMask & objectdata[OBJ_URN].oLvlTypes) {
		InitRndBarrels(OBJ_URN);
	}
	assert(objectdata[OBJ_POD].oLvlTypes == objectdata[OBJ_PODEX].oLvlTypes);
	if (lvlMask & objectdata[OBJ_POD].oLvlTypes) {
		InitRndBarrels(OBJ_POD);
	}
#endif
	assert(objectdata[OBJ_CHEST1].oLvlTypes == DTM_ANY && objectdata[OBJ_CHEST2].oLvlTypes == DTM_ANY && objectdata[OBJ_CHEST3].oLvlTypes == DTM_ANY);
	InitRndLocObj(5, 10, OBJ_CHEST1);
	InitRndLocObj(3, 6, OBJ_CHEST2);
	InitRndLocObj(1, 5, OBJ_CHEST3);
	assert(objectdata[OBJ_TRAPL].oLvlTypes == objectdata[OBJ_TRAPR].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TRAPL].oLvlTypes) {
		AddObjTraps();
	}
	assert(objectdata[OBJ_TCHEST1].oLvlTypes == objectdata[OBJ_TCHEST2].oLvlTypes && objectdata[OBJ_TCHEST1].oLvlTypes == objectdata[OBJ_TCHEST3].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TCHEST1].oLvlTypes) {
		AddChestTraps();
	}
	//gbInitObjFlag = false;
}

void SetMapObjects(BYTE* pMap)
{
	int i;
	//gbInitObjFlag = true;

	for (i = 0; i < NUM_OBJECTS; i++) {
		if (currLvl._dType == objectdata[i].oSetLvlType)
			AddObjectType(objectdata[i].ofindex);
	}

	AddDunObjs(DBORDERX, DBORDERY, MAXDUNX - DBORDERX - 1, MAXDUNY - DBORDERY - 1);

	LoadMapSetObjects(pMap, 0, 0);
	//gbInitObjFlag = false; -- setmap levers?
}

/*static void DeleteObject_(int oi, int idx)
{
	//objectavail[MAXOBJECTS - numobjects] = oi;
	dObject[objects[oi]._ox][objects[oi]._oy] = 0;
	//objectavail[numobjects] = oi;
	numobjects--;
	if (numobjects > 0 && idx != numobjects)
		objectactive[idx] = objectactive[numobjects];
}*/

void SetObjMapRange(int oi, int x1, int y1, int x2, int y2, int v)
{
	ObjectStruct* os;

	os = &objects[oi];
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
	ObjectStruct* os;
	int num, rnum, itype;

	os = &objects[oi];
	if (random_(147, 2) == 0)
		os->_oAnimFrame += 3;
	os->_oRndSeed = NextRndSeed(); // CHEST_ITEM_SEED1
	//assert(os->_otype >= OBJ_CHEST1 && os->_otype <= OBJ_CHEST3
	//	|| os->_otype >= OBJ_TCHEST1 && os->_otype <= OBJ_TCHEST3);
	num = os->_otype;
	num = (num >= OBJ_TCHEST1 && num <= OBJ_TCHEST3) ? num - OBJ_TCHEST1 + 1 : num - OBJ_CHEST1 + 1;
	rnum = random_low(147, num + 1); // CHEST_ITEM_SEED2
	if (!currLvl._dSetLvl)
		num = rnum;
	os->_oVar1 = num;        // CHEST_ITEM_NUM
	itype = random_(147, 8); // CHEST_ITEM_SEED3
	if (currLvl._dSetLvl)
		itype = 8;
	os->_oVar2 = itype;      // CHEST_ITEM_TYPE
	//assert(num <= 3); otherwise the seeds are not 'reserved'
}

static void OpenDoor(int oi);
static void AddDoor(int oi)
{
	ObjectStruct* os;
	int x, y, bx, by;

	os = &objects[oi];
	x = os->_ox;
	y = os->_oy;
	os->_oVar4 = DOOR_CLOSED;
	//os->_oPreFlag = FALSE;
	//os->_oSelFlag = 3;
	//os->_oSolidFlag = FALSE; // TODO: should be TRUE;
	//os->_oMissFlag = FALSE;
	//os->_oDoorFlag = ldoor ? ODT_LEFT : ODT_RIGHT;
	os->_oVar1 = dPiece[x][y]; // DOOR_PIECE_CLOSED
	// DOOR_SIDE_PIECE_CLOSED
	bx = x;
	by = y;
	if (os->_oDoorFlag == ODT_LEFT)
		by--;
	else
		bx--;
	os->_oVar2 = dPiece[bx][by];
}

static void AddSarc(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	dObject[os->_ox][os->_oy - 1] = -(oi + 1);
	os->_oVar1 = random_(153, 10);       // SARC_ITEM
	os->_oRndSeed = NextRndSeed();
	if (os->_oVar1 >= 8)
		os->_oVar2 = PreSpawnSkeleton(); // SARC_SKELE
}

/*static void AddFlameTrap(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oVar1 = trapid; // FLAMETRAP_ID
	os->_oVar2 = TRAP_ACTIVE;
	os->_oVar3 = FLAMETRAP_DIR_Y;
	os->_oVar4 = FLAMETRAP_FIRE_INACTIVE;
}

static void AddFlameLever(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oVar1 = trapid; // FLAMETRAP_ID
}*/

static void AddTrap(int oi)
{
	ObjectStruct* os;
	int mt;

	mt = currLvl._dLevel;
	mt = mt / 6 + 1;
	mt = random_low(148, mt) & 3;
	os = &objects[oi];
	os->_oRndSeed = NextRndSeed();
	// TRAP_MISTYPE
	os->_oVar3 = MIS_ARROW;
	if (mt == 1)
		os->_oVar3 = MIS_FIREBOLT;
	else if (mt == 2)
		os->_oVar3 = MIS_LIGHTNINGC;
	os->_oVar4 = TRAP_ACTIVE; // TRAP_LIVE
}

static void AddObjLight(int oi, int diffr, int dx, int dy)
{
	ObjectStruct* os;

	os = &objects[oi];
	//if (gbInitObjFlag) {
#if FLICKER_LIGHT
		if (diffr == 0)
			os->_olid = NO_LIGHT;
		else
#endif
			DoLighting(os->_ox + dx, os->_oy + dy, diffr, NO_LIGHT);
	//}
}

static void AddBarrel(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oRndSeed = NextRndSeed();
	os->_oVar3 = random_(149, 3);  // BARREL_ITEM_TYPE
	os->_oVar2 = random_(149, 10); // BARREL_ITEM
	if (os->_oVar2 >= 8)
		os->_oVar4 = PreSpawnSkeleton(); // BARREL_SKELE
}

static int FindValidShrine(int filter)
{
	int rv;
	BYTE excl = IsMultiGame ? SHRINETYPE_SINGLE : SHRINETYPE_MULTI;

	while (TRUE) {
		rv = random_(0, NUM_SHRINETYPE);
		if (rv != filter && shrineavail[rv] != excl)
			break;
	}
	return rv;
}

static void AddShrine(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oPreFlag = TRUE;
	os->_oRndSeed = NextRndSeed();
	os->_oVar1 = FindValidShrine(NUM_SHRINETYPE); // SHRINE_TYPE
	if (random_(150, 2) != 0) {
		os->_oAnimFrame = 12;
		os->_oAnimLen = 22;
	}
}

static void AddBookcase(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oRndSeed = NextRndSeed();
	os->_oPreFlag = TRUE;
}

static void ObjAddRndSeed(int oi)
{
	objects[oi]._oRndSeed = NextRndSeed();
}

static void ObjAddBloodBook(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oRndSeed = NextRndSeed();
	os->_oVar6 = os->_oAnimFrame + 1; // LEVER_BOOK_ANIM
	os->_oVar7 = Q_BLOOD; // LEVER_BOOK_QUEST
	SetObjMapRange(oi, 0, 0, 0, 0, leverid); // NULL_LVR_EFFECT
	leverid++;
}

static void AddArmorStand(int oi)
{
	objects[oi]._oMissFlag = TRUE;
}

static void AddCauldronGoatShrine(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oRndSeed = NextRndSeed();
	os->_oVar1 = FindValidShrine(SHRINE_THAUMATURGIC); // SHRINE_TYPE
}

static void AddDecap(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oRndSeed = NextRndSeed();
	os->_oAnimFrame = RandRange(1, 8);
	os->_oPreFlag = TRUE;
}

static void AddVileBook(int oi)
{
	if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		objects[oi]._oAnimFrame = 4;
	}
}

static void AddMagicCircle(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	//os->_oRndSeed = NextRndSeed();
	os->_oPreFlag = TRUE;
	os->_oVar5 = 0; // VILE_CIRCLE_PROGRESS
}

static void AddStoryBook(int oi)
{
	ObjectStruct* os;
	BYTE bookframe, idx;

	static_assert((int)DLV_CATHEDRAL4 == 4, "AddStoryBook converts DLV to index with shift I.");
	static_assert((int)DLV_CATACOMBS4 == 8, "AddStoryBook converts DLV to index with shift II.");
	static_assert((int)DLV_CAVES4 == 12, "AddStoryBook converts DLV to index with shift III.");
	idx = (currLvl._dLevelIdx >> 2) - 1;
	bookframe = quests[Q_DIABLO]._qvar1;

	os = &objects[oi];
	// os->_oVar1 = bookframe;
	os->_oVar2 = StoryText[bookframe][idx]; // STORY_BOOK_MSG
	os->_oVar3 = 3 * bookframe + idx;       // STORY_BOOK_NAME
	os->_oAnimFrame = 5 - 2 * bookframe;    //
	os->_oVar4 = os->_oAnimFrame + 1;       // STORY_BOOK_READ_FRAME
}

static void AddWeaponRack(int oi)
{
	objects[oi]._oMissFlag = TRUE;
}

static void AddTorturedMaleBody(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	//os->_oRndSeed = NextRndSeed();
	os->_oAnimFrame = RandRange(1, 4);
	//os->_oPreFlag = TRUE;
}

static void AddTorturedFemaleBody(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	//os->_oRndSeed = NextRndSeed();
	os->_oAnimFrame = RandRange(1, 3);
	//os->_oPreFlag = TRUE;
}

static void SyncL1Doors(int oi);
static void SyncL2Doors(int oi);
static void SyncL3Doors(int oi);
#ifdef HELLFIRE
static void SyncL5Doors(int oi);
#endif
int AddObject(int type, int ox, int oy)
{
	int oi;

	if (numobjects >= MAXOBJECTS)
		return -1;

//	oi = objectavail[0];
	oi = numobjects;
	// objectactive[numobjects] = oi;
	numobjects++;
//	objectavail[0] = objectavail[MAXOBJECTS - numobjects];
	SetupObject(oi, type);
	// place object
	ObjectStruct* os = &objects[oi];
	os->_ox = ox;
	os->_oy = oy;
	assert(dObject[ox][oy] == 0);
	dObject[ox][oy] = oi + 1;
	// dFlags[ox][oy] |= BFLAG_POPULATED;
	if (nSolidTable[dPiece[ox][oy]] && (os->_oModeFlags & OMF_FLOOR)) {
		dObject[ox][oy] = 0;
		os->_oModeFlags |= OMF_RESERVED;
		os->_oSelFlag = 0;
	}
	// init object
	switch (type) {
	case OBJ_L1LIGHT:
#if FLICKER_LIGHT
		AddObjLight(oi, 0, 0, 0);
#else
		AddObjLight(oi, 10, 0, 0);
#endif
		break;
	case OBJ_SKFIRE:
	//case OBJ_CANDLE1:
	case OBJ_CANDLE2:
	case OBJ_BOOKCANDLE:
		AddObjLight(oi, 5, 0, 0);
		break;
	case OBJ_STORYCANDLE:
#ifdef HELLFIRE
	case OBJ_L5CANDLE:
#endif
		AddObjLight(oi, 3, 0, 0);
		break;
	case OBJ_TORCHL1:
		AddObjLight(oi, 8, 1, 0);
		break;
	case OBJ_TORCHR1:
		AddObjLight(oi, 8, 0, 1);
		break;
	case OBJ_TORCHR2:
	case OBJ_TORCHL2:
		AddObjLight(oi, 8, 0, 0);
		break;
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
#endif
		AddDoor(oi);
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
		objects[oi]._oTrapChance = RandRange(1, 64);
		objects[oi]._oVar5 = 0; // TRAP_OI_BACKREF
		break;
	case OBJ_SARC:
#ifdef HELLFIRE
	case OBJ_L5SARC:
#endif
		AddSarc(oi);
		break;
	/*case OBJ_FLAMEHOLE:
		AddFlameTrap(oi);
		break;
	case OBJ_FLAMELVR:
		AddFlameLever(oi);
		break;
	case OBJ_WATER:
		objects[oi]._oAnimFrame = 1;
		break;*/
	case OBJ_TRAPL:
	case OBJ_TRAPR:
		AddTrap(oi);
		break;
	case OBJ_BARREL:
#ifdef HELLFIRE
	case OBJ_URN:
	case OBJ_POD:
#endif
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
	case OBJ_DECAP:
		AddDecap(oi);
		break;
	case OBJ_BARRELEX:
#ifdef HELLFIRE
	case OBJ_URNEX:
	case OBJ_PODEX:
#endif
	case OBJ_BOOKSTAND:
	case OBJ_SKELBOOK:
	case OBJ_PEDESTAL:
	case OBJ_ARMORSTAND:
	case OBJ_WEAPONRACKL:
	case OBJ_WEAPONRACKR:
	case OBJ_SLAINHERO:
		ObjAddRndSeed(oi);
		break;
	case OBJ_BLOODBOOK:
		ObjAddBloodBook(oi);
		break;
	case OBJ_ARMORSTANDN:
		AddArmorStand(oi);
		break;
	case OBJ_WEAPONRACKLN:
	case OBJ_WEAPONRACKRN:
		AddWeaponRack(oi);
		break;
	case OBJ_GOATSHRINE:
	case OBJ_CAULDRON:
		AddCauldronGoatShrine(oi);
		break;
	case OBJ_PURIFYINGFTN:
	case OBJ_MURKYFTN:
	case OBJ_MUSHPATCH:
		Alloc2x2Obj(oi);
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
	case OBJ_TBCROSS:
		// ObjAddRndSeed(oi);
		AddObjLight(oi, 10, 0, 0);
		break;
	case OBJ_TNUDEM:
		AddTorturedMaleBody(oi);
		break;
	case OBJ_TNUDEW:
		AddTorturedFemaleBody(oi);
		break;
	}
	return oi;
}
#if FLICKER_LIGHT
static void Obj_Light(int oi)
{
	ObjectStruct* os;
	int ox, oy, tr;
	bool turnon;
	static_assert(MAX_LIGHT_RAD >= 9, "Obj_Light needs at least light-radius of 9.");
	const int lr = 8;

	os = &objects[oi];
	ox = os->_ox;
	oy = os->_oy;
	turnon = false;
#if DEBUG_MODE
	if (!lightflag)
#endif
	{
		tr = lr + 1 + (gsTileVp._vColumns + gsTileVp._vRows / 2) / 2;
		turnon = abs(ViewX - ox) < tr && abs(ViewY - oy) < tr;
	}
	if (turnon) {
		assert(objectdata[OBJ_L1LIGHT].ofindex == OFILE_L1BRAZ);
		assert(objfiledata[OFILE_L1BRAZ].oAnimFrameLen < lengthof(flickers));
		tr = lr + flickers[os->_oAnimFrame];
		if (os->_olid == NO_LIGHT)
			os->_olid = AddLight(ox, oy, tr);
		else {
			if (LightList[os->_olid]._lradius != tr)
				ChangeLightRadius(os->_olid, tr);
		}
	} else {
		if (os->_olid != NO_LIGHT) {
			AddUnLight(os->_olid);
			os->_olid = NO_LIGHT;
		}
	}
}
#endif
static void FindClosestPlr(int* dx, int* dy)
{
	int xx, yy, j, i;
	const int8_t* cr;

	for (i = 0; i < 10; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = *cr; j > 0; j--) {
			xx = *dx + *++cr;
			yy = *dy + *++cr;
			if (PosOkActor(xx, yy) && PosOkPortal(xx, yy)) {
				*dx = xx;
				*dy = yy;
				return;
			}
		}
	}
}

static void Obj_Circle(int oi)
{
	ObjectStruct* os;
	int ox, oy;

	os = &objects[oi];
	ox = os->_ox;
	oy = os->_oy;
	if (myplr._pmode == PM_STAND && myplr._px == ox && myplr._py == oy) {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oAnimFrame = 2;
		else {
			//assert(os->_otype == OBJ_MCIRCLE2);
			os->_oAnimFrame = 4;
		}
		if (ox == DBORDERX + 19 && oy == DBORDERY + 20 && os->_oVar5 == 2) { // VILE_CIRCLE_PROGRESS
			if (/*quests[Q_BETRAYER]._qactive == QUEST_ACTIVE &&*/ quests[Q_BETRAYER]._qvar1 < QV_BETRAYER_CENTRALOPEN) {
				quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_CENTRALOPEN;
				// ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, true*/); // LEVER_EFFECT
				ObjChangeMap(7, 11, 13, 18/*, true*/);
			}
			assert(currLvl._dLevelIdx == SL_VILEBETRAYER);
			AddMissile(0, 0, LAZ_CIRCLE_X, LAZ_CIRCLE_Y, 0, MIS_RNDTELEPORT, MST_OBJECT, mypnum, 0);
			gbActionBtnDown = false;
			gbAltActionBtnDown = false;
			ClrPlrPath(mypnum);
			myplr._pdir = DIR_NW;
			myplr._pAnimData = myplr._pAnims[PGX_STAND].paAnimData[DIR_NW];
			myplr._pDestAction = ACTION_NONE;
		}
	} else {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oAnimFrame = 1;
		else {
			//assert(os->_otype == OBJ_MCIRCLE2);
			os->_oAnimFrame = 3;
		}
	}
}

static void Obj_Door(int oi)
{
	ObjectStruct* os;
	int dx, dy;
	bool dok;

	os = &objects[oi];
	if (os->_oVar4 == DOOR_CLOSED)
		return;
	dx = os->_ox;
	dy = os->_oy;
	dok = (dMonster[dx][dy] | dItem[dx][dy] | dDead[dx][dy] | dPlayer[dx][dy]) == 0;
	os->_oVar4 = dok ? DOOR_OPEN : DOOR_BLOCKED;
}

/*static void ActivateTrapLine(int tid)
{
	ObjectStruct* os;
	int i;

	for (i = 0; i < numobjects; i++) {
		os = &objects[objectactive[i]];
		if (os->_otype == OBJ_FLAMEHOLE && os->_oVar1 == tid) { // FLAMETRAP_ID
			os->_oVar4 = FLAMETRAP_FIRE_ACTIVE;
			os->_oAnimFlag = OAM_LOOP;
			//os->_oAnimFrameLen = 1;
			static_assert(MAX_LIGHT_RAD >= 1, "ActivateTrapLine needs at least light-radius of 1.");
			os->_olid = AddLight(os->_ox, os->_oy, 1);
		}
	}
}

static void Obj_FlameTrap(int oi)
{
	ObjectStruct* os;
	int x, y;
	int i;

	os = &objects[oi];
	if (os->_oVar2 != TRAP_ACTIVE) {
		if (os->_oVar4 != FLAMETRAP_FIRE_INACTIVE) {
			os->_oAnimFrame--;
			if (os->_oAnimFrame == 1) {
				os->_oVar4 = FLAMETRAP_FIRE_INACTIVE;
				AddUnLight(os->_olid);
			} else if (os->_oAnimFrame <= 4) {
				static_assert(MAX_LIGHT_RAD >= 4, "Obj_FlameTrap needs at least light-radius of 4.");
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
		if (os->_oAnimFrame == 11) {
			SetRndSeed(os->_oRndSeed);
			AddMissile(os->_ox, os->_oy, 0, 0, 0, MIS_FIRETRAP, MST_OBJECT, -1, 0);
			os->_oRndSeed = NextRndSeed();
		} else if (os->_oAnimFrame <= 5) {
			static_assert(MAX_LIGHT_RAD >= 5, "Obj_FlameTrap needs at least light-radius of 5.");
			ChangeLightRadius(os->_olid, os->_oAnimFrame);
		}
	}
}*/

static void Obj_Trap(int oi)
{
	ObjectStruct *os, *on;
	int i, dir, trigNum;
	const POS32* trigArea;
	const POS32 baseTrigArea[] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 0 }, { -1, 1 }, { 0, -1 }, { -1, -1 }, { 1, -1 } };
	const POS32 sarcTrigArea[] = { { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 0 }, { -1, 1 }, { 0, -2 }, { -1, -2 }, { 1, -2 }, { -1, -1 }, { 1, -1 } };

	int sx, sy, dx, dy, x, y;

	os = &objects[oi];
	if (os->_oVar4 != TRAP_ACTIVE) // TRAP_LIVE
		return;

	trigNum = 0;
	on = &objects[os->_oVar1]; // TRAP_OI_REF
	switch (on->_otype) {
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
#endif
		if (on->_oVar4 != DOOR_CLOSED) {
			trigArea = baseTrigArea;
			trigNum = lengthof(baseTrigArea);
		}
		break;
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
	case OBJ_SWITCHSKL:
		if (!(on->_oModeFlags & OMF_ACTIVE)) {
			trigArea = baseTrigArea;
			trigNum = lengthof(baseTrigArea);
		}
		break;
	case OBJ_SARC:
#ifdef HELLFIRE
	case OBJ_L5SARC:
#endif
		if (!(on->_oModeFlags & OMF_ACTIVE)) {
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

	os->_oVar4 = TRAP_INACTIVE; // TRAP_LIVE
	on->_oTrapChance = 0;

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

	// SetRndSeed(os->_oRndSeed);
	sx = os->_ox;
	sy = os->_oy;
	dir = GetDirection(sx, sy, dx, dy);
	AddMissile(sx, sy, dx, dy, dir, os->_oVar3, MST_OBJECT, -1, 0); // TRAP_MISTYPE

	NetSendCmdParam1(CMD_TRAPDISABLE, oi);
}

static void Obj_BCrossDamage(int oi)
{
	int ox, oy, pnum, fire_resist, damage;

	ox = objects[oi]._ox;
	oy = objects[oi]._oy - 1;
	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!plr._pActive || currLvl._dLevelIdx != plr._pDunLevel || plr._pInvincible)
			continue;
		if (plr._px != ox || plr._py != oy)
			continue;

		damage = 4 + (currLvl._dLevel >> 2);
		fire_resist = plr._pFireResist;
		if (fire_resist > 0)
			damage -= fire_resist * damage / 100;

		if (!PlrDecHp(pnum, damage, DMGTYPE_NPC))
			PlaySfxLoc(sgSFXSets[SFXS_PLR_68][plr._pClass], ox, oy + 1);
	}
}

static void (*const OiProc[])(int i) = {
	// clang-format off
/*OPF_NONE*/        NULL,
/*OPF_DOOR*/        &Obj_Door,
/*OPF_FLAMETRAP*/// &Obj_FlameTrap,
/*OPF_TRAP*/        &Obj_Trap,
/*OPF_CIRCLE*/      &Obj_Circle,
/*OPF_BCROSS*/      &Obj_BCrossDamage,
#if FLICKER_LIGHT
/*OPF_LIGHT*/       &Obj_Light,
#endif
	// clang-format on
};
void ProcessObjects()
{
	int i, oi;

	for (i = 0; i < numobjects; ++i) {
		oi = i; // objectactive[i];
		if (objects[oi]._oProc != OPF_NONE)
			OiProc[objects[oi]._oProc](oi);

		if (objects[oi]._oAnimFlag == OAM_NONE)
			continue;

		objects[oi]._oAnimCnt++;

		if (objects[oi]._oAnimCnt < objects[oi]._oAnimFrameLen)
			continue;

		objects[oi]._oAnimCnt = 0;
		objects[oi]._oAnimFrame++;
		if (objects[oi]._oAnimFrame > objects[oi]._oAnimLen) {
			if (objects[oi]._oAnimFlag == OAM_LOOP) {
				objects[oi]._oAnimFrame = 1;
			} else {
				objects[oi]._oAnimFrame--; 
				objects[oi]._oAnimFlag = OAM_NONE;
			}
		}
	}
	/*for (i = 0; i < numobjects; ) {
		oi = objectactive[i];
		if (objects[oi]._oDelFlag) {
			DeleteObject_(oi, i);
		} else {
			i++;
		}
	}*/
}

static void ObjSetMini(int x, int y, int mt)
{
	int xx, yy;
	long v1, v2, v3, v4;
	uint16_t* Tiles;

	xx = 2 * x + DBORDERX;
	yy = 2 * y + DBORDERY;

	Tiles = &pMegaTiles[(mt - 1) * 4];
	v1 = SwapLE16(Tiles[0]) + 1;
	v2 = SwapLE16(Tiles[1]) + 1;
	v3 = SwapLE16(Tiles[2]) + 1;
	v4 = SwapLE16(Tiles[3]) + 1;

	dPiece[xx][yy] = v1;
	dPiece[xx + 1][yy] = v2;
	dPiece[xx][yy + 1] = v3;
	dPiece[xx + 1][yy + 1] = v4;
}

#ifdef HELLFIRE
static void ObjSetDoorSidePiece(int dx, int dy/*, int otype*/)
{
	int pn;

	pn = dPiece[dx][dy];
//#ifdef HELLFIRE
	//if (currLvl._dType == DTYPE_CRYPT) {
		if (pn == 75)
			pn = 204;
		else if (pn == 79)
			pn = 208;
		/* commented out because this is not possible with the current implementation
		else if (pn == 86)
			pn = otype == OBJ_L5LDOOR ? 232 : 234;
		else if (pn == 91)
			pn = 215;
		else if (pn == 93)
			pn = 218;
		else if (pn == 99)
			pn = 220;
		else if (pn == 111)
			pn = 222;
		else if (pn == 113)
			pn = 224;
		else if (pn == 115)
			pn = 226;
		else if (pn == 117)
			pn = 228;
		else if (pn == 119)
			pn = 230;
		else if (pn == 232 || pn == 234)
			pn = 212;*/
		else
			return;
	//} else
//#endif
	/* commented out because this does not make a visible difference.
	   the 'new' content is overwritten when the door is drawn.
		if (pn == 43)
			pn = 392;
		else if (pn == 45)
			pn = 394;
		/ * commented out because this is not possible with the current implementation
		else if (pn == 50)
			pn = otype == OBJ_L1LDOOR ? 411 : 412;
		else if (pn == 54)
			pn = 397;
		else if (pn == 55)
			pn = 398;
		else if (pn == 61)
			pn = 399;
		else if (pn == 67)
			pn = 400;
		else if (pn == 68)
			pn = 401;
		else if (pn == 69)
			pn = 403;
		else if (pn == 70)
			pn = 404;
		else if (pn == 72)
			pn = 406;
		else if (pn == 354)
			pn = 409;
		else if (pn == 355)
			pn = 410;
		else if (pn == 411 || pn == 412)
			pn = 396;* /
		else if (pn == 212)
			pn = 407;
		else
			return;*/
	dPiece[dx][dy] = pn;
}
#endif

static void OpenDoor(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oVar4 = DOOR_OPEN;
	os->_oPreFlag = TRUE;
	os->_oSelFlag = 2;
	// TODO: set os->_oSolidFlag = FALSE;
	os->_oMissFlag = TRUE;
	os->_oAnimFrame += 2;
}

static bool CloseDoor(int oi)
{
	ObjectStruct* os;
	int xp, yp;

	os = &objects[oi];
	xp = os->_ox;
	yp = os->_oy;
	os->_oVar4 = (dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0 ? DOOR_CLOSED : DOOR_BLOCKED;
	if (os->_oVar4 == DOOR_CLOSED) {
		os->_oPreFlag = FALSE;
		os->_oSelFlag = 3;
		// TODO: set os->_oSolidFlag = TRUE;
		os->_oMissFlag = FALSE;
		os->_oAnimFrame -= 2;
		return true;
	}
	return false;
}

/*
 * check if the given player is in the right position to open a door.
 * @param oi: the index of the door
 * @param pnum: the index of the player
 */
static bool PlrCheckDoor(int oi, int pnum)
{
	ObjectStruct* os;
	int dx, dy;

	os = &objects[oi];
	dx = abs(os->_ox - plr._px);
	dy = abs(os->_oy - plr._py);
	if (os->_oDoorFlag == ODT_LEFT)
		return dx == 1 && dy <= 1;
	else // if (os->_oDoorFlag == ODT_RIGHT)
		return dx <= 1 && dy == 1;
}

static void OperateL1Door(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOOROPEN, oi);
		if (!deltaload) {
			PlaySfxLoc(IS_DOOROPEN, os->_ox, os->_oy);
		}
		OpenDoor(oi);
		SyncL1Doors(oi);
		RedoLightAndVision();
		return;
	}
	// try to close the door
	if (!deltaload) {
		PlaySfxLoc(IS_DOORCLOS, os->_ox, os->_oy);
	}
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if (CloseDoor(oi)) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOORCLOSE, oi);
		SyncL1Doors(oi);
		RedoLightAndVision();
	}
}

#ifdef HELLFIRE
static void OperateL5Door(int oi, bool sendmsg)
{
	ObjectStruct* os;
	int sfx;

	os = &objects[oi];
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOOROPEN, oi);
		if (!deltaload) {
			PlaySfxLoc(IS_CROPEN, os->_ox, os->_oy);
		}
		OpenDoor(oi);
		SyncL5Doors(oi);
		RedoLightAndVision();
		return;
	}
	// try to close the door
	if (!deltaload) {
		sfx = os->_oVar4 == DOOR_BLOCKED ? IS_DOORCLOS : IS_CRCLOS;
		PlaySfxLoc(sfx, os->_ox, os->_oy);
	}
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if (CloseDoor(oi)) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOORCLOSE, oi);
		SyncL5Doors(oi);
		RedoLightAndVision();
	}
}
#endif

static void OperateL2Door(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOOROPEN, oi);
		if (!deltaload) {
			PlaySfxLoc(IS_DOOROPEN, os->_ox, os->_oy);
		}
		OpenDoor(oi);
		SyncL2Doors(oi);
		RedoLightAndVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, os->_ox, os->_oy);
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if (CloseDoor(oi)) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOORCLOSE, oi);
		SyncL2Doors(oi);
		RedoLightAndVision();
	}
}

static void OperateL3Door(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOOROPEN, oi);
		if (!deltaload) {
			PlaySfxLoc(IS_DOOROPEN, os->_ox, os->_oy);
		}
		OpenDoor(oi);
		SyncL3Doors(oi);
		RedoLightAndVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, os->_ox, os->_oy);
	if (os->_oVar4 == DOOR_BLOCKED)
		return;

	if (CloseDoor(oi)) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOORCLOSE, oi);
		SyncL3Doors(oi);
		RedoLightAndVision();
	}
}

void MonstCheckDoors(int mx, int my)
{
	int i, oi, type;

	for (i = 0; i < lengthof(offset_x); i++) {
		oi = dObject[mx + offset_x[i]][my + offset_y[i]];
		if (oi == 0)
			continue;
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (objects[oi]._oDoorFlag == ODT_NONE || objects[oi]._oVar4 != DOOR_CLOSED)
			continue;
		// assert(CheckDoor(oi, mnum));
		type = objects[oi]._otype;
		if (type == OBJ_L1LDOOR || type == OBJ_L1RDOOR) {
			OperateL1Door(oi, true);
#ifdef HELLFIRE
		} else if (type == OBJ_L5LDOOR || type == OBJ_L5RDOOR) {
			OperateL5Door(oi, true);
#endif
		} else if (type == OBJ_L2LDOOR || type == OBJ_L2RDOOR) {
			OperateL2Door(oi, true);
		} else {
			//assert(type == OBJ_L3LDOOR || type == OBJ_L3RDOOR);
			OperateL3Door(oi, true);
		}
	}
}

void ObjChangeMap(int x1, int y1, int x2, int y2/*, bool hasNewObjPiece*/)
{
	int i, j;

	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			dungeon[i][j] = pdungeon[i][j];
			ObjSetMini(i, j, pdungeon[i][j]);
		}
	}
	x1 = 2 * x1 + DBORDERX;
	y1 = 2 * y1 + DBORDERY;
	x2 = 2 * x2 + DBORDERX + 1;
	y2 = 2 * y2 + DBORDERY + 1;
	// init special pieces
	if (currLvl._dType == DTYPE_CATHEDRAL) {
		DRLG_InitL1Specials(x1, y1, x2, y2);
	} else if (currLvl._dType == DTYPE_CATACOMBS) {
		DRLG_InitL2Specials(x1, y1, x2, y2);
#ifdef HELLFIRE
	} else if (currLvl._dType == DTYPE_CRYPT) {
		DRLG_InitL5Specials(x1, y1, x2, y2);
#endif
	}
	// activate objects
	for (i = 0; i < numobjects; i++) {
		int oi = i; // objectactive[i];
		ObjectStruct* os = &objects[oi];
		if (!(os->_oModeFlags & OMF_RESERVED))
			continue;
		if (nSolidTable[dPiece[os->_ox][os->_oy]])
			continue;
		// assert(os->_ox >= x1 && os->_ox <= x2 && os->_oy >= y1 && os->_oy <= y2);
		dObject[os->_ox][os->_oy] = oi + 1;
		os->_oModeFlags &= ~OMF_RESERVED;
		os->_oSelFlag = objectdata[os->_otype].oSelFlag;
	}
	// add new objects (doors + light)
	AddDunObjs(x1, y1, x2, y2);
	// activate monsters
	MonChangeMap();
	RedoLightAndVision();
}

static bool CheckLeverGroup(int type, int lvrIdx)
{
	ObjectStruct* os;
	int i;

	for (i = 0; i < numobjects; i++) {
		os = &objects[i]; // objects[objectactive[i]]
		if (os->_otype != type) // OBJ_SWITCHSKL, OBJ_LEVER, OBJ_BOOK2L or OBJ_L5LEVER
			continue;
		if (lvrIdx != os->_oVar8 || !(os->_oModeFlags & OMF_ACTIVE)) // LEVER_INDEX
			continue;
		return false;
	}
	return true;
}

static bool CheckCrux(int lvrIdx)
{
	ObjectStruct* os;
	int i;

	for (i = 0; i < numobjects; i++) {
		os = &objects[i]; // objects[objectactive[i]]
		if (os->_otype != OBJ_CRUXM && os->_otype != OBJ_CRUXR && os->_otype != OBJ_CRUXL)
			continue;
		if (os->_oVar8 != lvrIdx || os->_oBreak == OBM_BROKEN) // LEVER_INDEX
			continue;
		return false;
	}

	return true;
}

static void OperateLever(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame++; // 2

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
	if (!CheckLeverGroup(os->_otype, os->_oVar8)) // LEVER_INDEX
		return;
#ifdef HELLFIRE
	if (currLvl._dLevelIdx == DLV_CRYPT4 && !deltaload) {
		if (quests[Q_NAKRUL]._qactive != QUEST_DONE) {
			// assert(quests[Q_NAKRUL]._qvar1 < QV_NAKRUL_BOOKOPEN);
			quests[Q_NAKRUL]._qactive = QUEST_DONE;
			// quests[Q_NAKRUL]._qvar1 = QV_NAKRUL_LEVEROPEN;
			if (sendmsg)
				NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
		}
		PlaySfxLoc(IS_CROPEN, os->_ox - 3, os->_oy + 1);
	}
#endif
	ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
}

static void OperateVileBook(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	int dx, dy, on;

	assert(currLvl._dSetLvl);

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		// assert(plr._pmode == PM_STAND && !deltaload);
		if (plr._px != os->_ox || plr._py != os->_oy + 1)
			return;
		if (os->_ox == DBORDERX + 10) {
			dx = DBORDERX + 11;
			dy = DBORDERY + 13;
		} else {
			assert(os->_ox == DBORDERX + 29);
			dx = DBORDERX + 27;
			dy = DBORDERY + 13;
		}
		on = dObject[dx][dy] - 1;
		assert(objects[on]._otype == OBJ_MCIRCLE1 || objects[on]._otype == OBJ_MCIRCLE2);

		FindClosestPlr(&dx, &dy);
		AddMissile(0, 0, dx, dy, 0, MIS_RNDTELEPORT, MST_OBJECT, pnum, 0);
		objects[dObject[DBORDERX + 19][DBORDERY + 20] - 1]._oVar5++; // VILE_CIRCLE_PROGRESS
	}
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame++; // 2

	if (currLvl._dLevelIdx == SL_BONECHAMB) {
		if (deltaload)
			return;
		if (plr._pSkillLvlBase[SPL_GUARDIAN] == 0) {
			plr._pSkillExp[SPL_GUARDIAN] = SkillExpLvlsTbl[0];
			IncreasePlrSkillLvl(pnum, SPL_GUARDIAN);
		}
		PlaySfxLoc(IS_QUESTDN, os->_ox, os->_oy);
		if (pnum == mypnum)
			InitDiabloMsg(EMSG_BONECHAMB);
		// SetRndSeed(os->_oRndSeed);
		AddMissile(plr._px, plr._py, os->_ox - 2, os->_oy - 4, 0, MIS_GUARDIAN, MST_PLAYER, pnum, 0);
		quests[Q_BCHAMB]._qactive = QUEST_DONE;
		if (sendmsg) {
			NetSendCmdQuest(Q_BCHAMB, true); // recipient should not matter
			NetSendCmdParam1(CMD_OPERATEOBJ, oi);
		}
	} //else if (currLvl._dLevelIdx == SL_VILEBETRAYER) { NULL_LVR_EFFECT
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
		//for (i = 0; i < numobjects; i++)
		//	SyncObjectAnim(objectactive[i]);
	//}
}

static void OperateBookLever(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	int qn;

	if (numitems >= MAXITEMS && !deltaload) {
		return;
	}
	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	// assert(os->_oSelFlag != 0);
	qn = os->_oVar7; // LEVER_BOOK_QUEST

	if (os->_oAnimFrame != os->_oVar6) { // LEVER_BOOK_ANIM
		os->_oAnimFrame = os->_oVar6;    // LEVER_BOOK_ANIM
		//if (qn != Q_BLOOD) NULL_LVR_EFFECT
			ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4 /*, qn == Q_BLIND*/); // LEVER_EFFECT
		if (qn == Q_BLIND) {
			if (!deltaload)
				SpawnUnique(UITEM_OPTAMULET, 2 * os->_oVar1 + DBORDERX + 5, 2 * os->_oVar2 + DBORDERY + 5, sendmsg ? ICM_SEND : ICM_DUMMY);
			//int tv = dTransVal[2 * os->_oVar1 + DBORDERX + 1][2 * os->_oVar2 + DBORDERY + 1];
			//DRLG_MRectTrans(os->_oVar1 + 2, os->_oVar2 + 2, os->_oVar1 + 4, os->_oVar2 + 4, tv); // LEVER_EFFECT
			//DRLG_MRectTrans(os->_oVar1 + 6, os->_oVar2 + 6, os->_oVar1 + 8, os->_oVar2 + 8, tv); // LEVER_EFFECT
		}
	}
	if (deltaload)
		return;
	if (pnum == mypnum)
		StartQTextMsg(questlist[qn]._qdmsg);
	if (quests[qn]._qvar1 == QV_INIT) {
		// assert(qn == [Q_BLOOD, Q_BCHAMB, Q_BLIND, Q_WARLORD]);
		static_assert(QV_BLOOD_BOOK == QV_BCHAMB_BOOK && QV_BLOOD_BOOK == QV_BLIND_BOOK && QV_BLOOD_BOOK == QV_WARLORD_BOOK, "OperateBookLever sets qvar1 of multiple quests in one statement.");
		quests[qn]._qvar1 = QV_BLOOD_BOOK;
		quests[qn]._qactive = QUEST_ACTIVE;
		quests[qn]._qlog = TRUE;
		if (qn == Q_BLOOD) {
			SetRndSeed(os->_oRndSeed);
			CreateQuestItemAt(IDI_BLDSTONE, 2 * pSetPieces[0]._spx + DBORDERX + 9, 2 * pSetPieces[0]._spy + DBORDERY + 17, sendmsg ? ICM_SEND : ICM_DUMMY);
		}
		if (sendmsg) {
			NetSendCmdQuest(qn, true);
			NetSendCmdParam1(CMD_OPERATEOBJ, oi);
		}
	}
}

static void OperateChest(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	int i, k, mtype, mdir;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame += 2;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	for (i = os->_oVar1; i > 0; i--) { // CHEST_ITEM_NUM
		SetRndSeed(os->_oRndSeed);     // CHEST_ITEM_SEEDx
		for (k = i; k > 1; k--)
			NextRndSeed();
		if (os->_oVar2 != 0)           // CHEST_ITEM_TYPE
			CreateRndItem(os->_ox, os->_oy, os->_oVar2 == 8 ? CFDQ_GOOD : CFDQ_NORMAL, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
		else
			SpawnRndUseful(os->_ox, os->_oy, sendmsg);
	}
	if (os->_otype >= OBJ_TCHEST1 && os->_otype <= OBJ_TCHEST3 && os->_oTrapChance != 0) {
		os->_oTrapChance = 0;
		SetRndSeed(os->_oRndSeed);
		if (currLvl._dType == DTYPE_CATACOMBS) {
			mtype = 2;
		} else {
#ifdef HELLFIRE
			mtype = 4;
#else
			mtype = 3;
#endif
		}
		mtype = random_(0, mtype);
		switch (mtype) {
		case 0:
			mtype = MIS_ARROW;
			break;
		case 1:
			mtype = MIS_FIREBOLT;
			break;
		case 2:
			mtype = MIS_LIGHTNOVAC;
			break;
#ifdef HELLFIRE
		case 3:
			mtype = MIS_FIRERING;
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			break;
		}
		mdir = GetDirection(os->_ox, os->_oy, plr._px, plr._py);
		AddMissile(os->_ox, os->_oy, plr._px, plr._py, mdir, mtype, MST_OBJECT, -1, 0);
	}
}

static void OperateMushPatch(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;

	if (numitems >= MAXITEMS) {
		return;
	}

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame++; // 2
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAt(IDI_MUSHROOM, os->_ox, os->_oy, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

static void OperateInnSignChest(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;

	if (numitems >= MAXITEMS) {
		return;
	}

	if (quests[Q_BANNER]._qvar1 != QV_BANNER_TALK1 && !deltaload) {
		if (sendmsg) // if (pnum == mypnum)
			PlaySFX(sgSFXSets[SFXS_PLR_24][plr._pClass]);
		return;
	}
	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame += 2;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAt(IDI_BANNER, os->_ox, os->_oy, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

static void OperateSlainHero(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	BYTE pc;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

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
	SpawnMagicItem(typeCurs[pc][0], typeCurs[pc][1], os->_ox, os->_oy, sendmsg);
	PlaySfxLoc(sgSFXSets[SFXS_PLR_09][pc], plr._px, plr._py);
}

/*static void OperateFlameTrapLever(int oi, bool sendmsg)
{
	ObjectStruct *os, *on;
	int i;
	bool disable;

	os = &objects[oi];
	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);

	disable = os->_oAnimFrame == FLAMETRAP_ACTIVE_FRAME;
	os->_oAnimFrame = disable ? FLAMETRAP_INACTIVE_FRAME : FLAMETRAP_ACTIVE_FRAME;

	if (sendmsg)
		NetSendCmdParam1(disable ? CMD_TRAPCLOSE : CMD_TRAPOPEN, oi);
	for (i = 0; i < numobjects; i++) {
		on = &objects[objectactive[i]]; //         FLAMETRAP_ID
		if (on->_otype == OBJ_FLAMEHOLE && on->_oVar1 == os->_oVar1) {
			on->_oVar2 = disable ? TRAP_INACTIVE : TRAP_ACTIVE;
		}
	}
}*/

static void OperateSarc(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		return;
	}

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_SARC, os->_ox, os->_oy);

	os->_oAnimFlag = OAM_SINGLE;
	//os->_oAnimFrameLen = 3;
	SetRndSeed(os->_oRndSeed);
	if (os->_oVar1 <= 2) // SARC_ITEM
		CreateRndItem(os->_ox, os->_oy, CFDQ_NORMAL, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
	if (os->_oVar1 >= 8) // SARC_SKELE
		SpawnSkeleton(os->_oVar2, os->_ox, os->_oy, DIR_NONE);
}

static void SyncPedestal(/*int oi*/)
{
	switch (quests[Q_BLOOD]._qvar1) {
	case QV_INIT:
	case QV_BLOOD_BOOK:
		break;
	case QV_BLOOD_STONE2:
		ObjChangeMap(pSetPieces[0]._spx + 6, pSetPieces[0]._spy + 3, pSetPieces[0]._spx + 9/*setpc_w*/, pSetPieces[0]._spy + 7/*, false*/);
		/* fall-through */
	case QV_BLOOD_STONE1:
		ObjChangeMap(pSetPieces[0]._spx, pSetPieces[0]._spy + 3, pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 7/*, false*/);
		break;
	case QV_BLOOD_STONE3: {
		//ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h/*, false*/);
		ObjChangeMap(pSetPieces[0]._spx /*+ 2*/, pSetPieces[0]._spy, pSetPieces[0]._spx + 9/*6*/, pSetPieces[0]._spy + 8/*, false*/);
		LoadPreLighting();
		BYTE* setp = LoadFileInMem("Levels\\L2Data\\Blood2.DUN");
		LoadMapSetObjs(setp);
		mem_free_dbg(setp);
		SavePreLighting();
		//RedoLightAndVision();
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * Handle the using the pedistal of Q_BLOOD-quest.
 */
static void OperatePedestal(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	int iv;

	os = &objects[oi];
	if (!(os->_oModeFlags & OMF_ACTIVE))
		return;
	if (!deltaload && pnum != -1) { // TODO: possible desync of player-items?
		if (numitems >= MAXITEMS)
			return;
		if (!PlrHasStorageItem(pnum, IDI_BLDSTONE, &iv))
			return;
		if (sendmsg)
			// assert(pnum == mypnum);
			NetSendCmdParam1(CMD_BLOODPASS, oi);
		return;
	}

	os->_oAnimFrame = quests[Q_BLOOD]._qvar1;
	if (quests[Q_BLOOD]._qvar1 == QV_BLOOD_STONE3) {
		os->_oModeFlags &= ~OMF_ACTIVE;
		os->_oSelFlag = 0;
	}
	SyncPedestal();

	if (deltaload)
		return;
	iv = sendmsg ? ICM_SEND : ICM_DUMMY;
	SetRndSeed(os->_oRndSeed - (quests[Q_BLOOD]._qvar1 - QV_BLOOD_STONE1)); // used for IDI_BLDSTONE
	switch (quests[Q_BLOOD]._qvar1) {
	case QV_BLOOD_BOOK:
		break; // should not really happen
	case QV_BLOOD_STONE1:
		CreateQuestItemAt(IDI_BLDSTONE, 2 * pSetPieces[0]._spx + DBORDERX + 3, 2 * pSetPieces[0]._spy + DBORDERY + 10, iv);
		break;
	case QV_BLOOD_STONE2:
		CreateQuestItemAt(IDI_BLDSTONE, 2 * pSetPieces[0]._spx + DBORDERX + 15, 2 * pSetPieces[0]._spy + DBORDERY + 10, iv);
		break;
	case QV_BLOOD_STONE3:
		SpawnUnique(UITEM_ARMOFVAL, 2 * pSetPieces[0]._spx + DBORDERX + 9, 2 * pSetPieces[0]._spy + DBORDERY + 3, iv);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	PlaySfxLoc(quests[Q_BLOOD]._qvar1 == QV_BLOOD_STONE3 ? LS_BLODSTAR : LS_PUDDLE, os->_ox, os->_oy);
}

bool SyncBloodPass(int pnum, int oi)
{
	int iv;

	if (quests[Q_BLOOD]._qvar1 < QV_BLOOD_BOOK || quests[Q_BLOOD]._qvar1 >= QV_BLOOD_STONE3)
		return false; // prevent interaction with the pedistal before reading the book or after the 3. stone is placed
	if (!PlrHasStorageItem(pnum, IDI_BLDSTONE, &iv))
		return false;
	SyncPlrStorageRemove(pnum, iv);
	quests[Q_BLOOD]._qvar1++; // QV_BLOOD_STONE1, QV_BLOOD_STONE2, QV_BLOOD_STONE3
	if (plr._pDunLevel == currLvl._dLevelIdx)
		OperatePedestal(-1, oi, pnum == mypnum);
	return true;
}

void DisarmObject(int pnum, int oi)
{
	ObjectStruct *os, *on;
	int n;

	os = &objects[oi];
	if (os->_oTrapChance == 0)
		return;
	if ((4 * currLvl._dLevel + os->_oTrapChance) > plr._pDexterity)
		return;

	os->_oTrapChance = 0;
	n = os->_oVar5; // TRAP_OI_BACKREF
	if (n > 0) {
		n--;
		on = &objects[n];
		on->_oVar4 = TRAP_INACTIVE; // TRAP_LIVE
		if (pnum == mypnum)
			NetSendCmdParam1(CMD_TRAPDISABLE, n);
	}
}

static void CloseChest(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	if (!(os->_oModeFlags & OMF_ACTIVE))
		return;
	os->_oModeFlags |= OMF_ACTIVE;
	if (!(os->_oModeFlags & OMF_RESERVED)) {
		os->_oSelFlag = 1;
	}
	os->_oAnimFrame -= 2;

	//SetRndSeed(os->_oRndSeed); -- do NOT set RndSeed, might conflict with the other chests
	os->_oRndSeed = NextRndSeed();

	//if (sendmsg)
	//	NetSendCmdParam1(CMD_CHESTCLOSE, oi);
}

/** Raise the skill of the given player for the price of 10% of their maximum mana
*/
static void AddRaiseSkill(int pnum, int sn)
{
	int v1, v2;
	unsigned t;
	// add/raise the skill
	if (plr._pSkillLvlBase[sn] < MAXSPLLEVEL) {
		plr._pSkillExp[sn] = SkillExpLvlsTbl[plr._pSkillLvlBase[sn]];
		IncreasePlrSkillLvl(pnum, sn);
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

static void ConvertPotion(ItemStruct* pi)
{
	if (pi->_itype != ITYPE_MISC)
		return;
	if (pi->_iMiscId == IMISC_FULLHEAL || pi->_iMiscId == IMISC_FULLMANA) {
		CreateBaseItem(pi, IDI_FULLREJUV);
	} else if (pi->_iMiscId == IMISC_HEAL || pi->_iMiscId == IMISC_MANA) {
		CreateBaseItem(pi, IDI_REJUV);
	}
}

void SyncShrineCmd(int pnum, BYTE type, int seed)
{
	ItemStruct* pi;
	int i, cnt, r;

	switch (type) {
	case SHRINE_HIDDEN:
		SetRndSeed(seed);
		cnt = 0;
		pi = plr._pInvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype != ITYPE_NONE
			 && pi->_iMaxDur != DUR_INDESTRUCTIBLE
			 && pi->_iMaxDur != 0)
				cnt++;
		}
		if (cnt != 0) {
			r = random_low(0, cnt);
			pi = plr._pInvBody;
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
		break;
	case SHRINE_GLOOMY:
		pi = &plr._pInvBody[INVLOC_HAND_LEFT];
		pi->_iDurability = pi->_iMaxDur;

		pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
		if (pi->_iClass == ICLASS_WEAPON)
			pi->_iDurability = pi->_iMaxDur;
		break;
	case SHRINE_WEIRD:
		pi = &plr._pInvBody[INVLOC_HEAD];
		pi->_iDurability = pi->_iMaxDur;

		pi = &plr._pInvBody[INVLOC_CHEST];
		pi->_iDurability = std::max(1, pi->_iDurability >> 1);
		break;
	case SHRINE_RELIGIOUS:
		pi = plr._pInvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur;
		pi = plr._pInvList;
		for (i = NUM_INV_GRID_ELEM; i > 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur;
		//pi = plr._pSpdList;
		//for (i = MAXBELTITEMS; i != 0; i--, pi++)
		//	pi->_iDurability = pi->_iMaxDur; // belt items don't have durability?
		break;
	case SHRINE_STONE:
		pi = plr._pInvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges;
		pi = plr._pInvList;
		for (i = NUM_INV_GRID_ELEM; i > 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges;
		//pi = plr._pSpdList;
		//for (i = MAXBELTITEMS; i != 0; i--, pi++)
		//	pi->_iCharges = pi->_iMaxCharges; // belt items don't have charges?
		break;
	case SHRINE_CREEPY:
		pi = &plr._pInvBody[INVLOC_HAND_LEFT];
		pi->_iCharges = pi->_iMaxCharges;
		break;
	case SHRINE_FASCINATING:
		AddRaiseSkill(pnum, SPL_FIREBOLT);
		break;
	case SHRINE_SHIMMERING:
	case SHRINE_CRYPTIC:
		PlrFillMana(pnum);
		break;
	case SHRINE_ELDRITCH:
		SetRndSeed(seed);
		pi = plr._pInvList;
		for (i = NUM_INV_GRID_ELEM; i > 0; i--, pi++)
			ConvertPotion(pi);
		pi = plr._pSpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			ConvertPotion(pi);
		break;
	case SHRINE_EERIE:
		for (i = 0; i < MAX_PLRS; i++)
			if (i != pnum && plr._pDunLevel == plx(i)._pDunLevel)
				PlrFillMana(i);
		break;
	case SHRINE_SPOOKY:
		for (i = 0; i < MAX_PLRS; i++)
			if (i != pnum && plr._pDunLevel == plx(i)._pDunLevel) {
				PlrFillHp(i);
				PlrFillMana(i);
			}
		break;
	case SHRINE_QUIET:
		for (i = 0; i < MAX_PLRS; i++)
			if (i != pnum && plr._pDunLevel == plx(i)._pDunLevel)
				PlrFillHp(i);
		break;
	case SHRINE_DIVINE:
		PlrFillHp(pnum);
		PlrFillMana(pnum);
		break;
	case SHRINE_SACRED:
		AddRaiseSkill(pnum, SPL_CBOLT);
		break;
	case SHRINE_ORNATE:
		AddRaiseSkill(pnum, SPL_HBOLT);
		break;
	case SHRINE_SPIRITUAL:
		SetRndSeed(seed);
		cnt = plr._pDunLevel;
		// assert(cnt != 0);
		pi = plr._pInvList;
		for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
			if (pi->_itype == ITYPE_NONE) {
				CreateBaseItem(pi, IDI_GOLD);
				r = cnt + random_low(160, 2 * cnt);
				plr._pGold += r;
				SetGoldItemValue(pi, r);
			}
		}
		break;
	case SHRINE_GLIMMERING:
		pi = plr._pInvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE;
		pi = plr._pInvList;
		for (i = NUM_INV_GRID_ELEM; i > 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE;
		//pi = plr._pSpdList;
		//for (i = MAXBELTITEMS; i != 0; i--, pi++)
		//	if (pi->_iMagical != ITEM_QUALITY_NORMAL)
		//		pi->_iIdentified = TRUE; // belt items can't be magical?
		break;
	case SHRINE_SPARKLING:
		AddPlrExperience(pnum, plr._pLevel, 500 * plr._pDunLevel);
		break;
	case SHRINE_MURPHYS:
		SetRndSeed(seed);
		pi = plr._pInvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype == ITYPE_NONE)
				continue;
			r = pi->_iDurability;
			if (r == DUR_INDESTRUCTIBLE || r == 0)
				continue;
			if (random_(0, 3) != 0) {
				r >>= 1;
				if (r == 0)
					pi->_itype = ITYPE_NONE;
				else
					pi->_iDurability = r;
				break;
			}
		}
		if (i == 0) {
			TakePlrsMoney(pnum, plr._pGold / 3);
		}
		break;
	}

	CalcPlrInv(pnum, true);
	gbRedrawFlags = REDRAW_ALL;
}

static void OperateShrine(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	int i, mode;
#ifdef HELLFIRE
	int xx, yy;
#endif
	assert((unsigned)oi < MAXOBJECTS);

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;

	if (deltaload) {
		// os->_oAnimFlag = OAM_NONE;
		os->_oAnimFrame = os->_oAnimLen;
		return;
	}

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	SetRndSeed(os->_oRndSeed);

	PlaySfxLoc(os->_oSFX, os->_ox, os->_oy, os->_oSFXCnt);
	os->_oAnimFlag = OAM_SINGLE;
	//os->_oAnimFrameLen = 1;

	switch (os->_oVar1) { // SHRINE_TYPE
	case SHRINE_HIDDEN:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_HIDDEN, os->_oRndSeed);
		InitDiabloMsg(EMSG_SHRINE_HIDDEN);
		break;
	case SHRINE_GLOOMY:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_GLOOMY, 0);
		InitDiabloMsg(EMSG_SHRINE_GLOOMY);
		break;
	case SHRINE_WEIRD:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_WEIRD, 0);
		InitDiabloMsg(EMSG_SHRINE_WEIRD);
		break;
	case SHRINE_RELIGIOUS:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_RELIGIOUS, 0);
		InitDiabloMsg(EMSG_SHRINE_RELIGIOUS);
		break;
	case SHRINE_MAGICAL:
		AddMissile(0, 0, 0, 0, 0, MIS_MANASHIELD, MST_NA, pnum, (1 + currLvl._dLevel) >> 1);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_MAGICAL);
		break;
	case SHRINE_STONE:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_STONE, 0);
		InitDiabloMsg(EMSG_SHRINE_STONE);
		break;
	case SHRINE_CREEPY:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_CREEPY, 0);
		InitDiabloMsg(EMSG_SHRINE_CREEPY);
		break;
	case SHRINE_THAUMATURGIC:
		for (i = 0; i < numobjects; i++) {
			os = &objects[i]; // objects[objectactive[i]];
			if ((os->_otype >= OBJ_CHEST1 && os->_otype <= OBJ_CHEST3)
			 || (os->_otype >= OBJ_TCHEST1 && os->_otype <= OBJ_TCHEST3)) {
				CloseChest(i); // objectactive[i]
			}
		}
		assert(pnum == mypnum);
		InitDiabloMsg(EMSG_SHRINE_THAUMATURGIC);
		break;
	case SHRINE_FASCINATING:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_FASCINATING, 0);
		InitDiabloMsg(EMSG_SHRINE_FASCINATING);
		break;
	case SHRINE_SHIMMERING:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_SHIMMERING, 0);
		InitDiabloMsg(EMSG_SHRINE_SHIMMERING);
		break;
	case SHRINE_CRYPTIC:
		AddMissile(os->_ox, os->_oy, 0, 0, 0, MIS_LIGHTNOVAC, MST_OBJECT, -1, 0);
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_CRYPTIC, 0);
		InitDiabloMsg(EMSG_SHRINE_CRYPTIC);
		break;
	case SHRINE_ELDRITCH:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_ELDRITCH, 0);
		InitDiabloMsg(EMSG_SHRINE_ELDRITCH);
		break;
	case SHRINE_EERIE:
		if (pnum == mypnum) {
			NetSendShrineCmd(SHRINE_EERIE, 0);
			InitDiabloMsg(EMSG_SHRINE_EERIE1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_EERIE2);
		}
		break;
	case SHRINE_SPOOKY:
		if (pnum == mypnum) {
			NetSendShrineCmd(SHRINE_SPOOKY, 0);
			InitDiabloMsg(EMSG_SHRINE_SPOOKY1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_SPOOKY2);
		}
		break;
	case SHRINE_QUIET:
		if (pnum == mypnum) {
			NetSendShrineCmd(SHRINE_QUIET, 0);
			InitDiabloMsg(EMSG_SHRINE_QUIET1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_QUIET2);
		}
		break;
	case SHRINE_DIVINE:
		mode = sendmsg ? ICM_SEND_FLIP : ICM_DUMMY;
		if (currLvl._dLevelIdx <= DLV_CATHEDRAL3) {
			CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC, IMISC_FULLMANA, mode);
			CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC, IMISC_FULLHEAL, mode);
		} else {
			CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC, IMISC_FULLREJUV, mode);
			CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC, IMISC_FULLREJUV, mode);
		}
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_DIVINE, 0);
		InitDiabloMsg(EMSG_SHRINE_DIVINE);
		break;
	case SHRINE_HOLY:
		AddMissile(plr._px, plr._py, 0, 0, 0, MIS_RNDTELEPORT, MST_OBJECT, pnum, 0);
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_HOLY);
		break;
	case SHRINE_SACRED:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_SACRED, 0);
		InitDiabloMsg(EMSG_SHRINE_SACRED);
		break;
	case SHRINE_ORNATE:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_ORNATE, 0);
		InitDiabloMsg(EMSG_SHRINE_ORNATE);
		break;
	case SHRINE_SPIRITUAL:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_SPIRITUAL, os->_oRndSeed);
		InitDiabloMsg(EMSG_SHRINE_SPIRITUAL);
		break;
	case SHRINE_SECLUDED:
		if (pnum != mypnum)
			return;
		static_assert(sizeof(automapview) == DMAXY * DMAXX, "Linear traverse of automapview does not work in OperateShrine.");
		memset(automapview, TRUE, DMAXX * DMAXY);
		// TODO: set dFlags[][] |= BFLAG_EXPLORED ?
		InitDiabloMsg(EMSG_SHRINE_SECLUDED);
		break;
	case SHRINE_GLIMMERING:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_GLIMMERING, 0);
		InitDiabloMsg(EMSG_SHRINE_GLIMMERING);
		break;
	case SHRINE_TAINTED:
		static_assert(MAX_MINIONS == MAX_PLRS, "OperateShrine requires that owner of a monster has the same id as the monster itself.");
		if (monsters[mypnum]._mmode > MM_INGAME_LAST) {
			AddMissile(myplr._px, myplr._py, myplr._px, myplr._py, 0, MIS_GOLEM, MST_PLAYER, mypnum, currLvl._dLevel >> 1);
		}
		//if (pnum != mypnum)
		//	return;
		InitDiabloMsg(EMSG_SHRINE_TAINTED);
		break;
	case SHRINE_GLISTENING:
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_GLISTENING);
		AddMissile(0, 0, plr._px, plr._py, 0, MIS_TOWN, MST_NA, pnum, 0);
		break;
	case SHRINE_SPARKLING:
		AddMissile(os->_ox, os->_oy, 0, 0, 0, MIS_FLASH, MST_OBJECT, -1, 0);
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_SPARKLING, 0);
		InitDiabloMsg(EMSG_SHRINE_SPARKLING);
		break;
	case SHRINE_MURPHYS:
		if (pnum != mypnum)
			return;
		NetSendShrineCmd(SHRINE_MURPHYS, os->_oRndSeed);
		InitDiabloMsg(EMSG_SHRINE_MURPHYS);
		break;
#ifdef HELLFIRE
	case SHRINE_SOLAR: { // BUGFIX: missiles are not added/handled by the delta info...
		static_assert(MIS_RUNEFIRE + 1 == MIS_RUNELIGHT, "SHRINE_SOLAR expects runes in a given order I.");
		static_assert(MIS_RUNEFIRE + 2 == MIS_RUNENOVA, "SHRINE_SOLAR expects runes in a given order II.");
		static_assert(MIS_RUNEFIRE + 3 == MIS_RUNEWAVE, "SHRINE_SOLAR expects runes in a given order III.");
		static_assert(DBORDERX >= 3 && DBORDERY >= 3, "SHRINE_SOLAR expects a large enough border.");
		const int8_t* cr = &CrawlTable[CrawlNum[3]];
		mode = sendmsg ? ICM_SEND : ICM_DUMMY;
		for (i = (BYTE)*cr; i > 0; i--) {
			xx = plr._px + *++cr;
			yy = plr._py + *++cr;
			if (!ItemSpaceOk(xx, yy))
				continue;
			if (random_(0, 3) == 0)
				AddMissile(xx, yy, xx, yy, 0, MIS_RUNEFIRE + random_(0, 4), MST_OBJECT, -1, 0);
			else
				CreateTypeItem(xx, yy, CFDQ_NORMAL, ITYPE_MISC, IMISC_RUNE, mode);
		}
		if (pnum != mypnum)
			return;
		InitDiabloMsg(EMSG_SHRINE_SOLAR);
	} break;
#endif
	default:
		ASSUME_UNREACHABLE
	}
}

static void OperateSkelBook(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame += 2;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC,
		random_(161, 5) != 0 ? IMISC_SCROLL : IMISC_BOOK, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

static void OperateBookCase(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame -= 2;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC, IMISC_BOOK, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
	if (zharlib != -1 && themes[zharlib].ttval == dTransVal[os->_ox][os->_oy]
	 && quests[Q_ZHAR]._qvar1 < QV_ZHAR_ATTACK) {
		assert((monsters[MAX_MINIONS]._muniqtype - 1) == UMT_ZHAR);
		//assert(monsters[MAX_MINIONS]._mgoal == MGOAL_TALKING);
		monsters[MAX_MINIONS]._mgoalvar2 = TEXT_ZHAR2; // TALK_MESSAGE
		//MonStartStand(MAX_MINIONS);
		//monsters[MAX_MINIONS]._mgoal = MGOAL_ATTACK2;
		monsters[MAX_MINIONS]._mmode = MM_TALK;
		//monsters[MAX_MINIONS]._mVar8 = 0; // MON_TIMER
		quests[Q_ZHAR]._qvar1 = QV_ZHAR_ATTACK;
		if (sendmsg)
			NetSendCmdQuest(Q_ZHAR, true);
	}
}

static void OperateDecap(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	SetRndSeed(os->_oRndSeed);
	CreateRndItem(os->_ox, os->_oy, CFDQ_NORMAL, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

static void OperateArmorStand(int oi, bool sendmsg)
{
	ObjectStruct* os;
	int itype;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;
	// os->_oSolidFlag = TRUE;
	os->_oMissFlag = TRUE;

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	SetRndSeed(os->_oRndSeed);
	static_assert(ITYPE_LARMOR + 1 == ITYPE_MARMOR, "OperateArmorStand expects an ordered ITYPE_ for armors I.");
	static_assert(ITYPE_MARMOR + 1 == ITYPE_HARMOR, "OperateArmorStand expects an ordered ITYPE_ for armors II.");
	itype = ITYPE_LARMOR + random_(0, currLvl._dLevel >= 24 ? 3 : (currLvl._dLevel >= 10 ? 2 : 1));
	CreateTypeItem(os->_ox, os->_oy, CFDQ_GOOD, itype, IMISC_NONE, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

static void OperateGoatShrine(int pnum, int oi, bool sendmsg)
{
	OperateShrine(pnum, oi, sendmsg);
	// restore state
	objects[oi]._oAnimFlag = OAM_LOOP;
}

static void OperateCauldron(int pnum, int oi, bool sendmsg)
{
	OperateShrine(pnum, oi, sendmsg);
	// restore state
	objects[oi]._oAnimFrame = 3;
	objects[oi]._oAnimFlag = OAM_NONE;
}

static void OperateFountains(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// SetRndSeed(os->_oRndSeed);
	switch (os->_otype) {
	case OBJ_BLOODFTN:
		if (deltaload)
			return;
		PlrIncHp(pnum, 64);
		break;
	case OBJ_PURIFYINGFTN:
		if (deltaload)
			return;

		PlrIncMana(pnum, 64);
		break;
	case OBJ_MURKYFTN:
		// assert(os->_oModeFlags & OMF_ACTIVE);
		os->_oModeFlags &= ~OMF_ACTIVE;
		os->_oSelFlag = 0;
		if (deltaload)
			return;
		if (sendmsg)
			NetSendCmdParam1(CMD_OPERATEOBJ, oi);

		AddMissile(0, 0, 0, 0, 0, MIS_INFRA, MST_NA, pnum, currLvl._dLevel >> 1);
		break;
	case OBJ_TEARFTN:
		if (deltaload)
			return;

		PlrIncHp(pnum, 64);
		if (plr._pMana >= 64)
			PlrDecMana(pnum, 64);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);
}

static void OperateWeaponRack(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;
	// os->_oSolidFlag = TRUE;
	os->_oMissFlag = TRUE;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	assert(currLvl._dLevel >= 4); // must have an appropriate item for each type
	static_assert(ITYPE_SWORD + 1 == ITYPE_AXE, "OperateWeaponRack expects an ordered ITYPE_ for weapons I.");
	static_assert(ITYPE_AXE + 1 == ITYPE_BOW, "OperateWeaponRack expects an ordered ITYPE_ for weapons II.");
	static_assert(ITYPE_BOW + 1 == ITYPE_MACE, "OperateWeaponRack expects an ordered ITYPE_ for weapons III.");
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, CFDQ_GOOD, ITYPE_SWORD + random_(0, 4), IMISC_NONE, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

/**
 * Handle the reading of story books in the dungeon.
 */
static void OperateStoryBook(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	// assert(os->_oSelFlag != 0);

	os->_oAnimFrame = os->_oVar4; // STORY_BOOK_READ_FRAME
	if (deltaload) {
#ifdef HELLFIRE // STORY_BOOK_NAKRUL_IDX
		if (currLvl._dLevelIdx == DLV_CRYPT4 && os->_oVar8 == QNB_BOOK_C) {
			if (quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKOPEN)
				WakeUberDiablo();
			if (quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKOPEN /*|| quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_DEADOPEN*/)
				OpenUberRoom();
		}
#endif
		return;
	}
	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT && quests[Q_NAKRUL]._qactive != QUEST_DONE) {
		if (os->_oVar8 != 0) { // STORY_BOOK_NAKRUL_IDX
			assert(currLvl._dLevelIdx == DLV_CRYPT4);
			if (pnum != -1) {
				quests[Q_NAKRUL]._qvar1 = ProgressUberLever(os->_oVar8, quests[Q_NAKRUL]._qvar1);
				if (sendmsg)
					NetSendCmdQuest(Q_NAKRUL, true);
			}
			if (quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKC) {
				quests[Q_NAKRUL]._qvar1 = QV_NAKRUL_BOOKOPEN;
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
		StartQTextMsg(os->_oVar2); // STORY_BOOK_MSG
}

static void OperateLazStand(int oi, bool sendmsg)
{
	ObjectStruct* os = &objects[oi];

	if (numitems >= MAXITEMS) {
		return;
	}
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFrame++; // 2

	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	SpawnQuestItemAt(IDI_LAZSTAFF, os->_ox, os->_oy, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
}

static void OperateCrux(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	bool triggered;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oAnimFlag = OAM_SINGLE;
	// os->_oAnimFrame = 1;
	// os->_oAnimFrameLen = 1;
	// os->_oSolidFlag = TRUE;
	os->_oMissFlag = TRUE;
	os->_oBreak = OBM_BROKEN;

	triggered = CheckCrux(os->_oVar8); // LEVER_EFFECT
	if (triggered)
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/);

	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		// os->_oAnimCnt = 0;
		// os->_oAnimFrameLen = 1000;
		return;
	}

	if (sendmsg || pnum == -1) // send message if the crux was destroyed by a missile
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(LS_BONESP, os->_ox, os->_oy);

	if (triggered)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
}

static void OperateBarrel(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os = &objects[oi];
	int xotype, mpo;
	int xp, yp;

	if (!(os->_oModeFlags & OMF_ACTIVE))
		return;
	os->_oModeFlags &= ~OMF_ACTIVE;

	// os->_oVar1 = 0;
	// os->_oAnimFlag = OAM_SINGLE;
	// os->_oAnimFrame = 1;
	// os->_oAnimFrameLen = 1;
	os->_oSolidFlag = FALSE;
	// os->_oMissFlag = TRUE;
	os->_oBreak = OBM_BROKEN;
	os->_oSelFlag = 0;
	os->_oPreFlag = TRUE;
	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		// os->_oAnimCnt = 0;
		// os->_oAnimFrameLen = 1000;
		return;
	}
	os->_oAnimFlag = OAM_SINGLE;
	// os->_oAnimFrame = 1;

	assert(os->_oSFXCnt == 1);
	PlaySfxLoc(os->_oSFX, os->_ox, os->_oy);

	xotype = OBJ_BARRELEX;
#ifdef HELLFIRE
	if (currLvl._dType == DTYPE_CRYPT)
		xotype = OBJ_URNEX;
	else if (currLvl._dType == DTYPE_NEST)
		xotype = OBJ_PODEX;
#endif

	SetRndSeed(os->_oRndSeed);
	if (os->_otype == xotype) {
		for (yp = os->_oy - 1; yp <= os->_oy + 1; yp++) {
			for (xp = os->_ox - 1; xp <= os->_ox + 1; xp++) {
				AddMissile(xp, yp, 0, 0, 0, MIS_BARRELEX, MST_OBJECT, -1, 0);
				mpo = dObject[xp][yp];
				if (mpo > 0) {
					mpo--;
					if (objects[mpo]._otype == xotype && objects[mpo]._oBreak == OBM_BREAKABLE)
						OperateBarrel(pnum, mpo, sendmsg);
				}
			}
		}
	} else {
		if (os->_oVar2 <= 1) {   // BARREL_ITEM
			if (os->_oVar3 == 0) // BARREL_ITEM_TYPE
				SpawnRndUseful(os->_ox, os->_oy, sendmsg);
			else
				CreateRndItem(os->_ox, os->_oy, CFDQ_NORMAL, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
		} else if (os->_oVar2 >= 8)
			SpawnSkeleton(os->_oVar4, os->_ox, os->_oy, DIR_NONE); // BARREL_SKELE
	}
	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);
}

void OperateObject(int pnum, int oi, bool TeleFlag)
{
	bool sendmsg;

	sendmsg = (pnum == mypnum);
	if (!(objects[oi]._oModeFlags & OMF_ACTIVE))
		return;
	switch (objects[oi]._otype) {
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
		if (TeleFlag || PlrCheckDoor(oi, pnum))
			OperateL1Door(oi, sendmsg);
		break;
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
		if (TeleFlag || PlrCheckDoor(oi, pnum))
			OperateL5Door(oi, sendmsg);
		break;
#endif
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		if (TeleFlag || PlrCheckDoor(oi, pnum))
			OperateL2Door(oi, sendmsg);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		if (TeleFlag || PlrCheckDoor(oi, pnum))
			OperateL3Door(oi, sendmsg);
		break;
	case OBJ_LEVER:
#ifdef HELLFIRE
	case OBJ_L5LEVER:
#endif
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
#ifdef HELLFIRE
	case OBJ_L5SARC:
#endif
		OperateSarc(oi, sendmsg);
		break;
	//case OBJ_FLAMELVR:
	//	OperateFlameTrapLever(oi, sendmsg);
	//	break;
	case OBJ_BLINDBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_STEELTOME:
	//case OBJ_BOOKLVR:
	case OBJ_BOOK2R:
		OperateBookLever(pnum, oi, sendmsg);
		break;
	case OBJ_CRUXM:
	case OBJ_CRUXR:
	case OBJ_CRUXL:
		OperateCrux(pnum, oi, sendmsg);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
#ifdef HELLFIRE
	case OBJ_URN:
	case OBJ_URNEX:
	case OBJ_POD:
	case OBJ_PODEX:
#endif
		OperateBarrel(pnum, oi, sendmsg);
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		OperateShrine(pnum, oi, sendmsg);
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		OperateSkelBook(oi, sendmsg);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		OperateBookCase(oi, sendmsg);
		break;
	case OBJ_DECAP:
		OperateDecap(oi, sendmsg);
		break;
	case OBJ_ARMORSTAND:
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
#ifdef HELLFIRE
	case OBJ_L5BOOK:
#endif
		OperateStoryBook(pnum, oi, sendmsg);
		break;
	case OBJ_PEDESTAL:
		OperatePedestal(pnum, oi, sendmsg);
		break;
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
	if (objects[oi]._oVar4 == DOOR_CLOSED)
		SyncOpObject(oi);
}
void SyncDoorClose(int oi)
{
	if (objects[oi]._oVar4 == DOOR_OPEN)
		SyncOpObject(oi);
}

void SyncTrapDisable(int oi)
{
	objects[oi]._oVar4 = TRAP_INACTIVE;           // TRAP_LIVE
	objects[objects[oi]._oVar1]._oTrapChance = 0; // TRAP_OI_REF
}

void SyncTrapOpen(int oi)
{
	if (objects[oi]._oAnimFrame == FLAMETRAP_INACTIVE_FRAME)
		SyncOpObject(oi);
}
void SyncTrapClose(int oi)
{
	if (objects[oi]._oAnimFrame == FLAMETRAP_ACTIVE_FRAME)
		SyncOpObject(oi);
}

void SyncOpObject(/*int pnum,*/ int oi)
{
	OperateObject(-1, oi, true);

	/*if (!(objects[oi]._oModeFlags & OMF_ACTIVE))
		return;
	switch (objects[oi]._otype) {
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
		OperateL1Door(oi, false);
		break;
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
		OperateL5Door(oi, false);
		break;
#endif
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		OperateL2Door(oi, false);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		OperateL3Door(oi, false);
		break;
	case OBJ_LEVER:
#ifdef HELLFIRE
	case OBJ_L5LEVER:
#endif
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
#ifdef HELLFIRE
	case OBJ_L5SARC:
#endif
		OperateSarc(oi, false);
		break;
	//case OBJ_FLAMELVR:
	//	OperateFlameTrapLever(oi, false);
	//	break;
	case OBJ_BLINDBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_STEELTOME:
	//case OBJ_BOOKLVR:
	case OBJ_BOOK2R:
		OperateBookLever(pnum, oi, false);
		break;
	case OBJ_CRUXM:
	case OBJ_CRUXR:
	case OBJ_CRUXL:
		OperateCrux(pnum, oi, false);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
#ifdef HELLFIRE
	case OBJ_URN:
	case OBJ_URNEX:
	case OBJ_POD:
	case OBJ_PODEX:
#endif
		OperateBarrel(pnum, oi, false);
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		OperateShrine(pnum, oi, false);
		break;
	case OBJ_SKELBOOK:
	case OBJ_BOOKSTAND:
		OperateSkelBook(oi, false);
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		OperateBookCase(oi, false);
		break;
	case OBJ_DECAP:
		OperateDecap(oi, false);
		break;
	case OBJ_ARMORSTAND:
		OperateArmorStand(oi, false);
		break;
	case OBJ_GOATSHRINE:
		OperateGoatShrine(pnum, oi, false);
		break;
	case OBJ_CAULDRON:
		OperateCauldron(pnum, oi, false);
		break;
	// case OBJ_BLOODFTN:
	// case OBJ_PURIFYINGFTN:
	case OBJ_MURKYFTN:
	case OBJ_TEARFTN:
		OperateFountains(pnum, oi, false);
		break;
	case OBJ_STORYBOOK:
#ifdef HELLFIRE
	case OBJ_L5BOOK:
#endif
		OperateStoryBook(-1, oi, false);
		break;
	case OBJ_PEDESTAL:
		OperatePedestal(-1, oi, false);
		break;
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
	}*/
}

static void SyncLever(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	if (CheckLeverGroup(os->_otype, os->_oVar8)) // LEVER_INDEX
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
}

static void SyncBookLever(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	if (os->_oAnimFrame == os->_oVar6) { // LEVER_BOOK_ANIM
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, os->_otype == OBJ_BLINDBOOK*/); // LEVER_EFFECT
		//if (os->_otype == OBJ_BLINDBOOK) {
			//int tv = dTransVal[2 * os->_oVar1 + DBORDERX + 1][2 * os->_oVar2 + DBORDERY + 1];
			//DRLG_MRectTrans(os->_oVar1 + 2, os->_oVar2 + 2, os->_oVar1 + 4, os->_oVar2 + 4, tv); // LEVER_EFFECT
			//DRLG_MRectTrans(os->_oVar1 + 6, os->_oVar2 + 6, os->_oVar1 + 8, os->_oVar2 + 8, tv); // LEVER_EFFECT
		//}
	}
}

static void SyncCrux(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	if (CheckCrux(os->_oVar8)) // LEVER_EFFECT
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
}

static void SyncL1Doors(int oi)
{
	ObjectStruct* os;
	int x, y, pn;

	os = &objects[oi];

	x = os->_ox;
	y = os->_oy;
	if (os->_oVar4 == DOOR_CLOSED) {
		dPiece[x][y] = os->_oVar1; // DOOR_PIECE_CLOSED
		dSpecial[x][y] = 0;
		pn = os->_oVar2;           // DOOR_SIDE_PIECE_CLOSED

		if (os->_otype == OBJ_L1LDOOR)
			y--;
		else
			x--;
		// commented out because this is not possible with the current implementation
		//if (pn == 50 && dPiece[x][y] == 396)
		//	pn = os->_otype == OBJ_L1LDOOR ? 412 : 411;
		dPiece[x][y] = pn;
		return;
	}

	if (os->_otype == OBJ_L1LDOOR) {
		dPiece[x][y] = os->_oVar1 == 214 ? 408 : 393; // DOOR_PIECE_CLOSED
		dSpecial[x][y] = 7;
		y--;
	} else {
		dPiece[x][y] = 395;
		dSpecial[x][y] = 8;
		x--;
	}
	//ObjSetDoorSidePiece(x, y/*, os->_otype*/);
}

#ifdef HELLFIRE
static void SyncL5Doors(int oi)
{
	ObjectStruct* os;
	int x, y, pn;

	os = &objects[oi];

	x = os->_ox;
	y = os->_oy;
	if (os->_oVar4 == DOOR_CLOSED) {
		dPiece[x][y] = os->_oVar1; // DOOR_PIECE_CLOSED
		pn = os->_oVar2;           // DOOR_SIDE_PIECE_CLOSED

		if (os->_otype == OBJ_L5LDOOR)
			y--;
		else
			x--;
		// commented out because this is not possible with the current implementation
		//if (pn == 86 && dPiece[x][y] == 212)
		//	pn = os->_otype == OBJ_L5LDOOR ? 234 : 232;
		dPiece[x][y] = pn;
		return;
	}

	if (os->_otype == OBJ_L5LDOOR) {
		dPiece[x][y] = 206;
		//dSpecial[x][y] = 1;
		y--;
	} else {
		dPiece[x][y] = 209;
		//dSpecial[x][y] = 2;
		x--;
	}
	ObjSetDoorSidePiece(x, y/*, os->_otype*/);
}
#endif

static void SyncL2Doors(int oi)
{
	ObjectStruct* os;
	int pn, x, y;
	bool ldoor;
	BYTE sn;

	os = &objects[oi];
	ldoor = os->_otype == OBJ_L2LDOOR;
	if (os->_oVar4 == DOOR_CLOSED) {
		pn = ldoor ? 538 : 540;
		sn = 0;
	} else { // if (os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED) {
		pn = ldoor ? 13 : 17;
		sn = ldoor ? 5 : 6;
	}
	x = os->_ox;
	y = os->_oy;
	dPiece[x][y] = pn;
	dSpecial[x][y] = sn;
}

static void SyncL3Doors(int oi)
{
	ObjectStruct* os;
	int pn;
	bool ldoor;

	os = &objects[oi];
	ldoor = os->_otype == OBJ_L3LDOOR;
	if (os->_oVar4 == DOOR_CLOSED) {
		pn = ldoor ? 534 : 531;
	} else { // if (os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED)
		pn = ldoor ? 541 : 538;
	}
	dPiece[os->_ox][os->_oy] = pn;
}

void SyncObjectAnim(int oi)
{
	int type, ofidx;

	type = objects[oi]._otype;
	ofidx = objectdata[type].ofindex;
	objects[oi]._oAnimData = objanimdata[ofidx];
	objects[oi]._oAnimFrameLen = objfiledata[ofidx].oAnimFrameLen;
	objects[oi]._oAnimWidth = objfiledata[ofidx].oAnimWidth * ASSET_MPL;
	objects[oi]._oAnimXOffset = (objects[oi]._oAnimWidth - TILE_WIDTH) >> 1;
	switch (type) {
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
		SyncL1Doors(oi);
		break;
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
		SyncL5Doors(oi);
		break;
#endif
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		SyncL2Doors(oi);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		SyncL3Doors(oi);
		break;
	case OBJ_LEVER:
#ifdef HELLFIRE
	case OBJ_L5LEVER:
#endif
	case OBJ_BOOK2L: // TODO: only if currLvl._dLevelIdx == SL_VILEBETRAYER? NULL_LVR_EFFECT
	case OBJ_SWITCHSKL:
		SyncLever(oi);
		break;
	case OBJ_CRUXM:
	//case OBJ_CRUXR: -- check only one of them
	//case OBJ_CRUXL:
		SyncCrux(oi);
		break;
	case OBJ_BOOK2R:
	case OBJ_BLINDBOOK:
	//case OBJ_BLOODBOOK: -- NULL_LVR_EFFECT
	case OBJ_STEELTOME:
		SyncBookLever(oi);
		break;
	case OBJ_PEDESTAL:
		SyncPedestal(/*oi*/);
		break;
	}
}

void GetObjectStr(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	switch (os->_otype) {
	case OBJ_LEVER:
#ifdef HELLFIRE
	case OBJ_L5LEVER:
#endif
	//case OBJ_FLAMELVR:
		copy_cstr(infostr, "Lever");
		break;
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		copy_cstr(infostr, "Small Chest");
		break;
	case OBJ_L1LDOOR:
	case OBJ_L1RDOOR:
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
#endif
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
	case OBJ_CRUXM:
	case OBJ_CRUXR:
	case OBJ_CRUXL:
		copy_cstr(infostr, "Crucified Skeleton");
		break;
	case OBJ_SARC:
#ifdef HELLFIRE
	case OBJ_L5SARC:
#endif
		copy_cstr(infostr, "Sarcophagus");
		break;
	//case OBJ_BOOKSHELF:
	//	copy_cstr(infostr, "Bookshelf");
	//	break;
#ifdef HELLFIRE
	case OBJ_URN:
	case OBJ_URNEX:
		copy_cstr(infostr, "Urn");
		break;
	case OBJ_POD:
	case OBJ_PODEX:
		copy_cstr(infostr, "Pod");
		break;
#endif
	case OBJ_BARREL:
	case OBJ_BARRELEX:
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
	case OBJ_PEDESTAL:
		copy_cstr(infostr, "Pedestal of Blood");
		break;
	case OBJ_PURIFYINGFTN:
		copy_cstr(infostr, "Purifying Spring");
		break;
	case OBJ_ARMORSTAND:
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
#ifdef HELLFIRE
	case OBJ_L5BOOK:
#endif
		copy_cstr(infostr, StoryBookName[os->_oVar3]); // STORY_BOOK_NAME
		break;
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
	if (os->_oTrapChance != 0 && (3 * currLvl._dLevel + os->_oTrapChance) < myplr._pBaseDex) { // TRAP_CHANCE
		snprintf(tempstr, sizeof(tempstr), "Trapped %s", infostr);
		copy_str(infostr, tempstr);
		infoclr = COL_RED;
	}
}

#ifdef HELLFIRE
void OpenUberRoom()
{
	ObjChangeMap(pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 2, pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 3/*, false*/);
}
#endif

DEVILUTION_END_NAMESPACE
