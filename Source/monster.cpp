/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */
#include "all.h"
#include "monstai.h"
#include "engine/render/cl2_render.hpp"

DEVILUTION_BEGIN_NAMESPACE

/** Tracks which missile files are already loaded */
int MissileFileFlag;

int monstactive[MAXMONSTERS];
int nummonsters;
MonsterStruct monster[MAXMONSTERS];
int totalmonsters;
CMonster Monsters[MAX_LVLMTYPES];
int monstimgtot;
int uniquetrans;
int nummtypes;

/** Light radius of unique monsters */
#define MON_LIGHTRAD 3

/** Maps from walking path step to facing direction. */
//const char walk2dir[9] = { 0, DIR_NE, DIR_NW, DIR_SE, DIR_SW, DIR_N, DIR_E, DIR_S, DIR_W };
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
const char animletter[NUM_MON_ANIM] = { 'n', 'w', 'a', 'h', 'd', 's' };
/** Maps from direction to a left turn from the direction. */
const int left[8] = { 7, 0, 1, 2, 3, 4, 5, 6 };
/** Maps from direction to a right turn from the direction. */
const int right[8] = { 1, 2, 3, 4, 5, 6, 7, 0 };
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

static inline void InitMonsterTRN(const CMonster *cmon, const MonsterData *mdata)
{
	BYTE *tf, *cf;
	int i, n, j;

	// A TRN file contains a sequence of color transitions, represented
	// as indexes into a palette. (a 256 byte array of palette indices)
	tf = cf = LoadFileInMem(mdata->TransFile);
	for (i = 0; i < 256; i++) {
		if (*cf == 255) {
			*cf = 0;
		}
		cf++;
	}

	n = mdata->has_special ? 6 : 5;
	for (i = 0; i < n; i++) {
		if (i != 1 || cmon->cmType < MT_COUNSLR || cmon->cmType > MT_ADVOCATE) {
			for (j = 0; j < lengthof(cmon->cmAnims[i].aData); j++) {
				Cl2ApplyTrans(
				    cmon->cmAnims[i].aData[j],
				    tf,
				    cmon->cmAnims[i].aFrames);
			}
		}
	}
	mem_free_dbg(tf);
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

static int AddMonsterType(int type, BOOL scatter)
{
	int i;

	for (i = 0; i < nummtypes && Monsters[i].cmType != type; i++)
		;

	if (i == nummtypes) {
		nummtypes++;
		Monsters[i].cmType = type;
		Monsters[i].cmPlaceScatter = FALSE;
		monstimgtot += monsterdata[type].mImage;
		InitMonsterGFX(i);
		InitMonsterSND(i);
	}

	Monsters[i].cmPlaceScatter |= scatter;
	return i;
}

void GetLevelMTypes()
{
	int i, mtype;
	int montypes[NUM_MTYPES];
	const LevelDataStruct* lds;
	BYTE lvl;

	int nt; // number of types

	AddMonsterType(MT_GOLEM, FALSE);
	lvl = currLvl._dLevelIdx;
	if (!currLvl._dSetLvl) {
		if (lvl == DLV_HELL4) {
			AddMonsterType(MT_ADVOCATE, TRUE);
			AddMonsterType(MT_RBLACK, TRUE);
			AddMonsterType(MT_DIABLO, FALSE);
			return;
		}

#ifdef HELLFIRE
		if (lvl == DLV_NEST2)
			AddMonsterType(MT_HORKSPWN, TRUE);
		if (lvl == DLV_NEST3) {
			AddMonsterType(MT_HORKSPWN, TRUE);
			AddMonsterType(MT_HORKDMN, FALSE);
		}
		if (lvl == DLV_NEST4)
			AddMonsterType(MT_DEFILER, FALSE);
		if (lvl == DLV_CRYPT4) {
			AddMonsterType(MT_ARCHLICH, TRUE);
			AddMonsterType(MT_NAKRUL, FALSE);
		}
#endif
		if (QuestStatus(Q_BUTCHER))
			AddMonsterType(MT_CLEAVER, FALSE);
		if (QuestStatus(Q_GARBUD))
			AddMonsterType(UniqMonst[UMT_GARBUD].mtype, FALSE);
		if (QuestStatus(Q_ZHAR))
			AddMonsterType(UniqMonst[UMT_ZHAR].mtype, FALSE);
		if (QuestStatus(Q_LTBANNER))
			AddMonsterType(UniqMonst[UMT_SNOTSPIL].mtype, FALSE);
		if (QuestStatus(Q_VEIL))
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, TRUE);
		if (QuestStatus(Q_WARLORD))
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, TRUE);

		lds = &AllLevels[currLvl._dLevelIdx];
		for (nt = 0; nt < lengthof(lds->dMonTypes); nt++) {
			mtype = lds->dMonTypes[nt];
			if (mtype == MT_INVALID)
				break;
			montypes[nt] = mtype;
		}

#ifdef _DEBUG
		if (monstdebug) {
			for (i = 0; i < debugmonsttypes; i++)
				AddMonsterType(DebugMonsters[i], TRUE);
			return;
		}
#endif
		while (monstimgtot < 4000 && nt > 0 && nummtypes < MAX_LVLMTYPES) {
			for (i = 0; i < nt; ) {
				if (monsterdata[montypes[i]].mImage > 4000 - monstimgtot) {
					montypes[i] = montypes[--nt];
					continue;
				}

				i++;
			}

			if (nt != 0) {
				i = random_(88, nt);
				AddMonsterType(montypes[i], TRUE);
				montypes[i] = montypes[--nt];
			}
		}
	} else {
		if (lvl == SL_SKELKING) {
			AddMonsterType(MT_SKING, FALSE);
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
	mtype = cmon->cmType;
	mdata = &monsterdata[mtype];

	// static_assert(lengthof(animletter) == lengthof(monsterdata[0].aFrames), "");
	for (anim = 0; anim < NUM_MON_ANIM; anim++) {
		if ((animletter[anim] != 's' || mdata->has_special) && mdata->mAnimFrames[anim] > 0) {
			snprintf(strBuff, sizeof(strBuff), mdata->GraphicType, animletter[anim]);

			celBuf = LoadFileInMem(strBuff);
			cmon->cmAnims[anim].aCelData = celBuf;

			if (mtype != MT_GOLEM || (animletter[anim] != 's' && animletter[anim] != 'd')) {
				for (i = 0; i < lengthof(cmon->cmAnims[anim].aData); i++) {
					cmon->cmAnims[anim].aData[i] = const_cast<BYTE *>(CelGetFrameStart(celBuf, i));
				}
			} else {
				for (i = 0; i < lengthof(cmon->cmAnims[anim].aData); i++) {
					cmon->cmAnims[anim].aData[i] = celBuf;
				}
			}
		}

		cmon->cmAnims[anim].aFrames = mdata->mAnimFrames[anim];
		cmon->cmAnims[anim].aFrameLen = mdata->mAnimFrameLen[anim];
	}

	cmon->cmWidth = mdata->width;
	cmon->cmXOffset = (mdata->width - 64) >> 1;
	cmon->cmData = mdata;

	if (mdata->has_trans) {
		InitMonsterTRN(cmon, mdata);
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
	mon->_mType = cmon->cmType;
	mon->_mAnimWidth = cmon->cmWidth;
	mon->_mAnimXOffset = cmon->cmXOffset;
	mon->MData = cmon->cmData;
	mon->mName = cmon->cmData->mName;
	mon->_mFlags = cmon->cmData->mFlags;
	mon->mLevel = cmon->cmData->mLevel;
	mon->_mSelFlag = cmon->cmData->mSelFlag;
	mon->_mAi = cmon->cmData->mAi;
	mon->_mint = cmon->cmData->mInt;
	mon->_mHit = cmon->cmData->mHit;
	mon->_mMagic = cmon->cmData->mMagic;
	mon->_mAFNum = cmon->cmData->mAFNum;
	mon->_mMinDamage = cmon->cmData->mMinDamage;
	mon->_mMaxDamage = cmon->cmData->mMaxDamage;
	mon->_mHit2 = cmon->cmData->mHit2;
	mon->_mMagic2 = cmon->cmData->mMagic2;
	mon->_mAFNum2 = cmon->cmData->mAFNum2;
	mon->_mMinDamage2 = cmon->cmData->mMinDamage2;
	mon->_mMaxDamage2 = cmon->cmData->mMaxDamage2;
	mon->_mArmorClass = cmon->cmData->mArmorClass;
	mon->_mEvasion = cmon->cmData->mEvasion;
	mon->mMagicRes = cmon->cmData->mMagicRes;
	mon->mExp = cmon->cmData->mExp;
	mon->_mmaxhp = RandRange(cmon->cmData->mMinHP, cmon->cmData->mMaxHP) << 6;
	mon->_mAnims = cmon->cmAnims;
	mon->_mAnimData = cmon->cmAnims[MA_STAND].aData[dir];
	mon->_mAnimFrameLen = cmon->cmAnims[MA_STAND].aFrameLen;
	mon->_mAnimCnt = random_(88, mon->_mAnimFrameLen);
	mon->_mAnimLen = cmon->cmAnims[MA_STAND].aFrames;
	mon->_mAnimFrame = RandRange(1, mon->_mAnimLen);
	mon->_mmode = MM_STAND;
	mon->_mVar1 = MM_STAND; // STAND_PREV_MODE
	mon->_mVar2 = 0;        // STAND_TICK
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
	mon->mtalkmsg = TEXT_NONE;

	mon->_uniqtype = 0;
	mon->_uniqtrans = 0;
	mon->_udeadval = 0;
	mon->mlid = 0;

	mon->leader = 0;
	mon->leaderflag = MLEADER_NONE;
	mon->packsize = 0;
	mon->falign_CB = 0;

	if (gnDifficulty == DIFF_NIGHTMARE) {
		mon->_mmaxhp = 2 * mon->_mmaxhp + (100 << 6);
		mon->mLevel += NIGHTMARE_LEVEL_BONUS;
		mon->mExp = 2 * (mon->mExp + DIFFICULTY_EXP_BONUS);
		mon->_mHit += NIGHTMARE_TO_HIT_BONUS;
		mon->_mMagic += NIGHTMARE_MAGIC_BONUS;
		mon->_mMinDamage = 2 * (mon->_mMinDamage + 2);
		mon->_mMaxDamage = 2 * (mon->_mMaxDamage + 2);
		mon->_mHit2 += NIGHTMARE_TO_HIT_BONUS;
		//mon->_mMagic2 += NIGHTMARE_MAGIC_BONUS;
		mon->_mMinDamage2 = 2 * (mon->_mMinDamage2 + 2);
		mon->_mMaxDamage2 = 2 * (mon->_mMaxDamage2 + 2);
		mon->_mArmorClass += NIGHTMARE_AC_BONUS;
		mon->_mEvasion += NIGHTMARE_EVASION_BONUS;
	} else if (gnDifficulty == DIFF_HELL) {
		mon->_mmaxhp = 4 * mon->_mmaxhp + (200 << 6);
		mon->mLevel += HELL_LEVEL_BONUS;
		mon->mExp = 4 * (mon->mExp + DIFFICULTY_EXP_BONUS);
		mon->_mHit += HELL_TO_HIT_BONUS;
		mon->_mMagic += HELL_MAGIC_BONUS;
		mon->_mMinDamage = 4 * mon->_mMinDamage + 6;
		mon->_mMaxDamage = 4 * mon->_mMaxDamage + 6;
		mon->_mHit2 += HELL_TO_HIT_BONUS;
		//mon->_mMagic2 += HELL_MAGIC_BONUS;
		mon->_mMinDamage2 = 4 * mon->_mMinDamage2 + 6;
		mon->_mMaxDamage2 = 4 * mon->_mMaxDamage2 + 6;
		mon->_mArmorClass += HELL_AC_BONUS;
		mon->_mEvasion += HELL_EVASION_BONUS;
		mon->mMagicRes = cmon->cmData->mMagicRes2;
	}

	if (gbMaxPlayers == 1) {
		mon->_mmaxhp >>= 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}
	mon->_mhitpoints = mon->_mmaxhp;
}

/**
 * Check the location if a monster can be placed there in the init phase.
 * Must not consider the player's position, since it is already initialized
 * and messes up the pseudo-random generated dungeon.
 */
static bool MonstPlace(int xp, int yp)
{
	static_assert(DBORDERX >= 3, "MonstPlace does not check IN_DUNGEON_AREA but expects a large enough border I.");
	static_assert(DBORDERY >= 3, "MonstPlace does not check IN_DUNGEON_AREA but expects a large enough border II.");
	return (dMonster[xp][yp] | /*dPlayer[xp][yp] |*/ nSolidTable[dPiece[xp][yp]]
		 | (dFlags[xp][yp] & (BFLAG_VISIBLE | BFLAG_POPULATED))) == 0;
}

#ifdef HELLFIRE
void WakeUberDiablo()
{
	MonsterStruct *mon;

	if (!deltaload)
		PlayEffect(MAX_MINIONS, 2);
	mon = &monster[MAX_MINIONS];
	mon->_mArmorClass -= 50;
	//mon->_mEvasion -= 20;
	mon->mLevel /= 2;
	mon->mMagicRes = 0;
	mon->_mmaxhp /= 2;
	if (mon->_mhitpoints > mon->_mmaxhp)
		mon->_mhitpoints = mon->_mmaxhp;
}
#endif

static void PlaceMonster(int mnum, int mtype, int x, int y)
{
	int dir;

	dMonster[x][y] = mnum + 1;

	dir = random_(90, NUM_DIRS);
	InitMonster(mnum, dir, mtype, x, y);
}

static void PlaceGroup(int mtype, int num, int leaderf, int leader)
{
	int placed, offset, try1, try2;
	int xp, yp, x1, y1, x2, y2;

	if (num + nummonsters > totalmonsters) {
		num = totalmonsters - nummonsters;
	}

	placed = 0;
	for (try1 = 0; try1 < 10; try1++) {
		while (placed != 0) {
			nummonsters--;
			placed--;
			dMonster[monster[nummonsters]._mx][monster[nummonsters]._my] = 0;
		}

		if (leaderf & 1) {
			x1 = monster[leader]._mx;
			y1 = monster[leader]._my;
		} else {
			do {
				x1 = random_(93, DSIZEX) + DBORDERX;
				y1 = random_(93, DSIZEY) + DBORDERY;
			} while (!MonstPlace(x1, y1));
		}

		assert(dTransVal[x1][y1] != 0 || currLvl._dLevelIdx == DLV_HELL4); // quads of the diablo level is fixed
		static_assert(DBORDERX >= 1, "PlaceGroup expects a large enough border I.");
		static_assert(DBORDERY >= 1, "PlaceGroup expects a large enough border II.");
		xp = x1; yp = y1;
		for (try2 = 0; placed < num && try2 < 128; try2++) {
			offset = random_(94, NUM_DIRS);
			x2 = xp + offset_x[offset];
			y2 = yp + offset_y[offset];
			assert((unsigned)x2 < MAXDUNX);
			assert((unsigned)y2 < MAXDUNX);
			if (dTransVal[x2][y2] != dTransVal[x1][y1]
			 || ((leaderf & 2) && ((abs(x2 - x1) >= 4) || (abs(y2 - y1) >= 4)))) {
				continue;
			}
			xp = x2;
			yp = y2;
			if ((!MonstPlace(xp, yp)) || random_(0, 2) != 0)
				continue;

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

static void PlaceUniqueMonst(int uniqindex, int miniontype, int bosspacksize)
{
	int xp, yp, x, y;
	int uniqtype;
	int count2;
	char filestr[64];
	const UniqMonstStruct *uniqm;
	MonsterStruct *mon;
	int count;

	if (uniquetrans >= (LIGHTSIZE / 256 - (LIGHTMAX + 4))) {
		return;
	}

	for (uniqtype = 0; uniqtype < nummtypes; uniqtype++) {
		if (Monsters[uniqtype].cmType == UniqMonst[uniqindex].mtype) {
			break;
		}
	}

	xp = -1;
	switch (uniqindex) {
	case UMT_SKELKING:
		xp = DBORDERX + 19;
		yp = DBORDERY + 31;
		break;
	case UMT_ZHAR:
		assert(nummonsters == MAX_MINIONS);
		if (zharlib != -1) {
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
		xp = 2 * setpc_x + DBORDERX + 3;
		yp = 2 * setpc_y + DBORDERY + 3;
		break;
#ifdef HELLFIRE
	case UMT_NAKRUL:
		assert(nummonsters == MAX_MINIONS);
		xp = 2 * setpc_x + DBORDERX + 2;
		yp = 2 * setpc_y + DBORDERY + 6;
		break;
#endif
	}

	if (xp == -1) {
		count = 0;
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

	uniqm = &UniqMonst[uniqindex];
	mon->mLevel = uniqm->muLevel;

	mon->mExp *= 2;
	mon->mName = uniqm->mName;
	mon->_mmaxhp = uniqm->mmaxhp << 6;

	mon->_mAi = uniqm->mAi;
	mon->_mint = uniqm->mint;
	mon->_mMinDamage = uniqm->mMinDamage;
	mon->_mMaxDamage = uniqm->mMaxDamage;
	mon->_mMinDamage2 = uniqm->mMinDamage2;
	mon->_mMaxDamage2 = uniqm->mMaxDamage2;
	mon->mMagicRes = uniqm->mMagicRes;
	mon->mtalkmsg = uniqm->mtalkmsg;
	if (mon->mtalkmsg != TEXT_NONE)
		mon->_mgoal = MGOAL_INQUIRING;

	snprintf(filestr, sizeof(filestr), "Monsters\\Monsters\\%s.TRN", uniqm->mTrnName);
	LoadFileWithMem(filestr, &pLightTbl[256 * (LIGHTMAX + 4 + uniquetrans)]);

	mon->_uniqtrans = uniquetrans++;

	if (uniqm->mUnqHit != 0) {
		mon->_mHit = uniqm->mUnqHit;

		if (gnDifficulty == DIFF_NIGHTMARE) {
			mon->_mHit += NIGHTMARE_TO_HIT_BONUS;
		} else if (gnDifficulty == DIFF_HELL) {
			mon->_mHit += HELL_TO_HIT_BONUS;
		}
		mon->_mHit2 = mon->_mHit;
	}
	if (uniqm->mUnqAC != 0) {
		mon->_mArmorClass = uniqm->mUnqAC;

		if (gnDifficulty == DIFF_NIGHTMARE) {
			mon->_mArmorClass += NIGHTMARE_AC_BONUS;
		} else if (gnDifficulty == DIFF_HELL) {
			mon->_mArmorClass += HELL_AC_BONUS;
		}
	}

	if (gnDifficulty == DIFF_NIGHTMARE) {
		mon->_mmaxhp = 2 * mon->_mmaxhp + (100 << 6);
		mon->mLevel += NIGHTMARE_LEVEL_BONUS;
		mon->_mMinDamage = 2 * (mon->_mMinDamage + 2);
		mon->_mMaxDamage = 2 * (mon->_mMaxDamage + 2);
		mon->_mMinDamage2 = 2 * (mon->_mMinDamage2 + 2);
		mon->_mMaxDamage2 = 2 * (mon->_mMaxDamage2 + 2);
	} else if (gnDifficulty == DIFF_HELL) {
		mon->_mmaxhp = 4 * mon->_mmaxhp + (200 << 6);
		mon->mLevel += HELL_LEVEL_BONUS;
		mon->_mMinDamage = 4 * mon->_mMinDamage + 6;
		mon->_mMaxDamage = 4 * mon->_mMaxDamage + 6;
		mon->_mMinDamage2 = 4 * mon->_mMinDamage2 + 6;
		mon->_mMaxDamage2 = 4 * mon->_mMaxDamage2 + 6;
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

	for (u = 0; UniqMonst[u].mtype != MT_INVALID; u++) {
		if (UniqMonst[u].muLevelIdx != currLvl._dLevelIdx)
			continue;
		if (UniqMonst[u].mQuestId != Q_INVALID
		 && quests[UniqMonst[u].mQuestId]._qactive == QUEST_NOTAVAIL)
			continue;
		for (mt = 0; mt < nummtypes; mt++) {
			if (Monsters[mt].cmType == UniqMonst[u].mtype) {
				PlaceUniqueMonst(u, mt, 8);
				break;
			}
		}
	}
}

static void PlaceQuestMonsters()
{
	BYTE *setp;

	if (!currLvl._dSetLvl) {
		if (QuestStatus(Q_BUTCHER)) {
			PlaceUniqueMonst(UMT_BUTCHER, 0, 0);
		}
		if (QuestStatus(Q_LTBANNER)) {
			setp = LoadFileInMem("Levels\\L1Data\\Banner1.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_BLOOD)) {
			setp = LoadFileInMem("Levels\\L2Data\\Blood2.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_BLIND)) {
			setp = LoadFileInMem("Levels\\L2Data\\Blind2.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_ANVIL)) {
			setp = LoadFileInMem("Levels\\L3Data\\Anvil.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);
		}
		if (QuestStatus(Q_WARLORD)) {
			setp = LoadFileInMem("Levels\\L4Data\\Warlord.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);
		}

		if (currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl && gbMaxPlayers != 1) {
			setp = LoadFileInMem("Levels\\L4Data\\Vile1.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);

			AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, FALSE);
			AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, FALSE);
			assert(UniqMonst[UMT_RED_VEX].mtype == UniqMonst[UMT_BLACKJADE].mtype);
			PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
			PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
			PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
		}
		if (currLvl._dLevelIdx == DLV_HELL4) {
			setp = LoadFileInMem("Levels\\L4Data\\diab1.DUN");
			SetMapMonsters(setp, DIAB_QUAD_1X, DIAB_QUAD_1Y);
			mem_free_dbg(setp);
			setp = LoadFileInMem("Levels\\L4Data\\diab2a.DUN");
			SetMapMonsters(setp, DIAB_QUAD_2X, DIAB_QUAD_2Y);
			mem_free_dbg(setp);
			setp = LoadFileInMem("Levels\\L4Data\\diab3a.DUN");
			SetMapMonsters(setp, DIAB_QUAD_3X, DIAB_QUAD_3Y);
			mem_free_dbg(setp);
			setp = LoadFileInMem("Levels\\L4Data\\diab4a.DUN");
			SetMapMonsters(setp, DIAB_QUAD_4X, DIAB_QUAD_4Y);
			mem_free_dbg(setp);
#ifdef HELLFIRE
		} else if (currLvl._dLevelIdx == DLV_CRYPT4) {
			PlaceUniqueMonst(UMT_NAKRUL, 0, 0);
#endif
		}
	} else if (currLvl._dLevelIdx == SL_SKELKING) {
		PlaceUniqueMonst(UMT_SKELKING, 0, 0);
	} else if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, FALSE);
		AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, FALSE);
		assert(UniqMonst[UMT_RED_VEX].mtype == UniqMonst[UMT_BLACKJADE].mtype);
		PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
		PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
		PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
	}
}

void InitMonsters()
{
	TriggerStruct *ts;
	int na;
	int i, j, xx, yy;
	int numplacemonsters;
	int mtype;
	int numscattypes;
	int scattertypes[NUM_MTYPES];
	const int tdx[4] = { -1, -1,  2,  2 };
	const int tdy[4] = { -1,  2, -1,  2 };

	numscattypes = 0;
#ifdef _DEBUG
	if (gbMaxPlayers != 1)
		CheckDungeonClear();
#endif
	if (!currLvl._dSetLvl) {
		for (i = 0; i < MAX_MINIONS; i++)
			AddMonster(0, 0, 0, 0, false);
	}
	for (i = 0; i < numtrigs; i++) {
		ts = &trigs[i];
		if (ts->_tmsg == WM_DIABTWARPUP || ts->_tmsg == WM_DIABPREVLVL
		 || (ts->_tmsg == WM_DIABNEXTLVL && currLvl._dLevelIdx != DLV_HELL3)) {
			for (j = 0; j < lengthof(tdx); j++)
				DoVision(ts->_tx + tdx[j], ts->_ty + tdy[j], 15, false, false);
		}
	}
	PlaceQuestMonsters();
	if (!currLvl._dSetLvl) {
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
			if (Monsters[i].cmPlaceScatter) {
				scattertypes[numscattypes] = i;
				numscattypes++;
			}
		}
		i = currLvl._dLevelIdx;
		while (nummonsters < totalmonsters) {
			mtype = scattertypes[random_(95, numscattypes)];
			if (i == DLV_CATHEDRAL1 || random_(95, 2) == 0)
				na = 1;
#ifdef HELLFIRE
			else if (i == DLV_CATHEDRAL2 || (i >= DLV_CRYPT1 && i <= DLV_CRYPT4))
#else
			else if (i == DLV_CATHEDRAL2)
#endif
				na = RandRange(2, 3);
			else
				na = RandRange(3, 5);
			PlaceGroup(mtype, na, 0, 0);
		}
	}
	for (i = 0; i < numtrigs; i++) {
		ts = &trigs[i];
		if (ts->_tmsg == WM_DIABTWARPUP || ts->_tmsg == WM_DIABPREVLVL
		 || (ts->_tmsg == WM_DIABNEXTLVL && currLvl._dLevelIdx != DLV_HELL3)) {
			for (j = 0; j < lengthof(tdx); j++)
				DoUnVision(trigs[i]._tx + tdx[j], trigs[i]._ty + tdy[j], 15);
		}
	}
}

void SetMapMonsters(BYTE *pMap, int startx, int starty)
{
	uint16_t rw, rh;
	uint16_t *lm;
	int i, j;
	int mtype;

	if (currLvl._dSetLvl) {
		AddMonsterType(MT_GOLEM, FALSE);
		for (i = 0; i < MAX_MINIONS; i++)
			AddMonster(0, 0, 0, 0, false);
	}
	lm = (uint16_t *)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	lm += rw * rh;
	rw = rw << 1;
	rh = rh << 1;
	lm += rw * rh;

	startx *= 2;
	startx += DBORDERX;
	starty *= 2;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				mtype = AddMonsterType(MonstConvTbl[SwapLE16(*lm) - 1], FALSE);
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

int AddMonster(int x, int y, int dir, int mtype, bool InMap)
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
	mon->_mAnimData = as->aData[md];
	mon->_mAnimLen = as->aFrames;
	mon->_mAnimCnt = 0;
	mon->_mAnimFrame = 1;
	mon->_mAnimFrameLen = as->aFrameLen;
	mon->_mFlags &= ~(MFLAG_REV_ANIMATION | MFLAG_LOCK_ANIMATION);
}

static bool MonRanged(int mnum)
{
	char ai = monster[mnum]._mAi;
	return ai == AI_SKELBOW || ai == AI_GOATBOW || ai == AI_SUCC || ai == AI_LAZHELP;
}

bool MonTalker(int mnum)
{
	return monster[mnum].mtalkmsg != TEXT_NONE;
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
	bool sameroom, bestsameroom;
	MonsterStruct *mon = &monster[mnum], *tmon;
	const BYTE tv = dTransVal[mon->_mx][mon->_my];

	enemy = 0;
	best_dist = MAXDUNX + MAXDUNY;
	bestsameroom = false;
	if (mnum >= MAX_MINIONS) {
		for (i = 0; i < MAX_PLRS; i++) {
			if (!players[i].plractive || currLvl._dLevelIdx != players[i].plrlevel || players[i]._pLvlChanging || players[i]._pHitPoints < (1 << 6))
				continue;
			sameroom = tv == dTransVal[players[i]._px][players[i]._py];
			dist = std::max(abs(mon->_mx - players[i]._px), abs(mon->_my - players[i]._py));
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
			mon->_menemyx = players[enemy]._pfutx;
			mon->_menemyy = players[enemy]._pfuty;
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
	mon->_mVar1 = mon->_mmode; // STAND_PREV_MODE : previous mode of the monster
	mon->_mVar2 = 0;           // STAND_TICK : the time spent on standing
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
		mon->_mVar2 = len;      // DELAY_TICK : length of the delay
		mon->_mmode = MM_DELAY;
	}
}

/*
 * Start the special standing of monsters.
 *
 * Golem: spawning
 * Skeleton: spawning
 * SkeletonKing: raising a skeleton

 * Fallen with spear: taunting
 * Fallen with sword: backflip
 */
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

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonChangeLightOff: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	lx = mon->_mxoff + 2 * mon->_myoff;
	ly = 2 * mon->_myoff - mon->_mxoff;

	lx = lx / 8;
	ly = ly / 8;

	CondChangeLightOff(mon->mlid, lx, ly);
}

static void MonStartWalk1(int mnum, int xvel, int yvel, int xadd, int yadd)
{
	MonsterStruct *mon;
	int mx, my;

	mon = &monster[mnum];
	mon->_mmode = MM_WALK;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	//mon->_mVar1 = xadd; // dx after the movement
	//mon->_mVar2 = yadd; // dy after the movement
	mon->_mVar6 = 0;    // MWALK_XOFF : _mxoff in a higher range
	mon->_mVar7 = 0;    // MWALK_YOFF : _myoff in a higher range
	//mon->_mVar8 = 0;    // Value used to measure progress for moving from one tile to another

	mx = mon->_mx;
	my = mon->_my;
	mon->_moldx = mx;
	mon->_moldy = my;

	mx += xadd;
	my += yadd;
	mon->_mfutx = mx;
	mon->_mfuty = my;

	dMonster[mx][my] = -(mnum + 1);
}

static void MonStartWalk2(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd)
{
	MonsterStruct *mon;
	int mx, my;

	mon = &monster[mnum];
	mon->_mmode = MM_WALK2;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	mon->_mVar6 = xoff << 4; // MWALK_XOFF : _mxoff in a higher range
	mon->_mVar7 = yoff << 4; // MWALK_YOFF : _myoff in a higher range
	//mon->_mVar8 = 0;         // Value used to measure progress for moving from one tile to another

	mx = mon->_mx;
	my = mon->_my;
	dMonster[mx][my] = -(mnum + 1);
	/*mon->_mVar1 =*/ mon->_moldx = mx; // the starting x-coordinate of the monster
	/*mon->_mVar2 =*/ mon->_moldy = my; // the starting y-coordinate of the monster
	mx += xadd;
	my += yadd;
	mon->_mx = mon->_mfutx = mx;
	mon->_my = mon->_mfuty = my;
	dMonster[mx][my] = mnum + 1;
	if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN)) {
		ChangeLightXY(mon->mlid, mx, my);
		MonChangeLightOff(mnum);
	}
}

static void MonStartWalk3(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy)
{
	MonsterStruct *mon;
	int mx, my, x, y;

	mon = &monster[mnum];
	mon->_mmode = MM_WALK3;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	mon->_mVar6 = xoff << 4; // MWALK_XOFF : _mxoff in a higher range
	mon->_mVar7 = yoff << 4; // MWALK_YOFF : _myoff in a higher range
	//mon->_mVar8 = 0;         // Value used to measure progress for moving from one tile to another

	mx = mon->_mx;
	my = mon->_my;

	mon->_moldx = mx;
	mon->_moldy = my;

	dMonster[mx][my] = -(mnum + 1);
	x = mapx + mx;
	y = mapy + my;
	mon->_mVar4 = x; // MWALK_FLAG_X : X-position of a tile which should have its BFLAG_MONSTLR flag removed after walking. When starting to walk the game places the monster in the dMonster array -1 in the Y coordinate, and uses BFLAG_MONSTLR to check if it should be using -1 to the Y coordinate when rendering the monster
	mon->_mVar5 = y; // MWALK_FLAG_Y : Y-position of a tile which should have its BFLAG_MONSTLR flag removed after walking. When starting to walk the game places the monster in the dMonster array -1 in the Y coordinate, and uses BFLAG_MONSTLR to check if it should be using -1 to the Y coordinate when rendering the monster
	dFlags[x][y] |= BFLAG_MONSTLR;
	mx += xadd;
	my += yadd;
	mon->_mfutx = /*mon->_mVar1 =*/ mx; // the Monster's x-coordinate after the movement
	mon->_mfuty = /*mon->_mVar2 =*/ my; // the Monster's y-coordinate after the movement
	dMonster[mx][my] = -(mnum + 1);
	if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN)) {
		//ChangeLightXY(mon->mlid, mon->_mVar4, mon->_mVar5);
		MonChangeLightOff(mnum);
	}
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
	mon->_mVar1 = mitype; // RATTACK_SKILL
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

/*
 * Start the special ranged-attacks of monsters.
 * Used by: Thin(STORM), Acid, Magma, DemonSkeleton,
 *          Mega, Diablo, SpiderLord, HorkDemon, Hellbat, Torchant
 * Not implemented for Nakrul.
 */
static void MonStartRSpAttack(int mnum, int mitype)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon;

	NewMonsterAnim(mnum, MA_SPECIAL, md);
	mon = &monster[mnum];
	mon->_mmode = MM_RSPATTACK;
	mon->_mVar1 = mitype; // SPATTACK_SKILL
	mon->_mVar2 = 0;      // SPATTACK_ANIM : counter to enable/disable MFLAG_LOCK_ANIMATION for certain monsters
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

/*
 * Start the special 'attack' of monsters.
 *
 * Goat with maces: roundkick
 * Toad(AI_FAT): punch
 * Defiler: scorpion-hit
 *
 * Scavengers: eating
 * Gravediggers: digging
 * Gargoyle: standing up

 * Rhino: running effect - handled by MIS_RHINO and MM_CHARGE
 */
static void MonStartSpAttack(int mnum)
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

	if (mon->_mmode == MM_DEATH)
		return;

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

static void MonTeleport(int mnum)
{
	MonsterStruct *mon;
	int i, x, y, _mx, _my, rx;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonTeleport: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode == MM_STONE)
		return;

	_mx = mon->_menemyx;
	_my = mon->_menemyy;
	rx = random_(100, NUM_DIRS);
	static_assert(DBORDERX >= 1, "MonTeleport expects a large enough border I.");
	static_assert(DBORDERY >= 1, "MonTeleport expects a large enough border II.");
	for (i = 0; i < lengthof(offset_x); i++, rx = (rx + 1) & 7) {
		x = _mx + offset_x[rx];
		y = _my + offset_y[rx];
		assert(IN_DUNGEON_AREA(x, y));
		if (x != mon->_mx && y != mon->_my && PosOkMonst(mnum, x, y)) {
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

static void MonFallenFear(int x, int y)
{
	MonsterStruct *mon;
	int i;

	for (i = 0; i < nummonsters; i++) {
		mon = &monster[monstactive[i]];
		if (mon->_mAi == AI_FALLEN
		 && abs(x - mon->_mx) < 5
		 && abs(y - mon->_my) < 5
		 && mon->_mhitpoints >= (1 << 6)) {
			mon->_mgoal = MGOAL_RETREAT;
			mon->_mgoalvar1 = 7 - 2 * mon->_mint; // RETREAT_DISTANCE
			mon->_mdir = GetDirection(x, y, mon->_mx, mon->_my);
		}
	}
}

void MonGetKnockback(int mnum)
{
	MonsterStruct *mon = &monster[mnum];
	int dir;

	if (mon->_mmode == MM_DEATH)
		return;
	// assert(mnum >= MAX_MINIONS);
	dir = OPPOSITE(mon->_mdir);
	if (DirOK(mnum, dir)) {
		MonClearSquares(mnum);
		mon->_moldx += offset_x[dir];
		mon->_moldy += offset_y[dir];
		MonStartGetHit(mnum);
	}
}

void MonStartHit(int mnum, int pnum, int dam)
{
	MonsterStruct *mon;
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by player/trap", mnum);
	}
	mon = &monster[mnum];
	if ((unsigned)pnum < MAX_PLRS) {
		mon->_mWhoHit |= 1 << pnum;
		if (pnum == mypnum) {
			NetSendCmdMonstDamage(mnum, mon->_mhitpoints, dam);
		}
	}
	PlayEffect(mnum, 1);
	if (mnum < MAX_MINIONS)
		return;
	if ((dam << 2) >= mon->_mmaxhp) {
		if ((unsigned)pnum < MAX_PLRS) {
			mon->_mFlags &= ~MFLAG_TARGETS_MONSTER;
			mon->_menemy = pnum;
			mon->_menemyx = players[pnum]._pfutx;
			mon->_menemyy = players[pnum]._pfuty;
			if (mon->_mmode != MM_STONE)
				mon->_mdir = MonGetDir(mnum);
		}
		if (mon->_mType == MT_BLINK) {
			MonTeleport(mnum);
		}
		if (mon->_mmode != MM_STONE) {
			MonStartGetHit(mnum);
		}
	}
}

static void MonDiabloDeath(int mnum, bool sendmsg)
{
	MonsterStruct *mon;
	int i, j;
	int mx, my;

	quests[Q_DIABLO]._qactive = QUEST_DONE;
	if (sendmsg)
		NetSendCmdQuest(Q_DIABLO, false); // recipient should not matter
	for (i = 0; i < nummonsters; i++) {
		j = monstactive[i];
		if (j == mnum)
			continue;
		if (j < MAX_MINIONS && MINION_NR_INACTIVE(j))
			continue;
		mon = &monster[j];
		//if (mon->_msquelch == 0)
		//	continue;
		NewMonsterAnim(j, MA_DEATH, mon->_mdir);
		mon->_mxoff = 0;
		mon->_myoff = 0;
		mon->_mmode = MM_DEATH;
		MonClearSquares(j);
		mx = mon->_moldx;
		my = mon->_moldy;
		mon->_my = my;
		mon->_mfuty = my;
		mon->_mx = mx;
		mon->_mfutx = mx;
		dMonster[mx][my] = j + 1;
	}
	mon = &monster[mnum];
	mon->_mVar1 = 0;         // DIABLO_TICK
	mon->_mVar2 = gbSoundOn; // DIABLO_SOUND
	mx = mon->_mx;
	my = mon->_my;
	PlaySfxLoc(USFX_DIABLOD, mx, my);
	AddLight(mx, my, 8);
	DoVision(mx, my, 8, false, true);

	gbProcessPlayers = false;
	gbSoundOn = false;
}

static void SpawnLoot(int mnum, bool sendmsg)
{
	MonsterStruct *mon;

	mon = &monster[mnum];
	SetRndSeed(mon->_mRndSeed);
	switch (mon->_uniqtype - 1) {
	case UMT_GARBUD:
		if (QuestStatus(Q_GARBUD)) {
			CreateTypeItem(mon->_mx + 1, mon->_my + 1, true, ITYPE_MACE, IMISC_NONE, sendmsg, false);
			return;
		}
		break;
#ifdef HELLFIRE
	case UMT_HORKDMN:
		if (quests[Q_GIRL]._qactive != QUEST_NOTAVAIL) {
			SpawnRewardItem(IDI_THEODORE, mon->_mx, mon->_my, sendmsg, false);
			return;
		}/*else {
			CreateAmulet(mon->_mx, mon->_my, sendmsg, false);
		}*/
		break;
	case UMT_DEFILER:
		if (effect_is_playing(USFX_DEFILER8))
			stream_stop();
		// quests[Q_DEFILER]._qlog = FALSE;
		quests[Q_DEFILER]._qactive = QUEST_DONE;
		if (sendmsg)
			NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
		SpawnRewardItem(IDI_MAPOFDOOM, mon->_mx, mon->_my, sendmsg, false);
		return;
	case UMT_NAKRUL:
		stream_stop();
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		quests[Q_NAKRUL]._qvar1 = 5; // set to higher than 4 so innocent monters are not 'woke'
		if (sendmsg)
			NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
		/*CreateMagicItem(ITYPE_SWORD, ICURS_GREAT_SWORD, mon->_mx, mon->_my, sendmsg);
		CreateMagicItem(ITYPE_STAFF, ICURS_WAR_STAFF, mon->_mx, mon->_my, sendmsg);
		CreateMagicItem(ITYPE_BOW, ICURS_LONG_WAR_BOW, mon->_mx, mon->_my, sendmsg);*/
		SpawnItem(mnum, mon->_mx, mon->_my, sendmsg); // double reward
		break;
#endif
	}
	SpawnItem(mnum, mon->_mx, mon->_my, sendmsg);
}

static void M2MStartHit(int defm, int offm, int dam)
{
	MonsterStruct *dmon;
	if ((unsigned)defm >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by monster", defm);
	}
	dmon = &monster[defm];
	if ((unsigned)offm < MAX_MINIONS) {
		static_assert(MAX_MINIONS == MAX_PLRS, "M2MStartHit requires that owner of a monster has the same id as the monster itself.");
		dmon->_mWhoHit |= 1 << offm;
		if (offm == mypnum) {
			NetSendCmdMonstDamage(defm, dmon->_mhitpoints, dam);
		}
	}
	PlayEffect(defm, 1);
	if (defm < MAX_MINIONS)
		return;
	if ((dam << 2) >= dmon->_mmaxhp) {
		if (dmon->_mType == MT_BLINK) {
			MonTeleport(defm);
		}
		if (dmon->_mmode != MM_STONE) {
			//if (offm >= 0)
				dmon->_mdir = OPPOSITE(monster[offm]._mdir);
			MonStartGetHit(defm);
		}
	}
}

static void MonstStartKill(int mnum, int mpnum, bool sendmsg)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonstStartKill: Invalid monster %d", mnum);
	}
	if (sendmsg)
		NetSendCmdMonstKill(mnum, mpnum);
	mon = &monster[mnum];
	mon->_mhitpoints = 0;

	if (mnum >= MAX_MINIONS) {
		if ((unsigned)mpnum < MAX_PLRS)
			mon->_mWhoHit |= 1 << mpnum;
		AddPlrMonstExper(mon->mLevel, mon->mExp, mon->_mWhoHit);
		SpawnLoot(mnum, sendmsg);
	}

	if (mon->_mType == MT_DIABLO)
		MonDiabloDeath(mnum, sendmsg);
	else
		PlayEffect(mnum, 2);

	if (mon->_mmode != MM_STONE) {
		mon->_mmode = MM_DEATH;
		// TODO: might want to turn towards the offending enemy. Might not, though...
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
	if ((mon->_mType >= MT_NACID && mon->_mType <= MT_XACID) || mon->_mType == MT_SPIDLORD)
#else
	if (mon->_mType >= MT_NACID && mon->_mType <= MT_XACID)
#endif
		AddMissile(mon->_mx, mon->_my, 0, 0, 0, MIS_ACIDPUD, 1, mnum, mon->_mint + 1, mon->_mint + 1, 0);
}

static void M2MStartKill(int offm, int defm)
{
	bool sendmsg;

	if ((unsigned)offm >= MAXMONSTERS) {
		dev_fatal("M2MStartKill: Invalid monster (attacker) %d", offm);
	}
	if ((unsigned)defm >= MAXMONSTERS) {
		dev_fatal("M2MStartKill: Invalid monster (killed) %d", defm);
	}
	static_assert(MAX_MINIONS == MAX_PLRS, "M2MStartKill requires that owner of a monster has the same id as the monster itself.");
	// check if it is a golem vs. monster/golem -> the attacker's owner should send the message
	if (offm == mypnum)
		sendmsg = true;
	else if (offm < MAX_MINIONS)
		sendmsg = false;
	// check if it is a monster vs. golem -> the golem's owner should send the message
	else if (defm == mypnum)
		sendmsg = true;
	else if (defm < MAX_MINIONS)
		sendmsg = false;
	// monster vs. monster -> the host should send the message (should not happen at the moment)
	else
		sendmsg = true;

	MonstStartKill(defm, offm, sendmsg);

#ifdef HELLFIRE
	MonStartStand(offm, monster[offm]._mdir);
#endif
}

void MonStartKill(int mnum, int pnum)
{
	if (pnum != mypnum && pnum != -1)
		// Wait for the message from the killer so we get the exact location as well.
		// Necessary to have synced drops. Sendmsg should be false anyway.
		return;

	MonstStartKill(mnum, pnum, true);
}

void MonSyncStartKill(int mnum, int x, int y, int pnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonSyncStartKill: Invalid monster %d", mnum);
	}
	if (monster[mnum]._mmode == MM_DEATH) {
		return;
	}

	if (dMonster[x][y] == 0) {
		MonClearSquares(mnum);
		monster[mnum]._mx = x;
		monster[mnum]._my = y;
		monster[mnum]._moldx = x;
		monster[mnum]._moldy = y;
	}

	MonstStartKill(mnum, pnum, false);
}

/*
 * Start fade in using the special effect of monsters.
 * Used by: Sneak, Fireman, Mage, DarkMage
 */
static void MonStartFadein(int mnum, int md, bool backwards)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
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
		mon->_mVar8 = 1;				// FADE_END : target frame to end the fade mode
	} else {
		mon->_mVar8 = mon->_mAnimLen;	// FADE_END : target frame to end the fade mode
	}
}

static void MonStartFadeout(int mnum, int md, bool backwards)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
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
		mon->_mVar8 = 1;				// FADE_END : target frame to end the fade mode
	} else {
		mon->_mVar8 = mon->_mAnimLen;	// FADE_END : target frame to end the fade mode
	}
}

