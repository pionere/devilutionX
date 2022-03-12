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
	//pPack->px = p->_px;
	//pPack->py = p->_py;
	copy_str(pPack->pName, p->_pName);
	pPack->pLvlChanging = p->_pLvlChanging;
	pPack->pDunLevel = p->_pDunLevel;
	pPack->pClass = p->_pClass;
	pPack->pLevel = p->_pLevel;
	pPack->pRank = p->_pRank;
	pPack->pTeam = p->_pTeam;
	pPack->pStatPts = SwapLE16(p->_pStatPts);
	//pPack->pLightRad = p->_pLightRad;
	//pPack->pManaShield = p->_pManaShield;
	//pPack->pTimer[PLTR_INFRAVISION] = SwapLE16(p->_pTimer[PLTR_INFRAVISION]);
	//pPack->pTimer[PLTR_RAGE] = SwapLE16(p->_pTimer[PLTR_RAGE]);
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
 * @param is The destination item
 */
static void UnPackItem(const PkItemStruct* pis, ItemStruct* is)
{
	uint16_t idx = SwapLE16(pis->wIndx);

	if (idx == IDI_NONE) {
		is->_itype = ITYPE_NONE;
	} else if (idx == IDI_PHOLDER) {
		is->_itype = ITYPE_PLACEHOLDER;
		is->_iPHolder = SwapLE32(pis->dwBuff);
		// TODO: check the referenced item?
		net_assert((unsigned)is->_iPHolder < NUM_INV_GRID_ELEM);
	} else {
		UnPackPkItem(pis);
		copy_pod(*is, items[MAXITEMS]);
	}
}

void UnPackPlayer(PkPlayerStruct* pPack, int pnum)
{
	int i;
	ItemStruct* pi;
	PkItemStruct* pki;

	// TODO: validate data from the internet
	//SetPlayerLoc(&plr, pPack->px, pPack->py);
	copy_cstr(plr._pName, pPack->pName);
	plr._pLvlChanging = pPack->pLvlChanging;
	plr._pDunLevel = pPack->pDunLevel;
	plr._pClass = pPack->pClass;
	plr._pLevel = pPack->pLevel;
	plr._pRank = pPack->pRank;
	plr._pTeam = pPack->pTeam;
	plr._pStatPts = SwapLE16(pPack->pStatPts);
	//plr._pLightRad = pPack->pLightRad;
	//plr._pManaShield = pPack->pManaShield;
	//plr._pTimer[PLTR_INFRAVISION] = SwapLE16(pPack->pTimer[PLTR_INFRAVISION]);
	//plr._pTimer[PLTR_RAGE] = SwapLE16(pPack->pTimer[PLTR_RAGE]);
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
	// TODO: move these to SetupLocalPlr
	ClrPlrPath(pnum);
	plr.destAction = ACTION_NONE;
	plr._pInvincible = FALSE;
	plr._pmode = PM_NEWLVL;
	// commented out, because these should not matter
	//plr._plid = NO_LIGHT;
	//plr._pvid = NO_VISION;

#if INET_MODE
	// verify the data
	//  make sure the name is NULL terminated
	plr._pName[sizeof(plr._pName) - 1] = '\0';
	net_assert(plr._pClass < NUM_CLASSES);
	net_assert(plr._pLevel >= 1 && plr._pLevel <= MAXCHARLEVEL);
	net_assert(plr._pExperience < PlrExpLvlsTbl[plr._pLevel]);
	net_assert(plr._pDunLevel < NUM_LEVELS);
	net_assert(plr._pTeam < MAX_PLRS);
	net_assert((plr._pMemSkills & ~(SPELL_MASK(NUM_SPELLS) - 1)) == 0);	
	for (i = 0; i < NUM_SPELLS; i++) {
		if (plr._pMemSkills & SPELL_MASK(i))
			net_assert(spelldata[i].sBookLvl != SPELL_NA);
	}
	// check if the items conform to the wielding rules
	for (i = 0; i < MAXBELTITEMS; i++) {
		// - no placeholder in belt
		pi = &plr._pSpdList[i];
		net_assert(pi->_itype != ITYPE_PLACEHOLDER);
		// - only belt items in belt
		net_assert(pi->_itype == ITYPE_NONE || pi->_iLoc == ILOC_BELT);
	}
	// - no placeholder on body
	for (i = 0; i < NUM_INVLOC; i++)
		net_assert(plr._pInvBody[i]._itype != ITYPE_PLACEHOLDER);
	// - allow only helm on head
	pi = &plr._pInvBody[INVLOC_HEAD];
	net_assert(pi->_itype == ITYPE_NONE || pi->_iLoc == ILOC_HELM);
	// - allow only armor on chest
	pi = &plr._pInvBody[INVLOC_CHEST];
	net_assert(pi->_itype == ITYPE_NONE || pi->_iLoc == ILOC_ARMOR);
	// - allow only amulet on neck
	pi = &plr._pInvBody[INVLOC_AMULET];
	net_assert(pi->_itype == ITYPE_NONE || pi->_iLoc == ILOC_AMULET);
	// - allow only ring on hand
	pi = &plr._pInvBody[INVLOC_RING_LEFT];
	net_assert(pi->_itype == ITYPE_NONE || pi->_iLoc == ILOC_RING);
	pi = &plr._pInvBody[INVLOC_RING_RIGHT];
	net_assert(pi->_itype == ITYPE_NONE || pi->_iLoc == ILOC_RING);
	// - allow only weapon in left hand
	pi = &plr._pInvBody[INVLOC_HAND_LEFT];
	net_assert(pi->_itype == ITYPE_NONE || pi->_iClass == ICLASS_WEAPON);
	if (pi->_itype == ITYPE_NONE) {
		// - allow only shield in right hand when left hand is empty
		pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
		net_assert(pi->_itype == ITYPE_NONE || pi->_itype == ITYPE_SHIELD);
	} else {
		// - right hand must be empty when wielding a two handed weapon
		if (pi->_iLoc == ILOC_TWOHAND && TWOHAND_WIELD(&plr, pi)) {
			net_assert(plr._pInvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE);
		}
	}
#endif /* INET_MODE */

	// recalculate the cached fields
	InitPlayer(pnum);
	CalcPlrInv(pnum, false);
}

DEVILUTION_END_NAMESPACE
