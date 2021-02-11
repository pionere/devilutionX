/**
 * @file items.cpp
 *
 * Implementation of item functionality.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int itemactive[MAXITEMS];
int itemavail[MAXITEMS];
ItemGetRecordStruct itemrecord[MAXITEMS];
/** Contains the items on ground in the current game. */
ItemStruct item[MAXITEMS + 1];
BYTE *itemanims[ITEMTYPES];
BOOL UniqueItemFlag[NUM_UITEM];
int numitems;
int gnNumGetRecords;

/** Maps from item_cursor_graphic to in-memory item (drop) type. */
const BYTE ItemCAnimTbl[NUM_ICURS] = {
	20, 16, 16, 16,  4,  4,  4, 12, 12, 12,
	12, 12, 12, 12, 12, 21, 21, 25, 12, 28,
	28, 28,  0,  0,  0, 32,  0,  0,  0, 24,
	24, 26,  2, 25, 22, 23, 24, 25, 27, 27,
	29,  0,  0,  0, 12, 12, 12, 12, 12,  0,
	 8,  8,  0,  8,  8,  8,  8,  8,  8,  6, // 50 ...
	 8,  8,  8,  6,  8,  8,  6,  8,  8,  6,
	 6,  6,  8,  8,  8,  5,  9, 13, 13, 13,
	 5,  5,  5, 15,  5,  5, 18, 18, 18, 30,
	 5,  5, 14,  5, 14, 13, 16, 18,  5,  5,
	 7,  1,  3, 17,  1, 15, 10, 14,  3, 11, // 100 ...
	 8,  0,  1,  7,  0,  7, 15,  7,  3,  3,
	 3,  6,  6, 11, 11, 11, 31, 14, 14, 14,
	 6,  6,  7,  3,  8, 14,  0, 14, 14,  0,
	33,  1,  1,  1,  1,  1,  7,  7,  7, 14,
	14, 17, 17, 17,  0, 34,  1,  0,  3, 17, // 150 ...
	 8,  8,  6,  1,  3,  3, 11,  3,
#ifdef HELLFIRE
	                                12, 12,
	12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	12, 12, 12, 12, 12, 12, 12, 35, 39, 36,
	36, 36, 37, 38, 38, 38, 38, 38, 41, 42,
	 8,  8,  8, 17,  0,  6,  8, 11, 11,  3, // 200 ...
	 3,  1,  6,  6,  6,  1,  8,  6, 11,  3,
	 6,  8,  1,  6,  6, 17, 40
#endif
};
/** Map of item type .cel file names. */
const char *const ItemDropNames[ITEMTYPES] = {
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
const BYTE ItemAnimLs[ITEMTYPES] = {
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
const int ItemDropSnds[ITEMTYPES] = {
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
const int ItemInvSnds[ITEMTYPES] = {
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
/** Specifies the current Y-coordinate used for validation of items on ground. */
int idoppely = DBORDERY;
/** Maps from Griswold premium item number to a quality level delta as added to the base quality level. */
const int premiumlvladd[SMITH_PREMIUM_ITEMS] = {
	// clang-format off
#ifdef HELLFIRE
	-1, -1, -1,  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  3,  3
#else
	-1, -1,  0,  0,  1,  2
#endif
	// clang-format on
};

#ifdef HELLFIRE
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
	SpawnQuestItemAt(id, x, y);
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
		snprintf(arglist, sizeof(arglist), "Items\\%s.CEL", ItemDropNames[i]);
		itemanims[i] = LoadFileInMem(arglist, NULL);
	}
	memset(UniqueItemFlag, 0, sizeof(UniqueItemFlag));
}

BOOL ItemPlace(int x, int y)
{
	return (dMonster[x][y] | dPlayer[x][y] | dItem[x][y] | dObject[x][y]
	 | (dFlags[x][y] & BFLAG_POPULATED) | nSolidTable[dPiece[x][y]]) == 0;
}

static void SetItemLoc(int ii, int x, int y)
{
	item[ii]._ix = x;
	item[ii]._iy = y;
	dItem[x][y] = ii + 1;
}

static void GetRandomItemSpace(int ii)
{
	int x, y;

	do {
		x = random_(12, DSIZEX) + DBORDERX;
		y = random_(12, DSIZEY) + DBORDERY;
	} while (!ItemPlace(x, y));
	SetItemLoc(ii, x, y);
}

static void AddInitItems()
{
	int ii, i, lvl, seed;

	lvl = items_get_currlevel();

	for (i = RandRange(3, 5); i != 0; i--) {
		ii = itemavail[0];
		seed = GetRndSeed();
		SetRndSeed(seed);
		SetItemData(ii, random_(12, 2) != 0 ? IDI_HEAL : IDI_MANA);
		item[ii]._iSeed = seed;
		item[ii]._iCreateInfo = lvl | CF_PREGEN;
		SetupItem(ii);
		item[ii]._iAnimFrame = item[ii]._iAnimLen;
		item[ii]._iAnimFlag = FALSE;
		item[ii]._iSelFlag = 1;

		GetRandomItemSpace(ii);
		DeltaAddItem(ii);

		itemactive[numitems] = ii;
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
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
			SpawnQuestItemAt(IDI_ANVIL, 2 * setpc_x + 27, 2 * setpc_y + 27);
#ifdef HELLFIRE
		if (UseCowFarmer && currlevel == 20)
			SpawnQuestItemInArea(IDI_BROWNSUIT, 3);
		if (UseCowFarmer && currlevel == 19)
			SpawnQuestItemInArea(IDI_GREYSUIT, 3);
#endif
		if (currlevel > 0 && currlevel < 16)
			AddInitItems();
#ifdef HELLFIRE
		if (currlevel >= 21 && currlevel <= 23)
			SpawnNote();
#endif
	}
}

/*
 * Calculate the arrow-velocity bonus gained from attack-speed modifiers.
 *  ISPL_QUICKATTACK:   +1
 *  ISPL_FASTATTACK:    +2
 *  ISPL_FASTERATTACK:  +4
 *  ISPL_FASTESTATTACK: +8
 */
inline static int ArrowVelBonus(unsigned flags)
{
	flags &= (ISPL_QUICKATTACK | ISPL_FASTATTACK | ISPL_FASTERATTACK | ISPL_FASTESTATTACK);
	if (flags != 0) {
		static_assert((ISPL_QUICKATTACK & (ISPL_QUICKATTACK - 1)) == 0, "Optimized ArrowVelBonus depends simple flag-like attack-speed modifiers.");
		static_assert(ISPL_QUICKATTACK == ISPL_FASTATTACK / 2, "ArrowVelBonus depends on ordered attack-speed modifiers I.");
		static_assert(ISPL_FASTATTACK == ISPL_FASTERATTACK / 2, "ArrowVelBonus depends on ordered attack-speed modifiers II.");
		static_assert(ISPL_FASTERATTACK == ISPL_FASTESTATTACK / 2, "ArrowVelBonus depends on ordered attack-speed modifiers III.");
		flags /= ISPL_QUICKATTACK;
	}
	return flags;
}

void CalcPlrItemVals(int pnum, BOOL Loadgfx)
{
	PlayerStruct *p;
	ItemStruct *pi;
	ItemStruct *wRight, *wLeft;

	int tac = 0;  // accuracy

	unsigned g;
	int i;

	int bdam = 0;   // bonus damage
	int btohit = 0; // bonus chance to hit
	int bac = 0;    // bonus accuracy
	int av;			// arrow velocity bonus

	int iflgs = ISPL_NONE; // item_special_effect flags
	int iflgs2 = ISPH_NONE;// item_special_effect flags2

	int sadd = 0; // added strength
	int madd = 0; // added magic
	int dadd = 0; // added dexterity
	int vadd = 0; // added vitality

	unsigned __int64 spl = 0; // bitarray for all enabled/active spells

	int fr = 0; // fire resistance
	int lr = 0; // lightning resistance
	int mr = 0; // magic resistance
	int ar = 0; // acid resistance

	int dmod = 0; // bonus damage mod
	int pdmod;    // player damage mod
	BYTE dms;     // number of damage types
	int ghit = 0; // increased damage from enemies
	BYTE manasteal = 0;
	BYTE lifesteal = 0;

	int lrad = 10; // light radius

	int ihp = 0;   // increased HP
	int imana = 0; // increased mana

	int spllvladd = 0; // increased spell level
	int enac = 0;      // enhanced accuracy

	unsigned minsl = 0; // min slash-damage
	unsigned maxsl = 0; // max slash-damage
	unsigned minbl = 0; // min blunt-damage
	unsigned maxbl = 0; // max blunt-damage
	unsigned minpc = 0; // min puncture-damage
	unsigned maxpc = 0; // max puncture-damage
	unsigned fmin = 0;  // min fire damage
	unsigned fmax = 0;  // max fire damage
	unsigned lmin = 0;  // min lightning damage
	unsigned lmax = 0;  // max lightning damage
	unsigned mmin = 0;  // min magic damage
	unsigned mmax = 0;  // max magic damage
	unsigned amin = 0;  // min acid damage
	unsigned amax = 0;  // max acid damage

	p = &plr[pnum];
	pi = p->InvBody;
	for (i = NUM_INVLOC; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE && pi->_iStatFlag) {
			tac += pi->_iAC;
			switch (pi->_iDamType) {
			case IDAM_NONE: break;
			case IDAM_SLASH:
				minsl += (unsigned)pi->_iMinDam << 1;
				maxsl += (unsigned)pi->_iMaxDam << 1;
				break;
			case IDAM_BLUNT:
				minbl += (unsigned)pi->_iMinDam << 1;
				maxbl += (unsigned)pi->_iMaxDam << 1;
				break;
			case IDAM_SB_MIX:
				minsl += pi->_iMinDam;
				minbl += pi->_iMinDam;
				maxsl += pi->_iMaxDam;
				maxbl += pi->_iMaxDam;
				break;
			case IDAM_PUNCTURE:
				minpc += (unsigned)pi->_iMinDam << 1;
				maxpc += (unsigned)pi->_iMaxDam << 1;
				break;
			default:
				ASSUME_UNREACHABLE
			}

			if (pi->_iSpell != SPL_NULL) {
				spl |= SPELL_MASK(pi->_iSpell);
			}

			if (pi->_iMagical == ITEM_QUALITY_NORMAL || pi->_iIdentified) {
				bdam += pi->_iPLDam;
				btohit += pi->_iPLToHit;
				if (pi->_iPLAC != 0) {
					int tmpac = pi->_iPLAC * pi->_iAC / 100;
					if (tmpac == 0)
						tmpac = pi->_iPLAC >= 0 ? 1 : -1;
					bac += tmpac;
				}
				iflgs |= pi->_iFlags;
				iflgs2 |= pi->_iFlags2;

				sadd += pi->_iPLStr;
				madd += pi->_iPLMag;
				dadd += pi->_iPLDex;
				vadd += pi->_iPLVit;
				fr += pi->_iPLFR;
				lr += pi->_iPLLR;
				mr += pi->_iPLMR;
				ar += pi->_iPLAR;
				dmod += pi->_iPLDamMod;
				ghit += pi->_iPLGetHit;
				lrad += pi->_iPLLight;
				ihp += pi->_iPLHP;
				imana += pi->_iPLMana;
				spllvladd += pi->_iSplLvlAdd;
				lifesteal += pi->_iLifeSteal;
				manasteal += pi->_iManaSteal;
				enac += pi->_iPLEnAc;
				fmin += pi->_iFMinDam;
				fmax += pi->_iFMaxDam;
				lmin += pi->_iLMinDam;
				lmax += pi->_iLMaxDam;
				mmin += pi->_iMMinDam;
				mmax += pi->_iMMaxDam;
				amin += pi->_iAMinDam;
				amax += pi->_iAMaxDam;
			}
		}
	}

	p->_pIFlags = iflgs;
	p->_pInfraFlag = (iflgs & ISPL_INFRAVISION) != 0;
	p->_pIFlags2 = iflgs2;
	p->_pIGetHit = ghit;
	p->_pIEnAc = enac;
	p->_pIFMinDam = fmin << 6;
	p->_pIFMaxDam = fmax << 6;
	p->_pILMinDam = lmin << 6;
	p->_pILMaxDam = lmax << 6;
	p->_pIMMinDam = mmin << 6;
	p->_pIMMaxDam = mmax << 6;
	p->_pIAMinDam = amin << 6;
	p->_pIAMaxDam = amax << 6;
	p->_pISplLvlAdd = spllvladd;
	p->_pILifeSteal = lifesteal;
	p->_pIManaSteal = manasteal;
	p->_pISpells = spl;

	// check if the current RSplType is a valid/allowed spell
	if (p->_pRSplType == RSPLTYPE_CHARGES
	 && !(spl & SPELL_MASK(p->_pRSpell))) {
		p->_pRSpell = SPL_INVALID;
		p->_pRSplType = RSPLTYPE_INVALID;
		// unnecessary since MANA_FLASK is always set to redraw, which triggers the redraw of the spell-icon as well
		// gbRedrawFlags |= REDRAW_SPELL_ICON;
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

	lrad = std::max(2, std::min(15, lrad));
	if (p->_pLightRad != lrad && pnum == myplr) {
		p->_pLightRad = lrad;
		ChangeLightRadius(p->_plid, lrad);
		ChangeVisionRadius(p->_pvid, std::max(PLR_MIN_VISRAD, lrad));
	}

	p->_pStrength = sadd + p->_pBaseStr;
	if (p->_pStrength < 0) {
		p->_pStrength = 0;
	}

	p->_pMagic = madd + p->_pBaseMag;
	if (p->_pMagic < 0) {
		p->_pMagic = 0;
	}

	p->_pDexterity = dadd + p->_pBaseDex;
	if (p->_pDexterity < 0) {
		p->_pDexterity = 0;
	}

	p->_pVitality = vadd + p->_pBaseVit;
	if (p->_pVitality < 0) {
		p->_pVitality = 0;
	}

#ifdef HELLFIRE
	if (p->_pClass == PC_BARBARIAN) {
		fr += p->_pLevel;
		lr += p->_pLevel;
		mr += p->_pLevel;
		ar += p->_pLevel;
	}

	if (p->_pSpellFlags & PSE_LETHARGY) {
		fr -= p->_pLevel;
		lr -= p->_pLevel;
		mr -= p->_pLevel;
		ar -= p->_pLevel;
	}
#endif

	if (iflgs & ISPL_ALLRESZERO) {
		// reset resistances to zero if the respective special effect is active
		fr = 0;
		lr = 0;
		mr = 0;
		ar = 0;
	}

	if (fr > MAXRESIST)
		fr = MAXRESIST;
#ifdef HELLFIRE
	else if (fr < 0)
		fr = 0;
#endif
	p->_pFireResist = fr;

	if (lr > MAXRESIST)
		lr = MAXRESIST;
#ifdef HELLFIRE
	else if (lr < 0)
		lr = 0;
#endif
	p->_pLghtResist = lr;

	if (mr > MAXRESIST)
		mr = MAXRESIST;
#ifdef HELLFIRE
	else if (mr < 0)
		mr = 0;
#endif
	p->_pMagResist = mr;

	if (ar > MAXRESIST)
		ar = MAXRESIST;
#ifdef HELLFIRE
	else if (ar < 0)
		ar = 0;
#endif
	p->_pAcidResist = ar;

	switch (p->_pClass) {
	case PC_WARRIOR:   vadd *= 2;         break;
#ifdef HELLFIRE
	case PC_BARBARIAN: vadd *= 3;         break;
	case PC_MONK:
	case PC_BARD:
#endif
	case PC_ROGUE:	   vadd += vadd >> 1; break;
	case PC_SORCERER: break;
	default:
		ASSUME_UNREACHABLE
	}
	ihp += (vadd << 6); // BUGFIX: blood boil can cause negative shifts here (see line 757)

	switch (p->_pClass) {
	case PC_SORCERER: madd *= 2;                     break;
#ifdef HELLFIRE
	case PC_BARBARIAN: break;
	case PC_BARD: madd += (madd >> 2) + (madd >> 1); break;
	case PC_MONK:
#endif
	case PC_ROGUE: madd += madd >> 1;                break;
	case PC_WARRIOR: break;
	default:
		ASSUME_UNREACHABLE
	}
	imana += (madd << 6);

	p->_pHitPoints = ihp + p->_pHPBase;
	p->_pMaxHP = ihp + p->_pMaxHPBase;

	if (pnum == myplr && p->_pHitPoints < (1 << 6)) {
		PlrSetHp(pnum, 0);
	}

	p->_pMana = imana + p->_pManaBase;
	p->_pMaxMana = imana + p->_pMaxManaBase;

	p->_pBlockFlag = FALSE;
	wLeft = &p->InvBody[INVLOC_HAND_LEFT];
	wRight = &p->InvBody[INVLOC_HAND_RIGHT];
#ifdef HELLFIRE
	if (p->_pClass == PC_MONK) {
		if ((wLeft->_itype == ITYPE_STAFF && wLeft->_iStatFlag)
		|| (wRight->_itype == ITYPE_STAFF && wRight->_iStatFlag)) {
			p->_pBlockFlag = TRUE;
			p->_pIFlags |= ISPL_FASTBLOCK;
			p->_pIFlags2 |= ISPH_SWIPE;
		} else if ((wLeft->_itype == ITYPE_NONE && wRight->_itype == ITYPE_NONE)
			|| (wLeft->_iClass == ICLASS_WEAPON && wLeft->_iLoc != ILOC_TWOHAND && wRight->_itype == ITYPE_NONE)
			|| (wRight->_iClass == ICLASS_WEAPON && wRight->_iLoc != ILOC_TWOHAND && wLeft->_itype == ITYPE_NONE))
			p->_pBlockFlag = TRUE;
	} else 
	if ((p->_pClass == PC_BARD
		 && wLeft->_itype == ITYPE_SWORD && wRight->_itype == ITYPE_SWORD)
	 || (p->_pClass == PC_BARBARIAN
		 && (wLeft->_itype == ITYPE_AXE || wRight->_itype == ITYPE_AXE
			|| (((wLeft->_itype == ITYPE_MACE && wLeft->_iLoc == ILOC_TWOHAND)
			  || (wRight->_itype == ITYPE_MACE && wRight->_iLoc == ILOC_TWOHAND)
			  || (wLeft->_itype == ITYPE_SWORD && wLeft->_iLoc == ILOC_TWOHAND)
			  || (wRight->_itype == ITYPE_SWORD && wRight->_iLoc == ILOC_TWOHAND))
			 && !(wLeft->_itype == ITYPE_SHIELD || wRight->_itype == ITYPE_SHIELD))))) {
		p->_pIFlags2 |= ISPH_SWIPE;
	}
#endif
	p->_pwtype = WT_MELEE;

	g = ANIM_ID_UNARMED;

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

	if ((wLeft->_itype == ITYPE_SHIELD && wLeft->_iStatFlag)
	 || (wRight->_itype == ITYPE_SHIELD && wRight->_iStatFlag)) {
		p->_pBlockFlag = TRUE;
		g++;
	}

	if (g == ANIM_ID_UNARMED || g == ANIM_ID_UNARMED_SHIELD) {
		if (g == ANIM_ID_UNARMED_SHIELD) {
			minbl = 3;
			maxbl = 3;
		} else {
			minbl = 1;
			maxbl = 1;
		}
#ifdef HELLFIRE
		if (p->_pClass == PC_MONK) {
			minbl = std::max(minbl, (unsigned)p->_pLevel >> 1);
			maxbl = std::max(maxbl, (unsigned)p->_pLevel);
		}
#endif
	}

	pi = &p->InvBody[INVLOC_CHEST];
#ifdef HELLFIRE
	if (pi->_itype == ITYPE_HARMOR && pi->_iStatFlag) {
		if (p->_pClass == PC_MONK && pi->_iMagical == ITEM_QUALITY_UNIQUE)
			tac += p->_pLevel >> 1;
		g += ANIM_ID_HEAVY_ARMOR;
	} else if (pi->_itype == ITYPE_MARMOR && pi->_iStatFlag) {
		if (p->_pClass == PC_MONK) {
			if (pi->_iMagical == ITEM_QUALITY_UNIQUE)
				tac += p->_pLevel << 1;
			else
				tac += p->_pLevel >> 1;
		}
		g += ANIM_ID_MEDIUM_ARMOR;
	} else if (p->_pClass == PC_MONK) {
		tac += p->_pLevel << 1;
	}
#else
	if (pi->_itype == ITYPE_MARMOR && pi->_iStatFlag) {
		g += ANIM_ID_MEDIUM_ARMOR;
	} else if (pi->_itype == ITYPE_HARMOR && pi->_iStatFlag) {
		g += ANIM_ID_HEAVY_ARMOR;
	}
#endif

	if (p->_pgfxnum != g && Loadgfx) {
		p->_pgfxnum = g;
		p->_pGFXLoad = 0;
		LoadPlrGFX(pnum, PFILE_STAND);
		SetPlrAnims(pnum);

		NewPlrAnim(pnum, p->_pNAnim, p->_pdir, p->_pNFrames, 3, p->_pNWidth);
	} else {
		p->_pgfxnum = g;
	}

	// add class bonuses as item bonus
	if (p->_pClass == PC_ROGUE) {
		pdmod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 200;
#ifdef HELLFIRE
	} else if (p->_pClass == PC_MONK) {
		if (wLeft->_itype != ITYPE_STAFF) {
			if (wRight->_itype != ITYPE_STAFF && (wLeft->_itype != ITYPE_NONE || wRight->_itype != ITYPE_NONE)) {
				pdmod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 300;
			} else {
				pdmod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 150;
			}
		} else {
			pdmod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 150;
		}
	} else if (p->_pClass == PC_BARD) {
		if (wLeft->_itype == ITYPE_SWORD || wRight->_itype == ITYPE_SWORD)
			pdmod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 150;
		else if (wLeft->_itype == ITYPE_BOW || wRight->_itype == ITYPE_BOW) {
			pdmod = p->_pLevel * (p->_pStrength + p->_pDexterity) / 250;
		} else {
			pdmod = p->_pLevel * p->_pStrength / 100;
		}
	} else if (p->_pClass == PC_BARBARIAN) {
		if (wLeft->_itype == ITYPE_AXE || wRight->_itype == ITYPE_AXE) {
			pdmod = p->_pLevel * p->_pStrength / 75;
		} else if (wLeft->_itype == ITYPE_MACE || wRight->_itype == ITYPE_MACE) {
			pdmod = p->_pLevel * p->_pStrength / 75;
		} else if (wLeft->_itype == ITYPE_BOW || wRight->_itype == ITYPE_BOW) {
			pdmod = p->_pLevel * p->_pStrength / 300;
		} else {
			pdmod = p->_pLevel * p->_pStrength / 100;
		}

		if (wLeft->_itype == ITYPE_SHIELD || wRight->_itype == ITYPE_SHIELD) {
			if (wLeft->_itype == ITYPE_SHIELD)
				tac -= wLeft->_iAC / 2;
			else if (wRight->_itype == ITYPE_SHIELD)
				tac -= wRight->_iAC / 2;
		} else if (wLeft->_itype != ITYPE_STAFF && wRight->_itype != ITYPE_STAFF && wLeft->_itype != ITYPE_BOW && wRight->_itype != ITYPE_BOW) {
			pdmod += p->_pLevel * p->_pVitality / 100;
		}
		tac += p->_pLevel / 4;
#endif
	} else {
		pdmod = p->_pLevel * p->_pStrength / 100;
	}
	p->_pIBaseACBonus = bac == 0 ? IBONUS_NONE : (bac >= 0 ? IBONUS_POSITIVE : IBONUS_NEGATIVE);
	p->_pIBaseHitBonus = btohit == 0 ? IBONUS_NONE : (btohit >= 0 ? IBONUS_POSITIVE : IBONUS_NEGATIVE);
	p->_pIBaseDamBonus = bdam == 0 ? IBONUS_NONE : (bdam >= 0 ? IBONUS_POSITIVE : IBONUS_NEGATIVE);
	p->_pIAC = tac + bac + p->_pDexterity / 5;
	p->_pICritChance = 0;
	btohit += 50 + p->_pLevel;
	if (p->_pwtype == WT_MELEE) {
		btohit += p->_pDexterity >> 1;
		if (p->_pClass == PC_WARRIOR)
			btohit += 20;
#ifdef HELLFIRE
		if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARBARIAN)
#else
		if (p->_pClass == PC_WARRIOR)
#endif
			p->_pICritChance = p->_pLevel * 2;
	} else {
		assert(p->_pwtype == WT_RANGED);
		btohit += p->_pDexterity;
		if (p->_pClass == PC_ROGUE)
			btohit += 20;
#ifdef HELLFIRE
		else if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARD)
#else
		else if (p->_pClass == PC_WARRIOR)
#endif
			btohit += 10;
		if (p->_pClass != PC_ROGUE)
			pdmod >>= 1;
	}
	// calculate the damages for each type
	bdam += 100;
	bdam <<= 6;
	pdmod <<= 6 + 1;
	dms = 0;
	if (maxsl != 0)
		dms++;
	if (maxbl != 0)
		dms++;
	if (maxpc != 0)
		dms++;
	if (dms > 1) {
		if (dms == 2)
			pdmod >>= 1;
		else
			// assert(dms == 3);
			pdmod /= 3;
	}
	if (maxsl == 0) {
		p->_pISlMinDam = 0;
		p->_pISlMaxDam = 0;
	} else {
		p->_pISlMinDam = minsl * bdam / 100 + pdmod;
		p->_pISlMaxDam = maxsl * bdam / 100 + pdmod;
	}
	if (maxbl == 0) {
		p->_pIBlMinDam = 0;
		p->_pIBlMaxDam = 0;
	} else {
		p->_pIBlMinDam = minbl * bdam / 100 + pdmod;
		p->_pIBlMaxDam = maxbl * bdam / 100 + pdmod;
	}
	if (maxpc == 0) {
		p->_pIPcMinDam = 0;
		p->_pIPcMaxDam = 0;
	} else {
		p->_pIPcMinDam = minpc * bdam / 100 + pdmod;
		p->_pIPcMaxDam = maxpc * bdam / 100 + pdmod;
	}
	p->_pIHitChance = btohit;
	p->_pIMagToHit = 50 + p->_pMagic;
	if (p->_pClass == PC_SORCERER)
		p->_pIMagToHit += 20;
#ifdef HELLFIRE
	else if (p->_pClass == PC_BARD)
		p->_pIMagToHit += 10;
#endif
	// calculate arrow velocity bonus
	av = ArrowVelBonus(p->_pIFlags);
#ifdef HELLFIRE
	if (p->_pClass == PC_ROGUE)
		av += (p->_pLevel - 1) >> 2;
	else if (p->_pClass == PC_WARRIOR || p->_pClass == PC_BARD)
		av += (p->_pLevel - 1) >> 3;
#else
	if (p->_pClass == PC_ROGUE)
		av += (p->_pLevel - 1) >> 2;
	else if (p->_pClass == PC_WARRIOR)
		av += (p->_pLevel - 1) >> 3;
#endif
	p->_pIArrowVelBonus = av;

	gbRedrawFlags |= REDRAW_HP_FLASK | REDRAW_MANA_FLASK;
}

