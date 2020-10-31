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

/**
 * @brief Load game state
 * @param firstflag Can be set to false if we are simply reloading the current game
 */
void LoadGame(BOOL firstflag)
{
	int i, j;
	DWORD dwLen;
	char szName[MAX_PATH];
	BYTE *LoadBuff;
	int _ViewX, _ViewY, _nummonsters, _numitems, _nummissiles, _nobjects;

	FreeGameMem();
	pfile_remove_temp_files();
	pfile_get_game_name(szName);
	LoadBuff = pfile_read(szName, &dwLen);
	tbuff = LoadBuff;

	if (ILoad() != SAVE_INITIAL)
		app_fatal("Invalid save file");

	setlevel = OLoad();
	setlvlnum = WLoad();
	currlevel = WLoad();
	leveltype = WLoad();
	_ViewX = WLoad();
	_ViewY = WLoad();
	invflag = OLoad();
	chrflag = OLoad();
	_nummonsters = WLoad();
	_numitems = WLoad();
	_nummissiles = WLoad();
	_nobjects = WLoad();

	for (i = 0; i < NUMLEVELS; i++) {
		glSeedTbl[i] = ILoad();
		gnLevelTypeTbl[i] = WLoad();
	}

	LoadPlayer(myplr);

	gnDifficulty = plr[myplr].pDifficulty;
	if (gnDifficulty < DIFF_NORMAL || gnDifficulty > DIFF_HELL)
		gnDifficulty = DIFF_NORMAL;

	for (i = 0; i < MAXQUESTS; i++)
		LoadQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		LoadPortal(i);

	LoadGameLevel(firstflag, 4);
	SyncInitPlr(myplr);
	SyncPlrAnim(myplr);

	ViewX = _ViewX;
	ViewY = _ViewY;
	nummonsters = _nummonsters;
	numitems = _numitems;
	nummissiles = _nummissiles;
	nobjects = _nobjects;

	for (i = 0; i < MAXMONSTERS; i++)
		monstkills[i] = ILoad();

	if (leveltype != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			monstactive[i] = WLoad();
		for (i = 0; i < nummonsters; i++)
			LoadMonster(monstactive[i]);
		for (i = 0; i < MAXMISSILES; i++)
			missileactive[i] = BLoad();
		for (i = 0; i < MAXMISSILES; i++)
			missileavail[i] = BLoad();
		for (i = 0; i < nummissiles; i++)
			LoadMissile(missileactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			objectactive[i] = BLoad();
		for (i = 0; i < MAXOBJECTS; i++)
			objectavail[i] = BLoad();
		for (i = 0; i < nobjects; i++)
			LoadObject(objectactive[i]);
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);

		numlights = WLoad();

		for (i = 0; i < MAXLIGHTS; i++)
			lightactive[i] = BLoad();
		for (i = 0; i < numlights; i++)
			LoadLighting(lightactive[i]);

		visionid = WLoad();
		numvision = WLoad();

		for (i = 0; i < numvision; i++)
			LoadVision(i);
	}

	for (i = 0; i < MAXITEMS; i++)
		itemactive[i] = BLoad();
	for (i = 0; i < MAXITEMS; i++)
		itemavail[i] = BLoad();
	for (i = 0; i < numitems; i++)
		LoadItem(itemactive[i]);
	for (i = 0; i < 128; i++)
		UniqueItemFlag[i] = OLoad();

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			dLight[i][j] = BLoad();
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			dFlags[i][j] = BLoad();
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			dPlayer[i][j] = BLoad();
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			dItem[i][j] = BLoad();
	}

	if (leveltype != DTYPE_TOWN) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dMonster[i][j] = WLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dDead[i][j] = BLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dObject[i][j] = BLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dLight[i][j] = BLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dPreLight[i][j] = BLoad();
		}
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++)
				automapview[i][j] = OLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dMissile[i][j] = BLoad();
		}
	}

	numpremium = WLoad();
	premiumlevel = WLoad();

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		LoadPremium(i);

	automapflag = OLoad();
	AutoMapScale = WLoad();
	mem_free_dbg(LoadBuff);
	AutomapZoomReset();
	ResyncQuests();

	if (leveltype != DTYPE_TOWN)
		ProcessLightList();

	RedoPlayerVision();
	ProcessVisionList();
	missiles_process_charge();
	ResetPal();
	NewCursor(CURSOR_HAND);
	gbProcessPlayers = TRUE;
}

char BLoad()
{
	return *tbuff++;
}

int WLoad()
{
	int rv = *tbuff++ << 24;
	rv |= *tbuff++ << 16;
	rv |= *tbuff++ << 8;
	rv |= *tbuff++;

	return rv;
}

int ILoad()
{
	int rv = *tbuff++ << 24;
	rv |= *tbuff++ << 16;
	rv |= *tbuff++ << 8;
	rv |= *tbuff++;

	return rv;
}

BOOL OLoad()
{
	if (*tbuff++)
		return TRUE;
	else
		return FALSE;
}

void CopyBytes(const void *src, const int n, void *dst)
{
	memcpy(dst, src, n);
	tbuff += n;
}

void CopyChar(const void *src, void *dst)
{
	*(char *)dst = *(char *)src;
	tbuff += 1;
}

void CopyShort(const void *src, void *dst)
{
	unsigned short buf;
	memcpy(&buf, src, 2);
	tbuff += 2;
	buf = SwapLE16(buf);
	memcpy(dst, &buf, 2);
}

void CopyShorts(const void *src, const int n, void *dst)
{
	const unsigned short *s = reinterpret_cast<const unsigned short *>(src);
	unsigned short *d = reinterpret_cast<unsigned short *>(dst);
	for (int i = 0; i < n; i++) {
		CopyShort(s, d);
		++d;
		++s;
	}
}

void CopyInt(const void *src, void *dst)
{
	unsigned int buf;
	memcpy(&buf, src, 4);
	tbuff += 4;
	buf = SwapLE32(buf);
	memcpy(dst, &buf, 4);
}

void CopyInts(const void *src, const int n, void *dst)
{
	const unsigned int *s = reinterpret_cast<const unsigned int *>(src);
	const unsigned int *d = reinterpret_cast<unsigned int *>(dst);
	for (int i = 0; i < n; i++) {
		CopyInt(s, (void*)d);
		++d;
		++s;
	}
}

void CopyInt64(const void *src, void *dst)
{
	unsigned long long buf;
	memcpy(&buf, src, 8);
	tbuff += 8;
	buf = SDL_SwapLE64(buf);
	memcpy(dst, &buf, 8);
}

