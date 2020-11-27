/**
 * @file stores.cpp
 *
 * Implementation of functionality for stores and towner dialogs.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define STORAGE_LIMIT		NUM_INV_GRID_ELEM + (MAXBELTITEMS > NUM_INVLOC ? MAXBELTITEMS : NUM_INVLOC)

int stextup;
int storenumh;
int stextlhold;
ItemStruct boyitem;
int stextshold;
ItemStruct premiumitem[SMITH_PREMIUM_ITEMS];
BYTE *pSTextBoxCels;
int premiumlevel;
int talker;
STextStruct stext[STORE_LINES];
BOOL stextsize;
int stextsmax;
ItemStruct storehold[STORAGE_LIMIT];
int gossipstart;
ItemStruct witchitem[WITCH_ITEMS];
BOOL stextscrl;
int numpremium;
ItemStruct healitem[HEALER_ITEMS];
ItemStruct golditem;
char storehidx[STORAGE_LIMIT];
BYTE *pSTextSlidCels;
int stextvhold;
int stextsel;
char stextscrldbtn;
int gossipend;
BYTE *pSPentSpn2Cels;
int stextsidx;
int boylevel;
ItemStruct smithitem[SMITH_ITEMS];
int stextdown;
char stextscrlubtn;
char stextflag;

/** Maps from towner IDs to NPC names. */
const char *const talkname[9] = {
	"Griswold",
	"Pepin",
	"",
	"Ogden",
	"Cain",
	"Farnham",
	"Adria",
	"Gillian",
	"Wirt"
};

void InitStores()
{
	int i;

	pSTextBoxCels = LoadFileInMem("Data\\TextBox2.CEL", NULL);
	pSPentSpn2Cels = LoadFileInMem("Data\\PentSpn2.CEL", NULL);
	pSTextSlidCels = LoadFileInMem("Data\\TextSlid.CEL", NULL);
	ClearSText(0, STORE_LINES);
	stextflag = STORE_NONE;
	stextsize = FALSE;
	stextscrl = FALSE;
	numpremium = 0;
	premiumlevel = 1;

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		premiumitem[i]._itype = ITYPE_NONE;

	boyitem._itype = ITYPE_NONE;
	boylevel = 0;
}

int PentSpn2Spin()
{
	return (SDL_GetTicks() / 50) % 8 + 1;
}

void SetupTownStores()
{
	int i, l;

	SetRndSeed(glSeedTbl[currlevel] * SDL_GetTicks());
	if (gbMaxPlayers == 1) {
		l = 0;
		for (i = 0; i < NUMLEVELS; i++) {
			if (plr[myplr]._pLvlVisited[i])
				l = i;
		}
	} else {
		l = plr[myplr]._pLevel >> 1;
	}
	l += 2;
	if (l < 6)
		l = 6;
	if (l > 16)
		l = 16;
	SpawnStoreGold();
	SpawnSmith(l);
	SpawnWitch(l);
	SpawnHealer(l);
	SpawnBoy(plr[myplr]._pLevel);
	SpawnPremium(plr[myplr]._pLevel);
}

void FreeStoreMem()
{
	MemFreeDbg(pSTextBoxCels);
	MemFreeDbg(pSPentSpn2Cels);
	MemFreeDbg(pSTextSlidCels);
}

static void DrawSTextBack()
{
	CelDraw(PANEL_X + 344, 327 + SCREEN_Y + UI_OFFSET_Y, pSTextBoxCels, 1, 271);
	trans_rect(PANEL_LEFT + 347, UI_OFFSET_Y + 28, 265, 297);
}

