#include <string>
#include <fstream>

#include "diabloui.h"
#include "selok.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
static int hashCount;
static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_DONE = 100;

// base mapflags set in the corresponding .AMP file (only the lower byte is used)
#define MAPFLAG_TYPE      0x00FF
#define MAPFLAG_VERTDOOR  0x0100
#define MAPFLAG_HORZDOOR  0x0200
#define MAPFLAG_VERTARCH  0x0400
#define MAPFLAG_HORZARCH  0x0800
#define MAPFLAG_VERTGRATE 0x1000
#define MAPFLAG_HORZGRATE 0x2000

typedef enum filenames {
	FILE_TOWN_MIN,
	FILE_TOWN_CEL,
	FILE_CATHEDRAL_MIN,
	FILE_CATHEDRAL_SOL,
	FILE_CATACOMBS_AMP,
	FILE_CAVES_MIN,
	FILE_CAVES_SOL,
	FILE_HELL_SOL,
	FILE_HELL_AMP,
	FILE_BHSM_TRN,
	FILE_BSM_TRN,
	FILE_ACIDB_TRN,
	FILE_ACIDBLK_TRN,
	FILE_BLKKNTBE_TRN,
	FILE_DARK_TRN,
	FILE_BLUE_TRN,
	FILE_FATB_TRN,
	FILE_GARGB_TRN,
	FILE_GARGBR_TRN,
	FILE_GRAY_TRN,
	FILE_CNSELBK_TRN,
	FILE_CNSELG_TRN,
	FILE_GUARD_TRN,
	FILE_VTEXL_TRN,
	FILE_RHINOB_TRN,
	FILE_BLACK_TRN,
	FILE_WHITE_TRN,
	FILE_THINV1_TRN,
	FILE_GREY_TRN,
#ifdef HELLFIRE
	FILE_NTOWN_CEL,
	FILE_NTOWN_MIN,
	FILE_CRYPT_TIL,
	FILE_CRYPT_MIN,
	FILE_CRYPT_SOL,
	FILE_NEST_MIN,
	FILE_NEST_SOL,
	FILE_OBJCURS_CEL,
#endif
	NUM_FILENAMES
} filenames;

static const char* const filesToPatch[NUM_FILENAMES] = {
/*FILE_TOWN_MIN*/      "Levels\\TownData\\Town.MIN",
/*FILE_TOWN_CEL*/      "Levels\\TownData\\Town.CEL",
/*FILE_CATHEDRAL_MIN*/ "Levels\\L1Data\\L1.MIN",
/*FILE_CATHEDRAL_SOL*/ "Levels\\L1Data\\L1.SOL",
/*FILE_CATACOMBS_AMP*/ "Levels\\L2Data\\L2.AMP",
/*FILE_CAVES_MIN*/     "Levels\\L3Data\\L3.MIN",
/*FILE_CAVES_SOL*/     "Levels\\L3Data\\L3.SOL",
/*FILE_HELL_SOL*/      "Levels\\L4Data\\L4.SOL",
/*FILE_HELL_AMP*/      "Levels\\L4Data\\L4.AMP",
/*FILE_BHSM_TRN*/      "Monsters\\Monsters\\BHSM.TRN",
/*FILE_BSM_TRN*/       "Monsters\\Monsters\\BSM.TRN",
/*FILE_ACIDB_TRN*/     "Monsters\\Acid\\AcidB.TRN",
/*FILE_ACIDBLK_TRN*/   "Monsters\\Acid\\AcidBlk.TRN",
/*FILE_BLKKNTBE_TRN*/  "Monsters\\Black\\BlkKntBe.TRN",
/*FILE_DARK_TRN*/      "Monsters\\FalSpear\\Dark.TRN",
/*FILE_BLUE_TRN*/      "Monsters\\Fat\\Blue.TRN",
/*FILE_FATB_TRN*/      "Monsters\\Fat\\FatB.TRN",
/*FILE_GARGB_TRN*/     "Monsters\\Gargoyle\\GargB.TRN",
/*FILE_GARGBR_TRN*/    "Monsters\\Gargoyle\\GargBr.TRN",
/*FILE_GRAY_TRN*/      "Monsters\\GoatMace\\Gray.TRN",
/*FILE_CNSELBK_TRN*/   "Monsters\\Mage\\Cnselbk.TRN",
/*FILE_CNSELG_TRN*/    "Monsters\\Mage\\Cnselg.TRN",
/*FILE_GUARD_TRN*/     "Monsters\\Mega\\Guard.TRN",
/*FILE_VTEXL_TRN*/     "Monsters\\Mega\\Vtexl.TRN",
/*FILE_RHINOB_TRN*/    "Monsters\\Rhino\\RhinoB.TRN",
/*FILE_BLACK_TRN*/     "Monsters\\SkelSd\\Black.TRN",
/*FILE_WHITE_TRN*/     "Monsters\\SkelSd\\White.TRN",
/*FILE_THINV1_TRN*/    "Monsters\\Thin\\Thinv1.TRN",
/*FILE_GREY_TRN*/      "Monsters\\Zombie\\Grey.TRN",
#ifdef HELLFIRE
/*FILE_NTOWN_CEL*/     "NLevels\\TownData\\Town.CEL",
/*FILE_NTOWN_MIN*/     "NLevels\\TownData\\Town.MIN",
/*FILE_CRYPT_TIL*/     "NLevels\\L5Data\\L5.TIL",
/*FILE_CRYPT_MIN*/     "NLevels\\L5Data\\L5.MIN",
/*FILE_CRYPT_SOL*/     "NLevels\\L5Data\\L5.SOL",
/*FILE_NEST_MIN*/      "NLevels\\L6Data\\L6.MIN",
/*FILE_NEST_SOL*/      "NLevels\\L6Data\\L6.SOL",
/*FILE_OBJCURS_CEL*/   "Data\\Inv\\Objcurs.CEL",
#endif
};

