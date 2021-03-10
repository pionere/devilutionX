/**
 * @file objects.cpp
 *
 * Implementation of object functionality, interaction, spawning, loading, etc.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int trapid;
int trapdir;
BYTE *pObjCels[NUM_OFILE_TYPES] = { 0 };
int objectactive[MAXOBJECTS];
/** Specifies the number of active objects. */
int nobjects;
int leverid;
int objectavail[MAXOBJECTS];
ObjectStruct object[MAXOBJECTS];
bool gbInitObjFlag;
int UberLeverProgress;

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
/** Specifies the minimum dungeon level on which each shrine will appear. */
const char shrinemin[NUM_SHRINETYPE] = {
	1, // Hidden
	1, // Gloomy
	1, // Weird
	1, // Religious
	1, // Magical
	1, // Stone
	1, // Creepy
	1, // Thaumaturgic
	1, // Fascinating
	1, // Shimmering
	1, // Cryptic
	1, // Eldritch
	1, // Eerie
	1, // Spooky
	1, // Quiet
	1, // Divine
	1, // Holy
	1, // Sacred
	1, // Ornate
	1, // Spiritual
	1, // Secluded
	1, // Glimmering
	1, // Tainted
	1, // Glistening
#ifdef HELLFIRE
	1, // Sparkling
	1, // Solar,
	1, // Murphy's
#endif
};
/** Specifies the maximum dungeon level on which each shrine will appear. */
const char shrinemax[NUM_SHRINETYPE] = {
	MAX_LVLS, // Hidden
	MAX_LVLS, // Gloomy
	MAX_LVLS, // Weird
	MAX_LVLS, // Religious
	MAX_LVLS, // Magical
	MAX_LVLS, // Stone
	MAX_LVLS, // Creepy
	MAX_LVLS, // Thaumaturgic
	MAX_LVLS, // Fascinating
	MAX_LVLS, // Shimmering
	MAX_LVLS, // Cryptic
	MAX_LVLS, // Eldritch
	MAX_LVLS, // Eerie
	MAX_LVLS, // Spooky
	MAX_LVLS, // Quiet
	MAX_LVLS, // Divine
	MAX_LVLS, // Holy
	MAX_LVLS, // Sacred
	MAX_LVLS, // Ornate
	MAX_LVLS, // Spiritual
	MAX_LVLS, // Secluded
	MAX_LVLS, // Glimmering
	MAX_LVLS, // Tainted
	MAX_LVLS, // Glistening
#ifdef HELLFIRE
	MAX_LVLS, // Sparkling
	MAX_LVLS, // Solar,
	MAX_LVLS, // Murphy's
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
/** Specifies the speech IDs of each dungeon type narrator book, for each player class. */
const int StoryText[3][3] = {
	{ TEXT_BOOK11, TEXT_BOOK12, TEXT_BOOK13 },
	{ TEXT_BOOK21, TEXT_BOOK22, TEXT_BOOK23 },
	{ TEXT_BOOK31, TEXT_BOOK32, TEXT_BOOK33 }
};

const int textSets[NUM_TXTSets][NUM_CLASSES] = {
#ifdef HELLFIRE
	{ TEXT_BLINDING, TEXT_RBLINDING, TEXT_MBLINDING, TEXT_HBLINDING, TEXT_BBLINDING, TEXT_BLINDING },
	{ TEXT_BLOODY,   TEXT_RBLOODY,   TEXT_MBLOODY,   TEXT_HBLOODY,   TEXT_BBLOODY,   TEXT_BLOODY   },
	{ TEXT_BLOODWAR, TEXT_RBLOODWAR, TEXT_MBLOODWAR, TEXT_HBLOODWAR, TEXT_BBLOODWAR, TEXT_BLOODWAR },
	{ TEXT_BONER,    TEXT_RBONER,    TEXT_MBONER,    TEXT_HBONER,    TEXT_BBONER,    TEXT_BONER    },
	{ TEXT_BOOKA,    TEXT_RBOOKA,    TEXT_MBOOKA,    TEXT_OBOOKA,    TEXT_BBOOKA,    TEXT_BOOKA    },
	{ TEXT_BOOKB,    TEXT_RBOOKB,    TEXT_MBOOKB,    TEXT_OBOOKB,    TEXT_BBOOKB,    TEXT_BOOKB    },
	{ TEXT_BOOKC,    TEXT_RBOOKC,    TEXT_MBOOKC,    TEXT_OBOOKC,    TEXT_BBOOKC,    TEXT_BOOKC    },
#else
	{ TEXT_BLINDING, TEXT_RBLINDING, TEXT_MBLINDING },
	{ TEXT_BLOODY,   TEXT_RBLOODY,   TEXT_MBLOODY   },
	{ TEXT_BLOODWAR, TEXT_RBLOODWAR, TEXT_MBLOODWAR },
	{ TEXT_BONER,    TEXT_RBONER,    TEXT_MBONER    },
#endif
};

const int flickers[1][32] = {
	{ 1, 1, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, 0, 0, 1 },
	//{ 0, 0, 0, 0, 0, 0, 1, 1, 1 }
};

void InitObjectGFX()
{
	const ObjDataStruct *ods;
	bool fileload[NUM_OFILE_TYPES];
	char filestr[32];
	int i;

	static_assert(false == 0, "InitObjectGFX fills fileload with 0 instead of false values.");
	memset(fileload, 0, sizeof(fileload));

	int lvl = currlevel;
#ifdef HELLFIRE
	if (lvl >= 21 && lvl <= 24)
		lvl -= 20;
	else if (lvl >= 17 && lvl <= 20)
		lvl -= 8;
#endif
	for (ods = AllObjects; ods->oload != -1; ods++) {
		if ((ods->oload == 1 && lvl >= ods->ominlvl && lvl <= ods->omaxlvl)
		 || (ods->oquest != -1 && QuestStatus(ods->oquest))) {
			fileload[ods->ofindex] = true;
		} else if (ods->otheme != THEME_NONE) {
			for (i = 0; i < numthemes; i++) {
				if (themes[i].ttype == ods->otheme) {
					fileload[ods->ofindex] = true;
					break;
				}
			}
		}
	}

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		if (fileload[i]) {
#ifdef HELLFIRE
			if (currlevel >= 17 && currlevel < 21)
				snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", ObjHiveLoadList[i]);
			else if (currlevel >= 21)
				snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", ObjCryptLoadList[i]);
			else
#endif
				snprintf(filestr, sizeof(filestr), "Objects\\%s.CEL", ObjMasterLoadList[i]);
			pObjCels[i] = LoadFileInMem(filestr, NULL);
		}
	}
}

void FreeObjectGFX()
{
	int i;

	for (i = 0; i < NUM_OFILE_TYPES; i++) {
		if (pObjCels[i] != NULL) {
			MemFreeDbg(pObjCels[i]);
		}
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
	if (leveltype != DTYPE_CATHEDRAL || dPiece[xp][yp] <= 126 || dPiece[xp][yp] >= 144)
		return true;
	return false;
}

static bool WallTrapLocOk(int xp, int yp)
{
	if (dFlags[xp][yp] & BFLAG_POPULATED)
		return false;

	return nTrapTable[dPiece[xp][yp]];
}

static void InitRndLocObj(int min, int max, int objtype)
{
	int i, xp, yp, numobjs;

	numobjs = RandRange(min, max - 1);

	for (i = 0; i < numobjs; i++) {
		while (TRUE) {
			xp = random_(139, DSIZEX) + DBORDERX;
			yp = random_(139, DSIZEY) + DBORDERY;
			if (RndLocOk(xp - 1, yp - 1)
			    && RndLocOk(xp, yp - 1)
			    && RndLocOk(xp + 1, yp - 1)
			    && RndLocOk(xp - 1, yp)
			    && RndLocOk(xp, yp)
			    && RndLocOk(xp + 1, yp)
			    && RndLocOk(xp - 1, yp + 1)
			    && RndLocOk(xp, yp + 1)
			    && RndLocOk(xp + 1, yp + 1)) {
				AddObject(objtype, xp, yp);
				break;
			}
		}
	}
}

static void InitRndLocBigObj(int min, int max, int objtype)
{
	int i, xp, yp, numobjs;

	numobjs = RandRange(min, max - 1);
	for (i = 0; i < numobjs; i++) {
		while (TRUE) {
			xp = random_(140, DSIZEX) + DBORDERX;
			yp = random_(140, DSIZEY) + DBORDERY;
			if (RndLocOk(xp - 1, yp - 2)
			    && RndLocOk(xp, yp - 2)
			    && RndLocOk(xp + 1, yp - 2)
			    && RndLocOk(xp - 1, yp - 1)
			    && RndLocOk(xp, yp - 1)
			    && RndLocOk(xp + 1, yp - 1)
			    && RndLocOk(xp - 1, yp)
			    && RndLocOk(xp, yp)
			    && RndLocOk(xp + 1, yp)
			    && RndLocOk(xp - 1, yp + 1)
			    && RndLocOk(xp, yp + 1)
			    && RndLocOk(xp + 1, yp + 1)) {
				AddObject(objtype, xp, yp);
				break;
			}
		}
	}
}

static void InitRndLocObj5x5(int min, int max, int objtype)
{
	bool exit;
	int xp, yp, numobjs, i, tries, m, n;

	numobjs = RandRange(min, max - 1);
	for (i = 0; i < numobjs; i++) {
		tries = 0;
		exit = false;
		while (!exit) {
			exit = true;
			xp = random_(139, DSIZEX) + DBORDERX;
			yp = random_(139, DSIZEY) + DBORDERY;
			for (n = -2; n <= 2; n++) {
				for (m = -2; m <= 2; m++) {
					if (!RndLocOk(xp + m, yp + n))
						exit = false;
				}
			}
			if (!exit) {
				tries++;
				if (tries > 20000)
					return;
			}
		}
		AddObject(objtype, xp, yp);
	}
}

static void ClrAllObjects()
{
	int i;

	memset(object, 0, sizeof(object));
	memset(objectactive, 0, sizeof(objectactive));

	nobjects = 0;
	for (i = 0; i < MAXOBJECTS; i++)
		objectavail[i] = i;

	trapdir = 0;
	trapid = 1;
	leverid = 1;
}

static void AddTortures()
{
	int ox, oy;

	for (oy = 0; oy < MAXDUNY; oy++) {
		for (ox = 0; ox < MAXDUNX; ox++) {
			if (dPiece[ox][oy] == 367) {
				AddObject(OBJ_TORTURE1, ox, oy + 1);
				AddObject(OBJ_TORTURE3, ox + 2, oy - 1);
				AddObject(OBJ_TORTURE2, ox, oy + 3);
				AddObject(OBJ_TORTURE4, ox + 4, oy - 1);
				AddObject(OBJ_TORTURE5, ox, oy + 5);
				AddObject(OBJ_TNUDEM1, ox + 1, oy + 3);
				AddObject(OBJ_TNUDEM2, ox + 4, oy + 5);
				AddObject(OBJ_TNUDEM3, ox + 2, oy);
				AddObject(OBJ_TNUDEM4, ox + 3, oy + 2);
				AddObject(OBJ_TNUDEW1, ox + 2, oy + 4);
				AddObject(OBJ_TNUDEW2, ox + 2, oy + 1);
				AddObject(OBJ_TNUDEW3, ox + 4, oy + 2);
			}
		}
	}
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

static void AddBookLever(int type, int x, int y, int x1, int y1, int x2, int y2, int msg)
{
	bool exit;
	int xp, yp, oi, tries, m, n;

	tries = 0;
	exit = false;
	while (!exit) {
		exit = true;
		xp = random_(139, DSIZEX) + DBORDERX;
		yp = random_(139, DSIZEY) + DBORDERY;
		for (n = -2; n <= 2; n++) {
			for (m = -2; m <= 2; m++) {
				if (!RndLocOk(xp + m, yp + n))
					exit = false;
			}
		}
		if (!exit) {
			tries++;
			if (tries > 20000)
				return;
		}
	}

	if (x != -1) {
		xp = x;
		yp = y;
	}
	oi = AddObject(type, xp, yp);
	SetObjMapRange(oi, x1, y1, x2, y2, leverid);
	leverid++;
	object[oi]._oVar6 = object[oi]._oAnimFrame + 1;
	object[oi]._oVar7 = msg;
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
	int i, j, oi;
	int xp, yp;
	int rndv;

	if (currlevel == 1)
		rndv = 10;
	if (currlevel >= 2)
		rndv = 15;
	if (currlevel >= 5)
		rndv = 20;
	if (currlevel >= 7)
		rndv = 25;
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			oi = dObject[i][j];
			if (oi <= 0 || random_(144, 100) >= rndv)
				continue;

			oi--;
			if (!AllObjects[object[oi]._otype].oTrapFlag)
				continue;

			if (random_(144, 2) == 0) {
				xp = i - 1;
				while (!nSolidTable[dPiece[xp][j]])
					xp--;

				if (!WallTrapLocOk(xp, j) || i - xp <= 1)
					continue;

				object[oi]._oTrapFlag = TRUE;
				oi = AddObject(OBJ_TRAPL, xp, j);
				object[oi]._oVar1 = i;
				object[oi]._oVar2 = j;
			} else {
				yp = j - 1;
				while (!nSolidTable[dPiece[i][yp]])
					yp--;

				if (!WallTrapLocOk(i, yp) || j - yp <= 1)
					continue;

				object[oi]._oTrapFlag = TRUE;
				oi = AddObject(OBJ_TRAPR, i, yp);
				object[oi]._oVar1 = i;
				object[oi]._oVar2 = j;
			}
		}
	}
}