void PrintSString(int x, int y, BOOL cjustflag, const char *str, char col, int val)
{
	int xx, yy;
	int len, width, sx, sy, i, k, s;
	BYTE c;
	char valstr[32];

	s = y * 12 + stext[y]._syoff;
	if (stextsize)
		xx = PANEL_X + 32;
	else
		xx = PANEL_X + 352;
	sx = xx + x;
	sy = s + 44 + SCREEN_Y + UI_OFFSET_Y;
	len = strlen(str);
	if (stextsize)
		yy = 577;
	else
		yy = 257;
	k = 0;
	if (cjustflag) {
		width = 0;
		for (i = 0; i < len; i++)
			width += fontkern[fontframe[gbFontTransTbl[(BYTE)str[i]]]] + 1;
		if (width < yy)
			k = (yy - width) >> 1;
		sx += k;
	}
	if (stextsel == y) {
		CelDraw(sx - 20, sy + 1, pSPentSpn2Cels, PentSpn2Spin(), 12);
	}
	for (i = 0; i < len; i++) {
		c = fontframe[gbFontTransTbl[(BYTE)str[i]]];
		k += fontkern[c] + 1;
		if (c != '\0' && k <= yy) {
			PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + 1;
	}
	if (!cjustflag && val >= 0) {
		snprintf(valstr, sizeof(valstr), "%i", val);
		sx = PANEL_X + 592 - x;
		for (i = strlen(valstr) - 1; i >= 0; i--) {
			c = fontframe[gbFontTransTbl[(BYTE)valstr[i]]];
			sx -= fontkern[c] + 1;
			if (c != '\0') {
				PrintChar(sx, sy, c, col);
			}
		}
	}
	if (stextsel == y) {
		CelDraw(cjustflag ? (xx + x + k + 4) : (PANEL_X + 596 - x), sy + 1, pSPentSpn2Cels, PentSpn2Spin(), 12);
	}
}

void DrawSLine(int y)
{
	int xy, yy, width, line, sy;

	sy = y * 12;
	if (stextsize) {
		xy = SCREENXY(PANEL_LEFT + 26, 25 + UI_OFFSET_Y);
		yy = BUFFER_WIDTH * (sy + 38 + SCREEN_Y + UI_OFFSET_Y) + 26 + PANEL_X;
		width = 586 / 4;           // BUGFIX: should be 587, not 586
		line = BUFFER_WIDTH - 586; // BUGFIX: should be 587, not 586
	} else {
		xy = SCREENXY(PANEL_LEFT + 346, 25 + UI_OFFSET_Y);
		yy = BUFFER_WIDTH * (sy + 38 + SCREEN_Y + UI_OFFSET_Y) + 346 + PANEL_X;
		width = 266 / 4;           // BUGFIX: should be 267, not 266
		line = BUFFER_WIDTH - 266; // BUGFIX: should be 267, not 266
	}

	/// ASSERT: assert(gpBuffer);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[xy];
	dst = &gpBuffer[yy];

	for (i = 0; i < 3; i++, src += BUFFER_WIDTH, dst += BUFFER_WIDTH)
		memcpy(dst, src, BUFFER_WIDTH - line);
}

static void DrawSSlider(int y1, int y2)
{
	int yd1, yd2, yd3;

	yd1 = y1 * 12 + 44 + SCREEN_Y + UI_OFFSET_Y;
	yd2 = y2 * 12 + 44 + SCREEN_Y + UI_OFFSET_Y;
	if (stextscrlubtn != -1)
		CelDraw(PANEL_X + 601, yd1, pSTextSlidCels, 12, 12);
	else
		CelDraw(PANEL_X + 601, yd1, pSTextSlidCels, 10, 12);
	if (stextscrldbtn != -1)
		CelDraw(PANEL_X + 601, yd2, pSTextSlidCels, 11, 12);
	else
		CelDraw(PANEL_X + 601, yd2, pSTextSlidCels, 9, 12);
	yd1 += 12;
	for (yd3 = yd1; yd3 < yd2; yd3 += 12) {
		CelDraw(PANEL_X + 601, yd3, pSTextSlidCels, 14, 12);
	}
	if (stextsel == 22)
		yd3 = stextlhold;
	else
		yd3 = stextsel;
	if (storenumh > 1)
		yd3 = 1000 * (stextsidx + ((yd3 - stextup) >> 2)) / (storenumh - 1) * (y2 * 12 - y1 * 12 - 24) / 1000;
	else
		yd3 = 0;
	CelDraw(PANEL_X + 601, (y1 + 1) * 12 + 44 + SCREEN_Y + UI_OFFSET_Y + yd3, pSTextSlidCels, 13, 12);
}

void DrawSTextHelp()
{
	stextsel = -1;
	stextsize = TRUE;
}

void ClearSText(int s, int e)
{
	int i;

	for (i = s; i < e; i++) {
		stext[i]._sx = 0;
		stext[i]._syoff = 0;
		stext[i]._sstr[0] = 0;
		stext[i]._sjust = FALSE;
		stext[i]._sclr = COL_WHITE;
		stext[i]._sline = FALSE;
		stext[i]._ssel = FALSE;
		stext[i]._sval = -1;
	}
}

static void AddSLine(int y)
{
	STextStruct *ss;

	ss = &stext[y];
	ss->_sx = 0;
	ss->_syoff = 0;
	ss->_sstr[0] = 0;
	ss->_sline = TRUE;
}

static void AddSTextVal(int y, int val)
{
	stext[y]._sval = val;
}

static void OffsetSTextY(int y, int yo)
{
	stext[y]._syoff = yo;
}

static void AddSText(int x, int y, BOOL j, const char *str, char clr, BOOL sel)
{
	STextStruct *ss;

	ss = &stext[y];
	ss->_sx = x;
	ss->_syoff = 0;
	SStrCopy(ss->_sstr, str, sizeof(ss->_sstr));
	ss->_sjust = j;
	ss->_sclr = clr;
	ss->_sline = FALSE;
	ss->_ssel = sel;
}

static void PrintStoreItem(const ItemStruct *is, int l, char iclr)
{
	int cursor;
	char sstr[128];

	cursor = 0;
	if (is->_iIdentified) {
		if (is->_iPrePower != IPL_INVALID) {
			PrintItemPower(is->_iPrePower, is);
			cat_str(sstr, cursor, "%s", tempstr);
		}
		if (is->_iSufPower != IPL_INVALID) {
			PrintItemPower(is->_iSufPower, is);
			if (cursor != 0)
				cat_cstr(sstr, cursor, ",  ");
			cat_str(sstr, cursor, "%s", tempstr);
		}
		if (is->_iMagical == ITEM_QUALITY_UNIQUE) {
			if (cursor != 0) // should not happen, but better safe than sorry
				cat_cstr(sstr, cursor, ",  ");
			cat_cstr(sstr, cursor, "Unique Item");
		}
	}
	if (is->_iMiscId == IMISC_STAFF && is->_iMaxCharges != 0) {
		if (cursor != 0)
			cat_cstr(sstr, cursor, ",  ");
		cat_str(sstr, cursor, "Charges: %i/%i", is->_iCharges, is->_iMaxCharges);
	}
	if (cursor != 0) {
		AddSText(40, l++, FALSE, sstr, iclr, FALSE);
		cursor = 0;
	}
	if (is->_iClass == ICLASS_WEAPON) {
#ifdef HELLFIRE
		if (is->_iMinDam == is->_iMaxDam) {
			if (is->_iMaxDur != DUR_INDESTRUCTIBLE)
				cat_str(sstr, cursor, "Damage: %i  Dur: %i/%i", is->_iMinDam, is->_iDurability, is->_iMaxDur);
			else
				cat_str(sstr, cursor, "Damage: %i  Indestructible", is->_iMinDam);
		} else
#endif
			if (is->_iMaxDur != DUR_INDESTRUCTIBLE)
				cat_str(sstr, cursor, "Damage: %i-%i  Dur: %i/%i", is->_iMinDam, is->_iMaxDam, is->_iDurability, is->_iMaxDur);
			else
				cat_str(sstr, cursor, "Damage: %i-%i  Indestructible", is->_iMinDam, is->_iMaxDam);
	} else if (is->_iClass == ICLASS_ARMOR) {
		if (is->_iMaxDur != DUR_INDESTRUCTIBLE)
			cat_str(sstr, cursor, "Armor: %i  Dur: %i/%i", is->_iAC, is->_iDurability, is->_iMaxDur);
		else
			cat_str(sstr, cursor, "Armor: %i  Indestructible", is->_iAC);
	}
	if ((is->_iMinStr | is->_iMinMag | is->_iMinDex) != 0) {
		if (cursor != 0)
			cat_cstr(sstr, cursor, ",  ");
		cat_cstr(sstr, cursor, "Required:");
		if (is->_iMinStr != 0)
			cat_str(sstr, cursor, " %i Str", is->_iMinStr);
		if (is->_iMinMag != 0)
			cat_str(sstr, cursor, " %i Mag", is->_iMinMag);
		if (is->_iMinDex != 0)
			cat_str(sstr, cursor, " %i Dex", is->_iMinDex);
	}
	if (cursor != 0)
		AddSText(40, l++, FALSE, sstr, iclr, FALSE);
}

static void AddStoreFrame(const char* title, BOOL backSel)
{
	AddSText(0, 1, TRUE, title, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, backSel);
	OffsetSTextY(22, 6);
}

static void S_StartSmith()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 1, TRUE, "Welcome to the", COL_GOLD, FALSE);
	AddSText(0, 3, TRUE, "Blacksmith's shop", COL_GOLD, FALSE);
	AddSText(0, 7, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 10, TRUE, "Talk to Griswold", COL_BLUE, TRUE);
	AddSText(0, 12, TRUE, "Buy basic items", COL_WHITE, TRUE);
	AddSText(0, 14, TRUE, "Buy premium items", COL_WHITE, TRUE);
	AddSText(0, 16, TRUE, "Sell items", COL_WHITE, TRUE);
	AddSText(0, 18, TRUE, "Repair items", COL_WHITE, TRUE);
	AddSText(0, 20, TRUE, "Leave the shop", COL_WHITE, TRUE);
	AddSLine(5);
	storenumh = 20;
}

static char StoreItemColor(ItemStruct *is)
{
	if (!is->_iStatFlag)
		return COL_RED;
	if (is->_iMagical != ITEM_QUALITY_NORMAL)
		return COL_BLUE;
	return COL_WHITE;
}

