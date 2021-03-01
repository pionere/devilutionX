/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Tracks which missile files are already loaded */
int MissileFileFlag;

int monstactive[MAXMONSTERS];
int nummonsters;
BOOLEAN sgbSaveSoundOn;
MonsterStruct monster[MAXMONSTERS];
int totalmonsters;
CMonster Monsters[MAX_LVLMTYPES];
int monstimgtot;
int uniquetrans;
int nummtypes;

/** Light radius of unique monsters */
#define MON_LIGHTRAD 3

/** Maps from walking path step to facing direction. */
const char plr2monst[9] = { 0, 5, 3, 7, 1, 4, 6, 0, 2 };
/** Maps from monster intelligence factor to missile type. */
const BYTE counsmiss[4] = { MIS_FIREBOLT, MIS_CBOLTC, MIS_LIGHTNINGC, MIS_FIREBALL };

/* data */

/** Maps from monster walk animation frame num to monster velocity. */
const int MWVel[24][3] = {
	{ 256, 512, 1024 },
	{ 128, 256, 512 },
	{ 85, 170, 341 },
	{ 64, 128, 256 },
	{ 51, 102, 204 },
	{ 42, 85, 170 },
	{ 36, 73, 146 },
	{ 32, 64, 128 },
	{ 28, 56, 113 },
	{ 26, 51, 102 },
	{ 23, 46, 93 },
	{ 21, 42, 85 },
	{ 19, 39, 78 },
	{ 18, 36, 73 },
	{ 17, 34, 68 },
	{ 16, 32, 64 },
	{ 15, 30, 60 },
	{ 14, 28, 57 },
	{ 13, 26, 54 },
	{ 12, 25, 51 },
	{ 12, 24, 48 },
	{ 11, 23, 46 },
	{ 11, 22, 44 },
	{ 10, 21, 42 }
};
/** Maps from monster action to monster animation letter. */
const char animletter[6] = { 'n', 'w', 'a', 'h', 'd', 's' };
/** Maps from direction to a left turn from the direction. */
const int left[8] = { 7, 0, 1, 2, 3, 4, 5, 6 };
/** Maps from direction to a right turn from the direction. */
const int right[8] = { 1, 2, 3, 4, 5, 6, 7, 0 };
/** Maps from direction to the opposite direction. */
const int opposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };
/** Maps from direction to delta X-offset. */
const int offset_x[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to delta Y-offset. */
const int offset_y[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };

/** Maps from monster AI ID to monster AI function. */
void (*AiProc[])(int i) = {
	&MAI_Zombie,
	&MAI_Fat,
	&MAI_SkelSd,
	&MAI_SkelBow,
	&MAI_Scav,
	&MAI_Rhino,
	&MAI_GoatMc,
	&MAI_GoatBow,
	&MAI_Fallen,
	&MAI_Magma,
	&MAI_SkelKing,
	&MAI_Bat,
	&MAI_Garg,
	&MAI_Cleaver,
	&MAI_Succ,
	&MAI_SnowWich,
	&MAI_HlSpwn,
	&MAI_SolBrnr,
	&MAI_Sneak,
	&MAI_Storm,
	&MAI_Fireman,
	&MAI_Garbud,
	&MAI_Acid,
	&MAI_AcidUniq,
	&MAI_Golum,
	&MAI_Zhar,
	&MAI_SnotSpil,
	&MAI_Snake,
	&MAI_Counselor,
	&MAI_Mega,
	&MAI_Diablo,
	&MAI_Lazurus,
	&MAI_Lazhelp,
	&MAI_Lachdanan,
	&MAI_Warlord,
#ifdef HELLFIRE
	&MAI_Firebat,
	&MAI_Torchant,
	&MAI_Horkdemon,
	&MAI_Lich,
	&MAI_ArchLich,
	&MAI_PsychOrb,
	&MAI_NecromOrb,
	&MAI_BoneDemon,
#endif
	&MAI_Storm2,
};

static void InitMonsterTRN(int midx, BOOL special)
{
	CMonster *cmon;
	BYTE *f;
	int i, n, j;

	cmon = &Monsters[midx];
	f = cmon->trans_file;
	for (i = 0; i < 256; i++) {
		if (*f == 255) {
			*f = 0;
		}
		f++;
	}

	n = special ? 6 : 5;
	for (i = 0; i < n; i++) {
		if (i != 1 || cmon->mtype < MT_COUNSLR || cmon->mtype > MT_ADVOCATE) {
			for (j = 0; j < lengthof(cmon->Anims[i].Data); j++) {
				Cl2ApplyTrans(
				    cmon->Anims[i].Data[j],
				    cmon->trans_file,
				    cmon->Anims[i].Frames);
			}
		}
	}
}

void InitLevelMonsters()
{
	int i;

	nummonsters = 0;
	nummtypes = 0;
	monstimgtot = 0;
	MissileFileFlag = 0;
	uniquetrans = 0;

	memset(monster, 0, sizeof(monster));

	totalmonsters = MAXMONSTERS;
	for (i = 0; i < MAXMONSTERS; i++) {
		monstactive[i] = i;
	}
}

static int AddMonsterType(int type, int placeflag)
{
	int i;

	for (i = 0; i < nummtypes && Monsters[i].mtype != type; i++)
		;

	if (i == nummtypes) {
		nummtypes++;
		Monsters[i].mtype = type;
		Monsters[i].mPlaceFlags = 0;
		monstimgtot += monsterdata[type].mImage;
		InitMonsterGFX(i);
		InitMonsterSND(i);
	}

	Monsters[i].mPlaceFlags |= placeflag;
	return i;
}

void GetLevelMTypes()
{
	int i;

	// this array is merged with skeltypes down below.
	int typelist[MAXMONSTERS];
	int skeltypes[NUM_MTYPES];

	BYTE lvl;
	const int numskeltypes = 19;

	int nt; // number of types

	AddMonsterType(MT_GOLEM, PLACE_SPECIAL);
	lvl = currlevel;
	if (lvl == 16) {
		AddMonsterType(MT_ADVOCATE, PLACE_SCATTER);
		AddMonsterType(MT_RBLACK, PLACE_SCATTER);
		AddMonsterType(MT_DIABLO, PLACE_SPECIAL);
		return;
	}

#ifdef HELLFIRE
	if (lvl == 18)
		AddMonsterType(MT_HORKSPWN, PLACE_SCATTER);
	if (lvl == 19) {
		AddMonsterType(MT_HORKSPWN, PLACE_SCATTER);
		AddMonsterType(MT_HORKDMN, PLACE_UNIQUE);
	}
	if (lvl == 20)
		AddMonsterType(MT_DEFILER, PLACE_UNIQUE);
	if (lvl == 24) {
		AddMonsterType(MT_ARCHLICH, PLACE_SCATTER);
		AddMonsterType(MT_NAKRUL, PLACE_SPECIAL);
	}
#endif

	if (!setlevel) {
		if (QuestStatus(Q_BUTCHER))
			AddMonsterType(MT_CLEAVER, PLACE_SPECIAL);
		if (QuestStatus(Q_GARBUD))
			AddMonsterType(UniqMonst[UMT_GARBUD].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_ZHAR))
			AddMonsterType(UniqMonst[UMT_ZHAR].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_LTBANNER))
			AddMonsterType(UniqMonst[UMT_SNOTSPIL].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_VEIL))
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, PLACE_UNIQUE);
		if (QuestStatus(Q_WARLORD))
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, PLACE_UNIQUE);

		if (gbMaxPlayers != 1 && lvl == quests[Q_SKELKING]._qlevel) {

			AddMonsterType(MT_SKING, PLACE_UNIQUE);

			nt = 0;
			for (i = MT_WSKELAX; i <= MT_WSKELAX + numskeltypes; i++) {
				if (IsSkel(i)) {
					if (lvl >= monsterdata[i].mMinDLvl && lvl <= monsterdata[i].mMaxDLvl) {
						skeltypes[nt++] = i;
					}
				}
			}
			AddMonsterType(skeltypes[random_(88, nt)], PLACE_SCATTER);
		}

		nt = 0;
		for (i = 0; i < NUM_MTYPES; i++) {
			if (lvl >= monsterdata[i].mMinDLvl && lvl <= monsterdata[i].mMaxDLvl) {
				typelist[nt++] = i;
			}
		}

#ifdef _DEBUG
		if (monstdebug) {
			for (i = 0; i < debugmonsttypes; i++)
				AddMonsterType(DebugMonsters[i], PLACE_SCATTER);
			return;
		}
#endif
		while (nt > 0 && nummtypes < MAX_LVLMTYPES && monstimgtot < 4000) {
			for (i = 0; i < nt;) {
				if (monsterdata[typelist[i]].mImage > 4000 - monstimgtot) {
					typelist[i] = typelist[--nt];
					continue;
				}

				i++;
			}

			if (nt != 0) {
				i = random_(88, nt);
				AddMonsterType(typelist[i], PLACE_SCATTER);
				typelist[i] = typelist[--nt];
			}
		}
	} else {
		if (setlvlnum == SL_SKELKING) {
			AddMonsterType(MT_SKING, PLACE_UNIQUE);
		}
	}
}

void InitMonsterGFX(int midx)
{
	CMonster *cmon;
	const MonsterData *mdata;
	int mtype, anim, i;
	char strBuff[256];
	BYTE *celBuf;

	cmon = &Monsters[midx];
	mtype = cmon->mtype;
	mdata = &monsterdata[mtype];

	// static_assert(lengthof(animletter) == lengthof(monsterdata[0].Frames), "");
	for (anim = 0; anim < lengthof(animletter); anim++) {
		if ((animletter[anim] != 's' || mdata->has_special) && mdata->Frames[anim] > 0) {
			snprintf(strBuff, sizeof(strBuff), mdata->GraphicType, animletter[anim]);

			celBuf = LoadFileInMem(strBuff, NULL);
			cmon->Anims[anim].CMem = celBuf;

			if (mtype != MT_GOLEM || (animletter[anim] != 's' && animletter[anim] != 'd')) {
				for (i = 0; i < lengthof(cmon->Anims[anim].Data); i++) {
					cmon->Anims[anim].Data[i] = CelGetFrameStart(celBuf, i);
				}
			} else {
				for (i = 0; i < lengthof(cmon->Anims[anim].Data); i++) {
					cmon->Anims[anim].Data[i] = celBuf;
				}
			}
		}

		// TODO: either the AnimStruct members have wrong naming or the MonsterData ones it seems
		cmon->Anims[anim].Frames = mdata->Frames[anim];
		cmon->Anims[anim].Rate = mdata->Rate[anim];
	}

	cmon->width = mdata->width;
	cmon->width2 = (mdata->width - 64) >> 1;
	cmon->mMinHP = mdata->mMinHP;
	cmon->mMaxHP = mdata->mMaxHP;
	cmon->has_special = mdata->has_special;
	cmon->mAFNum = mdata->mAFNum;
	cmon->MData = mdata;

	if (mdata->has_trans) {
		cmon->trans_file = LoadFileInMem(mdata->TransFile, NULL);
		InitMonsterTRN(midx, mdata->has_special);
		MemFreeDbg(cmon->trans_file);
	}

	switch (mtype) {
	case MT_NMAGMA:
	case MT_YMAGMA:
	case MT_BMAGMA:
	case MT_WMAGMA:
		if (!(MissileFileFlag & 1)) {
			MissileFileFlag |= 1;
			LoadMissileGFX(MFILE_MAGBALL);
		}
		break;
	case MT_INCIN:
	case MT_FLAMLRD:
	case MT_DOOMFIRE:
	case MT_HELLBURN:
		if (!(MissileFileFlag & 8)) {
			MissileFileFlag |= 8;
			LoadMissileGFX(MFILE_KRULL);
		}
		break;
	case MT_STORM:
	case MT_RSTORM:
	case MT_STORML:
	case MT_MAEL:
		if (!(MissileFileFlag & 2)) {
			MissileFileFlag |= 2;
			LoadMissileGFX(MFILE_THINLGHT);
		}
		break;
	/*case MT_SUCCUBUS:
		if (!(MissileFileFlag & 4)) {
			MissileFileFlag |= 4;
			LoadMissileGFX(MFILE_FLARE);
			LoadMissileGFX(MFILE_FLAREEXP);
		}
		break;*/
	case MT_NACID:
	case MT_RACID:
	case MT_BACID:
	case MT_XACID:
#ifdef HELLFIRE
	case MT_SPIDLORD:
#endif
		if (!(MissileFileFlag & 0x10)) {
			MissileFileFlag |= 0x10;
			LoadMissileGFX(MFILE_ACIDBF);
			LoadMissileGFX(MFILE_ACIDSPLA);
			LoadMissileGFX(MFILE_ACIDPUD);
		}
		break;
	case MT_SNOWWICH:
		if (!(MissileFileFlag & 0x20)) {
			MissileFileFlag |= 0x20;
			LoadMissileGFX(MFILE_SCUBMISB);
			LoadMissileGFX(MFILE_SCBSEXPB);
		}
		break;
	case MT_HLSPWN:
		if (!(MissileFileFlag & 0x40)) {
			MissileFileFlag |= 0x40;
			LoadMissileGFX(MFILE_SCUBMISD);
			LoadMissileGFX(MFILE_SCBSEXPD);
		}
		break;
	case MT_SOLBRNR:
		if (!(MissileFileFlag & 0x80)) {
			MissileFileFlag |= 0x80;
			LoadMissileGFX(MFILE_SCUBMISC);
			LoadMissileGFX(MFILE_SCBSEXPC);
		}
		break;
	case MT_DIABLO:
		LoadMissileGFX(MFILE_FIREPLAR);
		break;
#ifdef HELLFIRE
	case MT_BONEDEMN:
		if (!(MissileFileFlag & 0x400)) {
			MissileFileFlag |= 0x400;
			LoadMissileGFX(MFILE_BONEDEMON);
		}
		if (!(MissileFileFlag & 0x2000)) {
			MissileFileFlag |= 0x2000;
			LoadMissileGFX(MFILE_EXBL3);
		}
		break;
	case MT_PSYCHORB:
		if (!(MissileFileFlag & 0x400)) {
			MissileFileFlag |= 0x400;
			LoadMissileGFX(MFILE_BONEDEMON);
		}
		if (!(MissileFileFlag & 0x1000)) {
			MissileFileFlag |= 0x1000;
			LoadMissileGFX(MFILE_EXBL2);
		}
		break;
	case MT_NECRMORB:
		if (!(MissileFileFlag & 0x800)) {
			MissileFileFlag |= 0x800;
			LoadMissileGFX(MFILE_NECROMORB);
			LoadMissileGFX(MFILE_EXRED3);
		}
		break;
	case MT_HORKDMN:
		if (!(MissileFileFlag & 0x4000)) {
			MissileFileFlag |= 0x4000;
			LoadMissileGFX(MFILE_SPAWNS);
		}
		break;
	case MT_LICH:
		if (!(MissileFileFlag & 0x100)) {
			MissileFileFlag |= 0x100;
			LoadMissileGFX(MFILE_LICH);
			LoadMissileGFX(MFILE_EXORA1);
		}
		break;
	case MT_ARCHLICH:
		if (!(MissileFileFlag & 0x200)) {
			MissileFileFlag |= 0x200;
			LoadMissileGFX(MFILE_ARCHLICH);
			LoadMissileGFX(MFILE_EXYEL2);
		}
		break;
#endif
	}
}

static void InitMonster(int mnum, int dir, int mtidx, int x, int y)
{
	CMonster *cmon = &Monsters[mtidx];
	MonsterStruct *mon = &monster[mnum];

	mon->_mMTidx = mtidx;
	mon->_mdir = dir;
	mon->_mx = x;
	mon->_my = y;
	mon->_mfutx = x;
	mon->_mfuty = y;
	mon->_moldx = x;
	mon->_moldy = y;
	mon->MType = cmon;
	mon->_mType = cmon->mtype;
	mon->_mAnimWidth = cmon->width;
	mon->_mAnimWidth2 = cmon->width2;
	mon->MData = cmon->MData;
	mon->mName = cmon->MData->mName;
	mon->_mFlags = cmon->MData->mFlags;
	mon->mLevel = cmon->MData->mLevel;
	mon->_mAi = cmon->MData->mAi;
	mon->_mint = cmon->MData->mInt;
	mon->mArmorClass = cmon->MData->mArmorClass;
	mon->mMinDamage = cmon->MData->mMinDamage;
	mon->mMaxDamage = cmon->MData->mMaxDamage;
	mon->mMinDamage2 = cmon->MData->mMinDamage2;
	mon->mMaxDamage2 = cmon->MData->mMaxDamage2;
	mon->mHit = cmon->MData->mHit;
	mon->mHit2 = cmon->MData->mHit2;
	mon->mMagicRes = cmon->MData->mMagicRes;
	mon->mExp = cmon->MData->mExp;
	mon->_mAnims = cmon->Anims;
	mon->_mAnimData = cmon->Anims[MA_STAND].Data[dir];
	mon->_mAnimDelay = cmon->Anims[MA_STAND].Rate;
	mon->_mAnimCnt = random_(88, mon->_mAnimDelay - 1);
	mon->_mAnimLen = cmon->Anims[MA_STAND].Frames;
	mon->_mAnimFrame = RandRange(1, mon->_mAnimLen - 1);
	mon->_mmode = MM_STAND;
	mon->_mVar1 = MM_STAND;
	mon->_mVar2 = 0;

	mon->_mmaxhp = RandRange(cmon->mMinHP, cmon->mMaxHP) << 6;

	mon->_msquelch = 0;
	mon->_mpathcount = 0;
	mon->_mWhoHit = 0;
	mon->_mgoal = MGOAL_NORMAL;
	mon->_mgoalvar1 = 0;
	mon->_mgoalvar2 = 0;
	mon->_mgoalvar3 = 0;
	mon->_mDelFlag = FALSE;
	mon->_mRndSeed = GetRndSeed();
	mon->_mAISeed = GetRndSeed();
	mon->mtalkmsg = 0;

	mon->_uniqtype = 0;
	mon->_uniqtrans = 0;
	mon->_udeadval = 0;
	mon->mlid = 0;

	mon->leader = 0;
	mon->leaderflag = MLEADER_NONE;
	mon->packsize = 0;
	mon->falign_CB = 0;

	if (gnDifficulty == DIFF_NIGHTMARE) {
		mon->_mmaxhp = 3 * mon->_mmaxhp + (100 << 6);
		mon->mLevel += 15;
		mon->mExp = 2 * (mon->mExp + 1000);
		mon->mHit += NIGHTMARE_TO_HIT_BONUS;
		mon->mMinDamage = 2 * (mon->mMinDamage + 2);
		mon->mMaxDamage = 2 * (mon->mMaxDamage + 2);
		mon->mHit2 += NIGHTMARE_TO_HIT_BONUS;
		mon->mMinDamage2 = 2 * (mon->mMinDamage2 + 2);
		mon->mMaxDamage2 = 2 * (mon->mMaxDamage2 + 2);
		mon->mArmorClass += NIGHTMARE_AC_BONUS;
	} else if (gnDifficulty == DIFF_HELL) {
		mon->_mmaxhp = 4 * mon->_mmaxhp + (200 << 6);
		mon->mLevel += 30;
		mon->mExp = 4 * (mon->mExp + 1000);
		mon->mHit += HELL_TO_HIT_BONUS;
		mon->mMinDamage = 4 * mon->mMinDamage + 6;
		mon->mMaxDamage = 4 * mon->mMaxDamage + 6;
		mon->mHit2 += HELL_TO_HIT_BONUS;
		mon->mMinDamage2 = 4 * mon->mMinDamage2 + 6;
		mon->mMaxDamage2 = 4 * mon->mMaxDamage2 + 6;
		mon->mArmorClass += HELL_AC_BONUS;
		mon->mMagicRes = cmon->MData->mMagicRes2;
	}

	if (gbMaxPlayers == 1) {
		mon->_mmaxhp >>= 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}
	mon->_mhitpoints = mon->_mmaxhp;
}

