/**
 * @file items.cpp
 *
 * Implementation of item functionality.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int itemactive[MAXITEMS];
BOOL uitemflag;
int itemavail[MAXITEMS];
ItemStruct curruitem;
ItemGetRecordStruct itemrecord[MAXITEMS];
/** Contains the items on ground in the current game. */
ItemStruct item[MAXITEMS + 1];
BOOL itemhold[3][3];
#ifdef HELLFIRE
CornerStoneStruct CornerStone;
int auricGold = 2 * GOLD_MAX_LIMIT;
int MaxGold = GOLD_MAX_LIMIT;
#endif
BYTE *itemanims[ITEMTYPES];
BOOL UniqueItemFlag[128];
int numitems;
int gnNumGetRecords;

/* data */

#ifdef HELLFIRE
int OilLevels[] = { 1, 10, 1, 10, 4, 1, 5, 17, 1, 10 };
int OilValues[] = { 500, 2500, 500, 2500, 1500, 100, 2500, 15000, 500, 2500 };
int OilMagic[] = {
	IMISC_OILACC,
	IMISC_OILMAST,
	IMISC_OILSHARP,
	IMISC_OILDEATH,
	IMISC_OILSKILL,
	IMISC_OILBSMTH,
	IMISC_OILFORT,
	IMISC_OILPERM,
	IMISC_OILHARD,
	IMISC_OILIMP,
};
char OilNames[10][25] = {
	"Oil of Accuracy",
	"Oil of Mastery",
	"Oil of Sharpness",
	"Oil of Death",
	"Oil of Skill",
	"Blacksmith Oil",
	"Oil of Fortitude",
	"Oil of Permanence",
	"Oil of Hardening",
	"Oil of Imperviousness"
};
#endif

/** Maps from item_cursor_graphic to in-memory item type. */
BYTE ItemCAnimTbl[] = {
#ifdef HELLFIRE
	20, 16, 16, 16, 4, 4, 4, 12, 12, 12,
	12, 12, 12, 12, 12, 21, 21, 25, 12, 28,
	28, 28, 38, 38, 38, 32, 38, 38, 38, 24,
	24, 26, 2, 25, 22, 23, 24, 25, 27, 27,
	29, 0, 0, 0, 12, 12, 12, 12, 12, 0,
	8, 8, 0, 8, 8, 8, 8, 8, 8, 6,
	8, 8, 8, 6, 8, 8, 6, 8, 8, 6,
	6, 6, 8, 8, 8, 5, 9, 13, 13, 13,
	5, 5, 5, 15, 5, 5, 18, 18, 18, 30,
	5, 5, 14, 5, 14, 13, 16, 18, 5, 5,
	7, 1, 3, 17, 1, 15, 10, 14, 3, 11,
	8, 0, 1, 7, 0, 7, 15, 7, 3, 3,
	3, 6, 6, 11, 11, 11, 31, 14, 14, 14,
	6, 6, 7, 3, 8, 14, 0, 14, 14, 0,
	33, 1, 1, 1, 1, 1, 7, 7, 7, 14,
	14, 17, 17, 17, 0, 34, 1, 0, 3, 17,
	8, 8, 6, 1, 3, 3, 11, 3, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 35, 39, 36,
	36, 36, 37, 38, 38, 38, 38, 38, 41, 42,
	8, 8, 8, 17, 0, 6, 8, 11, 11, 3,
	3, 1, 6, 6, 6, 1, 8, 6, 11, 3,
	6, 8, 1, 6, 6, 17, 40, 0, 0
#else
	20, 16, 16, 16, 4, 4, 4, 12, 12, 12,
	12, 12, 12, 12, 12, 21, 21, 25, 12, 28,
	28, 28, 0, 0, 0, 32, 0, 0, 0, 24,
	24, 26, 2, 25, 22, 23, 24, 25, 27, 27,
	29, 0, 0, 0, 12, 12, 12, 12, 12, 0,
	8, 8, 0, 8, 8, 8, 8, 8, 8, 6,
	8, 8, 8, 6, 8, 8, 6, 8, 8, 6,
	6, 6, 8, 8, 8, 5, 9, 13, 13, 13,
	5, 5, 5, 15, 5, 5, 18, 18, 18, 30,
	5, 5, 14, 5, 14, 13, 16, 18, 5, 5,
	7, 1, 3, 17, 1, 15, 10, 14, 3, 11,
	8, 0, 1, 7, 0, 7, 15, 7, 3, 3,
	3, 6, 6, 11, 11, 11, 31, 14, 14, 14,
	6, 6, 7, 3, 8, 14, 0, 14, 14, 0,
	33, 1, 1, 1, 1, 1, 7, 7, 7, 14,
	14, 17, 17, 17, 0, 34, 1, 0, 3, 17,
	8, 8, 6, 1, 3, 3, 11, 3, 4
#endif
};
/** Map of item type .cel file names. */
const char *const ItemDropNames[] = {
	"Armor2",
	"Axe",
	"FBttle",
	"Bow",
	"GoldFlip",
	"Helmut",
	"Mace",
	"Shield",
	"SwrdFlip",
	"Rock",
	"Cleaver",
	"Staff",
	"Ring",
	"CrownF",
	"LArmor",
	"WShield",
	"Scroll",
	"FPlateAr",
	"FBook",
	"Food",
	"FBttleBB",
	"FBttleDY",
	"FBttleOR",
	"FBttleBR",
	"FBttleBL",
	"FBttleBY",
	"FBttleWH",
	"FBttleDB",
	"FEar",
	"FBrain",
	"FMush",
	"Innsign",
	"Bldstn",
	"Fanvil",
	"FLazStaf",
#ifdef HELLFIRE
	"bombs1",
	"halfps1",
	"wholeps1",
	"runes1",
	"teddys1",
	"cows1",
	"donkys1",
	"mooses1",
#endif
};
/** Maps of item drop animation length. */
BYTE ItemAnimLs[] = {
	15,
	13,
	16,
	13,
	10,
	13,
	13,
	13,
	13,
	10,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	13,
	1,
	16,
	16,
	16,
	16,
	16,
	16,
	16,
	16,
	13,
	12,
	12,
	13,
	13,
	13,
	8,
#ifdef HELLFIRE
	10,
	16,
	16,
	10,
	10,
	15,
	15,
	15,
#endif
};
/** Maps of drop sounds effect of dropping the item on ground. */
int ItemDropSnds[] = {
	IS_FHARM,
	IS_FAXE,
	IS_FPOT,
	IS_FBOW,
	IS_GOLD,
	IS_FCAP,
	IS_FSWOR,
	IS_FSHLD,
	IS_FSWOR,
	IS_FROCK,
	IS_FAXE,
	IS_FSTAF,
	IS_FRING,
	IS_FCAP,
	IS_FLARM,
	IS_FSHLD,
	IS_FSCRL,
	IS_FHARM,
	IS_FBOOK,
	IS_FLARM,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FPOT,
	IS_FBODY,
	IS_FBODY,
	IS_FMUSH,
	IS_ISIGN,
	IS_FBLST,
	IS_FANVL,
	IS_FSTAF,
#ifdef HELLFIRE
	IS_FROCK,
	IS_FSCRL,
	IS_FSCRL,
	IS_FROCK,
	IS_FMUSH,
	IS_FHARM,
	IS_FLARM,
	IS_FLARM,
#endif
};
/** Maps of drop sounds effect of placing the item in the inventory. */
int ItemInvSnds[] = {
	IS_IHARM,
	IS_IAXE,
	IS_IPOT,
	IS_IBOW,
	IS_GOLD,
	IS_ICAP,
	IS_ISWORD,
	IS_ISHIEL,
	IS_ISWORD,
	IS_IROCK,
	IS_IAXE,
	IS_ISTAF,
	IS_IRING,
	IS_ICAP,
	IS_ILARM,
	IS_ISHIEL,
	IS_ISCROL,
	IS_IHARM,
	IS_IBOOK,
	IS_IHARM,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IPOT,
	IS_IBODY,
	IS_IBODY,
	IS_IMUSH,
	IS_ISIGN,
	IS_IBLST,
	IS_IANVL,
	IS_ISTAF,
#ifdef HELLFIRE
	IS_IROCK,
	IS_ISCROL,
	IS_ISCROL,
	IS_IROCK,
	IS_IMUSH,
	IS_IHARM,
	IS_ILARM,
	IS_ILARM,
#endif
};
#ifdef HELLFIRE
char *off_4A5AC4 = "SItem";
#endif
/** Specifies the current Y-coordinate used for validation of items on ground. */
int idoppely = DBORDERY;
/** Maps from Griswold premium item number to a quality level delta as added to the base quality level. */
int premiumlvladd[SMITH_PREMIUM_ITEMS] = {
	-1, -1, 0, 0, 1, 2
#ifdef HELLFIRE
	, 0, 0, 0, 0, 0, 0, 0, 0, 0 // BUGFIX: not the real values 
#endif
};