static void S_ScrollSBuy()
{
	ItemStruct *is;
	int l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	is = &smithitem[stextsidx];
	for (l = 5; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			ItemStatOk(myplr, is);
			iclr = StoreItemColor(is);

			if (is->_iMagical) {
				AddSText(20, l, FALSE, is->_iIName, iclr, TRUE);
			} else {
				AddSText(20, l, FALSE, is->_iName, iclr, TRUE);
			}

			AddSTextVal(l, is->_iIvalue);
			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
			is++;
		}
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartSBuy()
{
	int i;

	storenumh = 0;
	for (i = 0; smithitem[i]._itype != ITYPE_NONE; i++)
		storenumh++;

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsidx = 0;
	S_ScrollSBuy();

	snprintf(tempstr, sizeof(tempstr), "I have these items for sale :           Your gold : %i", plr[myplr]._pGold);
	AddStoreFrame(tempstr, FALSE);
}

static void S_ScrollSPBuy()
{
	ItemStruct* is;
	int idx, l, boughtitems;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	boughtitems = stextsidx;
	for (idx = 0; boughtitems != 0; idx++)
		if (premiumitem[idx]._itype != ITYPE_NONE)
			boughtitems--;

	for (l = 5; l < 20 && idx < SMITH_PREMIUM_ITEMS; ) {
		is = &premiumitem[idx];
		if (is->_itype != ITYPE_NONE) {
			ItemStatOk(myplr, is);
			iclr = StoreItemColor(is);
			AddSText(20, l, FALSE, is->_iIName, iclr, TRUE);
			AddSTextVal(l, is->_iIvalue);
			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
			l += 4;
		}
		idx++;
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static BOOL S_StartSPBuy()
{
	int i;

	storenumh = 0;
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		if (premiumitem[i]._itype != ITYPE_NONE)
			storenumh++;

	if (storenumh == 0) {
		StartStore(STORE_SMITH);
		stextsel = 14;
		return FALSE;
	}

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsidx = 0;
	S_ScrollSPBuy();

	snprintf(tempstr, sizeof(tempstr), "I have these premium items for sale :   Your gold : %i", plr[myplr]._pGold);
	AddStoreFrame(tempstr, FALSE);
	return TRUE;
}

static void AddStoreSell(ItemStruct *is, int i)
{
	copy_pod(storehold[storenumh], *is);

	is = &storehold[storenumh];
	if (is->_iMagical != ITEM_QUALITY_NORMAL && is->_iIdentified)
		is->_ivalue = is->_iIvalue;

	if ((is->_ivalue >>= 2) == 0)
		is->_ivalue = 1;
	is->_iIvalue = is->_ivalue;

	storehidx[storenumh++] = i;
}

static BOOL SmithSellOk(const ItemStruct *is)
{
	return is->_itype != ITYPE_NONE
		&& is->_itype != ITYPE_MISC
		&& is->_itype != ITYPE_GOLD
		&& is->_itype != ITYPE_MEAT
		&& is->_itype != ITYPE_STAFF
		&& (is->IDidx < IDI_FIRSTQUEST || is->IDidx > IDI_LASTQUEST)
		&& is->IDidx != IDI_LAZSTAFF;
}

static void S_ScrollSSell()
{
	ItemStruct* is;
	int idx, l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	idx = stextsidx;
	for (l = 5; l < 20; l += 4) {
		is = &storehold[idx];
		if (is->_itype != ITYPE_NONE) {
			iclr = StoreItemColor(is);

			if (is->_iMagical && is->_iIdentified) {
				AddSText(20, l, FALSE, is->_iIName, iclr, TRUE);
				AddSTextVal(l, is->_iIvalue);
			} else {
				AddSText(20, l, FALSE, is->_iName, iclr, TRUE);
				AddSTextVal(l, is->_ivalue);
			}

			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
		}
		idx++;
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartSSell()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++)
		if (SmithSellOk(pi))
			AddStoreSell(pi, i);
#ifdef HELLFIRE
	pi = p->SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++)
		if (SmithSellOk(pi))
			AddStoreSell(pi, -(i + 1));
#endif

	stextsize = TRUE;
	if (storenumh == 0) {
		stextscrl = FALSE;
		snprintf(tempstr, sizeof(tempstr), "You have nothing I want.            Your gold : %i", p->_pGold);
	} else {
		stextscrl = TRUE;
		stextsidx = 0;
		S_ScrollSSell();
		snprintf(tempstr, sizeof(tempstr), "Which item is for sale?            Your gold : %i", p->_pGold);
	}
	AddStoreFrame(tempstr, TRUE);
}

static BOOL SmithRepairOk(const ItemStruct *is)
{
	return is->_itype != ITYPE_NONE
		&& is->_itype != ITYPE_MISC
		&& is->_itype != ITYPE_GOLD
		&& is->_itype != ITYPE_MEAT
		&& is->_iDurability != is->_iMaxDur;
}

static void AddStoreHoldRepair(ItemStruct *is, int i)
{
	ItemStruct *holditem;
	int v;

	holditem = &storehold[storenumh];
	copy_pod(*holditem, *is);
	if (holditem->_iMagical != ITEM_QUALITY_NORMAL && holditem->_iIdentified)
		holditem->_ivalue = 30 * holditem->_iIvalue / 100;
	v = holditem->_ivalue * (100 * (holditem->_iMaxDur - holditem->_iDurability) / holditem->_iMaxDur) / 100;
	if (v == 0) {
		if (holditem->_iMagical != ITEM_QUALITY_NORMAL && holditem->_iIdentified)
			return;
		v = 1;
	}
	if (v > 1)
		v >>= 1;
	holditem->_iIvalue = v;
	holditem->_ivalue = v;
	storehidx[storenumh] = i;
	storenumh++;
}

static void S_StartSRepair()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++)
		if (SmithRepairOk(pi))
			AddStoreHoldRepair(pi, -(i + 1));
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++)
		if (SmithRepairOk(pi))
			AddStoreHoldRepair(pi, i);

	stextsize = TRUE;
	if (storenumh == 0) {
		stextscrl = FALSE;
		snprintf(tempstr, sizeof(tempstr), "You have nothing to repair.            Your gold : %i", p->_pGold);
	} else {
		stextscrl = TRUE;
		stextsidx = 0;
		S_ScrollSSell();
		snprintf(tempstr, sizeof(tempstr), "Repair which item?            Your gold : %i", p->_pGold);
	}
	AddStoreFrame(tempstr, TRUE);
}

static void S_StartWitch()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 2, TRUE, "Witch's shack", COL_GOLD, FALSE);
	AddSText(0, 9, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 12, TRUE, "Talk to Adria", COL_BLUE, TRUE);
	AddSText(0, 14, TRUE, "Buy items", COL_WHITE, TRUE);
	AddSText(0, 16, TRUE, "Sell items", COL_WHITE, TRUE);
	AddSText(0, 18, TRUE, "Recharge staves", COL_WHITE, TRUE);
	AddSText(0, 20, TRUE, "Leave the shack", COL_WHITE, TRUE);
	AddSLine(5);
	storenumh = 20;
}

static void S_ScrollWBuy()
{
	ItemStruct *is;
	int l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	is = &witchitem[stextsidx];
	for (l = 5; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			SetBookLevel(myplr, is);
			ItemStatOk(myplr, is);
			iclr = StoreItemColor(is);

			if (is->_iMagical) {
				AddSText(20, l, FALSE, is->_iIName, iclr, TRUE);
			} else {
				AddSText(20, l, FALSE, is->_iName, iclr, TRUE);
			}

			AddSTextVal(l, is->_iIvalue);
			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
			is++;
		}
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartWBuy()
{
	int i;

	storenumh = 0;
	for (i = 0; witchitem[i]._itype != ITYPE_NONE; i++)
		storenumh++;

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsidx = 0;
	S_ScrollWBuy();

	snprintf(tempstr, sizeof(tempstr), "I have these items for sale :           Your gold : %i", plr[myplr]._pGold);
	AddStoreFrame(tempstr, FALSE);
}

static BOOL WitchSellOk(const ItemStruct *is)
{
	return (is->_itype == ITYPE_MISC || is->_itype == ITYPE_STAFF)
		&& (is->IDidx < IDI_FIRSTQUEST || is->IDidx > IDI_LASTQUEST)
		&& is->IDidx != IDI_LAZSTAFF;
}

static void S_StartWSell()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++)
		if (WitchSellOk(pi))
			AddStoreSell(pi, i);
	pi = p->SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++)
		if (WitchSellOk(pi))
			AddStoreSell(pi, -(i + 1));

	stextsize = TRUE;
	if (storenumh == 0) {
		stextscrl = FALSE;
		snprintf(tempstr, sizeof(tempstr), "You have nothing I want.            Your gold : %i", p->_pGold);
	} else {
		stextscrl = TRUE;
		stextsidx = 0;
		S_ScrollSSell();
		snprintf(tempstr, sizeof(tempstr), "Which item is for sale?            Your gold : %i", p->_pGold);
	}
	AddStoreFrame(tempstr, TRUE);
}

static BOOL WitchRechargeOk(const ItemStruct *is)
{
	return is->_itype == ITYPE_STAFF && is->_iCharges != is->_iMaxCharges;
}

static void AddStoreHoldRecharge(ItemStruct *is, int i)
{
	ItemStruct *holditem;

	holditem = &storehold[storenumh];
	copy_pod(*holditem, *is);
	holditem->_ivalue += spelldata[holditem->_iSpell].sStaffCost;
	holditem->_ivalue = holditem->_ivalue * (100 * (holditem->_iMaxCharges - holditem->_iCharges) / holditem->_iMaxCharges) / 100 >> 1;
	holditem->_iIvalue = holditem->_ivalue;
	storehidx[storenumh] = i;
	storenumh++;
}

static void S_StartWRecharge()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++)
		if (WitchRechargeOk(pi))
			AddStoreHoldRecharge(pi, -(i + 1));
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++)
		if (WitchRechargeOk(pi))
			AddStoreHoldRecharge(pi, i);

	stextsize = TRUE;
	if (storenumh == 0) {
		stextscrl = FALSE;
		snprintf(tempstr, sizeof(tempstr), "You have nothing to recharge.            Your gold : %i", p->_pGold);
	} else {
		stextscrl = TRUE;
		stextsidx = 0;
		S_ScrollSSell();
		snprintf(tempstr, sizeof(tempstr), "Recharge which item?            Your gold : %i", p->_pGold);
	}
	AddStoreFrame(tempstr, TRUE);
}

