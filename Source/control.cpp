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
BOOL panbtn[NUM_PANBTNS];
BOOL chrbtn[4];
BYTE *pPanelButtons;
BYTE *pChrPanel;
BOOL lvlbtndown;
char sgszTalkSave[8][MAX_SEND_STR_LEN];
int dropGoldValue;
BOOL chrbtnactive;
char sgszTalkMsg[MAX_SEND_STR_LEN];
BYTE *pPanelText;
BYTE *pFlasks;
BYTE *pTalkPnl;
BYTE *pTalkBtns;
BOOL talkbtndown[MAX_PLRS - 1];
int pSpell;
BYTE infoclr;
BYTE *pGBoxBuff;
//BYTE *pSBkBtnCel;
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
BYTE SplTransTbl[256];
static_assert(RSPLTYPE_CHARGES != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SCROLL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_ABILITY != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_SPELL != -1, "Cached value of spellTrans must not be -1.");
static_assert(RSPLTYPE_INVALID != -1, "Cached value of spellTrans must not be -1.");
char lastSt = -1;
int initialDropGoldValue;
BYTE *pSpellCels;
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

/** Maps from spell_id to spelicon.cel frame number.
  unused ones : 17 (white burning eye),
				31 (red skull),
				32 (red star),
				33 (red burning stair?),
				34 (bull-head),
  unavailables: 19 (3-bladed fan?),
				20 (teleport?),
				22 (fan)
 */
const BYTE SpellITbl[NUM_SPELLS] = {
	27,// SPL_NULL
	1, // SPL_FIREBOLT
	2, // SPL_HEAL
	3, // SPL_LIGHTNING
	4, // SPL_FLASH
	5, // SPL_IDENTIFY
	6, // SPL_FIREWALL
	7, // SPL_TOWN
	8, // SPL_STONE
	9, // SPL_INFRA
	28,// SPL_RNDTELEPORT
	13,// SPL_MANASHIELD
	12,// SPL_FIREBALL
	18,// SPL_GUARDIAN
	16,// SPL_CHAIN
	14,// SPL_WAVE
	//18,// SPL_DOOMSERP
	//19,// SPL_BLODRIT
	11,// SPL_NOVA
	//20,// SPL_INVISIBIL
	15,// SPL_FLAME
	21,// SPL_GOLEM
	23,// SPL_BLODBOIL
	24,// SPL_TELEPORT
	//25,// SPL_APOCA
	//22,// SPL_ETHEREALIZE
	26,// SPL_REPAIR
	29,// SPL_RECHARGE
	37,// SPL_DISARM
	38,// SPL_ELEMENT
	39,// SPL_CBOLT
	42,// SPL_HBOLT
	41,// SPL_RESURRECT
	40,// SPL_TELEKINESIS
	10,// SPL_HEALOTHER
	36,// SPL_FLARE
	//30,// SPL_BONESPIRIT
#ifdef HELLFIRE
	//51,// SPL_MANA
	//51,// SPL_MAGI
	//50,// SPL_JESTER
	46,// SPL_LIGHTWALL
	47,// SPL_IMMOLAT
	//43,// SPL_WARP
	//45,// SPL_REFLECT
	//48,// SPL_BERSERK
	49,// SPL_FIRERING
	//44,// SPL_SEARCH
	35,// SPL_RUNEFIRE
	35,// SPL_RUNELIGHT
	35,// SPL_RUNENOVA
	35,// SPL_RUNEIMMOLAT
	35,// SPL_RUNESTONE
	40,// SPL_WHITTLE
	28,// SPL_WALK
	32,// SPL_WATTACK
	48,// SPL_ATTACK
	45,// SPL_BLOCK
#else
	28,// SPL_WALK
	32,// SPL_WATTACK
	22,// SPL_ATTACK
	19,// SPL_BLOCK
#endif
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
	{ SPL_NULL, SPL_FIREBOLT, SPL_CBOLT, SPL_HBOLT, SPL_HEAL, SPL_HEALOTHER, SPL_FLAME },
	{ SPL_RESURRECT, SPL_FIREWALL, SPL_TELEKINESIS, SPL_LIGHTNING, SPL_TOWN, SPL_FLASH, SPL_STONE },
	{ SPL_RNDTELEPORT, SPL_MANASHIELD, SPL_ELEMENT, SPL_FIREBALL, SPL_WAVE, SPL_CHAIN, SPL_GUARDIAN },
	{ SPL_NOVA, SPL_GOLEM, SPL_TELEPORT, SPL_FLARE },
#ifdef HELLFIRE
	{ SPL_LIGHTWALL, SPL_IMMOLAT, SPL_FIRERING },
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

static void DrawSpellIconOverlay(int sn, int st, int lvl, int x, int y)
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
		PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, TRUE, t, 1);
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
		PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, TRUE, COL_WHITE, 1);
		break;
	case RSPLTYPE_CHARGES:
		snprintf(tempstr, sizeof(tempstr), "%d/%d",
			p->InvBody[INVLOC_HAND_LEFT]._iCharges,
			p->InvBody[INVLOC_HAND_LEFT]._iMaxCharges);
		PrintString(x + 4, y, x + SPLICONLENGTH, tempstr, TRUE, COL_WHITE, 1);
		break;
	case RSPLTYPE_INVALID:
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
void DrawRSpell()
{
	char st;
	int spl, lvl;

	spl = plr[myplr]._pRSpell;
	st = plr[myplr]._pRSplType;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (spl == SPL_INVALID) {
		st = RSPLTYPE_INVALID;
		spl = SPL_NULL;
	} else if (currlevel == 0 && !spelldata[spl].sTownSpell)
		st = RSPLTYPE_INVALID;
	else if (st == RSPLTYPE_SPELL) {
		lvl = GetSpellLevel(myplr, spl);
		if (lvl <= 0 || !CheckSpell(myplr, spl))
			st = RSPLTYPE_INVALID;
	}
	SetSpellTrans(st);
	DrawSpellCel(SCREEN_X + SCREEN_WIDTH - SPLICONLENGTH, SCREEN_Y + SCREEN_HEIGHT - 1, pSpellCels,
		SpellITbl[spl], SPLICONLENGTH);
	DrawSpellIconOverlay(spl, st, lvl, SCREEN_X + SCREEN_WIDTH - SPLICONLENGTH, SCREEN_Y + SCREEN_HEIGHT - 1);
}

