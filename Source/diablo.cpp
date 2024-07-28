/**
 * @file diablo.cpp
 *
 * Implementation of the main game initialization functions.
 */
#include "all.h"
#include "engine/render/text_render.h"
#include "utils/display.h"
#include "utils/paths.h"
#include "utils/screen_reader.hpp"
#include "diabloui.h"
#include "plrctrls.h"
#include "storm/storm_cfg.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef GPERF_HEAP_FIRST_GAME_ITERATION
#include <gperftools/heap-profiler.h>
#endif

/** The X/Y-coordinate of the mouse on the screen. */
POS32 MousePos;
/** Controlls whether the main game-loop should run. */
bool gbRunGame;
/** Specifies whether the application should go to the menu after leaving a game. */
bool gbRunGameResult;
/** Specifies whether the view is zoomed in. */
bool gbZoomInFlag;
/** Specifies whether the ending cinematics should be played before the cutscene. */
bool gbCineflag;
/** The state of the game-logic progession. */
BYTE gbGameLogicProgress = GLP_NONE;
/** Specifies which part of the screen should be redrawn. */
int gbRedrawFlags;
bool gbGamePaused;
/** Specifies the 'dead' state of the local player (MYPLR_DEATH_MODE). */
BYTE gbDeathflag = MDM_ALIVE;
/** Specifies whether the main action button is pressed. */
bool gbActionBtnDown;
/** Specifies whether the secondary action button is pressed. */
bool gbAltActionBtnDown;
/** tick counter when the last time one of the mouse-buttons were pressed down. */
static Uint32 guLastABD, guLastAABD;
static int actionBtnKey, altActionBtnKey;
/** Specifies the speed of the game. */
int gnTicksRate = SPEED_NORMAL;
unsigned gnTickDelay = 1000 / SPEED_NORMAL;
/** Cursor before a timeout happened. */
int gnTimeoutCurs;
/** Specifies whether the intro should be skipped at startup. */
static bool _gbSkipIntro = false;
/** Specifies whether the in-game tooltip is always active. */
bool gbShowTooltip = false;
/** Default controls. */
// clang-format off
BYTE WMButtonInputTransTbl[] = { ACT_NONE,
// LBUTTON, RBUTTON,    CANCEL,   MBUTTON,  X1BUTTON, X2BUTTON, UNDEF,    BACK,     TAB,         UNDEF,
  ACT_ACT,  ACT_ALTACT, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_AUTOMAP, ACT_NONE,
// UNDEF,   CLEAR,    RETURN,     UNDEF,    UNDEF,    SHIFT,    CONTROL,  ALT,      PAUSE,     CAPS,
  ACT_NONE, ACT_NONE, ACT_RETURN, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_PAUSE, ACT_NONE,
// KANAH,   IMEON,    JUNJA,    FINAL,    HANJA,    IMEOFF,   ESCAPE,     CONVERT,  NONCONV,  ACCEPT,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_ESCAPE, ACT_NONE, ACT_NONE, ACT_NONE,
// CHANGE,  SPACE,       PGUP,     PGDOWN,     END,      HOME,     LEFT,     UP,     RIGHT,     DOWN,
  ACT_NONE, ACT_CLEARUI, ACT_PGUP, ACT_PGDOWN, ACT_NONE, ACT_NONE, ACT_LEFT, ACT_UP, ACT_RIGHT, ACT_DOWN,
// SELECT,  PRINT,    EXEC,     PRINTSCRN, INSERT,  DELETE,   HELP,     0,        1,         2,       
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_ITEM0, ACT_ITEM1,
// 3,        4,         5,         6,         7,         8,         9,        UNDEF,    UNDEF,    UNDEF,
  ACT_ITEM2, ACT_ITEM3, ACT_ITEM4, ACT_ITEM5, ACT_ITEM6, ACT_ITEM7, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    A,        B,           C,        D,        E,        F,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_SKL0, ACT_SKLBOOK, ACT_CHAR, ACT_SKL2, ACT_SKL6, ACT_SKL3,
// G,            H,                I,       J,        K,        L,        M,        N,        O,        P,
  ACT_GAMMA_INC, ACT_GAMMA_DEC, ACT_INV, ACT_NONE, ACT_NONE, ACT_SKLLIST, ACT_TEAM, ACT_NONE, ACT_VER, ACT_PAUSE,
// Q,         R,        S,           T,           U,        V,       W,        X,        Y,        Z,
  ACT_SKL4, ACT_SKL7, ACT_SKL1, ACT_TOOLTIP, ACT_QUESTS, ACT_TGT, ACT_SKL5, ACT_SWAP, ACT_NONE, ACT_ZOOM,
// LWIN,    RWIN,     APPS,     UNDEF,    SLEEP,    NUM0,     NUM1,     NUM2,     NUM3,     NUM4,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// NUM5,    NUM6,     NUM7,     NUM8,     NUM9,     MULT,     ADD,         SEP,      SUB,          DEC,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_MAPZ_IN, ACT_NONE, ACT_MAPZ_OUT, ACT_NONE,
// DIV,     F1,       F2,       F3,       F4,       F5,       F6,       F7,       F8,       F9,
  ACT_NONE, ACT_HELP, ACT_NONE, ACT_NONE, ACT_NONE, ACT_MSG0, ACT_MSG1, ACT_MSG2, ACT_MSG3, ACT_NONE,
// F10,     F11,      F12,      F13,      F14,      F15,      F16,      F17,      F18,      F19,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// F20,     F21,      F22,      F23,      F24,      UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    NUMLOCK,  SCRLLOCK, UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    LSHIFT,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// RSHIFT,  LCTRL,    RCTRL,    LMENU,    RMENU,    BBACK,    BFWD,     BREFRESH, BSTOP,    BSEARCH,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// BFAV,    BHOME,    MUTE,     VOL_UP,   VOL_DOWN, NTRACK,   PTRACK,   STOP,     PLAYP,    MAIL,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// MSEL,    APP1,     APP2,     UNDEF,    UNDEF,    OEM_1,    OEM_PLUS,    OEM_COMMA, OEM_MINUS,    OEM_PERIOD,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_MAPZ_IN, ACT_NONE,  ACT_MAPZ_OUT, ACT_NONE,
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
// OEM_2,   OEM_3,    CONTROLLER_1,    CONTROLLER_2,     CONTROLLER_3,    CONTROLLER_4,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_CTRL_ALTACT, ACT_CTRL_CASTACT, ACT_CTRL_USE_HP, ACT_CTRL_USE_MP, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
#else
// OEM_2,   OEM_3,    CONTROLLER_1,    CONTROLLER_2,     CONTROLLER_3,    CONTROLLER_4,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE,        ACT_NONE,         ACT_NONE,        ACT_NONE,        ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
#endif
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    OEM_4,    OEM_5,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// OEM_6,   OEM_7,    OEM_8,    UNDEF,    UNDEF,    OEM_102,  UNDEF,    UNDEF,    PROC,     UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// PACKET,  UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// UNDEF,   UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    ATTN,     CRSEL,    EREOF,    PLAY,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
// ZOOM,    UNDEF,    PA1,      OCLEAR,   UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
};
// clang-format on

