/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

BYTE sgbNextTalkSave;
BYTE sgbTalkSavePos;
BYTE *pDurIcons;
BYTE *pChrButtons;
bool gbDropGoldFlag;
int initialDropGoldIndex;
int initialDropGoldValue;
int dropGoldValue;
bool gabPanbtn[NUM_PANBTNS];
bool _gabChrbtn[4];
BYTE *pPanelButtons;
BYTE *pChrPanel;
bool gbLvlbtndown;
char sgszTalkSave[8][MAX_SEND_STR_LEN];
bool gbChrbtnactive;
char sgszTalkMsg[MAX_SEND_STR_LEN];
BYTE *pPanelText;
BYTE *pFlasks;
BYTE *pTalkPnl;
BYTE *pTalkBtns;
bool _gabTalkbtndown[MAX_PLRS - 1];
/**
 * The 'highlighted' skill in the Skill-List or in the Spell-Book.
 */
BYTE currSkill;
/**
 * The type of the 'highlighted' skill in the Skill-List or in the Spell-Book.
 */
BYTE currSkillType;
BYTE infoclr;
BYTE *pGBoxBuff;
char tempstr[256];
bool _gabWhisper[MAX_PLRS];
/**
 * The current tab in the Spell-Book.
 */
int sbooktab;
/**
 * Specifies whether the Chat-Panel is displayed.
 */
bool gbTalkflag;
BYTE *pSBkIconCels;
/**
 * Specifies whether the Spell-Book is displayed.
 */
bool gbSbookflag;
/**
 * Specifies whether the Character-Panel is displayed.
 */
bool gbChrflag;
BYTE *pSpellBkCel;
char infostr[256];
int numpanbtns;
BYTE SplTransTbl[256];
static_assert(RSPLTYPE_CHARGES != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SCROLL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_ABILITY != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SPELL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_INVALID != -1, "Cached value of spellTrans must not be -1.");
char lastSt = -1;
BYTE *pSpellCels;
/**
 * Specifies whether the Skill-List is displayed.
 */
bool gbSkillListFlag;
/**
 * Specifies whether the cursor should be moved to the active skill in the Skill-List.
 */
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
BYTE _gbMoveCursor = 0;
#endif

/** Maps from font index to smaltext.cel frame number. */
const BYTE fontframe[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 54, 44, 57, 58, 56, 55, 47, 40, 41, 59, 39, 50, 37, 51, 52,
	36, 27, 28, 29, 30, 31, 32, 33, 34, 35, 48, 49, 60, 38, 61, 53,
	62, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 42, 63, 43, 64, 65,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 40, 66, 41, 67, 0
};

/**
 * Maps from smaltext.cel frame number to character width. Note, the
 * character width may be distinct from the frame width, which is 13 for every
 * smaltext.cel frame.
 */
const BYTE fontkern[68] = {
	8, 10, 7, 9, 8, 7, 6, 8, 8, 3,
	3, 8, 6, 11, 9, 10, 6, 9, 9, 6,
	9, 11, 10, 13, 10, 11, 7, 5, 7, 7,
	8, 7, 7, 7, 7, 7, 10, 4, 5, 6,
	3, 3, 4, 3, 6, 6, 3, 3, 3, 3,
	3, 2, 7, 6, 3, 10, 10, 6, 6, 7,
	4, 4, 9, 6, 6, 12, 3, 7
};

/**
 * Maps ASCII character code to font index, as used by the
 * small, medium and large sized fonts; which corresponds to smaltext.cel,
 * medtexts.cel and bigtgold.cel respectively.
 */