#ifdef HELLFIRE
int items_4231CA(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_RING && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_RING && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_423230(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_BOW && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_BOW && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_423296(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_STAFF && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_STAFF && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_4232FC(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_SWORD && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_SWORD && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_423362(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_HELM && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_HELM && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_4233C8(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_SHIELD && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_SHIELD && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_42342E(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE &&
			(plr[i].InvBody[j]._itype == ITYPE_LARMOR || plr[i].InvBody[j]._itype == ITYPE_MARMOR || plr[i].InvBody[j]._itype == ITYPE_HARMOR)
			&& res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE &&
			(plr[i].InvList[j]._itype == ITYPE_LARMOR || plr[i].InvList[j]._itype == ITYPE_MARMOR || plr[i].InvList[j]._itype == ITYPE_HARMOR)
		    && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_4234B2(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_MACE && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_MACE && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_423518(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_AMULET && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_AMULET && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

int items_42357E(int i)
{
	int j, res;

	res = 0;
	for (j = 0; j < NUM_INVLOC; j++) {
		if (plr[i].InvBody[j]._iClass != ITYPE_NONE && plr[i].InvBody[j]._itype == ITYPE_AXE && res < plr[i].InvBody[j]._iIvalue)
			res = plr[i].InvBody[j]._iIvalue;
	}
	for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
		if (plr[i].InvList[j]._iClass != ITYPE_NONE && plr[i].InvList[j]._itype == ITYPE_AXE && res < plr[i].InvList[j]._iIvalue)
			res = plr[i].InvList[j]._iIvalue;
	}

	return res;
}

static void SpawnNote()
{
	int x, y, id;

	do {
		x = random_(12, DSIZEX) + DBORDERX;
		y = random_(12, DSIZEY) + DBORDERY;
	} while (!ItemPlace(x, y));
	switch (currlevel) {
	case 22:
		id = IDI_NOTE2;
		break;
	case 23:
		id = IDI_NOTE3;
		break;
	default:
		id = IDI_NOTE1;
		break;
	}
	SpawnQuestItem(id, x, y, 0, 1);
}
#endif

static inline int items_get_currlevel()
{
	int lvl;

	lvl = currlevel;
#ifdef HELLFIRE
	if (lvl >= 17) {
		if (lvl <= 20)
			lvl -= 8;
		else if (lvl <= 24)
			lvl -= 7;
	}
#endif
	return lvl;
}

void InitItemGFX()
{
	int i;
	char arglist[64];

	for (i = 0; i < ITEMTYPES; i++) {
		sprintf(arglist, "Items\\%s.CEL", ItemDropNames[i]);
		itemanims[i] = LoadFileInMem(arglist, NULL);
	}
	memset(UniqueItemFlag, 0, sizeof(UniqueItemFlag));
}

BOOL ItemPlace(int xp, int yp)
{
	if (dMonster[xp][yp] != 0)
		return FALSE;
	if (dPlayer[xp][yp] != 0)
		return FALSE;
	if (dItem[xp][yp] != 0)
		return FALSE;
	if (dObject[xp][yp] != 0)
		return FALSE;
	if (dFlags[xp][yp] & BFLAG_POPULATED)
		return FALSE;
	if (nSolidTable[dPiece[xp][yp]])
		return FALSE;

	return TRUE;
}

void AddInitItems()
{
	int x, y, i, j, rnd, lvl;

	lvl = items_get_currlevel();

	rnd = random_(11, 3) + 3;
	for (j = 0; j < rnd; j++) {
		i = itemavail[0];
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = i;
		do {
			x = random_(12, DSIZEX) + DBORDERX;
			y = random_(12, DSIZEY) + DBORDERY;
		} while (!ItemPlace(x, y));
		item[i]._ix = x;
		item[i]._iy = y;
		dItem[x][y] = i + 1;
		item[i]._iSeed = GetRndSeed();
		SetRndSeed(item[i]._iSeed);
		GetItemAttrs(i, random_(12, 2) ? IDI_HEAL : IDI_MANA, lvl);
		item[i]._iCreateInfo = lvl - CF_PREGEN;
		SetupItem(i);
		item[i]._iAnimFrame = item[i]._iAnimLen;
		item[i]._iAnimFlag = FALSE;
		item[i]._iSelFlag = 1;
		DeltaAddItem(i);
		numitems++;
	}
}

void InitItems()
{
	int i;

	SpawnStoreGold();

	numitems = 0;

	memset(item, 0, sizeof(item));
	memset(itemactive, 0, sizeof(itemactive));

	for (i = 0; i < MAXITEMS; i++) {
		itemavail[i] = i;
	}

	if (!setlevel) {
		GetRndSeed(); /* unused */
		if (QuestStatus(Q_ROCK))
			SpawnRock();
		if (QuestStatus(Q_ANVIL))
			SpawnQuestItem(IDI_ANVIL, 2 * setpc_x + 27, 2 * setpc_y + 27, 0, 1);
#ifdef HELLFIRE
		if (UseCowFarmer && currlevel == 20)
			SpawnQuestItem(IDI_BROWNSUIT, 25, 25, 3, 1);
		if (UseCowFarmer && currlevel == 19)
			SpawnQuestItem(IDI_GREYSUIT, 25, 25, 3, 1);
#endif
		if (currlevel > 0 && currlevel < 16)
			AddInitItems();
#ifdef HELLFIRE
		if (currlevel >= 21 && currlevel <= 23)
			SpawnNote();
#endif
	}

	uitemflag = FALSE;
}

void CalcPlrItemVals(int pnum, BOOL Loadgfx)
{
	PlayerStruct *p;
	ItemStruct *pi;
	ItemStruct *wRight, *wLeft;
	int pvid, d;

	int mind = 0; // min damage
	int maxd = 0; // max damage
	int tac = 0;  // accuracy

	int g;
	int i;
	MissileStruct* mi;

	int bdam = 0;   // bonus damage
	int btohit = 0; // bonus chance to hit
	int bac = 0;    // bonus accuracy

	int iflgs = ISPL_NONE; // item_special_effect flags

#ifdef HELLFIRE
	int pDamAcFlags = 0;
#endif

	int sadd = 0; // added strength
	int madd = 0; // added magic
	int dadd = 0; // added dexterity
	int vadd = 0; // added vitality

	unsigned __int64 spl = 0; // bitarray for all enabled/active spells

	int fr = 0; // fire resistance
	int lr = 0; // lightning resistance
	int mr = 0; // magic resistance

	int dmod = 0; // bonus damage mod?
	int ghit = 0; // increased damage from enemies

	int lrad = 10; // light radius

	int ihp = 0;   // increased HP
	int imana = 0; // increased mana

	int spllvladd = 0; // increased spell level
	int enac = 0;      // enhanced accuracy

	int fmin = 0; // minimum fire damage
	int fmax = 0; // maximum fire damage
	int lmin = 0; // minimum lightning damage
	int lmax = 0; // maximum lightning damage

	p = &plr[pnum];
	pi = p->InvBody;
	for (i = NUM_INVLOC; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE && pi->_iStatFlag) {

			tac += pi->_iAC;
			mind += pi->_iMinDam;
			maxd += pi->_iMaxDam;

			if (pi->_iSpell != SPL_NULL) {
				spl |= (unsigned __int64)1 << (pi->_iSpell - 1);
			}

			if (pi->_iMagical == ITEM_QUALITY_NORMAL || pi->_iIdentified) {
				bdam += pi->_iPLDam;
				btohit += pi->_iPLToHit;
				if (pi->_iPLAC) {
					int tmpac = pi->_iPLAC * pi->_iAC / 100;
					if (tmpac == 0)
						tmpac = 1;
					bac += tmpac;
				}
				iflgs |= pi->_iFlags;
#ifdef HELLFIRE
				pDamAcFlags |= p->pDamAcFlags;
#endif
				sadd += pi->_iPLStr;
				madd += pi->_iPLMag;
				dadd += pi->_iPLDex;
				vadd += pi->_iPLVit;
				fr += pi->_iPLFR;
				lr += pi->_iPLLR;
				mr += pi->_iPLMR;
				dmod += pi->_iPLDamMod;
				ghit += pi->_iPLGetHit;
				lrad += pi->_iPLLight;
				ihp += pi->_iPLHP;
				imana += pi->_iPLMana;
				spllvladd += pi->_iSplLvlAdd;
				enac += pi->_iPLEnAc;
				fmin += pi->_iFMinDam;
				fmax += pi->_iFMaxDam;
				lmin += pi->_iLMinDam;
				lmax += pi->_iLMaxDam;
			}
		}
	}

	wLeft = &p->InvBody[INVLOC_HAND_LEFT];
	wRight = &p->InvBody[INVLOC_HAND_RIGHT];
	if (mind == 0 && maxd == 0) {
		mind = 1;
		maxd = 1;

		if (wLeft->_itype == ITYPE_SHIELD && wLeft->_iStatFlag) {
			maxd = 3;
		}

		if (wRight->_itype == ITYPE_SHIELD && wRight->_iStatFlag) {
			maxd = 3;
		}

#ifdef HELLFIRE
		if (p->_pClass == PC_MONK) {
			if (p->_pLevel >> 1 >= 1)
				mind = p->_pLevel >> 1;
			if (maxd <= p->_pLevel)
				maxd = p->_pLevel;
		}
#endif
	}

#ifdef HELLFIRE
	if (p->_pSpellFlags & PSE_BLOOD_BOIL) {
		sadd += 2 * p->_pLevel;
		dadd += p->_pLevel + p->_pLevel / 2;
		vadd += 2 * p->_pLevel;
	}
	if (p->_pSpellFlags & PSE_LETHARGY) {
		sadd -= 2 * p->_pLevel;
		dadd -= p->_pLevel + p->_pLevel / 2;
		vadd -= 2 * p->_pLevel;
	}
#endif

	p->_pIMinDam = mind;
	p->_pIMaxDam = maxd;
	p->_pIAC = tac;
	p->_pIBonusDam = bdam;
	p->_pIBonusToHit = btohit;
	p->_pIBonusAC = bac;
	p->_pIFlags = iflgs;
#ifdef HELLFIRE
	p->pDamAcFlags = pDamAcFlags;
#endif
	p->_pIBonusDamMod = dmod;
	p->_pIGetHit = ghit;

	if (lrad < 2) {
		lrad = 2;
	}
	if (lrad > 15) {
		lrad = 15;
	}

	if (p->_pLightRad != lrad && pnum == myplr) {
		ChangeLightRadius(p->_plid, lrad);

		pvid = p->_pvid;
		if (lrad >= 10) {
			ChangeVisionRadius(pvid, lrad);
		} else {
			ChangeVisionRadius(pvid, 10);
		}

		p->_pLightRad = lrad;
	}

	p->_pStrength = sadd + p->_pBaseStr;
	if (plr[myplr]._pStrength <= 0) {
		plr[myplr]._pStrength = 0;
	}

	p->_pMagic = madd + p->_pBaseMag;
	if (plr[myplr]._pMagic <= 0) {
		plr[myplr]._pMagic = 0;
	}

	p->_pDexterity = dadd + p->_pBaseDex;
	if (plr[myplr]._pDexterity <= 0) {
		plr[myplr]._pDexterity = 0;
	}

	p->_pVitality = vadd + p->_pBaseVit;
	if (plr[myplr]._pVitality <= 0) {
		plr[myplr]._pVitality = 0;
	}

	if (p->_pClass == PC_ROGUE) {
		p->_pDamageMod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 200;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_MONK) {
		if (wLeft->_itype != ITYPE_STAFF) {
			if (wRight->_itype != ITYPE_STAFF && (wLeft->_itype != ITYPE_NONE || wRight->_itype != ITYPE_NONE)) {
				p->_pDamageMod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 300;
			} else {
				p->_pDamageMod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 150;
			}
		} else {
			p->_pDamageMod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 150;
		}
	} else if (p->_pClass == PC_BARD) {
		if (wLeft->_itype == ITYPE_SWORD || wRight->_itype == ITYPE_SWORD)
			p->_pDamageMod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 150;
		else if (wLeft->_itype == ITYPE_BOW || wRight->_itype == ITYPE_BOW) {
			p->_pDamageMod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 250;
		} else {
			p->_pDamageMod = p->_pLevel * p->_pStrength / 100;
		}
	} else if (p->_pClass == PC_BARBARIAN) {

		if (wLeft->_itype == ITYPE_AXE || wRight->_itype == ITYPE_AXE) {
			p->_pDamageMod = p->_pLevel * p->_pStrength / 75;
		} else if (wLeft->_itype == ITYPE_MACE || wRight->_itype == ITYPE_MACE) {
			p->_pDamageMod = p->_pLevel * p->_pStrength / 75;
		} else if (wLeft->_itype == ITYPE_BOW || wRight->_itype == ITYPE_BOW) {
			p->_pDamageMod = p->_pLevel * p->_pStrength / 300;
		} else {
			p->_pDamageMod = p->_pLevel * p->_pStrength / 100;
		}

		if (wLeft->_itype == ITYPE_SHIELD || wRight->_itype == ITYPE_SHIELD) {
			if (wLeft->_itype == ITYPE_SHIELD)
				p->_pIAC -= wLeft->_iAC / 2;
			else if (wRight->_itype == ITYPE_SHIELD)
				p->_pIAC -= wRight->_iAC / 2;
		} else if (wLeft->_itype != ITYPE_STAFF && wRight->_itype != ITYPE_STAFF && wLeft->_itype != ITYPE_BOW && wRight->_itype != ITYPE_BOW) {
				p->_pDamageMod += p->_pLevel * p->_pVitality / 100;
		}
		p->_pIAC += p->_pLevel/4;
#endif
	} else {
		p->_pDamageMod = p->_pLevel * p->_pStrength / 100;
	}

	p->_pISpells = spl;

	// check if the current RSplType is a valid/allowed spell
	if (p->_pRSplType == RSPLTYPE_CHARGES
	    && !(spl & ((unsigned __int64)1 << (p->_pRSpell - 1)))) {
		p->_pRSpell = SPL_INVALID;
		p->_pRSplType = RSPLTYPE_INVALID;
		force_redraw = 255;
	}

	p->_pISplLvlAdd = spllvladd;
	p->_pIEnAc = enac;

#ifdef HELLFIRE
	if (p->_pClass == PC_BARBARIAN) {
		mr += p->_pLevel;
		fr += p->_pLevel;
		lr += p->_pLevel;
	}

	if (p->_pSpellFlags & PSE_LETHARGY) {
		mr -= p->_pLevel;
		fr -= p->_pLevel;
		lr -= p->_pLevel;
	}
#endif

	if (iflgs & ISPL_ALLRESZERO) {
		// reset resistances to zero if the respective special effect is active
		mr = 0;
		fr = 0;
		lr = 0;
	}

	if (mr > MAXRESIST)
		mr = MAXRESIST;
	p->_pMagResist = mr;

	if (fr > MAXRESIST)
		fr = MAXRESIST;
	p->_pFireResist = fr;

	if (lr > MAXRESIST)
		lr = MAXRESIST;
	p->_pLghtResist = lr;

	if (p->_pClass == PC_WARRIOR) {
		vadd *= 2;
	}
#ifdef HELLFIRE
	if (p->_pClass == PC_BARBARIAN) {
		vadd *= 3;
	}
	if (p->_pClass == PC_ROGUE || p->_pClass == PC_MONK || p->_pClass == PC_BARD) {
#else
	if (p->_pClass == PC_ROGUE) {
#endif
		vadd += vadd >> 1;
	}
	ihp += (vadd << 6);

	if (p->_pClass == PC_SORCERER) {
		madd *= 2;
	}
#ifdef HELLFIRE
	if (p->_pClass == PC_ROGUE || p->_pClass == PC_MONK) {
#else
	if (p->_pClass == PC_ROGUE) {
#endif
		madd += madd >> 1;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_BARD) {
		madd += madd >> 2 + madd >> 1;
#endif
	}
	imana += (madd << 6);

	p->_pHitPoints = ihp + p->_pHPBase;
	p->_pMaxHP = ihp + p->_pMaxHPBase;
#ifdef HELLFIRE
	if (p->_pHitPoints > p->_pMaxHP)
		p->_pHitPoints = p->_pMaxHP;
#endif

	if (pnum == myplr && (p->_pHitPoints >> 6) <= 0) {
		SetPlayerHitPoints(pnum, 0);
	}

	p->_pMana = imana + p->_pManaBase;
	p->_pMaxMana = imana + p->_pMaxManaBase;
#ifdef HELLFIRE
	if (p->_pMana > p->_pMaxMana)
		p->_pMana = p->_pMaxMana;
#endif

	p->_pIFMinDam = fmin;
	p->_pIFMaxDam = fmax;
	p->_pILMinDam = lmin;
	p->_pILMaxDam = lmax;

	if (iflgs & ISPL_INFRAVISION) {
		p->_pInfraFlag = TRUE;
	} else {
		p->_pInfraFlag = FALSE;
	}

	p->_pBlockFlag = FALSE;
#ifdef HELLFIRE
	if (p->_pClass == PC_MONK) {
		if (wLeft->_itype == ITYPE_STAFF && wLeft->_iStatFlag) {
			p->_pBlockFlag = TRUE;
			p->_pIFlags |= ISPL_FASTBLOCK;
		}
		if (wRight->_itype == ITYPE_STAFF && wRight->_iStatFlag) {
			p->_pBlockFlag = TRUE;
			p->_pIFlags |= ISPL_FASTBLOCK;
		}
		if (wLeft->_itype == ITYPE_NONE && wRight->_itype == ITYPE_NONE)
			p->_pBlockFlag = TRUE;
		if (wLeft->_iClass == ICLASS_WEAPON && wLeft->_iLoc != ILOC_TWOHAND && wRight->_itype == ITYPE_NONE)
			p->_pBlockFlag = TRUE;
		if (wRight->_iClass == ICLASS_WEAPON && wRight->_iLoc != ILOC_TWOHAND && wLeft->_itype == ITYPE_NONE)
			p->_pBlockFlag = TRUE;
	}
#endif
	p->_pwtype = WT_MELEE;

	g = 0;

	if (wLeft->_itype != ITYPE_NONE
	    && wLeft->_iClass == ICLASS_WEAPON
	    && wLeft->_iStatFlag) {
		g = wLeft->_itype;
	}

	if (wRight->_itype != ITYPE_NONE
	    && wRight->_iClass == ICLASS_WEAPON
	    && wRight->_iStatFlag) {
		g = wRight->_itype;
	}

	switch (g) {
	case ITYPE_SWORD:
		g = ANIM_ID_SWORD;
		break;
	case ITYPE_AXE:
		g = ANIM_ID_AXE;
		break;
	case ITYPE_BOW:
		p->_pwtype = WT_RANGED;
		g = ANIM_ID_BOW;
		break;
	case ITYPE_MACE:
		g = ANIM_ID_MACE;
		break;
	case ITYPE_STAFF:
		g = ANIM_ID_STAFF;
		break;
	}

	if (wLeft->_itype == ITYPE_SHIELD && wLeft->_iStatFlag) {
		p->_pBlockFlag = TRUE;
		g++;
	}
	if (wRight->_itype == ITYPE_SHIELD && wRight->_iStatFlag) {
		p->_pBlockFlag = TRUE;
		g++;
	}

	pi = &p->InvBody[INVLOC_CHEST];
#ifdef HELLFIRE
	if (pi->_itype == ITYPE_HARMOR && pi->_iStatFlag) {
		if (p->_pClass == PC_MONK && pi->_iMagical == ITEM_QUALITY_UNIQUE)
			p->_pIAC += p->_pLevel >> 1;
		g += ANIM_ID_HEAVY_ARMOR;
	} else if (pi->_itype == ITYPE_MARMOR && pi->_iStatFlag) {
		if (p->_pClass == PC_MONK) {
			if (pi->_iMagical == ITEM_QUALITY_UNIQUE)
				p->_pIAC += p->_pLevel << 1;
			else
				p->_pIAC += p->_pLevel >> 1;
		}
		g += ANIM_ID_MEDIUM_ARMOR;
	} else if (p->_pClass == PC_MONK) {
		p->_pIAC += p->_pLevel << 1;
	}
#else
	if (pi->_itype == ITYPE_MARMOR && pi->_iStatFlag) {
		g += ANIM_ID_MEDIUM_ARMOR;
	}
	if (pi->_itype == ITYPE_HARMOR && pi->_iStatFlag) {
		g += ANIM_ID_HEAVY_ARMOR;
	}
#endif

	if (p->_pgfxnum != g && Loadgfx) {
		p->_pgfxnum = g;
		p->_pGFXLoad = 0;
		LoadPlrGFX(pnum, PFILE_STAND);
		SetPlrAnims(pnum);

		d = p->_pdir;

		assert(p->_pNAnim[d]);
		p->_pAnimData = p->_pNAnim[d];

		p->_pAnimLen = p->_pNFrames;
		p->_pAnimFrame = 1;
		p->_pAnimCnt = 0;
		p->_pAnimDelay = 3;
		p->_pAnimWidth = p->_pNWidth;
		p->_pAnimWidth2 = (p->_pNWidth - 64) >> 1;
	} else {
		p->_pgfxnum = g;
	}

	for (i = 0; i < nummissiles; i++) {
		mi = &missile[missileactive[i]];
		if (mi->_mitype == MIS_MANASHIELD && mi->_misource == pnum) {
			mi->_miVar1 = p->_pHitPoints;
			mi->_miVar2 = p->_pHPBase;
#ifdef HELLFIRE
			break;
#endif
		}
	}
#ifdef HELLFIRE
	pi = &p->InvBody[INVLOC_AMULET];
	if (pi->_itype == ITYPE_NONE || pi->IDidx != IDI_AURIC) {
		int half = MaxGold;
		MaxGold = auricGold / 2;

		if (half != MaxGold)
			StripTopGold(pnum);
	} else {
		MaxGold = auricGold;
	}
#endif

	drawmanaflag = TRUE;
	drawhpflag = TRUE;
}

void CalcPlrScrolls(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &plr[pnum];
	p->_pScrlSpells = 0;

	pi = p->InvList;
	for (i = p->_pNumInv; i > 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE
		 && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
		 && pi->_iStatFlag) {
			p->_pScrlSpells |= (__int64)1 << (pi->_iSpell - 1);
		}
	}
	pi = p->SpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE
		 && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
		 && pi->_iStatFlag) {
			p->_pScrlSpells |= (__int64)1 << (pi->_iSpell - 1);
		}
	}
	if (p->_pRSplType == RSPLTYPE_SCROLL && p->_pRSpell != SPL_INVALID
	 && !(p->_pScrlSpells & ((__int64)1 << (p->_pRSpell - 1)))) {
		p->_pRSpell = SPL_INVALID;
		p->_pRSplType = RSPLTYPE_INVALID;
		force_redraw = 255;
	}
}

void CalcPlrStaff(int pnum)
{
	plr[pnum]._pISpells = 0;
	if (plr[pnum].InvBody[INVLOC_HAND_LEFT]._itype != ITYPE_NONE
	    && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iStatFlag
	    && plr[pnum].InvBody[INVLOC_HAND_LEFT]._iCharges > 0) {
		plr[pnum]._pISpells |= (__int64)1 << (plr[pnum].InvBody[INVLOC_HAND_LEFT]._iSpell - 1);
	}
}

void CalcSelfItems(int pnum)
{
	int i;
	PlayerStruct *p;
	ItemStruct *pi;
	BOOL changeflag;
	int sa, ma, da;

	p = &plr[pnum];

	sa = p->_pBaseStr;
	ma = p->_pBaseMag;
	da = p->_pBaseDex;

	pi = p->InvBody;
	for (i = NUM_INVLOC; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE) {
			pi->_iStatFlag = TRUE;
			if (pi->_iIdentified) {
				sa += pi->_iPLStr;
				ma += pi->_iPLMag;
				da += pi->_iPLDex;
			}
		}
	}
	do {
		changeflag = FALSE;
		pi = p->InvBody;
		for (i = NUM_INVLOC; i != 0; i--, pi++) {
			if (pi->_itype != ITYPE_NONE && pi->_iStatFlag) {
				if (sa < pi->_iMinStr || ma < pi->_iMinMag || da < pi->_iMinDex) {
					changeflag = TRUE;
					pi->_iStatFlag = FALSE;
					if (pi->_iIdentified) {
						sa -= pi->_iPLStr;
						ma -= pi->_iPLMag;
						da -= pi->_iPLDex;
					}
				}
			}
		}
	} while (changeflag);
}

void CalcPlrItemMin(int pnum)
{
	ItemStruct *pi;
	int i;

	pi = plr[pnum].InvList;
	for (i = plr[pnum]._pNumInv; i != 0; i--, pi++) {
		ItemStatOk(pnum, pi);
	}

	pi = plr[pnum].SpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE) {
			ItemStatOk(pnum, pi);
		}
	}
}

void CalcPlrBookVals(int pnum)
{
	ItemStruct *pi;
	int i;

	pi = plr[pnum].InvList;
	for (i = plr[pnum]._pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_MISC && pi->_iMiscId == IMISC_BOOK) {
			SetBookLevel(pnum, pi);
			ItemStatOk(pnum, pi);
		}
	}
}

void CalcPlrInv(int pnum, BOOL Loadgfx)
{
	CalcPlrItemMin(pnum);
	CalcSelfItems(pnum);
	CalcPlrItemVals(pnum, Loadgfx);
	CalcPlrItemMin(pnum);
	if (pnum == myplr) {
		CalcPlrBookVals(pnum);
		CalcPlrScrolls(pnum);
		CalcPlrStaff(pnum);
	}
}

void SetPlrHandItem(ItemStruct *is, int idata)
{
	ItemDataStruct *ids;

	ids = &AllItemsList[idata];

	// zero-initialize struct
	memset(is, 0, sizeof(*is));

	is->_itype = ids->itype;
	is->_iCurs = ids->iCurs;
	strcpy(is->_iName, ids->iName);
	strcpy(is->_iIName, ids->iName);
	is->_iLoc = ids->iLoc;
	is->_iClass = ids->iClass;
	is->_iMinDam = ids->iMinDam;
	is->_iMaxDam = ids->iMaxDam;
	is->_iAC = ids->iMinAC;
	is->_iMiscId = ids->iMiscId;
	is->_iSpell = ids->iSpell;

	if (ids->iMiscId == IMISC_STAFF) {
#ifdef HELLFIRE
		is->_iCharges = 18;
#else
		is->_iCharges = 40;
#endif
	}

	is->_iMaxCharges = is->_iCharges;
	is->_iDurability = ids->iDurability;
	is->_iMaxDur = ids->iDurability;
	is->_iMinStr = ids->iMinStr;
	is->_iMinMag = ids->iMinMag;
	is->_iMinDex = ids->iMinDex;
	is->_ivalue = ids->iValue;
	is->_iIvalue = ids->iValue;
	is->_iPrePower = -1;
	is->_iSufPower = -1;
	is->_iMagical = ITEM_QUALITY_NORMAL;
	is->IDidx = idata;
}

void GetPlrHandSeed(ItemStruct *is)
{
	is->_iSeed = GetRndSeed();
}

void GetGoldSeed(int pnum, ItemStruct *is)
{
	int i, ii, s;
	BOOL doneflag;

	do {
		doneflag = TRUE;
		s = GetRndSeed();
		for (i = 0; i < numitems; i++) {
			ii = itemactive[i];
			if (item[ii]._iSeed == s)
				doneflag = FALSE;
		}
		if (pnum == myplr) {
			for (i = 0; i < plr[pnum]._pNumInv; i++) {
				if (plr[pnum].InvList[i]._iSeed == s)
					doneflag = FALSE;
			}
		}
	} while (!doneflag);

	is->_iSeed = s;
}

void SetGoldItemValue(ItemStruct *is, int value)
{
	is->_ivalue = value;
	if (value >= GOLD_MEDIUM_LIMIT)
		is->_iCurs = ICURS_GOLD_LARGE;
	else if (value <= GOLD_SMALL_LIMIT)
		is->_iCurs = ICURS_GOLD_SMALL;
	else
		is->_iCurs = ICURS_GOLD_MEDIUM;
}

void CreatePlrItems(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &plr[pnum];

	pi = p->InvBody;
	for (i = NUM_INVLOC; i != 0; i--) {
		pi->_itype = ITYPE_NONE;
		pi++;
	}

	// converting this to a for loop creates a `rep stosd` instruction,
	// so this probably actually was a memset
	memset(&p->InvGrid, 0, sizeof(p->InvGrid));

	pi = p->InvList;
	for (i = NUM_INV_GRID_ELEM; i != 0; i--) {
		pi->_itype = ITYPE_NONE;
		pi++;
	}

	p->_pNumInv = 0;

	pi = p->SpdList;
	for (i = MAXBELTITEMS; i != 0; i--) {
		pi->_itype = ITYPE_NONE;
		pi++;
	}

	switch (p->_pClass) {
	case PC_WARRIOR:
		SetPlrHandItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_WARRIOR);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_LEFT]);

		SetPlrHandItem(&p->InvBody[INVLOC_HAND_RIGHT], IDI_WARRSHLD);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_RIGHT]);

#ifdef _DEBUG
		if (!debug_mode_key_w) {
#endif
			SetPlrHandItem(&p->HoldItem, IDI_WARRCLUB);
			GetPlrHandSeed(&p->HoldItem);
			AutoPlace(pnum, 0, 1, 3, &p->HoldItem);
#ifdef _DEBUG
		}
#endif

		SetPlrHandItem(&p->SpdList[0], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[1]);
		break;
	case PC_ROGUE:
		SetPlrHandItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_ROGUE);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_LEFT]);

		SetPlrHandItem(&p->SpdList[0], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[1]);
		break;
	case PC_SORCERER:
		SetPlrHandItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_SORCEROR);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_LEFT]);

#ifdef HELLFIRE
		SetPlrHandItem(&p->SpdList[0], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[1]);
#else
		SetPlrHandItem(&p->SpdList[0], IDI_MANA);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_MANA);
		GetPlrHandSeed(&p->SpdList[1]);
#endif
		break;

#ifdef HELLFIRE
	case PC_MONK:
		SetPlrHandItem(&p->InvBody[INVLOC_HAND_LEFT], 36);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_LEFT]);
		SetPlrHandItem(&p->SpdList[0], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[1]);
		break;
	case PC_BARD:
		SetPlrHandItem(&p->InvBody[INVLOC_HAND_LEFT], 37);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_LEFT]);

		SetPlrHandItem(&p->InvBody[INVLOC_HAND_RIGHT], 38);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_RIGHT]);
		SetPlrHandItem(&p->SpdList[0], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[1]);
		break;
	case PC_BARBARIAN:
		SetPlrHandItem(&p->InvBody[INVLOC_HAND_LEFT], 139);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_LEFT]);

		SetPlrHandItem(&p->InvBody[INVLOC_HAND_RIGHT], 2);
		GetPlrHandSeed(&p->InvBody[INVLOC_HAND_RIGHT]);
		SetPlrHandItem(&p->SpdList[0], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[0]);

		SetPlrHandItem(&p->SpdList[1], IDI_HEAL);
		GetPlrHandSeed(&p->SpdList[1]);
		break;