static void AddChestTraps()
{
	int i, j, r;
	char oi;

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++) {
			oi = dObject[i][j];
			if (oi > 0) {
				oi--;
				if (object[oi]._otype >= OBJ_CHEST1 && object[oi]._otype <= OBJ_CHEST3 && !object[oi]._oTrapFlag && random_(0, 100) < 10) {
					object[oi]._otype += OBJ_TCHEST1 - OBJ_CHEST1;
					object[oi]._oTrapFlag = TRUE;
					if (leveltype == DTYPE_CATACOMBS) {
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
					object[oi]._oVar4 = r;

				}
			}
		}
	}
}

static void LoadMapObjects(BYTE *pMap, int startx, int starty, int x1, int y1, int w, int h, int leveridx)
{
	int rw, rh, i, j, oi, x2, y2;
	BYTE *lm;
	long mapoff;

	gbInitObjFlag = true;

	lm = pMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	mapoff = (rw * rh + 1) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += rw * 2 * rh * 2;
	lm += mapoff;

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
	gbInitObjFlag = false;
}

static void LoadMapObjs(BYTE *pMap, int startx, int starty)
{
	int rw, rh;
	int i, j;
	BYTE *lm;
	long mapoff;

	gbInitObjFlag = true;
	lm = pMap;
	rw = *lm;
	lm += 2;
	rh = *lm;
	mapoff = (rw * rh + 1) * 2;
	rw <<= 1;
	rh <<= 1;
	mapoff += 2 * rw * rh * 2;
	lm += mapoff;

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
	gbInitObjFlag = false;
}

static void AddDiabObjs()
{
	BYTE *lpSetPiece;

	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
	LoadMapObjects(lpSetPiece, 2 * diabquad1x, 2 * diabquad1y, diabquad2x, diabquad2y, 11, 12, 1);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab2a.DUN", NULL);
	LoadMapObjects(lpSetPiece, 2 * diabquad2x, 2 * diabquad2y, diabquad3x, diabquad3y, 11, 11, 2);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab3a.DUN", NULL);
	LoadMapObjects(lpSetPiece, 2 * diabquad3x, 2 * diabquad3y, diabquad4x, diabquad4y, 9, 9, 3);
	mem_free_dbg(lpSetPiece);
}

#ifdef HELLFIRE
static void AddLvl2xBooks(int bookidx)
{
	bool exit;
	int xp, yp, tries, i, j;

	tries = 0;
	exit = false;
	while (!exit) {
		exit = true;
		xp = random_(139, DSIZEX) + DBORDERX;
		yp = random_(139, DSIZEY) + DBORDERY;
		for (j = -2; j <= 2; j++) {
			for (i = -3; i <= 3; i++) {
				if (!RndLocOk(xp + i, yp + j))
					exit = false;
			}
		}
		if (!exit) {
			tries++;
			if (tries > 20000)
				return;
		}
	}

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
	int xp, yp;

	while (TRUE) {
		xp = random_(141, DSIZEX) + DBORDERX;
		yp = random_(141, DSIZEY) + DBORDERY;
		if (RndLocOk(xp - 1, yp - 1)
		    && RndLocOk(xp, yp - 1)
		    && RndLocOk(xp + 1, yp - 1)
		    && RndLocOk(xp - 1, yp)
		    && RndLocOk(xp, yp)
		    && RndLocOk(xp + 1, yp)
		    && RndLocOk(xp - 1, yp + 1)
		    && RndLocOk(xp, yp + 1)
		    && RndLocOk(xp + 1, yp + 1)) {
			break;
		}
	}
	UberLeverRow = UberRow + 3;
	UberLeverCol = UberCol - 1;
	AddObject(OBJ_LEVER, UberLeverRow, UberLeverCol);
}

static void AddLvl24Books()
{
	AddUberLever();
	switch (random_(0, 6)) {
	case 0:
		AddHBooks(5, UberRow + 3, UberCol);
		AddHBooks(6, UberRow + 2, UberCol - 3);
		AddHBooks(7, UberRow + 2, UberCol + 2);
		break;
	case 1:
		AddHBooks(5, UberRow + 3, UberCol);
		AddHBooks(7, UberRow + 2, UberCol - 3);
		AddHBooks(6, UberRow + 2, UberCol + 2);
		break;
	case 2:
		AddHBooks(6, UberRow + 3, UberCol);
		AddHBooks(5, UberRow + 2, UberCol - 3);
		AddHBooks(7, UberRow + 2, UberCol + 2);
		break;
	case 3:
		AddHBooks(6, UberRow + 3, UberCol);
		AddHBooks(7, UberRow + 2, UberCol - 3);
		AddHBooks(5, UberRow + 2, UberCol + 2);
		break;
	case 4:
		AddHBooks(7, UberRow + 3, UberCol);
		AddHBooks(6, UberRow + 2, UberCol - 3);
		AddHBooks(5, UberRow + 2, UberCol + 2);
		break;
	case 5:
		AddHBooks(7, UberRow + 3, UberCol);
		AddHBooks(5, UberRow + 2, UberCol - 3);
		AddHBooks(6, UberRow + 2, UberCol + 2);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static bool ProgressUberLever(int bookidx)
{
	switch (bookidx) {
	case 6:
		UberLeverProgress = 1;
		break;
	case 7:
		if (UberLeverProgress == 1) {
			UberLeverProgress = 2;
		} else {
			UberLeverProgress = 0;
		}
		break;
	case 8:
		if (UberLeverProgress == 2)
			return true;
		UberLeverProgress = 0;
		break;
	}
	return false;
}
#endif

static void AddStoryBooks()
{
	int xp, yp, xx, yy, tries;
	bool done;

	tries = 0;
	done = false;
	while (!done) {
		done = true;
		xp = random_(139, DSIZEX) + DBORDERX;
		yp = random_(139, DSIZEY) + DBORDERY;
		for (yy = -2; yy <= 2; yy++) {
			for (xx = -3; xx <= 3; xx++) {
				if (!RndLocOk(xx + xp, yy + yp))
					done = false;
			}
		}
		if (!done) {
			tries++;
			if (tries > 20000)
				return;
		}
	}

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
	int xp, yp, xx, yy, tries;
	bool found;

	tries = 0;
	found = false;
	while (!found) {
		found = true;
		xp = random_(139, DSIZEX) + DBORDERX;
		yp = random_(139, DSIZEY) + DBORDERY;
		for (yy = -3; yy <= 3; yy++) {
			for (xx = -2; xx <= 3; xx++) {
				if (!RndLocOk(xp + xx, yp + yy))
					found = false;
			}
		}
		if (!found) {
			tries++;
			if (tries > 10000) {
				InitRndLocObj(1, 1, OBJ_LAZSTAND);
				return;
			}
		}
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
	int sp_id;
	BYTE *mem;

	ClrAllObjects();
#ifdef HELLFIRE
	UberLeverProgress = 0;
#endif
	if (currlevel == 16) {
		AddDiabObjs();
	} else {
		gbInitObjFlag = true;
		if (currlevel == 9 && gbMaxPlayers == 1)
			AddSlainHero();
		if (currlevel == quests[Q_MUSHROOM]._qlevel && quests[Q_MUSHROOM]._qactive == QUEST_INIT)
			AddMushPatch();

		if (currlevel == 4 || currlevel == 8 || currlevel == 12) {
			AddStoryBooks();
#ifdef HELLFIRE
		} else if (currlevel == 21) {
			AddLvl2xBooks(0);
		} else if (currlevel == 22) {
			AddLvl2xBooks(1);
			AddLvl2xBooks(2);
		} else if (currlevel == 23) {
			AddLvl2xBooks(3);
			AddLvl2xBooks(4);
		} else if (currlevel == 24) {
			AddLvl24Books();
#endif
		}
		if (leveltype == DTYPE_CATHEDRAL) {
			if (QuestStatus(Q_BUTCHER))
				AddTortures();
			if (QuestStatus(Q_PWATER))
				AddCandles();
			if (QuestStatus(Q_LTBANNER))
				AddObject(OBJ_SIGNCHEST, 2 * setpc_x + DBORDERX + 10, 2 * setpc_y + DBORDERY + 3);
			InitRndLocBigObj(10, 15, OBJ_SARC);
#ifdef HELLFIRE
			if (currlevel >= 21)
				AddCryptObjs(0, 0, MAXDUNX, MAXDUNY);
			else
#endif
				AddL1Objs(0, 0, MAXDUNX, MAXDUNY);
			InitRndBarrels();
		} else if (leveltype == DTYPE_CATACOMBS) {
			if (QuestStatus(Q_ROCK))
				InitRndLocObj5x5(1, 1, OBJ_STAND);
			if (QuestStatus(Q_SCHAMB))
				InitRndLocObj5x5(1, 1, OBJ_BOOK2R);
			AddL2Objs(0, 0, MAXDUNX, MAXDUNY);
			AddL2Torches();
			if (QuestStatus(Q_BLIND)) {
				sp_id = textSets[TXTS_BLINDING][plr[myplr]._pClass];
				quests[Q_BLIND]._qmsg = sp_id;
				AddBookLever(OBJ_BLINDBOOK, -1, 0, setpc_x, setpc_y, setpc_w + setpc_x + 1, setpc_h + setpc_y + 1, sp_id);
				mem = LoadFileInMem("Levels\\L2Data\\Blind2.DUN", NULL);
				LoadMapObjs(mem, 2 * setpc_x, 2 * setpc_y);
				mem_free_dbg(mem);
			}
			if (QuestStatus(Q_BLOOD)) {
				sp_id = textSets[TXTS_BLOODY][plr[myplr]._pClass];
				quests[Q_BLOOD]._qmsg = sp_id;
				AddBookLever(OBJ_BLOODBOOK, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 24, setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7, sp_id);
				AddObject(OBJ_PEDISTAL, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 16);
			}
			InitRndBarrels();
		} else if (leveltype == DTYPE_CAVES) {
			AddL3Objs(0, 0, MAXDUNX, MAXDUNY);
			InitRndBarrels();
		} else if (leveltype == DTYPE_HELL) {
			if (QuestStatus(Q_WARLORD)) {
				sp_id = textSets[TXTS_BLOODWAR][plr[myplr]._pClass];
				quests[Q_WARLORD]._qmsg = sp_id;
				AddBookLever(OBJ_STEELTOME, -1, 0, setpc_x, setpc_y, setpc_x + setpc_w, setpc_y + setpc_h, sp_id);
				mem = LoadFileInMem("Levels\\L4Data\\Warlord.DUN", NULL);
				LoadMapObjs(mem, 2 * setpc_x, 2 * setpc_y);
				mem_free_dbg(mem);
			}
			if (QuestStatus(Q_BETRAYER) && gbMaxPlayers == 1)
				AddLazStand();
			InitRndBarrels();
			AddL4Goodies();
		}
		InitRndLocObj(5, 10, OBJ_CHEST1);
		InitRndLocObj(3, 6, OBJ_CHEST2);
		InitRndLocObj(1, 5, OBJ_CHEST3);
		if (leveltype != DTYPE_HELL)
			AddObjTraps();
		if (leveltype > DTYPE_CATHEDRAL)
			AddChestTraps();
		gbInitObjFlag = false;
	}
}

void SetMapObjects(BYTE *pMap, int startx, int starty)
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
	gbInitObjFlag = true;

	for (i = 0; AllObjects[i].oload != -1; i++) {
		if (AllObjects[i].oload == 1 && leveltype == AllObjects[i].olvltype)
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
		pObjCels[i] = LoadFileInMem(filestr, NULL);
	}

	lm = h;
	startx += DBORDERX;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0)
				AddObject(ObjTypeConv[*lm], i, j);
			lm += 2;
		}
	}
	gbInitObjFlag = false;
}

