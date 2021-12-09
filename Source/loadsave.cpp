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

BYTE* tbuff;

static bool LoadBool()
{
	return *tbuff++;
}

static void SaveBool(BOOL v)
{
	*tbuff++ = v != 0;
}

static void CopyBytes(const void* src, const int n, void* dst)
{
	memcpy(dst, src, n);
	tbuff += n;
}

static inline void SaveByte(const char* src)
{
	*tbuff = *src;
	tbuff += sizeof(BYTE);
}

static inline void SaveByte(const BYTE* src)
{
	*tbuff = *src;
	tbuff += sizeof(BYTE);
}

static inline void SaveByte(const int* src)
{
	*tbuff = *src;
	tbuff += sizeof(BYTE);
}

static inline void SaveInt16(const uint16_t* src)
{
	uint16_t buf = *src;

	buf = SwapLE16(buf);
	*(uint16_t*)tbuff = buf;

	tbuff += sizeof(uint16_t);
}

static inline void SaveInt16(const int16_t* src)
{
	uint16_t buf = *(uint16_t*)src;

	buf = SwapLE16(buf);
	*(uint16_t*)tbuff = buf;

	tbuff += sizeof(int16_t);
}

static inline void SaveInt32(const uint32_t* src)
{
	uint32_t buf = *(uint32_t*)src;

	buf = SwapLE32(buf);
	*(uint32_t*)tbuff = buf;

	tbuff += sizeof(uint32_t);
}

static inline void SaveInt(const int* src)
{
	uint32_t buf = *src;

	buf = SwapLE32(buf);
	*(uint32_t*)tbuff = buf;

	tbuff += sizeof(uint32_t);
}

static inline void SaveInt(const unsigned* src)
{
	uint32_t buf = *src;

	buf = SwapLE32(buf);
	*(uint32_t*)tbuff = buf;

	tbuff += sizeof(uint32_t);
}

static inline void SaveInt64(const uint64_t* src)
{
	uint64_t buf = *src;

	buf = SwapLE64(buf);
	*(uint64_t*)tbuff = buf;

	tbuff += sizeof(uint64_t);
}

static void SaveInts(const int* src, unsigned n)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	for (unsigned i = 0; i < n; i++, src++) {
		SaveInt(src);
	}
#else
	n *= sizeof(int);
	memcpy(tbuff, src, n);
	tbuff += n;
#endif
}

static void SaveInts(const unsigned* src, unsigned n)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	for (unsigned i = 0; i < n; i++, src++) {
		SaveInt(src);
	}
#else
	n *= sizeof(unsigned);
	memcpy(tbuff, src, n);
	tbuff += n;
#endif
}

static inline void LoadByte(char* dst)
{
	*dst = *tbuff;
	tbuff += sizeof(BYTE);
}

static inline void LoadByte(BYTE* dst)
{
	*dst = *tbuff;
	tbuff += sizeof(BYTE);
}

static inline void LoadByte(int* dst)
{
	*dst = *tbuff;
	tbuff += sizeof(BYTE);
}

static inline void LoadInt16(uint16_t* dst)
{
	uint16_t buf = *(uint16_t*)tbuff;

	buf = SwapLE16(buf);
	*(uint16_t*)dst = buf;

	tbuff += sizeof(uint16_t);
}

static inline void LoadInt16(int16_t* dst)
{
	uint16_t buf = *(uint16_t*)tbuff;

	buf = SwapLE16(buf);
	*(uint16_t*)dst = buf;

	tbuff += sizeof(uint16_t);
}

static inline void LoadInt32(uint32_t* dst)
{
	uint32_t buf = *(uint32_t*)tbuff;

	buf = SwapLE32(buf);
	*(uint32_t*)dst = buf;

	tbuff += sizeof(uint32_t);
}

static inline void LoadInt(int* dst)
{
	int32_t buf = *(uint32_t*)tbuff;

	buf = SwapLE32(buf);
	*dst = buf;

	tbuff += sizeof(uint32_t);
}

static inline void LoadInt(unsigned* dst)
{
	uint32_t buf = *(uint32_t*)tbuff;

	buf = SwapLE32(buf);
	*dst = buf;

	tbuff += sizeof(uint32_t);
}

static inline void LoadInt64(uint64_t* dst)
{
	uint64_t buf = *(uint64_t*)tbuff;

	buf = SwapLE64(buf);
	*dst = buf;

	tbuff += sizeof(uint64_t);
}

static void LoadInts(int* dst, unsigned n)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	for (unsigned i = 0; i < n; i++, dst++) {
		LoadInt(dst);
	}
#else
	n *= sizeof(int);
	memcpy(dst, tbuff, n);
	tbuff += n;
#endif
}

static void LoadInts(unsigned* dst, unsigned n)
{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN || INT_MAX != INT32_MAX
	for (unsigned i = 0; i < n; i++, dst++) {
		LoadInt(dst);
	}
#else
	n *= sizeof(unsigned);
	memcpy(dst, tbuff, n);
	tbuff += n;
#endif
}

static void LoadItemData(ItemStruct* is)
{
	LoadInt(&is->_iSeed);
	LoadInt16(&is->_iCreateInfo);
	LoadInt16(&is->_iIdx);
	LoadInt(&is->_ix);
	LoadInt(&is->_iy);
	LoadInt(&is->_iCurs);
	LoadInt(&is->_itype);
	LoadInt(&is->_iMiscId);
	LoadInt(&is->_iSpell);
	LoadByte(&is->_iClass);
	LoadByte(&is->_iLoc);
	LoadByte(&is->_iDamType);
	LoadByte(&is->_iMinDam);
	LoadByte(&is->_iMaxDam);
	LoadByte(&is->_iBaseCrit);
	LoadByte(&is->_iMinStr);
	LoadByte(&is->_iMinMag);
	LoadByte(&is->_iMinDex);
	tbuff += 3; // Alignment
	LoadInt(&is->_iAnimFlag);
	tbuff += 4; // Skip pointer _iAnimData
	tbuff += 4; // Skip _iAnimFrameLen
	LoadInt(&is->_iAnimCnt);
	LoadInt(&is->_iAnimLen);
	LoadInt(&is->_iAnimFrame);
	//LoadInt(&is->_iAnimWidth);
	//LoadInt(&is->_iAnimXOffset);
	LoadInt(&is->_iPostDraw);
	LoadInt(&is->_iIdentified);
	CopyBytes(tbuff, sizeof(is->_iName), is->_iName);
	LoadByte(&is->_iPrePower);
	LoadByte(&is->_iSufPower);
	LoadByte(&is->_iSelFlag);
	LoadByte(&is->_iMagical);
	LoadInt(&is->_ivalue);
	LoadInt(&is->_iIvalue);
	LoadInt(&is->_iAC);
	LoadInt(&is->_iFlags);
	LoadInt(&is->_iCharges);
	LoadInt(&is->_iMaxCharges);
	LoadInt(&is->_iDurability);
	LoadInt(&is->_iMaxDur);
	LoadInt(&is->_iPLDam);
	LoadInt(&is->_iPLToHit);
	LoadInt(&is->_iPLAC);
	LoadInt(&is->_iPLStr);
	LoadInt(&is->_iPLMag);
	LoadInt(&is->_iPLDex);
	LoadInt(&is->_iPLVit);
	LoadInt(&is->_iPLFR);
	LoadInt(&is->_iPLLR);
	LoadInt(&is->_iPLMR);
	LoadInt(&is->_iPLAR);
	LoadInt(&is->_iPLMana);
	LoadInt(&is->_iPLHP);
	LoadInt(&is->_iPLDamMod);
	LoadInt(&is->_iPLGetHit);
	LoadByte(&is->_iPLLight);
	LoadByte(&is->_iSplLvlAdd);
	LoadByte(&is->_iManaSteal);
	LoadByte(&is->_iLifeSteal);
	LoadByte(&is->_iPLCrit);
	tbuff += 3; // Alignment
	LoadInt(&is->_iUid);
	LoadByte(&is->_iFMinDam);
	LoadByte(&is->_iFMaxDam);
	LoadByte(&is->_iLMinDam);
	LoadByte(&is->_iLMaxDam);
	LoadByte(&is->_iMMinDam);
	LoadByte(&is->_iMMaxDam);
	LoadByte(&is->_iAMinDam);
	LoadByte(&is->_iAMaxDam);
	LoadInt(&is->_iVAdd);
	LoadInt(&is->_iVMult);
	LoadInt(&is->_iStatFlag);
}