/** To know if ui has been initialized or not */
bool gbWasUiInit = false;
bool gbSndInited = false;

static int diablo_parse_flags(int argc, char** argv)
{
	for (int i = 1; i < argc; i++) {
		if (SDL_strcasecmp("--data-dir", argv[i]) == 0) {
			i++;
			if (i < argc)
				SetBasePath(argv[i]);
		} else if (SDL_strcasecmp("--save-dir", argv[i]) == 0) {
			i++;
			if (i < argc)
				SetPrefPath(argv[i]);
		} else if (SDL_strcasecmp("-n", argv[i]) == 0) {
			_gbSkipIntro = true;
		} else if (SDL_strcasecmp("-x", argv[i]) == 0) {
			gbFullscreen = false;
		}
	}
	return EX_OK;
}

static void diablo_init_screen()
{
	MousePos.x = SCREEN_WIDTH / 2;
	MousePos.y = SCREEN_HEIGHT / 2;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	if (!sgbControllerActive)
#endif
		SetCursorPos(MousePos.x, MousePos.y);
	static_assert(EMSG_NONE == 0, "ClrDiabloMsg is not called, because zero initialization cares about it.");
	// not the best place to call this, since it is an in-game 'system'
	// InitGameFX would be more appropriate place, but calling it once would
	// suffice for the moment, because the player can not leave the game
	// until the messages are cleared
	//ClrDiabloMsg();
}

static void InitControls()
{
	int i;

	// load key-configuration from diablo.ini
	SLoadKeyMap(WMButtonInputTransTbl);

	// find the action-keys to trigger when the button is held down
	actionBtnKey = ACT_NONE;
	altActionBtnKey = ACT_NONE;
	for (i = 0; i < lengthof(WMButtonInputTransTbl); i++) {
		if (WMButtonInputTransTbl[i] == ACT_ACT)
			actionBtnKey = i;
		else if (WMButtonInputTransTbl[i] == ACT_ALTACT)
			altActionBtnKey = i;
	}
}

static void diablo_init()
{
	InitPaths();
	InitConfig();

	dx_init(); // inititalize SDL + create the window

	InitArchives();
#if DEBUG_MODE || DEV_MODE
	ValidateData();
#endif
	InitLighting();
	InitText();
	InitCursorGFX();
	UiInitialize();
	gbWasUiInit = true;

	diablo_init_screen();
#ifdef SCREEN_READER_INTEGRATION
	InitScreenReader();
#endif
	InitSound();
	gbSndInited = true;

	InitUiSFX(); // sfx

	InitGameUI();

	InitControls();
}

static bool diablo_splash()
{
	if (play_movie("gendata\\logo.smk", MOV_SKIP) == MPR_QUIT)
		return false;
	if (getIniBool("Diablo", "Intro", true)) {
		setIniInt("Diablo", "Intro", false);
		if (play_movie(INTRO_ARCHIVE, MOV_SKIP) == MPR_QUIT)
			return false;
	}

	return UiTitleDialog();
}

static void diablo_deinit()
{
	NetClose();
	SNetDestroy();
	pfile_flush(true);
	// FreeGameFX(); StopHelp/ClearPanels(); -- TODO: enable if the OS cares about non-freed memory
	if (gbSndInited) {
		StopSFX(); // stop click-effect
		FreeUiSFX();
		FreeSound();
	}
#ifdef SCREEN_READER_INTEGRATION
	FreeScreenReader();
#endif
	//if (gbWasUiInit)
		UiDestroy();
		FreeText();
		FreeCursorGFX();
	//if (_gbWasArchivesInit)
		FreeArchives();
	//if (_gbWasWindowInit) {
		dx_cleanup(); // close the window + SDL
	FreeConfig();
}

int DiabloMain(int argc, char** argv)
{
	int res = diablo_parse_flags(argc, argv);
	if (res != EX_OK)
		return res - 1;

	diablo_init();
#ifndef HOSTONLY
	if (_gbSkipIntro || diablo_splash())
#endif
		mainmenu_loop();
	diablo_deinit();
	return 0;
}

void diablo_quit(int exitStatus)
{
	diablo_deinit();
	exit(exitStatus);
}

void FreeLevelMem()
{
	StopSFX();
	music_stop();

	FreeLvlDungeon();
	FreeMonsters();
	FreeObjectGFX();
	FreeTownerGFX();
}

static int8_t ValidateSkill(BYTE sn, BYTE splType)
{
	int8_t result;
	assert(sn != SPL_INVALID);

	if ((spelldata[sn].sUseFlags & myplr._pSkillFlags) != spelldata[sn].sUseFlags) {
		// PlaySFX(sgSFXSets[SFXS_PLR_34][p->_pClass]);
		return SPLFROM_INVALID_TYPE;
	}

	switch (splType) {
	case RSPLTYPE_ABILITY:
		// assert(spelldata[sn].sManaCost == 0);
		result = SPLFROM_ABILITY;
		break;
	case RSPLTYPE_SPELL:
		if (myplr._pMana < GetManaAmount(mypnum, sn))
			result = SPLFROM_INVALID_MANA;
		else if (myplr._pSkillLvl[sn] > 0)
			result = SPLFROM_MANA; // PlaySFX(sgSFXSets[SFXS_PLR_35][p->_pClass]);
		else
			result = SPLFROM_INVALID_LEVEL;
		break;
	case RSPLTYPE_INV:
		result = SpellSourceInv(sn);
		break;
	case RSPLTYPE_CHARGES:
		result = SpellSourceEquipment(sn);
		break;
	case RSPLTYPE_INVALID:
		result = SPLFROM_INVALID_TYPE;
		break;
	default:
		result = SPLFROM_ABILITY;
		ASSUME_UNREACHABLE
		break;
	}

	return result;
}

