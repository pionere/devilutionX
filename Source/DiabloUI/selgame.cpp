
#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/selconn.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/text.h"
#include "storm/storm_net.h"
#include "all.h"
//#include "../diablo.h"
//#include <algorithm>
//#include "../engine.h"
//#include "storm/storm.h"

DEVILUTION_BEGIN_NAMESPACE

extern int provider;

static char selgame_Label[32];
static char selgame_Ip[128] = "";
static char selgame_Port[8] = "";
static char selgame_Password[16] = "";
static char selgame_Description[128];
static int selgame_mode;
static bool selgame_endMenu;
//int selgame_heroLevel;

static SNetGameData* selgame_gameData;

#define DESCRIPTION_OFFSET 35
#define DESCRIPTION_WIDTH  208

// Forward-declare UI-handlers, used by other handlers.
static void SelgameModeSelect(unsigned index);
static void SelgameSpeedSelect(unsigned index);
static void SelgamePasswordSelect(unsigned index);

static void selgame_handleEvents(SNetEvent* pEvt)
{
	SNetGameData* gameData;
	unsigned playerId;

	assert(pEvt->eventid == EVENT_TYPE_JOIN_ACCEPTED);
	assert(pEvt->databytes == sizeof(SNetGameData));
	gameData = (SNetGameData*)pEvt->_eData;
	assert(gameData->dwVersionId == GAME_VERSION);

	playerId = pEvt->playerid;
	assert((DWORD)playerId < MAX_PLRS);

	copy_pod(*selgame_gameData, *gameData);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	selgame_gameData->dwSeed = SwapLE32(selgame_gameData->dwSeed);
	selgame_gameData->dwVersionId = SwapLE32(selgame_gameData->dwVersionId);
#endif
	selgame_gameData->bPlayerId = playerId;
}

static void selgame_add_event_handlers(void (*event_handler)(SNetEvent *pEvt))
{
	SNetRegisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME, event_handler);
	SNetRegisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED, selgame_handleEvents);
}

static void selgame_remove_event_handlers()
{
	SNetUnregisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME);
	SNetUnregisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED);
}

static void SelgameFreeDlgItems()
{
	UiClearListItems();

	UiClearItems(gUiItems);
}

static void SelgameFree()
{
	MemFreeDbg(gbBackCel);
	SelgameFreeDlgItems();

	//UiInitList_clear();
}

static void SelgameModeEsc()
{
	selgame_mode = SELGAME_PREVIOUS;
	selgame_endMenu = true;
}

