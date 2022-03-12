/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */
#include "all.h"
#include "monstai.h"
#include "engine/render/cl2_render.hpp"

DEVILUTION_BEGIN_NAMESPACE

/* Limit the number of monsters to be placed. */
int totalmonsters;
/* Limit the number of monster-types on the current level by the required resources.
 * In CRYPT where the values are not valid).
 */
int monstimgtot;
/* Number of active monsters on the current level (minions are considered active). */
int nummonsters;
/* The list of the indices of the active monsters. */
int monstactive[MAXMONSTERS];
/* The data of the monsters on the current level. */
MonsterStruct monsters[MAXMONSTERS];
/* Monster types on the current level. */
MapMonData mapMonTypes[MAX_LVLMTYPES];
/* The number of monster types on the current level. */
int nummtypes;

static_assert(MAX_LVLMTYPES <= UCHAR_MAX, "Monster-type indices are stored in a BYTE fields.");
/* The number of skeleton-monster types on the current level. */
BYTE numSkelTypes;
/* The number of goat-monster types on the current level. */
BYTE numGoatTypes;
/* Skeleton-monster types on the current level. */
BYTE mapSkelTypes[MAX_LVLMTYPES];
/* Goat-monster types on the current level. */
BYTE mapGoatTypes[MAX_LVLMTYPES];

/* The next light-index to be used for the trn of a unique monster. */
BYTE uniquetrans;

/** 'leader' of monsters without leaders. */
static_assert(MAXMONSTERS <= UCHAR_MAX, "Leader of monsters are stored in a BYTE field.");
#define MON_NO_LEADER MAXMONSTERS

/** Light radius of unique monsters */
#define MON_LIGHTRAD 3

/** Maximum distance of the pack-monster from its leader. */
#define MON_PACK_DISTANCE 3

/** Number of the monsters in packs. */
#define MON_PACK_SIZE 9

/** Minimum tick delay between steps if the walk is not continuous. */
#define MON_WALK_DELAY 20

/** Check if the monster just finished a WALK (STAND_PREV_MODE, STAND_TICK)*/
#define MON_JUST_WALKED ((mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2) && mon->_mVar2 == 0)

/** Standard MAI check if the monster has other things to do. */
#define MON_ACTIVE (mon->_mmode != MM_STAND)
/**
 * Standard MAI check if the monster is (not) 'disturbed'.
 */
#define MON_RELAXED (mon->_msquelch < SQUELCH_LOW)

/** Maps from walking path step to facing direction. */
//const char walk2dir[9] = { 0, DIR_NE, DIR_NW, DIR_SE, DIR_SW, DIR_N, DIR_E, DIR_S, DIR_W };
/** Maps from monster intelligence factor to missile type. */
const BYTE counsmiss[4] = { MIS_FIREBOLT, MIS_CBOLTC, MIS_LIGHTNINGC, MIS_FIREBALL };

/* data */

