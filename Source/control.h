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

extern bool gbDropGoldFlag;
extern int dropGoldValue;
extern int initialDropGoldValue;
extern int initialDropGoldIndex;
extern bool gbLvlbtndown;
extern bool gbChrbtnactive;
extern BYTE *pPanelText;
extern BYTE infoclr;
extern char tempstr[256];
extern int sbooktab;
extern bool gbTalkflag;
extern bool gbSbookflag;
extern bool gbChrflag;
extern char infostr[256];
extern bool gabPanbtn[NUM_PANBTNS];
extern int numpanbtns;
extern bool gbSkillListFlag;

void DrawSkillList();
void SetSkill(bool shift, bool altSkill);
void SetSkillHotKey(int slot, bool altSkill);
void SelectHotKeySkill(int slot, bool altSkill);
void PrintChar(int sx, int sy, int nCel, char col);
void PrintString(int x, int y, int endX, const char *pszStr, bool center, BYTE col, int kern);
void DrawLifeFlask();
void DrawManaFlask();
void DrawSkillIcons();
void InitControlPan();
void DrawCtrlBtns();
void DoSkillList(bool altSkill);
bool DoPanBtn();
void DoLimitedPanBtn();
void CheckBtnUp();
void HandlePanBtn(int i);
void HandleSkillBtn(bool altSkill);
void FreeControlPan();
bool control_WriteStringToBuffer(BYTE *str);
void PrintGameStr(int x, int y, const char *str, BYTE color);
void DrawChr();
void ReleaseLvlBtn();
void DrawLevelUpIcon();
void DrawInfoStr();
bool CheckChrBtns();
void ReleaseChrBtns();
void DrawDurIcon();
void RedBack();
void DrawSpellBook();
void SelectBookSkill(bool shift, bool altSkill);
const char *get_pieces_str(int nGold);
void DrawGoldSplit(int amount);
void control_drop_gold(char vkey);
void DrawTalkPan();
bool control_check_talk_btn();
void control_release_talk_btn();
void control_type_message();
void control_reset_talk();
bool control_talk_last_key(int vkey);
bool control_presskeys(int vkey);

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
