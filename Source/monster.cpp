/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */
#include "all.h"
#include "monstai.h"
#include "engine/render/cl2_render.h"

DEVILUTION_BEGIN_NAMESPACE

/* Limit the number of monsters to be placed. */
int totalmonsters;
/* Limit the number of (scattered) monster-types on the current level by the required resources (In CRYPT the values are not valid). */
static int monstimgtot;
/* Number of active monsters on the current level (minions are considered active). */
int nummonsters;
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
#define MON_LIGHTRAD 7

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
/** Standard MAI check if the monster is (not) 'disturbed'. */
#define MON_RELAXED (mon->_msquelch < SQUELCH_LOW)
/** Standard MAI check if the monster has a set enemy. */
#define MON_HAS_ENEMY (mon->_menemyx != 0)

/** Temporary container to store info related to the enemy of a monster */
static MonEnemyStruct currEnemyInfo;

/** Maps from walking path step to facing direction. */
//const int8_t walk2dir[9] = { 0, DIR_NE, DIR_NW, DIR_SE, DIR_SW, DIR_N, DIR_E, DIR_S, DIR_W };

/* data */

/** Maps from monster walk animation length to monster velocity. */
// MWVel[animLen - 1] = (TILE_WIDTH << MON_WALK_SHIFT) / animLen;
const int MWVel[24] = {
	// clang-format off
	(TILE_WIDTH << MON_WALK_SHIFT) / 1,
	(TILE_WIDTH << MON_WALK_SHIFT) / 2,
	(TILE_WIDTH << MON_WALK_SHIFT) / 3,
	(TILE_WIDTH << MON_WALK_SHIFT) / 4,
	(TILE_WIDTH << MON_WALK_SHIFT) / 5,
	(TILE_WIDTH << MON_WALK_SHIFT) / 6,
	(TILE_WIDTH << MON_WALK_SHIFT) / 7,
	(TILE_WIDTH << MON_WALK_SHIFT) / 8,
	(TILE_WIDTH << MON_WALK_SHIFT) / 9,
	(TILE_WIDTH << MON_WALK_SHIFT) / 10,
	(TILE_WIDTH << MON_WALK_SHIFT) / 11,
	(TILE_WIDTH << MON_WALK_SHIFT) / 12,
	(TILE_WIDTH << MON_WALK_SHIFT) / 13,
	(TILE_WIDTH << MON_WALK_SHIFT) / 14,
	(TILE_WIDTH << MON_WALK_SHIFT) / 15,
	(TILE_WIDTH << MON_WALK_SHIFT) / 16,
	(TILE_WIDTH << MON_WALK_SHIFT) / 17,
	(TILE_WIDTH << MON_WALK_SHIFT) / 18,
	(TILE_WIDTH << MON_WALK_SHIFT) / 19,
	(TILE_WIDTH << MON_WALK_SHIFT) / 20,
	(TILE_WIDTH << MON_WALK_SHIFT) / 21,
	(TILE_WIDTH << MON_WALK_SHIFT) / 22,
	(TILE_WIDTH << MON_WALK_SHIFT) / 23,
	(TILE_WIDTH << MON_WALK_SHIFT) / 24,
	// clang-format on
};
/** Maps from monster action to monster animation letter. */
static const char animletter[NUM_MON_ANIM] = { 'n', 'w', 'a', 'h', 'd', 's' };
/** Maps from direction to delta X-offset. */
const int offset_x[NUM_DIRS] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to delta Y-offset. */
const int offset_y[NUM_DIRS] = { 1, 1, 1, 0, -1, -1, -1, 0 };

/**
 * Maps from monster AI ID to monster AI function.
 * Notable quirks of MAI functions:
 * - lastx/y must be set when a monster is 'alerted' the first time (squelch is set from zero to non-zero)
 * - if the monster is active, the function/monster must do something otherwise the enemy might not get updated
 * - can not rely on dLight, because it might not be in sync in multiplayer games
 */
static void (*const AiProc[])(int i) = {
	// clang-format off
/*AI_ZOMBIE*/       &MAI_Zombie,
/*AI_FAT*/          &MAI_Fat,
/*AI_SKELSD*/       &MAI_SkelSd,
/*AI_SKELBOW*/      &MAI_SkelBow,
/*AI_SCAV*/         &MAI_Scav,
/*AI_RHINO*/        &MAI_Rhino,
/*AI_ROUND*/        &MAI_Round,
/*AI_RANGED*/       &MAI_Ranged,
/*AI_FALLEN*/       &MAI_Fallen,
/*AI_ROUNDRANGED*/  &MAI_RoundRanged,
/*AI_SKELKING*/     &MAI_SkelKing,
/*AI_BAT*/          &MAI_Bat,
/*AI_GARG*/         &MAI_Garg,
/*AI_CLEAVER*/      &MAI_Cleaver,
/*AI_SNEAK*/        &MAI_Sneak,
/*AI_FIREMAN*///    &MAI_Fireman,
/*AI_GARBUD*/       &MAI_Garbud,
/*AI_GOLUM*/        &MAI_Golem,
/*AI_ZHAR*/         &MAI_Zhar,
/*AI_SNOTSPIL*/     &MAI_SnotSpil,
/*AI_SNAKE*/        &MAI_Snake,
/*AI_COUNSLR*/      &MAI_Counselor,
/*AI_MAGE*/         &MAI_Mage,
/*AI_ROUNDRANGED2*/ &MAI_RoundRanged2,
/*AI_LAZARUS*/      &MAI_Lazarus,
/*AI_LAZHELP*/      &MAI_Lazhelp,
/*AI_LACHDAN*/      &MAI_Lachdanan,
/*AI_WARLORD*/      &MAI_Warlord,
#ifdef HELLFIRE
/*AI_HORKDMN*/      &MAI_Horkdemon,
#endif
	// clang-format on
};

static inline void InitMonsterTRN(MonAnimStruct (&anims)[NUM_MON_ANIM], const char* transFile)
{
	int i, j;
	const MonAnimStruct* as;
	BYTE trn[NUM_COLORS];

	// A TRN file contains a sequence of color transitions, represented
	// as indexes into a palette. (a 256 byte array of palette indices)
	LoadFileWithMem(transFile, trn);
#if !USE_PATCH
	// patch TRN files - Monsters/*.TRN
	BYTE *cf = trn;
	for (i = 0; i < NUM_COLORS; i++) {
		if (*cf == 255) {
			*cf = 0;
		}
		cf++;
	}
#endif
	for (i = 0; i < NUM_MON_ANIM; i++) {
		as = &anims[i];
		if (as->maFrames > 1) {
			for (j = 0; j < lengthof(as->maAnimData); j++) {
				Cl2ApplyTrans(as->maAnimData[j], trn, as->maFrames);
			}
		}
	}
}

static void InitMonsterGFX(int midx)
{
	MapMonData* cmon;
	const MonFileData* mfdata;
	int mtype, anim, i;
	char strBuff[DATA_ARCHIVE_MAX_PATH];
	BYTE* celBuf;

	cmon = &mapMonTypes[midx];
	mfdata = &monfiledata[cmon->cmFileNum];
	cmon->cmWidth = mfdata->moWidth * ASSET_MPL;
	cmon->cmXOffset = (cmon->cmWidth - TILE_WIDTH) >> 1;
	cmon->cmAFNum = mfdata->moAFNum;
	cmon->cmAFNum2 = mfdata->moAFNum2;

	mtype = cmon->cmType;
	auto& monAnims = cmon->cmAnims;
	// static_assert(lengthof(animletter) == lengthof(monsterdata[0].maFrames), "");
	for (anim = 0; anim < NUM_MON_ANIM; anim++) {
		monAnims[anim].maFrames = mfdata->moAnimFrames[anim];
		monAnims[anim].maFrameLen = mfdata->moAnimFrameLen[anim];
		if (mfdata->moAnimFrames[anim] > 0) {
			snprintf(strBuff, sizeof(strBuff), mfdata->moGfxFile, animletter[anim]);

			celBuf = LoadFileInMem(strBuff);
			assert(cmon->cmAnimData[anim] == NULL);
			cmon->cmAnimData[anim] = celBuf;

			if (mtype != MT_GOLEM || (anim != MA_SPECIAL && anim != MA_DEATH)) {
				for (i = 0; i < lengthof(monAnims[anim].maAnimData); i++) {
					monAnims[anim].maAnimData[i] = const_cast<BYTE*>(CelGetFrameStart(celBuf, i));
				}
			} else {
				for (i = 0; i < lengthof(monAnims[anim].maAnimData); i++) {
					monAnims[anim].maAnimData[i] = celBuf;
				}
			}
		}
	}

	if (monsterdata[mtype].mTransFile != NULL) {
		InitMonsterTRN(monAnims, monsterdata[mtype].mTransFile);
	}

	// copy walk animation to the stand animation of the golem (except aCelData and alignment)
	if (mtype == MT_GOLEM) {
		copy_pod(monAnims[MA_STAND].maAnimData, monAnims[MA_WALK].maAnimData);
		monAnims[MA_STAND].maFrames = monAnims[MA_WALK].maFrames;
		monAnims[MA_STAND].maFrameLen = monAnims[MA_WALK].maFrameLen;
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
	case MT_RTHIN:
	case MT_NTHIN:
	case MT_XTHIN:
	case MT_GTHIN:
		LoadMissileGFX(MFILE_THINLGHT);
		break;
	/*case MT_NSUCC:
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
	case MT_GSUCC:
		LoadMissileGFX(MFILE_SCUBMISB);
		LoadMissileGFX(MFILE_SCBSEXPB);
		break;
	case MT_RSUCC:
		LoadMissileGFX(MFILE_SCUBMISD);
		LoadMissileGFX(MFILE_SCBSEXPD);
		break;
	case MT_BSUCC:
		LoadMissileGFX(MFILE_SCUBMISC);
		LoadMissileGFX(MFILE_SCBSEXPC);
		break;
	case MT_SMAGE:
	case MT_OMAGE:
		LoadMissileGFX(MFILE_MAGEMIS);
		LoadMissileGFX(MFILE_MAGEEXP);
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

static void InitMonsterStats(int midx)
{
	MapMonData* cmon;
	const MonsterData* mdata;
	unsigned baseLvl, lvlBonus, monLvl;

	cmon = &mapMonTypes[midx];

	mdata = &monsterdata[cmon->cmType];

	cmon->cmName = mdata->mName;
	cmon->cmFileNum = mdata->moFileNum;
	cmon->cmLevel = mdata->mLevel;
	cmon->cmSelFlag = mdata->mSelFlag;
	cmon->cmAI = mdata->mAI;
	cmon->cmFlags = mdata->mFlags;
	cmon->cmHit = mdata->mHit;
	cmon->cmMinDamage = mdata->mMinDamage;
	cmon->cmMaxDamage = mdata->mMaxDamage;
	cmon->cmHit2 = mdata->mHit2;
	cmon->cmMinDamage2 = mdata->mMinDamage2;
	cmon->cmMaxDamage2 = mdata->mMaxDamage2;
	cmon->cmMagic = mdata->mMagic;
	cmon->cmArmorClass = mdata->mArmorClass;
	cmon->cmEvasion = mdata->mEvasion;
	cmon->cmMagicRes = mdata->mMagicRes;
	cmon->cmExp = mdata->mExp;
	cmon->cmMinHP = mdata->mMinHP;
	cmon->cmMaxHP = mdata->mMaxHP;

	lvlBonus = currLvl._dLevelBonus;
	cmon->cmAI.aiInt += lvlBonus / 16;

	cmon->cmHit += lvlBonus * 5 / 2;
	cmon->cmHit2 += lvlBonus * 5 / 2;
	cmon->cmMagic += lvlBonus * 5 / 2;
	cmon->cmEvasion += lvlBonus * 5 / 2;
	cmon->cmArmorClass += lvlBonus * 5 / 2;

	baseLvl = cmon->cmLevel;
	monLvl = baseLvl + lvlBonus;
	cmon->cmLevel = monLvl;
	cmon->cmMinHP = monLvl * cmon->cmMinHP / baseLvl;
	cmon->cmMaxHP = monLvl * cmon->cmMaxHP / baseLvl;
	cmon->cmExp = monLvl * cmon->cmExp / baseLvl;
	cmon->cmMinDamage = monLvl * cmon->cmMinDamage / baseLvl;
	cmon->cmMaxDamage = monLvl * cmon->cmMaxDamage / baseLvl;
	cmon->cmMinDamage2 = monLvl * cmon->cmMinDamage2 / baseLvl;
	cmon->cmMaxDamage2 = monLvl * cmon->cmMaxDamage2 / baseLvl;

	if (gnDifficulty == DIFF_NIGHTMARE) {
		/*cmon->cmMinHP = 3 * cmon->cmMinHP / 2;
		cmon->cmMaxHP = 3 * cmon->cmMaxHP / 2;
		cmon->cmExp = 3 * cmon->cmExp / 2;
		cmon->cmMinDamage = 3 * cmon->cmMinDamage / 2;
		cmon->cmMaxDamage = 3 * cmon->cmMaxDamage / 2;
		cmon->cmMinDamage2 = 3 * cmon->cmMinDamage2 / 2;
		cmon->cmMaxDamage2 = 3 * cmon->cmMaxDamage2 / 2;
		cmon->cmHit += NIGHTMARE_TO_HIT_BONUS;
		cmon->cmMagic += NIGHTMARE_MAGIC_BONUS;
		cmon->cmHit2 += NIGHTMARE_TO_HIT_BONUS;
		cmon->cmArmorClass += NIGHTMARE_AC_BONUS;
		cmon->cmEvasion += NIGHTMARE_EVASION_BONUS;*/
	} else if (gnDifficulty == DIFF_HELL) {
		/*cmon->cmMinHP *= 2;
		cmon->cmMaxHP *= 2;
		cmon->cmExp *= 2;
		cmon->cmMinDamage2 *= 2;
		cmon->cmMaxDamage2 *= 2;
		cmon->cmHit += HELL_TO_HIT_BONUS;
		cmon->cmMagic += HELL_MAGIC_BONUS;
		cmon->cmHit2 += HELL_TO_HIT_BONUS;
		cmon->cmArmorClass += HELL_AC_BONUS;
		cmon->cmEvasion += HELL_EVASION_BONUS;*/
		cmon->cmMagicRes = monsterdata[cmon->cmType].mMagicRes2;
	}

	int mpl = currLvl._dLevelPlyrs;
	// assert(mpl != 0);
	mpl++;
	cmon->cmMinHP = (cmon->cmMinHP * mpl) >> 1;
	cmon->cmMaxHP = (cmon->cmMaxHP * mpl) >> 1;
	cmon->cmExp = (cmon->cmExp * mpl) >> 1;
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
		assert(nummtypes <= MAX_LVLMTYPES);
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
		InitMonsterStats(i); // init stats first because InitMonsterGFX depends on it (cmFileNum)
		InitMonsterGFX(i);
		InitMonsterSFX(i);
	}

	if (scatter && !mapMonTypes[i].cmPlaceScatter) {
		mapMonTypes[i].cmPlaceScatter = TRUE;
		monstimgtot -= monfiledata[monsterdata[type].moFileNum].moImage;
	}

	return i;
}

void InitLvlMonsters()
{
	int i;

	nummtypes = 0;
	numSkelTypes = 0;
	numGoatTypes = 0;
	uniquetrans = COLOR_TRN_UNIQ;
	monstimgtot = MAX_LVLMIMAGE - monfiledata[monsterdata[MT_GOLEM].moFileNum].moImage;
	totalmonsters = MAXMONSTERS;

	// reset monsters
	for (i = 0; i < MAXMONSTERS; i++) {
		monsters[i]._mmode = MM_UNUSED;
		// reset squelch value to simplify MonFallenFear, sync_all_monsters and LevelDeltaExport
		monsters[i]._msquelch = 0;
		// reset _mMTidx value to simplify SyncMonsterAnim (loadsave.cpp)
		monsters[i]._mMTidx = 0;
		monsters[i]._mpathcount = 0;
		monsters[i]._mAlign_1 = 0;
		monsters[i]._mgoal = MGOAL_NORMAL;
		// reset _muniqtype value to simplify SyncMonsterAnim (loadsave.cpp)
		// reset _muniqtrans to simplify InitTownerInfo (towner.cpp)
		// reset _mNameColor to simplify InitTownerInfo (towner.cpp)
		// reset _mlid value to simplify SyncMonstersLight (loadsave.cpp), DeltaLoadLevel, SummonMonster and InitTownerInfo (towner.cpp)
		monsters[i]._muniqtype = 0;
		monsters[i]._muniqtrans = 0;
		monsters[i]._mNameColor = COL_WHITE;
		monsters[i]._mlid = NO_LIGHT;
		// reset _mleaderflag value to simplify GroupUnity
		// monsters[i]._mleader = MON_NO_LEADER;
		// monsters[i]._mleaderflag = MLEADER_NONE;
		// monsters[i]._mpacksize = 0;
		// monsters[i]._mvid = NO_VISION;
	}
	// reserve minions
	nummonsters = MAX_MINIONS;
	if (currLvl._dLevelIdx != DLV_TOWN) {
		AddMonsterType(MT_GOLEM, FALSE);
		mapMonTypes[0].cmFlags |= MFLAG_NOCORPSE | MFLAG_NODROP;
		for (i = 0; i < MAX_MINIONS; i++) {
			InitMonster(i, DIR_S, 0, 0, 0);
			monsters[i]._mmode = MM_RESERVED;
		}
	}
}

void GetLevelMTypes()
{
	int i, mtype;
	int montypes[lengthof(AllLevels[0].dMonTypes)];
	const LevelData* lds;
	BYTE lvl;

	int nt; // number of types

	lvl = currLvl._dLevelIdx;
	//if (!currLvl._dSetLvl) {
#ifdef HELLFIRE
		if (lvl == uniqMonData[UMT_HORKDMN].muLevelIdx - 1)
			AddMonsterType(MT_HORKSPWN, TRUE);
		if (lvl == uniqMonData[UMT_HORKDMN].muLevelIdx) {
			AddMonsterType(MT_HORKSPWN, TRUE);
			AddMonsterType(uniqMonData[UMT_HORKDMN].mtype, FALSE);
		}
		if (lvl == uniqMonData[UMT_DEFILER].muLevelIdx)
			AddMonsterType(uniqMonData[UMT_DEFILER].mtype, FALSE);
		if (lvl == DLV_CRYPT4) {
			// AddMonsterType(uniqMonData[UMT_NAKRUL].mtype, FALSE);
		}
#endif
		//if (QuestStatus(Q_BUTCHER))
		//	AddMonsterType(uniqMonData[UMT_BUTCHER].mtype, FALSE);
		if (lvl == uniqMonData[UMT_GARBUD].muLevelIdx && quests[Q_GARBUD]._qactive != QUEST_NOTAVAIL)
			AddMonsterType(uniqMonData[UMT_GARBUD].mtype, FALSE);
		if (lvl == uniqMonData[UMT_ZHAR].muLevelIdx && quests[Q_ZHAR]._qactive != QUEST_NOTAVAIL)
			AddMonsterType(uniqMonData[UMT_ZHAR].mtype, FALSE);
		if (lvl == uniqMonData[UMT_ZAMPHIR].muLevelIdx && quests[Q_MUSHROOM]._qactive != QUEST_NOTAVAIL)
			AddMonsterType(uniqMonData[UMT_ZAMPHIR].mtype, FALSE);
		//if (QuestStatus(Q_BANNER)) {
		//	AddMonsterType(uniqMonData[UMT_SNOTSPIL].mtype, FALSE);
		//	// AddMonsterType(MT_NFAT, FALSE);
		//}
		//if (QuestStatus(Q_ANVIL)) {
		//	AddMonsterType(MT_GGOATBW, FALSE);
		//	AddMonsterType(MT_DRHINO, FALSE);
		//}
		//if (QuestStatus(Q_BLIND)) {
		//	AddMonsterType(MT_YSNEAK, FALSE);
		//}
		//if (QuestStatus(Q_BLOOD)) {
		//	AddMonsterType(MT_NRHINO, FALSE);
		//}
		if (lvl == uniqMonData[UMT_LACHDAN].muLevelIdx && quests[Q_VEIL]._qactive != QUEST_NOTAVAIL)
			AddMonsterType(uniqMonData[UMT_LACHDAN].mtype, TRUE);
		if (lvl == questlist[Q_WARLORD]._qdlvl && quests[Q_WARLORD]._qactive != QUEST_NOTAVAIL)
			AddMonsterType(uniqMonData[UMT_WARLORD].mtype, TRUE);
		//if (QuestStatus(Q_BETRAYER) && IsMultiGame) {
		//if (currLvl._dLevelIdx == questlist[Q_BETRAYER]._qdlvl && IsMultiGame) {
		//	AddMonsterType(uniqMonData[UMT_LAZARUS].mtype, FALSE);
		//	AddMonsterType(uniqMonData[UMT_RED_VEX].mtype, FALSE);
		//  assert(uniqMonData[UMT_RED_VEX].mtype == uniqMonData[UMT_BLACKJADE].mtype);
		//}
		lvl = currLvl._dLevelNum;
		if (lvl == DLV_HELL4) {
			AddMonsterType(MT_BMAGE, TRUE);
			AddMonsterType(MT_GBLACK, TRUE);
			// AddMonsterType(MT_NBLACK, FALSE);
			// AddMonsterType(uniqMonData[UMT_DIABLO].mtype, FALSE);
			// return;
		}
#ifdef HELLFIRE
		if (lvl == DLV_CRYPT4) {
			AddMonsterType(MT_ARCHLICH, TRUE);
		}
#endif
		lds = &AllLevels[lvl];
		for (nt = 0; nt < lengthof(lds->dMonTypes); nt++) {
			mtype = lds->dMonTypes[nt];
			if (mtype == MT_INVALID)
				break;
			montypes[nt] = mtype;
		}

		while (monstimgtot > 0/* && nummtypes < MAX_LVLMTYPES*/) { // nummtypes test is pointless, because PlaceSetMapMonsters can break it anyway...
			for (i = 0; i < nt; ) {
				if (monfiledata[monsterdata[montypes[i]].moFileNum].moImage > monstimgtot) {
					montypes[i] = montypes[--nt];
					continue;
				}

				i++;
			}

			if (nt == 0)
				break;

			i = random_low(88, nt);
			AddMonsterType(montypes[i], TRUE);
			montypes[i] = montypes[--nt];
		}
	//} else {
	//	if (lvl == SL_SKELKING) {
	//		AddMonsterType(uniqMonData[UMT_SKELKING].mtype, FALSE);
	//	} else if (lvl == SL_VILEBETRAYER) {
	//		AddMonsterType(uniqMonData[UMT_LAZARUS].mtype, FALSE);
	//		AddMonsterType(uniqMonData[UMT_RED_VEX].mtype, FALSE);
	//		assert(uniqMonData[UMT_RED_VEX].mtype == uniqMonData[UMT_BLACKJADE].mtype);
	//	}
	//}
}

