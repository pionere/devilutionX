#include "selconn.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

bool selconn_bMulti = false;
int provider;

static char selconn_MaxPlayers[21];
static char selconn_Description[64];
static bool selconn_ReturnValue = false;
static bool selconn_EndMenu = false;

static std::vector<UiListItem *> vecConnItems;
static std::vector<UiItemBase *> vecSelConnDlg;

#define DESCRIPTION_WIDTH 205

// Forward-declare UI-handlers, used by other handlers.
static void SelconnSelect(unsigned index);

static void SelconnEsc()
{
	selconn_ReturnValue = false;
	selconn_EndMenu = true;
}

static void SelconnFocus(unsigned index)
{
	int numplayers = MAX_PLRS;
	switch (vecConnItems[index]->m_value) {
#ifdef TCPIP
	case SELCONN_TCP:
	case SELCONN_TCPD:
#ifndef NOHOSTING
	case SELCONN_TCPS:
	case SELCONN_TCPDS:
#endif
		snprintf(selconn_Description, sizeof(selconn_Description), "All computers must be connected to a TCP-compatible network.");
		//numplayers = MAX_PLRS;
		break;
#endif
#ifdef ZEROTIER
	case SELCONN_ZT:
		snprintf(selconn_Description, sizeof(selconn_Description), "All computers must be connected to the internet.");
		//numplayers = MAX_PLRS;
		break;
#endif
	case SELCONN_LOOPBACK:
		snprintf(selconn_Description, sizeof(selconn_Description), "Play by yourself with no network exposure.");
		numplayers = 1;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	snprintf(selconn_MaxPlayers, sizeof(selconn_MaxPlayers), "Players Supported: %d", numplayers);
	WordWrapArtStr(selconn_Description, DESCRIPTION_WIDTH, AFT_SMALL);
}

static void SelconnLoad()
{
	LoadBackgroundArt("ui_art\\selconn.pcx");
#ifndef HOSTONLY
	vecConnItems.push_back(new UiListItem("Loopback", SELCONN_LOOPBACK));
#ifdef ZEROTIER
	vecConnItems.push_back(new UiListItem("Zerotier", SELCONN_ZT));
#endif
#endif // HOSTONLY
#ifdef TCPIP
#ifndef HOSTONLY
	vecConnItems.push_back(new UiListItem("Client-Server (TCP)", SELCONN_TCP));
	vecConnItems.push_back(new UiListItem("Client-Server (TCP-D)", SELCONN_TCPD));
#endif // HOSTONLY
#ifndef NOHOSTING
	vecConnItems.push_back(new UiListItem("Server (TCP)", SELCONN_TCPS));
	vecConnItems.push_back(new UiListItem("Server (TCP-D)", SELCONN_TCPDS));
#endif // NOHOSTING
#endif // TCPIP

	UiAddBackground(&vecSelConnDlg);
	UiAddLogo(&vecSelConnDlg);

	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelConnDlg.push_back(new UiArtText("Multi Player Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 218), DESCRIPTION_WIDTH, 21 };
	vecSelConnDlg.push_back(new UiArtText(selconn_MaxPlayers, rect2, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect3 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 256), DESCRIPTION_WIDTH, 21 };
	vecSelConnDlg.push_back(new UiArtText("Requirements:", rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { PANEL_LEFT + 35, (UI_OFFSET_Y + 275), DESCRIPTION_WIDTH, 66 };
	vecSelConnDlg.push_back(new UiArtText(selconn_Description, rect4, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect5 = { PANEL_LEFT + 30, (UI_OFFSET_Y + 356), 220, 31 };
	vecSelConnDlg.push_back(new UiArtText("no gateway needed", rect5, UIS_CENTER | UIS_MED | UIS_SILVER));

	SDL_Rect rect7 = { PANEL_LEFT + 300, (UI_OFFSET_Y + 211), 295, 33 };
	vecSelConnDlg.push_back(new UiArtText("Select Connection", rect7, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect8 = { PANEL_LEFT + 305, (UI_OFFSET_Y + 256), 285, 26 };
	vecSelConnDlg.push_back(new UiList(&vecConnItems, rect8, UIS_CENTER | UIS_VCENTER | UIS_SMALL | UIS_GOLD));

	SDL_Rect rect9 = { PANEL_LEFT + 299, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelConnDlg.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect9, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect10 = { PANEL_LEFT + 454, (UI_OFFSET_Y + 427), 140, 35 };
	vecSelConnDlg.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect10, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelConnDlg, vecConnItems.size(), SelconnFocus, SelconnSelect, SelconnEsc);
}

static void SelconnFree()
{
	ArtBackground.Unload();

	UiClearListItems(vecConnItems);

	UiClearItems(vecSelConnDlg);
}

static void SelconnSelect(unsigned index)
{
	provider = vecConnItems[index]->m_value;

	SNetInitializeProvider(provider);
	selconn_EndMenu = true;
}

bool UiSelectProvider(bool bMulti)
{
	selconn_bMulti = bMulti;
	SelconnLoad();

	selconn_ReturnValue = true;
	selconn_EndMenu = false;

	if (!selconn_bMulti) {
		assert(vecConnItems[0]->m_value == SELCONN_LOOPBACK);
		SelconnSelect(0);
	}

	while (!selconn_EndMenu) {
		UiClearScreen();
		UiPollAndRender();
	}
	SelconnFree();

	return selconn_ReturnValue;
}

DEVILUTION_END_NAMESPACE
