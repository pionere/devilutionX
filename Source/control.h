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

/** Scrollbar graphics */
extern CelImageBuf* pSTextSlidCels;

extern int gnHPPer;
extern int gnManaPer;
extern bool gbDropGoldFlag;
extern int dropGoldValue;
extern int initialDropGoldValue;
extern BYTE initialDropGoldIndex;
extern bool gbChrbtnactive;
extern BYTE infoclr;
extern char infostr[256];
extern char tempstr[256];
extern unsigned guBooktab;
extern bool gbTalkflag;
extern char sgszTalkMsg[MAX_SEND_STR_LEN];
extern int gnNumActiveWindows;
extern char gaActiveWindows[NUM_WNDS];
extern bool gbLvlUp;
extern bool gbLvlbtndown;
extern unsigned guTeamInviteRec;
extern unsigned guTeamInviteSent;
extern unsigned guTeamMute;
extern bool gabPanbtn[NUM_PANBTNS];
extern int numpanbtns;
extern bool gbSkillListFlag;

void DrawSkillList();
void SetSkill(bool shift, bool altSkill);
void SetSkillHotKey(int slot, bool altSkill);
void SelectHotKeySkill(int slot, bool altSkill);
void DrawLifeFlask();
void DrawManaFlask();
void DrawSkillIcons();
void InitControlPan();
void DrawCtrlBtns();
bool ToggleWindow(char idx);
void DoSkillList(bool altSkill);
bool DoPanBtn();
void DoLimitedPanBtn();
void CheckBtnUp();
void HandlePanBtn(int i);
void HandleSkillBtn(bool altSkill);
void FreeControlPan();
void DrawChr();
void ReleaseLvlBtn();
void DrawLevelUpIcon();
void DrawInfoStr();
bool CheckChrBtns();
void ReleaseChrBtns();
void DrawTextBox();
void DrawSTextBox(int x, int y);
void DrawTextBoxSLine(int x, int y, int dy, bool widePanel);
void DrawDurIcon();
void DrawSpellBook();
void SelectBookSkill(bool shift, bool altSkill);
const char *get_pieces_str(int nGold);
void DrawGoldSplit(int amount);
void control_drop_gold(char vkey);
void DrawTeamBook();
void CheckTeamClick(bool shift);
void control_type_message();
void control_reset_talk();
bool control_talk_last_key(int vkey);
bool control_presskeys(int vkey);

/* data */

extern const RECT32 ChrBtnsRect[4];

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CONTROL_H__ */
