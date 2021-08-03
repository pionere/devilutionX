/**
 * @file loadsave.cpp
 *
 * Implementation of save game functionality.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
#define SAVE_INITIAL 'HELF'
#else
#define SAVE_INITIAL 'RETL'
#endif

BYTE *tbuff;

static char LoadChar()
{
	return *tbuff++;
}

static int LoadInt()
{
	int rv = *tbuff++ << 24;
	rv |= *tbuff++ << 16;
	rv |= *tbuff++ << 8;
	rv |= *tbuff++;

	return rv;
}

static bool LoadBool()
{
	if (*tbuff++)
		return true;
	else
		return false;
}

static void CopyBytes(const void *src, const int n, void *dst)
{
	memcpy(dst, src, n);
	tbuff += n;
}

static void CopyChar(const void *src, void *dst)
{
	*(char *)dst = *(char *)src;
	tbuff += 1;
}

static void CopyShort(const void *src, void *dst)
{
	uint16_t buf;
	memcpy(&buf, src, 2);
	tbuff += 2;
	buf = SwapLE16(buf);
	memcpy(dst, &buf, 2);
}

/*static void CopyShorts(const void *src, const int n, void *dst)
{
	const uint16_t *s = reinterpret_cast<const uint16_t *>(src);
	uint16_t *d = reinterpret_cast<uint16_t *>(dst);
	for (int i = 0; i < n; i++) {
		CopyShort(s, d);
		++d;
		++s;
	}
}*/

static void CopyInt(const void *src, void *dst)
{
	unsigned int buf;
	memcpy(&buf, src, 4);
	tbuff += 4;
	buf = SwapLE32(buf);
	memcpy(dst, &buf, 4);
}

static void CopyInts(const void *src, const int n, void *dst)
{
	const unsigned int *s = reinterpret_cast<const unsigned int *>(src);
	const unsigned int *d = reinterpret_cast<unsigned int *>(dst);
	for (int i = 0; i < n; i++) {
		CopyInt(s, (void *)d);
		++d;
		++s;
	}
}

static void CopyInt64(const void *src, void *dst)
{
	unsigned long long buf;
	memcpy(&buf, src, 8);
	tbuff += 8;
	buf = SwapLE64(buf);
	memcpy(dst, &buf, 8);
}

static void LoadItemData(ItemStruct *is)
{
	CopyInt(tbuff, &is->_iSeed);
	CopyShort(tbuff, &is->_iCreateInfo);
	CopyShort(tbuff, &is->_iIdx);
	CopyInt(tbuff, &is->_itype);
	CopyInt(tbuff, &is->_ix);
	CopyInt(tbuff, &is->_iy);
	CopyInt(tbuff, &is->_iAnimFlag);
	tbuff += 4; // Skip pointer _iAnimData
	tbuff += 4; // Skip _iAnimFrameLen
	CopyInt(tbuff, &is->_iAnimCnt);
	CopyInt(tbuff, &is->_iAnimLen);
	CopyInt(tbuff, &is->_iAnimFrame);
	tbuff += 4; // CopyInt(tbuff, &is->_iAnimWidth);
	tbuff += 4; // CopyInt(tbuff, &is->_iAnimXOffset);
	CopyInt(tbuff, &is->_iPostDraw);
	CopyInt(tbuff, &is->_iIdentified);
	CopyBytes(tbuff, sizeof(is->_iName), is->_iName);
	CopyChar(tbuff, &is->_iSelFlag);
	CopyChar(tbuff, &is->_iMagical);
	CopyChar(tbuff, &is->_iLoc);
	CopyChar(tbuff, &is->_iClass);
	CopyInt(tbuff, &is->_iCurs);
	CopyInt(tbuff, &is->_ivalue);
	CopyInt(tbuff, &is->_iIvalue);
	CopyChar(tbuff, &is->_iMinDam);
	CopyChar(tbuff, &is->_iMaxDam);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &is->_iAC);
	CopyInt(tbuff, &is->_iFlags);
	CopyInt(tbuff, &is->_iMiscId);
	CopyInt(tbuff, &is->_iSpell);
	CopyInt(tbuff, &is->_iCharges);
	CopyInt(tbuff, &is->_iMaxCharges);
	CopyInt(tbuff, &is->_iDurability);
	CopyInt(tbuff, &is->_iMaxDur);
	CopyInt(tbuff, &is->_iPLDam);
	CopyInt(tbuff, &is->_iPLToHit);
	CopyInt(tbuff, &is->_iPLAC);
	CopyInt(tbuff, &is->_iPLStr);
	CopyInt(tbuff, &is->_iPLMag);
	CopyInt(tbuff, &is->_iPLDex);
	CopyInt(tbuff, &is->_iPLVit);
	CopyInt(tbuff, &is->_iPLFR);
	CopyInt(tbuff, &is->_iPLLR);
	CopyInt(tbuff, &is->_iPLMR);
	CopyInt(tbuff, &is->_iPLAR);
	CopyInt(tbuff, &is->_iPLMana);
	CopyInt(tbuff, &is->_iPLHP);
	CopyInt(tbuff, &is->_iPLDamMod);
	CopyInt(tbuff, &is->_iPLGetHit);
	CopyChar(tbuff, &is->_iPLLight);
	CopyChar(tbuff, &is->_iDamType);
	CopyChar(tbuff, &is->_iSplLvlAdd);
	CopyChar(tbuff, &is->_iRequest);
	CopyChar(tbuff, &is->_iManaSteal);
	CopyChar(tbuff, &is->_iLifeSteal);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &is->_iUid);
	CopyChar(tbuff, &is->_iFMinDam);
	CopyChar(tbuff, &is->_iFMaxDam);
	CopyChar(tbuff, &is->_iLMinDam);
	CopyChar(tbuff, &is->_iLMaxDam);
	CopyChar(tbuff, &is->_iMMinDam);
	CopyChar(tbuff, &is->_iMMaxDam);
	CopyChar(tbuff, &is->_iAMinDam);
	CopyChar(tbuff, &is->_iAMaxDam);
	CopyInt(tbuff, &is->_iPLEnAc);
	CopyChar(tbuff, &is->_iPrePower);
	CopyChar(tbuff, &is->_iSufPower);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &is->_iVAdd);
	CopyInt(tbuff, &is->_iVMult);
	CopyChar(tbuff, &is->_iMinStr);
	CopyChar(tbuff, &is->_iMinMag);
	CopyChar(tbuff, &is->_iMinDex);
	tbuff += 1; // Alignment
	CopyInt(tbuff, &is->_iStatFlag);
}

static void LoadItems(ItemStruct *pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		LoadItemData(&pItem[i]);
	}
}

