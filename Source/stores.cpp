/**
 * @file stores.cpp
 *
 * Implementation of functionality for stores and towner dialogs.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"

DEVILUTION_BEGIN_NAMESPACE

// required size of the store to accomodate the relevant items
#define STORAGE_LIMIT (NUM_INV_GRID_ELEM + (MAXBELTITEMS > NUM_INVLOC ? MAXBELTITEMS : NUM_INVLOC))

// line numbers in stores:
#define STORE_CONFIRM_YES 18
#define STORE_CONFIRM_NO  20

#define STORE_LIST_FIRST  5
#define STORE_LIST_FOOTER 21
#define STORE_SCROLL_UP   4
#define STORE_SCROLL_DOWN 20
#define STORE_BACK        22

#define STORE_SMITH_GOSSIP 10
#define STORE_SMITH_BUY    12
#define STORE_SMITH_SPBUY  14
#define STORE_SMITH_SELL   16
#define STORE_SMITH_REPAIR 18
#define STORE_SMITH_EXIT   20

#define STORE_WITCH_GOSSIP   12
#define STORE_WITCH_BUY      14
#define STORE_WITCH_SELL     16
#define STORE_WITCH_RECHARGE 18
#define STORE_WITCH_EXIT     20

#define STORE_HEALER_GOSSIP 12
#define STORE_HEALER_HEAL   14
#define STORE_HEALER_BUY    16
#define STORE_HEALER_EXIT   18

#define STORE_STORY_GOSSIP   12
#define STORE_STORY_IDENTIFY 14
#define STORE_STORY_EXIT     18

#define STORE_PEGBOY_GOSSIP1 8
#define STORE_PEGBOY_QUERY   18
#define STORE_PEGBOY_EXIT1   20
#define STORE_PEGBOY_GOSSIP2 12
#define STORE_PEGBOY_EXIT2   18
#define STORE_PEGBOY_BUY     10

#define STORE_TAVERN_GOSSIP 12
#define STORE_TAVERN_EXIT   18

#define STORE_BARMAID_GOSSIP 12
#define STORE_BARMAID_EXIT   18

#define STORE_DRUNK_GOSSIP 12
#define STORE_DRUNK_EXIT   18

#define STORE_PRIEST_ERRAND 12
#define STORE_PRIEST_EXIT   18

#define STORE_ERRAND_YES    18
#define STORE_ERRAND_NO     20

// service prices
#define STORE_ID_PRICE     100
#define STORE_PEGBOY_PRICE 50

/* The current item in store. */
ItemStruct storeitem;
/* The item for sale by Wirt. */
ItemStruct boyitem;
/* The level of the item by Wirt. */
unsigned boylevel;
/* The standard items for sale by the smith. */
ItemStruct smithitem[SMITH_ITEMS];
/* The premium items for sale by the smith. */
ItemStruct premiumitems[SMITH_PREMIUM_ITEMS];
/* The base level of the premium items by the smith. */
int premiumlevel;
/* The number of premium items for sale. */
int numpremium;
/* The items for sale by the witch. */
ItemStruct witchitem[WITCH_ITEMS];
/* The items for sale by the healer. */
ItemStruct healitem[HEALER_ITEMS];

/** The current towner being interacted with */
static int talker;
/** The current interaction mode(STORE*) */
BYTE stextflag;
/** Is the current dialog full size */
static bool gbWidePanel;
/** Should 'Your gold ' text be displayed on the top-right corner */
static bool gbRenderGold;
/** Does the current panel have a scrollbar */
static bool gbHasScroll;
/** The index of the first visible item in the store. */
static int stextsidx;
/** The line number of the last visible item in the store */
static int stextdown;
/** Previous scroll position */
//static int stextup;
/** Remember stextflag while displaying a dialog */
static int stextshold;
/** Text lines */
static STextStruct stextlines[STORE_LINES];
/** Currently selected text line from stextlines */
static int stextsel;
/** Remember stextsel while displaying a dialog */
static int stextlhold;
/** The maximum possible value of stextsidx */
static int stextsmax;
/** Copies of the players items as presented in the store */
static ItemStruct storehold[STORAGE_LIMIT];
/** Map of inventory items being presented in the store */
static int8_t storehidx[STORAGE_LIMIT];
/** The number of valid entries in storehidx/storehold */
static int storenumh;
/** Remember stextsidx while displaying a dialog */
static int stextvhold;
/** Count down for the push state of the scroll up button */
static int8_t stextscrlubtn;
/** Count down for the push state of the scroll down button */
static int8_t stextscrldbtn;

/** Maps from towner IDs to NPC names. */
static const char* const talkname[] = {
	// clang-format off
/*TOWN_SMITH*/  "Griswold",
/*TOWN_HEALER*/ "Pepin",
/*TOWN_TAVERN*/ "Ogden",
/*TOWN_STORY*/  "Cain",
/*TOWN_DRUNK*/  "Farnham",
/*TOWN_WITCH*/  "Adria",
/*TOWN_BARMAID*/"Gillian",
/*TOWN_PEGBOY*/ "Wirt"
	// clang-format on
};

static void ClearSText(int s, int e)
{
	int i;

	for (i = s; i < e; i++) {
		stextlines[i]._sx = 0;
		stextlines[i]._syoff = 0;
		stextlines[i]._sstr[0] = '\0';
		stextlines[i]._sjust = false;
		stextlines[i]._sclr = COL_WHITE;
		stextlines[i]._sline = false;
		stextlines[i]._ssel = false;
		stextlines[i]._sval = -1;
	}
}

static unsigned StoresLimitedItemLvl()
{
	int l = myplr._pLevel;

	l += 2;
	return l;
}

void InitStoresOnce()
{
	int i;

	ClearSText(0, STORE_LINES);
	stextflag = STORE_NONE;
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	numpremium = 0;
	premiumlevel = StoresLimitedItemLvl();
	// assert((premiumlevel + premiumlvladd[0]) >= 1);

	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		premiumitems[i]._itype = ITYPE_NONE;

	boyitem._itype = ITYPE_NONE;
	boylevel = 0;
}

void InitLvlStores()
{
	unsigned l;

	assert(currLvl._dLevelIdx == DLV_TOWN);
	SetRndSeed(glSeedTbl[DLV_TOWN] * SDL_GetTicks());
	l = StoresLimitedItemLvl();
	SpawnSmith(l);
	SpawnWitch(l);
	SpawnHealer(l);
	SpawnBoy(l + 8);
	SpawnPremium(l);
}

void PrintSString(int x, int y, bool cjustflag, const char* str, BYTE col, int val)
{
	int sx, sy, px;
	int width, limit;
	char valstr[32];

	sx = (gbWidePanel ? LTPANEL_X + 7 : STORE_PNL_X + 7) + x;
	sy = LTPANEL_Y + 20 + y * 12 + stextlines[y]._syoff;
	limit = gbWidePanel ? LTPANEL_WIDTH - 7 * 2 : STPANEL_WIDTH - 7 * 2;
	limit -= x;
	if (cjustflag) {
		// assert(x == 0); -- otherwise limit -= x; ?
		width = GetSmallStringWidth(str);
		if (width < limit) {
			sx += (limit - width) >> 1;
		}
	}
	px = stextsel == y ? sx : INT_MAX;
	sx = PrintLimitedString(sx, sy, str, limit, col);
	if (val >= 0) {
		assert(!cjustflag && gbWidePanel);
		snprintf(valstr, sizeof(valstr), "%d", val);
		sx = LTPANEL_X + LTPANEL_WIDTH - (2 * SMALL_SCROLL_WIDTH + x + GetSmallStringWidth(valstr));
		PrintGameStr(sx, sy, valstr, col);
	}
	if (px != INT_MAX) {
		assert(cjustflag || gbWidePanel);
		DrawSmallPentSpn(px - FOCUS_SMALL, cjustflag ? sx + 6 : (LTPANEL_X + LTPANEL_WIDTH - (x + FOCUS_SMALL)), sy + 1);
	}
}

