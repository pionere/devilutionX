/**
 * @file control.h
 *
 * Interface of the character and main control panels
 */
#ifndef __CONTROL_H__
#define __CONTROL_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CTRL_PANEL_LINES 4

extern BOOL dropGoldFlag;
extern BOOL lvlbtndown;
extern int dropGoldValue;
extern BOOL chrbtnactive;
extern BYTE *pPanelText;
extern char infoclr;
extern char tempstr[256];
extern int sbooktab;
extern int pSplType;
extern int initialDropGoldIndex;
extern BOOL talkflag;
extern BOOL sbookflag;
extern BOOL chrflag;
extern char infostr[256];
extern BOOL panelflag;
extern int initialDropGoldValue;
extern BOOL panbtndown;
extern BOOL spselflag;

void DrawSpeedBook();
void SetSpell();
void SetSpeedSpell(int slot);
void ToggleSpell(int slot);
void PrintChar(int sx, int sy, int nCel, char col);
void PrintString(int x, int y, int endX, const char *pszStr, BOOL center, int col, int kern);
void AddPanelString(const char *str, BOOL just);
void ClearPanel();
void DrawPanelBox(int x, int y, int w, int h, int sx, int sy);
void DrawLifeFlask();
void DrawManaFlask();
void DrawSpell();
void InitControlPan();
void DrawCtrlPan();
void DrawCtrlBtns();
void DoSpeedBook();
void DoPanBtn();
void control_check_btn_press();
void DoAutoMap();
void CheckPanelInfo();
void CheckBtnUp();
void FreeControlPan();
BOOL control_WriteStringToBuffer(BYTE *str);
void DrawInfoBox();
void PrintGameStr(int x, int y, const char *str, int color);
void DrawChr();
BOOL CheckLvlBtn();
void ReleaseLvlBtn();
void DrawLevelUpIcon();
void DrawInfoStr();
BOOL CheckChrBtns();
void ReleaseChrBtns();
void DrawDurIcon();
void RedBack();
void DrawSpellBook();
void CheckSBook();
const char *get_pieces_str(int nGold);
void DrawGoldSplit(int amount);
void control_drop_gold(char vkey);
void DrawTalkPan();
BOOL control_check_talk_btn();
void control_release_talk_btn();
void control_type_message();
void control_reset_talk();
BOOL control_talk_last_key(int vkey);
BOOL control_presskeys(int vkey);

/* rdata */
extern const BYTE fontframe[128];
extern const BYTE fontkern[68];
extern const BYTE gbFontTransTbl[256];

/* data */

extern RECT32 ChrBtnsRect[4];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CONTROL_H__ */