static void LoadPlayer(int pnum)
{
	CopyInt(tbuff, &plr._pmode);
	CopyBytes(tbuff, lengthof(plr.walkpath), plr.walkpath);
	CopyInt(tbuff, &plr.destAction);
	CopyInt(tbuff, &plr.destParam1);
	CopyInt(tbuff, &plr.destParam2);
	CopyInt(tbuff, &plr.destParam3);
	CopyInt(tbuff, &plr.destParam4);
	CopyChar(tbuff, &plr.plractive);
	CopyChar(tbuff, &plr._pLvlChanging);
	CopyChar(tbuff, &plr.plrlevel);
	tbuff += 1; // Alignment
	CopyInt(tbuff, &plr._px);
	CopyInt(tbuff, &plr._py);
	CopyInt(tbuff, &plr._pfutx);
	CopyInt(tbuff, &plr._pfuty);
	CopyInt(tbuff, &plr._poldx);
	CopyInt(tbuff, &plr._poldy);
	CopyInt(tbuff, &plr._pxoff);
	CopyInt(tbuff, &plr._pyoff);
	CopyInt(tbuff, &plr._pxvel);
	CopyInt(tbuff, &plr._pyvel);
	CopyInt(tbuff, &plr._pdir);
	CopyInt(tbuff, &plr._pgfxnum);
	tbuff += 4; // Skip pointer _pAnimData
	tbuff += 4; // Skip _pAnimFrameLen
	CopyInt(tbuff, &plr._pAnimCnt);
	CopyInt(tbuff, &plr._pAnimLen);
	CopyInt(tbuff, &plr._pAnimFrame);
	CopyInt(tbuff, &plr._pAnimWidth);
	CopyInt(tbuff, &plr._pAnimXOffset);
	CopyInt(tbuff, &plr._plid);
	CopyInt(tbuff, &plr._pvid);

	CopyChar(tbuff, &plr._pAtkSkill);
	CopyChar(tbuff, &plr._pAtkSkillType);
	CopyChar(tbuff, &plr._pMoveSkill);
	CopyChar(tbuff, &plr._pMoveSkillType);
	CopyChar(tbuff, &plr._pAltAtkSkill);
	CopyChar(tbuff, &plr._pAltAtkSkillType);
	CopyChar(tbuff, &plr._pAltMoveSkill);
	CopyChar(tbuff, &plr._pAltMoveSkillType);

	CopyChar(tbuff, &plr._pTSpell);
	CopyChar(tbuff, &plr._pTSplFrom);
	CopyChar(tbuff, &plr._pOilFrom);
	tbuff += 1; // Alignment

	CopyBytes(tbuff, 64, plr._pSkillLvl);
	CopyBytes(tbuff, 64, plr._pSkillActivity);
	CopyInts(tbuff, 64, plr._pSkillExp);
	CopyInt64(tbuff, &plr._pMemSkills);
	CopyInt64(tbuff, &plr._pAblSkills);
	CopyInt64(tbuff, &plr._pScrlSkills);

	CopyBytes(tbuff, 4, plr._pAtkSkillHotKey);
	CopyBytes(tbuff, 4, plr._pAtkSkillTypeHotKey);
	CopyBytes(tbuff, 4, plr._pMoveSkillHotKey);
	CopyBytes(tbuff, 4, plr._pMoveSkillTypeHotKey);
	CopyBytes(tbuff, 4, plr._pAltAtkSkillHotKey);
	CopyBytes(tbuff, 4, plr._pAltAtkSkillTypeHotKey);
	CopyBytes(tbuff, 4, plr._pAltMoveSkillHotKey);
	CopyBytes(tbuff, 4, plr._pAltMoveSkillTypeHotKey);

	CopyChar(tbuff, &plr._pSkillFlags);
	CopyChar(tbuff, &plr._pInvincible);
	CopyShort(tbuff, &plr._pTimer[PLTR_INFRAVISION]);
	CopyShort(tbuff, &plr._pTimer[PLTR_RAGE]);

	CopyBytes(tbuff, PLR_NAME_LEN, plr._pName);
	CopyChar(tbuff, &plr._pClass);
	CopyChar(tbuff, &plr._pLevel);
	CopyChar(tbuff, &plr._pLightRad);
	CopyChar(tbuff, &plr._pManaShield);
	CopyShort(tbuff, &plr._pBaseStr);
	CopyShort(tbuff, &plr._pBaseMag);
	CopyShort(tbuff, &plr._pBaseDex);
	CopyShort(tbuff, &plr._pBaseVit);
	CopyInt(tbuff, &plr._pStrength);
	CopyInt(tbuff, &plr._pMagic);
	CopyInt(tbuff, &plr._pDexterity);
	CopyInt(tbuff, &plr._pVitality);
	CopyInt(tbuff, &plr._pHPBase);
	CopyInt(tbuff, &plr._pMaxHPBase);
	CopyInt(tbuff, &plr._pHitPoints);
	CopyInt(tbuff, &plr._pMaxHP);
	CopyInt(tbuff, &plr._pHPPer);
	CopyInt(tbuff, &plr._pManaBase);
	CopyInt(tbuff, &plr._pMaxManaBase);
	CopyInt(tbuff, &plr._pMana);
	CopyInt(tbuff, &plr._pMaxMana);
	CopyInt(tbuff, &plr._pManaPer);
	CopyShort(tbuff, &plr._pStatPts);
	CopyChar(tbuff, &plr._pLvlUp);
	CopyChar(tbuff, &plr._pDiabloKillLevel);
	CopyInt(tbuff, &plr._pExperience);
	CopyInt(tbuff, &plr._pNextExper);
	tbuff += 1; // Skip to Calc _pMagResist
	tbuff += 1; // Skip to Calc _pFireResist
	tbuff += 1; // Skip to Calc _pLghtResist
	tbuff += 1; // Skip to Calc _pAcidResist
	CopyInt(tbuff, &plr._pGold);

	CopyInt(tbuff, &plr._pInfraFlag);
	CopyInt(tbuff, &plr._pVar1);
	CopyInt(tbuff, &plr._pVar2);
	CopyInt(tbuff, &plr._pVar3);
	CopyInt(tbuff, &plr._pVar4);
	CopyInt(tbuff, &plr._pVar5);
	CopyInt(tbuff, &plr._pVar6);
	CopyInt(tbuff, &plr._pVar7);
	CopyInt(tbuff, &plr._pVar8);
	CopyBytes(tbuff, NUM_LEVELS, plr._pLvlVisited);
	tbuff += 1; // Alignment

	CopyInt(tbuff, &plr._pGFXLoad);
	tbuff += 4 * 8; // Skip pointers _pNAnim
	CopyInt(tbuff, &plr._pNFrames);
	CopyInt(tbuff, &plr._pNWidth);
	tbuff += 4 * 8; // Skip pointers _pWAnim
	CopyInt(tbuff, &plr._pWFrames);
	CopyInt(tbuff, &plr._pWWidth);
	tbuff += 4 * 8; // Skip pointers _pAAnim
	CopyInt(tbuff, &plr._pAFrames);
	CopyInt(tbuff, &plr._pAWidth);
	CopyInt(tbuff, &plr._pAFNum);
	tbuff += 4 * 8; // Skip pointers _pLAnim
	tbuff += 4 * 8; // Skip pointers _pFAnim
	tbuff += 4 * 8; // Skip pointers _pTAnim
	CopyInt(tbuff, &plr._pSFrames);
	CopyInt(tbuff, &plr._pSWidth);
	CopyInt(tbuff, &plr._pSFNum);
	tbuff += 4 * 8; // Skip pointers _pHAnim
	CopyInt(tbuff, &plr._pHFrames);
	CopyInt(tbuff, &plr._pHWidth);
	tbuff += 4 * 8; // Skip pointers _pDAnim
	CopyInt(tbuff, &plr._pDFrames);
	CopyInt(tbuff, &plr._pDWidth);
	tbuff += 4 * 8; // Skip pointers _pBAnim
	CopyInt(tbuff, &plr._pBFrames);
	CopyInt(tbuff, &plr._pBWidth);

	LoadItems(plr.InvBody, NUM_INVLOC);
	LoadItems(plr.SpdList, MAXBELTITEMS);
	LoadItems(plr.InvList, NUM_INV_GRID_ELEM);
	CopyBytes(tbuff, NUM_INV_GRID_ELEM, plr.InvGrid);
	CopyInt(tbuff, &plr._pNumInv);
	LoadItemData(&plr.HoldItem);

	tbuff += 4; // Skip to Calc _pISlMinDam
	tbuff += 4; // Skip to Calc _pISlMaxDam
	tbuff += 4; // Skip to Calc _pIBlMinDam
	tbuff += 4; // Skip to Calc _pIBlMaxDam
	tbuff += 4; // Skip to Calc _pIPcMinDam
	tbuff += 4; // Skip to Calc _pIPcMaxDam
	//tbuff += 4; // Skip to Calc _pIEvasion
	tbuff += 4; // Skip to Calc _pIAC
	tbuff += 4; // Skip to Calc _pIHitChance
	tbuff += 1; // Skip to Calc _pIBaseHitBonus
	tbuff += 1; // Skip to Calc _pICritChance
	tbuff += 1; // Skip to Calc _pIBlockChance
	tbuff += 1; // Alignment
	tbuff += 4; // Skip to Calc _pIMagToHit

	tbuff += 8; // Skip to Calc _pISpells
	tbuff += 4; // Skip to Calc _pIFlags
	tbuff += 4; // Skip to Calc _pIGetHit
	tbuff += 1; // Skip to Calc _pISplLvlAdd
	tbuff += 1; // Skip to Calc _pIArrowVelBonus
	tbuff += 1; // Skip to Calc _pILifeSteal
	tbuff += 1; // Skip to Calc _pIManaSteal
	tbuff += 4; // Skip to Calc _pIEnAc
	tbuff += 4; // Skip to Calc _pIFMinDam
	tbuff += 4; // Skip to Calc _pIFMaxDam
	tbuff += 4; // Skip to Calc _pILMinDam
	tbuff += 4; // Skip to Calc _pILMaxDam
	tbuff += 4; // Skip to Calc _pIMMinDam
	tbuff += 4; // Skip to Calc _pIMMaxDam
	tbuff += 4; // Skip to Calc _pIAMinDam
	tbuff += 4; // Skip to Calc _pIAMaxDam
	CopyChar(tbuff, &plr.pTownWarps);
	CopyChar(tbuff, &plr.palign_CB);
	CopyChar(tbuff, &plr.pDungMsgs);
	CopyChar(tbuff, &plr.pDungMsgs2);

	CalcPlrInv(pnum, false);

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

	SyncPlrAnim(pnum);
}

