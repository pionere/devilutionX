/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"
#include "plrctrls.h"
#include "engine/render/text_render.hpp"

DEVILUTION_BEGIN_NAMESPACE

/** Menu button images CEL */
BYTE *pPanelButtons;
/** The number of buttons in the menu. */
int numpanbtns;
/** Specifies whether the menu-button is pressed. */
bool gabPanbtn[NUM_PANBTNS];
/** Specifies whether the LevelUp-button is pressed. */
bool gbLvlbtndown;
/** Flask images CEL */
BYTE *pFlasks;
/** Low-Durability images CEL */
BYTE *pDurIcons;

/** Specifies whether the Team-Panel is displayed. */
bool gbTeamFlag;
/** The current tab in the Team-Book. */
unsigned guTeamTab;
static_assert(MAX_PLRS < sizeof(int) * CHAR_BIT, "Players mask is used to maintain the team information.");
/** The mask of players who invited to their team. */
unsigned guTeamInviteRec;
/** The mask of players who were invited to the current players team. */
unsigned guTeamInviteSent;
/** The mask of players who were muted. */
unsigned guTeamMute;

/** Specifies whether the Chat-Panel is displayed. */
bool gbTalkflag;
/** Chat-Panel background CEL */
BYTE *pTalkPnl;
/** Chat-Panel button images CEL */
BYTE *pTalkBtns;
/** The current message in the Chat-Panel. */
char sgszTalkMsg[MAX_SEND_STR_LEN];
/** The cached messages of the Chat-Panel. */
char sgszTalkSave[8][MAX_SEND_STR_LEN];
/** The next position in the sgszTalkSave to save the message. */
BYTE sgbNextTalkSave;
/** The index of selected message in the sgszTalkSave array. */
BYTE sgbTalkSavePos;
/** Specifies whether the Golddrop is displayed. */
bool gbDropGoldFlag;
/** Golddrop background CEL */
BYTE *pGBoxBuff;
/** The gold-stack index which is used as a source in Golddrop. */
int initialDropGoldIndex;
/** The gold-stack size which is used as a source in Golddrop. */
int initialDropGoldValue;
/** The current value in Golddrop. */
int dropGoldValue;
BYTE infoclr;
char tempstr[256];
char infostr[256];
/**Specifies whether the Spell-Book is displayed. */
bool gbSbookflag;
/** SpellBook background CEL */
BYTE *pSpellBkCel;
/** SpellBook icons CEL */
BYTE *pSBkIconCels;
/** The current tab in the Spell-Book. */
unsigned guBooktab;
/** Specifies whether the Character-Panel is displayed. */
bool gbChrflag;
/** Char-Panel background CEL */
BYTE *pChrPanel;
/** Char-Panel button images CEL */
BYTE *pChrButtons;
/** Specifies whether the button of the given attribute is pressed on Character-Panel. */
bool _gabChrbtn[NUM_ATTRIBS];
/** Specifies whether any attribute-button is pressed on Character-Panel. */
bool gbChrbtnactive;

BYTE SplTransTbl[256];
static_assert(RSPLTYPE_CHARGES != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SCROLL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_ABILITY != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SPELL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_INVALID != -1, "Cached value of spellTrans must not be -1.");
char lastSt = -1;
/** Specifies whether the Skill-List is displayed. */
bool gbSkillListFlag;
/** Skill-List images CEL */
BYTE *pSpellCels;
/** The 'highlighted' skill in the Skill-List or in the Spell-Book. */
BYTE currSkill;
/** The type of the 'highlighted' skill in the Skill-List or in the Spell-Book. */
BYTE currSkillType;
/** Specifies whether the cursor should be moved to the active skill in the Skill-List. */
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
BYTE _gbMoveCursor = 0;
#endif

const int PanBtnPos[NUM_PANBTNS][2] = {
	// clang-format off
	{   0, 20 + 0 * MENUBTN_HEIGHT }, // menu button
	{   0, 20 + 1 * MENUBTN_HEIGHT }, // options button
	{   0, 20 + 2 * MENUBTN_HEIGHT }, // char button
	{   0, 20 + 3 * MENUBTN_HEIGHT }, // inv button
	{   0, 20 + 4 * MENUBTN_HEIGHT }, // spells button
	{   0, 20 + 5 * MENUBTN_HEIGHT }, // quests button
	{   0, 20 + 6 * MENUBTN_HEIGHT }, // map button
	{   0, 20 + 7 * MENUBTN_HEIGHT }, // chat button
	{   0, 20 + 8 * MENUBTN_HEIGHT }, // teams button
	// clang-format on
};
const char *PanBtnTxt[NUM_PANBTNS] = {
	// clang-format off
	"Menu",
	"Options",
	"Char",
	"Inv",
	"Spells",
	"Quests",
	"Map",
	"Chat",
	"Teams"
	// clang-format on
};
/** Maps from attribute_id to the rectangle on screen used for attribute increment buttons. */
const RECT32 ChrBtnsRect[NUM_ATTRIBS] = {
	// clang-format off
	{ 132, 102, CHRBTN_WIDTH, CHRBTN_HEIGHT },
	{ 132, 130, CHRBTN_WIDTH, CHRBTN_HEIGHT },
	{ 132, 159, CHRBTN_WIDTH, CHRBTN_HEIGHT },
	{ 132, 187, CHRBTN_WIDTH, CHRBTN_HEIGHT }
	// clang-format on
};

/** Maps from spellbook page number and position to spell_id. */
#define NUM_BOOK_ENTRIES 7
int SpellPages[SPLBOOKTABS][NUM_BOOK_ENTRIES] = {
	{ SPL_NULL, SPL_SWIPE, SPL_FIREBOLT, SPL_FIREBALL, SPL_FIREWALL, SPL_WAVE, SPL_FLAME },
	{ SPL_POINT_BLANK, SPL_FAR_SHOT, SPL_CBOLT, SPL_LIGHTNING, SPL_FLASH, SPL_NOVA, SPL_CHAIN },
	{ SPL_HBOLT, SPL_FLARE, SPL_ELEMENT, SPL_STONE, SPL_TELEKINESIS, SPL_GOLEM, SPL_GUARDIAN },
	{ SPL_MANASHIELD, SPL_HEAL, SPL_HEALOTHER, SPL_RNDTELEPORT, SPL_TELEPORT, SPL_TOWN, SPL_RESURRECT },
#ifdef HELLFIRE
	{ SPL_IMMOLAT, SPL_FIRERING, SPL_LIGHTWALL, SPL_RUNEFIRE, SPL_RUNEIMMOLAT, SPL_RUNELIGHT, SPL_RUNENOVA },
#endif
};
/** Maps from player-class to team-icon id. */
int ClassIconTbl[NUM_CLASSES] = { 8, 13, 42,
#ifdef HELLFIRE
	41, 9, 38,
#endif
};

/**
 * Draw spell cell onto the back buffer.
 * @param xp Back buffer coordinate
 * @param yp Back buffer coordinate
 * @param Trans Pointer to the cel buffer.
 * @param nCel Index of the cel frame to draw. 0 based.
 * @param w Width of the frame.
 */
static void DrawSpellCel(int xp, int yp, BYTE *Trans, int nCel, int w)
{
	CelDrawLight(xp, yp, Trans, nCel, w, SplTransTbl);
}

