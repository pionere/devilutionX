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
extern int gnNumActiveWindows;
extern BYTE gaActiveWindows[NUM_WNDS];
extern BYTE gbDragWnd;
extern int gnDragWndX;
extern int gnDragWndY;
extern bool gbLvlUp;
extern bool gbLvlbtndown;
extern unsigned guTeamInviteRec;
extern unsigned guTeamInviteSent;
extern unsigned guTeamMute;
extern bool gabPanbtn[NUM_PANBTNS];
extern int numpanbtns;
extern bool gbSkillListFlag;
extern BYTE gbCampaignMapFlag;
extern int camItemIndex;
extern CampaignMapEntry selCamEntry;

void DrawSkillList();
void SetSkill(bool altSkill);
void SetSkillHotKey(int slot, bool altSkill);
void SelectHotKeySkill(int slot, bool altSkill);
void DrawLifeFlask();
void DrawManaFlask();
void DrawSkillIcons();
void InitControlPan();
void FreeControlPan();
void StartWndDrag(BYTE wnd);
void DoWndDrag();
void DrawCtrlBtns();
bool ToggleWindow(BYTE wnd);
bool TryPanBtnClick();
void TryLimitedPanBtnClick();
void ReleasePanBtn();
void HandlePanBtn(int i);
void HandleSkillBtn(bool altSkill);
void DrawChr();
void ReleaseLvlBtn();
void DrawLevelUpIcon();
void DrawInfoStr();
void CheckChrBtnClick();
void ReleaseChrBtn();
void DrawTextBox(unsigned separators);
void DrawSTextBox(int x, int y);
void DrawTextBoxSLine(int x, int y, int dy, bool widePanel);
void DrawDurIcon();
void DrawSpellBook();
void CheckBookClick(bool altSkill);
const char* get_pieces_str(int nGold);
void DrawGoldSplit(int amount);
void control_drop_gold(int vkey);
void DrawTeamBook();
void CheckTeamClick();
void DrawGolemBar();
void InitCampaignMap(int cii);
void DrawCampaignMap();
void TryCampaignMapClick(bool altAction);

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __CONTROL_H__ */
