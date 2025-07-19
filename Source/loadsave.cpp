/**
 * @file loadsave.cpp
 *
 * Implementation of save game functionality.
 */
#include "all.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
#define SAVE_INITIAL 'HELF'
#else
#define SAVE_INITIAL 'RETL'
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
#define LE_LOAD_INTS(dest, src, n)                \
	{                                             \
		LE_INT32* ssi = (LE_INT32*)src;           \
		int* dst = dest;                          \
		for (i = 0; i < (n); i++, ssi++, dst++) { \
			*dest = *ssi;                         \
		}                                         \
	}
#define LE_SAVE_INTS(dest, src, n)                    \
	{                                                 \
		LE_INT32* dst = (LE_INT32*)dest;              \
		int* ssi = src;                               \
		for (int i = 0; i < (n); i++, ssi++, dst++) { \
			*dst = *ssi;                              \
		}                                             \
	}
#else
#define LE_LOAD_INTS(dest, src, n) \
	memcpy(dest, src, (n) * sizeof(int));
#define LE_SAVE_INTS(dest, src, n) \
	memcpy(dest, src, (n) * sizeof(int));
#endif

static BYTE* LoadItem(BYTE* DVL_RESTRICT src, ItemStruct* DVL_RESTRICT is)
{
	LSaveItemStruct* DVL_RESTRICT savedItem = (LSaveItemStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	is->_iSeed = savedItem->viSeed;
	is->_iIdx = savedItem->viIdx;
	is->_iCreateInfo = savedItem->viCreateInfo;
	is->_ix = savedItem->vix;
	is->_iy = savedItem->viy;
	is->_iCurs = savedItem->viCurs;
	is->_itype = savedItem->vitype;
	is->_iMiscId = savedItem->viMiscId;
	is->_iSpell = savedItem->viSpell;

	is->_iClass = savedItem->viClass;
	is->_iLoc = savedItem->viLoc;
	is->_iDamType = savedItem->viDamType;
	is->_iMinDam = savedItem->viMinDam;

	is->_iMaxDam = savedItem->viMaxDam;
	is->_iBaseCrit = savedItem->viBaseCrit;
	is->_iMinStr = savedItem->viMinStr;
	is->_iMinMag = savedItem->viMinMag;

	is->_iMinDex = savedItem->viMinDex;
	is->_iUsable = savedItem->viUsable;
	is->_iPrePower = savedItem->viPrePower;
	is->_iSufPower = savedItem->viSufPower;

	is->_iMagical = savedItem->viMagical;
	is->_iSelFlag = savedItem->viSelFlag;
	is->_iFloorFlag = savedItem->viFloorFlag;
	is->_iAnimFlag = savedItem->viAnimFlag;

	// is->_iAnimData = savedItem->viAnimDataAlign
	// is->_iAnimFrameLen = savedItem->viAnimFrameLenAlign
	is->_iAnimCnt = savedItem->viAnimCnt;
	is->_iAnimLen = savedItem->viAnimLen;
	is->_iAnimFrame = savedItem->viAnimFrame;
	is->_iPostDraw = savedItem->viPostDraw;
	is->_iIdentified = savedItem->viIdentified;
	memcpy(is->_iName, savedItem->viName, lengthof(is->_iName));
	is->_ivalue = savedItem->vivalue;
	is->_iIvalue = savedItem->viIvalue;
	is->_iAC = savedItem->viAC;
	is->_iPLFlags = savedItem->viPLFlags;
	is->_iCharges = savedItem->viCharges;
	is->_iMaxCharges = savedItem->viMaxCharges;
	is->_iDurability = savedItem->viDurability;
	is->_iMaxDur = savedItem->viMaxDur;
	is->_iPLDam = savedItem->viPLDam;
	is->_iPLToHit = savedItem->viPLToHit;
	is->_iPLAC = savedItem->viPLAC;
	is->_iPLStr = savedItem->viPLStr;
	is->_iPLMag = savedItem->viPLMag;
	is->_iPLDex = savedItem->viPLDex;
	is->_iPLVit = savedItem->viPLVit;
	is->_iPLFR = savedItem->viPLFR;
	is->_iPLLR = savedItem->viPLLR;
	is->_iPLMR = savedItem->viPLMR;
	is->_iPLAR = savedItem->viPLAR;
	is->_iPLMana = savedItem->viPLMana;
	is->_iPLHP = savedItem->viPLHP;
	is->_iPLDamMod = savedItem->viPLDamMod;

	is->_iPLToBlk = savedItem->viPLToBlk;
	is->_iPLAtkSpdMod = savedItem->viPLAtkSpdMod;
	is->_iPLAbsAnyHit = savedItem->viPLAbsAnyHit;
	is->_iPLAbsPhyHit = savedItem->viPLAbsPhyHit;

	is->_iPLLight = savedItem->viPLLight;
	is->_iPLSkillLevels = savedItem->viPLSkillLevels;
	is->_iPLSkill = savedItem->viPLSkill;
	is->_iPLSkillLvl = savedItem->viPLSkillLvl;

	is->_iPLManaSteal = savedItem->viPLManaSteal;
	is->_iPLLifeSteal = savedItem->viPLLifeSteal;
	is->_iPLCrit = savedItem->viPLCrit;
	is->_iStatFlag = savedItem->viStatFlag;

	is->_iUid = savedItem->viUid;

	is->_iPLFMinDam = savedItem->viPLFMinDam;
	is->_iPLFMaxDam = savedItem->viPLFMaxDam;
	is->_iPLLMinDam = savedItem->viPLLMinDam;
	is->_iPLLMaxDam = savedItem->viPLLMaxDam;

	is->_iPLMMinDam = savedItem->viPLMMinDam;
	is->_iPLMMaxDam = savedItem->viPLMMaxDam;
	is->_iPLAMinDam = savedItem->viPLAMinDam;
	is->_iPLAMaxDam = savedItem->viPLAMaxDam;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveItemStruct, viAnimDataAlign) == offsetof(ItemStruct, _iAnimData), "LoadItem uses memcpy to load the LSaveItemStruct in ItemStruct I.");
	memcpy(is, savedItem, offsetof(ItemStruct, _iAnimData));
	static_assert(sizeof(LSaveItemStruct) - offsetof(LSaveItemStruct, viAnimCnt) == sizeof(ItemStruct) - offsetof(ItemStruct, _iAnimCnt) - sizeof(is->alignment), "LoadItem uses memcpy to load the LSaveItemStruct in ItemStruct II.");
	memcpy(&is->_iAnimCnt, &savedItem->viAnimCnt, sizeof(LSaveItemStruct) - offsetof(LSaveItemStruct, viAnimCnt));
