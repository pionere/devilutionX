
#include "DiabloUI/diablo.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/selconn.h"
#include "DiabloUI/selhero.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/text.h"
#include "storm/storm_cfg.h"
#include "storm/storm_net.h"
#include "all.h"
//#include "../diablo.h"
//#include <algorithm>
//#include "../engine.h"
//#include "storm/storm.h"

DEVILUTION_BEGIN_NAMESPACE

DISABLE_SPEED_OPTIMIZATION

#define MAX_VIEWPORT_ITEMS ((unsigned)((SELGAME_RPANEL_HEIGHT - 22) / 26))

typedef struct ConnectionInfo {
	const char *ci_GameName;
	const char *ci_GamePort;
} ConnectionInfo;

typedef struct ZtUiLabels {
	UiText *difficultyTxt;
	UiText *speedTxt;
} ZtUiLabels;

typedef struct ZtUiPlrDescription {
	char nameContent[PLR_NAME_LEN];
	UiText *nameTxt;
	UiText *classTxt;
	char levelContent[4];
	UiText *levelTxt;
	UiText *rankTxt;
	UiText *teamTxt;
} ZtUiPlrDescription;

static std::vector<ConnectionInfo> selgame_coninfos;
#ifdef ZEROTIER
static std::vector<SNetZtGame> selgame_ztGames;
static ZtUiLabels selgame_ztGameLabels;
static ZtUiPlrDescription selgame_ztPlrDescription[MAX_PLRS];
static bool ztProvider;
static UiTxtButton* ztBlOckBtn;
static UiTxtButton* ztRefreshBtn;
static Uint32 ztNextRefresh;
#else
static constexpr bool ztProvider = false;
static constexpr UiTxtButton* ztBlOckBtn = NULL;
#endif
static unsigned selgame_connum;
static unsigned selgame_conidx;

static UiTxtButton* SELLIST_DIALOG_DELETE_BUTTON;

static char selgame_Label[32];
static_assert(NET_MAX_GAMENAME_LEN >= NET_ZT_GAMENAME_LEN, "zerotier game-id does not fit to selgame_GameName.");
static char selgame_GameName[NET_MAX_GAMENAME_LEN + 1] = "";
static char selgame_GamePort[8] = "";
static char selgame_Password[NET_MAX_PASSWD_LEN + 1] = "";
static char selgame_Description[128];
static int selgame_mode; // _selgame_selections
static bool selgame_endMenu;
//int selgame_heroLevel;

static _uigamedata* selgame_gameData;

#define DESCRIPTION_WIDTH (SELGAME_LPANEL_WIDTH - 2 * 10)

// Forward-declare UI-handlers, used by other handlers.
static void SelgameAddressListInit();
static void SelgameDiffSelect(unsigned index);
static void SelgameSpeedSelect(unsigned index);
static void SelgamePasswordSelect(unsigned index);
static void SelgameDiffEsc();
static void SelgamePasswordEsc();

static void selgame_handleEvents(SNetEventHdr* pEvt)
{
	const SNetGameData* gameData;
	unsigned playerId;
	turn_t turn;
	SNetJoinEvent *ev = (SNetJoinEvent*)pEvt;

	assert(ev->neHdr.eventid == EVENT_TYPE_JOIN_ACCEPTED);
	gameData = ev->neGameData;
	assert(gameData->ngVersionId == GAME_VERSION);
	playerId = ev->neHdr.playerid;
	assert((DWORD)playerId < MAX_PLRS);
	turn = ev->neTurn + 1;

	selgame_gameData->aeVersionId = gameData->ngVersionId;
	selgame_gameData->aeSeed = gameData->ngSeed;
	selgame_gameData->aeDifficulty = gameData->ngDifficulty;
	selgame_gameData->aeTickRate = gameData->ngTickRate;
	selgame_gameData->aeNetUpdateRate = gameData->ngNetUpdateRate;
	selgame_gameData->aeMaxPlayers = gameData->ngMaxPlayers;

	selgame_gameData->aeTurn = turn;
	selgame_gameData->aePlayerId = playerId;
}

static void selgame_add_event_handlers()
{
	SNetRegisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED, selgame_handleEvents);
	SNetRegisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME, multi_ui_handle_events);
#ifdef ZEROTIER
	SNetRegisterEventHandler(EVENT_TYPE_PLAYER_INFO, multi_ui_handle_events);
#endif
}

static void selgame_remove_event_handlers()
{
#ifdef ZEROTIER
	SNetUnregisterEventHandler(EVENT_TYPE_PLAYER_INFO);
#endif
	SNetUnregisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME);
	SNetUnregisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED);
}