static void DoActionBtnCmd(BYTE moveSkill, BYTE moveSkillType, BYTE atkSkill, BYTE atkSkillType, bool bShift)
{
	int8_t msf = 0, asf = 0;

	if (bShift)
		moveSkill = SPL_INVALID;
	else if (moveSkill != SPL_INVALID) {
		msf = ValidateSkill(moveSkill, moveSkillType);
		if (SPLFROM_INVALID(msf))
			moveSkill = SPL_INVALID;
	}

	if (atkSkill != SPL_INVALID) {
		asf = ValidateSkill(atkSkill, atkSkillType);
		if (SPLFROM_INVALID(asf))
			atkSkill = SPL_INVALID;
	}

	if (atkSkill != SPL_INVALID) {
		if (atkSkill == SPL_BLOCK) {
			int dir = GetDirection(myplr._px, myplr._py, pcurspos.x, pcurspos.y);
			NetSendCmdBParam1(CMD_BLOCK, dir);
			return;
		}

		if (spelldata[atkSkill].spCurs != CURSOR_NONE) {
			NewCursor(spelldata[atkSkill].spCurs);
			gbTSpell = atkSkill;
			gbTSplFrom = asf;
			return;
		}

		if (bShift) {
			NetSendCmdLocSkill(pcurspos.x, pcurspos.y, atkSkill, asf);
			return;
		}
		if (pcursmonst != MON_NONE) {
			if (CanTalkToMonst(pcursmonst)) {
				NetSendCmdParam1(CMD_TALKXY, pcursmonst);
			} else {
				NetSendCmdMonSkill(pcursmonst, atkSkill, asf);
			}
			return;
		}
		if (pcursplr != PLR_NONE && myplr._pTeam != players[pcursplr]._pTeam) {
			NetSendCmdPlrSkill(pcursplr, atkSkill, asf);
			return;
		}
		if (moveSkill == SPL_INVALID) {
			NetSendCmdLocSkill(pcurspos.x, pcurspos.y, atkSkill, asf);
			return;
		}
	} else if (moveSkill == SPL_INVALID) {
		const int* sfx;
		if (asf == SPLFROM_INVALID_MANA || msf == SPLFROM_INVALID_MANA)
			sfx = sgSFXSets[SFXS_PLR_35]; // no mana
		else
			sfx = sgSFXSets[SFXS_PLR_34]; // nothing to do/not ready
		PlaySFX(sfx[myplr._pClass]);
		return;
	}

	// assert(moveSkill != SPL_INVALID);
	// assert(spelldata[atkSkill].spCurs == CURSOR_NONE); -- TODO extend if there are targeted move skills

	if (pcursmonst != MON_NONE) {
		if (CanTalkToMonst(pcursmonst)) {
			NetSendCmdParam1(CMD_TALKXY, pcursmonst);
			return;
		}

		// TODO: move closer, execute moveSkill if not SPL_WALK?
		//return;
	}

	if (pcursplr != PLR_NONE) {
		// TODO: move closer, execute moveSkill if not SPL_WALK? Trade?
		//return;
	}

	if (pcursobj != OBJ_NONE) {
		bool bNear = abs(myplr._px - pcurspos.x) < 2 && abs(myplr._py - pcurspos.y) < 2;
		if (moveSkill == SPL_WALK || (bNear && objects[pcursobj]._oBreak == OBM_BREAKABLE)) {
			NetSendCmdLocParam1(CMD_OPOBJXY, pcurspos.x, pcurspos.y, pcursobj);
			return;
		}
		//return; // TODO: proceed in case moveSkill != SPL_WALK?
	}
	if (moveSkill != SPL_WALK) {
		// TODO: check if pcurspos.x/y == _px/y ?
		NetSendCmdLocSkill(pcurspos.x, pcurspos.y, moveSkill, msf);
		return;
	}

	if (pcursitem != ITEM_NONE) {
		NetSendCmdLocParam1(gbInvflag ? CMD_GOTOGETITEM : CMD_GOTOAGETITEM, pcurspos.x, pcurspos.y, pcursitem);
		return;
	}

	NetSendCmdLoc(CMD_WALKXY, pcurspos.x, pcurspos.y);
}

void ActionBtnCmd(bool bShift)
{
	assert(pcursicon == CURSOR_HAND);

	DoActionBtnCmd(myplr._pMoveSkill, myplr._pMoveSkillType,
		myplr._pAtkSkill, myplr._pAtkSkillType, bShift);
}

bool TryIconCurs(bool bShift)
{
	switch (pcursicon) {
	case CURSOR_IDENTIFY:
	case CURSOR_REPAIR:
	case CURSOR_RECHARGE:
	case CURSOR_OIL:
		if (pcursinvitem != INVITEM_NONE) {
			NetSendCmdItemSkill(pcursinvitem, gbTSpell, gbTSplFrom);
		}
		break;
	case CURSOR_DISARM:
		if (pcursobj != OBJ_NONE && objects[pcursobj]._oBreak == OBM_UNBREAKABLE) {
			if (!bShift ||
			 (abs(myplr._px - pcurspos.x) < 2 && abs(myplr._py - pcurspos.y) < 2)) {
				// assert(gbTSpell == SPL_DISARM);
				NetSendCmdLocDisarm(pcurspos.x, pcurspos.y, pcursobj, gbTSplFrom);
			}
		}
		break;
	case CURSOR_TELEKINESIS: {
		// assert(gbTSpell == SPL_TELEKINESIS);
		if (pcursobj != OBJ_NONE) {
			NetSendCmdParamBW(CMD_TELEKINOID, gbTSplFrom, pcursobj);
		} else if (pcursitem != ITEM_NONE) {
			NetSendCmdLocBParam2(CMD_TELEKINXY, items[pcursitem]._ix, items[pcursitem]._iy, gbTSplFrom, pcursitem);
		} else if (pcursmonst != MON_NONE) {
			NetSendCmdParamBW(CMD_TELEKINID, gbTSplFrom, pcursmonst);
		}
	} break;
	case CURSOR_TELEPORT:
	case CURSOR_HEALOTHER:
	case CURSOR_RESURRECT:
		if (pcursmonst != MON_NONE)
			NetSendCmdMonSkill(pcursmonst, gbTSpell, gbTSplFrom);
		else if (pcursplr != PLR_NONE)
			NetSendCmdPlrSkill(pcursplr, gbTSpell, gbTSplFrom);
		else if (pcursicon == CURSOR_TELEPORT)
			NetSendCmdLocSkill(pcurspos.x, pcurspos.y, gbTSpell, gbTSplFrom);
		break;
	default:
		return false;
	}
	NewCursor(CURSOR_HAND);
	return true;
}