static void LoadMonster(int mnum)
{
	MonsterStruct *mon = &monster[mnum];

	CopyInt(tbuff, &mon->_mmode);
	CopyInt(tbuff, &mon->_msquelch);
	CopyChar(tbuff, &mon->_mMTidx);
	CopyChar(tbuff, &mon->_mpathcount);
	CopyChar(tbuff, &mon->_mWhoHit);
	CopyChar(tbuff, &mon->_mgoal);
	CopyInt(tbuff, &mon->_mgoalvar1);
	CopyInt(tbuff, &mon->_mgoalvar2);
	CopyInt(tbuff, &mon->_mgoalvar3);
	CopyInt(tbuff, &mon->_mx);
	CopyInt(tbuff, &mon->_my);
	CopyInt(tbuff, &mon->_mfutx);
	CopyInt(tbuff, &mon->_mfuty);
	CopyInt(tbuff, &mon->_moldx);
	CopyInt(tbuff, &mon->_moldy);
	CopyInt(tbuff, &mon->_mxoff);
	CopyInt(tbuff, &mon->_myoff);
	CopyInt(tbuff, &mon->_mxvel);
	CopyInt(tbuff, &mon->_myvel);
	CopyInt(tbuff, &mon->_mdir);
	CopyInt(tbuff, &mon->_menemy);
	CopyChar(tbuff, &mon->_menemyx);
	CopyChar(tbuff, &mon->_menemyy);
	CopyChar(tbuff, &mon->_mListener);
	tbuff += 1; // Alignment

	tbuff += 4; // Skip pointer _mAnimData
	tbuff += 4; // Skip _mAnimFrameLen
	CopyInt(tbuff, &mon->_mAnimCnt);
	CopyInt(tbuff, &mon->_mAnimLen);
	CopyInt(tbuff, &mon->_mAnimFrame);
	CopyInt(tbuff, &mon->_mDelFlag);
	CopyInt(tbuff, &mon->_mVar1);
	CopyInt(tbuff, &mon->_mVar2);
	CopyInt(tbuff, &mon->_mVar3);
	CopyInt(tbuff, &mon->_mVar4);
	CopyInt(tbuff, &mon->_mVar5);
	CopyInt(tbuff, &mon->_mVar6);
	CopyInt(tbuff, &mon->_mVar7);
	CopyInt(tbuff, &mon->_mVar8);
	CopyInt(tbuff, &mon->_mmaxhp);
	CopyInt(tbuff, &mon->_mhitpoints);
	CopyInt(tbuff, &mon->_mFlags);
	CopyInt(tbuff, &mon->_lastx);
	CopyInt(tbuff, &mon->_lasty);
	CopyInt(tbuff, &mon->_mRndSeed);
	CopyInt(tbuff, &mon->_mAISeed);

	CopyChar(tbuff, &mon->_uniqtype);
	CopyChar(tbuff, &mon->_uniqtrans);
	CopyChar(tbuff, &mon->_udeadval);
	CopyChar(tbuff, &mon->mlid);

	CopyChar(tbuff, &mon->leader);
	CopyChar(tbuff, &mon->leaderflag);
	CopyChar(tbuff, &mon->packsize);
	CopyChar(tbuff, &mon->falign_CB); // Alignment

	CopyChar(tbuff, &mon->mLevel);
	CopyChar(tbuff, &mon->_mSelFlag);
	CopyChar(tbuff, &mon->_mAi);
	CopyChar(tbuff, &mon->_mint);

	CopyShort(tbuff, &mon->_mHit);
	CopyChar(tbuff, &mon->_mMagic);
	CopyChar(tbuff, &mon->_mAFNum);
	CopyChar(tbuff, &mon->_mMinDamage);
	CopyChar(tbuff, &mon->_mMaxDamage);

	CopyShort(tbuff, &mon->_mHit2);
	CopyChar(tbuff, &mon->_mMagic2);
	CopyChar(tbuff, &mon->_mAFNum2);
	CopyChar(tbuff, &mon->_mMinDamage2);
	CopyChar(tbuff, &mon->_mMaxDamage2);

	CopyChar(tbuff, &mon->_mArmorClass);
	CopyChar(tbuff, &mon->_mEvasion);
	tbuff += 2; // Alignment

	CopyShort(tbuff, &mon->mMagicRes);
	CopyShort(tbuff, &mon->mExp);

	CopyInt(tbuff, &mon->mtalkmsg);

	// Omit pointer mName;
	// Omit pointer MType;
	// Omit pointer MData;

	SyncMonsterAnim(mnum);
}

static void LoadMissile(int mi)
{
	MissileStruct *mis = &missile[mi];

	CopyInt(tbuff, &mis->_miType);
	CopyChar(tbuff, &mis->_miSubType);
	CopyChar(tbuff, &mis->_miFlags);
	CopyChar(tbuff, &mis->_miResist);
	CopyChar(tbuff, &mis->_miAnimType);
	CopyInt(tbuff, &mis->_miAnimFlag);
	tbuff += 4; // Skip pointer _miAnimData
	tbuff += 4; // Skip _miAnimFrameLen
	CopyInt(tbuff, &mis->_miAnimLen);
	CopyInt(tbuff, &mis->_miAnimWidth);
	CopyInt(tbuff, &mis->_miAnimXOffset);
	CopyInt(tbuff, &mis->_miAnimCnt);
	CopyInt(tbuff, &mis->_miAnimAdd);
	CopyInt(tbuff, &mis->_miAnimFrame);
	CopyInt(tbuff, &mis->_miDelFlag);
	CopyInt(tbuff, &mis->_miDrawFlag);
	CopyInt(tbuff, &mis->_miLightFlag);
	CopyInt(tbuff, &mis->_miPreFlag);
	CopyInt(tbuff, &mis->_miUniqTrans);
	CopyInt(tbuff, &mis->_misx);
	CopyInt(tbuff, &mis->_misy);
	CopyInt(tbuff, &mis->_mix);
	CopyInt(tbuff, &mis->_miy);
	CopyInt(tbuff, &mis->_mixoff);
	CopyInt(tbuff, &mis->_miyoff);
	CopyInt(tbuff, &mis->_mixvel);
	CopyInt(tbuff, &mis->_miyvel);
	CopyInt(tbuff, &mis->_mitxoff);
	CopyInt(tbuff, &mis->_mityoff);
	CopyInt(tbuff, &mis->_miDir);
	CopyInt(tbuff, &mis->_miSpllvl);
	CopyInt(tbuff, &mis->_miRange);
	CopyInt(tbuff, &mis->_miSource);
	CopyInt(tbuff, &mis->_miCaster);
	CopyInt(tbuff, &mis->_miMinDam);
	CopyInt(tbuff, &mis->_miMaxDam);
	CopyInt(tbuff, &mis->_miDist);
	CopyInt(tbuff, &mis->_miLid);
	CopyInt(tbuff, &mis->_miVar1);
	CopyInt(tbuff, &mis->_miVar2);
	CopyInt(tbuff, &mis->_miVar3);
	CopyInt(tbuff, &mis->_miVar4);
	CopyInt(tbuff, &mis->_miVar5);
	CopyInt(tbuff, &mis->_miVar6);
	CopyInt(tbuff, &mis->_miVar7);
	CopyInt(tbuff, &mis->_miVar8);
}

