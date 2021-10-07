/**
 * @file doom.cpp
 *
 * Implementation of the map of the stars quest.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE* pDoomCel = NULL;
bool gbDoomflag;

#define DOOM_CELSIZE 0x38000

#ifndef HELLFIRE
int doom_quest_time;
int doom_stars_drawn;
int DoomQuestState;

/*
stastic void doom_reset_state()
{
	if (DoomQuestState < 0) {
		DoomQuestState = 0;
	}
}

static void doom_play_movie()
{
	if (DoomQuestState < 36001) {
		DoomQuestState++;
		if (DoomQuestState == 36001) {
			PlayInGameMovie("gendata\\doom.smk");
			DoomQuestState++;
		}
	}
}
*/

static int doom_get_frame_from_time()
{
	if (DoomQuestState == 36001) {
		return 31;
	}

	return DoomQuestState / 1200;
}
#endif

static void doom_load_graphics()
{
#ifdef HELLFIRE
	LoadFileWithMem("Items\\Map\\MapZtown.CEL", pDoomCel);
#else
	if (doom_quest_time == 31) {
		copy_cstr(tempstr, "Items\\Map\\MapZDoom.CEL");
	} else if (doom_quest_time < 10) {
		snprintf(tempstr, sizeof(tempstr), "Items\\Map\\MapZ000%i.CEL", doom_quest_time);
	} else {
		snprintf(tempstr, sizeof(tempstr), "Items\\Map\\MapZ00%i.CEL", doom_quest_time);
	}
	LoadFileWithMem(tempstr, pDoomCel);
#endif
}

void doom_init()
{
	doom_close();

	pDoomCel = DiabloAllocPtr(DOOM_CELSIZE);
	// assert(pDoomCel != NULL);
#ifndef HELLFIRE
	doom_quest_time = doom_get_frame_from_time() == 31 ? 31 : 0;
#endif
	doom_load_graphics();
	gbDoomflag = true;
}

void doom_close()
{
	gbDoomflag = false;
	MemFreeDbg(pDoomCel);
}

void doom_draw()
{
	assert(gbDoomflag);
#ifndef HELLFIRE
	if (doom_quest_time != 31) {
		doom_stars_drawn++;
		if (doom_stars_drawn >= 5) {
			doom_stars_drawn = 0;
			doom_quest_time++;
			if (doom_quest_time > doom_get_frame_from_time()) {
				doom_quest_time = 0;
			}
			doom_load_graphics();
		}
	}
#endif

	CelDraw(PANEL_X, PANEL_Y - 1, pDoomCel, 1, 640);
}

DEVILUTION_END_NAMESPACE