#endif
	}

	SetPlrHandItem(&p->HoldItem, IDI_GOLD);
	GetPlrHandSeed(&p->HoldItem);

#ifdef _DEBUG
	if (!debug_mode_key_w) {
#endif
		SetGoldItemValue(&p->HoldItem, 100);
		p->_pGold = p->HoldItem._ivalue;
		p->InvList[p->_pNumInv++] = p->HoldItem;
		p->InvGrid[30] = p->_pNumInv;
#ifdef _DEBUG
	} else {
		SetGoldItemValue(&p->HoldItem, GOLD_MAX_LIMIT);
		p->_pGold = p->HoldItem._ivalue * 40;
		for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
			GetPlrHandSeed(&p->HoldItem);
			p->InvList[p->_pNumInv++] = p->HoldItem;
			p->InvGrid[i] = p->_pNumInv;
		}
	}
#endif

	CalcPlrItemVals(pnum, FALSE);
}

BOOL ItemSpaceOk(int i, int j)
{
	int oi;

	// BUGFIX: Check `i + 1 >= MAXDUNX` and `j + 1 >= MAXDUNY` (applied)
	if (i < 0 || i + 1 >= MAXDUNX || j < 0 || j + 1 >= MAXDUNY)
		return FALSE;

	if (dMonster[i][j] != 0)
		return FALSE;

	if (dPlayer[i][j] != 0)
		return FALSE;

	if (dItem[i][j] != 0)
		return FALSE;

	oi = dObject[i][j];
	if (oi != 0) {
		oi = oi > 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return FALSE;
	}

	oi = dObject[i + 1][j + 1];
	if (oi != 0) {
		oi = oi > 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSelFlag != 0)
			return FALSE;
	}

	if (dObject[i + 1][j] > 0
	    && dObject[i][j + 1] > 0
	    && object[dObject[i + 1][j] - 1]._oSelFlag != 0
	    && object[dObject[i][j + 1] - 1]._oSelFlag != 0) {
		return FALSE;
	}

	return !nSolidTable[dPiece[i][j]];
}

BOOL GetItemSpace(int x, int y, char ii)
{
	int i, j, rs;
	int xx, yy;
	BOOL savail;

	yy = 0;
	for (j = y - 1; j <= y + 1; j++) {
		xx = 0;
		for (i = x - 1; i <= x + 1; i++) {
			itemhold[xx][yy] = ItemSpaceOk(i, j);
			xx++;
		}
		yy++;
	}

	savail = FALSE;
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 3; i++) {
			if (itemhold[i][j])
				savail = TRUE;
		}
	}

	rs = random_(13, 15) + 1;

	if (!savail)
		return FALSE;

	xx = 0;
	yy = 0;
	while (rs > 0) {
		if (itemhold[xx][yy])
			rs--;
		if (rs > 0) {
			xx++;
			if (xx == 3) {
				xx = 0;
				yy++;
				if (yy == 3)
					yy = 0;
			}
		}
	}

	xx += x - 1;
	yy += y - 1;
	item[ii]._ix = xx;
	item[ii]._iy = yy;
	dItem[xx][yy] = ii + 1;

	return TRUE;
}

void GetSuperItemSpace(int x, int y, char ii)
{
	int xx, yy;
	int i, j, k;

	if (!GetItemSpace(x, y, ii)) {
		for (k = 2; k < 50; k++) {
			for (j = -k; j <= k; j++) {
				yy = y + j;
				for (i = -k; i <= k; i++) {
					xx = i + x;
					if (ItemSpaceOk(xx, yy)) {
						item[ii]._ix = xx;
						item[ii]._iy = yy;
						dItem[xx][yy] = ii + 1;
						return;
					}
				}
			}
		}
	}
}

void GetSuperItemLoc(int x, int y, int *xx, int *yy)
{
	int i, j, k;

	for (k = 1; k < 50; k++) {
		for (j = -k; j <= k; j++) {
			*yy = y + j;
			for (i = -k; i <= k; i++) {
				*xx = i + x;
				if (ItemSpaceOk(*xx, *yy)) {
					return;
				}
			}
		}
	}
}

void CalcItemValue(int ii)
{
	int v;

	v = item[ii]._iVMult1 + item[ii]._iVMult2;
	if (v > 0) {
		v *= item[ii]._ivalue;
	}
	if (v < 0) {
		v = item[ii]._ivalue / v;
	}
	v += item[ii]._iVAdd1 + item[ii]._iVAdd2;
	if (v <= 0) {
		v = 1;
	}
	item[ii]._iIvalue = v;
}

void GetBookSpell(int ii, int lvl)
{
	SpellData *sd;
	ItemStruct *is;
	int rv, s, bs;

	if (lvl == 0)
		lvl = 1;
	rv = random_(14, MAX_SPELLS) + 1;

#ifdef SPAWN
	if (lvl > 5)
		lvl = 5;
#endif

	s = SPL_FIREBOLT;
#ifdef HELLFIRE
	bs = SPL_FIREBOLT;
#endif
	while (rv > 0) {
		if (spelldata[s].sBookLvl != -1 && lvl >= spelldata[s].sBookLvl) {
			rv--;
			bs = s;
		}
		s++;
		if (gbMaxPlayers == 1) {
			if (s == SPL_RESURRECT)
				s = SPL_TELEKINESIS;
		}
		if (gbMaxPlayers == 1) {
			if (s == SPL_HEALOTHER)
				s = SPL_FLARE;
		}
		if (s == MAX_SPELLS)
			s = 1;
	}
	is = &item[ii];
	is->_iSpell = bs;
	sd = &spelldata[bs];
	strcat(is->_iName, sd->sNameText);
	strcat(is->_iIName, sd->sNameText);
	is->_iMinMag = sd->sMinInt;
	is->_ivalue += sd->sBookCost;
	is->_iIvalue += sd->sBookCost;
	if (sd->sType == STYPE_FIRE)
		is->_iCurs = ICURS_BOOK_RED;
	else if (sd->sType == STYPE_LIGHTNING)
		is->_iCurs = ICURS_BOOK_BLUE;
	else if (sd->sType == STYPE_MAGIC)
		is->_iCurs = ICURS_BOOK_GREY;
}

void GetStaffPower(int ii, int lvl, int bs, BOOL onlygood)
{
	const PLStruct *l[256];
	char istr[128];
	int nl;
	const PLStruct *pres;
	char *iname;

	pres = NULL;
	if (random_(15, 10) == 0 || onlygood) {
		nl = 0;
		for (pres = PL_Prefix; pres->PLPower != -1; pres++) {
			if (pres->PLIType & PLT_STAFF && pres->PLMinLvl <= lvl) {
				if (!onlygood || pres->PLOk) {
					l[nl] = pres;
					nl++;
					if (pres->PLDouble) {
						l[nl] = pres;
						nl++;
					}
				}
			}
		}
		pres = NULL;
		if (nl != 0) {
			pres = l[random_(16, nl)];
			sprintf(istr, "%s %s", pres->PLName, item[ii]._iIName);
			strcpy(item[ii]._iIName, istr);
			item[ii]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    ii,
			    pres->PLPower,
			    pres->PLParam1,
			    pres->PLParam2,
			    pres->PLMinVal,
			    pres->PLMaxVal,
			    pres->PLMultVal);
			item[ii]._iPrePower = pres->PLPower;
		}
	}
	iname = item[ii]._iIName;
	if (!control_WriteStringToBuffer((BYTE *)iname)) {
		strcpy(iname, AllItemsList[item[ii].IDidx].iSName);
		if (pres != NULL) {
			sprintf(istr, "%s %s", pres->PLName, iname);
			strcpy(iname, istr);
		}
		sprintf(istr, "%s of %s", iname, spelldata[bs].sNameText);
		strcpy(iname, istr);
		if (item[ii]._iMagical == ITEM_QUALITY_NORMAL)
			strcpy(item[ii]._iName, iname);
	}
	CalcItemValue(ii);
}

void GetStaffSpell(int ii, int lvl, BOOL onlygood)
{
	SpellData *sd;
	ItemStruct *is;
	int l, rv, s, minc, maxc, v, bs;
	char istr[64];

#ifndef HELLFIRE
	if (random_(17, 4) == 0) {
		GetItemPower(ii, lvl >> 1, lvl, PLT_STAFF, onlygood);
	} else
#endif
	{
		l = lvl >> 1;
		if (l == 0)
			l = 1;
		rv = random_(18, MAX_SPELLS) + 1;

#ifdef SPAWN
		if (lvl > 10)
			lvl = 10;
#endif

		s = SPL_FIREBOLT;
		while (rv > 0) {
			if (spelldata[s].sStaffLvl != -1 && l >= spelldata[s].sStaffLvl) {
				rv--;
				bs = s;
			}
			s++;
			if (gbMaxPlayers == 1 && s == SPL_RESURRECT)
				s = SPL_TELEKINESIS;
			if (gbMaxPlayers == 1 && s == SPL_HEALOTHER)
				s = SPL_FLARE;
			if (s == MAX_SPELLS)
				s = SPL_FIREBOLT;
		}
		is = &item[ii];
		sd = &spelldata[bs];
		sprintf(istr, "%s of %s", is->_iName, sd->sNameText);
		if (!control_WriteStringToBuffer((BYTE *)istr))
			sprintf(istr, "Staff of %s", sd->sNameText);
		strcpy(is->_iName, istr);
		strcpy(is->_iIName, istr);

		minc = sd->sStaffMin;
		maxc = sd->sStaffMax - minc + 1;
		is->_iSpell = bs;
		is->_iCharges = minc + random_(19, maxc);
		is->_iMaxCharges = is->_iCharges;

		is->_iMinMag = sd->sMinInt;
		v = is->_iCharges * sd->sStaffCost / 5;
		is->_ivalue += v;
		is->_iIvalue += v;
		GetStaffPower(ii, lvl, bs, onlygood);
	}
}

#ifdef HELLFIRE
void GetOilType(int ii, int max_lvl)
{
	int cnt, t, j, r;
	char rnd[32];

	if (gbMaxPlayers == 1) {
		if (max_lvl == 0)
			max_lvl = 1;
		cnt = 0;

		for (j = 0; j < (int)(sizeof(OilLevels) / sizeof(OilLevels[0])); j++) {
			if (OilLevels[j] <= max_lvl) {
				rnd[cnt] = j;
				cnt++;
			}
		}
		r = random_(165, cnt);
		t = rnd[r];
	} else {
		r = random_(165, 2);
		t = (r != 0 ? 6 : 5);
	}
	strcpy(item[ii]._iName, OilNames[t]);
	strcpy(item[ii]._iIName, OilNames[t]);
	item[ii]._iMiscId = OilMagic[t];
	item[ii]._ivalue = OilValues[t];
	item[ii]._iIvalue = OilValues[t];
}
#endif

void GetItemAttrs(int ii, int idata, int lvl)
{
	ItemStruct *is;
	ItemDataStruct *ids;
	int rndv;

	is = &item[ii];
	ids = &AllItemsList[idata];

	is->_itype = ids->itype;
	is->_iCurs = ids->iCurs;
	strcpy(is->_iName, ids->iName);
	strcpy(is->_iIName, ids->iName);
	is->_iLoc = ids->iLoc;
	is->_iClass = ids->iClass;
	is->_iMinDam = ids->iMinDam;
	is->_iMaxDam = ids->iMaxDam;
	is->_iAC = ids->iMinAC + random_(20, ids->iMaxAC - ids->iMinAC + 1);
#ifndef HELLFIRE
	is->_iFlags = ids->iFlags;
#endif
	is->_iMiscId = ids->iMiscId;
	is->_iSpell = ids->iSpell;
	is->_iMagical = ITEM_QUALITY_NORMAL;
	is->_ivalue = ids->iValue;
	is->_iIvalue = ids->iValue;
	is->_iVAdd1 = 0;
	is->_iVMult1 = 0;
	is->_iVAdd2 = 0;
	is->_iVMult2 = 0;
	is->_iPLDam = 0;
	is->_iPLToHit = 0;
	is->_iPLAC = 0;
	is->_iPLStr = 0;
	is->_iPLMag = 0;
	is->_iPLDex = 0;
	is->_iPLVit = 0;
	is->_iCharges = 0;
	is->_iMaxCharges = 0;
	is->_iDurability = ids->iDurability;
	is->_iMaxDur = ids->iDurability;
	is->_iMinStr = ids->iMinStr;
	is->_iMinMag = ids->iMinMag;
	is->_iMinDex = ids->iMinDex;
	is->_iPLFR = 0;
	is->_iPLLR = 0;
	is->_iPLMR = 0;
	is->IDidx = idata;
	is->_iPLDamMod = 0;
	is->_iPLGetHit = 0;
	is->_iPLLight = 0;
	is->_iSplLvlAdd = 0;
	is->_iRequest = FALSE;
	is->_iFMinDam = 0;
	is->_iFMaxDam = 0;
	is->_iLMinDam = 0;
	is->_iLMaxDam = 0;
	is->_iPLEnAc = 0;
	is->_iPLMana = 0;
	is->_iPLHP = 0;
	is->_iPrePower = -1;
	is->_iSufPower = -1;

	if (is->_iMiscId == IMISC_BOOK)
		GetBookSpell(ii, lvl);

#ifdef HELLFIRE
	is->_iFlags = 0;
	is->_iDamAcFlags = 0;

	if (is->_iMiscId == IMISC_OILOF)
		GetOilType(ii, lvl);
#endif
	if (is->_itype == ITYPE_GOLD) {
		lvl = items_get_currlevel();
		if (gnDifficulty == DIFF_NORMAL)
			rndv = 5 * lvl + random_(21, 10 * lvl);
		else if (gnDifficulty == DIFF_NIGHTMARE)
			rndv = 5 * (lvl + 16) + random_(21, 10 * (lvl + 16));
		else if (gnDifficulty == DIFF_HELL)
			rndv = 5 * (lvl + 32) + random_(21, 10 * (lvl + 32));
		if (leveltype == DTYPE_HELL)
			rndv += rndv >> 3;
		if (rndv > GOLD_MAX_LIMIT)
			rndv = GOLD_MAX_LIMIT;

		SetGoldItemValue(is, rndv);
	}
}

int RndPL(int param1, int param2)
{
	return param1 + random_(22, param2 - param1 + 1);
}

int PLVal(int pv, int p1, int p2, int minv, int maxv)
{
	int dp, dv, rv;

	rv = minv;
	dp = p2 - p1;
	if (dp != 0) {
		dv = maxv - minv;
		if (dv != 0)
			rv += dv * (100 * (pv - p1) / dp) / 100;
	}
	return rv;
}