void CalcPlrAbilities(int pnum)
{
	PlayerStruct *p;

	p = &plr[pnum];
	if (p->_pBlockFlag) {
		p->_pAblSpells |= SPELL_MASK(SPL_BLOCK);
	} else {
		p->_pAblSpells &= ~SPELL_MASK(SPL_BLOCK);
		// check if the current RSplType is a valid/allowed ability
		if (p->_pRSpell == SPL_BLOCK) {
			p->_pRSpell = SPL_INVALID;
			p->_pRSplType = RSPLTYPE_INVALID;
			//gbRedrawFlags |= REDRAW_SPELL_ICON;
		}
	}
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
		if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL && pi->_iStatFlag)
			p->_pScrlSpells |= SPELL_MASK(pi->_iSpell);
	}
	pi = p->SpdList;
	for (i = MAXBELTITEMS; i != 0; i--, pi++) {
		if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL && pi->_iStatFlag)
			p->_pScrlSpells |= SPELL_MASK(pi->_iSpell);
	}
	// check if the current RSplType is a valid/allowed spell
	if (p->_pRSplType == RSPLTYPE_SCROLL && !(p->_pScrlSpells & SPELL_MASK(p->_pRSpell))) {
		p->_pRSpell = SPL_INVALID;
		p->_pRSplType = RSPLTYPE_INVALID;
		//gbRedrawFlags |= REDRAW_SPELL_ICON;
	}
}

