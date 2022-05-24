/**
 * @file gameui.cpp
 *
 * Implementation of the in-game ui functions.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int gnWndBeltX, gnWndBeltY;
int gnWndInvX, gnWndInvY;
int gnWndCharX, gnWndCharY;
int gnWndBookX, gnWndBookY;
int gnWndTeamX, gnWndTeamY;
int gnWndQuestX, gnWndQuestY;

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

	if (gnWndBeltX >= SCREEN_X + SCREEN_WIDTH - BELT_WIDTH + BORDER_LEFT || gnWndBeltX < -BORDER_LEFT) {
		DoLog("BeltX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBeltX, -BORDER_LEFT, SCREEN_X + SCREEN_WIDTH - BELT_WIDTH + BORDER_LEFT);
		gnWndBeltX = 0;
	}
	if (gnWndBeltY >= SCREEN_Y + SCREEN_HEIGHT - BELT_HEIGHT + BORDER_BOTTOM || gnWndBeltY < -BORDER_TOP) {
		DoLog("BeltY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBeltY, -BORDER_TOP, SCREEN_Y + SCREEN_HEIGHT - BELT_HEIGHT + BORDER_BOTTOM);
		gnWndBeltY = 0;
	}
	if (gnWndInvX >= SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT || gnWndInvX < -BORDER_LEFT) {
		DoLog("InvX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndInvX, -BORDER_LEFT, SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndInvX = 0;
	}
	if (gnWndInvY >= SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM || gnWndInvY < -BORDER_TOP) {
		DoLog("InvY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndInvY, -BORDER_TOP, SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM);
		gnWndInvY = 0;
	}
	if (gnWndCharX >= SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT || gnWndCharX < -BORDER_LEFT) {
		DoLog("CharX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndCharX, -BORDER_LEFT, SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndCharX = 0;
	}
	if (gnWndCharY >= SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM || gnWndCharY < -BORDER_TOP) {
		DoLog("CharY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndCharY, -BORDER_TOP, SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM);
		gnWndCharY = 0;
	}
	if (gnWndBookX >= SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT || gnWndBookX < -BORDER_LEFT) {
		DoLog("BookX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBookX, -BORDER_LEFT, SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndBookX = 0;
	}
	if (gnWndBookY >= SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM || gnWndBookY < -BORDER_TOP) {
		DoLog("BookY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndBookY, -BORDER_TOP, SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM);
		gnWndBookY = 0;
	}
	if (gnWndTeamX >= SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT || gnWndTeamX < -BORDER_LEFT) {
		DoLog("TeamX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndTeamX, -BORDER_LEFT, SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndTeamX = 0;
	}
	if (gnWndTeamY >= SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM || gnWndTeamY < -BORDER_TOP) {
		DoLog("TeamY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndTeamY, -BORDER_TOP, SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM);
		gnWndTeamY = 0;
	}
	if (gnWndQuestX >= SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT || gnWndQuestX < -BORDER_LEFT) {
		DoLog("QuesX(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndQuestX, -BORDER_LEFT, SCREEN_X + SCREEN_WIDTH - SPANEL_WIDTH + BORDER_LEFT);
		gnWndQuestX = 0;
	}
	if (gnWndQuestY >= SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM || gnWndQuestY < -BORDER_TOP) {
		DoLog("QuesY(GameUI) setting(%d) is ignored because it is out of range (%d .. %d).", gnWndQuestY, -BORDER_TOP, SCREEN_Y + SCREEN_HEIGHT - SPANEL_HEIGHT + BORDER_BOTTOM);
		gnWndQuestY = 0;
	}
}

DEVILUTION_END_NAMESPACE