#define DESCRIPTION_WIDTH (SELGAME_LPANEL_WIDTH - 2 * 10)

#define MICRO_IDX(subtile, blockSize, microIndex) ((subtile) * (blockSize) + (blockSize) - (2 + ((microIndex) & ~1)) + ((microIndex) & 1))
/*#define blkMicro(subtileRef, blockSize, microIndex, value) \
{ \
	assert((pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] & 0xFFF) == SwapLE16(value) || pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] == SwapLE16(0)); \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}*/
#define blkMicro(subtileRef, microIndex) \
{ \
	pSubtiles[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}

#define nSolidTable(pn, v) \
if (v) { \
   buf[pn - 1] |= PFLAG_BLOCK_PATH; \
} else { \
   buf[pn - 1] &= ~PFLAG_BLOCK_PATH; \
}

#define nMissileTable(pn, v) \
if (v) { \
   buf[pn - 1] |= PFLAG_BLOCK_MISSILE; \
} else { \
   buf[pn - 1] &= ~PFLAG_BLOCK_MISSILE; \
}

#define nBlockTable(pn, v) \
if (v) { \
   buf[pn - 1] |= PFLAG_BLOCK_LIGHT; \
} else { \
   buf[pn - 1] &= ~PFLAG_BLOCK_LIGHT; \
}

static void patchTownFile(BYTE* buf)
{
	// pointless tree micros (re-drawn by dSpecial)
	uint16_t *pSubtiles = (uint16_t*)buf;
	constexpr int blockSize = 16;
	blkMicro(117, 3);
	blkMicro(117, 5);
	blkMicro(128, 2);
	blkMicro(128, 3);
	blkMicro(128, 4);
	blkMicro(128, 5);
	blkMicro(128, 6);
	blkMicro(128, 7);
	blkMicro(129, 3);
	blkMicro(129, 5);
	blkMicro(129, 7);
	blkMicro(130, 2);
	blkMicro(130, 4);
	blkMicro(130, 6);
	blkMicro(156, 2);
	blkMicro(156, 3);
	blkMicro(156, 4);
	blkMicro(156, 5);
	blkMicro(156, 6);
	blkMicro(156, 7);
	blkMicro(156, 8);
	blkMicro(156, 9);
	blkMicro(156, 10);
	blkMicro(156, 11);
	blkMicro(157, 3);
	blkMicro(157, 5);
	blkMicro(157, 7);
	blkMicro(157, 9);
	blkMicro(157, 11);
	blkMicro(158, 2);
	blkMicro(158, 4);
	blkMicro(160, 2);
	blkMicro(160, 3);
	blkMicro(160, 4);
	blkMicro(160, 5);
	blkMicro(160, 6);
	blkMicro(160, 7);
	blkMicro(160, 8);
	blkMicro(160, 9);
	blkMicro(162, 2);
	blkMicro(162, 4);
	blkMicro(162, 6);
	blkMicro(162, 8);
	blkMicro(162, 10);
	blkMicro(212, 3);
	blkMicro(212, 4);
	blkMicro(212, 5);
	blkMicro(212, 6);
	blkMicro(212, 7);
	blkMicro(212, 8);
	blkMicro(212, 9);
	blkMicro(212, 10);
	blkMicro(212, 11);
	// blkMicro(214, 4);
	// blkMicro(214, 6);
	blkMicro(216, 2);
	blkMicro(216, 4);
	blkMicro(216, 6);
	// blkMicro(217, 4);
	// blkMicro(217, 6);
	// blkMicro(217, 8);
	// blkMicro(358, 4);
	// blkMicro(358, 5);
	// blkMicro(358, 6);
	// blkMicro(358, 7);
	// blkMicro(358, 8);
	// blkMicro(358, 9);
	// blkMicro(358, 10);
	// blkMicro(358, 11);
	// blkMicro(358, 12);
	// blkMicro(358, 13);
	// blkMicro(360, 4);
	// blkMicro(360, 6);
	// blkMicro(360, 8);
	// blkMicro(360, 10);
	// fix bad artifact
	blkMicro(233, 6);
	// useless black micros
	blkMicro(426, 1);
	blkMicro(427, 0);
	blkMicro(427, 1);
	blkMicro(429, 1);
	// fix bad artifacts
	blkMicro(828, 12);
	blkMicro(828, 13);
	blkMicro(1018, 2);
	// useless black micros
	blkMicro(1143, 0);
	blkMicro(1145, 0);
	blkMicro(1145, 1);
	blkMicro(1146, 0);
	blkMicro(1153, 0);
	blkMicro(1155, 1);
	blkMicro(1156, 0);
	blkMicro(1169, 1);
	blkMicro(1170, 0);
	blkMicro(1170, 1);
	blkMicro(1172, 1);
	blkMicro(1176, 1);
	blkMicro(1199, 1);
	blkMicro(1200, 0);
	blkMicro(1200, 1);
	blkMicro(1202, 1);
	blkMicro(1203, 1);
	blkMicro(1205, 1);
	blkMicro(1212, 0);
	blkMicro(1219, 0);
}

static BYTE* patchFile(int index, size_t *dwLen)
{
	BYTE* buf = LoadFileInMem(filesToPatch[index], dwLen);
	if (buf == NULL) {
		app_warn("Unable to open file %s in the mpq.", filesToPatch[index]);
		return NULL;
	}

	switch (index) {
	case FILE_TOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(1219 - 1, 16, 0) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		patchTownFile(buf);
#endif
	} break;
	case FILE_TOWN_CEL:
#ifdef HELLFIRE
	case FILE_NTOWN_CEL:
#endif
	{
#if ASSET_MPL == 1
		if (*dwLen < 3547 * 4) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// patch dMicroCels - TOWN.CEL
		// - overwrite subtile 557 and 558 with subtile 939 and 940 to make the inner tile of Griswold's house non-walkable
		BYTE *pMicrosCel = buf;
		memcpy(&pMicrosCel[SwapLE32(((DWORD*)pMicrosCel)[557])], &pMicrosCel[SwapLE32(((DWORD*)pMicrosCel)[939])], SwapLE32(((DWORD*)pMicrosCel)[940]) - SwapLE32(((DWORD*)pMicrosCel)[939]));
		memcpy(&pMicrosCel[SwapLE32(((DWORD*)pMicrosCel)[558])], &pMicrosCel[SwapLE32(((DWORD*)pMicrosCel)[940])], SwapLE32(((DWORD*)pMicrosCel)[941]) - SwapLE32(((DWORD*)pMicrosCel)[940]));
#endif
	} break;
	case FILE_CATHEDRAL_MIN:
	{	// patch dMiniTiles - L1.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(140 - 1, 10, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		// useless black micros
		blkMicro(107, 0);
		blkMicro(107, 1);
		blkMicro(109, 1);
		blkMicro(137, 1);
		blkMicro(138, 0);
		blkMicro(138, 1);
		blkMicro(140, 1);
#endif /* ASSET_MPL == 1 */
	} break;
	case FILE_CATHEDRAL_SOL:
	{	// patch dSolidTable - L1.SOL
		if (*dwLen <= 8) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(8, false); // the only column which was blocking missiles
	} break;
	case FILE_CATACOMBS_AMP:
	{	// patch dAutomapData - L2.AMP
		if (*dwLen < 157 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *automaptype = (uint16_t*)buf;
		automaptype[42 - 1] &= SwapLE16(~MAPFLAG_HORZARCH);
		automaptype[156 - 1] &= SwapLE16(~(MAPFLAG_VERTDOOR | MAPFLAG_TYPE));
		automaptype[157 - 1] &= SwapLE16(~(MAPFLAG_HORZDOOR | MAPFLAG_TYPE));
	} break;
	case FILE_CAVES_MIN:
	{	// patch dMiniTiles - L3.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(82 - 1, 10, 4) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		// fix bad artifact
		blkMicro(82, 4);
#endif /* ASSET_MPL == 1 */
	} break;
	case FILE_CAVES_SOL:
	{	// patch dSolidTable - L3.SOL
		if (*dwLen <= 249) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(249, false); // sync tile 68 and 69 by making subtile 249 of tile 68 walkable.
	} break;
	case FILE_HELL_SOL:
	{	// patch dSolidTable - L4.SOL
		if (*dwLen <= 211) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(141, false); // fix missile-blocking tile of down-stairs.
		// nMissileTable(137, false); // fix missile-blocking tile of down-stairs. - skip to keep in sync with the nSolidTable
		// nSolidTable(137, false);   // fix non-walkable tile of down-stairs. - skip, because it causes a graphic glitch
		nSolidTable(130, true);    // make the inner tiles of the down-stairs non-walkable I.
		nSolidTable(132, true);    // make the inner tiles of the down-stairs non-walkable II.
		nSolidTable(131, true);    // make the inner tiles of the down-stairs non-walkable III.
		nSolidTable(133, true);    // make the inner tiles of the down-stairs non-walkable IV.
		// fix all-blocking tile on the diablo-level
		nSolidTable(211, false);
		nMissileTable(211, false);
		nBlockTable(211, false);
	} break;
	case FILE_HELL_AMP:
	{	// patch dAutomapData - L4.AMP
		if (*dwLen < 56 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *automaptype = (uint16_t*)buf;
		automaptype[52 - 1] |= SwapLE16(MAPFLAG_VERTGRATE);
		automaptype[56 - 1] |= SwapLE16(MAPFLAG_HORZGRATE);
	} break;
	case FILE_BHSM_TRN:
	{	// patch TRN for 'Blighthorn Steelmace' - BHSM.TRN
		if (*dwLen != 256) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// assert(buf[188] == 255 || buf[188] == 0);
		buf[188] = 0;
	} break;
	case FILE_BSM_TRN:
	{	// patch TRN for 'Baron Sludge' - BSM.TRN
		if (*dwLen != 256) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// assert(buf[241] == 255 || buf[241] == 0);
		buf[241] = 0;
	} break;
	case FILE_ACIDB_TRN:
	case FILE_ACIDBLK_TRN:
	case FILE_BLKKNTBE_TRN:
	case FILE_DARK_TRN:
	case FILE_BLUE_TRN:
	case FILE_FATB_TRN:
	case FILE_GARGB_TRN:
	case FILE_GARGBR_TRN:
	case FILE_GRAY_TRN:
	case FILE_CNSELBK_TRN:
	case FILE_CNSELG_TRN:
	case FILE_GUARD_TRN:
	case FILE_VTEXL_TRN:
	case FILE_RHINOB_TRN:
	case FILE_BLACK_TRN:
	case FILE_WHITE_TRN:
	case FILE_THINV1_TRN:
	case FILE_GREY_TRN:
	{	// patch TRN for normal monsters - ...TRN
		if (*dwLen != 256) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		for (int i = 0; i < NUM_COLORS - 1; i++) {
			if (buf[i] == 255)
				buf[i] = 0;
		}
	} break;
#ifdef HELLFIRE
	case FILE_NTOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(1303 - 1, 16, 7) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq. File len: %d vs %d", filesToPatch[index], *dwLen, MICRO_IDX(1303 - 1, 16, 7) * 2);
			return NULL;
		}
		patchTownFile(buf);
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 16;
		// fix bad artifacts
		blkMicro(1273, 7);
		blkMicro(1303, 7);
#endif // ASSET_MPL
	} break;
	case FILE_NEST_MIN:
	{	// patch dMiniTiles - L6.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(366 - 1, 10, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		// useless black micros
		blkMicro(21, 0);
		blkMicro(21, 1);
		// fix bad artifacts
		blkMicro(132, 7);
		blkMicro(366, 1);
#endif /* ASSET_MPL == 1 */
	} break;
	case FILE_NEST_SOL:
	{	// patch dSolidTable - L6.SOL
		if (*dwLen <= 416) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(390, false); // make a pool tile walkable I.
		nSolidTable(413, false); // make a pool tile walkable II.
		nSolidTable(416, false); // make a pool tile walkable III.
	} break;
	case FILE_CRYPT_TIL:
	{	// patch dMegaTiles - L5.TIL
		if (*dwLen < (4 * (72 - 1) + 2) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// use common subtiles of doors
		uint16_t *pMegaTiles = (uint16_t*)buf;
		assert(pMegaTiles[4 * (71 - 1) + 2] == SwapLE16(213 - 1) || pMegaTiles[4 * (71 - 1) + 2] ==  SwapLE16(206 - 1));
		pMegaTiles[4 * (71 - 1) + 2] = SwapLE16(206 - 1);
		assert(pMegaTiles[4 * (72 - 1) + 2] == SwapLE16(216 - 1) || pMegaTiles[4 * (72 - 1) + 2] ==  SwapLE16(206 - 1));
		pMegaTiles[4 * (72 - 1) + 2] = SwapLE16(206 - 1);
	} break;
	case FILE_CRYPT_MIN:
	{	// patch dMiniTiles - L5.MIN
#if ASSET_MPL == 1
		if (*dwLen < MICRO_IDX(197 - 1, 10, 1) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// pointless door micros (re-drawn by dSpecial)
		uint16_t *pSubtiles = (uint16_t*)buf;
		constexpr int blockSize = 10;
		blkMicro(77, 6);
		blkMicro(77, 8);
		blkMicro(80, 7);
		blkMicro(80, 9);
		blkMicro(206, 6);
		blkMicro(206, 8);
		blkMicro(209, 7);
		blkMicro(209, 9);
		blkMicro(213, 6);
		blkMicro(213, 8);
		blkMicro(216, 6);
		blkMicro(216, 8);
		// useless black micros
		blkMicro(130, 0);
		blkMicro(130, 1);
		blkMicro(132, 1);
		blkMicro(134, 0);
		blkMicro(134, 1);
		blkMicro(149, 0);
		blkMicro(149, 1);
		blkMicro(149, 2);
		blkMicro(150, 0);
		blkMicro(150, 1);
		blkMicro(150, 2);
		blkMicro(150, 4);
		blkMicro(151, 0);
		blkMicro(151, 1);
		blkMicro(151, 3);
		blkMicro(152, 0);
		blkMicro(152, 1);
		blkMicro(152, 3);
		blkMicro(152, 5);
		blkMicro(153, 0);
		blkMicro(153, 1);
		// fix bad artifact
		blkMicro(156, 2);
		// useless black micros
		blkMicro(172, 0);
		blkMicro(172, 1);
		blkMicro(172, 2);
		blkMicro(173, 0);
		blkMicro(173, 1);
		blkMicro(174, 0);
		blkMicro(174, 1);
		blkMicro(174, 2);
		blkMicro(174, 4);
		blkMicro(175, 0);
		blkMicro(175, 1);
		blkMicro(176, 0);
		blkMicro(176, 1);
		blkMicro(176, 3);
		blkMicro(177, 0);
		blkMicro(177, 1);
		blkMicro(177, 3);
		blkMicro(177, 5);
		blkMicro(178, 0);
		blkMicro(178, 1);
		blkMicro(179, 0);
		blkMicro(179, 1);
#endif // ASSET_MPL
	} break;
	case FILE_CRYPT_SOL:
	{ // patch dSolidTable - L5.SOL
		if (*dwLen <= 600) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(143, false); // make right side of down-stairs consistent (walkable)
		nSolidTable(148, false); // make the back of down-stairs consistent (walkable)
		// make collision-checks more reasonable
		//  - prevent non-crossable floor-tile configurations I.
		nSolidTable(461, false);
		//  - set top right tile of an arch non-walkable (full of lava)
		//nSolidTable(471, true);
		//  - set top right tile of a pillar walkable (just a small obstacle)
		nSolidTable(481, false);
		//  - tile 491 is the same as tile 594 which is not solid
		//  - prevents non-crossable floor-tile configurations
		nSolidTable(491, false);
		//  - set bottom left tile of a rock non-walkable (rather large obstacle, feet of the hero does not fit)
		//  - prevents non-crossable floor-tile configurations
		nSolidTable(523, true);
		//  - set the top right tile of a floor mega walkable (similar to 594 which is not solid)
		nSolidTable(570, false);
		//  - prevent non-crossable floor-tile configurations II.
		nSolidTable(598, false);
		nSolidTable(600, false);
	} break;
	case FILE_OBJCURS_CEL:
	{
		size_t sizeB, sizeAB;
		BYTE *aCursCels, *bCursCels;
		DWORD numA, numAB;

		numA = SwapLE32(((DWORD*)buf)[0]);
		if (numA != 179) {
			mem_free_dbg(buf);
			if (numA != 179 + 61 - 2) {
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			}
			return NULL;
		}
		bCursCels = LoadFileInMem("Data\\Inv\\Objcurs2.CEL", &sizeB);
		// merge the two cel files
		aCursCels = buf;
		buf = CelMerge(aCursCels, *dwLen, bCursCels, sizeB);

		*dwLen += sizeB - 4 * 2;

		mem_free_dbg(aCursCels);
		mem_free_dbg(bCursCels);

		// remove the last two entries
		numAB = SwapLE32(((DWORD*)buf)[0]) - 2;
		sizeAB = SwapLE32(((DWORD*)buf)[numAB + 1]) - 4 * 2;
		aCursCels = DiabloAllocPtr(sizeAB);
		*(DWORD*)aCursCels = SwapLE32(numAB);
		for (unsigned i = 0; i < numAB + 1; i++) {
			((DWORD*)aCursCels)[i + 1] = SwapLE32(((DWORD*)buf)[i + 1]) - 4 * 2;
		}
		memcpy(aCursCels + (numAB + 2) * 4, buf + (numAB + 2 + 2) * 4 , sizeAB - (numAB + 2) * 4);

		mem_free_dbg(buf);
		buf = aCursCels;
		*dwLen = sizeAB;
	} break;
#endif // HELLFIRE
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return buf;
}

static int patcher_callback()
{
	switch (workPhase) {
	case 0:
	{	// first round - read the content and prepare the metadata
		std::string listpath = std::string(GetBasePath()) + "mpqfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}
		std::string line;
		int entryCount = lengthof(filesToPatch);
		while (std::getline(input, line)) {
			for (int i = 0; i < NUM_MPQS; i++) {
				//if (diabdat_mpqs[i] != NULL && SFileHasFile(diabdat_mpqs[i], line.c_str())) {
				if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_CHECK_EXISTS, NULL)) {
					entryCount++;
					break;
				}
			}
		}

		if (entryCount == 0) {
			// app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}

		// calculate the required number of hashes
		// TODO: use GetNearestPowerOfTwo of StormCommon.h?
		hashCount = 1;
		while (hashCount <= entryCount) {
			hashCount <<= 1;
		}
		workPhase++;
	} break;
	case 1:
	{	// create the mpq file
		std::string path = std::string(GetBasePath()) + "devilx.mpq.foo";
		if (!OpenMPQ(path.c_str(), hashCount, hashCount)) {
			app_warn("Unable to open MPQ file %s.", path.c_str());
			return RETURN_ERROR;
		}
		hashCount = 0;
		workPhase++;
	} break;
	case 2:
	{	// add the current content of devilx.mpq
		std::string listpath = std::string(GetBasePath()) + "mpqfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}		
		int skip = hashCount;
		std::string line;
		while (std::getline(input, line)) {
			if (--skip >= 0) {
				continue;
			}
			if (skip <= -10) {
				break;
			}
			for (int i = 0; i < NUM_MPQS; i++) {
				HANDLE hFile;
				if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_FROM_MPQ, &hFile)) {
					DWORD dwLen = SFileGetFileSize(hFile);
					BYTE* buf = DiabloAllocPtr(dwLen);
					if (!SFileReadFile(hFile, buf, dwLen)) {
						app_warn("Unable to open file %s in the mpq.", line.c_str());
						return RETURN_ERROR;
					}
					if (!mpqapi_write_entry(line.c_str(), buf, dwLen)) {
						app_warn("Unable to write %s to the MPQ.", line.c_str());
						return RETURN_ERROR;
					}
					mem_free_dbg(buf);
					SFileCloseFile(hFile);
					break;
				}
			}
			hashCount++;
		}
		input.close();
		if (skip <= -10)
			break;
		hashCount = 0;
		workPhase++;
	} break;
	case 3:
	{	// add patches
		int i = hashCount;
		for ( ; i < lengthof(filesToPatch); i++) {
			if (i >= hashCount + 10) {
				break;
			}
			size_t dwLen;
			BYTE* buf = patchFile(i, &dwLen);
			if (buf == NULL) {
				return RETURN_ERROR;
			}
			if (!mpqapi_write_entry(filesToPatch[i], buf, dwLen)) {
				app_warn("Unable to write %s to the MPQ.", filesToPatch[i]);
				return RETURN_ERROR;
			}
			mem_free_dbg(buf);
		}
		hashCount += 10;
		if (i >= hashCount)
			break;
		mpqapi_flush_and_close(true);
		workPhase++;
	} break;
	case 4:
	{	// replace the devilx.mpq with the new file
		std::string pathTo = std::string(GetBasePath()) + "devilx.mpq";
		if (diabdat_mpqs[MPQ_DEVILX] != NULL) {
			SFileCloseArchive(diabdat_mpqs[MPQ_DEVILX]);
			diabdat_mpqs[MPQ_DEVILX] = NULL;
			RemoveFile(pathTo.c_str());
		}
		std::string pathFrom = std::string(GetBasePath()) + "devilx.mpq.foo";
		RenameFile(pathFrom.c_str(), pathTo.c_str());
		if (FileExists(pathFrom.c_str())) {
			app_warn("Failed to rename %s to %s.", pathFrom.c_str(), pathTo.c_str());
			return RETURN_ERROR;
		}
		workPhase++;
	} break;
	case 5:
	{	// test the result
		std::string path = std::string(GetBasePath()) + "devilx.mpq";
		diabdat_mpqs[MPQ_DEVILX] = SFileOpenArchive(path.c_str(), MPQ_OPEN_READ_ONLY);
		if (diabdat_mpqs[MPQ_DEVILX] == NULL) {
			app_warn("Failed to create %s.", path.c_str());
			return RETURN_ERROR;
		}
	} return RETURN_DONE;
	}

	while (++workProgress >= 100)
		workProgress -= 100;
	return workProgress;
}

void UiPatcherDialog()
{
	workProgress = 0;
	workPhase = 0;
	bool result = UiProgressDialog("...Patch in progress...", patcher_callback);

	if (!result) {
		return;
	}

	if (diabdat_mpqs[NUM_MPQS] != NULL) {
		char dialogTitle[32];
		char dialogText[256];
		snprintf(dialogTitle, sizeof(dialogTitle), "");
		snprintf(dialogText, sizeof(dialogText), "The base assets are patched. The file of the merged assets needs to be recreated.");

		UiSelOkDialog(dialogTitle, dialogText);
	}
}

DEVILUTION_END_NAMESPACE
