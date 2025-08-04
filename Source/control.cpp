/**
 * @file control.cpp
 *
 * Implementation of the character and main control panels
 */
#include "all.h"
#include "plrctrls.h"
#include "engine/render/cel_render.h"
#include "engine/render/raw_render.h"
#include "engine/render/text_render.h"
#include "storm/storm_net.h"
#include "utils/screen_reader.hpp"

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

/** Campaign-map images CEL */
static CelImageBuf* pMapIconCels;
/** Specifies the campaign-map status. */
BYTE gbCampaignMapFlag;
/** The 'highlighted' map on the campaign-map. */
static CampaignMapEntry currCamEntry;
/** The entries of the current campaign-map. */
static CampaignMapEntry camEntries[16][16];
/** The index of the campaign-map in the inventory. */
int camItemIndex;
/** The 'selected' map on the campaign-map. */
CampaignMapEntry selCamEntry;

/** Golddrop background CEL */
static CelImageBuf* pGoldDropCel;
/** The current value in Golddrop. */
int gnDropGoldValue;
/** The gold-stack index which is used as a source in Golddrop (inv_item). */
BYTE gbDropGoldIndex;
BYTE infoclr;
char tempstr[256];
char infostr[256];
/** Number of active windows on the screen. */
int gnNumActiveWindows;
/** The list of active windows on the screen. */
BYTE gaActiveWindows[NUM_WNDS];
BYTE gbDragWnd;
int gnDragWndX;
int gnDragWndY;
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
static bool gabChrbtn[NUM_ATTRIBS];
/** Specifies whether any attribute-button is pressed on Character-Panel. */
bool gbChrbtnactive;
/** Color translations for the skill icons. */
static BYTE SkillTrns[NUM_RSPLTYPES][NUM_COLORS];
/** Specifies whether the Skill-List is displayed. */
bool gbSkillListFlag;
/** Skill-List images CEL */
static CelImageBuf* pSpellCels;
/** The 'highlighted' skill in the Skill-List or in the Spell-Book. */
static PlrSkillUse currSkill;
/** Specifies which skill should be selected by the cursor in the Skill-List. */
static PlrSkillUse targetSkill;
/** Specifies where the cursor should be moved relative to the current skill in the Skill-List. */
static POS32 deltaSkillPos;

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
/** The number of spells/skills on a single spellbook page. */
#define NUM_BOOK_ENTRIES 7
/** Maps from spellbook page number and position to spell_id. */
static BYTE SpellPages[SPLBOOKTABS][NUM_BOOK_ENTRIES] = {
	// clang-format off
	{ SPL_NULL, SPL_CBOLT, SPL_LIGHTNING, SPL_CHAIN, SPL_NOVA, SPL_INVALID, SPL_INVALID },
	{ SPL_FIREBOLT, SPL_FIREBALL, SPL_INFERNO, SPL_FIREWALL, SPL_WAVE, SPL_GUARDIAN, SPL_ELEMENTAL },
	{ SPL_WHIPLASH, SPL_WALLOP, SPL_SWIPE, SPL_POINT_BLANK, SPL_FAR_SHOT, SPL_MULTI_SHOT, SPL_PIERCE_SHOT },
	{ SPL_GOLEM, SPL_MANASHIELD, SPL_HEAL, SPL_HEALOTHER, SPL_RNDTELEPORT, SPL_TELEPORT, SPL_TOWN },
	{ SPL_METEOR, SPL_HBOLT, SPL_FLARE, SPL_FLASH, SPL_POISON, SPL_BLOODBOIL, SPL_WIND },
	{ SPL_CHARGE, SPL_RAGE, SPL_SWAMP, SPL_SHROUD, SPL_TELEKINESIS, SPL_ATTRACT, SPL_STONE },
#ifdef HELLFIRE
	{ SPL_FIRERING, SPL_RUNEFIRE, SPL_RUNEWAVE, SPL_RUNELIGHT, SPL_RUNENOVA, SPL_INVALID, SPL_INVALID },
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
	if (st != RSPLTYPE_INV)
		return st;
	return SPELL_RUNE(sn) ? RSPLTYPE_RUNE : RSPLTYPE_SCROLL;
#else
	return st;
#endif
}

/*static void SetSpellTrans(BYTE st)
{
	int i;

	if (lastSt == st)
		return;
	lastSt = st;

	switch (st) {
	case RSPLTYPE_ABILITY:
		SkillTrns[PAL8_YELLOW] = PAL8_YELLOW + 1;
		SkillTrns[PAL8_YELLOW + 1] = PAL8_YELLOW + 1;
		SkillTrns[PAL8_YELLOW + 2] = PAL8_YELLOW + 2;
		for (i = 0; i < 16; i++) {
			SkillTrns[PAL16_BEIGE + i] = PAL16_BEIGE + i;
			SkillTrns[PAL16_YELLOW + i] = PAL16_YELLOW + i;
			SkillTrns[PAL16_ORANGE + i] = PAL16_ORANGE + i;
		}
		break;
	case RSPLTYPE_SPELL:
		SkillTrns[PAL8_YELLOW] = PAL16_BLUE + 1;
		SkillTrns[PAL8_YELLOW + 1] = PAL16_BLUE + 3;
		SkillTrns[PAL8_YELLOW + 2] = PAL16_BLUE + 5;
		for (i = PAL16_BLUE; i < PAL16_BLUE + 16; i++) {
			SkillTrns[PAL16_BEIGE - PAL16_BLUE + i] = i;
			SkillTrns[PAL16_YELLOW - PAL16_BLUE + i] = i;
			SkillTrns[PAL16_ORANGE - PAL16_BLUE + i] = i;
		}
		break;
	case RSPLTYPE_INV:
		SkillTrns[PAL8_YELLOW] = PAL16_BEIGE + 1;
		SkillTrns[PAL8_YELLOW + 1] = PAL16_BEIGE + 3;
		SkillTrns[PAL8_YELLOW + 2] = PAL16_BEIGE + 5;
		for (i = PAL16_BEIGE; i < PAL16_BEIGE + 16; i++) {
			SkillTrns[i] = i;
			SkillTrns[PAL16_YELLOW - PAL16_BEIGE + i] = i;
			SkillTrns[PAL16_ORANGE - PAL16_BEIGE + i] = i;
		}
		break;
	case RSPLTYPE_CHARGES:
		SkillTrns[PAL8_YELLOW] = PAL16_ORANGE + 1;
		SkillTrns[PAL8_YELLOW + 1] = PAL16_ORANGE + 3;
		SkillTrns[PAL8_YELLOW + 2] = PAL16_ORANGE + 5;
		for (i = PAL16_ORANGE; i < PAL16_ORANGE + 16; i++) {
			SkillTrns[PAL16_BEIGE - PAL16_ORANGE + i] = i;
			SkillTrns[PAL16_YELLOW - PAL16_ORANGE + i] = i;
			SkillTrns[i] = i;
		}
		break;
	case RSPLTYPE_INVALID:
		SkillTrns[PAL8_YELLOW] = PAL16_GRAY + 1;
		SkillTrns[PAL8_YELLOW + 1] = PAL16_GRAY + 3;
		SkillTrns[PAL8_YELLOW + 2] = PAL16_GRAY + 5;
		for (i = PAL16_GRAY; i < PAL16_GRAY + 15; i++) {
			SkillTrns[PAL16_BEIGE - PAL16_GRAY + i] = i;
			SkillTrns[PAL16_YELLOW - PAL16_GRAY + i] = i;
			SkillTrns[PAL16_ORANGE - PAL16_GRAY + i] = i;
		}
		SkillTrns[PAL16_BEIGE + 15] = 0;
		SkillTrns[PAL16_YELLOW + 15] = 0;
		SkillTrns[PAL16_ORANGE + 15] = 0;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}*/

static void DrawSpellIconOverlay(int x, int y, int sn, int st)
{
	ItemStruct* pi;
	int t = COL_WHITE, v, mv;

	switch (st) {
	case RSPLTYPE_ABILITY:
		return;
	case RSPLTYPE_SPELL:
		if (myplr._pHasUnidItem) {
			copy_cstr(tempstr, "?");
			break;
		}
		v = myplr._pSkillLvl[sn];
		if (v > 0) {
			snprintf(tempstr, sizeof(tempstr), "l%02d", v);
		} else {
			copy_cstr(tempstr, "X");
			t = COL_RED;
		}
		break;
	case RSPLTYPE_INV:
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
		break;
	case RSPLTYPE_CHARGES:
		if (myplr._pHasUnidItem) {
			copy_cstr(tempstr, "?");
			break;
		}
		v = 0;
		mv = 0;
		pi = myplr._pInvBody;
		for (t = NUM_INVLOC; t > 0; t--, pi++) {
			if (pi->_itype != ITYPE_NONE && pi->_iSpell == sn && pi->_iStatFlag) {
				v += pi->_iCharges;
				mv += pi->_iMaxCharges;
			}
		}
		snprintf(tempstr, sizeof(tempstr), "%d/%d", v, mv);
		break;
	case RSPLTYPE_INVALID:
		return;
	default:
		ASSUME_UNREACHABLE
		return;
	}
	PrintJustifiedString(x, y, x + SPLICON_WIDTH, tempstr, t, FONT_KERN_SMALL);
}

static void DrawSkillIcon(int pnum, BYTE spl, BYTE st, BYTE offset)
{
	int lvl, y;

	// BUGFIX: Move the next line into the if statement to avoid OOB (SPL_INVALID is -1) (fixed)
	if (spl == SPL_NULL) {
		st = RSPLTYPE_INVALID;
	} else if ((spelldata[spl].sUseFlags & plr._pSkillFlags) != spelldata[spl].sUseFlags)
		st = RSPLTYPE_INVALID;
	else if (st == RSPLTYPE_SPELL) {
		lvl = plr._pSkillLvl[spl];
		if (lvl <= 0 || plr._pMana < GetManaAmount(pnum, spl))
			st = RSPLTYPE_INVALID;
	}
	y = SCREEN_Y + SCREEN_HEIGHT - 1 - offset;
	CelDrawTrnTbl(PANEL_X + PANEL_WIDTH - SPLICON_WIDTH, y, pSpellCels,
		spelldata[spl].sIcon, SkillTrns[GetSpellTrans(st, spl)]);
	DrawSpellIconOverlay(PANEL_X + PANEL_WIDTH - SPLICON_WIDTH, y, spl, st);
}

