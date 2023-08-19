#include <string>
#include <fstream>
#include <set>

#include "diabloui.h"
#include "selok.h"
#include "utils/paths.h"
#include "utils/file_util.h"
#include "engine/render/cel_render.h"
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
static HANDLE mpqone;
static int hashCount;
static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_DONE = 100;

typedef enum filenames {
#if ASSET_MPL == 1
	FILE_TOWN_CEL,
	FILE_TOWN_MIN,
	FILE_CATHEDRAL_CEL,
	FILE_CATHEDRAL_MIN,
#endif
	FILE_CATHEDRAL_TIL,
	FILE_CATHEDRAL_SOL,
	FILE_BONESTR1_DUN,
	FILE_BONESTR2_DUN,
	FILE_BONECHA1_DUN,
	FILE_BONECHA2_DUN,
	FILE_BLIND1_DUN,
	FILE_BLIND2_DUN,
	FILE_BLOOD1_DUN,
	FILE_BLOOD2_DUN,
#if ASSET_MPL == 1
	FILE_L2DOORS_CEL,
	FILE_CATACOMBS_SCEL,
	FILE_CATACOMBS_CEL,
	FILE_CATACOMBS_MIN,
#endif
	FILE_CATACOMBS_TIL,
	FILE_CATACOMBS_SOL,
	FILE_FOULWATR_DUN,
#if ASSET_MPL == 1
	FILE_L3DOORS_CEL,
	FILE_CAVES_CEL,
	FILE_CAVES_MIN,
#endif
	FILE_CAVES_TIL,
	FILE_CAVES_SOL,
	FILE_DIAB1_DUN,
	FILE_DIAB2A_DUN,
	FILE_DIAB2B_DUN,
	FILE_DIAB3A_DUN,
	FILE_DIAB3B_DUN,
	FILE_DIAB4A_DUN,
	FILE_DIAB4B_DUN,
	FILE_VILE1_DUN,
	FILE_WARLORD_DUN,
	FILE_WARLORD2_DUN,
#if ASSET_MPL == 1
	FILE_HELL_CEL,
	FILE_HELL_MIN,
#endif
	FILE_HELL_TIL,
	FILE_HELL_SOL,
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
	FILE_PLR_WHBAT,
	FILE_PLR_WLBAT,
	FILE_PLR_WMBAT,
	FILE_PLR_WMHAS,
#ifdef HELLFIRE
#if ASSET_MPL == 1
	FILE_NTOWN_CEL,
	FILE_NTOWN_MIN,
#endif
#if ASSET_MPL == 1
	FILE_CRYPT_CEL,
	FILE_CRYPT_MIN,
#endif
	FILE_CRYPT_TIL,
	FILE_CRYPT_SOL,
#if ASSET_MPL == 1
	FILE_NEST_CEL,
	FILE_NEST_MIN,
#endif
	FILE_NEST_TIL,
	FILE_NEST_SOL,
	FILE_OBJCURS_CEL,
#endif
	NUM_FILENAMES
} filenames;

static const char* const filesToPatch[NUM_FILENAMES] = {
#if ASSET_MPL == 1
/*FILE_TOWN_CEL*/      "Levels\\TownData\\Town.CEL",
/*FILE_TOWN_MIN*/      "Levels\\TownData\\Town.MIN",
/*FILE_CATHEDRAL_CEL*/ "Levels\\L1Data\\L1.CEL",
/*FILE_CATHEDRAL_MIN*/ "Levels\\L1Data\\L1.MIN",
#endif
/*FILE_CATHEDRAL_TIL*/ "Levels\\L1Data\\L1.TIL",
/*FILE_CATHEDRAL_SOL*/ "Levels\\L1Data\\L1.SOL",
/*FILE_BONESTR1_DUN*/  "Levels\\L2Data\\Bonestr1.DUN",
/*FILE_BONESTR2_DUN*/  "Levels\\L2Data\\Bonestr2.DUN",
/*FILE_BONECHA1_DUN*/  "Levels\\L2Data\\Bonecha1.DUN",
/*FILE_BONECHA2_DUN*/  "Levels\\L2Data\\Bonecha2.DUN",
/*FILE_BLIND1_DUN*/    "Levels\\L2Data\\Blind1.DUN",
/*FILE_BLIND2_DUN*/    "Levels\\L2Data\\Blind2.DUN",
/*FILE_BLOOD1_DUN*/    "Levels\\L2Data\\Blood1.DUN",
/*FILE_BLOOD2_DUN*/    "Levels\\L2Data\\Blood2.DUN",
#if ASSET_MPL == 1
/*FILE_L2DOORS_CEL*/   "Objects\\L2Doors.CEL",
/*FILE_CATACOMBS_SCEL*/"Levels\\L2Data\\L2S.CEL",
/*FILE_CATACOMBS_CEL*/ "Levels\\L2Data\\L2.CEL",
/*FILE_CATACOMBS_MIN*/ "Levels\\L2Data\\L2.MIN",
#endif
/*FILE_CATACOMBS_TIL*/ "Levels\\L2Data\\L2.TIL",
/*FILE_CATACOMBS_SOL*/ "Levels\\L2Data\\L2.SOL",
/*FILE_FOULWATR_DUN*/  "Levels\\L3Data\\Foulwatr.DUN",
#if ASSET_MPL == 1
/*FILE_L3DOORS_CEL*/   "Objects\\L3Doors.CEL",
/*FILE_CAVES_CEL*/     "Levels\\L3Data\\L3.CEL",
/*FILE_CAVES_MIN*/     "Levels\\L3Data\\L3.MIN",
#endif
/*FILE_CAVES_TIL*/     "Levels\\L3Data\\L3.TIL",
/*FILE_CAVES_SOL*/     "Levels\\L3Data\\L3.SOL",
/*FILE_DIAB1_DUN*/     "Levels\\L4Data\\Diab1.DUN",
/*FILE_DIAB2A_DUN*/    "Levels\\L4Data\\Diab2a.DUN",
/*FILE_DIAB2B_DUN*/    "Levels\\L4Data\\Diab2b.DUN",
/*FILE_DIAB3A_DUN*/    "Levels\\L4Data\\Diab3a.DUN",
/*FILE_DIAB3B_DUN*/    "Levels\\L4Data\\Diab3b.DUN",
/*FILE_DIAB4A_DUN*/    "Levels\\L4Data\\Diab4a.DUN",
/*FILE_DIAB4B_DUN*/    "Levels\\L4Data\\Diab4b.DUN",
/*FILE_VILE1_DUN*/     "Levels\\L4Data\\Vile1.DUN",
/*FILE_WARLORD_DUN*/   "Levels\\L4Data\\Warlord.DUN",
/*FILE_WARLORD2_DUN*/  "Levels\\L4Data\\Warlord2.DUN",
#if ASSET_MPL == 1
/*FILE_HELL_CEL*/      "Levels\\L4Data\\L4.CEL",
/*FILE_HELL_MIN*/      "Levels\\L4Data\\L4.MIN",
#endif
/*FILE_HELL_TIL*/      "Levels\\L4Data\\L4.TIL",
/*FILE_HELL_SOL*/      "Levels\\L4Data\\L4.SOL",
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
/*FILE_PLR_WHBAT*/     "PlrGFX\\Warrior\\WHB\\WHBAT.CL2",
/*FILE_PLR_WLBAT*/     "PlrGFX\\Warrior\\WLB\\WLBAT.CL2",
/*FILE_PLR_WMBAT*/     "PlrGFX\\Warrior\\WMB\\WMBAT.CL2",
/*FILE_PLR_WMHAS*/     "PlrGFX\\Warrior\\WMH\\WMHAS.CL2",
#ifdef HELLFIRE
#if ASSET_MPL == 1
/*FILE_NTOWN_CEL*/     "NLevels\\TownData\\Town.CEL",
/*FILE_NTOWN_MIN*/     "NLevels\\TownData\\Town.MIN",
#endif
#if ASSET_MPL == 1
/*FILE_CRYPT_CEL*/     "NLevels\\L5Data\\L5.CEL",
/*FILE_CRYPT_MIN*/     "NLevels\\L5Data\\L5.MIN",
#endif
/*FILE_CRYPT_TIL*/     "NLevels\\L5Data\\L5.TIL",
/*FILE_CRYPT_SOL*/     "NLevels\\L5Data\\L5.SOL",
#if ASSET_MPL == 1
/*FILE_NEST_CEL*/      "NLevels\\L6Data\\L6.CEL",
/*FILE_NEST_MIN*/      "NLevels\\L6Data\\L6.MIN",
#endif
/*FILE_NEST_TIL*/      "NLevels\\L6Data\\L6.TIL",
/*FILE_NEST_SOL*/      "NLevels\\L6Data\\L6.SOL",
/*FILE_OBJCURS_CEL*/   "Data\\Inv\\Objcurs.CEL",
#endif
};

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

