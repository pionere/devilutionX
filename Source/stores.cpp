/**
 * @file stores.cpp
 *
 * Implementation of functionality for stores and towner dialogs.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

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
char stextsize;
int stextsmax;
ItemStruct storehold[48];
int gossipstart;
#ifdef HELLFIRE
ItemStruct witchitem[25];
#else
ItemStruct witchitem[20];
#endif
BOOL stextscrl;
int numpremium;
ItemStruct healitem[20];
ItemStruct golditem;
char storehidx[48];
BYTE *pSTextSlidCels;
int stextvhold;
int stextsel;
char stextscrldbtn;
int gossipend;
BYTE *pSPentSpn2Cels;
BYTE PentSpn2Frame;
DWORD PentSpn2Tick;
int stextsval;
int boylevel;
ItemStruct smithitem[SMITH_ITEMS];
int stextdown;
char stextscrlubtn;
char stextflag;

/** Maps from towner IDs to NPC names. */
char *talkname[9] = {
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
	PentSpn2Frame = 1;
	stextsize = FALSE;
	stextscrl = FALSE;
	numpremium = 0;
	premiumlevel = 1;

	for (i = 0; i < 6; i++)
		premiumitem[i]._itype = ITYPE_NONE;

	boyitem._itype = ITYPE_NONE;
	boylevel = 0;
}

void PentSpn2Spin()
{
	DWORD ticks = SDL_GetTicks();
	if (ticks - PentSpn2Tick > 50) {
		PentSpn2Frame = (PentSpn2Frame & 7) + 1;
		PentSpn2Tick = ticks;
	}
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
#ifdef HELLFIRE
	SpawnPremium(myplr);
#else
	SpawnPremium(plr[myplr]._pLevel);
#endif
}

void FreeStoreMem()
{
	MemFreeDbg(pSTextBoxCels);
	MemFreeDbg(pSPentSpn2Cels);
	MemFreeDbg(pSTextSlidCels);
}

void DrawSTextBack()
{
	CelDraw(PANEL_X + 344, 327 + SCREEN_Y + UI_OFFSET_Y, pSTextBoxCels, 1, 271);
	trans_rect(PANEL_LEFT + 347, UI_OFFSET_Y + 28, 265, 297);
}

void PrintSString(int x, int y, BOOL cjustflag, char *str, char col, int val)
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
		CelDraw(sx - 20, sy + 1, pSPentSpn2Cels, PentSpn2Frame, 12);
	}
	for (i = 0; i < len; i++) {
		c = fontframe[gbFontTransTbl[(BYTE)str[i]]];
		k += fontkern[c] + 1;
		if (c && k <= yy) {
			PrintChar(sx, sy, c, col);
		}
		sx += fontkern[c] + 1;
	}
	if (!cjustflag && val >= 0) {
		sprintf(valstr, "%i", val);
		sx = PANEL_X + 592 - x;
		for (i = strlen(valstr) - 1; i >= 0; i--) {
			c = fontframe[gbFontTransTbl[(BYTE)valstr[i]]];
			sx -= fontkern[c] + 1;
			if (c) {
				PrintChar(sx, sy, c, col);
			}
		}
	}
	if (stextsel == y) {
		CelDraw(cjustflag ? (xx + x + k + 4) : (PANEL_X + 596 - x), sy + 1, pSPentSpn2Cels, PentSpn2Frame, 12);
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

void DrawSSlider(int y1, int y2)
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
		yd3 = 1000 * (stextsval + ((yd3 - stextup) >> 2)) / (storenumh - 1) * (y2 * 12 - y1 * 12 - 24) / 1000;
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
		stext[i]._sline = 0;
		stext[i]._ssel = FALSE;
		stext[i]._sval = -1;
	}
}

void AddSLine(int y)
{
	stext[y]._sx = 0;
	stext[y]._syoff = 0;
	stext[y]._sstr[0] = 0;
	stext[y]._sline = 1;
}

void AddSTextVal(int y, int val)
{
	stext[y]._sval = val;
}

void OffsetSTextY(int y, int yo)
{
	stext[y]._syoff = yo;
}

void AddSText(int x, int y, BOOL j, char *str, char clr, BOOL sel)
{
	stext[y]._sx = x;
	stext[y]._syoff = 0;
	strcpy(stext[y]._sstr, str);
	stext[y]._sjust = j;
	stext[y]._sclr = clr;
	stext[y]._sline = 0;
	stext[y]._ssel = sel;
}

void StoreAutoPlace()
{
	BOOL done;
	int i, w, h, idx;

	SetICursor(plr[myplr].HoldItem._iCurs + CURSOR_FIRSTITEM);
	w = icursW28;
	h = icursH28;
	done = FALSE;
	if (w == 1 && h == 1) {
		idx = plr[myplr].HoldItem.IDidx;
		if (plr[myplr].HoldItem._iStatFlag && AllItemsList[idx].iUsable) {
			for (i = 0; i < MAXBELTITEMS && !done; i++) {
				if (plr[myplr].SpdList[i]._itype == ITYPE_NONE) {
					plr[myplr].SpdList[i] = plr[myplr].HoldItem;
					done = TRUE;
				}
			}
		}
		for (i = 30; i <= 39 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 20; i <= 29 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 10; i <= 19 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 0; i <= 9 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
	}
	if (w == 1 && h == 2) {
		for (i = 29; i >= 20 && !done; i--) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 9; i >= 0 && !done; i--) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 19; i >= 10 && !done; i--) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
	}
	if (w == 1 && h == 3) {
		for (i = 0; i < 20 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
	}
	if (w == 2 && h == 2) {
		for (i = 0; i < 10 && !done; i++) {
			done = AutoPlace(myplr, AP2x2Tbl[i], w, h, TRUE);
		}
		for (i = 21; i < 29 && !done; i += 2) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 1; i < 9 && !done; i += 2) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 10; i < 19 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
	}
	if (w == 2 && h == 3) {
		for (i = 0; i < 9 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
		for (i = 10; i < 19 && !done; i++) {
			done = AutoPlace(myplr, i, w, h, TRUE);
		}
	}
}

void S_StartSmith()
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