void LoadPlayer(int pnum)
{
	PlayerStruct *p = &plr[pnum];

	CopyInt(tbuff, &p->_pmode);
	CopyBytes(tbuff, MAX_PATH_LENGTH, p->walkpath);
	CopyBytes(tbuff, 1, &p->plractive);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &p->destAction);
	CopyInt(tbuff, &p->destParam1);
	CopyInt(tbuff, &p->destParam2);
	CopyInt(tbuff, &p->destParam3);
	CopyInt(tbuff, &p->destParam4);
	CopyInt(tbuff, &p->plrlevel);
	CopyInt(tbuff, &p->_px);
	CopyInt(tbuff, &p->_py);
	CopyInt(tbuff, &p->_pfutx);
	CopyInt(tbuff, &p->_pfuty);
	CopyInt(tbuff, &p->_ptargx);
	CopyInt(tbuff, &p->_ptargy);
	CopyInt(tbuff, &p->_pownerx);
	CopyInt(tbuff, &p->_pownery);
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
	CopyInt(tbuff, &p->_pAnimDelay);
	CopyInt(tbuff, &p->_pAnimCnt);
	CopyInt(tbuff, &p->_pAnimLen);
	CopyInt(tbuff, &p->_pAnimFrame);
	CopyInt(tbuff, &p->_pAnimWidth);
	CopyInt(tbuff, &p->_pAnimWidth2);
	tbuff += 4; // Skip _peflag
	CopyInt(tbuff, &p->_plid);
	CopyInt(tbuff, &p->_pvid);

	CopyInt(tbuff, &p->_pSpell);
	CopyChar(tbuff, &p->_pSplType);
	CopyChar(tbuff, &p->_pSplFrom);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &p->_pTSpell);
	CopyChar(tbuff, &p->_pTSplType);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &p->_pRSpell);
	CopyChar(tbuff, &p->_pRSplType);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &p->_pSBkSpell);
	CopyChar(tbuff, &p->_pSBkSplType);
	CopyBytes(tbuff, 64, &p->_pSplLvl);
	tbuff += 7; // Alignment
	CopyInt64(tbuff, &p->_pMemSpells);
	CopyInt64(tbuff, &p->_pAblSpells);
	CopyInt64(tbuff, &p->_pScrlSpells);
	CopyChar(tbuff, &p->_pSpellFlags);
	tbuff += 3; // Alignment
	CopyInts(tbuff, 4, &p->_pSplHotKey);
	CopyBytes(tbuff, 4, &p->_pSplTHotKey);

	CopyInt(tbuff, &p->_pwtype);
	CopyChar(tbuff, &p->_pBlockFlag);
	CopyChar(tbuff, &p->_pInvincible);
	CopyChar(tbuff, &p->_pLightRad);
	CopyChar(tbuff, &p->_pLvlChanging);

	CopyBytes(tbuff, PLR_NAME_LEN, &p->_pName);
	CopyChar(tbuff, &p->_pClass);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &p->_pStrength);
	CopyInt(tbuff, &p->_pBaseStr);
	CopyInt(tbuff, &p->_pMagic);
	CopyInt(tbuff, &p->_pBaseMag);
	CopyInt(tbuff, &p->_pDexterity);
	CopyInt(tbuff, &p->_pBaseDex);
	CopyInt(tbuff, &p->_pVitality);
	CopyInt(tbuff, &p->_pBaseVit);
	CopyInt(tbuff, &p->_pStatPts);
	CopyInt(tbuff, &p->_pDamageMod);
	CopyInt(tbuff, &p->_pBaseToBlk);
	if (p->_pBaseToBlk == 0) {
		p->_pBaseToBlk = ToBlkTbl[p->_pClass];
	}
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
	CopyChar(tbuff, &p->_pLevel);
	CopyChar(tbuff, &p->_pMaxLvl);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &p->_pExperience);
	CopyInt(tbuff, &p->_pMaxExp);
	CopyInt(tbuff, &p->_pNextExper);
	CopyChar(tbuff, &p->_pArmorClass);
	CopyChar(tbuff, &p->_pMagResist);
	CopyChar(tbuff, &p->_pFireResist);
	CopyChar(tbuff, &p->_pLghtResist);
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
	CopyBytes(tbuff, NUMLEVELS, &p->_pLvlVisited);
	CopyBytes(tbuff, NUMLEVELS, &p->_pSLvlVisited);
	tbuff += 2; // Alignment

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

	LoadItems(NUM_INVLOC, p->InvBody);
	LoadItems(NUM_INV_GRID_ELEM, p->InvList);
	CopyInt(tbuff, &p->_pNumInv);
	CopyBytes(tbuff, NUM_INV_GRID_ELEM, p->InvGrid);
	LoadItems(MAXBELTITEMS, p->SpdList);
	LoadItemData(&p->HoldItem);

	CopyInt(tbuff, &p->_pIMinDam);
	CopyInt(tbuff, &p->_pIMaxDam);
	CopyInt(tbuff, &p->_pIAC);
	CopyInt(tbuff, &p->_pIBonusDam);
	CopyInt(tbuff, &p->_pIBonusToHit);
	CopyInt(tbuff, &p->_pIBonusAC);
	CopyInt(tbuff, &p->_pIBonusDamMod);
	tbuff += 4; // Alignment

	CopyInt64(tbuff, &p->_pISpells);
	CopyInt(tbuff, &p->_pIFlags);
	CopyInt(tbuff, &p->_pIGetHit);
	CopyChar(tbuff, &p->_pISplLvlAdd);
	CopyChar(tbuff, &p->_pISplCost);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &p->_pISplDur);
	CopyInt(tbuff, &p->_pIEnAc);
	CopyInt(tbuff, &p->_pIFMinDam);
	CopyInt(tbuff, &p->_pIFMaxDam);
	CopyInt(tbuff, &p->_pILMinDam);
	CopyInt(tbuff, &p->_pILMaxDam);
	CopyInt(tbuff, &p->_pOilType);
	CopyChar(tbuff, &p->pTownWarps);
	CopyChar(tbuff, &p->pDungMsgs);
	CopyChar(tbuff, &p->pLvlLoad);
#ifdef HELLFIRE
	CopyChar(tbuff, &p->pDungMsgs2);
	p->pBattleNet = false;
#else
	CopyChar(tbuff, &p->pBattleNet);
#endif
	CopyChar(tbuff, &p->pManaShield);
#ifdef HELLFIRE
	CopyChar(tbuff, &p->pDungMsgs2);
	CopyBytes(tbuff, 2, &p->bReserved);
#else
	CopyBytes(tbuff, 3, &p->bReserved);
#endif
	CopyShort(tbuff, &p->wReflection);
	CopyShorts(tbuff, 7, &p->wReserved);

	CopyInt(tbuff, &p->pDiabloKillLevel);
	CopyInt(tbuff, &p->pDifficulty);
	CopyInt(tbuff, &p->pDamAcFlags);
	CopyInts(tbuff, 5, &p->dwReserved);

	// Omit pointer _pNData
	// Omit pointer _pWData
	// Omit pointer _pAData
	// Omit pointer _pLData
	// Omit pointer _pFData
	// Omit pointer  _pTData
	// Omit pointer _pHData
	// Omit pointer _pDData
	// Omit pointer _pBData
	// Omit pointer pReserved
}