static void SelgameInit()
{
	LoadBackgroundArt(NULL, "ui_art\\menu.pal");
}

static void SelgameFreeDlgItems()
{
	UiClearListItems();

	UiClearItems();
}

static void SelgameFree()
{
	// FreeBackgroundArt();
	SelgameFreeDlgItems();

	// memset(&selgame_Password, 0, sizeof(selgame_Password)); - pointless because the plain password is stored in storm anyway...
}

static void SelgameResetScreen(const char* title, const char* rheader)
{
	SelgameFreeDlgItems();

	// UiAddBackground();
	UiAddLogo();

	gUiItems.push_back(new UiTextBox({ SELGAME_LPANEL_LEFT - BOXBORDER_WIDTH, SELGAME_PNL_TOP - BOXBORDER_WIDTH, SELGAME_LPANEL_WIDTH + 2 * BOXBORDER_WIDTH, SELGAME_HEADER_HEIGHT + SELGAME_LPANEL_HEIGHT + 2 * BOXBORDER_WIDTH }, UIS_HCENTER | UIS_SILVER));

	gUiItems.push_back(new UiTextBox({ SELGAME_RPANEL_LEFT - BOXBORDER_WIDTH, SELGAME_PNL_TOP - BOXBORDER_WIDTH, SELGAME_RPANEL_WIDTH + 2 * BOXBORDER_WIDTH, SELGAME_HEADER_HEIGHT + SELGAME_RPANEL_HEIGHT + 2 * BOXBORDER_WIDTH }, UIS_HCENTER | UIS_GOLD));

	SDL_Rect rect1 = { 0, SELGAME_TITLE_TOP, SCREEN_WIDTH, 35 };
	gUiItems.push_back(new UiText(title, rect1, UIS_HCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { SELGAME_LPANEL_LEFT + 10, SELGAME_PNL_TOP, DESCRIPTION_WIDTH, SELGAME_HEADER_HEIGHT };
	gUiItems.push_back(new UiText(selgame_Label, rect2, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect3 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LIST_TOP, DESCRIPTION_WIDTH, 192 };
	gUiItems.push_back(new UiText(selgame_Description, rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { SELGAME_RPANEL_LEFT, SELGAME_PNL_TOP, SELGAME_RPANEL_WIDTH, SELGAME_HEADER_HEIGHT };
	gUiItems.push_back(new UiText(rheader, rect4, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	bool hosted = false;
#ifndef NOHOSTING
	hosted = provider == SELCONN_TCPS || provider == SELCONN_TCPDS;
#endif
	SDL_Rect rect5 = { SELGAME_LPANEL_LEFT + (DESCRIPTION_WIDTH - SELHERO_HEROS_WIDTH) / 2, SELGAME_LPANEL_BOTTOM - 30 - SELHERO_HEROS_HEIGHT, SELHERO_HEROS_WIDTH, SELHERO_HEROS_HEIGHT };
	gUiItems.push_back(new UiImage(gbHerosCel, hosted ? 0 : selhero_heroInfo.hiClass + 1, rect5, false));
	if (!hosted) {
		SDL_Rect rect6 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LPANEL_BOTTOM - 30, DESCRIPTION_WIDTH, 30 };
		gUiItems.push_back(new UiText(selhero_heroInfo.hiName, rect6, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));
	}
#ifdef ZEROTIER
	ztBlOckBtn = NULL;
	ztRefreshBtn = NULL;
#endif
}

static void SelgameModeEsc()
{
	selgame_mode = SELGAME_PREVIOUS;
	selgame_endMenu = true;
}

static void SelgameModeFocus(unsigned index)
{
	selgame_Label[0] = '\0';
	const char* txt;
	switch (gUIListItems[index]->m_value) {
	case SELGAME_CREATE:
		txt = "Create a new game with a difficulty setting of your choice";
		break;
	case SELGAME_JOIN:
		txt = "Enter an IP or a hostname and join a game already in progress";
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	DISABLE_WARNING(format-security, format-security, 4774)
	snprintf(selgame_Description, sizeof(selgame_Description), txt);
	ENABLE_WARNING(format-security, format-security, 4774)
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH, (unsigned)UIS_SMALL >> 0);
}

static std::pair<const char*, const char*> SelgameDiffText(int difficulty)
{
	std::pair<const char*, const char*> result = { "Normal", "This is where a starting character should begin the quest to defeat Diablo" };
	if (difficulty == DIFF_NIGHTMARE)
		result = { "Nightmare", "The denizens of the Labyrinth have been bolstered and will prove to be a greater challenge" };
	if (difficulty == DIFF_HELL)
		result = { "Hell", "The most powerful of the underworld's creatures lurk at the gateway into Hell" };
	return result;
}

static void SelgameDiffFocus(unsigned index)
{
	int diff = gUIListItems[index]->m_value;
	const std::pair<const char*, const char*> diffTexts = SelgameDiffText(diff);
	DISABLE_WARNING(format-security, format-security, 4774)
	snprintf(selgame_Label, sizeof(selgame_Label), diffTexts.first);
	snprintf(selgame_Description, sizeof(selgame_Description), diffTexts.second);
	ENABLE_WARNING(format-security, format-security, 4774)
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH, (unsigned)UIS_SMALL >> 0);
}

static std::pair<const char*, const char*> SelgameSpeedText(int speed)
{
	std::pair<const char*, const char*> result = { "Normal", "This is where a starting character should begin the quest to defeat Diablo" };
	if (speed == SPEED_FAST)
		result = { "Fast", "The denizens of the Labyrinth have been hastened and will prove to be a greater challenge" };
	if (speed == SPEED_FASTER)
		result = { "Faster", "Most monsters of the dungeon will seek you out quicker than ever before" };
	if (speed == SPEED_FASTEST)
		result = { "Fastest", "The minions of the underworld will rush to attack without hesitation" };
	return result;
}

static void SelgameSpeedFocus(unsigned index)
{
	int speed = gUIListItems[index]->m_value;
	const std::pair<const char*, const char*> speedTexts = SelgameSpeedText(speed);
	DISABLE_WARNING(format-security, format-security, 4774)
	snprintf(selgame_Label, sizeof(selgame_Label), speedTexts.first);
	snprintf(selgame_Description, sizeof(selgame_Description), speedTexts.second);
	ENABLE_WARNING(format-security, format-security, 4774)
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH, (unsigned)UIS_SMALL >> 0);
}

static void SelgameNoFocus()
{
	selgame_Label[0] = '\0';
	selgame_Description[0] = '\0';
}

static void SelgameDiffInit()
{
	SelgameResetScreen("Create Game", "Select Difficulty");

	gUIListItems.push_back(new UiListItem("Normal", DIFF_NORMAL));
	gUIListItems.push_back(new UiListItem("Nightmare", DIFF_NIGHTMARE));
	gUIListItems.push_back(new UiListItem("Hell", DIFF_HELL));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + (SELGAME_RPANEL_WIDTH - 250) / 2, SELGAME_LIST_TOP, 250, 26 * 3 };
	gUiItems.push_back(new UiList(&gUIListItems, 3, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(gUIListItems.size() == 3);
	UiInitScreen(3, SelgameDiffFocus, SelgameDiffSelect, SelgameDiffEsc);
}

static void SelgameModeSet(unsigned value)
{
	selgame_mode = value;

	//gfnHeroInfo(UpdateHeroLevel);
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	snprintf(selgame_GamePort, sizeof(selgame_GamePort), "%d", port);
	if (value == SELGAME_CREATE) {
		SelgameDiffInit();
		return;
	}
	if (value == SELGAME_LOAD) {
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}
	SelgameAddressListInit();
}

static void SelgameModeSelect(unsigned index)
{
	SelgameModeSet(gUIListItems[index]->m_value);
}

static void SelgameModeInit()
{
#ifndef NOHOSTING
	if (provider == SELCONN_TCPS || provider == SELCONN_TCPDS) {
		SelgameModeSet(SELGAME_CREATE);
		return;
	}
#endif
	if (provider == SELCONN_LOOPBACK) {
		if (!selhero_heroInfo.hiSaveFile) {
			SelgameModeSet(SELGAME_CREATE);
			return;
		}

		SelgameResetScreen("Single Player Game", "Select Action");

		gUIListItems.push_back(new UiListItem("Load Game", SELGAME_LOAD));
		gUIListItems.push_back(new UiListItem("New Game", SELGAME_CREATE));
		SDL_Rect rect2 = { SELGAME_RPANEL_LEFT + (SELGAME_RPANEL_WIDTH - 280) / 2, SELGAME_LIST_TOP, 280, 26 * 2 };
		gUiItems.push_back(new UiList(&gUIListItems, 2, rect2, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect3 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
		gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect3, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect4 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
		gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		//assert(gUIListItems.size() == 2);
		UiInitScreen(2, NULL, SelgameModeSelect, SelgameModeEsc);
		return;
	}

	SelgameResetScreen("Multi Player Game", "Select Action");

	gUIListItems.push_back(new UiListItem("Create Game", SELGAME_CREATE));
	gUIListItems.push_back(new UiListItem("Join Game", SELGAME_JOIN));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + (SELGAME_RPANEL_WIDTH - 260) / 2, SELGAME_LIST_TOP, 260, 26 * 2 };
	gUiItems.push_back(new UiList(&gUIListItems, 2, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(gUIListItems.size() == 2);
	UiInitScreen(2, SelgameModeFocus, SelgameModeSelect, SelgameModeEsc);
}

/**
 * @brief Load the current hero level from save file
 * @param pInfo Hero info
 * @return always true
 */
/*bool UpdateHeroLevel(_uiheroinfo *pInfo)
{
	if (pInfo->hiIdx == mySaveIdx)
		selgame_heroLevel = pInfo->hiLevel;

	return true;
}*/

static void SelgamePasswordInit(unsigned index)
{
	selgame_Password[0] = '\0';

	SelgameNoFocus();

	if (selgame_mode != SELGAME_CREATE) {
		snprintf(selgame_Description, sizeof(selgame_Description), "Game: %s", selgame_GameName);
	}

	SelgameResetScreen(selgame_mode == SELGAME_CREATE ? "Create Game" : "Join Game", "Enter Password");

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	UiTxtButton *btn = new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | (ztProvider ? UIS_SILVER | UIS_DISABLED : UIS_GOLD));
#ifdef ZEROTIER
	if (ztProvider) {
		ztBlOckBtn = btn;
	}
#endif
	gUiItems.push_back(btn);

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 24, SELGAME_CONTENT_TOP + (SELGAME_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELGAME_RPANEL_WIDTH - 24 * 2, FOCUS_MEDIUM };
	static_assert(sizeof(selgame_Password) <= UIEDIT_MAXLENGTH, "The edit field of SelgamePasswordInit must fit to UIEdit.");
	UiEdit* edit = new UiEdit("Enter Password", selgame_Password, sizeof(selgame_Password), rect5);
	edit->m_iFlags |= UIS_OPTIONAL;
	gUiItems.push_back(edit);

	UiInitScreen(0, NULL, SelgamePasswordSelect, SelgamePasswordEsc);
	UiInitEdit(edit);
}

static void SelgamePortInit(unsigned index)
{
	SelgameNoFocus();
	SelgameResetScreen(selgame_mode == SELGAME_CREATE ? "Create Game" : "Join Game", "Enter Port");

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 24, SELGAME_CONTENT_TOP + (SELGAME_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELGAME_RPANEL_WIDTH - 24 * 2, FOCUS_MEDIUM };
	static_assert(sizeof(selgame_GamePort) <= UIEDIT_MAXLENGTH, "The edit field of SelgamePortInit must fit to UIEdit.");
	UiEdit* edit = new UiEdit("Enter Port", selgame_GamePort, sizeof(selgame_GamePort), rect5);
	gUiItems.push_back(edit);

	UiInitScreen(0, NULL, SelgamePasswordInit, SelgamePasswordEsc);
	UiInitEdit(edit);
}

static void SelgameUpdateViewportItems()
{
	const unsigned numViewportCons = std::min(selgame_connum + 1 - ListOffset, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < numViewportCons; i++) {
		const unsigned index = i + ListOffset;
		gUIListItems[i]->m_text = selgame_coninfos[index].ci_GameName;
		//gUIListItems[i]->m_value = index;
	}
}

static void SelgameAddressListFocus(unsigned index)
{
	SelgameUpdateViewportItems();

	selgame_Label[0] = '\0';
	selgame_Description[0] = '\0';

	int baseFlags = UIS_HCENTER | UIS_VCENTER | UIS_BIG;
	if (!ztProvider) {
		if (index != selgame_connum) {
			SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_GOLD;

			snprintf(selgame_Description, sizeof(selgame_Description), "Port: %s", selgame_coninfos[index].ci_GamePort);
		} else {
			SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_SILVER | UIS_DISABLED;
		}
#ifdef ZEROTIER
	} else {
		if (index != selgame_connum) {
			const SNetZtGame& ztGame = selgame_ztGames[index];
			selgame_ztGameLabels.difficultyTxt->m_text = SelgameDiffText(ztGame.ngData.ngDifficulty).first;
			selgame_ztGameLabels.speedTxt->m_text = SelgameSpeedText(ztGame.ngData.ngTickRate).first;

			for (int i = 0; i < MAX_PLRS; i++) {
				const SNetZtPlr &ztPlr = ztGame.ngPlayers[i];
				memcpy(selgame_ztPlrDescription[i].nameContent, ztPlr.npName, sizeof(selgame_ztPlrDescription[i].nameContent));
				if (ztPlr.npName[0] == '\0') {
					selgame_ztPlrDescription[i].levelContent[0] = '\0';
					selgame_ztPlrDescription[i].classTxt->m_text = "";
					selgame_ztPlrDescription[i].rankTxt->m_text = "";
					selgame_ztPlrDescription[i].teamTxt->m_text = "";
					continue;
				}
				snprintf(selgame_ztPlrDescription[i].levelContent, sizeof(selgame_ztPlrDescription[i].levelContent), "%d", ztPlr.npLevel);
				selgame_ztPlrDescription[i].classTxt->m_text = ztPlr.npClass < NUM_CLASSES ? ClassStrTbl[ztPlr.npClass] : "?";
				const char* tmp = "";
				if (ztPlr.npRank == 1)
					tmp = "*";
				if (ztPlr.npRank == 2)
					tmp = "**";
				if (ztPlr.npRank == 3)
					tmp = "***";
				selgame_ztPlrDescription[i].rankTxt->m_text = tmp;
				tmp = "A";
				if (ztPlr.npTeam == 1)
					tmp = "B";
				if (ztPlr.npTeam == 2)
					tmp = "C";
				if (ztPlr.npTeam == 3)
					tmp = "D";
				selgame_ztPlrDescription[i].teamTxt->m_text = tmp;
			}
		} else {
			selgame_ztGameLabels.difficultyTxt->m_text = "";
			selgame_ztGameLabels.speedTxt->m_text = "";
			for (int i = 0; i < MAX_PLRS; i++) {
				selgame_ztPlrDescription[i].nameContent[0] = '\0';
				selgame_ztPlrDescription[i].levelContent[0] = '\0';
				selgame_ztPlrDescription[i].classTxt->m_text = "";
				selgame_ztPlrDescription[i].rankTxt->m_text = "";
				selgame_ztPlrDescription[i].teamTxt->m_text = "";
			}
		}
#endif
	}
}

static void SelgameAddressEsc()
{
	if (selgame_connum == 0) {
		SelgameModeInit();
	} else {
		SelgameAddressListInit();
	}
}

static void SelgameAddressInit()
{
	SelgameNoFocus();
	SelgameResetScreen("Join Game", ztProvider ? "Enter Game ID" : "Enter Address");

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 24, SELGAME_CONTENT_TOP + (SELGAME_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELGAME_RPANEL_WIDTH - 24 * 2, FOCUS_MEDIUM };
	static_assert(NET_ZT_GAMENAME_LEN <= UIEDIT_MAXLENGTH, "The edit field of SelgameAddressInit must fit to UIEdit I.");
	static_assert(sizeof(selgame_GameName) - (NET_TCP_PORT_LENGTH + 1) <= UIEDIT_MAXLENGTH, "The edit field of SelgameAddressInit must fit to UIEdit II.");
	UiEdit* edit = new UiEdit(ztProvider ? "Enter Game ID" : "Enter Address", selgame_GameName, ztProvider ? NET_ZT_GAMENAME_LEN : (sizeof(selgame_GameName) - (NET_TCP_PORT_LENGTH + 1)), rect5);
	gUiItems.push_back(edit);

	UiInitScreen(0, NULL, ztProvider ? SelgamePasswordInit : SelgamePortInit, SelgameAddressEsc);
	UiInitEdit(edit);
}

static void SelgameAddressListSelect(unsigned index)
{
	if (ztBlOckBtn != NULL)
		return;

	selgame_conidx = index;

	if (index != selgame_connum) {
		SStrCopy(selgame_GameName, selgame_coninfos[index].ci_GameName, sizeof(selgame_GameName));
		SStrCopy(selgame_GamePort, selgame_coninfos[index].ci_GamePort, sizeof(selgame_GamePort));

		SelgamePasswordInit(0);
	} else {
		selgame_GameName[0] = '\0';
		// TODO: load NET_DEFAULT_PORT ?

		SelgameAddressInit();
	}
}

static void SelgameAddressListDelete()
{
	unsigned index = SelectedItem;
	// if (SELLIST_DIALOG_DELETE_BUTTON->m_iFlags & UIS_DISABLED)
	if (index == selgame_connum)
		return;
	unsigned lastIndex = selgame_connum - 1;
	for (unsigned i = index; ; i++) {
		snprintf(tempstr, sizeof(tempstr), "Entry%d", i);
		if (i < lastIndex) {
			setIniValue("Phone Book", tempstr, selgame_coninfos[i + 1].ci_GameName);
		} else {
			delIniValue("Phone Book", tempstr);
		}
		snprintf(tempstr, sizeof(tempstr), "Entry%dPort", i);
		if (i < lastIndex) {
			setIniValue("Phone Book", tempstr, selgame_coninfos[i + 1].ci_GamePort);
		} else {
			delIniValue("Phone Book", tempstr);
			break;
		}
	}
	SelgameAddressListInit();
}

static void SelgameAddressListInit()
{
	UiTxtButton* btn;
	int numEntries = 0;
	selgame_coninfos.clear();

	if (!ztProvider) {
		while (true) {
			snprintf(tempstr, sizeof(tempstr), "Entry%d", numEntries);
			const char* entryName = getIniStr("Phone Book", tempstr);
			if (entryName == NULL) {
				break;
			}
			snprintf(tempstr, sizeof(tempstr), "Entry%dPort", numEntries);
			const char* entryPort = getIniStr("Phone Book", tempstr);
			if (entryPort == NULL) {
				break;
			}
			ConnectionInfo ci = { entryName, entryPort };
			selgame_coninfos.push_back(ci);
			numEntries++;
		}
#ifdef ZEROTIER
	} else {
		if (SNetReady()) {
			SNetGetGamelist(selgame_ztGames);
			for (const SNetZtGame &game : selgame_ztGames) {
				ConnectionInfo ci = { game.ngName, "" };
				selgame_coninfos.push_back(ci);
				numEntries++;
			}
			ztNextRefresh = SDL_GetTicks() + 1000;
		} else {
			ztNextRefresh = 0;
		}
#endif
	}
	selgame_connum = numEntries;

	if (numEntries == 0 && !ztProvider) {
		SelgameAddressInit();
		return;
	}

	ConnectionInfo ci = { ztProvider ? "Enter manually..." : "New Address", "" };
	selgame_coninfos.push_back(ci);

	SelgameResetScreen("Join Game", ztProvider ? "Select Game" : "Select Address");
#ifdef ZEROTIER
	if (ztProvider) {
		SDL_Rect rect0 = { SELGAME_LPANEL_LEFT + 10, SELGAME_PNL_TOP, DESCRIPTION_WIDTH, SELGAME_HEADER_HEIGHT };
		selgame_ztGameLabels.difficultyTxt = new UiText("", rect0, UIS_LEFT | UIS_VCENTER | UIS_SMALL | UIS_SILVER);
		gUiItems.push_back(selgame_ztGameLabels.difficultyTxt);

		SDL_Rect rect1 = { SELGAME_LPANEL_LEFT + 10, SELGAME_PNL_TOP, DESCRIPTION_WIDTH, SELGAME_HEADER_HEIGHT };
		selgame_ztGameLabels.speedTxt = new UiText("", rect1, UIS_RIGHT | UIS_VCENTER | UIS_SMALL | UIS_SILVER);
		gUiItems.push_back(selgame_ztGameLabels.speedTxt);
		static_assert(SELGAME_LPANEL_HEIGHT >= MAX_PLRS * 2 * (SMALL_FONT_HEIGHT + 2) + 30 + SELHERO_HEROS_HEIGHT, "Not enough space to display the player-list of zerotier games.");
		for (int i = 0; i < MAX_PLRS; i++) {
			SDL_Rect rect2 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LIST_TOP + 2 * i * (SMALL_FONT_HEIGHT + 2), DESCRIPTION_WIDTH, SMALL_FONT_HEIGHT };
			selgame_ztPlrDescription[i].nameContent[0] = '\0';
			selgame_ztPlrDescription[i].nameTxt = new UiText(&selgame_ztPlrDescription[i].nameContent[0], rect2, UIS_LEFT | UIS_SMALL | UIS_SILVER);
			gUiItems.push_back(selgame_ztPlrDescription[i].nameTxt);

			SDL_Rect rect4 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LIST_TOP + (2 * i + 1) * (SMALL_FONT_HEIGHT + 2), DESCRIPTION_WIDTH / 2, SMALL_FONT_HEIGHT };
			selgame_ztPlrDescription[i].classTxt = new UiText("", rect4, UIS_LEFT | UIS_SMALL | UIS_SILVER);
			gUiItems.push_back(selgame_ztPlrDescription[i].classTxt);

			SDL_Rect rect3 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LIST_TOP + (2 * i + 1) * (SMALL_FONT_HEIGHT + 2), DESCRIPTION_WIDTH / 2 + 40, SMALL_FONT_HEIGHT };
			selgame_ztPlrDescription[i].levelContent[0] = '\0';
			selgame_ztPlrDescription[i].levelTxt = new UiText(&selgame_ztPlrDescription[i].levelContent[0], rect3, UIS_RIGHT | UIS_SMALL | UIS_SILVER);
			gUiItems.push_back(selgame_ztPlrDescription[i].levelTxt);

			SDL_Rect rect5 = { SELGAME_LPANEL_LEFT + 10 + DESCRIPTION_WIDTH / 2 + 40, SELGAME_LIST_TOP + (2 * i + 1) * (SMALL_FONT_HEIGHT + 2) - 3, 10, SMALL_FONT_HEIGHT };
			selgame_ztPlrDescription[i].rankTxt = new UiText("", rect5, UIS_LEFT | UIS_SMALL | UIS_SILVER);
			gUiItems.push_back(selgame_ztPlrDescription[i].rankTxt);

			SDL_Rect rect6 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LIST_TOP + (2 * i + 1) * (SMALL_FONT_HEIGHT + 2), DESCRIPTION_WIDTH, SMALL_FONT_HEIGHT };
			selgame_ztPlrDescription[i].teamTxt = new UiText("", rect6, UIS_RIGHT | UIS_SMALL | UIS_SILVER);
			gUiItems.push_back(selgame_ztPlrDescription[i].teamTxt);
		}
	}
#endif
	unsigned num_viewport_cons = std::min(selgame_connum + 1, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < num_viewport_cons; i++) {
		gUIListItems.push_back(new UiListItem("", -1));
	}
	SelgameUpdateViewportItems();

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 25, SELGAME_LIST_TOP, SELGAME_RPANEL_WIDTH - 2 * 25, 26 * (int)num_viewport_cons };
	gUiItems.push_back(new UiList(&gUIListItems, num_viewport_cons, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH - SCROLLBAR_BG_WIDTH + 1, SELGAME_CONTENT_TOP - 1, SCROLLBAR_BG_WIDTH, SELGAME_RPANEL_HEIGHT + 1 };
	UiScrollBar* scrollBar = new UiScrollBar(rect6);
	gUiItems.push_back(scrollBar);

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 3, 35 };
	btn = new UiTxtButton("OK", &UiFocusNavigationSelect, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | (ztProvider ? UIS_SILVER | UIS_DISABLED : UIS_GOLD));
#ifdef ZEROTIER
	if (ztProvider) {
		ztBlOckBtn = btn;
	}
#endif
	gUiItems.push_back(btn);

	SDL_Rect rect8 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 3, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 3, 35 };
	if (!ztProvider) {
		btn = SELLIST_DIALOG_DELETE_BUTTON = new UiTxtButton("Delete", &UiFocusNavigationDelete, rect8, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER | UIS_DISABLED);
	} else {
		btn = new UiTxtButton("Refresh", &SelgameAddressListInit, rect8, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER | UIS_DISABLED);
#ifdef ZEROTIER
		ztRefreshBtn = btn;
#endif
	}
	gUiItems.push_back(btn);

	SDL_Rect rect9 = { SELGAME_RPANEL_LEFT + 2 * SELGAME_RPANEL_WIDTH / 3, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect9, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitScreen(selgame_connum + 1, SelgameAddressListFocus, SelgameAddressListSelect, SelgameModeInit);
	UiInitScrollBar(scrollBar, MAX_VIEWPORT_ITEMS, ztProvider ? NULL : SelgameAddressListDelete);
}

static void SelgameSpeedInit()
{
	// gfnHeroInfo(UpdateHeroLevel);

	SelgameResetScreen("Create Game", "Select Game Speed");

	gUIListItems.push_back(new UiListItem("Normal", SPEED_NORMAL));
	gUIListItems.push_back(new UiListItem("Fast", SPEED_FAST));
	gUIListItems.push_back(new UiListItem("Faster", SPEED_FASTER));
	gUIListItems.push_back(new UiListItem("Fastest", SPEED_FASTEST));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + (SELGAME_RPANEL_WIDTH - 240) / 2, SELGAME_LIST_TOP, 240, 26 * 4 };
	gUiItems.push_back(new UiList(&gUIListItems, 4, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(gUIListItems.size() == 4);
	UiInitScreen(4, SelgameSpeedFocus, SelgameSpeedSelect, SelgameDiffInit);
}

static void SelgamePasswordEsc()
{
	if (selgame_mode == SELGAME_JOIN)
		SelgameAddressInit();
	else
		SelgameSpeedInit();
}

static void SelgameDiffEsc()
{
#ifndef NOHOSTING
	if (provider == SELCONN_TCPS || provider == SELCONN_TCPDS) {
		SelgameModeEsc();
		return;
	}
#endif
	if (provider == SELCONN_LOOPBACK && !selhero_heroInfo.hiSaveFile) {
		SelgameModeEsc();
		return;
	}

	SelgameModeInit();
}

static void SelgameDiffSelect(unsigned index)
{
	selgame_gameData->aeDifficulty = gUIListItems[index]->m_value;

	if (!selconn_bMulti) {
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}

	SelgameSpeedInit();
}

static void SelgameSpeedSelect(unsigned index)
{
	selgame_gameData->aeTickRate = gUIListItems[index]->m_value;
	if (provider == SELCONN_LOOPBACK) {
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}
	if (ztProvider) {
		SelgamePasswordInit(0);
		return;
	}
	SelgamePortInit(0);
}

static void SelgamePasswordSelect(unsigned index)
{
	if (ztBlOckBtn != NULL)
		return;

	char dialogText[256];
	int port = 0;

	if (selgame_mode != SELGAME_JOIN) {
		// assert(selgame_mode == SELGAME_CREATE || selgame_mode == SELGAME_LOAD);
		selgame_gameData->aeNetUpdateRate = 1;
		selgame_gameData->aeMaxPlayers = !selconn_bMulti ? 1 : MAX_PLRS;
		if (!selconn_bMulti) {
			selgame_gameData->aeTickRate = gnTicksRate;
#ifndef ADAPTIVE_NETUPDATE
		} else {
			int latency = 80;
			getIniInt("Network", "Latency", &latency);
			selgame_gameData->aeNetUpdateRate = std::max(2, latency / (1000 / selgame_gameData->aeTickRate));
#endif
		}
		if (!ztProvider) {
			setIniValue("Network", "Port", selgame_GamePort);
			getIniInt("Network", "Port", &port);
		}
		if (SNetCreateGame(port, selgame_Password, selgame_gameData, dialogText)) {
			selgame_endMenu = true;
			return;
		}
	} else {
		// assert(selgame_mode == SELGAME_JOIN);
		if (selgame_conidx == selgame_connum) {
			selgame_connum++; // ensure SelgamePasswordEsc choose the right path in case SNetJoinGame fails. The clearest solution would be a whole selgame_coninfos-reload, but it's not worth it...
		}
		if (!ztProvider) {
			snprintf(tempstr, sizeof(tempstr), "Entry%d", selgame_conidx);
			setIniValue("Phone Book", tempstr, selgame_GameName);
			snprintf(tempstr, sizeof(tempstr), "Entry%dPort", selgame_conidx);
			setIniValue("Phone Book", tempstr, selgame_GamePort);
			getIniInt("Phone Book", tempstr, &port);
		}
		if (SNetJoinGame(selgame_GameName, port, selgame_Password, dialogText)) {
			selgame_endMenu = true;
			return;
		}
	}
	// assert(provider != SELCONN_LOOPBACK);
	SelgameFree();
	UiSelOkDialog(selgame_mode == SELGAME_CREATE ? "Create Game" : "Join Game", dialogText);
	SelgameInit();
	SelgamePasswordInit(0);
}

int UiSelectGame(_uigamedata* game_data)
{
	selgame_gameData = game_data;

	selgame_add_event_handlers();
#ifdef ZEROTIER
	ztProvider = provider == SELCONN_ZT;
#endif

	SelgameInit();
	SelgameModeInit();

	selgame_endMenu = false;
	do {
#ifdef ZEROTIER
		if (ztBlOckBtn != NULL && SNetReady()) {
			ztBlOckBtn->m_iFlags &= ~(UIS_SILVER | UIS_DISABLED);
			ztBlOckBtn->m_iFlags |= UIS_GOLD;
			ztBlOckBtn = NULL;
		}
		if (ztRefreshBtn != NULL && ztBlOckBtn == NULL && SDL_TICKS_PASSED(SDL_GetTicks(), ztNextRefresh)) {
			ztRefreshBtn->m_iFlags &= ~(UIS_SILVER | UIS_DISABLED);
			ztRefreshBtn->m_iFlags |= UIS_GOLD;
			ztRefreshBtn = NULL;
		}
#endif
		UiRenderAndPoll();
	} while (!selgame_endMenu);
	SelgameFree();

	return selgame_mode;
}

void UIDisconnectGame()
{
	selgame_remove_event_handlers();
	SNetLeaveGame();
}

ENABLE_SPEED_OPTIMIZATION

DEVILUTION_END_NAMESPACE