static BYTE* buildBlkCel(BYTE* celBuf, size_t *celLen)
{
	removeMicros.erase(0);

	if (removeMicros.empty()) {
		return celBuf;
	}
	// create the new CEL file
	BYTE* resCelBuf = DiabloAllocPtr(*celLen);

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	DWORD srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;
	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	DWORD dstCelEntries = srcCelEntries - removeMicros.size();
	dstHeaderCursor[0] = SwapLE32(dstCelEntries);
	dstHeaderCursor++;
	BYTE* dstDataCursor = resCelBuf + 4 * (dstCelEntries + 2);
	while (!removeMicros.empty()) {
		// select the next frame
		unsigned nextRef = *removeMicros.begin();
		removeMicros.erase(nextRef);

		// copy entries till the next frame
		int numEntries = nextRef - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (int i = 0; i < numEntries; i++) {
			dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
			dstHeaderCursor++;
			DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
			memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
			dstDataCursor += len;
			srcHeaderCursor++;
		}

		// skip the original frame
		srcHeaderCursor++;
	}
	// add remaining entries
	int numEntries = srcCelEntries + 1 - ((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
	for (int i = 0; i < numEntries; i++) {
		dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;
	}
	// add file-size
	dstHeaderCursor[0] = SwapLE32((size_t)dstDataCursor - (size_t)resCelBuf);

	*celLen = SwapLE32(dstHeaderCursor[0]);

	mem_free_dbg(celBuf);

	return resCelBuf;
}

static BYTE* buildBlkMin(BYTE* minBuf, size_t *minLen, unsigned blockSize)
{
	removeMicros.erase(0);

	if (removeMicros.empty()) {
		return minBuf;
	}

	uint16_t* pSubtiles = (uint16_t*)minBuf;

	for (unsigned i = 0; i < *minLen / 2; i++) {
		if (pSubtiles[i] == 0) {
			continue;
		}
		unsigned frameRef = SwapLE16(pSubtiles[i]) & 0xFFF;
		unsigned newFrameRef = frameRef;
		for (unsigned removedRef : removeMicros) {
			if (removedRef > frameRef)
				continue;
			if (removedRef == frameRef) {
				removeMicros.clear();
				mem_free_dbg(minBuf);
				app_warn("Frame %d is removed, but it is still used by subtile (%d).", frameRef, (i / blockSize) + 1);
				return NULL;
			}
			newFrameRef--;
		}
		pSubtiles[i] = SwapLE16((SwapLE16(pSubtiles[i]) & ~0xFFF) | newFrameRef);
	}
	removeMicros.clear();
	return minBuf;
}

static void patchDungeon(int fileIndex, BYTE* fileBuf, size_t* fileSize)
{
	uint16_t* lm = (uint16_t*)fileBuf;

	// TODO: validate file-size
	switch (fileIndex) {
	case FILE_BONESTR1_DUN:
	{	// patch premap - Bonestr1.DUN
		// useless tiles
		lm[2 + 0 + 0 * 7] = 0;
		lm[2 + 0 + 4 * 7] = 0;
		lm[2 + 0 + 5 * 7] = 0;
		lm[2 + 0 + 6 * 7] = 0;
		lm[2 + 6 + 6 * 7] = 0;
		lm[2 + 6 + 0 * 7] = 0;
		lm[2 + 2 + 3 * 7] = 0;
		lm[2 + 3 + 3 * 7] = 0;
		// + eliminate obsolete stair-tile
		lm[2 + 2 + 4 * 7] = 0;
		// shadow of the external-left column
		lm[2 + 0 + 4 * 7] = SwapLE16(48);
		lm[2 + 0 + 5 * 7] = SwapLE16(50);
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 1; y < 6; y++) {
			for (int x = 1; x < 6; x++) {
				lm[2 + 7 * 7 + x + y * 7] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
	} break;
	case FILE_BONESTR2_DUN:
	{	// patch the map - Bonestr2.DUN
		// useless tiles
		lm[2 + 0 + 0 * 7] = 0;
		lm[2 + 0 + 6 * 7] = 0;
		lm[2 + 6 + 6 * 7] = 0;
		lm[2 + 6 + 0 * 7] = 0;
		// add the separate pillar tile
		lm[2 + 5 + 5 * 7] = SwapLE16(52);
		// add tiles with subtiles for arches
		lm[2 + 2 + 1 * 7] = SwapLE16(45);
		lm[2 + 4 + 1 * 7] = SwapLE16(45);
		lm[2 + 2 + 5 * 7] = SwapLE16(45);
		lm[2 + 4 + 5 * 7] = SwapLE16(45);
		lm[2 + 1 + 2 * 7] = SwapLE16(44);
		lm[2 + 1 + 4 * 7] = SwapLE16(44);
		lm[2 + 5 + 2 * 7] = SwapLE16(44);
		lm[2 + 5 + 4 * 7] = SwapLE16(44);
		// - remove tile to leave space for shadow
		lm[2 + 2 + 4 * 7] = 0;
		// protect the main structure
		for (int y = 1; y < 6; y++) {
			for (int x = 1; x < 6; x++) {
				lm[2 + 7 * 7 + x + y * 7] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 7 * 7 + 7 * 7 * 2 * 2) * 2;
	} break;
	case FILE_BONECHA1_DUN:
	{	// patch premap - Bonecha1.DUN
		// external tiles
		lm[2 + 20 +  4 * 32] = 12;
		lm[2 + 21 +  4 * 32] = 12;
		// useless tiles
		for (int y = 0; y < 18; y++) {
			for (int x = 0; x < 32; x++) {
				if (x >= 13 && x <= 21 && y >= 1 && y <= 4) {
					continue;
				}
				if (x == 18 && y == 5) {
					continue;
				}
				if (x == 14 && y == 5) {
					continue;
				}
				lm[2 + x + y * 32] = 0;
			}
		}
		// remove rooms
		*fileSize = (2 + 32 * 18 + 32 * 18 * 2 * 2 + 32 * 18 * 2 * 2 + 32 * 18 * 2 * 2) * 2;
	} break;
	case FILE_BONECHA2_DUN:
	{	// patch the map - Bonecha2.DUN
		// reduce pointless bone-chamber complexity
		lm[2 + 16 + 9 * 32] = SwapLE16(57);
		lm[2 + 16 + 10 * 32] = SwapLE16(62);
		lm[2 + 16 + 11 * 32] = SwapLE16(62);
		lm[2 + 16 + 12 * 32] = SwapLE16(62);
		lm[2 + 13 + 12 * 32] = SwapLE16(53);
		lm[2 + 14 + 12 * 32] = SwapLE16(62);
		lm[2 + 15 + 12 * 32] = SwapLE16(62);
		// external tiles
		lm[2 + 2 + 15 * 32] = SwapLE16(11);
		lm[2 + 3 + 15 * 32] = SwapLE16(11);
		lm[2 + 4 + 15 * 32] = SwapLE16(11);
		lm[2 + 5 + 15 * 32] = SwapLE16(11);
		lm[2 + 6 + 15 * 32] = SwapLE16(11);
		lm[2 + 7 + 15 * 32] = SwapLE16(11);
		lm[2 + 8 + 15 * 32] = SwapLE16(11);

		lm[2 + 10 + 17 * 32] = SwapLE16(11);
		lm[2 + 11 + 17 * 32] = SwapLE16(11);
		lm[2 + 12 + 17 * 32] = SwapLE16(11);
		lm[2 + 13 + 17 * 32] = SwapLE16(15);
		lm[2 + 14 + 17 * 32] = SwapLE16(11);
		lm[2 + 15 + 17 * 32] = SwapLE16(11);
		lm[2 + 16 + 17 * 32] = SwapLE16(11);
		lm[2 + 17 + 17 * 32] = SwapLE16(15);
		lm[2 + 18 + 17 * 32] = SwapLE16(11);
		lm[2 + 19 + 17 * 32] = SwapLE16(11);
		lm[2 + 20 + 17 * 32] = SwapLE16(11);
		lm[2 + 21 + 17 * 32] = SwapLE16(16);
		lm[2 + 21 + 16 * 32] = SwapLE16(10);
		lm[2 + 21 + 15 * 32] = SwapLE16(10);
		lm[2 + 21 + 14 * 32] = SwapLE16(10);

		lm[2 + 20 + 0 * 32] = SwapLE16(12);
		lm[2 + 21 + 0 * 32] = SwapLE16(12);
		lm[2 + 21 + 1 * 32] = SwapLE16(14);
		lm[2 + 21 + 2 * 32] = SwapLE16(10);
		lm[2 + 21 + 3 * 32] = SwapLE16(10);
		lm[2 + 21 + 4 * 32] = SwapLE16(10);
		lm[2 + 21 + 5 * 32] = SwapLE16(14);
		lm[2 + 21 + 6 * 32] = SwapLE16(10);
		lm[2 + 21 + 7 * 32] = SwapLE16(10);
		lm[2 + 21 + 8 * 32] = SwapLE16(10);

		lm[2 + 31 + 8 * 32] = SwapLE16(10);
		lm[2 + 31 + 9 * 32] = SwapLE16(10);
		lm[2 + 31 + 10 * 32] = SwapLE16(10);
		lm[2 + 31 + 11 * 32] = SwapLE16(10);
		lm[2 + 31 + 12 * 32] = SwapLE16(10);
		lm[2 + 31 + 13 * 32] = SwapLE16(10);
		lm[2 + 31 + 14 * 32] = SwapLE16(10);
		lm[2 + 31 + 15 * 32] = SwapLE16(16);
		lm[2 + 24 + 15 * 32] = SwapLE16(11);
		lm[2 + 25 + 15 * 32] = SwapLE16(11);
		lm[2 + 26 + 15 * 32] = SwapLE16(11);
		lm[2 + 27 + 15 * 32] = SwapLE16(11);
		lm[2 + 28 + 15 * 32] = SwapLE16(11);
		lm[2 + 29 + 15 * 32] = SwapLE16(11);
		lm[2 + 30 + 15 * 32] = SwapLE16(11);

		lm[2 + 21 + 13 * 32] = SwapLE16(13);
		lm[2 + 22 + 13 * 32] = SwapLE16(11);

		lm[2 + 8 + 15 * 32] = SwapLE16(11);
		lm[2 + 8 + 16 * 32] = SwapLE16(12);
		lm[2 + 8 + 17 * 32] = SwapLE16(12);
		lm[2 + 9 + 17 * 32] = SwapLE16(15);

		// add tiles with subtiles for arches
		lm[2 + 13 + 6 * 32] = SwapLE16(44);
		lm[2 + 13 + 8 * 32] = SwapLE16(44);
		lm[2 + 17 + 6 * 32] = SwapLE16(44);
		lm[2 + 17 + 8 * 32] = SwapLE16(96);

		lm[2 + 13 + 14 * 32] = SwapLE16(44);
		lm[2 + 13 + 16 * 32] = SwapLE16(44);
		lm[2 + 17 + 14 * 32] = SwapLE16(44);
		lm[2 + 17 + 16 * 32] = SwapLE16(44);

		lm[2 + 18 + 9 * 32] = SwapLE16(45);
		lm[2 + 20 + 9 * 32] = SwapLE16(45);
		lm[2 + 18 + 13 * 32] = SwapLE16(45);
		lm[2 + 20 + 13 * 32] = SwapLE16(45);

		// place pieces with closed doors
		lm[2 + 17 + 11 * 32] = SwapLE16(150);
		// place shadows
		// - right corridor
		lm[2 + 12 + 6 * 32] = SwapLE16(47);
		lm[2 + 12 + 7 * 32] = SwapLE16(51);
		lm[2 + 16 + 6 * 32] = SwapLE16(47);
		lm[2 + 16 + 7 * 32] = SwapLE16(51);
		lm[2 + 16 + 8 * 32] = SwapLE16(47);
		// - central room (top)
		// lm[2 + 17 + 8 * 32] = SwapLE16(96);
		lm[2 + 18 + 8 * 32] = SwapLE16(49);
		lm[2 + 19 + 8 * 32] = SwapLE16(49);
		lm[2 + 20 + 8 * 32] = SwapLE16(49);
		// - central room (bottom)
		lm[2 + 18 + 12 * 32] = SwapLE16(46);
		// lm[2 + 19 + 12 * 32] = SwapLE16(49); -- ugly with the candle
		// - left corridor
		lm[2 + 12 + 14 * 32] = SwapLE16(47);
		lm[2 + 12 + 15 * 32] = SwapLE16(51);
		lm[2 + 16 + 14 * 32] = SwapLE16(47);
		lm[2 + 16 + 15 * 32] = SwapLE16(51);
		// remove monsters, objects, items
		*fileSize = (2 + 32 * 18 + 32 * 18 * 2 * 2) * 2;
	} break;
	case FILE_BLIND1_DUN:
	{	// patch the map - Blind1.DUN
		// place pieces with closed doors
		lm[2 + 4 + 3 * 11] = SwapLE16(150);
		lm[2 + 6 + 7 * 11] = SwapLE16(150);
		// remove obsolete 'protection' (item)
		// lm[2 + 11 * 11 + 5 + 10 * 11] = 0;
		// protect the main structure
		for (int y = 0; y < 7; y++) {
			for (int x = 0; x < 7; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16(3);
			}
		}
		for (int y = 4; y < 11; y++) {
			for (int x = 4; x < 11; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 11 * 11 + 11 * 11 * 2 * 2) * 2;
	} break;
	case FILE_BLIND2_DUN:
	{	// patch premap - Blind2.DUN
		// external tiles
		lm[2 + 2 + 2 * 11] = SwapLE16(13);
		lm[2 + 2 + 3 * 11] = SwapLE16(10);
		lm[2 + 3 + 2 * 11] = SwapLE16(11);
		lm[2 + 3 + 3 * 11] = SwapLE16(12);

		lm[2 + 6 + 6 * 11] = SwapLE16(13);
		lm[2 + 6 + 7 * 11] = SwapLE16(10);
		lm[2 + 7 + 6 * 11] = SwapLE16(11);
		lm[2 + 7 + 7 * 11] = SwapLE16(12);
		// useless tiles
		for (int y = 0; y < 11; y++) {
			for (int x = 0; x < 11; x++) {
				// keep the boxes
				if (x >= 2 && y >= 2 && x < 4 && y < 4) {
					continue;
				}
				if (x >= 6 && y >= 6 && x < 8 && y < 8) {
					continue;
				}
				// keep the doors
				if (x == 0 && y == 1/* || x == 4 && y == 3*/ || x == 10 && y == 8) {
					continue;
				}
				lm[2 + x + y * 11] = 0;
			}
		}
		// replace the door with wall
		lm[2 + 4 + 3 * 11] = SwapLE16(25);
		// remove obsolete 'protection' (item)
		// lm[2 + 11 * 11 + 5 + 10 * 11] = 0;
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 0; y < 6; y++) {
			for (int x = 0; x < 6; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		for (int y = 4; y < 11; y++) {
			for (int x = 4; x < 11; x++) {
				lm[2 + 11 * 11 + x + y * 11] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// remove rooms
		*fileSize = (2 + 11 * 11 + 11 * 11 * 2 * 2 + 11 * 11 * 2 * 2 + 11 * 11 * 2 * 2) * 2;
	} break;
	case FILE_BLOOD1_DUN:
	{	// patch the map - Blood1.DUN
		// eliminate invisible 'fancy' tile to leave space for shadow
		lm[2 + 3 + 9 * 10] = 0;
		// - place pieces with closed doors
		lm[2 + 4 + 10 * 10] = SwapLE16(151);
		lm[2 + 4 + 15 * 10] = SwapLE16(151);
		lm[2 + 5 + 15 * 10] = SwapLE16(151);
		// protect the main structure
		for (int y = 0; y <= 15; y++) {
			for (int x = 2; x <= 7; x++) {
				lm[2 + 10 * 16 + x + y * 10] = SwapLE16(3);
			}
		}
		for (int y = 3; y <= 8; y++) {
			for (int x = 0; x <= 9; x++) {
				lm[2 + 10 * 16 + x + y * 10] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 10 * 16 + 10 * 16 * 2 * 2) * 2;
	} break;
	case FILE_BLOOD2_DUN:
	{	// patch premap - Blood2.DUN
		// external tiles
		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 10; x++) {
				uint16_t wv = SwapLE16(lm[2 + x + y * 10]);
				if (wv >= 143 && wv <= 149) {
					lm[2 + x + y * 10] = SwapLE16(wv - 133);
				}
			}
		}
		// useless tiles
		for (int y = 8; y < 16; y++) {
			for (int x = 0; x < 10; x++) {
				lm[2 + x + y * 10] = 0;
			}
		}
		// - place pieces with closed doors
		// lm[2 + 4 + 10 * 10] = SwapLE16(151);
		// lm[2 + 4 + 15 * 10] = SwapLE16(151);
		// lm[2 + 5 + 15 * 10] = SwapLE16(151);
		// shadow of the external-left column -- do not place to prevent overwriting large decorations
		//dungeon[pSetPieces[0]._spx - 1][pSetPieces[0]._spy + 7] = 48;
		//dungeon[pSetPieces[0]._spx - 1][pSetPieces[0]._spy + 8] = 50;
		// - shadow of the bottom-left column(s) -- one is missing
		// lm[2 + 1 + 13 * 10] = SwapLE16(48);
		// lm[2 + 1 + 14 * 10] = SwapLE16(50);
		// - shadow of the internal column next to the pedistal
		lm[2 + 5 + 7 * 10] = SwapLE16(142);
		lm[2 + 5 + 8 * 10] = SwapLE16(50);
		// remove 'items'
		lm[2 + 10 * 16 + 9 + 2 * 10 * 2] = 0;
		// adjust objects
		// - add book and pedistal
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 9 + 24 * 10 * 2] = SwapLE16(15);
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 9 + 16 * 10 * 2] = SwapLE16(91);
		// - remove torches
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 8 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 10 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 11 + 12 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 8 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 10 * 10 * 2] = 0;
		lm[2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 6 + 12 * 10 * 2] = 0;
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 7; y < 15; y++) {
			for (int x = 2; x <= 6; x++) {
				lm[2 + 10 * 16 + x + y * 10] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// remove rooms
		*fileSize = (2 + 10 * 16 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2 + 10 * 16 * 2 * 2) * 2;
	} break;
	case FILE_FOULWATR_DUN:
	{	// patch the map - Foulwatr.DUN
		// - separate subtiles for the automap
		lm[2 + 6 + 33 * 19] = SwapLE16(111);
		// protect island tiles from spawning additional monsters
		for (int y = 1; y < 7; y++) {
			for (int x = 7; x < 14; x++) {
				lm[2 + 19 * 37 + x + y * 19] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// remove most of the monsters
		for (int y = 13; y < 61; y++) {
			for (int x = 4; x < 30; x++) {
				if (x == 6 && y == 33) {
					continue;
				}
				lm[2 + 19 * 37 + 19 * 37 * 2 * 2 + x + y * 19 * 2] = 0;
			}
		}
		// remove rooms
		*fileSize = (2 + 19 * 37 + 19 * 37 * 2 * 2 + 19 * 37 * 2 * 2 + 19 * 37 * 2 * 2) * 2;
	} break;
	case FILE_DIAB1_DUN:
	{	// patch the map - Diab1.DUN
		// - fix shadow of the left corner
		lm[2 + 0 + 4 * 6] = SwapLE16(75);
		lm[2 + 0 + 5 * 6] = SwapLE16(74);
		// - fix shadow of the right corner
		lm[2 + 4 + 1 * 6] = SwapLE16(131);
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 6 * 6 + 1 + 1 * 6] = SwapLE16((3 << 14));
		lm[2 + 6 * 6 + 1 + 4 * 6] = SwapLE16((3 << 14));
		lm[2 + 6 * 6 + 2 + 3 * 6] = SwapLE16((3 << 12));
		lm[2 + 6 * 6 + 3 + 3 * 6] = SwapLE16((3 << 14));
		lm[2 + 6 * 6 + 3 + 2 * 6] = SwapLE16((3 << 10));
		lm[2 + 6 * 6 + 4 + 1 * 6] = SwapLE16((3 << 14));
		lm[2 + 6 * 6 + 4 + 4 * 6] = SwapLE16((3 << 14));
		lm[2 + 6 * 6 + 2 + 2 * 6] = SwapLE16((3 << 14));
		// protect tiles with monsters/objects from decoration
		lm[2 + 6 * 6 + 1 + 4 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 2 + 3 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 1 + 1 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 3 + 2 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 3 + 3 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 4 + 4 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 4 + 1 * 6] |= SwapLE16(3);
		lm[2 + 6 * 6 + 2 + 2 * 6] |= SwapLE16(3);
		// remove rooms
		*fileSize = (2 + 6 * 6 + 6 * 6 * 2 * 2 + 6 * 6 * 2 * 2 + 6 * 6 * 2 * 2) * 2;
	} break;
	case FILE_DIAB2A_DUN:
	{	// patch premap - Diab2a.DUN
		// external tiles
		for (int y = 1; y <= 3; y++) {
			for (int x = 9; x <= 10; x++) {
				uint16_t wv = SwapLE16(lm[2 + x + y * 11]);
				if (wv >= 116 && wv <= 128) {
					// if (wv == 118) {
					//	wv = 128;
					// }
					lm[2 + x + y * 11] = SwapLE16(wv - 98);
				}
			}
		}
		// useless tiles
		for (int y = 0; y < 12; y++) {
			for (int x = 0; x < 11; x++) {
				if (x >= 9 && x <= 10 && y >= 1 && y <= 3) {
					continue;
				}
				lm[2 + x + y * 11] = 0;
			}
		}
		// protect changing tiles from objects
		lm[2 + 11 * 12 + 10 + 1 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 10 + 1 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 10 + 2 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 10 + 2 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 10 + 3 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 10 + 3 * 11] |= SwapLE16((3 << 14));
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 11 * 12 + 2 + 1 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 2 + 9 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 5 + 4 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 5 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 6 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 12 + 7 + 5 * 11] = SwapLE16((3 << 8));
		lm[2 + 11 * 12 + 8 + 4 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 8 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 7 + 8 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 12 + 8 + 1 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 12 + 8 + 2 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 12 + 6 + 5 * 11] = SwapLE16((3 << 10));
		// remove rooms
		*fileSize = (2 + 11 * 12 + 11 * 12 * 2 * 2 + 11 * 12 * 2 * 2 + 11 * 12 * 2 * 2) * 2;
	} break;
	case FILE_DIAB2B_DUN:
	{	// patch the map - Diab2b.DUN
		// external tiles
		for (int y = 0; y < 12; y++) {
			for (int x = 0; x < 11; x++) {
				uint16_t wv = SwapLE16(lm[2 + x + y * 11]);
				if (wv >= 116 && wv <= 128) {
					// if (wv == 118) {
					//	wv = 128;
					// }
					lm[2 + x + y * 11] = SwapLE16(wv - 98);
				}
			}
		}
		// use base tiles and let the engine decorate the walls
		for (int y = 0; y < 12; y++) {
			for (int x = 0; x < 11; x++) {
				if (y == 1 && (x == 8 || x == 9)) {
					continue; // skip protected tiles
				}
				uint16_t wv = SwapLE16(lm[2 + x + y * 11]);
				if (wv >= 63 && wv <= 70) {
					if (wv == 63 || wv == 65 || wv == 67 || wv == 68) {
						wv = 1;
					} else {
						wv = 2;
					}
					lm[2 + x + y * 11] = SwapLE16(wv);
				}
			}
		}
		// remove shadow to enable optional connection
		lm[2 + 0 + 10 * 11] = 0;
		lm[2 + 0 + 11 * 11] = 0;
		// ensure the changing tiles are reserved
		lm[2 + 11 * 12 + 9 + 1 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 9 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 9 + 3 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 10 + 1 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 10 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 10 + 3 * 11] = SwapLE16(3);
		// protect tiles with monsters/objects from decoration
		lm[2 + 11 * 12 + 2 + 1 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 2 + 9 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 5 + 4 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 5 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 6 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 7 + 5 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 8 + 4 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 8 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 7 + 8 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 8 + 1 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 8 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 12 + 6 + 5 * 11] = SwapLE16(3);
		// remove monsters, objects, items
		*fileSize = (2 + 11 * 12 + 11 * 12 * 2 * 2) * 2;
	} break;
	case FILE_DIAB3A_DUN:
	{	// patch premap - Diab3a.DUN
		// useless tiles
		for (int y = 0; y < 11; y++) {
			for (int x = 0; x < 11; x++) {
				if (x >= 4 && x <= 6 && y >= 10 && y <= 10) {
					continue; // SW-wall
				}
				if (x >= 0 && x <= 0 && y >= 4 && y <= 6) {
					continue; // NW-wall
				}
				if (x >= 4 && x <= 6 && y >= 0 && y <= 0) {
					continue; // NE-wall
				}
				if (x >= 10 && x <= 10 && y >= 4 && y <= 6) {
					continue; // SE-wall
				}
				lm[2 + x + y * 11] = 0;
			}
		}
		// protect changing tiles from objects
		// - SW-wall
		lm[2 + 11 * 11 + 4 + 10 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 4 + 10 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 5 + 10 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 5 + 10 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 6 + 10 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 10 * 11] |= SwapLE16((3 << 14));
		// - NE-wall
		lm[2 + 11 * 11 + 4 + 0 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 4 + 0 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 5 + 0 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 5 + 0 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 6 + 0 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 0 * 11] |= SwapLE16((3 << 14));
		// - NW-wall
		lm[2 + 11 * 11 + 0 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 0 + 4 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 0 + 5 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 0 + 5 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 0 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 0 + 6 * 11] |= SwapLE16((3 << 14));
		// - SE-wall
		lm[2 + 11 * 11 + 10 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 10 + 4 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 10 + 5 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 10 + 5 * 11] |= SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 10 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 10 + 6 * 11] |= SwapLE16((3 << 14));
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 11 * 11 + 0 + 2 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 0 + 7 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 2 + 0 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 2 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 2 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 2 + 9 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 3 + 3 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 3 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 4 + 1 * 11] = SwapLE16((3 << 8));
		lm[2 + 11 * 11 + 4 + 2 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 4 + 7 * 11] = SwapLE16((3 << 8));
		lm[2 + 11 * 11 + 4 + 7 * 11] |= SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 2 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 7 * 11] = SwapLE16((3 << 12));
		lm[2 + 11 * 11 + 6 + 3 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 6 + 6 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 7 + 0 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 7 + 4 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 7 + 6 * 11] = SwapLE16((3 << 10));
		lm[2 + 11 * 11 + 7 + 9 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 9 + 2 * 11] = SwapLE16((3 << 14));
		lm[2 + 11 * 11 + 9 + 7 * 11] = SwapLE16((3 << 14));
		// remove rooms
		*fileSize = (2 + 11 * 11 + 11 * 11 * 2 * 2 + 11 * 11 * 2 * 2 + 11 * 11 * 2 * 2) * 2;
	} break;
	case FILE_DIAB3B_DUN:
	{	// patch the map - Diab3b.DUN
		// external tiles
		lm[2 + 4 + 4 * 11] = SwapLE16(21);
		lm[2 + 4 + 5 * 11] = SwapLE16(18);
		lm[2 + 5 + 4 * 11] = SwapLE16(19);
		lm[2 + 5 + 5 * 11] = SwapLE16(30);
		// remove partial shadow
		lm[2 + 5 + 0 * 11] = SwapLE16(50);
		// remove shadow to enable optional connection
		lm[2 + 1 + 9 * 11] = 0;
		lm[2 + 1 + 10 * 11] = 0;
		// use base tiles and let the engine decorate the walls
		lm[2 + 3 + 10 * 11] = SwapLE16(2);
		lm[2 + 9 + 8 * 11] = SwapLE16(2);
		lm[2 + 8 + 9 * 11] = SwapLE16(1);
		lm[2 + 6 + 5 * 11] = SwapLE16(1);
		lm[2 + 5 + 6 * 11] = SwapLE16(2);
		lm[2 + 10 + 7 * 11] = SwapLE16(1);
		lm[2 + 2 + 1 * 11] = SwapLE16(1);
		lm[2 + 1 + 2 * 11] = SwapLE16(2);
		lm[2 + 0 + 3 * 11] = SwapLE16(1);
		lm[2 + 10 + 3 * 11] = SwapLE16(1);
		// ensure the changing tiles are reserved
		// - SW-wall
		lm[2 + 11 * 11 + 4 + 10 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 5 + 10 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 6 + 10 * 11] = SwapLE16(3);
		// - NE-wall
		lm[2 + 11 * 11 + 4 + 0 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 5 + 0 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 6 + 0 * 11] = SwapLE16(3);
		// - NW-wall
		lm[2 + 11 * 11 + 0 + 4 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 0 + 5 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 0 + 6 * 11] = SwapLE16(3);
		// - SE-wall
		lm[2 + 11 * 11 + 10 + 4 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 10 + 5 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 10 + 6 * 11] = SwapLE16(3);
		// protect tiles with monsters/objects from decoration
		lm[2 + 11 * 11 + 0 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 0 + 7 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 2 + 0 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 2 + 4 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 2 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 2 + 9 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 3 + 3 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 3 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 4 + 1 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 4 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 4 + 7 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 4 + 7 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 6 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 6 + 7 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 6 + 3 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 6 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 7 + 0 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 7 + 4 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 7 + 6 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 7 + 9 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 9 + 2 * 11] = SwapLE16(3);
		lm[2 + 11 * 11 + 9 + 7 * 11] = SwapLE16(3);
		// remove monsters, objects, items
		*fileSize = (2 + 11 * 11 + 11 * 11 * 2 * 2) * 2;
	} break;
	case FILE_DIAB4A_DUN:
	{	// patch premap - Diab4a.DUN
		for (int y = 0; y < 9; y++) {
			for (int x = 0; x < 9; x++) {
				// external tiles
				uint16_t wv = SwapLE16(lm[2 + x + y * 9]);
				if (wv >= 116 && wv <= 128) {
					// if (wv == 118) {
					//	wv = 128;
					// }
					lm[2 + x + y * 9] = SwapLE16(wv - 98);
				}
				// useless tiles
				if (x >= 2 && x <= 6 && y >= 7 && y <= 8) {
					continue; // SW-wall
				}
				if (x >= 0 && x <= 1 && y >= 2 && y <= 6) {
					continue; // NW-wall
				}
				if (x >= 2 && x <= 6 && y >= 0 && y <= 1) {
					continue; // NE-wall
				}
				if (x >= 7 && x <= 8 && y >= 2 && y <= 6) {
					continue; // SE-wall
				}
				lm[2 + x + y * 9] = 0;
			}
		}
		// - replace diablo
		lm[2 + 9 * 9 + 9 * 9 * 2 * 2 + 8 + 8 * 9 * 2] = SwapLE16((UMT_DIABLO + 1) | (1 << 15));
		// - replace the only black knight
		lm[2 + 9 * 9 + 9 * 9 * 2 * 2 + 4 + 6 * 9 * 2] = SwapLE16(101);
		// protect changing tiles from objects
		// - SW-wall
		for (int y = 2; y < 7; y++) {
			lm[2 + 9 * 9 + 8 + y * 9] = SwapLE16((3 << 10) | (3 << 14));
		}
		// - NE-wall
		for (int x = 2; x < 7; x++) {
			lm[2 + 9 * 9 + x + 1 * 9] = SwapLE16((3 << 12) | (3 << 14));
		}
		// - NW-wall
		for (int y = 2; y < 7; y++) {
			lm[2 + 9 * 9 + 1 + y * 9] = SwapLE16((3 << 10) | (3 << 14));
		}
		// - SE-wall
		for (int x = 2; x < 7; x++) {
			lm[2 + 9 * 9 + x + 8 * 9] = SwapLE16((3 << 12) | (3 << 14));
		}
		// protect tiles with monsters/objects from spawning additional monsters/objects
		lm[2 + 9 * 9 + 2 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 2 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 2 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 2 + 6 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 3 + 6 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 4 + 6 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 3 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 5 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 2 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 4 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 5 * 9] = SwapLE16((3 << 8));
		lm[2 + 9 * 9 + 6 + 6 * 9] = SwapLE16((3 << 8));
		// remove rooms
		*fileSize = (2 + 9 * 9 + 9 * 9 * 2 * 2 + 9 * 9 * 2 * 2 + 9 * 9 * 2 * 2) * 2;
	} break;
	case FILE_DIAB4B_DUN:
	{	// patch the map - Diab4b.DUN
		// external tiles
		for (int y = 0; y < 9; y++) {
			for (int x = 0; x < 9; x++) {
				uint16_t wv = SwapLE16(lm[2 + x + y * 9]);
				if (wv >= 116 && wv <= 128) {
					// if (wv == 118) {
					//	wv = 128;
					// }
					lm[2 + x + y * 9] = SwapLE16(wv - 98);
				}
			}
		}
		// ensure the box is not connected to the rest of the dungeon and the changing tiles are reserved + protect inner tiles from decoration
		for (int y = 0; y < 9; y++) {
			for (int x = 0; x < 9; x++) {
				lm[2 + 9 * 9 + x + y * 9] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 9 * 9 + 9 * 9 * 2 * 2) * 2;
	} break;
	case FILE_VILE1_DUN:
	{	// patch the map - Vile1.DUN (L4Data)
		// fix corner
		lm[2 + 5 + 0 * 7] = SwapLE16(16);
		lm[2 + 6 + 1 * 7] = SwapLE16(16);
		lm[2 + 5 + 1 * 7] = SwapLE16(15);
		// use base tiles and decorate the walls randomly
		lm[2 + 0 + 0 * 7] = SwapLE16(9);
		lm[2 + 0 + 6 * 7] = SwapLE16(15);
		lm[2 + 1 + 0 * 7] = SwapLE16(2);
		lm[2 + 2 + 0 * 7] = SwapLE16(2);
		lm[2 + 3 + 0 * 7] = SwapLE16(2);
		lm[2 + 4 + 0 * 7] = SwapLE16(2);
		lm[2 + 1 + 6 * 7] = SwapLE16(2);
		lm[2 + 2 + 6 * 7] = SwapLE16(2);
		lm[2 + 4 + 6 * 7] = SwapLE16(2);
		// lm[2 + 6 + 3 * 7] = SwapLE16(50);
		// add unique monsters
		lm[2 + 7 * 7 + 7 * 7 * 2 * 2 + 3 + 6 * 7 * 2] = SwapLE16((UMT_LAZARUS + 1) | (1 << 15));
		lm[2 + 7 * 7 + 7 * 7 * 2 * 2 + 5 + 3 * 7 * 2] = SwapLE16((UMT_RED_VEX + 1) | (1 << 15));
		lm[2 + 7 * 7 + 7 * 7 * 2 * 2 + 5 + 9 * 7 * 2] = SwapLE16((UMT_BLACKJADE + 1) | (1 << 15));
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 0; y <= 5; y++) {
			for (int x = 0; x <= 5; x++) {
				lm[2 + 7 * 7 + x + y * 7] |= SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// ensure the box is not connected to the rest of the dungeon + protect inner tiles from decoration
		for (int y = 0; y <= 6; y++) {
			for (int x = 0; x <= 6; x++) {
				if (x == 6 && (y == 0 || y == 6)) {
					continue;
				}
				if (x == 0 || y == 0 || x == 6 || y == 6) {
					lm[2 + 7 * 7 + x + y * 7] |= SwapLE16(1);
				} else {
					lm[2 + 7 * 7 + x + y * 7] |= SwapLE16(3);
				}
			}
		}
		// remove rooms
		*fileSize = (2 + 7 * 7 + 7 * 7 * 2 * 2 + 7 * 7 * 2 * 2 + 7 * 7 * 2 * 2) * 2;
	} break;
	case FILE_WARLORD_DUN:
	{	// patch the map - Warlord.DUN
		// fix corner
		lm[2 + 6 + 1 * 8] = SwapLE16(10);
		lm[2 + 6 + 5 * 8] = SwapLE16(10);
		// separate subtiles for the automap
		lm[2 + 1 + 2 * 8] = SwapLE16(136);
		// use base tiles and decorate the walls randomly
		lm[2 + 0 + 0 * 8] = SwapLE16(9);
		lm[2 + 0 + 6 * 8] = SwapLE16(15);
		lm[2 + 1 + 0 * 8] = SwapLE16(2);
		lm[2 + 2 + 0 * 8] = SwapLE16(2);
		lm[2 + 3 + 0 * 8] = SwapLE16(2);
		lm[2 + 4 + 0 * 8] = SwapLE16(2);
		lm[2 + 5 + 0 * 8] = SwapLE16(2);
		lm[2 + 1 + 6 * 8] = SwapLE16(2);
		lm[2 + 2 + 6 * 8] = SwapLE16(2);
		lm[2 + 3 + 6 * 8] = SwapLE16(2);
		lm[2 + 4 + 6 * 8] = SwapLE16(2);
		lm[2 + 5 + 6 * 8] = SwapLE16(2);
		// lm[2 + 6 + 3 * 8] = SwapLE16(50);
		// ensure the changing tiles are protected + protect inner tiles from decoration
		for (int y = 1; y <= 5; y++) {
			for (int x = 1; x <= 7; x++) {
				lm[2 + 8 * 7 + x + y * 8] = SwapLE16(3);
			}
		}
		// remove monsters, objects, items
		*fileSize = (2 + 8 * 7 + 8 * 7 * 2 * 2) * 2;
	} break;
	case FILE_WARLORD2_DUN:
	{	// patch premap - Warlord2.DUN
		// useless tiles
		for (int y = 0; y < 7; y++) {
			for (int x = 0; x < 8; x++) {
				if (x >= 7 && y >= 1 && x <= 7 && y <= 5) {
					continue;
				}
				lm[2 + x + y * 8] = 0;
			}
		}
		// replace monsters
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 2 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 2 + 10 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 13 + 4 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 13 + 9 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 10 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 10 + 10 * 8 * 2] = SwapLE16(100);
		// add monsters
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 6 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 6 + 10 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 11 + 2 * 8 * 2] = SwapLE16(100);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 11 + 10 * 8 * 2] = SwapLE16(100);
		// - add unique
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 6 + 7 * 8 * 2] = SwapLE16((UMT_WARLORD + 1) | (1 << 15));
		// add objects
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 2 + 3 * 8 * 2] = SwapLE16(108);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 2 + 9 * 8 * 2] = SwapLE16(108);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 5 + 2 * 8 * 2] = SwapLE16(109);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 + 2 * 8 * 2] = SwapLE16(109);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 5 + 10 * 8 * 2] = SwapLE16(109);
		lm[2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 + 10 * 8 * 2] = SwapLE16(109);
		// protect inner tiles from spawning additional monsters/objects
		for (int y = 0; y <= 5; y++) {
			for (int x = 0; x <= 6; x++) {
				lm[2 + 8 * 7 + x + y * 8] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// eliminate 'items'
		lm[2 + 8 * 7 + 2 + 6 * 8] = 0;
		lm[2 + 8 * 7 + 8 * 7 + 90] = 0;
		lm[2 + 8 * 7 + 8 * 7 + 109] = 0;
		lm[2 + 8 * 7 + 8 * 7 + 112] = 0;
		// remove rooms
		*fileSize = (2 + 8 * 7 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2 + 8 * 7 * 2 * 2) * 2;
	} break;
	}
}