const BYTE gbFontTransTbl[256] = {
	// clang-format off
	'\0', 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	' ',  '!',  '\"', '#',  '$',  '%',  '&',  '\'', '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	'@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
	'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\\', ']',  '^',  '_',
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x01,
	'C',  'u',  'e',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'i',  'i',  'i',  'A',  'A',
	'E',  'a',  'A',  'o',  'o',  'o',  'u',  'u',  'y',  'O',  'U',  'c',  'L',  'Y',  'P',  'f',
	'a',  'i',  'o',  'u',  'n',  'N',  'a',  'o',  '?',  0x01, 0x01, 0x01, 0x01, '!',  '<',  '>',
	'o',  '+',  '2',  '3',  '\'', 'u',  'P',  '.',  ',',  '1',  '0',  '>',  0x01, 0x01, 0x01, '?',
	'A',  'A',  'A',  'A',  'A',  'A',  'A',  'C',  'E',  'E',  'E',  'E',  'I',  'I',  'I',  'I',
	'D',  'N',  'O',  'O',  'O',  'O',  'O',  'X',  '0',  'U',  'U',  'U',  'U',  'Y',  'b',  'B',
	'a',  'a',  'a',  'a',  'a',  'a',  'a',  'c',  'e',  'e',  'e',  'e',  'i',  'i',  'i',  'i',
	'o',  'n',  'o',  'o',  'o',  'o',  'o',  '/',  '0',  'u',  'u',  'u',  'u',  'y',  'b',  'y',
	// clang-format on
};

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
	{   0, 20 + 8 * MENUBTN_HEIGHT }, // pvp button
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
	"PvP"
	// clang-format on
};
/** Maps from attribute_id to the rectangle on screen used for attribute increment buttons. */
const RECT32 ChrBtnsRect[4] = {
	// clang-format off
	{ 132, 102, CHRBTN_WIDTH, CHRBTN_HEIGHT },
	{ 132, 130, CHRBTN_WIDTH, CHRBTN_HEIGHT },
	{ 132, 159, CHRBTN_WIDTH, CHRBTN_HEIGHT },
	{ 132, 187, CHRBTN_WIDTH, CHRBTN_HEIGHT }
	// clang-format on
};

/** Maps from spellbook page number and position to spell_id. */
int SpellPages[SPLBOOKTABS][7] = {
	{ SPL_NULL, SPL_SWIPE, SPL_FIREBOLT, SPL_FIREBALL, SPL_FIREWALL, SPL_WAVE, SPL_FLAME },
	{ SPL_POINT_BLANK, SPL_FAR_SHOT, SPL_CBOLT, SPL_LIGHTNING, SPL_FLASH, SPL_NOVA, SPL_CHAIN },
	{ SPL_HBOLT, SPL_FLARE, SPL_ELEMENT, SPL_STONE, SPL_TELEKINESIS, SPL_GOLEM, SPL_GUARDIAN },
	{ SPL_MANASHIELD, SPL_HEAL, SPL_HEALOTHER, SPL_RNDTELEPORT, SPL_TELEPORT, SPL_TOWN, SPL_RESURRECT },
#ifdef HELLFIRE
	{ SPL_IMMOLAT, SPL_FIRERING, SPL_LIGHTWALL, SPL_RUNEFIRE, SPL_RUNEIMMOLAT, SPL_RUNELIGHT, SPL_RUNENOVA },
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
	PlayerStruct *p;
	ItemStruct *pi;
	int t, v;

	p = &plr[myplr];
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
		pi = p->InvList;
		for (t = p->_pNumInv; t > 0; t--, pi++) {
			if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
			    && pi->_iSpell == sn) {
				v++;
			}
		}
		pi = p->SpdList;
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
			p->InvBody[INVLOC_HAND_LEFT]._iCharges,
			p->InvBody[INVLOC_HAND_LEFT]._iMaxCharges);
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
	int lvl, y;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (spl == SPL_INVALID) {
		st = RSPLTYPE_INVALID;
		spl = SPL_NULL;
	} else if ((spelldata[spl].sFlags & plr[myplr]._pSkillFlags) != spelldata[spl].sFlags)
		st = RSPLTYPE_INVALID;
	else if (st == RSPLTYPE_SPELL) {
		lvl = GetSpellLevel(myplr, spl);
		if (lvl <= 0 || !CheckSpell(myplr, spl))
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

	p = &plr[myplr];
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
	unsigned __int64 mask;

	currSkill = SPL_INVALID;
	x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	y = PANEL_Y - 17;
	p = &plr[myplr];
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
				sl = GetSpellLevel(myplr, j);
				st = sl > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID;
			}
			if ((spelldata[j].sFlags & plr[myplr]._pSkillFlags) != spelldata[j].sFlags)
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

	p = &plr[myplr];
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

		p = &plr[myplr];
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

	p = &plr[myplr];
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