static void S_StartNoMoney()
{
	StartStore(stextshold);
	stextscrl = FALSE;
	stextsize = TRUE;
	ClearSText(5, 23);
	AddSText(0, 14, TRUE, "You do not have enough gold", COL_WHITE, TRUE);
}

static void S_StartNoRoom()
{
	StartStore(stextshold);
	stextscrl = FALSE;
	ClearSText(5, 23);
	AddSText(0, 14, TRUE, "You do not have enough room in inventory", COL_WHITE, TRUE);
}

static void S_StartConfirm()
{
	BOOL idprint;
	char iclr;

	StartStore(stextshold);
	stextscrl = FALSE;
	ClearSText(5, 23);
	iclr = StoreItemColor(&plr[myplr].HoldItem);

	idprint = plr[myplr].HoldItem._iMagical != ITEM_QUALITY_NORMAL;

	if (stextshold == STORE_SIDENTIFY)
		idprint = FALSE;
	if (plr[myplr].HoldItem._iMagical != ITEM_QUALITY_NORMAL && !plr[myplr].HoldItem._iIdentified) {
		if (stextshold == STORE_SSELL)
			idprint = FALSE;
		if (stextshold == STORE_WSELL)
			idprint = FALSE;
		if (stextshold == STORE_SREPAIR)
			idprint = FALSE;
		if (stextshold == STORE_WRECHARGE)
			idprint = FALSE;
	}
	if (idprint)
		AddSText(20, 8, FALSE, plr[myplr].HoldItem._iIName, iclr, FALSE);
	else
		AddSText(20, 8, FALSE, plr[myplr].HoldItem._iName, iclr, FALSE);

	AddSTextVal(8, plr[myplr].HoldItem._iIvalue);
	PrintStoreItem(&plr[myplr].HoldItem, 9, iclr);

	switch (stextshold) {
	case STORE_BBOY:
		copy_cstr(tempstr, "Do we have a deal?");
		break;
	case STORE_SIDENTIFY:
		copy_cstr(tempstr, "Are you sure you want to identify this item?");
		break;
	case STORE_HBUY:
	case STORE_SPBUY:
	case STORE_WBUY:
	case STORE_SBUY:
		copy_cstr(tempstr, "Are you sure you want to buy this item?");
		break;
	case STORE_WRECHARGE:
		copy_cstr(tempstr, "Are you sure you want to recharge this item?");
		break;
	case STORE_SSELL:
	case STORE_WSELL:
		copy_cstr(tempstr, "Are you sure you want to sell this item?");
		break;
	case STORE_SREPAIR:
		copy_cstr(tempstr, "Are you sure you want to repair this item?");
		break;
	}
	AddSText(0, 15, TRUE, tempstr, COL_WHITE, FALSE);
	AddSText(0, 18, TRUE, "Yes", COL_WHITE, TRUE);
	AddSText(0, 20, TRUE, "No", COL_WHITE, TRUE);
}

static void S_StartBoy()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 2, TRUE, "Wirt the Peg-legged boy", COL_GOLD, FALSE);
	AddSLine(5);
	if (boyitem._itype != ITYPE_NONE) {
		AddSText(0, 8, TRUE, "Talk to Wirt", COL_BLUE, TRUE);
		AddSText(0, 12, TRUE, "I have something for sale,", COL_GOLD, FALSE);
		AddSText(0, 14, TRUE, "but it will cost 50 gold", COL_GOLD, FALSE);
		AddSText(0, 16, TRUE, "just to take a look. ", COL_GOLD, FALSE);
		AddSText(0, 18, TRUE, "What have you got?", COL_WHITE, TRUE);
		AddSText(0, 20, TRUE, "Say goodbye", COL_WHITE, TRUE);
	} else {
		AddSText(0, 12, TRUE, "Talk to Wirt", COL_BLUE, TRUE);
		AddSText(0, 18, TRUE, "Say goodbye", COL_WHITE, TRUE);
	}
}

static void S_StartBBoy()
{
	int iclr;

	stextsize = TRUE;
	stextscrl = FALSE;
	snprintf(tempstr, sizeof(tempstr), "I have this item for sale :           Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);

	ItemStatOk(myplr, &boyitem);
	iclr = StoreItemColor(&boyitem);

	if (boyitem._iMagical != ITEM_QUALITY_NORMAL)
		AddSText(20, 10, FALSE, boyitem._iIName, iclr, TRUE);
	else
		AddSText(20, 10, FALSE, boyitem._iName, iclr, TRUE);

#ifdef HELLFIRE
	AddSTextVal(10, boyitem._iIvalue - (boyitem._iIvalue >> 2));
#else
	AddSTextVal(10, boyitem._iIvalue + (boyitem._iIvalue >> 1));
#endif
	PrintStoreItem(&boyitem, 11, iclr);
	AddSText(0, 22, TRUE, "Leave", COL_WHITE, TRUE);
	OffsetSTextY(22, 6);
}

static void S_StartHealer()
{
#ifdef HELLFIRE
	if (plr[myplr]._pHitPoints != plr[myplr]._pMaxHP) {
		PlaySFX(IS_CAST8);
	}
	plr[myplr]._pHitPoints = plr[myplr]._pMaxHP;
	plr[myplr]._pHPBase = plr[myplr]._pMaxHPBase;
	drawhpflag = TRUE;
#endif
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 1, TRUE, "Welcome to the", COL_GOLD, FALSE);
	AddSText(0, 3, TRUE, "Healer's home", COL_GOLD, FALSE);
	AddSText(0, 9, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 12, TRUE, "Talk to Pepin", COL_BLUE, TRUE);
#ifdef HELLFIRE
	AddSText(0, 14, TRUE, "Buy items", COL_WHITE, TRUE);
	AddSText(0, 16, TRUE, "Leave Healer's home", COL_WHITE, TRUE);
#else
	AddSText(0, 14, TRUE, "Receive healing", COL_WHITE, TRUE);
	AddSText(0, 16, TRUE, "Buy items", COL_WHITE, TRUE);
	AddSText(0, 18, TRUE, "Leave Healer's home", COL_WHITE, TRUE);
#endif
	AddSLine(5);
	storenumh = 20;
}

static void S_ScrollHBuy()
{
	ItemStruct* is;
	int l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	is = &healitem[stextsidx];
	for (l = 5; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			ItemStatOk(myplr, is);
			iclr = StoreItemColor(is);

			AddSText(20, l, FALSE, is->_iName, iclr, TRUE);
			AddSTextVal(l, is->_iIvalue);
			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
			is++;
		}
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartHBuy()
{
	int i;

	storenumh = 0;
	for (i = 0; healitem[i]._itype != ITYPE_NONE; i++)
		storenumh++;

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsidx = 0;
	S_ScrollHBuy();

	snprintf(tempstr, sizeof(tempstr), "I have these items for sale :           Your gold : %i", plr[myplr]._pGold);
	AddStoreFrame(tempstr, FALSE);
}

static void S_StartStory()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 2, TRUE, "The Town Elder", COL_GOLD, FALSE);
	AddSText(0, 9, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 12, TRUE, "Talk to Cain", COL_BLUE, TRUE);
	AddSText(0, 14, TRUE, "Identify an item", COL_WHITE, TRUE);
	AddSText(0, 18, TRUE, "Say goodbye", COL_WHITE, TRUE);
	AddSLine(5);
}

static BOOL IdItemOk(const ItemStruct *is)
{
	return is->_itype != ITYPE_NONE
		&& is->_iMagical != ITEM_QUALITY_NORMAL
		&& !is->_iIdentified;
}

static void AddStoreHoldId(ItemStruct *is, int i)
{
	ItemStruct *holditem;

	holditem = &storehold[storenumh];
	copy_pod(*holditem, *is);
	holditem->_ivalue = 100;
	holditem->_iIvalue = 100;
	storehidx[storenumh] = i;
	storenumh++;
}

static void S_StartSIdentify()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++)
		if (IdItemOk(pi))
			AddStoreHoldId(pi, -(i + 1));
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++)
		if (IdItemOk(pi))
			AddStoreHoldId(pi, i);

	stextsize = TRUE;
	if (storenumh == 0) {
		stextscrl = FALSE;
		snprintf(tempstr, sizeof(tempstr), "You have nothing to identify.            Your gold : %i", p->_pGold);
	} else {
		stextscrl = TRUE;
		stextsidx = 0;
		S_ScrollSSell();
		snprintf(tempstr, sizeof(tempstr), "Identify which item?            Your gold : %i", p->_pGold);
	}
	AddStoreFrame(tempstr, TRUE);
}

