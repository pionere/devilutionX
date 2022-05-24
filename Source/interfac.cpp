/**
 * @file interfac.cpp
 *
 * Implementation of load screens.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

/** Cutscene image CEL */
CelImageBuf* sgpBackCel;

unsigned sgdwProgress;

/** Specifies whether the progress bar is drawn on top or at the bottom of the image. */
static BOOLEAN sgbLoadBarOnTop;
/** Color of the progress bar. */
static BYTE sgbLoadBarCol;

static void FreeCutscene()
{
	MemFreeDbg(sgpBackCel);
}

static void InitLvlCutscene(BYTE lvl)
{
	sgbLoadBarOnTop = AllLevels[lvl].dLoadBarOnTop;
	sgbLoadBarCol = AllLevels[lvl].dLoadBarColor;
	sgpBackCel = CelLoadImage(AllLevels[lvl].dLoadCels, PANEL_WIDTH);
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
		sgpBackCel = CelLoadImage("Gendata\\Cutportl.CEL", PANEL_WIDTH);
		LoadPalette("Gendata\\Cutportl.pal");
		sgbLoadBarOnTop = FALSE;
		sgbLoadBarCol = 43;
		break;
	case DVL_DWM_NEWGAME:
		sgpBackCel = CelLoadImage("Gendata\\Cutstart.CEL", PANEL_WIDTH);
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

	screen_x = PANEL_CENTERX(BAR_WIDTH);
	screen_y = PANEL_Y + (sgbLoadBarOnTop ? TOP_BAR_Y : BOTTOM_BAR_Y);
	dst = &gpBuffer[screen_x + BUFFER_WIDTH * screen_y];
	col = sgbLoadBarCol;
	w = sgdwProgress;
	for (j = 0; j < BAR_HEIGHT; j++) {
		for (i = 0; i < w; i++, dst++) {
			*dst = col;
		}
		dst += BUFFER_WIDTH - w;
	}
}

static void DrawCutscene()
{
	lock_buf(1);
	CelDraw(PANEL_X, PANEL_Y + PANEL_HEIGHT - 1, sgpBackCel, 1);

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

void IncProgress()
{
	interface_msg_pump();
	sgdwProgress += (BAR_WIDTH + 17) / 18;
	if (sgdwProgress > BAR_WIDTH)
		sgdwProgress = BAR_WIDTH;
	// do not draw in case of quick-load
	if (sgpBackCel != NULL)
		DrawCutscene();
	//return sgdwProgress >= BAR_WIDTH;
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

void LoadGameLevel(int lvldir)
{
#if DEBUG_MODE
	if (setseed)
		glSeedTbl[currLvl._dLevelIdx] = setseed;
#endif

	music_stop();
	//if (pcursicon > CURSOR_HAND && pcursicon < CURSOR_FIRSTITEM) {
	//	NewCursor(CURSOR_HAND);
	//}
	//SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
	IncProgress();
	InitLvlDungeon();
	IncProgress();

	InitLvlAutomap();

	//if (lvldir != ENTRY_LOAD) {
		InitLighting();
		InitVision();
	//}

	InitLevelMonsters();
	InitLevelObjects();
	IncProgress();

	SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);

	if (!currLvl._dSetLvl) {
		CreateLevel(lvldir);
		IncProgress();
		if (currLvl._dType != DTYPE_TOWN) {
			GetLevelMTypes();
			InitThemes();
			IncProgress();
			InitObjectGFX();
		} else {
			InitLvlStores();
			// TODO: might want to reset RndSeed, since InitLvlStores is player dependent, but it does not matter at the moment
			// SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
			IncProgress();
		}
		IncProgress();
		IncProgress();

		if (lvldir == ENTRY_RTNLVL)
			GetReturnLvlPos();
		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		if (currLvl._dType != DTYPE_TOWN) {
			HoldThemeRooms();
			InitMonsters();
			IncProgress();
			if (IsMultiGame || lvldir == ENTRY_LOAD || !IsLvlVisited(currLvl._dLevelIdx)) {
				InitObjects();
				InitItems();
				CreateThemeRooms();
			}
		} else {
			InitTowners();
			IncProgress();
			InitItems();
		}
	} else {
		LoadSetMap();
		IncProgress();
		GetLevelMTypes();
		IncProgress();
		InitMonsters();
		IncProgress();
		IncProgress();
		IncProgress();

		if (lvldir == ENTRY_WARPLVL)
			GetPortalLvlPos();

		InitItems();
	}
	IncProgress();
	InitMissiles();
	SavePreLighting();

	IncProgress();

	if (!IsMultiGame) {
		ResyncQuests();
		if (lvldir != ENTRY_LOAD && IsLvlVisited(currLvl._dLevelIdx)) {
			LoadLevel();
		}
		//SyncPortals();
	}
	IncProgress();
	InitSync();
	PlayDungMsgs();

	guLvlVisited |= LEVEL_MASK(currLvl._dLevelIdx);

	music_start(AllLevels[currLvl._dLevelIdx].dMusic);
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

	nthread_run();
	if (uMsg != DVL_DWM_NEWGAME) {
		if (IsMultiGame)
			pfile_write_hero(false);
		// turned off to have a consistent fade in/out logic + reduces de-sync by 
		// eliminating the need for special handling in InitLevelChange (player.cpp)
		//PaletteFadeOut();
	}

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
		IncProgress();
		if (gbLoadGame/*&& gbValidSaveFile*/) {
			LoadGame();
		} else {
			//FreeLevelMem();
			pfile_delete_save_file(false);
			LoadGameLevel(ENTRY_MAIN);
		}
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
	// process packets arrived during LoadLevel / delta-load and disable nthread
	nthread_finish(uMsg);

	if (IsLocalGame) { // do not block other players
		sgdwProgress = BAR_WIDTH;
		IncProgress();
		PaletteFadeOut();
	}
	FreeCutscene();

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(saveProc);
	assert(saveProc == DisableInputWndProc);
}

DEVILUTION_END_NAMESPACE