void SaveItemPower(int ii, int power, int param1, int param2, int minval, int maxval, int multval)
{
	ItemStruct *is;
	int r, r2;

	is = &item[ii];
	r = RndPL(param1, param2);
	switch (power) {
	case IPL_TOHIT:
		is->_iPLToHit += r;
		break;
	case IPL_TOHIT_CURSE:
		is->_iPLToHit -= r;
		break;
	case IPL_DAMP:
		is->_iPLDam += r;
		break;
	case IPL_DAMP_CURSE:
		is->_iPLDam -= r;
		break;
#ifdef HELLFIRE
	case IPL_DOPPELGANGER:
		is->_iDamAcFlags |= ISPH_DOPPELGANGER;
		// no break
#endif
	case IPL_TOHIT_DAMP:
		r = RndPL(param1, param2);
		is->_iPLDam += r;
		if (param1 == 20)
			r2 = RndPL(1, 5);
		if (param1 == 36)
			r2 = RndPL(6, 10);
		if (param1 == 51)
			r2 = RndPL(11, 15);
		if (param1 == 66)
			r2 = RndPL(16, 20);
		if (param1 == 81)
			r2 = RndPL(21, 30);
		if (param1 == 96)
			r2 = RndPL(31, 40);
		if (param1 == 111)
			r2 = RndPL(41, 50);
		if (param1 == 126)
			r2 = RndPL(51, 75);
		if (param1 == 151)
			r2 = RndPL(76, 100);
		is->_iPLToHit += r2;
		break;
	case IPL_TOHIT_DAMP_CURSE:
		is->_iPLDam -= r;
		if (param1 == 25)
			r2 = RndPL(1, 5);
		if (param1 == 50)
			r2 = RndPL(6, 10);
		is->_iPLToHit -= r2;
		break;
	case IPL_ACP:
		is->_iPLAC += r;
		break;
	case IPL_ACP_CURSE:
		is->_iPLAC -= r;
		break;
	case IPL_SETAC:
		is->_iAC = r;
		break;
	case IPL_AC_CURSE:
		is->_iAC -= r;
		break;
	case IPL_FIRERES:
		is->_iPLFR += r;
		break;
	case IPL_LIGHTRES:
		is->_iPLLR += r;
		break;
	case IPL_MAGICRES:
		is->_iPLMR += r;
		break;
	case IPL_ALLRES:
		is->_iPLFR += r;
		is->_iPLLR += r;
		is->_iPLMR += r;
		if (is->_iPLFR < 0)
			is->_iPLFR = 0;
		if (is->_iPLLR < 0)
			is->_iPLLR = 0;
		if (is->_iPLMR < 0)
			is->_iPLMR = 0;
		break;
	case IPL_SPLLVLADD:
		is->_iSplLvlAdd = r;
		break;
	case IPL_CHARGES:
		is->_iCharges *= param1;
		is->_iMaxCharges = is->_iCharges;
		break;
	case IPL_SPELL:
		is->_iSpell = param1;
#ifdef HELLFIRE
		is->_iCharges = param2;
#else
		is->_iCharges = param1; // BUGFIX: should be param2. This code was correct in v1.04, and the bug was introduced between 1.04 and 1.09b.
#endif
		is->_iMaxCharges = param2;
		break;
	case IPL_FIREDAM:
		is->_iFlags |= ISPL_FIREDAM;
#ifdef HELLFIRE
		is->_iFlags &= ~ISPL_LIGHTDAM;
#endif
		is->_iFMinDam = param1;
		is->_iFMaxDam = param2;
#ifdef HELLFIRE
		is->_iLMinDam = 0;
		is->_iLMaxDam = 0;
#endif
		break;
	case IPL_LIGHTDAM:
		is->_iFlags |= ISPL_LIGHTDAM;
#ifdef HELLFIRE
		is->_iFlags &= ~ISPL_FIREDAM;
#endif
		is->_iLMinDam = param1;
		is->_iLMaxDam = param2;
#ifdef HELLFIRE
		is->_iFMinDam = 0;
		is->_iFMaxDam = 0;
#endif
		break;
	case IPL_STR:
		is->_iPLStr += r;
		break;
	case IPL_STR_CURSE:
		is->_iPLStr -= r;
		break;
	case IPL_MAG:
		is->_iPLMag += r;
		break;
	case IPL_MAG_CURSE:
		is->_iPLMag -= r;
		break;
	case IPL_DEX:
		is->_iPLDex += r;
		break;
	case IPL_DEX_CURSE:
		is->_iPLDex -= r;
		break;
	case IPL_VIT:
		is->_iPLVit += r;
		break;
	case IPL_VIT_CURSE:
		is->_iPLVit -= r;
		break;
	case IPL_ATTRIBS:
		is->_iPLStr += r;
		is->_iPLMag += r;
		is->_iPLDex += r;
		is->_iPLVit += r;
		break;
	case IPL_ATTRIBS_CURSE:
		is->_iPLStr -= r;
		is->_iPLMag -= r;
		is->_iPLDex -= r;
		is->_iPLVit -= r;
		break;
	case IPL_GETHIT_CURSE:
		is->_iPLGetHit += r;
		break;
	case IPL_GETHIT:
		is->_iPLGetHit -= r;
		break;
	case IPL_LIFE:
		is->_iPLHP += r << 6;
		break;
	case IPL_LIFE_CURSE:
		is->_iPLHP -= r << 6;
		break;
	case IPL_MANA:
		is->_iPLMana += r << 6;
		drawmanaflag = TRUE;
		break;
	case IPL_MANA_CURSE:
		is->_iPLMana -= r << 6;
		drawmanaflag = TRUE;
		break;
	case IPL_DUR:
		r2 = r * is->_iMaxDur / 100;
		is->_iMaxDur += r2;
		is->_iDurability += r2;
		break;
#ifdef HELLFIRE
	case IPL_CRYSTALLINE:
		is->_iPLDam += 140 + r * 2;
		// no break
#endif
	case IPL_DUR_CURSE:
		is->_iMaxDur -= r * is->_iMaxDur / 100;
		if (is->_iMaxDur < 1)
			is->_iMaxDur = 1;
		is->_iDurability = is->_iMaxDur;
		break;
	case IPL_INDESTRUCTIBLE:
		is->_iDurability = DUR_INDESTRUCTIBLE;
		is->_iMaxDur = DUR_INDESTRUCTIBLE;
		break;
	case IPL_LIGHT:
		is->_iPLLight += param1;
		break;
	case IPL_LIGHT_CURSE:
		is->_iPLLight -= param1;
		break;
#ifdef HELLFIRE
	case IPL_MULT_ARROWS:
		is->_iFlags |= ISPL_MULT_ARROWS;
		break;
#endif
	case IPL_FIRE_ARROWS:
		is->_iFlags |= ISPL_FIRE_ARROWS;
#ifdef HELLFIRE
		is->_iFlags &= ~ISPL_LIGHT_ARROWS;
#endif
		is->_iFMinDam = param1;
		is->_iFMaxDam = param2;
#ifdef HELLFIRE
		is->_iLMinDam = 0;
		is->_iLMaxDam = 0;
#endif
		break;
	case IPL_LIGHT_ARROWS:
		is->_iFlags |= ISPL_LIGHT_ARROWS;
#ifdef HELLFIRE
		is->_iFlags &= ~ISPL_FIRE_ARROWS;
#endif
		is->_iLMinDam = param1;
		is->_iLMaxDam = param2;
#ifdef HELLFIRE
		is->_iFMinDam = 0;
		is->_iFMaxDam = 0;
#endif
		break;
#ifdef HELLFIRE
	case IPL_FIREBALL:
		is->_iFlags |= (ISPL_LIGHT_ARROWS | ISPL_FIRE_ARROWS);
		is->_iFMinDam = param1;
		is->_iFMaxDam = param2;
		is->_iLMinDam = 0;
		is->_iLMaxDam = 0;
		break;
#endif
	case IPL_THORNS:
		is->_iFlags |= ISPL_THORNS;
		break;
	case IPL_NOMANA:
		is->_iFlags |= ISPL_NOMANA;
		drawmanaflag = TRUE;
		break;
	case IPL_NOHEALPLR:
		is->_iFlags |= ISPL_NOHEALPLR;
		break;
	case IPL_ABSHALFTRAP:
		is->_iFlags |= ISPL_ABSHALFTRAP;
		break;
	case IPL_KNOCKBACK:
		is->_iFlags |= ISPL_KNOCKBACK;
		break;
	case IPL_3XDAMVDEM:
		is->_iFlags |= ISPL_3XDAMVDEM;
		break;
	case IPL_ALLRESZERO:
		is->_iFlags |= ISPL_ALLRESZERO;
		break;
	case IPL_NOHEALMON:
		is->_iFlags |= ISPL_NOHEALMON;
		break;
	case IPL_STEALMANA:
		if (param1 == 3)
			is->_iFlags |= ISPL_STEALMANA_3;
		if (param1 == 5)
			is->_iFlags |= ISPL_STEALMANA_5;
		drawmanaflag = TRUE;
		break;
	case IPL_STEALLIFE:
		if (param1 == 3)
			is->_iFlags |= ISPL_STEALLIFE_3;
		if (param1 == 5)
			is->_iFlags |= ISPL_STEALLIFE_5;
		drawhpflag = TRUE;
		break;
	case IPL_TARGAC:
#ifdef HELLFIRE
		is->_iPLEnAc = param1;
#else
		is->_iPLEnAc += r;
#endif
		break;
	case IPL_FASTATTACK:
		if (param1 == 1)
			is->_iFlags |= ISPL_QUICKATTACK;
		if (param1 == 2)
			is->_iFlags |= ISPL_FASTATTACK;
		if (param1 == 3)
			is->_iFlags |= ISPL_FASTERATTACK;
		if (param1 == 4)
			is->_iFlags |= ISPL_FASTESTATTACK;
		break;
	case IPL_FASTRECOVER:
		if (param1 == 1)
			is->_iFlags |= ISPL_FASTRECOVER;
		if (param1 == 2)
			is->_iFlags |= ISPL_FASTERRECOVER;
		if (param1 == 3)
			is->_iFlags |= ISPL_FASTESTRECOVER;
		break;
	case IPL_FASTBLOCK:
		is->_iFlags |= ISPL_FASTBLOCK;
		break;
	case IPL_DAMMOD:
		is->_iPLDamMod += r;
		break;
	case IPL_RNDARROWVEL:
		is->_iFlags |= ISPL_RNDARROWVEL;
		break;
	case IPL_SETDAM:
		is->_iMinDam = param1;
		is->_iMaxDam = param2;
		break;
	case IPL_SETDUR:
		is->_iDurability = param1;
		is->_iMaxDur = param1;
		break;
	case IPL_FASTSWING:
		is->_iFlags |= ISPL_FASTERATTACK;
		break;
	case IPL_ONEHAND:
		is->_iLoc = ILOC_ONEHAND;
		break;
	case IPL_DRAINLIFE:
		is->_iFlags |= ISPL_DRAINLIFE;
		break;
	case IPL_RNDSTEALLIFE:
		is->_iFlags |= ISPL_RNDSTEALLIFE;
		break;
	case IPL_INFRAVISION:
		is->_iFlags |= ISPL_INFRAVISION;
		break;
	case IPL_NOMINSTR:
		is->_iMinStr = 0;
		break;
	case IPL_INVCURS:
		is->_iCurs = param1;
		break;
	case IPL_ADDACLIFE:
#ifdef HELLFIRE
		is->_iFlags |= (ISPL_LIGHT_ARROWS | ISPL_FIRE_ARROWS);
		is->_iFMinDam = param1;
		is->_iFMaxDam = param2;
		is->_iLMinDam = 1;
		is->_iLMaxDam = 0;
#else
		is->_iPLHP = (plr[myplr]._pIBonusAC + plr[myplr]._pIAC + plr[myplr]._pDexterity / 5) << 6;
#endif
		break;
	case IPL_ADDMANAAC:
#ifdef HELLFIRE
		is->_iFlags |= (ISPL_LIGHTDAM | ISPL_FIREDAM);
		is->_iFMinDam = param1;
		is->_iFMaxDam = param2;
		is->_iLMinDam = 2;
		is->_iLMaxDam = 0;
#else
		is->_iAC += (plr[myplr]._pMaxManaBase >> 6) / 10;
#endif
		break;
	case IPL_FIRERESCLVL:
		is->_iPLFR = 30 - plr[myplr]._pLevel;
		if (is->_iPLFR < 0)
			is->_iPLFR = 0;
		break;
#ifdef HELLFIRE
	case IPL_FIRERES_CURSE:
		is->_iPLFR -= r;
		break;
	case IPL_LIGHTRES_CURSE:
		is->_iPLLR -= r;
		break;
	case IPL_MAGICRES_CURSE:
		is->_iPLMR -= r;
		break;
	case IPL_ALLRES_CURSE:
		is->_iPLFR -= r;
		is->_iPLLR -= r;
		is->_iPLMR -= r;
		break;
	case IPL_DEVASTATION:
		is->_iDamAcFlags |= ISPH_DEVASTATION;
		break;
	case IPL_DECAY:
		is->_iDamAcFlags |= ISPH_DECAY;
		is->_iPLDam += r;
		break;
	case IPL_PERIL:
		is->_iDamAcFlags |= ISPH_PERIL;
		break;
	case IPL_JESTERS:
		is->_iDamAcFlags |= ISPH_JESTERS;
		break;
	case IPL_ACDEMON:
		is->_iDamAcFlags |= ISPH_ACDEMON;
		break;
	case IPL_ACUNDEAD:
		is->_iDamAcFlags |= ISPH_ACUNDEAD;
		break;
	case IPL_MANATOLIFE:
		r2 = ((plr[myplr]._pMaxManaBase >> 6) * 50 / 100);
		is->_iPLMana -= (r2 << 6);
		is->_iPLHP += (r2 << 6);
		break;
	case IPL_LIFETOMANA:
		r2 = ((plr[myplr]._pMaxHPBase >> 6) * 40 / 100);
		is->_iPLHP -= (r2 << 6);
		is->_iPLMana += (r2 << 6);
		break;
#endif
	}
	if (is->_iVAdd1 || is->_iVMult1) {
		is->_iVAdd2 = PLVal(r, param1, param2, minval, maxval);
		is->_iVMult2 = multval;
	} else {
		is->_iVAdd1 = PLVal(r, param1, param2, minval, maxval);
		is->_iVMult1 = multval;
	}
}

void GetItemPower(int ii, int minlvl, int maxlvl, int flgs, BOOL onlygood)
{
	int pre, post, nl;
	const PLStruct *pres, *sufs;
	const PLStruct *l[256];
	char istr[128];
	BYTE goe;
	char *iname;

	pre = random_(23, 4);
	post = random_(23, 3);
	if (pre != 0 && post == 0) {
		if (random_(23, 2) != 0)
			post = 1;
		else
			pre = 0;
	}
	pres = NULL;
	sufs = NULL;
	goe = 0;
	if (!onlygood && random_(0, 3) != 0)
		onlygood = TRUE;
	if (pre == 0) {
		nl = 0;
		for (pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {
			if ((flgs & pres->PLIType)
			 && pres->PLMinLvl >= minlvl && pres->PLMinLvl <= maxlvl 
			 && (!onlygood || pres->PLOk)
			 && (flgs != 256 || pres->PLPower != 15)) {
				l[nl] = pres;
				nl++;
				if (pres->PLDouble) {
					l[nl] = pres;
					nl++;
				}
			}
		}
		pres = NULL;
		if (nl != 0) {
			pres = l[random_(23, nl)];
			sprintf(istr, "%s %s", pres->PLName, item[ii]._iIName);
			strcpy(item[ii]._iIName, istr);
			item[ii]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    ii,
			    pres->PLPower,
			    pres->PLParam1,
			    pres->PLParam2,
			    pres->PLMinVal,
			    pres->PLMaxVal,
			    pres->PLMultVal);
			item[ii]._iPrePower = pres->PLPower;
			goe = pres->PLGOE;
		}
	}
	if (post != 0) {
		nl = 0;
		for (sufs = PL_Suffix; sufs->PLPower != IPL_INVALID; sufs++) {
			if (sufs->PLIType & flgs
			    && sufs->PLMinLvl >= minlvl && sufs->PLMinLvl <= maxlvl
			    && (goe | sufs->PLGOE) != 0x11
			    && (!onlygood || sufs->PLOk)) {
				l[nl] = sufs;
				nl++;
			}
		}
		sufs = NULL;
		if (nl != 0) {
			sufs = l[random_(23, nl)];
			sprintf(istr, "%s of %s", item[ii]._iIName, sufs->PLName);
			strcpy(item[ii]._iIName, istr);
			item[ii]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    ii,
			    sufs->PLPower,
			    sufs->PLParam1,
			    sufs->PLParam2,
			    sufs->PLMinVal,
			    sufs->PLMaxVal,
			    sufs->PLMultVal);
			item[ii]._iSufPower = sufs->PLPower;
		}
	}
	iname = item[ii]._iIName;
	if (!control_WriteStringToBuffer((BYTE *)iname)) {
		strcpy(iname, AllItemsList[item[ii].IDidx].iSName);
		if (pres != NULL) {
			sprintf(istr, "%s %s", pres->PLName, iname);
			strcpy(iname, istr);
		}
		if (sufs != NULL) {
			sprintf(istr, "%s of %s", iname, sufs->PLName);
			strcpy(iname, istr);
		}
	}
	if (pres != NULL || sufs != NULL)
		CalcItemValue(ii);
}

void GetItemBonus(int ii, int minlvl, int maxlvl, BOOL onlygood, BOOLEAN allowspells)
{
	if (item[ii]._iClass != ICLASS_GOLD) {
		if (minlvl > 25)
			minlvl = 25;

		switch (item[ii]._itype) {
		case ITYPE_SWORD:
		case ITYPE_AXE:
		case ITYPE_MACE:
			GetItemPower(ii, minlvl, maxlvl, PLT_WEAP, onlygood);
			break;
		case ITYPE_BOW:
			GetItemPower(ii, minlvl, maxlvl, PLT_BOW, onlygood);
			break;
		case ITYPE_SHIELD:
			GetItemPower(ii, minlvl, maxlvl, PLT_SHLD, onlygood);
			break;
		case ITYPE_LARMOR:
		case ITYPE_HELM:
		case ITYPE_MARMOR:
		case ITYPE_HARMOR:
			GetItemPower(ii, minlvl, maxlvl, PLT_ARMO, onlygood);
			break;
		case ITYPE_STAFF:
			if (allowspells)
				GetStaffSpell(ii, maxlvl, onlygood);
			else
				GetItemPower(ii, minlvl, maxlvl, PLT_STAFF, onlygood);
			break;
		case ITYPE_RING:
		case ITYPE_AMULET:
			GetItemPower(ii, minlvl, maxlvl, PLT_MISC, onlygood);
			break;
		}
	}
}

void SetupItem(int ii)
{
	ItemStruct *is;
	int it;

	is = &item[ii];
	it = ItemCAnimTbl[is->_iCurs];
	is->_iAnimData = itemanims[it];
	is->_iAnimLen = ItemAnimLs[it];
	is->_iAnimWidth = 96;
	is->_iAnimWidth2 = 16;
	is->_iIdentified = FALSE;
	is->_iPostDraw = FALSE;

	if (plr[myplr].pLvlLoad == 0) {
		is->_iAnimFrame = 1;
		is->_iAnimFlag = TRUE;
		is->_iSelFlag = 0;
	} else {
		is->_iAnimFrame = is->_iAnimLen;
		is->_iAnimFlag = FALSE;
		is->_iSelFlag = 1;
	}
}

int RndItem(int lvl)
{
	int i, ri, r;
	int ril[512];

	if (random_(24, 100) > 40)
		return 0;

	if (random_(24, 100) > 25)
		return IDI_GOLD + 1;

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER
		 || lvl < AllItemsList[i].iMinMLvl
		 || (gbMaxPlayers == 1 && (AllItemsList[i].iSpell == SPL_RESURRECT || AllItemsList[i].iSpell == SPL_HEALOTHER)))
			continue;
#ifdef HELLFIRE
		if (ri == 512)
			break;
#endif
		ril[ri] = i;
		ri++;
		if (AllItemsList[i].iRnd == IDROP_DOUBLE) {
#ifdef HELLFIRE
			if (ri == 512)
				break;
#endif
			ril[ri] = i;
			ri++;
		}
	}

	r = random_(24, ri);
	return ril[r] + 1;
}

int RndUItem(int lvl)
{
	int i, ri;
	int ril[512];
	BOOL okflag;

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		okflag = TRUE;
		if (AllItemsList[i].iRnd == IDROP_NEVER)
			okflag = FALSE;
		if (lvl < AllItemsList[i].iMinMLvl)
			okflag = FALSE;
		if (AllItemsList[i].itype == ITYPE_MISC)
			okflag = FALSE;
		if (AllItemsList[i].itype == ITYPE_GOLD)
			okflag = FALSE;
		if (AllItemsList[i].itype == ITYPE_MEAT)
			okflag = FALSE;
		if (AllItemsList[i].iMiscId == IMISC_BOOK)
			okflag = TRUE;
		if (AllItemsList[i].iSpell == SPL_RESURRECT && gbMaxPlayers == 1)
			okflag = FALSE;
		if (AllItemsList[i].iSpell == SPL_HEALOTHER && gbMaxPlayers == 1)
			okflag = FALSE;
#ifdef HELLFIRE
		if (okflag && ri < 512) {
#else
		if (okflag) {
#endif
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(25, ri)];
}

int RndAllItems(int lvl)
{
	int i, ri;
	int ril[512];

	if (random_(26, 100) > 25)
		return 0;

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER
		 || lvl < AllItemsList[i].iMinMLvl
		 || (gbMaxPlayers == 1 && (AllItemsList[i].iSpell == SPL_RESURRECT || AllItemsList[i].iSpell == SPL_HEALOTHER)))
			continue;
#ifdef HELLFIRE
		if (ri == 512)
			break;
#endif
		ril[ri] = i;
		ri++;
	}

	return ril[random_(26, ri)];
}

int RndTypeItems(int itype, int imid, int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER
		 || lvl < AllItemsList[i].iMinMLvl
		 || AllItemsList[i].itype != itype
		 || (imid != -1 && AllItemsList[i].iMiscId != imid))
			continue;
#ifdef HELLFIRE
		if (ri == 512)
			break;
#endif
		ril[ri] = i;
		ri++;
	}

	return ril[random_(27, ri)];
}

int CheckUnique(int ii, int lvl, int uper, BOOL recreate)
{
	int j, idata, numu;
	BOOLEAN uok[128];

	if (random_(28, 100) > uper)
		return UITYPE_INVALID;

	numu = 0;
	memset(uok, 0, sizeof(uok));
	for (j = 0; UniqueItemList[j].UIItemId != UITYPE_INVALID; j++) {
		if (UniqueItemList[j].UIItemId == AllItemsList[item[ii].IDidx].iItemId
		    && lvl >= UniqueItemList[j].UIMinLvl
		    && (recreate || !UniqueItemFlag[j] || gbMaxPlayers != 1)) {
			uok[j] = TRUE;
			numu++;
		}
	}

	if (numu == 0)
		return UITYPE_INVALID;

	random_(29, 10); /// BUGFIX: unused, last unique in array always gets chosen
	idata = 0;
	while (numu > 0) {
		if (uok[idata])
			numu--;
		if (numu > 0) {
			idata++;
			if (idata == 128)
				idata = 0;
		}
	}

	return idata;
}

void GetUniqueItem(int ii, int uid)
{
	const UItemStruct *ui;

	UniqueItemFlag[uid] = TRUE;
	ui = &UniqueItemList[uid];
	SaveItemPower(ii, ui->UIPower1, ui->UIParam1, ui->UIParam2, 0, 0, 1);

	if (ui->UINumPL > 1)
		SaveItemPower(ii, ui->UIPower2, ui->UIParam3, ui->UIParam4, 0, 0, 1);
	if (ui->UINumPL > 2)
		SaveItemPower(ii, ui->UIPower3, ui->UIParam5, ui->UIParam6, 0, 0, 1);
	if (ui->UINumPL > 3)
		SaveItemPower(ii, ui->UIPower4, ui->UIParam7, ui->UIParam8, 0, 0, 1);
	if (ui->UINumPL > 4)
		SaveItemPower(ii, ui->UIPower5, ui->UIParam9, ui->UIParam10, 0, 0, 1);
	if (ui->UINumPL > 5)
		SaveItemPower(ii, ui->UIPower6, ui->UIParam11, ui->UIParam12, 0, 0, 1);

	strcpy(item[ii]._iIName, ui->UIName);
	item[ii]._iIvalue = ui->UIValue;

	if (item[ii]._iMiscId == IMISC_UNIQUE)
		item[ii]._iSeed = uid;

	item[ii]._iUid = uid;
	item[ii]._iMagical = ITEM_QUALITY_UNIQUE;
	item[ii]._iCreateInfo |= CF_UNIQUE;
}

void SpawnUnique(int uid, int x, int y)
{
	int ii, idx, lvl;

	lvl = items_get_currlevel();

	if (numitems >= MAXITEMS)
		return;

	ii = itemavail[0];
	GetSuperItemSpace(x, y, ii);
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = ii;

	idx = 0;
	while (AllItemsList[idx].iItemId != UniqueItemList[uid].UIItemId) {
		idx++;
	}

	GetItemAttrs(ii, idx, lvl);
	GetUniqueItem(ii, uid);
	SetupItem(ii);
	numitems++;
}

