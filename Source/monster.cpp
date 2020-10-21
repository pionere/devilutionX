/**
 * @file monster.cpp
 *
 * Implementation of monster functionality, AI, actions, spawning, loading, etc.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Tracks which missile files are already loaded */
int MissileFileFlag;

// BUGFIX: replace monstkills[MAXMONSTERS] with monstkills[NUM_MTYPES].
/** Tracks the total number of monsters killed per monster_id. */
int monstkills[MAXMONSTERS];
int monstactive[MAXMONSTERS];
int nummonsters;
BOOLEAN sgbSaveSoundOn;
MonsterStruct monster[MAXMONSTERS];
int totalmonsters;
CMonster Monsters[MAX_LVLMTYPES];
#ifdef HELLFIRE
int GraphicTable[NUMLEVELS][MAX_LVLMTYPES];
#else
BYTE GraphicTable[NUMLEVELS][MAX_LVLMTYPES];
#endif
int monstimgtot;
int uniquetrans;
int nummtypes;

#ifdef HELLFIRE
int HorkXAdd[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
int HorkYAdd[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };
#endif
/** Maps from walking path step to facing direction. */
const char plr2monst[9] = { 0, 5, 3, 7, 1, 4, 6, 0, 2 };
/** Maps from monster intelligence factor to missile type. */
const BYTE counsmiss[4] = { MIS_FIREBOLT, MIS_CBOLT, MIS_LIGHTCTRL, MIS_FIREBALL };

/* data */

/** Maps from monster walk animation frame num to monster velocity. */
int MWVel[24][3] = {
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
char animletter[7] = "nwahds";
/** Maps from direction to a left turn from the direction. */
int left[8] = { 7, 0, 1, 2, 3, 4, 5, 6 };
/** Maps from direction to a right turn from the direction. */
int right[8] = { 1, 2, 3, 4, 5, 6, 7, 0 };
/** Maps from direction to the opposite direction. */
int opposite[8] = { 4, 5, 6, 7, 0, 1, 2, 3 };
/** Maps from direction to delta X-offset. */
int offset_x[8] = { 1, 0, -1, -1, -1, 0, 1, 1 };
/** Maps from direction to delta Y-offset. */
int offset_y[8] = { 1, 1, 1, 0, -1, -1, -1, 0 };

/** unused */
int rnd5[4] = { 5, 10, 15, 20 };
int rnd10[4] = { 10, 15, 20, 30 };
int rnd20[4] = { 20, 30, 40, 50 };
int rnd60[4] = { 60, 70, 80, 90 };

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
	&mai_ranged_441680,
	&mai_ranged_44168B,
	&mai_horkdemon,
	&mai_ranged_441649,
	&mai_ranged_441654,
	&mai_ranged_44165F,
	&mai_ranged_44166A,
	&mai_roundranged_441EA0
#endif
};

void InitMonsterTRN(int midx, BOOL special)
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
			for (j = 0; j < 8; j++) {
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

	nummtypes = 0;
	monstimgtot = 0;
	MissileFileFlag = 0;

	for (i = 0; i < MAX_LVLMTYPES; i++) {
		Monsters[i].mPlaceFlags = 0;
	}

	ClrAllMonsters();
	nummonsters = 0;
	totalmonsters = MAXMONSTERS;

	for (i = 0; i < MAXMONSTERS; i++) {
		monstactive[i] = i;
	}

	uniquetrans = 0;
}

int AddMonsterType(int type, int placeflag)
{
	BOOL done = FALSE;
	int i;

	for (i = 0; i < nummtypes && !done; i++) {
		done = Monsters[i].mtype == type;
	}

	i--;

	if (!done) {
		i = nummtypes;
		nummtypes++;
		Monsters[i].mtype = type;
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

	int minl; // min level
	int maxl; // max level
	char mamask;
	const int numskeltypes = 19;

	int nt; // number of types

#ifdef SPAWN
	mamask = 1; // monster availability mask
#else
	mamask = 3; // monster availability mask
#endif

	AddMonsterType(MT_GOLEM, 2);
	if (currlevel == 16) {
		AddMonsterType(MT_ADVOCATE, 1);
		AddMonsterType(MT_RBLACK, 1);
		AddMonsterType(MT_DIABLO, 2);
		return;
	}

#ifdef HELLFIRE
	if (currlevel == 18)
		AddMonsterType(117, 1);
	if (currlevel == 19) {
		AddMonsterType(117, 1);
		AddMonsterType(123, 4);
	}
	if (currlevel == 20)
		AddMonsterType(124, 4);
	if (currlevel == 24) {
		AddMonsterType(133, 1);
		AddMonsterType(137, 2);
	}
#endif

	if (!setlevel) {
		if (QuestStatus(Q_BUTCHER))
			AddMonsterType(MT_CLEAVER, 2);
		if (QuestStatus(Q_GARBUD))
			AddMonsterType(UniqMonst[UMT_GARBUD].mtype, 4);
		if (QuestStatus(Q_ZHAR))
			AddMonsterType(UniqMonst[UMT_ZHAR].mtype, 4);
		if (QuestStatus(Q_LTBANNER))
			AddMonsterType(UniqMonst[UMT_SNOTSPIL].mtype, 4);
		if (QuestStatus(Q_VEIL))
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, 4);
		if (QuestStatus(Q_WARLORD))
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, 4);

		if (gbMaxPlayers != 1 && currlevel == quests[Q_SKELKING]._qlevel) {

			AddMonsterType(MT_SKING, 4);

			nt = 0;
			for (i = MT_WSKELAX; i <= MT_WSKELAX + numskeltypes; i++) {
				if (IsSkel(i)) {
					minl = 15 * monsterdata[i].mMinDLvl / 30 + 1;
					maxl = 15 * monsterdata[i].mMaxDLvl / 30 + 1;

					if (currlevel >= minl && currlevel <= maxl) {
						if (MonstAvailTbl[i] & mamask) {
							skeltypes[nt++] = i;
						}
					}
				}
			}
			AddMonsterType(skeltypes[random_(88, nt)], 1);
		}

		nt = 0;
		for (i = 0; i < NUM_MTYPES; i++) {
			minl = 15 * monsterdata[i].mMinDLvl / 30 + 1;
			maxl = 15 * monsterdata[i].mMaxDLvl / 30 + 1;

			if (currlevel >= minl && currlevel <= maxl) {
				if (MonstAvailTbl[i] & mamask) {
					typelist[nt++] = i;
				}
			}
		}

		if (monstdebug) {
			for (i = 0; i < debugmonsttypes; i++)
				AddMonsterType(DebugMonsters[i], 1);
		} else {

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
					AddMonsterType(typelist[i], 1);
					typelist[i] = typelist[--nt];
				}
			}
		}

	} else {
		if (setlvlnum == SL_SKELKING) {
			AddMonsterType(MT_SKING, 4);
		}
	}
}