void LoadMonster(int mnum)
{
	MonsterStruct *mon = &monster[mnum];

	CopyInt(tbuff, &mon->_mMTidx);
	CopyInt(tbuff, &mon->_mmode);
	CopyChar(tbuff, &mon->_mgoal);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &mon->_mgoalvar1);
	CopyInt(tbuff, &mon->_mgoalvar2);
	CopyInt(tbuff, &mon->_mgoalvar3);
	CopyInt(tbuff, &mon->field_18);
	CopyChar(tbuff, &mon->_pathcount);
	tbuff += 3; // Alignment
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
	CopyShort(tbuff, &mon->falign_52);

	tbuff += 4; // Skip pointer _mAnimData
	CopyInt(tbuff, &mon->_mAnimDelay);
	CopyInt(tbuff, &mon->_mAnimCnt);
	CopyInt(tbuff, &mon->_mAnimLen);
	CopyInt(tbuff, &mon->_mAnimFrame);
	tbuff += 4; // Skip _meflag
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

	CopyChar(tbuff, &mon->_mAi);
	CopyChar(tbuff, &mon->_mint);
	CopyShort(tbuff, &mon->falign_9A);
	CopyInt(tbuff, &mon->_mFlags);
	CopyChar(tbuff, &mon->_msquelch);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &mon->falign_A4);
	CopyInt(tbuff, &mon->_lastx);
	CopyInt(tbuff, &mon->_lasty);
	CopyInt(tbuff, &mon->_mRndSeed);
	CopyInt(tbuff, &mon->_mAISeed);
	CopyInt(tbuff, &mon->falign_B8);

	CopyChar(tbuff, &mon->_uniqtype);
	CopyChar(tbuff, &mon->_uniqtrans);
	CopyChar(tbuff, &mon->_udeadval);

	CopyChar(tbuff, &mon->mWhoHit);
	CopyChar(tbuff, &mon->mLevel);
	tbuff += 1; // Alignment
	CopyShort(tbuff, &mon->mExp);

	tbuff += 1; // Skip mHit as it's already initialized
	CopyChar(tbuff, &mon->mMinDamage);
	CopyChar(tbuff, &mon->mMaxDamage);
	tbuff += 1; // Skip mHit2 as it's already initialized
	CopyChar(tbuff, &mon->mMinDamage2);
	CopyChar(tbuff, &mon->mMaxDamage2);
	CopyChar(tbuff, &mon->mArmorClass);
	CopyChar(tbuff, &mon->falign_CB);
	CopyShort(tbuff, &mon->mMagicRes);
	tbuff += 2; // Alignment

	CopyInt(tbuff, &mon->mtalkmsg);
	CopyChar(tbuff, &mon->leader);
	CopyChar(tbuff, &mon->leaderflag);
	CopyChar(tbuff, &mon->packsize);
	CopyChar(tbuff, &mon->mlid);

	// Omit pointer mName;
	// Omit pointer MType;
	// Omit pointer MData;

	SyncMonsterAnim(mnum);
}

void LoadMissile(int mi)
{
	MissileStruct *mis = &missile[mi];

	CopyInt(tbuff, &mis->_mitype);
	CopyInt(tbuff, &mis->_mix);
	CopyInt(tbuff, &mis->_miy);
	CopyInt(tbuff, &mis->_mixoff);
	CopyInt(tbuff, &mis->_miyoff);
	CopyInt(tbuff, &mis->_mixvel);
	CopyInt(tbuff, &mis->_miyvel);
	CopyInt(tbuff, &mis->_misx);
	CopyInt(tbuff, &mis->_misy);
	CopyInt(tbuff, &mis->_mitxoff);
	CopyInt(tbuff, &mis->_mityoff);
	CopyInt(tbuff, &mis->_mimfnum);
	CopyInt(tbuff, &mis->_mispllvl);
	CopyInt(tbuff, &mis->_miDelFlag);
	CopyChar(tbuff, &mis->_miAnimType);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &mis->_miAnimFlags);
	tbuff += 4; // Skip pointer _miAnimData
	CopyInt(tbuff, &mis->_miAnimDelay);
	CopyInt(tbuff, &mis->_miAnimLen);
	CopyInt(tbuff, &mis->_miAnimWidth);
	CopyInt(tbuff, &mis->_miAnimWidth2);
	CopyInt(tbuff, &mis->_miAnimCnt);
	CopyInt(tbuff, &mis->_miAnimAdd);
	CopyInt(tbuff, &mis->_miAnimFrame);
	CopyInt(tbuff, &mis->_miDrawFlag);
	CopyInt(tbuff, &mis->_miLightFlag);
	CopyInt(tbuff, &mis->_miPreFlag);
	CopyInt(tbuff, &mis->_miUniqTrans);
	CopyInt(tbuff, &mis->_mirange);
	CopyInt(tbuff, &mis->_misource);
	CopyInt(tbuff, &mis->_micaster);
	CopyInt(tbuff, &mis->_midam);
	CopyInt(tbuff, &mis->_miHitFlag);
	CopyInt(tbuff, &mis->_midist);
	CopyInt(tbuff, &mis->_mlid);
	CopyInt(tbuff, &mis->_mirnd);
	CopyInt(tbuff, &mis->_miVar1);
	CopyInt(tbuff, &mis->_miVar2);
	CopyInt(tbuff, &mis->_miVar3);
	CopyInt(tbuff, &mis->_miVar4);
	CopyInt(tbuff, &mis->_miVar5);
	CopyInt(tbuff, &mis->_miVar6);
	CopyInt(tbuff, &mis->_miVar7);
	CopyInt(tbuff, &mis->_miVar8);
}

void LoadObject(int oi)
{
	ObjectStruct *os = &object[oi];

	CopyInt(tbuff, &os->_otype);
	CopyInt(tbuff, &os->_ox);
	CopyInt(tbuff, &os->_oy);
	CopyInt(tbuff, &os->_oLight);
	CopyInt(tbuff, &os->_oAnimFlag);
	tbuff += 4; // Skip pointer _oAnimData
	CopyInt(tbuff, &os->_oAnimDelay);
	CopyInt(tbuff, &os->_oAnimCnt);
	CopyInt(tbuff, &os->_oAnimLen);
	CopyInt(tbuff, &os->_oAnimFrame);
	CopyInt(tbuff, &os->_oAnimWidth);
	CopyInt(tbuff, &os->_oAnimWidth2);
	CopyInt(tbuff, &os->_oDelFlag);
	CopyChar(tbuff, &os->_oBreak);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &os->_oSolidFlag);
	CopyInt(tbuff, &os->_oMissFlag);

	CopyChar(tbuff, &os->_oSelFlag);
	tbuff += 3; // Alignment
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

void LoadItem(int ii)
{
	LoadItemData(&item[ii]);
	GetItemFrm(ii);
}