void S_ScrollSBuy(int idx)
{
	int l, ls;
	char iclr;

	ls = idx;
	ClearSText(5, 21);
	stextup = 5;

	for (l = 5; l < 20; l += 4) {
		if (smithitem[ls]._itype != ITYPE_NONE) {
			iclr = COL_WHITE;
			if (smithitem[ls]._iMagical) {
				iclr = COL_BLUE;
			}

			if (!smithitem[ls]._iStatFlag) {
				iclr = COL_RED;
			}

			if (smithitem[ls]._iMagical) {
				AddSText(20, l, FALSE, smithitem[ls]._iIName, iclr, TRUE);
			} else {
				AddSText(20, l, FALSE, smithitem[ls]._iName, iclr, TRUE);
			}

			AddSTextVal(l, smithitem[ls]._iIvalue);
			PrintStoreItem(&smithitem[ls], l + 1, iclr);
			stextdown = l;
			ls++;
		}
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
}

void PrintStoreItem(ItemStruct *is, int l, char iclr)
{
	char sstr[128];

	sstr[0] = '\0';
	if (is->_iIdentified) {
		if (is->_iMagical != ITEM_QUALITY_UNIQUE) {
			if (is->_iPrePower != -1) {
				PrintItemPower(is->_iPrePower, is);
				strcat(sstr, tempstr);
			}
		}
		if (is->_iSufPower != -1) {
			PrintItemPower(is->_iSufPower, is);
			if (sstr[0])
				strcat(sstr, ",  ");
			strcat(sstr, tempstr);
		}
	}
	if (is->_iMiscId == IMISC_STAFF && is->_iMaxCharges) {
		sprintf(tempstr, "Charges: %i/%i", is->_iCharges, is->_iMaxCharges);
		if (sstr[0])
			strcat(sstr, ",  ");
		strcat(sstr, tempstr);
	}
	if (sstr[0]) {
		AddSText(40, l, FALSE, sstr, iclr, FALSE);
		l++;
	}
	sstr[0] = '\0';
	if (is->_iClass == ICLASS_WEAPON)
		sprintf(sstr, "Damage: %i-%i  ", is->_iMinDam, is->_iMaxDam);
	if (is->_iClass == ICLASS_ARMOR)
		sprintf(sstr, "Armor: %i  ", is->_iAC);
	if (is->_iMaxDur != DUR_INDESTRUCTIBLE && is->_iMaxDur) {
		sprintf(tempstr, "Dur: %i/%i,  ", is->_iDurability, is->_iMaxDur);
		strcat(sstr, tempstr);
	} else {
		strcat(sstr, "Indestructible,  ");
	}
	if (is->_itype == ITYPE_MISC)
		sstr[0] = '\0';
	if ((is->_iMinStr | is->_iMinMag | is->_iMinDex) == 0) {
		strcat(sstr, "No required attributes");
	} else {
		strcpy(tempstr, "Required:");
		if (is->_iMinStr)
			sprintf(tempstr, "%s %i Str", tempstr, is->_iMinStr);
		if (is->_iMinMag)
			sprintf(tempstr, "%s %i Mag", tempstr, is->_iMinMag);
		if (is->_iMinDex)
			sprintf(tempstr, "%s %i Dex", tempstr, is->_iMinDex);
		strcat(sstr, tempstr);
	}
	AddSText(40, l++, FALSE, sstr, iclr, FALSE);
	if (is->_iMagical == ITEM_QUALITY_UNIQUE) {
		if (is->_iIdentified)
			AddSText(40, l, FALSE, "Unique Item", iclr, FALSE);
	}
}

void S_StartSBuy()
{
	int i;

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsval = 0;
	sprintf(tempstr, "I have these items for sale :           Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	S_ScrollSBuy(stextsval);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, FALSE);
	OffsetSTextY(22, 6);
	storenumh = 0;
	for (i = 0; smithitem[i]._itype != ITYPE_NONE; i++) {
		storenumh++;
	}

	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

void S_ScrollSPBuy(int idx)
{
	ItemStruct* is;
	int l, boughtitems;
	char iclr;

	ClearSText(5, 21);
	boughtitems = idx;

	stextup = 5;
	for (idx = 0; boughtitems; idx++) {
		if (premiumitem[idx]._itype != ITYPE_NONE)
			boughtitems--;
	}

	for (l = 5; l < 20 && idx < SMITH_PREMIUM_ITEMS; l += 4) {
		is = &premiumitem[idx];
		if (is->_itype != ITYPE_NONE) {
			iclr = COL_WHITE;
			if (is->_iMagical)
				iclr = COL_BLUE;
			if (!is->_iStatFlag)
				iclr = COL_RED;
			AddSText(20, l, FALSE, is->_iIName, iclr, TRUE);
			AddSTextVal(l, is->_iIvalue);
			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
		} else {
			l -= 4;
		}
		idx++;
	}
	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
}

BOOL S_StartSPBuy()
{
	int i;

	storenumh = 0;
	for (i = 0; i < 6; i++) {
		if (premiumitem[i]._itype != ITYPE_NONE)
			storenumh++;
	}
	if (!storenumh) {
		StartStore(STORE_SMITH);
		stextsel = 14;
		return FALSE;
	}

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsval = 0;

	sprintf(tempstr, "I have these premium items for sale :   Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, FALSE);
	OffsetSTextY(22, 6);

	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;

	S_ScrollSPBuy(stextsval);

	return TRUE;
}

BOOL SmithSellOk(int i)
{
	if (plr[myplr].InvList[i]._itype == ITYPE_NONE)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_MISC)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_GOLD)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_MEAT)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_STAFF)
		return FALSE;
	if (plr[myplr].InvList[i].IDidx == IDI_LAZSTAFF)
		return FALSE;

	return TRUE;
}