void InitMonsterGFX(int midx)
{
	CMonster *cmon;
	MonsterData *mdata;
	int mtype, anim, i;
	char strBuff[256];
	BYTE *celBuf;

	cmon = &Monsters[midx];
	mtype = cmon->mtype;
	mdata = &monsterdata[mtype];
	for (anim = 0; anim < 6; anim++) {
		if ((animletter[anim] != 's' || mdata->has_special) && mdata->Frames[anim] > 0) {
			sprintf(strBuff, mdata->GraphicType, animletter[anim]);

			celBuf = LoadFileInMem(strBuff, NULL);
			cmon->Anims[anim].CMem = celBuf;

			if (cmon->mtype != MT_GOLEM || (animletter[anim] != 's' && animletter[anim] != 'd')) {

				for (i = 0; i < 8; i++) {
					cmon->Anims[anim].Data[i] = CelGetFrameStart(celBuf, i);
				}
			} else {
				for (i = 0; i < 8; i++) {
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
	cmon->MData = &monsterdata[mtype];

	if (mdata->has_trans) {
		cmon->trans_file = LoadFileInMem(mdata->TransFile, NULL);
		InitMonsterTRN(midx, mdata->has_special);
		MemFreeDbg(cmon->trans_file);
	}

	if (mtype >= MT_NMAGMA && mtype <= MT_WMAGMA && !(MissileFileFlag & 1)) {
		MissileFileFlag |= 1;
		LoadMissileGFX(MFILE_MAGBALL);
	}
	if (mtype >= MT_STORM && mtype <= MT_MAEL && !(MissileFileFlag & 2)) {
		MissileFileFlag |= 2;
		LoadMissileGFX(MFILE_THINLGHT);
	}
	if (mtype == MT_SUCCUBUS && !(MissileFileFlag & 4)) {
		MissileFileFlag |= 4;
#ifndef HELLFIRE
		LoadMissileGFX(MFILE_FLARE);
		LoadMissileGFX(MFILE_FLAREEXP);
#endif
	}
#ifdef HELLFIRE
	if (mtype >= MT_INCIN && mtype <= MT_HELLBURN && !(MissileFileFlag & 8)) {
		MissileFileFlag |= 8;
		LoadMissileGFX(MFILE_KRULL);
	}
	if ((mtype >= MT_NACID && mtype <= MT_XACID || mtype == MT_SPIDLORD) && !(MissileFileFlag & 0x10)) {
		MissileFileFlag |= 0x10;
		LoadMissileGFX(MFILE_ACIDBF);
		LoadMissileGFX(MFILE_ACIDSPLA);
		LoadMissileGFX(MFILE_ACIDPUD);
	}
#endif
	if (mtype == MT_SNOWWICH && !(MissileFileFlag & 0x20)) {
		MissileFileFlag |= 0x20;
		LoadMissileGFX(MFILE_SCUBMISB);
		LoadMissileGFX(MFILE_SCBSEXPB);
	}
	if (mtype == MT_HLSPWN && !(MissileFileFlag & 0x40)) {
		MissileFileFlag |= 0x40;
		LoadMissileGFX(MFILE_SCUBMISD);
		LoadMissileGFX(MFILE_SCBSEXPD);
	}
	if (mtype == MT_SOLBRNR && !(MissileFileFlag & 0x80)) {
		MissileFileFlag |= 0x80;
		LoadMissileGFX(MFILE_SCUBMISC);
		LoadMissileGFX(MFILE_SCBSEXPC);
	}
#ifndef HELLFIRE
	if (mtype >= MT_INCIN && mtype <= MT_HELLBURN && !(MissileFileFlag & 8)) {
		MissileFileFlag |= 8;
		LoadMissileGFX(MFILE_KRULL);
	}
	if (mtype >= MT_NACID && mtype <= MT_XACID && !(MissileFileFlag & 0x10)) {
		MissileFileFlag |= 0x10;
		LoadMissileGFX(MFILE_ACIDBF);
		LoadMissileGFX(MFILE_ACIDSPLA);
		LoadMissileGFX(MFILE_ACIDPUD);
	}
#else
	if (mtype == MT_LICH && !(MissileFileFlag & 0x100)) {
		MissileFileFlag |= 0x100u;
		LoadMissileGFX(MFILE_LICH);
		LoadMissileGFX(MFILE_EXORA1);
	}
	if (mtype == MT_ARCHLICH && !(MissileFileFlag & 0x200)) {
		MissileFileFlag |= 0x200u;
		LoadMissileGFX(MFILE_ARCHLICH);
		LoadMissileGFX(MFILE_EXYEL2);
	}
	if ((mtype == MT_PSYCHORB || mtype == MT_BONEDEMN) && !(MissileFileFlag & 0x400)) {
		MissileFileFlag |= 0x400u;
		LoadMissileGFX(MFILE_BONEDEMON);
	}
	if (mtype == MT_NECRMORB && !(MissileFileFlag & 0x800)) {
		MissileFileFlag |= 0x800u;
		LoadMissileGFX(MFILE_NECROMORB);
		LoadMissileGFX(MFILE_EXRED3);
	}
	if (mtype == MT_PSYCHORB && !(MissileFileFlag & 0x1000)) {
		MissileFileFlag |= 0x1000u;
		LoadMissileGFX(MFILE_EXBL2);
	}
	if (mtype == MT_BONEDEMN && !(MissileFileFlag & 0x2000)) {
		MissileFileFlag |= 0x2000u;
		LoadMissileGFX(MFILE_EXBL3);
	}
#endif
	if (mtype == MT_DIABLO) {
		LoadMissileGFX(MFILE_FIREPLAR);
	}
}

void ClearMVars(int mnum)
{
	monster[mnum]._mVar1 = 0;
	monster[mnum]._mVar2 = 0;
	monster[mnum]._mVar3 = 0;
	monster[mnum]._mVar4 = 0;
	monster[mnum]._mVar5 = 0;
	monster[mnum]._mVar6 = 0;
	monster[mnum]._mVar7 = 0;
	monster[mnum]._mVar8 = 0;
}

void InitMonster(int mnum, int rd, int mtype, int x, int y)
{
	CMonster *cmon = &Monsters[mtype];
	MonsterStruct *mon = &monster[mnum];

	mon->_mdir = rd;
	mon->_mx = x;
	mon->_my = y;
	mon->_mfutx = x;
	mon->_mfuty = y;
	mon->_moldx = x;
	mon->_moldy = y;
	mon->_mMTidx = mtype;
	mon->_mmode = MM_STAND;
	mon->mName = cmon->MData->mName;
	mon->MType = cmon;
	mon->MData = cmon->MData;
	mon->_mAnimData = cmon->Anims[MA_STAND].Data[rd];
	mon->_mAnimDelay = cmon->Anims[MA_STAND].Rate;
	mon->_mAnimCnt = random_(88, mon->_mAnimDelay - 1);
	mon->_mAnimLen = cmon->Anims[MA_STAND].Frames;
	mon->_mAnimFrame = random_(88, mon->_mAnimLen - 1) + 1;

	if (cmon->mtype == MT_DIABLO) {
#ifdef HELLFIRE
		mon->_mmaxhp = (random_(88, 1) + 3333) << 6;
#else
		mon->_mmaxhp = (random_(88, 1) + 1666) << 6;
#endif
	} else {
		mon->_mmaxhp = (cmon->mMinHP + random_(88, cmon->mMaxHP - cmon->mMinHP + 1)) << 6;
	}

	if (gbMaxPlayers == 1) {
		mon->_mmaxhp >>= 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}

	mon->_mhitpoints = mon->_mmaxhp;
	mon->_mAi = cmon->MData->mAi;
	mon->_mint = cmon->MData->mInt;
	mon->_mgoal = MGOAL_NORMAL;
	mon->_mgoalvar1 = 0;
	mon->_mgoalvar2 = 0;
	mon->_mgoalvar3 = 0;
	mon->field_18 = 0;
	mon->_pathcount = 0;
	mon->_mDelFlag = FALSE;
	mon->_uniqtype = 0;
	mon->_msquelch = 0;
#ifdef HELLFIRE
	mon->mlid = 0;
#endif
	mon->_mRndSeed = GetRndSeed();
	mon->_mAISeed = GetRndSeed();
	mon->mWhoHit = 0;
	mon->mLevel = cmon->MData->mLevel;
	mon->mExp = cmon->MData->mExp;
	mon->mHit = cmon->MData->mHit;
	mon->mMinDamage = cmon->MData->mMinDamage;
	mon->mMaxDamage = cmon->MData->mMaxDamage;
	mon->mHit2 = cmon->MData->mHit2;
	mon->mMinDamage2 = cmon->MData->mMinDamage2;
	mon->mMaxDamage2 = cmon->MData->mMaxDamage2;
	mon->mArmorClass = cmon->MData->mArmorClass;
	mon->mMagicRes = cmon->MData->mMagicRes;
	mon->leader = 0;
	mon->leaderflag = 0;
	mon->_mFlags = cmon->MData->mFlags;
	mon->mtalkmsg = 0;

	if (mon->_mAi == AI_GARG) {
		mon->_mAnimData = cmon->Anims[MA_SPECIAL].Data[rd];
		mon->_mAnimFrame = 1;
		mon->_mFlags |= MFLAG_ALLOW_SPECIAL;
		mon->_mmode = MM_SATTACK;
	}

	if (gnDifficulty == DIFF_NIGHTMARE) {
#ifdef HELLFIRE
		mon->_mmaxhp = 3 * mon->_mmaxhp + ((gbMaxPlayers != 1 ? 100 : 50) << 6);
#else
		mon->_mmaxhp = 3 * mon->_mmaxhp + 64;
#endif
		mon->_mhitpoints = mon->_mmaxhp;
		mon->mLevel += 15;
		mon->mExp = 2 * (mon->mExp + 1000);
		mon->mHit += NIGHTMARE_TO_HIT_BONUS;
		mon->mMinDamage = 2 * (mon->mMinDamage + 2);
		mon->mMaxDamage = 2 * (mon->mMaxDamage + 2);
		mon->mHit2 += NIGHTMARE_TO_HIT_BONUS;
		mon->mMinDamage2 = 2 * (mon->mMinDamage2 + 2);
		mon->mMaxDamage2 = 2 * (mon->mMaxDamage2 + 2);
		mon->mArmorClass += NIGHTMARE_AC_BONUS;
	}

#ifdef HELLFIRE
	else
#endif
	    if (gnDifficulty == DIFF_HELL) {
#ifdef HELLFIRE
		mon->_mmaxhp = 4 * mon->_mmaxhp + ((gbMaxPlayers != 1 ? 200 : 100) << 6);
#else
		mon->_mmaxhp = 4 * mon->_mmaxhp + 192;
#endif
		mon->_mhitpoints = mon->_mmaxhp;
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
}

void ClrAllMonsters()
{
	int i;
	MonsterStruct *mon;

	for (i = 0; i < MAXMONSTERS; i++) {
		mon = &monster[i];
		ClearMVars(i);
		mon->mName = "Invalid Monster";
		mon->_mgoal = 0;
		mon->_mmode = MM_STAND;
		mon->_mVar1 = 0;
		mon->_mVar2 = 0;
		mon->_mx = 0;
		mon->_my = 0;
		mon->_mfutx = 0;
		mon->_mfuty = 0;
		mon->_moldx = 0;
		mon->_moldy = 0;
		mon->_mdir = random_(89, 8);
		mon->_mxvel = 0;
		mon->_myvel = 0;
		mon->_mAnimData = NULL;
		mon->_mAnimDelay = 0;
		mon->_mAnimCnt = 0;
		mon->_mAnimLen = 0;
		mon->_mAnimFrame = 0;
		mon->_mFlags = 0;
		mon->_mDelFlag = FALSE;
		mon->_menemy = random_(89, gbActivePlayers);
		mon->_menemyx = plr[mon->_menemy]._pfutx;
		mon->_menemyy = plr[mon->_menemy]._pfuty;
	}
}

BOOL MonstPlace(int xp, int yp)
{
	char f;

	if (xp < 0 || xp >= MAXDUNX
	    || yp < 0 || yp >= MAXDUNY
	    || dMonster[xp][yp]
	    || dPlayer[xp][yp]) {
		return FALSE;
	}

	f = dFlags[xp][yp];

	if (f & BFLAG_VISIBLE) {
		return FALSE;
	}

	if (f & BFLAG_POPULATED) {
		return FALSE;
	}

	return !SolidLoc(xp, yp);
}

void monster_some_crypt()
{
	MonsterStruct *mon;
	int hp;

	if (currlevel == 24 && UberDiabloMonsterIndex >= 0 && UberDiabloMonsterIndex < nummonsters) {
		mon = &monster[UberDiabloMonsterIndex];
		PlayEffect(UberDiabloMonsterIndex, 2);
		quests[Q_NAKRUL]._qlog = 0;
		mon->mArmorClass -= 50;
		hp = mon->_mmaxhp / 2;
		mon->mMagicRes = 0;
		mon->_mhitpoints = hp;
		mon->_mmaxhp = hp;
	}
}

void PlaceMonster(int mnum, int mtype, int x, int y)
{
	int rd;

#ifdef HELLFIRE
	if (Monsters[mtype].mtype == MT_NAKRUL) {
		for (int j = 0; j < nummonsters; j++) {
			if (monster[j]._mMTidx == mtype) {
				return;
			}
			if (monster[j].MType->mtype == MT_NAKRUL) {
				return;
			}
		}
	}
#endif
	dMonster[x][y] = mnum + 1;

	rd = random_(90, 8);
	InitMonster(mnum, rd, mtype, x, y);
}

void PlaceUniqueMonst(int uniqindex, int miniontype, int unpackfilesize)
{
	int xp, yp, x, y, i;
	int uniqtype;
	int count2;
	char filestr[64];
	BOOL zharflag, done;
	UniqMonstStruct *uniqm;
	MonsterStruct *mon;
	int count;

	mon = monster + nummonsters;
	count = 0;
	uniqm = UniqMonst + uniqindex;

	if ((uniquetrans + 19) << 8 >= LIGHTSIZE) {
		return;
	}

	for (uniqtype = 0; uniqtype < nummtypes; uniqtype++) {
		if (Monsters[uniqtype].mtype == UniqMonst[uniqindex].mtype) {
			break;
		}
	}

	while (1) {
		xp = random_(91, 80) + 16;
		yp = random_(91, 80) + 16;
		count2 = 0;
		for (x = xp - 3; x < xp + 3; x++) {
			for (y = yp - 3; y < yp + 3; y++) {
				if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX && MonstPlace(x, y)) {
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

	if (uniqindex == UMT_SNOTSPIL) {
		xp = 2 * setpc_x + 24;
		yp = 2 * setpc_y + 28;
	}
	if (uniqindex == UMT_WARLORD) {
		xp = 2 * setpc_x + 22;
		yp = 2 * setpc_y + 23;
	}
	if (uniqindex == UMT_ZHAR) {
		zharflag = TRUE;
		for (i = 0; i < themeCount; i++) {
			if (i == zharlib && zharflag == TRUE) {
				zharflag = FALSE;
				xp = 2 * themeLoc[i].x + 20;
				yp = 2 * themeLoc[i].y + 20;
			}
		}
	}
	if (gbMaxPlayers == 1) {
		if (uniqindex == UMT_LAZURUS) {
			xp = 32;
			yp = 46;
		}
		if (uniqindex == UMT_RED_VEX) {
			xp = 40;
			yp = 45;
		}
		if (uniqindex == UMT_BLACKJADE) {
			xp = 38;
			yp = 49;
		}
		if (uniqindex == UMT_SKELKING) {
			xp = 35;
			yp = 47;
		}
	} else {
		if (uniqindex == UMT_LAZURUS) {
			xp = 2 * setpc_x + 19;
			yp = 2 * setpc_y + 22;
		}
		if (uniqindex == UMT_RED_VEX) {
			xp = 2 * setpc_x + 21;
			yp = 2 * setpc_y + 19;
		}
		if (uniqindex == UMT_BLACKJADE) {
			xp = 2 * setpc_x + 21;
			yp = 2 * setpc_y + 25;
		}
	}
	if (uniqindex == UMT_BUTCHER) {
		done = FALSE;
		for (yp = 0; yp < MAXDUNY && !done; yp++) {
			for (xp = 0; xp < MAXDUNX && !done; xp++) {
				done = dPiece[xp][yp] == 367;
			}
		}
	}

#ifdef HELLFIRE
	if (uniqindex == UMT_NAKRUL) {
		if (UberRow == 0 || UberCol == 0) {
			UberDiabloMonsterIndex = -1;
			return;
		}
		xp = UberRow - 2;
		yp = UberCol;
		UberDiabloMonsterIndex = nummonsters;
	}
#endif
	PlaceMonster(nummonsters, uniqtype, xp, yp);
	mon->_uniqtype = uniqindex + 1;

	if (uniqm->mlevel) {
		mon->mLevel = 2 * uniqm->mlevel;
	} else {
		mon->mLevel += 5;
	}

	mon->mExp *= 2;
	mon->mName = uniqm->mName;
	mon->_mmaxhp = uniqm->mmaxhp << 6;

	if (gbMaxPlayers == 1) {
		mon->_mmaxhp = mon->_mmaxhp >> 1;
		if (mon->_mmaxhp < 64) {
			mon->_mmaxhp = 64;
		}
	}

	mon->_mhitpoints = mon->_mmaxhp;
	mon->_mAi = uniqm->mAi;
	mon->_mint = uniqm->mint;
	mon->mMinDamage = uniqm->mMinDamage;
	mon->mMaxDamage = uniqm->mMaxDamage;
	mon->mMinDamage2 = uniqm->mMinDamage;
	mon->mMaxDamage2 = uniqm->mMaxDamage;
	mon->mMagicRes = uniqm->mMagicRes;
	mon->mtalkmsg = uniqm->mtalkmsg;
#ifdef HELLFIRE
	if (uniqindex == UMT_HORKDMN)
		mon->mlid = 0;
	else
#endif
		mon->mlid = AddLight(mon->_mx, mon->_my, 3);

	if (gbMaxPlayers != 1) {
		if (mon->_mAi == AI_LAZHELP)
			mon->mtalkmsg = 0;
#ifndef HELLFIRE
		if (mon->_mAi != AI_LAZURUS || quests[Q_BETRAYER]._qvar1 <= 3) {
			if (mon->mtalkmsg) {
				mon->_mgoal = MGOAL_INQUIRING;
			}
		} else {
			mon->_mgoal = MGOAL_NORMAL;
		}
#endif
	}
#ifdef HELLFIRE
	if (mon->mtalkmsg)
#else
	else if (mon->mtalkmsg)
#endif
		mon->_mgoal = MGOAL_INQUIRING;

	if (gnDifficulty == DIFF_NIGHTMARE) {
#ifdef HELLFIRE
		mon->_mmaxhp = 3 * mon->_mmaxhp + ((gbMaxPlayers != 1 ? 100 : 50) << 6);
#else
		mon->_mmaxhp = 3 * mon->_mmaxhp + 64;
#endif
		mon->mLevel += 15;
		mon->_mhitpoints = mon->_mmaxhp;
		mon->mExp = 2 * (mon->mExp + 1000);
		mon->mMinDamage = 2 * (mon->mMinDamage + 2);
		mon->mMaxDamage = 2 * (mon->mMaxDamage + 2);
		mon->mMinDamage2 = 2 * (mon->mMinDamage2 + 2);
		mon->mMaxDamage2 = 2 * (mon->mMaxDamage2 + 2);
	}

#ifdef HELLFIRE
	else if (gnDifficulty == DIFF_HELL) {
#else
	if (gnDifficulty == DIFF_HELL) {
#endif
#ifdef HELLFIRE
		mon->_mmaxhp = 4 * mon->_mmaxhp + ((gbMaxPlayers != 1 ? 200 : 100) << 6);
#else
		mon->_mmaxhp = 4 * mon->_mmaxhp + 192;
#endif
		mon->mLevel += 30;
		mon->_mhitpoints = mon->_mmaxhp;
		mon->mExp = 4 * (mon->mExp + 1000);
		mon->mMinDamage = 4 * mon->mMinDamage + 6;
		mon->mMaxDamage = 4 * mon->mMaxDamage + 6;
		mon->mMinDamage2 = 4 * mon->mMinDamage2 + 6;
		mon->mMaxDamage2 = 4 * mon->mMaxDamage2 + 6;
	}

	sprintf(filestr, "Monsters\\Monsters\\%s.TRN", uniqm->mTrnName);
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

	nummonsters++;

	if (uniqm->mUnqAttr & 1) {
		PlaceGroup(miniontype, unpackfilesize, uniqm->mUnqAttr, nummonsters - 1);
	}

	if (mon->_mAi != AI_GARG) {
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];
		mon->_mAnimFrame = random_(88, mon->_mAnimLen - 1) + 1;
		mon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		mon->_mmode = MM_STAND;
	}
}

void PlaceQuestMonsters()
{
	int skeltype;
	BYTE *setp;

	if (!setlevel) {
		if (QuestStatus(Q_BUTCHER)) {
			PlaceUniqueMonst(UMT_BUTCHER, 0, 0);
		}

		if (currlevel == quests[Q_SKELKING]._qlevel && gbMaxPlayers != 1) {
			skeltype = 0;

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
			AddMonsterType(UniqMonst[UMT_WARLORD].mtype, 1);
		}
		if (QuestStatus(Q_VEIL)) {
			AddMonsterType(UniqMonst[UMT_LACHDAN].mtype, 1);
		}
		if (QuestStatus(Q_ZHAR) && zharlib == -1) {
			quests[Q_ZHAR]._qactive = QUEST_NOTAVAIL;
		}

		if (currlevel == quests[Q_BETRAYER]._qlevel && gbMaxPlayers != 1) {
			AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, 4);
			AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, 4);
			PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
			PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
			PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
			setp = LoadFileInMem("Levels\\L4Data\\Vile1.DUN", NULL);
			SetMapMonsters(setp, 2 * setpc_x, 2 * setpc_y);
			mem_free_dbg(setp);
		}
#ifdef HELLFIRE

		if (currlevel == 24) {
			UberDiabloMonsterIndex = -1;
			int i1;
			for (i1 = 0; i1 < nummtypes; i1++) {
				if (Monsters[i1].mtype == UniqMonst[UMT_NAKRUL].mtype)
					break;
			}

			if (i1 < nummtypes) {
				for (int i2 = 0; i2 < nummonsters; i2++) {
					if (monster[i2]._uniqtype == 0 || monster[i2]._mMTidx == i1) {
						UberDiabloMonsterIndex = i2;
						break;
					}
				}
			}
			if (UberDiabloMonsterIndex == -1)
				PlaceUniqueMonst(UMT_NAKRUL, 0, 0);
		}
#endif
	} else if (setlvlnum == SL_SKELKING) {
		PlaceUniqueMonst(UMT_SKELKING, 0, 0);
	}
}

void PlaceGroup(int mtype, int num, int leaderf, int leader)
{
	int placed, try1, try2, j;
	int xp, yp, x1, y1;

	placed = 0;

	for (try1 = 0; try1 < 10; try1++) {
		while (placed) {
			nummonsters--;
			placed--;
			dMonster[monster[nummonsters]._mx][monster[nummonsters]._my] = 0;
		}

		if (leaderf & 1) {
			int offset = random_(92, 8);
			x1 = xp = monster[leader]._mx + offset_x[offset];
			y1 = yp = monster[leader]._my + offset_y[offset];
		} else {
			do {
				x1 = xp = random_(93, 80) + 16;
				y1 = yp = random_(93, 80) + 16;
			} while (!MonstPlace(xp, yp));
		}

		if (num + nummonsters > totalmonsters) {
			num = totalmonsters - nummonsters;
		}

		j = 0;
		for (try2 = 0; j < num && try2 < 100; xp += offset_x[random_(94, 8)], yp += offset_x[random_(94, 8)]) { /// BUGFIX: `yp += offset_y`
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
					monster[nummonsters].leaderflag = 1;
					monster[nummonsters]._mAi = monster[leader]._mAi;
				}

				if (monster[nummonsters]._mAi != AI_GARG) {
					monster[nummonsters]._mAnimData = monster[nummonsters].MType->Anims[MA_STAND].Data[monster[nummonsters]._mdir];
					monster[nummonsters]._mAnimFrame = random_(88, monster[nummonsters]._mAnimLen - 1) + 1;
					monster[nummonsters]._mFlags &= ~MFLAG_ALLOW_SPECIAL;
					monster[nummonsters]._mmode = MM_STAND;
				}
			}
			nummonsters++;
			placed++;
			j++;
		}

		if (placed >= num) {
			break;
		}
	}

	if (leaderf & 2) {
		monster[leader].packsize = placed;
	}
}

void LoadDiabMonsts()
{
	BYTE *lpSetPiece;

	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab1.DUN", NULL);
	SetMapMonsters(lpSetPiece, 2 * diabquad1x, 2 * diabquad1y);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab2a.DUN", NULL);
	SetMapMonsters(lpSetPiece, 2 * diabquad2x, 2 * diabquad2y);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab3a.DUN", NULL);
	SetMapMonsters(lpSetPiece, 2 * diabquad3x, 2 * diabquad3y);
	mem_free_dbg(lpSetPiece);
	lpSetPiece = LoadFileInMem("Levels\\L4Data\\diab4a.DUN", NULL);
	SetMapMonsters(lpSetPiece, 2 * diabquad4x, 2 * diabquad4y);
	mem_free_dbg(lpSetPiece);
}

void InitMonsters()
{
	int na, nt;
	int i, s, t;
	int numplacemonsters;
	int mtype;
	int numscattypes;
	int scattertypes[NUM_MTYPES];

	numscattypes = 0;
	if (gbMaxPlayers != 1)
		CheckDungeonClear();
	if (!setlevel) {
		AddMonster(1, 0, 0, 0, FALSE);
		AddMonster(1, 0, 0, 0, FALSE);
		AddMonster(1, 0, 0, 0, FALSE);
		AddMonster(1, 0, 0, 0, FALSE);
	}

#ifndef SPAWN
	if (!setlevel && currlevel == 16)
		LoadDiabMonsts();
#endif

	nt = numtrigs;
	if (currlevel == 15)
		nt = 1;
	for (i = 0; i < nt; i++) {
		for (s = -2; s < 2; s++) {
			for (t = -2; t < 2; t++)
				DoVision(s + trigs[i]._tx, t + trigs[i]._ty, 15, FALSE, FALSE);
		}
	}
#ifndef SPAWN
	PlaceQuestMonsters();
#endif
	if (!setlevel) {
#ifndef SPAWN
		PlaceUniques();
#endif
		na = 0;
		for (s = 16; s < 96; s++)
			for (t = 16; t < 96; t++)
				if (!SolidLoc(s, t))
					na++;
		numplacemonsters = na / 30;
		if (gbMaxPlayers != 1)
			numplacemonsters += numplacemonsters >> 1;
		if (nummonsters + numplacemonsters > 190)
			numplacemonsters = 190 - nummonsters;
		totalmonsters = nummonsters + numplacemonsters;
		for (i = 0; i < nummtypes; i++) {
			if (Monsters[i].mPlaceFlags & 1) {
				scattertypes[numscattypes] = i;
				numscattypes++;
			}
		}
		while (nummonsters < totalmonsters) {
			mtype = scattertypes[random_(95, numscattypes)];
			if (currlevel == 1 || random_(95, 2) == 0)
				na = 1;
#ifdef HELLFIRE
			else if (currlevel == 2 || currlevel >= 21 && currlevel <= 24)
#else
			else if (currlevel == 2)
#endif
				na = random_(95, 2) + 2;
			else
				na = random_(95, 3) + 3;
			PlaceGroup(mtype, na, 0, 0);
		}
	}
	for (i = 0; i < nt; i++) {
		for (s = -2; s < 2; s++) {
			for (t = -2; t < 2; t++)
				DoUnVision(s + trigs[i]._tx, t + trigs[i]._ty, 15);
		}
	}
}

void PlaceUniques()
{
	int u, mt;
	BOOL done;

	for (u = 0; UniqMonst[u].mtype != -1; u++) {
		if (UniqMonst[u].mlevel != currlevel)
			continue;
		done = FALSE;
		for (mt = 0; mt < nummtypes; mt++) {
			if (done)
				break;
			done = (Monsters[mt].mtype == UniqMonst[u].mtype);
		}
		mt--;
		if (u == UMT_GARBUD && quests[Q_GARBUD]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_ZHAR && quests[Q_ZHAR]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_SNOTSPIL && quests[Q_LTBANNER]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_LACHDAN && quests[Q_VEIL]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (u == UMT_WARLORD && quests[Q_WARLORD]._qactive == QUEST_NOTAVAIL)
			done = FALSE;
		if (done)
			PlaceUniqueMonst(u, mt, 8);
	}
}

void SetMapMonsters(BYTE *pMap, int startx, int starty)
{
	WORD rw, rh;
	WORD *lm;
	int i, j;
	int mtype;

	AddMonsterType(MT_GOLEM, 2);
	AddMonster(1, 0, 0, 0, FALSE);
	AddMonster(1, 0, 0, 0, FALSE);
	AddMonster(1, 0, 0, 0, FALSE);
	AddMonster(1, 0, 0, 0, FALSE);
	if (setlevel && setlvlnum == SL_VILEBETRAYER) {
		AddMonsterType(UniqMonst[UMT_LAZURUS].mtype, 4);
		AddMonsterType(UniqMonst[UMT_RED_VEX].mtype, 4);
		AddMonsterType(UniqMonst[UMT_BLACKJADE].mtype, 4);
		PlaceUniqueMonst(UMT_LAZURUS, 0, 0);
		PlaceUniqueMonst(UMT_RED_VEX, 0, 0);
		PlaceUniqueMonst(UMT_BLACKJADE, 0, 0);
	}
	lm = (WORD *)pMap;
	rw = SDL_SwapLE16(*lm);
	lm++;
	rh = SDL_SwapLE16(*lm);
	lm += (rw * rh + 1);
	rw = rw << 1;
	rh = rh << 1;
	lm += rw * rh;

	for (j = 0; j < rh; j++) {
		for (i = 0; i < rw; i++) {
			if (*lm) {
				mtype = AddMonsterType(MonstConvTbl[SDL_SwapLE16(*lm) - 1], 2);
				PlaceMonster(nummonsters++, mtype, i + startx + 16, j + starty + 16);
			}
			lm++;
		}
	}
}

void DeleteMonster(int i)
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
		int i = monstactive[nummonsters++];
		if (InMap)
			dMonster[x][y] = i + 1;
		InitMonster(i, dir, mtype, x, y);
		return i;
	}

	return -1;
}

#ifdef HELLFIRE
void monster_43C785(int mnum)
{
	int x, y, d, j, oi, dir, mx, my;

	if (monster[mnum].MType) {
		mx = monster[mnum]._mx;
		my = monster[mnum]._my;
		dir = monster[mnum]._mdir;
		for (d = 0; d < 8; d++) {
			x = mx + offset_x[d];
			y = my + offset_y[d];
			if (!SolidLoc(x, y)) {
				if (!dPlayer[x][y] && !dMonster[x][y]) {
					if (!dObject[x][y])
						break;
					oi = dObject[x][y] > 0 ? dObject[x][y] - 1 : -(dObject[x][y] + 1);
					if (!object[oi]._oSolidFlag)
						break;
				}
			}
		}
		if (d < 8) {
			for (j = 0; j < MAX_LVLMTYPES; j++) {
				if (Monsters[j].mtype == monster[mnum].MType->mtype)
					break;
			}
			if (j < MAX_LVLMTYPES)
				AddMonster(x, y, dir, j, TRUE);
		}
	}
}
#endif

void NewMonsterAnim(int mnum, AnimStruct *anim, int md)
{
	MonsterStruct *mon = monster + mnum;
	mon->_mAnimData = anim->Data[md];
	mon->_mAnimLen = anim->Frames;
	mon->_mAnimCnt = 0;
	mon->_mAnimFrame = 1;
	mon->_mAnimDelay = anim->Rate;
	mon->_mFlags &= ~(MFLAG_LOCK_ANIMATION | MFLAG_ALLOW_SPECIAL);
	mon->_mdir = md;
}

BOOL MonRanged(int mnum)
{
	char ai = monster[mnum]._mAi;
	return ai == AI_SKELBOW || ai == AI_GOATBOW || ai == AI_SUCC || ai == AI_LAZHELP;
}

BOOL MonTalker(int mnum)
{
	char ai = monster[mnum]._mAi;
	return ai == AI_LAZURUS
	    || ai == AI_WARLORD
	    || ai == AI_GARBUD
	    || ai == AI_ZHAR
	    || ai == AI_SNOTSPIL
	    || ai == AI_LACHDAN
	    || ai == AI_LAZHELP;
}

void MonEnemy(int mnum)
{
	int j;
	int mi, pnum;
	int dist, best_dist;
	int _menemy;
	BOOL sameroom, bestsameroom;
	MonsterStruct *mon;
	BYTE enemyx, enemyy;

	_menemy = -1;
	best_dist = -1;
	bestsameroom = 0;
	mon = monster + mnum;
	if (!(mon->_mFlags & MFLAG_GOLEM)) {
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (!plr[pnum].plractive || currlevel != plr[pnum].plrlevel || plr[pnum]._pLvlChanging || (plr[pnum]._pHitPoints == 0 && gbMaxPlayers != 1))
				continue;
			if (dTransVal[mon->_mx][mon->_my] == dTransVal[plr[pnum]._px][plr[pnum]._py])
				sameroom = TRUE;
			else
				sameroom = FALSE;
			if (abs(mon->_mx - plr[pnum]._px) > abs(mon->_my - plr[pnum]._py))
				dist = mon->_mx - plr[pnum]._px;
			else
				dist = mon->_my - plr[pnum]._py;
			dist = abs(dist);
			if ((sameroom && !bestsameroom)
			    || ((sameroom || !bestsameroom) && dist < best_dist)
			    || (_menemy == -1)) {
				mon->_mFlags &= ~MFLAG_TARGETS_MONSTER;
				_menemy = pnum;
				enemyx = plr[pnum]._pfutx;
				enemyy = plr[pnum]._pfuty;
				best_dist = dist;
				bestsameroom = sameroom;
			}
		}
	}
	for (j = 0; j < nummonsters; j++) {
		mi = monstactive[j];
		if (mi == mnum)
			continue;
		if (monster[mi]._mx == 1 && monster[mi]._my == 0)
			continue;
		if (MonTalker(mi) && monster[mi].mtalkmsg)
			continue;
		if (!(mon->_mFlags & MFLAG_GOLEM)
		    && ((abs(monster[mi]._mx - mon->_mx) >= 2 || abs(monster[mi]._my - mon->_my) >= 2) && !MonRanged(mnum)
		           || (!(mon->_mFlags & MFLAG_GOLEM) && !(monster[mi]._mFlags & MFLAG_GOLEM)))) {
			continue;
		}
		sameroom = dTransVal[mon->_mx][mon->_my] == dTransVal[monster[mi]._mx][monster[mi]._my];
		if (abs(mon->_mx - monster[mi]._mx) > abs(mon->_my - monster[mi]._my))
			dist = mon->_mx - monster[mi]._mx;
		else
			dist = mon->_my - monster[mi]._my;
		dist = abs(dist);
		if ((sameroom && !bestsameroom)
		    || ((sameroom || !bestsameroom) && dist < best_dist)
		    || (_menemy == -1)) {
			mon->_mFlags |= MFLAG_TARGETS_MONSTER;
			_menemy = mi;
			enemyx = monster[mi]._mfutx;
			enemyy = monster[mi]._mfuty;
			best_dist = dist;
			bestsameroom = sameroom;
		}
	}
	if (_menemy != -1) {
		mon->_mFlags &= ~MFLAG_NO_ENEMY;
		mon->_menemy = _menemy;
		mon->_menemyx = enemyx;
		mon->_menemyy = enemyy;
	} else {
		mon->_mFlags |= MFLAG_NO_ENEMY;
	}
}

int MonGetDir(int mnum)
{
	return GetDirection(monster[mnum]._mx, monster[mnum]._my, monster[mnum]._menemyx, monster[mnum]._menemyy);
}

void MonStartStand(int mnum, int md)
{
	MonsterStruct *mon;

	ClearMVars(mnum);
	mon = &monster[mnum];
	if (mon->MType->mtype == MT_GOLEM)
		NewMonsterAnim(mnum, &mon->MType->Anims[MA_WALK], md);
	else
		NewMonsterAnim(mnum, &mon->MType->Anims[MA_STAND], md);
	mon->_mVar1 = mon->_mmode;
	mon->_mVar2 = 0;
	mon->_mmode = MM_STAND;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
	MonEnemy(mnum);
}

void MonStartDelay(int mnum, int len)
{
	if (len <= 0) {
		return;
	}

	if (monster[mnum]._mAi != AI_LAZURUS) {
		monster[mnum]._mVar2 = len;
		monster[mnum]._mmode = MM_DELAY;
	}
}

void MonStartSpStand(int mnum, int md)
{
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_SPECIAL], md);
	mon->_mmode = MM_SPSTAND;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
}