void InitMonster(int mnum, int dir, int mtidx, int x, int y)
{
	MapMonData* cmon = &mapMonTypes[mtidx];
	MonsterStruct* mon = &monsters[mnum];

	mon->_mMTidx = mtidx;
	mon->_mdir = dir;
	SetMonsterLoc(mon, x, y);
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mType = cmon->cmType;
	/*mon->_mName = cmon->cmName;
	mon->_mFileNum = cmon->cmFileNum;
	mon->_mLevel = cmon->cmLevel;
	mon->_mSelFlag = cmon->cmSelFlag;
	mon->_mAI = cmon->cmAI; // aiType, aiInt, aiParam1, aiParam2
	mon->_mFlags = cmon->cmFlags;
	mon->_mHit = cmon->cmHit;
	mon->_mMinDamage = cmon->cmMinDamage;
	mon->_mMaxDamage = cmon->cmMaxDamage;
	mon->_mHit2 = cmon->cmHit2;
	mon->_mMinDamage2 = cmon->cmMinDamage2;
	mon->_mMaxDamage2 = cmon->cmMaxDamage2;
	mon->_mMagic = cmon->cmMagic;
	mon->_mArmorClass = cmon->cmArmorClass;
	mon->_mEvasion = cmon->cmEvasion;
	mon->_mMagicRes = cmon->cmMagicRes;
	mon->_mAlign_1 = cmon->cmAlign_1;
	mon->_mExp = cmon->cmExp;
	mon->_mAnimWidth = cmon->cmWidth;
	mon->_mAnimXOffset = cmon->cmXOffset;
	mon->_mAFNum = cmon->cmAFNum;
	mon->_mAFNum2 = cmon->cmAFNum2;
	mon->_mAlign_0 = cmon->cmAlign_0;*/
	static_assert(offsetof(MapMonData, cmAlign_0) > offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance I.");
	static_assert(offsetof(MonsterStruct, _mAlign_0) > offsetof(MonsterStruct, _mName), "InitMonster uses DWORD-memcpy to optimize performance II.");
	static_assert(((offsetof(MapMonData, cmAlign_0) - offsetof(MapMonData, cmName) + sizeof(cmon->cmAlign_0)) % 4) == 0, "InitMonster uses DWORD-memcpy to optimize performance III.");
	static_assert(offsetof(MonsterStruct, _mFileNum) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmFileNum) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance IV.");
	static_assert(offsetof(MonsterStruct, _mLevel) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmLevel) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance Va.");
	static_assert(offsetof(MonsterStruct, _mSelFlag) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmSelFlag) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance Vb.");
	static_assert(offsetof(MonsterStruct, _mAI) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmAI) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance VI.");
	static_assert(offsetof(MonsterStruct, _mFlags) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmFlags) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance VII.");
	static_assert(offsetof(MonsterStruct, _mHit) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmHit) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance VIII.");
	static_assert(offsetof(MonsterStruct, _mMinDamage) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmMinDamage) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance IX.");
	static_assert(offsetof(MonsterStruct, _mMaxDamage) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmMaxDamage) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance X.");
	static_assert(offsetof(MonsterStruct, _mHit2) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmHit2) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XI.");
	static_assert(offsetof(MonsterStruct, _mMinDamage2) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmMinDamage2) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XII.");
	static_assert(offsetof(MonsterStruct, _mMaxDamage2) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmMaxDamage2) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XIII.");
	static_assert(offsetof(MonsterStruct, _mMagic) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmMagic) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XIV.");
	static_assert(offsetof(MonsterStruct, _mArmorClass) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmArmorClass) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XVI.");
	static_assert(offsetof(MonsterStruct, _mEvasion) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmEvasion) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XVII.");
	static_assert(offsetof(MonsterStruct, _mMagicRes) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmMagicRes) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XVIII.");
	static_assert(offsetof(MonsterStruct, _mExp) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmExp) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XX.");
	static_assert(offsetof(MonsterStruct, _mAnimWidth) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmWidth) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XXII.");
	static_assert(offsetof(MonsterStruct, _mAnimXOffset) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmXOffset) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XXIII.");
	static_assert(offsetof(MonsterStruct, _mAFNum) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmAFNum) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XXIV.");
	static_assert(offsetof(MonsterStruct, _mAFNum2) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmAFNum2) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XXV.");
	static_assert(offsetof(MonsterStruct, _mAlign_0) - offsetof(MonsterStruct, _mName) == offsetof(MapMonData, cmAlign_0) - offsetof(MapMonData, cmName), "InitMonster uses DWORD-memcpy to optimize performance XXVI.");
	memcpy(&mon->_mName, &cmon->cmName, offsetof(MapMonData, cmAlign_0) - offsetof(MapMonData, cmName) + sizeof(cmon->cmAlign_0));
	mon->_mhitpoints = mon->_mmaxhp = RandRange(cmon->cmMinHP, cmon->cmMaxHP) << 6;
	mon->_mAnims = cmon->cmAnims;
	mon->_mAnimData = cmon->cmAnims[MA_STAND].maAnimData[dir];
	mon->_mAnimFrameLen = cmon->cmAnims[MA_STAND].maFrameLen;
	mon->_mAnimCnt = random_low(88, mon->_mAnimFrameLen);
	mon->_mAnimLen = cmon->cmAnims[MA_STAND].maFrames;
	mon->_mAnimFrame = mon->_mAnimLen == 0 ? 1 : RandRangeLow(1, mon->_mAnimLen);
	mon->_mmode = MM_STAND;
	mon->_mVar1 = MM_STAND;           // STAND_PREV_MODE
	mon->_mVar2 = MON_WALK_DELAY + 1; // STAND_TICK
	//mon->_mVar3 = 0;	-- should be set before use
	//mon->_mVar4 = 0;
	//mon->_mVar5 = 0;
	//mon->_mVar6 = 0;
	//mon->_mVar7 = 0;
	//mon->_mVar8 = 0;
	mon->_msquelch = 0;
	mon->_mpathcount = 0;
	mon->_mAlign_1 = 0;
	mon->_mgoal = MGOAL_NORMAL;
	//mon->_mgoalvar1 = 0;	-- should be set before use
	//mon->_mgoalvar2 = 0;
	//mon->_mgoalvar3 = 0;
	mon->_menemy = 0;
	mon->_menemyx = 0;
	mon->_menemyy = 0;
	mon->_mListener = 0;
	mon->_mDelFlag = FALSE;
	mon->_mlastx = 0; // should be set before use (except for stone-rune)
	mon->_mlasty = 0;
	mon->_mRndSeed = NextRndSeed();
	// mon->_mAISeed = -- should be set before use

	mon->_muniqtype = 0;
	mon->_muniqtrans = 0;
	mon->_mNameColor = COL_WHITE;
	mon->_mlid = NO_LIGHT;

	mon->_mleader = MON_NO_LEADER;
	mon->_mleaderflag = MLEADER_NONE;
	mon->_mpacksize = 0;
	mon->_mvid = NO_VISION;
}

/**
 * Check the location if a monster can be placed there in the init phase.
 * Must not consider the player's position, since it is already initialized
 * and messes up the pseudo-random generated dungeon.
 */
static bool MonstPlace(int xp, int yp)
{
	return (dMonster[xp][yp]/* | dPlayer[xp][yp] | dObject[xp][yp]*/
		 | nSolidTable[dPiece[xp][yp]] | (dFlags[xp][yp] & (BFLAG_ALERT | BFLAG_MON_PROTECT))) == 0;
}

#ifdef HELLFIRE
void WakeNakrul()
{
	MonsterStruct* mon;

	if (!deltaload)
		PlayMonSfx(MAX_MINIONS, MS_DEATH);
	mon = &monsters[MAX_MINIONS];
	// assert(mon->_mType == MT_NAKRUL);
	constexpr int targetRes = MORS_SLASH_PROTECTED | MORS_BLUNT_PROTECTED | MORS_PUNCTURE_PROTECTED | MORS_MAGIC_RESIST | MORS_FIRE_RESIST | MORS_LIGHTNING_RESIST | MORS_ACID_RESIST;
	if (mon->_mMagicRes == targetRes) {
		return;
	}
	mon->_mMagicRes = targetRes;
	mon->_mArmorClass -= 30;
	//mon->_mEvasion -= 10;
	mon->_mLevel -= 16;
	mon->_mmaxhp -= 400;
	if (mon->_mhitpoints > mon->_mmaxhp)
		mon->_mhitpoints = mon->_mmaxhp;
}
#endif

void InitSummonedMonster(int mnum, int dir, int mtidx, int x, int y)
{
	static_assert(DLV_TOWN == 0, "InitSummonedMonster skips the first entry glSeedTbl assuming the 'dynamic' seed is stored there.");
	SetRndSeed(glSeedTbl[(mnum % (NUM_FIXLVLS - 1)) + 1]);
	InitMonster(mnum, dir, mtidx, x, y);
	monsters[mnum]._mFlags |= MFLAG_NOCORPSE | MFLAG_NODROP;
}

int SummonMonster(int x, int y, int dir, int mtidx)
{
	unsigned mnum;

	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		if (monsters[mnum]._mmode != MM_UNUSED || monsters[mnum]._mlid != NO_LIGHT)
			continue;

		nummonsters++;
		dMonster[x][y] = mnum + 1;
		InitSummonedMonster(mnum, dir, mtidx, x, y);
		NetSendCmdMonstSummon(mnum);
		return mnum;
	}
	return -1;
}

static int PlaceMonster(int mtidx, int x, int y)
{
	int mnum, dir;

	mnum = nummonsters;
	nummonsters++;
	dMonster[x][y] = mnum + 1;

	dir = random_(90, NUM_DIRS);
	InitMonster(mnum, dir, mtidx, x, y);
	return mnum;
}

void AddMonster(int mtidx, int x, int y)
{
	if (nummonsters < MAXMONSTERS) {
		PlaceMonster(mtidx, x, y);
	}
}

static void PlaceGroup(int mtidx, int num, int leaderf, int leader)
{
	int placed, offset, try1, try2;
	int xp, yp, x1, y1, x2, y2, mnum;

	if (num + nummonsters > totalmonsters) {
		num = totalmonsters - nummonsters;
	}

	placed = 0;
	for (try1 = 0; try1 < 10; try1++) {
		while (placed != 0) {
			nummonsters--;
			placed--;
			// monsters[nummonsters]._mmode = MM_UNUSED; -- unnecessary assuming these are going to be overwritten...
			dMonster[monsters[nummonsters]._mx][monsters[nummonsters]._my] = 0;
		}

		if (leaderf) {
			// assert(leaderf & UMF_GROUP);
			x1 = monsters[leader]._mx;
			y1 = monsters[leader]._my;
		} else {
			do {
				x1 = random_(93, DSIZEX) + DBORDERX;
				y1 = random_(93, DSIZEY) + DBORDERY;
			} while (!MonstPlace(x1, y1));
		}

		static_assert(DBORDERX >= 1, "PlaceGroup expects a large enough border I.");
		static_assert(DBORDERY >= 1, "PlaceGroup expects a large enough border II.");
		xp = x1; yp = y1;
		for (try2 = 0; placed < num && try2 < 128; try2++) {
			offset = random_(94, NUM_DIRS);
			x2 = xp + offset_x[offset];
			y2 = yp + offset_y[offset];
			assert((unsigned)x2 < MAXDUNX);
			assert((unsigned)y2 < MAXDUNY);
			if ((leaderf & UMF_LEADER) && ((abs(x2 - x1) > MON_PACK_DISTANCE) || (abs(y2 - y1) > MON_PACK_DISTANCE))) {
				continue;
			}
			if (!MonstPlace(x2, y2)) {
				if (dMonster[x2][y2] != 0) {
					xp = x2;
					yp = y2;
				}
				continue;
			}
			xp = x2;
			yp = y2;
			if (random_(0, 2) != 0)
				continue;
			// assert(nummonsters < MAXMONSTERS);
			mnum = PlaceMonster(mtidx, xp, yp);
			if (leaderf) {
				// assert(leaderf & UMF_GROUP);
				monsters[mnum]._mNameColor = COL_BLUE;
				monsters[mnum]._mmaxhp *= 2;
				monsters[mnum]._mhitpoints = monsters[mnum]._mmaxhp;
				monsters[mnum]._mAI.aiInt = monsters[leader]._mAI.aiInt;

				if (leaderf & UMF_LEADER) {
					monsters[mnum]._mleader = leader;
					monsters[mnum]._mleaderflag = MLEADER_PRESENT;
					monsters[mnum]._mAI = monsters[leader]._mAI;
				}
			}
			placed++;
		}

		if (placed >= num) {
			break;
		}
	}

	if (leaderf & UMF_LEADER) {
		monsters[leader]._mleaderflag = MLEADER_SELF;
		monsters[leader]._mpacksize = placed;
	}
}

static unsigned InitUniqueMonster(int mnum, int uniqindex)
{
	char filestr[DATA_ARCHIVE_MAX_PATH];
	const UniqMonData* uniqm;
	MonsterStruct* mon;
	unsigned baseLvl, lvlBonus, monLvl;

	mon = &monsters[mnum];
	mon->_mNameColor = COL_GOLD;
	mon->_muniqtype = uniqindex + 1;

	uniqm = &uniqMonData[uniqindex];
	mon->_mLevel = uniqm->muLevel;

	mon->_mExp *= 2;
	mon->_mName = uniqm->mName;
	mon->_mmaxhp = uniqm->mmaxhp;

	mon->_mAI = uniqm->mAI;
	mon->_mMinDamage = uniqm->mMinDamage;
	mon->_mMaxDamage = uniqm->mMaxDamage;
	mon->_mMinDamage2 = uniqm->mMinDamage2;
	mon->_mMaxDamage2 = uniqm->mMaxDamage2;
	mon->_mMagicRes = uniqm->mMagicRes;
	if (uniqm->mtalkmsg != TEXT_NONE) {
		mon->_mgoal = MGOAL_TALKING;
		mon->_mgoalvar1 = FALSE;           // TALK_INQUIRING
		mon->_mgoalvar2 = uniqm->mtalkmsg; // TALK_MESSAGE
	}

	if (uniqm->mTrnName != NULL) {
		snprintf(filestr, sizeof(filestr), "Monsters\\Monsters\\%s.TRN", uniqm->mTrnName);
		LoadFileWithMem(filestr, ColorTrns[uniquetrans]);
		static_assert(NUM_COLOR_TRNS <= UCHAR_MAX, "Color transform index stored in BYTE field.");
		mon->_muniqtrans = uniquetrans++;
	}

	mon->_mHit += uniqm->mUnqHit;
	mon->_mHit2 += uniqm->mUnqHit2;
	mon->_mMagic += uniqm->mUnqMag;
	mon->_mEvasion += uniqm->mUnqEva;
	mon->_mArmorClass += uniqm->mUnqAC;

	lvlBonus = currLvl._dLevelBonus;
	mon->_mAI.aiInt += lvlBonus / 16;

	/*mon->_mHit += lvlBonus * 5 / 2;
	mon->_mHit2 += lvlBonus * 5 / 2;
	mon->_mMagic += lvlBonus * 5 / 2;
	mon->_mEvasion += lvlBonus * 5 / 2;
	mon->_mArmorClass += lvlBonus * 5 / 2;*/

	baseLvl = mon->_mLevel;
	monLvl = baseLvl + lvlBonus;
	mon->_mLevel = monLvl;
	mon->_mmaxhp = monLvl * mon->_mmaxhp / baseLvl;
	// mon->_mExp = monLvl * mon->_mExp / baseLvl;
	mon->_mMinDamage = monLvl * mon->_mMinDamage / baseLvl;
	mon->_mMaxDamage = monLvl * mon->_mMaxDamage / baseLvl;
	mon->_mMinDamage2 = monLvl * mon->_mMinDamage2 / baseLvl;
	mon->_mMaxDamage2 = monLvl * mon->_mMaxDamage2 / baseLvl;

	if (gnDifficulty == DIFF_NIGHTMARE) {
		/*mon->_mmaxhp = 3 * mon->_mmaxhp / 2;
		mon->_mMinDamage = 3 * mon->_mMinDamage / 2;
		mon->_mMaxDamage = 3 * mon->_mMaxDamage / 2;
		mon->_mMinDamage2 = 3 * mon->_mMinDamage2 / 2;
		mon->_mMaxDamage2 = 3 * mon->_mMaxDamage2 / 2;*/
	} else if (gnDifficulty == DIFF_HELL) {
		/*mon->_mmaxhp *= 2;
		mon->_mMinDamage *= 2;
		mon->_mMaxDamage *= 2;
		mon->_mMinDamage2 *= 2;
		mon->_mMaxDamage2 *= 2;*/
		mon->_mMagicRes = uniqm->mMagicRes2;
	}

	int mpl = currLvl._dLevelPlyrs;
	// assert(mpl != 0);
	mpl++;
	/*mon->_mmaxhp = (mon->_mmaxhp * mpl) >> 1;
	// mon->_mExp = (mon->_mExp * mpl) >> 1;

	mon->_mmaxhp <<= 6;*/
	mon->_mmaxhp = (mon->_mmaxhp * mpl) << (6 - 1);

	mon->_mhitpoints = mon->_mmaxhp;

	unsigned flags = uniqm->mUnqFlags;
	if (flags & UMF_NODROP)
		mon->_mFlags |= MFLAG_NODROP;
	static_assert(MAX_LIGHT_RAD >= MON_LIGHTRAD, "Light-radius of unique monsters are too high.");
	if (flags & UMF_LIGHT) {
		mon->_mlid = AddLight(mon->_mx, mon->_my, MON_LIGHTRAD);
	}
	return flags;
}

#define MON_PACK_AREA (1 + 2 * MON_PACK_DISTANCE)
#define MON_PACK_SPACE 8
static_assert(MON_PACK_AREA <= MON_PACK_SPACE, "Not enough space to search for monster placement.");
static int MonPackSpace(int dx, int dy, int px, int py, bool (&visited)[MON_PACK_SPACE][MON_PACK_SPACE])
{
	int result = 0;

	if (!visited[px][py]) {
		visited[px][py] = true;
		if (MonstPlace(dx, dy)) {
			result = 1;
			if (px != 0)
				result += MonPackSpace(dx - 1, dy, px - 1, py, visited);
			if (px != MON_PACK_AREA - 1)
				result += MonPackSpace(dx + 1, dy, px + 1, py, visited);
			if (py != 0)
				result += MonPackSpace(dx, dy - 1, px, py - 1, visited);
			if (py != MON_PACK_AREA - 1)
				result += MonPackSpace(dx, dy + 1, px, py + 1, visited);
		}
	}
	return result;
}

static void PlaceUniqueMonst(int uniqindex, int mtidx)
{
	int xp, yp;
	int count2;
	int mnum, count;

	switch (uniqindex) {
	case UMT_ZHAR:
		if (zharlib == -1)
			return;
		xp = themes[zharlib]._tsx1 + 3;
		yp = themes[zharlib]._tsy1 + 3;
		break;
	default:
		count = 1000;
		while (true) {
			xp = random_(91, DSIZEX) + DBORDERX;
			yp = random_(91, DSIZEY) + DBORDERY;
			static_assert(DBORDERX >= MON_PACK_DISTANCE, "PlaceUniqueMonst does not check IN_DUNGEON_AREA but expects a large enough border I.");
			static_assert(DBORDERY >= MON_PACK_DISTANCE, "PlaceUniqueMonst does not check IN_DUNGEON_AREA but expects a large enough border II.");
			bool visited[MON_PACK_SPACE][MON_PACK_SPACE] = { 0 };
			count2 = MonPackSpace(xp, yp, MON_PACK_DISTANCE, MON_PACK_DISTANCE, visited);
			if (count2 < 2 * MON_PACK_SIZE) {
				if (count2 == 0 || --count != 0) {
					continue;
				}
			}
			break;
		}
	}
	// assert(nummonsters < MAXMONSTERS);
	mnum = PlaceMonster(mtidx, xp, yp);
	unsigned flags = InitUniqueMonster(mnum, uniqindex);
	if (flags & UMF_GROUP) {
		PlaceGroup(mtidx, MON_PACK_SIZE - 1, flags, mnum);
	}
}

static void PlaceUniques()
{
	int u, mt;

	for (u = 0; uniqMonData[u].mtype != MT_INVALID; u++) {
		if (uniquetrans >= NUM_COLOR_TRNS)
			continue;
		/*if (uniqMonData[u].muLevelIdx != currLvl._dLevelIdx)
			continue;
		if (uniqMonData[u].mQuestId != Q_INVALID
		 && quests[uniqMonData[u].mQuestId]._qactive == QUEST_NOTAVAIL)
			continue;*/
		if (uniqMonData[u].muLevelIdx != currLvl._dLevelNum)
			continue;
		if (uniqMonData[u].mQuestId != Q_INVALID && !QuestStatus(uniqMonData[u].mQuestId))
			continue;
		for (mt = 0; mt < nummtypes; mt++) {
			if (mapMonTypes[mt].cmType == uniqMonData[u].mtype) {
				PlaceUniqueMonst(u, mt);
				break;
			}
		}
	}
}

static void SetMapMonsters(int idx)
{
	int startx = DBORDERX + pSetPieces[idx]._spx * 2;
	int starty = DBORDERY + pSetPieces[idx]._spy * 2;
	const BYTE* pMap = pSetPieces[idx]._spData;
	uint16_t rw, rh, *lm, mtype;
	int i, j;
	int mtidx, mnum;
	bool posOk;

	lm = (uint16_t*)pMap;
	rw = SwapLE16(*lm);
	lm++;
	rh = SwapLE16(*lm);
	lm++;
	lm += rw * rh; // skip dun
	rw <<= 1;
	rh <<= 1;
	lm += rw * rh; // skip items?

	rw += startx;
	rh += starty;
	for (j = starty; j < rh; j++) {
		for (i = startx; i < rw; i++) {
			if (*lm != 0) {
				mtype = SwapLE16(*lm);
				// assert(nummonsters < MAXMONSTERS);
				posOk = PosOkActor(i, j);
				if ((mtype & (1 << 15)) == 0) {
					mtidx = AddMonsterType(MonstConvTbl[mtype], FALSE);
					mnum = PlaceMonster(mtidx, i, j);
				} else {
					mtype = (mtype & INT16_MAX) - 1;
					mtidx = AddMonsterType(uniqMonData[mtype].mtype, FALSE);
					// assert(uniquetrans < NUM_COLOR_TRNS);
					mnum = PlaceMonster(mtidx, i, j);
					InitUniqueMonster(mnum, mtype);
				}
				if (!posOk) {
					dMonster[i][j] = 0;
					monsters[mnum]._mmode = MM_RESERVED;
					// assert(monsters[mnum]._mlid == NO_LIGHT);
					//ChangeLightRadius(monsters[mnum]._mlid, 0);
				}
			}
			lm++;
		}
	}
}