static void MonStartHeal(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartHeal: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mAnimData = mon->_mAnims[MA_SPECIAL].aData[mon->_mdir];
	mon->_mAnimFrame = mon->_mAnims[MA_SPECIAL].aFrames;
	mon->_mFlags |= MFLAG_REV_ANIMATION;
	mon->_mmode = MM_HEAL;
	mon->_mVar1 = mon->_mmaxhp / (16 * RandRange(4, 8)); // HEAL_SPEED
}

static bool MonDoStand(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoStand: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mAnimData = mon->_mAnims[mon->_mType != MT_GOLEM ? MA_STAND : MA_WALK].aData[mon->_mdir];

	if (mon->_mAnimFrame == mon->_mAnimLen)
		MonEnemy(mnum);

	mon->_mVar2++; // STAND_TICK

	return false;
}

static bool MonDoWalk(int mnum)
{
	MonsterStruct *mon;
	bool rv;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoWalk: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->_mAnims[MA_WALK].aFrames) {
		switch (mon->_mmode) {
		case MM_WALK: // Movement towards NW, N, and NE
			dMonster[mon->_mx][mon->_my] = 0;
			//mon->_mx += mon->_mVar1;
			//mon->_my += mon->_mVar2;
			mon->_mx = mon->_mfutx;
			mon->_my = mon->_mfuty;
			dMonster[mon->_mx][mon->_my] = mnum + 1;
			break;
		case MM_WALK2: // Movement towards SW, S, and SE
			//dMonster[mon->_mVar1][mon->_mVar2] = 0;
			dMonster[mon->_moldx][mon->_moldy] = 0;
			break;
		case MM_WALK3: // Movement towards W and E
			dFlags[mon->_mVar4][mon->_mVar5] &= ~BFLAG_MONSTLR; // MWALK_FLAG_X, MWALK_FLAG_Y
			dMonster[mon->_mx][mon->_my] = 0;
			//mon->_mx = mon->_mVar1;
			//mon->_my = mon->_mVar2;
			mon->_mx = mon->_mfutx;
			mon->_my = mon->_mfuty;
			dMonster[mon->_mx][mon->_my] = mnum + 1;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
			ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = true;
	} else {
		//if (mon->_mAnimCnt == 0) {
#ifdef HELLFIRE
			if (mon->_mAnimFrame == 1 && mon->_mType == MT_FLESTHNG)
				PlayEffect(mnum, 3);
#endif
			//mon->_mVar8++;
			mon->_mVar6 += mon->_mxvel; // MWALK_XOFF
			mon->_mVar7 += mon->_myvel; // MWALK_YOFF
			mon->_mxoff = mon->_mVar6 >> 4;
			mon->_myoff = mon->_mVar7 >> 4;
			if (mon->mlid != 0 && !(mon->_mFlags & MFLAG_HIDDEN))
				MonChangeLightOff(mnum);
		//}
		rv = false;
	}

	return rv;
}