void S_ScrollSSell(int idx)
{
	ItemStruct* is;
	int l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	for (l = 5; l < 20; l += 4) {
		if (idx >= storenumh)
			break;
		is = &storehold[idx];
		if (is->_itype != ITYPE_NONE) {
			iclr = COL_WHITE;
			if (is->_iMagical) {
				iclr = COL_BLUE;
			}

			if (!is->_iStatFlag) {
				iclr = COL_RED;
			}

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

	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

void S_StartSSell()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	stextsize = TRUE;
	storenumh = 0;

	for (i = 0; i < 48; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
#ifdef HELLFIRE
		if (storenumh >= 48)
			break;
#endif
		if (SmithSellOk(i)) {
			storehold[storenumh] = *pi;

			if (storehold[storenumh]._iMagical != ITEM_QUALITY_NORMAL && storehold[storenumh]._iIdentified)
				storehold[storenumh]._ivalue = storehold[storenumh]._iIvalue;

			if ((storehold[storenumh]._ivalue >>= 2) == 0)
				storehold[storenumh]._ivalue = 1;

			storehold[storenumh]._iIvalue = storehold[storenumh]._ivalue;
			storehidx[storenumh++] = i;
		}
	}
#ifdef HELLFIRE

	pi = p->SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++) {
		if (storenumh >= 48)
			break;
		if (SmithSellOk(-(i + 1))) {
			storehold[storenumh] = *pi;

			if (storehold[storenumh]._iMagical != ITEM_QUALITY_NORMAL && storehold[storenumh]._iIdentified)
				storehold[storenumh]._ivalue = storehold[storenumh]._iIvalue;

			if (!(storehold[storenumh]._ivalue >>= 2))
				storehold[storenumh]._ivalue = 1;

			storehold[storenumh]._iIvalue = storehold[storenumh]._ivalue;
			storehidx[storenumh++] = -(i + 1);
		}
	}
#endif

	if (storenumh == 0) {
		stextscrl = FALSE;
		sprintf(tempstr, "You have nothing I want.            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	} else {
		stextscrl = TRUE;
		stextsval = 0;
		stextsmax = plr[myplr]._pNumInv;
		sprintf(tempstr, "Which item is for sale?            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		S_ScrollSSell(stextsval);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	}
}

BOOL SmithRepairOk(int i)
{
	if (plr[myplr].InvList[i]._itype == ITYPE_NONE)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_MISC)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_GOLD)
		return FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_MEAT)
		return FALSE;
	if (plr[myplr].InvList[i]._iDurability == plr[myplr].InvList[i]._iMaxDur)
		return FALSE;

	return TRUE;
}

void S_StartSRepair()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	stextsize = TRUE;
	storenumh = 0;
	for (i = 0; i < 48; i++)
		storehold[i]._itype = ITYPE_NONE;
	p = &plr[myplr];
	pi = &p->InvBody[INVLOC_HEAD];
	if (pi->_itype != ITYPE_NONE && pi->_iDurability != pi->_iMaxDur) {
		AddStoreHoldRepair(p->InvBody, -1);
	}
	pi = &p->InvBody[INVLOC_CHEST];
	if (pi->_itype != ITYPE_NONE && pi->_iDurability != pi->_iMaxDur) {
		AddStoreHoldRepair(pi, -2);
	}
	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (pi->_itype != ITYPE_NONE && pi->_iDurability != pi->_iMaxDur) {
		AddStoreHoldRepair(pi, -3);
	}
	pi = &p->InvBody[INVLOC_HAND_RIGHT];
	if (pi->_itype != ITYPE_NONE && pi->_iDurability != pi->_iMaxDur) {
		AddStoreHoldRepair(pi, -4);
	}
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
#ifdef HELLFIRE
		if (storenumh >= 48)
			break;
#endif
		if (SmithRepairOk(i)) {
			AddStoreHoldRepair(pi, i);
		}
	}
	if (storenumh == 0) {
		stextscrl = FALSE;
		sprintf(tempstr, "You have nothing to repair.            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
		return;
	}

	stextscrl = TRUE;
	stextsval = 0;
	stextsmax = plr[myplr]._pNumInv;
	sprintf(tempstr, "Repair which item?            Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	S_ScrollSSell(stextsval);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
	OffsetSTextY(22, 6);
}

void AddStoreHoldRepair(ItemStruct *is, int i)
{
	ItemStruct *item;
	int v;

	item = &storehold[storenumh];
	storehold[storenumh] = *is;
	if (item->_iMagical != ITEM_QUALITY_NORMAL && item->_iIdentified)
		item->_ivalue = 30 * item->_iIvalue / 100;
	v = item->_ivalue * (100 * (item->_iMaxDur - item->_iDurability) / item->_iMaxDur) / 100;
	if (!v) {
		if (item->_iMagical != ITEM_QUALITY_NORMAL && item->_iIdentified)
			return;
		v = 1;
	}
	if (v > 1)
		v >>= 1;
	item->_iIvalue = v;
	item->_ivalue = v;
	storehidx[storenumh] = i;
	storenumh++;
}

void S_StartWitch()
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

void S_ScrollWBuy(int idx)
{
	ItemStruct *is;
	int l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;

	is = &witchitem[idx];
	for (l = 5; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			iclr = COL_WHITE;
			if (is->_iMagical) {
				iclr = COL_BLUE;
			}

			if (!is->_iStatFlag) {
				iclr = COL_RED;
			}

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
}

void S_StartWBuy()
{
	int i;

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsval = 0;
	stextsmax = 20;
	sprintf(tempstr, "I have these items for sale :           Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	S_ScrollWBuy(stextsval);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, FALSE);
	OffsetSTextY(22, 6);

	storenumh = 0;
	for (i = 0; witchitem[i]._itype != ITYPE_NONE; i++) {
		storenumh++;
	}
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

BOOL WitchSellOk(int i)
{
	BOOL rv;
	ItemStruct *pI;

	rv = FALSE;

	if (i >= 0)
		pI = &plr[myplr].InvList[i];
	else
		pI = &plr[myplr].SpdList[-(i + 1)];

	if (pI->_itype == ITYPE_MISC)
		rv = TRUE;
	if (pI->_itype == ITYPE_STAFF)
		rv = TRUE;
	if (pI->IDidx >= IDI_FIRSTQUEST && pI->IDidx <= IDI_LASTQUEST)
		rv = FALSE;
	if (pI->IDidx == IDI_LAZSTAFF)
		rv = FALSE;
	return rv;
}

void S_StartWSell()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	stextsize = TRUE;
	storenumh = 0;

	for (i = 0; i < 48; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &plr[myplr];
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
#ifdef HELLFIRE
		if (storenumh >= 48)
			break;
#endif
		if (WitchSellOk(i)) {
			storehold[storenumh] = *pi;

			if (storehold[storenumh]._iMagical != ITEM_QUALITY_NORMAL && storehold[storenumh]._iIdentified)
				storehold[storenumh]._ivalue = storehold[storenumh]._iIvalue;

			if ((storehold[storenumh]._ivalue >>= 2) == 0)
				storehold[storenumh]._ivalue = 1;

			storehold[storenumh]._iIvalue = storehold[storenumh]._ivalue;
			storehidx[storenumh++] = i;
		}
	}

	pi = p->SpdList;
	for (i = 0; i < MAXBELTITEMS; i++, pi++) {
#ifdef HELLFIRE
		if (storenumh >= 48)
			break;
#endif
		if (pi->_itype != ITYPE_NONE && WitchSellOk(-(i + 1))) {
			storehold[storenumh] = *pi;

			if (storehold[storenumh]._iMagical != ITEM_QUALITY_NORMAL && storehold[storenumh]._iIdentified)
				storehold[storenumh]._ivalue = storehold[storenumh]._iIvalue;

			if ((storehold[storenumh]._ivalue >>= 2) == 0)
				storehold[storenumh]._ivalue = 1;

			storehold[storenumh]._iIvalue = storehold[storenumh]._ivalue;
			storehidx[storenumh++] = -(i + 1);
		}
	}

	if (storenumh == 0) {
		stextscrl = FALSE;
		sprintf(tempstr, "You have nothing I want.            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	} else {
		stextscrl = TRUE;
		stextsval = 0;
		stextsmax = plr[myplr]._pNumInv;
		sprintf(tempstr, "Which item is for sale?            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		S_ScrollSSell(stextsval);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	}
}

BOOL WitchRechargeOk(int i)
{
	BOOL rv;

	rv = FALSE;
	if (plr[myplr].InvList[i]._itype == ITYPE_STAFF
	    && plr[myplr].InvList[i]._iCharges != plr[myplr].InvList[i]._iMaxCharges) {
		rv = TRUE;
	}
	return rv;
}

void AddStoreHoldRecharge(ItemStruct is, int i)
{
	storehold[storenumh] = is;
	storehold[storenumh]._ivalue += spelldata[is._iSpell].sStaffCost;
	storehold[storenumh]._ivalue = storehold[storenumh]._ivalue * (100 * (storehold[storenumh]._iMaxCharges - storehold[storenumh]._iCharges) / storehold[storenumh]._iMaxCharges) / 100 >> 1;
	storehold[storenumh]._iIvalue = storehold[storenumh]._ivalue;
	storehidx[storenumh] = i;
	storenumh++;
}

void S_StartWRecharge()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	stextsize = TRUE;
	storenumh = 0;

	for (i = 0; i < 48; i++) {
		storehold[i]._itype = ITYPE_NONE;
	}

	p = &plr[myplr];
	pi = &p->InvBody[INVLOC_HAND_LEFT];
#ifdef HELLFIRE
	if ((pi->_itype == ITYPE_STAFF || pi->_iMiscId == IMISC_UNIQUE)
#else
	if (pi->_itype == ITYPE_STAFF
#endif
	    && pi->_iCharges != pi->_iMaxCharges) {
		AddStoreHoldRecharge(*pi, -1);
	}

	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
#ifdef HELLFIRE
		if (storenumh >= 48)
			break;
#endif
		if (WitchRechargeOk(i)) {
			AddStoreHoldRecharge(*pi, i);
		}
	}

	if (storenumh == 0) {
		stextscrl = FALSE;
		sprintf(tempstr, "You have nothing to recharge.            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	} else {
		stextscrl = TRUE;
		stextsval = 0;
		stextsmax = plr[myplr]._pNumInv;
		sprintf(tempstr, "Recharge which item?            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		S_ScrollSSell(stextsval);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	}
}

void S_StartNoMoney()
{
	StartStore(stextshold);
	stextscrl = FALSE;
	stextsize = TRUE;
	ClearSText(5, 23);
	AddSText(0, 14, TRUE, "You do not have enough gold", COL_WHITE, TRUE);
}

void S_StartNoRoom()
{
	StartStore(stextshold);
	stextscrl = FALSE;
	ClearSText(5, 23);
	AddSText(0, 14, TRUE, "You do not have enough room in inventory", COL_WHITE, TRUE);
}

void S_StartConfirm()
{
	BOOL idprint;
	char iclr;

	StartStore(stextshold);
	stextscrl = FALSE;
	ClearSText(5, 23);
	iclr = COL_WHITE;

	if (plr[myplr].HoldItem._iMagical != ITEM_QUALITY_NORMAL)
		iclr = COL_BLUE;
	if (!plr[myplr].HoldItem._iStatFlag)
		iclr = COL_RED;

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
		strcpy(tempstr, "Do we have a deal?");
		break;
	case STORE_SIDENTIFY:
		strcpy(tempstr, "Are you sure you want to identify this item?");
		break;
	case STORE_HBUY:
	case STORE_SPBUY:
	case STORE_WBUY:
	case STORE_SBUY:
		strcpy(tempstr, "Are you sure you want to buy this item?");
		break;
	case STORE_WRECHARGE:
		strcpy(tempstr, "Are you sure you want to recharge this item?");
		break;
	case STORE_SSELL:
	case STORE_WSELL:
		strcpy(tempstr, "Are you sure you want to sell this item?");
		break;
	case STORE_SREPAIR:
		strcpy(tempstr, "Are you sure you want to repair this item?");
		break;
	}
	AddSText(0, 15, TRUE, tempstr, COL_WHITE, FALSE);
	AddSText(0, 18, TRUE, "Yes", COL_WHITE, TRUE);
	AddSText(0, 20, TRUE, "No", COL_WHITE, TRUE);
}

void S_StartBoy()
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

void S_StartBBoy()
{
	int iclr;

	stextsize = TRUE;
	stextscrl = FALSE;
	sprintf(tempstr, "I have this item for sale :           Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	iclr = COL_WHITE;

	if (boyitem._iMagical != ITEM_QUALITY_NORMAL)
		iclr = COL_BLUE;
	if (!boyitem._iStatFlag)
		iclr = COL_RED;
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

void S_StartHealer()
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

void S_ScrollHBuy(int idx)
{
	ItemStruct* is;
	int l;
	char iclr;

	ClearSText(5, 21);
	stextup = 5;
	is = &healitem[idx];
	for (l = 5; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			iclr = COL_WHITE;
			if (!is->_iStatFlag) {
				iclr = COL_RED;
			}

			AddSText(20, l, FALSE, is->_iName, iclr, TRUE);
			AddSTextVal(l, is->_iIvalue);
			PrintStoreItem(is, l + 1, iclr);
			stextdown = l;
			is++;
		}
	}

	if (!stext[stextsel]._ssel && stextsel != 22)
		stextsel = stextdown;
}

void S_StartHBuy()
{
	int i;

	stextsize = TRUE;
	stextscrl = TRUE;
	stextsval = 0;
	sprintf(tempstr, "I have these items for sale :           Your gold : %i", plr[myplr]._pGold);
	AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(3);
	AddSLine(21);
	S_ScrollHBuy(stextsval);
	AddSText(0, 22, TRUE, "Back", COL_WHITE, FALSE);
	OffsetSTextY(22, 6);

	storenumh = 0;
	for (i = 0; healitem[i]._itype != ITYPE_NONE; i++) {
		storenumh++;
	}
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

void S_StartStory()
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

BOOL IdItemOk(ItemStruct *is)
{
	if (is->_itype == ITYPE_NONE) {
		return FALSE;
	}
	if (is->_iMagical == ITEM_QUALITY_NORMAL) {
		return FALSE;
	}
	return !is->_iIdentified;
}

void AddStoreHoldId(ItemStruct is, int i)
{
	storehold[storenumh] = is;
	storehold[storenumh]._ivalue = 100;
	storehold[storenumh]._iIvalue = 100;
	storehidx[storenumh] = i;
	storenumh++;
}

void S_StartSIdentify()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i;

	stextsize = TRUE;
	storenumh = 0;

	for (i = 0; i < 48; i++)
		storehold[i]._itype = ITYPE_NONE;
	p = &plr[myplr];
	pi = &p->InvBody[INVLOC_HEAD];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -1);
	}
	pi = &p->InvBody[INVLOC_CHEST];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -2);
	}
	pi = &p->InvBody[INVLOC_HAND_LEFT];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -3);
	}
	pi = &p->InvBody[INVLOC_HAND_RIGHT];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -4);
	}
	pi = &p->InvBody[INVLOC_RING_LEFT];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -5);
	}
	pi = &p->InvBody[INVLOC_RING_RIGHT];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -6);
	}
	pi = &p->InvBody[INVLOC_AMULET];
	if (IdItemOk(pi)) {
		AddStoreHoldId(*pi, -7);
	}
	pi = p->InvList;
	for (i = 0; i < p->_pNumInv; i++, pi++) {
#ifdef HELLFIRE
		if (storenumh >= 48)
			break;
#endif
		if (IdItemOk(pi)) {
			AddStoreHoldId(*pi, i);
		}
	}

	if (storenumh == 0) {
		stextscrl = FALSE;
		sprintf(tempstr, "You have nothing to identify.            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	} else {
		stextscrl = TRUE;
		stextsval = 0;
		stextsmax = p->_pNumInv;
		sprintf(tempstr, "Identify which item?            Your gold : %i", plr[myplr]._pGold);
		AddSText(0, 1, TRUE, tempstr, COL_GOLD, FALSE);
		AddSLine(3);
		AddSLine(21);
		S_ScrollSSell(stextsval);
		AddSText(0, 22, TRUE, "Back", COL_WHITE, TRUE);
		OffsetSTextY(22, 6);
	}
}