static BOOL MonstPlace(int xp, int yp)
{
	if (IN_DUNGEON_AREA(xp, yp)) {
		return (dMonster[xp][yp] | dPlayer[xp][yp] | nSolidTable[dPiece[xp][yp]]
			 | (dFlags[xp][yp] & (BFLAG_VISIBLE | BFLAG_POPULATED))) == 0;
	}
	return FALSE;
}

#ifdef HELLFIRE
void WakeUberDiablo()
{
	MonsterStruct *mon;

	if (currlevel == 24 && UberDiabloMonsterIndex >= 0) {
		mon = &monster[UberDiabloMonsterIndex];
		PlayEffect(UberDiabloMonsterIndex, 2);
		mon->mArmorClass -= 50;
		mon->mMagicRes = 0;
		mon->_mmaxhp /= 2;
		mon->_mhitpoints = mon->_mmaxhp;
	}
}
#endif

static void PlaceMonster(int mnum, int mtype, int x, int y)
{
	int dir;

	dMonster[x][y] = mnum + 1;

	dir = random_(90, 8);
	InitMonster(mnum, dir, mtype, x, y);
}

static void PlaceUniqueMonst(int uniqindex, int miniontype, int bosspacksize)
{
	int xp, yp, x, y;
	int uniqtype;
	int count2;
	char filestr[64];
	BOOL done;
	const UniqMonstStruct *uniqm;
	MonsterStruct *mon;
	int count;

	count = 0;
	uniqm = &UniqMonst[uniqindex];

	if ((uniquetrans + 19) << 8 >= LIGHTSIZE) {
		return;
	}

	for (uniqtype = 0; uniqtype < nummtypes; uniqtype++) {
		if (Monsters[uniqtype].mtype == UniqMonst[uniqindex].mtype) {
			break;
		}
	}

	xp = -1;
	switch (uniqindex) {
	case UMT_SKELKING:
		if (gbMaxPlayers == 1) {
			xp = DBORDERX + 19;
			yp = DBORDERY + 31;
		}
		break;
	case UMT_ZHAR:
		if (zharlib >= 0 && zharlib < themeCount) {
			xp = 2 * themeLoc[zharlib].x + DBORDERX + 4;
			yp = 2 * themeLoc[zharlib].y + DBORDERY + 4;
		}
		break;
	case UMT_SNOTSPIL:
		xp = 2 * setpc_x + DBORDERX + 8;
		yp = 2 * setpc_y + DBORDERY + 12;
		break;
	case UMT_LAZURUS:
		if (gbMaxPlayers == 1) {
			xp = DBORDERX + 16;
			yp = DBORDERY + 30;
		} else {
			xp = 2 * setpc_x + DBORDERX + 3;
			yp = 2 * setpc_y + DBORDERY + 6;
		}
		break;
	case UMT_RED_VEX:
		if (gbMaxPlayers == 1) {
			xp = DBORDERX + 24;
			yp = DBORDERY + 29;
		} else {
			xp = 2 * setpc_x + DBORDERX + 5;
			yp = 2 * setpc_y + DBORDERY + 3;
		}
		break;
	case UMT_BLACKJADE:
		if (gbMaxPlayers == 1) {
			xp = DBORDERX + 22;
			yp = DBORDERY + 33;
		} else {
			xp = 2 * setpc_x + DBORDERX + 5;
			yp = 2 * setpc_y + DBORDERY + 9;
		}
		break;
	case UMT_WARLORD:
		xp = 2 * setpc_x + DBORDERX + 6;
		yp = 2 * setpc_y + DBORDERY + 7;
		break;
	case UMT_BUTCHER:
		done = FALSE;
		for (yp = 0; yp < MAXDUNY && !done; yp++) {
			for (xp = 0; xp < MAXDUNX && !done; xp++) {
				done = dPiece[xp][yp] == 367;
			}
		}
		break;
#ifdef HELLFIRE
	case UMT_NAKRUL:
		if (UberRow == 0 || UberCol == 0) {
			UberDiabloMonsterIndex = -1;
			return;
		}
		xp = UberRow - 2;
		yp = UberCol;
		UberDiabloMonsterIndex = nummonsters;
		break;
#endif
	}

	if (xp == -1) {
		while (TRUE) {
			xp = random_(91, DSIZEX) + DBORDERX;
			yp = random_(91, DSIZEY) + DBORDERY;
			count2 = 0;
			for (x = xp - 3; x < xp + 3; x++) {
				for (y = yp - 3; y < yp + 3; y++) {
					if (MonstPlace(x, y)) {
						count2++;
					}
				}
			}

			if (count2 < 9) {
				count++;
				if (count < 1000) {
					continue;
				}
			}

			if (MonstPlace(xp, yp)) {
				break;
			}
		}
	}

	PlaceMonster(nummonsters, uniqtype, xp, yp);
	mon = &monster[nummonsters];
	mon->_uniqtype = uniqindex + 1;
#ifdef HELLFIRE
	if (uniqindex != UMT_HORKDMN)
#endif
		mon->mlid = AddLight(mon->_mx, mon->_my, MON_LIGHTRAD);

	mon->mLevel = 2 * uniqm->mlevel;

	mon->mExp *= 2;
	mon->mName = uniqm->mName;
	mon->_mmaxhp = uniqm->mmaxhp << 6;

	mon->_mAi = uniqm->mAi;
	mon->_mint = uniqm->mint;
	mon->mMinDamage = uniqm->mMinDamage;
	mon->mMaxDamage = uniqm->mMaxDamage;
	mon->mMinDamage2 = uniqm->mMinDamage2;
	mon->mMaxDamage2 = uniqm->mMaxDamage2;
	mon->mMagicRes = uniqm->mMagicRes;
	mon->mtalkmsg = uniqm->mtalkmsg;
	if (gbMaxPlayers != 1 && mon->_mAi == AI_LAZHELP)
		mon->mtalkmsg = 0;
	else if (mon->mtalkmsg != 0)
		mon->_mgoal = MGOAL_INQUIRING;

	snprintf(filestr, sizeof(filestr), "Monsters\\Monsters\\%s.TRN", uniqm->mTrnName);
	LoadFileWithMem(filestr, &pLightTbl[256 * (uniquetrans + 19)]);

	mon->_uniqtrans = uniquetrans++;

	if (uniqm->mUnqAttr & 4) {
		mon->mHit = uniqm->mUnqVar1;
		mon->mHit2 = uniqm->mUnqVar1;

		if (gnDifficulty == DIFF_NIGHTMARE) {
			mon->mHit += NIGHTMARE_TO_HIT_BONUS;
			mon->mHit2 += NIGHTMARE_TO_HIT_BONUS;
		} else if (gnDifficulty == DIFF_HELL) {
			mon->mHit += HELL_TO_HIT_BONUS;
			mon->mHit2 += HELL_TO_HIT_BONUS;
		}
	}
	if (uniqm->mUnqAttr & 8) {
		mon->mArmorClass = uniqm->mUnqVar1;

		if (gnDifficulty == DIFF_NIGHTMARE) {
			mon->mArmorClass += NIGHTMARE_AC_BONUS;
		} else if (gnDifficulty == DIFF_HELL) {
			mon->mArmorClass += HELL_AC_BONUS;
		}
	}

	if (gnDifficulty == DIFF_NIGHTMARE) {
		mon->_mmaxhp = 3 * mon->_mmaxhp + (100 << 6);
		mon->mLevel += 15;
		mon->mMinDamage = 2 * (mon->mMinDamage + 2);
		mon->mMaxDamage = 2 * (mon->mMaxDamage + 2);
		mon->mMinDamage2 = 2 * (mon->mMinDamage2 + 2);
		mon->mMaxDamage2 = 2 * (mon->mMaxDamage2 + 2);
	} else if (gnDifficulty == DIFF_HELL) {
		mon->_mmaxhp = 4 * mon->_mmaxhp + (200 << 6);
		mon->mLevel += 30;
		mon->mMinDamage = 4 * mon->mMinDamage + 6;
		mon->mMaxDamage = 4 * mon->mMaxDamage + 6;
		mon->mMinDamage2 = 4 * mon->mMinDamage2 + 6;
		mon->mMaxDamage2 = 4 * mon->mMaxDamage2 + 6;
	}
	if (gbMaxPlayers == 1) {
		mon->_mmaxhp = mon->_mmaxhp >> 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}
	mon->_mhitpoints = mon->_mmaxhp;

	nummonsters++;

	if (uniqm->mUnqAttr & 1) {
		PlaceGroup(miniontype, bosspacksize, uniqm->mUnqAttr, nummonsters - 1);
	}
}

static void PlaceUniques()
{
	int u, mt;

	for (u = 0; UniqMonst[u].mtype != -1; u++) {
		if (UniqMonst[u].mlevel != currlevel)
			continue;
		if (UniqMonst[u].mQuestId != Q_INVALID
		 && quests[UniqMonst[u].mQuestId]._qactive == QUEST_NOTAVAIL)
			continue;
		for (mt = 0; mt < nummtypes; mt++) {
			if (Monsters[mt].mtype == UniqMonst[u].mtype) {
				PlaceUniqueMonst(u, mt, 8);
				break;
			}
		}
	}
}

static void PlaceQuestMonsters()
{
	int skeltype;
	BYTE *setp;

	if (!setlevel) {
		if (QuestStatus(Q_BUTCHER)) {
			PlaceUniqueMonst(UMT_BUTCHER, 0, 0);
		}

		if (currlevel == quests[Q_SKELKING]._qlevel && gbMaxPlayers != 1) {
			for (skeltype = 0; skeltype < nummtypes; skeltype++) {
				if (IsSkel(Monsters[skeltype].mtype)) {
					break;
				}
			}

			PlaceUniqueMonst(UMT_SKELKING, skeltype, 30);
		}

		if (QuestStatus(Q_LTBANNER)) {
			setp = LoadFileInMem("Levels\\L1Data\\Banner1.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_BLOOD)) {
			setp = LoadFileInMem("Levels\\L2Data\\Blood2.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_BLIND)) {
			setp = LoadFileInMem("Levels\\L2Data\\Blind2.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_ANVIL)) {
			setp = LoadFileInMem("Levels\\L3Data\\Anvil.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x + 2, 2 * setpc_y + 2);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_WARLORD)) {
			setp = LoadFileInMem("Levels\\L4Data\\Warlord.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(setp);
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, PLACE_SCATTER);
		}
		if (QuestStatus(Q_VEIL)) {
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, PLACE_SCATTER);
		}
		if (QuestStatus(Q_ZHAR) && zharlib == -1) {
			quests[Q_ZHAR]._qactive = QUEST_NOTAVAIL;
		}

		if (currlevel == quests[Q_BETRAYER]._qlevel && gbMaxPlayers != 1) {
			setp = LoadFileInMem("Levels\\L4Data\\Vile1.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(setp);

			AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, PLACE_UNIQUE);
			AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, PLACE_UNIQUE);
			assert(UniqMonst[UMT_RED_VEX].mtype == UniqMonst[UMT_BLACKJADE].mtype);
			PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
			PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
			PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
		}
		if (currlevel == 16) {
			setp = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
			SetMapMonsters(setp, 2 * diabquad1x, 2 * diabquad1y);
			mem_free_dbg(setp);
			setp = LoadFileInMem("Levels\\L4Data\\diab2a.DUN", NULL);
			SetMapMonsters(setp, 2 * diabquad2x, 2 * diabquad2y);
			mem_free_dbg(setp);
			setp = LoadFileInMem("Levels\\L4Data\\diab3a.DUN", NULL);
			SetMapMonsters(setp, 2 * diabquad3x, 2 * diabquad3y);
			mem_free_dbg(setp);
			setp = LoadFileInMem("Levels\\L4Data\\diab4a.DUN", NULL);
			SetMapMonsters(setp, 2 * diabquad4x, 2 * diabquad4y);
			mem_free_dbg(setp);
#ifdef HELLFIRE
		} else if (currlevel == 24) {
			PlaceUniqueMonst(UMT_NAKRUL, 0, 0);
#endif
		}
	} else if (setlvlnum == SL_SKELKING) {
		PlaceUniqueMonst(UMT_SKELKING, 0, 0);
	} else if (setlvlnum == SL_VILEBETRAYER) {
		AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, PLACE_UNIQUE);
		AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, PLACE_UNIQUE);
		assert(UniqMonst[UMT_RED_VEX].mtype == UniqMonst[UMT_BLACKJADE].mtype);
		PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
		PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
		PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
	}
}

void PlaceGroup(int mtype, int num, int leaderf, int leader)
{
	int placed, offset, try1, try2;
	int xp, yp, x1, y1;

	if (num + nummonsters > totalmonsters) {
		num = totalmonsters - nummonsters;
	}

	placed = 0;
	for (try1 = 0; try1 < 10; try1++) {
		while (placed) {
			nummonsters--;
			placed--;
			dMonster[monster[nummonsters]._mx][monster[nummonsters]._my] = 0;
		}

		if (leaderf & 1) {
			offset = random_(92, 8);
			x1 = xp = monster[leader]._mx + offset_x[offset];
			y1 = yp = monster[leader]._my + offset_y[offset];
		} else {
			do {
				x1 = xp = random_(93, DSIZEX) + DBORDERX;
				y1 = yp = random_(93, DSIZEY) + DBORDERY;
			} while (!MonstPlace(xp, yp));
		}

		for (try2 = 0; placed < num && try2 < 100; xp += offset_x[random_(94, 8)], yp += offset_x[random_(94, 8)]) { /// BUGFIX: `yp += offset_y`
			if (!MonstPlace(xp, yp)
			    || (dTransVal[xp][yp] != dTransVal[x1][y1])
			    || (leaderf & 2) && ((abs(xp - x1) >= 4) || (abs(yp - y1) >= 4))) {
				try2++;
				continue;
			}

			PlaceMonster(nummonsters, mtype, xp, yp);
			if (leaderf & 1) {
				monster[nummonsters]._mmaxhp *= 2;
				monster[nummonsters]._mhitpoints = monster[nummonsters]._mmaxhp;
				monster[nummonsters]._mint = monster[leader]._mint;

				if (leaderf & 2) {
					monster[nummonsters].leader = leader;
					monster[nummonsters].leaderflag = MLEADER_PRESENT;
					monster[nummonsters]._mAi = monster[leader]._mAi;
				}
			}
			nummonsters++;
			placed++;
		}

		if (placed >= num) {
			break;
		}
	}

	if (leaderf & 2) {
		monster[leader].packsize = placed;
	}
}

void InitMonsters()
{
	int na, nt;
	int i, xx, yy;
	int numplacemonsters;
	int mtype;
	int numscattypes;
	int scattertypes[NUM_MTYPES];

	numscattypes = 0;
#ifdef _DEBUG
	if (gbMaxPlayers != 1)
		CheckDungeonClear();
#endif
	if (!setlevel) {
		for (i = 0; i < MAX_MINIONS; i++)
			AddMonster(0, 0, 0, 0, FALSE);
	}
	nt = numtrigs;
	if (currlevel == 15)
		nt = 1;
	for (i = 0; i < nt; i++) {
		for (xx = -2; xx < 2; xx++) {
			for (yy = -2; yy < 2; yy++)
				DoVision(xx + trigs[i]._tx, yy + trigs[i]._ty, 15, FALSE, FALSE);
		}
	}
	PlaceQuestMonsters();
	if (!setlevel) {
		PlaceUniques();

		na = 0;
		for (xx = DBORDERX; xx < DSIZEX + DBORDERX; xx++)
			for (yy = DBORDERY; yy < DSIZEY + DBORDERY; yy++)
				if (!nSolidTable[dPiece[xx][yy]])
					na++;
		numplacemonsters = na / 30;
		if (gbMaxPlayers != 1)
			numplacemonsters += numplacemonsters >> 1;
		if (numplacemonsters > MAXMONSTERS - (MAX_MINIONS + 6) - nummonsters)
			numplacemonsters = MAXMONSTERS - (MAX_MINIONS + 6) - nummonsters;
		totalmonsters = nummonsters + numplacemonsters;
		for (i = 0; i < nummtypes; i++) {
			if (Monsters[i].mPlaceFlags & PLACE_SCATTER) {
				scattertypes[numscattypes] = i;
				numscattypes++;
			}
		}
		i = currlevel;
		while (nummonsters < totalmonsters) {
			mtype = scattertypes[random_(95, numscattypes)];
			if (i == 1 || random_(95, 2) == 0)
				na = 1;
#ifdef HELLFIRE
			else if (i == 2 || i >= 21 && i <= 24)
#else
			else if (i == 2)
#endif
				na = RandRange(2, 3);
			else
				na = RandRange(3, 5);
			PlaceGroup(mtype, na, 0, 0);
		}
	}
	for (i = 0; i < nt; i++) {
		for (xx = -2; xx < 2; xx++) {
			for (yy = -2; yy < 2; yy++)
				DoUnVision(xx + trigs[i]._tx, yy + trigs[i]._ty, 15);
		}
	}
}

void SetMapMonsters(BYTE *pMap, int startx, int starty)
{
	WORD rw, rh;
	WORD *lm;
	int i, j;
	int mtype;

	if (setlevel) {
		AddMonsterType(MT_GOLEM, PLACE_SPECIAL);
		for (i = 0; i < MAX_MINIONS; i++)
			AddMonster(0, 0, 0, 0, FALSE);
	}
	lm = (WORD *)pMap;
	rw = SDL_SwapLE16(*lm);
	lm++;
	rh = SDL_SwapLE16(*lm);
	lm++;
	lm += rw * rh;
	rw = rw << 1;
	rh = rh << 1;
	lm += rw * rh;

	startx += DBORDERX;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				mtype = AddMonsterType(MonstConvTbl[SDL_SwapLE16(*lm) - 1], PLACE_SPECIAL);
				PlaceMonster(nummonsters++, mtype, i, j);
			}
			lm++;
		}
	}
}

static void DeleteMonster(int i)
{
	int temp;

	nummonsters--;
	temp = monstactive[nummonsters];
	monstactive[nummonsters] = monstactive[i];
	monstactive[i] = temp;
}

int AddMonster(int x, int y, int dir, int mtype, BOOL InMap)
{
	if (nummonsters < MAXMONSTERS) {
		int mnum = monstactive[nummonsters++];
		if (InMap)
			dMonster[x][y] = mnum + 1;
		InitMonster(mnum, dir, mtype, x, y);
		return mnum;
	}

	return -1;
}

static void NewMonsterAnim(int mnum, int anim, int md)
{
	MonsterStruct *mon = &monster[mnum];
	AnimStruct* as = &mon->_mAnims[anim];

	mon->_mdir = md;
	mon->_mAnimData = as->Data[md];
	mon->_mAnimLen = as->Frames;
	mon->_mAnimCnt = 0;
	mon->_mAnimFrame = 1;
	mon->_mAnimDelay = as->Rate;
	mon->_mFlags &= ~(MFLAG_REV_ANIMATION | MFLAG_LOCK_ANIMATION);
}

static BOOL MonRanged(int mnum)
{
	char ai = monster[mnum]._mAi;
	return ai == AI_SKELBOW || ai == AI_GOATBOW || ai == AI_SUCC || ai == AI_LAZHELP;
}

BOOL MonTalker(int mnum)
{
	return monster[mnum].mtalkmsg != 0;
	/*char ai = monster[mnum]._mAi;
	return ai == AI_LAZURUS
	    || ai == AI_WARLORD
	    || ai == AI_GARBUD
	    || ai == AI_ZHAR
	    || ai == AI_SNOTSPIL
	    || ai == AI_LACHDAN
	    || ai == AI_LAZHELP;*/
}

