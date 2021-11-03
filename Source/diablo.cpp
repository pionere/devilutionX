/**
 * @file diablo.cpp
 *
 * Implementation of the main game initialization functions.
 */
#include "all.h"
#include <config.h>
#include "utils/paths.h"
#include "diabloui.h"
#include "plrctrls.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef GPERF_HEAP_FIRST_GAME_ITERATION
#include <gperftools/heap-profiler.h>
#endif
uint32_t glSeedTbl[NUM_LEVELS];
int MouseX;
int MouseY;
bool gbRunGame;
bool gbRunGameResult;
bool gbZoomInFlag;
/** Enable updating of player character, set to false once Diablo dies */
bool gbProcessPlayers;
bool gbLoadGame;
bool gbCineflag = false;
int gbRedrawFlags;
bool gbGamePaused;
bool gbDeathflag;
bool gbActionBtnDown;
bool gbAltActionBtnDown;
static Uint32 guLastABD, guLastAABD; // tick counter when the last time one of the mouse-buttons were pressed down
static int actionBtnKey, altActionBtnKey;
int gnTicksRate = SPEED_NORMAL;
unsigned gnTickDelay = 1000 / SPEED_NORMAL;
/* Cursor before a timeout happened. */
int gnTimeoutCurs;

/* rdata */

/**
 * Specifies whether to give the game exclusive access to the
 * screen, as needed for efficient rendering in fullscreen mode.
 */
bool gbFullscreen = true;
static bool _gbSkipIntro = false;
bool gbShowTooltip = false;
#ifdef _DEBUG
int DebugMonsters[10];
BOOL visiondebug;
/** unused */
BOOL scrollflag;
BOOL lightflag;
BOOL light4flag;
BOOL leveldebug;
BOOL monstdebug;
/** unused */
BOOL trigdebug;
int setseed;
int debugmonsttypes;
bool allquests;
int questdebug = -1;
int debug_mode_key_s;
int debug_mode_key_w;
int debug_mode_key_inverted_v;
BOOL debug_mode_god_mode;
int debug_mode_key_i;
int arrowdebug;
#endif
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
  ACT_GAMMA_INC, ACT_GAMMA_DEC, ACT_INV, ACT_NONE, ACT_NONE, ACT_SKLLIST, ACT_TEAM, ACT_NONE, ACT_NONE, ACT_PAUSE,
// Q,         R,        S,           T,           U,        V,       W,        X,        Y,        Z,
  ACT_SKL4, ACT_SKL7, ACT_SKL1, ACT_TOOLTIP, ACT_QUESTS, ACT_VER, ACT_SKL5, ACT_NONE, ACT_NONE, ACT_ZOOM,
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
// MSEL,    APP1,     APP2,     UNDEF,    UNDEF,    OEM_1,    OPLUS,       OCOMMA,   OMINUS,       OPERIOD,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_MAPZ_IN, ACT_NONE, ACT_MAPZ_OUT, ACT_NONE,
// OEM_2,   OEM_3,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,    UNDEF,
  ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE, ACT_NONE,
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

/** To know if these things have been done when we get to the diablo_deinit() function */
bool _gbWasArchivesInit = false;
/** To know if surfaces have been initialized or not */
bool _gbWasWindowInit = false;
bool gbWasUiInit = false;
bool gbSndInited = false;

static void print_help_and_exit()
{
	printf("Options:\n");
	printf("    %-20s %-30s\n", "-h, --help", "Print this message and exit");
	printf("    %-20s %-30s\n", "--version", "Print the version and exit");
	printf("    %-20s %-30s\n", "--data-dir", "Specify the folder of diabdat.mpq");
	printf("    %-20s %-30s\n", "--save-dir", "Specify the folder of save files");
	printf("    %-20s %-30s\n", "--config-dir", "Specify the location of diablo.ini");
	printf("    %-20s %-30s\n", "-n", "Skip startup videos");
	printf("    %-20s %-30s\n", "-x", "Run in windowed mode");
#ifdef _DEBUG
	printf("\nDebug options:\n");
	printf("    %-20s %-30s\n", "-w", "Enable cheats");
	printf("    %-20s %-30s\n", "-$", "Enable god mode");
	printf("    %-20s %-30s\n", "-^", "Enable god mode and debug tools");
	printf("    %-20s %-30s\n", "-v", "Highlight visibility");
	printf("    %-20s %-30s\n", "-i", "Ignore network timeout");
	printf("    %-20s %-30s\n", "-l <##> <##>", "Start in level as type");
	printf("    %-20s %-30s\n", "-m <##>", "Add debug monster, up to 10 allowed");
	printf("    %-20s %-30s\n", "-q <#>", "Force a certain quest");
	printf("    %-20s %-30s\n", "-r <##########>", "Set map seed");
	printf("    %-20s %-30s\n", "-t <##>", "Set current quest level");
	printf("    %-20s %-30s\n", "--allquests", "Force all quests to generate in a singleplayer game");
#endif
	printf("\nReport bugs at https://github.com/diasurgical/devilutionX/\n");
	diablo_quit(0);
}