void MonStartWalk(int mnum, int xvel, int yvel, int xadd, int yadd, int EndDir)
{
	MonsterStruct *mon = &monster[mnum];
	int fx = xadd + mon->_mx;
	int fy = yadd + mon->_my;

	dMonster[fx][fy] = -(mnum + 1);
	mon->_mmode = MM_WALK;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mfutx = fx;
	mon->_mfuty = fy;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mVar1 = xadd;
	mon->_mVar2 = yadd;
	mon->_mVar3 = EndDir;
	mon->_mdir = EndDir;
	NewMonsterAnim(mnum, &mon->MType->Anims[MA_WALK], EndDir);
	mon->_mVar6 = 0;
	mon->_mVar7 = 0;
	mon->_mVar8 = 0;
}

void MonStartWalk2(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int EndDir)
{
	MonsterStruct *mon = &monster[mnum];
	int fx = xadd + mon->_mx;
	int fy = yadd + mon->_my;

	dMonster[mon->_mx][mon->_my] = -(mnum + 1);
	mon->_mVar1 = mon->_mx;
	mon->_mVar2 = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mx = fx;
	mon->_my = fy;
	mon->_mfutx = fx;
	mon->_mfuty = fy;
	dMonster[fx][fy] = mnum + 1;
#ifdef HELLFIRE
	if (!(mon->_mFlags & MFLAG_HIDDEN) && mon->mlid != 0)
#else
	if (mon->_uniqtype != 0)
#endif
		ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	mon->_mmode = MM_WALK2;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mVar3 = EndDir;
	mon->_mdir = EndDir;
	NewMonsterAnim(mnum, &mon->MType->Anims[MA_WALK], EndDir);
	mon->_mVar6 = 16 * xoff;
	mon->_mVar7 = 16 * yoff;
	mon->_mVar8 = 0;
}

void MonStartWalk3(int mnum, int xvel, int yvel, int xoff, int yoff, int xadd, int yadd, int mapx, int mapy, int EndDir)
{
	MonsterStruct *mon = &monster[mnum];
	int fx = xadd + mon->_mx;
	int fy = yadd + mon->_my;
	int x = mapx + mon->_mx;
	int y = mapy + mon->_my;

#ifdef HELLFIRE
	if (!(mon->_mFlags & MFLAG_HIDDEN) && mon->mlid != 0)
#else
	if (mon->_uniqtype != 0)
#endif
		ChangeLightXY(mon->mlid, x, y);

	dMonster[mon->_mx][mon->_my] = -(mnum + 1);
	dMonster[fx][fy] = -(mnum + 1);
	mon->_mVar4 = x;
	mon->_mVar5 = y;
	dFlags[x][y] |= BFLAG_MONSTLR;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mfutx = fx;
	mon->_mfuty = fy;
	mon->_mxoff = xoff;
	mon->_myoff = yoff;
	mon->_mmode = MM_WALK3;
	mon->_mxvel = xvel;
	mon->_myvel = yvel;
	mon->_mVar1 = fx;
	mon->_mVar2 = fy;
	mon->_mVar3 = EndDir;
	mon->_mdir = EndDir;
	NewMonsterAnim(mnum, &mon->MType->Anims[MA_WALK], EndDir);
	mon->_mVar6 = 16 * xoff;
	mon->_mVar7 = 16 * yoff;
	mon->_mVar8 = 0;
}

void MonStartAttack(int mnum)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_ATTACK], md);
	mon->_mmode = MM_ATTACK;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
}

void MonStartRAttack(int mnum, int mitype, int dam)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_ATTACK], md);
	mon->_mmode = MM_RATTACK;
	mon->_mVar1 = mitype;
	mon->_mVar2 = dam;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
}

void MonStartRSpAttack(int mnum, int mitype, int dam)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_SPECIAL], md);
	mon->_mmode = MM_RSPATTACK;
	mon->_mVar1 = mitype;
	mon->_mVar2 = 0;
	mon->_mVar3 = dam;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
}

void MonStartSpAttack(int mnum)
{
	int md = MonGetDir(mnum);
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_SPECIAL], md);
	mon->_mmode = MM_SATTACK;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
}

void MonStartEat(int mnum)
{
	MonsterStruct *mon = &monster[mnum];

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_SPECIAL], mon->_mdir);
	mon->_mmode = MM_SATTACK;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
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

void MonGetKnockback(int mnum)
{
	MonsterStruct *mon = &monster[mnum];
	int d = (mon->_mdir - 4) & 7;
	if (DirOK(mnum, d)) {
		MonClearSquares(mnum);
		mon->_moldx += offset_x[d];
		mon->_moldy += offset_y[d];
		NewMonsterAnim(mnum, &mon->MType->Anims[MA_GOTHIT], mon->_mdir);
		mon->_mmode = MM_GOTHIT;
		mon->_mxoff = 0;
		mon->_myoff = 0;
		mon->_mx = mon->_moldx;
		mon->_my = mon->_moldy;
		mon->_mfutx = mon->_mx;
		mon->_mfuty = mon->_my;
		// BUGFIX useless assignment
		mon->_moldx = mon->_mx;
		mon->_moldy = mon->_my;
		MonClearSquares(mnum);
		dMonster[mon->_mx][mon->_my] = mnum + 1;
	}
}

void MonStartHit(int mnum, int pnum, int dam)
{
	MonsterStruct *mon = &monster[mnum];

	if (pnum >= 0)
		mon->mWhoHit |= 1 << pnum;
	if (pnum == myplr) {
		delta_monster_hp(mnum, mon->_mhitpoints, currlevel);
		NetSendCmdParam2(FALSE, CMD_MONSTDAMAGE, mnum, dam);
	}
	PlayEffect(mnum, 1);
	if (mon->MType->mtype >= MT_SNEAK && mon->MType->mtype <= MT_ILLWEAV || dam >> 6 >= mon->mLevel + 3) {
		if (pnum >= 0) {
			mon->_mFlags &= ~MFLAG_TARGETS_MONSTER;
			mon->_menemy = pnum;
			mon->_menemyx = plr[pnum]._pfutx;
			mon->_menemyy = plr[pnum]._pfuty;
			mon->_mdir = MonGetDir(mnum);
		}
		if (mon->MType->mtype == MT_BLINK) {
			MonTeleport(mnum);
		} else if (mon->MType->mtype >= MT_NSCAV && mon->MType->mtype <= MT_YSCAV) {
			mon->_mgoal = MGOAL_NORMAL;
		}
		if (mon->_mmode != MM_STONE) {
			NewMonsterAnim(mnum, &mon->MType->Anims[MA_GOTHIT], mon->_mdir);
			mon->_mmode = MM_GOTHIT;
			mon->_mxoff = 0;
			mon->_myoff = 0;
			mon->_mx = mon->_moldx;
			mon->_my = mon->_moldy;
			mon->_mfutx = mon->_moldx;
			mon->_mfuty = mon->_moldy;
			MonClearSquares(mnum);
			dMonster[mon->_mx][mon->_my] = mnum + 1;
		}
	}
}

void MonDiabloDeath(int mnum, BOOL sendmsg)
{
	MonsterStruct *mon, *pmonster;
	int dist;
	int j, k;
	int _moldx, _moldy;

	mon = &monster[mnum];
	PlaySFX(USFX_DIABLOD);
	quests[Q_DIABLO]._qactive = QUEST_DONE;
	if (sendmsg)
		NetSendCmdQuest(TRUE, Q_DIABLO);
	gbProcessPlayers = FALSE;
	sgbSaveSoundOn = gbSoundOn;
	for (j = 0; j < nummonsters; j++) {
		k = monstactive[j];
		if (k == mnum || mon->_msquelch == 0)
			continue;

		pmonster = &monster[k];
		NewMonsterAnim(k, &pmonster->MType->Anims[MA_DEATH], pmonster->_mdir);
		pmonster->_mxoff = 0;
		pmonster->_myoff = 0;
		pmonster->_mVar1 = 0;
		_moldx = pmonster->_moldx;
		_moldy = pmonster->_moldy;
		pmonster->_my = _moldy;
		pmonster->_mfuty = _moldy;
		pmonster->_mmode = MM_DEATH;
		pmonster->_mx = _moldx;
		pmonster->_mfutx = _moldx;
		MonClearSquares(k);
		dMonster[pmonster->_mx][pmonster->_my] = k + 1;
	}
	AddLight(mon->_mx, mon->_my, 8);
	DoVision(mon->_mx, mon->_my, 8, FALSE, TRUE);
	if (abs(ViewX - mon->_mx) > abs(ViewY - mon->_my))
		dist = abs(ViewX - mon->_mx);
	else
		dist = abs(ViewY - mon->_my);
	if (dist > 20)
		dist = 20;
	j = ViewX << 16;
	k = ViewY << 16;
	mon->_mVar3 = j;
	mon->_mVar4 = k;
	mon->_mVar5 = (int)((j - (mon->_mx << 16)) / (double)dist);
	mon->_mVar6 = (int)((k - (mon->_my << 16)) / (double)dist);
}

#ifdef HELLFIRE
void SpawnLoot(int mnum, BOOL sendmsg)
{
	int nSFX;
	MonsterStruct *mon;

	mon = &monster[mnum];
	if (QuestStatus(Q_GARBUD) && mon->mName == UniqMonst[UMT_GARBUD].mName) {
		CreateTypeItem(mon->_mx + 1, mon->_my + 1, TRUE, ITYPE_MACE, IMISC_NONE, TRUE, FALSE);
	} else if (mon->mName == UniqMonst[UMT_DEFILER].mName) {
		stream_stop();
		quests[Q_DEFILER]._qlog = 0;
		SpawnMapOfDoom(mon->_mx, mon->_my);
	} else if (mon->mName == UniqMonst[UMT_HORKDMN].mName) {
		if (UseTheoQuest) {
			SpawnTheodore(mon->_mx, mon->_my);
		} else {
			CreateAmulet(mon->_mx, mon->_my, 13, FALSE, TRUE);
		}
	} else if (mon->MType->mtype == MT_HORKSPWN) {
	} else if (mon->MType->mtype == MT_NAKRUL) {
		stream_stop();
		quests[Q_NAKRUL]._qlog = 0;
		UberDiabloMonsterIndex = -2;
		CreateMagicWeapon(mon->_mx, mon->_my, ITYPE_SWORD, ICURS_GREAT_SWORD, FALSE, TRUE);
		CreateMagicWeapon(mon->_mx, mon->_my, ITYPE_STAFF, ICURS_WAR_STAFF, FALSE, TRUE);
		CreateMagicWeapon(mon->_mx, mon->_my, ITYPE_BOW, ICURS_LONG_WAR_BOW, FALSE, TRUE);
		CreateSpellBook(mon->_mx, mon->_my, SPL_APOCA, FALSE, TRUE);
	} else if (mnum > 3) {
		SpawnItem(mnum, mon->_mx, mon->_my, sendmsg);
	}
}
#endif

void M2MStartHit(int defm, int offm, int dam)
{
	MonsterStruct *dmon;
	if ((DWORD)defm >= MAXMONSTERS) {
		app_fatal("Invalid monster %d getting hit by monster", defm);
	}

	dmon = &monster[defm];
	if (dmon->MType == NULL) {
		app_fatal("Monster %d \"%s\" getting hit by monster: MType NULL", defm, dmon->mName);
	}

	if (offm >= 0)
		monster[offm].mWhoHit |= 1 << offm;

	delta_monster_hp(defm, dmon->_mhitpoints, currlevel);
	NetSendCmdParam2(FALSE, CMD_MONSTDAMAGE, defm, dam);
	PlayEffect(defm, 1);

	if (dmon->MType->mtype >= MT_SNEAK && dmon->MType->mtype <= MT_ILLWEAV || dam >> 6 >= dmon->mLevel + 3) {
		if (offm >= 0)
			dmon->_mdir = (monster[offm]._mdir - 4) & 7;

		if (dmon->MType->mtype == MT_BLINK) {
			MonTeleport(defm);
		} else if (dmon->MType->mtype >= MT_NSCAV && dmon->MType->mtype <= MT_YSCAV) {
			dmon->_mgoal = MGOAL_NORMAL;
		}

		if (dmon->_mmode != MM_STONE) {
			if (dmon->MType->mtype != MT_GOLEM) {
				NewMonsterAnim(defm, &dmon->MType->Anims[MA_GOTHIT], dmon->_mdir);
				dmon->_mmode = MM_GOTHIT;
			}

			dmon->_mxoff = 0;
			dmon->_myoff = 0;
			dmon->_mx = dmon->_moldx;
			dmon->_my = dmon->_moldy;
			dmon->_mfutx = dmon->_moldx;
			dmon->_mfuty = dmon->_moldy;
			MonClearSquares(defm);
			dMonster[dmon->_mx][dmon->_my] = defm + 1;
		}
	}
}

void MonstStartKill(int mnum, int pnum, BOOL sendmsg)
{
	int md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonstStartKill: Invalid monster %d", mnum);
#endif
	}

	mon = &monster[mnum];
	if (!mon->MType) {
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonstStartKill: Monster %d \"%s\" MType NULL", mnum, monster[mnum].mName);
#endif
	}

	if (pnum >= 0)
		mon->mWhoHit |= 1 << pnum;
	if (pnum < MAX_PLRS && mnum > MAX_PLRS) /// BUGFIX: mnum >= MAX_PLRS
		AddPlrMonstExper(mon->mLevel, mon->mExp, mon->mWhoHit);
	monstkills[mon->MType->mtype]++;
	mon->_mhitpoints = 0;
	SetRndSeed(mon->_mRndSeed);
#ifdef HELLFIRE
	SpawnLoot(mnum, sendmsg);
#else
	if (QuestStatus(Q_GARBUD) && mon->mName == UniqMonst[UMT_GARBUD].mName) {
		CreateTypeItem(mon->_mx + 1, mon->_my + 1, TRUE, ITYPE_MACE, IMISC_NONE, TRUE, FALSE);
	} else if (mnum > MAX_PLRS - 1) { // Golems should not spawn items
		SpawnItem(mnum, mon->_mx, mon->_my, sendmsg);
	}
#endif
	if (mon->MType->mtype == MT_DIABLO)
		MonDiabloDeath(mnum, TRUE);
	else
		PlayEffect(mnum, 2);

	if (pnum >= 0)
		md = MonGetDir(mnum);
	else
		md = mon->_mdir;
	mon->_mdir = md;
	NewMonsterAnim(mnum, &mon->MType->Anims[MA_DEATH], md);
	mon->_mmode = MM_DEATH;
#ifdef HELLFIRE
	mon->_mgoal = 0;
#endif
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mVar1 = 0;
	mon->_mx = mon->_moldx;
	mon->_my = mon->_moldy;
	mon->_mfutx = mon->_moldx;
	mon->_mfuty = mon->_moldy;
	MonClearSquares(mnum);
	dMonster[mon->_mx][mon->_my] = mnum + 1;
	CheckQuestKill(mnum, sendmsg);
	MonFallenFear(mon->_mx, mon->_my);
#ifdef HELLFIRE
	if (mon->MType->mtype >= MT_NACID && mon->MType->mtype <= MT_XACID || mon->MType->mtype == MT_SPIDLORD)
#else
	if (mon->MType->mtype >= MT_NACID && mon->MType->mtype <= MT_XACID)
#endif
		AddMissile(mon->_mx, mon->_my, 0, 0, 0, MIS_ACIDPUD, 1, mnum, mon->_mint + 1, 0);
}

void M2MStartKill(int offm, int defm)
{
	MonsterStruct *dmon;
	int md;

	if ((DWORD)offm >= MAXMONSTERS) {
		app_fatal("M2MStartKill: Invalid monster (attacker) %d", offm);
	}
	if ((DWORD)offm >= MAXMONSTERS) { /// BUGFIX: should check `defm`
		app_fatal("M2MStartKill: Invalid monster (killed) %d", defm);
	}
	dmon = &monster[defm];
	if (!monster[offm].MType)
		app_fatal("M2MStartKill: Monster %d \"%s\" MType NULL", defm, dmon->mName);

	delta_kill_monster(defm, dmon->_mx, dmon->_my, currlevel);
	NetSendCmdLocParam1(FALSE, CMD_MONSTDEATH, dmon->_mx, dmon->_my, defm);

	dmon->mWhoHit |= 1 << offm;
	if (offm < MAX_PLRS)
		AddPlrMonstExper(dmon->mLevel, dmon->mExp, dmon->mWhoHit);

	monstkills[dmon->MType->mtype]++;
	dmon->_mhitpoints = 0;
	SetRndSeed(dmon->_mRndSeed);

	if (defm >= MAX_PLRS)
		SpawnItem(defm, dmon->_mx, dmon->_my, TRUE);

	if (dmon->MType->mtype == MT_DIABLO)
		MonDiabloDeath(defm, TRUE);
	else
		PlayEffect(offm, 2);

	PlayEffect(defm, 2);

	md = (monster[offm]._mdir - 4) & 7;
	if (dmon->MType->mtype == MT_GOLEM)
		md = 0;

	dmon->_mdir = md;
	NewMonsterAnim(defm, &dmon->MType->Anims[MA_DEATH], md);
	dmon->_mmode = MM_DEATH;
	dmon->_mxoff = 0;
	dmon->_myoff = 0;
	dmon->_mx = dmon->_moldx;
	dmon->_my = dmon->_moldy;
	dmon->_mfutx = dmon->_moldx;
	dmon->_mfuty = dmon->_moldy;
	MonClearSquares(defm);
	dMonster[dmon->_mx][dmon->_my] = defm + 1;
	CheckQuestKill(defm, TRUE);
	MonFallenFear(dmon->_mx, dmon->_my);
	if (dmon->MType->mtype >= MT_NACID && dmon->MType->mtype <= MT_XACID)
		AddMissile(dmon->_mx, dmon->_my, 0, 0, 0, MIS_ACIDPUD, 1, defm, dmon->_mint + 1, 0);
}