void LoadItemData(ItemStruct *is)
{
	CopyInt(tbuff, &is->_iSeed);
	CopyShort(tbuff, &is->_iCreateInfo);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &is->_itype);
	CopyInt(tbuff, &is->_ix);
	CopyInt(tbuff, &is->_iy);
	CopyInt(tbuff, &is->_iAnimFlag);
	tbuff += 4; // Skip pointer _iAnimData
	CopyInt(tbuff, &is->_iAnimLen);
	CopyInt(tbuff, &is->_iAnimFrame);
	CopyInt(tbuff, &is->_iAnimWidth);
	CopyInt(tbuff, &is->_iAnimWidth2);
	CopyInt(tbuff, &is->_iDelFlag);
	CopyChar(tbuff, &is->_iSelFlag);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &is->_iPostDraw);
	CopyInt(tbuff, &is->_iIdentified);
	CopyChar(tbuff, &is->_iMagical);
	CopyBytes(tbuff, 64, &is->_iName);
	CopyBytes(tbuff, 64, &is->_iIName);
	CopyChar(tbuff, &is->_iLoc);
	CopyChar(tbuff, &is->_iClass);
	tbuff += 1; // Alignment
	CopyInt(tbuff, &is->_iCurs);
	CopyInt(tbuff, &is->_ivalue);
	CopyInt(tbuff, &is->_iIvalue);
	CopyInt(tbuff, &is->_iMinDam);
	CopyInt(tbuff, &is->_iMaxDam);
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
	CopyInt(tbuff, &is->_iPLMana);
	CopyInt(tbuff, &is->_iPLHP);
	CopyInt(tbuff, &is->_iPLDamMod);
	CopyInt(tbuff, &is->_iPLGetHit);
	CopyInt(tbuff, &is->_iPLLight);
	CopyChar(tbuff, &is->_iSplLvlAdd);
	CopyChar(tbuff, &is->_iRequest);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &is->_iUid);
	CopyInt(tbuff, &is->_iFMinDam);
	CopyInt(tbuff, &is->_iFMaxDam);
	CopyInt(tbuff, &is->_iLMinDam);
	CopyInt(tbuff, &is->_iLMaxDam);
	CopyInt(tbuff, &is->_iPLEnAc);
	CopyChar(tbuff, &is->_iPrePower);
	CopyChar(tbuff, &is->_iSufPower);
	tbuff += 2; // Alignment
	CopyInt(tbuff, &is->_iVAdd1);
	CopyInt(tbuff, &is->_iVMult1);
	CopyInt(tbuff, &is->_iVAdd2);
	CopyInt(tbuff, &is->_iVMult2);
	CopyChar(tbuff, &is->_iMinStr);
	CopyChar(tbuff, &is->_iMinMag);
	CopyChar(tbuff, &is->_iMinDex);
	tbuff += 1; // Alignment
	CopyInt(tbuff, &is->_iStatFlag);
	CopyInt(tbuff, &is->IDidx);
	CopyInt(tbuff, &is->offs016C);
}

void LoadItems(const int n, ItemStruct *pItem)
{
	for (int i = 0; i < n; i++) {
		LoadItemData(&pItem[i]);
	}
}

void LoadPremium(int i)
{
	LoadItemData(&premiumitem[i]);
}

void LoadQuest(int i)
{
	QuestStruct *pQuest = &quests[i];

	CopyChar(tbuff, &pQuest->_qlevel);
	CopyChar(tbuff, &pQuest->_qtype);
	CopyChar(tbuff, &pQuest->_qactive);
	CopyChar(tbuff, &pQuest->_qlvltype);
	CopyInt(tbuff, &pQuest->_qtx);
	CopyInt(tbuff, &pQuest->_qty);
	CopyChar(tbuff, &pQuest->_qslvl);
	CopyChar(tbuff, &pQuest->_qidx);
	CopyChar(tbuff, &pQuest->_qmsg);
	CopyChar(tbuff, &pQuest->_qvar1);
	CopyChar(tbuff, &pQuest->_qvar2);
	tbuff += 3; // Alignment
	CopyInt(tbuff, &pQuest->_qlog);

	ReturnLvlX = WLoad();
	ReturnLvlY = WLoad();
	ReturnLvl = WLoad();
	ReturnLvlT = WLoad();
	DoomQuestState = WLoad();
}

void LoadLighting(int lnum)
{
	LightListStruct *pLight = &LightList[lnum];

	CopyInt(tbuff, &pLight->_lx);
	CopyInt(tbuff, &pLight->_ly);
	CopyInt(tbuff, &pLight->_lradius);
	CopyInt(tbuff, &pLight->_lid);
	CopyInt(tbuff, &pLight->_ldel);
	CopyInt(tbuff, &pLight->_lunflag);
	CopyInt(tbuff, &pLight->field_18);
	CopyInt(tbuff, &pLight->_lunx);
	CopyInt(tbuff, &pLight->_luny);
	CopyInt(tbuff, &pLight->_lunr);
	CopyInt(tbuff, &pLight->_xoff);
	CopyInt(tbuff, &pLight->_yoff);
	CopyInt(tbuff, &pLight->_lflags);
}

void LoadVision(int vnum)
{
	LightListStruct *pVision = &VisionList[vnum];

	CopyInt(tbuff, &pVision->_lx);
	CopyInt(tbuff, &pVision->_ly);
	CopyInt(tbuff, &pVision->_lradius);
	CopyInt(tbuff, &pVision->_lid);
	CopyInt(tbuff, &pVision->_ldel);
	CopyInt(tbuff, &pVision->_lunflag);
	CopyInt(tbuff, &pVision->field_18);
	CopyInt(tbuff, &pVision->_lunx);
	CopyInt(tbuff, &pVision->_luny);
	CopyInt(tbuff, &pVision->_lunr);
	CopyInt(tbuff, &pVision->_xoff);
	CopyInt(tbuff, &pVision->_yoff);
	CopyInt(tbuff, &pVision->_lflags);
}

void LoadPortal(int i)
{
	PortalStruct *pPortal = &portal[i];

	CopyInt(tbuff, &pPortal->open);
	CopyInt(tbuff, &pPortal->x);
	CopyInt(tbuff, &pPortal->y);
	CopyInt(tbuff, &pPortal->level);
	CopyInt(tbuff, &pPortal->ltype);
	CopyInt(tbuff, &pPortal->setlvl);
}