void ItemRndDur(int ii)
{
	if (item[ii]._iDurability && item[ii]._iDurability != DUR_INDESTRUCTIBLE)
		item[ii]._iDurability = random_(0, item[ii]._iMaxDur >> 1) + (item[ii]._iMaxDur >> 2) + 1;
}

void SetupAllItems(int ii, int idx, int iseed, int lvl, int uper, BOOL onlygood, BOOL recreate, BOOL pregen)
{
	int iblvl, uid;

	item[ii]._iSeed = iseed;
	SetRndSeed(iseed);
	GetItemAttrs(ii, idx, lvl >> 1);
	item[ii]._iCreateInfo = lvl;

	if (pregen)
		item[ii]._iCreateInfo = lvl | CF_PREGEN;
	if (onlygood)
		item[ii]._iCreateInfo |= CF_ONLYGOOD;

	if (uper == 15)
		item[ii]._iCreateInfo |= CF_UPER15;
	else if (uper == 1)
		item[ii]._iCreateInfo |= CF_UPER1;

	if (item[ii]._iMiscId != IMISC_UNIQUE) {
		iblvl = -1;
		if ((random_(32, 100) <= 10 || random_(33, 100) <= lvl)
		 || item[ii]._iMiscId == IMISC_STAFF
		 || item[ii]._iMiscId == IMISC_RING
		 || item[ii]._iMiscId == IMISC_AMULET)
			iblvl = lvl;
		if (onlygood)
			iblvl = lvl;
		if (uper == 15)
			iblvl = lvl + 4;
		if (iblvl != -1) {
			uid = CheckUnique(ii, iblvl, uper, recreate);
			if (uid == UITYPE_INVALID) {
				GetItemBonus(ii, iblvl >> 1, iblvl, onlygood, TRUE);
			} else {
				GetUniqueItem(ii, uid);
			}
		}
		if (item[ii]._iMagical != ITEM_QUALITY_UNIQUE)
			ItemRndDur(ii);
	} else {
		if (item[ii]._iLoc != ILOC_UNEQUIPABLE) {
			//uid = CheckUnique(ii, iblvl, uper, recreate);
			//if (uid != UITYPE_INVALID) {
			//	GetUniqueItem(ii, uid);
			//}
			GetUniqueItem(ii, iseed); // BUG: the second argument to GetUniqueItem should be uid.
		}
	}
	SetupItem(ii);
}

void SpawnItem(int mnum, int x, int y, BOOL sendmsg)
{
	MonsterStruct* mon;
	int ii, idx;
	BOOL onlygood;

	mon = &monster[mnum];
	if (mon->_uniqtype || ((mon->MData->mTreasure & 0x8000) && gbMaxPlayers != 1)) {
		if ((mon->MData->mTreasure & 0x8000) != 0 && gbMaxPlayers == 1)
			idx = -1 - (mon->MData->mTreasure & 0xFFF);
		else
			idx = RndUItem(mon->mLevel);
		if (idx < 0) {
			SpawnUnique(-(idx + 1), x, y);
			return;
		}
		onlygood = TRUE;
	} else if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 != QS_MUSHGIVEN) {
		if ((mon->MData->mTreasure & 0x8000) != 0)
			idx = -1 - (mon->MData->mTreasure & 0xFFF);
		else if (mon->MData->mTreasure & 0x4000)
			idx = 0;
		else
			idx = RndItem(mon->mLevel);
		if (!idx)
			return;
		if (idx > 0) {
			idx--;
			onlygood = FALSE;
		} else {
			SpawnUnique(-(idx + 1), x, y);
			return;
		}
	} else {
		idx = IDI_BRAIN;
		quests[Q_MUSHROOM]._qvar1 = QS_BRAINSPAWNED;
	}

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		SetupAllItems(ii, idx, GetRndSeed(), mon->MData->mLevel,
			mon->_uniqtype != 0 ? 15 : 1, onlygood, FALSE, FALSE);
		numitems++;
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
	}
}

void CreateRndItem(int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta)
{
	int idx, ii, lvl;

	lvl = items_get_currlevel();
	lvl <<= 1;

	if (onlygood)
		idx = RndUItem(lvl);
	else
		idx = RndAllItems(lvl);

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, onlygood, FALSE, delta);
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
		if (delta)
			DeltaAddItem(ii);
		numitems++;
	}
}

void SetupAllUseful(int ii, int iseed, int lvl)
{
	int idx;

	item[ii]._iSeed = iseed;
	SetRndSeed(iseed);

#ifdef HELLFIRE
	idx = random_(34, 7);
	switch (idx) {
	case 0:
		idx = lvl <= 1 ? IDI_HEAL : IDI_PORTAL;
		break;
	case 1:
	case 2:
		idx = IDI_HEAL;
		break;
	case 3:
		idx = lvl <= 1 ? IDI_MANA : IDI_PORTAL;
		break;
	case 4:
	case 5:
		idx = IDI_MANA;
		break;
	default:
		idx = IDI_OIL;
		break;
	}
#else
	if (random_(34, 2))
		idx = IDI_HEAL;
	else
		idx = IDI_MANA;

	if (lvl > 1 && random_(34, 3) == 0)
		idx = IDI_PORTAL;
#endif

	GetItemAttrs(ii, idx, lvl);
	item[ii]._iCreateInfo = lvl + CF_USEFUL;
	SetupItem(ii);
}

void CreateRndUseful(int pnum, int x, int y, BOOL sendmsg)
{
	int ii, lvl;

	lvl = items_get_currlevel();

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		SetupAllUseful(ii, GetRndSeed(), lvl);
		if (sendmsg) {
			NetSendCmdDItem(FALSE, ii);
		}
		numitems++;
	}
}

void CreateTypeItem(int x, int y, BOOL onlygood, int itype, int imisc, BOOL sendmsg, BOOL delta)
{
	int idx, ii, lvl;

	lvl = items_get_currlevel();
	lvl <<= 1;

	if (itype != ITYPE_GOLD)
		idx = RndTypeItems(itype, imisc, lvl);
	else
		idx = IDI_GOLD;

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, onlygood, FALSE, delta);

		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
		if (delta)
			DeltaAddItem(ii);

		numitems++;
	}
}

void RecreateItem(int ii, int idx, WORD icreateinfo, int iseed, int ivalue)
{
	ItemStruct *is;
	int uper;
	BOOL onlygood, recreate, pregen;

	is = &item[ii];
	if (idx == 0) {
		SetPlrHandItem(is, IDI_GOLD);
		is->_iSeed = iseed;
		is->_iCreateInfo = icreateinfo;
		SetGoldItemValue(is, ivalue);
	} else {
		if (icreateinfo == 0) {
			SetPlrHandItem(is, idx);
			is->_iSeed = iseed;
		} else {
			if (icreateinfo & CF_TOWN) {
				RecreateTownItem(ii, idx, icreateinfo, iseed, ivalue);
			} else if ((icreateinfo & CF_USEFUL) == CF_USEFUL) {
				SetupAllUseful(ii, iseed, icreateinfo & CF_LEVEL);
			} else {
				uper = 0;
				onlygood = FALSE;
				recreate = FALSE;
				pregen = FALSE;
				if (icreateinfo & CF_UPER1)
					uper = 1;
				if (icreateinfo & CF_UPER15)
					uper = 15;
				if (icreateinfo & CF_ONLYGOOD)
					onlygood = TRUE;
				if (icreateinfo & CF_UNIQUE)
					recreate = TRUE;
				if (icreateinfo & CF_PREGEN)
					pregen = TRUE;
				SetupAllItems(ii, idx, iseed, icreateinfo & CF_LEVEL, uper, onlygood, recreate, pregen);
			}
		}
	}
}

void RecreateEar(int ii, WORD ic, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, int ibuff)
{
	SetPlrHandItem(&item[ii], IDI_EAR);
	tempstr[0] = (ic >> 8) & 0x7F;
	tempstr[1] = ic & 0x7F;
	tempstr[2] = (iseed >> 24) & 0x7F;
	tempstr[3] = (iseed >> 16) & 0x7F;
	tempstr[4] = (iseed >> 8) & 0x7F;
	tempstr[5] = iseed & 0x7F;
	tempstr[6] = Id & 0x7F;
	tempstr[7] = dur & 0x7F;
	tempstr[8] = mdur & 0x7F;
	tempstr[9] = ch & 0x7F;
	tempstr[10] = mch & 0x7F;
	tempstr[11] = (ivalue >> 8) & 0x7F;
	tempstr[12] = (ibuff >> 24) & 0x7F;
	tempstr[13] = (ibuff >> 16) & 0x7F;
	tempstr[14] = (ibuff >> 8) & 0x7F;
	tempstr[15] = ibuff & 0x7F;
	tempstr[16] = '\0';
	sprintf(item[ii]._iName, "Ear of %s", tempstr);
	item[ii]._iCurs = ((ivalue >> 6) & 3) + ICURS_EAR_SORCEROR;
	item[ii]._ivalue = ivalue & 0x3F;
	item[ii]._iCreateInfo = ic;
	item[ii]._iSeed = iseed;
}

#ifdef HELLFIRE
void SaveCornerStone()
{
	PkItemStruct id;
	if (CornerStone.activated) {
		if (CornerStone.item.IDidx >= 0) {
			PackItem(&id, &CornerStone.item);
			setIniValue("Hellfire", off_4A5AC4, (char *)&id, 19);
		} else {
			setIniValue("Hellfire", off_4A5AC4, (char *)"", 1);
		}
	}
}

void LoadCornerStone(int x, int y)
{
	int i, ii;
	int dwSize;
	PkItemStruct PkSItem;

	if (CornerStone.activated || x == 0 || y == 0) {
		return;
	}

	CornerStone.item.IDidx = 0;
	CornerStone.activated = TRUE;
	ii = dItem[x][y];
	if (ii != 0) {
		ii--;
		for (i = 0; i < numitems; i++) {
			if (itemactive[i] == ii) {
				DeleteItem(ii, i);
				break;
			}
		}
		dItem[x][y] = 0;
	}
	dwSize = 0;
	if (getIniValue("Hellfire", off_4A5AC4, (char *)&PkSItem, sizeof(PkSItem), &dwSize)) {
		if (dwSize == sizeof(PkSItem)) {
			ii = itemavail[0];
			dItem[x][y] = ii + 1;
			itemavail[0] = itemavail[MAXITEMS - numitems - 1];
			itemactive[numitems] = ii;
			UnPackItem(&PkSItem, &item[ii]);
			item[ii]._ix = x;
			item[ii]._iy = y;
			RespawnItem(ii, FALSE);
			CornerStone.item = item[ii];
			numitems++;
		}
	}
}
#endif

void SpawnQuestItem(int itemid, int x, int y, int randarea, int selflag)
{
	BOOL failed;
	int i, j, tries, lvl;

	lvl = items_get_currlevel();
	if (randarea != 0) {
		tries = 0;
		while (1) {
			tries++;
			if (tries > 1000 && randarea > 1)
				randarea--;
			x = random_(0, MAXDUNX);
			y = random_(0, MAXDUNY);
			failed = FALSE;
			for (i = x; i < x + randarea && !failed; i++) {
				for (j = y; j < y + randarea && !failed; j++) {
					failed = !ItemSpaceOk(i, j);
				}
			}
			if (!failed)
				break;
		}
	}

	if (numitems < MAXITEMS) {
		i = itemavail[0];
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = i;
		item[i]._ix = x;
		item[i]._iy = y;
		dItem[x][y] = i + 1;
		GetItemAttrs(i, itemid, lvl);
		SetupItem(i);
		item[i]._iPostDraw = TRUE;
		if (selflag != 0) {
			item[i]._iSelFlag = selflag;
			item[i]._iAnimFrame = item[i]._iAnimLen;
			item[i]._iAnimFlag = FALSE;
		}
		numitems++;
	}
}

void SpawnRock()
{
	int i, oi, lvl;
	int xx, yy;

	for (i = 0; i < nobjects; i++) {
		oi = objectactive[i];
		if (object[oi]._otype == OBJ_STAND)
			break;
	}
	if (i != nobjects) {
		lvl = items_get_currlevel();

		i = itemavail[0];
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = i;
		xx = object[oi]._ox;
		yy = object[oi]._oy;
		item[i]._ix = xx;
		item[i]._iy = yy;
		dItem[xx][yy] = i + 1;
		GetItemAttrs(i, IDI_ROCK, lvl);
		SetupItem(i);
		item[i]._iSelFlag = 2;
		item[i]._iPostDraw = TRUE;
		item[i]._iAnimFrame = 11;
		numitems++;
	}
}

#ifdef HELLFIRE
void SpawnRewardItem(int itemid, int xx, int yy)
{
	int i, lvl;

	lvl = items_get_currlevel();

	i = itemavail[0];
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = i;
	item[i]._ix = xx;
	item[i]._iy = yy;
	dItem[xx][yy] = i + 1;
	GetItemAttrs(i, itemid, lvl);
	SetupItem(i);
	item[i]._iSelFlag = 2;
	item[i]._iPostDraw = TRUE;
	item[i]._iAnimFrame = 1;
	item[i]._iAnimFlag = TRUE;
	item[i]._iIdentified = TRUE;
	numitems++;
}

void SpawnMapOfDoom(int xx, int yy)
{
	SpawnRewardItem(IDI_MAPOFDOOM, xx, yy);
}

void SpawnRuneBomb(int xx, int yy)
{
	SpawnRewardItem(IDI_RUNEBOMB, xx, yy);
}

void SpawnTheodore(int xx, int yy)
{
	SpawnRewardItem(IDI_THEODORE, xx, yy);
}
#endif

void RespawnItem(int ii, BOOL FlipFlag)
{
	ItemStruct *is;
	int it;

	is = &item[ii];
	it = ItemCAnimTbl[is->_iCurs];
	is->_iAnimData = itemanims[it];
	is->_iAnimLen = ItemAnimLs[it];
	is->_iAnimWidth = 96;
	is->_iAnimWidth2 = 16;
	is->_iPostDraw = FALSE;
	is->_iRequest = FALSE;
	if (FlipFlag) {
		is->_iAnimFrame = 1;
		is->_iAnimFlag = TRUE;
		is->_iSelFlag = 0;
	} else {
		is->_iAnimFrame = is->_iAnimLen;
		is->_iAnimFlag = FALSE;
		is->_iSelFlag = 1;
	}

	if (is->_iCurs == ICURS_MAGIC_ROCK) {
		is->_iSelFlag = 1;
		PlaySfxLoc(ItemDropSnds[it], is->_ix, is->_iy);
	}
	if (is->_iCurs == ICURS_TAVERN_SIGN)
		is->_iSelFlag = 1;
	if (is->_iCurs == ICURS_ANVIL_OF_FURY)
		is->_iSelFlag = 1;
}

void DeleteItem(int ii, int i)
{
	itemavail[MAXITEMS - numitems] = ii;
	numitems--;
	if (numitems > 0 && i != numitems)
		itemactive[i] = itemactive[numitems];
}

void ItemDoppel()
{
	int i;
	ItemStruct *is;

	if (gbMaxPlayers != 1) {
		for (i = DBORDERX; i < DSIZEX + DBORDERX; i++) {
			if (dItem[i][idoppely] != 0) {
				is = &item[dItem[i][idoppely] - 1];
				if (is->_ix != i || is->_iy != idoppely)
					dItem[i][idoppely] = 0;
			}
		}
		idoppely++;
		if (idoppely == DSIZEY + DBORDERY)
			idoppely = DBORDERY;
	}
}

void ProcessItems()
{
	ItemStruct *is;
	int i;

	for (i = 0; i < numitems; i++) {
		is = &item[itemactive[i]];
		if (is->_iAnimFlag) {
			is->_iAnimFrame++;
			if (is->_iCurs == ICURS_MAGIC_ROCK) {
				if (is->_iSelFlag == 1 && is->_iAnimFrame == 11)
					is->_iAnimFrame = 1;
				if (is->_iSelFlag == 2 && is->_iAnimFrame == 21)
					is->_iAnimFrame = 11;
			} else {
				if (is->_iAnimFrame == is->_iAnimLen >> 1)
					PlaySfxLoc(ItemDropSnds[ItemCAnimTbl[is->_iCurs]], is->_ix, is->_iy);

				if (is->_iAnimFrame >= is->_iAnimLen) {
					is->_iAnimFrame = is->_iAnimLen;
					is->_iAnimFlag = FALSE;
					is->_iSelFlag = 1;
				}
			}
		}
	}
	ItemDoppel();
}

void FreeItemGFX()
{
	int i;

	for (i = 0; i < ITEMTYPES; i++) {
		MemFreeDbg(itemanims[i]);
	}
}

void GetItemFrm(int ii)
{
	item[ii]._iAnimData = itemanims[ItemCAnimTbl[item[ii]._iCurs]];
}

void GetItemStr(int ii)
{
	ItemStruct *is;

	is = &item[ii];
	if (is->_itype != ITYPE_GOLD) {
		if (is->_iIdentified)
			strcpy(infostr, is->_iIName);
		else
			strcpy(infostr, is->_iName);

		if (is->_iMagical == ITEM_QUALITY_MAGIC)
			infoclr = COL_BLUE;
		if (is->_iMagical == ITEM_QUALITY_UNIQUE)
			infoclr = COL_GOLD;
	} else {
		sprintf(infostr, "%i gold %s", is->_ivalue, get_pieces_str(is->_ivalue));
	}
}

void CheckIdentify(int pnum, int cii)
{
	ItemStruct *pi;

	if (cii >= NUM_INVLOC)
		pi = &plr[pnum].InvList[cii - NUM_INVLOC];
	else
		pi = &plr[pnum].InvBody[cii];

	pi->_iIdentified = TRUE;
	CalcPlrInv(pnum, TRUE);
}

static void RepairItem(ItemStruct *is, int lvl)
{
	int rep, d, md;

	md = is->_iMaxDur;
	if (is->_iDurability == md) {
		return;
	}

	if (md <= 0) {
		is->_itype = ITYPE_NONE;
		return;
	}

	rep = 0;
	do {
		rep += lvl + random_(37, lvl);
		d = md / (lvl + 9);
		if (d < 1)
			d = 1;
		md -= d;
		if (md == 0) {
			is->_iMaxDur = 0;
			is->_itype = ITYPE_NONE;
			return;
		}
	} while (rep + is->_iDurability < md);

	is->_iDurability += rep;
	if (is->_iDurability > md)
		is->_iDurability = md;
	is->_iMaxDur = md;
}

void DoRepair(int pnum, int cii)
{
	PlayerStruct *p;
	ItemStruct *pi;

	p = &plr[pnum];
	PlaySfxLoc(IS_REPAIR, p->_px, p->_py);

	if (cii >= NUM_INVLOC) {
		pi = &p->InvList[cii - NUM_INVLOC];
	} else {
		pi = &p->InvBody[cii];
	}

	RepairItem(pi, p->_pLevel);
	CalcPlrInv(pnum, TRUE);
}

static void RechargeItem(ItemStruct *is, int r)
{
	int mc;

	mc = is->_iMaxCharges;
	if (is->_iCharges != mc) {
		do {
			mc--;
			if (mc == 0) {
				is->_iMaxCharges = 0;
				return;
			}
			is->_iCharges += r;
		} while (is->_iCharges < mc);
		if (is->_iCharges > mc)
			is->_iCharges = mc;
		is->_iMaxCharges = mc;
	}
}

void DoRecharge(int pnum, int cii)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int r;

	p = &plr[pnum];
	if (cii >= NUM_INVLOC) {
		pi = &p->InvList[cii - NUM_INVLOC];
	} else {
		pi = &p->InvBody[cii];
	}
	if (pi->_itype == ITYPE_STAFF && pi->_iSpell) {
		r = spelldata[pi->_iSpell].sBookLvl;
		r = random_(38, p->_pLevel / r) + 1;
		RechargeItem(pi, r);
		CalcPlrInv(pnum, TRUE);
	}
}

