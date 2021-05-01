/**
 * @file interfac.cpp
 *
 * Implementation of load screens.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE *sgpBackCel;
unsigned sgdwProgress;
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
	int lvl;
	assert(sgpBackCel == NULL);

	switch (uMsg) {
	case WM_DIABNEXTLVL:
		lvl = currLvl._dLevelIdx;
		assert(currLvl._dType == AllLevels[lvl].dType);
		switch (AllLevels[lvl].dType) {
		case DTYPE_CATHEDRAL:
			progress_id = 0;
			break;
		case DTYPE_CATACOMBS:
			progress_id = 2;
			break;
		case DTYPE_CAVES:
			progress_id = 1;
			break;
		case DTYPE_HELL:
			lvl = plr[myplr].plrlevel; // the destination level
			progress_id = 1;
			break;
		case DTYPE_CRYPT:
		case DTYPE_NEST:
			progress_id = 1;
			break;
		default:
			ASSUME_UNREACHABLE
		}
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels, NULL);
		LoadPalette(AllLevels[lvl].dLoadPal);
		break;
	case WM_DIABPREVLVL:
		lvl = currLvl._dLevelIdx;
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels, NULL);
		LoadPalette(AllLevels[lvl].dLoadPal);
		assert(currLvl._dType == AllLevels[lvl].dType);
		switch (AllLevels[lvl].dType) {
		case DTYPE_CATHEDRAL:
			progress_id = 0;
			break;
		case DTYPE_CATACOMBS:
			progress_id = 2;
			break;
		case DTYPE_CAVES:
		case DTYPE_HELL:
		case DTYPE_CRYPT:
		case DTYPE_NEST:
			progress_id = 1;
			break;
		default:
			ASSUME_UNREACHABLE
		}
		break;
	case WM_DIABSETLVL:
		lvl = plr[myplr].plrlevel; // the destination level
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels, NULL);
		LoadPalette(AllLevels[lvl].dLoadPal);
		if (lvl == SL_BONECHAMB) {
			progress_id = 2;
		} else if (lvl == SL_VILEBETRAYER) {
			progress_id = 1;
		} else {
			progress_id = 0;
		}
		break;
	case WM_DIABRTNLVL:
		lvl = currLvl._dLevelIdx;
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels, NULL);
		LoadPalette(AllLevels[lvl].dLoadPal);
		if (lvl == SL_BONECHAMB) {
			progress_id = 2;
		} else if (lvl == SL_VILEBETRAYER) {
			progress_id = 1;
		} else {
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
		lvl = plr[myplr].plrlevel; // the destination level
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels, NULL);
		LoadPalette(AllLevels[lvl].dLoadPal);
		switch (AllLevels[lvl].dType) {
		case DTYPE_TOWN:
			progress_id = 1;
			break;
		case DTYPE_CATHEDRAL:
			progress_id = 0;
			break;
		case DTYPE_CATACOMBS:
			progress_id = 2;
			break;
		case DTYPE_CAVES:
		case DTYPE_HELL:
		case DTYPE_CRYPT:
		case DTYPE_NEST:
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

	while (PeekMessage(&Msg)) {
		if (Msg.message != DVL_WM_QUIT) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
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
	const LevelDataStruct *lds;
	assert(pDungeonCels == NULL);
	lds = &AllLevels[currLvl._dLevelIdx];

	pDungeonCels = LoadFileInMem(lds->dDunCels, NULL);
	pMegaTiles = LoadFileInMem(lds->dMegaTiles, NULL);
	pLevelPieces = LoadFileInMem(lds->dLvlPieces, NULL);
	pSpecialCels = LoadFileInMem(lds->dSpecCels, NULL);
}

/**
 * @param lvldir method of entry
 */