void MonTryM2MHit(int offm, int defm, int hper, int mind, int maxd)
{
	bool ret;

	if ((unsigned)defm >= MAXMONSTERS) {
		dev_fatal("MonTryM2MHit: Invalid monster %d", defm);
	}
	if (CheckMonsterHit(defm, &ret))
		return;

	hper = hper + (monster[offm].mLevel << 1) - (monster[defm].mLevel << 1);
	int hit = monster[defm]._mmode == MM_STONE ? 0 : random_(4, 100);
	if (hit < hper) {
		int dam = RandRange(mind, maxd) << 6;
		monster[defm]._mhitpoints -= dam;
		if (monster[defm]._mhitpoints < (1 << 6)) {
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

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonTryH2HHit: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];

	if (mon->_mFlags & MFLAG_TARGETS_MONSTER) {
		MonTryM2MHit(mnum, pnum, Hit, MinDam, MaxDam);
		return;
	}
	p = &players[pnum];
	if (p->_pInvincible)
		return;
	if (abs(mon->_mx - p->_px) >= 2 || abs(mon->_my - p->_py) >= 2)
		return;

	hper = 30 + Hit
		+ (2 * mon->mLevel)
		- (2 * p->_pLevel)
		- p->_pIAC;
	if (random_(98, 100) >= hper)
#ifdef _DEBUG
		if (!debug_mode_god_mode)
#endif
			return;

	blkper = p->_pIBlockChance;
	if (blkper != 0
	 && (p->_pmode == PM_STAND || p->_pmode == PM_BLOCK)) {
		// assert(p->_pSkillFlags & SFLAG_BLOCK);
		blkper = blkper - (mon->mLevel << 1);
		if (blkper > random_(98, 100)) {
			PlrStartBlock(pnum, GetDirection(p->_px, p->_py, mon->_mx, mon->_my));
			return;
		}
	}
	if (mon->_mType == MT_YZOMBIE && pnum == mypnum) {
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
	if (mon->_mFlags & MFLAG_LIFESTEAL) {
		mon->_mhitpoints += dam;
		if (mon->_mhitpoints > mon->_mmaxhp)
			mon->_mhitpoints = mon->_mmaxhp;
	}
	if (pnum == mypnum) {
		if (PlrDecHp(pnum, dam, DMGTYPE_NPC)) {
#ifdef HELLFIRE
			MonStartStand(mnum, mon->_mdir);
#endif
			return;
		}
	}
	StartPlrHit(pnum, dam, false);
	if (mon->_mFlags & MFLAG_KNOCKBACK) {
		if (p->_pmode != PM_GOTHIT)
			StartPlrHit(pnum, 0, true);
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

static bool MonDoAttack(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum) {
		MonTryH2HHit(mnum, mon->_menemy, mon->_mHit, mon->_mMinDamage, mon->_mMaxDamage);
		if (mon->_mAi != AI_SNAKE)
			PlayEffect(mnum, 0);
	} else if (mon->_mAi == AI_MAGMA && mon->_mAnimFrame == 9) {
		// mon->_mType >= MT_NMAGMA && mon->_mType <= MT_WMAGMA
		MonTryH2HHit(mnum, mon->_menemy, mon->_mHit + 10, mon->_mMinDamage - 2, mon->_mMaxDamage - 2);
		PlayEffect(mnum, 0);
	} else if (mon->_mAi == AI_STORM2 && mon->_mAnimFrame == 13) {
		// mon->_mType >= MT_STORM && mon->_mType <= MT_MAEL
		MonTryH2HHit(mnum, mon->_menemy, mon->_mHit - 20, mon->_mMinDamage + 4, mon->_mMaxDamage + 4);
		PlayEffect(mnum, 0);
	} else if (mon->_mAi == AI_SNAKE && mon->_mAnimFrame == 1)
		PlayEffect(mnum, 0);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return true;
	}

	return false;
}

static bool MonDoRAttack(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoRAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum) {
		AddMissile(
		    mon->_mx,
		    mon->_my,
		    mon->_menemyx,
		    mon->_menemyy,
		    mon->_mdir,
		    mon->_mVar1, // RATTACK_SKILL
		    1,
		    mnum,
		    0,
		    0,
		    0);
		PlayEffect(mnum, 0);
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return true;
	}

	return false;
}

static bool MonDoRSpAttack(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoRSpAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2 && mon->_mAnimCnt == 0) {
		AddMissile(
		    mon->_mx,
		    mon->_my,
		    mon->_menemyx,
		    mon->_menemyy,
		    mon->_mdir,
		    mon->_mVar1, // SPATTACK_SKILL
		    1,
		    mnum,
		    0,
		    0,
		    0);
		PlayEffect(mnum, 3);
	}

	if (mon->_mAi == AI_MEGA && mon->_mAnimFrame == 3) {
		if (mon->_mVar2++ == 0) { // SPATTACK_ANIM
			mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		} else if (mon->_mVar2 == 15) {
			mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return true;
	}

	return false;
}

static bool MonDoSAttack(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoSAttack: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2)
		MonTryH2HHit(mnum, mon->_menemy, mon->_mHit2, mon->_mMinDamage2, mon->_mMaxDamage2);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return true;
	}

	return false;
}

