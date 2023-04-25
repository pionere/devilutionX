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

static const char* const filesToPatch[] = {
	"Levels\\TownData\\Town.MIN",
#ifdef HELLFIRE
	"NLevels\\TownData\\Town.MIN",
	"NLevels\\L5Data\\L5.TIL",
	"NLevels\\L5Data\\L5.MIN",
#endif
};

#define DESCRIPTION_WIDTH (SELGAME_LPANEL_WIDTH - 2 * 10)

#define MICRO_IDX(subtile, blockSize, microIndex) ((subtile) * (blockSize) + (blockSize) - (2 + ((microIndex) & ~1)) + ((microIndex) & 1))
/*#define blkMicro(subtileRef, blockSize, microIndex, value) \
{ \
	assert((pMicroPieces[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] & 0xFFF) == SwapLE16(value) || pMicroPieces[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] == SwapLE16(0)); \
	pMicroPieces[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}*/
#define blkMicro(subtileRef, microIndex) \
{ \
	pMicroPieces[MICRO_IDX(subtileRef - 1, blockSize, microIndex)] = 0; \
}

static void patchTownFile(BYTE* buf)
{
	// pointless tree micros (re-drawn by dSpecial)
	uint16_t *pMicroPieces = (uint16_t*)buf;
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
	case 0:
	{	// patch dMiniTiles - Town.MIN
		if (*dwLen < MICRO_IDX(1219 - 1, 16, 0) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq. File len: %d vs %d", filesToPatch[index], *dwLen, MICRO_IDX(1219 - 1, 16, 0) * 2);
			return NULL;
		}
		patchTownFile(buf);
	} break;
#ifdef HELLFIRE
	case 1:
	{	// patch dMiniTiles - Town.MIN
		if (*dwLen < MICRO_IDX(1303 - 1, 16, 7) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq. File len: %d vs %d", filesToPatch[index], *dwLen, MICRO_IDX(1303 - 1, 16, 7) * 2);
			return NULL;
		}
		patchTownFile(buf);
		uint16_t *pMicroPieces = (uint16_t*)buf;
		constexpr int blockSize = 16;
		// fix bad artifacts
		blkMicro(1273, 7);
		blkMicro(1303, 7);
	} break;
	case 2:
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
	case 3:
	{   // patch dMiniTiles - L5.MIN
		if (*dwLen < MICRO_IDX(77 - 1, 10, 8) * 2) {
			mem_free_dbg(buf);
			app_warn("Invalid file %s in the mpq.", filesToPatch[index]);
			return NULL;
		}
		// pointless door micros (re-drawn by dSpecial)
		uint16_t *pMicroPieces = (uint16_t*)buf;
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
	} break;
#endif
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
			// app_warn("Can not find/access '%s' in the game folder.", "listfiles.txt");
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