#ifdef HELLFIRE
BOOL DoOil(int pnum, int cii)
{
	PlayerStruct *p;
	ItemStruct *is;
	int dur, r;

	p = &plr[pnum];
	is = &p->InvBody[cii];
	switch (p->_pOilType) {
	case IMISC_OILACC:
	case IMISC_OILMAST:
	case IMISC_OILSHARP:
		if (is->_iClass != ICLASS_WEAPON)
			return FALSE;
		break;
	case IMISC_OILDEATH:
		if (is->_iClass != ICLASS_WEAPON || is->_itype == ITYPE_BOW)
			return FALSE;
		break;
	case IMISC_OILSKILL:
	case IMISC_OILBSMTH:
	case IMISC_OILFORT:
	case IMISC_OILPERM:
		if (is->_iClass != ICLASS_WEAPON && is->_iClass != ICLASS_ARMOR)
			return FALSE;
		break;
	case IMISC_OILHARD:
	case IMISC_OILIMP:
		if (is->_iClass != ICLASS_ARMOR) {
			return FALSE;
		}
		break;
	}

	switch (p->_pOilType) {
	case IMISC_OILACC:
		if (is->_iPLToHit < 50) {
			is->_iPLToHit += random_(68, 2) + 1;
		}
		break;
	case IMISC_OILMAST:
		if (is->_iPLToHit < 100) {
			is->_iPLToHit += random_(68, 3) + 3;
		}
		break;
	case IMISC_OILSHARP:
		if (is->_iMaxDam - is->_iMinDam < 30) {
			is->_iMaxDam++;
		}
		break;
	case IMISC_OILDEATH:
		if (is->_iMaxDam - is->_iMinDam < 30) {
			is->_iMinDam++;
			is->_iMaxDam += 2;
		}
		break;
	case IMISC_OILSKILL:
		r = random_(68, 6) + 5;
		if (is->_iMinStr > r) {
			is->_iMinStr -= r;
		} else {
			is->_iMinStr = 0;
		}
		if (is->_iMinMag > r) {
			is->_iMinMag -= r;
		} else {
			is->_iMinMag = 0;
		}
		if (is->_iMinDex > r) {
			is->_iMinDex -= r;
		} else {
			is->_iMinDex = 0;
		}
		break;
	case IMISC_OILBSMTH:
		dur = is->_iMaxDur;
		if (dur != DUR_INDESTRUCTIBLE) {
			if (is->_iDurability < dur) {
				r = (dur + 4) / 5 + is->_iDurability;
				if (r > dur)
					r = dur;
			} else {
				if (dur >= 100)
					break;
				r = dur + 1;
				is->_iMaxDur = r;
			}
			is->_iDurability = r;
		}
		break;
	case IMISC_OILFORT:
		if (is->_iMaxDur != DUR_INDESTRUCTIBLE && is->_iMaxDur < 200) {
			r = random_(68, 41) + 10;
			is->_iMaxDur += r;
			is->_iDurability += r;
		}
		break;
	case IMISC_OILPERM:
		is->_iDurability = DUR_INDESTRUCTIBLE;
		is->_iMaxDur = DUR_INDESTRUCTIBLE;
		break;
	case IMISC_OILHARD:
		if (is->_iAC < 60) {
			is->_iAC += random_(68, 2) + 1;
		}
		break;
	case IMISC_OILIMP:
		if (is->_iAC < 120) {
			is->_iAC += random_(68, 3) + 3;
		}
		break;
	}

	CalcPlrInv(pnum, TRUE);
	return TRUE;
}

#endif

void PrintItemPower(char plidx, const ItemStruct *is)
{
	switch (plidx) {
	case IPL_TOHIT:
	case IPL_TOHIT_CURSE:
		sprintf(tempstr, "chance to hit: %+i%%", is->_iPLToHit);
		break;
	case IPL_DAMP:
	case IPL_DAMP_CURSE:
		sprintf(tempstr, "%+i%% damage", is->_iPLDam);
		break;
	case IPL_TOHIT_DAMP:
	case IPL_TOHIT_DAMP_CURSE:
#ifdef HELLFIRE
	case IPL_DOPPELGANGER:
#endif
		sprintf(tempstr, "to hit: %+i%%, %+i%% damage", is->_iPLToHit, is->_iPLDam);
		break;
	case IPL_ACP:
	case IPL_ACP_CURSE:
		sprintf(tempstr, "%+i%% armor", is->_iPLAC);
		break;
	case IPL_SETAC:
	case IPL_AC_CURSE:
		sprintf(tempstr, "armor class: %i", is->_iAC);
		break;
	case IPL_FIRERES:
#ifdef HELLFIRE
	case IPL_FIRERES_CURSE:
#endif
		if (is->_iPLFR < 75)
			sprintf(tempstr, "Resist Fire: %+i%%", is->_iPLFR);
		else
			sprintf(tempstr, "Resist Fire: 75%% MAX");
		break;
	case IPL_LIGHTRES:
#ifdef HELLFIRE
	case IPL_LIGHTRES_CURSE:
#endif
		if (is->_iPLLR < 75)
			sprintf(tempstr, "Resist Lightning: %+i%%", is->_iPLLR);
		else
			sprintf(tempstr, "Resist Lightning: 75%% MAX");
		break;
	case IPL_MAGICRES:
#ifdef HELLFIRE
	case IPL_MAGICRES_CURSE:
#endif
		if (is->_iPLMR < 75)
			sprintf(tempstr, "Resist Magic: %+i%%", is->_iPLMR);
		else
			sprintf(tempstr, "Resist Magic: 75%% MAX");
		break;
	case IPL_ALLRES:
#ifdef HELLFIRE
	case IPL_ALLRES_CURSE:
#endif
		if (is->_iPLFR < 75)
			sprintf(tempstr, "Resist All: %+i%%", is->_iPLFR);
		else
			sprintf(tempstr, "Resist All: 75%% MAX");
		break;
	case IPL_SPLLVLADD:
		if (is->_iSplLvlAdd == 1)
			strcpy(tempstr, "spells are increased 1 level");
#ifdef HELLFIRE
		if (is->_iSplLvlAdd > 1)
			sprintf(tempstr, "spells are increased %i levels", is->_iSplLvlAdd);
#else
		if (is->_iSplLvlAdd == 2)
			strcpy(tempstr, "spells are increased 2 levels");
#endif
		if (is->_iSplLvlAdd < 1)
			strcpy(tempstr, "spells are decreased 1 level");
#ifdef HELLFIRE
		if (is->_iSplLvlAdd < -1)
			sprintf(tempstr, "spells are decreased %i levels", is->_iSplLvlAdd);
		if (is->_iSplLvlAdd == 0)
			strcpy(tempstr, "spell levels unchanged (?)");
#endif
		break;
	case IPL_CHARGES:
		strcpy(tempstr, "Extra charges");
		break;
	case IPL_SPELL:
		sprintf(tempstr, "%i %s charges", is->_iMaxCharges, spelldata[is->_iSpell].sNameText);
		break;
	case IPL_FIREDAM:
#ifdef HELLFIRE
		if (is->_iFMinDam == is->_iFMaxDam)
			sprintf(tempstr, "Fire hit damage: %i", is->_iFMinDam);
		else
#endif
			sprintf(tempstr, "Fire hit damage: %i-%i", is->_iFMinDam, is->_iFMaxDam);
		break;
	case IPL_LIGHTDAM:
#ifdef HELLFIRE
		if (is->_iLMinDam == is->_iLMaxDam)
			sprintf(tempstr, "Lightning hit damage: %i", is->_iLMinDam);
		else
#endif
			sprintf(tempstr, "Lightning hit damage: %i-%i", is->_iLMinDam, is->_iLMaxDam);
		break;
	case IPL_STR:
	case IPL_STR_CURSE:
		sprintf(tempstr, "%+i to strength", is->_iPLStr);
		break;
	case IPL_MAG:
	case IPL_MAG_CURSE:
		sprintf(tempstr, "%+i to magic", is->_iPLMag);
		break;
	case IPL_DEX:
	case IPL_DEX_CURSE:
		sprintf(tempstr, "%+i to dexterity", is->_iPLDex);
		break;
	case IPL_VIT:
	case IPL_VIT_CURSE:
		sprintf(tempstr, "%+i to vitality", is->_iPLVit);
		break;
	case IPL_ATTRIBS:
	case IPL_ATTRIBS_CURSE:
		sprintf(tempstr, "%+i to all attributes", is->_iPLStr);
		break;
	case IPL_GETHIT_CURSE:
	case IPL_GETHIT:
		sprintf(tempstr, "%+i damage from enemies", is->_iPLGetHit);
		break;
	case IPL_LIFE:
	case IPL_LIFE_CURSE:
		sprintf(tempstr, "Hit Points: %+i", is->_iPLHP >> 6);
		break;
	case IPL_MANA:
	case IPL_MANA_CURSE:
		sprintf(tempstr, "Mana: %+i", is->_iPLMana >> 6);
		break;
	case IPL_DUR:
		strcpy(tempstr, "high durability");
		break;
	case IPL_DUR_CURSE:
		strcpy(tempstr, "decreased durability");
		break;
	case IPL_INDESTRUCTIBLE:
		strcpy(tempstr, "indestructible");
		break;
	case IPL_LIGHT:
		sprintf(tempstr, "+%i%% light radius", 10 * is->_iPLLight);
		break;
	case IPL_LIGHT_CURSE:
		sprintf(tempstr, "-%i%% light radius", -10 * is->_iPLLight);
		break;
#ifdef HELLFIRE
	case IPL_MULT_ARROWS:
		strcpy(tempstr, "multiple arrows per shot");
		break;
#endif
	case IPL_FIRE_ARROWS:
#ifdef HELLFIRE
		if (is->_iFMinDam == is->_iFMaxDam)
			sprintf(tempstr, "fire arrows damage: %i", is->_iFMinDam);
		else
#endif
			sprintf(tempstr, "fire arrows damage: %i-%i", is->_iFMinDam, is->_iFMaxDam);
		break;
	case IPL_LIGHT_ARROWS:
#ifdef HELLFIRE
		if (is->_iLMinDam == is->_iLMaxDam)
			sprintf(tempstr, "lightning arrows damage %i", is->_iLMinDam);
		else
#endif
			sprintf(tempstr, "lightning arrows damage %i-%i", is->_iLMinDam, is->_iLMaxDam);
		break;
	case IPL_THORNS:
		strcpy(tempstr, "attacker takes 1-3 damage");
		break;
	case IPL_NOMANA:
		strcpy(tempstr, "user loses all mana");
		break;
	case IPL_NOHEALPLR:
		strcpy(tempstr, "you can't heal");
		break;
#ifdef HELLFIRE
	case IPL_FIREBALL:
		if (is->_iFMinDam != is->_iFMaxDam)
			sprintf(tempstr, "fireball damage: %i-%i", is->_iFMinDam, is->_iFMaxDam);
		else
			sprintf(tempstr, "fireball damage: %i", is->_iFMinDam);
		break;
#endif
	case IPL_ABSHALFTRAP:
		strcpy(tempstr, "absorbs half of trap damage");
		break;
	case IPL_KNOCKBACK:
		strcpy(tempstr, "knocks target back");
		break;
	case IPL_3XDAMVDEM:
		strcpy(tempstr, "+200% damage vs. demons");
		break;
	case IPL_ALLRESZERO:
		strcpy(tempstr, "All Resistance equals 0");
		break;
	case IPL_NOHEALMON:
		strcpy(tempstr, "hit monster doesn't heal");
		break;
	case IPL_STEALMANA:
		if (is->_iFlags & ISPL_STEALMANA_3)
			strcpy(tempstr, "hit steals 3% mana");
		if (is->_iFlags & ISPL_STEALMANA_5)
			strcpy(tempstr, "hit steals 5% mana");
		break;
	case IPL_STEALLIFE:
		if (is->_iFlags & ISPL_STEALLIFE_3)
			strcpy(tempstr, "hit steals 3% life");
		if (is->_iFlags & ISPL_STEALLIFE_5)
			strcpy(tempstr, "hit steals 5% life");
		break;
	case IPL_TARGAC:
#ifdef HELLFIRE
		strcpy(tempstr, "penetrates target\'s armor");
#else
		strcpy(tempstr, "damages target's armor");
#endif
		break;
	case IPL_FASTATTACK:
		if (is->_iFlags & ISPL_QUICKATTACK)
			strcpy(tempstr, "quick attack");
		if (is->_iFlags & ISPL_FASTATTACK)
			strcpy(tempstr, "fast attack");
		if (is->_iFlags & ISPL_FASTERATTACK)
			strcpy(tempstr, "faster attack");
		if (is->_iFlags & ISPL_FASTESTATTACK)
			strcpy(tempstr, "fastest attack");
		break;
	case IPL_FASTRECOVER:
		if (is->_iFlags & ISPL_FASTRECOVER)
			strcpy(tempstr, "fast hit recovery");
		if (is->_iFlags & ISPL_FASTERRECOVER)
			strcpy(tempstr, "faster hit recovery");
		if (is->_iFlags & ISPL_FASTESTRECOVER)
			strcpy(tempstr, "fastest hit recovery");
		break;
	case IPL_FASTBLOCK:
		strcpy(tempstr, "fast block");
		break;
	case IPL_DAMMOD:
		sprintf(tempstr, "adds %i points to damage", is->_iPLDamMod);
		break;
	case IPL_RNDARROWVEL:
		strcpy(tempstr, "fires random speed arrows");
		break;
	case IPL_SETDAM:
		sprintf(tempstr, "unusual item damage");
		break;
	case IPL_SETDUR:
		strcpy(tempstr, "altered durability");
		break;
	case IPL_FASTSWING:
		strcpy(tempstr, "Faster attack swing");
		break;
	case IPL_ONEHAND:
		strcpy(tempstr, "one handed sword");
		break;
	case IPL_DRAINLIFE:
		strcpy(tempstr, "constantly lose hit points");
		break;
	case IPL_RNDSTEALLIFE:
		strcpy(tempstr, "life stealing");
		break;
	case IPL_NOMINSTR:
		strcpy(tempstr, "no strength requirement");
		break;
	case IPL_INFRAVISION:
		strcpy(tempstr, "see with infravision");
		break;
	case IPL_INVCURS:
		strcpy(tempstr, " ");
		break;
	case IPL_ADDACLIFE:
#ifdef HELLFIRE
		if (is->_iFMinDam != is->_iFMaxDam)
			sprintf(tempstr, "lightning: %i-%i", is->_iFMinDam, is->_iFMaxDam);
		else
			sprintf(tempstr, "lightning damage: %i", is->_iFMinDam);
#else
		strcpy(tempstr, "Armor class added to life");
#endif
		break;
	case IPL_ADDMANAAC:
#ifdef HELLFIRE
		strcpy(tempstr, "charged bolts on hits");
#else
		strcpy(tempstr, "10% of mana added to armor");
#endif
		break;
	case IPL_FIRERESCLVL:
		if (is->_iPLFR <= 0)
			sprintf(tempstr, " ");
		else
			sprintf(tempstr, "Resist Fire: %+i%%", is->_iPLFR);
		break;
#ifdef HELLFIRE
	case IPL_DEVASTATION:
		strcpy(tempstr, "occasional triple damage");
		break;
	case IPL_DECAY:
		sprintf(tempstr, "decaying %+i%% damage", is->_iPLDam);
		break;
	case IPL_PERIL:
		strcpy(tempstr, "2x dmg to monst, 1x to you");
		break;
	case IPL_JESTERS:
		strcpy(tempstr, "Random 0 - 500% damage");
		break;
	case IPL_CRYSTALLINE:
		sprintf(tempstr, "low dur, %+i%% damage", is->_iPLDam);
		break;
	case IPL_ACDEMON:
		strcpy(tempstr, "extra AC vs demons");
		break;
	case IPL_ACUNDEAD:
		strcpy(tempstr, "extra AC vs undead");
		break;
	case IPL_MANATOLIFE:
		strcpy(tempstr, "50%% Mana moved to Health");
		break;
	case IPL_LIFETOMANA:
		strcpy(tempstr, "40%% Health moved to Mana");
		break;
#endif
	default:
		strcpy(tempstr, "Another ability (NW)");
		break;
	}
}

void DrawUTextBack()
{
	CelDraw(RIGHT_PANEL_X - SPANEL_WIDTH + 24, SCREEN_Y + 327, pSTextBoxCels, 1, 271);
	trans_rect(RIGHT_PANEL - SPANEL_WIDTH + 27, 28, 265, 297);
}

void PrintUString(int x, int y, BOOL cjustflag, const char *str, int col)
{
	int len, width, sx, sy, i, k;
	BYTE c;

	sx = x + 32 + SCREEN_X;
	sy = y * 12 + 44 + SCREEN_Y;
	len = strlen(str);
	k = 0;
	if (cjustflag) {
		width = 0;
		for (i = 0; i < len; i++)
			width += fontkern[fontframe[gbFontTransTbl[(BYTE)str[i]]]] + 1;
		if (width < 257)
			k = (257 - width) >> 1;
		sx += k;
	}

	for (i = 0; i < len; i++) {
		c = fontframe[gbFontTransTbl[(BYTE)str[i]]];
		k += fontkern[c] + 1;
		if (c != '\0' && k <= 257) {
			PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + 1;
	}
}

void DrawULine(int y)
{
	assert(gpBuffer);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[SCREENXY(26 + RIGHT_PANEL - SPANEL_WIDTH, 25)];
	dst = &gpBuffer[BUFFER_WIDTH * (y * 12 + 38 + SCREEN_Y) + 26 + RIGHT_PANEL_X - SPANEL_WIDTH];

	for (i = 0; i < 3; i++, src += BUFFER_WIDTH, dst += BUFFER_WIDTH)
		memcpy(dst, src, 266); // BUGFIX: should be 267
}

void DrawUniqueInfo()
{
	const UItemStruct *uis;
	int y;

	if ((!chrflag && !questlog) || SCREEN_WIDTH >= SPANEL_WIDTH * 3) {
		uis = &UniqueItemList[curruitem._iUid];
		DrawUTextBack();
		PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, 2, TRUE, uis->UIName, 3);
		DrawULine(5);
		PrintItemPower(uis->UIPower1, &curruitem);
		y = 6 - uis->UINumPL + 8;
		PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, y, TRUE, tempstr, 0);
		if (uis->UINumPL > 1) {
			PrintItemPower(uis->UIPower2, &curruitem);
			PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, y + 2, TRUE, tempstr, 0);
		}
		if (uis->UINumPL > 2) {
			PrintItemPower(uis->UIPower3, &curruitem);
			PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, y + 4, TRUE, tempstr, 0);
		}
		if (uis->UINumPL > 3) {
			PrintItemPower(uis->UIPower4, &curruitem);
			PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, y + 6, TRUE, tempstr, 0);
		}
		if (uis->UINumPL > 4) {
			PrintItemPower(uis->UIPower5, &curruitem);
			PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, y + 8, TRUE, tempstr, 0);
		}
		if (uis->UINumPL > 5) {
			PrintItemPower(uis->UIPower6, &curruitem);
			PrintUString(0 + RIGHT_PANEL - SPANEL_WIDTH, y + 10, TRUE, tempstr, 0);
		}
	}
}