static void DeleteObject_(int oi, int idx)
{
	objectavail[MAXOBJECTS - nobjects] = oi;
	dObject[object[oi]._ox][object[oi]._oy] = 0;
	nobjects--;
	if (nobjects > 0 && idx != nobjects)
		objectactive[idx] = objectactive[nobjects];
}

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
	if (ods->oAnimFlag) {
		os->_oAnimDelay = ods->oAnimDelay;
		os->_oAnimCnt = random_(146, ods->oAnimDelay);
		os->_oAnimLen = ods->oAnimLen;
		os->_oAnimFrame = RandRange(1, ods->oAnimLen - 1);
	} else {
		os->_oAnimDelay = 1000;
		os->_oAnimCnt = 0;
		os->_oAnimLen = ods->oAnimLen;
		os->_oAnimFrame = ods->oAnimDelay;
	}
	os->_oAnimWidth = ods->oAnimWidth;
	os->_oAnimWidth2 = (os->_oAnimWidth - 64) >> 1;
	os->_oSolidFlag = ods->oSolidFlag;
	os->_oMissFlag = ods->oMissFlag;
	os->_oLight = ods->oLightFlag;
	os->_oDelFlag = FALSE;
	os->_oBreak = ods->oBreak;
	os->_oSelFlag = ods->oSelFlag;
	os->_oPreFlag = FALSE;
	os->_oTrapFlag = FALSE;
	os->_oDoorFlag = FALSE;
}

void SetObjMapRange(int oi, int x1, int y1, int x2, int y2, int v)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = x1;
	os->_oVar2 = y1;
	os->_oVar3 = x2;
	os->_oVar4 = y2;
	os->_oVar8 = v;
}

static void AddL1Door(int oi, int x, int y, int type)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oDoorFlag = TRUE;
	if (type == OBJ_L1LDOOR) {
		os->_oVar1 = dPiece[x][y];
		os->_oVar2 = dPiece[x][y - 1];
	} else {
		os->_oVar1 = dPiece[x][y];
		os->_oVar2 = dPiece[x - 1][y];
	}
	os->_oVar4 = 0;
}

static void AddSCambBook(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = setpc_x;
	os->_oVar2 = setpc_y;
	os->_oVar3 = setpc_w + setpc_x + 1;
	os->_oVar4 = setpc_h + setpc_y + 1;
	os->_oVar6 = os->_oAnimFrame + 1;
}

static void AddChest(int oi, int type)
{
	ObjectStruct *os;

	os = &object[oi];
	if (random_(147, 2) == 0)
		os->_oAnimFrame += 3;
	os->_oRndSeed = GetRndSeed();
	switch (type) {
	case OBJ_CHEST1:
	case OBJ_TCHEST1:
		os->_oVar1 = gbSetlevel ? 1 : random_(147, 2);
		break;
	case OBJ_TCHEST2:
	case OBJ_CHEST2:
		os->_oVar1 = gbSetlevel ? 2 : random_(147, 3);
		break;
	case OBJ_TCHEST3:
	case OBJ_CHEST3:
		os->_oVar1 = gbSetlevel ? 3 : random_(147, 4);
		break;
	}
	os->_oVar2 = random_(147, 8);
}

static void AddL2Door(int oi, int x, int y, int type)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oDoorFlag = TRUE;
	if (type == OBJ_L2LDOOR)
		ObjSetMicro(x, y, 538);
	else
		ObjSetMicro(x, y, 540);
	os->_oVar4 = 0;
}

static void AddL3Door(int oi, int x, int y, int type)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oDoorFlag = TRUE;
	if (type == OBJ_L3LDOOR)
		ObjSetMicro(x, y, 531);
	else
		ObjSetMicro(x, y, 534);
	os->_oVar4 = 0;
}

static void AddSarc(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	dObject[os->_ox][os->_oy - 1] = -(oi + 1);
	os->_oVar1 = random_(153, 10);
	os->_oRndSeed = GetRndSeed();
	if (os->_oVar1 >= 8)
		os->_oVar2 = PreSpawnSkeleton();
}

static void AddFlameTrap(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = trapid;
	os->_oVar2 = 0;
	os->_oVar3 = trapdir;
	os->_oVar4 = 0;
}

static void AddFlameLvr(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = trapid;
	os->_oVar2 = MIS_FLAMEC;
}

static void AddTrap(int oi)
{
	ObjectStruct *os;
	int mt;

	mt = currlevel;
#ifdef HELLFIRE
	if (currlevel >= 17) {
		if (currlevel <= 20)
			mt -= 4;
		else
			mt -= 8;
	}
#endif
	mt = mt / 3 + 1;
	mt = random_(148, mt);
	os = &object[oi];
	if (mt == 0)
		os->_oVar3 = MIS_ARROW;
	if (mt == 1)
		os->_oVar3 = MIS_FIREBOLT;
	if (mt == 2)
		os->_oVar3 = MIS_LIGHTNINGC;
	os->_oVar4 = 0;
}

static void AddObjLight(int oi, int diffr)
{
	ObjectStruct *os;

	os = &object[oi];
	if (gbInitObjFlag) {
		if (diffr != 0)
			DoLighting(os->_ox, os->_oy, diffr, -1);
		os->_olid = -1;
	}
}

static void AddBarrel(int oi, int type)
{
	ObjectStruct *os;

	os = &object[oi];
	//os->_oVar1 = 0;
	os->_oRndSeed = GetRndSeed();
	os->_oVar2 = (type == OBJ_BARRELEX) ? 0 : random_(149, 10);
	os->_oVar3 = random_(149, 3);

	if (os->_oVar2 >= 8)
		os->_oVar4 = PreSpawnSkeleton();
}

