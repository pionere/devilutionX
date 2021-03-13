/**
 * @file interfac.cpp
 *
 * Implementation of load screens.
 */
#include "all.h"
#include "display.h"

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
	scrollrt_draw_game_screen(false);
}

void interface_msg_pump()
{
	MSG Msg;

	while (FetchMessage(&Msg)) {
		if (Msg.message != DVL_WM_QUIT) {
			TranslateMessage(&Msg);
			PushMessage(&Msg);
		}
	}
}

bool IncProgress()
{
	interface_msg_pump();
	sgdwProgress += 23;
	if (sgdwProgress > BAR_WIDTH)
		sgdwProgress = BAR_WIDTH;
	if (sgpBackCel != NULL)
		DrawCutscene();
	return sgdwProgress >= BAR_WIDTH;
}

static void LoadLvlGFX()
{
	assert(pDungeonCels == NULL);

	switch (leveltype) {
	case DTYPE_TOWN:
#ifdef HELLFIRE
		pDungeonCels = LoadFileInMem("NLevels\\TownData\\Town.CEL", NULL);
		pMegaTiles = LoadFileInMem("NLevels\\TownData\\Town.TIL", NULL);
		pLevelPieces = LoadFileInMem("NLevels\\TownData\\Town.MIN", NULL);
#else
		pDungeonCels = LoadFileInMem("Levels\\TownData\\Town.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\TownData\\Town.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\TownData\\Town.MIN", NULL);
#endif
		pSpecialCels = LoadFileInMem("Levels\\TownData\\TownS.CEL", NULL);
		break;
	case DTYPE_CATHEDRAL:
#ifdef HELLFIRE
		if (currlevel >= 21) {
			pDungeonCels = LoadFileInMem("NLevels\\L5Data\\L5.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\L5Data\\L5.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\L5Data\\L5.MIN", NULL);
			pSpecialCels = LoadFileInMem("NLevels\\L5Data\\L5S.CEL", NULL);
			break;
		}
#endif
		pDungeonCels = LoadFileInMem("Levels\\L1Data\\L1.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L1Data\\L1.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L1Data\\L1.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L1Data\\L1S.CEL", NULL);
		break;
	case DTYPE_CATACOMBS:
		pDungeonCels = LoadFileInMem("Levels\\L2Data\\L2.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L2Data\\L2.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L2Data\\L2.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L2Data\\L2S.CEL", NULL);
		break;
	case DTYPE_CAVES:
#ifdef HELLFIRE
		if (currlevel >= 17) {
			pDungeonCels = LoadFileInMem("NLevels\\L6Data\\L6.CEL", NULL);
			pMegaTiles = LoadFileInMem("NLevels\\L6Data\\L6.TIL", NULL);
			pLevelPieces = LoadFileInMem("NLevels\\L6Data\\L6.MIN", NULL);
		} else
#endif
		{
			pDungeonCels = LoadFileInMem("Levels\\L3Data\\L3.CEL", NULL);
			pMegaTiles = LoadFileInMem("Levels\\L3Data\\L3.TIL", NULL);
			pLevelPieces = LoadFileInMem("Levels\\L3Data\\L3.MIN", NULL);
		}
		pSpecialCels = LoadFileInMem("Levels\\L1Data\\L1S.CEL", NULL);
		break;
	case DTYPE_HELL:
		pDungeonCels = LoadFileInMem("Levels\\L4Data\\L4.CEL", NULL);
		pMegaTiles = LoadFileInMem("Levels\\L4Data\\L4.TIL", NULL);
		pLevelPieces = LoadFileInMem("Levels\\L4Data\\L4.MIN", NULL);
		pSpecialCels = LoadFileInMem("Levels\\L2Data\\L2S.CEL", NULL);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void LoadAllGFX()
{
	IncProgress();
	IncProgress();
	InitObjectGFX();
	IncProgress();
	InitMissileGFX();
	IncProgress();
}

/**
 * @param lvldir method of entry
 */
static void CreateLevel(int lvldir)
{
	switch (leveltype) {
	case DTYPE_TOWN:
		CreateTown(lvldir);
		InitTownTriggers();
		LoadRndLvlPal(DTYPE_TOWN);
		break;
	case DTYPE_CATHEDRAL:
		CreateL1Dungeon(glSeedTbl[currlevel], lvldir);
		InitL1Triggers();
		Freeupstairs();
#ifdef HELLFIRE
		if (currlevel >= 21) {
			LoadRndLvlPal(DTYPE_NEST);
			break;
		}
#endif
		LoadRndLvlPal(DTYPE_CATHEDRAL);
		break;
	case DTYPE_CATACOMBS:
		CreateL2Dungeon(glSeedTbl[currlevel], lvldir);
		InitL2Triggers();
		Freeupstairs();
		LoadRndLvlPal(DTYPE_CATACOMBS);
		break;
	case DTYPE_CAVES:
		CreateL3Dungeon(glSeedTbl[currlevel], lvldir);
		InitL3Triggers();
		Freeupstairs();
#ifdef HELLFIRE
		if (currlevel >= 17) {
			LoadRndLvlPal(DTYPE_CRYPT);
			break;
		}
#endif
		LoadRndLvlPal(DTYPE_CAVES);
		break;
	case DTYPE_HELL:
		CreateL4Dungeon(glSeedTbl[currlevel], lvldir);
		InitL4Triggers();
		Freeupstairs();
		LoadRndLvlPal(DTYPE_HELL);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void LoadGameLevel(bool firstflag, int lvldir)
{
	int i;

	if (firstflag && lvldir == ENTRY_MAIN) {
		InitLevels();
		InitQuests();
		InitPortals();
		InitDungMsgs(myplr);
	}

#ifdef _DEBUG
	if (setseed)
		glSeedTbl[currlevel] = setseed;
#endif

	music_stop();
	//if (pcurs > CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
	//	NewCursor(CURSOR_HAND);
	//}
	//SetRndSeed(glSeedTbl[currlevel]);
	IncProgress();
	MakeLightTable();
	LoadLvlGFX();
	IncProgress();

	if (firstflag) {
		InitInv();
		InitItemGFX();
		InitQuestText();

		for (i = 0; i < gbMaxPlayers; i++)
			InitPlrGFXMem(i);

		InitStores();
		InitAutomapOnce();
		InitHelp();
		InitControlPan();
	}

	//SetRndSeed(glSeedTbl[currlevel]);

	IncProgress();
	InitAutomap();

	if (lvldir != ENTRY_LOAD) {
		InitLighting();
		InitVision();
	}

	InitLevelMonsters();
	IncProgress();

	if (!gbSetlevel) {
		CreateLevel(lvldir);
		IncProgress();
		FillSolidBlockTbls();
		if (leveltype != DTYPE_TOWN) {
			SetRndSeed(glSeedTbl[currlevel]);
			GetLevelMTypes();
			InitThemes();
			LoadAllGFX();
		} else {
			SetupTownStores();
			IncProgress();
			IncProgress();
			InitMissileGFX();
			IncProgress();
			IncProgress();
		}

		IncProgress();

		if (lvldir == ENTRY_RTNLVL)
			GetReturnLvlPos();
		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag, true);
			}
		}

		PlayDungMsgs();
		IncProgress();

		SetRndSeed(glSeedTbl[currlevel]);

		if (leveltype != DTYPE_TOWN) {
			if (firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pLvlVisited[currlevel] || gbMaxPlayers != 1) {
				HoldThemeRooms();
				InitMonsters();
				IncProgress();
				InitObjects();
				InitItems();
#ifdef HELLFIRE
				if (currlevel < 17)
#endif
					CreateThemeRooms();
				IncProgress();
				InitMissiles();
				InitDead();

				if (gbMaxPlayers != 1)
					DeltaLoadLevel();

				IncProgress();
				SavePreLighting();
			} else {
				HoldThemeRooms();
				InitMonsters();
				InitMissiles();
				InitDead();
				IncProgress();
				LoadLevel();
				IncProgress();
			}
		} else {
			InitTowners();
			InitItems();
			InitMissiles();
			IncProgress();

			if (gbMaxPlayers != 1)
				DeltaLoadLevel();
			else if (!firstflag && lvldir != ENTRY_LOAD && plr[myplr]._pLvlVisited[currlevel])
				LoadLevel();

			IncProgress();
		}
		if (gbMaxPlayers == 1)
			ResyncQuests();
		else
			ResyncMPQuests();
	} else {
		LoadSetMap();
		IncProgress();
		GetLevelMTypes();
		IncProgress();
		InitMonsters();
		IncProgress();
		InitMissileGFX();
		IncProgress();
		InitDead();
		IncProgress();
		FillSolidBlockTbls();
		IncProgress();

		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();
		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currlevel == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag, true);
			}
		}

		//PlayDungMsgs();
		if (setlvlnum == SL_SKELKING && quests[Q_SKELKING]._qactive == QUEST_ACTIVE) {
			sfxdelay = 30;
			sfxdnum = USFX_SKING1;
		}

		IncProgress();
		IncProgress();

		if (firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pSLvlVisited[setlvlnum]) {
			InitItems();
			SavePreLighting();
		} else {
			LoadLevel();
		}

		InitMissiles();
		IncProgress();
	}

	SyncPortals();

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && plr[i].plrlevel == currlevel && (!plr[i]._pLvlChanging || i == myplr)) {
			if (plr[i]._pHitPoints >= (1 << 6)) {
				/*if (gbMaxPlayers == 1)
					dPlayer[plr[i]._px][plr[i]._py] = i + 1;
				else*/
					SyncInitPlrPos(i);
			} else {
				dFlags[plr[i]._px][plr[i]._py] |= BFLAG_DEAD_PLAYER;
			}
		}
	}

	SetDungeonMicros(0, 0, MAXDUNX, MAXDUNY);

	InitLightMax();
	IncProgress();
	IncProgress();
	IncProgress();
	ProcessLightList();
	ProcessVisionList();

