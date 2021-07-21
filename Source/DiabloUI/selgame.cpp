#include "selgame.h"

#include <stdexcept>
#include "config.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/selconn.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/text.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

extern int provider;

namespace {

char selgame_Label[32];
char selgame_Ip[128] = "";
char selgame_Port[8] = "";
char selgame_Password[16] = "";
char selgame_Description[256];
int selgame_selectedGame;
bool selgame_endMenu;
//int selgame_heroLevel;

_SNETGAMEDATA *selgame_gameData;

#define DESCRIPTION_WIDTH 205

std::vector<UiListItem *> vecSelGameDlgItems;
std::vector<UiItemBase *> vecSelGameDialog;

} // namespace

static void selgame_handleEvents(_SNETEVENT *pEvt)
{
	_SNETGAMEDATA *gameData;
	DWORD playerId;

	assert(pEvt->eventid == EVENT_TYPE_JOIN_ACCEPTED);

	playerId = pEvt->playerid;
	assert((DWORD)playerId < MAX_PLRS);

	gameData = (_SNETGAMEDATA *)pEvt->_eData;
	if (gameData->dwVersionId != GAME_VERSION)
		throw std::runtime_error("Mismatching game versions.");
	assert(pEvt->databytes == sizeof(_SNETGAMEDATA));
	copy_pod(*selgame_gameData, *gameData);
	selgame_gameData->bPlayerId = playerId;
}

static void selgame_add_event_handlers(void (*event_handler)(_SNETEVENT *pEvt))
{
	SNetRegisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME, event_handler);
	SNetRegisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED, selgame_handleEvents);
}

static void selgame_remove_event_handlers()
{
	SNetUnregisterEventHandler(EVENT_TYPE_PLAYER_LEAVE_GAME);
	SNetUnregisterEventHandler(EVENT_TYPE_JOIN_ACCEPTED);
}

static void selgame_FreeVectors()
{
	for (unsigned i = 0; i < vecSelGameDlgItems.size(); i++) {
		UiListItem *pUIItem = vecSelGameDlgItems[i];
		delete pUIItem;
	}
	vecSelGameDlgItems.clear();

	for (unsigned i = 0; i < vecSelGameDialog.size(); i++) {
		UiItemBase *pUIItem = vecSelGameDialog[i];
		delete pUIItem;
	}
	vecSelGameDialog.clear();
}

static void selgame_Free()
{
	ArtBackground.Unload();

	selgame_FreeVectors();
}

void selgame_GameSelection_Init()
{
	if (provider == SELCONN_LOOPBACK) {
		selgame_GameSelection_Select(SELGAME_CREATE);
		return;
	}

	getIniValue("Phone Book", "Entry1", selgame_Ip, sizeof(selgame_Ip) - 1);
	int port = NET_DEFAULT_PORT;
	getIniInt("Network", "Port", &port);
	snprintf(selgame_Port, sizeof(selgame_Port), "%d", port);
	getIniValue("Phone Book", "Entry1Port", selgame_Port, sizeof(selgame_Port) - 1);

	selgame_FreeVectors();

	UiAddBackground(&vecSelGameDialog);
	UiAddLogo(&vecSelGameDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelGameDialog.push_back(new UiArtText("Multi Player Game", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 211), 205, 192 };
	vecSelGameDialog.push_back(new UiArtText("Description:", rect2, UIS_MED));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Description, rect3));

	SDL_Rect rect4 = { PANEL_LEFT + 300, (UI_OFFSET_Y + 211), 295, 33 };
	vecSelGameDialog.push_back(new UiArtText("Select Action", rect4, UIS_CENTER | UIS_BIG));

	vecSelGameDlgItems.push_back(new UiListItem("Create Game", SELGAME_CREATE));
	vecSelGameDlgItems.push_back(new UiListItem("Join Game", SELGAME_JOIN));

	vecSelGameDialog.push_back(new UiList(vecSelGameDlgItems, PANEL_LEFT + 305, (UI_OFFSET_Y + 255), 285, 26, UIS_CENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect5 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect5, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect6 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("CANCEL", &UiFocusNavigationEsc, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelGameDialog, vecSelGameDlgItems.size(), selgame_GameSelection_Focus, selgame_GameSelection_Select, selgame_GameSelection_Esc);
}

void selgame_GameSelection_Focus(unsigned index)
{
	switch (vecSelGameDlgItems[index]->m_value) {
	case SELGAME_CREATE:
		snprintf(selgame_Description, sizeof(selgame_Description), "Create a new game with a difficulty setting of your choice.");
		break;
	case SELGAME_JOIN:
		snprintf(selgame_Description, sizeof(selgame_Description), "Enter an IP or a hostname and join a game already in progress.");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH);
}

/**
 * @brief Load the current hero level from save file
 * @param pInfo Hero info
 * @return always true
 */
/*bool UpdateHeroLevel(_uiheroinfo *pInfo)
{
	if (strcasecmp(pInfo->name, gszHero) == 0)
		selgame_heroLevel = pInfo->level;

	return true;
}*/

static void selgame_Port_Init(unsigned index)
{
	selgame_FreeVectors();

	UiAddBackground(&vecSelGameDialog);
	UiAddLogo(&vecSelGameDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelGameDialog.push_back(new UiArtText("Join Game", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 211), 205, 192 };
	vecSelGameDialog.push_back(new UiArtText("Description:", rect2, UIS_MED));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Description, rect3));

	SDL_Rect rect4 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 211), 285, 33 };
	vecSelGameDialog.push_back(new UiArtText("Enter Port", rect4, UIS_CENTER | UIS_BIG));

	SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 314), 285, 33 };
	vecSelGameDialog.push_back(new UiEdit("Enter Port", selgame_Port, sizeof(selgame_Port) - 1, rect5, UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelGameDialog, 0, NULL, selgame_Password_Init, selgame_Password_Esc);
}

