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

static void FreeCutscene()
{
	MemFreeDbg(sgpBackCel);
}

static void InitCutscene(unsigned int uMsg)
{
	int lvl;
	assert(sgpBackCel == NULL);

	switch (uMsg) {
	case DVL_DWM_NEXTLVL:
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
			lvl = myplr._pDunLevel; // the destination level
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
	case DVL_DWM_PREVLVL:
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
	case DVL_DWM_SETLVL:
		lvl = myplr._pDunLevel; // the destination level
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
	case DVL_DWM_RTNLVL:
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
	case DVL_DWM_WARPLVL:
		sgpBackCel = LoadFileInMem("Gendata\\Cutportl.CEL");
		LoadPalette("Gendata\\Cutportl.pal");
		progress_id = 1;
		break;
	case DVL_DWM_NEWGAME:
		sgpBackCel = LoadFileInMem("Gendata\\Cutstart.CEL");
		LoadPalette("Gendata\\Cutstart.pal");
		progress_id = 1;
		break;
	case DVL_DWM_TWARPDN:
	case DVL_DWM_TWARPUP:
	case DVL_DWM_RETOWN: {
		lvl = myplr._pDunLevel; // the destination level
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
	scrollrt_draw_screen(false);
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

static void InitPlayers()
{
	int pnum;

	for (pnum = 0; pnum < MAX_PLRS; pnum++) {
		if (!plr._pActive || currLvl._dLevelIdx != plr._pDunLevel || plr._pLvlChanging)
			continue;
		InitLvlPlayer(pnum);
	}
}

void LoadGameLevel(int lvldir)
{
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

	InitLvlAutomap();

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
			InitStores();
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

		SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

		if (currLvl._dType != DTYPE_TOWN) {
			HoldThemeRooms();
			InitMonsters();
			IncProgress();
			if (IsMultiGame || lvldir == ENTRY_LOAD || !IsLvlVisited(currLvl._dLevelIdx)) {
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

		IncProgress();

		InitItems();
	}
	InitMissiles();
	SavePreLighting();
	if (IsMultiGame)
		DeltaLoadLevel();
	else if (lvldir != ENTRY_LOAD && IsLvlVisited(currLvl._dLevelIdx)) {
		LoadLevel();
	}

	IncProgress();

	InitSync();
	ResyncQuests();
	SyncPortals();

	IncProgress();
	if (lvldir != ENTRY_LOAD)
		InitPlayers();
	PlayDungMsgs();

	SetDungeonMicros(0, 0, MAXDUNX, MAXDUNY);

	IncProgress();
	IncProgress();
	IncProgress();
	ProcessLightList();
	ProcessVisionList();

	guLvlVisited |= LEVEL_MASK(currLvl._dLevelIdx);

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
	EnterLevel(myplr._pDunLevel);
	IncProgress();
	LoadGameLevel(lvldir);
}

void ShowCutscene(unsigned uMsg)
{
	WNDPROC saveProc;

	gbSomebodyWonGameKludge = false;

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(DisableInputWndProc);

	interface_msg_pump();
	ClearScreenBuffer();
	scrollrt_draw_screen(false);
	InitCutscene(uMsg);
	BlackPalette();
	DrawCutscene();
	PaletteFadeIn();
	IncProgress();
	sound_init();
	IncProgress();

	switch (uMsg) {
	case DVL_DWM_NEWGAME:
		IncProgress();
		if (gbLoadGame/*&& gbValidSaveFile*/) {
			LoadGame();
		} else {
			//FreeLevelMem();
			pfile_remove_temp_files();
			LoadGameLevel(ENTRY_MAIN);
		}
		IncProgress();
		break;
	case DVL_DWM_NEXTLVL:
		assert(myplr._pDunLevel == currLvl._dLevelIdx + 1);
		SwitchGameLevel(ENTRY_MAIN);
		break;
	case DVL_DWM_PREVLVL:
		assert(myplr._pDunLevel == currLvl._dLevelIdx - 1);
		SwitchGameLevel(ENTRY_PREV);
		break;
	case DVL_DWM_SETLVL:
		SetReturnLvlPos();
		SwitchGameLevel(ENTRY_SETLVL);
		break;
	case DVL_DWM_RTNLVL:
		assert(myplr._pDunLevel == gnReturnLvl);
		SwitchGameLevel(ENTRY_RTNLVL);
		break;
	case DVL_DWM_WARPLVL:
		SwitchGameLevel(ENTRY_WARPLVL);
		break;
	case DVL_DWM_TWARPDN:
		SwitchGameLevel(ENTRY_TWARPDN);
		break;
	case DVL_DWM_TWARPUP:
		SwitchGameLevel(ENTRY_TWARPUP);
		break;
	case DVL_DWM_RETOWN:
		SwitchGameLevel(ENTRY_MAIN);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	IncProgress();
	assert(ghMainWnd != NULL);

	PaletteFadeOut();
	FreeCutscene();

	saveProc = SetWindowProc(saveProc);
	assert(saveProc == DisableInputWndProc);

	if (gbSomebodyWonGameKludge && myplr._pDunLevel == DLV_HELL4) {
		PrepDoEnding(gbSoundOn);
	}
}

DEVILUTION_END_NAMESPACE
