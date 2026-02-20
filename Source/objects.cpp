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
static int objanimdim[NUM_OFILE_TYPES];
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
const char* const BookName[NUM_BOOKS] = {
/*BK_STORY_MAINA_1*/  "The Great Conflict",
/*BK_STORY_MAINA_2*/  "The Wages of Sin are War",
/*BK_STORY_MAINA_3*/  "The Tale of the Horadrim",
/*BK_STORY_MAINB_1*/  "The Dark Exile",
/*BK_STORY_MAINB_2*/  "The Sin War",
/*BK_STORY_MAINB_3*/  "The Binding of the Three",
/*BK_STORY_MAINC_1*/  "The Realms Beyond",
/*BK_STORY_MAINC_2*/  "Tale of the Three",
/*BK_STORY_MAINC_3*/  "The Black King",
/*BK_BLOOD*/          "Book of Blood",
/*BK_ANCIENT*/        "Ancient Book",
/*BK_STEEL*/          "Steel Tome",
/*BK_BLIND*/          "Book of the Blind",
/*BK_MYTHIC*/         "Mythical Book",
/*BK_VILENESS*/       "Book of Vileness",
#ifdef HELLFIRE
	//"Journal: The Ensorcellment",
/*BK_STORY_NAKRUL_1*/ "Journal: The Meeting",
/*BK_STORY_NAKRUL_2*/ "Journal: The Tirade",
/*BK_STORY_NAKRUL_3*/ "Journal: His Power Grows",
/*BK_STORY_NAKRUL_4*/ "Journal: NA-KRUL",
/*BK_STORY_NAKRUL_5*/ "Journal: The End",
/*BK_NAKRUL_SPELL*/   "Spellbook",
#endif
};

#if FLICKER_LIGHT
const int flickers[32] = {
	1, 1, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 1
	//{ 0, 0, 0, 0, 0, 0, 1, 1, 1 }
};
#endif

static void AddObjectType(const ObjectData* ods)
{
	const int ofindex = ods->ofindex;
	char filestr[DATA_ARCHIVE_MAX_PATH];

	if (objanimdata[ofindex] != NULL) {
		return;
	}

	snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", objfiledata[ofindex].ofName);
	objanimdata[ofindex] = LoadFileInMem(filestr);
	objanimdim[ofindex] = CelClippedWidth(objanimdata[ofindex]);
}

void InitObjectGFX()
{
	const ObjectData* ods;
	bool themeload[NUM_THEMES];
	int i;

	static_assert(false == 0, "InitObjectGFX fills themeload with 0 instead of false values.");
	memset(themeload, 0, sizeof(themeload));

	for (i = 0; i < numthemes; i++)
		themeload[themes[i]._tsType] = true;

	BYTE lvlMask = 1 << currLvl._dType;
	for (i = NUM_OBJECTS - 1; i >= 0; i--) {
		ods = &objectdata[i];
		if (!(ods->oLvlTypes & lvlMask)
		 && (ods->otheme == THEME_NONE || !themeload[ods->otheme])
		 && (ods->oquest == Q_INVALID || !QuestStatus(ods->oquest))) {
			continue;
		}
		AddObjectType(ods);
	}
}

void FreeObjectGFX()
{
	int i;

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		MemFreeDbg(objanimdata[i]);
	}
}

void InitLvlObjects()
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

static void SetObjMapRange(int oi, int x1, int y1, int x2, int y2, int v)
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

/**
 * Check the location if an object can be placed there in the init phase.
 * Must not consider the player's position, since it could change the dungeon
 * when a player re-enters the dungeon.
 */
static bool RndLocOk(int xp, int yp)
{
	if ((dMonster[xp][yp] | /*dPlayer[xp][yp] |*/ dObject[xp][yp]
	 | nSolidTable[dPiece[xp][yp]] | (dFlags[xp][yp] & BFLAG_OBJ_PROTECT)) != 0)
		return false;
	// should be covered by Freeupstairs.
	//if (currLvl._dDunType != DGT_CATHEDRAL || dPiece[xp][yp] <= 126 || dPiece[xp][yp] >= 144)
		return true;
	//return false;
}

