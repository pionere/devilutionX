#include <string>
#include <fstream>
#include <set>

#include "diabloui.h"
#include "selok.h"
#include "utils/display.h"
#include "utils/paths.h"
#include "utils/file_util.h"
#include "engine/render/cel_render.h"
#include "engine/render/cl2_render.h"
#include "engine/render/dun_render.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
static Uint32 sgRenderTc;
static std::vector<std::string> mpqfiles;
static HANDLE archive;
static int hashCount;
static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_DONE = 100;

#define SUB_HEADER_SIZE 0x0A

typedef enum filenames {
	FILE_MOVIE_VIC1,
	FILE_MOVIE_VIC2,
	FILE_MOVIE_VIC3,
#if ASSET_MPL == 1
//	FILE_TOWN_SCEL,
	FILE_TOWN_CEL,
	FILE_TOWN_MIN,
	FILE_L1DOORS_CEL,
	FILE_CATHEDRAL_SCEL,
	FILE_CATHEDRAL_CEL,
	FILE_CATHEDRAL_MIN,
#endif
	FILE_CATHEDRAL_TIL,
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
	FILE_FOULWATR_DUN,
#if ASSET_MPL == 1
	FILE_L3DOORS_CEL,
	FILE_CAVES_CEL,
	FILE_CAVES_MIN,
#endif
	FILE_CAVES_TIL,
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
#if ASSET_MPL == 1
	FILE_OBJ_MCIRL_CEL,
	FILE_OBJ_CNDL2_CEL,
	FILE_OBJ_LSHR_CEL,
	FILE_OBJ_RSHR_CEL,
	FILE_PLR_WHBAT,
	FILE_PLR_WLBAT,
	FILE_PLR_WMBAT,
	FILE_PLR_WMHAS,
#endif
#ifdef HELLFIRE
#if ASSET_MPL == 1
	FILE_NTOWN_CEL,
	FILE_NTOWN_MIN,
#endif
#if ASSET_MPL == 1
	FILE_CRYPT_SCEL,
	FILE_CRYPT_CEL,
	FILE_CRYPT_MIN,
#endif
	FILE_CRYPT_TIL,
#if ASSET_MPL == 1
	FILE_NEST_CEL,
	FILE_NEST_MIN,
#endif
	FILE_NEST_TIL,
#if ASSET_MPL == 1
	FILE_L5LIGHT_CEL,
	FILE_MON_FALLGW,
	FILE_MON_GOATLD,
#endif
#endif // HELLFIRE
#if ASSET_MPL == 1
	FILE_ITEM_ARMOR2,
	FILE_ITEM_GOLDFLIP,
	FILE_ITEM_MACE,
	FILE_ITEM_STAFF,
	FILE_ITEM_RING,
	FILE_ITEM_CROWNF,
	FILE_ITEM_LARMOR,
	FILE_ITEM_WSHIELD,
	FILE_ITEM_SCROLL,
	FILE_ITEM_FEAR,
	FILE_ITEM_FBRAIN,
	FILE_ITEM_FMUSH,
	FILE_ITEM_INNSIGN,
	FILE_ITEM_BLDSTN,
	FILE_ITEM_FANVIL,
	FILE_ITEM_FLAZSTAF,
#ifdef HELLFIRE
	FILE_ITEM_TEDDYS1,
	FILE_ITEM_COWS1,
	FILE_ITEM_DONKYS1,
	FILE_ITEM_MOOSES1,
#endif
#endif
	FILE_OBJCURS_CEL,
	NUM_FILENAMES
} filenames;