static void MonEnemy(int mnum)
{
	int i, tnum;
	int enemy, dist, best_dist;
	BOOL sameroom, bestsameroom;
	MonsterStruct *mon = &monster[mnum], *tmon;
	const char tv = dTransVal[mon->_mx][mon->_my];

	enemy = 0;
	best_dist = MAXDUNX + MAXDUNY;
	bestsameroom = FALSE;
	if (mnum >= MAX_MINIONS) {
		for (i = 0; i < MAX_PLRS; i++) {
			if (!plr[i].plractive || currlevel != plr[i].plrlevel || plr[i]._pLvlChanging || plr[i]._pHitPoints < (1 << 6))
				continue;
			sameroom = tv == dTransVal[plr[i]._px][plr[i]._py];
			dist = std::max(abs(mon->_mx - plr[i]._px), abs(mon->_my - plr[i]._py));
			if (sameroom == bestsameroom) {
				if (dist >= best_dist)
					continue;
			} else if (!sameroom)
				continue;
			enemy = i + 1;
			best_dist = dist;
			bestsameroom = sameroom;
		}
		for (i = 0; i < MAX_MINIONS; i++) {
			tmon = &monster[i];
#ifdef HELLFIRE
			if (tmon->_mhitpoints < (1 << 6))
				continue;
#endif
			if (MINION_INACTIVE(tmon))
				continue;
			dist = std::max(abs(mon->_mx - tmon->_mx), abs(mon->_my - tmon->_my));
			if (dist >= 2 && !MonRanged(mnum)) {
				continue;
			}
			sameroom = tv == dTransVal[tmon->_mx][tmon->_my];
			if (sameroom == bestsameroom) {
				if (dist >= best_dist)
					continue;
			} else if (!sameroom)
				continue;
			enemy = -(i + 1);
			best_dist = dist;
			bestsameroom = sameroom;
		}
	} else {
		for (i = 0; i < nummonsters; i++) {
			tnum = monstactive[i];
			if (tnum == mnum)
				continue;
			tmon = &monster[tnum];
#ifdef HELLFIRE
			if (tmon->_mhitpoints < (1 << 6))
				continue;
#endif
			if (MINION_INACTIVE(tmon))
				continue;
			if (MonTalker(tnum))
				continue;
			dist = std::max(abs(mon->_mx - tmon->_mx), abs(mon->_my - tmon->_my));
			sameroom = tv == dTransVal[tmon->_mx][tmon->_my];
			if (sameroom == bestsameroom) {
				if (dist >= best_dist)
					continue;
			} else if (!sameroom)
				continue;
			enemy = -(tnum + 1);
			best_dist = dist;
			bestsameroom = sameroom;
		}
	}

	if (enemy != 0) {
		if (enemy > 0) {
			enemy--;
			mon->_mFlags &= ~MFLAG_TARGETS_MONSTER;
			mon->_menemyx = plr[enemy]._pfutx;
			mon->_menemyy = plr[enemy]._pfuty;
		} else {
			enemy = -(enemy + 1);
			mon->_mFlags |= MFLAG_TARGETS_MONSTER;
			mon->_menemyx = monster[enemy]._mfutx;
			mon->_menemyy = monster[enemy]._mfuty;
		}
		mon->_mFlags &= ~MFLAG_NO_ENEMY;
		mon->_menemy = enemy;
	} else {
		mon->_mFlags |= MFLAG_NO_ENEMY;
	}
}

static int MonGetDir(int mnum)
{
	return GetDirection(monster[mnum]._mx, monster[mnum]._my, monster[mnum]._menemyx, monster[mnum]._menemyy);
}

void MonStartStand(int mnum, int md)
{
	MonsterStruct *mon;

	mon = &monster[mnum];
	NewMonsterAnim(mnum, mon->_mType == MT_GOLEM ? MA_WALK : MA_STAND, md);
	mon->_mVar1 = mon->_mmode; // preserve the previous mode of the monster
	mon->_mVar2 = 0;           // the time spent on standing
	mon->_mmode = MM_STAND;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
	MonEnemy(mnum);
}

static void MonStartDelay(int mnum, int len)
{
	MonsterStruct *mon;

	if (len <= 0) {
		return;
	}

	mon = &monster[mnum];
	if (mon->_mAi != AI_LAZURUS) {
		mon->_mVar2 = len;      // length of the delay
		mon->_mmode = MM_DELAY;
	}
}

static void MonStartSpStand(int mnum, int md)
{
	MonsterStruct *mon;

	NewMonsterAnim(mnum, MA_SPECIAL, md);
	mon = &monster[mnum];
	mon->_mmode = MM_SPSTAND;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

static void MonChangeLightOff(int mnum)
{
	MonsterStruct *mon;
	int lx, ly;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonChangeLightOff: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	lx = mon->_mxoff + 2 * mon->_myoff;
	ly = 2 * mon->_myoff - mon->_mxoff;

	lx = lx / 8;
	ly = ly / 8;

	CondChangeLightOff(mon->mlid, lx, ly);
}

static void MonStartWalk(int mnum, int xvel, int yvel, int xadd, int yadd, int EndDir)
{
	MonsterStruct *mon = &monster[mnum];
	int mx, my;

	mx = mon->_mx;
	my = mon->_my;
	mon->_moldx = mx;
	mon->_moldy = my;

	mx += xadd;
	my += yadd;
	dMonster[mx][my] = -(mnum + 1);
	mon->_mfutx = mx;
	mon->_mfuty = my;
	mon->_mmode = MM_WALK;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mVar1 = xadd; // dx after the movement
	mon->_mVar2 = yadd; // dy after the movement
	NewMonsterAnim(mnum, MA_WALK, EndDir);
	mon->_mVar6 = 0;    // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar7 = 0;    // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar8 = 0;    // Value used to measure progress for moving from one tile to another
}

static void MonStartWalk2(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir)
{
	MonsterStruct *mon = &monster[mnum];
	int mx, my;

	mx = mon->_mx;
	my = mon->_my;
	dMonster[mx][my] = -(mnum + 1);
	mon->_mVar1 = mon->_moldx = mx; // the starting x-coordinate of the monster
	mon->_mVar2 = mon->_moldy = my; // the starting y-coordinate of the monster

	mx += xadd;
	my += yadd;
	mon->_mx = mon->_mfutx = mx;
	mon->_my = mon->_mfuty = my;
	dMonster[mx][my] = mnum + 1;
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN)) {
		ChangeLightXY(mon->mlid, mx, my);
		MonChangeLightOff(mnum);
	}
	mon->_mmode = MM_WALK2;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	NewMonsterAnim(mnum, MA_WALK, EndDir);
	mon->_mVar6 = 16 * xoff; // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar7 = 16 * yoff; // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar8 = 0;         // Value used to measure progress for moving from one tile to another
}

static void MonStartWalk3(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir)
{
	MonsterStruct *mon = &monster[mnum];
	int fx = xadd + mon->_mx;
	int fy = yadd + mon->_my;
	mapx += mon->_mx;
	mapy += mon->_my;

	dMonster[mon->_mx][mon->_my] = -(mnum + 1);
	dMonster[fx][fy] = -(mnum + 1);
	mon->_mVar4 = mapx; // Used for storing X-position of a tile which should have its BFLAG_MONSTLR flag removed after walking. When starting to walk the game places the monster in the dMonster array -1 in the Y coordinate, and uses BFLAG_MONSTLR to check if it should be using -1 to the Y coordinate when rendering the monster
	mon->_mVar5 = mapy; // Used for storing Y-position of a tile which should have its BFLAG_MONSTLR flag removed after walking. When starting to walk the game places the monster in the dMonster array -1 in the Y coordinate, and uses BFLAG_MONSTLR to check if it should be using -1 to the Y coordinate when rendering the monster
	dFlags[mapx][mapy] |= BFLAG_MONSTLR;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mfutx = fx;
	mon->_mfuty = fy;
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN)) {
		//ChangeLightXY(mon->mlid, mon->_mVar4, mon->_mVar5);
		MonChangeLightOff(mnum);
	}
	mon->_mmode = MM_WALK3;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mVar1 = fx;        // the Monster's x-coordinate after the movement
	mon->_mVar2 = fy;        // the Monster's y-coordinate after the movement
	NewMonsterAnim(mnum, MA_WALK, EndDir);
	mon->_mVar6 = 16 * xoff; // Used as _mxoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar7 = 16 * yoff; // Used as _myoff but with a higher range so that we can correctly apply velocities of a smaller number
	mon->_mVar8 = 0;         // Value used to measure progress for moving from one tile to another
}

static void MonStartAttack(int mnum)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon;

	NewMonsterAnim(mnum, MA_ATTACK, md);
	mon = &monster[mnum];
	mon->_mmode = MM_ATTACK;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

static void MonStartRAttack(int mnum, int mitype)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon;

	NewMonsterAnim(mnum, MA_ATTACK, md);
	mon = &monster[mnum];
	mon->_mmode = MM_RATTACK;
	mon->_mVar1 = mitype;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

static void MonStartRSpAttack(int mnum, int mitype)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon;

	NewMonsterAnim(mnum, MA_SPECIAL, md);
	mon = &monster[mnum];
	mon->_mmode = MM_RSPATTACK;
	mon->_mVar1 = mitype;
	mon->_mVar2 = 0;      // counter to enable/disable MFLAG_LOCK_ANIMATION for certain monsters
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

static void MonStartSpAttack(int mnum)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon;

	NewMonsterAnim(mnum, MA_SPECIAL, md);
	mon = &monster[mnum];
	mon->_mmode = MM_SATTACK;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

static void MonStartEat(int mnum)
{
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);

	mon->_mmode = MM_SATTACK;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

void MonClearSquares(int mnum)
{
	int x, y, mx, my, m1, m2;

	mx = monster[mnum]._moldx;
	my = monster[mnum]._moldy;
	m1 = -1 - mnum;
	m2 = mnum + 1;

#ifdef HELLFIRE
	for (y = my - 1; y <= my + 1; y++) {
		for (x = mx - 1; x <= mx + 1; x++) {
			if (dMonster[x][y] == m1 || dMonster[x][y] == m2)
				dMonster[x][y] = 0;
		}
	}

	dFlags[mx + 1][my] &= ~BFLAG_MONSTLR;
	dFlags[mx][my + 1] &= ~BFLAG_MONSTLR;
#else
	for (y = my - 1; y <= my + 1; y++) {
		if (y >= 0 && y < MAXDUNY) {
			for (x = mx - 1; x <= mx + 1; x++) {
				if (x >= 0 && x < MAXDUNX && (dMonster[x][y] == m1 || dMonster[x][y] == m2))
					dMonster[x][y] = 0;
			}
		}
	}

	if (mx + 1 < MAXDUNX)
		dFlags[mx + 1][my] &= ~BFLAG_MONSTLR;
	if (my + 1 < MAXDUNY)
		dFlags[mx][my + 1] &= ~BFLAG_MONSTLR;
#endif
}

static void MonStartGetHit(int mnum)
{
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, MA_GOTHIT, mon->_mdir);

	mon->_mmode = MM_GOTHIT;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mx = mon->_mfutx = mon->_moldx;
	mon->_my = mon->_mfuty = mon->_moldy;
	if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
		ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
	MonClearSquares(mnum);
	dMonster[mon->_mx][mon->_my] = mnum + 1;
}

void MonGetKnockback(int mnum)
{
	MonsterStruct *mon = &monster[mnum];
	int dir = (mon->_mdir - 4) & 7;

	if (DirOK(mnum, dir)) {
		MonClearSquares(mnum);
		mon->_moldx += offset_x[dir];
		mon->_moldy += offset_y[dir];
		MonStartGetHit(mnum);
	}
}

void MonStartHit(int mnum, int pnum, int dam)
{
	MonsterStruct *mon = &monster[mnum];

	if (pnum == myplr) {
		delta_monster_hp(mnum, mon->_mhitpoints, currlevel);
		NetSendCmdDwParam2(FALSE, CMD_MONSTDAMAGE, mnum, dam);
	}
	PlayEffect(mnum, 1);
	if (mnum < MAX_MINIONS)
		return;
	if (pnum >= 0)
		mon->_mWhoHit |= 1 << pnum;
	if ((mon->_mType >= MT_SNEAK && mon->_mType <= MT_ILLWEAV) || (dam >> 6) >= (mon->mLevel + 3)) {
		if (pnum >= 0) {
			mon->_mFlags &= ~MFLAG_TARGETS_MONSTER;
			mon->_menemy = pnum;
			mon->_menemyx = plr[pnum]._pfutx;
			mon->_menemyy = plr[pnum]._pfuty;
			if (mon->_mmode != MM_STONE)
				mon->_mdir = MonGetDir(mnum);
		}
		if (mon->_mType == MT_BLINK) {
			MonTeleport(mnum);
		} else if ((mon->_mType >= MT_NSCAV && mon->_mType <= MT_YSCAV)
#ifdef HELLFIRE
				 || mon->_mType == MT_GRAVEDIG
#endif
			) {
			mon->_mgoal = MGOAL_NORMAL;
#ifdef HELLFIRE
			mon->_mgoalvar1 = 0;
			mon->_mgoalvar2 = 0;
#endif
		}
		if (mon->_mmode != MM_STONE) {
			MonStartGetHit(mnum);
		}
	}
}

static void MonDiabloDeath(int mnum, BOOL sendmsg)
{
	MonsterStruct *mon, *pmonster;
	int i, j;
	int _moldx, _moldy;

	mon = &monster[mnum];
	quests[Q_DIABLO]._qactive = QUEST_DONE;
	if (sendmsg)
		NetSendCmdQuest(TRUE, Q_DIABLO);
	sgbSaveSoundOn = gbSoundOn;
	gbProcessPlayers = FALSE;
#ifdef HELLFIRE
	gbSoundOn = FALSE;
#endif
	for (i = 0; i < nummonsters; i++) {
		j = monstactive[i];
		if (j == mnum || mon->_msquelch == 0)
			continue;

		pmonster = &monster[j];
		NewMonsterAnim(j, MA_DEATH, pmonster->_mdir);
		pmonster->_mxoff = 0;
		pmonster->_myoff = 0;
		pmonster->_mmode = MM_DEATH;
		MonClearSquares(j);
		_moldx = pmonster->_moldx;
		_moldy = pmonster->_moldy;
		pmonster->_my = _moldy;
		pmonster->_mfuty = _moldy;
		pmonster->_mx = _moldx;
		pmonster->_mfutx = _moldx;
		dMonster[_moldx][_moldy] = j + 1;
	}
	mon->_mVar1 = 0;

	PlaySfxLoc(USFX_DIABLOD, mon->_mx, mon->_my);
	AddLight(mon->_mx, mon->_my, 8);
	DoVision(mon->_mx, mon->_my, 8, FALSE, TRUE);
}

static void SpawnLoot(int mnum, BOOL sendmsg)
{
	MonsterStruct *mon;

	mon = &monster[mnum];
	SetRndSeed(mon->_mRndSeed);
	switch (mon->_uniqtype - 1) {
	case UMT_GARBUD:
		if (QuestStatus(Q_GARBUD)) {
			CreateTypeItem(mon->_mx + 1, mon->_my + 1, TRUE, ITYPE_MACE, IMISC_NONE, TRUE, FALSE);
			return;
		}
		break;
#ifdef HELLFIRE
	case UMT_HORKDMN:
		if (UseTheoQuest) {
			SpawnRewardItem(IDI_THEODORE, mon->_mx, mon->_my);
		} else {
			CreateAmulet(mon->_mx, mon->_my);
		}
		return;
	case UMT_DEFILER:
		if (effect_is_playing(USFX_DEFILER8))
			stream_stop();
		quests[Q_DEFILER]._qlog = FALSE;
		SpawnRewardItem(IDI_MAPOFDOOM, mon->_mx, mon->_my);
		return;
	case UMT_NAKRUL:
		stream_stop();
		quests[Q_NAKRUL]._qlog = FALSE; // TODO: instead of _qlog the _qactive should be set to QUEST_DONE?
		UberDiabloMonsterIndex = -2;
		CreateMagicItem(ITYPE_SWORD, ICURS_GREAT_SWORD, mon->_mx, mon->_my, TRUE, FALSE);
		CreateMagicItem(ITYPE_STAFF, ICURS_WAR_STAFF, mon->_mx, mon->_my, TRUE, FALSE);
		CreateMagicItem(ITYPE_BOW, ICURS_LONG_WAR_BOW, mon->_mx, mon->_my, TRUE, FALSE);
		return;
#endif
	}
	SpawnItem(mnum, mon->_mx, mon->_my, sendmsg);
}

static void M2MStartHit(int defm, int offm, int dam)
{
	MonsterStruct *dmon;
	if ((DWORD)defm >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by monster", defm);
	}
	dmon = &monster[defm];
	if ((DWORD)offm < MAX_MINIONS) {
		static_assert(MAX_MINIONS == MAX_PLRS, "M2MStartHit requires that owner of a monster has the same id as the monster itself.");
		dmon->_mWhoHit |= 1 << offm;
	}

	delta_monster_hp(defm, dmon->_mhitpoints, currlevel);
	NetSendCmdDwParam2(FALSE, CMD_MONSTDAMAGE, defm, dam);
	PlayEffect(defm, 1);

	if (dmon->_mType >= MT_SNEAK && dmon->_mType <= MT_ILLWEAV || dam >> 6 >= dmon->mLevel + 3) {
		if (dmon->_mType == MT_BLINK) {
			MonTeleport(defm);
		} else if ((dmon->_mType >= MT_NSCAV && dmon->_mType <= MT_YSCAV)
#ifdef HELLFIRE
				 || dmon->_mType == MT_GRAVEDIG
#endif
		) {
			dmon->_mgoal = MGOAL_NORMAL;
#ifdef HELLFIRE
			dmon->_mgoalvar1 = 0;
			dmon->_mgoalvar2 = 0;
#endif
		}

		if (dmon->_mmode != MM_STONE && dmon->_mType != MT_GOLEM) {
			if (offm >= 0)
				dmon->_mdir = (monster[offm]._mdir - 4) & 7;
			MonStartGetHit(defm);
		}
	}
}

static void MonstStartKill(int mnum, int pnum, BOOL sendmsg)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonstStartKill: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mhitpoints = 0;

	if (mnum >= MAX_MINIONS) {
		if ((DWORD)pnum < MAX_PLRS)
			mon->_mWhoHit |= 1 << pnum;
		AddPlrMonstExper(mon->mLevel, mon->mExp, mon->_mWhoHit);
		SpawnLoot(mnum, sendmsg);
	}

	if (mon->_mType == MT_DIABLO)
		MonDiabloDeath(mnum, TRUE);
	else
		PlayEffect(mnum, 2);

	if (mon->_mmode != MM_STONE) {
		mon->_mmode = MM_DEATH;
		if (pnum >= 0)
			mon->_mdir = MonGetDir(mnum);
		NewMonsterAnim(mnum, MA_DEATH, mon->_mdir);
	}
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mx = mon->_mfutx = mon->_moldx;
	mon->_my = mon->_mfuty = mon->_moldy;
	MonClearSquares(mnum);
	dMonster[mon->_mx][mon->_my] = mnum + 1;
	CheckQuestKill(mnum, sendmsg);
	MonFallenFear(mon->_mx, mon->_my);
#ifdef HELLFIRE
	if (mon->_mType >= MT_NACID && mon->_mType <= MT_XACID || mon->_mType == MT_SPIDLORD)
#else
	if (mon->_mType >= MT_NACID && mon->_mType <= MT_XACID)
#endif
		AddMissile(mon->_mx, mon->_my, 0, 0, 0, MIS_ACIDPUD, 1, mnum, mon->_mint + 1, mon->_mint + 1, 0);
}