static void LoadItems(ItemStruct* pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		LoadItemData(&pItem[i]);
	}
}

static void LoadPlayer(int pnum)
{
	LoadInt(&plr._pmode);
	CopyBytes(tbuff, lengthof(plr.walkpath), plr.walkpath);
	LoadInt(&plr.destAction);
	LoadInt(&plr.destParam1);
	LoadInt(&plr.destParam2);
	LoadInt(&plr.destParam3);
	LoadInt(&plr.destParam4);
	LoadByte(&plr._pActive);
	LoadByte(&plr._pInvincible);
	LoadByte(&plr._pLvlChanging);
	LoadByte(&plr._pDunLevel);
	LoadByte(&plr._pClass);
	LoadByte(&plr._pLevel);
	LoadByte(&plr._pRank);
	LoadByte(&plr._pTeam);
	LoadInt16(&plr._pStatPts);
	LoadByte(&plr._pLightRad);
	LoadByte(&plr._pManaShield);
	LoadInt16(&plr._pTimer[PLTR_INFRAVISION]);
	LoadInt16(&plr._pTimer[PLTR_RAGE]);
	LoadInt(&plr._pExperience);
	LoadInt(&plr._pNextExper);
	LoadInt(&plr._px);
	LoadInt(&plr._py);
	LoadInt(&plr._pfutx);
	LoadInt(&plr._pfuty);
	LoadInt(&plr._poldx);
	LoadInt(&plr._poldy);
	LoadInt(&plr._pxoff);
	LoadInt(&plr._pyoff);
	LoadInt(&plr._pxvel);
	LoadInt(&plr._pyvel);
	LoadInt(&plr._pdir);
	tbuff += 4; // Skip pointer _pAnimData
	tbuff += 4; // Skip _pAnimFrameLen
	LoadInt(&plr._pAnimCnt);
	LoadInt(&plr._pAnimLen);
	LoadInt(&plr._pAnimFrame);
	LoadInt(&plr._pAnimWidth);
	LoadInt(&plr._pAnimXOffset);
	LoadInt(&plr._plid);
	LoadInt(&plr._pvid);

	LoadByte(&plr._pAtkSkill);
	LoadByte(&plr._pAtkSkillType);
	LoadByte(&plr._pMoveSkill);
	LoadByte(&plr._pMoveSkillType);

	LoadByte(&plr._pAltAtkSkill);
	LoadByte(&plr._pAltAtkSkillType);
	LoadByte(&plr._pAltMoveSkill);
	LoadByte(&plr._pAltMoveSkillType);

	CopyBytes(tbuff, lengthof(plr._pAtkSkillHotKey), plr._pAtkSkillHotKey);
	CopyBytes(tbuff, lengthof(plr._pAtkSkillTypeHotKey), plr._pAtkSkillTypeHotKey);
	CopyBytes(tbuff, lengthof(plr._pMoveSkillHotKey), plr._pMoveSkillHotKey);
	CopyBytes(tbuff, lengthof(plr._pMoveSkillTypeHotKey), plr._pMoveSkillTypeHotKey);

	CopyBytes(tbuff, lengthof(plr._pAltAtkSkillHotKey), plr._pAltAtkSkillHotKey);
	CopyBytes(tbuff, lengthof(plr._pAltAtkSkillTypeHotKey), plr._pAltAtkSkillTypeHotKey);
	CopyBytes(tbuff, lengthof(plr._pAltMoveSkillHotKey), plr._pAltMoveSkillHotKey);
	CopyBytes(tbuff, lengthof(plr._pAltMoveSkillTypeHotKey), plr._pAltMoveSkillTypeHotKey);

	CopyBytes(tbuff, lengthof(plr._pSkillLvl), plr._pSkillLvl);
	CopyBytes(tbuff, lengthof(plr._pSkillActivity), plr._pSkillActivity);
	LoadInts(plr._pSkillExp, lengthof(plr._pSkillExp));
	LoadInt64(&plr._pMemSkills);
	LoadInt64(&plr._pAblSkills);
	LoadInt64(&plr._pScrlSkills);
	CopyBytes(tbuff, PLR_NAME_LEN, plr._pName);

	LoadInt16(&plr._pBaseStr);
	LoadInt16(&plr._pBaseMag);
	LoadInt16(&plr._pBaseDex);
	LoadInt16(&plr._pBaseVit);
	LoadInt(&plr._pHPBase);
	LoadInt(&plr._pMaxHPBase);
	LoadInt(&plr._pManaBase);
	LoadInt(&plr._pMaxManaBase);

	LoadInt(&plr._pStrength);
	LoadInt(&plr._pMagic);
	LoadInt(&plr._pDexterity);
	LoadInt(&plr._pVitality);
	LoadInt(&plr._pHitPoints);
	LoadInt(&plr._pMaxHP);
	LoadInt(&plr._pMana);
	LoadInt(&plr._pMaxMana);

	LoadInt(&plr._pVar1);
	LoadInt(&plr._pVar2);
	LoadInt(&plr._pVar3);
	LoadInt(&plr._pVar4);
	LoadInt(&plr._pVar5);
	LoadInt(&plr._pVar6);
	LoadInt(&plr._pVar7);
	LoadInt(&plr._pVar8);

	//tbuff += 4; // Skip _pGFXLoad
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pNAnim
	LoadInt(&plr._pNFrames);
	LoadInt(&plr._pNWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pWAnim
	LoadInt(&plr._pWFrames);
	LoadInt(&plr._pWWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pAAnim
	LoadInt(&plr._pAFrames);
	LoadInt(&plr._pAWidth);
	LoadInt(&plr._pAFNum);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pLAnim
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pFAnim
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pTAnim
	LoadInt(&plr._pSFrames);
	LoadInt(&plr._pSWidth);
	LoadInt(&plr._pSFNum);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pHAnim
	LoadInt(&plr._pHFrames);
	LoadInt(&plr._pHWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pDAnim
	LoadInt(&plr._pDFrames);
	LoadInt(&plr._pDWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pBAnim
	LoadInt(&plr._pBFrames);
	LoadInt(&plr._pBWidth);

	LoadItemData(&plr._pHoldItem);
	LoadItems(plr._pInvBody, NUM_INVLOC);
	LoadItems(plr._pSpdList, MAXBELTITEMS);
	LoadItems(plr._pInvList, NUM_INV_GRID_ELEM);
	LoadInt(&plr._pGold);

	/*Skip to Calc
	tbuff += 1; // _pInfraFlag
	tbuff += 1; // _pgfxnum
	tbuff += 1; // _pHasUnidItem
	tbuff += 1; // Alignment
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
	tbuff += 1; // _pSkillFlags
	tbuff += 1; // _pIBaseHitBonus
	tbuff += 1; // _pICritChance
	tbuff += 1; // _pIBlockChance

	tbuff += 8; // _pISpells
	tbuff += 4; // _pIFlags
	tbuff += 4; // _pIFlags2
	tbuff += 4; // _pIGetHit
	tbuff += 1; // _pISplLvlAdd
	tbuff += 1; // _pIArrowVelBonus
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

	CalcPlrInv(pnum, false);

	// Omit pointer _pNData
	// Omit pointer _pWData
	// Omit pointer _pAData
	// Omit pointer _pLData
	// Omit pointer _pFData
	// Omit pointer _pTData
	// Omit pointer _pHData
	// Omit pointer _pDData
	// Omit pointer _pBData
	// Omit pointer alignment

	InitPlayerGFX(pnum);
	SyncPlrAnim(pnum);
}

static void LoadMonster(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];

	LoadInt(&mon->_mmode);
	LoadInt(&mon->_msquelch);
	LoadByte(&mon->_mMTidx);
	LoadByte(&mon->_mpathcount);
	LoadByte(&mon->_mWhoHit);
	LoadByte(&mon->_mgoal);
	LoadInt(&mon->_mgoalvar1);
	LoadInt(&mon->_mgoalvar2);
	LoadInt(&mon->_mgoalvar3);
	LoadInt(&mon->_mx);
	LoadInt(&mon->_my);
	LoadInt(&mon->_mfutx);
	LoadInt(&mon->_mfuty);
	LoadInt(&mon->_moldx);
	LoadInt(&mon->_moldy);
	LoadInt(&mon->_mxoff);
	LoadInt(&mon->_myoff);
	LoadInt(&mon->_mxvel);
	LoadInt(&mon->_myvel);
	LoadInt(&mon->_mdir);
	LoadInt(&mon->_menemy);
	LoadByte(&mon->_menemyx);
	LoadByte(&mon->_menemyy);
	LoadByte(&mon->_mListener);
	tbuff += 1; // Alignment

	tbuff += 4; // Skip pointer _mAnimData
	tbuff += 4; // Skip _mAnimFrameLen
	LoadInt(&mon->_mAnimCnt);
	LoadInt(&mon->_mAnimLen);
	LoadInt(&mon->_mAnimFrame);
	LoadInt(&mon->_mDelFlag);
	LoadInt(&mon->_mVar1);
	LoadInt(&mon->_mVar2);
	LoadInt(&mon->_mVar3);
	LoadInt(&mon->_mVar4);
	LoadInt(&mon->_mVar5);
	LoadInt(&mon->_mVar6);
	LoadInt(&mon->_mVar7);
	LoadInt(&mon->_mVar8);
	LoadInt(&mon->_mmaxhp);
	LoadInt(&mon->_mhitpoints);
	LoadInt(&mon->_lastx);
	LoadInt(&mon->_lasty);
	LoadInt(&mon->_mRndSeed);
	LoadInt(&mon->_mAISeed);

	LoadByte(&mon->_uniqtype);
	LoadByte(&mon->_uniqtrans);
	LoadByte(&mon->_udeadval);
	LoadByte(&mon->mlid);

	LoadByte(&mon->leader);
	LoadByte(&mon->leaderflag);
	LoadByte(&mon->packsize);
	LoadByte(&mon->falign_CB); // Alignment

	LoadByte(&mon->_mLevel);
	LoadByte(&mon->_mSelFlag);
	LoadByte(&mon->_mAi);
	LoadByte(&mon->_mInt);

	LoadInt(&mon->_mFlags);

	LoadInt16(&mon->_mHit);
	LoadByte(&mon->_mMinDamage);
	LoadByte(&mon->_mMaxDamage);

	LoadInt16(&mon->_mHit2);
	LoadByte(&mon->_mMinDamage2);
	LoadByte(&mon->_mMaxDamage2);

	LoadByte(&mon->_mMagic);
	LoadByte(&mon->_mMagic2);
	LoadByte(&mon->_mArmorClass);
	LoadByte(&mon->_mEvasion);

	LoadByte(&mon->_mAFNum);
	LoadByte(&mon->_mAFNum2);
	LoadInt16(&mon->_mMagicRes);

	LoadInt16(&mon->_mTreasure);
	LoadInt16(&mon->_mExp);

	LoadInt(&mon->mtalkmsg);

	// Omit pointer mName;
	// Omit pointer MType;

	SyncMonsterAnim(mnum);
}

static void LoadTowner(int tnum)
{
	TownerStruct* tw = &towners[tnum];

	LoadInt(&tw->_tAnimCnt);
	LoadInt(&tw->_tAnimFrame);
	LoadInt(&tw->_tAnimFrameCnt);
}

static void LoadMissile(int mi)
{
	MissileStruct* mis = &missile[mi];

	LoadInt(&mis->_miType);
	LoadByte(&mis->_miSubType);
	LoadByte(&mis->_miFlags);
	LoadByte(&mis->_miResist);
	LoadByte(&mis->_miAnimType);
	LoadInt(&mis->_miAnimFlag);
	tbuff += 4; // Skip pointer _miAnimData
	tbuff += 4; // Skip _miAnimFrameLen
	LoadInt(&mis->_miAnimLen);
	LoadInt(&mis->_miAnimWidth);
	LoadInt(&mis->_miAnimXOffset);
	LoadInt(&mis->_miAnimCnt);
	LoadInt(&mis->_miAnimAdd);
	LoadInt(&mis->_miAnimFrame);
	LoadInt(&mis->_miDelFlag);
	LoadInt(&mis->_miDrawFlag);
	LoadInt(&mis->_miLightFlag);
	LoadInt(&mis->_miPreFlag);
	LoadInt(&mis->_miUniqTrans);
	LoadInt(&mis->_misx);
	LoadInt(&mis->_misy);
	LoadInt(&mis->_mix);
	LoadInt(&mis->_miy);
	LoadInt(&mis->_mixoff);
	LoadInt(&mis->_miyoff);
	LoadInt(&mis->_mixvel);
	LoadInt(&mis->_miyvel);
	LoadInt(&mis->_mitxoff);
	LoadInt(&mis->_mityoff);
	LoadInt(&mis->_miDir);
	LoadInt(&mis->_miSpllvl);
	LoadInt(&mis->_miSource);
	LoadInt(&mis->_miCaster);
	LoadInt(&mis->_miMinDam);
	LoadInt(&mis->_miMaxDam);
	LoadInt(&mis->_miRndSeed);
	LoadInt(&mis->_miRange);
	LoadInt(&mis->_miDist);
	LoadInt(&mis->_miLid);
	LoadInt(&mis->_miVar1);
	LoadInt(&mis->_miVar2);
	LoadInt(&mis->_miVar3);
	LoadInt(&mis->_miVar4);
	LoadInt(&mis->_miVar5);
	LoadInt(&mis->_miVar6);
	LoadInt(&mis->_miVar7);
	LoadInt(&mis->_miVar8);
}

static void LoadObject(int oi, bool full)
{
	ObjectStruct* os = &objects[oi];

	LoadInt(&os->_otype);
	LoadInt(&os->_ox);
	LoadInt(&os->_oy);
	LoadInt(&os->_oSFX);
	LoadByte(&os->_oSFXCnt);
	LoadByte(&os->_oAnimFlag);
	tbuff += 2; // Alignment
	tbuff += 4; // Skip pointer _oAnimData
	tbuff += 4; // Skip _oAnimFrameLen
	LoadInt(&os->_oAnimCnt);
	LoadInt(&os->_oAnimLen);
	LoadInt(&os->_oAnimFrame);
	LoadInt(&os->_oAnimWidth);
	LoadInt(&os->_oAnimXOffset);

	LoadByte(&os->_oSolidFlag);
	LoadByte(&os->_oMissFlag);
	LoadByte(&os->_oLightFlag);
	LoadByte(&os->_oBreak);

	LoadByte(&os->_oDoorFlag);
	LoadByte(&os->_oSelFlag);
	LoadByte(&os->_oTrapFlag);
	tbuff += 1; // Alignment

	LoadInt(&os->_oPreFlag);
	LoadInt(&os->_olid);
	LoadInt(&os->_oRndSeed);
	LoadInt(&os->_oVar1);
	LoadInt(&os->_oVar2);
	LoadInt(&os->_oVar3);
	LoadInt(&os->_oVar4);
	LoadInt(&os->_oVar5);
	LoadInt(&os->_oVar6);
	LoadInt(&os->_oVar7);
	LoadInt(&os->_oVar8);

	if (!full) {
		// reset dynamic lights
		os->_olid = -1;
	}
}

static void LoadItem(int ii)
{
	LoadItemData(&items[ii]);
	SyncItemAnim(ii);
}

static void LoadQuest(int i)
{
	QuestStruct* pQuest = &quests[i];

	LoadByte(&pQuest->_qactive);
	LoadByte(&pQuest->_qvar1);
	LoadByte(&pQuest->_qvar2);
	LoadByte(&pQuest->_qlog);
	LoadInt(&pQuest->_qtx);
	LoadInt(&pQuest->_qty);
	LoadInt(&pQuest->_qmsg);
}

static void LoadLight(LightListStruct* pLight)
{
	LoadInt(&pLight->_lx);
	LoadInt(&pLight->_ly);
	LoadInt(&pLight->_lunx);
	LoadInt(&pLight->_luny);
	LoadByte(&pLight->_lradius);
	LoadByte(&pLight->_lunr);
	pLight->_ldel = LoadBool();
	pLight->_lunflag = LoadBool();
	pLight->_lmine = LoadBool();
	tbuff += 3; // Alignment
	LoadInt(&pLight->_xoff);
	LoadInt(&pLight->_yoff);
}

static void LoadPortal(int i)
{
	PortalStruct* pPortal = &portals[i];

	pPortal->_wopen = LoadBool();
	tbuff += 3; // Alignment
	LoadInt(&pPortal->x);
	LoadInt(&pPortal->y);
	LoadInt(&pPortal->level);
}

/*static void RedoPlayerLight()
{
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr._pActive && currLvl._dLevelIdx == plr._pDunLevel)
			ChangeLightXY(plr._plid, plr._px, plr._py);
	}
}*/

static void LoadLevelData(bool full)
{
	int i;

	if (currLvl._dType != DTYPE_TOWN) {
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dDead);
		if (!full)
			SyncDeadLight();
	}

	LoadInt(&nummonsters);
	if (full)
		LoadInt(&nummissiles);
	LoadInt(&numobjects);
	LoadInt(&numitems);

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			LoadInt(&monstactive[i]);
		for (i = 0; i < nummonsters; i++)
			LoadMonster(monstactive[i]);
		if (full) {
			static_assert(MAXMISSILES <= UCHAR_MAX, "LoadLevelData handles missile-ids as bytes.");
			for (i = 0; i < MAXMISSILES; i++)
				LoadByte(&missileactive[i]);
			for (i = 0; i < MAXMISSILES; i++)
				LoadByte(&missileavail[i]);
			for (i = 0; i < nummissiles; i++)
				LoadMissile(missileactive[i]);
		}
		static_assert(MAXOBJECTS <= UCHAR_MAX, "LoadLevelData handles object-ids as bytes.");
		for (i = 0; i < MAXOBJECTS; i++)
			LoadByte(&objectactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			LoadByte(&objectavail[i]);
		for (i = 0; i < numobjects; i++)
			LoadObject(objectactive[i], full);
		// run in a separate loop because objects (e.g. crux) might depend on each other
		for (i = 0; i < numobjects; i++)
			SyncObjectAnim(objectactive[i]);
	}
	static_assert(MAXITEMS <= UCHAR_MAX, "LoadLevelData handles item-ids as bytes.");
	for (i = 0; i < MAXITEMS; i++)
		LoadByte(&itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		LoadByte(&itemavail[i]);
	for (i = 0; i < numitems; i++)
		LoadItem(itemactive[i]);

	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dFlags);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dItem);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPreLight);
	if (full) {
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dLight);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPlayer);
	}

	if (currLvl._dType != DTYPE_TOWN) {
		LoadInts(&dMonster[0][0], MAXDUNX * MAXDUNY);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dObject);
		CopyBytes(tbuff, DMAXX * DMAXY, automapview);
		if (full)
			CopyBytes(tbuff, MAXDUNX * MAXDUNY, dMissile);
		else {
			memset(dMissile, 0, sizeof(dMissile));
			assert(nummissiles == 0);
		}
	}
}

