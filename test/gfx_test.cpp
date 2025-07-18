/**
 * @file gfx_test.cpp
 *
 * Test whether the headers (CEL_FRAME_HEADER) of the clipped CEL/CL2 files are correct.
 */
#include <gtest/gtest.h>
#include "all.h"

using namespace dvl;

static const char animletter[NUM_MON_ANIM] = { 'n', 'w', 'a', 'h', 'd', 's' };
/** Maps from armor animation to letter used in graphic files. */
const char ArmorChar[4] = {
	'L', // light
	'M', // medium
	'H', // heavy
	0
};
/** Maps from weapon animation to letter used in graphic files. */
const char WepChar[10] = {
	'N', // unarmed
	'U', // no weapon + shield
	'S', // sword + no shield
	'D', // sword + shield
	'B', // bow
	'A', // axe
	'M', // blunt + no shield
	'H', // blunt + shield
	'T', // staff
	0
};
/** Maps from player class to letter used in graphic files. */
const char CharChar[NUM_CLASSES] = { 'W', 'R', 'S',
#ifdef HELLFIRE
//	'M', 'B', 'C'
	'M', 'R', 'W'
#endif
};
/** Maps from player class to the folder of graphic files. */
const char* const gfxClassTbl[NUM_CLASSES] = { "Warrior", "Rogue", "Sorceror",
#ifdef HELLFIRE
//	"Monk", "Bard", "Barbarian"
	"Monk", "Rogue", "Warrior"
#endif
};
/* Data related to the player-animation types. */
static const PlrAnimType PlrAnimTypes[NUM_PGTS] = {
	// clang-format off
	{ "ST", PGX_STAND },     // PGT_STAND_TOWN
	{ "AS", PGX_STAND },     // PGT_STAND_DUNGEON
	{ "WL", PGX_WALK },      // PGT_WALK_TOWN
	{ "AW", PGX_WALK },      // PGT_WALK_DUNGEON
	{ "AT", PGX_ATTACK },    // PGT_ATTACK
	{ "FM", PGX_FIRE },      // PGT_FIRE
	{ "LM", PGX_LIGHTNING }, // PGT_LIGHTNING
	{ "QM", PGX_MAGIC },     // PGT_MAGIC
	{ "BL", PGX_BLOCK },     // PGT_BLOCK
	{ "HT", PGX_GOTHIT },    // PGT_GOTHIT
	{ "DT", PGX_DEATH },     // PGT_DEATH
	// clang-format on
};

static inline void GetPlrGFXCells(int pc, const char** szCel, const char** cs)
{
	*szCel = &CharChar[pc];
	*cs = gfxClassTbl[pc];
}

static void Cl2Blit(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	int i;
	int8_t width;
	BYTE fill, *dst;
	const BYTE *src, *end;

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width < 0) {
				width = -width;
				if (width > 65) {
					width -= 65;
					fill = *src++;
					//assert(dst < &gpBuffer[BUFFER_WIDTH * BUFFER_HEIGHT] && dst >= gpBuffer);
					//if (dst < gpBufEnd && dst >= gpBufStart) {
						assert(i >= width);
						i -= width;
						memset(dst, fill, width);
						// DUFFS_LOOP4 ?
						dst += width;
						continue;
					//}
				} else {
					//assert(dst < &gpBuffer[BUFFER_WIDTH * BUFFER_HEIGHT] && dst >= gpBuffer);
					//if (dst < gpBufEnd && dst >= gpBufStart) {
						assert(i >= width);
						i -= width;
						memcpy(dst, src, width);
						// DUFFS_LOOP4 ?
						dst += width;
						src += width;
						continue;
					//} else {
					//	src += width;
					//}
				}
			}
			while (true) {
				if (width <= i) {
					dst += width;
					i -= width;
					break;
				} else {
					dst += i;
					width -= i;
					i = nWidth;
					dst -= BUFFER_WIDTH + nWidth;
				}
			}
		}
	}
}
//int currClass, currAnim, currGfx, currGfxIdx;
static bool checkOneCl2(BYTE* cl2Data, int nCel, int nWidth, const char* filename, int group)
{
	for (int iCel = 1; iCel <= nCel; iCel++) {
		//LogErrorF("PLR", "Checking %s (idx %d of %d, w:%d, group %d) anim:%d gfx:%d gxi:%d class%d", filename, iCel, nCel, nWidth, group, currAnim, currGfx, currGfxIdx, currClass);
		memset(gpBuffer, 0, BUFFER_WIDTH * BUFFER_HEIGHT);

		for (int cb = 0; ; cb++) {
			int sx = SCREEN_X;
			int sy = SCREEN_Y + 480 - cb * CEL_BLOCK_HEIGHT;
			int nDataSize;
			const BYTE* pRLEBytes = CelGetFrameClippedAt(cl2Data, iCel, cb, &nDataSize);
			if (nDataSize <= 0) {
				break;
			}
			BYTE* pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];
			Cl2Blit(pDecodeTo, pRLEBytes, nDataSize, nWidth);

			for (int yy = sy - CEL_BLOCK_HEIGHT; yy >= SCREEN_Y; yy--) {
				for (int xx = SCREEN_X; xx < SCREEN_X + 320; xx++) {
					if (gpBuffer[BUFFERXY(xx, yy)] != 0) {
						app_fatal("Failed to block-draw %s (%d of %d in group %d) with width %d. Current block %d", filename, iCel, nCel, group, nWidth, cb);
						return false;
					}
				}
			}
		}

		int sx = SCREEN_X + 320;
		int sy = SCREEN_Y + 480;
		int nDataSize;
		const BYTE* pRLEBytes = CelGetFrameClipped(cl2Data, iCel, &nDataSize, &sy);
		BYTE* pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];
		Cl2Blit(pDecodeTo, pRLEBytes, nDataSize, nWidth);

		for (int yy = sy; yy >= SCREEN_Y; yy--) {
			for (int xx = SCREEN_X; xx < SCREEN_X + 320; xx++) {
				if (gpBuffer[BUFFERXY(xx, yy)] != gpBuffer[BUFFERXY(xx + 320, yy)]) {
					app_fatal("Mismatching draw %s (%d of %d in group %d) with width %d.", filename, iCel, nCel, group, nWidth);
					return false;
				}
			}
		}
		//LogErrorF("PLR", "Check done.");
	}

	return true;
}