static void S_StartIdShow()
{
	char iclr;

	StartStore(stextshold);
	stextscrl = FALSE;
	ClearSText(5, 23);

	iclr = StoreItemColor(&plr[myplr].HoldItem);

	AddSText(0, 7, TRUE, "This item is:", COL_WHITE, FALSE);
	AddSText(20, 11, FALSE, plr[myplr].HoldItem._iIName, iclr, FALSE);
	PrintStoreItem(&plr[myplr].HoldItem, 12, iclr);
	AddSText(0, 18, TRUE, "Done", COL_WHITE, TRUE);
}

static void S_StartTalk()
{
	int i, sn, sn2, la;

	stextsize = FALSE;
	stextscrl = FALSE;
	snprintf(tempstr, sizeof(tempstr), "Talk to %s", talkname[talker]);
	AddSText(0, 2, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(5);
#ifdef SPAWN
	snprintf(tempstr, sizeof(tempstr), "Talking to %s", talkname[talker]);
	AddSText(0, 10, TRUE, tempstr, COL_WHITE, FALSE);
	AddSText(0, 12, TRUE, "is not available", COL_WHITE, FALSE);
	AddSText(0, 14, TRUE, "in the shareware", COL_WHITE, FALSE);
	AddSText(0, 16, TRUE, "version", COL_WHITE, FALSE);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
	return;
#endif
	sn = 0;
	for (i = 0; i < MAXQUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && ((DWORD *)&Qtalklist[talker])[i] != -1 && quests[i]._qlog)
			sn++;
	}

	if (sn > 6) {
		sn = 14 - (sn >> 1);
		la = 1;
	} else {
		sn = 15 - sn;
		la = 2;
	}

	sn2 = sn - 2;

	for (i = 0; i < MAXQUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && ((DWORD *)&Qtalklist[talker])[i] != -1 && quests[i]._qlog) {
			AddSText(0, sn, TRUE, questlist[i]._qlstr, COL_WHITE, TRUE);
			sn += la;
		}
	}
	AddSText(0, sn2, TRUE, "Gossip", COL_BLUE, TRUE);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
}

static void S_StartTavern()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 1, TRUE, "Welcome to the", COL_GOLD, FALSE);
	AddSText(0, 3, TRUE, "Rising Sun", COL_GOLD, FALSE);
	AddSText(0, 9, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 12, TRUE, "Talk to Ogden", COL_BLUE, TRUE);
	AddSText(0, 18, TRUE, "Leave the tavern", COL_WHITE, TRUE);
	AddSLine(5);
	storenumh = 20;
}

static void S_StartBarMaid()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 2, TRUE, "Gillian", COL_GOLD, FALSE);
	AddSText(0, 9, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 12, TRUE, "Talk to Gillian", COL_BLUE, TRUE);
	AddSText(0, 18, TRUE, "Say goodbye", COL_WHITE, TRUE);
	AddSLine(5);
	storenumh = 20;
}

static void S_StartDrunk()
{
	stextsize = FALSE;
	stextscrl = FALSE;
	AddSText(0, 2, TRUE, "Farnham the Drunk", COL_GOLD, FALSE);
	AddSText(0, 9, TRUE, "Would you like to:", COL_GOLD, FALSE);
	AddSText(0, 12, TRUE, "Talk to Farnham", COL_BLUE, TRUE);
	AddSText(0, 18, TRUE, "Say Goodbye", COL_WHITE, TRUE);
	AddSLine(5);
	storenumh = 20;
}

void StartStore(char s)
{
	char t;
	int i;

	for (t = s;; t = STORE_SMITH) {
		sbookflag = FALSE;
		invflag = FALSE;
		chrflag = FALSE;
		questlog = FALSE;
		dropGoldFlag = FALSE;
		ClearSText(0, STORE_LINES);
		ReleaseStoreBtn();
		switch (t) {
		case STORE_SMITH:
			S_StartSmith();
			break;
		case STORE_SBUY:
			if (storenumh > 0)
				S_StartSBuy();
			break;
		case STORE_SSELL:
			S_StartSSell();
			break;
		case STORE_SREPAIR:
			S_StartSRepair();
			break;
		case STORE_WITCH:
			S_StartWitch();
			break;
		case STORE_WBUY:
			if (storenumh > 0)
				S_StartWBuy();
			break;
		case STORE_WSELL:
			S_StartWSell();
			break;
		case STORE_WRECHARGE:
			S_StartWRecharge();
			break;
		case STORE_NOMONEY:
			S_StartNoMoney();
			break;
		case STORE_NOROOM:
			S_StartNoRoom();
			break;
		case STORE_CONFIRM:
			S_StartConfirm();
			break;
		case STORE_BOY:
			S_StartBoy();
			break;
		case STORE_BBOY:
			S_StartBBoy();
			break;
		case STORE_HEALER:
			S_StartHealer();
			break;
		case STORE_STORY:
			S_StartStory();
			break;
		case STORE_HBUY:
			if (storenumh > 0)
				S_StartHBuy();
			break;
		case STORE_SIDENTIFY:
			S_StartSIdentify();
			break;
		case STORE_SPBUY:
			if (!S_StartSPBuy())
				return;
			break;
		case STORE_GOSSIP:
			S_StartTalk();
			break;
		case STORE_IDSHOW:
			S_StartIdShow();
			break;
		case STORE_TAVERN:
			S_StartTavern();
			break;
		case STORE_DRUNK:
			S_StartDrunk();
			break;
		case STORE_BARMAID:
			S_StartBarMaid();
			break;
		}

		for (i = 0; i < STORE_LINES; i++) {
			if (stext[i]._ssel)
				break;
		}

		stextsel = i == STORE_LINES ? -1 : i;
		stextflag = t;
		if (t != STORE_SBUY || storenumh != 0)
			break;
	}
}

void DrawSText()
{
	int i;

	if (!stextsize)
		DrawSTextBack();
	else
		DrawQTextBack();

	if (stextscrl) {
		switch (stextflag) {
		case STORE_SBUY:
			S_ScrollSBuy();
			break;
		case STORE_SSELL:
		case STORE_SREPAIR:
		case STORE_WSELL:
		case STORE_WRECHARGE:
		case STORE_SIDENTIFY:
			S_ScrollSSell();
			break;
		case STORE_WBUY:
			S_ScrollWBuy();
			break;
		case STORE_HBUY:
			S_ScrollHBuy();
			break;
		case STORE_SPBUY:
			S_ScrollSPBuy();
			break;
		}
	}

	for (i = 0; i < STORE_LINES; i++) {
		if (stext[i]._sline)
			DrawSLine(i);
		if (stext[i]._sstr[0] != '\0')
			PrintSString(stext[i]._sx, i, stext[i]._sjust, stext[i]._sstr, stext[i]._sclr, stext[i]._sval);
	}

	if (stextscrl)
		DrawSSlider(4, 20);
}

void STextESC()
{
	if (qtextflag) {
		qtextflag = FALSE;
		if (leveltype == DTYPE_TOWN)
			stream_stop();
	} else {
		switch (stextflag) {
		case STORE_SMITH:
		case STORE_WITCH:
		case STORE_BOY:
		case STORE_BBOY:
		case STORE_HEALER:
		case STORE_STORY:
		case STORE_TAVERN:
		case STORE_DRUNK:
		case STORE_BARMAID:
			stextflag = STORE_NONE;
			break;
		case STORE_GOSSIP:
			StartStore(stextshold);
			stextsel = stextlhold;
			break;
		case STORE_SBUY:
			StartStore(STORE_SMITH);
			stextsel = 12;
			break;
		case STORE_SPBUY:
			StartStore(STORE_SMITH);
			stextsel = 14;
			break;
		case STORE_SSELL:
			StartStore(STORE_SMITH);
			stextsel = 16;
			break;
		case STORE_SREPAIR:
			StartStore(STORE_SMITH);
			stextsel = 18;
			break;
		case STORE_WBUY:
			StartStore(STORE_WITCH);
			stextsel = 14;
			break;
		case STORE_WSELL:
			StartStore(STORE_WITCH);
			stextsel = 16;
			break;
		case STORE_WRECHARGE:
			StartStore(STORE_WITCH);
			stextsel = 18;
			break;
		case STORE_HBUY:
			StartStore(STORE_HEALER);
			stextsel = 16;
			break;
		case STORE_SIDENTIFY:
			StartStore(STORE_STORY);
			stextsel = 14;
			break;
		case STORE_IDSHOW:
			StartStore(STORE_SIDENTIFY);
			break;
		case STORE_NOMONEY:
		case STORE_NOROOM:
		case STORE_CONFIRM:
			StartStore(stextshold);
			stextsel = stextlhold;
			stextsidx = stextvhold;
			break;
		}
	}
}

