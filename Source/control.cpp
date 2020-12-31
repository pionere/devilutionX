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
BOOL dropGoldFlag;
BOOL panbtn[8];
BOOL chrbtn[4];
BYTE *pMultiBtns;
BYTE *pPanelButtons;
BYTE *pChrPanel;
BOOL lvlbtndown;
char sgszTalkSave[8][80];
int dropGoldValue;
BOOL chrbtnactive;
char sgszTalkMsg[MAX_SEND_STR_LEN];
BYTE *pPanelText;
BYTE *pLifeBuff;
BYTE *pBtmBuff;
BYTE *pTalkBtns;
BOOL pstrjust[MAX_CTRL_PANEL_LINES];
int pnumlines;
BOOL pinfoflag;
BOOL talkbtndown[MAX_PLRS - 1];
int pSpell;
BYTE *pManaBuff;
char infoclr;
int sgbPlrTalkTbl;
BYTE *pGBoxBuff;
BYTE *pSBkBtnCel;
char tempstr[256];
BOOLEAN whisper[MAX_PLRS];
int sbooktab;
int pSplType;
int initialDropGoldIndex;
BOOL talkflag;
BYTE *pSBkIconCels;
BOOL sbookflag;
BOOL chrflag;
BYTE *pSpellBkCel;
char infostr[256];
int numpanbtns;
char panelstr[MAX_CTRL_PANEL_LINES][64];
BOOL panelflag;
BYTE SplTransTbl[256];
static_assert(RSPLTYPE_CHARGES != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SCROLL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SKILL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SPELL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_INVALID != -1, "Cached value of spellTrans must not be -1.");
char lastSt = -1;
int initialDropGoldValue;
BYTE *pSpellCels;
BOOL panbtndown;
BOOL spselflag;

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
 * Line start position for info box text when displaying 1, 2, 3, 4 and 5 lines respectivly
 */
