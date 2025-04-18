/**
 * @file interfac.cpp
 *
 * Implementation of load screens.
 */
#include "all.h"
#include "plrctrls.h"
#include "utils/display.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

/** Cutscene image CEL */
CelImageBuf* sgpBackCel;
/** Counter to maintain the status of the level-change. */
unsigned sgdwProgress;
/** Specifies the next tick-count to draw the cutscene and progress bar. */
static Uint32 sgdwNextCut;
/** Specifies whether the progress bar is drawn on top or at the bottom of the image. */
static BOOLEAN sgbLoadBarOnTop;
/** Color of the progress bar. */
static BYTE sgbLoadBarCol;

#define BAR_STEP ((BAR_WIDTH + 16) / 17)

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
	case DVL_DWM_DYNLVL:
	case DVL_DWM_PORTLVL:
		sgpBackCel = CelLoadImage("Gendata\\Cutportl.CEL", PANEL_WIDTH);
		LoadPalette("Gendata\\Cutportl.pal");
		sgbLoadBarOnTop = FALSE;
		sgbLoadBarCol = 43;
		break;
	case DVL_DWM_NEWGAME:
	case DVL_DWM_LOADGAME:
		sgpBackCel = CelLoadImage("Gendata\\Cutstart.CEL", PANEL_WIDTH);
		LoadPalette("Gendata\\Cutstart.pal");
		sgbLoadBarOnTop = FALSE;
		sgbLoadBarCol = 43;
		break;
	case DVL_DWM_RETOWN:
		if (gbCineflag) {
			gbCineflag = false;
			DoEnding();
		}
	case DVL_DWM_TWARPDN:
	case DVL_DWM_TWARPUP:
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
	dst = &gpBuffer[BUFFERXY(screen_x, screen_y)];
	if (dst >= gpBufEnd)
		return;
	static_assert(BORDER_BOTTOM >= BAR_HEIGHT, "DrawProgress needs larger border.");
	col = sgbLoadBarCol;
	w = sgdwProgress;
	for (j = 0; j < BAR_HEIGHT; j++) {
		for (i = 0; i < w; i++, dst++) {
			*dst = col;
		}
		dst += BUFFER_WIDTH - w;
	}
#if DEBUG_MODE || DEV_MODE
	const char* progession[] {
/*  0 */"Startup",
/*  1 */"Memfree",
/*  2 */"Music stop",
/*  3 */"Init Dungeon",
/*  4 */"Init Level",
/*  5 */"Create Dungeon",
/*  6 */"MonsterFX",
/*  7 */"Monsters",
/*  8 */"ObjectsGFX",
/*  9 */"Objects/Items",
/* 10 */"Missiles/Light",
/* 11 */"Music start",
/* 12 */"Network - Pending Turns",
/* 13 */"Network - Msg Queue",
/* 14 */"Network - Join Level",
/* 15 */"Network - Sync delta",
/* 16 */"Fadeout",
	};
	static_assert(((BAR_WIDTH + lengthof(progession) - 1) / lengthof(progession)) == BAR_STEP, "Progression steps and labels are not in sync.");
	unsigned progress = sgdwProgress / BAR_STEP;
	PrintLimitedString(screen_x + 10, screen_y + (BAR_HEIGHT - SMALL_FONT_HEIGHT) / 2 + SMALL_FONT_HEIGHT, progress < (unsigned)lengthof(progession) ? progession[progress] : "Unknown", BAR_WIDTH - 20, COL_WHITE, FONT_KERN_SMALL);
#endif
}

static void DrawCutsceneBack()
{
	lock_buf(1);

	CelDraw(PANEL_X, PANEL_Y + PANEL_HEIGHT - 1, sgpBackCel, 1);

	unlock_buf(1);
}