void SaveGame()
{
	int i, j;
	char szName[MAX_PATH];

	DWORD dwLen = codec_get_encoded_len(FILEBUFF);
	BYTE *SaveBuff = DiabloAllocPtr(dwLen);
	tbuff = SaveBuff;

	ISave(SAVE_INITIAL);

	OSave(setlevel);
	WSave(setlvlnum);
	WSave(currlevel);
	WSave(leveltype);
	WSave(ViewX);
	WSave(ViewY);
	OSave(invflag);
	OSave(chrflag);
	WSave(nummonsters);
	WSave(numitems);
	WSave(nummissiles);
	WSave(nobjects);

	for (i = 0; i < NUMLEVELS; i++) {
		ISave(glSeedTbl[i]);
		WSave(gnLevelTypeTbl[i]);
	}

	plr[myplr].pDifficulty = gnDifficulty;
	SavePlayer(myplr);

	for (i = 0; i < MAXQUESTS; i++)
		SaveQuest(i);
	for (i = 0; i < MAXPORTAL; i++)
		SavePortal(i);
	for (i = 0; i < MAXMONSTERS; i++)
		ISave(monstkills[i]);

	if (leveltype != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			WSave(monstactive[i]);
		for (i = 0; i < nummonsters; i++)
			SaveMonster(monstactive[i]);
		for (i = 0; i < MAXMISSILES; i++)
			BSave(missileactive[i]);
		for (i = 0; i < MAXMISSILES; i++)
			BSave(missileavail[i]);
		for (i = 0; i < nummissiles; i++)
			SaveMissile(missileactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			BSave(objectactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			BSave(objectavail[i]);
		for (i = 0; i < nobjects; i++)
			SaveObject(objectactive[i]);

		WSave(numlights);

		for (i = 0; i < MAXLIGHTS; i++)
			BSave(lightactive[i]);
		for (i = 0; i < numlights; i++)
			SaveLighting(lightactive[i]);

		WSave(visionid);
		WSave(numvision);

		for (i = 0; i < numvision; i++)
			SaveVision(i);
	}

	for (i = 0; i < MAXITEMS; i++)
		BSave(itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		BSave(itemavail[i]);
	for (i = 0; i < numitems; i++)
		SaveItem(&item[itemactive[i]]);
	for (i = 0; i < 128; i++)
		OSave(UniqueItemFlag[i]);

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			BSave(dLight[i][j]);
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			BSave(dFlags[i][j] & ~(BFLAG_MISSILE | BFLAG_VISIBLE | BFLAG_DEAD_PLAYER));
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			BSave(dPlayer[i][j]);
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			BSave(dItem[i][j]);
	}

	if (leveltype != DTYPE_TOWN) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				WSave(dMonster[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dDead[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dObject[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dLight[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dPreLight[i][j]);
		}
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++)
				OSave(automapview[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dMissile[i][j]);
		}
	}

	WSave(numpremium);
	WSave(premiumlevel);

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		SavePremium(i);

	OSave(automapflag);
	WSave(AutoMapScale);
	pfile_get_game_name(szName);
	dwLen = codec_get_encoded_len(tbuff - SaveBuff);
	pfile_write_save_file(szName, SaveBuff, tbuff - SaveBuff, dwLen);
	mem_free_dbg(SaveBuff);
	gbValidSaveFile = TRUE;
	pfile_rename_temp_to_perm();
	pfile_write_hero();
}

void BSave(char v)
{
	*tbuff++ = v;
}

void WSave(int v)
{
	*tbuff++ = v >> 24;
	*tbuff++ = v >> 16;
	*tbuff++ = v >> 8;
	*tbuff++ = v;
}

void ISave(int v)
{
	*tbuff++ = v >> 24;
	*tbuff++ = v >> 16;
	*tbuff++ = v >> 8;
	*tbuff++ = v;
}

void OSave(BOOL v)
{
	if (v)
		*tbuff++ = TRUE;
	else
		*tbuff++ = FALSE;
}

void SavePlayer(int pnum)
{
	PlayerStruct *p = &plr[pnum];

	CopyInt(&p->_pmode, tbuff);
	CopyBytes(&p->walkpath, MAX_PATH_LENGTH, tbuff);
	CopyBytes(&p->plractive, 1, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&p->destAction, tbuff);
	CopyInt(&p->destParam1, tbuff);
	CopyInt(&p->destParam2, tbuff);
	CopyInt(&p->destParam3, tbuff);
	CopyInt(&p->destParam4, tbuff);
	CopyInt(&p->plrlevel, tbuff);
	CopyInt(&p->_px, tbuff);
	CopyInt(&p->_py, tbuff);
	CopyInt(&p->_pfutx, tbuff);
	CopyInt(&p->_pfuty, tbuff);
	CopyInt(&p->_ptargx, tbuff);
	CopyInt(&p->_ptargy, tbuff);
	CopyInt(&p->_pownerx, tbuff);
	CopyInt(&p->_pownery, tbuff);
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
	CopyInt(&p->_pAnimDelay, tbuff);
	CopyInt(&p->_pAnimCnt, tbuff);
	CopyInt(&p->_pAnimLen, tbuff);
	CopyInt(&p->_pAnimFrame, tbuff);
	CopyInt(&p->_pAnimWidth, tbuff);
	CopyInt(&p->_pAnimWidth2, tbuff);
	tbuff += 4; // Skip _peflag
	CopyInt(&p->_plid, tbuff);
	CopyInt(&p->_pvid, tbuff);

	CopyInt(&p->_pSpell, tbuff);
	CopyChar(&p->_pSplType, tbuff);
	CopyChar(&p->_pSplFrom, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&p->_pTSpell, tbuff);
	CopyChar(&p->_pTSplType, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&p->_pRSpell, tbuff);
	CopyChar(&p->_pRSplType, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&p->_pSBkSpell, tbuff);
	CopyChar(&p->_pSBkSplType, tbuff);
	CopyBytes(&p->_pSplLvl, 64, tbuff);
	tbuff += 7; // Alignment
	CopyInt64(&p->_pMemSpells, tbuff);
	CopyInt64(&p->_pAblSpells, tbuff);
	CopyInt64(&p->_pScrlSpells, tbuff);
	CopyChar(&p->_pSpellFlags, tbuff);
	tbuff += 3; // Alignment
	CopyInts(&p->_pSplHotKey, 4, tbuff);
	CopyBytes(&p->_pSplTHotKey, 4, tbuff);

	CopyInt(&p->_pwtype, tbuff);
	CopyChar(&p->_pBlockFlag, tbuff);
	CopyChar(&p->_pInvincible, tbuff);
	CopyChar(&p->_pLightRad, tbuff);
	CopyChar(&p->_pLvlChanging, tbuff);

	CopyBytes(&p->_pName, PLR_NAME_LEN, tbuff);
	CopyChar(&p->_pClass, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&p->_pStrength, tbuff);
	CopyInt(&p->_pBaseStr, tbuff);
	CopyInt(&p->_pMagic, tbuff);
	CopyInt(&p->_pBaseMag, tbuff);
	CopyInt(&p->_pDexterity, tbuff);
	CopyInt(&p->_pBaseDex, tbuff);
	CopyInt(&p->_pVitality, tbuff);
	CopyInt(&p->_pBaseVit, tbuff);
	CopyInt(&p->_pStatPts, tbuff);
	CopyInt(&p->_pDamageMod, tbuff);
	CopyInt(&p->_pBaseToBlk, tbuff);
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
	CopyChar(&p->_pLevel, tbuff);
	CopyChar(&p->_pMaxLvl, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&p->_pExperience, tbuff);
	CopyInt(&p->_pMaxExp, tbuff);
	CopyInt(&p->_pNextExper, tbuff);
	CopyChar(&p->_pArmorClass, tbuff);
	CopyChar(&p->_pMagResist, tbuff);
	CopyChar(&p->_pFireResist, tbuff);
	CopyChar(&p->_pLghtResist, tbuff);
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
	CopyBytes(&p->_pLvlVisited, NUMLEVELS, tbuff);
	CopyBytes(&p->_pSLvlVisited, NUMLEVELS, tbuff); // only 10 used
	tbuff += 2;                                     // Alignment

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
	SaveItems(p->InvList, NUM_INV_GRID_ELEM);
	CopyInt(&p->_pNumInv, tbuff);
	CopyBytes(p->InvGrid, NUM_INV_GRID_ELEM, tbuff);
	SaveItems(p->SpdList, MAXBELTITEMS);
	SaveItem(&p->HoldItem);

	CopyInt(&p->_pIMinDam, tbuff);
	CopyInt(&p->_pIMaxDam, tbuff);
	CopyInt(&p->_pIAC, tbuff);
	CopyInt(&p->_pIBonusDam, tbuff);
	CopyInt(&p->_pIBonusToHit, tbuff);
	CopyInt(&p->_pIBonusAC, tbuff);
	CopyInt(&p->_pIBonusDamMod, tbuff);
	tbuff += 4; // Alignment

	CopyInt64(&p->_pISpells, tbuff);
	CopyInt(&p->_pIFlags, tbuff);
	CopyInt(&p->_pIGetHit, tbuff);

	CopyChar(&p->_pISplLvlAdd, tbuff);
	CopyChar(&p->_pISplCost, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&p->_pISplDur, tbuff);
	CopyInt(&p->_pIEnAc, tbuff);
	CopyInt(&p->_pIFMinDam, tbuff);
	CopyInt(&p->_pIFMaxDam, tbuff);
	CopyInt(&p->_pILMinDam, tbuff);
	CopyInt(&p->_pILMaxDam, tbuff);
	CopyInt(&p->_pOilType, tbuff);
	CopyChar(&p->pTownWarps, tbuff);
	CopyChar(&p->pDungMsgs, tbuff);
	CopyChar(&p->pLvlLoad, tbuff);
#ifdef HELLFIRE
	CopyChar(&p->pDungMsgs2, tbuff);
#else
	CopyChar(&p->pBattleNet, tbuff);
#endif
	CopyChar(&p->pManaShield, tbuff);
	CopyChar(&p->pDungMsgs2, tbuff);
	CopyBytes(&p->bReserved, 2, tbuff);
	CopyShort(&p->wReflection, tbuff);
	CopyShorts(&p->wReserved, 7, tbuff);

	CopyInt(&p->pDiabloKillLevel, tbuff);
	CopyInt(&p->pDifficulty, tbuff);
	CopyInt(&p->pDamAcFlags, tbuff);
	CopyInts(&p->dwReserved, 5, tbuff);

	// Omit pointer _pNData
	// Omit pointer _pWData
	// Omit pointer _pAData
	// Omit pointer _pLData
	// Omit pointer _pFData
	// Omit pointer  _pTData
	// Omit pointer _pHData
	// Omit pointer _pDData
	// Omit pointer _pBData
	// Omit pointer pReserved
}

void SaveMonster(int mnum)
{
	MonsterStruct *mon = &monster[mnum];
	char tempChar;

	CopyInt(&mon->_mMTidx, tbuff);
	CopyInt(&mon->_mmode, tbuff);
	CopyChar(&mon->_mgoal, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&mon->_mgoalvar1, tbuff);
	CopyInt(&mon->_mgoalvar2, tbuff);
	CopyInt(&mon->_mgoalvar3, tbuff);
	CopyInt(&mon->field_18, tbuff);
	CopyChar(&mon->_pathcount, tbuff);
	tbuff += 3; // Alignment
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
	CopyShort(&mon->falign_52, tbuff);

	tbuff += 4; // Skip pointer _mAnimData
	CopyInt(&mon->_mAnimDelay, tbuff);
	CopyInt(&mon->_mAnimCnt, tbuff);
	CopyInt(&mon->_mAnimLen, tbuff);
	CopyInt(&mon->_mAnimFrame, tbuff);
	tbuff += 4; // Skip _meflag
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

	CopyChar(&mon->_mAi, tbuff);
	CopyChar(&mon->_mint, tbuff);
	CopyShort(&mon->falign_9A, tbuff);
	CopyInt(&mon->_mFlags, tbuff);
	CopyChar(&mon->_msquelch, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&mon->falign_A4, tbuff);
	CopyInt(&mon->_lastx, tbuff);
	CopyInt(&mon->_lasty, tbuff);
	CopyInt(&mon->_mRndSeed, tbuff);
	CopyInt(&mon->_mAISeed, tbuff);
	CopyInt(&mon->falign_B8, tbuff);

	CopyChar(&mon->_uniqtype, tbuff);
	CopyChar(&mon->_uniqtrans, tbuff);
	CopyChar(&mon->_udeadval, tbuff);

	CopyChar(&mon->mWhoHit, tbuff);
	CopyChar(&mon->mLevel, tbuff);
	tbuff += 1; // Alignment
	CopyShort(&mon->mExp, tbuff);

	// Wtite mHit for backwards compatabiliyt
	tempChar = mon->mHit < SCHAR_MAX ? mon->mHit : SCHAR_MAX;
	CopyChar(&tempChar, tbuff);
	CopyChar(&mon->mMinDamage, tbuff);
	CopyChar(&mon->mMaxDamage, tbuff);
	// Wtite mHit2 for backwards compatabiliyt
	tempChar = mon->mHit2 < SCHAR_MAX ? mon->mHit2 : SCHAR_MAX;
	CopyChar(&tempChar, tbuff);
	CopyChar(&mon->mMinDamage2, tbuff);
	CopyChar(&mon->mMaxDamage2, tbuff);
	CopyChar(&mon->mArmorClass, tbuff);
	CopyChar(&mon->falign_CB, tbuff);
	CopyShort(&mon->mMagicRes, tbuff);
	tbuff += 2; // Alignment

	CopyInt(&mon->mtalkmsg, tbuff);
	CopyChar(&mon->leader, tbuff);
	CopyChar(&mon->leaderflag, tbuff);
	CopyChar(&mon->packsize, tbuff);
	CopyChar(&mon->mlid, tbuff);

	// Omit pointer mName;
	// Omit pointer MType;
	// Omit pointer MData;
}

void SaveMissile(int mi)
{
	MissileStruct *mis = &missile[mi];

	CopyInt(&mis->_mitype, tbuff);
	CopyInt(&mis->_mix, tbuff);
	CopyInt(&mis->_miy, tbuff);
	CopyInt(&mis->_mixoff, tbuff);
	CopyInt(&mis->_miyoff, tbuff);
	CopyInt(&mis->_mixvel, tbuff);
	CopyInt(&mis->_miyvel, tbuff);
	CopyInt(&mis->_misx, tbuff);
	CopyInt(&mis->_misy, tbuff);
	CopyInt(&mis->_mitxoff, tbuff);
	CopyInt(&mis->_mityoff, tbuff);
	CopyInt(&mis->_mimfnum, tbuff);
	CopyInt(&mis->_mispllvl, tbuff);
	CopyInt(&mis->_miDelFlag, tbuff);
	CopyChar(&mis->_miAnimType, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&mis->_miAnimFlags, tbuff);
	tbuff += 4; // Skip pointer _miAnimData
	CopyInt(&mis->_miAnimDelay, tbuff);
	CopyInt(&mis->_miAnimLen, tbuff);
	CopyInt(&mis->_miAnimWidth, tbuff);
	CopyInt(&mis->_miAnimWidth2, tbuff);
	CopyInt(&mis->_miAnimCnt, tbuff);
	CopyInt(&mis->_miAnimAdd, tbuff);
	CopyInt(&mis->_miAnimFrame, tbuff);
	CopyInt(&mis->_miDrawFlag, tbuff);
	CopyInt(&mis->_miLightFlag, tbuff);
	CopyInt(&mis->_miPreFlag, tbuff);
	CopyInt(&mis->_miUniqTrans, tbuff);
	CopyInt(&mis->_mirange, tbuff);
	CopyInt(&mis->_misource, tbuff);
	CopyInt(&mis->_micaster, tbuff);
	CopyInt(&mis->_midam, tbuff);
	CopyInt(&mis->_miHitFlag, tbuff);
	CopyInt(&mis->_midist, tbuff);
	CopyInt(&mis->_mlid, tbuff);
	CopyInt(&mis->_mirnd, tbuff);
	CopyInt(&mis->_miVar1, tbuff);
	CopyInt(&mis->_miVar2, tbuff);
	CopyInt(&mis->_miVar3, tbuff);
	CopyInt(&mis->_miVar4, tbuff);
	CopyInt(&mis->_miVar5, tbuff);
	CopyInt(&mis->_miVar6, tbuff);
	CopyInt(&mis->_miVar7, tbuff);
	CopyInt(&mis->_miVar8, tbuff);
}

void SaveObject(int oi)
{
	ObjectStruct *os = &object[oi];

	CopyInt(&os->_otype, tbuff);
	CopyInt(&os->_ox, tbuff);
	CopyInt(&os->_oy, tbuff);
	CopyInt(&os->_oLight, tbuff);
	CopyInt(&os->_oAnimFlag, tbuff);
	tbuff += 4; // Skip pointer _oAnimData
	CopyInt(&os->_oAnimDelay, tbuff);
	CopyInt(&os->_oAnimCnt, tbuff);
	CopyInt(&os->_oAnimLen, tbuff);
	CopyInt(&os->_oAnimFrame, tbuff);
	CopyInt(&os->_oAnimWidth, tbuff);
	CopyInt(&os->_oAnimWidth2, tbuff);
	CopyInt(&os->_oDelFlag, tbuff);
	CopyChar(&os->_oBreak, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&os->_oSolidFlag, tbuff);
	CopyInt(&os->_oMissFlag, tbuff);

	CopyChar(&os->_oSelFlag, tbuff);
	tbuff += 3; // Alignment
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

void SaveItem(ItemStruct *is)
{
	CopyInt(&is->_iSeed, tbuff);
	CopyShort(&is->_iCreateInfo, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&is->_itype, tbuff);
	CopyInt(&is->_ix, tbuff);
	CopyInt(&is->_iy, tbuff);
	CopyInt(&is->_iAnimFlag, tbuff);
	tbuff += 4; // Skip pointer _iAnimData
	CopyInt(&is->_iAnimLen, tbuff);
	CopyInt(&is->_iAnimFrame, tbuff);
	CopyInt(&is->_iAnimWidth, tbuff);
	CopyInt(&is->_iAnimWidth2, tbuff);
	CopyInt(&is->_iDelFlag, tbuff);
	CopyChar(&is->_iSelFlag, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&is->_iPostDraw, tbuff);
	CopyInt(&is->_iIdentified, tbuff);
	CopyChar(&is->_iMagical, tbuff);
	CopyBytes(&is->_iName, 64, tbuff);
	CopyBytes(&is->_iIName, 64, tbuff);
	CopyChar(&is->_iLoc, tbuff);
	CopyChar(&is->_iClass, tbuff);
	tbuff += 1; // Alignment
	CopyInt(&is->_iCurs, tbuff);
	CopyInt(&is->_ivalue, tbuff);
	CopyInt(&is->_iIvalue, tbuff);
	CopyInt(&is->_iMinDam, tbuff);
	CopyInt(&is->_iMaxDam, tbuff);
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
	CopyInt(&is->_iPLMana, tbuff);
	CopyInt(&is->_iPLHP, tbuff);
	CopyInt(&is->_iPLDamMod, tbuff);
	CopyInt(&is->_iPLGetHit, tbuff);
	CopyInt(&is->_iPLLight, tbuff);
	CopyChar(&is->_iSplLvlAdd, tbuff);
	CopyChar(&is->_iRequest, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&is->_iUid, tbuff);
	CopyInt(&is->_iFMinDam, tbuff);
	CopyInt(&is->_iFMaxDam, tbuff);
	CopyInt(&is->_iLMinDam, tbuff);
	CopyInt(&is->_iLMaxDam, tbuff);
	CopyInt(&is->_iPLEnAc, tbuff);
	CopyChar(&is->_iPrePower, tbuff);
	CopyChar(&is->_iSufPower, tbuff);
	tbuff += 2; // Alignment
	CopyInt(&is->_iVAdd1, tbuff);
	CopyInt(&is->_iVMult1, tbuff);
	CopyInt(&is->_iVAdd2, tbuff);
	CopyInt(&is->_iVMult2, tbuff);
	CopyChar(&is->_iMinStr, tbuff);
	CopyChar(&is->_iMinMag, tbuff);
	CopyChar(&is->_iMinDex, tbuff);
	tbuff += 1; // Alignment
	CopyInt(&is->_iStatFlag, tbuff);
	CopyInt(&is->IDidx, tbuff);
	CopyInt(&is->offs016C, tbuff);
}

void SaveItems(ItemStruct *pItem, const int n)
{
	for (int i = 0; i < n; i++) {
		SaveItem(&pItem[i]);
	}
}

void SavePremium(int i)
{
	SaveItem(&premiumitem[i]);
}

void SaveQuest(int i)
{
	QuestStruct *pQuest = &quests[i];

	CopyChar(&pQuest->_qlevel, tbuff);
	CopyChar(&pQuest->_qtype, tbuff);
	CopyChar(&pQuest->_qactive, tbuff);
	CopyChar(&pQuest->_qlvltype, tbuff);
	CopyInt(&pQuest->_qtx, tbuff);
	CopyInt(&pQuest->_qty, tbuff);
	CopyChar(&pQuest->_qslvl, tbuff);
	CopyChar(&pQuest->_qidx, tbuff);
	CopyChar(&pQuest->_qmsg, tbuff);
	CopyChar(&pQuest->_qvar1, tbuff);
	CopyChar(&pQuest->_qvar2, tbuff);
	tbuff += 3; // Alignment
	CopyInt(&pQuest->_qlog, tbuff);

	WSave(ReturnLvlX);
	WSave(ReturnLvlY);
	WSave(ReturnLvl);
	WSave(ReturnLvlT);
	WSave(DoomQuestState);
}

void SaveLighting(int lnum)
{
	LightListStruct *pLight = &LightList[lnum];

	CopyInt(&pLight->_lx, tbuff);
	CopyInt(&pLight->_ly, tbuff);
	CopyInt(&pLight->_lradius, tbuff);
	CopyInt(&pLight->_lid, tbuff);
	CopyInt(&pLight->_ldel, tbuff);
	CopyInt(&pLight->_lunflag, tbuff);
	CopyInt(&pLight->field_18, tbuff);
	CopyInt(&pLight->_lunx, tbuff);
	CopyInt(&pLight->_luny, tbuff);
	CopyInt(&pLight->_lunr, tbuff);
	CopyInt(&pLight->_xoff, tbuff);
	CopyInt(&pLight->_yoff, tbuff);
	CopyInt(&pLight->_lflags, tbuff);
}

void SaveVision(int vnum)
{
	LightListStruct *pVision = &VisionList[vnum];

	CopyInt(&pVision->_lx, tbuff);
	CopyInt(&pVision->_ly, tbuff);
	CopyInt(&pVision->_lradius, tbuff);
	CopyInt(&pVision->_lid, tbuff);
	CopyInt(&pVision->_ldel, tbuff);
	CopyInt(&pVision->_lunflag, tbuff);
	CopyInt(&pVision->field_18, tbuff);
	CopyInt(&pVision->_lunx, tbuff);
	CopyInt(&pVision->_luny, tbuff);
	CopyInt(&pVision->_lunr, tbuff);
	CopyInt(&pVision->_xoff, tbuff);
	CopyInt(&pVision->_yoff, tbuff);
	CopyInt(&pVision->_lflags, tbuff);
}

void SavePortal(int i)
{
	PortalStruct *pPortal = &portal[i];

	CopyInt(&pPortal->open, tbuff);
	CopyInt(&pPortal->x, tbuff);
	CopyInt(&pPortal->y, tbuff);
	CopyInt(&pPortal->level, tbuff);
	CopyInt(&pPortal->ltype, tbuff);
	CopyInt(&pPortal->setlvl, tbuff);
}

void SaveLevel()
{
	int i, j;
	char szName[MAX_PATH];
	int dwLen;
	BYTE *SaveBuff;

	if (currlevel == 0)
		glSeedTbl[0] = GetRndSeed();

	dwLen = codec_get_encoded_len(FILEBUFF);
	SaveBuff = DiabloAllocPtr(dwLen);
	tbuff = SaveBuff;

	if (leveltype != DTYPE_TOWN) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dDead[i][j]);
		}
	}

	WSave(nummonsters);
	WSave(numitems);
	WSave(nobjects);

	if (leveltype != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			WSave(monstactive[i]);
		for (i = 0; i < nummonsters; i++)
			SaveMonster(monstactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			BSave(objectactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			BSave(objectavail[i]);
		for (i = 0; i < nobjects; i++)
			SaveObject(objectactive[i]);
	}

	for (i = 0; i < MAXITEMS; i++)
		BSave(itemactive[i]);
	for (i = 0; i < MAXITEMS; i++)
		BSave(itemavail[i]);
	for (i = 0; i < numitems; i++)
		SaveItem(&item[itemactive[i]]);

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			BSave(dFlags[i][j] & ~(BFLAG_MISSILE | BFLAG_VISIBLE | BFLAG_DEAD_PLAYER));
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			BSave(dItem[i][j]);
	}

	if (leveltype != DTYPE_TOWN) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				WSave(dMonster[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dObject[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dLight[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dPreLight[i][j]);
		}
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++)
				OSave(automapview[i][j]);
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				BSave(dMissile[i][j]);
		}
	}

	GetTempLevelNames(szName);
	dwLen = codec_get_encoded_len(tbuff - SaveBuff);
	pfile_write_save_file(szName, SaveBuff, tbuff - SaveBuff, dwLen);
	mem_free_dbg(SaveBuff);

	if (!setlevel)
		plr[myplr]._pLvlVisited[currlevel] = TRUE;
	else
		plr[myplr]._pSLvlVisited[setlvlnum] = TRUE;
}

void LoadLevel()
{
	int i, j;
	DWORD dwLen;
	char szName[MAX_PATH];
	BYTE *LoadBuff;

	GetPermLevelNames(szName);
	LoadBuff = pfile_read(szName, &dwLen);
	tbuff = LoadBuff;

	if (leveltype != DTYPE_TOWN) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dDead[i][j] = BLoad();
		}
		SetDead();
	}

	nummonsters = WLoad();
	numitems = WLoad();
	nobjects = WLoad();

	if (leveltype != DTYPE_TOWN) {
		for (i = 0; i < MAXMONSTERS; i++)
			monstactive[i] = WLoad();
		for (i = 0; i < nummonsters; i++)
			LoadMonster(monstactive[i]);
		for (i = 0; i < MAXOBJECTS; i++)
			objectactive[i] = BLoad();
		for (i = 0; i < MAXOBJECTS; i++)
			objectavail[i] = BLoad();
		for (i = 0; i < nobjects; i++)
			LoadObject(objectactive[i]);
		for (i = 0; i < nobjects; i++)
			SyncObjectAnim(objectactive[i]);
	}

	for (i = 0; i < MAXITEMS; i++)
		itemactive[i] = BLoad();
	for (i = 0; i < MAXITEMS; i++)
		itemavail[i] = BLoad();
	for (i = 0; i < numitems; i++)
		LoadItem(itemactive[i]);

	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			dFlags[i][j] = BLoad();
	}
	for (j = 0; j < MAXDUNY; j++) {
		for (i = 0; i < MAXDUNX; i++)
			dItem[i][j] = BLoad();
	}

	if (leveltype != DTYPE_TOWN) {
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dMonster[i][j] = WLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dObject[i][j] = BLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dLight[i][j] = BLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dPreLight[i][j] = BLoad();
		}
		for (j = 0; j < DMAXY; j++) {
			for (i = 0; i < DMAXX; i++)
				automapview[i][j] = OLoad();
		}
		for (j = 0; j < MAXDUNY; j++) {
			for (i = 0; i < MAXDUNX; i++)
				dMissile[i][j] = 0; /// BUGFIX: supposed to load saved missiles with "BLoad()"?
		}
	}

	AutomapZoomReset();
	ResyncQuests();
	SyncPortals();

	dolighting = TRUE;
	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && currlevel == plr[i].plrlevel)
			LightList[plr[i]._plid]._lunflag = TRUE;
	}

	mem_free_dbg(LoadBuff);
}

DEVILUTION_END_NAMESPACE