void PrintItemMisc(const ItemStruct *is)
{
	const char *desc;

	switch (is->_iMiscId) {
	case IMISC_NONE:
		return;
	case IMISC_USEFIRST:
		break;
	case IMISC_FULLHEAL:
		desc = "fully recover life";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_HEAL:
		desc = "recover partial life";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OLDHEAL:
		desc = "recover life";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_DEADHEAL:
		desc = "deadly heal";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_MANA:
		desc = "recover mana";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_FULLMANA:
		desc = "fully recover mana";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_POTEXP:
	case IMISC_POTFORG:
		break;
	case IMISC_ELIXSTR:
		desc = "increase strength";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXMAG:
		desc = "increase magic";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXDEX:
		desc = "increase dexterity";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXVIT:
		desc = "increase vitality";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXWEAK:
		desc = "decrease strength";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXDIS:
		desc = "decrease strength";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXCLUM:
		desc = "decrease dexterity";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_ELIXSICK:
		desc = "decrease vitality";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_REJUV:
		desc = "recover life and mana";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_FULLREJUV:
		desc = "fully recover life and mana";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_USELAST:
		break;
	case IMISC_SCROLL:
		desc = "Right-click to read";
		AddPanelString(desc, TRUE);
		return;
	case IMISC_SCROLLT:
		desc = "Right-click to read, then";
		AddPanelString(desc, TRUE);
		desc = "left-click to target";
		AddPanelString(desc, TRUE);
		return;
	case IMISC_STAFF:
		return;
	case IMISC_BOOK:
		desc = "Right-click to read";
		AddPanelString(desc, TRUE);
		return;
	case IMISC_RING:
	case IMISC_AMULET:
	case IMISC_UNIQUE:
	case IMISC_MEAT:
	case IMISC_OILFIRST:
		return;
#ifdef HELLFIRE
	case IMISC_OILOF:
		break;
	case IMISC_OILACC:
		desc = "increases a weapon\'s";
		AddPanelString(desc, TRUE);
		desc = "chance to hit";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILMAST:
		desc = "greatly increases a";
		AddPanelString(desc, TRUE);
		desc = "weapon\'s chance to hit";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILSHARP:
		desc = "increases a weapon\'s";
		AddPanelString(desc, TRUE);
		desc = "damage potential";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILDEATH:
		desc = "greatly increases a weapon\'s";
		AddPanelString(desc, TRUE);
		desc = "damage potential - not bows";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILSKILL:
		desc = "reduces attributes needed";
		AddPanelString(desc, TRUE);
		desc = "to use armor or weapons";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILBSMTH:
		desc = "restores 20% of an";
		AddPanelString(desc, TRUE);
		desc = "item\'s durability";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILFORT:
		desc = "increases an item\'s";
		AddPanelString(desc, TRUE);
		desc = "current and max durability";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILPERM:
		desc = "makes an item indestructible";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILHARD:
		desc = "increases the armor class";
		AddPanelString(desc, TRUE);
		desc = "of armor and shields";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILIMP:
		desc = "greatly increases the armor";
		AddPanelString(desc, TRUE);
		desc = "class of armor and shields";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_OILLAST:
		return;
	case IMISC_MAPOFDOOM:
		desc = "Right-click to view";
		AddPanelString(desc, TRUE);
		return;
#endif
	case IMISC_EAR:
		sprintf(tempstr, "Level : %i", is->_ivalue);
		AddPanelString(tempstr, TRUE);
		return;
	case IMISC_SPECELIX:
		return;
#ifdef HELLFIRE
	case IMISC_RUNEFIRST:
		return;
	case IMISC_RUNEF:
		desc = "sets fire trap";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_RUNEL:
		desc = "sets lightning trap";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_GR_RUNEL:
		desc = "sets lightning trap";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_GR_RUNEF:
		desc = "sets fire trap";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_RUNES:
		desc = "sets petrification trap";
		AddPanelString(desc, TRUE);
		break;
	case IMISC_RUNELAST:
		return;
	case IMISC_AURIC:
		desc = "Doubles gold capacity";
		AddPanelString(desc, TRUE);
		return;
	case IMISC_NOTE:
		desc = "Right click to read";
		AddPanelString(desc, TRUE);
		return;
#endif
	default:
		return;
	}

	desc = "Right click to use";
	AddPanelString(desc, TRUE);
	return;
}

void PrintItemDetails(const ItemStruct *is)
{
	if (is->_iClass == ICLASS_WEAPON) {
#ifdef HELLFIRE
		if (is->_iMinDam == is->_iMaxDam) {
			if (is->_iMaxDur == 255)
				sprintf(tempstr, "damage: %i  Indestructible", is->_iMinDam);
			else
				sprintf(tempstr, "damage: %i  Dur: %i/%i", is->_iMinDam, is->_iDurability, is->_iMaxDur);
		} else
#endif
		if (is->_iMaxDur == DUR_INDESTRUCTIBLE)
			sprintf(tempstr, "damage: %i-%i  Indestructible", is->_iMinDam, is->_iMaxDam);
		else
			sprintf(tempstr, "damage: %i-%i  Dur: %i/%i", is->_iMinDam, is->_iMaxDam, is->_iDurability, is->_iMaxDur);
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iClass == ICLASS_ARMOR) {
		if (is->_iMaxDur == DUR_INDESTRUCTIBLE)
			sprintf(tempstr, "armor: %i  Indestructible", is->_iAC);
		else
			sprintf(tempstr, "armor: %i  Dur: %i/%i", is->_iAC, is->_iDurability, is->_iMaxDur);
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iMiscId == IMISC_STAFF && is->_iMaxCharges) {
#ifdef HELLFIRE
		if (is->_iMinDam == is->_iMaxDam)
			sprintf(tempstr, "dam: %i  Dur: %i/%i", is->_iMinDam, is->_iDurability, is->_iMaxDur);
		else
#endif
		sprintf(tempstr, "dam: %i-%i  Dur: %i/%i", is->_iMinDam, is->_iMaxDam, is->_iDurability, is->_iMaxDur);
		sprintf(tempstr, "Charges: %i/%i", is->_iCharges, is->_iMaxCharges);
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iPrePower != -1) {
		PrintItemPower(is->_iPrePower, is);
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iSufPower != -1) {
		PrintItemPower(is->_iSufPower, is);
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iMagical == ITEM_QUALITY_UNIQUE) {
		AddPanelString("unique item", TRUE);
		uitemflag = TRUE;
		curruitem = *is;
	}
	PrintItemMisc(is);
	if ((is->_iMinMag | is->_iMinDex | is->_iMinStr) != 0) {
		strcpy(tempstr, "Required:");
		if (is->_iMinStr)
			sprintf(tempstr, "%s %i Str", tempstr, is->_iMinStr);
		if (is->_iMinMag)
			sprintf(tempstr, "%s %i Mag", tempstr, is->_iMinMag);
		if (is->_iMinDex)
			sprintf(tempstr, "%s %i Dex", tempstr, is->_iMinDex);
		AddPanelString(tempstr, TRUE);
	}
	pinfoflag = TRUE;
}

void PrintItemDur(const ItemStruct *is)
{
	if (is->_iClass == ICLASS_WEAPON) {
#ifdef HELLFIRE
		if (is->_iMinDam == is->_iMaxDam) {
			if (is->_iMaxDur == 255)
				sprintf(tempstr, "damage: %i  Indestructible", is->_iMinDam);
			else
				sprintf(tempstr, "damage: %i  Dur: %i/%i", is->_iMinDam, is->_iDurability, is->_iMaxDur);
		} else
#endif
		if (is->_iMaxDur == DUR_INDESTRUCTIBLE)
			sprintf(tempstr, "damage: %i-%i  Indestructible", is->_iMinDam, is->_iMaxDam);
		else
			sprintf(tempstr, "damage: %i-%i  Dur: %i/%i", is->_iMinDam, is->_iMaxDam, is->_iDurability, is->_iMaxDur);
		AddPanelString(tempstr, TRUE);
		if (is->_iMiscId == IMISC_STAFF && is->_iMaxCharges) {
			sprintf(tempstr, "Charges: %i/%i", is->_iCharges, is->_iMaxCharges);
			AddPanelString(tempstr, TRUE);
		}
		if (is->_iMagical != ITEM_QUALITY_NORMAL)
			AddPanelString("Not Identified", TRUE);
	}
	if (is->_iClass == ICLASS_ARMOR) {
		if (is->_iMaxDur == DUR_INDESTRUCTIBLE)
			sprintf(tempstr, "armor: %i  Indestructible", is->_iAC);
		else
			sprintf(tempstr, "armor: %i  Dur: %i/%i", is->_iAC, is->_iDurability, is->_iMaxDur);
		AddPanelString(tempstr, TRUE);
		if (is->_iMagical != ITEM_QUALITY_NORMAL)
			AddPanelString("Not Identified", TRUE);
		if (is->_iMiscId == IMISC_STAFF && is->_iMaxCharges) {
			sprintf(tempstr, "Charges: %i/%i", is->_iCharges, is->_iMaxCharges);
			AddPanelString(tempstr, TRUE);
		}
	}
	if (is->_itype == ITYPE_RING || is->_itype == ITYPE_AMULET)
		AddPanelString("Not Identified", TRUE);
	PrintItemMisc(is);
	if ((is->_iMinStr | is->_iMinMag | is->_iMinDex) != 0) {
		strcpy(tempstr, "Required:");
		if (is->_iMinStr)
			sprintf(tempstr, "%s %i Str", tempstr, is->_iMinStr);
		if (is->_iMinMag)
			sprintf(tempstr, "%s %i Mag", tempstr, is->_iMinMag);
		if (is->_iMinDex)
			sprintf(tempstr, "%s %i Dex", tempstr, is->_iMinDex);
		AddPanelString(tempstr, TRUE);
	}
	pinfoflag = TRUE;
}

void UseItem(int pnum, int Mid, int spl)
{
	PlayerStruct *p;
	int l, j;

	switch (Mid) {
	case IMISC_HEAL:
	case IMISC_MEAT:
		p = &plr[pnum];
		j = p->_pMaxHP >> 8;
		l = ((j >> 1) + random_(39, j)) << 6;
		if (p->_pClass == PC_WARRIOR)
			l *= 2;
		if (p->_pClass == PC_ROGUE)
			l += l >> 1;
		p->_pHitPoints += l;
		if (p->_pHitPoints > p->_pMaxHP)
			p->_pHitPoints = p->_pMaxHP;
		p->_pHPBase += l;
		if (p->_pHPBase > p->_pMaxHPBase)
			p->_pHPBase = p->_pMaxHPBase;
		drawhpflag = TRUE;
		break;
	case IMISC_FULLHEAL:
		plr[pnum]._pHitPoints = plr[pnum]._pMaxHP;
		plr[pnum]._pHPBase = plr[pnum]._pMaxHPBase;
		drawhpflag = TRUE;
		break;
	case IMISC_MANA:
		p = &plr[pnum];
		if (!(p->_pIFlags & ISPL_NOMANA)) {
			j = p->_pMaxMana >> 8;
			l = ((j >> 1) + random_(40, j)) << 6;
			if (p->_pClass == PC_SORCERER)
				l *= 2;
			if (p->_pClass == PC_ROGUE)
				l += l >> 1;
			p->_pMana += l;
			if (p->_pMana > p->_pMaxMana)
				p->_pMana = p->_pMaxMana;
			p->_pManaBase += l;
			if (p->_pManaBase > p->_pMaxManaBase)
				p->_pManaBase = p->_pMaxManaBase;
			drawmanaflag = TRUE;
		}
		break;
	case IMISC_FULLMANA:
		if (!(plr[pnum]._pIFlags & ISPL_NOMANA)) {
			plr[pnum]._pMana = plr[pnum]._pMaxMana;
			plr[pnum]._pManaBase = plr[pnum]._pMaxManaBase;
			drawmanaflag = TRUE;
		}
		break;
	case IMISC_ELIXSTR:
		ModifyPlrStr(pnum, 1);
		break;
	case IMISC_ELIXMAG:
		ModifyPlrMag(pnum, 1);
		break;
	case IMISC_ELIXDEX:
		ModifyPlrDex(pnum, 1);
		break;
	case IMISC_ELIXVIT:
		ModifyPlrVit(pnum, 1);
		break;
	case IMISC_REJUV:
		p = &plr[pnum];
		j = p->_pMaxHP >> 8;
		l = ((j >> 1) + random_(39, j)) << 6;
		if (p->_pClass == PC_WARRIOR)
			l *= 2;
		if (p->_pClass == PC_ROGUE)
			l += l >> 1;
		p->_pHitPoints += l;
		if (p->_pHitPoints > p->_pMaxHP)
			p->_pHitPoints = p->_pMaxHP;
		p->_pHPBase += l;
		if (p->_pHPBase > p->_pMaxHPBase)
			p->_pHPBase = p->_pMaxHPBase;
		drawhpflag = TRUE;
		if (!(p->_pIFlags & ISPL_NOMANA)) {
			j = p->_pMaxMana >> 8;
			l = ((j >> 1) + random_(40, j)) << 6;
			if (p->_pClass == PC_SORCERER)
				l *= 2;
			if (p->_pClass == PC_ROGUE)
				l += l >> 1;
			p->_pMana += l;
			if (p->_pMana > p->_pMaxMana)
				p->_pMana = p->_pMaxMana;
			p->_pManaBase += l;
			if (p->_pManaBase > p->_pMaxManaBase)
				p->_pManaBase = p->_pMaxManaBase;
			drawmanaflag = TRUE;
		}
		break;
	case IMISC_FULLREJUV:
		p = &plr[pnum];
		p->_pHitPoints = p->_pMaxHP;
		p->_pHPBase = p->_pMaxHPBase;
		drawhpflag = TRUE;
		if (!(p->_pIFlags & ISPL_NOMANA)) {
			p->_pMana = p->_pMaxMana;
			p->_pManaBase = p->_pMaxManaBase;
			drawmanaflag = TRUE;
		}
		break;
	case IMISC_SCROLL:
		p = &plr[pnum];
		if (spelldata[spl].sTargeted) {
			p->_pTSpell = spl;
			p->_pTSplType = RSPLTYPE_INVALID;
			if (pnum == myplr)
				NewCursor(CURSOR_TELEPORT);
		} else {
			ClrPlrPath(pnum);
			p->_pSpell = spl;
			p->_pSplType = RSPLTYPE_INVALID;
			p->_pSplFrom = 3;
			p->destAction = ACTION_SPELL;
			p->destParam1 = cursmx;
			p->destParam2 = cursmy;
#ifndef HELLFIRE
			if (pnum == myplr && spl == SPL_NOVA)
				NetSendCmdLoc(TRUE, CMD_NOVA, cursmx, cursmy);
#endif
		}
		break;
	case IMISC_SCROLLT:
		p = &plr[pnum];
		if (spelldata[spl].sTargeted) {
			p->_pTSpell = spl;
			p->_pTSplType = RSPLTYPE_INVALID;
			if (pnum == myplr)
				NewCursor(CURSOR_TELEPORT);
		} else {
			ClrPlrPath(pnum);
			p->_pSpell = spl;
			p->_pSplType = RSPLTYPE_INVALID;
			p->_pSplFrom = 3;
			p->destAction = ACTION_SPELL;
			p->destParam1 = cursmx;
			p->destParam2 = cursmy;
		}
		break;
	case IMISC_BOOK:
		p = &plr[pnum];
		p->_pMemSpells |= (__int64)1 << (spl - 1);
		if (p->_pSplLvl[spl] < MAXSPLLEVEL)
			p->_pSplLvl[spl]++;
		p->_pMana += spelldata[spl].sManaCost << 6;
		if (p->_pMana > p->_pMaxMana)
			p->_pMana = p->_pMaxMana;
		p->_pManaBase += spelldata[spl].sManaCost << 6;
		if (p->_pManaBase > p->_pMaxManaBase)
			p->_pManaBase = p->_pMaxManaBase;
		if (pnum == myplr)
			CalcPlrBookVals(pnum);
		drawmanaflag = TRUE;
		break;
	case IMISC_MAPOFDOOM:
		doom_init();
		break;
#ifdef HELLFIRE
	case IMISC_OILACC:
	case IMISC_OILMAST:
	case IMISC_OILSHARP:
	case IMISC_OILDEATH:
	case IMISC_OILSKILL:
	case IMISC_OILBSMTH:
	case IMISC_OILFORT:
	case IMISC_OILPERM:
	case IMISC_OILHARD:
	case IMISC_OILIMP:
		plr[pnum]._pOilType = Mid;
		if (pnum != myplr) {
			return;
		}
		if (sbookflag) {
			sbookflag = FALSE;
		}
		if (!invflag) {
			invflag = TRUE;
		}
		NewCursor(CURSOR_OIL);
		break;
#endif
	case IMISC_SPECELIX:
		ModifyPlrStr(pnum, 3);
		ModifyPlrMag(pnum, 3);
		ModifyPlrDex(pnum, 3);
		ModifyPlrVit(pnum, 3);
		break;
#ifdef HELLFIRE
	case IMISC_RUNEF:
		plr[pnum]._pTSpell = SPL_RUNEFIRE;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			NewCursor(CURSOR_TELEPORT);
		break;
	case IMISC_RUNEL:
		plr[pnum]._pTSpell = SPL_RUNELIGHT;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			NewCursor(CURSOR_TELEPORT);
		break;
	case IMISC_GR_RUNEL:
		plr[pnum]._pTSpell = SPL_RUNENOVA;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			NewCursor(CURSOR_TELEPORT);
		break;
	case IMISC_GR_RUNEF:
		plr[pnum]._pTSpell = SPL_RUNEIMMOLAT;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			NewCursor(CURSOR_TELEPORT);
		break;
	case IMISC_RUNES:
		plr[pnum]._pTSpell = SPL_RUNESTONE;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			NewCursor(CURSOR_TELEPORT);
		break;
#endif
	}
}

void ItemStatOk(int pnum, ItemStruct *is)
{
	PlayerStruct *p = &plr[pnum];

	is->_iStatFlag = p->_pStrength >= is->_iMinStr
				  && p->_pDexterity >= is->_iMinDex
				  && p->_pMagic >= is->_iMinMag;
}

BOOL SmithItemOk(int i)
{
	return AllItemsList[i].itype != ITYPE_MISC
	 && AllItemsList[i].itype != ITYPE_GOLD
#ifdef HELLFIRE
	 && (AllItemsList[i].itype != ITYPE_STAFF || AllItemsList[i].iSpell == SPL_NULL)
#else
	 && AllItemsList[i].itype != ITYPE_STAFF
#endif
	 && AllItemsList[i].itype != ITYPE_RING
	 && AllItemsList[i].itype != ITYPE_AMULET
	 && AllItemsList[i].itype != ITYPE_MEAT;
}

int RndSmithItem(int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && SmithItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
#ifdef HELLFIRE
			if (ri == 512)
				break;
#endif
			ril[ri] = i;
			ri++;
			if (AllItemsList[i].iRnd == IDROP_DOUBLE) {
#ifdef HELLFIRE
				if (ri == 512)
					break;
#endif
				ril[ri] = i;
				ri++;
			}
		}
	}

	return ril[random_(50, ri)] + 1;
}

void BubbleSwapItem(ItemStruct *a, ItemStruct *b)
{
	ItemStruct h;

	h = *a;
	*a = *b;
	*b = h;
}

void SortSmith()
{
	int j, k;
	BOOL sorted;

	j = 0;
	while (smithitem[j + 1]._itype != ITYPE_NONE) {
		j++;
	}

	sorted = FALSE;
	while (j > 0 && !sorted) {
		sorted = TRUE;
		for (k = 0; k < j; k++) {
			if (smithitem[k].IDidx > smithitem[k + 1].IDidx) {
				BubbleSwapItem(&smithitem[k], &smithitem[k + 1]);
				sorted = FALSE;
			}
		}
		j--;
	}
}

void SpawnSmith(int lvl)
{
	int i, iCnt, idata;

#ifdef HELLFIRE
	ItemStruct holditem;
	holditem = item[0];
#endif
	iCnt = random_(50, SMITH_ITEMS - 10) + 10;
	for (i = 0; i < iCnt; i++) {
		do {
			item[0]._iSeed = GetRndSeed();
			SetRndSeed(item[0]._iSeed);
			idata = RndSmithItem(lvl) - 1;
			GetItemAttrs(0, idata, lvl);
		} while (item[0]._iIvalue > SMITH_MAX_VALUE);
		smithitem[i] = item[0];
		smithitem[i]._iCreateInfo = lvl | CF_SMITH;
		smithitem[i]._iIdentified = TRUE;
	}
	for (i = iCnt; i < SMITH_ITEMS; i++)
		smithitem[i]._itype = ITYPE_NONE;

	SortSmith();
#ifdef HELLFIRE
	item[0] = holditem;
#endif
}