static void DrawSSlider(/*int y1, int y2*/)
{
	const int y1 = STORE_SCROLL_UP, y2 = STORE_SCROLL_DOWN;
	int x, i, yd1, yd2, yd3;

	//assert(LTPANEL_X + LTPANEL_WIDTH == STORE_PNL_X + STPANEL_WIDTH);
	//x = STORE_PNL_X + STPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2);
	assert(gbWidePanel);
	x = LTPANEL_X + LTPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2);
	yd1 = y1 * SMALL_SCROLL_HEIGHT + LTPANEL_Y + 20; // top position of the scrollbar
	yd2 = y2 * SMALL_SCROLL_HEIGHT + LTPANEL_Y + 20; // bottom position of the scrollbar
	yd3 = ((y2 - y1 - 2) * SMALL_SCROLL_HEIGHT);     // height of the scrollbar
	// draw the up arrow
	CelDraw(x, yd1, pSTextSlidCels, stextscrlubtn != -1 ? 12 : 10);
	// draw the down arrow
	CelDraw(x, yd2, pSTextSlidCels, stextscrldbtn != -1 ? 11 : 9);
	// the the bar
	yd1 += SMALL_SCROLL_HEIGHT;
	for (i = yd1; i < yd2; i += SMALL_SCROLL_HEIGHT) {
		CelDraw(x, i, pSTextSlidCels, 14);
	}
	// draw the scroll thumb
	if (stextsmax != 0) {
		yd3 = yd3 * stextsidx / stextsmax;
		CelDraw(x, yd1 + yd3, pSTextSlidCels, 13);
	}
}

void InitSTextHelp()
{
	stextsel = -1;
	gbWidePanel = true;
	// assert(gbRenderGold == false); - not necessary since it is not used in PrintSString
	// assert(gbHasScroll == false); - not necessary since it is not used in PrintSString
	ClearSText(0, STORE_LINES); // necessary to reset the _syoff values
}

static void AddSLine(int y)
{
	stextlines[y]._sline = true;
}

static void AddSTextVal(int y, int val)
{
	stextlines[y]._sval = val;
}

static void OffsetSTextY(int y, int yo)
{
	stextlines[y]._syoff = yo;
}

static void AddSText(int x, int y, bool j, const char* str, BYTE clr, bool sel)
{
	STextStruct* ss;

	ss = &stextlines[y];
	ss->_sx = x;
	ss->_syoff = 0;
	SStrCopy(ss->_sstr, str, sizeof(ss->_sstr));
	ss->_sjust = j;
	ss->_sclr = clr;
	ss->_sline = false;
	ss->_ssel = sel;
}

static BYTE StoreItemColor(const ItemStruct* is)
{
	if (!is->_iStatFlag)
		return COL_RED;
	if (is->_iMagical == ITEM_QUALITY_MAGIC)
		return COL_BLUE;
	if (is->_iMagical == ITEM_QUALITY_UNIQUE)
		return COL_GOLD;
	return COL_WHITE;
}

static void PrintStoreItem(const ItemStruct* is, int l, bool sel)
{
	int cursor;
	BYTE iclr = StoreItemColor(is);
	char sstr[128];

	AddSText(20, l, false, ItemName(is), iclr, sel);

	if (is->_iMagical != ITEM_QUALITY_NORMAL && !is->_iIdentified)
		return;
	l++;
	cursor = 0;
	if (is->_iMagical != ITEM_QUALITY_NORMAL) {
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
	}
	if (is->_iMaxCharges != 0) {
		if (cursor != 0)
			cat_cstr(sstr, cursor, ",  ");
		cat_str(sstr, cursor, "Charges: %d/%d", is->_iCharges, is->_iMaxCharges);
	}
	if (cursor != 0) {
		AddSText(40, l++, false, sstr, iclr, false);
		cursor = 0;
	}
	if (is->_iClass == ICLASS_WEAPON || is->_iClass == ICLASS_ARMOR) {
		if (is->_iClass == ICLASS_WEAPON) {
			if (is->_iMinDam == is->_iMaxDam)
				cat_str(sstr, cursor, "Damage: %d", is->_iMinDam);
			else
				cat_str(sstr, cursor, "Damage: %d-%d", is->_iMinDam, is->_iMaxDam);
		} else {
			cat_str(sstr, cursor, "Armor: %d", is->_iAC);
		}
		if (is->_iMaxDur != DUR_INDESTRUCTIBLE)
			cat_str(sstr, cursor, "  Dur: %d/%d", is->_iDurability, is->_iMaxDur);
		else
			cat_str(sstr, cursor, "  indestructible");
	}
	if ((is->_iMinStr | is->_iMinMag | is->_iMinDex) != 0) {
		if (cursor != 0)
			cat_cstr(sstr, cursor, ",  ");
		cat_cstr(sstr, cursor, "Req.:");
		if (is->_iMinStr != 0)
			cat_str(sstr, cursor, " %d Str", is->_iMinStr);
		if (is->_iMinMag != 0)
			cat_str(sstr, cursor, " %d Mag", is->_iMinMag);
		if (is->_iMinDex != 0)
			cat_str(sstr, cursor, " %d Dex", is->_iMinDex);
	}
	if (cursor != 0)
		AddSText(40, l++, false, sstr, iclr, false);
}

static void AddStoreFrame(const char* title)
{
	AddSText(10, 1, false, title, COL_GOLD, false);
	AddSLine(3);
	AddSLine(21);
	AddSText(0, STORE_BACK, true, "Back", COL_WHITE, true);
	OffsetSTextY(STORE_BACK, 6);
}

static void S_StartSmith()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 1, true, "Welcome to the", COL_GOLD, false);
	AddSText(0, 3, true, "Blacksmith's shop", COL_GOLD, false);
	AddSText(0, 7, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_SMITH_GOSSIP, true, "Talk to Griswold", COL_BLUE, true);
	AddSText(0, STORE_SMITH_BUY, true, "Buy basic items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_SPBUY, true, "Buy premium items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_SELL, true, "Sell items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_REPAIR, true, "Repair items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_EXIT, true, "Leave the shop", COL_WHITE, true);
	AddSLine(5);
}

static void StorePrepareItemBuy(ItemStruct* is)
{
	ItemStatOk(mypnum, is);
	if (is->_iMagical != ITEM_QUALITY_NORMAL)
		is->_iIdentified = TRUE;
}

static void S_ScrollSBuy()
{
	ItemStruct* is;
	int l;

	ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);
	//stextup = STORE_LIST_FIRST;

	is = &smithitem[stextsidx];
	for (l = STORE_LIST_FIRST; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			StorePrepareItemBuy(is);
			PrintStoreItem(is, l, true);
			AddSTextVal(l, is->_iIvalue);
			stextdown = l;
			is++;
		}
	}

	if (stextsel != -1 && /*stextsel != STORE_BACK &&*/ !stextlines[stextsel]._ssel)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartSBuy()
{
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; smithitem[i]._itype != ITYPE_NONE; i++)
		storenumh++;

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		//StartStore(STORE_SMITH);
		//stextshold = STORE_SMITH;
		//stextsel = STORE_SMITH_BUY;
		//return false;
		gbHasScroll = false;
		msg = "I have no basic item for sale.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSBuy();

		msg = "I have these basic items for sale:";
	}
	AddStoreFrame(msg);
	//return true;
}