static void diablo_parse_flags(int argc, char **argv)
{
	for (int i = 1; i < argc; i++) {
		if (strcasecmp("-h", argv[i]) == 0 || strcasecmp("--help", argv[i]) == 0) {
			print_help_and_exit();
		} else if (strcasecmp("--version", argv[i]) == 0) {
			printf("%s\n", gszProductName);
			diablo_quit(0);
		} else if (strcasecmp("--data-dir", argv[i]) == 0) {
			SetBasePath(argv[++i]);
		} else if (strcasecmp("--save-dir", argv[i]) == 0) {
			SetPrefPath(argv[++i]);
		} else if (strcasecmp("--config-dir", argv[i]) == 0) {
			SetConfigPath(argv[++i]);
		} else if (strcasecmp("-n", argv[i]) == 0) {
			_gbSkipIntro = true;
		} else if (strcasecmp("-x", argv[i]) == 0) {
			gbFullscreen = false;
#ifdef _DEBUG
		} else if (strcasecmp("-^", argv[i]) == 0) {
			debug_mode_key_inverted_v = TRUE;
			debug_mode_god_mode = TRUE;
		} else if (strcasecmp("-$", argv[i]) == 0) {
			debug_mode_god_mode = TRUE;
			/*
		} else if (strcasecmp("-b", argv[i]) == 0) {
			debug_mode_key_b = 1;
		*/
		} else if (strcasecmp("-i", argv[i]) == 0) {
			debug_mode_key_i = TRUE;
			/*
		} else if (strcasecmp("-j", argv[i]) == 0) {
			debug_mode_key_J_trigger = argv[++i];
		*/
		} else if (strcasecmp("-l", argv[i]) == 0) {
			leveldebug = TRUE;
			EnterLevel(SDL_atoi(argv[++i]));
			players[0]._pDunLevel = currLvl._dLevelIdx;
		} else if (strcasecmp("-m", argv[i]) == 0) {
			monstdebug = TRUE;
			DebugMonsters[debugmonsttypes++] = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-q", argv[i]) == 0) {
			questdebug = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-r", argv[i]) == 0) {
			setseed = SDL_atoi(argv[++i]);
		} else if (strcasecmp("-s", argv[i]) == 0) {
			debug_mode_key_s = TRUE;
		} else if (strcasecmp("-t", argv[i]) == 0) {
			leveldebug = TRUE;
			EnterLevel(SDL_atoi(argv[++i]));
		} else if (strcasecmp("-v", argv[i]) == 0) {
			visiondebug = TRUE;
		} else if (strcasecmp("-w", argv[i]) == 0) {
			debug_mode_key_w = TRUE;
		} else if (strcasecmp("--allquests", argv[i]) == 0) {
			allquests = true;
#endif
		} else {
			printf("unrecognized option '%s'\n", argv[i]);
			print_help_and_exit();
		}
	}
}

static void diablo_init_screen()
{
	MouseX = SCREEN_WIDTH / 2;
	MouseY = SCREEN_HEIGHT / 2;
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (!sgbControllerActive)
#endif
		SetCursorPos(MouseX, MouseY);
	static_assert(EMSG_NONE == 0, "ClrDiabloMsg is not called, because zero initialization cares about it.");
	// not the best place to call this, since it is an in-game 'system'
	// InitGameUI would be more appropriate place, but calling it once would
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

	init_create_window();
	_gbWasWindowInit = true;

	init_archives();
	_gbWasArchivesInit = true;

	UiInitialize();
	gbWasUiInit = true;

	diablo_init_screen();

	snd_init();
	gbSndInited = true;

	ui_sound_init();

	InitControls();
}

static void diablo_splash()
{
#ifndef HOSTONLY
	if (_gbSkipIntro)
		return;

	play_movie("gendata\\logo.smk", MOV_SKIP);

	if (getIniBool("Diablo", "Intro", true)) {
		play_movie(INTRO_ARCHIVE, MOV_SKIP);
		setIniInt("Diablo", "Intro", false);
	}

	UiTitleDialog();
#endif
}

static void diablo_deinit()
{
	NetClose();
	if (gbSndInited)
		effects_cleanup_sfx();
	if (gbWasUiInit)
		UiDestroy();
	if (_gbWasArchivesInit)
		init_cleanup();
	if (_gbWasWindowInit)
		dx_cleanup(); // Cleanup SDL surfaces stuff, so we have to do it before SDL_Quit().
	if (SDL_WasInit(SDL_INIT_EVERYTHING & ~SDL_INIT_HAPTIC) != 0)
		SDL_Quit();
}

