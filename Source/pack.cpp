/**
 * @file pack.cpp
 *
 * Implementation of functions for minifying player data structure.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

void PackItem(PkItemStruct *pis, ItemStruct *is)
{
	if (is->_itype == ITYPE_NONE) {
		pis->idx = 0xFFFF;
	} else {
		pis->idx = SwapLE16(is->_iIdx);
		if (is->_iIdx == IDI_EAR) {
			pis->iCreateInfo = is->_iName[8] | (is->_iName[7] << 8);
			pis->iSeed = SwapLE32(is->_iName[12] | ((is->_iName[11] | ((is->_iName[10] | (is->_iName[9] << 8)) << 8)) << 8));
			pis->bId = is->_iName[13];
			pis->bDur = is->_iName[14];
			pis->bMDur = is->_iName[15];
			pis->bCh = is->_iName[16];
			pis->bMCh = is->_iName[17];
			pis->wValue = SwapLE16(is->_ivalue | (is->_iName[18] << 8) | ((is->_iCurs - ICURS_EAR_SORCEROR) << 6));
			pis->dwBuff = SwapLE32(is->_iName[22] | ((is->_iName[21] | ((is->_iName[20] | (is->_iName[19] << 8)) << 8)) << 8));
		} else {
			pis->iSeed = SwapLE32(is->_iSeed);
			pis->iCreateInfo = SwapLE16(is->_iCreateInfo);
			pis->bId = is->_iIdentified + 2 * is->_iMagical;
			pis->bDur = is->_iDurability;
			pis->bMDur = is->_iMaxDur;
			pis->bCh = is->_iCharges;
			pis->bMCh = is->_iMaxCharges;
			if (is->_iIdx == IDI_GOLD)
				pis->wValue = SwapLE16(is->_ivalue);
		}
	}
}

void PackPlayer(PkPlayerStruct *pPack, int pnum)
{
	PlayerStruct *p;
	int i;
	ItemStruct *pi;
	PkItemStruct *pki;

	memset(pPack, 0, sizeof(*pPack));
	p = &plr[pnum];
	pPack->destAction = p->destAction;
	pPack->destParam1 = p->destParam1;
	pPack->destParam2 = p->destParam2;
	pPack->plrlevel = p->plrlevel;
	pPack->px = p->_px;
	pPack->py = p->_py;
	pPack->targx = p->_ptargx;
	pPack->targy = p->_ptargy;
	copy_str(pPack->pName, p->_pName);
	pPack->pClass = p->_pClass;
	pPack->pBaseStr = p->_pBaseStr;
	pPack->pBaseMag = p->_pBaseMag;
	pPack->pBaseDex = p->_pBaseDex;
	pPack->pBaseVit = p->_pBaseVit;
	pPack->pLevel = p->_pLevel;
	pPack->pStatPts = p->_pStatPts;
	pPack->pExperience = SwapLE32(p->_pExperience);
	pPack->pGold = SwapLE32(p->_pGold);
	pPack->pHPBase = SwapLE32(p->_pHPBase);
	pPack->pMaxHPBase = SwapLE32(p->_pMaxHPBase);
	pPack->pManaBase = SwapLE32(p->_pManaBase);
	pPack->pMaxManaBase = SwapLE32(p->_pMaxManaBase);
	pPack->pMemSpells = SDL_SwapLE64(p->_pMemSpells);

	for (i = 0; i < 37; i++) // Should be NUM_SPELLS but set to 37 to make save games compatible
		pPack->pSplLvl[i] = p->_pSplLvl[i];
#ifdef HELLFIRE
	for (i = 37; i < 47; i++)
		pPack->pSplLvl2[i - 37] = p->_pSplLvl[i];
#endif

	pki = &pPack->InvBody[0];
	pi = &p->InvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->InvList[0];
	pi = &p->InvList[0];

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++)
		pPack->InvGrid[i] = p->InvGrid[i];

	pPack->_pNumInv = p->_pNumInv;
	pki = &pPack->SpdList[0];
	pi = &p->SpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}

	static_assert((sizeof(p->_pSplHotKey) / sizeof(p->_pSplHotKey[0])) == 4, "Packing is no longer compatible with _pSplHotKey");
	static_assert((sizeof(p->_pSplTHotKey) / sizeof(p->_pSplTHotKey[0])) == 4, "Packing is no longer compatible with _pSplTHotKey");
	for (i = 0; i < 4; i++) {
		pPack->pSplHotKey[i] = p->_pSplHotKey[i];
		pPack->pSplTHotKey[i] = p->_pSplTHotKey[i];
	}

#ifdef HELLFIRE
	pPack->wReflection = p->wReflection;
#endif
	pPack->pDiabloKillLevel = SwapLE32(p->pDiabloKillLevel);
	pPack->pManaShield = p->pManaShield;
}

/**
 * Expand a PkItemStruct in to a ItemStruct
 *
 * Note: last slot of item[MAXITEMS] used as temporary buffer
 * find real name reference below, possibly [sizeof(item[])/sizeof(ItemStruct)]
 * @param pis The source packed item
 * @param is The distination item
 */