static void ActionBtnDown(bool bShift)
{
	assert(!gbDropGoldFlag);
	assert(!gmenu_is_active());
	assert(gnTimeoutCurs == CURSOR_NONE);
	// assert(!gbTalkflag || !control_check_talk_btn());
	assert(gbDeathflag == MDM_ALIVE);
	assert(!gbGamePaused);
	//assert(!gbDoomflag);
	assert(!gbQtextflag);

	if (gbCampaignMapFlag) {
		TryCampaignMapClick(bShift, false);
		return;
	}

	if (gbSkillListFlag) {
		SetSkill(bShift, false);
		return;
	}

	if (stextflag != STORE_NONE) {
		TryStoreBtnClick();
		return;
	}

	if (gmenu_is_active()) {
		TryLimitedPanBtnClick();
		return;
	}

	if (TryPanBtnClick()) {
		return;
	}

	if (TryIconCurs(bShift))
		return;

	if (pcurswnd == WND_BELT) {
		// in belt
		// assert(!TryPanBtnClick());
		CheckBeltClick(bShift);
		return;
	}

	if (pcurswnd == WND_INV) {
		// in inventory
		CheckInvClick(bShift);
		return;
	}

	if (pcurswnd == WND_CHAR) {
		CheckChrBtnClick();
		return;
	}

	if (pcurswnd == WND_QUEST) {
		CheckQuestlogClick();
		return;
	}

	if (pcurswnd == WND_TEAM) {
		CheckTeamClick(bShift);
		return;
	}

	if (pcurswnd == WND_BOOK) {
		CheckBookClick(bShift, false);
		return;
	}

	if (pcursicon >= CURSOR_FIRSTITEM) {
		DropItem();
		return;
	}

	ActionBtnCmd(bShift);
}

void AltActionBtnCmd(bool bShift)
{
	assert(pcursicon == CURSOR_HAND);

	DoActionBtnCmd(myplr._pAltMoveSkill, myplr._pAltMoveSkillType,
		myplr._pAltAtkSkill, myplr._pAltAtkSkillType, bShift);
}

static void AltActionBtnDown(bool bShift)
{
	assert(!gmenu_is_active());
	assert(gnTimeoutCurs == CURSOR_NONE);
	assert(gbDeathflag == MDM_ALIVE);
	assert(!gbGamePaused);
	//assert(!gbDoomflag);
	assert(!gbQtextflag);

	if (gbCampaignMapFlag) {
		TryCampaignMapClick(bShift, true);
		return;
	}

	if (gbSkillListFlag) {
		SetSkill(bShift, true);
		return;
	}

	if (stextflag != STORE_NONE)
		return;

	if (TryIconCurs(bShift))
		return;
	if (pcursicon >= CURSOR_FIRSTITEM) {
		DropItem();
		return;
	}

	if (pcursinvitem != INVITEM_NONE && InvUseItem(pcursinvitem))
		return;

	if (pcurswnd == WND_BOOK) {
		CheckBookClick(bShift, true);
		return;
	}

	if (pcurswnd == WND_TEAM) {
		CheckTeamClick(bShift);
		return;
	}

	AltActionBtnCmd(bShift);
}

static void diablo_pause_game()
{
	if (!IsMultiGame) {
		gbGamePaused = !gbGamePaused;
		sound_pause(gbGamePaused);
		//gbRedrawFlags = REDRAW_ALL;
	}
}

static void diablo_hotkey_msg(int actKey)
{
	char entryKey[16];

	if (IsLocalGame)
		return;

	static_assert(ACT_MSG0 + 1 == ACT_MSG1, "diablo_hotkey_msg expects a continuous assignment of ACT_MSGx 1.");
	static_assert(ACT_MSG1 + 1 == ACT_MSG2, "diablo_hotkey_msg expects a continuous assignment of ACT_MSGx 2.");
	static_assert(ACT_MSG2 + 1 == ACT_MSG3, "diablo_hotkey_msg expects a continuous assignment of ACT_MSGx 3.");
	snprintf(entryKey, sizeof(entryKey), "QuickMsg%02d", actKey - ACT_MSG0);
	if (!getIniValue("NetMsg", entryKey, gbNetMsg, sizeof(gbNetMsg)))
		return;

	NetSendCmdString(SNPLAYER_ALL);
}

/*static bool PressSysKey(int wParam)
{
	if (gmenu_is_active() || wParam != DVL_VK_F10)
		return false;
	diablo_hotkey_msg(ACT_MSG1);
	return true;
}*/

static void ReleaseKey(int vkey)
{
	if (vkey == DVL_VK_LBUTTON) {
		if (gmenu_is_active())
			gmenu_left_mouse(false);
		if (gabPanbtn[PANBTN_MAINMENU])
			ReleasePanBtn();
		if (gbChrbtnactive)
			ReleaseChrBtn();
		if (gbLvlbtndown)
			ReleaseLvlBtn();
		if (stextflag != STORE_NONE)
			ReleaseStoreBtn();
		if (gbTalkflag)
			plrmsg_HandleMouseReleaseEvent();
		gbDragWnd = WND_NONE;
	} else if (vkey == DVL_VK_SNAPSHOT) {
		CaptureScreen();
	}

	if (WMButtonInputTransTbl[vkey] == ACT_ACT) {
		gbActionBtnDown = false;
	} else if (WMButtonInputTransTbl[vkey] == ACT_ALTACT) {
		gbAltActionBtnDown = false;
	}
}

