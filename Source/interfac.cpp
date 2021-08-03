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
			lvl = myplr.plrlevel; // the destination level
			progress_id = 1;
			break;
		case DTYPE_CRYPT:
		case DTYPE_NEST:
			progress_id = 1;
			break;
		default:
			ASSUME_UNREACHABLE
		}
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels);
		LoadPalette(AllLevels[lvl].dLoadPal);
		break;
	case WM_DIABPREVLVL:
		lvl = currLvl._dLevelIdx;
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels);
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
		lvl = myplr.plrlevel; // the destination level
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels);
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
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels);
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
		sgpBackCel = LoadFileInMem("Gendata\\Cutportl.CEL");
		LoadPalette("Gendata\\Cutportl.pal");
		progress_id = 1;
		break;
	case WM_DIABLOADGAME:
	case WM_DIABNEWGAME:
		sgpBackCel = LoadFileInMem("Gendata\\Cutstart.CEL");
		LoadPalette("Gendata\\Cutstart.pal");
		progress_id = 1;
		break;
	case WM_DIABTWARPDN:
	case WM_DIABTWARPUP:
	case WM_DIABRETOWN: {
		lvl = myplr.plrlevel; // the destination level
		sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels);
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
	const LevelData *lds;
	assert(pDungeonCels == NULL);
	lds = &AllLevels[currLvl._dLevelIdx];

	pDungeonCels = LoadFileInMem(lds->dDunCels);
	pMegaTiles = LoadFileInMem(lds->dMegaTiles);
	pLevelPieces = (uint16_t *)LoadFileInMem(lds->dLvlPieces);
	pSpecialCels = LoadFileInMem(lds->dSpecCels);
}

void FreeLvlGFX()
{
	MemFreeDbg(pDungeonCels);
	MemFreeDbg(pMegaTiles);
	MemFreeDbg(pLevelPieces);
	MemFreeDbg(pSpecialCels);
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
	int pnum;

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
	InitLightMax();
	MakeLightTable();
	LoadLvlGFX();
	IncProgress();

	if (firstflag) {
		// initialize values which are not stored in savefiles
		InitHelp(); // values

		InitInv(); // gfx + values
		InitItemGFX(); // gfx + values (some stored in savefiles)
		InitQuestGfx(); // values
		InitQuestText(); // gfx + values

		for (pnum = 0; pnum < (IsLocalGame ? 1 : MAX_PLRS); pnum++)
			InitPlrGFXMem(pnum); // gfx

		InitStores(); // gfx + values (some stored in savefiles)
		InitText(); // gfx
		InitControlPan(); // gfx + values
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
			IncProgress();
			InitThemes();
			IncProgress();
			InitObjectGFX();
		} else {
			SetupTownStores();
			IncProgress();
			IncProgress();
		}
		IncProgress();
		InitMissileGFX();
		IncProgress();

		if (lvldir == ENTRY_RTNLVL)
			GetReturnLvlPos();
		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		IncProgress();

		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr.plractive && currLvl._dLevelIdx == plr.plrlevel) {
				InitPlayerGFX(pnum);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(pnum, firstflag, true);
			}
		}

		PlayDungMsgs();
		IncProgress();

		SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

		if (currLvl._dType != DTYPE_TOWN) {
			HoldThemeRooms();
			InitMonsters();
			IncProgress();
			if (IsMultiGame || firstflag || lvldir == ENTRY_LOAD || !myplr._pLvlVisited[currLvl._dLevelIdx]) {
				InitObjects();
				InitItems();
				CreateThemeRooms();
			}
			IncProgress();
			InitDead();
		} else {
			InitTowners();
			IncProgress();
			InitItems();
			IncProgress();
		}
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

		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr.plractive && currLvl._dLevelIdx == plr.plrlevel) {
				InitPlayerGFX(pnum);
				if (lvldir != ENTRY_LOAD)
					InitPlayer(pnum, firstflag, true);
			}
		}

		PlayDungMsgs();

		IncProgress();
		IncProgress();

		InitItems();
	}
	InitMissiles();
	SavePreLighting();
	if (IsMultiGame)
		DeltaLoadLevel();
	else if (!firstflag && lvldir != ENTRY_LOAD && myplr._pLvlVisited[currLvl._dLevelIdx])
		LoadLevel();

	IncProgress();

	InitSync();
	ResyncQuests();
	SyncPortals();

	if (lvldir != ENTRY_LOAD) {
		for (pnum = 0; pnum < MAX_PLRS; pnum++) {
			if (plr.plractive && plr.plrlevel == currLvl._dLevelIdx && (!plr._pLvlChanging || pnum == mypnum)) {
				if (plr._pHitPoints >= (1 << 6)) {
					/*if (!IsMultiGame)
						dPlayer[plr._px][plr._py] = pnum + 1;
					else*/
						SyncInitPlrPos(pnum);
				} else {
					dFlags[plr._px][plr._py] |= BFLAG_DEAD_PLAYER;
				}
			}
		}
	}

	SetDungeonMicros(0, 0, MAXDUNX, MAXDUNY);

	IncProgress();
	IncProgress();
	IncProgress();
	ProcessLightList();
	ProcessVisionList();

	myplr._pLvlVisited[currLvl._dLevelIdx] = TRUE;

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

static void SwitchGameLevel(int lvldir)
{
	if (IsMultiGame) {
		DeltaSaveLevel();
	} else {
		SaveLevel();
	}
	IncProgress();
	FreeLevelMem();
	EnterLevel(myplr.plrlevel);
	IncProgress();
	LoadGameLevel(false, lvldir);
}

void ShowProgress(unsigned int uMsg)
{
	WNDPROC saveProc;

	gbSomebodyWonGameKludge = false;

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
		LoadGame(true);
		IncProgress();
		break;
	case WM_DIABNEWGAME:
		FreeLevelMem();
		IncProgress();
		pfile_remove_temp_files();
		IncProgress();
		LoadGameLevel(true, ENTRY_MAIN);
		break;
	case WM_DIABNEXTLVL:
		assert(myplr.plrlevel == currLvl._dLevelIdx + 1);
		SwitchGameLevel(ENTRY_MAIN);
		break;
	case WM_DIABPREVLVL:
		assert(myplr.plrlevel == currLvl._dLevelIdx - 1);
		SwitchGameLevel(ENTRY_PREV);
		break;
	case WM_DIABSETLVL:
		SetReturnLvlPos();
		SwitchGameLevel(ENTRY_SETLVL);
		break;
	case WM_DIABRTNLVL:
		assert(myplr.plrlevel == ReturnLvl);
		SwitchGameLevel(ENTRY_RTNLVL);
		break;
	case WM_DIABWARPLVL:
		UseCurrentPortal();
		SwitchGameLevel(ENTRY_WARPLVL);
		break;
	case WM_DIABTWARPDN:
		SwitchGameLevel(ENTRY_TWARPDN);
		break;
	case WM_DIABTWARPUP:
		SwitchGameLevel(ENTRY_TWARPUP);
		break;
	case WM_DIABRETOWN:
		SwitchGameLevel(ENTRY_MAIN);
		break;
	}
	IncProgress();
	assert(ghMainWnd != NULL);

	PaletteFadeOut();
	FreeInterface();

	saveProc = SetWindowProc(saveProc);
	assert(saveProc == DisableInputWndProc);

	if (gbSomebodyWonGameKludge && myplr.plrlevel == DLV_HELL4) {
		PrepDoEnding(gbSoundOn);
	}
}

DEVILUTION_END_NAMESPACE