void STextUp()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel == -1) {
		return;
	}

	if (stextscrl && stextsel == stextup) {
		if (stextsidx != 0)
			stextsidx--;
		return;
	}

	do {
		stextsel--;
		if (stextsel < 0)
			stextsel = STORE_LINES - 1;
	} while (!stext[stextsel]._ssel);
}

void STextDown()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel == -1) {
		return;
	}

	if (stextscrl && stextsel == stextdown) {
		if (stextsidx < stextsmax)
			stextsidx++;
		return;
	}

	do {
		stextsel++;
		if (stextsel == STORE_LINES)
			stextsel = 0;
	} while (!stext[stextsel]._ssel);
}

void STextPrior()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel != -1 && stextscrl) {
		if (stextsel == stextup) {
			stextsidx -= 4;
			if (stextsidx < 0)
				stextsidx = 0;
		} else {
			stextsel = stextup;
		}
	}
}

void STextNext()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel != -1 && stextscrl) {
		if (stextsel == stextdown) {
			stextsidx += 4;
			if (stextsidx > stextsmax)
				stextsidx = stextsmax;
		} else {
			stextsel = stextdown;
		}
	}
}

static void S_SmithEnter()
{
	switch (stextsel) {
	case 10:
		talker = TOWN_SMITH;
		stextlhold = 10;
		stextshold = STORE_SMITH;
		gossipstart = TEXT_GRISWOLD2;
		gossipend = TEXT_GRISWOLD13;
		StartStore(STORE_GOSSIP);
		break;
	case 12:
		StartStore(STORE_SBUY);
		break;
	case 14:
		StartStore(STORE_SPBUY);
		break;
	case 16:
		StartStore(STORE_SSELL);
		break;
	case 18:
		StartStore(STORE_SREPAIR);
		break;
	case 20:
		stextflag = STORE_NONE;
		break;
	}
}

void TakePlrsMoney(int cost)
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	p = &plr[myplr];
	p->_pGold = CalculateGold(myplr) - cost;
	for (i = 0; i < MAXBELTITEMS && cost > 0; i++) {
		pi = &p->SpdList[i];
		if (pi->_itype == ITYPE_GOLD && pi->_ivalue != GOLD_MAX_LIMIT) {
			if (cost < pi->_ivalue) {
				SetGoldItemValue(pi, pi->_ivalue - cost);
				cost = 0;
			} else {
				cost -= pi->_ivalue;
				RemoveSpdBarItem(myplr, i);
				i = -1;
			}
		}
	}
	if (cost > 0) {
		for (i = 0; i < MAXBELTITEMS && cost > 0; i++) {
			pi = &p->SpdList[i];
			if (pi->_itype == ITYPE_GOLD) {
				if (cost < pi->_ivalue) {
					SetGoldItemValue(pi, pi->_ivalue - cost);
					cost = 0;
				} else {
					cost -= pi->_ivalue;
					RemoveSpdBarItem(myplr, i);
					i = -1;
				}
			}
		}
	}
	force_redraw = 255;
	if (cost > 0) {
		for (i = 0; i < p->_pNumInv && cost > 0; i++) {
			pi = &p->InvList[i];
			if (pi->_itype == ITYPE_GOLD && pi->_ivalue != GOLD_MAX_LIMIT) {
				if (cost < pi->_ivalue) {
					SetGoldItemValue(pi, pi->_ivalue - cost);
					cost = 0;
				} else {
					cost -= pi->_ivalue;
					RemoveInvItem(myplr, i);
					i = -1;
				}
			}
		}
		if (cost > 0) {
			for (i = 0; i < p->_pNumInv && cost > 0; i++) {
				pi = &p->InvList[i];
				if (pi->_itype == ITYPE_GOLD) {
					if (cost < pi->_ivalue) {
						SetGoldItemValue(pi, pi->_ivalue - cost);
						cost = 0;
					} else {
						cost -= pi->_ivalue;
						RemoveInvItem(myplr, i);
						i = -1;
					}
				}
			}
		}
	}
}

static BOOL StoreAutoPlace(BOOL saveflag)
{
	return WeaponAutoPlace(myplr, &plr[myplr].HoldItem, saveflag)
		|| AutoPlaceInv(myplr, &plr[myplr].HoldItem, saveflag);
}

static void SmithBuyItem()
{
	int idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_NORMAL)
		plr[myplr].HoldItem._iIdentified = FALSE;
	StoreAutoPlace(TRUE);
	idx = stextvhold + ((stextlhold - stextup) >> 2);
	do {
		copy_pod(smithitem[idx], smithitem[idx + 1]);
		idx++;
	} while (smithitem[idx]._itype != ITYPE_NONE);
}

static void S_SBuyEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 12;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsidx;
		stextshold = STORE_SBUY;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		if (plr[myplr]._pGold < smithitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			copy_pod(plr[myplr].HoldItem, smithitem[idx]);
			if (StoreAutoPlace(FALSE))
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
		}
	}
}

static void SmithBuyPItem()
{
	int i, xx, idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_NORMAL)
		plr[myplr].HoldItem._iIdentified = FALSE;
	StoreAutoPlace(TRUE);

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	xx = 0;
	for (i = 0; idx >= 0; i++) {
		if (premiumitem[i]._itype != ITYPE_NONE) {
			idx--;
			xx = i;
		}
	}

	premiumitem[xx]._itype = ITYPE_NONE;
	numpremium--;
	SpawnPremium(plr[myplr]._pLevel);
}

static void S_SPBuyEnter()
{
	int i, idx, xx;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 14;
	} else {
		stextshold = STORE_SPBUY;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		xx = stextsidx + ((stextsel - stextup) >> 2);
		idx = 0;
		for (i = 0; xx >= 0; i++) {
			if (premiumitem[i]._itype != ITYPE_NONE) {
				xx--;
				idx = i;
			}
		}
		if (plr[myplr]._pGold < premiumitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			copy_pod(plr[myplr].HoldItem, premiumitem[idx]);
			if (StoreAutoPlace(FALSE))
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
		}
	}
}

static BOOL StoreGoldFit(int idx)
{
	ItemStruct *pi;
	int i, cost, numsqrs;

	cost = storehold[idx]._iIvalue;

	numsqrs = 0;
	// add the item slots but only if it is not in the belt, since gold can not be placed there
	if (storehidx[idx] >= 0) {
		i = storehold[idx]._iCurs + CURSOR_FIRSTITEM;
		numsqrs = InvItemHeight[i] * InvItemWidth[i] / (INV_SLOT_SIZE_PX * INV_SLOT_SIZE_PX);
	}
	// add the empty slots
	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (plr[myplr].InvGrid[i] == 0)
			numsqrs++;
	}

	cost -= numsqrs * GOLD_MAX_LIMIT;
	if (cost <= 0)
		return TRUE;

	// check for not full piles of gold if there is still not enough place
	pi = plr[myplr].InvList;
	for (i = plr[myplr]._pNumInv; i > 0; i--, pi++) {
		if (pi->_itype == ITYPE_GOLD) {
			cost -= GOLD_MAX_LIMIT - pi->_ivalue;
			if (cost <= 0)
				return TRUE;
		}
	}
	return FALSE;
}