bool ToggleWindow(BYTE wnd)
{
	int i;

	for (i = 0; i < gnNumActiveWindows; i++) {
		if (gaActiveWindows[i] == wnd) {
			gnNumActiveWindows--;
			for ( ; i < gnNumActiveWindows; i++) {
				gaActiveWindows[i] = gaActiveWindows[i + 1];
			}
			return false;
		}
	}

	gaActiveWindows[gnNumActiveWindows] = wnd;
	gnNumActiveWindows++;
	return true;
}

/**
 * Sets the spell frame to draw and its position then draws it.
 */
void DrawSkillIcons()
{
	int pnum;
	PlrSkillUse skill;

	pnum = mypnum;
	skill = plr._pMainSkill._psAttack._suSkill == SPL_NULL ? plr._pMainSkill._psMove : plr._pMainSkill._psAttack;
	DrawSkillIcon(pnum, skill._suSkill, skill._suType, 0);

	skill = plr._pAltSkill._psAttack._suSkill == SPL_NULL ? plr._pAltSkill._psMove : plr._pAltSkill._psAttack;
	DrawSkillIcon(pnum, skill._suSkill, skill._suType, SPLICON_WIDTH);

	const char* str;
	unsigned numchar;
	switch (pcurstgt) {
	case TGT_NORMAL:
		return;
	case TGT_ITEM:
		str = "Item";
		numchar = lengthof("Item") - 1;
		break;
	case TGT_OBJECT:
		str = "Object";
		numchar = lengthof("Object") - 1;
		static_assert((lengthof("Object") - 1) * SMALL_FONT_HEIGHT <= 2 * SPLICON_WIDTH, "DrawSkillIcons uses unsigned division to calculate sy.");
		break;
	case TGT_OTHER:
		str = "Other";
		numchar = lengthof("Other") - 1;
		break;
	case TGT_DEAD:
		str = "Dead";
		numchar = lengthof("Dead") - 1;
		break;
	case TGT_NONE:
		str = "X";
		numchar = lengthof("X") - 1;
		break;
	default:
		ASSUME_UNREACHABLE
	}
	// PrintSmallVerticalStr centered
	int sx = PANEL_X + PANEL_WIDTH - SMALL_FONT_HEIGHT - 2;
	int sy = SCREEN_Y + SCREEN_HEIGHT - 2 * SPLICON_WIDTH + (2 * SPLICON_WIDTH - numchar * SMALL_FONT_HEIGHT) / 2u;
	for (unsigned i = 0; i < numchar; i++) {
		sy += SMALL_FONT_HEIGHT;
		BYTE nCel = gbStdFontFrame[str[i]];
		// PrintSmallChar(sx + (SMALL_FONT_WIDTH - smallFontWidth[nCel]) / 2, sy, str[i], COL_GOLD);
		PrintSmallColorChar(sx + (13 - smallFontWidth[nCel]) / 2u, sy, nCel, COL_GOLD);
	}
}

static void DrawSkillIconHotKey(int x, int y, PlrSkillUse skill, int offset, const PlrSkillStruct (&hotkey)[4])
{
	int i, col;
	if (skill._suSkill == SPL_NULL) return;
	for (i = 0; i < 4; i++) {
		if (hotkey[i]._psAttack == skill)
			col = COL_GOLD;
		else if (hotkey[i]._psMove == skill)
			col = COL_BLUE;
		else
			continue;
		snprintf(tempstr, sizeof(tempstr), "#%d", i + 1);
		PrintGameStr(x + offset, y - SPLICON_HEIGHT + SMALL_FONT_HEIGHT + SPLICON_OVERY, tempstr, col);
	}
}