/**
 * @brief Print letter to the back buffer
 * @param sx Backbuffer offset
 * @param sy Backbuffer offset
 * @param nCel Number of letter in Windows-1252
 * @param col text_color color value
 */
void PrintChar(int sx, int sy, int nCel, char col)
{
	assert(gpBuffer != NULL);

	int i;
	BYTE pix;
	BYTE tbl[256];

	switch (col) {
	case COL_WHITE:
		CelDraw(sx, sy, pPanelText, nCel, 13);
		return;
	case COL_BLUE:
		for (i = 0; i < lengthof(tbl); i++) {
			pix = i;
			if (pix > PAL16_GRAY + 13)
				pix = PAL16_BLUE + 15;
			else if (pix >= PAL16_GRAY)
				pix -= PAL16_GRAY - (PAL16_BLUE + 2);
			tbl[i] = pix;
		}
		break;
	case COL_RED:
		for (i = 0; i < lengthof(tbl); i++) {
			pix = i;
			if (pix >= PAL16_GRAY)
				pix -= PAL16_GRAY - PAL16_RED;
			tbl[i] = pix;
		}
		break;
	case COL_GOLD:
		for (i = 0; i < lengthof(tbl); i++) {
			pix = i;
			if (pix >= PAL16_GRAY) {
				if (pix >= PAL16_GRAY + 14)
					pix = PAL16_YELLOW + 15;
				else
					pix -= PAL16_GRAY - (PAL16_YELLOW + 2);
			}
			tbl[i] = pix;
		}
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	CelDrawLight(sx, sy, pPanelText, nCel, 13, tbl);
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
	BYTE *empty, *full;
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
		maxHP = plr[myplr]._pMaxHP;
		hp = plr[myplr]._pHitPoints;
		if (hp <= 0 || maxHP <= 0)
			filled = 0;
		else
			filled = 82 * hp / maxHP;
		if (filled > 82)
			filled = 82;
		plr[myplr]._pHPPer = filled;
	} else {
		filled = plr[myplr]._pHPPer;
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
		maxMana = plr[myplr]._pMaxMana;
		mana = plr[myplr]._pMana;

		if (mana <= 0 || maxMana <= 0)
			filled = 0;
		else
			filled = 82 * mana / maxMana;
		if (filled > 82)
			filled = 82;
		plr[myplr]._pManaPer = filled;
	} else {
		filled = plr[myplr]._pManaPer;
	}

	x = SCREEN_X + SCREEN_WIDTH - (SPLICONLENGTH + 92);
	DrawFlask2(x, filled, 3, plr[myplr]._pManaShield == 0 ? 4 : 5, 93);
}