bool PressEscKey()
{
	bool rv = false;

	/*assert(!gmenu_is_active());
	if (gmenu_is_active()) {
		gamemenu_off();
		rv = true;
	}
	//assert(!gbTalkflag);
	if (gbTalkflag) {
		StopPlrMsg();
		rv = true;
	}*/
	/*if (gbDoomflag) {
		doom_close();
		rv = true;
	}*/
	if (gbHelpflag) {
		StopHelp();
		rv = true;
	}
	if (gbQtextflag) {
		StopQTextMsg();
		rv = true;
	} else if (stextflag != STORE_NONE) {
		STextESC();
		rv = true;
	}
	if (currmsg != EMSG_NONE) {
		msgdelay = 0;
		rv = true;
	}
	if (gbDropGoldFlag) {
		control_drop_gold(DVL_VK_ESCAPE);
		rv = true;
	}
	if (gbSkillListFlag) {
		gbSkillListFlag = false;
		rv = true;
	}
	if (gbCampaignMapFlag) {
		gbCampaignMapFlag = false;
		rv = true;
	}
	if (gabPanbtn[PANBTN_MAINMENU]) {
		gabPanbtn[PANBTN_MAINMENU] = false;
		rv = true;
	}
	if (pcursicon != CURSOR_HAND && pcursicon < CURSOR_FIRSTITEM) {
		NewCursor(CURSOR_HAND);
		rv = true;
	}

	return rv;
}

void ClearPanels()
{
	StopHelp();
	gbInvflag = false;
	gnNumActiveWindows = 0;
	gbSkillListFlag = false;
	gbCampaignMapFlag = false;
	gbDropGoldFlag = false;
}

static void ClearUI()
{
	ClearPanels();
	//gamemenu_off();
	assert(!gbQtextflag);
	gbAutomapflag = AMM_NONE;
	msgdelay = 0;
	gabPanbtn[PANBTN_MAINMENU] = false;
	//doom_close();
}

#if DEBUG_MODE
static void PressDebugChar(int vkey)
{
	switch (vkey) {
	case 'R':
	case 'r':
		snprintf(gbNetMsg, sizeof(gbNetMsg), "seed = %d", glSeedTbl[currLvl._dLevelIdx]);
		NetSendCmdString(1 << mypnum);
		break;
	case 'T':
	case 't':
		snprintf(gbNetMsg, sizeof(gbNetMsg), "PX = %d  PY = %d", myplr._px, myplr._py);
		NetSendCmdString(1 << mypnum);
		snprintf(gbNetMsg, sizeof(gbNetMsg), "CX = %d  CY = %d  DP = %d", pcurspos.x, pcurspos.y, dungeon[pcurspos.x][pcurspos.y]);
		NetSendCmdString(1 << mypnum);
		break;
	case '[':
		if (pcursitem != ITEM_NONE) {
			snprintf(
			    gbNetMsg,
				sizeof(gbNetMsg),
			    "IDX = %d  :  Seed = %d  :  CF = %d",
			    items[pcursitem]._iIdx,
			    items[pcursitem]._iSeed,
			    items[pcursitem]._iCreateInfo);
			NetSendCmdString(1 << mypnum);
		}
		snprintf(gbNetMsg, sizeof(gbNetMsg), "Numitems : %d", numitems);
		NetSendCmdString(1 << mypnum);
		break;
	}
}
#endif