int DiabloMain(int argc, char **argv)
{
	diablo_parse_flags(argc, argv);
	diablo_init();
	diablo_splash();
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
	stream_stop();
	music_stop();

	FreeLvlDungeon();
	FreeMissiles();
	FreeMonsters();
	FreeObjectGFX();
	FreeMonsterSnd();
	FreeTownerGFX();
}

static char ValidateSkill(BYTE sn, BYTE splType)
{
	char result;
	assert(sn != SPL_INVALID);

	if ((spelldata[sn].sFlags & myplr._pSkillFlags) != spelldata[sn].sFlags) {
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
		else if (GetSpellLevel(mypnum, sn) > 0)
			result = SPLFROM_MANA; // PlaySFX(sgSFXSets[SFXS_PLR_35][p->_pClass]);
		else
			result = SPLFROM_INVALID_LEVEL;
		break;
	case RSPLTYPE_SCROLL:
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
	char msf = 0, asf = 0;

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
			int dir = GetDirection(myplr._px, myplr._py, cursmx, cursmy);
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
			if (spelldata[atkSkill].sType != STYPE_NONE)
				NetSendCmdLocSkill(cursmx, cursmy, atkSkill, asf);
			else
				NetSendCmdLocAttack(cursmx, cursmy, atkSkill, asf);
			return;
		}
		if (pcursmonst != -1) {
			if (CanTalkToMonst(pcursmonst)) {
				NetSendCmdMonstAttack(CMD_ATTACKID, pcursmonst, (myplr._pSkillFlags & SFLAG_RANGED) ? SPL_RATTACK : SPL_ATTACK, SPLFROM_ABILITY);
			} else {
				if (spelldata[atkSkill].sType != STYPE_NONE)
					NetSendCmdMonstSkill(pcursmonst, atkSkill, asf);
				else if (myplr._pSkillFlags & SFLAG_RANGED)
					NetSendCmdMonstAttack(CMD_RATTACKID, pcursmonst, atkSkill, asf);
				else
					NetSendCmdMonstAttack(CMD_ATTACKID, pcursmonst, atkSkill, asf);
			}
			return;
		}
		if (pcursplr != PLR_NONE && myplr._pTeam != players[pcursplr]._pTeam) {
			if (spelldata[atkSkill].sType != STYPE_NONE)
				NetSendCmdPlrSkill(pcursplr, atkSkill, asf);
			else
				NetSendCmdPlrAttack(pcursplr, atkSkill, asf);
			return;
		}
		if (moveSkill == SPL_INVALID) {
			if (spelldata[atkSkill].sType != STYPE_NONE)
				NetSendCmdLocSkill(cursmx, cursmy, atkSkill, asf);
			else
				NetSendCmdLocAttack(cursmx, cursmy, atkSkill, asf);
			return;
		}
	} else if (moveSkill == SPL_INVALID) {
		const int *sfx;
		if (asf == SPLFROM_INVALID_MANA || msf == SPLFROM_INVALID_MANA)
			sfx = sgSFXSets[SFXS_PLR_35]; // no mana
		else
			sfx = sgSFXSets[SFXS_PLR_34]; // nothing to do/not ready
		PlaySFX(sfx[myplr._pClass]);
		return;
	}

	// assert(moveSkill != SPL_INVALID);
	// assert(spelldata[atkSkill].spCurs == CURSOR_NONE); -- TODO extend if there are targeted move skills

	if (pcursmonst != -1) {
		if (currLvl._dType == DTYPE_TOWN) {
			NetSendCmdLocParam1(CMD_TALKXY, cursmx, cursmy, pcursmonst);
			return;
		}
		// TODO: extend TALKXY?
		if (CanTalkToMonst(pcursmonst)) {
			NetSendCmdMonstAttack(CMD_ATTACKID, pcursmonst, (myplr._pSkillFlags & SFLAG_RANGED) ? SPL_RATTACK : SPL_ATTACK, SPLFROM_ABILITY);
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
		bool bNear = abs(myplr._px - cursmx) < 2 && abs(myplr._py - cursmy) < 2;
		if (moveSkill == SPL_WALK || (bNear && objects[pcursobj]._oBreak == OBM_BREAKABLE)) {
			NetSendCmdLocParam1(CMD_OPOBJXY, cursmx, cursmy, pcursobj);
			return;
		}
		//return; // TODO: proceed in case moveSkill != SPL_WALK?
	}
	if (moveSkill != SPL_WALK) {
		// TODO: check if cursmx/y == _px/y ?
		NetSendCmdLocSkill(cursmx, cursmy, moveSkill, msf);
		return;
	}

	if (pcursitem != ITEM_NONE) {
		NetSendCmdLocParam1(gbInvflag ? CMD_GOTOGETITEM : CMD_GOTOAGETITEM, cursmx, cursmy, pcursitem);
		return;
	}

	NetSendCmdLoc(CMD_WALKXY, cursmx, cursmy);
}