static void S_ScrollSPBuy()
{
	ItemStruct* is;
	int idx, l, boughtitems;

	ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);
	//stextup = STORE_LIST_FIRST;

	boughtitems = stextsidx;
	for (idx = 0; boughtitems != 0; idx++)
		if (premiumitems[idx]._itype != ITYPE_NONE)
			boughtitems--;

	for (l = STORE_LIST_FIRST; l < 20 && idx < SMITH_PREMIUM_ITEMS; ) {
		is = &premiumitems[idx];
		if (is->_itype != ITYPE_NONE) {
			StorePrepareItemBuy(is);
			PrintStoreItem(is, l, true);
			AddSTextVal(l, is->_iIvalue);
			stextdown = l;
			l += 4;
		}
		idx++;
	}

	if (stextsel != -1 && /*stextsel != STORE_BACK &&*/ !stextlines[stextsel]._ssel)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartSPBuy()
{
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; i < SMITH_PREMIUM_ITEMS; i++)
		if (premiumitems[i]._itype != ITYPE_NONE)
			storenumh++;

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		//StartStore(STORE_SMITH);
		//stextshold = STORE_SMITH;
		//stextsel = STORE_SMITH_SPBUY;
		//return false;
		gbHasScroll = false;
		msg = "I have no premium item for sale.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSPBuy();

		msg = "I have these premium items for sale:";
	}
	AddStoreFrame(msg);
	//return true;
}

static void AddStoreSell(ItemStruct* is, int i)
{
	int value;

	copy_pod(storehold[storenumh], *is);

	is = &storehold[storenumh];
	value = (is->_iMagical != ITEM_QUALITY_NORMAL && is->_iIdentified) ? is->_iIvalue : is->_ivalue;

	value >>= 3;
	if (value == 0)
		value = 1;
	is->_iIvalue = is->_ivalue = value;

	storehidx[storenumh++] = i;
}

static bool SmithSellOk(const ItemStruct* is)
{
	return /* commented out because _ivalue of stackable items are not maintained
		(ITYPE_DURABLE(is->_itype) || is->_itype == ITYPE_MISC)
#ifdef HELLFIRE
	 && (is->_itype != ITYPE_MISC || (is->_iMiscId > IMISC_OILFIRST && is->_iMiscId < IMISC_OILLAST))
#else
	 && is->_itype != ITYPE_MISC
#endif
	 && is->_iClass != ICLASS_QUEST*/
		ITYPE_DURABLE(is->_itype) && (is->_itype != ITYPE_STAFF || is->_iSpell == SPL_NULL);
}

static void S_ScrollSSell()
{
	ItemStruct* is;
	int idx, l;

	ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);
	//stextup = STORE_LIST_FIRST;

	idx = stextsidx;
	for (l = STORE_LIST_FIRST; l < 20; l += 4) {
		is = &storehold[idx];
		if (is->_itype != ITYPE_NONE) {
			PrintStoreItem(is, l, true);
			AddSTextVal(l, is->_iMagical != ITEM_QUALITY_NORMAL && is->_iIdentified ? is->_iIvalue : is->_ivalue);
			stextdown = l;
		}
		idx++;
	}

	if (stextsel != -1 && /*stextsel != STORE_BACK &&*/ !stextlines[stextsel]._ssel)
		stextsel = stextdown;
	stextsmax = storenumh - 4;
	if (stextsmax < 0)
		stextsmax = 0;
}

static void S_StartSSell()
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &myplr;
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++)
		if (SmithSellOk(pi))
			AddStoreSell(pi, i);

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		gbHasScroll = false;
		msg = "You have nothing I want.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSSell();

		msg = "Which item is for sale?";
	}
	AddStoreFrame(msg);
}

static bool SmithRepairOk(const ItemStruct* is)
{
	return ITYPE_DURABLE(is->_itype) && is->_iDurability != is->_iMaxDur;
}

static void AddStoreHoldRepair(const ItemStruct* is, int i)
{
	ItemStruct* itm;
	int v;

	itm = &storehold[storenumh];
	copy_pod(*itm, *is);

	v = std::max(itm->_ivalue, itm->_iIvalue / 3);
	v = v * (itm->_iMaxDur - itm->_iDurability) / (itm->_iMaxDur * 2);
	if (v == 0)
		v = 1;

	itm->_iIvalue = v;
	itm->_ivalue = v;
	storehidx[storenumh] = i;
	storenumh++;
}

static void S_StartSRepair()
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &myplr;
	pi = p->_pInvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++)
		if (SmithRepairOk(pi))
			AddStoreHoldRepair(pi, -(i + 1));
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++)
		if (SmithRepairOk(pi))
			AddStoreHoldRepair(pi, i);

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		gbHasScroll = false;
		msg = "You have nothing to repair.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSSell();

		msg = "Repair which item?";
	}
	AddStoreFrame(msg);
}

static void S_StartWitch()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;

	AddSText(0, 2, true, "Witch's shack", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_WITCH_GOSSIP, true, "Talk to Adria", COL_BLUE, true);
	AddSText(0, STORE_WITCH_BUY, true, "Buy items", COL_WHITE, true);
	AddSText(0, STORE_WITCH_SELL, true, "Sell items", COL_WHITE, true);
	AddSText(0, STORE_WITCH_RECHARGE, true, "Recharge staves", COL_WHITE, true);
	AddSText(0, STORE_WITCH_EXIT, true, "Leave the shack", COL_WHITE, true);
	AddSLine(5);
}

static void S_ScrollWBuy()
{
	ItemStruct* is;
	int l;

	ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);
	//stextup = STORE_LIST_FIRST;

	is = &witchitem[stextsidx];
	for (l = STORE_LIST_FIRST; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			StorePrepareItemBuy(is);
			PrintStoreItem(is, l, true);
			AddSTextVal(l, is->_iIvalue);
			stextdown = l;
			is++;
		}
	}

	if (stextsel != -1 && /*stextsel != STORE_BACK &&*/ !stextlines[stextsel]._ssel)
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

	gbWidePanel = true;
	gbRenderGold = true;
	gbHasScroll = true;
	stextsidx = 0;
	S_ScrollWBuy();

	AddStoreFrame("I have these items for sale:");
}

static bool WitchSellOk(const ItemStruct* is)
{
	/* commented out because _ivalue of stackable items are not maintained
#ifdef HELLFIRE
	return ((is->_itype == ITYPE_MISC && (is->_iMiscId < IMISC_OILFIRST || is->_iMiscId > IMISC_OILLAST))
	 || (is->_itype == ITYPE_STAFF && is->_iSpell != SPL_NULL))
#else
	return (is->_itype == ITYPE_MISC || is->_itype == ITYPE_STAFF)
#endif
		&& is->_iClass != ICLASS_QUEST;*/
	return (is->_itype == ITYPE_STAFF && is->_iSpell != SPL_NULL);
}

static void S_StartWSell()
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &myplr;
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++)
		if (WitchSellOk(pi))
			AddStoreSell(pi, i);

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		gbHasScroll = false;
		msg = "You have nothing I want.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSSell();
		msg = "Which item is for sale?";
	}
	AddStoreFrame(msg);
}

static bool WitchRechargeOk(const ItemStruct* is)
{
	return is->_itype != ITYPE_NONE && is->_itype != ITYPE_PLACEHOLDER && is->_iCharges != is->_iMaxCharges;
}

static void AddStoreHoldRecharge(const ItemStruct* is, int i)
{
	ItemStruct* itm;

	itm = &storehold[storenumh];
	copy_pod(*itm, *is);
	itm->_ivalue += spelldata[itm->_iSpell].sStaffCost;
	itm->_ivalue = itm->_ivalue * (itm->_iMaxCharges - itm->_iCharges) / itm->_iMaxCharges;
	itm->_iIvalue = itm->_ivalue;
	storehidx[storenumh] = i;
	storenumh++;
}

static void S_StartWRecharge()
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &myplr;
	pi = p->_pInvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++)
		if (WitchRechargeOk(pi))
			AddStoreHoldRecharge(pi, -(i + 1));
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++)
		if (WitchRechargeOk(pi))
			AddStoreHoldRecharge(pi, i);

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		gbHasScroll = false;
		msg = "You have nothing to recharge.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSSell();

		msg = "Recharge which item?";
	}
	AddStoreFrame(msg);
}