void DrawSpeedBook()
{
	PlayerStruct *p;
	int i, sn, x, y, /*c,*/ s, t, lx, ly;
	unsigned __int64 mask;

	pSpell = SPL_INVALID;
	x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	y = PANEL_Y - 17;
	p = &plr[myplr];
	static_assert(RSPLTYPE_ABILITY == 0, "Looping over the spell-types in DrawSpeedBook relies on ordered, indexed enum values 1.");
	static_assert(RSPLTYPE_SPELL == 1, "Looping over the spell-types in DrawSpeedBook relies on ordered, indexed enum values 2.");
	static_assert(RSPLTYPE_SCROLL == 2, "Looping over the spell-types in DrawSpeedBook relies on ordered, indexed enum values 3.");
	static_assert(RSPLTYPE_CHARGES == 3, "Looping over the spell-types in DrawSpeedBook relies on ordered, indexed enum values 4.");
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
		for (sn = 1; mask != 0 && sn < NUM_SPELLS; mask >>= 1, sn++) {
			if (!(mask & 1))
				continue;
			t = i;
			if (i == RSPLTYPE_SPELL) {
				s = GetSpellLevel(myplr, sn);
				t = s > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID;
			}
			if (currlevel == 0 && !spelldata[sn].sTownSpell)
				t = RSPLTYPE_INVALID;
			SetSpellTrans(t);
			DrawSpellCel(x, y, pSpellCels, SpellITbl[sn], SPLICONLENGTH);
			lx = x - BORDER_LEFT;
			ly = y - BORDER_TOP - SPLICONLENGTH;
			if (MouseX >= lx && MouseX < lx + SPLICONLENGTH && MouseY >= ly && MouseY < ly + SPLICONLENGTH) {
				//DrawSpellCel(x, y, pSpellCels, c, SPLICONLENGTH);
				DrawSpellCel(x, y, pSpellCels, SPLICONLAST, SPLICONLENGTH);

				pSpell = sn;
				pSplType = i;

				DrawSpellIconOverlay(sn, i, s, x, y);

				for (t = 0; t < 4; t++) {
					if (p->_pSplHotKey[t] == sn && p->_pSplTHotKey[t] == i) {
						//DrawSpellCel(x, y, pSpellCels, t + SPLICONLAST + 5, SPLICONLENGTH);
						snprintf(tempstr, sizeof(tempstr), "#%d", t + 1);
						PrintString(x + SPLICONLENGTH - 18, y - SPLICONLENGTH + 16, x + SPLICONLENGTH, tempstr, FALSE, COL_GOLD, 1);
					}
				}
			}
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
		if (sn != 1 && x != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS) {
			x -= SPLICONLENGTH;
			if (x == PANEL_X + 12 - SPLICONLENGTH) {
				x = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
				y -= SPLICONLENGTH;
			}
		}
	}
}