void MonStartKill(int mnum, int pnum)
{
	if ((DWORD)mnum >= MAXMONSTERS) {
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartKill: Invalid monster %d", mnum);
#endif
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
	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonSyncStartKill: Invalid monster %d", mnum);
#endif

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

#ifdef HELLFIRE
	MonstStartKill(mnum, pnum, FALSE);
#else
	if (monster[mnum]._mmode == MM_STONE) {
		MonstStartKill(mnum, pnum, FALSE);
		monster[mnum]._mmode = MM_STONE;
	} else {
		MonstStartKill(mnum, pnum, FALSE);
	}
#endif
}

void MonStartFadein(int mnum, int md, BOOL backwards)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartFadein: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->MType == NULL)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartFadein: Monster %d \"%s\" MType NULL", mnum, mon->mName);
#endif

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_SPECIAL], md);
	mon->_mmode = MM_FADEIN;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
	mon->_mFlags &= ~MFLAG_HIDDEN;
	if (backwards) {
		mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		mon->_mAnimFrame = mon->_mAnimLen;
	}
}

void MonStartFadeout(int mnum, int md, BOOL backwards)
{
	MonsterStruct *mon;
	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartFadeout: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->MType == NULL)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartFadeout: Monster %d \"%s\" MType NULL", mnum, mon->mName);
#endif

	NewMonsterAnim(mnum, &mon->MType->Anims[MA_SPECIAL], md);
	mon->_mmode = MM_FADEOUT;
	mon->_mxoff = 0;
	mon->_myoff = 0;
	mon->_mfutx = mon->_mx;
	mon->_mfuty = mon->_my;
	mon->_moldx = mon->_mx;
	mon->_moldy = mon->_my;
	mon->_mdir = md;
	if (backwards) {
		mon->_mFlags |= MFLAG_LOCK_ANIMATION;
		mon->_mAnimFrame = mon->_mAnimLen;
	}
}

void MonStartHeal(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartHeal: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->MType == NULL)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonStartHeal: Monster %d \"%s\" MType NULL", mnum, mon->mName);
#endif

	mon->_mAnimData = mon->MType->Anims[MA_SPECIAL].Data[mon->_mdir];
	mon->_mAnimFrame = mon->MType->Anims[MA_SPECIAL].Frames;
	mon->_mFlags |= MFLAG_LOCK_ANIMATION;
	mon->_mmode = MM_HEAL;
	mon->_mVar1 = mon->_mmaxhp / (16 * (random_(97, 5) + 4));
}

void MonChangeLightOffset(int mnum)
{
	MonsterStruct *mon;
	int lx, ly, _mxoff, _myoff, sign;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MonChangeLightOffset: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	lx = mon->_mxoff + 2 * mon->_myoff;
	ly = 2 * mon->_myoff - mon->_mxoff;

	if (lx < 0) {
		sign = -1;
		lx = -lx;
	} else {
		sign = 1;
	}

	_mxoff = sign * (lx >> 3);
	if (ly < 0) {
		_myoff = -1;
		ly = -ly;
	} else {
		_myoff = 1;
	}

	_myoff *= (ly >> 3);
#ifdef HELLFIRE
	if (mon->mlid)
#endif
		ChangeLightOff(mon->mlid, _mxoff, _myoff);
}

BOOL MonDoStand(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return 0;
#else
		app_fatal("MonDoStand: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->MType == NULL)
#ifdef HELLFIRE
		return 0;
#else
		app_fatal("MonDoStand: Monster %d \"%s\" MType NULL", mnum, mon->mName);
#endif

	if (mon->MType->mtype == MT_GOLEM)
		mon->_mAnimData = mon->MType->Anims[MA_WALK].Data[mon->_mdir];
	else
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];

	if (mon->_mAnimFrame == mon->_mAnimLen)
		MonEnemy(mnum);

	mon->_mVar2++;

	return FALSE;
}

BOOL MonDoWalk(int mnum)
{
	MonsterStruct *mon;
	BOOL rv;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoWalk: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoWalk: Monster %d \"%s\" MType NULL", mnum, mon->mName);

	rv = FALSE;
	if (mon->_mVar8 == mon->MType->Anims[MA_WALK].Frames) {
		dMonster[mon->_mx][mon->_my] = 0;
		mon->_mx += mon->_mVar1;
		mon->_my += mon->_mVar2;
		dMonster[mon->_mx][mon->_my] = mnum + 1;
		if (mon->_uniqtype != 0)
			ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = TRUE;
	} else if (!mon->_mAnimCnt) {
		mon->_mVar8++;
		mon->_mVar6 += mon->_mxvel;
		mon->_mVar7 += mon->_myvel;
		mon->_mxoff = mon->_mVar6 >> 4;
		mon->_myoff = mon->_mVar7 >> 4;
	}

	if (mon->_uniqtype != 0)
		MonChangeLightOffset(mnum);

	return rv;
}

BOOL MonDoWalk2(int mnum)
{
	MonsterStruct *mon;
	BOOL rv;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoWalk2: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoWalk2: Monster %d \"%s\" MType NULL", mnum, mon->mName);

	if (mon->_mVar8 == mon->MType->Anims[MA_WALK].Frames) {
		dMonster[mon->_mVar1][mon->_mVar2] = 0;
		if (mon->_uniqtype != 0)
			ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = TRUE;
	} else {
		if (!mon->_mAnimCnt) {
			mon->_mVar8++;
			mon->_mVar6 += mon->_mxvel;
			mon->_mVar7 += mon->_myvel;
			mon->_mxoff = mon->_mVar6 >> 4;
			mon->_myoff = mon->_mVar7 >> 4;
		}
		rv = FALSE;
	}
	if (mon->_uniqtype != 0)
		MonChangeLightOffset(mnum);

	return rv;
}

BOOL MonDoWalk3(int mnum)
{
	MonsterStruct *mon;
	BOOL rv;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoWalk3: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoWalk3: Monster %d \"%s\" MType NULL", mnum, mon->mName);

	if (mon->_mVar8 == mon->MType->Anims[MA_WALK].Frames) {
		dMonster[mon->_mx][mon->_my] = 0;
		mon->_mx = mon->_mVar1;
		mon->_my = mon->_mVar2;
		dFlags[mon->_mVar4][mon->_mVar5] &= ~BFLAG_MONSTLR;
		dMonster[mon->_mx][mon->_my] = mnum + 1;
		if (mon->_uniqtype)
			ChangeLightXY(mon->mlid, mon->_mx, mon->_my);
		MonStartStand(mnum, mon->_mdir);
		rv = TRUE;
	} else {
		if (!mon->_mAnimCnt) {
			mon->_mVar8++;
			mon->_mVar6 += mon->_mxvel;
			mon->_mVar7 += mon->_myvel;
			mon->_mxoff = mon->_mVar6 >> 4;
			mon->_myoff = mon->_mVar7 >> 4;
		}
		rv = FALSE;
	}
	if (mon->_uniqtype != 0)
		MonChangeLightOffset(mnum);

	return rv;
}

void MonTryM2MHit(int offm, int defm, int hper, int mind, int maxd)
{
	MonsterStruct *dmon;
	BOOL ret;

	if ((DWORD)defm >= MAXMONSTERS) {
		app_fatal("MonTryM2MHit: Invalid monster %d", defm);
	}
	dmon = &monster[defm];
	if (dmon->MType == NULL)
		app_fatal("MonTryM2MHit: Monster %d \"%s\" MType NULL", defm, dmon->mName);
	if (dmon->_mhitpoints >> 6 > 0 && (dmon->MType->mtype != MT_ILLWEAV || dmon->_mgoal != MGOAL_RETREAT)) {
		int hit = random_(4, 100);
		if (dmon->_mmode == MM_STONE)
			hit = 0;
		if (!CheckMonsterHit(defm, &ret) && hit < hper) {
			int dam = (mind + random_(5, maxd - mind + 1)) << 6;
			dmon->_mhitpoints -= dam;
			if (dmon->_mhitpoints >> 6 <= 0) {
				if (dmon->_mmode == MM_STONE) {
					M2MStartKill(offm, defm);
					dmon->_mmode = MM_STONE;
				} else {
					M2MStartKill(offm, defm);
				}
			} else {
				if (dmon->_mmode == MM_STONE) {
					M2MStartHit(defm, offm, dam);
					dmon->_mmode = MM_STONE;
				} else {
					M2MStartHit(defm, offm, dam);
				}
			}
		}
	}
}

void MonTryH2HHit(int mnum, int pnum, int Hit, int MinDam, int MaxDam)
{
	MonsterStruct *mon;
	PlayerStruct *p;
	int hit, hper;
	int dx, dy;
	int blk, blkper;
	int dam, mdam;
	int newx, newy;
	int j, misnum, ms_num, cur_ms_num, new_hp;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonTryH2HHit: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonTryH2HHit: Monster %d \"%s\" MType NULL", mnum, mon->mName);
	if (mon->_mFlags & MFLAG_TARGETS_MONSTER) {
		MonTryM2MHit(mnum, pnum, Hit, MinDam, MaxDam);
		return;
	}
	p = &plr[pnum];
	if (p->_pHitPoints >> 6 <= 0 || p->_pInvincible || p->_pSpellFlags & 1)
		return;
	dx = abs(mon->_mx - p->_px);
	dy = abs(mon->_my - p->_py);
	if (dx >= 2 || dy >= 2)
		return;

	hper = random_(98, 100);
#ifdef _DEBUG
	if (debug_mode_dollar_sign || debug_mode_key_inverted_v)
		hper = 1000;
#endif
	hit = Hit
	    + 2 * (mon->mLevel - p->_pLevel)
	    + 30
	    - p->_pIBonusAC
	    - p->_pIAC
	    - p->_pDexterity / 5;
	if (hit < 15)
		hit = 15;
	if (currlevel == 14 && hit < 20)
		hit = 20;
	if (currlevel == 15 && hit < 25)
		hit = 25;
	if (currlevel == 16 && hit < 30)
		hit = 30;
	if ((p->_pmode == PM_STAND || p->_pmode == PM_ATTACK) && p->_pBlockFlag) {
		blkper = random_(98, 100);
	} else {
		blkper = 100;
	}
	blk = p->_pDexterity
	    + p->_pBaseToBlk
	    - (mon->mLevel << 1)
	    + (p->_pLevel << 1);
	if (blk < 0)
		blk = 0;
	if (blk > 100)
		blk = 100;
	if (hper >= hit)
		return;
	if (blkper < blk) {
		PlrStartBlock(pnum, GetDirection(p->_px, p->_py, mon->_mx, mon->_my));
		return;
	}
	if (mon->MType->mtype == MT_YZOMBIE && pnum == myplr) {
		ms_num = -1;
		cur_ms_num = -1;
		for (j = 0; j < nummissiles; j++) {
			misnum = missileactive[j];
			if (missile[misnum]._mitype != MIS_MANASHIELD)
				continue;
			if (missile[misnum]._misource == pnum)
				cur_ms_num = misnum;
			else
				ms_num = misnum;
		}
		if (p->_pMaxHP > 64) {
			if (p->_pMaxHPBase > 64) {
				new_hp = p->_pMaxHP - 64;
				p->_pMaxHP = new_hp;
				if (p->_pHitPoints > new_hp) {
					p->_pHitPoints = new_hp;
					if (cur_ms_num >= 0)
						missile[cur_ms_num]._miVar1 = new_hp;
				}
				new_hp = p->_pMaxHPBase - 64;
				p->_pMaxHPBase = new_hp;
				if (p->_pHPBase > new_hp) {
					p->_pHPBase = new_hp;
					if (cur_ms_num >= 0)
						missile[cur_ms_num]._miVar2 = new_hp;
				}
			}
		}
	}
	dam = (MinDam << 6) + random_(99, (MaxDam - MinDam + 1) << 6);
	dam += (p->_pIGetHit << 6);
	if (dam < 64)
		dam = 64;
	if (pnum == myplr) {
		p->_pHitPoints -= dam;
		p->_pHPBase -= dam;
	}
	if (p->_pIFlags & ISPL_THORNS) {
		mdam = (random_(99, 3) + 1) << 6;
		mon->_mhitpoints -= mdam;
		if (mon->_mhitpoints >> 6 <= 0)
			MonStartKill(mnum, pnum);
		else
			MonStartHit(mnum, pnum, mdam);
	}
	if (!(mon->_mFlags & MFLAG_NOLIFESTEAL) && mon->MType->mtype == MT_SKING && gbMaxPlayers != 1)
		mon->_mhitpoints += dam;
	if (p->_pHitPoints > p->_pMaxHP) {
		p->_pHitPoints = p->_pMaxHP;
		p->_pHPBase = p->_pMaxHPBase;
	}
	if (p->_pHitPoints >> 6 <= 0) {
		SyncPlrKill(pnum, 0);
		return;
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
			FixPlayerLocation(pnum, p->_pdir);
			FixPlrWalkTags(pnum);
			dPlayer[newx][newy] = pnum + 1;
			SetPlayerOld(pnum);
		}
	}
}

BOOL MonDoAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoAttack: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoAttack: Monster %d \"%s\" MType NULL", mnum, mon->mName);
	if (mon->MType == NULL) // BUGFIX: should check MData
		app_fatal("MonDoAttack: Monster %d \"%s\" MData NULL", mnum, mon->mName);

	if (mon->_mAnimFrame == mon->MData->mAFNum) {
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit, mon->mMinDamage, mon->mMaxDamage);
		if (mon->_mAi != AI_SNAKE)
			PlayEffect(mnum, 0);
	}
	if (mon->MType->mtype >= MT_NMAGMA && mon->MType->mtype <= MT_WMAGMA && mon->_mAnimFrame == 9) {
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit + 10, mon->mMinDamage - 2, mon->mMaxDamage - 2);
		PlayEffect(mnum, 0);
	}
	if (mon->MType->mtype >= MT_STORM && mon->MType->mtype <= MT_MAEL && mon->_mAnimFrame == 13) {
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit - 20, mon->mMinDamage + 4, mon->mMaxDamage + 4);
		PlayEffect(mnum, 0);
	}
	if (mon->_mAi == AI_SNAKE && mon->_mAnimFrame == 1)
		PlayEffect(mnum, 0);
	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

BOOL MonDoRAttack(int mnum)
{
	MonsterStruct *mon;
	int multimissiles, i;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoRAttack: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoRAttack: Monster %d \"%s\" MType NULL", mnum, mon->mName);
	if (mon->MType == NULL) // BUGFIX: should check MData
		app_fatal("MonDoRAttack: Monster %d \"%s\" MData NULL", mnum, mon->mName);

	if (mon->_mAnimFrame == mon->MData->mAFNum) {
		if (mon->_mVar1 != -1) {
			if (mon->_mVar1 != MIS_CBOLT)
				multimissiles = 1;
			else
				multimissiles = 3;
			for (i = 0; i < multimissiles; i++) {
				AddMissile(
				    mon->_mx,
				    mon->_my,
				    mon->_menemyx,
				    mon->_menemyy,
				    mon->_mdir,
				    mon->_mVar1,
				    1,
				    mnum,
				    mon->_mVar2,
				    0);
			}
		}
		PlayEffect(mnum, 0);
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

int MonDoRSpAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoRSpAttack: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoRSpAttack: Monster %d \"%s\" MType NULL", mnum, mon->mName);
	if (mon->MType == NULL) // BUGFIX: should check MData
		app_fatal("MonDoRSpAttack: Monster %d \"%s\" MData NULL", mnum, mon->mName);

	if (mon->_mAnimFrame == mon->MData->mAFNum2 && !mon->_mAnimCnt) {
		AddMissile(
		    mon->_mx,
		    mon->_my,
		    mon->_menemyx,
		    mon->_menemyy,
		    mon->_mdir,
		    mon->_mVar1,
		    1,
		    mnum,
		    mon->_mVar3,
		    0);
		PlayEffect(mnum, 3);
	}

	if (mon->_mAi == AI_MEGA && mon->_mAnimFrame == 3) {
		int hadV2 = mon->_mVar2;
		mon->_mVar2++;
		if (hadV2 == 0) {
			mon->_mFlags |= MFLAG_ALLOW_SPECIAL;
		} else if (mon->_mVar2 == 15) {
			mon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

BOOL MonDoSAttack(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoSAttack: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoSAttack: Monster %d \"%s\" MType NULL", mnum, mon->mName);
	if (mon->MType == NULL) // BUGFIX: should check MData
		app_fatal("MonDoSAttack: Monster %d \"%s\" MData NULL", mnum, mon->mName);

	if (mon->_mAnimFrame == mon->MData->mAFNum2)
		MonTryH2HHit(mnum, mon->_menemy, mon->mHit2, mon->mMinDamage2, mon->mMaxDamage2);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

BOOL MonDoFadein(int mnum)
{
	MonsterStruct *mon;
	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoFadein: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if ((!(mon->_mFlags & MFLAG_LOCK_ANIMATION) || mon->_mAnimFrame != 1)
	    && (mon->_mFlags & MFLAG_LOCK_ANIMATION || mon->_mAnimFrame != mon->_mAnimLen)) {
		return FALSE;
	}

	MonStartStand(mnum, mon->_mdir);
	mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;

	return TRUE;
}

BOOL MonDoFadeout(int mnum)
{
	MonsterStruct *mon;
	int mt;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoFadeout: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if ((!(mon->_mFlags & MFLAG_LOCK_ANIMATION) || mon->_mAnimFrame != 1)
	    && (mon->_mFlags & MFLAG_LOCK_ANIMATION || mon->_mAnimFrame != mon->_mAnimLen)) {
		return FALSE;
	}

	mt = mon->MType->mtype;
	if (mt < MT_INCIN || mt > MT_HELLBURN) {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		mon->_mFlags |= MFLAG_HIDDEN;
	} else {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
	}

	MonStartStand(mnum, mon->_mdir);

	return TRUE;
}

int MonDoHeal(int mnum)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoHeal: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->_mFlags & MFLAG_NOHEAL) {
		mon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		mon->_mmode = MM_SATTACK;
		return FALSE;
	}

	if (mon->_mAnimFrame == 1) {
		mon->_mFlags &= ~MFLAG_LOCK_ANIMATION;
		mon->_mFlags |= MFLAG_ALLOW_SPECIAL;
		if (mon->_mVar1 + mon->_mhitpoints < mon->_mmaxhp) {
			mon->_mhitpoints = mon->_mVar1 + mon->_mhitpoints;
		} else {
			mon->_mhitpoints = mon->_mmaxhp;
			mon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
			mon->_mmode = MM_SATTACK;
		}
	}
	return FALSE;
}

int MonDoTalk(int mnum)
{
	MonsterStruct *mon;
	int tren;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoTalk: Invalid monster %d", mnum);

	mon = &monster[mnum];
	MonStartStand(mnum, mon->_mdir);
	mon->_mgoal = MGOAL_TALKING;
	if (effect_is_playing(alltext[mon->mtalkmsg].sfxnr))
		return FALSE;
	InitQTextMsg(mon->mtalkmsg);
	if (mon->mName == UniqMonst[UMT_GARBUD].mName) {
		if (mon->mtalkmsg == TEXT_GARBUD1)
			quests[Q_GARBUD]._qactive = QUEST_ACTIVE;
		quests[Q_GARBUD]._qlog = TRUE;
		if (mon->mtalkmsg == TEXT_GARBUD2 && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
			SpawnItem(mnum, mon->_mx + 1, mon->_my + 1, TRUE);
			mon->_mFlags |= MFLAG_QUEST_COMPLETE;
		}
	}
	if (mon->mName == UniqMonst[UMT_ZHAR].mName
	    && mon->mtalkmsg == TEXT_ZHAR1
	    && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
		quests[Q_ZHAR]._qactive = QUEST_ACTIVE;
		quests[Q_ZHAR]._qlog = TRUE;
		CreateTypeItem(mon->_mx + 1, mon->_my + 1, FALSE, ITYPE_MISC, IMISC_BOOK, TRUE, FALSE);
		mon->_mFlags |= MFLAG_QUEST_COMPLETE;
	}
	if (mon->mName == UniqMonst[UMT_SNOTSPIL].mName) {
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
			sprintf(tempstr, "SS Talk = %i, Flags = %i", mon->mtalkmsg, mon->_mFlags);
			app_fatal(tempstr);
		}
	}
	if (mon->mName == UniqMonst[UMT_LACHDAN].mName) {
		if (mon->mtalkmsg == TEXT_VEIL9) {
			quests[Q_VEIL]._qactive = QUEST_ACTIVE;
			quests[Q_VEIL]._qlog = TRUE;
		}
		if (mon->mtalkmsg == TEXT_VEIL11 && !(mon->_mFlags & MFLAG_QUEST_COMPLETE)) {
			SpawnUnique(UITEM_STEELVEIL, mon->_mx + 1, mon->_my + 1);
			mon->_mFlags |= MFLAG_QUEST_COMPLETE;
		}
	}
	if (mon->mName == UniqMonst[UMT_WARLORD].mName)
		quests[Q_WARLORD]._qvar1 = 2;
	if (mon->mName == UniqMonst[UMT_LAZURUS].mName && gbMaxPlayers != 1) {
		mon->_msquelch = UCHAR_MAX;
		mon->mtalkmsg = 0;
		quests[Q_BETRAYER]._qvar1 = 6;
		mon->_mgoal = MGOAL_NORMAL;
	}
	return FALSE;
}