/**
 * @brief Load game state
 */
void LoadGame()
{
	int i;
	BYTE* fileBuff;
	int _ViewX, _ViewY;

	FreeLevelMem();
	// TODO: UIDisconnectGame() ?
	SNetLeaveGame(LEAVE_UNKNOWN);

	pfile_delete_save_file(false);
	pfile_read_save_file(true);
	fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	LoadInt(&i);
	if (i != SAVE_INITIAL)
		app_fatal("Invalid save file");
	// load game-info
	LoadInt32(&gdwGameLogicTurn);
	LoadInt32(&sgbSentThisCycle);
	LoadInt(&i);
	gnDifficulty = (i >> 8) & 0xFF;
	currLvl._dLevelIdx = i & 0xFF;
	EnterLevel(i & 0xFF);
	for (i = 0; i < NUM_LEVELS; i++) {
		LoadInt(&glSeedTbl[i]);
	}
	// load player-data
	LoadInt(&_ViewX);
	LoadInt(&_ViewY);
	LoadInt(&ScrollInfo._sdx);
	LoadInt(&ScrollInfo._sdy);
	LoadInt(&ScrollInfo._sxoff);
	LoadInt(&ScrollInfo._syoff);
	LoadInt(&ScrollInfo._sdir);
	gbZoomInFlag = LoadBool();
	gbInvflag = LoadBool();
	gbChrflag = LoadBool();
	gbLvlUp = LoadBool();
	LoadInt(&gnHPPer);
	LoadInt(&gnManaPer);

	gbAutomapflag = LoadBool();
	tbuff += 3; // Alignment
	LoadInt(&AutoMapScale);
	LoadInt(&AutoMapXOfs);
	LoadInt(&AutoMapYOfs);

	LoadPlayer(mypnum);

	// load meta-data I.
	LoadInt(&gnReturnLvlX);
	LoadInt(&gnReturnLvlY);
	LoadInt(&gnReturnLvl);
	LoadByte(&gbTownWarps);
	LoadByte(&gbWaterDone);
	LoadByte(&gbDungMsgs);
	LoadByte(&gbDungMsgs2);
	LoadInt32(&guLvlVisited);
	// load meta-data II. (used by LoadGameLevel)
	for (i = 0; i < NUM_QUESTS; i++)
		LoadQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		LoadPortal(i);
	// load level-data
	LoadGameLevel(ENTRY_LOAD);
	ViewX = _ViewX;
	ViewY = _ViewY;
	LoadLevelData(true);

	// load meta-data III. (modified by LoadGameLevel)
	//LoadInt(&numtowners);
	LoadInt(&boylevel);
	LoadInt(&numpremium);
	LoadInt(&premiumlevel);
	LoadInt(&numlights);
	LoadInt(&numvision);
	//LoadByte(&numtrans);
	//tbuff += 3; // Alignment

	for (i = 0; i < MAXLIGHTS; i++)
		LoadByte(&lightactive[i]);
	for (i = 0; i < numlights; i++)
		LoadLight(&LightList[lightactive[i]]);

	for (i = 0; i < MAXVISION; i++)
		LoadByte(&visionactive[i]);
	for (i = 0; i < numvision; i++)
		LoadLight(&VisionList[visionactive[i]]);

	CopyBytes(tbuff, sizeof(TransList), TransList);

	LoadItemData(&boyitem);
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		LoadItemData(&premiumitems[i]);
	if (currLvl._dType == DTYPE_TOWN) {
		for (i = 0; i < SMITH_ITEMS; i++)
			LoadItemData(&smithitem[i]);
		for (i = 0; i < HEALER_ITEMS; i++)
			LoadItemData(&healitem[i]);
		for (i = 0; i < WITCH_ITEMS; i++)
			LoadItemData(&witchitem[i]);
		for (i = 0; i < MAX_TOWNERS; i++)
			LoadTowner(i);
	}

	InitAutomapScale();
	//ResyncQuests();

	//RedoPlayerLight();
	//ProcessLightList();
	//RedoPlayerVision();
	//ProcessVisionList();

	SyncMissilesAnim();
	CalcViewportGeometry();

	sgbSentThisCycle--;
	nthread_send_turn();
}