static void M2MStartKill(int offm, int defm)
{
	MonsterStruct *dmon;
	int md;

	if ((DWORD)offm >= MAXMONSTERS) {
		dev_fatal("M2MStartKill: Invalid monster (attacker) %d", offm);
	}
	if ((DWORD)defm >= MAXMONSTERS) {
		dev_fatal("M2MStartKill: Invalid monster (killed) %d", defm);
	}
	dmon = &monster[defm];

	delta_kill_monster(defm, dmon->_mx, dmon->_my, currlevel);
	NetSendCmdLocParam1(FALSE, CMD_MONSTDEATH, dmon->_mx, dmon->_my, defm);

	dmon->_mhitpoints = 0;

	if (defm >= MAX_PLRS) {
		if (offm < MAX_MINIONS) {
			static_assert(MAX_MINIONS == MAX_PLRS, "M2MStartKill requires that owner of a monster has the same id as the monster itself.");
			dmon->_mWhoHit |= 1 << offm;
		}
		AddPlrMonstExper(dmon->mLevel, dmon->mExp, dmon->_mWhoHit);
		SpawnLoot(defm, TRUE);
	}

	if (dmon->_mType == MT_DIABLO)
		MonDiabloDeath(defm, TRUE);
	else
		PlayEffect(defm, 2);

	if (dmon->_mmode != MM_STONE) {
		dmon->_mmode = MM_DEATH;
		md = (monster[offm]._mdir - 4) & 7;
		if (dmon->_mType == MT_GOLEM)
			md = 0;
		NewMonsterAnim(defm, MA_DEATH, md);
	}
	dmon->_mxoff = 0;
	dmon->_myoff = 0;
	dmon->_mx = dmon->_mfutx = dmon->_moldx;
	dmon->_my = dmon->_mfuty = dmon->_moldy;
	MonClearSquares(defm);
	dMonster[dmon->_mx][dmon->_my] = defm + 1;
	CheckQuestKill(defm, TRUE);
	MonFallenFear(dmon->_mx, dmon->_my);
	if ((dmon->_mType >= MT_NACID && dmon->_mType <= MT_XACID)
#ifdef HELLFIRE
	 || dmon->_mType == MT_SPIDLORD
#endif
	)
		AddMissile(dmon->_mx, dmon->_my, 0, 0, 0, MIS_ACIDPUD, 1, defm, dmon->_mint + 1, dmon->_mint + 1, 0);

#ifdef HELLFIRE
	MonStartStand(offm, monster[offm]._mdir);
#endif
}

void MonStartKill(int mnum, int pnum)
{
	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartKill: Invalid monster %d", mnum);
	}
	if (myplr == pnum) {
		delta_kill_monster(mnum, monster[mnum]._mx, monster[mnum]._my, currlevel);
		if (mnum != pnum) {
			NetSendCmdLocParam1(FALSE, CMD_MONSTDEATH, monster[mnum]._mx, monster[mnum]._my, mnum);
		} else {
			NetSendCmdLocParam1(FALSE, CMD_KILLGOLEM, monster[mnum]._mx, monster[mnum]._my, currlevel);
		}
	}

	MonstStartKill(mnum, pnum, TRUE);
}

void MonSyncStartKill(int mnum, int x, int y, int pnum)
{
	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonSyncStartKill: Invalid monster %d", mnum);
	}
	if (monster[mnum]._mhitpoints == 0 || monster[mnum]._mmode == MM_DEATH) {
		return;
	}

	if (dMonster[x][y] == 0) {
		MonClearSquares(mnum);
		monster[mnum]._mx = x;
		monster[mnum]._my = y;
		monster[mnum]._moldx = x;
		monster[mnum]._moldy = y;
	}

	MonstStartKill(mnum, pnum, FALSE);
}

static void MonStartFadein(int mnum, int md, BOOL backwards)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartFadein: Invalid monster %d", mnum);
	}

	NewMonsterAnim(mnum, MA_SPECIAL, md);

	mon = &monster[mnum];
	mon->_mmode = MM_FADEIN;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
	mon->_mFlags &= ~MFLAG_HIDDEN;
	if (backwards) {
		mon->_mFlags |= MFLAG_REV_ANIMATION;
		mon->_mAnimFrame = mon->_mAnimLen;
		mon->_mVar8 = 1;				// the target frame to end the fade mode
	} else {
		mon->_mVar8 = mon->_mAnimLen;	// the target frame to end the fade mode
	}
}

static void MonStartFadeout(int mnum, int md, BOOL backwards)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartFadeout: Invalid monster %d", mnum);
	}
	NewMonsterAnim(mnum, MA_SPECIAL, md);

	mon = &monster[mnum];
	mon->_mmode = MM_FADEOUT;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
	if (backwards) {
		mon->_mFlags |= MFLAG_REV_ANIMATION;
		mon->_mAnimFrame = mon->_mAnimLen;
		mon->_mVar8 = 1;				// the target frame to end the fade mode
	} else {
		mon->_mVar8 = mon->_mAnimLen;	// the target frame to end the fade mode
	}
}

static void MonStartHeal(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartHeal: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mAnimData = mon->_mAnims[MA_SPECIAL].Data[mon->_mdir];
	mon->_mAnimFrame = mon->_mAnims[MA_SPECIAL].Frames;
	mon->_mFlags |= MFLAG_REV_ANIMATION;
	mon->_mmode = MM_HEAL;
	mon->_mVar1 = mon->_mmaxhp / (16 * RandRange(4, 8)); // the healing speed of the monster
}

static BOOL MonDoStand(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoStand: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mAnimData = mon->_mAnims[mon->_mType != MT_GOLEM ? MA_STAND : MA_WALK].Data[mon->_mdir];

	if (mon->_mAnimFrame == mon->_mAnimLen)
		MonEnemy(mnum);

	mon->_mVar2++;

	return FALSE;
}

static BOOL MonDoWalk(int mnum)
{
	MonsterStruct *mon;
	BOOL rv;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoWalk: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 == mon->_mAnims[MA_WALK].Frames) {
		dMonster[mon->_mx][mon->_my] = 0;
		mon->_mx += mon->_mVar1;
		mon->_my += mon->_mVar2;
		dMonster[mon->_mx][mon->_my] = mnum + 1;
		if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
			ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = TRUE;
	} else {
		if (mon->_mAnimCnt == 0) {
#ifdef HELLFIRE
			if (mon->_mVar8 == 0 && mon->_mType == MT_FLESTHNG)
				PlayEffect(mnum, 3);
#endif
			mon->_mVar8++;
			mon->_mVar6 += mon->_mxvel;
			mon->_mVar7 += mon->_myvel;
			mon->_mxoff = mon->_mVar6 >> 4;
			mon->_myoff = mon->_mVar7 >> 4;
			if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
				MonChangeLightOff(mnum);
		}
		rv = FALSE;
	}

	return rv;
}

static BOOL MonDoWalk2(int mnum)
{
	MonsterStruct *mon;
	BOOL rv;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoWalk2: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 == mon->_mAnims[MA_WALK].Frames) {
		dMonster[mon->_mVar1][mon->_mVar2] = 0;
		if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
			ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = TRUE;
	} else {
		if (mon->_mAnimCnt == 0) {
#ifdef HELLFIRE
			if (mon->_mVar8 == 0 && mon->_mType == MT_FLESTHNG)
				PlayEffect(mnum, 3);
#endif
			mon->_mVar8++;
			mon->_mVar6 += mon->_mxvel;
			mon->_mVar7 += mon->_myvel;
			mon->_mxoff = mon->_mVar6 >> 4;
			mon->_myoff = mon->_mVar7 >> 4;
			if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
				MonChangeLightOff(mnum);
		}
		rv = FALSE;
	}

	return rv;
}

static BOOL MonDoWalk3(int mnum)
{
	MonsterStruct *mon;
	BOOL rv;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoWalk3: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 == mon->_mAnims[MA_WALK].Frames) {
		dMonster[mon->_mx][mon->_my] = 0;
		mon->_mx = mon->_mVar1;
		mon->_my = mon->_mVar2;
		dFlags[mon->_mVar4][mon->_mVar5] &= ~BFLAG_MONSTLR;
		dMonster[mon->_mx][mon->_my] = mnum + 1;
		if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
			ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = TRUE;
	} else {
		if (mon->_mAnimCnt == 0) {
#ifdef HELLFIRE
			if (mon->_mVar8 == 0 && mon->_mType == MT_FLESTHNG)
				PlayEffect(mnum, 3);
#endif
			mon->_mVar8++;
			mon->_mVar6 += mon->_mxvel;
			mon->_mVar7 += mon->_myvel;
			mon->_mxoff = mon->_mVar6 >> 4;
			mon->_myoff = mon->_mVar7 >> 4;
			if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
				MonChangeLightOff(mnum);
		}
		rv = FALSE;
	}

	return rv;
}

void MonTryM2MHit(int offm, int defm, int hper, int mind, int maxd)
{
	MonsterStruct *dmon;
	BOOL ret;

	if ((DWORD)defm >= MAXMONSTERS) {
		dev_fatal("MonTryM2MHit: Invalid monster %d", defm);
	}
	if (CheckMonsterHit(defm, &ret))
		return;

	dmon = &monster[defm];
	int hit = dmon->_mmode == MM_STONE ? 0 : random_(4, 100);
	if (hit < hper) {
		int dam = RandRange(mind, maxd) << 6;
		dmon->_mhitpoints -= dam;
		if (dmon->_mhitpoints < (1 << 6)) {
			M2MStartKill(offm, defm);
		} else {
			M2MStartHit(defm, offm, dam);
		}
	}
}

static void MonTryH2HHit(int mnum, int pnum, int Hit, int MinDam, int MaxDam)
{
	MonsterStruct *mon;
	PlayerStruct *p;
	int tmp, dam, hper, blkper;
	int newx, newy;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonTryH2HHit: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];

	if (mon->_mFlags & MFLAG_TARGETS_MONSTER) {
		MonTryM2MHit(mnum, pnum, Hit, MinDam, MaxDam);
		return;
	}
	p = &plr[pnum];
	if (p->_pInvincible || p->_pSpellFlags & PSE_ETHERALIZED)
		return;
	if (abs(mon->_mx - p->_px) >= 2 || abs(mon->_my - p->_py) >= 2)
		return;

	tmp = p->_pIAC;
	hper = 30 + Hit
		+ (mon->mLevel << 1)
		- (p->_pLevel << 1)
		- tmp;
	tmp = 15;
	if (currlevel == 14)
		tmp = 20;
	else if (currlevel == 15)
		tmp = 25;
	else if (currlevel == 16)
		tmp = 30;
	if (hper < tmp)
		hper = tmp;
	if (random_(98, 100) >= hper)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return;

	if (p->_pBlockFlag
	 && (p->_pmode == PM_STAND || p->_pmode == PM_BLOCK)) {
		blkper = p->_pDexterity + p->_pBaseToBlk
			+ (p->_pLevel << 1)
			- (mon->mLevel << 1);
		if (blkper >= 100 || blkper > random_(98, 100)) {
			PlrStartBlock(pnum, GetDirection(p->_px, p->_py, mon->_mx, mon->_my));
			return;
		}
	}
	if (mon->_mType == MT_YZOMBIE && pnum == myplr) {
#ifdef HELLFIRE
		if (p->_pMaxHP > 64) {
#else
		if (p->_pMaxHP > 64 && p->_pMaxHPBase > 64) {
#endif
			tmp = p->_pMaxHP - 64;
			p->_pMaxHP = tmp;
			if (p->_pHitPoints > tmp) {
				p->_pHitPoints = tmp;
			}
			tmp = p->_pMaxHPBase - 64;
			p->_pMaxHPBase = tmp;
			if (p->_pHPBase > tmp) {
				p->_pHPBase = tmp;
			}
		}
	}
	if (p->_pIFlags & ISPL_THORNS) {
		tmp = RandRange(1, 3) << 6;
		mon->_mhitpoints -= tmp;
		if (mon->_mhitpoints < (1 << 6))
			MonStartKill(mnum, pnum);
		else
			MonStartHit(mnum, pnum, tmp);
	}
	dam = RandRange(MinDam, MaxDam);
	dam += p->_pIGetHit;
	if (dam <= 0)
		dam = 1;
	dam <<= 6;
	if (mon->_mFlags & MFLAG_LIFESTEAL)
		mon->_mhitpoints += dam;
	if (pnum == myplr) {
		if (PlrDecHp(pnum, dam, 0)) {
#ifdef HELLFIRE
			MonStartStand(mnum, mon->_mdir);
#endif
			return;
		}
	}
	StartPlrHit(pnum, dam, FALSE);
	if (mon->_mFlags & MFLAG_KNOCKBACK) {
		if (p->_pmode != PM_GOTHIT)
			StartPlrHit(pnum, 0, TRUE);
		newx = p->_px + offset_x[mon->_mdir];
		newy = p->_py + offset_y[mon->_mdir];
		if (PosOkPlayer(pnum, newx, newy)) {
			p->_px = newx;
			p->_py = newy;
			RemovePlrFromMap(pnum);
			dPlayer[newx][newy] = pnum + 1;
			FixPlayerLocation(pnum);
		}
	}
}

static BOOL MonDoAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->MData->mAFNum) {
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit, mon->mMinDamage, mon->mMaxDamage);
		if (mon->_mAi != AI_SNAKE)
			PlayEffect(mnum, 0);
	} else if (mon->_mAi == AI_MAGMA && mon->_mAnimFrame == 9) {
		// mon->_mType >= MT_NMAGMA && mon->_mType <= MT_WMAGMA
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit + 10, mon->mMinDamage - 2, mon->mMaxDamage - 2);
		PlayEffect(mnum, 0);
	} else if (mon->_mAi == AI_STORM2 && mon->_mAnimFrame == 13) {
		// mon->_mType >= MT_STORM && mon->_mType <= MT_MAEL
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit - 20, mon->mMinDamage + 4, mon->mMaxDamage + 4);
		PlayEffect(mnum, 0);
	} else if (mon->_mAi == AI_SNAKE && mon->_mAnimFrame == 1)
		PlayEffect(mnum, 0);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

static BOOL MonDoRAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoRAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->MData->mAFNum) {
		AddMissile(
		    mon->_mx,
		    mon->_my,
		    mon->_menemyx,
		    mon->_menemyy,
		    mon->_mdir,
		    mon->_mVar1,
		    1,
		    mnum,
		    0,
		    0,
		    0);
		PlayEffect(mnum, 0);
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

static BOOL MonDoRSpAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoRSpAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->MData->mAFNum2 && mon->_mAnimCnt == 0) {
		AddMissile(
		    mon->_mx,
		    mon->_my,
		    mon->_menemyx,
		    mon->_menemyy,
		    mon->_mdir,
		    mon->_mVar1,
		    1,
		    mnum,
		    0,
		    0,
		    0);
		PlayEffect(mnum, 3);
	}

	if (mon->_mAi == AI_MEGA && mon->_mAnimFrame == 3) {
		if (mon->_mVar2++ == 0) {
			mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		} else if (mon->_mVar2 == 15) {
			mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

static BOOL MonDoSAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoSAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->MData->mAFNum2)
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit2, mon->mMinDamage2, mon->mMaxDamage2);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

static BOOL MonDoFadein(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoFadein: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame)
		return FALSE;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	if (mon->mlid != 0) { // && !(mon->_mFlags & MFLAG_HIDDEN)) {
		ChangeLightRadius(mon->mlid, MON_LIGHTRAD);
		ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
	}

	MonStartStand(mnum, mon->_mdir);
	return TRUE;
}

static BOOL MonDoFadeout(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoFadeout: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame)
		return FALSE;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	if (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN) {
		mon->_mFlags |= MFLAG_HIDDEN;
		if (mon->mlid != 0) {
			ChangeLightRadius(mon->mlid, 0);
		}
	}

	MonStartStand(mnum, mon->_mdir);
	return TRUE;
}

static BOOL MonDoHeal(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoHeal: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mFlags & MFLAG_NOHEAL) {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		mon->_mmode = MM_SATTACK;
		return FALSE;
	}

	if (mon->_mAnimFrame == 1) {
		mon->_mFlags &= ~MFLAG_REV_ANIMATION;
		if (mon->_mVar1 + mon->_mhitpoints < mon->_mmaxhp) {
			mon->_mhitpoints = mon->_mVar1 + mon->_mhitpoints;
			mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		} else {
			mon->_mhitpoints = mon->_mmaxhp;
			mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
			mon->_mmode = MM_SATTACK;
		}
	}
	return FALSE;
}

static BOOL MonDoTalk(int mnum)
{
	MonsterStruct *mon;
	int tren;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoTalk: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	MonStartStand(mnum, mon->_mdir);
	mon->_mgoal = MGOAL_TALKING;
	if (effect_is_playing(alltext[mon->mtalkmsg].sfxnr))
		return FALSE;
	InitQTextMsg(mon->mtalkmsg);
	switch (mon->_uniqtype - 1) {
	case UMT_GARBUD:
		if (mon->mtalkmsg == TEXT_GARBUD1) {
			quests[Q_GARBUD]._qactive = QUEST_ACTIVE;
			quests[Q_GARBUD]._qlog = TRUE; // BUGFIX: (?) for other quests qactive and qlog go together, maybe this should actually go into the if above (fixed)
		}
		if (mon->mtalkmsg == TEXT_GARBUD2 && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
			//SetRndSeed(mon->_mRndSeed);
			SpawnItem(mnum, mon->_mx + 1, mon->_my + 1, TRUE);
			mon->_mFlags |= MFLAG_QUEST_COMPLETE;
		}
		break;
	case UMT_ZHAR:
		if (mon->mtalkmsg == TEXT_ZHAR1 && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
			quests[Q_ZHAR]._qactive = QUEST_ACTIVE;
			quests[Q_ZHAR]._qlog = TRUE;
			//SetRndSeed(mon->_mRndSeed);
			CreateTypeItem(mon->_mx + 1, mon->_my + 1, FALSE, ITYPE_MISC, IMISC_BOOK, TRUE, FALSE);
			mon->_mFlags |= MFLAG_QUEST_COMPLETE;
		}
		break;
	case UMT_SNOTSPIL:
		if (mon->mtalkmsg == TEXT_BANNER10 && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
			ObjChangeMap(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 2, (setpc_h >> 1) + setpc_y - 2);
			tren = TransVal;
			TransVal = 9;
			DRLG_MRectTrans(setpc_x, setpc_y, (setpc_w >> 1) + setpc_x + 4, setpc_y + (setpc_h >> 1));
			TransVal = tren;
			quests[Q_LTBANNER]._qvar1 = 2;
			if (quests[Q_LTBANNER]._qactive == QUEST_INIT)
				quests[Q_LTBANNER]._qactive = QUEST_ACTIVE;
			mon->_mFlags |= MFLAG_QUEST_COMPLETE;
		}
		if (quests[Q_LTBANNER]._qvar1 < 2) {
			dev_fatal("SS Talk = %i, Flags = %i", mon->mtalkmsg, mon->_mFlags);
		}
		break;
	case UMT_LAZURUS:
		if (gbMaxPlayers != 1) {
			quests[Q_BETRAYER]._qvar1 = 6;
			mon->_msquelch = UCHAR_MAX;
			mon->mtalkmsg = 0;
			mon->_mgoal = MGOAL_NORMAL;
		}
	case UMT_LACHDAN:
		if (mon->mtalkmsg == TEXT_VEIL9) {
			quests[Q_VEIL]._qactive = QUEST_ACTIVE;
			quests[Q_VEIL]._qlog = TRUE;
		}
		if (mon->mtalkmsg == TEXT_VEIL11 && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
			//SetRndSeed(mon->_mRndSeed);
			SpawnUnique(UITEM_STEELVEIL, mon->_mx + 1, mon->_my + 1);
			mon->_mFlags |= MFLAG_QUEST_COMPLETE;
		}
		break;
	case UMT_WARLORD:
		quests[Q_WARLORD]._qvar1 = 2;
		break;
	}
	return FALSE;
}

void MonTeleport(int mnum)
{
	MonsterStruct *mon;
	int k, j, x, y, _mx, _my, rx, ry;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonTeleport: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STONE)
		return;

	_mx = mon->_menemyx;
	_my = mon->_menemyy;
	rx = 2 * random_(100, 2) - 1;
	ry = 2 * random_(100, 2) - 1;

	for (j = -1; j <= 1; j++) {
		for (k = -1; k <= 1; k++) {
			if (j != 0 || k != 0) {
				x = _mx + rx * j;
				y = _my + ry * k;
				if (IN_DUNGEON_AREA(x, y) && x != mon->_mx && y != mon->_my) {
					if (PosOkMonst(mnum, x, y)) {
						MonClearSquares(mnum);
						dMonster[mon->_mx][mon->_my] = 0;
						dMonster[x][y] = mnum + 1;
						mon->_moldx = x;
						mon->_moldy = y;
						mon->_mdir = MonGetDir(mnum);
						return;
					}
				}
			}
		}
	}
}

static BOOL MonDoGotHit(int mnum)
{
	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoGotHit: Invalid monster %d", mnum);
	}
	if (monster[mnum]._mAnimFrame == monster[mnum]._mAnimLen) {
		MonStartStand(mnum, monster[mnum]._mdir);

		return TRUE;
	}

	return FALSE;
}