static BYTE* EncodeCl2(BYTE* pBuf, const BYTE* pSrc, int width, int height, BYTE transparentPixel)
{
	const int RLE_LEN = 4; // number of matching colors to switch from bmp encoding to RLE

	int subHeaderSize = CEL_FRAME_HEADER_SIZE;
	int hs = (height - 1) / CEL_BLOCK_HEIGHT;
	hs = (hs + 1) * sizeof(WORD);
	subHeaderSize = std::max(subHeaderSize, hs);

	bool clipped = true; // frame->isClipped();
	// convert one image to cl2-data
	BYTE* pHeader = pBuf;
	if (clipped) {
		// add CL2 FRAME HEADER
		*(WORD*)&pBuf[0] = SwapLE16(subHeaderSize); // SUB_HEADER_SIZE
		*(DWORD*)&pBuf[2] = 0;
		*(DWORD*)&pBuf[6] = 0;
		pBuf += subHeaderSize;
	}

	BYTE* pHead = pBuf;
	BYTE col, lastCol;
	BYTE colMatches = 0;
	bool alpha = false;
	bool first = true;
	for (int i = 1; i <= height; i++) {
		if (clipped && (i % CEL_BLOCK_HEIGHT) == 1 /*&& (i / CEL_BLOCK_HEIGHT) * 2 < SUB_HEADER_SIZE*/) {
			pHead = pBuf;
			*(WORD*)(&pHeader[(i / CEL_BLOCK_HEIGHT) * 2]) = SwapLE16(pHead - pHeader); // pHead - buf - SUB_HEADER_SIZE;

			colMatches = 0;
			alpha = false;
			// first = true;
		}
		first = true;
		for (int j = 0; j < width; j++, pSrc++) {
			BYTE pixel = *pSrc; // frame->getPixel(j, height - i);
			if (pixel != transparentPixel) {
				// add opaque pixel
				col = pixel;
				if (alpha || first || col != lastCol)
					colMatches = 1;
				else
					colMatches++;
				if (colMatches < RLE_LEN || (char)*pHead <= -127) {
					// bmp encoding
					if (alpha || (char)*pHead <= -65 || first) {
						pHead = pBuf;
						pBuf++;
						colMatches = 1;
					}
					*pBuf = col;
					pBuf++;
				} else {
					// RLE encoding
					if (colMatches == RLE_LEN) {
						memset(pBuf - (RLE_LEN - 1), 0, RLE_LEN - 1);
						*pHead += RLE_LEN - 1;
						if (*pHead != 0) {
							pHead = pBuf - (RLE_LEN - 1);
						}
						*pHead = -65 - (RLE_LEN - 1);
						pBuf = pHead + 1;
						*pBuf = col;
						pBuf++;
					}
				}
				--*pHead;

				lastCol = col;
				alpha = false;
			} else {
				// add transparent pixel
				if (!alpha || (char)*pHead >= 127) {
					pHead = pBuf;
					pBuf++;
				}
				++*pHead;
				alpha = true;
			}
			first = false;
		}
		pSrc -= BUFFER_WIDTH + width;
	}
	return pBuf;
}


