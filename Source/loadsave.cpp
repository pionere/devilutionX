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
	unsigned short buf;
	memcpy(&buf, src, 2);
	tbuff += 2;
	buf = SwapLE16(buf);
	memcpy(dst, &buf, 2);
}

/*static void CopyShorts(const void *src, const int n, void *dst)
{
	const unsigned short *s = reinterpret_cast<const unsigned short *>(src);
	unsigned short *d = reinterpret_cast<unsigned short *>(dst);
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
	CopyInt(tbuff, &is->_iAnimLen);
	CopyInt(tbuff, &is->_iAnimFrame);
	CopyInt(tbuff, &is->_iAnimWidth);
	CopyInt(tbuff, &is->_iAnimXOffset);
	CopyInt(tbuff, &is->_iPostDraw);
	CopyInt(tbuff, &is->_iIdentified);
	CopyBytes(tbuff, 64, is->_iName);
	CopyBytes(tbuff, 64, is->_iIName);
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
	CopyInt(tbuff, &is->_iFlags2);
}

static void LoadItems(ItemStruct *pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		LoadItemData(&pItem[i]);
	}
}

static void LoadPlayer(int pnum)
{
	PlayerStruct *p = &plr[pnum];

	CopyInt(tbuff, &p->_pmode);
	CopyBytes(tbuff, MAX_PATH_LENGTH, p->walkpath);
	CopyInt(tbuff, &p->destAction);
	CopyInt(tbuff, &p->destParam1);
	CopyInt(tbuff, &p->destParam2);
	CopyInt(tbuff, &p->destParam3);
	CopyInt(tbuff, &p->destParam4);
	CopyChar(tbuff, &p->plractive);
	CopyChar(tbuff, &p->_pLvlChanging);
	CopyChar(tbuff, &p->plrlevel);
	tbuff += 1; // Alignment
	CopyInt(tbuff, &p->_px);
	CopyInt(tbuff, &p->_py);
	CopyInt(tbuff, &p->_pfutx);
	CopyInt(tbuff, &p->_pfuty);
	CopyInt(tbuff, &p->_poldx);
	CopyInt(tbuff, &p->_poldy);
	CopyInt(tbuff, &p->_pxoff);
	CopyInt(tbuff, &p->_pyoff);
	CopyInt(tbuff, &p->_pxvel);
	CopyInt(tbuff, &p->_pyvel);
	CopyInt(tbuff, &p->_pdir);
	CopyInt(tbuff, &p->_nextdir);
	CopyInt(tbuff, &p->_pgfxnum);
	tbuff += 4; // Skip pointer _pAnimData
	CopyInt(tbuff, &p->_pAnimFrameLen);
	CopyInt(tbuff, &p->_pAnimCnt);
	CopyInt(tbuff, &p->_pAnimLen);
	CopyInt(tbuff, &p->_pAnimFrame);
	CopyInt(tbuff, &p->_pAnimWidth);
	CopyInt(tbuff, &p->_pAnimXOffset);
	CopyInt(tbuff, &p->_plid);
	CopyInt(tbuff, &p->_pvid);

	CopyChar(tbuff, &p->_pAtkSkill);
	CopyChar(tbuff, &p->_pAtkSkillType);
	CopyChar(tbuff, &p->_pMoveSkill);
	CopyChar(tbuff, &p->_pMoveSkillType);
	CopyChar(tbuff, &p->_pAltAtkSkill);
	CopyChar(tbuff, &p->_pAltAtkSkillType);
	CopyChar(tbuff, &p->_pAltMoveSkill);
	CopyChar(tbuff, &p->_pAltMoveSkillType);

	CopyChar(tbuff, &p->_pTSpell);
	CopyChar(tbuff, &p->_pTSplFrom);
	CopyChar(tbuff, &p->_pOilFrom);
	tbuff += 1; // Alignment

	CopyBytes(tbuff, 64, p->_pSkillLvl);
	CopyBytes(tbuff, 64, p->_pSkillActivity);
	CopyInts(tbuff, 64, p->_pSkillExp);
	CopyInt64(tbuff, &p->_pMemSkills);
	CopyInt64(tbuff, &p->_pAblSkills);
	CopyInt64(tbuff, &p->_pScrlSkills);

	CopyBytes(tbuff, 4, p->_pAtkSkillHotKey);
	CopyBytes(tbuff, 4, p->_pAtkSkillTypeHotKey);
	CopyBytes(tbuff, 4, p->_pMoveSkillHotKey);
	CopyBytes(tbuff, 4, p->_pMoveSkillTypeHotKey);
	CopyBytes(tbuff, 4, p->_pAltAtkSkillHotKey);
	CopyBytes(tbuff, 4, p->_pAltAtkSkillTypeHotKey);
	CopyBytes(tbuff, 4, p->_pAltMoveSkillHotKey);
	CopyBytes(tbuff, 4, p->_pAltMoveSkillTypeHotKey);

	CopyChar(tbuff, &p->_pSkillFlags);
	CopyChar(tbuff, &p->_pSpellFlags);
	CopyChar(tbuff, &p->_pInvincible);
	tbuff += 1; // Alignment

	CopyBytes(tbuff, PLR_NAME_LEN, p->_pName);
	CopyChar(tbuff, &p->_pClass);
	CopyChar(tbuff, &p->_pLevel);
	CopyChar(tbuff, &p->_pLightRad);
	CopyChar(tbuff, &p->_pManaShield);
	CopyShort(tbuff, &p->_pBaseStr);
	CopyShort(tbuff, &p->_pBaseMag);
	CopyShort(tbuff, &p->_pBaseDex);
	CopyShort(tbuff, &p->_pBaseVit);
	CopyInt(tbuff, &p->_pStrength);
	CopyInt(tbuff, &p->_pMagic);
	CopyInt(tbuff, &p->_pDexterity);
	CopyInt(tbuff, &p->_pVitality);
	CopyInt(tbuff, &p->_pHPBase);
	CopyInt(tbuff, &p->_pMaxHPBase);
	CopyInt(tbuff, &p->_pHitPoints);
	CopyInt(tbuff, &p->_pMaxHP);
	CopyInt(tbuff, &p->_pHPPer);
	CopyInt(tbuff, &p->_pManaBase);
	CopyInt(tbuff, &p->_pMaxManaBase);
	CopyInt(tbuff, &p->_pMana);
	CopyInt(tbuff, &p->_pMaxMana);
	CopyInt(tbuff, &p->_pManaPer);
	CopyShort(tbuff, &p->_pStatPts);
	CopyChar(tbuff, &p->_pLvlUp);
	CopyChar(tbuff, &p->_pDiabloKillLevel);
	CopyInt(tbuff, &p->_pExperience);
	CopyInt(tbuff, &p->_pNextExper);
	tbuff += 1; // Skip to Calc _pMagResist
	tbuff += 1; // Skip to Calc _pFireResist
	tbuff += 1; // Skip to Calc _pLghtResist
	tbuff += 1; // Skip to Calc _pAcidResist
	CopyInt(tbuff, &p->_pGold);

	CopyInt(tbuff, &p->_pInfraFlag);
	CopyInt(tbuff, &p->_pVar1);
	CopyInt(tbuff, &p->_pVar2);
	CopyInt(tbuff, &p->_pVar3);
	CopyInt(tbuff, &p->_pVar4);
	CopyInt(tbuff, &p->_pVar5);
	CopyInt(tbuff, &p->_pVar6);
	CopyInt(tbuff, &p->_pVar7);
	CopyInt(tbuff, &p->_pVar8);
	CopyBytes(tbuff, NUMLEVELS + NUM_SETLVL, p->_pLvlVisited);
	tbuff += 1; // Alignment

	CopyInt(tbuff, &p->_pGFXLoad);
	tbuff += 4 * 8; // Skip pointers _pNAnim
	CopyInt(tbuff, &p->_pNFrames);
	CopyInt(tbuff, &p->_pNWidth);
	tbuff += 4 * 8; // Skip pointers _pWAnim
	CopyInt(tbuff, &p->_pWFrames);
	CopyInt(tbuff, &p->_pWWidth);
	tbuff += 4 * 8; // Skip pointers _pAAnim
	CopyInt(tbuff, &p->_pAFrames);
	CopyInt(tbuff, &p->_pAWidth);
	CopyInt(tbuff, &p->_pAFNum);
	tbuff += 4 * 8; // Skip pointers _pLAnim
	tbuff += 4 * 8; // Skip pointers _pFAnim
	tbuff += 4 * 8; // Skip pointers _pTAnim
	CopyInt(tbuff, &p->_pSFrames);
	CopyInt(tbuff, &p->_pSWidth);
	CopyInt(tbuff, &p->_pSFNum);
	tbuff += 4 * 8; // Skip pointers _pHAnim
	CopyInt(tbuff, &p->_pHFrames);
	CopyInt(tbuff, &p->_pHWidth);
	tbuff += 4 * 8; // Skip pointers _pDAnim
	CopyInt(tbuff, &p->_pDFrames);
	CopyInt(tbuff, &p->_pDWidth);
	tbuff += 4 * 8; // Skip pointers _pBAnim
	CopyInt(tbuff, &p->_pBFrames);
	CopyInt(tbuff, &p->_pBWidth);

	LoadItems(p->InvBody, NUM_INVLOC);
	LoadItems(p->SpdList, MAXBELTITEMS);
	LoadItems(p->InvList, NUM_INV_GRID_ELEM);
	CopyBytes(tbuff, NUM_INV_GRID_ELEM, p->InvGrid);
	CopyInt(tbuff, &p->_pNumInv);
	LoadItemData(&p->HoldItem);

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
	tbuff += 4; // Skip to Calc _pIFlags2
	tbuff += 4; // Skip to Calc _pIEnAc
	tbuff += 4; // Skip to Calc _pIFMinDam
	tbuff += 4; // Skip to Calc _pIFMaxDam
	tbuff += 4; // Skip to Calc _pILMinDam
	tbuff += 4; // Skip to Calc _pILMaxDam
	tbuff += 4; // Skip to Calc _pIMMinDam
	tbuff += 4; // Skip to Calc _pIMMaxDam
	tbuff += 4; // Skip to Calc _pIAMinDam
	tbuff += 4; // Skip to Calc _pIAMaxDam
	CopyChar(tbuff, &p->pTownWarps);
	CopyChar(tbuff, &p->pLvlLoad);
	CopyChar(tbuff, &p->pDungMsgs);
	CopyChar(tbuff, &p->pDungMsgs2);

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
}

static void LoadMonster(int mnum)
{
	MonsterStruct *mon = &monster[mnum];

	CopyInt(tbuff, &mon->_mMTidx);
	CopyInt(tbuff, &mon->_mmode);
	CopyChar(tbuff, &mon->_msquelch);
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
	CopyChar(tbuff, &mon->_mAi);
	CopyChar(tbuff, &mon->_mint);
	CopyChar(tbuff, &mon->mArmorClass);

	CopyChar(tbuff, &mon->_mEvasion);
	tbuff += 3; // Alignment

	CopyChar(tbuff, &mon->mMinDamage);
	CopyChar(tbuff, &mon->mMaxDamage);
	CopyChar(tbuff, &mon->mMinDamage2);
	CopyChar(tbuff, &mon->mMaxDamage2);
	CopyShort(tbuff, &mon->mHit);
	CopyShort(tbuff, &mon->mHit2);
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
	CopyInt(tbuff, &mis->_miAnimFlags);
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

static void LoadObject(int oi)
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
	CopyInt(tbuff, &os->_olid);
	CopyInt(tbuff, &os->_oRndSeed);
	CopyInt(tbuff, &os->_oVar1);
	CopyInt(tbuff, &os->_oVar2);
	CopyInt(tbuff, &os->_oVar3);
	CopyInt(tbuff, &os->_oVar4);
	CopyInt(tbuff, &os->_oVar5);
	CopyInt(tbuff, &os->_oVar6);
	CopyInt(tbuff, &os->_oVar7);
	CopyInt(tbuff, &os->_oVar8);
}

static void LoadItem(int ii)
{
	LoadItemData(&items[ii]);
	GetItemFrm(ii);
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
	PortalStruct *pPortal = &portal[i];

	pPortal->_wopen = LoadBool();
	CopyInt(tbuff, &pPortal->x);
	CopyInt(tbuff, &pPortal->y);
	CopyInt(tbuff, &pPortal->level);
}

/*static void RedoPlayerLight()
{
	for (int i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && currLvl._dLevelIdx == plr[i].plrlevel)
			ChangeLightXY(plr[i]._plid, plr[i]._px, plr[i]._py);
	}
}*/