static int FindValidShrine(int filter)
{
	int rv;
	BYTE excl = gbMaxPlayers != 1 ? SHRINETYPE_SINGLE : SHRINETYPE_MULTI;

	while (TRUE) {
		rv = random_(0, NUM_SHRINETYPE);
		if (currlevel >= shrinemin[rv] && currlevel <= shrinemax[rv]
		 && rv != filter && shrineavail[rv] != excl)
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
	os->_oVar1 = FindValidShrine(NUM_SHRINETYPE);
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

static void AddBookstand(int oi)
{
	object[oi]._oRndSeed = GetRndSeed();
}

static void AddBloodFtn(int oi)
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
	object[oi]._oRndSeed = GetRndSeed();
}

static void AddArmorStand(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (!gbArmorFlag) {
		os->_oAnimFlag = 2;
		os->_oSelFlag = 0;
	}

	os->_oRndSeed = GetRndSeed();
}

static void AddCauldronGoatShrine(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
	os->_oVar1 = FindValidShrine(SHRINE_THAUMATURGIC);
}

static void AddMurkyFountain(int oi)
{
	int ox, oy;

	ox = object[oi]._ox;
	oy = object[oi]._oy;
	dObject[ox][oy - 1] = -(oi + 1);
	dObject[ox - 1][oy] = -(oi + 1);
	dObject[ox - 1][oy - 1] = -(oi + 1);
	object[oi]._oRndSeed = GetRndSeed();
}

static void AddTearFountain(int oi)
{
	object[oi]._oRndSeed = GetRndSeed();
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
	if (gbSetlevel && setlvlnum == SL_VILEBETRAYER) {
		object[oi]._oAnimFrame = 4;
	}
}

static void AddMagicCircle(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
	os->_oPreFlag = TRUE;
	os->_oVar6 = 0;
	os->_oVar5 = 1;
}

static void AddBrnCross(int oi)
{
	object[oi]._oRndSeed = GetRndSeed();
}

static void AddPedistal(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oVar1 = setpc_x;
	os->_oVar2 = setpc_y;
	os->_oVar3 = setpc_x + setpc_w;
	os->_oVar4 = setpc_y + setpc_h;
	os->_oVar6 = 0;
}

static void AddStoryBook(int oi)
{
	ObjectStruct *os;
	int bookframe;

	SetRndSeed(glSeedTbl[16]);
	bookframe = random_(0, 3);

	os = &object[oi];
	os->_oVar1 = bookframe;
	if (currlevel == 4)
		os->_oVar2 = StoryText[bookframe][0];
	else if (currlevel == 8)
		os->_oVar2 = StoryText[bookframe][1];
	else if (currlevel == 12)
		os->_oVar2 = StoryText[bookframe][2];
	os->_oVar3 = (currlevel >> 2) + 3 * bookframe - 1;
	os->_oAnimFrame = 5 - 2 * bookframe;
	os->_oVar4 = os->_oAnimFrame + 1;
}

static void AddWeaponRack(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (!gbWeaponFlag) {
		os->_oAnimFlag = 2;
		os->_oSelFlag = 0;
	}
	os->_oRndSeed = GetRndSeed();
}

static void AddTorturedBody(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	os->_oRndSeed = GetRndSeed();
	os->_oAnimFrame = RandRange(1, 4);
	os->_oPreFlag = TRUE;
}

static void GetRndObjLoc(int randarea, int *xx, int *yy)
{
	bool failed;
	int i, j, tries;

	assert(randarea > 0);

	tries = 0;
	while (TRUE) {
		tries++;
		if (tries > 1000 && randarea > 1)
			randarea--;
		*xx = random_(0, DSIZEX) + DBORDERX;
		*yy = random_(0, DSIZEY) + DBORDERY;
		failed = false;
		for (i = 0; i < randarea && !failed; i++) {
			for (j = 0; j < randarea && !failed; j++) {
				failed = !RndLocOk(i + *xx, j + *yy);
			}
		}
		if (!failed)
			break;
	}
}

void AddMushPatch()
{
	int oi;
	int y, x;

	if (nobjects < MAXOBJECTS) {
		GetRndObjLoc(5, &x, &y);
		oi = AddObject(OBJ_MUSHPATCH, x + 2, y + 2);
		oi = -(oi + 1);
		dObject[x + 1][y + 1] = oi;
		dObject[x + 2][y + 1] = oi;
		dObject[x + 1][y + 2] = oi;
	}
}

void AddSlainHero()
{
	int x, y;

	GetRndObjLoc(5, &x, &y);
	AddObject(OBJ_SLAINHERO, x + 2, y + 2);
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
	os->_oVar4 = os->_oAnimFrame + 1;
	if (bookidx >= 5) {
		os->_oVar2 = textSets[TXTS_BOOKA + bookidx - 5][plr[myplr]._pClass];
		os->_oVar3 = 15;
		os->_oVar8 = bookidx + 1;
	} else {
		os->_oVar2 = TEXT_BOOK4 + bookidx;
		os->_oVar3 = bookidx + 10;
		os->_oVar8 = 0;
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
		AddL1Door(oi, ox, oy, type);
		break;
	case OBJ_L2LDOOR:
	case OBJ_L2RDOOR:
		AddL2Door(oi, ox, oy, type);
		break;
	case OBJ_L3LDOOR:
	case OBJ_L3RDOOR:
		AddL3Door(oi, ox, oy, type);
		break;
	case OBJ_BOOK2R:
		AddSCambBook(oi);
		break;
	case OBJ_CHEST1:
	case OBJ_CHEST2:
	case OBJ_CHEST3:
		AddChest(oi, type);
		break;
	case OBJ_TCHEST1:
	case OBJ_TCHEST2:
	case OBJ_TCHEST3:
		AddChest(oi, type);
		object[oi]._oTrapFlag = TRUE;
		object[oi]._oVar4 = random_(0, leveltype == DTYPE_CATACOMBS ? 2 : 3);
		break;
	case OBJ_SARC:
		AddSarc(oi);
		break;
	case OBJ_FLAMEHOLE:
		AddFlameTrap(oi);
		break;
	case OBJ_FLAMELVR:
		AddFlameLvr(oi);
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
		AddBarrel(oi, type);
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
		AddBookstand(oi);
		break;
	case OBJ_BLOODFTN:
		AddBloodFtn(oi);
		break;
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
	case OBJ_TEARFTN:
		AddTearFountain(oi);
		break;
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
		AddBrnCross(oi);
		AddObjLight(oi, 10);
		break;
	case OBJ_PEDISTAL:
		AddPedistal(oi);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK:
		AddWeaponRack(oi);
		break;
	case OBJ_TNUDEM2:
		AddTorturedBody(oi);
		break;
	}
	return oi;
}

static void Obj_Light(int oi, int lr, const int *flicker)
{
	ObjectStruct *os;
	int ox, oy, dx, dy, i, tr;
	bool turnon;

	os = &object[oi];
	ox = os->_ox;
	oy = os->_oy;
	tr = lr + 10;
	turnon = false;
#ifdef _DEBUG
	if (!lightflag)
#endif
	{
		for (i = 0; i < MAX_PLRS && !turnon; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				dx = abs(plr[i]._px - ox);
				dy = abs(plr[i]._py - oy);
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
				if (PosOkPlayer(myplr, xx, yy)) {
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
	if (plr[myplr]._px == ox && plr[myplr]._py == oy) {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oAnimFrame = 2;
		else {
			assert(os->_otype == OBJ_MCIRCLE2);
			os->_oAnimFrame = 4;
		}
		if (ox == DBORDERX + 29 && oy == DBORDERY + 31) {
			os->_oVar6 = 2;
		} else if (ox == DBORDERX + 10 && oy == DBORDERY + 30) {
			os->_oVar6 = 1;
		} else {
			os->_oVar6 = 0;
		}
		if (ox == DBORDERX + 19 && oy == DBORDERY + 20 && os->_oVar5 == 3) {
			os->_oVar6 = 4;
			ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
			if (quests[Q_BETRAYER]._qactive == QUEST_ACTIVE && quests[Q_BETRAYER]._qvar1 < 4) // BUGFIX stepping on the circle again will break the quest state (fixed)
				quests[Q_BETRAYER]._qvar1 = 4;
			int dx = 0, dy = 0;
			if (gbSetlevel && setlvlnum == SL_VILEBETRAYER) {
				dx = DBORDERX + 19; dy = DBORDERY + 30;
				GetVileMissPos(&dx, &dy);
			}
			AddMissile(ox, oy, dx, dy, 0, MIS_RNDTELEPORT, -1, myplr, 0, 0, 0);
			gbActionBtnDown = false;
			gbAltActionBtnDown = false;
			ClrPlrPath(myplr);
			PlrStartStand(myplr, 0);
		}
	} else {
		if (os->_otype == OBJ_MCIRCLE1)
			os->_oAnimFrame = 1;
		else {
			assert(os->_otype == OBJ_MCIRCLE2);
			os->_oAnimFrame = 3;
		}
		os->_oVar6 = 0;
	}
}

static void Obj_StopAnim(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oAnimFrame == os->_oAnimLen) {
		os->_oAnimCnt = 0;
		os->_oAnimDelay = 1000;
	}
}

static void Obj_Door(int oi)
{
	ObjectStruct *os;
	int dx, dy;
	bool dok;

	os = &object[oi];
	if (os->_oVar4 == 0) {
		os->_oSelFlag = 3;
		os->_oMissFlag = FALSE;
	} else {
		dx = os->_ox;
		dy = os->_oy;
		dok = (dMonster[dx][dy] | dItem[dx][dy] | dDead[dx][dy] | dPlayer[dx][dy]) == 0;
		os->_oVar4 = dok ? 1 : 2;
		os->_oSelFlag = 2;
		os->_oMissFlag = TRUE;
	}
}

static void Obj_Sarc(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oAnimFrame == os->_oAnimLen)
		os->_oAnimFlag = 0;
}

static void ActivateTrapLine(int ttype, int tid)
{
	ObjectStruct *os;
	int i;

	for (i = 0; i < nobjects; i++) {
		os = &object[objectactive[i]];
		if (os->_otype == ttype && os->_oVar1 == tid) {
			os->_oVar4 = 1;
			os->_oAnimFlag = 1;
			os->_oAnimDelay = 1;
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
	if (os->_oVar2 != 0) {
		if (os->_oVar4 != 0) {
			os->_oAnimFrame--;
			if (os->_oAnimFrame == 1) {
				os->_oVar4 = 0;
				AddUnLight(os->_olid);
			} else if (os->_oAnimFrame <= 4) {
				ChangeLightRadius(os->_olid, os->_oAnimFrame);
			}
		}
	} else if (os->_oVar4 == 0) {
		if (os->_oVar3 == 2) {
			x = os->_ox - 2;
			y = os->_oy;
			for (i = 0; i < 5; i++) {
				if ((dPlayer[x][y] | dMonster[x][y]) != 0)
					os->_oVar4 = 1;
				x++;
			}
		} else {
			x = os->_ox;
			y = os->_oy - 2;
			for (i = 0; i < 5; i++) {
				if ((dPlayer[x][y] | dMonster[x][y]) != 0)
					os->_oVar4 = 1;
				y++;
			}
		}
		if (os->_oVar4 != 0)
			ActivateTrapLine(os->_otype, os->_oVar1);
	} else {
		if (os->_oAnimFrame == os->_oAnimLen)
			os->_oAnimFrame = 11;
		if (os->_oAnimFrame == 11)
			AddMissile(os->_ox, os->_oy, 0, 0, 0, MIS_FIRETRAP, -1, -1, 0, 0, 0);
		else if (os->_oAnimFrame <= 5)
			ChangeLightRadius(os->_olid, os->_oAnimFrame);
	}
}

void Obj_Trap(int oi)
{
	ObjectStruct *os, *on;
	int dir;
	bool otrig;
	int sx, sy, dx, dy, x, y;

	otrig = false;
	os = &object[oi];
	if (os->_oVar4 == 0) {
		on = &object[dObject[os->_oVar1][os->_oVar2] - 1];
		switch (on->_otype) {
		case OBJ_L1LDOOR:
		case OBJ_L1RDOOR:
		case OBJ_L2LDOOR:
		case OBJ_L2RDOOR:
		case OBJ_L3LDOOR:
		case OBJ_L3RDOOR:
			if (on->_oVar4 != 0)
				otrig = true;
			break;
		case OBJ_LEVER:
		case OBJ_CHEST1:
		case OBJ_CHEST2:
		case OBJ_CHEST3:
		case OBJ_SWITCHSKL:
		case OBJ_SARC:
			if (on->_oSelFlag == 0)
				otrig = true;
			break;
		}
		if (otrig) {
			os->_oVar4 = 1;
			if (!deltaload) {
				dx = sx = on->_ox;
				dy = sy = on->_oy;
				for (y = sy - 1; y <= sy + 1; y++) {
					for (x = sx - 1; x <= sx + 1; x++) {
						if (dPlayer[x][y] != 0) {
							dx = x;
							dy = y;
						}
					}
				}
				sx = os->_ox;
				sy = os->_oy;
				dir = GetDirection(sx, sy, dx, dy);
				AddMissile(sx, sy, dx, dy, dir, os->_oVar3, 1, -1, 0, 0, 0);
				PlaySfxLoc(IS_TRAP, on->_ox, on->_oy);
			}
			on->_oTrapFlag = FALSE;
		}
	}
}

static void Obj_BCrossDamage(int oi)
{
	PlayerStruct *p;
	int fire_resist, damage;

	p = &plr[myplr];
	if (p->_pInvincible)
		return;
	if (p->_px != object[oi]._ox || p->_py != object[oi]._oy - 1)
		return;

	damage = 4 + 2 * leveltype;
	fire_resist = p->_pFireResist;
	if (fire_resist > 0)
		damage -= fire_resist * damage / 100;

	if (!PlrDecHp(myplr, damage, 0))
		PlaySfxLoc(sgSFXSets[SFXS_PLR_68][p->_pClass], p->_px, p->_py);
}

void ProcessObjects()
{
	int i, oi;

	for (i = 0; i < nobjects; ++i) {
		oi = objectactive[i];
		switch (object[oi]._otype) {
		case OBJ_L1LIGHT:
			Obj_Light(oi, 8, flickers[0]);
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
		case OBJ_SARC:
			Obj_Sarc(oi);
			break;
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
		if (object[oi]._oAnimFlag == 0)
			continue;

		object[oi]._oAnimCnt++;

		if (object[oi]._oAnimCnt < object[oi]._oAnimDelay)
			continue;

		object[oi]._oAnimCnt = 0;
		object[oi]._oAnimFrame++;
		if (object[oi]._oAnimFrame > object[oi]._oAnimLen)
			object[oi]._oAnimFrame = 1;
	}
	for (i = 0; i < nobjects; ) {
		oi = objectactive[i];
		if (object[oi]._oDelFlag) {
			DeleteObject_(oi, i);
		} else {
			i++;
		}
	}
}

void ObjSetMicro(int dx, int dy, int pn)
{
	WORD *v;
	MICROS *defs;
	int i;

	dPiece[dx][dy] = pn;
	pn--;
	defs = &dpiece_defs_map_2[dx][dy];
	if (leveltype != DTYPE_HELL) {
		v = (WORD *)pLevelPieces + 10 * pn;
		for (i = 0; i < 10; i++) {
			defs->mt[i] = SDL_SwapLE16(v[(i & 1) - (i & 0xE) + 8]);
		}
	} else {
		v = (WORD *)pLevelPieces + 16 * pn;
		for (i = 0; i < 16; i++) {
			defs->mt[i] = SDL_SwapLE16(v[(i & 1) - (i & 0xE) + 14]);
		}
	}
}

static void objects_set_door_piece(int x, int y)
{
	int pn;
	WORD v1, v2;

	pn = dPiece[x][y] - 1;

	v1 = *((WORD *)pLevelPieces + 10 * pn + 8);
	v2 = *((WORD *)pLevelPieces + 10 * pn + 9);
	dpiece_defs_map_2[x][y].mt[0] = SDL_SwapLE16(v1);
	dpiece_defs_map_2[x][y].mt[1] = SDL_SwapLE16(v2);
}

static void ObjSetMini(int x, int y, int v)
{
	int xx, yy;
	long v1, v2, v3, v4;
	WORD *MegaTiles;

	MegaTiles = (WORD *)&pMegaTiles[((WORD)v - 1) * 8];
	v1 = SDL_SwapLE16(*(MegaTiles + 0)) + 1;
	v2 = SDL_SwapLE16(*(MegaTiles + 1)) + 1;
	v3 = SDL_SwapLE16(*(MegaTiles + 2)) + 1;
	v4 = SDL_SwapLE16(*(MegaTiles + 3)) + 1;

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
			if (pn == 13)
				dSpecial[i][j] = 5;
			if (pn == 17)
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
	if (currlevel >= 17) {
		if (pn == 75)
			ObjSetMicro(dx, dy, 204);
		if (pn == 79)
			ObjSetMicro(dx, dy, 208);
		if (pn == 86) {
			if (object[oi]._otype == OBJ_L1LDOOR)
				ObjSetMicro(dx, dy, 232);
			else if (object[oi]._otype == OBJ_L1RDOOR)
				ObjSetMicro(dx, dy, 234);
		}
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
	if (pn == 50) {
		if (object[oi]._otype == OBJ_L1LDOOR)
			ObjSetMicro(dx, dy, 411);
		else if (object[oi]._otype == OBJ_L1RDOOR)
			ObjSetMicro(dx, dy, 412);
	}
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
	int i;

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && currlevel == plr[i].plrlevel) {
			ChangeVisionXY(plr[i]._pvid, plr[i]._px, plr[i]._py);
		}
	}
}

static void OperateL1RDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp, pn;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;
	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) > 1 || abs(yp - y) != 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_OPENDOOR, oi);
#ifdef HELLFIRE
		if (!deltaload)
			PlaySfxLoc(currlevel < 21 ? IS_DOOROPEN : IS_CROPEN, xp, yp);
		ObjSetMicro(xp, yp, currlevel < 21 ? 395 : 209);
		dSpecial[xp][yp] = currlevel < 17 ? 8 : 2;
#else
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		ObjSetMicro(xp, yp, 395);
		dSpecial[xp][yp] = 8;
#endif
		objects_set_door_piece(xp, yp - 1);
		os->_oAnimFrame += 2;
		os->_oPreFlag = TRUE;
		DoorSet(oi, xp - 1, yp);
		os->_oVar4 = 1;
		os->_oSelFlag = 2;
		RedoPlayerVision();
		return;
	}
	// try to close the door
	// check if the door is blocked
	if (os->_oVar4 == 2) {
		if (!deltaload)
			PlaySfxLoc(IS_DOORCLOS, xp, yp);
		return;
	}
	if (!deltaload)
#ifdef HELLFIRE
		PlaySfxLoc(currlevel < 21 ? IS_DOORCLOS : IS_CRCLOS, xp, yp);
#else
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
#endif
	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_CLOSEDOOR, oi);
		os->_oVar4 = 0;
		os->_oSelFlag = 3;
		ObjSetMicro(xp, yp, os->_oVar1);

		pn = os->_oVar2;