void MonTeleport(int mnum)
{
	BOOL tren;
	MonsterStruct *mon;
	int k, j, x, y, _mx, _my, rx, ry;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonTeleport: Invalid monster %d", mnum);

	tren = FALSE;

	mon = &monster[mnum];
	if (mon->_mmode != MM_STONE) {
		_mx = mon->_menemyx;
		_my = mon->_menemyy;
		rx = 2 * random_(100, 2) - 1;
		ry = 2 * random_(100, 2) - 1;

		for (j = -1; j <= 1 && !tren; j++) {
			for (k = -1; k < 1 && !tren; k++) {
				if (j != 0 || k != 0) {
					x = _mx + rx * j;
					y = _my + ry * k;
					if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX && x != mon->_mx && y != mon->_my) {
						if (PosOkMonst(mnum, x, y))
							tren = TRUE;
					}
				}
			}
		}
	}

	if (tren) {
		MonClearSquares(mnum);
		dMonster[mon->_mx][mon->_my] = 0;
		dMonster[x][y] = mnum + 1;
		mon->_moldx = x;
		mon->_moldy = y;
		mon->_mdir = MonGetDir(mnum);
	}
}

BOOL MonDoGotHit(int mnum)
{
	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoGotHit: Invalid monster %d", mnum);

	if (monster[mnum].MType == NULL)
		app_fatal("MonDoGotHit: Monster %d \"%s\" MType NULL", mnum, monster[mnum].mName);
	if (monster[mnum]._mAnimFrame == monster[mnum]._mAnimLen) {
		MonStartStand(mnum, monster[mnum]._mdir);

		return TRUE;
	}

	return FALSE;
}

void MonUpdateLeader(int mnum)
{
	MonsterStruct *mon;
	int ma, j;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonUpdateLeader: Invalid monster %d", mnum);

	for (j = 0; j < nummonsters; j++) {
		mon = &monster[monstactive[j]];
		if (mon->leaderflag == 1 && mon->leader == mnum)
			mon->leaderflag = 0;
	}

	if (monster[mnum].leaderflag == 1) {
		monster[monster[mnum].leader].packsize--;
	}
}

void DoEnding()
{
	BOOL bMusicOn;
	int musicVolume;

	if (gbMaxPlayers > 1) {
		SNetLeaveGame(LEAVE_ENDING);
	}

	music_stop();

	if (gbMaxPlayers > 1) {
		SDL_Delay(1000);
	}

#ifdef SPAWN
	return;
#endif

	if (plr[myplr]._pClass == PC_WARRIOR) {
		play_movie("gendata\\DiabVic2.smk", FALSE);
	} else if (plr[myplr]._pClass == PC_SORCERER) {
		play_movie("gendata\\DiabVic1.smk", FALSE);
	} else {
		play_movie("gendata\\DiabVic3.smk", FALSE);
	}
	play_movie("gendata\\Diabend.smk", FALSE);

	bMusicOn = gbMusicOn;
	gbMusicOn = TRUE;

	musicVolume = sound_get_or_set_music_volume(1);
	sound_get_or_set_music_volume(0);

	music_start(TMUSIC_L2);
	loop_movie = TRUE;
	play_movie("gendata\\loopdend.smk", TRUE);
	loop_movie = FALSE;
	music_stop();

	sound_get_or_set_music_volume(musicVolume);
	gbMusicOn = bMusicOn;
}

void PrepDoEnding()
{
	int newKillLevel, i;
	DWORD *killLevel;

	gbSoundOn = sgbSaveSoundOn;
	gbRunGame = FALSE;
	deathflag = FALSE;
	cineflag = TRUE;

	killLevel = &plr[myplr].pDiabloKillLevel;
	newKillLevel = gnDifficulty + 1;
	if (*killLevel > newKillLevel)
		newKillLevel = *killLevel;
	plr[myplr].pDiabloKillLevel = newKillLevel;

	for (i = 0; i < MAX_PLRS; i++) {
		plr[i]._pmode = PM_QUIT;
		plr[i]._pInvincible = TRUE;
		if (gbMaxPlayers > 1) {
			if (plr[i]._pHitPoints >> 6 == 0)
				plr[i]._pHitPoints = 64;
			if (plr[i]._pMana >> 6 == 0)
				plr[i]._pMana = 64;
		}
	}
}

BOOL MonDoDeath(int mnum)
{
	MonsterStruct *mon;
	int var1;
	int x, y;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return FALSE;
#else
		app_fatal("MonDoDeath: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->MType == NULL)
#ifdef HELLFIRE
		return FALSE;
#else
		app_fatal("MonDoDeath: Monster %d \"%s\" MType NULL", mnum, mon->mName);
#endif

	mon->_mVar1++;
	var1 = mon->_mVar1;
	if (mon->MType->mtype == MT_DIABLO) {
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

		if (var1 == 140)
			PrepDoEnding();
	} else if (mon->_mAnimFrame == mon->_mAnimLen) {
		if (mon->_uniqtype == 0)
			AddDead(mon->_mx, mon->_my, mon->MType->mdeadval, (direction)mon->_mdir);
		else
			AddDead(mon->_mx, mon->_my, mon->_udeadval, (direction)mon->_mdir);

		dMonster[mon->_mx][mon->_my] = 0;
		mon->_mDelFlag = TRUE;

		MonUpdateLeader(mnum);
	}
	return FALSE;
}

BOOL MonDoSpStand(int mnum)
{
	MonsterStruct *mon;
	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return FALSE;
#else
		app_fatal("Mon: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->MType == NULL)
#ifdef HELLFIRE
		return FALSE;
#else
		app_fatal("MonDoSpStand: Monster %d \"%s\" MType NULL", mnum, mon->mName);
#endif

	if (mon->_mAnimFrame == mon->MData->mAFNum2)
		PlayEffect(mnum, 3);

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		MonStartStand(mnum, mon->_mdir);
		return TRUE;
	}

	return FALSE;
}

BOOL MonDoDelay(int mnum)
{
	MonsterStruct *mon;
	int mVar2;
	int oFrame;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoDelay: Invalid monster %d", mnum);
	mon = &monster[mnum];
	if (mon->MType == NULL)
		app_fatal("MonDoDelay: Monster %d \"%s\" MType NULL", mnum, mon->mName);

	mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[MonGetDir(mnum)];
	if (mon->_mAi == AI_LAZURUS) {
		if (mon->_mVar2 > 8 || mon->_mVar2 < 0)
			mon->_mVar2 = 8;
	}

	mVar2 = mon->_mVar2;
	mon->_mVar2--;

	if (!mVar2) {
		oFrame = mon->_mAnimFrame;
		MonStartStand(mnum, mon->_mdir);
		mon->_mAnimFrame = oFrame;
		return TRUE;
	}

	return FALSE;
}

BOOL MonDoStone(int mnum)
{
	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonDoStone: Invalid monster %d", mnum);

	if (!monster[mnum]._mhitpoints) {
		dMonster[monster[mnum]._mx][monster[mnum]._my] = 0;
		monster[mnum]._mDelFlag = TRUE;
	}

	return FALSE;
}

void MonWalkDir(int mnum, int md)
{
	int mwi;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonWalkDir: Invalid monster %d", mnum);

	mwi = monster[mnum].MType->Anims[MA_WALK].Frames - 1;
	switch (md) {
	case DIR_N:
		MonStartWalk(mnum, 0, -MWVel[mwi][1], -1, -1, DIR_N);
		break;
	case DIR_NE:
		MonStartWalk(mnum, MWVel[mwi][1], -MWVel[mwi][0], 0, -1, DIR_NE);
		break;
	case DIR_E:
		MonStartWalk3(mnum, MWVel[mwi][2], 0, -32, -16, 1, -1, 1, 0, DIR_E);
		break;
	case DIR_SE:
		MonStartWalk2(mnum, MWVel[mwi][1], MWVel[mwi][0], -32, -16, 1, 0, DIR_SE);
		break;
	case DIR_S:
		MonStartWalk2(mnum, 0, MWVel[mwi][1], 0, -32, 1, 1, DIR_S);
		break;
	case DIR_SW:
		MonStartWalk2(mnum, -MWVel[mwi][1], MWVel[mwi][0], 32, -16, 0, 1, DIR_SW);
		break;
	case DIR_W:
		MonStartWalk3(mnum, -MWVel[mwi][2], 0, 32, -16, -1, 1, 0, 1, DIR_W);
		break;
	case DIR_NW:
		MonStartWalk(mnum, -MWVel[mwi][1], -MWVel[mwi][0], -1, 0, DIR_NW);
		break;
	}
}

void GroupUnity(int mnum)
{
	MonsterStruct *mon, *leader, *bmon;
	int j;
	BOOL clear;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("GroupUnity: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (mon->leaderflag) {
		leader = &monster[mon->leader];
		clear = LineClearF(CheckNoSolid, mon->_mx, mon->_my, leader->_mfutx, leader->_mfuty);
		if (clear || mon->leaderflag != 1) {
			if (clear
			    && mon->leaderflag == 2
			    && abs(mon->_mx - leader->_mfutx) < 4
			    && abs(mon->_my - leader->_mfuty) < 4) {
				leader->packsize++;
				mon->leaderflag = 1;
			}
		} else {
			leader->packsize--;
			mon->leaderflag = 2;
		}
	}

	if (mon->leaderflag == 1) {
		if (mon->_msquelch > leader->_msquelch) {
			leader->_lastx = mon->_mx;
			leader->_lasty = mon->_my;
			leader->_msquelch = mon->_msquelch - 1;
		}
		if (leader->_mAi == AI_GARG) {
			if (leader->_mFlags & MFLAG_ALLOW_SPECIAL) {
				leader->_mmode = MM_SATTACK;
				leader->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
			}
		}
		return;
	}
	if (mon->_uniqtype != 0) {
		if (UniqMonst[mon->_uniqtype - 1].mUnqAttr & 2) {
			for (j = 0; j < nummonsters; j++) {
				bmon = &monster[monstactive[j]];
				if (bmon->leaderflag == 1 && bmon->leader == mnum) {
					if (mon->_msquelch > bmon->_msquelch) {
						bmon->_lastx = mon->_mx;
						bmon->_lasty = mon->_my;
						bmon->_msquelch = mon->_msquelch - 1;
					}
					if (bmon->_mAi == AI_GARG) {
						if (bmon->_mFlags & MFLAG_ALLOW_SPECIAL) {
							bmon->_mmode = MM_SATTACK;
							bmon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
						}
					}
				}
			}
		}
	}
}

BOOL MonCallWalk(int mnum, int md)
{
	int mdtemp;
	BOOL ok;

	mdtemp = md;
	ok = DirOK(mnum, md);
	if (random_(101, 2))
		ok = ok || (md = left[mdtemp], DirOK(mnum, md)) || (md = right[mdtemp], DirOK(mnum, md));
	else
		ok = ok || (md = right[mdtemp], DirOK(mnum, md)) || (md = left[mdtemp], DirOK(mnum, md));
	if (random_(102, 2))
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

BOOL MonPathWalk(int mnum)
{
	char path[MAX_PATH_LENGTH];
	BOOL(*Check)
	(int, int, int);

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MonPathWalk: Invalid monster %d", mnum);

	Check = PosOkMonst3;
	if (!(monster[mnum]._mFlags & MFLAG_CAN_OPEN_DOOR))
		Check = PosOkMonst;

	if (FindPath(Check, mnum, monster[mnum]._mx, monster[mnum]._my, monster[mnum]._menemyx, monster[mnum]._menemyy, path)) {
		MonCallWalk(mnum, plr2monst[path[0]]); /* plr2monst is local */
		return TRUE;
	}

	return FALSE;
}

BOOL MonCallWalk2(int mnum, int md)
{
	BOOL ok;
	int mdtemp;

	mdtemp = md;
	ok = DirOK(mnum, md);     // Can we continue in the same direction
	if (random_(101, 2)) { // Randomly go left or right
		ok = ok || (mdtemp = left[md], DirOK(mnum, left[md])) || (mdtemp = right[md], DirOK(mnum, right[md]));
	} else {
		ok = ok || (mdtemp = right[md], DirOK(mnum, right[md])) || (mdtemp = left[md], DirOK(mnum, left[md]));
	}

	if (ok)
		MonWalkDir(mnum, mdtemp);

	return ok;
}

BOOL MonDumbWalk(int mnum, int md)
{
	BOOL ok;
	ok = DirOK(mnum, md);
	if (ok)
		MonWalkDir(mnum, md);

	return ok;
}

BOOL MonRoundWalk(int mnum, int md, int *dir)
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
	int mx, my;
	int md, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		app_fatal("MAI_Zombie: Invalid monster %d", mnum);
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

	mx = mx - mon->_menemyx;
	my = my - mon->_menemyy;
	md = mon->_mdir;
	v = random_(103, 100);
	if (abs(mx) >= 2 || abs(my) >= 2) {
		if (v < 2 * mon->_mint + 10) {
			if (abs(mx) >= 2 * mon->_mint + 4 || abs(my) >= 2 * mon->_mint + 4) {
				if (random_(104, 100) < 2 * mon->_mint + 20) {
					md = random_(104, 8);
				}
				MonDumbWalk(mnum, md);
			} else {
				md = MonGetDir(mnum);
				MonCallWalk(mnum, md);
			}
		}
	} else if (v < 2 * mon->_mint + 10) {
		MonStartAttack(mnum);
	}

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];
}

void MAI_SkelSd(int mnum)
{
	MonsterStruct *mon;
	int mx, my, x, y, md;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_SkelSd: Invalid monster %d", mnum);

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

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

BOOL MAI_Path(int mnum)
{
	MonsterStruct *mon;
	BOOL clear;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Path: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (mon->MType->mtype != MT_GOLEM) {
		if (mon->_msquelch == 0)
			return FALSE;
		if (mon->_mmode != MM_STAND)
			return FALSE;
		if (mon->_mgoal != MGOAL_NORMAL && mon->_mgoal != MGOAL_MOVE && mon->_mgoal != MGOAL_SHOOT)
			return FALSE;
		if (mon->_mx == 1 && mon->_my == 0)
			return FALSE;
	}

	clear = LineClearF1(
	    PosOkMonst2,
	    mnum,
	    mon->_mx,
	    mon->_my,
	    mon->_menemyx,
	    mon->_menemyy);
	if (!clear || mon->_pathcount >= 5 && mon->_pathcount < 8) {
		if (mon->_mFlags & MFLAG_CAN_OPEN_DOOR)
			MonstCheckDoors(mnum);
		mon->_pathcount++;
		if (mon->_pathcount < 5)
			return FALSE;
		if (MonPathWalk(mnum))
			return TRUE;
	}

	if (mon->MType->mtype != MT_GOLEM)
		mon->_pathcount = 0;

	return FALSE;
}

void MAI_Snake(int mnum)
{
	MonsterStruct *mon;
	int fx, fy, mx, my, md;
	int pnum;
	int tmp;

	if ((DWORD)mnum >= MAXMONSTERS) {
#ifdef HELLFIRE
		return;
#else
		app_fatal("MAI_Snake: Invalid monster %d", mnum);
#endif
	}
	char pattern[6] = { 1, 1, 0, -1, -1, 0 };
	mon = monster + mnum;
	pnum = mon->_menemy;
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
			if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, pnum, mnum, 0, 0) != -1) {
				PlayEffect(mnum, 0);
				dMonster[mon->_mx][mon->_my] = -(mnum + 1);
				mon->_mmode = MM_CHARGE;
			}
		} else if (mon->_mVar1 == MM_DELAY || random_(106, 100) >= 35 - 2 * mon->_mint) {
			if (md + pattern[mon->_mgoalvar1] < 0) {
				tmp = md + pattern[mon->_mgoalvar1] + 8;
			} else {
				tmp = md + pattern[mon->_mgoalvar1] - 8;
				if (md + pattern[mon->_mgoalvar1] < 8)
					tmp = md + pattern[mon->_mgoalvar1];
			}
			mon->_mgoalvar1++;
			if (mon->_mgoalvar1 > 5)
				mon->_mgoalvar1 = 0;
			if (tmp - mon->_mgoalvar2 < 0) {
				md = tmp - mon->_mgoalvar2 + 8;
			} else if (tmp - mon->_mgoalvar2 >= 8) {
				md = tmp - mon->_mgoalvar2 - 8;
			} else
				md = tmp - mon->_mgoalvar2;
			if (md > 0) {
				if (md < 4) {
					if (mon->_mgoalvar2 + 1 < 0) {
						md = mon->_mgoalvar2 + 9;
					} else if (mon->_mgoalvar2 + 1 >= 8) {
						md = mon->_mgoalvar2 - 7;
					} else
						md = mon->_mgoalvar2 + 1;
					mon->_mgoalvar2 = md;
				} else if (md == 4) {
					mon->_mgoalvar2 = tmp;
				} else {
					if (mon->_mgoalvar2 - 1 < 0) {
						md = mon->_mgoalvar2 + 7;
					} else if (mon->_mgoalvar2 - 1 >= 8) {
						md = mon->_mgoalvar2 - 9;
					} else
						md = mon->_mgoalvar2 - 1;
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
	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];
}

void MAI_Bat(int mnum)
{
	MonsterStruct *mon;
	int md, v, pnum;
	int fx, fy, xd, yd;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Bat: Invalid monster %d", mnum);

	mon = &monster[mnum];
	pnum = mon->_menemy;
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	xd = mon->_mx - mon->_menemyx;
	yd = mon->_my - mon->_menemyy;
	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
	mon->_mdir = md;
	v = random_(107, 100);
	if (mon->_mgoal == MGOAL_RETREAT) {
		if (!mon->_mgoalvar1) {
			MonCallWalk(mnum, opposite[md]);
			mon->_mgoalvar1++;
		} else {
			if (random_(108, 2))
				MonCallWalk(mnum, left[md]);
			else
				MonCallWalk(mnum, right[md]);
			mon->_mgoal = MGOAL_NORMAL;
		}
		return;
	}

	fx = mon->_menemyx;
	fy = mon->_menemyy;
	if (mon->MType->mtype == MT_GLOOM
	    && (abs(xd) >= 5 || abs(yd) >= 5)
	    && v < 4 * mon->_mint + 33
	    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
		if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, pnum, mnum, 0, 0) != -1) {
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
		if (mon->MType->mtype == MT_FAMILIAR) {
			AddMissile(mon->_menemyx, mon->_menemyy, mon->_menemyx + 1, 0, -1, MIS_LIGHTNING, 1, mnum, random_(109, 10) + 1, 0);
		}
	}

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_SkelBow(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md, v;
	BOOL walking;

	walking = FALSE;
	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_SkelBow: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;

	md = MonGetDir(mnum);
	mon->_mdir = md;
	v = random_(110, 100);

	if (abs(mx) < 4 && abs(my) < 4) {
		if (mon->_mVar2 > 20 && v < 2 * mon->_mint + 13
		    || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3)
		        && mon->_mVar2 == 0
		        && v < 2 * mon->_mint + 63) {
			walking = MonDumbWalk(mnum, opposite[md]);
		}
	}

	mx = mon->_menemyx;
	my = mon->_menemyy;
	if (!walking) {
		if (random_(110, 100) < 2 * mon->_mint + 3) {
			if (LineClear(mon->_mx, mon->_my, mx, my))
				MonStartRAttack(mnum, MIS_ARROW, 4);
		}
	}

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Fat(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md, v;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Fat: Invalid monster %d", mnum);

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

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Sneak(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md;
	int dist, v;

	if ((DWORD)mnum >= MAXMONSTERS) {
		app_fatal("MAI_Sneak: Invalid monster %d", mnum);
	}

	mon = monster + mnum;
	if (mon->_mmode == MM_STAND) {
		mx = mon->_mx;
		my = mon->_my;
		if (dLight[mx][my] != lightmax) {
			mx -= mon->_menemyx;
			my -= mon->_menemyy;

			md = MonGetDir(mnum);
			dist = 5 - mon->_mint;
			if (mon->_mVar1 == MM_GOTHIT) {
				mon->_mgoalvar1 = 0;
				mon->_mgoal = MGOAL_RETREAT;
			} else {
				if (abs(mx) >= dist + 3 || abs(my) >= dist + 3 || mon->_mgoalvar1 > 8) {
					mon->_mgoalvar1 = 0;
					mon->_mgoal = MGOAL_NORMAL;
				}
			}
			if (mon->_mgoal == MGOAL_RETREAT) {
				if (mon->_mFlags & MFLAG_TARGETS_MONSTER)
					md = GetDirection(mon->_mx, mon->_my, plr[mon->_menemy]._pownerx, plr[mon->_menemy]._pownery);
				md = opposite[md];
				if (mon->MType->mtype == MT_UNSEEN) {
					if (random_(112, 2))
						md = left[md];
					else
						md = right[md];
				}
			}
			mon->_mdir = md;
			v = random_(112, 100);
			if (abs(mx) < dist && abs(my) < dist && mon->_mFlags & MFLAG_HIDDEN) {
				MonStartFadein(mnum, md, FALSE);
			} else {
				if ((abs(mx) >= dist + 1 || abs(my) >= dist + 1) && !(mon->_mFlags & MFLAG_HIDDEN)) {
					MonStartFadeout(mnum, md, TRUE);
				} else {
					if (mon->_mgoal == MGOAL_RETREAT
					    || (abs(mx) >= 2 || abs(my) >= 2) && (mon->_mVar2 > 20 && v < 4 * mon->_mint + 14 || (mon->_mVar1 == MM_WALK || mon->_mVar1 == MM_WALK2 || mon->_mVar1 == MM_WALK3) && mon->_mVar2 == 0 && v < 4 * mon->_mint + 64)) {
						mon->_mgoalvar1++;
						MonCallWalk(mnum, md);
					}
				}
			}
			if (mon->_mmode == MM_STAND) {
				if (abs(mx) >= 2 || abs(my) >= 2 || v >= 4 * mon->_mint + 10)
					mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
				else
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

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Fireman: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND || mon->_msquelch == 0)
		return;

	pnum = monster[mnum]._menemy;
	fx = monster[mnum]._menemyx;
	fy = monster[mnum]._menemyy;
	xd = monster[mnum]._mx - fx;
	yd = monster[mnum]._my - fy;

	md = MonGetDir(mnum);
	if (mon->_mgoal == MGOAL_NORMAL) {
		if (LineClear(mon->_mx, mon->_my, fx, fy)
		    && AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_FIREMAN, pnum, mnum, 0, 0) != -1) {
			mon->_mmode = MM_CHARGE;
			mon->_mgoal = MGOAL_SHOOT;
			mon->_mgoalvar1 = 0;
		}
	} else if (mon->_mgoal == MGOAL_SHOOT) {
		if (mon->_mgoalvar1 == 3) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartFadeout(mnum, md, TRUE);
		} else if (LineClear(mon->_mx, mon->_my, fx, fy)) {
			MonStartRAttack(mnum, MIS_KRULL, 4);
			mon->_mgoalvar1++;
		} else {
			MonStartDelay(mnum, random_(112, 10) + 5);
			mon->_mgoalvar1++;
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		MonStartFadein(mnum, md, FALSE);
		mon->_mgoal = MGOAL_SHOOT;
	}
	mon->_mdir = md;
	random_(112, 100);
	if (mon->_mmode != MM_STAND)
		return;

	if (abs(xd) < 2 && abs(yd) < 2 && mon->_mgoal == MGOAL_NORMAL) {
		MonTryH2HHit(mnum, monster[mnum]._menemy, monster[mnum].mHit, monster[mnum].mMinDamage, monster[mnum].mMaxDamage);
		mon->_mgoal = MGOAL_RETREAT;
		if (!MonCallWalk(mnum, opposite[md])) {
			MonStartFadein(mnum, md, FALSE);
			mon->_mgoal = MGOAL_SHOOT;
		}
	} else if (!MonCallWalk(mnum, md) && (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT)) {
		MonStartFadein(mnum, md, FALSE);
		mon->_mgoal = MGOAL_SHOOT;
	}
}

void MAI_Fallen(int mnum)
{
	int x, y, xpos, ypos;
	int m, rad, md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS) {
		app_fatal("MAI_Fallen: Invalid monster %d", mnum);
	}
	if (monster[mnum]._mgoal == MGOAL_SHOOT) {
		if (monster[mnum]._mgoalvar1)
			monster[mnum]._mgoalvar1--;
		else
			monster[mnum]._mgoal = MGOAL_NORMAL;
	}

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND || monster[mnum]._msquelch == 0) {
		return;
	}

	if (mon->_mgoal == MGOAL_RETREAT) {
		if (!mon->_mgoalvar1--) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartStand(mnum, opposite[mon->_mdir]);
		}
	}

	if (mon->_mAnimFrame == mon->_mAnimLen) {
		if (random_(113, 4)) {
			return;
		}
		if (!(mon->_mFlags & MFLAG_NOHEAL)) {
			MonStartSpStand(mnum, mon->_mdir);
			rad = 2 * mon->_mint + 2;
			if (mon->_mmaxhp - rad >= mon->_mhitpoints)
				mon->_mhitpoints = rad + mon->_mhitpoints;
			else
				mon->_mhitpoints = mon->_mmaxhp;
		}
		rad = 2 * mon->_mint + 4;
		for (y = -rad; y <= rad; y++) {
			for (x = -rad; x <= rad; x++) {
				if (y >= 0 && y < MAXDUNY && x >= 0 && x < MAXDUNX) {
					m = dMonster[x + mon->_mx][y + mon->_my];
					if (m > 0) {
						m--;
						if (monster[m]._mAi == AI_FALLEN) {
							monster[m]._mgoal = MGOAL_SHOOT;
							monster[m]._mgoalvar1 = 30 * mon->_mint + 105;
						}
					}
				}
			}
		}
	} else if (mon->_mgoal == MGOAL_RETREAT) {
		md = mon->_mdir;
		MonCallWalk(mnum, md);
	} else if (mon->_mgoal == MGOAL_SHOOT) {
		xpos = mon->_mx - mon->_menemyx;
		ypos = mon->_my - mon->_menemyy;
		if (abs(xpos) < 2 && abs(ypos) < 2) {
			MonStartAttack(mnum);
		} else {
			md = MonGetDir(mnum);
			MonCallWalk(mnum, md);
		}
	} else {
		MAI_SkelSd(mnum);
	}
}