void ActionBtnCmd(bool bShift)
{
	assert(pcurs == CURSOR_HAND);

	DoActionBtnCmd(myplr._pMoveSkill, myplr._pMoveSkillType,
		myplr._pAtkSkill, myplr._pAtkSkillType, bShift);
}

bool TryIconCurs(bool bShift)
{
	switch (pcurs) {
	case CURSOR_IDENTIFY:
	case CURSOR_REPAIR:
	case CURSOR_RECHARGE:
	case CURSOR_OIL:
		if (pcursinvitem != INVITEM_NONE) {
			NetSendCmdItemSkill(pcursinvitem, gbTSpell, gbTSplFrom);
		}
		break;
	case CURSOR_DISARM:
		if (pcursobj != OBJ_NONE) {
			if (!bShift ||
			 (abs(myplr._px - cursmx) < 2 && abs(myplr._py - cursmy) < 2)) {
				NetSendCmdLocParam1(CMD_DISARMXY, cursmx, cursmy, pcursobj);
				return true;
			}
		}
		break;
	case CURSOR_TELEKINESIS:
		if (pcursobj != OBJ_NONE)
			NetSendCmdParam1(CMD_OPOBJT, pcursobj);
		if (pcursitem != ITEM_NONE)
			NetSendCmdGItem(CMD_AUTOGETITEM, pcursitem);
		if (pcursmonst != -1 && !MonTalker(pcursmonst))
			NetSendCmdParam1(CMD_KNOCKBACK, pcursmonst);
		break;
	case CURSOR_RESURRECT:
		if (pcursplr != PLR_NONE) {
			NetSendCmdPlrSkill(pcursplr, gbTSpell, gbTSplFrom);
		}
		break;
	case CURSOR_TELEPORT: {
		if (pcursmonst != -1)
			NetSendCmdMonstSkill(pcursmonst, gbTSpell, gbTSplFrom);
		else if (pcursplr != PLR_NONE)
			NetSendCmdPlrSkill(pcursplr, gbTSpell, gbTSplFrom);
		else
			NetSendCmdLocSkill(cursmx, cursmy, gbTSpell, gbTSplFrom);
	} break;
	case CURSOR_HEALOTHER:
		if (pcursplr != PLR_NONE) {
			NetSendCmdPlrSkill(pcursplr, gbTSpell, gbTSplFrom);
		}
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
	assert(!gmenu_is_active() || !gmenu_left_mouse(true));
	assert(gnTimeoutCurs == CURSOR_NONE);
	// assert(!gbTalkflag || !control_check_talk_btn());
	assert(!gbDeathflag);
	assert(!gbGamePaused);
	assert(!gbDoomflag);

	if (gbSkillListFlag) {
		SetSkill(bShift, false);
		return;
	}

	if (stextflag != STORE_NONE) {
		CheckStoreBtn();
		return;
	}

	if (gmenu_is_active()) {
		DoLimitedPanBtn();
		return;
	}

	if (DoPanBtn()) {
		return;
	}

	if (TryIconCurs(bShift))
		return;

	if (gbQtextflag) {
		gbQtextflag = false;
		stream_stop();
		return;
	}

	if (gbQuestlog && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT) {
		CheckQuestlog();
		return;
	}

	if (MouseX <= InvRect[SLOTXY_BELT_LAST].X + INV_SLOT_SIZE_PX && MouseY >= SCREEN_HEIGHT - InvRect[SLOTXY_BELT_FIRST].Y - INV_SLOT_SIZE_PX) {
		// in belt
		// assert(!DoPanBtn());
		CheckBeltClick(bShift);
		return;
	}

	if (gbChrflag && MouseX < SPANEL_WIDTH && MouseY < SPANEL_HEIGHT) {
		CheckChrBtns();
		return;
	}

	if (gbInvflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		// in inventory
		CheckInvClick(bShift);
		return;
	}

	if (gbSbookflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		SelectBookSkill(bShift, false);
		return;
	}

	if (gbTeamFlag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		CheckTeamClick(bShift);
		return;
	}

	if (pcurs >= CURSOR_FIRSTITEM) {
		DropItem();
		return;
	}

	ActionBtnCmd(bShift);
}

void AltActionBtnCmd(bool bShift)
{
	assert(pcurs == CURSOR_HAND);

	DoActionBtnCmd(myplr._pAltMoveSkill, myplr._pAltMoveSkillType,
		myplr._pAltAtkSkill, myplr._pAltAtkSkillType, bShift);
}

static void AltActionBtnDown(bool bShift)
{
	assert(!gmenu_is_active());
	assert(gnTimeoutCurs == CURSOR_NONE);
	assert(!gbGamePaused);
	assert(!gbDoomflag);

	if (myplr._pInvincible)
		return;

	if (gbSkillListFlag) {
		SetSkill(bShift, true);
		return;
	}

	if (stextflag != STORE_NONE)
		return;

	if (TryIconCurs(bShift))
		return;
	if (pcurs >= CURSOR_FIRSTITEM) {
		DropItem();
		return;
	}

	if (gbQuestlog) {
		gbQuestlog = false;
		return;
	}

	if (gbQtextflag) {
		gbQtextflag = false;
		stream_stop();
		return;
	}

	if (pcursinvitem != INVITEM_NONE && InvUseItem(pcursinvitem))
		return;

	if (gbSbookflag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
		SelectBookSkill(bShift, true);
		return;
	}

	if (gbTeamFlag && MouseX > RIGHT_PANEL && MouseY < SPANEL_HEIGHT) {
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
			CheckBtnUp();
		if (gbChrbtnactive)
			ReleaseChrBtns();
		if (gbLvlbtndown)
			ReleaseLvlBtn();
		if (stextflag != STORE_NONE)
			ReleaseStoreBtn();
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

	if (gbDoomflag) {
		doom_close();
		rv = true;
	}
	if (gbHelpflag) {
		gbHelpflag = false;
		rv = true;
	}

	if (gbQtextflag) {
		gbQtextflag = false;
		stream_stop();
		rv = true;
	} else if (stextflag != STORE_NONE) {
		STextESC();
		rv = true;
	}

	if (currmsg != EMSG_NONE) {
		msgdelay = 0;
		rv = true;
	}
	if (gbTalkflag) {
		control_reset_talk();
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
	if (pcurs != CURSOR_HAND && pcurs < CURSOR_FIRSTITEM) {
		NewCursor(CURSOR_HAND);
		rv = true;
	}

	return rv;
}

void ClearPanels()
{
	gbHelpflag = false;
	gbInvflag = false;
	gbChrflag = false;
	gbSbookflag = false;
	gbTeamFlag = false;
	gbSkillListFlag = false;
	gbDropGoldFlag = false;
	gbQuestlog = false;
}

static void ClearUI()
{
	ClearPanels();
	if (gbQtextflag && currLvl._dType == DTYPE_TOWN) {
		gbQtextflag = false;
		stream_stop();
	}
	gbAutomapflag = false;
	msgdelay = 0;
	gabPanbtn[PANBTN_MAINMENU] = false;
	gamemenu_off();
	//doom_close();
}

static void PressKey(int vkey)
{
	if (gmenu_is_active()) {
		if (gmenu_presskeys(vkey))
			return;
	}
	if (gbTalkflag) {
		if (control_presskeys(vkey))
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

	if (vkey == DVL_VK_RETURN && GetAsyncKeyState(DVL_VK_MENU)) {
		ToggleFullscreen();
		return;
	}

	int transKey = WMButtonInputTransTbl[vkey];
	if (gbDeathflag) {
		if (vkey == DVL_VK_RETURN) {
			control_type_message();
		} else if (vkey == DVL_VK_LBUTTON) {
			DoLimitedPanBtn();
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

	if (gbDoomflag) {
		doom_close();
		return;
	}

	switch (transKey) {
	case ACT_NONE:
		break;
	case ACT_ACT:
		if (!gbActionBtnDown) {
			gbActionBtnDown = true;
			guLastABD = SDL_GetTicks();
			ActionBtnDown(GetAsyncKeyState(DVL_VK_SHIFT) != 0);
		}
		break;
	case ACT_ALTACT:
		if (!gbAltActionBtnDown) {
			gbAltActionBtnDown = true;
			guLastAABD = SDL_GetTicks();
			AltActionBtnDown(GetAsyncKeyState(DVL_VK_SHIFT) != 0);
		}
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
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_INVENTORY);
		}
		break;
	case ACT_CHAR:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_CHARINFO);
		}
		break;
	case ACT_SKLBOOK:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_SPELLBOOK);
		}
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
		HandlePanBtn(PANBTN_AUTOMAP);
		break;
	case ACT_MAPZ_IN:
		if (gbAutomapflag) {
			AutomapZoomIn();
		}
		break;
	case ACT_MAPZ_OUT:
		if (gbAutomapflag) {
			AutomapZoomOut();
		}
		break;
	case ACT_CLEARUI:
		ClearUI();
		break;
	case ACT_UP:
		if (stextflag != STORE_NONE) {
			STextUp();
		} else if (gbQuestlog) {
			QuestlogUp();
		} else if (gbHelpflag) {
			HelpScrollUp();
		} else if (gbAutomapflag) {
			AutomapUp();
		}
		break;
	case ACT_DOWN:
		if (stextflag != STORE_NONE) {
			STextDown();
		} else if (gbQuestlog) {
			QuestlogDown();
		} else if (gbHelpflag) {
			HelpScrollDown();
		} else if (gbAutomapflag) {
			AutomapDown();
		}
		break;
	case ACT_LEFT:
		if (gbAutomapflag) {
			AutomapLeft();
		}
		break;
	case ACT_RIGHT:
		if (gbAutomapflag) {
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
		} else if (gbQuestlog) {
			QuestlogEnter();
		} else {
			control_type_message();
		}
		break;
	case ACT_TEAM:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_TEAMBOOK);
		}
		break;
	case ACT_QUESTS:
		if (stextflag == STORE_NONE) {
			HandlePanBtn(PANBTN_QLOG);
		}
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
		EventPlrMsg(gszProductName);
		if (!GetAsyncKeyState(DVL_VK_SHIFT)) {
			if (!IsLocalGame) {
				EventPlrMsg(szGameName);
				if (szGamePassword[0] != '\0') {
					char desc[128];
					snprintf(desc, sizeof(desc), "password: %s", szGamePassword);
					EventPlrMsg(desc);
				}
			}
		} else {
			const char *difficulties[3] = { "Normal", "Nightmare", "Hell" };
			EventPlrMsg(difficulties[gnDifficulty]);
		}
		break;
	case ACT_HELP:
		if (gbHelpflag) {
			gbHelpflag = false;
		} else if (stextflag == STORE_NONE) {
			ClearUI();
			DisplayHelp();
		}
		break;
	case ACT_PAUSE:
		break;
	case ACT_ESCAPE:
		break;
	case ACT_TOOLTIP:
		gbShowTooltip = !gbShowTooltip;
		break;
	default:
		ASSUME_UNREACHABLE
	}

