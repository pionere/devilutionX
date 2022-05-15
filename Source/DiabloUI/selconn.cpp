#include "selconn.h"

#include "DiabloUI/diabloui.h"
#include "DiabloUI/text.h"
#include "storm/storm_net.h"
//#include "all.h"
#include <algorithm>
#include "../gameui.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

bool selconn_bMulti = false;
int provider;

static char selconn_MaxPlayers[21];
static char selconn_Description[64];
static bool selconn_ReturnValue;
static bool selconn_EndMenu;

#define DESCRIPTION_WIDTH	(SELCONN_LPANEL_WIDTH - 2 * 10)

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
	switch (gUIListItems[index]->m_value) {
#ifdef TCPIP
	case SELCONN_TCP:
	case SELCONN_TCPD:
#ifndef NOHOSTING
	case SELCONN_TCPS:
	case SELCONN_TCPDS:
#endif
		copy_cstr(selconn_Description, "All computers must be connected to a TCP-compatible network.");
		//numplayers = MAX_PLRS;
		break;
#endif
#ifdef ZEROTIER
	case SELCONN_ZT:
		copy_cstr(selconn_Description, "All computers must be connected to the internet.");
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
	int numOptions = 0;

	LoadBackgroundArt("ui_art\\selconn.CEL", "ui_art\\menu.pal");
#ifndef HOSTONLY
	gUIListItems.push_back(new UiListItem("Loopback", SELCONN_LOOPBACK));
	numOptions++;
#ifdef ZEROTIER
	gUIListItems.push_back(new UiListItem("Zerotier", SELCONN_ZT));
	numOptions++;
#endif
#endif // HOSTONLY
#ifdef TCPIP
#ifndef HOSTONLY
	gUIListItems.push_back(new UiListItem("Client-Server (TCP)", SELCONN_TCP));
	numOptions++;
	gUIListItems.push_back(new UiListItem("Client-Server (TCP-D)", SELCONN_TCPD));
	numOptions++;
#endif // HOSTONLY
#ifndef NOHOSTING
	gUIListItems.push_back(new UiListItem("Server (TCP)", SELCONN_TCPS));
	numOptions++;
	gUIListItems.push_back(new UiListItem("Server (TCP-D)", SELCONN_TCPDS));
	numOptions++;
#endif // NOHOSTING
#endif // TCPIP

	UiAddBackground(&gUiItems);
	UiAddLogo(&gUiItems);

	SDL_Rect rect1 = { PANEL_LEFT + 0, SELCONN_TITLE_TOP, PANEL_WIDTH, 35 };
	gUiItems.push_back(new UiText("Multi Player Game", rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { SELCONN_LPANEL_LEFT + 10, SELCONN_PNL_TOP, DESCRIPTION_WIDTH, SELCONN_HEADER_HEIGHT };
	gUiItems.push_back(new UiText(selconn_MaxPlayers, rect2, UIS_LEFT | UIS_VCENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect3 = { SELCONN_LPANEL_LEFT + 10, SELCONN_CONTENT_TOP + 11, DESCRIPTION_WIDTH, 21 };
	gUiItems.push_back(new UiText("Requirements:", rect3, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect4 = { SELCONN_LPANEL_LEFT + 10, SELCONN_CONTENT_TOP + 32, DESCRIPTION_WIDTH, 66 };
	gUiItems.push_back(new UiText(selconn_Description, rect4, UIS_LEFT | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect7 = { SELCONN_RPANEL_LEFT, SELCONN_PNL_TOP, SELCONN_RPANEL_WIDTH, SELCONN_HEADER_HEIGHT };
	gUiItems.push_back(new UiText("Select Connection", rect7, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	//assert(numOptions == gUIListItems.size());
	SDL_Rect rect8 = { SELCONN_RPANEL_LEFT + (SELCONN_RPANEL_WIDTH - 320) / 2, SELCONN_LIST_TOP, 320, 26 * numOptions };
	gUiItems.push_back(new UiList(&gUIListItems, numOptions, rect8, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect9 = { SELCONN_RPANEL_LEFT + 19, SELCONN_RBUTTON_TOP, 140, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect9, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect10 = { SELCONN_RPANEL_LEFT + 174, SELCONN_RBUTTON_TOP, 140, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect10, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(numOptions == gUIListItems.size());
	UiInitList(numOptions, SelconnFocus, SelconnSelect, SelconnEsc);
}

static void SelconnFree()
{
	MemFreeDbg(gbBackCel);
	UiClearListItems();

	UiClearItems(gUiItems);

	//UiInitList_clear();
}

static void SelconnSelect(unsigned index)
{
	provider = gUIListItems[index]->m_value;

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
		assert(gUIListItems[0]->m_value == SELCONN_LOOPBACK);
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