static const char* const filesToPatch[NUM_FILENAMES] = {
/*FILE_MOVIE_VIC1*/    "gendata\\DiabVic1.smk",
/*FILE_MOVIE_VIC2*/    "gendata\\DiabVic2.smk",
/*FILE_MOVIE_VIC3*/    "gendata\\DiabVic3.smk",
#if ASSET_MPL == 1
///*FILE_TOWN_SCEL*/     "Levels\\TownData\\TownS.CEL",
/*FILE_TOWN_CEL*/      "Levels\\TownData\\Town.CEL",
/*FILE_TOWN_MIN*/      "Levels\\TownData\\Town.MIN",
/*FILE_L1DOORS_CEL*/   "Objects\\L1Doors.CEL",
/*FILE_CATHEDRAL_SCEL*/"Levels\\L1Data\\L1S.CEL",
/*FILE_CATHEDRAL_CEL*/ "Levels\\L1Data\\L1.CEL",
/*FILE_CATHEDRAL_MIN*/ "Levels\\L1Data\\L1.MIN",
#endif
/*FILE_CATHEDRAL_TIL*/ "Levels\\L1Data\\L1.TIL",
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
/*FILE_FOULWATR_DUN*/  "Levels\\L3Data\\Foulwatr.DUN",
#if ASSET_MPL == 1
/*FILE_L3DOORS_CEL*/   "Objects\\L3Doors.CEL",
/*FILE_CAVES_CEL*/     "Levels\\L3Data\\L3.CEL",
/*FILE_CAVES_MIN*/     "Levels\\L3Data\\L3.MIN",
#endif
/*FILE_CAVES_TIL*/     "Levels\\L3Data\\L3.TIL",
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
#if ASSET_MPL == 1
/*FILE_OBJ_MCIRL_CEL*/ "Objects\\Mcirl.CEL",
/*FILE_OBJ_CNDL2_CEL*/ "Objects\\Candle2.CEL",
/*FILE_OBJ_LSHR_CEL*/  "Objects\\LShrineG.CEL",
/*FILE_OBJ_RSHR_CEL*/  "Objects\\RShrineG.CEL",
/*FILE_PLR_WHBAT*/     "PlrGFX\\Warrior\\WHB\\WHBAT.CL2",
/*FILE_PLR_WLBAT*/     "PlrGFX\\Warrior\\WLB\\WLBAT.CL2",
/*FILE_PLR_WMBAT*/     "PlrGFX\\Warrior\\WMB\\WMBAT.CL2",
/*FILE_PLR_WMHAS*/     "PlrGFX\\Warrior\\WMH\\WMHAS.CL2",
#endif
#ifdef HELLFIRE
#if ASSET_MPL == 1
/*FILE_NTOWN_CEL*/     "NLevels\\TownData\\Town.CEL",
/*FILE_NTOWN_MIN*/     "NLevels\\TownData\\Town.MIN",
#endif
#if ASSET_MPL == 1
/*FILE_CRYPT_SCEL*/    "NLevels\\L5Data\\L5S.CEL",
/*FILE_CRYPT_CEL*/     "NLevels\\L5Data\\L5.CEL",
/*FILE_CRYPT_MIN*/     "NLevels\\L5Data\\L5.MIN",
#endif
/*FILE_CRYPT_TIL*/     "NLevels\\L5Data\\L5.TIL",
#if ASSET_MPL == 1
/*FILE_NEST_CEL*/      "NLevels\\L6Data\\L6.CEL",
/*FILE_NEST_MIN*/      "NLevels\\L6Data\\L6.MIN",
#endif
/*FILE_NEST_TIL*/      "NLevels\\L6Data\\L6.TIL",
#if ASSET_MPL == 1
/*FILE_L5LIGHT_CEL*/   "Objects\\L5Light.CEL",
/*FILE_MON_FALLGW*/    "Monsters\\BigFall\\Fallgw.CL2",
/*FILE_MON_GOATLD*/    "Monsters\\GoatLord\\GoatLd.CL2",
#endif
#endif // HELLFIRE
#if ASSET_MPL == 1
/*FILE_ITEM_ARMOR2*/   "Items\\Armor2.CEL",
/*FILE_ITEM_GOLDFLIP*/ "Items\\GoldFlip.CEL",
/*FILE_ITEM_MACE*/     "Items\\Mace.CEL",
/*FILE_ITEM_STAFF*/    "Items\\Staff.CEL",
/*FILE_ITEM_RING*/     "Items\\Ring.CEL",
/*FILE_ITEM_CROWNF*/   "Items\\CrownF.CEL",
/*FILE_ITEM_LARMOR*/   "Items\\LArmor.CEL",
/*FILE_ITEM_WSHIELD*/  "Items\\WShield.CEL",
/*FILE_ITEM_SCROLL*/   "Items\\Scroll.CEL",
/*FILE_ITEM_FEAR*/     "Items\\FEar.CEL",
/*FILE_ITEM_FBRAIN*/   "Items\\FBrain.CEL",
/*FILE_ITEM_FMUSH*/    "Items\\FMush.CEL",
/*FILE_ITEM_INNSIGN*/  "Items\\Innsign.CEL",
/*FILE_ITEM_BLDSTN*/   "Items\\Bldstn.CEL",
/*FILE_ITEM_FANVIL*/   "Items\\Fanvil.CEL",
/*FILE_ITEM_FLAZSTAF*/ "Items\\FLazStaf.CEL",
#ifdef HELLFIRE
/*FILE_ITEM_TEDDYS1*/  "Items\\teddys1.CEL",
/*FILE_ITEM_COWS1*/    "Items\\cows1.CEL",
/*FILE_ITEM_DONKYS1*/  "Items\\donkys1.CEL",
/*FILE_ITEM_MOOSES1*/  "Items\\mooses1.CEL",
#endif
#endif
/*FILE_OBJCURS_CEL*/   "Data\\Inv\\Objcurs.CEL",
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
	DWORD dstCelEntries = srcCelEntries - (DWORD)removeMicros.size();
	dstHeaderCursor[0] = SwapLE32(dstCelEntries);
	dstHeaderCursor++;
	BYTE* dstDataCursor = resCelBuf + 4 * (dstCelEntries + 2);
	while (!removeMicros.empty()) {
		// select the next frame
		unsigned nextRef = *removeMicros.begin();
		removeMicros.erase(nextRef);

		// copy entries till the next frame
		unsigned numEntries = nextRef - (unsigned)((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
		for (unsigned i = 0; i < numEntries; i++) {
			dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
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
	unsigned numEntries = srcCelEntries + 1 - (unsigned)((size_t)srcHeaderCursor - (size_t)celBuf) / 4;
	for (unsigned i = 0; i < numEntries; i++) {
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;
		DWORD len = srcHeaderCursor[1] - srcHeaderCursor[0];
		memcpy(dstDataCursor, celBuf + srcHeaderCursor[0], len);
		dstDataCursor += len;
		srcHeaderCursor++;
	}
	// add file-size
	dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));

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

static void patchMovie(int fileIndex, BYTE* fileBuf, size_t* fileSize)
{
	uint32_t* lm = (uint32_t*)fileBuf;

	// TODO: validate file-size
	switch (fileIndex) {
	case FILE_MOVIE_VIC1:
	{	// patch movie - DiabVic1.smk
		lm += 2365112 / 4;
		lm[0] = SwapLE32(-550965321);
		lm[1] = SwapLE32(-272898787);
		lm[2] = SwapLE32(2091106043);
		lm[3] = SwapLE32(1596505681);
		lm[4] = SwapLE32(2009739156);
		lm[5] = SwapLE32(-202827765);
		lm[6] = SwapLE32(2104344764);
		lm[7] = SwapLE32(-2141622575);
		lm[8] = SwapLE32(48038255);
		lm[9] = SwapLE32(45358492);
		lm[10] = SwapLE32(-1267717305);
		lm[11] = SwapLE32(-1587331326);
		lm[12] = SwapLE32(-2120218010);
		lm[13] = SwapLE32(-2011561816);
		lm[14] = SwapLE32(1414846530);
		lm[15] = SwapLE32(-1349224483);
		lm[16] = SwapLE32(1596506156);
		lm[17] = SwapLE32(-1101954983);
		lm[18] = SwapLE32(1199431464);
		lm[19] = SwapLE32(1367122681);
		lm[20] = SwapLE32(-1805702978);
		lm[21] = SwapLE32(-545797585);
		lm[22] = SwapLE32(1596505681);
		lm[23] = SwapLE32(-225513580);
		lm[24] = SwapLE32(1005517125);
		lm[25] = SwapLE32(-1947920438);
		lm[26] = SwapLE32(-1560721934);
		lm[27] = SwapLE32(-71413380);
		lm[28] = SwapLE32(-112856434);
		lm[29] = SwapLE32(-1101955934);
		lm[30] = SwapLE32(-1349230808);
		lm[31] = SwapLE32(199978722);
		lm[32] = SwapLE32(-834474092);
		lm[33] = SwapLE32(1232795968);
		lm[34] = SwapLE32(1968370609);
		lm[35] = SwapLE32(-1751256130);
		lm[36] = SwapLE32(26469610);
		lm[37] = SwapLE32(-1792990555);
		lm[38] = SwapLE32(-1099607468);
		lm[39] = SwapLE32(1251410675);
		lm[40] = SwapLE32(-136193238);
		lm[41] = SwapLE32(2079387026);
		lm[42] = SwapLE32(-130195607);
		lm[43] = SwapLE32(1596987095);
		lm[44] = SwapLE32(-905946117);
		lm[45] = SwapLE32(-113243273);
		lm[46] = SwapLE32(781362926);
		lm[47] = SwapLE32(399126420);
		lm[48] = SwapLE32(1173523429);
		lm[49] = SwapLE32(-176381191);
		lm[50] = SwapLE32(275362381);
		lm[51] = SwapLE32(-155288568);
		lm[52] = SwapLE32(1439028673);
		lm[53] = SwapLE32(-2129603742);
		lm[54] = SwapLE32(766280566);
		lm[55] = SwapLE32(919806055);
		lm[56] = SwapLE32(1681665368);
		lm[57] = SwapLE32(-863511906);
		lm[58] = SwapLE32(-1957164198);
		lm[59] = SwapLE32(-1063860819);
		lm[60] = SwapLE32(-883300707);
		lm[61] = SwapLE32(-1663365712);
		lm[62] = SwapLE32(-107803491);
		lm[63] = SwapLE32(706130563);
		lm[64] = SwapLE32(-905866675);
		lm[65] = SwapLE32(-1308512774);
		lm[66] = SwapLE32(1493841248);
		lm[67] = SwapLE32(256192880);
		lm[68] = SwapLE32(-619848059);
		lm[69] = SwapLE32(369285801);
		lm[70] = SwapLE32(-876682358);
		lm[71] = SwapLE32(334941186);
		lm[72] = SwapLE32(2128183385);
		lm[73] = SwapLE32(23856562);
	} break;
	case FILE_MOVIE_VIC2:
	{	// patch movie - DiabVic2.smk
		lm += 2303424 / 4;
		lm[0] = SwapLE32(-692176717);
		lm[1] = SwapLE32(1924240277);
		lm[2] = SwapLE32(-1116350926);
		lm[3] = SwapLE32(846226659);
		lm[4] = SwapLE32(211556664);
		lm[5] = SwapLE32(1836009038);
		lm[6] = SwapLE32(1847955435);
		lm[7] = SwapLE32(-1153933168);
		lm[8] = SwapLE32(-14372439);
		lm[9] = SwapLE32(-1352999270);
		lm[10] = SwapLE32(2047723429);
		lm[11] = SwapLE32(231186652);
		lm[12] = SwapLE32(-748630536);
		lm[13] = SwapLE32(1183210856);
		lm[14] = SwapLE32(-1080806677);
		lm[15] = SwapLE32(-1159502474);
		lm[16] = SwapLE32(1490301554);
		lm[17] = SwapLE32(1666238914);
		lm[18] = SwapLE32(-1820121335);
		lm[19] = SwapLE32(211594929);
		lm[20] = SwapLE32(-2094594482);
		lm[21] = SwapLE32(-523648621);
		lm[22] = SwapLE32(657173604);
		lm[23] = SwapLE32(1540461443);
		lm[24] = SwapLE32(1663511471);
		lm[25] = SwapLE32(-1676068515);
		lm[26] = SwapLE32(654724620);
		lm[27] = SwapLE32(-910060669);
		lm[28] = SwapLE32(-1047294120);
		lm[29] = SwapLE32(1885659337);
		lm[30] = SwapLE32(-1676068814);
		lm[31] = SwapLE32(76938764);
		lm[32] = SwapLE32(1988926659);
		lm[33] = SwapLE32(-2121035208);
		lm[34] = SwapLE32(162658691);
		lm[35] = SwapLE32(1300551711);
		lm[36] = SwapLE32(-1315994408);
		lm[37] = SwapLE32(-943693268);
		lm[38] = SwapLE32(1911569951);
		lm[39] = SwapLE32(-598899496);
		lm[40] = SwapLE32(-1136785472);
		lm[41] = SwapLE32(-130681637);
		lm[42] = SwapLE32(-84905981);
		lm[43] = SwapLE32(1866660917);
		lm[44] = SwapLE32(-1123243555);
		lm[45] = SwapLE32(-1214044363);
		lm[46] = SwapLE32(1991009510);
		lm[47] = SwapLE32(-2094590931);
		lm[48] = SwapLE32(-523648621);
		lm[49] = SwapLE32(942829668);
		lm[50] = SwapLE32(-661056999);
		lm[51] = SwapLE32(-663336473);
		lm[52] = SwapLE32(-2103178071);
		lm[53] = SwapLE32(-810450266);
		lm[54] = SwapLE32(-1798627861);
		lm[55] = SwapLE32(468686179);
		lm[56] = SwapLE32(-510503487);
		lm[57] = SwapLE32(1505680763);
		lm[58] = SwapLE32(172312093);
		lm[59] = SwapLE32(-450712496);
		lm[60] = SwapLE32(-284186083);
		lm[61] = SwapLE32(124493483);
		lm[62] = SwapLE32(2141065680);
		lm[63] = SwapLE32(344695419);
		lm[64] = SwapLE32(-174581497);
		lm[65] = SwapLE32(2096023608);
		lm[66] = SwapLE32(-1908604696);
		lm[67] = SwapLE32(-1021173752);
		lm[68] = SwapLE32(-1211136129);
		lm[69] = SwapLE32(-688626406);
		lm[70] = SwapLE32(1030122221);
		lm[71] = SwapLE32(-1448569291);
		lm[72] = SwapLE32(1780845896);
		lm[73] = SwapLE32(-810685910);
	} break;
	case FILE_MOVIE_VIC3:
	{	// patch movie - DiabVic3.smk
		lm += 2313520 / 4;
		lm[0] = SwapLE32(1810378268);
		lm[1] = SwapLE32(1956082596);
		lm[2] = SwapLE32(-1852204306);
		lm[3] = SwapLE32(-1528346162);
		lm[4] = SwapLE32(-382086541);
		lm[5] = SwapLE32(248990172);
		lm[6] = SwapLE32(-1442865169);
		lm[7] = SwapLE32(1302565187);
		lm[8] = SwapLE32(-15115782);
		lm[9] = SwapLE32(1349539657);
		lm[10] = SwapLE32(-3497991);
		lm[11] = SwapLE32(1715994196);
		lm[12] = SwapLE32(-94472062);
		lm[13] = SwapLE32(-2014741919);
		lm[14] = SwapLE32(1392874889);
		lm[15] = SwapLE32(-414656584);
		lm[16] = SwapLE32(-1658193332);
		lm[17] = SwapLE32(1957161267);
		lm[18] = SwapLE32(1186702574);
		lm[19] = SwapLE32(-1818417350);
		lm[20] = SwapLE32(-1528346162);
		lm[21] = SwapLE32(1807340147);
		lm[22] = SwapLE32(970093476);
		lm[23] = SwapLE32(-1658462999);
		lm[24] = SwapLE32(593350291);
		lm[25] = SwapLE32(1238274973);
		lm[26] = SwapLE32(-764173081);
		lm[27] = SwapLE32(451843001);
		lm[28] = SwapLE32(1316265193);
		lm[29] = SwapLE32(-1818417350);
		lm[30] = SwapLE32(2028423630);
		lm[31] = SwapLE32(1213262382);
		lm[32] = SwapLE32(1233196775);
		lm[33] = SwapLE32(-466215703);
		lm[34] = SwapLE32(14841857);
		lm[35] = SwapLE32(1006878780);
		lm[36] = SwapLE32(-192136591);
		lm[37] = SwapLE32(-2008008564);
		lm[38] = SwapLE32(14818371);
		lm[39] = SwapLE32(1342160956);
		lm[40] = SwapLE32(-2142182884);
		lm[41] = SwapLE32(-1934654195);
		lm[42] = SwapLE32(859320077);
		lm[43] = SwapLE32(384383416);
		lm[44] = SwapLE32(-1817907820);
		lm[45] = SwapLE32(-764109362);
		lm[46] = SwapLE32(-40358983);
		lm[47] = SwapLE32(-414593385);
		lm[48] = SwapLE32(970093476);
		lm[49] = SwapLE32(1316265193);
		lm[50] = SwapLE32(145792077);
		lm[51] = SwapLE32(-1810424179);
		lm[52] = SwapLE32(-183817203);
		lm[53] = SwapLE32(689004884);
		lm[54] = SwapLE32(-296328173);
		lm[55] = SwapLE32(-673137993);
		lm[56] = SwapLE32(502011765);
		lm[57] = SwapLE32(1689426827);
		lm[58] = SwapLE32(-547293725);
		lm[59] = SwapLE32(-474683808);
		lm[60] = SwapLE32(-1511228563);
		lm[61] = SwapLE32(-1515184584);
		lm[62] = SwapLE32(-753466767);
		lm[63] = SwapLE32(-880436852);
		lm[64] = SwapLE32(1545606400);
		lm[65] = SwapLE32(316322038);
		lm[66] = SwapLE32(-1061779178);
		lm[67] = SwapLE32(719126690);
		lm[68] = SwapLE32(-1785938886);
		lm[69] = SwapLE32(1415292139);
		lm[70] = SwapLE32(-191309975);
		lm[71] = SwapLE32(644310227);
		lm[72] = SwapLE32(-222423811);
		lm[73] = SwapLE32(1019295561);
	} break;
	}
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
		// protect the central room from torch placement
		for (int y = 9; y < 13; y++) {
			for (int x = 13; x < 17; x++) {
				lm[2 + 32 * 18 + x + y * 32] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
			}
		}
		// protect the changing tiles from torch placement
		lm[2 + 32 * 18 + 14 + 5 * 32] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
		lm[2 + 32 * 18 + 18 + 5 * 32] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12) | (3 << 14));
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
		lm[2 + 10 * 16 + 2 + 3 * 10] = SwapLE16((3 << 10));
		lm[2 + 10 * 16 + 3 + 3 * 10] = SwapLE16((3 << 8) | (3 << 12));
		lm[2 + 10 * 16 + 6 + 3 * 10] = SwapLE16((3 << 8) | (3 << 10) | (3 << 12));
		for (int y = 4; y < 7; y++) {
			lm[2 + 10 * 16 + 3 + y * 10] = SwapLE16((3 << 8) | (3 << 12));
			lm[2 + 10 * 16 + 6 + y * 10] = SwapLE16((3 << 8) | (3 << 12));
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
#if ASSET_MPL == 1
static BYTE* fixObjCircle(BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int FRAME_WIDTH = 96;
	constexpr int FRAME_HEIGHT = 96;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 4) {
		app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_OBJ_MCIRL_CEL]);
		mem_free_dbg(celBuf);
		return NULL;
	}

	// create the new CEL file
	size_t maxCelSize = *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);
	for (int i = 0; i < srcCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		if (i == 0 && gpBuffer[5 + 70 *  BUFFER_WIDTH] == TRANS_COLOR) {
			mem_free_dbg(resCelBuf);
			return celBuf; // assume it is already done
		}

		int sy = 52;
		int ey = 91;
		int sx = 5;
		int ex = 85;
		int nw = (ex - sx) / 2;
		int nh = (ey - sy) / 2;
		// down-scale to half
		for (int y = sy; y < ey; y += 2) {
			for (int x = sx; x < ex; x += 2) {
				gpBuffer[sx + (x - sx) / 2 + (sy + (y - sy) / 2) *  BUFFER_WIDTH] = gpBuffer[x + y *  BUFFER_WIDTH];
			}
		}
		for (int y = sy; y < ey; y++) {
			memset(&gpBuffer[sx + nw + y * BUFFER_WIDTH], TRANS_COLOR, FRAME_WIDTH - (sx + nw));
		}
		for (int y = sy + nh; y < ey; y++) {
			memset(&gpBuffer[sx + y * BUFFER_WIDTH], TRANS_COLOR, FRAME_WIDTH - sx);
		}
		// move to the center
		constexpr int offx = 4;
		constexpr int offy = -2;
		for (int y = ey - 1; y >= sy + nh; y--) {
			memmove(&gpBuffer[nw / 2 + offx + (y + offy) * BUFFER_WIDTH], &gpBuffer[(y - nh) * BUFFER_WIDTH], FRAME_WIDTH - nw / 2);
			memset(&gpBuffer[(y - nh) * BUFFER_WIDTH], TRANS_COLOR, FRAME_WIDTH - nw / 2);
		}
		// fix colors
		if (i == 0) {
			gpBuffer[57 + 70 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[65 + 84 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[58 + 87 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[34 + 85 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[40 + 87 * BUFFER_WIDTH] = TRANS_COLOR;

			gpBuffer[46 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[47 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[48 + 69 * BUFFER_WIDTH] = 250;
			gpBuffer[49 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[50 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[51 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[46 + 88 * BUFFER_WIDTH] = 248;
			gpBuffer[47 + 88 * BUFFER_WIDTH] = 248;
			gpBuffer[48 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[49 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[50 + 88 * BUFFER_WIDTH] = 250;
			gpBuffer[51 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[40 + 70 * BUFFER_WIDTH] = 249;
			gpBuffer[30 + 75 * BUFFER_WIDTH] = 248;
			gpBuffer[31 + 74 * BUFFER_WIDTH] = 248;
			gpBuffer[33 + 73 * BUFFER_WIDTH] = 250;
			gpBuffer[37 + 71 * BUFFER_WIDTH] = 251;
			gpBuffer[67 + 75 * BUFFER_WIDTH] = 250;
			gpBuffer[66 + 74 * BUFFER_WIDTH] = 250;
			gpBuffer[63 + 72 * BUFFER_WIDTH] = 251;
			gpBuffer[61 + 86 * BUFFER_WIDTH] = 252;

			gpBuffer[29 + 77 * BUFFER_WIDTH] = 249;
			gpBuffer[29 + 78 * BUFFER_WIDTH] = 250;
			gpBuffer[29 + 79 * BUFFER_WIDTH] = 251;
			gpBuffer[29 + 80 * BUFFER_WIDTH] = 249;
		}
		if (i == 1) {
			gpBuffer[57 + 70 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[61 + 71 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[65 + 73 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[68 + 81 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[58 + 87 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[65 + 84 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[40 + 87 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[46 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[47 + 88 * BUFFER_WIDTH] = 166;
			gpBuffer[48 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[49 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[50 + 88 * BUFFER_WIDTH] = 169;
			gpBuffer[51 + 88 * BUFFER_WIDTH] = 248;
			gpBuffer[30 + 75 * BUFFER_WIDTH] = 248;
			gpBuffer[31 + 74 * BUFFER_WIDTH] = 248;
			gpBuffer[30 + 82 * BUFFER_WIDTH] = 166;
			gpBuffer[31 + 83 * BUFFER_WIDTH] = 166;
			gpBuffer[51 + 69 * BUFFER_WIDTH] = 250;
			gpBuffer[46 + 69 * BUFFER_WIDTH] = 250;
			gpBuffer[29 + 77 * BUFFER_WIDTH] = 249;
			gpBuffer[29 + 78 * BUFFER_WIDTH] = 250;
			gpBuffer[29 + 80 * BUFFER_WIDTH] = 248;
		}
		if (i == 2) {
			gpBuffer[37 + 71 * BUFFER_WIDTH] = 250;
			gpBuffer[40 + 70 * BUFFER_WIDTH] = 250;
			gpBuffer[40 + 87 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[57 + 70 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[63 + 72 * BUFFER_WIDTH] = 250;
			gpBuffer[58 + 87 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[65 + 84 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[67 + 75 * BUFFER_WIDTH] = 251;
			gpBuffer[66 + 74 * BUFFER_WIDTH] = 251;
			gpBuffer[29 + 77 * BUFFER_WIDTH] = 249;
			gpBuffer[29 + 78 * BUFFER_WIDTH] = 250;
			gpBuffer[29 + 79 * BUFFER_WIDTH] = 251;
			gpBuffer[29 + 80 * BUFFER_WIDTH] = 250;
			gpBuffer[30 + 75 * BUFFER_WIDTH] = 249;
			gpBuffer[31 + 74 * BUFFER_WIDTH] = 249;
			gpBuffer[30 + 82 * BUFFER_WIDTH] = 250;
			gpBuffer[31 + 83 * BUFFER_WIDTH] = 250;

			gpBuffer[46 + 88 * BUFFER_WIDTH] = 248;
			gpBuffer[47 + 88 * BUFFER_WIDTH] = 248;
			gpBuffer[48 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[49 + 88 * BUFFER_WIDTH] = 250;
			gpBuffer[50 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[51 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[46 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[47 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[48 + 69 * BUFFER_WIDTH] = 250;
			gpBuffer[49 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[50 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[51 + 69 * BUFFER_WIDTH] = 249;
		}
		if (i == 3) {
			gpBuffer[30 + 78 * BUFFER_WIDTH] = 167;
			gpBuffer[29 + 77 * BUFFER_WIDTH] = 249;
			gpBuffer[29 + 78 * BUFFER_WIDTH] = 250;
			gpBuffer[29 + 79 * BUFFER_WIDTH] = 251;
			gpBuffer[29 + 80 * BUFFER_WIDTH] = 250;
			gpBuffer[40 + 87 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[63 + 85 * BUFFER_WIDTH] = 252;
			gpBuffer[57 + 70 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[61 + 71 * BUFFER_WIDTH] = TRANS_COLOR;
			gpBuffer[30 + 75 * BUFFER_WIDTH] = 249;
			gpBuffer[31 + 74 * BUFFER_WIDTH] = 248;
			gpBuffer[30 + 82 * BUFFER_WIDTH] = 249;
			gpBuffer[31 + 83 * BUFFER_WIDTH] = 250;
			gpBuffer[47 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[48 + 88 * BUFFER_WIDTH] = 250;
			gpBuffer[49 + 88 * BUFFER_WIDTH] = 251;
			gpBuffer[67 + 75 * BUFFER_WIDTH] = 250;
			gpBuffer[66 + 74 * BUFFER_WIDTH] = 250;
			gpBuffer[48 + 69 * BUFFER_WIDTH] = 251;
			gpBuffer[47 + 69 * BUFFER_WIDTH] = 250;
			gpBuffer[40 + 70 * BUFFER_WIDTH] = 250;
			gpBuffer[37 + 71 * BUFFER_WIDTH] = 250;
			gpBuffer[61 + 86 * BUFFER_WIDTH] = 250;
			gpBuffer[63 + 72 * BUFFER_WIDTH] = 251;

			gpBuffer[46 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[50 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[51 + 88 * BUFFER_WIDTH] = 249;
			gpBuffer[46 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[50 + 69 * BUFFER_WIDTH] = 249;
			gpBuffer[51 + 69 * BUFFER_WIDTH] = 249;
		}

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

static BYTE* fixObjCandle(BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int FRAME_WIDTH = 96;
	constexpr int FRAME_HEIGHT = 96;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 4) {
		app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_OBJ_CNDL2_CEL]);
		mem_free_dbg(celBuf);
		return NULL;
	}

	// create the new CEL file
	size_t maxCelSize = *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);
	for (int i = 0; i < srcCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		if (i == 0 && gpBuffer[32 + 65 *  BUFFER_WIDTH] == TRANS_COLOR) {
			mem_free_dbg(resCelBuf);
			return celBuf; // assume it is already done
		}
		// remove shadow
		for (int y = 63; y < 80; y++) {
			for (int x = 28; x < 45; x++) {
				if (gpBuffer[x + y * BUFFER_WIDTH] == 0) {
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

static BYTE* fixObjLShrine(BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int FRAME_WIDTH = 128;
	constexpr int FRAME_HEIGHT = 128;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 22) {
		return celBuf; // assume it is already done
	}

	const int resCelEntries = 11;

	// create the new CEL file
	size_t maxCelSize = *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(resCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (resCelEntries + 2);
	for (int i = 0; i < resCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		// use the more rounded shrine-graphics
		CelClippedDraw(FRAME_WIDTH, FRAME_HEIGHT - 1, celBuf, 11 + 1, FRAME_WIDTH);
		for (int y = 88; y < 110; y++) {
			for (int x = 28; x < 80; x++) {
				if (gpBuffer[x + y * BUFFER_WIDTH] == 248) {
					continue; // preserve the dirt/shine on the floor
				}
				gpBuffer[x + y * BUFFER_WIDTH] = gpBuffer[x + 7 + FRAME_WIDTH + (y - 2) * BUFFER_WIDTH];
			}
		}

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

static BYTE* fixObjRShrine(BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int FRAME_WIDTH = 128;
	constexpr int FRAME_HEIGHT = 128;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 22) {
		return celBuf; // assume it is already done
	}

	const int resCelEntries = 11;

	// create the new CEL file
	size_t maxCelSize = *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(resCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (resCelEntries + 2);
	for (int i = 0; i < resCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		gpBuffer[85 + 101 * BUFFER_WIDTH] = TRANS_COLOR;
		gpBuffer[88 + 100 * BUFFER_WIDTH] = TRANS_COLOR;

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}

#ifdef HELLFIRE
static BYTE* fixL5Light(BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 128;
	constexpr int FRAME_WIDTH = 96;
	constexpr int FRAME_HEIGHT = 96;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	if (srcCelEntries != 9) {
		return celBuf; // assume it is already done
	}

	const int resCelEntries = 1;

	// create the new CEL file
	size_t maxCelSize = *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(resCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (resCelEntries + 2);
	for (int i = 0; i < resCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		// remove shadow
		for (int y = 61; y < 86; y++) {
			for (int x = 15; x < 63; x++) {
				if (gpBuffer[x + y * BUFFER_WIDTH] == 0) {
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)*celLen);

	return resCelBuf;
}
#endif // HELLFIRE

static BYTE* EncodeCl2(BYTE* pBuf, const BYTE* pSrc, int width, int height, BYTE transparentPixel)
{
	const int RLE_LEN = 4; // number of matching colors to switch from bmp encoding to RLE

	unsigned subHeaderSize = SUB_HEADER_SIZE;
	unsigned hs = (height - 1) / CEL_BLOCK_HEIGHT;
	hs = (hs + 1) * sizeof(WORD);
	subHeaderSize = std::max(subHeaderSize, hs);

	bool clipped = true; // frame->isClipped();
	// convert one image to cl2-data
	BYTE* pHeader = pBuf;
	if (clipped) {
		// add CL2 FRAME HEADER
		*(WORD*)&pBuf[0] = SwapLE16((WORD)subHeaderSize);
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
			*(WORD*)(&pHeader[(i / CEL_BLOCK_HEIGHT) * 2]) = SwapLE16((WORD)((size_t)pHead - (size_t)pHeader)); // pHead - buf - SUB_HEADER_SIZE;

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
				if (colMatches < RLE_LEN || (int8_t)*pHead <= -127) {
					// bmp encoding
					if (alpha || (int8_t)*pHead <= -65 || first) {
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
				if (!alpha || (int8_t)*pHead >= 127) {
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
		hdr[1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, (size_t)BUFFER_WIDTH * height);

			Cl2Draw(0, height - 1, frameBuf, n, width);
			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));
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
		hdr[1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, BUFFER_WIDTH * height);
			if (ii == 1) { // DIR_SW
				// draw the stand frame
				const BYTE* stdFrameBuf = CelGetFrameStart(stdBuf, ii);
				// for (int y = 0; y < height; y++) {
				//	memset(&gpBuffer[0 + BUFFER_WIDTH * y], TRANS_COLOR, width);
				// }
				Cl2Draw(0, height - 1, stdFrameBuf, n, width);
				// draw the attack frame
				constexpr int atkWidth = 128;
				const BYTE* atkFrameBuf = CelGetFrameStart(atkBuf, ii);
				// for (int y = 0; y < height; y++) {
				//	memset(&gpBuffer[0 + width + BUFFER_WIDTH * y], TRANS_COLOR, atkWidth);
				// }
				Cl2Draw(width, height - 1, atkFrameBuf, 1, atkWidth);

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
				Cl2Draw(0, height - 1, frameBuf, n, width);
			}

			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));
		}
		hdr += ni + 2;
	}

	*dwLen = (size_t)pBuf - (size_t)resCl2Buf;

	mem_free_dbg(cl2Buf);
	return resCl2Buf;
}
#endif // ASSET_MPL == 1
static void moveImage(int width, int height, int dx, int dy, BYTE TRANS_COLOR)
{
	if (dx > 0) {
		for (int y = 0; y < height; y++) {
			for (int x = width - dx - 1; x >= 0; x--) {
				gpBuffer[x + dx + BUFFER_WIDTH * y] = gpBuffer[x + BUFFER_WIDTH * y];
				gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
			}
		}
	}
	if (dx < 0) {
		for (int y = 0; y < height; y++) {
			for (int x = -dx; x < width; x++) {
				gpBuffer[x + dx + BUFFER_WIDTH * y] = gpBuffer[x + BUFFER_WIDTH * y];
				gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
			}
		}
	}
	if (dy > 0) {
		for (int y = height - dy - 1; y >= 0; y--) {
			for (int x = 0; x < width; x++) {
				gpBuffer[x + BUFFER_WIDTH * (y + dy)] = gpBuffer[x + BUFFER_WIDTH * y];
				gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
			}
		}
	}
	if (dy < 0) {
		for (int y = -dy; y < height; y++) {
			for (int x = 0; x < width; x++) {
				gpBuffer[x + BUFFER_WIDTH * (y + dy)] = gpBuffer[x + BUFFER_WIDTH * y];
				gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
			}
		}
	}
}

static BYTE* centerCursors(BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 1;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	const int resCelEntries = NUM_ICURS + CURSOR_FIRSTITEM - 1;
	assert(srcCelEntries == resCelEntries);

	// create the new CEL file
	size_t maxCelSize = *celLen * 2;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(resCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (resCelEntries + 2);
	bool needsPatch = false;
	for (int i = 0; i < resCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, InvItemHeight[i + 1] * BUFFER_WIDTH);
		CelClippedDraw(0, InvItemHeight[i + 1] - 1, celBuf, i + 1, InvItemWidth[i + 1]);

		int dx = 0, dy = 0;
		switch (i + 1) {
		case 236: dx = 0; dy = -2; break;
		case 234: dx = 0; dy = -2; break;
		case 233: dx = 0; dy = -1; break;
		case 232: dx = -2; dy = -2; break;
		case 231: dx = 0; dy = -1; break;
		case 230: dx = 2; dy = 0; break;
		case 229: dx = -1; dy = -3; break;
		case 228: dx = 0; dy = -2; break;
		case 227: dx = -1; dy = -2; break;
		case 226: dx = -1; dy = -2; break;
		case 225: dx = 0; dy = 2; break;
		case 224: dx = -1; dy = 1; break;
		case 223: dx = 0; dy = 1; break;
		case 222: dx = -2; dy = 3; break;
		case 221: dx = -1; dy = 2; break;
		case 220: dx = -1; dy = 0; break;
		case 219: dx = -1; dy = 0; break;
		case 218: dx = -1; dy = 0; break;
		case 217: dx = 0; dy = 1; break;
		case 216: dx = -1; dy = 9; break; // a
		case 215: dx = 0; dy = 12; break; // a
		case 213: dx = -1; dy = 0; break;
		case 179: dx = 1; dy = 5; break;
		case 177: dx = 0; dy = 2; break;
		case 176: dx = 1; dy = 0; break;
		case 173: dx = 1; dy = 0; break;
		case 171: dx = 0; dy = -3; break; // a
		case 167: dx = - 1; dy = 0; break;
		case 165: dx = 2; dy = -3; // overwrite bright yellow colors on the armor
			for (int y = 0; y < InvItemHeight[i + 1]; y++) {
				for (int x = 0; x < InvItemWidth[i + 1]; x++) {
					BYTE color = gpBuffer[x + BUFFER_WIDTH * y];
					if (color == 144 || color == 145)
						gpBuffer[x + BUFFER_WIDTH * y] = color + 96;
				}
			}
			break;
		case 164: dx = 0; dy = 4; break;// a
		case 162: dx = 0; dy = -4; break;// a
		case 161: dx = 0; dy = 1; break;
		case 159: dx = 1; dy = 1; break;
		case 158: dx = 0; dy = 3; break;
		case 157: dx = 1; dy = -1; break;
		case 156: dx = 0; dy = 1; break;
		case 154: dx = 1; dy = 0; break;
		case 153: dx = -1; dy = 1; break;
		case 152: dx = 0; dy = 2; break;
		case 151: dx = 0; dy = 2; break;// a
		case 147: dx = 0; dy = 2; break;// a
		case 146: dx = - 2; dy = -2; break;
		case 145: dx = 0; dy = -2; break;
		case 144: dx = 0; dy = 1; break;
		case 142: dx = 1; dy = 2; break;
		case 141: dx = 0; dy = 4; break;// a
		case 140: dx = 0; dy = 0;  break; //a
		case 139: dx = 1; dy = 1; break;// a
		case 138: dx = 3; dy = 1; break;
		case 137: dx = 1; dy = 0; break;
		case 135: dx = - 1; dy = 0; break;
		case 134: dx = 0; dy = 2; break;
		case 133: dx = - 1; dy = -1; break;
		case 130: dx = - 1; dy = 2; break;
		case 127: dx = 0; dy = 1; break;
		case 126: dx = 1; dy = 4; break;// a
		case 124: dx = 0; dy = 3; break;
		case 123: dx = - 1; dy = 3; break;// a
		case 120: dx = 0; dy = 1; break;
		case 119: dx = 0; dy = 1; break;
		case 116: dx = 0; dy = 1; break;
		case 115: dx = 0; dy = 4; break;// a
		case 114: dx = 0; dy = 1; break;
		case 112: dx = 0; dy = -1; break;
		case 109: dx = 3; dy = 3; break;
		case 108: dx = 2; dy = 3; break;
		case 106: dx = 0; dy = 2; break;
		case 105: dx = 1; dy = 0; break;
		case 104: dx = 0; dy = 1; break;
		case 103: dx = 0; dy = 1; break;
		case 102: dx = 0; dy = 1; break;
		case 100: dx = 0; dy = 3; break;
		case 99: dx = 1; dy = 2; break;
		case 98: dx = 0; dy = 2; break;
		case 95: dx = 1; dy = 1; break;
		case 94: dx = 1; dy = 0; break;
		case 93: dx = 0; dy = 2; break;
		case 92: dx = 0; dy = 2; break;
		case 90: dx = 0; dy = 1; break;
		case 89: dx = 1; dy = 8; break;
		case 88: dx = 0; dy = 1; break;
		case 87: dx = 0; dy = 1; break;
		case 86: dx = 0; dy = 2; break;
		case 85: dx = 0; dy = 2; break;
		case 84: dx = 1; dy = 1; break;
		case 83: dx = 0; dy = 3; break;
		case 82: dx = 1; dy = 2; break;
		case 81: dx = - 2; dy = -3; break;
		case 76: dx = -1; dy = -3; break;
		case 75: dx = -2; dy = 0; break;
		case 74: dx = 2; dy = 0; break;
		case 72: dx = -1; dy = 0; break;
		case 69: dx = 0; dy = 1; break;
		case 68: dx = 1; dy = 0; break;
		case 66: dx = 0; dy = 1; break;
		case 65: dx = 1; dy = 0; break;
		case 64: dx = 0; dy = 4; break;
		case 63: dx = 0; dy = 2; break;
		case 60: dx = 0; dy = 1; break;// ?
		case 57: dx = 0; dy = 1; break;// ?
		case 56: dx = 0; dy = 1; // make the amulet more round
			if (needsPatch) {
				gpBuffer[16 + BUFFER_WIDTH * 25] = 188;
				gpBuffer[17 + BUFFER_WIDTH * 25] = 186;
				gpBuffer[18 + BUFFER_WIDTH * 25] = 185;
				gpBuffer[19 + BUFFER_WIDTH * 25] = 184;
				gpBuffer[20 + BUFFER_WIDTH * 25] = 183;
			}
			break;
		case 55: dx = -1; dy = 3; break;
		case 52: dx = 0; dy = 1; break;
		case 51: dx = -1; dy = 0; break;
		case 49: dx = 1; dy = 0; break;
		case 43: dx = -1; dy = 0; break;
		case 42: dx = 1; dy = 0; break;
		case 39: dx = 0; dy = -1; break;
		case 36: dx = 1; dy = 0; break;
		case 37: dx = 0; dy = 1; break;
		case 35: dx = 1; dy = 0; break;
		case 34: dx = 2; dy = 1; break;
		case 33: dx = 0; dy = 1; break;
		case 32: dx = -1; dy = 1; break;
		case 31: dx = 0; dy = 1; break;
		case 30: dx = 2; dy = 0; break;
		case 27: dx = 0; dy = 1; break;
		case 26: dx = 1; dy = 0; break;
		case 25: dx = 0; dy = 1; break;
		case 24: dx = 1; dy = 0; break;
		case 22: dx = 0; dy = 1; break;
		case 20: dx = 1; dy = 2; break;
		case 19: dx = 1; dy = 1; break;
		case 13: dx = 1; dy = 0;
			needsPatch = gpBuffer[25 + BUFFER_WIDTH * 4] == TRANS_COLOR; // assume it is already done
			break;
		}

		if (needsPatch) {
			moveImage(InvItemWidth[i + 1], InvItemHeight[i + 1], dx, dy, TRANS_COLOR);
		}

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, InvItemWidth[i + 1], InvItemHeight[i + 1], SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}
#if ASSET_MPL == 1
#ifdef HELLFIRE
static BYTE* createFallgwAnim(BYTE* cl2Buf, size_t *dwLen, BYTE* stdBuf)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int numGroups = NUM_DIRS;
	constexpr int frameCount = 8;
	constexpr bool groupped = true;
	constexpr int height = 128;
	constexpr int width = 128;

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
		hdr[1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, BUFFER_WIDTH * height);

			if (ii == 6) { // DIR_E
				// duplicate the current frame
				// for (int y = 0; y < height; y++) {
				//	memset(&gpBuffer[0 + BUFFER_WIDTH * y], TRANS_COLOR, width);
				// }
				Cl2Draw(0, height - 1, frameBuf, n, width);
				// draw the west-walk frame
				const BYTE* wwFrameBuf = CelGetFrameStart(cl2Buf, 2); // DIR_W
				// for (int y = 0; y < height; y++) {
				//	memset(&gpBuffer[width + BUFFER_WIDTH * y], TRANS_COLOR, width);
				// }
				Cl2Draw(width, height - 1, wwFrameBuf, n, width);

				int i = n - 1;
				// mirror the west-walk frame
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						unsigned addr = x + BUFFER_WIDTH * y;
						unsigned addr2 = width + (width - x - 1) + BUFFER_WIDTH * y;
						BYTE color = gpBuffer[addr2];
						if (color != TRANS_COLOR) {
							if ((color >= 170 && color <= 175) || (color >= 190 && color <= 205) || color >= 251) {
								if (i == 0) {
									if (x >= 71 && y >= 99 && y <= 112) {
										continue;
									}
								}
								if (i == 1) {
									if (x >= 67 && y >= 101 && y <= 110) {
										continue;
									}
								}
								if (i == 2) {
									if (x >= 62 && y >= 105 && y <= 114) {
										continue;
									}
								}
								if (i == 3) {
									if (x >= 58 && y >= 109 && y <= 118) {
										continue;
									}
								}
								if (i == 4) {
									if (x >= 57 && y >= 110 && y <= 121) {
										continue;
									}
								}
								if (i == 5) {
									if (x >= 58 && y >= 110 && y <= 119) {
										continue;
									}
								}
								if (i == 6) {
									if (x >= 62 && y >= 106 && y <= 114) {
										continue;
									}
								}
								if (i == 7) {
									if (x >= 66 && y >= 96 && y <= 112) {
										continue;
									}
								}
								/*if (x >= 62 && y >= 99 && y <= 121) {
									continue;
								}*/
							}
							if (color == 0) {
								if (i == 0) {
									if (/*x >= 87 || */(x >= 81 && y >= 118 + 10 - x / 8)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 1) {
									if (/*x >= 87 || */(x >= 79 && y >= 118 + 10 - x / 8)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 2) {
									if (/*x >= 87 || */(x >= 75 && y >= 118 + 10 - x / 8)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 3) {
									if (/*x >= 87 || */(x >= 70 && y >= 118)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 4) {
									if (/*x >= 87 || */(x >= 67 && y >= 119)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 5) {
									if (/*x >= 87 || */(x >= 71 && y >= 119)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 6) {
									if (/*x >= 87 || */(x >= 72 && y >= 117 && y >= 120 + 72 - x)) {
										color = TRANS_COLOR;
									}
								}
								if (i == 7) {
									if (/*x >= 87 || */(x >= 85 && y <= 118 && y >= 118 + 85 - x)) {
										color = TRANS_COLOR;
									}
								}
								/*if (x >= 65 && y >= 115 && y <= 122) {
									continue;
								}*/
							}
						}
						gpBuffer[addr] = color;
					}
				}

				// copy the club from the stand frame
				int fn, dx, dy;
				switch (i) {
				case 0: fn = 8; dx = -11; dy = 11; break;
				case 1: fn = 9; dx =  -8; dy =  9; break;
				case 2: fn = 9; dx =  -7; dy =  9; break;
				case 3: fn = 9; dx =  -5; dy =  9; break;
				case 4: fn = 9; dx =  -4; dy =  9; break;
				case 5: fn = 9; dx =  -5; dy =  8; break;
				case 6: fn = 8; dx =  -2; dy =  5; break;
				case 7: fn = 9; dx =  -8; dy =  8; break;
				}
				const BYTE* stdFrameBuf = CelGetFrameStart(stdBuf, ii);
				for (int y = 0; y < height; y++) {
					memset(&gpBuffer[width + BUFFER_WIDTH * y], TRANS_COLOR, width);
				}
				Cl2Draw(width, height - 1, stdFrameBuf, fn + 1, width);

				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						unsigned addr = x + dx + BUFFER_WIDTH * (y + dy);
						unsigned addr2 = width + x + BUFFER_WIDTH * y;
						BYTE color = gpBuffer[addr2];
						if (color == TRANS_COLOR) {
							continue;
						}
						switch (fn) {
						case 7:
							if (x < 60 || y < 88 || y > 111 || (color != 0 && !(color >= 170 && color <= 175) && !(color >= 188 && color <= 205) && color != 223 && color != 251 && color != 252))
								continue;
							break;
						case 8:
						case 9:
							if (x < 80 || y < 86 || y > 109 || (color != 0 && !(color >= 170 && color <= 175) && !(color >= 188 && color <= 205) && color != 223 && color != 251 && color != 252))
								continue;
							break;
						}
						BYTE curr_color = gpBuffer[addr];
						if (curr_color == TRANS_COLOR
							|| (i == 0 && curr_color == 0 && x >= 75 + 11)) {
							gpBuffer[addr] = color;
						}
					}
				}

				// fix artifacts
				switch (i) {
				case 0: dx = 85; break;
				case 1: dx = 92; break;
				case 2: dx = 93; break;
				case 3: dx = 95; break;
				case 4: dx = 96; break;
				case 5: dx = 95; break;
				case 6: dx = 94; break;
				case 7: dx = 93; break;
				}
				for (int y = 85; y < height; y++) {
					for (int x = dx; x < width; x++) {
						gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
					}
				}

				if (i == 0) {
					gpBuffer[76 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 110] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 110] = TRANS_COLOR; // color0)
					gpBuffer[78 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[82 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[83 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[82 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[83 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[69 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 118] = 0; // was tp
				}
				if (i == 1) {
					for (int y = 106; y < 114; y++) {
						for (int x = 73; x < 86; x++) {
							if (y < 276 - 2 * x) {
								gpBuffer[x + BUFFER_WIDTH * y] = 0;
							}
						}
					}
					for (int y = 105; y < 109; y++) {
						for (int x = 84; x < 88; x++) {
							if (x != 87 || (y != 105 || y != 108)) {
								gpBuffer[x + BUFFER_WIDTH * y] = 0;
							}
						}
					}
					gpBuffer[73 + BUFFER_WIDTH * 100] = 175; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 100] = 175; // was tp
					gpBuffer[91 + BUFFER_WIDTH * 102] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color191)
					gpBuffer[84 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[82 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[83 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color207)
					gpBuffer[82 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[82 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[83 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[82 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[83 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[76 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[77 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[78 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[79 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[74 + BUFFER_WIDTH * 119] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 119] = TRANS_COLOR; // color0)
					gpBuffer[76 + BUFFER_WIDTH * 119] = TRANS_COLOR; // color0)
				}
				if (i == 2) {
					gpBuffer[71 + BUFFER_WIDTH * 96] = 238; // was color191)
					gpBuffer[71 + BUFFER_WIDTH * 97] = 237; // was color191)
					gpBuffer[72 + BUFFER_WIDTH * 97] = 236; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 97] = 237; // was color191)
					gpBuffer[75 + BUFFER_WIDTH * 97] = 236; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 97] = 236; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 98] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 98] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 98] = 236; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 98] = 236; // was color203)
					gpBuffer[70 + BUFFER_WIDTH * 99] = 238; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 99] = 236; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 99] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 99] = 238; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 99] = 238; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 100] = 235; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 100] = 238; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 100] = 238; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 100] = 236; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 101] = 236; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 101] = 238; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 101] = 236; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 101] = 173; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 101] = 173; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 102] = 237; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 102] = 237; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 102] = 236; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 102] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 102] = 238; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 102] = 236; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 103] = 238; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 103] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 103] = 238; // was tp
					gpBuffer[92 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 104] = 238; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 104] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 104] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 104] = 236; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 105] = 235; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 105] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 105] = 236; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 105] = 238; // was color170)
					gpBuffer[85 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 106] = 236; // was color238)
					gpBuffer[71 + BUFFER_WIDTH * 106] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 106] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 106] = 238; // was tp
					gpBuffer[92 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[71 + BUFFER_WIDTH * 107] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 107] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 107] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 107] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 107] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 107] = 0; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 108] = 238; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 108] = 236; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 108] = 238; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 108] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 108] = 0; // was color191)
					gpBuffer[76 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 109] = 238; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 109] = 238; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 109] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color207)
					gpBuffer[73 + BUFFER_WIDTH * 110] = 0; // was color189)
					gpBuffer[74 + BUFFER_WIDTH * 110] = 0; // was color189)
					gpBuffer[75 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 110] = TRANS_COLOR; // color223)
					gpBuffer[71 + BUFFER_WIDTH * 111] = 237; // was color0)
					gpBuffer[83 + BUFFER_WIDTH * 111] = TRANS_COLOR; // color0)
					gpBuffer[71 + BUFFER_WIDTH * 112] = 237; // was color0)
					gpBuffer[72 + BUFFER_WIDTH * 112] = 237; // was color0)
					gpBuffer[73 + BUFFER_WIDTH * 112] = 238; // was color0)
					gpBuffer[74 + BUFFER_WIDTH * 112] = 237; // was color0)
					gpBuffer[81 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 113] = 236; // was color0)
					gpBuffer[71 + BUFFER_WIDTH * 113] = 236; // was color0)
					gpBuffer[72 + BUFFER_WIDTH * 113] = 237; // was color0)
					gpBuffer[73 + BUFFER_WIDTH * 113] = 237; // was color0)
					gpBuffer[83 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[89 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[73 + BUFFER_WIDTH * 114] = 236; // was color0)
					gpBuffer[85 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[79 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
				}
				if (i == 3) { // 52
					gpBuffer[68 + BUFFER_WIDTH * 87] = 237; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 87] = 237; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 88] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 88] = 237; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 89] = 237; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 89] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 90] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 91] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 91] = 175; // was color237)
					gpBuffer[77 + BUFFER_WIDTH * 91] = TRANS_COLOR; // color237)
					gpBuffer[77 + BUFFER_WIDTH * 92] = 175; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 94] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 95] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 95] = 175; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 96] = 235; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 96] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 96] = 237; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 96] = 237; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 97] = 235; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 97] = 203; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 97] = 203; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 97] = 237; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 98] = 235; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 98] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 99] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 99] = 174; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 99] = 203; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 99] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 99] = 203; // was color170)
					gpBuffer[73 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 100] = 174; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 100] = 174; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 101] = 174; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 101] = 175; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 101] = 172; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 102] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 102] = 238; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 103] = 238; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 103] = 237; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 103] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 103] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color189)
					gpBuffer[76 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color188)
					gpBuffer[86 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[66 + BUFFER_WIDTH * 104] = 237; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 104] = 237; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 104] = 235; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 104] = 235; // was color207)
					gpBuffer[81 + BUFFER_WIDTH * 104] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[66 + BUFFER_WIDTH * 105] = 237; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 105] = 237; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 105] = 235; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 105] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color170)
					gpBuffer[84 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[66 + BUFFER_WIDTH * 106] = 237; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 106] = 235; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 106] = 235; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 106] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 106] = 237; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[66 + BUFFER_WIDTH * 107] = 235; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 107] = 237; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 107] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 107] = 237; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[66 + BUFFER_WIDTH * 108] = 237; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 108] = 235; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 108] = 235; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[67 + BUFFER_WIDTH * 109] = 237; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 109] = 235; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 109] = 238; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[68 + BUFFER_WIDTH * 110] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 110] = 238; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 110] = 0; // was 235
					gpBuffer[80 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 111] = 238; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 111] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 112] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 112] = 237; // was color0
					gpBuffer[84 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 113] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[72 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 114] = 0; // was color188
					gpBuffer[88 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[72 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 118] = 0; // was tp
				}
				if (i == 4) { // 53
					gpBuffer[72 + BUFFER_WIDTH * 89] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 90] = 238; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 91] = 238; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 91] = 238; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 92] = 238; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 94] = 238; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 95] = 238; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 95] = 237; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 95] = 237; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 95] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 96] = 236; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 96] = 237; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 96] = 236; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 96] = 237; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 96] = 234; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 97] = 237; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 97] = 203; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 97] = 236; // was tp
					gpBuffer[65 + BUFFER_WIDTH * 98] = 238; // was color190)
					gpBuffer[66 + BUFFER_WIDTH * 98] = 238; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 98] = 238; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 98] = 237; // was tp
					gpBuffer[65 + BUFFER_WIDTH * 99] = 236; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 99] = 236; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 99] = 238; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 99] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 99] = 237; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 100] = 236; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 100] = 236; // was tp
					gpBuffer[65 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 101] = 236; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 102] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 103] = 237; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color188)
					gpBuffer[75 + BUFFER_WIDTH * 104] = 236; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[95 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 105] = 236; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[95 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 106] = 237; // was color0)
					gpBuffer[83 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[95 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[74 + BUFFER_WIDTH * 107] = 237; // was color0)
					gpBuffer[84 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[95 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 108] = 237; // was color0)
					gpBuffer[76 + BUFFER_WIDTH * 108] = 237; // was color0)
					gpBuffer[86 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 109] = 236; // was color0)
					gpBuffer[76 + BUFFER_WIDTH * 109] = 236; // was color0)
					gpBuffer[86 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 109] = TRANS_COLOR; // color0)
					gpBuffer[76 + BUFFER_WIDTH * 110] = 219; // was color0)
					gpBuffer[77 + BUFFER_WIDTH * 110] = 236; // was color0)
					gpBuffer[78 + BUFFER_WIDTH * 110] = 237; // was color0)
					gpBuffer[77 + BUFFER_WIDTH * 111] = 237; // was color0)
					gpBuffer[78 + BUFFER_WIDTH * 111] = 237; // was color0)
					gpBuffer[79 + BUFFER_WIDTH * 111] = 236; // was color0)
					gpBuffer[80 + BUFFER_WIDTH * 111] = 237; // was color188)
					gpBuffer[81 + BUFFER_WIDTH * 111] = 236; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 111] = TRANS_COLOR; // color0)
					gpBuffer[77 + BUFFER_WIDTH * 112] = 237; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 112] = 237; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 112] = 236; // was color219)
					gpBuffer[83 + BUFFER_WIDTH * 112] = 237; // was color219)
					gpBuffer[85 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[64 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[65 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[66 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[69 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[72 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 115] = 0; // was color172)
					gpBuffer[89 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[60 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[61 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[62 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 116] = 0; // was color172)
					gpBuffer[89 + BUFFER_WIDTH * 116] = TRANS_COLOR; // color0)
					gpBuffer[69 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[80 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[76 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[77 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[56 + BUFFER_WIDTH * 120] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 120] = TRANS_COLOR; // color223)
				}
				if (i == 5) { // 54
					gpBuffer[74 + BUFFER_WIDTH * 94] = 203; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 95] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 95] = 236; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 95] = 173; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 95] = 172; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 95] = 203; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 96] = 174; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 96] = 173; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 96] = 203; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 97] = 172; // was color235)
					gpBuffer[75 + BUFFER_WIDTH * 97] = 171; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 98] = 172; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 98] = 171; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 99] = 237; // was color0)
					gpBuffer[75 + BUFFER_WIDTH * 99] = 171; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 99] = 203; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 99] = 203; // was tp
					// gpBuffer[78 + BUFFER_WIDTH * 99] = 171; // was color174)
					gpBuffer[73 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 100] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 100] = 172; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 100] = 171; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 102] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 102] = 220; // was tp
					gpBuffer[92 + BUFFER_WIDTH * 102] = TRANS_COLOR; // color0)
					gpBuffer[73 + BUFFER_WIDTH * 103] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 103] = 220; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 103] = 172; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[73 + BUFFER_WIDTH * 104] = 220; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 104] = 235; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[73 + BUFFER_WIDTH * 105] = 236; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 105] = 235; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 105] = 172; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[72 + BUFFER_WIDTH * 106] = 236; // was color191)
					gpBuffer[73 + BUFFER_WIDTH * 106] = 236; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[94 + BUFFER_WIDTH * 108] = TRANS_COLOR; // color0)
					gpBuffer[69 + BUFFER_WIDTH * 110] = 236; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 110] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 110] = 235; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 110] = TRANS_COLOR; // color188)
					gpBuffer[70 + BUFFER_WIDTH * 111] = 237; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 111] = 236; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 111] = 236; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 112] = 236; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 112] = 236; // was color0)
					gpBuffer[82 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 113] = 237; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 113] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 113] = 236; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 113] = 237; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 113] = 236; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 113] = 236; // was color0)
					gpBuffer[77 + BUFFER_WIDTH * 113] = 236; // was color0)
					gpBuffer[89 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[69 + BUFFER_WIDTH * 114] = 236; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 114] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 114] = 236; // was tp
					gpBuffer[89 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[73 + BUFFER_WIDTH * 115] = 236; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 115] = TRANS_COLOR; // color0)
					gpBuffer[81 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[90 + BUFFER_WIDTH * 117] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 118] = TRANS_COLOR; // color0)
					gpBuffer[60 + BUFFER_WIDTH * 119] = 0; // was tp
					gpBuffer[57 + BUFFER_WIDTH * 120] = 0; // was tp
					gpBuffer[58 + BUFFER_WIDTH * 120] = 0; // was tp
				}
				if (i == 6) { // 55
					gpBuffer[70 + BUFFER_WIDTH * 90] = 173; // was color237)
					gpBuffer[71 + BUFFER_WIDTH * 91] = 235; // was color237)
					gpBuffer[72 + BUFFER_WIDTH * 91] = 235; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 92] = 235; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 93] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 93] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 93] = 173; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 93] = 173; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 94] = 173; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 94] = 173; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 94] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 94] = 173; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 95] = 221; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 95] = 173; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 95] = 173; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 95] = 203; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 95] = 171; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 95] = 203; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 95] = 252; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 96] = 173; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 96] = 203; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 96] = 171; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 96] = 203; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 96] = 203; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 96] = 171; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 96] = 171; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 96] = 171; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 96] = 171; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 97] = 237; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 97] = 203; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 97] = 171; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 97] = 171; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 97] = 171; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 97] = 171; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 97] = 203; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 98] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 98] = 237; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 98] = 171; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 98] = 203; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 98] = 203; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 98] = 203; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 99] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 99] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 99] = 171; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 99] = 171; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 99] = 203; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 100] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 100] = 171; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 101] = 237; // was tp
					gpBuffer[93 + BUFFER_WIDTH * 102] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[70 + BUFFER_WIDTH * 104] = 237; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 104] = 237; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 104] = 236; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[69 + BUFFER_WIDTH * 105] = 237; // was tp
					gpBuffer[70 + BUFFER_WIDTH * 105] = 237; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[68 + BUFFER_WIDTH * 106] = 237; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 106] = 236; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 106] = TRANS_COLOR; // color0)
					gpBuffer[68 + BUFFER_WIDTH * 107] = 236; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color191)
					gpBuffer[93 + BUFFER_WIDTH * 107] = TRANS_COLOR; // color0)
					gpBuffer[61 + BUFFER_WIDTH * 108] = 236; // was tp
					gpBuffer[62 + BUFFER_WIDTH * 108] = 236; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 108] = 237; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[59 + BUFFER_WIDTH * 109] = 236; // was tp
					gpBuffer[68 + BUFFER_WIDTH * 109] = 237; // was tp
					gpBuffer[69 + BUFFER_WIDTH * 109] = 236; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[71 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[72 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 110] = 0; // was color171)
					gpBuffer[79 + BUFFER_WIDTH * 110] = 0; // was color188)
					gpBuffer[80 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 110] = TRANS_COLOR; // color207)
					gpBuffer[77 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 112] = TRANS_COLOR; // color0)
					gpBuffer[76 + BUFFER_WIDTH * 113] = 237; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 113] = TRANS_COLOR; // color0)
					gpBuffer[75 + BUFFER_WIDTH * 114] = 237; // was tp
					gpBuffer[90 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[93 + BUFFER_WIDTH * 114] = TRANS_COLOR; // color0)
					gpBuffer[66 + BUFFER_WIDTH * 115] = 237; // was tp
					gpBuffer[67 + BUFFER_WIDTH * 115] = 237; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[88 + BUFFER_WIDTH * 115] = 0; // was tp
					gpBuffer[64 + BUFFER_WIDTH * 116] = 237; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 116] = 0; // was tp
					gpBuffer[73 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[74 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[87 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[64 + BUFFER_WIDTH * 118] = 237; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 118] = 0; // was tp
				}
				if (i == 7) { // 56
					gpBuffer[90 + BUFFER_WIDTH * 102] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 102] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 102] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[90 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[91 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 103] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[88 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[89 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color0)
					gpBuffer[92 + BUFFER_WIDTH * 104] = TRANS_COLOR; // color191)
					gpBuffer[83 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[84 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[85 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[86 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[87 + BUFFER_WIDTH * 105] = TRANS_COLOR; // color0)
					gpBuffer[82 + BUFFER_WIDTH * 107] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 107] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 108] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[78 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 109] = 0; // was tp
					gpBuffer[75 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[76 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[77 + BUFFER_WIDTH * 110] = 0; // was tp
					gpBuffer[79 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 111] = 0; // was tp
					gpBuffer[80 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[81 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[82 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[83 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 112] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 113] = 0; // was tp
					gpBuffer[86 + BUFFER_WIDTH * 114] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 117] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 118] = 0; // was tp
					gpBuffer[85 + BUFFER_WIDTH * 119] = 0; // was tp
					gpBuffer[84 + BUFFER_WIDTH * 120] = 0; // was tp
				}
			} else {
				Cl2Draw(0, height - 1, frameBuf, n, width);
			}

			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));
		}
		hdr += ni + 2;
	}

	*dwLen = (size_t)pBuf - (size_t)resCl2Buf;

	mem_free_dbg(cl2Buf);
	return resCl2Buf;
}