#ifdef _DEBUG
	if (vkey == DVL_VK_F2) {
	}
	else if (vkey == DVL_VK_F3) {
		if (pcursitem != ITEM_NONE) {
			snprintf(
			    gbNetMsg,
				sizeof(gbNetMsg),
			    "IDX = %i  :  Seed = %i  :  CF = %i",
			    items[pcursitem]._iIdx,
			    items[pcursitem]._iSeed,
			    items[pcursitem]._iCreateInfo);
			NetSendCmdString(1 << mypnum);
		}
		snprintf(gbNetMsg, sizeof(gbNetMsg), "Numitems : %i", numitems);
		NetSendCmdString(1 << mypnum);
	}
	else if (vkey == DVL_VK_F4) {
		PrintDebugQuest();
	}
#endif
}

/**
 * @internal `return` must be used instead of `break` to be bin exact as C++
 */
static void PressChar(WPARAM vkey)
{
	if (gmenu_is_active()) {
		return;
	}
	if (gbTalkflag) {
		if (control_talk_last_key(vkey))
			return;
	}
#ifdef _DEBUG
	if (gnTimeoutCurs != CURSOR_NONE || gbDeathflag)
		return;

	if (gbGamePaused) {
		return;
	}
	switch (vkey) {
/*	case ')':
	case '0':
		if (debug_mode_key_inverted_v) {
			if (arrowdebug > 2) {
				arrowdebug = 0;
			}
			if (arrowdebug == 0) {
				myplr._pIFlags &= ~ISPL_FIRE_ARROWS;
				myplr._pIFlags &= ~ISPL_LIGHT_ARROWS;
			}
			if (arrowdebug == 1) {
				myplr._pIFlags |= ISPL_FIRE_ARROWS;
			}
			if (arrowdebug == 2) {
				myplr._pIFlags |= ISPL_LIGHT_ARROWS;
			}
			arrowdebug++;
		}
		break;*/
	case '9':
		if (debug_mode_key_inverted_v || debug_mode_key_w) {
			NetSendCmd(CMD_CHEAT_EXPERIENCE);
		}
		break;
	case ':':
		if (currLvl._dLevelIdx == 0 && debug_mode_key_w) {
			SetAllSpellsCheat();
		}
		break;
	case '[':
		if (currLvl._dLevelIdx == 0 && debug_mode_key_w) {
			TakeGoldCheat();
		}
		break;
	case ']':
		if (currLvl._dLevelIdx == 0 && debug_mode_key_w) {
			MaxSpellsCheat();
		}
		break;
	case 'a':
		if (debug_mode_key_inverted_v) {
			myplr._pSkillLvl[myplr._pAltAtkSkill]++;
		}
		break;
	case 'D':
		PrintDebugPlayer(true);
		break;
	case 'd':
		PrintDebugPlayer(false);
		break;
	case 'L':
	case 'l':
		if (debug_mode_key_inverted_v) {
			ToggleLighting();
		}
		break;
	case 'M':
		NextDebugMonster();
		break;
	case 'm':
		GetDebugMonster();
		break;
	case 'R':
	case 'r':
		snprintf(gbNetMsg, sizeof(gbNetMsg), "seed = %i", glSeedTbl[currLvl._dLevelIdx]);
		NetSendCmdString(1 << mypnum);
		break;
	case 'T':
	case 't':
		if (debug_mode_key_inverted_v) {
			snprintf(gbNetMsg, sizeof(gbNetMsg), "PX = %i  PY = %i", myplr._px, myplr._py);
			NetSendCmdString(1 << mypnum);
			snprintf(gbNetMsg, sizeof(gbNetMsg), "CX = %i  CY = %i  DP = %i", cursmx, cursmy, dungeon[cursmx][cursmy]);
			NetSendCmdString(1 << mypnum);
		}
		break;
	case '|':
		if (currLvl._dLevelIdx == 0 && debug_mode_key_w) {
			GiveGoldCheat();
		}
		break;
	case '~':
		if (currLvl._dLevelIdx == 0 && debug_mode_key_w) {
			StoresCheat();
		}
		break;
	}
#endif
}