static void RenderCutscene()
{
	Uint32 now = SDL_GetTicks();
	bool skipRender = false;
	// assert(sgdwProgress != 0);
	if (/*sgdwProgress > 0 &&*/ !SDL_TICKS_PASSED(now, sgdwNextCut)) {
		if (sgdwProgress < BAR_WIDTH)
			return; // skip drawing if the progression is too fast
		skipRender = true; // update the progress bar for fade-out
	} else {
		sgdwNextCut = now + gnRefreshDelay; // calculate the next tick to draw the cutscene
	}

	lock_buf(1);
	// if (sgdwProgress == 0)
	//	CelDraw(PANEL_X, PANEL_Y + PANEL_HEIGHT - 1, sgpBackCel, 1);

	DrawProgress();

	unlock_buf(1);

	if (!skipRender)
		scrollrt_render_screen(false);
}

void interface_msg_pump()
{
	Dvl_Event e;

	while (PeekMessage(e)) {
		//if (Msg.message != DVL_WM_QUIT) {
			DispatchMessage(&e);
		//}
	}
#if HAS_TOUCHPAD
	finish_simulated_mouse_clicks();
#endif
}

void IncProgress()
{
	interface_msg_pump();
	sgdwProgress += BAR_STEP;
	assert(sgdwProgress <= BAR_WIDTH); // || sgpBackCel == NULL
	// do not draw in case of quick-load
	// if (sgpBackCel != NULL)
		RenderCutscene();
	//return sgdwProgress >= BAR_WIDTH;
}

