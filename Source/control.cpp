/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"
#include "plrctrls.h"
#include "engine/render/text_render.hpp"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

/** Menu button images CEL */
static CelImageBuf* pPanelButtonCels;
/** The number of buttons in the menu. */
int numpanbtns;
/** Specifies whether the menu-button is pressed. */
bool gabPanbtn[NUM_PANBTNS];
/** Flask images CEL */
static BYTE* pFlaskCels;
/** Specifies how much the life flask is filled (percentage). */
int gnHPPer;
/** Specifies how much the mana flask is filled (percentage). */
int gnManaPer;
/** Graphics for the (transparent) text box */
static CelImageBuf* pTextBoxCels;
/** Graphics for the (transparent) small text box */
static CelImageBuf* pSTextBoxCels;
/** Graphics for the scrollbar of text boxes. */
CelImageBuf* pSTextSlidCels;
/** Low-Durability images CEL */
static CelImageBuf* pDurIconCels;

/** The current tab in the Team-Book. */
static unsigned guTeamTab;
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
static CelImageBuf* pTalkPnlCel;
/** The current message in the Chat-Panel. */
static char sgszTalkMsg[MAX_SEND_STR_LEN];
/** The cached messages of the Chat-Panel. */
static char sgszTalkSave[8][MAX_SEND_STR_LEN];
/** The next position in the sgszTalkSave to save the message. */
static BYTE sgbNextTalkSave;
/** The index of selected message in the sgszTalkSave array. */
static BYTE sgbTalkSavePos;
/** Specifies whether the Golddrop is displayed. */
bool gbDropGoldFlag;
/** Golddrop background CEL */
static CelImageBuf* pGoldDropCel;
/** The gold-stack index which is used as a source in Golddrop. */
BYTE initialDropGoldIndex;
/** The gold-stack size which is used as a source in Golddrop. */
int initialDropGoldValue;
/** The current value in Golddrop. */
int dropGoldValue;
BYTE infoclr;
char tempstr[256];
char infostr[256];
/** Number of active windows on the screen. */
int gnNumActiveWindows;
/** The list of active windows on the screen. */
char gaActiveWindows[NUM_WNDS];
/** SpellBook background CEL */
static CelImageBuf* pSpellBkCel;
/** SpellBook icons CEL */
static CelImageBuf* pSBkIconCels;
/** The current tab in the Spell-Book. */
unsigned guBooktab;
/** Specifies whether the LevelUp button is displayed. */
bool gbLvlUp;
/** Specifies whether the LevelUp button is pressed. */
bool gbLvlbtndown;
/** Char-Panel background CEL */
static CelImageBuf* pChrPanelCel;
/** Char-Panel button images CEL */
static CelImageBuf* pChrButtonCels;
/** Specifies whether the button of the given attribute is pressed on Character-Panel. */
static bool _gabChrbtn[NUM_ATTRIBS];
/** Specifies whether any attribute-button is pressed on Character-Panel. */
bool gbChrbtnactive;
/** Color translations for the skill icons. */
#ifdef HELLFIRE
static BYTE SkillTrns[NUM_RSPLTYPES + 1][256];
#else
static BYTE SkillTrns[NUM_RSPLTYPES][256];
#endif
/** Specifies whether the Skill-List is displayed. */
bool gbSkillListFlag;
/** Skill-List images CEL */
static CelImageBuf* pSpellCels;
/** The 'highlighted' skill in the Skill-List or in the Spell-Book. */
static BYTE currSkill;
/** The type of the 'highlighted' skill in the Skill-List or in the Spell-Book. */
static BYTE currSkillType;
/** Specifies whether the cursor should be moved to the active skill in the Skill-List. */
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
static BYTE _gbMoveCursor = 0;
#endif

static const int PanBtnPos[NUM_PANBTNS][2] = {
	// clang-format off
	{   0, (1 + 0) * MENUBTN_HEIGHT }, // menu button
	{   0, (1 + 1) * MENUBTN_HEIGHT }, // options button
	{   0, (1 + 2) * MENUBTN_HEIGHT }, // char button
	{   0, (1 + 3) * MENUBTN_HEIGHT }, // inv button
	{   0, (1 + 4) * MENUBTN_HEIGHT }, // spells button
	{   0, (1 + 5) * MENUBTN_HEIGHT }, // quests button
	{   0, (1 + 6) * MENUBTN_HEIGHT }, // map button
	{   0, (1 + 7) * MENUBTN_HEIGHT }, // chat button
	{   0, (1 + 8) * MENUBTN_HEIGHT }, // teams button
	// clang-format on
};
static const char* PanBtnTxt[NUM_PANBTNS] = {
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
/** The number of spells/skills on a single spellbook page. */
#define NUM_BOOK_ENTRIES 7
/** Maps from spellbook page number and position to spell_id. TODO: add SPL_ATTRACT */
static BYTE SpellPages[SPLBOOKTABS][NUM_BOOK_ENTRIES] = {
	// clang-format off
	{ SPL_NULL, SPL_CBOLT, SPL_LIGHTNING, SPL_CHAIN, SPL_NOVA, SPL_TELEKINESIS, SPL_GOLEM },
	{ SPL_FIREBOLT, SPL_FIREBALL, SPL_INFERNO, SPL_FIREWALL, SPL_WAVE, SPL_GUARDIAN, SPL_ELEMENTAL },
	{ SPL_RAGE, SPL_SWIPE, SPL_POINT_BLANK, SPL_FAR_SHOT, SPL_HBOLT, SPL_FLARE, SPL_FLASH  },
	{ SPL_STONE, SPL_MANASHIELD, SPL_HEAL, SPL_HEALOTHER, SPL_RNDTELEPORT, SPL_TELEPORT, SPL_TOWN },
#ifdef HELLFIRE
	{ SPL_RUNEFIRE, SPL_FIRERING, SPL_RUNEWAVE, SPL_RUNELIGHT, SPL_RUNENOVA, SPL_INVALID, SPL_INVALID },
#endif
	// clang-format on
};
/** Maps from player-class to team-icon id in pSBkIconCels. */
static BYTE ClassIconTbl[NUM_CLASSES] = { 8, 13, 42,
#ifdef HELLFIRE
	41, 9, 38,
#endif
};

static BYTE GetSpellTrans(BYTE st, BYTE sn)
{
#ifdef HELLFIRE
	if (st != RSPLTYPE_SCROLL) return st;
	return SPELL_RUNE(sn) ? NUM_RSPLTYPES : RSPLTYPE_SCROLL;
#else
	return st;
#endif
}

/*static void SetSpellTrans(char st)
{
	int i;

	if (lastSt == st)
		return;
	lastSt = st;

	switch (st) {
	case RSPLTYPE_ABILITY:
		SplTransTbl[PAL8_YELLOW] = PAL8_YELLOW + 1;
		SplTransTbl[PAL8_YELLOW + 1] = PAL8_YELLOW + 1;
		SplTransTbl[PAL8_YELLOW + 2] = PAL8_YELLOW + 2;
		for (i = 0; i < 16; i++) {
			SplTransTbl[PAL16_BEIGE + i] = PAL16_BEIGE + i;
			SplTransTbl[PAL16_YELLOW + i] = PAL16_YELLOW + i;
			SplTransTbl[PAL16_ORANGE + i] = PAL16_ORANGE + i;
		}
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
			SplTransTbl[i] = i;
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
			SplTransTbl[i] = i;
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
}*/

static void DrawSpellIconOverlay(int x, int y, int sn, int st, int lvl)
{
	ItemStruct* pi;
	int t, v;

	switch (st) {
	case RSPLTYPE_ABILITY:
		return;
	case RSPLTYPE_SPELL:
		t = COL_WHITE;
		if (lvl > 0) {
			snprintf(tempstr, sizeof(tempstr), "lvl%02d", lvl);
		} else if (lvl == 0) {
			copy_cstr(tempstr, "X");
			t = COL_RED;
		} else { // SPLLVL_UNDEF
			copy_cstr(tempstr, "?");
		}
		break;
	case RSPLTYPE_SCROLL:
		v = 0;
		pi = myplr._pInvList;
		for (t = NUM_INV_GRID_ELEM; t > 0; t--, pi++) {
			if (pi->_itype == ITYPE_MISC && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_RUNE) && pi->_iSpell == sn) {
				// assert(pi->_iUsable);
				v += pi->_iDurability; // STACK
			}
		}
		pi = myplr._pSpdList;
		for (t = MAXBELTITEMS; t > 0; t--, pi++) {
			if (pi->_itype == ITYPE_MISC && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_RUNE) && pi->_iSpell == sn) {
				// assert(pi->_iUsable);
				v += pi->_iDurability; // STACK
			}
		}
		snprintf(tempstr, sizeof(tempstr), "%d", v);
		t = COL_WHITE;
		break;
	case RSPLTYPE_CHARGES:
		pi = &myplr._pInvBody[INVLOC_HAND_LEFT];
		if (pi->_iMagical != ITEM_QUALITY_NORMAL && !pi->_iIdentified)
			return;
		snprintf(tempstr, sizeof(tempstr), "%d/%d", pi->_iCharges, pi->_iMaxCharges);
		t = COL_WHITE;
		break;
	case RSPLTYPE_INVALID:
		return;
	default:
		ASSUME_UNREACHABLE
		return;
	}
	PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, true, t, FONT_KERN_SMALL);
}