static void PlaceSetMapMonsters()
{
	for (int i = lengthof(pSetPieces) - 1; i >= 0; i--) {
		if (pSetPieces[i]._spData != NULL) { // pSetPieces[i]._sptype != SPT_NONE
			SetMapMonsters(i);
		}
	}
}

void InitMonsters()
{
	unsigned na, numplacemonsters, numscattypes;
	int i, j, xx, yy;
	int mtidx;
	int scatteridx[MAX_LVLMTYPES];
	const int tdx[4] = { -1, -1,  2,  2 };
	const int tdy[4] = { -1,  2, -1,  2 };

#if DEBUG_MODE
	if (IsMultiGame)
		CheckDungeonClear();
#endif
	// reserve the entry/exit area
	for (i = lengthof(pWarps) - 1; i >= 0; i--) {
		if (pWarps[i]._wx == 0)
			continue;
		if (i == DWARP_EXIT && currLvl._dLevelIdx == DLV_HELL3)
			continue;
		static_assert(MAX_LIGHT_RAD >= 15, "Tile reservation in InitMonsters requires at least 15 light radius.");
		for (j = lengthof(tdx) - 1; j >= 0; j--)
			DoVision(pWarps[i]._wx + tdx[j], pWarps[i]._wy + tdy[j], 15, false);
	}
	// if (currLvl._dLevelIdx == DLV_HELL3) {
	//	DoVision(quests[Q_BETRAYER]._qtx + 2, quests[Q_BETRAYER]._qty + 2, 4, false);
	// }
	// place the setmap/setpiece monsters
	PlaceSetMapMonsters();
	// if (!currLvl._dSetLvl) {
		// calculate the available space for monsters
		na = 0;
		for (xx = DBORDERX; xx < DSIZEX + DBORDERX; xx++)
			for (yy = DBORDERY; yy < DSIZEY + DBORDERY; yy++)
				if ((nSolidTable[dPiece[xx][yy]] | (dFlags[xx][yy] & (BFLAG_ALERT | BFLAG_MON_PROTECT))) == 0)
					na++;
		na = na * AllLevels[currLvl._dLevelNum].dMonDensity / 32;

		numplacemonsters = na / 32;
		totalmonsters = nummonsters + numplacemonsters;
		if (totalmonsters > MAXMONSTERS - 10)
			totalmonsters = MAXMONSTERS - 10;
		// place quest/unique monsters
		PlaceUniques();
		numscattypes = 0;
		for (i = 0; i < nummtypes; i++) {
			if (mapMonTypes[i].cmPlaceScatter) {
				scatteridx[numscattypes] = i;
				numscattypes++;
			}
		}
		// assert(numscattypes != 0 || na == 0);
		i = currLvl._dLevelIdx;
		while (nummonsters < totalmonsters) {
			mtidx = scatteridx[random_low(95, numscattypes)];
			if (i == DLV_CATHEDRAL1 || random_(95, 2) == 0)
				na = 1;
#ifdef HELLFIRE
			else if (i == DLV_CATHEDRAL2 || (i >= DLV_CRYPT1 && i <= DLV_CRYPT4))
#else
			else if (i == DLV_CATHEDRAL2)
#endif
				na = RandRange(2, 3);
			else
				na = RandRange(2, 5);
			PlaceGroup(mtidx, na, 0, 0);
		}
	// }
	// revert entry/exit area reservation
	for (i = lengthof(pWarps) - 1; i >= 0; i--) {
		if (pWarps[i]._wx == 0)
			continue;
		if (i == DWARP_EXIT && currLvl._dLevelIdx == DLV_HELL3)
			continue;
		for (j = lengthof(tdx) - 1; j >= 0; j--)
			DoUnVision(pWarps[i]._wx + tdx[j], pWarps[i]._wy + tdy[j], 15);
	}
	// if (currLvl._dLevelIdx == DLV_HELL3) {
	//	DoUnVision(quests[Q_BETRAYER]._qtx + 2, quests[Q_BETRAYER]._qty + 2, 4, false);
	// }
}

void MonChangeMap()
{
	int mnum;

	for (mnum = MAX_MINIONS; mnum < MAXMONSTERS; mnum++) {
		if (monsters[mnum]._mmode == MM_RESERVED
		 && PosOkActor(monsters[mnum]._mx, monsters[mnum]._my)) {
			dMonster[monsters[mnum]._mx][monsters[mnum]._my] = mnum + 1;
			monsters[mnum]._mmode = MM_STAND;
			// assert(monsters[mnum]._mlid == NO_LIGHT);
			//ChangeLightRadius(monsters[mnum]._mlid, MON_LIGHTRAD);
		}
	}
}

static bool CheckAllowMissile(int x, int y)
{
	int oi;

	if (nMissileTable[dPiece[x][y]])
		return false;

	oi = dObject[x][y];
	if (oi != 0) {
		oi = oi >= 0 ? oi - 1 : -(oi + 1);
		return objects[oi]._oMissFlag;
	}

	return true;
}

static bool CheckVisible(int x, int y)
{
	return !nMissileTable[dPiece[x][y]];
}

static int _gnCheckMnum;
static bool CheckMonMissile(int x, int y)
{
	return PosOkMonst(_gnCheckMnum, x, y);
}

/**
 * Walks from (x1; y1) to (x2; y2) and calls the Clear check for 
 * every position inbetween.
 * The target and source positions are NOT checked.
 * @return TRUE if the Clear checks succeeded.
 */
static bool LineClearF(bool (*Clear)(int, int), int x1, int y1, int x2, int y2)
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
			// shift start/end positions to account for the missile trajectory
			// (starting from the back of the tile, ending at the front of the tile)
			x1++;
			y1 += dy < 0 ? -1 : (dy != 0 ? 1 : 0);
			if (dx == 1) // x1 == x2
				return true;
			if (!Clear(x1, y1))
				return false; // dy == 0 && dx == 1;
		}
		// find out step size and direction on the y coordinate
		if (dy >= 0) {
			xyinc = 1;
		} else {
			dy = -dy;
			xyinc = -1;
		}
		// multiply by 2 so we round up -- keep in sync with GetMissilePos + handle swap above!
		//dy *= 2;
		d = 0;
		do {
			d += dy;
			if (d >= dx) {
				d -= /*2 **/ dx; // multiply by 2 to support rounding
				y1 += xyinc;
				if (d != 0 && !Clear(x1, y1))
					break;
			}
			x1++;
			if (x1 == x2)
				return true;
		} while (Clear(x1, y1));
	} else {
		// alway proceed from lower to higher y
		if (dy < 0) {
			tmp = y1;
			y1 = y2;
			y2 = tmp;
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			dy = -dy;
			dx = -dx;
			// shift start/end positions to account for the missile trajectory
			// (starting from the back of the tile, ending at the front of the tile)
			y1++;
			x1 += dx < 0 ? -1 : (dx != 0 ? 1 : 0);
			if (dy == 1) // y1 == y2
				return true;
			if (!Clear(x1, y1))
				return false; // dx == 0 && dy == 1;
		}
		// find out step size and direction on the x coordinate
		if (dx >= 0) {
			xyinc = 1;
		} else {
			dx = -dx;
			xyinc = -1;
		}
		// multiply by 2 so we round up
		//dx *= 2; -- keep in sync with GetMissilePos + handle swap above!
		d = 0;
		do {
			d += dx;
			if (d >= dy) {
				d -= /*2 **/ dy; // multiply by 2 to support rounding
				x1 += xyinc;
				if (d != 0 && !Clear(x1, y1))
					break;
			}
			y1++;
			if (y1 == y2)
				return true;
		} while (Clear(x1, y1));
	}
	return false;
}

// test if the destination (x2;y2) is 'visible' from the source (x1;y1)
bool LineClear(int x1, int y1, int x2, int y2)
{
	return LineClearF(CheckVisible, x1, y1, x2, y2);
}

static bool EnemyInLine(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];

	//if (!MON_HAS_ENEMY) -- does not really help. usually there is an enemy. If not the LineClearF supposed to fail.
	//	return false;
	return LineClearF(CheckAllowMissile, mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy);
}

/**
 * Same as LineClearF, only with a different Clear function.
 */
static bool LineClearMon(int mnum, int x1, int y1, int x2, int y2)
{
	_gnCheckMnum = mnum;
	return LineClearF(CheckMonMissile, x1, y1, x2, y2);
}

static void NewMonsterAnim(int mnum, int anim, int md)
{
	MonsterStruct* mon = &monsters[mnum];
	MonAnimStruct* as = &mon->_mAnims[anim];

	mon->_mdir = md;
	mon->_mAnimData = as->maAnimData[md];
	mon->_mAnimLen = as->maFrames;
	// assert(gbGameLogicMnum <= mnum || anim == MA_STAND);
	mon->_mAnimCnt = gbGameLogicProgress < GLP_MONSTERS_DONE ? -1 : 0;
	mon->_mAnimFrame = 1;
	mon->_mAnimFrameLen = as->maFrameLen;
	mon->_mFlags &= ~(MFLAG_REV_ANIMATION | MFLAG_LOCK_ANIMATION);
}

static void MonFindEnemy(int mnum)
{
	int i, tnum;
	int enemy, dist, best_dist;
	bool sameroom, bestsameroom;
	MonsterStruct *mon = &monsters[mnum], *tmon;
	const BYTE tv = dTransVal[mon->_mfutx][mon->_mfuty];
	int x, y;

	enemy = 0;
	best_dist = MAXDUNX + MAXDUNY;
	bestsameroom = false;
	if ((dFlags[mon->_mfutx][mon->_mfuty] & BFLAG_ALERT) == 0) {
		// enemy = 0;
	} else if (mnum >= MAX_MINIONS) {
		for (i = 0; i < MAX_PLRS; i++) {
			if (!plx(i)._pActive || currLvl._dLevelIdx != plx(i)._pDunLevel ||
				plx(i)._pInvincible/*plx(i)._pLvlChanging || plx(i)._pHitPoints == 0*/)
				continue;
			if ((plx(i)._pmode < PM_WALK || plx(i)._pmode > PM_WALK2) || plx(i)._pAnimFrame <= (plx(i)._pAnimLen >> 1)) {
				x = plx(i)._px;
				y = plx(i)._py;
			} else {
				x = plx(i)._pfutx;
				y = plx(i)._pfuty;
			}
			if (!LineClear(mon->_mfutx, mon->_mfuty, x, y))
				continue;
			sameroom = tv == dTransVal[x][y];
			dist = std::max(abs(mon->_mfutx - x), abs(mon->_mfuty - y));
			if (sameroom == bestsameroom) {
				if (dist > best_dist)
					continue;
				if (dist == best_dist) {
					if (mon->_menemy != i)
						continue;
				}
			} else if (!sameroom)
				continue;
			enemy = i + 1;
			best_dist = dist;
			bestsameroom = sameroom;
		}
		for (i = 0; i < MAX_MINIONS; i++) {
			tmon = &monsters[i];
			if (tmon->_mmode > MM_INGAME_LAST)
				continue;
			if (tmon->_mmode == MM_DEATH)
				continue;
			//if (tmon->_mFlags & MFLAG_HIDDEN)
			//	continue;
			if ((tmon->_mmode < MM_WALK || tmon->_mmode > MM_WALK2) || tmon->_mAnimFrame <= (tmon->_mAnimLen >> 1)) {
				x = tmon->_mx;
				y = tmon->_my;
			} else {
				x = tmon->_mfutx;
				y = tmon->_mfuty;
			}
			if (!LineClear(mon->_mfutx, mon->_mfuty, x, y))
				continue;
			dist = std::max(abs(mon->_mfutx - x), abs(mon->_mfuty - y));
			sameroom = tv == dTransVal[x][y];
			if (sameroom == bestsameroom) {
				if (dist > best_dist)
					continue;
				if (dist == best_dist) {
					if (mon->_menemy != -(i + 1))
						continue;
				}
			} else if (!sameroom)
				continue;
			enemy = -(i + 1);
			best_dist = dist;
			bestsameroom = sameroom;
		}
	} else {
		for (tnum = 0; tnum < MAXMONSTERS; tnum++) {
			if (tnum == mnum)
				continue;
			tmon = &monsters[tnum];
			if (tmon->_mmode > MM_INGAME_LAST)
				continue;
			if (tmon->_mmode == MM_DEATH)
				continue;
			if (tmon->_mFlags & MFLAG_HIDDEN)
				continue;
			if (tmon->_mgoal == MGOAL_TALKING) // CanTalkToMonst(tnum)
				continue;
			if ((tmon->_mmode < MM_WALK || tmon->_mmode > MM_WALK2) || tmon->_mAnimFrame <= (tmon->_mAnimLen >> 1)) {
				x = tmon->_mx;
				y = tmon->_my;
			} else {
				x = tmon->_mfutx;
				y = tmon->_mfuty;
			}
			if (!LineClear(mon->_mfutx, mon->_mfuty, x, y))
				continue;
			// if (!(dFlags[x][y] & BFLAG_ALERT)) - stick to line of sight to prevent stuck golems in multiplayer games
			//	continue;
			dist = std::max(abs(mon->_mfutx - x), abs(mon->_mfuty - y));
			sameroom = tv == dTransVal[x][y];
			if (sameroom == bestsameroom) {
				if (dist > best_dist)
					continue;
				if (dist == best_dist) {
					if (mon->_menemy != -(tnum + 1))
						continue;
				}
			} else if (!sameroom)
				continue;
			enemy = -(tnum + 1);
			best_dist = dist;
			bestsameroom = sameroom;
		}
	}
	// clear previous target-flags
	if (enemy != 0) {
		if (enemy > 0) {
			enemy--;
			x = plx(enemy)._pfutx;
			y = plx(enemy)._pfuty;
		} else {
			tnum = -(enemy + 1);
			x = monsters[tnum]._mfutx;
			y = monsters[tnum]._mfuty;
		}
		mon->_menemy = enemy;
		mon->_mlastx = x;
		mon->_mlasty = y;
	} else {
		x = 0;
		y = 0;
	}
	mon->_menemyx = x;
	mon->_menemyy = y;
}

static void MonEnemyInfo(int mnum)
{
	int mx, my, dx, dy;

	mx = monsters[mnum]._mx;
	my = monsters[mnum]._my;

	currEnemyInfo._meLastDir = GetDirection(mx, my, monsters[mnum]._mlastx, monsters[mnum]._mlasty);

	dx = monsters[mnum]._menemyx - mx;
	dy = monsters[mnum]._menemyy - my;

	currEnemyInfo._meRealDir = GetDirection(0, 0, dx, dy);
	currEnemyInfo._meRealDist = std::max(abs(dx), abs(dy));
}

//static int MonEnemyRealDir(int mnum)
//{
//	return GetDirection(monsters[mnum]._mx, monsters[mnum]._my, monsters[mnum]._menemyx, monsters[mnum]._menemyy);
//}

static int MonEnemyLastDir(int mnum)
{
	return GetDirection(monsters[mnum]._mx, monsters[mnum]._my, monsters[mnum]._mlastx, monsters[mnum]._mlasty);
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

static void AssertFixMonLocation(int mnum)
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
	MonsterStruct* mon = &monsters[mnum];

	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_STAND, mon->_mdir);
	mon->_mVar1 = mon->_mmode; // STAND_PREV_MODE : previous mode of the monster
	mon->_mVar2 = 0;           // STAND_TICK : the time spent on standing
	mon->_mmode = MM_STAND;
	// MonFindEnemy(mnum);
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

	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_SPECIAL, md);
	mon = &monsters[mnum];
	mon->_mmode = MM_SPSTAND;
}

static void MonStartAttack(int mnum)
{
	int md = currEnemyInfo._meRealDir;
	MonsterStruct* mon;

	// assert(md == MonEnemyRealDir(mnum));
	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_ATTACK, md);
	mon = &monsters[mnum];
	mon->_mmode = MM_ATTACK;
}

static void MonStartRAttack(int mnum, int mitype)
{
	int md = currEnemyInfo._meRealDir;
	MonsterStruct* mon;

	// assert(md == MonEnemyRealDir(mnum));
	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_ATTACK, md);
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
	int md = currEnemyInfo._meRealDir;
	MonsterStruct* mon;

	// assert(md == MonEnemyRealDir(mnum));
	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_SPECIAL, md);
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

	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);

	mon->_mmode = MM_SPATTACK;
}

/*
 * Start fade in using the special effect of monsters.
 * Used by: Sneak, Fireman, Mage, DarkMage
 */
static void MonStartFadein(int mnum, bool backwards)
{
	MonsterStruct* mon = &monsters[mnum];

	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);

	mon->_mmode = MM_FADEIN;
	mon->_mFlags &= ~MFLAG_HIDDEN;
	if (backwards) {
		mon->_mFlags |= MFLAG_REV_ANIMATION;
		mon->_mAnimFrame = mon->_mAnimLen;
		mon->_mVar8 = 1;              // FADE_END : target frame to end the fade mode
	} else {                          //
		mon->_mVar8 = mon->_mAnimLen; // FADE_END : target frame to end the fade mode
	}
}

static void MonStartFadeout(int mnum, bool backwards)
{
	MonsterStruct* mon = &monsters[mnum];

	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);

	mon->_mmode = MM_FADEOUT;
	if (backwards) {
		mon->_mFlags |= MFLAG_REV_ANIMATION;
		mon->_mAnimFrame = mon->_mAnimLen;
		mon->_mVar8 = 1;              // FADE_END : target frame to end the fade mode
	} else {                          //
		mon->_mVar8 = mon->_mAnimLen; // FADE_END : target frame to end the fade mode
	}
}

static void MonStartHeal(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];

	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_SPECIAL, mon->_mdir);

	mon->_mAnimFrame = mon->_mAnimLen;
	mon->_mFlags |= MFLAG_REV_ANIMATION;
	mon->_mmode = MM_HEAL;
	static_assert((SQUELCH_MAX - SQUELCH_LOW) >= 16 * 8, "MonStartHeal might relax while healing.");
	// assert(mon->_msquelch == SQUELCH_MAX);
	// assert(mon->_mmaxhp >= 2 * 64);
	mon->_mVar1 = mon->_mmaxhp / (16 * RandRange(5, 8)); // HEAL_SPEED
}

static void MonStartGetHit(int mnum, int dir)
{
	MonsterStruct* mon = &monsters[mnum];
	// assert(mon->_mmode != MM_DEATH && mon->_mmode != MM_STONE && mon->_mType != MT_GOLEM);
	AssertFixMonLocation(mnum);
	NewMonsterAnim(mnum, MA_GOTHIT, dir);

	mon->_mmode = MM_GOTHIT;
}

static void MonStartDelay(int mnum, int len)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	mon->_mVar2 = len; // DELAY_TICK : length of the delay
	mon->_mmode = MM_DELAY;
}

/**
 * @brief Starting a move action towards NW, N, NE or W
 */
static void MonStartWalk1(int mnum, int xvel, int yvel, int dir)
{
	MonsterStruct* mon;
	int mx, my;

	mon = &monsters[mnum];
	mon->_mmode = MM_WALK;
	mon->_mVar4 = xvel; // WALK_XVEL : velocity of the monster in the X-direction
	mon->_mVar5 = yvel; // WALK_YVEL : velocity of the monster in the Y-direction
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

	mx += offset_x[dir];
	my += offset_y[dir];
	mon->_mfutx = mx;
	mon->_mfuty = my;

	dMonster[mx][my] = -(mnum + 1);
}

/**
 * @brief Starting a move action towards SW, S, SE or E
 */
static void MonStartWalk2(int mnum, int xvel, int yvel, int xoff, int yoff, int dir)
{
	MonsterStruct* mon;
	int mx, my;

	mon = &monsters[mnum];
	mon->_mmode = MM_WALK2;
	mon->_mVar4 = xvel; // WALK_XVEL : velocity of the monster in the X-direction
	mon->_mVar5 = yvel; // WALK_YVEL : velocity of the monster in the Y-direction
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
	mx += offset_x[dir];
	my += offset_y[dir];
	mon->_mx = mon->_mfutx = mx;
	mon->_my = mon->_mfuty = my;
	dMonster[mx][my] = mnum + 1;
	// assert(mon->_mlid == NO_LIGHT);
	//if (mon->_mlid != NO_LIGHT && !(mon->_mFlags & MFLAG_HIDDEN)) {
	//	ChangeLightXY(mon->_mlid, mx, my);
	//	ChangeLightScreenOff(mon->_mlid, mon->_mxoff, mon->_myoff);
	//}
}

/*
 * Disconnect monster from its pack/leader temporarily.
 */
void MonLeaveLeader(int mnum)
{
	int ma;

	//if (monsters[mnum]._mleaderflag == MLEADER_NONE)
	//	return;
	if (monsters[mnum]._mleaderflag == MLEADER_PRESENT) {
		monsters[mnum]._mleaderflag = MLEADER_AWAY;
		monsters[monsters[mnum]._mleader]._mpacksize--;
	} else if (monsters[mnum]._mleaderflag == MLEADER_SELF) {
		monsters[mnum]._mpacksize = 0;
		// assert(mnum + MON_PACK_SIZE <= MAXMONSTERS);
		for (ma = mnum + 1; ma < mnum + MON_PACK_SIZE; ma++) {
			if (/*monsters[ma]._mleaderflag != MLEADER_NONE && */monsters[ma]._mleader == mnum) {
				monsters[ma]._mleaderflag = MLEADER_AWAY;
			}
		}
	}
}

/*
 * Disconnect monster from its pack/leader permanently.
 */