static bool checkCl2(const char* filename, int nCel, int nWidth, int numGroups)
{
	bool result;
	BYTE* cl2Data = LoadFileInMem(filename);

	if (numGroups != 0) {
		result = true;
		for (int i = 0; i < numGroups; i++) {
			BYTE* oneCl2Data = const_cast<BYTE*>(CelGetFrameStart(cl2Data, i));
			result &= checkOneCl2(oneCl2Data, nCel, nWidth, filename, i);
		}
	} else {
		result = checkOneCl2(cl2Data, nCel, nWidth, filename, 0);
	}
	
	mem_free_dbg(cl2Data);
	return result;
}

static void CelBlit(BYTE* pDecodeTo, const BYTE* pRLEBytes, int nDataSize, int nWidth)
{
	int i;
	int8_t width;
	const BYTE *src, *end;
	BYTE* dst;

	assert(pDecodeTo != NULL);
	assert(pRLEBytes != NULL);
	assert(gpBuffer != NULL);

	src = pRLEBytes;
	end = &pRLEBytes[nDataSize];
	dst = pDecodeTo;

	for ( ; src != end; dst -= BUFFER_WIDTH + nWidth) {
		for (i = nWidth; i != 0; ) {
			width = *src++;
			if (width >= 0) {
				i -= width;
				//assert(dst < gpBufEnd && dst >= gpBufStart);
				//if (dst < gpBufEnd && dst >= gpBufStart) {
				memcpy(dst, src, width);
				//}
				src += width;
				dst += width;
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

static bool checkOneCel(BYTE* celData, int nCel, int nWidth, const char* filename, int group)
{
	for (int iCel = 1; iCel <= nCel; iCel++) {
		//LogErrorF("PLR", "Checking %s (idx %d of %d, w:%d in group %d)", filename, iCel, nCel, nWidth, group);
		memset(gpBuffer, 0, BUFFER_WIDTH * BUFFER_HEIGHT);

		for (int cb = 0; ; cb++) {
			int sx = SCREEN_X + 0;
			int sy = SCREEN_Y + 480 - cb * CEL_BLOCK_HEIGHT;
			int nDataSize;
			const BYTE* pRLEBytes = CelGetFrameClippedAt(celData, iCel, cb, &nDataSize);
			if (nDataSize <= 0) {
				break;
			}
			BYTE* pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];
			CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);

			for (int yy = sy - CEL_BLOCK_HEIGHT; yy >= SCREEN_Y; yy--) {
				for (int xx = SCREEN_X; xx < SCREEN_X + 320; xx++) {
					if (gpBuffer[BUFFERXY(xx, yy)] != 0) {
						app_fatal("Failed to block-draw %s (%d of %d in group %d) with width %d. Current block %d", filename, iCel, nCel, group, nWidth, cb);
						return false;
					}
				}
			}
		}

		int sx = SCREEN_X + 320;
		int sy = SCREEN_Y + 480;
		int nDataSize;
		const BYTE* pRLEBytes = CelGetFrameClipped(celData, iCel, &nDataSize, &sy);
		BYTE* pDecodeTo = &gpBuffer[BUFFERXY(sx, sy)];
		CelBlit(pDecodeTo, pRLEBytes, nDataSize, nWidth);

		for (int yy = sy; yy >= SCREEN_Y; yy--) {
			for (int xx = SCREEN_X; xx < SCREEN_X + 320; xx++) {
				if (gpBuffer[BUFFERXY(xx, yy)] != gpBuffer[BUFFERXY(xx + 320, yy)]) {
					app_fatal("Mismatching draw %s (%d of %d in group %d) with width %d.", filename, iCel, nCel, group, nWidth);
					return false;
				}
			}
		}
		//LogErrorF("PLR", "Check done.");
	}

	return true;
}

static bool checkCel(const char* filename, int nCel, int nWidth, int numGroups)
{
	BYTE* celData = LoadFileInMem(filename);
	bool result;

	if (numGroups != 0) {
		result = true;
		for (int i = 0; i < numGroups; i++) {
			BYTE* oneCl2Data = const_cast<BYTE*>(CelGetFrameStart(celData, i));
			result &= checkOneCel(oneCl2Data, nCel, nWidth, filename, i);
		}
	} else {
		result = checkOneCel(celData, nCel, nWidth, filename, 0);
	}

	mem_free_dbg(celData);
	return result;
}

TEST(Gfx, Missiles)
{
	for (int i = 0; i < lengthof(misfiledata); i++) {
		char pszName[DATA_ARCHIVE_MAX_PATH];
		auto& mfd = misfiledata[i];
		if (mfd.mfAnimFAmt == 1) {
			snprintf(pszName, sizeof(pszName), "Missiles\\%s.CL2", mfd.mfName);
			bool result = checkCl2(pszName, mfd.mfAnimLen[0], mfd.mfAnimWidth, 0);
			EXPECT_TRUE(result);
		} else {
			for (int j = 0; j < mfd.mfAnimFAmt; j++) {
				snprintf(pszName, sizeof(pszName), "Missiles\\%s%d.CL2", mfd.mfName, j + 1);
				bool result = checkCl2(pszName, mfd.mfAnimLen[j], mfd.mfAnimWidth, 0);
				EXPECT_TRUE(result);
			}
		}
	}
}

TEST(Gfx, Monsters)
{
	for (int i = 0; i < lengthof(monfiledata); i++) {
		char strBuff[DATA_ARCHIVE_MAX_PATH];
		auto& mfdata = monfiledata[i];
		for (int anim = 0; anim < NUM_MON_ANIM; anim++) {
			if (mfdata.moAnimFrames[anim] > 0) {
				snprintf(strBuff, sizeof(strBuff), mfdata.moGfxFile, animletter[anim]);
				if (i != MOFILE_GOLEM || (anim != MA_SPECIAL && anim != MA_DEATH)) {
					checkCl2(strBuff, mfdata.moAnimFrames[anim], mfdata.moWidth, NUM_DIRS);
				} else {
					checkCl2(strBuff, mfdata.moAnimFrames[anim], mfdata.moWidth, 0);
				}
			}
		}
	}
}

TEST(Gfx, Players)
{
	for (int i = 0; i < NUM_CLASSES; i++) {
#ifdef HELLFIRE
		if (i == PC_BARBARIAN || i == PC_BARD)
			continue;
#endif
		char prefix[4];
		char pszName[DATA_ARCHIVE_MAX_PATH];
		const char *szCel, *chrArmor, *chrWeapon, *chrClass, *strClass;

		GetPlrGFXCells(i, &chrClass, &strClass);

		for (int anim = 0; anim < lengthof(PlrAnimTypes); anim++) {
			auto& pAnimType = PlrAnimTypes[anim];

			szCel = pAnimType.patTxt;
			for (chrArmor = &ArmorChar[0]; *chrArmor != '\0'; chrArmor++) {
				for (chrWeapon = &WepChar[0]; *chrWeapon != '\0'; chrWeapon++) { // BUGFIX loads non-existing animations; DT is only for N, BL is only for U, D & H (fixed)
					if (szCel[0] == 'D' && *chrArmor != 'L' && *chrWeapon != 'N') {
						// assert(szCel[1] == 'T');
						continue; //Death has no weapon or armor
					}
					prefix[0] = *chrClass;
					prefix[1] = *chrArmor;
					prefix[2] = *chrWeapon;
					prefix[3] = '\0';
					snprintf(pszName, sizeof(pszName), "PlrGFX\\%s\\%s\\%s%s.CL2", strClass, prefix, prefix, szCel);
					BYTE* fileData = LoadFileInMem(pszName);
					if (fileData == NULL)
						continue;
					mem_free_dbg(fileData);

					int gfx = 0;
					//switch (*chrArmor) {
					//case 'L': gfx |= ANIM_ID_LIGHT_ARMOR; break;
					//case 'M': gfx |= ANIM_ID_MEDIUM_ARMOR;break;
					//case 'H': gfx |= ANIM_ID_HEAVY_ARMOR; break;
					//default: ASSUME_UNREACHABLE; break;
					//}
					switch (*chrWeapon) {
					case 'N': gfx |= ANIM_ID_UNARMED; break; // unarmed
					case 'U': gfx |= ANIM_ID_UNARMED_SHIELD; break; // no weapon + shield
					case 'S': gfx |= ANIM_ID_SWORD; break; // sword + no shield
					case 'D': gfx |= ANIM_ID_SWORD_SHIELD; break; // sword + shield
					case 'B': gfx |= ANIM_ID_BOW; break; // bow
					case 'A': gfx |= ANIM_ID_AXE; break; // axe
					case 'M': gfx |= ANIM_ID_MACE; break; // blunt + no shield
					case 'H': gfx |= ANIM_ID_MACE_SHIELD; break; // blunt + shield
					case 'T': gfx |= ANIM_ID_STAFF; break; // staff
					default: ASSUME_UNREACHABLE; break;
					}
					//if (i == PC_WARRIOR && anim == ANIM_ID_BOW && pAnimType.patGfxIdx == PGX_ATTACK)
					//	continue;
					//currClass = i;
					//currAnim = anim;
					//currGfx = gfx;
					//currGfxIdx = pAnimType.patGfxIdx;
					if (anim == PGT_STAND_TOWN || anim == PGT_WALK_TOWN)
						currLvl._dType = DTYPE_TOWN;
					else
						currLvl._dType = DTYPE_CATACOMBS;
					plx(0)._pClass = i;
					plx(0)._pgfxnum = gfx;
					SetPlrAnims(0);
					checkCl2(pszName, plx(0)._pAnims[pAnimType.patGfxIdx].paFrames, plx(0)._pAnims[pAnimType.patGfxIdx].paAnimWidth, NUM_DIRS);
					currLvl._dType = 0;
				}
			}
		}
	}
}

TEST(Gfx, Objects)
{
	for (int i = 0; i < lengthof(objfiledata); i++) {
		char strBuff[DATA_ARCHIVE_MAX_PATH];
		auto& ofdata = objfiledata[i];
		snprintf(strBuff, sizeof(strBuff), "Objects\\%s.CEL", ofdata.ofName);
		checkCel(strBuff, ofdata.oAnimLen, ofdata.oAnimWidth, 0);
	}
}

TEST(Gfx, Items)
{
	for (int i = 0; i < lengthof(itemfiledata); i++) {
		char strBuff[DATA_ARCHIVE_MAX_PATH];
		auto& ifdata = itemfiledata[i];
		snprintf(strBuff, sizeof(strBuff), "Items\\%s.CEL", ifdata.ifName);
		checkCel(strBuff, ifdata.iAnimLen, ITEM_ANIM_WIDTH, 0);
	}
}

TEST(Gfx, Towners)
{
	typedef struct TownerData {
		const char* tfName;
		int tAnimLen;
	} TownerData;
	const TownerData townerfiledata[] = {
		{ "Towners\\Smith\\SmithN.CEL", 16 },
		{ "Towners\\TwnF\\TwnFN.CEL", 16 },
		{ "Towners\\Butch\\Deadguy.CEL", 8 },
		{ "Towners\\TownWmn1\\Witch.CEL", 19 },
		{ "Towners\\TownWmn1\\WmnN.CEL", 18 },
		{ "Towners\\TownBoy\\PegKid1.CEL", 20 },
		{ "Towners\\Healer\\Healer.CEL", 20 },
		{ "Towners\\Strytell\\Strytell.CEL", 25 },
		{ "Towners\\Drunk\\TwnDrunk.CEL", 18 },
		{ "Towners\\Priest\\Priest8.CEL", 33 },
	};

	for (int i = 0; i < lengthof(townerfiledata); i++) {
		auto& tfdata = townerfiledata[i];
		checkCel(tfdata.tfName, tfdata.tAnimLen, 96 * ASSET_MPL, 0);
	}

	checkCel("Towners\\Animals\\Cow.CEL", 12, 128 * ASSET_MPL, NUM_DIRS);
}