static void CreateLevel(int lvldir)
{
	switch (currLvl._dDunType) {
	case DTYPE_TOWN:
		CreateTown(lvldir);
		break;
	case DTYPE_CATHEDRAL:
		CreateL1Dungeon(lvldir);
		break;
	case DTYPE_CATACOMBS:
		CreateL2Dungeon(lvldir);
		break;
	case DTYPE_CAVES:
		CreateL3Dungeon(lvldir);
		break;
	case DTYPE_HELL:
		CreateL4Dungeon(lvldir);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	InitTriggers();
	LoadRndLvlPal();
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
		glSeedTbl[currLvl._dLevelIdx] = setseed;
#endif

	music_stop();
	//if (pcurs > CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
	//	NewCursor(CURSOR_HAND);
	//}
	//SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
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

	//SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

	IncProgress();
	InitAutomap();

	if (lvldir != ENTRY_LOAD) {
		InitLighting();
		InitVision();
	}

	InitLevelMonsters();
	IncProgress();

	SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

	FillSolidBlockTbls();
	if (!currLvl._dSetLvl) {
		CreateLevel(lvldir);
		IncProgress();
		if (currLvl._dType != DTYPE_TOWN) {
			SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
			GetLevelMTypes();
			InitThemes();
			IncProgress();
			IncProgress();
			InitObjectGFX();
			IncProgress();
		} else {
			SetupTownStores();
			IncProgress();
			IncProgress();
			IncProgress();
		}
		InitMissileGFX();
		IncProgress();
		IncProgress();

		if (lvldir == ENTRY_RTNLVL)
			GetReturnLvlPos();
		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currLvl._dLevelIdx == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag, true);
			}
		}

		PlayDungMsgs();
		IncProgress();

		SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

		if (currLvl._dType != DTYPE_TOWN) {
			HoldThemeRooms();
			InitMonsters();
			if (gbMaxPlayers != 1 || firstflag || lvldir == ENTRY_LOAD || !plr[myplr]._pLvlVisited[currLvl._dLevelIdx]) {
				IncProgress();
				InitObjects();
				InitItems();
				CreateThemeRooms();
				IncProgress();
				InitMissiles();
				InitDead();
				SavePreLighting();

				if (gbMaxPlayers != 1)
					DeltaLoadLevel();
			} else {
				InitMissiles();
				InitDead();
				IncProgress();
				LoadLevel();
			}
			IncProgress();
		} else {
			InitTowners();
			InitItems();
			InitMissiles();
			SavePreLighting();
			IncProgress();

			if (gbMaxPlayers != 1)
				DeltaLoadLevel();
			else if (!firstflag && lvldir != ENTRY_LOAD && plr[myplr]._pLvlVisited[currLvl._dLevelIdx])
				LoadLevel();

			IncProgress();
		}
		ResyncQuests();
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

		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();
		IncProgress();

		for (i = 0; i < MAX_PLRS; i++) {
			if (plr[i].plractive && currLvl._dLevelIdx == plr[i].plrlevel) {
				InitPlayerGFX(i);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(i, firstflag, true);
			}
		}

		PlayDungMsgs();

		IncProgress();
		IncProgress();

		InitMissiles();
		InitItems();
		SavePreLighting();
		if (gbMaxPlayers != 1)
			DeltaLoadLevel();
		else if (!firstflag && lvldir != ENTRY_LOAD && plr[myplr]._pLvlVisited[currLvl._dLevelIdx])
			LoadLevel();

		IncProgress();
	}

	SyncPortals();

	for (i = 0; i < MAX_PLRS; i++) {
		if (plr[i].plractive && plr[i].plrlevel == currLvl._dLevelIdx && (!plr[i]._pLvlChanging || i == myplr)) {
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

	music_start(AllLevels[currLvl._dLevelIdx].dMusic);

	while (!IncProgress())
		;
}

void EnterLevel(BYTE lvl)
{
	currLvl._dLevelIdx = lvl;
	currLvl._dLevel = AllLevels[lvl].dLevel;
	currLvl._dSetLvl = AllLevels[lvl].dSetLvl;
	currLvl._dType = AllLevels[lvl].dType;
	currLvl._dDunType = AllLevels[lvl].dDunType;
	if (gnDifficulty == DIFF_NIGHTMARE)
		currLvl._dLevel += NIGHTMARE_LEVEL_BONUS;
	else if (gnDifficulty == DIFF_NIGHTMARE)
		currLvl._dLevel += HELL_LEVEL_BONUS;
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
		assert(plr[myplr].plrlevel == currLvl._dLevelIdx + 1);
		EnterLevel(plr[myplr].plrlevel);
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
		assert(plr[myplr].plrlevel == currLvl._dLevelIdx - 1);
		EnterLevel(plr[myplr].plrlevel);
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
		EnterLevel(plr[myplr].plrlevel);
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
		EnterLevel(plr[myplr].plrlevel);
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
		EnterLevel(plr[myplr].plrlevel);
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
		EnterLevel(plr[myplr].plrlevel);
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
