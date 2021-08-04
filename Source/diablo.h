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

extern DWORD glSeedTbl[NUM_LEVELS];
extern int MouseX;
extern int MouseY;
extern bool gbSndInited;
extern bool gbRunGame;
extern bool gbRunGameResult;
extern bool gbZoomInFlag;
extern bool gbProcessPlayers;
extern bool gbLoadGame;
extern bool gbCineflag;
extern int gbRedrawFlags;
extern bool gbGamePaused;
#ifdef HELLFIRE
extern bool gbUseNestArt;
#endif
extern bool gbActionBtnDown;
extern bool gbAltActionBtnDown;
extern int gnTicksRate;
extern unsigned gnTickDelay;

void FreeLevelMem();
bool StartGame(bool bSinglePlayer);
void diablo_quit(int exitStatus);
int DiabloMain(int argc, char **argv);
void AltActionBtnCmd(bool bShift);
bool TryIconCurs(bool bShift);
bool PressEscKey();
void ClearPanels();
void DisableInputWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
void GameWndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
void diablo_color_cyc_logic();
void game_logic();

/* rdata */

extern bool gbFullscreen;
extern bool gbShowTooltip;
#ifdef _DEBUG
extern int DebugMonsters[10];
extern BOOL visiondebug;
extern BOOL lightflag;
extern BOOL light4flag;
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
