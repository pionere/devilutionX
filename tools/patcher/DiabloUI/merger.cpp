#include <string>
#include <fstream>
#if ASSET_MPL != 1
#include <set>
#endif
#include <vector>

#include "diablo.h"
#include "diabloui.h"
#include "selok.h"
#include "selyesno.h"
#include "utils/display.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

typedef enum _merger_selections {
	MERGER_RUN,
#if ASSET_MPL != 1
	MERGER_HDONLY,
#endif
	MERGER_NOSOUND,
	MERGER_CANCEL,
	NUM_MERGER,
} _merger_selections;

static unsigned workProgress;
static unsigned workPhase;
static Uint32 sgMergerRenderTc;
static std::vector<std::string> listfiles;
#if ASSET_MPL != 1
static std::set<std::string> hdfiles;
static bool hdOnly = false;
#endif
#ifdef NOSOUND
static bool noSound = true;
#else
static bool noSound = false;
#endif
static HANDLE archive;
static int hashCount;
static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_CANCEL = 102;
static constexpr int RETURN_DONE = 100;

static const char* const filesToSkip[] = {
	"Data\\Inv\\Objcurs2.CEL",
#ifdef HELLFIRE
	"gendata\\diablo1.smk",
	"Levels\\TownData\\Town.DUN",
	"Levels\\TownData\\Town.RDUN",
	"Levels\\TownData\\Town.SLA",
	"Levels\\TownData\\Town.CEL",
	"Levels\\TownData\\Town.TIL",
	"Levels\\TownData\\Town.MIN",
	"Meta\\credits.txt",
#else
	"gendata\\Hellfire.smk",
	"Meta\\credits_hf.txt",
	"NLevels\\L5Data\\L5.SLA",
	"NLevels\\L5Data\\L5.TLA",
	"NLevels\\L5Data\\Nakrul1.DUN",
	"NLevels\\L5Data\\Nakrul2.DUN",
	"NLevels\\L6Data\\L6.SLA",
	"NLevels\\L6Data\\L6.TLA",
	"NLevels\\L6Data\\L6.TRS",
	"NLevels\\TownData\\Town.DUN",
	"NLevels\\TownData\\Town.RDUN",
	"NLevels\\TownData\\Town.SLA",
	"NLevels\\TownData\\Town.CEL",
	"NLevels\\TownData\\Town.TIL",
	"NLevels\\TownData\\Town.TRS",
	"NLevels\\TownData\\Town.MIN",
#endif
};

// Forward-declare UI-handlers, used by other handlers.
static void MergerSelect(unsigned index);

static void MergerFreeDlgItems()
{
	UiClearListItems();

	UiClearItems();
}

static void MergerEsc()
{
	workProgress = RETURN_CANCEL;
}