static void DrawSkillIcon(int pnum, BYTE spl, BYTE st, BYTE offset)
{
	int lvl = 0, y;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (spl == SPL_INVALID) {
		st = RSPLTYPE_INVALID;
		spl = SPL_NULL;
	} else if ((spelldata[spl].sUseFlags & plr._pSkillFlags) != spelldata[spl].sUseFlags)
		st = RSPLTYPE_INVALID;
	else if (st == RSPLTYPE_SPELL) {
		lvl = plr._pSkillLvl[spl];
		if (lvl <= 0 || plr._pMana < GetManaAmount(pnum, spl))
			st = RSPLTYPE_INVALID;
		if (plr._pHasUnidItem)
			lvl = -1; // SPLLVL_UNDEF
	}
	y = PANEL_Y + PANEL_HEIGHT - 1 - offset;
	CelDrawLight(PANEL_X + PANEL_WIDTH - SPLICONLENGTH, y, pSpellCels,
		spelldata[spl].sIcon, SkillTrns[GetSpellTrans(st, spl)]);
	DrawSpellIconOverlay(PANEL_X + PANEL_WIDTH - SPLICONLENGTH, y, spl, st, lvl);
}

bool ToggleWindow(char idx)
{
	int i;

	for (i = 0; i < gnNumActiveWindows; i++) {
		if (gaActiveWindows[i] == idx) {
			gnNumActiveWindows--;
			for ( ; i < gnNumActiveWindows; i++) {
				gaActiveWindows[i] = gaActiveWindows[i + 1];
			}
			return false;
		}
	}

	gaActiveWindows[gnNumActiveWindows] = idx;
	gnNumActiveWindows++;
	return true;
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
void DrawSkillIcons()
{
	int pnum;
	BYTE spl, type;

	pnum = mypnum;
	if (plr._pAtkSkill == SPL_INVALID) {
		spl = plr._pMoveSkill;
		type = plr._pMoveSkillType;
	} else {
		spl = plr._pAtkSkill;
		type = plr._pAtkSkillType;
	}
	DrawSkillIcon(pnum, spl, type, 0);

	if (plr._pAltAtkSkill == SPL_INVALID) {
		spl = plr._pAltMoveSkill;
		type = plr._pAltMoveSkillType;
	} else {
		spl = plr._pAltAtkSkill;
		type = plr._pAltAtkSkillType;
	}
	DrawSkillIcon(pnum, spl, type, SPLICONLENGTH);
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
		PrintString(x + offset, y - SPLICONLENGTH + 16, x + offset + 18, tempstr, false, col, FONT_KERN_SMALL);
	}
}

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
static bool MoveToAtkMoveSkill(int sn, int st, BYTE atk_sn, BYTE atk_st, BYTE move_sn, BYTE move_st)
{
	if (atk_sn != SPL_INVALID)
		return sn == atk_sn && st == atk_st;
	if (move_sn != SPL_INVALID)
		return sn == move_sn && st == move_st;
	return sn == SPL_NULL || sn == SPL_INVALID;
}

static bool MoveToSkill(int pnum, int sn, int st)
{
	if (_gbMoveCursor == 0)
		return false;
	if (_gbMoveCursor == 1) {
		return MoveToAtkMoveSkill(sn, st,
			plr._pAltAtkSkill, plr._pAltAtkSkillType,
			plr._pAltMoveSkill, plr._pAltMoveSkillType);
	} else {
		return MoveToAtkMoveSkill(sn, st,
			plr._pAtkSkill, plr._pAtkSkillType,
			plr._pMoveSkill, plr._pMoveSkillType);
	}
}
#endif