static void SaveItemData(ItemStruct* is)
{
	SaveInt(&is->_iSeed);
	SaveInt16(&is->_iCreateInfo);
	SaveInt16(&is->_iIdx);
	SaveInt(&is->_ix);
	SaveInt(&is->_iy);
	SaveInt(&is->_iCurs);
	SaveInt(&is->_itype);
	SaveInt(&is->_iMiscId);
	SaveInt(&is->_iSpell);
	SaveByte(&is->_iClass);
	SaveByte(&is->_iLoc);
	SaveByte(&is->_iDamType);
	SaveByte(&is->_iMinDam);
	SaveByte(&is->_iMaxDam);
	SaveByte(&is->_iBaseCrit);
	SaveByte(&is->_iMinStr);
	SaveByte(&is->_iMinMag);
	SaveByte(&is->_iMinDex);
	tbuff += 3; // Alignment
	SaveInt(&is->_iAnimFlag);
	tbuff += 4; // Skip pointer _iAnimData
	tbuff += 4; // Skip _iAnimFrameLen
	SaveInt(&is->_iAnimCnt);
	SaveInt(&is->_iAnimLen);
	SaveInt(&is->_iAnimFrame);
	//SaveInt(&is->_iAnimWidth);
	//SaveInt(&is->_iAnimXOffset);
	SaveInt(&is->_iPostDraw);
	SaveInt(&is->_iIdentified);
	CopyBytes(is->_iName, sizeof(is->_iName), tbuff);
	SaveByte(&is->_iPrePower);
	SaveByte(&is->_iSufPower);
	SaveByte(&is->_iSelFlag);
	SaveByte(&is->_iMagical);
	SaveInt(&is->_ivalue);
	SaveInt(&is->_iIvalue);
	SaveInt(&is->_iAC);
	SaveInt(&is->_iFlags);
	SaveInt(&is->_iCharges);
	SaveInt(&is->_iMaxCharges);
	SaveInt(&is->_iDurability);
	SaveInt(&is->_iMaxDur);
	SaveInt(&is->_iPLDam);
	SaveInt(&is->_iPLToHit);
	SaveInt(&is->_iPLAC);
	SaveInt(&is->_iPLStr);
	SaveInt(&is->_iPLMag);
	SaveInt(&is->_iPLDex);
	SaveInt(&is->_iPLVit);
	SaveInt(&is->_iPLFR);
	SaveInt(&is->_iPLLR);
	SaveInt(&is->_iPLMR);
	SaveInt(&is->_iPLAR);
	SaveInt(&is->_iPLMana);
	SaveInt(&is->_iPLHP);
	SaveInt(&is->_iPLDamMod);
	SaveInt(&is->_iPLGetHit);
	SaveByte(&is->_iPLLight);
	SaveByte(&is->_iSplLvlAdd);
	SaveByte(&is->_iManaSteal);
	SaveByte(&is->_iLifeSteal);
	SaveByte(&is->_iPLCrit);
	tbuff += 3; // Alignment
	SaveInt(&is->_iUid);
	SaveByte(&is->_iFMinDam);
	SaveByte(&is->_iFMaxDam);
	SaveByte(&is->_iLMinDam);
	SaveByte(&is->_iLMaxDam);
	SaveByte(&is->_iMMinDam);
	SaveByte(&is->_iMMaxDam);
	SaveByte(&is->_iAMinDam);
	SaveByte(&is->_iAMaxDam);
	SaveInt(&is->_iVAdd);
	SaveInt(&is->_iVMult);
	SaveInt(&is->_iStatFlag);
}