/**
 * @brief Load game state
 * @param firstflag Can be set to false if we are simply reloading the current game
 */
void LoadGame(bool firstflag)
{
	int i;
	BYTE *LoadBuff;
	int _ViewX, _ViewY, _nummonsters, _numitems, _nummissiles, _nobjects;

	FreeGameMem();
	pfile_remove_temp_files();
	LoadBuff = pfile_read(SAVEFILE_GAME);
	tbuff = LoadBuff;

	if (LoadInt() != SAVE_INITIAL)
		app_fatal("Invalid save file");

	i = LoadInt();
	currLvl._dLevelIdx = i & 0xFF;
	EnterLevel(i & 0xFF);
	gnDifficulty = (i >> 8) & 0xFF;

	_ViewX = LoadInt();
	_ViewY = LoadInt();
	gbInvflag = LoadBool();
	gbChrflag = LoadBool();
	_nummonsters = LoadInt();
	_numitems = LoadInt();
	_nummissiles = LoadInt();
	_nobjects = LoadInt();

	for (i = 0; i < NUMLEVELS; i++) {
		glSeedTbl[i] = LoadInt();
	}

	LoadPlayer(myplr);

	ReturnLvlX = LoadInt();
	ReturnLvlY = LoadInt();
	ReturnLvl = LoadInt();

	for (i = 0; i < NUM_QUESTS; i++)
		LoadQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		LoadPortal(i);

	LoadGameLevel(firstflag, ENTRY_LOAD);
	SyncPlrAnim(myplr);

	ViewX = _ViewX;
	ViewY = _ViewY;
	nummonsters = _nummonsters;
	numitems = _numitems;
	nummissiles = _nummissiles;
	nobjects = _nobjects;

	for (i = 0; i < MAXMONSTERS; i++)
		tbuff += 4; // Skip monstkills[i]

	if (currLvl._dType != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			monstactive[i] = LoadInt();
		for (i = 0; i < nummonsters; i++)
			LoadMonster(monstactive[i]);
		for (i = 0; i < MAXMISSILES; i++)
			missileactive[i] = LoadChar();
		for (i = 0; i < MAXMISSILES; i++)
			missileavail[i] = LoadChar();
		for (i = 0; i < nummissiles; i++)
			LoadMissile(missileactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			objectactive[i] = LoadChar();
		for (i = 0; i < MAXOBJECTS; i++)
			objectavail[i] = LoadChar();
		for (i = 0; i < nobjects; i++)
			LoadObject(objectactive[i]);
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);
	}

	numlights = LoadInt();
	for (i = 0; i < MAXLIGHTS; i++)
		lightactive[i] = LoadChar();
	for (i = 0; i < numlights; i++)
		LoadLight(&LightList[lightactive[i]]);

	numvision = LoadInt();
	for (i = 0; i < MAXVISION; i++)
		visionactive[i] = LoadChar();
	for (i = 0; i < numvision; i++)
		LoadLight(&VisionList[visionactive[i]]);

	for (i = 0; i < MAXITEMS; i++)
		itemactive[i] = LoadChar();
	for (i = 0; i < MAXITEMS; i++)
		itemavail[i] = LoadChar();
	for (i = 0; i < numitems; i++)
		LoadItem(itemactive[i]);

	static_assert(NUM_UITEM <= 128, "Save files are no longer compatible.");
	for (i = 0; i < NUM_UITEM; i++)
		UniqueItemFlags[i] = LoadBool();
	for ( ; i < 128; i++)
		LoadBool();

	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dLight);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPreLight);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dFlags);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dPlayer);
	CopyBytes(tbuff, MAXDUNX * MAXDUNY, dItem);

	if (currLvl._dType != DTYPE_TOWN) {
		CopyInts(tbuff, MAXDUNX * MAXDUNY, dMonster);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dDead);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dObject);
		CopyBytes(tbuff, DMAXX * DMAXY, automapview);
		CopyBytes(tbuff, MAXDUNX * MAXDUNY, dMissile);
	}

	numpremium = LoadInt();
	premiumlevel = LoadInt();

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		LoadItemData(&premiumitems[i]);

	gbAutomapflag = LoadBool();
	AutoMapScale = LoadInt();
	mem_free_dbg(LoadBuff);
	AutomapZoomReset();
	ResyncQuests();

	//RedoPlayerLight();
	//ProcessLightList();
	//RedoPlayerVision();
	//ProcessVisionList();

	SyncMissilesAnim();
	ResetPal();
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
	CopyInt(&is->_iAnimLen, tbuff);
	CopyInt(&is->_iAnimFrame, tbuff);
	CopyInt(&is->_iAnimWidth, tbuff);
	CopyInt(&is->_iAnimXOffset, tbuff);
	CopyInt(&is->_iPostDraw, tbuff);
	CopyInt(&is->_iIdentified, tbuff);
	CopyBytes(is->_iName, 64, tbuff);
	CopyBytes(is->_iIName, 64, tbuff);
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
	CopyInt(&is->_iFlags2, tbuff);
}

