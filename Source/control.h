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

extern BOOL dropGoldFlag;
extern BOOL lvlbtndown;
extern int dropGoldValue;
extern BOOL chrbtnactive;
extern BYTE *pPanelText;
extern BYTE infoclr;
extern char tempstr[256];
extern int sbooktab;
extern int initialDropGoldIndex;
extern BOOL talkflag;
extern BOOL sbookflag;
extern BOOL chrflag;
extern char infostr[256];
extern int initialDropGoldValue;
extern BOOL panbtn[NUM_PANBTNS];
extern int numpanbtns;
extern BOOL spselflag;

void DrawSpeedBook();
void SetRSpell();
void SetSpeedSpell(int slot);
void ToggleSpell(int slot);
void PrintChar(int sx, int sy, int nCel, char col);
void PrintString(int x, int y, int endX, const char *pszStr, BOOL center, BYTE col, int kern);
void DrawLifeFlask();
void DrawManaFlask();
void DrawRSpell();
void InitControlPan();
void DrawCtrlBtns();
void DoSpeedBook();
BOOL DoPanBtn();
void DoLimitedPanBtn();
void CheckBtnUp();
void HandlePanBtn(int i);
void HandleSpellBtn();
void FreeControlPan();
BOOL control_WriteStringToBuffer(BYTE *str);
void PrintGameStr(int x, int y, const char *str, BYTE color);
void DrawChr();
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

extern const RECT32 ChrBtnsRect[4];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CONTROL_H__ */
