#include <string>
#include <fstream>

#include "diablo.h"
#include "diabloui.h"
#include "selok.h"
#include "selyesno.h"
#include "utils/paths.h"
#include "utils/file_util.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
#ifdef NOSOUND
static bool noSound = true;
#else
static bool noSound = false;
#endif

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

	UiAddBackground();
	UiAddLogo();

	SDL_Rect rect1 = { PANEL_LEFT, SELHERO_TITLE_TOP, PANEL_WIDTH, 35 };
	gUiItems.push_back(new UiText("Merge MPQ files", rect1, UIS_HCENTER | UIS_BIG | UIS_SILVER));

	gUIListItems.push_back(new UiListItem("Start merge", 0));
	gUIListItems.push_back(new UiListItem(noSound ? "With Sound Assets: No" : "With Sound Assets: Yes", 1));
	gUIListItems.push_back(new UiListItem("Cancel", 2));

	SDL_Rect rect5 = { PANEL_MIDX(MAINMENU_WIDTH), SELGAME_LIST_TOP, MAINMENU_WIDTH, 26 * 3 };
	gUiItems.push_back(new UiList(&gUIListItems, 3, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	//assert(gUIListItems.size() == 3);
	UiInitScreen(3, NULL, MergerSelect, MergerEsc);

	UiFocus(workPhase);
}

static void MergerSelect(unsigned index)
{
	workPhase = index;

	switch (index) {
	case 0:
		workProgress = 0;
		break;
	case 1:
		noSound = !noSound;
		MergerInit();
		break;
	case 2:
		workProgress = RETURN_CANCEL;
		break;
	}
}

static int merger_callback()
{
	switch (workPhase) {
	case 0:
	{	// first round - read the content and prepare the metadata
		std::string listpath = std::string(GetBasePath()) + "listfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "listfiles.txt");
			return RETURN_ERROR;
		}
		std::string line;
		int entryCount = 0;
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
		std::string path = std::string(GetBasePath()) + MPQONE;
		if (!OpenMPQ(path.c_str(), hashCount, hashCount)) {
			app_warn("Unable to open MPQ file %s.", path.c_str());
			return RETURN_ERROR;
		}
		hashCount = 0;
		workPhase++;
	} break;
	case 2:
	{	// add the content
		std::string listpath = std::string(GetBasePath()) + "listfiles.txt";
		std::ifstream input(listpath);
		if (input.fail()) {
			app_warn("Can not find/access '%s' in the game folder.", "listfiles.txt");
			return RETURN_ERROR;
		}
		// create the mpq file
		int skip = hashCount;
		std::string line;
		while (std::getline(input, line)) {
			// skip sound files if requested
			if (noSound && line.size() >= 4 && SDL_strcasecmp(line.c_str() + line.size() - 4, ".wav") == 0)
				continue;
			// skip hellfire/vanilla files
			int n = 0;
			for ( ; n < lengthof(filesToSkip); n++) {
				if (SDL_strcmp(line.c_str(), filesToSkip[n]) == 0) {
					break;
				}
			}
			if (n != lengthof(filesToSkip)) {
				continue;
			}
			// process only a bunch of files at a time to be more responsive
			if (--skip >= 0) {
				continue;
			}
			if (skip <= -10) {
				break;
			}
			// add the file to the mpq
			for (int i = 0; i < NUM_MPQS; i++) {
				HANDLE hFile;
				if (diabdat_mpqs[i] != NULL && SFileOpenFileEx(diabdat_mpqs[i], line.c_str(), SFILE_OPEN_FROM_MPQ, &hFile)) {
					DWORD dwLen = SFileGetFileSize(hFile);
					BYTE* buf = DiabloAllocPtr(dwLen);
					if (!SFileReadFile(hFile, buf, dwLen)) {
						app_warn("Unable to open file archive");
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
		mpqapi_flush_and_close(true);
		workPhase++;
	} break;
	case 3:
	{	// test the result
		std::string path = std::string(GetBasePath()) + MPQONE;
		diabdat_mpqs[NUM_MPQS] = SFileOpenArchive(path.c_str(), MPQ_OPEN_READ_ONLY);
		if (diabdat_mpqs[NUM_MPQS] == NULL) {
			app_warn("Failed to create %s.", path.c_str());
			return RETURN_ERROR;
		}
		diabdat_paths[NUM_MPQS] = path;
	} return RETURN_DONE;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	while (++workProgress >= 100)
		workProgress -= 100;
	return workProgress;
}

void UiMergerDialog()
{
	LoadBackgroundArt("ui_art\\mainmenu.CEL", "ui_art\\menu.pal");
	workPhase = 0;
	MergerInit();

	workProgress = RETURN_DONE;
	do {
		UiRenderAndPoll();
	} while (workProgress == RETURN_DONE);
	MergerFreeDlgItems();
	FreeBackgroundArt();

	if (workProgress != 0)
		return;

	// check if the mpq already exists
	if (diabdat_mpqs[NUM_MPQS] != NULL) {
		char dialogTitle[32];
		char dialogText[256];
		snprintf(dialogTitle, sizeof(dialogTitle), "Merged MPQ exists");
		snprintf(dialogText, sizeof(dialogText), "Are you sure you want to overwrite the existing file?");

		if (!UiSelYesNoDialog(dialogTitle, dialogText))
			return;

		SFileCloseArchive(diabdat_mpqs[NUM_MPQS]);
		diabdat_mpqs[NUM_MPQS] = NULL;
		diabdat_paths[NUM_MPQS].clear();
		std::string path = std::string(GetBasePath()) + MPQONE;
		RemoveFile(path.c_str());
	}

	// do the actual merge
	workPhase = 0;
	UiProgressDialog("...Merge in progress...", merger_callback);
}

DEVILUTION_END_NAMESPACE