static void SaveItems(ItemStruct *pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		SaveItemData(&pItem[i]);
	}
}

static void SavePlayer(int pnum)
{
	PlayerStruct *p = &plr[pnum];

	CopyInt(&p->_pmode, tbuff);
	CopyBytes(p->walkpath, MAX_PATH_LENGTH, tbuff);
	CopyInt(&p->destAction, tbuff);
	CopyInt(&p->destParam1, tbuff);
	CopyInt(&p->destParam2, tbuff);
	CopyInt(&p->destParam3, tbuff);
	CopyInt(&p->destParam4, tbuff);
	CopyChar(&p->plractive, tbuff);
	CopyChar(&p->_pLvlChanging, tbuff);
	CopyChar(&p->plrlevel, tbuff);
	tbuff += 1; // Alignment
	CopyInt(&p->_px, tbuff);
	CopyInt(&p->_py, tbuff);
	CopyInt(&p->_pfutx, tbuff);
	CopyInt(&p->_pfuty, tbuff);
	CopyInt(&p->_poldx, tbuff);
	CopyInt(&p->_poldy, tbuff);
	CopyInt(&p->_pxoff, tbuff);
	CopyInt(&p->_pyoff, tbuff);
	CopyInt(&p->_pxvel, tbuff);
	CopyInt(&p->_pyvel, tbuff);
	CopyInt(&p->_pdir, tbuff);
	CopyInt(&p->_nextdir, tbuff);
	CopyInt(&p->_pgfxnum, tbuff);
	tbuff += 4; // Skip pointer _pAnimData
	CopyInt(&p->_pAnimFrameLen, tbuff);
	CopyInt(&p->_pAnimCnt, tbuff);
	CopyInt(&p->_pAnimLen, tbuff);
	CopyInt(&p->_pAnimFrame, tbuff);
	CopyInt(&p->_pAnimWidth, tbuff);
	CopyInt(&p->_pAnimXOffset, tbuff);
	CopyInt(&p->_plid, tbuff);
	CopyInt(&p->_pvid, tbuff);

	CopyChar(&p->_pAtkSkill, tbuff);
	CopyChar(&p->_pAtkSkillType, tbuff);
	CopyChar(&p->_pMoveSkill, tbuff);
	CopyChar(&p->_pMoveSkillType, tbuff);

	CopyChar(&p->_pAltAtkSkill, tbuff);
	CopyChar(&p->_pAltAtkSkillType, tbuff);
	CopyChar(&p->_pAltMoveSkill, tbuff);
	CopyChar(&p->_pAltMoveSkillType, tbuff);

	CopyChar(&p->_pTSpell, tbuff);
	CopyChar(&p->_pTSplFrom, tbuff);
	CopyChar(&p->_pOilFrom, tbuff);
	tbuff += 1; // Alignment

	CopyBytes(p->_pSkillLvl, 64, tbuff);
	CopyBytes(p->_pSkillActivity, 64, tbuff);
	CopyInts(p->_pSkillExp, 64, tbuff);
	CopyInt64(&p->_pMemSkills, tbuff);
	CopyInt64(&p->_pAblSkills, tbuff);
	CopyInt64(&p->_pScrlSkills, tbuff);

	CopyBytes(p->_pAtkSkillHotKey, 4, tbuff);
	CopyBytes(p->_pAtkSkillTypeHotKey, 4, tbuff);
	CopyBytes(p->_pMoveSkillHotKey, 4, tbuff);
	CopyBytes(p->_pMoveSkillTypeHotKey, 4, tbuff);
	CopyBytes(p->_pAltAtkSkillHotKey, 4, tbuff);
	CopyBytes(p->_pAltAtkSkillTypeHotKey, 4, tbuff);
	CopyBytes(p->_pAltMoveSkillHotKey, 4, tbuff);
	CopyBytes(p->_pAltMoveSkillTypeHotKey, 4, tbuff);

	CopyChar(&p->_pSkillFlags, tbuff);
	CopyChar(&p->_pSpellFlags, tbuff);
	CopyChar(&p->_pInvincible, tbuff);
	tbuff += 1; // Alignment

	CopyBytes(p->_pName, PLR_NAME_LEN, tbuff);
	CopyChar(&p->_pClass, tbuff);
	CopyChar(&p->_pLevel, tbuff);
	CopyChar(&p->_pLightRad, tbuff);
	CopyChar(&p->_pManaShield, tbuff);
	CopyShort(&p->_pBaseStr, tbuff);
	CopyShort(&p->_pBaseMag, tbuff);
	CopyShort(&p->_pBaseDex, tbuff);
	CopyShort(&p->_pBaseVit, tbuff);
	CopyInt(&p->_pStrength, tbuff);
	CopyInt(&p->_pMagic, tbuff);
	CopyInt(&p->_pDexterity, tbuff);
	CopyInt(&p->_pVitality, tbuff);
	CopyInt(&p->_pHPBase, tbuff);
	CopyInt(&p->_pMaxHPBase, tbuff);
	CopyInt(&p->_pHitPoints, tbuff);
	CopyInt(&p->_pMaxHP, tbuff);
	CopyInt(&p->_pHPPer, tbuff);
	CopyInt(&p->_pManaBase, tbuff);
	CopyInt(&p->_pMaxManaBase, tbuff);
	CopyInt(&p->_pMana, tbuff);
	CopyInt(&p->_pMaxMana, tbuff);
	CopyInt(&p->_pManaPer, tbuff);
	CopyShort(&p->_pStatPts, tbuff);
	CopyChar(&p->_pLvlUp, tbuff);
	CopyChar(&p->_pDiabloKillLevel, tbuff);
	CopyInt(&p->_pExperience, tbuff);
	CopyInt(&p->_pNextExper, tbuff);
	tbuff += 1; // Skip to Calc _pMagResist
	tbuff += 1; // Skip to Calc _pFireResist
	tbuff += 1; // Skip to Calc _pLghtResist
	tbuff += 1; // Skip to Calc _pAcidResist
	CopyInt(&p->_pGold, tbuff);

	CopyInt(&p->_pInfraFlag, tbuff);
	CopyInt(&p->_pVar1, tbuff);
	CopyInt(&p->_pVar2, tbuff);
	CopyInt(&p->_pVar3, tbuff);
	CopyInt(&p->_pVar4, tbuff);
	CopyInt(&p->_pVar5, tbuff);
	CopyInt(&p->_pVar6, tbuff);
	CopyInt(&p->_pVar7, tbuff);
	CopyInt(&p->_pVar8, tbuff);
	CopyBytes(p->_pLvlVisited, NUMLEVELS + NUM_SETLVL, tbuff);
	tbuff += 1;                                     // Alignment

	CopyInt(&p->_pGFXLoad, tbuff);
	tbuff += 4 * 8; // Skip pointers _pNAnim
	CopyInt(&p->_pNFrames, tbuff);
	CopyInt(&p->_pNWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pWAnim
	CopyInt(&p->_pWFrames, tbuff);
	CopyInt(&p->_pWWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pAAnim
	CopyInt(&p->_pAFrames, tbuff);
	CopyInt(&p->_pAWidth, tbuff);
	CopyInt(&p->_pAFNum, tbuff);
	tbuff += 4 * 8; // Skip pointers _pLAnim
	tbuff += 4 * 8; // Skip pointers _pFAnim
	tbuff += 4 * 8; // Skip pointers _pTAnim
	CopyInt(&p->_pSFrames, tbuff);
	CopyInt(&p->_pSWidth, tbuff);
	CopyInt(&p->_pSFNum, tbuff);
	tbuff += 4 * 8; // Skip pointers _pHAnim
	CopyInt(&p->_pHFrames, tbuff);
	CopyInt(&p->_pHWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pDAnim
	CopyInt(&p->_pDFrames, tbuff);
	CopyInt(&p->_pDWidth, tbuff);
	tbuff += 4 * 8; // Skip pointers _pBAnim
	CopyInt(&p->_pBFrames, tbuff);
	CopyInt(&p->_pBWidth, tbuff);

	SaveItems(p->InvBody, NUM_INVLOC);
	SaveItems(p->SpdList, MAXBELTITEMS);
	SaveItems(p->InvList, NUM_INV_GRID_ELEM);
	CopyBytes(p->InvGrid, NUM_INV_GRID_ELEM, tbuff);
	CopyInt(&p->_pNumInv, tbuff);
	SaveItemData(&p->HoldItem);

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
	tbuff += 4; // Skip to Calc _pIFlags2
	tbuff += 4; // Skip to Calc _pIEnAc
	tbuff += 4; // Skip to Calc _pIFMinDam
	tbuff += 4; // Skip to Calc _pIFMaxDam
	tbuff += 4; // Skip to Calc _pILMinDam
	tbuff += 4; // Skip to Calc _pILMaxDam
	tbuff += 4; // Skip to Calc _pIMMinDam
	tbuff += 4; // Skip to Calc _pIMMaxDam
	tbuff += 4; // Skip to Calc _pIAMinDam
	tbuff += 4; // Skip to Calc _pIAMaxDam
	CopyChar(&p->pTownWarps, tbuff);
	CopyChar(&p->pLvlLoad, tbuff);
	CopyChar(&p->pDungMsgs, tbuff);
	CopyChar(&p->pDungMsgs2, tbuff);

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

	CopyInt(&mon->_mMTidx, tbuff);
	CopyInt(&mon->_mmode, tbuff);
	CopyChar(&mon->_msquelch, tbuff);
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
	CopyChar(&mon->_mAi, tbuff);
	CopyChar(&mon->_mint, tbuff);
	CopyChar(&mon->mArmorClass, tbuff);

	CopyChar(&mon->_mEvasion, tbuff);
	tbuff += 3; // Alignment

	CopyChar(&mon->mMinDamage, tbuff);
	CopyChar(&mon->mMaxDamage, tbuff);
	CopyChar(&mon->mMinDamage2, tbuff);
	CopyChar(&mon->mMaxDamage2, tbuff);
	CopyShort(&mon->mHit, tbuff);
	CopyShort(&mon->mHit2, tbuff);
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
	CopyInt(&mis->_miAnimFlags, tbuff);
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
	PortalStruct *pPortal = &portal[i];

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

	SaveInt((gnDifficulty << 8) | currLvl._dLevelIdx);
	SaveInt(ViewX);
	SaveInt(ViewY);
	SaveBool(gbInvflag);
	SaveBool(gbChrflag);
	SaveInt(nummonsters);
	SaveInt(numitems);
	SaveInt(nummissiles);
	SaveInt(nobjects);

	for (i = 0; i < NUMLEVELS; i++) {
		SaveInt(glSeedTbl[i]);
	}

	SavePlayer(myplr);

	SaveInt(ReturnLvlX);
	SaveInt(ReturnLvlY);
	SaveInt(ReturnLvl);

	for (i = 0; i < NUM_QUESTS; i++)
		SaveQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		SavePortal(i);
	for (i = 0; i < MAXMONSTERS; i++)
		tbuff += 4; // Skip monstkills[i]

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
	SaveInt(numlights);
	for (i = 0; i < MAXLIGHTS; i++)
		SaveChar(lightactive[i]);
	for (i = 0; i < numlights; i++)
		SaveLight(&LightList[lightactive[i]]);

	SaveInt(numvision);
	for (i = 0; i < MAXVISION; i++)
		SaveChar(visionactive[i]);
	for (i = 0; i < numvision; i++)
		SaveLight(&VisionList[visionactive[i]]);

	for (i = 0; i < MAXITEMS; i++)
		SaveChar(itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		SaveChar(itemavail[i]);
	for (i = 0; i < numitems; i++)
		SaveItemData(&items[itemactive[i]]);
	static_assert(NUM_UITEM <= 128, "Save files are no longer compatible.");
	for (i = 0; i < NUM_UITEM; i++)
		SaveBool(UniqueItemFlags[i]);
	for ( ; i < 128; i++)
		SaveBool(FALSE);

	CopyBytes(dLight, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dPreLight, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dFlags, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dPlayer, MAXDUNX * MAXDUNY, tbuff);
	CopyBytes(dItem, MAXDUNX * MAXDUNY, tbuff);

	if (currLvl._dType != DTYPE_TOWN) {
		CopyInts(dMonster, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(dDead, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(dObject, MAXDUNX * MAXDUNY, tbuff);
		CopyBytes(automapview, DMAXX * DMAXY, tbuff);
		CopyBytes(dMissile, MAXDUNX * MAXDUNY, tbuff);
	}

	SaveInt(numpremium);
	SaveInt(premiumlevel);

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		SaveItemData(&premiumitems[i]);

	SaveBool(gbAutomapflag);
	SaveInt(AutoMapScale);
	dwLen = codec_get_encoded_len(tbuff - fileBuff);
	pfile_write_save_file(SAVEFILE_GAME, fileBuff, tbuff - fileBuff, dwLen);
	mem_free_dbg(fileBuff);
	gbValidSaveFile = TRUE;
	pfile_rename_temp_to_perm();
	pfile_write_hero();
}

void SaveLevel()
{
	int i, j;
	char szName[MAX_PATH];
	int dwLen;
	BYTE *SaveBuff;

	if (currLvl._dLevelIdx == 0)
		glSeedTbl[0] = GetRndSeed();

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

	for (i = 0; i < MAXDUNX; i++)
		for (j = 0; j < MAXDUNY; j++)
			SaveChar(dFlags[i][j] & ~(BFLAG_MISSILE | BFLAG_VISIBLE | BFLAG_DEAD_PLAYER));

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

	plr[myplr]._pLvlVisited[currLvl._dLevelIdx] = TRUE;
}

void LoadLevel()
{
	int i;
	char szName[MAX_PATH];
	BYTE *fileBuff;

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
		for (i = 0; i < MAXOBJECTS; i++)
			objectactive[i] = LoadChar();
		for (i = 0; i < MAXOBJECTS; i++)
			objectavail[i] = LoadChar();
		for (i = 0; i < nobjects; i++)
			LoadObject(objectactive[i]);
		for (i = 0; i < nobjects; i++) {
			object[objectactive[i]]._olid = -1; // reset dynamic lights
			SyncObjectAnim(objectactive[i]);
		}
	}

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
		memset(dMissile, 0, MAXDUNX * MAXDUNY); /// BUGFIX: supposed to load saved missiles with "CopyBytes"?
	}

	AutomapZoomReset();
	ResyncQuests();
	SyncPortals();

	//RedoPlayerLight();

	mem_free_dbg(fileBuff);
}

DEVILUTION_END_NAMESPACE