static void MergerInit()
{
	MergerFreeDlgItems();

	// UiAddBackground();
	UiAddLogo();

	SDL_Rect rect1 = { PANEL_LEFT, SELHERO_TITLE_TOP, PANEL_WIDTH, 35 };
	gUiItems.push_back(new UiText("Merge MPQ files", rect1, UIS_HCENTER | UIS_BIG | UIS_SILVER));

	gUIListItems.push_back(new UiListItem("Start merge", MERGER_RUN));
#if ASSET_MPL != 1
	gUIListItems.push_back(new UiListItem(!hdOnly ? "Only HD Assets: No" : "Only HD Assets: Yes", MERGER_HDONLY));
#endif
	gUIListItems.push_back(new UiListItem(noSound ? "With Sound Assets: No" : "With Sound Assets: Yes", MERGER_NOSOUND));
	gUIListItems.push_back(new UiListItem("Cancel", MERGER_CANCEL));

	SDL_Rect rect5 = { SCREEN_MIDX(MAINMENU_WIDTH), SELGAME_LIST_TOP, MAINMENU_WIDTH, 26 * NUM_MERGER };
	gUiItems.push_back(new UiList(&gUIListItems, NUM_MERGER, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	//assert(gUIListItems.size() == numOptions);
	UiInitScreen(NUM_MERGER, NULL, MergerSelect, MergerEsc);

	UiFocus(workPhase);
}

static void MergerSelect(unsigned index)
{
	workPhase = index;

	switch (index) {
	case MERGER_RUN:
		workProgress = 0;
		break;
#if ASSET_MPL != 1
	case MERGER_HDONLY:
		hdOnly = !hdOnly;
		MergerInit();
		break;
#endif
	case MERGER_NOSOUND:
		noSound = !noSound;
		MergerInit();
		break;
	case MERGER_CANCEL:
		workProgress = RETURN_CANCEL;
		break;
	}
}

static bool merger_skipFile(const std::string &path)
{
	// skip files which are 'commented out'
	if (path[0] == '_') return true;
	// skip sound files if requested
	if (noSound && path.size() >= 4 && SDL_strcasecmp(path.c_str() + path.size() - 4, ".wav") == 0)
		return true;
	// skip hellfire/vanilla files
	for (int n = 0; n < lengthof(filesToSkip); n++) {
		if (SDL_strcmp(path.c_str(), filesToSkip[n]) == 0) {
			return true;
		}
	}
	return false;
}
#if ASSET_MPL != 1
static std::string assetPath(const std::string &basePath, std::string &entry)
{
#ifndef _WIN32
	int i;
	for (i = 0; i < entry.size(); ++i)
		if (entry[i] == '\\')
			entry[i] = '/';
#endif
	return basePath + entry;
}
#endif

static std::string mpqPath(int *mpqIdx)
{
	std::string path = std::string(GetBasePath());
	*mpqIdx = NUM_MPQS;
#if ASSET_MPL != 1
	if (hdOnly) {
		*mpqIdx = MPQ_DEVILHD;

		char tmpstr[32];
		snprintf(tmpstr, lengthof(tmpstr), "devilx_hd%d.mpq", ASSET_MPL);
		path += tmpstr;
	} else
#endif
		path += MPQONE;
	return path;
}

static int merger_callback()
{
restart:
	switch (workPhase) {
	case 0:
	{	// first round - read the content and prepare the metadata
		std::string basePath = std::string(GetBasePath());
#if ASSET_MPL != 1
		if (diabdat_mpqs[MPQ_DEVILHD] == NULL) {
			std::string hdPath = basePath + "hdfiles.txt";
			std::ifstream input(hdPath);
			if (input.fail()) {
				app_warn("Can not find/access '%s'.", hdPath.c_str());
				return RETURN_ERROR;
			}
			std::string line;
			// hdfiles.clear();
			while (std::getline(input, line)) {
				if (merger_skipFile(line)) continue;
				hdfiles.insert(line);
			}
		}
#endif
		std::string listPath = basePath + "listfiles.txt";
		std::ifstream input(listPath);
		if (input.fail()) {
			app_warn("Can not find/access '%s'.", listPath.c_str());
			return RETURN_ERROR;
		}
		std::string line;
		// listfiles.clear();
		while (std::getline(input, line)) {
			if (merger_skipFile(line)) continue;
#if ASSET_MPL != 1
			if (hdfiles.count(line) != 0) {
				std::string path = assetPath(basePath, line);
				if (SFileReadLocalFile(path.c_str(), NULL) != 0) {
					listfiles.push_back(line);
				}
				continue;
			}
			if (hdOnly)
				continue;
#endif
			for (int i = 0; i < NUM_MPQS; i++) {
				if (diabdat_mpqs[i] == NULL) continue;
				if (SFileReadArchive(diabdat_mpqs[i], line.c_str(), NULL) != 0) {
					listfiles.push_back(line);
					break;
				}
			}
		}

		int entryCount = listfiles.size();
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
		int mpqIdx;
		std::string path = mpqPath(&mpqIdx);
		archive = SFileCreateArchive(path.c_str(), hashCount, hashCount);
		if (archive == NULL) {
			app_warn("Unable to create MPQ file %s.", path.c_str());
			return RETURN_ERROR;
		}
		hashCount = 0;
		workPhase++;
	} break;
	case 2:
	{	// add the next file to the mpq
		const char* fileName = listfiles[hashCount].c_str();
		BYTE* buf = NULL;
		DWORD dwLen = 0;
#if ASSET_MPL != 1
		if (hdfiles.count(listfiles[hashCount]) != 0) {
			std::string basePath = std::string(GetBasePath());
			std::string path = assetPath(basePath, listfiles[hashCount]);
			dwLen = SFileReadLocalFile(path.c_str(), &buf);
			if (dwLen == 0) {
				mem_free_dbg(buf);
				app_warn("Could not read file: '%s'.", path.c_str());
				return RETURN_ERROR;
			}
		} else {
#else
		{
#endif
		for (int i = 0; i < NUM_MPQS; i++) {
			if (diabdat_mpqs[i] == NULL) continue;
			dwLen = SFileReadArchive(diabdat_mpqs[i], fileName, &buf);
			if (dwLen != 0) {
				break;
			}
			MemFreeDbg(buf);
		}
		if (dwLen == 0) {
			app_warn("Failed to read %s from the MPQs.", fileName);
			return RETURN_ERROR;
		}
		}
		bool success = SFileWriteFile(archive, fileName, buf, dwLen);
		mem_free_dbg(buf);
		if (!success) {
			app_warn("Unable to write %s to the MPQ.", fileName);
			return RETURN_ERROR;
		}

		hashCount++;
		if (hashCount < listfiles.size())
			break;
		SFileFlushAndCloseArchive(archive);
		archive = NULL;
		workPhase++;
	} break;
	case 3:
	{	// test the result
		int mpqIdx;
		std::string path = mpqPath(&mpqIdx);
		diabdat_mpqs[mpqIdx] = SFileOpenArchive(path.c_str(), MPQ_OPEN_READ_ONLY);
		if (diabdat_mpqs[mpqIdx] == NULL) {
			app_warn("Failed to create %s.", path.c_str());
			return RETURN_ERROR;
		}
		diabdat_paths[mpqIdx] = path;
	} return RETURN_DONE;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	Uint32 now = SDL_GetTicks();
	if (!SDL_TICKS_PASSED(now, sgMergerRenderTc + gnRefreshDelay))
		goto restart;
	sgMergerRenderTc = now;

	while (++workProgress >= 100)
		workProgress -= 100;
	return workProgress;
}

void UiMergerDialog()
{
	LoadBackgroundArt(NULL, "ui_art\\menu.pal");
	workPhase = 0;
	MergerInit();

	workProgress = RETURN_DONE;
	do {
		UiRenderAndPoll();
	} while (workProgress == RETURN_DONE);
	MergerFreeDlgItems();
	// FreeBackgroundArt();

	if (workProgress != 0)
		return;

	// check if the mpq already exists
	int mpqIdx;
	std::string path = mpqPath(&mpqIdx);
	if (diabdat_mpqs[mpqIdx] != NULL) {
		char dialogTitle[32];
		char dialogText[256];
		snprintf(dialogTitle, sizeof(dialogTitle), "Merged MPQ exists");
		snprintf(dialogText, sizeof(dialogText), "Are you sure you want to overwrite the existing file?");

		if (!UiSelYesNoDialog(dialogTitle, dialogText))
			return;

		SFileCloseArchive(diabdat_mpqs[mpqIdx]);
		diabdat_mpqs[mpqIdx] = NULL;
		diabdat_paths[mpqIdx].clear();
		RemoveFile(path.c_str());
	}
	// ensure the merged mpq is not used as a source
	HANDLE mergedHandle = diabdat_mpqs[NUM_MPQS];
	diabdat_mpqs[NUM_MPQS] = NULL;

	// do the actual merge
	workPhase = 0;
	sgMergerRenderTc = SDL_GetTicks();

	UiProgressDialog("...Merge in progress...", merger_callback);
	// cleanup
	listfiles.clear();
#if ASSET_MPL != 1
	hdfiles.clear();
#endif
	// ensure mpq-archive is closed on error
	// if (workProgress == RETURN_ERROR && workPhase == 2) {
		SFileCloseArchive(archive);
		archive = NULL;
	// }
	// restore merged mpq if the target was hd-only
	// if (hdOnly) {
		if (mergedHandle != NULL) {
			diabdat_mpqs[NUM_MPQS] = mergedHandle;
		}
	// }
}

DEVILUTION_END_NAMESPACE