void DrawSkillList()
{
	int pnum, i, j, x, y, sx, /*c,*/ sl, sn, st, lx, ly;
	uint64_t mask;

	currSkill = SPL_INVALID;
	sx = PANEL_CENTERX(SPLICONLENGTH * SPLROWICONLS);
	x = sx + SPLICONLENGTH * SPLROWICONLS - SPLICONLENGTH;
	y = SCREEN_Y + PANEL_BOTTOM - (128 + 17);
	//y = PANEL_CENTERY(190) + 190;
	pnum = mypnum;
	static_assert(RSPLTYPE_ABILITY == 0, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 1.");
	static_assert(RSPLTYPE_SPELL == 1, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 2.");
	static_assert(RSPLTYPE_SCROLL == 2, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 3.");
	static_assert(RSPLTYPE_CHARGES == 3, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 4.");
	for (i = 0; i < 4; i++) {
		switch (i) {
		case RSPLTYPE_ABILITY:
			mask = plr._pAblSkills;
			//c = SPLICONLAST + 3;
			break;
		case RSPLTYPE_SPELL:
			mask = plr._pMemSkills;
			//c = SPLICONLAST + 4;
			break;
		case RSPLTYPE_SCROLL:
			mask = plr._pScrlSkills;
			//c = SPLICONLAST + 1;
			break;
		case RSPLTYPE_CHARGES:
			mask = plr._pISpells;
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
				sl = plr._pSkillLvl[j];
				st = sl > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID;
				if (plr._pHasUnidItem)
					sl = -1; // SPLLVL_UNDEF
			}
			if ((spelldata[j].sUseFlags & plr._pSkillFlags) != spelldata[j].sUseFlags)
				st = RSPLTYPE_INVALID;
			else
				st = GetSpellTrans(st, j);
			CelDrawLight(x, y, pSpellCels, spelldata[j].sIcon, SkillTrns[st]);
			lx = x - BORDER_LEFT;
			ly = y - BORDER_TOP - SPLICONLENGTH;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
			if (MoveToSkill(pnum, j, i)) {
				SetCursorPos(lx + SPLICONLENGTH / 2, ly + SPLICONLENGTH / 2);
			}
#endif
			if (POS_IN_RECT(MouseX, MouseY, lx, ly, SPLICONLENGTH, SPLICONLENGTH)) {
				//CelDrawLight(x, y, pSpellCels, c, SkillTrns[st]);
				CelDrawLight(x, y, pSpellCels, SPLICONLAST, SkillTrns[st]);

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
					plr._pAtkSkillHotKey, plr._pAtkSkillTypeHotKey,
					plr._pMoveSkillHotKey, plr._pMoveSkillTypeHotKey);

				DrawSkillIconHotKey(x, y, sn, st, SPLICONLENGTH - 18, 
					plr._pAltAtkSkillHotKey, plr._pAltAtkSkillTypeHotKey,
					plr._pAltMoveSkillHotKey, plr._pAltMoveSkillTypeHotKey);
			}
			x -= SPLICONLENGTH;
			if (x == sx - SPLICONLENGTH) {
				x = sx + SPLICONLENGTH * SPLROWICONLS - SPLICONLENGTH;
				y -= SPLICONLENGTH;
			}
		}
		if (j != 0 && x != sx + SPLICONLENGTH * SPLROWICONLS - SPLICONLENGTH) {
			x -= SPLICONLENGTH;
			if (x == sx - SPLICONLENGTH) {
				x = sx + SPLICONLENGTH * SPLROWICONLS - SPLICONLENGTH;
				y -= SPLICONLENGTH;
			}
		}
	}
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
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
	PlayerStruct* p;
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
	moveskill = sn == SPL_WALK || sn == SPL_CHARGE || sn == SPL_TELEPORT || sn == SPL_RNDTELEPORT;

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
	PlayerStruct* p;
	int sn = currSkill;
	bool moveskill;

	if (sn != SPL_INVALID) {
		// TODO: add flag for movement-skills
		moveskill = sn == SPL_WALK || sn == SPL_CHARGE || sn == SPL_TELEPORT || sn == SPL_RNDTELEPORT;
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
	BYTE* destSkill, BYTE* destSkillType)
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
	PlayerStruct* p;

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
/*static void SetFlaskHeight(BYTE* pCelBuff, int min, int max, int sx, int sy)
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
/*static void DrawFlask(BYTE* pCelBuff, int w, int nSrcOff, int nDstOff, int h)
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

	sy = PANEL_Y + PANEL_HEIGHT - 1;

	filled += FLASK_TOTAL_HEIGHT - FLASK_BULB_HEIGHT;
	int emptied = FLASK_TOTAL_HEIGHT - filled;
	empty = CelGetFrame(pFlaskCels, emptyCel, &dataSize);
	full = CelGetFrame(pFlaskCels, fullCel, &dataSize);

	BYTE* dst = &gpBuffer[sx + BUFFER_WIDTH * sy];
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
			filled = FLASK_BULB_HEIGHT * hp / maxHP;
		if (filled > FLASK_BULB_HEIGHT)
			filled = FLASK_BULB_HEIGHT;
		gnHPPer = filled;
	} else {
		filled = gnHPPer;
	}

	x = LIFE_FLASK_X;
	DrawFlask2(x, filled, 1, 2, LIFE_FLASK_WIDTH);
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
			filled = FLASK_BULB_HEIGHT * mana / maxMana;
		if (filled > FLASK_BULB_HEIGHT)
			filled = FLASK_BULB_HEIGHT;
		gnManaPer = filled;
	} else {
		filled = gnManaPer;
	}

	x = MANA_FLASK_X;
	DrawFlask2(x, filled, 3, myplr._pManaShield == 0 ? 4 : 5, MANA_FLASK_WIDTH);
}

void InitControlPan()
{
	int i;

	assert(pFlaskCels == NULL);
	pFlaskCels = LoadFileInMem("CtrlPan\\Flasks.CEL");;
	assert(pChrPanelCel == NULL);
	pChrPanelCel = CelLoadImage("Data\\Char.CEL", SPANEL_WIDTH);
	assert(pSpellCels == NULL);
#ifdef HELLFIRE
	pSpellCels = CelLoadImage("Data\\SpelIcon.CEL", SPLICONLENGTH);
#else
	pSpellCels = CelLoadImage("CtrlPan\\SpelIcon.CEL", SPLICONLENGTH);
#endif
	/*for (i = 0; i < NUM_RSPLTYPES; i++) {
		for (j = 0; j < 256; j++)
			SplTransTbl[j] = j;
		SplTransTbl[255] = 0;
	}*/
	LoadFileWithMem("PlrGFX\\SNone.TRN", SkillTrns[RSPLTYPE_ABILITY]);
	LoadFileWithMem("PlrGFX\\SBlue.TRN", SkillTrns[RSPLTYPE_SPELL]);
	LoadFileWithMem("PlrGFX\\SRed.TRN", SkillTrns[RSPLTYPE_SCROLL]);
	LoadFileWithMem("PlrGFX\\SOrange.TRN", SkillTrns[RSPLTYPE_CHARGES]);
	LoadFileWithMem("PlrGFX\\SGray.TRN", SkillTrns[RSPLTYPE_INVALID]);
#ifdef HELLFIRE
	LoadFileWithMem("PlrGFX\\Coral.TRN", SkillTrns[NUM_RSPLTYPES]);
#endif
	gnNumActiveWindows = 0;
	gbInvflag = false;
	gbTalkflag = false;
	guTeamInviteRec = 0;
	guTeamInviteSent = 0;
	guTeamMute = 0;
	sgszTalkMsg[0] = '\0';
	if (!IsLocalGame) {
		assert(pTalkPnlCel == NULL);
		pTalkPnlCel = CelLoadImage("CtrlPan\\TalkPnl.CEL", TALK_PNL_WIDTH);
	}
	gbLvlbtndown = false;
	assert(pPanelButtonCels == NULL);
	pPanelButtonCels = CelLoadImage("CtrlPan\\Menu.CEL", MENUBTN_WIDTH);
	for (i = 0; i < lengthof(gabPanbtn); i++)
		gabPanbtn[i] = false;
	numpanbtns = IsLocalGame ? NUM_PANBTNS - 2 : NUM_PANBTNS;
	assert(pChrButtonCels == NULL);
	pChrButtonCels = CelLoadImage("Data\\CharBut.CEL", CHRBTN_WIDTH);
	for (i = 0; i < lengthof(_gabChrbtn); i++)
		_gabChrbtn[i] = false;
	gbChrbtnactive = false;
	assert(pTextBoxCels == NULL);
	pTextBoxCels = CelLoadImage("Data\\TextBox.CEL", LTPANEL_WIDTH);
	assert(pSTextBoxCels == NULL);
	pSTextBoxCels = CelLoadImage("Data\\TextBox2.CEL", STPANEL_WIDTH);
	assert(pSTextSlidCels == NULL);
	pSTextSlidCels = CelLoadImage("Data\\TextSlid.CEL", SMALL_SCROLL_WIDTH);
	assert(pDurIconCels == NULL);
	pDurIconCels = CelLoadImage("Items\\DurIcons.CEL", DURICON_WIDTH);
	infostr[0] = '\0';
	gbRedrawFlags |= REDRAW_HP_FLASK | REDRAW_MANA_FLASK | REDRAW_SPEED_BAR;
	gbLvlUp = false;
	gbSkillListFlag = false;
	assert(pSpellBkCel == NULL);
	pSpellBkCel = CelLoadImage("Data\\SpellBk.CEL", SPANEL_WIDTH);
	assert(pSBkIconCels == NULL);
	pSBkIconCels = CelLoadImage("Data\\SpellI2.CEL", SBOOK_CELWIDTH);
	guBooktab = 0;
	SpellPages[0][0] = Abilities[myplr._pClass];
	assert(pGoldDropCel == NULL);
	pGoldDropCel = CelLoadImage("CtrlPan\\Golddrop.cel", GOLDDROP_WIDTH);
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
	x = PANEL_X + PanBtnPos[i][0];
	pb = gabPanbtn[PANBTN_MAINMENU];
	CelDraw(x, PANEL_Y + PANEL_HEIGHT - PanBtnPos[i][1] + MENUBTN_HEIGHT - 1, pPanelButtonCels, pb ? 4 : 3);
	if (!pb)
		return;
	for (i = 1; i < numpanbtns; i++) {
		y = PANEL_Y + PANEL_HEIGHT - PanBtnPos[i][1];
		pb = gabPanbtn[i];
		CelDraw(x, y + MENUBTN_HEIGHT - 1, pPanelButtonCels, 2);
		// print the text of the button
		PrintString(x + 3, y + (MENUBTN_HEIGHT + SMALL_FONT_HEIGHT) / 2, x + MENUBTN_WIDTH - 1, PanBtnTxt[i], true, pb ? COL_GOLD : COL_WHITE, FONT_KERN_SMALL);
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

#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
	StoreSpellCoords();

	_gbMoveCursor = 0;
	if (sgbControllerActive)
		_gbMoveCursor = altSkill ? 1 : 2;
#endif
}

void HandleSkillBtn(bool altSkill)
{
	if (!gbSkillListFlag) {
		ClearPanels();
		gamemenu_off();
		DoSkillList(altSkill);
	} else {
		gbSkillListFlag = false;
	}
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
	return POS_IN_RECT(MouseX, MouseY,
		LVLUP_LEFT, PANEL_BOTTOM - LVLUP_OFFSET - CHRBTN_HEIGHT,
		CHRBTN_WIDTH, CHRBTN_HEIGHT);
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
		if (POS_IN_RECT(mx, my,
			PANEL_LEFT + PanBtnPos[i][0],  PANEL_BOTTOM - PanBtnPos[i][1],
			MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
			control_set_button_down(i);
			return true;
		}
	}
	if (POS_IN_RECT(mx, my,
		PANEL_LEFT + PANEL_WIDTH - SPLICONLENGTH,  PANEL_BOTTOM - 2 * SPLICONLENGTH,
		SPLICONLENGTH + 1, 2 * SPLICONLENGTH + 1)) {
		HandleSkillBtn(my < PANEL_BOTTOM - SPLICONLENGTH);
		return true;
	}
	if (gbLvlUp && InLvlUpRect())
		gbLvlbtndown = true;
	return gbLvlbtndown;
}

void DoLimitedPanBtn()
{
	if (POS_IN_RECT(MouseX, MouseY,
		PANEL_LEFT + PanBtnPos[PANBTN_MAINMENU][0],  PANEL_BOTTOM - PanBtnPos[PANBTN_MAINMENU][1],
		MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
		control_set_button_down(PANBTN_MAINMENU);
	} else if (gabPanbtn[PANBTN_MAINMENU] && !IsLocalGame) {
		if (POS_IN_RECT(MouseX, MouseY,
			PANEL_LEFT + PanBtnPos[PANBTN_SENDMSG][0],  PANEL_BOTTOM - PanBtnPos[PANBTN_SENDMSG][1],
			MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
			control_set_button_down(PANBTN_SENDMSG);
		}
	}
}

void HandlePanBtn(int i)
{
	if (stextflag != STORE_NONE)
		return;

	switch (i) {
	case PANBTN_MAINMENU:
		gbQtextflag = false;
		break;
	case PANBTN_OPTIONS:
		gamemenu_on();
		return;
	case PANBTN_CHARINFO:
		gbSkillListFlag = false;
		gbLvlUp = false;
		if (ToggleWindow(WND_CHAR)) {
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
			if (sgbControllerActive)
				FocusOnCharInfo();
#endif
		}
		break;
	case PANBTN_INVENTORY:
		gbSkillListFlag = false;
		gbInvflag = ToggleWindow(WND_INV);
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (gbInvflag && sgbControllerActive)
			FocusOnInventory();
#endif
		break;
	case PANBTN_SPELLBOOK:
		gbSkillListFlag = false;
		ToggleWindow(WND_BOOK);
		break;
	case PANBTN_QLOG:
		gbSkillListFlag = false;
		if (ToggleWindow(WND_QUEST))
			StartQuestlog();
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
		gbSkillListFlag = false;
		ToggleWindow(WND_TEAM);
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
		if (!POS_IN_RECT(MouseX, MouseY,
			PANEL_LEFT + PanBtnPos[i][0],  PANEL_BOTTOM - PanBtnPos[i][1],
			MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
			continue;
		}

		HandlePanBtn(i);

		gabPanbtn[PANBTN_MAINMENU] = false;
		//gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
	}
}

void FreeControlPan()
{
	MemFreeDbg(pFlaskCels);
	MemFreeDbg(pChrPanelCel);
	MemFreeDbg(pSpellCels);
	MemFreeDbg(pPanelButtonCels);
	MemFreeDbg(pTalkPnlCel);
	MemFreeDbg(pChrButtonCels);
	MemFreeDbg(pSTextBoxCels);
	MemFreeDbg(pSTextSlidCels);
	MemFreeDbg(pTextBoxCels);
	MemFreeDbg(pDurIconCels);
	MemFreeDbg(pSpellBkCel);
	MemFreeDbg(pSBkIconCels);
	MemFreeDbg(pGoldDropCel);
}

#define ADD_PlrStringXY(x, y, endX, pszStr, col) PrintString(x + SCREEN_X, y + SCREEN_Y, endX + SCREEN_X, pszStr, true, col, FONT_KERN_SMALL)

void DrawChr()
{
	PlayerStruct* p;
	BYTE col;
	char chrstr[64];
	int pc, val, mindam, maxdam;

	p = &myplr;
	pc = p->_pClass;

	CelDraw(SCREEN_X, SCREEN_Y + SPANEL_HEIGHT - 1, pChrPanelCel, 1);
	ADD_PlrStringXY(5, 19, 144, p->_pName, COL_WHITE);

	ADD_PlrStringXY(153, 19, 292, ClassStrTbl[pc], COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%d", p->_pLevel);
	ADD_PlrStringXY(53, 46, 96, chrstr, COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%d", p->_pExperience);
	ADD_PlrStringXY(200, 46, 292, chrstr, COL_WHITE);

	if (p->_pLevel == MAXCHARLEVEL) {
		copy_cstr(chrstr, "None");
		col = COL_GOLD;
	} else {
		snprintf(chrstr, sizeof(chrstr), "%d", p->_pNextExper);
		col = COL_WHITE;
	}
	ADD_PlrStringXY(200, 71, 292, chrstr, col);

	snprintf(chrstr, sizeof(chrstr), "%d", p->_pGold);
	ADD_PlrStringXY(221, 97, 292, chrstr, COL_WHITE);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseStr);
	ADD_PlrStringXY(88, 119, 125, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseMag);
	ADD_PlrStringXY(88, 147, 125, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseDex);
	ADD_PlrStringXY(88, 175, 125, chrstr, col);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseVit);
	ADD_PlrStringXY(88, 203, 125, chrstr, col);

	if (p->_pStatPts > 0) {
		snprintf(chrstr, sizeof(chrstr), "%d", p->_pStatPts);
		ADD_PlrStringXY(88, 231, 125, chrstr, COL_RED);
		CelDraw(ChrBtnsRect[ATTRIB_STR].x + SCREEN_X, ChrBtnsRect[ATTRIB_STR].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtonCels, _gabChrbtn[ATTRIB_STR] ? 3 : 2);
		CelDraw(ChrBtnsRect[ATTRIB_MAG].x + SCREEN_X, ChrBtnsRect[ATTRIB_MAG].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtonCels, _gabChrbtn[ATTRIB_MAG] ? 5 : 4);
		CelDraw(ChrBtnsRect[ATTRIB_DEX].x + SCREEN_X, ChrBtnsRect[ATTRIB_DEX].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtonCels, _gabChrbtn[ATTRIB_DEX] ? 7 : 6);
		CelDraw(ChrBtnsRect[ATTRIB_VIT].x + SCREEN_X, ChrBtnsRect[ATTRIB_VIT].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtonCels, _gabChrbtn[ATTRIB_VIT] ? 9 : 8);
	}

	if (p->_pHasUnidItem)
		return;

	if (p->_pStatPts <= 0) {
		val = p->_pStrength;
		col = COL_WHITE;
		if (val > p->_pBaseStr)
			col = COL_BLUE;
		else if (val < p->_pBaseStr)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		ADD_PlrStringXY(135, 119, 172, chrstr, col);

		val = p->_pMagic;
		col = COL_WHITE;
		if (val > p->_pBaseMag)
			col = COL_BLUE;
		else if (val < p->_pBaseMag)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		ADD_PlrStringXY(135, 147, 172, chrstr, col);

		val = p->_pDexterity;
		col = COL_WHITE;
		if (val > p->_pBaseDex)
			col = COL_BLUE;
		else if (val < p->_pBaseDex)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		ADD_PlrStringXY(135, 175, 172, chrstr, col);

		val = p->_pVitality;
		col = COL_WHITE;
		if (val > p->_pBaseVit)
			col = COL_BLUE;
		else if (val < p->_pBaseVit)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		ADD_PlrStringXY(135, 203, 172, chrstr, col);
	}

	snprintf(chrstr, sizeof(chrstr), "%d/%d", p->_pIAC, p->_pIEvasion);
	// instead of (242;291) x-limits, (239;294) are used to make sure the values are displayed
	PrintString(239 + SCREEN_X, 122 + SCREEN_Y, 294 + SCREEN_X, chrstr, true, COL_WHITE, -1);

	snprintf(chrstr, sizeof(chrstr), "%d/%d", p->_pIBlockChance, p->_pICritChance / 2);
	// instead of (242;291) x-limits, (241;292) are used to make sure the values are displayed
	PrintString(241 + SCREEN_X, 150 + SCREEN_Y, 292 + SCREEN_X, chrstr, true, COL_WHITE, -1);

	val = p->_pIHitChance;
	col = COL_WHITE;
	if (p->_pIBaseHitBonus == IBONUS_POSITIVE)
		col = COL_BLUE;
	else if (p->_pIBaseHitBonus == IBONUS_NEGATIVE)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	ADD_PlrStringXY(242, 178, 291, chrstr, col);

	col = COL_WHITE;
	mindam = (p->_pIFMinDam + p->_pILMinDam + p->_pIMMinDam + p->_pIAMinDam) >> 6;
	maxdam = (p->_pIFMaxDam + p->_pILMaxDam + p->_pIMMaxDam + p->_pIAMaxDam) >> 6;
	if (maxdam != 0)
		col = COL_BLUE;
	mindam += (p->_pISlMinDam + p->_pIBlMinDam + p->_pIPcMinDam) >> (6 + 1); // +1 is a temporary(?) adjustment for backwards compatibility
	maxdam += (p->_pISlMaxDam + p->_pIBlMaxDam + p->_pIPcMaxDam) >> (6 + 1);
	snprintf(chrstr, sizeof(chrstr), "%d-%d", mindam, maxdam);
	// instead of (242;291) x-limits, (240;293) are used to make sure the values are displayed
	PrintString(240 + SCREEN_X, 206 + SCREEN_Y, 293 + SCREEN_X, chrstr, true, col, mindam < 100 ? 0 : -1);

	val = p->_pMagResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(185, 254, 234, chrstr, col);

	val = p->_pFireResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(242, 254, 291, chrstr, col);

	val = p->_pLghtResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(185, 289, 234, chrstr, col);

	val = p->_pAcidResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(242, 289, 291, chrstr, col);

	val = p->_pMaxHP;
	col = val <= p->_pMaxHPBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%d", val >> 6);
	ADD_PlrStringXY(87, 260, 126, chrstr, col); // 88, 125 -> 87, 126 otherwise '1000' is truncated
	if (p->_pHitPoints != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pHitPoints >> 6);
	ADD_PlrStringXY(134, 260, 173, chrstr, col); // 135, 172 -> 134, 173 otherwise '1000' is truncated

	val = p->_pMaxMana;
	col = val <= p->_pMaxManaBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%d", val >> 6);
	ADD_PlrStringXY(87, 288, 126, chrstr, col); // 88, 125 -> 87, 126 otherwise '1000' is truncated
	if (p->_pMana != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pMana >> 6);
	ADD_PlrStringXY(134, 288, 173, chrstr, col); // 135, 172 -> 134, 173 otherwise '1000' is truncated
}

void DrawLevelUpIcon()
{
	ADD_PlrStringXY(LVLUP_LEFT - 38, PANEL_TOP + PANEL_HEIGHT - (LVLUP_OFFSET - 20), LVLUP_LEFT - 38 + 120, "Level Up", COL_WHITE);
	CelDraw(SCREEN_X + LVLUP_LEFT, PANEL_Y + PANEL_HEIGHT - LVLUP_OFFSET, pChrButtonCels, gbLvlbtndown ? 3 : 2);
}

static int DrawTooltip2(const char* text1, const char* text2, int x, int y, BYTE col)
{
	int width, result = 0;
	BYTE* dst;
	const int border = 4, height = 26;
	int w1 = GetSmallStringWidth(text1);
	int w2 = GetSmallStringWidth(text2);

	width = std::max(w1, w2) + 2 * border;

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
	PrintGameStr(SCREEN_X + x + border + w1, SCREEN_Y + y + height - 14, text1, col);
	PrintGameStr(SCREEN_X + x + border + w2, SCREEN_Y + y + height - 3, text2, COL_WHITE);
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
static void GetMousePos(int x, int y, int* outx, int* outy)
{
	int px, py;

	x -= ViewX;
	y -= ViewY;
	
	px = 0;
	py = 0;
	SHIFT_GRID(px, py, -y, x);

	px *= TILE_WIDTH / 2;
	py *= TILE_HEIGHT / 2;

	if (gbZoomInFlag) {
		px <<= 1;
		py <<= 1;
	}

	px += SCREEN_WIDTH / 2;
	py += VIEWPORT_HEIGHT / 2;

	*outx = px;
	*outy = py;
}

static BYTE DrawItemColor(ItemStruct* is)
{
	if (is->_iMagical == ITEM_QUALITY_NORMAL)
		return COL_WHITE;
	return is->_iMagical == ITEM_QUALITY_UNIQUE ? COL_GOLD : COL_BLUE;
}

static void GetItemInfo(ItemStruct* is)
{
	infoclr = DrawItemColor(is);
	if (is->_itype != ITYPE_GOLD) {
		copy_str(infostr, is->_iName);
	} else {
		snprintf(infostr, sizeof(infostr), "%d gold %s", is->_ivalue, get_pieces_str(is->_ivalue));
	}
}
#define TOOLTIP_OFFSET		32
#define TOOLTIP_HEIGHT		16
#define TOOLTIP2_HEIGHT		26
#define HEALTHBAR_HEIGHT	4
#define HEALTHBAR_WIDTH		66
static int DrawTooltip(const char* text, int x, int y, BYTE col)
{
	int width, result = 0;
	BYTE* dst;
	const int border = 4;

	width = GetSmallStringWidth(text) + 2 * border;

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
	for (int i = 0; i < TOOLTIP_HEIGHT; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_GRAY + 8, width);

	// draw background
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < TOOLTIP_HEIGHT - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_ORANGE + 14, width - 2);

	// print the info
	PrintGameStr(SCREEN_X + x + border, SCREEN_Y + y + TOOLTIP_HEIGHT - 3, text, col);
	return result;
}

static void DrawHealthBar(int hp, int maxhp, int x, int y)
{
	BYTE* dst;
	int h, dhp, w, dw;

	if (maxhp <= 0)
		return;

	if (y < 0)
		return;
	x -= HEALTHBAR_WIDTH / 2;
	if (x < 0)
		x = 0;
	else if (x > SCREEN_WIDTH - HEALTHBAR_WIDTH)
		x = SCREEN_WIDTH - HEALTHBAR_WIDTH;

	// draw gray border
	dst = &gpBuffer[SCREENXY(x, y)];
	for (int i = 0; i < HEALTHBAR_HEIGHT; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_GRAY + 5, HEALTHBAR_WIDTH);

	// draw the bar
	//width = (HEALTHBAR_WIDTH - 2) * hp / maxhp;
	dhp = (maxhp + 7) >> 3;
	dw = ((HEALTHBAR_WIDTH - 2) >> 3);
	for (w = 0, h = 0; h < hp; h += dhp, w += dw) {
	}
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < HEALTHBAR_HEIGHT - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_RED + 9, w);
}

static void DrawTrigInfo()
{
	int xx, yy, qn;

	if (pcurstrig >= MAXTRIGGERS + 1) {
		// portal
		MissileStruct* mis = &missile[pcurstrig - (MAXTRIGGERS + 1)];
		if (mis->_miType == MIS_TOWN) {
			copy_cstr(infostr, "Town Portal");
			snprintf(tempstr, sizeof(tempstr), "(%s)", players[mis->_miSource]._pName);
			GetMousePos(cursmx, cursmy, &xx, &yy);
			yy -= TILE_HEIGHT * 2 + TOOLTIP_OFFSET;
			DrawTooltip2(infostr, tempstr, xx, yy, COL_WHITE);
		} else {
			if (!currLvl._dSetLvl) {
				copy_cstr(infostr, "Portal to The Unholy Altar");
			} else {
				copy_cstr(infostr, "Portal back to hell");
			}
			GetMousePos(cursmx, cursmy, &xx, &yy);
			yy -= TILE_HEIGHT * 2 + TOOLTIP_OFFSET;
			DrawTooltip(infostr, xx, yy, COL_WHITE);
		}
		return;
	} else {
		// standard trigger
		switch (trigs[pcurstrig]._tmsg) {
		case DVL_DWM_NEXTLVL:
			snprintf(infostr, sizeof(infostr), "Down to %s", AllLevels[currLvl._dLevelIdx + 1].dLevelName);
			break;
		case DVL_DWM_PREVLVL:
			//if (currLvl._dLevelIdx == DLV_CATHEDRAL1)
			//	copy_cstr(infostr, "Up to town");
			//else
				snprintf(infostr, sizeof(infostr), "Up to %s", AllLevels[currLvl._dLevelIdx - 1].dLevelName);
			break;
		case DVL_DWM_RTNLVL:
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
		case DVL_DWM_SETLVL:
			// quest trigger
			// TODO: use dLevelName?
			switch (trigs[pcurstrig]._tlvl) {
			case SL_SKELKING:
				copy_cstr(infostr, "To King Leoric's Tomb");
				break;
			case SL_BONECHAMB:
				copy_cstr(infostr, "To The Chamber of Bone");
				break;
			/*case SL_MAZE:
				copy_cstr(infostr, "To Maze");
				break;*/
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
		case DVL_DWM_TWARPDN:
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
		case DVL_DWM_TWARPUP:
			copy_cstr(infostr, "Up to town");
			break;
		default:
			ASSUME_UNREACHABLE
		}
	}

	GetMousePos(cursmx, cursmy, &xx, &yy);
	yy -= TILE_HEIGHT + TOOLTIP_OFFSET;
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
		yy -= TOOLTIP_OFFSET;
		DrawTooltip(infostr, xx, yy, infoclr);
	} else if (pcursobj != OBJ_NONE) {
		GetObjectStr(pcursobj);
		ObjectStruct* os = &objects[pcursobj];
		x = os->_ox;
		y = os->_oy;
		GetMousePos(x, y, &xx, &yy);
		yy -= TILE_HEIGHT + TOOLTIP_OFFSET;
		DrawTooltip(infostr, xx, yy, infoclr);
	} else if (pcursmonst != MON_NONE) {
		MonsterStruct* mon = &monsters[pcursmonst];
		x = mon->_mx;
		y = mon->_my;
		col = COL_WHITE;
		strcpy(infostr, mon->mName); // TNR_NAME or a monster's name
		if (mon->_uniqtype != 0) {
			col = COL_GOLD;
		}
		GetMousePos(x, y, &xx, &yy);
		yy -= ((mon->_mSelFlag & 6) ? TILE_HEIGHT * 2 : TILE_HEIGHT) + TOOLTIP_OFFSET;
		xx += DrawTooltip(infostr, xx, yy, col);
		DrawHealthBar(mon->_mhitpoints, mon->_mmaxhp, xx, yy + TOOLTIP_HEIGHT - HEALTHBAR_HEIGHT / 2);
	} else if (pcursplr != PLR_NONE) {
		PlayerStruct* p = &players[pcursplr];
		x = p->_px;
		y = p->_py;
		GetMousePos(x, y, &xx, &yy);
		yy -= TILE_HEIGHT * 2 + TOOLTIP_OFFSET;
		snprintf(infostr, sizeof(infostr), p->_pManaShield == 0 ? "%s(%d)" : "%s(%d)*", ClassStrTbl[p->_pClass], p->_pLevel);
		xx += DrawTooltip2(p->_pName, infostr, xx, yy, COL_GOLD);
		DrawHealthBar(p->_pHitPoints, p->_pMaxHP, xx, yy + TOOLTIP2_HEIGHT - HEALTHBAR_HEIGHT / 2);
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
			fmt = SPELL_RUNE(currSkill) ? "Rune of %s" : "Scroll of %s";
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
		DrawTooltip(infostr, MouseX, MouseY - (SPLICONLENGTH / 4 + TOOLTIP_OFFSET), COL_WHITE);
	} else if (pcursinvitem != INVITEM_NONE) {
		DrawInvItemDetails();
	} else if (pcurstrig != -1) {
		DrawTrigInfo();
	} else if (pcurs >= CURSOR_FIRSTITEM) {
		GetItemInfo(&myplr._pHoldItem);
		DrawTooltip(infostr, MouseX + cursW / 2, MouseY - TOOLTIP_OFFSET, infoclr);
	}
}


bool CheckChrBtns()
{
	int i;

	if (myplr._pStatPts != 0 && !gbChrbtnactive) {
		for (i = 0; i < lengthof(ChrBtnsRect); i++) {
			if (!POS_IN_RECT(MouseX, MouseY,
				ChrBtnsRect[i].x,  ChrBtnsRect[i].y,
				ChrBtnsRect[i].w, ChrBtnsRect[i].h))
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
			if (POS_IN_RECT(MouseX, MouseY,
				ChrBtnsRect[i].x,  ChrBtnsRect[i].y,
				ChrBtnsRect[i].w, ChrBtnsRect[i].h)) {
				switch (i) {
				case 0:
					NetSendCmd(CMD_ADDSTR);
					break;
				case 1:
					NetSendCmd(CMD_ADDMAG);
					break;
				case 2:
					NetSendCmd(CMD_ADDDEX);
					break;
				case 3:
					NetSendCmd(CMD_ADDVIT);
					break;
				default:
					ASSUME_UNREACHABLE
					break;
				}
			}
		}
	}
}

/**
 * @brief Draw a large text box with transparent background.
 *  used as background to quest dialog window and in stores.
 */
void DrawTextBox()
{
	int x, y;

	x = LTPANEL_X;
	y = LTPANEL_Y;

	CelDraw(x, y + TPANEL_HEIGHT, pTextBoxCels, 1);
	trans_rect(x + TPANEL_BORDER, y + TPANEL_BORDER, LTPANEL_WIDTH - 2 * TPANEL_BORDER, TPANEL_HEIGHT - 2 * TPANEL_BORDER);
}

/**
 * @brief Draw a small text box with transparent background.
 *  used as background to items and in stores.
 * @param x: the starting x-coordinate of the text box
 * @param y: the starting y-coordinate of the text box
 */
void DrawSTextBox(int x, int y)
{
	CelDraw(x, y + TPANEL_HEIGHT, pSTextBoxCels, 1);
	trans_rect(x + TPANEL_BORDER, y + TPANEL_BORDER, STPANEL_WIDTH - 2 * TPANEL_BORDER, TPANEL_HEIGHT - 2 * TPANEL_BORDER);
}

/**
 * @brief Draw a separator line into the text box.
 *  used with items and in stores.
 * @param x: the starting x-coordinate of the text box
 * @param y: the starting y-coordinate of the text box
 * @param dy: the distance from the top of the box where the separator should be drawn
 * @param widePanel: true if large text box is used, false if small text box
 */
void DrawTextBoxSLine(int x, int y, int dy, bool widePanel)
{
	int sxy, dxy, width, length;
	
	width = BUFFER_WIDTH;
	sxy = x + 2 + width * (y + 1);
	dxy = x + 2 + width * (y + dy);
	length = widePanel ? LTPANEL_WIDTH - 4 : STPANEL_WIDTH - 4;

	/// ASSERT: assert(gpBuffer != NULL);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[sxy];
	dst = &gpBuffer[dxy];

	for (i = 0; i < TPANEL_BORDER; i++, src += width, dst += width)
		memcpy(dst, src, length);
}

static int DrawDurIcon4Item(ItemStruct* pItem, int x)
{
	int c;

	if (pItem->_iDurability > 5)
		return x;
	switch (pItem->_itype) {
	case ITYPE_NONE:
		return x;
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
	case ITYPE_SHIELD:
		c = 1;
		break;
	case ITYPE_HELM:
		c = 4;
		break;
	case ITYPE_LARMOR:
	case ITYPE_MARMOR:
	case ITYPE_HARMOR:
		c = 3;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (pItem->_iDurability > 2)
		c += 8;
	CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - 8, pDurIconCels, c);
	return x - DURICON_WIDTH - 8;
}

void DrawDurIcon()
{
	ItemStruct* inv;
	int x;

	x = SCREEN_X + SCREEN_WIDTH - (SPLICONLENGTH + 92 + DURICON_WIDTH);

	inv = myplr._pInvBody;
	x = DrawDurIcon4Item(&inv[INVLOC_HEAD], x);
	x = DrawDurIcon4Item(&inv[INVLOC_CHEST], x);
	x = DrawDurIcon4Item(&inv[INVLOC_HAND_LEFT], x);
	DrawDurIcon4Item(&inv[INVLOC_HAND_RIGHT], x);
}

static BYTE GetSBookTrans(int sn)
{
	PlayerStruct* p;
	BYTE st;

	p = &myplr;
	if (p->_pAblSkills & SPELL_MASK(sn)) { /// BUGFIX: missing (uint64_t) (fixed)
		st = RSPLTYPE_ABILITY;
	} else if (p->_pISpells & SPELL_MASK(sn)) {
		st = RSPLTYPE_CHARGES;
	} else if (p->_pScrlSkills & SPELL_MASK(sn)) {
		st = RSPLTYPE_SCROLL;
	} else if (CheckSpell(mypnum, sn)) {
		st = RSPLTYPE_SPELL;
	} else {
		st = RSPLTYPE_INVALID;
	}
	return st;
}

void DrawSpellBook()
{
	ItemStruct* pi;
	int pnum, i, sn, mana, lvl, sx, yp, offset;
	BYTE st;
	uint64_t spl;

	// back panel
	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pSpellBkCel, 1);
	// selected page
	snprintf(tempstr, sizeof(tempstr), "%d.", guBooktab + 1);
	PrintString(RIGHT_PANEL_X + 2, SCREEN_Y + SPANEL_HEIGHT - 7, RIGHT_PANEL_X + SPANEL_WIDTH, tempstr, true, COL_WHITE, 0);

	currSkill = SPL_INVALID;

	pnum = mypnum;
	spl = plr._pMemSkills | plr._pISpells | plr._pAblSkills | plr._pScrlSkills;

	yp = SCREEN_Y + SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx = RIGHT_PANEL_X + SBOOK_CELBORDER;
	for (i = 0; i < lengthof(SpellPages[guBooktab]); i++) {
		sn = SpellPages[guBooktab][i];
		if (sn != SPL_INVALID && (spl & SPELL_MASK(sn))) {
			st = GetSBookTrans(sn);
			if (POS_IN_RECT(MouseX, MouseY,
				sx - BORDER_LEFT, yp - BORDER_TOP - SBOOK_CELHEIGHT,
				SBOOK_CELWIDTH, SBOOK_CELHEIGHT)) {
				currSkill = sn;
				currSkillType = st;
			}
			lvl = plr._pSkillLvl[sn];
			// assert(lvl >= 0);
			mana = 0;
			switch (st) {
			case RSPLTYPE_ABILITY:
				copy_cstr(tempstr, "Skill");
				// lvl = -1; // SPLLVL_UNDEF
				break;
			case RSPLTYPE_SCROLL:
				if (SPELL_RUNE(sn)) {
					copy_cstr(tempstr, "Rune");
				} else {
					copy_cstr(tempstr, "Scroll");
				}
				break;
			case RSPLTYPE_CHARGES:
				pi = &plr._pInvBody[INVLOC_HAND_LEFT];
				if (pi->_iMagical == ITEM_QUALITY_NORMAL || pi->_iIdentified) {
					snprintf(tempstr, sizeof(tempstr), "Charges: %d/%d", pi->_iCharges, pi->_iMaxCharges);
				} else {
					copy_cstr(tempstr, "Charge");
					lvl = -1; // SPLLVL_UNDEF
				}
				break;
			case RSPLTYPE_SPELL:
			case RSPLTYPE_INVALID:
				if (plr._pHasUnidItem) {
					copy_cstr(tempstr, "Spell");
					lvl = -1; // SPLLVL_UNDEF
					break;
				}
				if (lvl > 0) {
					snprintf(tempstr, sizeof(tempstr), "Spell Level %d", lvl);
				} else {
					copy_cstr(tempstr, "Spell Level 0 - Unusable");
				}
				mana = GetManaAmount(pnum, sn) >> 6;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			int min, max;
			if (lvl != -1) // SPLLVL_UNDEF
				GetDamageAmt(sn, lvl, &min, &max);
			else
				min = -1;
			offset = mana == 0 && min == -1 ? 5 : 0;
			PrintString(sx + SBOOK_LINE_TAB, yp - 23 + offset, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, spelldata[sn].sNameText, false, COL_WHITE, FONT_KERN_SMALL);
			PrintString(sx + SBOOK_LINE_TAB, yp - 12 + offset, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, false, COL_WHITE, FONT_KERN_SMALL);

			if (offset == 0) {
				if (mana != 0)
					cat_str(tempstr, offset, "Mana: %d  ", mana);
				if (min != -1)
					cat_str(tempstr, offset, "Dam: %d-%d", min, max);
				PrintString(sx + SBOOK_LINE_TAB, yp - 1, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, false, COL_WHITE, FONT_KERN_SMALL);
			}

			if ((spelldata[sn].sUseFlags & plr._pSkillFlags) != spelldata[sn].sUseFlags)
				st = RSPLTYPE_INVALID;
			CelDrawLight(sx, yp, pSBkIconCels, spelldata[sn].sIcon, SkillTrns[GetSpellTrans(st, sn)]);
			// TODO: differenciate between Atk/Move skill ? Add icon for primary skills?
			if ((sn == plr._pAltAtkSkill && st == plr._pAltAtkSkillType)
			 || (sn == plr._pAltMoveSkill && st == plr._pAltMoveSkillType)) {
				CelDrawLight(sx, yp, pSBkIconCels, SPLICONLAST, SkillTrns[RSPLTYPE_ABILITY]);
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

const char* get_pieces_str(int nGold)
{
	return nGold != 1 ? "pieces" : "piece";
}

void DrawGoldSplit(int amount)
{
	int screen_x;

	screen_x = 0;
	CelDraw(351 + SCREEN_X, 178 + SCREEN_Y, pGoldDropCel, 1);
	snprintf(tempstr, sizeof(tempstr), "You have %d gold", initialDropGoldValue);
	ADD_PlrStringXY(351 + 15, 87, 351 + GOLDDROP_WIDTH - 15, tempstr, COL_GOLD);
	snprintf(tempstr, sizeof(tempstr), "%s.  How many do", get_pieces_str(initialDropGoldValue));
	ADD_PlrStringXY(351 + 15, 103, 351 + GOLDDROP_WIDTH - 15, tempstr, COL_GOLD);
	ADD_PlrStringXY(351 + 15, 121, 351 + GOLDDROP_WIDTH - 15, "you want to remove?", COL_GOLD);
	if (amount > 0) {
		snprintf(tempstr, sizeof(tempstr), "%d", amount);
		PrintGameStr(388 + SCREEN_X, 140 + SCREEN_Y, tempstr, COL_WHITE);
		screen_x += GetSmallStringWidth(tempstr);
		screen_x += 452;
	} else {
		screen_x = 450;
	}
	DrawSingleSmallPentSpn(screen_x, 140 + SCREEN_Y);
}

static void control_remove_gold()
{
	int gi;

	assert(initialDropGoldIndex <= INVITEM_INV_LAST && initialDropGoldIndex >= INVITEM_INV_FIRST);
	static_assert((int)INVITEM_INV_LAST - (int)INVITEM_INV_FIRST < UCHAR_MAX, "control_remove_gold sends inv item index in BYTE field.");
	gi = initialDropGoldIndex - INVITEM_INV_FIRST;
	static_assert(GOLD_MAX_LIMIT <= UINT16_MAX, "control_remove_gold send gold pile value using WORD.");
	NetSendCmdParamBW(CMD_SPLITPLRGOLD, gi, dropGoldValue);
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
	PrintString(x, y - 1, x + SBOOK_LINE_LENGTH, label, false, color, FONT_KERN_SMALL);
}

static bool PlrHasTeam()
{
	int pnum;

	for (pnum = 0; pnum < MAX_PLRS; pnum++)
		if (pnum != mypnum && plr._pTeam == myplr._pTeam && plr._pActive)
			return true;
	return false;
}

#define TBOOKTABS		((MAX_PLRS + NUM_BOOK_ENTRIES - 1) / NUM_BOOK_ENTRIES)
void DrawTeamBook()
{
	int i, pnum, sx, yp;
	bool hasTeam;
	BYTE st;

	// back panel
	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pSpellBkCel, 1);
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
		if (!plr._pActive)
			continue;
		// name
		PrintString(sx + SBOOK_LINE_TAB, yp - 25, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, plr._pName, false, COL_WHITE, 0);
		// class(level) - team
		static_assert(MAXCHARLEVEL < 100, "Level must fit to the TeamBook.");
		snprintf(tempstr, sizeof(tempstr), "%s (lvl:%2d) %c", ClassStrTbl[plr._pClass], plr._pLevel, 'a' + plr._pTeam);
		PrintString(sx + SBOOK_LINE_TAB, yp - 13, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, false, COL_WHITE, FONT_KERN_SMALL);

		// mute
		if (pnum != mypnum) {
			DrawTeamButton(sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH - (TBOOK_BTN_WIDTH - 8), yp - 24, TBOOK_BTN_WIDTH,
				(guTeamMute & (1 << pnum)) != 0, "mute", 10);
		}

		// drop/leave
		if (hasTeam && (pnum == mypnum || plr._pTeam == mypnum)) {
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
		if (pnum != mypnum && plr._pTeam != myplr._pTeam && myplr._pTeam == mypnum) {
			unsigned invited = (guTeamInviteSent & (1 << pnum));
			DrawTeamButton(sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH - (TBOOK_BTN_WIDTH - 8), yp, TBOOK_BTN_WIDTH, false,
				!invited ? "invite" : "cancel", !invited ? 7 : 2);
		}

		// icon
		st = plr._pDunLevel == DLV_TOWN ? RSPLTYPE_ABILITY : (plr._pmode == PM_DEATH ? RSPLTYPE_INVALID : RSPLTYPE_SPELL);
		CelDrawLight(sx, yp, pSBkIconCels, ClassIconTbl[plr._pClass], SkillTrns[st]);

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
		if (pnum >= MAX_PLRS || !plr._pActive)
			return;
		if (dx <= SBOOK_CELWIDTH) {
			// clicked on the icon
			if (!gbTalkflag)
				control_type_message();
			if (!shift) {
				snprintf(sgszTalkMsg, sizeof(sgszTalkMsg), "/p%d ", pnum);
			} else {
				snprintf(sgszTalkMsg, sizeof(sgszTalkMsg), "/t%d ", plr._pTeam);
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
				if (PlrHasTeam() && (pnum == mypnum || plr._pTeam == mypnum))
					NetSendCmdBParam1(CMD_KICK_PLR, pnum);
			} else /*if (dy == 2)*/ {
				// invite/cancel
				if (pnum != mypnum && plr._pTeam != myplr._pTeam && myplr._pTeam == mypnum) {
					if (guTeamInviteSent & (1 << pnum)) {
						NetSendCmdBParam1(CMD_REV_INVITE, pnum);
					} else {
						NetSendCmdBParam1(CMD_INVITE, pnum);
					}
					guTeamInviteSent ^= (1 << pnum);
				}
			}
		} else if (dy >= (2 * (SBOOK_CELBORDER + SBOOK_CELHEIGHT) / 3)) {
			if (guTeamInviteRec & (1 << pnum)) {
				if (dx > SBOOK_LINE_TAB && dx < SBOOK_LINE_TAB + TBOOK_BTN_WIDTH) {
					// accept (invite)
					NetSendCmdBParam1(CMD_ACK_INVITE, pnum);
				} else if (dx > SBOOK_LINE_TAB + TBOOK_BTN_WIDTH + 10 && dx < SBOOK_LINE_TAB + 2 * TBOOK_BTN_WIDTH + 10) {
					// reject (invite)
					NetSendCmdBParam1(CMD_DEC_INVITE, pnum);
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

static char* control_print_talk_msg(char* msg, int* x, int y)
{
	int limit = TALK_PNL_WIDTH - 2 * TALK_PNL_BORDER;
	BYTE c;

	while (*msg != '\0') {
		c = gbStdFontFrame[(BYTE)*msg];
		limit -= smallFontWidth[c] + FONT_KERN_SMALL;
		if (limit < 0)
			return msg;
		msg++;
		if (c != 0) {
			PrintChar(*x, y, c, COL_WHITE);
		}
		*x += smallFontWidth[c] + FONT_KERN_SMALL;
	}
	return NULL;
}

void DrawTalkPan()
{
	int x, y;
	char* msg;

	assert(gbTalkflag);

	int sx = TALK_PNL_X;
	int sy = TALK_PNL_Y;

	// add background
	CelDraw(sx, sy + TALK_PNL_HEIGHT, pTalkPnlCel, 1);

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
	DrawSingleSmallPentSpn(x, y);
}

void control_type_message()
{
	if (IsLocalGame) {
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

	pmask = SNPLAYER_ALL;
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
	} else {
		control_reset_talk();
	}
}

bool control_talk_last_key(int vkey)
{
	unsigned result;

	assert(gbTalkflag);
	assert(!IsLocalGame);

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