void MonUpdateLeader(int mnum)
{
	int ma;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonUpdateLeader: Invalid monster %d", mnum);
	}
	if (monsters[mnum]._mleaderflag == MLEADER_NONE)
		return;
	if (monsters[mnum]._mleaderflag == MLEADER_SELF) {
		// assert(mnum + MON_PACK_SIZE <= MAXMONSTERS);
		for (ma = mnum + 1; ma < mnum + MON_PACK_SIZE; ma++) {
			if (/*monsters[ma]._mleaderflag != MLEADER_NONE && */monsters[ma]._mleader == mnum) {
				monsters[ma]._mleader = MON_NO_LEADER;
				monsters[ma]._mleaderflag = MLEADER_NONE;
			}
		}
	} else if (monsters[mnum]._mleaderflag == MLEADER_PRESENT) {
		monsters[monsters[mnum]._mleader]._mpacksize--;
	}
	monsters[mnum]._mleader = MON_NO_LEADER;
	monsters[mnum]._mleaderflag = MLEADER_NONE;
	monsters[mnum]._mpacksize = 0;
	// assert(monsters[mnum]._mvid == NO_VISION);
	monsters[mnum]._mvid = NO_VISION;
}

void RemoveMonFromMap(int mnum)
{
	MonsterStruct* mon;
	int m1;

	m1 = mnum + 1;
	mon = &monsters[mnum];
	if (abs(dMonster[mon->_moldx][mon->_moldy]) == m1)
		dMonster[mon->_moldx][mon->_moldy] = 0;
	if (abs(dMonster[mon->_mfutx][mon->_mfuty]) == m1)
		dMonster[mon->_mfutx][mon->_mfuty] = 0;
#if DEV_MODE
	int x, y, mx, my;

	mx = mon->_moldx;
	my = mon->_moldy;
	for (x = mx - 1; x <= mx + 1; x++) {
		for (y = my - 1; y <= my + 1; y++) {
			if (abs(dMonster[x][y]) == m1)
				app_fatal("dMonster not cleared on %d:%d mode:%d ai:%d goal:%d", x, y, mon->_mmode, mon->_mAI.aiType, mon->_mgoal);
		}
	}
#endif
}

static void MonPlace(int mnum)
{
	MonsterStruct* mon;
	int mx, my;

	// set monster position to x/y with zero offset
	FixMonLocation(mnum);
	// move light/vision of the monster
	mon = &monsters[mnum];
	mx = mon->_mx;
	my = mon->_my;
	// assert(mon->_mlid == NO_LIGHT || (LightList[mon->_mlid]._lx == mx && LightList[mon->_mlid]._ly == my));
	//if (mon->_mlid != NO_LIGHT && !(mon->_mFlags & MFLAG_HIDDEN))
	//	ChangeLightXYOff(mon->_mlid, mx, my);
	if (mon->_mvid != NO_VISION)
		ChangeVisionXY(mon->_mvid, mx, my);
	// place monster in the new position
	dMonster[mx][my] = mnum + 1;
}

static void MonStopWalk(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int x, y;

	// assert(mon->_mmode != MM_STONE);
	if (mon->_mmode < MM_WALK || mon->_mmode > MM_WALK2)
		return;

	// assert(mon->_mAnims[MA_WALK].maFrames == mon->_mAnimLen);
	if (mon->_mAnimFrame > (mon->_mAnimLen >> 1)) {
		x = mon->_mfutx;
		y = mon->_mfuty;
	} else {
		x = mon->_moldx;
		y = mon->_moldy;
	}
	mon->_mx = x;
	mon->_my = y;
	RemoveMonFromMap(mnum);
	MonPlace(mnum);
	MonStartStand(mnum);
}

static int MonTeleport(int mnum, int tx, int ty, int dir)
{
	MonsterStruct* mon = &monsters[mnum];
	int i, oldx, oldy, newx, newy, rx;
	// assert(mon->_mmode != MM_DEATH && mon->_mmode != MM_STONE);
	AssertFixMonLocation(mnum);

	oldx = mon->_mx;
	oldy = mon->_my;
	rx = random_(100, NUM_DIRS);
	static_assert(DBORDERX >= 1, "MonTeleport expects a large enough border I.");
	static_assert(DBORDERY >= 1, "MonTeleport expects a large enough border II.");
	for (i = 0; i < lengthof(offset_x); i++, rx = (rx + 1) & 7) {
		newx = tx + offset_x[rx];
		newy = ty + offset_y[rx];
		// assert(IN_DUNGEON_AREA(newx, newy));
		if (newx != oldx && newy != oldy && PosOkMonst(mnum, newx, newy)) {
			mon->_mx = newx;
			mon->_my = newy;
			// assert(OPPOSITE(rx) == GetDirection(newx, newy, tx, ty));
			dir = rx;
			RemoveMonFromMap(mnum);
			MonPlace(mnum);
			MonLeaveLeader(mnum);
			break;
		}
	}
	return dir;
}

static void MonGetKnockback(int mnum, int dir)
{
	MonsterStruct* mon = &monsters[mnum];
	int oldx, oldy, newx, newy;
	// assert(mon->_mmode != MM_DEATH && mon->_mmode != MM_STONE);
	AssertFixMonLocation(mnum);

	oldx = mon->_mx;
	oldy = mon->_my;
	if (PathWalkable(oldx, oldy, dir2pdir[dir])) {
		newx = oldx + offset_x[dir];
		newy = oldy + offset_y[dir];
		if (PosOkMonster(mnum, newx, newy)) {
			mon->_mx = newx;
			mon->_my = newy;
			RemoveMonFromMap(mnum);
			MonPlace(mnum);
			MonLeaveLeader(mnum);
		}
	}
}

void MonHitByPlr(int mnum, int pnum, int dam, unsigned hitflags, int dir)
{
	MonsterStruct* mon;
	bool knockback, stun;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by player", mnum);
	}
	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("Invalid player %d hitting monster %d", pnum, mnum);
	}
	mon = &monsters[mnum];
	// assert(mon->_mmode != MM_DEATH);
	if (pnum == mypnum) {
		NetSendCmdMonstDamage(mnum, mon->_mhitpoints);
	}
	PlayMonSfx(mnum, MS_GOTHIT);
	// assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_BLEED));
	if (mon->_mType != MT_GOLEM && mon->_mmode != MM_STONE) {
		if (mon->_mFlags & MFLAG_CAN_BLEED && (hitflags & ISPL_FAKE_CAN_BLEED)
		 && ((hitflags & ISPL_BLEED) ? random_(47, 32) == 0 : random_(48, 64) == 0))
			AddMissile(0, 0, 0, 0, 0, MIS_BLEED, MST_PLAYER, pnum, mnum);
		knockback = (hitflags & ISPL_KNOCKBACK) != 0;
		stun = (dam << ((hitflags & ISPL_STUN) ? 3 : 2)) >= mon->_mmaxhp;
		if (knockback || stun) {
			MonStopWalk(mnum);
			if (knockback)
				MonGetKnockback(mnum, dir);
			if (stun && mon->_mType == MT_NBAT)
				dir = MonTeleport(mnum, plr._pfutx, plr._pfuty, dir);
			MonStartGetHit(mnum, OPPOSITE(dir));
		}
	}
}

void MonHitByMon(int defm, int offm, int dam, int dir)
{
	MonsterStruct* dmon;
	bool stun;

	if ((unsigned)defm >= MAXMONSTERS) {
		dev_fatal("Invalid monster %d getting hit by monster/trap", defm);
	}
	dmon = &monsters[defm];
	// assert(dmon->_mmode != MM_DEATH);
	if ((unsigned)offm < MAX_MINIONS) {
		static_assert(MAX_MINIONS == MAX_PLRS, "M2MStartHit requires that owner of a monster has the same id as the monster itself.");
		if (offm == mypnum) {
			NetSendCmdMonstDamage(defm, dmon->_mhitpoints);
		}
	}
	PlayMonSfx(defm, MS_GOTHIT);
	// assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_BLEED));
	if (dmon->_mType != MT_GOLEM && dmon->_mmode != MM_STONE) {
		// TODO: implement monster vs. monster knockback & bleed?
		//       assert(!(monsterdata[MT_GOLEM].mFlags & MFLAG_KNOCKBACK));
		stun = (dam << 2) >= dmon->_mmaxhp;
		if (stun) {
			MonStopWalk(defm);
			if (/*stun && */dmon->_mType == MT_NBAT && offm >= 0)
				dir = MonTeleport(defm, monsters[offm]._mfutx, monsters[offm]._mfuty, dir);
			MonStartGetHit(defm, OPPOSITE(dir));
		}
	}
}

