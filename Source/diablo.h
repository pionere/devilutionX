/**
 * @file diablo.h
 *
 * Interface of the main game initialization functions.
 */
#ifndef __DIABLO_H__
#define __DIABLO_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t glSeedTbl[NUM_LEVELS];
extern int MouseX;
extern int MouseY;
extern bool gbWasUiInit;
extern bool gbSndInited;
extern bool gbRunGame;
extern bool gbRunGameResult;
extern bool gbZoomInFlag;
extern bool gbLoadGame;
extern bool gbCineflag;
extern BYTE gbGameLogicProgress;
extern int gbRedrawFlags;
extern bool gbGamePaused;
extern BYTE gbDeathflag;
extern bool gbActionBtnDown;
extern bool gbAltActionBtnDown;
extern int gnTicksRate;
extern unsigned gnTickDelay;
extern int gnTimeoutCurs;

void FreeLevelMem();
bool StartGame(bool bSinglePlayer);
void diablo_quit(int exitStatus);
int DiabloMain(int argc, char **argv);
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
void ActionBtnCmd(bool bShift);
void AltActionBtnCmd(bool bShift);
#endif
bool TryIconCurs(bool bShift);
bool PressEscKey();
void ClearPanels();
void DisableInputWndProc(UINT uMsg, WPARAM wParam);
void game_logic();

/* rdata */

extern bool gbFullscreen;
extern bool gbShowTooltip;
#if DEBUG_MODE
extern int DebugMonsters[10];
extern BOOL visiondebug;
extern bool lightflag;
extern BOOL leveldebug;
extern BOOL monstdebug;
extern int setseed;
extern int debugmonsttypes;
extern bool allquests;
extern int questdebug;
extern int debug_mode_key_w;
extern int debug_mode_key_inverted_v;
extern BOOL debug_mode_god_mode;
extern int debug_mode_key_i;
#endif

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLO_H__ */