void selgame_GameSelection_Select(unsigned index)
{
	assert(index == vecSelGameDlgItems[index]->m_value);

	selgame_selectedGame = index;

	//gfnHeroInfo(UpdateHeroLevel);

	selgame_FreeVectors();

	UiAddBackground(&vecSelGameDialog);
	UiAddLogo(&vecSelGameDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelGameDialog.push_back(new UiArtText(index == SELGAME_CREATE ? "Create Game" : "Join Game", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 34, (UI_OFFSET_Y + 211), 205, 33 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Label, rect2, UIS_CENTER | UIS_BIG));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Description, rect3));

	switch (index) {
	case SELGAME_CREATE: {
		SDL_Rect rect4 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 211), 295, 35 };
		vecSelGameDialog.push_back(new UiArtText("Select Difficulty", rect4, UIS_CENTER | UIS_BIG));

		vecSelGameDlgItems.push_back(new UiListItem("Normal", DIFF_NORMAL));
		vecSelGameDlgItems.push_back(new UiListItem("Nightmare", DIFF_NIGHTMARE));
		vecSelGameDlgItems.push_back(new UiListItem("Hell", DIFF_HELL));

		vecSelGameDialog.push_back(new UiList(vecSelGameDlgItems, PANEL_LEFT + 300, (UI_OFFSET_Y + 282), 295, 26, UIS_CENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect5 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelGameDialog.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect5, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect6 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelGameDialog.push_back(new UiArtTextButton("CANCEL", &UiFocusNavigationEsc, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		UiInitList(vecSelGameDialog, vecSelGameDlgItems.size(), selgame_Diff_Focus, selgame_Diff_Select, selgame_Diff_Esc, NULL, true);
	} break;
	case SELGAME_JOIN: {
		SDL_Rect rect4 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 211), 285, 33 };
		vecSelGameDialog.push_back(new UiArtText("Enter Address", rect4, UIS_CENTER | UIS_BIG));

		SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 314), 285, 33 };
		vecSelGameDialog.push_back(new UiEdit("Enter Address", selgame_Ip, sizeof(selgame_Ip) - 1, rect5, UIS_MED | UIS_GOLD));

		SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelGameDialog.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelGameDialog.push_back(new UiArtTextButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		UiInitList(vecSelGameDialog, 0, NULL, selgame_Port_Init, selgame_GameSelection_Init);
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void selgame_GameSelection_Esc()
{
	UiInitList_clear();
	selgame_selectedGame = SELGAME_PREVIOUS;
	selgame_endMenu = true;
}

void selgame_Diff_Focus(unsigned index)
{
	switch (vecSelGameDlgItems[index]->m_value) {
	case DIFF_NORMAL:
		snprintf(selgame_Label, sizeof(selgame_Label), "Normal");
		snprintf(selgame_Description, sizeof(selgame_Description), "Normal Difficulty\nThis is where a starting character should begin the quest to defeat Diablo.");
		break;
	case DIFF_NIGHTMARE:
		snprintf(selgame_Label, sizeof(selgame_Label), "Nightmare");
		snprintf(selgame_Description, sizeof(selgame_Description), "Nightmare Difficulty\nThe denizens of the Labyrinth have been bolstered and will prove to be a greater challenge. This is recommended for experienced characters only.");
		break;
	case DIFF_HELL:
		snprintf(selgame_Label, sizeof(selgame_Label), "Hell");
		snprintf(selgame_Description, sizeof(selgame_Description), "Hell Difficulty\nThe most powerful of the underworld's creatures lurk at the gateway into Hell. Only the most experienced characters should venture in this realm.");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH);
}

static void ShowErrorMsgDialog()
{
	selgame_Free();
	UiSelOkDialog(selgame_selectedGame == SELGAME_CREATE ? "Create Game" : "Join Game", tempstr, false);
	LoadBackgroundArt("ui_art\\selgame.pcx");
}

void selgame_Diff_Select(unsigned index)
{
	int value = vecSelGameDlgItems[index]->m_value;

	selgame_gameData->bDifficulty = value;

	if (!selconn_bMulti) {
		selgame_gameData->bMaxPlayers = 1;
		selgame_gameData->bTickRate = gnTicksRate;
		selgame_Password[0] = '\0';
		selgame_Password_Select(0);
		return;
	}
	selgame_gameData->bMaxPlayers = MAX_PLRS;

	selgame_GameSpeedSelection();
}

void selgame_Diff_Esc()
{
	if (provider == SELCONN_LOOPBACK) {
		selgame_GameSelection_Esc();
		return;
	}

	selgame_GameSelection_Init();
}

void selgame_GameSpeedSelection()
{
	// gfnHeroInfo(UpdateHeroLevel);

	selgame_FreeVectors();

	UiAddBackground(&vecSelGameDialog);
	UiAddLogo(&vecSelGameDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelGameDialog.push_back(new UiArtText("Create Game", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 34, (UI_OFFSET_Y + 211), 205, 33 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Label, rect2, UIS_CENTER | UIS_BIG));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Description, rect3));

	SDL_Rect rect4 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 211), 295, 35 };
	vecSelGameDialog.push_back(new UiArtText("Select Game Speed", rect4, UIS_CENTER | UIS_BIG));

	vecSelGameDlgItems.push_back(new UiListItem("Normal", SPEED_NORMAL));
	vecSelGameDlgItems.push_back(new UiListItem("Fast", SPEED_FAST));
	vecSelGameDlgItems.push_back(new UiListItem("Faster", SPEED_FASTER));
	vecSelGameDlgItems.push_back(new UiListItem("Fastest", SPEED_FASTEST));

	vecSelGameDialog.push_back(new UiList(vecSelGameDlgItems, PANEL_LEFT + 300, (UI_OFFSET_Y + 279), 295, 26, UIS_CENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect5 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect5, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect6 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("CANCEL", &UiFocusNavigationEsc, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelGameDialog, vecSelGameDlgItems.size(), selgame_Speed_Focus, selgame_Speed_Select, selgame_Speed_Esc, NULL, true);
}

void selgame_Speed_Focus(unsigned index)
{
	switch (vecSelGameDlgItems[index]->m_value) {
	case SPEED_NORMAL:
		snprintf(selgame_Label, sizeof(selgame_Label), "Normal");
		snprintf(selgame_Description, sizeof(selgame_Description), "Normal Speed\nThis is where a starting character should begin the quest to defeat Diablo.");
		break;
	case SPEED_FAST:
		snprintf(selgame_Label, sizeof(selgame_Label), "Fast");
		snprintf(selgame_Description, sizeof(selgame_Description), "Fast Speed\nThe denizens of the Labyrinth have been hastened and will prove to be a greater challenge. This is recommended for experienced characters only.");
		break;
	case SPEED_FASTER:
		snprintf(selgame_Label, sizeof(selgame_Label), "Faster");
		snprintf(selgame_Description, sizeof(selgame_Description), "Faster Speed\nMost monsters of the dungeon will seek you out quicker than ever before. Only an experienced champion should try their luck at this speed.");
		break;
	case SPEED_FASTEST:
		snprintf(selgame_Label, sizeof(selgame_Label), "Fastest");
		snprintf(selgame_Description, sizeof(selgame_Description), "Fastest Speed\nThe minions of the underworld will rush to attack without hesitation. Only a true speed demon should enter at this pace.");
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	WordWrapArtStr(selgame_Description, DESCRIPTION_WIDTH);
}

void selgame_Speed_Esc()
{
	selgame_GameSelection_Select(SELGAME_CREATE);
}

void selgame_Speed_Select(unsigned index)
{
	selgame_gameData->bTickRate = vecSelGameDlgItems[index]->m_value;

	if (provider == SELCONN_LOOPBACK) {
		selgame_Password[0] = '\0';
		selgame_Password_Select(0);
		return;
	}

	selgame_Password_Init(0);
}

void selgame_Password_Init(unsigned index)
{
	memset(&selgame_Password, 0, sizeof(selgame_Password));

	selgame_FreeVectors();

	UiAddBackground(&vecSelGameDialog);
	UiAddLogo(&vecSelGameDialog);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelGameDialog.push_back(new UiArtText(selgame_selectedGame == SELGAME_CREATE ? "Create Game" : "Join Game", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 211), 205, 192 };
	vecSelGameDialog.push_back(new UiArtText("Description:", rect2, UIS_MED));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 192 };
	vecSelGameDialog.push_back(new UiArtText(selgame_Description, rect3));

	SDL_Rect rect4 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 211), 285, 33 };
	vecSelGameDialog.push_back(new UiArtText("Enter Password", rect4, UIS_CENTER | UIS_BIG));

	SDL_Rect rect5 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 314), 285, 33 };
	vecSelGameDialog.push_back(new UiEdit("Enter Password", selgame_Password, sizeof(selgame_Password) - 1, rect5, UIS_MED | UIS_GOLD));

	SDL_Rect rect6 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect6, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect7 = { PANEL_LEFT + 449, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelGameDialog.push_back(new UiArtTextButton("CANCEL", &UiFocusNavigationEsc, rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelGameDialog, 0, NULL, selgame_Password_Select, selgame_Password_Esc);
}

void selgame_Password_Select(unsigned index)
{
	if (selgame_selectedGame == SELGAME_CREATE) {
		if (SNetCreateGame(selgame_Password, selgame_gameData)) {
			UiInitList_clear();
			selgame_endMenu = true;
			return;
		}
	} else {
		assert(selgame_selectedGame == SELGAME_JOIN);
		setIniValue("Phone Book", "Entry1", selgame_Ip);
		setIniValue("Phone Book", "Entry1Port", selgame_Port);
		int port;
		getIniInt("Phone Book", "Entry1Port", &port);
		if (SNetJoinGame(selgame_Ip, port, selgame_Password)) {
			UiInitList_clear();
			selgame_endMenu = true;
			return;
		}
	}

	SStrCopy(tempstr, SDL_GetError(), sizeof(tempstr));
	ShowErrorMsgDialog();
	selgame_Password_Init(0);
}

void selgame_Password_Esc()
{
	if (selgame_selectedGame == SELGAME_JOIN)
		selgame_GameSelection_Select(SELGAME_JOIN);
	else
		selgame_GameSpeedSelection();
}

int UiSelectGame(_SNETGAMEDATA *game_data, void (*event_handler)(_SNETEVENT *pEvt))
{
	selgame_gameData = game_data;

	selgame_add_event_handlers(event_handler);

	LoadBackgroundArt("ui_art\\selgame.pcx");
	selgame_GameSelection_Init();

	selgame_endMenu = false;
	while (!selgame_endMenu) {
		UiClearScreen();
		UiPollAndRender();
	}
	selgame_Free();

	return selgame_selectedGame;
}

void UIDisconnectGame()
{
	selgame_remove_event_handlers();
	SNetLeaveGame(LEAVE_UNKNOWN);
	if (gbMaxPlayers != 1 && provider != SELCONN_LOOPBACK)
		SDL_Delay(2000);
}

DEVILUTION_END_NAMESPACE
