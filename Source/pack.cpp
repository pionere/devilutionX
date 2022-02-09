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
		pis->wIndx = SwapLE16(IDI_NONE);
	} else if (is->_itype == ITYPE_PLACEHOLDER) {
		pis->wIndx = SwapLE16(IDI_PHOLDER);
		pis->dwBuff = SwapLE32(is->_iPHolder);
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
	pPack->px = p->_px;
	pPack->py = p->_py;
	copy_str(pPack->pName, p->_pName);
	pPack->pLvlChanging = p->_pLvlChanging;
	pPack->pDunLevel = p->_pDunLevel;
	pPack->pClass = p->_pClass;
	pPack->pLevel = p->_pLevel;
	pPack->pRank = p->_pRank;
	pPack->pTeam = p->_pTeam;
	pPack->pStatPts = SwapLE16(p->_pStatPts);
	pPack->pLightRad = p->_pLightRad;
	pPack->pManaShield = p->_pManaShield;
	pPack->pTimer[PLTR_INFRAVISION] = SwapLE16(p->_pTimer[PLTR_INFRAVISION]);
	pPack->pTimer[PLTR_RAGE] = SwapLE16(p->_pTimer[PLTR_RAGE]);
	pPack->pExperience = SwapLE32(p->_pExperience);
	pPack->pBaseStr = SwapLE16(p->_pBaseStr);
	pPack->pBaseMag = SwapLE16(p->_pBaseMag);
	pPack->pBaseDex = SwapLE16(p->_pBaseDex);
	pPack->pBaseVit = SwapLE16(p->_pBaseVit);
	pPack->pHPBase = SwapLE32(p->_pHPBase);
	pPack->pMaxHPBase = SwapLE32(p->_pMaxHPBase);
	pPack->pManaBase = SwapLE32(p->_pManaBase);
	pPack->pMaxManaBase = SwapLE32(p->_pMaxManaBase);

	memcpy(pPack->pAtkSkillHotKey, p->_pAtkSkillHotKey, sizeof(pPack->pAtkSkillHotKey));
	memcpy(pPack->pAtkSkillTypeHotKey, p->_pAtkSkillTypeHotKey, sizeof(pPack->pAtkSkillTypeHotKey));
	memcpy(pPack->pMoveSkillHotKey, p->_pMoveSkillHotKey, sizeof(pPack->pMoveSkillHotKey));
	memcpy(pPack->pMoveSkillTypeHotKey, p->_pMoveSkillTypeHotKey, sizeof(pPack->pMoveSkillTypeHotKey));
	memcpy(pPack->pAltAtkSkillHotKey, p->_pAltAtkSkillHotKey, sizeof(pPack->pAltAtkSkillHotKey));
	memcpy(pPack->pAltAtkSkillTypeHotKey, p->_pAltAtkSkillTypeHotKey, sizeof(pPack->pAltAtkSkillTypeHotKey));
	memcpy(pPack->pAltMoveSkillHotKey, p->_pAltMoveSkillHotKey, sizeof(pPack->pAltMoveSkillHotKey));
	memcpy(pPack->pAltMoveSkillTypeHotKey, p->_pAltMoveSkillTypeHotKey, sizeof(pPack->pAltMoveSkillTypeHotKey));

	static_assert(sizeof(p->_pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy I.");
	static_assert(sizeof(pPack->pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy II.");
	memcpy(pPack->pSkillLvlBase, p->_pSkillLvlBase, sizeof(p->_pSkillLvlBase));
	static_assert(sizeof(p->_pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy III.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy IV.");
	memcpy(pPack->pSkillActivity, p->_pSkillActivity, sizeof(p->_pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "Packing of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		pPack->pSkillExp[i] = SwapLE32(p->_pSkillExp[i]);
	}
	pPack->pMemSkills = SwapLE64(p->_pMemSkills);

	PackItem(&pPack->pHoldItem, &p->_pHoldItem);

	pki = &pPack->pInvBody[0];
	pi = &p->_pInvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->pSpdList[0];
	pi = &p->_pSpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->pInvList[0];
	pi = &p->_pInvList[0];

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		PackItem(pki, pi);
		pki++;
		pi++;
	}
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

	if (idx == IDI_NONE) {
		is->_itype = ITYPE_NONE;
	} else if (idx == IDI_PHOLDER) {
		is->_itype = ITYPE_PLACEHOLDER;
		is->_iPHolder = SwapLE32(pis->dwBuff);
	} else {
		UnPackPkItem(pis);
		copy_pod(*is, items[MAXITEMS]);
	}
}

void UnPackPlayer(PkPlayerStruct *pPack, int pnum)
{
	int i, j;
	ItemStruct *pi, *is;
	PkItemStruct *pki;

	// TODO: validate data from the internet
	SetPlayerLoc(&plr, pPack->px, pPack->py);
	copy_cstr(plr._pName, pPack->pName);
	plr._pLvlChanging = pPack->pLvlChanging;
	plr._pDunLevel = pPack->pDunLevel;
	plr._pClass = pPack->pClass;
	plr._pLevel = pPack->pLevel;
	plr._pRank = pPack->pRank;
	plr._pTeam = pPack->pTeam;
	plr._pStatPts = SwapLE16(pPack->pStatPts);
	plr._pLightRad = pPack->pLightRad;
	plr._pManaShield = pPack->pManaShield;
	plr._pTimer[PLTR_INFRAVISION] = SwapLE16(pPack->pTimer[PLTR_INFRAVISION]);
	plr._pTimer[PLTR_RAGE] = SwapLE16(pPack->pTimer[PLTR_RAGE]);
	plr._pExperience = SwapLE32(pPack->pExperience);
	plr._pBaseStr = SwapLE16(pPack->pBaseStr);
	plr._pBaseMag = SwapLE16(pPack->pBaseMag);
	plr._pBaseDex = SwapLE16(pPack->pBaseDex);
	plr._pBaseVit = SwapLE16(pPack->pBaseVit);
	plr._pMaxHPBase = SwapLE32(pPack->pMaxHPBase);
	plr._pHPBase = SwapLE32(pPack->pHPBase);
	plr._pMaxManaBase = SwapLE32(pPack->pMaxManaBase);
	plr._pManaBase = SwapLE32(pPack->pManaBase);

	memcpy(plr._pAtkSkillHotKey, pPack->pAtkSkillHotKey, sizeof(plr._pAtkSkillHotKey));
	memcpy(plr._pAtkSkillTypeHotKey, pPack->pAtkSkillTypeHotKey, sizeof(plr._pAtkSkillTypeHotKey));
	memcpy(plr._pMoveSkillHotKey, pPack->pMoveSkillHotKey, sizeof(plr._pMoveSkillHotKey));
	memcpy(plr._pMoveSkillTypeHotKey, pPack->pMoveSkillTypeHotKey, sizeof(plr._pMoveSkillTypeHotKey));
	memcpy(plr._pAltAtkSkillHotKey, pPack->pAltAtkSkillHotKey, sizeof(plr._pAltAtkSkillHotKey));
	memcpy(plr._pAltAtkSkillTypeHotKey, pPack->pAltAtkSkillTypeHotKey, sizeof(plr._pAltAtkSkillTypeHotKey));
	memcpy(plr._pAltMoveSkillHotKey, pPack->pAltMoveSkillHotKey, sizeof(plr._pAltMoveSkillHotKey));
	memcpy(plr._pAltMoveSkillTypeHotKey, pPack->pAltMoveSkillTypeHotKey, sizeof(plr._pAltMoveSkillTypeHotKey));

	static_assert(sizeof(plr._pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy V.");
	static_assert(sizeof(pPack->pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VI.");
	memcpy(plr._pSkillLvlBase, pPack->pSkillLvlBase, sizeof(pPack->pSkillLvlBase));
	static_assert(sizeof(plr._pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VII.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VIII.");
	memcpy(plr._pSkillActivity, pPack->pSkillActivity, sizeof(pPack->pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "UnPacking of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		plr._pSkillExp[i] = SwapLE32(pPack->pSkillExp[i]);
	}
	plr._pMemSkills = SwapLE64(pPack->pMemSkills);

	UnPackItem(&pPack->pHoldItem, &plr._pHoldItem);

	pki = &pPack->pInvBody[0];
	pi = &plr._pInvBody[0];

	for (i = 0; i < NUM_INVLOC; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->pSpdList[0];
	pi = &plr._pSpdList[0];

	for (i = 0; i < MAXBELTITEMS; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	pki = &pPack->pInvList[0];
	pi = &plr._pInvList[0];

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		UnPackItem(pki, pi);
		pki++;
		pi++;
	}

	// reset fields which are used even by non-local players, but not part of pPack
	ClrPlrPath(pnum);
	plr.destAction = ACTION_NONE;
	// TODO: add to pPack? (_pInvincible, _pmode)
	plr._pInvincible = FALSE;
	plr._pmode = PM_NEWLVL;
	plr._plid = -1;
	plr._pvid = -1;

	// verify the data
	//  make sure the name is NULL terminated
	plr._pName[sizeof(plr._pName) - 1] = '\0';
	if (plr._pClass >= NUM_CLASSES)
		plr._pClass = PC_WARRIOR;	// reset invalid class
	if (plr._pLevel > MAXCHARLEVEL)
		plr._pLevel = MAXCHARLEVEL;	// reduce invalid level
	if (plr._pTeam >= MAX_PLRS)
		plr._pTeam = pnum;			// overwrite invalid team
	pi = &plr._pInvBody[INVLOC_HAND_LEFT];
	if (pi->_iClass != ICLASS_WEAPON || pi->_itype == ITYPE_PLACEHOLDER)
		pi->_itype = ITYPE_NONE;    // remove invalid weapon in left hand
	if (pi->_itype == ITYPE_NONE) {
		pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
		if (pi->_itype != ITYPE_SHIELD)
			pi->_itype = ITYPE_NONE; // remove invalid weapon/item in right hand
	}
	// TODO: check if the items conform to the wielding rules?
	// TODO: check placeholders
	// verify the gold-seeds TODO check gold values?
	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		pi = &plr._pInvList[i];
		if (pi->_iIdx == IDI_GOLD
		 && pi->_itype != ITYPE_NONE && pi->_itype != ITYPE_PLACEHOLDER) {
			//if (pi->_ivalue > GOLD_MAX_LIMIT)
			//	pi->_ivalue = GOLD_MAX_LIMIT;
			for (j = 0; j < NUM_INV_GRID_ELEM; j++) {
				if (i == j)
					continue;
				is = &plr._pInvList[j];
				if (is->_iIdx == IDI_GOLD && is->_iSeed == pi->_iSeed
				 && is->_itype != ITYPE_NONE && is->_itype != ITYPE_PLACEHOLDER) {
					pi->_iSeed = GetRndSeed();
					j = -1;
				}
			}
		}
	}
	// recalculate the cached fields
	InitPlayer(pnum);
	CalcPlrInv(pnum, false);
}

DEVILUTION_END_NAMESPACE
