#include "diablo.h"
#include "diabloui.h"
#include "selok.h"
#include "utils/paths.h"
#include "utils/md5.h"

DEVILUTION_BEGIN_NAMESPACE

static unsigned workProgress;
static unsigned workPhase;
static MD5* md5;

static constexpr int RETURN_ERROR = 101;
static constexpr int RETURN_DONE = 100;

static int checkResult[NUM_MPQS + 1];

typedef struct FileMetaInfo {
	const char* fileName;
	const char* fileHash0; // GOG | patched devilx-mpq
	const char* fileHash1; // ??? | non-patched devilx-mpq
	bool optional;
} FileMetaInfo;

#ifdef HELLFIRE
#define MPQDEVXP_HASH "17ff1a81ac05e54d552c7f75cc8d94d8"
#define MPQONE_HASH "03d484146ad5191e81074ca9af6c6c2a"
#else
#define MPQDEVXP_HASH "4c0681b79a0af6d3d6907f985b9a3bc5"
#define MPQONE_HASH "ba32299daae9a8e96c7c8d923fd4272b"
#endif
#if USE_MPQONE
#define MPQONE_OPTIONAL false
#else
#define MPQONE_OPTIONAL true
#endif

static const FileMetaInfo filemetadata[] = {
#if ASSET_MPL != 1
	{ "DEVILHD.MPQ", "", "", false },
#endif
	{ "DEVILX.MPQ", MPQDEVXP_HASH, "646be569d22f1d542851008ea6c9e22f", false },
#ifdef HELLFIRE
	//{ "HF_OPT2.MPQ", "", "", true },
	//{ "HF_OPT1.MPQ", "", "", true },
	{ "HF_VOICE.MPQ", "6ae6ce3e89ece92c1e3e912a91d0b186", "6ae6ce3e89ece92c1e3e912a91d0b186", false },
	{ "HF_MUSIC.MPQ", "5f79b271b4a291fc8968df7e8aa80d52", "f40a5dede03e84b663a1ced6ddc1cc20", false },
	//{ "HF_BARB.MPQ", "", "", true },
	//{ "HF_BARD.MPQ", "", "", true },
	{ "HF_MONK.MPQ", "5a6b8f1ef6d505d469c31aef6e48e89d", "5a6b8f1ef6d505d469c31aef6e48e89d", false },
	{ "HELLFIRE.MPQ", "c996bd970df13ea7aa5e2417f8e78b9f", "c996bd970df13ea7aa5e2417f8e78b9f", false },
#endif
	//{ "PATCH_RT.MPQ", "d2488b30310c1d293eaf068a29709e65", "", true },
	{ "DIABDAT.MPQ", "011bc6518e6166206231080a4440b373", "68f049866b44688a7af65ba766bef75a", false },
	{ MPQONE, MPQONE_HASH, "", MPQONE_OPTIONAL },
};
static_assert(NUM_MPQS + 1 == sizeof(filemetadata) / sizeof(FileMetaInfo), "Mismatching metadata.");

static int checker_callback()
{
	if (!diabdat_paths[workPhase].empty()) {
		char* res = md5->digestFile(diabdat_paths[workPhase].c_str());

		if (SDL_strcmp(filemetadata[workPhase].fileHash0, res) == 0) {
			checkResult[workPhase] = 0;
		} else if (SDL_strcmp(filemetadata[workPhase].fileHash1, res) == 0) {
			checkResult[workPhase] = 1;
		} else {
			// LogErrorF("", "File:%s hash:%s", diabdat_paths[workPhase].c_str(), res);
			checkResult[workPhase] = 2;
		}
	} else {
		checkResult[workPhase] = -1;
	}

	if (++workPhase > NUM_MPQS) {
		return RETURN_DONE;
	}

	workProgress = (99 * workPhase / NUM_MPQS) - 1;

	while (++workProgress >= 100)
	workProgress -= 100;
	return workProgress;
}

void UiCheckerDialog()
{
	workProgress = 0;
	workPhase = 0;
	md5 = new MD5();

	bool result = UiProgressDialog("...Check in progress...", checker_callback);

	delete md5;
	md5 = NULL;

	if (!result) {
		return;
	}

	{
		char dialogTitle[32];
		char dialogText[256];
		dialogTitle[0] = '\0';
		dialogText[0] = '\0';
		int cursor = 0;

		for (int i = 0; i <= NUM_MPQS && cursor < sizeof(dialogText) - 1; i++) {
			if (checkResult[i] == 0) {
				// LogErrorF("", "File:%s is correct (GOG).", filemetadata[i].fileName);
				continue;
			}
			if (checkResult[i] == 1) {
				if (i != MPQ_DEVILX) {
					// LogErrorF("", "File:%s is correct (???).", filemetadata[i].fileName);
					continue;
				}
			}
			if (filemetadata[i].optional && i != NUM_MPQS) {
				// LogErrorF("", "File:%s ignored.", filemetadata[i].fileName);
				continue;
			}
			if (dialogText[0] != '\0') {
				dialogText[cursor] = '\n';
				cursor++;
				dialogText[cursor] = '0';
			}
			if (checkResult[i] < 0) {
				// file is missing/not created
				if (i != NUM_MPQS)
					cursor += snprintf(&dialogText[cursor], sizeof(dialogText) - cursor, "%s is missing.", filemetadata[i].fileName);
				else
					cursor += snprintf(&dialogText[cursor], sizeof(dialogText) - cursor, "%s is not created.", filemetadata[i].fileName);
			} else if (checkResult[i] == 1) {
				// matching hash
				assert(i == MPQ_DEVILX);
				cursor += snprintf(&dialogText[cursor], sizeof(dialogText) - cursor, "%s must be patched.", filemetadata[i].fileName);
			} else {
				// hash does not match
				if (i != NUM_MPQS)
					cursor += snprintf(&dialogText[cursor], sizeof(dialogText) - cursor, "%s is corrupt.", filemetadata[i].fileName);
				else
					cursor += snprintf(&dialogText[cursor], sizeof(dialogText) - cursor, "%s must be recreated.", filemetadata[i].fileName);
			}
			// LogErrorF("", "File:%s reported. cursor at %d.", filemetadata[i].fileName, cursor);
		}
#if USE_MPQONE
		if (checkResult[NUM_MPQS] == 0 && dialogText[0] != '\0') {
			snprintf(dialogText, sizeof(dialogText), "The merged mpq file is ready to be used.");
		}
#endif
		if (dialogText[0] == '\0') {
			snprintf(dialogText, sizeof(dialogText), "The mpq files are complete.");
		}

		UiSelOkDialog(dialogTitle, dialogText);
	}
}

DEVILUTION_END_NAMESPACE