static void S_StartNoMoney()
{
	// StartStore(stextshold);
	// ClearSText(STORE_LIST_FIRST, STORE_LINES);
	gbHasScroll = false;
	gbWidePanel = true;
	gbRenderGold = false;
	AddSText(0, 14, true, "You do not have enough gold", COL_WHITE, true);

	AddSLine(3);
	AddSLine(21);
}

static void S_StartNoRoom()
{
	// StartStore(stextshold);
	// ClearSText(STORE_LIST_FIRST, STORE_LINES);
	gbHasScroll = false;
	AddSText(0, 14, true, "You do not have enough room in inventory", COL_WHITE, true);

	AddSLine(3);
	AddSLine(21);
}

static void S_StartWait()
{
	if (pcursicon == CURSOR_HAND)
		NewCursor(CURSOR_HOURGLASS);
	stextflag = STORE_WAIT;
}

static void S_StartConfirm()
{
	// StartStore(stextshold);
	// ClearSText(STORE_LIST_FIRST, STORE_LINES);
	gbHasScroll = false;
	PrintStoreItem(&storeitem, 8, false);
	AddSTextVal(8, storeitem._iIvalue);
	AddSLine(3);
	AddSLine(21);

	switch (stextshold) {
	case STORE_PBUY:
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
	default:
		ASSUME_UNREACHABLE
		break;
	}
	AddSText(0, 15, true, tempstr, COL_WHITE, false);
	AddSText(0, STORE_CONFIRM_YES, true, "Yes", COL_WHITE, true);
	AddSText(0, STORE_CONFIRM_NO, true, "No", COL_WHITE, true);
}

static void S_StartBoy()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 2, true, "Wirt the Peg-legged boy", COL_GOLD, false);
	AddSLine(5);
	if (boyitem._itype != ITYPE_NONE) {
		AddSText(0, STORE_PEGBOY_GOSSIP1, true, "Talk to Wirt", COL_BLUE, true);
		AddSText(0, 12, true, "I have something for sale,", COL_GOLD, false);
		if (!boyitem._iIdentified) {
			static_assert(STORE_PEGBOY_PRICE == 50, "Hardcoded boy price is 50.");
			AddSText(0, 14, true, "but it will cost 50 gold", COL_GOLD, false);
			AddSText(0, 16, true, "just to take a look. ", COL_GOLD, false);
		}
		AddSText(0, STORE_PEGBOY_QUERY, true, "What have you got?", COL_WHITE, true);
		AddSText(0, STORE_PEGBOY_EXIT1, true, "Say goodbye", COL_WHITE, true);
	} else {
		AddSText(0, STORE_PEGBOY_GOSSIP2, true, "Talk to Wirt", COL_BLUE, true);
		AddSText(0, STORE_PEGBOY_EXIT2, true, "Say goodbye", COL_WHITE, true);
	}
}

static void S_StartBBoy()
{
	if (boyitem._itype == ITYPE_NONE) {
		stextflag = STORE_PEGBOY;
		stextlhold = STORE_PEGBOY_EXIT2;
		S_StartBoy();
		return;
	}
	gbWidePanel = true;
	gbRenderGold = true;
	gbHasScroll = false;

	StorePrepareItemBuy(&boyitem);
	PrintStoreItem(&boyitem, STORE_PEGBOY_BUY, true);
	AddSTextVal(STORE_PEGBOY_BUY, boyitem._iIvalue);

	AddStoreFrame("I have this item for sale:");
}

static void S_StartHealer()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 1, true, "Welcome to the", COL_GOLD, false);
	AddSText(0, 3, true, "Healer's home", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_HEALER_GOSSIP, true, "Talk to Pepin", COL_BLUE, true);
	AddSText(0, STORE_HEALER_HEAL, true, "Receive healing", COL_WHITE, true);
	AddSText(0, STORE_HEALER_BUY, true, "Buy items", COL_WHITE, true);
	AddSText(0, STORE_HEALER_EXIT, true, "Leave Healer's home", COL_WHITE, true);
	AddSLine(5);
}

static void S_ScrollHBuy()
{
	ItemStruct* is;
	int l;

	ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);
	//stextup = STORE_LIST_FIRST;

	is = &healitem[stextsidx];
	for (l = STORE_LIST_FIRST; l < 20; l += 4) {
		if (is->_itype != ITYPE_NONE) {
			StorePrepareItemBuy(is);
			PrintStoreItem(is, l, true);
			AddSTextVal(l, is->_iIvalue);
			stextdown = l;
			is++;
		}
	}

	if (stextsel != -1 && /*stextsel != STORE_BACK &&*/ !stextlines[stextsel]._ssel)
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

	gbWidePanel = true;
	gbRenderGold = true;
	gbHasScroll = true;
	stextsidx = 0;
	S_ScrollHBuy();

	AddStoreFrame("I have these items for sale:");
}

static void S_StartStory()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 2, true, "The Town Elder", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_STORY_GOSSIP, true, "Talk to Cain", COL_BLUE, true);
	AddSText(0, STORE_STORY_IDENTIFY, true, "Identify an item", COL_WHITE, true);
	AddSText(0, STORE_STORY_EXIT, true, "Say goodbye", COL_WHITE, true);
	AddSLine(5);
}

static bool IdItemOk(const ItemStruct* is)
{
	return is->_itype != ITYPE_NONE && is->_itype != ITYPE_PLACEHOLDER
		&& is->_iMagical != ITEM_QUALITY_NORMAL
		&& !is->_iIdentified;
}

static void AddStoreHoldId(const ItemStruct* is, int i)
{
	ItemStruct* holditem;

	holditem = &storehold[storenumh];
	copy_pod(*holditem, *is);
	holditem->_ivalue = STORE_ID_PRICE;
	holditem->_iIvalue = STORE_ID_PRICE;
	storehidx[storenumh] = i;
	storenumh++;
}

static void S_StartSIdentify()
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i;
	const char* msg;

	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;

	p = &myplr;
	pi = p->_pInvBody;
	for (i = 0; i < NUM_INVLOC; i++, pi++)
		if (IdItemOk(pi))
			AddStoreHoldId(pi, -(i + 1));
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++)
		if (IdItemOk(pi))
			AddStoreHoldId(pi, i);

	gbWidePanel = true;
	gbRenderGold = true;
	if (storenumh == 0) {
		gbHasScroll = false;
		msg = "You have nothing to identify.";
	} else {
		gbHasScroll = true;
		stextsidx = 0;
		S_ScrollSSell();

		msg = "Identify which item?";
	}
	AddStoreFrame(msg);
}

static void S_StartIdShow()
{
	//assert(stextshold == STORE_SIDENTIFY);
	//StartStore(STORE_SIDENTIFY);
	//ClearSText(STORE_LIST_FIRST, STORE_LINES);

	//gbWidePanel = true;
	//gbRenderGold = true;
	gbHasScroll = false;

	PrintStoreItem(&storeitem, 11, false);

	AddStoreFrame("This item is:");
}

static void S_StartTalk()
{
	int i, sn, la;

	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	snprintf(tempstr, sizeof(tempstr), "Talk to %s", talkname[talker]);
	AddSText(0, 2, true, tempstr, COL_GOLD, false);
	AddSLine(5);
	sn = 0;
	for (i = 0; i < NUM_QUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && Qtalklist[talker][i] != TEXT_NONE && quests[i]._qlog)
			sn++;
	}

	if (sn > 6) {
		sn = 14 - std::min(sn >> 1, 12);
		la = 1;
	} else {
		sn = 15 - sn;
		la = 2;
	}

	AddSText(0, sn - 2, true, "Gossip", COL_BLUE, true);
	for (i = 0; i < NUM_QUESTS && sn < 22; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && Qtalklist[talker][i] != TEXT_NONE && quests[i]._qlog) {
			AddSText(0, sn, true, questlist[i]._qlstr, COL_WHITE, true);
			sn += la;
		}
	}
	AddSText(0, 22, true, "Back", COL_WHITE, true);
}