static void LoadObject(int oi, bool full)
{
	ObjectStruct *os = &object[oi];

	CopyInt(tbuff, &os->_otype);
	CopyInt(tbuff, &os->_ox);
	CopyInt(tbuff, &os->_oy);
	CopyInt(tbuff, &os->_oAnimFlag);
	tbuff += 4; // Skip pointer _oAnimData
	tbuff += 4; // Skip _oAnimFrameLen
	CopyInt(tbuff, &os->_oAnimCnt);
	CopyInt(tbuff, &os->_oAnimLen);
	CopyInt(tbuff, &os->_oAnimFrame);
	CopyInt(tbuff, &os->_oAnimWidth);
	CopyInt(tbuff, &os->_oAnimXOffset);
	CopyInt(tbuff, &os->_oSolidFlag);
	CopyInt(tbuff, &os->_oMissFlag);
	CopyInt(tbuff, &os->_oLight);
	CopyChar(tbuff, &os->_oBreak);
	CopyChar(tbuff, &os->_oSelFlag);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &os->_oPreFlag);
	CopyInt(tbuff, &os->_oTrapFlag);
	CopyInt(tbuff, &os->_oDoorFlag);
	if (full) {
		CopyInt(tbuff, &os->_olid);
	} else {
		// reset dynamic lights
		os->_olid = -1;
		tbuff += 4;
	}
	CopyInt(tbuff, &os->_oRndSeed);
	CopyInt(tbuff, &os->_oVar1);
	CopyInt(tbuff, &os->_oVar2);
	CopyInt(tbuff, &os->_oVar3);
	CopyInt(tbuff, &os->_oVar4);
	CopyInt(tbuff, &os->_oVar5);
	CopyInt(tbuff, &os->_oVar6);
	CopyInt(tbuff, &os->_oVar7);
	CopyInt(tbuff, &os->_oVar8);

	SyncObjectAnim(oi);
}

static void LoadItem(int ii)
{
	LoadItemData(&items[ii]);
	SyncItemAnim(ii);
}

static void LoadQuest(int i)
{
	QuestStruct *pQuest = &quests[i];

	CopyChar(tbuff, &pQuest->_qidx);
	CopyChar(tbuff, &pQuest->_qactive);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &pQuest->_qtx);
	CopyInt(tbuff, &pQuest->_qty);
	CopyInt(tbuff, &pQuest->_qmsg);
	CopyChar(tbuff, &pQuest->_qvar1);
	CopyChar(tbuff, &pQuest->_qvar2);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &pQuest->_qlog);
}

static void LoadLight(LightListStruct *pLight)
{
	CopyInt(tbuff, &pLight->_lx);
	CopyInt(tbuff, &pLight->_ly);
	CopyInt(tbuff, &pLight->_lunx);
	CopyInt(tbuff, &pLight->_luny);
	CopyChar(tbuff, &pLight->_lradius);
	CopyChar(tbuff, &pLight->_lunr);
	pLight->_ldel = LoadBool();
	pLight->_lunflag = LoadBool();
	pLight->_lmine = LoadBool();
	tbuff += 3; // Alignment
	CopyInt(tbuff, &pLight->_xoff);
	CopyInt(tbuff, &pLight->_yoff);
}

static void LoadPortal(int i)
{
	PortalStruct *pPortal = &portals[i];

	pPortal->_wopen = LoadBool();
	CopyInt(tbuff, &pPortal->x);
	CopyInt(tbuff, &pPortal->y);
	CopyInt(tbuff, &pPortal->level);
}

/*static void RedoPlayerLight()
{
	for (int pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (plr.plractive && currLvl._dLevelIdx == plr.plrlevel)
			ChangeLightXY(plr._plid, plr._px, plr._py);
	}
}*/

/**
 * @brief Load game state
 * @param firstflag Can be set to false if we are simply reloading the current game
 */
void LoadGame(bool firstflag)
{
	int i;
	BYTE* fileBuff;
	int _ViewX, _ViewY;

	FreeLevelMem();

	pfile_remove_temp_files();
	fileBuff = pfile_read(SAVEFILE_GAME);
	tbuff = fileBuff;

	if (LoadInt() != SAVE_INITIAL)
		app_fatal("Invalid save file");
	// load game-info
	i = LoadInt();
	currLvl._dLevelIdx = i & 0xFF;
	EnterLevel(i & 0xFF);
	gnDifficulty = (i >> 8) & 0xFF;
	for (i = 0; i < NUM_LEVELS; i++) {
		glSeedTbl[i] = LoadInt();
	}
	// load player-data
	_ViewX = LoadInt();
	_ViewY = LoadInt();
	gbInvflag = LoadBool();
	gbChrflag = LoadBool();
	gbAutomapflag = LoadBool();
	AutoMapScale = LoadInt();

	LoadPlayer(mypnum);

	// load meta-data I.
	ReturnLvlX = LoadInt();
	ReturnLvlY = LoadInt();
	ReturnLvl = LoadInt();
	// load meta-data II. (used by LoadGameLevel)
	for (i = 0; i < NUM_QUESTS; i++)
		LoadQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		LoadPortal(i);
	// load level-data
	LoadGameLevel(firstflag, ENTRY_LOAD);
	ViewX = _ViewX;
	ViewY = _ViewY;

	if (currLvl._dType != DTYPE_TOWN) {
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dDead);
	}

	nummonsters = LoadInt();
	nummissiles = LoadInt();
	nobjects = LoadInt();
	numitems = LoadInt();

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			monstactive[i] = LoadInt();
		for (i = 0; i < nummonsters; i++)
			LoadMonster(monstactive[i]);
		static_assert(MAXMISSILES <= CHAR_MAX, "LoadGame handles missile-ids as chars.");
		for (i = 0; i < MAXMISSILES; i++)
			missileactive[i] = LoadChar();
		for (i = 0; i < MAXMISSILES; i++)
			missileavail[i] = LoadChar();
		for (i = 0; i < nummissiles; i++)
			LoadMissile(missileactive[i]);
		static_assert(MAXOBJECTS <= CHAR_MAX, "LoadGame handles object-ids as chars.");
		for (i = 0; i < MAXOBJECTS; i++)
			objectactive[i] = LoadChar();
		for (i = 0; i < MAXOBJECTS; i++)
			objectavail[i] = LoadChar();
		for (i = 0; i < nobjects; i++)
			LoadObject(objectactive[i], true);
	}
	static_assert(MAXITEMS <= CHAR_MAX, "LoadGame handles item-ids as chars.");
	for (i = 0; i < MAXITEMS; i++)
		itemactive[i] = LoadChar();
	for (i = 0; i < MAXITEMS; i++)
		itemavail[i] = LoadChar();
	for (i = 0; i < numitems; i++)
		LoadItem(itemactive[i]);

	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dFlags);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dItem);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dLight);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPreLight);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPlayer);

	if (currLvl._dType != DTYPE_TOWN) {
		CopyInts(tbuff, MAXDUNX * MAXDUNY, dMonster);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dObject);
		CopyBytes(tbuff, DMAXX * DMAXY, automapview);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dMissile);
	}
	// load meta-data III. (modified by LoadGameLevel)
	numpremium = LoadInt();
	premiumlevel = LoadInt();
	numlights = LoadInt();
	numvision = LoadInt();

	for (i = 0; i < MAXLIGHTS; i++)
		lightactive[i] = LoadChar();
	for (i = 0; i < numlights; i++)
		LoadLight(&LightList[lightactive[i]]);

	for (i = 0; i < MAXVISION; i++)
		visionactive[i] = LoadChar();
	for (i = 0; i < numvision; i++)
		LoadLight(&VisionList[visionactive[i]]);

	static_assert(NUM_UITEM <= 128, "Save files are no longer compatible.");
	for (i = 0; i < NUM_UITEM; i++)
		UniqueItemFlags[i] = LoadBool();
	for ( ; i < 128; i++)
		LoadBool();

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		LoadItemData(&premiumitems[i]);

	mem_free_dbg(fileBuff);

	AutomapZoomReset();
	//ResyncQuests();

	//RedoPlayerLight();
	//ProcessLightList();
	//RedoPlayerVision();
	//ProcessVisionList();

	SyncMissilesAnim();

	NewCursor(CURSOR_HAND);
	gbProcessPlayers = true;
}


static void SaveChar(char v)
{
	*tbuff++ = v;
}

static void SaveInt(unsigned v)
{
	*tbuff++ = v >> 24;
	*tbuff++ = v >> 16;
	*tbuff++ = v >> 8;
	*tbuff++ = v;
}

static void SaveBool(BOOL v)
{
	if (v)
		*tbuff++ = TRUE;
	else
		*tbuff++ = FALSE;
}