static void PlaceStoreGold(int v)
{
	PlayerStruct *p;
	int ii, i;

	p = &plr[myplr];
	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (p->InvGrid[i] == 0) {
			ii = p->_pNumInv;
			GetGoldSeed(myplr, &golditem);
			copy_pod(p->InvList[ii], golditem);
			SetGoldItemValue(&p->InvList[ii], v);
			p->_pNumInv++;
			p->InvGrid[i] = p->_pNumInv;
			break;
		}
	}
}

static void StoreSellItem()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i, idx, cost, val;

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	if (storehidx[idx] >= 0)
		RemoveInvItem(myplr, storehidx[idx]);
	else
		RemoveSpdBarItem(myplr, -(storehidx[idx] + 1));
	cost = storehold[idx]._iIvalue;
	storenumh--;
	while (idx < storenumh) {
		copy_pod(storehold[idx], storehold[idx + 1]);
		storehidx[idx] = storehidx[idx + 1];
		idx++;
	}
	p = &plr[myplr];
	p->_pGold += cost;
	pi = p->InvList;
	for (i = p->_pNumInv; i > 0 && cost > 0; i--, pi++) {
		if (pi->_itype != ITYPE_GOLD)
			continue;
		val = GOLD_MAX_LIMIT - pi->_ivalue;
		if (val > 0) {
			if (cost < val) {
				SetGoldItemValue(pi, pi->_ivalue + cost);
				cost = 0;
			} else {
				cost -= val;
				SetGoldItemValue(pi, GOLD_MAX_LIMIT);
			}
		}
	}
	if (cost > 0) {
		while (cost > GOLD_MAX_LIMIT) {
			PlaceStoreGold(GOLD_MAX_LIMIT);
			cost -= GOLD_MAX_LIMIT;
		}
		PlaceStoreGold(cost);
	}
}

static void S_SSellEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 16;
	} else {
		stextlhold = stextsel;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		stextshold = STORE_SSELL;
		stextvhold = stextsidx;
		copy_pod(plr[myplr].HoldItem, storehold[idx]);

		if (StoreGoldFit(idx))
			StartStore(STORE_CONFIRM);
		else
			StartStore(STORE_NOROOM);
	}
}

static void SmithRepairItem()
{
	ItemStruct *pi;
	int i, idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	storehold[idx]._iDurability = storehold[idx]._iMaxDur;

	i = storehidx[idx];
	if (i < 0) {
		pi = &plr[myplr].InvBody[-(i + 1)];
	} else {
		pi = &plr[myplr].InvList[i];
	}
	pi->_iDurability = pi->_iMaxDur;
}

static void S_SRepairEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 18;
	} else {
		stextshold = STORE_SREPAIR;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		copy_pod(plr[myplr].HoldItem, storehold[idx]);
		if (plr[myplr]._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_WitchEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = TOWN_WITCH;
		stextshold = STORE_WITCH;
		gossipstart = TEXT_ADRIA2;
		gossipend = TEXT_ADRIA13;
		StartStore(STORE_GOSSIP);
		return;
	case 14:
		StartStore(STORE_WBUY);
		return;
	case 16:
		StartStore(STORE_WSELL);
		return;
	case 18:
		StartStore(STORE_WRECHARGE);
		return;
	case 20:
		stextflag = STORE_NONE;
		break;
	}
}

static void WitchBuyItem()
{
	int idx;

	idx = stextvhold + ((stextlhold - stextup) >> 2);

	if (idx < 3)
		plr[myplr].HoldItem._iSeed = GetRndSeed();

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	StoreAutoPlace(TRUE);

	if (idx >= 3) {
		do {
			copy_pod(witchitem[idx], witchitem[idx + 1]);
			idx++;
		} while (witchitem[idx]._itype != ITYPE_NONE);
	}
}

static void S_WBuyEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_WITCH);
		stextsel = 14;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsidx;
		stextshold = STORE_WBUY;
		idx = stextsidx + ((stextsel - stextup) >> 2);

		if (plr[myplr]._pGold < witchitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			copy_pod(plr[myplr].HoldItem, witchitem[idx]);
			if (StoreAutoPlace(FALSE))
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
		}
	}
}

static void S_WSellEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_WITCH);
		stextsel = 16;
	} else {
		stextlhold = stextsel;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		stextshold = STORE_WSELL;
		stextvhold = stextsidx;
		copy_pod(plr[myplr].HoldItem, storehold[idx]);
		if (StoreGoldFit(idx))
			StartStore(STORE_CONFIRM);
		else
			StartStore(STORE_NOROOM);
	}
}

static void WitchRechargeItem()
{
	ItemStruct *pi;
	int i, idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	storehold[idx]._iCharges = storehold[idx]._iMaxCharges;

	i = storehidx[idx];
	if (i < 0)
		pi = &plr[myplr].InvBody[-(i + 1)];
	else
		pi = &plr[myplr].InvList[i];
	pi->_iCharges = pi->_iMaxCharges;

	CalcPlrInv(myplr, TRUE);
}

static void S_WRechargeEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_WITCH);
		stextsel = 18;
	} else {
		stextshold = STORE_WRECHARGE;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		copy_pod(plr[myplr].HoldItem, storehold[idx]);
		if (plr[myplr]._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_BoyEnter()
{
	if (boyitem._itype != ITYPE_NONE && stextsel == 18) {
		if (plr[myplr]._pGold < 50) {
			stextshold = STORE_BOY;
			stextlhold = 18;
			stextvhold = stextsidx;
			StartStore(STORE_NOMONEY);
		} else {
			TakePlrsMoney(50);
			StartStore(STORE_BBOY);
		}
	} else if (stextsel == 8 && boyitem._itype != ITYPE_NONE || stextsel == 12 && boyitem._itype == ITYPE_NONE) {
		talker = TOWN_PEGBOY;
		stextshold = STORE_BOY;
		stextlhold = stextsel;
		gossipstart = TEXT_WIRT2;
		gossipend = TEXT_WIRT12;
		StartStore(STORE_GOSSIP);
	} else {
		stextflag = STORE_NONE;
	}
}

static void BoyBuyItem()
{
	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	// restore the price of the item
	plr[myplr].HoldItem._iIvalue = boyitem._iIvalue;
	StoreAutoPlace(TRUE);
	boyitem._itype = ITYPE_NONE;
	stextshold = STORE_BOY;
}

static void HealerBuyItem()
{
	int idx;
	BOOL infinite;

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	infinite = idx < (gbMaxPlayers == 1 ? 2 : 3);
	if (infinite)
		plr[myplr].HoldItem._iSeed = GetRndSeed();

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_NORMAL)
		plr[myplr].HoldItem._iIdentified = FALSE;
	StoreAutoPlace(TRUE);

	if (infinite)
		return;

	do {
		copy_pod(healitem[idx], healitem[idx + 1]);
		idx++;
	} while (healitem[idx]._itype != ITYPE_NONE);
}

static void S_BBuyEnter()
{
	int sellValue;

	if (stextsel == 10) {
		stextshold = STORE_BBOY;
		stextvhold = stextsidx;
		stextlhold = 10;
#ifdef HELLFIRE
		sellValue = boyitem._iIvalue - (boyitem._iIvalue >> 2);
#else
		sellValue = boyitem._iIvalue + (boyitem._iIvalue >> 1);
#endif
		if (plr[myplr]._pGold < sellValue) {
			StartStore(STORE_NOMONEY);
		} else {
			copy_pod(plr[myplr].HoldItem, boyitem);
			plr[myplr].HoldItem._iIvalue = sellValue;
			if (StoreAutoPlace(FALSE))
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
		}
	} else {
		stextflag = STORE_NONE;
	}
}

static void StoryIdItem()
{
	int idx;

	idx = storehidx[((stextlhold - stextup) >> 2) + stextvhold];
	if (idx < 0) {
		plr[myplr].InvBody[-(idx + 1)]._iIdentified = TRUE;
	} else {
		plr[myplr].InvList[idx]._iIdentified = TRUE;
	}
	plr[myplr].HoldItem._iIdentified = TRUE;
	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	CalcPlrInv(myplr, TRUE);
}

static void S_ConfirmEnter()
{
	if (stextsel == 18) {
		switch (stextshold) {
		case STORE_SBUY:
			SmithBuyItem();
			break;
		case STORE_SSELL:
		case STORE_WSELL:
			StoreSellItem();
			break;
		case STORE_SREPAIR:
			SmithRepairItem();
			break;
		case STORE_WBUY:
			WitchBuyItem();
			break;
		case STORE_WRECHARGE:
			WitchRechargeItem();
			break;
		case STORE_BBOY:
			BoyBuyItem();
			break;
		case STORE_HBUY:
			HealerBuyItem();
			break;
		case STORE_SIDENTIFY:
			StoryIdItem();
			StartStore(STORE_IDSHOW);
			return;
		case STORE_SPBUY:
			SmithBuyPItem();
			break;
		}
		StartStore(stextshold);
	} else {
		StartStore(stextshold);
		stextsel = stextlhold;
		stextsidx = stextvhold;
	}
}

static void S_HealerEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = TOWN_HEALER;
		stextshold = STORE_HEALER;
		gossipstart = TEXT_PEPIN2;
		gossipend = TEXT_PEPIN11;
		StartStore(STORE_GOSSIP);
		break;
#ifdef HELLFIRE
	case 14:
		StartStore(STORE_HBUY);
		break;
	case 16:
		stextflag = STORE_NONE;
		break;
#else
	case 14:
		if (plr[myplr]._pHitPoints != plr[myplr]._pMaxHP)
			PlaySFX(IS_CAST8);
		drawhpflag = TRUE;
		plr[myplr]._pHitPoints = plr[myplr]._pMaxHP;
		plr[myplr]._pHPBase = plr[myplr]._pMaxHPBase;
		break;
	case 16:
		StartStore(STORE_HBUY);
		break;
	case 18:
		stextflag = STORE_NONE;
		break;
#endif
	}
}