static void S_StartTavern()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 1, true, "Welcome to the", COL_GOLD, false);
	AddSText(0, 3, true, "Rising Sun", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_TAVERN_GOSSIP, true, "Talk to Ogden", COL_BLUE, true);
	AddSText(0, STORE_TAVERN_EXIT, true, "Leave the tavern", COL_WHITE, true);
	AddSLine(5);
}

static void S_StartBarMaid()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 2, true, "Gillian", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_BARMAID_GOSSIP, true, "Talk to Gillian", COL_BLUE, true);
	AddSText(0, STORE_BARMAID_EXIT, true, "Say goodbye", COL_WHITE, true);
	AddSLine(5);
}

static void S_StartDrunk()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 2, true, "Farnham the Drunk", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_DRUNK_GOSSIP, true, "Talk to Farnham", COL_BLUE, true);
	AddSText(0, STORE_DRUNK_EXIT, true, "Say Goodbye", COL_WHITE, true);
	AddSLine(5);
}

static void S_StartPriest()
{
	gbWidePanel = false;
	gbRenderGold = false;
	gbHasScroll = false;
	AddSText(0, 2, true, "Tremain the Priest", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_PRIEST_ERRAND, true, "Run errand", COL_BLUE, true);
	AddSText(0, STORE_PRIEST_EXIT, true, "Say Goodbye", COL_WHITE, true);
	AddSLine(5);
}

static void S_StartErrand()
{
	AddSText(0, 2, true, "Tremain the Priest", COL_GOLD, false);
	// AddSText(0, 12, true, "Would you like to", COL_WHITE, false);
	AddSText(0, 14, true, "Go on an errand?", COL_WHITE, false);
	AddSText(0, STORE_ERRAND_YES, true, "Yes", COL_WHITE, true);
	AddSText(0, STORE_ERRAND_NO, true, "No", COL_WHITE, true);
	AddSLine(5);
}

void StartStore(int s)
{
	int i;

	ClearSText(0, STORE_LINES);
	ReleaseStoreBtn();
	stextflag = s;
	switch (s) {
	case STORE_SMITH:
		S_StartSmith();
		break;
	case STORE_SBUY:
		//if (!S_StartSBuy())
		//	return;
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
	case STORE_PEGBOY:
		S_StartBoy();
		break;
	case STORE_PBUY:
		S_StartBBoy();
		break;
	case STORE_HEALER:
		S_StartHealer();
		break;
	case STORE_STORY:
		S_StartStory();
		break;
	case STORE_HBUY:
		S_StartHBuy();
		break;
	case STORE_SIDENTIFY:
		S_StartSIdentify();
		break;
	case STORE_SPBUY:
		//if (!S_StartSPBuy())
		//	return;
		S_StartSPBuy();
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
	case STORE_PRIEST:
		S_StartPriest();
		break;
	case STORE_ERRAND:
		S_StartErrand();
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	for (i = 0; i < STORE_LINES; i++) {
		if (stextlines[i]._ssel)
			break;
	}

	stextsel = i == STORE_LINES ? -1 : i;
}

void DrawStore()
{
	STextStruct* sts;
	int i;

	if (gbWidePanel)
		DrawTextBox();
	else
		DrawSTextBox(STORE_PNL_X, LTPANEL_Y);

	if (gbHasScroll) {
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
		default:
			ASSUME_UNREACHABLE
			break;
		}
	}

	for (i = 0; i < STORE_LINES; i++) {
		sts = &stextlines[i];
		if (sts->_sline)
			DrawTextBoxSLine(gbWidePanel ? LTPANEL_X : STORE_PNL_X, LTPANEL_Y, i * 12 + 14, gbWidePanel);
		if (sts->_sstr[0] != '\0')
			PrintSString(sts->_sx, i, sts->_sjust, sts->_sstr, sts->_sclr, sts->_sval);
	}

	if (gbRenderGold) {
		snprintf(tempstr, sizeof(tempstr), "Your gold: %d", myplr._pGold);
		// assert(gbWidePanel);
		PrintSString(LTPANEL_WIDTH - 178, 1, false, tempstr, COL_GOLD);
	}
	if (gbHasScroll)
		DrawSSlider();
}

void STextESC()
{
	assert(!gbQtextflag);
	switch (stextflag) {
	case STORE_SMITH:
	case STORE_WITCH:
	case STORE_PEGBOY:
	case STORE_HEALER:
	case STORE_STORY:
	case STORE_TAVERN:
	case STORE_DRUNK:
	case STORE_BARMAID:
	case STORE_PRIEST:
		stextflag = STORE_NONE;
		break;
	case STORE_GOSSIP:
		StartStore(stextshold);
		stextsel = stextlhold;
		break;
	case STORE_SBUY:
		StartStore(STORE_SMITH);
		stextsel = STORE_SMITH_BUY;
		break;
	case STORE_SPBUY:
		StartStore(STORE_SMITH);
		stextsel = STORE_SMITH_SPBUY;
		break;
	case STORE_SSELL:
		StartStore(STORE_SMITH);
		stextsel = STORE_SMITH_SELL;
		break;
	case STORE_SREPAIR:
		StartStore(STORE_SMITH);
		stextsel = STORE_SMITH_REPAIR;
		break;
	case STORE_WBUY:
		StartStore(STORE_WITCH);
		stextsel = STORE_WITCH_BUY;
		break;
	case STORE_WSELL:
		StartStore(STORE_WITCH);
		stextsel = STORE_WITCH_SELL;
		break;
	case STORE_WRECHARGE:
		StartStore(STORE_WITCH);
		stextsel = STORE_WITCH_RECHARGE;
		break;
	case STORE_HBUY:
		StartStore(STORE_HEALER);
		stextsel = STORE_HEALER_BUY;
		break;
	case STORE_SIDENTIFY:
		StartStore(STORE_STORY);
		stextsel = STORE_STORY_IDENTIFY;
		break;
	case STORE_PBUY:
		StartStore(STORE_PEGBOY);
		stextsel = STORE_PEGBOY_QUERY;
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
	case STORE_ERRAND:
		StartStore(STORE_PRIEST);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void STextUp()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel == -1) {
		return;
	}

	if (gbHasScroll && stextsel == STORE_LIST_FIRST) {
		if (stextsidx != 0)
			stextsidx--;
		return;
	}

	do {
		stextsel--;
		if (stextsel < 0)
			stextsel = STORE_LINES - 1;
	} while (!stextlines[stextsel]._ssel);
}

void STextDown()
{
	PlaySFX(IS_TITLEMOV);
	if (stextsel == -1) {
		return;
	}

	if (gbHasScroll && stextsel == stextdown) {
		if (stextsidx < stextsmax)
			stextsidx++;
		return;
	}

	do {
		stextsel++;
		if (stextsel == STORE_LINES)
			stextsel = 0;
	} while (!stextlines[stextsel]._ssel);
}

void STextPageUp()
{
	if (stextsel != -1 && gbHasScroll) {
		PlaySFX(IS_TITLEMOV);
		stextsidx -= 4;
		if (stextsidx < 0) {
			stextsidx = 0;
			stextsel = STORE_LIST_FIRST;
		}
	}
}

void STextPageDown()
{
	if (stextsel != -1 && gbHasScroll) {
		PlaySFX(IS_TITLEMOV);
		stextsidx += 4;
		if (stextsidx > stextsmax) {
			stextsidx = stextsmax;
			stextsel = stextdown;
		}
	}
}