static void SaveItemData(ItemStruct *is)
{
	CopyInt(&is->_iSeed, tbuff);
	CopyShort(&is->_iCreateInfo, tbuff);
	CopyShort(&is->_iIdx, tbuff);
	CopyInt(&is->_itype, tbuff);
	CopyInt(&is->_ix, tbuff);
	CopyInt(&is->_iy, tbuff);
	CopyInt(&is->_iAnimFlag, tbuff);
	tbuff += 4; // Skip pointer _iAnimData
	tbuff += 4; // Skip _iAnimFrameLen
	CopyInt(&is->_iAnimCnt, tbuff);
	CopyInt(&is->_iAnimLen, tbuff);
	CopyInt(&is->_iAnimFrame, tbuff);
	tbuff += 4; // CopyInt(&is->_iAnimWidth, tbuff);
	tbuff += 4; // CopyInt(&is->_iAnimXOffset, tbuff);
	CopyInt(&is->_iPostDraw, tbuff);
	CopyInt(&is->_iIdentified, tbuff);
	CopyBytes(is->_iName, sizeof(is->_iName), tbuff);
	CopyChar(&is->_iSelFlag, tbuff);
	CopyChar(&is->_iMagical, tbuff);
	CopyChar(&is->_iLoc, tbuff);
	CopyChar(&is->_iClass, tbuff);
	CopyInt(&is->_iCurs, tbuff);
	CopyInt(&is->_ivalue, tbuff);
	CopyInt(&is->_iIvalue, tbuff);
	CopyChar(&is->_iMinDam, tbuff);
	CopyChar(&is->_iMaxDam, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&is->_iAC, tbuff);
	CopyInt(&is->_iFlags, tbuff);
	CopyInt(&is->_iMiscId, tbuff);
	CopyInt(&is->_iSpell, tbuff);
	CopyInt(&is->_iCharges, tbuff);
	CopyInt(&is->_iMaxCharges, tbuff);
	CopyInt(&is->_iDurability, tbuff);
	CopyInt(&is->_iMaxDur, tbuff);
	CopyInt(&is->_iPLDam, tbuff);
	CopyInt(&is->_iPLToHit, tbuff);
	CopyInt(&is->_iPLAC, tbuff);
	CopyInt(&is->_iPLStr, tbuff);
	CopyInt(&is->_iPLMag, tbuff);
	CopyInt(&is->_iPLDex, tbuff);
	CopyInt(&is->_iPLVit, tbuff);
	CopyInt(&is->_iPLFR, tbuff);
	CopyInt(&is->_iPLLR, tbuff);
	CopyInt(&is->_iPLMR, tbuff);
	CopyInt(&is->_iPLAR, tbuff);
	CopyInt(&is->_iPLMana, tbuff);
	CopyInt(&is->_iPLHP, tbuff);
	CopyInt(&is->_iPLDamMod, tbuff);
	CopyInt(&is->_iPLGetHit, tbuff);
	CopyChar(&is->_iPLLight, tbuff);
	CopyChar(&is->_iDamType, tbuff);
	CopyChar(&is->_iSplLvlAdd, tbuff);
	CopyChar(&is->_iRequest, tbuff);
	CopyChar(&is->_iManaSteal, tbuff);
	CopyChar(&is->_iLifeSteal, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&is->_iUid, tbuff);
	CopyChar(&is->_iFMinDam, tbuff);
	CopyChar(&is->_iFMaxDam, tbuff);
	CopyChar(&is->_iLMinDam, tbuff);
	CopyChar(&is->_iLMaxDam, tbuff);
	CopyChar(&is->_iMMinDam, tbuff);
	CopyChar(&is->_iMMaxDam, tbuff);
	CopyChar(&is->_iAMinDam, tbuff);
	CopyChar(&is->_iAMaxDam, tbuff);
	CopyInt(&is->_iPLEnAc, tbuff);
	CopyChar(&is->_iPrePower, tbuff);
	CopyChar(&is->_iSufPower, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&is->_iVAdd, tbuff);
	CopyInt(&is->_iVMult, tbuff);
	CopyChar(&is->_iMinStr, tbuff);
	CopyChar(&is->_iMinMag, tbuff);
	CopyChar(&is->_iMinDex, tbuff);
	tbuff += 1; // Alignment
	CopyInt(&is->_iStatFlag, tbuff);
}

static void SaveItems(ItemStruct *pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		SaveItemData(&pItem[i]);
	}
}