void S_StartIdShow()
{
	char iclr;

	StartStore(stextshold);
	stextscrl = FALSE;
	ClearSText(5, 23);
	iclr = COL_WHITE;

	if (plr[myplr].HoldItem._iMagical != ITEM_QUALITY_NORMAL)
		iclr = COL_BLUE;
	if (!plr[myplr].HoldItem._iStatFlag)
		iclr = COL_RED;

	AddSText(0, 7, TRUE, "This item is:", COL_WHITE, FALSE);
	AddSText(20, 11, FALSE, plr[myplr].HoldItem._iIName, iclr, FALSE);
	PrintStoreItem(&plr[myplr].HoldItem, 12, iclr);
	AddSText(0, 18, TRUE, "Done", COL_WHITE, TRUE);
}

void S_StartTalk()
{
	int i, sn, sn2, la;

	stextsize = FALSE;
	stextscrl = FALSE;
	sprintf(tempstr, "Talk to %s", talkname[talker]);
	AddSText(0, 2, TRUE, tempstr, COL_GOLD, FALSE);
	AddSLine(5);
#ifdef SPAWN
	sprintf(tempstr, "Talking to %s", talkname[talker]);
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

void S_StartTavern()
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

void S_StartBarMaid()
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

void S_StartDrunk()
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
		if (t != STORE_SBUY || storenumh)
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
			S_ScrollSBuy(stextsval);
			break;
		case STORE_SSELL:
		case STORE_SREPAIR:
		case STORE_WSELL:
		case STORE_WRECHARGE:
		case STORE_SIDENTIFY:
			S_ScrollSSell(stextsval);
			break;
		case STORE_WBUY:
			S_ScrollWBuy(stextsval);
			break;
		case STORE_HBUY:
			S_ScrollHBuy(stextsval);
			break;
		case STORE_SPBUY:
			S_ScrollSPBuy(stextsval);
			break;
		}
	}

	for (i = 0; i < STORE_LINES; i++) {
		if (stext[i]._sline)
			DrawSLine(i);
		if (stext[i]._sstr[0])
			PrintSString(stext[i]._sx, i, stext[i]._sjust, stext[i]._sstr, stext[i]._sclr, stext[i]._sval);
	}

	if (stextscrl)
		DrawSSlider(4, 20);

	PentSpn2Spin();
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
			stextsval = stextvhold;
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

	if (stextscrl) {
		if (stextsel == stextup) {
			if (stextsval)
				stextsval--;
			return;
		}

		stextsel--;
		while (!stext[stextsel]._ssel) {
			if (!stextsel)
				stextsel = STORE_LINES - 1;
			else
				stextsel--;
		}
		return;
	}

	if (!stextsel)
		stextsel = STORE_LINES - 1;
	else
		stextsel--;

	while (!stext[stextsel]._ssel) {
		if (!stextsel)
			stextsel = STORE_LINES - 1;
		else
			stextsel--;
	}
}