static BYTE* fixGoatLdAnim(BYTE* cl2Buf, size_t *dwLen)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int numGroups = NUM_DIRS;
	constexpr int frameCount = 16;
	constexpr bool groupped = true;
	constexpr int height = 128;
	constexpr int width = 160;

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
	bool needsPatch = false;
	for (int ii = 0; ii < numGroups; ii++) {
		int ni = frameCount;
		hdr[0] = SwapLE32(ni);
		hdr[1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));

		const BYTE* frameBuf = CelGetFrameStart(cl2Buf, ii);

		for (int n = 1; n <= ni; n++) {
			memset(&gpBuffer[0], TRANS_COLOR, BUFFER_WIDTH * height);
			// draw the frame to the buffer
			Cl2Draw(0, height - 1, frameBuf, n, width);
			// test if the animation is already patched
			if (ii == 1 && n == 9) {
				needsPatch = gpBuffer[71 + BUFFER_WIDTH * 127] != TRANS_COLOR; // assume it is already done
			}

			if (needsPatch) {
				switch (ii) {
				case DIR_SW: {
					switch (n) {
					case 9:
					case 10:
					case 11:
					case 12: {
						// shift the monster with (0;16) up
						for (int y = 16; y < height; y++) {
							for (int x = 0; x < width; x++) {
								unsigned addr = x + BUFFER_WIDTH * y;
								unsigned addr2 = x + BUFFER_WIDTH * (y - 16);
								BYTE color = gpBuffer[addr];
								if (color == TRANS_COLOR)
									continue;
								gpBuffer[addr2] = color;
								gpBuffer[addr] = TRANS_COLOR;
							}
						}
						// copy pixels from the followup frames
						if (n != 9) {
							Cl2Draw(width, height - 1, frameBuf, n + 4, width);
							for (int y = 4 - 1; y >= 0; y--) {
								for (int x = 0; x < width; x++) {
									unsigned addr = width + x + BUFFER_WIDTH * y;
									unsigned addr2 = x + BUFFER_WIDTH * (y + 112);
									BYTE color = gpBuffer[addr];
									if (color == TRANS_COLOR)
										continue;
									gpBuffer[addr2] = color;
								}
							}
						}
					} break;
					case 14:
					case 15:
					case 16: {
						// clear pixels from the first rows
						for (int y = 4 - 1; y >= 0; y--) {
							for (int x = 0; x < width; x++) {
								gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
							}
						}
					} break;
					}
				} break;
				case DIR_W: {
					switch (n) {
					case 9:
					case 10:
					case 11:
					case 12: {
						// shift the monster with (0;16) up
						for (int y = 16; y < height; y++) {
							for (int x = 0; x < width; x++) {
								unsigned addr = x + BUFFER_WIDTH * y;
								unsigned addr2 = x + BUFFER_WIDTH * (y - 16);
								BYTE color = gpBuffer[addr];
								if (color == TRANS_COLOR)
									continue;
								gpBuffer[addr2] = color;
								gpBuffer[addr] = TRANS_COLOR;
							}
						}
						// copy pixels from the followup frames
						{
							Cl2Draw(width, height - 1, frameBuf, n + 4, width);
							for (int y = 16 - 1; y >= 0; y--) {
								for (int x = 0; x < width; x++) {
									unsigned addr = width + x + BUFFER_WIDTH * y;
									unsigned addr2 = x + BUFFER_WIDTH * (y + 112);
									BYTE color = gpBuffer[addr];
									if (color == TRANS_COLOR)
										continue;
									gpBuffer[addr2] = color;
								}
							}
						}
					} break;
					case 13:
					case 14:
					case 15:
					case 16: {
						// shift the monster with (1;9) up/right
						for (int y = 16; y < height; y++) {
							for (int x = width - 1 - 1; x >= 0; x--) {
								unsigned addr = x + BUFFER_WIDTH * y;
								unsigned addr2 = x + 1 + BUFFER_WIDTH * (y - 9);
								BYTE color = gpBuffer[addr];
								if (color == TRANS_COLOR)
									continue;
								gpBuffer[addr2] = color;
								gpBuffer[addr] = TRANS_COLOR;
							}
						}
						// clear pixels from the first rows
						for (int y = 16 - 1; y >= 0; y--) {
							for (int x = 0; x < width; x++) {
								gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
							}
						}
					} break;
					}
				} break;
				case DIR_NW: {
					switch (n) {
					case 12:
					case 13:
					case 14:
					case 15:
					case 16: {
						// shift the monster with (0;4) down
						for (int y = height - 4 - 1; y >= 0; y--) {
							for (int x = width - 1; x >= 0; x--) {
								unsigned addr = x + BUFFER_WIDTH * y;
								unsigned addr2 = x + BUFFER_WIDTH * (y + 4);
								BYTE color = gpBuffer[addr];
								if (color == TRANS_COLOR)
									continue;
								gpBuffer[addr2] = color;
								gpBuffer[addr] = TRANS_COLOR;
							}
						}
					} break;
					}
				} break;
				case DIR_E: {
					switch (n) {
					case 9:
					case 10:
					case 11:
					case 12: {
						// clear pixels from the first rows
						for (int y = 4 - 1; y >= 0; y--) {
							for (int x = 0; x < width; x++) {
								gpBuffer[x + BUFFER_WIDTH * y] = TRANS_COLOR;
							}
						}
					} break;
					}
				} break;
				case DIR_SE: {
					switch (n) {
					case 12:
					case 13:
					case 14:
					case 15:
					case 16: {
						// shift the monster with (0;16) up
						for (int y = 16; y < height; y++) {
							for (int x = 0; x < width; x++) {
								unsigned addr = x + BUFFER_WIDTH * y;
								unsigned addr2 = x + BUFFER_WIDTH * (y - 16);
								BYTE color = gpBuffer[addr];
								if (color == TRANS_COLOR)
									continue;
								gpBuffer[addr2] = color;
								gpBuffer[addr] = TRANS_COLOR;
							}
						}
					} break;
					}
				} break;
				}
			}

			BYTE* frameSrc = &gpBuffer[0 + (height - 1) * BUFFER_WIDTH];

			pBuf = EncodeCl2(pBuf, frameSrc, width, height, TRANS_COLOR);
			hdr[n + 1] = SwapLE32((DWORD)((size_t)pBuf - (size_t)hdr));
		}
		hdr += ni + 2;
	}

	*dwLen = (size_t)pBuf - (size_t)resCl2Buf;

	mem_free_dbg(cl2Buf);
	return resCl2Buf;
}

