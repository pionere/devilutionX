/**
 * @file interfac.cpp
 *
 * Implementation of load screens.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE *sgpBackCel;
DWORD sgdwProgress;
int progress_id;

/** The color used for the progress bar as an index into the palette. */
const BYTE BarColor[3] = { 138, 43, 254 };
/** The screen position of the top left corner of the progress bar. */
const int BarPos[3][2] = { { 53, 37 }, { 53, 421 }, { 53, 37 } };
/** The width of the progress bar. PANEL_WIDTH - std::max(BarPos[i][0]) * 2 */
#define BAR_WIDTH		PANEL_WIDTH - 53 * 2

static void FreeInterface()
{
	MemFreeDbg(sgpBackCel);
}

static void InitCutscene(unsigned int uMsg)
{
	assert(sgpBackCel == NULL);

	switch (uMsg) {
	case WM_DIABNEXTLVL:
		switch (gnLevelTypeTbl[currlevel]) {
		/*case DTYPE_TOWN:
			sgpBackCel = LoadFileInMem("Gendata\\Cuttt.CEL", NULL);
			LoadPalette("Gendata\\Cuttt.pal");
			progress_id = 1;
			break;*/
		case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
			if (currlevel >= 17) {
				sgpBackCel = LoadFileInMem("Nlevels\\cutl5.CEL", NULL);
				LoadPalette("Nlevels\\cutl5.pal");
				progress_id = 1;
				break;
			}
#endif
			sgpBackCel = LoadFileInMem("Gendata\\Cutl1d.CEL", NULL);
			LoadPalette("Gendata\\Cutl1d.pal");
			progress_id = 0;
			break;
		case DTYPE_CATACOMBS:
			sgpBackCel = LoadFileInMem("Gendata\\Cut2.CEL", NULL);
			LoadPalette("Gendata\\Cut2.pal");
			progress_id = 2;
			break;
		case DTYPE_CAVES:
#ifdef HELLFIRE
			if (currlevel >= 17) {
				sgpBackCel = LoadFileInMem("Nlevels\\cutl6.CEL", NULL);
				LoadPalette("Nlevels\\cutl6.pal");
				progress_id = 1;
				break;
			}
#endif
			sgpBackCel = LoadFileInMem("Gendata\\Cut3.CEL", NULL);
			LoadPalette("Gendata\\Cut3.pal");
			progress_id = 1;
			break;
		case DTYPE_HELL:
			if (currlevel < 15) {
				sgpBackCel = LoadFileInMem("Gendata\\Cut4.CEL", NULL);
				LoadPalette("Gendata\\Cut4.pal");
				progress_id = 1;
			} else {
				sgpBackCel = LoadFileInMem("Gendata\\Cutgate.CEL", NULL);
				LoadPalette("Gendata\\Cutgate.pal");
				progress_id = 1;
			}
			break;
		default:
			ASSUME_UNREACHABLE
		}
		break;
	case WM_DIABPREVLVL:
		/*if (gnLevelTypeTbl[currlevel - 1] == DTYPE_TOWN) {
			sgpBackCel = LoadFileInMem("Gendata\\Cuttt.CEL", NULL);
			LoadPalette("Gendata\\Cuttt.pal");
			progress_id = 1;
		} else {*/
			switch (gnLevelTypeTbl[currlevel]) {
			/*case DTYPE_TOWN:
				sgpBackCel = LoadFileInMem("Gendata\\Cuttt.CEL", NULL);
				LoadPalette("Gendata\\Cuttt.pal");
				progress_id = 1;
				break;*/
			case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
				if (currlevel >= 17) {
					sgpBackCel = LoadFileInMem("Nlevels\\cutl5.CEL", NULL);
					LoadPalette("Nlevels\\cutl5.pal");
					progress_id = 1;
					break;
				}
#endif
				sgpBackCel = LoadFileInMem("Gendata\\Cutl1d.CEL", NULL);
				LoadPalette("Gendata\\Cutl1d.pal");
				progress_id = 0;
				break;
			case DTYPE_CATACOMBS:
				sgpBackCel = LoadFileInMem("Gendata\\Cut2.CEL", NULL);
				LoadPalette("Gendata\\Cut2.pal");
				progress_id = 2;
				break;
			case DTYPE_CAVES:
#ifdef HELLFIRE
				if (currlevel >= 17) {
					sgpBackCel = LoadFileInMem("Nlevels\\cutl6.CEL", NULL);
					LoadPalette("Nlevels\\cutl6.pal");
					progress_id = 1;
					break;
				}
#endif
				sgpBackCel = LoadFileInMem("Gendata\\Cut3.CEL", NULL);
				LoadPalette("Gendata\\Cut3.pal");
				progress_id = 1;
				break;
			case DTYPE_HELL:
				sgpBackCel = LoadFileInMem("Gendata\\Cut4.CEL", NULL);
				LoadPalette("Gendata\\Cut4.pal");
				progress_id = 1;
				break;
			default:
				ASSUME_UNREACHABLE
			}
		//}
		break;
	case WM_DIABSETLVL:
	case WM_DIABRTNLVL:
		if (setlvlnum == SL_BONECHAMB) {
			sgpBackCel = LoadFileInMem("Gendata\\Cut2.CEL", NULL);
			LoadPalette("Gendata\\Cut2.pal");
			progress_id = 2;
		} else if (setlvlnum == SL_VILEBETRAYER) {
			sgpBackCel = LoadFileInMem("Gendata\\Cutportr.CEL", NULL);
			LoadPalette("Gendata\\Cutportr.pal");
			progress_id = 1;
		} else {
			sgpBackCel = LoadFileInMem("Gendata\\Cutl1d.CEL", NULL);
			LoadPalette("Gendata\\Cutl1d.pal");
			progress_id = 0;
		}
		break;
	case WM_DIABWARPLVL:
		sgpBackCel = LoadFileInMem("Gendata\\Cutportl.CEL", NULL);
		LoadPalette("Gendata\\Cutportl.pal");
		progress_id = 1;
		break;
	case WM_DIABLOADGAME:
	case WM_DIABNEWGAME:
		sgpBackCel = LoadFileInMem("Gendata\\Cutstart.CEL", NULL);
		LoadPalette("Gendata\\Cutstart.pal");
		progress_id = 1;
		break;
	case WM_DIABTOWNWARP:
	case WM_DIABTWARPUP:
	case WM_DIABRETOWN: {
		int destlvl = plr[myplr].plrlevel;
		switch (gnLevelTypeTbl[destlvl]) {
		case DTYPE_TOWN:
			sgpBackCel = LoadFileInMem("Gendata\\Cuttt.CEL", NULL);
			LoadPalette("Gendata\\Cuttt.pal");
			progress_id = 1;
			break;
		case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
			if (destlvl >= 17) {
				sgpBackCel = LoadFileInMem("Nlevels\\Cutl5.CEL", NULL);
				LoadPalette("Nlevels\\Cutl5.pal");
				progress_id = 1;
				break;
			}
#endif
			sgpBackCel = LoadFileInMem("Gendata\\Cutl1d.CEL", NULL);
			LoadPalette("Gendata\\Cutl1d.pal");
			progress_id = 0;
			break;
		case DTYPE_CATACOMBS:
			sgpBackCel = LoadFileInMem("Gendata\\Cut2.CEL", NULL);
			LoadPalette("Gendata\\Cut2.pal");
			progress_id = 2;
			break;
		case DTYPE_CAVES:
#ifdef HELLFIRE
			if (destlvl >= 17) {
				sgpBackCel = LoadFileInMem("Nlevels\\Cutl6.CEL", NULL);
				LoadPalette("Nlevels\\Cutl6.pal");
				progress_id = 1;
				break;
			}
#endif
			sgpBackCel = LoadFileInMem("Gendata\\Cut3.CEL", NULL);
			LoadPalette("Gendata\\Cut3.pal");
			progress_id = 1;
			break;
		case DTYPE_HELL:
			sgpBackCel = LoadFileInMem("Gendata\\Cut4.CEL", NULL);
			LoadPalette("Gendata\\Cut4.pal");
			progress_id = 1;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	} break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	sgdwProgress = 0;
}

static void DrawProgress()
{
	BYTE *dst, col;
	int screen_x, screen_y, cursor, i, j;

	screen_x = BarPos[progress_id][0] + PANEL_X;
	screen_y = BarPos[progress_id][1] + SCREEN_Y + UI_OFFSET_Y;
	cursor = screen_x + BUFFER_WIDTH * screen_y;
	col = BarColor[progress_id];
	for (i = sgdwProgress; i != 0; i--, cursor++) {
		dst = &gpBuffer[cursor];
		for (j = 0; j < 22; j++) {
			*dst = col;
			dst += BUFFER_WIDTH;
		}
	}
}

static void DrawCutscene()
{
	lock_buf(1);
	CelDraw(PANEL_X, 480 + SCREEN_Y - 1 + UI_OFFSET_Y, sgpBackCel, 1, 640);

	DrawProgress();

	unlock_buf(1);
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game_screen(FALSE);
}

void interface_msg_pump()
{
	MSG Msg;

	while (PeekMessage(&Msg)) {
		if (Msg.message != DVL_WM_QUIT) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
	}
}

BOOL IncProgress()
{
	interface_msg_pump();
	sgdwProgress += 23;
	if (sgdwProgress > BAR_WIDTH)
		sgdwProgress = BAR_WIDTH;
	if (sgpBackCel != NULL)
		DrawCutscene();
	return sgdwProgress >= BAR_WIDTH;
}


void ShowProgress(unsigned int uMsg)
{
	WNDPROC saveProc;

	gbSomebodyWonGameKludge = FALSE;
	plrmsg_delay(TRUE);

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(DisableInputWndProc);

	interface_msg_pump();
	ClearScreenBuffer();
	scrollrt_draw_game_screen(TRUE);
	InitCutscene(uMsg);
	BlackPalette();
	DrawCutscene();
	PaletteFadeIn();
	IncProgress();
	sound_init();
	IncProgress();

	switch (uMsg) {
	case WM_DIABLOADGAME:
		IncProgress();
		IncProgress();
		LoadGame(TRUE);
		IncProgress();
		IncProgress();
		break;
	case WM_DIABNEWGAME:
		IncProgress();
		FreeGameMem();
		IncProgress();
		pfile_remove_temp_files();
		IncProgress();
		LoadGameLevel(TRUE, ENTRY_MAIN);
		IncProgress();
		break;
	case WM_DIABNEXTLVL:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		FreeGameMem();
		currlevel++;
		leveltype = gnLevelTypeTbl[currlevel];
		assert(plr[myplr].plrlevel == currlevel);
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_MAIN);
		IncProgress();
		break;
	case WM_DIABPREVLVL:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		FreeGameMem();
		currlevel--;
		leveltype = gnLevelTypeTbl[currlevel];
		assert(plr[myplr].plrlevel == currlevel);
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_PREV);
		IncProgress();
		break;
	case WM_DIABSETLVL:
		SetReturnLvlPos();
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		setlevel = TRUE;
		leveltype = gnSetLevelTypeTbl[setlvlnum];
		FreeGameMem();
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_SETLVL);
		IncProgress();
		break;
	case WM_DIABRTNLVL:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		setlevel = FALSE;
		FreeGameMem();
		IncProgress();
		GetReturnLvlPos();
		LoadGameLevel(FALSE, ENTRY_RTNLVL);
		IncProgress();
		break;
	case WM_DIABWARPLVL:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		FreeGameMem();
		GetPortalLevel();
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_WARPLVL);
		IncProgress();
		break;
	case WM_DIABTOWNWARP:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		FreeGameMem();
		currlevel = plr[myplr].plrlevel;
		leveltype = gnLevelTypeTbl[currlevel];
		assert(plr[myplr].plrlevel == currlevel);
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_TWARPDN);
		IncProgress();
		break;
	case WM_DIABTWARPUP:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		FreeGameMem();
		currlevel = plr[myplr].plrlevel;
		leveltype = gnLevelTypeTbl[currlevel];
		assert(plr[myplr].plrlevel == currlevel);
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_TWARPUP);
		IncProgress();
		break;
	case WM_DIABRETOWN:
		IncProgress();
		if (gbMaxPlayers == 1) {
			SaveLevel();
		} else {
			DeltaSaveLevel();
		}
		IncProgress();
		FreeGameMem();
		currlevel = plr[myplr].plrlevel;
		leveltype = gnLevelTypeTbl[currlevel];
		assert(plr[myplr].plrlevel == currlevel);
		IncProgress();
		LoadGameLevel(FALSE, ENTRY_MAIN);
		IncProgress();
		break;
	}

	assert(ghMainWnd != NULL);

	PaletteFadeOut();
	FreeInterface();

	saveProc = SetWindowProc(saveProc);
	assert(saveProc == DisableInputWndProc);

	NetSendCmdLocBParam1(TRUE, CMD_PLAYER_JOINLEVEL, plr[myplr]._px, plr[myplr]._py, plr[myplr].plrlevel);
	plrmsg_delay(FALSE);
	ResetPal();

	if (gbSomebodyWonGameKludge && plr[myplr].plrlevel == 16) {
		PrepDoEnding();
	}

	gbSomebodyWonGameKludge = FALSE;
}

DEVILUTION_END_NAMESPACE