#else
	static_assert(sizeof(LSaveItemStruct) == offsetof(LSaveItemStruct, viPLAMaxDam) + sizeof(savedItem->viPLAMaxDam)
	 && offsetof(ItemStruct, _iPLAMaxDam) == offsetof(LSaveItemStruct, viPLAMaxDam), "LoadItem uses memcpy to load the LSaveItemStruct in ItemStruct.");
	memcpy(is, savedItem, sizeof(LSaveItemStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	src += sizeof(LSaveItemStruct);

	return src;
}

static BYTE* LoadPlayer(BYTE* DVL_RESTRICT src, int pnum)
{
	PlayerStruct* DVL_RESTRICT pr = &players[pnum];

	LSavePlayerStruct* DVL_RESTRICT savedPlr = (LSavePlayerStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	pr->_pmode = savedPlr->vpmode;
	pr->_pDestAction = savedPlr->vpDestAction;
	pr->_pDestParam1 = savedPlr->vpDestParam1;
	pr->_pDestParam2 = savedPlr->vpDestParam2;
	pr->_pDestParam3 = savedPlr->vpDestParam3;
	pr->_pDestParam4 = savedPlr->vpDestParam4;

	pr->_pActive = savedPlr->vpActive;
	pr->_pInvincible = savedPlr->vpInvincible;
	pr->_pLvlChanging = savedPlr->vpLvlChanging;
	pr->_pDunLevel = savedPlr->vpDunLevel;

	pr->_pClass = savedPlr->vpClass;
	pr->_pLevel = savedPlr->vpLevel;
	pr->_pRank = savedPlr->vpRank;
	pr->_pTeam = savedPlr->vpTeam;

	pr->_pStatPts = savedPlr->vpStatPts;
	pr->_pLightRad = savedPlr->vpLightRad;
	pr->_pManaShield = savedPlr->vpManaShield;

	static_assert(NUM_PLRTIMERS == 2, "LoadPlayer copies a fixed amount of timers.");
	pr->_pTimer[PLTR_INFRAVISION] = savedPlr->vpTimer[PLTR_INFRAVISION];
	pr->_pTimer[PLTR_RAGE] = savedPlr->vpTimer[PLTR_RAGE];

	pr->_pExperience = savedPlr->vpExperience;
	pr->_pNextExper = savedPlr->vpNextExper;
	pr->_px = savedPlr->vpx;
	pr->_py = savedPlr->vpy;
	pr->_pfutx = savedPlr->vpfutx;
	pr->_pfuty = savedPlr->vpfuty;
	pr->_poldx = savedPlr->vpoldx;
	pr->_poldy = savedPlr->vpoldy;
	pr->_pxoff = savedPlr->vpxoff;
	pr->_pyoff = savedPlr->vpyoff;
	pr->_pdir = savedPlr->vpdir;
	// savedPlr->vpAnimDataAlign = pr->_pAnimData;
	// savedPlr->vpAnimFrameLenAlign = pr->_pAnimFrameLen;
	pr->_pAnimCnt = savedPlr->vpAnimCnt;
	pr->_pAnimLen = savedPlr->vpAnimLenAlign;
	pr->_pAnimFrame = savedPlr->vpAnimFrame;
	pr->_pAnimWidth = savedPlr->vpAnimWidthAlign;
	pr->_pAnimXOffset = savedPlr->vpAnimXOffsetAlign;
	pr->_plid = savedPlr->vplid;
	pr->_pvid = savedPlr->vpvid;

	pr->_pAtkSkill = savedPlr->vpAtkSkill;
	pr->_pAtkSkillType = savedPlr->vpAtkSkillType;
	pr->_pMoveSkill = savedPlr->vpMoveSkill;
	pr->_pMoveSkillType = savedPlr->vpMoveSkillType;

	pr->_pAltAtkSkill = savedPlr->vpAltAtkSkill;
	pr->_pAltAtkSkillType = savedPlr->vpAltAtkSkillType;
	pr->_pAltMoveSkill = savedPlr->vpAltMoveSkill;
	pr->_pAltMoveSkillType = savedPlr->vpAltMoveSkillType;

	memcpy(pr->_pAtkSkillHotKey, savedPlr->vpAtkSkillHotKey, lengthof(pr->_pAtkSkillHotKey));
	memcpy(pr->_pAtkSkillTypeHotKey, savedPlr->vpAtkSkillTypeHotKey, lengthof(pr->_pAtkSkillTypeHotKey));
	memcpy(pr->_pMoveSkillHotKey, savedPlr->vpMoveSkillHotKey, lengthof(pr->_pMoveSkillHotKey));
	memcpy(pr->_pMoveSkillTypeHotKey, savedPlr->vpMoveSkillTypeHotKey, lengthof(pr->_pMoveSkillTypeHotKey));

	memcpy(pr->_pAltAtkSkillHotKey, savedPlr->vpAltAtkSkillHotKey, lengthof(pr->_pAltAtkSkillHotKey));
	memcpy(pr->_pAltAtkSkillTypeHotKey, savedPlr->vpAltAtkSkillTypeHotKey, lengthof(pr->_pAltAtkSkillTypeHotKey));
	memcpy(pr->_pAltMoveSkillHotKey, savedPlr->vpAltMoveSkillHotKey, lengthof(pr->_pAltMoveSkillHotKey));
	memcpy(pr->_pAltMoveSkillTypeHotKey, savedPlr->vpAltMoveSkillTypeHotKey, lengthof(pr->_pAltMoveSkillTypeHotKey));

	memcpy(pr->_pAltAtkSkillSwapKey, savedPlr->vpAltAtkSkillSwapKey, lengthof(pr->_pAltAtkSkillSwapKey));
	memcpy(pr->_pAltAtkSkillTypeSwapKey, savedPlr->vpAltAtkSkillTypeSwapKey, lengthof(pr->_pAltAtkSkillTypeSwapKey));
	memcpy(pr->_pAltMoveSkillSwapKey, savedPlr->vpAltMoveSkillSwapKey, lengthof(pr->_pAltMoveSkillSwapKey));
	memcpy(pr->_pAltMoveSkillTypeSwapKey, savedPlr->vpAltMoveSkillTypeSwapKey, lengthof(pr->_pAltMoveSkillTypeSwapKey));

	memcpy(pr->_pSkillLvlBase, savedPlr->vpSkillLvlBase, lengthof(pr->_pSkillLvlBase));
	memcpy(pr->_pSkillActivity, savedPlr->vpSkillActivity, lengthof(pr->_pSkillActivity));

	for (int i = 0; i < lengthof(pr->_pSkillExp); i++)
		pr->_pSkillExp[i] = savedPlr->vpSkillExp[i];

	pr->_pMemSkills = savedPlr->vpMemSkills;
	pr->_pAblSkills = savedPlr->vpAblSkills;
	pr->_pInvSkills = savedPlr->vpInvSkills;
	memcpy(pr->_pName, savedPlr->vpName, lengthof(pr->_pName));

	pr->_pBaseStr = savedPlr->vpBaseStr;
	pr->_pBaseMag = savedPlr->vpBaseMag;

	pr->_pBaseDex = savedPlr->vpBaseDex;
	pr->_pBaseVit = savedPlr->vpBaseVit;

	pr->_pHPBase = savedPlr->vpHPBase;
	pr->_pMaxHPBase = savedPlr->vpMaxHPBase;
	pr->_pManaBase = savedPlr->vpManaBase;
	pr->_pMaxManaBase = savedPlr->vpMaxManaBase;
	pr->_pVar1 = savedPlr->vpVar1;
	pr->_pVar2 = savedPlr->vpVar2;
	pr->_pVar3 = savedPlr->vpVar3;
	pr->_pVar4 = savedPlr->vpVar4;
	pr->_pVar5 = savedPlr->vpVar5;
	pr->_pVar6 = savedPlr->vpVar6;
	pr->_pVar7 = savedPlr->vpVar7;
	pr->_pVar8 = savedPlr->vpVar8;
	//int _pGFXLoad; // flags of the loaded gfx('s)  (player_graphic_flag)
	//PlrAnimStruct _pAnims[NUM_PGXS];
	//unsigned _pAFNum;
	//unsigned _pSFNum;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSavePlayerStruct, vpAnimDataAlign) == offsetof(PlayerStruct, _pdir) + sizeof(pr->_pdir), "LoadPlayer uses memcpy to load most of the LSavePlayerStruct in PlayerStruct I.");
	memcpy(pr, savedPlr, offsetof(PlayerStruct, _pdir) + sizeof(pr->_pdir));
	static_assert((offsetof(LSavePlayerStruct, vpVar8) + sizeof(savedPlr->vpVar8)) - offsetof(LSavePlayerStruct, vpAnimCnt)
		== (offsetof(PlayerStruct, _pVar8) + sizeof(pr->_pVar8)) - offsetof(PlayerStruct, _pAnimCnt), "LoadPlayer uses memcpy to load most of the LSavePlayerStruct in PlayerStruct II.");
	memcpy(&pr->_pAnimCnt, &savedPlr->vpAnimCnt, (offsetof(PlayerStruct, _pVar8) + sizeof(pr->_pVar8)) - offsetof(PlayerStruct, _pAnimCnt));
#else
	static_assert(offsetof(LSavePlayerStruct, vpHoldItem) == offsetof(LSavePlayerStruct, vpVar8) + sizeof(savedPlr->vpVar8)
	 && offsetof(PlayerStruct, _pVar8) == offsetof(LSavePlayerStruct, vpVar8), "LoadPlayer uses memcpy to load most of the LSavePlayerStruct in PlayerStruct.");
	memcpy(pr, savedPlr, offsetof(LSavePlayerStruct, vpHoldItem));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	// src = LoadItem(src, &pr->_pHoldItem);
	LoadItem((BYTE*)&savedPlr->vpHoldItem, &pr->_pHoldItem);
	for (int i = 0; i < NUM_INVLOC; i++)
		LoadItem((BYTE*)&savedPlr->vpInvBody[i], &pr->_pInvBody[i]);
		// src = LoadItem(src, &pr->_pInvBody[i]);
	for (int i = 0; i < MAXBELTITEMS; i++)
		LoadItem((BYTE*)&savedPlr->vpSpdList[i], &pr->_pSpdList[i]);
		// src = LoadItem(src, &pr->_pSpdList[i]);
	for (int i = 0; i < NUM_INV_GRID_ELEM; i++)
		LoadItem((BYTE*)&savedPlr->vpInvList[i], &pr->_pInvList[i]);
		// src = LoadItem(src, &pr->_pInvList[i]);
	pr->_pGold = savedPlr->vpGold;

	src += sizeof(LSavePlayerStruct);

	/*Skip to Calc
	tbuff += 4; // _pStrength
	tbuff += 4; // _pMagic
	tbuff += 4; // _pDexterity
	tbuff += 4; // _pVitality
	tbuff += 4; // _pHitPoints
	tbuff += 4; // _pMaxHP
	tbuff += 4; // _pMana
	tbuff += 4; // _pMaxMana
	tbuff += 64; // _pSkillLvl
	tbuff += 8; // _pISpells
	tbuff += 1; // _pSkillFlags
	tbuff += 1; // _pInfraFlag
	tbuff += 1; // _pgfxnum
	tbuff += 1; // _pHasUnidItem
	tbuff += 4; // _pISlMinDam
	tbuff += 4; // _pISlMaxDam
	tbuff += 4; // _pIBlMinDam
	tbuff += 4; // _pIBlMaxDam
	tbuff += 4; // _pIPcMinDam
	tbuff += 4; // _pIPcMaxDam
	tbuff += 4; // _pIChMinDam
	tbuff += 4; // _pIChMaxDam
	tbuff += 4; // _pIEvasion
	tbuff += 4; // _pIAC
	tbuff += 1; // _pMagResist
	tbuff += 1; // _pFireResist
	tbuff += 1; // _pLghtResist
	tbuff += 1; // _pAcidResist
	tbuff += 4; // _pIHitChance
	tbuff += 1; // _pIBaseHitBonus
	tbuff += 1; // _pICritChance
	tbuff += 2; // _pIBlockChance

	tbuff += 4; // _pIFlags
	tbuff += 1; // _pIWalkSpeed
	tbuff += 1; // _pIRecoverySpeed
	tbuff += 1; // _pIBaseCastSpeed
	tbuff += 1; // _pAlign_B1
	tbuff += 4; // _pIAbsAnyHit
	tbuff += 4; // _pIAbsPhyHit
	tbuff += 1; // _pIBaseAttackSpeed
	tbuff += 1; // _pAlign_B2
	tbuff += 1; // _pILifeSteal
	tbuff += 1; // _pIManaSteal
	tbuff += 4; // _pIFMinDam
	tbuff += 4; // _pIFMaxDam
	tbuff += 4; // _pILMinDam
	tbuff += 4; // _pILMaxDam
	tbuff += 4; // _pIMMinDam
	tbuff += 4; // _pIMMaxDam
	tbuff += 4; // _pIAMinDam
	tbuff += 4; // _pIAMaxDam*/

	return src;
}

static BYTE* LoadMonster(BYTE* DVL_RESTRICT src, int mnum, bool full)
{
	MonsterStruct* DVL_RESTRICT mon = &monsters[mnum];

	LSaveMonsterStruct* DVL_RESTRICT savedMon = (LSaveMonsterStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	mon->_mmode = savedMon->vmmode;
	mon->_msquelch = savedMon->vmsquelch;

	mon->_mMTidx = savedMon->vmMTidx;
	mon->_mpathcount = savedMon->vmpathcount; // unused
	mon->_mAlign_1 = savedMon->vmAlign_1;     // unused
	mon->_mgoal = savedMon->vmgoal;

	mon->_mgoalvar1 = savedMon->vmgoalvar1;
	mon->_mgoalvar2 = savedMon->vmgoalvar2;
	mon->_mgoalvar3 = savedMon->vmgoalvar3;
	mon->_mx = savedMon->vmx;
	mon->_my = savedMon->vmy;
	mon->_mfutx = savedMon->vmfutx;
	mon->_mfuty = savedMon->vmfuty;
	mon->_moldx = savedMon->vmoldx;
	mon->_moldy = savedMon->vmoldy;
	mon->_mxoff = savedMon->vmxoff;
	mon->_myoff = savedMon->vmyoff;
	mon->_mdir = savedMon->vmdir;
	mon->_menemy = savedMon->vmenemy;
	mon->_menemyx = savedMon->vmenemyx;
	mon->_menemyy = savedMon->vmenemyy;
	mon->_mListener = savedMon->vmListener;
	mon->_mDelFlag = savedMon->vmDelFlag; // unused
	// mon->_mAnimData = savedMon->vmAnimDataAlign;
	// mon->_mAnimFrameLen = savedMon->vmAnimFrameLenAlign;
	mon->_mAnimCnt = savedMon->vmAnimCnt;
	mon->_mAnimLen = savedMon->vmAnimLenAlign;
	mon->_mAnimFrame = savedMon->vmAnimFrame;
	mon->_mVar1 = savedMon->vmVar1;
	mon->_mVar2 = savedMon->vmVar2;
	mon->_mVar3 = savedMon->vmVar3;
	mon->_mVar4 = savedMon->vmVar4;
	mon->_mVar5 = savedMon->vmVar5;
	mon->_mVar6 = savedMon->vmVar6;
	mon->_mVar7 = savedMon->vmVar7;
	mon->_mVar8 = savedMon->vmVar8;
	mon->_mmaxhp = savedMon->vmmaxhp;
	mon->_mhitpoints = savedMon->vmhitpoints;
	mon->_mlastx = savedMon->vmlastx;
	mon->_mlasty = savedMon->vmlasty;
	mon->_mRndSeed = savedMon->vmRndSeed;
	mon->_mAISeed = savedMon->vmAISeed;

	mon->_muniqtype = savedMon->vmuniqtype;
	mon->_muniqtrans = savedMon->vmuniqtrans;
	mon->_mNameColor = savedMon->vmNameColor;
	mon->_mlid = savedMon->vmlid;

	mon->_mleader = savedMon->vmleader;
	mon->_mleaderflag = savedMon->vmleaderflag;
	mon->_mpacksize = savedMon->vmpacksize;
	mon->_mvid = savedMon->vmvid;
	// mon->_mName = savedMon->vmNameAlign;

	mon->_mFileNum = savedMon->vmFileNum;
	mon->_mLevel = savedMon->vmLevel;
	mon->_mSelFlag = savedMon->vmSelFlag;

	mon->_mAI.aiType = savedMon->vmAI_aiType;
	mon->_mAI.aiInt = savedMon->vmAI_aiInt;
	mon->_mAI.aiParam1 = savedMon->vmAI_aiParam1;
	mon->_mAI.aiParam2 = savedMon->vmAI_aiParam2;

	mon->_mFlags = savedMon->vmFlags;

	mon->_mHit = savedMon->vmHit;
	mon->_mMinDamage = savedMon->vmMinDamage;
	mon->_mMaxDamage = savedMon->vmMaxDamage;

	mon->_mHit2 = savedMon->vmHit2;
	mon->_mMinDamage2 = savedMon->vmMinDamage2;
	mon->_mMaxDamage2 = savedMon->vmMaxDamage2;

	mon->_mMagic = savedMon->vmMagic;
	mon->_mArmorClass = savedMon->vmArmorClass;
	mon->_mEvasion = savedMon->vmEvasion;

	mon->_mMagicRes = savedMon->vmMagicRes;
	mon->_mExp = savedMon->vmExp;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveMonsterStruct, vmAnimDataAlign) == offsetof(MonsterStruct, _mDelFlag) + sizeof(mon->_mDelFlag), "LoadMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct I.");
	memcpy(mon, savedMon, offsetof(MonsterStruct, _mDelFlag) + sizeof(mon->_mDelFlag));
	static_assert((offsetof(LSaveMonsterStruct, vmvid) + sizeof(savedMon->vmvid)) - offsetof(LSaveMonsterStruct, vmAnimCnt)
		== (offsetof(MonsterStruct, _mvid) + sizeof(mon->_mvid)) - offsetof(MonsterStruct, _mAnimCnt), "LoadMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct II.");
	memcpy(&mon->_mAnimCnt, &savedMon->vmAnimCnt, (offsetof(MonsterStruct, _mvid) + sizeof(mon->_mvid)) - offsetof(MonsterStruct, _mAnimCnt));
	static_assert((offsetof(LSaveMonsterStruct, vmExp) + sizeof(savedMon->vmExp)) - offsetof(LSaveMonsterStruct, vmFileNum)
		== (offsetof(MonsterStruct, _mExp) + sizeof(mon->_mExp)) - offsetof(MonsterStruct, _mFileNum), "LoadMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct III.");
	memcpy(&mon->_mFileNum, &savedMon->vmFileNum, (offsetof(MonsterStruct, _mExp) + sizeof(mon->_mExp)) - offsetof(MonsterStruct, _mFileNum));
#else
	// preserve AnimData, AnimFrameLen and Name members for towners to prevent the need for SyncTownerAnim
	BYTE* tmpAnimData = mon->_mAnimData;
	int tmpAnimFrameLen = mon->_mAnimFrameLen;
	const char* tmpName = mon->_mName;

	static_assert(sizeof(LSaveMonsterStruct) == offsetof(LSaveMonsterStruct, vmExp) + sizeof(savedMon->vmExp)
	 && offsetof(MonsterStruct, _mExp) == offsetof(LSaveMonsterStruct, vmExp), "LoadMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct.");
	memcpy(mon, savedMon, sizeof(LSaveMonsterStruct));

	mon->_mAnimData = tmpAnimData;
	mon->_mAnimFrameLen = tmpAnimFrameLen;
	mon->_mName = tmpName;
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	src += sizeof(LSaveMonsterStruct);

	// Skip _mAnimWidth
	// Skip _mAnimXOffset
	// Skip _mAFNum
	// Skip _mAFNum2
	// Skip _mAlign_0
	// Skip pointer mAnims
	// Skip _mType

	if (!full) {
		// reset charging and stoned monsters, because the missiles are not saved
		if (mon->_mmode == MM_STONE) {
			mon->_mmode = mon->_mVar3;
		} else if (mon->_mmode == MM_CHARGE) {
			mon->_mmode = MM_STAND;
			// TODO: set mVar1 and mVar2?
			// mon->_mVar1 = MM_CHARGE; // STAND_PREV_MODE
			// mon->_mVar2 = ...;
		}
	}

	return src;
}

static BYTE* LoadMissile(BYTE* DVL_RESTRICT src, int mi)
{
	MissileStruct* DVL_RESTRICT mis = &missile[mi];

	LSaveMissileStruct* DVL_RESTRICT savedMis = (LSaveMissileStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	mis->_miType = savedMis->vmiType;

	mis->_miFlags = savedMis->vmiFlags;
	mis->_miResist = savedMis->vmiResist;
	mis->_miFileNum = savedMis->vmiFileNum;
	mis->_miDelFlag = savedMis->vmiDelFlag;

	mis->_miUniqTrans = savedMis->vmiUniqTrans;

	// mis->_miDrawFlag = savedMis->vmiDrawFlagAlign;
	// mis->_miAnimFlag = savedMis->vmiAnimFlagAlign;
	// mis->_miLightFlag = savedMis->vmiLightFlagAlign;
	// mis->_miPreFlag = savedMis->vmiPreFlagAlign;

	// mis->_miAnimData = savedMis->vmiAnimDataAlign;
	// mis->_miAnimFrameLen = savedMis->vmiAnimFrameLenAlign;
	// mis->_miAnimLen = savedMis->vmiAnimLenAlign;
	// mis->_miAnimWidth = savedMis->vmiAnimWidthAlign;
	// mis->_miAnimXOffset = savedMis->vmiAnimXOffsetAlign;
	mis->_miAnimCnt = savedMis->vmiAnimCnt;
	mis->_miAnimAdd = savedMis->vmiAnimAdd;
	mis->_miAnimFrame = savedMis->vmiAnimFrame;
	mis->_misx = savedMis->vmisx;
	mis->_misy = savedMis->vmisy;
	mis->_mix = savedMis->vmix;
	mis->_miy = savedMis->vmiy;
	mis->_mixoff = savedMis->vmixoff;
	mis->_miyoff = savedMis->vmiyoff;
	mis->_mixvel = savedMis->vmixvel;
	mis->_miyvel = savedMis->vmiyvel;
	mis->_mitxoff = savedMis->vmitxoff;
	mis->_mityoff = savedMis->vmityoff;
	mis->_miDir = savedMis->vmiDir;
	mis->_miSpllvl = savedMis->vmiSpllvl;
	mis->_miSource = savedMis->vmiSource;
	mis->_miCaster = savedMis->vmiCaster;
	mis->_miMinDam = savedMis->vmiMinDam;
	mis->_miMaxDam = savedMis->vmiMaxDam;
	mis->_miRange = savedMis->vmiRange;
	mis->_miLid = savedMis->vmiLid;
	mis->_miVar1 = savedMis->vmiVar1;
	mis->_miVar2 = savedMis->vmiVar2;
	mis->_miVar3 = savedMis->vmiVar3;
	mis->_miVar4 = savedMis->vmiVar4;
	mis->_miVar5 = savedMis->vmiVar5;
	mis->_miVar6 = savedMis->vmiVar6;
	mis->_miVar7 = savedMis->vmiVar7;
	mis->_miVar8 = savedMis->vmiVar8;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveMissileStruct, vmiDrawFlagAlign) == offsetof(MissileStruct, _miUniqTrans) + sizeof(mis->_miUniqTrans), "LoadMissile uses memcpy to load the LSaveMissileStruct in MissileStruct I.");
	memcpy(mis, savedMis, offsetof(MissileStruct, _miUniqTrans) + sizeof(mis->_miUniqTrans));
	static_assert((offsetof(LSaveMissileStruct, vmiVar8) + sizeof(savedMis->vmiVar8)) - offsetof(LSaveMissileStruct, vmiAnimCnt)
		== (offsetof(MissileStruct, _miVar8) + sizeof(mis->_miVar8)) - offsetof(MissileStruct, _miAnimCnt), "LoadMissile uses memcpy to load the LSaveMissileStruct in MissileStruct II.");
	memcpy(&mis->_miAnimCnt, &savedMis->vmiAnimCnt, (offsetof(MissileStruct, _miVar8) + sizeof(mis->_miVar8)) - offsetof(MissileStruct, _miAnimCnt));
#else
	static_assert(sizeof(LSaveMissileStruct) == offsetof(LSaveMissileStruct, vmiVar8) + sizeof(savedMis->vmiVar8)
	 && offsetof(MissileStruct, _miVar8) == offsetof(LSaveMissileStruct, vmiVar8), "LoadMissile uses memcpy to load the LSaveMissileStruct in MissileStruct.");
	memcpy(mis, savedMis, sizeof(LSaveMissileStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	src += sizeof(LSaveMissileStruct);

	return src;
}

static BYTE* LoadObject(BYTE* DVL_RESTRICT src, int oi, bool full)
{
	ObjectStruct* DVL_RESTRICT os = &objects[oi];

	LSaveObjectStruct* DVL_RESTRICT savedObj = (LSaveObjectStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	os->_otype = savedObj->votype;
	os->_ox = savedObj->vox;
	os->_oy = savedObj->voy;
	os->_oSFX = savedObj->voSFX;

	os->_oSFXCnt = savedObj->voSFXCnt;
	os->_oAnimFlag = savedObj->voAnimFlag;
	os->_oProc = savedObj->voProc;
	os->_oModeFlags = savedObj->voModeFlags;

	// os->_oAnimData = savedObj->voAnimDataAlign;
	os->_oAnimFrameLen = savedObj->voAnimFrameLen;
	os->_oAnimCnt = savedObj->voAnimCnt;
	os->_oAnimLen = savedObj->voAnimLen;
	os->_oAnimFrame = savedObj->voAnimFrame;
	os->_oAnimWidth = savedObj->voAnimWidthAlign;
	os->_oAnimXOffset = savedObj->voAnimXOffsetAlign;

	os->_oSolidFlag = savedObj->voSolidFlag;
	os->_oBreak = savedObj->voBreak;
	os->_oTrapChance = savedObj->voTrapChance;
	os->_oAlign = savedObj->voAlign;

	os->_oMissFlag = savedObj->voMissFlag;
	os->_oDoorFlag = savedObj->voDoorFlag;
	os->_oSelFlag = savedObj->voSelFlag;
	os->_oPreFlag = savedObj->voPreFlag;

	os->_olid = savedObj->volid;
	os->_oRndSeed = savedObj->voRndSeed;
	os->_oVar1 = savedObj->voVar1;
	os->_oVar2 = savedObj->voVar2;
	os->_oVar3 = savedObj->voVar3;
	os->_oVar4 = savedObj->voVar4;
	os->_oVar5 = savedObj->voVar5;
	os->_oVar6 = savedObj->voVar6;
	os->_oVar7 = savedObj->voVar7;
	os->_oVar8 = savedObj->voVar8;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveObjectStruct, voAnimDataAlign) == offsetof(ObjectStruct, _oModeFlags) + sizeof(os->_oModeFlags), "LoadObject uses memcpy to load the LSaveObjectStruct in ObjectStruct I.");
	memcpy(os, savedObj, offsetof(ObjectStruct, _oModeFlags) + sizeof(os->_oModeFlags));
	static_assert((offsetof(LSaveObjectStruct, voVar8) + sizeof(savedObj->voVar8)) - offsetof(LSaveObjectStruct, voAnimFrameLen)
		== (offsetof(ObjectStruct, _oVar8) + sizeof(os->_oVar8)) - offsetof(ObjectStruct, _oAnimFrameLen), "LoadObject uses memcpy to load the LSaveObjectStruct in ObjectStruct II.");
	static_assert(sizeof(LSaveObjectStruct) - offsetof(LSaveObjectStruct, voAnimFrameLen) == sizeof(ObjectStruct) - offsetof(ObjectStruct, _oAnimFrameLen) - sizeof(os->alignment), "LoadObject uses memcpy to load the LSaveObjectStruct in ObjectStruct III.");
	memcpy(&os->_oAnimFrameLen, &savedObj->voAnimFrameLen, (offsetof(ObjectStruct, _oVar8) + sizeof(os->_oVar8)) - offsetof(ObjectStruct, _oAnimFrameLen));
#else
	static_assert(sizeof(LSaveObjectStruct) == offsetof(LSaveObjectStruct, voVar8) + sizeof(savedObj->voVar8)
	 && offsetof(ObjectStruct, _oVar8) == offsetof(LSaveObjectStruct, voVar8), "LoadObject uses memcpy to load the LSaveObjectStruct in ObjectStruct.");
	memcpy(os, savedObj, sizeof(LSaveObjectStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	src += sizeof(LSaveObjectStruct);

	if (!full) {
		// reset dynamic lights
		os->_olid = NO_LIGHT;
	}

	return src;
}

static BYTE* LoadQuest(BYTE* DVL_RESTRICT src, int i)
{
	QuestStruct* DVL_RESTRICT pQuest = &quests[i];

	LSaveQuestStruct* DVL_RESTRICT savedQuest = (LSaveQuestStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	pQuest->_qactive = savedQuest->vqactive;
	pQuest->_qvar1 = savedQuest->vqvar1;
	pQuest->_qvar2 = savedQuest->vqvar2;
	pQuest->_qlog = savedQuest->vqlog;

	pQuest->_qmsg = savedQuest->vqmsg;
#else
	static_assert(sizeof(LSaveQuestStruct) == offsetof(LSaveQuestStruct, vqmsg) + sizeof(savedQuest->vqmsg)
	 && offsetof(QuestStruct, _qmsg) == offsetof(LSaveQuestStruct, vqmsg), "LoadQuest uses memcpy to load the LSaveQuestStruct in QuestStruct.");
	memcpy(pQuest, savedQuest, sizeof(LSaveQuestStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	src += sizeof(LSaveQuestStruct);

	return src;
}

static BYTE* LoadLight(BYTE* src, LightListStruct* DVL_RESTRICT pLight)
{
	LSaveLightListStruct* DVL_RESTRICT savedLight = (LSaveLightListStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	pLight->_lx = savedLight->vlx;
	pLight->_ly = savedLight->vly;
	pLight->_lunx = savedLight->vlunx;
	pLight->_luny = savedLight->vluny;

	pLight->_lradius = savedLight->vlradius;
	pLight->_lunr = savedLight->vlunr;
	pLight->_lunxoff = savedLight->vlunxoff;
	pLight->_lunyoff = savedLight->vlunyoff;

	pLight->_ldel = savedLight->vldel;
	pLight->_lunflag = savedLight->vlunflag;
	pLight->_lmine = savedLight->vlmine;
	pLight->_lAlign2 = savedLight->vlAlign2;

	pLight->_lxoff = savedLight->vlxoff;
	pLight->_lyoff = savedLight->vlyoff;
#else
	static_assert(sizeof(LSaveLightListStruct) == offsetof(LSaveLightListStruct, vlyoff) + sizeof(savedLight->vlyoff)
	 && offsetof(LightListStruct, _lyoff) == offsetof(LSaveLightListStruct, vlyoff), "LoadLight uses memcpy to load the LSaveLightListStruct in LightListStruct.");
	memcpy(pLight, savedLight, sizeof(LSaveLightListStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	src += sizeof(LSaveLightListStruct);

	return src;
}

static BYTE* LoadPortal(BYTE* DVL_RESTRICT src, int i)
{
	PortalStruct* DVL_RESTRICT pPortal = &portals[i];

	LSavePortalStruct* DVL_RESTRICT savedPortal = (LSavePortalStruct*)src;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	pPortal->_rlevel = savedPortal->vrlevel;
	pPortal->_rx = savedPortal->vrx;
	pPortal->_ry = savedPortal->vry;
	// pPortal->alignment[0] = savedPortal->vrAlign0;
#else
	static_assert(sizeof(LSavePortalStruct) == offsetof(LSavePortalStruct, vrAlign0) + sizeof(savedPortal->vrAlign0)
	 && offsetof(PortalStruct, _rlevel) == offsetof(LSavePortalStruct, vrlevel), "LoadPortal uses memcpy to load the LSavePortalStruct in PortalStruct.");
	memcpy(pPortal, savedPortal, sizeof(LSavePortalStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	src += sizeof(LSavePortalStruct);

	return src;
}

static BYTE* LoadLevelData(BYTE* src, bool full)
{
	int i, moncount, ii;
	LSaveGameLvlMetaStruct* lms;

	deltaload = true;
	lms = (LSaveGameLvlMetaStruct*)src;
	// if (full || currLvl._dType != DTYPE_TOWN)
	nummonsters = lms->vvnummonsters;
	if (full)
		nummissiles = lms->vvnummissiles;
	// if (full || currLvl._dType != DTYPE_TOWN)
	numobjects = lms->vvnumobjects;
	numitems = lms->vvnumitems;
	src += sizeof(LSaveGameLvlMetaStruct);
	moncount = currLvl._dType != DTYPE_TOWN ? MAXMONSTERS : (full ? MAX_MINIONS + MAX_TOWNERS : 0);
	for (i = 0; i < moncount; i++)
		src = LoadMonster(src, i, full);
	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			SyncMonsterAnim(i);
	}
	if (full) {
		LE_LOAD_INTS(missileactive, src, lengthof(missileactive));
		src += lengthof(missileactive) * sizeof(LE_INT32);
		for (i = 0; i < nummissiles; i++)
			src = LoadMissile(src, missileactive[i]);
	}
	if (full || currLvl._dType != DTYPE_TOWN) {
//		LE_LOAD_INTS(objectactive, src, lengthof(objectactive));
//		src += lengthof(objectactive) * sizeof(LE_INT32);
//		LE_LOAD_INTS(objectavail, src, lengthof(objectavail));
//		src += lengthof(objectavail) * sizeof(LE_INT32);
		for (i = 0; i < numobjects; i++)
			src = LoadObject(src, i, full); // objectactive[i]
		// run in a separate loop because objects (e.g. crux) might depend on each other
		for (i = 0; i < numobjects; i++)
			SyncObjectAnim(i); // objectactive[i]
	}

	LE_LOAD_INTS(itemactive, src, lengthof(itemactive));
	src += lengthof(itemactive) * sizeof(LE_INT32);
	for (i = 0; i < numitems; i++) {
		ii = itemactive[i];
		src = LoadItem(src, &items[ii]);
		SyncItemAnim(ii);
	}

	memcpy(dFlags, src, MAXDUNX * MAXDUNY);
	src += MAXDUNX * MAXDUNY;
	memcpy(dItem, src, MAXDUNX * MAXDUNY);
	src += MAXDUNX * MAXDUNY;
	memcpy(dPreLight, src, MAXDUNX * MAXDUNY);
	src += MAXDUNX * MAXDUNY;

	if (full) {
		memcpy(dLight, src, MAXDUNX * MAXDUNY);
		src += MAXDUNX * MAXDUNY;
		memcpy(dPlayer, src, MAXDUNX * MAXDUNY);
		src += MAXDUNX * MAXDUNY;
		memcpy(dMissile, src, MAXDUNX * MAXDUNY);
		src += MAXDUNX * MAXDUNY;
	}

	if (full || currLvl._dType != DTYPE_TOWN) {
		LE_LOAD_INTS(&dMonster[0][0], src, MAXDUNX * MAXDUNY);
		src += MAXDUNX * MAXDUNY * sizeof(LE_INT32);
		memcpy(dObject, src, MAXDUNX * MAXDUNY);
		src += MAXDUNX * MAXDUNY;
	}

	if (currLvl._dType != DTYPE_TOWN) {
		memcpy(dDead, src, MAXDUNX * MAXDUNY);
		src += MAXDUNX * MAXDUNY;
	}

	deltaload = false;
	return src;
}

/**
 * @brief Load game state
 */
void LoadGame()
{
	int i;
	LSaveGameHeaderStruct* ghs;
	LSaveGameMetaStruct* gms;
	BYTE *fileBuff, *tbuff;
	int _ViewX, _ViewY;
	int32_t _CurrSeed;

	// TODO: UIDisconnectGame() ?
	SNetLeaveGame();

	pfile_read_save_file(true);
	fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	ghs = (LSaveGameHeaderStruct*)tbuff;
	if (ghs->vhInitial != SAVE_INITIAL)
		app_fatal("Invalid save file");
	// load game-info
	gdwGameLogicTurn = ghs->vhLogicTurn;
	// assert(gbNetUpdateRate == 1);
	gdwLastGameTurn = gdwGameLogicTurn;
	sgbSentThisCycle = ghs->vhSentCycle;
	for (i = 0; i < NUM_LEVELS; i++) {
		glSeedTbl[i] = ghs->vhSeeds[i];
	}
	_CurrSeed = ghs->vhCurrSeed;
	for (i = 0; i < NUM_DYNLVLS; i++) {
		gDynLevels[i]._dnLevel = ghs->vhDynLvls[i].vdLevel;
		gDynLevels[i]._dnType = ghs->vhDynLvls[i].vdType;
	}
	// load player-data
	_ViewX = ghs->vhViewX;
	_ViewY = ghs->vhViewY;
	// ghs->vhScrollX = ScrollInfo._sdx;
	// ghs->vhScrollY = ScrollInfo._sdy;
	ScrollInfo._sxoff = ghs->vhScrollXOff;
	ScrollInfo._syoff = ghs->vhScrollYOff;
	ScrollInfo._sdir = ghs->vhScrollDir;
	gnHPPer = ghs->vhHPPer;
	gnManaPer = ghs->vhManaPer;

	gbLvlUp = ghs->vhLvlUpFlag;
	gbAutomapflag = ghs->vhAutomapflag;
	gbZoomInFlag = ghs->vhZoomInFlag;
	gbInvflag = ghs->vhInvflag;

	gnNumActiveWindows = ghs->vhNumActiveWindows;
	memcpy(gaActiveWindows, ghs->vhActiveWindows, sizeof(gaActiveWindows));
	gnDifficulty = ghs->vhDifficulty;
	gbTownWarps = ghs->vhTownWarps;
	gbWaterDone = ghs->vhWaterDone;

	AutoMapScale = ghs->vhAutoMapScale;
	MiniMapScale = ghs->vhMiniMapScale;
	NormalMapScale = ghs->vhNormalMapScale;

	guLvlVisited = ghs->vhLvlVisited;

	tbuff += sizeof(LSaveGameHeaderStruct);
	// assert(mypnum == 0);
	tbuff = LoadPlayer(tbuff, 0);

	// load meta-data I. (used by LoadGameLevel)
	for (i = 0; i < NUM_QUESTS; i++)
		tbuff = LoadQuest(tbuff, i);
	for (i = 0; i < MAXPORTAL; i++)
		tbuff = LoadPortal(tbuff, i);
	// load level
	// assert(mypnum == 0);
	EnterLevel(plx(0)._pDunLevel);
	LoadGameLevel(ENTRY_LOAD);
	ViewX = _ViewX;
	ViewY = _ViewY;
	ResyncQuests();
	// load level-data
	tbuff = LoadLevelData(tbuff, true);

	// load meta-data III. (modified by LoadGameLevel)
	gms = (LSaveGameMetaStruct*)tbuff;

	//gms->vanumtowners = numtowners;
	boylevel = gms->vaboylevel;
	numpremium = gms->vanumpremium;
	premiumlevel = gms->vapremiumlevel;
	AutoMapXOfs = gms->vaAutoMapXOfs;
	AutoMapYOfs = gms->vaAutoMapYOfs;
	numlights = gms->vanumlights;
	numvision = gms->vanumvision;

	//numtrans = gms->vanumtrans;
	tbuff += sizeof(LSaveGameMetaStruct);

	memcpy(lightactive, tbuff, sizeof(lightactive));
	tbuff += sizeof(lightactive);
	for (i = 0; i < numlights; i++)
		tbuff = LoadLight(tbuff, &LightList[lightactive[i]]);

	memcpy(visionactive, tbuff, sizeof(visionactive));
	tbuff += sizeof(visionactive);
	for (i = 0; i < numvision; i++)
		tbuff = LoadLight(tbuff, &VisionList[visionactive[i]]);

	memcpy(TransList, tbuff, sizeof(TransList));
	tbuff += sizeof(TransList);

	tbuff = LoadItem(tbuff, &boyitem);
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		tbuff = LoadItem(tbuff, &premiumitems[i]);
	if (currLvl._dType == DTYPE_TOWN) {
		for (i = 0; i < SMITH_ITEMS; i++)
			tbuff = LoadItem(tbuff, &smithitem[i]);
		for (i = 0; i < HEALER_ITEMS; i++)
			tbuff = LoadItem(tbuff, &healitem[i]);
		for (i = 0; i < WITCH_ITEMS; i++)
			tbuff = LoadItem(tbuff, &witchitem[i]);
	}

	// assert(mypnum == 0);
	// CalculateGold(0);
	CalcPlrInv(0, false);
	InitPlayerGFX(0);
	SetPlrAnims(0);
	SyncPlrAnim(0);

	InitAutomapScale();
	//ResyncQuests();

	DRLG_RedoTrans();
	LightAndVisionDone();
	//RedoLightAndVision();
	//ProcessLightList();
	//ProcessVisionList();

	SyncMissilesAnim();
	CalcViewportGeometry();

	// restrore RNG seed
	SetRndSeed(_CurrSeed);

	sgbSentThisCycle--;
	nthread_send_turn();
}

static BYTE* SaveItem(BYTE* DVL_RESTRICT dest, ItemStruct* DVL_RESTRICT is)
{
	LSaveItemStruct* DVL_RESTRICT itemSave = (LSaveItemStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	itemSave->viSeed = is->_iSeed;
	itemSave->viIdx = is->_iIdx;
	itemSave->viCreateInfo = is->_iCreateInfo;
	itemSave->vix = is->_ix;
	itemSave->viy = is->_iy;
	itemSave->viCurs = is->_iCurs;
	itemSave->vitype = is->_itype;
	itemSave->viMiscId = is->_iMiscId;
	itemSave->viSpell = is->_iSpell;

	itemSave->viClass = is->_iClass;
	itemSave->viLoc = is->_iLoc;
	itemSave->viDamType = is->_iDamType;
	itemSave->viMinDam = is->_iMinDam;

	itemSave->viMaxDam = is->_iMaxDam;
	itemSave->viBaseCrit = is->_iBaseCrit;
	itemSave->viMinStr = is->_iMinStr;
	itemSave->viMinMag = is->_iMinMag;

	itemSave->viMinDex = is->_iMinDex;
	itemSave->viUsable = is->_iUsable;
	itemSave->viPrePower = is->_iPrePower;
	itemSave->viSufPower = is->_iSufPower;

	itemSave->viMagical = is->_iMagical;
	itemSave->viSelFlag = is->_iSelFlag;
	itemSave->viFloorFlag = is->_iFloorFlag;
	itemSave->viAnimFlag = is->_iAnimFlag;

	// itemSave->viAnimDataAlign = is->_iAnimData;
	// itemSave->viAnimFrameLenAlign = is->_iAnimFrameLen;
	itemSave->viAnimCnt = is->_iAnimCnt;
	itemSave->viAnimLen = is->_iAnimLen;
	itemSave->viAnimFrame = is->_iAnimFrame;
	itemSave->viPostDraw = is->_iPostDraw;
	itemSave->viIdentified = is->_iIdentified;
	memcpy(itemSave->viName, is->_iName, lengthof(is->_iName));
	itemSave->vivalue = is->_ivalue;
	itemSave->viIvalue = is->_iIvalue;
	itemSave->viAC = is->_iAC;
	itemSave->viPLFlags = is->_iPLFlags;
	itemSave->viCharges = is->_iCharges;
	itemSave->viMaxCharges = is->_iMaxCharges;
	itemSave->viDurability = is->_iDurability;
	itemSave->viMaxDur = is->_iMaxDur;
	itemSave->viPLDam = is->_iPLDam;
	itemSave->viPLToHit = is->_iPLToHit;
	itemSave->viPLAC = is->_iPLAC;
	itemSave->viPLStr = is->_iPLStr;
	itemSave->viPLMag = is->_iPLMag;
	itemSave->viPLDex = is->_iPLDex;
	itemSave->viPLVit = is->_iPLVit;
	itemSave->viPLFR = is->_iPLFR;
	itemSave->viPLLR = is->_iPLLR;
	itemSave->viPLMR = is->_iPLMR;
	itemSave->viPLAR = is->_iPLAR;
	itemSave->viPLMana = is->_iPLMana;
	itemSave->viPLHP = is->_iPLHP;
	itemSave->viPLDamMod = is->_iPLDamMod;

	itemSave->viPLToBlk = is->_iPLToBlk;
	itemSave->viPLAtkSpdMod = is->_iPLAtkSpdMod;
	itemSave->viPLAbsAnyHit = is->_iPLAbsAnyHit;
	itemSave->viPLAbsPhyHit = is->_iPLAbsPhyHit;

	itemSave->viPLLight = is->_iPLLight;
	itemSave->viPLSkillLevels = is->_iPLSkillLevels;
	itemSave->viPLSkill = is->_iPLSkill;
	itemSave->viPLSkillLvl = is->_iPLSkillLvl;

	itemSave->viPLManaSteal = is->_iPLManaSteal;
	itemSave->viPLLifeSteal = is->_iPLLifeSteal;
	itemSave->viPLCrit = is->_iPLCrit;
	itemSave->viStatFlag = is->_iStatFlag;

	itemSave->viUid = is->_iUid;

	itemSave->viPLFMinDam = is->_iPLFMinDam;
	itemSave->viPLFMaxDam = is->_iPLFMaxDam;
	itemSave->viPLLMinDam = is->_iPLLMinDam;
	itemSave->viPLLMaxDam = is->_iPLLMaxDam;

	itemSave->viPLMMinDam = is->_iPLMMinDam;
	itemSave->viPLMMaxDam = is->_iPLMMaxDam;
	itemSave->viPLAMinDam = is->_iPLAMinDam;
	itemSave->viPLAMaxDam = is->_iPLAMaxDam;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveItemStruct, viAnimDataAlign) == offsetof(ItemStruct, _iAnimData), "SaveItem uses memcpy to store the ItemStruct in LSaveItemStruct I.");
	memcpy(itemSave, is, offsetof(ItemStruct, _iAnimData));
	static_assert(sizeof(LSaveItemStruct) - offsetof(LSaveItemStruct, viAnimCnt) == sizeof(ItemStruct) - offsetof(ItemStruct, _iAnimCnt) - sizeof(is->alignment), "SaveItem uses memcpy to store the ItemStruct in LSaveItemStruct II.");
	memcpy(&itemSave->viAnimCnt, &is->_iAnimCnt, sizeof(LSaveItemStruct) - offsetof(LSaveItemStruct, viAnimCnt));
#else
	static_assert(sizeof(LSaveItemStruct) == offsetof(LSaveItemStruct, viPLAMaxDam) + sizeof(itemSave->viPLAMaxDam)
	 && offsetof(ItemStruct, _iPLAMaxDam) == offsetof(LSaveItemStruct, viPLAMaxDam), "SaveItem uses memcpy to store the ItemStruct in LSaveItemStruct.");
	memcpy(itemSave, is, sizeof(LSaveItemStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	dest += sizeof(LSaveItemStruct);

	return dest;
}

static BYTE* SavePlayer(BYTE* DVL_RESTRICT dest, int pnum)
{
	PlayerStruct* DVL_RESTRICT pr = &players[pnum];

	LSavePlayerStruct* DVL_RESTRICT plrSave = (LSavePlayerStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	plrSave->vpmode = pr->_pmode;
	plrSave->vpDestAction = pr->_pDestAction;
	plrSave->vpDestParam1 = pr->_pDestParam1;
	plrSave->vpDestParam2 = pr->_pDestParam2;
	plrSave->vpDestParam3 = pr->_pDestParam3;
	plrSave->vpDestParam4 = pr->_pDestParam4;

	plrSave->vpActive = pr->_pActive;
	plrSave->vpInvincible = pr->_pInvincible;
	plrSave->vpLvlChanging = pr->_pLvlChanging;
	plrSave->vpDunLevel = pr->_pDunLevel;

	plrSave->vpClass = pr->_pClass;
	plrSave->vpLevel = pr->_pLevel;
	plrSave->vpRank = pr->_pRank;
	plrSave->vpTeam = pr->_pTeam;

	plrSave->vpStatPts = pr->_pStatPts;
	plrSave->vpLightRad = pr->_pLightRad;
	plrSave->vpManaShield = pr->_pManaShield;

	static_assert(NUM_PLRTIMERS == 2, "SavePlayer copies a fixed amount of timers.");
	plrSave->vpTimer[PLTR_INFRAVISION] = pr->_pTimer[PLTR_INFRAVISION];
	plrSave->vpTimer[PLTR_RAGE] = pr->_pTimer[PLTR_RAGE];

	plrSave->vpExperience = pr->_pExperience;
	plrSave->vpNextExper = pr->_pNextExper;
	plrSave->vpx = pr->_px;
	plrSave->vpy = pr->_py;
	plrSave->vpfutx = pr->_pfutx;
	plrSave->vpfuty = pr->_pfuty;
	plrSave->vpoldx = pr->_poldx;
	plrSave->vpoldy = pr->_poldy;
	plrSave->vpxoff = pr->_pxoff;
	plrSave->vpyoff = pr->_pyoff;
	plrSave->vpdir = pr->_pdir;
	// plrSave->vpAnimDataAlign = pr->_pAnimData;
	// plrSave->vpAnimFrameLenAlign = pr->_pAnimFrameLen;
	plrSave->vpAnimCnt = pr->_pAnimCnt;
	plrSave->vpAnimLenAlign = pr->_pAnimLen;
	plrSave->vpAnimFrame = pr->_pAnimFrame;
	plrSave->vpAnimWidthAlign = pr->_pAnimWidth;
	plrSave->vpAnimXOffsetAlign = pr->_pAnimXOffset;
	plrSave->vplid = pr->_plid;
	plrSave->vpvid = pr->_pvid;

	plrSave->vpAtkSkill = pr->_pAtkSkill;
	plrSave->vpAtkSkillType = pr->_pAtkSkillType;
	plrSave->vpMoveSkill = pr->_pMoveSkill;
	plrSave->vpMoveSkillType = pr->_pMoveSkillType;

	plrSave->vpAltAtkSkill = pr->_pAltAtkSkill;
	plrSave->vpAltAtkSkillType = pr->_pAltAtkSkillType;
	plrSave->vpAltMoveSkill = pr->_pAltMoveSkill;
	plrSave->vpAltMoveSkillType = pr->_pAltMoveSkillType;

	memcpy(plrSave->vpAtkSkillHotKey, pr->_pAtkSkillHotKey, lengthof(plrSave->vpAtkSkillHotKey));
	memcpy(plrSave->vpAtkSkillTypeHotKey, pr->_pAtkSkillTypeHotKey, lengthof(plrSave->vpAtkSkillTypeHotKey));
	memcpy(plrSave->vpMoveSkillHotKey, pr->_pMoveSkillHotKey, lengthof(plrSave->vpMoveSkillHotKey));
	memcpy(plrSave->vpMoveSkillTypeHotKey, pr->_pMoveSkillTypeHotKey, lengthof(plrSave->vpMoveSkillTypeHotKey));

	memcpy(plrSave->vpAltAtkSkillHotKey, pr->_pAltAtkSkillHotKey, lengthof(plrSave->vpAltAtkSkillHotKey));
	memcpy(plrSave->vpAltAtkSkillTypeHotKey, pr->_pAltAtkSkillTypeHotKey, lengthof(plrSave->vpAltAtkSkillTypeHotKey));
	memcpy(plrSave->vpAltMoveSkillHotKey, pr->_pAltMoveSkillHotKey, lengthof(plrSave->vpAltMoveSkillHotKey));
	memcpy(plrSave->vpAltMoveSkillTypeHotKey, pr->_pAltMoveSkillTypeHotKey, lengthof(plrSave->vpAltMoveSkillTypeHotKey));

	memcpy(plrSave->vpAltAtkSkillSwapKey, pr->_pAltAtkSkillSwapKey, lengthof(plrSave->vpAltAtkSkillSwapKey));
	memcpy(plrSave->vpAltAtkSkillTypeSwapKey, pr->_pAltAtkSkillTypeSwapKey, lengthof(plrSave->vpAltAtkSkillTypeSwapKey));
	memcpy(plrSave->vpAltMoveSkillSwapKey, pr->_pAltMoveSkillSwapKey, lengthof(plrSave->vpAltMoveSkillSwapKey));
	memcpy(plrSave->vpAltMoveSkillTypeSwapKey, pr->_pAltMoveSkillTypeSwapKey, lengthof(plrSave->vpAltMoveSkillTypeSwapKey));

	memcpy(plrSave->vpSkillLvlBase, pr->_pSkillLvlBase, lengthof(plrSave->vpSkillLvlBase));
	memcpy(plrSave->vpSkillActivity, pr->_pSkillActivity, lengthof(plrSave->vpSkillActivity));

	for (int i = 0; i < lengthof(plrSave->vpSkillExp); i++)
		plrSave->vpSkillExp[i] = pr->_pSkillExp[i];

	plrSave->vpMemSkills = pr->_pMemSkills;
	plrSave->vpAblSkills = pr->_pAblSkills;
	plrSave->vpInvSkills = pr->_pInvSkills;
	memcpy(plrSave->vpName, pr->_pName, lengthof(plrSave->vpName));

	plrSave->vpBaseStr = pr->_pBaseStr;
	plrSave->vpBaseMag = pr->_pBaseMag;

	plrSave->vpBaseDex = pr->_pBaseDex;
	plrSave->vpBaseVit = pr->_pBaseVit;

	plrSave->vpHPBase = pr->_pHPBase;
	plrSave->vpMaxHPBase = pr->_pMaxHPBase;
	plrSave->vpManaBase = pr->_pManaBase;
	plrSave->vpMaxManaBase = pr->_pMaxManaBase;
	plrSave->vpVar1 = pr->_pVar1;
	plrSave->vpVar2 = pr->_pVar2;
	plrSave->vpVar3 = pr->_pVar3;
	plrSave->vpVar4 = pr->_pVar4;
	plrSave->vpVar5 = pr->_pVar5;
	plrSave->vpVar6 = pr->_pVar6;
	plrSave->vpVar7 = pr->_pVar7;
	plrSave->vpVar8 = pr->_pVar8;
	//int _pGFXLoad; // flags of the loaded gfx('s)  (player_graphic_flag)
	//PlrAnimStruct _pAnims[NUM_PGXS];
	//unsigned _pAFNum;
	//unsigned _pSFNum;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSavePlayerStruct, vpAnimDataAlign) == offsetof(PlayerStruct, _pdir) + sizeof(pr->_pdir), "SavePlayer uses memcpy to store most of the PlayerStruct in LSavePlayerStruct I.");
	memcpy(plrSave, pr, offsetof(PlayerStruct, _pdir) + sizeof(pr->_pdir));
	static_assert((offsetof(LSavePlayerStruct, vpVar8) + sizeof(plrSave->vpVar8)) - offsetof(LSavePlayerStruct, vpAnimCnt)
		== (offsetof(PlayerStruct, _pVar8) + sizeof(pr->_pVar8)) - offsetof(PlayerStruct, _pAnimCnt), "SavePlayer uses memcpy to store most of the PlayerStruct in LSavePlayerStruct II.");
	memcpy(&plrSave->vpAnimCnt, &pr->_pAnimCnt, (offsetof(PlayerStruct, _pVar8) + sizeof(pr->_pVar8)) - offsetof(PlayerStruct, _pAnimCnt));
#else
	static_assert(offsetof(LSavePlayerStruct, vpHoldItem) == offsetof(LSavePlayerStruct, vpVar8) + sizeof(plrSave->vpVar8)
	 && offsetof(PlayerStruct, _pVar8) == offsetof(LSavePlayerStruct, vpVar8), "SavePlayer uses memcpy to store most of the PlayerStruct in LSavePlayerStruct.");
	memcpy(plrSave, pr, offsetof(LSavePlayerStruct, vpHoldItem));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	// dest += offsetof(LSavePlayerStruct, vpHoldItem);
	// dest = SaveItem(dest, &pr->_pHoldItem);
	SaveItem((BYTE*)&plrSave->vpHoldItem, &pr->_pHoldItem);	
	for (int i = 0; i < NUM_INVLOC; i++)
		SaveItem((BYTE*)&plrSave->vpInvBody[i], &pr->_pInvBody[i]);
		// dest = SaveItem(dest, &pr->_pInvBody[i]);
	for (int i = 0; i < MAXBELTITEMS; i++)
		SaveItem((BYTE*)&plrSave->vpSpdList[i], &pr->_pSpdList[i]);
		// dest = SaveItem(dest, &pr->_pSpdList[i]);
	for (int i = 0; i < NUM_INV_GRID_ELEM; i++)
		SaveItem((BYTE*)&plrSave->vpInvList[i], &pr->_pInvList[i]);
		// dest = SaveItem(dest, &pr->_pInvList[i]);
	plrSave->vpGold = pr->_pGold;

	dest += sizeof(LSavePlayerStruct);

	/*Skip to Calc
	tbuff += 4; // _pStrength
	tbuff += 4; // _pMagic
	tbuff += 4; // _pDexterity
	tbuff += 4; // _pVitality
	tbuff += 4; // _pHitPoints
	tbuff += 4; // _pMaxHP
	tbuff += 4; // _pMana
	tbuff += 4; // _pMaxMana
	tbuff += 64; // _pSkillLvl
	tbuff += 8; // _pISpells
	tbuff += 1; // _pSkillFlags
	tbuff += 1; // _pInfraFlag
	tbuff += 1; // _pgfxnum
	tbuff += 1; // _pHasUnidItem
	tbuff += 4; // _pISlMinDam
	tbuff += 4; // _pISlMaxDam
	tbuff += 4; // _pIBlMinDam
	tbuff += 4; // _pIBlMaxDam
	tbuff += 4; // _pIPcMinDam
	tbuff += 4; // _pIPcMaxDam
	tbuff += 4; // _pIChMinDam
	tbuff += 4; // _pIChMaxDam
	tbuff += 4; // _pIEvasion
	tbuff += 4; // _pIAC
	tbuff += 1; // _pMagResist
	tbuff += 1; // _pFireResist
	tbuff += 1; // _pLghtResist
	tbuff += 1; // _pAcidResist
	tbuff += 4; // _pIHitChance
	tbuff += 1; // _pIBaseHitBonus
	tbuff += 1; // _pICritChance
	tbuff += 2; // _pIBlockChance

	tbuff += 4; // _pIFlags
	tbuff += 1; // _pIWalkSpeed
	tbuff += 1; // _pIRecoverySpeed
	tbuff += 1; // _pIBaseCastSpeed
	tbuff += 1; // _pAlign_B1
	tbuff += 4; // _pIAbsAnyHit
	tbuff += 4; // _pIAbsPhyHit
	tbuff += 1; // _pIBaseAttackSpeed
	tbuff += 1; // _pAlign_B2
	tbuff += 1; // _pILifeSteal
	tbuff += 1; // _pIManaSteal
	tbuff += 4; // _pIFMinDam
	tbuff += 4; // _pIFMaxDam
	tbuff += 4; // _pILMinDam
	tbuff += 4; // _pILMaxDam
	tbuff += 4; // _pIMMinDam
	tbuff += 4; // _pIMMaxDam
	tbuff += 4; // _pIAMinDam
	tbuff += 4; // _pIAMaxDam*/

	// Omit pointers _pAnimFileData
	// Omit pointer alignment

	return dest;
}

static BYTE* SaveMonster(BYTE* DVL_RESTRICT dest, int mnum)
{
	MonsterStruct* DVL_RESTRICT mon = &monsters[mnum];

	LSaveMonsterStruct* DVL_RESTRICT monSave = (LSaveMonsterStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	monSave->vmmode = mon->_mmode;
	monSave->vmsquelch = mon->_msquelch;

	monSave->vmMTidx = mon->_mMTidx;
	monSave->vmpathcount = mon->_mpathcount; // unused
	monSave->vmAlign_1 = mon->_mAlign_1;     // unused
	monSave->vmgoal = mon->_mgoal;

	monSave->vmgoalvar1 = mon->_mgoalvar1;
	monSave->vmgoalvar2 = mon->_mgoalvar2;
	monSave->vmgoalvar3 = mon->_mgoalvar3;
	monSave->vmx = mon->_mx;
	monSave->vmy = mon->_my;
	monSave->vmfutx = mon->_mfutx;
	monSave->vmfuty = mon->_mfuty;
	monSave->vmoldx = mon->_moldx;
	monSave->vmoldy = mon->_moldy;
	monSave->vmxoff = mon->_mxoff;
	monSave->vmyoff = mon->_myoff;
	monSave->vmdir = mon->_mdir;
	monSave->vmenemy = mon->_menemy;
	monSave->vmenemyx = mon->_menemyx;
	monSave->vmenemyy = mon->_menemyy;
	monSave->vmListener = mon->_mListener;
	monSave->vmDelFlag = mon->_mDelFlag; // unused
	// monSave->vmAnimDataAlign = mon->_mAnimData;
	// monSave->vmAnimFrameLenAlign = mon->_mAnimFrameLen;
	monSave->vmAnimCnt = mon->_mAnimCnt;
	monSave->vmAnimLenAlign = mon->_mAnimLen;
	monSave->vmAnimFrame = mon->_mAnimFrame;
	monSave->vmVar1 = mon->_mVar1;
	monSave->vmVar2 = mon->_mVar2;
	monSave->vmVar3 = mon->_mVar3;
	monSave->vmVar4 = mon->_mVar4;
	monSave->vmVar5 = mon->_mVar5;
	monSave->vmVar6 = mon->_mVar6;
	monSave->vmVar7 = mon->_mVar7;
	monSave->vmVar8 = mon->_mVar8;
	monSave->vmmaxhp = mon->_mmaxhp;
	monSave->vmhitpoints = mon->_mhitpoints;
	monSave->vmlastx = mon->_mlastx;
	monSave->vmlasty = mon->_mlasty;
	monSave->vmRndSeed = mon->_mRndSeed;
	monSave->vmAISeed = mon->_mAISeed;

	monSave->vmuniqtype = mon->_muniqtype;
	monSave->vmuniqtrans = mon->_muniqtrans;
	monSave->vmNameColor = mon->_mNameColor;
	monSave->vmlid = mon->_mlid;

	monSave->vmleader = mon->_mleader;
	monSave->vmleaderflag = mon->_mleaderflag;
	monSave->vmpacksize = mon->_mpacksize;
	monSave->vmvid = mon->_mvid;
	// monSave->vmNameAlign = mon->_mName;

	monSave->vmFileNum = mon->_mFileNum;
	monSave->vmLevel = mon->_mLevel;
	monSave->vmSelFlag = mon->_mSelFlag;

	monSave->vmAI_aiType = mon->_mAI.aiType;
	monSave->vmAI_aiInt = mon->_mAI.aiInt;
	monSave->vmAI_aiParam1 = mon->_mAI.aiParam1;
	monSave->vmAI_aiParam2 = mon->_mAI.aiParam2;

	monSave->vmFlags = mon->_mFlags;

	monSave->vmHit = mon->_mHit;
	monSave->vmMinDamage = mon->_mMinDamage;
	monSave->vmMaxDamage = mon->_mMaxDamage;

	monSave->vmHit2 = mon->_mHit2;
	monSave->vmMinDamage2 = mon->_mMinDamage2;
	monSave->vmMaxDamage2 = mon->_mMaxDamage2;

	monSave->vmMagic = mon->_mMagic;
	monSave->vmArmorClass = mon->_mArmorClass;
	monSave->vmEvasion = mon->_mEvasion;

	monSave->vmMagicRes = mon->_mMagicRes;
	monSave->vmExp = mon->_mExp;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveMonsterStruct, vmAnimDataAlign) == offsetof(MonsterStruct, _mDelFlag) + sizeof(mon->_mDelFlag), "SaveMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct I.");
	memcpy(monSave, mon, offsetof(MonsterStruct, _mDelFlag) + sizeof(mon->_mDelFlag));
	static_assert((offsetof(LSaveMonsterStruct, vmvid) + sizeof(monSave->vmvid)) - offsetof(LSaveMonsterStruct, vmAnimCnt)
		== (offsetof(MonsterStruct, _mvid) + sizeof(mon->_mvid)) - offsetof(MonsterStruct, _mAnimCnt), "SaveMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct II.");
	memcpy(&monSave->vmAnimCnt, &mon->_mAnimCnt, (offsetof(MonsterStruct, _mvid) + sizeof(mon->_mvid)) - offsetof(MonsterStruct, _mAnimCnt));
	static_assert((offsetof(LSaveMonsterStruct, vmExp) + sizeof(monSave->vmExp)) - offsetof(LSaveMonsterStruct, vmFileNum)
		== (offsetof(MonsterStruct, _mExp) + sizeof(mon->_mExp)) - offsetof(MonsterStruct, _mFileNum), "SaveMonster uses memcpy to load the LSaveMonsterStruct in MonsterStruct III.");
	memcpy(&monSave->vmFileNum, &mon->_mFileNum, (offsetof(MonsterStruct, _mExp) + sizeof(mon->_mExp)) - offsetof(MonsterStruct, _mFileNum));
#else
	static_assert(sizeof(LSaveMonsterStruct) == offsetof(LSaveMonsterStruct, vmExp) + sizeof(monSave->vmExp)
	 && offsetof(MonsterStruct, _mExp) == offsetof(LSaveMonsterStruct, vmExp), "SaveMonster uses memcpy to store the MonsterStruct in LSaveMonsterStruct.");
	memcpy(monSave, mon, sizeof(LSaveMonsterStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	dest += sizeof(LSaveMonsterStruct);

	// Skip _mAnimWidth
	// Skip _mAnimXOffset
	// Skip _mAFNum
	// Skip _mAFNum2
	// Skip _mAlign_0
	// Skip pointer mAnims
	// Skip _mType

	return dest;
}

static BYTE* SaveMissile(BYTE* DVL_RESTRICT dest, int mi)
{
	MissileStruct* DVL_RESTRICT mis = &missile[mi];

	LSaveMissileStruct* DVL_RESTRICT misSave = (LSaveMissileStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	misSave->vmiType = mis->_miType;

	misSave->vmiFlags = mis->_miFlags;
	misSave->vmiResist = mis->_miResist;
	misSave->vmiFileNum = mis->_miFileNum;
	misSave->vmiDelFlag = mis->_miDelFlag;

	misSave->vmiUniqTrans = mis->_miUniqTrans;

	// misSave->vmiDrawFlagAlign = mis->_miDrawFlag;
	// misSave->vmiAnimFlagAlign = mis->_miAnimFlag;
	// misSave->vmiLightFlagAlign = mis->_miLightFlag;
	// misSave->vmiPreFlagAlign = mis->_miPreFlag;

	// misSave->vmiAnimDataAlign = mis->_miAnimData;
	// misSave->vmiAnimFrameLenAlign = mis->_miAnimFrameLen;
	// misSave->vmiAnimLenAlign = mis->_miAnimLen;
	// misSave->vmiAnimWidthAlign = mis->_miAnimWidth;
	// misSave->vmiAnimXOffsetAlign = mis->_miAnimXOffset;
	misSave->vmiAnimCnt = mis->_miAnimCnt;
	misSave->vmiAnimAdd = mis->_miAnimAdd;
	misSave->vmiAnimFrame = mis->_miAnimFrame;
	misSave->vmisx = mis->_misx;
	misSave->vmisy = mis->_misy;
	misSave->vmix = mis->_mix;
	misSave->vmiy = mis->_miy;
	misSave->vmixoff = mis->_mixoff;
	misSave->vmiyoff = mis->_miyoff;
	misSave->vmixvel = mis->_mixvel;
	misSave->vmiyvel = mis->_miyvel;
	misSave->vmitxoff = mis->_mitxoff;
	misSave->vmityoff = mis->_mityoff;
	misSave->vmiDir = mis->_miDir;
	misSave->vmiSpllvl = mis->_miSpllvl;
	misSave->vmiSource = mis->_miSource;
	misSave->vmiCaster = mis->_miCaster;
	misSave->vmiMinDam = mis->_miMinDam;
	misSave->vmiMaxDam = mis->_miMaxDam;
	misSave->vmiRange = mis->_miRange;
	misSave->vmiLid = mis->_miLid;
	misSave->vmiVar1 = mis->_miVar1;
	misSave->vmiVar2 = mis->_miVar2;
	misSave->vmiVar3 = mis->_miVar3;
	misSave->vmiVar4 = mis->_miVar4;
	misSave->vmiVar5 = mis->_miVar5;
	misSave->vmiVar6 = mis->_miVar6;
	misSave->vmiVar7 = mis->_miVar7;
	misSave->vmiVar8 = mis->_miVar8;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveMissileStruct, vmiDrawFlagAlign) == offsetof(MissileStruct, _miUniqTrans) + sizeof(mis->_miUniqTrans), "SaveMissile uses memcpy to store the MissileStruct in LSaveMissileStruct I.");
	memcpy(misSave, mis, offsetof(MissileStruct, _miUniqTrans) + sizeof(mis->_miUniqTrans));
	static_assert((offsetof(LSaveMissileStruct, vmiVar8) + sizeof(misSave->vmiVar8)) - offsetof(LSaveMissileStruct, vmiAnimCnt)
		== (offsetof(MissileStruct, _miVar8) + sizeof(mis->_miVar8)) - offsetof(MissileStruct, _miAnimCnt), "SaveMissile uses memcpy to store the MissileStruct in LSaveMissileStruct II.");
	memcpy(&misSave->vmiAnimCnt, &mis->_miAnimCnt, (offsetof(MissileStruct, _miVar8) + sizeof(mis->_miVar8)) - offsetof(MissileStruct, _miAnimCnt));
#else
	static_assert(sizeof(LSaveMissileStruct) == offsetof(LSaveMissileStruct, vmiVar8) + sizeof(misSave->vmiVar8)
	 && offsetof(MissileStruct, _miVar8) == offsetof(LSaveMissileStruct, vmiVar8), "SaveMissile uses memcpy to store the MissileStruct in LSaveMissileStruct.");
	memcpy(misSave, mis, sizeof(LSaveMissileStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	dest += sizeof(LSaveMissileStruct);

	return dest;
}

static BYTE* SaveObject(BYTE* DVL_RESTRICT dest, int oi)
{
	ObjectStruct* DVL_RESTRICT os = &objects[oi];

	LSaveObjectStruct* DVL_RESTRICT objSave = (LSaveObjectStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	objSave->votype = os->_otype;
	objSave->vox = os->_ox;
	objSave->voy = os->_oy;
	objSave->voSFX = os->_oSFX;

	objSave->voSFXCnt = os->_oSFXCnt;
	objSave->voAnimFlag = os->_oAnimFlag;
	objSave->voProc = os->_oProc;
	objSave->voModeFlags = os->_oModeFlags;

	//objSave->voAnimDataAlign = os->_oAnimData;
	objSave->voAnimFrameLen = os->_oAnimFrameLen;
	objSave->voAnimCnt = os->_oAnimCnt;
	objSave->voAnimLen = os->_oAnimLen;
	objSave->voAnimFrame = os->_oAnimFrame;
	objSave->voAnimWidthAlign = os->_oAnimWidth;
	objSave->voAnimXOffsetAlign = os->_oAnimXOffset;

	objSave->voSolidFlag = os->_oSolidFlag;
	objSave->voBreak = os->_oBreak;
	objSave->voTrapChance = os->_oTrapChance;
	objSave->voAlign = os->_oAlign;

	objSave->voMissFlag = os->_oMissFlag;
	objSave->voDoorFlag = os->_oDoorFlag;
	objSave->voSelFlag = os->_oSelFlag;
	objSave->voPreFlag = os->_oPreFlag;

	objSave->volid = os->_olid;
	objSave->voRndSeed = os->_oRndSeed;
	objSave->voVar1 = os->_oVar1;
	objSave->voVar2 = os->_oVar2;
	objSave->voVar3 = os->_oVar3;
	objSave->voVar4 = os->_oVar4;
	objSave->voVar5 = os->_oVar5;
	objSave->voVar6 = os->_oVar6;
	objSave->voVar7 = os->_oVar7;
	objSave->voVar8 = os->_oVar8;
#elif INTPTR_MAX != INT32_MAX
	static_assert(offsetof(LSaveObjectStruct, voAnimDataAlign) == offsetof(ObjectStruct, _oModeFlags) + sizeof(os->_oModeFlags), "SaveObject uses memcpy to store the ObjectStruct in LSaveObjectStruct I.");
	memcpy(objSave, os, offsetof(ObjectStruct, _oModeFlags) + sizeof(os->_oModeFlags));
	static_assert((offsetof(LSaveObjectStruct, voVar8) + sizeof(objSave->voVar8)) - offsetof(LSaveObjectStruct, voAnimFrameLen)
		== (offsetof(ObjectStruct, _oVar8) + sizeof(os->_oVar8)) - offsetof(ObjectStruct, _oAnimFrameLen), "SaveObject uses memcpy to store the ObjectStruct in LSaveObjectStruct II.");
	static_assert(sizeof(LSaveObjectStruct) - offsetof(LSaveObjectStruct, voAnimFrameLen) == sizeof(ObjectStruct) - offsetof(ObjectStruct, _oAnimFrameLen) - sizeof(os->alignment), "SaveObject uses memcpy to store the ObjectStruct in LSaveObjectStruct III.");
	memcpy(&objSave->voAnimFrameLen, &os->_oAnimFrameLen, (offsetof(ObjectStruct, _oVar8) + sizeof(os->_oVar8)) - offsetof(ObjectStruct, _oAnimFrameLen));
#else
	static_assert(sizeof(LSaveObjectStruct) == offsetof(LSaveObjectStruct, voVar8) + sizeof(objSave->voVar8)
	 && offsetof(ObjectStruct, _oVar8) == offsetof(LSaveObjectStruct, voVar8), "SaveObject uses memcpy to store the ObjectStruct in LSaveObjectStruct.");
	memcpy(objSave, os, sizeof(LSaveObjectStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	dest += sizeof(LSaveObjectStruct);

	return dest;
}

static BYTE* SaveQuest(BYTE* DVL_RESTRICT dest, int i)
{
	QuestStruct* DVL_RESTRICT pQuest = &quests[i];

	LSaveQuestStruct* DVL_RESTRICT questSave = (LSaveQuestStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	questSave->vqactive = pQuest->_qactive;
	questSave->vqvar1 = pQuest->_qvar1;
	questSave->vqvar2 = pQuest->_qvar2;
	questSave->vqlog = pQuest->_qlog;

	questSave->vqmsg = pQuest->_qmsg;
#else
	static_assert(sizeof(LSaveQuestStruct) == offsetof(LSaveQuestStruct, vqmsg) + sizeof(questSave->vqmsg)
	 && offsetof(QuestStruct, _qmsg) == offsetof(LSaveQuestStruct, vqmsg), "SaveQuest uses memcpy to load the QuestStruct in LSaveQuestStruct.");
	memcpy(questSave, pQuest, sizeof(LSaveQuestStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	dest += sizeof(LSaveQuestStruct);

	return dest;
}

static BYTE* SaveLight(BYTE* DVL_RESTRICT dest, LightListStruct* DVL_RESTRICT pLight)
{
	LSaveLightListStruct* DVL_RESTRICT lightSave = (LSaveLightListStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	lightSave->vlx = pLight->_lx;
	lightSave->vly = pLight->_ly;
	lightSave->vlunx = pLight->_lunx;
	lightSave->vluny = pLight->_luny;

	lightSave->vlradius = pLight->_lradius;
	lightSave->vlunr = pLight->_lunr;
	lightSave->vlunxoff = pLight->_lunxoff;
	lightSave->vlunyoff = pLight->_lunyoff;

	lightSave->vldel = pLight->_ldel;
	lightSave->vlunflag = pLight->_lunflag;
	lightSave->vlmine = pLight->_lmine;
	lightSave->vlAlign2 = pLight->_lAlign2;

	lightSave->vlxoff = pLight->_lxoff;
	lightSave->vlyoff = pLight->_lyoff;
#else
	static_assert(sizeof(LSaveLightListStruct) == offsetof(LSaveLightListStruct, vlyoff) + sizeof(lightSave->vlyoff)
	 && offsetof(LightListStruct, _lyoff) == offsetof(LSaveLightListStruct, vlyoff), "SaveLight uses memcpy to store the LightListStruct in LSaveLightListStruct.");
	memcpy(lightSave, pLight, sizeof(LSaveLightListStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	dest += sizeof(LSaveLightListStruct);

	return dest;
}

static BYTE* SavePortal(BYTE* DVL_RESTRICT dest, int i)
{
	PortalStruct* DVL_RESTRICT pPortal = &portals[i];

	LSavePortalStruct* DVL_RESTRICT portalSave = (LSavePortalStruct*)dest;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	portalSave->vrlevel = pPortal->_rlevel;
	portalSave->vrx = pPortal->_rx;
	portalSave->vry = pPortal->_ry;
	// portalSave->vrAlign0 = pPortal->alignment[0];
#else
	static_assert(sizeof(LSavePortalStruct) == offsetof(LSavePortalStruct, vrAlign0) + sizeof(portalSave->vrAlign0)
	 && offsetof(PortalStruct, _rlevel) == offsetof(LSavePortalStruct, vrlevel), "SavePortal uses memcpy to store the PortalStruct in LSavePortalStruct.");
	memcpy(portalSave, pPortal, sizeof(LSavePortalStruct));
#endif // SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX

	dest += sizeof(LSavePortalStruct);

	return dest;
}

static BYTE* SaveLevelData(BYTE* dest, bool full)
{
	int i, moncount;
	LSaveGameLvlMetaStruct* lms;

	lms = (LSaveGameLvlMetaStruct*)dest;
	lms->vvnummonsters = nummonsters;
	// if (full)
		lms->vvnummissiles = nummissiles;
	lms->vvnumobjects = numobjects;
	lms->vvnumitems = numitems;
	dest += sizeof(LSaveGameLvlMetaStruct);
	moncount = currLvl._dType != DTYPE_TOWN ? MAXMONSTERS : (full ? MAX_MINIONS + MAX_TOWNERS : 0);
	for (i = 0; i < moncount; i++)
		dest = SaveMonster(dest, i);
	if (full) {
		LE_SAVE_INTS(dest, missileactive, lengthof(missileactive));
		dest += lengthof(missileactive) * sizeof(LE_INT32);
		for (i = 0; i < nummissiles; i++)
			dest = SaveMissile(dest, missileactive[i]);
	}
	if (full || currLvl._dType != DTYPE_TOWN) {
//		LE_SAVE_INTS(dest, objectactive, lengthof(objectactive));
//		dest += lengthof(objectactive) * sizeof(LE_INT32);
//		LE_SAVE_INTS(dest, objectavail, lengthof(objectavail));
//		dest += lengthof(objectavail) * sizeof(LE_INT32);
		for (i = 0; i < numobjects; i++)
			dest = SaveObject(dest, i); // objectactive[i]
	}

	LE_SAVE_INTS(dest, itemactive, lengthof(itemactive));
	dest += lengthof(itemactive) * sizeof(LE_INT32);
	for (i = 0; i < numitems; i++)
		dest = SaveItem(dest, &items[itemactive[i]]);

	memcpy(dest, dFlags, MAXDUNX * MAXDUNY);
	dest += MAXDUNX * MAXDUNY;
	memcpy(dest, dItem, MAXDUNX * MAXDUNY);
	dest += MAXDUNX * MAXDUNY;
	memcpy(dest, dPreLight, MAXDUNX * MAXDUNY);
	dest += MAXDUNX * MAXDUNY;

	if (full) {
		memcpy(dest, dLight, MAXDUNX * MAXDUNY);
		dest += MAXDUNX * MAXDUNY;
		memcpy(dest, dPlayer, MAXDUNX * MAXDUNY);
		dest += MAXDUNX * MAXDUNY;
		memcpy(dest, dMissile, MAXDUNX * MAXDUNY);
		dest += MAXDUNX * MAXDUNY;
	}

	if (full || currLvl._dType != DTYPE_TOWN) {
		LE_SAVE_INTS(dest, &dMonster[0][0], MAXDUNX * MAXDUNY);
		dest += MAXDUNX * MAXDUNY * sizeof(LE_INT32);
		memcpy(dest, dObject, MAXDUNX * MAXDUNY);
		dest += MAXDUNX * MAXDUNY;
	}

	if (currLvl._dType != DTYPE_TOWN) {
		memcpy(dest, dDead, MAXDUNX * MAXDUNY);
		dest += MAXDUNX * MAXDUNY;
	}

	return dest;
}

/*static void RedoPlayerLight()
{
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr._pActive && currLvl._dLevelIdx == plr._pDunLevel)
			ChangeLightXY(plr._plid, plr._px, plr._py);
	}
}

static BYTE* SaveMonstersLight(BYTE* dest)
{
	MonsterStruct* mon;
	int mnum;
	LE_INT32* nl = (LE_INT32*)dest;
	*nl = 0;
	dest += sizeof(LE_INT32);
	for (mnum = 0; mnum < MAXMONSTERS; mnum++) {
		mon = &monsters[mnum];
		if (mon->_mlid != NO_LIGHT) {
			LE_INT32* mp = (LE_INT32*)dest;
			*mp = mnum;
			dest += sizeof(LE_INT32);
			dest = SaveLight(dest, &LightList[mon->_mlid]);
			*nl = *nl + 1;
		}
	}
	return dest;
}

static BYTE* SyncMonstersLight(BYTE* src)
{
	int i, lid;
	int nl = *(LE_INT32*)src;
	src += sizeof(LE_INT32);
	for (i = 0; i < nl; i++) {
		lid = monsters[*(LE_INT32*)src]._mlid;
		src += sizeof(LE_INT32);
		assert(lid != NO_LIGHT);
		LightListStruct lls;
		src = LoadLight(src, &lls);
		ChangeLight(lid, lls._lx, lls._ly, lls._lradius);
		ChangeLightScreenOff(lid, lls._lxoff, lls._lyoff);
	}
	return src;
}*/

void SaveGame()
{
	int i;
	LSaveGameHeaderStruct* DVL_RESTRICT ghs;
	LSaveGameMetaStruct* DVL_RESTRICT gms;
	BYTE* fileBuff = gsDeltaData.ddBuffer;
	BYTE* tbuff = fileBuff;

	constexpr size_t ss = sizeof(LSaveGameHeaderStruct) + sizeof(LSavePlayerStruct) + sizeof(LSaveQuestStruct) * NUM_QUESTS + sizeof(LSavePortalStruct) * MAXPORTAL;
	ghs = (LSaveGameHeaderStruct*)tbuff;
	ghs->vhInitial = SAVE_INITIAL;
	// save game-info
	ghs->vhLogicTurn = gdwGameLogicTurn;
	assert(gdwLastGameTurn == gdwGameLogicTurn
	 || ((gdwLastGameTurn + 1) == gdwGameLogicTurn && gbNetUpdateRate == 1));
	ghs->vhSentCycle = sgbSentThisCycle;
	for (i = 0; i < NUM_LEVELS; i++) {
		ghs->vhSeeds[i] = glSeedTbl[i];
	}
	for (i = 0; i < NUM_DYNLVLS; i++) {
		ghs->vhDynLvls[i].vdLevel = gDynLevels[i]._dnLevel;
		ghs->vhDynLvls[i].vdType = gDynLevels[i]._dnType;
	}
	ghs->vhCurrSeed = GetRndSeed();
	// save player-data
	ghs->vhViewX = ViewX;
	ghs->vhViewY = ViewY;
	// ghs->vhScrollX = ScrollInfo._sdx;
	// ghs->vhScrollY = ScrollInfo._sdy;
	ghs->vhScrollXOff = ScrollInfo._sxoff;
	ghs->vhScrollYOff = ScrollInfo._syoff;
	ghs->vhScrollDir = ScrollInfo._sdir;
	ghs->vhHPPer = gnHPPer;
	ghs->vhManaPer = gnManaPer;

	ghs->vhLvlUpFlag = gbLvlUp;
	ghs->vhAutomapflag = gbAutomapflag;
	ghs->vhZoomInFlag = gbZoomInFlag;
	ghs->vhInvflag = gbInvflag;

	ghs->vhNumActiveWindows = gnNumActiveWindows;
	memcpy(ghs->vhActiveWindows, gaActiveWindows, sizeof(gaActiveWindows));
	ghs->vhDifficulty = gnDifficulty;
	ghs->vhTownWarps = gbTownWarps;
	ghs->vhWaterDone = gbWaterDone;

	ghs->vhAutoMapScale = AutoMapScale;
	ghs->vhMiniMapScale = MiniMapScale;
	ghs->vhNormalMapScale = NormalMapScale;

	ghs->vhLvlVisited = guLvlVisited;

	tbuff += sizeof(LSaveGameHeaderStruct);
	// assert(mypnum == 0);
	tbuff = SavePlayer(tbuff, 0);

	// save meta-data I.
	for (i = 0; i < NUM_QUESTS; i++)
		tbuff = SaveQuest(tbuff, i);
	for (i = 0; i < MAXPORTAL; i++)
		tbuff = SavePortal(tbuff, i);
	// save level-data
	// assert(currLvl._dLevelIdx == plx(0)._pDunLevel);
	constexpr size_t slt = /*MAXDUNX * MAXDUNY +*/ sizeof(LSaveGameLvlMetaStruct) + (MAX_MINIONS + MAX_TOWNERS) * sizeof(LSaveMonsterStruct) + MAXMISSILES * 4
	 + MAXMISSILES * sizeof(LSaveMissileStruct)/* + MAXOBJECTS * (4 + sizeof(LSaveObjectStruct))*/ + MAXITEMS * (4 + sizeof(LSaveItemStruct))
	 + 5 * MAXDUNX * MAXDUNY + MAXDUNX * MAXDUNY * sizeof(INT)/* + MAXDUNX * MAXDUNY*/ + MAXDUNX * MAXDUNY;
	constexpr size_t sld = (MAXDUNX * MAXDUNY) + sizeof(LSaveGameLvlMetaStruct) + (MAXMONSTERS * sizeof(LSaveMonsterStruct) + MAXMISSILES * 4
	 + MAXMISSILES * sizeof(LSaveMissileStruct) + MAXOBJECTS * (4 + sizeof(LSaveObjectStruct))) + MAXITEMS * (4 + sizeof(LSaveItemStruct))
	 + 5 * MAXDUNX * MAXDUNY + (MAXDUNX * MAXDUNY * 4 + MAXDUNX * MAXDUNY + MAXDUNX * MAXDUNY);
	tbuff = SaveLevelData(tbuff, true);

	// save meta-data II. (modified by LoadGameLevel)
	constexpr size_t smt = sizeof(LSaveGameMetaStruct) + MAXLIGHTS + sizeof(LSaveLightListStruct) * MAXLIGHTS + MAXVISION + sizeof(LSaveLightListStruct) * MAXVISION + sizeof(TransList)
	 + (1 + SMITH_PREMIUM_ITEMS + (SMITH_ITEMS + HEALER_ITEMS + WITCH_ITEMS)) * sizeof(LSaveItemStruct);
	constexpr size_t smd = sizeof(LSaveGameMetaStruct) + MAXLIGHTS + sizeof(LSaveLightListStruct) * MAXLIGHTS + MAXVISION + sizeof(LSaveLightListStruct) * MAXVISION + sizeof(TransList)
	 + (1 + SMITH_PREMIUM_ITEMS/* + (SMITH_ITEMS + HEALER_ITEMS + WITCH_ITEMS)*/) * sizeof(LSaveItemStruct);
	gms = (LSaveGameMetaStruct*)tbuff;

	//gms->vanumtowners = numtowners;
	gms->vaboylevel = boylevel;
	gms->vanumpremium = numpremium;
	gms->vapremiumlevel = premiumlevel;
	gms->vaAutoMapXOfs = AutoMapXOfs;
	gms->vaAutoMapYOfs = AutoMapYOfs;
	gms->vanumlights = numlights;
	gms->vanumvision = numvision;

	//gms->vanumtrans = numtrans;
	tbuff += sizeof(LSaveGameMetaStruct);

	memcpy(tbuff, lightactive, sizeof(lightactive));
	tbuff += sizeof(lightactive);
	for (i = 0; i < numlights; i++)
		tbuff = SaveLight(tbuff, &LightList[lightactive[i]]);

	memcpy(tbuff, visionactive, sizeof(visionactive));
	tbuff += sizeof(visionactive);
	for (i = 0; i < numvision; i++)
		tbuff = SaveLight(tbuff, &VisionList[visionactive[i]]);

	memcpy(tbuff, TransList, sizeof(TransList));
	tbuff += sizeof(TransList);

	tbuff = SaveItem(tbuff, &boyitem);
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		tbuff = SaveItem(tbuff, &premiumitems[i]);
	if (currLvl._dType == DTYPE_TOWN) {
		for (i = 0; i < SMITH_ITEMS; i++)
			tbuff = SaveItem(tbuff, &smithitem[i]);
		for (i = 0; i < HEALER_ITEMS; i++)
			tbuff = SaveItem(tbuff, &healitem[i]);
		for (i = 0; i < WITCH_ITEMS; i++)
			tbuff = SaveItem(tbuff, &witchitem[i]);
	}

	constexpr size_t tst = ss + slt + smt;
	constexpr size_t tsd = ss + sld + smd;
	constexpr size_t mss = sizeof(gsDeltaData.ddBuffer) - SHA1BlockSize - 8 /*sizeof(CodecSignature)*/;
	static_assert(tst < mss, "Town might not fit to the preallocated buffer.");
	static_assert(tsd < mss, "Dungeon might not fit to the preallocated buffer.");
	static_assert(mss <= UINT32_MAX, "File is to large to be written by pfile_write_save_file I.");
	assert((size_t)tbuff - (size_t)fileBuff < mss);
	pfile_write_save_file(true, (DWORD)((size_t)tbuff - (size_t)fileBuff));
}

void SaveLevel()
{
	BYTE *fileBuff, *tbuff;

	//if (currLvl._dLevelIdx == DLV_TOWN)
	//	glSeedTbl[DLV_TOWN] = NextRndSeed();

	fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	tbuff = SaveLevelData(tbuff, false);
	//tbuff = SaveMonstersLight(tbuff); -- assuming there are no moving monsters with light

	constexpr size_t mss = sizeof(gsDeltaData.ddBuffer) - SHA1BlockSize - 8 /*sizeof(CodecSignature)*/;
	// static_assert(max(sld, slt) < mss, "Dungeon might not fit to the preallocated buffer.");
	static_assert(mss <= UINT32_MAX, "File is to large to be written by pfile_write_save_file II.");
	assert((size_t)tbuff - (size_t)fileBuff < mss);
	pfile_write_save_file(false, (DWORD)((size_t)tbuff - (size_t)fileBuff));
}

void LoadLevel()
{
	int i;
	BYTE *fileBuff, *tbuff, *tmp;

	pfile_read_save_file(false);
	fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	tbuff = LoadLevelData(tbuff, false);
	//tbuff = SyncMonstersLight(tbuff); -- assuming there are no moving monsters with light

	//ResyncQuests();
	//SyncPortals();

	// clear flags of the eliminated missiles
	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in LoadLevel.");
	tmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, tmp++)
		*tmp &= ~(BFLAG_MISSILE_PRE | BFLAG_HAZARD | BFLAG_ALERT /*| BFLAG_DEAD_PLAYER*/);
	// reload light to clear the lights of the eliminated missiles
	LoadPreLighting();
	// doLightning is not necessary, because it is going to be triggered
	// when the player is placed in the dungeon
	//RedoPlayerLight();
}

DEVILUTION_END_NAMESPACE