const int lineOffsets[5][5] = {
	{ 82 },
	{ 70, 94 },
	{ 64, 82, 100 },
	{ 60, 75, 89, 104 },
	{ 58, 70, 82, 94, 105 },
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

/* data */

/** Maps from spell_id to spelicon.cel frame number. */
const char SpellITbl[NUM_SPELLS] = {
#ifdef HELLFIRE
	27,
#else
	1,
#endif
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	28,
	13,
	12,
	18,
	16,
	14,
	18,
	19,
	11,
	20,
	15,
	21,
	23,
	24,
	25,
	22,
	26,
	29,
	37,
	38,
	39,
	42,
	41,
	40,
	10,
	36,
	30,
#ifdef HELLFIRE
	51,
	51,
	50,
	46,
	47,
	43,
	45,
	48,
	49,
	44,
	35,
	35,
	35,
	35,
	35,
#endif
};
/** Maps from panel_button_id to the position and dimensions of a panel button. */
int PanBtnPos[8][5] = {
	// clang-format off
	{   9,   9, 71, 19, TRUE  }, // char button
	{   9,  35, 71, 19, FALSE }, // quests button
	{   9,  75, 71, 19, TRUE  }, // map button
	{   9, 101, 71, 19, FALSE }, // menu button
	{ 560,   9, 71, 19, TRUE  }, // inv button
	{ 560,  35, 71, 19, FALSE }, // spells button
	{  87,  91, 33, 32, TRUE  }, // chat button
	{ 527,  91, 33, 32, TRUE  }, // friendly fire button
	// clang-format on
};
/** Maps from panel_button_id to hotkey name. */
const char *const PanBtnHotKey[8] = { "'c'", "'q'", "Tab", "Esc", "'i'", "'b'", "Enter", NULL };
/** Maps from panel_button_id to panel button description. */
const char PanBtnStr[8][24] = {
	"Character Information",
	"Quests log",
	"Automap",
	"Main Menu",
	"Inventory",
	"Spell book",
	"Send Message",
	"Player Attack"
};
/** Maps from attribute_id to the rectangle on screen used for attribute increment buttons. */
RECT32 ChrBtnsRect[4] = {
	{ 137, 138, 41, 22 },
	{ 137, 166, 41, 22 },
	{ 137, 195, 41, 22 },
	{ 137, 223, 41, 22 }
};

/** Maps from spellbook page number and position to spell_id. */
int SpellPages[][7] = {
	{ SPL_NULL, SPL_FIREBOLT, SPL_CBOLT, SPL_HBOLT, SPL_HEAL, SPL_HEALOTHER, SPL_FLAME },
	{ SPL_RESURRECT, SPL_FIREWALL, SPL_TELEKINESIS, SPL_LIGHTNING, SPL_TOWN, SPL_FLASH, SPL_STONE },
	{ SPL_RNDTELEPORT, SPL_MANASHIELD, SPL_ELEMENT, SPL_FIREBALL, SPL_WAVE, SPL_CHAIN, SPL_GUARDIAN },
	{ SPL_NOVA, SPL_GOLEM, SPL_TELEPORT, SPL_APOCA, SPL_BONESPIRIT, SPL_FLARE, SPL_ETHEREALIZE },
#ifdef HELLFIRE
	{ SPL_LIGHTWALL, SPL_IMMOLAT, SPL_WARP, SPL_REFLECT, SPL_BERSERK, SPL_FIRERING, SPL_SEARCH },
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

	if (st == RSPLTYPE_SKILL) {
		for (i = 0; i < 128; i++)
			SplTransTbl[i] = i;
	}
	for (i = 128; i < 256; i++)
		SplTransTbl[i] = i;
	SplTransTbl[255] = 0;

	switch (st) {
	case RSPLTYPE_SKILL:
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

/**
 * Sets the spell frame to draw and its position then draws it.
 */
void DrawSpell()
{
	PlayerStruct *p;
	char st;
	int spl, tlvl;

	p = &plr[myplr];
	spl = p->_pRSpell;
	st = p->_pRSplType;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (spl == SPL_INVALID)
		st = RSPLTYPE_INVALID;
	else if (currlevel == 0 && !spelldata[spl].sTownSpell)
		st = RSPLTYPE_INVALID;
	else if (st == RSPLTYPE_SPELL) {
		tlvl = p->_pISplLvlAdd + p->_pSplLvl[spl];
		if (tlvl <= 0 || !CheckSpell(myplr, spl))
			st = RSPLTYPE_INVALID;
	}
	SetSpellTrans(st);
	DrawSpellCel(PANEL_X + 565, PANEL_Y + 119, pSpellCels,
		spl != SPL_INVALID ? SpellITbl[spl] : 27, SPLICONLENGTH);
}

void DrawSpellList()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i, j, x, y, c, s, t, v, lx, ly;
	unsigned __int64 mask, spl;

	pSpell = SPL_INVALID;
	infostr[0] = '\0';
	x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	y = PANEL_Y - 17;
	ClearPanel();
	p = &plr[myplr];
	for (i = 0; i < 4; i++) {
		switch ((spell_type)i) {
		case RSPLTYPE_SKILL:
			SetSpellTrans(RSPLTYPE_SKILL);
			mask = p->_pAblSpells;
			c = SPLICONLAST + 3;
			break;
		case RSPLTYPE_SPELL:
			mask = p->_pMemSpells;
			c = SPLICONLAST + 4;
			break;
		case RSPLTYPE_SCROLL:
			SetSpellTrans(RSPLTYPE_SCROLL);
			mask = p->_pScrlSpells;
			c = SPLICONLAST + 1;
			break;
		case RSPLTYPE_CHARGES:
			SetSpellTrans(RSPLTYPE_CHARGES);
			mask = p->_pISpells;
			c = SPLICONLAST + 2;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		for (spl = 1, j = 1; j < NUM_SPELLS; spl <<= 1, j++) {
			if (!(mask & spl))
				continue;
			if (currlevel == 0 && !spelldata[j].sTownSpell)
				SetSpellTrans(RSPLTYPE_INVALID);
			else if (i == RSPLTYPE_SPELL) {
				s = p->_pISplLvlAdd + p->_pSplLvl[j];
				SetSpellTrans(s > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID);
			}
			DrawSpellCel(x, y, pSpellCels, SpellITbl[j], SPLICONLENGTH);
			lx = x - BORDER_LEFT;
			ly = y - BORDER_TOP - SPLICONLENGTH;
			if (MouseX >= lx && MouseX < lx + SPLICONLENGTH && MouseY >= ly && MouseY < ly + SPLICONLENGTH) {
				pSpell = j;
				pSplType = i;
#ifdef HELLFIRE
				if (p->_pClass == PC_MONK && j == SPL_SEARCH)
					pSplType = RSPLTYPE_SKILL;
#endif
				DrawSpellCel(x, y, pSpellCels, c, SPLICONLENGTH);
#ifdef HELLFIRE
				switch (pSplType) {
#else
				switch (i) {
#endif
				case RSPLTYPE_SKILL:
					snprintf(infostr, sizeof(infostr), "%s Skill", spelldata[j].sNameText);
					break;
				case RSPLTYPE_SPELL:
					snprintf(infostr, sizeof(infostr), "%s Spell", spelldata[j].sNameText);
					if (j == SPL_HBOLT) {
						copy_cstr(tempstr, "Damages undead only");
						AddPanelString(tempstr, TRUE);
					}
					if (s <= 0)
						copy_cstr(tempstr, "Spell Level 0 - Unusable");
					else
						snprintf(tempstr, sizeof(tempstr), "Spell Level %i", s);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_SCROLL:
					snprintf(infostr, sizeof(infostr), "Scroll of %s", spelldata[j].sNameText);
					v = 0;
					pi = p->InvList;
					for (t = p->_pNumInv; t > 0; t--, pi++) {
						if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
						    && pi->_iSpell == j) {
							v++;
						}
					}
					pi = p->SpdList;
					for (t = MAXBELTITEMS; t > 0; t--, pi++) {
						if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
						    && pi->_iSpell == j) {
							v++;
						}
					}
					if (v == 1)
						copy_cstr(tempstr, "1 Scroll");
					else
						snprintf(tempstr, sizeof(tempstr), "%i Scrolls", v);
					AddPanelString(tempstr, TRUE);
					break;
				case RSPLTYPE_CHARGES:
					snprintf(infostr, sizeof(infostr), "Staff of %s", spelldata[j].sNameText);
					if (p->InvBody[INVLOC_HAND_LEFT]._iCharges == 1)
						copy_cstr(tempstr, "1 Charge");
					else
						snprintf(tempstr, sizeof(tempstr), "%i Charges", p->InvBody[INVLOC_HAND_LEFT]._iCharges);
					AddPanelString(tempstr, TRUE);
					break;
				default:
					ASSUME_UNREACHABLE
					break;
				}
				for (t = 0; t < 4; t++) {
					if (p->_pSplHotKey[t] == j && p->_pSplTHotKey[t] == pSplType) {
						DrawSpellCel(x, y, pSpellCels, t + SPLICONLAST + 5, SPLICONLENGTH);
						snprintf(tempstr, sizeof(tempstr), "Spell Hot Key #F%i", t + 5);
						AddPanelString(tempstr, TRUE);
					}
				}
			}
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
		if (mask != 0 && x != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS)
			x -= SPLICONLENGTH;
		if (x == PANEL_X + 12 - SPLICONLENGTH) {
			x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
			y -= SPLICONLENGTH;
		}
	}
}

void SetSpell()
{
	spselflag = FALSE;
	if (pSpell != SPL_INVALID) {
		ClearPanel();
		plr[myplr]._pRSpell = pSpell;
		plr[myplr]._pRSplType = pSplType;
		gbRedrawFlags = REDRAW_ALL;
	}
}

void SetSpeedSpell(int slot)
{
	PlayerStruct *p;
	int i;

	if (pSpell != SPL_INVALID) {
		p = &plr[myplr];
		for (i = 0; i < lengthof(p->_pSplHotKey); ++i) {
			if (p->_pSplHotKey[i] == pSpell && p->_pSplTHotKey[i] == pSplType) {
				p->_pSplHotKey[i] = SPL_INVALID;
				p->_pSplTHotKey[i] = RSPLTYPE_INVALID;
			}
		}
		p->_pSplHotKey[slot] = pSpell;
		p->_pSplTHotKey[slot] = pSplType;
	}
}

void ToggleSpell(int slot)
{
	PlayerStruct *p;
	unsigned __int64 spells;

	p = &plr[myplr];
	switch (p->_pSplTHotKey[slot]) {
	case RSPLTYPE_SKILL:
		spells = p->_pAblSpells;
		break;
	case RSPLTYPE_SPELL:
		spells = p->_pMemSpells;
		break;
	case RSPLTYPE_SCROLL:
		spells = p->_pScrlSpells;
		break;
	case RSPLTYPE_CHARGES:
		spells = p->_pISpells;
		break;
	case RSPLTYPE_INVALID:
		return;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	if (spells & SPELL_MASK(p->_pSplHotKey[slot])) {
		p->_pRSpell = p->_pSplHotKey[slot];
		p->_pRSplType = p->_pSplTHotKey[slot];
		gbRedrawFlags = REDRAW_ALL;
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

void AddPanelString(const char *str, BOOL just)
{
	SStrCopy(panelstr[pnumlines], str, sizeof(panelstr[pnumlines]));
	pstrjust[pnumlines] = just;

	if (pnumlines < MAX_CTRL_PANEL_LINES)
		pnumlines++;
}

void ClearPanel()
{
	pnumlines = 0;
	pinfoflag = FALSE;
}

void DrawPanelBox(int x, int y, int w, int h, int sx, int sy)
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
}

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
static void SetFlaskHeight(BYTE *pCelBuff, int min, int max, int sx, int sy)
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
}

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
static void DrawFlask(BYTE *pCelBuff, int w, int nSrcOff, int nDstOff, int h)
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
}

void DrawLifeFlask()
{
	int filled, height;
	int maxHP, hp;

	if (gbRedrawFlags & REDRAW_HP_FLASK) {
		maxHP = plr[myplr]._pMaxHP;
		hp = plr[myplr]._pHitPoints;
		if (hp <= 0 || maxHP <= 0)
			filled = 0;
		else
			filled = 80 * hp / maxHP;
		if (filled > 80)
			filled = 80;
		plr[myplr]._pHPPer = filled;

		/**
		 * Draw the life flask within the control panel.
		 * First sets the fill amount then draws the empty flask cel portion then the filled
		 * flask portion.
		 */
		height = filled;
		if (height > 69)
			height = 69;
		if (height != 69)
			SetFlaskHeight(pLifeBuff, 16, 85 - height, 96 + PANEL_X, PANEL_Y);
		if (height != 0)
			DrawPanelBox(96, 85 - height, 88, height, 96 + PANEL_X, PANEL_Y + 69 - height);
	} else {
		filled = plr[myplr]._pHPPer;
	}

	/**
	 * Draw the top dome of the life flask (that part that protrudes out of the control panel).
	 * First it draws the empty flask cel and then draws the filled part on top if needed.
	 */
	height = 80 - filled;
	if (height > 12)
		height = 12;
	height += 1;

	DrawFlask(pLifeBuff, 88, 88 * 3 + 13, SCREENXY(PANEL_LEFT + 109, PANEL_TOP - 13), height);
	if (height != 13)
		DrawFlask(pBtmBuff, PANEL_WIDTH, PANEL_WIDTH * (height + 3) + 109, SCREENXY(PANEL_LEFT + 109, PANEL_TOP - 13 + height), 13 - height);
}

void DrawManaFlask()
{
	int filled, height;
	int maxMana, mana;

	if (gbRedrawFlags & REDRAW_MANA_FLASK) {
		maxMana = plr[myplr]._pMaxMana;
		mana = plr[myplr]._pMana;

		if (mana <= 0 || maxMana <= 0)
			filled = 0;
		else
			filled = 80 * mana / maxMana;
		if (filled > 80)
			filled = 80;
		plr[myplr]._pManaPer = filled;

		/**
		 * Draw the mana flask within the control panel.
		 * First sets the fill amount then draws the empty flask cel portion then the filled
		 * flask portion.
		 */
		height = filled;
		if (height > 69)
			height = 69;
		if (height != 69)
			SetFlaskHeight(pManaBuff, 16, 85 - height, PANEL_X + 464, PANEL_Y);
		if (height != 0)
			DrawPanelBox(464, 85 - height, 88, height, PANEL_X + 464, PANEL_Y + 69 - height);
	} else {
		filled = plr[myplr]._pManaPer;
	}

	/**
	 * Draw the top dome of the mana flask (that part that protrudes out of the control panel).
	 * First it draws the empty flask cel and then draws the filled part on top if needed.
	 */
	height = 80 - filled;
	if (height > 12)
		height = 12;
	height += 1;
	DrawFlask(pManaBuff, 88, 88 * 3 + 13, SCREENXY(PANEL_LEFT + 475, PANEL_TOP - 13), height);
	if (height != 13)
		DrawFlask(pBtmBuff, PANEL_WIDTH, PANEL_WIDTH * (height + 3) + 475, SCREENXY(PANEL_LEFT + 475, PANEL_TOP - 13 + height), 13 - height);
}

void InitControlPan()
{
	int i;
	BYTE *tBuff;

	if (gbMaxPlayers == 1) {
		pBtmBuff = DiabloAllocPtr((PANEL_HEIGHT + 16) * PANEL_WIDTH);
		memset(pBtmBuff, 0, (PANEL_HEIGHT + 16) * PANEL_WIDTH);
	} else {
		pBtmBuff = DiabloAllocPtr((PANEL_HEIGHT + 16) * 2 * PANEL_WIDTH);
		memset(pBtmBuff, 0, (PANEL_HEIGHT + 16) * 2 * PANEL_WIDTH);
	}
	pManaBuff = DiabloAllocPtr(88 * 88);
	memset(pManaBuff, 0, 88 * 88);
	pLifeBuff = DiabloAllocPtr(88 * 88);
	memset(pLifeBuff, 0, 88 * 88);
	pPanelText = LoadFileInMem("CtrlPan\\SmalText.CEL", NULL);
	pChrPanel = LoadFileInMem("Data\\Char.CEL", NULL);
#ifdef HELLFIRE
	pSpellCels = LoadFileInMem("Data\\SpelIcon.CEL", NULL);
#else
	pSpellCels = LoadFileInMem("CtrlPan\\SpelIcon.CEL", NULL);
#endif
	SetSpellTrans(RSPLTYPE_SKILL);
	tBuff = LoadFileInMem("CtrlPan\\Panel8.CEL", NULL);
	CelBlitWidth(pBtmBuff, 0, (PANEL_HEIGHT + 16) - 1, PANEL_WIDTH, tBuff, 1, PANEL_WIDTH);
	MemFreeDbg(tBuff);
	tBuff = LoadFileInMem("CtrlPan\\P8Bulbs.CEL", NULL);
	CelBlitWidth(pLifeBuff, 0, 87, 88, tBuff, 1, 88);
	CelBlitWidth(pManaBuff, 0, 87, 88, tBuff, 2, 88);
	MemFreeDbg(tBuff);
	talkflag = FALSE;
	if (gbMaxPlayers != 1) {
		tBuff = LoadFileInMem("CtrlPan\\TalkPanl.CEL", NULL);
		CelBlitWidth(pBtmBuff, 0, (PANEL_HEIGHT + 16) * 2 - 1, PANEL_WIDTH, tBuff, 1, PANEL_WIDTH);
		MemFreeDbg(tBuff);
		pMultiBtns = LoadFileInMem("CtrlPan\\P8But2.CEL", NULL);
		pTalkBtns = LoadFileInMem("CtrlPan\\TalkButt.CEL", NULL);
		sgbPlrTalkTbl = 0;
		sgszTalkMsg[0] = '\0';
		for (i = 0; i < lengthof(whisper); i++)
			whisper[i] = TRUE;
		for (i = 0; i < lengthof(talkbtndown); i++)
			talkbtndown[i] = FALSE;
	}
	panelflag = FALSE;
	lvlbtndown = FALSE;
	pPanelButtons = LoadFileInMem("CtrlPan\\Panel8bu.CEL", NULL);
	for (i = 0; i < lengthof(panbtn); i++)
		panbtn[i] = FALSE;
	panbtndown = FALSE;
	if (gbMaxPlayers == 1)
		numpanbtns = 6;
	else
		numpanbtns = 8;
	pChrButtons = LoadFileInMem("Data\\CharBut.CEL", NULL);
	for (i = 0; i < lengthof(chrbtn); i++)
		chrbtn[i] = FALSE;
	chrbtnactive = FALSE;
	pDurIcons = LoadFileInMem("Items\\DurIcons.CEL", NULL);
	infostr[0] = '\0';
	ClearPanel();
	gbRedrawFlags |= REDRAW_HP_FLASK | REDRAW_MANA_FLASK | REDRAW_SPEED_BAR;
	chrflag = FALSE;
	spselflag = FALSE;
	pSpellBkCel = LoadFileInMem("Data\\SpellBk.CEL", NULL);
	pSBkBtnCel = LoadFileInMem("Data\\SpellBkB.CEL", NULL);
	pSBkIconCels = LoadFileInMem("Data\\SpellI2.CEL", NULL);
	sbooktab = 0;
	sbookflag = FALSE;
	SpellPages[0][0] = Abilities[plr[myplr]._pClass];
	pQLogCel = LoadFileInMem("Data\\Quest.CEL", NULL);
	pGBoxBuff = LoadFileInMem("CtrlPan\\Golddrop.cel", NULL);
	dropGoldFlag = FALSE;
	dropGoldValue = 0;
	initialDropGoldValue = 0;
	initialDropGoldIndex = 0;
}

void DrawCtrlPan()
{
	DrawPanelBox(0, sgbPlrTalkTbl + 16, PANEL_WIDTH, PANEL_HEIGHT, PANEL_X, PANEL_Y);
	DrawInfoBox();
}

/**
 * Draws the control panel buttons in their current state. If the button is in the default
 * state draw it from the panel cel(extract its sub-rect). Else draw it from the buttons cel.
 */
void DrawCtrlBtns()
{
	int i;

	for (i = 0; i < 6; i++) {
		if (!panbtn[i])
			DrawPanelBox(PanBtnPos[i][0], PanBtnPos[i][1] + 16, 71, 20, PanBtnPos[i][0] + PANEL_X, PanBtnPos[i][1] + PANEL_Y);
		else
			CelDraw(PanBtnPos[i][0] + PANEL_X, PanBtnPos[i][1] + PANEL_Y + 18, pPanelButtons, i + 1, 71);
	}
	if (numpanbtns == 8) {
		CelDraw(87 + PANEL_X, 122 + PANEL_Y, pMultiBtns, panbtn[6] + 1, 33);
		CelDraw(527 + PANEL_X, 122 + PANEL_Y, pMultiBtns,
			panbtn[7] + FriendlyMode ? 3 : 5, 33);
	}
}

/**
 * Draws the "Speed Book": the rows of known spells for quick-setting a spell that
 * show up when you click the spell slot at the control panel.
 */
void DoSpeedBook()
{
	PlayerStruct *p;
	unsigned __int64 spell, spells;
	int xo, yo, X, Y, i, j;

	p = &plr[myplr];
	spselflag = TRUE;
	xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	yo = PANEL_Y - 17;
	X = xo - (BORDER_LEFT - SPLICONLENGTH / 2);
	Y = yo - (BORDER_TOP + SPLICONLENGTH / 2);
	if (p->_pRSpell != SPL_INVALID) {
		for (i = 0; i < 4; i++) {
			switch (i) {
			case RSPLTYPE_SKILL:
				spells = p->_pAblSpells;
				break;
			case RSPLTYPE_SPELL:
				spells = p->_pMemSpells;
				break;
			case RSPLTYPE_SCROLL:
				spells = p->_pScrlSpells;
				break;
			case RSPLTYPE_CHARGES:
				spells = p->_pISpells;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			for (spell = 1, j = 1; j < NUM_SPELLS; spell <<= 1, j++) {
				if (spell & spells) {
					if (j == p->_pRSpell && i == p->_pRSplType) {
						X = xo - (BORDER_LEFT - SPLICONLENGTH / 2);
						Y = yo - (BORDER_TOP + SPLICONLENGTH / 2);
					}
					xo -= SPLICONLENGTH;
					if (xo == PANEL_X + 12 - SPLICONLENGTH) {
						xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
						yo -= SPLICONLENGTH;
					}
				}
			}
			if (spells != 0 && xo != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS)
				xo -= SPLICONLENGTH;
			if (xo == PANEL_X + 12 - SPLICONLENGTH) {
				xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				yo -= SPLICONLENGTH;
			}
		}
	}

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (sgbControllerActive)
		SetCursorPos(X, Y);
#endif
}

static void control_set_button_down(int btn_id)
{
	panbtn[btn_id] = TRUE;
	gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
	panbtndown = TRUE;
}

/**
 * Checks if the mouse cursor is within any of the panel buttons and flag it if so.
 */
void DoPanBtn()
{
	int i, mx, my;

	mx = MouseX;
	my = MouseY;
	for (i = 0; i < numpanbtns; i++) {
		if (mx >= PanBtnPos[i][0] + PANEL_LEFT
		 && mx <= PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2]
		 && my >= PanBtnPos[i][1] + PANEL_TOP
		 && my <= PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3]) {
			control_set_button_down(i);
			return;
		}
	}
	if (mx >= 565 + PANEL_LEFT && mx < 621 + PANEL_LEFT && my >= 64 + PANEL_TOP && my < 120 + PANEL_TOP) {
		DoSpeedBook();
		gamemenu_off();
	}
}

void control_check_btn_press()
{
	if (MouseX >= PanBtnPos[3][0] + PANEL_LEFT
	 && MouseX <= PanBtnPos[3][0] + PANEL_LEFT + PanBtnPos[3][2]
	 && MouseY >= PanBtnPos[3][1] + PANEL_TOP
	 && MouseY <= PanBtnPos[3][1] + PANEL_TOP + PanBtnPos[3][3]) {
		control_set_button_down(3);
	}
	if (MouseX >= PanBtnPos[6][0] + PANEL_LEFT
	 && MouseX <= PanBtnPos[6][0] + PANEL_LEFT + PanBtnPos[6][2]
	 && MouseY >= PanBtnPos[6][1] + PANEL_TOP
	 && MouseY <= PanBtnPos[6][1] + PANEL_TOP + PanBtnPos[6][3]) {
		control_set_button_down(6);
	}
}

void DoAutoMap()
{
	if (currlevel != 0 || gbMaxPlayers != 1) {
		if (!automapflag)
			StartAutomap();
		else
			automapflag = FALSE;
	} else {
		InitDiabloMsg(EMSG_NO_AUTOMAP_IN_TOWN);
	}
}

/**
 * Checks the mouse cursor position within the control panel and sets information
 * strings if needed.
 */
void CheckPanelInfo()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i, c, sn;

	panelflag = FALSE;
	ClearPanel();
	for (i = 0; i < numpanbtns; i++) {
		if (MouseX >= PanBtnPos[i][0] + PANEL_LEFT
		 && MouseX <= PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2]
		 && MouseY >= PanBtnPos[i][1] + PANEL_TOP
		 && MouseY <= PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3]) {
			if (i != PANBTN_FRIENDLY) {
				copy_cstr(infostr, PanBtnStr[i]);
			} else {
				if (FriendlyMode)
					copy_cstr(infostr, "Player friendly");
				else
					copy_cstr(infostr, "Player attack");
			}
			if (PanBtnHotKey[i] != NULL) {
				snprintf(tempstr, sizeof(tempstr), "Hotkey : %s", PanBtnHotKey[i]);
				AddPanelString(tempstr, TRUE);
			}
			infoclr = COL_WHITE;
			panelflag = TRUE;
			pinfoflag = TRUE;
		}
	}
	if (!spselflag && MouseX >= 565 + PANEL_LEFT && MouseX < 621 + PANEL_LEFT && MouseY >= 64 + PANEL_TOP && MouseY < 120 + PANEL_TOP) {
		copy_cstr(infostr, "Select current spell button");
		infoclr = COL_WHITE;
		panelflag = TRUE;
		pinfoflag = TRUE;
		copy_cstr(tempstr, "Hotkey : 's'");
		AddPanelString(tempstr, TRUE);
		p = &plr[myplr];
		sn = p->_pRSpell;
			switch (p->_pRSplType) {
			case RSPLTYPE_SKILL:
				snprintf(tempstr, sizeof(tempstr), "%s Skill", spelldata[sn].sNameText);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_SPELL:
				snprintf(tempstr, sizeof(tempstr), "%s Spell", spelldata[sn].sNameText);
				AddPanelString(tempstr, TRUE);
				c = p->_pISplLvlAdd + p->_pSplLvl[sn];
				if (c <= 0)
					copy_cstr(tempstr, "Spell Level 0 - Unusable");
				else
					snprintf(tempstr, sizeof(tempstr), "Spell Level %i", c);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_SCROLL:
				snprintf(tempstr, sizeof(tempstr), "Scroll of %s", spelldata[sn].sNameText);
				AddPanelString(tempstr, TRUE);
				c = 0;
				pi = p->InvList;
				for (i = p->_pNumInv; i > 0; i--, pi++) {
					if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
					    && pi->_iSpell == sn) {
						c++;
					}
				}
				pi = p->SpdList;
				for (i = 0; i < MAXBELTITEMS; i++, pi++) {
					if (pi->_itype != ITYPE_NONE && pi->_iMiscId == IMISC_SCROLL
					    && pi->_iSpell == sn) {
						c++;
					}
				}
				if (c == 1)
					copy_cstr(tempstr, "1 Scroll");
				else
					snprintf(tempstr, sizeof(tempstr), "%i Scrolls", c);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_CHARGES:
				snprintf(tempstr, sizeof(tempstr), "Staff of %s", spelldata[sn].sNameText);
				AddPanelString(tempstr, TRUE);
				c = p->InvBody[INVLOC_HAND_LEFT]._iCharges;
				if (c == 1)
					copy_cstr(tempstr, "1 Charge");
				else
					snprintf(tempstr, sizeof(tempstr), "%i Charges", c);
				AddPanelString(tempstr, TRUE);
				break;
			case RSPLTYPE_INVALID:
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
	}
	if (MouseX > 190 + PANEL_LEFT && MouseX < 437 + PANEL_LEFT && MouseY > 4 + PANEL_TOP && MouseY < 33 + PANEL_TOP)
		pcursinvitem = CheckInvHLight();
}

