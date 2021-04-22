#include "selconn.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"

DEVILUTION_BEGIN_NAMESPACE

char selconn_MaxPlayers[21];
char selconn_Description[64];
char selconn_Gateway[129];
bool selconn_ReturnValue = false;
bool selconn_EndMenu = false;
_SNETPROGRAMDATA *selconn_ClientInfo;
_SNETPLAYERDATA *selconn_UserInfo;
_SNETUIDATA *selconn_UiInfo;
_SNETVERSIONDATA *selconn_FileInfo;

int provider;

std::vector<UiListItem *> vecConnItems;
std::vector<UiItemBase *> vecSelConnDlg;

#define DESCRIPTION_WIDTH 205

void SelconnLoad()
{
	LoadBackgroundArt("ui_art\\selconn.pcx");

#ifndef NONET
#ifdef ZEROTIER
	vecConnItems.push_back(new UiListItem("Zerotier", SELCONN_ZT));
#endif
	vecConnItems.push_back(new UiListItem("Client-Server (TCP)", SELCONN_TCP));
#endif
	vecConnItems.push_back(new UiListItem("Loopback", SELCONN_LOOPBACK));

	UiAddBackground(&vecSelConnDlg);
	UiAddLogo(&vecSelConnDlg);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelConnDlg.push_back(new UiArtText("Multi Player Game", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 218), DESCRIPTION_WIDTH, 21 };
	vecSelConnDlg.push_back(new UiArtText(selconn_MaxPlayers, rect2));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 21 };
	vecSelConnDlg.push_back(new UiArtText("Requirements:", rect3));

	SDL_Rect rect4 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 275), DESCRIPTION_WIDTH, 66 };
	vecSelConnDlg.push_back(new UiArtText(selconn_Description, rect4));

	SDL_Rect rect5 = { PANEL_LEFT + 30, (UI_OFFSET_Y + 356), 220, 31 };
	vecSelConnDlg.push_back(new UiArtText("no gateway needed", rect5, UIS_CENTER | UIS_MED));

	SDL_Rect rect6 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 393), DESCRIPTION_WIDTH, 21 };
	vecSelConnDlg.push_back(new UiArtText(selconn_Gateway, rect6, UIS_CENTER));

	SDL_Rect rect7 = { PANEL_LEFT + 300, (UI_OFFSET_Y + 211), 295, 33 };
	vecSelConnDlg.push_back(new UiArtText("Select Connection", rect7, UIS_CENTER | UIS_BIG));

	SDL_Rect rect8 = { PANEL_LEFT + 16, (UI_OFFSET_Y + 427), 250, 35 };
	vecSelConnDlg.push_back(new UiArtTextButton("Change Gateway", NULL, rect8, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD | UIS_HIDDEN));

	vecSelConnDlg.push_back(new UiList(vecConnItems, PANEL_LEFT + 305, (UI_OFFSET_Y + 256), 285, 26, UIS_CENTER | UIS_VCENTER | UIS_GOLD));

	SDL_Rect rect9 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelConnDlg.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect9, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect10 = { PANEL_LEFT + 454, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelConnDlg.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect10, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelConnDlg, vecConnItems.size(), SelconnFocus, SelconnSelect, SelconnEsc);
}

void SelconnFree()
{
	ArtBackground.Unload();

	for (auto pUIItem : vecConnItems) {
		delete pUIItem;
	}
	vecConnItems.clear();

	for (auto pUIMenuItem : vecSelConnDlg) {
		delete pUIMenuItem;
	}
	vecSelConnDlg.clear();
}

void SelconnEsc()
{
	selconn_ReturnValue = false;
	selconn_EndMenu = true;
}

void SelconnFocus(std::size_t index)
{
	int players = MAX_PLRS;
	switch (vecConnItems[index]->m_value) {
	case SELCONN_TCP:
		snprintf(selconn_Description, sizeof(selconn_Description), "All computers must be connected to a TCP-compatible network.");
		players = MAX_PLRS;
		break;
#ifdef ZEROTIER
	case SELCONN_ZT:
		snprintf(selconn_Description, sizeof(selconn_Description), "All computers must be connected to the internet.");
		players = MAX_PLRS;
		break;
#endif
	case SELCONN_LOOPBACK:
		snprintf(selconn_Description, sizeof(selconn_Description), "Play by yourself with no network exposure.");
		players = 1;
		break;
	}

	snprintf(selconn_MaxPlayers, sizeof(selconn_MaxPlayers), "Players Supported: %d", players);
	WordWrapArtStr(selconn_Description, DESCRIPTION_WIDTH);
}

void SelconnSelect(std::size_t index)
{
	provider = vecConnItems[index]->m_value;

	SelconnFree();
	selconn_EndMenu = SNetInitializeProvider(provider, selconn_ClientInfo, selconn_UserInfo, selconn_UiInfo, selconn_FileInfo);
	SelconnLoad();
}

int UiSelectProvider(
    _SNETPROGRAMDATA *client_info,
    _SNETPLAYERDATA *user_info,
    _SNETUIDATA *ui_info,
    _SNETVERSIONDATA *file_info)
{
	selconn_ClientInfo = client_info;
	selconn_UserInfo = user_info;
	selconn_UiInfo = ui_info;
	selconn_FileInfo = file_info;
	SelconnLoad();

	selconn_ReturnValue = true;
	selconn_EndMenu = false;
	while (!selconn_EndMenu) {
		UiClearScreen();
		UiPollAndRender();
	}
	SelconnFree();

	return selconn_ReturnValue;
}

DEVILUTION_END_NAMESPACE
