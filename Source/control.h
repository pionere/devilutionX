/**
 * @file control.h
 *
 * Interface of the character and main control panels
 */
#ifndef __CONTROL_H__
#define __CONTROL_H__

DEVILUTION_BEGIN_NAMESPACE

static_assert(NUM_WNDS <= INT8_MAX, "WND_VALID checks only the sign of the WND_-value I.");
static_assert((int8_t)WND_NONE < 0, "WND_VALID checks only the sign of the WND_-value II.");
#define WND_VALID(x) ((int8_t)x >= 0)

#ifdef __cplusplus
extern "C" {
#endif

/** The number of buttons in the menu. */
extern int numpanbtns;
/** Specifies whether the menu-button is pressed. */
extern bool gabPanbtn[NUM_PANBTNS];
/** Specifies how much the life flask is filled (percentage). */
extern int gnHPPer;
/** Specifies how much the mana flask is filled (percentage). */
extern int gnManaPer;
/** Graphics for the scrollbar of text boxes. */
extern CelImageBuf* pSTextSlidCels;
/** The mask of players who invited to their team. */
extern unsigned guTeamInviteRec;
/** The mask of players who were invited to the current players team. */
extern unsigned guTeamInviteSent;
/** The mask of players who were muted. */
extern unsigned guTeamMute;

/** Specifies the campaign-map status. */
extern BYTE gbCampaignMapFlag;
/** The index of the campaign-map in the inventory. */
extern int camItemIndex;
/** The 'selected' map on the campaign-map. */
extern CampaignMapEntry selCamEntry;

/** The current value in Golddrop. */
extern int gnDropGoldValue;
/** The gold-stack index which is used as a source in Golddrop (inv_item). */
extern BYTE gbDropGoldIndex;
extern BYTE infoclr;
extern char tempstr[256];
extern char infostr[256];
/** Number of active windows on the screen. */
extern int gnNumActiveWindows;
/** The list of active windows on the screen. */
extern BYTE gaActiveWindows[NUM_WNDS];
extern BYTE gbDragWnd;
extern int gnDragWndX;
extern int gnDragWndY;
/** The current tab in the Spell-Book. */
extern unsigned guBooktab;
/** Specifies whether the LevelUp button is displayed. */
extern bool gbLvlUp;
/** Specifies whether the LevelUp button is pressed. */
extern bool gbLvlbtndown;
/** Specifies whether any attribute-button is pressed on Character-Panel. */
extern bool gbChrbtnactive;
/** Specifies whether the Skill-List is displayed. */
extern bool gbSkillListFlag;

void DrawSkillList();
void SkillListMove(int dir);
void SetSkill(bool altSkill);
void SkillHotKey(int slot, bool altSkill);
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
void CheckChrBtnClick(bool altAction);
void ReleaseChrBtn();
void DrawSTextBox(int x, int y);
void DrawDurIcon();
void DrawSpellBook();
void CheckBookClick(bool altSkill);
const char* get_pieces_str(int nGold);
void DrawGoldSplit();
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