static void S_HBuyEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_HEALER);
		stextsel = 16;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsidx;
		stextshold = STORE_HBUY;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		if (plr[myplr]._pGold < healitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			copy_pod(plr[myplr].HoldItem, healitem[idx]);
			if (StoreAutoPlace(FALSE))
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
		}
	}
}

static void S_StoryEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = TOWN_STORY;
		stextshold = STORE_STORY;
		gossipstart = TEXT_STORY2;
		gossipend = TEXT_STORY11;
		StartStore(STORE_GOSSIP);
		break;
	case 14:
		StartStore(STORE_SIDENTIFY);
		break;
	case 18:
		stextflag = STORE_NONE;
		break;
	}
}

static void S_SIDEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_STORY);
		stextsel = 14;
	} else {
		stextshold = STORE_SIDENTIFY;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		idx = stextsidx + ((stextsel - stextup) >> 2);
		copy_pod(plr[myplr].HoldItem, storehold[idx]);
		if (plr[myplr]._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_TalkEnter()
{
	int i, tq, sn, la;

	if (stextsel == 22) {
		StartStore(stextshold);
		stextsel = stextlhold;
		return;
	}

	sn = 0;
	for (i = 0; i < MAXQUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && ((DWORD *)&Qtalklist[talker])[i] != -1 && quests[i]._qlog)
			sn++;
	}
	if (sn > 6) {
		sn = 14 - (sn >> 1);
		la = 1;
	} else {
		sn = 15 - sn;
		la = 2;
	}

	if (stextsel == sn - 2) {
		SetRndSeed(towner[talker]._tSeed);
		tq = RandRange(gossipstart, gossipend);
		InitQTextMsg(tq);
		return;
	}

	for (i = 0; i < MAXQUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && ((DWORD *)&Qtalklist[talker])[i] != -1 && quests[i]._qlog) {
			if (sn == stextsel) {
				InitQTextMsg(((DWORD *)&Qtalklist[talker])[i]);
			}
			sn += la;
		}
	}
}

static void S_TavernEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = TOWN_TAVERN;
		stextshold = STORE_TAVERN;
		gossipstart = TEXT_OGDEN2;
		gossipend = TEXT_OGDEN10;
		StartStore(STORE_GOSSIP);
		break;
	case 18:
		stextflag = STORE_NONE;
		break;
	}
}

static void S_BarmaidEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = TOWN_BMAID;
		stextshold = STORE_BARMAID;
		gossipstart = TEXT_GILLIAN2;
		gossipend = TEXT_GILLIAN10;
		StartStore(STORE_GOSSIP);
		break;
	case 18:
		stextflag = STORE_NONE;
		break;
	}
}

static void S_DrunkEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = TOWN_DRUNK;
		stextshold = STORE_DRUNK;
		gossipstart = TEXT_FARNHAM2;
		gossipend = TEXT_FARNHAM13;
		StartStore(STORE_GOSSIP);
		break;
	case 18:
		stextflag = STORE_NONE;
		break;
	}
}

void STextEnter()
{
	if (qtextflag) {
		qtextflag = FALSE;
		if (leveltype == DTYPE_TOWN)
			stream_stop();
	} else {
		PlaySFX(IS_TITLSLCT);
		switch (stextflag) {
		case STORE_SMITH:
			S_SmithEnter();
			break;
		case STORE_SPBUY:
			S_SPBuyEnter();
			break;
		case STORE_SBUY:
			S_SBuyEnter();
			break;
		case STORE_SSELL:
			S_SSellEnter();
			break;
		case STORE_SREPAIR:
			S_SRepairEnter();
			break;
		case STORE_WITCH:
			S_WitchEnter();
			break;
		case STORE_WBUY:
			S_WBuyEnter();
			break;
		case STORE_WSELL:
			S_WSellEnter();
			break;
		case STORE_WRECHARGE:
			S_WRechargeEnter();
			break;
		case STORE_NOMONEY:
		case STORE_NOROOM:
			StartStore(stextshold);
			stextsel = stextlhold;
			stextsidx = stextvhold;
			break;
		case STORE_CONFIRM:
			S_ConfirmEnter();
			break;
		case STORE_BOY:
			S_BoyEnter();
			break;
		case STORE_BBOY:
			S_BBuyEnter();
			break;
		case STORE_HEALER:
			S_HealerEnter();
			break;
		case STORE_STORY:
			S_StoryEnter();
			break;
		case STORE_HBUY:
			S_HBuyEnter();
			break;
		case STORE_SIDENTIFY:
			S_SIDEnter();
			break;
		case STORE_GOSSIP:
			S_TalkEnter();
			break;
		case STORE_IDSHOW:
			StartStore(STORE_SIDENTIFY);
			break;
		case STORE_DRUNK:
			S_DrunkEnter();
			break;
		case STORE_TAVERN:
			S_TavernEnter();
			break;
		case STORE_BARMAID:
			S_BarmaidEnter();
			break;
		}
	}
}

void CheckStoreBtn()
{
	int y;

	if (qtextflag) {
		qtextflag = FALSE;
		if (leveltype == DTYPE_TOWN)
			stream_stop();
	} else if (stextsel != -1 && MouseY >= (32 + UI_OFFSET_Y) && MouseY <= (320 + UI_OFFSET_Y)) {
		if (!stextsize) {
			if (MouseX < 344 + PANEL_LEFT || MouseX > 616 + PANEL_LEFT)
				return;
		} else {
			if (MouseX < 24 + PANEL_LEFT || MouseX > 616 + PANEL_LEFT)
				return;
		}
		y = (MouseY - (32 + UI_OFFSET_Y)) / 12;
		if (stextscrl && MouseX > 600 + PANEL_LEFT) {
			if (y == 4) {
				if (stextscrlubtn <= 0) {
					STextUp();
					stextscrlubtn = 10;
				} else {
					stextscrlubtn--;
				}
			}
			if (y == 20) {
				if (stextscrldbtn <= 0) {
					STextDown();
					stextscrldbtn = 10;
				} else {
					stextscrldbtn--;
				}
			}
		} else if (y >= 5) {
			if (y >= 23)
				y = 22;
			if (stextscrl && y < 21 && !stext[y]._ssel) {
				if (stext[y - 2]._ssel) {
					y -= 2;
				} else if (stext[y - 1]._ssel) {
					y--;
				}
			}
			if (stext[y]._ssel || stextscrl && y == 22) {
				stextsel = y;
				STextEnter();
			}
		}
	}
}

void ReleaseStoreBtn()
{
	stextscrlubtn = -1;
	stextscrldbtn = -1;
}

DEVILUTION_END_NAMESPACE