static bool MonDoFadein(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoFadein: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame) // FADE_END
		return false;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	if (mon->mlid != 0) { // && !(mon->_mFlags & MFLAG_HIDDEN)) {
		ChangeLightRadius(mon->mlid, MON_LIGHTRAD);
		ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
	}

	MonStartStand(mnum, mon->_mdir);
	return true;
}

static bool MonDoFadeout(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoFadeout: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame) // FADE_END
		return false;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	if (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN) {
		mon->_mFlags |= MFLAG_HIDDEN;
		if (mon->mlid != 0) {
			ChangeLightRadius(mon->mlid, 0);
		}
	}

	MonStartStand(mnum, mon->_mdir);
	return true;
}

static bool MonDoHeal(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoHeal: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mFlags & MFLAG_NOHEAL) {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		mon->_mmode = MM_SATTACK;
		return false;
	}

	if (mon->_mAnimFrame == 1) {
		mon->_mFlags &= ~MFLAG_REV_ANIMATION;
		if (mon->_mVar1 + mon->_mhitpoints < mon->_mmaxhp) { // HEAL_SPEED
			mon->_mhitpoints = mon->_mVar1 + mon->_mhitpoints;
			mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		} else {
			mon->_mhitpoints = mon->_mmaxhp;
			mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
			mon->_mmode = MM_SATTACK;
		}
	}
	return false;
}

static bool MonDoTalk(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoTalk: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	MonStartStand(mnum, mon->_mdir);
	mon->_mgoal = MGOAL_TALKING;
	if (effect_is_playing(alltext[mon->mtalkmsg].sfxnr))
		return false;
	InitQTextMsg(mon->mtalkmsg, gbMaxPlayers == 1 /*mon->_mListener == mypnum*/); // MON_TIMER
	if (mon->_uniqtype - 1 == UMT_LAZURUS) {
		if (gbMaxPlayers != 1) {
			quests[Q_BETRAYER]._qvar1 = 6;
			mon->_msquelch = UCHAR_MAX;
			mon->mtalkmsg = TEXT_NONE;
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	return false;
}

static bool MonDoGotHit(int mnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoGotHit: Invalid monster %d", mnum);
	}
	if (monster[mnum]._mAnimFrame == monster[mnum]._mAnimLen) {
		MonStartStand(mnum, monster[mnum]._mdir);

		return true;
	}

	return false;
}

void MonUpdateLeader(int mnum)
{
	MonsterStruct *mon;
	int i;

	if ((unsigned)mnum >= MAXMONSTERS) {
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
	bool bMusicOn;
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
	play_movie(vicSets[players[mypnum]._pClass], 0);
	play_movie("gendata\\Diabend.smk", 0);

	bMusicOn = gbMusicOn;
	gbMusicOn = true;

	musicVolume = sound_get_music_volume();
	sound_set_music_volume(VOLUME_MAX);

	music_start(TMUSIC_L2);
	play_movie("gendata\\loopdend.smk", MOV_SKIP | MOV_LOOP);
	music_stop();

	sound_set_music_volume(musicVolume);
	gbMusicOn = bMusicOn;
}

void PrepDoEnding(bool soundOn)
{
	int i;
	unsigned killLevel;

	gbSoundOn = soundOn;
	gbRunGame = false;
	gbDeathflag = false;
	gbCineflag = true;

	killLevel = gnDifficulty + 1;
	if (killLevel > players[mypnum]._pDiabloKillLevel)
		players[mypnum]._pDiabloKillLevel = killLevel;

	if (gbMaxPlayers == 1) {
		// save the hero + items
		pfile_write_hero();
		// delete the game
		pfile_delete_save_file(SAVEFILE_GAME);
	} else {
		for (i = 0; i < MAX_PLRS; i++) {
			players[i]._pmode = PM_QUIT;
			players[i]._pInvincible = TRUE;
			if (players[i]._pHitPoints < (1 << 6))
				players[i]._pHitPoints = (1 << 6);
		}
	}
}

static bool MonDoDeath(int mnum)
{
	MonsterStruct *mon;
	int x, y;

	if ((unsigned)mnum >= MAXMONSTERS) {
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

		if (++mon->_mVar1 == 140)      // DIABLO_TICK
			PrepDoEnding(mon->_mVar2); // DIABLO_SOUND
	} else if (mon->_mAnimFrame == mon->_mAnimLen) {
		AddDead(mnum);
	}
	return false;
}

static bool MonDoSpStand(int mnum)
{
	MonsterStruct *mon;
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoSpStand: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2)
		PlayEffect(mnum, 3);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return true;
	}

	return false;
}

static bool MonDoDelay(int mnum)
{
	MonsterStruct *mon;
	int oFrame;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoDelay: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	mon->_mAnimData = mon->_mAnims[MA_STAND].aData[MonGetDir(mnum)];

	if (mon->_mVar2-- == 0) { // DELAY_TICK
		oFrame = mon->_mAnimFrame;
		MonStartStand(mnum, mon->_mdir);
		mon->_mAnimFrame = oFrame;
		return true;
	}

	return false;
}

static bool MonDoStone(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoStone: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mhitpoints == 0) {
		mon->_mDelFlag = TRUE;
		dMonster[mon->_mx][mon->_my] = 0;
	}

	return false;
}