void STextDown()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel == -1) {
		return;
	}

	if (stextscrl) {
		if (stextsel == stextdown) {
			if (stextsval < stextsmax)
				stextsval++;
			return;
		}

		stextsel++;
		while (!stext[stextsel]._ssel) {
			if (stextsel == STORE_LINES - 1)
				stextsel = 0;
			else
				stextsel++;
		}
		return;
	}

	do {
		if (stextsel == STORE_LINES - 1)
			stextsel = 0;
		else
			stextsel++;
	} while (!stext[stextsel]._ssel);
}

void STextPrior()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel != -1 && stextscrl) {
		if (stextsel == stextup) {
			if (stextsval)
				stextsval -= 4;
			stextsval = stextsval;
			if (stextsval < 0)
				stextsval = 0;
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
			if (stextsval < stextsmax)
				stextsval += 4;
			if (stextsval > stextsmax)
				stextsval = stextsmax;
		} else {
			stextsel = stextdown;
		}
	}
}

void S_SmithEnter()
{
	switch (stextsel) {
	case 10:
		talker = 0;
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

void SmithBuyItem()
{
	int idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_NORMAL)
		plr[myplr].HoldItem._iIdentified = FALSE;
	StoreAutoPlace();
	idx = stextvhold + ((stextlhold - stextup) >> 2);
	if (idx == SMITH_ITEMS - 1) {
		smithitem[SMITH_ITEMS - 1]._itype = ITYPE_NONE;
	} else {
		for (; smithitem[idx + 1]._itype != ITYPE_NONE; idx++) {
			smithitem[idx] = smithitem[idx + 1];
		}
		smithitem[idx]._itype = ITYPE_NONE;
	}
	CalcPlrInv(myplr, TRUE);
}

void S_SBuyEnter()
{
	int idx, i;
	BOOL done;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 12;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsval;
		stextshold = STORE_SBUY;
		idx = stextsval + ((stextsel - stextup) >> 2);
		if (plr[myplr]._pGold < smithitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			plr[myplr].HoldItem = smithitem[idx];
			SetCursor_(plr[myplr].HoldItem._iCurs + CURSOR_FIRSTITEM);
			done = FALSE;

			for (i = 0; i < NUM_INV_GRID_ELEM && !done; i++) {
				done = AutoPlace(myplr, i, cursW / 28, cursH / 28, FALSE);
			}
			if (done)
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
			SetCursor_(CURSOR_HAND);
		}
	}
}