void InitControlPan()
{
	int i;

	pFlasks = LoadFileInMem("CtrlPan\\Flasks.CEL", NULL);;
	pPanelText = LoadFileInMem("CtrlPan\\SmalText.CEL", NULL);
	pChrPanel = LoadFileInMem("Data\\Char.CEL", NULL);
#ifdef HELLFIRE
	pSpellCels = LoadFileInMem("Data\\SpelIcon.CEL", NULL);
#else
	pSpellCels = LoadFileInMem("CtrlPan\\SpelIcon.CEL", NULL);
#endif
	SetSpellTrans(RSPLTYPE_ABILITY);
	gbTalkflag = false;
	if (gbMaxPlayers != 1) {
		pTalkPnl = LoadFileInMem("CtrlPan\\TalkPnl.CEL", NULL);
		pTalkBtns = LoadFileInMem("CtrlPan\\TalkButt.CEL", NULL);
		sgszTalkMsg[0] = '\0';
		for (i = 0; i < lengthof(_gabWhisper); i++)
			_gabWhisper[i] = true;
		for (i = 0; i < lengthof(_gabTalkbtndown); i++)
			_gabTalkbtndown[i] = false;
	}
	gbLvlbtndown = false;
	pPanelButtons = LoadFileInMem("CtrlPan\\Menu.CEL", NULL);
	for (i = 0; i < lengthof(gabPanbtn); i++)
		gabPanbtn[i] = false;
	numpanbtns = gbMaxPlayers == 1 ? NUM_PANBTNS - 2 : NUM_PANBTNS;
	pChrButtons = LoadFileInMem("Data\\CharBut.CEL", NULL);
	for (i = 0; i < lengthof(_gabChrbtn); i++)
		_gabChrbtn[i] = false;
	gbChrbtnactive = false;
	pDurIcons = LoadFileInMem("Items\\DurIcons.CEL", NULL);
	infostr[0] = '\0';
	gbRedrawFlags |= REDRAW_HP_FLASK | REDRAW_MANA_FLASK | REDRAW_SPEED_BAR;
	gbChrflag = false;
	gbSkillListFlag = false;
	pSpellBkCel = LoadFileInMem("Data\\SpellBk.CEL", NULL);
	pSBkIconCels = LoadFileInMem("Data\\SpellI2.CEL", NULL);
	sbooktab = 0;
	gbSbookflag = false;
	SpellPages[0][0] = Abilities[plr[myplr]._pClass];
	pQLogCel = LoadFileInMem("Data\\Quest.CEL", NULL);
	pGBoxBuff = LoadFileInMem("CtrlPan\\Golddrop.cel", NULL);
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
	const char* text;

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
		text = PanBtnTxt[i];
		if (i == PANBTN_FRIENDLY)
			text = gbFriendlyMode ? "PvP:Off" : "PvP:On";
		PrintString(x + 3, y + 15, x + 70, text, true, pb ? COL_GOLD : COL_WHITE, 1);
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
		gbInvflag = false;
		gbChrflag = false;
		gbQuestlog = false;
		gbSbookflag = false;
		gbHelpflag = false;
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
	if (plr[myplr]._pLvlUp && InLvlUpRect())
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
		plr[myplr]._pLvlUp = FALSE;
		gbChrflag = !gbChrflag;
		break;
	case PANBTN_INVENTORY:
		gbSbookflag = false;
		gbSkillListFlag = false;
		gbInvflag = !gbInvflag;
		break;
	case PANBTN_SPELLBOOK:
		gbInvflag = false;
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
	case PANBTN_FRIENDLY:
		gbFriendlyMode = !gbFriendlyMode;
		NetSendCmdBParam1(TRUE, CMD_PLRFRIENDY, gbFriendlyMode);
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
	MemFreeDbg(pPanelText);
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

bool control_WriteStringToBuffer(BYTE *str)
{
	int k;

	k = 0;
	while (*str != '\0') {
		k += fontkern[fontframe[gbFontTransTbl[*str]]];
		str++;
		if (k >= 125)
			return false;
	}

	return true;
}

static int StringWidth(const char *str)
{
	BYTE c;
	int strWidth;

	strWidth = 0;
	while (*str != '\0') {
		c = gbFontTransTbl[(BYTE)*str++];
		strWidth += fontkern[fontframe[c]] + 1; // + kern
	}
	return strWidth;
}

/**
 * @brief Render text string to back buffer
 * @param x Screen coordinate
 * @param y Screen coordinate
 * @param endX End of line in screen coordinate
 * @param pszStr String to print, in Windows-1252 encoding
 * @param center 
 * @param col text_color color value
 * @param kern Letter spacing
 */
void PrintString(int x, int y, int endX, const char *pszStr, bool center, BYTE col, int kern)
{
	BYTE c;
	const char *tmp;
	int strEnd;
	int cw;

	if (center) {
		strEnd = x;
		tmp = pszStr;
		while (*tmp != '\0') {
			c = gbFontTransTbl[(BYTE)*tmp++];
			strEnd += fontkern[fontframe[c]] + kern;
		}
		if (strEnd < endX) {
			x += (endX - strEnd) >> 1;
		}
	}
	while (*pszStr != '\0') {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		cw = fontkern[c] + kern;
		if (x + cw < endX && c != '\0') {
			PrintChar(x, y, c, col);
		}
		x += cw;
	}
}

#define ADD_PlrStringXY(x, y, endX, pszStr, col) PrintString(x + SCREEN_X, y + SCREEN_Y, endX + SCREEN_X, pszStr, true, col, 1)

void PrintGameStr(int x, int y, const char *str, BYTE color)
{
	BYTE c;
	int sx, sy;
	sx = x + SCREEN_X;
	sy = y + SCREEN_Y;
	while (*str != '\0') {
		c = gbFontTransTbl[(BYTE)*str++];
		c = fontframe[c];
		if (c != '\0')
			PrintChar(sx, sy, c, color);
		sx += fontkern[c] + 1;
	}
}

void DrawChr()
{
	PlayerStruct *p;
	BYTE col;
	char chrstr[64];
	int pc, val, mindam, maxdam;

	p = &plr[myplr];
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

static void DrawTooltip2(const char *text1, const char* text2, int x, int y, BYTE col)
{
	int width;
	BYTE *dst;
	const int border = 4, height = 26;
	int w1 = StringWidth(text1);
	int w2 = StringWidth(text2);

	width = std::max(w1, w2) + 2 * border;

	y -= TILE_HEIGHT;
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
		memset(dst, PAL16_GRAY + 2, width);

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
	ShiftGrid(&px, &py, -y, x);

	px *= TILE_WIDTH / 2;
	py *= TILE_HEIGHT / 2;

	if (!gbZoomflag) {
		px <<= 1;
		py <<= 1;
	}

	if (PANELS_COVER) {
		if (gbChrflag | gbQuestlog) {
			px += SPANEL_WIDTH / 2;
		} else if (gbInvflag | gbSbookflag) {
			px -= SPANEL_WIDTH / 2;
		}
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

static void DrawTooltip(const char* text, int x, int y, BYTE col)
{
	int width;
	BYTE *dst;
	const int border = 4, height = 16;

	width = StringWidth(text) + 2 * border;

	y -= TILE_HEIGHT;
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
		memset(dst, PAL16_GRAY + 2, width);

	// draw background
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_ORANGE + 14, width - 2);

	// print the info
	PrintGameStr(x + border, y + height - 3, text, col);
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
		memset(dst, PAL16_GRAY + 2, width);

	// draw the bar
	//width = (width - 2) * hp / maxhp;
	dhp = (maxhp + 7) >> 3;
	dw = ((width - 2) >> 3);
	for (w = 0, h = 0; h < hp; h += dhp, w += dw) {
	}
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_RED + 6, w);
}

static void DrawTrigInfo()
{
	int xx, yy;

	if (pcurstrig >= MAXTRIGGERS + 1) {
		// portal
		MissileStruct *mis = &missile[pcurstrig - (MAXTRIGGERS + 1)];
		if (mis->_miType == MIS_TOWN) {
			copy_cstr(infostr, "Town Portal");
			snprintf(tempstr, sizeof(tempstr), "(%s)", plr[mis->_miSource]._pName);
			GetMousePos(cursmx - 2, cursmy - 2, &xx, &yy);
			DrawTooltip2(infostr, tempstr, xx, yy, COL_WHITE);
		} else {
			if (!gbSetlevel) {
				copy_cstr(infostr, "Portal to The Unholy Altar");
			} else {
				copy_cstr(infostr, "Portal to level 15");
			}
			GetMousePos(cursmx - 2, cursmy - 2, &xx, &yy);
			DrawTooltip(infostr, xx, yy, COL_WHITE);
		}
		return;
	} else if (pcurstrig >= 0) {
		// standard trigger
		switch (trigs[pcurstrig]._tmsg) {
		case WM_DIABNEXTLVL:
			/*if (currlevel == 0)
				copy_cstr(infostr, "Down to dungeon");
			else*/ if (currlevel == 15)
				copy_cstr(infostr, "Down to Diablo");
#ifdef HELLFIRE
			else if (currlevel >= 21)
				snprintf(infostr, sizeof(infostr), "Down to Crypt level %i", currlevel - 19);
			else if (currlevel >= 17)
				snprintf(infostr, sizeof(infostr), "Down to Nest level %i", currlevel - 15);
#endif
			else
				snprintf(infostr, sizeof(infostr), "Down to level %i", currlevel + 1);
			break;
		case WM_DIABPREVLVL:
			if (currlevel == 1)
				copy_cstr(infostr, "Up to town");
#ifdef HELLFIRE
			else if (currlevel >= 21)
				snprintf(infostr, sizeof(infostr), "Up to Crypt level %i", currlevel - 21);
			else if (currlevel >= 17)
				snprintf(infostr, sizeof(infostr), "Up to Nest level %i", currlevel - 17);
#endif
			else
				snprintf(infostr, sizeof(infostr), "Up to level %i", currlevel - 1);
			break;
		case WM_DIABRTNLVL:
			assert(gbSetlevel);
			switch (setlvlnum) {
			case SL_SKELKING:
				xx = Q_SKELKING;
				break;
			case SL_BONECHAMB:
				xx = Q_SCHAMB;
				break;
			case SL_POISONWATER:
				xx = Q_PWATER;
				break;
			default:
				app_fatal("Unrecognized setlevel %d to return.", setlvlnum);
			}
			snprintf(infostr, sizeof(infostr), "Back to Level %i", quests[xx]._qlevel);
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
			case TWARP_HIVE:
				copy_cstr(infostr, "Down to Hive");
				break;
			case TWARP_CRYPT:
				copy_cstr(infostr, "Down to Crypt");
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
	} else {
		// quest trigger
		switch (quests[quests[-2 - pcurstrig]._qidx]._qslvl) {
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
			copy_cstr(infostr, "To Unholy Altar");
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

	if (pcursitem != -1) {
		ItemStruct* is = &item[pcursitem];
		GetItemInfo(is);
		x = is->_ix;
		y = is->_iy;
		GetMousePos(x, y, &xx, &yy);
		DrawTooltip(infostr, xx, yy, infoclr);
	} else if (pcursobj != -1) {
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
		if (leveltype != DTYPE_TOWN) {
			strcpy(infostr, mon->mName);
			if (mon->_uniqtype != 0) {
				col = COL_GOLD;
			}
		} else if (pcursitem == -1) {
			strcpy(infostr, towner[pcursmonst]._tName);
		}
		GetMousePos(x, y, &xx, &yy);
		DrawTooltip(infostr, xx, yy, col);
		DrawHealthBar(mon->_mhitpoints, mon->_mmaxhp, xx, yy);
	} else if (pcursplr != -1) {
		PlayerStruct* p = &plr[pcursplr];
		x = p->_px - 2;
		y = p->_py - 2;
		GetMousePos(x, y, &xx, &yy);
		snprintf(infostr, sizeof(infostr), p->_pManaShield == 0 ? "%s(%i)" : "%s(%i)*", ClassStrTbl[p->_pClass], p->_pLevel);
		DrawTooltip2(p->_pName, infostr, xx, yy, COL_GOLD);
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
	} else if (pcursinvitem != -1) {
		DrawInvItemDetails();
	} else if (pcurstrig != -1) {
		DrawTrigInfo();
	} else if (pcurs >= CURSOR_FIRSTITEM) {
		ItemStruct *is = &plr[myplr].HoldItem;
		GetItemInfo(is);
		DrawTooltip(infostr, MouseX + cursW / 2, MouseY, infoclr);
	}
}


bool CheckChrBtns()
{
	int i;

	if (plr[myplr]._pStatPts != 0 && !gbChrbtnactive) {
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
				plr[myplr]._pStatPts--;
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

	inv = plr[myplr].InvBody;
	x = DrawDurIcon4Item(&inv[INVLOC_HEAD], x, 4);
	x = DrawDurIcon4Item(&inv[INVLOC_CHEST], x, 3);
	x = DrawDurIcon4Item(&inv[INVLOC_HAND_LEFT], x, 0);
	DrawDurIcon4Item(&inv[INVLOC_HAND_RIGHT], x, 0);
}

void RedBack()
{
	int idx;

	idx = 4608;
#ifdef _DEBUG
	if (light4flag)
		idx = 1536;
#endif
	assert(gpBuffer != NULL);

	int w, h;
	BYTE *dst, *tbl;

	dst = &gpBuffer[SCREENXY(0, 0)];
	tbl = &pLightTbl[idx];
	if (leveltype != DTYPE_HELL) {
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

	p = &plr[myplr];
	if (p->_pAblSkills & SPELL_MASK(sn)) { /// BUGFIX: missing (__int64) (fixed)
		st = RSPLTYPE_ABILITY;
	} else if (p->_pISpells & SPELL_MASK(sn)) {
		st = RSPLTYPE_CHARGES;
	} else if (CheckSpell(myplr, sn)) {
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
	unsigned __int64 spl;

	// back panel
	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pSpellBkCel, 1, SPANEL_WIDTH);
	// selected page
	snprintf(tempstr, sizeof(tempstr), "%d.", sbooktab + 1);
	PrintString(RIGHT_PANEL_X + 2, SCREEN_Y + SPANEL_HEIGHT - 7, RIGHT_PANEL_X + SPANEL_WIDTH, tempstr, true, COL_WHITE, 0);

	currSkill = SPL_INVALID;

	p = &plr[myplr];
	spl = p->_pMemSkills | p->_pISpells | p->_pAblSkills;

	yp = SCREEN_Y + SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx = RIGHT_PANEL_X + SBOOK_CELBORDER;
	for (i = 0; i < lengthof(SpellPages[sbooktab]); i++) {
		sn = SpellPages[sbooktab][i];
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
				lvl = GetSpellLevel(myplr, sn);
				if (lvl > 0) {
					snprintf(tempstr, sizeof(tempstr), "Spell Level %i", lvl);
				} else {
					copy_cstr(tempstr, "Spell Level 0 - Unusable");
				}
				mana = GetManaAmount(myplr, sn) >> 6;
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

			if ((spelldata[sn].sFlags & plr[myplr]._pSkillFlags) != spelldata[sn].sFlags)
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

	if (dy >= lengthof(SpellPages[sbooktab]) * (SBOOK_CELBORDER + SBOOK_CELHEIGHT)) {
		if (dx <= SBOOK_PAGER_WIDTH * 2) {
			if (dx <= SBOOK_PAGER_WIDTH) {
				sbooktab = 0;
			} else {
				if (sbooktab != 0)
					sbooktab--;
			}
		} else if (dx >= SPANEL_WIDTH - SBOOK_PAGER_WIDTH * 2) {
			if (dx >= SPANEL_WIDTH - SBOOK_PAGER_WIDTH) {
				sbooktab = SPLBOOKTABS - 1;
			} else {
				if (sbooktab < SPLBOOKTABS - 1)
					sbooktab++;
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
	int screen_x, i;

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
		for (i = 0; i < tempstr[i]; i++) {
			screen_x += fontkern[fontframe[gbFontTransTbl[(BYTE)tempstr[i]]]] + 1;
		}
		screen_x += 452;
	} else {
		screen_x = 450;
	}
	CelDraw(screen_x, 140 + SCREEN_Y, pSPentSpn2Cels, PentSpn2Spin(), 12);
}

static void control_remove_gold()
{
	ItemStruct *is;
	int gi, val;
	int pnum = myplr, gold_index = initialDropGoldIndex;

	if (gold_index <= INVITEM_INV_LAST) {
		gi = gold_index - INVITEM_INV_FIRST;
		is = &plr[pnum].InvList[gi];
		val = is->_ivalue - dropGoldValue;
		if (val > 0)
			SetGoldItemValue(is, val);
		else
			RemoveInvItem(pnum, gi);
	} else {
		gi = gold_index - INVITEM_BELT_FIRST;
		is = &plr[pnum].SpdList[gi];
		val = is->_ivalue - dropGoldValue;
		if (val > 0)
			SetGoldItemValue(is, val);
		else
			RemoveSpdBarItem(pnum, gi);
	}
	is = &plr[pnum].HoldItem;
	CreateBaseItem(is, IDI_GOLD);
	is->_iStatFlag = TRUE;
	SetGoldItemValue(is, dropGoldValue);
	NewCursor(is->_iCurs + CURSOR_FIRSTITEM);
	CalculateGold(pnum);
}

void control_drop_gold(char vkey)
{
	int newValue;

	if (plr[myplr]._pHitPoints < (1 << 6)) {
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

#define TALK_PNL_WIDTH		302
#define TALK_PNL_HEIGHT		116
#define TALK_PNL_TOP		(SCREEN_HEIGHT - 8 - TALK_PNL_HEIGHT)
#define TALK_PNL_LEFT		((SCREEN_WIDTH - TALK_PNL_WIDTH) / 2)
static char *control_print_talk_msg(char *msg, int *x, int y, int color)
{
	BYTE c;

	while (*msg != '\0') {
		c = gbFontTransTbl[(BYTE)*msg];
		c = fontframe[c];
		if ((*x + fontkern[c] + 1) > SCREEN_X + TALK_PNL_LEFT + TALK_PNL_WIDTH - 15)
			return msg;
		msg++;
		if (c != '\0') {
			PrintChar(*x, y, c, color);
		}
		*x += fontkern[c] + 1;
	}
	return NULL;
}

void DrawTalkPan()
{
	int i, talk_btn, color, nCel, x, y;
	char *msg;

	assert(gbTalkflag);

	int sx = SCREEN_X + TALK_PNL_LEFT;
	int sy = SCREEN_Y + TALK_PNL_TOP;

	// add background
	CelDraw(sx, sy + TALK_PNL_HEIGHT, pTalkPnl, 1, TALK_PNL_WIDTH);

	// print the current (not sent) message
	sy += 17;
	msg = sgszTalkMsg;
	for (y = sy; ; y += 13) {
		x = sx + 15;
		msg = control_print_talk_msg(msg, &x, y, COL_WHITE);
		if (msg == NULL)
			break;
	}
	if (msg != NULL)
		*msg = '\0';
	CelDraw(x, y, pSPentSpn2Cels, PentSpn2Spin(), 12);

	// add the party members
	sy += 61;
	talk_btn = 0;
	static_assert(lengthof(_gabWhisper) == MAX_PLRS, "Table _gabWhisper does not work with the current MAX_PLRS in DrawTalkPan.");
	for (i = 0; i < MAX_PLRS; i++) {
		if (i == myplr)
			continue;
		if (_gabWhisper[i]) {
			color = COL_GOLD;
			nCel = 0;
		} else {
			color = COL_RED;
			nCel = 2;
		}
		if (_gabTalkbtndown[talk_btn])
			nCel += 4;
		if (nCel != 0)
			CelDraw(sx + 3, sy + 2, pTalkBtns, nCel, 61);
		if (plr[i].plractive) {
			x = sx + 31 + 46;
			control_print_talk_msg(plr[i]._pName, &x, sy, color);
		}
		sy += 18;
		talk_btn++;
	}
}

bool control_check_talk_btn()
{
	int i;

	if (MouseX < TALK_PNL_LEFT + 3 || MouseX > TALK_PNL_LEFT + 64)
		return false;
	if (MouseY < TALK_PNL_TOP + 65 || MouseY > TALK_PNL_TOP + 65 + 18 * lengthof(_gabTalkbtndown))
		return false;

	for (i = 0; i < lengthof(_gabTalkbtndown); i++) {
		_gabTalkbtndown[i] = false;
	}

	_gabTalkbtndown[(MouseY - (TALK_PNL_TOP + 65)) / 18] = true;

	return true;
}

void control_release_talk_btn()
{
	int i, y;

	if (MouseX >= TALK_PNL_LEFT + 3  && MouseX <= TALK_PNL_LEFT + 64) {
		y = MouseY - (TALK_PNL_TOP + 65);
		for (i = 0; i < lengthof(_gabTalkbtndown); i++, y -= 18) {
			if (_gabTalkbtndown[i] && y >= 0 && y <= 18) {
				if (i >= myplr)
					i++;
				_gabWhisper[i] = !_gabWhisper[i];
			}
		}
	}
	for (i = 0; i < lengthof(_gabTalkbtndown); i++) {
		_gabTalkbtndown[i] = false;
	}
}

void control_type_message()
{
	int i;

	if (gbMaxPlayers == 1) {
		return;
	}

	gbTalkflag = true;
	SDL_StartTextInput();
	sgszTalkMsg[0] = '\0';
	for (i = 0; i < lengthof(_gabTalkbtndown); i++) {
		_gabTalkbtndown[i] = false;
	}
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
	int i, pmask;
	BYTE talk_save;

	if (sgszTalkMsg[0] != '\0') {
		pmask = 0;

		static_assert(lengthof(_gabWhisper) == MAX_PLRS, "Table _gabWhisper does not work with the current MAX_PLRS in control_press_enter.");
		for (i = 0; i < MAX_PLRS; i++) {
			if (_gabWhisper[i])
				pmask |= 1 << i;
		}
		copy_str(gbNetMsg, sgszTalkMsg);
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
	int result;

	assert(gbTalkflag);
	assert(gbMaxPlayers != 1);

	if ((unsigned)vkey < DVL_VK_SPACE)
		return false;

	result = strlen(sgszTalkMsg);
	if (result < 78) {
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
		return control_check_talk_btn();
	} else if (vkey == DVL_VK_RBUTTON) {
		return false;
	}
	return true;
}

DEVILUTION_END_NAMESPACE