void MonUpdateLeader(int mnum)
{
	MonsterStruct *mon;
	int i;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonUpdateLeader: Invalid monster %d", mnum);
	}
	for (i = 0; i < nummonsters; i++) {
		mon = &monster[monstactive[i]];
		if (mon->leaderflag != MLEADER_NONE && mon->leader == mnum)
			mon->leaderflag = MLEADER_NONE;
	}

	if (monster[mnum].leaderflag == MLEADER_PRESENT) {
		monster[monster[mnum].leader].packsize--;
	}
}

void DoEnding()
{
	BOOL bMusicOn;
	int musicVolume;

	if (gbMaxPlayers != 1) {
		SNetLeaveGame(LEAVE_ENDING);
	}

	music_stop();

	if (gbMaxPlayers != 1) {
		SDL_Delay(1000);
	}

	const char *vicSets[NUM_CLASSES] = {
		"gendata\\DiabVic2.smk", "gendata\\DiabVic3.smk", "gendata\\DiabVic1.smk"
#ifdef HELLFIRE
		, "gendata\\DiabVic1.smk", "gendata\\DiabVic3.smk", "gendata\\DiabVic2.smk"
#endif
	};
	play_movie(vicSets[plr[myplr]._pClass], 0);
	play_movie("gendata\\Diabend.smk", 0);

	bMusicOn = gbMusicOn;
	gbMusicOn = TRUE;

	musicVolume = sound_get_or_set_music_volume(1);
	sound_get_or_set_music_volume(0);

	music_start(TMUSIC_L2);
	play_movie("gendata\\loopdend.smk", MOV_SKIP | MOV_LOOP);
	music_stop();

	sound_get_or_set_music_volume(musicVolume);
	gbMusicOn = bMusicOn;
}

void PrepDoEnding()
{
	int i;
	DWORD killLevel;

	gbSoundOn = sgbSaveSoundOn;
	gbRunGame = FALSE;
	deathflag = FALSE;
	cineflag = TRUE;

	killLevel = gnDifficulty + 1;
	if (killLevel > plr[myplr]._pDiabloKillLevel)
		plr[myplr]._pDiabloKillLevel = killLevel;

	if (gbMaxPlayers == 1) {
		// save the hero + items
		pfile_write_hero();
		// delete the game
		pfile_delete_save_file(SAVEFILE_GAME);
	} else {
		for (i = 0; i < MAX_PLRS; i++) {
			plr[i]._pmode = PM_QUIT;
			plr[i]._pInvincible = TRUE;
			if (plr[i]._pHitPoints < (1 << 6))
				plr[i]._pHitPoints = (1 << 6);
		}
	}
}

static BOOL MonDoDeath(int mnum)
{
	MonsterStruct *mon;
	int x, y;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoDeath: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mType == MT_DIABLO) {
		x = mon->_mx - ViewX;
		if (x < 0)
			x = -1;
		else
			x = x > 0;
		ViewX += x;

		y = mon->_my - ViewY;
		if (y < 0) {
			y = -1;
		} else {
			y = y > 0;
		}
		ViewY += y;

		if (++mon->_mVar1 == 140)
			PrepDoEnding();
	} else if (mon->_mAnimFrame == mon->_mAnimLen) {
		AddDead(mnum);
	}
	return FALSE;
}

static BOOL MonDoSpStand(int mnum)
{
	MonsterStruct *mon;
	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoSpStand: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->MData->mAFNum2)
		PlayEffect(mnum, 3);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

static BOOL MonDoDelay(int mnum)
{
	MonsterStruct *mon;
	int oFrame;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoDelay: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mAnimData = mon->_mAnims[MA_STAND].Data[MonGetDir(mnum)];

	if (mon->_mVar2-- == 0) {
		oFrame = mon->_mAnimFrame;
		MonStartStand(mnum, mon->_mdir);
		mon->_mAnimFrame = oFrame;
		return TRUE;
	}

	return FALSE;
}

static BOOL MonDoStone(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoStone: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mhitpoints == 0) {
		mon->_mDelFlag = TRUE;
		dMonster[mon->_mx][mon->_my] = 0;
	}

	return FALSE;
}

void MonWalkDir(int mnum, int md)
{
	const int *mwi;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonWalkDir: Invalid monster %d", mnum);
	}
	mwi = MWVel[monster[mnum]._mAnims[MA_WALK].Frames - 1];
	switch (md) {
	case DIR_N:
		MonStartWalk(mnum, 0, -mwi[1], -1, -1, DIR_N);
		break;
	case DIR_NE:
		MonStartWalk(mnum, mwi[1], -mwi[0], 0, -1, DIR_NE);
		break;
	case DIR_E:
		MonStartWalk3(mnum, mwi[2], 0, -32, -16, 1, -1, 1, 0, DIR_E);
		break;
	case DIR_SE:
		MonStartWalk2(mnum, mwi[1], mwi[0], -32, -16, 1, 0, DIR_SE);
		break;
	case DIR_S:
		MonStartWalk2(mnum, 0, mwi[1], 0, -32, 1, 1, DIR_S);
		break;
	case DIR_SW:
		MonStartWalk2(mnum, -mwi[1], mwi[0], 32, -16, 0, 1, DIR_SW);
		break;
	case DIR_W:
		MonStartWalk3(mnum, -mwi[2], 0, 32, -16, -1, 1, 0, 1, DIR_W);
		break;
	case DIR_NW:
		MonStartWalk(mnum, -mwi[1], -mwi[0], -1, 0, DIR_NW);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void GroupUnity(int mnum)
{
	MonsterStruct *mon, *leader, *bmon;
	int i;
	BOOL clear;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("GroupUnity: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->leaderflag != MLEADER_NONE) {
		leader = &monster[mon->leader];
		clear = LineClearF(CheckNoSolid, mon->_mx, mon->_my, leader->_mfutx, leader->_mfuty);
		if (clear) {
			if (mon->leaderflag == MLEADER_AWAY
			 && abs(mon->_mx - leader->_mfutx) < 4
			 && abs(mon->_my - leader->_mfuty) < 4) {
				leader->packsize++;
				mon->leaderflag = MLEADER_PRESENT;
			}
		} else if (mon->leaderflag == MLEADER_PRESENT) {
			leader->packsize--;
			mon->leaderflag = MLEADER_AWAY;
		}
	}

	if (mon->leaderflag == MLEADER_PRESENT) {
		if (mon->_msquelch > leader->_msquelch) {
			leader->_lastx = mon->_mx;
			leader->_lasty = mon->_my;
			leader->_msquelch = mon->_msquelch - 1;
		}
	} else if (mon->_uniqtype != 0) {
		if (UniqMonst[mon->_uniqtype - 1].mUnqAttr & 2) {
			for (i = 0; i < nummonsters; i++) {
				bmon = &monster[monstactive[i]];
				if (bmon->leaderflag == MLEADER_PRESENT && bmon->leader == mnum) {
					if (mon->_msquelch > bmon->_msquelch) {
						bmon->_lastx = mon->_mx;
						bmon->_lasty = mon->_my;
						bmon->_msquelch = mon->_msquelch - 1;
					}
				}
			}
		}
	}
}

static BOOL MonCallWalk(int mnum, int md)
{
	int mdtemp;
	BOOL ok;

	mdtemp = md;
	ok = DirOK(mnum, md);
	if (random_(101, 2) != 0)
		ok = ok || (md = left[mdtemp], DirOK(mnum, md)) || (md = right[mdtemp], DirOK(mnum, md));
	else
		ok = ok || (md = right[mdtemp], DirOK(mnum, md)) || (md = left[mdtemp], DirOK(mnum, md));
	if (random_(102, 2) != 0)
		ok = ok
		    || (md = right[right[mdtemp]], DirOK(mnum, md))
		    || (md = left[left[mdtemp]], DirOK(mnum, md));
	else
		ok = ok
		    || (md = left[left[mdtemp]], DirOK(mnum, md))
		    || (md = right[right[mdtemp]], DirOK(mnum, md));
	if (ok)
		MonWalkDir(mnum, md);
	return ok;
}

static BOOL MonPathWalk(int mnum)
{
	char path[MAX_PATH_LENGTH];
	BOOL(*Check)
	(int, int, int);

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MonPathWalk: Invalid monster %d", mnum);
	}
	Check = PosOkMonst3;
	if (!(monster[mnum]._mFlags & MFLAG_CAN_OPEN_DOOR))
		Check = PosOkMonst;

	if (FindPath(Check, mnum, monster[mnum]._mx, monster[mnum]._my, monster[mnum]._menemyx, monster[mnum]._menemyy, path) != 0) {
		MonCallWalk(mnum, plr2monst[path[0]]); /* plr2monst is local */
		return TRUE;
	}

	return FALSE;
}

static BOOL MonCallWalk2(int mnum, int md)
{
	BOOL ok;
	int mdtemp;

	mdtemp = md;
	ok = DirOK(mnum, md);       // Can we continue in the same direction
	if (random_(101, 2) != 0) { // Randomly go left or right
		ok = ok || (mdtemp = left[md], DirOK(mnum, left[md])) || (mdtemp = right[md], DirOK(mnum, right[md]));
	} else {
		ok = ok || (mdtemp = right[md], DirOK(mnum, right[md])) || (mdtemp = left[md], DirOK(mnum, left[md]));
	}

	if (ok)
		MonWalkDir(mnum, mdtemp);

	return ok;
}

static BOOL MonDumbWalk(int mnum, int md)
{
	BOOL ok;
	ok = DirOK(mnum, md);
	if (ok)
		MonWalkDir(mnum, md);

	return ok;
}

static BOOL MonRoundWalk(int mnum, int md, int *dir)
{
	int mdtemp;
	BOOL ok;
	if (*dir)
		md = left[left[md]];
	else
		md = right[right[md]];

	ok = DirOK(mnum, md);
	mdtemp = md;
	if (!ok) {
		if (*dir) {
			md = right[mdtemp];
			ok = DirOK(mnum, md) || (md = right[right[mdtemp]], DirOK(mnum, md));
		} else {
			md = left[mdtemp];
			ok = (DirOK(mnum, md) || (md = left[left[mdtemp]], DirOK(mnum, md)));
		}
	}
	if (ok) {
		MonWalkDir(mnum, md);
	} else {
		*dir = !*dir;
		ok = MonCallWalk(mnum, opposite[mdtemp]);
	}
	return ok;
}

void MAI_Zombie(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Zombie: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	if (!(dFlags[mx][my] & BFLAG_VISIBLE)) {
		return;
	}

	if (random_(103, 100) < 2 * mon->_mint + 10) {
		md = std::max(abs(mx - mon->_menemyx), abs(my - mon->_menemyy));
		if (md >= 2) {
			if (md >= 2 * mon->_mint + 4) {
				md = mon->_mdir;
				if (random_(104, 100) < 2 * mon->_mint + 20) {
					md = random_(104, 8);
				}
				MonDumbWalk(mnum, md);
			} else {
				md = MonGetDir(mnum);
				MonCallWalk(mnum, md);
			}
		} else {
			MonStartAttack(mnum);
		}
	}
}

void MAI_SkelSd(int mnum)
{
	MonsterStruct *mon;
	int mx, my, x, y, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelSd: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	x = mx - mon->_menemyx;
	y = my - mon->_menemyy;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	if (abs(x) >= 2 || abs(y) >= 2) {
		if (mon->_mVar1 == MM_DELAY || (random_(106, 100) >= 35 - 4 * mon->_mint)) {
			MonCallWalk(mnum, md);
		} else {
			MonStartDelay(mnum, 15 - 2 * mon->_mint + random_(106, 10));
		}
	} else {
		if (mon->_mVar1 == MM_DELAY || (random_(105, 100) < 2 * mon->_mint + 20)) {
			MonStartAttack(mnum);
		} else {
			MonStartDelay(mnum, 2 * (5 - mon->_mint) + random_(105, 10));
		}
	}
}

static BOOL MAI_Path(int mnum)
{
	MonsterStruct *mon;
	BOOL clear;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Path: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mType != MT_GOLEM) {
		if (mon->_msquelch == 0)
			return FALSE;
		if (mon->_mmode != MM_STAND)
			return FALSE;
		if (mon->_mgoal != MGOAL_NORMAL && mon->_mgoal != MGOAL_MOVE && mon->_mgoal != MGOAL_ATTACK2)
			return FALSE;
	}

	clear = LineClearF1(
	    PosOkMonst2,
	    mnum,
	    mon->_mx,
	    mon->_my,
	    mon->_menemyx,
	    mon->_menemyy);
	if (!clear || mon->_mpathcount >= 5 && mon->_mpathcount < 8) {
		if (mon->_mFlags & MFLAG_CAN_OPEN_DOOR)
			MonstCheckDoors(mnum);
		mon->_mpathcount++;
		if (mon->_mpathcount < 5)
			return FALSE;
		if (MonPathWalk(mnum))
			return TRUE;
	}

	if (mon->_mType != MT_GOLEM)
		mon->_mpathcount = 0;

	return FALSE;
}

void MAI_Snake(int mnum)
{
	MonsterStruct *mon;
	int fx, fy, mx, my, md;
	int tmp;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Snake: Invalid monster %d", mnum);
	}
	char pattern[6] = { 1, 1, 0, -1, -1, 0 };
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	mx = mon->_mx - fx;
	my = mon->_my - fy;
	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if (abs(mx) < 3 && abs(my) < 3 && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy) && mon->_mVar1 != MM_CHARGE) {
			if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, 1, mnum, 0, 0, 0) != -1) {
				PlayEffect(mnum, 0);
				dMonster[mon->_mx][mon->_my] = -(mnum + 1);
				mon->_mmode = MM_CHARGE;
			}
		} else if (mon->_mVar1 == MM_DELAY || random_(106, 100) >= 35 - 2 * mon->_mint) {
			tmp = md + pattern[mon->_mgoalvar1];
			if (tmp < 0) {
				tmp += 8;
			} else if (tmp >= 8) {
				tmp -= 8;
			}
			mon->_mgoalvar1++;
			if (mon->_mgoalvar1 > 5)
				mon->_mgoalvar1 = 0;
			md = tmp - mon->_mgoalvar2;
			if (md < 0) {
				md += 8;
			} else if (md >= 8) {
				md -= 8;
			}
			if (md > 0) {
				if (md < 4) {
					md = mon->_mgoalvar2 + 1;
					if (md < 0) {
						md += 8;
					} else if (md >= 8) {
						md -= 8;
					}
					mon->_mgoalvar2 = md;
				} else if (md == 4) {
					mon->_mgoalvar2 = tmp;
				} else {
					md = mon->_mgoalvar2 - 1;
					if (md < 0) {
						md += 8;
					} else if (md >= 8) {
						md -= 8;
					}
					mon->_mgoalvar2 = md;
				}
			}
			if (!MonDumbWalk(mnum, mon->_mgoalvar2))
				MonCallWalk2(mnum, mon->_mdir);
		} else {
			MonStartDelay(mnum, 15 - mon->_mint + random_(106, 10));
		}
	} else {
		if (mon->_mVar1 == MM_DELAY
		    || mon->_mVar1 == MM_CHARGE
		    || (random_(105, 100) < mon->_mint + 20)) {
			MonStartAttack(mnum);
		} else
			MonStartDelay(mnum, 10 - mon->_mint + random_(105, 10));
	}
}

void MAI_Bat(int mnum)
{
	MonsterStruct *mon;
	int md, v, pnum;
	int fx, fy, xd, yd;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Bat: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	v = random_(107, 100);
	if (mon->_mgoal == MGOAL_RETREAT) {
		if (mon->_mgoalvar1 == 0) {
			MonCallWalk(mnum, opposite[md]);
			mon->_mgoalvar1++;
		} else {
			if (random_(108, 2) != 0)
				MonCallWalk(mnum, left[md]);
			else
				MonCallWalk(mnum, right[md]);
			mon->_mgoal = MGOAL_NORMAL;
		}
		return;
	}

	pnum = mon->_menemy;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	xd = mon->_mx - fx;
	yd = mon->_my - fy;
	if (mon->_mType == MT_GLOOM
	    && (abs(xd) >= 5 || abs(yd) >= 5)
	    && v < 4 * mon->_mint + 33
	    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
		if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, 1, mnum, 0, 0, 0) != -1) {
			dMonster[mon->_mx][mon->_my] = -(mnum + 1);
			mon->_mmode = MM_CHARGE;
		}
	} else if (abs(xd) >= 2 || abs(yd) >= 2) {
		if (mon->_mVar2 > 20 && v < mon->_mint + 13
		    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < mon->_mint + 63) {
			MonCallWalk(mnum, md);
		}
	} else if (v < 4 * mon->_mint + 8) {
		MonStartAttack(mnum);
		mon->_mgoal = MGOAL_RETREAT;
		mon->_mgoalvar1 = 0;
		if (mon->_mType == MT_FAMILIAR) {
			AddMissile(mon->_menemyx, mon->_menemyy, 0, 0, -1, MIS_LIGHTNING, 1, mnum, 1, 10, 0);
		}
	}
}

void MAI_SkelBow(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md, v;
	BOOL walking;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelBow: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;

	md = MonGetDir(mnum);
	mon->_mdir = md;
	v = random_(110, 100);

	walking = FALSE;
	if (abs(mx) < 4 && abs(my) < 4) {
		if (mon->_mVar2 > 20 && v < 2 * mon->_mint + 13
		    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < 2 * mon->_mint + 63) {
			walking = MonDumbWalk(mnum, opposite[md]);
		}
	}

	if (!walking) {
		if (random_(110, 100) < 2 * mon->_mint + 3) {
			if (LineClear(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy))
				MonStartRAttack(mnum, MIS_ARROW);
		}
	}
}

void MAI_Fat(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fat: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;
	md = MonGetDir(mnum);
	mon->_mdir = md;
	v = random_(111, 100);
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if (mon->_mVar2 > 20 && v < 4 * mon->_mint + 20
		    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < 4 * mon->_mint + 70) {
			MonCallWalk(mnum, md);
		}
	} else if (v < 4 * mon->_mint + 15) {
		MonStartAttack(mnum);
	} else if (v < 4 * mon->_mint + 20) {
		MonStartSpAttack(mnum);
	}
}

void MAI_Sneak(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md;
	int dist, range, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Sneak: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STAND) {
		mx = mon->_mx;
		my = mon->_my;
		if (dLight[mx][my] != LIGHTMAX) {
			mx -= mon->_menemyx;
			my -= mon->_menemyy;

			md = MonGetDir(mnum);
			range = 5 - mon->_mint;
			dist = std::max(abs(mx), abs(my));
			if (mon->_mVar1 == MM_GOTHIT) {
				mon->_mgoal = MGOAL_RETREAT;
				mon->_mgoalvar1 = 0;
			} else {
				if (dist >= range + 3 || mon->_mgoalvar1 > 8) {
					mon->_mgoal = MGOAL_NORMAL;
					mon->_mgoalvar1 = 0;
				}
			}
			if (mon->_mgoal == MGOAL_RETREAT && !(mon->_mFlags & MFLAG_NO_ENEMY)) {
				md = mon->_menemy;
				if (mon->_mFlags & MFLAG_TARGETS_MONSTER)
					md = GetDirection(monster[md]._mx, monster[md]._my, mon->_mx, mon->_my);
				else
					md = GetDirection(plr[md]._px, plr[md]._py, mon->_mx, mon->_my);
				if (mon->_mType == MT_UNSEEN) {
					if (random_(112, 2) != 0)
						md = left[md];
					else
						md = right[md];
				}
			}
			mon->_mdir = md;
			v = random_(112, 100);
			if (dist < range && mon->_mFlags & MFLAG_HIDDEN) {
				MonStartFadein(mnum, md, FALSE);
			} else {
				if ((dist >= range + 1) && !(mon->_mFlags & MFLAG_HIDDEN)) {
					MonStartFadeout(mnum, md, TRUE);
				} else {
					if (mon->_mgoal == MGOAL_RETREAT
					    || dist >= 2 && (mon->_mVar2 > 20 && v < 4 * mon->_mint + 14 || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3) && mon->_mVar2 == 0 && v < 4 * mon->_mint + 64)) {
						mon->_mgoalvar1++;
						MonCallWalk(mnum, md);
					}
				}
			}
			if (mon->_mmode == MM_STAND
			 && (dist < 2 && v < 4 * mon->_mint + 10)) {
				MonStartAttack(mnum);
			}
		}
	}
}