static void PressKey(int vkey)
{
	if (gmenu_is_active()) {
		gmenu_presskey(vkey);
		return;
	}
	if (gbTalkflag) {
		if (plrmsg_presskey(vkey))
			return;
	}

	if (vkey == DVL_VK_ESCAPE) {
		if (!PressEscKey()) {
			gamemenu_on();
		}
		return;
	}

	if (gnTimeoutCurs != CURSOR_NONE) {
		return;
	}
#if !__IPHONEOS__ && !__ANDROID__
	if (vkey == DVL_VK_RETURN && (SDL_GetModState() & KMOD_ALT)) {
		ToggleFullscreen();
		return;
	}
#endif
	int transKey = WMButtonInputTransTbl[vkey];
	if (gbDeathflag != MDM_ALIVE) {
		if (vkey == DVL_VK_RETURN) {
			StartPlrMsg();
		} else if (vkey == DVL_VK_LBUTTON) {
			TryLimitedPanBtnClick();
		} else {
			if (transKey >= ACT_MSG0 && transKey <= ACT_MSG3)
				diablo_hotkey_msg(transKey);
		}
		return;
	}

	if (transKey == ACT_PAUSE) {
		diablo_pause_game();
		return;
	}
	if (gbGamePaused) {
		return;
	}

	if (gbDropGoldFlag) {
		control_drop_gold(vkey);
		return;
	}

	/*if (gbDoomflag) {
		doom_close();
		return;
	}*/

	if (gbQtextflag) {
		StopQTextMsg();
		return;
	}

	switch (transKey) {
	case ACT_NONE:
#if DEBUG_MODE
		transKey = TranslateKey2Char(vkey);
		PressDebugChar(transKey);
#endif
		break;
	case ACT_ACT:
		if (!gbActionBtnDown) {
			gbActionBtnDown = true;
			guLastABD = SDL_GetTicks();
			ActionBtnDown((SDL_GetModState() & KMOD_SHIFT));
		}
		break;
	case ACT_ALTACT:
		if (!gbAltActionBtnDown) {
			gbAltActionBtnDown = true;
			guLastAABD = SDL_GetTicks();
			AltActionBtnDown((SDL_GetModState() & KMOD_SHIFT));
		}
		break;
	case ACT_SWAP: {
		struct TmpKeys {
			BYTE _pAtkSkillSwapKey[4];         // the attack skill selected by the hotkey after skill-set swap
			BYTE _pAtkSkillTypeSwapKey[4];     // the (RSPLTYPE_)type of the attack skill selected by the hotkey after skill-set swap
			BYTE _pMoveSkillSwapKey[4];        // the movement skill selected by the hotkey after skill-set swap
			BYTE _pMoveSkillTypeSwapKey[4];    // the (RSPLTYPE_)type of the movement skill selected by the hotkey after skill-set swap
			BYTE _pAltAtkSkillSwapKey[4];      // the attack skill selected by the alt-hotkey after skill-set swap
			BYTE _pAltAtkSkillTypeSwapKey[4];  // the (RSPLTYPE_)type of the attack skill selected by the alt-hotkey after skill-set swap
			BYTE _pAltMoveSkillSwapKey[4];     // the movement skill selected by the alt-hotkey after skill-set swap
			BYTE _pAltMoveSkillTypeSwapKey[4]; // the (RSPLTYPE_)type of the movement skill selected by the alt-hotkey after skill-set swap
		} tmpKeys;
		static_assert(offsetof(PlayerStruct, _pAltMoveSkillTypeSwapKey) + sizeof(myplr._pAltMoveSkillTypeSwapKey) == offsetof(PlayerStruct, _pAtkSkillSwapKey) + sizeof(tmpKeys));
		static_assert(offsetof(PlayerStruct, _pAltMoveSkillTypeHotKey) + sizeof(myplr._pAltMoveSkillTypeHotKey) == offsetof(PlayerStruct, _pAtkSkillHotKey) + sizeof(tmpKeys));
		memcpy(&tmpKeys, myplr._pAtkSkillSwapKey, sizeof(tmpKeys));
		memcpy(myplr._pAtkSkillSwapKey, myplr._pAtkSkillHotKey, sizeof(tmpKeys));
		memcpy(myplr._pAtkSkillHotKey, &tmpKeys, sizeof(tmpKeys));
	} break;
	case ACT_TGT:
		ChangeTarget();
		break;
	case ACT_SKL0:
	case ACT_SKL1:
	case ACT_SKL2:
	case ACT_SKL3:
		static_assert(ACT_SKL0 + 1 == ACT_SKL1, "PressKey expects a continuous assignment of ACT_SKLx 1.");
		static_assert(ACT_SKL1 + 1 == ACT_SKL2, "PressKey expects a continuous assignment of ACT_SKLx 2.");
		static_assert(ACT_SKL2 + 1 == ACT_SKL3, "PressKey expects a continuous assignment of ACT_SKLx 3.");
		if (gbSkillListFlag)
			SetSkillHotKey(transKey - ACT_SKL0, false);
		else
			SelectHotKeySkill(transKey - ACT_SKL0, false);
		break;
	case ACT_SKL4:
	case ACT_SKL5:
	case ACT_SKL6:
	case ACT_SKL7:
		static_assert(ACT_SKL4 + 1 == ACT_SKL5, "PressKey expects a continuous assignment of ACT_SKLx 4.");
		static_assert(ACT_SKL5 + 1 == ACT_SKL6, "PressKey expects a continuous assignment of ACT_SKLx 5.");
		static_assert(ACT_SKL6 + 1 == ACT_SKL7, "PressKey expects a continuous assignment of ACT_SKLx 6.");
		if (gbSkillListFlag)
			SetSkillHotKey(transKey - ACT_SKL4, true);
		else
			SelectHotKeySkill(transKey - ACT_SKL4, true);
		break;
	case ACT_INV:
		HandlePanBtn(PANBTN_INVENTORY);
		break;
	case ACT_CHAR:
		HandlePanBtn(PANBTN_CHARINFO);
		break;
	case ACT_SKLBOOK:
		HandlePanBtn(PANBTN_SPELLBOOK);
		break;
	case ACT_SKLLIST:
		if (stextflag == STORE_NONE) {
			HandleSkillBtn(false);
		}
		break;
	case ACT_ITEM0:
	case ACT_ITEM1:
	case ACT_ITEM2:
	case ACT_ITEM3:
	case ACT_ITEM4:
	case ACT_ITEM5:
	case ACT_ITEM6:
	case ACT_ITEM7:
		static_assert(ACT_ITEM0 + 1 == ACT_ITEM1, "PressKey expects a continuous assignment of ACT_ITEMx 1.");
		static_assert(ACT_ITEM1 + 1 == ACT_ITEM2, "PressKey expects a continuous assignment of ACT_ITEMx 2.");
		static_assert(ACT_ITEM2 + 1 == ACT_ITEM3, "PressKey expects a continuous assignment of ACT_ITEMx 3.");
		static_assert(ACT_ITEM3 + 1 == ACT_ITEM4, "PressKey expects a continuous assignment of ACT_ITEMx 4.");
		static_assert(ACT_ITEM4 + 1 == ACT_ITEM5, "PressKey expects a continuous assignment of ACT_ITEMx 5.");
		static_assert(ACT_ITEM5 + 1 == ACT_ITEM6, "PressKey expects a continuous assignment of ACT_ITEMx 6.");
		static_assert(ACT_ITEM6 + 1 == ACT_ITEM7, "PressKey expects a continuous assignment of ACT_ITEMx 7.");
		InvUseItem(INVITEM_BELT_FIRST + transKey - ACT_ITEM0);
		break;
	case ACT_AUTOMAP:
		ToggleAutomap();
		break;
	case ACT_MAPZ_IN:
		if (gbAutomapflag != AMM_NONE) {
			AutomapZoomIn();
		}
		break;
	case ACT_MAPZ_OUT:
		if (gbAutomapflag != AMM_NONE) {
			AutomapZoomOut();
		}
		break;
	case ACT_CLEARUI:
		ClearUI();
		break;
	case ACT_UP:
		if (stextflag != STORE_NONE) {
			STextUp();
		} else if (gnNumActiveWindows != 0 && gaActiveWindows[gnNumActiveWindows - 1] == WND_QUEST) {
			QuestlogUp();
		} else if (gbHelpflag) {
			HelpScrollUp();
		} else if (gbAutomapflag != AMM_NONE) {
			AutomapUp();
		}
		break;
	case ACT_DOWN:
		if (stextflag != STORE_NONE) {
			STextDown();
		} else if (gnNumActiveWindows != 0 && gaActiveWindows[gnNumActiveWindows - 1] == WND_QUEST) {
			QuestlogDown();
		} else if (gbHelpflag) {
			HelpScrollDown();
		} else if (gbAutomapflag != AMM_NONE) {
			AutomapDown();
		}
		break;
	case ACT_LEFT:
		if (gbAutomapflag != AMM_NONE) {
			AutomapLeft();
		}
		break;
	case ACT_RIGHT:
		if (gbAutomapflag != AMM_NONE) {
			AutomapRight();
		}
		break;
	case ACT_PGUP:
		if (stextflag != STORE_NONE) {
			STextPageUp();
		}
		break;
	case ACT_PGDOWN:
		if (stextflag != STORE_NONE) {
			STextPageDown();
		}
		break;
	case ACT_RETURN:
		if (stextflag != STORE_NONE) {
			STextEnter();
		} else if (gnNumActiveWindows != 0 && gaActiveWindows[gnNumActiveWindows - 1] == WND_QUEST) {
			QuestlogEnter();
		} else {
			StartPlrMsg();
		}
		break;
	case ACT_TEAM:
		HandlePanBtn(PANBTN_TEAMBOOK);
		break;
	case ACT_QUESTS:
		HandlePanBtn(PANBTN_QLOG);
		break;
	case ACT_MSG0:
	case ACT_MSG1:
	case ACT_MSG2:
	case ACT_MSG3:
		diablo_hotkey_msg(transKey);
		break;
	case ACT_GAMMA_DEC:
		DecreaseGamma();
		break;
	case ACT_GAMMA_INC:
		IncreaseGamma();
		break;
	case ACT_ZOOM:
		gbZoomInFlag = !gbZoomInFlag;
		CalcViewportGeometry();
		break;
	case ACT_VER:
		VersionPlrMsg();
		break;
	case ACT_HELP:
		if (gbHelpflag) {
			StopHelp();
		} else if (stextflag == STORE_NONE) {
			ClearPanels();
			//gamemenu_off();
			StartHelp();
		}
		break;
	case ACT_PAUSE:
		break;
	case ACT_ESCAPE:
		break;
	case ACT_TOOLTIP:
		gbShowTooltip = !gbShowTooltip;
		break;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	case ACT_CTRL_ALTACT:
		PerformSecondaryAction();
		break;
	case ACT_CTRL_CASTACT:
		PerformSpellAction();
		break;
	case ACT_CTRL_USE_HP:
		UseBeltItem(false);
		break;
	case ACT_CTRL_USE_MP:
		UseBeltItem(true);
		break;
#endif
	default:
		ASSUME_UNREACHABLE
	}
}

