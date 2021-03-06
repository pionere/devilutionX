/**
 * @file pack.cpp
 *
 * Implementation of functions for minifying player data structure.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static void PackItem(PkItemStruct *pis, const ItemStruct *is)
{
	if (is->_itype == ITYPE_NONE) {
		pis->wIndx = 0xFFFF;
	} else {
		PackPkItem(pis, is);
	}
}

void PackPlayer(PkPlayerStruct *pPack, int pnum)
{
	PlayerStruct *p;
	int i;
	ItemStruct *pi;
	PkItemStruct *pki;

	memset(pPack, 0, sizeof(*pPack));
	p = &plr;
	pPack->plrlevel = p->plrlevel;
	pPack->pTeam = p->_pTeam;
	pPack->px = p->_px;
	pPack->py = p->_py;
	copy_str(pPack->pName, p->_pName);
	pPack->pClass = p->_pClass;
	pPack->pLevel = p->_pLevel;
	pPack->pLightRad = p->_pLightRad;
	pPack->pManaShield = p->_pManaShield;
	pPack->pTimer[PT_INFRAVISION] = SwapLE16(p->_pTimer[PT_INFRAVISION]);
	pPack->pTimer[PT_RAGE] = SwapLE16(p->_pTimer[PT_RAGE]);
	pPack->pBaseStr = SwapLE16(p->_pBaseStr);
	pPack->pBaseMag = SwapLE16(p->_pBaseMag);
	pPack->pBaseDex = SwapLE16(p->_pBaseDex);
	pPack->pBaseVit = SwapLE16(p->_pBaseVit);
	pPack->pDiabloKillLevel = p->_pDiabloKillLevel;
	pPack->pStatPts = SwapLE16(p->_pStatPts);
	pPack->pExperience = SwapLE32(p->_pExperience);
	pPack->pGold = SwapLE32(p->_pGold);
	pPack->pHPBase = SwapLE32(p->_pHPBase);
	pPack->pMaxHPBase = SwapLE32(p->_pMaxHPBase);
	pPack->pManaBase = SwapLE32(p->_pManaBase);
	pPack->pMaxManaBase = SwapLE32(p->_pMaxManaBase);

	static_assert(sizeof(p->_pSkillLvl[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy I.");
	static_assert(sizeof(pPack->pSkillLvl[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy II.");
	memcpy(pPack->pSkillLvl, p->_pSkillLvl, sizeof(p->_pSkillLvl));
	static_assert(sizeof(p->_pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy III.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy IV.");
	memcpy(pPack->pSkillActivity, p->_pSkillActivity, sizeof(p->_pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "Packing of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		pPack->pSkillExp[i] = SwapLE32(p->_pSkillExp[i]);
	}
	pPack->pMemSkills = SwapLE64(p->_pMemSkills);

	pki = &pPack->InvBody[0];
	pi = &p->InvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->SpdList[0];
	pi = &p->SpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
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

	memcpy(pPack->pAtkSkillHotKey, p->_pAtkSkillHotKey, sizeof(pPack->pAtkSkillHotKey));
	memcpy(pPack->pAtkSkillTypeHotKey, p->_pAtkSkillTypeHotKey, sizeof(pPack->pAtkSkillTypeHotKey));
	memcpy(pPack->pMoveSkillHotKey, p->_pMoveSkillHotKey, sizeof(pPack->pMoveSkillHotKey));
	memcpy(pPack->pMoveSkillTypeHotKey, p->_pMoveSkillTypeHotKey, sizeof(pPack->pMoveSkillTypeHotKey));
	memcpy(pPack->pAltAtkSkillHotKey, p->_pAltAtkSkillHotKey, sizeof(pPack->pAltAtkSkillHotKey));
	memcpy(pPack->pAltAtkSkillTypeHotKey, p->_pAltAtkSkillTypeHotKey, sizeof(pPack->pAltAtkSkillTypeHotKey));
	memcpy(pPack->pAltMoveSkillHotKey, p->_pAltMoveSkillHotKey, sizeof(pPack->pAltMoveSkillHotKey));
	memcpy(pPack->pAltMoveSkillTypeHotKey, p->_pAltMoveSkillTypeHotKey, sizeof(pPack->pAltMoveSkillTypeHotKey));
}

/**
 * Expand a PkItemStruct in to a ItemStruct
 *
 * Note: last slot of items[MAXITEMS] used as temporary buffer
 * find real name reference below, possibly [sizeof(items)/sizeof(ItemStruct)]
 * @param pis The source packed item
 * @param is The distination item
 */
static void UnPackItem(const PkItemStruct *pis, ItemStruct *is)
{
	uint16_t idx = SwapLE16(pis->wIndx);

	if (idx == 0xFFFF) {
		is->_itype = ITYPE_NONE;
	} else {
		UnPackPkItem(pis);
		copy_pod(*is, items[MAXITEMS]);
	}
}