#ifdef HELLFIRE
	// BUGFIX: TODO: does not belong here, DeltaLoadLevel should take care about this
	if (currlevel == 24 && quests[Q_NAKRUL]._qactive == QUEST_DONE) {
		OpenUberRoom();
	}
#endif

#ifdef HELLFIRE
	if (currlevel >= 17)
		music_start(currlevel > 20 ? TMUSIC_L5 : TMUSIC_L6);
	else
#endif
		music_start(leveltype);

	while (!IncProgress())
		;
}

void ShowProgress(unsigned int uMsg)
{
	WNDPROC saveProc;

	gbSomebodyWonGameKludge = false;
	plrmsg_delay(true);

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(DisableInputWndProc);

	interface_msg_pump();
	ClearScreenBuffer();
	scrollrt_draw_game_screen(true);
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
		LoadGame(true);
		IncProgress();
		IncProgress();
		break;
	case WM_DIABNEWGAME:
		IncProgress();
		FreeGameMem();
		IncProgress();
		pfile_remove_temp_files();
		IncProgress();
		LoadGameLevel(true, ENTRY_MAIN);
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
		LoadGameLevel(false, ENTRY_MAIN);
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
		LoadGameLevel(false, ENTRY_PREV);
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
		gbSetlevel = true;
		leveltype = gnSetLevelTypeTbl[setlvlnum];
		FreeGameMem();
		IncProgress();
		LoadGameLevel(false, ENTRY_SETLVL);
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
		gbSetlevel = false;
		FreeGameMem();
		IncProgress();
		GetReturnLvlPos();
		LoadGameLevel(false, ENTRY_RTNLVL);
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
		LoadGameLevel(false, ENTRY_WARPLVL);
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
		LoadGameLevel(false, ENTRY_TWARPDN);
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
		LoadGameLevel(false, ENTRY_TWARPUP);
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
		LoadGameLevel(false, ENTRY_MAIN);
		IncProgress();
		break;
	}

	assert(ghMainWnd != NULL);

	PaletteFadeOut();
	FreeInterface();

	saveProc = SetWindowProc(saveProc);
	assert(saveProc == DisableInputWndProc);

	NetSendCmdLocBParam1(true, CMD_PLAYER_JOINLEVEL, plr[myplr]._px, plr[myplr]._py, plr[myplr].plrlevel);
	plrmsg_delay(false);
	ResetPal();

	if (gbSomebodyWonGameKludge && plr[myplr].plrlevel == 16) {
		PrepDoEnding(gbSoundOn);
	}

	gbSomebodyWonGameKludge = false;
}

DEVILUTION_END_NAMESPACE