static void UpdateActionBtnState(int vKey, bool dir)
{
	if (vKey == actionBtnKey)
		gbActionBtnDown = dir;
	if (vKey == altActionBtnKey)
		gbAltActionBtnDown = dir;
}

void DisableInputWndProc(const Dvl_Event* e)
{
	switch (e->type) {
	case DVL_WM_KEYDOWN:
		UpdateActionBtnState(e->vkcode, true);
		return;
	case DVL_WM_KEYUP:
		UpdateActionBtnState(e->vkcode, false);
		return;
	case DVL_WM_TEXT:
	//case DVL_WM_SYSKEYDOWN:
	//case DVL_WM_SYSCOMMAND:
		return;
	case DVL_WM_QUIT:
		NetSendCmd(CMD_DISCONNECT);
		gbRunGameResult = false;
		return;
	case DVL_WM_MOUSEMOVE:
		return;
	case DVL_WM_LBUTTONDOWN:
		UpdateActionBtnState(DVL_VK_LBUTTON, true);
		return;
	case DVL_WM_LBUTTONUP:
		UpdateActionBtnState(DVL_VK_LBUTTON, false);
		return;
	case DVL_WM_RBUTTONDOWN:
		UpdateActionBtnState(DVL_VK_RBUTTON, true);
		return;
	case DVL_WM_RBUTTONUP:
		UpdateActionBtnState(DVL_VK_RBUTTON, false);
		return;
	case DVL_WM_CAPTURECHANGED:
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		return;
	case DVL_WM_PAINT:
		gbRedrawFlags = REDRAW_ALL;
		return;
	}

	// MainWndProc(uMsg);
}

static void GameWndProc(const Dvl_Event* e)
{
	switch (e->type) {
	case DVL_WM_KEYDOWN:
		PressKey(e->vkcode);
		return;
	case DVL_WM_KEYUP:
		ReleaseKey(e->vkcode);
		return;
	case DVL_WM_TEXT:
#ifndef USE_SDL1
		if (gmenu_is_active()) {
			return;
		}
		if (gbTalkflag) {
			plrmsg_CatToText(e->text.text);
			return;
		}
#endif
		return;
	//case DVL_WM_SYSKEYDOWN:
	//	if (PressSysKey(wParam))
	//		return;
	//	break;
	//case DVL_WM_SYSCOMMAND:
	//	if (wParam != DVL_SC_CLOSE)
	//		break;
	//	/* fall-through */
	case DVL_WM_QUIT:
		if (gmenu_is_active())
			gamemenu_off();
		NetSendCmd(CMD_DISCONNECT);
		gbRunGameResult = false;
		gbGamePaused = false;
		return;
	case DVL_WM_MOUSEMOVE:
		if (gmenu_is_active())
			gmenu_on_mouse_move();
		else if (gbDragWnd != WND_NONE)
			DoWndDrag();
		else if (gbTalkflag)
			plrmsg_HandleMouseMoveEvent();
		return;
	case DVL_WM_LBUTTONDOWN:
		//GetMousePos(wParam); -- disabled to prevent inconsistent MousePos.x/y vs. CheckCursMove state
		PressKey(DVL_VK_LBUTTON);
		return;
	case DVL_WM_LBUTTONUP:
		//GetMousePos(wParam);
		ReleaseKey(DVL_VK_LBUTTON);
		return;
	case DVL_WM_RBUTTONDOWN:
		//GetMousePos(wParam);
		PressKey(DVL_VK_RBUTTON);
		return;
	case DVL_WM_RBUTTONUP:
		//GetMousePos(wParam);
		ReleaseKey(DVL_VK_RBUTTON);
		return;
	case DVL_WM_CAPTURECHANGED:
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		return;
	case DVL_WM_PAINT:
		gbRedrawFlags = REDRAW_ALL;
		return;
	case DVL_DWM_NEXTLVL:
	case DVL_DWM_PREVLVL:
	case DVL_DWM_SETLVL:
	case DVL_DWM_RTNLVL:
	case DVL_DWM_DYNLVL:
	case DVL_DWM_PORTLVL:
	case DVL_DWM_TWARPDN:
	case DVL_DWM_TWARPUP:
	case DVL_DWM_RETOWN:
	case DVL_DWM_NEWGAME:
	case DVL_DWM_LOADGAME:
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		if (gbQtextflag) {
			StopQTextMsg();
		}
		ShowCutscene(e->type);
		if (gbRunGame) {
			InitLevelCursor();
			LoadPWaterPalette();
			PaletteFadeIn(true);
			gbRedrawFlags = REDRAW_ALL;
			scrollrt_draw_game();
			//gbRedrawFlags = REDRAW_ALL;
		}
		return;
	}

	// MainWndProc(uMsg);
}

static bool ProcessInput()
{
	if (gbGamePaused) {
		return false;
	}

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	plrctrls_every_frame();
#endif

	if (gmenu_is_active()) {
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		CheckMenuMove();
#endif
		return IsMultiGame;
	}

	if (gnTimeoutCurs == CURSOR_NONE && gbDragWnd == WND_NONE) {
#if HAS_TOUCHPAD
		finish_simulated_mouse_clicks();
#endif
		CheckCursMove();
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		plrctrls_after_check_curs_move();
#endif
		Uint32 tick = SDL_GetTicks();
		if (gbActionBtnDown && (tick - guLastABD) >= 200) {
			gbActionBtnDown = false;
			PressKey(actionBtnKey);
		}
		if (gbAltActionBtnDown && (tick - guLastAABD) >= 200) {
			gbAltActionBtnDown = false;
			PressKey(altActionBtnKey);
		}
	}

	return true;
}