static void SetSpellTrans(char st)
{
	int i;

	if (lastSt == st)
		return;

	if (st == RSPLTYPE_ABILITY) {
		for (i = 0; i < 128; i++)
			SplTransTbl[i] = i;
	}
	for (i = 128; i < 256; i++)
		SplTransTbl[i] = i;
	SplTransTbl[255] = 0;

	switch (st) {
	case RSPLTYPE_ABILITY:
		break;
	case RSPLTYPE_SPELL:
		SplTransTbl[PAL8_YELLOW] = PAL16_BLUE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BLUE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BLUE + 5;
		for (i = PAL16_BLUE; i < PAL16_BLUE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_BLUE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BLUE + i] = i;
		}
		break;
	case RSPLTYPE_SCROLL:
		SplTransTbl[PAL8_YELLOW] = PAL16_BEIGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_BEIGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_BEIGE + 5;
		for (i = PAL16_BEIGE; i < PAL16_BEIGE + 16; i++) {
			SplTransTbl[PAL16_YELLOW - PAL16_BEIGE + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_BEIGE + i] = i;
		}
		break;
	case RSPLTYPE_CHARGES:
		SplTransTbl[PAL8_YELLOW] = PAL16_ORANGE + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_ORANGE + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_ORANGE + 5;
		for (i = PAL16_ORANGE; i < PAL16_ORANGE + 16; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_ORANGE + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_ORANGE + i] = i;
		}
		break;
	case RSPLTYPE_INVALID:
		SplTransTbl[PAL8_YELLOW] = PAL16_GRAY + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL16_GRAY + 3;
		SplTransTbl[PAL8_YELLOW + 2] = PAL16_GRAY + 5;
		for (i = PAL16_GRAY; i < PAL16_GRAY + 15; i++) {
			SplTransTbl[PAL16_BEIGE - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_YELLOW - PAL16_GRAY + i] = i;
			SplTransTbl[PAL16_ORANGE - PAL16_GRAY + i] = i;
		}
		SplTransTbl[PAL16_BEIGE + 15] = 0;
		SplTransTbl[PAL16_YELLOW + 15] = 0;
		SplTransTbl[PAL16_ORANGE + 15] = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void DrawSpellIconOverlay(int x, int y, int sn, int st, int lvl)
{
	ItemStruct *pi;
	int t, v;

	switch (st) {
	case RSPLTYPE_ABILITY:
		break;
	case RSPLTYPE_SPELL:
		if (lvl > 0) {
			snprintf(tempstr, sizeof(tempstr), "lvl%02d", lvl);
			t = COL_WHITE;
		} else {
			snprintf(tempstr, sizeof(tempstr), "X");
			t = COL_RED;
		}
		PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, true, t, 1);
		break;
	case RSPLTYPE_SCROLL:
		v = 0;
		pi = myplr.InvList;
		for (t = myplr._pNumInv; t > 0; t--, pi++) {
			if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
			    && pi->_iSpell == sn) {
				v++;
			}
		}
		pi = myplr.SpdList;
		for (t = MAXBELTITEMS; t > 0; t--, pi++) {
			if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
			    && pi->_iSpell == sn) {
				v++;
			}
		}
		snprintf(tempstr, sizeof(tempstr), "%d", v);
		PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, true, COL_WHITE, 1);
		break;
	case RSPLTYPE_CHARGES:
		snprintf(tempstr, sizeof(tempstr), "%d/%d",
			myplr.InvBody[INVLOC_HAND_LEFT]._iCharges,
			myplr.InvBody[INVLOC_HAND_LEFT]._iMaxCharges);
		PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, true, COL_WHITE, 1);
		break;
	case RSPLTYPE_INVALID:
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void DrawSkillIcon(BYTE spl, BYTE st, BYTE offset)
{
	int lvl = 0, y;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (spl == SPL_INVALID) {
		st = RSPLTYPE_INVALID;
		spl = SPL_NULL;
	} else if ((spelldata[spl].sFlags & myplr._pSkillFlags) != spelldata[spl].sFlags)
		st = RSPLTYPE_INVALID;
	else if (st == RSPLTYPE_SPELL) {
		lvl = GetSpellLevel(mypnum, spl);
		if (lvl <= 0 || !CheckSpell(mypnum, spl))
			st = RSPLTYPE_INVALID;
	}
	SetSpellTrans(st);
	y = SCREEN_Y + SCREEN_HEIGHT - 1 - offset;
	DrawSpellCel(SCREEN_X + SCREEN_WIDTH - SPLICONLENGTH, y, pSpellCels,
		spelldata[spl].sIcon, SPLICONLENGTH);
	DrawSpellIconOverlay(SCREEN_X + SCREEN_WIDTH - SPLICONLENGTH, y, spl, st, lvl);
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
void DrawSkillIcons()
{
	PlayerStruct *p;
	BYTE spl, type;

	p = &myplr;
	if (p->_pAtkSkill == SPL_INVALID) {
		spl = p->_pMoveSkill;
		type = p->_pMoveSkillType;
	} else {
		spl = p->_pAtkSkill;
		type = p->_pAtkSkillType;
	}
	DrawSkillIcon(spl, type, 0);

	if (p->_pAltAtkSkill == SPL_INVALID) {
		spl = p->_pAltMoveSkill;
		type = p->_pAltMoveSkillType;
	} else {
		spl = p->_pAltAtkSkill;
		type = p->_pAltAtkSkillType;
	}
	DrawSkillIcon(spl, type, SPLICONLENGTH);
}

static void DrawSkillIconHotKey(int x, int y, int sn, int st, int offset,
	BYTE (&hotKeyGroupA)[4], BYTE (&hotKeyTypeGroupA)[4],
	BYTE (&hotKeyGroupB)[4], BYTE (&hotKeyTypeGroupB)[4])
{
	int i, col;

	for (i = 0; i < 4; i++) {
		if (hotKeyGroupA[i] == sn && hotKeyTypeGroupA[i] == st)
			col = COL_GOLD;
		else if (hotKeyGroupB[i] == sn && hotKeyTypeGroupB[i] == st)
			col = COL_BLUE;
		else
			continue;
		snprintf(tempstr, sizeof(tempstr), "#%d", i + 1);
		PrintString(x + offset, y - SPLICONLENGTH + 16, x + offset + 18, tempstr, false, col, 1);
	}
}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
static bool MoveToAtkMoveSkill(int sn, int st, BYTE atk_sn, BYTE atk_st, BYTE move_sn, BYTE move_st)
{
	if (atk_sn != SPL_INVALID)
		return sn == atk_sn && st == atk_st;
	if (move_sn != SPL_INVALID)
		return sn == move_sn && st == move_st;
	return sn == SPL_NULL || sn == SPL_INVALID;
}

static bool MoveToSkill(PlayerStruct* p, int sn, int st)
{
	if (_gbMoveCursor == 0)
		return false;
	if (_gbMoveCursor == 1) {
		return MoveToAtkMoveSkill(sn, st,
			p->_pAltAtkSkill, p->_pAltAtkSkillType,
			p->_pAltMoveSkill, p->_pAltMoveSkillType);
	} else {
		return MoveToAtkMoveSkill(sn, st,
			p->_pAtkSkill, p->_pAtkSkillType,
			p->_pMoveSkill, p->_pMoveSkillType);
	}
}
#endif

void DrawSkillList()
{
	PlayerStruct *p;
	int i, j, x, y, /*c,*/ sl, sn, st, lx, ly;
	uint64_t mask;

	currSkill = SPL_INVALID;
	x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	y = PANEL_Y - 17;
	p = &myplr;
	static_assert(RSPLTYPE_ABILITY == 0, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 1.");
	static_assert(RSPLTYPE_SPELL == 1, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 2.");
	static_assert(RSPLTYPE_SCROLL == 2, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 3.");
	static_assert(RSPLTYPE_CHARGES == 3, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 4.");
	for (i = 0; i < 4; i++) {
		switch (i) {
		case RSPLTYPE_ABILITY:
			mask = p->_pAblSkills;
			//c = SPLICONLAST + 3;
			break;
		case RSPLTYPE_SPELL:
			mask = p->_pMemSkills;
			//c = SPLICONLAST + 4;
			break;
		case RSPLTYPE_SCROLL:
			mask = p->_pScrlSkills;
			//c = SPLICONLAST + 1;
			break;
		case RSPLTYPE_CHARGES:
			mask = p->_pISpells;
			//c = SPLICONLAST + 2;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		for (j = 0; mask != 0 && j < NUM_SPELLS; j++) {
			if (j == SPL_NULL) {
				if (i != 0)
					continue;
			} else {
				if (!(mask & 1)) {
					mask >>= 1;
					continue;
				}
				mask >>= 1;
			}
			st = i;
			if (i == RSPLTYPE_SPELL) {
				sl = GetSpellLevel(mypnum, j);
				st = sl > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID;
			}
			if ((spelldata[j].sFlags & p->_pSkillFlags) != spelldata[j].sFlags)
				st = RSPLTYPE_INVALID;
			SetSpellTrans(st);
			DrawSpellCel(x, y, pSpellCels, spelldata[j].sIcon, SPLICONLENGTH);
			lx = x - BORDER_LEFT;
			ly = y - BORDER_TOP - SPLICONLENGTH;
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
			if (MoveToSkill(p, j, i)) {
				SetCursorPos(lx + SPLICONLENGTH / 2, ly + SPLICONLENGTH / 2);
			}
#endif
			if (MouseX >= lx && MouseX < lx + SPLICONLENGTH && MouseY >= ly && MouseY < ly + SPLICONLENGTH) {
				//DrawSpellCel(x, y, pSpellCels, c, SPLICONLENGTH);
				DrawSpellCel(x, y, pSpellCels, SPLICONLAST, SPLICONLENGTH);

				currSkill = j;
				if (j == SPL_NULL) {
					sn = SPL_INVALID;
					st = RSPLTYPE_INVALID;
				} else {
					sn = j;
					st = i;
				}
				currSkillType = st;

				DrawSpellIconOverlay(x, y, sn, st, sl);

				DrawSkillIconHotKey(x, y, sn, st, 4,
					p->_pAtkSkillHotKey, p->_pAtkSkillTypeHotKey,
					p->_pMoveSkillHotKey, p->_pMoveSkillTypeHotKey);

				DrawSkillIconHotKey(x, y, sn, st, SPLICONLENGTH - 18, 
					p->_pAltAtkSkillHotKey, p->_pAltAtkSkillTypeHotKey,
					p->_pAltMoveSkillHotKey, p->_pAltMoveSkillTypeHotKey);
			}
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
		if (j != 0 && x != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS) {
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
	}
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	_gbMoveCursor = 0;
#endif
}

/*
 * @brief Select the current skill to use for the (alt)action button.
 * @param shift true: the other (move/attack)skill is kept
 *             false: the other (move/attack)skill is set to INVALID
 * @param altSkill set it as the action or the alt action skill
 */
void SetSkill(bool shift, bool altSkill)
{
	PlayerStruct *p;
	BYTE sn;
	bool moveskill;

	sn = currSkill;
	if (sn == SPL_INVALID) {
		gbSkillListFlag = false;
		return;
	}
	if (sn == SPL_NULL)
		sn = SPL_INVALID;
	// TODO: add flag for movement-skills
	moveskill = sn == SPL_WALK || sn == SPL_TELEPORT || sn == SPL_RNDTELEPORT;

	p = &myplr;
	if (shift) {
		if (!altSkill) {
			if (moveskill) {
				p->_pMoveSkill = sn;
				p->_pMoveSkillType = currSkillType;
			} else {
				p->_pAtkSkill = sn;
				p->_pAtkSkillType = currSkillType;
			}
		} else {
			if (moveskill) {
				p->_pAltMoveSkill = sn;
				p->_pAltMoveSkillType = currSkillType;
			} else {
				p->_pAltAtkSkill = sn;
				p->_pAltAtkSkillType = currSkillType;
			}
		}
	} else {
		if (!altSkill) {
			if (moveskill) {
				p->_pMoveSkill = sn;
				p->_pMoveSkillType = currSkillType;
				p->_pAtkSkill = SPL_INVALID;
				p->_pAtkSkillType = RSPLTYPE_INVALID;
			} else {
				p->_pAtkSkill = sn;
				p->_pAtkSkillType = currSkillType;
				p->_pMoveSkill = SPL_INVALID;
				p->_pMoveSkillType = RSPLTYPE_INVALID;
			}
		} else {
			if (moveskill) {
				p->_pAltMoveSkill = sn;
				p->_pAltMoveSkillType = currSkillType;
				p->_pAltAtkSkill = SPL_INVALID;
				p->_pAltAtkSkillType = RSPLTYPE_INVALID;
			} else {
				p->_pAltAtkSkill = sn;
				p->_pAltAtkSkillType = currSkillType;
				p->_pAltMoveSkill = SPL_INVALID;
				p->_pAltMoveSkillType = RSPLTYPE_INVALID;
			}
		}
		
		gbSkillListFlag = false;
	}
	
	//gbRedrawFlags = REDRAW_ALL;
}

static void SetSkillHotKey(BYTE (&hotKeyGroup)[4], BYTE (&hotKeyTypeGroup)[4], int slot, int sn)
{
	int i;

	for (i = 0; i < lengthof(hotKeyGroup); ++i) {
		if (hotKeyGroup[i] == sn && hotKeyTypeGroup[i] == currSkillType) {
			hotKeyGroup[i] = SPL_INVALID;
			hotKeyTypeGroup[i] = RSPLTYPE_INVALID;
			if (slot == i)
				return;
		}
	}
	hotKeyGroup[slot] = sn;
	hotKeyTypeGroup[slot] = currSkillType;

}

/*
 * @brief Select the current skill to be activated by the given hotkey
 * @param slot the index of the hotkey
 * @param altSkill type of the hotkey (true: alt-hotkey, false: normal hotkey)
 */
void SetSkillHotKey(int slot, bool altSkill)
{
	PlayerStruct *p;
	int sn = currSkill;
	bool moveskill;

	if (sn != SPL_INVALID) {
		// TODO: add flag for movement-skills
		moveskill = sn == SPL_WALK || sn == SPL_TELEPORT || sn == SPL_RNDTELEPORT;
		if (sn == SPL_NULL)
			sn = SPL_INVALID;

		p = &myplr;
		if (!altSkill) {
			if (moveskill)
				SetSkillHotKey(p->_pMoveSkillHotKey, p->_pMoveSkillTypeHotKey, slot, sn);
			else 
				SetSkillHotKey(p->_pAtkSkillHotKey, p->_pAtkSkillTypeHotKey, slot, sn);
		} else {
			if (moveskill)
				SetSkillHotKey(p->_pAltMoveSkillHotKey, p->_pAltMoveSkillTypeHotKey, slot, sn);
			else 
				SetSkillHotKey(p->_pAltAtkSkillHotKey, p->_pAltAtkSkillTypeHotKey, slot, sn);
		}
	}
}

static void SelectHotKeySkill(BYTE (&hotKeyGroup)[4], BYTE (&hotKeyTypeGroup)[4], int slot,
	BYTE *destSkill, BYTE *destSkillType)
{
	*destSkill = hotKeyGroup[slot];
	*destSkillType = hotKeyTypeGroup[slot];
	//gbRedrawFlags = REDRAW_ALL;
}

/*
 * @brief Select a skill for the current player with a hotkey
 * @param slot the index of the hotkey
 * @param altSkill type of the hotkey (true: alt-hotkey, false: normal hotkey)
 */
void SelectHotKeySkill(int slot, bool altSkill)
{
	PlayerStruct *p;

	p = &myplr;
	if (!altSkill) {
		SelectHotKeySkill(p->_pMoveSkillHotKey, p->_pMoveSkillTypeHotKey, slot,
			&p->_pMoveSkill, &p->_pMoveSkillType);
		SelectHotKeySkill(p->_pAtkSkillHotKey, p->_pAtkSkillTypeHotKey, slot,
			&p->_pAtkSkill, &p->_pAtkSkillType);
	} else {
		SelectHotKeySkill(p->_pAltMoveSkillHotKey, p->_pAltMoveSkillTypeHotKey, slot,
			&p->_pAltMoveSkill, &p->_pAltMoveSkillType);
		SelectHotKeySkill(p->_pAltAtkSkillHotKey, p->_pAltAtkSkillTypeHotKey, slot,
			&p->_pAltAtkSkill, &p->_pAltAtkSkillType);
	}
}

/*void DrawPanelBox(int x, int y, int w, int h, int sx, int sy)
{
	int nSrcOff, nDstOff;

	assert(gpBuffer != NULL);

	nSrcOff = x + PANEL_WIDTH * y;
	nDstOff = sx + BUFFER_WIDTH * sy;

	int hgt;
	BYTE *src, *dst;

	src = &pBtmBuff[nSrcOff];
	dst = &gpBuffer[nDstOff];

	for (hgt = h; hgt != 0; hgt--, src += PANEL_WIDTH, dst += BUFFER_WIDTH) {
		memcpy(dst, src, w);
	}
}*/

/**
 * Draws a section of the empty flask cel on top of the panel to create the illusion
 * of the flask getting empty. This function takes a cel and draws a
 * horizontal stripe of height (max-min) onto the back buffer.
 * @param pCelBuff Buffer of the empty flask cel.
 * @param min Top of the flask cel section to draw.
 * @param max Bottom of the flask cel section to draw.
 * @param sx Back buffer coordinate
 * @param sy Back buffer coordinate
 */
/*static void SetFlaskHeight(BYTE *pCelBuff, int min, int max, int sx, int sy)
{
	int nSrcOff, nDstOff, w;

	assert(gpBuffer != NULL);

	nSrcOff = 88 * min;
	nDstOff = sx + BUFFER_WIDTH * sy;
	w = max - min;

	BYTE *src, *dst;

	src = &pCelBuff[nSrcOff];
	dst = &gpBuffer[nDstOff];

	for ( ; w != 0; w--, src += 88, dst += BUFFER_WIDTH)
		memcpy(dst, src, 88);
}*/

/**
 * Draws the dome of the flask that protrudes above the panel top line.
 * It draws a rectangle of fixed width 59 and height 'h' from the source buffer
 * into the target buffer.
 * @param pCelBuff The flask cel buffer.
 * @param w Width of the cel.
 * @param nSrcOff Offset of the source buffer from where the bytes will start to be copied from.
 * @param nDstOff Offset of the target buffer where the bytes will start to be copied to.
 * @param h How many lines of the source buffer that will be copied.
 */
/*static void DrawFlask(BYTE *pCelBuff, int w, int nSrcOff, int nDstOff, int h)
{
	int wdt, hgt;
	BYTE *src, *dst;

	assert(gpBuffer != NULL);

	src = &pCelBuff[nSrcOff];
	dst = &gpBuffer[nDstOff];

	for (hgt = h; hgt != 0; hgt--, src += w - 59, dst += BUFFER_WIDTH - 59) {
		for (wdt = 59; wdt != 0; wdt--) {
			if (*src != 0)
				*dst = *src;
			src++;
			dst++;
		}
	}
}*/

static void DrawFlask2(int sx, int filled, int emptyCel, int fullCel, int w)
{
	const BYTE *empty, *full;
	int sy, dataSize, i;
	char width;

	sy = SCREEN_Y + SCREEN_HEIGHT - 1;

	filled += 13;
	int emptied = 95 - filled;
	empty = CelGetFrame(pFlasks, emptyCel, &dataSize);
	full = CelGetFrame(pFlasks, fullCel, &dataSize);

	BYTE *dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
	for ( ; filled-- != 0; dst -= BUFFER_WIDTH + w) {
		for (i = w; i != 0; ) {
			width = *full++;
			if (width >= 0) {
				i -= width;
				memcpy(dst, full, width);
				full += width;
				dst += width;
			} else {
				dst -= width;
				i += width;
			}
		}
		for (i = w; i != 0; ) {
			width = *empty++;
			if (width >= 0) {
				i -= width;
				empty += width;
			} else {
				i += width;
			}
		}
	}

	for ( ; emptied-- != 0; dst -= BUFFER_WIDTH + w) {
		for (i = w; i != 0; ) {
			width = *empty++;
			if (width >= 0) {
				i -= width;
				memcpy(dst, empty, width);
				empty += width;
				dst += width;
			} else {
				dst -= width;
				i += width;
			}
		}
	}
}

void DrawLifeFlask()
{
	int filled;
	int maxHP, hp;
	int x;

	if (gbRedrawFlags & REDRAW_HP_FLASK) {
		maxHP = myplr._pMaxHP;
		hp = myplr._pHitPoints;
		if (hp <= 0 || maxHP <= 0)
			filled = 0;
		else
			filled = 82 * hp / maxHP;
		if (filled > 82)
			filled = 82;
		myplr._pHPPer = filled;
	} else {
		filled = myplr._pHPPer;
	}

	x = SCREEN_X + 80 - 28;
	DrawFlask2(x, filled, 1, 2, 118);
}

void DrawManaFlask()
{
	int filled;
	int maxMana, mana;
	int x;

	if (gbRedrawFlags & REDRAW_MANA_FLASK) {
		maxMana = myplr._pMaxMana;
		mana = myplr._pMana;

		if (mana <= 0 || maxMana <= 0)
			filled = 0;
		else
			filled = 82 * mana / maxMana;
		if (filled > 82)
			filled = 82;
		myplr._pManaPer = filled;
	} else {
		filled = myplr._pManaPer;
	}

	x = SCREEN_X + SCREEN_WIDTH - (SPLICONLENGTH + 92);
	DrawFlask2(x, filled, 3, myplr._pManaShield == 0 ? 4 : 5, 93);
}

void InitControlPan()
{
	int i;

	pFlasks = LoadFileInMem("CtrlPan\\Flasks.CEL");;
	pChrPanel = LoadFileInMem("Data\\Char.CEL");
#ifdef HELLFIRE
	pSpellCels = LoadFileInMem("Data\\SpelIcon.CEL");
#else
	pSpellCels = LoadFileInMem("CtrlPan\\SpelIcon.CEL");
#endif
	SetSpellTrans(RSPLTYPE_ABILITY);
	gbTalkflag = false;
	gbTeamFlag = false;
	guTeamInviteRec = 0;
	guTeamInviteSent = 0;
	guTeamMute = 0;
	sgszTalkMsg[0] = '\0';
	if (gbMaxPlayers != 1) {
		pTalkPnl = LoadFileInMem("CtrlPan\\TalkPnl.CEL");
		pTalkBtns = LoadFileInMem("CtrlPan\\TalkButt.CEL");
	}
	gbLvlbtndown = false;
	pPanelButtons = LoadFileInMem("CtrlPan\\Menu.CEL");
	for (i = 0; i < lengthof(gabPanbtn); i++)
		gabPanbtn[i] = false;
	numpanbtns = gbMaxPlayers == 1 ? NUM_PANBTNS - 2 : NUM_PANBTNS;
	pChrButtons = LoadFileInMem("Data\\CharBut.CEL");
	for (i = 0; i < lengthof(_gabChrbtn); i++)
		_gabChrbtn[i] = false;
	gbChrbtnactive = false;
	pDurIcons = LoadFileInMem("Items\\DurIcons.CEL");
	infostr[0] = '\0';
	gbRedrawFlags |= REDRAW_HP_FLASK | REDRAW_MANA_FLASK | REDRAW_SPEED_BAR;
	gbChrflag = false;
	gbSkillListFlag = false;
	pSpellBkCel = LoadFileInMem("Data\\SpellBk.CEL");
	pSBkIconCels = LoadFileInMem("Data\\SpellI2.CEL");
	guBooktab = 0;
	gbSbookflag = false;
	SpellPages[0][0] = Abilities[myplr._pClass];
	pQLogCel = LoadFileInMem("Data\\Quest.CEL");
	pGBoxBuff = LoadFileInMem("CtrlPan\\Golddrop.cel");
	gbDropGoldFlag = false;
	dropGoldValue = 0;
	initialDropGoldValue = 0;
	initialDropGoldIndex = 0;
}

/**
 * Draws the control panel buttons in their current state. If the button is in the default
 * state draw it from the panel cel(extract its sub-rect). Else draw it from the buttons cel.
 */
void DrawCtrlBtns()
{
	int i, x, y;
	bool pb;

	i = 0;
	x = SCREEN_X + PanBtnPos[i][0];
	if (!gabPanbtn[PANBTN_MAINMENU]) {
		CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1] + 18, pPanelButtons, 4, 71);
		return;
	}
	CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1] + 18, pPanelButtons, 3, 71);
	for (i = 1; i < numpanbtns; i++) {
		y = SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1];
		pb = gabPanbtn[i];
		CelDraw(x, y + 18, pPanelButtons, 1, 71);
		// print the text of the button
		PrintString(x + 3, y + 15, x + 70, PanBtnTxt[i], true, pb ? COL_GOLD : COL_WHITE, 1);
	}
}

/**
 * Opens the "Skill List": the rows of known spells for quick-setting a spell that
 * show up when you click the spell slot at the control panel.
 * @param altSkill whether the cursor is moved to the active skill or altSkill (controllers-only)
 */
void DoSkillList(bool altSkill)
{
	gbSkillListFlag = true;

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	_gbMoveCursor = 0;
	if (sgbControllerActive)
		_gbMoveCursor = altSkill ? 1 : 2;
#endif
}

void HandleSkillBtn(bool altSkill)
{
	if (!gbSkillListFlag) {
		ClearPanels();
		DoSkillList(altSkill);
	} else {
		gbSkillListFlag = false;
	}
	gamemenu_off();
}

static void control_set_button_down(int btn_id)
{
	if (btn_id == PANBTN_MAINMENU) {
		gabPanbtn[PANBTN_MAINMENU] = !gabPanbtn[PANBTN_MAINMENU];
	} else {
		assert(gabPanbtn[PANBTN_MAINMENU]);
		gabPanbtn[btn_id] = true;
	}
	//gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
}

static bool InLvlUpRect()
{
	return MouseX >= 175
		&& MouseX <= 175 + CHRBTN_WIDTH
		&& MouseY >= SCREEN_HEIGHT - 24 - CHRBTN_HEIGHT
		&& MouseY <= SCREEN_HEIGHT - 24;
}

void ReleaseLvlBtn()
{
	if (InLvlUpRect()) {
		HandlePanBtn(PANBTN_CHARINFO);
	}
	gbLvlbtndown = false;
}
/**
 * Checks if the mouse cursor is within any of the panel buttons and flag it if so.
 */
bool DoPanBtn()
{
	int i, mx, my;

	mx = MouseX;
	my = MouseY;
	for (i = gabPanbtn[PANBTN_MAINMENU] ? numpanbtns - 1 : 0; i >= 0; i--) {
		if (mx >= PanBtnPos[i][0]
		 && mx <= PanBtnPos[i][0] + MENUBTN_WIDTH
		 && my >= SCREEN_HEIGHT - PanBtnPos[i][1]
		 && my <= SCREEN_HEIGHT - PanBtnPos[i][1] + MENUBTN_HEIGHT) {
			control_set_button_down(i);
			return true;
		}
	}
	if (mx >= SCREEN_WIDTH - (SPLICONLENGTH + 4)
	 && mx <= SCREEN_WIDTH - 4
	 && my >= SCREEN_HEIGHT - 2 * (SPLICONLENGTH + 4)
	 && my <= SCREEN_HEIGHT - 4) {
		HandleSkillBtn(my < SCREEN_HEIGHT - (SPLICONLENGTH + 4));
		return true;
	}
	if (myplr._pLvlUp && InLvlUpRect())
		gbLvlbtndown = true;
	return gbLvlbtndown;
}

void DoLimitedPanBtn()
{
	if (MouseX >= PanBtnPos[PANBTN_MAINMENU][0]
	 && MouseX <= PanBtnPos[PANBTN_MAINMENU][0] + MENUBTN_WIDTH
	 && MouseY >= SCREEN_HEIGHT - PanBtnPos[PANBTN_MAINMENU][1]
	 && MouseY <= SCREEN_HEIGHT - PanBtnPos[PANBTN_MAINMENU][1] + MENUBTN_HEIGHT) {
		control_set_button_down(PANBTN_MAINMENU);
	} else if (gabPanbtn[PANBTN_MAINMENU] && gbMaxPlayers != 1) {
		if (MouseX >= PanBtnPos[PANBTN_SENDMSG][0]
		 && MouseX <= PanBtnPos[PANBTN_SENDMSG][0] + MENUBTN_WIDTH
		 && MouseY >= SCREEN_HEIGHT - PanBtnPos[PANBTN_SENDMSG][1]
		 && MouseY <= SCREEN_HEIGHT - PanBtnPos[PANBTN_SENDMSG][1] + MENUBTN_HEIGHT) {
			control_set_button_down(PANBTN_SENDMSG);
		}
	}
}

void HandlePanBtn(int i)
{
	switch (i) {
	case PANBTN_MAINMENU:
		gbQtextflag = false;
		break;
	case PANBTN_OPTIONS:
		gamemenu_on();
		return;
	case PANBTN_CHARINFO:
		gbQuestlog = false;
		gbSkillListFlag = false;
		myplr._pLvlUp = FALSE;
		gbChrflag = !gbChrflag;
		break;
	case PANBTN_INVENTORY:
		gbSbookflag = false;
		gbSkillListFlag = false;
		gbTeamFlag = false;
		gbInvflag = !gbInvflag;
		break;
	case PANBTN_SPELLBOOK:
		gbInvflag = false;
		gbTeamFlag = false;
		gbSkillListFlag = false;
		gbSbookflag = !gbSbookflag;
		break;
	case PANBTN_QLOG:
		gbChrflag = false;
		gbSkillListFlag = false;
		if (!gbQuestlog)
			StartQuestlog();
		else
			gbQuestlog = false;
		break;
	case PANBTN_AUTOMAP:
		ToggleAutomap();
		break;
	case PANBTN_SENDMSG:
		if (gbTalkflag)
			control_reset_talk();
		else
			control_type_message();
		break;
	case PANBTN_TEAMBOOK:
		gbInvflag = false;
		gbSkillListFlag = false;
		gbSbookflag = false;
		gbTeamFlag = !gbTeamFlag;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	gbHelpflag = false;
	gamemenu_off();
}

/**
 * Check if the mouse is within a control panel button that's flagged.
 * Takes apropiate action if so.
 */
void CheckBtnUp()
{
	int i;

	static_assert(lengthof(gabPanbtn) == lengthof(PanBtnPos), "Mismatching gabPanbtn and panbtnpos tables.");
	static_assert(PANBTN_MAINMENU == 0, "CheckBtnUp needs to skip the mainmenu-button.");
	for (i = 1; i < lengthof(gabPanbtn); i++) {
		if (!gabPanbtn[i]) {
			continue;
		}

		gabPanbtn[i] = false;
		if (MouseX < PanBtnPos[i][0]
		 || MouseX > PanBtnPos[i][0] + MENUBTN_WIDTH
		 || MouseY < SCREEN_HEIGHT - PanBtnPos[i][1]
		 || MouseY > SCREEN_HEIGHT - PanBtnPos[i][1] + MENUBTN_HEIGHT) {
			continue;
		}

		HandlePanBtn(i);

		gabPanbtn[PANBTN_MAINMENU] = false;
		//gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
	}
}

void FreeControlPan()
{
	MemFreeDbg(pChrPanel);
	MemFreeDbg(pSpellCels);
	MemFreeDbg(pPanelButtons);
	MemFreeDbg(pTalkBtns);
	MemFreeDbg(pChrButtons);
	MemFreeDbg(pDurIcons);
	MemFreeDbg(pQLogCel);
	MemFreeDbg(pSpellBkCel);
	MemFreeDbg(pSBkIconCels);
	MemFreeDbg(pGBoxBuff);
}

#define ADD_PlrStringXY(x, y, endX, pszStr, col) PrintString(x + SCREEN_X, y + SCREEN_Y, endX + SCREEN_X, pszStr, true, col, 1)

void DrawChr()
{
	PlayerStruct *p;
	BYTE col;
	char chrstr[64];
	int pc, val, mindam, maxdam;

	p = &myplr;
	pc = p->_pClass;

	CelDraw(SCREEN_X, SCREEN_Y + SPANEL_HEIGHT - 1, pChrPanel, 1, SPANEL_WIDTH);
	ADD_PlrStringXY(7, 19, 138, p->_pName, COL_WHITE);

	ADD_PlrStringXY(155, 19, 286, ClassStrTbl[pc], COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%i", p->_pLevel);
	ADD_PlrStringXY(53, 46, 96, chrstr, COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%i", p->_pExperience);
	ADD_PlrStringXY(203, 46, 288, chrstr, COL_WHITE);

	if (p->_pLevel == MAXCHARLEVEL) {
		copy_cstr(chrstr, "None");
		col = COL_GOLD;
	} else {
		snprintf(chrstr, sizeof(chrstr), "%i", p->_pNextExper);
		col = COL_WHITE;
	}
	ADD_PlrStringXY(203, 74, 288, chrstr, col);

	snprintf(chrstr, sizeof(chrstr), "%i", p->_pGold);
	ADD_PlrStringXY(202, 118, 287, chrstr, COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%i", p->_pIAC);
	ADD_PlrStringXY(245, 149, 288, chrstr, COL_WHITE);

	val = p->_pIHitChance;
	col = COL_WHITE;
	if (p->_pIBaseHitBonus == IBONUS_POSITIVE)
		col = COL_BLUE;
	else if (p->_pIBaseHitBonus == IBONUS_NEGATIVE)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	ADD_PlrStringXY(245, 177, 288, chrstr, col);

	col = COL_WHITE;
	mindam = (p->_pIFMinDam + p->_pILMinDam + p->_pIMMinDam + p->_pIAMinDam) >> 6;
	maxdam = (p->_pIFMaxDam + p->_pILMaxDam + p->_pIMMaxDam + p->_pIAMaxDam) >> 6;
	if (maxdam != 0)
		col = COL_BLUE;
	mindam += (p->_pISlMinDam + p->_pIBlMinDam + p->_pIPcMinDam) >> (6 + 1); // +1 is a temporary(?) adjustment for backwards compatibility
	maxdam += (p->_pISlMaxDam + p->_pIBlMaxDam + p->_pIPcMaxDam) >> (6 + 1);
	snprintf(chrstr, sizeof(chrstr), "%i-%i", mindam, maxdam);
	if (mindam >= 100 || maxdam >= 100)
		PrintString(241 + SCREEN_X, 205 + SCREEN_Y, 292 + SCREEN_X, chrstr, true, col, -1);
	else
		PrintString(245 + SCREEN_X, 205 + SCREEN_Y, 288 + SCREEN_X, chrstr, true, col, 0);

	val = p->_pMagResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(190, 254, 233, chrstr, col);

	val = p->_pFireResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(246, 254, 289, chrstr, col);

	val = p->_pLghtResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(190, 289, 233, chrstr, col);

	val = p->_pAcidResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(246, 289, 289, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseStr);
	ADD_PlrStringXY(90, 119, 121, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseMag);
	ADD_PlrStringXY(90, 147, 121, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseDex);
	ADD_PlrStringXY(90, 175, 121, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseVit);
	ADD_PlrStringXY(90, 203, 121, chrstr, col);

	val = p->_pStrength;
	col = COL_WHITE;
	if (val > p->_pBaseStr)
		col = COL_BLUE;
	else if (val < p->_pBaseStr)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(138, 119, 169, chrstr, col);

	val = p->_pMagic;
	col = COL_WHITE;
	if (val > p->_pBaseMag)
		col = COL_BLUE;
	else if (val < p->_pBaseMag)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(138, 147, 169, chrstr, col);

	val = p->_pDexterity;
	col = COL_WHITE;
	if (val > p->_pBaseDex)
		col = COL_BLUE;
	else if (val < p->_pBaseDex)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(138, 175, 169, chrstr, col);

	val = p->_pVitality;
	col = COL_WHITE;
	if (val > p->_pBaseVit)
		col = COL_BLUE;
	else if (val < p->_pBaseVit)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(138, 203, 169, chrstr, col);

	if (p->_pStatPts > 0) {
		snprintf(chrstr, sizeof(chrstr), "%i", p->_pStatPts);
		ADD_PlrStringXY(90, 231, 121, chrstr, COL_RED);
		CelDraw(ChrBtnsRect[ATTRIB_STR].x + SCREEN_X, ChrBtnsRect[ATTRIB_STR].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, _gabChrbtn[ATTRIB_STR] ? 3 : 2, CHRBTN_WIDTH);
		CelDraw(ChrBtnsRect[ATTRIB_MAG].x + SCREEN_X, ChrBtnsRect[ATTRIB_MAG].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, _gabChrbtn[ATTRIB_MAG] ? 5 : 4, CHRBTN_WIDTH);
		CelDraw(ChrBtnsRect[ATTRIB_DEX].x + SCREEN_X, ChrBtnsRect[ATTRIB_DEX].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, _gabChrbtn[ATTRIB_DEX] ? 7 : 6, CHRBTN_WIDTH);
		CelDraw(ChrBtnsRect[ATTRIB_VIT].x + SCREEN_X, ChrBtnsRect[ATTRIB_VIT].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, _gabChrbtn[ATTRIB_VIT] ? 9 : 8, CHRBTN_WIDTH);
	}

	val = p->_pMaxHP;
	col = val <= p->_pMaxHPBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%i", val >> 6);
	ADD_PlrStringXY(90, 260, 121, chrstr, col);
	if (p->_pHitPoints != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pHitPoints >> 6);
	ADD_PlrStringXY(138, 260, 169, chrstr, col);

	val = p->_pMaxMana;
	col = val <= p->_pMaxManaBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%i", val >> 6);
	ADD_PlrStringXY(90, 288, 121, chrstr, col);
	if (p->_pMana != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pMana >> 6);
	ADD_PlrStringXY(138, 288, 169, chrstr, col);
}

void DrawLevelUpIcon()
{
	ADD_PlrStringXY(137, SCREEN_HEIGHT - 4, 137 + 120, "Level Up", COL_WHITE);
	CelDraw(SCREEN_X + 175, SCREEN_Y + SCREEN_HEIGHT - 24, pChrButtons, gbLvlbtndown ? 3 : 2, CHRBTN_WIDTH);
}

static int DrawTooltip2(const char *text1, const char* text2, int x, int y, BYTE col)
{
	int width, result = 0;
	BYTE *dst;
	const int border = 4, height = 26;
	int w1 = GetStringWidth(text1);
	int w2 = GetStringWidth(text2);

	width = std::max(w1, w2) + 2 * border;

	y -= TILE_HEIGHT;
	if (y < 0)
		return result;
	x -= width / 2;
	if (x < 0) {
		result = -x;
		x = 0;
	} else if (x > SCREEN_WIDTH - width) {
		result = (SCREEN_WIDTH - width) - x;
		x = SCREEN_WIDTH - width;
	}

	// draw gray border
	dst = &gpBuffer[SCREENXY(x, y)];
	for (int i = 0; i < height; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_GRAY + 8, width);

	// draw background
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_ORANGE + 14, width - 2);

	// print the info
	if (w1 > w2) {
		w2 = (w1 - w2) >> 1;
		w1 = 0;
	} else {
		w1 = (w2 - w1) >> 1;
		w2 = 0;
	}
	PrintGameStr(x + border + w1, y + height - 14, text1, col);
	PrintGameStr(x + border + w2, y + height - 3, text2, COL_WHITE);
	return result;
}

/*
 * Return the screen position of the given tile (x;y).
 *
 * @param x the x index of the tile
 * @param y the y index of the tile
 * @param outx the screen x-coordinate of the tile
 * @param outy the screen y-coordinate of the tile
 */
static void GetMousePos(int x, int y, int *outx, int *outy)
{
	int px, py;

	x -= ViewX;
	y -= ViewY;
	
	px = 0;
	py = 0;
	SHIFT_GRID(px, py, -y, x);

	px *= TILE_WIDTH / 2;
	py *= TILE_HEIGHT / 2;

	if (!gbZoomflag) {
		px <<= 1;
		py <<= 1;
	}

	px += SCREEN_WIDTH / 2;
	py += VIEWPORT_HEIGHT / 2;

	*outx = px;
	*outy = py;
}

static BYTE DrawItemColor(ItemStruct *is)
{
	if (is->_iMagical == ITEM_QUALITY_NORMAL)
		return COL_WHITE;
	return is->_iMagical == ITEM_QUALITY_UNIQUE ? COL_GOLD : COL_BLUE;
}

static void GetItemInfo(ItemStruct *is)
{
	infoclr = DrawItemColor(is);
	if (is->_itype != ITYPE_GOLD) {
		if (is->_iIdentified)
			copy_str(infostr, is->_iIName);
		else
			copy_str(infostr, is->_iName);
	} else {
		snprintf(infostr, sizeof(infostr), "%i gold %s", is->_ivalue, get_pieces_str(is->_ivalue));
	}
}

static int DrawTooltip(const char* text, int x, int y, BYTE col)
{
	int width, result = 0;
	BYTE *dst;
	const int border = 4, height = 16;

	width = GetStringWidth(text) + 2 * border;

	y -= TILE_HEIGHT;
	if (y < 0)
		return result;
	x -= width / 2;
	if (x < 0) {
		result = -x;
		x = 0;
	} else if (x > SCREEN_WIDTH - width) {
		result = (SCREEN_WIDTH - width) - x;
		x = SCREEN_WIDTH - width;
	}

	// draw gray border
	dst = &gpBuffer[SCREENXY(x, y)];
	for (int i = 0; i < height; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_GRAY + 8, width);

	// draw background
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_ORANGE + 14, width - 2);

	// print the info
	PrintGameStr(x + border, y + height - 3, text, col);
	return result;
}

static void DrawHealthBar(int hp, int maxhp, int x, int y)
{
	BYTE *dst;
	const int height = 4, width = 66;
	int h, dhp, w, dw;

	if (maxhp <= 0)
		return;

	y -= TILE_HEIGHT / 2 + 2;
	if (y < 0)
		return;
	x -= width / 2;
	if (x < 0)
		x = 0;
	else if (x > SCREEN_WIDTH - width)
		x = SCREEN_WIDTH - width;

	// draw gray border
	dst = &gpBuffer[SCREENXY(x, y)];
	for (int i = 0; i < height; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_GRAY + 5, width);

	// draw the bar
	//width = (width - 2) * hp / maxhp;
	dhp = (maxhp + 7) >> 3;
	dw = ((width - 2) >> 3);
	for (w = 0, h = 0; h < hp; h += dhp, w += dw) {
	}
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_RED + 9, w);
}

static void DrawTrigInfo()
{
	int xx, yy, qn;

	if (pcurstrig >= MAXTRIGGERS + 1) {
		// portal
		MissileStruct *mis = &missile[pcurstrig - (MAXTRIGGERS + 1)];
		if (mis->_miType == MIS_TOWN) {
			copy_cstr(infostr, "Town Portal");
			snprintf(tempstr, sizeof(tempstr), "(%s)", players[mis->_miSource]._pName);
			GetMousePos(cursmx - 2, cursmy - 2, &xx, &yy);
			DrawTooltip2(infostr, tempstr, xx, yy, COL_WHITE);
		} else {
			if (!currLvl._dSetLvl) {
				copy_cstr(infostr, "Portal to The Unholy Altar");
			} else {
				copy_cstr(infostr, "Portal back to hell");
			}
			GetMousePos(cursmx - 2, cursmy - 2, &xx, &yy);
			DrawTooltip(infostr, xx, yy, COL_WHITE);
		}
		return;
	} else {
		// standard trigger
		switch (trigs[pcurstrig]._tmsg) {
		case WM_DIABNEXTLVL:
			snprintf(infostr, sizeof(infostr), "Down to %s", AllLevels[currLvl._dLevelIdx + 1].dLevelName);
			break;
		case WM_DIABPREVLVL:
			if (currLvl._dLevelIdx == 1)
				copy_cstr(infostr, "Up to town");
			else
				snprintf(infostr, sizeof(infostr), "Up to %s", AllLevels[currLvl._dLevelIdx - 1].dLevelName);
			break;
		case WM_DIABRTNLVL:
			switch (currLvl._dLevelIdx) {
			case SL_SKELKING:
				qn = Q_SKELKING;
				break;
			case SL_BONECHAMB:
				qn = Q_SCHAMB;
				break;
			case SL_POISONWATER:
				qn = Q_PWATER;
				break;
			default:
				ASSUME_UNREACHABLE;
				break;
			}
			snprintf(infostr, sizeof(infostr), "Back to %s", AllLevels[questlist[qn]._qdlvl].dLevelName);
			break;
		case WM_DIABSETLVL:
			// quest trigger
			// TODO: use dLevelName?
			switch (trigs[pcurstrig]._tlvl) {
			case SL_SKELKING:
				copy_cstr(infostr, "To King Leoric's Tomb");
				break;
			case SL_BONECHAMB:
				copy_cstr(infostr, "To The Chamber of Bone");
				break;
			case SL_MAZE:
				copy_cstr(infostr, "To Maze");
				break;
			case SL_POISONWATER:
				copy_cstr(infostr, "To A Dark Passage");
				break;
			case SL_VILEBETRAYER:
				copy_cstr(infostr, "To The Unholy Altar");
				break;
			default:
				ASSUME_UNREACHABLE
			}
			break;
		case WM_DIABTOWNWARP:
			switch (pcurstrig) {
			case TWARP_CATHEDRAL:
				copy_cstr(infostr, "Down to dungeon");
				break;
			case TWARP_CATACOMB:
				copy_cstr(infostr, "Down to catacombs");
				break;
			case TWARP_CAVES:
				copy_cstr(infostr, "Down to caves");
				break;
			case TWARP_HELL:
				copy_cstr(infostr, "Down to hell");
				break;
#ifdef HELLFIRE
			case TWARP_NEST:
				copy_cstr(infostr, "Down to nest");
				break;
			case TWARP_CRYPT:
				copy_cstr(infostr, "Down to crypt");
				break;
#endif
			default:
				ASSUME_UNREACHABLE
			}
			break;
		case WM_DIABTWARPUP:
			copy_cstr(infostr, "Up to town");
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}

	GetMousePos(cursmx - 1, cursmy - 1, &xx, &yy);
	DrawTooltip(infostr, xx, yy, COL_WHITE);
}

void DrawInfoStr()
{
	int x, y, xx, yy;
	BYTE col;

	if (pcursitem != ITEM_NONE) {
		ItemStruct* is = &items[pcursitem];
		GetItemInfo(is);
		x = is->_ix;
		y = is->_iy;
		GetMousePos(x, y, &xx, &yy);
		DrawTooltip(infostr, xx, yy, infoclr);
	} else if (pcursobj != OBJ_NONE) {
		GetObjectStr(pcursobj);
		ObjectStruct* os = &object[pcursobj];
		x = os->_ox - 1;
		y = os->_oy - 1;
		GetMousePos(x, y, &xx, &yy);
		DrawTooltip(infostr, xx, yy, infoclr);
	} else if (pcursmonst != -1) {
		MonsterStruct* mon = &monster[pcursmonst];
		x = mon->_mx - 2;
		y = mon->_my - 2;
		col = COL_WHITE;
		if (currLvl._dType != DTYPE_TOWN) {
			strcpy(infostr, mon->mName);
			if (mon->_uniqtype != 0) {
				col = COL_GOLD;
			}
		} else if (pcursitem == ITEM_NONE) {
			strcpy(infostr, towners[pcursmonst]._tName);
		}
		GetMousePos(x, y, &xx, &yy);
		xx += DrawTooltip(infostr, xx, yy, col);
		DrawHealthBar(mon->_mhitpoints, mon->_mmaxhp, xx, yy);
	} else if (pcursplr != PLR_NONE) {
		PlayerStruct* p = &players[pcursplr];
		x = p->_px - 2;
		y = p->_py - 2;
		GetMousePos(x, y, &xx, &yy);
		snprintf(infostr, sizeof(infostr), p->_pManaShield == 0 ? "%s(%i)" : "%s(%i)*", ClassStrTbl[p->_pClass], p->_pLevel);
		xx += DrawTooltip2(p->_pName, infostr, xx, yy, COL_GOLD);
		DrawHealthBar(p->_pHitPoints, p->_pMaxHP, xx, yy + 10);
	} else if (gbSkillListFlag) {
		if (currSkill == SPL_INVALID || currSkill == SPL_NULL)
			return;
		const char* fmt;
		switch (currSkillType) {
		case RSPLTYPE_ABILITY:
			fmt = "%s Skill";
			break;
		case RSPLTYPE_SPELL:
			fmt = "%s Spell";
			break;
		case RSPLTYPE_SCROLL:
			fmt = "Scroll of %s";
			break;
		case RSPLTYPE_CHARGES:
			fmt = "Staff of %s";
			break;
		//case RSPLTYPE_INVALID:
		//	break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		snprintf(infostr, sizeof(infostr), fmt, spelldata[currSkill].sNameText);
		DrawTooltip(infostr, MouseX, MouseY - 8, COL_WHITE);
	} else if (pcursinvitem != INVITEM_NONE) {
		DrawInvItemDetails();
	} else if (pcurstrig != -1) {
		DrawTrigInfo();
	} else if (pcurs >= CURSOR_FIRSTITEM) {
		ItemStruct *is = &myplr.HoldItem;
		GetItemInfo(is);
		DrawTooltip(infostr, MouseX + cursW / 2, MouseY, infoclr);
	}
}


bool CheckChrBtns()
{
	int i;

	if (myplr._pStatPts != 0 && !gbChrbtnactive) {
		for (i = 0; i < lengthof(ChrBtnsRect); i++) {
			if (MouseX < ChrBtnsRect[i].x
			 || MouseX > ChrBtnsRect[i].x + ChrBtnsRect[i].w
			 || MouseY < ChrBtnsRect[i].y
			 || MouseY > ChrBtnsRect[i].y + ChrBtnsRect[i].h)
				continue;

			_gabChrbtn[i] = true;
			gbChrbtnactive = true;
			return true;
		}
	}
	return false;
}

void ReleaseChrBtns()
{
	int i;

	gbChrbtnactive = false;
	static_assert(lengthof(_gabChrbtn) == lengthof(ChrBtnsRect), "Mismatching _gabChrbtn and ChrBtnsRect tables.");
	static_assert(lengthof(_gabChrbtn) == 4, "Table _gabChrbtn does not work with ReleaseChrBtns function.");
	for (i = 0; i < lengthof(_gabChrbtn); ++i) {
		if (_gabChrbtn[i]) {
			_gabChrbtn[i] = false;
			if (MouseX >= ChrBtnsRect[i].x
			 && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
			 && MouseY >= ChrBtnsRect[i].y
			 && MouseY <= ChrBtnsRect[i].y + ChrBtnsRect[i].h) {
				switch (i) {
				case 0:
					NetSendCmd(true, CMD_ADDSTR);
					break;
				case 1:
					NetSendCmd(true, CMD_ADDMAG);
					break;
				case 2:
					NetSendCmd(true, CMD_ADDDEX);
					break;
				case 3:
					NetSendCmd(true, CMD_ADDVIT);
					break;
				default:
					ASSUME_UNREACHABLE
					break;
				}
				myplr._pStatPts--;
			}
		}
	}
}

static int DrawDurIcon4Item(ItemStruct *pItem, int x, int c)
{
	if (pItem->_itype == ITYPE_NONE)
		return x;
	if (pItem->_iDurability > 5)
		return x;
	if (c == 0) {
		if (pItem->_iClass == ICLASS_WEAPON) {
			switch (pItem->_itype) {
			case ITYPE_SWORD:
				c = 2;
				break;
			case ITYPE_AXE:
				c = 6;
				break;
			case ITYPE_BOW:
				c = 7;
				break;
			case ITYPE_MACE:
				c = 5;
				break;
			case ITYPE_STAFF:
				c = 8;
				break;
			}
		} else {
			c = 1;
		}
	}
	if (pItem->_iDurability > 2)
		c += 8;
	CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - 8, pDurIcons, c, 32);
	return x - 32 - 8;
}

void DrawDurIcon()
{
	ItemStruct *inv;
	int x;

	x = SCREEN_X + SCREEN_WIDTH - (SPLICONLENGTH + 92 + 32);

	inv = myplr.InvBody;
	x = DrawDurIcon4Item(&inv[INVLOC_HEAD], x, 4);
	x = DrawDurIcon4Item(&inv[INVLOC_CHEST], x, 3);
	x = DrawDurIcon4Item(&inv[INVLOC_HAND_LEFT], x, 0);
	DrawDurIcon4Item(&inv[INVLOC_HAND_RIGHT], x, 0);
}

void RedBack()
{
	const int idx = (LIGHTMAX + 3) * 256;

	assert(gpBuffer != NULL);

	int w, h;
	BYTE *dst, *tbl;

	dst = &gpBuffer[SCREENXY(0, 0)];
	tbl = &pLightTbl[idx];
	if (currLvl._dType != DTYPE_HELL) {
		for (h = VIEWPORT_HEIGHT; h; h--, dst += BUFFER_WIDTH - SCREEN_WIDTH) {
			for (w = SCREEN_WIDTH; w; w--) {
				*dst = tbl[*dst];
				dst++;
			}
		}
	} else {
		for (h = VIEWPORT_HEIGHT; h; h--, dst += BUFFER_WIDTH - SCREEN_WIDTH) {
			for (w = SCREEN_WIDTH; w; w--) {
				if (*dst >= 32)
					*dst = tbl[*dst];
				dst++;
			}
		}
	}
}

static char GetSBookTrans(int sn)
{
	PlayerStruct *p;
	char st;

	p = &myplr;
	if (p->_pAblSkills & SPELL_MASK(sn)) { /// BUGFIX: missing (uint64_t) (fixed)
		st = RSPLTYPE_ABILITY;
	} else if (p->_pISpells & SPELL_MASK(sn)) {
		st = RSPLTYPE_CHARGES;
	} else if (CheckSpell(mypnum, sn)) {
		st = RSPLTYPE_SPELL;
	} else {
		st = RSPLTYPE_INVALID;
	}
	return st;
}

#define SBOOK_CELWIDTH		37
#define SBOOK_CELHEIGHT		38
#define SBOOK_CELBORDER		 1
#define SBOOK_TOP_BORDER	 1
#define SBOOK_LEFT_BORDER	 1
#define SBOOK_LINE_BORDER	10
#define SBOOK_LINE_LENGTH	SPANEL_WIDTH - 2 * SBOOK_LEFT_BORDER - SBOOK_CELBORDER - SBOOK_CELWIDTH - 2 * SBOOK_LINE_BORDER
#define SBOOK_LINE_TAB		SBOOK_CELWIDTH + SBOOK_CELBORDER + SBOOK_LINE_BORDER
#define SBOOK_PAGER_WIDTH	56

void DrawSpellBook()
{
	PlayerStruct* p;
	int i, sn, mana, lvl, sx, yp, offset, min, max;
	char st;
	uint64_t spl;

	// back panel
	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pSpellBkCel, 1, SPANEL_WIDTH);
	// selected page
	snprintf(tempstr, sizeof(tempstr), "%d.", guBooktab + 1);
	PrintString(RIGHT_PANEL_X + 2, SCREEN_Y + SPANEL_HEIGHT - 7, RIGHT_PANEL_X + SPANEL_WIDTH, tempstr, true, COL_WHITE, 0);

	currSkill = SPL_INVALID;

	p = &myplr;
	spl = p->_pMemSkills | p->_pISpells | p->_pAblSkills;

	yp = SCREEN_Y + SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx = RIGHT_PANEL_X + SBOOK_CELBORDER;
	for (i = 0; i < lengthof(SpellPages[guBooktab]); i++) {
		sn = SpellPages[guBooktab][i];
		if (sn != SPL_INVALID && (spl & SPELL_MASK(sn))) {
			st = GetSBookTrans(sn);
			if (MouseX >= sx - BORDER_LEFT
			 && MouseX < sx - BORDER_LEFT + SBOOK_CELWIDTH
			 && MouseY >= yp - BORDER_TOP - SBOOK_CELHEIGHT
			 && MouseY < yp - BORDER_TOP) {
				currSkill = sn;
				currSkillType = st;
			}
			switch (st) {
			case RSPLTYPE_ABILITY:
				copy_cstr(tempstr, "Skill");
				mana = 0;
				break;
			case RSPLTYPE_CHARGES:
				snprintf(tempstr, sizeof(tempstr), "Staff (%i charges)", p->InvBody[INVLOC_HAND_LEFT]._iCharges);
				mana = 0;
				break;
			case RSPLTYPE_SPELL:
			case RSPLTYPE_INVALID:
				lvl = GetSpellLevel(mypnum, sn);
				if (lvl > 0) {
					snprintf(tempstr, sizeof(tempstr), "Spell Level %i", lvl);
				} else {
					copy_cstr(tempstr, "Spell Level 0 - Unusable");
				}
				mana = GetManaAmount(mypnum, sn) >> 6;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			GetDamageAmt(sn, &min, &max);
			offset = mana == 0 && min == -1 ? 5 : 0;
			PrintString(sx + SBOOK_LINE_TAB, yp - 23 + offset, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, spelldata[sn].sNameText, false, COL_WHITE, 1);
			PrintString(sx + SBOOK_LINE_TAB, yp - 12 + offset, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, false, COL_WHITE, 1);

			if (offset == 0) {
				if (mana != 0)
					cat_str(tempstr, offset, "Mana: %i  ", mana);
				if (min != -1)
					cat_str(tempstr, offset, "Dam: %i-%i", min, max);
				PrintString(sx + SBOOK_LINE_TAB, yp - 1, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, false, COL_WHITE, 1);
			}

			if ((spelldata[sn].sFlags & p->_pSkillFlags) != spelldata[sn].sFlags)
				st = RSPLTYPE_INVALID;
			SetSpellTrans(st);
			DrawSpellCel(sx, yp, pSBkIconCels, spelldata[sn].sIcon, SBOOK_CELWIDTH);
			// TODO: differenciate between Atk/Move skill ? Add icon for primary skills?
			if ((sn == p->_pAltAtkSkill && st == p->_pAltAtkSkillType)
			 || (sn == p->_pAltMoveSkill && st == p->_pAltMoveSkillType)) {
				SetSpellTrans(RSPLTYPE_ABILITY);
				DrawSpellCel(sx, yp, pSBkIconCels, SPLICONLAST, SBOOK_CELWIDTH);
			}
		}
		yp += SBOOK_CELBORDER + SBOOK_CELHEIGHT;
	}
}

void SelectBookSkill(bool shift, bool altSkill)
{
	int dx, dy;

	if (currSkill != SPL_INVALID) {
		SetSkill(shift, altSkill);
		return;
	}

	dx = MouseX - (RIGHT_PANEL + SBOOK_LEFT_BORDER);
	if (dx < 0)
		return;
	dy = MouseY - SBOOK_TOP_BORDER;
	if (dy < 0)
		return;

	if (dy >= lengthof(SpellPages[guBooktab]) * (SBOOK_CELBORDER + SBOOK_CELHEIGHT)) {
		if (dx <= SBOOK_PAGER_WIDTH * 2) {
			if (dx <= SBOOK_PAGER_WIDTH) {
				guBooktab = 0;
			} else {
				if (guBooktab != 0)
					guBooktab--;
			}
		} else if (dx >= SPANEL_WIDTH - SBOOK_PAGER_WIDTH * 2) {
			if (dx >= SPANEL_WIDTH - SBOOK_PAGER_WIDTH) {
				guBooktab = SPLBOOKTABS - 1;
			} else {
				if (guBooktab < SPLBOOKTABS - 1)
					guBooktab++;
			}
		}
	}
}

const char *get_pieces_str(int nGold)
{
	const char *result;

	result = "piece";
	if (nGold != 1)
		result = "pieces";
	return result;
}

void DrawGoldSplit(int amount)
{
	int screen_x;

	screen_x = 0;
	CelDraw(351 + SCREEN_X, 178 + SCREEN_Y, pGBoxBuff, 1, 261);
	snprintf(tempstr, sizeof(tempstr), "You have %u gold", initialDropGoldValue);
	ADD_PlrStringXY(366, 87, 600, tempstr, COL_GOLD);
	snprintf(tempstr, sizeof(tempstr), "%s.  How many do", get_pieces_str(initialDropGoldValue));
	ADD_PlrStringXY(366, 103, 600, tempstr, COL_GOLD);
	ADD_PlrStringXY(366, 121, 600, "you want to remove?", COL_GOLD);
	if (amount > 0) {
		snprintf(tempstr, sizeof(tempstr), "%u", amount);
		PrintGameStr(388, 140, tempstr, COL_WHITE);
		screen_x += GetStringWidth(tempstr);
		screen_x += 452;
	} else {
		screen_x = 450;
	}
	DrawSinglePentSpn2(screen_x, 140 + SCREEN_Y);
}

static void control_remove_gold()
{
	ItemStruct *is;
	int val;
	int pnum = mypnum, gi = initialDropGoldIndex;

	assert(initialDropGoldIndex <= INVITEM_INV_LAST && initialDropGoldIndex >= INVITEM_INV_FIRST);
	gi = initialDropGoldIndex - INVITEM_INV_FIRST;
	is = &players[pnum].InvList[gi];
	val = is->_ivalue - dropGoldValue;
	if (val > 0) {
		SetGoldItemValue(is, val);
	} else {
		PlrInvItemRemove(pnum, gi);
	}
	is = &players[pnum].HoldItem;
	CreateBaseItem(is, IDI_GOLD);
	is->_iStatFlag = TRUE;
	SetGoldItemValue(is, dropGoldValue);
	NewCursor(is->_iCurs + CURSOR_FIRSTITEM);
	CalculateGold(pnum);
}

void control_drop_gold(char vkey)
{
	int newValue;

	if (myplr._pHitPoints < (1 << 6)) {
		gbDropGoldFlag = false;
		return;
	}

	if (vkey == DVL_VK_RETURN) {
		if (dropGoldValue > 0)
			control_remove_gold();
	} else if (vkey == DVL_VK_BACK) {
		dropGoldValue /= 10;
		return;
	} else if (vkey == DVL_VK_DELETE) {
		dropGoldValue = 0;
		return;
	} else if (vkey >= DVL_VK_0 && vkey <= DVL_VK_9) {
		newValue = dropGoldValue * 10 + vkey - DVL_VK_0;
		if (newValue <= initialDropGoldValue)
			dropGoldValue = newValue;
		return;
	} else if (vkey >= DVL_VK_NUMPAD0 && vkey <= DVL_VK_NUMPAD9) {
		newValue = dropGoldValue * 10 + vkey - DVL_VK_NUMPAD0;
		if (newValue <= initialDropGoldValue)
			dropGoldValue = newValue;
		return;
	}
	gbDropGoldFlag = false;
}

static void DrawTeamButton(int x, int y, int width, bool pressed, const char* label, int txtoff)
{
	const int height = 12;
	BYTE* dst;
	int color;

	// draw gray border
	color = pressed ? PAL16_GRAY + 8 : PAL16_GRAY + 6;
	dst = &gpBuffer[SCREENXY(x - SCREEN_X, y - height - SCREEN_Y)];
	for (int i = 0; i < height; i++, dst += BUFFER_WIDTH)
		memset(dst, color, width);

	// draw background
	color = pressed ? PAL16_ORANGE + 8 : PAL16_ORANGE + 10;
	dst = &gpBuffer[SCREENXY(x - SCREEN_X + 1, y - height - SCREEN_Y + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, color, width - 2);

	// label
	color = pressed ? COL_GOLD : COL_WHITE;
	x += txtoff;
	PrintString(x, y - 1, x + SBOOK_LINE_LENGTH, label, false, color, 1);
}

static bool PlrHasTeam()
{
	int i;

	for (i = 0; i < MAX_PLRS; i++)
		if (i != mypnum && players[i]._pTeam == myplr._pTeam && players[i].plractive)
			return true;
	return false;
}

#define TBOOK_BTN_WIDTH	60
#define TBOOKTABS		((MAX_PLRS + NUM_BOOK_ENTRIES - 1) / NUM_BOOK_ENTRIES)
void DrawTeamBook()
{
	PlayerStruct* p;
	int i, pnum, sx, yp;
	bool hasTeam;

	// back panel
	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pSpellBkCel, 1, SPANEL_WIDTH);
	// selected page
	snprintf(tempstr, sizeof(tempstr), "%d.", guTeamTab + 1);
	PrintString(RIGHT_PANEL_X + 2, SCREEN_Y + SPANEL_HEIGHT - 7, RIGHT_PANEL_X + SPANEL_WIDTH, tempstr, true, COL_WHITE, 0);

	hasTeam = PlrHasTeam();

	yp = SCREEN_Y + SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx = RIGHT_PANEL_X + SBOOK_CELBORDER;
	for (i = 0; i < NUM_BOOK_ENTRIES; i++) {
		pnum = i + guTeamTab * NUM_BOOK_ENTRIES;
		if (pnum >= MAX_PLRS)
			break;
		p = &players[pnum];
		if (!p->plractive)
			continue;
		// name
		PrintString(sx + SBOOK_LINE_TAB, yp - 25, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, p->_pName, false, COL_WHITE, 0);
		// class(level) - team
		static_assert(MAXCHARLEVEL < 100, "Level must fit to the TeamBook.");
		snprintf(tempstr, sizeof(tempstr), "%s (lvl:%2d) %c", ClassStrTbl[p->_pClass], p->_pLevel, 'a' + p->_pTeam);
		PrintString(sx + SBOOK_LINE_TAB, yp - 13, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, false, COL_WHITE, 1);

		// mute
		if (pnum != mypnum) {
			DrawTeamButton(sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH - (TBOOK_BTN_WIDTH - 8), yp - 24, TBOOK_BTN_WIDTH,
				(guTeamMute & (1 << pnum)) != 0, "mute", 10);
		}

		// drop/leave
		if (hasTeam && (pnum == mypnum || p->_pTeam == mypnum)) {
			DrawTeamButton(sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH - (TBOOK_BTN_WIDTH - 8), yp - 12, TBOOK_BTN_WIDTH, false,
				pnum == mypnum ? "leave" : "drop", pnum == mypnum ? 8 : 12);
		}

		// accept/reject
		if (guTeamInviteRec & (1 << pnum)) {
			DrawTeamButton(sx + SBOOK_LINE_TAB, yp, TBOOK_BTN_WIDTH,
				false, "accept", 2);
			DrawTeamButton(sx + SBOOK_LINE_TAB + TBOOK_BTN_WIDTH + 10, yp, TBOOK_BTN_WIDTH,
				false, "reject", 6);
		}

		// invite/cancel
		if (pnum != mypnum && players[pnum]._pTeam != myplr._pTeam && myplr._pTeam == mypnum) {
			unsigned invited = (guTeamInviteSent & (1 << pnum));
			DrawTeamButton(sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH - (TBOOK_BTN_WIDTH - 8), yp, TBOOK_BTN_WIDTH, false,
				!invited ? "invite" : "cancel", !invited ? 7 : 2);
		}

		// icon
		DrawSpellCel(sx, yp, pSBkIconCels, ClassIconTbl[p->_pClass], SBOOK_CELWIDTH);

		yp += SBOOK_CELBORDER + SBOOK_CELHEIGHT;
	}
}

void CheckTeamClick(bool shift)
{
	int dx, dy;

	dx = MouseX - (RIGHT_PANEL + SBOOK_LEFT_BORDER);
	if (dx < 0)
		return;
	dy = MouseY - SBOOK_TOP_BORDER;
	if (dy < 0)
		return;

	if (dy < NUM_BOOK_ENTRIES * (SBOOK_CELBORDER + SBOOK_CELHEIGHT)) {
		int pnum = dy / (SBOOK_CELBORDER + SBOOK_CELHEIGHT);
		dy = dy % (SBOOK_CELBORDER + SBOOK_CELHEIGHT);
		pnum += guTeamTab * NUM_BOOK_ENTRIES;
		if (pnum >= MAX_PLRS || !players[pnum].plractive)
			return;
		if (dx <= SBOOK_CELWIDTH) {
			// clicked on the icon
			if (!gbTalkflag)
				control_type_message();
			if (!shift) {
				snprintf(sgszTalkMsg, sizeof(sgszTalkMsg), "/p%d ", pnum);
			} else {
				snprintf(sgszTalkMsg, sizeof(sgszTalkMsg), "/t%d ", players[pnum]._pTeam);
			}
		} else if (dx > SBOOK_LINE_TAB + SBOOK_LINE_LENGTH - (TBOOK_BTN_WIDTH - 8)
		 && dx <= SBOOK_LINE_TAB + SBOOK_LINE_LENGTH + 8) {
			// clicked on the right column of buttons
			dy = 3 * dy / (SBOOK_CELBORDER + SBOOK_CELHEIGHT);
			if (dy == 0) {
				// mute
				if (pnum != mypnum)
					guTeamMute ^= (1 << pnum);
			} else if (dy == 1) {
				// drop/leave
				if (PlrHasTeam() && (pnum == mypnum || players[pnum]._pTeam == mypnum))
					NetSendCmdBParam1(false, CMD_KICK_PLR, pnum);
			} else /*if (dy == 2)*/ {
				// invite/cancel
				if (pnum != mypnum && players[pnum]._pTeam != myplr._pTeam && myplr._pTeam == mypnum) {
					if (guTeamInviteSent & (1 << pnum)) {
						NetSendCmdBParam1(false, CMD_REV_INVITE, pnum);
					} else {
						NetSendCmdBParam1(false, CMD_INVITE, pnum);
					}
					guTeamInviteSent ^= (1 << pnum);
				}
			}
		} else if (dy >= (2 * (SBOOK_CELBORDER + SBOOK_CELHEIGHT) / 3)) {
			if (guTeamInviteRec & (1 << pnum)) {
				if (dx > SBOOK_LINE_TAB && dx < SBOOK_LINE_TAB + TBOOK_BTN_WIDTH) {
					// accept (invite)
					NetSendCmdBParam1(false, CMD_ACK_INVITE, pnum);
				} else if (dx > SBOOK_LINE_TAB + TBOOK_BTN_WIDTH + 10 && dx < SBOOK_LINE_TAB + 2 * TBOOK_BTN_WIDTH + 10) {
					// reject (invite)
					NetSendCmdBParam1(false, CMD_DEC_INVITE, pnum);
				}
				guTeamInviteRec &= ~(1 << pnum);
			}
		}
	} else {
		if (dx <= SBOOK_PAGER_WIDTH * 2) {
			if (dx <= SBOOK_PAGER_WIDTH) {
				guTeamTab = 0;
			} else {
				if (guTeamTab != 0)
					guTeamTab--;
			}
		} else if (dx >= SPANEL_WIDTH - SBOOK_PAGER_WIDTH * 2) {
			if (dx >= SPANEL_WIDTH - SBOOK_PAGER_WIDTH) {
				guTeamTab = TBOOKTABS - 1;
			} else {
				if (guTeamTab < TBOOKTABS - 1)
					guTeamTab++;
			}
		}
	}
}

#define TALK_PNL_WIDTH		302
#define TALK_PNL_HEIGHT		51
#define TALK_PNL_BORDER		15
#define TALK_PNL_TOP		(SCREEN_HEIGHT - 48 - TALK_PNL_HEIGHT)
#define TALK_PNL_LEFT		((SCREEN_WIDTH - TALK_PNL_WIDTH) / 2)
static char *control_print_talk_msg(char *msg, int *x, int y)
{
	int limit = TALK_PNL_WIDTH - 2 * TALK_PNL_BORDER;
	BYTE c;

	while (*msg != '\0') {
		c = sfontframe[gbFontTransTbl[(BYTE)*msg]];
		limit -= sfontkern[c] + 1;
		if (limit < 0)
			return msg;
		msg++;
		if (c != 0) {
			PrintChar(*x, y, c, COL_WHITE);
		}
		*x += sfontkern[c] + 1;
	}
	return NULL;
}

void DrawTalkPan()
{
	int x, y;
	char *msg;

	assert(gbTalkflag);

	int sx = SCREEN_X + TALK_PNL_LEFT;
	int sy = SCREEN_Y + TALK_PNL_TOP;

	// add background
	CelDraw(sx, sy + TALK_PNL_HEIGHT, pTalkPnl, 1, TALK_PNL_WIDTH);

	// print the current (not sent) message
	sy += 17;
	sx += TALK_PNL_BORDER;
	msg = sgszTalkMsg;
	for (y = sy; ; y += 13) {
		x = sx;
		msg = control_print_talk_msg(msg, &x, y);
		if (msg == NULL)
			break;
	}
	if (msg != NULL)
		*msg = '\0';
	DrawSinglePentSpn2(x, y);
}

void control_type_message()
{
	if (gbMaxPlayers == 1) {
		return;
	}

	gbTalkflag = true;
	SDL_StartTextInput();
	sgszTalkMsg[0] = '\0';
	//gbRedrawFlags = REDRAW_ALL;
	sgbTalkSavePos = sgbNextTalkSave;
}

void control_reset_talk()
{
	gbTalkflag = false;
	SDL_StopTextInput();
	//gbRedrawFlags = REDRAW_ALL;
}

static void control_press_enter()
{
	int i, team, pmask;
	BYTE talk_save;
	char* msg;

	pmask = -1;
	msg = sgszTalkMsg;
	if (msg[0] == '/') {
		if (msg[1] == 'p') {
			// "/pX msg" -> send message to player X
			i = strtol(&msg[2], &msg, 10);
			if (msg != &sgszTalkMsg[2]) {
				pmask = 1 << i;
				if (*msg == ' ') {
					msg++;
				}
			} else {
				msg = sgszTalkMsg;
			}
		} else if (msg[1] == 't') {
			team = -1;
			if (msg[2] == ' ') {
				// "/t msg" -> send message to the player's team
				team = myplr._pTeam;
			} else {
				// "/tX msg" -> send message to the team N
				team= strtol(&msg[2], &msg, 10);
				if (msg == &sgszTalkMsg[2]) {
					team = -1;
					msg = sgszTalkMsg;
				}
			}
			if (team!= -1) {
				pmask = 0;
				for (i = 0; i < MAX_PLRS; i++) {
					if (players[i]._pTeam == team)
						pmask |= 1 << i;
				}
			}
		}
	}

	if (*msg != '\0') {
		SStrCopy(gbNetMsg, msg, sizeof(gbNetMsg));
		NetSendCmdString(pmask);

		for (i = 0; i < lengthof(sgszTalkSave); i++) {
			if (!strcmp(sgszTalkSave[i], sgszTalkMsg))
				break;
		}
		if (i == lengthof(sgszTalkSave)) {
			copy_str(sgszTalkSave[sgbNextTalkSave], sgszTalkMsg);
			sgbNextTalkSave++;
			sgbNextTalkSave &= 7;
		} else {
			talk_save = sgbNextTalkSave - 1;
			talk_save &= 7;
			if (i != talk_save) {
				copy_str(sgszTalkSave[i], sgszTalkSave[talk_save]);
				copy_str(sgszTalkSave[talk_save], sgszTalkMsg);
			}
		}
		sgszTalkMsg[0] = '\0';
		sgbTalkSavePos = sgbNextTalkSave;
	}
	control_reset_talk();
}

bool control_talk_last_key(int vkey)
{
	unsigned result;

	assert(gbTalkflag);
	assert(gbMaxPlayers != 1);

	if ((unsigned)vkey < DVL_VK_SPACE)
		return false;

	result = strlen(sgszTalkMsg);
	if (result < sizeof(sgszTalkMsg) - 1) {
		sgszTalkMsg[result] = vkey;
		sgszTalkMsg[result + 1] = '\0';
	}
	return true;
}

static void control_up_down(int v)
{
	int i;

	static_assert(lengthof(sgszTalkSave) == 8, "Table sgszTalkSave does not work in control_up_down.");
	for (i = 0; i < lengthof(sgszTalkSave); i++) {
		sgbTalkSavePos = (v + sgbTalkSavePos) & 7;
		if (sgszTalkSave[sgbTalkSavePos][0] != '\0') {
			copy_str(sgszTalkMsg, sgszTalkSave[sgbTalkSavePos]);
			return;
		}
	}
}

bool control_presskeys(int vkey)
{
	int len;

	assert(gbTalkflag);

	if (vkey == DVL_VK_ESCAPE) {
		control_reset_talk();
	} else if (vkey == DVL_VK_RETURN) {
		control_press_enter();
	} else if (vkey == DVL_VK_BACK) {
		len = strlen(sgszTalkMsg);
		if (len > 0)
			sgszTalkMsg[len - 1] = '\0';
	} else if (vkey == DVL_VK_DOWN) {
		control_up_down(1);
	} else if (vkey == DVL_VK_UP) {
		control_up_down(-1);
	} else if (vkey == DVL_VK_LBUTTON) {
		return false;
	} else if (vkey == DVL_VK_RBUTTON) {
		return false;
	}
	return true;
}

DEVILUTION_END_NAMESPACE
