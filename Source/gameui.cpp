/**
 * @file gameui.cpp
 *
 * Implementation of the in-game ui functions.
 */
#include "all.h"
#include "storm/storm_cfg.h"

DEVILUTION_BEGIN_NAMESPACE

int gnWndBeltX, gnWndBeltY;
int gnWndInvX, gnWndInvY;
int gnWndCharX, gnWndCharY;
int gnWndBookX, gnWndBookY;
int gnWndTeamX, gnWndTeamY;
int gnWndQuestX, gnWndQuestY;

#define WND_CHECK_X(x, dx, w) ((x + dx) >= SCREEN_WIDTH - w + BORDER_LEFT || (x + dx) < -BORDER_LEFT)
#define WND_CHECK_Y(y, dy, h) ((y + dy) >= SCREEN_HEIGHT - (h + 1) + BORDER_BOTTOM || (y + dy) < -BORDER_TOP)

void InitGameUI()
{
	gnWndBeltX = BELT_LEFT, gnWndBeltY = BELT_TOP;
	gnWndInvX = RIGHT_PANEL, gnWndInvY = 0;
	gnWndCharX = 0, gnWndCharY = 0;
	gnWndBookX = RIGHT_PANEL, gnWndBookY = 0;
	gnWndTeamX = RIGHT_PANEL, gnWndTeamY = 0;
	gnWndQuestX = 0, gnWndQuestY = 0;

	getIniInt("GameUI", "BeltX", &gnWndBeltX);
	getIniInt("GameUI", "BeltY", &gnWndBeltY);
	getIniInt("GameUI", "InvX", &gnWndInvX);
	getIniInt("GameUI", "InvY", &gnWndInvY);
	getIniInt("GameUI", "CharX", &gnWndCharX);
	getIniInt("GameUI", "CharY", &gnWndCharY);
	getIniInt("GameUI", "BookX", &gnWndBookX);
	getIniInt("GameUI", "BookY", &gnWndBookY);
	getIniInt("GameUI", "TeamX", &gnWndTeamX);
	getIniInt("GameUI", "TeamY", &gnWndTeamY);
	getIniInt("GameUI", "QuestX", &gnWndQuestX);
	getIniInt("GameUI", "QuestY", &gnWndQuestY);

	if (WND_CHECK_X(gnWndBeltX, 0, BELT_WIDTH)) {
		DoLog("BeltX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBeltX, -BORDER_LEFT, SCREEN_WIDTH - BELT_WIDTH + BORDER_LEFT);
		gnWndBeltX = 0;
	}
	if (WND_CHECK_Y(gnWndBeltY, 0, BELT_HEIGHT)) {
		DoLog("BeltY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBeltY, -BORDER_TOP, SCREEN_HEIGHT - (BELT_HEIGHT + 1) + BORDER_BOTTOM);
		gnWndBeltY = 0;
	}
	if (WND_CHECK_X(gnWndInvX, 0, SPANEL_WIDTH)) {
		DoLog("InvX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndInvX, -BORDER_LEFT, SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndInvX = 0;
	}
	if (WND_CHECK_Y(gnWndInvY, 0, SPANEL_HEIGHT)) {
		DoLog("InvY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndInvY, -BORDER_TOP, SCREEN_HEIGHT - (SPANEL_HEIGHT + 1) + BORDER_BOTTOM);
		gnWndInvY = 0;
	}
	if (WND_CHECK_X(gnWndCharX, 0, SPANEL_WIDTH)) {
		DoLog("CharX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndCharX, -BORDER_LEFT, SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndCharX = 0;
	}
	if (WND_CHECK_Y(gnWndCharY, 0, SPANEL_HEIGHT)) {
		DoLog("CharY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndCharY, -BORDER_TOP, SCREEN_HEIGHT - (SPANEL_HEIGHT + 1) + BORDER_BOTTOM);
		gnWndCharY = 0;
	}
	if (WND_CHECK_X(gnWndBookX, 0, SPANEL_WIDTH)) {
		DoLog("BookX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBookX, -BORDER_LEFT, SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndBookX = 0;
	}
	if (WND_CHECK_Y(gnWndBookY, 0, SPANEL_HEIGHT)) {
		DoLog("BookY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBookY, -BORDER_TOP, SCREEN_HEIGHT - (SPANEL_HEIGHT + 1) + BORDER_BOTTOM);
		gnWndBookY = 0;
	}
	if (WND_CHECK_X(gnWndTeamX, 0, SPANEL_WIDTH)) {
		DoLog("TeamX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndTeamX, -BORDER_LEFT, SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndTeamX = 0;
	}
	if (WND_CHECK_Y(gnWndTeamY, 0, SPANEL_HEIGHT)) {
		DoLog("TeamY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndTeamY, -BORDER_TOP, SCREEN_HEIGHT - (SPANEL_HEIGHT + 1) + BORDER_BOTTOM);
		gnWndTeamY = 0;
	}
	if (WND_CHECK_X(gnWndQuestX, 0, SPANEL_WIDTH)) {
		DoLog("QuesX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndQuestX, -BORDER_LEFT, SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndQuestX = 0;
	}
	if (WND_CHECK_Y(gnWndQuestY, 0, SPANEL_HEIGHT)) {
		DoLog("QuesY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndQuestY, -BORDER_TOP, SCREEN_HEIGHT - (SPANEL_HEIGHT + 1) + BORDER_BOTTOM);
		gnWndQuestY = 0;
	}
}

bool MoveWndPos(BYTE wnd, int dx, int dy)
{
	switch (wnd) {
	case WND_INV:
		if (WND_CHECK_X(gnWndInvX, dx, SPANEL_WIDTH))
			break;
		if (WND_CHECK_Y(gnWndInvY, dy, SPANEL_HEIGHT))
			break;
		gnWndInvX += dx;
		gnWndInvY += dy;
		return true;
	case WND_CHAR:
		if (WND_CHECK_X(gnWndCharX, dx, SPANEL_WIDTH))
			break;
		if (WND_CHECK_Y(gnWndCharY, dy, SPANEL_HEIGHT))
			break;
		gnWndCharX += dx;
		gnWndCharY += dy;
		return true;
	case WND_BOOK:
		if (WND_CHECK_X(gnWndBookX, dx, SPANEL_WIDTH))
			break;
		if (WND_CHECK_Y(gnWndBookY, dy, SPANEL_HEIGHT))
			break;
		gnWndBookX += dx;
		gnWndBookY += dy;
		return true;
	case WND_TEAM:
		if (WND_CHECK_X(gnWndTeamX, dx, SPANEL_WIDTH))
			break;
		if (WND_CHECK_Y(gnWndTeamY, dy, SPANEL_HEIGHT))
			break;
		gnWndTeamX += dx;
		gnWndTeamY += dy;
		return true;
	case WND_QUEST:
		if (WND_CHECK_X(gnWndQuestX, dx, SPANEL_WIDTH))
			break;
		if (WND_CHECK_Y(gnWndQuestY, dy, SPANEL_HEIGHT))
			break;
		gnWndQuestX += dx;
		gnWndQuestY += dy;
		return true;
	case WND_BELT:
		if (WND_CHECK_X(gnWndBeltX, dx, BELT_WIDTH))
			break;
		if (WND_CHECK_Y(gnWndBeltY, dy, BELT_HEIGHT))
			break;
		gnWndBeltX += dx;
		gnWndBeltY += dy;
		return true;
	}

	return false;
}

DEVILUTION_END_NAMESPACE