void CalcPlrStaff(int pnum)
{
	PlayerStruct *p;
	ItemStruct *pi;

	p = &plr[pnum];
	p->_pISpells = 0;
	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iCharges > 0 && pi->_iStatFlag) {
		p->_pISpells |= SPELL_MASK(pi->_iSpell);
	}
	// check if the current RSplType is a valid/allowed spell
	if (p->_pRSplType == RSPLTYPE_CHARGES && !(p->_pISpells & SPELL_MASK(p->_pRSpell))) {
		p->_pRSpell = SPL_INVALID;
		p->_pRSplType = RSPLTYPE_INVALID;
		//gbRedrawFlags |= REDRAW_SPELL_ICON;
	}
}

static void CalcSelfItems(int pnum)
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

static void CalcPlrItemMin(int pnum)
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
		if (pi->_iMiscId == IMISC_BOOK) {
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
		CalcPlrAbilities(pnum);
		CalcPlrBookVals(pnum);
		CalcPlrScrolls(pnum);
		CalcPlrStaff(pnum);
	}
}

void SetItemData(int ii, int idata)
{
	ItemStruct *is;
	const ItemDataStruct *ids;

	is = &item[ii];
	// zero-initialize struct
	memset(is, 0, sizeof(*is));

	is->_iIdx = idata;
	ids = &AllItemsList[idata];
	is->_itype = ids->itype;
	is->_iCurs = ids->iCurs;
	strcpy(is->_iName, ids->iName);
	strcpy(is->_iIName, ids->iName);
	is->_iLoc = ids->iLoc;
	is->_iClass = ids->iClass;
	is->_iDamType = ids->iDamType;
	is->_iMinDam = ids->iMinDam;
	is->_iMaxDam = ids->iMaxDam;
	is->_iAC = ids->iMinAC == ids->iMaxAC ? ids->iMinAC : RandRange(ids->iMinAC, ids->iMaxAC);
	is->_iMiscId = ids->iMiscId;
	is->_iSpell = ids->iSpell;
	is->_iDurability = ids->iDurability;
	is->_iMaxDur = ids->iDurability;
	is->_iMinStr = ids->iMinStr;
	is->_iMinMag = ids->iMinMag;
	is->_iMinDex = ids->iMinDex;
	is->_ivalue = ids->iValue;
	is->_iIvalue = ids->iValue;

	if (is->_itype == ITYPE_STAFF && is->_iSpell != SPL_NULL) {
		is->_iCharges = BASESTAFFCHARGES;
		is->_iMaxCharges = is->_iCharges;
	}

	is->_iPrePower = IPL_INVALID;
	is->_iSufPower = IPL_INVALID;
	static_assert(ITEM_QUALITY_NORMAL == 0, "Zero-fill expects ITEM_QUALITY_NORMAL == 0.");
	//is->_iMagical = ITEM_QUALITY_NORMAL;
}

void SetItemSData(ItemStruct *is, int idata)
{
	SetItemData(MAXITEMS, idata);
	copy_pod(*is, item[MAXITEMS]);
}

void GetItemSeed(ItemStruct *is)
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

void CreateBaseItem(ItemStruct *is, int idata)
{
	SetItemSData(is, idata);
	GetItemSeed(is);
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
		CreateBaseItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_WARRSWORD);

		CreateBaseItem(&p->InvBody[INVLOC_HAND_RIGHT], IDI_WARRSHLD);

		CreateBaseItem(&p->HoldItem, IDI_WARRCLUB);
		AutoPlace(pnum, 0, 1, 3, &p->HoldItem);

		CreateBaseItem(&p->SpdList[0], IDI_HEAL);
		CreateBaseItem(&p->SpdList[1], IDI_HEAL);
		break;
	case PC_ROGUE:
		CreateBaseItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_ROGUEBOW);

		CreateBaseItem(&p->SpdList[0], IDI_HEAL);
		CreateBaseItem(&p->SpdList[1], IDI_HEAL);
		break;
	case PC_SORCERER:
		CreateBaseItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_SORCSTAFF);

#ifdef HELLFIRE
		CreateBaseItem(&p->SpdList[0], IDI_HEAL);
		CreateBaseItem(&p->SpdList[1], IDI_HEAL);
#else
		CreateBaseItem(&p->SpdList[0], IDI_MANA);
		CreateBaseItem(&p->SpdList[1], IDI_MANA);
#endif
		break;
#ifdef HELLFIRE
	case PC_MONK:
		CreateBaseItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_SHORTSTAFF);

		CreateBaseItem(&p->SpdList[0], IDI_HEAL);
		CreateBaseItem(&p->SpdList[1], IDI_HEAL);
		break;
	case PC_BARD:
		CreateBaseItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_BARDSWORD);
		CreateBaseItem(&p->InvBody[INVLOC_HAND_RIGHT], IDI_BARDDAGGER);

		CreateBaseItem(&p->SpdList[0], IDI_HEAL);
		CreateBaseItem(&p->SpdList[1], IDI_HEAL);
		break;
	case PC_BARBARIAN:
		CreateBaseItem(&p->InvBody[INVLOC_HAND_LEFT], IDI_BARBCLUB);
		CreateBaseItem(&p->InvBody[INVLOC_HAND_RIGHT], IDI_WARRSHLD);

		CreateBaseItem(&p->SpdList[0], IDI_HEAL);
		CreateBaseItem(&p->SpdList[1], IDI_HEAL);
		break;
#endif
	}

	CreateBaseItem(&p->HoldItem, IDI_GOLD);

#ifdef _DEBUG
	if (debug_mode_key_w) {
		SetGoldItemValue(&p->HoldItem, GOLD_MAX_LIMIT);
		for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
			if (p->InvGrid[i] == 0) {
				GetItemSeed(&p->HoldItem);
				copy_pod(p->InvList[p->_pNumInv], p->HoldItem);
				p->InvGrid[i] = ++p->_pNumInv;
				p->_pGold += GOLD_MAX_LIMIT;
			}
		}
	} else
#endif
	{
		SetGoldItemValue(&p->HoldItem, 100);
		p->_pGold = p->HoldItem._ivalue;
		copy_pod(p->InvList[p->_pNumInv++], p->HoldItem);
		p->InvGrid[30] = p->_pNumInv;
	}

	CalcPlrItemVals(pnum, FALSE);
}

BOOL ItemSpaceOk(int x, int y)
{
	int oi, oi2;

	if (x < DBORDERX || x >= DBORDERX + DSIZEX || y < DBORDERY || y >= DBORDERY + DSIZEY)
		return FALSE;

	if ((dItem[x][y] | dMonster[x][y] | dPlayer[x][y] | nSolidTable[dPiece[x][y]]) != 0)
		return FALSE;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return FALSE;
	}

	oi = dObject[x + 1][y + 1];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSelFlag != 0)
			return FALSE;
	}

	oi = dObject[x + 1][y];
	if (oi > 0) {
		oi2 = dObject[x][y + 1];
		if (oi2 > 0 && object[oi - 1]._oSelFlag != 0 && object[oi2 - 1]._oSelFlag != 0)
			return FALSE;
	}

	return TRUE;
}

static BOOL GetItemSpace(int x, int y, int ii)
{
	int i, j, rs;
	BOOL slist[9];
	BOOL savail;

	rs = 0;
	savail = FALSE;
	for (j = -1; j <= 1; j++) {
		for (i = -1; i <= 1; i++) {
			slist[rs] = ItemSpaceOk(x + i, y + j);
			if (slist[rs])
				savail = TRUE;
			rs++;
		}
	}

	rs = random_(13, 15);

	if (!savail)
		return FALSE;

	i = 0;
	while (TRUE) {
		if (slist[i]) {
			if (rs == 0)
				break;
			rs--;
		}
		if (++i == 9) {
			i = 0;
		}
	}

	x--;
	y--;
	x += i % 3;
	y += i / 3;
	SetItemLoc(ii, x, y);
	return TRUE;
}

static void GetSuperItemSpace(int x, int y, int ii)
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
						SetItemLoc(ii, xx, yy);
						return;
					}
				}
			}
		}
	}
}

static void GetSuperItemLoc(int x, int y, int ii)
{
	int xx, yy;
	int i, j, k;

	for (k = 1; k < 50; k++) {
		for (j = -k; j <= k; j++) {
			yy = y + j;
			for (i = -k; i <= k; i++) {
				xx = i + x;
				if (ItemSpaceOk(xx, yy)) {
					SetItemLoc(ii, xx, yy);
					return;
				}
			}
		}
	}
}

static void CalcItemValue(int ii)
{
	ItemStruct *is;
	int v;

	is = &item[ii];
	v = is->_iVMult;
	if (v >= 0) {
		v *= is->_ivalue;
	} else {
		v = is->_ivalue / -v;
	}
	v += is->_iVAdd;
	if (v <= 0) {
		v = 1;
	}
	is->_iIvalue = v;
}