void DrawSkillList()
{
	int pnum = mypnum, i, j, x, y, sx, /*c,*/ st, lx, ly;
	uint64_t mask;
	bool selected;
#if SCREEN_READER_INTEGRATION
	BYTE prevSkill = currSkill._suSkill;
#endif
	PlrSkillUse plrSkill = targetSkill;
	currSkill._suSkill = SPL_INVALID;
	sx = SCREEN_CENTERX(SPLICON_WIDTH * SPLROWICONLS);
	x = sx + SPLICON_WIDTH * SPLROWICONLS - SPLICON_WIDTH;
	y = SCREEN_Y + SCREEN_HEIGHT - (128 + 17);
	//y = SCREEN_CENTERY(190) + 190;
	static_assert(RSPLTYPE_ABILITY == 0, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 1.");
	static_assert(RSPLTYPE_SPELL == 1, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 2.");
	static_assert(RSPLTYPE_INV == 2, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 3.");
	static_assert(RSPLTYPE_CHARGES == 3, "Looping over the spell-types in DrawSkillList relies on ordered, indexed enum values 4.");
	const PlrSkillUse empty = { SPL_NULL, 1 };
	PlrSkillUse plrSkills[NUM_SPELLS * 2];
	unsigned numPlrSkills = 0;
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
		case RSPLTYPE_INV:
			mask = plr._pInvSkills;
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
			const PlrSkillUse listSkill = { (BYTE)j, (BYTE)i };
			plrSkills[numPlrSkills] = listSkill;
			numPlrSkills++;
			st = i;
			if (i == RSPLTYPE_SPELL) {
				st = plr._pSkillLvl[j] > 0 ? RSPLTYPE_SPELL : RSPLTYPE_INVALID;
			}
			if ((spelldata[j].sUseFlags & plr._pSkillFlags) != spelldata[j].sUseFlags)
				st = RSPLTYPE_INVALID;
			else
				st = GetSpellTrans(st, j);
			CelDrawTrnTbl(x, y, pSpellCels, spelldata[j].sIcon, SkillTrns[st]);
			lx = x - SCREEN_X;
			ly = y - SCREEN_Y - SPLICON_HEIGHT;
			selected = POS_IN_RECT(MousePos.x, MousePos.y, lx, ly, SPLICON_WIDTH, SPLICON_HEIGHT);
			if (plrSkill._suSkill != SPL_INVALID) {
				selected = plrSkill == listSkill;
				if (selected) {
					SetCursorPos(lx + SPLICON_WIDTH / 2, ly + SPLICON_HEIGHT / 2);
				}
			}

			if (selected) {
				//CelDrawTrnTbl(x, y, pSpellCels, c, SkillTrns[st]);
				CelDrawTrnTbl(x, y, pSpellCels, SPLICONLAST, SkillTrns[st]);

				currSkill = listSkill;

				DrawSpellIconOverlay(x, y, listSkill._suSkill, listSkill._suType);

				DrawSkillIconHotKey(x, y, listSkill, SPLICON_OVERX, plr._pSkillHotKey);

				DrawSkillIconHotKey(x, y, listSkill, SPLICON_WIDTH - (6 + 7 + SPLICON_OVERX), plr._pAltSkillHotKey);
			}
			x -= SPLICON_WIDTH;
			if (x == sx - SPLICON_WIDTH) {
				x = sx + SPLICON_WIDTH * SPLROWICONLS - SPLICON_WIDTH;
				y -= SPLICON_HEIGHT;
			}
		}
		if (j != 0 && x != sx + SPLICON_WIDTH * SPLROWICONLS - SPLICON_WIDTH) {
			x -= SPLICON_WIDTH;
			if (x == sx - SPLICON_WIDTH) {
				x = sx + SPLICON_WIDTH * SPLROWICONLS - SPLICON_WIDTH;
				y -= SPLICON_HEIGHT;
			} else {
				plrSkills[numPlrSkills] = empty;
				numPlrSkills++;
			}
		}
	}
	if (deltaSkillPos.x != 0 || deltaSkillPos.y != 0) {
		targetSkill = { SPL_NULL, 0 };

		const PlrSkillUse selSkill = currSkill;
		for (unsigned i = 0; i < numPlrSkills; i++) {
			if (plrSkills[i] == selSkill) {
				while (deltaSkillPos.x != 0 && deltaSkillPos.y != 0) {
					int dx = deltaSkillPos.x < 0 ? 1 : -1;
					int dy = deltaSkillPos.y < 0 ? SPLROWICONLS : -SPLROWICONLS;
					int dn = i + dx + dy;
					if ((unsigned)dn < numPlrSkills && plrSkills[dn] != empty) {
						i = dn;
					}
					deltaSkillPos.x += deltaSkillPos.x < 0 ? 1 : -1;
					deltaSkillPos.y += deltaSkillPos.y < 0 ? 1 : -1;
				}
				while (deltaSkillPos.y != 0) {
					int dy = deltaSkillPos.y < 0 ? SPLROWICONLS : -SPLROWICONLS;
					int dn = i;
					while (true) {
						dn += dy;
						if ((unsigned)dn < numPlrSkills) {
							if (plrSkills[dn] == empty) continue;
							i = dn;
						}
						break;
					}
					deltaSkillPos.y += deltaSkillPos.y < 0 ? 1 : -1;
				}
				while (deltaSkillPos.x != 0) {
					int dx = deltaSkillPos.x < 0 ? 1 : -1;
					int dn = i;
					while (true) {
						dn += dx;
						if ((unsigned)dn < numPlrSkills) {
							if (plrSkills[dn] == empty) continue;
							if ((i / SPLROWICONLS) != dn / SPLROWICONLS) {
								i -= i % SPLROWICONLS;
								if (dx >= 0) {
									i += SPLROWICONLS - 1;
									if (plrSkills[i] == empty) {
										i--;
									}
								}
							} else {
								i = dn;
							}
						}
						break;
					}
					deltaSkillPos.x += deltaSkillPos.x < 0 ? 1 : -1;
				}
				targetSkill = plrSkills[i];
				break;
			}
		}

		deltaSkillPos = { 0, 0 };
	} else {
		targetSkill = { SPL_INVALID, 0 };
	}
#if SCREEN_READER_INTEGRATION
	if (prevSkill != currSkill._suSkill && currSkill._suSkill < NUM_SPELLS && currSkill._suSkill != SPL_NULL) {
		SpeakText(spelldata[currSkill._suSkill].sNameText);
	}
#endif
}

static void SkillListUp()
{
	deltaSkillPos.y--;
}

static void SkillListDown()
{
	deltaSkillPos.y++;
}

static void SkillListLeft()
{
	deltaSkillPos.x--;
}

static void SkillListRight()
{
	deltaSkillPos.x++;
}

void SkillListMove(int dir)
{
	switch (dir) {
	case MDIR_UP:    SkillListUp();    break;
	case MDIR_DOWN:  SkillListDown();  break;
	case MDIR_LEFT:  SkillListLeft();  break;
	case MDIR_RIGHT: SkillListRight(); break;
	default: ASSUME_UNREACHABLE;       break;
	}
}

/*
 * @brief Select the current skill to use for the (alt)action button.
 *   If shift is pressed:  the other (move/attack)skill is kept
 *            is released: the other (move/attack)skill is set to INVALID
 * @param altSkill set it as the action or the alt action skill
 */
void SetSkill(bool altSkill)
{
	PlayerStruct* p;
	BYTE sn;
	bool moveskill;

	sn = currSkill._suSkill;
	if (sn == SPL_INVALID) {
		gbSkillListFlag = false;
		return;
	}
	// TODO: add flag for movement-skills
	moveskill = sn == SPL_WALK || sn == SPL_CHARGE || sn == SPL_TELEPORT || sn == SPL_RNDTELEPORT;

	p = &myplr;
	PlrSkillStruct* psSkill = altSkill ? &p->_pAltSkill : &p->_pMainSkill;
	if (gbModBtnDown & ACTBTN_MASK(ACT_MODACT)) {
		if (moveskill) {
			psSkill->_psMove = currSkill;
		} else {
			psSkill->_psAttack = currSkill;
		}
	} else {
		if (moveskill) {
			psSkill->_psMove = currSkill;
			psSkill->_psAttack = { SPL_NULL, 0 };
		} else {
			psSkill->_psAttack = currSkill;
			psSkill->_psMove = { SPL_NULL, 0 };
		}

		gbSkillListFlag = false;
	}

	// gbRedrawFlags |= REDRAW_SPELL_ICON;
}

/*
 * @brief Select the current skill to be activated by the given hotkey
 * @param slot the index of the hotkey
 * @param altSkill type of the hotkey (true: alt-hotkey, false: normal hotkey)
 */
static void SetSkillHotKey(int slot, bool altSkill)
{
	PlayerStruct* p;
	int sn = currSkill._suSkill;
	bool moveskill;

	if (sn != SPL_INVALID) {
		// TODO: add flag for movement-skills
		moveskill = sn == SPL_WALK || sn == SPL_CHARGE || sn == SPL_TELEPORT || sn == SPL_RNDTELEPORT;

		p = &myplr;
		PlrSkillStruct* ps = altSkill ? p->_pAltSkillHotKey : p->_pSkillHotKey;
		ps = (PlrSkillStruct*)(moveskill ? &ps->_psMove : &ps->_psAttack);
		{
			int i;

			for (i = 0; i < lengthof(p->_pSkillHotKey); ++i) {
				static_assert(offsetof(PlrSkillStruct, _psAttack) == 0, "SetSkillHotKey sets the wrong skill");
				if (ps[i]._psAttack == currSkill) {
					ps[i]._psAttack = { SPL_NULL, 0 };
					if (slot == i)
						return;
				}
			}
			ps[slot]._psAttack = currSkill;
		}
	}
}

/*
 * @brief Select a skill for the current player with a hotkey
 * @param slot the index of the hotkey
 * @param altSkill type of the hotkey (true: alt-hotkey, false: normal hotkey)
 */
static void SelectHotKeySkill(int slot, bool altSkill)
{
	PlayerStruct* p;

	p = &myplr;
	PlrSkillStruct* ps = altSkill ? &p->_pAltSkill : &p->_pMainSkill;
	PlrSkillStruct* pss = &(altSkill ? p->_pAltSkillHotKey : p->_pSkillHotKey)[slot];
	*ps = *pss;
	// gbRedrawFlags |= REDRAW_SPELL_ICON;
}

void SkillHotKey(int slot, bool altSkill)
{
	if (gbSkillListFlag)
		SetSkillHotKey(slot, altSkill);
	else
		SelectHotKeySkill(slot, altSkill);
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

static void DrawFlask2(int sx, unsigned filled, int emptyCel, int fullCel, int w)
{
	const BYTE *empty, *full;
	int sy, dataSize, i;
	int8_t width;

	sy = SCREEN_Y + SCREEN_HEIGHT - 1;

	filled += FLASK_TOTAL_HEIGHT - FLASK_BULB_HEIGHT;
	unsigned emptied = FLASK_TOTAL_HEIGHT - filled;
	full = CelGetFrameClippedAt(pFlaskCels, fullCel, 0, &dataSize);

	BYTE* dst = &gpBuffer[BUFFERXY(sx, sy)];
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
	}

	if (emptied == 0)
		return;

	filled = FLASK_TOTAL_HEIGHT - emptied;
	unsigned blocks = filled / CEL_BLOCK_HEIGHT;
	empty = CelGetFrameClippedAt(pFlaskCels, emptyCel, blocks, &dataSize);
	filled = filled % CEL_BLOCK_HEIGHT; // -= blocks * CEL_BLOCK_HEIGHT;
	while (filled-- != 0) {
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

	if (gbRedrawFlags & REDRAW_RECALC_HP) {
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

	if (gbRedrawFlags & REDRAW_RECALC_MANA) {
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
	pFlaskCels = LoadFileInMem("CtrlPan\\Flasks.CEL");
	assert(pChrPanelCel == NULL);
	pChrPanelCel = CelLoadImage("Data\\Char.CEL", SPANEL_WIDTH);
	gnNumActiveWindows = 0;
	gbDragWnd = WND_NONE;
	gbInvflag = false;
	guTeamInviteRec = 0;
	guTeamInviteSent = 0;
	guTeamMute = 0;
	gbLvlbtndown = false;
	assert(pPanelButtonCels == NULL);
	pPanelButtonCels = CelLoadImage("CtrlPan\\Menu.CEL", MENUBTN_WIDTH);
	for (i = 0; i < lengthof(gabPanbtn); i++)
		gabPanbtn[i] = false;
	numpanbtns = IsLocalGame ? NUM_PANBTNS - 2 : NUM_PANBTNS;
	assert(pChrButtonCels == NULL);
	pChrButtonCels = CelLoadImage("Data\\CharBut.CEL", CHRBTN_WIDTH);
	for (i = 0; i < lengthof(gabChrbtn); i++)
		gabChrbtn[i] = false;
	gbChrbtnactive = false;
	assert(pSTextSlidCels == NULL);
	pSTextSlidCels = CelLoadImage("Data\\TextSlid.CEL", SMALL_SCROLL_WIDTH);
	assert(pDurIconCels == NULL);
	pDurIconCels = CelLoadImage("Items\\DurIcons.CEL", DURICON_WIDTH);
	// infostr[0] = '\0';
	// tempstr[0] = '\0';
	gbRedrawFlags = REDRAW_RECALC_FLASKS; // | REDRAW_DRAW_ALL;
	gbLvlUp = false;
	gbSkillListFlag = false;
	guBooktab = 0;
	assert(pSpellBkCel == NULL);
	pSpellBkCel = CelLoadImage("Data\\SpellBk.CEL", SPANEL_WIDTH);
	assert(pSBkIconCels == NULL);
	pSBkIconCels = CelLoadImage("Data\\SpellI2.CEL", SBOOK_CELWIDTH);
	assert(pSpellCels == NULL);
#if ASSET_MPL == 1
	pSpellCels = pSBkIconCels;
#else
#ifdef HELLFIRE
	pSpellCels = CelLoadImage("Data\\SpelIcon.CEL", SPLICON_WIDTH);
#else
	pSpellCels = CelLoadImage("CtrlPan\\SpelIcon.CEL", SPLICON_WIDTH);
#endif // HELLFIRE
#endif // ASSET_MPL == 1
	assert(pMapIconCels == NULL);
	pMapIconCels = CelLoadImage("Data\\MapIcon.CEL", CAMICON_WIDTH);
	gbCampaignMapFlag = CMAP_NONE;
	/*for (i = 0; i < NUM_RSPLTYPES; i++) {
		for (j = 0; j < NUM_COLORS; j++)
			SkillTrns[j] = j;
		SkillTrns[NUM_COLORS - 1] = 0;
	}*/
	LoadFileWithMem("PlrGFX\\SNone.TRN", SkillTrns[RSPLTYPE_ABILITY]);
	LoadFileWithMem("PlrGFX\\SBlue.TRN", SkillTrns[RSPLTYPE_SPELL]);
	LoadFileWithMem("PlrGFX\\SRed.TRN", SkillTrns[RSPLTYPE_SCROLL]);
	LoadFileWithMem("PlrGFX\\SOrange.TRN", SkillTrns[RSPLTYPE_CHARGES]);
	LoadFileWithMem("PlrGFX\\SGray.TRN", SkillTrns[RSPLTYPE_INVALID]);
#ifdef HELLFIRE
	LoadFileWithMem("PlrGFX\\Coral.TRN", SkillTrns[RSPLTYPE_RUNE]);
#endif
	SpellPages[0][0] = Abilities[myplr._pClass];
	assert(pGoldDropCel == NULL);
	pGoldDropCel = CelLoadImage("CtrlPan\\Golddrop.cel", GOLDDROP_WIDTH);
	gbDropGoldIndex = INVITEM_NONE;
	// gnDropGoldValue = 0;
}

void StartWndDrag(BYTE wnd)
{
	gbDragWnd = wnd;
	gnDragWndX = MousePos.x;
	gnDragWndY = MousePos.y;
}

void DoWndDrag()
{
	int dx = MousePos.x - gnDragWndX;
	int dy = MousePos.y - gnDragWndY;

	// assert(WND_VALID(gbDragWnd));
	if (dx == 0 && dy == 0)
		return;

	if (MoveWndPos(gbDragWnd, dx, dy)) {
		gnDragWndX = MousePos.x;
		gnDragWndY = MousePos.y;
	} else {
		SetCursorPos(gnDragWndX, gnDragWndY);
	}
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
	CelDraw(x, SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1] + MENUBTN_HEIGHT - 1, pPanelButtonCels, pb ? 4 : 3);
	if (!pb)
		return;
	for (i = 1; i < numpanbtns; i++) {
		y = SCREEN_Y + SCREEN_HEIGHT - PanBtnPos[i][1];
		pb = gabPanbtn[i];
		CelDraw(x, y + MENUBTN_HEIGHT - 1, pPanelButtonCels, 2);
		// print the text of the button
		PrintJustifiedString(x + 3, y + (MENUBTN_HEIGHT + SMALL_FONT_HEIGHT) / 2, x + MENUBTN_WIDTH - 1, PanBtnTxt[i], pb ? COL_GOLD : COL_WHITE, FONT_KERN_SMALL);
	}
}

/**
 * Toggles the "Skill List": the rows of known spells for quick-setting a spell that
 * show up when you click the spell slot at the control panel.
 * @param altSkill whether the cursor is moved to the active skill or altSkill (controllers-only)
 */
void HandleSkillBtn(bool altSkill)
{
	if (!gbSkillListFlag) {
		ClearPanels();
		// gamemenu_off();
		gbSkillListFlag = true;
		currSkill._suSkill = SPL_INVALID;
		targetSkill._suSkill = SPL_INVALID;
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (sgbControllerActive) {
			PlrSkillStruct* skill = altSkill ? &myplr._pAltSkill : &myplr._pMainSkill;
			targetSkill = skill->_psAttack._suSkill != SPL_NULL ? skill->_psAttack : skill->_psMove;
		}
#endif
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
	// gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
}

static bool InLvlUpRect()
{
	return POS_IN_RECT(MousePos.x, MousePos.y,
		LVLUP_LEFT, SCREEN_HEIGHT - LVLUP_OFFSET - CHRBTN_HEIGHT,
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
bool TryPanBtnClick()
{
	int i, mx, my;

	mx = MousePos.x;
	my = MousePos.y;
	for (i = gabPanbtn[PANBTN_MAINMENU] ? numpanbtns - 1 : 0; i >= 0; i--) {
		if (POS_IN_RECT(mx, my,
			PANEL_LEFT + PanBtnPos[i][0],  SCREEN_HEIGHT - PanBtnPos[i][1],
			MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
			control_set_button_down(i);
			return true;
		}
	}
	if (POS_IN_RECT(mx, my,
		PANEL_LEFT + PANEL_WIDTH - SPLICON_WIDTH,  SCREEN_HEIGHT - 2 * SPLICON_HEIGHT,
		SPLICON_WIDTH + 1, 2 * SPLICON_HEIGHT + 1)) {
		HandleSkillBtn(my < SCREEN_HEIGHT - SPLICON_HEIGHT);
		return true;
	}
	if (gbLvlUp && InLvlUpRect())
		gbLvlbtndown = true;
	return gbLvlbtndown;
}

void TryLimitedPanBtnClick()
{
	if (POS_IN_RECT(MousePos.x, MousePos.y,
		PANEL_LEFT + PanBtnPos[PANBTN_MAINMENU][0],  SCREEN_HEIGHT - PanBtnPos[PANBTN_MAINMENU][1],
		MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
		control_set_button_down(PANBTN_MAINMENU);
	} else if (gabPanbtn[PANBTN_MAINMENU] && !IsLocalGame) {
		if (POS_IN_RECT(MousePos.x, MousePos.y,
			PANEL_LEFT + PanBtnPos[PANBTN_SENDMSG][0],  SCREEN_HEIGHT - PanBtnPos[PANBTN_SENDMSG][1],
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
			StopPlrMsg();
		else
			StartPlrMsg();
		break;
	case PANBTN_TEAMBOOK:
		gbSkillListFlag = false;
		ToggleWindow(WND_TEAM);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	if (gbQtextflag)
		StopQTextMsg();
	StopHelp();
	gamemenu_off();
}

/**
 * Check if the mouse is within a control panel button that's flagged.
 * Takes apropiate action if so.
 */
void ReleasePanBtn()
{
	int i;

	static_assert(lengthof(gabPanbtn) == lengthof(PanBtnPos), "Mismatching gabPanbtn and panbtnpos tables.");
	static_assert(PANBTN_MAINMENU == 0, "CheckBtnUp needs to skip the mainmenu-button.");
	for (i = 1; i < lengthof(gabPanbtn); i++) {
		if (!gabPanbtn[i]) {
			continue;
		}

		gabPanbtn[i] = false;
		if (!POS_IN_RECT(MousePos.x, MousePos.y,
			PANEL_LEFT + PanBtnPos[i][0],  SCREEN_HEIGHT - PanBtnPos[i][1],
			MENUBTN_WIDTH + 1, MENUBTN_HEIGHT + 1)) {
			continue;
		}

		HandlePanBtn(i);

		gabPanbtn[PANBTN_MAINMENU] = false;
		// gbRedrawFlags |= REDRAW_CTRL_BUTTONS;
	}
}

void FreeControlPan()
{
	MemFreeDbg(pFlaskCels);
	MemFreeDbg(pChrPanelCel);
	MemFreeDbg(pPanelButtonCels);
	MemFreeDbg(pChrButtonCels);
	MemFreeDbg(pSTextSlidCels);
	MemFreeDbg(pDurIconCels);
	MemFreeDbg(pSpellBkCel);
#if ASSET_MPL == 1
	assert(pSpellCels == pSBkIconCels);
	pSpellCels = NULL;
#else
	MemFreeDbg(pSpellCels);
#endif
	MemFreeDbg(pSBkIconCels);
	MemFreeDbg(pMapIconCels);
	MemFreeDbg(pGoldDropCel);
}

void DrawChr()
{
	PlayerStruct* p;
	BYTE col;
	char chrstr[64];
	int screen_x, screen_y, pc, val, mindam, maxdam;
	bool showStats;

	p = &myplr;
	pc = p->_pClass;

	screen_x = SCREEN_X + gnWndCharX;
	screen_y = SCREEN_Y + gnWndCharY;
	CelDraw(screen_x, screen_y + SPANEL_HEIGHT - 1, pChrPanelCel, 1);
	PrintJustifiedString(screen_x + 5, screen_y + 19, screen_x + 144, p->_pName, COL_WHITE, FONT_KERN_SMALL);

	PrintJustifiedString(screen_x + 153, screen_y + 19, screen_x + 292, ClassStrTbl[pc], COL_WHITE, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pLevel);
	PrintJustifiedString(screen_x + 53, screen_y + 46, screen_x + 96, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pExperience);
	PrintJustifiedString(screen_x + 200, screen_y + 46, screen_x + 292, chrstr, col, FONT_KERN_SMALL);

	if (p->_pLevel == MAXCHARLEVEL) {
		copy_cstr(chrstr, "None");
		col = COL_GOLD;
	} else {
		snprintf(chrstr, sizeof(chrstr), "%d", p->_pNextExper);
		col = COL_WHITE;
	}
	PrintJustifiedString(screen_x + 200, screen_y + 71, screen_x + 292, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pGold);
	PrintJustifiedString(screen_x + 221, screen_y + 97, screen_x + 292, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseStr);
	PrintJustifiedString(screen_x + 88, screen_y + 119, screen_x + 125, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseMag);
	PrintJustifiedString(screen_x + 88, screen_y + 147, screen_x + 125, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseDex);
	PrintJustifiedString(screen_x + 88, screen_y + 175, screen_x + 125, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pBaseVit);
	PrintJustifiedString(screen_x + 88, screen_y + 203, screen_x + 125, chrstr, col, FONT_KERN_SMALL);

	showStats = p->_pStatPts <= 0;
	if (!showStats) {
		showStats = (gbModBtnDown & ACTBTN_MASK(ACT_MODACT)) != 0;
		snprintf(chrstr, sizeof(chrstr), "%d", p->_pStatPts);
		PrintJustifiedString(screen_x + 88, screen_y + 231, screen_x + 125, chrstr, COL_RED, FONT_KERN_SMALL);
		int sx = screen_x + (showStats ? CHRBTN_ALT : CHRBTN_LEFT);
		CelDraw(sx, screen_y + CHRBTN_TOP(ATTRIB_STR) + CHRBTN_HEIGHT - 1, pChrButtonCels, gabChrbtn[ATTRIB_STR] ? 2 : 1);
		CelDraw(sx, screen_y + CHRBTN_TOP(ATTRIB_MAG) + CHRBTN_HEIGHT - 1, pChrButtonCels, gabChrbtn[ATTRIB_MAG] ? 2 : 1);
		CelDraw(sx, screen_y + CHRBTN_TOP(ATTRIB_DEX) + CHRBTN_HEIGHT - 1, pChrButtonCels, gabChrbtn[ATTRIB_DEX] ? 2 : 1);
		CelDraw(sx, screen_y + CHRBTN_TOP(ATTRIB_VIT) + CHRBTN_HEIGHT - 1, pChrButtonCels, gabChrbtn[ATTRIB_VIT] ? 2 : 1);
	}

	if (p->_pHasUnidItem)
		return;

	if (showStats) {
		val = p->_pStrength;
		col = COL_WHITE;
		if (val > p->_pBaseStr)
			col = COL_BLUE;
		else if (val < p->_pBaseStr)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		PrintJustifiedString(screen_x + 135, screen_y + 119, screen_x + 172, chrstr, col, FONT_KERN_SMALL);

		val = p->_pMagic;
		col = COL_WHITE;
		if (val > p->_pBaseMag)
			col = COL_BLUE;
		else if (val < p->_pBaseMag)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		PrintJustifiedString(screen_x + 135, screen_y + 147, screen_x + 172, chrstr, col, FONT_KERN_SMALL);

		val = p->_pDexterity;
		col = COL_WHITE;
		if (val > p->_pBaseDex)
			col = COL_BLUE;
		else if (val < p->_pBaseDex)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		PrintJustifiedString(screen_x + 135, screen_y + 175, screen_x + 172, chrstr, col, FONT_KERN_SMALL);

		val = p->_pVitality;
		col = COL_WHITE;
		if (val > p->_pBaseVit)
			col = COL_BLUE;
		else if (val < p->_pBaseVit)
			col = COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d", val);
		PrintJustifiedString(screen_x + 135, screen_y + 203, screen_x + 172, chrstr, col, FONT_KERN_SMALL);
	}

	snprintf(chrstr, sizeof(chrstr), "%d/%d", p->_pIAC, p->_pIEvasion);
	// instead of (242;291) x-limits, (239;294) are used to make sure the values are displayed
	PrintJustifiedString(screen_x + 239, screen_y + 122, screen_x + 294, chrstr, COL_WHITE, -1);

	snprintf(chrstr, sizeof(chrstr), "%d/%d", p->_pIBlockChance, p->_pICritChance / 2);
	// instead of (242;291) x-limits, (241;292) are used to make sure the values are displayed
	PrintJustifiedString(screen_x + 241, screen_y + 150, screen_x + 292, chrstr, COL_WHITE, -1);

	val = p->_pIHitChance;
	col = COL_WHITE;
	if (p->_pIBaseHitBonus == IBONUS_POSITIVE)
		col = COL_BLUE;
	else if (p->_pIBaseHitBonus == IBONUS_NEGATIVE)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	PrintJustifiedString(screen_x + 242, screen_y + 178, screen_x + 291, chrstr, col, FONT_KERN_SMALL);

	col = COL_WHITE;
	mindam = (p->_pIFMinDam + p->_pILMinDam + p->_pIMMinDam + p->_pIAMinDam) >> 6;
	maxdam = (p->_pIFMaxDam + p->_pILMaxDam + p->_pIMMaxDam + p->_pIAMaxDam) >> 6;
	if (maxdam != 0)
		col = COL_BLUE;
	mindam += (p->_pISlMinDam + p->_pIBlMinDam + p->_pIPcMinDam) >> (6 + 1); // +1 is a temporary(?) adjustment for backwards compatibility
	maxdam += (p->_pISlMaxDam + p->_pIBlMaxDam + p->_pIPcMaxDam) >> (6 + 1);
	snprintf(chrstr, sizeof(chrstr), "%d-%d", mindam, maxdam);
	// instead of (242;291) x-limits, (240;293) are used to make sure the values are displayed
	PrintJustifiedString(screen_x + 240, screen_y + 206, screen_x + 293, chrstr, col, mindam < 100 ? 0 : -1);

	val = p->_pMagResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	PrintJustifiedString(screen_x + 185, screen_y + 254, screen_x + 234, chrstr, col, FONT_KERN_SMALL);

	val = p->_pFireResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	PrintJustifiedString(screen_x + 242, screen_y + 254, screen_x + 291, chrstr, col, FONT_KERN_SMALL);

	val = p->_pLghtResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	PrintJustifiedString(screen_x + 185, screen_y + 289, screen_x + 234, chrstr, col, FONT_KERN_SMALL);

	val = p->_pAcidResist;
	if (val < MAXRESIST) {
		col = val >= 0 ? COL_WHITE : COL_RED;
		snprintf(chrstr, sizeof(chrstr), "%d%%", val);
	} else {
		col = COL_GOLD;
		copy_cstr(chrstr, "MAX");
	}
	PrintJustifiedString(screen_x + 242, screen_y + 289, screen_x + 291, chrstr, col, FONT_KERN_SMALL);

	val = p->_pMaxHP;
	col = val <= p->_pMaxHPBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%d", val >> 6);
	PrintJustifiedString(screen_x + 87, screen_y + 260, screen_x + 126, chrstr, col, FONT_KERN_SMALL); // 88, 125 -> 87, 126 otherwise '1000' is truncated
	if (p->_pHitPoints != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pHitPoints >> 6);
	PrintJustifiedString(screen_x + 134, screen_y + 260, screen_x + 173, chrstr, col, FONT_KERN_SMALL); // 135, 172 -> 134, 173 otherwise '1000' is truncated

	val = p->_pMaxMana;
	col = val <= p->_pMaxManaBase ? COL_WHITE : COL_BLUE;
	snprintf(chrstr, sizeof(chrstr), "%d", val >> 6);
	PrintJustifiedString(screen_x + 87, screen_y + 288, screen_x + 126, chrstr, col, FONT_KERN_SMALL); // 88, 125 -> 87, 126 otherwise '1000' is truncated
	if (p->_pMana != val)
		col = COL_RED;
	snprintf(chrstr, sizeof(chrstr), "%d", p->_pMana >> 6);
	PrintJustifiedString(screen_x + 134, screen_y + 288, screen_x + 173, chrstr, col, FONT_KERN_SMALL); // 135, 172 -> 134, 173 otherwise '1000' is truncated
}

void DrawLevelUpIcon()
{
	int screen_x, screen_y;

	screen_x = SCREEN_X + LVLUP_LEFT;
	screen_y = SCREEN_Y + SCREEN_HEIGHT - LVLUP_OFFSET;
	PrintJustifiedString(screen_x - 38, screen_y + 20, screen_x - 38 + 120, "Level Up", COL_WHITE, FONT_KERN_SMALL);
	CelDraw(screen_x, screen_y, pChrButtonCels, gbLvlbtndown ? 2 : 1);
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
	x -= width / 2u;
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
#if SCREEN_READER_INTEGRATION
	unsigned len1 = strlen(text1);
	unsigned len = len1 + strlen(text2) + 2;
	char *text = new char[len];
	memcpy(text, text1, len1);
	text[len1] = '\n';
	memcpy(&text[len1 + 1], text1, len - (len1 + 1));
	SpeakText(text);
	free(text);
#endif
	return result;
}

/*
 * Return the screen position of the given tile (x;y).
 *
 * @param x the x index of the tile
 * @param y the y index of the tile
 * @return the screen x/y-coordinates of the tile
 */
static POS32 GetMousePos(int x, int y)
{
	POS32 pos;

	x -= ViewX;
	y -= ViewY;

	pos = { 0, 0 };
	SHIFT_GRID(pos.x, pos.y, -y, x);

	pos.x *= TILE_WIDTH / 2;
	pos.y *= TILE_HEIGHT / 2;

	pos.x += ScrollInfo._sxoff;
	pos.y += ScrollInfo._syoff;

	if (gbZoomInFlag) {
		pos.x <<= 1;
		pos.y <<= 1;
	}

	pos.x += SCREEN_WIDTH / 2u;
	pos.y += SCREEN_HEIGHT / 2u;

	return pos;
}

static void GetItemInfo(const ItemStruct* is)
{
	infoclr = ItemColor(is);
	if (is->_itype != ITYPE_GOLD) {
		snprintf(infostr, sizeof(infostr), "%s", ItemName(is));
	} else {
		snprintf(infostr, sizeof(infostr), "%d gold %s", is->_ivalue, get_pieces_str(is->_ivalue));
	}
}
#define TOOLTIP_OFFSET   32
#define TOOLTIP_HEIGHT   16
#define TOOLTIP2_HEIGHT  26
#define HEALTHBAR_HEIGHT 4
#define HEALTHBAR_WIDTH  66
static int DrawTooltip(const char* text, int x, int y, BYTE col)
{
	int width, result = 0;
	BYTE* dst;
	const int border = 4;

	width = GetSmallStringWidth(text) + 2 * border;

	if (y < 0)
		return result;
	x -= width / 2u;
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
#if SCREEN_READER_INTEGRATION
	SpeakText(text);
#endif
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
	static_assert(HEALTHBAR_HEIGHT < BORDER_BOTTOM, "DrawHealthBar might draw out of the buffer.");
	x -= HEALTHBAR_WIDTH / 2;
	if (x < 0)
		x = 0;
	else if (x > SCREEN_WIDTH - HEALTHBAR_WIDTH)
		x = SCREEN_WIDTH - HEALTHBAR_WIDTH;

	// draw gray border
	dst = &gpBuffer[SCREENXY(x, y)];
	for (int i = 0; i < HEALTHBAR_HEIGHT; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_YELLOW + 8, HEALTHBAR_WIDTH);

	// draw the bar
	//width = (HEALTHBAR_WIDTH - 2) * hp / maxhp;
	dhp = (maxhp + 7) >> 3;
	dw = ((HEALTHBAR_WIDTH - 2) >> 3);
	for (w = 0, h = 0; h < hp; h += dhp, w += dw) {
	}
	dst = &gpBuffer[SCREENXY(x + 1, y + 1)];
	for (int i = 0; i < HEALTHBAR_HEIGHT - 2; i++, dst += BUFFER_WIDTH)
		memset(dst, PAL16_RED + 11, w);
}

static void DrawTrigInfo()
{
	POS32 pos;

	if (pcurstrig >= MAXTRIGGERS + 1) {
		// portal
		MissileStruct* mis = &missile[pcurstrig - (MAXTRIGGERS + 1)];
		if (mis->_miType == MIS_TOWN) {
			copy_cstr(infostr, "Town Portal");
			snprintf(tempstr, sizeof(tempstr), "(%s)", players[mis->_miSource]._pName);
			pos = GetMousePos(pcurspos.x, pcurspos.y);
			pos.y -= TILE_HEIGHT * 2 + TOOLTIP_OFFSET;
			DrawTooltip2(infostr, tempstr, pos.x, pos.y, COL_WHITE);
		} else {
			if (!currLvl._dSetLvl) {
				copy_cstr(infostr, "Portal to The Unholy Altar");
			} else {
				copy_cstr(infostr, "Portal back to hell");
			}
			pos = GetMousePos(pcurspos.x, pcurspos.y);
			pos.y -= TILE_HEIGHT * 2 + TOOLTIP_OFFSET;
			DrawTooltip(infostr, pos.x, pos.y, COL_WHITE);
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
			snprintf(infostr, sizeof(infostr), "Back to %s", AllLevels[trigs[pcurstrig]._tlvl].dLevelName);
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
			//case SL_MAZE:
			//	copy_cstr(infostr, "To Maze");
			//	break;
			case SL_POISONWATER:
				copy_cstr(infostr, "To A Dark Passage");
				break;
			//case SL_VILEBETRAYER:
			//	copy_cstr(infostr, "To The Unholy Altar");
			//	break;
			default:
				ASSUME_UNREACHABLE
			}
			break;
		case DVL_DWM_TWARPDN:
			switch (trigs[pcurstrig]._ttype) {
			case WRPT_TOWN_L1:
				copy_cstr(infostr, "Down to dungeon");
				break;
			case WRPT_TOWN_L2:
				copy_cstr(infostr, "Down to catacombs");
				break;
			case WRPT_TOWN_L3:
				copy_cstr(infostr, "Down to caves");
				break;
			case WRPT_TOWN_L4:
				copy_cstr(infostr, "Down to hell");
				break;
#ifdef HELLFIRE
			case WRPT_TOWN_L6:
				copy_cstr(infostr, "Down to nest");
				break;
			case WRPT_TOWN_L5:
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

	pos = GetMousePos(pcurspos.x, pcurspos.y);
	pos.y -= TILE_HEIGHT + TOOLTIP_OFFSET;
	DrawTooltip(infostr, pos.x, pos.y, COL_WHITE);
}

void DrawInfoStr()
{
	POS32 pos;

	if (ITEM_VALID(pcursitem)) {
		ItemStruct* is = &items[pcursitem];
		GetItemInfo(is);
		pos = GetMousePos(is->_ix, is->_iy);
		pos.y -= TOOLTIP_OFFSET;
		DrawTooltip(infostr, pos.x, pos.y, infoclr);
	} else if (OBJ_VALID(pcursobj)) {
		GetObjectStr(pcursobj);
		ObjectStruct* os = &objects[pcursobj];
		pos = GetMousePos(os->_ox, os->_oy);
		pos.y -= TILE_HEIGHT + TOOLTIP_OFFSET;
		DrawTooltip(infostr, pos.x, pos.y, infoclr);
	} else if (MON_VALID(pcursmonst)) {
		MonsterStruct* mon = &monsters[pcursmonst];
		DISABLE_WARNING(deprecated-declarations, deprecated-declarations, 4996)
		strcpy(infostr, mon->_mName); // TNR_NAME or a monster's name
		ENABLE_WARNING(deprecated-declarations, deprecated-declarations, 4996)
		pos = GetMousePos(mon->_mx, mon->_my);
		pos.x += mon->_mxoff;
		pos.y += mon->_myoff;
		pos.y -= ((mon->_mSelFlag & 6) ? TILE_HEIGHT * 2 : TILE_HEIGHT) + TOOLTIP_OFFSET;
		pos.x += DrawTooltip(infostr, pos.x, pos.y, mon->_mNameColor);
		DrawHealthBar(mon->_mhitpoints, mon->_mmaxhp, pos.x, pos.y + TOOLTIP_HEIGHT - HEALTHBAR_HEIGHT / 2);
	} else if (PLR_VALID(pcursplr)) {
		PlayerStruct* p = &players[pcursplr];
		pos = GetMousePos(p->_px, p->_py);
		pos.x += p->_pxoff;
		pos.y += p->_pyoff;
		pos.y -= TILE_HEIGHT * 2 + TOOLTIP_OFFSET;
		snprintf(infostr, sizeof(infostr), p->_pManaShield == 0 ? "%s(%d)" : "%s(%d)*", ClassStrTbl[p->_pClass], p->_pLevel);
		pos.x += DrawTooltip2(p->_pName, infostr, pos.x, pos.y, COL_GOLD);
		DrawHealthBar(p->_pHitPoints, p->_pMaxHP, pos.x, pos.y + TOOLTIP2_HEIGHT - HEALTHBAR_HEIGHT / 2);
	} else if (gbSkillListFlag) {
		if (currSkill._suSkill == SPL_INVALID || currSkill._suSkill == SPL_NULL)
			return;
		const char* src;
		switch (currSkill._suType) {
		case RSPLTYPE_ABILITY:
			src = "Ability";
			break;
		case RSPLTYPE_SPELL:
			src = "Spell";
			break;
		case RSPLTYPE_INV:
			src = SPELL_RUNE(currSkill._suSkill) ? "Rune" : "Scroll";
			break;
		case RSPLTYPE_CHARGES:
			src = "Equipment";
			break;
		//case RSPLTYPE_INVALID:
		//	break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		DrawTooltip2(spelldata[currSkill._suSkill].sNameText, src, MousePos.x, MousePos.y - (SPLICON_HEIGHT / 4 + TOOLTIP_OFFSET), COL_WHITE);
	} else if (gbCampaignMapFlag != CMAP_NONE) {
		if (currCamEntry.ceIndex == 0)
			return;
		const char* type;
		switch (currCamEntry.ceDunType) {
		case DTYPE_CATHEDRAL:
			type = "Cathedral";
			break;
		case DTYPE_CATACOMBS:
			type = "Catacombs";
			break;
		case DTYPE_CAVES:
			type = "Caves";
			break;
		case DTYPE_HELL:
			type = "Hell";
			break;
#ifdef HELLFIRE
		case DTYPE_CRYPT:
			type = "Crypt";
			break;
		case DTYPE_NEST:
			type = "Nest";
			break;
#endif
		default:
			ASSUME_UNREACHABLE
			break;
		}
		BYTE lvl = currCamEntry.ceLevel;
		if (gnDifficulty == DIFF_NIGHTMARE) {
			lvl += NIGHTMARE_LEVEL_BONUS;
		} else if (gnDifficulty == DIFF_HELL) {
			lvl += HELL_LEVEL_BONUS;
		}
		snprintf(infostr, sizeof(infostr), gbCampaignMapFlag == CMAP_IDENTIFIED ? "(lvl: %d)" : "(lvl: \?\?)", lvl);
		DrawTooltip2(type, infostr, MousePos.x, MousePos.y - (CAMICON_HEIGHT / 4 + TOOLTIP_OFFSET), COL_WHITE);
	} else if (INVIDX_VALID(pcursinvitem)) {
		const ItemStruct* is = PlrItem(mypnum, pcursinvitem);
		DrawItemDetails(is);
	} else if (TRIG_VALID(pcurstrig)) {
		DrawTrigInfo();
	} else if (pcursicon >= CURSOR_FIRSTITEM) {
		GetItemInfo(&myplr._pHoldItem);
		DrawTooltip(infostr, MousePos.x + cursW / 2u, MousePos.y - TOOLTIP_OFFSET, infoclr);
	} else if (stextflag != STORE_NONE) {
		const ItemStruct* si = CurrentStoreItem();
		if (si != NULL) {
			DrawItemDetails(si);
		}
	}
}

static bool CheckInChrBtnRect(int i)
{
	int sx = (gbModBtnDown & ACTBTN_MASK(ACT_MODCTX)) ? CHRBTN_ALT : CHRBTN_LEFT;
	return POS_IN_RECT(MousePos.x, MousePos.y,
			gnWndCharX + sx, gnWndCharY + CHRBTN_TOP(i),
			CHRBTN_WIDTH, CHRBTN_HEIGHT);
}

void CheckChrBtnClick()
{
	int i;

	if (myplr._pStatPts != 0) {
		if (gbChrbtnactive)
			return; // true;
		for (i = 0; i < lengthof(gabChrbtn); i++) {
			if (!CheckInChrBtnRect(i))
				continue;

			gabChrbtn[i] = true;
			gbChrbtnactive = true;
			return; // true;
		}
	}
	StartWndDrag(WND_CHAR);
	// return false;
}

void ReleaseChrBtn()
{
	int i;

	gbChrbtnactive = false;
	static_assert(lengthof(gabChrbtn) == 4, "Table gabChrbtn does not work with ReleaseChrBtns function.");
	for (i = 0; i < lengthof(gabChrbtn); ++i) {
		if (gabChrbtn[i]) {
			gabChrbtn[i] = false;
			if (CheckInChrBtnRect(i)) {
				static_assert((int)CMD_ADDSTR + 1 == (int)CMD_ADDMAG, "ReleaseChrBtn expects ordered CMD_ADD values I.");
				static_assert((int)CMD_ADDMAG + 1 == (int)CMD_ADDDEX, "ReleaseChrBtn expects ordered CMD_ADD values II.");
				static_assert((int)CMD_ADDDEX + 1 == (int)CMD_ADDVIT, "ReleaseChrBtn expects ordered CMD_ADD values III.");
				NetSendCmd(CMD_ADDSTR + i);
			}
		}
	}
}

/**
 * @brief Draw a small text box with transparent background with a separator.
 *  used as background to items and in stores.
 * @param x: the starting x-coordinate of the text box
 * @param y: the starting y-coordinate of the text box
 */
void DrawSTextBox(int x, int y)
{
	// draw the box
	DrawColorTextBox(x, y, STPANEL_WIDTH, TPANEL_HEIGHT, COL_GOLD);
	// add separator
	DrawColorTextBoxSLine(x, y, STPANEL_WIDTH, 5 * 12 + 14);
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

	x = SCREEN_X + PANEL_RIGHT - (SPLICON_WIDTH + MANA_FLASK_WIDTH + DURICON_WIDTH);

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
	} else if (p->_pInvSkills & SPELL_MASK(sn)) {
		st = RSPLTYPE_INV;
	} else if (CheckSpell(mypnum, sn)) {
		st = RSPLTYPE_SPELL;
	} else {
		st = RSPLTYPE_INVALID;
	}
	return st;
}

void DrawSpellBook()
{
	int pnum, i, sn, mana, lvl, sx, yp, offset;
	BYTE st;
	uint64_t spl;

	// back panel
	sx = SCREEN_X + gnWndBookX;
	yp = SCREEN_Y + gnWndBookY;
	CelDraw(sx, yp + SPANEL_HEIGHT - 1, pSpellBkCel, 1);
	// selected page
	snprintf(tempstr, sizeof(tempstr), "%d.", guBooktab + 1);
	PrintJustifiedString(sx + 2, yp + SPANEL_HEIGHT - 7, sx + SPANEL_WIDTH, tempstr, COL_WHITE, 0);

#if SCREEN_READER_INTEGRATION
	BYTE prevSkill = currSkill._suSkill;
#endif
	currSkill._suSkill = SPL_INVALID;

	pnum = mypnum;
	spl = plr._pMemSkills | plr._pISpells | plr._pAblSkills | plr._pInvSkills;

	yp += SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx += SBOOK_CELBORDER;
	for (i = 0; i < lengthof(SpellPages[guBooktab]); i++) {
		sn = SpellPages[guBooktab][i];
		if (sn != SPL_INVALID && (spl & SPELL_MASK(sn))) {
			st = GetSBookTrans(sn);
			lvl = plr._pHasUnidItem ? -1 : plr._pSkillLvl[sn]; // SPLLVL_UNDEF : spllvl
			// assert(lvl >= 0 || lvl == -1);
			mana = 0;
			switch (st) {
			case RSPLTYPE_ABILITY:
				copy_cstr(tempstr, "Ability");
				// lvl = -1; // SPLLVL_UNDEF
				break;
			case RSPLTYPE_INV:
				if (SPELL_RUNE(sn)) {
					copy_cstr(tempstr, "Rune");
				} else {
					copy_cstr(tempstr, "Scroll");
				}
				break;
			case RSPLTYPE_CHARGES:
				copy_cstr(tempstr, "Equipment");
				break;
			case RSPLTYPE_SPELL:
			case RSPLTYPE_INVALID:
				if (lvl < 0) {
					copy_cstr(tempstr, "Spell");
					break;
				}
				if (lvl != 0) {
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
			PrintGameStr(sx + SBOOK_LINE_TAB, yp - 23 + offset, spelldata[sn].sNameText, COL_WHITE);
			PrintGameStr(sx + SBOOK_LINE_TAB, yp - 12 + offset, tempstr, COL_WHITE);

			if (offset == 0) {
				if (mana != 0)
					cat_str(tempstr, offset, "Mana: %d  ", mana);
				if (min != -1)
					cat_str(tempstr, offset, "Dam: %d-%d", min, max);
				PrintGameStr(sx + SBOOK_LINE_TAB, yp - 1, tempstr, COL_WHITE);
			}
			const PlrSkillUse bookSkill = { (BYTE)sn, (BYTE)st };
			if ((spelldata[sn].sUseFlags & plr._pSkillFlags) != spelldata[sn].sUseFlags)
				st = RSPLTYPE_INVALID;
			CelDrawTrnTbl(sx, yp, pSBkIconCels, spelldata[sn].sIcon, SkillTrns[GetSpellTrans(st, sn)]);
			// TODO: differenciate between Atk/Move skill ? Add icon for primary skills?
			if ((sn == plr._pAltSkill._psAttack._suSkill && st == plr._pAltSkill._psAttack._suType)
			 || (sn == plr._pAltSkill._psMove._suSkill && st == plr._pAltSkill._psMove._suType)) {
				CelDrawTrnTbl(sx, yp, pSBkIconCels, SPLICONLAST, SkillTrns[RSPLTYPE_ABILITY]);
			}
			if (POS_IN_RECT(MousePos.x, MousePos.y,
				sx - SCREEN_X, yp - SCREEN_Y - SBOOK_CELHEIGHT,
				SBOOK_CELWIDTH, SBOOK_CELHEIGHT)) {
				currSkill = bookSkill;
			}
		}
		yp += SBOOK_CELBORDER + SBOOK_CELHEIGHT;
	}
#if SCREEN_READER_INTEGRATION
	if (prevSkill != currSkill._suSkill && currSkill._suSkill < NUM_SPELLS && currSkill._suSkill != SPL_NULL) {
		SpeakText(spelldata[currSkill._suSkill].sNameText);
	}
#endif
}

void CheckBookClick(bool altSkill)
{
	int dx, dy;

	if (currSkill._suSkill != SPL_INVALID) {
		SetSkill(altSkill);
		return;
	}
	if (altSkill) {
		return;
	}

	dx = MousePos.x - (gnWndBookX + SBOOK_LEFT_BORDER);
	dy = MousePos.y - (gnWndBookY + SBOOK_TOP_BORDER);
	if (dx < 0 || dy < 0)
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
	} else {
		StartWndDrag(WND_BOOK);
	}
}

const char* get_pieces_str(int nGold)
{
	return nGold != 1 ? "pieces" : "piece";
}

void DrawGoldSplit()
{
	int screen_x, screen_y, amount;

	screen_x = SCREEN_X + gnWndInvX + (SPANEL_WIDTH - GOLDDROP_WIDTH) / 2;
	screen_y = SCREEN_Y + gnWndInvY + 178;
	// draw the background
	CelDraw(screen_x, screen_y, pGoldDropCel, 1);
	// draw the info-text
	amount = PlrItem(mypnum, gbDropGoldIndex)->_ivalue;
	snprintf(tempstr, sizeof(tempstr), "You have %d gold", amount);
	PrintJustifiedString(screen_x + 15, screen_y - (18 + 18 * 4), screen_x + GOLDDROP_WIDTH - 15, tempstr, COL_GOLD, FONT_KERN_SMALL);
	snprintf(tempstr, sizeof(tempstr), "%s.  How many do", get_pieces_str(amount));
	PrintJustifiedString(screen_x + 15, screen_y - (18 + 18 * 3), screen_x + GOLDDROP_WIDTH - 15, tempstr, COL_GOLD, FONT_KERN_SMALL);
	PrintJustifiedString(screen_x + 15, screen_y - (18 + 18 * 2), screen_x + GOLDDROP_WIDTH - 15, "you want to remove?", COL_GOLD, FONT_KERN_SMALL);
	// draw the edit-field
	screen_x += 37;
	screen_y -= 18 + 18 * 1;
	amount = gnDropGoldValue;
	if (amount > 0) {
		snprintf(tempstr, sizeof(tempstr), "%d", amount);
		// PrintGameStr(screen_x, screen_y, tempstr, COL_WHITE);
		// screen_x += GetSmallStringWidth(tempstr);
		screen_x = PrintLimitedString(screen_x, screen_y, tempstr, GOLDDROP_WIDTH - (37 * 2), COL_WHITE, FONT_KERN_SMALL);
	}
	screen_x += 2;
	DrawSingleSmallPentSpn(screen_x, screen_y);
}

static void control_remove_gold()
{
	BYTE cii = gbDropGoldIndex;

	assert(cii >= INVITEM_INV_FIRST && cii <= INVITEM_INV_LAST);
	static_assert((int)INVITEM_INV_LAST - (int)INVITEM_INV_FIRST < UCHAR_MAX, "control_remove_gold sends inv item index in BYTE field.");
	static_assert(GOLD_MAX_LIMIT <= UINT16_MAX, "control_remove_gold send gold pile value using uint16_t.");
	NetSendCmdParamBW(CMD_SPLITPLRGOLD, cii - INVITEM_INV_FIRST, gnDropGoldValue);
}

static void control_inc_dropgold(int value)
{
	int newValue;
	int maxValue = PlrItem(mypnum, gbDropGoldIndex)->_ivalue;

	newValue = gnDropGoldValue * 10 + value;
	if (newValue <= maxValue)
		gnDropGoldValue = newValue;
}

void control_drop_gold(int vkey)
{
	// assert(myplr._pHitPoints != 0 || vkey == DVL_VK_ESCAPE);

	if (vkey == DVL_VK_RETURN) {
		if (gnDropGoldValue > 0)
			control_remove_gold();
	} else if (vkey == DVL_VK_BACK) {
		gnDropGoldValue /= 10;
		return;
	} else if (vkey == DVL_VK_DELETE) {
		gnDropGoldValue = 0;
		return;
	} else if (vkey >= DVL_VK_0 && vkey <= DVL_VK_9) {
		control_inc_dropgold(vkey - DVL_VK_0);
		return;
	} else if (vkey >= DVL_VK_NUMPAD0 && vkey <= DVL_VK_NUMPAD9) {
		control_inc_dropgold(vkey - DVL_VK_NUMPAD0);
		return;
	}
	gbDropGoldIndex = INVITEM_NONE;
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
	PrintGameStr(x, y - 1, label, color);
}

static bool PlrHasTeam()
{
	int pnum;

	for (pnum = 0; pnum < MAX_PLRS; pnum++)
		if (pnum != mypnum && plr._pTeam == myplr._pTeam && plr._pActive)
			return true;
	return false;
}

#define TBOOKTABS ((MAX_PLRS + NUM_BOOK_ENTRIES - 1) / NUM_BOOK_ENTRIES)
void DrawTeamBook()
{
	int i, pnum, sx, yp;
	bool hasTeam;
	BYTE st;

	// back panel
	sx = SCREEN_X + gnWndTeamX;
	yp = SCREEN_Y + gnWndTeamY;
	CelDraw(sx, yp + SPANEL_HEIGHT - 1, pSpellBkCel, 1);
	// selected page
	snprintf(tempstr, sizeof(tempstr), "%d.", guTeamTab + 1);
	PrintJustifiedString(sx + 2, yp + SPANEL_HEIGHT - 7, sx + SPANEL_WIDTH, tempstr, COL_WHITE, 0);

	hasTeam = PlrHasTeam();

	yp += SBOOK_TOP_BORDER + SBOOK_CELHEIGHT;
	sx += SBOOK_CELBORDER;
	for (i = 0; i < NUM_BOOK_ENTRIES; i++) {
		pnum = i + guTeamTab * NUM_BOOK_ENTRIES;
		if (pnum >= MAX_PLRS)
			break;
		if (!plr._pActive)
			continue;
		// name
		PrintLimitedString(sx + SBOOK_LINE_TAB, yp - 25, plr._pName, SBOOK_LINE_LENGTH, COL_WHITE, 0);
		// class(level) - team
		static_assert(MAXCHARLEVEL < 100, "Level must fit to the TeamBook.");
		snprintf(tempstr, sizeof(tempstr), "%s (lvl:%2d) %c", ClassStrTbl[plr._pClass], plr._pLevel, 'a' + plr._pTeam);
		PrintGameStr(sx + SBOOK_LINE_TAB, yp - 13, tempstr, COL_WHITE);

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
		CelDrawTrnTbl(sx, yp, pSBkIconCels, ClassIconTbl[plr._pClass], SkillTrns[st]);

		yp += SBOOK_CELBORDER + SBOOK_CELHEIGHT;
	}
}

void CheckTeamClick()
{
	int dx, dy;

	dx = MousePos.x - (gnWndTeamX + SBOOK_LEFT_BORDER);
	dy = MousePos.y - (gnWndTeamY + SBOOK_TOP_BORDER);
	if (dx < 0 || dy < 0) {
		return;
	}

	if (dy < NUM_BOOK_ENTRIES * (SBOOK_CELBORDER + SBOOK_CELHEIGHT)) {
		int pnum = dy / (SBOOK_CELBORDER + SBOOK_CELHEIGHT);
		dy = dy % (SBOOK_CELBORDER + SBOOK_CELHEIGHT);
		pnum += guTeamTab * NUM_BOOK_ENTRIES;
		if (pnum >= MAX_PLRS || !plr._pActive) {
			StartWndDrag(WND_TEAM);
			return;
		}
		if (dx <= SBOOK_CELWIDTH) {
			// clicked on the icon
			SetupPlrMsg(pnum);
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

void DrawGolemBar()
{
	MonsterStruct* mon = &monsters[mypnum];

	if (mon->_mmode <= MM_INGAME_LAST) {
		DrawHealthBar(mon->_mhitpoints, mon->_mmaxhp, LIFE_FLASK_X + LIFE_FLASK_WIDTH / 2 - SCREEN_X, SCREEN_Y + SCREEN_HEIGHT - 1 - HEALTHBAR_HEIGHT + 2 - SCREEN_Y);
	}
}

#define CAM_RADIUS 5
#define CAMROWICONLS (2 * CAM_RADIUS + 1)
static void control_addmappos(int x, int y, BYTE type, BYTE idx, WORD available, int* border)
{
	int db = -1;
	camEntries[x][y].ceDunType = type;
	camEntries[x][y].ceIndex = idx;
	camEntries[x][y].ceAvailable = ((available >> (idx - 1)) & 1) ? SDL_TRUE : SDL_FALSE;
	if (x > 8 - CAM_RADIUS && camEntries[x - 1][y].ceDunType == DTYPE_TOWN) {
		camEntries[x - 1][y].ceDunType = NUM_DTYPES + type;
		db++;
	}
	if (x < 8 + CAM_RADIUS && camEntries[x + 1][y].ceDunType == DTYPE_TOWN) {
		camEntries[x + 1][y].ceDunType = NUM_DTYPES + type;
		db++;
	}
	if (y > 8 - CAM_RADIUS && camEntries[x][y - 1].ceDunType == DTYPE_TOWN) {
		camEntries[x][y - 1].ceDunType = NUM_DTYPES + type;
		db++;
	}
	if (y < 8 + CAM_RADIUS && camEntries[x][y + 1].ceDunType == DTYPE_TOWN) {
		camEntries[x][y + 1].ceDunType = NUM_DTYPES + type;
		db++;
	}
	*border += db;
}

static void control_setmaplevel(int x, int y, BYTE lvl, BYTE dlvl)
{
	if (camEntries[x][y].ceIndex != 0 && (camEntries[x][y].ceLevel == 0 || camEntries[x][y].ceLevel > lvl)) {
		camEntries[x][y].ceLevel = lvl;

		lvl += dlvl;
		if (lvl > MAXCAMPAIGNLVL)
			lvl = MAXCAMPAIGNLVL;
		control_setmaplevel(x + 1, y + 0, lvl, dlvl);
		control_setmaplevel(x - 1, y + 0, lvl, dlvl);
		control_setmaplevel(x + 0, y + 1, lvl, dlvl);
		control_setmaplevel(x + 0, y - 1, lvl, dlvl);
	}
}

void InitCampaignMap(int cii)
{
	BYTE idx = 0;
	WORD available;
	int border = 1;
	// TODO: prevent map-open after CMD_USEPLRMAP?
	camItemIndex = cii;
	ItemStruct* is = PlrItem(mypnum, camItemIndex);
	available = is->_ivalue;
	// generate the map
	SetRndSeed(is->_iSeed);

	memset(camEntries, 0, sizeof(camEntries));
	static_assert(DTYPE_TOWN == 0, "InitCampaignMap must be adjusted.");
	control_addmappos(lengthof(camEntries) / 2, lengthof(camEntries[0]) / 2, random_(200, NUM_DTYPES - 1) + 1, ++idx, available, &border);

	int numMaps = MAXCAMPAIGNSIZE - 6;
	BYTE lvl = (is->_iCreateInfo & CF_LEVEL);
	BYTE dlvl = 2;
	for (unsigned i = 0; i < is->_iNumAffixes; i++) {
		const ItemAffixStruct* ias = &is->_iAffixes[i];
		int v = ias->asValue0;
		switch (ias->asPower) {
		case IMP_LVLMOD:  lvl += v;     break;
		case IMP_AREAMOD: numMaps += v; break;
		case IMP_LVLGAIN: dlvl += v;    break;
		case IMP_SETLVL:  lvl = v;      break;
		default: ASSUME_UNREACHABLE;    break;
		}
	}
	lvl -= HELL_LEVEL_BONUS;
	for (int i = 0; i < numMaps - 1; i++) {
		int step = random_low(201, border) + 1;
		for (int x = 0; x < lengthof(camEntries); x++) {
			for (int y = 0; y < lengthof(camEntries[0]); y++) {
				if (camEntries[x][y].ceDunType > NUM_DTYPES && --step == 0) {
					BYTE type = random_(202, 3 * (NUM_DTYPES - 1)) + 1;
					if (type >= NUM_DTYPES) {
						type = camEntries[x][y].ceDunType - NUM_DTYPES;
					}
					control_addmappos(x, y, type, ++idx, available, &border);
					x = lengthof(camEntries);
					break;
				}
			}
		}
	}

	control_setmaplevel(lengthof(camEntries) / 2, lengthof(camEntries[0]) / 2, lvl, dlvl);

	gbCampaignMapFlag = is->_iMagical == ITEM_QUALITY_NORMAL || is->_iIdentified ? CMAP_IDENTIFIED : CMAP_UNIDENTIFIED;
}

/*
 * @brief Manipulate the campaign map.
 *   If shift is pressed:  the inventory is kept open
 *            is released: the inventory is closed
 * @param altAction: if set the map is just closed
 */
void TryCampaignMapClick(bool altAction)
{
	if (!altAction) {
		BYTE mIdx = currCamEntry.ceIndex;
		if (mIdx != 0) {
			if (currCamEntry.ceAvailable) {
				selCamEntry = currCamEntry;
				NetSendCmdBParam2(CMD_USEPLRMAP, camItemIndex, mIdx - 1);
				if (!(gbModBtnDown & ACTBTN_MASK(ACT_MODACT))) {
					if (gbInvflag) {
						gbInvflag = false;
						/* gbInvflag =*/ ToggleWindow(WND_INV);
					}
				}
			} else {
				return;
			}
		}
	}

	gbCampaignMapFlag = CMAP_NONE;
}

void DrawCampaignMap()
{
	int x, y, sx, sy, lx, ly;
	sx = SCREEN_CENTERX(CAMICON_WIDTH * CAMROWICONLS);
	sy = SCREEN_CENTERY(CAMICON_HEIGHT * CAMROWICONLS) + CAMICON_HEIGHT;

	sx += CAMICON_WIDTH * (CAM_RADIUS - lengthof(camEntries) / 2);
	sy += CAMICON_HEIGHT * (CAM_RADIUS - lengthof(camEntries[0]) / 2);

	currCamEntry = { 0, 0, 0, FALSE };
	for (int cy = 0; cy < lengthof(camEntries[0]); cy++) {
		for (int cx = 0; cx < lengthof(camEntries); cx++) {
			const CampaignMapEntry &cme = camEntries[cy][cx];
			if (cme.ceIndex != 0) {
				x = sx + CAMICON_WIDTH * (cx/* + CAM_RADIUS  - lengthof(camEntries) / 2*/);
				y = sy + CAMICON_HEIGHT * (cy/* + CAM_RADIUS - lengthof(camEntries[0]) / 2*/);
				lx = x - SCREEN_X;
				ly = y - (SCREEN_Y + CAMICON_HEIGHT);

				const BYTE* tbl = ColorTrns[COLOR_TRN_GRAY];
				if (cme.ceAvailable) { // not visited
					if ((cx == lengthof(camEntries) / 2 && cy == lengthof(camEntries[0]) / 2) // starting place
					 || (camEntries[cy + 0][cx + 1].ceIndex != 0 && !camEntries[cy + 0][cx + 1].ceAvailable) // neighbour of a visited place
					 || (camEntries[cy + 0][cx - 1].ceIndex != 0 && !camEntries[cy + 0][cx - 1].ceAvailable)
					 || (camEntries[cy + 1][cx + 0].ceIndex != 0 && !camEntries[cy + 1][cx + 0].ceAvailable)
					 || (camEntries[cy - 1][cx + 0].ceIndex != 0 && !camEntries[cy - 1][cx + 0].ceAvailable)) {
						tbl = ColorTrns[0];
					} else {
						continue;
					}
				}

				CelDrawTrnTbl(x, y, pMapIconCels, cme.ceDunType, tbl);

				if (POS_IN_RECT(MousePos.x, MousePos.y, lx, ly, CAMICON_WIDTH, CAMICON_HEIGHT)) {
					CelDrawTrnTbl(x, y, pMapIconCels, CAMICONLAST, tbl);

					currCamEntry = cme;
				}
			}
		}
	}
}

DEVILUTION_END_NAMESPACE