static void SavePlayer(int pnum)
{
	CopyInt(&plr._pmode, tbuff);
	static_assert(sizeof(plr.walkpath) == MAX_PATH_LENGTH + 1, "Save files are no longer compatible.");
	CopyBytes(plr.walkpath, lengthof(plr.walkpath), tbuff);
	CopyInt(&plr.destAction, tbuff);
	CopyInt(&plr.destParam1, tbuff);
	CopyInt(&plr.destParam2, tbuff);
	CopyInt(&plr.destParam3, tbuff);
	CopyInt(&plr.destParam4, tbuff);
	CopyChar(&plr.plractive, tbuff);
	CopyChar(&plr._pLvlChanging, tbuff);
	CopyChar(&plr.plrlevel, tbuff);
	tbuff += 1; // Alignment
	CopyInt(&plr._px, tbuff);
	CopyInt(&plr._py, tbuff);
	CopyInt(&plr._pfutx, tbuff);
	CopyInt(&plr._pfuty, tbuff);
	CopyInt(&plr._poldx, tbuff);
	CopyInt(&plr._poldy, tbuff);
	CopyInt(&plr._pxoff, tbuff);
	CopyInt(&plr._pyoff, tbuff);
	CopyInt(&plr._pxvel, tbuff);
	CopyInt(&plr._pyvel, tbuff);
	CopyInt(&plr._pdir, tbuff);
	CopyInt(&plr._pgfxnum, tbuff);
	tbuff += 4; // Skip pointer _pAnimData
	tbuff += 4; // Skip _pAnimFrameLen
	CopyInt(&plr._pAnimCnt, tbuff);
	CopyInt(&plr._pAnimLen, tbuff);
	CopyInt(&plr._pAnimFrame, tbuff);
	CopyInt(&plr._pAnimWidth, tbuff);
	CopyInt(&plr._pAnimXOffset, tbuff);
	CopyInt(&plr._plid, tbuff);
	CopyInt(&plr._pvid, tbuff);

	CopyChar(&plr._pAtkSkill, tbuff);
	CopyChar(&plr._pAtkSkillType, tbuff);
	CopyChar(&plr._pMoveSkill, tbuff);
	CopyChar(&plr._pMoveSkillType, tbuff);

	CopyChar(&plr._pAltAtkSkill, tbuff);
	CopyChar(&plr._pAltAtkSkillType, tbuff);
	CopyChar(&plr._pAltMoveSkill, tbuff);
	CopyChar(&plr._pAltMoveSkillType, tbuff);

	CopyChar(&plr._pTSpell, tbuff);
	CopyChar(&plr._pTSplFrom, tbuff);
	CopyChar(&plr._pOilFrom, tbuff);
	tbuff += 1; // Alignment

	CopyBytes(plr._pSkillLvl, 64, tbuff);
	CopyBytes(plr._pSkillActivity, 64, tbuff);
	CopyInts(plr._pSkillExp, 64, tbuff);
	CopyInt64(&plr._pMemSkills, tbuff);
	CopyInt64(&plr._pAblSkills, tbuff);
	CopyInt64(&plr._pScrlSkills, tbuff);

	CopyBytes(plr._pAtkSkillHotKey, 4, tbuff);
	CopyBytes(plr._pAtkSkillTypeHotKey, 4, tbuff);
	CopyBytes(plr._pMoveSkillHotKey, 4, tbuff);
	CopyBytes(plr._pMoveSkillTypeHotKey, 4, tbuff);
	CopyBytes(plr._pAltAtkSkillHotKey, 4, tbuff);
	CopyBytes(plr._pAltAtkSkillTypeHotKey, 4, tbuff);
	CopyBytes(plr._pAltMoveSkillHotKey, 4, tbuff);
	CopyBytes(plr._pAltMoveSkillTypeHotKey, 4, tbuff);

	CopyChar(&plr._pSkillFlags, tbuff);
	CopyChar(&plr._pInvincible, tbuff);
	CopyShort(&plr._pTimer[PLTR_INFRAVISION], tbuff);
	CopyShort(&plr._pTimer[PLTR_RAGE], tbuff);

	CopyBytes(plr._pName, PLR_NAME_LEN, tbuff);
	CopyChar(&plr._pClass, tbuff);
	CopyChar(&plr._pLevel, tbuff);
	CopyChar(&plr._pLightRad, tbuff);
	CopyChar(&plr._pManaShield, tbuff);
	CopyShort(&plr._pBaseStr, tbuff);
	CopyShort(&plr._pBaseMag, tbuff);
	CopyShort(&plr._pBaseDex, tbuff);
	CopyShort(&plr._pBaseVit, tbuff);
	CopyInt(&plr._pStrength, tbuff);
	CopyInt(&plr._pMagic, tbuff);
	CopyInt(&plr._pDexterity, tbuff);
	CopyInt(&plr._pVitality, tbuff);
	CopyInt(&plr._pHPBase, tbuff);
	CopyInt(&plr._pMaxHPBase, tbuff);
	CopyInt(&plr._pHitPoints, tbuff);
	CopyInt(&plr._pMaxHP, tbuff);
	CopyInt(&plr._pHPPer, tbuff);
	CopyInt(&plr._pManaBase, tbuff);
	CopyInt(&plr._pMaxManaBase, tbuff);
	CopyInt(&plr._pMana, tbuff);
	CopyInt(&plr._pMaxMana, tbuff);
	CopyInt(&plr._pManaPer, tbuff);
	CopyShort(&plr._pStatPts, tbuff);
	CopyChar(&plr._pLvlUp, tbuff);
	CopyChar(&plr._pDiabloKillLevel, tbuff);
	CopyInt(&plr._pExperience, tbuff);
	CopyInt(&plr._pNextExper, tbuff);
	tbuff += 1; // Skip to Calc _pMagResist
	tbuff += 1; // Skip to Calc _pFireResist
	tbuff += 1; // Skip to Calc _pLghtResist
	tbuff += 1; // Skip to Calc _pAcidResist
	CopyInt(&plr._pGold, tbuff);

	CopyInt(&plr._pInfraFlag, tbuff);
	CopyInt(&plr._pVar1, tbuff);
	CopyInt(&plr._pVar2, tbuff);
	CopyInt(&plr._pVar3, tbuff);
	CopyInt(&plr._pVar4, tbuff);
	CopyInt(&plr._pVar5, tbuff);
	CopyInt(&plr._pVar6, tbuff);
	CopyInt(&plr._pVar7, tbuff);
	CopyInt(&plr._pVar8, tbuff);
	CopyBytes(plr._pLvlVisited, NUM_LEVELS, tbuff);
	tbuff += 1;                                     // Alignment

	CopyInt(&plr._pGFXLoad, tbuff);
	tbuff += 4 * 8; // Skip pointers _pNAnim
	CopyInt(&plr._pNFrames, tbuff);
	CopyInt(&plr._pNWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pWAnim
	CopyInt(&plr._pWFrames, tbuff);
	CopyInt(&plr._pWWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pAAnim
	CopyInt(&plr._pAFrames, tbuff);
	CopyInt(&plr._pAWidth, tbuff);
	CopyInt(&plr._pAFNum, tbuff);
	tbuff += 4 * 8; // Skip pointers _pLAnim
	tbuff += 4 * 8; // Skip pointers _pFAnim
	tbuff += 4 * 8; // Skip pointers _pTAnim
	CopyInt(&plr._pSFrames, tbuff);
	CopyInt(&plr._pSWidth, tbuff);
	CopyInt(&plr._pSFNum, tbuff);
	tbuff += 4 * 8; // Skip pointers _pHAnim
	CopyInt(&plr._pHFrames, tbuff);
	CopyInt(&plr._pHWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pDAnim
	CopyInt(&plr._pDFrames, tbuff);
	CopyInt(&plr._pDWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pBAnim
	CopyInt(&plr._pBFrames, tbuff);
	CopyInt(&plr._pBWidth, tbuff);

	SaveItems(plr.InvBody, NUM_INVLOC);
	SaveItems(plr.SpdList, MAXBELTITEMS);
	SaveItems(plr.InvList, NUM_INV_GRID_ELEM);
	CopyBytes(plr.InvGrid, NUM_INV_GRID_ELEM, tbuff);
	CopyInt(&plr._pNumInv, tbuff);
	SaveItemData(&plr.HoldItem);

	tbuff += 4; // Skip to Calc _pISlMinDam
	tbuff += 4; // Skip to Calc _pISlMaxDam
	tbuff += 4; // Skip to Calc _pIBlMinDam
	tbuff += 4; // Skip to Calc _pIBlMaxDam
	tbuff += 4; // Skip to Calc _pIPcMinDam
	tbuff += 4; // Skip to Calc _pIPcMaxDam
	//tbuff += 4; // Skip to Calc _pIEvasion
	tbuff += 4; // Skip to Calc _pIAC
	tbuff += 4; // Skip to Calc _pIHitChance
	tbuff += 1; // Skip to Calc _pIBaseHitBonus
	tbuff += 1; // Skip to Calc _pICritChance
	tbuff += 1; // Skip to Calc _pIBlockChance
	tbuff += 1; // Alignment
	tbuff += 4; // Skip to Calc _pIMagToHit

	tbuff += 8; // Skip to Calc _pISpells
	tbuff += 4; // Skip to Calc _pIFlags
	tbuff += 4; // Skip to Calc _pIGetHit

	tbuff += 1; // Skip to Calc _pISplLvlAdd
	tbuff += 1; // Skip to Calc _pIArrowVelBonus (_pISplCost in vanilla)
	tbuff += 1; // Skip to Calc _pILifeSteal
	tbuff += 1; // Skip to Calc _pIManaSteal
	tbuff += 4; // Skip to Calc _pIEnAc
	tbuff += 4; // Skip to Calc _pIFMinDam
	tbuff += 4; // Skip to Calc _pIFMaxDam
	tbuff += 4; // Skip to Calc _pILMinDam
	tbuff += 4; // Skip to Calc _pILMaxDam
	tbuff += 4; // Skip to Calc _pIMMinDam
	tbuff += 4; // Skip to Calc _pIMMaxDam
	tbuff += 4; // Skip to Calc _pIAMinDam
	tbuff += 4; // Skip to Calc _pIAMaxDam
	CopyChar(&plr.pTownWarps, tbuff);
	CopyChar(&plr.palign_CB, tbuff);
	CopyChar(&plr.pDungMsgs, tbuff);
	CopyChar(&plr.pDungMsgs2, tbuff);

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
	MonsterStruct *mon = &monster[mnum];

	CopyInt(&mon->_mmode, tbuff);
	CopyInt(&mon->_msquelch, tbuff);
	CopyChar(&mon->_mMTidx, tbuff);
	CopyChar(&mon->_mpathcount, tbuff);
	CopyChar(&mon->_mWhoHit, tbuff);
	CopyChar(&mon->_mgoal, tbuff);
	CopyInt(&mon->_mgoalvar1, tbuff);
	CopyInt(&mon->_mgoalvar2, tbuff);
	CopyInt(&mon->_mgoalvar3, tbuff);
	CopyInt(&mon->_mx, tbuff);
	CopyInt(&mon->_my, tbuff);
	CopyInt(&mon->_mfutx, tbuff);
	CopyInt(&mon->_mfuty, tbuff);
	CopyInt(&mon->_moldx, tbuff);
	CopyInt(&mon->_moldy, tbuff);
	CopyInt(&mon->_mxoff, tbuff);
	CopyInt(&mon->_myoff, tbuff);
	CopyInt(&mon->_mxvel, tbuff);
	CopyInt(&mon->_myvel, tbuff);
	CopyInt(&mon->_mdir, tbuff);
	CopyInt(&mon->_menemy, tbuff);
	CopyChar(&mon->_menemyx, tbuff);
	CopyChar(&mon->_menemyy, tbuff);
	CopyChar(&mon->_mListener, tbuff);
	tbuff += 1; // Alignment

	tbuff += 4; // Skip pointer _mAnimData
	tbuff += 4; // Skip _mAnimFrameLen
	CopyInt(&mon->_mAnimCnt, tbuff);
	CopyInt(&mon->_mAnimLen, tbuff);
	CopyInt(&mon->_mAnimFrame, tbuff);
	CopyInt(&mon->_mDelFlag, tbuff);
	CopyInt(&mon->_mVar1, tbuff);
	CopyInt(&mon->_mVar2, tbuff);
	CopyInt(&mon->_mVar3, tbuff);
	CopyInt(&mon->_mVar4, tbuff);
	CopyInt(&mon->_mVar5, tbuff);
	CopyInt(&mon->_mVar6, tbuff);
	CopyInt(&mon->_mVar7, tbuff);
	CopyInt(&mon->_mVar8, tbuff);
	CopyInt(&mon->_mmaxhp, tbuff);
	CopyInt(&mon->_mhitpoints, tbuff);
	CopyInt(&mon->_mFlags, tbuff);
	CopyInt(&mon->_lastx, tbuff);
	CopyInt(&mon->_lasty, tbuff);
	CopyInt(&mon->_mRndSeed, tbuff);
	CopyInt(&mon->_mAISeed, tbuff);

	CopyChar(&mon->_uniqtype, tbuff);
	CopyChar(&mon->_uniqtrans, tbuff);
	CopyChar(&mon->_udeadval, tbuff);
	CopyChar(&mon->mlid, tbuff);

	CopyChar(&mon->leader, tbuff);
	CopyChar(&mon->leaderflag, tbuff);
	CopyChar(&mon->packsize, tbuff);
	CopyChar(&mon->falign_CB, tbuff); // Alignment

	CopyChar(&mon->mLevel, tbuff);
	CopyChar(&mon->_mSelFlag, tbuff);
	CopyChar(&mon->_mAi, tbuff);
	CopyChar(&mon->_mint, tbuff);

	CopyShort(&mon->_mHit, tbuff);
	CopyChar(&mon->_mMagic, tbuff);
	CopyChar(&mon->_mAFNum, tbuff);
	CopyChar(&mon->_mMinDamage, tbuff);
	CopyChar(&mon->_mMaxDamage, tbuff);

	CopyShort(&mon->_mHit2, tbuff);
	CopyChar(&mon->_mMagic2, tbuff);
	CopyChar(&mon->_mAFNum2, tbuff);
	CopyChar(&mon->_mMinDamage2, tbuff);
	CopyChar(&mon->_mMaxDamage2, tbuff);

	CopyChar(&mon->_mArmorClass, tbuff);
	CopyChar(&mon->_mEvasion, tbuff);
	tbuff += 2; // Alignment

	CopyShort(&mon->mMagicRes, tbuff);
	CopyShort(&mon->mExp, tbuff);

	CopyInt(&mon->mtalkmsg, tbuff);

	// Omit pointer mName;
	// Omit pointer MType;
	// Omit pointer MData;
}

static void SaveMissile(int mi)
{
	MissileStruct *mis = &missile[mi];

	CopyInt(&mis->_miType, tbuff);
	CopyChar(&mis->_miSubType, tbuff);
	CopyChar(&mis->_miFlags, tbuff);
	CopyChar(&mis->_miResist, tbuff);
	CopyChar(&mis->_miAnimType, tbuff);
	CopyInt(&mis->_miAnimFlag, tbuff);
	tbuff += 4; // Skip pointer _miAnimData
	tbuff += 4; // Skip _miAnimFrameLen
	CopyInt(&mis->_miAnimLen, tbuff);
	CopyInt(&mis->_miAnimWidth, tbuff);
	CopyInt(&mis->_miAnimXOffset, tbuff);
	CopyInt(&mis->_miAnimCnt, tbuff);
	CopyInt(&mis->_miAnimAdd, tbuff);
	CopyInt(&mis->_miAnimFrame, tbuff);
	CopyInt(&mis->_miDelFlag, tbuff);
	CopyInt(&mis->_miDrawFlag, tbuff);
	CopyInt(&mis->_miLightFlag, tbuff);
	CopyInt(&mis->_miPreFlag, tbuff);
	CopyInt(&mis->_miUniqTrans, tbuff);
	CopyInt(&mis->_misx, tbuff);
	CopyInt(&mis->_misy, tbuff);
	CopyInt(&mis->_mix, tbuff);
	CopyInt(&mis->_miy, tbuff);
	CopyInt(&mis->_mixoff, tbuff);
	CopyInt(&mis->_miyoff, tbuff);
	CopyInt(&mis->_mixvel, tbuff);
	CopyInt(&mis->_miyvel, tbuff);
	CopyInt(&mis->_mitxoff, tbuff);
	CopyInt(&mis->_mityoff, tbuff);
	CopyInt(&mis->_miDir, tbuff);
	CopyInt(&mis->_miSpllvl, tbuff);
	CopyInt(&mis->_miRange, tbuff);
	CopyInt(&mis->_miSource, tbuff);
	CopyInt(&mis->_miCaster, tbuff);
	CopyInt(&mis->_miMinDam, tbuff);
	CopyInt(&mis->_miMaxDam, tbuff);
	CopyInt(&mis->_miDist, tbuff);
	CopyInt(&mis->_miLid, tbuff);
	CopyInt(&mis->_miVar1, tbuff);
	CopyInt(&mis->_miVar2, tbuff);
	CopyInt(&mis->_miVar3, tbuff);
	CopyInt(&mis->_miVar4, tbuff);
	CopyInt(&mis->_miVar5, tbuff);
	CopyInt(&mis->_miVar6, tbuff);
	CopyInt(&mis->_miVar7, tbuff);
	CopyInt(&mis->_miVar8, tbuff);
}

static void SaveObject(int oi)
{
	ObjectStruct *os = &object[oi];

	CopyInt(&os->_otype, tbuff);
	CopyInt(&os->_ox, tbuff);
	CopyInt(&os->_oy, tbuff);
	CopyInt(&os->_oAnimFlag, tbuff);
	tbuff += 4; // Skip pointer _oAnimData
	tbuff += 4; // Skip _oAnimFrameLen
	CopyInt(&os->_oAnimCnt, tbuff);
	CopyInt(&os->_oAnimLen, tbuff);
	CopyInt(&os->_oAnimFrame, tbuff);
	CopyInt(&os->_oAnimWidth, tbuff);
	CopyInt(&os->_oAnimXOffset, tbuff);
	CopyInt(&os->_oSolidFlag, tbuff);
	CopyInt(&os->_oMissFlag, tbuff);
	CopyInt(&os->_oLight, tbuff);
	CopyChar(&os->_oBreak, tbuff);
	CopyChar(&os->_oSelFlag, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&os->_oPreFlag, tbuff);
	CopyInt(&os->_oTrapFlag, tbuff);
	CopyInt(&os->_oDoorFlag, tbuff);
	CopyInt(&os->_olid, tbuff);
	CopyInt(&os->_oRndSeed, tbuff);
	CopyInt(&os->_oVar1, tbuff);
	CopyInt(&os->_oVar2, tbuff);
	CopyInt(&os->_oVar3, tbuff);
	CopyInt(&os->_oVar4, tbuff);
	CopyInt(&os->_oVar5, tbuff);
	CopyInt(&os->_oVar6, tbuff);
	CopyInt(&os->_oVar7, tbuff);
	CopyInt(&os->_oVar8, tbuff);
}

static void SaveQuest(int i)
{
	QuestStruct *pQuest = &quests[i];

	CopyChar(&pQuest->_qidx, tbuff);
	CopyChar(&pQuest->_qactive, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&pQuest->_qtx, tbuff);
	CopyInt(&pQuest->_qty, tbuff);
	CopyInt(&pQuest->_qmsg, tbuff);
	CopyChar(&pQuest->_qvar1, tbuff);
	CopyChar(&pQuest->_qvar2, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&pQuest->_qlog, tbuff);
}

static void SaveLight(LightListStruct *pLight)
{
	CopyInt(&pLight->_lx, tbuff);
	CopyInt(&pLight->_ly, tbuff);
	CopyInt(&pLight->_lunx, tbuff);
	CopyInt(&pLight->_luny, tbuff);
	CopyChar(&pLight->_lradius, tbuff);
	CopyChar(&pLight->_lunr, tbuff);
	SaveBool(pLight->_ldel);
	SaveBool(pLight->_lunflag);
	SaveBool(pLight->_lmine);
	tbuff += 3; // Alignment
	CopyInt(&pLight->_xoff, tbuff);
	CopyInt(&pLight->_yoff, tbuff);
}

static void SavePortal(int i)
{
	PortalStruct *pPortal = &portals[i];

	SaveBool(pPortal->_wopen);
	CopyInt(&pPortal->x, tbuff);
	CopyInt(&pPortal->y, tbuff);
	CopyInt(&pPortal->level, tbuff);
}

void SaveGame()
{
	int i;

	DWORD dwLen = codec_get_encoded_len(FILEBUFF);
	BYTE *fileBuff = DiabloAllocPtr(dwLen);
	tbuff = fileBuff;

	SaveInt(SAVE_INITIAL);
	// save game-info
	SaveInt((gnDifficulty << 8) | currLvl._dLevelIdx);
	for (i = 0; i < NUM_LEVELS; i++) {
		SaveInt(glSeedTbl[i]);
	}
	// save player-data
	SaveInt(ViewX);
	SaveInt(ViewY);
	SaveBool(gbInvflag);
	SaveBool(gbChrflag);
	SaveBool(gbAutomapflag);
	SaveInt(AutoMapScale);

	SavePlayer(mypnum);

	// save meta-data I.
	SaveInt(ReturnLvlX);
	SaveInt(ReturnLvlY);
	SaveInt(ReturnLvl);
	// save meta-data II. (used by LoadGameLevel)
	for (i = 0; i < NUM_QUESTS; i++)
		SaveQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		SavePortal(i);
	// save level-data
	if (currLvl._dType != DTYPE_TOWN) {
		CopyBytes(dDead, MAXDUNX * MAXDUNY, tbuff);
	}

	SaveInt(nummonsters);
	SaveInt(nummissiles);
	SaveInt(nobjects);
	SaveInt(numitems);

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			SaveInt(monstactive[i]);
		for (i = 0; i < nummonsters; i++)
			SaveMonster(monstactive[i]);
		for (i = 0; i < MAXMISSILES; i++)
			SaveChar(missileactive[i]);
		for (i = 0; i < MAXMISSILES; i++)
			SaveChar(missileavail[i]);
		for (i = 0; i < nummissiles; i++)
			SaveMissile(missileactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			SaveChar(objectactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			SaveChar(objectavail[i]);
		for (i = 0; i < nobjects; i++)
			SaveObject(objectactive[i]);
	}
	for (i = 0; i < MAXITEMS; i++)
		SaveChar(itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		SaveChar(itemavail[i]);
	for (i = 0; i < numitems; i++)
		SaveItemData(&items[itemactive[i]]);
	CopyBytes(dFlags, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dItem, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dLight, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dPreLight, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dPlayer, MAXDUNX * MAXDUNY, tbuff);

	if (currLvl._dType != DTYPE_TOWN) {
		CopyInts(dMonster, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(dObject, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(automapview, DMAXX * DMAXY, tbuff);
		CopyBytes(dMissile, MAXDUNX * MAXDUNY, tbuff);
	}
	// save meta-data III. (modified by LoadGameLevel)
	SaveInt(numpremium);
	SaveInt(premiumlevel);
	SaveInt(numlights);
	SaveInt(numvision);

	for (i = 0; i < MAXLIGHTS; i++)
		SaveChar(lightactive[i]);
	for (i = 0; i < numlights; i++)
		SaveLight(&LightList[lightactive[i]]);

	for (i = 0; i < MAXVISION; i++)
		SaveChar(visionactive[i]);
	for (i = 0; i < numvision; i++)
		SaveLight(&VisionList[visionactive[i]]);

	static_assert(NUM_UITEM <= 128, "Save files are no longer compatible.");
	for (i = 0; i < NUM_UITEM; i++)
		SaveBool(UniqueItemFlags[i]);
	for ( ; i < 128; i++)
		SaveBool(FALSE);

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		SaveItemData(&premiumitems[i]);

	dwLen = codec_get_encoded_len(tbuff - fileBuff);
	pfile_write_save_file(SAVEFILE_GAME, fileBuff, tbuff - fileBuff, dwLen);
	mem_free_dbg(fileBuff);
	gbValidSaveFile = TRUE;
	pfile_rename_temp_to_perm();
	pfile_write_hero();
}

void SaveLevel()
{
	int i;
	char szName[MAX_PATH];
	int dwLen;
	BYTE *SaveBuff;

	if (currLvl._dLevelIdx == DLV_TOWN)
		glSeedTbl[DLV_TOWN] = GetRndSeed();

	dwLen = codec_get_encoded_len(FILEBUFF);
	SaveBuff = DiabloAllocPtr(dwLen);
	tbuff = SaveBuff;

	if (currLvl._dType != DTYPE_TOWN) {
		CopyBytes(dDead, MAXDUNX * MAXDUNY, tbuff);
	}

	SaveInt(nummonsters);
	SaveInt(numitems);
	SaveInt(nobjects);

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			SaveInt(monstactive[i]);
		for (i = 0; i < nummonsters; i++)
			SaveMonster(monstactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			SaveChar(objectactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			SaveChar(objectavail[i]);
		for (i = 0; i < nobjects; i++)
			SaveObject(objectactive[i]);
	}

	for (i = 0; i < MAXITEMS; i++)
		SaveChar(itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		SaveChar(itemavail[i]);
	for (i = 0; i < numitems; i++)
		SaveItemData(&items[itemactive[i]]);

	CopyBytes(dFlags, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dItem, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dLight, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dPreLight, MAXDUNX * MAXDUNY, tbuff);

	if (currLvl._dType != DTYPE_TOWN) {
		CopyInts(dMonster, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(dObject, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(automapview, DMAXX * DMAXY, tbuff);
		CopyBytes(dMissile, MAXDUNX * MAXDUNY, tbuff);
	}

	GetTempLevelNames(szName);
	dwLen = codec_get_encoded_len(tbuff - SaveBuff);
	pfile_write_save_file(szName, SaveBuff, tbuff - SaveBuff, dwLen);
	mem_free_dbg(SaveBuff);
}

void LoadLevel()
{
	int i;
	char szName[MAX_PATH];
	BYTE *fileBuff, *tmp;

	GetPermLevelNames(szName);
	fileBuff = pfile_read(szName);
	tbuff = fileBuff;

	if (currLvl._dType != DTYPE_TOWN) {
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dDead);
		SetDead();
	}

	nummonsters = LoadInt();
	numitems = LoadInt();
	nobjects = LoadInt();

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			monstactive[i] = LoadInt();
		for (i = 0; i < nummonsters; i++)
			LoadMonster(monstactive[i]);
		static_assert(MAXOBJECTS <= CHAR_MAX, "LoadLevel handles object-ids as chars.");
		for (i = 0; i < MAXOBJECTS; i++)
			objectactive[i] = LoadChar();
		for (i = 0; i < MAXOBJECTS; i++)
			objectavail[i] = LoadChar();
		for (i = 0; i < nobjects; i++)
			LoadObject(objectactive[i], false);
	}

	static_assert(MAXITEMS <= CHAR_MAX, "LoadLevel handles item-ids as chars.");
	for (i = 0; i < MAXITEMS; i++)
		itemactive[i] = LoadChar();
	for (i = 0; i < MAXITEMS; i++)
		itemavail[i] = LoadChar();
	for (i = 0; i < numitems; i++)
		LoadItem(itemactive[i]);

	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dFlags);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dItem);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dLight);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPreLight);

	if (currLvl._dType != DTYPE_TOWN) {
		CopyInts(tbuff, MAXDUNX * MAXDUNY, dMonster);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dObject);
		CopyBytes(tbuff, DMAXX * DMAXY, automapview);
		memset(dMissile, 0, MAXDUNX * MAXDUNY);
		assert(nummissiles == 0);
	}

	mem_free_dbg(fileBuff);

	AutomapZoomReset();
	//ResyncQuests();
	//SyncPortals();

	static_assert(sizeof(dFlags) == MAXDUNX * MAXDUNY, "Linear traverse of dFlags does not work in LoadLevel.");
	tmp = &dFlags[0][0];
	for (i = 0; i < MAXDUNX * MAXDUNY; i++, tmp++)
		*tmp &= ~(BFLAG_MISSILE | BFLAG_VISIBLE /*| BFLAG_DEAD_PLAYER*/);

	//RedoPlayerLight();
}

DEVILUTION_END_NAMESPACE
