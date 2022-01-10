/**
 * @file interfac.cpp
 *
 * Implementation of load screens.
 */
#include "all.h"
#ifndef NOWIDESCREEN
#include "DiabloUI/diabloui.h"
#include "DiabloUI/art_draw.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

/** The starting x-position of the progress bar. */
#define BAR_POS_X		53
/** The width of the progress bar. */
#define BAR_WIDTH		PANEL_WIDTH - BAR_POS_X * 2

/** Cutscene image CEL */
BYTE* sgpBackCel;

unsigned sgdwProgress;

/** Specifies whether the progress bar is drawn on top or at the bottom of the image. */
static BOOLEAN sgbLoadBarOnTop;
/** Color of the progress bar. */
static BYTE sgbLoadBarCol;

static void FreeCutscene()
{
	MemFreeDbg(sgpBackCel);
#ifndef NOWIDESCREEN
	ArtBackgroundWidescreen.Unload();
#endif
}

static void InitLvlCutscene(BYTE lvl)
{
	sgbLoadBarOnTop = AllLevels[lvl].dLoadBarOnTop;
	sgbLoadBarCol = AllLevels[lvl].dLoadBarColor;
	sgpBackCel = LoadFileInMem(AllLevels[lvl].dLoadCels);
	LoadPalette(AllLevels[lvl].dLoadPal);
}

static void InitCutscene(unsigned int uMsg)
{
	BYTE lvl;
	assert(sgpBackCel == NULL);

	switch (uMsg) {
	case DVL_DWM_NEXTLVL:
		lvl = currLvl._dLevelIdx;
		assert(currLvl._dType == AllLevels[lvl].dType);
		if (lvl == DLV_HELL3)
			lvl = DLV_HELL4; // the destination level
		InitLvlCutscene(lvl);
		break;
	case DVL_DWM_PREVLVL:
		lvl = currLvl._dLevelIdx;
		InitLvlCutscene(lvl);
		assert(currLvl._dType == AllLevels[lvl].dType);
		break;
	case DVL_DWM_SETLVL:
		lvl = myplr._pDunLevel; // the destination level
		InitLvlCutscene(lvl);
		break;
	case DVL_DWM_RTNLVL:
		lvl = currLvl._dLevelIdx;
		InitLvlCutscene(lvl);
		break;
	case DVL_DWM_WARPLVL:
#ifndef NOWIDESCREEN
		LoadArt("Gendata\\Cutportlw.pcx", &ArtBackgroundWidescreen);
#endif
		sgpBackCel = LoadFileInMem("Gendata\\Cutportl.CEL");
		LoadPalette("Gendata\\Cutportl.pal");
		sgbLoadBarOnTop = FALSE;
		sgbLoadBarCol = 43;
		break;
	case DVL_DWM_NEWGAME:
#ifndef NOWIDESCREEN
		LoadArt("Gendata\\Cutstartw.pcx", &ArtBackgroundWidescreen);
#endif
		sgpBackCel = LoadFileInMem("Gendata\\Cutstart.CEL");
		LoadPalette("Gendata\\Cutstart.pal");
		sgbLoadBarOnTop = FALSE;
		sgbLoadBarCol = 43;
		break;
	case DVL_DWM_TWARPDN:
	case DVL_DWM_TWARPUP:
	case DVL_DWM_RETOWN:
		lvl = myplr._pDunLevel; // the destination level
		InitLvlCutscene(lvl);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	sgdwProgress = 0;
}

static void DrawProgress()
{
	BYTE *dst, col;
	int screen_x, screen_y;
	unsigned w, i, j;

	screen_x = PANEL_X + BAR_POS_X;
	screen_y = SCREEN_Y + UI_OFFSET_Y + (sgbLoadBarOnTop ? 37 : 421);
	dst = &gpBuffer[screen_x + BUFFER_WIDTH * screen_y];
	col = sgbLoadBarCol;
	w = sgdwProgress;
	for (j = 0; j < 22; j++) {
		for (i = 0; i < w; i++, dst++) {
			*dst = col;
		}
		dst += BUFFER_WIDTH - w;
	}
}

static void DrawCutscene()
{
	lock_buf(1);
#ifndef NOWIDESCREEN
	DrawArt(PANEL_X - (ArtBackgroundWidescreen.logical_width - PANEL_WIDTH) / 2, SCREEN_Y + UI_OFFSET_Y, &ArtBackgroundWidescreen);
#endif
	CelDraw(PANEL_X, 480 + SCREEN_Y - 1 + UI_OFFSET_Y, sgpBackCel, 1, 640);

	DrawProgress();

	unlock_buf(1);
	scrollrt_draw_screen(false);
}

void interface_msg_pump()
{
	MSG Msg;

	while (PeekMessage(&Msg)) {
		//if (Msg.message != DVL_WM_QUIT) {
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		//}
	}
}

bool IncProgress()
{
	interface_msg_pump();
	sgdwProgress += 23;
	if (sgdwProgress > BAR_WIDTH)
		sgdwProgress = BAR_WIDTH;
	// do not draw in case of quick-load
	if (sgpBackCel != NULL)
		DrawCutscene();
	return sgdwProgress >= BAR_WIDTH;
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
#if DEBUG_MODE
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
	InitLvlDungeon();
	IncProgress();

	InitLvlAutomap();

	if (lvldir != ENTRY_LOAD) {
		InitLighting();
		InitVision();
	}

	InitLevelMonsters();
	IncProgress();

	SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

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
			InitLvlStores();
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

	ResyncQuests();
	if (IsMultiGame)
		DeltaLoadLevel();
	else if (lvldir != ENTRY_LOAD && IsLvlVisited(currLvl._dLevelIdx)) {
		LoadLevel();
	}

	IncProgress();

	InitSync();
	SyncPortals();

	IncProgress();
	if (lvldir != ENTRY_LOAD)
		InitPlayers();
	PlayDungMsgs();

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
	else if (gnDifficulty == DIFF_HELL)
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

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(DisableInputWndProc);

	interface_msg_pump();
	ClearScreenBuffer();
	scrollrt_draw_screen(false);
	InitCutscene(uMsg);
	SetFadeLevel(0); // TODO: set _gbFadedIn to false?
	DrawCutscene();
	PaletteFadeIn(false);
	IncProgress();

	switch (uMsg) {
	case DVL_DWM_NEWGAME:
		IncProgress();
		if (gbLoadGame/*&& gbValidSaveFile*/) {
			LoadGame();
		} else {
			//FreeLevelMem();
			pfile_delete_save_file(false);
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
}

DEVILUTION_END_NAMESPACE