static BYTE* ReEncodeCL2(BYTE* cl2Buf, size_t *dwLen, int numGroups, int frameCount, int height, int width)
{
	constexpr BYTE TRANS_COLOR = 1;

	BYTE* resCl2Buf = DiabloAllocPtr(2 * *dwLen);
	memset(resCl2Buf, 0, 2 * *dwLen);

	bool groupped = true;
	int headerSize = 0;
	for (int i = 0; i < numGroups; i++) {
		int ni = frameCount;
		headerSize += 4 + 4 * (ni + 1);
	}
	if (groupped) {
		headerSize += sizeof(DWORD) * numGroups;
	}

	DWORD* hdr = (DWORD*)resCl2Buf;
	if (groupped) {
		// add optional {CL2 GROUP HEADER}
		int offset = numGroups * 4;
		for (int i = 0; i < numGroups; i++, hdr++) {
			hdr[0] = offset;
			int ni = frameCount;
			offset += 4 + 4 * (ni + 1);
		}
	}

	BYTE* pBuf = &resCl2Buf[headerSize];
	for (int ii = 0; ii < numGroups; ii++) {
		int ni = frameCount;
		hdr[0] = SwapLE32(ni);
		hdr[1] = SwapLE32((size_t)pBuf - (size_t)hdr);

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, BUFFER_WIDTH * height);

			Cl2DrawLightTbl(0, height - 1, frameBuf, n, width, 0);
			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((size_t)pBuf - (size_t)hdr);
		}
		hdr += ni + 2;
	}

	*dwLen = (size_t)pBuf - (size_t)resCl2Buf;

	mem_free_dbg(cl2Buf);
	return resCl2Buf;
}