static void SaveItems(ItemStruct* pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		SaveItemData(&pItem[i]);
	}
}

static void SavePlayer(int pnum)
{
	SaveInt(&plr._pmode);
	static_assert(sizeof(plr.walkpath) == MAX_PATH_LENGTH + 1, "Save files are no longer compatible.");
	CopyBytes(plr.walkpath, lengthof(plr.walkpath), tbuff);
	SaveInt(&plr.destAction);
	SaveInt(&plr.destParam1);
	SaveInt(&plr.destParam2);
	SaveInt(&plr.destParam3);
	SaveInt(&plr.destParam4);
	SaveByte(&plr._pActive);
	SaveByte(&plr._pInvincible);
	SaveByte(&plr._pLvlChanging);
	SaveByte(&plr._pDunLevel);
	SaveByte(&plr._pClass);
	SaveByte(&plr._pLevel);
	SaveByte(&plr._pRank);
	SaveByte(&plr._pTeam);
	SaveInt16(&plr._pStatPts);
	SaveByte(&plr._pLightRad);
	SaveByte(&plr._pManaShield);
	SaveInt16(&plr._pTimer[PLTR_INFRAVISION]);
	SaveInt16(&plr._pTimer[PLTR_RAGE]);
	SaveInt(&plr._pExperience);
	SaveInt(&plr._pNextExper);
	SaveInt(&plr._px);
	SaveInt(&plr._py);
	SaveInt(&plr._pfutx);
	SaveInt(&plr._pfuty);
	SaveInt(&plr._poldx);
	SaveInt(&plr._poldy);
	SaveInt(&plr._pxoff);
	SaveInt(&plr._pyoff);
	SaveInt(&plr._pxvel);
	SaveInt(&plr._pyvel);
	SaveInt(&plr._pdir);
	tbuff += 4; // Skip pointer _pAnimData
	tbuff += 4; // Skip _pAnimFrameLen
	SaveInt(&plr._pAnimCnt);
	SaveInt(&plr._pAnimLen);
	SaveInt(&plr._pAnimFrame);
	SaveInt(&plr._pAnimWidth);
	SaveInt(&plr._pAnimXOffset);
	SaveInt(&plr._plid);
	SaveInt(&plr._pvid);

	SaveByte(&plr._pAtkSkill);
	SaveByte(&plr._pAtkSkillType);
	SaveByte(&plr._pMoveSkill);
	SaveByte(&plr._pMoveSkillType);

	SaveByte(&plr._pAltAtkSkill);
	SaveByte(&plr._pAltAtkSkillType);
	SaveByte(&plr._pAltMoveSkill);
	SaveByte(&plr._pAltMoveSkillType);

	CopyBytes(plr._pAtkSkillHotKey, lengthof(plr._pAtkSkillHotKey), tbuff);
	CopyBytes(plr._pAtkSkillTypeHotKey, lengthof(plr._pAtkSkillTypeHotKey), tbuff);
	CopyBytes(plr._pMoveSkillHotKey, lengthof(plr._pMoveSkillHotKey), tbuff);
	CopyBytes(plr._pMoveSkillTypeHotKey, lengthof(plr._pMoveSkillTypeHotKey), tbuff);

	CopyBytes(plr._pAltAtkSkillHotKey, lengthof(plr._pAltAtkSkillHotKey), tbuff);
	CopyBytes(plr._pAltAtkSkillTypeHotKey, lengthof(plr._pAltAtkSkillTypeHotKey), tbuff);
	CopyBytes(plr._pAltMoveSkillHotKey, lengthof(plr._pAltMoveSkillHotKey), tbuff);
	CopyBytes(plr._pAltMoveSkillTypeHotKey, lengthof(plr._pAltMoveSkillTypeHotKey), tbuff);

	CopyBytes(plr._pSkillLvl, lengthof(plr._pSkillLvl), tbuff);
	CopyBytes(plr._pSkillActivity, lengthof(plr._pSkillActivity), tbuff);
	SaveInts(plr._pSkillExp, lengthof(plr._pSkillExp));
	SaveInt64(&plr._pMemSkills);
	SaveInt64(&plr._pAblSkills);
	SaveInt64(&plr._pScrlSkills);
	CopyBytes(plr._pName, PLR_NAME_LEN, tbuff);

	SaveInt16(&plr._pBaseStr);
	SaveInt16(&plr._pBaseMag);
	SaveInt16(&plr._pBaseDex);
	SaveInt16(&plr._pBaseVit);
	SaveInt(&plr._pHPBase);
	SaveInt(&plr._pMaxHPBase);
	SaveInt(&plr._pManaBase);
	SaveInt(&plr._pMaxManaBase);

	SaveInt(&plr._pStrength);
	SaveInt(&plr._pMagic);
	SaveInt(&plr._pDexterity);
	SaveInt(&plr._pVitality);
	SaveInt(&plr._pHitPoints);
	SaveInt(&plr._pMaxHP);
	SaveInt(&plr._pMana);
	SaveInt(&plr._pMaxMana);

	SaveInt(&plr._pVar1);
	SaveInt(&plr._pVar2);
	SaveInt(&plr._pVar3);
	SaveInt(&plr._pVar4);
	SaveInt(&plr._pVar5);
	SaveInt(&plr._pVar6);
	SaveInt(&plr._pVar7);
	SaveInt(&plr._pVar8);

	//tbuff += 4; // Skip _pGFXLoad
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pNAnim
	SaveInt(&plr._pNFrames);
	SaveInt(&plr._pNWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pWAnim
	SaveInt(&plr._pWFrames);
	SaveInt(&plr._pWWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pAAnim
	SaveInt(&plr._pAFrames);
	SaveInt(&plr._pAWidth);
	SaveInt(&plr._pAFNum);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pLAnim
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pFAnim
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pTAnim
	SaveInt(&plr._pSFrames);
	SaveInt(&plr._pSWidth);
	SaveInt(&plr._pSFNum);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pHAnim
	SaveInt(&plr._pHFrames);
	SaveInt(&plr._pHWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pDAnim
	SaveInt(&plr._pDFrames);
	SaveInt(&plr._pDWidth);
	//tbuff += 4 * NUM_DIRS; // Skip pointers _pBAnim
	SaveInt(&plr._pBFrames);
	SaveInt(&plr._pBWidth);

	SaveItemData(&plr._pHoldItem);
	SaveItems(plr._pInvBody, NUM_INVLOC);
	SaveItems(plr._pSpdList, MAXBELTITEMS);
	SaveItems(plr._pInvList, NUM_INV_GRID_ELEM);
	SaveInt(&plr._pGold);

	/*Skip to Calc
	tbuff += 1; // _pInfraFlag
	tbuff += 1; // _pgfxnum
	tbuff += 1; // _pHasUnidItem
	tbuff += 1; // Alignment
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
	tbuff += 1; // _pSkillFlags
	tbuff += 1; // _pIBaseHitBonus
	tbuff += 1; // _pICritChance
	tbuff += 1; // _pIBlockChance

	tbuff += 8; // _pISpells
	tbuff += 4; // _pIFlags
	tbuff += 4; // _pIFlags2
	tbuff += 4; // _pIGetHit
	tbuff += 1; // _pISplLvlAdd
	tbuff += 1; // _pIArrowVelBonus
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

	// Omit pointer _pNData
	// Omit pointer _pWData
	// Omit pointer _pAData
	// Omit pointer _pLData
	// Omit pointer _pFData
	// Omit pointer  _pTData
	// Omit pointer _pHData
	// Omit pointer _pDData
	// Omit pointer _pBData
	// Omit pointer alignment
}

static void SaveMonster(int mnum)
{
	MonsterStruct* mon = &monsters[mnum];

	SaveInt(&mon->_mmode);
	SaveInt(&mon->_msquelch);
	SaveByte(&mon->_mMTidx);
	SaveByte(&mon->_mpathcount);
	SaveByte(&mon->_mWhoHit);
	SaveByte(&mon->_mgoal);
	SaveInt(&mon->_mgoalvar1);
	SaveInt(&mon->_mgoalvar2);
	SaveInt(&mon->_mgoalvar3);
	SaveInt(&mon->_mx);
	SaveInt(&mon->_my);
	SaveInt(&mon->_mfutx);
	SaveInt(&mon->_mfuty);
	SaveInt(&mon->_moldx);
	SaveInt(&mon->_moldy);
	SaveInt(&mon->_mxoff);
	SaveInt(&mon->_myoff);
	SaveInt(&mon->_mxvel);
	SaveInt(&mon->_myvel);
	SaveInt(&mon->_mdir);
	SaveInt(&mon->_menemy);
	SaveByte(&mon->_menemyx);
	SaveByte(&mon->_menemyy);
	SaveByte(&mon->_mListener);
	tbuff += 1; // Alignment

	tbuff += 4; // Skip pointer _mAnimData
	tbuff += 4; // Skip _mAnimFrameLen
	SaveInt(&mon->_mAnimCnt);
	SaveInt(&mon->_mAnimLen);
	SaveInt(&mon->_mAnimFrame);
	SaveInt(&mon->_mDelFlag);
	SaveInt(&mon->_mVar1);
	SaveInt(&mon->_mVar2);
	SaveInt(&mon->_mVar3);
	SaveInt(&mon->_mVar4);
	SaveInt(&mon->_mVar5);
	SaveInt(&mon->_mVar6);
	SaveInt(&mon->_mVar7);
	SaveInt(&mon->_mVar8);
	SaveInt(&mon->_mmaxhp);
	SaveInt(&mon->_mhitpoints);
	SaveInt(&mon->_lastx);
	SaveInt(&mon->_lasty);
	SaveInt(&mon->_mRndSeed);
	SaveInt(&mon->_mAISeed);

	SaveByte(&mon->_uniqtype);
	SaveByte(&mon->_uniqtrans);
	SaveByte(&mon->_udeadval);
	SaveByte(&mon->mlid);

	SaveByte(&mon->leader);
	SaveByte(&mon->leaderflag);
	SaveByte(&mon->packsize);
	SaveByte(&mon->falign_CB); // Alignment

	SaveByte(&mon->_mLevel);
	SaveByte(&mon->_mSelFlag);
	SaveByte(&mon->_mAi);
	SaveByte(&mon->_mInt);

	SaveInt(&mon->_mFlags);

	SaveInt16(&mon->_mHit);
	SaveByte(&mon->_mMinDamage);
	SaveByte(&mon->_mMaxDamage);

	SaveInt16(&mon->_mHit2);
	SaveByte(&mon->_mMinDamage2);
	SaveByte(&mon->_mMaxDamage2);

	SaveByte(&mon->_mMagic);
	SaveByte(&mon->_mMagic2);
	SaveByte(&mon->_mArmorClass);
	SaveByte(&mon->_mEvasion);

	SaveByte(&mon->_mAFNum);
	SaveByte(&mon->_mAFNum2);
	SaveInt16(&mon->_mMagicRes);

	SaveInt16(&mon->_mTreasure);
	SaveInt16(&mon->_mExp);

	SaveInt(&mon->mtalkmsg);

	// Omit pointer mName;
	// Omit pointer MType;
}

static void SaveTowner(int tnum)
{
	TownerStruct* tw = &towners[tnum];

	SaveInt(&tw->_tAnimCnt);
	SaveInt(&tw->_tAnimFrame);
	SaveInt(&tw->_tAnimFrameCnt);
}

static void SaveMissile(int mi)
{
	MissileStruct* mis = &missile[mi];

	SaveInt(&mis->_miType);
	SaveByte(&mis->_miSubType);
	SaveByte(&mis->_miFlags);
	SaveByte(&mis->_miResist);
	SaveByte(&mis->_miAnimType);
	SaveInt(&mis->_miAnimFlag);
	tbuff += 4; // Skip pointer _miAnimData
	tbuff += 4; // Skip _miAnimFrameLen
	SaveInt(&mis->_miAnimLen);
	SaveInt(&mis->_miAnimWidth);
	SaveInt(&mis->_miAnimXOffset);
	SaveInt(&mis->_miAnimCnt);
	SaveInt(&mis->_miAnimAdd);
	SaveInt(&mis->_miAnimFrame);
	SaveInt(&mis->_miDelFlag);
	SaveInt(&mis->_miDrawFlag);
	SaveInt(&mis->_miLightFlag);
	SaveInt(&mis->_miPreFlag);
	SaveInt(&mis->_miUniqTrans);
	SaveInt(&mis->_misx);
	SaveInt(&mis->_misy);
	SaveInt(&mis->_mix);
	SaveInt(&mis->_miy);
	SaveInt(&mis->_mixoff);
	SaveInt(&mis->_miyoff);
	SaveInt(&mis->_mixvel);
	SaveInt(&mis->_miyvel);
	SaveInt(&mis->_mitxoff);
	SaveInt(&mis->_mityoff);
	SaveInt(&mis->_miDir);
	SaveInt(&mis->_miSpllvl);
	SaveInt(&mis->_miSource);
	SaveInt(&mis->_miCaster);
	SaveInt(&mis->_miMinDam);
	SaveInt(&mis->_miMaxDam);
	SaveInt(&mis->_miRndSeed);
	SaveInt(&mis->_miRange);
	SaveInt(&mis->_miDist);
	SaveInt(&mis->_miLid);
	SaveInt(&mis->_miVar1);
	SaveInt(&mis->_miVar2);
	SaveInt(&mis->_miVar3);
	SaveInt(&mis->_miVar4);
	SaveInt(&mis->_miVar5);
	SaveInt(&mis->_miVar6);
	SaveInt(&mis->_miVar7);
	SaveInt(&mis->_miVar8);
}

static void SaveObject(int oi)
{
	ObjectStruct* os = &objects[oi];

	SaveInt(&os->_otype);
	SaveInt(&os->_ox);
	SaveInt(&os->_oy);
	SaveInt(&os->_oSFX);
	SaveByte(&os->_oSFXCnt);
	SaveByte(&os->_oAnimFlag);
	tbuff += 2; // Alignment
	tbuff += 4; // Skip pointer _oAnimData
	tbuff += 4; // Skip _oAnimFrameLen
	SaveInt(&os->_oAnimCnt);
	SaveInt(&os->_oAnimLen);
	SaveInt(&os->_oAnimFrame);
	SaveInt(&os->_oAnimWidth);
	SaveInt(&os->_oAnimXOffset);

	SaveByte(&os->_oSolidFlag);
	SaveByte(&os->_oMissFlag);
	SaveByte(&os->_oLightFlag);
	SaveByte(&os->_oBreak);

	SaveByte(&os->_oDoorFlag);
	SaveByte(&os->_oSelFlag);
	SaveByte(&os->_oTrapFlag);
	tbuff += 1; // Alignment

	SaveInt(&os->_oPreFlag);
	SaveInt(&os->_olid);
	SaveInt(&os->_oRndSeed);
	SaveInt(&os->_oVar1);
	SaveInt(&os->_oVar2);
	SaveInt(&os->_oVar3);
	SaveInt(&os->_oVar4);
	SaveInt(&os->_oVar5);
	SaveInt(&os->_oVar6);
	SaveInt(&os->_oVar7);
	SaveInt(&os->_oVar8);
}

static void SaveQuest(int i)
{
	QuestStruct* pQuest = &quests[i];

	SaveByte(&pQuest->_qactive);
	SaveByte(&pQuest->_qvar1);
	SaveByte(&pQuest->_qvar2);
	SaveByte(&pQuest->_qlog);
	SaveInt(&pQuest->_qtx);
	SaveInt(&pQuest->_qty);
	SaveInt(&pQuest->_qmsg);
}

static void SaveLight(LightListStruct* pLight)
{
	SaveInt(&pLight->_lx);
	SaveInt(&pLight->_ly);
	SaveInt(&pLight->_lunx);
	SaveInt(&pLight->_luny);
	SaveByte(&pLight->_lradius);
	SaveByte(&pLight->_lunr);
	SaveBool(pLight->_ldel);
	SaveBool(pLight->_lunflag);
	SaveBool(pLight->_lmine);
	tbuff += 3; // Alignment
	SaveInt(&pLight->_xoff);
	SaveInt(&pLight->_yoff);
}

static void SavePortal(int i)
{
	PortalStruct* pPortal = &portals[i];

	SaveBool(pPortal->_wopen);
	tbuff += 3; // Alignment
	SaveInt(&pPortal->x);
	SaveInt(&pPortal->y);
	SaveInt(&pPortal->level);
}

static void SaveLevelData(bool full)
{
	int i;

	if (currLvl._dType != DTYPE_TOWN) {
		CopyBytes(dDead, MAXDUNX * MAXDUNY, tbuff);
	}

	SaveInt(&nummonsters);
	if (full)
		SaveInt(&nummissiles);
	SaveInt(&numobjects);
	SaveInt(&numitems);

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			SaveInt(&monstactive[i]);
		for (i = 0; i < nummonsters; i++)
			SaveMonster(monstactive[i]);
		if (full) {
			for (i = 0; i < MAXMISSILES; i++)
				SaveByte(&missileactive[i]);
			for (i = 0; i < MAXMISSILES; i++)
				SaveByte(&missileavail[i]);
			for (i = 0; i < nummissiles; i++)
				SaveMissile(missileactive[i]);
		}
		for (i = 0; i < MAXOBJECTS; i++)
			SaveByte(&objectactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			SaveByte(&objectavail[i]);
		for (i = 0; i < numobjects; i++)
			SaveObject(objectactive[i]);
	}
	for (i = 0; i < MAXITEMS; i++)
		SaveByte(&itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		SaveByte(&itemavail[i]);
	for (i = 0; i < numitems; i++)
		SaveItemData(&items[itemactive[i]]);
	CopyBytes(dFlags, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dItem, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dPreLight, MAXDUNX * MAXDUNY, tbuff);
	if (full) {
		CopyBytes(dLight, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(dPlayer, MAXDUNX * MAXDUNY, tbuff);
	}

	if (currLvl._dType != DTYPE_TOWN) {
		SaveInts(&dMonster[0][0], MAXDUNX * MAXDUNY);
		CopyBytes(dObject, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(automapview, DMAXX * DMAXY, tbuff);
		if (full)
			CopyBytes(dMissile, MAXDUNX * MAXDUNY, tbuff);
	}
}

void SaveGame()
{
	int i;
	BYTE* fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	constexpr size_t ss = 4 + 12 + 4 * NUM_LEVELS + 56 + 14004 + 20 + 16 * NUM_QUESTS + 16 * MAXPORTAL;
	// initial
	i = SAVE_INITIAL;
	SaveInt(&i);
	// save game-info
	SaveInt32(&gdwGameLogicTurn);
	SaveInt32(&sgbSentThisCycle);
	i = (gnDifficulty << 8) | currLvl._dLevelIdx;
	SaveInt(&i);
	for (i = 0; i < NUM_LEVELS; i++) {
		SaveInt(&glSeedTbl[i]);
	}
	// save player-data
	SaveInt(&ViewX);
	SaveInt(&ViewY);
	SaveInt(&ScrollInfo._sdx);
	SaveInt(&ScrollInfo._sdy);
	SaveInt(&ScrollInfo._sxoff);
	SaveInt(&ScrollInfo._syoff);
	SaveInt(&ScrollInfo._sdir);
	SaveBool(gbZoomInFlag);
	SaveBool(gbInvflag);
	SaveBool(gbChrflag);
	SaveBool(gbLvlUp);
	SaveInt(&gnHPPer);
	SaveInt(&gnManaPer);

	SaveBool(gbAutomapflag);
	tbuff += 3; // Alignment
	SaveInt(&AutoMapScale);
	SaveInt(&AutoMapXOfs);
	SaveInt(&AutoMapYOfs);

	SavePlayer(mypnum);

	// save meta-data I.
	SaveInt(&gnReturnLvlX);
	SaveInt(&gnReturnLvlY);
	SaveInt(&gnReturnLvl);
	SaveByte(&gbTownWarps);
	SaveByte(&gbWaterDone);
	SaveByte(&gbDungMsgs);
	SaveByte(&gbDungMsgs2);
	SaveInt32(&guLvlVisited);
	// save meta-data II. (used by LoadGameLevel)
	for (i = 0; i < NUM_QUESTS; i++)
		SaveQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		SavePortal(i);
	// save level-data
	constexpr size_t slt = /*112 * 112 +*/ 16 + /*MAXMONSTERS * 4 + MAXMONSTERS * 196 + 2 * MAXMISSILES
	 + MAXMISSILES * 176 + 2 * MAXOBJECTS + MAXOBJECTS * 100*/ + 2 * MAXITEMS
	 + MAXITEMS * 236 + 112 * 112 + 112 * 112 + 112 * 112 + 112 * 112 + 112 * 112
	/* + 112 * 112 * 4 + 112 * 112 + 40 * 40 + 112 * 112*/;
	constexpr size_t sld = (112 * 112) + 16 + (MAXMONSTERS * 4 + MAXMONSTERS * 196 + 2 * MAXMISSILES
	 + MAXMISSILES * 176 + 2 * MAXOBJECTS + MAXOBJECTS * 100) + 2 * MAXITEMS
	 + MAXITEMS * 236 + 112 * 112 + 112 * 112 + 112 * 112 + 112 * 112 + 112 * 112
	 + (112 * 112 * 4 + 112 * 112 + 40 * 40 + 112 * 112);
	SaveLevelData(true);

	// save meta-data III. (modified by LoadGameLevel)
	constexpr size_t smt = 5 * 4 + MAXLIGHTS + 32 * MAXLIGHTS + MAXVISION + 32 * MAXVISION + 256
	 + 236 + SMITH_PREMIUM_ITEMS * 236 + (SMITH_ITEMS * 236 + HEALER_ITEMS * 236
	 + WITCH_ITEMS * 236 + 12 * MAX_TOWNERS);
	constexpr size_t smd = 5 * 4 + MAXLIGHTS + 32 * MAXLIGHTS + MAXVISION + 32 * MAXVISION + 256
	 + 236 + SMITH_PREMIUM_ITEMS * 236 /*+ SMITH_ITEMS * 236 + HEALER_ITEMS * 236
	 + WITCH_ITEMS * 236 + 12 * MAX_TOWNERS*/;
	//SaveInt(&numtowners);
	SaveInt(&boylevel);
	SaveInt(&numpremium);
	SaveInt(&premiumlevel);
	SaveInt(&numlights);
	SaveInt(&numvision);
	//SaveByte(&numtrans);
	//tbuff += 3; // Alignment

	for (i = 0; i < MAXLIGHTS; i++)
		SaveByte(&lightactive[i]);
	for (i = 0; i < numlights; i++)
		SaveLight(&LightList[lightactive[i]]);

	for (i = 0; i < MAXVISION; i++)
		SaveByte(&visionactive[i]);
	for (i = 0; i < numvision; i++)
		SaveLight(&VisionList[visionactive[i]]);

	CopyBytes(TransList, sizeof(TransList), tbuff);

	SaveItemData(&boyitem);
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		SaveItemData(&premiumitems[i]);
	if (currLvl._dType == DTYPE_TOWN) {
		for (i = 0; i < SMITH_ITEMS; i++)
			SaveItemData(&smithitem[i]);
		for (i = 0; i < HEALER_ITEMS; i++)
			SaveItemData(&healitem[i]);
		for (i = 0; i < WITCH_ITEMS; i++)
			SaveItemData(&witchitem[i]);
		for (i = 0; i < MAX_TOWNERS; i++)
			SaveTowner(i);
	}

	constexpr size_t tst = ss + slt + smt;
	constexpr size_t tsd = ss + sld + smd;
	constexpr size_t mss = sizeof(gsDeltaData.ddBuffer) - SHA1BlockSize - 8/*sizeof(CodecSignature)*/;
	static_assert(tst < mss, "Town might not fit to the preallocated buffer.");
	static_assert(tsd < mss, "Dungeon might not fit to the preallocated buffer.");
	assert(tbuff - fileBuff < mss);
	pfile_write_save_file(true, tbuff - fileBuff);
	gbValidSaveFile = true;
	pfile_rename_temp_to_perm();
	pfile_write_hero(true);
}

void SaveLevel()
{
	BYTE* fileBuff;

	if (currLvl._dLevelIdx == DLV_TOWN)
		glSeedTbl[DLV_TOWN] = GetRndSeed();

	fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	SaveLevelData(false);

	assert(tbuff - fileBuff < sizeof(gsDeltaData.ddBuffer) - SHA1BlockSize - 8/*sizeof(CodecSignature)*/);
	pfile_write_save_file(false, tbuff - fileBuff);
}

void LoadLevel()
{
	int i;
	BYTE *fileBuff, *tmp;

	pfile_read_save_file(false);
	fileBuff = gsDeltaData.ddBuffer;
	tbuff = fileBuff;

	LoadLevelData(false);

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