static POS32 RndLoc3x3()
{
	int xp, yp, i, j, tries;
	static_assert(DBORDERX != 0, "RndLoc3x3 returns 0;0 position as a failed location.");
	tries = 0;
	while (true) {
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
	while (true) {
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
	while (true) {
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
	while (true) {
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

static void InitRndLocObj(int numobjs, int objtype)
{
	int i;
	POS32 pos;

	for (i = numobjs; i > 0; i--) {
		pos = RndLoc3x3();
		if (pos.x == 0)
			break;
		AddObject(objtype, pos.x, pos.y);
	}
}

static void InitRndSarcs(int numobjs, int objtype)
{
	int i;
	POS32 pos;

	for (i = numobjs; i > 0; i--) {
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

static void AddCandles()
{
	int tx, ty;

	tx = pWarps[DWARP_SIDE]._wx;
	ty = pWarps[DWARP_SIDE]._wy;
	AddObject(OBJ_STORYCANDLE, tx - 2, ty + 1);
	AddObject(OBJ_STORYCANDLE, tx + 2, ty + 1);
	AddObject(OBJ_STORYCANDLE, tx - 1, ty + 2);
	AddObject(OBJ_STORYCANDLE, tx + 1, ty + 2);
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
	objects[oi]._oVar6 = objects[oi]._oGfxFrame + 1; // LEVER_BOOK_ANIM
	objects[oi]._oVar7 = qn; // LEVER_BOOK_QUEST
}

// generate numobjs groups of barrels
static void InitRndBarrels(int numobjs, int otype)
{
	int i, xp, yp;
	int dir;
	int t; // number of tries of placing next barrel in current group
	int c; // number of barrels in current group

	// assert(otype == OBJ_BARREL || otype == OBJ_URN || otype == OBJ_POD);
	static_assert((int)OBJ_BARREL + 1 == (int)OBJ_BARRELEX, "InitRndBarrels expects ordered BARREL enum I.");
#ifdef HELLFIRE
	static_assert((int)OBJ_URN + 1 == (int)OBJ_URNEX, "InitRndBarrels expects ordered BARREL enum II.");
	static_assert((int)OBJ_POD + 1 == (int)OBJ_PODEX, "InitRndBarrels expects ordered BARREL enum III.");
#endif

	for (i = numobjs; i > 0; i--) {
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
	int i, j, pn, wdoor, edoor, type;

	assert((objectdata[OBJ_L1LDOOR].oLvlTypes & DTM_CATHEDRAL) && (objectdata[OBJ_L1RDOOR].oLvlTypes & DTM_CATHEDRAL) && (objectdata[OBJ_L1LIGHT].oLvlTypes & DTM_CATHEDRAL));
	assert((objectdata[OBJ_L2LDOOR].oLvlTypes & DTM_CATACOMBS) && (objectdata[OBJ_L2RDOOR].oLvlTypes & DTM_CATACOMBS));
	assert((objectdata[OBJ_L3LDOOR].oLvlTypes & DTM_CAVES) && (objectdata[OBJ_L3RDOOR].oLvlTypes & DTM_CAVES));
#ifdef HELLFIRE
	assert((objectdata[OBJ_L5LDOOR].oLvlTypes & DTM_CRYPT) && (objectdata[OBJ_L5RDOOR].oLvlTypes & DTM_CRYPT));
#endif
	switch (currLvl._dType) {
	case DTYPE_TOWN:
		return;
	case DTYPE_CATHEDRAL:
		for (j = y1; j <= y2; j++) {
			for (i = x1; i <= x2; i++) {
				pn = dPiece[i][j];
				if (pn == 270)
					AddObject(OBJ_L1LIGHT, i, j);
			}
		}
		wdoor = OBJ_L1LDOOR;
		edoor = OBJ_L1RDOOR;
		break;
	case DTYPE_CATACOMBS:
		wdoor = OBJ_L2LDOOR;
		edoor = OBJ_L2RDOOR;
		break;
	case DTYPE_CAVES:
		wdoor = OBJ_L3LDOOR;
		edoor = OBJ_L3RDOOR;
		break;
	case DTYPE_HELL:
		return;
#ifdef HELLFIRE
	case DTYPE_CRYPT:
		wdoor = OBJ_L5LDOOR;
		edoor = OBJ_L5RDOOR;
		break;
	case DTYPE_NEST:
		return;
#endif
	default:
		ASSUME_UNREACHABLE
		return;
	}
	for (j = y1; j <= y2; j++) {
		for (i = x1; i <= x2; i++) {
			pn = dPiece[i][j];
			type = automaptype[pn] & MAT_TYPE;
			if ((type == MAT_DOOR_WEST || type == MAT_DOOR_EAST) && !nSolidTable[pn]) {
				dev_fatal("Non-blocking door pn:%d type:%d tile:%d", pn, type, dungeon[(i - DBORDERX) >> 1][(j - DBORDERY) >> 1]);
			}
			if (type == MAT_DOOR_WEST) {
				AddObject(wdoor, i, j);
			} else if (type == MAT_DOOR_EAST) {
				AddObject(edoor, i, j);
			}
		}
	}
}

static void AddL2Torches()
{
	int i, j;
	// place torches on NW->SE walls
	for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
		for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
			// skip setmap pieces
			if (dFlags[i][j] & BFLAG_OBJ_PROTECT)
				continue;
			// select 'trapable' position
			if ((nSpecTrapTable[dPiece[i][j]] & PST_TRAP_TYPE) != PST_LEFT)
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
	// place torches on NE->SW walls
	for (j = DBORDERY; j < DBORDERY + DSIZEY; j++) {
		for (i = DBORDERX; i < DBORDERX + DSIZEX; i++) {
			// skip setmap pieces
			if (dFlags[i][j] & BFLAG_OBJ_PROTECT)
				continue;
			// select 'trapable' position
			if ((nSpecTrapTable[dPiece[i][j]] & PST_TRAP_TYPE) != PST_RIGHT)
				continue;
			if (random_(145, 32) != 0)
				continue;
			// assert(nSolidTable[dPiece[i - 1][j]] | nSolidTable[dPiece[i + 1][j]]);
			if (!nSolidTable[dPiece[i][j + 1]]) {
				if (dObject[i][j] == 0) // check torches from the previous loop
					AddObject(OBJ_TORCHR1, i, j);
			} else {
				if (dObject[i][j - 1] == 0) // check torches from the previous loop
					AddObject(OBJ_TORCHR2, i, j - 1);
			}
			// skip a few tiles to prevent close placement
			i += 4;
		}
	}
}

static void AddObjTraps()
{
	int i, ox, oy, tx, ty, on, rndv;

	rndv = 10 + (currLvl._dLevel >> 1);
	for (i = numobjects - 1; i >= 0; i--) {
		int oi = i; // objectactive[i];
		ObjectStruct* os = &objects[oi];
		if (!objectdata[os->_otype].oTrapFlag)
			continue;
		if (random_(144, 128) >= rndv)
			continue;
		ox = os->_ox;
		oy = os->_oy;
		if (random_(144, 2) == 0) {
			tx = ox - 1;
			while (!nSolidTable[dPiece[tx][oy]])
				tx--;

			if (ox - tx <= 1)
				continue;

			ty = oy;
			on = OBJ_TRAPL;
		} else {
			ty = oy - 1;
			while (!nSolidTable[dPiece[ox][ty]])
				ty--;

			if (oy - ty <= 1)
				continue;

			tx = ox;
			on = OBJ_TRAPR;
		}
		if (dFlags[tx][ty] & BFLAG_OBJ_PROTECT)
			continue;
		if (dObject[tx][ty] != 0)
			continue;
		if ((nSpecTrapTable[dPiece[tx][ty]] & PST_TRAP_TYPE) == PST_NONE)
			continue;
		on = AddObject(on, tx, ty);
		if (on == -1)
			return;
		objects[on]._oVar1 = oi; // TRAP_OI_REF
		objects[oi]._oTrapChance = RandRange(1, 64);
		objects[oi]._oVar5 = on + 1; // TRAP_OI_BACKREF
	}
}

static void AddChestTraps()
{
	int i;

	for (i = numobjects - 1; i >= 0; i--) {
		int oi = i; // objectactive[i];
		ObjectStruct* os = &objects[oi];
		if (os->_otype >= OBJ_CHEST1 && os->_otype <= OBJ_CHEST3 && os->_oTrapChance == 0 && random_(0, 100) < 10) {
			os->_otype += OBJ_TCHEST1 - OBJ_CHEST1;
			os->_oTrapChance = RandRange(1, 64);
			os->_oVar5 = 0; // TRAP_OI_BACKREF
		}
	}
}

static void LoadMapSetObjects(int idx)
{
	int startx = DBORDERX + pSetPieces[idx]._spx * 2;
	int starty = DBORDERY + pSetPieces[idx]._spy * 2;
	const BYTE* pMap = pSetPieces[idx]._spData;
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

	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				oidx = SwapLE16(*lm); // index of ObjConvTbl
				assert(oidx < lengthof(ObjConvTbl));
				oidx = ObjConvTbl[oidx]; // index of objectdata
				assert(oidx != 0);
				AddObjectType(&objectdata[oidx]);
				AddObject(oidx, i, j);
			}
			lm++;
		}
	}
	//gbInitObjFlag = false; -- setpieces, setmap levers?
}

static int SetupObject(int type, int ox, int oy)
{
	int oi;
	ObjectStruct* os;
	const ObjectData* ods;
	const ObjFileData* ofd;

	if (numobjects >= MAXOBJECTS)
		return -1;

//	oi = objectavail[0];
	oi = numobjects;
	// objectactive[numobjects] = oi;
	numobjects++;
//	objectavail[0] = objectavail[MAXOBJECTS - numobjects];

	os = &objects[oi];
	os->_otype = type;
	ods = &objectdata[type];
	os->_oMissFlag = ods->oMissFlag;
	os->_oDoorFlag = ods->oDoorFlag;
	os->_oSelFlag = ods->oSelFlag;
	os->_oPreFlag = ods->oPreFlag;
	os->_oProc = ods->oProc;
	os->_oModeFlags = ods->oModeFlags;
	os->_oGfxFrame = ods->oBaseFrame;
	os->_oAnimData = objanimdata[ods->ofindex];
	ofd = &objfiledata[ods->ofindex];
	os->_oSFX = ofd->oSFX;
	os->_oSFXCnt = ofd->oSFXCnt;
	os->_oAnimFlag = ofd->oAnimFlag;
	os->_oAnimFrameLen = ofd->oAnimFrameLen;
	os->_oAnimLen = ofd->oAnimLen;
	os->_oAnimCnt = 0;
	os->_oAnimFrame = 0;
	if (ofd->oAnimFlag != OAM_NONE) {
		if (ofd->oAnimFlag == OAM_SINGLE) {
			os->_oAnimFlag = OAM_NONE;
			os->_oAnimFrame = 1;
		} else {
			// assert(ofd->oAnimFlag == OAM_LOOP);
			os->_oAnimCnt = random_low(146, os->_oAnimFrameLen);
			os->_oAnimFrame = RandRangeLow(1, os->_oAnimLen);
		}
	}
	os->_oAnimWidth = objanimdim[ods->ofindex];
	os->_oAnimXOffset = (os->_oAnimWidth - TILE_WIDTH) >> 1;
	os->_oSolidFlag = ofd->oSolidFlag;
	os->_oBreak = ofd->oBreak;
	// os->_oDelFlag = FALSE; - unused
	os->_oTrapChance = 0;
	// place object
	os->_ox = ox;
	os->_oy = oy;
	assert(dObject[ox][oy] == 0);
	dObject[ox][oy] = oi + 1;
	// dFlags[ox][oy] |= BFLAG_OBJ_PROTECT | BFLAG_MON_PROTECT;
	if (nSolidTable[dPiece[ox][oy]] && (os->_oModeFlags & OMF_FLOOR)) {
		dObject[ox][oy] = 0;
		os->_oModeFlags |= OMF_RESERVED;
		os->_oSelFlag = 0;
	} else {
		const BYTE olr = ods->oLightRadius;
		if (olr != 0) {
#if FLICKER_LIGHT
			if (type == OBJ_L1LIGHT) {
				os->_olid = NO_LIGHT;
			} else
#endif
			{
				TraceLightSource(ox + ((olr & OLF_XO) ? 1 : 0), oy + ((olr & OLF_YO) ? 1 : 0), olr & OLF_MASK);
			}
		}
		if (ods->oDoorFlag != ODT_NONE) {
			os->_oVar4 = DOOR_CLOSED;
			//os->_oPreFlag = FALSE;
			//os->_oSelFlag = 3;
			//os->_oSolidFlag = FALSE; // TODO: should be TRUE;
			//os->_oMissFlag = FALSE;
			//os->_oDoorFlag = ldoor ? ODT_LEFT : ODT_RIGHT;
			os->_oVar1 = dPiece[ox][oy]; // DOOR_PIECE_CLOSED
		}
	}
	return oi;
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

static void AddSKingObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 48, DBORDERY + 18), 20, 7, 23, 10, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 48, DBORDERY + 43), 20, 14, 21, 16, 2);
	SetObjMapRange(ObjIndex(DBORDERX + 11, DBORDERY + 21), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 30, DBORDERY + 19), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 33, DBORDERY + 37), 8, 1, 15, 11, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 11, DBORDERY + 37), 8, 1, 15, 11, 3);
}