#ifdef HELLFIRE
		if (currlevel >= 17) {
			if (pn == 86 && dPiece[xp - 1][yp] == 210)
				pn = 232;
		} else
#endif
		{
			if (pn == 50 && dPiece[xp - 1][yp] == 396)
				pn = 411;
		}
		ObjSetMicro(xp - 1, yp, pn);
		os->_oAnimFrame -= 2;
		os->_oPreFlag = FALSE;
		RedoPlayerVision();
	} else {
		os->_oVar4 = 2;
	}
}

static void OperateL1LDoor(int x, int y, int oi, bool sendmsg)
{
	ObjectStruct *os;
	int xp, yp, pn;

	os = &object[oi];
	xp = os->_ox;
	yp = os->_oy;

	// check if (x;y) is the right position
	if (x != -1) {
		if (abs(xp - x) != 1 || abs(yp - y) > 1)
			return;
	}
	// open a closed door
	if (os->_oVar4 == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_OPENDOOR, oi);
#ifdef HELLFIRE
		if (!deltaload)
			PlaySfxLoc(currlevel < 21 ? IS_DOOROPEN : IS_CROPEN, xp, yp);
		if (currlevel < 21) {
			ObjSetMicro(xp, yp, os->_oVar1 == 214 ? 408 : 393);
		} else {
			ObjSetMicro(xp, yp, 206);
		}
		dSpecial[xp][yp] = currlevel < 17 ? 7 : 1;
#else
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		ObjSetMicro(xp, yp, os->_oVar1 == 214 ? 408 : 393);
		dSpecial[xp][yp] = 7;
#endif
		objects_set_door_piece(xp - 1, yp);
		os->_oAnimFrame += 2;
		os->_oPreFlag = TRUE;
		DoorSet(oi, xp, yp - 1);
		os->_oVar4 = 1;
		os->_oSelFlag = 2;
		RedoPlayerVision();
		return;
	}
	// try to close the door
	// check if the door is blocked
	if (os->_oVar4 == 2) {
		if (!deltaload)
			PlaySfxLoc(IS_DOORCLOS, xp, yp);
		return;
	}
	if (!deltaload)
#ifdef HELLFIRE
		PlaySfxLoc(currlevel < 21 ? IS_DOORCLOS : IS_CRCLOS, xp, yp);
#else
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
#endif
	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_CLOSEDOOR, oi);
		os->_oVar4 = 0;
		os->_oSelFlag = 3;
		ObjSetMicro(xp, yp, os->_oVar1);
		pn = os->_oVar2;
#ifdef HELLFIRE
		if (currlevel >= 17) {
			if (pn == 86 && dPiece[xp][yp - 1] == 210)
				pn = 234;
		} else
#endif
		{
			if (pn == 50 && dPiece[xp][yp - 1] == 396)
				pn = 412;
		}
		ObjSetMicro(xp, yp - 1, pn);
		os->_oAnimFrame -= 2;
		os->_oPreFlag = FALSE;
		RedoPlayerVision();
	} else {
		os->_oVar4 = 2;
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
	if (os->_oVar4 == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_OPENDOOR, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		ObjSetMicro(xp, yp, 17);
		os->_oAnimFrame += 2;
		os->_oPreFlag = TRUE;
		os->_oVar4 = 1;
		os->_oSelFlag = 2;
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	// check if the door is blocked
	if (os->_oVar4 == 2)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_CLOSEDOOR, oi);
		os->_oVar4 = 0;
		os->_oSelFlag = 3;
		ObjSetMicro(xp, yp, 540);
		os->_oAnimFrame -= 2;
		os->_oPreFlag = FALSE;
		RedoPlayerVision();
	} else {
		os->_oVar4 = 2;
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
	if (os->_oVar4 == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_OPENDOOR, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		ObjSetMicro(xp, yp, 13);
		os->_oAnimFrame += 2;
		os->_oPreFlag = TRUE;
		os->_oVar4 = 1;
		os->_oSelFlag = 2;
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	// check if the door is blocked
	if (os->_oVar4 == 2)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_CLOSEDOOR, oi);
		os->_oVar4 = 0;
		os->_oSelFlag = 3;
		ObjSetMicro(xp, yp, 538);
		os->_oAnimFrame -= 2;
		os->_oPreFlag = FALSE;
		RedoPlayerVision();
	} else {
		os->_oVar4 = 2;
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
	if (os->_oVar4 == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_OPENDOOR, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		ObjSetMicro(xp, yp, 541);
		os->_oAnimFrame += 2;
		os->_oPreFlag = TRUE;
		os->_oVar4 = 1;
		os->_oSelFlag = 2;
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	// check if the door is blocked
	if (os->_oVar4 == 2)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_CLOSEDOOR, oi);
		os->_oVar4 = 0;
		os->_oSelFlag = 3;
		ObjSetMicro(xp, yp, 534);
		os->_oAnimFrame -= 2;
		os->_oPreFlag = FALSE;
		RedoPlayerVision();
	} else {
		os->_oVar4 = 2;
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
	if (os->_oVar4 == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_OPENDOOR, oi);
		if (!deltaload)
			PlaySfxLoc(IS_DOOROPEN, xp, yp);
		ObjSetMicro(xp, yp, 538);
		os->_oAnimFrame += 2;
		os->_oPreFlag = TRUE;
		os->_oVar4 = 1;
		os->_oSelFlag = 2;
		RedoPlayerVision();
		return;
	}
	// try to close the door
	if (!deltaload)
		PlaySfxLoc(IS_DOORCLOS, xp, yp);
	// check if the door is blocked
	if (os->_oVar4 == 2)
		return;

	if ((dMonster[xp][yp] | dItem[xp][yp] | dDead[xp][yp]) == 0) {
		if (sendmsg)
			NetSendCmdParam1(true, CMD_CLOSEDOOR, oi);
		os->_oVar4 = 0;
		os->_oSelFlag = 3;
		ObjSetMicro(xp, yp, 531);
		os->_oAnimFrame -= 2;
		os->_oPreFlag = FALSE;
		RedoPlayerVision();
	} else {
		os->_oVar4 = 2;
	}
}

