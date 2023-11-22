
#include "DiabloUI/diablo.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/scrollbar.h"
#include "DiabloUI/selconn.h"
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

#define MAX_VIEWPORT_ITEMS ((unsigned)((SELGAME_RPANEL_HEIGHT - 22) / 26))

extern int provider;

typedef struct ConnectionInfo {
	const char *ci_GameName;
	const char *ci_GamePort;
} ConnectionInfo;

static std::vector<ConnectionInfo> selgame_coninfos;
static unsigned selgame_connum;
static unsigned selgame_conidx;

static UiTxtButton* SELLIST_DIALOG_DELETE_BUTTON;

static char selgame_Label[32];
static char selgame_GameName[NET_MAX_GAMENAME_LEN + 1] = "";
static char selgame_GamePort[8] = "";
static char selgame_Password[NET_MAX_PASSWD_LEN + 1] = "";
static char selgame_Description[128];
static int selgame_mode;
static bool selgame_endMenu;
//int selgame_heroLevel;

static _uigamedata* selgame_gameData;

#define DESCRIPTION_WIDTH (SELGAME_LPANEL_WIDTH - 2 * 10)

// Forward-declare UI-handlers, used by other handlers.
static void SelgameModeSelect(unsigned index);
static void SelgameDiffSelect(unsigned index);
static void SelgameSpeedSelect(unsigned index);
static void SelgamePasswordSelect(unsigned index);
static void SelgameDiffEsc();
static void SelgamePasswordEsc();

static void selgame_handleEvents(SNetEventHdr* pEvt)
{
	SNetGameData* gameData;
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

	selgame_gameData->aePlayerId = playerId;
	selgame_gameData->aeTurn = turn;
}

static void selgame_add_event_handlers()
{
	SNetRegisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME, multi_ui_handle_events);
	SNetRegisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED, selgame_handleEvents);
}

static void selgame_remove_event_handlers()
{
	SNetUnregisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME);
	SNetUnregisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED);
}

static void SelgameInit()
{
	LoadScrollBar();
	gbHerosCel = CelLoadImage("ui_art\\heros.CEL", SELHERO_HEROS_WIDTH);
	LoadBackgroundArt("ui_art\\selgame.CEL", "ui_art\\menu.pal");
}

static void SelgameFreeDlgItems()
{
	UiClearListItems();

	UiClearItems();
}

static void SelgameFree()
{
	FreeBackgroundArt();
	MemFreeDbg(gbHerosCel);
	UnloadScrollBar();
	SelgameFreeDlgItems();

	// memset(&selgame_Password, 0, sizeof(selgame_Password)); - pointless because the plain password is stored in storm anyway...
}

static void SelgameResetScreen(const char* title, const char* rheader)
{
	SelgameFreeDlgItems();

	UiAddBackground();
	UiAddLogo();

	SDL_Rect rect1 = { PANEL_LEFT + 0, SELGAME_TITLE_TOP, PANEL_WIDTH, 35 };
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
	// assert(mypnum == 0 || hosted);
	gUiItems.push_back(new UiImage(gbHerosCel, hosted ? 0 : plx(0)._pClass + 1, rect5, false));
	if (!hosted) {
		SDL_Rect rect6 = { SELGAME_LPANEL_LEFT + 10, SELGAME_LPANEL_BOTTOM - 30, DESCRIPTION_WIDTH, 30 };
		gUiItems.push_back(new UiText(plx(0)._pName, rect6, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));
	}
}

static void SelgameModeEsc()
{
	selgame_mode = SELGAME_PREVIOUS;
	selgame_endMenu = true;
}

static void SelgameModeFocus(unsigned index)
{
	selgame_Label[0] = '\0';
	switch (gUIListItems[index]->m_value) {
	case SELGAME_CREATE:
		copy_cstr(selgame_Description, "Create a new game with a difficulty setting of your choice.");
		break;
	case SELGAME_JOIN:
		copy_cstr(selgame_Description, "Enter an IP or a hostname and join a game already in progress.");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH, AFT_SMALL);
}