static void SelgameModeFocus(unsigned index)
{
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

static void SelgameSpeedEsc()
{
	SelgameModeSelect(SELGAME_CREATE);
}

static void SelgameSpeedInit()
{
	// gfnHeroInfo(UpdateHeroLevel);

	SelgameFreeDlgItems();

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	gUiItems.push_back(new UiText("Create Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 211), DESCRIPTION_WIDTH, 34 };
	gUiItems.push_back(new UiText(selgame_Label, rect2, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect3 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	gUiItems.push_back(new UiText(selgame_Description, rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { PANEL_LEFT + 280, (UI_OFFSET_Y + 211), 334, 34 };
	gUiItems.push_back(new UiText("Select Game Speed", rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	gUIListItems.push_back(new UiListItem("Normal", SPEED_NORMAL));
	gUIListItems.push_back(new UiListItem("Fast", SPEED_FAST));
	gUIListItems.push_back(new UiListItem("Faster", SPEED_FASTER));
	gUIListItems.push_back(new UiListItem("Fastest", SPEED_FASTEST));

	SDL_Rect rect5 = { PANEL_LEFT + 300, (UI_OFFSET_Y + 279), 295, 26 * 4 };
	gUiItems.push_back(new UiList(&gUIListItems, 4, rect5, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(gUIListItems.size() == 4);
	UiInitList(4, SelgameSpeedFocus, SelgameSpeedSelect, SelgameSpeedEsc);
}

static void SelgamePasswordEsc()
{
	if (selgame_mode == SELGAME_JOIN)
		SelgameModeSelect(SELGAME_JOIN);
	else
		SelgameSpeedInit();
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

	getIniValue("Phone Book", "Entry1", selgame_Ip, sizeof(selgame_Ip) - 1);
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	snprintf(selgame_Port, sizeof(selgame_Port), "%d", port);
	getIniValue("Phone Book", "Entry1Port", selgame_Port, sizeof(selgame_Port) - 1);

	SelgameFreeDlgItems();

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	gUiItems.push_back(new UiText("Multi Player Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect3 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	gUiItems.push_back(new UiText(selgame_Description, rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { PANEL_LEFT + 280, (UI_OFFSET_Y + 211), 334, 34 };
	gUiItems.push_back(new UiText("Select Action", rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	static_assert(0 == (int)SELGAME_CREATE, "SelgameModeSelect expects the index and its value to match I.");
	static_assert(1 == (int)SELGAME_JOIN, "SelgameModeSelect expects the index and its value to match II.");
	gUIListItems.push_back(new UiListItem("Create Game", SELGAME_CREATE));
	gUIListItems.push_back(new UiListItem("Join Game", SELGAME_JOIN));

	SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 255), 285, 26 * 2 };
	gUiItems.push_back(new UiList(&gUIListItems, 2, rect5, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(gUIListItems.size() == 2);
	UiInitList(2, SelgameModeFocus, SelgameModeSelect, SelgameModeEsc);
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
	memset(&selgame_Password, 0, sizeof(selgame_Password));

	SelgameFreeDlgItems();

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	gUiItems.push_back(new UiText(selgame_mode == SELGAME_CREATE ? "Create Game" : "Join Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect3 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	gUiItems.push_back(new UiText(selgame_Description, rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { PANEL_LEFT + 280, (UI_OFFSET_Y + 211), 334, 34 };
	gUiItems.push_back(new UiText("Enter Password", rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 314), 285, 33 };
	gUiItems.push_back(new UiEdit("Enter Password", selgame_Password, sizeof(selgame_Password) - 1, rect5));

	SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(0, NULL, SelgamePasswordSelect, SelgamePasswordEsc);
}

static void SelgamePortInit(unsigned index)
{
	SelgameFreeDlgItems();

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	gUiItems.push_back(new UiText("Join Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect3 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	gUiItems.push_back(new UiText(selgame_Description, rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { PANEL_LEFT + 280, (UI_OFFSET_Y + 211), 334, 34 };
	gUiItems.push_back(new UiText("Enter Port", rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 314), 285, 33 };
	gUiItems.push_back(new UiEdit("Enter Port", selgame_Port, sizeof(selgame_Port) - 1, rect5));

	SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	gUiItems.push_back(new UiTxtButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(0, NULL, SelgamePasswordInit, SelgamePasswordEsc);
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

	selgame_gameData->bDifficulty = value;

	if (!selconn_bMulti) {
		selgame_gameData->bMaxPlayers = 1;
		selgame_gameData->bTickRate = gnTicksRate;
		selgame_gameData->bNetUpdateRate = 1;
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}
	selgame_gameData->bMaxPlayers = MAX_PLRS;

	SelgameSpeedInit();
}

static void SelgameModeSelect(unsigned index)
{
	selgame_mode = index;

	//gfnHeroInfo(UpdateHeroLevel);

	SelgameFreeDlgItems();

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	gUiItems.push_back(new UiText(index == SELGAME_CREATE ? "Create Game" : "Join Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect3 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	gUiItems.push_back(new UiText(selgame_Description, rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	switch (index) {
	case SELGAME_CREATE: {
		SDL_Rect rect2 = { PANEL_LEFT + DESCRIPTION_OFFSET, (UI_OFFSET_Y + 211), DESCRIPTION_WIDTH, 34 };
		gUiItems.push_back(new UiText(selgame_Label, rect2, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

		SDL_Rect rect4 = { PANEL_LEFT + 280, (UI_OFFSET_Y + 211), 334, 34 };
		gUiItems.push_back(new UiText("Select Difficulty", rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

		gUIListItems.push_back(new UiListItem("Normal", DIFF_NORMAL));
		gUIListItems.push_back(new UiListItem("Nightmare", DIFF_NIGHTMARE));
		gUIListItems.push_back(new UiListItem("Hell", DIFF_HELL));

		SDL_Rect rect5 = { PANEL_LEFT + 300, (UI_OFFSET_Y + 282), 295, 26 * 3 };
		gUiItems.push_back(new UiList(&gUIListItems, 3, rect5, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
		gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
		gUiItems.push_back(new UiTxtButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		//assert(gUIListItems.size() == 3);
		UiInitList(3, SelgameDiffFocus, SelgameDiffSelect, SelgameDiffEsc);
	} break;
	case SELGAME_JOIN: {
		SDL_Rect rect4 = { PANEL_LEFT + 280, (UI_OFFSET_Y + 211), 334, 34 };
		gUiItems.push_back(new UiText("Enter Address", rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

		SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 314), 285, 33 };
		gUiItems.push_back(new UiEdit("Enter Address", selgame_Ip, sizeof(selgame_Ip) - 1, rect5));

		SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
		gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
		gUiItems.push_back(new UiTxtButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		UiInitList(0, NULL, SelgamePortInit, SelgameModeInit);
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void SelgameSpeedSelect(unsigned index)
{
	selgame_gameData->bTickRate = gUIListItems[index]->m_value;
#ifdef ADAPTIVE_NETUPDATE
	selgame_gameData->bNetUpdateRate = 1;
#else
	int latency = 80;
	getIniInt("Network", "Latency", &latency);
	selgame_gameData->bNetUpdateRate = std::max(2, latency / (1000 / selgame_gameData->bTickRate));
#endif
	if (provider == SELCONN_LOOPBACK) {
		selgame_gameData->bNetUpdateRate = 1;
		selgame_Password[0] = '\0';
		SelgamePasswordSelect(0);
		return;
	}

	SelgamePasswordInit(0);
}

static void SelgamePasswordSelect(unsigned index)
{
	char dialogText[256];

	if (selgame_mode == SELGAME_CREATE) {
		if (SNetCreateGame(selgame_Password, selgame_gameData, dialogText)) {
			selgame_endMenu = true;
			return;
		}
	} else {
		assert(selgame_mode == SELGAME_JOIN);
		setIniValue("Phone Book", "Entry1", selgame_Ip);
		setIniValue("Phone Book", "Entry1Port", selgame_Port);
		int port;
		getIniInt("Phone Book", "Entry1Port", &port);
		if (SNetJoinGame(selgame_Ip, port, selgame_Password, dialogText)) {
			selgame_endMenu = true;
			return;
		}
	}

	SelgameFree();
	UiSelOkDialog(selgame_mode == SELGAME_CREATE ? "Create Game" : "Join Game", dialogText);
	LoadBackgroundArt("ui_art\\selgame.CEL", "ui_art\\menu.pal");
	SelgamePasswordInit(0);
}

int UiSelectGame(SNetGameData* game_data, void (*event_handler)(SNetEvent* pEvt))
{
	selgame_gameData = game_data;

	selgame_add_event_handlers(event_handler);

	LoadBackgroundArt("ui_art\\selgame.CEL", "ui_art\\menu.pal");
	SelgameModeInit();

	selgame_endMenu = false;
	do {
		UiClearScreen();
		UiPollAndRender();
	} while (!selgame_endMenu);
	SelgameFree();

	return selgame_mode;
}

void UIDisconnectGame(int reason)
{
	selgame_remove_event_handlers();
	SNetLeaveGame(reason);
}

DEVILUTION_END_NAMESPACE