void MAI_Cleaver(int mnum)
{
	MonsterStruct *mon;
	int x, y, mx, my, md;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Cleaver: Invalid monster %d", mnum);

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

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Round(int mnum, BOOL special)
{
	MonsterStruct *mon;
	int fx, fy;
	int mx, my, md;
	int dist, v;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Round: Invalid monster %d", mnum);
	mon = monster + mnum;
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fy = mon->_menemyy;
		fx = mon->_menemyx;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(114, 100);
		if ((abs(mx) >= 2 || abs(my) >= 2) && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || (abs(mx) >= 4 || abs(my) >= 4) && random_(115, 4) == 0) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(116, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (abs(mx) > abs(my))
					dist = abs(mx);
				else
					dist = abs(my);
				if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md) || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
					MonStartDelay(mnum, random_(125, 10) + 10);
				}
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (abs(mx) >= 2 || abs(my) >= 2) {
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
		if (mon->_mmode == MM_STAND)
			mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
	}
}

void MAI_GoatMc(int mnum)
{
	MAI_Round(mnum, TRUE);
}

void MAI_Ranged(int mnum, int mitype, BOOL special)
{
	int md;
	int fx, fy, mx, my;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MAI_Ranged: Invalid monster %d", mnum);
#endif

	if (monster[mnum]._mmode != MM_STAND) {
		return;
	}

	mon = monster + mnum;
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
					MonStartRSpAttack(mnum, mitype, 4);
				else
					MonStartRAttack(mnum, mitype, 4);
			} else {
				mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
			}
		}
	} else if (mon->_msquelch != 0) {
		fx = mon->_lastx;
		fy = mon->_lasty;
		md = GetDirection(mon->_mx, mon->_my, fx, fy);
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

#ifdef HELLFIRE
void mai_ranged_441649(int mnum)
{
	MAI_Ranged(mnum, MIS_LICH, FALSE);
}

void mai_ranged_441654(int mnum)
{
	MAI_Ranged(mnum, MIS_ARCHLICH, FALSE);
}

void mai_ranged_44165F(int mnum)
{
	MAI_Ranged(mnum, MIS_PSYCHORB, FALSE);
}

void mai_ranged_44166A(int i)
{
	MAI_Ranged(i, MIS_NECROMORB, FALSE);
}
#endif

void MAI_AcidUniq(int mnum)
{
	MAI_Ranged(mnum, MIS_ACID, TRUE);
}

#ifdef HELLFIRE
void mai_ranged_441680(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBOLT, FALSE);
}

void mai_ranged_44168B(int mnum)
{
	MAI_Ranged(mnum, MIS_FIREBALL, FALSE);
}
#endif

void MAI_Scav(int mnum)
{
	BOOL done;
	int x, y;
	int _mx, _my;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Scav: Invalid monster %d", mnum);
	mon = monster + mnum;
	_mx = mon->_mx;
	_my = mon->_my;
	done = FALSE;
	if (monster[mnum]._mmode != MM_STAND)
		return;
	if (mon->_mhitpoints < (mon->_mmaxhp >> 1) && mon->_mgoal != MGOAL_HEALING) {
		if (mon->leaderflag) {
			monster[mon->leader].packsize--;
			mon->leaderflag = 0;
		}
		mon->_mgoal = MGOAL_HEALING;
		mon->_mgoalvar3 = 10;
	}
	if (mon->_mgoal == MGOAL_HEALING && mon->_mgoalvar3 != 0) {
		mon->_mgoalvar3--;
		if (dDead[mon->_mx][mon->_my]) {
			MonStartEat(mnum);
			if (!(mon->_mFlags & MFLAG_NOHEAL))
				mon->_mhitpoints += 64;
			if (mon->_mhitpoints >= (mon->_mmaxhp >> 1) + (mon->_mmaxhp >> 2)) {
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
							if (y < 0 || y >= MAXDUNY || x < 0 || x >= MAXDUNX)
								continue;
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
							if (y < 0 || y >= MAXDUNY || x < 0 || x >= MAXDUNX)
								continue;
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
	int mx, my, dx, dy, md;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Garg: Invalid monster %d", mnum);

	mon = &monster[mnum];
	dx = mon->_mx - mon->_lastx;
	dy = mon->_my - mon->_lasty;
	md = MonGetDir(mnum);
	if (mon->_msquelch != 0 && mon->_mFlags & MFLAG_ALLOW_SPECIAL) {
		MonEnemy(mnum);
		mx = mon->_mx - mon->_menemyx;
		my = mon->_my - mon->_menemyy;
		if (abs(mx) < mon->_mint + 2 && abs(my) < mon->_mint + 2) {
			mon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		}
		return;
	}

	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	if (mon->_mhitpoints<mon->_mmaxhp>> 1 && !(mon->_mFlags & MFLAG_NOHEAL))
		mon->_mgoal = MGOAL_RETREAT;
	if (mon->_mgoal == MGOAL_RETREAT) {
		if (abs(dx) >= mon->_mint + 2 || abs(dy) >= mon->_mint + 2) {
			mon->_mgoal = MGOAL_NORMAL;
			MonStartHeal(mnum);
		} else if (!MonCallWalk(mnum, opposite[md])) {
			mon->_mgoal = MGOAL_NORMAL;
		}
	}
	MAI_Round(mnum, FALSE);
}

void MAI_RoundRanged(int mnum, int mitype, BOOL checkdoors, int dam, int lessmissiles)
{
	MonsterStruct *mon;
	int mx, my;
	int fx, fy;
	int md, dist, v;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_RoundRanged: Invalid monster %d", mnum);
	mon = monster + mnum;
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (checkdoors && mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(121, 10000);
		if ((abs(mx) >= 2 || abs(my) >= 2) && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || ((abs(mx) >= 3 || abs(my) >= 3) && random_(122, 4 << lessmissiles) == 0)) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(123, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (abs(mx) > abs(my)) {
					dist = abs(mx);
				} else {
					dist = abs(my);
				}
				if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md)) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (v<500 * (mon->_mint + 1)>> lessmissiles
				    && (LineClear(mon->_mx, mon->_my, fx, fy))) {
					MonStartRSpAttack(mnum, mitype, dam);
				} else {
					MonRoundWalk(mnum, md, &mon->_mgoalvar2);
				}
			}
		} else {
			mon->_mgoal = MGOAL_NORMAL;
		}
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (((abs(mx) >= 3 || abs(my) >= 3) && v < ((500 * (mon->_mint + 2)) >> lessmissiles)
			        || v < ((500 * (mon->_mint + 1)) >> lessmissiles))
			    && LineClear(mon->_mx, mon->_my, fx, fy)) {
				MonStartRSpAttack(mnum, mitype, dam);
			} else if (abs(mx) >= 2 || abs(my) >= 2) {
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
			MonStartDelay(mnum, random_(125, 10) + 5);
		}
	}
}

void MAI_Magma(int mnum)
{
	MAI_RoundRanged(mnum, MIS_MAGMABALL, TRUE, 4, 0);
}

void MAI_Storm(int mnum)
{
	MAI_RoundRanged(mnum, MIS_LIGHTCTRL2, TRUE, 4, 0);
}

#ifdef HELLFIRE
void mai_roundranged_441EA0(int mnum)
{
	MAI_RoundRanged(mnum, MIS_BONEDEMON, TRUE, 4, 0);
}
#endif

void MAI_Acid(int mnum)
{
	MAI_RoundRanged(mnum, MIS_ACID, FALSE, 4, 1);
}

void MAI_Diablo(int mnum)
{
	MAI_RoundRanged(mnum, MIS_DIABAPOCA, FALSE, 40, 0);
}

void MAI_RR2(int mnum, int mitype, int dam)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int dist, v, md;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_RR2: Invalid monster %d", mnum);

	mon = monster + mnum;
	mx = mon->_mx - mon->_menemyx;
	my = mon->_my - mon->_menemyy;
	if (abs(mx) >= 5 || abs(my) >= 5) {
		MAI_SkelSd(mnum);
		return;
	}

	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(121, 100);
		if ((abs(mx) >= 2 || abs(my) >= 2) && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || (abs(mx) >= 3 || abs(my) >= 3)) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(123, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				mon->_mgoalvar3 = 4;
				if (abs(mx) > abs(my)) {
					dist = abs(mx);
				} else {
					dist = abs(my);
				}
				if (mon->_mgoalvar1++ < 2 * dist || !DirOK(mnum, md)) {
					if (v < 5 * (mon->_mint + 16))
						MonRoundWalk(mnum, md, &mon->_mgoalvar2);
				} else
					mon->_mgoal = MGOAL_NORMAL;
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (((abs(mx) >= 3 || abs(my) >= 3) && v < 5 * (mon->_mint + 2) || v < 5 * (mon->_mint + 1) || mon->_mgoalvar3 == 4) && LineClear(mon->_mx, mon->_my, fx, fy)) {
				MonStartRSpAttack(mnum, mitype, dam);
			} else if (abs(mx) >= 2 || abs(my) >= 2) {
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
						MonStartRSpAttack(mnum, mitype, dam);
				}
			}
			mon->_mgoalvar3 = 1;
		}
		if (mon->_mmode == MM_STAND) {
			MonStartDelay(mnum, random_(125, 10) + 5);
		}
	}
}

void MAI_Mega(int mnum)
{
	MAI_RR2(mnum, MIS_FLAMEC, 0);
}

void MAI_Golum(int mnum)
{
	int mx, my, _mex, _mey;
	int md, j, k, _menemy;
	MonsterStruct *mon;
	BOOL have_enemy, ok;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Golum: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (mon->_mx == 1 && mon->_my == 0) {
		return;
	}

	if (mon->_mmode == MM_DEATH
	    || mon->_mmode == MM_SPSTAND
	    || (mon->_mmode >= MM_WALK && mon->_mmode <= MM_WALK3)) {
		return;
	}

	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER))
		MonEnemy(mnum);

	have_enemy = !(monster[mnum]._mFlags & MFLAG_NO_ENEMY);

	if (mon->_mmode == MM_ATTACK) {
		return;
	}

	_menemy = monster[mnum]._menemy;

	mx = monster[mnum]._mx;
	my = monster[mnum]._my;
	_mex = mx - monster[_menemy]._mfutx;
	_mey = my - monster[_menemy]._mfuty;
	md = GetDirection(mx, my, monster[_menemy]._mx, monster[_menemy]._my);
	monster[mnum]._mdir = md;
	if (abs(_mex) >= 2 || abs(_mey) >= 2) {
		if (have_enemy && MAI_Path(mnum))
			return;
	} else if (have_enemy) {
		_menemy = monster[mnum]._menemy;
		monster[mnum]._menemyx = monster[_menemy]._mx;
		monster[mnum]._menemyy = monster[_menemy]._my;
		if (monster[_menemy]._msquelch == 0) {
			monster[_menemy]._msquelch = UCHAR_MAX;
			monster[monster[mnum]._menemy]._lastx = monster[mnum]._mx;
			monster[monster[mnum]._menemy]._lasty = monster[mnum]._my;
			for (j = 0; j < 5; j++) {
				for (k = 0; k < 5; k++) {
					_menemy = dMonster[monster[mnum]._mx + k - 2][monster[mnum]._my + j - 2];
					if (_menemy > 0)
						monster[_menemy]._msquelch = UCHAR_MAX; // BUGFIX: should be `monster[_menemy-1]`, not monster[_menemy].
				}
			}
		}
		MonStartAttack(mnum);
		return;
	}

	monster[mnum]._pathcount++;
	if (monster[mnum]._pathcount > 8)
		monster[mnum]._pathcount = 5;

	ok = MonCallWalk(mnum, plr[mnum]._pdir);
	if (!ok) {
		md = (md - 1) & 7;
		for (j = 0; j < 8 && !ok; j++) {
			md = (md + 1) & 7;
			ok = DirOK(mnum, md);
		}
		if (!ok) {
			return;
		}
		MonWalkDir(mnum, md);
	}
}

void MAI_SkelKing(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy, nx, ny;
	int dist, v, md;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_SkelKing: Invalid monster %d", mnum);
	mon = monster + mnum;
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(126, 100);
		if ((abs(mx) >= 2 || abs(my) >= 2) && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
			if (mon->_mgoal == MGOAL_MOVE || (abs(mx) >= 3 || abs(my) >= 3) && random_(127, 4) == 0) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(128, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (abs(mx) > abs(my)) {
					dist = abs(mx);
				} else {
					dist = abs(my);
				}
				if (mon->_mgoalvar1++ >= 2 * dist && DirOK(mnum, md) || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
					MonStartDelay(mnum, random_(125, 10) + 10);
				}
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if (gbMaxPlayers == 1
			    && ((abs(mx) >= 3 || abs(my) >= 3) && v < 4 * mon->_mint + 35 || v < 6)
			    && LineClear(mon->_mx, mon->_my, fx, fy)) {
				nx = mon->_mx + offset_x[md];
				ny = mon->_my + offset_y[md];
				if (PosOkMonst(mnum, nx, ny) && nummonsters < MAXMONSTERS) {
					MonSpawnSkel(nx, ny, md);
					MonStartSpStand(mnum, md);
				}
			} else {
				if (abs(mx) >= 2 || abs(my) >= 2) {
					v = random_(129, 100);
					if (v >= mon->_mint + 25
					    && (mon->_mVar1 != MM_WALK && mon->_mVar1 != MM_WALK2 && mon->_mVar1 != MM_WALK3 || mon->_mVar2 != 0 || (v >= mon->_mint + 75))) {
						MonStartDelay(mnum, random_(130, 10) + 10);
					} else {
						MonCallWalk(mnum, md);
					}
				} else if (v < mon->_mint + 20) {
					mon->_mdir = md;
					MonStartAttack(mnum);
				}
			}
		}
		if (mon->_mmode == MM_STAND)
			mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
	}
}