void UnPackItem(PkItemStruct *pis, ItemStruct *is)
{
	WORD idx = SwapLE16(pis->idx);

	if (idx == 0xFFFF) {
		is->_itype = ITYPE_NONE;
	} else {
		if (idx == IDI_EAR) {
			RecreateEar(
			    SwapLE16(pis->iCreateInfo),
			    SwapLE32(pis->iSeed),
			    pis->bId,
			    pis->bDur,
			    pis->bMDur,
			    pis->bCh,
			    pis->bMCh,
			    SwapLE16(pis->wValue),
			    SwapLE32(pis->dwBuff));
		} else {
			RecreateItem(
				idx,
				SwapLE16(pis->iCreateInfo),
				SwapLE32(pis->iSeed),
				SwapLE16(pis->wValue));
			item[MAXITEMS]._iMagical = pis->bId >> 1;
			item[MAXITEMS]._iIdentified = pis->bId & 1;
			item[MAXITEMS]._iDurability = pis->bDur;
			item[MAXITEMS]._iMaxDur = pis->bMDur;
			item[MAXITEMS]._iCharges = pis->bCh;
			item[MAXITEMS]._iMaxCharges = pis->bMCh;
		}
		copy_pod(*is, item[MAXITEMS]);
	}
}

static void VerifyGoldSeeds(PlayerStruct *p)
{
	int i, j;

	for (i = 0; i < p->_pNumInv; i++) {
		if (p->InvList[i]._iIdx == IDI_GOLD) {
			for (j = 0; j < p->_pNumInv; j++) {
				if (i != j) {
					if (p->InvList[j]._iIdx == IDI_GOLD && p->InvList[i]._iSeed == p->InvList[j]._iSeed) {
						p->InvList[i]._iSeed = GetRndSeed();
						j = -1;
					}
				}
			}
		}
	}
}

void UnPackPlayer(PkPlayerStruct *pPack, int pnum, BOOL killok)
{
	PlayerStruct *p;
	int i;
	ItemStruct *pi;
	PkItemStruct *pki;

	p = &plr[pnum];
	p->_px = pPack->px;
	p->_py = pPack->py;
	p->_pfutx = pPack->px;
	p->_pfuty = pPack->py;
	p->_ptargx = pPack->targx;
	p->_ptargy = pPack->targy;
	p->plrlevel = pPack->plrlevel;
	ClrPlrPath(pnum);
	p->destAction = ACTION_NONE;
	copy_str(p->_pName, pPack->pName);
	p->_pClass = pPack->pClass;
	p->_pLevel = pPack->pLevel;
	InitPlayer(pnum, TRUE);
	p->_pBaseStr = pPack->pBaseStr;
	p->_pStrength = pPack->pBaseStr;
	p->_pBaseMag = pPack->pBaseMag;
	p->_pMagic = pPack->pBaseMag;
	p->_pBaseDex = pPack->pBaseDex;
	p->_pDexterity = pPack->pBaseDex;
	p->_pBaseVit = pPack->pBaseVit;
	p->_pVitality = pPack->pBaseVit;
	p->_pStatPts = pPack->pStatPts;
	p->_pExperience = SwapLE32(pPack->pExperience);
	p->_pGold = SwapLE32(pPack->pGold);
	p->_pMaxHPBase = SwapLE32(pPack->pMaxHPBase);
	p->_pHPBase = SwapLE32(pPack->pHPBase);
	if (!killok)
		if (p->_pHPBase < 64)
			p->_pHPBase = 64;

	p->_pMaxManaBase = SwapLE32(pPack->pMaxManaBase);
	p->_pManaBase = SwapLE32(pPack->pManaBase);
	p->_pMemSpells = SDL_SwapLE64(pPack->pMemSpells);

	for (i = 0; i < 37; i++) // Should be NUM_SPELLS but set to 37 to make save games compatible
		p->_pSplLvl[i] = pPack->pSplLvl[i];
#ifdef HELLFIRE
	for (i = 37; i < 47; i++)
		p->_pSplLvl[i] = pPack->pSplLvl2[i - 37];
#endif

	pki = &pPack->InvBody[0];
	pi = &p->InvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->InvList[0];
	pi = &p->InvList[0];

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++)
		p->InvGrid[i] = pPack->InvGrid[i];

	p->_pNumInv = pPack->_pNumInv;
	VerifyGoldSeeds(p);

	pki = &pPack->SpdList[0];
	pi = &p->SpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	static_assert((sizeof(p->_pSplHotKey) / sizeof(p->_pSplHotKey[0])) == 4, "Packing is no longer compatible with _pSplHotKey");
	static_assert((sizeof(p->_pSplTHotKey) / sizeof(p->_pSplTHotKey[0])) == 4, "Packing is no longer compatible with _pSplTHotKey");
	for (i = 0; i < 4; i++) {
		p->_pSplHotKey[i] = pPack->pSplHotKey[i];
		p->_pSplTHotKey[i] = pPack->pSplTHotKey[i];
	}

	CalcPlrInv(pnum, FALSE);

	p->wReflection = pPack->wReflection;
	p->pTownWarps = 0;
	p->pDungMsgs = 0;
	p->pDungMsgs2 = 0;
	p->pLvlLoad = 0;
	p->pDiabloKillLevel = SwapLE32(pPack->pDiabloKillLevel);
	p->pBattleNet = pPack->pBattleNet;
	p->pManaShield = pPack->pManaShield;
}

DEVILUTION_END_NAMESPACE