void SmithBuyPItem()
{
	int i, xx, idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_NORMAL)
		plr[myplr].HoldItem._iIdentified = FALSE;
	StoreAutoPlace();

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
#ifdef HELLFIRE
	SpawnPremium(myplr);
#else
	SpawnPremium(plr[myplr]._pLevel);
#endif
}

void S_SPBuyEnter()
{
	int i, idx, xx;
	BOOL done;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 14;
	} else {
		stextshold = STORE_SPBUY;
		stextlhold = stextsel;
		stextvhold = stextsval;
		xx = stextsval + ((stextsel - stextup) >> 2);
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
			plr[myplr].HoldItem = premiumitem[idx];
			SetCursor_(plr[myplr].HoldItem._iCurs + CURSOR_FIRSTITEM);
			done = FALSE;
			for (i = 0; i < NUM_INV_GRID_ELEM && !done; i++) {
				done = AutoPlace(myplr, i, cursW / 28, cursH / 28, FALSE);
			}
			if (done)
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
			SetCursor_(CURSOR_HAND);
		}
	}
}

BOOL StoreGoldFit(int idx)
{
	int i, sz, cost, numsqrs;

	cost = storehold[idx]._iIvalue;
	sz = cost / GOLD_MAX_LIMIT;
	if (cost % GOLD_MAX_LIMIT)
		sz++;

	SetCursor_(storehold[idx]._iCurs + CURSOR_FIRSTITEM);
	numsqrs = cursW / 28 * (cursH / 28);
	SetCursor_(CURSOR_HAND);

	if (numsqrs >= sz)
		return TRUE;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		if (!plr[myplr].InvGrid[i])
			numsqrs++;
	}

	for (i = 0; i < plr[myplr]._pNumInv; i++) {
		if (plr[myplr].InvList[i]._itype == ITYPE_GOLD && plr[myplr].InvList[i]._ivalue != GOLD_MAX_LIMIT) {
			if (cost + plr[myplr].InvList[i]._ivalue <= GOLD_MAX_LIMIT)
				cost = 0;
			else
				cost -= GOLD_MAX_LIMIT - plr[myplr].InvList[i]._ivalue;
		}
	}

	sz = cost / GOLD_MAX_LIMIT;
	if (cost % GOLD_MAX_LIMIT)
		sz++;

	return numsqrs >= sz;
}

void PlaceStoreGold(int v)
{
	int ii, xx, yy, i;

	for (i = 0; i < NUM_INV_GRID_ELEM; i++) {
		yy = 10 * (i / 10);
		xx = i % 10;
		if (!plr[myplr].InvGrid[xx + yy]) {
			ii = plr[myplr]._pNumInv;
			GetGoldSeed(myplr, &golditem);
			plr[myplr].InvList[ii] = golditem;
			plr[myplr]._pNumInv++;
			plr[myplr].InvGrid[xx + yy] = plr[myplr]._pNumInv;
			SetGoldItemValue(&plr[myplr].InvList[ii], v);
			break;
		}
	}
}