static void S_SmithEnter()
{
	switch (stextsel) {
	case STORE_SMITH_GOSSIP:
		stextlhold = STORE_SMITH_GOSSIP;
		talker = TOWN_SMITH;
		stextshold = STORE_SMITH;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_SMITH_BUY:
		StartStore(STORE_SBUY);
		break;
	case STORE_SMITH_SPBUY:
		StartStore(STORE_SPBUY);
		break;
	case STORE_SMITH_SELL:
		StartStore(STORE_SSELL);
		break;
	case STORE_SMITH_REPAIR:
		StartStore(STORE_SREPAIR);
		break;
	case STORE_SMITH_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

bool TakePlrsMoney(int pnum, int cost)
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i, value;

	p = &plr;
	if (p->_pGold < cost)
		return false;
	p->_pGold -= cost;
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM && cost > 0; i++, pi++) {
		if (pi->_itype != ITYPE_GOLD)
			continue;
		value = pi->_ivalue;
		if (value == GOLD_MAX_LIMIT)
			continue;
		cost -= value;
		if (cost < 0) {
			SetGoldItemValue(pi, -cost);
		} else {
			pi->_itype = ITYPE_NONE;
		}
	}
	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM && cost > 0; i++, pi++) {
		if (pi->_itype != ITYPE_GOLD)
			continue;
		value = pi->_ivalue;
		cost -= value;
		if (cost < 0) {
			SetGoldItemValue(pi, -cost);
		} else {
			pi->_itype = ITYPE_NONE;
		}
	}
	return true;
}

static bool StoreAutoPlace(int pnum, ItemStruct* is, bool saveflag)
{
	return /*WeaponAutoPlace(pnum, is, saveflag) ||*/ AutoPlaceBelt(pnum, is, saveflag) || AutoPlaceInv(pnum, is, saveflag);
}

/**
 * @brief Purchases an item from the smith.
 */
static void SmithBuyItem()
{
	int idx;

	SendStoreCmd2(STORE_SBUY);

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);
	do {
		copy_pod(smithitem[idx], smithitem[idx + 1]);
		idx++;
	} while (smithitem[idx]._itype != ITYPE_NONE);
}

static void StoreStartBuy(ItemStruct* is, int price)
{
	if (myplr._pGold < price) {
		StartStore(STORE_NOMONEY);
	} else {
		copy_pod(storeitem, *is);
		//storeitem._iIvalue = price; // only for boyitem
		if (StoreAutoPlace(mypnum, &storeitem, false))
			StartStore(STORE_CONFIRM);
		else
			StartStore(STORE_NOROOM);
	}
}

static void S_SBuyEnter()
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_SMITH);
		// stextsel = STORE_SMITH_BUY;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsidx;
		stextshold = STORE_SBUY;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
		StoreStartBuy(&smithitem[idx], smithitem[idx]._iIvalue);
	}
}

/**
 * @brief Purchases a premium item from the smith.
 */
static void SmithBuyPItem()
{
	int i, xx, idx;

	SendStoreCmd2(STORE_SPBUY);

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);
	xx = 0;
	for (i = 0; idx >= 0; i++) {
		if (premiumitems[i]._itype != ITYPE_NONE) {
			idx--;
			xx = i;
		}
	}

	premiumitems[xx]._itype = ITYPE_NONE;
	numpremium--;
	//SpawnPremium(StoresLimitedItemLvl());
}

static void S_SPBuyEnter()
{
	int i, idx, xx;

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_SMITH);
		// stextsel = STORE_SMITH_SPBUY;
	} else {
		stextshold = STORE_SPBUY;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		xx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
		idx = 0;
		for (i = 0; xx >= 0; i++) {
			if (premiumitems[i]._itype != ITYPE_NONE) {
				xx--;
				idx = i;
			}
		}
		StoreStartBuy(&premiumitems[idx], premiumitems[idx]._iIvalue);
	}
}

static bool StoreGoldFit(int cost, int slotCurs)
{
	ItemStruct* pi;
	int i, numsqrs;

	// add the item slots but only if it is not in the belt, since gold can not be placed there
	if (slotCurs != CURSOR_NONE) {
		numsqrs = InvItemHeight[slotCurs] * InvItemWidth[slotCurs] / (INV_SLOT_SIZE_PX * INV_SLOT_SIZE_PX);
		cost -= numsqrs * GOLD_MAX_LIMIT;
	}
	pi = myplr._pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_itype == ITYPE_NONE)
			// add the empty slots
			cost -= GOLD_MAX_LIMIT;
		else if (pi->_itype == ITYPE_GOLD)
			// check for not full piles of gold
			cost -= GOLD_MAX_LIMIT - pi->_ivalue;
	}

	return cost <= 0;
}

/**
 * @brief Add gold pile to the players inventory
 * @param v The value of the gold pile
 */
static void PlaceStoreGold(PlayerStruct* p, int v)
{
	ItemStruct* pi;
	int i;

	pi = p->_pInvList;
	for (i = 0; i < NUM_INV_GRID_ELEM; i++, pi++) {
		if (pi->_itype == ITYPE_NONE) {
			CreateBaseItem(pi, IDI_GOLD);
			SetGoldItemValue(pi, v);
			break;
		}
	}
}

static bool SyncSellItem(int pnum, int cii, int cost)
{
	PlayerStruct* p;
	ItemStruct* pi;
	int i, val;

	// assert(cii < NUM_INVELEM);

	pi = PlrItem(pnum, cii);
	if (pi->_itype == ITYPE_NONE)
		return false;
	i = pi->_iCurs + CURSOR_FIRSTITEM;
	if (cii > INVITEM_INV_LAST || cii < INVITEM_INV_FIRST)
		i = CURSOR_NONE;
	if (!StoreGoldFit(cost, i))
		return false;

	// set seed from the item to ensure synchronized (gold stack-)seeds generated by PlaceStoreGold
	SetRndSeed(pi->_iSeed);
	// remove item from the inventory
	SyncPlrItemRemove(pnum, cii);
	// AddPlrMoney
	p = &plr;
	p->_pGold += cost;
	pi = p->_pInvList;
	for (i = NUM_INV_GRID_ELEM; i > 0 && cost > 0; i--, pi++) {
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
			PlaceStoreGold(p, GOLD_MAX_LIMIT);
			cost -= GOLD_MAX_LIMIT;
		}
		PlaceStoreGold(p, cost);
	}
	return true;
}

void SyncStoreCmd(int pnum, int cmd, int ii, int price)
{
	ItemStruct* pi;
	BYTE lastshold, nextMode = STORE_NONE;

	// assert(plr._pDunLevel == DLV_TOWN);

	if (pnum == mypnum && stextflag != STORE_NONE) {
		if (stextflag == STORE_WAIT) {
			nextMode = stextshold;
			if (pcursicon == CURSOR_HOURGLASS && gnTimeoutCurs == CURSOR_NONE)
				NewCursor(CURSOR_HAND);
		}
		stextflag = STORE_NONE;
	}

	lastshold = cmd;
	switch (lastshold) {
	case STORE_HBUY:
	case STORE_SBUY:
	case STORE_SPBUY:
	case STORE_WBUY:
	case STORE_PBUY:
		// assert(ii == MAXITEMS);
		pi = &items[MAXITEMS];
		// TODO: validate price?
		//StorePrepareItemBuy(pi);
		if (pi->_iMagical != ITEM_QUALITY_NORMAL)
			pi->_iIdentified = TRUE;
		ItemStatOk(pnum, pi);
		if (!StoreAutoPlace(pnum, pi, false) || !TakePlrsMoney(pnum, price))
			return;
		StoreAutoPlace(pnum, pi, true);
		break;
	case STORE_SSELL:
		if (!SyncSellItem(pnum, ii, price)) {
			return;
		}
		lastshold = nextMode;
		break;
	case STORE_SIDENTIFY:
		assert(price == STORE_ID_PRICE);
		if (!TakePlrsMoney(pnum, STORE_ID_PRICE))
			return;
		pi = PlrItem(pnum, ii);
		if (pi->_iMagical != ITEM_QUALITY_NORMAL) {
			pi->_iIdentified = TRUE;
		}
		lastshold = STORE_IDSHOW;
		break;
	case STORE_SREPAIR:
		if (!TakePlrsMoney(pnum, price))
			return;
		pi = PlrItem(pnum, ii);
		// TODO: validate price?
		pi->_iDurability = pi->_iMaxDur;
		break;
	case STORE_WRECHARGE:
		if (!TakePlrsMoney(pnum, price))
			return;
		pi = PlrItem(pnum, ii);
		// TODO: validate price?
		pi->_iCharges = pi->_iMaxCharges;
		break;
	case STORE_PEGBOY:
		assert(price == STORE_PEGBOY_PRICE);
		if (!TakePlrsMoney(pnum, STORE_PEGBOY_PRICE))
			return;
		//lastshold = STORE_PEGBOY;
		lastshold = STORE_PBUY;
		break;
	}

	CalcPlrInv(pnum, true);

	// pnum != mypnum or the current player was impatient -> done
	if (nextMode == STORE_NONE)
		return;

	StartStore(lastshold);
	// deliberate redirect -> done
	if (stextshold != lastshold)
		return;
	// store page is empty -> done
	if (stextsel == STORE_BACK) {
		return;
	}

	stextsel = stextlhold;
	stextsidx = std::min(stextvhold, stextsmax);

	while (stextsel != -1 && !stextlines[stextsel]._ssel) {
		stextsel--;
	}
}