void SetRSpell()
{
	spselflag = FALSE;
	if (pSpell != SPL_INVALID) {
		plr[myplr]._pRSpell = pSpell;
		plr[myplr]._pRSplType = pSplType;
		//gbRedrawFlags = REDRAW_ALL;
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
	case RSPLTYPE_ABILITY:
		spells = p->_pAblSkills;
		break;
	case RSPLTYPE_SPELL:
		spells = p->_pMemSkills;
		break;
	case RSPLTYPE_SCROLL:
		spells = p->_pScrlSkills;
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
		//gbRedrawFlags = REDRAW_ALL;
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
	DrawFlask2(x, filled, 3, plr[myplr].pManaShield == 0 ? 4 : 5, 93);
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
	talkflag = FALSE;
	if (gbMaxPlayers != 1) {
		pTalkPnl = LoadFileInMem("CtrlPan\\TalkPnl.CEL", NULL);
		pTalkBtns = LoadFileInMem("CtrlPan\\TalkButt.CEL", NULL);
		sgszTalkMsg[0] = '\0';
		for (i = 0; i < lengthof(whisper); i++)
			whisper[i] = TRUE;
		for (i = 0; i < lengthof(talkbtndown); i++)
			talkbtndown[i] = FALSE;
	}
	lvlbtndown = FALSE;
	pPanelButtons = LoadFileInMem("CtrlPan\\Menu.CEL", NULL);
	for (i = 0; i < lengthof(panbtn); i++)
		panbtn[i] = FALSE;
	numpanbtns = gbMaxPlayers == 1 ? NUM_PANBTNS - 2 : NUM_PANBTNS;
	pChrButtons = LoadFileInMem("Data\\CharBut.CEL", NULL);
	for (i = 0; i < lengthof(chrbtn); i++)
		chrbtn[i] = FALSE;
	chrbtnactive = FALSE;
	pDurIcons = LoadFileInMem("Items\\DurIcons.CEL", NULL);
	infostr[0] = '\0';
	gbRedrawFlags |= REDRAW_HP_FLASK | REDRAW_MANA_FLASK | REDRAW_SPEED_BAR;
	chrflag = FALSE;
	spselflag = FALSE;
	pSpellBkCel = LoadFileInMem("Data\\SpellBk.CEL", NULL);
	//pSBkBtnCel = LoadFileInMem("Data\\SpellBkB.CEL", NULL);
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

/**
 * Draws the control panel buttons in their current state. If the button is in the default
 * state draw it from the panel cel(extract its sub-rect). Else draw it from the buttons cel.
 */
void DrawCtrlBtns()
{
	int i, x, y;
	BOOLEAN pb;
	const char* text;

	i = 0;
	x = SCREEN_X + PanBtnPos[i][0];
	if (!panbtn[PANBTN_MAINMENU]) {
		CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1] + 18, pPanelButtons, 4, 71);
		return;
	}
	CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1] + 18, pPanelButtons, 3, 71);
	for (i = 1; i < numpanbtns; i++) {
		y = SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1];
		pb = panbtn[i];
		CelDraw(x, y + 18, pPanelButtons, 1, 71);
		// print the text of the button
		text = PanBtnTxt[i];
		if (i == PANBTN_FRIENDLY)
			text = FriendlyMode ? "PvP:Off" : "PvP:On";
		PrintString(x + 3, y + 15, x + 70, text, TRUE, pb ? COL_GOLD : COL_WHITE, 1);
	}
}

