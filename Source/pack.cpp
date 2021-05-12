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
	p = &plr[pnum];
	pPack->plrlevel = p->plrlevel;
	pPack->pTeam = p->_pTeam;
	pPack->px = p->_px;
	pPack->py = p->_py;
	copy_str(pPack->pName, p->_pName);
	pPack->pClass = p->_pClass;
	pPack->pBaseStr = SwapLE16(p->_pBaseStr);
	pPack->pBaseMag = SwapLE16(p->_pBaseMag);
	pPack->pBaseDex = SwapLE16(p->_pBaseDex);
	pPack->pBaseVit = SwapLE16(p->_pBaseVit);
	pPack->pLevel = p->_pLevel;
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
	pPack->pManaShield = p->_pManaShield;

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

void UnPackPlayer(PkPlayerStruct *pPack, int pnum)
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
	p->plrlevel = pPack->plrlevel;
	p->_pTeam = pPack->pTeam;
	ClrPlrPath(pnum);
	p->destAction = ACTION_NONE;
	copy_str(p->_pName, pPack->pName);
	p->_pClass = pPack->pClass;
	p->_pLevel = pPack->pLevel;
	p->_pLightRad = pPack->pLightRad;
	p->_pManaShield = pPack->pManaShield;
	p->_pDiabloKillLevel = pPack->pDiabloKillLevel;
	p->_pStatPts = SwapLE16(pPack->pStatPts);
	InitPlayer(pnum, true, false);
	p->_pBaseStr = SwapLE16(pPack->pBaseStr);
	p->_pBaseMag = SwapLE16(pPack->pBaseMag);
	p->_pBaseDex = SwapLE16(pPack->pBaseDex);
	p->_pBaseVit = SwapLE16(pPack->pBaseVit);
	p->_pExperience = SwapLE32(pPack->pExperience);
	p->_pGold = SwapLE32(pPack->pGold);
	p->_pMaxHPBase = SwapLE32(pPack->pMaxHPBase);
	p->_pHPBase = SwapLE32(pPack->pHPBase);
	if (p->_pHPBase < 64) // TODO: is this necessary?
		p->_pHPBase = 64;

	p->_pMaxManaBase = SwapLE32(pPack->pMaxManaBase);
	p->_pManaBase = SwapLE32(pPack->pManaBase);

	static_assert(sizeof(p->_pSkillLvl[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy V.");
	static_assert(sizeof(pPack->pSkillLvl[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VI.");
	memcpy(p->_pSkillLvl, pPack->pSkillLvl, sizeof(pPack->pSkillLvl));
	static_assert(sizeof(p->_pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VII.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VIII.");
	memcpy(p->_pSkillActivity, pPack->pSkillActivity, sizeof(pPack->pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "UnPacking of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		p->_pSkillExp[i] = SwapLE32(pPack->pSkillExp[i]);
	}
	p->_pMemSkills = SwapLE64(pPack->pMemSkills);

	pki = &pPack->InvBody[0];
	pi = &p->InvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->SpdList[0];
	pi = &p->SpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
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

	memcpy(p->_pAtkSkillHotKey, pPack->pAtkSkillHotKey, sizeof(p->_pAtkSkillHotKey));
	memcpy(p->_pAtkSkillTypeHotKey, pPack->pAtkSkillTypeHotKey, sizeof(p->_pAtkSkillTypeHotKey));
	memcpy(p->_pMoveSkillHotKey, pPack->pMoveSkillHotKey, sizeof(p->_pMoveSkillHotKey));
	memcpy(p->_pMoveSkillTypeHotKey, pPack->pMoveSkillTypeHotKey, sizeof(p->_pMoveSkillTypeHotKey));
	memcpy(p->_pAltAtkSkillHotKey, pPack->pAltAtkSkillHotKey, sizeof(p->_pAltAtkSkillHotKey));
	memcpy(p->_pAltAtkSkillTypeHotKey, pPack->pAltAtkSkillTypeHotKey, sizeof(p->_pAltAtkSkillTypeHotKey));
	memcpy(p->_pAltMoveSkillHotKey, pPack->pAltMoveSkillHotKey, sizeof(p->_pAltMoveSkillHotKey));
	memcpy(p->_pAltMoveSkillTypeHotKey, pPack->pAltMoveSkillTypeHotKey, sizeof(p->_pAltMoveSkillTypeHotKey));

	p->pTownWarps = 0;
	p->palign_CB = 0;
	p->pDungMsgs = 0;
	p->pDungMsgs2 = 0;
	p->_plid = -1;
	p->_pvid = -1;

	VerifyGoldSeeds(p);
	CalcPlrInv(pnum, false);
}

DEVILUTION_END_NAMESPACE