void game_logic()
{
	multi_rnd_seeds();
	// assert(gbGameLogicProgress == GLP_NONE);
	ProcessPlayers();
	gbGameLogicProgress = GLP_PLAYERS_DONE;
	ProcessMonsters();
	gbGameLogicProgress = GLP_MONSTERS_DONE;
	if (currLvl._dType == DTYPE_TOWN) {
		ProcessTowners();
		// gbGameLogicProgress = GLP_TOWNERS_DONE;
	}
	ProcessObjects();
	// gbGameLogicProgress = GLP_OBJECTS_DONE;
	ProcessMissiles();
	// gbGameLogicProgress = GLP_MISSILES_DONE;
	ProcessItems();
	// gbGameLogicProgress = GLP_ITEMS_DONE;
	ProcessLightList();
	ProcessVisionList();

#if DEBUG_MODE
	if ((SDL_GetModState() & KMOD_SHIFT)) {
		ScrollView();
	}
#endif

	CheckStreamSFX();
	CheckTriggers();
	CheckQuests();
	pfile_update(false);
	if (gmenu_is_active())
		gmenu_update();
	gbGameLogicProgress = GLP_NONE;
}

static void game_loop()
{
	int i;

	i = IsMultiGame ? 3 : 1;

	do {
		if (!ProcessInput())
			break;
		if (!multi_handle_turn()) {
			if (multi_check_timeout() && gnTimeoutCurs == CURSOR_NONE) {
				gnTimeoutCurs = pcursicon;
				NewCursor(CURSOR_HOURGLASS);
				// gbRedrawFlags = REDRAW_ALL;
			}
			//scrollrt_draw_screen(true);
			break;
		}
		if (gnTimeoutCurs != CURSOR_NONE) {
			NewCursor(gnTimeoutCurs);
			gnTimeoutCurs = CURSOR_NONE;
			// gbRedrawFlags = REDRAW_ALL;
		}
		//if (ProcessInput()) {
			game_logic();
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
			plrctrls_after_game_logic();
#endif
		//}
	} while (--i != 0 && gbRunGame && nthread_has_50ms_passed());
}

static void diablo_color_cyc_logic()
{
	if (!gbColorCyclingEnabled || gbGamePaused)
		return;

	if (currLvl._dType == DTYPE_HELL)
		lighting_update_hell();
	else if (currLvl._dType == DTYPE_CAVES)
		lighting_update_caves();
#ifdef HELLFIRE
	else if (currLvl._dType == DTYPE_CRYPT)
		lighting_update_crypt();
	else if (currLvl._dType == DTYPE_NEST)
		lighting_update_nest();
#endif
}

static WNDPROC InitGameFX()
{
	int i;

	InitAutomapOnce(); // values
	// InitHelp(); // values -- unnecessary since the player can not leave the game with active help
	InitControlPan(); // gfx + values
	InitInv();        // gfx + values
	InitGMenu();      // gfx
	InitQuestGFX();   // gfx + values
	InitStoresOnce(); // values (some stored in savefiles)
	for (i = 0; i < (IsLocalGame ? 1 : MAX_PLRS); i++)
		InitPlrGFXMem(i); // gfx
	InitItemGFX();        // gfx + values (some stored in savefiles)
	InitGameMissileGFX(); // gfx
	InitGameSFX();        // sfx

	gbDeathflag = MDM_ALIVE;
	gbZoomInFlag = false;
	CalcViewportGeometry();
	//ScrollInfo._sdx = 0;
	//ScrollInfo._sdy = 0;
	ScrollInfo._sxoff = 0;
	ScrollInfo._syoff = 0;
	ScrollInfo._sdir = SDIR_NONE;

	gnTimeoutCurs = CURSOR_NONE;
	gbActionBtnDown = false;
	gbAltActionBtnDown = false;
	gbRunGame = true;
	gbRunGameResult = true;

	return SetWindowProc(GameWndProc);
}

static void FreeGameFX()
{
	int i;

	FreeLevelMem();

	FreeControlPan();
	FreeInvGFX();
	FreeGMenu();
	FreeQuestGFX();

	for (i = 0; i < MAX_PLRS; i++)
		FreePlayerGFX(i);

	FreeItemGFX();
	FreeGameMissileGFX();
	FreeGameSFX();

	//doom_close();
}

static void run_game()
{
	WNDPROC saveProc = InitGameFX();
	SDL_Event event;

	event.type = gbLoadGame ? DVL_DWM_LOADGAME : DVL_DWM_NEWGAME;
	GameWndProc(&event);

#ifdef GPERF_HEAP_FIRST_GAME_ITERATION
	unsigned run_game_iteration = 0;
#endif
	while (TRUE) {
		while (gbRunGame && PeekMessage(event)) {
			DispatchMessage(&event);
		}
		if (!gbRunGame)
			break;
		if (!nthread_has_50ms_passed()) {
			ProcessInput();
			scrollrt_draw_game();
			continue;
		}
		diablo_color_cyc_logic();
		game_loop();
		scrollrt_draw_game();
#ifdef GPERF_HEAP_FIRST_GAME_ITERATION
		if (run_game_iteration++ == 0)
			HeapProfilerDump("first_game_iteration");
#endif
	}
	NetClose();
	if (IsMultiGame)
		pfile_write_hero(true);
	//else
	//	pfile_flush(true);

	PaletteFadeOut();
	//NewCursor(CURSOR_NONE);
	//ClearScreenBuffer();
	//scrollrt_draw_screen(true);
	saveProc = SetWindowProc(saveProc);
	assert(saveProc == GameWndProc);
	FreeGameFX();
}

bool StartGame(bool bSinglePlayer)
{
	gbSelectProvider = true;
	gbSelectHero = true;

	while (TRUE) {
		if (!NetInit(bSinglePlayer)) {
			return true;
		}
#ifndef HOSTONLY
		// Save 2.8 MiB of RAM by freeing all main menu resources before starting the game.
		UiDestroy();

		run_game();
		if (!gbRunGameResult)
			return false;
		// If the player left the game into the main menu,
		// initialize main menu resources.
		UiInitialize();
#endif
	}
}

DEVILUTION_END_NAMESPACE