static void GetMousePos(WPARAM wParam)
{
	MouseX = (int16_t)(wParam & 0xFFFF);
	MouseY = (int16_t)((wParam >> 16) & 0xFFFF);
}

static void UpdateActionBtnState(int vKey, bool dir)
{
	if (vKey == actionBtnKey)
		gbActionBtnDown = dir;
	if (vKey == altActionBtnKey)
		gbAltActionBtnDown = dir;
}

void DisableInputWndProc(UINT uMsg, WPARAM wParam)
{
	switch (uMsg) {
	case DVL_WM_KEYDOWN:
		UpdateActionBtnState(wParam, true);
		return;
	case DVL_WM_KEYUP:
		UpdateActionBtnState(wParam, false);
		return;
	case DVL_WM_CHAR:
	//case DVL_WM_SYSKEYDOWN:
	//case DVL_WM_SYSCOMMAND:
		return;
	case DVL_WM_MOUSEMOVE:
		GetMousePos(wParam);
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
	}

	MainWndProc(uMsg);
}

static void GameWndProc(UINT uMsg, WPARAM wParam)
{
	switch (uMsg) {
	case DVL_WM_KEYDOWN:
		PressKey(wParam);
		return;
	case DVL_WM_KEYUP:
		ReleaseKey(wParam);
		return;
	case DVL_WM_CHAR:
		PressChar(wParam);
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
		gbRunGame = false;
		gbRunGameResult = false;
		return;
	case DVL_WM_MOUSEMOVE:
		GetMousePos(wParam);
		if (gmenu_is_active())
			gmenu_on_mouse_move();
		return;
	case DVL_WM_LBUTTONDOWN:
		GetMousePos(wParam);
		PressKey(DVL_VK_LBUTTON);
		return;
	case DVL_WM_LBUTTONUP:
		GetMousePos(wParam);
		ReleaseKey(DVL_VK_LBUTTON);
		return;
	case DVL_WM_RBUTTONDOWN:
		GetMousePos(wParam);
		PressKey(DVL_VK_RBUTTON);
		return;
	case DVL_WM_RBUTTONUP:
		GetMousePos(wParam);
		ReleaseKey(DVL_VK_RBUTTON);
		return;
	case DVL_WM_CAPTURECHANGED:
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		break;
	case DVL_DWM_NEXTLVL:
	case DVL_DWM_PREVLVL:
	case DVL_DWM_RTNLVL:
	case DVL_DWM_SETLVL:
	case DVL_DWM_WARPLVL:
	case DVL_DWM_TWARPDN:
	case DVL_DWM_TWARPUP:
	case DVL_DWM_RETOWN:
		if (IsMultiGame)
			pfile_write_hero(false);
		nthread_run();
		PaletteFadeOut();
		sound_stop();
		music_stop();
		gbActionBtnDown = false;
		gbAltActionBtnDown = false;
		ShowCutscene(uMsg);
		gbRedrawFlags = REDRAW_ALL;
		scrollrt_draw_game();
		LoadPWaterPalette();
		if (gbRunGame)
			PaletteFadeIn();
		// process packets arrived during LoadLevel and disable nthread
		nthread_finish();
		return;
	}

	MainWndProc(uMsg);
}