void MonWalkDir(int mnum, int md)
{
	const int *mwi;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonWalkDir: Invalid monster %d", mnum);
	}
	NewMonsterAnim(mnum, MA_WALK, md);
	mwi = MWVel[monster[mnum]._mAnims[MA_WALK].aFrames - 1];
	switch (md) {
	case DIR_N:
		MonStartWalk1(mnum, 0, -mwi[1], -1, -1);
		break;
	case DIR_NE:
		MonStartWalk1(mnum, mwi[1], -mwi[0], 0, -1);
		break;
	case DIR_E:
		MonStartWalk3(mnum, mwi[2], 0, -32, -16, 1, -1, 1, 0);
		break;
	case DIR_SE:
		MonStartWalk2(mnum, mwi[1], mwi[0], -32, -16, 1, 0);
		break;
	case DIR_S:
		MonStartWalk2(mnum, 0, mwi[1], 0, -32, 1, 1);
		break;
	case DIR_SW:
		MonStartWalk2(mnum, -mwi[1], mwi[0], 32, -16, 0, 1);
		break;
	case DIR_W:
		MonStartWalk3(mnum, -mwi[2], 0, 32, -16, -1, 1, 0, 1);
		break;
	case DIR_NW:
		MonStartWalk1(mnum, -mwi[1], -mwi[0], -1, 0);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

bool IsSkel(int mt)
{
	return (mt >= MT_WSKELAX && mt <= MT_XSKELAX)
	    || (mt >= MT_WSKELBW && mt <= MT_XSKELBW)
	    || (mt >= MT_WSKELSD && mt <= MT_XSKELSD);
}

bool IsGoat(int mt)
{
	return (mt >= MT_NGOATMC && mt <= MT_GGOATMC)
	    || (mt >= MT_NGOATBW && mt <= MT_GGOATBW);
}

static void MonSpawnSkel(int x, int y, int dir)
{
	int i;

	i = PreSpawnSkeleton();
	assert(i != -1);
	SpawnSkeleton(i, x, y, dir);
}

static void GroupUnity(int mnum)
{
	MonsterStruct *mon, *leader, *bmon;
	int i;
	bool clear;

	if ((unsigned)mnum >= MAXMONSTERS) {
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

static bool MonCallWalk(int mnum, int md)
{
	int mdtemp;
	bool ok;

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

static bool MonPathWalk(int mnum)
{
	char path[MAX_PATH_LENGTH];
	bool(*Check)
	(int, int, int);

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonPathWalk: Invalid monster %d", mnum);
	}
	Check = PosOkMonst3;
	if (!(monster[mnum]._mFlags & MFLAG_CAN_OPEN_DOOR))
		Check = PosOkMonst;

	if (FindPath(Check, mnum, monster[mnum]._mx, monster[mnum]._my, monster[mnum]._menemyx, monster[mnum]._menemyy, path) != 0) {
		//MonCallWalk(mnum, walk2dir[path[0]]);
		MonCallWalk(mnum, path[0]);
		return true;
	}

	return false;
}

static void MonCallWalk2(int mnum, int md)
{
	bool ok;
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

	//return ok;
}

static bool MonDumbWalk(int mnum, int md)
{
	bool ok;

	ok = DirOK(mnum, md);
	if (ok)
		MonWalkDir(mnum, md);

	return ok;
}

static bool MonRoundWalk(int mnum, int md, int *dir)
{
	int mdtemp;
	bool ok;

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
		ok = MonCallWalk(mnum, OPPOSITE(mdtemp));
	}
	return ok;
}

void MAI_Zombie(int mnum)
{
	MonsterStruct *mon;
	int md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Zombie: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	if (random_(103, 100) < 2 * mon->_mint + 10) {
		md = std::max(abs(mon->_mx - mon->_menemyx), abs(mon->_my - mon->_menemyy));
		if (md >= 2) {
			if (md >= 2 * mon->_mint + 4) {
				md = mon->_mdir;
				if (random_(104, 100) < 2 * mon->_mint + 20) {
					md = random_(104, NUM_DIRS);
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
	int mx, my;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelSd: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	mon->_mdir = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	mx -= mon->_menemyx;
	my -= mon->_menemyy;
	if (abs(mx) >= 2 || abs(my) >= 2) { // STAND_PREV_MODE
		if (mon->_mVar1 == MM_DELAY || (random_(106, 100) >= 35 - 4 * mon->_mint)) {
			MonCallWalk(mnum, mon->_mdir);
		} else {
			MonStartDelay(mnum, RandRange(15, 24) - 2 * mon->_mint);
		}
	} else {
		if (mon->_mVar1 == MM_DELAY || (random_(105, 100) < 2 * mon->_mint + 20)) {
			MonStartAttack(mnum);
		} else {
			MonStartDelay(mnum, RandRange(10, 19) - 2 * mon->_mint);
		}
	}
}

static bool MAI_Path(int mnum)
{
	MonsterStruct *mon;
	bool clear;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Path: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mType != MT_GOLEM) {
		if (mon->_msquelch == 0)
			return false;
		if (mon->_mmode != MM_STAND)
			return false;
		if (mon->_mgoal != MGOAL_NORMAL && mon->_mgoal != MGOAL_MOVE && mon->_mgoal != MGOAL_ATTACK2)
			return false;
	}

	clear = LineClearF1(
	    PosOkMonst2,
	    mnum,
	    mon->_mx,
	    mon->_my,
	    mon->_menemyx,
	    mon->_menemyy);
	if (!clear || (mon->_mpathcount >= 5 && mon->_mpathcount < 8)) {
		if (mon->_mFlags & MFLAG_CAN_OPEN_DOOR)
			MonstCheckDoors(mon->_mx, mon->_my);
		mon->_mpathcount++;
		if (mon->_mpathcount < 5)
			return false;
		if (MonPathWalk(mnum))
			return true;
	}

	if (mon->_mType != MT_GOLEM)
		mon->_mpathcount = 0;

	return false;
}

void MAI_Snake(int mnum)
{
	MonsterStruct *mon;
	int fx, fy, mx, my, dist, md;
	int tmp;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Snake: Invalid monster %d", mnum);
	}
	const char pattern[6] = { 1, 1, 0, -1, -1, 0 };
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my -fy));
	if (dist >= 2) { // STAND_PREV_MODE
		if (dist < 3 && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy) && mon->_mVar1 != MM_CHARGE) {
			if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, 1, mnum, 0, 0, 0) != -1) {
				PlayEffect(mnum, 0);
				dMonster[mon->_mx][mon->_my] = -(mnum + 1);
				mon->_mmode = MM_CHARGE;
			}
		} else if (mon->_mVar1 == MM_DELAY || random_(106, 100) >= 35 - 2 * mon->_mint) {
			tmp = md + pattern[mon->_mgoalvar1]; // SNAKE_DIRECTION_DELTA
			if (tmp < 0) {
				tmp += NUM_DIRS;
			} else if (tmp >= NUM_DIRS) {
				tmp -= NUM_DIRS;
			}
			mon->_mgoalvar1++;
			if (mon->_mgoalvar1 >= lengthof(pattern))
				mon->_mgoalvar1 = 0;
			md = tmp - mon->_mgoalvar2; // SNAKE_DIRECTION
			if (md < 0) {
				md += NUM_DIRS;
			} else if (md >= NUM_DIRS) {
				md -= NUM_DIRS;
			}
			if (md > 0) {
				if (md < 4) {
					md = mon->_mgoalvar2 + 1;
					if (md < 0) {
						md += NUM_DIRS;
					} else if (md >= NUM_DIRS) {
						md -= NUM_DIRS;
					}
					mon->_mgoalvar2 = md;
				} else if (md == 4) {
					mon->_mgoalvar2 = tmp;
				} else {
					md = mon->_mgoalvar2 - 1;
					if (md < 0) {
						md += NUM_DIRS;
					} else if (md >= NUM_DIRS) {
						md -= NUM_DIRS;
					}
					mon->_mgoalvar2 = md;
				}
			}
			if (!MonDumbWalk(mnum, mon->_mgoalvar2))
				MonCallWalk2(mnum, mon->_mdir);
		} else {
			MonStartDelay(mnum, RandRange(15, 24) - mon->_mint);
		}
	} else { // STAND_PREV_MODE
		if (mon->_mVar1 == MM_DELAY
		    || mon->_mVar1 == MM_CHARGE
		    || (random_(105, 100) < mon->_mint + 20)) {
			MonStartAttack(mnum);
		} else
			MonStartDelay(mnum, RandRange(10, 19) - mon->_mint);
	}
}

void MAI_Bat(int mnum)
{
	MonsterStruct *mon;
	int md, v, mx, my, fx, fy, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Bat: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_RETREAT) {
		if (mon->_mgoalvar1 == 0) { // RETREAT_FINISHED
			mon->_mgoalvar1++;
			MonCallWalk(mnum, OPPOSITE(md));
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			MonCallWalk(mnum, random_(108, 2) != 0 ? left[md] : right[md]);
		}
		return;
	}

	v = random_(107, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	mx = mon->_mx;
	my = mon->_my;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (mon->_mType == MT_GLOOM
	    && dist >= 5
	    && v < 4 * mon->_mint + 33
	    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
		if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, 1, mnum, 0, 0, 0) != -1) {
			dMonster[mon->_mx][mon->_my] = -(mnum + 1);
			mon->_mmode = MM_CHARGE;
		}
	} else if (dist >= 2) {
		if ((mon->_mVar2 > 20 && v < mon->_mint + 13) // STAND_TICK, STAND_PREV_MODE
		 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < mon->_mint + 63)) {
			MonCallWalk(mnum, md);
		}
	} else if (v < 4 * mon->_mint + 8) {
		MonStartAttack(mnum);
		mon->_mgoal = MGOAL_RETREAT;
		mon->_mgoalvar1 = 0; // RETREAT_FINISHED
		if (mon->_mType == MT_FAMILIAR) {
			AddMissile(mon->_menemyx, mon->_menemyy, 0, 0, -1, MIS_LIGHTNING, 1, mnum, 1, 10, 0);
		}
	}
}

void MAI_SkelBow(int mnum)
{
	MonsterStruct *mon;
	int mx, my, v;
	bool walking;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelBow: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;

	mon->_mdir = MonGetDir(mnum);
	v = random_(110, 100);

	walking = false;
	if (abs(mx) < 4 && abs(my) < 4) {
		if ((mon->_mVar2 > 20 && v < 2 * mon->_mint + 13) // STAND_TICK, STAND_PREV_MODE
		 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < 2 * mon->_mint + 63)) {
			walking = MonDumbWalk(mnum, OPPOSITE(mon->_mdir));
		}
	}

	if (!walking) {
		if (v < 2 * mon->_mint + 3) {
			if (LineClear(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy))
				MonStartRAttack(mnum, MIS_ARROWC);
		}
	}
}

void MAI_Fat(int mnum)
{
	MonsterStruct *mon;
	int mx, my, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fat: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);
	v = random_(111, 100);
	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if ((mon->_mVar2 > 20 && v < 4 * mon->_mint + 20) // STAND_TICK, STAND_PREV_MODE
		 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < 4 * mon->_mint + 70)) {
			MonCallWalk(mnum, mon->_mdir);
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

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Sneak: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND)
		return;
	mx = mon->_mx;
	my = mon->_my;
	if (dLight[mx][my] == LIGHTMAX)
		return;
	mx -= mon->_menemyx;
	my -= mon->_menemyy;
	dist = std::max(abs(mx), abs(my)); // STAND_PREV_MODE

	md = MonGetDir(mnum);
	range = 5 - mon->_mint;
	if (mon->_mVar1 == MM_GOTHIT && mon->_mgoal != MGOAL_RETREAT) {
		mon->_mgoal = MGOAL_RETREAT;
		mon->_mgoalvar1 = 0; // RETREAT_DISTANCE
	} else {
		if (dist >= range + 3 || mon->_mgoalvar1 > 8) { // RETREAT_DISTANCE
			mon->_mgoal = MGOAL_NORMAL;
			mon->_mgoalvar1 = 0;
		}
	}
	if (mon->_mgoal == MGOAL_RETREAT && !(mon->_mFlags & MFLAG_NO_ENEMY)) {
		md = mon->_menemy;
		if (mon->_mFlags & MFLAG_TARGETS_MONSTER)
			md = GetDirection(monster[md]._mx, monster[md]._my, mon->_mx, mon->_my);
		else
			md = GetDirection(players[md]._px, players[md]._py, mon->_mx, mon->_my);
		if (mon->_mType == MT_UNSEEN) {
			md = random_(112, 2) != 0 ? left[md] : right[md];
		}
	}
	mon->_mdir = md;
	v = random_(112, 100);
	if (dist < range && (mon->_mFlags & MFLAG_HIDDEN)) {
		MonStartFadein(mnum, md, false);
	} else if ((dist >= range + 1) && !(mon->_mFlags & MFLAG_HIDDEN)) {
		MonStartFadeout(mnum, md, true);
	} else {
		if (mon->_mgoal == MGOAL_RETREAT
		 || (dist >= 2 &&
			 ((mon->_mVar2 > 20 && v < 4 * mon->_mint + 14) // STAND_TICK, STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				 && mon->_mVar2 == 0 && v < 4 * mon->_mint + 64)))) {
			mon->_mgoalvar1++; // RETREAT_DISTANCE ?
			MonCallWalk(mnum, md);
		}
		if (mon->_mmode == MM_STAND
		 && (dist < 2 && v < 4 * mon->_mint + 10)) {
			MonStartAttack(mnum);
		}
	}
}

void MAI_Fireman(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy, md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fireman: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;

	mx = mon->_mx;
	my = mon->_my;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	md = MonGetDir(mnum);
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (LineClear(mx, my, fx, fy)
		    && AddMissile(mx, my, fx, fy, md, MIS_FIREMAN, 1, mnum, 0, 0, 0) != -1) {
			mon->_mmode = MM_CHARGE;
			mon->_mgoal = MGOAL_ATTACK2;
			//mon->_mgoalvar1 = 0; // FIREMAN_ACTION_PROGRESS
		} else {
			mx -= fx;
			my -= fy;
			if (abs(mx) < 2 && abs(my) < 2) {
				MonTryH2HHit(mnum, mon->_menemy, mon->_mHit, mon->_mMinDamage, mon->_mMaxDamage);
				mon->_mgoal = MGOAL_RETREAT;
				md = OPPOSITE(md);
			}
			if (!MonCallWalk(mnum, md)) {
				mon->_mgoal = MGOAL_ATTACK2;
				MonStartFadein(mnum, mon->_mdir, false);
			}
		}
	} else if (mon->_mgoal == MGOAL_ATTACK2) {
		if (++mon->_mgoalvar1 > 3) { // FIREMAN_ACTION_PROGRESS
			mon->_mgoal = MGOAL_NORMAL;
			mon->_mgoalvar1 = 0;
			MonStartFadeout(mnum, md, true);
		} else if (LineClear(mx, my, fx, fy)) {
			MonStartRAttack(mnum, MIS_KRULL);
		} else {
			MonStartDelay(mnum, RandRange(5, 14));
		}
	} else {
		assert(mon->_mgoal == MGOAL_RETREAT);
		mon->_mgoal = MGOAL_ATTACK2;
		MonStartFadein(mnum, md, false);
	}
}