/**
 * @brief Sells an item from the player's inventory or belt.
 */
static void StoreSellItem()
{
	int i, idx, cost;

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);
	i = storehidx[idx];
	if (i >= 0) {
		i += INVITEM_INV_FIRST;
	} else {
		i = INVITEM_BELT_FIRST - (i + 1);
	}
	cost = storehold[idx]._iIvalue;
	SendStoreCmd1(i, STORE_SSELL, cost);

	storenumh--;
	while (idx < storenumh) {
		copy_pod(storehold[idx], storehold[idx + 1]);
		storehidx[idx] = storehidx[idx + 1];
		idx++;
	}
}

static void S_SSell()
{
	int idx;

	stextlhold = stextsel;
	idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
	stextshold = stextflag;
	stextvhold = stextsidx;
	copy_pod(storeitem, storehold[idx]);

	idx = storehidx[idx] >= 0 ? storeitem._iCurs + CURSOR_FIRSTITEM : CURSOR_NONE;
	if (StoreGoldFit(storeitem._iIvalue, idx))
		StartStore(STORE_CONFIRM);
	else
		StartStore(STORE_NOROOM);
}

static void S_SSellEnter()
{
	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_SMITH);
		// stextsel = STORE_SMITH_SELL;
	} else {
		S_SSell();
	}
}

/**
 * @brief Repairs an item in the player's inventory or body in the smith.
 */
static void SmithRepairItem()
{
	int i, idx;

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);

	i = storehidx[idx];
	if (i < 0) {
		i = INVITEM_BODY_FIRST - (i + 1);
	} else {
		i += INVITEM_INV_FIRST;
	}

	SendStoreCmd1(i, STORE_SREPAIR, storeitem._iIvalue);
}

static void S_SRepairEnter()
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_SMITH);
		// stextsel = STORE_SMITH_REPAIR;
	} else {
		stextshold = STORE_SREPAIR;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
		copy_pod(storeitem, storehold[idx]);
		if (myplr._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_WitchEnter()
{
	switch (stextsel) {
	case STORE_WITCH_GOSSIP:
		stextlhold = STORE_WITCH_GOSSIP;
		talker = TOWN_WITCH;
		stextshold = STORE_WITCH;
		StartStore(STORE_GOSSIP);
		return;
	case STORE_WITCH_BUY:
		StartStore(STORE_WBUY);
		return;
	case STORE_WITCH_SELL:
		StartStore(STORE_WSELL);
		return;
	case STORE_WITCH_RECHARGE:
		StartStore(STORE_WRECHARGE);
		return;
	case STORE_WITCH_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

/**
 * @brief Purchases an item from the witch.
 */
static void WitchBuyItem()
{
	int idx;

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);

	if (idx < 3)
		storeitem._iSeed = NextRndSeed();

	SendStoreCmd2(STORE_WBUY);

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

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_WITCH);
		// stextsel = STORE_WITCH_BUY;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsidx;
		stextshold = STORE_WBUY;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);

		StoreStartBuy(&witchitem[idx], witchitem[idx]._iIvalue);
	}
}

static void S_WSellEnter()
{
	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_WITCH);
		// stextsel = STORE_WITCH_SELL;
	} else {
		S_SSell();
	}
}

/**
 * @brief Recharges an item in the player's inventory or body in the witch.
 */
static void WitchRechargeItem()
{
	int i, idx;

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);

	i = storehidx[idx];
	if (i < 0) {
		i = INVITEM_BODY_FIRST - (i + 1);
	} else {
		i += INVITEM_INV_FIRST;
	}
	SendStoreCmd1(i, STORE_WRECHARGE, storeitem._iIvalue);
}

static void S_WRechargeEnter()
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_WITCH);
		// stextsel = STORE_WITCH_RECHARGE;
	} else {
		stextshold = STORE_WRECHARGE;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
		copy_pod(storeitem, storehold[idx]);
		if (myplr._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_BoyEnter()
{
	if (boyitem._itype != ITYPE_NONE) {
		if (stextsel == STORE_PEGBOY_QUERY) {
			stextshold = STORE_PEGBOY;
			stextlhold = STORE_PEGBOY_QUERY;
			stextvhold = stextsidx;
			if (boyitem._iIdentified) {
				StartStore(STORE_PBUY);
			} else if (myplr._pGold < STORE_PEGBOY_PRICE) {
				StartStore(STORE_NOMONEY);
			} else {
				SendStoreCmd1(0, STORE_PEGBOY, STORE_PEGBOY_PRICE);
				S_StartWait();
			}
			return;
		}
		if (stextsel != STORE_PEGBOY_GOSSIP1) {
			stextflag = STORE_NONE;
			return;
		}
	} else {
		if (stextsel != STORE_PEGBOY_GOSSIP2) {
			stextflag = STORE_NONE;
			return;
		}
	}
	stextlhold = stextsel;
	talker = TOWN_PEGBOY;
	stextshold = STORE_PEGBOY;
	StartStore(STORE_GOSSIP);
}

static void BoyBuyItem()
{
	boyitem._itype = ITYPE_NONE;

	SendStoreCmd2(STORE_PBUY);
}

/**
 * @brief Purchases an item from the healer.
 */
static void HealerBuyItem()
{
	int idx;
	bool infinite;

	idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) >> 2);
	infinite = idx < (IsMultiGame ? 3 : 2);
	if (infinite)
		storeitem._iSeed = NextRndSeed();

	SendStoreCmd2(STORE_HBUY);

	if (infinite)
		return;

	do {
		copy_pod(healitem[idx], healitem[idx + 1]);
		idx++;
	} while (healitem[idx]._itype != ITYPE_NONE);
}

static void S_BBuyEnter()
{
	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_PEGBOY);
		// stextsel = STORE_PEGBOY_QUERY;
	} else {
		stextshold = STORE_PBUY;
		stextvhold = stextsidx;
		stextlhold = STORE_PEGBOY_BUY;
		StoreStartBuy(&boyitem, boyitem._iIvalue);
	}
}

static void StoryIdItem()
{
	int idx;

	idx = storehidx[((stextlhold - STORE_LIST_FIRST) >> 2) + stextvhold];
	if (idx < 0)
		idx = INVITEM_BODY_FIRST - (idx + 1);
	else
		idx += INVITEM_INV_FIRST;
	storeitem._iIdentified = TRUE;
	SendStoreCmd1(idx, STORE_SIDENTIFY, STORE_ID_PRICE);
}