static void CreateDungeon()
{
	switch (currLvl._dDunType) {
	case DGT_TOWN:
		CreateTown();
		break;
	case DGT_CATHEDRAL:
		CreateL1Dungeon();
		break;
	case DGT_CATACOMBS:
		CreateL2Dungeon();
		break;
	case DGT_CAVES:
		CreateL3Dungeon();
		break;
	case DGT_HELL:
		CreateL4Dungeon();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void LoadGameLevel(int lvldir)
{
	extern int32_t sglGameSeed;
	assert(sglGameSeed == glSeedTbl[currLvl._dLevelIdx] || currLvl._dDynLvl);

	music_stop();
	//if (pcursicon > CURSOR_HAND && pcursicon < CURSOR_FIRSTITEM) {
	//	NewCursor(CURSOR_HAND);
	//}
	//SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
	IncProgress(); // "Init Dungeon" (3)
	InitLvlDungeon();
	IncProgress(); // "Init Level" (4)

	InitLvlAutomap();

	//if (lvldir != ENTRY_LOAD) {
		InitLvlLighting();
		InitLvlVision();
	//}

	InitLvlMonsters(); // reset monsters
	InitLvlObjects();  // reset objects
	InitLvlThemes();   // reset themes
	InitLvlItems();    // reset items
	IncProgress(); // "Create Dungeon" (5)

	// SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
	// fill pre: pSetPieces
	// fill in loop: dungeon, pWarps, uses drlgFlags, dungBlock
	// fill post: themeLoc, pdungeon, dPiece, dTransVal
	CreateDungeon();
	LoadLvlPalette();
	// reset: dMonster, dObject, dPlayer, dItem, dMissile, dFlags+, dLight+
	InitLvlMap();
	IncProgress(); // "MonsterFX" (6)
	if (currLvl._dType != DTYPE_TOWN) {
		GetLevelMTypes(); // select monster types and load their fx
		InitThemes();     // protect themes with dFlags and select theme types
		IncProgress(); // "Monsters" (7)
		InitMonsters();   // place monsters
	} else {
		InitLvlStores();
		// TODO: might want to reset RndSeed, since InitLvlStores is player dependent, but it does not matter at the moment
		// SetRndSeed(glSeedTbl[currLvl._dLevelIdx]);
		IncProgress(); // "Monsters" (7)
		InitTowners();
	}
	IncProgress(); // "ObjectsGFX" (8)
	InitObjectGFX();    // load object graphics
	IncProgress(); // "Objects/Items" (9)
	InitObjects();      // place objects
	InitItems();        // place items
	CreateThemeRooms(); // populate theme rooms
	FreeSetPieces();
	IncProgress(); // "Missiles/Light" (10)
	InitMissiles();
	SavePreLighting();
	InitView(lvldir);

	IncProgress(); // "Music start" (11)

	music_start(AllLevels[currLvl._dLevelNum].dMusic);
}

void EnterLevel(BYTE lvl)
{
	int lvlBonus;

	SetRndSeed(glSeedTbl[lvl]);
	currLvl._dLevelPlyrs = IsMultiGame ? gsDeltaData.ddLevelPlrs[lvl] : 1;
	currLvl._dLevelIdx = lvl;
	currLvl._dDynLvl = lvl >= NUM_FIXLVLS;
	if (currLvl._dDynLvl) {
		// select level
		unsigned baseLevel = gDynLevels[lvl - NUM_FIXLVLS]._dnLevel;
		unsigned leveltype = gDynLevels[lvl - NUM_FIXLVLS]._dnType;
		// assert(baseLevel + HELL_LEVEL_BONUS < CF_LEVEL);
		int availableLvls[NUM_FIXLVLS];
		int numLvls = 0;
		for (int i = DLV_CATHEDRAL1; i < NUM_STDLVLS; i++) {
			if (AllLevels[i].dLevel <= baseLevel && (leveltype == DLV_TOWN || leveltype == AllLevels[i].dType) /*&& AllLevels[i].dMonTypes[0] != MT_INVALID*/) {
				availableLvls[numLvls] = i;
				numLvls++;
			}
		}
		lvl = DLV_CATHEDRAL1;
		if (numLvls != 0) {
			lvl = availableLvls[random_low(141, numLvls)];
		} else {
			baseLevel = AllLevels[DLV_CATHEDRAL1].dLevel;
		}
		currLvl._dLevelNum = lvl;
		currLvl._dLevel = AllLevels[lvl].dLevel;
		currLvl._dSetLvl = false; // AllLevels[lvl].dSetLvl;
		currLvl._dType = AllLevels[lvl].dType;
		currLvl._dDunType = AllLevels[lvl].dDunType;
		lvlBonus = baseLevel - AllLevels[lvl].dLevel;
	} else {
		currLvl._dLevelNum = lvl;
		currLvl._dLevel = AllLevels[lvl].dLevel;
		currLvl._dSetLvl = AllLevels[lvl].dSetLvl;
		currLvl._dType = AllLevels[lvl].dType;
		currLvl._dDunType = AllLevels[lvl].dDunType;
		lvlBonus = 0;
	}
	if (gnDifficulty == DIFF_NIGHTMARE) {
		lvlBonus += NIGHTMARE_LEVEL_BONUS;
	} else if (gnDifficulty == DIFF_HELL) {
		lvlBonus += HELL_LEVEL_BONUS;
	}
	currLvl._dLevelBonus = lvlBonus;
	currLvl._dLevel += lvlBonus;
}

/*
 * Load Game          Load In-Game             Single Game               Multi Game
 *
 * LoadGame           LoadGame                 LoadGameLevel             LoadGameLevel
 *  LoadGameLevel      LoadGameLevel           nthread_finish            nthread_finish
 *  Resync             Resync                   Resync                    Resync
 *   ChangeMap          ChangeMap                ChangeMap                 ChangeMap
 *  LoadLevelData      LoadLevelData            LoadLevel                 DeltaLoadLevel
 *   ChangeMap     	    ChangeMap                LoadLevelData             ChangeMap
 *                                                ChangeMap               LevelDeltaLoad
 *                                              ProcessLightList          ProcessLightList
 *                                              ProcessVisionList         ProcessVisionList
 */
void ShowCutscene(unsigned uMsg)
{
	WNDPROC saveProc;

	nthread_run();
	static_assert((unsigned)DVL_DWM_LOADGAME == (unsigned)DVL_DWM_NEWGAME + 1 && (unsigned)NUM_WNDMSGS == (unsigned)DVL_DWM_LOADGAME + 1, "Check to save hero/level in ShowCutscene must be adjusted.");
	if (uMsg < DVL_DWM_NEWGAME) {
		pfile_update(true);
		if (IsMultiGame) {
			DeltaSaveLevel();
		} else {
			SaveLevel();
		}
		// turned off to have a consistent fade in/out logic + reduces de-sync by
		// eliminating the need for special handling in InitLevelChange (player.cpp)
		//PaletteFadeOut();
	}

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(DisableInputWndProc);
	assert(saveProc == GameWndProc);
	interface_msg_pump();
	// scrollrt_render_screen(false); -- unnecessary, because it is going to be updated/presented by DrawCutsceneBack/PaletteFadeIn
	InitCutscene(uMsg);
	ClearScreenBuffer(); // must be after InitCutscene in case gbCineflag is set to clear after video-playback
	// SetFadeLevel(0); // -- unnecessary, PaletteFadeIn starts with fade-level 0 anyway
	DrawCutsceneBack();
	PaletteFadeIn(false);
	sgdwNextCut = SDL_GetTicks() + gnRefreshDelay; // calculate the next tick to draw the cutscene
	IncProgress(); // "Memfree" (1)
	FreeLevelMem();
	IncProgress(); // "Music stop" (2)

	if (uMsg != DVL_DWM_LOADGAME) {
		int lvldir = ENTRY_MAIN;
		if (uMsg != DVL_DWM_NEWGAME) {
			lvldir += (uMsg - DVL_DWM_NEXTLVL);
			if (uMsg == DVL_DWM_NEXTLVL)
				assert(myplr._pDunLevel == currLvl._dLevelIdx + 1);
			if (uMsg == DVL_DWM_PREVLVL)
				assert(myplr._pDunLevel == currLvl._dLevelIdx - 1);
			static_assert((int)DVL_DWM_NEXTLVL - (int)DVL_DWM_NEXTLVL == (int)ENTRY_MAIN    - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted I.");
			static_assert((int)DVL_DWM_PREVLVL - (int)DVL_DWM_NEXTLVL == (int)ENTRY_PREV    - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted II.");
			static_assert((int)DVL_DWM_SETLVL  - (int)DVL_DWM_NEXTLVL == (int)ENTRY_SETLVL  - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted III.");
			static_assert((int)DVL_DWM_RTNLVL  - (int)DVL_DWM_NEXTLVL == (int)ENTRY_RTNLVL  - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted IV.");
			static_assert((int)DVL_DWM_DYNLVL  - (int)DVL_DWM_NEXTLVL == (int)ENTRY_DYNLVL  - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted V.");
			static_assert((int)DVL_DWM_PORTLVL - (int)DVL_DWM_NEXTLVL == (int)ENTRY_PORTLVL - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted VI.");
			static_assert((int)DVL_DWM_TWARPDN - (int)DVL_DWM_NEXTLVL == (int)ENTRY_TWARPDN - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted VII.");
			static_assert((int)DVL_DWM_TWARPUP - (int)DVL_DWM_NEXTLVL == (int)ENTRY_TWARPUP - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted VIII.");
			static_assert((int)DVL_DWM_RETOWN  - (int)DVL_DWM_NEXTLVL == (int)ENTRY_RETOWN  - (int)ENTRY_MAIN, "Conversion from DVL_DWM_* to ENTRY_* in ShowCutscene must be adjusted IX.");
		}
		EnterLevel(myplr._pDunLevel);
		LoadGameLevel(lvldir);
	} else {
		LoadGame();
	}
	IncProgress(); // "Network" (13..15)
	// process packets arrived during LoadLevel / delta-load and disable nthread
	nthread_finish(uMsg);

	if (IsLocalGame) { // do not block other players
		sgdwProgress = BAR_WIDTH - BAR_STEP;
		IncProgress(); // "Fadeout" (16)
		PaletteFadeOut();
		// skip time due to fadein/out
		extern Uint32 guNextTick;
		guNextTick = SDL_GetTicks() + gnTickDelay; // += (uMsg < DVL_DWM_NEWGAME ? 2 : 1) * FADE_LEVELS;
	}
	FreeCutscene();

	assert(ghMainWnd != NULL);
	saveProc = SetWindowProc(GameWndProc); // saveProc);
	assert(saveProc == DisableInputWndProc);
}

DEVILUTION_END_NAMESPACE