static bool ProcessInput()
{
	if (gbGamePaused) {
		return false;
	}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	plrctrls_every_frame();
#endif

	if (gmenu_is_active()) {
		return IsMultiGame;
	}

	if (gnTimeoutCurs == CURSOR_NONE) {
#if HAS_TOUCHPAD == 1
		finish_simulated_mouse_clicks(MouseX, MouseY);
#endif
		CheckCursMove();
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
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
	if (gbProcessPlayers) {
		ProcessPlayers();
	}
	if (currLvl._dType != DTYPE_TOWN) {
		ProcessMonsters();
		ProcessObjects();
	} else {
		ProcessTowners();
	}
	multi_mis_seeds();
	ProcessMissiles();
	ProcessItems();
	ProcessLightList();
	ProcessVisionList();

#ifdef _DEBUG
	if (debug_mode_key_inverted_v && GetAsyncKeyState(DVL_VK_SHIFT)) {
		ScrollView();
	}
#endif

	sound_update();
	ClearPlrMsg();
	CheckTriggers();
	CheckQuests();
	pfile_update(false);
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
				gnTimeoutCurs = pcurs;
				NewCursor(CURSOR_HOURGLASS);
				//gbRedrawFlags = REDRAW_ALL;
			}
			//scrollrt_draw_screen(true);
			break;
		}
		if (gnTimeoutCurs != CURSOR_NONE) {
			NewCursor(gnTimeoutCurs);
			gnTimeoutCurs = CURSOR_NONE;
			//gbRedrawFlags = REDRAW_ALL;
		}
		//if (ProcessInput()) {
			game_logic();
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
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
		lighting_color_cycling();