void MAI_Rhino(int mnum)
{
	MonsterStruct *mon;
	int mx, my, fx, fy;
	int v, dist, md;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Rhino: Invalid monster %d", mnum);
	mon = monster + mnum;
	if (mon->_mmode == MM_STAND && mon->_msquelch != 0) {
		fx = mon->_menemyx;
		fy = mon->_menemyy;
		mx = mon->_mx - fx;
		my = mon->_my - fy;
		md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);
		if (mon->_msquelch < UCHAR_MAX)
			MonstCheckDoors(mnum);
		v = random_(131, 100);
		if (abs(mx) >= 2 || abs(my) >= 2) {
			if (mon->_mgoal == MGOAL_MOVE || (abs(mx) >= 5 || abs(my) >= 5) && random_(132, 4) != 0) {
				if (mon->_mgoal != MGOAL_MOVE) {
					mon->_mgoalvar1 = 0;
					mon->_mgoalvar2 = random_(133, 2);
				}
				mon->_mgoal = MGOAL_MOVE;
				if (abs(mx) > abs(my)) {
					dist = abs(mx);
				} else {
					dist = abs(my);
				}
				if (mon->_mgoalvar1++ >= 2 * dist || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
					mon->_mgoal = MGOAL_NORMAL;
				} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
					MonStartDelay(mnum, random_(125, 10) + 10);
				}
			}
		} else
			mon->_mgoal = MGOAL_NORMAL;
		if (mon->_mgoal == MGOAL_NORMAL) {
			if ((abs(mx) >= 5 || abs(my) >= 5)
			    && v < 2 * mon->_mint + 43
			    && LineClearF1(PosOkMonst, mnum, mon->_mx, mon->_my, fx, fy)) {
				if (AddMissile(mon->_mx, mon->_my, fx, fy, md, MIS_RHINO, mon->_menemy, mnum, 0, 0) != -1) {
					if (mon->MData->snd_special)
						PlayEffect(mnum, 3);
					mon->_mmode = MM_CHARGE;
					dMonster[mon->_mx][mon->_my] = -1 - mnum;
				}
			} else {
				if (abs(mx) >= 2 || abs(my) >= 2) {
					v = random_(134, 100);
					if (v >= 2 * mon->_mint + 33
					    && (mon->_mVar1 != MM_WALK && mon->_mVar1 != MM_WALK2 && mon->_mVar1 != MM_WALK3
					           || mon->_mVar2
					           || v >= 2 * mon->_mint + 83)) {
						MonStartDelay(mnum, random_(135, 10) + 10);
					} else {
						MonCallWalk(mnum, md);
					}
				} else if (v < 2 * mon->_mint + 28) {
					mon->_mdir = md;
					MonStartAttack(mnum);
				}
			}
		}
		if (mon->_mmode == MM_STAND)
			mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];
	}
}

#ifdef HELLFIRE
void mai_horkdemon(int mnum)
{
	MonsterStruct *mon;
	int fx, fy, mx, my, md, v, dist;

	if ((DWORD)mnum >= MAXMONSTERS) {
		return;
	}

	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND || mon->_msquelch == 0) {
		return;
	}

	fx = mon->_menemyx;
	fy = mon->_menemyy;
	mx = mon->_mx - fx;
	my = mon->_my - fy;
	md = GetDirection(mon->_mx, mon->_my, mon->_lastx, mon->_lasty);

	if (mon->_msquelch < 255) {
		MonstCheckDoors(mnum);
	}

	v = random_(131, 100);

	if (abs(mx) < 2 && abs(my) < 2) {
		mon->_mgoal = 1;
	} else if (mon->_mgoal == 4 || (abs(mx) >= 5 || abs(my) >= 5) && random_(132, 4) != 0) {
		if (mon->_mgoal != 4) {
			mon->_mgoalvar1 = 0;
			mon->_mgoalvar2 = random_(133, 2);
		}
		mon->_mgoal = 4;
		if (abs(mx) > abs(my)) {
			dist = abs(mx);
		} else {
			dist = abs(my);
		}
		if (mon->_mgoalvar1++ >= 2 * dist || dTransVal[mon->_mx][mon->_my] != dTransVal[fx][fy]) {
			mon->_mgoal = 1;
		} else if (!MonRoundWalk(mnum, md, &mon->_mgoalvar2)) {
			MonStartDelay(mnum, random_(125, 10) + 10);
		}
	}

	if (mon->_mgoal == 1) {
		if ((abs(mx) >= 3 || abs(my) >= 3) && v < 2 * mon->_mint + 43) {
			if (PosOkMonst(mnum, mon->_mx + HorkXAdd[mon->_mdir], mon->_my + HorkYAdd[mon->_mdir]) && nummonsters < MAXMONSTERS) {
				MonStartRSpAttack(mnum, MIS_HORKDMN, 0);
			}
		} else if (abs(mx) < 2 && abs(my) < 2) {
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
				MonStartDelay(mnum, random_(135, 10) + 10);
			}
		}
	}
	if (mon->_mmode == MM_STAND) {
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];
	}
}
#endif

void MAI_Counselor(int mnum)
{
	int mx, my, fx, fy;
	int dist, md, v;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Counselor: Invalid monster %d", mnum);
	if (monster[mnum]._mmode == MM_STAND && monster[mnum]._msquelch != 0) {
		mon = monster + mnum;
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
			if (abs(mx) > abs(my))
				dist = abs(mx);
			else
				dist = abs(my);
			if ((abs(mx) >= 2 || abs(my) >= 2) && mon->_msquelch == UCHAR_MAX && dTransVal[mon->_mx][mon->_my] == dTransVal[fx][fy]) {
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
					MonStartRAttack(mnum, counsmiss[mon->_mint], mon->mMinDamage + random_(77, mon->mMaxDamage - mon->mMinDamage + 1));
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
					MonStartRAttack(mnum, -1, 0);
					AddMissile(mon->_mx, mon->_my, 0, 0, mon->_mdir, MIS_FLASH, 1, mnum, 4, 0);
					AddMissile(mon->_mx, mon->_my, 0, 0, mon->_mdir, MIS_FLASH2, 1, mnum, 4, 0);
				} else
					MonStartDelay(mnum, random_(105, 10) + 2 * (5 - mon->_mint));
			}
		}
		if (mon->_mmode == MM_STAND) {
			MonStartDelay(mnum, random_(125, 10) + 5);
		}
	}
}