void MAI_Fireman(int mnum)
{
	int xd, yd;
	int md, pnum;
	int fx, fy;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fireman: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;

	pnum = mon->_menemy;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	xd = mon->_mx - fx;
	yd = mon->_my - fy;

	md = MonGetDir(mnum);
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (LineClear(mon->_mx, mon->_my, fx, fy)
		    && AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_FIREMAN, 1, mnum, 0, 0, 0) != -1) {
			mon->_mmode = MM_CHARGE;
			mon->_mgoal = MGOAL_ATTACK2;
			mon->_mgoalvar1 = 0;
		}
	} else if (mon->_mgoal == MGOAL_ATTACK2) {
		if (mon->_mgoalvar1 == 3) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadeout(mnum, md, TRUE);
		} else if (LineClear(mon->_mx, mon->_my, fx, fy)) {
			MonStartRAttack(mnum, MIS_KRULL);
			mon->_mgoalvar1++;
		} else {
			MonStartDelay(mnum, RandRange(5, 14));
			mon->_mgoalvar1++;
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		MonStartFadein(mnum, md, FALSE);
		mon->_mgoal = MGOAL_ATTACK2;
	}
	mon->_mdir = md;
	random_(112, 100);
	if (mon->_mmode != MM_STAND)
		return;

	if (abs(xd) < 2 && abs(yd) < 2 && mon->_mgoal == MGOAL_NORMAL) {
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit, mon->mMinDamage, mon->mMaxDamage);
		mon->_mgoal = MGOAL_RETREAT;
		if (!MonCallWalk(mnum, opposite[md])) {
			MonStartFadein(mnum, md, FALSE);
			mon->_mgoal = MGOAL_ATTACK2;
		}
	} else if (!MonCallWalk(mnum, md) && (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT)) {
		MonStartFadein(mnum, md, FALSE);
		mon->_mgoal = MGOAL_ATTACK2;
	}
}

void MAI_Fallen(int mnum)
{
	MonsterStruct *mon;
	int x, y, mx, my, m, rad;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fallen: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mgoal == MGOAL_ATTACK2) {
		if (mon->_mgoalvar1 != 0)
			mon->_mgoalvar1--;
		else
			mon->_mgoal = MGOAL_NORMAL;
	}
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	if (mon->_mgoal == MGOAL_RETREAT) {
		if (mon->_mgoalvar1-- == 0) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartStand(mnum, opposite[mon->_mdir]);
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		if (random_(113, 4) != 0) {
			return;
		}
		if (!(mon->_mFlags & MFLAG_NOHEAL)) {
			MonStartSpStand(mnum, mon->_mdir);
			rad = mon->_mhitpoints + 2 * mon->_mint + 2;
			if (mon->_mmaxhp >= rad)
				mon->_mhitpoints = rad;
			else
				mon->_mhitpoints = mon->_mmaxhp;
		}
		rad = std::min(2 * mon->_mint + 4, MAXDUNX / 2 - 1);
		mx = std::min(std::max(mon->_mx, rad), MAXDUNX - rad - 1);
		my = std::min(std::max(mon->_my, rad), MAXDUNY - rad - 1);
		for (y = -rad; y <= rad; y++) {
			for (x = -rad; x <= rad; x++) {
				m = dMonster[x + mx][y + my];
				if (m > 0) {
					m--;
					if (monster[m]._mAi == AI_FALLEN) {
						monster[m]._mgoal = MGOAL_ATTACK2;
						monster[m]._mgoalvar1 = 30 * mon->_mint + 105;
					}
				}
			}
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		MonCallWalk(mnum, mon->_mdir);
	} else if (mon->_mgoal == MGOAL_ATTACK2) {
		if (abs(mon->_mx - mon->_menemyx) < 2 && abs(mon->_my - mon->_menemyy) < 2) {
			MonStartAttack(mnum);
		} else {
			MonCallWalk(mnum, MonGetDir(mnum));
		}
	} else {
		MAI_SkelSd(mnum);
	}
}

void MAI_Cleaver(int mnum)
{
	MonsterStruct *mon;
	int x, y, mx, my, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Cleaver: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	x = mx - mon->_menemyx;
	y = my - mon->_menemyy;

	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;

	if (abs(x) >= 2 || abs(y) >= 2)
		MonCallWalk(mnum, md);
	else
		MonStartAttack(mnum);
}

static void MAI_Round(int mnum, BOOL special)
{
	MonsterStruct *mon;
	int fx, fy;
	int mx, my, md;
	int dist, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Round: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fy = mon->_menemyy;
		fx = mon->_menemyx;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(114, 100);
		dist = std::max(abs(mx), abs(my));
		if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || dist >= 4 && random_(115, 4) == 0) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(116, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md) || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
					MonStartDelay(mnum, RandRange(10, 19));
				}
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (dist >= 2) {
				if (mon->_mVar2 > 20 && v < 2 * mon->_mint + 28
				    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				        && mon->_mVar2 == 0
				        && v < 2 * mon->_mint + 78) {
					MonCallWalk(mnum, md);
				}
			} else if (v < 2 * mon->_mint + 23) {
				mon->_mdir = md;
				if (special && mon->_mhitpoints < (mon->_mmaxhp >> 1) && random_(117, 2) != 0)
					MonStartSpAttack(mnum);
				else
					MonStartAttack(mnum);
			}
		}
	}
}

void MAI_GoatMc(int mnum)
{
	MAI_Round(mnum, TRUE);
}

static void MAI_Ranged(int mnum, int mitype, BOOL special)
{
	int md;
	int fx, fy, mx, my;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Ranged: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	if (mon->_msquelch == UCHAR_MAX || mon->_mFlags & MFLAG_TARGETS_MONSTER) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = MonGetDir(mnum);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		mon->_mdir = md;
		if (mon->_mVar1 == MM_RATTACK) {
			MonStartDelay(mnum, random_(118, 20));
		} else if (abs(mx) < 4 && abs(my) < 4) {
			if (random_(119, 100) < 10 * (mon->_mint + 7))
				MonCallWalk(mnum, opposite[md]);
		}
		if (mon->_mmode == MM_STAND) {
			if (LineClear(mon->_mx, mon->_my, fx, fy)) {
				if (special)
					MonStartRSpAttack(mnum, mitype);
				else
					MonStartRAttack(mnum, mitype);
			}
		}
	} else if (mon->_msquelch != 0) {
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		MonCallWalk(mnum, md);
	}
}

void MAI_GoatBow(int mnum)
{
	MAI_Ranged(mnum, MIS_ARROW, FALSE);
}

void MAI_Succ(int mnum)
{
	MAI_Ranged(mnum, MIS_FLARE, FALSE);
}

void MAI_SnowWich(int mnum)
{
	MAI_Ranged(mnum, MIS_SNOWWICH, FALSE);
}

void MAI_HlSpwn(int mnum)
{
	MAI_Ranged(mnum, MIS_HLSPWN, FALSE);
}

void MAI_SolBrnr(int mnum)
{
	MAI_Ranged(mnum, MIS_SOLBRNR, FALSE);
}

#ifdef HELLFIRE
void MAI_Lich(int mnum)
{
	MAI_Ranged(mnum, MIS_LICH, FALSE);
}

void MAI_ArchLich(int mnum)
{
	MAI_Ranged(mnum, MIS_ARCHLICH, FALSE);
}

void MAI_PsychOrb(int mnum)
{
	MAI_Ranged(mnum, MIS_PSYCHORB, FALSE);
}

void MAI_NecromOrb(int mnum)
{
	MAI_Ranged(mnum, MIS_NECROMORB, FALSE);
}
#endif

void MAI_AcidUniq(int mnum)
{
	MAI_Ranged(mnum, MIS_ACID, TRUE);
}

#ifdef HELLFIRE
void MAI_Firebat(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBOLT, FALSE);
}

void MAI_Torchant(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBALL, FALSE);
}
#endif

void MAI_Scav(int mnum)
{
	BOOL done;
	int x, y, maxhp;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Scav: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	done = FALSE;
	if (mon->_mmode != MM_STAND)
		return;
	if (mon->_mhitpoints < (mon->_mmaxhp >> 1) && mon->_mgoal != MGOAL_HEALING) {
		if (mon->leaderflag != MLEADER_NONE) {
			monster[mon->leader].packsize--;
			mon->leaderflag = MLEADER_NONE;
		}
		mon->_mgoal = MGOAL_HEALING;
		mon->_mgoalvar3 = 10;
	}
	if (mon->_mgoal == MGOAL_HEALING && mon->_mgoalvar3 != 0) {
		mon->_mgoalvar3--;
		if (dDead[mon->_mx][mon->_my] != 0) {
			MonStartEat(mnum);
			maxhp = mon->_mmaxhp;
			if (!(mon->_mFlags & MFLAG_NOHEAL)) {
#ifdef HELLFIRE
				mon->_mhitpoints += maxhp >> 3;
				if (mon->_mhitpoints > maxhp)
					mon->_mhitpoints = maxhp;
				if (mon->_mhitpoints == maxhp || mon->_mgoalvar3 <= 0)
					dDead[mon->_mx][mon->_my] = 0;
			}
			if (mon->_mhitpoints == maxhp) {
#else
				mon->_mhitpoints += 1 << 6;
				if (mon->_mhitpoints > maxhp)
					mon->_mhitpoints = maxhp;
			}
			if (mon->_mhitpoints >= (maxhp >> 1) + (maxhp >> 2)) {
#endif
				mon->_mgoal = MGOAL_NORMAL;
				mon->_mgoalvar1 = 0;
				mon->_mgoalvar2 = 0;
			}
		} else {
			if (mon->_mgoalvar1 == 0) {
				if (random_(120, 2) != 0) {
					for (y = -4; y <= 4 && !done; y++) {
						for (x = -4; x <= 4 && !done; x++) {
							// BUGFIX: incorrect check of offset against limits of the dungeon
							if (IN_DUNGEON_AREA(x, y))
								done = dDead[mon->_mx + x][mon->_my + y] != 0
								    && LineClearF(
								           CheckNoSolid,
								           mon->_mx,
								           mon->_my,
								           mon->_mx + x,
								           mon->_my + y);
						}
					}
					x--;
					y--;
				} else {
					for (y = 4; y >= -4 && !done; y--) {
						for (x = 4; x >= -4 && !done; x--) {
							// BUGFIX: incorrect check of offset against limits of the dungeon
							if (IN_DUNGEON_AREA(x, y))
								done = dDead[mon->_mx + x][mon->_my + y] != 0
								    && LineClearF(
								           CheckNoSolid,
								           mon->_mx,
								           mon->_my,
								           mon->_mx + x,
								           mon->_my + y);
						}
					}
					x++;
					y++;
				}
				if (done) {
					mon->_mgoalvar1 = x + mon->_mx + 1;
					mon->_mgoalvar2 = y + mon->_my + 1;
				}
			}
			if (mon->_mgoalvar1) {
				x = mon->_mgoalvar1 - 1;
				y = mon->_mgoalvar2 - 1;
				mon->_mdir = GetDirection(mon->_mx, mon->_my, x, y);
				MonCallWalk(mnum, mon->_mdir);
			}
		}
	}

	if (mon->_mmode == MM_STAND)
		MAI_SkelSd(mnum);
}

void MAI_Garg(int mnum)
{
	MonsterStruct *mon;
	int mx, my;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Garg: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mFlags & MFLAG_GARG_STONE) {
		if (mon->_msquelch != 0) {
			MonEnemy(mnum);
			mx = mon->_mx - mon->_menemyx;
			my = mon->_my - mon->_menemyy;
			if (abs(mx) < mon->_mint + 2 && abs(my) < mon->_mint + 2) {
				mon->_mFlags &= ~(MFLAG_LOCK_ANIMATION | MFLAG_GARG_STONE);
			}
		} else if (mon->_mmode != MM_SATTACK) {
			if (mon->leaderflag == MLEADER_NONE && mon->_uniqtype == 0) {
				mon->_mmode = MM_SATTACK;
				NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);
				mon->_mFlags |= MFLAG_LOCK_ANIMATION;
			} else {
				mon->_mFlags &= ~MFLAG_GARG_STONE;
			}
		}
		return;
	}

	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	if (mon->_mhitpoints < (mon->_mmaxhp >> 1))
#ifndef HELLFIRE
		if (!(mon->_mFlags & MFLAG_NOHEAL))
#endif
			mon->_mgoal = MGOAL_RETREAT;
	if (mon->_mgoal == MGOAL_RETREAT) {
		mx = mon->_mx - mon->_lastx;
		my = mon->_my - mon->_lasty;
		if (abs(mx) >= mon->_mint + 2 || abs(my) >= mon->_mint + 2) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartHeal(mnum);
		} else if (!MonCallWalk(mnum, opposite[MonGetDir(mnum)])) {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	MAI_Round(mnum, FALSE);
}

static void MAI_RoundRanged(int mnum, int mitype, BOOL checkdoors, int lessmissiles)
{
	MonsterStruct *mon;
	int mx, my;
	int fx, fy;
	int md, dist, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_RoundRanged: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (checkdoors && mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(121, 10000);
		dist = std::max(abs(mx), abs(my));
		if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || (dist >= 3 && random_(122, 4 << lessmissiles) == 0)) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(123, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md)) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (v < ((500 * (mon->_mint + 1)) >> lessmissiles)
				    && (LineClear(mon->_mx, mon->_my, fx, fy))) {
					MonStartRSpAttack(mnum, mitype);
				} else {
					MonRoundWalk(mnum, md, &mon->_mgoalvar2);
				}
			}
		} else {
			mon->_mgoal = MGOAL_NORMAL;
		}
		if (mon->_mgoal == MGOAL_NORMAL) {
			if ((dist >= 3 && v < ((500 * (mon->_mint + 2)) >> lessmissiles)
			        || v < ((500 * (mon->_mint + 1)) >> lessmissiles))
			    && LineClear(mon->_mx, mon->_my, fx, fy)) {
				MonStartRSpAttack(mnum, mitype);
			} else if (dist >= 2) {
				v = random_(124, 100);
				if (v < 1000 * (mon->_mint + 5)
				    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3) && mon->_mVar2 == 0 && v < 1000 * (mon->_mint + 8)) {
					MonCallWalk(mnum, md);
				}
			} else if (v < 1000 * (mon->_mint + 6)) {
				mon->_mdir = md;
				MonStartAttack(mnum);
			}
		}
		if (mon->_mmode == MM_STAND) {
			MonStartDelay(mnum, RandRange(5, 14));
		}
	}
}

void MAI_Magma(int mnum)
{
	MAI_RoundRanged(mnum, MIS_MAGMABALL, TRUE, 0);
}

void MAI_Storm(int mnum)
{
	MAI_RoundRanged(mnum, MIS_LIGHTNINGC, TRUE, 0);
}

void MAI_Storm2(int mnum)
{
	MAI_RoundRanged(mnum, MIS_LIGHTNINGC2, TRUE, 0);
}

#ifdef HELLFIRE
void MAI_BoneDemon(int mnum)
{
	MAI_RoundRanged(mnum, MIS_BONEDEMON, TRUE, 0);
}
#endif

void MAI_Acid(int mnum)
{
	MAI_RoundRanged(mnum, MIS_ACID, FALSE, 1);
}

void MAI_Diablo(int mnum)
{
	MAI_RoundRanged(mnum, MIS_APOCAC2, FALSE, 0);
}

static void MAI_RR2(int mnum, int mitype)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int dist, v, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_RR2: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	mx = mon->_mx;
	my = mon->_my;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist >= 5) {
		MAI_SkelSd(mnum);
		return;
	}

		md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(121, 100);
		if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || dist >= 3) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(123, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar3 = 4;
				if (mon->_mgoalvar1++ < 2 * dist || !DirOK(mnum, md)) {
					if (v < 5 * (mon->_mint + 16))
						MonRoundWalk(mnum, md, &mon->_mgoalvar2);
				} else
					mon->_mgoal = MGOAL_NORMAL;
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if ((dist >= 3 && v < 5 * (mon->_mint + 2) || v < 5 * (mon->_mint + 1) || mon->_mgoalvar3 == 4) && LineClear(mon->_mx, mon->_my, fx, fy)) {
				MonStartRSpAttack(mnum, mitype);
			} else if (dist >= 2) {
				v = random_(124, 100);
				if (v < 2 * (5 * mon->_mint + 25)
				    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				        && mon->_mVar2 == 0
				        && v < 2 * (5 * mon->_mint + 40)) {
					MonCallWalk(mnum, md);
				}
			} else {
				if (random_(124, 100) < 10 * (mon->_mint + 4)) {
					mon->_mdir = md;
					if (random_(124, 2) != 0)
						MonStartAttack(mnum);
					else
						MonStartRSpAttack(mnum, mitype);
				}
			}
			mon->_mgoalvar3 = 1;
		}
		if (mon->_mmode == MM_STAND) {
			MonStartDelay(mnum, RandRange(5, 14));
		}
}

void MAI_Mega(int mnum)
{
	MAI_RR2(mnum, MIS_FLAMEC);
}

void MAI_Golum(int mnum)
{
	MonsterStruct *mon, *tmon;
	int md, j, k;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Golum: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	assert(!MINION_INACTIVE(mon));

	if (mon->_mmode == MM_DEATH
	 || mon->_mmode == MM_SPSTAND
	 || mon->_mmode == MM_ATTACK
	 || (mon->_mmode >= MM_WALK && mon->_mmode <= MM_WALK3)) {
		return;
	}

	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER))
		MonEnemy(mnum);

	if (!(mon->_mFlags & MFLAG_NO_ENEMY)) {
		tmon = &monster[mon->_menemy];

		if (abs(mon->_mx - tmon->_mfutx) >= 2 || abs(mon->_my - tmon->_mfuty) >= 2) {
			if (MAI_Path(mnum)) {
				return;
			}
		} else {
			mon->_menemyx = tmon->_mx;
			mon->_menemyy = tmon->_my;
			if (tmon->_msquelch == 0) {
				tmon->_msquelch = UCHAR_MAX;
				tmon->_lastx = mon->_mx;
				tmon->_lasty = mon->_my;
				for (j = -2; j <= 2; j++) {
					for (k = -2; k <= 2; k++) {
						md = dMonster[mon->_mx + k][mon->_my + j];
						if (md > 0)
							monster[md - 1]._msquelch = UCHAR_MAX;
					}
				}

			}
			MonStartAttack(mnum);
			return;
		}
	}

	mon->_mpathcount++;
	if (mon->_mpathcount > 8)
		mon->_mpathcount = 5;

	md = plr[mnum]._pdir;
	if (!MonCallWalk(mnum, md)) {
		for (j = 0; j < 8; j++) {
			md = (md + 1) & 7;
			if (DirOK(mnum, md)) {
				MonWalkDir(mnum, md);
				break;
			}
		}
	}
}

void MAI_SkelKing(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy, nx, ny;
	int dist, v, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelKing: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		mx = mon->_mx;
		my = mon->_my;
		md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(126, 100);
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		dist = std::max(abs(mx - fx), abs(my - fy));
		if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || dist >= 3 && random_(127, 4) == 0) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(128, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md) || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
					MonStartDelay(mnum, RandRange(10, 19));
				}
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (gbMaxPlayers == 1
			    && (dist >= 3 && v < 4 * mon->_mint + 35 || v < 6)
			    && LineClear(mon->_mx, mon->_my, fx, fy)) {
				nx = mon->_mx + offset_x[md];
				ny = mon->_my + offset_y[md];
				if (PosOkMonst(mnum, nx, ny) && nummonsters < MAXMONSTERS) {
					MonSpawnSkel(nx, ny, md);
					MonStartSpStand(mnum, md);
				}
			} else {
				if (dist >= 2) {
					v = random_(129, 100);
					if (v >= mon->_mint + 25
					    && (mon->_mVar1 != MM_WALK && mon->_mVar1 != MM_WALK2 && mon->_mVar1 != MM_WALK3 || mon->_mVar2 != 0 || (v >= mon->_mint + 75))) {
						MonStartDelay(mnum, RandRange(10, 19));
					} else {
						MonCallWalk(mnum, md);
					}
				} else if (v < mon->_mint + 20) {
					mon->_mdir = md;
					MonStartAttack(mnum);
				}
			}
		}
	}
}

