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
// current mouse position
extern POS32 MousePos;
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
extern bool gbShowTooltip;

void FreeLevelMem();
bool StartGame(bool bSinglePlayer);
void diablo_quit(int exitStatus);
int DiabloMain(int argc, char** argv);
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
void ActionBtnCmd(bool bShift);
void AltActionBtnCmd(bool bShift);
#endif
bool TryIconCurs(bool bShift);
bool PressEscKey();
void ClearPanels();
void DisableInputWndProc(UINT uMsg, WPARAM wParam);
void game_logic();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLO_H__ */