static void AddBChamObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 21, DBORDERY + 14), 17, 0, 21, 5, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 21, DBORDERY + 30), 13, 0, 16, 5, 2);
}

static void AddVileObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 10, DBORDERY + 29), 3, 4, 8, 10, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 29, DBORDERY + 30), 11, 4, 16, 10, 2);
	//SetObjMapRange(ObjIndex(DBORDERX + 19, DBORDERY + 20), 7, 11, 13, 18, 3);
}

/*static void AddMazeObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 33, DBORDERY + 25), 0?, 0?, 45?, ?, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 15, DBORDERY + 51), ?, ?, ?, ?, ?);
	SetObjMapRange(ObjIndex(DBORDERX + 27, DBORDERY + 51), ?, ?, ?, ?, ?);
	SetObjMapRange(ObjIndex(DBORDERX + 33, DBORDERY + 57), ?, ?, ?, ?, ?);
	SetObjMapRange(ObjIndex(DBORDERX + 79, DBORDERY + 51), ?, ?, ?, ?, ?);
}*/

static void AddDiabObjs()
{
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[0]._spx + 5, DBORDERY + 2 * pSetPieces[0]._spy + 5), pSetPieces[1]._spx, pSetPieces[1]._spy, pSetPieces[1]._spx + 11, pSetPieces[1]._spy + 12, 1);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[1]._spx + 13, DBORDERY + 2 * pSetPieces[1]._spy + 10), pSetPieces[2]._spx, pSetPieces[2]._spy, pSetPieces[2]._spx + 11, pSetPieces[2]._spy + 11, 2);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[2]._spx + 8, DBORDERY + 2 * pSetPieces[2]._spy + 2), pSetPieces[3]._spx, pSetPieces[3]._spy, pSetPieces[3]._spx + 9, pSetPieces[3]._spy + 9, 3);
	SetObjMapRange(ObjIndex(DBORDERX + 2 * pSetPieces[2]._spx + 8, DBORDERY + 2 * pSetPieces[2]._spy + 14), pSetPieces[3]._spx, pSetPieces[3]._spy, pSetPieces[3]._spx + 9, pSetPieces[3]._spy + 9, 3);
}

#ifdef HELLFIRE
static void AddL5StoryBook(int bookidx, int ox, int oy)
{
	ObjectStruct* os;
	int oi = AddObject(OBJ_L5BOOK, ox, oy);
	// assert(oi != -1);

	os = &objects[oi];
	// assert(os->_oGfxFrame == objectdata[OBJ_L5BOOK].oBaseFrame);
	os->_oVar4 = objectdata[OBJ_L5BOOK].oBaseFrame + 1; // STORY_BOOK_READ_FRAME
	os->_oVar2 = TEXT_BOOK4 + bookidx;                      // STORY_BOOK_MSG
	os->_oVar5 = BK_STORY_NAKRUL_1 + bookidx;               // STORY_BOOK_NAME
}

static void AddNakrulBook(int oi)
{
	ObjectStruct* os;

	int bookidx;
	if (leverid == 1) {
		bookidx = random_(11, 3); // select base book index
		leverid = 16 | bookidx;   // store the selected book index + a flag
	} else {
		if ((leverid & 32) == 0) {
			bookidx = random_(12, 2); // select from the remaining two books
			leverid |= 32 | (bookidx << 2); // store second choice + a flag
			bookidx = (bookidx & 1) ? 4 : 2; // make the choice
		} else {
			bookidx = (leverid >> 2); // read second choice
			bookidx = (bookidx & 1) ? 2 : 4; // choose the remaining one
		}
		bookidx += leverid & 3; // add base book index
		bookidx = bookidx % 3;
	}
	bookidx += QNB_BOOK_A;

	os = &objects[oi];
	// assert(os->_oGfxFrame == objectdata[OBJ_NAKRULBOOK].oBaseFrame);
	os->_oVar4 = objectdata[OBJ_NAKRULBOOK].oBaseFrame + 1; // STORY_BOOK_READ_FRAME
	os->_oVar2 = TEXT_BOOKA + bookidx - QNB_BOOK_A;             // STORY_BOOK_MSG
	os->_oVar3 = bookidx;                                       // STORY_BOOK_NAKRUL_IDX
	os->_oVar5 = BK_NAKRUL_SPELL;                               // STORY_BOOK_NAME
}