void MAI_Fallen(int mnum)
{
	MonsterStruct *mon;
	int x, y, mx, my, m, rad;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fallen: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mgoal == MGOAL_ATTACK2) {
		if (mon->_mgoalvar1 != 0) // FALLEN_ALARM_TICK
			mon->_mgoalvar1--;
		else
			mon->_mgoal = MGOAL_NORMAL;
	}
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	if (mon->_mgoal == MGOAL_RETREAT) {
		if (--mon->_mgoalvar1 < 0) { // RETREAT_DISTANCE
			mon->_mgoal = MGOAL_NORMAL;
			MonStartStand(mnum, OPPOSITE(mon->_mdir));
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
		rad = 2 * mon->_mint + 4;
		static_assert(DBORDERX == DBORDERY && DBORDERX >= 10, "MAI_Fallen expects a large enough border.");
		assert(rad <= DBORDERX);
		mx = mon->_mx;
		my = mon->_my;
		for (y = -rad; y <= rad; y++) {
			for (x = -rad; x <= rad; x++) {
				m = dMonster[x + mx][y + my];
				if (m > 0) {
					m--;
					if (monster[m]._mAi == AI_FALLEN) {
						monster[m]._mgoal = MGOAL_ATTACK2;
						monster[m]._mgoalvar1 = 30 * mon->_mint + 105; // FALLEN_ALARM_TICK
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
	int mx, my;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Cleaver: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	mon->_mdir = GetDirection(mx, my, mon->_lastx, mon->_lasty);

	mx -= mon->_menemyx;
	my -= mon->_menemyy;
	if (abs(mx) >= 2 || abs(my) >= 2)
		MonCallWalk(mnum, mon->_mdir);
	else
		MonStartAttack(mnum);
}

static void MAI_Round(int mnum, bool special)
{
	MonsterStruct *mon;
	int fx, fy;
	int mx, my, md;
	int dist, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Round: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;
	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	fy = mon->_menemyy;
	fx = mon->_menemyx;
	dist = std::max(abs(mx - fx), abs(my - fy));
	v = random_(114, 100);
	if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
		if (mon->_mgoal == MGOAL_MOVE || (dist >= 4 && random_(115, 4) == 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(116, 2); // MOVE_TURN_DIRECTION
			}
			if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md)) {
				mon->_mgoal = MGOAL_NORMAL;
			} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
				MonStartDelay(mnum, RandRange(10, 19));
			}
		}
	} else
		mon->_mgoal = MGOAL_NORMAL;
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist >= 2) {
			if ((mon->_mVar2 > 20 && v < 2 * mon->_mint + 28) // STAND_TICK, STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
			        && mon->_mVar2 == 0
			        && v < 2 * mon->_mint + 78)) {
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

void MAI_GoatMc(int mnum)
{
	MAI_Round(mnum, true);
}

static void MAI_Ranged(int mnum, int mitype, bool special)
{
	int fx, fy, mx, my, md;
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Ranged: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	if (mon->_msquelch == UCHAR_MAX || (mon->_mFlags & MFLAG_TARGETS_MONSTER)) {
		if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
			MonstCheckDoors(mon->_mx, mon->_my);
		mon->_mdir = MonGetDir(mnum);
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		if (mon->_mVar1 == MM_RATTACK) { // STAND_PREV_MODE
			MonStartDelay(mnum, random_(118, 20));
		} else if (abs(mx) < 4 && abs(my) < 4) {
			if (random_(119, 100) < 10 * (mon->_mint + 7))
				MonCallWalk(mnum, OPPOSITE(mon->_mdir));
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
	MAI_Ranged(mnum, MIS_ARROWC, false);
}

void MAI_Succ(int mnum)
{
	MAI_Ranged(mnum, MIS_FLARE, false);
}

void MAI_SnowWich(int mnum)
{
	MAI_Ranged(mnum, MIS_SNOWWICH, false);
}

void MAI_HlSpwn(int mnum)
{
	MAI_Ranged(mnum, MIS_HLSPWN, false);
}

void MAI_SolBrnr(int mnum)
{
	MAI_Ranged(mnum, MIS_SOLBRNR, false);
}

#ifdef HELLFIRE
void MAI_Lich(int mnum)
{
	MAI_Ranged(mnum, MIS_LICH, false);
}

void MAI_ArchLich(int mnum)
{
	MAI_Ranged(mnum, MIS_ARCHLICH, false);
}

void MAI_PsychOrb(int mnum)
{
	MAI_Ranged(mnum, MIS_PSYCHORB, false);
}

void MAI_NecromOrb(int mnum)
{
	MAI_Ranged(mnum, MIS_NECROMORB, false);
}
#endif

void MAI_AcidUniq(int mnum)
{
	MAI_Ranged(mnum, MIS_ACID, true);
}

#ifdef HELLFIRE
void MAI_Firebat(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBOLT, false);
}

void MAI_Torchant(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBALL, false);
}
#endif

void MAI_Scav(int mnum)
{
	bool done;
	int dx, dy, dir, maxhp;
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Scav: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND)
		return;
	if (mon->_mhitpoints < (mon->_mmaxhp >> 1) && mon->_mgoal != MGOAL_HEALING) {
		if (mon->leaderflag != MLEADER_NONE) {
			monster[mon->leader].packsize--;
			mon->leaderflag = MLEADER_NONE;
		}
		mon->_mgoal = MGOAL_HEALING;
		mon->_mgoalvar3 = 10; // HEALING_ROUNDS
	}
	if (mon->_mgoal == MGOAL_HEALING && mon->_mgoalvar3 != 0) {
		mon->_mgoalvar3--; // HEALING_ROUNDS
		if (dDead[mon->_mx][mon->_my] != 0) {
			MonStartSpAttack(mnum);
			maxhp = mon->_mmaxhp;
			if (!(mon->_mFlags & MFLAG_NOHEAL)) {
#ifdef HELLFIRE
				mon->_mhitpoints += maxhp >> 3;
				if (mon->_mhitpoints > maxhp)
					mon->_mhitpoints = maxhp;
				if (mon->_mhitpoints == maxhp || mon->_mgoalvar3 <= 0) // HEALING_ROUNDS
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
			if (mon->_mgoalvar1 == 0) { // HEALING_LOCATION_X
				done = false;
				static_assert(DBORDERX >= 4, "MAI_Scav expects a large enough border I.");
				static_assert(DBORDERY >= 4, "MAI_Scav expects a large enough border II.");
				if (random_(120, 2) != 0) {
					for (dy = -4; dy <= 4 && !done; dy++) {
						for (dx = -4; dx <= 4 && !done; dx++) {
							// BUGFIX: incorrect check of offset against limits of the dungeon (fixed)
							//assert(IN_DUNGEON_AREA(mon->_mx + dx, mon->_mx + dy));
							done = dDead[mon->_mx + dx][mon->_my + dy] != 0
							    && LineClearF(
							           CheckNoSolid,
							           mon->_mx,
							           mon->_my,
							           mon->_mx + dx,
							           mon->_my + dy);
						}
					}
					dx--;
					dy--;
				} else {
					for (dy = 4; dy >= -4 && !done; dy--) {
						for (dx = 4; dx >= -4 && !done; dx--) {
							// BUGFIX: incorrect check of offset against limits of the dungeon (fixed)
							//assert(IN_DUNGEON_AREA(mon->_mx + dx, mon->_mx + dy));
							done = dDead[mon->_mx + dx][mon->_my + dy] != 0
							    && LineClearF(
							           CheckNoSolid,
							           mon->_mx,
							           mon->_my,
							           mon->_mx + dx,
							           mon->_my + dy);
						}
					}
					dx++;
					dy++;
				}
				if (done) {
					mon->_mgoalvar1 = mon->_mx + dx; // HEALING_LOCATION_X
					mon->_mgoalvar2 = mon->_my + dy; // HEALING_LOCATION_Y
				}
			}
			if (mon->_mgoalvar1 != 0) {
				//                                  HEALING_LOCATION_X, HEALING_LOCATION_Y
				dir = GetDirection(mon->_mx, mon->_my, mon->_mgoalvar1, mon->_mgoalvar2);
				MonCallWalk(mnum, dir);
			}
		}
	}

	if (mon->_mmode == MM_STAND)
		MAI_SkelSd(mnum);
}

void MAI_Garg(int mnum)
{
	MonsterStruct *mon;
	int mx, my, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Garg: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mFlags & MFLAG_GARG_STONE) {
		if (mon->_msquelch != 0) {
			MonEnemy(mnum);
			mx = mon->_mx - mon->_menemyx;
			my = mon->_my - mon->_menemyy;
			dist = std::max(abs(mx), abs(my));
			if (dist < mon->_mint + 2) {
				mon->_mFlags &= ~(MFLAG_LOCK_ANIMATION | MFLAG_GARG_STONE);
			}
		} else if (mon->_mmode != MM_SATTACK) {
			if (mon->leaderflag == MLEADER_NONE && mon->_uniqtype == 0) {
				MonStartSpAttack(mnum);
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
		dist = std::max(abs(mx), abs(my));
		if (dist >= mon->_mint + 2) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartHeal(mnum);
		} else if (!MonCallWalk(mnum, OPPOSITE(MonGetDir(mnum)))) {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	MAI_Round(mnum, false);
}

static void MAI_RoundRanged(int mnum, int mitype, int lessmissiles)
{
	MonsterStruct *mon;
	int mx, my;
	int fx, fy;
	int md, dist, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_RoundRanged: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;

	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	v = random_(121, 10000);
	if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
		if (mon->_mgoal == MGOAL_MOVE || (dist >= 3 && random_(122, 4 << lessmissiles) == 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(123, 2); // MOVE_TURN_DIRECTION
			}
			if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md)) {
				mon->_mgoal = MGOAL_NORMAL;
			} else if (v < ((500 * (mon->_mint + 1)) >> lessmissiles)
			    && (LineClear(mon->_mx, mon->_my, fx, fy))) {
				MonStartRSpAttack(mnum, mitype);
			} else {
				MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (((dist >= 3 && v < ((500 * (mon->_mint + 2)) >> lessmissiles))
		        || v < ((500 * (mon->_mint + 1)) >> lessmissiles))
		    && LineClear(mon->_mx, mon->_my, fx, fy)) {
			MonStartRSpAttack(mnum, mitype);
		} else if (dist >= 2) {
			v = random_(124, 100);
			if (v < 10 * (mon->_mint + 5) // STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				 && mon->_mVar2 == 0 && v < 10 * (mon->_mint + 8))) { // STAND_TICK
				MonCallWalk(mnum, md);
			}
		} else if (v < 1000 * (mon->_mint + 6)) {
			MonStartAttack(mnum);
		}
	}
	if (mon->_mmode == MM_STAND) {
		MonStartDelay(mnum, RandRange(5, 14));
	}
}

void MAI_Magma(int mnum)
{
	MAI_RoundRanged(mnum, MIS_MAGMABALL, 0);
}

void MAI_Storm(int mnum)
{
	MAI_RoundRanged(mnum, MIS_LIGHTNINGC, 0);
}

void MAI_Storm2(int mnum)
{
	MAI_RoundRanged(mnum, MIS_LIGHTNINGC2, 0);
}

#ifdef HELLFIRE
void MAI_BoneDemon(int mnum)
{
	MAI_RoundRanged(mnum, MIS_BONEDEMON, 0);
}
#endif

void MAI_Acid(int mnum)
{
	MAI_RoundRanged(mnum, MIS_ACID, 1);
}

void MAI_Diablo(int mnum)
{
	MAI_RoundRanged(mnum, MIS_APOCAC2, 0);
}

static void MAI_RR2(int mnum, int mitype)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int dist, v, md;

	if ((unsigned)mnum >= MAXMONSTERS) {
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

	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mx, my);
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(121, 100);
	if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mx][my] == dTransVal[fx][fy]) {
		if (mon->_mgoal == MGOAL_MOVE || dist >= 3) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(123, 2); // MOVE_TURN_DIRECTION
			}
			mon->_mgoalvar3 = TRUE;                // MOVE_POSITIONED
			if (mon->_mgoalvar1++ < 2 * dist || !DirOK(mnum, md)) {
				if (v < 5 * (mon->_mint + 16))
					MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else
				mon->_mgoal = MGOAL_NORMAL;
		}
	} else
		mon->_mgoal = MGOAL_NORMAL;
	if (mon->_mgoal == MGOAL_NORMAL) { // MOVE_POSITIONED
		if (((dist >= 3 && v < 5 * (mon->_mint + 2)) || v < 5 * (mon->_mint + 1) || mon->_mgoalvar3) && LineClear(mon->_mx, mon->_my, fx, fy)) {
			MonStartRSpAttack(mnum, mitype);
			return;
		}
		v = random_(124, 100);
		if (dist >= 2) {
			if (v < 10 * (mon->_mint + 5) // STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
			        && mon->_mVar2 == 0 // STAND_TICK
			        && v < 10 * (mon->_mint + 8))) {
				MonCallWalk(mnum, md);
			}
		} else {
			if (v < 10 * (mon->_mint + 4)) {
				mon->_mdir = md;
				if (random_(124, 2) != 0)
					MonStartAttack(mnum);
				else
					MonStartRSpAttack(mnum, mitype);
			}
		}
		mon->_mgoalvar3 = FALSE; // MOVE_POSITIONED
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

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Golum: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	assert(!MINION_INACTIVE(mon));

	if (mon->_mmode != MM_STAND) {
		//assert(mon->_mmode == MM_DEATH || mon->_mmode == MM_SPSTAND
		// || mon->_mmode == MM_ATTACK || (mon->_mmode >= MM_WALK && mon->_mmode <= MM_WALK3));
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
				static_assert(DBORDERX >= 2, "MAI_Golum expects a large enough border I.");
				static_assert(DBORDERY >= 2, "MAI_Golum expects a large enough border II.");
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

	md = players[mnum]._pdir;
	if (!MonCallWalk(mnum, md)) {
		for (j = 0; j < NUM_DIRS; j++) {
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

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelKing: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;
	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(126, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2 || mon->_msquelch != UCHAR_MAX) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 3 && random_(127, 4) == 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoal = MGOAL_MOVE;
			mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
			mon->_mgoalvar2 = random_(128, 2); // MOVE_TURN_DIRECTION
		}
		if ((mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md)) || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			MonStartDelay(mnum, RandRange(10, 19));
		}
	}
		
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (gbMaxPlayers == 1
		    && ((dist >= 3 && v < 4 * mon->_mint + 35) || v < 6)
		    && LineClear(mon->_mx, mon->_my, fx, fy)) {
			nx = mon->_mx + offset_x[md];
			ny = mon->_my + offset_y[md];
			if (PosOkMonst(mnum, nx, ny) && nummonsters < MAXMONSTERS) {
				MonSpawnSkel(nx, ny, md);
				MonStartSpStand(mnum, md);
			}
		} else if (dist < 2) {
			if (v < mon->_mint + 20) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(129, 100);
			if (v < mon->_mint + 25 // STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				 && mon->_mVar2 == 0 && v < mon->_mint + 75)) { // STAND_TICK
				MonCallWalk(mnum, md);
			} else {
				MonStartDelay(mnum, RandRange(10, 19));
			}
		}
	}
}

void MAI_Rhino(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int v, dist, md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Rhino: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;
	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(131, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoal = MGOAL_MOVE;
			mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
			mon->_mgoalvar2 = random_(133, 2); // MOVE_TURN_DIRECTION
		}
		if (mon->_mgoalvar1++ >= 2 * dist || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			MonStartDelay(mnum, RandRange(10, 19));
		}
	}
		
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist >= 5 && v < 2 * mon->_mint + 43
		    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
			if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, 1, mnum, 0, 0, 0) != -1) {
				if (mon->MData->snd_special)
					PlayEffect(mnum, 3);
				mon->_mmode = MM_CHARGE;
				dMonster[mon->_mx][mon->_my] = -(mnum + 1);
			}
		} else if (dist < 2) {
			if (v < 2 * mon->_mint + 28) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mint + 33 // STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				 && mon->_mVar2 == 0 && v < 2 * mon->_mint + 83)) { // STAND_TICK
				MonCallWalk(mnum, md);
			} else {
				MonStartDelay(mnum, RandRange(10, 19));
			}
		}
	}
}

#ifdef HELLFIRE
void MAI_Horkdemon(int mnum)
{
	MonsterStruct *mon;
	int fx, fy, mx, my, md, v, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Horkdemon: Invalid monster %d", mnum);
	}

	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(131, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoal = MGOAL_MOVE;
			mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
			mon->_mgoalvar2 = random_(133, 2); // MOVE_TURN_DIRECTION
		}
		if (mon->_mgoalvar1++ >= 2 * dist || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
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
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mint + 33 // STAND_PREV_MODE
			 || ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
				 && mon->_mVar2 == 0 && v < 2 * mon->_mint + 83)) { // STAND_TICK
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

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Counselor: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0)
		return;
	if (mon->_msquelch < UCHAR_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	mx = mon->_mx - fx;
	my = mon->_my - fy;
	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
	if (mon->_mgoal == MGOAL_RETREAT) {
		if (mon->_mgoalvar1++ <= 3) // RETREAT_DISTANCE
			MonCallWalk(mnum, OPPOSITE(md));
		else {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, md, true);
		}
	} else if (mon->_mgoal == MGOAL_MOVE) {
		dist = std::max(abs(mx), abs(my));
		if (dist >= 2 && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]
		 && (mon->_mgoalvar1++ < 2 * dist || !DirOK(mnum, md))) { // MOVE_DISTANCE
			MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, md, true);
		}
	} else if (mon->_mgoal == MGOAL_NORMAL) {
		v = random_(121, 100);
		if (abs(mx) >= 2 || abs(my) >= 2) {
			if (v < 5 * (mon->_mint + 10) && LineClear(mon->_mx, mon->_my, fx, fy)) {
				MonStartRAttack(mnum, counsmiss[mon->_mint]);
			} else if (random_(124, 100) < 30) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(125, 2); // MOVE_TURN_DIRECTION
				MonStartFadeout(mnum, md, false);
			} else
				MonStartDelay(mnum, RandRange(10, 19) - 2 * mon->_mint);
		} else {
			mon->_mdir = md;
			if (mon->_mhitpoints < (mon->_mmaxhp >> 1)) {
				mon->_mgoal = MGOAL_RETREAT;
				mon->_mgoalvar1 = 0; // RETREAT_DISTANCE
				MonStartFadeout(mnum, md, false);
			} else if (mon->_mVar1 == MM_DELAY // STAND_PREV_MODE
			    || v < 2 * mon->_mint + 20) {
				MonStartRAttack(mnum, MIS_FLASH);
			} else
				MonStartDelay(mnum, RandRange(10, 19) - 2 * mon->_mint);
		}
		return;
	}
	if (mon->_mmode == MM_STAND) {
		MonStartDelay(mnum, RandRange(5, 14));
	}
}