BYTE* createWarriorAnim(BYTE* cl2Buf, size_t *dwLen, const BYTE* atkBuf, const BYTE* stdBuf)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int numGroups = NUM_DIRS;
	constexpr int frameCount = 10;
	constexpr bool groupped = true;
	constexpr int height = 96;
	constexpr int width = 96;

	BYTE* resCl2Buf = DiabloAllocPtr(2 * *dwLen);
	memset(resCl2Buf, 0, 2 * *dwLen);

	int headerSize = 0;
	for (int i = 0; i < numGroups; i++) {
		int ni = frameCount;
		headerSize += 4 + 4 * (ni + 1);
	}
	if (groupped) {
		headerSize += sizeof(DWORD) * numGroups;
	}

	DWORD* hdr = (DWORD*)resCl2Buf;
	if (groupped) {
		// add optional {CL2 GROUP HEADER}
		int offset = numGroups * 4;
		for (int i = 0; i < numGroups; i++, hdr++) {
			hdr[0] = offset;
			int ni = frameCount;
			offset += 4 + 4 * (ni + 1);
		}
	}

	BYTE* pBuf = &resCl2Buf[headerSize];
	for (int ii = 0; ii < numGroups; ii++) {
		int ni = frameCount;
		hdr[0] = SwapLE32(ni);
		hdr[1] = SwapLE32((size_t)pBuf - (size_t)hdr);

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, BUFFER_WIDTH * height);

			if (ii == 1) {
				// draw the stand frame
				const BYTE* stdFrameBuf = CelGetFrameStart(stdBuf, ii);
				// for (int y = 0; y < height; y++) {
				//	memset(&gpBuffer[0 + BUFFER_WIDTH * y], TRANS_COLOR, width);
				// }
				Cl2DrawLightTbl(0, height - 1, stdFrameBuf, n, width, 0);
				// draw the attack frame
				constexpr int atkWidth = 128;
				const BYTE* atkFrameBuf = CelGetFrameStart(atkBuf, ii);
				// for (int y = 0; y < height; y++) {
				//	memset(&gpBuffer[0 + width + BUFFER_WIDTH * y], TRANS_COLOR, atkWidth);
				// }
				Cl2DrawLightTbl(width, height - 1, atkFrameBuf, 1, atkWidth, 0);

				// copy the shield to the stand frame
				unsigned addr = 0 + BUFFER_WIDTH * 0;
				unsigned addr2 = 0 + width + BUFFER_WIDTH * 0;
				int dy = 0;
				switch (n) {
				case 1: dy = 0; break;
				case 2: dy = 1; break;
				case 3: dy = 2; break;
				case 4: dy = 2; break;
				case 5: dy = 3; break;
				case 6: dy = 3; break;
				case 7: dy = 3; break;
				case 8: dy = 2; break;
				case 9: dy = 2; break;
				case 10: dy = 1; break;
				}
				for (int y = 38; y < 66; y++) {
					for (int x = 19; x < 32; x++) {
						if (x == 31 && y >= 60) {
							break;
						}
						BYTE color = gpBuffer[addr2 + x + 17 + y * BUFFER_WIDTH];
						if (color == TRANS_COLOR || color == 0) {
							continue;
						}
						gpBuffer[addr + x + (y + dy) * BUFFER_WIDTH] = color;
					}
				}
				// fix the shadow
				// - main shield
				for (int y = 72; y < 80; y++) {
					for (int x = 12; x < 31; x++) {
						unsigned addr = x + BUFFER_WIDTH * y;
						if (gpBuffer[addr] != TRANS_COLOR) {
							continue;
						}
						if (y < 67 + x / 2 && y > x + 48) {
							gpBuffer[addr] = 0;
						}
					}
				}
				// -  sink effect on the top-left side
				//if (n > 2 && n < 10) {
				if (dy > 1) {
					// if (n >= 5 && n <= 7) {
					if (dy == 3) {
						gpBuffer[addr + 17 + 75 * BUFFER_WIDTH] = 0;
					} else {
						gpBuffer[addr + 15 + 74 * BUFFER_WIDTH] = 0;
					}
				}
				// - sink effect on the top-right side
				// if (n > 2 && n < 10) {
				if (dy > 1) {
					gpBuffer[addr + 27 + 75 * BUFFER_WIDTH] = 0;
					// if (n > 4 && n < 8) {
					if (dy == 3) {
						gpBuffer[addr + 26 + 74 * BUFFER_WIDTH] = 0;
					}
				}
				// - sink effect on the bottom
				// if (n > 1) {
				if (dy != 0) {
					gpBuffer[addr + 28 + 80 * BUFFER_WIDTH] = 0;
				}
				// if (n > 2 && n < 6) {
				if (dy > 1) {
					gpBuffer[addr + 29 + 80 * BUFFER_WIDTH] = 0;
				}
				// if (n > 4 && n < 8) {
				if (dy == 3) {
					gpBuffer[addr + 27 + 80 * BUFFER_WIDTH] = 0;
				}

				// copy the result to the target
				// for (int y = 0; y < height; y++) {
				//	memcpy(&gpBuffer[0 + BUFFER_WIDTH * y], &gpBuffer[0 + width + BUFFER_WIDTH * y], width);
				// }
			} else {
				Cl2DrawLightTbl(0, height - 1, frameBuf, n, width, 0);
			}

			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((size_t)pBuf - (size_t)hdr);
		}
		hdr += ni + 2;
	}

	*dwLen = (size_t)pBuf - (size_t)resCl2Buf;

	mem_free_dbg(cl2Buf);
	return resCl2Buf;
}

