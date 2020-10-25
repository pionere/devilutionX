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
#endif
BYTE *itemanims[ITEMTYPES];
BOOL UniqueItemFlag[128];
#ifdef HELLFIRE
int auricGold = 10000;
#endif
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
int MaxGold = GOLD_MAX_LIMIT;
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
char *ItemDropNames[] = {
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
int idoppely = 16;
/** Maps from Griswold premium item number to a quality level delta as added to the base quality level. */
int premiumlvladd[6] = { -1, -1, 0, 0, 1, 2 };

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

int items_get_currlevel()
{
	int lvl;

	lvl = currlevel;
	if (currlevel >= 17 && currlevel <= 20)
		lvl = currlevel - 8;
	if (currlevel >= 21 && currlevel <= 24)
		lvl = currlevel - 7;

	return lvl;
}

void SpawnNote()
{
	int x, y, id;

	do {
		x = random_(12, 80) + 16;
		y = random_(12, 80) + 16;
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

void InitItemGFX()
{
#ifdef HELLFIRE
	DWORD i;
#else
	int i;
#endif
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

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	rnd = random_(11, 3) + 3;
	for (j = 0; j < rnd; j++) {
		i = itemavail[0];
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = i;
		do {
			x = random_(12, 80) + 16;
			y = random_(12, 80) + 16;
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
	long s;

	GetItemAttrs(0, IDI_GOLD, 1);
	golditem = item[0];
	golditem._iStatFlag = TRUE;
	numitems = 0;

	for (i = 0; i < MAXITEMS; i++) {
		item[i]._itype = ITYPE_MISC;
		item[i]._ix = 0;
		item[i]._iy = 0;
		item[i]._iAnimFlag = FALSE;
		item[i]._iSelFlag = 0;
		item[i]._iIdentified = FALSE;
		item[i]._iPostDraw = FALSE;
	}

	for (i = 0; i < MAXITEMS; i++) {
		itemavail[i] = i;
		itemactive[i] = 0;
	}

	if (!setlevel) {
		s = GetRndSeed(); /* unused */
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
	if ((p->_pSpellFlags & 2) == 2) {
		sadd += 2 * p->_pLevel;
		dadd += p->_pLevel + p->_pLevel / 2;
		vadd += 2 * p->_pLevel;
	}
	if ((p->_pSpellFlags & 4) == 4) {
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
	}
#ifdef HELLFIRE
	else if (p->_pClass == PC_MONK) {
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
	}
#endif
	else {
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

	if ((p->_pSpellFlags & 4) == 4) {
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
	}
#ifdef HELLFIRE
	else if (p->_pClass == PC_BARD) {
		madd += madd >> 2 + madd >> 1;
	}
#endif
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
		if (pi->_itype != ITYPE_NONE && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)) {
			if (pi->_iStatFlag)
				p->_pScrlSpells |= (__int64)1 << (pi->_iSpell - 1);
		}
	}
	pi = p->SpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)) {
			if (pi->_iStatFlag)
				p->_pScrlSpells |= (__int64)1 << (pi->_iSpell - 1);
		}
	}
	if (p->_pRSplType == RSPLTYPE_SCROLL) {
		if (!(p->_pScrlSpells & 1 << (p->_pRSpell - 1))) {
			p->_pRSpell = SPL_INVALID;
			p->_pRSplType = RSPLTYPE_INVALID;
			force_redraw = 255;
		}
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
	for (i = 0; i < NUM_INVLOC; i++, pi++) {
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
		for (i = 0; i < NUM_INVLOC; i++, pi++) {
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
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &plr[pnum];
	pi = p->InvList;
	i = p->_pNumInv;

	while (i--) {
		ItemStatOk(pnum, pi);
		pi++;
	}

	pi = p->SpdList;
	for (i = MAXBELTITEMS; i != 0; i--) {
		if (pi->_itype != ITYPE_NONE) {
			ItemStatOk(pnum, pi);
		}
		pi++;
	}
}

void CalcPlrBookVals(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i, slvl;

	if (currlevel == 0) {
		for (i = 1; witchitem[i]._itype != ITYPE_NONE; i++) {
			WitchBookLevel(i);
#ifndef HELLFIRE
			ItemStatOk(myplr, &witchitem[i]);
#endif
		}
	}

	p = &plr[pnum];
	pi = p->InvList;
	for (i = p->_pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_MISC && pi->_iMiscId == IMISC_BOOK) {
			pi->_iMinMag = spelldata[pi->_iSpell].sMinInt;
			slvl = p->_pSplLvl[pi->_iSpell];

			while (slvl != 0) {
				pi->_iMinMag += 20 * pi->_iMinMag / 100;
				slvl--;
				if (pi->_iMinMag + 20 * pi->_iMinMag / 100 > 255) {
					pi->_iMinMag = 255;
					slvl = 0;
				}
			}
			ItemStatOk(pnum, pi);
		}
	}
}

void CalcPlrInv(int p, BOOL Loadgfx)
{
	CalcPlrItemMin(p);
	CalcSelfItems(p);
	CalcPlrItemVals(p, Loadgfx);
	CalcPlrItemMin(p);
	if (p == myplr) {
		CalcPlrBookVals(p);
		CalcPlrScrolls(p);
		CalcPlrStaff(p);
		if (p == myplr && currlevel == 0)
			RecalcStoreStats();
	}
}

void SetPlrHandItem(ItemStruct *is, int idata)
{
	ItemDataStruct *pAllItem;

	pAllItem = &AllItemsList[idata];

	// zero-initialize struct
	memset(is, 0, sizeof(*is));

	is->_itype = pAllItem->itype;
	is->_iCurs = pAllItem->iCurs;
	strcpy(is->_iName, pAllItem->iName);
	strcpy(is->_iIName, pAllItem->iName);
	is->_iLoc = pAllItem->iLoc;
	is->_iClass = pAllItem->iClass;
	is->_iMinDam = pAllItem->iMinDam;
	is->_iMaxDam = pAllItem->iMaxDam;
	is->_iAC = pAllItem->iMinAC;
	is->_iMiscId = pAllItem->iMiscId;
	is->_iSpell = pAllItem->iSpell;

	if (pAllItem->iMiscId == IMISC_STAFF) {
#ifdef HELLFIRE
		is->_iCharges = 18;
#else
		is->_iCharges = 40;
#endif
	}

	is->_iMaxCharges = is->_iCharges;
	is->_iDurability = pAllItem->iDurability;
	is->_iMaxDur = pAllItem->iDurability;
	is->_iMinStr = pAllItem->iMinStr;
	is->_iMinMag = pAllItem->iMinMag;
	is->_iMinDex = pAllItem->iMinDex;
	is->_ivalue = pAllItem->iValue;
	is->_iIvalue = pAllItem->iValue;
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

void SetPlrHandSeed(ItemStruct *is, int iseed)
{
	is->_iSeed = iseed;
}

void SetGoldItemValue(ItemStruct *is, int value)
{
	is->_ivalue = value;
	if (is->_ivalue >= GOLD_MEDIUM_LIMIT)
		is->_iCurs = ICURS_GOLD_LARGE;
	else if (is->_ivalue <= GOLD_SMALL_LIMIT)
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
			AutoPlace(pnum, 0, 1, 3, TRUE);
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
	v = item[ii]._iVAdd1 + item[ii]._iVAdd2 + v;
	if (v <= 0) {
		v = 1;
	}
	item[ii]._iIvalue = v;
}

void GetBookSpell(int ii, int lvl)
{
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
	strcat(is->_iName, spelldata[bs].sNameText);
	strcat(is->_iIName, spelldata[bs].sNameText);
	is->_iSpell = bs;
	is->_iMinMag = spelldata[bs].sMinInt;
	is->_ivalue += spelldata[bs].sBookCost;
	is->_iIvalue += spelldata[bs].sBookCost;
	if (spelldata[bs].sType == STYPE_FIRE)
		is->_iCurs = ICURS_BOOK_RED;
	else if (spelldata[bs].sType == STYPE_LIGHTNING)
		is->_iCurs = ICURS_BOOK_BLUE;
	else if (spelldata[bs].sType == STYPE_MAGIC)
		is->_iCurs = ICURS_BOOK_GREY;
}

void GetStaffPower(int ii, int lvl, int bs, BOOL onlygood)
{
	int l[256];
	char istr[128];
	int nl, j, preidx;

	preidx = -1;
	if (random_(15, 10) == 0 || onlygood) {
		nl = 0;
		for (j = 0; PL_Prefix[j].PLPower != -1; j++) {
			if (PL_Prefix[j].PLIType & PLT_STAFF && PL_Prefix[j].PLMinLvl <= lvl) {
				if (!onlygood || PL_Prefix[j].PLOk) {
					l[nl] = j;
					nl++;
					if (PL_Prefix[j].PLDouble) {
						l[nl] = j;
						nl++;
					}
				}
			}
		}
		if (nl != 0) {
			preidx = l[random_(16, nl)];
			sprintf(istr, "%s %s", PL_Prefix[preidx].PLName, item[ii]._iIName);
			strcpy(item[ii]._iIName, istr);
			item[ii]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    ii,
			    PL_Prefix[preidx].PLPower,
			    PL_Prefix[preidx].PLParam1,
			    PL_Prefix[preidx].PLParam2,
			    PL_Prefix[preidx].PLMinVal,
			    PL_Prefix[preidx].PLMaxVal,
			    PL_Prefix[preidx].PLMultVal);
			item[ii]._iPrePower = PL_Prefix[preidx].PLPower;
		}
	}
	if (!control_WriteStringToBuffer((BYTE *)item[ii]._iIName)) {
		strcpy(item[ii]._iIName, AllItemsList[item[ii].IDidx].iSName);
		if (preidx != -1) {
			sprintf(istr, "%s %s", PL_Prefix[preidx].PLName, item[ii]._iIName);
			strcpy(item[ii]._iIName, istr);
		}
		sprintf(istr, "%s of %s", item[ii]._iIName, spelldata[bs].sNameText);
		strcpy(item[ii]._iIName, istr);
		if (item[ii]._iMagical == ITEM_QUALITY_NORMAL)
			strcpy(item[ii]._iName, item[ii]._iIName);
	}
	CalcItemValue(ii);
}

void GetStaffSpell(int ii, int lvl, BOOL onlygood)
{
	ItemStruct *is;
	int l, rv, s, minc, maxc, v, bs;
	char istr[64];

#ifndef HELLFIRE
	if (!random_(17, 4)) {
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
		sprintf(istr, "%s of %s", is->_iName, spelldata[bs].sNameText);
		if (!control_WriteStringToBuffer((BYTE *)istr))
			sprintf(istr, "Staff of %s", spelldata[bs].sNameText);
		strcpy(is->_iName, istr);
		strcpy(is->_iIName, istr);

		minc = spelldata[bs].sStaffMin;
		maxc = spelldata[bs].sStaffMax - minc + 1;
		is->_iSpell = bs;
		is->_iCharges = minc + random_(19, maxc);
		is->_iMaxCharges = is->_iCharges;

		is->_iMinMag = spelldata[bs].sMinInt;
		v = is->_iCharges * spelldata[bs].sStaffCost / 5;
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
		if (gnDifficulty == DIFF_NORMAL)
			rndv = 5 * lvl + random_(21, 10 * lvl);
		if (gnDifficulty == DIFF_NIGHTMARE)
			rndv = 5 * (lvl + 16) + random_(21, 10 * (lvl + 16));
		if (gnDifficulty == DIFF_HELL)
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
	if (p1 == p2)
		return minv;
	if (minv == maxv)
		return minv;
	return minv + (maxv - minv) * (100 * (pv - p1) / (p2 - p1)) / 100;
}

void SaveItemPower(int ii, int power, int param1, int param2, int minval, int maxval, int multval)
{
	int r, r2;

	r = RndPL(param1, param2);
	switch (power) {
	case IPL_TOHIT:
		item[ii]._iPLToHit += r;
		break;
	case IPL_TOHIT_CURSE:
		item[ii]._iPLToHit -= r;
		break;
	case IPL_DAMP:
		item[ii]._iPLDam += r;
		break;
	case IPL_DAMP_CURSE:
		item[ii]._iPLDam -= r;
		break;
#ifdef HELLFIRE
	case IPL_DOPPELGANGER:
		item[ii]._iDamAcFlags |= 16;
		// no break
#endif
	case IPL_TOHIT_DAMP:
		r = RndPL(param1, param2);
		item[ii]._iPLDam += r;
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
		item[ii]._iPLToHit += r2;
		break;
	case IPL_TOHIT_DAMP_CURSE:
		item[ii]._iPLDam -= r;
		if (param1 == 25)
			r2 = RndPL(1, 5);
		if (param1 == 50)
			r2 = RndPL(6, 10);
		item[ii]._iPLToHit -= r2;
		break;
	case IPL_ACP:
		item[ii]._iPLAC += r;
		break;
	case IPL_ACP_CURSE:
		item[ii]._iPLAC -= r;
		break;
	case IPL_SETAC:
		item[ii]._iAC = r;
		break;
	case IPL_AC_CURSE:
		item[ii]._iAC -= r;
		break;
	case IPL_FIRERES:
		item[ii]._iPLFR += r;
		break;
	case IPL_LIGHTRES:
		item[ii]._iPLLR += r;
		break;
	case IPL_MAGICRES:
		item[ii]._iPLMR += r;
		break;
	case IPL_ALLRES:
		item[ii]._iPLFR += r;
		item[ii]._iPLLR += r;
		item[ii]._iPLMR += r;
		if (item[ii]._iPLFR < 0)
			item[ii]._iPLFR = 0;
		if (item[ii]._iPLLR < 0)
			item[ii]._iPLLR = 0;
		if (item[ii]._iPLMR < 0)
			item[ii]._iPLMR = 0;
		break;
	case IPL_SPLLVLADD:
		item[ii]._iSplLvlAdd = r;
		break;
	case IPL_CHARGES:
		item[ii]._iCharges *= param1;
		item[ii]._iMaxCharges = item[ii]._iCharges;
		break;
	case IPL_SPELL:
		item[ii]._iSpell = param1;
#ifdef HELLFIRE
		item[ii]._iCharges = param2;
#else
		item[ii]._iCharges = param1; // BUGFIX: should be param2. This code was correct in v1.04, and the bug was introduced between 1.04 and 1.09b.
#endif
		item[ii]._iMaxCharges = param2;
		break;
	case IPL_FIREDAM:
		item[ii]._iFlags |= ISPL_FIREDAM;
#ifdef HELLFIRE
		item[ii]._iFlags &= ~ISPL_LIGHTDAM;
#endif
		item[ii]._iFMinDam = param1;
		item[ii]._iFMaxDam = param2;
#ifdef HELLFIRE
		item[ii]._iLMinDam = 0;
		item[ii]._iLMaxDam = 0;
#endif
		break;
	case IPL_LIGHTDAM:
		item[ii]._iFlags |= ISPL_LIGHTDAM;
#ifdef HELLFIRE
		item[ii]._iFlags &= ~ISPL_FIREDAM;
#endif
		item[ii]._iLMinDam = param1;
		item[ii]._iLMaxDam = param2;
#ifdef HELLFIRE
		item[ii]._iFMinDam = 0;
		item[ii]._iFMaxDam = 0;
#endif
		break;
	case IPL_STR:
		item[ii]._iPLStr += r;
		break;
	case IPL_STR_CURSE:
		item[ii]._iPLStr -= r;
		break;
	case IPL_MAG:
		item[ii]._iPLMag += r;
		break;
	case IPL_MAG_CURSE:
		item[ii]._iPLMag -= r;
		break;
	case IPL_DEX:
		item[ii]._iPLDex += r;
		break;
	case IPL_DEX_CURSE:
		item[ii]._iPLDex -= r;
		break;
	case IPL_VIT:
		item[ii]._iPLVit += r;
		break;
	case IPL_VIT_CURSE:
		item[ii]._iPLVit -= r;
		break;
	case IPL_ATTRIBS:
		item[ii]._iPLStr += r;
		item[ii]._iPLMag += r;
		item[ii]._iPLDex += r;
		item[ii]._iPLVit += r;
		break;
	case IPL_ATTRIBS_CURSE:
		item[ii]._iPLStr -= r;
		item[ii]._iPLMag -= r;
		item[ii]._iPLDex -= r;
		item[ii]._iPLVit -= r;
		break;
	case IPL_GETHIT_CURSE:
		item[ii]._iPLGetHit += r;
		break;
	case IPL_GETHIT:
		item[ii]._iPLGetHit -= r;
		break;
	case IPL_LIFE:
		item[ii]._iPLHP += r << 6;
		break;
	case IPL_LIFE_CURSE:
		item[ii]._iPLHP -= r << 6;
		break;
	case IPL_MANA:
		item[ii]._iPLMana += r << 6;
		drawmanaflag = TRUE;
		break;
	case IPL_MANA_CURSE:
		item[ii]._iPLMana -= r << 6;
		drawmanaflag = TRUE;
		break;
	case IPL_DUR:
		r2 = r * item[ii]._iMaxDur / 100;
		item[ii]._iMaxDur += r2;
		item[ii]._iDurability += r2;
		break;
#ifdef HELLFIRE
	case IPL_CRYSTALLINE:
		item[ii]._iPLDam += 140 + r * 2;
		// no break
#endif
	case IPL_DUR_CURSE:
		item[ii]._iMaxDur -= r * item[ii]._iMaxDur / 100;
		if (item[ii]._iMaxDur < 1)
			item[ii]._iMaxDur = 1;
		item[ii]._iDurability = item[ii]._iMaxDur;
		break;
	case IPL_INDESTRUCTIBLE:
		item[ii]._iDurability = DUR_INDESTRUCTIBLE;
		item[ii]._iMaxDur = DUR_INDESTRUCTIBLE;
		break;
	case IPL_LIGHT:
		item[ii]._iPLLight += param1;
		break;
	case IPL_LIGHT_CURSE:
		item[ii]._iPLLight -= param1;
		break;
#ifdef HELLFIRE
	case IPL_MULT_ARROWS:
		item[ii]._iFlags |= ISPL_MULT_ARROWS;
		break;
#endif
	case IPL_FIRE_ARROWS:
		item[ii]._iFlags |= ISPL_FIRE_ARROWS;
#ifdef HELLFIRE
		item[ii]._iFlags &= ~ISPL_LIGHT_ARROWS;
#endif
		item[ii]._iFMinDam = param1;
		item[ii]._iFMaxDam = param2;
#ifdef HELLFIRE
		item[ii]._iLMinDam = 0;
		item[ii]._iLMaxDam = 0;
#endif
		break;
	case IPL_LIGHT_ARROWS:
		item[ii]._iFlags |= ISPL_LIGHT_ARROWS;
#ifdef HELLFIRE
		item[ii]._iFlags &= ~ISPL_FIRE_ARROWS;
#endif
		item[ii]._iLMinDam = param1;
		item[ii]._iLMaxDam = param2;
#ifdef HELLFIRE
		item[ii]._iFMinDam = 0;
		item[ii]._iFMaxDam = 0;
#endif
		break;
#ifdef HELLFIRE
	case IPL_FIREBALL:
		item[ii]._iFlags |= (ISPL_LIGHT_ARROWS | ISPL_FIRE_ARROWS);
		item[ii]._iFMinDam = param1;
		item[ii]._iFMaxDam = param2;
		item[ii]._iLMinDam = 0;
		item[ii]._iLMaxDam = 0;
		break;
#endif
	case IPL_THORNS:
		item[ii]._iFlags |= ISPL_THORNS;
		break;
	case IPL_NOMANA:
		item[ii]._iFlags |= ISPL_NOMANA;
		drawmanaflag = TRUE;
		break;
	case IPL_NOHEALPLR:
		item[ii]._iFlags |= ISPL_NOHEALPLR;
		break;
	case IPL_ABSHALFTRAP:
		item[ii]._iFlags |= ISPL_ABSHALFTRAP;
		break;
	case IPL_KNOCKBACK:
		item[ii]._iFlags |= ISPL_KNOCKBACK;
		break;
	case IPL_3XDAMVDEM:
		item[ii]._iFlags |= ISPL_3XDAMVDEM;
		break;
	case IPL_ALLRESZERO:
		item[ii]._iFlags |= ISPL_ALLRESZERO;
		break;
	case IPL_NOHEALMON:
		item[ii]._iFlags |= ISPL_NOHEALMON;
		break;
	case IPL_STEALMANA:
		if (param1 == 3)
			item[ii]._iFlags |= ISPL_STEALMANA_3;
		if (param1 == 5)
			item[ii]._iFlags |= ISPL_STEALMANA_5;
		drawmanaflag = TRUE;
		break;
	case IPL_STEALLIFE:
		if (param1 == 3)
			item[ii]._iFlags |= ISPL_STEALLIFE_3;
		if (param1 == 5)
			item[ii]._iFlags |= ISPL_STEALLIFE_5;
		drawhpflag = TRUE;
		break;
	case IPL_TARGAC:
#ifdef HELLFIRE
		item[ii]._iPLEnAc = param1;
#else
		item[ii]._iPLEnAc += r;
#endif
		break;
	case IPL_FASTATTACK:
		if (param1 == 1)
			item[ii]._iFlags |= ISPL_QUICKATTACK;
		if (param1 == 2)
			item[ii]._iFlags |= ISPL_FASTATTACK;
		if (param1 == 3)
			item[ii]._iFlags |= ISPL_FASTERATTACK;
		if (param1 == 4)
			item[ii]._iFlags |= ISPL_FASTESTATTACK;
		break;
	case IPL_FASTRECOVER:
		if (param1 == 1)
			item[ii]._iFlags |= ISPL_FASTRECOVER;
		if (param1 == 2)
			item[ii]._iFlags |= ISPL_FASTERRECOVER;
		if (param1 == 3)
			item[ii]._iFlags |= ISPL_FASTESTRECOVER;
		break;
	case IPL_FASTBLOCK:
		item[ii]._iFlags |= ISPL_FASTBLOCK;
		break;
	case IPL_DAMMOD:
		item[ii]._iPLDamMod += r;
		break;
	case IPL_RNDARROWVEL:
		item[ii]._iFlags |= ISPL_RNDARROWVEL;
		break;
	case IPL_SETDAM:
		item[ii]._iMinDam = param1;
		item[ii]._iMaxDam = param2;
		break;
	case IPL_SETDUR:
		item[ii]._iDurability = param1;
		item[ii]._iMaxDur = param1;
		break;
	case IPL_FASTSWING:
		item[ii]._iFlags |= ISPL_FASTERATTACK;
		break;
	case IPL_ONEHAND:
		item[ii]._iLoc = ILOC_ONEHAND;
		break;
	case IPL_DRAINLIFE:
		item[ii]._iFlags |= ISPL_DRAINLIFE;
		break;
	case IPL_RNDSTEALLIFE:
		item[ii]._iFlags |= ISPL_RNDSTEALLIFE;
		break;
	case IPL_INFRAVISION:
		item[ii]._iFlags |= ISPL_INFRAVISION;
		break;
	case IPL_NOMINSTR:
		item[ii]._iMinStr = 0;
		break;
	case IPL_INVCURS:
		item[ii]._iCurs = param1;
		break;
	case IPL_ADDACLIFE:
#ifdef HELLFIRE
		item[ii]._iFlags |= (ISPL_LIGHT_ARROWS | ISPL_FIRE_ARROWS);
		item[ii]._iFMinDam = param1;
		item[ii]._iFMaxDam = param2;
		item[ii]._iLMinDam = 1;
		item[ii]._iLMaxDam = 0;
#else
		item[ii]._iPLHP = (plr[myplr]._pIBonusAC + plr[myplr]._pIAC + plr[myplr]._pDexterity / 5) << 6;
#endif
		break;
	case IPL_ADDMANAAC:
#ifdef HELLFIRE
		item[ii]._iFlags |= (ISPL_LIGHTDAM | ISPL_FIREDAM);
		item[ii]._iFMinDam = param1;
		item[ii]._iFMaxDam = param2;
		item[ii]._iLMinDam = 2;
		item[ii]._iLMaxDam = 0;
#else
		item[ii]._iAC += (plr[myplr]._pMaxManaBase >> 6) / 10;
#endif
		break;
	case IPL_FIRERESCLVL:
		item[ii]._iPLFR = 30 - plr[myplr]._pLevel;
		if (item[ii]._iPLFR < 0)
			item[ii]._iPLFR = 0;
		break;
#ifdef HELLFIRE
	case IPL_FIRERES_CURSE:
		item[ii]._iPLFR -= r;
		break;
	case IPL_LIGHTRES_CURSE:
		item[ii]._iPLLR -= r;
		break;
	case IPL_MAGICRES_CURSE:
		item[ii]._iPLMR -= r;
		break;
	case IPL_ALLRES_CURSE:
		item[ii]._iPLFR -= r;
		item[ii]._iPLLR -= r;
		item[ii]._iPLMR -= r;
		break;
	case IPL_DEVASTATION:
		item[ii]._iDamAcFlags |= 0x01;
		break;
	case IPL_DECAY:
		item[ii]._iDamAcFlags |= 0x02;
		item[ii]._iPLDam += r;
		break;
	case IPL_PERIL:
		item[ii]._iDamAcFlags |= 0x04;
		break;
	case IPL_JESTERS:
		item[ii]._iDamAcFlags |= 0x08;
		break;
	case IPL_ACDEMON:
		item[ii]._iDamAcFlags |= 0x20;
		break;
	case IPL_ACUNDEAD:
		item[ii]._iDamAcFlags |= 0x40;
		break;
	case IPL_MANATOLIFE:
		r2 = ((plr[myplr]._pMaxManaBase >> 6) * 50 / 100);
		item[ii]._iPLMana -= (r2 << 6);
		item[ii]._iPLHP += (r2 << 6);
		break;
	case IPL_LIFETOMANA:
		r2 = ((plr[myplr]._pMaxHPBase >> 6) * 40 / 100);
		item[ii]._iPLHP -= (r2 << 6);
		item[ii]._iPLMana += (r2 << 6);
		break;
#endif
	}
	if (item[ii]._iVAdd1 || item[ii]._iVMult1) {
		item[ii]._iVAdd2 = PLVal(r, param1, param2, minval, maxval);
		item[ii]._iVMult2 = multval;
	} else {
		item[ii]._iVAdd1 = PLVal(r, param1, param2, minval, maxval);
		item[ii]._iVMult1 = multval;
	}
}

void GetItemPower(int ii, int minlvl, int maxlvl, int flgs, BOOL onlygood)
{
	int pre, post, nt, nl, j, preidx, sufidx;
	int l[256];
	char istr[128];
	BYTE goe;

	pre = random_(23, 4);
	post = random_(23, 3);
	if (pre != 0 && post == 0) {
		if (random_(23, 2))
			post = 1;
		else
			pre = 0;
	}
	preidx = -1;
	sufidx = -1;
	goe = 0;
	if (!onlygood && random_(0, 3))
		onlygood = TRUE;
	if (!pre) {
		nt = 0;
		for (j = 0; PL_Prefix[j].PLPower != -1; j++) {
			if (flgs & PL_Prefix[j].PLIType) {
				if (PL_Prefix[j].PLMinLvl >= minlvl && PL_Prefix[j].PLMinLvl <= maxlvl && (!onlygood || PL_Prefix[j].PLOk) && (flgs != 256 || PL_Prefix[j].PLPower != 15)) {
					l[nt] = j;
					nt++;
					if (PL_Prefix[j].PLDouble) {
						l[nt] = j;
						nt++;
					}
				}
			}
		}
		if (nt != 0) {
			preidx = l[random_(23, nt)];
			sprintf(istr, "%s %s", PL_Prefix[preidx].PLName, item[ii]._iIName);
			strcpy(item[ii]._iIName, istr);
			item[ii]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    ii,
			    PL_Prefix[preidx].PLPower,
			    PL_Prefix[preidx].PLParam1,
			    PL_Prefix[preidx].PLParam2,
			    PL_Prefix[preidx].PLMinVal,
			    PL_Prefix[preidx].PLMaxVal,
			    PL_Prefix[preidx].PLMultVal);
			item[ii]._iPrePower = PL_Prefix[preidx].PLPower;
			goe = PL_Prefix[preidx].PLGOE;
		}
	}
	if (post != 0) {
		nl = 0;
		for (j = 0; PL_Suffix[j].PLPower != -1; j++) {
			if (PL_Suffix[j].PLIType & flgs
			    && PL_Suffix[j].PLMinLvl >= minlvl && PL_Suffix[j].PLMinLvl <= maxlvl
			    && (goe | PL_Suffix[j].PLGOE) != 0x11
			    && (!onlygood || PL_Suffix[j].PLOk)) {
				l[nl] = j;
				nl++;
			}
		}
		if (nl != 0) {
			sufidx = l[random_(23, nl)];
			sprintf(istr, "%s of %s", item[ii]._iIName, PL_Suffix[sufidx].PLName);
			strcpy(item[ii]._iIName, istr);
			item[ii]._iMagical = ITEM_QUALITY_MAGIC;
			SaveItemPower(
			    ii,
			    PL_Suffix[sufidx].PLPower,
			    PL_Suffix[sufidx].PLParam1,
			    PL_Suffix[sufidx].PLParam2,
			    PL_Suffix[sufidx].PLMinVal,
			    PL_Suffix[sufidx].PLMaxVal,
			    PL_Suffix[sufidx].PLMultVal);
			item[ii]._iSufPower = PL_Suffix[sufidx].PLPower;
		}
	}
	if (!control_WriteStringToBuffer((BYTE *)item[ii]._iIName)) {
		strcpy(item[ii]._iIName, AllItemsList[item[ii].IDidx].iSName);
		if (preidx != -1) {
			sprintf(istr, "%s %s", PL_Prefix[preidx].PLName, item[ii]._iIName);
			strcpy(item[ii]._iIName, istr);
		}
		if (sufidx != -1) {
			sprintf(istr, "%s of %s", item[ii]._iIName, PL_Suffix[sufidx].PLName);
			strcpy(item[ii]._iIName, istr);
		}
	}
	if (preidx != -1 || sufidx != -1)
		CalcItemValue(ii);
}

void GetItemBonus(int ii, int idata, int minlvl, int maxlvl, BOOL onlygood, BOOLEAN allowspells)
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
				GetItemPower(ii, minlvl, maxlvl, 0x100, onlygood);
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
	ItemStruct* is;
	int it;

	is = &item[ii];
	it = ItemCAnimTbl[is->_iCurs];
	is->_iAnimData = itemanims[it];
	is->_iAnimLen = ItemAnimLs[it];
	is->_iAnimWidth = 96;
	is->_iAnimWidth2 = 16;
	is->_iIdentified = FALSE;
	is->_iPostDraw = FALSE;

	if (!plr[myplr].pLvlLoad) {
		is->_iAnimFrame = 1;
		is->_iAnimFlag = TRUE;
		is->_iSelFlag = 0;
	} else {
		is->_iAnimFrame = is->_iAnimLen;
		is->_iAnimFlag = FALSE;
		is->_iSelFlag = 1;
	}
}

int RndItem(int m)
{
	int i, ri, r;
	int ril[512];

	if ((monster[m].MData->mTreasure & 0x8000) != 0)
		return -1 - (monster[m].MData->mTreasure & 0xFFF);

	if (monster[m].MData->mTreasure & 0x4000)
		return 0;

	if (random_(24, 100) > 40)
		return 0;

	if (random_(24, 100) > 25)
		return IDI_GOLD + 1;

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		if (AllItemsList[i].iRnd == IDROP_DOUBLE && monster[m].mLevel >= AllItemsList[i].iMinMLvl
#ifdef HELLFIRE
			&& ri < 512
#endif
		) {
			ril[ri] = i;
			ri++;
		}
		if (AllItemsList[i].iRnd && monster[m].mLevel >= AllItemsList[i].iMinMLvl
#ifdef HELLFIRE
			&& ri < 512
#endif
		) {
			ril[ri] = i;
			ri++;
		}
		if (AllItemsList[i].iSpell == SPL_RESURRECT && gbMaxPlayers == 1)
			ri--;
		if (AllItemsList[i].iSpell == SPL_HEALOTHER && gbMaxPlayers == 1)
			ri--;
	}

	r = random_(24, ri);
	return ril[r] + 1;
}

int RndUItem(int m)
{
	int i, ri;
	int ril[512];
	BOOL okflag;

	if (m != -1 && (monster[m].MData->mTreasure & 0x8000) != 0 && gbMaxPlayers == 1)
		return -1 - (monster[m].MData->mTreasure & 0xFFF);

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		okflag = TRUE;
		if (!AllItemsList[i].iRnd)
			okflag = FALSE;
		if (m != -1) {
			if (monster[m].mLevel < AllItemsList[i].iMinMLvl)
				okflag = FALSE;
		} else {
#ifdef HELLFIRE
			if (2 * curlv < AllItemsList[i].iMinMLvl)
#else
			if (2 * currlevel < AllItemsList[i].iMinMLvl)
#endif
				okflag = FALSE;
		}
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

int RndAllItems()
{
	int i, ri;
	int ril[512];

	if (random_(26, 100) > 25)
		return 0;

#ifdef HELLFIRE
	int curlv = items_get_currlevel();
#endif
	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
#ifdef HELLFIRE
		if (AllItemsList[i].iRnd && 2 * curlv >= AllItemsList[i].iMinMLvl && ri < 512) {
#else
		if (AllItemsList[i].iRnd && 2 * currlevel >= AllItemsList[i].iMinMLvl) {
#endif
			ril[ri] = i;
			ri++;
		}
		if (AllItemsList[i].iSpell == SPL_RESURRECT && gbMaxPlayers == 1)
			ri--;
		if (AllItemsList[i].iSpell == SPL_HEALOTHER && gbMaxPlayers == 1)
			ri--;
	}

	return ril[random_(26, ri)];
}

int RndTypeItems(int itype, int imid, int lvl)
{
	int i, ri;
	BOOL okflag;
	int ril[512];

	ri = 0;
	for (i = 0; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
		okflag = TRUE;
		if (!AllItemsList[i].iRnd)
			okflag = FALSE;
		if (lvl << 1 < AllItemsList[i].iMinMLvl)
			okflag = FALSE;
		if (AllItemsList[i].itype != itype)
			okflag = FALSE;
		if (imid != -1 && AllItemsList[i].iMiscId != imid)
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
	int ii, itype, lvl;

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	if (numitems >= MAXITEMS)
		return;

	ii = itemavail[0];
	GetSuperItemSpace(x, y, ii);
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	itemactive[numitems] = ii;

	itype = 0;
	while (AllItemsList[itype].iItemId != UniqueItemList[uid].UIItemId) {
		itype++;
	}

	GetItemAttrs(ii, itype, lvl);
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
		if (random_(32, 100) <= 10 || random_(33, 100) <= lvl) {
			iblvl = lvl;
		}
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_STAFF) {
			iblvl = lvl;
		}
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_RING) {
			iblvl = lvl;
		}
		if (iblvl == -1 && item[ii]._iMiscId == IMISC_AMULET) {
			iblvl = lvl;
		}
		if (onlygood)
			iblvl = lvl;
		if (uper == 15)
			iblvl = lvl + 4;
		if (iblvl != -1) {
			uid = CheckUnique(ii, iblvl, uper, recreate);
			if (uid == UITYPE_INVALID) {
				GetItemBonus(ii, idx, iblvl >> 1, iblvl, onlygood, TRUE);
			} else {
				GetUniqueItem(ii, uid);
				item[ii]._iCreateInfo |= CF_UNIQUE;
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
	int ii, idx;
	BOOL onlygood;

	if (monster[mnum]._uniqtype || ((monster[mnum].MData->mTreasure & 0x8000) && gbMaxPlayers != 1)) {
		idx = RndUItem(mnum);
		if (idx < 0) {
			SpawnUnique(-(idx + 1), x, y);
			return;
		}
		onlygood = TRUE;
	} else if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 != QS_MUSHGIVEN) {
		idx = RndItem(mnum);
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
		if (monster[mnum]._uniqtype) {
			SetupAllItems(ii, idx, GetRndSeed(), monster[mnum].MData->mLevel, 15, onlygood, FALSE, FALSE);
		} else {
			SetupAllItems(ii, idx, GetRndSeed(), monster[mnum].MData->mLevel, 1, onlygood, FALSE, FALSE);
		}
		numitems++;
		if (sendmsg)
			NetSendCmdDItem(FALSE, ii);
	}
}

void CreateRndItem(int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta)
{
	int idx, ii, lvl;

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	if (onlygood)
		idx = RndUItem(-1);
	else
		idx = RndAllItems();

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		SetupAllItems(ii, idx, GetRndSeed(), 2 * lvl, 1, onlygood, FALSE, delta);
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
		idx = IDI_PORTAL;
		if ((lvl <= 1))
			idx = IDI_HEAL;
		break;
	case 1:
	case 2:
		idx = IDI_HEAL;
		break;
	case 3:
		idx = IDI_PORTAL;
		if ((lvl <= 1))
			idx = IDI_MANA;
		break;
	case 4:
	case 5:
		idx = IDI_MANA;
		break;
	case 6:
		idx = IDI_OIL;
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

	if (lvl > 1 && !random_(34, 3))
		idx = IDI_PORTAL;
#endif

	GetItemAttrs(ii, idx, lvl);
	item[ii]._iCreateInfo = lvl + CF_USEFUL;
	SetupItem(ii);
}

void CreateRndUseful(int pnum, int x, int y, BOOL sendmsg)
{
	int ii, lvl;

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
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

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	if (itype != ITYPE_GOLD)
		idx = RndTypeItems(itype, imisc, lvl);
	else
		idx = 0;

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		SetupAllItems(ii, idx, GetRndSeed(), 2 * lvl, 1, onlygood, FALSE, delta);

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
	if (!idx) {
		SetPlrHandItem(is, IDI_GOLD);
		is->_iSeed = iseed;
		is->_iCreateInfo = icreateinfo;
		SetGoldItemValue(is, ivalue);
	} else {
		if (!icreateinfo) {
			SetPlrHandItem(is, idx);
			SetPlrHandSeed(is, iseed);
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
	if (dItem[x][y]) {
		ii = dItem[x][y] - 1;
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

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	if (randarea) {
		tries = 0;
		while (1) {
			tries++;
			if (tries > 1000 && randarea > 1)
				randarea--;
			x = random_(0, MAXDUNX);
			y = random_(0, MAXDUNY);
			failed = FALSE;
			for (i = 0; i < randarea && !failed; i++) {
				for (j = 0; j < randarea && !failed; j++) {
					failed = !ItemSpaceOk(i + x, j + y);
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
		if (selflag) {
			item[i]._iSelFlag = selflag;
			item[i]._iAnimFrame = item[i]._iAnimLen;
			item[i]._iAnimFlag = FALSE;
		}
		numitems++;
	}
}

void SpawnRock()
{
	int i, ii, lvl;
	int xx, yy;
	int ostand;

	ostand = FALSE;
	for (i = 0; i < nobjects && !ostand; i++) {
		ii = objectactive[i];
		ostand = object[ii]._otype == OBJ_STAND;
	}
#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	if (ostand) {
		i = itemavail[0];
		itemavail[0] = itemavail[127 - numitems - 1];
		itemactive[numitems] = i;
		xx = object[ii]._ox;
		yy = object[ii]._oy;
		item[i]._ix = xx;
		item[i]._iy = yy;
		dItem[xx][item[i]._iy] = i + 1;
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
	itemavail[0] = itemavail[127 - numitems - 1];
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
	int idoppelx;
	ItemStruct *i;

	if (gbMaxPlayers != 1) {
		for (idoppelx = 16; idoppelx < 96; idoppelx++) {
			if (dItem[idoppelx][idoppely]) {
				i = &item[dItem[idoppelx][idoppely] - 1];
				if (i->_ix != idoppelx || i->_iy != idoppely)
					dItem[idoppelx][idoppely] = 0;
			}
		}
		idoppely++;
		if (idoppely == 96)
			idoppely = 16;
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
#ifdef HELLFIRE
	DWORD i;
#else
	int i;
#endif

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

	if (pnum == myplr)
		SetCursor_(CURSOR_HAND);
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

	if (pnum == myplr)
		SetCursor_(CURSOR_HAND);
}

void RepairItem(ItemStruct *is, int lvl)
{
	int rep, d;

	if (is->_iDurability == is->_iMaxDur) {
		return;
	}

	if (is->_iMaxDur <= 0) {
		is->_itype = ITYPE_NONE;
		return;
	}

	rep = 0;
	do {
		rep += lvl + random_(37, lvl);
		d = is->_iMaxDur / (lvl + 9);
		if (d < 1)
			d = 1;
		is->_iMaxDur = is->_iMaxDur - d;
		if (!is->_iMaxDur) {
			is->_itype = ITYPE_NONE;
			return;
		}
	} while (rep + is->_iDurability < is->_iMaxDur);

	is->_iDurability += rep;
	if (is->_iDurability > is->_iMaxDur)
		is->_iDurability = is->_iMaxDur;
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

	if (pnum == myplr)
		SetCursor_(CURSOR_HAND);
}

#ifdef HELLFIRE
void DoOil(int pnum, int cii)
{
	PlayerStruct *p = &plr[pnum];

	if (cii >= NUM_INVLOC || cii == INVLOC_HEAD || (cii > INVLOC_AMULET && cii <= INVLOC_CHEST)) {
		if (OilItem(&p->InvBody[cii], p)) {
			CalcPlrInv(pnum, TRUE);
			if (pnum == myplr) {
				SetCursor_(CURSOR_HAND);
			}
		}
	}
}

BOOL OilItem(ItemStruct *is, PlayerStruct *p)
{
	int dur, r;

	if (is->_iClass == ICLASS_MISC) {
		return FALSE;
	}
	if (is->_iClass == ICLASS_GOLD) {
		return FALSE;
	}
	if (is->_iClass == ICLASS_QUEST) {
		return FALSE;
	}

	switch (p->_pOilType) {
	case IMISC_OILACC:
	case IMISC_OILMAST:
	case IMISC_OILSHARP:
		if (is->_iClass == ICLASS_ARMOR) {
			return FALSE;
		}
		break;
	case IMISC_OILDEATH:
		if (is->_iClass == ICLASS_ARMOR) {
			return FALSE;
		}
		if (is->_itype == ITYPE_BOW) {
			return FALSE;
		}
		break;
	case IMISC_OILHARD:
	case IMISC_OILIMP:
		if (is->_iClass == ICLASS_WEAPON) {
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
			is->_iMaxDam = is->_iMaxDam + 1;
		}
		break;
	case IMISC_OILDEATH:
		if (is->_iMaxDam - is->_iMinDam < 30) {
			is->_iMinDam = is->_iMinDam + 1;
			is->_iMaxDam = is->_iMaxDam + 2;
		}
		break;
	case IMISC_OILSKILL:
		r = random_(68, 6) + 5;
		if (is->_iMinStr > r) {
			is->_iMinStr = is->_iMinStr - r;
		} else {
			is->_iMinStr = 0;
		}
		if (is->_iMinMag > r) {
			is->_iMinMag = is->_iMinMag - r;
		} else {
			is->_iMinMag = 0;
		}
		if (is->_iMinDex > r) {
			is->_iMinDex = is->_iMinDex - r;
		} else {
			is->_iMinDex = 0;
		}
		break;
	case IMISC_OILBSMTH:
		if (is->_iMaxDur != 255) {
			if (is->_iDurability < is->_iMaxDur) {
				dur = (is->_iMaxDur + 4) / 5 + is->_iDurability;
				if (dur > is->_iMaxDur) {
					dur = is->_iMaxDur;
				}
			} else {
				if (is->_iMaxDur >= 100) {
					return TRUE;
				}
				dur = is->_iMaxDur + 1;
				is->_iMaxDur = dur;
			}
			is->_iDurability = dur;
		}
		break;
	case IMISC_OILFORT:
		if (is->_iMaxDur != 255 && is->_iMaxDur < 200) {
			r = random_(68, 41) + 10;
			is->_iMaxDur += r;
			is->_iDurability += r;
		}
		break;
	case IMISC_OILPERM:
		is->_iDurability = 255;
		is->_iMaxDur = 255;
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
	return TRUE;
}

#endif
void RechargeItem(ItemStruct *is, int r)
{
	if (is->_iCharges != is->_iMaxCharges) {
		do {
			is->_iMaxCharges--;
			if (is->_iMaxCharges == 0) {
				return;
			}
			is->_iCharges += r;
		} while (is->_iCharges < is->_iMaxCharges);
		if (is->_iCharges > is->_iMaxCharges)
			is->_iCharges = is->_iMaxCharges;
	}
}

void PrintItemOil(char IDidx)
{
	switch (IDidx) {
#ifdef HELLFIRE
	case IMISC_OILACC:
		strcpy(tempstr, "increases a weapon\'s");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "chance to hit");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILMAST:
		strcpy(tempstr, "greatly increases a");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "weapon\'s chance to hit");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILSHARP:
		strcpy(tempstr, "increases a weapon\'s");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "damage potential");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILDEATH:
		strcpy(tempstr, "greatly increases a weapon\'s");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "damage potential - not bows");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILSKILL:
		strcpy(tempstr, "reduces attributes needed");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "to use armor or weapons");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILBSMTH:
		strcpy(tempstr, "restores 20% of an");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "item\'s durability");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILFORT:
		strcpy(tempstr, "increases an item\'s");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "current and max durability");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILPERM:
		strcpy(tempstr, "makes an item indestructible");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILHARD:
		strcpy(tempstr, "increases the armor class");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "of armor and shields");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OILIMP:
		strcpy(tempstr, "greatly increases the armor");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "class of armor and shields");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_RUNEF:
		strcpy(tempstr, "sets fire trap");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_RUNEL:
		strcpy(tempstr, "sets lightning trap");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_GR_RUNEL:
		strcpy(tempstr, "sets lightning trap");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_GR_RUNEF:
		strcpy(tempstr, "sets fire trap");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_RUNES:
		strcpy(tempstr, "sets petrification trap");
		AddPanelString(tempstr, TRUE);
		break;
#endif
	case IMISC_FULLHEAL:
		strcpy(tempstr, "fully recover life");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_HEAL:
		strcpy(tempstr, "recover partial life");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_OLDHEAL:
		strcpy(tempstr, "recover life");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_DEADHEAL:
		strcpy(tempstr, "deadly heal");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_MANA:
		strcpy(tempstr, "recover mana");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_FULLMANA:
		strcpy(tempstr, "fully recover mana");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXSTR:
		strcpy(tempstr, "increase strength");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXMAG:
		strcpy(tempstr, "increase magic");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXDEX:
		strcpy(tempstr, "increase dexterity");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXVIT:
		strcpy(tempstr, "increase vitality");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXWEAK:
		strcpy(tempstr, "decrease strength");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXDIS:
		strcpy(tempstr, "decrease strength");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXCLUM:
		strcpy(tempstr, "decrease dexterity");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_ELIXSICK:
		strcpy(tempstr, "decrease vitality");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_REJUV:
		strcpy(tempstr, "recover life and mana");
		AddPanelString(tempstr, TRUE);
		break;
	case IMISC_FULLREJUV:
		strcpy(tempstr, "fully recover life and mana");
		AddPanelString(tempstr, TRUE);
		break;
	}
}

void PrintItemPower(char plidx, ItemStruct *is)
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
		sprintf(tempstr, "armor class: %i", is->_iAC);
		break;
	case IPL_AC_CURSE:
		sprintf(tempstr, "armor class: %i", is->_iAC);
		break;
	case IPL_FIRERES:
#ifdef HELLFIRE
	case IPL_FIRERES_CURSE:
#endif
		if (is->_iPLFR < 75)
			sprintf(tempstr, "Resist Fire: %+i%%", is->_iPLFR);
		if (is->_iPLFR >= 75)
			sprintf(tempstr, "Resist Fire: 75%% MAX");
		break;
	case IPL_LIGHTRES:
#ifdef HELLFIRE
	case IPL_LIGHTRES_CURSE:
#endif
		if (is->_iPLLR < 75)
			sprintf(tempstr, "Resist Lightning: %+i%%", is->_iPLLR);
		if (is->_iPLLR >= 75)
			sprintf(tempstr, "Resist Lightning: 75%% MAX");
		break;
	case IPL_MAGICRES:
#ifdef HELLFIRE
	case IPL_MAGICRES_CURSE:
#endif
		if (is->_iPLMR < 75)
			sprintf(tempstr, "Resist Magic: %+i%%", is->_iPLMR);
		if (is->_iPLMR >= 75)
			sprintf(tempstr, "Resist Magic: 75%% MAX");
		break;
	case IPL_ALLRES:
#ifdef HELLFIRE
	case IPL_ALLRES_CURSE:
#endif
		if (is->_iPLFR < 75)
			sprintf(tempstr, "Resist All: %+i%%", is->_iPLFR);
		if (is->_iPLFR >= 75)
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
		else if (is->_iPLFR >= 1)
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

void PrintUString(int x, int y, BOOL cjustflag, char *str, int col)
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
		if (c && k <= 257) {
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

void PrintItemMisc(ItemStruct *is)
{
	if (is->_iMiscId == IMISC_SCROLL) {
		strcpy(tempstr, "Right-click to read");
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iMiscId == IMISC_SCROLLT) {
		strcpy(tempstr, "Right-click to read, then");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "left-click to target");
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iMiscId >= IMISC_USEFIRST && is->_iMiscId <= IMISC_USELAST) {
		PrintItemOil(is->_iMiscId);
		strcpy(tempstr, "Right-click to use");
		AddPanelString(tempstr, TRUE);
	}
#ifdef HELLFIRE
	if (is->_iMiscId > IMISC_OILFIRST && is->_iMiscId < IMISC_OILLAST) {
		PrintItemOil(is->_iMiscId);
		strcpy(tempstr, "Right click to use");
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iMiscId > IMISC_RUNEFIRST && is->_iMiscId < IMISC_RUNELAST) {
		PrintItemOil(is->_iMiscId);
		strcpy(tempstr, "Right click to use");
		AddPanelString(tempstr, TRUE);
	}
#endif
	if (is->_iMiscId == IMISC_BOOK) {
		strcpy(tempstr, "Right-click to read");
		AddPanelString(tempstr, TRUE);
	}
#ifdef HELLFIRE
	if (is->_iMiscId == IMISC_NOTE) {
		strcpy(tempstr, "Right click to read");
		AddPanelString(tempstr, TRUE);
	}
#endif
	if (is->_iMiscId == IMISC_MAPOFDOOM) {
		strcpy(tempstr, "Right-click to view");
		AddPanelString(tempstr, TRUE);
	}
	if (is->_iMiscId == IMISC_EAR) {
		sprintf(tempstr, "Level : %i", is->_ivalue);
		AddPanelString(tempstr, TRUE);
	}
#ifdef HELLFIRE
	if (is->_iMiscId == IMISC_AURIC) {
		sprintf(tempstr, "Doubles gold capacity");
		AddPanelString(tempstr, TRUE);
	}
#endif
}

void PrintItemDetails(ItemStruct *is)
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

void PrintItemDur(ItemStruct *is)
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
		if (p->_pSplLvl[spl] < 15)
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
		SetCursor_(CURSOR_OIL);
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
			SetCursor_(CURSOR_TELEPORT);
		break;
	case IMISC_RUNEL:
		plr[pnum]._pTSpell = SPL_RUNELIGHT;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			SetCursor_(CURSOR_TELEPORT);
		break;
	case IMISC_GR_RUNEL:
		plr[pnum]._pTSpell = SPL_RUNENOVA;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			SetCursor_(CURSOR_TELEPORT);
		break;
	case IMISC_GR_RUNEF:
		plr[pnum]._pTSpell = SPL_RUNEIMMOLAT;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			SetCursor_(CURSOR_TELEPORT);
		break;
	case IMISC_RUNES:
		plr[pnum]._pTSpell = SPL_RUNESTONE;
		plr[pnum]._pTSplType = RSPLTYPE_INVALID;
		if (pnum == myplr)
			SetCursor_(CURSOR_TELEPORT);
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
	 && AllItemsList[i].itype != ITYPE_MEAT
#ifdef HELLFIRE
	 && (AllItemsList[i].itype != ITYPE_STAFF || AllItemsList[i].iSpell == SPL_NULL)
#else
	 && AllItemsList[i].itype != ITYPE_STAFF
#endif
	 && AllItemsList[i].itype != ITYPE_RING
	 && AllItemsList[i].itype != ITYPE_AMULET;
}

int RndSmithItem(int lvl)
{
	int i, ri;
	int ril[512];

	ri = 0;
	for (i = 1; AllItemsList[i].iLoc != ILOC_INVALID; i++) {
#ifdef HELLFIRE
		if (AllItemsList[i].iRnd && SmithItemOk(i) && lvl >= AllItemsList[i].iMinMLvl && ri < 512) {
#else
		if (AllItemsList[i].iRnd && SmithItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
#endif
			ril[ri] = i;
			ri++;
#ifdef HELLFIRE
			if (AllItemsList[i].iRnd == IDROP_DOUBLE && ri < 512) {
#else
			if (AllItemsList[i].iRnd == IDROP_DOUBLE)
				{
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
		ItemStatOk(myplr, &smithitem[i]);
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
		if (AllItemsList[i].iRnd) {
			if (PremiumItemOk(i)) {
#ifdef HELLFIRE
				if (AllItemsList[i].iMinMLvl >= minlvl && AllItemsList[i].iMinMLvl <= maxlvl && ri < 512) {
#else
				if (AllItemsList[i].iMinMLvl >= minlvl && AllItemsList[i].iMinMLvl <= maxlvl) {
#endif
					ril[ri] = i;
					ri++;
				}
			}
		}
	}

	return ril[random_(50, ri)] + 1;
}

void SpawnOnePremium(int i, int plvl)
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
		GetItemBonus(0, itype, plvl >> 1, plvl, TRUE, FALSE);
	} while (item[0]._iIvalue > SMITH_MAX_PREMIUM_VALUE);
	premiumitem[i] = item[0];
	premiumitem[i]._iCreateInfo = plvl | CF_SMITHPREMIUM;
	premiumitem[i]._iIdentified = TRUE;
	ItemStatOk(myplr, &premiumitem[i]);
	item[0] = holditem;
}

#ifdef HELLFIRE
void SpawnPremium(int pnum)
#else
void SpawnPremium(int lvl)
#endif
{
	int i;

#ifdef HELLFIRE
	int lvl = plr[pnum]._pLevel;
#endif
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
#ifdef HELLFIRE
		if (AllItemsList[i].iRnd && WitchItemOk(i) && lvl >= AllItemsList[i].iMinMLvl && ri < 512) {
#else
		if (AllItemsList[i].iRnd && WitchItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
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

void WitchBookLevel(int ii)
{
	int slvl;

	if (witchitem[ii]._iMiscId == IMISC_BOOK) {
		witchitem[ii]._iMinMag = spelldata[witchitem[ii]._iSpell].sMinInt;
		slvl = plr[myplr]._pSplLvl[witchitem[ii]._iSpell];
		while (slvl) {
			witchitem[ii]._iMinMag += 20 * witchitem[ii]._iMinMag / 100;
			slvl--;
			if (witchitem[ii]._iMinMag + 20 * witchitem[ii]._iMinMag / 100 > 255) {
				witchitem[ii]._iMinMag = 255;
				slvl = 0;
			}
		}
	}
}

void SpawnWitch(int lvl)
{
	int i, iCnt;
	int idata, maxlvl;

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
			maxlvl = -1;
			if (random_(51, 100) <= 5)
				maxlvl = 2 * lvl;
			if (maxlvl == -1 && item[0]._iMiscId == IMISC_STAFF)
				maxlvl = 2 * lvl;
			if (maxlvl != -1)
				GetItemBonus(0, idata, maxlvl >> 1, maxlvl, TRUE, TRUE);
		} while (item[0]._iIvalue > 140000);
		witchitem[i] = item[0];
		witchitem[i]._iCreateInfo = lvl | CF_WITCH;
		witchitem[i]._iIdentified = TRUE;
		WitchBookLevel(i);
		ItemStatOk(myplr, &witchitem[i]);
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
#ifdef HELLFIRE
		if (AllItemsList[i].iRnd && PremiumItemOk(i) && lvl >= AllItemsList[i].iMinMLvl && ri < 512) {
#else
		if (AllItemsList[i].iRnd && PremiumItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
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

	if (boylevel<lvl>> 1 || boyitem._itype == ITYPE_NONE) {
		do {
			item[0]._iSeed = GetRndSeed();
			SetRndSeed(item[0]._iSeed);
			itype = RndBoyItem(lvl) - 1;
			GetItemAttrs(0, itype, lvl);
			GetItemBonus(0, itype, lvl, 2 * lvl, TRUE, TRUE);
		} while (item[0]._iIvalue > 90000);
		boyitem = item[0];
		boyitem._iCreateInfo = lvl | CF_BOY;
		boyitem._iIdentified = TRUE;
		ItemStatOk(myplr, &boyitem);
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
#ifdef HELLFIRE
		if (AllItemsList[i].iRnd && HealerItemOk(i) && lvl >= AllItemsList[i].iMinMLvl && ri < 512) {
#else
		if (AllItemsList[i].iRnd && HealerItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
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
	iCnt = random_(50, 10) + 10;
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
		ItemStatOk(myplr, &healitem[i]);
	}
	for (i = iCnt; i < 20; i++) {
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
	GetItemBonus(ii, itype, plvl >> 1, plvl, TRUE, FALSE);

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
	GetItemBonus(ii, itype, lvl, 2 * lvl, TRUE, TRUE);
	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_BOY;
	item[ii]._iIdentified = TRUE;
}

void RecreateWitchItem(int ii, int idx, int lvl, int iseed)
{
	int iblvl, itype;

	if (idx == IDI_MANA || idx == IDI_FULLMANA || idx == IDI_PORTAL) {
		GetItemAttrs(ii, idx, lvl);
	} else {
#ifdef HELLFIRE
		if (idx >= 114 && idx <= 117) {
			SetRndSeed(iseed);
			volatile int hi_predelnik = random_(0, 1);
			iblvl = lvl;
			GetItemAttrs(ii, idx, iblvl);
		} else {
#endif
			SetRndSeed(iseed);
			itype = RndWitchItem(lvl) - 1;
			GetItemAttrs(ii, itype, lvl);
			iblvl = -1;
			if (random_(51, 100) <= 5)
				iblvl = 2 * lvl;
			if (iblvl == -1 && item[ii]._iMiscId == IMISC_STAFF)
				iblvl = 2 * lvl;
			if (iblvl != -1)
				GetItemBonus(ii, itype, iblvl >> 1, iblvl, TRUE, TRUE);
#ifdef HELLFIRE
		}
#endif
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

void RecalcStoreStats()
{
	int i;

	for (i = 0; i < SMITH_ITEMS; i++) {
		if (smithitem[i]._itype != ITYPE_NONE) {
			ItemStatOk(myplr, &smithitem[i]);
		}
	}
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++) {
		if (premiumitem[i]._itype != ITYPE_NONE) {
			ItemStatOk(myplr, &premiumitem[i]);
		}
	}
	for (i = 0; i < 20; i++) {
		if (witchitem[i]._itype != ITYPE_NONE) {
			ItemStatOk(myplr, &witchitem[i]);
		}
	}
	for (i = 0; i < 20; i++) {
		if (healitem[i]._itype != ITYPE_NONE) {
			ItemStatOk(myplr, &healitem[i]);
		}
	}
	ItemStatOk(myplr, &boyitem);
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
	idx = RndTypeItems(ITYPE_MISC, IMISC_BOOK, lvl);
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			SetupAllItems(ii, idx, GetRndSeed(), 2 * lvl, 1, TRUE, FALSE, delta);
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

#ifdef HELLFIRE
	lvl = items_get_currlevel();
#else
	lvl = currlevel;
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			idx = RndTypeItems(imisc, IMISC_NONE, lvl);
			SetupAllItems(ii, idx, GetRndSeed(), 2 * lvl, 1, TRUE, FALSE, delta);
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

	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			idx = RndTypeItems(ITYPE_AMULET, IMISC_AMULET, lvl);
			SetupAllItems(ii, idx, GetRndSeed(), 2 * lvl, 1, TRUE, FALSE, delta);
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

#ifdef HELLFIRE
	if (imisc == ITYPE_STAFF)
		imid = IMISC_STAFF;
	else
		imid = IMISC_NONE;
	lvl = items_get_currlevel();
#else
	imid = IMISC_NONE;
	lvl = currlevel;
#endif
	if (numitems < MAXITEMS) {
		ii = itemavail[0];
		GetSuperItemSpace(x, y, ii);
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		itemactive[numitems] = ii;
		while (TRUE) {
			idx = RndTypeItems(imisc, imid, lvl);
			SetupAllItems(ii, idx, GetRndSeed(), 2 * lvl, 1, TRUE, FALSE, delta);
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

void NextItemRecord(int i)
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

void SetItemRecord(int nSeed, WORD wCI, int nIndex)
{
	DWORD dwTicks;

	dwTicks = SDL_GetTicks();

	if (gnNumGetRecords == MAXITEMS) {
		return;
	}

	itemrecord[gnNumGetRecords].dwTimestamp = dwTicks;
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