void MAI_Garbud(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Garbud: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);
	if (mon->_mgoal == MGOAL_TALKING) {
		if ((dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE)) { // MON_TIMER
			//if (quests[Q_GARBUD]._qvar1 == 4 && mon->_mVar8++ >= gnTicksRate * 6) {
			if (quests[Q_GARBUD]._qvar1 == 4 && (gbMaxPlayers != 1 || !effect_is_playing(USFX_GARBUD4))) {
				mon->_mgoal = MGOAL_NORMAL;
				// mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = TEXT_NONE;
			}
		} else {
			if (quests[Q_GARBUD]._qvar1 < 4)
				mon->_mgoal = MGOAL_INQUIRING;
		}
	} else if (mon->_mgoal == MGOAL_INQUIRING && quests[Q_GARBUD]._qvar1 == 4) {
		// TODO: does not work when a player enters the level and the timer is running
		mon->_mgoal = MGOAL_NORMAL;
		mon->mtalkmsg = TEXT_NONE;
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_MOVE)
		MAI_Round(mnum, true);
}

void MAI_Zhar(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Zhar: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_TALKING) {
		if (quests[Q_ZHAR]._qvar1 == 1)
			mon->_mgoal = MGOAL_INQUIRING;
		if (dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) { // MON_TIMER - also set in objects.cpp
			//if (quests[Q_ZHAR]._qvar1 == 2 && mon->_mVar8++ >= gnTicksRate * 4/*!effect_is_playing(USFX_ZHAR2)*/) {
			if (quests[Q_ZHAR]._qvar1 == 2 && (gbMaxPlayers != 1 || !effect_is_playing(USFX_ZHAR2))) {
				// mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = TEXT_NONE;
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else if (mon->_mgoal == MGOAL_INQUIRING && quests[Q_ZHAR]._qvar1 == 2) {
		// TODO: does not work when a player enters the level and the timer is running
		mon->_mgoal = MGOAL_NORMAL;
		mon->mtalkmsg = TEXT_NONE;
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT || mon->_mgoal == MGOAL_MOVE)
		MAI_Counselor(mnum);
}

void MAI_SnotSpil(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SnotSpil: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);

	switch (quests[Q_LTBANNER]._qvar1) {
	case 0: // quest not started -> skip
		return;
	case 1: // quest just started -> waiting for the banner
		// switch to new text if the player(s) left
		if (mon->mtalkmsg == TEXT_BANNER10 && !(dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE))
			mon->mtalkmsg = TEXT_BANNER11;
		if (mon->_mgoal == MGOAL_TALKING)
			mon->_mgoal = MGOAL_INQUIRING;
		return;
	case 2: // banner given to ogden -> wait to lure the player
		if (mon->_mgoal == MGOAL_TALKING)
			mon->_mgoal = MGOAL_INQUIRING;
		return;
	case 3: // banner received or talked after the banner was given to ogden -> attack
		//if (mon->_mVar8++ < gnTicksRate * 6) // MON_TIMER
		//	return; // wait till the sfx is running, but don't rely on effect_is_playing
		if (gbMaxPlayers == 1 && effect_is_playing(alltext[TEXT_BANNER12].sfxnr))
			return;
		quests[Q_LTBANNER]._qactive = QUEST_DONE;
		quests[Q_LTBANNER]._qvar1 = 4;
		if (mon->_mListener == mypnum || !players[mon->_mListener].plractive || players[mon->_mListener].plrlevel != currLvl._dLevelIdx) {
			NetSendCmd(true, CMD_OPENSPIL);
			NetSendCmdQuest(Q_LTBANNER, true);
		}
		// mon->_msquelch = UCHAR_MAX;
	case 4:
		if (mon->mtalkmsg != TEXT_NONE) {
			// TODO: does not work when a player enters the level and the timer is running
			mon->mtalkmsg = TEXT_NONE;
			mon->_mgoal = MGOAL_NORMAL;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	MAI_Fallen(mnum);
}

void MAI_Lazurus(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lazurus: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);
	if ((dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) && mon->mtalkmsg == TEXT_VILE13) {
		if (gbMaxPlayers == 1) {
			if (mon->_mgoal == MGOAL_INQUIRING && players[mypnum]._px == DBORDERX + 19 && players[mypnum]._py == DBORDERY + 30) {
				PlayInGameMovie("gendata\\fprst3.smk");
				mon->_mmode = MM_TALK;
				mon->_mListener = mypnum;
				quests[Q_BETRAYER]._qvar1 = 5;
			} else if (mon->_mgoal == MGOAL_TALKING && !effect_is_playing(USFX_LAZ1)) {
				ObjChangeMapResync(1, 18, 20, 24);
				RedoPlayerVision();
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = TEXT_NONE;
				mon->_mgoal = MGOAL_NORMAL;
				quests[Q_BETRAYER]._qvar1 = 6;
			}
		} else {
			if (mon->_mgoal == MGOAL_INQUIRING) {
				if (quests[Q_BETRAYER]._qvar1 <= 3) {
					mon->_mmode = MM_TALK;
					mon->_mListener = mypnum;
				} else {
					mon->mtalkmsg = TEXT_NONE;
					mon->_mgoal = MGOAL_NORMAL;
				}
			}
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT || mon->_mgoal == MGOAL_MOVE) {
		MAI_Counselor(mnum);
	}
}

void MAI_Lazhelp(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lazhelp: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND)
		return;

	mon->_mdir = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_INQUIRING || mon->_mgoal == MGOAL_TALKING) {
		if (gbMaxPlayers == 1 && quests[Q_BETRAYER]._qvar1 <= 5)
			return;
		mon->mtalkmsg = TEXT_NONE;
		mon->_mgoal = MGOAL_NORMAL;
	}

	MAI_HlSpwn(mnum);
}

void MAI_Lachdanan(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lachdanan: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_TALKING) {
		if (quests[Q_VEIL]._qactive == QUEST_DONE) { // MON_TIMER
			//if (mon->_mVar8++ >= gnTicksRate * 32) {
			if (gbMaxPlayers != 1 || !effect_is_playing(USFX_LACH3)) {
				mon->mtalkmsg = TEXT_NONE;
				MonStartKill(mnum, -1);
			}
			return;
		}
		if (!(dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE) && mon->mtalkmsg == TEXT_VEIL9) {
			mon->mtalkmsg = TEXT_VEIL10;
		}
		mon->_mgoal = MGOAL_INQUIRING;
	}
}

void MAI_Warlord(int mnum)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Warlord: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	mon->_mdir = MonGetDir(mnum);

	switch (quests[Q_WARLORD]._qvar1) {
	case 0: // quest not started
		if (!(dFlags[mon->_mx][mon->_my] & BFLAG_VISIBLE))
			return;
		quests[Q_WARLORD]._qvar1 = 1;
		if (mon->_menemy == mypnum || !players[mon->_menemy].plractive || players[mon->_menemy].plrlevel != currLvl._dLevelIdx) {
			NetSendCmdQuest(Q_WARLORD, true);
		}
		mon->_mmode = MM_TALK;
		mon->_mListener = mon->_menemy;
		//mon->_mVar8 = 0; // MON_TIMER
		return;
	case 1: // warlord spotted
		//if (mon->_mVar8++ < gnTicksRate * 8) // MON_TIMER
		//	return; // wait till the sfx is running, but don't rely on effect_is_playing
		if (gbMaxPlayers == 1 && effect_is_playing(alltext[TEXT_WARLRD9].sfxnr))
			return;
		quests[Q_WARLORD]._qvar1 = 2;
		if (mon->_mListener == mypnum || !players[mon->_mListener].plractive || players[mon->_mListener].plrlevel != currLvl._dLevelIdx) {
			NetSendCmdQuest(Q_WARLORD, true);
		}
		// mon->_msquelch = UCHAR_MAX;
	case 2:
		if (mon->mtalkmsg != TEXT_NONE) {
			// TODO: does not work when a player enters the level and the timer is running
			mon->mtalkmsg = TEXT_NONE;
			mon->_mgoal = MGOAL_NORMAL;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (mon->_mgoal == MGOAL_NORMAL)
		MAI_SkelSd(mnum);
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
	bool raflag;
	MonsterStruct *mon;

	DeleteMonsterList();

	assert((unsigned)nummonsters <= MAXMONSTERS);
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
				// quests[Q_NAKRUL]._qvar1 == 4 -> UberRoom was opened by the books
				PlaySfxLoc(quests[Q_JERSEY]._qactive != QUEST_NOTAVAIL ? USFX_NAKRUL6 : (quests[Q_NAKRUL]._qvar1 == 4 ? USFX_NAKRUL4 : USFX_NAKRUL5), mx, my);
			else if (mon->_mType == MT_DEFILER)
				PlaySfxLoc(USFX_DEFILER8, mx, my);
			MonEnemy(mnum);
#endif
		}

		_menemy = mon->_menemy;
		if (mon->_mFlags & MFLAG_TARGETS_MONSTER) {
			if ((unsigned)_menemy >= MAXMONSTERS) {
				dev_fatal("Illegal enemy monster %d for monster \"%s\"", _menemy, mon->mName);
			}
			mon->_lastx = mon->_menemyx = monster[_menemy]._mfutx;
			mon->_lasty = mon->_menemyy = monster[_menemy]._mfuty;
		} else {
			if ((unsigned)_menemy >= MAX_PLRS) {
				dev_fatal("Illegal enemy player %d for monster \"%s\"", _menemy, mon->mName);
			}
			mon->_menemyx = players[_menemy]._pfutx;
			mon->_menemyy = players[_menemy]._pfuty;
			if (dFlags[mx][my] & BFLAG_VISIBLE) {
				mon->_msquelch = UCHAR_MAX;
				mon->_lastx = mon->_menemyx;
				mon->_lasty = mon->_menemyy;
			} else if (mon->_msquelch != 0 && mon->_mType != MT_DIABLO) { /// BUGFIX: change '_mAi' to '_mType' (fixed)
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
			case MM_WALK2:
			case MM_WALK3:
				raflag = MonDoWalk(mnum);
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
				raflag = false;
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
				raflag = false;
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
			if (mon->_mAnimCnt >= mon->_mAnimFrameLen) {
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
		mtype = Monsters[i].cmType;
		for (j = 0; j < lengthof(animletter); j++) {
			if (animletter[j] != 's' || monsterdata[mtype].has_special) {
				MemFreeDbg(Monsters[i].cmAnims[j].aCelData);
			}
		}
	}

	FreeMissiles2();
}

bool DirOK(int mnum, int mdir)
{
	int fx, fy;
	int x, y;
	int mcount, ma;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("DirOK: Invalid monster %d", mnum);
	}
	fx = monster[mnum]._mx + offset_x[mdir];
	fy = monster[mnum]._my + offset_y[mdir];
	static_assert(DBORDERX >= 3, "DirOK expects a large enough border I.");
	static_assert(DBORDERY >= 3, "DirOK expects a large enough border II.");
	assert(IN_DUNGEON_AREA(fx, fy));
	if (!PosOkMonst(mnum, fx, fy))
		return false;
	if (mdir == DIR_E) {
		if (nSolidTable[dPiece[fx][fy + 1]] || dFlags[fx][fy + 1] & BFLAG_MONSTLR)
			return false;
	} else if (mdir == DIR_W) {
		if (nSolidTable[dPiece[fx + 1][fy]] || dFlags[fx + 1][fy] & BFLAG_MONSTLR)
			return false;
	} else if (mdir == DIR_N) {
		if (nSolidTable[dPiece[fx + 1][fy]] || nSolidTable[dPiece[fx][fy + 1]])
			return false;
	} else if (mdir == DIR_S)
		if (nSolidTable[dPiece[fx - 1][fy]] || nSolidTable[dPiece[fx][fy - 1]])
			return false;
	if (monster[mnum].leaderflag == MLEADER_PRESENT) {
		if (abs(fx - monster[monster[mnum].leader]._mfutx) >= 4
		    || abs(fy - monster[monster[mnum].leader]._mfuty) >= 4) {
			return false;
		}
		return true;
	}
	if (monster[mnum]._uniqtype == 0 || !(UniqMonst[monster[mnum]._uniqtype - 1].mUnqAttr & 2))
		return true;
	mcount = 0;
	for (x = fx - 3; x <= fx + 3; x++) {
		for (y = fy - 3; y <= fy + 3; y++) {
			assert(IN_DUNGEON_AREA(x, y));
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
	return mcount == monster[mnum].packsize;
}

bool PosOkMissile(int x, int y)
{
	return !nMissileTable[dPiece[x][y]] && !(dFlags[x][y] & BFLAG_MONSTLR);
}

bool CheckNoSolid(int x, int y)
{
	return !nSolidTable[dPiece[x][y]];
}

/**
 * Walks from (x1; y1) to (x2; y2) and calls the Clear check for 
 * every position inbetween.
 * The target and source positions are NOT checked.
 * @returns TRUE if the Clear checks succeeded.
 */
bool LineClearF(bool (*Clear)(int, int), int x1, int y1, int x2, int y2)
{
	int dx, dy;
	int tmp, d, xyinc;

	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) >= abs(dy)) {
		if (dx == 0)
			return true;
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
				return true;
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
				return true;
		} while (Clear(x1, y1));
	}
	return false;
}

bool LineClear(int x1, int y1, int x2, int y2)
{
	return LineClearF(PosOkMissile, x1, y1, x2, y2);
}

/**
 * Same as LineClearF, only with a different Clear function.
 */
bool LineClearF1(bool (*Clear)(int, int, int), int mnum, int x1, int y1, int x2, int y2)
{
	int dx, dy;
	int tmp, d, xyinc;

	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) >= abs(dy)) {
		if (dx == 0)
			return true;
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
				return true;
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
				return true;
		} while (Clear(mnum, x1, y1));
	}
	return false;
}