void MAI_Garbud(int mnum)
{
	int _mx, _my, md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MAI_Garbud: Invalid monster %d", mnum);
#endif

	mon = &monster[mnum];
	if (mon->_mmode != MM_STAND) {
		return;
	}

	_mx = mon->_mx;
	_my = mon->_my;
	md = MonGetDir(mnum);

	if (mon->mtalkmsg < TEXT_GARBUD4
	    && mon->mtalkmsg > TEXT_DOOM10
	    && !(dFlags[_mx][_my] & BFLAG_VISIBLE)
	    && mon->_mgoal == MGOAL_TALKING) {
		mon->_mgoal = MGOAL_INQUIRING;
		mon->mtalkmsg++;
	}

	if (dFlags[_mx][_my] & BFLAG_VISIBLE) {
		if (mon->mtalkmsg == TEXT_GARBUD4) {
			if (!effect_is_playing(USFX_GARBUD4) && mon->_mgoal == MGOAL_TALKING) {
				mon->_mgoal = MGOAL_NORMAL;
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
			}
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_MOVE)
		MAI_Round(mnum, TRUE);

	monster[mnum]._mdir = md;

	if (mon->_mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Zhar(int mnum)
{
	int mx, my, _mx, _my, md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Zhar: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	md = MonGetDir(mnum);
	if (mon->mtalkmsg == TEXT_ZHAR1 && !(dFlags[mx][my] & BFLAG_VISIBLE) && mon->_mgoal == MGOAL_TALKING) {
		mon->mtalkmsg = TEXT_ZHAR2;
		mon->_mgoal = MGOAL_INQUIRING;
	}

	if (dFlags[mx][my] & BFLAG_VISIBLE) {
		_mx = mon->_mx - mon->_menemyx;
		_my = mon->_my - mon->_menemyy;
		if (abs(_mx) > abs(_my))
			abs(_mx);
		else
			abs(_my);
		if (mon->mtalkmsg == TEXT_ZHAR2) {
			if (!effect_is_playing(USFX_ZHAR2) && mon->_mgoal == MGOAL_TALKING) {
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT || mon->_mgoal == MGOAL_MOVE)
		MAI_Counselor(mnum);

	mon->_mdir = md;

	if (monster[mnum]._mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_SnotSpil(int mnum)
{
	int mx, my, md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_SnotSpil: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND) {
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
			if (!effect_is_playing(USFX_SNOT3) && mon->_mgoal == MGOAL_TALKING) {
				ObjChangeMap(setpc_x, setpc_y, setpc_x + setpc_w + 1, setpc_y + setpc_h + 1);
				quests[Q_LTBANNER]._qvar1 = 3;
				RedoPlayerVision();
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
				mon->_mgoal = MGOAL_NORMAL;
			}
		}
		if (quests[Q_LTBANNER]._qvar1 == 3) {
			if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_SHOOT)
				MAI_Fallen(mnum);
		}
	}

	mon->_mdir = md;

	if (monster[mnum]._mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Lazurus(int mnum)
{
	int mx, my, md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Lazurus: Invalid monster %d", mnum);

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	md = MonGetDir(mnum);
	if (dFlags[mx][my] & BFLAG_VISIBLE) {
		if (gbMaxPlayers == 1) {
			if (mon->mtalkmsg == TEXT_VILE13 && mon->_mgoal == MGOAL_INQUIRING && plr[myplr]._px == TEXT_VILE13 && plr[myplr]._py == 46) {
				PlayInGameMovie("gendata\\fprst3.smk");
				mon->_mmode = MM_TALK;
				quests[Q_BETRAYER]._qvar1 = 5;
			}

			if (mon->mtalkmsg == TEXT_VILE13 && !effect_is_playing(USFX_LAZ1) && mon->_mgoal == MGOAL_TALKING) {
				ObjChangeMapResync(1, 18, 20, 24);
				RedoPlayerVision();
				mon->_msquelch = UCHAR_MAX;
				mon->mtalkmsg = 0;
				quests[Q_BETRAYER]._qvar1 = 6;
				mon->_mgoal = MGOAL_NORMAL;
			}
		}

		if (gbMaxPlayers != 1 && mon->mtalkmsg == TEXT_VILE13 && mon->_mgoal == MGOAL_INQUIRING && quests[Q_BETRAYER]._qvar1 <= 3) {
			mon->_mmode = MM_TALK;
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL || mon->_mgoal == MGOAL_RETREAT || mon->_mgoal == MGOAL_MOVE) {
		mon->mtalkmsg = 0;
		MAI_Counselor(mnum);
	}

	mon->_mdir = md;

	if (monster[mnum]._mmode == MM_STAND || monster[mnum]._mmode == MM_TALK)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Lazhelp(int mnum)
{
	int _mx, _my;
	volatile int md; // BUGFIX: very questionable volatile
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MAI_Lazhelp: Invalid monster %d", mnum);
	if (monster[mnum]._mmode != MM_STAND)
		return;

	mon = monster + mnum;
	_mx = mon->_mx;
	_my = mon->_my;
	md = MonGetDir(mnum);

	if (dFlags[_mx][_my] & BFLAG_VISIBLE) {
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
		MAI_Succ(mnum);
	mon->_mdir = md;
	if (monster[mnum]._mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Lachdanan(int mnum)
{
	int _mx, _my, md;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MAI_Lachdanan: Invalid monster %d", mnum);
#endif

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND) {
		return;
	}

	_mx = mon->_mx;
	_my = mon->_my;
	md = MonGetDir(mnum);

	if (mon->mtalkmsg == TEXT_VEIL9 && !(dFlags[_mx][_my] & BFLAG_VISIBLE) && monster[mnum]._mgoal == MGOAL_TALKING) {
		mon->mtalkmsg = TEXT_VEIL10;
		monster[mnum]._mgoal = MGOAL_INQUIRING;
	}

	if (dFlags[_mx][_my] & BFLAG_VISIBLE) {
		if (mon->mtalkmsg == TEXT_VEIL11) {
			if (!effect_is_playing(USFX_LACH3) && mon->_mgoal == MGOAL_TALKING) {
				mon->mtalkmsg = 0;
				quests[Q_VEIL]._qactive = QUEST_DONE;
				MonStartKill(mnum, -1);
			}
		}
	}

	mon->_mdir = md;

	if (monster[mnum]._mmode == MM_STAND)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[md];
}

void MAI_Warlord(int mnum)
{
	MonsterStruct *mon;
	int mx, my, md;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("MAI_Warlord: Invalid monster %d", mnum);
#endif

	mon = &monster[mnum];
	if (monster[mnum]._mmode != MM_STAND) {
		return;
	}

	mx = mon->_mx;
	my = mon->_my;
	md = MonGetDir(mnum);
	if (dFlags[mx][my] & BFLAG_VISIBLE) {
		if (mon->mtalkmsg == TEXT_WARLRD9 && mon->_mgoal == MGOAL_INQUIRING)
			mon->_mmode = MM_TALK;
		if (mon->mtalkmsg == TEXT_WARLRD9 && !effect_is_playing(USFX_WARLRD1) && mon->_mgoal == MGOAL_TALKING) {
			mon->_msquelch = UCHAR_MAX;
			mon->mtalkmsg = 0;
			mon->_mgoal = MGOAL_NORMAL;
		}
	}

	if (mon->_mgoal == MGOAL_NORMAL)
		MAI_SkelSd(mnum);

	mon->_mdir = md;

	if (monster[mnum]._mmode == MM_STAND || monster[mnum]._mmode == MM_TALK)
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[mon->_mdir];
}

void DeleteMonsterList()
{
	int i;
	for (i = 0; i < MAX_PLRS; i++) {
		if (monster[i]._mDelFlag) {
			monster[i]._mx = 1;
			monster[i]._my = 0;
			monster[i]._mfutx = 0;
			monster[i]._mfuty = 0;
			monster[i]._moldx = 0;
			monster[i]._moldy = 0;
			monster[i]._mDelFlag = FALSE;
		}
	}

	i = MAX_PLRS;
	while (i < nummonsters) {
		if (monster[monstactive[i]]._mDelFlag) {
			DeleteMonster(i);
			i = 0; // TODO: check if this should be MAX_PLRS.
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
		mnum = monstactive[i];
		mon = &monster[mnum];
		raflag = FALSE;
		if (gbMaxPlayers > 1) {
			SetRndSeed(mon->_mAISeed);
			mon->_mAISeed = GetRndSeed();
		}
		if (!(monster[mnum]._mFlags & MFLAG_NOHEAL) && mon->_mhitpoints < mon->_mmaxhp && mon->_mhitpoints >> 6 > 0) {
			if (mon->mLevel > 1) {
				mon->_mhitpoints += mon->mLevel >> 1;
			} else {
				mon->_mhitpoints += mon->mLevel;
			}
		}
		mx = mon->_mx;
		my = mon->_my;

		if (dFlags[mx][my] & BFLAG_VISIBLE && mon->_msquelch == 0) {
			if (mon->MType->mtype == MT_CLEAVER) {
				PlaySFX(USFX_CLEAVER);
			}
#ifdef HELLFIRE
			if (mon->MType->mtype == MT_NAKRUL) {
				if (UseCowFarmer) {
					PlaySFX(USFX_NAKRUL6);
				} else {
					if (IsUberRoomOpened)
						PlaySFX(USFX_NAKRUL4);
					else
						PlaySFX(USFX_NAKRUL5);
				}
			}
			if (mon->MType->mtype == MT_DEFILER)
				PlaySFX(USFX_DEFILER8);
			MonEnemy(mnum);
#endif
		}

		if (mon->_mFlags & MFLAG_TARGETS_MONSTER) {
			_menemy = mon->_menemy;
			if ((DWORD)_menemy >= MAXMONSTERS) {
#ifdef HELLFIRE
				return;
#else
				app_fatal("Illegal enemy monster %d for monster \"%s\"", _menemy, mon->mName);
#endif
			}
			mon->_lastx = monster[mon->_menemy]._mfutx;
			mon->_menemyx = mon->_lastx;
			mon->_lasty = monster[mon->_menemy]._mfuty;
			mon->_menemyy = mon->_lasty;
		} else {
			_menemy = mon->_menemy;
			if ((DWORD)_menemy >= MAX_PLRS) {
#ifdef HELLFIRE
				return;
#else
				app_fatal("Illegal enemy player %d for monster \"%s\"", _menemy, mon->mName);
#endif
			}
			mon->_menemyx = plr[mon->_menemy]._pfutx;
			mon->_menemyy = plr[mon->_menemy]._pfuty;
			if (dFlags[mx][my] & BFLAG_VISIBLE) {
				mon->_msquelch = UCHAR_MAX;
				mon->_lastx = plr[mon->_menemy]._pfutx;
				mon->_lasty = plr[mon->_menemy]._pfuty;
			} else if (mon->_msquelch != 0 && mon->_mAi != MT_DIABLO) { /// BUGFIX: change '_mAi' to 'MType->mtype'
				mon->_msquelch--;
			}
		}
		do {
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
			}
			if (raflag) {
				GroupUnity(mnum);
			}
		} while (raflag);
		if (mon->_mmode != MM_STONE) {
			mon->_mAnimCnt++;
			if (!(mon->_mFlags & MFLAG_ALLOW_SPECIAL) && mon->_mAnimCnt >= mon->_mAnimDelay) {
				mon->_mAnimCnt = 0;
				if (mon->_mFlags & MFLAG_LOCK_ANIMATION) {
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
		for (j = 0; j < 6; j++) {
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
	int mcount, mi;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("DirOK: Invalid monster %d", mnum);
	fx = monster[mnum]._mx + offset_x[mdir];
	fy = monster[mnum]._my + offset_y[mdir];
	if (fy < 0 || fy >= MAXDUNY || fx < 0 || fx >= MAXDUNX || !PosOkMonst(mnum, fx, fy))
		return FALSE;
	if (mdir == DIR_E) {
		if (SolidLoc(fx, fy + 1) || dFlags[fx][fy + 1] & BFLAG_MONSTLR)
			return FALSE;
	}
	if (mdir == DIR_W) {
		if (SolidLoc(fx + 1, fy) || dFlags[fx + 1][fy] & BFLAG_MONSTLR)
			return FALSE;
	}
	if (mdir == DIR_N) {
		if (SolidLoc(fx + 1, fy) || SolidLoc(fx, fy + 1))
			return FALSE;
	}
	if (mdir == DIR_S)
		if (SolidLoc(fx - 1, fy) || SolidLoc(fx, fy - 1))
			return FALSE;
	if (monster[mnum].leaderflag == 1) {
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
			if (y < 0 || y >= MAXDUNY || x < 0 || x >= MAXDUNX)
				continue;
			mi = dMonster[x][y];
			if (mi < 0)
				mi = -mi;
			if (mi != 0)
				mi--;
			// BUGFIX: should only run pack member check if mi was non-zero prior to executing the body of the above if-statement.
			if (monster[mi].leaderflag == 1
			    && monster[mi].leader == mnum
			    && monster[mi]._mfutx == x
			    && monster[mi]._mfuty == y) {
				mcount++;
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
	return nSolidTable[dPiece[x][y]] == FALSE;
}

BOOL LineClearF(BOOL (*Clear)(int, int), int x1, int y1, int x2, int y2)
{
	int xorg, yorg;
	int dx, dy;
	int d;
	int xincD, yincD, dincD, dincH;
	int tmp;

	xorg = x1;
	yorg = y1;
	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) > abs(dy)) {
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
		if (dy > 0) {
			d = 2 * dy - dx;
			dincH = 2 * (dy - dx);
			dincD = 2 * dy;
			yincD = 1;
		} else {
			d = 2 * dy + dx;
			dincH = 2 * (dx + dy);
			dincD = 2 * dy;
			yincD = -1;
		}
		while (x1 != x2 || y1 != y2) {
			if ((d <= 0) ^ (yincD < 0)) {
				d += dincD;
			} else {
				d += dincH;
				y1 += yincD;
			}
			x1++;
			if ((x1 != xorg || y1 != yorg) && !Clear(x1, y1))
				break;
		}
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
		if (dx > 0) {
			d = 2 * dx - dy;
			dincH = 2 * (dx - dy);
			dincD = 2 * dx;
			xincD = 1;
		} else {
			d = 2 * dx + dy;
			dincH = 2 * (dy + dx);
			dincD = 2 * dx;
			xincD = -1;
		}
		while (y1 != y2 || x1 != x2) {
			if ((d <= 0) ^ (xincD < 0)) {
				d += dincD;
			} else {
				d += dincH;
				x1 += xincD;
			}
			y1++;
			if ((y1 != yorg || x1 != xorg) && !Clear(x1, y1))
				break;
		}
	}
	return x1 == x2 && y1 == y2;
}

BOOL LineClear(int x1, int y1, int x2, int y2)
{
	return LineClearF(PosOkMissile, x1, y1, x2, y2);
}

BOOL LineClearF1(BOOL (*Clear)(int, int, int), int monst, int x1, int y1, int x2, int y2)
{
	int xorg, yorg;
	int dx, dy;
	int d;
	int xincD, yincD, dincD, dincH;
	int tmp;

	xorg = x1;
	yorg = y1;
	dx = x2 - x1;
	dy = y2 - y1;
	if (abs(dx) > abs(dy)) {
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
		if (dy > 0) {
			d = 2 * dy - dx;
			dincH = 2 * (dy - dx);
			dincD = 2 * dy;
			yincD = 1;
		} else {
			d = 2 * dy + dx;
			dincH = 2 * (dx + dy);
			dincD = 2 * dy;
			yincD = -1;
		}
		while (x1 != x2 || y1 != y2) {
			if ((d <= 0) ^ (yincD < 0)) {
				d += dincD;
			} else {
				d += dincH;
				y1 += yincD;
			}
			x1++;
			if ((x1 != xorg || y1 != yorg) && !Clear(monst, x1, y1))
				break;
		}
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
		if (dx > 0) {
			d = 2 * dx - dy;
			dincH = 2 * (dx - dy);
			dincD = 2 * dx;
			xincD = 1;
		} else {
			d = 2 * dx + dy;
			dincH = 2 * (dy + dx);
			dincD = 2 * dx;
			xincD = -1;
		}
		while (y1 != y2 || x1 != x2) {
			if ((d <= 0) ^ (xincD < 0)) {
				d += dincD;
			} else {
				d += dincH;
				x1 += xincD;
			}
			y1++;
			if ((y1 != yorg || x1 != xorg) && !Clear(monst, x1, y1))
				break;
		}
	}
	return x1 == x2 && y1 == y2;
}

void SyncMonsterAnim(int mnum)
{
	MonsterData *MData;
	int _mdir;
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("SyncMonsterAnim: Invalid monster %d", mnum);
	mon = &monster[mnum];
	mon->MType = Monsters + mon->_mMTidx;
	MData = Monsters[mon->_mMTidx].MData;
	mon->MData = MData;
	if (mon->_uniqtype != 0)
		mon->mName = UniqMonst[mon->_uniqtype - 1].mName;
	else
		mon->mName = MData->mName;
	_mdir = mon->_mdir;

	switch (mon->_mmode) {
	case MM_WALK:
	case MM_WALK2:
	case MM_WALK3:
		mon->_mAnimData = mon->MType->Anims[MA_WALK].Data[_mdir];
		return;
	case MM_ATTACK:
	case MM_RATTACK:
		mon->_mAnimData = mon->MType->Anims[MA_ATTACK].Data[_mdir];
		return;
	case MM_GOTHIT:
		mon->_mAnimData = mon->MType->Anims[MA_GOTHIT].Data[_mdir];
		return;
	case MM_DEATH:
		mon->_mAnimData = mon->MType->Anims[MA_DEATH].Data[_mdir];
		return;
	case MM_SATTACK:
	case MM_FADEIN:
	case MM_FADEOUT:
		mon->_mAnimData = mon->MType->Anims[MA_SPECIAL].Data[_mdir];
		return;
	case MM_SPSTAND:
	case MM_RSPATTACK:
		mon->_mAnimData = mon->MType->Anims[MA_SPECIAL].Data[_mdir];
		return;
	case MM_HEAL:
		mon->_mAnimData = mon->MType->Anims[MA_SPECIAL].Data[_mdir];
		return;
	case MM_STAND:
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[_mdir];
		return;
	case MM_DELAY:
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[_mdir];
		return;
	case MM_TALK:
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[_mdir];
		return;
	case MM_CHARGE:
		mon->_mAnimData = mon->MType->Anims[MA_ATTACK].Data[_mdir];
		mon->_mAnimFrame = 1;
		mon->_mAnimLen = mon->MType->Anims[MA_ATTACK].Frames;
		break;
	default:
		mon->_mAnimData = mon->MType->Anims[MA_STAND].Data[_mdir];
		mon->_mAnimFrame = 1;
		mon->_mAnimLen = mon->MType->Anims[MA_STAND].Frames;
		break;
	}
}

void MonFallenFear(int x, int y)
{
	MonsterStruct *mon;
	int i, rundist, aitype;

	for (i = 0; i < nummonsters; i++) {
		rundist = 0;
		mon = &monster[monstactive[i]];
		switch (mon->MType->mtype) {
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
		}
		aitype = mon->_mAi;
		if (aitype == AI_FALLEN
		    && rundist
		    && abs(x - mon->_mx) < 5
		    && abs(y - mon->_my) < 5
		    && mon->_mhitpoints >> 6 > 0) {
			mon->_mgoal = MGOAL_RETREAT;
			mon->_mgoalvar1 = rundist;
			mon->_mdir = GetDirection(x, y, monster[i]._mx, monster[i]._my);
		}
	}
}

void PrintMonstHistory(int mt)
{
	int minHP, maxHP, res;

	sprintf(tempstr, "Total kills: %i", monstkills[mt]);
	AddPanelString(tempstr, TRUE);
	if (monstkills[mt] >= 30) {
		minHP = monsterdata[mt].mMinHP;
		maxHP = monsterdata[mt].mMaxHP;
		if (gbMaxPlayers == 1) {
			minHP = monsterdata[mt].mMinHP >> 1;
			maxHP = monsterdata[mt].mMaxHP >> 1;
		}
		if (minHP < 1)
			minHP = 1;
		if (maxHP < 1)
			maxHP = 1;
		if (gnDifficulty == DIFF_NIGHTMARE) {
			minHP = 3 * minHP + 1;
			maxHP = 3 * maxHP + 1;
		}
		if (gnDifficulty == DIFF_HELL) {
			minHP = 4 * minHP + 3;
			maxHP = 4 * maxHP + 3;
		}
		sprintf(tempstr, "Hit Points: %i-%i", minHP, maxHP);
		AddPanelString(tempstr, TRUE);
	}
	if (monstkills[mt] >= 15) {
		if (gnDifficulty != DIFF_HELL)
			res = monsterdata[mt].mMagicRes;
		else
			res = monsterdata[mt].mMagicRes2;
		res = res & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING | IMUNE_MAGIC | IMUNE_FIRE | IMUNE_LIGHTNING);
		if (!res) {
			strcpy(tempstr, "No magic resistance");
			AddPanelString(tempstr, TRUE);
		} else {
			if (res & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING)) {
				strcpy(tempstr, "Resists: ");
				if (res & RESIST_MAGIC)
					strcat(tempstr, "Magic ");
				if (res & RESIST_FIRE)
					strcat(tempstr, "Fire ");
				if (res & RESIST_LIGHTNING)
					strcat(tempstr, "Lightning ");
				tempstr[strlen(tempstr) - 1] = '\0';
				AddPanelString(tempstr, TRUE);
			}
			if (res & (IMUNE_MAGIC | IMUNE_FIRE | IMUNE_LIGHTNING)) {
				strcpy(tempstr, "Immune: ");
				if (res & IMUNE_MAGIC)
					strcat(tempstr, "Magic ");
				if (res & IMUNE_FIRE)
					strcat(tempstr, "Fire ");
				if (res & IMUNE_LIGHTNING)
					strcat(tempstr, "Lightning ");
				tempstr[strlen(tempstr) - 1] = '\0';
				AddPanelString(tempstr, TRUE);
			}
		}
	}
	pinfoflag = TRUE;
}

void PrintUniqueHistory()
{
	int res;

	res = monster[pcursmonst].mMagicRes & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING | IMUNE_MAGIC | IMUNE_FIRE | IMUNE_LIGHTNING);
	if (!res) {
		strcpy(tempstr, "No resistances");
		AddPanelString(tempstr, TRUE);
		strcpy(tempstr, "No Immunities");
	} else {
		if (res & (RESIST_MAGIC | RESIST_FIRE | RESIST_LIGHTNING))
			strcpy(tempstr, "Some Magic Resistances");
		else
			strcpy(tempstr, "No resistances");
		AddPanelString(tempstr, TRUE);
		if (res & (IMUNE_MAGIC | IMUNE_FIRE | IMUNE_LIGHTNING)) {
			strcpy(tempstr, "Some Magic Immunities");
		} else {
			strcpy(tempstr, "No Immunities");
		}
	}
	AddPanelString(tempstr, TRUE);
	pinfoflag = TRUE;
}

void MissToMonst(int mi, int x, int y)
{
	int oldx, oldy;
	int newx, newy;
	int mnum, pnum;
	MissileStruct *mis;
	MonsterStruct *mon;

	if ((DWORD)mi >= MAXMISSILES)
		app_fatal("MissToMonst: Invalid missile %d", mi);

	mis = &missile[mi];
	mnum = mis->_misource;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("MissToMonst: Invalid monster %d", mnum);

	mon = &monster[mnum];
	oldx = mis->_mix;
	oldy = mis->_miy;
	dMonster[x][y] = mnum + 1;
	mon->_mdir = mis->_mimfnum;
	mon->_mx = x;
	mon->_my = y;
	MonStartStand(mnum, mon->_mdir);
	if (mon->MType->mtype < MT_INCIN || mon->MType->mtype > MT_HELLBURN) {
		if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER))
			MonStartHit(mnum, -1, 0);
		else
			M2MStartHit(mnum, -1, 0);
	} else {
		MonStartFadein(mnum, mon->_mdir, FALSE);
	}

	if (!(mon->_mFlags & MFLAG_TARGETS_MONSTER)) {
		pnum = dPlayer[oldx][oldy] - 1;
		if (dPlayer[oldx][oldy] > 0) {
			if (mon->MType->mtype != MT_GLOOM && (mon->MType->mtype < MT_INCIN || mon->MType->mtype > MT_HELLBURN)) {
				MonTryH2HHit(mnum, dPlayer[oldx][oldy] - 1, 500, mon->mMinDamage2, mon->mMaxDamage2);
				if (pnum == dPlayer[oldx][oldy] - 1 && (mon->MType->mtype < MT_NSNAKE || mon->MType->mtype > MT_GSNAKE)) {
					if (plr[pnum]._pmode != PM_GOTHIT && plr[pnum]._pmode != PM_DEATH)
						StartPlrHit(pnum, 0, TRUE);
					newx = oldx + offset_x[mon->_mdir];
					newy = oldy + offset_y[mon->_mdir];
					if (PosOkPlayer(pnum, newx, newy)) {
						plr[pnum]._px = newx;
						plr[pnum]._py = newy;
						FixPlayerLocation(pnum, plr[pnum]._pdir);
						FixPlrWalkTags(pnum);
						dPlayer[newx][newy] = pnum + 1;
						SetPlayerOld(pnum);
					}
				}
			}
		}
	} else {
		if (dMonster[oldx][oldy] > 0) {
			if (mon->MType->mtype != MT_GLOOM && (mon->MType->mtype < MT_INCIN || mon->MType->mtype > MT_HELLBURN)) {
				MonTryM2MHit(mnum, dMonster[oldx][oldy] - 1, 500, mon->mMinDamage2, mon->mMaxDamage2);
				if (mon->MType->mtype < MT_NSNAKE || mon->MType->mtype > MT_GSNAKE) {
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

BOOL PosOkMonst(int mnum, int x, int y)
{
#ifdef HELLFIRE
	int oi;
	BOOL ret;

	ret = !SolidLoc(x, y) && !dPlayer[x][y] && !dMonster[x][y];
	oi = dObject[x][y];
	if (ret && oi) {
		oi = oi > 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			ret = FALSE;
	}

	if (ret)
		ret = monster_posok(mnum, x, y);
#else
	int oi, mi, j;
	BOOL ret, fire;

	fire = FALSE;
	ret = !SolidLoc(x, y) && !dPlayer[x][y] && !dMonster[x][y];
	if (ret && dObject[x][y]) {
		oi = dObject[x][y] > 0 ? dObject[x][y] - 1 : -(dObject[x][y] + 1);
		if (object[oi]._oSolidFlag)
			ret = FALSE;
	}

	if (ret && dMissile[x][y] && mnum >= 0) {
		mi = dMissile[x][y];
		if (mi > 0) {
			if (missile[mi - 1]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
				fire = TRUE;
			} else {
				for (j = 0; j < nummissiles; j++) {
					if (missile[missileactive[j]]._mitype == MIS_FIREWALL)
						fire = TRUE;
				}
			}
		}
		if (fire && (!(monster[mnum].mMagicRes & IMUNE_FIRE) || monster[mnum].MType->mtype == MT_DIABLO))
			ret = FALSE;
	}
#endif

	return ret;
}

#ifdef HELLFIRE
BOOLEAN monster_posok(int mnum, int x, int y)
{
	MissileStruct* mis;
	int mi, j;
	BOOLEAN ret, fire, lightning;

	ret = TRUE;
	mi = dMissile[x][y];
	if (mi && mnum >= 0) {
		fire = FALSE;
		lightning = FALSE;
		if (mi > 0) {
			if (missile[mi - 1]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
				fire = TRUE;
			} else if (missile[mi - 1]._mitype == MIS_LIGHTWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
				lightning = TRUE;
			}
		} else {
			for (j = 0; j < nummissiles; j++) {
				mis = &missile[missileactive[j]];
				if (mis->_mix == x && mis->_miy == y) {
					if (mis->_mitype == MIS_FIREWALL) {
						fire = TRUE;
						break;
					}
					if (mis->_mitype == MIS_LIGHTWALL) {
						lightning = TRUE;
						break;
					}
				}
			}
		}
		if ((fire && !(monster[mnum].mMagicRes & IMUNE_FIRE)) || (fire && monster[mnum].MType->mtype == MT_DIABLO))
			ret = FALSE;
		if ((lightning && !(monster[mnum].mMagicRes & IMUNE_LIGHTNING)) || (lightning && monster[mnum].MType->mtype == MT_DIABLO))
			ret = FALSE;
	}
	return ret;
}
#endif

BOOL PosOkMonst2(int mnum, int x, int y)
{
	int oi, mi, j;
#ifdef HELLFIRE
	BOOL ret;

	oi = dObject[x][y];
	ret = !SolidLoc(x, y);
	if (ret && oi) {
		oi = oi > 0 ? oi - 1 : -(oi + 1);
		if (object[oi]._oSolidFlag)
			ret = FALSE;
	}

	if (ret)
		ret = monster_posok(mnum, x, y);
#else
	BOOL ret, fire;

	fire = FALSE;
	ret = !SolidLoc(x, y);
	if (ret && dObject[x][y]) {
		oi = dObject[x][y] > 0 ? dObject[x][y] - 1 : -(dObject[x][y] + 1);
		if (object[oi]._oSolidFlag)
			ret = FALSE;
	}

	if (ret && dMissile[x][y] && mnum >= 0) {
		mi = dMissile[x][y];
		if (mi > 0) {
			if (missile[mi - 1]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
				fire = TRUE;
			} else {
				for (j = 0; j < nummissiles; j++) {
					if (missile[missileactive[j]]._mitype == MIS_FIREWALL)
						fire = TRUE;
				}
			}
		}
		if (fire && (!(monster[mnum].mMagicRes & IMUNE_FIRE) || monster[mnum].MType->mtype == MT_DIABLO))
			ret = FALSE;
	}
#endif

	return ret;
}

BOOL PosOkMonst3(int mnum, int x, int y)
{
	int j, oi, objtype, mi;
#ifdef HELLFIRE
	BOOL ret;
	DIABOOL isdoor;

	ret = TRUE;
	isdoor = FALSE;

	oi = dObject[x][y];
	if (ret && oi != 0) {
		oi = oi > 0 ? oi - 1 : -(oi + 1);
		objtype = object[oi]._otype;
		isdoor = objtype == OBJ_L1LDOOR || objtype == OBJ_L1RDOOR
		    || objtype == OBJ_L2LDOOR || objtype == OBJ_L2RDOOR
		    || objtype == OBJ_L3LDOOR || objtype == OBJ_L3RDOOR;
		if (object[oi]._oSolidFlag && !isdoor) {
			ret = FALSE;
		}
	}
	if (ret) {
		ret = (!SolidLoc(x, y) || isdoor) && dPlayer[x][y] == 0 && dMonster[x][y] == 0;
	}
	if (ret)
		ret = monster_posok(mnum, x, y);
#else
	BOOL ret, fire, isdoor;

	fire = FALSE;
	ret = TRUE;
	isdoor = FALSE;

	if (ret && dObject[x][y] != 0) {
		oi = dObject[x][y] > 0 ? dObject[x][y] - 1 : -(dObject[x][y] + 1);
		objtype = object[oi]._otype;
		isdoor = objtype == OBJ_L1LDOOR || objtype == OBJ_L1RDOOR
		    || objtype == OBJ_L2LDOOR || objtype == OBJ_L2RDOOR
		    || objtype == OBJ_L3LDOOR || objtype == OBJ_L3RDOOR;
		if (object[oi]._oSolidFlag && !isdoor) {
			ret = FALSE;
		}
	}
	if (ret) {
		ret = (!SolidLoc(x, y) || isdoor) && dPlayer[x][y] == 0 && dMonster[x][y] == 0;
	}
	if (ret && dMissile[x][y] != 0 && mnum >= 0) {
		mi = dMissile[x][y];
		if (mi > 0) {
			if (missile[mi - 1]._mitype == MIS_FIREWALL) { // BUGFIX: Change 'mi' to 'mi - 1' (fixed)
				fire = TRUE;
			} else {
				for (j = 0; j < nummissiles; j++) {
					if (missile[missileactive[j]]._mitype == MIS_FIREWALL) {
						fire = TRUE;
					}
				}
			}
		}
		if (fire && (!(monster[mnum].mMagicRes & IMUNE_FIRE) || monster[mnum].MType->mtype == MT_DIABLO)) {
			ret = FALSE;
		}
	}
#endif

	return ret;
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

	if (j) {
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

void ActivateSpawn(int mnum, int x, int y, int dir)
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

	rs = random_(137, 15) + 1;
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

	if (j) {
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
	int pnum, itm;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return;
#else
		app_fatal("TalktoMonster: Invalid monster %d", mnum);
#endif

	mon = &monster[mnum];
	pnum = mon->_menemy;
	mon->_mmode = MM_TALK;
	if (mon->_mAi == AI_SNOTSPIL || mon->_mAi == AI_LACHDAN) {
		if (QuestStatus(Q_LTBANNER) && quests[Q_LTBANNER]._qvar1 == 2 && PlrHasItem(pnum, IDI_BANNER, &itm)) {
			RemoveInvItem(pnum, itm);
			quests[Q_LTBANNER]._qactive = QUEST_DONE;
			mon->mtalkmsg = TEXT_BANNER12;
			mon->_mgoal = MGOAL_INQUIRING;
		}
		if (QuestStatus(Q_VEIL) && mon->mtalkmsg >= TEXT_VEIL9) {
			if (PlrHasItem(pnum, IDI_GLDNELIX, &itm)) {
				RemoveInvItem(pnum, itm);
				mon->mtalkmsg = TEXT_VEIL11;
				mon->_mgoal = MGOAL_INQUIRING;
			}
		}
	}
}

void SpawnGolum(int mnum, int x, int y, int mi)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
		app_fatal("SpawnGolum: Invalid monster %d", mnum);
	dMonster[x][y] = mnum + 1;
	mon = &monster[mnum];
	mon->_mx = x;
	mon->_my = y;
	mon->_mfutx = x;
	mon->_mfuty = y;
	mon->_moldx = x;
	mon->_moldy = y;
	mon->_pathcount = 0;
	mon->_mFlags |= MFLAG_GOLEM;
	mon->mArmorClass = 25;
	mon->_mmaxhp = 2 * (320 * missile[mi]._mispllvl + plr[mnum]._pMaxMana / 3);
	mon->_mhitpoints = mon->_mmaxhp;
	mon->mHit = 5 * (missile[mi]._mispllvl + 8) + 2 * plr[mnum]._pLevel;
	mon->mMinDamage = 2 * (missile[mi]._mispllvl + 4);
	mon->mMaxDamage = 2 * (missile[mi]._mispllvl + 8);
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
	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return FALSE;
#else
		app_fatal("CanTalkToMonst: Invalid monster %d", mnum);
#endif

	if (monster[mnum]._mgoal == MGOAL_INQUIRING) {
		return TRUE;
	}

	return monster[mnum]._mgoal == MGOAL_TALKING;
}

BOOL CheckMonsterHit(int mnum, BOOL *ret)
{
	MonsterStruct *mon;

	if ((DWORD)mnum >= MAXMONSTERS)
#ifdef HELLFIRE
		return FALSE;
#else
		app_fatal("CheckMonsterHit: Invalid monster %d", mnum);
#endif
	mon = &monster[mnum];
	if (mon->_mAi == AI_GARG && mon->_mFlags & MFLAG_ALLOW_SPECIAL) {
		mon->_mFlags &= ~MFLAG_ALLOW_SPECIAL;
		mon->_mmode = MM_SATTACK;
		*ret = TRUE;
		return TRUE;
	}

	if (mon->MType->mtype >= MT_COUNSLR && mon->MType->mtype <= MT_ADVOCATE) {
		if (mon->_mgoal != MGOAL_NORMAL) {
			*ret = FALSE;
			return TRUE;
		}
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