/**
 * Opens the "Speed Book": the rows of known spells for quick-setting a spell that
 * show up when you click the spell slot at the control panel.
 */
void DoSpeedBook()
{
	spselflag = TRUE;

#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	PlayerStruct *p;
	unsigned __int64 mask;
	int xo, yo, X, Y, i, sn;

	xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
	yo = PANEL_Y - 17;
	X = xo - (BORDER_LEFT - SPLICONLENGTH / 2);
	Y = yo - (BORDER_TOP + SPLICONLENGTH / 2);
	p = &plr[myplr];
	if (p->_pRSpell != SPL_INVALID) {
		static_assert(RSPLTYPE_ABILITY == 0, "Looping over the spell-types in DoSpeedBook relies on ordered, indexed enum values 1.");
		static_assert(RSPLTYPE_SPELL == 1, "Looping over the spell-types in DoSpeedBook relies on ordered, indexed enum values 2.");
		static_assert(RSPLTYPE_SCROLL == 2, "Looping over the spell-types in DoSpeedBook relies on ordered, indexed enum values 3.");
		static_assert(RSPLTYPE_CHARGES == 3, "Looping over the spell-types in DoSpeedBook relies on ordered, indexed enum values 4.");
		for (i = 0; i < 4; i++) {
			switch (i) {
			case RSPLTYPE_ABILITY:
				mask = p->_pAblSkills;
				break;
			case RSPLTYPE_SPELL:
				mask = p->_pMemSkills;
				break;
			case RSPLTYPE_SCROLL:
				mask = p->_pScrlSkills;
				break;
			case RSPLTYPE_CHARGES:
				mask = p->_pISpells;
				break;
			default:
				ASSUME_UNREACHABLE
				break;
			}
			for (sn = 1; mask != 0 && sn < NUM_SPELLS; mask >>= 1, sn++) {
				if (!(mask & 1))
					continue;
				if (sn == p->_pRSpell && i == p->_pRSplType) {
					X = xo - (BORDER_LEFT - SPLICONLENGTH / 2);
					Y = yo - (BORDER_TOP + SPLICONLENGTH / 2);
				}
				xo -= SPLICONLENGTH;
				if (xo == PANEL_X + 12 - SPLICONLENGTH) {
					xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
					yo -= SPLICONLENGTH;
				}
			}
			if (sn != 1 && xo != PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS) {
				xo -= SPLICONLENGTH;
				if (xo == PANEL_X + 12 - SPLICONLENGTH) {
					xo = PANEL_X + 12 + SPLICONLENGTH * SPLROWICONLS;
					yo -= SPLICONLENGTH;
				}
			}
		}
	}

	if (sgbControllerActive)
		SetCursorPos(X, Y);
#endif
}

void HandleSpellBtn()
{
	if (!spselflag) {
		invflag = FALSE;
		chrflag = FALSE;
		questlog = FALSE;
		sbookflag = FALSE;
		helpflag = FALSE;
		DoSpeedBook();
	} else {
		spselflag = FALSE;
	}
	gamemenu_off();
}

static void control_set_button_down(int btn_id)
{
	if (btn_id == PANBTN_MAINMENU) {
		panbtn[PANBTN_MAINMENU] = !panbtn[PANBTN_MAINMENU];
	} else {
		assert(panbtn[PANBTN_MAINMENU]);
		panbtn[btn_id] = TRUE;
	}
	//gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
}

static BOOL InLvlUpRect()
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
	lvlbtndown = FALSE;
}
/**
 * Checks if the mouse cursor is within any of the panel buttons and flag it if so.
 */
BOOL DoPanBtn()
{
	int i, mx, my;

	mx = MouseX;
	my = MouseY;
	for (i = panbtn[PANBTN_MAINMENU] ? numpanbtns - 1 : 0; i >= 0; i--) {
		if (mx >= PanBtnPos[i][0]
		 && mx <= PanBtnPos[i][0] + MENUBTN_WIDTH
		 && my >= SCREEN_HEIGHT - PanBtnPos[i][1]
		 && my <= SCREEN_HEIGHT - PanBtnPos[i][1] + MENUBTN_HEIGHT) {
			control_set_button_down(i);
			return TRUE;
		}
	}
	if (mx >= SCREEN_WIDTH - (SPLICONLENGTH + 4)
	 && mx <= SCREEN_WIDTH - 4
	 && my >= SCREEN_HEIGHT - (SPLICONLENGTH + 4)
	 && my <= SCREEN_HEIGHT - 4) {
		HandleSpellBtn();
		return TRUE;
	}
	if (plr[myplr]._pLvlUp && InLvlUpRect())
		lvlbtndown = TRUE;
	return lvlbtndown;
}