void MAI_Rhino(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int v, dist, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Rhino: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		mx = mon->_mx;
		my = mon->_my;
		md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(131, 100);
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		dist = std::max(abs(mx - fx), abs(my - fy));
		if (dist >= 2) {
			if (mon->_mgoal == MGOAL_MOVE || dist >= 5 && random_(132, 4) != 0) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(133, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (mon->_mgoalvar1++ >= 2 * dist || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
					MonStartDelay(mnum, RandRange(10, 19));
				}
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (dist >= 5
			    && v < 2 * mon->_mint + 43
			    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
				if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, 1, mnum, 0, 0, 0) != -1) {
					if (mon->MData->snd_special)
						PlayEffect(mnum, 3);
					mon->_mmode = MM_CHARGE;
					dMonster[mon->_mx][mon->_my] = -(mnum + 1);
				}
			} else {
				if (dist >= 2) {
					v = random_(134, 100);
					if (v >= 2 * mon->_mint + 33
					    && (mon->_mVar1 != MM_WALK && mon->_mVar1 != MM_WALK2 && mon->_mVar1 != MM_WALK3
					           || mon->_mVar2 != 0
					           || v >= 2 * mon->_mint + 83)) {
						MonStartDelay(mnum, RandRange(10, 19));
					} else {
						MonCallWalk(mnum, md);
					}
				} else if (v < 2 * mon->_mint + 28) {
					mon->_mdir = md;
					MonStartAttack(mnum);
				}
			}
		}
	}
}

#ifdef HELLFIRE
void MAI_Horkdemon(int mnum)
{
	MonsterStruct *mon;
	int fx, fy, mx, my, md, v, dist;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Horkdemon: Invalid monster %d", mnum);
	}

	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	if (mon->_msquelch < UCHAR_MAX) {
		MonstCheckDoors(mnum);
	}
	v = random_(131, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoalvar1 = 0;
			mon->_mgoalvar2 = random_(133, 2);
		}
		mon->_mgoal = MGOAL_MOVE;
		if (mon->_mgoalvar1++ >= 2 * dist || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
			MonStartDelay(mnum, RandRange(10, 19));
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist >= 3 && v < 2 * mon->_mint + 43) {
			if (PosOkMonst(mnum, mon->_mx + offset_x[mon->_mdir], mon->_my + offset_y[mon->_mdir]) && nummonsters < MAXMONSTERS) {
				MonStartRSpAttack(mnum, MIS_HORKDMN);
			}
		} else if (dist < 2) {
			if (v < 2 * mon->_mint + 28) {
				mon->_mdir = md;
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mint + 33
			    || (mon->_mVar1 == 1 || mon->_mVar1 == 2 || mon->_mVar1 == 3) && mon->_mVar2 == 0 && v < 2 * mon->_mint + 83) {
				MonCallWalk(mnum, md);
			} else {
				MonStartDelay(mnum, RandRange(10, 19));
			}
		}
	}
}
#endif

void MAI_Counselor(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int dist, md, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Counselor: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(121, 100);
		if (mon->_mgoal == MGOAL_RETREAT) {
			if (mon->_mgoalvar1++ <= 3)
				MonCallWalk(mnum, opposite[md]);
			else {
				mon->_mgoal = MGOAL_NORMAL;
				MonStartFadein(mnum, md, TRUE);
			}
		} else if (mon->_mgoal == MGOAL_MOVE) {
			dist = std::max(abs(mx), abs(my));
			if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
				if (mon->_mgoalvar1++ < 2 * dist || !DirOK(mnum, md)) {
					MonRoundWalk(mnum, md, &mon->_mgoalvar2);
				} else {
					mon->_mgoal = MGOAL_NORMAL;
					MonStartFadein(mnum, md, TRUE);
				}
			} else {
				mon->_mgoal = MGOAL_NORMAL;
				MonStartFadein(mnum, md, TRUE);
			}
		} else if (mon->_mgoal == MGOAL_NORMAL) {
			if (abs(mx) >= 2 || abs(my) >= 2) {
				if (v < 5 * (mon->_mint + 10) && LineClear(mon->_mx, mon->_my, fx, fy)) {
					MonStartRAttack(mnum, counsmiss[mon->_mint]);
				} else if (random_(124, 100) < 30) {
					mon->_mgoal = MGOAL_MOVE;
					mon->_mgoalvar1 = 0;
					MonStartFadeout(mnum, md, FALSE);
				} else
					MonStartDelay(mnum, random_(105, 10) + 2 * (5 - mon->_mint));
			} else {
				mon->_mdir = md;
				if (mon->_mhitpoints < (mon->_mmaxhp >> 1)) {
					mon->_mgoal = MGOAL_RETREAT;
					mon->_mgoalvar1 = 0;
					MonStartFadeout(mnum, md, FALSE);
				} else if (mon->_mVar1 == MM_DELAY
				    || random_(105, 100) < 2 * mon->_mint + 20) {
					MonStartRAttack(mnum, MIS_FLASH);
				} else
					MonStartDelay(mnum, random_(105, 10) + 2 * (5 - mon->_mint));
			}
		}
		if (mon->_mmode == MM_STAND) {
			MonStartDelay(mnum, RandRange(5, 14));
		}
	}
}

void MAI_Garbud(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Garbud: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	md = MonGetDir(mnum);
	if (mon->_mgoal == MGOAL_TALKING) {
		if ((dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE)) {
			if (mon->mtalkmsg == TEXT_GARBUD4 && !effect_is_playing(USFX_GARBUD4)) {
				mon->_mgoal = MGOAL_NORMAL;
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
			}
		} else {
			if (mon->mtalkmsg < TEXT_GARBUD4 && mon->mtalkmsg >= TEXT_GARBUD1) {
				mon->_mgoal = MGOAL_INQUIRING;
				mon->mtalkmsg++;
			}
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_MOVE)
		MAI_Round(mnum, TRUE);

	mon->_mdir = md;
}

void MAI_Zhar(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Zhar: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	md = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_TALKING) {
		if (dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) {
			if (mon->mtalkmsg == TEXT_ZHAR2 && !effect_is_playing(USFX_ZHAR2)) {
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
				mon->_mgoal = MGOAL_NORMAL;
			}
		} else {
			if (mon->mtalkmsg == TEXT_ZHAR1) {
				mon->mtalkmsg = TEXT_ZHAR2;
				mon->_mgoal = MGOAL_INQUIRING;
			}
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT || mon->_mgoal == MGOAL_MOVE)
		MAI_Counselor(mnum);

	mon->_mdir = md;
}

void MAI_SnotSpil(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SnotSpil: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	md = MonGetDir(mnum);

	if (mon->mtalkmsg == TEXT_BANNER10 && !(dFlags[mx][my] & BFLAG_VISIBLE) && mon->_mgoal == MGOAL_TALKING) {
		mon->mtalkmsg = TEXT_BANNER11;
		mon->_mgoal = MGOAL_INQUIRING;
	}

	if (mon->mtalkmsg == TEXT_BANNER11 && quests[Q_LTBANNER]._qvar1 == 3) {
		mon->mtalkmsg = 0;
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (dFlags[mx][my] & BFLAG_VISIBLE) {
		if (mon->mtalkmsg == TEXT_BANNER12) {
			if (mon->_mgoal == MGOAL_TALKING && !effect_is_playing(USFX_SNOT3)) {
				ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1);
				quests[Q_LTBANNER]._qvar1 = 3;
				RedoPlayerVision();
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
		if (quests[Q_LTBANNER]._qvar1 == 3) {
			if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_ATTACK2)
				MAI_Fallen(mnum);
		}
	}

	mon->_mdir = md;
}

void MAI_Lazurus(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lazurus: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	md = MonGetDir(mnum);
	if ((dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) && mon->mtalkmsg == TEXT_VILE13) {
		if (gbMaxPlayers == 1) {
			if (mon->_mgoal == MGOAL_INQUIRING && plr[myplr]._px == DBORDERX + 19 && plr[myplr]._py == DBORDERY + 30) {
				PlayInGameMovie("gendata\\fprst3.smk");
				mon->_mmode = MM_TALK;
				quests[Q_BETRAYER]._qvar1 = 5;
			} else if (mon->_mgoal == MGOAL_TALKING && !effect_is_playing(USFX_LAZ1)) {
				ObjChangeMapResync(1, 18, 20, 24);
				RedoPlayerVision();
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
				quests[Q_BETRAYER]._qvar1 = 6;
				mon->_mgoal = MGOAL_NORMAL;
			}
		} else {
			if (mon->_mgoal == MGOAL_INQUIRING && quests[Q_BETRAYER]._qvar1 <= 3)
				mon->_mmode = MM_TALK;
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT || mon->_mgoal == MGOAL_MOVE) {
		if (gbMaxPlayers == 1 && quests[Q_BETRAYER]._qvar1 == 4 && mon->mtalkmsg == 0) { // Fix save games affected by teleport bug
			ObjChangeMapResync(1, 18, 20, 24);
			RedoPlayerVision();
			quests[Q_BETRAYER]._qvar1 = 6;
		}
#ifndef HELLFIRE
		mon->mtalkmsg = 0;
#endif
		MAI_Counselor(mnum);
	}

	mon->_mdir = md;
}

void MAI_Lazhelp(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lazhelp: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND)
		return;

	md = MonGetDir(mnum);

	if (dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) {
		if (gbMaxPlayers == 1) {
			if (quests[Q_BETRAYER]._qvar1 <= 5) {
				mon->_mgoal = MGOAL_INQUIRING;
			} else {
				mon->mtalkmsg = 0;
				mon->_mgoal = MGOAL_NORMAL;
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
	}
	if (mon->_mgoal == MGOAL_NORMAL)
		MAI_HlSpwn(mnum);
	mon->_mdir = md;
}

void MAI_Lachdanan(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lachdanan: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	md = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_TALKING) {
		if (dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) {
			if (mon->mtalkmsg == TEXT_VEIL11 && !effect_is_playing(USFX_LACH3)) {
				mon->mtalkmsg = 0;
				quests[Q_VEIL]._qactive = QUEST_DONE;
				MonStartKill(mnum, -1);
			}
		} else {
			if (mon->mtalkmsg == TEXT_VEIL9) {
				mon->mtalkmsg = TEXT_VEIL10;
				mon->_mgoal = MGOAL_INQUIRING;
			}
		}
	}

	mon->_mdir = md;
}

void MAI_Warlord(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Warlord: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	md = MonGetDir(mnum);
	if ((dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) && mon->mtalkmsg == TEXT_WARLRD9) {
		if (mon->_mgoal == MGOAL_INQUIRING)
			mon->_mmode = MM_TALK;
		else if (mon->_mgoal == MGOAL_TALKING && !effect_is_playing(USFX_WARLRD1)) {
			mon->_msquelch = UCHAR_MAX;
			mon->mtalkmsg = 0;
			mon->_mgoal = MGOAL_NORMAL;
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL)
		MAI_SkelSd(mnum);

	mon->_mdir = md;
}

void DeleteMonsterList()
{
	int i;

	for (i = 0; i < MAX_MINIONS; i++) {
		if (monster[i]._mDelFlag) {
			monster[i]._mx = 0;
			monster[i]._my = 0;
			monster[i]._mfutx = 0;
			monster[i]._mfuty = 0;
			monster[i]._moldx = 0;
			monster[i]._moldy = 0;
			monster[i]._mDelFlag = FALSE;
			assert(MINION_NR_INACTIVE(i));
		}
	}

	for (i = MAX_MINIONS; i < nummonsters; ) {
		if (monster[monstactive[i]]._mDelFlag) {
			DeleteMonster(i);
		} else {
			i++;
		}
	}
}

void ProcessMonsters()
{
	int i, mnum, mx, my, _menemy;
	BOOL raflag;
	MonsterStruct *mon;

	DeleteMonsterList();

	assert((DWORD)nummonsters <= MAXMONSTERS);
	for (i = 0; i < nummonsters; i++) {
		if (i < MAX_MINIONS && MINION_NR_INACTIVE(i))
			continue;
		mnum = monstactive[i];
		mon = &monster[mnum];
		if (gbMaxPlayers != 1) {
			SetRndSeed(mon->_mAISeed);
			mon->_mAISeed = GetRndSeed();
		}
		if (mon->_mhitpoints < mon->_mmaxhp && mon->_mhitpoints >= (1 << 6) && !(mon->_mFlags & MFLAG_NOHEAL)) {
			mon->_mhitpoints += (mon->mLevel + 1) >> 1;
			if (mon->_mhitpoints > mon->_mmaxhp)
				mon->_mhitpoints = mon->_mmaxhp;
		}
		mx = mon->_mx;
		my = mon->_my;

		if (dFlags[mx][my] & BFLAG_VISIBLE && mon->_msquelch == 0) {
			if (mon->_mType == MT_CLEAVER)
				PlaySfxLoc(USFX_CLEAVER, mx, my);
#ifdef HELLFIRE
			else if (mon->_mType == MT_NAKRUL)
				PlaySfxLoc(UseCowFarmer ? USFX_NAKRUL6 : (IsUberRoomOpened ? USFX_NAKRUL4 : USFX_NAKRUL5), mx, my);
			else if (mon->_mType == MT_DEFILER)
				PlaySfxLoc(USFX_DEFILER8, mx, my);
			MonEnemy(mnum);
#endif
		}

		_menemy = mon->_menemy;
		if (mon->_mFlags & MFLAG_TARGETS_MONSTER) {
			if ((DWORD)_menemy >= MAXMONSTERS) {
				dev_fatal("Illegal enemy monster %d for monster \"%s\"", _menemy, mon->mName);
			}
			mon->_lastx = mon->_menemyx = monster[_menemy]._mfutx;
			mon->_lasty = mon->_menemyy = monster[_menemy]._mfuty;
		} else {
			if ((DWORD)_menemy >= MAX_PLRS) {
				dev_fatal("Illegal enemy player %d for monster \"%s\"", _menemy, mon->mName);
			}
			mon->_menemyx = plr[_menemy]._pfutx;
			mon->_menemyy = plr[_menemy]._pfuty;
			if (dFlags[mx][my] & BFLAG_VISIBLE) {
				mon->_msquelch = UCHAR_MAX;
				mon->_lastx = mon->_menemyx;
				mon->_lasty = mon->_menemyy;
			} else if (mon->_msquelch != 0 && mon->_mAi != MT_DIABLO) { /// BUGFIX: change '_mAi' to '_mType'
				mon->_msquelch--;
			}
		}
		while (TRUE) {
			if (!(mon->_mFlags & MFLAG_SEARCH)) {
				AiProc[mon->_mAi](mnum);
			} else if (!MAI_Path(mnum)) {
				AiProc[mon->_mAi](mnum);
			}
			switch (mon->_mmode) {
			case MM_STAND:
				raflag = MonDoStand(mnum);
				break;
			case MM_WALK:
				raflag = MonDoWalk(mnum);
				break;
			case MM_WALK2:
				raflag = MonDoWalk2(mnum);
				break;
			case MM_WALK3:
				raflag = MonDoWalk3(mnum);
				break;
			case MM_ATTACK:
				raflag = MonDoAttack(mnum);
				break;
			case MM_GOTHIT:
				raflag = MonDoGotHit(mnum);
				break;
			case MM_DEATH:
				raflag = MonDoDeath(mnum);
				break;
			case MM_SATTACK:
				raflag = MonDoSAttack(mnum);
				break;
			case MM_FADEIN:
				raflag = MonDoFadein(mnum);
				break;
			case MM_FADEOUT:
				raflag = MonDoFadeout(mnum);
				break;
			case MM_RATTACK:
				raflag = MonDoRAttack(mnum);
				break;
			case MM_SPSTAND:
				raflag = MonDoSpStand(mnum);
				break;
			case MM_RSPATTACK:
				raflag = MonDoRSpAttack(mnum);
				break;
			case MM_DELAY:
				raflag = MonDoDelay(mnum);
				break;
			case MM_CHARGE:
				raflag = FALSE;
				break;
			case MM_STONE:
				raflag = MonDoStone(mnum);
				break;
			case MM_HEAL:
				raflag = MonDoHeal(mnum);
				break;
			case MM_TALK:
				raflag = MonDoTalk(mnum);
				break;
			default:
				ASSUME_UNREACHABLE
				raflag = FALSE;
				break;
			}
			if (raflag) {
				GroupUnity(mnum);
				continue;
			}
			break;
		}
		if (mon->_mmode != MM_STONE && !(mon->_mFlags & MFLAG_LOCK_ANIMATION)) {
			mon->_mAnimCnt++;
			if (mon->_mAnimCnt >= mon->_mAnimDelay) {
				mon->_mAnimCnt = 0;
				if (mon->_mFlags & MFLAG_REV_ANIMATION) {
					mon->_mAnimFrame--;
					if (mon->_mAnimFrame == 0) {
						mon->_mAnimFrame = mon->_mAnimLen;
					}
				} else {
					mon->_mAnimFrame++;
					if (mon->_mAnimFrame > mon->_mAnimLen) {
						mon->_mAnimFrame = 1;
					}
				}
			}
		}
	}

	DeleteMonsterList();
}

void FreeMonsters()
{
	int mtype;
	int i, j;

	for (i = 0; i < nummtypes; i++) {
		mtype = Monsters[i].mtype;
		for (j = 0; j < lengthof(animletter); j++) {
			if (animletter[j] != 's' || monsterdata[mtype].has_special) {
				MemFreeDbg(Monsters[i].Anims[j].CMem);
			}
		}
	}

	FreeMissiles2();
}

BOOL DirOK(int mnum, int mdir)
{
	int fx, fy;
	int x, y;
	int mcount, ma;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("DirOK: Invalid monster %d", mnum);
	}
	fx = monster[mnum]._mx + offset_x[mdir];
	fy = monster[mnum]._my + offset_y[mdir];
	if (!IN_DUNGEON_AREA(fx, fy) || !PosOkMonst(mnum, fx, fy))
		return FALSE;
	if (mdir == DIR_E) {
		if (nSolidTable[dPiece[fx][fy + 1]] || dFlags[fx][fy + 1] & BFLAG_MONSTLR)
			return FALSE;
	} else if (mdir == DIR_W) {
		if (nSolidTable[dPiece[fx + 1][fy]] || dFlags[fx + 1][fy] & BFLAG_MONSTLR)
			return FALSE;
	} else if (mdir == DIR_N) {
		if (nSolidTable[dPiece[fx + 1][fy]] || nSolidTable[dPiece[fx][fy + 1]])
			return FALSE;
	} else if (mdir == DIR_S)
		if (nSolidTable[dPiece[fx - 1][fy]] || nSolidTable[dPiece[fx][fy - 1]])
			return FALSE;
	if (monster[mnum].leaderflag == MLEADER_PRESENT) {
		if (abs(fx - monster[monster[mnum].leader]._mfutx) >= 4
		    || abs(fy - monster[monster[mnum].leader]._mfuty) >= 4) {
			return FALSE;
		}
		return TRUE;
	}
	if (monster[mnum]._uniqtype == 0 || !(UniqMonst[monster[mnum]._uniqtype - 1].mUnqAttr & 2))
		return TRUE;
	mcount = 0;
	for (x = fx - 3; x <= fx + 3; x++) {
		for (y = fy - 3; y <= fy + 3; y++) {
			if (IN_DUNGEON_AREA(x, y)) {
				ma = dMonster[x][y];
				if (ma == 0)
					continue;
				ma = ma >= 0 ? ma - 1 : -(ma + 1);
				if (monster[ma].leaderflag == MLEADER_PRESENT
				    && monster[ma].leader == mnum
				    && monster[ma]._mfutx == x
					&& monster[ma]._mfuty == y) {
					mcount++;
				}
			}
		}
	}
	return mcount == monster[mnum].packsize;
}

BOOL PosOkMissile(int x, int y)
{
	return !nMissileTable[dPiece[x][y]] && !(dFlags[x][y] & BFLAG_MONSTLR);
}

BOOL CheckNoSolid(int x, int y)
{
	return !nSolidTable[dPiece[x][y]];
}

/**
 * Walks from (x1; y1) to (x2; y2) and calls the Clear check for 
 * every position inbetween.
 * The target and source positions are NOT checked.
 * @returns TRUE if the Clear checks succeeded.
 */
BOOL LineClearF(BOOL (*Clear)(int, int), int x1, int y1, int x2, int y2)
{
	int dx, dy;
	int tmp, d, xyinc;

	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) >= abs(dy)) {
		if (dx == 0)
			return TRUE;
		// alway proceed from lower to higher x
		if (dx < 0) {
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			dx = -dx;
			dy = -dy;
		}
		// find out step size and direction on the y coordinate
		if (dy >= 0) {
			xyinc = 1;
		} else {
			dy = -dy;
			xyinc = -1;
		}
		// multiply by 2 so we round up
		dy *= 2;
		d = 0;
		do {
			d += dy;
			if (d >= dx) {
				d -= 2 * dx; // multiply by 2 to support rounding
				y1 += xyinc;
			}
			x1++;
			if (x1 == x2)
				return TRUE;
		} while (Clear(x1, y1));
	} else {
		if (dy < 0) {
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			dy = -dy;
			dx = -dx;
		}
		if (dx >= 0) {
			xyinc = 1;
		} else {
			dx = -dx;
			xyinc = -1;
		}
		dx *= 2;
		d = 0;
		do {
			d += dx;
			if (d >= dy) {
				d -= 2 * dy;
				x1 += xyinc;
			}
			y1++;
			if (y1 == y2)
				return TRUE;
		} while (Clear(x1, y1));
	}
	return FALSE;
}

BOOL LineClear(int x1, int y1, int x2, int y2)
{
	return LineClearF(PosOkMissile, x1, y1, x2, y2);
}

/**
 * Same as LineClearF, only with a different Clear function.
 */
BOOL LineClearF1(BOOL (*Clear)(int, int, int), int mnum, int x1, int y1, int x2, int y2)
{
	int dx, dy;
	int tmp, d, xyinc;

	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) >= abs(dy)) {
		if (dx == 0)
			return TRUE;
		// alway proceed from lower to higher x
		if (dx < 0) {
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			dx = -dx;
			dy = -dy;
		}
		// find out step size and direction on the y coordinate
		if (dy >= 0) {
			xyinc = 1;
		} else {
			dy = -dy;
			xyinc = -1;
		}
		// multiply by 2 so we round up
		dy *= 2;
		d = 0;
		do {
			d += dy;
			if (d >= dx) {
				d -= 2 * dx; // multiply by 2 to support rounding
				y1 += xyinc;
			}
			x1++;
			if (x1 == x2)
				return TRUE;
		} while (Clear(mnum, x1, y1));
	} else {
		if (dy < 0) {
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			dy = -dy;
			dx = -dx;
		}
		if (dx >= 0) {
			xyinc = 1;
		} else {
			dx = -dx;
			xyinc = -1;
		}
		dx *= 2;
		d = 0;
		do {
			d += dx;
			if (d >= dy) {
				d -= 2 * dy;
				x1 += xyinc;
			}
			y1++;
			if (y1 == y2)
				return TRUE;
		} while (Clear(mnum, x1, y1));
	}
	return FALSE;
}