void MonstCheckDoors(int mnum)
{
	int mx, my, i, j, oi;

	mx = monster[mnum]._mx;
	my = monster[mnum]._my;
	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++) {
			oi = dObject[mx + i][my + j];
			if (oi != 0) {
				oi = oi >= 0 ? oi - 1 : -(oi + 1);
				if (object[oi]._oVar4 != 0)
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
#ifdef HELLFIRE
	if (leveltype == DTYPE_CATHEDRAL && currlevel < 17) {
#else
	if (leveltype == DTYPE_CATHEDRAL) {
#endif
		ObjL1Special(x1, y1, x2, y2);
		AddL1Objs(x1, y1, x2, y2);
	} else if (leveltype == DTYPE_CATACOMBS) {
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
#ifdef HELLFIRE
	if (leveltype == DTYPE_CATHEDRAL && currlevel < 17) {
#else
	if (leveltype == DTYPE_CATHEDRAL) {
#endif
		ObjL1Special(x1, y1, x2, y2);
	} else if (leveltype == DTYPE_CATACOMBS) {
		ObjL2Special(x1, y1, x2, y2);
	}
}

static void OperateLever(int oi, bool sendmsg)
{
	ObjectStruct *os, *on;
	int i;
	bool mapflag;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;

	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
	mapflag = true;
	if (currlevel == 16) {
		for (i = 0; i < nobjects; i++) {
			on = &object[objectactive[i]];
			if (on->_otype == OBJ_SWITCHSKL && os->_oVar8 == on->_oVar8 && on->_oSelFlag != 0) {
				mapflag = false;
				break;
			}
		}
	}
#ifdef HELLFIRE
	if (currlevel == 24) {
		DoOpenUberRoom();
		gbUberLeverActivated = true;
		mapflag = false;
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		//quests[Q_NAKRUL]._qlog = FALSE;
	}
#endif
	if (mapflag)
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateVileBook(int pnum, int oi)
{
	ObjectStruct *os, *on;
	int i;
	int dx, dy;
	bool missile_added;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	if (gbSetlevel && setlvlnum == SL_VILEBETRAYER) {
		missile_added = false;
		for (i = 0; i < nobjects; i++) {
			on = &object[objectactive[i]];
			if (on->_otype != OBJ_MCIRCLE2)
				continue;
			if (on->_oVar6 == 1) {
				dx = DBORDERX + 11;
				dy = DBORDERY + 13;
			} else if (on->_oVar6 == 2) {
				dx = DBORDERX + 27;
				dy = DBORDERY + 13;
			} else {
				continue;
			}
			on->_oVar6 = 4;
			object[dObject[DBORDERX + 19][DBORDERY + 20] - 1]._oVar5++;
			GetVileMissPos(&dx, &dy);
			AddMissile(plr[pnum]._px, plr[pnum]._py, dx, dy, 0, MIS_RNDTELEPORT, -1, pnum, 0, 0, 0);
			missile_added = true;
		}
		if (!missile_added)
			return;
	}
	os->_oSelFlag = 0;
	os->_oAnimFrame++;
	if (!gbSetlevel)
		return;

	if (setlvlnum == SL_BONECHAMB) {
		if (plr[pnum]._pSkillLvl[SPL_GUARDIAN] == 0) {
			plr[pnum]._pSkillLvl[SPL_GUARDIAN] = 1;
			plr[pnum]._pSkillExp[SPL_GUARDIAN] = SkillExpLvlsTbl[0];
			plr[pnum]._pMemSkills |= SPELL_MASK(SPL_GUARDIAN);
		}
		quests[Q_SCHAMB]._qactive = QUEST_DONE;
		if (!deltaload)
			PlaySfxLoc(IS_QUESTDN, os->_ox, os->_oy);
		InitDiabloMsg(EMSG_BONECHAMB);
		AddMissile(
		    plr[pnum]._px,
		    plr[pnum]._py,
		    os->_ox - 2,
		    os->_oy - 4,
		    plr[pnum]._pdir,
		    MIS_GUARDIAN,
		    0,
		    pnum,
		    0,
		    0,
		    0);
	} else if (setlvlnum == SL_VILEBETRAYER) {
		ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);
	}
}

static void OperateBookLever(int oi, bool sendmsg)
{
	ObjectStruct *os;
	int tren;

	if (numitems >= MAXITEMS) {
		return;
	}
	os = &object[oi];
	if (os->_oSelFlag != 0 && !gbQtextflag) {
		if (os->_otype == OBJ_BLINDBOOK && quests[Q_BLIND]._qvar1 == 0) {
			quests[Q_BLIND]._qactive = QUEST_ACTIVE;
			quests[Q_BLIND]._qlog = TRUE;
			quests[Q_BLIND]._qvar1 = 1;
		}
		if (os->_otype == OBJ_BLOODBOOK && quests[Q_BLOOD]._qvar1 == 0) {
			quests[Q_BLOOD]._qactive = QUEST_ACTIVE;
			quests[Q_BLOOD]._qlog = TRUE;
			quests[Q_BLOOD]._qvar1 = 1;
			SpawnQuestItemAt(IDI_BLDSTONE, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 17);
		}
		if (os->_otype == OBJ_STEELTOME && quests[Q_WARLORD]._qvar1 == 0) {
			quests[Q_WARLORD]._qactive = QUEST_ACTIVE;
			quests[Q_WARLORD]._qlog = TRUE;
			quests[Q_WARLORD]._qvar1 = 1;
		}
		if (os->_oAnimFrame != os->_oVar6) {
			if (os->_otype != OBJ_BLOODBOOK)
				ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
			if (os->_otype == OBJ_BLINDBOOK) {
				SpawnUnique(UITEM_OPTAMULET, 2 * setpc_x + DBORDERX + 5, 2 * setpc_y + DBORDERY + 5);
				tren = TransVal;
				TransVal = 9;
				DRLG_MRectTrans(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
				TransVal = tren;
			}
		}
		os->_oAnimFrame = os->_oVar6;
		InitQTextMsg(os->_oVar7);
		if (sendmsg)
			NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
	}
}

static void OperateSChambBk(int pnum, int oi)
{
	ObjectStruct *os;
	int i, textdef;

	os = &object[oi];
	if (os->_oSelFlag != 0 && !gbQtextflag) {
		if (os->_oAnimFrame != os->_oVar6) {
			ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
			for (i = 0; i < nobjects; i++)
				SyncObjectAnim(objectactive[i]);
			os->_oAnimFrame = os->_oVar6;
		}
		if (quests[Q_SCHAMB]._qactive == QUEST_INIT) {
			quests[Q_SCHAMB]._qactive = QUEST_ACTIVE;
			quests[Q_SCHAMB]._qlog = TRUE;
		}
		textdef = textSets[TXTS_BONER][plr[myplr]._pClass];
		quests[Q_SCHAMB]._qmsg = textdef;
		InitQTextMsg(textdef);
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

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	if (gbSetlevel) {
		for (i = os->_oVar1; i > 0; i--) {
			CreateRndItem(os->_ox, os->_oy, true, sendmsg, false);
		}
	} else {
		for (i = os->_oVar1; i > 0; i--) {
			if (os->_oVar2 != 0)
				CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
			else
				CreateRndUseful(os->_ox, os->_oy, sendmsg, false);
		}
	}
	if (os->_oTrapFlag && os->_otype >= OBJ_TCHEST1 && os->_otype <= OBJ_TCHEST3) {
		mdir = GetDirection(os->_ox, os->_oy, plr[pnum]._px, plr[pnum]._py);
		AddMissile(os->_ox, os->_oy, plr[pnum]._px, plr[pnum]._py, mdir, os->_oVar4, 1, -1, 0, 0, 0);
		os->_oTrapFlag = FALSE;
	}
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateMushPatch(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	if (numitems >= MAXITEMS) {
		return;
	}

	if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 < QS_TOMEGIVEN) {
		if (!deltaload && pnum == myplr) {
			PlaySFX(sgSFXSets[SFXS_PLR_13][plr[myplr]._pClass]);
		}
		return;
	}
	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;
	if (deltaload)
		return;

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAround(IDI_MUSHROOM, os->_ox, os->_oy, sendmsg);
	quests[Q_MUSHROOM]._qvar1 = QS_MUSHSPAWNED;
}

static void OperateInnSignChest(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;

	if (numitems >= MAXITEMS) {
		return;
	}

	if (quests[Q_LTBANNER]._qvar1 != 2) {
		if (!deltaload && pnum == myplr) {
			PlaySFX(sgSFXSets[SFXS_PLR_24][plr[pnum]._pClass]);
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

	PlaySfxLoc(IS_CHEST, os->_ox, os->_oy);
	SpawnQuestItemAround(IDI_BANNER, os->_ox, os->_oy, sendmsg);
}

static void OperateSlainHero(int pnum, int oi, bool sendmsg)
{
	ObjectStruct *os;
	char pc;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;

	if (deltaload)
		return;

	pc = plr[pnum]._pClass;
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
	PlaySfxLoc(sgSFXSets[SFXS_PLR_09][pc], plr[pnum]._px, plr[pnum]._py);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateTrapLever(int oi)
{
	ObjectStruct *os, *on;
	int frame, i;

	os = &object[oi];
	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);

	frame = os->_oAnimFrame;
	if (frame == 1) {
		os->_oAnimFrame = 2;
		for (i = 0; i < nobjects; i++) {
			on = &object[objectactive[i]];
			if (on->_otype == os->_oVar2 && on->_oVar1 == os->_oVar1) {
				on->_oVar2 = 1;
				on->_oAnimFlag = 0;
			}
		}
		return;
	}

	os->_oAnimFrame = frame - 1;
	for (i = 0; i < nobjects; i++) {
		on = &object[objectactive[i]];
		if (on->_otype == os->_oVar2 && on->_oVar1 == os->_oVar1) {
			on->_oVar2 = 0;
			if (on->_oVar4 != 0)
				on->_oAnimFlag = 1;
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

	PlaySfxLoc(IS_SARC, os->_ox, os->_oy);

	os->_oAnimFlag = 1;
	os->_oAnimDelay = 3;
	SetRndSeed(os->_oRndSeed);
	if (os->_oVar1 <= 2)
		CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
	if (os->_oVar1 >= 8)
		SpawnSkeleton(os->_oVar2, os->_ox, os->_oy);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperatePedistal(int pnum, int oi)
{
	ObjectStruct *os;
	BYTE *mem;
	int iv;

	if (numitems >= MAXITEMS) {
		return;
	}
	os = &object[oi];
	if (os->_oVar6 != 3 && PlrHasItem(pnum, IDI_BLDSTONE, &iv)) {
		RemoveInvItem(pnum, iv);
		os->_oAnimFrame++;
		os->_oVar6++;

		if (os->_oVar6 == 1) {
			if (!deltaload)
				PlaySfxLoc(LS_PUDDLE, os->_ox, os->_oy);
			ObjChangeMap(setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7);
			SpawnQuestItemAt(IDI_BLDSTONE, 2 * setpc_x + DBORDERX + 3, 2 * setpc_y + DBORDERY + 10);
		}
		if (os->_oVar6 == 2) {
			if (!deltaload)
				PlaySfxLoc(LS_PUDDLE, os->_ox, os->_oy);
			ObjChangeMap(setpc_x + 6, setpc_y + 3, setpc_x + setpc_w, setpc_y + 7);
			SpawnQuestItemAt(IDI_BLDSTONE, 2 * setpc_x + DBORDERX + 15, 2 * setpc_y + DBORDERY + 10);
		}
		if (os->_oVar6 == 3) {
			if (!deltaload)
				PlaySfxLoc(LS_BLODSTAR, os->_ox, os->_oy);
			ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
			mem = LoadFileInMem("Levels\\L2Data\\Blood2.DUN", NULL);
			LoadMapObjs(mem, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(mem);
			SpawnUnique(UITEM_ARMOFVAL, 2 * setpc_x + DBORDERX + 9, 2 * setpc_y + DBORDERY + 3);
			os->_oSelFlag = 0;
		}
	}
}

void DisarmObject(int pnum, int oi)
{
	ObjectStruct *os, *on;
	int i, trapdisper;

	if (pnum == myplr)
		NewCursor(CURSOR_HAND);
	os = &object[oi];
	if (os->_oTrapFlag) {
		trapdisper = 2 * plr[pnum]._pDexterity - 5 * currlevel;
		if (random_(154, 100) <= trapdisper) {
			for (i = 0; i < nobjects; i++) {
				on = &object[objectactive[i]];
				if ((on->_otype == OBJ_TRAPL || on->_otype == OBJ_TRAPR)
				 && dObject[on->_oVar1][on->_oVar2] - 1 == oi) {
					on->_oVar4 = 1;
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

/** Reduce the maximum mana of the given player by 10%
*/
static void ReducePlrMana10(PlayerStruct *p)
{
	int v1, v2;
	DWORD t;

	t = p->_pMaxManaBase / 10;
	v1 = p->_pMana - p->_pManaBase;
	v2 = p->_pMaxMana - p->_pMaxManaBase;

	p->_pManaBase -= t;
	p->_pMana -= t;
	p->_pMaxMana -= t;
	p->_pMaxManaBase -= t;
	if (p->_pMana >> 6 <= 0) {
		p->_pManaBase = 0;
		p->_pMana = v1;
	}
	if (p->_pMaxMana >> 6 <= 0) {
		p->_pMaxManaBase = 0;
		p->_pMaxMana = v2;
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

static void AddRaiseSkill(PlayerStruct *p, int sn)
{
	p->_pMemSkills |= SPELL_MASK(sn);
	if (p->_pSkillLvl[sn] < MAXSPLLEVEL) {
		p->_pSkillExp[sn] = SkillExpLvlsTbl[p->_pSkillLvl[sn]];
		p->_pSkillLvl[sn]++;
	}
}

static void OperateShrine(int pnum, int psfx, int psfxCnt, int oi, bool sendmsg)
{
	ObjectStruct *os;
	PlayerStruct *p;
	ItemStruct *pi;
	int i, r, cnt;
	int xx, yy;

	assert((DWORD)oi < MAXOBJECTS);

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;

	SetRndSeed(os->_oRndSeed);
	os->_oSelFlag = 0;

	if (!deltaload) {
		PlaySfxLoc(psfx, os->_ox, os->_oy, psfxCnt);
		os->_oAnimFlag = 1;
		os->_oAnimDelay = 1;
	} else {
		os->_oAnimFlag = 0;
		os->_oAnimFrame = os->_oAnimLen;
	}
	p = &plr[pnum];
	switch (os->_oVar1) {
	case SHRINE_HIDDEN:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		cnt = 0;
		pi = p->InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype != ITYPE_NONE
			 && pi->_iMaxDur != DUR_INDESTRUCTIBLE
			 && pi->_iMaxDur != 0)
				cnt++;
		}
		if (cnt != 0) {
			r = random_(0, cnt);
			pi = p->InvBody;
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
		InitDiabloMsg(EMSG_SHRINE_HIDDEN);
		break;
	case SHRINE_GLOOMY:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;

		pi = &p->InvBody[INVLOC_HAND_LEFT];
		pi->_iDurability = pi->_iMaxDur;

		pi = &p->InvBody[INVLOC_HAND_RIGHT];
		if (pi->_iClass == ICLASS_WEAPON)
			pi->_iDurability = pi->_iMaxDur;

		InitDiabloMsg(EMSG_SHRINE_GLOOMY);
		break;
	case SHRINE_WEIRD:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;

		pi = &p->InvBody[INVLOC_HEAD];
		pi->_iDurability = pi->_iMaxDur;
		
		pi = &p->InvBody[INVLOC_CHEST];
		pi->_iDurability = std::max(1, pi->_iDurability >> 1);

		InitDiabloMsg(EMSG_SHRINE_WEIRD);
		break;
	case SHRINE_RELIGIOUS:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;

		pi = p->InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur;
		pi = p->InvList;
		for (i = p->_pNumInv; i > 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur;
		pi = p->SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			pi->_iDurability = pi->_iMaxDur; // belt items don't have durability?
		InitDiabloMsg(EMSG_SHRINE_RELIGIOUS);
		break;
	case SHRINE_MAGICAL:
		if (deltaload)
			return;
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
		    2 * leveltype);
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_MAGICAL);
		break;
	case SHRINE_STONE:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;

		pi = p->InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges;
		pi = p->InvList;
		for (i = p->_pNumInv; i > 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges;
		pi = p->SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			pi->_iCharges = pi->_iMaxCharges; // belt items don't have charges?
		InitDiabloMsg(EMSG_SHRINE_STONE);
		break;
	case SHRINE_CREEPY:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;

		pi = &p->InvBody[INVLOC_HAND_LEFT];
		pi->_iCharges = pi->_iMaxCharges;

		InitDiabloMsg(EMSG_SHRINE_CREEPY);
		break;
	case SHRINE_THAUMATURGIC:
		for (i = 0; i < nobjects; i++) {
			os = &object[objectactive[i]];
			if ((os->_otype == OBJ_CHEST1
			        || os->_otype == OBJ_CHEST2
			        || os->_otype == OBJ_CHEST3)
			    && os->_oSelFlag == 0) {
				os->_oRndSeed = GetRndSeed();
				os->_oAnimFrame -= 2;
				os->_oSelFlag = 1;
			}
		}
		if (deltaload)
			return;
		if (pnum == myplr)
			InitDiabloMsg(EMSG_SHRINE_THAUMATURGIC);
		break;
	case SHRINE_FASCINATING:
		if (deltaload)
			return;

		AddRaiseSkill(p, SPL_FIREBOLT);
		ReducePlrMana10(p);

		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_FASCINATING);
		break;
	case SHRINE_SHIMMERING:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		PlrFillMana(pnum);
		InitDiabloMsg(EMSG_SHRINE_SHIMMERING);
		break;
	case SHRINE_CRYPTIC:
		if (deltaload)
			return;
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
		if (pnum != myplr)
			return;
		PlrFillMana(pnum);
		InitDiabloMsg(EMSG_SHRINE_CRYPTIC);
		break;
	case SHRINE_ELDRITCH:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		pi = p->InvList;
		for (i = p->_pNumInv; i > 0; i--, pi++)
			ConvertPotion(pi);
		pi = p->SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			ConvertPotion(pi);
		InitDiabloMsg(EMSG_SHRINE_ELDRITCH);
		break;
	case SHRINE_EERIE:
		if (deltaload)
			return;
		if (pnum == myplr) {
			InitDiabloMsg(EMSG_SHRINE_EERIE1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_EERIE2);

			PlrFillMana(myplr);
		}
		break;
	case SHRINE_SPOOKY:
		if (deltaload)
			return;
		if (pnum == myplr) {
			InitDiabloMsg(EMSG_SHRINE_SPOOKY1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_SPOOKY2);
			PlrFillHp(myplr);
			PlrFillMana(myplr);
		}
		break;
	case SHRINE_QUIET:
		if (deltaload)
			return;
		if (pnum == myplr) {
			InitDiabloMsg(EMSG_SHRINE_QUIET1);
		} else {
			InitDiabloMsg(EMSG_SHRINE_QUIET2);

			PlrFillHp(myplr);
		}
		break;
	case SHRINE_DIVINE:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		if (currlevel <= 3) {
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLMANA, sendmsg, false);
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLHEAL, sendmsg, false);
		} else {
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLREJUV, sendmsg, false);
			CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_FULLREJUV, sendmsg, false);
		}
		PlrFillHp(pnum);
		PlrFillMana(pnum);
		InitDiabloMsg(EMSG_SHRINE_DIVINE);
		break;
	case SHRINE_HOLY:
		if (deltaload)
			return;
		AddMissile(p->_px, p->_py, 0, 0, 0, MIS_RNDTELEPORT, -1, pnum, 0, 0, 0);
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_HOLY);
		break;
	case SHRINE_SACRED:
		if (deltaload)
			return;
		AddRaiseSkill(p, SPL_CBOLT);
		ReducePlrMana10(p);

		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_SACRED);
		break;
	case SHRINE_ORNATE:
		if (deltaload)
			return;
		AddRaiseSkill(p, SPL_HBOLT);
		ReducePlrMana10(p);

		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_ORNATE);
		break;
	case SHRINE_SPIRITUAL:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
			if (p->InvGrid[i] == 0) {
				r = 5 * leveltype + random_(160, 10 * leveltype);
				pi = &p->InvList[p->_pNumInv]; // check
				copy_pod(*pi, golditem);
				pi->_iSeed = GetRndSeed();
				p->_pNumInv++;
				p->InvGrid[i] = p->_pNumInv;
				SetGoldItemValue(pi, r);
				p->_pGold += r;
			}
		}
		InitDiabloMsg(EMSG_SHRINE_SPIRITUAL);
		break;
	case SHRINE_SECLUDED:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;

		for (yy = 0; yy < DMAXY; yy++) {
			for (xx = 0; xx < DMAXX; xx++) {
				automapview[xx][yy] = TRUE;
			}
		}
		InitDiabloMsg(EMSG_SHRINE_SECLUDED);
		break;
	case SHRINE_GLIMMERING:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		pi = p->InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE;
		pi = p->InvList;
		for (i = p->_pNumInv; i > 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE;
		pi = p->SpdList;
		for (i = MAXBELTITEMS; i != 0; i--, pi++)
			if (pi->_iMagical != ITEM_QUALITY_NORMAL)
				pi->_iIdentified = TRUE; // belt items can't be magical?
		InitDiabloMsg(EMSG_SHRINE_GLIMMERING);
		break;
	case SHRINE_TAINTED:
		if (deltaload)
			return;
		if (MINION_NR_INACTIVE(myplr)) {
			AddMissile(
				plr[myplr]._px,
				plr[myplr]._py,
				plr[myplr]._px,
				plr[myplr]._py,
				0,
				MIS_GOLEM,
				0,
				myplr,
				0,
				0,
				currlevel);
		}
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_TAINTED);
		break;
	case SHRINE_GLISTENING:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_GLISTENING);
		AddMissile(
		    os->_ox,
		    os->_oy,
		    p->_px,
		    p->_py,
		    p->_pdir,
		    MIS_TOWN,
		    1,
		    pnum,
		    0,
		    0,
		    0);
		break;
#ifdef HELLFIRE
	case SHRINE_SPARKLING:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_SPARKLING);
		AddPlrExperience(myplr, p->_pLevel, 1000 * currlevel);
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
		break;
	case SHRINE_SOLAR: {
		if (deltaload)
			return;

		static_assert(MIS_RUNEFIRE + 1 == MIS_RUNELIGHT, "SHRINE_SOLAR expects runes in a given order I.");
		static_assert(MIS_RUNEFIRE + 2 == MIS_RUNENOVA, "SHRINE_SOLAR expects runes in a given order II.");
		static_assert(MIS_RUNEFIRE + 3 == MIS_RUNEIMMOLAT, "SHRINE_SOLAR expects runes in a given order III.");
		const char *cr = &CrawlTable[CrawlNum[3]];
		for (i = (BYTE)*cr; i > 0; i--) {
			xx = plr[pnum]._px + *++cr;
			yy = plr[pnum]._py + *++cr;
			if (!ItemSpaceOk(xx, yy))
				continue;
			if (random_(0, 3) == 0)
				AddMissile(xx, yy, xx, yy, 0, MIS_RUNEFIRE + random_(0, 4), -1, -1, 0, 0, currlevel);
			else
				CreateTypeItem(xx, yy, false, ITYPE_MISC, IMISC_RUNE, sendmsg, false);
		}
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_SOLAR);
	} break;
	case SHRINE_MURPHYS:
		if (deltaload)
			return;
		if (pnum != myplr)
			return;
		InitDiabloMsg(EMSG_SHRINE_MURPHYS);
		pi = p->InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype == ITYPE_NONE || random_(0, 3) != 0)
				continue;
			r = pi->_iDurability;
			if (r != DUR_INDESTRUCTIBLE && r != 0) {
				r /= 2;
				if (r == 0) {
					NetSendCmdDelItem(true, i);
					pi->_itype = ITYPE_NONE;
				}
				else
					pi->_iDurability = r;
				break;
			}
		}
		if (i == 0) {
			TakePlrsMoney(p->_pGold / 3);
		}
		break;
#endif
	default:
		ASSUME_UNREACHABLE
	}

	CalcPlrInv(pnum, true);
	gbRedrawFlags = REDRAW_ALL;

	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
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

	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC,
		random_(161, 5) != 0 ? IMISC_SCROLL : IMISC_BOOK, sendmsg, false);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateBookCase(int oi, bool sendmsg)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame -= 2;
	if (deltaload)
		return;
	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy, false, ITYPE_MISC, IMISC_BOOK, sendmsg, false);
	if (QuestStatus(Q_ZHAR)
	 && (monster[MAX_MINIONS]._uniqtype - 1) == UMT_ZHAR
	 && monster[MAX_MINIONS]._msquelch == UCHAR_MAX
	 && monster[MAX_MINIONS]._mhitpoints != 0) {
		monster[MAX_MINIONS].mtalkmsg = TEXT_ZHAR2;
		MonStartStand(0, monster[MAX_MINIONS]._mdir);
		monster[MAX_MINIONS]._mgoal = MGOAL_ATTACK2;
		monster[MAX_MINIONS]._mmode = MM_TALK;
	}
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
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

	SetRndSeed(os->_oRndSeed);
	CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateArmorStand(int oi, bool sendmsg)
{
	ObjectStruct *os;
	int itype;
	bool uniqueRnd, onlygood;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;
	os->_oSelFlag = 0;
	os->_oAnimFrame++;

	if (deltaload)
		return;

	SetRndSeed(os->_oRndSeed);
	uniqueRnd = random_(0, 2);
	if (currlevel <= 5) {
		itype = ITYPE_LARMOR;
		onlygood = true;
	} else if (currlevel <= 9) {
		itype = ITYPE_MARMOR;
		onlygood = uniqueRnd;
	} else if (currlevel <= 12) {
		itype = ITYPE_HARMOR;
		onlygood = false;
	} else if (currlevel <= 16) {
		itype = ITYPE_HARMOR;
		onlygood = true;
#ifdef HELLFIRE
	} else {
		itype = ITYPE_HARMOR;
		onlygood = true;
#endif
	}
	CreateTypeItem(os->_ox, os->_oy, onlygood, itype, IMISC_NONE, sendmsg, false);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateGoatShrine(int pnum, int oi, bool sendmsg)
{
	OperateShrine(pnum, LS_GSHRINE, 1, oi, sendmsg);
	object[oi]._oAnimDelay = 2;
}

static void OperateCauldron(int pnum, int oi, bool sendmsg)
{
	OperateShrine(pnum, LS_CALDRON, 1, oi, sendmsg);
	object[oi]._oAnimFrame = 3;
	object[oi]._oAnimFlag = 0;
}

static void OperateFountains(int pnum, int oi)
{
	PlayerStruct *p;
	ObjectStruct *os;

	os = &object[oi];
	// SetRndSeed(os->_oRndSeed);
	switch (os->_otype) {
	case OBJ_BLOODFTN:
		if (deltaload)
			return;
		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		if (pnum != myplr)
			return;
		PlrIncHp(pnum, 64);
		break;
	case OBJ_PURIFYINGFTN:
		if (deltaload)
			return;
		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		if (pnum != myplr)
			return;
		PlrIncMana(pnum, 64);
		break;
	case OBJ_MURKYFTN:
		if (os->_oSelFlag == 0)
			break;
		os->_oSelFlag = 0;
		if (deltaload)
			return;

		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		p = &plr[pnum];
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
		    2 * leveltype);
		if (pnum == myplr)
			NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
		break;
	case OBJ_TEARFTN:
		if (deltaload)
			return;

		PlaySfxLoc(LS_FOUNTAIN, os->_ox, os->_oy);

		if (pnum != myplr)
			return;
		PlrIncHp(pnum, 64);
		if (plr[pnum]._pMana >= 64)
			PlrDecMana(pnum, 64);
		break;
	}
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

	SetRndSeed(os->_oRndSeed);
	CreateTypeItem(os->_ox, os->_oy,
		leveltype > DTYPE_CATHEDRAL,
		ITYPE_SWORD + random_(0, 4),
		IMISC_NONE, sendmsg, false);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateStoryBook(int pnum, int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		return;

	if (deltaload)
		return;
	if (gbQtextflag || pnum != myplr)
		return;
	os->_oAnimFrame = os->_oVar4;
	PlaySfxLoc(IS_ISCROL, os->_ox, os->_oy);
#ifdef HELLFIRE
	if (os->_oVar8 != 0 && currlevel == 24) {
		if (!gbUberLeverActivated && quests[Q_NAKRUL]._qactive != QUEST_DONE && ProgressUberLever(os->_oVar8)) {
			NetSendCmd(false, CMD_NAKRUL);
			return;
		}
	} else if (currlevel >= 21) {
		quests[Q_NAKRUL]._qactive = QUEST_ACTIVE;
		quests[Q_NAKRUL]._qlog = TRUE;
		quests[Q_NAKRUL]._qmsg = os->_oVar2;
	}
#endif
	InitQTextMsg(os->_oVar2);
	NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

static void OperateLazStand(int oi, bool sendmsg)
{
	ObjectStruct *os = &object[oi];

	if (numitems >= MAXITEMS) {
		return;
	}
	if (os->_oSelFlag == 0)
		return;
	if (deltaload)
		return;
	os->_oAnimFrame++;
	os->_oSelFlag = 0;
	SpawnQuestItemAround(IDI_LAZSTAFF, os->_ox, os->_oy, sendmsg);
	if (sendmsg)
		NetSendCmdParam1(false, CMD_OPERATEOBJ, oi);
}

void OperateObject(int pnum, int oi, bool TeleFlag)
{
	bool sendmsg;

	sendmsg = (pnum == myplr);
	switch (object[oi]._otype) {
	case OBJ_L1LDOOR:
		if (TeleFlag)
			OperateL1LDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == myplr
			OperateL1LDoor(plr[pnum]._px, plr[pnum]._py, oi, sendmsg);
		break;
	case OBJ_L1RDOOR:
		if (TeleFlag)
			OperateL1RDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == myplr
			OperateL1RDoor(plr[pnum]._px, plr[pnum]._py, oi, sendmsg);
		break;
	case OBJ_L2LDOOR:
		if (TeleFlag)
			OperateL2LDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == myplr
			OperateL2LDoor(plr[pnum]._px, plr[pnum]._py, oi, sendmsg);
		break;
	case OBJ_L2RDOOR:
		if (TeleFlag)
			OperateL2RDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == myplr
			OperateL2RDoor(plr[pnum]._px, plr[pnum]._py, oi, sendmsg);
		break;
	case OBJ_L3LDOOR:
		if (TeleFlag)
			OperateL3LDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == myplr
			OperateL3LDoor(plr[pnum]._px, plr[pnum]._py, oi, sendmsg);
		break;
	case OBJ_L3RDOOR:
		if (TeleFlag)
			OperateL3RDoor(-1, -1, oi, sendmsg);
		else //if (sendmsg) // pnum == myplr
			OperateL3RDoor(plr[pnum]._px, plr[pnum]._py, oi, sendmsg);
		break;
	case OBJ_LEVER:
	case OBJ_SWITCHSKL:
		OperateLever(oi, sendmsg);
		break;
	case OBJ_BOOK2L:
		OperateVileBook(pnum, oi);
		break;
	case OBJ_BOOK2R:
		OperateSChambBk(pnum, oi);
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
		OperateTrapLever(oi);
		break;
	case OBJ_BLINDBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_STEELTOME:
		OperateBookLever(oi, sendmsg);
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
		OperateBookCase(oi, sendmsg);
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
		OperateFountains(pnum, oi);
		break;
	case OBJ_STORYBOOK:
		OperateStoryBook(pnum, oi);
		break;
	case OBJ_PEDISTAL:
		OperatePedistal(pnum, oi);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK:
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
	}
}

void SyncOpenDoor(int oi)
{
	if (object[oi]._oVar4 == 0)
		SyncOpObject(-1, oi);
}
void SyncCloseDoor(int oi)
{
	if (object[oi]._oVar4 == 1)
		SyncOpObject(-1, oi);
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
	case OBJ_BLINDBOOK:
	case OBJ_BLOODBOOK:
	case OBJ_STEELTOME:
		OperateBookLever(oi, false);
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
		OperateBookCase(oi, false);
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
		OperateStoryBook(pnum, oi);
		break;
	case OBJ_PEDISTAL:
		OperatePedistal(pnum, oi);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK:
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
	}
}

static void BreakCrux(int oi)
{
	ObjectStruct *os, *on;
	int i;

	os = &object[oi];
	os->_oAnimFlag = 1;
	os->_oAnimFrame = 1;
	os->_oAnimDelay = 1;
	os->_oSolidFlag = TRUE;
	os->_oMissFlag = TRUE;
	os->_oBreak = -1;
	os->_oSelFlag = 0;
	for (i = 0; i < nobjects; i++) {
		on = &object[objectactive[i]];
		if (on->_otype != OBJ_CRUX1 && on->_otype != OBJ_CRUX2 && on->_otype != OBJ_CRUX3)
			continue;
		if (os->_oVar8 != on->_oVar8 || on->_oBreak == -1)
			continue;
		return;
	}
	if (!deltaload)
		PlaySfxLoc(IS_LEVER, os->_ox, os->_oy);
	ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
}

static void BreakBarrel(int pnum, int oi, bool forcebreak, bool sendmsg)
{
	ObjectStruct *os = &object[oi];
	int mpo;
	int xp, yp;

	if (os->_oSelFlag == 0)
		return;
	if (!forcebreak) {
		if (pnum != myplr) {
			if (deltaload)
				return;

			PlaySfxLoc(IS_IBOW, os->_ox, os->_oy);
			return;
		}
		/*if (pnum != -1) {
			dam = PlrAtkDam(pnum) >> 6;
		} else {
			dam = 10;
		}
		os->_oVar1 -= dam;
		if (pnum != myplr && os->_oVar1 <= 0)
			os->_oVar1 = 1;
		if (os->_oVar1 > 0) {
			if (deltaload)
				return;

			PlaySfxLoc(IS_IBOW, os->_ox, os->_oy);
			return;
		}*/
	}

	os->_oVar1 = 0;
	os->_oAnimFlag = 1;
	os->_oAnimFrame = 1;
	os->_oAnimDelay = 1;
	os->_oSolidFlag = FALSE;
	os->_oMissFlag = TRUE;
	os->_oBreak = -1;
	os->_oSelFlag = 0;
	os->_oPreFlag = TRUE;
	if (deltaload) {
		os->_oAnimFrame = os->_oAnimLen;
		os->_oAnimCnt = 0;
		os->_oAnimDelay = 1000;
		return;
	}

	if (os->_otype == OBJ_BARRELEX) {
#ifdef HELLFIRE
		if (currlevel >= 21 && currlevel <= 24)
			PlaySfxLoc(IS_POPPOP3, os->_ox, os->_oy);
		else if (currlevel >= 17 && currlevel <= 20)
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
						BreakBarrel(pnum, mpo, true, sendmsg);
				}
			}
		}
	} else {
#ifdef HELLFIRE
		if (currlevel >= 21 && currlevel <= 24)
			PlaySfxLoc(IS_POPPOP2, os->_ox, os->_oy);
		else if (currlevel >= 17 && currlevel <= 20)
			PlaySfxLoc(IS_POPPOP5, os->_ox, os->_oy);
		else
#endif
			PlaySfxLoc(IS_BARREL, os->_ox, os->_oy);
		SetRndSeed(os->_oRndSeed);
		if (os->_oVar2 <= 1) {
			if (os->_oVar3 == 0)
				CreateRndUseful(os->_ox, os->_oy, sendmsg, false);
			else
				CreateRndItem(os->_ox, os->_oy, false, sendmsg, false);
		} else if (os->_oVar2 >= 8)
			SpawnSkeleton(os->_oVar4, os->_ox, os->_oy);
	}
	if (sendmsg)
		NetSendCmdParam2(false, CMD_BREAKOBJ, pnum, oi);
}

void BreakObject(int pnum, int oi)
{
	switch (object[oi]._otype) {
	case OBJ_CRUX1:
	case OBJ_CRUX2:
	case OBJ_CRUX3:
		BreakCrux(oi);
		break;
	case OBJ_BARREL:
	case OBJ_BARRELEX:
		BreakBarrel(pnum, oi, false, true);
		break;
	}
}

void SyncBreakObj(int pnum, int oi)
{
	if (object[oi]._otype >= OBJ_BARREL && object[oi]._otype <= OBJ_BARRELEX)
		BreakBarrel(pnum, oi, true, false);
}

static void SyncL1Doors(int oi)
{
	ObjectStruct *os;
	int x, y;

	os = &object[oi];
	if (os->_oVar4 == 0) {
		os->_oMissFlag = FALSE;
		return;
	}

	x = os->_ox;
	y = os->_oy;
	os->_oMissFlag = TRUE;
	os->_oSelFlag = 2;
#ifdef HELLFIRE
	if (currlevel >= 17) {
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
			if (os->_oVar1 == 214)
				ObjSetMicro(x, y, 408);
			else
				ObjSetMicro(x, y, 393);
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

static void SyncCrux(int oi)
{
	ObjectStruct *os, *on;
	int i;

	os = &object[oi];
	for (i = 0; i < nobjects; i++) {
		on = &object[objectactive[i]];
		if (on->_otype != OBJ_CRUX1 && on->_otype != OBJ_CRUX2 && on->_otype != OBJ_CRUX3)
			continue;
		if (os->_oVar8 != on->_oVar8 || on->_oBreak == -1)
			continue;
		return;
	}
	ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
}

static void SyncLever(int oi)
{
	ObjectStruct *os;

	os = &object[oi];
	if (os->_oSelFlag == 0)
		ObjChangeMap(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
}

static void SyncQSTLever(int oi)
{
	ObjectStruct *os;
	int tren;

	os = &object[oi];
	if (os->_oAnimFrame == os->_oVar6) {
		ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
		if (os->_otype == OBJ_BLINDBOOK) {
			tren = TransVal;
			TransVal = 9;
			DRLG_MRectTrans(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
			TransVal = tren;
		}
	}
}

static void SyncPedistal(int oi)
{
	ObjectStruct *os;
	BYTE *setp;

	os = &object[oi];
	if (os->_oVar6 == 1)
		ObjChangeMapResync(setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7);
	else if (os->_oVar6 == 2) {
		ObjChangeMapResync(setpc_x, setpc_y + 3, setpc_x + 2, setpc_y + 7);
		ObjChangeMapResync(setpc_x + 6, setpc_y + 3, setpc_x + setpc_w, setpc_y + 7);
	} else if (os->_oVar6 == 3) {
		ObjChangeMapResync(os->_oVar1, os->_oVar2, os->_oVar3, os->_oVar4);
		setp = LoadFileInMem("Levels\\L2Data\\Blood2.DUN", NULL);
		LoadMapObjs(setp, 2 * setpc_x, 2 * setpc_y);
		mem_free_dbg(setp);
	}
}

static void SyncL2Doors(int oi)
{
	ObjectStruct *os;
	int x, y;

	os = &object[oi];
	os->_oMissFlag = os->_oVar4 != 0;
	x = os->_ox;
	y = os->_oy;
	os->_oSelFlag = 2;
	if (os->_otype == OBJ_L2LDOOR) {
		if (os->_oVar4 == 0)
			ObjSetMicro(x, y, 538);
		else if (os->_oVar4 == 1 || os->_oVar4 == 2)
			ObjSetMicro(x, y, 13);
	} else if (os->_otype == OBJ_L2RDOOR) {
		if (os->_oVar4 == 0)
			ObjSetMicro(x, y, 540);
		else if (os->_oVar4 == 1 || os->_oVar4 == 2)
			ObjSetMicro(x, y, 17);
	}
}

static void SyncL3Doors(int oi)
{
	ObjectStruct *os;
	int x, y;

	os = &object[oi];
	os->_oMissFlag = TRUE;
	x = os->_ox;
	y = os->_oy;
	os->_oSelFlag = 2;
	if (os->_otype == OBJ_L3LDOOR) {
		if (os->_oVar4 == 0)
			ObjSetMicro(x, y, 531);
		else if (os->_oVar4 == 1 || os->_oVar4 == 2)
			ObjSetMicro(x, y, 538);
	} else if (os->_otype == OBJ_L3RDOOR) {
		if (os->_oVar4 == 0)
			ObjSetMicro(x, y, 534);
		else if (os->_oVar4 == 1 || os->_oVar4 == 2)
			ObjSetMicro(x, y, 541);
	}
}

void SyncObjectAnim(int oi)
{
	int type;

	type = object[oi]._otype;
	object[oi]._oAnimData = pObjCels[AllObjects[type].ofindex];
	switch (object[oi]._otype) {
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
	case OBJ_CRUX1:
	case OBJ_CRUX2:
	case OBJ_CRUX3:
		SyncCrux(oi);
		break;
	case OBJ_LEVER:
	case OBJ_BOOK2L:
	case OBJ_SWITCHSKL:
		SyncLever(oi);
		break;
	case OBJ_BOOK2R:
	case OBJ_BLINDBOOK:
	case OBJ_STEELTOME:
		SyncQSTLever(oi);
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
		if (os->_oVar4 == 1)
			copy_cstr(infostr, "Open Door");
		else if (os->_oVar4 == 0)
			copy_cstr(infostr, "Closed Door");
		else if (os->_oVar4 == 2)
			copy_cstr(infostr, "Blocked Door");
		break;
	case OBJ_BOOK2L:
		if (gbSetlevel) {
			if (setlvlnum == SL_BONECHAMB)
				copy_cstr(infostr, "Ancient Tome");
			else if (setlvlnum == SL_VILEBETRAYER)
				copy_cstr(infostr, "Book of Vileness");
		}
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
		if (currlevel >= 17 && currlevel <= 20)      // for hive levels
			copy_cstr(infostr, "Pod");               //Then a barrel is called a pod
		else if (currlevel >= 21 && currlevel <= 24) // for crypt levels
			copy_cstr(infostr, "Urn");               //Then a barrel is called an urn
		else
#endif
			copy_cstr(infostr, "Barrel");
		break;
	case OBJ_SKELBOOK:
		copy_cstr(infostr, "Skeleton Tome");
		break;
	case OBJ_SHRINEL:
	case OBJ_SHRINER:
		snprintf(infostr, sizeof(infostr), "%s Shrine", shrinestrs[os->_oVar1]);
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
		copy_cstr(infostr, StoryBookName[os->_oVar3]);
		break;
	case OBJ_WARWEAP:
	case OBJ_WEAPONRACK:
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
	}
	infoclr = COL_WHITE;
	if (plr[myplr]._pClass == PC_ROGUE) {
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
	dPiece[UberRow][UberCol] = 298;
	dPiece[UberRow][UberCol - 1] = 301;
	dPiece[UberRow][UberCol - 2] = 300;
	dPiece[UberRow][UberCol + 1] = 299;

	SetDungeonMicros(UberRow, UberCol - 1, UberRow + 1, UberCol + 2);
}

void DoOpenUberRoom()
{
	if (currlevel == 24) {
		PlaySfxLoc(IS_CROPEN, UberRow, UberCol);
		OpenUberRoom();
	}
}
#endif

DEVILUTION_END_NAMESPACE