#ifdef HELLFIRE
	else if (currLvl._dType == DTYPE_CRYPT)
		palette_update_crypt();
	else if (currLvl._dType == DTYPE_NEST)
		palette_update_hive();
#endif
	else if (currLvl._dType == DTYPE_CAVES)
		palette_update_caves();
}

static WNDPROC InitGameUI()
{
	int i;

	InitAutomapOnce(); // values
	InitHelp(); // values
	InitControlPan(); // gfx + values
	InitText(); // gfx
	InitInv(); // gfx + values
	InitGMenu(); // gfx
	InitQuestGFX(); // gfx + values
	InitQuestText(); // values
	InitStoresOnce(); // values (some stored in savefiles)
	for (i = 0; i < (IsLocalGame ? 1 : MAX_PLRS); i++)
		InitPlrGFXMem(i); // gfx
	InitItemGFX(); // gfx + values (some stored in savefiles)

	gbDeathflag = false;
	gbZoomInFlag = false;
	InitCursorGFX(); // gfx + values
	ScrollInfo._sdx = 0;
	ScrollInfo._sdy = 0;
	ScrollInfo._sxoff = 0;
	ScrollInfo._syoff = 0;
	ScrollInfo._sdir = SDIR_NONE;

#ifdef _DEBUG
	LoadDebugGFX();
#endif

	gnTimeoutCurs = CURSOR_NONE;
	gbActionBtnDown = false;
	gbAltActionBtnDown = false;
	gbRunGame = true;
	gbProcessPlayers = true;
	gbRunGameResult = true;

	return SetWindowProc(GameWndProc);
}

static void FreeGameUI()
{
	int i;

	FreeControlPan();
	FreeText();
	FreeInvGFX();
	FreeGMenu();
	FreeQuestGFX();

	for (i = 0; i < MAX_PLRS; i++)
		FreePlayerGFX(i);

	FreeItemGFX();
	FreeCursorGFX();
#ifdef _DEBUG
	FreeDebugGFX();
#endif
	FreeLevelMem();
}

static void run_game()
{
	WNDPROC saveProc;
	MSG msg;

	saveProc = InitGameUI();

	nthread_run();
	music_stop();
	ShowCutscene(DVL_DWM_NEWGAME);
	CalcViewportGeometry();
	InitLevelCursor();
	// process remaining packets of delta-load
	RunDeltaPackets();
	gbRedrawFlags = REDRAW_ALL;
	scrollrt_draw_game();
	LoadPWaterPalette();
	PaletteFadeIn();
	gbRedrawFlags = REDRAW_ALL;
	// process packets arrived during LoadLevel / delta-load and disable nthread
	nthread_finish();

#ifdef GPERF_HEAP_FIRST_GAME_ITERATION
	unsigned run_game_iteration = 0;
#endif
	while (TRUE) {
		while (gbRunGame && PeekMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
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
	else
		pfile_flush(true);

	PaletteFadeOut();
	//NewCursor(CURSOR_NONE);
	//ClearScreenBuffer();
	//scrollrt_draw_screen(true);
	saveProc = SetWindowProc(saveProc);
	assert(saveProc == GameWndProc);
	FreeGameUI();

	if (gbCineflag) {
		gbCineflag = false;
		DoEnding();
	}
}

bool StartGame(bool bSinglePlayer)
{
	gbSelectProvider = true;
	gbSelectHero = true;

	while (TRUE) {
		if (!NetInit(bSinglePlayer)) {
			gbRunGameResult = true;
			break;
		}
#ifndef HOSTONLY
		// Save 2.8 MiB of RAM by freeing all main menu resources before starting the game.
		UiDestroy();

		run_game();
		if (!gbRunGameResult)
			break;
		// If the player left the game into the main menu,
		// initialize main menu resources.
		UiInitialize();
		pfile_read_hero_from_save();
#endif
	}

	return gbRunGameResult;
}

DEVILUTION_END_NAMESPACE