static void GetBookSpell(int ii, int lvl)
{
	SpellData *sd;
	ItemStruct *is;
	int rv, bs;

	rv = random_(14, NUM_SPELLS);

	if (lvl < BOOK_MIN)
		lvl = BOOK_MIN;

	bs = 0;
	while (TRUE) {
		if (spelldata[bs].sBookLvl != SPELL_NA && lvl >= spelldata[bs].sBookLvl
		 && (gbMaxPlayers != 1
			 || (bs != SPL_RESURRECT && bs != SPL_HEALOTHER))) {
			if (rv == 0)
				break;
			rv--;
		}
		if (++bs == NUM_SPELLS)
			bs = 0;
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

static void GetScrollSpell(int ii, int lvl)
{
	SpellData *sd;
	ItemStruct *is;
	int rv, bs;

	rv = random_(14, NUM_SPELLS);

	if (lvl < SCRL_MIN)
		lvl = SCRL_MIN;

	bs = 0;
	while (TRUE) {
		if (spelldata[bs].sScrollLvl != SPELL_NA && lvl >= spelldata[bs].sScrollLvl
		 && (gbMaxPlayers != 1
			 || (bs != SPL_RESURRECT && bs != SPL_HEALOTHER))) {
			if (rv == 0)
				break;
			rv--;
		}
		if (++bs == NUM_SPELLS)
			bs = 0;
	}
	is = &item[ii];
	is->_iSpell = bs;
	sd = &spelldata[bs];
	strcat(is->_iName, sd->sNameText);
	strcat(is->_iIName, sd->sNameText);
	is->_iMinMag = sd->sMinInt > 20 ? sd->sMinInt - 20 : 0;
	is->_ivalue += sd->sStaffCost;
	is->_iIvalue += sd->sStaffCost;
}

static BOOL GetStaffSpell(int ii, int lvl)
{
	SpellData *sd;
	ItemStruct *is;
	int rv, v, bs;
	char istr[64];

	if (random_(17, 4) == 0) {
		return FALSE;
	}
	rv = random_(18, NUM_SPELLS);

	if (lvl < STAFF_MIN)
		lvl = STAFF_MIN;

	bs = 0;
	while (TRUE) {
		if (spelldata[bs].sStaffLvl != SPELL_NA && lvl >= spelldata[bs].sStaffLvl
		 && (gbMaxPlayers != 1
			 || (bs != SPL_RESURRECT && bs != SPL_HEALOTHER))) {
			if (rv == 0)
				break;
			rv--;
		}
		if (++bs == NUM_SPELLS)
			bs = 0;
	}
	is = &item[ii];
	sd = &spelldata[bs];
	snprintf(istr, sizeof(istr), "%s of %s", is->_iName, sd->sNameText);
	if (!control_WriteStringToBuffer((BYTE *)istr))
		snprintf(istr, sizeof(istr), "Staff of %s", sd->sNameText);
	copy_str(is->_iName, istr);
	copy_str(is->_iIName, istr);

	is->_iSpell = bs;
	is->_iCharges = RandRange(sd->sStaffMin, sd->sStaffMax);
	is->_iMaxCharges = is->_iCharges;

	is->_iMinMag = sd->sMinInt;
	v = is->_iCharges * sd->sStaffCost / 5;
	is->_ivalue += v;
	is->_iIvalue += v;
	return TRUE;
}

void GetItemAttrs(int ii, int idata, int lvl)
{
	ItemStruct *is;
	int rndv;

	SetItemData(ii, idata);

	is = &item[ii];
	if (is->_iMiscId == IMISC_BOOK)
		GetBookSpell(ii, lvl);
	else if (is->_iMiscId == IMISC_SCROLL)
		GetScrollSpell(ii, lvl);
	else if (is->_itype == ITYPE_GOLD) {
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

static int PLVal(int pv, int p1, int p2, int minv, int maxv)
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
	r = RandRange(param1, param2);
	is->_iVAdd += PLVal(r, param1, param2, minval, maxval);
	is->_iVMult += multval;
	switch (power) {
	case IPL_TOHIT:
		is->_iPLToHit += r;
		break;
	case IPL_DAMP:
		is->_iPLDam += r;
		break;
	case IPL_TOHIT_DAMP:
		is->_iPLDam += r;
		r = RandRange(param1 >> 2, param2 >> 2);
		is->_iPLToHit += r;
		break;
	case IPL_ACP:
		is->_iPLAC += r;
		break;
	case IPL_SETAC:
		is->_iAC = r;
		break;
	case IPL_AC:
		is->_iAC += r;
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
	case IPL_ACIDRES:
		is->_iPLAR += r;
		break;
	case IPL_ALLRES:
		is->_iPLFR += r;
		if (is->_iPLFR < 0)
			is->_iPLFR = 0;
		is->_iPLLR += r;
		if (is->_iPLLR < 0)
			is->_iPLLR = 0;
		is->_iPLMR += r;
		if (is->_iPLMR < 0)
			is->_iPLMR = 0;
		is->_iPLAR += r;
		if (is->_iPLAR < 0)
			is->_iPLAR = 0;
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
		is->_iCharges = param2;
		is->_iMaxCharges = param2;
		// TODO: is->_iMinMag = spelldata[param1].sMinInt; ?
		break;
	case IPL_FIREDAM:
		is->_iFMinDam = param1;
		is->_iFMaxDam = param2;
		break;
	case IPL_LIGHTDAM:
		is->_iLMinDam = param1;
		is->_iLMaxDam = param2;
		break;
	case IPL_MAGICDAM:
		is->_iMMinDam = param1;
		is->_iMMaxDam = param2;
		break;
	case IPL_ACIDDAM:
		is->_iAMinDam = param1;
		is->_iAMaxDam = param2;
		break;
	case IPL_STR:
		is->_iPLStr += r;
		break;
	case IPL_MAG:
		is->_iPLMag += r;
		break;
	case IPL_DEX:
		is->_iPLDex += r;
		break;
	case IPL_VIT:
		is->_iPLVit += r;
		break;
	case IPL_ATTRIBS:
		is->_iPLStr += r;
		is->_iPLMag += r;
		is->_iPLDex += r;
		is->_iPLVit += r;
		break;
	case IPL_GETHIT:
		is->_iPLGetHit -= r;
		break;
	case IPL_LIFE:
		is->_iPLHP += r << 6;
		break;
	case IPL_MANA:
		is->_iPLMana += r << 6;
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
		is->_iMaxDur = r < 100 ? (is->_iMaxDur - r * is->_iMaxDur / 100) : 1;
		is->_iDurability = is->_iMaxDur;
		break;
	case IPL_INDESTRUCTIBLE:
		is->_iDurability = DUR_INDESTRUCTIBLE;
		is->_iMaxDur = DUR_INDESTRUCTIBLE;
		break;
	case IPL_LIGHT:
		is->_iPLLight += param1;
		break;
#ifdef HELLFIRE
	case IPL_MULT_ARROWS:
		is->_iFlags |= ISPL_MULT_ARROWS;
		break;
#endif
	case IPL_INVCURS:
		is->_iCurs = param1;
		break;
	case IPL_THORNS:
		is->_iFlags |= ISPL_THORNS;
		break;
	case IPL_NOMANA:
		is->_iFlags |= ISPL_NOMANA;
		break;
	case IPL_KNOCKBACK:
		is->_iFlags |= ISPL_KNOCKBACK;
		break;
	case IPL_ALLRESZERO:
		is->_iFlags |= ISPL_ALLRESZERO;
		break;
	case IPL_NOHEALMON:
		is->_iFlags |= ISPL_NOHEALMON;
		break;
	case IPL_STEALMANA:
		is->_iManaSteal = r;
		break;
	case IPL_STEALLIFE:
		is->_iLifeSteal = r;
		break;
	case IPL_TARGAC:
		is->_iPLEnAc += r;
		break;
	case IPL_FASTATTACK:
		static_assert((ISPL_QUICKATTACK & (ISPL_QUICKATTACK - 1)) == 0, "Optimized SaveItemPower depends simple flag-like attack-speed modifiers.");
		static_assert(ISPL_QUICKATTACK == ISPL_FASTATTACK / 2, "SaveItemPower depends on ordered attack-speed modifiers I.");
		static_assert(ISPL_FASTATTACK == ISPL_FASTERATTACK / 2, "SaveItemPower depends on ordered attack-speed modifiers II.");
		static_assert(ISPL_FASTERATTACK == ISPL_FASTESTATTACK / 2, "SaveItemPower depends on ordered attack-speed modifiers III.");
		if ((unsigned)(param1 - 1) < 4)
			is->_iFlags |= ISPL_QUICKATTACK << (param1 - 1);
		break;
	case IPL_FASTRECOVER:
		static_assert((ISPL_FASTRECOVER & (ISPL_FASTRECOVER - 1)) == 0, "Optimized SaveItemPower depends simple flag-like hit-recovery modifiers.");
		static_assert(ISPL_FASTRECOVER == ISPL_FASTERRECOVER / 2, "SaveItemPower depends on ordered hit-recovery modifiers I.");
		static_assert(ISPL_FASTERRECOVER == ISPL_FASTESTRECOVER / 2, "SaveItemPower depends on ordered hit-recovery modifiers II.");
		if ((unsigned)(param1 - 1) < 3)
			is->_iFlags |= ISPL_FASTRECOVER << (param1 - 1);
		break;
	case IPL_FASTBLOCK:
		is->_iFlags |= ISPL_FASTBLOCK;
		break;
	case IPL_DAMMOD:
		is->_iPLDamMod += r;
		break;
	case IPL_SETDAM:
		is->_iMinDam = param1;
		is->_iMaxDam = param2;
		break;
	case IPL_SETDUR:
		is->_iDurability = param1;
		is->_iMaxDur = param1;
		break;
	case IPL_ONEHAND:
		is->_iLoc = ILOC_ONEHAND;
		break;
	case IPL_DRAINLIFE:
		is->_iFlags |= ISPL_DRAINLIFE;
		break;
	case IPL_INFRAVISION:
		is->_iFlags |= ISPL_INFRAVISION;
		break;
	case IPL_NOMINSTR:
		is->_iMinStr = 0;
		break;
#ifdef HELLFIRE
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
	case IPL_FASTCAST:
		static_assert((ISPH_FASTCAST & (ISPH_FASTCAST - 1)) == 0, "Optimized SaveItemPower depends simple flag-like cast-speed modifiers.");
		static_assert(ISPH_FASTCAST == ISPH_FASTERCAST / 2, "SaveItemPower depends on ordered cast-speed modifiers I.");
		static_assert(ISPH_FASTERCAST == ISPH_FASTESTCAST / 2, "SaveItemPower depends on ordered cast-speed modifiers II.");
		if ((unsigned)(param1 - 1) < 3)
			is->_iFlags2 |= ISPH_FASTCAST << (param1 - 1);
		break;
	case IPL_FASTWALK:
		static_assert((ISPH_FASTWALK & (ISPH_FASTWALK - 1)) == 0, "Optimized SaveItemPower depends simple flag-like walk-speed modifiers.");
		static_assert(ISPH_FASTWALK == ISPH_FASTERWALK / 2, "SaveItemPower depends on ordered walk-speed modifiers I.");
		static_assert(ISPH_FASTERWALK == ISPH_FASTESTWALK / 2, "SaveItemPower depends on ordered walk-speed modifiers II.");
		if ((unsigned)(param1 - 1) < 3)
			is->_iFlags2 |= ISPH_FASTWALK << (param1 - 1);
		break;
	default:
		ASSUME_UNREACHABLE
	}
}

void GetItemPower(int ii, int minlvl, int maxlvl, int flgs, BOOL onlygood)
{
	int pre, post, nl;
	const PLStruct *pres, *sufs;
	const PLStruct *l[256];
	BYTE goe;

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
	goe = GOE_ANY;
	if (!onlygood && random_(0, 3) != 0)
		onlygood = TRUE;
	if (pre == 0) {
		nl = 0;
		for (pres = PL_Prefix; pres->PLPower != IPL_INVALID; pres++) {
			if ((flgs & pres->PLIType)
			 && pres->PLMinLvl >= minlvl && pres->PLMinLvl <= maxlvl
			 && (!onlygood || pres->PLOk)) {
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
			if ((sufs->PLIType & flgs)
			    && sufs->PLMinLvl >= minlvl && sufs->PLMinLvl <= maxlvl
			    && (goe | sufs->PLGOE) != (GOE_GOOD | GOE_EVIL)
			    && (!onlygood || sufs->PLOk)) {
				l[nl] = sufs;
				nl++;
			}
		}
		sufs = NULL;
		if (nl != 0) {
			sufs = l[random_(23, nl)];
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
	if (pres != NULL || sufs != NULL)
		CalcItemValue(ii);
}

static void GetItemBonus(int ii, int minlvl, int maxlvl, BOOL onlygood, BOOLEAN allowspells)
{
	int flgs;

	switch (item[ii]._itype) {
	case ITYPE_MISC:
		return;
	case ITYPE_SWORD:
	case ITYPE_AXE:
	case ITYPE_MACE:
		flgs = PLT_MELEE;
		break;
	case ITYPE_BOW:
		flgs = PLT_BOW;
		break;
	case ITYPE_SHIELD:
		flgs = PLT_SHLD;
		break;
	case ITYPE_LARMOR:
		flgs = PLT_ARMO | PLT_LARMOR;
		break;
	case ITYPE_HELM:
		flgs = PLT_ARMO;
		break;
	case ITYPE_MARMOR:
		flgs = PLT_ARMO | PLT_MARMOR;
		break;
	case ITYPE_HARMOR:
		flgs = PLT_ARMO | PLT_HARMOR;
		break;
	case ITYPE_STAFF:
		flgs = PLT_STAFF;
		if (allowspells && GetStaffSpell(ii, maxlvl)) {
			if (random_(51, 2) != 0)
				return;
			flgs |= PLT_CHRG;
		}
		break;
	case ITYPE_GOLD:
		return;
	case ITYPE_RING:
	case ITYPE_AMULET:
		flgs = PLT_MISC;
		break;
	default:
		ASSUME_UNREACHABLE
		return;
	}

	if (minlvl > 25)
		minlvl = 25;
	GetItemPower(ii, minlvl, maxlvl, flgs, onlygood);
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

static int RndItem(int lvl)
{
	int i, ri;
	int ril[NUM_IDI * 2];

	if (random_(24, 100) > 40)
		return -1;

	if (random_(24, 100) > 25)
		return IDI_GOLD;

	ri = 0;
	for (i = 0; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER || lvl < AllItemsList[i].iMinMLvl)
			continue;
		ril[ri] = i;
		ri++;
		if (AllItemsList[i].iRnd == IDROP_DOUBLE) {
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(24, ri)];
}

static int RndUItem(int lvl)
{
	int i, ri;
	int ril[NUM_IDI];

	ri = 0;
	for (i = 0; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER || lvl < AllItemsList[i].iMinMLvl
		 || AllItemsList[i].itype == ITYPE_GOLD
		 || (AllItemsList[i].itype == ITYPE_MISC && AllItemsList[i].iMiscId != IMISC_BOOK))
			continue;
		ril[ri] = i;
		ri++;
	}

	return ril[random_(25, ri)];
}

static int RndAllItems(int lvl)
{
	int i, ri;
	int ril[NUM_IDI];

	if (random_(26, 100) > 25)
		return IDI_GOLD;

	ri = 0;
	for (i = 0; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER || lvl < AllItemsList[i].iMinMLvl)
			continue;
		ril[ri] = i;
		ri++;
	}

	return ril[random_(26, ri)];
}

static int RndTypeItems(int itype, int imid, int lvl)
{
	int i, ri;
	int ril[NUM_IDI];

	ri = 0;
	for (i = 0; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd == IDROP_NEVER || lvl < AllItemsList[i].iMinMLvl
		 || AllItemsList[i].itype != itype
		 || (imid != -1 && AllItemsList[i].iMiscId != imid))
			continue;
		ril[ri] = i;
		ri++;
	}

	return ril[random_(27, ri)];
}

static int CheckUnique(int ii, int lvl, int uper, BOOL recreate)
{
	int i, ui;
	BYTE uok[NUM_UITEM];
	BOOL uniq;
	char uid;

	if (random_(28, 100) > uper)
		return -1;

	static_assert(NUM_UITEM <= UCHAR_MAX, "Unique index must fit to a BYTE in CheckUnique.");

	uid = AllItemsList[item[ii]._iIdx].iItemId;
	uniq = !recreate && gbMaxPlayers == 1;
	ui = 0;
	for (i = 0; i < NUM_UITEM; i++) {
		if (UniqueItemList[i].UIItemId == uid
		 && lvl >= UniqueItemList[i].UIMinLvl
		 && (!uniq || !UniqueItemFlag[i])) {
			uok[ui] = i;
			ui++;
		}
	}

	if (ui == 0)
		return -1;

	return uok[random_(29, ui)];
}

static void GetUniqueItem(int ii, int uid)
{
	const UItemStruct *ui;

	UniqueItemFlag[uid] = TRUE;
	ui = &UniqueItemList[uid];
	SaveItemPower(ii, ui->UIPower1, ui->UIParam1a, ui->UIParam1b, 0, 0, 1);

	if (ui->UIPower2 != IPL_INVALID) {
		SaveItemPower(ii, ui->UIPower2, ui->UIParam2a, ui->UIParam2b, 0, 0, 1);
	if (ui->UIPower3 != IPL_INVALID) {
		SaveItemPower(ii, ui->UIPower3, ui->UIParam3a, ui->UIParam3b, 0, 0, 1);
	if (ui->UIPower4 != IPL_INVALID) {
		SaveItemPower(ii, ui->UIPower4, ui->UIParam4a, ui->UIParam4b, 0, 0, 1);
	if (ui->UIPower5 != IPL_INVALID) {
		SaveItemPower(ii, ui->UIPower5, ui->UIParam5a, ui->UIParam5b, 0, 0, 1);
	if (ui->UIPower6 != IPL_INVALID) {
		SaveItemPower(ii, ui->UIPower6, ui->UIParam6a, ui->UIParam6b, 0, 0, 1);
	}}}}}

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
	int ii, idx;

	if (numitems >= MAXITEMS)
		return;

	idx = 0;
	while (AllItemsList[idx].iItemId != UniqueItemList[uid].UIItemId) {
		idx++;
	}
	assert(AllItemsList[idx].iMiscId == IMISC_UNIQUE);

	ii = itemavail[0];
	GetItemAttrs(ii, idx, items_get_currlevel());
	GetUniqueItem(ii, uid);
	SetupItem(ii);
	GetSuperItemSpace(x, y, ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

static void ItemRndDur(int ii)
{
	if (item[ii]._iDurability != 0 && item[ii]._iDurability != DUR_INDESTRUCTIBLE)
		item[ii]._iDurability = random_(0, item[ii]._iMaxDur >> 1) + (item[ii]._iMaxDur >> 2) + 1;
}

static void SetupAllItems(int ii, int idx, int iseed, int lvl, int uper, BOOL onlygood, BOOL recreate, BOOL pregen)
{
	int iblvl, uid;

	SetRndSeed(iseed);
	GetItemAttrs(ii, idx, lvl >> 1);
	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl;

	if (pregen)
		item[ii]._iCreateInfo |= CF_PREGEN;
	if (onlygood)
		item[ii]._iCreateInfo |= CF_ONLYGOOD;

	if (uper == 15)
		item[ii]._iCreateInfo |= CF_UPER15;
	else if (uper == 1)
		item[ii]._iCreateInfo |= CF_UPER1;

	if (item[ii]._iMiscId != IMISC_UNIQUE) {
		iblvl = -1;
		if (uper == 15)
			iblvl = lvl + 4;
		else if (onlygood
		 || item[ii]._itype == ITYPE_STAFF
		 || item[ii]._itype == ITYPE_RING
		 || item[ii]._itype == ITYPE_AMULET
		 || random_(32, 100) <= 10 || random_(33, 100) <= lvl)
			iblvl = lvl;
		if (iblvl != -1) {
			uid = CheckUnique(ii, iblvl, uper, recreate);
			if (uid < 0) {
				GetItemBonus(ii, iblvl >> 1, iblvl, onlygood, TRUE);
			} else {
				GetUniqueItem(ii, uid);
			}
		}
		if (item[ii]._iMagical != ITEM_QUALITY_UNIQUE)
			ItemRndDur(ii);
	} else {
		assert(item[ii]._iLoc != ILOC_UNEQUIPABLE);
			//uid = CheckUnique(ii, iblvl, uper, recreate);
			//if (uid >= 0) {
			//	GetUniqueItem(ii, uid);
			//}
			GetUniqueItem(ii, iseed);
	}
	SetupItem(ii);
}

void SpawnItem(int mnum, int x, int y, BOOL sendmsg)
{
	MonsterStruct* mon;
	int ii, idx;
	BOOL onlygood = FALSE;

	if (numitems >= MAXITEMS)
		return;

	mon = &monster[mnum];
	if ((mon->MData->mTreasure & 0x8000) != 0 && gbMaxPlayers == 1) {
		// fix drop in single player
		idx = mon->MData->mTreasure & 0xFFF;
		SpawnUnique(idx, x, y);
		return;
	}
	if (mon->MData->mTreasure & 0x4000)
		// no drop
		return;

	if (mon->_uniqtype != 0) {
		idx = RndUItem(mon->mLevel);
		onlygood = TRUE;
	} else if (quests[Q_MUSHROOM]._qactive != QUEST_ACTIVE || quests[Q_MUSHROOM]._qvar1 != QS_MUSHGIVEN) {
		idx = RndItem(mon->mLevel);
		if (idx < 0)
			return;
	} else {
		idx = IDI_BRAIN;
		quests[Q_MUSHROOM]._qvar1 = QS_BRAINSPAWNED;
	}

	ii = itemavail[0];
	SetupAllItems(ii, idx, GetRndSeed(), mon->MData->mLevel,
		onlygood ? 15 : 1, onlygood, FALSE, FALSE);
	GetSuperItemSpace(x, y, ii);
	if (sendmsg)
		NetSendCmdDItem(FALSE, ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void CreateRndItem(int x, int y, BOOL onlygood, BOOL sendmsg, BOOL delta)
{
	int idx, ii, lvl;

	if (numitems >= MAXITEMS)
		return;

	lvl = items_get_currlevel();
	lvl <<= 1;

	if (onlygood)
		idx = RndUItem(lvl);
	else
		idx = RndAllItems(lvl);

	ii = itemavail[0];
	SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, onlygood, FALSE, delta);
	GetSuperItemSpace(x, y, ii);
	if (sendmsg)
		NetSendCmdDItem(FALSE, ii);
	if (delta)
		DeltaAddItem(ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

static void SetupAllUseful(int ii, int iseed, int lvl)
{
	int idx;

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
	case 6:
		idx = IDI_OIL;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
#else
	if (random_(34, 2) != 0)
		idx = IDI_HEAL;
	else
		idx = IDI_MANA;

	if (lvl > 1 && random_(34, 3) == 0)
		idx = IDI_PORTAL;
#endif

	SetItemData(ii, idx);
	SetupItem(ii);
	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_USEFUL;
}

void CreateRndUseful(int x, int y, BOOL sendmsg)
{
	int ii, lvl;

	if (numitems >= MAXITEMS)
		return;

	lvl = items_get_currlevel();

	ii = itemavail[0];
	SetupAllUseful(ii, GetRndSeed(), lvl);
	GetSuperItemSpace(x, y, ii);
	if (sendmsg) {
		NetSendCmdDItem(FALSE, ii);
	}
	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void CreateTypeItem(int x, int y, BOOL onlygood, int itype, int imisc, BOOL sendmsg, BOOL delta)
{
	int idx, ii, lvl;

	if (numitems >= MAXITEMS)
		return;

	lvl = items_get_currlevel();
	lvl <<= 1;

	if (itype != ITYPE_GOLD)
		idx = RndTypeItems(itype, imisc, lvl);
	else
		idx = IDI_GOLD;

	ii = itemavail[0];
	SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, onlygood, FALSE, delta);
	GetSuperItemSpace(x, y, ii);

	if (sendmsg)
		NetSendCmdDItem(FALSE, ii);
	if (delta)
		DeltaAddItem(ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void RecreateItem(int idx, WORD icreateinfo, int iseed, int ivalue)
{
	int uper;
	BOOL onlygood, recreate, pregen;

	if (idx == IDI_GOLD) {
		SetItemData(MAXITEMS, IDI_GOLD);
		item[MAXITEMS]._iSeed = iseed;
		item[MAXITEMS]._iCreateInfo = icreateinfo;
		SetGoldItemValue(&item[MAXITEMS], ivalue);
	} else {
		if (icreateinfo == 0) {
			SetItemData(MAXITEMS, idx);
			item[MAXITEMS]._iSeed = iseed;
		} else {
			if (icreateinfo & CF_TOWN) {
				RecreateTownItem(MAXITEMS, idx, icreateinfo, iseed);
			} else if ((icreateinfo & CF_USEFUL) == CF_USEFUL) {
				SetupAllUseful(MAXITEMS, iseed, icreateinfo & CF_LEVEL);
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
				SetupAllItems(MAXITEMS, idx, iseed, icreateinfo & CF_LEVEL, uper, onlygood, recreate, pregen);
			}
		}
	}
}

void RecreateEar(WORD ic, int iseed, int Id, int dur, int mdur, int ch, int mch, int ivalue, int ibuff)
{
	SetItemData(MAXITEMS, IDI_EAR);
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
	snprintf(item[MAXITEMS]._iName, sizeof(item[MAXITEMS]._iName), "Ear of %s", tempstr);
	item[MAXITEMS]._iCurs = ((ivalue >> 6) & 3) + ICURS_EAR_SORCEROR;
	item[MAXITEMS]._ivalue = ivalue & 0x3F;
	item[MAXITEMS]._iCreateInfo = ic;
	item[MAXITEMS]._iSeed = iseed;
}

static void GetRandomItemSpace(int randarea, int ii)
{
	BOOL failed;
	int x, y, i, j, tries;

	assert(randarea > 0);

	tries = 0;
	while (TRUE) {
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
		tries++;
		if (tries > 1000 && randarea > 1)
			randarea--;
	}

	SetItemLoc(ii, x, y);
}

void SpawnQuestItemAt(int idx, int x, int y)
{
	int ii;

	if (numitems >= MAXITEMS)
		return;

	ii = itemavail[0];
	GetItemAttrs(ii, idx, items_get_currlevel());
	SetupItem(ii);
	item[ii]._iPostDraw = TRUE;
	item[ii]._iSelFlag = 1;
	item[ii]._iAnimFrame = item[ii]._iAnimLen;
	item[ii]._iAnimFlag = FALSE;
	SetItemLoc(ii, x, y);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void SpawnQuestItemAround(int idx, int x, int y)
{
	int ii;

	if (numitems >= MAXITEMS)
		return;

	ii = itemavail[0];
	GetItemAttrs(ii, idx, items_get_currlevel());
	SetupItem(ii);
	item[ii]._iPostDraw = TRUE;
	GetSuperItemLoc(x, y, ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void SpawnQuestItemInArea(int idx, int areasize)
{
	int ii;

	if (numitems >= MAXITEMS)
		return;

	ii = itemavail[0];
	GetItemAttrs(ii, idx, items_get_currlevel());
	SetupItem(ii);
	item[ii]._iPostDraw = TRUE;
	item[ii]._iSelFlag = 1;
	item[ii]._iAnimFrame = item[ii]._iAnimLen;
	item[ii]._iAnimFlag = FALSE;
	GetRandomItemSpace(areasize, ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void SpawnRock()
{
	int i, oi;

	for (i = 0; i < nobjects; i++) {
		oi = objectactive[i];
		if (object[oi]._otype == OBJ_STAND)
			break;
	}
	if (i != nobjects) {
		i = itemavail[0];
		SetItemData(i, IDI_ROCK);
		SetupItem(i);
		item[i]._iSelFlag = 2;
		item[i]._iPostDraw = TRUE;
		item[i]._iAnimFrame = 11;
		SetItemLoc(i, object[oi]._ox, object[oi]._oy);

		itemactive[numitems] = i;
		itemavail[0] = itemavail[MAXITEMS - numitems - 1];
		numitems++;
	}
}

#ifdef HELLFIRE
void SpawnRewardItem(int idx, int xx, int yy)
{
	int ii;

	ii = itemavail[0];
	GetItemAttrs(ii, idx, items_get_currlevel());
	SetupItem(ii);
	item[ii]._iSelFlag = 2;
	item[ii]._iPostDraw = TRUE;
	item[ii]._iAnimFrame = 1;
	item[ii]._iAnimFlag = TRUE;
	item[ii]._iIdentified = TRUE;
	GetSuperItemSpace(xx, yy, ii);

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
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
		PlaySfxLoc(ItemDropSnds[ItemCAnimTbl[ICURS_MAGIC_ROCK]], is->_ix, is->_iy);
	} else if (is->_iCurs == ICURS_TAVERN_SIGN || is->_iCurs == ICURS_ANVIL_OF_FURY)
		is->_iSelFlag = 1;
}

void DeleteItem(int ii, int i)
{
	itemavail[MAXITEMS - numitems] = ii;
	numitems--;
	if (numitems > 0 && i != numitems)
		itemactive[i] = itemactive[numitems];
}

static void ItemDoppel()
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
	rep = is->_iDurability;

	while (rep < md) {
		rep += lvl + random_(37, lvl);
		d = md / (lvl + 9);
		if (d < 1)
			d = 1;
		md -= d;
	}

	is->_iMaxDur = md;
	if (rep > md)
		rep = md;
	is->_iDurability = rep;
}

void DoRepair(int pnum, int cii)
{
	PlayerStruct *p;
	ItemStruct *pi;

	p = &plr[pnum];

	if (cii >= NUM_INVLOC) {
		pi = &p->InvList[cii - NUM_INVLOC];
	} else {
		pi = &p->InvBody[cii];
	}

	RepairItem(pi, p->_pLevel);
	if (pi->_iMaxDur == 0) {
		// NetSendCmdDelItem(TRUE, cii);
		pi->_itype = ITYPE_NONE;
	}
	CalcPlrInv(pnum, TRUE);
}

static void RechargeItem(ItemStruct *is, int r)
{
	int cc, mc;

	mc = is->_iMaxCharges;
	cc = is->_iCharges;

	while (cc < mc) {
		mc--;
		cc += r;
	}

	is->_iMaxCharges = mc;
	if (cc > mc)
		cc = mc;
	is->_iCharges = cc;
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
	if (pi->_itype == ITYPE_STAFF && pi->_iSpell != SPL_NULL) {
		r = spelldata[pi->_iSpell].sBookLvl;
		r = random_(38, p->_pLevel / r) + 1;
		RechargeItem(pi, r);
		CalcPlrInv(pnum, TRUE);
	}
}

void DoClean(ItemStruct *pi, BOOL whittle)
{
	int seed, spell;
	WORD ci, idx;

	seed = pi->_iSeed;
	spell = pi->_iSpell;
	idx = pi->_iIdx;

	ci = (pi->_iCreateInfo & CF_LEVEL) | CF_CRAFTED;
	if (whittle) {
		if (idx == IDI_SORCSTAFF)
			idx = IDI_DROPSHSTAFF;
		spell = SPL_NULL;
	}

	while (TRUE) {
		RecreateItem(idx, ci, seed, 0);
		assert(item[MAXITEMS]._iIdx == idx);
		if (item[MAXITEMS]._iPrePower == IPL_INVALID
		 && item[MAXITEMS]._iSufPower == IPL_INVALID
		 && item[MAXITEMS]._iSpell == spell)
			break;
		seed = GetRndSeed();
	}
	item[MAXITEMS]._iDurability = std::min(pi->_iDurability, item[MAXITEMS]._iDurability);
	item[MAXITEMS]._iCharges = std::min(pi->_iCharges, item[MAXITEMS]._iCharges);
	copy_pod(*pi, item[MAXITEMS]);
}

#ifdef HELLFIRE
void DoWhittle(int pnum, int cii)
{
	PlayerStruct *p;
	ItemStruct *pi;

	p = &plr[pnum];
	if (cii >= NUM_INVLOC) {
		pi = &p->InvList[cii - NUM_INVLOC];
	} else {
		pi = &p->InvBody[cii];
	}

	if (pi->_itype == ITYPE_STAFF
	 && (pi->_iSpell != SPL_NULL || pi->_iMagical != ITEM_QUALITY_NORMAL)) {
		DoClean(pi, TRUE);
		CalcPlrInv(pnum, TRUE);
	}
}
#endif

static ItemStruct* PlrItem(int pnum, int cii)
{
	if (cii <= INVITEM_INV_LAST) {
		if (cii < INVITEM_INV_FIRST) {
			return &plr[pnum].InvBody[cii];
		} else
			return &plr[pnum].InvList[cii - INVITEM_INV_FIRST];
	} else {
		return &plr[pnum].SpdList[cii - INVITEM_BELT_FIRST];
	}
}

static void RemovePlrItem(int pnum, int cii)
{
	if (cii <= INVITEM_INV_LAST) {
		if (cii < INVITEM_INV_FIRST) {
			plr[pnum].InvBody[cii]._itype = ITYPE_NONE;
		} else
			RemoveInvItem(pnum, cii - INVITEM_INV_FIRST);
	} else {
		RemoveSpdBarItem(pnum, cii - INVITEM_BELT_FIRST);
	}
}

void DoOil(int pnum, int from, int cii)
{
	ItemStruct *pi, *is;
	int oilType, seed, spell;
	WORD idx, ci;
	BYTE targetPowerFrom, targetPowerTo;

	is = PlrItem(pnum, from);
	if (is->_itype == ITYPE_NONE)
		return;
	oilType = is->_iMiscId;
	if (oilType < IMISC_OILFIRST || oilType > IMISC_OILLAST)
		return;

	pi = PlrItem(pnum, cii);
	assert(pi->_itype != ITYPE_NONE);
	if (pi->_itype == ITYPE_MISC || pi->_itype == ITYPE_GOLD)
		return;

	if (oilType == IMISC_OILCLEAN) {
		if (pi->_iMagical != ITEM_QUALITY_MAGIC)
			return;

		DoClean(pi, FALSE);
		RemovePlrItem(pnum, from);
		CalcPlrInv(pnum, TRUE);
		return;
	}
	if (pi->_iMagical != ITEM_QUALITY_NORMAL)
		return;

	switch (oilType) {
	case IMISC_OILQLTY:
		static_assert(IPL_TOHIT < IPL_DAMP, "DoOil requires a given order of TOHIT and DAMP affixes.");
		static_assert(IPL_TOHIT < IPL_TOHIT_DAMP, "DoOil requires a given order of TOHIT and TOHIT_DAMP affixes.");
		static_assert(IPL_DAMP < IPL_ACP, "DoOil requires a given order of DAMP and ACP affixes.");
		static_assert(IPL_TOHIT_DAMP < IPL_ACP, "DoOil requires a given order of TOHIT_DAMP and ACP affixes.");
		targetPowerFrom = IPL_TOHIT;
		targetPowerTo = IPL_ACP;
		break;
	case IMISC_OILZEN:
		static_assert(IPL_STR < IPL_VIT, "DoOil requires a given order of STR and VIT, affixes.");
		static_assert(IPL_STR < IPL_DEX, "DoOil requires a given order of STR and DEX affixes.");
		static_assert(IPL_STR < IPL_MAG, "DoOil requires a given order of STR and MAG affixes.");
		static_assert(IPL_VIT < IPL_ATTRIBS, "DoOil requires a given order of VIT, and ATTRIBS affixes.");
		static_assert(IPL_DEX < IPL_ATTRIBS, "DoOil requires a given order of DEX and ATTRIBS affixes.");
		static_assert(IPL_MAG < IPL_ATTRIBS, "DoOil requires a given order of MAG and ATTRIBS affixes.");
		targetPowerFrom = IPL_STR;
		targetPowerTo = IPL_ATTRIBS;
		break;
	case IMISC_OILSTR:
		targetPowerFrom = targetPowerTo = IPL_STR;
		break;
	case IMISC_OILDEX:
		targetPowerFrom = targetPowerTo = IPL_DEX;
		break;
	case IMISC_OILVIT:
		targetPowerFrom = targetPowerTo = IPL_VIT;
		break;
	case IMISC_OILMAG:
		targetPowerFrom = targetPowerTo = IPL_MAG;
		break;
	case IMISC_OILRESIST:
		static_assert(IPL_FIRERES < IPL_LIGHTRES, "DoOil requires a given order of FIRERES and LIGHTRES, affixes.");
		static_assert(IPL_FIRERES < IPL_MAGICRES, "DoOil requires a given order of FIRERES and MAGICRES affixes.");
		static_assert(IPL_FIRERES < IPL_ACIDRES, "DoOil requires a given order of FIRERES and ACIDRES affixes.");
		static_assert(IPL_LIGHTRES < IPL_ALLRES, "DoOil requires a given order of LIGHTRES, and ALLRES affixes.");
		static_assert(IPL_MAGICRES < IPL_ALLRES, "DoOil requires a given order of MAGICRES and ALLRES affixes.");
		static_assert(IPL_ACIDRES < IPL_ALLRES, "DoOil requires a given order of ACIDRES and ALLRES affixes.");
		targetPowerFrom = IPL_FIRERES;
		targetPowerTo = IPL_ALLRES;
		break;
	case IMISC_OILCHANCE:
		targetPowerFrom = 0;
		static_assert(IPL_INVALID >= UCHAR_MAX, "DoOil requires the invalid to be the last.");
		targetPowerTo = UCHAR_MAX - 1;
		break;
	default:
		ASSUME_UNREACHABLE
		targetPowerFrom = 0;
		targetPowerTo = UCHAR_MAX;
		break;
	}

	idx = pi->_iIdx;
	ci = (pi->_iCreateInfo & CF_LEVEL) | CF_CRAFTED | CF_ONLYGOOD;
	spell = pi->_iSpell;
	seed = pi->_iSeed;

	while (TRUE) {
		RecreateItem(idx, ci, seed, 0);
		assert(item[MAXITEMS]._iIdx == idx);
		if (item[MAXITEMS]._iSpell == spell
		 && ((item[MAXITEMS]._iPrePower >= targetPowerFrom && item[MAXITEMS]._iPrePower <= targetPowerTo)
		  || (item[MAXITEMS]._iSufPower >= targetPowerFrom && item[MAXITEMS]._iSufPower <= targetPowerTo)))
			break;
		seed = GetRndSeed();
	}

	item[MAXITEMS]._iDurability = std::min(pi->_iDurability, item[MAXITEMS]._iDurability);
	item[MAXITEMS]._iCharges = std::min(pi->_iCharges, item[MAXITEMS]._iCharges);
	copy_pod(*pi, item[MAXITEMS]);

	pi->_iIdentified = TRUE;
	RemovePlrItem(pnum, from);
	CalcPlrInv(pnum, TRUE);
}

void PrintItemPower(BYTE plidx, const ItemStruct *is)
{
	switch (plidx) {
	case IPL_TOHIT:
		snprintf(tempstr, sizeof(tempstr), "chance to hit: %+i%%", is->_iPLToHit);
		break;
	case IPL_DAMP:
		snprintf(tempstr, sizeof(tempstr), "%+i%% damage", is->_iPLDam);
		break;
	case IPL_TOHIT_DAMP:
		snprintf(tempstr, sizeof(tempstr), "to hit: %+i%%, %+i%% damage", is->_iPLToHit, is->_iPLDam);
		break;
	case IPL_ACP:
		snprintf(tempstr, sizeof(tempstr), "%+i%% armor", is->_iPLAC);
		break;
	case IPL_SETAC:
	case IPL_AC:
		snprintf(tempstr, sizeof(tempstr), "armor class: %i", is->_iAC);
		break;
	case IPL_FIRERES:
		if (is->_iPLFR < 75)
			snprintf(tempstr, sizeof(tempstr), "Resist Fire: %+i%%", is->_iPLFR);
		else
			copy_cstr(tempstr, "Resist Fire: 75% MAX");
		break;
	case IPL_LIGHTRES:
		if (is->_iPLLR < 75)
			snprintf(tempstr, sizeof(tempstr), "Resist Lightning: %+i%%", is->_iPLLR);
		else
			copy_cstr(tempstr, "Resist Lightning: 75% MAX");
		break;
	case IPL_MAGICRES:
		if (is->_iPLMR < 75)
			snprintf(tempstr, sizeof(tempstr), "Resist Magic: %+i%%", is->_iPLMR);
		else
			copy_cstr(tempstr, "Resist Magic: 75% MAX");
		break;
	case IPL_ACIDRES:
		if (is->_iPLAR < 75)
			snprintf(tempstr, sizeof(tempstr), "Resist Acid: %+i%%", is->_iPLAR);
		else
			copy_cstr(tempstr, "Resist Acid: 75% MAX");
		break;
	case IPL_ALLRES:
		if (is->_iPLFR < 75)
			snprintf(tempstr, sizeof(tempstr), "Resist All: %+i%%", is->_iPLFR);
		else
			copy_cstr(tempstr, "Resist All: 75% MAX");
		break;
	case IPL_SPLLVLADD:
		snprintf(tempstr, sizeof(tempstr), "%+i to spell levels", is->_iSplLvlAdd);
		break;
	case IPL_CHARGES:
		copy_cstr(tempstr, "Extra charges");
		break;
	case IPL_SPELL:
		snprintf(tempstr, sizeof(tempstr), "%i %s charges", is->_iMaxCharges, spelldata[is->_iSpell].sNameText);
		break;
	case IPL_FIREDAM:
		if (is->_iFMinDam != is->_iFMaxDam)
			snprintf(tempstr, sizeof(tempstr), "fire damage: %i-%i", is->_iFMinDam, is->_iFMaxDam);
		else
			snprintf(tempstr, sizeof(tempstr), "fire damage: %i", is->_iFMinDam);
		break;
	case IPL_LIGHTDAM:
		if (is->_iLMinDam != is->_iLMaxDam)
			snprintf(tempstr, sizeof(tempstr), "lightning damage: %i-%i", is->_iLMinDam, is->_iLMaxDam);
		else
			snprintf(tempstr, sizeof(tempstr), "lightning damage: %i", is->_iLMinDam);
		break;
	case IPL_MAGICDAM:
		if (is->_iMMinDam != is->_iMMaxDam)
			snprintf(tempstr, sizeof(tempstr), "magic damage: %i-%i", is->_iMMinDam, is->_iMMaxDam);
		else
			snprintf(tempstr, sizeof(tempstr), "magic damage: %i", is->_iMMinDam);
		break;
	case IPL_ACIDDAM:
		if (is->_iAMinDam != is->_iAMaxDam)
			snprintf(tempstr, sizeof(tempstr), "holy damage: %i-%i", is->_iAMinDam, is->_iAMaxDam);
		else
			snprintf(tempstr, sizeof(tempstr), "holy damage: %i", is->_iAMinDam);
		break;
	case IPL_STR:
		snprintf(tempstr, sizeof(tempstr), "%+i to strength", is->_iPLStr);
		break;
	case IPL_MAG:
		snprintf(tempstr, sizeof(tempstr), "%+i to magic", is->_iPLMag);
		break;
	case IPL_DEX:
		snprintf(tempstr, sizeof(tempstr), "%+i to dexterity", is->_iPLDex);
		break;
	case IPL_VIT:
		snprintf(tempstr, sizeof(tempstr), "%+i to vitality", is->_iPLVit);
		break;
	case IPL_ATTRIBS:
		snprintf(tempstr, sizeof(tempstr), "%+i to all attributes", is->_iPLStr);
		break;
	case IPL_GETHIT:
		snprintf(tempstr, sizeof(tempstr), "%+i damage from enemies", is->_iPLGetHit);
		break;
	case IPL_LIFE:
		snprintf(tempstr, sizeof(tempstr), "Hit Points: %+i", is->_iPLHP >> 6);
		break;
	case IPL_MANA:
		snprintf(tempstr, sizeof(tempstr), "Mana: %+i", is->_iPLMana >> 6);
		break;
	case IPL_DUR:
		copy_cstr(tempstr, "high durability");
		break;
	case IPL_DUR_CURSE:
		copy_cstr(tempstr, "decreased durability");
		break;
	case IPL_INDESTRUCTIBLE:
		copy_cstr(tempstr, "indestructible");
		break;
	case IPL_LIGHT:
		snprintf(tempstr, sizeof(tempstr), "%+i%% light radius", 10 * is->_iPLLight);
		break;
#ifdef HELLFIRE
	case IPL_MULT_ARROWS:
		copy_cstr(tempstr, "multiple arrows per shot");
		break;
#endif
	case IPL_THORNS:
		copy_cstr(tempstr, "attacker takes 1-3 damage");
		break;
	case IPL_NOMANA:
		copy_cstr(tempstr, "user loses all mana");
		break;
	case IPL_KNOCKBACK:
		copy_cstr(tempstr, "knocks target back");
		break;
	case IPL_ALLRESZERO:
		copy_cstr(tempstr, "All Resistance equals 0");
		break;
	case IPL_NOHEALMON:
		copy_cstr(tempstr, "hit monster doesn't heal");
		break;
	case IPL_STEALMANA:
		snprintf(tempstr, sizeof(tempstr), "hit steals %i%% mana", (is->_iManaSteal * 100) >> 7);
		break;
	case IPL_STEALLIFE:
		snprintf(tempstr, sizeof(tempstr), "hit steals %i%% life", (is->_iLifeSteal * 100) >> 7);
		break;
	case IPL_TARGAC:
#ifdef HELLFIRE
		copy_cstr(tempstr, "penetrates target's armor");
#else
		copy_cstr(tempstr, "damages target's armor");
#endif
		break;
	case IPL_FASTATTACK:
		if (is->_iFlags & ISPL_FASTESTATTACK)
			copy_cstr(tempstr, "fastest attack");
		else if (is->_iFlags & ISPL_FASTERATTACK)
			copy_cstr(tempstr, "faster attack");
		else if (is->_iFlags & ISPL_FASTATTACK)
			copy_cstr(tempstr, "fast attack");
		else if (is->_iFlags & ISPL_QUICKATTACK)
			copy_cstr(tempstr, "quick attack");
		break;
	case IPL_FASTRECOVER:
		if (is->_iFlags & ISPL_FASTESTRECOVER)
			copy_cstr(tempstr, "fastest hit recovery");
		else if (is->_iFlags & ISPL_FASTERRECOVER)
			copy_cstr(tempstr, "faster hit recovery");
		else if (is->_iFlags & ISPL_FASTRECOVER)
			copy_cstr(tempstr, "fast hit recovery");
		break;
	case IPL_FASTBLOCK:
		copy_cstr(tempstr, "fast block");
		break;
	case IPL_DAMMOD:
		snprintf(tempstr, sizeof(tempstr), "adds %i points to damage", is->_iPLDamMod);
		break;
	case IPL_SETDAM:
		copy_cstr(tempstr, "unusual item damage");
		break;
	case IPL_SETDUR:
		copy_cstr(tempstr, "altered durability");
		break;
	case IPL_ONEHAND:
		copy_cstr(tempstr, "one handed sword");
		break;
	case IPL_DRAINLIFE:
		copy_cstr(tempstr, "constantly lose hit points");
		break;
	case IPL_NOMINSTR:
		copy_cstr(tempstr, "no strength requirement");
		break;
	case IPL_INFRAVISION:
		copy_cstr(tempstr, "see with infravision");
		break;
	case IPL_INVCURS:
		copy_cstr(tempstr, " ");
		break;
#ifdef HELLFIRE
	case IPL_CRYSTALLINE:
		snprintf(tempstr, sizeof(tempstr), "low dur, %+i%% damage", is->_iPLDam);
		break;
	case IPL_MANATOLIFE:
		copy_cstr(tempstr, "50% Mana moved to Health");
		break;
	case IPL_LIFETOMANA:
		copy_cstr(tempstr, "40% Health moved to Mana");
		break;
#endif
	case IPL_FASTCAST:
		if (is->_iFlags2 & ISPH_FASTESTCAST)
			copy_cstr(tempstr, "fastest cast");
		else if (is->_iFlags2 & ISPH_FASTERCAST)
			copy_cstr(tempstr, "faster cast");
		else if (is->_iFlags2 & ISPH_FASTCAST)
			copy_cstr(tempstr, "fast cast");
		break;
	case IPL_FASTWALK:
		if (is->_iFlags2 & ISPH_FASTESTWALK)
			copy_cstr(tempstr, "fastest walk");
		else if (is->_iFlags2 & ISPH_FASTERWALK)
			copy_cstr(tempstr, "faster walk");
		else if (is->_iFlags2 & ISPH_FASTWALK)
			copy_cstr(tempstr, "fast walk");
		break;
	default:
		ASSUME_UNREACHABLE
	}
}

static void DrawULine(int x)
{
	assert(gpBuffer != NULL);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[x - 6 + (SCREEN_Y + 25) * BUFFER_WIDTH];
	dst = &gpBuffer[x - 6 + (SCREEN_Y + 5 * 12 + 38) * BUFFER_WIDTH];

	for (i = 0; i < 3; i++, src += BUFFER_WIDTH, dst += BUFFER_WIDTH)
		memcpy(dst, src, 267);
}

static void PrintItemString(int x, int &y)
{
	PrintString(x, y, x + 257, tempstr, TRUE, COL_WHITE, 1);
	y += 24;
}

static void PrintItemString(int x, int &y, const char* str)
{
	PrintString(x, y, x + 257, str, TRUE, COL_WHITE, 1);
	y += 24;
}

static void PrintItemString(int x, int &y, const char* str, int col)
{
	PrintString(x, y, x + 257, str, TRUE, col, 1);
	y += 24;
}

static void PrintUniquePower(BYTE plidx, ItemStruct *is, int x, int &y)
{
	if (plidx != IPL_INVCURS) {
		PrintItemPower(plidx, is);
		PrintItemString(x, y);
	}
}

void DrawUniqueInfo(ItemStruct *is, int x, int &y)
{
	const UItemStruct *uis;

	uis = &UniqueItemList[is->_iUid];
	PrintUniquePower(uis->UIPower1, is, x, y);
	if (uis->UIPower2 == IPL_INVALID)
		return;
	PrintUniquePower(uis->UIPower2, is, x, y);
	if (uis->UIPower3 == IPL_INVALID)
		return;
	PrintUniquePower(uis->UIPower3, is, x, y);
	if (uis->UIPower4 == IPL_INVALID)
		return;
	PrintUniquePower(uis->UIPower4, is, x, y);
	if (uis->UIPower5 == IPL_INVALID)
		return;
	PrintUniquePower(uis->UIPower5, is, x, y);
	if (uis->UIPower6 == IPL_INVALID)
		return;
	PrintUniquePower(uis->UIPower6, is, x, y);
}

static int ItemColor(ItemStruct *is)
{
	if (is->_iMagical == ITEM_QUALITY_MAGIC)
		return COL_BLUE;
	if (is->_iMagical == ITEM_QUALITY_UNIQUE)
		return COL_GOLD;
	return COL_WHITE;
}

static void PrintItemMiscInfo(const ItemStruct *is, int x, int &y)
{
	const char *desc;

	switch (is->_iMiscId) {
	case IMISC_NONE:
		return;
	case IMISC_HEAL:
		desc = "recover partial life";
		PrintItemString(x, y, desc);
		break;
	case IMISC_FULLHEAL:
		desc = "fully recover life";
		PrintItemString(x, y, desc);
		break;
	case IMISC_MANA:
		desc = "recover mana";
		PrintItemString(x, y, desc);
		break;
	case IMISC_FULLMANA:
		desc = "fully recover mana";
		PrintItemString(x, y, desc);
		break;
	case IMISC_REJUV:
		desc = "recover life and mana";
		PrintItemString(x, y, desc);
		break;
	case IMISC_FULLREJUV:
		desc = "fully recover life and mana";
		PrintItemString(x, y, desc);
		break;
	case IMISC_SCROLL:
		desc = "Right-click to read";
		PrintItemString(x, y, desc);
		return;
	/*case IMISC_SCROLLT:
		desc = "Right-click to read, then";
		PrintItemString(x, y, desc);
		desc = "left-click to target";
		PrintItemString(x, y, desc);
		return;*/
	case IMISC_BOOK:
		desc = "Right-click to read";
		PrintItemString(x, y, desc);
		return;
	case IMISC_UNIQUE:
		return;
	case IMISC_EAR:
		snprintf(tempstr, sizeof(tempstr), "Level : %i", is->_ivalue);
		PrintItemString(x, y);
		return;
	case IMISC_SPECELIX:
		desc = "strange glowing liquid";
		PrintItemString(x, y, desc);
		return;
	case IMISC_OILQLTY:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "increase its potential";
		PrintItemString(x, y, desc);
		desc = "(damage/defense)";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILZEN:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "increase inner potential";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILSTR:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "increase inner strength";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILDEX:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "increase inner dexterity";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILVIT:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "increase inner vitality";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILMAG:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "increase inner magic";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILCLEAN:
		desc = "removes the affixes";
		PrintItemString(x, y, desc);
		desc = "of a magic item";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILRESIST:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "protect against magic";
		PrintItemString(x, y, desc);
		break;
	case IMISC_OILCHANCE:
		desc = "imbues a normal item to";
		PrintItemString(x, y, desc);
		desc = "a magic item";
		PrintItemString(x, y, desc);
		break;
#ifdef HELLFIRE
	case IMISC_MAPOFDOOM:
		desc = "Right-click to view";
		PrintItemString(x, y, desc);
		return;
	case IMISC_RUNE:
		desc = "Right-click to activate, then";
		PrintItemString(x, y, desc);
		desc = "left-click to place";
		PrintItemString(x, y, desc);
		return;
	case IMISC_NOTE:
		desc = "Right click to read";
		PrintItemString(x, y, desc);
		return;
#endif
	default:
		ASSUME_UNREACHABLE;
		return;
	}

	desc = "Right click to use";
	PrintItemString(x, y, desc);
	return;
}

void DrawInvItemDetails()
{
	ItemStruct* is = PlrItem(myplr, pcursinvitem);
	int x = SCREEN_X + (RIGHT_PANEL - 271) / 2 + 8;
	int y = SCREEN_Y + 44 + 24;

	// draw the background
	CelDraw(x - 8, SCREEN_Y + 327, pSTextBoxCels, 1, 271);
	trans_rect(x - 5, SCREEN_Y + 28, 265, 297);

	// print the name as title
	PrintItemString(x, y, 
		is->_iIdentified ? is->_iIName : is->_iName, ItemColor(is));

	// add separator
	DrawULine(x);
	y += 30;
	if (is->_iClass == ICLASS_GOLD) {
		snprintf(tempstr, sizeof(tempstr), "%i gold %s", is->_ivalue, get_pieces_str(is->_ivalue));
		PrintItemString(x, y);
	} else if (is->_iClass == ICLASS_WEAPON) {
		if (is->_iMinDam == is->_iMaxDam)
			snprintf(tempstr, sizeof(tempstr), "Damage: %i", is->_iMinDam);
		else
			snprintf(tempstr, sizeof(tempstr), "Damage: %i-%i", is->_iMinDam, is->_iMaxDam);
		PrintItemString(x, y);
		if (is->_iMaxDur != DUR_INDESTRUCTIBLE) {
			snprintf(tempstr, sizeof(tempstr), "Durability: %i/%i", is->_iDurability, is->_iMaxDur);
			PrintItemString(x, y);
		}
		if (is->_iMaxCharges != 0) {
			snprintf(tempstr, sizeof(tempstr), "Charges: %i/%i", is->_iCharges, is->_iMaxCharges);
			PrintItemString(x, y);
		}
	} else if (is->_iClass == ICLASS_ARMOR) {
		snprintf(tempstr, sizeof(tempstr), "Armor: %i", is->_iAC);
		PrintItemString(x, y);
		if (is->_iMaxDur != DUR_INDESTRUCTIBLE) {
			snprintf(tempstr, sizeof(tempstr), "Durability: %i/%i", is->_iDurability, is->_iMaxDur);
			PrintItemString(x, y);
		}
	} else {
		y += 12 * 2;
	}
	if (is->_iMagical != ITEM_QUALITY_NORMAL) {
		if (is->_iIdentified) {
			if (is->_iPrePower != IPL_INVALID) {
				PrintItemPower(is->_iPrePower, is);
				PrintItemString(x, y);
			}
			if (is->_iSufPower != IPL_INVALID) {
				PrintItemPower(is->_iSufPower, is);
				PrintItemString(x, y);
			}
			if (is->_iMagical == ITEM_QUALITY_UNIQUE) {
				DrawUniqueInfo(is, x, y);
			}
		} else {
			copy_cstr(tempstr, "Not Identified");
			PrintItemString(x, y);
		}
	}
	PrintItemMiscInfo(is, x, y);
	if ((is->_iMinStr | is->_iMinMag | is->_iMinDex) != 0) {
		int cursor = 0;
		cat_cstr(tempstr, cursor, "Required:");
		if (is->_iMinStr != 0)
			cat_str(tempstr, cursor, " %i Str", is->_iMinStr);
		if (is->_iMinMag != 0)
			cat_str(tempstr, cursor, " %i Mag", is->_iMinMag);
		if (is->_iMinDex != 0)
			cat_str(tempstr, cursor, " %i Dex", is->_iMinDex);
		PrintItemString(x, y, tempstr, is->_iStatFlag ? COL_WHITE : COL_RED);
	}
}

void ItemStatOk(int pnum, ItemStruct *is)
{
	PlayerStruct *p = &plr[pnum];

	is->_iStatFlag = p->_pStrength >= is->_iMinStr
				  && p->_pDexterity >= is->_iMinDex
				  && p->_pMagic >= is->_iMinMag;
}

static BOOL SmithItemOk(int i)
{
	return AllItemsList[i].itype != ITYPE_MISC
	 && AllItemsList[i].itype != ITYPE_GOLD
	 && AllItemsList[i].itype != ITYPE_RING
	 && AllItemsList[i].itype != ITYPE_AMULET;
}

static int RndSmithItem(int lvl)
{
	int i, ri;
	int ril[NUM_IDI * 2];

	ri = 0;
	for (i = 1; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && SmithItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
			ril[ri] = i;
			ri++;
			if (AllItemsList[i].iRnd == IDROP_DOUBLE) {
				ril[ri] = i;
				ri++;
			}
		}
	}

	return ril[random_(50, ri)];
}

static void BubbleSwapItem(ItemStruct *a, ItemStruct *b)
{
	ItemStruct h;

	copy_pod(h, *a);
	copy_pod(*a, *b);
	copy_pod(*b, h);
}

static void SortSmith()
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
			if (smithitem[k]._iIdx > smithitem[k + 1]._iIdx) {
				BubbleSwapItem(&smithitem[k], &smithitem[k + 1]);
				sorted = FALSE;
			}
		}
		j--;
	}
}

void SpawnSmith(int lvl)
{
	int i, iCnt, seed;

	iCnt = RandRange(10, SMITH_ITEMS - 1);
	for (i = 0; i < iCnt; i++) {
		do {
			seed = GetRndSeed();
			SetRndSeed(seed);
			GetItemAttrs(0, RndSmithItem(lvl), lvl);
		} while (item[0]._iIvalue > SMITH_MAX_VALUE);
		item[0]._iSeed = seed;
		item[0]._iCreateInfo = lvl | CF_SMITH;
		copy_pod(smithitem[i], item[0]);
	}
	for ( ; i < SMITH_ITEMS; i++)
		smithitem[i]._itype = ITYPE_NONE;

	SortSmith();
}

static int RndPremiumItem(int minlvl, int maxlvl)
{
	int i, ri;
	int ril[NUM_IDI];

	ri = 0;
	for (i = 1; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && SmithItemOk(i)) {
			if (AllItemsList[i].iMinMLvl >= minlvl && AllItemsList[i].iMinMLvl <= maxlvl) {
				ril[ri] = i;
				ri++;
			}
		}
	}

	return ril[random_(50, ri)];
}

static void SpawnOnePremium(int i, int plvl)
{
	int seed;

	if (plvl > 30)
		plvl = 30;
	if (plvl < 1)
		plvl = 1;
	do {
		seed = GetRndSeed();
		SetRndSeed(seed);
		GetItemAttrs(0, RndPremiumItem(plvl >> 2, plvl), plvl);
		GetItemBonus(0, plvl >> 1, plvl, TRUE, FALSE);
	} while (item[0]._iIvalue > SMITH_MAX_PREMIUM_VALUE);
	item[0]._iSeed = seed;
	item[0]._iCreateInfo = plvl | CF_SMITHPREMIUM;
	copy_pod(premiumitem[i], item[0]);
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
#ifdef HELLFIRE
		copy_pod(premiumitem[0], premiumitem[3]);
		copy_pod(premiumitem[1], premiumitem[4]);
		copy_pod(premiumitem[2], premiumitem[5]);
		copy_pod(premiumitem[3], premiumitem[6]);
		copy_pod(premiumitem[4], premiumitem[7]);
		copy_pod(premiumitem[5], premiumitem[8]);
		copy_pod(premiumitem[6], premiumitem[9]);
		copy_pod(premiumitem[7], premiumitem[10]);
		copy_pod(premiumitem[8], premiumitem[11]);
		copy_pod(premiumitem[9], premiumitem[12]);
		SpawnOnePremium(10, premiumlevel + premiumlvladd[10]);
		copy_pod(premiumitem[11], premiumitem[13]);
		SpawnOnePremium(12, premiumlevel + premiumlvladd[12]);
		copy_pod(premiumitem[13], premiumitem[14]);
		SpawnOnePremium(14, premiumlevel + premiumlvladd[14]);
#else
		copy_pod(premiumitem[0], premiumitem[2]);
		copy_pod(premiumitem[1], premiumitem[3]);
		copy_pod(premiumitem[2], premiumitem[4]);
		SpawnOnePremium(3, premiumlevel + premiumlvladd[3]);
		copy_pod(premiumitem[4], premiumitem[5]);
		SpawnOnePremium(5, premiumlevel + premiumlvladd[5]);
#endif
	}
}

static BOOL WitchItemOk(int i)
{
	return AllItemsList[i].itype == ITYPE_STAFF
	 || (AllItemsList[i].itype == ITYPE_MISC
	  && (AllItemsList[i].iMiscId == IMISC_BOOK
	   || AllItemsList[i].iMiscId == IMISC_SCROLL
	   || AllItemsList[i].iMiscId == IMISC_RUNE
	   || AllItemsList[i].iMiscId == IMISC_REJUV
	   || AllItemsList[i].iMiscId == IMISC_FULLREJUV));
}

static int RndWitchItem(int lvl)
{
	int i, ri;
	int ril[NUM_IDI];

	ri = 0;
	for (i = 1; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && WitchItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(51, ri)];
}

static void SortWitch()
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
			if (witchitem[k]._iIdx > witchitem[k + 1]._iIdx) {
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
			if (is->_iMinMag + 20 * is->_iMinMag / 100 > UCHAR_MAX) {
				is->_iMinMag = UCHAR_MAX;
				slvl = 0;
			}
		}
	}
}

void SpawnWitch(int lvl)
{
	int i, iCnt;
	int seed;

	SetItemSData(&witchitem[0], IDI_MANA);
	SetItemSData(&witchitem[1], IDI_FULLMANA);
	SetItemSData(&witchitem[2], IDI_PORTAL);

	iCnt = RandRange(10, WITCH_ITEMS - 1);
	for (i = 3; i < iCnt; i++) {
		do {
			seed = GetRndSeed();
			SetRndSeed(seed);
			GetItemAttrs(0, RndWitchItem(lvl), lvl);
			if (random_(51, 100) <= 5 || item[0]._itype == ITYPE_STAFF)
				GetItemBonus(0, lvl, lvl << 1, TRUE, TRUE);
		} while (item[0]._iIvalue > WITCH_MAX_VALUE);
		item[0]._iSeed = seed;
		item[0]._iCreateInfo = lvl | CF_WITCH;
		copy_pod(witchitem[i], item[0]);
	}

	for ( ; i < WITCH_ITEMS; i++)
		witchitem[i]._itype = ITYPE_NONE;

	SortWitch();
}

static int RndBoyItem(int lvl)
{
	int i, ri;
	int ril[NUM_IDI];

	ri = 0;
	for (i = 1; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && SmithItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(49, ri)];
}

void SpawnBoy(int lvl)
{
	int seed;

	if (boylevel < (lvl >> 1) || boyitem._itype == ITYPE_NONE) {
		do {
			seed = GetRndSeed();
			SetRndSeed(seed);
			GetItemAttrs(0, RndBoyItem(lvl), lvl);
			GetItemBonus(0, lvl, lvl << 1, TRUE, TRUE);
		} while (item[0]._iIvalue > BOY_MAX_VALUE);
		item[0]._iSeed = seed;
		item[0]._iCreateInfo = lvl | CF_BOY;
		copy_pod(boyitem, item[0]);
		boylevel = lvl >> 1;
	}
}

static BOOL HealerItemOk(int i)
{
	return AllItemsList[i].iMiscId == IMISC_REJUV
		|| AllItemsList[i].iMiscId == IMISC_FULLREJUV
		|| AllItemsList[i].iMiscId == IMISC_SCROLL;
}

static int RndHealerItem(int lvl)
{
	int i, ri;
	int ril[NUM_IDI];

	ri = 0;
	for (i = 1; i < NUM_IDI; i++) {
		if (AllItemsList[i].iRnd != IDROP_NEVER && HealerItemOk(i) && lvl >= AllItemsList[i].iMinMLvl) {
			ril[ri] = i;
			ri++;
		}
	}

	return ril[random_(50, ri)];
}

static void SortHealer()
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
			if (healitem[k]._iIdx > healitem[k + 1]._iIdx) {
				BubbleSwapItem(&healitem[k], &healitem[k + 1]);
				sorted = FALSE;
			}
		}
		j--;
	}
}

void SpawnHealer(int lvl)
{
	int i, iCnt, srnd, seed;

	SetItemSData(&healitem[0], IDI_HEAL);
	SetItemSData(&healitem[1], IDI_FULLHEAL);

	if (gbMaxPlayers != 1) {
		SetItemSData(&healitem[2], IDI_RESURRECT);

		srnd = 3;
	} else {
		srnd = 2;
	}
	iCnt = RandRange(10, HEALER_ITEMS - 1);
	for (i = srnd; i < iCnt; i++) {
		do {
			seed = GetRndSeed();
			SetRndSeed(seed);
			GetItemAttrs(0, RndHealerItem(lvl), lvl);
		} while (item[0]._iSpell != SPL_NULL && item[0]._iSpell != SPL_HEAL
			&& (item[0]._iSpell != SPL_HEALOTHER || gbMaxPlayers == 1));
		item[0]._iSeed = seed;
		item[0]._iCreateInfo = lvl | CF_HEALER;
		copy_pod(healitem[i], item[0]);
	}
	for ( ; i < HEALER_ITEMS; i++) {
		healitem[i]._itype = ITYPE_NONE;
	}
	SortHealer();
}

void SpawnStoreGold()
{
	SetItemSData(&golditem, IDI_GOLD);
	golditem._iStatFlag = TRUE;
}

static void RecreateSmithItem(int ii, int idx, int lvl, int iseed)
{
	SetRndSeed(iseed);
	GetItemAttrs(ii, RndSmithItem(lvl), lvl);

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_SMITH;
}

static void RecreatePremiumItem(int ii, int idx, int plvl, int iseed)
{
	SetRndSeed(iseed);
	GetItemAttrs(ii, RndPremiumItem(plvl >> 2, plvl), plvl);
	GetItemBonus(ii, plvl >> 1, plvl, TRUE, FALSE);

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = plvl | CF_SMITHPREMIUM;
}

static void RecreateBoyItem(int ii, int idx, int lvl, int iseed)
{
	SetRndSeed(iseed);
	GetItemAttrs(ii, RndBoyItem(lvl), lvl);
	GetItemBonus(ii, lvl, lvl << 1, TRUE, TRUE);
	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_BOY;
}

static void RecreateWitchItem(int ii, int idx, int lvl, int iseed)
{
	if (idx == IDI_MANA || idx == IDI_FULLMANA || idx == IDI_PORTAL) {
		GetItemAttrs(ii, idx, lvl);
	} else {
		SetRndSeed(iseed);
		GetItemAttrs(ii, RndWitchItem(lvl), lvl);
		if (random_(51, 100) <= 5 || item[ii]._itype == ITYPE_STAFF)
			GetItemBonus(ii, lvl, lvl << 1, TRUE, TRUE);
	}

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_WITCH;
}

static void RecreateHealerItem(int ii, int idx, int lvl, int iseed)
{
	if (idx == IDI_HEAL || idx == IDI_FULLHEAL || idx == IDI_RESURRECT) {
		GetItemAttrs(ii, idx, lvl);
	} else {
		SetRndSeed(iseed);
		GetItemAttrs(ii, RndHealerItem(lvl), lvl);
	}

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_HEALER;
}

static void RecreateCraftedItem(int ii, int idx, int lvl, int iseed)
{
	SetRndSeed(iseed);
	GetItemAttrs(ii, idx, lvl);
	if (random_(51, 2) != 0)
		GetItemBonus(ii, 0, lvl != 0 ? lvl : 1, TRUE, TRUE);

	item[ii]._iSeed = iseed;
	item[ii]._iCreateInfo = lvl | CF_CRAFTED;
}

void RecreateTownItem(int ii, int idx, WORD icreateinfo, int iseed)
{
	int loc, lvl;

	loc = (icreateinfo & CF_TOWN) >> 10;
	lvl = icreateinfo & CF_LEVEL;
	switch (loc) {
	case CFL_SMITH:
		RecreateSmithItem(ii, idx, lvl, iseed);
		break;
	case CFL_SMITHPREMIUM:
		RecreatePremiumItem(ii, idx, lvl, iseed);
		break;
	case CFL_BOY:
		RecreateBoyItem(ii, idx, lvl, iseed);
		break;
	case CFL_WITCH:
		RecreateWitchItem(ii, idx, lvl, iseed);
		break;
	case CFL_HEALER:
		RecreateHealerItem(ii, idx, lvl, iseed);
		break;
	case CFL_CRAFTED:
		RecreateCraftedItem(ii, idx, lvl, iseed);
		break;
	default:
		ASSUME_UNREACHABLE;
		break;
	}
}

int ItemNoFlippy()
{
	int ii;

	ii = itemactive[numitems - 1];
	item[ii]._iAnimFrame = item[ii]._iAnimLen;
	item[ii]._iAnimFlag = FALSE;
	item[ii]._iSelFlag = 1;

	return ii;
}

void CreateSpellBook(int ispell, int x, int y)
{
	int ii, idx, lvl;

	if (numitems >= MAXITEMS)
		return;

	lvl = spelldata[ispell].sBookLvl;
	assert(lvl != SPELL_NA);
	lvl <<= 1;

	idx = RndTypeItems(ITYPE_MISC, IMISC_BOOK, lvl);

	ii = itemavail[0];
	while (TRUE) {
		SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, TRUE); // BUGFIX: pregen?
		assert(item[ii]._iMiscId == IMISC_BOOK);
		if (item[ii]._iSpell == ispell)
			break;
	}
	GetSuperItemSpace(x, y, ii);
	DeltaAddItem(ii); // BUGFIX: sure? How about sending a message?

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

void CreateMagicArmor(int imisc, int icurs, int x, int y)
{
	int ii, idx, lvl;

	if (numitems >= MAXITEMS)
		return;

	lvl = items_get_currlevel();
	lvl <<= 1;

	ii = itemavail[0];
	while (TRUE) {
		idx = RndTypeItems(imisc, IMISC_NONE, lvl);
		SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, TRUE); // BUGFIX: pregen?
		if (item[ii]._iCurs == icurs)
			break;
	}
	GetSuperItemSpace(x, y, ii);
	DeltaAddItem(ii); // BUGFIX: sure? How about sending a message?

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}

#ifdef HELLFIRE
void CreateAmulet(int x, int y)
{
	int ii, lvl, idx;

	if (numitems >= MAXITEMS)
		return;

	lvl = 26; // BUGFIX: make sure there is an amulet which fits?

	ii = itemavail[0];
	while (TRUE) {
		idx = RndTypeItems(ITYPE_AMULET, IMISC_NONE, lvl);
		SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, TRUE); // BUGFIX: pregen?
		if (item[ii]._iCurs == ICURS_AMULET)
			break;
	}
	GetSuperItemSpace(x, y, ii);
	DeltaAddItem(ii); // BUGFIX: sure? How about sending a message?

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
}
#endif

void CreateMagicWeapon(int itype, int icurs, int x, int y)
{
	int ii, idx, lvl;

	if (numitems >= MAXITEMS)
		return;

	lvl = items_get_currlevel();
	lvl <<= 1;

	ii = itemavail[0];
	while (TRUE) {
		idx = RndTypeItems(itype, IMISC_NONE, lvl);
		SetupAllItems(ii, idx, GetRndSeed(), lvl, 1, TRUE, FALSE, TRUE); // BUGFIX: pregen?
		if (item[ii]._iCurs == icurs)
			break;
	}
	GetSuperItemSpace(x, y, ii);
	DeltaAddItem(ii); // BUGFIX: sure? How about sending a message?

	itemactive[numitems] = ii;
	itemavail[0] = itemavail[MAXITEMS - numitems - 1];
	numitems++;
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
