/**
 * @file pack.cpp
 *
 * Implementation of functions for minifying player data structure.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static void PackItem(PkItemStruct* pis, const ItemStruct* is)
{
	if (is->_itype == ITYPE_NONE) {
		pis->wIndx = static_cast<uint16_t>(IDI_NONE);
	} else if (is->_itype == ITYPE_PLACEHOLDER) {
		pis->wIndx = static_cast<uint16_t>(IDI_PHOLDER);
		pis->dwBuff = static_cast<uint32_t>(is->_iPHolder);
	} else {
		PackPkItem(pis, is);
	}
}

void PackPlayer(PkPlayerStruct* pPack, int pnum)
{
	PlayerStruct* p;
	int i;
	ItemStruct* pi;
	PkItemStruct* pki;

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
	pPack->pStatPts = p->_pStatPts;
	//pPack->pLightRad = p->_pLightRad;
	//pPack->pManaShield = p->_pManaShield;
	//pPack->pTimer[PLTR_INFRAVISION] = p->_pTimer[PLTR_INFRAVISION];
	//pPack->pTimer[PLTR_RAGE] = p->_pTimer[PLTR_RAGE];
	pPack->pExperience = p->_pExperience;
	pPack->pBaseStr = p->_pBaseStr;
	pPack->pBaseMag = p->_pBaseMag;
	pPack->pBaseDex = p->_pBaseDex;
	pPack->pBaseVit = p->_pBaseVit;
	pPack->pHPBase = p->_pHPBase;
	pPack->pMaxHPBase = p->_pMaxHPBase;
	pPack->pManaBase = p->_pManaBase;
	pPack->pMaxManaBase = p->_pMaxManaBase;

	/*memcpy(pPack->pAtkSkillHotKey, p->_pAtkSkillHotKey, sizeof(pPack->pAtkSkillHotKey));
	memcpy(pPack->pAtkSkillTypeHotKey, p->_pAtkSkillTypeHotKey, sizeof(pPack->pAtkSkillTypeHotKey));
	memcpy(pPack->pMoveSkillHotKey, p->_pMoveSkillHotKey, sizeof(pPack->pMoveSkillHotKey));
	memcpy(pPack->pMoveSkillTypeHotKey, p->_pMoveSkillTypeHotKey, sizeof(pPack->pMoveSkillTypeHotKey));
	memcpy(pPack->pAltAtkSkillHotKey, p->_pAltAtkSkillHotKey, sizeof(pPack->pAltAtkSkillHotKey));
	memcpy(pPack->pAltAtkSkillTypeHotKey, p->_pAltAtkSkillTypeHotKey, sizeof(pPack->pAltAtkSkillTypeHotKey));
	memcpy(pPack->pAltMoveSkillHotKey, p->_pAltMoveSkillHotKey, sizeof(pPack->pAltMoveSkillHotKey));
	memcpy(pPack->pAltMoveSkillTypeHotKey, p->_pAltMoveSkillTypeHotKey, sizeof(pPack->pAltMoveSkillTypeHotKey));
	memcpy(pPack->pAltAtkSkillSwapKey, p->_pAltAtkSkillSwapKey, sizeof(pPack->pAltAtkSkillSwapKey));
	memcpy(pPack->pAltAtkSkillTypeSwapKey, p->_pAltAtkSkillTypeSwapKey, sizeof(pPack->pAltAtkSkillTypeSwapKey));
	memcpy(pPack->pAltMoveSkillSwapKey, p->_pAltMoveSkillSwapKey, sizeof(pPack->pAltMoveSkillSwapKey));
	memcpy(pPack->pAltMoveSkillTypeSwapKey, p->_pAltMoveSkillTypeSwapKey, sizeof(pPack->pAltMoveSkillTypeSwapKey));*/
	memcpy(pPack->pAtkSkillHotKey, plr._pAtkSkillHotKey, offsetof(PlayerStruct, _pAltMoveSkillTypeSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) + sizeof(plr._pAltMoveSkillTypeSwapKey));

	static_assert(sizeof(p->_pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy I.");
	static_assert(sizeof(pPack->pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy II.");
	memcpy(pPack->pSkillLvlBase, p->_pSkillLvlBase, sizeof(p->_pSkillLvlBase));
	static_assert(sizeof(p->_pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy III.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy IV.");
	memcpy(pPack->pSkillActivity, p->_pSkillActivity, sizeof(p->_pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "Packing of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		pPack->pSkillExp[i] = p->_pSkillExp[i];
	}
	pPack->pMemSkills = p->_pMemSkills;

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
	uint16_t idx = pis->wIndx;

	if (idx == IDI_NONE) {
		is->_itype = ITYPE_NONE;
	} else if (idx == IDI_PHOLDER) {
		is->_itype = ITYPE_PLACEHOLDER;
		is->_iPHolder = pis->dwBuff;
		// TODO: check the referenced item?
		net_assert((unsigned)is->_iPHolder < NUM_INV_GRID_ELEM);
	} else {
		UnPackPkItem(pis);
		copy_pod(*is, items[MAXITEMS]);
	}
}

void UnPackPlayer(const PkPlayerStruct* pPack, int pnum)
{
	int i;
	ItemStruct* pi;
	const PkItemStruct* pki;

	// TODO: validate data from the internet
	//SetPlayerLoc(&plr, pPack->px, pPack->py);
	copy_cstr(plr._pName, pPack->pName);
	plr._pLvlChanging = pPack->pLvlChanging;
	plr._pDunLevel = pPack->pDunLevel;
	plr._pClass = pPack->pClass;
	plr._pLevel = pPack->pLevel;
	plr._pRank = pPack->pRank;
	plr._pTeam = pPack->pTeam;
	plr._pStatPts = pPack->pStatPts;
	// plr._pLightRad = pPack->pLightRad;
	// plr._pManaShield = pPack->pManaShield;
	// plr._pTimer[PLTR_INFRAVISION] = pPack->pTimer[PLTR_INFRAVISION];
	// plr._pTimer[PLTR_RAGE] = pPack->pTimer[PLTR_RAGE];
	plr._pExperience = pPack->pExperience;
	plr._pBaseStr = pPack->pBaseStr;
	plr._pBaseMag = pPack->pBaseMag;
	plr._pBaseDex = pPack->pBaseDex;
	plr._pBaseVit = pPack->pBaseVit;
	plr._pMaxHPBase = pPack->pMaxHPBase;
	plr._pHPBase = pPack->pHPBase;
	plr._pMaxManaBase = pPack->pMaxManaBase;
	plr._pManaBase = pPack->pManaBase;

	/*memcpy(plr._pAtkSkillHotKey, pPack->pAtkSkillHotKey, sizeof(plr._pAtkSkillHotKey));
	memcpy(plr._pAtkSkillTypeHotKey, pPack->pAtkSkillTypeHotKey, sizeof(plr._pAtkSkillTypeHotKey));
	memcpy(plr._pMoveSkillHotKey, pPack->pMoveSkillHotKey, sizeof(plr._pMoveSkillHotKey));
	memcpy(plr._pMoveSkillTypeHotKey, pPack->pMoveSkillTypeHotKey, sizeof(plr._pMoveSkillTypeHotKey));
	memcpy(plr._pAltAtkSkillHotKey, pPack->pAltAtkSkillHotKey, sizeof(plr._pAltAtkSkillHotKey));
	memcpy(plr._pAltAtkSkillTypeHotKey, pPack->pAltAtkSkillTypeHotKey, sizeof(plr._pAltAtkSkillTypeHotKey));
	memcpy(plr._pAltMoveSkillHotKey, pPack->pAltMoveSkillHotKey, sizeof(plr._pAltMoveSkillHotKey));
	memcpy(plr._pAltMoveSkillTypeHotKey, pPack->pAltMoveSkillTypeHotKey, sizeof(plr._pAltMoveSkillTypeHotKey));
	memcpy(plr._pAltAtkSkillSwapKey, pPack->pAltAtkSkillSwapKey, sizeof(plr._pAltAtkSkillSwapKey));
	memcpy(plr._pAltAtkSkillTypeSwapKey, pPack->pAltAtkSkillTypeSwapKey, sizeof(plr._pAltAtkSkillTypeSwapKey));
	memcpy(plr._pAltMoveSkillSwapKey, pPack->pAltMoveSkillSwapKey, sizeof(plr._pAltMoveSkillSwapKey));
	memcpy(plr._pAltMoveSkillTypeSwapKey, pPack->pAltMoveSkillTypeSwapKey, sizeof(plr._pAltMoveSkillTypeSwapKey));*/
	static_assert(offsetof(PlayerStruct, _pAtkSkillTypeHotKey) == offsetof(PlayerStruct, _pAtkSkillHotKey) + sizeof(plr._pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance I.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillHotKey) == offsetof(PlayerStruct, _pAtkSkillTypeHotKey) + sizeof(plr._pAtkSkillTypeHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance II.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillTypeHotKey) == offsetof(PlayerStruct, _pMoveSkillHotKey) + sizeof(plr._pMoveSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance III.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillHotKey) == offsetof(PlayerStruct, _pMoveSkillTypeHotKey) + sizeof(plr._pMoveSkillTypeHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance IV.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillTypeHotKey) == offsetof(PlayerStruct, _pAltAtkSkillHotKey) + sizeof(plr._pAltAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance V.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillHotKey) == offsetof(PlayerStruct, _pAltAtkSkillTypeHotKey) + sizeof(plr._pAltAtkSkillTypeHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance VI.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillTypeHotKey) == offsetof(PlayerStruct, _pAltMoveSkillHotKey) + sizeof(plr._pAltMoveSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance VII.");
	static_assert(offsetof(PlayerStruct, _pAtkSkillSwapKey) == offsetof(PlayerStruct, _pAltMoveSkillTypeHotKey) + sizeof(plr._pAltMoveSkillTypeHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance VIII.");
	static_assert(offsetof(PlayerStruct, _pAtkSkillTypeSwapKey) == offsetof(PlayerStruct, _pAtkSkillSwapKey) + sizeof(plr._pAtkSkillSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XI.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillSwapKey) == offsetof(PlayerStruct, _pAtkSkillTypeSwapKey) + sizeof(plr._pAtkSkillTypeSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance X.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillTypeSwapKey) == offsetof(PlayerStruct, _pMoveSkillSwapKey) + sizeof(plr._pMoveSkillSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XI.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillSwapKey) == offsetof(PlayerStruct, _pMoveSkillTypeSwapKey) + sizeof(plr._pMoveSkillTypeSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XII.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillTypeSwapKey) == offsetof(PlayerStruct, _pAltAtkSkillSwapKey) + sizeof(plr._pAltAtkSkillSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XIII.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillSwapKey) == offsetof(PlayerStruct, _pAltAtkSkillTypeSwapKey) + sizeof(plr._pAltAtkSkillTypeSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XIV.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillTypeSwapKey) == offsetof(PlayerStruct, _pAltMoveSkillSwapKey) + sizeof(plr._pAltMoveSkillSwapKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XV.");
	static_assert(sizeof(plr._pAtkSkillHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XVI.");
	static_assert(sizeof(plr._pAtkSkillTypeHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XVII.");
	static_assert(sizeof(plr._pMoveSkillHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XVIII.");
	static_assert(sizeof(plr._pMoveSkillTypeHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XIX.");
	static_assert(sizeof(plr._pAltAtkSkillHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XX.");
	static_assert(sizeof(plr._pAltAtkSkillTypeHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXI.");
	static_assert(sizeof(plr._pAltMoveSkillHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXII.");
	static_assert(sizeof(plr._pAltMoveSkillTypeHotKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXIII.");
	static_assert(sizeof(plr._pAtkSkillSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXIV.");
	static_assert(sizeof(plr._pAtkSkillTypeSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXV.");
	static_assert(sizeof(plr._pMoveSkillSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXVI.");
	static_assert(sizeof(plr._pMoveSkillTypeSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXVII.");
	static_assert(sizeof(plr._pAltAtkSkillSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXVIII.");
	static_assert(sizeof(plr._pAltAtkSkillTypeSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXIX.");
	static_assert(sizeof(plr._pAltMoveSkillSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXX.");
	static_assert(sizeof(plr._pAltMoveSkillTypeSwapKey[0]) == 1, "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXII.");
	static_assert(offsetof(PlayerStruct, _pAtkSkillTypeHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAtkSkillTypeHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXIII.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pMoveSkillHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXIV.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillTypeHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pMoveSkillTypeHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXV.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltAtkSkillHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXVI.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillTypeHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltAtkSkillTypeHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXVII.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltMoveSkillHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXVIII.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillTypeHotKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltMoveSkillTypeHotKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXIX.");
	static_assert(offsetof(PlayerStruct, _pAtkSkillSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAtkSkillSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXX.");
	static_assert(offsetof(PlayerStruct, _pAtkSkillTypeSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAtkSkillTypeSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXI.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pMoveSkillSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXII.");
	static_assert(offsetof(PlayerStruct, _pMoveSkillTypeSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pMoveSkillTypeSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXIII.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltAtkSkillSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXIV.");
	static_assert(offsetof(PlayerStruct, _pAltAtkSkillTypeSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltAtkSkillTypeSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXV.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltMoveSkillSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXVI.");
	static_assert(offsetof(PlayerStruct, _pAltMoveSkillTypeSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) == offsetof(PkPlayerStruct, pAltMoveSkillTypeSwapKey) - offsetof(PkPlayerStruct, pAtkSkillHotKey), "(Un)PackPlayer uses DWORD-memcpy to optimize performance XXXXVII.");
	memcpy(plr._pAtkSkillHotKey, pPack->pAtkSkillHotKey, offsetof(PlayerStruct, _pAltMoveSkillTypeSwapKey) - offsetof(PlayerStruct, _pAtkSkillHotKey) + sizeof(plr._pAltMoveSkillTypeSwapKey));

	static_assert(sizeof(plr._pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy V.");
	static_assert(sizeof(pPack->pSkillLvlBase[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VI.");
	memcpy(plr._pSkillLvlBase, pPack->pSkillLvlBase, sizeof(pPack->pSkillLvlBase));
	static_assert(sizeof(plr._pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VII.");
	static_assert(sizeof(pPack->pSkillActivity[0]) == 1, "Big vs. Little-Endian requires a byte-by-byte copy VIII.");
	memcpy(plr._pSkillActivity, pPack->pSkillActivity, sizeof(pPack->pSkillActivity));
	static_assert(NUM_SPELLS <= 64, "UnPacking of PkPlayerStruct is no longer compatible.");
	for (i = 0; i < 64; i++) {
		plr._pSkillExp[i] = pPack->pSkillExp[i];
	}
	plr._pMemSkills = pPack->pMemSkills;

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
	plr._pGFXLoad = 0;
	plr._pmode = PM_NEWLVL; // used by CalcPlrInv
	// commented out, because these should not matter
	//plr._pDestAction = ACTION_NONE;
	//plr._pInvincible = 0;
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
		if (TWOHAND_WIELD(&plr, pi)) {
			// - right hand must be empty when wielding a two handed weapon
			net_assert(plr._pInvBody[INVLOC_HAND_RIGHT]._itype == ITYPE_NONE);
		} else {
			// - allow only shield or one handed weapon in right hand with weapon in the left hand
			pi = &plr._pInvBody[INVLOC_HAND_RIGHT];
			net_assert(pi->_itype == ITYPE_NONE || pi->_itype == ITYPE_SHIELD
			 || (pi->_iClass == ICLASS_WEAPON && !TWOHAND_WIELD(&plr, pi)));
		}
	}
#endif /* INET_MODE */

	// recalculate the cached fields
	InitPlayer(pnum);
	CalcPlrInv(pnum, false);
}

DEVILUTION_END_NAMESPACE