void UnPackPlayer(PkPlayerStruct *pPack, int pnum)
{
	int i, j;
	ItemStruct *pi;
	PkItemStruct *pki;

	// TODO: validate data from the internet
	SetPlayerLoc(&plr, pPack->px, pPack->py);
	plr.plrlevel = pPack->plrlevel;
	plr._pTeam = pPack->pTeam;
	ClrPlrPath(pnum);
	plr.destAction = ACTION_NONE;
	copy_str(plr._pName, pPack->pName);
	plr._pClass = pPack->pClass;
	plr._pLevel = pPack->pLevel;
	plr._pLightRad = pPack->pLightRad;
	plr._pManaShield = pPack->pManaShield;
	plr._pTimer[PT_INFRAVISION] = SwapLE16(pPack->pTimer[PT_INFRAVISION]);
	plr._pTimer[PT_RAGE] = SwapLE16(pPack->pTimer[PT_RAGE]);
	plr._pDiabloKillLevel = pPack->pDiabloKillLevel;
	plr._pStatPts = SwapLE16(pPack->pStatPts);
	InitPlayer(pnum, true, false);
	plr._pBaseStr = SwapLE16(pPack->pBaseStr);
	plr._pBaseMag = SwapLE16(pPack->pBaseMag);
	plr._pBaseDex = SwapLE16(pPack->pBaseDex);
	plr._pBaseVit = SwapLE16(pPack->pBaseVit);
	plr._pExperience = SwapLE32(pPack->pExperience);
	plr._pGold = SwapLE32(pPack->pGold);
	plr._pMaxHPBase = SwapLE32(pPack->pMaxHPBase);
	plr._pHPBase = SwapLE32(pPack->pHPBase);
	if (plr._pHPBase < 64) // TODO: is this necessary?
		plr._pHPBase = 64;

	plr._pMaxManaBase = SwapLE32(pPack->pMaxManaBase);
	plr._pManaBase = SwapLE32(pPack->pManaBase);

	static_assert(sizeof(plr._pSkillLvl[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy V.");
	static_assert(sizeof(pPack->pSkillLvl[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VI.");
	memcpy(plr._pSkillLvl, pPack->pSkillLvl, sizeof(pPack->pSkillLvl));
	static_assert(sizeof(plr._pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VII.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VIII.");
	memcpy(plr._pSkillActivity, pPack->pSkillActivity, sizeof(pPack->pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "UnPacking of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		plr._pSkillExp[i] = SwapLE32(pPack->pSkillExp[i]);
	}
	plr._pMemSkills = SwapLE64(pPack->pMemSkills);

	pki = &pPack->InvBody[0];
	pi = &plr.InvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->SpdList[0];
	pi = &plr.SpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->InvList[0];
	pi = &plr.InvList[0];

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	for (i = 0; i < NUM_INV_GRID_ELEM; i++)
		plr.InvGrid[i] = pPack->InvGrid[i];

	plr._pNumInv = pPack->_pNumInv;

	memcpy(plr._pAtkSkillHotKey, pPack->pAtkSkillHotKey, sizeof(plr._pAtkSkillHotKey));
	memcpy(plr._pAtkSkillTypeHotKey, pPack->pAtkSkillTypeHotKey, sizeof(plr._pAtkSkillTypeHotKey));
	memcpy(plr._pMoveSkillHotKey, pPack->pMoveSkillHotKey, sizeof(plr._pMoveSkillHotKey));
	memcpy(plr._pMoveSkillTypeHotKey, pPack->pMoveSkillTypeHotKey, sizeof(plr._pMoveSkillTypeHotKey));
	memcpy(plr._pAltAtkSkillHotKey, pPack->pAltAtkSkillHotKey, sizeof(plr._pAltAtkSkillHotKey));
	memcpy(plr._pAltAtkSkillTypeHotKey, pPack->pAltAtkSkillTypeHotKey, sizeof(plr._pAltAtkSkillTypeHotKey));
	memcpy(plr._pAltMoveSkillHotKey, pPack->pAltMoveSkillHotKey, sizeof(plr._pAltMoveSkillHotKey));
	memcpy(plr._pAltMoveSkillTypeHotKey, pPack->pAltMoveSkillTypeHotKey, sizeof(plr._pAltMoveSkillTypeHotKey));

	plr.pTownWarps = 0;
	plr.palign_CB = 0;
	plr.pDungMsgs = 0;
	plr.pDungMsgs2 = 0;
	plr._plid = -1;
	plr._pvid = -1;

	// verify the gold-seeds
	for (i = 0; i < plr._pNumInv; i++) {
		if (plr.InvList[i]._iIdx == IDI_GOLD) {
			for (j = 0; j < plr._pNumInv; j++) {
				if (i != j) {
					if (plr.InvList[j]._iIdx == IDI_GOLD && plr.InvList[i]._iSeed == plr.InvList[j]._iSeed) {
						plr.InvList[i]._iSeed = GetRndSeed();
						j = -1;
					}
				}
			}
		}
	}
	// recalculate the cached fields
	CalcPlrInv(pnum, false);
}

DEVILUTION_END_NAMESPACE