void SyncMonsterAnim(int mnum)
{
	const MonsterData *MData;
	MonsterStruct *mon;
	MON_ANIM anim;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("SyncMonsterAnim: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if ((DWORD)mon->_mMTidx >= MAX_LVLMTYPES) {
		dev_fatal("SyncMonsterAnim: Invalid monster type %d for %d", mon->_mMTidx, mnum);
	}
	mon->MType = &Monsters[mon->_mMTidx];
	mon->_mType = mon->MType->mtype;
	mon->_mAnims = mon->MType->Anims;
	mon->_mAnimWidth = mon->MType->width;
	mon->_mAnimWidth2 = mon->MType->width2;
	MData = mon->MType->MData;
	if (MData == NULL) {
		dev_fatal("SyncMonsterAnim: Monster %d \"%s\" MData NULL", mon->_mMTidx, mon->mName);
	}
	mon->MData = MData;
	if (mon->_uniqtype != 0)
		mon->mName = UniqMonst[mon->_uniqtype - 1].mName;
	else
		mon->mName = MData->mName;

	switch (mon->_mmode) {
	case MM_WALK:
	case MM_WALK2:
	case MM_WALK3:
		anim = MA_WALK;
		break;
	case MM_ATTACK:
	case MM_RATTACK:
		anim = MA_ATTACK;
		break;
	case MM_GOTHIT:
		anim = MA_GOTHIT;
		break;
	case MM_DEATH:
		anim = MA_DEATH;
		break;
	case MM_SATTACK:
	case MM_FADEIN:
	case MM_FADEOUT:
	case MM_SPSTAND:
	case MM_RSPATTACK:
	case MM_HEAL:
		anim = MA_SPECIAL;
		break;
	case MM_STAND:
	case MM_DELAY:
	case MM_TALK:
		anim = MA_STAND;
		break;
	case MM_CHARGE:
		anim = MA_ATTACK;
		mon->_mAnimFrame = 1;
		mon->_mAnimLen = mon->_mAnims[MA_ATTACK].Frames;
		break;
	default:
		anim = MA_STAND;
		mon->_mAnimFrame = 1;
		mon->_mAnimLen = mon->_mAnims[MA_STAND].Frames;
		break;
	}
	mon->_mAnimData = mon->_mAnims[anim].Data[mon->_mdir];
}

void MonFallenFear(int x, int y)
{
	MonsterStruct *mon;
	int i, rundist;

	for (i = 0; i < nummonsters; i++) {
		mon = &monster[monstactive[i]];
		switch (mon->_mType) {
		case MT_RFALLSP:
		case MT_RFALLSD:
			rundist = 7;
			break;
		case MT_DFALLSP:
		case MT_DFALLSD:
			rundist = 5;
			break;
		case MT_YFALLSP:
		case MT_YFALLSD:
			rundist = 3;
			break;
		case MT_BFALLSP:
		case MT_BFALLSD:
			rundist = 2;
			break;
		default:
			continue;
		}
		if (mon->_mAi == AI_FALLEN
		 && abs(x - mon->_mx) < 5
		 && abs(y - mon->_my) < 5
		 && mon->_mhitpoints >= (1 << 6)) {
			mon->_mgoal = MGOAL_RETREAT;
			mon->_mgoalvar1 = rundist;
			mon->_mdir = GetDirection(x, y, mon->_mx, mon->_my);
		}
	}
}

void MissToMonst(int mi, int x, int y)
{
	int oldx, oldy;
	int newx, newy;
	int mnum, tnum;
	MissileStruct *mis;
	MonsterStruct *mon;

	if ((DWORD)mi >= MAXMISSILES) {
		dev_fatal("MissToMonst: Invalid missile %d", mi);
	}
	mis = &missile[mi];
	mnum = mis->_miSource;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("MissToMonst: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	oldx = mis->_mix;
	oldy = mis->_miy;
	dMonster[x][y] = mnum + 1;
	mon->_mdir = mis->_miDir;
	mon->_mx = x;
	mon->_my = y;
	MonStartStand(mnum, mon->_mdir);
	if (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN) {
		if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER))
			MonStartHit(mnum, -1, 0);
		else
			M2MStartHit(mnum, -1, 0);
	} else {
		MonStartFadein(mnum, mon->_mdir, FALSE);
	}

	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER)) {
		tnum = dPlayer[oldx][oldy];
		if (tnum > 0) {
			tnum--;
			if (mon->_mType != MT_GLOOM && (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN)) {
				MonTryH2HHit(mnum, tnum, 500, mon->mMinDamage2, mon->mMaxDamage2);
				if (tnum == dPlayer[oldx][oldy] - 1 && (mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE)) {
					if (plr[tnum]._pmode != PM_GOTHIT && plr[tnum]._pmode != PM_DEATH)
						StartPlrHit(tnum, 0, TRUE);
					newx = oldx + offset_x[mon->_mdir];
					newy = oldy + offset_y[mon->_mdir];
					if (PosOkPlayer(tnum, newx, newy)) {
						plr[tnum]._px = newx;
						plr[tnum]._py = newy;
						RemovePlrFromMap(tnum);
						dPlayer[newx][newy] = tnum + 1;
						FixPlayerLocation(tnum);
					}
				}
			}
		}
	} else {
		tnum = dMonster[oldx][oldy];
		if (tnum > 0) {
			tnum--;
			if (mon->_mType != MT_GLOOM && (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN)) {
				MonTryM2MHit(mnum, tnum, 500, mon->mMinDamage2, mon->mMaxDamage2);
				if (mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE) {
					newx = oldx + offset_x[mon->_mdir];
					newy = oldy + offset_y[mon->_mdir];
					if (PosOkMonst(dMonster[oldx][oldy] - 1, newx, newy)) {
						mnum = dMonster[oldx][oldy];
						dMonster[newx][newy] = mnum;
						dMonster[oldx][oldy] = 0;
						mnum--;
						monster[mnum]._mx = newx;
						monster[mnum]._mfutx = newx;
						monster[mnum]._my = newy;
						monster[mnum]._mfuty = newy;
					}
				}
			}
		}
	}
}

static BOOL monster_posok(int mnum, int x, int y)
{
	MissileStruct* mis;
	BOOL ret = TRUE, fire = FALSE;
	int mi = dMissile[x][y], i;

	if (mi == 0 || mnum < 0)
		return TRUE;

#ifdef HELLFIRE
	BOOL lightning = FALSE;

	if (mi > 0) {
		if (missile[mi - 1]._miType == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			fire = TRUE;
		} else if (missile[mi - 1]._miType == MIS_LIGHTWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			lightning = TRUE;
		}
	} else {
		for (i = 0; i < nummissiles; i++) {
			mis = &missile[missileactive[i]];
			if (mis->_mix == x && mis->_miy == y) {
				if (mis->_miType == MIS_FIREWALL) {
					fire = TRUE;
				} else if (mis->_miType == MIS_LIGHTWALL) {
					lightning = TRUE;
				}
			}
		}
	}
	if (fire && (monster[mnum].mMagicRes & MORS_FIRE_IMMUNE) != MORS_FIRE_IMMUNE)
		ret = FALSE;
	if (lightning && (monster[mnum].mMagicRes & MORS_LIGHTNING_IMMUNE) != MORS_LIGHTNING_IMMUNE)
		ret = FALSE;
#else
	if (mi > 0) {
		if (missile[mi - 1]._miType == MIS_FIREWALL) // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			fire = TRUE;
	} else {
		for (i = 0; i < nummissiles; i++) {
			mis = &missile[missileactive[i]];
			if (mis->_mix == x && mis->_miy == y
			 && mis->_miType == MIS_FIREWALL)
				fire = TRUE;
		}
	}
	if (fire && (monster[mnum].mMagicRes & MORS_FIRE_IMMUNE) != MORS_FIRE_IMMUNE)
		ret = FALSE;
#endif
	return ret;
}

BOOL PosOkMonst(int mnum, int x, int y)
{
	int oi;

	if ((nSolidTable[dPiece[x][y]] | dPlayer[x][y] | dMonster[x][y]) != 0)
		return FALSE;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return FALSE;
	}

	return monster_posok(mnum, x, y);
}

BOOL PosOkMonst2(int mnum, int x, int y)
{
	int oi;

	if (nSolidTable[dPiece[x][y]])
		return FALSE;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return FALSE;
	}

	return monster_posok(mnum, x, y);
}

BOOL PosOkMonst3(int mnum, int x, int y)
{
	BOOL isdoor;
	int oi, objtype;

	isdoor = FALSE;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		objtype = object[oi]._otype;
		isdoor = objtype == OBJ_L1LDOOR || objtype == OBJ_L1RDOOR
		    || objtype == OBJ_L2LDOOR || objtype == OBJ_L2RDOOR
		    || objtype == OBJ_L3LDOOR || objtype == OBJ_L3RDOOR;
		if (object[oi]._oSolidFlag && !isdoor) {
			return FALSE;
		}
	}

	if ((nSolidTable[dPiece[x][y]] && !isdoor) || (dPlayer[x][y] | dMonster[x][y]) != 0)
		return FALSE;

	return monster_posok(mnum, x, y);
}

BOOL IsSkel(int mt)
{
	return mt >= MT_WSKELAX && mt <= MT_XSKELAX
	    || mt >= MT_WSKELBW && mt <= MT_XSKELBW
	    || mt >= MT_WSKELSD && mt <= MT_XSKELSD;
}

BOOL IsGoat(int mt)
{
	return mt >= MT_NGOATMC && mt <= MT_GGOATMC
	    || mt >= MT_NGOATBW && mt <= MT_GGOATBW;
}

int MonSpawnSkel(int x, int y, int dir)
{
	int i, j, skeltypes, skel;

	j = 0;
	for (i = 0; i < nummtypes; i++) {
		if (IsSkel(Monsters[i].mtype))
			j++;
	}

	if (j != 0) {
		skeltypes = random_(136, j);
		j = 0;
		for (i = 0; i < nummtypes && j <= skeltypes; i++) {
			if (IsSkel(Monsters[i].mtype))
				j++;
		}
		skel = AddMonster(x, y, dir, i - 1, TRUE);
		if (skel != -1)
			MonStartSpStand(skel, dir);

		return skel;
	}

	return -1;
}

static void ActivateSpawn(int mnum, int x, int y, int dir)
{
	dMonster[x][y] = mnum + 1;
	monster[mnum]._mx = x;
	monster[mnum]._my = y;
	monster[mnum]._mfutx = x;
	monster[mnum]._mfuty = y;
	monster[mnum]._moldx = x;
	monster[mnum]._moldy = y;
	MonStartSpStand(mnum, dir);
}

BOOL SpawnSkeleton(int mnum, int x, int y)
{
	int dx, dy, xx, yy, dir, j, k, rs;
	BOOL savail;
	int monstok[3][3];

	if (mnum == -1)
		return FALSE;

	if (PosOkMonst(-1, x, y)) {
		dir = GetDirection(x, y, x, y);
		ActivateSpawn(mnum, x, y, dir);
		return TRUE;
	}

	savail = FALSE;
	yy = 0;
	for (j = y - 1; j <= y + 1; j++) {
		xx = 0;
		for (k = x - 1; k <= x + 1; k++) {
			monstok[xx][yy] = PosOkMonst(-1, k, j);
			savail |= monstok[xx][yy];
			xx++;
		}
		yy++;
	}
	if (!savail) {
		return FALSE;
	}

	rs = RandRange(1, 15);
	xx = 0;
	yy = 0;
	while (rs > 0) {
		if (monstok[xx][yy])
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

	dx = x - 1 + xx;
	dy = y - 1 + yy;
	dir = GetDirection(dx, dy, x, y);
	ActivateSpawn(mnum, dx, dy, dir);

	return TRUE;
}

int PreSpawnSkeleton()
{
	int i, j, skeltypes, skel;

	j = 0;

	for (i = 0; i < nummtypes; i++) {
		if (IsSkel(Monsters[i].mtype))
			j++;
	}

	if (j != 0) {
		skeltypes = random_(136, j);
		j = 0;
		for (i = 0; i < nummtypes && j <= skeltypes; i++) {
			if (IsSkel(Monsters[i].mtype))
				j++;
		}
		skel = AddMonster(0, 0, 0, i - 1, FALSE);
		if (skel != -1)
			MonStartStand(skel, 0);

		return skel;
	}

	return -1;
}

void TalktoMonster(int mnum)
{
	MonsterStruct *mon;
	int pnum, iv;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("TalktoMonster: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mmode = MM_TALK;
	if (mon->_mAi == AI_SNOTSPIL || mon->_mAi == AI_LACHDAN) {
		pnum = mon->_menemy;
		if (QuestStatus(Q_LTBANNER) && quests[Q_LTBANNER]._qvar1 == 2 && PlrHasItem(pnum, IDI_BANNER, &iv)) {
			RemoveInvItem(pnum, iv);
			quests[Q_LTBANNER]._qactive = QUEST_DONE;
			mon->mtalkmsg = TEXT_BANNER12;
			mon->_mgoal = MGOAL_INQUIRING;
		}
		if (QuestStatus(Q_VEIL) && mon->mtalkmsg >= TEXT_VEIL9) {
			if (PlrHasItem(pnum, IDI_GLDNELIX, &iv)) {
				RemoveInvItem(pnum, iv);
				mon->mtalkmsg = TEXT_VEIL11;
				mon->_mgoal = MGOAL_INQUIRING;
			}
		}
	}
}

void SpawnGolum(int mnum, int x, int y, int level)
{
	MonsterStruct *mon;

	static_assert(MAX_MINIONS == MAX_PLRS, "SpawnGolum requires that owner of a monster has the same id as the monster itself.");
	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("SpawnGolum: Invalid monster %d", mnum);
	}
	dMonster[x][y] = mnum + 1;
	mon = &monster[mnum];
	mon->_mx = x;
	mon->_my = y;
	mon->_mfutx = x;
	mon->_mfuty = y;
	mon->_moldx = x;
	mon->_moldy = y;
	mon->_mpathcount = 0;
	mon->mArmorClass = 25;
	mon->_mmaxhp = 2 * (320 * level + plr[mnum]._pMaxMana / 3);
	mon->_mhitpoints = mon->_mmaxhp;
	mon->mHit = 5 * (level + 8) + 2 * plr[mnum]._pLevel;
	mon->mMinDamage = 2 * (level + 4);
	mon->mMaxDamage = 2 * (level + 8);
	MonStartSpStand(mnum, 0);
	MonEnemy(mnum);
	if (mnum == myplr) {
		NetSendCmdGolem(
		    mon->_mx,
		    mon->_my,
		    mon->_mdir,
		    mon->_menemy,
		    mon->_mhitpoints,
		    currlevel);
	}
}

BOOL CanTalkToMonst(int mnum)
{
	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("CanTalkToMonst: Invalid monster %d", mnum);
	}
	assert((monster[mnum]._mgoal != MGOAL_INQUIRING
		&& monster[mnum]._mgoal != MGOAL_TALKING)
		|| monster[mnum].mtalkmsg != 0);
	return monster[mnum]._mgoal == MGOAL_INQUIRING
		|| monster[mnum]._mgoal == MGOAL_TALKING;
}

BOOL CheckMonsterHit(int mnum, BOOL *ret)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		dev_fatal("CheckMonsterHit: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];

	if (mon->mtalkmsg != 0 || mon->_mmode == MM_CHARGE || mon->_mhitpoints < (1 << 6)
	 || (mon->_mAi == AI_SNEAK && mon->_mgoal == MGOAL_RETREAT)
	 || (mon->_mAi == AI_COUNSLR && mon->_mgoal != MGOAL_NORMAL)) {
		*ret = FALSE;
		return TRUE;
	}

	if (mon->_mAi == AI_GARG && mon->_mFlags & MFLAG_GARG_STONE) {
		mon->_mFlags &= ~(MFLAG_GARG_STONE | MFLAG_LOCK_ANIMATION);
		// mon->_mmode = MM_SATTACK;
		*ret = TRUE;
		return TRUE;
	}

	return FALSE;
}

int encode_enemy(int mnum)
{
	if (monster[mnum]._mFlags & MFLAG_TARGETS_MONSTER)
		return monster[mnum]._menemy + MAX_PLRS;
	else
		return monster[mnum]._menemy;
}

void decode_enemy(int mnum, int enemy)
{
	if (enemy < MAX_PLRS) {
		monster[mnum]._mFlags &= ~MFLAG_TARGETS_MONSTER;
		monster[mnum]._menemy = enemy;
		monster[mnum]._menemyx = plr[enemy]._pfutx;
		monster[mnum]._menemyy = plr[enemy]._pfuty;
	} else {
		monster[mnum]._mFlags |= MFLAG_TARGETS_MONSTER;
		enemy -= MAX_PLRS;
		monster[mnum]._menemy = enemy;
		monster[mnum]._menemyx = monster[enemy]._mfutx;
		monster[mnum]._menemyy = monster[enemy]._mfuty;
	}
}

DEVILUTION_END_NAMESPACE