static void MonFallenFear(int x, int y)
{
	MonsterStruct* mon;
	int i;

	for (i = 0; i < MAXMONSTERS; i++) {
		mon = &monsters[i];
		if (!MON_RELAXED // TODO: use LineClear instead to prevent retreat behind walls?
		 && mon->_mAI.aiType == AI_FALLEN
		 && abs(x - mon->_mx) < 5
		 && abs(y - mon->_my) < 5
		 && mon->_mhitpoints != 0
		 && mon->_mAI.aiInt < 4) {
#if DEBUG
			assert(mon->_mAnims[MA_WALK].maFrames * mon->_mAnims[MA_WALK].maFrameLen * (8 - 2 * 0) < SQUELCH_MAX - SQUELCH_LOW);
			assert(mon->_mmode <= MM_INGAME_LAST);
#endif
			static_assert((8 - 2 * 0) * 12 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Fallen might relax with retreat goal.");
			mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
			mon->_mgoal = MGOAL_RETREAT;
			mon->_mgoalvar1 = 8 - 2 * mon->_mAI.aiInt; // RETREAT_DISTANCE
			mon->_mdir = GetDirection(x, y, mon->_mx, mon->_my);
		}
	}
}

static void MonDiabloDeath(int mnum)
{
	MonsterStruct* mon;
	int i, mx, my;
	unsigned killLevel;

	for (i = 0; i < MAXMONSTERS; i++) {
		// commented out because this is a pointless complexity
		//if (i == mnum)
		//	continue;
		mon = &monsters[i];
		if (mon->_mmode > MM_INGAME_LAST)
			continue;
		RemoveMonFromMap(i);
		MonPlace(i);
		//if (mon->_msquelch == 0)
		//	continue;
		mon->_mhitpoints = 0;
		if (mon->_mmode != MM_STONE) {
			NewMonsterAnim(i, MA_DEATH, mon->_mdir);
			mon->_mmode = MM_DEATH;
		} else {
			dMonster[mon->_mx][mon->_my] = 0;
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
	killLevel = gnDifficulty + 1;
	for (i = 0; i < MAX_PLRS; i++) {
		// ensure the players are not processed and not sending level-delta
		// might produce a slight inconsistent state in case a player enters the level
		// while the diablo is still dying. TODO: find a better solution for hosted games?
		if (plx(i)._pActive && plx(i)._pDunLevel == DLV_HELL4) {
			plx(i)._pLvlChanging = TRUE;
			plx(i)._pInvincible = TRUE;
			// grant rank TODO: sync with other players?
			if (killLevel > plx(i)._pRank)
				plx(i)._pRank = killLevel;
		}
	}
}

static void SpawnLoot(int mnum, bool sendmsg)
{
	MonsterStruct* mon;
	int mx, my;

	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_NODROP) {
		return;
	}
	SetRndSeed(mon->_mRndSeed);
	mx = mon->_mx;
	my = mon->_my;
	switch (mon->_muniqtype - 1) {
	case UMT_GARBUD:
		// assert(QuestStatus(Q_GARBUD));
		CreateTypeItem(mx, my, CFDQ_GOOD, ITYPE_MACE, IMISC_NONE, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
		return;
	case UMT_LAZARUS:
		//if (IsSfxStreaming(USFX_LAZ1)) // minitxtdata[TEXT_VILE13].sfxnr
			StopStreamSFX();
		break;
	case UMT_SKELKING:
	case UMT_BUTCHER:
		if (!IsMultiGame) {
			int idx = (mon->_muniqtype - 1 == UMT_BUTCHER) ? UITEM_CLEAVER : UITEM_SKCROWN;
			SpawnUnique(idx, mx, my, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
			return;
		}
		break;
	case UMT_ZAMPHIR:
		if (quests[Q_MUSHROOM]._qactive != QUEST_NOTAVAIL && currLvl._dLevelIdx == uniqMonData[UMT_ZAMPHIR].muLevelIdx) {
			SpawnQuestItemAt(IDI_BRAIN, mx, my, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
			return;
		}
		break;
#ifdef HELLFIRE
	case UMT_HORKDMN:
		if (quests[Q_GIRL]._qactive != QUEST_NOTAVAIL && currLvl._dLevelIdx == uniqMonData[UMT_HORKDMN].muLevelIdx) {
			SpawnQuestItemAt(IDI_THEODORE, mx, my, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
			return;
		}
		break;
	case UMT_DEFILER:
		// assert(QuestStatus(Q_DEFILER));
		//if (IsSfxStreaming(USFX_DEFILER8)) // minitxtdata[TEXT_DEFILER5].sfxnr
			StopStreamSFX();
		// quests[Q_DEFILER]._qlog = FALSE;
		quests[Q_DEFILER]._qactive = QUEST_DONE;
		if (sendmsg)
			NetSendCmdQuest(Q_DEFILER, false); // recipient should not matter
		SpawnQuestItemAt(IDI_FANG, mx, my, sendmsg ? ICM_SEND_FLIP : ICM_DUMMY);
		return;
	case UMT_NAKRUL:
		//    minitxtdata[TEXT_NAKRUL4].sfxnr   minitxtdata[TEXT_NAKRUL5].sfxnr   minitxtdata[TEXT_NAKRUL?].sfxnr
		//if (IsSfxStreaming(USFX_NAKRUL4) || IsSfxStreaming(USFX_NAKRUL5) || IsSfxStreaming(USFX_NAKRUL6))
			StopStreamSFX();
		quests[Q_NAKRUL]._qactive = QUEST_DONE;
		// quests[Q_NAKRUL]._qvar1 = quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKOPEN ? QV_NAKRUL_DEADOPEN : QV_NAKRUL_DEAD; // set to new state so innocent monsters are not 'woke' -- does not sync if quest was done
		if (sendmsg)
			NetSendCmdQuest(Q_NAKRUL, false); // recipient should not matter
		/*SpawnMagicItem(ITYPE_SWORD, ICURS_GREAT_SWORD, mon->_mx, mon->_my, sendmsg);
		SpawnMagicItem(ITYPE_STAFF, ICURS_WAR_STAFF, mon->_mx, mon->_my, sendmsg);
		SpawnMagicItem(ITYPE_BOW, ICURS_LONG_WAR_BOW, mon->_mx, mon->_my, sendmsg);*/
		SpawnMonItem(mnum, mx, my, sendmsg); // double reward
		break;
#endif
	}
	SpawnMonItem(mnum, mx, my, sendmsg);
}

static void MonInitKill(int mnum, int mpnum, bool sendmsg)
{
	MonsterStruct* mon;

	// fix the location of the monster before spawning loot or sending a message
	RemoveMonFromMap(mnum);
	MonPlace(mnum);

	mon = &monsters[mnum];
	if (mon->_mmode != MM_STONE) {
		mon->_mmode = MM_DEATH;
		// TODO: might want to turn towards the offending enemy. Might not, though...
		NewMonsterAnim(mnum, MA_DEATH, mon->_mdir);
	} else {
		dMonster[mon->_mx][mon->_my] = 0;
	}
	mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
	mon->_mhitpoints = 0;
	// if (mnum < MAX_MINIONS) {
		AddUnVision(mon->_mvid);
	//} else {
		MonUpdateLeader(mnum);
		SpawnLoot(mnum, sendmsg);
	// }
	CheckQuestKill(mnum, sendmsg);
	if (sendmsg) {
		static_assert(MAXMONSTERS <= UCHAR_MAX, "MonInitKill uses mnum as pnum, which must fit to BYTE.");
		NetSendCmdMonstKill(mnum, mpnum);
	}

	if (mon->_mType == MT_DIABLO)
		MonDiabloDeath(mnum);
	else
		PlayMonSfx(mnum, MS_DEATH);

	MonFallenFear(mon->_mx, mon->_my);
#ifdef HELLFIRE
	if ((mon->_mType >= MT_NACID && mon->_mType <= MT_XACID) || mon->_mType == MT_SPIDLORD)
#else
	if (mon->_mType >= MT_NACID && mon->_mType <= MT_XACID)
#endif
		AddMissile(mon->_mx, mon->_my, 0, 0, 0, MIS_ACIDPUD, MST_MONSTER, mnum, 1);
}

void MonKill(int mnum, int mpnum)
{
	bool sendmsg;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonKill: Invalid monster %d", mnum);
	}
#if 0 // commented out because it seems to be an unnecessary complexity
	static_assert(MAX_MINIONS == MAX_PLRS, "MonKill requires that owner of a monster has the same id as the monster itself.");
	// check if it is a plr/golem vs. monster/golem -> the attacker('s owner) should send the message
	sendmsg = mpnum == mypnum;
	if (!sendmsg) {
		// not a kill by the local plr/golem -> check if the attacker is an active player on the level
		if ((unsigned)mpnum >= MAX_PLRS || !plx(mpnum)._pActive || plx(mpnum)._pLvlChanging || plx(mpnum)._pDunLevel != currLvl._dLevelIdx) {
			// select the first active player on the level
			for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
				if (plr._pActive && !plr._pLvlChanging && plr._pDunLevel == currLvl._dLevelIdx) {
					sendmsg = pnum == mypnum;
					break;
				}
			}
		}
	}
#else
	sendmsg = true;
#endif

	MonInitKill(mnum, mpnum, sendmsg);
}

void MonSyncKill(int mnum, int x, int y, int pnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonSyncKill: Invalid monster %d", mnum);
	}
	if (monsters[mnum]._mmode == MM_DEATH || monsters[mnum]._mmode > MM_INGAME_LAST
	 || (monsters[mnum]._mmode == MM_STONE && monsters[mnum]._mhitpoints == 0)) {
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

	MonInitKill(mnum, pnum, false);
}

static bool MonDoStand(int mnum)
{
	MonsterStruct* mon;

	//if (((gdwGameLogicTurn + mnum) % 16) == 0)
	//	MonFindEnemy(mnum);

	mon = &monsters[mnum];
	mon->_mAnimData = mon->_mAnims[MA_STAND].maAnimData[mon->_mdir];
	mon->_mVar2++; // STAND_TICK

	return false;
}

static bool MonDoWalk(int mnum)
{
	MonsterStruct* mon;
	bool rv;

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
			//mon->_mVar8++;
			mon->_mVar6 += mon->_mVar4; // MWALK_XOFF <- WALK_XVEL
			mon->_mVar7 += mon->_mVar5; // MWALK_YOFF <- WALK_YVEL
			mon->_mxoff = mon->_mVar6 >> MON_WALK_SHIFT;
			mon->_myoff = mon->_mVar7 >> MON_WALK_SHIFT;
			// assert(mon->_mlid == NO_LIGHT);
			//if (mon->_mlid != NO_LIGHT && !(mon->_mFlags & MFLAG_HIDDEN))
			//	CondChangeLightScreenOff(mon->_mlid, mon->_mxoff, mon->_myoff);
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
	if (!CheckMonsterHit(defm, &ret))
		return;

	hper += 30 + 2 * monsters[offm]._mLevel;
	hper -= monsters[defm]._mArmorClass;
	if (CheckHit(hper) || monsters[defm]._mmode == MM_STONE) {
		int dam = RandRange(mind, maxd) << 6;
		monsters[defm]._mhitpoints -= dam;
		if (monsters[defm]._mhitpoints < (1 << 6)) {
			MonKill(defm, offm);
		} else {
			MonHitByMon(defm, offm, dam, monsters[offm]._mdir);
		}
	}
}

static void MonHitPlr(int mnum, int pnum, int hper, int MinDam, int MaxDam)
{
	MonsterStruct* mon;
	int dam;
	unsigned hitFlags;

	if ((unsigned)pnum >= MAX_PLRS) {
		dev_fatal("MonHitPlr: Invalid player %d", pnum);
	}
	mon = &monsters[mnum];

	if (plr._pInvincible)
		return;

	hper += 30 + (2 * mon->_mLevel);
	hper -= plr._pIAC;
	if (!CheckHit(hper))
		return;

	if (PlrCheckBlock(pnum, mon->_mLevel, OPPOSITE(mon->_mdir)))
		return;

	if (mon->_mType == MT_YZOMBIE && pnum == mypnum) {
		NetSendCmd(CMD_DECHP);
	}
	/*if (plr._pIFlags & ISPL_THORNS) {
		dam = RandRange(1, 3) << 6;
		mon->_mhitpoints -= dam;
		if (mon->_mhitpoints < (1 << 6))
			MonKill(mnum, pnum);
		else
			MonHitByPlr(mnum, pnum, dam, OPPOSITE(mon->_mdir));
	}*/
	dam = RandRange(MinDam, MaxDam) << 6;
	dam -= plr._pIAbsAnyHit + plr._pIAbsPhyHit;
	if (dam <= 0) {
		dam = 1;
	}
	if (mon->_mFlags & MFLAG_LIFESTEAL) {
		mon->_mhitpoints += dam;
		if (mon->_mhitpoints > mon->_mmaxhp)
			mon->_mhitpoints = mon->_mmaxhp;
	}
	if (!PlrDecHp(pnum, dam, DMGTYPE_NPC)) {
		hitFlags = (mon->_mFlags & ISPL_HITFLAGS_MASK) | ISPL_FAKE_CAN_BLEED;
		static_assert((int)MFLAG_KNOCKBACK == (int)ISPL_KNOCKBACK, "MonHitPlr uses _mFlags as hitFlags.");
		PlrHitByAny(pnum, mnum, dam, hitFlags, mon->_mdir);
	}
}

static void MonTryH2HHit(int mnum, int Hit, int MinDam, int MaxDam)
{
	MonsterStruct* mon;
	int mpnum;

	mon = &monsters[mnum];
	if (mon->_menemy >= 0) {
		mpnum = dPlayer[mon->_mx + offset_x[mon->_mdir]][mon->_my + offset_y[mon->_mdir]];
		if (mpnum == 0)
			return;
		mpnum = CheckPlrCol(mpnum);
		if (mpnum < 0)
			return;
		MonHitPlr(mnum, mpnum, Hit, MinDam, MaxDam);
	} else {
		mpnum = dMonster[mon->_mx + offset_x[mon->_mdir]][mon->_my + offset_y[mon->_mdir]];
		if (mpnum == 0)
			return;
		mpnum = CheckMonCol(mpnum);
		if (mpnum < 0)
			return;
		MonHitMon(mnum, mpnum, Hit, MinDam, MaxDam);
	}
}

static bool MonDoAttack(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum) {
		MonTryH2HHit(mnum, mon->_mHit, mon->_mMinDamage, mon->_mMaxDamage);
		if (mon->_mAI.aiType != AI_SNAKE)
			PlayMonSfx(mnum, MS_ATTACK);
	} else if (mon->_mFileNum == MOFILE_MAGMA && mon->_mAnimFrame == 9) {
		// mon->_mType >= MT_NMAGMA && mon->_mType <= MT_WMAGMA
		MonTryH2HHit(mnum, mon->_mHit + 10, mon->_mMinDamage - 2, mon->_mMaxDamage - 2);
		PlayMonSfx(mnum, MS_ATTACK);
	} else if (mon->_mFileNum == MOFILE_THIN && mon->_mAnimFrame == 13) {
		// mon->_mType >= MT_RTHIN && mon->_mType <= MT_GTHIN
		MonTryH2HHit(mnum, mon->_mHit - 20, mon->_mMinDamage + 4, mon->_mMaxDamage + 4);
		PlayMonSfx(mnum, MS_ATTACK);
	} else if (mon->_mFileNum == MOFILE_SNAKE && mon->_mAnimFrame == 1)
		PlayMonSfx(mnum, MS_ATTACK);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoRAttack(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum) {
		AddMissile(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy, mon->_mdir, mon->_mVar1, MST_MONSTER, mnum, 0); // RATTACK_SKILL
		PlayMonSfx(mnum, MS_ATTACK);
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoRSpAttack(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2) {
		if (mon->_mAnimCnt == 0) {
			AddMissile(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy, mon->_mdir, mon->_mVar1, MST_MONSTER, mnum, 0); // SPATTACK_SKILL
			PlayMonSfx(mnum, MS_SPECIAL);
		}

		if (mon->_mFileNum == MOFILE_MEGA) {
			if (mon->_mAnimCnt++ == 0) {
				mon->_mFlags |= MFLAG_LOCK_ANIMATION;
			} else if (mon->_mAnimCnt == 15) {
				mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
			}
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoSpAttack(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2)
		MonTryH2HHit(mnum, mon->_mHit2, mon->_mMinDamage2, mon->_mMaxDamage2);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoFadein(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame) // FADE_END
		return false;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	// assert(mon->_mlid == NO_LIGHT);
	//if (mon->_mlid != NO_LIGHT) { // && !(mon->_mFlags & MFLAG_HIDDEN)) {
	//	ChangeLightRadius(mon->_mlid, MON_LIGHTRAD);
	//	ChangeLightXYOff(mon->_mlid, mon->_mx, mon->_my);
	//}
	MonStartStand(mnum);
	return true;
}

static bool MonDoFadeout(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mVar8 != mon->_mAnimFrame) // FADE_END
		return false;

	mon->_mFlags &= ~MFLAG_REV_ANIMATION;
	//if (mon->_mType < MT_INCIN || mon->_mType > MT_HELLBURN) {
		mon->_mFlags |= MFLAG_HIDDEN;
		// assert(mon->_mlid == NO_LIGHT);
		//ChangeLightRadius(mon->_mlid, 0);
	//}
	MonStartStand(mnum);
	return true;
}

static bool MonDoHeal(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	/*if (mon->_mFlags & MFLAG_NOHEAL) {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		mon->_mmode = MM_SPATTACK;
		return false;
	}*/

	if (mon->_mAnimFrame == 1) {
		mon->_mFlags &= ~MFLAG_REV_ANIMATION;
		if (mon->_mVar1 + mon->_mhitpoints < mon->_mmaxhp) { // HEAL_SPEED
			mon->_mhitpoints = mon->_mVar1 + mon->_mhitpoints;
			mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		} else {
			mon->_mhitpoints = mon->_mmaxhp;
			// MonStartSpAttack(mnum);
			mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
			mon->_mmode = MM_SPATTACK;
		}
	}
	return false;
}

static bool MonDoTalk(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	MonStartStand(mnum);
	// assert(mon->_mgoal == MGOAL_TALKING);
	mon->_mgoalvar1 = TRUE; // TALK_SPEAKING
	if (!IsSfxStreaming(minitxtdata[mon->_mgoalvar2].sfxnr)) // TALK_MESSAGE
		StartQTextMsg(mon->_mgoalvar2, !IsMultiGame /*mon->_mListener == mypnum*/); // TALK_MESSAGE
	return false;
}

static bool MonDoGotHit(int mnum)
{
	if (monsters[mnum]._mAnimFrame == monsters[mnum]._mAnimLen) {
		MonStartStand(mnum);
		return true;
	}
	return false;
}

void DoEnding()
{
	// music_stop(); -- no need, music is already stopped at this point

	const char* vicSets[NUM_CLASSES] = {
		"gendata\\DiabVic2.smk", "gendata\\DiabVic3.smk", "gendata\\DiabVic1.smk",
#ifdef HELLFIRE
		"gendata\\DiabVic1.smk", "gendata\\DiabVic3.smk", "gendata\\DiabVic2.smk",
#endif
	};
	if (play_movie(vicSets[myplr._pClass], 0) == MPR_DONE
	 && play_movie("gendata\\Diabend.smk", 0) == MPR_DONE
	// they tried to play TMUSIC_L2 in vanilla but failed, because
	// music is stopped/paused in play_movie
	 && !IsMultiGame) // skip movie in multiplayer games to prevent overflow due to pending turns
		play_movie("gendata\\loopdend.smk", MOV_SKIP | MOV_LOOP);
}

static void PrepDoEnding()
{
	gbCineflag = true;
	NetSendCmd(CMD_RETOWN);
}

/*
 * Add the corpse of the monster.
 *
 * @param mnum: the monster which died
 */
void MonAddDead(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	dDead[mon->_mx][mon->_my] = dDead[mon->_mx][mon->_my] == 0 ? mnum + 1 : DEAD_MULTI;
}

static bool MonDoDeath(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mType == MT_DIABLO) {
		if (mon->_mx != myview.x)
			myview.x += mon->_mx > myview.x ? 1 : -1;
		if (mon->_my != myview.y)
			myview.y += mon->_my > myview.y ? 1 : -1;

		if (--mon->_mVar1 == 0) // DIABLO_TICK
			PrepDoEnding();
	} else if (mon->_mAnimFrame == mon->_mAnimLen) {
		// TODO: RemoveMonFromGame ?
		mon->_mAnimCnt = -1;
		// reset squelch value to simplify MonFallenFear, sync_all_monsters and LevelDeltaExport
		mon->_msquelch = 0;
		mon->_mmode = mnum >= MAX_MINIONS ? ((mon->_mFlags & MFLAG_NOCORPSE) ? MM_UNUSED : MM_DEAD) : MM_RESERVED;
		if (mnum >= MAX_MINIONS)
			nummonsters--;
		dMonster[mon->_mx][mon->_my] = 0;
		if (!(mon->_mFlags & MFLAG_NOCORPSE))
			MonAddDead(mnum);
	}
	return false;
}

static bool MonDoSpStand(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mAnimFrame == mon->_mAFNum2)
		PlayMonSfx(mnum, MS_SPECIAL);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum);
		return true;
	}

	return false;
}

static bool MonDoDelay(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	mon->_mdir = MonEnemyLastDir(mnum);
	mon->_mAnimData = mon->_mAnims[MA_STAND].maAnimData[mon->_mdir];

	if (mon->_mVar2-- <= 0) { // DELAY_TICK
		mon->_mmode = MM_STAND;
		mon->_mVar1 = MM_DELAY; // STAND_PREV_MODE
		// MonFindEnemy(mnum);
		return true;
	}

	return false;
}

static bool monster_posok(int mnum, int x, int y)
{
	if ((dFlags[x][y] & BFLAG_HAZARD) == 0)
		return true;
	return (monsters[mnum]._mMagicRes & MORS_FIRE_IMMUNE) == MORS_FIRE_IMMUNE;
}

static bool MonDoCharge(int mnum)
{
	MonsterStruct* mon;
	int dir;

	mon = &monsters[mnum];
	dir = mon->_mdir;
	if (!monster_posok(mnum, mon->_mx + offset_x[dir], mon->_my + offset_y[dir])) {
		//assert(dMonster[mon->_mx][mon->_my] == -(mnum + 1));
		dMonster[mon->_mx][mon->_my] = mnum + 1;
		MonStartStand(mnum);
		return true;
	}
	return false;
}

static bool MonDoStone(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (mon->_mVar3 == MM_DEATH) {
		// TODO: RemoveMonFromGame ?
		// mon->_mAnimFrame = mon->_mAnimLen;
		// mon->_mAnimCnt = -1;
		// reset squelch value to simplify MonFallenFear, sync_all_monsters and LevelDeltaExport
		mon->_msquelch = 0;
		// assert(mnum >= MAX_MINIONS);
		// mon->_mmode = (mon->_mFlags & MFLAG_NOCORPSE) ? MM_UNUSED : MM_DEAD;
		mon->_mmode = MM_UNUSED;
		nummonsters--;
	}

	return false;
}

static void MonWalkDir(int mnum, int md)
{
	int mwi;

	NewMonsterAnim(mnum, MA_WALK, md);
#ifdef HELLFIRE
	if (monsters[mnum]._mType == MT_FLESTHNG)
		PlayMonSfx(mnum, MS_SPECIAL);
#endif
	mwi = MWVel[monsters[mnum]._mAnimLen - 1];
	static_assert(TILE_WIDTH / TILE_HEIGHT == 2, "MonWalkDir relies on fix width/height ratio of the floor-tile.");
	switch (md) {
	case DIR_N:
		MonStartWalk1(mnum, 0, -(mwi >> 1), md);
		break;
	case DIR_NE:
		MonStartWalk1(mnum, (mwi >> 1), -(mwi >> 2), md);
		break;
	case DIR_E:
		MonStartWalk2(mnum, mwi, 0, -TILE_WIDTH, 0, md);
		break;
	case DIR_SE:
		MonStartWalk2(mnum, (mwi >> 1), (mwi >> 2), -TILE_WIDTH/2, -TILE_HEIGHT/2, md);
		break;
	case DIR_S:
		MonStartWalk2(mnum, 0, (mwi >> 1), 0, -TILE_HEIGHT, md);
		break;
	case DIR_SW:
		MonStartWalk2(mnum, -(mwi >> 1), (mwi >> 2), TILE_WIDTH/2, -TILE_HEIGHT/2, md);
		break;
	case DIR_W:
		MonStartWalk1(mnum, -mwi, 0, md);
		break;
	case DIR_NW:
		MonStartWalk1(mnum, -(mwi >> 1), -(mwi >> 2), md);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void ActivateSpawn(int mnum, int x, int y, int dir)
{
	dMonster[x][y] = mnum + 1;
	SetMonsterLoc(&monsters[mnum], x, y);
	MonStartSpStand(mnum, dir);
	monsters[mnum]._msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
}

static void GroupUnity(int mnum)
{
	MonsterStruct *mon, *leader, *bmon;
	int ma;
	bool clear;

	mon = &monsters[mnum];
	// track/update enemy
	MonFindEnemy(mnum);
	// check if the leader is still available and update its squelch value + enemy location
	if (mon->_mleader != MON_NO_LEADER) {
		leader = &monsters[mon->_mleader];
		clear = LineClear(mon->_mx, mon->_my, leader->_mfutx, leader->_mfuty);
		if (clear
		 && abs(mon->_mx - leader->_mfutx) <= MON_PACK_DISTANCE
		 && abs(mon->_my - leader->_mfuty) <= MON_PACK_DISTANCE) {
			if (mon->_mleaderflag == MLEADER_AWAY) {
				leader->_mpacksize++;
				mon->_mleaderflag = MLEADER_PRESENT;
			}
		} else if (mon->_mleaderflag == MLEADER_PRESENT) { // MonLeaveLeader
			mon->_mleaderflag = MLEADER_AWAY;
			leader->_mpacksize--;
		}
		if (mon->_mleaderflag == MLEADER_PRESENT) {
			if (mon->_msquelch > leader->_msquelch) {
				leader->_mlastx = mon->_mlastx; // BUGFIX: use _mlastx instead of _mx (fixed)
				leader->_mlasty = mon->_mlasty; // BUGFIX: use _mlasty instead of _my (fixed)
				leader->_msquelch = mon->_msquelch - 1;
			}
		}
	}
	// update squelch value + enemy location of the pack monsters
	if (mon->_mpacksize != 0) {
		// assert(mnum + MON_PACK_SIZE <= MAXMONSTERS);
		for (ma = mnum + 1; ma < mnum + MON_PACK_SIZE; ma++) {
			bmon = &monsters[ma];
			if (bmon->_mleaderflag == MLEADER_PRESENT && bmon->_mleader == mnum) {
				if (mon->_msquelch > bmon->_msquelch) {
					bmon->_mlastx = mon->_mlastx; // BUGFIX: use _mlastx instead of _mx (fixed)
					bmon->_mlasty = mon->_mlasty; // BUGFIX: use _mlasty instead of _my (fixed)
					bmon->_msquelch = mon->_msquelch - 1;
				}
			}
		}
	}
}

static bool MonDirOK(int mnum, int mdir)
{
	int fx, fy;
	int x, y;
	int mcount, ma;

	x = monsters[mnum]._mx;
	y = monsters[mnum]._my;
	if (!PathWalkable(x, y, dir2pdir[mdir]))
		return false;

	fx = x + offset_x[mdir];
	fy = y + offset_y[mdir];
	static_assert(DBORDERX >= MON_PACK_DISTANCE, "DirOK expects a large enough border I.");
	static_assert(DBORDERY >= MON_PACK_DISTANCE, "DirOK expects a large enough border II.");
	assert(IN_DUNGEON_AREA(fx, fy));
	if (!PosOkMonst(mnum, fx, fy))
		return false;

	if (monsters[mnum]._mleaderflag == MLEADER_PRESENT) {
		return abs(fx - monsters[monsters[mnum]._mleader]._mfutx) <= MON_PACK_DISTANCE
		    && abs(fy - monsters[monsters[mnum]._mleader]._mfuty) <= MON_PACK_DISTANCE;
	}
	mcount = monsters[mnum]._mpacksize;
	if (mcount == 0)
		return true;
	// assert(mnum + MON_PACK_SIZE <= MAXMONSTERS);
	for (ma = mnum + 1; ma < mnum + MON_PACK_SIZE; ma++) {
		if (monsters[ma]._mleaderflag == MLEADER_PRESENT && monsters[ma]._mleader == mnum
		 && abs(fx - monsters[ma]._mfutx) <= MON_PACK_DISTANCE
		 && abs(fy - monsters[ma]._mfuty) <= MON_PACK_DISTANCE) {
			mcount--;
		}
	}
	return mcount == 0;
}

static int MonFindDir(int mnum, int md)
{
	int mdtemp;
	bool ok;

	// try desired direction
	ok = MonDirOK(mnum, md);
	if (!ok) {
		mdtemp = md;
		// Randomly go left or right
		if (random_(101, 2) != 0)
			ok = (md = (mdtemp - 1) & 7, MonDirOK(mnum, md))
			  || (md = (mdtemp + 1) & 7, MonDirOK(mnum, md));
		else
			ok = (md = (mdtemp + 1) & 7, MonDirOK(mnum, md))
			  || (md = (mdtemp - 1) & 7, MonDirOK(mnum, md));
		if (!ok) {
			// Randomly go further left or right
			if (random_(102, 2) != 0)
				ok = (md = (mdtemp + 2) & 7, MonDirOK(mnum, md))
				  || (md = (mdtemp - 2) & 7, MonDirOK(mnum, md));
			else
				ok = (md = (mdtemp - 2) & 7, MonDirOK(mnum, md))
				  || (md = (mdtemp + 2) & 7, MonDirOK(mnum, md));
		}
	}
	return ok ? md : -1;
}

static bool MonCallWalk(int mnum, int md)
{
	md = MonFindDir(mnum, md);
	if (md >= 0)
		MonWalkDir(mnum, md);
	//else
	//	MonFindEnemy(mnum); // prevent from stucking with an inaccessible enemy - not necessary if GroupUnity runs it unconditionally
	return md >= 0;
}

static bool MonDestWalk(int mnum)
{
	MonsterStruct* mon;
	int8_t path[MAX_PATH_LENGTH];
	bool (*Check)(int, int, int);
	int md;

	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_SEARCH) {
		Check = (mon->_mFlags & MFLAG_CAN_OPEN_DOOR) != 0 ? PosOkMonst3 : PosOkMonst;
		if (mon->_mFlags & MFLAG_CAN_OPEN_DOOR)
			MonstCheckDoors(mon->_mx, mon->_my);
		md = FindPath(Check, mnum, mon->_mx, mon->_my, mon->_mlastx, mon->_mlasty, path);
		if (md > 0) { // found path to the enemy -> go
			md = path[0];
		} else if (md != 0) { // could not find path to the enemy -> just go in its generic direction
			md = currEnemyInfo._meLastDir;
		} else { // enemy disappeared -> walk around randomly
			md = random_(145, NUM_DIRS);
			for (int i = 0; i < NUM_DIRS; i++) {
				if (MonDirOK(mnum, md)) {
					mon->_mlastx += offset_x[md];
					mon->_mlasty += offset_y[md];
					break;
				}
				md = (md + 1) & 7;
			}
		}
	} else {
		md = currEnemyInfo._meLastDir;
	}

	return MonCallWalk(mnum, md);
}

static bool MonDumbWalk(int mnum, int md)
{
	bool ok;

	ok = MonDirOK(mnum, md);
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

	ok = MonDirOK(mnum, md);
	mdtemp = md;
	if (!ok) {
		if (*dir) {
			ok = (md = (mdtemp + 1) & 7, MonDirOK(mnum, md))
			  || (md = (mdtemp + 2) & 7, MonDirOK(mnum, md));
		} else {
			ok = (md = (mdtemp - 1) & 7, MonDirOK(mnum, md))
			  || (md = (mdtemp - 2) & 7, MonDirOK(mnum, md));
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
	MonsterStruct* mon = &monsters[mnum];
	int md;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	if (random_(103, 100) < 2 * mon->_mAI.aiInt + 10) {
		MonEnemyInfo(mnum);
		md = currEnemyInfo._meRealDist;
		if (md >= 2) {
			if (md >= 2 * mon->_mAI.aiInt + 4) {
				md = mon->_mdir;
				if (random_(104, 100) < 2 * mon->_mAI.aiInt + 20) {
					md = random_(104, NUM_DIRS);
				}
			} else {
				md = currEnemyInfo._meLastDir;
			}
			MonCallWalk(mnum, md);
		} else {
			MonStartAttack(mnum);
		}
	}
}

void MAI_SkelSd(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mon->_mdir = currEnemyInfo._meLastDir;
	if (currEnemyInfo._meRealDist >= 2) { // STAND_PREV_MODE
		if (mon->_mVar1 == MM_DELAY || (random_(106, 100) >= 35 - 4 * mon->_mAI.aiInt)) {
			MonDestWalk(mnum);
		} else {
			MonStartDelay(mnum, RandRange(16, 23) - 2 * mon->_mAI.aiInt);
		}
	} else {
		if (mon->_mVar1 == MM_DELAY || (random_(105, 100) < 2 * mon->_mAI.aiInt + 20)) {
			MonStartAttack(mnum);
		} else {
			MonStartDelay(mnum, RandRange(11, 18) - 2 * mon->_mAI.aiInt);
		}
	}
}

void MAI_Snake(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int dist, md;
	const BYTE pattern[6] = { 1, 1, 0, 7, 7, 0 };
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR));
	mon->_mdir = currEnemyInfo._meLastDir;
	dist = currEnemyInfo._meRealDist;
	if (dist >= 2) { // STAND_PREV_MODE
		if (dist == 2 && LineClearMon(mnum, mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy) && mon->_mVar1 != MM_CHARGE) {
			if (AddMissile(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy, mon->_mdir, MIS_RHINO, MST_MONSTER, mnum, 0) != -1) {
				PlayMonSfx(mnum, MS_ATTACK);
				MonLeaveLeader(mnum);
			}
		} else if (mon->_mVar1 == MM_DELAY || random_(106, 100) >= 35 - 2 * mon->_mAI.aiInt) {
			// calculate the desired direction
			md = mon->_mdir + pattern[mon->_mgoalvar1]; // SNAKE_DIRECTION_DELTA
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
				MonDestWalk(mnum);
		} else {
			MonStartDelay(mnum, RandRange(16, 23) - mon->_mAI.aiInt);
		}
	} else { // STAND_PREV_MODE
		if (mon->_mVar1 == MM_DELAY
		 || mon->_mVar1 == MM_CHARGE
		 || (random_(105, 100) < mon->_mAI.aiInt + 20)) {
			MonStartAttack(mnum);
		} else
			MonStartDelay(mnum, RandRange(11, 18) - mon->_mAI.aiInt);
	}
}

void MAI_Bat(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int md, v, dist;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// commented out because only a single retreating, unique monster would benefit from this
	// if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
	//	MonstCheckDoors(mon->_mx, mon->_my);
	mon->_mdir = currEnemyInfo._meLastDir;
	if (mon->_mgoal == MGOAL_RETREAT) {
		md = mon->_mdir;
		if (mon->_mgoalvar1 == 0) { // RETREAT_FINISHED
			mon->_mgoalvar1++;
			md = OPPOSITE(md);
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			// md = random_(108, 2) != 0 ? left[md] : right[md];
			md = (md + 2 * random_(108, 2) - 1) & 7;
		}
		MonCallWalk(mnum, md);
		return;
	}

	v = random_(107, 100);
	dist = currEnemyInfo._meRealDist;
	if (mon->_mType == MT_GBAT
	 && dist >= 5
	 && v < 4 * mon->_mAI.aiInt + 33
	 && LineClearMon(mnum, mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy)) {
		if (AddMissile(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy, mon->_mdir, MIS_RHINO, MST_MONSTER, mnum, 0) != -1) {
			MonLeaveLeader(mnum);
		}
	} else if (dist >= 2) {
		if (((unsigned)mon->_mVar2 > MON_WALK_DELAY && v < mon->_mAI.aiInt + 13) // STAND_TICK
		 || (MON_JUST_WALKED && v < mon->_mAI.aiInt + 63)) {
			MonDestWalk(mnum);
		}
	} else if (v < 4 * mon->_mAI.aiInt + 8) {
		MonStartAttack(mnum);
		mon->_mgoal = MGOAL_RETREAT;
		mon->_mgoalvar1 = 0; // RETREAT_FINISHED
		if (mon->_mType == MT_XBAT) {
			AddMissile(mon->_menemyx, mon->_menemyy, 0, 0, -1, MIS_LIGHTNING, MST_MONSTER, mnum, -1);
		}
	}
}

void MAI_SkelBow(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int v;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR));
	mon->_mdir = currEnemyInfo._meLastDir;

	if (currEnemyInfo._meRealDist < 4) {
		v = random_(110, 100);
		if (v < (70 + 8 * mon->_mAI.aiInt)) {
			if (MonDumbWalk(mnum, OPPOSITE(mon->_mdir)))
				return;
		}
	}

	// STAND_PREV_MODE
	if (mon->_mVar1 == MM_DELAY) {
		if (MON_HAS_ENEMY /*&& EnemyInLine(mnum)*/) {
			// assert(LineClear(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy)); -- or just left the view, but who cares...
			MonStartRAttack(mnum, MIS_ARROW);
		} else {
			mon->_mVar1 = MM_STAND; // STAND_PREV_MODE
		}
	} else {
		MonStartDelay(mnum, RandRange(21, 24) - 4 * mon->_mAI.aiInt);
	}
}