void StoreSellItem()
{
	PlayerStruct *p;
	ItemStruct *pi;
	int i, idx, cost;

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	if (storehidx[idx] >= 0)
		RemoveInvItem(myplr, storehidx[idx]);
	else
		RemoveSpdBarItem(myplr, -(storehidx[idx] + 1));
	cost = storehold[idx]._iIvalue;
	storenumh--;
	while (idx < storenumh) {
		storehold[idx] = storehold[idx + 1];
		storehidx[idx] = storehidx[idx + 1];
		idx++;
	}
	p = &plr[myplr];
	p->_pGold += cost;
	for (i = 0; i < p->_pNumInv && cost > 0; i++) {
		pi = &p->InvList[i];
		if (pi->_itype == ITYPE_GOLD && pi->_ivalue != GOLD_MAX_LIMIT) {
			if (cost + pi->_ivalue <= GOLD_MAX_LIMIT) {
				SetGoldItemValue(pi, pi->_ivalue + cost);
				cost = 0;
			} else {
				cost -= GOLD_MAX_LIMIT - pi->_ivalue;
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

void S_SSellEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 16;
	} else {
		stextlhold = stextsel;
		idx = stextsval + ((stextsel - stextup) >> 2);
		stextshold = STORE_SSELL;
		stextvhold = stextsval;
		plr[myplr].HoldItem = storehold[idx];

		if (StoreGoldFit(idx))
			StartStore(STORE_CONFIRM);
		else
			StartStore(STORE_NOROOM);
	}
}

void SmithRepairItem()
{
	int i, idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	storehold[idx]._iDurability = storehold[idx]._iMaxDur;

	i = storehidx[idx];
	if (i < 0) {
		if (i == -1)
			plr[myplr].InvBody[INVLOC_HEAD]._iDurability = plr[myplr].InvBody[INVLOC_HEAD]._iMaxDur;
		if (i == -2)
			plr[myplr].InvBody[INVLOC_CHEST]._iDurability = plr[myplr].InvBody[INVLOC_CHEST]._iMaxDur;
		if (i == -3)
			plr[myplr].InvBody[INVLOC_HAND_LEFT]._iDurability = plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMaxDur;
		if (i == -4)
			plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iDurability = plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iMaxDur;
	} else {
		plr[myplr].InvList[i]._iDurability = plr[myplr].InvList[i]._iMaxDur;
	}
}

void S_SRepairEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_SMITH);
		stextsel = 18;
	} else {
		stextshold = STORE_SREPAIR;
		stextlhold = stextsel;
		stextvhold = stextsval;
		idx = stextsval + ((stextsel - stextup) >> 2);
		plr[myplr].HoldItem = storehold[idx];
		if (plr[myplr]._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

void S_WitchEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = 6;
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

void WitchBuyItem()
{
	int idx;

	idx = stextvhold + ((stextlhold - stextup) >> 2);

	if (idx < 3)
		plr[myplr].HoldItem._iSeed = GetRndSeed();

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	StoreAutoPlace();

	if (idx >= 3) {
		if (idx == WITCH_ITEMS - 1) {
			witchitem[WITCH_ITEMS - 1]._itype = ITYPE_NONE;
		} else {
			for (; witchitem[idx + 1]._itype != ITYPE_NONE; idx++) {
				witchitem[idx] = witchitem[idx + 1];
			}
			witchitem[idx]._itype = ITYPE_NONE;
		}
	}

	CalcPlrInv(myplr, TRUE);
}

void S_WBuyEnter()
{
	int i, idx;
	BOOL done;

	if (stextsel == 22) {
		StartStore(STORE_WITCH);
		stextsel = 14;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsval;
		stextshold = STORE_WBUY;
		idx = stextsval + ((stextsel - stextup) >> 2);

		if (plr[myplr]._pGold < witchitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			plr[myplr].HoldItem = witchitem[idx];
			SetCursor_(plr[myplr].HoldItem._iCurs + CURSOR_FIRSTITEM);
			done = FALSE;

			for (i = 0; i < NUM_INV_GRID_ELEM && !done; i++) {
				done = SpecialAutoPlace(myplr, i, cursW / 28, cursH / 28, FALSE);
			}

			if (done)
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);

			SetCursor_(CURSOR_HAND);
		}
	}
}

void S_WSellEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_WITCH);
		stextsel = 16;
	} else {
		stextlhold = stextsel;
		idx = stextsval + ((stextsel - stextup) >> 2);
		stextshold = STORE_WSELL;
		stextvhold = stextsval;
		plr[myplr].HoldItem = storehold[idx];
		if (StoreGoldFit(idx))
			StartStore(STORE_CONFIRM);
		else
			StartStore(STORE_NOROOM);
	}
}

void WitchRechargeItem()
{
	int i, idx;

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	storehold[idx]._iCharges = storehold[idx]._iMaxCharges;

	i = storehidx[idx];
	if (i < 0)
		plr[myplr].InvBody[INVLOC_HAND_LEFT]._iCharges = plr[myplr].InvBody[INVLOC_HAND_LEFT]._iMaxCharges;
	else
		plr[myplr].InvList[i]._iCharges = plr[myplr].InvList[i]._iMaxCharges;

	CalcPlrInv(myplr, TRUE);
}