static void AddLvl2xBooks(int bookidx)
{
	POS32 pos = RndLoc5x5();

	if (pos.x == 0)
		return;

	AddL5StoryBook(bookidx, pos.x, pos.y);
	AddObject(OBJ_L5CANDLE, pos.x - 1, pos.y - 1);
	AddObject(OBJ_L5CANDLE, pos.x - 1, pos.y + 1);
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
			ttv = nSpecTrapTable[dPiece[i][j]] & PST_TRAP_TYPE;
			if (ttv == PST_NONE)
				continue;
			if (dFlags[i][j] & BFLAG_OBJ_PROTECT)
				continue;
			type = random_(0, 32);
			if (type >= 3)
				continue;
			if (ttv == PST_LEFT) {
				type = OBJ_TORTUREL1 + type;
			} else {
				// assert(ttv == PST_RIGHT);
				type = OBJ_TORTURER1 + type;
			}
			AddObject(type, i, j);
		}
	}
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
	for (int i = lengthof(pSetPieces) - 1; i >= 0; i--) {
		if (pSetPieces[i]._spData != NULL) { // pSetPieces[i]._sptype != SPT_NONE
			LoadMapSetObjects(i);
		}
	}
	if (pSetPieces[0]._sptype == SPT_WARLORD) { // QuestStatus(Q_WARLORD)
		AddBookLever(OBJ_STEELTOME, pSetPieces[0]._spx + 7, pSetPieces[0]._spy + 1, pSetPieces[0]._spx + 7, pSetPieces[0]._spy + 5, Q_WARLORD);
	}
	if (pSetPieces[0]._sptype == SPT_BCHAMB) { // QuestStatus(Q_BCHAMB)
		AddBookLever(OBJ_MYTHICBOOK, pSetPieces[0]._spx, pSetPieces[0]._spy, pSetPieces[0]._spx + 5, pSetPieces[0]._spy + 5, Q_BCHAMB);
	}
	if (pSetPieces[0]._sptype == SPT_BLIND) { // QuestStatus(Q_BLIND)
		AddBookLever(OBJ_BLINDBOOK, pSetPieces[0]._spx, pSetPieces[0]._spy + 1, pSetPieces[0]._spx + 11, pSetPieces[0]._spy + 10, Q_BLIND);
	}
	if (pSetPieces[0]._sptype == SPT_LVL_SKELKING) {
		AddSKingObjs();
	}
	if (pSetPieces[0]._sptype == SPT_LVL_BCHAMB) {
		AddBChamObjs();
	}
	if (pSetPieces[0]._sptype == SPT_LVL_BETRAYER) {
		AddVileObjs();
	}
	switch (currLvl._dLevelIdx) {
	case DLV_CATHEDRAL4:
		AddStoryBook();
		break;
	case DLV_CATACOMBS4:
		AddStoryBook();
		break;
	case DLV_CAVES4:
		AddStoryBook();
		break;
	case DLV_HELL4:
		AddDiabObjs();
		return;
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
#endif
	}
	AddDunObjs(DBORDERX, DBORDERY, MAXDUNX - DBORDERX - 1, MAXDUNY - DBORDERY - 1);
	BYTE lvlMask = 1 << currLvl._dType;
	assert(objectdata[OBJ_TORCHL1].oLvlTypes == objectdata[OBJ_TORCHL2].oLvlTypes && objectdata[OBJ_TORCHL1].oLvlTypes == objectdata[OBJ_TORCHR1].oLvlTypes && objectdata[OBJ_TORCHR1].oLvlTypes == objectdata[OBJ_TORCHR2].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TORCHL1].oLvlTypes) {
		AddL2Torches();
	}
	assert(objectdata[OBJ_TORTUREL1].oLvlTypes == objectdata[OBJ_TORTUREL2].oLvlTypes);
	assert(objectdata[OBJ_TORTUREL1].oLvlTypes == objectdata[OBJ_TORTUREL3].oLvlTypes);
	assert(objectdata[OBJ_TORTUREL1].oLvlTypes == objectdata[OBJ_TORTURER1].oLvlTypes);
	assert(objectdata[OBJ_TORTUREL1].oLvlTypes == objectdata[OBJ_TORTURER2].oLvlTypes);
	assert(objectdata[OBJ_TORTUREL1].oLvlTypes == objectdata[OBJ_TORTURER3].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TORTUREL1].oLvlTypes) {
		AddHookedBodies();
	}

	unsigned na = 0;
	for (int xx = DBORDERX; xx < DSIZEX + DBORDERX; xx++)
		for (int yy = DBORDERY; yy < DSIZEY + DBORDERY; yy++)
			if ((nSolidTable[dPiece[xx][yy]] | (dFlags[xx][yy] & BFLAG_OBJ_PROTECT)) == 0)
				na++;
	na = na * AllLevels[currLvl._dLevelNum].dObjDensity / 32;

	if (lvlMask & objectdata[OBJ_SARC].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow I.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndSarcs(num / 512, OBJ_SARC);
	}
#ifdef HELLFIRE
	if (lvlMask & objectdata[OBJ_L5SARC].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow II.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndSarcs(num / 512, OBJ_L5SARC);
	}
#endif
	if (lvlMask & objectdata[OBJ_TNUDEM].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY * 2 < 0x7FFF, "InitObjects uses RandRangeLow III.");
		unsigned num = RandRangeLow(na, na * 3 + 1);
		InitRndLocObj(num / 512, OBJ_TNUDEM);
	}
	if (lvlMask & objectdata[OBJ_TNUDEW].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY * 2 < 0x7FFF, "InitObjects uses RandRangeLow IV.");
		unsigned num = RandRangeLow(na, na * 3 + 1);
		InitRndLocObj(num / 768, OBJ_TNUDEW);
	}
	if (lvlMask & objectdata[OBJ_DECAP].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow V.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndLocObj(num / 1024, OBJ_DECAP);
	}
	if (lvlMask & objectdata[OBJ_CAULDRON].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow VI.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndLocObj(num / 2048, OBJ_CAULDRON);
	}
	assert(objectdata[OBJ_BARREL].oLvlTypes == objectdata[OBJ_BARRELEX].oLvlTypes);
	if (lvlMask & objectdata[OBJ_BARREL].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow VII.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndBarrels(num / 1024, OBJ_BARREL);
	}
#ifdef HELLFIRE
	assert(objectdata[OBJ_URN].oLvlTypes == objectdata[OBJ_URNEX].oLvlTypes);
	if (lvlMask & objectdata[OBJ_URN].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow VIII.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndBarrels(num / 1024, OBJ_URN);
	}
	assert(objectdata[OBJ_POD].oLvlTypes == objectdata[OBJ_PODEX].oLvlTypes);
	if (lvlMask & objectdata[OBJ_POD].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow IX.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndBarrels(num / 1024, OBJ_POD);
	}
#endif
	assert(objectdata[OBJ_CHEST1].oLvlTypes == objectdata[OBJ_CHEST2].oLvlTypes && objectdata[OBJ_CHEST1].oLvlTypes == objectdata[OBJ_CHEST3].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TCHEST1].oLvlTypes) {
		static_assert(DSIZEX * DSIZEY < 0x7FFF, "InitObjects uses RandRangeLow X.");
		unsigned num = RandRangeLow(na, na * 2 + 1);
		InitRndLocObj(num / 512, OBJ_CHEST1);
		num = RandRangeLow(na, na * 2 + 1);
		InitRndLocObj(num / 1024, OBJ_CHEST2);
		num = RandRangeLow(na, na * 2 + 1);
		InitRndLocObj(num / 2048, OBJ_CHEST3);
	}
	assert(objectdata[OBJ_TRAPL].oLvlTypes == objectdata[OBJ_TRAPR].oLvlTypes);
	if (lvlMask & objectdata[OBJ_TRAPL].oLvlTypes) {
		AddObjTraps();
	}
	assert(objectdata[OBJ_TCHEST1].oLvlTypes == objectdata[OBJ_TCHEST2].oLvlTypes && objectdata[OBJ_TCHEST1].oLvlTypes == objectdata[OBJ_TCHEST3].oLvlTypes); // trapped chest are placed in one place
	assert((objectdata[OBJ_TCHEST1].oLvlTypes & ~objectdata[OBJ_CHEST1].oLvlTypes) == 0); // no point to place traps if there are not matching chests
	assert((objectdata[OBJ_TCHEST2].oLvlTypes & ~objectdata[OBJ_CHEST2].oLvlTypes) == 0);
	assert((objectdata[OBJ_TCHEST3].oLvlTypes & ~objectdata[OBJ_CHEST3].oLvlTypes) == 0);
	if (lvlMask & objectdata[OBJ_TCHEST1].oLvlTypes) {
		AddChestTraps();
	}
	//gbInitObjFlag = false;
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