#endif // HELLFIRE
static BYTE* patchFloorItems(int fileIndex, BYTE* celBuf, size_t* celLen)
{
	constexpr BYTE TRANS_COLOR = 1;
	constexpr int FRAME_WIDTH = 96;
	int FRAME_HEIGHT = (fileIndex == FILE_ITEM_CROWNF || fileIndex == FILE_ITEM_FEAR || fileIndex == FILE_ITEM_LARMOR || fileIndex == FILE_ITEM_WSHIELD) ? 128 : 160;

	DWORD* srcHeaderCursor = (DWORD*)celBuf;
	int srcCelEntries = SwapLE32(srcHeaderCursor[0]);
	srcHeaderCursor++;

	// create the new CEL file
	size_t maxCelSize = 2 * *celLen;
	BYTE* resCelBuf = DiabloAllocPtr(maxCelSize);
	memset(resCelBuf, 0, maxCelSize);

	DWORD* dstHeaderCursor = (DWORD*)resCelBuf;
	*dstHeaderCursor = SwapLE32(srcCelEntries);
	dstHeaderCursor++;

	BYTE* dstDataCursor = resCelBuf + 4 * (srcCelEntries + 2);
	for (int i = 0; i < srcCelEntries; i++) {
		// draw the frame to the back-buffer
		memset(&gpBuffer[0], TRANS_COLOR, (size_t)FRAME_HEIGHT * BUFFER_WIDTH);
		CelClippedDraw(0, FRAME_HEIGHT - 1, celBuf, i + 1, FRAME_WIDTH);

		// center frames
		// - shift crown, larmor, wshield (-12;0), ear (-16;0)
		if (fileIndex == FILE_ITEM_CROWNF || fileIndex == FILE_ITEM_FEAR || fileIndex == FILE_ITEM_LARMOR || fileIndex == FILE_ITEM_WSHIELD) {
			// check if it is already done
			if (i == 0) {
				for (int y = 0; y < FRAME_HEIGHT; y++) {
					for (int x = 0; x < FRAME_WIDTH / 2 - 1; x++) {
						if (gpBuffer[x + y * BUFFER_WIDTH] == TRANS_COLOR) continue;
						mem_free_dbg(resCelBuf);
						return celBuf; // assume it is already done
					}
				}
			}
			for (int y = 0; y < FRAME_HEIGHT; y++) {
				for (int x = 16; x < FRAME_WIDTH; x++) {
					gpBuffer[(x - (fileIndex == FILE_ITEM_FEAR ? 16 : 12)) + y * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift mace (+2;-2)
		if (fileIndex == FILE_ITEM_MACE) {
			// check if it is already done
			if (i == 0 && gpBuffer[41 + 91 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = 2; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH - 1 - 2; x >= 0; x--) {
					gpBuffer[(x + 2) + (y - 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift scroll (0;-2)
		if (fileIndex == FILE_ITEM_SCROLL) {
			// check if it is already done
			if (i == 0 && gpBuffer[51 + 94 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = 2; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH - 1 - 0; x >= 0; x--) {
					gpBuffer[(x + 0) + (y - 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift ring (0;-3)
		if (fileIndex == FILE_ITEM_RING) {
			// check if it is already done
			if (i == 0 && gpBuffer[45 + 87 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = 3; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH - 1 - 0; x >= 0; x--) {
					gpBuffer[(x + 0) + (y - 3) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift staff (-7;+5)
		if (fileIndex == FILE_ITEM_STAFF) {
			// check if it is already done
			if (i == 0 && gpBuffer[53 + 52 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 5; y >= 0; y--) {
				for (int x = 7; x < FRAME_WIDTH; x++) {
					gpBuffer[(x - 7) + (y + 5) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift brain (+5;+11)
		if (fileIndex == FILE_ITEM_FBRAIN) {
			// check if it is already done
			if (i == 0 && gpBuffer[40 + 85 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 11; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 5; x >= 0; x--) {
					gpBuffer[(x + 5) + (y + 11) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift mushroom (0;+6)
		if (fileIndex == FILE_ITEM_FMUSH) {
			// check if it is already done
			if (i == 0 && gpBuffer[43 + 83 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 6; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 0; x >= 0; x--) {
					gpBuffer[(x + 0) + (y + 6) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift innsign (+14;+8)
		if (fileIndex == FILE_ITEM_INNSIGN) {
			// check if it is already done
			if (i == 0 && gpBuffer[18 + 96 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 8; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 14; x >= 0; x--) {
					gpBuffer[(x + 14) + (y + 8) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift bloodstone (0;+5)
		if (fileIndex == FILE_ITEM_BLDSTN) {
			// check if it is already done
			if (i == 0 && gpBuffer[45 + 75 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 5; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 0; x >= 0; x--) {
					gpBuffer[(x + 0) + (y + 5) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift anvil (+3;+6)
		if (fileIndex == FILE_ITEM_FANVIL) {
			// check if it is already done
			if (i == 0 && gpBuffer[22 + 80 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 6; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 3; x >= 0; x--) {
					gpBuffer[(x + 3) + (y + 6) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift lazarus's staff (-3;+8)
		if (fileIndex == FILE_ITEM_FLAZSTAF) {
			// check if it is already done
			if (i == 0 && gpBuffer[30 + 58 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 8; y >= 0; y--) {
				for (int x = 3; x < FRAME_WIDTH; x++) {
					gpBuffer[(x - 3) + (y + 8) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
			// fix the shadow of the staff
			if (i == 6) {
				gpBuffer[51 + 127 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[63 + 131 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[64 + 131 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[35 + 140 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[36 + 140 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[39 + 140 * BUFFER_WIDTH] = 0;
			}
			if (i == 7) {
				for (int x = 27; x < 38; x++)
					gpBuffer[x + 140 * BUFFER_WIDTH] = TRANS_COLOR;
				for (int x = 62; x < 66; x++)
					gpBuffer[x + 139 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[73 + 140 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[74 + 140 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[81 + 139 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[48 + 140 * BUFFER_WIDTH] = 0;
				gpBuffer[49 + 140 * BUFFER_WIDTH] = 0;
			}
		}
		// - shift armor (0;-2)
		if (fileIndex == FILE_ITEM_ARMOR2) {
			// check if it is already done
			if (i == 0 && gpBuffer[29 + 111 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = 2; y < FRAME_HEIGHT; y++) {
				for (int x = FRAME_WIDTH - 1; x >= 0; x--) {
					gpBuffer[(x + 0) + (y - 2) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
			// mask shadow
			if (i == 14) {
				gpBuffer[22 + 147 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[23 + 147 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[20 + 148 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[21 + 148 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[22 + 148 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[23 + 148 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[23 + 149 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[24 + 149 * BUFFER_WIDTH] = TRANS_COLOR;
				gpBuffer[25 + 149 * BUFFER_WIDTH] = TRANS_COLOR;
			}
		}
#ifdef HELLFIRE
		// - shift cowhide (+2;+4)
		if (fileIndex == FILE_ITEM_COWS1) {
			// check if it is already done
			if (i == 0 && gpBuffer[30 + 78 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 4; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 2; x >= 0; x--) {
					gpBuffer[(x + 2) + (y + 4) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift last frame of donkeyhide (+2;+4)
		if (i == 14 && fileIndex == FILE_ITEM_DONKYS1) {
			// check if it is already done
			if (gpBuffer[40 + 119 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 4; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 2; x >= 0; x--) {
					gpBuffer[(x + 2) + (y + 4) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift moosehide (0;+3)
		if (fileIndex == FILE_ITEM_MOOSES1) {
			// check if it is already done
			if (i == 0 && gpBuffer[44 + 83 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 3; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 0; x >= 0; x--) {
					gpBuffer[(x + 0) + (y + 3) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
		// - shift teddy (0;+6)
		if (fileIndex == FILE_ITEM_TEDDYS1) {
			// check if it is already done
			if (i == 0 && gpBuffer[46 + 100 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			for (int y = FRAME_HEIGHT - 1 - 6; y >= 0; y--) {
				for (int x = FRAME_WIDTH - 1 - 0; x >= 0; x--) {
					gpBuffer[(x + 0) + (y + 6) * BUFFER_WIDTH] = gpBuffer[x + y * BUFFER_WIDTH];
					gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
				}
			}
		}
#endif
		// reduce gold stack
		if (fileIndex == FILE_ITEM_GOLDFLIP) {
			// check if it is already done
			if (i == 4 && gpBuffer[22 + 147 * BUFFER_WIDTH] == TRANS_COLOR) {
				mem_free_dbg(resCelBuf);
				return celBuf; // assume it is already done
			}
			if (i == 4) {
				for (int y = 0; y < FRAME_HEIGHT; y++) {
					for (int x = 0; x < FRAME_WIDTH; x++) {
						if (y >= 144 + x - 27)
							gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			if (i == 5) {
				for (int y = 0; y < FRAME_HEIGHT; y++) {
					for (int x = 0; x < FRAME_WIDTH; x++) {
						if (x <= 23)
							gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			if (i == 6) {
				for (int y = 0; y < FRAME_HEIGHT; y++) {
					for (int x = 0; x < FRAME_WIDTH; x++) {
						if (y >= 150 - x + 63 || y >= 146 + x - 24)
							gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			if (i == 7) {
				for (int y = 0; y < FRAME_HEIGHT; y++) {
					for (int x = 0; x < FRAME_WIDTH; x++) {
						if (x <= 23 || y >= 152 - x + 63 || y >= 146 + x - 23 || (x <= 34 && y >= 150))
							gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			if (i == 8 || i == 9) {
				for (int y = 0; y < FRAME_HEIGHT; y++) {
					for (int x = 0; x < FRAME_WIDTH; x++) {
						if (y >= 152 - x + 63 || y >= 146 + x - 23 || (x <= 34 && y >= 150) || (x <= 39 && y >= 152) || (x >= 57 && y >= 153) || (x <= 34 && y <= 136))
							gpBuffer[x + y * BUFFER_WIDTH] = TRANS_COLOR;
					}
				}
			}
			if (i == 9) {
				gpBuffer[37 + 149 * BUFFER_WIDTH] = 203; // (was color204)
				gpBuffer[38 + 149 * BUFFER_WIDTH] = 198; // (was color204)
				gpBuffer[37 + 150 * BUFFER_WIDTH] = 199; // (was transparent)
				gpBuffer[38 + 150 * BUFFER_WIDTH] = 196; // (was transparent)
				gpBuffer[39 + 150 * BUFFER_WIDTH] = 196; // (was color204)
				gpBuffer[40 + 150 * BUFFER_WIDTH] = 197; // (was color204)
				gpBuffer[37 + 151 * BUFFER_WIDTH] = 204; // (was transparent)
				gpBuffer[38 + 151 * BUFFER_WIDTH] = 201; // (was transparent)
				gpBuffer[39 + 151 * BUFFER_WIDTH] = 198; // (was transparent)
				gpBuffer[40 + 151 * BUFFER_WIDTH] = 204; // (was transparent)
				gpBuffer[40 + 153 * BUFFER_WIDTH] = TRANS_COLOR; // (was color203)
				gpBuffer[41 + 153 * BUFFER_WIDTH] = TRANS_COLOR; // (was color198)
				gpBuffer[42 + 153 * BUFFER_WIDTH] = TRANS_COLOR; // (was color198)
				gpBuffer[44 + 153 * BUFFER_WIDTH] = 202; // (was color198)
				gpBuffer[40 + 154 * BUFFER_WIDTH] = TRANS_COLOR; // (was color197)
				gpBuffer[41 + 154 * BUFFER_WIDTH] = TRANS_COLOR; // (was color196)
				gpBuffer[42 + 154 * BUFFER_WIDTH] = TRANS_COLOR; // (was color196)
				gpBuffer[43 + 154 * BUFFER_WIDTH] = TRANS_COLOR; // (was color196)
				gpBuffer[44 + 154 * BUFFER_WIDTH] = TRANS_COLOR; // (was color202)
				gpBuffer[40 + 155 * BUFFER_WIDTH] = TRANS_COLOR; // (was color199)
				gpBuffer[41 + 155 * BUFFER_WIDTH] = TRANS_COLOR; // (was color196)
				gpBuffer[42 + 155 * BUFFER_WIDTH] = TRANS_COLOR; // (was color196)
				gpBuffer[43 + 155 * BUFFER_WIDTH] = TRANS_COLOR; // (was color197)
				gpBuffer[44 + 155 * BUFFER_WIDTH] = TRANS_COLOR; // (was color204)
				gpBuffer[40 + 156 * BUFFER_WIDTH] = TRANS_COLOR; // (was color204)
				gpBuffer[41 + 156 * BUFFER_WIDTH] = TRANS_COLOR; // (was color201)
				gpBuffer[42 + 156 * BUFFER_WIDTH] = TRANS_COLOR; // (was color198)
				gpBuffer[43 + 156 * BUFFER_WIDTH] = TRANS_COLOR; // (was color204
			}
		}

		// write to the new CEL file
		dstHeaderCursor[0] = SwapLE32((DWORD)((size_t)dstDataCursor - (size_t)resCelBuf));
		dstHeaderCursor++;

		dstDataCursor = EncodeFrame(dstDataCursor, FRAME_WIDTH, FRAME_HEIGHT, SUB_HEADER_SIZE, TRANS_COLOR);

		// skip the original frame
		srcHeaderCursor++;
	}

	// add file-size
	*celLen = (size_t)dstDataCursor - (size_t)resCelBuf;
	dstHeaderCursor[0] = SwapLE32((DWORD)(*celLen));

	return resCelBuf;
}
#endif // ASSET_MPL == 1
static BYTE* patchFile(int index, size_t *dwLen)
{
	BYTE* buf = LoadFileInMem(filesToPatch[index], dwLen);
	if (buf == NULL) {
		app_warn("Unable to open file %s in the mpq.", filesToPatch[index]);
		return NULL;
	}

	switch (index) {
	case FILE_MOVIE_VIC1:
	case FILE_MOVIE_VIC2:
	case FILE_MOVIE_VIC3:
	{	// patch .SMK
		patchMovie(index, buf, dwLen);
	} break;
#if ASSET_MPL == 1
#if 0
	case FILE_TOWN_SCEL:
	{	// patch pSpecialsCel - TownS.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_TOWN_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_TOWN_MIN]);
			return NULL;
		}
		if (minLen < 1258 * BLOCK_SIZE_TOWN * 2) {
			mem_free_dbg(minBuf);
			// mem_free_dbg(buf);
			// app_warn("Invalid file %s in the mpq.", filesToPatch[FILE_TOWN_MIN]);
			// return NULL;
			return buf; // -- assume it is already done
		}
		size_t celLen;
		BYTE* celBuf = LoadFileInMem(filesToPatch[FILE_TOWN_CEL], &celLen);
		if (celBuf == NULL) {
			mem_free_dbg(minBuf);
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_TOWN_CEL]);
			return NULL;
		}
		buf = Town_PatchSpec(minBuf, minLen, celBuf, celLen, buf, dwLen);
		mem_free_dbg(celBuf);
		mem_free_dbg(minBuf);
	} break;
#endif
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
			mem_free_dbg(minBuf);
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
	case FILE_L1DOORS_CEL:
	{	// patch L1Doors.CEL
		buf = DRLP_L1_PatchDoors(buf, dwLen);
	} break;
	case FILE_CATHEDRAL_SCEL:
	{	// patch pSpecialsCel - L1S.CEL
		buf = DRLP_L1_PatchSpec(buf, dwLen);
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
			mem_free_dbg(minBuf);
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
			mem_free_dbg(minBuf);
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
			mem_free_dbg(minBuf);
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
			mem_free_dbg(minBuf);
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
#if ASSET_MPL == 1
	case FILE_OBJ_MCIRL_CEL:
	{	// fix object gfx file - Mcirls.CEL
		buf = fixObjCircle(buf, dwLen);
	} break;
	case FILE_OBJ_CNDL2_CEL:
	{	// fix object gfx file - Candle2.CEL
		buf = fixObjCandle(buf, dwLen);
	} break;
	case FILE_OBJ_LSHR_CEL:
	{	// fix object gfx file - LShrineG.CEL
		buf = fixObjLShrine(buf, dwLen);
	} break;
	case FILE_OBJ_RSHR_CEL:
	{	// fix object gfx file - RShrineG.CEL
		buf = fixObjRShrine(buf, dwLen);
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
#endif
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
			mem_free_dbg(minBuf);
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
			mem_free_dbg(minBuf);
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
#if ASSET_MPL == 1
	case FILE_CRYPT_SCEL:
	{	// patch pSpecialsCel - L5S.CEL
		size_t minLen;
		BYTE* minBuf = LoadFileInMem(filesToPatch[FILE_CRYPT_MIN], &minLen);
		if (minBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CRYPT_MIN]);
			return NULL;
		}
		size_t celLen;
		BYTE* celBuf = LoadFileInMem(filesToPatch[FILE_CRYPT_CEL], &celLen);
		if (celBuf == NULL) {
			mem_free_dbg(minBuf);
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", filesToPatch[FILE_CRYPT_CEL]);
			return NULL;
		}
		buf = DRLP_L5_PatchSpec(minBuf, minLen, celBuf, celLen, buf, dwLen);
		mem_free_dbg(celBuf);
		mem_free_dbg(minBuf);
	} break;
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
#if ASSET_MPL == 1
	case FILE_L5LIGHT_CEL:
	{	// fix object gfx file - L5Light.CEL
		buf = fixL5Light(buf, dwLen);
	} break;
	case FILE_MON_FALLGW:
	{
		size_t stdLen;
		const char* stdFileName = "Monsters\\BigFall\\Fallgn.CL2";
		BYTE* stdBuf = LoadFileInMem(stdFileName, &stdLen);
		if (stdBuf == NULL) {
			mem_free_dbg(buf);
			app_warn("Unable to open file %s in the mpq.", stdFileName);
			return NULL;
		}
		buf = createFallgwAnim(buf, dwLen, stdBuf);
		mem_free_dbg(stdBuf);
	} break;
	case FILE_MON_GOATLD:
	{	// fix monster gfx file - GoatLd.CL2
		buf = fixGoatLdAnim(buf, dwLen);
	} break;
#endif // ASSET_MPL
#endif // HELLFIRE
#if ASSET_MPL == 1
	case FILE_ITEM_ARMOR2:
	case FILE_ITEM_GOLDFLIP:
	case FILE_ITEM_MACE:
	case FILE_ITEM_STAFF:
	case FILE_ITEM_RING:
	case FILE_ITEM_CROWNF:
	case FILE_ITEM_LARMOR:
	case FILE_ITEM_WSHIELD:
	case FILE_ITEM_SCROLL:
	case FILE_ITEM_FEAR:
	case FILE_ITEM_FBRAIN:
	case FILE_ITEM_FMUSH:
	case FILE_ITEM_INNSIGN:
	case FILE_ITEM_BLDSTN:
	case FILE_ITEM_FANVIL:
	case FILE_ITEM_FLAZSTAF:
#ifdef HELLFIRE
	case FILE_ITEM_TEDDYS1:
	case FILE_ITEM_COWS1:
	case FILE_ITEM_DONKYS1:
	case FILE_ITEM_MOOSES1:
#endif
	{	// fix item gfx files to improve the drop-animations
		buf = patchFloorItems(index, buf, dwLen);
	} break;
#endif // ASSET_MPL
	case FILE_OBJCURS_CEL:
	{
#ifdef HELLFIRE
		size_t sizeB, sizeAB;
		BYTE *aCursCels, *bCursCels;
		DWORD numAB;
#endif
		DWORD numA;

		numA = SwapLE32(((DWORD*)buf)[0]);
		if (numA != 179) {
			if (numA != NUM_ICURS + CURSOR_FIRSTITEM - 1 /* 179 + 61 - 2*/) {
				mem_free_dbg(buf);
				app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
				buf = NULL;
			}
			return buf;
		}
#ifdef HELLFIRE
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
#endif // HELLFIRE
		// move the graphics to the center + minor adjustments
		buf = centerCursors(buf, dwLen);
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	return buf;
}

static int patcher_callback()
{
restart:
	switch (workPhase) {
	case 0:
	{	// first round - read the content and prepare the metadata
		std::string listpath = std::string(GetBasePath()) + "mpqfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "mpqfiles.txt");
			return RETURN_ERROR;
		}
		// mpqfiles.clear();
		std::string line;
		while (std::getline(input, line)) {
			for (int i = 0; i < NUM_MPQS; i++) {
				if (SFileReadArchive(diabdat_mpqs[i], line.c_str(), NULL) != 0) {
					mpqfiles.push_back(line);
					break;
				}
			}
		}

		int entryCount = mpqfiles.size() + lengthof(filesToPatch);
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
		// - open a new work-file
		archive = SFileCreateArchive(path.c_str(), hashCount, hashCount);
		if (archive == NULL) {
			app_warn("Unable to create MPQ file %s.", path.c_str());
			return RETURN_ERROR;
		}
		hashCount = 0;
		workPhase++;
	} break;
	case 2:
	{	// add the next file from devilx.mpq
		const char* fileName = mpqfiles[hashCount].c_str();
		for (int i = 0; i < NUM_MPQS; i++) {
			BYTE* buf = NULL;
			DWORD dwLen = SFileReadArchive(diabdat_mpqs[i], fileName, &buf);
			if (dwLen != 0) {
				bool success = SFileWriteFile(archive, fileName, buf, dwLen);
				mem_free_dbg(buf);
				if (!success) {
					app_warn("Unable to write %s to the MPQ.", fileName);
					return RETURN_ERROR;
				}
				break;
			}
		}
		hashCount++;
		if (hashCount < mpqfiles.size())
			break;
		hashCount = 0;
		workPhase++;
	} break;
	case 3:
	{	// add patches
		int i = hashCount;
		{
			size_t dwLen;
			BYTE* buf = patchFile(i, &dwLen);
			if (buf == NULL) {
				return RETURN_ERROR;
			}
			if (dwLen > UINT32_MAX) {
				app_warn("Patched file %s is too large to be included in an MPQ archive.", filesToPatch[i]);
				return RETURN_ERROR;
			}
			if (!SFileWriteFile(archive, filesToPatch[i], buf, (DWORD)dwLen)) {
				app_warn("Unable to write %s to the MPQ.", filesToPatch[i]);
				return RETURN_ERROR;
			}
			mem_free_dbg(buf);
		}
		hashCount++;
		if (hashCount < lengthof(filesToPatch))
			break;
		SFileFlushAndCloseArchive(archive);
		archive = NULL;
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

	Uint32 now = SDL_GetTicks();
	if (!SDL_TICKS_PASSED(now, sgRenderTc + gnRefreshDelay))
		goto restart;
	sgRenderTc = now;

	while (++workProgress >= 100)
		workProgress -= 100;
	return workProgress;
}

void UiPatcherDialog()
{
	workProgress = 0;
	workPhase = 0;
	sgRenderTc = SDL_GetTicks();

	// ignore the merged mpq during the patch
	HANDLE mpqone = diabdat_mpqs[NUM_MPQS];
	diabdat_mpqs[NUM_MPQS] = NULL;
	// use the whole buffer for drawing
	BYTE* bufstart = gpBufStart;
	BYTE* bufend = gpBufEnd;
	gpBufStart = &gpBuffer[0];
	gpBufEnd = &gpBuffer[BUFFER_WIDTH * BUFFER_HEIGHT];

	bool result = UiProgressDialog("...Patch in progress...", patcher_callback);
	// cleanup
	mpqfiles.clear();
	// restore the merged mpq
	diabdat_mpqs[NUM_MPQS] = mpqone;
	// restore buffer start/end
	gpBufStart = bufstart;
	gpBufEnd = bufend;

	if (!result) {
		// if (workPhase == 2 || workPhase == 3) {
			SFileCloseArchive(archive);
			archive = NULL;
		// }
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