void MAI_Fat(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int v;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR) || (mon->_mFlags & MFLAG_SEARCH));
	mon->_mdir = currEnemyInfo._meLastDir;
	v = random_(111, 100);
	if (currEnemyInfo._meRealDist >= 2) {
		if (((unsigned)mon->_mVar2 > MON_WALK_DELAY && v < 4 * mon->_mAI.aiInt + 20) // STAND_TICK
		 || (MON_JUST_WALKED && v < 4 * mon->_mAI.aiInt + 70)) {
			MonDestWalk(mnum);
		}
	} else if (v < 4 * mon->_mAI.aiInt + 15) {
		MonStartAttack(mnum);
	} else if (v < 5 * mon->_mAI.aiInt + 18) {
		MonStartSpAttack(mnum);
	}
}

void MAI_Sneak(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int md, dist, range, v;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	// mx = mon->_mx;
	// my = mon->_my;
	// commented out because dLight is not in-sync in multiplayer games and with the added
	// BFLAG_ALERT check there is not much point to this any more.
	// + dynamic light is now localized
	// TODO: change MonstPlace to prefer non-lit tiles in case of AI_SNEAK?
	//if (dLight[mx][my] == MAXDARKNESS && (dFlags[mx][my] & BFLAG_ALERT) == 0)) {
	//	return;
	//}

	MonEnemyInfo(mnum);
	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR));
	dist = currEnemyInfo._meRealDist;
	range = 7 - mon->_mAI.aiInt;
	if (range < 4)
		range = 4;
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (mon->_mVar1 == MM_GOTHIT) { // STAND_PREV_MODE
			mon->_mgoal = MGOAL_RETREAT;
#if DEBUG
			assert(mon->_mAnims[MA_WALK].maFrames * mon->_mAnims[MA_WALK].maFrameLen * (7 + 2) < SQUELCH_MAX - SQUELCH_LOW);
#endif
			static_assert(12 * (7 + 2) < SQUELCH_MAX - SQUELCH_LOW, "MAI_Sneak might relax with retreat goal.");
			mon->_mgoalvar1 = range + 2; // RETREAT_DISTANCE
		}
	} else {
		if (dist > range || --mon->_mgoalvar1 == 0) { // RETREAT_DISTANCE
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	md = currEnemyInfo._meLastDir;
	if (mon->_mgoal != MGOAL_NORMAL) {
		// assert(mon->_mgoal == MGOAL_RETREAT);
		md = OPPOSITE(md);
		if (mon->_mType == MT_BSNEAK) {
			//md = random_(112, 2) != 0 ? left[md] : right[md];
			md = (md + 2 * random_(112, 2) - 1) & 7;
		}
	}
	mon->_mdir = md;
	v = random_(112, 100);
	range -= 2;
	// assert(range >= 2);
	if (dist < range && (mon->_mFlags & MFLAG_HIDDEN)) {
		MonStartFadein(mnum, false);
	} else if (dist > range && !(mon->_mFlags & MFLAG_HIDDEN)) {
		MonStartFadeout(mnum, true);
	} else if (mon->_mgoal == MGOAL_NORMAL || !MonCallWalk(mnum, mon->_mdir)) {
		// assert(mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT);
		mon->_mgoal = MGOAL_NORMAL;
		if (dist >= 2) {
			if (((unsigned)mon->_mVar2 > MON_WALK_DELAY && v < 4 * mon->_mAI.aiInt + 14) // STAND_TICK
			 || (MON_JUST_WALKED && v < 4 * mon->_mAI.aiInt + 64))
				MonDestWalk(mnum);
		} else {
			if (v < 4 * mon->_mAI.aiInt + 10)
				MonStartAttack(mnum);
		}
	}
}

/*void MAI_Fireman(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int mx, my, fx, fy, md;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	mx = mon->_mx;
	my = mon->_my;
	fx = mon->_menemyx;
	fy = mon->_menemyy;
	md = currEnemyInfo._meRealDir;
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (EnemyInLine(mnum)
		 && AddMissile(mx, my, fx, fy, md, MIS_FIREMAN, MST_MONSTER, mnum, 0) != -1) {
			mon->_mmode = MM_CHARGE;
			mon->_mgoal = MGOAL_ATTACK;
			//mon->_mgoalvar1 = 0; // FIREMAN_ACTION_PROGRESS
		} else {
			if (currEnemyInfo._meRealDist < 2) {
				MonTryH2HHit(mnum, mon->_mHit, mon->_mMinDamage, mon->_mMaxDamage);
				mon->_mgoal = MGOAL_RETREAT;
				md = OPPOSITE(md);
			}
			if (!MonCallWalk(mnum, md)) {
				mon->_mgoal = MGOAL_ATTACK;
				MonStartFadein(mnum, false);
			}
		}
	} else if (mon->_mgoal == MGOAL_ATTACK) {
		if (++mon->_mgoalvar1 > 3) { // FIREMAN_ACTION_PROGRESS
			mon->_mgoal = MGOAL_NORMAL;
			mon->_mgoalvar1 = 0;
			MonStartFadeout(mnum, true);
		} else if (EnemyInLine(mnum)) {
			MonStartRAttack(mnum, MIS_KRULL);
		} else {
			MonStartDelay(mnum, RandRange(6, 13) - mon->_mAI.aiInt);
		}
	} else {
		assert(mon->_mgoal == MGOAL_RETREAT);
		mon->_mgoal = MGOAL_ATTACK;
		MonStartFadein(mnum, false);
	}
}*/

void MAI_Fallen(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int x, y, mx, my, tx, ty, m, rad, amount;
	const int MAX_RAD = 5;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR));
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (random_(113, 48) == 0) {
			MonStartSpStand(mnum, mon->_mdir);
			rad = mon->_mAI.aiInt;
			//if (!(mon->_mFlags & MFLAG_NOHEAL)) {
				amount = mon->_mhitpoints + 2 * rad + 2;
				mon->_mhitpoints = std::min(mon->_mmaxhp, amount);
			//}
			if (MON_HAS_ENEMY) {
				if (rad > MAX_RAD) {
					rad = MAX_RAD;
				}
				amount = 2 * rad + 8;
				rad = 2 * rad + 4;
				static_assert(DBORDERX == DBORDERY && DBORDERX >= 2 * MAX_RAD + 4, "MAI_Fallen expects a large enough border.");
				mx = mon->_mx;
				my = mon->_my;
				tx = mon->_menemyx;
				ty = mon->_menemyy;
				for (y = -rad; y <= rad; y++) {
					for (x = -rad; x <= rad; x++) {
						m = dMonster[x + mx][y + my];
						if (m > 0) {
							mon = &monsters[m - 1];
							if (mon->_mAI.aiType == AI_FALLEN /*&& !MON_RELAXED*/ && (mon->_mleader == MON_NO_LEADER || mon->_mleader == mnum) && LineClear(mx, my, mon->_mx, mon->_my)) {
								mon->_msquelch = SQUELCH_MAX; // prevent monster from getting in relaxed state
#if DEBUG
								assert(mon->_mAnims[MA_WALK].maFrames * mon->_mAnims[MA_WALK].maFrameLen * (2 * MAX_RAD + 8) < SQUELCH_MAX - SQUELCH_LOW);
								assert(mon->_mAnims[MA_ATTACK].maFrames * mon->_mAnims[MA_ATTACK].maFrameLen * (2 * MAX_RAD + 8) < SQUELCH_MAX - SQUELCH_LOW);
								assert(amount * 13 < SQUELCH_MAX - SQUELCH_LOW);
#endif
								static_assert((2 * MAX_RAD + 8) * 13 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Fallen might relax with attack goal.");
								mon->_mgoal = MGOAL_ATTACK;
								mon->_mgoalvar1 = amount; // FALLEN_ATTACK_AMOUNT

								mon->_mlastx = tx;
								mon->_mlasty = ty;
							}
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
			MonStartStand(mnum);
		}
	} else {
		assert(mon->_mgoal == MGOAL_ATTACK);
		if (--mon->_mgoalvar1 != 0) { // FALLEN_ATTACK_AMOUNT
			MonEnemyInfo(mnum);
			if (currEnemyInfo._meRealDist < 2) {
				MonStartAttack(mnum);
			} else {
				if (!MonDestWalk(mnum)) {
					// prevent isolated fallens from burnout
					m = 12 - 1; // mon->_mAnims[MA_WALK].maFrameLen * mon->_mAnims[MA_WALK].maFrames - 1;
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
	MonsterStruct* mon = &monsters[mnum];
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR) || (mon->_mFlags & MFLAG_SEARCH));
	mon->_mdir = currEnemyInfo._meLastDir;

	if (currEnemyInfo._meRealDist >= 2)
		MonDestWalk(mnum);
	else
		MonStartAttack(mnum);
}

/*
 * AI for monsters using special or standard attacks.
 * Attempts to walk in a circle around the target.
 *
 * @param mnum: the id of the monster
 * @param aiParam1: whether the monster should use its special attack
 */
void MAI_Round(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int md, dist, v;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	md = currEnemyInfo._meLastDir;
	dist = currEnemyInfo._meRealDist;
	v = random_(114, 100);
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/) {
		if (mon->_mgoal == MGOAL_MOVE || (dist >= 4 && random_(115, 4) == 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(116, 2); // MOVE_TURN_DIRECTION
			}
			if (mon->_mgoalvar1++ < 2 * dist || !MonDirOK(mnum, md)) {
				MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist >= 2) {
			if (((unsigned)mon->_mVar2 > MON_WALK_DELAY && v < 2 * mon->_mAI.aiInt + 28) // STAND_TICK
			 || (MON_JUST_WALKED && v < 2 * mon->_mAI.aiInt + 78)) {
				MonDestWalk(mnum);
			}
		} else if (v < 2 * mon->_mAI.aiInt + 23) {
			mon->_mdir = md;
			if (mon->_mAI.aiParam1 && mon->_mhitpoints < (mon->_mmaxhp >> 1) && random_(117, 2) != 0)
				MonStartSpAttack(mnum);
			else
				MonStartAttack(mnum);
		}
	}
}

/*
 * AI for monsters using special or standard ranged attacks.
 * Attempts to keep distance from the target, but tries to follow it if its out of sight.
 *
 * @param mnum: the id of the monster
 * @param aiParam1: the missile type to be launched at the end of the attack animation.
 * @param aiParam2: whether the monster should use its special attack
 */
void MAI_Ranged(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int md;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	mon->_mdir = currEnemyInfo._meLastDir;
	if (mon->_msquelch >= SQUELCH_MAX - 1 /* || mon->_menemy < 0*/) {
		bool walking = false;
		if (currEnemyInfo._meRealDist < 4) {
			if (random_(119, 100) < (76 + 8 * mon->_mAI.aiInt))
				walking = MonCallWalk(mnum, OPPOSITE(mon->_mdir));
		}
		if (!walking) {
			md = std::max(1, 20 - mon->_mAI.aiInt);
			md = random_low(118, md); // STAND_PREV_MODE
			if (md == 0 || mon->_mVar1 == MM_DELAY) {
				if (EnemyInLine(mnum)) {
					if (mon->_mAI.aiParam2)
						MonStartRSpAttack(mnum, mon->_mAI.aiParam1);
					else
						MonStartRAttack(mnum, mon->_mAI.aiParam1);
				} else if (currEnemyInfo._meRealDist >= 4
				 && random_(120, 100) < 10 * (mon->_mAI.aiInt + (currEnemyInfo._meRealDist != 4 ? 4 : 0))) {
					MonDestWalk(mnum);
				}
			} else {
				MonStartDelay(mnum, md + 1);
			}
		}
	} else {
		MonDestWalk(mnum);
	}
}

#ifdef HELLFIRE
static void MonConsumeCorpse(MonsterStruct* mon)
{
	int i, n, mx, my;

	mx = mon->_mx;
	my = mon->_my;
	i = dDead[mx][my];
	if (i != DEAD_MULTI) {
		// single body -> consume it
		i--;
		mon = &monsters[i];
		assert(mon->_mmode == MM_DEAD);
		mon->_mmode = MM_UNUSED;
		static_assert(MAXMONSTERS < UCHAR_MAX, "ConsumeCorpse sends mnum in BYTE.");
		NetSendCmdBParam2(CMD_MONSTCORPSE, currLvl._dLevelIdx, i);
		n = 0;
	} else {
		// multiple bodies
		//  1. find the last(top) one
		for (i = MAXMONSTERS - 1; i >= 0; i--) {
			mon = &monsters[i];
			if (mon->_mmode != MM_DEAD || mon->_mx != mx || mon->_my != my)
				continue;
			mon->_mmode = MM_UNUSED;
			static_assert(MAXMONSTERS < UCHAR_MAX, "ConsumeCorpse sends mnum in BYTE.");
			NetSendCmdBParam2(CMD_MONSTCORPSE, currLvl._dLevelIdx, i);
			break;
		}
		assert(i >= 0);
		//  2. find out how many left
		n = DEAD_MULTI;
		while (--i >= 0) {
			mon = &monsters[i];
			if (mon->_mmode != MM_DEAD || mon->_mx != mx || mon->_my != my)
				continue;
			if (n == DEAD_MULTI) {
				n = i;
				continue;
			}
			n = DEAD_MULTI;
			break;
		}
		assert(n == DEAD_MULTI || i < 0);
		n = n == DEAD_MULTI ? n : n + 1;
	}
	//  3. update the matrix
	dDead[mx][my] = n;
}
#endif

void MAI_Scav(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int i, j, tx, ty, maxhp, tmp;
	const int8_t* cr;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	if (mon->_mhitpoints < (mon->_mmaxhp >> 1) && mon->_mgoal != MGOAL_HEALING) {
		MonLeaveLeader(mnum);
		mon->_mgoal = MGOAL_HEALING;
		mon->_mgoalvar1 = 0; // HEALING_LOCATION_X
		//mon->_mgoalvar2 = 0;
#if DEBUG
		assert(mon->_mAnims[MA_SPECIAL].maFrames * mon->_mAnims[MA_SPECIAL].maFrameLen * 9 < SQUELCH_MAX - SQUELCH_LOW);
		assert(mon->_mAnims[MA_WALK].maFrames * mon->_mAnims[MA_WALK].maFrameLen * 9 < SQUELCH_MAX - SQUELCH_LOW);
		assert(24 * 9 < SQUELCH_MAX - SQUELCH_LOW); // max delay of MAI_SkelSd
#endif
		static_assert(24 * 9 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Scav might relax with healing goal.");
		mon->_mgoalvar3 = 9; // HEALING_ROUNDS
	}
	if (mon->_mgoal == MGOAL_HEALING) {
		if (mon->_mgoalvar3 != 0) {
			mon->_mgoalvar3--; // HEALING_ROUNDS
			if (dDead[mon->_mx][mon->_my] != 0) {
				MonStartSpAttack(mnum);
				maxhp = mon->_mmaxhp;
				//if (!(mon->_mFlags & MFLAG_NOHEAL)) {
#ifdef HELLFIRE
					mon->_mhitpoints += maxhp >> 3;
					if (mon->_mhitpoints > maxhp)
						mon->_mhitpoints = maxhp;
					if (mon->_mhitpoints == maxhp || mon->_mgoalvar3 == 0) { // HEALING_ROUNDS
						MonConsumeCorpse(mon);
					}
#else
					mon->_mhitpoints += mon->_mLevel << (6 - 2);
					if (mon->_mhitpoints > maxhp)
						mon->_mhitpoints = maxhp;
					if (mon->_mhitpoints >= (maxhp >> 1) + (maxhp >> 2))
						mon->_mgoal = MGOAL_NORMAL;
#endif
				//}
			} else {
				if (mon->_mgoalvar1 == 0) { // HEALING_LOCATION_X
					static_assert(DBORDERX >= 4, "MAI_Scav expects a large enough border I.");
					static_assert(DBORDERY >= 4, "MAI_Scav expects a large enough border II.");
					static_assert(MAXDUNX < UCHAR_MAX, "MAI_Scav stores dungeon coordinates in BYTE field I.");
					static_assert(MAXDUNY < UCHAR_MAX, "MAI_Scav stores dungeon coordinates in BYTE field II.");
					static_assert(lengthof(CrawlNum) > 4, "MAI_Scav uses CrawlTable/CrawlNum up to radius 4.");
					// assert(CrawlTable[CrawlNum[4]] == 24);
					BYTE corpseLocs[24 * 2];
					tmp = 0;
					for (i = 1; i <= 4; i++) {
						cr = &CrawlTable[CrawlNum[i]];
						for (j = (BYTE)*cr; j > 0; j--) {
							tx = mon->_mx + *++cr;
							ty = mon->_my + *++cr;
							if (dDead[tx][ty] != 0
							 && LineClear(mon->_mx, mon->_my, tx, ty)) {
								corpseLocs[tmp] = tx;
								tmp++;
								corpseLocs[tmp] = ty;
								tmp++;
							}
						}
						if (tmp != 0) {
							tmp = random_low(0, tmp);
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

	mon = &monsters[mnum];
	if (mon->_mFlags & MFLAG_GARG_STONE) {
		if (mon->_mmode != MM_SPATTACK) {
			// assert(mon->_mmode != MM_STONE);
			if (mon->_mleaderflag == MLEADER_NONE) {
				MonStartSpAttack(mnum);
				mon->_mFlags |= MFLAG_LOCK_ANIMATION;
			} else {
				mon->_mFlags &= ~MFLAG_GARG_STONE;
			}
		}
		if (MON_HAS_ENEMY) {
			MonEnemyInfo(mnum);
			// wake up if the enemy is close
			if (currEnemyInfo._meRealDist < mon->_mAI.aiInt + 2) {
				mon->_mFlags &= ~(MFLAG_LOCK_ANIMATION | MFLAG_GARG_STONE);
			}
		}
		return;
	}

	if (MON_RELAXED || MON_ACTIVE)
		return;

	if (mon->_mhitpoints < (mon->_mmaxhp >> 1))
#ifndef HELLFIRE
//		if (!(mon->_mFlags & MFLAG_NOHEAL))
#endif
			mon->_mgoal = MGOAL_RETREAT;
	if (mon->_mgoal == MGOAL_RETREAT) {
		mon->_msquelch = SQUELCH_MAX;
		MonEnemyInfo(mnum);
		if (currEnemyInfo._meRealDist >= mon->_mAI.aiInt + 2) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartHeal(mnum);
		} else if (mon->_mhitpoints == mon->_mmaxhp ||
		 !MonCallWalk(mnum, OPPOSITE(currEnemyInfo._meLastDir))) {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	MAI_Round(mnum);
}

/*
 * AI for ranged monsters using special ranged attacks or standard attacks.
 * Attempts to walk in a circle around the target.
 *
 * @param mnum: the id of the monster
 * @param aiParam1: the missile type to be launched at the end of the attack animation.
 * @param aiParam2: control parameter to reduce the frequency of ranged attacks.
 */
void MAI_RoundRanged(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int dist, v;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	dist = currEnemyInfo._meRealDist;
	//v = random_(121, 10000);
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]*/) {
		if (mon->_mgoal == MGOAL_MOVE || (dist > 2 && random_low(122, 4 << mon->_mAI.aiParam2) == 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(123, 2); // MOVE_TURN_DIRECTION
			}
			/*if (mon->_mgoalvar1++ >= 2 * dist && MonDirOK(mnum, currEnemyInfo._meLastDir)) {
				mon->_mgoal = MGOAL_NORMAL;
			} else if (v < ((6 * (mon->_mAI.aiInt + 1)) >> mon->_mAI.aiParam2) && EnemyInLine(mnum)) {
				MonStartRSpAttack(mnum, mon->_mAI.aiParam1);
			} else {
				MonRoundWalk(mnum, currEnemyInfo._meLastDir, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			}*/
			if (mon->_mgoalvar1++ < 2 * dist || !MonDirOK(mnum, currEnemyInfo._meLastDir)) { // MOVE_DISTANCE
				MonRoundWalk(mnum, currEnemyInfo._meLastDir, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		v = random_(124, 100);
		if (((dist > 2 && v < ((8 * (mon->_mAI.aiInt + 2)) >> mon->_mAI.aiParam2))
		        || v < ((8 * (mon->_mAI.aiInt + 1)) >> mon->_mAI.aiParam2))
			&& EnemyInLine(mnum)) {
			MonStartRSpAttack(mnum, mon->_mAI.aiParam1);
			return;
		} else if (dist >= 2) {
			if (v < 10 * (mon->_mAI.aiInt + 5)
			 || (MON_JUST_WALKED && v < 10 * (mon->_mAI.aiInt + 8))) {
				MonDestWalk(mnum);
				return;
			}
		} else if (v < 10 * (mon->_mAI.aiInt + 6)) {
			MonStartAttack(mnum);
			return;
		}
		MonStartDelay(mnum, RandRange(6, 13) - mon->_mAI.aiInt);
	}
}

/*
 * AI for ranged monsters using special ranged attacks or standard attacks.
 * Attempts to walk in a circle around the target. Ranged attack is limited to distance of 4.
 *
 * @param mnum: the id of the monster
 * @param aiParam1: the missile type to be launched at the end of the attack animation.
 */
void MAI_RoundRanged2(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int dist, v;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	dist = currEnemyInfo._meRealDist;
	/*if (dist >= 5) {
		mon->_mgoal = MGOAL_NORMAL;
		MAI_SkelSd(mnum);
		return;
	}*/

	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	v = random_(121, 100);
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/) {
		if (mon->_mgoal == MGOAL_MOVE || (dist > 2 && dist < 5)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(123, 2); // MOVE_TURN_DIRECTION
			}
			if (mon->_mgoalvar1++ < 2 * dist || !MonDirOK(mnum, currEnemyInfo._meLastDir)) {
				if (v < 5 * (mon->_mAI.aiInt + 16))
					MonRoundWalk(mnum, currEnemyInfo._meLastDir, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist < 5 && (dist > 2 || v < 5 * (mon->_mAI.aiInt + 1)) && EnemyInLine(mnum)) {
			MonStartRSpAttack(mnum, mon->_mAI.aiParam1);
			return;
		}
		v = random_(124, 100);
		if (dist >= 2) {
			if (v < 10 * (mon->_mAI.aiInt + 5)
			 || (MON_JUST_WALKED && v < 10 * (mon->_mAI.aiInt + 8))) {
				MonDestWalk(mnum);
				return;
			}
		} else {
			if (v < 10 * (mon->_mAI.aiInt + 4)) {
				if (random_(124, 2) != 0)
					MonStartAttack(mnum);
				else
					MonStartRSpAttack(mnum, mon->_mAI.aiParam1);
				return;
			}
		}
		MonStartDelay(mnum, RandRange(6, 13) - mon->_mAI.aiInt);
	}
}

void MAI_Golem(int mnum)
{
	MonsterStruct* mon;
	int md, ld, i;

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;
	assert(mon->_msquelch == SQUELCH_MAX);
	if (MON_HAS_ENEMY) {
		MAI_Cleaver(mnum);
		if (mon->_mmode != MM_STAND)
			return;
	}
	if (mon->_mgoal == MGOAL_NORMAL) {
		// go to the player
		int8_t path[MAX_PATH_LENGTH];
		assert(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_OPEN_DOOR);
		if (FindPath(PosOkMonst3, mnum, mon->_mx, mon->_my, plx(mnum)._px, plx(mnum)._py, path) > 1) {
			md = path[0];
			MonCallWalk(mnum, md);
			return;
		}

		mon->_mgoal = MGOAL_MOVE;
		mon->_mgoalvar1 = 0; // MOVE_DIRECTION
	}
	// follow the gaze of the player
	assert(monsterdata[MT_GOLEM].mFlags & MFLAG_CAN_OPEN_DOOR);
	// assert(mon->_mFlags & MFLAG_CAN_OPEN_DOOR);
	MonstCheckDoors(mon->_mx, mon->_my);
	ld = mon->_mdir;
	md = plx(mnum)._pdir;
	if (MonCallWalk(mnum, md)) {
		if (ld != OPPOSITE(mon->_mdir)) {
			return;
		}
		if (++mon->_mgoalvar1 <= 2) {
			return; // MOVE_DIRECTION
		}
		// had to turn twice -> give up following the gaze of the player
		MonStopWalk(mnum);
	}
	// walk around randomly
	mon->_mgoal = MGOAL_NORMAL;
	md = md + 2;
	for (i = 0; i < NUM_DIRS - 5; i++) {
		md = (md + 1) & 7;
		if (MonDirOK(mnum, md)) {
			MonWalkDir(mnum, md);
			break;
		}
	}
}

void MAI_SkelKing(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int nx, ny, md, v, dist;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX) {
		assert(monsterdata[MT_SKING].mFlags & MFLAG_CAN_OPEN_DOOR);
		// assert(mon->_mFlags & MFLAG_CAN_OPEN_DOOR);
		MonstCheckDoors(mon->_mx, mon->_my);
	}
	md = currEnemyInfo._meLastDir;
	v = random_(126, 100);
	dist = currEnemyInfo._meRealDist;
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX) {
		if (mon->_mgoal == MGOAL_MOVE || (dist > 2 && random_(127, 4) == 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(128, 2); // MOVE_TURN_DIRECTION
			}
			if ((mon->_mgoalvar1++ < 2 * dist && MonDirOK(mnum, md)) /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/) {
				MonRoundWalk(mnum, md, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (((dist > 2 && v < 4 * mon->_mAI.aiInt + 35) || v < 6)
			&& MON_HAS_ENEMY /*&& EnemyInLine(mnum)*/) {
			// assert(LineClear(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy)); -- or just left the view, but who cares...
			nx = mon->_mx + offset_x[md];
			ny = mon->_my + offset_y[md];
			if (PosOkMonst(mnum, nx, ny)) {
				v = mapSkelTypes[random_low(136, numSkelTypes)];
				v = SummonMonster(nx, ny, 0, v);
				if (v != -1) {
					MonStartSpStand(v, md);
					MonStartSpStand(mnum, md);
				}
			}
		} else if (dist < 2) {
			if (v < mon->_mAI.aiInt + 20) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(129, 100);
			if (v < mon->_mAI.aiInt + 25
			 || (MON_JUST_WALKED && v < mon->_mAI.aiInt + 75)) {
				MonDestWalk(mnum);
			} else {
				MonStartDelay(mnum, RandRange(11, 18) - mon->_mAI.aiInt);
			}
		}
	}
}

void MAI_Rhino(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int v, dist;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	v = random_(131, 100);
	dist = currEnemyInfo._meRealDist;
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX) {
		if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(133, 2); // MOVE_TURN_DIRECTION
			}
			if (mon->_mgoalvar1++ < 2 * dist /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/) {
				MonRoundWalk(mnum, currEnemyInfo._meLastDir, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist >= 5 && v < 2 * mon->_mAI.aiInt + 43
		 && LineClearMon(mnum, mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy)) {
			mon->_mdir = currEnemyInfo._meLastDir;
			if (AddMissile(mon->_mx, mon->_my, mon->_menemyx, mon->_menemyy, mon->_mdir, MIS_RHINO, MST_MONSTER, mnum, 0) != -1) {
				PlayMonSfx(mnum, MS_SPECIAL);
				MonLeaveLeader(mnum);
			}
		} else if (dist < 2) {
			if (v < 2 * mon->_mAI.aiInt + 28) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mAI.aiInt + 33
			 || (MON_JUST_WALKED && v < 2 * mon->_mAI.aiInt + 83)) {
				MonDestWalk(mnum);
			} else {
				MonStartDelay(mnum, RandRange(11, 18) - mon->_mAI.aiInt);
			}
		}
	}
}

#ifdef HELLFIRE
void MAI_Horkdemon(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int v, dist;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// assert(!(mon->_mFlags & MFLAG_CAN_OPEN_DOOR));
	v = random_(131, 100);
	dist = currEnemyInfo._meRealDist;
	if (dist >= 2 && mon->_msquelch == SQUELCH_MAX) {
		if (mon->_mgoal == MGOAL_MOVE || (dist >= 5 && random_(132, 4) != 0)) {
			if (mon->_mgoal != MGOAL_MOVE) {
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 0;               // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(133, 2); // MOVE_TURN_DIRECTION
			}
			if (mon->_mgoalvar1++ < 2 * dist /*&& dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/) {
				MonRoundWalk(mnum, currEnemyInfo._meLastDir, &mon->_mgoalvar2); // MOVE_TURN_DIRECTION
			} else {
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		mon->_mgoal = MGOAL_NORMAL;
	}

	if (mon->_mgoal == MGOAL_NORMAL) {
		if (dist > 2 && v < 2 * mon->_mAI.aiInt + 43) {
			if (PosOkMonst(mnum, mon->_mx + offset_x[mon->_mdir], mon->_my + offset_y[mon->_mdir]) && nummonsters < MAXMONSTERS) {
				MonStartRSpAttack(mnum, MIS_HORKDMN);
			}
		} else if (dist < 2) {
			if (v < 2 * mon->_mAI.aiInt + 28) {
				MonStartAttack(mnum);
			}
		} else {
			v = random_(134, 100);
			if (v < 2 * mon->_mAI.aiInt + 33
			 || (MON_JUST_WALKED && v < 2 * mon->_mAI.aiInt + 83)) {
				MonDestWalk(mnum);
			} else {
				MonStartDelay(mnum, RandRange(11, 18) - mon->_mAI.aiInt);
			}
		}
	}
}
#endif

/*
 * AI for monsters using ranged attacks. Uses MIS_FLASH when the target is next to the monster.
 * Attempts to walk in a circle around the target. Retreats on low hp.
 * Uses fade in/out while moving.
 *
 * @param mnum: the id of the monster
 * @param aiParam1: the missile type to be launched at the end of the attack animation when the target is far away.
 */
void MAI_Counselor(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int md, v, dist;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
		MonstCheckDoors(mon->_mx, mon->_my);
	md = currEnemyInfo._meLastDir;
	dist = currEnemyInfo._meRealDist;
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_NORMAL) {
		v = random_(121, 100);
		if (dist >= 2) {
			if (v < 5 * (mon->_mAI.aiInt + 10) && EnemyInLine(mnum)) {
				MonStartRAttack(mnum, mon->_mAI.aiParam1);
			} else if (random_(124, 128) < 39 && mon->_msquelch == SQUELCH_MAX) {
#if DEBUG
				assert((mon->_mAnims[MA_SPECIAL].maFrames - 1) * mon->_mAnims[MA_SPECIAL].maFrameLen * 2 +
					(mon->_mAnims[MA_WALK].maFrames - 1) * mon->_mAnims[MA_WALK].maFrameLen * (6 + 4) < SQUELCH_MAX - SQUELCH_LOW);
#endif
				static_assert((20 - 1) * 1 * 2 + (1 - 1) * 1 * (6 + 4) < SQUELCH_MAX - SQUELCH_LOW, "MAI_Counselor might relax with move goal.");
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 6 + random_low(0, std::min(dist, 4)); // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(125, 2);               // MOVE_TURN_DIRECTION
				MonStartFadeout(mnum, false);
			}
		} else {
			if (mon->_mVar1 == MM_FADEIN) // STAND_PREV_MODE
				v >>= 1;
			if (mon->_mVar1 != MM_FADEIN && mon->_mhitpoints < (mon->_mmaxhp >> 1) && MonFindDir(mnum, OPPOSITE(md)) >= 0) {
#if DEBUG
				assert((mon->_mAnims[MA_SPECIAL].maFrames - 1) * mon->_mAnims[MA_SPECIAL].maFrameLen * 2 +
					(mon->_mAnims[MA_WALK].maFrames - 1) * mon->_mAnims[MA_WALK].maFrameLen * 5 < SQUELCH_MAX - SQUELCH_LOW);
#endif
				static_assert((20 - 1) * 1 * 2 + (1 - 1) * 1 * 5 < SQUELCH_MAX - SQUELCH_LOW, "MAI_Counselor might relax with retreat goal.");
				mon->_mgoal = MGOAL_RETREAT;
				mon->_mgoalvar1 = 5; // RETREAT_DISTANCE
				MonStartFadeout(mnum, false);
			} else if (mon->_mVar1 == MM_DELAY || v < 2 * mon->_mAI.aiInt + 20) {
				MonStartRAttack(mnum, MIS_FLASH);
			}
		}
		if (mon->_mmode == MM_STAND && mon->_mAI.aiType != AI_LAZARUS) {
			v = std::max(1, RandRange(11, 18) - 2 * mon->_mAI.aiInt);
			MonStartDelay(mnum, v);
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		if (--mon->_mgoalvar1 == 0 // RETREAT_DISTANCE
		 || !MonCallWalk(mnum, OPPOSITE(md))) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, true);
		}
	} else {
		assert(mon->_mgoal == MGOAL_MOVE);
		if (dist >= 2 /*&& mon->_msquelch == SQUELCH_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/
		 && (--mon->_mgoalvar1 > 4 || (mon->_mgoalvar1 > 0 && !MonDirOK(mnum, md))) // MOVE_DISTANCE
		 && MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			;
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, true);
		}
	}
}

/*
 * AI for monsters using ranged attacks (MIS_MAGE normally, aiParam1 occasionally).
 * Uses MIS_FLASH when the target is next to the monster.
 * Attempts to walk in a circle around the target. Might move a bit when the enemy is far away.
 * Uses fade in/out while moving.
 *
 * @param mnum: the id of the monster
 * @param aiParam1: the missile type to be launched at the end of the attack animation when the target is far away.
 */
void MAI_Mage(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	int md, v, dist;
	if (MON_RELAXED || MON_ACTIVE)
		return;

	MonEnemyInfo(mnum);
	// if (mon->_msquelch < SQUELCH_MAX && (mon->_mFlags & MFLAG_CAN_OPEN_DOOR))
	//	MonstCheckDoors(mon->_mx, mon->_my);
	md = currEnemyInfo._meLastDir;
	dist = currEnemyInfo._meRealDist;
	mon->_mdir = md;
	if (mon->_mgoal == MGOAL_NORMAL) {
		v = random_(121, 100);
		if (dist >= 2) {
			if (v < 5 * (mon->_mAI.aiInt + 10) && EnemyInLine(mnum)) {
				MonStartRAttack(mnum, v == (5 * (4 + 10) - 1) ? mon->_mAI.aiParam1 : MIS_MAGE /*mon->_mAI.aiParam2*/);
			} else if (mon->_msquelch >= SQUELCH_MAX && random_(124, 128) < 39) {
#if DEBUG
				assert((mon->_mAnims[MA_SPECIAL].maFrames - 1) * mon->_mAnims[MA_SPECIAL].maFrameLen * 2 +
					(mon->_mAnims[MA_WALK].maFrames - 1) * mon->_mAnims[MA_WALK].maFrameLen * (6 + 4) < SQUELCH_MAX - SQUELCH_LOW);
#endif
				static_assert((20 - 1) * 1 * 2 + (1 - 1) * 1 * (6 + 4) < SQUELCH_MAX - SQUELCH_LOW, "MAI_Mage might relax with move goal.");
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar1 = 6 + random_low(0, std::min(dist, 4)); // MOVE_DISTANCE
				mon->_mgoalvar2 = random_(130, 2);               // MOVE_TURN_DIRECTION
				MonStartFadeout(mnum, false);
			} else if (mon->_msquelch < SQUELCH_MAX && mon->_msquelch >= SQUELCH_MAX - 20 && mon->_mVar1 != MM_DELAY) {
				static_assert((20 - 1) * 1 * 2 + (1 - 1) * 1 * 2 < SQUELCH_MAX - 20 - SQUELCH_LOW, "MAI_Mage might relax with retreat goal.");
				mon->_mgoal = MGOAL_RETREAT;
				mon->_mgoalvar1 = mon->_mAI.aiInt + 1; // RETREAT_DISTANCE
				MonStartFadeout(mnum, false);
			}
		} else {
			if (mon->_mVar1 == MM_FADEIN) // STAND_PREV_MODE
				v >>= 1;
			if (mon->_mVar1 == MM_DELAY || v < 2 * mon->_mAI.aiInt + 20) {
				MonStartRAttack(mnum, v < 10 ? MIS_FLASH : MIS_MAGE /*mon->_mAI.aiParam2*/);
			}
		}
		if (mon->_mmode == MM_STAND) {
			v = std::max(1, RandRange(11, 18) - 2 * mon->_mAI.aiInt);
			MonStartDelay(mnum, v);
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		if (--mon->_mgoalvar1 == 0 // RETREAT_DISTANCE
		 || !MonCallWalk(mnum, random_(130, NUM_DIRS))) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, true);
		}
	} else {
		assert(mon->_mgoal == MGOAL_MOVE);
		if (dist >= 2 /*&& mon->_msquelch == SQUELCH_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[mon->_menemyx][mon->_menemyy]*/
		 && (--mon->_mgoalvar1 > 4 || (mon->_mgoalvar1 > 0 && !MonDirOK(mnum, md))) // MOVE_DISTANCE
		 && MonRoundWalk(mnum, md, &mon->_mgoalvar2)) { // MOVE_TURN_DIRECTION
			;
		} else {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadein(mnum, true);
		}
	}
}

void MAI_Garbud(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);
	if (mon->_mgoal == MGOAL_TALKING) {
		if (mon->_mgoalvar1) { // TALK_SPEAKING
			if (dFlags[mon->_mx][mon->_my] & BFLAG_ALERT) { // MON_TIMER
				//if (quests[Q_GARBUD]._qvar1 == QV_GARBUD_ATTACK && mon->_mVar8++ >= gnTicksRate * 6) {
				if (quests[Q_GARBUD]._qvar1 == QV_GARBUD_ATTACK && (IsMultiGame || !IsSfxStreaming(USFX_GARBUD4))) { // minitxtdata[TEXT_GARBUD4].sfxnr
					mon->_mgoal = MGOAL_NORMAL;
					// mon->_msquelch = SQUELCH_MAX;
				}
			} else {
				if (quests[Q_GARBUD]._qvar1 < QV_GARBUD_ATTACK)
					mon->_mgoalvar1 = FALSE; // TALK_INQUIRING
			}
		} else if (quests[Q_GARBUD]._qvar1 == QV_GARBUD_ATTACK) {
			// TODO: does not work when a player enters the level and the timer is running
			mon->_mgoal = MGOAL_NORMAL;
		}
	}

	if (mon->_mgoal != MGOAL_TALKING)
		MAI_Round(mnum);
}

void MAI_Zhar(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);
	if (mon->_mgoal == MGOAL_TALKING) {
		if (quests[Q_ZHAR]._qvar1 < QV_ZHAR_ATTACK) {
			// assert(zharlib != -1);
			int oi = dObject[themes[zharlib]._tsObjX][themes[zharlib]._tsObjY];
			// assert(oi != 0);
			if (oi != 0) {
				ObjectStruct* os = &objects[oi - 1];
				if (os->_oSelFlag == 0) {
					quests[Q_ZHAR]._qvar1 = QV_ZHAR_ATTACK;
					mon->_mgoalvar2 = TEXT_ZHAR2; // TALK_MESSAGE
					mon->_mmode = MM_TALK;
					NetSendCmdQuest(Q_ZHAR, true);
					return;
				}
			}
		}
		//if (quests[Q_ZHAR]._qvar1 == QV_ZHAR_ATTACK && mon->_mVar8++ >= gnTicksRate * 4/*!IsSfxStreaming(USFX_ZHAR2)*/) {
		if (quests[Q_ZHAR]._qvar1 == QV_ZHAR_ATTACK && (IsMultiGame || !IsSfxStreaming(USFX_ZHAR2))) { // minitxtdata[TEXT_ZHAR2].sfxnr
			// mon->_msquelch = SQUELCH_MAX;
			mon->_mgoal = MGOAL_NORMAL;
		}
		if (quests[Q_ZHAR]._qvar1 == QV_ZHAR_TALK1 && mon->_mgoalvar1) { // TALK_SPEAKING
			mon->_mgoalvar1 = FALSE; // TALK_INQUIRING
		}
	}

	if (mon->_mgoal != MGOAL_TALKING)
		MAI_Counselor(mnum);
}

void MAI_SnotSpil(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);

	switch (quests[Q_BANNER]._qvar1) {
	case QV_INIT: // quest not started -> skip
		return;
	case QV_BANNER_TALK1:
		// switch to new text if the player(s) left
		// assert(mon->_mgoal == MGOAL_TALKING);
		if (mon->_mgoalvar2 == TEXT_BANNER10 && !(dFlags[mon->_mx][mon->_my] & BFLAG_ALERT))
			mon->_mgoalvar2 = TEXT_BANNER11; // TALK_MESSAGE
		// if (mon->_mgoalvar1)
			mon->_mgoalvar1 = FALSE; // TALK_INQUIRING
		return;
	case QV_BANNER_GIVEN:
		// assert(mon->_mgoal == MGOAL_TALKING);
		// if (mon->_mgoalvar1)
			mon->_mgoalvar1 = FALSE; // TALK_INQUIRING
		return;
	case QV_BANNER_TALK2:
		//if (mon->_mVar8++ < gnTicksRate * 6) // MON_TIMER
		//	return; // wait till the sfx is running, but don't rely on IsSfxStreaming
		if (IsMultiGame || IsSfxStreaming(USFX_SNOT3)) // minitxtdata[TEXT_BANNER12].sfxnr
			return;
		//if (mon->_mListener == mypnum || !plx(mon->_mListener)._pActive || plx(mon->_mListener)._pDunLevel != currLvl._dLevelIdx) {
			NetSendCmd(CMD_OPENSPIL);
		//}
		return;
	case QV_BANNER_ATTACK:
		if (mon->_mgoal == MGOAL_TALKING) {
			// TODO: does not work when a player enters the level and the timer is running
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

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);

	if (IsMultiGame) {
		if (mon->_mgoal == MGOAL_TALKING) {
			if (!mon->_mgoalvar1) { // TALK_INQUIRING
				if (quests[Q_BETRAYER]._qvar1 < QV_BETRAYER_TALK1) {
					assert(mon->_mgoalvar2 == TEXT_VILE13); // TALK_MESSAGE
					if (mon->_msquelch != SQUELCH_MAX)
						return;
					mon->_mmode = MM_TALK;
					// mon->_mListener = mypnum;
					quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_TALK1;
					NetSendCmdQuest(Q_BETRAYER, true);
				} else {
					mon->_mgoal = MGOAL_NORMAL;
				}
			} else { // TALK_SPEAKING
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	} else {
		if (mon->_mgoal == MGOAL_TALKING) {
			if (!mon->_mgoalvar1) { // TALK_INQUIRING
				if (mon->_msquelch != SQUELCH_MAX)
					return;
				// assert(quests[Q_BETRAYER]._qvar1 < QV_BETRAYER_TALK1);
				// assert(myplr._px == LAZ_CIRCLE_X && myplr._py == LAZ_CIRCLE_Y);
				PlayInGameMovie("gendata\\fprst3.smk");
				mon->_mmode = MM_TALK;
				// mon->_mListener = mypnum;
			} else { // TALK_SPEAKING  minitxtdata[TEXT_VILE13].sfxnr
				if (IsSfxStreaming(USFX_LAZ1) && myplr._pmode == PM_STAND) // myplr._px == LAZ_CIRCLE_X && myplr._py == LAZ_CIRCLE_Y)
					return;
				DRLG_ChangeMap(7, 20, 11, 22/*, false*/);
				//RedoLightAndVision();
				// mon->_msquelch = SQUELCH_MAX;
				mon->_mgoal = MGOAL_NORMAL;
				quests[Q_BETRAYER]._qvar1 = QV_BETRAYER_TALK1;
			}
		}
	}

	MAI_Counselor(mnum);
}

void MAI_Lazhelp(int mnum)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);

	if (mon->_mgoal == MGOAL_TALKING) {
		if (!IsMultiGame && quests[Q_BETRAYER]._qvar1 < QV_BETRAYER_TALK1)
			return;
		mon->_mgoal = MGOAL_NORMAL;
	}

	MAI_Ranged(mnum);
}

void MAI_Lachdanan(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];
	if (MON_RELAXED || MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);

	if (quests[Q_VEIL]._qactive == QUEST_DONE) { // MON_TIMER
		//if (mon->_mVar8++ >= gnTicksRate * 32) {
		if (IsMultiGame || !IsSfxStreaming(USFX_LACH3)) { // minitxtdata[TEXT_VEIL11].sfxnr
			// mon->_mgoal = MGOAL_NORMAL;
			MonKill(mnum, -1);
		}
		return;
	}
	if (quests[Q_VEIL]._qactive == QUEST_ACTIVE) {
		if (mon->_mgoalvar2 == TEXT_VEIL9 && !(dFlags[mon->_mx][mon->_my] & BFLAG_ALERT))
			mon->_mgoalvar2 = TEXT_VEIL10; // TALK_MESSAGE
	}
	// assert(mon->_mgoal == MGOAL_TALKING);
	// if (mon->_mgoalvar1)
		mon->_mgoalvar1 = FALSE;
}

void MAI_Warlord(int mnum)
{
	MonsterStruct* mon;
	int pnum;

	mon = &monsters[mnum];
	if (MON_ACTIVE)
		return;

	mon->_mdir = MonEnemyLastDir(mnum);

	switch (quests[Q_WARLORD]._qvar1) {
	case QV_INIT: // quest not started
	case QV_WARLORD_BOOK:
		if (!MON_HAS_ENEMY)
			return;
		quests[Q_WARLORD]._qvar1 = IsMultiGame ? QV_WARLORD_ATTACK : QV_WARLORD_TALK;
		pnum = mon->_menemy >= 0 ? mon->_menemy : -(mon->_menemy + 1);
		// assert(plr._pActive && plr._pDunLevel == currLvl._dLevelIdx && !plr._pLvlChanging);
		if (pnum == mypnum) {
			NetSendCmdQuest(Q_WARLORD, true);
		}
		mon->_mmode = MM_TALK;
		//mon->_mListener = mon->_menemy;
		//mon->_mVar8 = 0; // MON_TIMER
		return;
	case QV_WARLORD_TALK:
		//if (mon->_mVar8++ < gnTicksRate * 8) // MON_TIMER
		//	return; // wait till the sfx is running, but don't rely on IsSfxStreaming
		// assert(!IsMultiGame);
		if (/*!IsMultiGame &&*/ IsSfxStreaming(USFX_WARLRD1)) // minitxtdata[TEXT_WARLRD9].sfxnr
			return;
		quests[Q_WARLORD]._qvar1 = QV_WARLORD_ATTACK;
		//if (mon->_mListener == mypnum || !plx(mon->_mListener)._pActive || plx(mon->_mListener)._pDunLevel != currLvl._dLevelIdx) {
		//	NetSendCmdQuest(Q_WARLORD, true);
		//}
		// mon->_msquelch = SQUELCH_MAX;
	case QV_WARLORD_ATTACK:
		if (mon->_mgoal == MGOAL_TALKING) {
			// TODO: does not work when a player enters the level and the timer is running
			mon->_mgoal = MGOAL_NORMAL;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	// assert(mon->_mgoal == MGOAL_NORMAL);
		MAI_SkelSd(mnum);
}

void ProcessMonsters()
{
	int mnum;
	bool alert, raflag;
	MonsterStruct* mon;

	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		// gbGameLogicMnum = mnum;
		mon = &monsters[mnum];
		if (mon->_mmode > MM_INGAME_LAST)
			continue;
		if (IsMultiGame) {
			SetRndSeed(mon->_mAISeed);
			mon->_mAISeed = NextRndSeed();
		}
		if (mon->_mhitpoints < mon->_mmaxhp && mon->_mhitpoints != 0 /*&& !(mon->_mFlags & MFLAG_NOHEAL)*/) {
			mon->_mhitpoints += (mon->_mLevel + 1) >> 1;
			if (mon->_mhitpoints > mon->_mmaxhp)
				mon->_mhitpoints = mon->_mmaxhp;
		}

		alert = (dFlags[mon->_mfutx][mon->_mfuty] & BFLAG_ALERT) != 0;
		if ((alert || MON_HAS_ENEMY) && (!MON_ACTIVE || (mon->_mFlags & MFLAG_GARG_STONE))) {
			MonFindEnemy(mnum);
			// commented out, because the player might went out of sight in the meantime
			// assert(MON_HAS_ENEMY || myplr._pInvincible);
		}
		if (alert) {
			if (MON_HAS_ENEMY) {
				if (mon->_msquelch == 0) {
					if (mon->_mType == MT_CLEAVER)
						PlaySfxLoc(USFX_CLEAVER, mon->_mfutx, mon->_mfuty);
#ifdef HELLFIRE
					else if (mon->_mType == MT_NAKRUL)
						PlaySfxLoc(quests[Q_JERSEY]._qactive != QUEST_NOTAVAIL ? USFX_NAKRUL6 : (quests[Q_NAKRUL]._qvar1 == QV_NAKRUL_BOOKOPEN ? USFX_NAKRUL4 : USFX_NAKRUL5), mon->_mfutx, mon->_mfuty);
					else if (mon->_mType == MT_DEFILER)
						PlaySfxLoc(USFX_DEFILER8, mon->_mfutx, mon->_mfuty);
#endif
				}
				mon->_msquelch = SQUELCH_MAX;
			}
		} else if (mon->_msquelch != 0 && mon->_mhitpoints == mon->_mmaxhp && mon->_mleaderflag != MLEADER_AWAY) {
			mon->_msquelch--;
			if (mon->_msquelch == 0) {
				// reset monster state to ensure sync in multiplayer games
				if (mon->_mmode == MM_DELAY)
					mon->_mmode = MM_STAND;
				if (mapMonTypes[mon->_mMTidx].cmFlags & MFLAG_GARG_STONE) {
					mon->_mFlags |= MFLAG_GARG_STONE;
					assert(mon->_mmode == MM_STAND || mon->_mmode == MM_SPATTACK);
				} else if (mapMonTypes[mon->_mMTidx].cmFlags & MFLAG_HIDDEN) {
					mon->_mFlags |= MFLAG_HIDDEN;
					assert(mon->_mmode == MM_STAND);
				} else {
					mon->_mFlags &= ~(MFLAG_HIDDEN | MFLAG_GARG_STONE);
					assert(mon->_mmode == MM_STAND);
				}
				mon->_menemy = 0;
				mon->_menemyx = 0;
				mon->_menemyy = 0;
				mon->_mlastx = 0;
				mon->_mlasty = 0;
				mon->_mVar1 = MM_STAND;           // STAND_PREV_MODE
				mon->_mVar2 = MON_WALK_DELAY + 1; // STAND_TICK
				assert(mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_TALKING);
			}
		}

		while (true) {
			AiProc[mon->_mAI.aiType](mnum);

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
	// gbGameLogicMnum = 0;
}

void FreeMonsters()
{
	int i, j;

	for (i = 0; i < nummtypes; i++) {
		for (j = 0; j < NUM_MON_ANIM; j++) {
			MemFreeDbg(mapMonTypes[i].cmAnimData[j]);
		}
	}

	FreeMonMissileGFX();
	FreeMonsterSFX();
}

void SyncMonsterAnim(int mnum)
{
	MonsterStruct* mon;
	MapMonData* mmdata;
	int mode;
	MON_ANIM anim;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("SyncMonsterAnim: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if ((unsigned)mon->_mMTidx >= MAX_LVLMTYPES) {
		dev_fatal("SyncMonsterAnim: Invalid monster type %d for %d", mon->_mMTidx, mnum);
	}
	mmdata = &mapMonTypes[mon->_mMTidx];
	mon->_mType = mmdata->cmType;
	mon->_mAnims = mmdata->cmAnims;
	mon->_mAnimWidth = mmdata->cmWidth;
	mon->_mAnimXOffset = mmdata->cmXOffset;
	mon->_mAFNum = mmdata->cmAFNum;
	mon->_mAFNum2 = mmdata->cmAFNum2;
	if (mon->_muniqtype != 0)
		mon->_mName = uniqMonData[mon->_muniqtype - 1].mName;
	else
		mon->_mName = monsterdata[mon->_mType].mName;

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
	case MM_DEAD:
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
	case MM_UNUSED:
	case MM_RESERVED:
		anim = MA_STAND;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	mon->_mAnimData = mon->_mAnims[anim].maAnimData[mon->_mdir];
	mon->_mAnimFrameLen = mon->_mAnims[anim].maFrameLen;
	mon->_mAnimLen = mon->_mAnims[anim].maFrames;
}

void MonHinder(int mnum, int spllvl, unsigned tick)
{
	MonsterStruct* mon;
	int effect;
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MonHinder: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if ((mon->_mmode < MM_WALK || mon->_mmode > MM_WALK2) && mon->_mmode != MM_CHARGE)
		return;

	effect = spllvl * 6 - mon->_mLevel;
	effect = effect >= 6 ? 2 : (effect > 0 ? 3 : (effect >= -6 ? 4 : 0));
	if (effect != 0 && ((unsigned)tick % (unsigned)effect) == 0) {
		if (mon->_mmode != MM_CHARGE) {
			mon->_mAnimCnt--;
			mon->_mVar6 -= mon->_mVar4; // MWALK_XOFF <- WALK_XVEL
			mon->_mVar7 -= mon->_mVar5; // MWALK_YOFF <- WALK_YVEL
		} else {
			// assert(dMonster[mon->_mx][mon->_my] == -(mnum + 1));
			dMonster[mon->_mx][mon->_my] = mnum + 1;
			// assert(dPlayer[mon->_mx][mon->_my] == 0);
			// assert(!(mon->_mFlags & MFLAG_HIDDEN));
			//ChangeLightXYOff(mon->_mlid, mon->_mx, mon->_my);
			MonStartStand(mnum);
		}
	}
}

void MissToMonst(int mi)
{
	MissileStruct* mis;
	MonsterStruct* mon;
	int mnum, oldx, oldy, mpnum, pnum, defm;

	if ((unsigned)mi >= MAXMISSILES) {
		dev_fatal("MissToMonst: Invalid missile %d", mi);
	}
	mis = &missile[mi];
	mnum = mis->_miSource;
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("MissToMonst: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	// assert(dMonster[mon->_mx][mon->_my] == 0);
	dMonster[mon->_mx][mon->_my] = mnum + 1;
	// assert(dPlayer[mon->_mx][mon->_my] == 0);
	// assert(!(mon->_mFlags & MFLAG_HIDDEN));
	//ChangeLightXYOff(mon->_mlid, mon->_mx, mon->_my);
	MonStartStand(mnum);
	/*if (mon->_mType >= MT_INCIN && mon->_mType <= MT_HELLBURN) {
		MonStartFadein(mnum, false);
		return;
	}*/
	PlayMonSfx(mnum, MS_GOTHIT);
	if (mon->_mType == MT_GBAT) /* mon->_mAI.aiType == AI_BAT Foulwing? */
		return;

	oldx = mis->_mix;
	oldy = mis->_miy;
	mpnum = dPlayer[oldx][oldy];
	if (mpnum != 0) {
		pnum = CheckPlrCol(mpnum);
		if (pnum < 0)
			return;
		if (PlrCheckBlock(pnum, mon->_mLevel + 16, OPPOSITE(mon->_mdir)))
			return;
		// TODO: prevent bleeding if MonsterAI is AI_RHINO ?
		MonHitPlr(mnum, pnum, mon->_mHit * 8, mon->_mMinDamage2, mon->_mMaxDamage2);
		if (mpnum == dPlayer[oldx][oldy] && mon->_mAI.aiType == AI_RHINO) { /* mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE */
			PlrHitByAny(pnum, mnum, 0, ISPL_KNOCKBACK, mon->_mdir);
		}
		return;
	}
	mpnum = dMonster[oldx][oldy];
	if (mpnum != 0) {
		defm = CheckMonCol(mpnum);
		if (defm < 0 || defm >= MAX_MINIONS)
			return; // do not hit team-mate : assert(mnum >= MAX_MINIONS);
		MonHitMon(mnum, defm, mon->_mHit * 8, mon->_mMinDamage2, mon->_mMaxDamage2);
		if (mpnum == dMonster[oldx][oldy] && mon->_mAI.aiType == AI_RHINO) { /* mon->_mType < MT_NSNAKE || mon->_mType > MT_GSNAKE */
			// TODO: use MonHitByMon ?
			PlayMonSfx(mnum, MS_GOTHIT);
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

void SpawnSkeleton(int mnum, int x, int y, int dir)
{
	int i, nok;
	int monstok[NUM_DIRS];

	if (mnum == -1)
		return; // FALSE;

	if (PosOkActor(x, y)) {
		if (dir == DIR_NONE)
			dir = random_(11, NUM_DIRS);
		ActivateSpawn(mnum, x, y, dir);
		return; // TRUE;
	}

	nok = 0;
	for (i = 0; i < lengthof(offset_x); i++) {
		if (PosOkActor(x + offset_x[i], y + offset_y[i])) {
			monstok[nok] = i;
			nok++;
		}
	}
	if (nok == 0)
		return;
	dir = monstok[random_low(12, nok)];
	ActivateSpawn(mnum, x + offset_x[dir], y + offset_y[dir], dir);

	// return TRUE;
}

int PreSpawnSkeleton()
{
	int n = numSkelTypes, mnum = -1;

	if (n != 0 && nummonsters < MAXMONSTERS) {
		mnum = nummonsters;
		nummonsters++;
		n = mapSkelTypes[random_low(136, n)];
		InitMonster(mnum, 0, n, 0, 0);
		monsters[mnum]._mmode = MM_RESERVED;
	}
	return mnum;
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
	if (mon->_mgoal != MGOAL_TALKING || mon->_mgoalvar1) // TALK_SPEAKING / TALK_INQUIRING
		return; // already talking (or does not want to talk at all)
	mon->_mmode = MM_TALK;
	// mon->_mListener = pnum;
	if (mon->_mAI.aiType == AI_SNOTSPIL) {
		assert(QuestStatus(Q_BANNER));
		if (quests[Q_BANNER]._qvar1 == QV_INIT) {
			assert(mon->_mgoalvar2 == TEXT_BANNER10); // TALK_MESSAGE
			quests[Q_BANNER]._qvar1 = QV_BANNER_TALK1;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_BANNER, true);
		} else if (quests[Q_BANNER]._qvar1 == QV_BANNER_TALK1) {
			if (PlrHasStorageItem(pnum, IDI_BANNER, &iv)) {
				mon->_mgoalvar2 = TEXT_BANNER12; // TALK_MESSAGE
				NetSendCmdParam1(CMD_QMONSTER, IDI_BANNER);
			}
		} else if (quests[Q_BANNER]._qvar1 == QV_BANNER_GIVEN) {
			mon->_mgoalvar2 = TEXT_BANNER12; // TALK_MESSAGE
		}
		if (mon->_mgoalvar2 == TEXT_BANNER12) { // TALK_MESSAGE
			// mon->_mVar8 = 0; // init MON_TIMER
			quests[Q_BANNER]._qvar1 = QV_BANNER_TALK2;
			if (IsMultiGame && pnum == mypnum) {
				// NetSendCmdQuest(Q_BANNER, true);
				NetSendCmd(CMD_OPENSPIL);
			}
		}
	} else if (mon->_mAI.aiType == AI_GARBUD) {
		mon->_mgoalvar2 = TEXT_GARBUD1 + quests[Q_GARBUD]._qvar1; // TALK_MESSAGE (TEXT_GARBUD1, TEXT_GARBUD2, TEXT_GARBUD3, TEXT_GARBUD4)
		if (mon->_mgoalvar2 == TEXT_GARBUD1) { // TALK_MESSAGE
			quests[Q_GARBUD]._qactive = QUEST_ACTIVE;
			quests[Q_GARBUD]._qlog = TRUE; // BUGFIX: (?) for other quests qactive and qlog go together, maybe this should actually go into the if above (fixed)
		} else if (mon->_mgoalvar2 == TEXT_GARBUD2) { // TALK_MESSAGE
			SetRndSeed(mon->_mRndSeed);
			SpawnMonItem(mnum, plr._px, plr._py, pnum == mypnum);
		} //else if (mon->_mgoalvar2 == TEXT_GARBUD4) // TALK_MESSAGE
		//	mon->_mVar8 = 0; // init MON_TIMER
		quests[Q_GARBUD]._qvar1++; // (QV_GARBUD_TALK1, QV_GARBUD_TALK2, QV_GARBUD_TALK3, QV_GARBUD_ATTACK)
		if (quests[Q_GARBUD]._qvar1 > QV_GARBUD_ATTACK)
			quests[Q_GARBUD]._qvar1 = QV_GARBUD_ATTACK;
		if (pnum == mypnum)
			NetSendCmdQuest(Q_GARBUD, true);
	} else if (mon->_mAI.aiType == AI_LACHDAN) {
		assert(QuestStatus(Q_VEIL));
		assert(mon->_mgoalvar2 != TEXT_NONE); // TALK_MESSAGE
		if (quests[Q_VEIL]._qactive == QUEST_INIT) {
			quests[Q_VEIL]._qactive = QUEST_ACTIVE;
			quests[Q_VEIL]._qlog = TRUE;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_VEIL, true);
		} else if (quests[Q_VEIL]._qactive == QUEST_ACTIVE && PlrHasStorageItem(pnum, IDI_GLDNELIX, &iv)) {
			mon->_mgoalvar2 = TEXT_VEIL11; // TALK_MESSAGE
			NetSendCmdParam1(CMD_QMONSTER, IDI_GLDNELIX);
		}
	} else if (mon->_mAI.aiType == AI_ZHAR) {
		if (quests[Q_ZHAR]._qactive == QUEST_INIT) {
			quests[Q_ZHAR]._qactive = QUEST_ACTIVE;
			quests[Q_ZHAR]._qvar1 = QV_ZHAR_TALK1;
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
		} else if (quests[Q_ZHAR]._qvar1 == QV_ZHAR_TALK1) {
			mon->_mgoalvar2 = TEXT_ZHAR2; // TALK_MESSAGE
			//mon->_mVar8 = 0; // init MON_TIMER
			quests[Q_ZHAR]._qvar1 = QV_ZHAR_ATTACK;
			if (pnum == mypnum)
				NetSendCmdQuest(Q_ZHAR, true);
		}
	}
}

void InitGolemStats(int mnum, int level)
{
	MonsterStruct* mon;

	mon = &monsters[mnum];
	int lvlBonus = level > 0 ? level - 1 : 0;

	// mon->_mAI.aiInt = monsterdata[MT_GOLEM].mAI.aiInt + lvlBonus / 16;
	mon->_mHit = monsterdata[MT_GOLEM].mHit + lvlBonus * 5 / 2;
	// mon->_mHit2 = monsterdata[MT_GOLEM].mHit2 + lvlBonus * 5 / 2;
	// mon->_mMagic = monsterdata[MT_GOLEM].mMagic + lvlBonus * 5 / 2;
	mon->_mEvasion = monsterdata[MT_GOLEM].mEvasion + lvlBonus * 5 / 2;
	mon->_mArmorClass = monsterdata[MT_GOLEM].mArmorClass + lvlBonus * 5 / 2;

	int baseLvl = monsterdata[MT_GOLEM].mLevel;
	int monLvl = baseLvl + lvlBonus;
	mon->_mLevel = monLvl;
	mon->_mmaxhp = (monLvl * monsterdata[MT_GOLEM].mMinHP / baseLvl) << 6;
	mon->_mExp = 0; // monLvl * mon->_mExp / baseLvl;
	mon->_mMinDamage = monLvl * monsterdata[MT_GOLEM].mMinDamage / baseLvl;
	mon->_mMaxDamage = monLvl * monsterdata[MT_GOLEM].mMaxDamage / baseLvl;
	// mon->_mMinDamage2 = monLvl * monsterdata[MT_GOLEM].mMinDamage2 / baseLvl;
	// mon->_mMaxDamage2 = monLvl * monsterdata[MT_GOLEM].mMaxDamage2 / baseLvl;
}

void SpawnGolem(int mnum, int x, int y, int level)
{
	MonsterStruct* mon;

	static_assert(MAX_MINIONS == MAX_PLRS, "SpawnGolem requires that owner of a monster has the same id as the monster itself.");
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("SpawnGolem: Invalid monster %d", mnum);
	}
	InitMonster(mnum, DIR_S, 0, 0, 0); // reset goal, enemy (+last)
	level = level * 4 + (plx(mnum)._pMagic >> 6);
	InitGolemStats(mnum, level);
	mon = &monsters[mnum];
	mon->_mhitpoints = mon->_mmaxhp;
	mon->_mvid = AddVision(x, y, PLR_MIN_VISRAD, false);
	// assert((mon->_mFlags & (MFLAG_NOCORPSE | MFLAG_NODROP)) == (MFLAG_NOCORPSE | MFLAG_NODROP));
	ActivateSpawn(mnum, x, y, DIR_S);
	PlaySfxLoc(LS_GOLUM, x, y);
	if (mnum == mypnum)
		NetSendCmdGolem(x, y, level);
}

bool CanTalkToMonst(int mnum)
{
	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("CanTalkToMonst: Invalid monster %d", mnum);
	}
	assert(monsters[mnum]._mgoal != MGOAL_TALKING || monsters[mnum]._mgoalvar2 != TEXT_NONE); // TALK_MESSAGE
	return monsters[mnum]._mgoal == MGOAL_TALKING;
}

bool CheckMonsterHit(int mnum, bool* ret)
{
	MonsterStruct* mon;
	bool result = true;

	if ((unsigned)mnum >= MAXMONSTERS) {
		dev_fatal("CheckMonsterHit: Invalid monster %d", mnum);
	}
	mon = &monsters[mnum];
	if (mon->_mgoal == MGOAL_TALKING // CanTalkToMonst(mnum)
	 || mon->_mmode == MM_CHARGE || mon->_mmode == MM_DEATH || mon->_mmode == MM_FADEIN || mon->_mmode == MM_FADEOUT
	 || (mon->_mAI.aiType == AI_SNEAK && mon->_mgoal == MGOAL_RETREAT && mon->_mmode != MM_STONE)) {
		*ret = false;
		result = false;
	} else if (mon->_mAI.aiType == AI_GARG && mon->_mFlags & MFLAG_GARG_STONE) {
		mon->_mFlags &= ~(MFLAG_GARG_STONE | MFLAG_LOCK_ANIMATION);
		*ret = true;
		result = false;
	}

	return result;
}

DEVILUTION_END_NAMESPACE