void SyncMonsterAnim(int mnum)
{
	const MonsterData *MData;
	MonsterStruct *mon;
	MON_ANIM anim;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("SyncMonsterAnim: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	if ((unsigned)mon->_mMTidx >= MAX_LVLMTYPES) {
		dev_fatal("SyncMonsterAnim: Invalid monster type %d for %d", mon->_mMTidx, mnum);
	}
	mon->MType = &Monsters[mon->_mMTidx];
	mon->_mType = mon->MType->cmType;
	mon->_mAnims = mon->MType->cmAnims;
	mon->_mAnimWidth = mon->MType->cmWidth;
	mon->_mAnimXOffset = mon->MType->cmXOffset;
	MData = mon->MType->cmData;
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
		mon->_mAnimLen = mon->_mAnims[MA_ATTACK].aFrames;
		break;
	default:
		anim = MA_STAND;
		mon->_mAnimFrame = 1;
		mon->_mAnimLen = mon->_mAnims[MA_STAND].aFrames;
		break;
	}
	mon->_mAnimData = mon->_mAnims[anim].aData[mon->_mdir];
	mon->_mAnimFrameLen = mon->MData->mAnimFrameLen[anim];
}

void MissToMonst(int mi, int x, int y)
{
	int oldx, oldy;
	int newx, newy;
	int mnum, tnum;
	MissileStruct *mis;
	MonsterStruct *mon;

	if ((unsigned)mi >= MAXMISSILES) {
		dev_fatal("MissToMonst: Invalid missile %d", mi);
	}
	mis = &missile[mi];
	mnum = mis->_miSource;
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MissToMonst: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];
	dMonster[x][y] = mnum + 1;
	mon->_mx = x;
	mon->_my = y;
	mon->_mdir = mis->_miDir;
	MonStartStand(mnum, mon->_mdir);
	if (mon->_mType >= MT_INCIN && mon->_mType <= MT_HELLBURN) {
		MonStartFadein(mnum, mon->_mdir, false);
		return;
	}
	PlayEffect(mnum, 1);
	if (mon->_mType == MT_GLOOM)
		return;

	oldx = mis->_mix;
	oldy = mis->_miy;
	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER)) {
		tnum = dPlayer[oldx][oldy];
		if (tnum > 0) {
			tnum--;
			MonTryH2HHit(mnum, tnum, 500, mon->_mMinDamage2, mon->_mMaxDamage2);
			if (tnum == dPlayer[oldx][oldy] - 1 && (mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE)) {
				if (players[tnum]._pmode != PM_GOTHIT && players[tnum]._pmode != PM_DEATH)
					StartPlrHit(tnum, 0, true);
				newx = oldx + offset_x[mon->_mdir];
				newy = oldy + offset_y[mon->_mdir];
				if (PosOkPlayer(tnum, newx, newy)) {
					players[tnum]._px = newx;
					players[tnum]._py = newy;
					RemovePlrFromMap(tnum);
					dPlayer[newx][newy] = tnum + 1;
					FixPlayerLocation(tnum);
				}
			}
		}
	} else {
		tnum = dMonster[oldx][oldy];
		if (tnum > 0) {
			tnum--;
			MonTryM2MHit(mnum, tnum, 500, mon->_mMinDamage2, mon->_mMaxDamage2);
			if (mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE) {
				newx = oldx + offset_x[mon->_mdir];
				newy = oldy + offset_y[mon->_mdir];
				if (PosOkMonst(dMonster[oldx][oldy] - 1, newx, newy)) {
					mnum = dMonster[oldx][oldy];
					dMonster[oldx][oldy] = 0;
					dMonster[newx][newy] = mnum;
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

static bool monster_posok(int mnum, int x, int y)
{
	MissileStruct* mis;
	bool ret = true, fire = false;
	int mi = dMissile[x][y], i;

	if (mi == 0 || mnum < 0)
		return true;

#ifdef HELLFIRE
	bool lightning = false;

	if (mi > 0) {
		if (missile[mi - 1]._miType == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			fire = true;
		} else if (missile[mi - 1]._miType == MIS_LIGHTWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			lightning = true;
		}
	} else {
		for (i = 0; i < nummissiles; i++) {
			mis = &missile[missileactive[i]];
			if (mis->_mix == x && mis->_miy == y) {
				if (mis->_miType == MIS_FIREWALL) {
					fire = true;
				} else if (mis->_miType == MIS_LIGHTWALL) {
					lightning = true;
				}
			}
		}
	}
	if (fire && (monster[mnum].mMagicRes & MORS_FIRE_IMMUNE) != MORS_FIRE_IMMUNE)
		ret = false;
	if (lightning && (monster[mnum].mMagicRes & MORS_LIGHTNING_IMMUNE) != MORS_LIGHTNING_IMMUNE)
		ret = false;
#else
	if (mi > 0) {
		if (missile[mi - 1]._miType == MIS_FIREWALL) // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
			fire = true;
	} else {
		for (i = 0; i < nummissiles; i++) {
			mis = &missile[missileactive[i]];
			if (mis->_mix == x && mis->_miy == y
			 && mis->_miType == MIS_FIREWALL)
				fire = true;
		}
	}
	if (fire && (monster[mnum].mMagicRes & MORS_FIRE_IMMUNE) != MORS_FIRE_IMMUNE)
		ret = false;
#endif
	return ret;
}

bool PosOkMonst(int mnum, int x, int y)
{
	int oi;

	if ((nSolidTable[dPiece[x][y]] | dPlayer[x][y] | dMonster[x][y]) != 0)
		return false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return false;
	}

	return monster_posok(mnum, x, y);
}

bool PosOkMonst2(int mnum, int x, int y)
{
	int oi;

	if (nSolidTable[dPiece[x][y]])
		return false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			return false;
	}

	return monster_posok(mnum, x, y);
}

bool PosOkMonst3(int mnum, int x, int y)
{
	bool isdoor;
	int oi;

	isdoor = false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		isdoor = object[oi]._oDoorFlag;
		if (object[oi]._oSolidFlag && !isdoor) {
			return false;
		}
	}

	if ((nSolidTable[dPiece[x][y]] && !isdoor) || (dPlayer[x][y] | dMonster[x][y]) != 0)
		return false;

	return monster_posok(mnum, x, y);
}

static void ActivateSpawn(int mnum, int x, int y, int dir)
{
	dMonster[x][y] = mnum + 1;
	monster[mnum]._mx = x;
	monster[mnum]._my = y;
	//monster[mnum]._mfutx = x;
	//monster[mnum]._mfuty = y;
	//monster[mnum]._moldx = x;
	//monster[mnum]._moldy = y;
	MonStartSpStand(mnum, dir);
}

void SpawnSkeleton(int mnum, int x, int y, int dir)
{
	int i, nok;
	int monstok[NUM_DIRS];

	if (mnum == -1)
		return; // FALSE;

	if (PosOkMonst(-1, x, y)) {
		if (dir == DIR_NONE)
			dir = random_(11, NUM_DIRS);
		ActivateSpawn(mnum, x, y, dir);
		return; // TRUE;
	}

	nok = 0;
	for (i = 0; i < lengthof(offset_x); i++) {
		if (PosOkMonst(-1, x + offset_x[i], y + offset_y[i])) {
			monstok[nok] = i;
			nok++;
		}
	}
	if (nok == 0)
		return;
	dir = monstok[random_(12, nok)];
	ActivateSpawn(mnum, x + offset_x[dir], y + offset_y[dir], dir);

	// return TRUE;
}

int PreSpawnSkeleton()
{
	int i, n;
	int types[MAX_LVLMTYPES];

	n = 0;
	for (i = 0; i < nummtypes; i++) {
		if (IsSkel(Monsters[i].cmType)) {
			types[n] = i;
			n++;
		}
	}

	if (n == 0)
		return -1;

	n = types[random_(136, n)];
	n = AddMonster(0, 0, 0, n, false);
	if (n != -1)
		MonStartStand(n, 0);

	return n;
}

void TalktoMonster(int mnum, int pnum)
{
	MonsterStruct *mon;
	int iv;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("TalktoMonster: Invalid monster %d", mnum);
	}
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("TalktoMonster: Invalid player %d", pnum);
	}
	mon = &monster[mnum];
	if (mon->_mgoal == MGOAL_TALKING)
		return; // already talking
	mon->_mmode = MM_TALK;
	mon->_mListener = pnum;
	if (mon->_mAi == AI_SNOTSPIL) {
		assert(QuestStatus(Q_LTBANNER));
		if (quests[Q_LTBANNER]._qvar1 == 0) {
			assert(mon->mtalkmsg == TEXT_BANNER10);
			quests[Q_LTBANNER]._qvar1 = 1;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_LTBANNER, true);
		} else if (quests[Q_LTBANNER]._qvar1 == 1) {
			if (PlrHasItem(pnum, IDI_BANNER, &iv)) {
				if (pnum == mypnum) {
					PlrInvItemRemove(pnum, iv);
				}
				mon->mtalkmsg = TEXT_BANNER12;
			}
		} else if (quests[Q_LTBANNER]._qvar1 == 2) {
			mon->mtalkmsg = TEXT_BANNER12;
		}
		if (mon->mtalkmsg == TEXT_BANNER12) {
			// mon->_mVar8 = 0; // init MON_TIMER
			quests[Q_LTBANNER]._qvar1 = 3;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_LTBANNER, true);
		}
	} else if (mon->_mAi == AI_GARBUD) {
		assert(quests[Q_GARBUD]._qvar1 < 4);
		mon->mtalkmsg = TEXT_GARBUD1 + quests[Q_GARBUD]._qvar1;
		if (mon->mtalkmsg == TEXT_GARBUD1) {
			quests[Q_GARBUD]._qactive = QUEST_ACTIVE;
			quests[Q_GARBUD]._qlog = TRUE; // BUGFIX: (?) for other quests qactive and qlog go together, maybe this should actually go into the if above (fixed)
		} else if (mon->mtalkmsg == TEXT_GARBUD2) {
			SetRndSeed(mon->_mRndSeed);
			SpawnItem(mnum, players[pnum]._px, players[pnum]._py, true);
		} //else if (mon->mtalkmsg == TEXT_GARBUD4)
		//	mon->_mVar8 = 0; // init MON_TIMER
		quests[Q_GARBUD]._qvar1++;
		if (pnum == mypnum)
			NetSendCmdQuest(Q_GARBUD, true);
	} else if (mon->_mAi == AI_LACHDAN) {
		assert(QuestStatus(Q_VEIL));
		assert(mon->mtalkmsg != TEXT_NONE);
		if (quests[Q_VEIL]._qactive == QUEST_INIT) {
			quests[Q_VEIL]._qactive = QUEST_ACTIVE;
			quests[Q_VEIL]._qlog = TRUE;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_VEIL, true);
		} else if (quests[Q_VEIL]._qactive != QUEST_DONE && PlrHasItem(pnum, IDI_GLDNELIX, &iv)) {
			mon->mtalkmsg = TEXT_VEIL11;
			// mon->_mgoal = MGOAL_INQUIRING;
			//mon->_mVar8 = 0; // init MON_TIMER
			quests[Q_VEIL]._qactive = QUEST_DONE;
			if (pnum == mypnum) {
				PlrInvItemRemove(pnum, iv);
				NetSendCmdQuest(Q_VEIL, false);
			}
			SpawnUnique(UITEM_STEELVEIL, players[pnum]._px, players[pnum]._py, true, false);
		}
	} else if (mon->_mAi == AI_ZHAR) {
		if (quests[Q_ZHAR]._qactive == QUEST_INIT) {
			quests[Q_ZHAR]._qactive = QUEST_ACTIVE;
			quests[Q_ZHAR]._qvar1 = 1;
			quests[Q_ZHAR]._qlog = TRUE;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_ZHAR, true);
			iv = SPL_SWIPE;
			if (players[pnum]._pClass == PC_ROGUE)
				iv = SPL_POINT_BLANK;
			else if (players[pnum]._pClass == PC_SORCERER)
				iv = SPL_LIGHTNING;
			SetRndSeed(mon->_mRndSeed);
			CreateSpellBook(iv, players[pnum]._px, players[pnum]._py);
		} else if (quests[Q_ZHAR]._qvar1 == 1) {
			mon->mtalkmsg = TEXT_ZHAR2;
			//mon->_mVar8 = 0; // init MON_TIMER
			quests[Q_ZHAR]._qvar1 = 2;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_ZHAR, true);
		}
	}
}

void SpawnGolum(int mnum, int x, int y, int level)
{
	MonsterStruct *mon;

	static_assert(MAX_MINIONS == MAX_PLRS, "SpawnGolum requires that owner of a monster has the same id as the monster itself.");
	if ((unsigned)mnum >= MAXMONSTERS) {
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
	mon->_mArmorClass = 25;
	//mon->_mEvasion = 5;
	mon->_mmaxhp = 2 * (320 * level + players[mnum]._pMaxMana / 3);
	mon->_mhitpoints = mon->_mmaxhp;
	mon->_mHit = 5 * (level + 8) + 2 * players[mnum]._pLevel;
	mon->_mMinDamage = 2 * (level + 4);
	mon->_mMaxDamage = 2 * (level + 8);
	MonStartSpStand(mnum, 0);
	MonEnemy(mnum);
	if (mnum == mypnum) {
		NetSendCmdGolem(
		    mon->_mx,
		    mon->_my,
		    mon->_mdir,
		    mon->_menemy,
		    mon->_mhitpoints,
		    currLvl._dLevelIdx);
	}
}

bool CanTalkToMonst(int mnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("CanTalkToMonst: Invalid monster %d", mnum);
	}
	// TODO: merge with MonTalker?
	assert((monster[mnum]._mgoal != MGOAL_INQUIRING
		&& monster[mnum]._mgoal != MGOAL_TALKING)
		|| monster[mnum].mtalkmsg != TEXT_NONE);
	return monster[mnum]._mgoal == MGOAL_INQUIRING
		|| monster[mnum]._mgoal == MGOAL_TALKING;
}

bool CheckMonsterHit(int mnum, bool *ret)
{
	MonsterStruct *mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("CheckMonsterHit: Invalid monster %d", mnum);
	}
	mon = &monster[mnum];

	if (mon->mtalkmsg != TEXT_NONE || mon->_mmode == MM_CHARGE || mon->_mhitpoints < (1 << 6)
	 || (mon->_mAi == AI_SNEAK && mon->_mgoal == MGOAL_RETREAT)
	 || (mon->_mAi == AI_COUNSLR && mon->_mgoal != MGOAL_NORMAL)) {
		*ret = false;
		return true;
	}

	if (mon->_mAi == AI_GARG && mon->_mFlags & MFLAG_GARG_STONE) {
		mon->_mFlags &= ~(MFLAG_GARG_STONE | MFLAG_LOCK_ANIMATION);
		// mon->_mmode = MM_SATTACK;
		*ret = true;
		return true;
	}

	return false;
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
		monster[mnum]._menemyx = players[enemy]._pfutx;
		monster[mnum]._menemyy = players[enemy]._pfuty;
	} else {
		monster[mnum]._mFlags |= MFLAG_TARGETS_MONSTER;
		enemy -= MAX_PLRS;
		monster[mnum]._menemy = enemy;
		monster[mnum]._menemyx = monster[enemy]._mfutx;
		monster[mnum]._menemyy = monster[enemy]._mfuty;
	}
}

DEVILUTION_END_NAMESPACE