/**
 * Check if the mouse is within a control panel button that's flagged.
 * Takes apropiate action if so.
 */
void CheckBtnUp()
{
	int i;
	BOOLEAN gamemenuOff;

	gamemenuOff = TRUE;
	gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
	panbtndown = FALSE;

	static_assert(lengthof(panbtn) == lengthof(PanBtnPos), "Mismatching panbtn and panbtnpos tables.");
	for (i = 0; i < lengthof(panbtn); i++) {
		if (!panbtn[i]) {
			continue;
		}

		panbtn[i] = FALSE;

		if (MouseX < PanBtnPos[i][0] + PANEL_LEFT
		 || MouseX > PanBtnPos[i][0] + PANEL_LEFT + PanBtnPos[i][2]
		 || MouseY < PanBtnPos[i][1] + PANEL_TOP
		 || MouseY > PanBtnPos[i][1] + PANEL_TOP + PanBtnPos[i][3]) {
			continue;
		}

		switch (i) {
		case PANBTN_CHARINFO:
			questlog = FALSE;
			plr[myplr]._pLvlUp = FALSE;
			chrflag = !chrflag;
			break;
		case PANBTN_QLOG:
			chrflag = FALSE;
			if (!questlog)
				StartQuestlog();
			else
				questlog = FALSE;
			break;
		case PANBTN_AUTOMAP:
			DoAutoMap();
			break;
		case PANBTN_MAINMENU:
			qtextflag = FALSE;
			gamemenu_handle_previous();
			gamemenuOff = FALSE;
			break;
		case PANBTN_INVENTORY:
			sbookflag = FALSE;
			invflag = !invflag;
			break;
		case PANBTN_SPELLBOOK:
			invflag = FALSE;
			sbookflag = !sbookflag;
			break;
		case PANBTN_SENDMSG:
			if (talkflag)
				control_reset_talk();
			else
				control_type_message();
			break;
		case PANBTN_FRIENDLY:
			FriendlyMode = !FriendlyMode;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}

	if (gamemenuOff)
		gamemenu_off();
}

void FreeControlPan()
{
	MemFreeDbg(pBtmBuff);
	MemFreeDbg(pManaBuff);
	MemFreeDbg(pLifeBuff);
	MemFreeDbg(pPanelText);
	MemFreeDbg(pChrPanel);
	MemFreeDbg(pSpellCels);
	MemFreeDbg(pPanelButtons);
	MemFreeDbg(pMultiBtns);
	MemFreeDbg(pTalkBtns);
	MemFreeDbg(pChrButtons);
	MemFreeDbg(pDurIcons);
	MemFreeDbg(pQLogCel);
	MemFreeDbg(pSpellBkCel);
	MemFreeDbg(pSBkBtnCel);
	MemFreeDbg(pSBkIconCels);
	MemFreeDbg(pGBoxBuff);
}

BOOL control_WriteStringToBuffer(BYTE *str)
{
	int k;

	k = 0;
	while (*str != '\0') {
		k += fontkern[fontframe[gbFontTransTbl[*str]]];
		str++;
		if (k >= 125)
			return FALSE;
	}

	return TRUE;
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
static void PrintString(int x, int y, int endX, const char *pszStr, BOOL center, int col, int kern)
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

static void PrintInfo()
{
	int i, j, x, y;

	if (!talkflag) {
		x = 177 + PANEL_LEFT + SCREEN_X;
		y = PANEL_TOP + SCREEN_Y;
		j = 0;
		const int *los = lineOffsets[pnumlines - 1];
		if (infostr[0] != '\0') {
			los = lineOffsets[pnumlines];
			PrintString(x, y + los[0], x + 287, infostr, TRUE, infoclr, 2);
			j = 1;
		}

		for (i = 0; i < pnumlines; i++, j++) {
			PrintString(x, y + los[j], x + 287, panelstr[i], pstrjust[i], infoclr, 2);
		}
	}
}

/**
 * Sets a string to be drawn in the info box and then draws it.
 */
void DrawInfoBox()
{
	PlayerStruct *p;
	ItemStruct *is;

	DrawPanelBox(177, 62, 288, 60, PANEL_X + 177, PANEL_Y + 46);
	if (!panelflag && !trigflag && pcursinvitem == -1 && !spselflag) {
		infostr[0] = '\0';
		infoclr = COL_WHITE;
		ClearPanel();
	}
	if (spselflag || trigflag) {
		infoclr = COL_WHITE;
	} else if (pcurs >= CURSOR_FIRSTITEM) {
		is = &plr[myplr].HoldItem;
		if (is->_itype == ITYPE_GOLD) {
			snprintf(infostr, sizeof(infostr), "%i gold %s", is->_ivalue, get_pieces_str(is->_ivalue));
		} else if (!is->_iStatFlag) {
			ClearPanel();
			AddPanelString("Requirements not met", TRUE);
			pinfoflag = TRUE;
		} else {
			if (is->_iIdentified)
				copy_str(infostr, is->_iIName);
			else
				copy_str(infostr, is->_iName);
			if (is->_iMagical == ITEM_QUALITY_MAGIC)
				infoclr = COL_BLUE;
			else if (is->_iMagical == ITEM_QUALITY_UNIQUE)
				infoclr = COL_GOLD;
		}
	} else {
		if (pcursitem != -1)
			GetItemStr(pcursitem);
		else if (pcursobj != -1)
			GetObjectStr(pcursobj);
		if (pcursmonst != -1) {
			if (leveltype != DTYPE_TOWN) {
				infoclr = COL_WHITE;
				strcpy(infostr, monster[pcursmonst].mName);
				ClearPanel();
				if (monster[pcursmonst]._uniqtype != 0) {
					infoclr = COL_GOLD;
					PrintUniqueHistory(pcursmonst);
				} else {
					PrintMonstHistory(monster[pcursmonst]._mType);
				}
			} else if (pcursitem == -1) {
				copy_str(infostr, towner[pcursmonst]._tName);
			}
		}
		if (pcursplr != -1) {
			infoclr = COL_GOLD;
			p = &plr[pcursplr];
			copy_str(infostr, p->_pName);
			ClearPanel();
#ifdef HELLFIRE
			snprintf(tempstr, sizeof(tempstr), "%s, Level : %i", ClassStrTbl[p->_pClass], p->_pLevel);
#else
			snprintf(tempstr, sizeof(tempstr), "Level : %i", p->_pLevel);
#endif
			AddPanelString(tempstr, TRUE);
			snprintf(tempstr, sizeof(tempstr), "Hit Points %i of %i", p->_pHitPoints >> 6, p->_pMaxHP >> 6);
			AddPanelString(tempstr, TRUE);
		}
	}
	if (infostr[0] != '\0' || pnumlines != 0)
		PrintInfo();
}

#define ADD_PlrStringXY(x, y, endX, pszStr, col) PrintString(x + SCREEN_X, y + SCREEN_Y, endX + SCREEN_X, pszStr, TRUE, col, 1)

void PrintGameStr(int x, int y, const char *str, int color)
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
	char col;
	char chrstr[64];
	int pc, val, mindam, maxdam;
	BOOL bow;

	p = &plr[myplr];
	pc = p->_pClass;
	bow = p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_BOW;

	CelDraw(SCREEN_X, 351 + SCREEN_Y, pChrPanel, 1, SPANEL_WIDTH);
	ADD_PlrStringXY(20, 32, 151, p->_pName, COL_WHITE);

	ADD_PlrStringXY(168, 32, 299, ClassStrTbl[pc], COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%i", p->_pLevel);
	ADD_PlrStringXY(66, 69, 109, chrstr, COL_WHITE);

	snprintf(chrstr, sizeof(chrstr), "%li", p->_pExperience);
	ADD_PlrStringXY(216, 69, 300, chrstr, COL_WHITE);

	if (p->_pLevel == MAXCHARLEVEL) {
		copy_cstr(chrstr, "None");
		col = COL_GOLD;
	} else {
		snprintf(chrstr, sizeof(chrstr), "%li", p->_pNextExper);
		col = COL_WHITE;
	}
	ADD_PlrStringXY(216, 97, 300, chrstr, col);

	snprintf(chrstr, sizeof(chrstr), "%i", p->_pGold);
	ADD_PlrStringXY(216, 146, 300, chrstr, COL_WHITE);

	val = p->_pIBonusAC;
	col = COL_WHITE;
	if (val > 0)
		col = COL_BLUE;
	if (val < 0)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val + p->_pIAC + p->_pDexterity / 5);
	ADD_PlrStringXY(258, 183, 301, chrstr, col);

	val = p->_pIBonusToHit;
	col = COL_WHITE;
	if (val > 0)
		col = COL_BLUE;
	if (val < 0)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i%%", val + (p->_pDexterity >> 1) + 50);
	ADD_PlrStringXY(258, 211, 301, chrstr, col);

	val = p->_pIBonusDam;
	col = COL_WHITE;
	if (val > 0)
		col = COL_BLUE;
	if (val < 0)
		col = COL_RED;
	mindam = p->_pIMinDam;
	mindam += val * mindam / 100;
	maxdam = p->_pIMaxDam;
	maxdam += val * maxdam / 100;
	mindam += p->_pIBonusDamMod;
	maxdam += p->_pIBonusDamMod;
	if (bow && pc != PC_ROGUE) {
		mindam += p->_pDamageMod >> 1;
		maxdam += p->_pDamageMod >> 1;
	} else {
		mindam += p->_pDamageMod;
		maxdam += p->_pDamageMod;
	}
	snprintf(chrstr, sizeof(chrstr), "%i-%i", mindam, maxdam);
	if (mindam >= 100 || maxdam >= 100)
		PrintString(254 + SCREEN_X, 239 + SCREEN_Y, 305 + SCREEN_X, chrstr, TRUE, col, -1);
	else
		PrintString(258 + SCREEN_X, 239 + SCREEN_Y, 301 + SCREEN_X, chrstr, TRUE, col, 0);

	val = p->_pMagResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(257, 276, 300, chrstr, col);

	val = p->_pFireResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(257, 304, 300, chrstr, col);

	val = p->_pLghtResist;
	if (val < MAXRESIST) {
		col = val == 0 ? COL_WHITE : COL_BLUE;
		snprintf(chrstr, sizeof(chrstr), "%i%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	ADD_PlrStringXY(257, 332, 300, chrstr, col);

	col = COL_WHITE;
	if (MaxStats[pc][ATTRIB_STR] == p->_pBaseStr)
		col = COL_GOLD;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseStr);
	ADD_PlrStringXY(95, 155, 126, chrstr, col);

	col = COL_WHITE;
	if (MaxStats[pc][ATTRIB_MAG] == p->_pBaseMag)
		col = COL_GOLD;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseMag);
	ADD_PlrStringXY(95, 183, 126, chrstr, col);

	col = COL_WHITE;
	if (MaxStats[pc][ATTRIB_DEX] == p->_pBaseDex)
		col = COL_GOLD;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseDex);
	ADD_PlrStringXY(95, 211, 126, chrstr, col);

	col = COL_WHITE;
	if (MaxStats[pc][ATTRIB_VIT] == p->_pBaseVit)
		col = COL_GOLD;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pBaseVit);
	ADD_PlrStringXY(95, 239, 126, chrstr, col);

	val = p->_pStrength;
	col = COL_WHITE;
	if (val > p->_pBaseStr)
		col = COL_BLUE;
	else if (val < p->_pBaseStr)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(143, 155, 173, chrstr, col);

	val = p->_pMagic;
	col = COL_WHITE;
	if (val > p->_pBaseMag)
		col = COL_BLUE;
	else if (val < p->_pBaseMag)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(143, 183, 173, chrstr, col);

	val = p->_pDexterity;
	col = COL_WHITE;
	if (val > p->_pBaseDex)
		col = COL_BLUE;
	else if (val < p->_pBaseDex)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(143, 211, 173, chrstr, col);

	val = p->_pVitality;
	col = COL_WHITE;
	if (val > p->_pBaseVit)
		col = COL_BLUE;
	else if (val < p->_pBaseVit)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", val);
	ADD_PlrStringXY(143, 239, 173, chrstr, col);

	if (p->_pStatPts > 0) {
		snprintf(chrstr, sizeof(chrstr), "%i", p->_pStatPts);
		ADD_PlrStringXY(95, 266, 126, chrstr, COL_RED);
		if (p->_pBaseStr < MaxStats[pc][ATTRIB_STR])
			CelDraw(137 + SCREEN_X, 159 + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_STR] + 2, 41);
		if (p->_pBaseMag < MaxStats[pc][ATTRIB_MAG])
			CelDraw(137 + SCREEN_X, 187 + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_MAG] + 4, 41);
		if (p->_pBaseDex < MaxStats[pc][ATTRIB_DEX])
			CelDraw(137 + SCREEN_X, 216 + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_DEX] + 6, 41);
		if (p->_pBaseVit < MaxStats[pc][ATTRIB_VIT])
			CelDraw(137 + SCREEN_X, 244 + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_VIT] + 8, 41);
	}

	val = p->_pMaxHP;
	col = val <= p->_pMaxHPBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%i", val >> 6);
	ADD_PlrStringXY(95, 304, 126, chrstr, col);
	if (p->_pHitPoints != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pHitPoints >> 6);
	ADD_PlrStringXY(143, 304, 174, chrstr, col);

	val = p->_pMaxMana;
	col = val <= p->_pMaxManaBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%i", val >> 6);
	ADD_PlrStringXY(95, 332, 126, chrstr, col);
	if (p->_pMana != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%i", p->_pMana >> 6);
	ADD_PlrStringXY(143, 332, 174, chrstr, col);
}