/** Maps from monster walk animation frame num to monster velocity. */
#define MON_WALK_SHIFT 8
// MWVel[animLen - 1][2] = (TILE_WIDTH << MON_WALK_SHIFT) / animLen;
// MWVel[animLen - 1][1] = MWVel[animLen - 1][2] / 2;
// MWVel[animLen - 1][0] = ((TILE_HEIGHT << MON_WALK_SHIFT) / animLen) / 2;
const int MWVel[24][3] = {
	// clang-format off
	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (1 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (1 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 1 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (2 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (2 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 2 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (3 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (3 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 3 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (4 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (4 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 4 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (5 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (5 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 5 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (6 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (6 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 6 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (7 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (7 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 7 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (8 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (8 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 8 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (9 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (9 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 9 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (10 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (10 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 10 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (11 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (11 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 11 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (12 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (12 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 12 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (13 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (13 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 13 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (14 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (14 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 14 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (15 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (15 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 15 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (16 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (16 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 16 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (17 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (17 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 17 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (18 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (18 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 18 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (19 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (19 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 19 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (20 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (20 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 20 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (21 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (21 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 21 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (22 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (22 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 22 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (23 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (23 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 23 },

	{ (TILE_HEIGHT << MON_WALK_SHIFT) / (24 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / (24 * 2),
	  (TILE_WIDTH << MON_WALK_SHIFT) / 24 },
	// clang-format on
};
/** Maps from monster action to monster animation letter. */
const char animletter[NUM_MON_ANIM] = { 'n', 'w', 'a', 'h', 'd', 's' };
/** Maps from direction to delta X-offset. */
const int offset_x[NUM_DIRS] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to delta Y-offset. */
const int offset_y[NUM_DIRS] = { 1, 1, 1, 0, -1, -1, -1, 0 };

/** Maps from monster AI ID to monster AI function. */
void (*AiProc[])(int i) = {
/*AI_ZOMBIE*/	&MAI_Zombie,
/*AI_FAT*/		&MAI_Fat,
/*AI_SKELSD*/	&MAI_SkelSd,
/*AI_SKELBOW*/	&MAI_SkelBow,
/*AI_SCAV*/		&MAI_Scav,
/*AI_RHINO*/	&MAI_Rhino,
/*AI_GOATMC*/	&MAI_GoatMc,
/*AI_GOATBOW*/	&MAI_GoatBow,
/*AI_FALLEN*/	&MAI_Fallen,
/*AI_MAGMA*/	&MAI_Magma,
/*AI_SKELKING*/	&MAI_SkelKing,
/*AI_BAT*/		&MAI_Bat,
/*AI_GARG*/		&MAI_Garg,
/*AI_CLEAVER*/	&MAI_Cleaver,
/*AI_SUCC*/		&MAI_Succ,
/*AI_SNOWWICH*/	&MAI_SnowWich,
/*AI_HLSPWN*/	&MAI_HlSpwn,
/*AI_SOLBRNR*/	&MAI_SolBrnr,
/*AI_SNEAK*/	&MAI_Sneak,
/*AI_STORM*/	&MAI_Storm,
/*AI_FIREMAN*///&MAI_Fireman,
/*AI_GARBUD*/	&MAI_Garbud,
/*AI_ACID*/		&MAI_Acid,
/*AI_ACIDUNIQ*/	&MAI_AcidUniq,
/*AI_GOLUM*/	&MAI_Golem,
/*AI_ZHAR*/		&MAI_Zhar,
/*AI_SNOTSPIL*/	&MAI_SnotSpil,
/*AI_SNAKE*/	&MAI_Snake,
/*AI_COUNSLR*/	&MAI_Counselor,
/*AI_MEGA*/		&MAI_Mega,
/*AI_DIABLO*/	&MAI_Diablo,
/*AI_LAZARUS*/	&MAI_Lazarus,
/*AI_LAZHELP*/	&MAI_Lazhelp,
/*AI_LACHDAN*/	&MAI_Lachdanan,
/*AI_WARLORD*/	&MAI_Warlord,
#ifdef HELLFIRE
/*AI_FIREBAT*/	&MAI_Firebat,
/*AI_TORCHANT*/	&MAI_Torchant,
/*AI_HORKDMN*/	&MAI_Horkdemon,
/*AI_LICH*/		&MAI_Lich,
/*AI_ARCHLICH*/	&MAI_ArchLich,
/*AI_PSYCHORB*/	&MAI_PsychOrb,
/*AI_NECROMORB*/&MAI_NecromOrb,
/*AI_BONEDEMON*/&MAI_BoneDemon,
#endif
/*AI_STORM2*/	&MAI_Storm2,
};

static inline void InitMonsterTRN(AnimStruct (&anims)[NUM_MON_ANIM], const char* transFile)
{
	BYTE *tf, *cf;
	int i, j;
	const AnimStruct* as;

	// A TRN file contains a sequence of color transitions, represented
	// as indexes into a palette. (a 256 byte array of palette indices)
	// TODO: this filter should have been done in 'compile time'
	tf = cf = LoadFileInMem(transFile);
	for (i = 0; i < 256; i++) {
		if (*cf == 255) {
			*cf = 0;
		}
		cf++;
	}

	for (i = 0; i < NUM_MON_ANIM; i++) {
		as = &anims[i];
		if (as->aFrames > 1) {
			for (j = 0; j < lengthof(as->aData); j++) {
				Cl2ApplyTrans(as->aData[j], tf, as->aFrames);
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
	numSkelTypes = 0;
	numGoatTypes = 0;
	uniquetrans = COLOR_TRN_UNIQ;
	monstimgtot = 4000;
	totalmonsters = MAXMONSTERS;

	// simplify InitSync if this line is uncommented
	//memset(monsters, 0, sizeof(monsters));

	for (i = 0; i < MAXMONSTERS; i++) {
		monstactive[i] = i;
	}
}

static bool IsSkel(int mt)
{
	return (mt >= MT_WSKELAX && mt <= MT_XSKELAX)
	    || (mt >= MT_WSKELBW && mt <= MT_XSKELBW)
	    || (mt >= MT_WSKELSD && mt <= MT_XSKELSD);
}

static bool IsGoat(int mt)
{
	return (mt >= MT_NGOATMC && mt <= MT_GGOATMC)
	    || (mt >= MT_NGOATBW && mt <= MT_GGOATBW);
}

static int AddMonsterType(int type, BOOL scatter)
{
	int i;

	for (i = 0; i < nummtypes && mapMonTypes[i].cmType != type; i++)
		;

	if (i == nummtypes) {
		nummtypes++;
		if (IsGoat(type)) {
			mapGoatTypes[numGoatTypes] = i;
			numGoatTypes++;
		}
		if (IsSkel(type)) {
			mapSkelTypes[numSkelTypes] = i;
			numSkelTypes++;
		}
		mapMonTypes[i].cmType = type;
		mapMonTypes[i].cmPlaceScatter = FALSE;
		monstimgtot -= monfiledata[monsterdata[type].moFileNum].moImage;
		InitMonsterGFX(i);
		InitMonsterSND(i);
	}

	mapMonTypes[i].cmPlaceScatter |= scatter;
	return i;
}

void GetLevelMTypes()
{
	int i, mtype;
	int montypes[lengthof(AllLevels[0].dMonTypes)];
	const LevelData* lds;
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
			AddMonsterType(MT_NGOATMC, FALSE);
		if (QuestStatus(Q_ZHAR))
			AddMonsterType(MT_COUNSLR, FALSE);
		if (QuestStatus(Q_LTBANNER))
			AddMonsterType(MT_BFALLSP, FALSE);
		if (QuestStatus(Q_VEIL))
			AddMonsterType(MT_RBLACK, TRUE);
		if (QuestStatus(Q_WARLORD))
			AddMonsterType(MT_BTBLACK, TRUE);

		lds = &AllLevels[lvl];
		for (nt = 0; nt < lengthof(lds->dMonTypes); nt++) {
			mtype = lds->dMonTypes[nt];
			if (mtype == MT_INVALID)
				break;
			montypes[nt] = mtype;
		}

#if DEBUG_MODE
		if (monstdebug) {
			for (i = 0; i < debugmonsttypes; i++)
				AddMonsterType(DebugMonsters[i], TRUE);
			return;
		}
#endif
		while (monstimgtot > 0 && nt > 0 && nummtypes < MAX_LVLMTYPES) {
			for (i = 0; i < nt; ) {
				if (monfiledata[monsterdata[montypes[i]].moFileNum].moImage > monstimgtot) {
					montypes[i] = montypes[--nt];
					continue;
				}

				i++;
			}

			if (nt > 0) {
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
	MapMonData* cmon;
	const MonsterData* mdata;
	const MonFileData* mfdata;
	int mtype, anim, i;
	char strBuff[256];
	BYTE* celBuf;

	cmon = &mapMonTypes[midx];
	mtype = cmon->cmType;
	mdata = &monsterdata[mtype];
	mfdata = &monfiledata[mdata->moFileNum];
	cmon->cmWidth = mfdata->moWidth;
	cmon->cmXOffset = (mfdata->moWidth - TILE_WIDTH) >> 1;
	cmon->cmAFNum = mfdata->moAFNum;
	cmon->cmAFNum2 = mfdata->moAFNum2;
	cmon->cmData = mdata;

	auto &monAnims = cmon->cmAnims;
	// static_assert(lengthof(animletter) == lengthof(monsterdata[0].aFrames), "");
	for (anim = 0; anim < NUM_MON_ANIM; anim++) {
		monAnims[anim].aFrames = mfdata->moAnimFrames[anim];
		monAnims[anim].aFrameLen = mfdata->moAnimFrameLen[anim];
		if (mfdata->moAnimFrames[anim] > 0) {
			snprintf(strBuff, sizeof(strBuff), mfdata->moGfxFile, animletter[anim]);

			celBuf = LoadFileInMem(strBuff);
			assert(monAnims[anim].aCelData == NULL);
			monAnims[anim].aCelData = celBuf;

			if (mtype != MT_GOLEM || (anim != MA_SPECIAL && anim != MA_DEATH)) {
				for (i = 0; i < lengthof(monAnims[anim].aData); i++) {
					monAnims[anim].aData[i] = const_cast<BYTE*>(CelGetFrameStart(celBuf, i));
				}
			} else {
				for (i = 0; i < lengthof(monAnims[anim].aData); i++) {
					monAnims[anim].aData[i] = celBuf;
				}
			}
		}
	}


	if (mdata->mTransFile != NULL) {
		InitMonsterTRN(monAnims, mdata->mTransFile);
	}

	// copy walk animation to the stand animation of the golem (except aCelData and alignment)
	if (mtype == MT_GOLEM) {
		copy_pod(monAnims[MA_STAND].aData, monAnims[MA_WALK].aData);
		monAnims[MA_STAND].aFrames = monAnims[MA_WALK].aFrames;
		monAnims[MA_STAND].aFrameLen = monAnims[MA_WALK].aFrameLen;
	}

	// load optional missile-gfxs
	switch (mtype) {
	case MT_NMAGMA:
	case MT_YMAGMA:
	case MT_BMAGMA:
	case MT_WMAGMA:
		LoadMissileGFX(MFILE_MAGBALL);
		break;
	/*case MT_INCIN:
	case MT_FLAMLRD:
	case MT_DOOMFIRE:
	case MT_HELLBURN:
		LoadMissileGFX(MFILE_KRULL);
		break;*/
	case MT_STORM:
	case MT_RSTORM:
	case MT_STORML:
	case MT_MAEL:
		LoadMissileGFX(MFILE_THINLGHT);
		break;
	/*case MT_SUCCUBUS:
		LoadMissileGFX(MFILE_FLARE);
		LoadMissileGFX(MFILE_FLAREEXP);
		break;*/
	case MT_NACID:
	case MT_RACID:
	case MT_BACID:
	case MT_XACID:
#ifdef HELLFIRE
	case MT_SPIDLORD:
#endif
		LoadMissileGFX(MFILE_ACIDBF);
		LoadMissileGFX(MFILE_ACIDSPLA);
		LoadMissileGFX(MFILE_ACIDPUD);
		break;
	case MT_SNOWWICH:
		LoadMissileGFX(MFILE_SCUBMISB);
		LoadMissileGFX(MFILE_SCBSEXPB);
		break;
	case MT_HLSPWN:
		LoadMissileGFX(MFILE_SCUBMISD);
		LoadMissileGFX(MFILE_SCBSEXPD);
		break;
	case MT_SOLBRNR:
		LoadMissileGFX(MFILE_SCUBMISC);
		LoadMissileGFX(MFILE_SCBSEXPC);
		break;
	case MT_DIABLO:
		LoadMissileGFX(MFILE_FIREPLAR);
		break;
#ifdef HELLFIRE
	case MT_SKLWING:
	case MT_BONEDEMN:
		LoadMissileGFX(MFILE_MS_ORA_B);
		LoadMissileGFX(MFILE_EXORA1_B);
		break;
	case MT_PSYCHORB:
		LoadMissileGFX(MFILE_MS_ORA);
		LoadMissileGFX(MFILE_EXORA1);
		break;
	case MT_NECRMORB:
		LoadMissileGFX(MFILE_MS_REB_B);
		LoadMissileGFX(MFILE_EXYEL2_B);
		break;
	case MT_HORKDMN:
		LoadMissileGFX(MFILE_SPAWNS);
		break;
	case MT_LICH:
		LoadMissileGFX(MFILE_MS_ORA_A);
		LoadMissileGFX(MFILE_EXORA1_A);
		break;
	case MT_ARCHLICH:
		LoadMissileGFX(MFILE_MS_YEB_A);
		LoadMissileGFX(MFILE_EXYEL2_A);
		break;
#endif
	}
}

static void InitMonster(int mnum, int dir, int mtidx, int x, int y)
{
	MapMonData* cmon = &mapMonTypes[mtidx];
	MonsterStruct* mon = &monsters[mnum];

	mon->_mMTidx = mtidx;
	mon->_mdir = dir;
	SetMonsterLoc(mon, x, y);
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->MType = cmon;
	mon->_mType = cmon->cmType;
	mon->_mAnimWidth = cmon->cmWidth;
	mon->_mAnimXOffset = cmon->cmXOffset;
	mon->_mAFNum = cmon->cmAFNum;
	mon->_mAFNum2 = cmon->cmAFNum2;
	mon->mName = cmon->cmData->mName;
	mon->_mLevel = cmon->cmData->mLevel;
	mon->_mSelFlag = cmon->cmData->mSelFlag;
	mon->_mAi = cmon->cmData->mAi;
	mon->_mInt = cmon->cmData->mInt;
	mon->_mFlags = cmon->cmData->mFlags;
	mon->_mHit = cmon->cmData->mHit;
	mon->_mMinDamage = cmon->cmData->mMinDamage;
	mon->_mMaxDamage = cmon->cmData->mMaxDamage;
	mon->_mHit2 = cmon->cmData->mHit2;
	mon->_mMinDamage2 = cmon->cmData->mMinDamage2;
	mon->_mMaxDamage2 = cmon->cmData->mMaxDamage2;
	mon->_mMagic = cmon->cmData->mMagic;
	mon->_mMagic2 = cmon->cmData->mMagic2;
	mon->_mArmorClass = cmon->cmData->mArmorClass;
	mon->_mEvasion = cmon->cmData->mEvasion;
	mon->_mMagicRes = cmon->cmData->mMagicRes;
	mon->_mTreasure = cmon->cmData->mTreasure;
	mon->_mExp = cmon->cmData->mExp;
	mon->_mmaxhp = RandRange(cmon->cmData->mMinHP, cmon->cmData->mMaxHP) << 6;
	mon->_mAnims = cmon->cmAnims;
	mon->_mAnimData = cmon->cmAnims[MA_STAND].aData[dir];
	mon->_mAnimFrameLen = cmon->cmAnims[MA_STAND].aFrameLen;
	mon->_mAnimCnt = random_(88, mon->_mAnimFrameLen);
	mon->_mAnimLen = cmon->cmAnims[MA_STAND].aFrames;
	mon->_mAnimFrame = RandRange(1, mon->_mAnimLen);
	mon->_mmode = MM_STAND;
	mon->_mVar1 = MM_STAND; // STAND_PREV_MODE
	mon->_mVar2 = MON_WALK_DELAY + 1; // STAND_TICK
	//mon->_mVar3 = 0;	-- should be set before use
	//mon->_mVar4 = 0;
	//mon->_mVar5 = 0;
	//mon->_mVar6 = 0;
	//mon->_mVar7 = 0;
	//mon->_mVar8 = 0;
	mon->_msquelch = 0;
	mon->_mpathcount = 0;
	mon->_mWhoHit = 0;
	mon->_mgoal = MGOAL_NORMAL;
	//mon->_mgoalvar1 = 0;	-- should be set before use
	//mon->_mgoalvar2 = 0;
	//mon->_mgoalvar3 = 0;
	//mon->_menemy = 0;	-- should be set before use
	mon->_menemyx = 0;
	mon->_menemyy = 0;
	mon->_mListener = 0;
	mon->_mDelFlag = FALSE;
	//mon->_lastx = 0;	-- should be set before use
	//mon->_lasty = 0;
	mon->_mAISeed = mon->_mRndSeed = GetRndSeed();
	mon->mtalkmsg = TEXT_NONE;

	mon->_uniqtype = 0;
	mon->_uniqtrans = 0;
	mon->_udeadval = 0;
	mon->mlid = NO_LIGHT;

	mon->leader = MON_NO_LEADER;
	mon->leaderflag = MLEADER_NONE;
	mon->packsize = 0;
	mon->_mvid = NO_VISION;

	if (gnDifficulty == DIFF_NIGHTMARE) {
		mon->_mmaxhp = 2 * mon->_mmaxhp + (100 << 6);
		mon->_mLevel += NIGHTMARE_LEVEL_BONUS;
		mon->_mExp = 2 * (mon->_mExp + DIFFICULTY_EXP_BONUS);
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
		mon->_mLevel += HELL_LEVEL_BONUS;
		mon->_mExp = 4 * (mon->_mExp + DIFFICULTY_EXP_BONUS);
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
		mon->_mMagicRes = cmon->cmData->mMagicRes2;
	}

	if (!IsMultiGame) {
		mon->_mmaxhp >>= 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}
	mon->_mhitpoints = mon->_mmaxhp;

	mon->_mFlags |= MFLAG_NO_ENEMY;
}

/**
 * Check the location if a monster can be placed there in the init phase.
 * Must not consider the player's position, since it is already initialized
 * and messes up the pseudo-random generated dungeon.
 */
static bool MonstPlace(int xp, int yp)
{
	static_assert(DBORDERX >= MON_PACK_DISTANCE, "MonstPlace does not check IN_DUNGEON_AREA but expects a large enough border I.");
	static_assert(DBORDERY >= MON_PACK_DISTANCE, "MonstPlace does not check IN_DUNGEON_AREA but expects a large enough border II.");
	return (dMonster[xp][yp] | /*dPlayer[xp][yp] |*/ nSolidTable[dPiece[xp][yp]]
		 | (dFlags[xp][yp] & (BFLAG_ALERT | BFLAG_POPULATED))) == 0;
}

#ifdef HELLFIRE
void WakeUberDiablo()
{
	MonsterStruct* mon;

	if (!deltaload)
		PlayEffect(MAX_MINIONS, MS_DEATH);
	mon = &monsters[MAX_MINIONS];
	mon->_mArmorClass -= 50;
	//mon->_mEvasion -= 20;
	mon->_mLevel /= 2;
	mon->_mMagicRes = 0;
	mon->_mmaxhp /= 2;
	if (mon->_mhitpoints > mon->_mmaxhp)
		mon->_mhitpoints = mon->_mmaxhp;
}
#endif

static void PlaceMonster(int mnum, int mtidx, int x, int y)
{
	int dir;

	dMonster[x][y] = mnum + 1;

	dir = random_(90, NUM_DIRS);
	InitMonster(mnum, dir, mtidx, x, y);
}

static void PlaceGroup(int mtidx, int num, int leaderf, int leader)
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
			dMonster[monsters[nummonsters]._mx][monsters[nummonsters]._my] = 0;
		}

		if (leaderf & UMF_GROUP) {
			x1 = monsters[leader]._mx;
			y1 = monsters[leader]._my;
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
			 || ((leaderf & UMF_LEADER) && ((abs(x2 - x1) > MON_PACK_DISTANCE) || (abs(y2 - y1) > MON_PACK_DISTANCE)))) {
				continue;
			}
			xp = x2;
			yp = y2;
			if ((!MonstPlace(xp, yp)) || random_(0, 2) != 0)
				continue;

			PlaceMonster(nummonsters, mtidx, xp, yp);
			if (leaderf & UMF_GROUP) {
				monsters[nummonsters]._mmaxhp *= 2;
				monsters[nummonsters]._mhitpoints = monsters[nummonsters]._mmaxhp;
				monsters[nummonsters]._mInt = monsters[leader]._mInt;

				if (leaderf & UMF_LEADER) {
					monsters[nummonsters].leader = leader;
					monsters[nummonsters].leaderflag = MLEADER_PRESENT;
					monsters[nummonsters]._mAi = monsters[leader]._mAi;
				}
			}
			nummonsters++;
			placed++;
		}

		if (placed >= num) {
			break;
		}
	}

	if (leaderf & UMF_LEADER) {
		monsters[leader].leaderflag = MLEADER_SELF;
		monsters[leader].packsize = placed;
	}
}

static void PlaceUniqueMonst(int uniqindex, int miniontidx, int bosspacksize)
{
	int xp, yp, x, y;
	int uniqtype;
	int count2;
	char filestr[64];
	const UniqMonData* uniqm;
	MonsterStruct* mon;
	int count;
	static_assert(NUM_COLOR_TRNS <= UCHAR_MAX, "Color transform index stored in BYTE field.");
	if (uniquetrans >= NUM_COLOR_TRNS) {
		return;
	}

	for (uniqtype = 0; uniqtype < nummtypes; uniqtype++) {
		if (mapMonTypes[uniqtype].cmType == uniqMonData[uniqindex].mtype) {
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
		if (zharlib == -1)
			return;
		xp = 2 * themeLoc[zharlib].x + DBORDERX + 4;
		yp = 2 * themeLoc[zharlib].y + DBORDERY + 4;
		break;
	case UMT_SNOTSPIL:
		xp = 2 * setpc_x + DBORDERX + 8;
		yp = 2 * setpc_y + DBORDERY + 12;
		break;
	case UMT_LAZARUS:
		if (IsMultiGame) {
			xp = 2 * setpc_x + DBORDERX + 3;
			yp = 2 * setpc_y + DBORDERY + 6;
		} else {
			xp = DBORDERX + 16;
			yp = DBORDERY + 30;
		}
		break;
	case UMT_RED_VEX:
		if (IsMultiGame) {
			xp = 2 * setpc_x + DBORDERX + 5;
			yp = 2 * setpc_y + DBORDERY + 3;
		} else {
			xp = DBORDERX + 24;
			yp = DBORDERY + 29;
		}
		break;
	case UMT_BLACKJADE:
		if (IsMultiGame) {
			xp = 2 * setpc_x + DBORDERX + 5;
			yp = 2 * setpc_y + DBORDERY + 9;
		} else {
			xp = DBORDERX + 22;
			yp = DBORDERY + 33;
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
			for (x = xp - MON_PACK_DISTANCE; x <= xp + MON_PACK_DISTANCE; x++) {
				for (y = yp - MON_PACK_DISTANCE; y <= yp + MON_PACK_DISTANCE; y++) {
					if (MonstPlace(x, y)) {
						count2++;
					}
				}
			}

			if (count2 < 2 * MON_PACK_SIZE) {
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
	mon = &monsters[nummonsters];
	mon->_uniqtype = uniqindex + 1;
	static_assert(MAX_LIGHT_RAD >= MON_LIGHTRAD, "Light-radius of unique monsters are too high.");
#ifdef HELLFIRE
	if (uniqindex != UMT_HORKDMN)
#endif
		mon->mlid = AddLight(mon->_mx, mon->_my, MON_LIGHTRAD);

	uniqm = &uniqMonData[uniqindex];
	mon->_mLevel = uniqm->muLevel;

	mon->_mExp *= 2;
	mon->mName = uniqm->mName;
	mon->_mmaxhp = uniqm->mmaxhp << 6;

	mon->_mAi = uniqm->mAi;
	mon->_mInt = uniqm->mInt;
	mon->_mMinDamage = uniqm->mMinDamage;
	mon->_mMaxDamage = uniqm->mMaxDamage;
	mon->_mMinDamage2 = uniqm->mMinDamage2;
	mon->_mMaxDamage2 = uniqm->mMaxDamage2;
	mon->_mMagicRes = uniqm->mMagicRes;
	mon->mtalkmsg = uniqm->mtalkmsg;
	if (mon->mtalkmsg != TEXT_NONE)
		mon->_mgoal = MGOAL_INQUIRING;

	snprintf(filestr, sizeof(filestr), "Monsters\\Monsters\\%s.TRN", uniqm->mTrnName);
	LoadFileWithMem(filestr, ColorTrns[uniquetrans]);

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
		mon->_mLevel += NIGHTMARE_LEVEL_BONUS;
		mon->_mMinDamage = 2 * (mon->_mMinDamage + 2);
		mon->_mMaxDamage = 2 * (mon->_mMaxDamage + 2);
		mon->_mMinDamage2 = 2 * (mon->_mMinDamage2 + 2);
		mon->_mMaxDamage2 = 2 * (mon->_mMaxDamage2 + 2);
	} else if (gnDifficulty == DIFF_HELL) {
		mon->_mmaxhp = 4 * mon->_mmaxhp + (200 << 6);
		mon->_mLevel += HELL_LEVEL_BONUS;
		mon->_mMinDamage = 4 * mon->_mMinDamage + 6;
		mon->_mMaxDamage = 4 * mon->_mMaxDamage + 6;
		mon->_mMinDamage2 = 4 * mon->_mMinDamage2 + 6;
		mon->_mMaxDamage2 = 4 * mon->_mMaxDamage2 + 6;
	}
	if (!IsMultiGame) {
		mon->_mmaxhp = mon->_mmaxhp >> 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}
	mon->_mhitpoints = mon->_mmaxhp;

	nummonsters++;

	if (uniqm->mUnqAttr & UMF_NODROP)
		mon->_mTreasure = NO_DROP;
	if (uniqm->mUnqAttr & UMF_GROUP) {
		PlaceGroup(miniontidx, bosspacksize, uniqm->mUnqAttr, nummonsters - 1);
	}
}

static void PlaceUniques()
{
	int u, mt;

	for (u = 0; uniqMonData[u].mtype != MT_INVALID; u++) {
		if (uniqMonData[u].muLevelIdx != currLvl._dLevelIdx)
			continue;
		if (uniqMonData[u].mQuestId != Q_INVALID
		 && quests[uniqMonData[u].mQuestId]._qactive == QUEST_NOTAVAIL)
			continue;
		for (mt = 0; mt < nummtypes; mt++) {
			if (mapMonTypes[mt].cmType == uniqMonData[u].mtype) {
				PlaceUniqueMonst(u, mt, MON_PACK_SIZE - 1);
				break;
			}
		}
	}
}

static void PlaceSetMapMonsters()
{
	BYTE* setp;

	if (!currLvl._dSetLvl) {
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

		if (currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl && IsMultiGame) {
			setp = LoadFileInMem("Levels\\L4Data\\Vile1.DUN");
			SetMapMonsters(setp, setpc_x, setpc_y);
			mem_free_dbg(setp);

			AddMonsterType(MT_ADVOCATE, FALSE);
			AddMonsterType(MT_HLSPWN, FALSE);
			PlaceUniqueMonst(UMT_LAZARUS, 0, 0);
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
		}
	} else if (currLvl._dLevelIdx == SL_SKELKING) {
		PlaceUniqueMonst(UMT_SKELKING, 0, 0);
	} else if (currLvl._dLevelIdx == SL_VILEBETRAYER) {
		AddMonsterType(MT_ADVOCATE, FALSE);
		AddMonsterType(MT_HLSPWN, FALSE);
		PlaceUniqueMonst(UMT_LAZARUS, 0, 0);
		PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
		PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
	}
}

void InitMonsters()
{
	TriggerStruct* ts;
	int na;
	int i, j, xx, yy;
	int numplacemonsters;
	int mtidx;
	int numscattypes;
	int scatteridx[MAX_LVLMTYPES];
	const int tdx[4] = { -1, -1,  2,  2 };
	const int tdy[4] = { -1,  2, -1,  2 };

#if DEBUG_MODE
	if (IsMultiGame)
		CheckDungeonClear();
#endif
	if (!currLvl._dSetLvl) {
		for (i = 0; i < MAX_MINIONS; i++)
			AddMonster(0, 0, 0, 0, false);
	}
	// reserve the entry/exit area
	for (i = 0; i < numtrigs; i++) {
		ts = &trigs[i];
		if (ts->_tmsg == DVL_DWM_TWARPUP || ts->_tmsg == DVL_DWM_PREVLVL
		 || (ts->_tmsg == DVL_DWM_NEXTLVL && currLvl._dLevelIdx != DLV_HELL3)) {
			static_assert(MAX_LIGHT_RAD >= 15, "Tile reservation in InitMonsters requires at least 15 light radius.");
			for (j = 0; j < lengthof(tdx); j++)
				DoVision(ts->_tx + tdx[j], ts->_ty + tdy[j], 15, false);
		}
	}
	// place the setmap/setpiece monsters
	PlaceSetMapMonsters();
	if (!currLvl._dSetLvl) {
		// calculate the available space for monsters
		na = 0;
		for (xx = DBORDERX; xx < DSIZEX + DBORDERX; xx++)
			for (yy = DBORDERY; yy < DSIZEY + DBORDERY; yy++)
				if (!nSolidTable[dPiece[xx][yy]])
					na++;
		numplacemonsters = na / 30;
		if (IsMultiGame)
			numplacemonsters += numplacemonsters >> 1;
		if (numplacemonsters > MAXMONSTERS - (MAX_MINIONS + 6) - nummonsters)
			numplacemonsters = MAXMONSTERS - (MAX_MINIONS + 6) - nummonsters;
		totalmonsters = nummonsters + numplacemonsters;
		// place quest/unique monsters
		PlaceUniques();
		numscattypes = 0;
		for (i = 0; i < nummtypes; i++) {
			if (mapMonTypes[i].cmPlaceScatter) {
				scatteridx[numscattypes] = i;
				numscattypes++;
			}
		}
		i = currLvl._dLevelIdx;
		while (nummonsters < totalmonsters) {
			mtidx = scatteridx[random_(95, numscattypes)];
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
			PlaceGroup(mtidx, na, 0, 0);
		}
	}
	for (i = 0; i < numtrigs; i++) {
		ts = &trigs[i];
		if (ts->_tmsg == DVL_DWM_TWARPUP || ts->_tmsg == DVL_DWM_PREVLVL
		 || (ts->_tmsg == DVL_DWM_NEXTLVL && currLvl._dLevelIdx != DLV_HELL3)) {
			for (j = 0; j < lengthof(tdx); j++)
				DoUnVision(trigs[i]._tx + tdx[j], trigs[i]._ty + tdy[j], 15);
		}
	}
}

void SetMapMonsters(BYTE* pMap, int startx, int starty)
{
	uint16_t rw, rh, *lm;
	int i, j;
	int mtidx;

	if (currLvl._dSetLvl) {
		AddMonsterType(MT_GOLEM, FALSE);
		for (i = 0; i < MAX_MINIONS; i++)
			AddMonster(0, 0, 0, 0, false);
	}
	lm = (uint16_t*)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	lm += rw * rh; // skip dun
	rw <<= 1;
	rh <<= 1;
	lm += rw * rh; // skip items?

	startx *= 2;
	startx += DBORDERX;
	starty *= 2;
	starty += DBORDERY;
	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				assert(SwapLE16(*lm) < lengthof(MonstConvTbl) && MonstConvTbl[SwapLE16(*lm)] != 0);
				mtidx = AddMonsterType(MonstConvTbl[SwapLE16(*lm)], FALSE);
				PlaceMonster(nummonsters++, mtidx, i, j);
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

int AddMonster(int x, int y, int dir, int mtidx, bool InMap)
{
	if (nummonsters < MAXMONSTERS) {
		int mnum = monstactive[nummonsters++];
		if (InMap)
			dMonster[x][y] = mnum + 1;
		InitMonster(mnum, dir, mtidx, x, y);
		return mnum;
	}

	return -1;
}

static void NewMonsterAnim(int mnum, int anim, int md)
{
	MonsterStruct* mon = &monsters[mnum];
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
	char ai = monsters[mnum]._mAi;
	return ai == AI_SKELBOW || ai == AI_GOATBOW || ai == AI_SUCC || ai == AI_LAZHELP;
}

static void MonEnemy(int mnum)
{
	int i, tnum;
	int enemy, dist, best_dist;
	bool sameroom, bestsameroom;
	MonsterStruct *mon = &monsters[mnum], *tmon;
	const BYTE tv = dTransVal[mon->_mx][mon->_my];

	enemy = 0;
	best_dist = MAXDUNX + MAXDUNY;
	bestsameroom = false;
	if (mnum >= MAX_MINIONS) {
		for (i = 0; i < MAX_PLRS; i++) {
			if (!plx(i)._pActive || currLvl._dLevelIdx != plx(i)._pDunLevel ||
				plx(i)._pInvincible/*plx(i)._pLvlChanging || plx(i)._pHitPoints < (1 << 6)*/)
				continue;
			sameroom = tv == dTransVal[plx(i)._px][plx(i)._py];
			dist = std::max(abs(mon->_mx - plx(i)._px), abs(mon->_my - plx(i)._py));
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
			tmon = &monsters[i];
			if (tmon->_mhitpoints < (1 << 6))
				continue;
			if (MINION_INACTIVE(tmon))
				continue;
			dist = std::max(abs(mon->_mx - tmon->_mx), abs(mon->_my - tmon->_my));
			if (dist > 3 && !MonRanged(mnum)) {
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
			tmon = &monsters[tnum];
			if (tmon->_mhitpoints < (1 << 6))
				continue;
			if (MINION_INACTIVE(tmon))
				continue;
			if (CanTalkToMonst(tnum))
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
	// clear previous target-flags
	mon->_mFlags &= ~(MFLAG_TARGETS_MONSTER | MFLAG_NO_ENEMY);
	if (enemy != 0) {
		if (enemy > 0) {
			enemy--;
			mon->_menemyx = plx(enemy)._pfutx;
			mon->_menemyy = plx(enemy)._pfuty;
		} else {
			enemy = -(enemy + 1);
			mon->_mFlags |= MFLAG_TARGETS_MONSTER;
			mon->_menemyx = monsters[enemy]._mfutx;
			mon->_menemyy = monsters[enemy]._mfuty;
		}
		mon->_menemy = enemy;
	} else {
		mon->_mFlags |= MFLAG_NO_ENEMY;
	}
}

static int MonGetDir(int mnum)
{
	return GetDirection(monsters[mnum]._mx, monsters[mnum]._my, monsters[mnum]._menemyx, monsters[mnum]._menemyy);
}

static void FixMonLocation(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_moldx = mon->_mx;
	mon->_mfuty = mon->_moldy = mon->_my;
}

void AssertFixMonLocation(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	assert(mon->_mxoff == 0);
	assert(mon->_myoff == 0);
	assert(mon->_mfutx == mon->_mx);
	assert(mon->_moldx == mon->_mx);
	assert(mon->_mfuty == mon->_my);
	assert(mon->_moldy == mon->_my);
}

static void MonStartStand(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	NewMonsterAnim(mnum, MA_STAND, mon->_mdir);
	//FixMonLocation(mnum);
	mon->_mVar1 = mon->_mmode; // STAND_PREV_MODE : previous mode of the monster
	mon->_mVar2 = 0;           // STAND_TICK : the time spent on standing
	mon->_mmode = MM_STAND;
	//MonEnemy(mnum);
}

static void MonStartDelay(int mnum, int len)
{
	MonsterStruct* mon;

	if (len <= 0) {
		return;
	}

	mon = &monsters[mnum];
	mon->_mVar2 = len;      // DELAY_TICK : length of the delay
	mon->_mmode = MM_DELAY;
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
	MonsterStruct* mon;

	NewMonsterAnim(mnum, MA_SPECIAL, md);
	AssertFixMonLocation(mnum);
	mon = &monsters[mnum];
	mon->_mmode = MM_SPSTAND;
}

static void MonChangeLightOff(int mnum)
{
	MonsterStruct* mon;
	int lx, ly;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonChangeLightOff: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	lx = mon->_mxoff + 2 * mon->_myoff;
	ly = 2 * mon->_myoff - mon->_mxoff;

	lx = lx / 8;
	ly = ly / 8;

	CondChangeLightOff(mon->mlid, lx, ly);
}

/**
 * @brief Starting a move action towards NW, N, NE or W
 */
static void MonStartWalk1(int mnum, int xvel, int yvel, int xadd, int yadd)
{
	MonsterStruct* mon;
	int mx, my;

	mon = &monsters[mnum];
	mon->_mmode = MM_WALK;
	mon->_mVar4 = xvel;   // WALK_XVEL : velocity of the monster in the X-direction
	mon->_mVar5 = yvel;   // WALK_YVEL : velocity of the monster in the Y-direction
	mon->_mxoff = 0;
	mon->_myoff = 0;
	//mon->_mVar1 = xadd; // dx after the movement
	//mon->_mVar2 = yadd; // dy after the movement
	mon->_mVar6 = 0;    // MWALK_XOFF : _mxoff in a higher range
	mon->_mVar7 = 0;    // MWALK_YOFF : _myoff in a higher range
	//mon->_mVar8 = 0;    // Value used to measure progress for moving from one tile to another

	mx = mon->_mx;
	my = mon->_my;
	assert(mon->_moldx == mx);
	assert(mon->_moldy == my);

	mx += xadd;
	my += yadd;
	mon->_mfutx = mx;
	mon->_mfuty = my;

	dMonster[mx][my] = -(mnum + 1);
}

/**
 * @brief Starting a move action towards SW, S, SE or E
 */
static void MonStartWalk2(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd)
{
	MonsterStruct* mon;
	int mx, my;

	mon = &monsters[mnum];
	mon->_mmode = MM_WALK2;
	mon->_mVar4 = xvel;   // WALK_XVEL : velocity of the monster in the X-direction
	mon->_mVar5 = yvel;   // WALK_YVEL : velocity of the monster in the Y-direction
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	mon->_mVar6 = xoff << MON_WALK_SHIFT; // MWALK_XOFF : _mxoff in a higher range
	mon->_mVar7 = yoff << MON_WALK_SHIFT; // MWALK_YOFF : _myoff in a higher range
	//mon->_mVar8 = 0;         // Value used to measure progress for moving from one tile to another

	mx = mon->_mx;
	my = mon->_my;
	assert(mon->_moldx == mx);
	assert(mon->_moldy == my);
	dMonster[mx][my] = -(mnum + 1);
	mx += xadd;
	my += yadd;
	mon->_mx = mon->_mfutx = mx;
	mon->_my = mon->_mfuty = my;
	dMonster[mx][my] = mnum + 1;
	if (mon->mlid != NO_LIGHT && !(mon->_mFlags & MFLAG_HIDDEN)) {
		ChangeLightXY(mon->mlid, mx, my);
		MonChangeLightOff(mnum);
	}
}

static void MonStartAttack(int mnum)
{
	int md = MonGetDir(mnum);
	MonsterStruct* mon;

	NewMonsterAnim(mnum, MA_ATTACK, md);
	AssertFixMonLocation(mnum);
	mon = &monsters[mnum];
	mon->_mmode = MM_ATTACK;
}

static void MonStartRAttack(int mnum, int mitype)
{
	int md = MonGetDir(mnum);
	MonsterStruct* mon;

	NewMonsterAnim(mnum, MA_ATTACK, md);
	AssertFixMonLocation(mnum);
	mon = &monsters[mnum];
	mon->_mmode = MM_RATTACK;
	mon->_mVar1 = mitype; // RATTACK_SKILL
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
	MonsterStruct* mon;

	NewMonsterAnim(mnum, MA_SPECIAL, md);
	AssertFixMonLocation(mnum);
	mon = &monsters[mnum];
	mon->_mmode = MM_RSPATTACK;
	mon->_mVar1 = mitype; // SPATTACK_SKILL
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
	MonsterStruct* mon = &monsters[mnum];

	NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);

	mon->_mmode = MM_SPATTACK;
	AssertFixMonLocation(mnum);
}

void RemoveMonFromMap(int mnum)
{
	int x, y, mx, my, m1;

	mx = monsters[mnum]._moldx;
	my = monsters[mnum]._moldy;

	m1 = mnum + 1;

	static_assert(DBORDERX >= 1, "RemoveMonFromMap expects a large enough border I.");
	static_assert(DBORDERY >= 1, "RemoveMonFromMap expects a large enough border II.");
	for (x = mx - 1; x <= mx + 1; x++) {
		for (y = my - 1; y <= my + 1; y++) {
			if (abs(dMonster[x][y]) == m1)
				dMonster[x][y] = 0;
		}
	}
}

static void MonPlace(int mnum)
{
	MonsterStruct* mon;

	FixMonLocation(mnum);

	mon = &monsters[mnum];
	if (mon->mlid != NO_LIGHT && !(mon->_mFlags & MFLAG_HIDDEN))
		ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
	if (mon->_mvid != NO_VISION)
		ChangeVisionXY(mon->_mvid, mon->_mx, mon->_my);
	dMonster[mon->_mx][mon->_my] = mnum + 1;
}

static void MonStartGetHit(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];

	if (mon->_mmode == MM_DEATH)
		return;

	RemoveMonFromMap(mnum);
	MonPlace(mnum);

	NewMonsterAnim(mnum, MA_GOTHIT, mon->_mdir);

	mon->_mmode = MM_GOTHIT;
}

static void MonTeleport(int mnum)
{
	MonsterStruct* mon;
	int i, x, y, _mx, _my, rx;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonTeleport: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	//assert(mon->_mmode != MM_STONE);

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
			//RemoveMonFromMap(mnum);
			//assert(dMonster[mon->_mx][mon->_my] == 0);
			//dMonster[x][y] = mnum + 1;
			mon->_mx = x;
			mon->_my = y;
			mon->_mdir = OPPOSITE(rx);
			return;
		}
	}
}

static void MonFallenFear(int x, int y)
{
	MonsterStruct* mon;
	int i;

	for (i = 0; i < nummonsters; i++) {
		mon = &monsters[monstactive[i]];
		if (mon->_mAi == AI_FALLEN
		 && abs(x - mon->_mx) < 5
		 && abs(y - mon->_my) < 5
		 && mon->_mhitpoints >= (1 << 6)
		 && !MON_RELAXED) { // TODO: use LineClear instead to prevent retreat behind walls?
#if DEBUG
			assert(mon->_mAnims[MA_WALK].aFrames * mon->_mAnims[MA_WALK].aFrameLen * (8 - 2 * 0) < SQUELCH_MAX - SQUELCH_LOW);
#endif
			static_assert((8 - 2 * 0) * 12 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Fallen might relax with retreat goal.");
			mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
			mon->_mgoal = MGOAL_RETREAT;
			mon->_mgoalvar1 = 8 - 2 * mon->_mInt; // RETREAT_DISTANCE
			mon->_mdir = GetDirection(x, y, mon->_mx, mon->_my);
		}
	}
}

void MonGetKnockback(int mnum, int sx, int sy)
{
	MonsterStruct* mon = &monsters[mnum];
	int oldx, oldy, newx, newy, dir;

	if (mon->_mmode < MM_WALK || mon->_mmode > MM_WALK2) {
		if (mon->_mmode == MM_DEATH || mon->_mmode == MM_STONE)
			return;
		oldx = mon->_mx;
		oldy = mon->_my;
	} else {
		if (mon->_mAnimFrame > (mon->_mAnims[MA_WALK].aFrames >> 1)) {
			oldx = mon->_mfutx;
			oldy = mon->_mfuty;
		} else {
			oldx = mon->_moldx;
			oldy = mon->_moldy;
		}
	}

	dir = GetDirection(sx, sy, oldx, oldy);
	if (PathWalkable(oldx, oldy, dir2pdir[dir])) {
		newx = oldx + offset_x[dir];
		newy = oldy + offset_y[dir];
		if (PosOkMonster(mnum, newx, newy)) {
			mon->_mx = newx;
			mon->_my = newy;
			RemoveMonFromMap(mnum);
			MonPlace(mnum);
		}
	}

	if (mnum >= MAX_MINIONS)
		MonStartGetHit(mnum);
}

void MonStartHit(int mnum, int pnum, int dam, unsigned hitflags)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by player/trap", mnum);
	}
	mon = &monsters[mnum];
	if ((unsigned)pnum < MAX_PLRS) {
		mon->_mWhoHit |= 1 << pnum;
		if (pnum == mypnum) {
			NetSendCmdMonstDamage(mnum, mon->_mhitpoints);
		}
	}
	PlayEffect(mnum, MS_GOTHIT);
	if (mnum < MAX_MINIONS)
		return;
	if ((dam << ((hitflags & ISPL_STUN) ? 3 : 2)) >= mon->_mmaxhp && mon->_mmode != MM_STONE) {
		if ((unsigned)pnum < MAX_PLRS) {
			mon->_mFlags &= ~(MFLAG_TARGETS_MONSTER | MFLAG_NO_ENEMY);
			mon->_menemy = pnum;
			mon->_menemyx = plr._pfutx;
			mon->_menemyy = plr._pfuty;
			mon->_mdir = MonGetDir(mnum);
			if (mon->_mType == MT_BLINK)
				MonTeleport(mnum);
		}
		MonStartGetHit(mnum);
	}
}

static void MonDiabloDeath(int mnum, bool sendmsg)
{
	MonsterStruct* mon;
	int i, j;
	int mx, my;

	quests[Q_DIABLO]._qactive = QUEST_DONE;
	if (sendmsg) {
		NetSendCmdQuest(Q_DIABLO, false); // recipient should not matter
		NetSendCmdMonstCorpse(mnum); // remove the 'corpse' of diablo
	}
	for (i = 0; i < nummonsters; i++) {
		j = monstactive[i];
		// commented out because this is a pointless complexity
		//if (j == mnum)
		//	continue;
		if (j < MAX_MINIONS && MINION_NR_INACTIVE(j))
			continue;
		RemoveMonFromMap(j);
		MonPlace(j);
		mon = &monsters[j];
		//if (mon->_msquelch == 0)
		//	continue;
		mon->_mhitpoints = 0;
		if (mon->_mmode != MM_STONE) {
			NewMonsterAnim(j, MA_DEATH, mon->_mdir);
			mon->_mmode = MM_DEATH;
		}
	}
	mon = &monsters[mnum];
	mon->_mVar1 = 7 * gnTicksRate; // DIABLO_TICK
	mx = mon->_mx;
	my = mon->_my;
	PlaySfxLoc(USFX_DIABLOD, mx, my);
	static_assert(MAX_LIGHT_RAD >= 8, "MonDiabloDeath needs at least light-radius of 8.");
	AddLight(mx, my, 8);
	DoVision(mx, my, 8, true);

	// assert(currLvl._dLevelIdx == DLV_HELL4);
	for (i = 0; i < MAX_PLRS; i++) {
		// ensure the players are not processed and not sending level-delta
		// might produce a slight inconsistent state in case a player enters the level
		// while the diablo is still dying. TODO: find a better solution for hosted games?
		if (plx(i)._pActive && plx(i)._pDunLevel == DLV_HELL4) {
			plx(i)._pLvlChanging = TRUE;
			plx(i)._pInvincible = TRUE;
		}
	}
}

static void SpawnLoot(int mnum, bool sendmsg)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	SetRndSeed(mon->_mRndSeed);
	switch (mon->_uniqtype - 1) {
	case UMT_GARBUD:
		assert(QuestStatus(Q_GARBUD));
		CreateTypeItem(mon->_mx, mon->_my, true, ITYPE_MACE, IMISC_NONE, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
		return;
	case UMT_LAZARUS:
		//if (effect_is_playing(USFX_LAZ1))
			stream_stop();
		break;
#ifdef HELLFIRE
	case UMT_HORKDMN:
		if (quests[Q_GIRL]._qactive != QUEST_NOTAVAIL) {
			SpawnQuestItemAt(IDI_THEODORE, mon->_mx, mon->_my, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
			return;
		}
		break;
	case UMT_DEFILER:
		//if (effect_is_playing(USFX_DEFILER8))
			stream_stop();
		// quests[Q_DEFILER]._qlog = FALSE;
		quests[Q_DEFILER]._qactive = QUEST_DONE;
		if (sendmsg)
			NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
		SpawnQuestItemAt(IDI_MAPOFDOOM, mon->_mx, mon->_my, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
		return;
	case UMT_NAKRUL:
		//if (effect_is_playing(USFX_NAKRUL4) || effect_is_playing(USFX_NAKRUL5) || effect_is_playing(USFX_NAKRUL6))
			stream_stop();
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		quests[Q_NAKRUL]._qvar1 = 5; // set to higher than 4 so innocent monters are not 'woke'
		if (sendmsg)
			NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
		/*SpawnMagicItem(ITYPE_SWORD, ICURS_GREAT_SWORD, mon->_mx, mon->_my, sendmsg);
		SpawnMagicItem(ITYPE_STAFF, ICURS_WAR_STAFF, mon->_mx, mon->_my, sendmsg);
		SpawnMagicItem(ITYPE_BOW, ICURS_LONG_WAR_BOW, mon->_mx, mon->_my, sendmsg);*/
		SpawnMonItem(mnum, mon->_mx, mon->_my, sendmsg); // double reward
		break;
#endif
	}
	SpawnMonItem(mnum, mon->_mx, mon->_my, sendmsg);
}

static void M2MStartHit(int defm, int offm, int dam)
{
	MonsterStruct* dmon;

	if ((unsigned)defm >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by monster", defm);
	}
	dmon = &monsters[defm];
	if ((unsigned)offm < MAX_MINIONS) {
		static_assert(MAX_MINIONS == MAX_PLRS, "M2MStartHit requires that owner of a monster has the same id as the monster itself.");
		dmon->_mWhoHit |= 1 << offm;
		if (offm == mypnum) {
			NetSendCmdMonstDamage(defm, dmon->_mhitpoints);
		}
	}
	PlayEffect(defm, MS_GOTHIT);
	if (defm < MAX_MINIONS)
		return;
	if ((dam << 2) >= dmon->_mmaxhp && dmon->_mmode != MM_STONE) {
		//if (offm >= 0) {
			dmon->_mFlags &= ~(MFLAG_TARGETS_MONSTER | MFLAG_NO_ENEMY);
			dmon->_mFlags |= MFLAG_TARGETS_MONSTER;
			dmon->_menemy = offm;
			dmon->_menemyx = monsters[offm]._mfutx;
			dmon->_menemyy = monsters[offm]._mfuty;
			dmon->_mdir = MonGetDir(offm);
			if (dmon->_mType == MT_BLINK)
				MonTeleport(defm);
		//}
		MonStartGetHit(defm);
	}
}

static void MonstStartKill(int mnum, int mpnum, bool sendmsg)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonstStartKill: Invalid monster %d", mnum);
	}
	// fix the location of the monster before spawning loot or sending a message
	RemoveMonFromMap(mnum);
	MonPlace(mnum);

	mon = &monsters[mnum];
	mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
	mon->_mhitpoints = 0;
	dDead[mon->_mx][mon->_my] = 0;
	CheckQuestKill(mnum, sendmsg);
	if (sendmsg) {
		static_assert(MAXMONSTERS <= UCHAR_MAX, "MonstStartKill uses mnum as pnum, which must fit to BYTE.");
		NetSendCmdMonstKill(mnum, mpnum);
	}
	if (mnum >= MAX_MINIONS) {
		MonUpdateLeader(mnum);
		SpawnLoot(mnum, sendmsg);
	} else {
		AddUnVision(mon->_mvid);
	}

	if (mon->_mType == MT_DIABLO)
		MonDiabloDeath(mnum, sendmsg);
	else
		PlayEffect(mnum, MS_DEATH);

	if (mon->_mmode != MM_STONE) {
		mon->_mmode = MM_DEATH;
		// TODO: might want to turn towards the offending enemy. Might not, though...
		NewMonsterAnim(mnum, MA_DEATH, mon->_mdir);
	}
	MonFallenFear(mon->_mx, mon->_my);
#ifdef HELLFIRE
	if ((mon->_mType >= MT_NACID && mon->_mType <= MT_XACID) || mon->_mType == MT_SPIDLORD)
#else
	if (mon->_mType >= MT_NACID && mon->_mType <= MT_XACID)
#endif
		AddMissile(mon->_mx, mon->_my, 0, 0, 0, MIS_ACIDPUD, MST_MONSTER, mnum, 1);
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
}

void MonStartKill(int mnum, int pnum)
{
	MonstStartKill(mnum, pnum, pnum == mypnum || pnum == -1);
}

void MonSyncStartKill(int mnum, int x, int y, int pnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonSyncStartKill: Invalid monster %d", mnum);
	}
	if (monsters[mnum]._mmode == MM_DEATH ||
	(monsters[mnum]._mmode == MM_STONE && monsters[mnum]._mhitpoints == 0)) {
		return;
	}
	if (dMonster[x][y] == 0 || abs(dMonster[x][y]) == mnum + 1) {
		RemoveMonFromMap(mnum);
		//dMonster[x][y] = mnum + 1;
		//monsters[mnum]._mx = x;
		//monsters[mnum]._my = y;
		monsters[mnum]._moldx = x;
		monsters[mnum]._moldy = y;
	}

	MonstStartKill(mnum, pnum, false);
}

/*
 * Start fade in using the special effect of monsters.
 * Used by: Sneak, Fireman, Mage, DarkMage
 */
static void MonStartFadein(int mnum, int md, bool backwards)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartFadein: Invalid monster %d", mnum);
	}
	NewMonsterAnim(mnum, MA_SPECIAL, md);
	AssertFixMonLocation(mnum);

	mon = &monsters[mnum];
	mon->_mmode = MM_FADEIN;
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
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartFadeout: Invalid monster %d", mnum);
	}
	NewMonsterAnim(mnum, MA_SPECIAL, md);
	AssertFixMonLocation(mnum);

	mon = &monsters[mnum];
	mon->_mmode = MM_FADEOUT;
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
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonStartHeal: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	mon->_mAnimData = mon->_mAnims[MA_SPECIAL].aData[mon->_mdir];
	mon->_mAnimFrame = mon->_mAnims[MA_SPECIAL].aFrames;
	mon->_mFlags |= MFLAG_REV_ANIMATION;
	mon->_mmode = MM_HEAL;
	mon->_mVar1 = mon->_mmaxhp / (16 * RandRange(4, 8)); // HEAL_SPEED
}

static bool MonDoStand(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoStand: Invalid monster %d", mnum);
	}
	//if (((gdwGameLogicTurn + mnum) % 16) == 0)
	//	MonEnemy(mnum);

	mon = &monsters[mnum];
	mon->_mAnimData = mon->_mAnims[MA_STAND].aData[mon->_mdir];
	mon->_mVar2++; // STAND_TICK

	return false;
}

static bool MonDoWalk(int mnum)
{
	MonsterStruct* mon;
	bool rv;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoWalk: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAnimLen) {
		dMonster[mon->_moldx][mon->_moldy] = 0;
		mon->_mx = mon->_mfutx;
		mon->_my = mon->_mfuty;
		MonPlace(mnum);
		MonStartStand(mnum);
		rv = true;
	} else {
		//if (mon->_mAnimCnt == 0) {
#ifdef HELLFIRE
			if (mon->_mAnimFrame == 1 && mon->_mType == MT_FLESTHNG)
				PlayEffect(mnum, MS_SPECIAL);
#endif
			//mon->_mVar8++;
			mon->_mVar6 += mon->_mVar4; // MWALK_XOFF <- WALK_XVEL
			mon->_mVar7 += mon->_mVar5; // MWALK_YOFF <- WALK_YVEL
			mon->_mxoff = mon->_mVar6 >> MON_WALK_SHIFT;
			mon->_myoff = mon->_mVar7 >> MON_WALK_SHIFT;
			if (mon->mlid != NO_LIGHT && !(mon->_mFlags & MFLAG_HIDDEN))
				MonChangeLightOff(mnum);
		//}
		rv = false;
	}

	return rv;
}

static void MonHitMon(int offm, int defm, int hper, int mind, int maxd)
{
	bool ret;

	if ((unsigned)defm >= MAXMONSTERS) {
		dev_fatal("MonHitMon: Invalid monster %d", defm);
	}
	if (CheckMonsterHit(defm, &ret))
		return;

	hper = hper + (monsters[offm]._mLevel << 1) - (monsters[defm]._mLevel << 1);
	int hit = monsters[defm]._mmode == MM_STONE ? 0 : random_(4, 100);
	if (hit < hper) {
		int dam = RandRange(mind, maxd) << 6;
		monsters[defm]._mhitpoints -= dam;
		if (monsters[defm]._mhitpoints < (1 << 6)) {
			M2MStartKill(offm, defm);
		} else {
			M2MStartHit(defm, offm, dam);
		}
	}
}

static void MonHitPlr(int mnum, int pnum, int Hit, int MinDam, int MaxDam)
{
	MonsterStruct* mon;
	int tmp, dam, hper, blkper;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("MonHitPlr: Invalid player %d", pnum);
	}
	mon = &monsters[mnum];

	if (plr._pInvincible)
		return;

	hper = 30 + Hit
		+ (2 * mon->_mLevel)
		- plr._pIAC;
	if (random_(98, 100) >= hper)
#if DEBUG_MODE
		if (!debug_mode_god_mode)
#endif
			return;

	blkper = plr._pIBlockChance;
	if (blkper != 0
	 && (plr._pmode == PM_STAND || plr._pmode == PM_BLOCK)) {
		// assert(plr._pSkillFlags & SFLAG_BLOCK);
		blkper = blkper - (mon->_mLevel << 1);
		if (blkper > random_(98, 100)) {
			PlrStartBlock(pnum, OPPOSITE(mon->_mdir));
			return;
		}
	}
	if (mon->_mType == MT_YZOMBIE) {
		if (plr._pMaxHPBase > 64 && plr._pMaxHP > 64) {
			// FIXME: notify players on other level or add to CMD_*_JOINLEVEL?
			tmp = plr._pMaxHP - 64;
			plr._pMaxHP = tmp;
			if (plr._pHitPoints > tmp) {
				plr._pHitPoints = tmp;
			}
			tmp = plr._pMaxHPBase - 64;
			plr._pMaxHPBase = tmp;
			if (plr._pHPBase > tmp) {
				plr._pHPBase = tmp;
			}
		}
	}
	if (plr._pIFlags & ISPL_THORNS) {
		tmp = RandRange(1, 3) << 6;
		mon->_mhitpoints -= tmp;
		if (mon->_mhitpoints < (1 << 6))
			MonStartKill(mnum, pnum);
		else
			MonStartHit(mnum, pnum, tmp, 0);
	}
	dam = RandRange(MinDam, MaxDam) << 6;
	dam += plr._pIGetHit;
	if (dam < 64)
		dam = 64;
	if (mon->_mFlags & MFLAG_LIFESTEAL) {
		mon->_mhitpoints += dam;
		if (mon->_mhitpoints > mon->_mmaxhp)
			mon->_mhitpoints = mon->_mmaxhp;
	}
	if (PlrDecHp(pnum, dam, DMGTYPE_NPC)) {
		return;
	}
	StartPlrHit(pnum, dam, false, mon->_mdir);
	if (mon->_mFlags & MFLAG_KNOCKBACK) {
		KnockbackPlr(pnum, mon->_mdir);
	}
}

static void MonTryH2HHit(int mnum, int Hit, int MinDam, int MaxDam)
{
	MonsterStruct* mon;
	int mpnum;

	mon = &monsters[mnum];
	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER)) {
		mpnum = dPlayer[mon->_mx + offset_x[mon->_mdir]][mon->_my + offset_y[mon->_mdir]];
		if (mpnum == 0)
			return;
		mpnum = CheckPlrCol(mpnum);
		if (mpnum == -1)
			return;
		MonHitPlr(mnum, mpnum, Hit, MinDam, MaxDam);
	} else {
		mpnum = dMonster[mon->_mx + offset_x[mon->_mdir]][mon->_my + offset_y[mon->_mdir]];
		if (mpnum == 0)
			return;
		mpnum = CheckMonCol(mpnum);
		if (mpnum == -1)
			return;
		MonHitMon(mnum, mpnum, Hit, MinDam, MaxDam);
	}
}

static bool MonDoAttack(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoAttack: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum) {
		MonTryH2HHit(mnum, mon->_mHit, mon->_mMinDamage, mon->_mMaxDamage);
		if (mon->_mAi != AI_SNAKE)
			PlayEffect(mnum, MS_ATTACK);
	} else if (mon->_mAi == AI_MAGMA && mon->_mAnimFrame == 9) {
		// mon->_mType >= MT_NMAGMA && mon->_mType <= MT_WMAGMA
		MonTryH2HHit(mnum, mon->_mHit + 10, mon->_mMinDamage - 2, mon->_mMaxDamage - 2);
		PlayEffect(mnum, MS_ATTACK);
	} else if (mon->_mAi == AI_STORM2 && mon->_mAnimFrame == 13) {
		// mon->_mType >= MT_STORM && mon->_mType <= MT_MAEL
		MonTryH2HHit(mnum, mon->_mHit - 20, mon->_mMinDamage + 4, mon->_mMaxDamage + 4);
		PlayEffect(mnum, MS_ATTACK);
	} else if (mon->_mAi == AI_SNAKE && mon->_mAnimFrame == 1)
		PlayEffect(mnum, MS_ATTACK);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoRAttack(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoRAttack: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum) {
		AddMissile(
		    mon->_mx,
		    mon->_my,
		    mon->_menemyx,
		    mon->_menemyy,
		    mon->_mdir,
		    mon->_mVar1, // RATTACK_SKILL
		    MST_MONSTER,
		    mnum,
		    0);
		PlayEffect(mnum, MS_ATTACK);
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoRSpAttack(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoRSpAttack: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2) {
		if (mon->_mAnimCnt == 0) {
			AddMissile(
				mon->_mx,
				mon->_my,
				mon->_menemyx,
				mon->_menemyy,
				mon->_mdir,
				mon->_mVar1, // SPATTACK_SKILL
				MST_MONSTER,
				mnum,
				0);
			PlayEffect(mnum, MS_SPECIAL);
		}

		if (mon->_mAi == AI_MEGA) {
			if (mon->_mAnimCnt++ == 0) {
				mon->_mFlags |= MFLAG_LOCK_ANIMATION;
			} else if (mon->_mAnimCnt == 15) {
				mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
			}
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoSpAttack(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoSpAttack: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2)
		MonTryH2HHit(mnum, mon->_mHit2, mon->_mMinDamage2, mon->_mMaxDamage2);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoFadein(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoFadein: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame) // FADE_END
		return false;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	if (mon->mlid != NO_LIGHT) { // && !(mon->_mFlags & MFLAG_HIDDEN)) {
		ChangeLightRadius(mon->mlid, MON_LIGHTRAD);
		ChangeLightXYOff(mon->mlid, mon->_mx, mon->_my);
	}
	AssertFixMonLocation(mnum);
	MonStartStand(mnum);
	return true;
}

static bool MonDoFadeout(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoFadeout: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame) // FADE_END
		return false;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	//if (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN) {
		mon->_mFlags |= MFLAG_HIDDEN;
		if (mon->mlid != NO_LIGHT) {
			ChangeLightRadius(mon->mlid, 0);
		}
	//}
	AssertFixMonLocation(mnum);
	MonStartStand(mnum);
	return true;
}

static bool MonDoHeal(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoHeal: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_NOHEAL) {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		mon->_mmode = MM_SPATTACK;
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
			mon->_mmode = MM_SPATTACK;
		}
	}
	return false;
}

static bool MonDoTalk(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoTalk: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	AssertFixMonLocation(mnum);
	MonStartStand(mnum);
	mon->_mgoal = MGOAL_TALKING;
	if (effect_is_playing(alltext[mon->mtalkmsg].sfxnr))
		return false;
	InitQTextMsg(mon->mtalkmsg, !IsMultiGame /*mon->_mListener == mypnum*/); // MON_TIMER
	return false;
}

static bool MonDoGotHit(int mnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoGotHit: Invalid monster %d", mnum);
	}
	if (monsters[mnum]._mAnimFrame == monsters[mnum]._mAnimLen) {
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}
	return false;
}

/*
 * Disconnect monster from its pack/leader.
 */
void MonUpdateLeader(int mnum)
{
	MonsterStruct* mon;
	int i;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonUpdateLeader: Invalid monster %d", mnum);
	}
	if (monsters[mnum].leaderflag == MLEADER_NONE)
		return;
	if (monsters[mnum].leaderflag == MLEADER_SELF) {
		for (i = 0; i < nummonsters; i++) {
			mon = &monsters[monstactive[i]];
			if (/*mon->leaderflag != MLEADER_NONE && */mon->leader == mnum) {
				mon->leader = MON_NO_LEADER;
				mon->leaderflag = MLEADER_NONE;
			}
		}
	} else if (monsters[mnum].leaderflag == MLEADER_PRESENT) {
		monsters[monsters[mnum].leader].packsize--;
	}
	monsters[mnum].leader = MON_NO_LEADER;
	monsters[mnum].leaderflag = MLEADER_NONE;
	monsters[mnum].packsize = 0;
	// assert(monsters[mnum]._mvid == NO_VISION);
	monsters[mnum]._mvid = NO_VISION;
}

void DoEnding()
{
	// music_stop(); -- no need, music is already stopped at this point

	const char* vicSets[NUM_CLASSES] = {
		"gendata\\DiabVic2.smk", "gendata\\DiabVic3.smk", "gendata\\DiabVic1.smk"
#ifdef HELLFIRE
		, "gendata\\DiabVic1.smk", "gendata\\DiabVic3.smk", "gendata\\DiabVic2.smk"
#endif
	};
	play_movie(vicSets[myplr._pClass], 0);
	play_movie("gendata\\Diabend.smk", 0);

	// they tried to play TMUSIC_L2 in vanilla but failed, because
	// music is stopped/paused in play_movie
	play_movie("gendata\\loopdend.smk", MOV_SKIP | MOV_LOOP);
}

static void PrepDoEnding()
{
	unsigned killLevel;

	gbRunGame = false;
	gbDeathflag = MDM_ALIVE;
	gbCineflag = true;

	killLevel = gnDifficulty + 1;
	if (killLevel > myplr._pRank)
		myplr._pRank = killLevel;

	if (!IsMultiGame) {
		// save the hero + items
		pfile_write_hero(true);
		// delete the game
		pfile_delete_save_file(true);
	}
}

static bool MonDoDeath(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoDeath: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mType == MT_DIABLO) {
		if (mon->_mx != ViewX)
			ViewX += mon->_mx > ViewX ? 1 : -1;
		if (mon->_my != ViewY)
			ViewY += mon->_my > ViewY ? 1 : -1;

		if (--mon->_mVar1 == 0) // DIABLO_TICK
			PrepDoEnding();
	} else if (mon->_mAnimFrame == mon->_mAnimLen) {
		AddDead(mnum, DCMD_MON_INVALID);
	}
	return false;
}

static bool MonDoSpStand(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoSpStand: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2)
		PlayEffect(mnum, MS_SPECIAL);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoDelay(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoDelay: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	mon->_mdir = MonGetDir(mnum);
	mon->_mAnimData = mon->_mAnims[MA_STAND].aData[mon->_mdir];

	if (mon->_mVar2-- == 0) { // DELAY_TICK
		mon->_mmode = MM_STAND;
		mon->_mVar1 = MM_DELAY; // STAND_PREV_MODE
		//MonEnemy(mnum);
		return true;
	}

	return false;
}

static bool monster_posok(int mnum, int x, int y)
{
	if ((dFlags[x][y] & BFLAG_HAZARD) == 0)
		return true;
	return mnum < 0 || (monsters[mnum]._mMagicRes & MORS_FIRE_IMMUNE) == MORS_FIRE_IMMUNE;
}

static bool MonDoCharge(int mnum)
{
	MonsterStruct* mon;
	int dir;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoCharge: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	dir = mon->_mdir;
	if (!monster_posok(mnum, mon->_mx + offset_x[dir], mon->_my + offset_y[dir])) {
		//assert(dMonster[mon->_mx][mon->_my] == -(mnum + 1));
		dMonster[mon->_mx][mon->_my] = mnum + 1;
		AssertFixMonLocation(mnum);
		MonStartStand(mnum);
		return true;
	}
	return false;
}

static bool MonDoStone(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonDoStone: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mhitpoints == 0) {
		mon->_mDelFlag = TRUE;
		dMonster[mon->_mx][mon->_my] = 0;
	}
	return false;
}

void MonWalkDir(int mnum, int md)
{
	const int* mwi;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonWalkDir: Invalid monster %d", mnum);
	}
	NewMonsterAnim(mnum, MA_WALK, md);
	mwi = MWVel[monsters[mnum]._mAnimLen - 1];
	switch (md) {
	case DIR_N:
		MonStartWalk1(mnum, 0, -mwi[1], -1, -1);
		break;
	case DIR_NE:
		MonStartWalk1(mnum, mwi[1], -mwi[0], 0, -1);
		break;
	case DIR_E:
		MonStartWalk2(mnum, mwi[2], 0, -TILE_WIDTH, 0, 1, -1);
		break;
	case DIR_SE:
		MonStartWalk2(mnum, mwi[1], mwi[0], -TILE_WIDTH/2, -TILE_HEIGHT/2, 1, 0);
		break;
	case DIR_S:
		MonStartWalk2(mnum, 0, mwi[1], 0, -TILE_HEIGHT, 1, 1);
		break;
	case DIR_SW:
		MonStartWalk2(mnum, -mwi[1], mwi[0], TILE_WIDTH/2, -TILE_HEIGHT/2, 0, 1);
		break;
	case DIR_W:
		MonStartWalk1(mnum, -mwi[2], 0, -1, 1);
		break;
	case DIR_NW:
		MonStartWalk1(mnum, -mwi[1], -mwi[0], -1, 0);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
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
	mon = &monsters[mnum];
	// track/update enemy if still active
	if (mon->_msquelch != 0)
		MonEnemy(mnum);
	// check if the leader is still available and update its squelch value + enemy location
	if (mon->leader != MON_NO_LEADER) {
		leader = &monsters[mon->leader];
		clear = LineClearF(CheckNoSolid, mon->_mx, mon->_my, leader->_mfutx, leader->_mfuty);
		if (clear) {
			if (mon->leaderflag == MLEADER_AWAY
			 && abs(mon->_mx - leader->_mfutx) <= MON_PACK_DISTANCE
			 && abs(mon->_my - leader->_mfuty) <= MON_PACK_DISTANCE) {
				leader->packsize++;
				mon->leaderflag = MLEADER_PRESENT;
			}
		} else if (mon->leaderflag == MLEADER_PRESENT) {
			leader->packsize--;
			mon->leaderflag = MLEADER_AWAY;
		}
		if (mon->leaderflag == MLEADER_PRESENT) {
			if (mon->_msquelch > leader->_msquelch) {
				leader->_lastx = mon->_lastx; // BUGFIX: use _lastx instead of _mx (fixed)
				leader->_lasty = mon->_lasty; // BUGFIX: use _lasty instead of _my (fixed)
				leader->_msquelch = mon->_msquelch - 1;
			}
		}
	}
	// update squelch value + enemy location of the pack monsters
	if (mon->packsize != 0) {
		for (i = 0; i < nummonsters; i++) {
			bmon = &monsters[monstactive[i]];
			if (bmon->leaderflag == MLEADER_PRESENT && bmon->leader == mnum) {
				if (mon->_msquelch > bmon->_msquelch) {
					bmon->_lastx = mon->_lastx; // BUGFIX: use _lastx instead of _mx (fixed)
					bmon->_lasty = mon->_lasty; // BUGFIX: use _lasty instead of _my (fixed)
					bmon->_msquelch = mon->_msquelch - 1;
				}
			}
		}
	}
}

static bool MonCallWalk(int mnum, int md)
{
	int mdtemp;
	bool ok;

	// try desired direction
	ok = DirOK(mnum, md);
	if (!ok) {
		mdtemp = md;
		// Randomly go left or right
		if (random_(101, 2) != 0)
			ok = (md = (mdtemp - 1) & 7, DirOK(mnum, md))
			  || (md = (mdtemp + 1) & 7, DirOK(mnum, md));
		else
			ok = (md = (mdtemp + 1) & 7, DirOK(mnum, md))
			  || (md = (mdtemp - 1) & 7, DirOK(mnum, md));
		if (!ok) {
			// Randomly go further left or right
			if (random_(102, 2) != 0)
				ok = (md = (mdtemp + 2) & 7, DirOK(mnum, md))
				 || (md = (mdtemp - 2) & 7, DirOK(mnum, md));
			else
				ok = (md = (mdtemp - 2) & 7, DirOK(mnum, md))
				 || (md = (mdtemp + 2) & 7, DirOK(mnum, md));
		}
	}
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
	Check = (monsters[mnum]._mFlags & MFLAG_CAN_OPEN_DOOR) != 0 ? PosOkMonst3 : PosOkMonst;

	if (FindPath(Check, mnum, monsters[mnum]._mx, monsters[mnum]._my, monsters[mnum]._menemyx, monsters[mnum]._menemyy, path) != 0) {
		//MonCallWalk(mnum, walk2dir[path[0]]);
		return MonCallWalk(mnum, path[0]);
	}

	return false;
}

static bool MonDumbWalk(int mnum, int md)
{
	bool ok;

	ok = DirOK(mnum, md);
	if (ok)
		MonWalkDir(mnum, md);

	return ok;
}

static bool MonRoundWalk(int mnum, int md, int* dir)
{
	int mdtemp;
	bool ok;

	if (*dir)
		md = (md - 2) & 7;
	else
		md = (md + 2) & 7;

	ok = DirOK(mnum, md);
	mdtemp = md;
	if (!ok) {
		if (*dir) {
			ok = (md = (mdtemp + 1) & 7, DirOK(mnum, md))
			  || (md = (mdtemp + 2) & 7, DirOK(mnum, md));
		} else {
			ok = (md = (mdtemp - 1) & 7, DirOK(mnum, md))
			  || (md = (mdtemp - 2) & 7, DirOK(mnum, md));
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
	MonsterStruct* mon;
	int md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Zombie: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (random_(103, 100) < 2 * mon->_mInt + 10) {
		md = std::max(abs(mon->_mx - mon->_menemyx), abs(mon->_my - mon->_menemyy));
		if (md >= 2) {
			if (md >= 2 * mon->_mInt + 4) {
				md = mon->_mdir;
				if (random_(104, 100) < 2 * mon->_mInt + 20) {
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
	MonsterStruct* mon;
	int mx, my;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelSd: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	mx = mon->_mx;
	my = mon->_my;
	mon->_mdir = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	mx -= mon->_menemyx;
	my -= mon->_menemyy;
	if (abs(mx) >= 2 || abs(my) >= 2) { // STAND_PREV_MODE
		if (mon->_mVar1 == MM_DELAY || (random_(106, 100) >= 35 - 4 * mon->_mInt)) {
			MonCallWalk(mnum, mon->_mdir);
		} else {
			MonStartDelay(mnum, RandRange(15, 24) - 2 * mon->_mInt);
		}
	} else {
		if (mon->_mVar1 == MM_DELAY || (random_(105, 100) < 2 * mon->_mInt + 20)) {
			MonStartAttack(mnum);
		} else {
			MonStartDelay(mnum, RandRange(10, 19) - 2 * mon->_mInt);
		}
	}
}

static bool MAI_Path(int mnum)
{
	MonsterStruct* mon;
	bool clear;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Path: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return false;
	if (mon->_mgoal != MGOAL_NORMAL && mon->_mgoal != MGOAL_MOVE && mon->_mgoal != MGOAL_ATTACK2)
		return false;

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

	mon->_mpathcount = 0;

	return false;
}

void MAI_Snake(int mnum)
{
	MonsterStruct* mon;
	int fx, fy, mx, my, dist, md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Snake: Invalid monster %d", mnum);
	}
	const BYTE pattern[6] = { 1, 1, 0, 7, 7, 0 };
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my -fy));
	if (dist >= 2) { // STAND_PREV_MODE
		if (dist == 2 && LineClearF1(PosOkMonst, mnum, mx, my, fx, fy) && mon->_mVar1 != MM_CHARGE) {
			if (AddMissile(mx, my, fx, fy, md, MIS_RHINO, MST_MONSTER, mnum, 0) != -1) {
				PlayEffect(mnum, MS_ATTACK);
			}
		} else if (mon->_mVar1 == MM_DELAY || random_(106, 100) >= 35 - 2 * mon->_mInt) {
			// calculate the desired direction
			md = md + pattern[mon->_mgoalvar1]; // SNAKE_DIRECTION_DELTA
			md = md & 7;
			mon->_mgoalvar1++;
			if (mon->_mgoalvar1 >= lengthof(pattern))
				mon->_mgoalvar1 = 0;
			// check if it is matching with the current one
			md = md - mon->_mgoalvar2; // SNAKE_DIRECTION
			if (md != 0) {
				md = (md + NUM_DIRS) & 7;
				if (md < 4) {
					// adjust the direction to the right
					md = 1;
				} else if (md == 4) {
					// turn around
					md = 4;
				} else {
					// adjust the direction to the left
					md = 7;
				}
				mon->_mgoalvar2 = (mon->_mgoalvar2 + md) & 7;
			}
			if (!MonDumbWalk(mnum, mon->_mgoalvar2))
				MonCallWalk(mnum, mon->_mdir);
		} else {
			MonStartDelay(mnum, RandRange(15, 24) - mon->_mInt);
		}
	} else { // STAND_PREV_MODE
		if (mon->_mVar1 == MM_DELAY
		    || mon->_mVar1 == MM_CHARGE
		    || (random_(105, 100) < mon->_mInt + 20)) {
			MonStartAttack(mnum);
		} else
			MonStartDelay(mnum, RandRange(10, 19) - mon->_mInt);
	}
}

void MAI_Bat(int mnum)
{
	MonsterStruct* mon;
	int md, v, mx, my, fx, fy, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Bat: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_RETREAT) {
		if (mon->_mgoalvar1 == 0) { // RETREAT_FINISHED
			mon->_mgoalvar1++;
			MonCallWalk(mnum, OPPOSITE(md));
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			//MonCallWalk(mnum, random_(108, 2) != 0 ? left[md] : right[md]);
			MonCallWalk(mnum, (md + 2 * random_(108, 2) - 1) & 7);
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
	    && v < 4 * mon->_mInt + 33
	    && LineClearF1(PosOkMonst, mnum, mx, my, fx, fy)) {
		if (AddMissile(mx, my, fx, fy, md, MIS_RHINO, MST_MONSTER, mnum, 0) != -1) {
			MonUpdateLeader(mnum);
		}
	} else if (dist >= 2) {
		if ((mon->_mVar2 > MON_WALK_DELAY && v < mon->_mInt + 13) // STAND_TICK
		 || (MON_JUST_WALKED && v < mon->_mInt + 63)) {
			MonCallWalk(mnum, md);
		}
	} else if (v < 4 * mon->_mInt + 8) {
		MonStartAttack(mnum);
		mon->_mgoal = MGOAL_RETREAT;
		mon->_mgoalvar1 = 0; // RETREAT_FINISHED
		if (mon->_mType == MT_FAMILIAR) {
			AddMissile(mon->_menemyx, mon->_menemyy, 0, 0, -1, MIS_LIGHTNING, MST_MONSTER, mnum, -1);
		}
	}
}

void MAI_SkelBow(int mnum)
{
	MonsterStruct* mon;
	int mx, my, v;
	bool walking;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelBow: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;

	mon->_mdir = MonGetDir(mnum);
	v = random_(110, 100);

	walking = false;
	if (abs(mx) < 4 && abs(my) < 4) {
		if ((mon->_mVar2 > MON_WALK_DELAY && v < 2 * mon->_mInt + 13) // STAND_TICK
		 || (MON_JUST_WALKED && v < 2 * mon->_mInt + 63)) {
			walking = MonDumbWalk(mnum, OPPOSITE(mon->_mdir));
		}
	}

	if (!walking) {
		if (v < 2 * mon->_mInt + 3) {
			if (LineClear(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy))
				MonStartRAttack(mnum, MIS_ARROWC);
		}
	}
}

void MAI_Fat(int mnum)
{
	MonsterStruct* mon;
	int mx, my, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fat: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	mon->_mdir = MonGetDir(mnum);
	v = random_(111, 100);
	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if ((mon->_mVar2 > MON_WALK_DELAY && v < 4 * mon->_mInt + 20) // STAND_TICK
		 || (MON_JUST_WALKED && v < 4 * mon->_mInt + 70)) {
			MonCallWalk(mnum, mon->_mdir);
		}
	} else if (v < 4 * mon->_mInt + 15) {
		MonStartAttack(mnum);
	} else if (v < 4 * mon->_mInt + 20) {
		MonStartSpAttack(mnum);
	}
}

void MAI_Sneak(int mnum)
{
	MonsterStruct* mon;
	int mx, my, md;
	int dist, range, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Sneak: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;
	mx = mon->_mx;
	my = mon->_my;
	// commented out because dLight is not in-sync in multiplayer games and with the added
	// BFLAG_ALERT check there is not much point to this any more.
	// TODO: change MonstPlace to prefer non-lit tiles in case of AI_SNEAK?
	//if (dLight[mx][my] == MAXDARKNESS && (dFlags[mx][my] & BFLAG_ALERT) == 0)) {
	//	return;	
	//}
	mx -= mon->_menemyx;
	my -= mon->_menemyy;
	dist = std::max(abs(mx), abs(my));

	md = MonGetDir(mnum);
	range = 7 - mon->_mInt;
	if (mon->_mgoal != MGOAL_RETREAT) {
		if (mon->_mVar1 == MM_GOTHIT) { // STAND_PREV_MODE
			mon->_mgoal = MGOAL_RETREAT;
#if DEBUG
			assert(mon->_mAnims[MA_WALK].aFrames * mon->_mAnims[MA_WALK].aFrameLen * 9 < SQUELCH_MAX - SQUELCH_LOW);
#endif
			static_assert(12 * 9 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Sneak might relax with retreat goal.");
			mon->_mgoalvar1 = 9; // RETREAT_DISTANCE
		}
	} else {
		if (dist > range || --mon->_mgoalvar1 == 0) { // RETREAT_DISTANCE
			mon->_mgoal = MGOAL_NORMAL;
			//mon->_mgoalvar1 = 0;
		}
	}
	if (mon->_mgoal == MGOAL_RETREAT && !(mon->_mFlags & MFLAG_NO_ENEMY)) {
		md = mon->_menemy;
		if (mon->_mFlags & MFLAG_TARGETS_MONSTER)
			md = GetDirection(monsters[md]._mx, monsters[md]._my, mon->_mx, mon->_my);
		else
			md = GetDirection(plx(md)._px, plx(md)._py, mon->_mx, mon->_my);
		if (mon->_mType == MT_UNSEEN) {
			//md = random_(112, 2) != 0 ? left[md] : right[md];
			md = (md + 2 * random_(112, 2) - 1) & 7;
		}
	}
	mon->_mdir = md;
	v = random_(112, 100);
	range -= 2;
	if (dist < range && (mon->_mFlags & MFLAG_HIDDEN)) {
		MonStartFadein(mnum, mon->_mdir, false);
	} else if ((dist > range) && !(mon->_mFlags & MFLAG_HIDDEN)) {
		MonStartFadeout(mnum, mon->_mdir, true);
	} else {
		if (mon->_mgoal == MGOAL_RETREAT
		 || (dist >= 2 &&
			 ((mon->_mVar2 > MON_WALK_DELAY && v < 4 * mon->_mInt + 14) // STAND_TICK
			 || (MON_JUST_WALKED && v < 4 * mon->_mInt + 64)))) {
			MonCallWalk(mnum, mon->_mdir);
		}
		if (mon->_mmode == MM_STAND
		 && (dist < 2 && v < 4 * mon->_mInt + 10)) {
			MonStartAttack(mnum);
		}
	}
}

/*void MAI_Fireman(int mnum)
{
	MonsterStruct* mon;
	int mx, my, fx, fy, md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fireman: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	mx = mon->_mx;
	my = mon->_my;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	md = MonGetDir(mnum);
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (LineClear(mx, my, fx, fy)
		    && AddMissile(mx, my, fx, fy, md, MIS_FIREMAN, MST_MONSTER, mnum, 0) != -1) {
			mon->_mmode = MM_CHARGE;
			mon->_mgoal = MGOAL_ATTACK2;
			//mon->_mgoalvar1 = 0; // FIREMAN_ACTION_PROGRESS
		} else {
			mx -= fx;
			my -= fy;
			if (abs(mx) < 2 && abs(my) < 2) {
				MonTryH2HHit(mnum, mon->_mHit, mon->_mMinDamage, mon->_mMaxDamage);
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
}*/

void MAI_Fallen(int mnum)
{
	MonsterStruct* mon;
	int x, y, mx, my, m, rad, amount;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Fallen: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (random_(113, 48) == 0) {
			MonStartSpStand(mnum, mon->_mdir);
			if (!(mon->_mFlags & MFLAG_NOHEAL)) {
				rad = mon->_mhitpoints + 2 * mon->_mInt + 2;
				mon->_mhitpoints = std::min(mon->_mmaxhp, rad);
			}
			rad = 2 * mon->_mInt + 4;
#if DEBUG
			assert(mon->_mAnims[MA_WALK].aFrames * mon->_mAnims[MA_WALK].aFrameLen * (3 * 3 + 9) < SQUELCH_MAX - SQUELCH_LOW);
			assert(mon->_mAnims[MA_ATTACK].aFrames * mon->_mAnims[MA_ATTACK].aFrameLen * (3 * 3 + 9) < SQUELCH_MAX - SQUELCH_LOW);
#endif
			static_assert((3 * 3 + 9) * 13 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Fallen might relax with attack goal.");
			amount = 3 * mon->_mInt + 9;
			static_assert(DBORDERX == DBORDERY && DBORDERX >= 10, "MAI_Fallen expects a large enough border.");
			assert(rad <= DBORDERX);
			mx = mon->_mx;
			my = mon->_my;
			for (y = -rad; y <= rad; y++) {
				for (x = -rad; x <= rad; x++) {
					m = dMonster[x + mx][y + my];
					if (m > 0) {
						mon = &monsters[m - 1];
						if (mon->_mAi == AI_FALLEN && !MON_RELAXED) {
							mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
							mon->_mgoal = MGOAL_ATTACK2;
							mon->_mgoalvar1 = amount; // FALLEN_ATTACK_AMOUNT
						}
					}
				}
			}
		} else {
			MAI_SkelSd(mnum);
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		if (--mon->_mgoalvar1 != 0) { // RETREAT_DISTANCE
			MonCallWalk(mnum, mon->_mdir);
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			mon->_mdir = OPPOSITE(mon->_mdir);
			AssertFixMonLocation(mnum);
			MonStartStand(mnum);
		}
	} else {
		assert(mon->_mgoal == MGOAL_ATTACK2);
		if (--mon->_mgoalvar1 != 0) { // FALLEN_ATTACK_AMOUNT
			if (abs(mon->_mx - mon->_menemyx) < 2 && abs(mon->_my - mon->_menemyy) < 2) {
				MonStartAttack(mnum);
			} else {
				if (!MonCallWalk(mnum, MonGetDir(mnum))) {
					// prevent isolated fallens from burnout
					m = 12 - 1; // mon->_mAnims[MA_WALK].aFrameLen * mon->_mAnims[MA_WALK].aFrames - 1;
					if (mon->_msquelch > (unsigned)m)
						mon->_msquelch -= m;
				}
			}
		} else {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
}

void MAI_Cleaver(int mnum)
{
	MonsterStruct* mon;
	int mx, my;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Cleaver: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

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
	MonsterStruct* mon;
	int fx, fy;
	int mx, my, md;
	int dist, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Round: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	fy = mon->_menemyy;
	fx = mon->_menemyx;
	dist = std::max(abs(mx - fx), abs(my - fy));
	v = random_(114, 100);
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]*/) {
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
			if ((mon->_mVar2 > MON_WALK_DELAY && v < 2 * mon->_mInt + 28) // STAND_TICK
			 || (MON_JUST_WALKED && v < 2 * mon->_mInt + 78)) {
				MonCallWalk(mnum, md);
			}
		} else if (v < 2 * mon->_mInt + 23) {
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

static void MAI_Ranged(int mnum, int mitype, int attackMode)
{
	int fx, fy, mx, my, md;
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Ranged: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	if (mon->_msquelch >= SQUELCH_MAX - 1 /* || (mon->_mFlags & MFLAG_TARGETS_MONSTER)*/) {
		mon->_mdir = MonGetDir(mnum);
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		if (mon->_mVar1 == attackMode) { // STAND_PREV_MODE
			MonStartDelay(mnum, random_(118, 20));
		} else if (abs(mx) < 4 && abs(my) < 4) {
			if (random_(119, 100) < 10 * (mon->_mInt + 7))
				MonCallWalk(mnum, OPPOSITE(mon->_mdir));
		}
		if (mon->_mmode == MM_STAND) {
			if (LineClear(mon->_mx, mon->_my, fx, fy)) {
				if (attackMode == MM_RSPATTACK)
					MonStartRSpAttack(mnum, mitype);
				else
					MonStartRAttack(mnum, mitype);
			}
		}
	} else {
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		MonCallWalk(mnum, md);
	}
}

void MAI_GoatBow(int mnum)
{
	MAI_Ranged(mnum, MIS_ARROWC, MM_RATTACK);
}

void MAI_Succ(int mnum)
{
	MAI_Ranged(mnum, MIS_FLARE, MM_RATTACK);
}

void MAI_SnowWich(int mnum)
{
	MAI_Ranged(mnum, MIS_SNOWWICH, MM_RATTACK);
}

void MAI_HlSpwn(int mnum)
{
	MAI_Ranged(mnum, MIS_HLSPWN, MM_RATTACK);
}

void MAI_SolBrnr(int mnum)
{
	MAI_Ranged(mnum, MIS_SOLBRNR, MM_RATTACK);
}

#ifdef HELLFIRE
void MAI_Lich(int mnum)
{
	MAI_Ranged(mnum, MIS_LICH, MM_RATTACK);
}

void MAI_ArchLich(int mnum)
{
	MAI_Ranged(mnum, MIS_ARCHLICH, MM_RATTACK);
}

void MAI_PsychOrb(int mnum)
{
	MAI_Ranged(mnum, MIS_PSYCHORB, MM_RATTACK);
}

void MAI_NecromOrb(int mnum)
{
	MAI_Ranged(mnum, MIS_NECROMORB, MM_RATTACK);
}
#endif

void MAI_AcidUniq(int mnum)
{
	MAI_Ranged(mnum, MIS_ACID, MM_RSPATTACK);
}

#ifdef HELLFIRE
void MAI_Firebat(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBOLT, MM_RATTACK);
}

void MAI_Torchant(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBALL, MM_RATTACK);
}
#endif

void MAI_Scav(int mnum)
{
	MonsterStruct* mon;
	int i, j, tx, ty, maxhp, tmp;
	const char* cr;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Scav: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;
	if (mon->_mhitpoints < (mon->_mmaxhp >> 1) && mon->_mgoal != MGOAL_HEALING) {
		MonUpdateLeader(mnum);
		mon->_mgoal = MGOAL_HEALING;
		mon->_mgoalvar1 = 0; // HEALING_LOCATION_X
		//mon->_mgoalvar2 = 0;
#if DEBUG
		assert(mon->_mAnims[MA_SPECIAL].aFrames * mon->_mAnims[MA_SPECIAL].aFrameLen * 9 < SQUELCH_MAX - SQUELCH_LOW);
		assert(mon->_mAnims[MA_WALK].aFrames * mon->_mAnims[MA_WALK].aFrameLen * 9 < SQUELCH_MAX - SQUELCH_LOW);
		assert(24 * 9 < SQUELCH_MAX - SQUELCH_LOW); // max delay of MAI_SkelSd
#endif
		static_assert(24 * 9 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Scav might relax with healing goal.");
		mon->_mgoalvar3 = 9; // HEALING_ROUNDS
	}
	if (mon->_mgoal == MGOAL_HEALING) {
		if (mon->_mgoalvar3 != 0) {
			mon->_mgoalvar3--; // HEALING_ROUNDS
			if (dDead[mon->_mx][mon->_my] != 0 && dDead[mon->_mx][mon->_my] != STONENDX) {
				MonStartSpAttack(mnum);
				maxhp = mon->_mmaxhp;
				if (!(mon->_mFlags & MFLAG_NOHEAL)) {
#ifdef HELLFIRE
					mon->_mhitpoints += maxhp >> 3;
					if (mon->_mhitpoints > maxhp)
						mon->_mhitpoints = maxhp;
					if (mon->_mhitpoints == maxhp || mon->_mgoalvar3 == 0) { // HEALING_ROUNDS
						NetSendCmdMonstCorpse(mnum);
						dDead[mon->_mx][mon->_my] = 0;
					}
#else
					mon->_mhitpoints += 1 << 6;
					if (mon->_mhitpoints > maxhp)
						mon->_mhitpoints = maxhp;
					if (mon->_mhitpoints >= (maxhp >> 1) + (maxhp >> 2))
						mon->_mgoal = MGOAL_NORMAL;
#endif
				}
			} else {
				if (mon->_mgoalvar1 == 0) { // HEALING_LOCATION_X
					static_assert(DBORDERX >= 4, "MAI_Scav expects a large enough border I.");
					static_assert(DBORDERY >= 4, "MAI_Scav expects a large enough border II.");
					static_assert(MAXDUNX < UCHAR_MAX, "MAI_Scav stores dungeon coordinates in BYTE field I.");
					static_assert(MAXDUNY < UCHAR_MAX, "MAI_Scav stores dungeon coordinates in BYTE field II.");
					assert(CrawlTable[CrawlNum[4]] == 32);
					BYTE corpseLocs[32 * 2];
					tmp = 0;
					for (i = 1; i <= 4; i++) {
						cr = &CrawlTable[CrawlNum[i]];
						for (j = (BYTE)*cr; j > 0; j--) {
							tx = mon->_mx + *++cr;
							ty = mon->_my + *++cr;
							if (dDead[tx][ty] != 0 && dDead[tx][ty] != STONENDX
							 && LineClearF(CheckNoSolid, mon->_mx, mon->_my, tx, ty)) {
								corpseLocs[tmp] = tx;
								tmp++;
								corpseLocs[tmp] = ty;
								tmp++;
							}
						}
						if (tmp != 0) {
							tmp = random_(0, tmp);
							tmp &= ~1;
							mon->_mgoalvar1 = corpseLocs[tmp];     // HEALING_LOCATION_X
							mon->_mgoalvar2 = corpseLocs[tmp + 1]; // HEALING_LOCATION_Y
							break;
						}
					}
				}
				if (mon->_mgoalvar1 != 0) {
					//                                  HEALING_LOCATION_X, HEALING_LOCATION_Y
					tmp = GetDirection(mon->_mx, mon->_my, mon->_mgoalvar1, mon->_mgoalvar2);
					if (!MonCallWalk(mnum, tmp))
						mon->_mgoalvar3 = 0; // reset HEALING_ROUNDS to prevent back-and-forth with MAI_SkelSd
				}
			}
		}
		if (mon->_mhitpoints == mon->_mmaxhp) {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}

	if (mon->_mmode == MM_STAND)
		MAI_SkelSd(mnum);
}

void MAI_Garg(int mnum)
{
	MonsterStruct* mon;
	int mx, my, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Garg: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_GARG_STONE) {
		if (!MON_RELAXED) {
			//MonEnemy(mnum);
			mx = mon->_mx - mon->_menemyx;
			my = mon->_my - mon->_menemyy;
			dist = std::max(abs(mx), abs(my));
			// wake up if the enemy is close
			static_assert(DBORDERX + DBORDERY > (3 + 2) * 2, "MAI_Garg skips MFLAG_NO_ENEMY-check by assuming a monster is always 'far' from (0;0).");
			if (dist < mon->_mInt + 2) {
				mon->_mFlags &= ~(MFLAG_LOCK_ANIMATION | MFLAG_GARG_STONE);
				return;
			}
		}
		if (mon->_mmode != MM_SPATTACK) {
			if (mon->leaderflag == MLEADER_NONE) {
				MonStartSpAttack(mnum);
				mon->_mFlags |= MFLAG_LOCK_ANIMATION;
			} else {
				mon->_mFlags &= ~MFLAG_GARG_STONE;
			}
		}
		return;
	}

	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_mhitpoints < (mon->_mmaxhp >> 1))
#ifndef HELLFIRE
		if (!(mon->_mFlags & MFLAG_NOHEAL))
#endif
			mon->_mgoal = MGOAL_RETREAT;
	if (mon->_mgoal == MGOAL_RETREAT) {
		mx = mon->_mx - mon->_lastx;
		my = mon->_my - mon->_lasty;
		dist = std::max(abs(mx), abs(my));
		if (dist >= mon->_mInt + 2) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartHeal(mnum);
		} else if (mon->_mhitpoints == mon->_mmaxhp ||
		 !MonCallWalk(mnum, GetDirection(0, 0, mx, my))) {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	MAI_Round(mnum, false);
}

static void MAI_RoundRanged(int mnum, int mitype, int lessmissiles)
{
	MonsterStruct* mon;
	int mx, my;
	int fx, fy;
	int md, dist, v;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_RoundRanged: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	//v = random_(121, 10000);
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]*/) {
		if (mon->_mgoal == MGOAL_MOVE || (dist >= 3 && random_(122, 4 << lessmissiles) == 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 4 + RandRange(2, dist); // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(123, 2);        // MOVE_TURN_DIRECTION
			}
			/*if ((--mon->_mgoalvar1 <= 4 && DirOK(mnum, md)) || mon->_mgoalvar1 == 0) {
				mon->_mgoal = MGOAL_NORMAL;
			} else if (v < ((6 * (mon->_mInt + 1)) >> lessmissiles)
			    && (LineClear(mon->_mx, mon->_my, fx, fy))) {
				MonStartRSpAttack(mnum, mitype);
			} else {
				MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			}*/
			if (--mon->_mgoalvar1 > 4 || (mon->_mgoalvar1 > 0 && !DirOK(mnum, md))) { // MOVE_DISTANCE
				MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}
	if (mon->_mgoal == MGOAL_NORMAL) {
		v = random_(124, 100);
		if (((dist >= 3 && v < ((8 * (mon->_mInt + 2)) >> lessmissiles))
		        || v < ((8 * (mon->_mInt + 1)) >> lessmissiles))
		    && LineClear(mon->_mx, mon->_my, fx, fy)) {
			MonStartRSpAttack(mnum, mitype);
		} else if (dist >= 2) {
			if (v < 10 * (mon->_mInt + 5)
			 || (MON_JUST_WALKED && v < 10 * (mon->_mInt + 8))) {
				MonCallWalk(mnum, md);
			}
		} else if (v < 10 * (mon->_mInt + 6)) {
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
	MonsterStruct* mon;
	int mx, my, fx, fy;
	int dist, v, md;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_RR2: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	fx = mon->_menemyx;
	fy = mon->_menemyy;
	mx = mon->_mx;
	my = mon->_my;
	dist = std::max(abs(mx - fx), abs(my - fy));
	/*if (dist >= 5) {
		mon->_mgoal = MGOAL_NORMAL;
		MAI_SkelSd(mnum);
		return;
	}*/

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mx, my);
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(121, 100);
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX /*&& dTransVal[mx][my] == dTransVal[fx][fy]*/) {
		if (mon->_mgoal == MGOAL_MOVE || dist >= 3) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(123, 2); // MOVE_TURN_DIRECTION
			}
			mon->_mgoalvar3 = TRUE;                // MOVE_POSITIONED
			if (mon->_mgoalvar1++ < 2 * dist || !DirOK(mnum, md)) {
				if (v < 5 * (mon->_mInt + 16))
					MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else
				mon->_mgoal = MGOAL_NORMAL;
		}
	} else
		mon->_mgoal = MGOAL_NORMAL;
	if (mon->_mgoal == MGOAL_NORMAL) { // MOVE_POSITIONED
		if (dist < 5 && ((dist >= 3 && v < 5 * (mon->_mInt + 2)) || v < 5 * (mon->_mInt + 1) || mon->_mgoalvar3) && LineClear(mon->_mx, mon->_my, fx, fy)) {
			MonStartRSpAttack(mnum, mitype);
			return;
		}
		v = random_(124, 100);
		if (dist >= 2) {
			if (v < 10 * (mon->_mInt + 5)
			 || (MON_JUST_WALKED && v < 10 * (mon->_mInt + 8))) {
				MonCallWalk(mnum, md);
			}
		} else {
			if (v < 10 * (mon->_mInt + 4)) {
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
	MAI_RR2(mnum, MIS_INFERNOC);
}

void MAI_Golem(int mnum)
{
	MonsterStruct* mon;
	int md, i;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Golem: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	assert(!MINION_INACTIVE(mon));
	if (MON_ACTIVE) {
		//assert(mon->_mmode == MM_DEATH || mon->_mmode == MM_SPSTAND
		// || mon->_mmode == MM_ATTACK || mon->_mmode == MM_WALK || mon->_mmode == MM_WALK2);
		return;
	}
	mon->_msquelch = SQUELCH_MAX;

	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER))
		MonEnemy(mnum);

	if (!(mon->_mFlags & MFLAG_NO_ENEMY)) {
		if (abs(mon->_mx - mon->_menemyx) >= 2 || abs(mon->_my - mon->_menemyy) >= 2) {
			// assert(mon->_mgoal == MGOAL_NORMAL);
			mon->_mpathcount = 5; // make sure MonPathWalk is always called
			if (MAI_Path(mnum)) {
				return;
			}
		} else {
			MonStartAttack(mnum);
			return;
		}
	}

	md = plx(mnum)._pdir;
	if (!MonCallWalk(mnum, md)) {
		for (i = 0; i < NUM_DIRS; i++) {
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
	MonsterStruct* mon;
	int mx, my, fx, fy, nx, ny, md, v, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SkelKing: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(126, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2 || mon->_msquelch != SQUELCH_MAX) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 3 && random_(127, 4) == 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoal = MGOAL_MOVE;
			mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
			mon->_mgoalvar2 = random_(128, 2); // MOVE_TURN_DIRECTION
		}
		if ((mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md)) /*|| dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]*/) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			MonStartDelay(mnum, RandRange(10, 19));
		}
	}
		
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (!IsMultiGame
		    && ((dist >= 3 && v < 4 * mon->_mInt + 35) || v < 6)
		    && LineClear(mon->_mx, mon->_my, fx, fy)) {
			nx = mon->_mx + offset_x[md];
			ny = mon->_my + offset_y[md];
			if (PosOkMonst(mnum, nx, ny) && nummonsters < MAXMONSTERS) {
				MonSpawnSkel(nx, ny, md);
				MonStartSpStand(mnum, md);
			}
		} else if (dist < 2) {
			if (v < mon->_mInt + 20) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(129, 100);
			if (v < mon->_mInt + 25
			 || (MON_JUST_WALKED && v < mon->_mInt + 75)) {
				MonCallWalk(mnum, md);
			} else {
				MonStartDelay(mnum, RandRange(10, 19));
			}
		}
	}
}

void MAI_Rhino(int mnum)
{
	MonsterStruct* mon;
	int mx, my, fx, fy, md, v, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Rhino: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(131, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2 || mon->_msquelch != SQUELCH_MAX) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoal = MGOAL_MOVE;
			mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
			mon->_mgoalvar2 = random_(133, 2); // MOVE_TURN_DIRECTION
		}
		if (mon->_mgoalvar1++ >= 2 * dist /*|| dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]*/) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			MonStartDelay(mnum, RandRange(10, 19));
		}
	}
		
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist >= 5 && v < 2 * mon->_mInt + 43
		    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
			mon->_mdir = md;
			if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, MST_MONSTER, mnum, 0) != -1) {
				PlayEffect(mnum, MS_SPECIAL);
				MonUpdateLeader(mnum);
			}
		} else if (dist < 2) {
			if (v < 2 * mon->_mInt + 28) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mInt + 33
			 || (MON_JUST_WALKED && v < 2 * mon->_mInt + 83)) {
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
	MonsterStruct* mon;
	int mx, my, fx, fy, md, v, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Horkdemon: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	v = random_(131, 100);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (dist < 2 || mon->_msquelch != SQUELCH_MAX) {
		mon->_mgoal = MGOAL_NORMAL;
	} else if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
		if (mon->_mgoal != MGOAL_MOVE) {
			mon->_mgoal = MGOAL_MOVE;
			mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
			mon->_mgoalvar2 = random_(133, 2); // MOVE_TURN_DIRECTION
		}
		if (mon->_mgoalvar1++ >= 2 * dist /*|| dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]*/) {
			mon->_mgoal = MGOAL_NORMAL;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			MonStartDelay(mnum, RandRange(10, 19));
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (!IsMultiGame && dist >= 3 && v < 2 * mon->_mInt + 43) {
			if (PosOkMonst(mnum, mon->_mx + offset_x[mon->_mdir], mon->_my + offset_y[mon->_mdir]) && nummonsters < MAXMONSTERS) {
				MonStartRSpAttack(mnum, MIS_HORKDMN);
			}
		} else if (dist < 2) {
			if (v < 2 * mon->_mInt + 28) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mInt + 33
			 || (MON_JUST_WALKED && v < 2 * mon->_mInt + 83)) {
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
	MonsterStruct* mon;
	int mx, my, fx, fy, md, v, dist;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Counselor: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mx = mon->_mx;
	my = mon->_my;
	md = GetDirection(mx, my, mon->_lastx, mon->_lasty);
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	dist = std::max(abs(mx - fx), abs(my - fy));
	if (mon->_mgoal == MGOAL_NORMAL) {
		v = random_(121, 100);
		if (dist >= 2) {
			if (v < 5 * (mon->_mInt + 10) && LineClear(mon->_mx, mon->_my, fx, fy)) {
				MonStartRAttack(mnum, counsmiss[mon->_mInt]);
			} else if (random_(124, 100) < 30 && mon->_msquelch == SQUELCH_MAX) {
#if DEBUG
				assert(mon->_mAnims[MA_SPECIAL].aFrames * mon->_mAnims[MA_SPECIAL].aFrameLen * 2 + 
					mon->_mAnims[MA_WALK].aFrames * mon->_mAnims[MA_WALK].aFrameLen * (6 + 4) < SQUELCH_MAX - SQUELCH_LOW);
#endif
				static_assert(2 * 20 + (6 + 4) * 1 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Counselor might relax with move goal.");
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 6 + random_(0, std::min(dist, 4)); // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(125, 2);               // MOVE_TURN_DIRECTION
				MonStartFadeout(mnum, md, false);
			}
		} else {
			mon->_mdir = md;
			if (mon->_mVar1 == MM_FADEIN)
				v >>= 1;
			if (mon->_mVar1 != MM_FADEIN && mon->_mhitpoints < (mon->_mmaxhp >> 1)) {
#if DEBUG
				assert(mon->_mAnims[MA_SPECIAL].aFrames * mon->_mAnims[MA_SPECIAL].aFrameLen * 2 + 
					mon->_mAnims[MA_WALK].aFrames * mon->_mAnims[MA_WALK].aFrameLen * 5 < SQUELCH_MAX - SQUELCH_LOW);
#endif
				static_assert(2 * 20 + 5 * 1 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Counselor might relax with retreat goal.");
				mon->_mgoal = MGOAL_RETREAT;
				mon->_mgoalvar1 = 5; // RETREAT_DISTANCE
				MonStartFadeout(mnum, md, false);
			} else if (mon->_mVar1 == MM_DELAY // STAND_PREV_MODE
			    || v < 2 * mon->_mInt + 20) {
				MonStartRAttack(mnum, MIS_FLASH);
			}
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		if (--mon->_mgoalvar1 != 0) // RETREAT_DISTANCE
			MonCallWalk(mnum, OPPOSITE(md));
		else {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, md, true);
		}
	} else {
		assert(mon->_mgoal == MGOAL_MOVE);
		if (dist >= 2 /*&& mon->_msquelch == SQUELCH_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]*/
		 && (--mon->_mgoalvar1 > 4 || (mon->_mgoalvar1 > 0 && !DirOK(mnum, md)))) { // MOVE_DISTANCE
			MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, md, true);
		}
	}
	if (mon->_mmode == MM_STAND && mon->_mAi != AI_LAZARUS) {
		MonStartDelay(mnum, RandRange(10, 19) - 2 * mon->_mInt);
	}
}

void MAI_Garbud(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Garbud: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonGetDir(mnum);
	if (mon->_mgoal == MGOAL_TALKING) {
		if (dFlags[mon->_mx][mon->_my] & BFLAG_ALERT) { // MON_TIMER
			//if (quests[Q_GARBUD]._qvar1 == 4 && mon->_mVar8++ >= gnTicksRate * 6) {
			if (quests[Q_GARBUD]._qvar1 == 4 && (IsMultiGame || !effect_is_playing(USFX_GARBUD4))) {
				mon->_mgoal = MGOAL_NORMAL;
				// mon->_msquelch = SQUELCH_MAX;
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
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Zhar: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_TALKING) {
		if (quests[Q_ZHAR]._qvar1 == 1)
			mon->_mgoal = MGOAL_INQUIRING;
		if (dFlags[mon->_mx][mon->_my] & BFLAG_ALERT) { // MON_TIMER - also set in objects.cpp
			//if (quests[Q_ZHAR]._qvar1 == 2 && mon->_mVar8++ >= gnTicksRate * 4/*!effect_is_playing(USFX_ZHAR2)*/) {
			if (quests[Q_ZHAR]._qvar1 == 2 && (IsMultiGame || !effect_is_playing(USFX_ZHAR2))) {
				// mon->_msquelch = SQUELCH_MAX;
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
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_SnotSpil: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonGetDir(mnum);

	switch (quests[Q_LTBANNER]._qvar1) {
	case 0: // quest not started -> skip
		return;
	case 1: // quest just started -> waiting for the banner
		// switch to new text if the player(s) left
		if (mon->mtalkmsg == TEXT_BANNER10 && !(dFlags[mon->_mx][mon->_my] & BFLAG_ALERT))
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
		if (!IsMultiGame && effect_is_playing(alltext[TEXT_BANNER12].sfxnr))
			return;
		if (mon->_mListener == mypnum || !plx(mon->_mListener)._pActive || plx(mon->_mListener)._pDunLevel != currLvl._dLevelIdx) {
			NetSendCmd(CMD_OPENSPIL);
		}
		return;
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

void MAI_Lazarus(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lazarus: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonGetDir(mnum);
	if (IsMultiGame) {
		if (mon->_mgoal == MGOAL_INQUIRING) {
			if (quests[Q_BETRAYER]._qvar1 <= 3) {
				assert(mon->mtalkmsg == TEXT_VILE13);
				if (mon->_msquelch != SQUELCH_MAX)
					return;
				mon->_mmode = MM_TALK;
				// mon->_mListener = mypnum;
				quests[Q_BETRAYER]._qvar1 = 6;
				NetSendCmdQuest(Q_BETRAYER, true);
			} else {
				mon->mtalkmsg = TEXT_NONE;
				mon->_mgoal = MGOAL_NORMAL;
			}
		} else if (mon->_mgoal == MGOAL_TALKING) {
			mon->mtalkmsg = TEXT_NONE;
			mon->_mgoal = MGOAL_NORMAL;
		}
	} else {
		if (mon->_mgoal == MGOAL_INQUIRING) {
			if (mon->_msquelch != SQUELCH_MAX)
				return;
			// assert(quests[Q_BETRAYER]._qvar1 < 5);
			// assert(myplr._px == LAZ_CIRCLE_X && myplr._py == LAZ_CIRCLE_Y);
			PlayInGameMovie("gendata\\fprst3.smk");
			mon->_mmode = MM_TALK;
			// mon->_mListener = mypnum;
		} else if (mon->_mgoal == MGOAL_TALKING) {
			if (effect_is_playing(USFX_LAZ1) && myplr._px == LAZ_CIRCLE_X && myplr._py == LAZ_CIRCLE_Y)
				return;
			ObjChangeMap(7, 20, 11, 22/*, false*/);
			//RedoLightAndVision();
			// mon->_msquelch = SQUELCH_MAX;
			mon->mtalkmsg = TEXT_NONE;
			mon->_mgoal = MGOAL_NORMAL;
			quests[Q_BETRAYER]._qvar1 = 6;
		}
	}

	MAI_Counselor(mnum);
}

void MAI_Lazhelp(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lazhelp: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonGetDir(mnum);

	if (mon->_mgoal == MGOAL_INQUIRING || mon->_mgoal == MGOAL_TALKING) {
		if (!IsMultiGame && quests[Q_BETRAYER]._qvar1 <= 5)
			return;
		mon->mtalkmsg = TEXT_NONE;
		mon->_mgoal = MGOAL_NORMAL;
	}

	MAI_HlSpwn(mnum);
}

void MAI_Lachdanan(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Lachdanan: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE || MON_RELAXED)
		return;

	mon->_mdir = MonGetDir(mnum);

	if (quests[Q_VEIL]._qactive == QUEST_DONE) { // MON_TIMER
		//if (mon->_mVar8++ >= gnTicksRate * 32) {
		if (IsMultiGame || !effect_is_playing(USFX_LACH3)) {
			mon->mtalkmsg = TEXT_NONE;
			MonStartKill(mnum, -1);
		}
		return;
	}
	if (quests[Q_VEIL]._qactive == QUEST_ACTIVE) {
		if (mon->mtalkmsg == TEXT_VEIL9 && !(dFlags[mon->_mx][mon->_my] & BFLAG_ALERT))
			mon->mtalkmsg = TEXT_VEIL10;
	}
	//if (mon->_mgoal == MGOAL_TALKING)
		mon->_mgoal = MGOAL_INQUIRING;
}

void MAI_Warlord(int mnum)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MAI_Warlord: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonGetDir(mnum);

	switch (quests[Q_WARLORD]._qvar1) {
	case 0: // quest not started
		if (!(dFlags[mon->_mx][mon->_my] & BFLAG_ALERT))
			return;
		quests[Q_WARLORD]._qvar1 = 1;
		if (mon->_menemy == mypnum || !plx(mon->_menemy)._pActive || plx(mon->_menemy)._pDunLevel != currLvl._dLevelIdx) {
			NetSendCmdQuest(Q_WARLORD, true);
		}
		mon->_mmode = MM_TALK;
		mon->_mListener = mon->_menemy;
		//mon->_mVar8 = 0; // MON_TIMER
		return;
	case 1: // warlord spotted
		//if (mon->_mVar8++ < gnTicksRate * 8) // MON_TIMER
		//	return; // wait till the sfx is running, but don't rely on effect_is_playing
		if (!IsMultiGame && effect_is_playing(alltext[TEXT_WARLRD9].sfxnr))
			return;
		quests[Q_WARLORD]._qvar1 = 2;
		if (mon->_mListener == mypnum || !plx(mon->_mListener)._pActive || plx(mon->_mListener)._pDunLevel != currLvl._dLevelIdx) {
			NetSendCmdQuest(Q_WARLORD, true);
		}
		// mon->_msquelch = SQUELCH_MAX;
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
		if (monsters[i]._mDelFlag) {
			monsters[i]._mDelFlag = FALSE;
			monsters[i]._mx = 0;
			monsters[i]._my = 0;
			// do not reset mfutx/y to zero, otherwise monsters targeting this minion might try to
			// reach out of the dungeon. If necessary, add a check in ProcessMonsters to prevent the OOB.
			//monsters[i]._mfutx = 0;
			//monsters[i]._mfuty = 0;
			monsters[i]._moldx = 0;
			monsters[i]._moldy = 0;
			assert(MINION_NR_INACTIVE(i));
		}
	}

	for (i = MAX_MINIONS; i < nummonsters; ) {
		if (monsters[monstactive[i]]._mDelFlag) {
			DeleteMonster(i);
		} else {
			i++;
		}
	}
}

void ProcessMonsters()
{
	int i, mnum, _menemy;
	bool alert, hasenemy, raflag;
	MonsterStruct* mon;

	DeleteMonsterList();

	assert((unsigned)nummonsters <= MAXMONSTERS);
	for (i = 0; i < nummonsters; i++) {
		if (i < MAX_MINIONS && MINION_NR_INACTIVE(i))
			continue;
		mnum = monstactive[i];
		mon = &monsters[mnum];
		if (IsMultiGame) {
			SetRndSeed(mon->_mAISeed);
			mon->_mAISeed = GetRndSeed();
		}
		if (mon->_mhitpoints < mon->_mmaxhp && mon->_mhitpoints >= (1 << 6) && !(mon->_mFlags & MFLAG_NOHEAL)) {
			mon->_mhitpoints += (mon->_mLevel + 1) >> 1;
			if (mon->_mhitpoints > mon->_mmaxhp)
				mon->_mhitpoints = mon->_mmaxhp;
		}

		alert = (dFlags[mon->_mx][mon->_my] & BFLAG_ALERT) != 0;
		hasenemy = !(mon->_mFlags & MFLAG_NO_ENEMY);
		if (alert && !hasenemy) {
			MonEnemy(mnum);
			assert(!(mon->_mFlags & MFLAG_NO_ENEMY) || myplr._pInvincible);
			alert = hasenemy = !(mon->_mFlags & MFLAG_NO_ENEMY);
		}
		if (hasenemy) {
			_menemy = mon->_menemy;
			if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER)) {
				mon->_menemyx = plx(_menemy)._pfutx;
				mon->_menemyy = plx(_menemy)._pfuty;
			} else {
				mon->_menemyx = monsters[_menemy]._mfutx;
				mon->_menemyy = monsters[_menemy]._mfuty;
				alert = true;
			}
		}
		if (alert) {
			assert(hasenemy);
			mon->_lastx = mon->_menemyx;
			mon->_lasty = mon->_menemyy;
			if (mon->_msquelch == 0) {
				if (mon->_mType == MT_CLEAVER)
					PlaySfxLoc(USFX_CLEAVER, mon->_mx, mon->_my);
#ifdef HELLFIRE
				else if (mon->_mType == MT_NAKRUL)
					// quests[Q_NAKRUL]._qvar1 == 4 -> UberRoom was opened by the books
					PlaySfxLoc(quests[Q_JERSEY]._qactive != QUEST_NOTAVAIL ? USFX_NAKRUL6 : (quests[Q_NAKRUL]._qvar1 == 4 ? USFX_NAKRUL4 : USFX_NAKRUL5), mon->_mx, mon->_my);
				else if (mon->_mType == MT_DEFILER)
					PlaySfxLoc(USFX_DEFILER8, mon->_mx, mon->_my);
#endif
			}
			mon->_msquelch = SQUELCH_MAX;
		} else if (mon->_msquelch != 0 && mon->_mhitpoints == mon->_mmaxhp) {
			mon->_msquelch--;
			if (mon->_msquelch == 0) {
				// reset monster state to ensure sync in multiplayer games
				if (mon->_mmode == MM_DELAY)
					mon->_mmode = MM_STAND;
				if (mon->_mAi == AI_GARG) {
					mon->_mFlags |= MFLAG_GARG_STONE;
					assert(mon->_mmode == MM_STAND || mon->_mmode == MM_SPATTACK);
				} else {
					assert(mon->_mmode == MM_STAND);
				}
				mon->_mVar1 = MM_STAND; // STAND_PREV_MODE
				mon->_mVar2 = MON_WALK_DELAY + 1; // STAND_TICK
				mon->_mpathcount = 0;
				assert(mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_INQUIRING || mon->_mgoal == MGOAL_TALKING);
				mon->_mFlags |= MFLAG_NO_ENEMY;
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
			case MM_SPATTACK:
				raflag = MonDoSpAttack(mnum);
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
				raflag = MonDoCharge(mnum);
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
	int i, j;

	for (i = 0; i < nummtypes; i++) {
		for (j = 0; j < NUM_MON_ANIM; j++) {
			MemFreeDbg(mapMonTypes[i].cmAnims[j].aCelData);
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

	x = monsters[mnum]._mx;
	y = monsters[mnum]._my;
	if (!PathWalkable(x, y, dir2pdir[mdir]))
		return false;

	fx = x + offset_x[mdir];
	fy = y + offset_y[mdir];
	static_assert(DBORDERX >= 3, "DirOK expects a large enough border I.");
	static_assert(DBORDERY >= 3, "DirOK expects a large enough border II.");
	assert(IN_DUNGEON_AREA(fx, fy));
	if (!PosOkMonst(mnum, fx, fy))
		return false;

	if (monsters[mnum].leaderflag == MLEADER_PRESENT) {
		return abs(fx - monsters[monsters[mnum].leader]._mfutx) < 4
		    && abs(fy - monsters[monsters[mnum].leader]._mfuty) < 4;
	}
	if (monsters[mnum].packsize == 0)
		return true;
	mcount = 0;
	for (x = fx - 3; x <= fx + 3; x++) {
		for (y = fy - 3; y <= fy + 3; y++) {
			assert(IN_DUNGEON_AREA(x, y));
			ma = dMonster[x][y];
			if (ma == 0)
				continue;
			ma = ma >= 0 ? ma - 1 : -(ma + 1);
			if (monsters[ma].leaderflag == MLEADER_PRESENT
			    && monsters[ma].leader == mnum
			    && monsters[ma]._mfutx == x
				&& monsters[ma]._mfuty == y) {
				mcount++;
			}
		}
	}
	return mcount == monsters[mnum].packsize;
}

bool CheckAllowMissile(int x, int y)
{
	return !nMissileTable[dPiece[x][y]];
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
	return LineClearF(CheckAllowMissile, x1, y1, x2, y2);
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
	const MonsterData* MData;
	MonsterStruct* mon;
	int mode;
	MON_ANIM anim;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("SyncMonsterAnim: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if ((unsigned)mon->_mMTidx >= MAX_LVLMTYPES) {
		dev_fatal("SyncMonsterAnim: Invalid monster type %d for %d", mon->_mMTidx, mnum);
	}
	mon->MType = &mapMonTypes[mon->_mMTidx];
	mon->_mType = mon->MType->cmType;
	mon->_mAnims = mon->MType->cmAnims;
	mon->_mAnimWidth = mon->MType->cmWidth;
	mon->_mAnimXOffset = mon->MType->cmXOffset;
	MData = mon->MType->cmData;
	if (MData == NULL) {
		dev_fatal("SyncMonsterAnim: Monster %d \"%s\" MData NULL", mon->_mMTidx, mon->mName);
	}
	if (mon->_uniqtype != 0)
		mon->mName = uniqMonData[mon->_uniqtype - 1].mName;
	else
		mon->mName = MData->mName;

	mode = mon->_mmode;
	if (mode == MM_STONE)
		mode = mon->_mVar3;
	switch (mode) {
	case MM_WALK:
	case MM_WALK2:
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
	case MM_SPATTACK:
	case MM_FADEIN:
	case MM_FADEOUT:
	case MM_SPSTAND:
	case MM_RSPATTACK:
	case MM_HEAL:
		anim = MA_SPECIAL;
		break;
	case MM_STAND:
	case MM_DELAY:
	case MM_CHARGE:
	case MM_TALK:
		anim = MA_STAND;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	mon->_mAnimData = mon->_mAnims[anim].aData[mon->_mdir];
	mon->_mAnimFrameLen = mon->_mAnims[anim].aFrameLen;
	mon->_mAnimLen = mon->_mAnims[anim].aFrames;
}

void MissToMonst(int mi)
{
	int oldx, oldy;
	int mnum, tnum;
	MissileStruct* mis;
	MonsterStruct* mon;

	if ((unsigned)mi >= MAXMISSILES) {
		dev_fatal("MissToMonst: Invalid missile %d", mi);
	}
	mis = &missile[mi];
	mnum = mis->_miSource;
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MissToMonst: Invalid monster %d", mnum);
	}
	AssertFixMonLocation(mnum);
	mon = &monsters[mnum];
	// assert(dMonster[mon->_mx][mon->_my] == 0);
	dMonster[mon->_mx][mon->_my] = mnum + 1;
	// assert(dPlayer[mon->_mx][mon->_my] == 0);
	// assert(!(mon->_mFlags & MFLAG_HIDDEN));
	ChangeLightOff(mon->mlid, 0, 0);
	assert(mon->_mdir == mis->_miDir);
	MonStartStand(mnum);
	/*if (mon->_mType >= MT_INCIN && mon->_mType <= MT_HELLBURN) {
		MonStartFadein(mnum, mon->_mdir, false);
		return;
	}*/
	PlayEffect(mnum, MS_GOTHIT);
	if (mon->_mType == MT_GLOOM)
		return;

	oldx = mis->_mix;
	oldy = mis->_miy;
	tnum = dPlayer[oldx][oldy];
	// TODO: use CheckPlrCol instead?
	if (tnum > 0) {
		tnum--;
		MonHitPlr(mnum, tnum, 500, mon->_mMinDamage2, mon->_mMaxDamage2);
		if (tnum == dPlayer[oldx][oldy] - 1 && (mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE)) {
			KnockbackPlr(tnum, mon->_mdir);
		}
		return;
	}
	tnum = dMonster[oldx][oldy];
	// TODO: use CheckMonCol instead?
	if (tnum > 0) {
		tnum--;
		if (tnum >= MAX_MINIONS)
			return;
		MonHitMon(mnum, tnum, 500, mon->_mMinDamage2, mon->_mMaxDamage2);
		if (tnum == dMonster[oldx][oldy] - 1 && (mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE)) {
			MonGetKnockback(tnum, mis->_misx, mis->_misy);
		}
	}
}

/*static bool monster_posok(int mnum, int x, int y)
{
	MissileStruct* mis;
	bool ret = true, fire = false;
	int mi = dMissile[x][y], i;

	if (mi == 0 || mnum < 0)
		return true;

//#ifdef HELLFIRE
//	bool lightning = false;

//	if (mi != MIS_MULTI) {
//		if (missile[mi - 1]._miType == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
//			fire = true;
//		} else if (missile[mi - 1]._miType == MIS_LIGHTWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
//			lightning = true;
//		}
//	} else {
//		for (i = 0; i < nummissiles; i++) {
//			mis = &missile[missileactive[i]];
//			if (mis->_mix == x && mis->_miy == y) {
//				if (mis->_miType == MIS_FIREWALL) {
//					fire = true;
//				} else if (mis->_miType == MIS_LIGHTWALL) {
//					lightning = true;
//				}
//			}
//		}
//	}
//	if (fire && (monsters[mnum]._mMagicRes & MORS_FIRE_IMMUNE) != MORS_FIRE_IMMUNE)
//		ret = false;
//	if (lightning && (monsters[mnum]._mMagicRes & MORS_LIGHTNING_IMMUNE) != MORS_LIGHTNING_IMMUNE)
//		ret = false;
//#else
	if (mi != MIS_MULTI) {
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
	if (fire && (monsters[mnum]._mMagicRes & MORS_FIRE_IMMUNE) != MORS_FIRE_IMMUNE)
		ret = false;
//#endif
	return ret;
}*/

bool PosOkMonster(int mnum, int x, int y)
{
	int mpo;

	if ((nSolidTable[dPiece[x][y]] | dPlayer[x][y]) != 0)
		return false;

	mpo = dMonster[x][y];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		return mpo == mnum;
	}

	mpo = dObject[x][y];
	if (mpo != 0) {
		mpo = mpo >= 0 ? mpo - 1 : -(mpo + 1);
		if (objects[mpo]._oSolidFlag)
			return false;
	}

	return true;
}

bool PosOkMonst(int mnum, int x, int y)
{
	int oi;

	if ((nSolidTable[dPiece[x][y]] | dPlayer[x][y] | dMonster[x][y]) != 0)
		return false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		if (objects[oi]._oSolidFlag)
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
		if (objects[oi]._oSolidFlag)
			return false;
	}

	return monster_posok(mnum, x, y);
}

bool PosOkMonst3(int mnum, int x, int y)
{
	BYTE door;
	int oi;

	door = ODT_NONE;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		door = objects[oi]._oDoorFlag;
		if (objects[oi]._oSolidFlag && door == ODT_NONE) {
			return false;
		}
	}

	if ((nSolidTable[dPiece[x][y]] && door == ODT_NONE) || (dPlayer[x][y] | dMonster[x][y]) != 0)
		return false;

	return monster_posok(mnum, x, y);
}

static void ActivateSpawn(int mnum, int x, int y, int dir)
{
	dMonster[x][y] = mnum + 1;
	SetMonsterLoc(&monsters[mnum], x, y);
	MonStartSpStand(mnum, dir);
	monsters[mnum]._msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
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
	int n = numSkelTypes;

	if (n == 0)
		return -1;

	n = mapSkelTypes[random_(136, n)];
	n = AddMonster(0, 0, 0, n, false);
	if (n != -1) {
		// inactive minions and prespawn skeletons have to be identifiable by DeltaLoadLevel
		assert(MINION_NR_INACTIVE(n));
	}

	return n;
}

void SyncMonsterQ(int pnum, int idx)
{
	int i;

	// assert(plr._pmode != PM_DEATH);

	if (!PlrHasStorageItem(pnum, idx, &i))
		return;

	switch (idx) {
	case IDI_BANNER:
		break;
	case IDI_GLDNELIX:
		//if (plr._pDunLevel != questlist[Q_VEIL]._qdlvl)
		//	return;
		if (quests[Q_VEIL]._qactive != QUEST_ACTIVE)
			return;
		quests[Q_VEIL]._qactive = QUEST_DONE;
		if (pnum == mypnum)
			SpawnUnique(UITEM_STEELVEIL, plr._px, plr._py, ICM_SEND_FLIP);
		break;
	default:
		return;
	}

	SyncPlrStorageRemove(pnum, i);
}

void TalktoMonster(int mnum, int pnum)
{
	MonsterStruct* mon;
	int iv;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("TalktoMonster: Invalid monster %d", mnum);
	}
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("TalktoMonster: Invalid player %d", pnum);
	}
	mon = &monsters[mnum];
	if (mon->_mgoal != MGOAL_INQUIRING)
		return; // already talking (or does not want to talk at all)
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
			if (PlrHasStorageItem(pnum, IDI_BANNER, &iv)) {
				mon->mtalkmsg = TEXT_BANNER12;
				NetSendCmdParam1(CMD_QMONSTER, IDI_BANNER);
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
		mon->mtalkmsg = TEXT_GARBUD1 + quests[Q_GARBUD]._qvar1;
		if (mon->mtalkmsg == TEXT_GARBUD1) {
			quests[Q_GARBUD]._qactive = QUEST_ACTIVE;
			quests[Q_GARBUD]._qlog = TRUE; // BUGFIX: (?) for other quests qactive and qlog go together, maybe this should actually go into the if above (fixed)
		} else if (mon->mtalkmsg == TEXT_GARBUD2) {
			SetRndSeed(mon->_mRndSeed);
			SpawnMonItem(mnum, plr._px, plr._py, pnum == mypnum);
		} //else if (mon->mtalkmsg == TEXT_GARBUD4)
		//	mon->_mVar8 = 0; // init MON_TIMER
		quests[Q_GARBUD]._qvar1++;
		if (quests[Q_GARBUD]._qvar1 > 4)
			quests[Q_GARBUD]._qvar1 = 4;
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
		} else if (quests[Q_VEIL]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_GLDNELIX, &iv)) {
			mon->mtalkmsg = TEXT_VEIL11;
			NetSendCmdParam1(CMD_QMONSTER, IDI_GLDNELIX);
		}
	} else if (mon->_mAi == AI_ZHAR) {
		if (quests[Q_ZHAR]._qactive == QUEST_INIT) {
			quests[Q_ZHAR]._qactive = QUEST_ACTIVE;
			quests[Q_ZHAR]._qvar1 = 1;
			quests[Q_ZHAR]._qlog = TRUE;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_ZHAR, true);
			iv = SPL_SWIPE;
			if (plr._pClass == PC_ROGUE)
				iv = SPL_POINT_BLANK;
			else if (plr._pClass == PC_SORCERER)
				iv = SPL_LIGHTNING;
			SetRndSeed(mon->_mRndSeed);
			SpawnSpellBook(iv, plr._px, plr._py, pnum == mypnum);
		} else if (quests[Q_ZHAR]._qvar1 == 1) {
			mon->mtalkmsg = TEXT_ZHAR2;
			//mon->_mVar8 = 0; // init MON_TIMER
			quests[Q_ZHAR]._qvar1 = 2;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_ZHAR, true);
		}
	}
}

void InitGolemStats(int mnum, int level)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	mon->_mLevel = level;
	mon->_mmaxhp = 640 * level;
	mon->_mArmorClass = 25 + level;
	mon->_mEvasion = 10 + (level >> 1);
	mon->_mHit = 4 * level + 40;
	mon->_mMinDamage = 4 + (level >> 1);
	mon->_mMaxDamage = 2 * mon->_mMinDamage;
	mon->_mExp = 0;
}

void SpawnGolem(int mnum, int x, int y, int level)
{
	MonsterStruct* mon;

	static_assert(MAX_MINIONS == MAX_PLRS, "SpawnGolem requires that owner of a monster has the same id as the monster itself.");
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("SpawnGolem: Invalid monster %d", mnum);
	}
	InitGolemStats(mnum, level * 2 + (plx(mnum)._pMagic >> 6));
	mon = &monsters[mnum];
	mon->_mhitpoints = mon->_mmaxhp;
	mon->_mvid = AddVision(x, y, PLR_MIN_VISRAD, false);
	ActivateSpawn(mnum, x, y, DIR_S);
	if (mnum == mypnum)
		NetSendCmdGolem();
}

bool CanTalkToMonst(int mnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("CanTalkToMonst: Invalid monster %d", mnum);
	}
	assert((monsters[mnum]._mgoal == MGOAL_INQUIRING
		|| monsters[mnum]._mgoal == MGOAL_TALKING) == (monsters[mnum].mtalkmsg != TEXT_NONE));
	return monsters[mnum].mtalkmsg != TEXT_NONE;
}

bool CheckMonsterHit(int mnum, bool* ret)
{
	MonsterStruct* mon;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("CheckMonsterHit: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];

	if (mon->mtalkmsg != TEXT_NONE || mon->_mmode == MM_CHARGE || mon->_mhitpoints < (1 << 6)
	 || (mon->_mAi == AI_SNEAK && mon->_mgoal == MGOAL_RETREAT)
	 || (mon->_mAi == AI_COUNSLR && mon->_mgoal != MGOAL_NORMAL)) {
		*ret = false;
		return true;
	}

	if (mon->_mAi == AI_GARG && mon->_mFlags & MFLAG_GARG_STONE) {
		mon->_mFlags &= ~(MFLAG_GARG_STONE | MFLAG_LOCK_ANIMATION);
		// mon->_mmode = MM_SPATTACK;
		*ret = true;
		return true;
	}

	return false;
}

DEVILUTION_END_NAMESPACE