BOOL PremiumItemOk(int i)
{
	return AllItemsList[i].itype != ITYPE_MISC
		&& AllItemsList[i].itype != ITYPE_GOLD
		&& AllItemsList[i].itype != ITYPE_MEAT
#ifdef HELLFIRE
		&& (gbMaxPlayers == 1 || (AllItemsList[i].iMiscId != IMISC_OILOF && AllItemsList[i].itype != ITYPE_RING && AllItemsList[i].itype != ITYPE_AMULET));
#else
		&& AllItemsList[i].itype != ITYPE_STAFF
		&& (gbMaxPlayers == 1 || (AllItemsList[i].itype != ITYPE_RING && AllItemsList[i].itype != ITYPE_AMULET));
#endif
}

int RndPremiumItem(int minlvl, int maxlvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && PremiumItemOk(i)) {
			if (AllItemsList[i].iMinMLvl >= minlvl && AllItemsList[i].iMinMLvl <= maxlvl) {
#ifdef HELLFIRE
				if (ri == 512)
					break;
#endif
				ril[ri] = i;
				ri++;
			}
		}
	}

	return ril[random_(50, ri)] + 1;
}

static void SpawnOnePremium(int i, int plvl)
{
	int itype;
	ItemStruct holditem;

	holditem = item[0];
	if (plvl > 30)
		plvl = 30;
	if (plvl < 1)
		plvl = 1;
	do {
		item[0]._iSeed = GetRndSeed();
		SetRndSeed(item[0]._iSeed);
		itype = RndPremiumItem(plvl >> 2, plvl) - 1;
		GetItemAttrs(0, itype, plvl);
		GetItemBonus(0, plvl >> 1, plvl, TRUE, FALSE);
	} while (item[0]._iIvalue > SMITH_MAX_PREMIUM_VALUE);
	premiumitem[i] = item[0];
	premiumitem[i]._iCreateInfo = plvl | CF_SMITHPREMIUM;
	premiumitem[i]._iIdentified = TRUE;
	item[0] = holditem;
}

void SpawnPremium(int lvl)
{
	int i;

	if (numpremium < SMITH_PREMIUM_ITEMS) {
		for (i = 0; i < SMITH_PREMIUM_ITEMS; i++) {
			if (premiumitem[i]._itype == ITYPE_NONE)
				SpawnOnePremium(i, premiumlevel + premiumlvladd[i]);
		}
		numpremium = SMITH_PREMIUM_ITEMS;
	}
	while (premiumlevel < lvl) {
		premiumlevel++;
		premiumitem[0] = premiumitem[2];
		premiumitem[1] = premiumitem[3];
		premiumitem[2] = premiumitem[4];
		SpawnOnePremium(3, premiumlevel + premiumlvladd[3]);
		premiumitem[4] = premiumitem[5];
		SpawnOnePremium(5, premiumlevel + premiumlvladd[5]);
	}
}

BOOL WitchItemOk(int i)
{
	BOOL rv;

	rv = FALSE;
#ifdef HELLFIRE
	if (AllItemsList[i].itype == ITYPE_MISC || AllItemsList[i].itype == ITYPE_STAFF)
		rv = TRUE;
	if (AllItemsList[i].iMiscId == IMISC_MANA || AllItemsList[i].iMiscId == IMISC_FULLMANA)
		rv = FALSE;
	if (AllItemsList[i].iSpell == SPL_TOWN)
		rv = FALSE;
	if (AllItemsList[i].iMiscId == IMISC_FULLHEAL || AllItemsList[i].iMiscId == IMISC_HEAL)
		rv = FALSE;
	if (AllItemsList[i].iMiscId > IMISC_OILFIRST && AllItemsList[i].iMiscId < IMISC_OILLAST)
		rv = FALSE;
	if ((AllItemsList[i].iSpell == SPL_RESURRECT && gbMaxPlayers == 1) || (AllItemsList[i].iSpell == SPL_HEALOTHER && gbMaxPlayers == 1))
		rv = FALSE;
#else
	if (AllItemsList[i].itype == ITYPE_MISC)
		rv = TRUE;
	if (AllItemsList[i].itype == ITYPE_STAFF)
		rv = TRUE;
	if (AllItemsList[i].iMiscId == IMISC_MANA)
		rv = FALSE;
	if (AllItemsList[i].iMiscId == IMISC_FULLMANA)
		rv = FALSE;
	if (AllItemsList[i].iSpell == SPL_TOWN)
		rv = FALSE;
	if (AllItemsList[i].iMiscId == IMISC_FULLHEAL)
		rv = FALSE;
	if (AllItemsList[i].iMiscId == IMISC_HEAL)
		rv = FALSE;
	if (AllItemsList[i].iSpell == SPL_RESURRECT && gbMaxPlayers == 1)
		rv = FALSE;
	if (AllItemsList[i].iSpell == SPL_HEALOTHER && gbMaxPlayers == 1)
		rv = FALSE;
#endif

	return rv;
}

int RndWitchItem(int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && WitchItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
#ifdef HELLFIRE
			if (ri == 512)
				break;
#endif
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(51, ri)] + 1;
}

void SortWitch()
{
	int j, k;
	BOOL sorted;

	j = 3;
	while (witchitem[j + 1]._itype != ITYPE_NONE) {
		j++;
	}

	sorted = FALSE;
	while (j > 3 && !sorted) {
		sorted = TRUE;
		for (k = 3; k < j; k++) {
			if (witchitem[k].IDidx > witchitem[k + 1].IDidx) {
				BubbleSwapItem(&witchitem[k], &witchitem[k + 1]);
				sorted = FALSE;
			}
		}
		j--;
	}
}

void SetBookLevel(int pnum, ItemStruct *is)
{
	int slvl;

	if (is->_iMiscId == IMISC_BOOK) {
		is->_iMinMag = spelldata[is->_iSpell].sMinInt;
		slvl = plr[pnum]._pSplLvl[is->_iSpell];
		while (slvl != 0) {
			is->_iMinMag += 20 * is->_iMinMag / 100;
			slvl--;
			if (is->_iMinMag + 20 * is->_iMinMag / 100 > 255) {
				is->_iMinMag = 255;
				slvl = 0;
			}
		}
	}
}

void SpawnWitch(int lvl)
{
	int i, iCnt;
	int idata, minlvl;

	GetItemAttrs(0, IDI_MANA, 1);
	witchitem[0] = item[0];
	witchitem[0]._iCreateInfo = lvl;
	witchitem[0]._iStatFlag = TRUE;
	GetItemAttrs(0, IDI_FULLMANA, 1);
	witchitem[1] = item[0];
	witchitem[1]._iCreateInfo = lvl;
	witchitem[1]._iStatFlag = TRUE;
	GetItemAttrs(0, IDI_PORTAL, 1);
	witchitem[2] = item[0];
	witchitem[2]._iCreateInfo = lvl;
	witchitem[2]._iStatFlag = TRUE;
	iCnt = random_(51, 8) + 10;

	for (i = 3; i < iCnt; i++) {
		do {
			item[0]._iSeed = GetRndSeed();
			SetRndSeed(item[0]._iSeed);
			idata = RndWitchItem(lvl) - 1;
			GetItemAttrs(0, idata, lvl);
			minlvl = -1;
			if (random_(51, 100) <= 5)
				minlvl = lvl;
			if (minlvl == -1 && item[0]._iMiscId == IMISC_STAFF)
				minlvl = lvl;
			if (minlvl != -1)
				GetItemBonus(0, minlvl, minlvl << 1, TRUE, TRUE);
		} while (item[0]._iIvalue > 140000);
		witchitem[i] = item[0];
		witchitem[i]._iCreateInfo = lvl | CF_WITCH;
		witchitem[i]._iIdentified = TRUE;
	}

	for (i = iCnt; i < 20; i++)
		witchitem[i]._itype = ITYPE_NONE;

	SortWitch();
}

int RndBoyItem(int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && PremiumItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
#ifdef HELLFIRE
			if (ri == 512)
				break;
#endif
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(49, ri)] + 1;
}

void SpawnBoy(int lvl)
{
	int itype;

	if (boylevel < (lvl >> 1) || boyitem._itype == ITYPE_NONE) {
		do {
			item[0]._iSeed = GetRndSeed();
			SetRndSeed(item[0]._iSeed);
			itype = RndBoyItem(lvl) - 1;
			GetItemAttrs(0, itype, lvl);
			GetItemBonus(0, lvl, lvl << 1, TRUE, TRUE);
		} while (item[0]._iIvalue > 90000);
		boyitem = item[0];
		boyitem._iCreateInfo = lvl | CF_BOY;
		boyitem._iIdentified = TRUE;
		boylevel = lvl >> 1;
	}
}

BOOL HealerItemOk(int i)
{
	if (AllItemsList[i].itype != ITYPE_MISC)
		return FALSE;

	switch (AllItemsList[i].iMiscId) {
#ifdef HELLFIRE
	case IMISC_ELIXSTR:
		return gbMaxPlayers == 1 && plr[myplr]._pBaseStr < MaxStats[plr[myplr]._pClass][ATTRIB_STR];
	case IMISC_ELIXMAG:
		return gbMaxPlayers == 1 && plr[myplr]._pBaseMag < MaxStats[plr[myplr]._pClass][ATTRIB_MAG];
	case IMISC_ELIXDEX:
		return gbMaxPlayers == 1 && plr[myplr]._pBaseDex < MaxStats[plr[myplr]._pClass][ATTRIB_DEX];
	case IMISC_ELIXVIT:
		return gbMaxPlayers == 1 && plr[myplr]._pBaseVit < MaxStats[plr[myplr]._pClass][ATTRIB_VIT];
#else
	case IMISC_ELIXSTR:
	case IMISC_ELIXMAG:
	case IMISC_ELIXDEX:
	case IMISC_ELIXVIT:
		return gbMaxPlayers == 1;
#endif
	case IMISC_REJUV:
	case IMISC_FULLREJUV:
		return TRUE;
	case IMISC_SCROLL:
		return AllItemsList[i].iSpell == SPL_HEAL;
	case IMISC_SCROLLT:
		return gbMaxPlayers != 1 && (AllItemsList[i].iSpell == SPL_RESURRECT || AllItemsList[i].iSpell == SPL_HEALOTHER);
	}
	return FALSE;
}

int RndHealerItem(int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && HealerItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
#ifdef HELLFIRE
			if (ri == 512)
				break;
#endif
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(50, ri)] + 1;
}

void SortHealer()
{
	int j, k;
	BOOL sorted;

	j = 2;
	while (healitem[j + 1]._itype != ITYPE_NONE) {
		j++;
	}

	sorted = FALSE;
	while (j > 2 && !sorted) {
		sorted = TRUE;
		for (k = 2; k < j; k++) {
			if (healitem[k].IDidx > healitem[k + 1].IDidx) {
				BubbleSwapItem(&healitem[k], &healitem[k + 1]);
				sorted = FALSE;
			}
		}
		j--;
	}
}

void SpawnHealer(int lvl)
{
	int i, iCnt, srnd, itype;

	GetItemAttrs(0, IDI_HEAL, 1);
	healitem[0] = item[0];
	healitem[0]._iCreateInfo = lvl;
	healitem[0]._iStatFlag = TRUE;

	GetItemAttrs(0, IDI_FULLHEAL, 1);
	healitem[1] = item[0];
	healitem[1]._iCreateInfo = lvl;
	healitem[1]._iStatFlag = TRUE;

	if (gbMaxPlayers != 1) {
		GetItemAttrs(0, IDI_RESURRECT, 1);
		healitem[2] = item[0];
		healitem[2]._iCreateInfo = lvl;
		healitem[2]._iStatFlag = TRUE;

		srnd = 3;
	} else {
		srnd = 2;
	}
#ifdef HELLFIRE
	iCnt = random_(50, HEALER_ITEMS - 10) + 10;
#else
	iCnt = random_(50, 8) + 10;
#endif
	for (i = srnd; i < iCnt; i++) {
		item[0]._iSeed = GetRndSeed();
		SetRndSeed(item[0]._iSeed);
		itype = RndHealerItem(lvl) - 1;
		GetItemAttrs(0, itype, lvl);
		healitem[i] = item[0];
		healitem[i]._iCreateInfo = lvl | CF_HEALER;
		healitem[i]._iIdentified = TRUE;
	}
	for (i = iCnt; i < HEALER_ITEMS; i++) {
		healitem[i]._itype = ITYPE_NONE;
	}
	SortHealer();
}

void SpawnStoreGold()
{
	GetItemAttrs(0, IDI_GOLD, 1);
	golditem = item[0];
	golditem._iStatFlag = TRUE;
}

void RecreateSmithItem(int ii, int idx, int lvl, int iseed)
{
	int itype;

	SetRndSeed(iseed);
	itype = RndSmithItem(lvl) - 1;
	GetItemAttrs(ii, itype, lvl);

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_SMITH;
	item[ii]._iIdentified = TRUE;
}

void RecreatePremiumItem(int ii, int idx, int plvl, int iseed)
{
	int itype;

	SetRndSeed(iseed);
	itype = RndPremiumItem(plvl >> 2, plvl) - 1;
	GetItemAttrs(ii, itype, plvl);
	GetItemBonus(ii, plvl >> 1, plvl, TRUE, FALSE);

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = plvl | CF_SMITHPREMIUM;
	item[ii]._iIdentified = TRUE;
}

void RecreateBoyItem(int ii, int idx, int lvl, int iseed)
{
	int itype;

	SetRndSeed(iseed);
	itype = RndBoyItem(lvl) - 1;
	GetItemAttrs(ii, itype, lvl);
	GetItemBonus(ii, lvl, 2 * lvl, TRUE, TRUE);
	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_BOY;
	item[ii]._iIdentified = TRUE;
}

void RecreateWitchItem(int ii, int idx, int lvl, int iseed)
{
	int iblvl, itype;

	if (idx == IDI_MANA || idx == IDI_FULLMANA || idx == IDI_PORTAL) {
		GetItemAttrs(ii, idx, lvl);
#ifdef HELLFIRE
	} else if (idx >= 114 && idx <= 117) {
		SetRndSeed(iseed);
		volatile int hi_predelnik = random_(0, 1);
		GetItemAttrs(ii, idx, lvl);
#endif
	} else {
		SetRndSeed(iseed);
		itype = RndWitchItem(lvl) - 1;
		GetItemAttrs(ii, itype, lvl);
		iblvl = -1;
		if (random_(51, 100) <= 5)
			iblvl = lvl;
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_STAFF)
			iblvl = lvl;
		if (iblvl != -1)
			GetItemBonus(ii, iblvl, iblvl << 1, TRUE, TRUE);
	}

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_WITCH;
	item[ii]._iIdentified = TRUE;
}

void RecreateHealerItem(int ii, int idx, int lvl, int iseed)
{
	int itype;

	if (idx == IDI_HEAL || idx == IDI_FULLHEAL || idx == IDI_RESURRECT) {
		GetItemAttrs(ii, idx, lvl);
	} else {
		SetRndSeed(iseed);
		itype = RndHealerItem(lvl) - 1;
		GetItemAttrs(ii, itype, lvl);
	}

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_HEALER;
	item[ii]._iIdentified = TRUE;
}

void RecreateTownItem(int ii, int idx, WORD icreateinfo, int iseed, int ivalue)
{
	if (icreateinfo & CF_SMITH)
		RecreateSmithItem(ii, idx, icreateinfo & CF_LEVEL, iseed);
	else if (icreateinfo & CF_SMITHPREMIUM)
		RecreatePremiumItem(ii, idx, icreateinfo & CF_LEVEL, iseed);
	else if (icreateinfo & CF_BOY)
		RecreateBoyItem(ii, idx, icreateinfo & CF_LEVEL, iseed);
	else if (icreateinfo & CF_WITCH)
		RecreateWitchItem(ii, idx, icreateinfo & CF_LEVEL, iseed);
	else if (icreateinfo & CF_HEALER)
		RecreateHealerItem(ii, idx, icreateinfo & CF_LEVEL, iseed);
}

int ItemNoFlippy()
{
	int r;

	r = itemactive[numitems - 1];
	item[r]._iAnimFrame = item[r]._iAnimLen;
	item[r]._iAnimFlag = FALSE;
	item[r]._iSelFlag = 1;

	return r;
}

void CreateSpellBook(int x, int y, int ispell, BOOL sendmsg, BOOL delta)
{
	int ii, idx, lvl;

#ifdef HELLFIRE
	lvl = spelldata[ispell].sBookLvl + 1;
	if (lvl < 1)
		return;
#else
	lvl = currlevel;
#endif
	lvl <<= 1;

	idx = RndTypeItems(ITYPE_MISC, IMISC_BOOK, lvl);
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, delta);
			if (item[ii]._iMiscId == IMISC_BOOK && item[ii]._iSpell == ispell)
				break;
		}
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
		if (delta)
			DeltaAddItem(ii);
		numitems++;
	}
}

void CreateMagicArmor(int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta)
{
	int ii, idx, lvl;

	lvl = items_get_currlevel();
	lvl <<= 1;

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			idx = RndTypeItems(imisc, IMISC_NONE, lvl);
			SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, delta);
			if (item[ii]._iCurs == icurs)
				break;
		}
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
		if (delta)
			DeltaAddItem(ii);
		numitems++;
	}
}

#ifdef HELLFIRE
void CreateAmulet(int x, int y, int lvl, BOOL sendmsg, BOOL delta)
{
	int ii, idx;

	lvl <<= 1;

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			idx = RndTypeItems(ITYPE_AMULET, IMISC_AMULET, lvl);
			SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, delta);
			if (item[ii]._iCurs == ICURS_AMULET)
				break;
		}
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
		if (delta)
			DeltaAddItem(ii);
		numitems++;
	}
}
#endif

void CreateMagicWeapon(int x, int y, int imisc, int icurs, BOOL sendmsg, BOOL delta)
{
	int ii, idx, lvl, imid;

	imid = IMISC_NONE;
#ifdef HELLFIRE
	if (imisc == ITYPE_STAFF)
		imid = IMISC_STAFF;
#endif

	lvl = items_get_currlevel();
	lvl <<= 1;

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			idx = RndTypeItems(imisc, imid, lvl);
			SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, delta);
			if (item[ii]._iCurs == icurs)
				break;
		}
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
		if (delta)
			DeltaAddItem(ii);
		numitems++;
	}
}

static void NextItemRecord(int i)
{
	gnNumGetRecords--;

	if (gnNumGetRecords == 0) {
		return;
	}

	itemrecord[i].dwTimestamp = itemrecord[gnNumGetRecords].dwTimestamp;
	itemrecord[i].nSeed = itemrecord[gnNumGetRecords].nSeed;
	itemrecord[i].wCI = itemrecord[gnNumGetRecords].wCI;
	itemrecord[i].nIndex = itemrecord[gnNumGetRecords].nIndex;
}

BOOL GetItemRecord(int nSeed, WORD wCI, int nIndex)
{
	int i;
	DWORD dwTicks;

	dwTicks = SDL_GetTicks();

	for (i = 0; i < gnNumGetRecords; i++) {
		if (dwTicks - itemrecord[i].dwTimestamp > 6000) {
			NextItemRecord(i);
			i--;
		} else if (nSeed == itemrecord[i].nSeed && wCI == itemrecord[i].wCI && nIndex == itemrecord[i].nIndex) {
			return FALSE;
		}
	}

	return TRUE;
}

void SetItemRecord(int nSeed, WORD wCI, int nIndex)
{
	if (gnNumGetRecords == MAXITEMS) {
		return;
	}

	itemrecord[gnNumGetRecords].dwTimestamp = SDL_GetTicks();
	itemrecord[gnNumGetRecords].nSeed = nSeed;
	itemrecord[gnNumGetRecords].wCI = wCI;
	itemrecord[gnNumGetRecords].nIndex = nIndex;
	gnNumGetRecords++;
}

void PutItemRecord(int nSeed, WORD wCI, int nIndex)
{
	int i;
	DWORD dwTicks;

	dwTicks = SDL_GetTicks();

	for (i = 0; i < gnNumGetRecords; i++) {
		if (dwTicks - itemrecord[i].dwTimestamp > 6000) {
			NextItemRecord(i);
			i--;
		} else if (nSeed == itemrecord[i].nSeed && wCI == itemrecord[i].wCI && nIndex == itemrecord[i].nIndex) {
			NextItemRecord(i);
			break;
		}
	}
}

DEVILUTION_END_NAMESPACE
