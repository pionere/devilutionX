/**
 * @file diablo.h
 *
 * Interface of the main game initialization functions.
 */
#ifndef __DIABLO_H__
#define __DIABLO_H__

DEVILUTION_BEGIN_NAMESPACE

#define ACTBTN_MASK(btn)     (1 << (btn))

#ifdef __cplusplus
extern "C" {
#endif

// current mouse position
extern POS32 MousePos;
extern bool gbWasUiInit;
extern bool gbSndInited;
extern bool gbRunGame;
extern bool gbRunGameResult;
extern bool gbZoomInFlag;
extern bool gbCineflag;
extern BYTE gbGameLogicProgress;
extern int gbRedrawFlags;
extern Uint32 gnGamePaused;
extern BYTE gbDeathflag;
extern unsigned gbActionBtnDown;
extern unsigned gbModBtnDown;
extern int gnTicksRate;
extern unsigned gnTickDelay;
extern int gnTimeoutCurs;
extern bool gbShowTooltip;

void FreeLevelMem();
bool StartGame(bool bSinglePlayer);
void diablo_pause_game(bool pause);
void diablo_quit(int exitStatus);
int DiabloMain(int argc, char** argv);
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
void InputBtnDown(int transKey);
#endif
bool PressEscKey();
void ClearPanels();
void GameWndProc(const Dvl_Event* event);
void DisableInputWndProc(const Dvl_Event* event);
void game_logic();

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __DIABLO_H__ */