static BYTE* patchFile(int index, size_t *dwLen)
{
	BYTE* buf = LoadFileInMem(filesToPatch[index], dwLen);
	if (buf == NULL) {
		app_warn("Unable to open file %s in the mpq.", filesToPatch[index]);
		return NULL;
	}

	switch (index) {
#if ASSET_MPL == 1
	case FILE_TOWN_CEL:
	{	// patch dMicroCels - TOWN.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_TOWN_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_TOWN_MIN]);
			return NULL;
		}
		if (minLen < 1258 * BLOCK_SIZE_TOWN * 2) {
			// mem_free_dbg(buf);
			// app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_TOWN_MIN]);
			// return NULL;
			return buf; // -- assume it is already done
		}
		buf = Town_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			minBuf = Town_PatchMin(minBuf, &minLen, false);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_TOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
		constexpr int blockSize = BLOCK_SIZE_TOWN;
		if (*dwLen < 1258 * blockSize * 2) {
			// mem_free_dbg(buf);
			// app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			// return NULL;
			return buf; // -- assume it is already done
		}
		buf = Town_PatchMin(buf, dwLen, false);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
	case FILE_CATHEDRAL_CEL:
	{	// patch dMicroCels - L1.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CATHEDRAL_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CATHEDRAL_MIN]);
			return NULL;
		}
		if (minLen < 453 * BLOCK_SIZE_L1 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_CATHEDRAL_MIN]);
			return NULL;
		}
		buf = DRLP_L1_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			DRLP_L1_PatchMin(minBuf);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_CATHEDRAL_MIN:
	{	// patch dMiniTiles - L1.MIN
		constexpr int blockSize = BLOCK_SIZE_L1;
		if (*dwLen < 453 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L1_PatchMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_CATHEDRAL_TIL:
	{	// patch dMegaTiles - L1.TIL
		if (*dwLen < 206 * 4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L1_PatchTil(buf);
	} break;
	case FILE_CATHEDRAL_SOL:
	{	// patch dSolidTable - L1.SOL
		if (*dwLen < 453) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// adjust SOL after fixCathedralShadows
		nSolidTable(298, true);
		nSolidTable(304, true);
		// nBlockTable(330, false);
		nBlockTable(334, false);
		nMissileTable(334, false);
		// nBlockTable(339, true);
		// nBlockTable(340, true);
		// nBlockTable(342, true);
		// nBlockTable(343, true);
		// nBlockTable(344, false);
		// - special subtiles for the banner setpiece
		nBlockTable(336, false);
		nMissileTable(336, false);
		nBlockTable(337, false);
		nMissileTable(337, false);
		nBlockTable(338, false);
		nMissileTable(338, false);
		// - special subtile for the vile setmap
		nMissileTable(335, false);
		// - with subtile-based automap
		nBlockTable(139, false);
		nBlockTable(140, false);
		// - subtile for the separate pillar tile
		nBlockTable(61, false);
		nMissileTable(61, false);
	} break;
	case FILE_BONESTR1_DUN:
	case FILE_BONESTR2_DUN:
	case FILE_BONECHA1_DUN:
	case FILE_BONECHA2_DUN:
	case FILE_BLIND1_DUN:
	case FILE_BLIND2_DUN:
	case FILE_BLOOD1_DUN:
	case FILE_BLOOD2_DUN:
	case FILE_FOULWATR_DUN:
	case FILE_DIAB1_DUN:
	case FILE_DIAB2A_DUN:
	case FILE_DIAB2B_DUN:
	case FILE_DIAB3A_DUN:
	case FILE_DIAB3B_DUN:
	case FILE_DIAB4A_DUN:
	case FILE_DIAB4B_DUN:
	case FILE_VILE1_DUN:
	case FILE_WARLORD_DUN:
	case FILE_WARLORD2_DUN:
	{	// patch .DUN
		patchDungeon(index, buf, dwLen);
	} break;
#if ASSET_MPL == 1
	case FILE_L2DOORS_CEL:
	{	// patch L2Doors.CEL
		buf = DRLP_L2_PatchDoors(buf, dwLen);
	} break;
	case FILE_CATACOMBS_SCEL:
	{	// patch pSpecialsCel - L2S.CEL
		buf = DRLP_L2_PatchSpec(buf, dwLen);
	} break;
	case FILE_CATACOMBS_CEL:
	{	// patch dMicroCels - L2.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CATACOMBS_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CATACOMBS_MIN]);
			return NULL;
		}
		if (minLen < 559 * BLOCK_SIZE_L2 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_CATACOMBS_MIN]);
			return NULL;
		}
		buf = DRLP_L2_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			DRLP_L2_PatchMin(minBuf);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_CATACOMBS_MIN:
	{	// patch dMiniTiles - L2.MIN
		constexpr int blockSize = BLOCK_SIZE_L2;
		if (*dwLen < 559 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}

		DRLP_L2_PatchMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif
	case FILE_CATACOMBS_TIL:
	{	// patch dMegaTiles - L2.TIL
		if (*dwLen < 160 * 4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L2_PatchTil(buf);
	} break;
	case FILE_CATACOMBS_SOL:
	{	// patch dSolidTable - L2.SOL
		// add separate tiles and subtiles for the arches III.
		if (*dwLen < 559) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// fix the upstairs IV.
		// - make the back of the stairs non-walkable
		nSolidTable(252, true);
		nBlockTable(252, true);
		nMissileTable(252, true);
		// - make the stair-floor non light-blocker
		nBlockTable(267, false);
		// nBlockTable(559, false);
		// - adjust SOL after patchCatacombsMin
		nSolidTable(268, true);
		nBlockTable(268, true);
		nMissileTable(268, true);
	} break;
#if ASSET_MPL == 1
	case FILE_L3DOORS_CEL:
	{	// patch L3Doors.CEL
		buf = DRLP_L3_PatchDoors(buf, dwLen);
	} break;
	case FILE_CAVES_CEL:
	{	// patch dMicroCels - L3.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CAVES_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CAVES_MIN]);
			return NULL;
		}
		if (minLen < 560 * BLOCK_SIZE_L3 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_CAVES_MIN]);
			return NULL;
		}
		buf = DRLP_L3_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			DRLP_L3_PatchMin(minBuf);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_CAVES_MIN:
	{	// patch dMiniTiles - L3.MIN
		constexpr int blockSize = BLOCK_SIZE_L3;
		if (*dwLen < 560 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L3_PatchMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_CAVES_TIL:
	{	// patch dMegaTiles - L3.TIL
		if (*dwLen < 156 * 4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L3_PatchTil(buf);
	} break;
	case FILE_CAVES_SOL:
	{	// patch dSolidTable - L3.SOL
		if (*dwLen < 560) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nSolidTable(249, false); // sync tile 68 and 69 by making subtile 249 of tile 68 walkable.
		nBlockTable(146, false); // fix unreasonable light-blocker
		nBlockTable(150, false); // fix unreasonable light-blocker
		// fix fence subtiles
		nSolidTable(474, false);
		nSolidTable(479, false);
		// nSolidTable(487, false); // unused after patch
		nSolidTable(488, true);
		nSolidTable(540, false); // unused in base game
		// - with subtile-based automap
		nBlockTable(166, false);
		nBlockTable(168, false);
		// - separate subtiles for the automap
		nSolidTable(258, true);
		nMissileTable(258, true);
	} break;
#if ASSET_MPL == 1
	case FILE_HELL_CEL:
	{	// patch dMicroCels - L4.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_HELL_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_HELL_MIN]);
			return NULL;
		}
		if (minLen < 456 * BLOCK_SIZE_L4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_HELL_MIN]);
			return NULL;
		}
		buf = DRLP_L4_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			DRLP_L4_PatchMin(minBuf);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_HELL_MIN:
	{	// patch dMiniTiles - L4.MIN
		constexpr int blockSize = BLOCK_SIZE_L4;
		if (*dwLen < 456 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L4_PatchMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_HELL_TIL:
	{	// patch dMegaTiles - L4.TIL
		if (*dwLen < 137 * 4 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L4_PatchTil(buf);
	} break;
	case FILE_HELL_SOL:
	{	// patch dSolidTable - L4.SOL
		if (*dwLen <= 211) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		nMissileTable(141, false); // fix missile-blocking tile of down-stairs.
		// nMissileTable(137, false); // fix missile-blocking tile of down-stairs.
		// nSolidTable(137, false);   // fix non-walkable tile of down-stairs. - the subtile is not used any more
		nSolidTable(130, true);    // make the inner tiles of the down-stairs non-walkable I.
		nSolidTable(132, true);    // make the inner tiles of the down-stairs non-walkable II.
		nSolidTable(131, true);    // make the inner tiles of the down-stairs non-walkable III.
		// fix all-blocking tile on the diablo-level
		nSolidTable(211, false);
		nMissileTable(211, false);
		nBlockTable(211, false);
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
	case FILE_PLR_WHBAT:
	case FILE_PLR_WLBAT:
	case FILE_PLR_WMBAT:
	{	// reencode player gfx files - W*BAT.CL2
		buf = ReEncodeCL2(buf, dwLen, NUM_DIRS, 16, 128, 96);
	} break;
	case FILE_PLR_WMHAS:
	{	// fix player gfx file - WMHAS.CL2
		size_t atkLen;
		const char* atkFileName = "PlrGFX\\Warrior\\WMH\\WMHAT.CL2";
		BYTE* atkBuf = LoadFileInMem(atkFileName, &atkLen);
		if (atkBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", atkFileName);
			return NULL;
		}
		size_t stdLen;
		const char* stdFileName = "PlrGFX\\Warrior\\WMM\\WMMAS.CL2";
		BYTE* stdBuf = LoadFileInMem(stdFileName, &stdLen);
		if (stdBuf == NULL) {
			mem_free_dbg(atkBuf);
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", stdFileName);
			return NULL;
		}
		buf = createWarriorAnim(buf, dwLen, atkBuf, stdBuf);
		mem_free_dbg(atkBuf);
		mem_free_dbg(stdBuf);
	} break;
#ifdef HELLFIRE
#if ASSET_MPL == 1
	case FILE_NTOWN_CEL:
	{	// patch dMicroCels - TOWN.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_NTOWN_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_NTOWN_MIN]);
			return NULL;
		}
		if (minLen < 1379 * BLOCK_SIZE_TOWN * 2) {
			// mem_free_dbg(buf);
			// app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_NTOWN_MIN]);
			// return NULL;
			return buf; // -- assume it is already done
		}
		buf = Town_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			minBuf = Town_PatchMin(minBuf, &minLen, true);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_NTOWN_MIN:
	{	// patch dMiniTiles - Town.MIN
		constexpr int blockSize = BLOCK_SIZE_TOWN;
		if (*dwLen < 1379 * blockSize * 2) {
			// mem_free_dbg(buf);
			// app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			// return NULL;
			return buf; // -- assume it is already done
		}
		buf = Town_PatchMin(buf, dwLen, true);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
	case FILE_NEST_CEL:
	{	// patch dMicroCels - L6.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_NEST_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_NEST_MIN]);
			return NULL;
		}
		if (*dwLen < 606 * BLOCK_SIZE_L6 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_NEST_MIN]);
			return NULL;
		}
		buf = DRLP_L6_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			DRLP_L6_PatchMin(minBuf);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_NEST_MIN:
	{	// patch dMiniTiles - L6.MIN
		constexpr int blockSize = BLOCK_SIZE_L6;
		if (*dwLen < 606 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L6_PatchMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif /* ASSET_MPL == 1 */
	case FILE_NEST_TIL:
	{	// patch dMegaTiles - L6.TIL
		if (*dwLen < 4 * 166 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L6_PatchTil(buf);
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
		// - with subtile-based automap
		nBlockTable(61, false);
		nBlockTable(63, false);
		nBlockTable(65, false);
		nBlockTable(66, false);
	} break;
#if ASSET_MPL == 1
	case FILE_CRYPT_CEL:
	{	// patch dMicroCels - L5.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CRYPT_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CRYPT_MIN]);
			return NULL;
		}
		buf = DRLP_L5_PatchCel(minBuf, minLen, buf, dwLen);
		if (buf != NULL) {
			DRLP_L5_PatchMin(minBuf);
			buf = buildBlkCel(buf, dwLen);
		}
		mem_free_dbg(minBuf);
	} break;
	case FILE_CRYPT_MIN:
	{	// patch dMiniTiles - L5.MIN
		constexpr int blockSize = BLOCK_SIZE_L5;
		if (*dwLen < 650 * blockSize * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L5_PatchMin(buf);
		buf = buildBlkMin(buf, dwLen, blockSize);
	} break;
#endif // ASSET_MPL
	case FILE_CRYPT_TIL:
	{	// patch dMegaTiles - L5.TIL
		if (*dwLen < 4 * 217 * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		DRLP_L5_PatchTil(buf);
	} break;
	case FILE_CRYPT_SOL:
	{ // patch dSolidTable - L5.SOL
		if (*dwLen <= 600) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// make collision-checks more reasonable
		// - fix inconsistent subtile on the right side of down-stairs
		nSolidTable(143, false);
		//  - fix inconsistent entrance to Na-Krul
		nSolidTable(299, false);
		nMissileTable(299, false);
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
		//  - fix inconsistent arches
		nBlockTable(33, false);
		nBlockTable(39, false);
		nBlockTable(42, false);
		nBlockTable(91, false);
		nBlockTable(466, false);
		nBlockTable(470, false);
		nBlockTable(557, false);
		nBlockTable(559, false);
		nBlockTable(561, false);
		nBlockTable(563, false);
		nMissileTable(101, true);
		nMissileTable(104, true);
		nMissileTable(355, true);
		nMissileTable(357, true);
		// - adjust SOL after DRLP_L5_PatchMin
		nSolidTable(238, false);
		nMissileTable(238, false);
		nBlockTable(238, false);
		nMissileTable(178, false);
		nBlockTable(178, false);
		nSolidTable(242, false);
		nMissileTable(242, false);
		nBlockTable(242, false);
		// - fix automap of the entrance II.
		nMissileTable(158, false);
		nBlockTable(158, false);
		nSolidTable(159, false);
		nMissileTable(159, false);
		nBlockTable(159, false);
		nMissileTable(148, true);
		// nBlockTable(148, true);
		// - with subtile-based automap
		// nBlockTable(148, false);
		nBlockTable(149, false);
		nBlockTable(150, false);
		nBlockTable(153, false);
	} break;
	case FILE_OBJCURS_CEL:
	{
		size_t sizeB, sizeAB;
		BYTE *aCursCels, *bCursCels;
		DWORD numA, numAB;

		numA = SwapLE32(((DWORD*)buf)[0]);
		if (numA != 179) {
			if (numA != 179 + 61 - 2) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				buf = NULL;
			}
			return buf;
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

	// ignore the merged mpq during the patch
	mpqone = diabdat_mpqs[NUM_MPQS];
	diabdat_mpqs[NUM_MPQS] = NULL;
	bool result = UiProgressDialog("...Patch in progress...", patcher_callback);
	// restore the merged mpq
	diabdat_mpqs[NUM_MPQS] = mpqone;
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