void DoLimitedPanBtn()
{
	if (MouseX >= PanBtnPos[PANBTN_MAINMENU][0]
	 && MouseX <= PanBtnPos[PANBTN_MAINMENU][0] + MENUBTN_WIDTH
	 && MouseY >= SCREEN_HEIGHT - PanBtnPos[PANBTN_MAINMENU][1]
	 && MouseY <= SCREEN_HEIGHT - PanBtnPos[PANBTN_MAINMENU][1] + MENUBTN_HEIGHT) {
		control_set_button_down(PANBTN_MAINMENU);
	} else if (panbtn[PANBTN_MAINMENU] && gbMaxPlayers != 1) {
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
		qtextflag = FALSE;
		break;
	case PANBTN_OPTIONS:
		gamemenu_on();
		return;
	case PANBTN_CHARINFO:
		questlog = FALSE;
		spselflag = FALSE;
		plr[myplr]._pLvlUp = FALSE;
		chrflag = !chrflag;
		break;
	case PANBTN_INVENTORY:
		sbookflag = FALSE;
		spselflag = FALSE;
		invflag = !invflag;
		break;
	case PANBTN_SPELLBOOK:
		invflag = FALSE;
		spselflag = FALSE;
		sbookflag = !sbookflag;
		break;
	case PANBTN_QLOG:
		chrflag = FALSE;
		spselflag = FALSE;
		if (!questlog)
			StartQuestlog();
		else
			questlog = FALSE;
		break;
	case PANBTN_AUTOMAP:
		ToggleAutomap();
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
	helpflag = FALSE;
	gamemenu_off();
}

/**
 * Check if the mouse is within a control panel button that's flagged.
 * Takes apropiate action if so.
 */
void CheckBtnUp()
{
	int i;

	static_assert(lengthof(panbtn) == lengthof(PanBtnPos), "Mismatching panbtn and panbtnpos tables.");
	static_assert(PANBTN_MAINMENU == 0, "CheckBtnUp needs to skip the mainmenu-button.");
	for (i = 1; i < lengthof(panbtn); i++) {
		if (!panbtn[i]) {
			continue;
		}

		panbtn[i] = FALSE;
		if (MouseX < PanBtnPos[i][0]
		 || MouseX > PanBtnPos[i][0] + MENUBTN_WIDTH
		 || MouseY < SCREEN_HEIGHT - PanBtnPos[i][1]
		 || MouseY > SCREEN_HEIGHT - PanBtnPos[i][1] + MENUBTN_HEIGHT) {
			continue;
		}

		HandlePanBtn(i);

		panbtn[PANBTN_MAINMENU] = FALSE;
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
	//MemFreeDbg(pSBkBtnCel);
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
void PrintString(int x, int y, int endX, const char *pszStr, BOOL center, BYTE col, int kern)
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

#define ADD_PlrStringXY(x, y, endX, pszStr, col) PrintString(x + SCREEN_X, y + SCREEN_Y, endX + SCREEN_X, pszStr, TRUE, col, 1)

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
	BOOL bow;

	p = &plr[myplr];
	pc = p->_pClass;
	bow = p->InvBody[INVLOC_HAND_LEFT]._itype == ITYPE_BOW;

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
		PrintString(241 + SCREEN_X, 205 + SCREEN_Y, 292 + SCREEN_X, chrstr, TRUE, col, -1);
	else
		PrintString(245 + SCREEN_X, 205 + SCREEN_Y, 288 + SCREEN_X, chrstr, TRUE, col, 0);

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
		CelDraw(ChrBtnsRect[ATTRIB_STR].x + SCREEN_X, ChrBtnsRect[ATTRIB_STR].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_STR] + 2, CHRBTN_WIDTH);
		CelDraw(ChrBtnsRect[ATTRIB_MAG].x + SCREEN_X, ChrBtnsRect[ATTRIB_MAG].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_MAG] + 4, CHRBTN_WIDTH);
		CelDraw(ChrBtnsRect[ATTRIB_DEX].x + SCREEN_X, ChrBtnsRect[ATTRIB_DEX].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_DEX] + 6, CHRBTN_WIDTH);
		CelDraw(ChrBtnsRect[ATTRIB_VIT].x + SCREEN_X, ChrBtnsRect[ATTRIB_VIT].y + CHRBTN_HEIGHT + SCREEN_Y, pChrButtons, chrbtn[ATTRIB_VIT] + 8, CHRBTN_WIDTH);
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
	CelDraw(SCREEN_X + 175, SCREEN_Y + SCREEN_HEIGHT - 24, pChrButtons, lvlbtndown + 2, CHRBTN_WIDTH);
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
			if (!setlevel) {
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
			assert(setlevel);
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
		snprintf(infostr, sizeof(infostr), p->pManaShield == 0 ? "%s(%i)" : "%s(%i)*", ClassStrTbl[p->_pClass], p->_pLevel);
		DrawTooltip2(p->_pName, infostr, xx, yy, COL_GOLD);
		DrawHealthBar(p->_pHitPoints, p->_pMaxHP, xx, yy + 10);
	} else if (spselflag) {
		if (pSpell == SPL_INVALID)
			return;
		const char* fmt;
		switch (pSplType) {
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
		snprintf(infostr, sizeof(infostr), fmt, spelldata[pSpell].sNameText);
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


BOOL CheckChrBtns()
{
	int i;

	if (plr[myplr]._pStatPts != 0 && !chrbtnactive) {
		for (i = 0; i < lengthof(ChrBtnsRect); i++) {
			if (MouseX < ChrBtnsRect[i].x
			 || MouseX > ChrBtnsRect[i].x + ChrBtnsRect[i].w
			 || MouseY < ChrBtnsRect[i].y
			 || MouseY > ChrBtnsRect[i].y + ChrBtnsRect[i].h)
				continue;

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
					NetSendCmd(TRUE, CMD_ADDSTR);
					break;
				case 1:
					NetSendCmd(TRUE, CMD_ADDMAG);
					break;
				case 2:
					NetSendCmd(TRUE, CMD_ADDDEX);
					break;
				case 3:
					NetSendCmd(TRUE, CMD_ADDVIT);
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

static char GetSBookTrans(int sn, BOOL townok)
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
		return RSPLTYPE_INVALID;
	}
	if (townok && currlevel == 0 && !spelldata[sn].sTownSpell) {
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
	PrintString(RIGHT_PANEL_X + 2, SCREEN_Y + SPANEL_HEIGHT - 7, RIGHT_PANEL_X + SPANEL_WIDTH, tempstr, TRUE, COL_WHITE, 0);

	p = &plr[myplr];
	spl = p->_pMemSkills | p->_pISpells | p->_pAblSkills;

	yp = SCREEN_Y + SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx = RIGHT_PANEL_X + SBOOK_CELBORDER;
	for (i = 0; i < lengthof(SpellPages[sbooktab]); i++) {
		sn = SpellPages[sbooktab][i];
		if (sn != SPL_INVALID && (spl & SPELL_MASK(sn))) {
			st = GetSBookTrans(sn, TRUE);
			SetSpellTrans(st);
			DrawSpellCel(sx, yp, pSBkIconCels, SpellITbl[sn], SBOOK_CELWIDTH);
			if (sn == p->_pRSpell && st == p->_pRSplType) {
				SetSpellTrans(RSPLTYPE_ABILITY);
				DrawSpellCel(sx, yp, pSBkIconCels, SPLICONLAST, SBOOK_CELWIDTH);
			}
			switch (GetSBookTrans(sn, FALSE)) {
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
			PrintString(sx + SBOOK_LINE_TAB, yp - 23 + offset, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, spelldata[sn].sNameText, FALSE, COL_WHITE, 1);
			PrintString(sx + SBOOK_LINE_TAB, yp - 12 + offset, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, FALSE, COL_WHITE, 1);

			if (offset == 0) {
				if (mana != 0)
					cat_str(tempstr, offset, "Mana: %i  ", mana);
				if (min != -1)
					cat_str(tempstr, offset, "Dam: %i-%i", min, max);
				PrintString(sx + SBOOK_LINE_TAB, yp - 1, sx + SBOOK_LINE_TAB + SBOOK_LINE_LENGTH, tempstr, FALSE, COL_WHITE, 1);
			}
		}
		yp += SBOOK_CELBORDER + SBOOK_CELHEIGHT;
	}
}

void CheckSBook()
{
	PlayerStruct *p;
	int sn;
	char st;
	unsigned __int64 spl;

	int dx = MouseX - (RIGHT_PANEL + SBOOK_LEFT_BORDER);
	if (dx < 0)
		return;
	int dy = MouseY - SBOOK_TOP_BORDER;
	if (dy < 0)
		return;

	if (dy < lengthof(SpellPages[sbooktab]) * (SBOOK_CELBORDER + SBOOK_CELHEIGHT)) {
		if (dx < 2 * SBOOK_CELBORDER + SBOOK_CELWIDTH) {
			sn = SpellPages[sbooktab][dy / (SBOOK_CELBORDER + SBOOK_CELHEIGHT)];
			p = &plr[myplr];
			spl = p->_pMemSkills | p->_pISpells | p->_pAblSkills;
			if (sn != SPL_INVALID && spl & SPELL_MASK(sn)) {
				if (p->_pAblSkills & SPELL_MASK(sn))
					st = RSPLTYPE_ABILITY;
				else if (p->_pISpells & SPELL_MASK(sn))
					st = RSPLTYPE_CHARGES;
				else
					st = RSPLTYPE_SPELL;
				p->_pRSpell = sn;
				p->_pRSplType = st;
				//gbRedrawFlags = REDRAW_ALL;
			}
		}
	} else {
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
		dropGoldFlag = FALSE;
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
	dropGoldFlag = FALSE;
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

	assert(talkflag);

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
	static_assert(lengthof(whisper) == MAX_PLRS, "Table whisper does not work with the current MAX_PLRS in DrawTalkPan.");
	for (i = 0; i < MAX_PLRS; i++) {
		if (i == myplr)
			continue;
		if (whisper[i]) {
			color = COL_GOLD;
			nCel = 0;
		} else {
			color = COL_RED;
			nCel = 2;
		}
		if (talkbtndown[talk_btn])
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

BOOL control_check_talk_btn()
{
	int i;

	if (MouseX < TALK_PNL_LEFT + 3 || MouseX > TALK_PNL_LEFT + 64)
		return FALSE;
	if (MouseY < TALK_PNL_TOP + 65 || MouseY > TALK_PNL_TOP + 65 + 18 * lengthof(talkbtndown))
		return FALSE;

	for (i = 0; i < lengthof(talkbtndown); i++) {
		talkbtndown[i] = FALSE;
	}

	talkbtndown[(MouseY - (TALK_PNL_TOP + 65)) / 18] = TRUE;

	return TRUE;
}

void control_release_talk_btn()
{
	int i, y;

	if (MouseX >= TALK_PNL_LEFT + 3  && MouseX <= TALK_PNL_LEFT + 64) {
		y = MouseY - (TALK_PNL_TOP + 65);
		for (i = 0; i < lengthof(talkbtndown); i++, y -= 18) {
			if (talkbtndown[i] && y >= 0 && y <= 18) {
				if (i >= myplr)
					i++;
				whisper[i] = !whisper[i];
			}
		}
	}
	for (i = 0; i < lengthof(talkbtndown); i++) {
		talkbtndown[i] = FALSE;
	}
}

void control_type_message()
{
	int i;

	if (gbMaxPlayers == 1) {
		return;
	}

	talkflag = TRUE;
	SDL_StartTextInput();
	sgszTalkMsg[0] = '\0';
	for (i = 0; i < lengthof(talkbtndown); i++) {
		talkbtndown[i] = FALSE;
	}
	//gbRedrawFlags = REDRAW_ALL;
	sgbTalkSavePos = sgbNextTalkSave;
}

void control_reset_talk()
{
	talkflag = FALSE;
	SDL_StopTextInput();
	//gbRedrawFlags = REDRAW_ALL;
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

	assert(talkflag);
	assert(gbMaxPlayers != 1);

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

	assert(talkflag);

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
		return FALSE;
	}
	return TRUE;
}

DEVILUTION_END_NAMESPACE