static void SelgameDiffFocus(unsigned index)
{
	switch (gUIListItems[index]->m_value) {
	case DIFF_NORMAL:
		copy_cstr(selgame_Label, "Normal");
		copy_cstr(selgame_Description, "Normal Difficulty\nThis is where a starting character should begin the quest to defeat Diablo.");
		break;
	case DIFF_NIGHTMARE:
		copy_cstr(selgame_Label, "Nightmare");
		copy_cstr(selgame_Description, "Nightmare Difficulty\nThe denizens of the Labyrinth have been bolstered and will prove to be a greater challenge.");
		break;
	case DIFF_HELL:
		copy_cstr(selgame_Label, "Hell");
		copy_cstr(selgame_Description, "Hell Difficulty\nThe most powerful of the underworld's creatures lurk at the gateway into Hell.");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH, AFT_SMALL);
}

static void SelgameSpeedFocus(unsigned index)
{
	switch (gUIListItems[index]->m_value) {
	case SPEED_NORMAL:
		copy_cstr(selgame_Label, "Normal");
		copy_cstr(selgame_Description, "Normal Speed\nThis is where a starting character should begin the quest to defeat Diablo.");
		break;
	case SPEED_FAST:
		copy_cstr(selgame_Label, "Fast");
		copy_cstr(selgame_Description, "Fast Speed\nThe denizens of the Labyrinth have been hastened and will prove to be a greater challenge.");
		break;
	case SPEED_FASTER:
		copy_cstr(selgame_Label, "Faster");
		copy_cstr(selgame_Description, "Faster Speed\nMost monsters of the dungeon will seek you out quicker than ever before.");
		break;
	case SPEED_FASTEST:
		copy_cstr(selgame_Label, "Fastest");
		copy_cstr(selgame_Description, "Fastest Speed\nThe minions of the underworld will rush to attack without hesitation.");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH, AFT_SMALL);
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

static void SelgameModeInit()
{
#ifndef NOHOSTING
	if (provider == SELCONN_LOOPBACK || provider == SELCONN_TCPS || provider == SELCONN_TCPDS) {
#else
	if (provider == SELCONN_LOOPBACK) {
#endif
		SelgameModeSelect(SELGAME_CREATE);
		return;
	}

	SelgameResetScreen("Multi Player Game", "Select Action");

	static_assert(0 == (int)SELGAME_CREATE, "SelgameModeSelect expects the index and its value to match I.");
	static_assert(1 == (int)SELGAME_JOIN, "SelgameModeSelect expects the index and its value to match II.");
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
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 24, SELGAME_CONTENT_TOP + (SELGAME_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELGAME_RPANEL_WIDTH - 24 * 2, FOCUS_MEDIUM };
	UiEdit* edit = new UiEdit("Enter Password", selgame_Password, sizeof(selgame_Password), rect5);
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
	if (index != selgame_connum) {
		SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_GOLD;

		snprintf(selgame_Description, sizeof(selgame_Description), "Port: %s", selgame_coninfos[index].ci_GamePort);
	} else {
		SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_SILVER | UIS_DISABLED;
	}
}

static void SelgameAddressEsc()
{
	if (selgame_connum == 0) {
		SelgameModeInit();
	} else {
		SelgameModeSelect(SELGAME_JOIN);
	}
}

static void SelgameAddressInit()
{
	SelgameNoFocus();
	SelgameResetScreen("Join Game", "Enter Address");

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 2, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 24, SELGAME_CONTENT_TOP + (SELGAME_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELGAME_RPANEL_WIDTH - 24 * 2, FOCUS_MEDIUM };
	UiEdit* edit = new UiEdit("Enter Address", selgame_GameName, sizeof(selgame_GameName), rect5);
	gUiItems.push_back(edit);

	UiInitScreen(0, NULL, SelgamePortInit, SelgameAddressEsc);
	UiInitEdit(edit);
}

static void SelgameAddressListSelect(unsigned index)
{
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
	SelgameModeSelect(SELGAME_JOIN);
}

static void SelgameAddressListInit()
{
	SelgameResetScreen("Join Game", "Select Address");

	unsigned num_viewport_cons = std::min(selgame_connum + 1, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < num_viewport_cons; i++) {
		gUIListItems.push_back(new UiListItem("", -1));
	}
	SelgameUpdateViewportItems();

	SDL_Rect rect5 = { SELGAME_RPANEL_LEFT + 25, SELCONN_LIST_TOP, SELGAME_RPANEL_WIDTH - 2 * 25, 26 * (int)num_viewport_cons };
	gUiItems.push_back(new UiList(&gUIListItems, num_viewport_cons, rect5, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH - SCROLLBAR_BG_WIDTH + 1, SELGAME_CONTENT_TOP - 1, SCROLLBAR_BG_WIDTH, SELGAME_RPANEL_HEIGHT + 1 };
	UiScrollBar* scrollBar = new UiScrollBar(rect6);
	gUiItems.push_back(scrollBar);

	SDL_Rect rect7 = { SELGAME_RPANEL_LEFT, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect7, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect8 = { SELGAME_RPANEL_LEFT + SELGAME_RPANEL_WIDTH / 3, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 3, 35 };
	SELLIST_DIALOG_DELETE_BUTTON = new UiTxtButton("Delete", &UiFocusNavigationDelete, rect8, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER | UIS_DISABLED);
	gUiItems.push_back(SELLIST_DIALOG_DELETE_BUTTON);

	SDL_Rect rect9 = { SELGAME_RPANEL_LEFT + 2 * SELGAME_RPANEL_WIDTH / 3, SELGAME_RBUTTON_TOP, SELGAME_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect9, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitScreen(selgame_connum + 1, SelgameAddressListFocus, SelgameAddressListSelect, SelgameModeInit);
	UiInitScrollBar(scrollBar, MAX_VIEWPORT_ITEMS, SelgameAddressListDelete);
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
	if (provider == SELCONN_LOOPBACK || provider == SELCONN_TCPS || provider == SELCONN_TCPDS) {
#else
	if (provider == SELCONN_LOOPBACK) {
#endif
		SelgameModeEsc();
		return;
	}

	SelgameModeInit();
}

static void SelgameDiffSelect(unsigned index)
{
	int value = gUIListItems[index]->m_value;

	selgame_gameData->aeDifficulty = value;

	if (!selconn_bMulti) {
		selgame_gameData->aeMaxPlayers = 1;
		selgame_gameData->aeTickRate = gnTicksRate;
		selgame_gameData->aeNetUpdateRate = 1;
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}
	selgame_gameData->aeMaxPlayers = MAX_PLRS;

	SelgameSpeedInit();
}

static void SelgameModeSelect(unsigned index)
{
	selgame_mode = index;

	//gfnHeroInfo(UpdateHeroLevel);
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	snprintf(selgame_GamePort, sizeof(selgame_GamePort), "%d", port);
	if (index == SELGAME_CREATE) {
		SelgameDiffInit();
		return;
	}

	int numEntries = 0;
	selgame_coninfos.clear();
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
	selgame_connum = numEntries;

	if (numEntries != 0) {
		ConnectionInfo ci = { "New Address", "" };
		selgame_coninfos.push_back(ci);

		SelgameAddressListInit();
	} else {
		SelgameAddressInit();
	}
}

static void SelgameSpeedSelect(unsigned index)
{
	selgame_gameData->aeTickRate = gUIListItems[index]->m_value;
	selgame_gameData->aeNetUpdateRate = 1;
	if (provider == SELCONN_LOOPBACK) {
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}
#ifndef ADAPTIVE_NETUPDATE
	int latency = 80;
	getIniInt("Network", "Latency", &latency);
	selgame_gameData->aeNetUpdateRate = std::max(2, latency / (1000 / selgame_gameData->aeTickRate));
#endif

	SelgamePortInit(0);
}

static void SelgamePasswordSelect(unsigned index)
{
	char dialogText[256];

	if (selgame_mode == SELGAME_CREATE) {
		setIniValue("Network", "Port", selgame_GamePort);
		int port;
		getIniInt("Network", "Port", &port);
		if (SNetCreateGame(port, selgame_Password, selgame_gameData, dialogText)) {
			selgame_endMenu = true;
			return;
		}
	} else {
		// assert(selgame_mode == SELGAME_JOIN);
		if (selgame_conidx == selgame_connum) {
			selgame_connum++; // ensure SelgamePasswordEsc choose the right path in case SNetJoinGame fails. The clearest solution would be a whole selgame_coninfos-reload, but it's not worth it...
		}
		snprintf(tempstr, sizeof(tempstr), "Entry%d", selgame_conidx);
		setIniValue("Phone Book", tempstr, selgame_GameName);
		snprintf(tempstr, sizeof(tempstr), "Entry%dPort", selgame_conidx);
		setIniValue("Phone Book", tempstr, selgame_GamePort);
		int port;
		getIniInt("Phone Book", tempstr, &port);
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

	SelgameInit();
	SelgameModeInit();

	selgame_endMenu = false;
	do {
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

DEVILUTION_END_NAMESPACE
