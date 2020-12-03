/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
#define SBOOK_PAGER_WIDTH 61
#else
#define SBOOK_PAGER_WIDTH 76
#endif

BYTE sgbNextTalkSave;
BYTE sgbTalkSavePos;
BYTE *pDurIcons;
BYTE *pChrButtons;
BOOL drawhpflag;
BOOL dropGoldFlag;
BOOL panbtn[8];
BOOL chrbtn[4];
BYTE *pMultiBtns;
BYTE *pPanelButtons;
BYTE *pChrPanel;
BOOL lvlbtndown;
char sgszTalkSave[8][80];
int dropGoldValue;
BOOL drawmanaflag;
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
BOOL drawbtnflag;
BYTE *pSpellBkCel;
char infostr[256];
int numpanbtns;
BYTE *pStatusPanel;
char panelstr[MAX_CTRL_PANEL_LINES][64];
BOOL panelflag;
BYTE SplTransTbl[256];
int initialDropGoldValue;
BYTE *pSpellCels;
BOOL panbtndown;
BYTE *pTalkPanel;
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
char SpellITbl[MAX_SPELLS] = {
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
int SpellPages[6][7] = {
	{ SPL_NULL, SPL_FIREBOLT, SPL_CBOLT, SPL_HBOLT, SPL_HEAL, SPL_HEALOTHER, SPL_FLAME },
	{ SPL_RESURRECT, SPL_FIREWALL, SPL_TELEKINESIS, SPL_LIGHTNING, SPL_TOWN, SPL_FLASH, SPL_STONE },
	{ SPL_RNDTELEPORT, SPL_MANASHIELD, SPL_ELEMENT, SPL_FIREBALL, SPL_WAVE, SPL_CHAIN, SPL_GUARDIAN },
	{ SPL_NOVA, SPL_GOLEM, SPL_TELEPORT, SPL_APOCA, SPL_BONESPIRIT, SPL_FLARE, SPL_ETHEREALIZE },
#ifdef HELLFIRE
	{ SPL_LIGHTWALL, SPL_IMMOLAT, SPL_WARP, SPL_REFLECT, SPL_BERSERK, SPL_FIRERING, SPL_SEARCH },
#else
	{ -1, -1, -1, -1, -1, -1, -1 },
#endif
	{ -1, -1, -1, -1, -1, -1, -1 }
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

	if (st == RSPLTYPE_SKILL) {
		for (i = 0; i < 128; i++)
			SplTransTbl[i] = i;
	}
	for (i = 128; i < 256; i++)
		SplTransTbl[i] = i;
	SplTransTbl[255] = 0;

	switch (st) {
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
	}
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
static void DrawSpell()
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
		if (tlvl <= 0 || !CheckSpell(myplr, spl, RSPLTYPE_SPELL, TRUE))
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
		}
		for (spl = 1, j = 1; j < MAX_SPELLS; spl <<= 1, j++) {
			if (!(mask & spl))
				continue;
			if (i == RSPLTYPE_SPELL) {
				s = p->_pISplLvlAdd + p->_pSplLvl[j];
				SetSpellTrans(s > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID);
			}
			if (currlevel == 0 && !spelldata[j].sTownSpell)
				SetSpellTrans(RSPLTYPE_INVALID);
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
					snprintf(infostr, sizeof(infostr), "%s Skill", spelldata[j].sSkillText);
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
						if (pi->_itype != ITYPE_NONE
						    && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
						    && pi->_iSpell == j) {
							v++;
						}
					}
					pi = p->SpdList;
					for (t = MAXBELTITEMS; t > 0; t--, pi++) {
						if (pi->_itype != ITYPE_NONE
						    && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
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
		force_redraw = 255;
	}
}

void SetSpeedSpell(int slot)
{
	PlayerStruct *p;
	int i;

	if (pSpell != SPL_INVALID) {
		p = &plr[myplr];
		for (i = 0; i < lengthof(p->_pSplHotKey); ++i) {
			if (p->_pSplHotKey[i] == pSpell && p->_pSplTHotKey[i] == pSplType)
				p->_pSplHotKey[i] = SPL_INVALID;
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
	if (p->_pSplHotKey[slot] == SPL_INVALID) {
		return;
	}

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
	}

	if (spells & (__int64)1 << (p->_pSplHotKey[slot] - 1)) {
		p->_pRSpell = p->_pSplHotKey[slot];
		p->_pRSplType = p->_pSplTHotKey[slot];
		force_redraw = 255;
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
	default:
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

	for (hgt = h; hgt; hgt--, src += PANEL_WIDTH, dst += BUFFER_WIDTH) {
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

	for (; w; w--, src += 88, dst += BUFFER_WIDTH)
		memcpy(dst, src, 88);
}

/**
 * Draws the dome of the flask that protrudes above the panel top line.
 * It draws a rectangle of fixed width 59 and height 'h' from the source buffer
 * into the target buffer.
 * @param pCelBuff The flask cel buffer.
 * @param w Width of the cel.
 * @param nSrcOff Offset of the source buffer from where the bytes will start to be copied from.
 * @param pBuff Target buffer.
 * @param nDstOff Offset of the target buffer where the bytes will start to be copied to.
 * @param h How many lines of the source buffer that will be copied.
 */
static void DrawFlask(BYTE *pCelBuff, int w, int nSrcOff, BYTE *pBuff, int nDstOff, int h)
{
	int wdt, hgt;
	BYTE *src, *dst;

	src = &pCelBuff[nSrcOff];
	dst = &pBuff[nDstOff];

	for (hgt = h; hgt; hgt--, src += w - 59, dst += BUFFER_WIDTH - 59) {
		for (wdt = 59; wdt; wdt--) {
			if (*src)
				*dst = *src;
			src++;
			dst++;
		}
	}
}

/**
 * Draws the top dome of the life flask (that part that protrudes out of the control panel).
 * First it draws the empty flask cel and then draws the filled part on top if needed.
 */
void DrawLifeFlask()
{
	int filled = plr[myplr]._pHPPer;

	if (filled > 80)
		filled = 80;

	filled = 80 - filled;
	if (filled > 11)
		filled = 11;
	filled += 2;

	DrawFlask(pLifeBuff, 88, 88 * 3 + 13, gpBuffer, SCREENXY(PANEL_LEFT + 109, PANEL_TOP - 13), filled);
	if (filled != 13)
		DrawFlask(pBtmBuff, PANEL_WIDTH, PANEL_WIDTH * (filled + 3) + 109, gpBuffer, SCREENXY(PANEL_LEFT + 109, PANEL_TOP - 13 + filled), 13 - filled);
}

/**
 * Controls the drawing of the area of the life flask within the control panel.
 * First sets the fill amount then draws the empty flask cel portion then the filled
 * flask portion.
 */
void UpdateLifeFlask()
{
	int filled;
	int maxHP = plr[myplr]._pMaxHP;
	int hp = plr[myplr]._pHitPoints;

	if (hp <= 0 || maxHP <= 0)
		filled = 0;
	else
		filled = 80 * hp / maxHP;
	plr[myplr]._pHPPer = filled;

	if (filled > 69)
		filled = 69;
	if (filled != 69)
		SetFlaskHeight(pLifeBuff, 16, 85 - filled, 96 + PANEL_X, PANEL_Y);
	if (filled != 0)
		DrawPanelBox(96, 85 - filled, 88, filled, 96 + PANEL_X, PANEL_Y + 69 - filled);
}

void DrawManaFlask()
{
	int filled = plr[myplr]._pManaPer;

	if (filled > 80)
		filled = 80;

	filled = 80 - filled;
	if (filled > 11)
		filled = 11;
	filled += 2;

	DrawFlask(pManaBuff, 88, 88 * 3 + 13, gpBuffer, SCREENXY(PANEL_LEFT + 475, PANEL_TOP - 13), filled);
	if (filled != 13)
		DrawFlask(pBtmBuff, PANEL_WIDTH, PANEL_WIDTH * (filled + 3) + 475, gpBuffer, SCREENXY(PANEL_LEFT + 475, PANEL_TOP - 13 + filled), 13 - filled);
}

void control_update_life_mana()
{
	int per;
	int maxVal = plr[myplr]._pMaxMana;
	int val = plr[myplr]._pMana;

	if (val <= 0 || maxVal <= 0)
		per = 0;
	else
		per = 80 * val / maxVal;
	plr[myplr]._pManaPer = per;

	maxVal = plr[myplr]._pMaxHP;
	val = plr[myplr]._pHitPoints;

	if (val <= 0 || maxVal <= 0)
		per = 0;
	else
		per = 80 * val / maxVal;
	plr[myplr]._pHPPer = per;
}

/**
 * Controls the drawing of the area of the life flask within the control panel.
 * Also for some reason draws the current right mouse button spell.
 */
void UpdateManaFlask()
{
	int filled;
	int maxMana = plr[myplr]._pMaxMana;
	int mana = plr[myplr]._pMana;

	if (mana <= 0 || maxMana <= 0)
		filled = 0;
	else
		filled = 80 * mana / maxMana;
	plr[myplr]._pManaPer = filled;

	if (filled > 69)
		filled = 69;
	if (filled != 69)
		SetFlaskHeight(pManaBuff, 16, 85 - filled, PANEL_X + 464, PANEL_Y);
	if (filled != 0)
		DrawPanelBox(464, 85 - filled, 88, filled, PANEL_X + 464, PANEL_Y + 69 - filled);

	DrawSpell();
}

void InitControlPan()
{
	int i;

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
	pStatusPanel = LoadFileInMem("CtrlPan\\Panel8.CEL", NULL);
	CelBlitWidth(pBtmBuff, 0, (PANEL_HEIGHT + 16) - 1, PANEL_WIDTH, pStatusPanel, 1, PANEL_WIDTH);
	MemFreeDbg(pStatusPanel);
	pStatusPanel = LoadFileInMem("CtrlPan\\P8Bulbs.CEL", NULL);
	CelBlitWidth(pLifeBuff, 0, 87, 88, pStatusPanel, 1, 88);
	CelBlitWidth(pManaBuff, 0, 87, 88, pStatusPanel, 2, 88);
	MemFreeDbg(pStatusPanel);
	talkflag = FALSE;
	if (gbMaxPlayers != 1) {
		pTalkPanel = LoadFileInMem("CtrlPan\\TalkPanl.CEL", NULL);
		CelBlitWidth(pBtmBuff, 0, (PANEL_HEIGHT + 16) * 2 - 1, PANEL_WIDTH, pTalkPanel, 1, PANEL_WIDTH);
		MemFreeDbg(pTalkPanel);
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
	drawhpflag = TRUE;
	drawmanaflag = TRUE;
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
			}
			for (spell = 1, j = 1; j < MAX_SPELLS; spell <<= 1, j++) {
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

	SetCursorPos(X, Y);
}

static void control_set_button_down(int btn_id)
{
	panbtn[btn_id] = TRUE;
	drawbtnflag = TRUE;
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
			if (i != 7) {
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
		if (sn != SPL_INVALID) {
			switch (p->_pRSplType) {
			case RSPLTYPE_SKILL:
				snprintf(tempstr, sizeof(tempstr), "%s Skill", spelldata[sn].sSkillText);
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
					if (pi->_itype != ITYPE_NONE
					    && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
					    && pi->_iSpell == sn) {
						c++;
					}
				}
				pi = p->SpdList;
				for (i = 0; i < MAXBELTITEMS; i++, pi++) {
					if (pi->_itype != ITYPE_NONE
					    && (pi->_iMiscId == IMISC_SCROLL || pi->_iMiscId == IMISC_SCROLLT)
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
			}
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
	drawbtnflag = TRUE;
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
			if (dropGoldFlag) {
				dropGoldFlag = FALSE;
				dropGoldValue = 0;
			}
			break;
		case PANBTN_SPELLBOOK:
			invflag = FALSE;
			if (dropGoldFlag) {
				dropGoldFlag = FALSE;
				dropGoldValue = 0;
			}
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

static void CPrintString(int y, const char *str, BOOL center, int lines)
{
	BYTE c;
	const char *tmp;
	int lineOffset, strWidth, sx, sy;

	lineOffset = 0;
	sx = 177 + PANEL_X;
	sy = lineOffsets[lines][y] + PANEL_Y;
	if (center) {
		strWidth = 0;
		tmp = str;
		while (*tmp != '\0') {
			c = gbFontTransTbl[(BYTE)*tmp++];
			strWidth += fontkern[fontframe[c]] + 2;
		}
		if (strWidth < 288)
			lineOffset = (288 - strWidth) >> 1;
		sx += lineOffset;
	}
	while (*str != '\0') {
		c = gbFontTransTbl[(BYTE)*str++];
		c = fontframe[c];
		lineOffset += fontkern[c] + 2;
		if (c != '\0') {
			if (lineOffset < 288) {
				PrintChar(sx, sy, c, infoclr);
			}
		}
		sx += fontkern[c] + 2;
	}
}

static void PrintInfo()
{
	int yo, lo, i;

	if (!talkflag) {
		yo = 0;
		lo = 1;
		if (infostr[0] != '\0') {
			CPrintString(0, infostr, TRUE, pnumlines);
			yo = 1;
			lo = 0;
		}

		for (i = 0; i < pnumlines; i++) {
			CPrintString(i + yo, panelstr[i], pstrjust[i], pnumlines - lo);
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
					PrintMonstHistory(monster[pcursmonst].MType->mtype);
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

#define ADD_PlrStringXY(x, y, width, pszStr, col) MY_PlrStringXY(x, y, width, pszStr, col, 1)

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

/**
 * @brief Render text string to back buffer
 * @param x Screen coordinate
 * @param y Screen coordinate
 * @param endX End of line in screen coordinate
 * @param pszStr String to print, in Windows-1252 encoding
 * @param col text_color color value
 * @param base Letter spacing
 */
static void MY_PlrStringXY(int x, int y, int endX, const char *pszStr, char col, int base)
{
	BYTE c;
	const char *tmp;
	int sx, sy, screen_x, line, widthOffset;

	sx = x + SCREEN_X;
	sy = y + SCREEN_Y;
	widthOffset = endX - x + 1;
	line = 0;
	screen_x = 0;
	tmp = pszStr;
	while (*tmp != '\0') {
		c = gbFontTransTbl[(BYTE)*tmp++];
		screen_x += fontkern[fontframe[c]] + base;
	}
	if (screen_x < widthOffset)
		line = (widthOffset - screen_x) >> 1;
	sx += line;
	while (*pszStr != '\0') {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		line += fontkern[c] + base;
		if (c != '\0') {
			if (line < widthOffset)
				PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + base;
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
		MY_PlrStringXY(254, 239, 305, chrstr, col, -1);
	else
		MY_PlrStringXY(258, 239, 301, chrstr, col, 0);

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
	if (plr[myplr]._pStatPts != 0 && MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		lvlbtndown = TRUE;
	return lvlbtndown;
}

void ReleaseLvlBtn()
{
	if (MouseX >= 40 + PANEL_LEFT && MouseX <= 81 + PANEL_LEFT && MouseY >= -39 + PANEL_TOP && MouseY <= -17 + PANEL_TOP)
		chrflag = TRUE;
	lvlbtndown = FALSE;
}

void DrawLevelUpIcon()
{
	int nCel;

	if (stextflag == STORE_NONE) {
		nCel = lvlbtndown ? 3 : 2;
		ADD_PlrStringXY(PANEL_LEFT + 0, PANEL_TOP - 49, PANEL_LEFT + 120, "Level Up", COL_WHITE);
		CelDraw(40 + PANEL_X, -17 + PANEL_Y, pChrButtons, nCel, 41);
	}
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
			default:
				if (p->_pBaseVit >= MaxStats[p->_pClass][ATTRIB_VIT])
					return FALSE;
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
				default:
					NetSendCmdParam1(TRUE, CMD_ADDVIT, 1);
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

static void PrintSBookStr(int x, int y, BOOL cjustflag, const char *pszStr, char col)
{
	BYTE c;
	const char *tmp;
	int screen_x, line, sx;

	sx = x + RIGHT_PANEL_X + SPLICONLENGTH;
	line = 0;
	if (cjustflag) {
		screen_x = 0;
		tmp = pszStr;
		while (*tmp != '\0') {
			c = gbFontTransTbl[(BYTE)*tmp++];
			screen_x += fontkern[fontframe[c]] + 1;
		}
		if (screen_x < 222)
			line = (222 - screen_x) >> 1;
		sx += line;
	}
	while (*pszStr != '\0') {
		c = gbFontTransTbl[(BYTE)*pszStr++];
		c = fontframe[c];
		line += fontkern[c] + 1;
		if (c != '\0') {
			if (line <= 222)
				PrintChar(sx, y, c, col);
		}
		sx += fontkern[c] + 1;
	}
}

static char GetSBookTrans(int sn, BOOL townok)
{
	PlayerStruct *p;
	char st;

	p = &plr[myplr];
#ifdef HELLFIRE
	if ((p->_pClass == PC_MONK) && (sn == SPL_SEARCH))
		return RSPLTYPE_SKILL;
#endif
	st = RSPLTYPE_SPELL;
	if (p->_pISpells & (__int64)1 << (sn - 1)) {
		st = RSPLTYPE_CHARGES;
	}
	if (p->_pAblSpells & (__int64)1 << (sn - 1)) { /// BUGFIX: missing (__int64) (fixed)
		st = RSPLTYPE_SKILL;
	}
	if (st == RSPLTYPE_SPELL) {
		if (!CheckSpell(myplr, sn, RSPLTYPE_SPELL, TRUE)) {
			st = RSPLTYPE_INVALID;
		}
		if ((char)(p->_pSplLvl[sn] + p->_pISplLvlAdd) <= 0) {
			st = RSPLTYPE_INVALID;
		}
	}
	if (townok && currlevel == 0 && st != RSPLTYPE_INVALID && !spelldata[sn].sTownSpell) {
		st = RSPLTYPE_INVALID;
	}

	return st;
}

void DrawSpellBook()
{
	PlayerStruct* p;
	int i, sn, mana, lvl, yp, min, max;
	char st;
	unsigned __int64 spl;

	CelDraw(RIGHT_PANEL_X, 351 + SCREEN_Y, pSpellBkCel, 1, SPANEL_WIDTH);
#ifdef HELLFIRE
	if (sbooktab < 5)
#endif
		CelDraw(RIGHT_PANEL_X + SBOOK_PAGER_WIDTH * sbooktab + 7, 348 + SCREEN_Y, pSBkBtnCel, sbooktab + 1, SBOOK_PAGER_WIDTH);

	p = &plr[myplr];
	spl = p->_pMemSpells | p->_pISpells | p->_pAblSpells;

	yp = 55 + SCREEN_Y;
	for (i = 0; i < lengthof(SpellPages[sbooktab]); i++) {
		sn = SpellPages[sbooktab][i];
		if (sn != -1 && spl & (__int64)1 << (sn - 1)) {
			st = GetSBookTrans(sn, TRUE);
			SetSpellTrans(st);
			DrawSpellCel(RIGHT_PANEL_X + 11, yp, pSBkIconCels, SpellITbl[sn], 37);
			if (sn == p->_pRSpell && st == p->_pRSplType) {
				SetSpellTrans(RSPLTYPE_SKILL);
				DrawSpellCel(RIGHT_PANEL_X + 11, yp, pSBkIconCels, SPLICONLAST, 37);
			}
			PrintSBookStr(10, yp - 23, FALSE, spelldata[sn].sNameText, COL_WHITE);
			switch (GetSBookTrans(sn, FALSE)) {
			case RSPLTYPE_SKILL:
				copy_cstr(tempstr, "Skill");
				break;
			case RSPLTYPE_CHARGES:
				snprintf(tempstr, sizeof(tempstr), "Staff (%i charges)", p->InvBody[INVLOC_HAND_LEFT]._iCharges);
				break;
			default:
				mana = GetManaAmount(myplr, sn) >> 6;
				GetDamageAmt(sn, &min, &max);
				if (min != -1) {
					snprintf(tempstr, sizeof(tempstr), "Mana: %i  Dam: %i - %i", mana, min, max);
				} else {
					snprintf(tempstr, sizeof(tempstr), "Mana: %i   Dam: n/a", mana);
				}
				if (sn == SPL_BONESPIRIT) {
					snprintf(tempstr, sizeof(tempstr), "Mana: %i  Dam: 1/3 tgt hp", mana);
				}
				PrintSBookStr(10, yp - 1, FALSE, tempstr, COL_WHITE);
				lvl = p->_pSplLvl[sn] + p->_pISplLvlAdd;
				if (lvl <= 0) {
					copy_cstr(tempstr, "Spell Level 0 - Unusable");
				} else {
					snprintf(tempstr, sizeof(tempstr), "Spell Level %i", lvl);
				}
				break;
			}
			PrintSBookStr(10, yp - 12, FALSE, tempstr, COL_WHITE);
		}
		yp += 43;
	}
}

void CheckSBook()
{
	PlayerStruct *p;
	int sn;
	char st;
	unsigned __int64 spl;

	if (MouseX >= RIGHT_PANEL + 11 && MouseX < RIGHT_PANEL + 48 && MouseY >= 18 && MouseY < 314) {
		p = &plr[myplr];
		sn = SpellPages[sbooktab][(MouseY - 18) / 43];
		spl = p->_pMemSpells | p->_pISpells | p->_pAblSpells;
		if (sn != -1 && spl & (__int64)1 << (sn - 1)) {
			st = RSPLTYPE_SPELL;
			if (p->_pISpells & (__int64)1 << (sn - 1)) {
				st = RSPLTYPE_CHARGES;
			}
			if (p->_pAblSpells & (__int64)1 << (sn - 1)) {
				st = RSPLTYPE_SKILL;
			}
			p->_pRSpell = sn;
			p->_pRSplType = st;
			force_redraw = 255;
		}
	}
#ifdef HELLFIRE
	if (MouseX >= RIGHT_PANEL + 7 && MouseX < RIGHT_PANEL + 312 && MouseY >= 320 && MouseY < 349)
#else
	if (MouseX >= RIGHT_PANEL + 7 && MouseX < RIGHT_PANEL + 311 && MouseY >= 320 && MouseY < 349) /// BUGFIX: change `< 313` to `< 311` (fixed)
#endif
		sbooktab = (MouseX - (RIGHT_PANEL + 7)) / SBOOK_PAGER_WIDTH;
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
	SetItemData(is, IDI_GOLD);
	GetGoldSeed(pnum, is);
	is->_iStatFlag = TRUE;
	SetGoldItemValue(is, dropGoldValue);
	NewCursor(is->_iCurs + CURSOR_FIRSTITEM);
	plr[pnum]._pGold = CalculateGold(pnum);
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
		msg = control_print_talk_msg(msg, &x, i, 0);
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
	force_redraw = 255;
	sgbTalkSavePos = sgbNextTalkSave;
}

void control_reset_talk()
{
	talkflag = FALSE;
	sgbPlrTalkTbl = 0;
	force_redraw = 255;
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
		if (sgszTalkSave[sgbTalkSavePos][0]) {
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