static void S_ConfirmEnter()
{
	int lastshold = stextshold;

	if (stextsel == STORE_CONFIRM_YES) {
		switch (lastshold) {
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
		case STORE_PBUY:
			BoyBuyItem();
			//lastshold = STORE_PEGBOY;
			break;
		case STORE_HBUY:
			HealerBuyItem();
			break;
		case STORE_SIDENTIFY:
			StoryIdItem();
			//lastshold = STORE_IDSHOW;
			break;
		case STORE_SPBUY:
			SmithBuyPItem();
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		//lastshold = STORE_WAIT;
		S_StartWait();
		return;
	}
	STextESC();
}

static void S_HealerEnter()
{
	switch (stextsel) {
	case STORE_HEALER_GOSSIP:
		stextlhold = STORE_HEALER_GOSSIP;
		talker = TOWN_HEALER;
		stextshold = STORE_HEALER;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_HEALER_HEAL:
		if (myplr._pHitPoints != myplr._pMaxHP) {
			PlrFillHp(mypnum);
			PlaySFX(IS_CAST8);
		}
		break;
	case STORE_HEALER_BUY:
		StartStore(STORE_HBUY);
		break;
	case STORE_HEALER_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void S_HBuyEnter()
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_HEALER);
		// stextsel = STORE_HEALER_BUY;
	} else {
		stextlhold = stextsel;
		stextvhold = stextsidx;
		stextshold = STORE_HBUY;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
		StoreStartBuy(&healitem[idx], healitem[idx]._iIvalue);
	}
}

static void S_StoryEnter()
{
	switch (stextsel) {
	case STORE_STORY_GOSSIP:
		stextlhold = STORE_STORY_GOSSIP;
		talker = TOWN_STORY;
		stextshold = STORE_STORY;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_STORY_IDENTIFY:
		StartStore(STORE_SIDENTIFY);
		break;
	case STORE_STORY_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void S_SIDEnter()
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
		// StartStore(STORE_STORY);
		// stextsel = STORE_STORY_IDENTIFY;
	} else {
		stextshold = STORE_SIDENTIFY;
		stextlhold = stextsel;
		stextvhold = stextsidx;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) >> 2);
		copy_pod(storeitem, storehold[idx]);
		if (myplr._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_TalkEnter()
{
	int i, tq, sn, la;

	if (stextsel == 22) {
		STextESC();
		// StartStore(stextshold);
		// stextsel = stextlhold;
		return;
	}

	sn = 0;
	for (i = 0; i < NUM_QUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && Qtalklist[talker][i] != TEXT_NONE && quests[i]._qlog)
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
		assert(monsters[MAX_MINIONS + talker]._mType == talker);
		SetRndSeed(monsters[MAX_MINIONS + talker]._mRndSeed); // TNR_SEED
		tq = RandRangeLow(GossipList[talker][0], GossipList[talker][1]);
		StartQTextMsg(tq);
		return;
	}

	for (i = 0; i < NUM_QUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && Qtalklist[talker][i] != TEXT_NONE && quests[i]._qlog) {
			if (sn == stextsel) {
				StartQTextMsg(Qtalklist[talker][i]);
			}
			sn += la;
		}
	}
}

static void S_TavernEnter()
{
	switch (stextsel) {
	case STORE_TAVERN_GOSSIP:
		stextlhold = STORE_TAVERN_GOSSIP;
		talker = TOWN_TAVERN;
		stextshold = STORE_TAVERN;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_TAVERN_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void S_BarmaidEnter()
{
	switch (stextsel) {
	case STORE_BARMAID_GOSSIP:
		stextlhold = STORE_BARMAID_GOSSIP;
		talker = TOWN_BARMAID;
		stextshold = STORE_BARMAID;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_BARMAID_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void S_DrunkEnter()
{
	switch (stextsel) {
	case STORE_DRUNK_GOSSIP:
		stextlhold = STORE_DRUNK_GOSSIP;
		talker = TOWN_DRUNK;
		stextshold = STORE_DRUNK;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_DRUNK_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void S_PriestEnter()
{
	switch (stextsel) {
	/*case STORE_PRIEST_GOSSIP:
		stextlhold = STORE_PRIEST_GOSSIP;
		talker = TOWN_PRIEST;
		stextshold = STORE_PRIEST;
		StartStore(STORE_GOSSIP);
		break;*/
	case STORE_PRIEST_ERRAND:
		stextlhold = STORE_PRIEST_ERRAND;
		talker = TOWN_PRIEST;
		stextshold = STORE_PRIEST;
		StartStore(STORE_ERRAND);
		break;
	case STORE_PRIEST_EXIT:
		stextflag = STORE_NONE;
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static void S_ErrandEnter()
{
	switch (stextsel) {
	case STORE_ERRAND_YES:
		NetSendCmdCreateLvl(GetRndSeed(), myplr._pLevel, DTYPE_TOWN);
		stextflag = STORE_NONE;
		break;
	case STORE_ERRAND_NO:
		STextESC();
		// StartStore(STORE_PRIEST);
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

void STextEnter()
{
	assert(!gbQtextflag);
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
		STextESC();
		// StartStore(stextshold);
		// stextsel = stextlhold;
		// stextsidx = stextvhold;
		break;
	case STORE_CONFIRM:
		S_ConfirmEnter();
		break;
	case STORE_PEGBOY:
		S_BoyEnter();
		break;
	case STORE_PBUY:
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
		STextESC();
		// StartStore(STORE_SIDENTIFY);
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
	case STORE_PRIEST:
		S_PriestEnter();
		break;
	case STORE_ERRAND:
		S_ErrandEnter();
		break;
	case STORE_WAIT:
		return;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	PlaySFX(IS_TITLSLCT);
}

void TryStoreBtnClick()
{
	int y, ly;

	assert(!gbQtextflag);
	if (stextsel != -1 && stextflag != STORE_WAIT) {
		if (gbWidePanel) {
			if (MousePos.x < LTPANEL_X - SCREEN_X || MousePos.x > LTPANEL_X + LTPANEL_WIDTH - SCREEN_X)
				return;
		} else {
			if (MousePos.x < STORE_PNL_X - SCREEN_X || MousePos.x > STORE_PNL_X + STPANEL_WIDTH - SCREEN_X)
				return;
		}
		y = (MousePos.y - (LTPANEL_Y - SCREEN_Y + 8)) / 12;
		//assert(LTPANEL_X + LTPANEL_WIDTH == STORE_PNL_X + STPANEL_WIDTH);
		//if (MousePos.x >= STORE_PNL_X + STPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2) - SCREEN_X && gbHasScroll) {
		if (MousePos.x >= LTPANEL_X + LTPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2) - SCREEN_X && gbHasScroll) {
			assert(gbWidePanel);
			if (stextsmax != 0 && y >= STORE_SCROLL_UP && y <= STORE_SCROLL_DOWN) {
				if (y == STORE_SCROLL_DOWN) {
					// down arrow
					stextscrldbtn--;
					if (stextscrldbtn < 0) {
						stextscrldbtn = 2;
						STextDown();
					}
				} else if (y == STORE_SCROLL_UP) {
					// up arrow
					stextscrlubtn--;
					if (stextscrlubtn < 0) {
						stextscrlubtn = 2;
						STextUp();
					}
				} else {
					// Scroll up or down based on thumb position.
					y -= STORE_SCROLL_UP;
					const int scrollmax = (STORE_SCROLL_DOWN - STORE_SCROLL_UP - 2);
					y--;
					ly = stextsidx * scrollmax / stextsmax;
					if (ly != y) {
						if (ly > y) {
							STextPageUp();
						} else {
							STextPageDown();
						}
					}
				}
			}
		} else if (y >= STORE_LIST_FIRST && y < STORE_LINES) {
			static_assert(STORE_BACK <= 22, "STORE_BACK does not fit to TryStoreBtnClick.");
			// add some freedom to the back button since it has an offset
			if (y >= 22)
				y = 22;
			// allow clicking on multi-line items
			else if (gbHasScroll /*&& y < 21*/ && !stextlines[y]._ssel) {
				y--;
				if (!stextlines[y]._ssel) {
					y--;
				}
			}
			//if (stextlines[y]._ssel || (gbHasScroll && y == STORE_BACK)) {
			if (stextlines[y]._ssel) {
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