static void AddChest(int oi)
{
	ObjectStruct* os;
	int num, rnum, itype;

	os = &objects[oi];
	if (random_(147, 2) == 0)
		os->_oGfxFrame += 3;
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

	while (true) {
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
	os->_oRndSeed = NextRndSeed();
	os->_oVar1 = FindValidShrine(NUM_SHRINETYPE); // SHRINE_TYPE
	/* -- disabled because the animation is shifted and not much point without trap (+ cel file is cut in the patcher)
	if (random_(150, 2) != 0) {
		os->_oAnimFrame = 12;
		os->_oAnimLen = 22;
	}*/
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
	os->_oVar5 = BK_BLOOD;                   // STORY_BOOK_NAME
	os->_oVar6 = os->_oGfxFrame + 1;         // LEVER_BOOK_ANIM
	os->_oVar7 = Q_BLOOD;                    // LEVER_BOOK_QUEST
	SetObjMapRange(oi, 0, 0, 0, 0, leverid); // NULL_LVR_EFFECT
	leverid++;
}

static void ObjAddBook(int oi, int bookidx)
{
	ObjectStruct* os;

	os = &objects[oi];
	os->_oVar5 = bookidx; // STORY_BOOK_NAME
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
	os->_oGfxFrame = RandRange(1, 8);
}

static void AddMagicCircle(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	//os->_oRndSeed = NextRndSeed();
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
	os->_oVar2 = 3 * bookframe + idx + TEXT_BOOK11;      // STORY_BOOK_MSG
	os->_oVar5 = 3 * bookframe + idx + BK_STORY_MAINA_1; // STORY_BOOK_NAME
	os->_oGfxFrame = 5 - 2 * bookframe;                  //
	os->_oVar4 = os->_oGfxFrame + 1;                     // STORY_BOOK_READ_FRAME
}

static void AddTorturedMaleBody(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	//os->_oRndSeed = NextRndSeed();
	os->_oGfxFrame = RandRange(1, 4);
}

static void AddTorturedFemaleBody(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	//os->_oRndSeed = NextRndSeed();
	os->_oGfxFrame = RandRange(1, 3);
}

int AddObject(int type, int ox, int oy)
{
	int oi = SetupObject(type, ox, oy);
	if (oi >= 0) {
		// init object
		switch (type) {
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
		case OBJ_DECAP:
			AddDecap(oi);
			break;
		case OBJ_BOOKCASEL:
		case OBJ_BOOKCASER:
		case OBJ_BARRELEX:
#ifdef HELLFIRE
		case OBJ_URNEX:
		case OBJ_PODEX:
#endif
		case OBJ_BOOK2L:
		case OBJ_BOOK2R:
		case OBJ_PEDESTAL:
		case OBJ_ARMORSTAND:
		case OBJ_WEAPONRACKL:
		case OBJ_WEAPONRACKR:
			ObjAddRndSeed(oi);
			break;
		case OBJ_BLOODBOOK:
			ObjAddBloodBook(oi);
			break;
		case OBJ_ANCIENTBOOK:
			ObjAddBook(oi, BK_ANCIENT);
			break;
		case OBJ_STEELTOME:
			ObjAddBook(oi, BK_STEEL);
			break;
		case OBJ_BLINDBOOK:
			ObjAddBook(oi, BK_BLIND);
			break;
		case OBJ_MYTHICBOOK:
			ObjAddBook(oi, BK_MYTHIC);
			break;
		case OBJ_VILEBOOK:
			ObjAddBook(oi, BK_VILENESS);
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
		//case OBJ_BLOODFTN:
		//case OBJ_TEARFTN:
		//	ObjAddRndSeed(oi);
		//	break;
		case OBJ_MCIRCLE1:
		case OBJ_MCIRCLE2:
			AddMagicCircle(oi);
			break;
		case OBJ_STORYBOOK:
			AddStoryBook(oi);
			break;
		case OBJ_TNUDEM:
			AddTorturedMaleBody(oi);
			break;
		case OBJ_TNUDEW:
			AddTorturedFemaleBody(oi);
			break;
#ifdef HELLFIRE
		case OBJ_NAKRULBOOK:
			AddNakrulBook(oi);
			break;
#endif
		}
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

	tr = lr + 1 + (gsTileVp._vColumns + gsTileVp._vRows / 2) / 2;
	turnon = abs(myview.x - ox) < tr && abs(myview.y - oy) < tr;

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
	static_assert(lengthof(CrawlNum) > 9, "FindClosestPlr uses CrawlTable/CrawlNum up to radius 9.");
	for (i = 0; i <= 9; i++) {
		cr = &CrawlTable[CrawlNum[i]];
		for (j = (BYTE)*cr; j > 0; j--) {
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
			os->_oGfxFrame = 2;
		else {
			//assert(os->_otype == OBJ_MCIRCLE2);
			os->_oGfxFrame = 4;
		}
		if (ox == DBORDERX + 19 && oy == DBORDERY + 20 && os->_oVar5 == 2) { // VILE_CIRCLE_PROGRESS
			if (/*quests[Q_BETRAYER]._qactive == QUEST_ACTIVE &&*/ quests[Q_BETRAYER]._qvar1 < QV_BETRAYER_CENTRALOPEN) {
				quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_CENTRALOPEN;
				// DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, true*/); // LEVER_EFFECT
				DRLG_ChangeMap(7, 11, 13, 18/*, true*/);
			}
			assert(currLvl._dLevelIdx == SL_VILEBETRAYER);
			AddMissile(DBORDERX + 19, DBORDERY + 20, LAZ_CIRCLE_X, LAZ_CIRCLE_Y, 0, MIS_RNDTELEPORT, MST_OBJECT, mypnum, 0);
			gbActionBtnDown = 0;
			// StartTurn(mypnum, DIR_NW); ?
			myplr._pdir = DIR_NW;
			myplr._pAnimData = myplr._pAnims[PGX_STAND].paAnimData[DIR_NW];
			myplr._pDestAction = ACTION_NONE;
		}
	} else {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oGfxFrame = 1;
		else {
			//assert(os->_otype == OBJ_MCIRCLE2);
			os->_oGfxFrame = 3;
		}
	}
}

static void UpdateDoorBlocks(ObjectStruct* os)
{
	int dx, dy;

	dx = os->_ox;
	dy = os->_oy;
	os->_oVar4 = (dMonster[dx][dy] | dItem[dx][dy] | dDead[dx][dy] | dPlayer[dx][dy]) == 0 ? DOOR_OPEN : DOOR_BLOCKED;
}

static void Obj_Door(int oi)
{
	ObjectStruct* os;

	os = &objects[oi];
	if (os->_oVar4 != DOOR_CLOSED) {
		UpdateDoorBlocks(os);
	}
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

static void OpenDoor(ObjectStruct* os)
{
	os->_oVar4 = DOOR_OPEN;
	os->_oPreFlag = TRUE;
	os->_oSelFlag = (objectdata[OBJ_L1LDOOR].oSelFlag & ~1) | 4;
	// TODO: set os->_oSolidFlag = FALSE;
	os->_oMissFlag = TRUE;
	os->_oGfxFrame += 2;
}

static void CloseDoor(ObjectStruct* os)
{
	os->_oVar4 = DOOR_CLOSED;
	os->_oPreFlag = FALSE;
	os->_oSelFlag = objectdata[OBJ_L1LDOOR].oSelFlag;
	// TODO: set os->_oSolidFlag = TRUE;
	os->_oMissFlag = FALSE;
	os->_oGfxFrame -= 2;
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

static void SyncDoors(const ObjectStruct* os);
static void OperateDoor(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// open a closed door
	if (os->_oVar4 == DOOR_CLOSED) {
		if (sendmsg)
			NetSendCmdParam1(CMD_DOOROPEN, oi);
		if (!deltaload) {
			PlaySfxLoc(os->_oSFX, os->_ox, os->_oy);
		}
		OpenDoor(os);
		SyncDoors(os);
		RedoLightAndVision();
		return;
	}
	// try to close the door
	if (!deltaload) {
		UpdateDoorBlocks(os);
		int sfx = IS_DOORCLOS;
#ifdef HELLFIRE
		if (currLvl._dType == DTYPE_CRYPT) {
			sfx = os->_oVar4 == DOOR_BLOCKED ? IS_DOORCLOS : IS_CRCLOS;
		}
#endif
		PlaySfxLoc(sfx, os->_ox, os->_oy);
	}
	if (os->_oVar4 == DOOR_BLOCKED)
		return;
	if (sendmsg)
		NetSendCmdParam1(CMD_DOORCLOSE, oi);
	CloseDoor(os);
	SyncDoors(os);
	RedoLightAndVision();
}

void MonstCheckDoors(int mx, int my)
{
	int i, oi;

	for (i = 0; i < lengthof(offset_x); i++) {
		oi = dObject[mx + offset_x[i]][my + offset_y[i]];
		if (oi == 0)
			continue;
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (objects[oi]._oDoorFlag == ODT_NONE || objects[oi]._oVar4 != DOOR_CLOSED)
			continue;
		// assert(CheckDoor(oi, mnum));
		OperateDoor(oi, true);
	}
}

void ObjChangeMap(int x1, int y1, int x2, int y2/*, bool hasNewObjPiece*/)
{
	int i;
	// const ObjectData* ods;

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
		assert(objectdata[os->_otype].oLightRadius == 0);
		/*ods = &objectdata[os->_otype];
		const BYTE olr = ods->oLightRadius;
		if (olr != 0) {
#if FLICKER_LIGHT
			if (type == OBJ_L1LIGHT) {
				os->_olid = NO_LIGHT;
			} else
#endif
			{
				TraceLightSource(os->_ox + ((olr & OLF_XO) ? 1 : 0), os->_oy + ((olr & OLF_YO) ? 1 : 0), olr & OLF_MASK);
			}
		}*/
	}
	// add new objects (doors + light) -- except when a delta of a single-player game is loaded
	if (!deltaload || IsMultiGame)
		AddDunObjs(x1, y1, x2, y2);
}

static bool CheckLeverGroup(int type, int lvrIdx)
{
	ObjectStruct* os;
	int i;

	for (i = 0; i < numobjects; i++) {
		os = &objects[i]; // objects[objectactive[i]]
		if (os->_otype != type) // OBJ_SWITCHSKL, OBJ_LEVER, OBJ_VILEBOOK
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
	os->_oGfxFrame++; // 2

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
	if (!CheckLeverGroup(os->_otype, os->_oVar8)) // LEVER_INDEX
		return;
	DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
}

static void OperateVileBook(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;
	int dx, dy;

	// assert(currLvl._dLevelIdx == SL_VILEBETRAYER);

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	// assert(plr._pmode == PM_STAND && !deltaload);
	if (plr._px != os->_ox || plr._py != os->_oy + 1)
		return;
	// assert(os->_ox == DBORDERX + 10 || os->_ox == DBORDERX + 29);
	dx = DBORDERX + ((os->_ox == DBORDERX + 10) ? 11 : 27);
	dy = DBORDERY + 13;
	// int on = dObject[dx][dy] - 1;
	// assert(objects[on]._otype == OBJ_MCIRCLE1 || objects[on]._otype == OBJ_MCIRCLE2);

	FindClosestPlr(&dx, &dy);
	AddMissile(os->_ox, os->_oy + 1, dx, dy, 0, MIS_RNDTELEPORT, MST_OBJECT, pnum, 0);
	objects[dObject[DBORDERX + 19][DBORDERY + 20] - 1]._oVar5++; // VILE_CIRCLE_PROGRESS

	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oGfxFrame++; // 5

	DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
	//for (i = 0; i < numobjects; i++)
	//	SyncObjectAnim(objectactive[i]);
}

static void OperateAncientTome(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;

	// assert(currLvl._dLevelIdx == SL_BONECHAMB);

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oGfxFrame++; // 2

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

	if (os->_oGfxFrame != os->_oVar6) { // LEVER_BOOK_ANIM
		os->_oGfxFrame = os->_oVar6;    // LEVER_BOOK_ANIM
		//if (qn != Q_BLOOD) NULL_LVR_EFFECT
			DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4 /*, qn == Q_BLIND*/); // LEVER_EFFECT
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
	os->_oGfxFrame += 2;

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
		PlaySfxLoc(IS_TRAP, os->_ox, os->_oy);
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
	os->_oGfxFrame++; // 2
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
			PlaySfx(sgSFXSets[SFXS_PLR_24][plr._pClass]);
		return;
	}
	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oGfxFrame += 2;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAt(IDI_BANNER, os->_ox, os->_oy, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
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
	int sx = pSetPieces[0]._spx;
	int sy = pSetPieces[0]._spy;

	switch (quests[Q_BLOOD]._qvar1) {
	case QV_INIT:
	case QV_BLOOD_BOOK:
		break;
	case QV_BLOOD_STONE2:
		DRLG_ChangeMap(sx + 6, sy + 3, sx + 9/*setpc_w*/, sy + 7/*, false*/);
		if (!deltaload)
			break;
		/* fall-through */
	case QV_BLOOD_STONE1:
		DRLG_ChangeMap(sx, sy + 3, sx + 2, sy + 7/*, false*/);
		break;
	case QV_BLOOD_STONE3: {
		if (!deltaload)
			DRLG_ChangeMap(sx + 2, sy, sx + 6, sy + 8/*, false*/);
		else
			DRLG_ChangeMap(sx, sy, sx + 9, sy + 8/*, false*/);
		// load the torches TODO: make this more generic (handle OMF_RESERVED in case of torches + always reload lighting)?
		// LoadPreLighting();
#if 1
		if (!deltaload || IsMultiGame) {
		// BYTE lvlMask = 1 << currLvl._dType;
		// assert(objectdata[OBJ_TORCHR1].oLvlTypes & lvlMask);
		// assert(objectdata[OBJ_TORCHR2].oLvlTypes & lvlMask);
		sx = 2 * sx + DBORDERX + 6;
		sy = 2 * sy + DBORDERX + 8;
		AddObject(OBJ_TORCHL1, sx, sy + 0);
		AddObject(OBJ_TORCHL1, sx, sy + 2);
		AddObject(OBJ_TORCHL1, sx, sy + 4);
		sx += 5;
		AddObject(OBJ_TORCHL2, sx, sy + 0);
		AddObject(OBJ_TORCHL2, sx, sy + 2);
		AddObject(OBJ_TORCHL2, sx, sy + 4);
		}
#else
		pSetPieces[0]._spData = LoadFileInMem("Levels\\L2Data\\Blood2.DUN");
		LoadMapSetObjects(0);
		MemFreeDbg(pSetPieces[0]._spData);
#endif
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

	os->_oGfxFrame = quests[Q_BLOOD]._qvar1;
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

DISABLE_SPEED_OPTIMIZATION

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
	os->_oGfxFrame -= 2;

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

static void HelpLvlPlayers(int lvl, unsigned mask, int flags)
{
	static_assert(MAX_PLRS < sizeof(mask) * 8, "HelpLvlPlayers uses unsigned mask for pnum.");

	for (int pnum = 0; mask != 0; pnum++, mask >>= 1) {
		if ((mask & 1) == 0) continue;
		if (!plr._pActive || plr._pDunLevel != lvl || plr._pLvlChanging || plr._pHitPoints == 0)
			continue;
		if (flags & 1)
			PlrFillHp(pnum);
		if (flags & 2)
			PlrFillMana(pnum);
	}
}

void SyncShrineCmd(int pnum, BYTE type, int seed)
{
	ItemStruct* pi;
	int i, cnt, r, lvl;

	SetRndSeed(seed);

	lvl = plr._pDunLevel;

	switch (type) {
	case SHRINE_HIDDEN:
		// SetRndSeed(seed);
		cnt = 0;
		lvl = lvl < NUM_FIXLVLS ? AllLevels[lvl].dLevel : gDynLevels[lvl - NUM_FIXLVLS]._dnLevel;
		pi = plr._pInvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype != ITYPE_NONE
			 && pi->_iMaxDur < DUR_INDESTRUCTIBLE
			 && pi->_iMaxDur > 0
			 && (pi->_iCreateInfo & CF_LEVEL) <= lvl)
				cnt++;
		}
		if (cnt != 0) {
			r = random_low(0, cnt);
			pi = plr._pInvBody;
			for (i = NUM_INVLOC; i != 0; i--, pi++) {
				if (pi->_itype != ITYPE_NONE
				 && pi->_iMaxDur < DUR_INDESTRUCTIBLE
				 && pi->_iMaxDur > 0
				 && (pi->_iCreateInfo & CF_LEVEL) <= lvl) {
					cnt = 1 + pi->_iMaxDur / 32u;
					if (r == 0) {
						pi->_iMaxDur = pi->_iMaxDur > 1 ? pi->_iMaxDur - cnt : pi->_iMaxDur;
						pi->_iDurability = pi->_iDurability > 1 ? pi->_iDurability - cnt : pi->_iDurability;
					} else {
						pi->_iMaxDur = std::min(pi->_iMaxDur + cnt, DUR_INDESTRUCTIBLE - 1);
						pi->_iDurability = std::min(pi->_iDurability + cnt, pi->_iMaxDur);
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
		HelpLvlPlayers(lvl, 1 << pnum, 2);
		break;
	case SHRINE_ELDRITCH:
		// SetRndSeed(seed);
		pi = plr._pInvList;
		for (i = NUM_INV_GRID_ELEM; i > 0; i--, pi++)
			ConvertPotion(pi);
		pi = plr._pSpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			ConvertPotion(pi);
		break;
	case SHRINE_EERIE:
		HelpLvlPlayers(lvl, ((1 << MAX_PLRS) - 1) & ~(1 << pnum), 2);
		break;
	case SHRINE_SPOOKY:
		HelpLvlPlayers(lvl, ((1 << MAX_PLRS) - 1) & ~(1 << pnum), 3);
		break;
	case SHRINE_QUIET:
		HelpLvlPlayers(lvl, ((1 << MAX_PLRS) - 1) & ~(1 << pnum), 1);
		break;
	case SHRINE_DIVINE:
		HelpLvlPlayers(lvl, 1 << pnum, 3);
		break;
	case SHRINE_SACRED:
		AddRaiseSkill(pnum, SPL_CBOLT);
		break;
	case SHRINE_ORNATE:
		AddRaiseSkill(pnum, SPL_HBOLT);
		break;
	case SHRINE_SPIRITUAL:
		// SetRndSeed(seed);
		lvl = lvl < NUM_FIXLVLS ? AllLevels[lvl].dLevel : gDynLevels[lvl - NUM_FIXLVLS]._dnLevel;
		// assert(lvl != 0);
		pi = plr._pInvList;
		for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
			if (pi->_itype == ITYPE_NONE) {
				CreateBaseItem(pi, IDI_GOLD);
				r = (lvl >> 1) + random_low(160, lvl);
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
		AddPlrExperience(pnum, plr._pLevel, 512 * lvl);
		break;
	case SHRINE_MURPHYS:
		// SetRndSeed(seed);
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
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
}
ENABLE_SPEED_OPTIMIZATION

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

	PlaySfxLocN(os->_oSFX, os->_ox, os->_oy, os->_oSFXCnt);
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
	case SHRINE_SECLUDED: {
		if (pnum != mypnum)
			return;
		BYTE* pTmp;
		static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in OperateShrine.");
		pTmp = &dFlags[0][0];
		for (i = 0; i < MAXDUNX * MAXDUNY; i++, pTmp++) {
			*pTmp |= BFLAG_EXPLORED;
		}
		InitDiabloMsg(EMSG_SHRINE_SECLUDED);
	} break;
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
		static_assert(lengthof(CrawlNum) > 3, "OperateShrine uses CrawlTable/CrawlNum radius 3.");
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
	os->_oGfxFrame += 2;

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
	os->_oGfxFrame -= 2;
	if (deltaload)
		return;

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, CFDQ_NORMAL, ITYPE_MISC, IMISC_BOOK, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
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
	os->_oGfxFrame++;
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
	os->_oGfxFrame++;
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

	os->_oGfxFrame = os->_oVar4; // STORY_BOOK_READ_FRAME
	if (deltaload) {
		return;
	}
	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);
	if (pnum == mypnum)
		StartQTextMsg(os->_oVar2); // STORY_BOOK_MSG
}

#ifdef HELLFIRE
void OpenNakrulRoom()
{
	DRLG_ChangeMap(pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 2, pSetPieces[0]._spx + 2, pSetPieces[0]._spy + 3/*, false*/);
}

static void OperateNakrulBook(int pnum, int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	// assert(os->_oSelFlag != 0);
	// assert(currLvl._dLevelIdx == DLV_CRYPT4);

	os->_oGfxFrame = os->_oVar4; // STORY_BOOK_READ_FRAME
	if (deltaload) {
		if (os->_oVar3 == QNB_BOOK_C) { // STORY_BOOK_NAKRUL_IDX
			if (quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKOPEN)
				WakeNakrul();
			if (quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKOPEN /*|| quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_DEADOPEN*/)
				OpenNakrulRoom();
		}
		return;
	}
	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);
	if (pnum == mypnum) {
		StartQTextMsg(os->_oVar2); // STORY_BOOK_MSG
		if (quests[Q_NAKRUL]._qactive != QUEST_DONE) {
			quests[Q_NAKRUL]._qvar2 = ProgressUberLever(os->_oVar3, quests[Q_NAKRUL]._qvar2); // STORY_BOOK_NAKRUL_IDX
			if (quests[Q_NAKRUL]._qvar2 == QV_NAKRUL_BOOKC) {
				NetSendCmd(CMD_OPENNAKRUL);
			}
		}
	}
}

static void OperateNakrulLever(int oi, bool sendmsg)
{
	ObjectStruct* os;

	os = &objects[oi];
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oGfxFrame++; // 2

	if (sendmsg)
		NetSendCmdParam1(CMD_OPERATEOBJ, oi);

	if (!deltaload) {
		if (quests[Q_NAKRUL]._qactive != QUEST_DONE) {
			// assert(quests[Q_NAKRUL]._qvar1 < QV_NAKRUL_BOOKOPEN);
			quests[Q_NAKRUL]._qactive = QUEST_DONE;
			// quests[Q_NAKRUL]._qvar1 = QV_NAKRUL_LEVEROPEN;
			if (sendmsg)
				NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
		}
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
		PlaySfxLoc(IS_CROPEN, os->_ox - 3, os->_oy + 1);
	}
	OpenNakrulRoom();
}
#endif

static void OperateLazStand(int oi, bool sendmsg)
{
	ObjectStruct* os = &objects[oi];

	if (numitems >= MAXITEMS) {
		return;
	}
	// assert(os->_oModeFlags & OMF_ACTIVE);
	os->_oModeFlags &= ~OMF_ACTIVE;
	os->_oSelFlag = 0;
	os->_oGfxFrame++; // 2

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
		DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/);

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

	// assert(os->_oSFXCnt == 1);
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
				AddMissile(xp, yp, 0, 0, 0, MIS_BARRELEX, MST_NA, -1, 0);
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
#ifdef HELLFIRE
	case OBJ_L5LDOOR:
	case OBJ_L5RDOOR:
#endif
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		if (TeleFlag || PlrCheckDoor(oi, pnum))
			OperateDoor(oi, sendmsg);
		break;
	case OBJ_LEVER:
	case OBJ_SWITCHSKL:
		OperateLever(oi, sendmsg);
		break;
	case OBJ_ANCIENTBOOK:
		OperateAncientTome(pnum, oi, sendmsg);
		break;
	case OBJ_VILEBOOK:
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
	case OBJ_MYTHICBOOK:
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
	case OBJ_CAULDRON:
		OperateShrine(pnum, oi, sendmsg);
		break;
	case OBJ_BOOK2R:
	case OBJ_BOOK2L:
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
	case OBJ_BLOODFTN:
	case OBJ_PURIFYINGFTN:
	case OBJ_MURKYFTN:
	case OBJ_TEARFTN:
		OperateFountains(pnum, oi, sendmsg);
		break;
#ifdef HELLFIRE
	case OBJ_NAKRULLEVER:
		OperateNakrulLever(oi, sendmsg);
		break;
	case OBJ_NAKRULBOOK:
		OperateNakrulBook(pnum, oi, sendmsg);
		break;
	case OBJ_L5BOOK:
#endif
	case OBJ_STORYBOOK:
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

/*void SyncTrapOpen(int oi)
{
	if (objects[oi]._oAnimFrame == FLAMETRAP_INACTIVE_FRAME)
		SyncOpObject(oi);
}
void SyncTrapClose(int oi)
{
	if (objects[oi]._oAnimFrame == FLAMETRAP_ACTIVE_FRAME)
		SyncOpObject(oi);
}*/

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
	case OBJ_SWITCHSKL:
		OperateLever(oi, false);
		break;
	case OBJ_ANCIENTBOOK:
		OperateAncientTome(pnum, oi, false);
		break;
	case OBJ_VILEBOOK:
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
	case OBJ_MYTHICBOOK:
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
	case OBJ_CAULDRON:
		OperateShrine(pnum, oi, false);
		break;
	case OBJ_BOOK2R:
	case OBJ_BOOK2L:
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
	// case OBJ_BLOODFTN:
	// case OBJ_PURIFYINGFTN:
	case OBJ_MURKYFTN:
	case OBJ_TEARFTN:
		OperateFountains(pnum, oi, false);
		break;
#ifdef HELLFIRE
	case OBJ_NAKRULLEVER:
		OperateNakrulLever(oi, false);
		break;
	case OBJ_NAKRULBOOK:
		OperateNakrulBook(pnum, oi, false);
		break;
	case OBJ_L5BOOK:
#endif
	case OBJ_STORYBOOK:
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
	case OBJ_SIGNCHEST:
		OperateInnSignChest(pnum, oi, false);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}*/
}

static void SyncLever(const ObjectStruct* os)
{
	if (CheckLeverGroup(os->_otype, os->_oVar8)) // LEVER_INDEX
		DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
}

static void SyncBookLever(const ObjectStruct* os)
{
	if (os->_oGfxFrame == os->_oVar6) { // LEVER_BOOK_ANIM
		DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, os->_otype == OBJ_BLINDBOOK*/); // LEVER_EFFECT
		//if (os->_otype == OBJ_BLINDBOOK) {
			//int tv = dTransVal[2 * os->_oVar1 + DBORDERX + 1][2 * os->_oVar2 + DBORDERY + 1];
			//DRLG_MRectTrans(os->_oVar1 + 2, os->_oVar2 + 2, os->_oVar1 + 4, os->_oVar2 + 4, tv); // LEVER_EFFECT
			//DRLG_MRectTrans(os->_oVar1 + 6, os->_oVar2 + 6, os->_oVar1 + 8, os->_oVar2 + 8, tv); // LEVER_EFFECT
		//}
	}
}

static void SyncCrux(const ObjectStruct* os)
{
	if (CheckCrux(os->_oVar8)) // LEVER_EFFECT
		DRLG_ChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4/*, false*/); // LEVER_EFFECT
}

#ifdef HELLFIRE
static void SyncNakrulLever(const ObjectStruct* os)
{
	if (os->_oSelFlag == 0) {
		OpenNakrulRoom();
	}
}
#endif

static void SyncDoors(const ObjectStruct* os)
{
	int pn;

	pn = os->_oVar1; // DOOR_PIECE_CLOSED
	if (os->_oVar4 != DOOR_CLOSED) {
		// assert(os->_oVar4 == DOOR_OPEN || os->_oVar4 == DOOR_BLOCKED);
		pn--;
	}
	dPiece[os->_ox][os->_oy] = pn;
}

void SyncObjectAnim(int oi)
{
	ObjectStruct* os;
	int type, ofidx;

	os = &objects[oi];
	type = os->_otype;
	ofidx = objectdata[type].ofindex;
	os->_oAnimData = objanimdata[ofidx];
	os->_oAnimFrameLen = objfiledata[ofidx].oAnimFrameLen;
	os->_oAnimWidth = objanimdim[ofidx];
	os->_oAnimXOffset = (os->_oAnimWidth - TILE_WIDTH) >> 1;
	switch (type) {
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
		SyncDoors(os);
		break;
	case OBJ_LEVER:
	case OBJ_VILEBOOK:
	case OBJ_SWITCHSKL:
		SyncLever(os);
		break;
#ifdef HELLFIRE
	case OBJ_NAKRULLEVER:
		SyncNakrulLever(os);
		break;
#endif
	case OBJ_CRUXM:
	//case OBJ_CRUXR: -- check only one of them
	//case OBJ_CRUXL:
		SyncCrux(os);
		break;
	case OBJ_MYTHICBOOK:
	case OBJ_BLINDBOOK:
	//case OBJ_BLOODBOOK: -- NULL_LVR_EFFECT
	case OBJ_STEELTOME:
		SyncBookLever(os);
		break;
	case OBJ_PEDESTAL:
		SyncPedestal(/*os*/);
		break;
	}
}

void GetObjectStr(int oi)
{
	ObjectStruct* os;
	const char* prefix = "";
	const char *txt0, *txt1 = "";

	os = &objects[oi];
	switch (os->_otype) {
	case OBJ_LEVER:
#ifdef HELLFIRE
	case OBJ_NAKRULLEVER:
#endif
	//case OBJ_FLAMELVR:
		txt0 = "Lever";
		break;
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		txt0 = "Small Chest";
		break;
	case OBJ_CHEST2:
	case OBJ_TCHEST2:
		txt0 = "Chest";
		break;
	case OBJ_CHEST3:
	case OBJ_TCHEST3:
	case OBJ_SIGNCHEST:
		txt0 = "Large Chest";
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
			txt0 = "Open";
		else if (os->_oVar4 == DOOR_CLOSED)
			txt0 = "Closed";
		else // if (os->_oVar4 == DOOR_BLOCKED)
			txt0 = "Blocked";
		txt1 = " Door";
		break;
	case OBJ_SWITCHSKL:
		txt0 = "Skull Lever";
		break;
	case OBJ_CRUXM:
	case OBJ_CRUXR:
	case OBJ_CRUXL:
		txt0 = "Crucified Skeleton";
		break;
	case OBJ_SARC:
#ifdef HELLFIRE
	case OBJ_L5SARC:
#endif
		txt0 = "Sarcophagus";
		break;
	//case OBJ_BOOKSHELF:
	//	txt0 = "Bookshelf";
	//	break;
#ifdef HELLFIRE
	case OBJ_URN:
	case OBJ_URNEX:
		txt0 = "Urn";
		break;
	case OBJ_POD:
	case OBJ_PODEX:
		txt0 = "Pod";
		break;
#endif
	case OBJ_BARREL:
	case OBJ_BARRELEX:
		txt0 = "Barrel";
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		txt0 = shrinestrs[os->_oVar1]; // SHRINE_TYPE
		txt1 = " Shrine";
		break;
	case OBJ_BOOKCASEL:
	case OBJ_BOOKCASER:
		txt0 = "Bookcase";
		break;
	case OBJ_BOOK2L:
	case OBJ_BOOK2R:
		txt0 = "Lectern";
		break;
	case OBJ_BLOODFTN:
		txt0 = "Blood Fountain";
		break;
	case OBJ_DECAP:
		txt0 = "Decapitated Body";
		break;
	case OBJ_PEDESTAL:
		txt0 = "Pedestal of Blood";
		break;
	case OBJ_PURIFYINGFTN:
		txt0 = "Purifying Spring";
		break;
	case OBJ_ARMORSTAND:
		txt0 = "Armor";
		break;
	case OBJ_GOATSHRINE:
		txt0 = "Goat Shrine";
		break;
	case OBJ_CAULDRON:
		txt0 = "Cauldron";
		break;
	case OBJ_MURKYFTN:
		txt0 = "Murky Pool";
		break;
	case OBJ_TEARFTN:
		txt0 = "Fountain of Tears";
		break;
	case OBJ_ANCIENTBOOK:
	case OBJ_VILEBOOK:
	case OBJ_MYTHICBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_BLINDBOOK:
	case OBJ_STEELTOME:
	case OBJ_STORYBOOK:
#ifdef HELLFIRE
	case OBJ_L5BOOK:
	case OBJ_NAKRULBOOK:
#endif
		txt0 = BookName[os->_oVar5]; // STORY_BOOK_NAME
		break;
	case OBJ_WEAPONRACKL:
	case OBJ_WEAPONRACKR:
		txt0 = "Weapon Rack";
		break;
	case OBJ_MUSHPATCH:
		txt0 = "Mushroom Patch";
		break;
	case OBJ_LAZSTAND:
		txt0 = "Vile Stand";
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	infoclr = COL_WHITE;
	if (os->_oTrapChance != 0 && (3 * currLvl._dLevel + os->_oTrapChance) < myplr._pBaseDex) { // TRAP_CHANCE
		prefix = "Trapped ";
		infoclr = COL_RED;
	}
	snprintf(infostr, sizeof(infostr), "%s%s%s", prefix, txt0, txt1);
}

DEVILUTION_END_NAMESPACE