BOOL CheckLvlBtn()
{
	if (plr[myplr]._pLvlUp && MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		lvlbtndown = TRUE;
	return lvlbtndown;
}

void ReleaseLvlBtn()
{
	if (MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP) {
		chrflag = TRUE;
		plr[myplr]._pLvlUp = FALSE;
	}
	lvlbtndown = FALSE;
}

void DrawLevelUpIcon()
{
	ADD_PlrStringXY(PANEL_LEFT + 0, PANEL_TOP - 49, PANEL_LEFT + 120, "Level Up", COL_WHITE);
	CelDraw(40 + PANEL_X, -17 + PANEL_Y, pChrButtons, lvlbtndown + 2, 41);
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

	if (!zoomflag) {
		px <<= 1;
		py <<= 1;
	}

	if (PANELS_COVER) {
		if (chrflag | questlog) {
			px += SPANEL_WIDTH / 2;
		} else if (invflag | sbookflag) {
			px -= SPANEL_WIDTH / 2;
		}
	}

	px += SCREEN_WIDTH / 2;
	py += PANEL_TOP / 2;

	*outx = px;
	*outy = py;
}

char DrawItemColor(ItemStruct *is)
{
	if (is->_iMagical == ITEM_QUALITY_NORMAL)
		return COL_WHITE;
	return is->_iMagical == ITEM_QUALITY_UNIQUE ? COL_GOLD : COL_BLUE;
}

void DrawInfoStr()
{
	int x, y, xx, yy, width;
	const char* text;
	BYTE *dst;
	char col;

	if (pcursitem != -1) {
		ItemStruct* is = &item[pcursitem];
		x = is->_ix;
		y = is->_iy;
		col = DrawItemColor(is);
	} else if (pcursobj != -1) {
		ObjectStruct* os = &object[pcursobj];
		x = os->_ox;
		y = os->_oy;
		col = COL_WHITE;
	} else if (pcursmonst != -1) {
		MonsterStruct* mon = &monster[pcursmonst];
		x = mon->_mx - 2;
		y = mon->_my - 2;
		col = COL_WHITE;
	} else if (pcursplr != -1) {
		PlayerStruct* p = &plr[pcursplr];
		x = p->_px;
		y = p->_py;
		col = COL_WHITE;
	} else {
		return;
	}
	text = infostr;

	GetMousePos(x, y, &xx, &yy);
	const int border = 4, height = 16;

	width = StringWidth(text) + 2 * border;

	yy -= TILE_HEIGHT;
	if (yy < 0)
		return;
	xx -= width / 2;
	if (xx < 0)
		xx = 0;
	else if (xx > SCREEN_WIDTH - width)
		xx = SCREEN_WIDTH - width;

	// draw gray border
	dst = &gpBuffer[SCREENXY(xx, yy)];
	for (int i = 0; i < height; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_GRAY + 2, width);

	// draw background
	dst = &gpBuffer[SCREENXY(xx + 1, yy + 1)];
	for (int i = 0; i < height - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_ORANGE + 14, width - 2);

	// print the info
	PrintGameStr(xx + border, yy + height - 3, text, col);
}


BOOL CheckChrBtns()
{
	PlayerStruct *p;
	int i;

	if (plr[myplr]._pStatPts != 0 && !chrbtnactive) {
		for (i = 0; i < lengthof(ChrBtnsRect); i++) {
			if (MouseX < ChrBtnsRect[i].x
			 || MouseX > ChrBtnsRect[i].x + ChrBtnsRect[i].w
			 || MouseY < ChrBtnsRect[i].y
			 || MouseY > ChrBtnsRect[i].y + ChrBtnsRect[i].h)
				continue;

			p = &plr[myplr];
			switch (i) {
			case 0:
				if (p->_pBaseStr >= MaxStats[p->_pClass][ATTRIB_STR])
					return FALSE;
				break;
			case 1:
				if (p->_pBaseMag >= MaxStats[p->_pClass][ATTRIB_MAG])
					return FALSE;
				break;
			case 2:
				if (p->_pBaseDex >= MaxStats[p->_pClass][ATTRIB_DEX])
					return FALSE;
				break;
			case 3:
				if (p->_pBaseVit >= MaxStats[p->_pClass][ATTRIB_VIT])
					return FALSE;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			chrbtn[i] = TRUE;
			chrbtnactive = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

void ReleaseChrBtns()
{
	int i;

	chrbtnactive = FALSE;
	static_assert(lengthof(chrbtn) == lengthof(ChrBtnsRect), "Mismatching chrbtn and ChrBtnsRect tables.");
	static_assert(lengthof(chrbtn) == 4, "Table chrbtn does not work with ReleaseChrBtns function.");
	for (i = 0; i < lengthof(chrbtn); ++i) {
		if (chrbtn[i]) {
			chrbtn[i] = FALSE;
			if (MouseX >= ChrBtnsRect[i].x
			 && MouseX <= ChrBtnsRect[i].x + ChrBtnsRect[i].w
			 && MouseY >= ChrBtnsRect[i].y
			 && MouseY <= ChrBtnsRect[i].y + ChrBtnsRect[i].h) {
				switch (i) {
				case 0:
					NetSendCmdParam1(TRUE, CMD_ADDSTR, 1);
					break;
				case 1:
					NetSendCmdParam1(TRUE, CMD_ADDMAG, 1);
					break;
				case 2:
					NetSendCmdParam1(TRUE, CMD_ADDDEX, 1);
					break;
				case 3:
					NetSendCmdParam1(TRUE, CMD_ADDVIT, 1);
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
	CelDraw(x, -17 + PANEL_Y, pDurIcons, c, 32);
	return x - 32 - 8;
}

void DrawDurIcon()
{
	ItemStruct *inv;
	int x;

	bool hasRoomBetweenPanels = SCREEN_WIDTH >= PANEL_WIDTH + 16 + (32 + 8 + 32 + 8 + 32 + 8 + 32) + 16;
	bool hasRoomUnderPanels = SCREEN_HEIGHT >= SPANEL_HEIGHT + PANEL_HEIGHT + 16 + 32 + 16;

	if (!hasRoomBetweenPanels && !hasRoomUnderPanels) {
		if ((chrflag || questlog) && (invflag || sbookflag))
			return;
	}

	x = PANEL_X + PANEL_WIDTH - 32 - 16;
	if (!hasRoomUnderPanels) {
		if (invflag || sbookflag)
			x -= SPANEL_WIDTH - (SCREEN_WIDTH - PANEL_WIDTH) / 2;
	}

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

static char GetSBookTrans(int sn, BOOL townok)
{
	PlayerStruct *p;
	char st;

	p = &plr[myplr];
	if (p->_pAblSpells & SPELL_MASK(sn)) { /// BUGFIX: missing (__int64) (fixed)
		st = RSPLTYPE_SKILL;
	} else if (p->_pISpells & SPELL_MASK(sn)) {
		st = RSPLTYPE_CHARGES;
	} else if (CheckSpell(myplr, sn)
	 && (p->_pSplLvl[sn] + p->_pISplLvlAdd) > 0) {
		st = RSPLTYPE_SPELL;
	} else {
		return RSPLTYPE_INVALID;
	}
	if (townok && currlevel == 0 && !spelldata[sn].sTownSpell) {
		st = RSPLTYPE_INVALID;
	}
	return st;
}

#define SBOOK_CELSIZE		37
#define SBOOK_CELBORDER		 3
#define SBOOK_TOP_BORDER	15
#define SBOOK_LEFT_BORDER	 7
#define SBOOK_BOTTOM_BORDER	 3
#define SBOOK_PAGER_HEIGHT	29
#define SBOOK_LINE_LENGTH  222
#ifdef HELLFIRE
#define SBOOK_PAGER_WIDTH	61
#else
#define SBOOK_PAGER_WIDTH	76
#endif

void DrawSpellBook()
{
	PlayerStruct* p;
	int i, sn, mana, lvl, sx, yp, offset, min, max;
	char st;
	unsigned __int64 spl;

	// back panel
	CelDraw(RIGHT_PANEL_X, SCREEN_Y + SPANEL_HEIGHT - 1, pSpellBkCel, 1, SPANEL_WIDTH);
	// selected pager
	CelDraw(RIGHT_PANEL_X + SBOOK_LEFT_BORDER + SBOOK_PAGER_WIDTH * sbooktab, SCREEN_Y + SPANEL_HEIGHT - 1 - SBOOK_BOTTOM_BORDER, pSBkBtnCel, sbooktab + 1, SBOOK_PAGER_WIDTH);

	p = &plr[myplr];
	spl = p->_pMemSpells | p->_pISpells | p->_pAblSpells;

	yp = SCREEN_Y + SBOOK_TOP_BORDER + SBOOK_CELBORDER + SBOOK_CELSIZE;
	sx = RIGHT_PANEL_X + SBOOK_LEFT_BORDER + SBOOK_CELBORDER;
	for (i = 0; i < lengthof(SpellPages[sbooktab]); i++) {
		sn = SpellPages[sbooktab][i];
		if (sn != SPL_INVALID && (spl & SPELL_MASK(sn))) {
			st = GetSBookTrans(sn, TRUE);
			SetSpellTrans(st);
			DrawSpellCel(sx + 1, yp, pSBkIconCels, SpellITbl[sn], SBOOK_CELSIZE);
			if (sn == p->_pRSpell && st == p->_pRSplType) {
				SetSpellTrans(RSPLTYPE_SKILL);
				DrawSpellCel(sx + 1, yp, pSBkIconCels, SPLICONLAST, SBOOK_CELSIZE);
			}
			switch (GetSBookTrans(sn, FALSE)) {
			case RSPLTYPE_SKILL:
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
				if (lvl <= 0) {
					copy_cstr(tempstr, "Spell Level 0 - Unusable");
				} else {
					snprintf(tempstr, sizeof(tempstr), "Spell Level %i", lvl);
				}
				mana = GetManaAmount(myplr, sn) >> 6;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			GetDamageAmt(sn, &min, &max);
			offset = mana == 0 && min == -1 && sn != SPL_BONESPIRIT ? 5 : 0;
			PrintString(sx + SPLICONLENGTH, yp - 23 + offset, sx + SPLICONLENGTH + SBOOK_LINE_LENGTH, spelldata[sn].sNameText, FALSE, COL_WHITE, 1);
			PrintString(sx + SPLICONLENGTH, yp - 12 + offset, sx + SPLICONLENGTH + SBOOK_LINE_LENGTH, tempstr, FALSE, COL_WHITE, 1);

			if (offset == 0) {
				if (mana != 0)
					cat_str(tempstr, offset, "Mana: %i  ", mana);
				if (min != -1)
					cat_str(tempstr, offset, "Dam: %i - %i", min, max);
				else if (sn == SPL_BONESPIRIT)
					cat_cstr(tempstr, offset, "Dam: 1/3 tgt hp");
				PrintString(sx + SPLICONLENGTH, yp - 1, sx + SPLICONLENGTH + SBOOK_LINE_LENGTH, tempstr, FALSE, COL_WHITE, 1);
			}
		}
		yp += 2 * SBOOK_CELBORDER + SBOOK_CELSIZE;
	}
}

void CheckSBook()
{
	PlayerStruct *p;
	int sn;
	char st;
	unsigned __int64 spl;

	int dx = MouseX - (RIGHT_PANEL + SBOOK_LEFT_BORDER + 1);
	if (dx < 0)
		return;
	int dy = MouseY - SBOOK_TOP_BORDER;
	if (dy < 0)
		return;

	if (dy < lengthof(SpellPages[sbooktab]) * (2 * SBOOK_CELBORDER + SBOOK_CELSIZE)) {
		if (dx < 2 * SBOOK_CELBORDER + SBOOK_CELSIZE) {
			sn = SpellPages[sbooktab][dy / (2 * SBOOK_CELBORDER + SBOOK_CELSIZE)];
			p = &plr[myplr];
			spl = p->_pMemSpells | p->_pISpells | p->_pAblSpells;
			if (sn != SPL_INVALID && spl & SPELL_MASK(sn)) {
				if (p->_pAblSpells & SPELL_MASK(sn))
					st = RSPLTYPE_SKILL;
				else if (p->_pISpells & SPELL_MASK(sn))
					st = RSPLTYPE_CHARGES;
				else
					st = RSPLTYPE_SPELL;
				p->_pRSpell = sn;
				p->_pRSplType = st;
				gbRedrawFlags = REDRAW_ALL;
			}
		}
	} else {
		dy -= SPANEL_HEIGHT - SBOOK_TOP_BORDER - SBOOK_BOTTOM_BORDER - SBOOK_PAGER_HEIGHT;
		if (dy >= 0 && dy < SBOOK_PAGER_HEIGHT) {
			dx /= SBOOK_PAGER_WIDTH;
			if (dx < lengthof(SpellPages))
				sbooktab = dx;
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

void control_drop_gold(char vkey)
{
	char input[6];

	if (plr[myplr]._pHitPoints >> 6 <= 0) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
		return;
	}

	memset(input, 0, sizeof(input));
	snprintf(input, sizeof(input), "%d", dropGoldValue);
	if (vkey == DVL_VK_RETURN) {
		if (dropGoldValue > 0)
			control_remove_gold(myplr, initialDropGoldIndex);
		dropGoldFlag = FALSE;
	} else if (vkey == DVL_VK_ESCAPE) {
		dropGoldFlag = FALSE;
		dropGoldValue = 0;
	} else if (vkey == DVL_VK_BACK) {
		input[strlen(input) - 1] = '\0';
		dropGoldValue = atoi(input);
	} else if (vkey - '0' >= 0 && vkey - '0' <= 9) {
		if (dropGoldValue != 0 || atoi(input) <= initialDropGoldValue) {
			input[strlen(input)] = vkey;
			if (atoi(input) > initialDropGoldValue)
				return;
			if (strlen(input) > strlen(input))
				return;
		} else {
			input[0] = vkey;
		}
		dropGoldValue = atoi(input);
	}
}

void control_remove_gold(int pnum, int gold_index)
{
	ItemStruct *is;
	int gi, val;

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
	SetGoldItemValue(is, dropGoldValue);
	NewCursor(is->_iCurs + CURSOR_FIRSTITEM);
	CalculateGold(pnum);
	dropGoldValue = 0;
}

static char *control_print_talk_msg(char *msg, int *x, int y, int color)
{
	BYTE c;
	int width;

	*x += 200 + SCREEN_X;
	y += 22 + PANEL_Y;
	width = *x;
	while (*msg != '\0') {
		c = gbFontTransTbl[(BYTE)*msg];
		c = fontframe[c];
		width += fontkern[c] + 1;
		if (width > 450 + PANEL_X)
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
	int i, off, talk_btn, color, nCel, x;
	char *msg;

	if (!talkflag)
		return;

	DrawPanelBox(175, sgbPlrTalkTbl + 20, 294, 5, PANEL_X + 175, PANEL_Y + 4);
	off = 0;
	for (i = 293; i > 283; off++, i--) {
		DrawPanelBox((off >> 1) + 175, sgbPlrTalkTbl + off + 25, i, 1, (off >> 1) + PANEL_X + 175, off + PANEL_Y + 9);
	}
	DrawPanelBox(185, sgbPlrTalkTbl + 35, 274, 30, PANEL_X + 185, PANEL_Y + 19);
	DrawPanelBox(180, sgbPlrTalkTbl + 65, 284, 5, PANEL_X + 180, PANEL_Y + 49);
	for (i = 0; i < 10; i++) {
		DrawPanelBox(180, sgbPlrTalkTbl + i + 70, i + 284, 1, PANEL_X + 180, i + PANEL_Y + 54);
	}
	DrawPanelBox(170, sgbPlrTalkTbl + 80, 310, 55, PANEL_X + 170, PANEL_Y + 64);
	msg = sgszTalkMsg;
	for (i = 0; i < 39; i += 13) {
		x = 0 + PANEL_LEFT;
		msg = control_print_talk_msg(msg, &x, i, COL_WHITE);
		if (msg == NULL)
			break;
	}
	if (msg != NULL)
		*msg = '\0';
	CelDraw(x, i + 22 + PANEL_Y, pSPentSpn2Cels, PentSpn2Spin(), 12);
	talk_btn = 0;
	static_assert(lengthof(whisper) == MAX_PLRS, "Table whisper does not work with the current MAX_PLRS in DrawTalkPan.");
	for (i = 0; i < MAX_PLRS; i++) {
		if (i == myplr)
			continue;
		if (whisper[i]) {
			color = COL_GOLD;
			if (talkbtndown[talk_btn]) {
				if (talk_btn != 0)
					nCel = 4;
				else
					nCel = 3;
				CelDraw(172 + PANEL_X, 84 + 18 * talk_btn + PANEL_Y, pTalkBtns, nCel, 61);
			}
		} else {
			color = COL_RED;
			if (talk_btn != 0)
				nCel = 2;
			else
				nCel = 1;
			if (talkbtndown[talk_btn])
				nCel += 4;
			CelDraw(172 + PANEL_X, 84 + 18 * talk_btn + PANEL_Y, pTalkBtns, nCel, 61);
		}
		if (plr[i].plractive) {
			x = 46 + PANEL_LEFT;
			control_print_talk_msg(plr[i]._pName, &x, 60 + talk_btn * 18, color);
		}

		talk_btn++;
	}
}

BOOL control_check_talk_btn()
{
	int i;

	if (MouseX < 172 + PANEL_LEFT)
		return FALSE;
	if (MouseY < 69 + PANEL_TOP)
		return FALSE;
	if (MouseX > 233 + PANEL_LEFT)
		return FALSE;
	if (MouseY > 123 + PANEL_TOP)
		return FALSE;

	for (i = 0; i < lengthof(talkbtndown); i++) {
		talkbtndown[i] = FALSE;
	}

	talkbtndown[(MouseY - (69 + PANEL_TOP)) / 18] = TRUE;

	return TRUE;
}

void control_release_talk_btn()
{
	int i, p, off;

	for (i = 0; i < lengthof(talkbtndown); i++)
		talkbtndown[i] = FALSE;
	if (MouseX >= 172 + PANEL_LEFT && MouseY >= 69 + PANEL_TOP && MouseX <= 233 + PANEL_LEFT && MouseY <= 123 + PANEL_TOP) {
		off = (MouseY - (69 + PANEL_TOP)) / 18;

		for (p = 0; p < MAX_PLRS && off != -1; p++) {
			if (p != myplr)
				off--;
		}
		if (p <= MAX_PLRS)
			whisper[p - 1] = !whisper[p - 1];
	}
}

void control_type_message()
{
	int i;

	if (gbMaxPlayers == 1) {
		return;
	}

	talkflag = TRUE;
	sgszTalkMsg[0] = '\0';
	for (i = 0; i < lengthof(talkbtndown); i++) {
		talkbtndown[i] = FALSE;
	}
	sgbPlrTalkTbl = PANEL_HEIGHT + 16;
	gbRedrawFlags = REDRAW_ALL;
	sgbTalkSavePos = sgbNextTalkSave;
}

void control_reset_talk()
{
	talkflag = FALSE;
	sgbPlrTalkTbl = 0;
	gbRedrawFlags = REDRAW_ALL;
}

static void control_press_enter()
{
	int i, pmask;
	BYTE talk_save;

	if (sgszTalkMsg[0] != '\0') {
		pmask = 0;

		static_assert(lengthof(whisper) == MAX_PLRS, "Table whisper does not work with the current MAX_PLRS in control_press_enter.");
		for (i = 0; i < MAX_PLRS; i++) {
			if (whisper[i])
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

BOOL control_talk_last_key(int vkey)
{
	int result;

	if (gbMaxPlayers == 1)
		return FALSE;

	if (!talkflag)
		return FALSE;

	if ((DWORD)vkey < DVL_VK_SPACE)
		return FALSE;

	result = strlen(sgszTalkMsg);
	if (result < 78) {
		sgszTalkMsg[result] = vkey;
		sgszTalkMsg[result + 1] = '\0';
	}
	return TRUE;
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

BOOL control_presskeys(int vkey)
{
	int len;

	if (gbMaxPlayers == 1 || !talkflag)
		return FALSE;
	if (vkey == DVL_VK_SPACE) {
	} else if (vkey == DVL_VK_ESCAPE) {
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
	} else {
		return FALSE;
	}
	return TRUE;
}

DEVILUTION_END_NAMESPACE