void S_WRechargeEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_WITCH);
		stextsel = 18;
	} else {
		stextshold = STORE_WRECHARGE;
		stextlhold = stextsel;
		stextvhold = stextsval;
		idx = stextsval + ((stextsel - stextup) >> 2);
		plr[myplr].HoldItem = storehold[idx];
		if (plr[myplr]._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

void S_BoyEnter()
{
	if (boyitem._itype != ITYPE_NONE && stextsel == 18) {
		if (plr[myplr]._pGold < 50) {
			stextshold = STORE_BOY;
			stextlhold = 18;
			stextvhold = stextsval;
			StartStore(STORE_NOMONEY);
		} else {
			TakePlrsMoney(50);
			StartStore(STORE_BBOY);
		}
	} else if (stextsel == 8 && boyitem._itype != ITYPE_NONE || stextsel == 12 && boyitem._itype == ITYPE_NONE) {
		talker = 8;
		stextshold = STORE_BOY;
		stextlhold = stextsel;
		gossipstart = TEXT_WIRT2;
		gossipend = TEXT_WIRT12;
		StartStore(STORE_GOSSIP);
	} else {
		stextflag = STORE_NONE;
	}
}

void BoyBuyItem()
{
	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	StoreAutoPlace();
	boyitem._itype = ITYPE_NONE;
	stextshold = STORE_BOY;
	CalcPlrInv(myplr, TRUE);
}

void HealerBuyItem()
{
	int idx;

	idx = stextvhold + ((stextlhold - stextup) >> 2);
	if (gbMaxPlayers == 1) {
		if (idx < 2)
			plr[myplr].HoldItem._iSeed = GetRndSeed();
	} else {
		if (idx < 3)
			plr[myplr].HoldItem._iSeed = GetRndSeed();
	}

	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	if (plr[myplr].HoldItem._iMagical == ITEM_QUALITY_NORMAL)
		plr[myplr].HoldItem._iIdentified = FALSE;
	StoreAutoPlace();

	if (gbMaxPlayers == 1) {
		if (idx < 2)
			return;
	} else {
		if (idx < 3)
			return;
	}
	idx = stextvhold + ((stextlhold - stextup) >> 2);
	if (idx == 19) {
		healitem[19]._itype = ITYPE_NONE;
	} else {
		for (; healitem[idx + 1]._itype != ITYPE_NONE; idx++) {
			healitem[idx] = healitem[idx + 1];
		}
		healitem[idx]._itype = ITYPE_NONE;
	}
	CalcPlrInv(myplr, TRUE);
}

void S_BBuyEnter()
{
	BOOL done;
	int i;

	if (stextsel == 10) {
		stextshold = STORE_BBOY;
		stextvhold = stextsval;
		stextlhold = 10;
#ifdef HELLFIRE
		if (plr[myplr]._pGold < boyitem._iIvalue - (boyitem._iIvalue >> 2)) {
#else
		if (plr[myplr]._pGold < boyitem._iIvalue + (boyitem._iIvalue >> 1)) {
#endif
			StartStore(STORE_NOMONEY);
		} else {
			plr[myplr].HoldItem = boyitem;
#ifdef HELLFIRE
			plr[myplr].HoldItem._iIvalue -= plr[myplr].HoldItem._iIvalue >> 2;
#else
			plr[myplr].HoldItem._iIvalue += plr[myplr].HoldItem._iIvalue >> 1;
#endif
			SetCursor_(plr[myplr].HoldItem._iCurs + CURSOR_FIRSTITEM);
			done = FALSE;
			for (i = 0; i < NUM_INV_GRID_ELEM && !done; i++) {
				done = AutoPlace(myplr, i, cursW / 28, cursH / 28, FALSE);
			}
			if (done)
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
			SetCursor_(CURSOR_HAND);
		}
	} else {
		stextflag = STORE_NONE;
	}
}

void StoryIdItem()
{
	int idx;

	idx = storehidx[((stextlhold - stextup) >> 2) + stextvhold];
	if (idx < 0) {
		if (idx == -1)
			plr[myplr].InvBody[INVLOC_HEAD]._iIdentified = TRUE;
		if (idx == -2)
			plr[myplr].InvBody[INVLOC_CHEST]._iIdentified = TRUE;
		if (idx == -3)
			plr[myplr].InvBody[INVLOC_HAND_LEFT]._iIdentified = TRUE;
		if (idx == -4)
			plr[myplr].InvBody[INVLOC_HAND_RIGHT]._iIdentified = TRUE;
		if (idx == -5)
			plr[myplr].InvBody[INVLOC_RING_LEFT]._iIdentified = TRUE;
		if (idx == -6)
			plr[myplr].InvBody[INVLOC_RING_RIGHT]._iIdentified = TRUE;
		if (idx == -7)
			plr[myplr].InvBody[INVLOC_AMULET]._iIdentified = TRUE;
	} else {
		plr[myplr].InvList[idx]._iIdentified = TRUE;
	}
	plr[myplr].HoldItem._iIdentified = TRUE;
	TakePlrsMoney(plr[myplr].HoldItem._iIvalue);
	CalcPlrInv(myplr, TRUE);
}

void S_ConfirmEnter()
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
		stextsval = stextvhold;
	}
}

void S_HealerEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = 1;
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

void S_HBuyEnter()
{
	int i, idx;
	BOOL done;

	if (stextsel == 22) {
		StartStore(STORE_HEALER);
		stextsel = 16;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsval;
		stextshold = STORE_HBUY;
		idx = stextsval + ((stextsel - stextup) >> 2);
		if (plr[myplr]._pGold < healitem[idx]._iIvalue) {
			StartStore(STORE_NOMONEY);
		} else {
			plr[myplr].HoldItem = healitem[idx];
			SetCursor_(plr[myplr].HoldItem._iCurs + CURSOR_FIRSTITEM);
			done = FALSE;
			i = 0;
			for (i = 0; i < NUM_INV_GRID_ELEM && !done; i++) {
				done = SpecialAutoPlace(myplr, i, cursW / 28, cursH / 28, FALSE);
			}
			if (done)
				StartStore(STORE_CONFIRM);
			else
				StartStore(STORE_NOROOM);
			SetCursor_(CURSOR_HAND);
		}
	}
}

void S_StoryEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = 4;
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

void S_SIDEnter()
{
	int idx;

	if (stextsel == 22) {
		StartStore(STORE_STORY);
		stextsel = 14;
	} else {
		stextshold = STORE_SIDENTIFY;
		stextlhold = stextsel;
		stextvhold = stextsval;
		idx = stextsval + ((stextsel - stextup) >> 2);
		plr[myplr].HoldItem = storehold[idx];
		if (plr[myplr]._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

void S_TalkEnter()
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
		tq = gossipstart + random_(0, gossipend - gossipstart + 1);
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

void S_TavernEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = 3;
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

void S_BarmaidEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = 7;
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

void S_DrunkEnter()
{
	switch (stextsel) {
	case 12:
		stextlhold = 12;
		talker = 5;
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
			stextsval = stextvhold;
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
