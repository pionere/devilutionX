/**
 * @file stores.cpp
 *
 * Implementation of functionality for stores and towner dialogs.
 */
#include "all.h"
#include "engine/render/cel_render.h"
#include "engine/render/text_render.h"
#include "plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

DISABLE_SPEED_OPTIMIZATION

// #define DEBUG_STORES
#ifdef DEBUG_STORES
#define DEBUG_ASSERT(x) assert(x)
#else
#define DEBUG_ASSERT(x) ((void)0)
#endif

// required size of the store to accomodate the relevant items
#define maxv(a, b) ((a) > (b) ? (a) : (b)) 
constexpr int STORAGE_LIMIT = maxv(maxv(maxv(maxv(NUM_INV_GRID_ELEM + maxv(MAXBELTITEMS, (int)NUM_INVLOC), SMITH_ITEMS), SMITH_PREMIUM_ITEMS), WITCH_ITEMS), HEALER_ITEMS);

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

#define STORE_TAVERN_GOSSIP 12
#define STORE_TAVERN_EXIT   18

#define STORE_BARMAID_GOSSIP 12
#define STORE_BARMAID_EXIT   18

#define STORE_DRUNK_GOSSIP 12
#define STORE_DRUNK_EXIT   18

//#define STORE_PRIEST_GOSSIP 12
#define STORE_PRIEST_ERRAND 12
#define STORE_PRIEST_EXIT   18

#define STORE_ERRAND_YES    18
#define STORE_ERRAND_NO     20

// service prices
#define STORE_ID_PRICE     100
#define STORE_PEGBOY_PRICE 50

#define STORE_ITEM_LINES   ((3 * INV_SLOT_SIZE_PX + 11) / 12)
#define STORE_LINE_ITEMS   8
#define STORE_PAGE_ITEMS   (STORE_LINE_ITEMS * 2)

// item positions
#define STORE_CONFIRM_ITEM (STORE_LIST_FIRST - 1 + STORE_ITEM_LINES)
#define STORE_STORY_ITEM   STORE_CONFIRM_ITEM
#define STORE_PEGBOY_ITEM  STORE_CONFIRM_ITEM

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
// static bool gbRenderGold;
/** Does the current panel have a scrollbar */
static bool gbHasScroll;
/** The index of the first visible item in the store. */
static int stextsidx;
/** The line number of the last visible item in the store */
static int stextdown;
/** Remember stextflag while displaying a dialog */
static int stextshold;
/** Text lines */
static STextStruct stextlines[STORE_LINES];
/** Currently selected text line from stextlines */
static int stextsel;
/** Currently selected item index from stextlines */
static int stextselx;
/** Remember stextsel while displaying a dialog */
static int stextlhold;
/** Remember stextselx while displaying a dialog */
static int stextxhold;
/** The maximum possible value of stextsidx */
static int stextsmax;
/** Copies of the items as presented in the store */
static ItemStruct storehold[STORAGE_LIMIT];
/** Map of items being presented in the store */
static int8_t storehidx[STORAGE_LIMIT];
/** The number of valid entries in storehidx/storehold */
static int storenumh;
/** Remember stextsidx while displaying a dialog */
static int stextvhold;
/** Count down for the push state of the scroll up button */
static int8_t stextscrlubtn;
/** Count down for the push state of the scroll down button */
static int8_t stextscrldbtn;

/** Maps from towner IDs to 'Talk to [NPC]'. */
static const char* const talkname[] = {
	// clang-format off
/*TOWN_SMITH*/  "Talk to Griswold",
/*TOWN_HEALER*/ "Talk to Pepin",
/*TOWN_TAVERN*/ "Talk to Ogden",
/*TOWN_STORY*/  "Talk to Cain",
/*TOWN_DRUNK*/  "Talk to Farnham",
/*TOWN_WITCH*/  "Talk to Adria",
/*TOWN_BARMAID*/"Talk to Gillian",
/*TOWN_PEGBOY*/ "Talk to Wirt"
	// clang-format on
};

static void ClearSText(int s, int e)
{
	// int i;

	static_assert(COL_WHITE == 0, "ClearSText skips color initialization by expecting COL_WHITE to be zero.");
	static_assert(CURSOR_NONE == 0, "ClearSText skips cursor initialization by expecting CURSOR_NONE to be zero.");
	memset(&stextlines[s], 0, (size_t)&stextlines[e] - (size_t)&stextlines[s]);
	/*for (i = s; i < e; i++) {
		stextlines[i]._sx = 0;
		stextlines[i]._syoff = 0;
		stextlines[i]._sstr[0] = '\0';
		stextlines[i]._sjust = false;
		stextlines[i]._sclr = COL_WHITE;
		// stextlines[i]._sline = false;
		stextlines[i]._ssel = false;
		stextlines[i]._sval = 0;
	}*/
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

	// ClearSText(0, STORE_LINES);
	stextflag = STORE_NONE;
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
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

static void PrintSString(int px, int py, int x, int y, bool cjustflag, const char* str, BYTE col, int val)
{
	int sx, sy, tx;
	int width, limit;

	sx = px + STORE_PNL_X_OFFSET + x;
	sy = py + STORE_PNL_Y_OFFSET + STORE_LINE_HEIGHT + y * STORE_LINE_HEIGHT + stextlines[y]._syoff;
	limit = gbWidePanel ? LTPANEL_WIDTH - STORE_PNL_X_OFFSET * 2 : STPANEL_WIDTH - STORE_PNL_X_OFFSET * 2;
	limit -= x;
	if (cjustflag) {
		// assert(x == 0); -- otherwise limit -= x; ?
		width = GetSmallStringWidth(str);
		// if (width < limit) {
			sx += (limit - width) >> 1;
		// }
	}
	tx = sx;
	sx = PrintLimitedString(sx, sy, str, limit, col, FONT_KERN_SMALL);
	if (stextsel == y) {
		DEBUG_ASSERT(cjustflag || !gbHasScroll);
		DEBUG_ASSERT(cjustflag || gbWidePanel);
		DrawSmallPentSpn(tx - FOCUS_SMALL, cjustflag ? sx + 6 : (px + LTPANEL_WIDTH - 20/*(x + (STORE_PNL_X_OFFSET + gbHasScroll ? SMALL_SCROLL_WIDTH : 0))*/), sy + 1);
	}
	if (val > 0) {
		DEBUG_ASSERT(!cjustflag && gbWidePanel);
		char valstr[32];
		snprintf(valstr, sizeof(valstr), "%d", val);
		PrintString(AFF_SMALL | AFF_RIGHT | (col << AFF_COLOR_SHL), valstr, px, sy - SMALL_FONT_HEIGHT, LTPANEL_WIDTH - (2 * SMALL_SCROLL_WIDTH), 0);
	}
}

static void DrawSSlider(int px, int py)
{
	const int y1 = STORE_SCROLL_UP, y2 = STORE_SCROLL_DOWN;
	int x, i, yd1, yd2, yd3;

	//x = px + (gbWidePanel ? LTPANEL_WIDTH : STPANEL_WIDTH) - (SMALL_SCROLL_WIDTH + 2);
	DEBUG_ASSERT(gbWidePanel);
	x = px + LTPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2);
	yd1 = y1 * STORE_LINE_HEIGHT + py + STORE_PNL_Y_OFFSET + SMALL_SCROLL_HEIGHT; // top position of the scrollbar
	yd2 = y2 * STORE_LINE_HEIGHT + py + STORE_PNL_Y_OFFSET + SMALL_SCROLL_HEIGHT; // bottom position of the scrollbar
	yd3 = (y2 - y1) * STORE_LINE_HEIGHT - 2 * SMALL_SCROLL_HEIGHT;                // height of the scrollbar
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

/*static void AddSLine(int y)
{
	stextlines[y]._sline = true;
}*/

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
	// ss->_syoff = 0;
	SStrCopy(ss->_sstr, str, sizeof(ss->_sstr));
	ss->_sjust = j;
	ss->_sclr = clr;
	// ss->_sline = false;
	ss->_ssel = sel;
}

static void AddSItem(int x, int y, int idx, const ItemStruct* is, bool selectable)
{
	STextStruct* ss;

	ss = &stextlines[y];
	ss->_sx = x;
	ss->_sitemlist = true;
	ss->_siItems[idx] = is;
	// ss->_sclr = 0;
	ss->_ssel = selectable;
}

static BYTE StoreItemColor(const ItemStruct* is)
{
	if (!is->_iStatFlag)
		return COL_RED;
	return ItemColor(is);
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
	if (is->_iMagical == ITEM_QUALITY_MAGIC) {
		for (unsigned i = 0; i < is->_iNumAffixes; i++) {
			PrintItemPower(i, is);
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

static void AddStoreHoldItem(const ItemStruct* is, int i, int value)
{
	ItemStruct* itm;

	itm = &storehold[storenumh];
	copy_pod(*itm, *is);

	ItemStatOk(mypnum, itm);

	itm->_iIvalue = itm->_ivalue = value;

	storehidx[storenumh] = i;
	storenumh++;
}

static void AddStoreHoldItemBuy(ItemStruct* is, int i)
{
	if (is->_iMagical != ITEM_QUALITY_NORMAL)
		is->_iIdentified = TRUE;

	AddStoreHoldItem(is, i, is->_iIvalue);
}

static void AddStoreItem(const ItemStruct* is, unsigned l)
{
	int line = STORE_LIST_FIRST + (l / STORE_LINE_ITEMS) * STORE_ITEM_LINES;
	stextdown = line;
	AddSItem(60, line, l % STORE_LINE_ITEMS, is, TRUE);
	if (stextsel == line && stextselx == (int)(l % STORE_LINE_ITEMS)) {
		PrintStoreItem(is, STORE_LIST_FOOTER - 3, false);
		AddSTextVal(STORE_LIST_FOOTER - 3, is->_iIvalue);
	}
}

static void AddStoreFrame(const char* title)
{
	AddSText(10, 1, false, title, COL_GOLD, false);
	// AddSLine(3);
	// AddSLine(21);
	AddSText(0, STORE_BACK, true, "Back", COL_WHITE, true);
	OffsetSTextY(STORE_BACK, 6);
}

static void S_StartSmith()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 1, true, "Welcome to the", COL_GOLD, false);
	AddSText(0, 3, true, "Blacksmith's shop", COL_GOLD, false);
	AddSText(0, 7, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_SMITH_GOSSIP, true, talkname[TOWN_SMITH], COL_BLUE, true);
	AddSText(0, STORE_SMITH_BUY, true, "Buy basic items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_SPBUY, true, "Buy premium items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_SELL, true, "Sell items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_REPAIR, true, "Repair items", COL_WHITE, true);
	AddSText(0, STORE_SMITH_EXIT, true, "Leave the shop", COL_WHITE, true);
	// AddSLine(5);
}

static void S_ScrollHold()
{
	ItemStruct* is;
	unsigned l;

	// ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);

	is = &storehold[stextsidx];
	for (l = 0; l < STORE_PAGE_ITEMS; l++) {
		if (is->_itype != ITYPE_NONE) {
			AddStoreItem(is, l);
			is++;
		}
	}
	stextsmax = storenumh - STORE_PAGE_ITEMS;
	if (stextsmax <= 0)
		stextsmax = 0;
	else
		stextsmax = ((stextsmax + STORE_LINE_ITEMS + 1) / STORE_LINE_ITEMS) * STORE_LINE_ITEMS;
}

static void S_StartBuy(ItemStruct* items, int n, const char* prefix)
{
	int i;
	storenumh = 0;
	for (i = 0; i < STORAGE_LIMIT; i++)
		storehold[i]._itype = ITYPE_NONE;
	for (i = 0; i < n; i++) {
		if (items[i]._itype != ITYPE_NONE)
			AddStoreHoldItemBuy(&items[i], i);
	}

	gbWidePanel = true;
	// gbRenderGold = true;
	gbHasScroll = storenumh != 0;
	char text[64];
	char *msg;
	if (storenumh == 0) {
		msg = "I have no %sitem for sale.";
	} else {
		// stextsidx = 0;
		S_ScrollHold();

		msg = "I have these %sitems for sale:";
	}
	snprintf(text, lengthof(text), msg, prefix);
	AddStoreFrame(text);
}

static void S_StartSBuy()
{
	S_StartBuy(&smithitem[0], SMITH_ITEMS, "basic ");
}

static void S_StartSPBuy()
{
	S_StartBuy(&premiumitems[0], SMITH_PREMIUM_ITEMS, "premium ");
}

static void AddStoreSell(const ItemStruct* is, int i)
{
	int value;

	value = (is->_iMagical != ITEM_QUALITY_NORMAL && is->_iIdentified) ? is->_iIvalue : is->_ivalue;
	value >>= 3;
	if (value == 0)
		value = 1;

	AddStoreHoldItem(is, i, value);
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
	// gbRenderGold = true;
	gbHasScroll = storenumh != 0;
	if (storenumh == 0) {
		msg = "You have nothing I want.";
	} else {
		// stextsidx = 0;
		S_ScrollHold();

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
	int value;

	value = std::max(is->_ivalue, is->_iIvalue / 3);
	value = value * (is->_iMaxDur - is->_iDurability) / (is->_iMaxDur * 2);
	if (value == 0)
		value = 1;

	AddStoreHoldItem(is, i, value);
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
	// gbRenderGold = true;
	gbHasScroll = storenumh != 0;
	if (storenumh == 0) {
		msg = "You have nothing to repair.";
	} else {
		// stextsidx = 0;
		S_ScrollHold();

		msg = "Repair which item?";
	}
	AddStoreFrame(msg);
}

static void S_StartWitch()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;

	AddSText(0, 2, true, "Witch's shack", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_WITCH_GOSSIP, true, talkname[TOWN_WITCH], COL_BLUE, true);
	AddSText(0, STORE_WITCH_BUY, true, "Buy items", COL_WHITE, true);
	AddSText(0, STORE_WITCH_SELL, true, "Sell items", COL_WHITE, true);
	AddSText(0, STORE_WITCH_RECHARGE, true, "Recharge staves", COL_WHITE, true);
	AddSText(0, STORE_WITCH_EXIT, true, "Leave the shack", COL_WHITE, true);
	// AddSLine(5);
}

static void S_StartWBuy()
{
	S_StartBuy(&witchitem[0], WITCH_ITEMS, "");
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
	// gbRenderGold = true;
	gbHasScroll = storenumh != 0;
	if (storenumh == 0) {
		msg = "You have nothing I want.";
	} else {
		// stextsidx = 0;
		S_ScrollHold();
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
	int value;

	value = is->_ivalue + spelldata[is->_iSpell].sStaffCost;
	value = value * (is->_iMaxCharges - is->_iCharges) / is->_iMaxCharges;

	AddStoreHoldItem(is, i, value);
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
	// gbRenderGold = true;
	gbHasScroll = storenumh != 0;
	if (storenumh == 0) {
		msg = "You have nothing to recharge.";
	} else {
		// stextsidx = 0;
		S_ScrollHold();

		msg = "Recharge which item?";
	}
	AddStoreFrame(msg);
}

static void S_StartNoMoney()
{
	// StartStore(stextshold);
	// ClearSText(STORE_LIST_FIRST, STORE_LINES);
	gbWidePanel = true;
	// gbRenderGold = true;
	// gbHasScroll = false;
	AddSText(0, 14, true, "You do not have enough gold", COL_WHITE, true);

	// AddSLine(3);
	// AddSLine(21);
}

static void S_StartNoRoom()
{
	// StartStore(stextshold);
	// ClearSText(STORE_LIST_FIRST, STORE_LINES);
	gbWidePanel = true;
	// gbRenderGold = true;
	// gbHasScroll = false;
	AddSText(0, 14, true, "You do not have enough room in inventory", COL_WHITE, true);

	// AddSLine(3);
	// AddSLine(21);
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
	gbWidePanel = true;
	// gbRenderGold = true;
	// gbHasScroll = false;
	AddSItem(260, STORE_LIST_FIRST, 0, &storeitem, FALSE);
	PrintStoreItem(&storeitem, STORE_CONFIRM_ITEM, false);
	AddSTextVal(STORE_CONFIRM_ITEM, storeitem._iIvalue);
	// AddSLine(3);
	// AddSLine(21);

	const char* msg;
	switch (stextshold) {
	case STORE_PBUY:
		msg = "Do we have a deal?";
		break;
	case STORE_SIDENTIFY:
		msg = "Are you sure you want to identify this item?";
		break;
	case STORE_HBUY:
	case STORE_SPBUY:
	case STORE_WBUY:
	case STORE_SBUY:
		msg = "Are you sure you want to buy this item?";
		break;
	case STORE_WRECHARGE:
		msg = "Are you sure you want to recharge this item?";
		break;
	case STORE_SSELL:
	case STORE_WSELL:
		msg = "Are you sure you want to sell this item?";
		break;
	case STORE_SREPAIR:
		msg = "Are you sure you want to repair this item?";
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}
	AddSText(0, 15, true, msg, COL_WHITE, false);
	AddSText(0, STORE_CONFIRM_YES, true, "Yes", COL_WHITE, true);
	AddSText(0, STORE_CONFIRM_NO, true, "No", COL_WHITE, true);
}

static void S_StartBoy()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, "Wirt the Peg-legged boy", COL_GOLD, false);
	// AddSLine(5);
	if (boyitem._itype != ITYPE_NONE) {
		AddSText(0, STORE_PEGBOY_GOSSIP1, true, talkname[TOWN_PEGBOY], COL_BLUE, true);
		AddSText(0, 12, true, "I have something for sale,", COL_GOLD, false);
		if (!boyitem._iIdentified) {
			static_assert(STORE_PEGBOY_PRICE == 50, "Hardcoded boy price is 50.");
			AddSText(0, 14, true, "but it will cost 50 gold", COL_GOLD, false);
			AddSText(0, 16, true, "just to take a look. ", COL_GOLD, false);
		}
		AddSText(0, STORE_PEGBOY_QUERY, true, "What have you got?", COL_WHITE, true);
		AddSText(0, STORE_PEGBOY_EXIT1, true, "Say goodbye", COL_WHITE, true);
	} else {
		AddSText(0, STORE_PEGBOY_GOSSIP2, true, talkname[TOWN_PEGBOY], COL_BLUE, true);
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

	S_StartBuy(&boyitem, 1, "");
}

static void S_StartHealer()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 1, true, "Welcome to the", COL_GOLD, false);
	AddSText(0, 3, true, "Healer's home", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_HEALER_GOSSIP, true, talkname[TOWN_HEALER], COL_BLUE, true);
	AddSText(0, STORE_HEALER_HEAL, true, "Receive healing", COL_WHITE, true);
	AddSText(0, STORE_HEALER_BUY, true, "Buy items", COL_WHITE, true);
	AddSText(0, STORE_HEALER_EXIT, true, "Leave Healer's home", COL_WHITE, true);
	// AddSLine(5);
}

static void S_StartHBuy()
{
	S_StartBuy(&healitem[0], HEALER_ITEMS, "");
}

static void S_StartStory()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, "The Town Elder", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_STORY_GOSSIP, true, talkname[TOWN_STORY], COL_BLUE, true);
	AddSText(0, STORE_STORY_IDENTIFY, true, "Identify an item", COL_WHITE, true);
	AddSText(0, STORE_STORY_EXIT, true, "Say goodbye", COL_WHITE, true);
	// AddSLine(5);
}

static bool IdItemOk(const ItemStruct* is)
{
	return is->_itype != ITYPE_NONE && is->_itype != ITYPE_PLACEHOLDER
		&& is->_iMagical != ITEM_QUALITY_NORMAL
		&& !is->_iIdentified;
}

static void AddStoreHoldId(const ItemStruct* is, int i)
{
	AddStoreHoldItem(is, i, STORE_ID_PRICE);
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
	// gbRenderGold = true;
	gbHasScroll = storenumh != 0;
	if (storenumh == 0) {
		msg = "You have nothing to identify.";
	} else {
		// stextsidx = 0;
		S_ScrollHold();

		msg = "Identify which item?";
	}
	AddStoreFrame(msg);
}

static void S_StartIdShow()
{
	//assert(stextshold == STORE_SIDENTIFY);
	//StartStore(STORE_SIDENTIFY);
	//ClearSText(STORE_LIST_FIRST, STORE_LINES);

	gbWidePanel = true;
	// gbRenderGold = true;
	// gbHasScroll = false;

	AddSItem(260, STORE_LIST_FIRST, 0, &storeitem, FALSE);
	PrintStoreItem(&storeitem, STORE_STORY_ITEM, false);

	AddStoreFrame("This item is:");
}

static void S_StartTalk()
{
	int sn, la;
	int tqs[NUM_QUESTS];
	unsigned i, qn;
	static_assert(STORE_LINES >= 22, "S_StartTalk does not fit to the store.");
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, talkname[talker], COL_GOLD, false);
	// AddSLine(5);
	qn = 0;
	for (i = 0; i < NUM_QUESTS; i++) {
		if (quests[i]._qactive == QUEST_ACTIVE && Qtalklist[talker][i] != TEXT_NONE && quests[i]._qlog) {
			tqs[qn] = i;
			qn++;
		}
	}

	if (qn > 7) {
		if (qn > 14)
			qn = 14;
		sn = 13 - ((qn + 1) >> 1);
		la = 1;
	} else {
		sn = 13 - qn;
		la = 2;
	}

	AddSText(0, sn, true, "Gossip", COL_BLUE, true);
	AddSTextVal(sn, -(int)NUM_QUESTS);
	sn += 2;
	for (i = 0; i < qn; i++) {
		AddSText(0, sn, true, questlist[tqs[i]]._qlstr, COL_WHITE, true);
		AddSTextVal(sn, -tqs[i]);
		sn += la;
	}
	AddSText(0, 22, true, "Back", COL_WHITE, true);
}

static void S_StartTavern()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 1, true, "Welcome to the", COL_GOLD, false);
	AddSText(0, 3, true, "Rising Sun", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_TAVERN_GOSSIP, true, talkname[TOWN_TAVERN], COL_BLUE, true);
	AddSText(0, STORE_TAVERN_EXIT, true, "Leave the tavern", COL_WHITE, true);
	// AddSLine(5);
}

static void S_StartBarMaid()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, "Gillian", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_BARMAID_GOSSIP, true, talkname[TOWN_BARMAID], COL_BLUE, true);
	AddSText(0, STORE_BARMAID_EXIT, true, "Say goodbye", COL_WHITE, true);
	// AddSLine(5);
}

static void S_StartDrunk()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, "Farnham the Drunk", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_DRUNK_GOSSIP, true, talkname[TOWN_DRUNK], COL_BLUE, true);
	AddSText(0, STORE_DRUNK_EXIT, true, "Say Goodbye", COL_WHITE, true);
	// AddSLine(5);
}

static void S_StartPriest()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, "Tremain the Priest", COL_GOLD, false);
	AddSText(0, 9, true, "Would you like to:", COL_GOLD, false);
	AddSText(0, STORE_PRIEST_ERRAND, true, "Run errand", COL_WHITE, true);
	AddSText(0, STORE_PRIEST_EXIT, true, "Say Goodbye", COL_WHITE, true);
	// AddSLine(5);
}

static void S_StartErrand()
{
	// gbWidePanel = false;
	// gbRenderGold = false;
	// gbHasScroll = false;
	AddSText(0, 2, true, "Tremain the Priest", COL_GOLD, false);
	// AddSText(0, 12, true, "Would you like to", COL_WHITE, false);
	AddSText(0, 14, true, "Go on an errand?", COL_WHITE, false);
	AddSText(0, STORE_ERRAND_YES, true, "Yes", COL_WHITE, true);
	AddSText(0, STORE_ERRAND_NO, true, "No", COL_WHITE, true);
	// AddSLine(5);
}

void StartStore(int s)
{
	int i;

	ClearSText(0, STORE_LINES);
	ReleaseStoreBtn();
	stextflag = s;
	stextsidx = 0;
	stextsmax = 0;
	gbWidePanel = false; // 11 vs 14
	// gbRenderGold = false;
	gbHasScroll = false;
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

	DEBUG_ASSERT(i != STORE_LINES);
	stextsel = /*i == STORE_LINES ? -1 :*/ i;
	stextselx = 0;
}

static void StoreUpdateSelection()
{
	if (stextsidx > stextsmax) {
		stextsidx = stextsmax;
	}
	DEBUG_ASSERT(stextsel != -1);
	while (/*stextsel != -1 && */ !stextlines[stextsel]._ssel) {
		stextsel--;
		DEBUG_ASSERT(stextsel != -1);
	}
	if (/*stextsel != -1 && */ stextlines[stextsel]._sitemlist) {
		while (stextselx >= 0 && stextlines[stextsel]._siItems[stextselx] == NULL) {
			stextselx--;
		}
	}
}

/*void DrawStoreLineX(int sx, int sy, int dx, int dy, int length)
{
	int sxy, dxy, width;

	width = BUFFER_WIDTH;
	sxy = sx + 0 + width * (sy + 0);
	dxy = dx + 0 + width * dy;

	/// ASSERT: assert(gpBuffer != NULL);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[sxy];
	dst = &gpBuffer[dxy];

	for (i = 0; i < BOXBORDER_WIDTH; i++, src += width, dst += width)
		memcpy(dst, src, length);
}
void DrawStoreLineY(int sx, int sy, int dx, int dy, int height)
{
	int sxy, dxy, width;

	width = BUFFER_WIDTH;
	sxy = sx + 0 + width * (sy + 0);
	dxy = dx + 0 + width * dy;

	/// ASSERT: assert(gpBuffer != NULL);

	int i;
	BYTE *src, *dst;

	src = &gpBuffer[sxy];
	dst = &gpBuffer[dxy];

	for (i = 0; i < height; i++, src += width, dst += width)
		memcpy(dst, src, BOXBORDER_WIDTH);
}*/
static int current_store_line(int px, int py)
{
	int mx, my, y;

	mx = MousePos.x;
	my = MousePos.y;

	y = (my - (py - SCREEN_Y + STORE_PNL_Y_OFFSET)) / STORE_LINE_HEIGHT;
	if (gbWidePanel) {
		if (mx < px - SCREEN_X || mx > px + LTPANEL_WIDTH - SCREEN_X)
			y = 0;
	} else {
		if (mx < px - SCREEN_X || mx > px + STPANEL_WIDTH - SCREEN_X)
			y = 0;
	}

	if (y >= STORE_LIST_FIRST && y < STORE_LINES) {
		static_assert(STORE_BACK <= 22, "STORE_BACK does not fit to current_store_line.");
		// add some freedom to the back button since it has an offset
		if (y >= 22)
			y = 22;
	} else {
		y = 0;
	}
	return y;
}

static int current_store_item(int y, int px)
{
	// assert(stextlines[y]._sitemlist);
	int x = MousePos.x - (px + STORE_PNL_X_OFFSET + 60 - SCREEN_X);
	if (x >= 0) {
		x /= (2 * INV_SLOT_SIZE_PX);
		if (x < lengthof(stextlines[y]._siItems)) {
			if (stextlines[y]._siItems[x] != NULL) {
				// EventPlrMsg("clicked pos %d:%d", x, y);
				return x;
			}
		}
	}
	return -1;
}

static POS32 current_store_target(int y, int px)
{
	POS32 result = { -1, -1 };
	// allow clicking on multi-line items
	if (gbHasScroll && y < 21 /*&& !stextlines[y]._ssel*/) {
		y++;
		for (int n = 0; n < STORE_ITEM_LINES; n++) {
			if (stextlines[y]._ssel)
				break;
			y--;
			if (y == 0)
				break;
		}
	}
	//if (stextlines[y]._ssel || (gbHasScroll && y == STORE_BACK)) {
	if (stextlines[y]._ssel) {
		result.y = y;
		if (stextlines[y]._sitemlist) {
			result.x = current_store_item(y, px);
			if (result.x < 0) {
				result.y = -1;
			}
		}
	}
	return result;
}

const ItemStruct* CurrentStoreItem()
{
	const ItemStruct* result = NULL;
	if (gbWidePanel) {
		int px = LTPANEL_X, py = LTPANEL_Y;
		int y = current_store_line(px, py);
		if (y != 0) {
			POS32 target = current_store_target(y, px);
			if (target.x >= 0) {
				result = stextlines[target.y]._siItems[target.x];
			}
		}
	}
	return result;
}

void DrawStore()
{
	STextStruct* sts;
	int i, x, y;

	y = LTPANEL_Y;
	if (gbWidePanel) {
		x = LTPANEL_X;

		DrawColorTextBox(x, y, LTPANEL_WIDTH, TPANEL_HEIGHT, COL_GOLD);
		DrawColorTextBoxSLine(x, y, LTPANEL_WIDTH, 3 * STORE_LINE_HEIGHT + 14);
		DrawColorTextBoxSLine(x, y, LTPANEL_WIDTH, 21 * STORE_LINE_HEIGHT + 14);
	} else {
		x = STORE_PNL_X;

		DrawSTextBox(x, y);
	}

	if (gbHasScroll) {
		// update list entries (TODO: only if stextsmax != 0?)
		ClearSText(STORE_LIST_FIRST, STORE_LIST_FOOTER);
		S_ScrollHold();
		StoreUpdateSelection(); // check maxx
	}

	int csi = current_store_line(x, y);
	for (i = 0; i < STORE_LINES; i++) {
		sts = &stextlines[i];
		// if (sts->_sline)
		//	DrawColorTextBoxSLine(x, y, i * STORE_LINE_HEIGHT + 14, gbWidePanel);
		if (sts->_sstr[0] != '\0')
			PrintSString(x, y, sts->_sx, i, sts->_sjust, sts->_sstr, (csi == i && sts->_ssel) ? COL_GOLD + 1 + 4 : sts->_sclr, sts->_sval);
		else if (sts->_sitemlist) {
			for (int n = 0; n < lengthof(sts->_siItems); n++) {
				const ItemStruct* is = sts->_siItems[n];
				if (is != NULL) {
					int frame = is->_iCurs + CURSOR_FIRSTITEM;
					// int sx = x + STORE_PNL_X_OFFSET + sts->_sx;
					int px = x, py = y + 1;
					int sx = px + STORE_PNL_X_OFFSET + sts->_sx;
					int sy = py + STORE_PNL_Y_OFFSET + STORE_LINE_HEIGHT - 1 + i * STORE_LINE_HEIGHT + sts->_syoff;
					int frame_width = InvItemWidth[frame];
					int frame_height = InvItemHeight[frame];

					sx += n * 2 * INV_SLOT_SIZE_PX;
					sy += (STORE_ITEM_LINES - (1 + 1)) * STORE_LINE_HEIGHT;

					sx += (2 * INV_SLOT_SIZE_PX - frame_width) >> 1;
					sy -= (3 * INV_SLOT_SIZE_PX - frame_height) >> 1;

					if (stextsel == i && stextselx == n) {
						// assert(gbWidePanel);
						// DrawColorTextBoxSLine(px, py, 20 + i * STORE_LINE_HEIGHT, false);
						/*
						// top-left corner
						DrawStoreLineX(px, py, sx + 0 * INV_SLOT_SIZE_PX, sy - 3 * INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX / 2);
						DrawStoreLineY(px, py, sx + 0 * INV_SLOT_SIZE_PX, sy - 3 * INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX / 2);
						// top-right corner
						DrawStoreLineX(px + LTPANEL_WIDTH - INV_SLOT_SIZE_PX / 2, py, sx + 2 * INV_SLOT_SIZE_PX - INV_SLOT_SIZE_PX / 2, sy - 3 * INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX / 2);
						DrawStoreLineY(px + LTPANEL_WIDTH - BOXBORDER_WIDTH, py, sx + 2 * INV_SLOT_SIZE_PX - BOXBORDER_WIDTH, sy - 3 * INV_SLOT_SIZE_PX, INV_SLOT_SIZE_PX / 2);
						// bottom-left corner
						DrawStoreLineX(px, py + TPANEL_HEIGHT - BOXBORDER_WIDTH, sx + 0 * INV_SLOT_SIZE_PX, sy - BOXBORDER_WIDTH, INV_SLOT_SIZE_PX / 2);
						DrawStoreLineY(px, py + TPANEL_HEIGHT - BOXBORDER_WIDTH - INV_SLOT_SIZE_PX / 2, sx + 0 * INV_SLOT_SIZE_PX, sy - INV_SLOT_SIZE_PX / 2 - BOXBORDER_WIDTH, INV_SLOT_SIZE_PX / 2);
						// bottom-right corner
						DrawStoreLineX(px + LTPANEL_WIDTH - INV_SLOT_SIZE_PX / 2, py + TPANEL_HEIGHT - BOXBORDER_WIDTH, sx + 2 * INV_SLOT_SIZE_PX - INV_SLOT_SIZE_PX / 2, sy - BOXBORDER_WIDTH, INV_SLOT_SIZE_PX / 2);
						DrawStoreLineY(px + LTPANEL_WIDTH - BOXBORDER_WIDTH, py + TPANEL_HEIGHT - BOXBORDER_WIDTH - INV_SLOT_SIZE_PX / 2, sx + 2 * INV_SLOT_SIZE_PX - BOXBORDER_WIDTH, sy - INV_SLOT_SIZE_PX / 2 - BOXBORDER_WIDTH, INV_SLOT_SIZE_PX / 2);
						*/
						CelClippedDrawOutline(ICOL_YELLOW, sx, sy, pCursCels, frame, frame_width);
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
						if (sgbControllerActive) {
							SetCursorPos(sx + frame_width / 2 - SCREEN_X, sy - frame_height / 2 - SCREEN_Y);
						}
#endif
					}
					CelClippedDrawLightTbl(sx, sy, pCursCels, frame, frame_width, is->_iStatFlag ? 0 : COLOR_TRN_RED);
				}
			}
		}
	}

	// if (gbRenderGold) {
	if (gbWidePanel) {
		char valstr[32];
		snprintf(valstr, sizeof(valstr), "%d", myplr._pGold);
		const int cursor = (int)CURSOR_FIRSTITEM + ICURS_GOLD_SMALL;
		const int cw = InvItemWidth[cursor];
		PrintString(AFF_SMALL | AFF_RIGHT | (COL_GOLD << AFF_COLOR_SHL), valstr, x, y + STORE_PNL_Y_OFFSET + STORE_LINE_HEIGHT + 1 * STORE_LINE_HEIGHT - SMALL_FONT_HEIGHT, LTPANEL_WIDTH - (STORE_PNL_X_OFFSET + cw + 3), 0);
		CelClippedDrawLightTbl(x + LTPANEL_WIDTH - (STORE_PNL_X_OFFSET + cw), y + STORE_PNL_Y_OFFSET + STORE_LINE_HEIGHT + (InvItemHeight[cursor] + STORE_LINE_HEIGHT) / 2, pCursCels, cursor, cw, 0);
	}
	if (gbHasScroll)
		DrawSSlider(x, y);
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
		// stextflag = stextshold;
		stextsel = stextlhold;
		// stextselx = stextxhold;
		// stextsidx = stextvhold;
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
		// stextflag = stextshold;
		stextsel = stextlhold;
		stextselx = stextxhold;
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
	PlaySfx(IS_TITLEMOV);
	DEBUG_ASSERT(stextsel != -1);

	if (gbHasScroll && stextsel == STORE_LIST_FIRST) {
		if (stextsidx != 0) {
			DEBUG_ASSERT(stextlines[stextsel]._sitemlist);
			stextsidx -= STORE_LINE_ITEMS;
		}
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
	PlaySfx(IS_TITLEMOV);
	DEBUG_ASSERT(stextsel != -1);

	if (gbHasScroll && stextsel == stextdown) {
		if (stextsidx < stextsmax) {
			DEBUG_ASSERT(stextlines[stextsel]._sitemlist);
			stextsidx += STORE_LINE_ITEMS;
		}
		return;
	}

	do {
		stextsel++;
		if (stextsel == STORE_LINES)
			stextsel = 0;
	} while (!stextlines[stextsel]._ssel);
}

void STextRight()
{
	DEBUG_ASSERT(stextsel != -1);
	if (/*stextsel == -1 || */!stextlines[stextsel]._sitemlist) {
		return;
	}
	PlaySfx(IS_TITLEMOV);

	do {
		stextselx++;
		if (stextselx == STORE_LINE_ITEMS)
			stextselx = 0;
	} while (stextlines[stextsel]._siItems[stextselx] == NULL);
}

void STextLeft()
{
	DEBUG_ASSERT(stextsel != -1);
	if (/*stextsel == -1 || */!stextlines[stextsel]._sitemlist) {
		return;
	}
	PlaySfx(IS_TITLEMOV);

	do {
		stextselx--;
		if (stextselx < 0)
			stextselx = STORE_LINE_ITEMS - 1;
	} while (stextlines[stextsel]._siItems[stextselx] == NULL);
}

static void STextPageUp()
{
	DEBUG_ASSERT(stextsel != -1);
	if (gbHasScroll) {
		DEBUG_ASSERT(stextsidx == 0 || stextlines[stextsel]._sitemlist);
		PlaySfx(IS_TITLEMOV);
		stextsidx -= STORE_PAGE_ITEMS;
		if (stextsidx < 0) {
			stextsidx = 0;
			stextsel = STORE_LIST_FIRST;
		}
	}
}

static void STextPageDown()
{
	DEBUG_ASSERT(stextsel != -1);
	if (gbHasScroll) {
		DEBUG_ASSERT(stextsmax == 0 || stextlines[stextsel]._sitemlist);
		PlaySfx(IS_TITLEMOV);
		stextsidx += STORE_PAGE_ITEMS;
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
		DEBUG_ASSERT(stextflag == STORE_SMITH);
		stextlhold = STORE_SMITH_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_SMITH;
		talker = TOWN_SMITH;
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

static void StoreShiftItems(ItemStruct *is)
{
	do {
		copy_pod(is[0], is[1]);
		is++;
	} while (is->_itype != ITYPE_NONE);
}

/**
 * @brief Purchases an item from the smith.
 */
static void SmithBuyItem(int idx)
{
	SendStoreCmd2(STORE_SBUY);

	StoreShiftItems(&smithitem[idx]);
}

static void StoreStartBuy(int mode)
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
	} else {
		DEBUG_ASSERT(stextflag == mode);
		stextlhold = stextsel;
		stextxhold = stextselx;
		stextvhold = stextsidx;
		stextshold = mode;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) / STORE_ITEM_LINES) * STORE_LINE_ITEMS + stextselx;

		const ItemStruct* is = &storehold[idx];
	int price = is->_iIvalue;
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
}

static void S_SBuyEnter()
{
	StoreStartBuy(STORE_SBUY);
}

/**
 * @brief Purchases a premium item from the smith.
 */
static void SmithBuyPItem(int idx)
{
	SendStoreCmd2(STORE_SPBUY);

	idx = storehidx[idx];
	premiumitems[idx]._itype = ITYPE_NONE;
	numpremium--;
	//SpawnPremium(StoresLimitedItemLvl());
}

static void S_SPBuyEnter()
{
	StoreStartBuy(STORE_SPBUY);
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
		DEBUG_ASSERT(price == STORE_ID_PRICE);
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
		DEBUG_ASSERT(price == STORE_PEGBOY_PRICE);
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

	// stextflag = stextshold; -- except for wirt's redirect
	stextsel = stextlhold;
	stextselx = stextxhold;
	stextsidx = stextvhold;
	StoreUpdateSelection();
}

/**
 * @brief Sells an item from the player's inventory or belt.
 */
static void StoreSellItem(int idx)
{
	int i, cost;

	i = storehidx[idx];
	if (i >= 0) {
		i += INVITEM_INV_FIRST;
	} else {
		i = INVITEM_BELT_FIRST - (i + 1);
	}
	cost = storehold[idx]._iIvalue;
	SendStoreCmd1(i, STORE_SSELL, cost);
}

static void S_SSell()
{
	int idx;

	if (stextsel == STORE_BACK) {
		STextESC();
	} else {
	stextlhold = stextsel;
	stextxhold = stextselx;
	stextvhold = stextsidx;
	stextshold = stextflag;

	idx = stextsidx + ((stextsel - STORE_LIST_FIRST) / STORE_ITEM_LINES) * STORE_LINE_ITEMS + stextselx;
	copy_pod(storeitem, storehold[idx]);

	idx = storehidx[idx] >= 0 ? storeitem._iCurs + CURSOR_FIRSTITEM : CURSOR_NONE;
	if (StoreGoldFit(storeitem._iIvalue, idx))
		StartStore(STORE_CONFIRM);
	else
		StartStore(STORE_NOROOM);
	}
}

static void S_SSellEnter()
{
	S_SSell();
}

/**
 * @brief Repairs an item in the player's inventory or body in the smith.
 */
static void SmithRepairItem(int idx)
{
	int i;

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
		DEBUG_ASSERT(stextflag == STORE_SREPAIR);
		stextlhold = stextsel;
		stextxhold = stextselx;
		stextvhold = stextsidx;
		stextshold = STORE_SREPAIR;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) / STORE_ITEM_LINES) * STORE_LINE_ITEMS + stextselx;
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
		DEBUG_ASSERT(stextflag == STORE_WITCH);
		stextlhold = STORE_WITCH_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_WITCH;
		talker = TOWN_WITCH;
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
static void WitchBuyItem(int idx)
{
	if (idx < 3)
		storeitem._iSeed = NextRndSeed();

	SendStoreCmd2(STORE_WBUY);

	if (idx >= 3) {
		StoreShiftItems(&witchitem[idx]);
	}
}

static void S_WBuyEnter()
{
	StoreStartBuy(STORE_WBUY);
}

static void S_WSellEnter()
{
	S_SSell();
}

/**
 * @brief Recharges an item in the player's inventory or body in the witch.
 */
static void WitchRechargeItem(int idx)
{
	int i;

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
		DEBUG_ASSERT(stextflag == STORE_WRECHARGE);
		stextlhold = stextsel;
		stextxhold = stextselx;
		stextvhold = stextsidx;
		stextshold = STORE_WRECHARGE;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) / STORE_ITEM_LINES) * STORE_LINE_ITEMS + stextselx;
		copy_pod(storeitem, storehold[idx]);
		if (myplr._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_BoyEnter()
{
	DEBUG_ASSERT(stextflag == STORE_PEGBOY);
	if (boyitem._itype != ITYPE_NONE) {
		if (stextsel == STORE_PEGBOY_QUERY) {
			stextlhold = STORE_PEGBOY_QUERY;
			// stextxhold = stextselx;
			stextvhold = stextsidx;
			stextshold = STORE_PEGBOY;
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
	// stextxhold = stextselx;
	// stextvhold = stextsidx;
	stextshold = STORE_PEGBOY;
	talker = TOWN_PEGBOY;
	StartStore(STORE_GOSSIP);
}

static void BoyBuyItem(int idx)
{
	// assert(idx == 0);
	boyitem._itype = ITYPE_NONE;

	SendStoreCmd2(STORE_PBUY);
}

/**
 * @brief Purchases an item from the healer.
 */
static void HealerBuyItem(int idx)
{
	bool infinite;

	infinite = idx < (IsMultiGame ? 3 : 2);
	if (infinite)
		storeitem._iSeed = NextRndSeed();

	SendStoreCmd2(STORE_HBUY);

	if (!infinite) {
		StoreShiftItems(&healitem[idx]);
	}
}

static void S_BBuyEnter()
{
	StoreStartBuy(STORE_PBUY);
}

static void StoryIdItem(int idx)
{
	idx = storehidx[idx];
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
		int idx = stextvhold + ((stextlhold - STORE_LIST_FIRST) / STORE_ITEM_LINES) * STORE_LINE_ITEMS + stextxhold;
		void (*func)(int);
		switch (lastshold) {
		case STORE_SBUY:
			func = SmithBuyItem;
			break;
		case STORE_SSELL:
		case STORE_WSELL:
			func = StoreSellItem;
			break;
		case STORE_SREPAIR:
			func = SmithRepairItem;
			break;
		case STORE_WBUY:
			func = WitchBuyItem;
			break;
		case STORE_WRECHARGE:
			func = WitchRechargeItem;
			break;
		case STORE_PBUY:
			func = BoyBuyItem;
			//lastshold = STORE_PEGBOY;
			break;
		case STORE_HBUY:
			func = HealerBuyItem;
			break;
		case STORE_SIDENTIFY:
			func = StoryIdItem;
			//lastshold = STORE_IDSHOW;
			break;
		case STORE_SPBUY:
			func = SmithBuyPItem;
			break;
		default:
			ASSUME_UNREACHABLE
			break;
		}
		func(idx);
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
		DEBUG_ASSERT(stextflag == STORE_HEALER);
		stextlhold = STORE_HEALER_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_HEALER;
		talker = TOWN_HEALER;
		StartStore(STORE_GOSSIP);
		break;
	case STORE_HEALER_HEAL:
		if (myplr._pHitPoints != myplr._pMaxHP) {
			PlrFillHp(mypnum);
			PlaySfx(IS_CAST8);
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
	StoreStartBuy(STORE_HBUY);
}

static void S_StoryEnter()
{
	switch (stextsel) {
	case STORE_STORY_GOSSIP:
		DEBUG_ASSERT(stextflag == STORE_STORY);
		stextlhold = STORE_STORY_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_STORY;
		talker = TOWN_STORY;
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
		DEBUG_ASSERT(stextflag == STORE_SIDENTIFY);
		stextlhold = stextsel;
		stextxhold = stextselx;
		stextvhold = stextsidx;
		stextshold = STORE_SIDENTIFY;
		idx = stextsidx + ((stextsel - STORE_LIST_FIRST) / STORE_ITEM_LINES) * STORE_LINE_ITEMS + stextselx;
		copy_pod(storeitem, storehold[idx]);
		if (myplr._pGold < storehold[idx]._iIvalue)
			StartStore(STORE_NOMONEY);
		else
			StartStore(STORE_CONFIRM);
	}
}

static void S_TalkEnter()
{
	if (stextsel == 22) {
		STextESC();
		// StartStore(stextshold);
		// stextsel = stextlhold;
		return;
	}
	DEBUG_ASSERT(stextsel != -1);
	int qn = -stextlines[stextsel]._sval;
	if (qn < NUM_QUESTS) {
		qn = Qtalklist[talker][qn];
	} else {
		assert(monsters[MAX_MINIONS + talker]._mType == talker);
		SetRndSeed(monsters[MAX_MINIONS + talker]._mRndSeed); // TNR_SEED
		qn = RandRangeLow(GossipList[talker][0], GossipList[talker][1]);
	}
	StartQTextMsg(qn);
}

static void S_TavernEnter()
{
	switch (stextsel) {
	case STORE_TAVERN_GOSSIP:
		DEBUG_ASSERT(stextflag == STORE_TAVERN);
		stextlhold = STORE_TAVERN_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_TAVERN;
		talker = TOWN_TAVERN;
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
		DEBUG_ASSERT(stextflag == STORE_BARMAID);
		stextlhold = STORE_BARMAID_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_BARMAID;
		talker = TOWN_BARMAID;
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
		DEBUG_ASSERT(stextflag == STORE_DRUNK);
		stextlhold = STORE_DRUNK_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_DRUNK;
		talker = TOWN_DRUNK;
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
		DEBUG_ASSERT(stextflag == STORE_PRIEST);
		stextlhold = STORE_PRIEST_GOSSIP;
		// stextxhold = stextselx;
		// stextvhold = stextsidx;
		stextshold = STORE_PRIEST;
		talker = TOWN_PRIEST;
		StartStore(STORE_GOSSIP);
		break;*/
	case STORE_PRIEST_ERRAND:
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
		// // stextflag = stextshold;
		// stextsel = stextlhold;
		// stextselx = stextxhold;
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
	PlaySfx(IS_TITLSLCT);
}

void TryStoreBtnClick()
{
	int y, ly;

	assert(!gbQtextflag);
	if (stextsel != -1 && stextflag != STORE_WAIT) {
		int px = gbWidePanel ? LTPANEL_X : STORE_PNL_X;
		int py = LTPANEL_Y;
		y = current_store_line(px, py);
		if (y == 0)
			return;
		//assert(LTPANEL_X + LTPANEL_WIDTH == STORE_PNL_X + STPANEL_WIDTH);
		//if (MousePos.x >= STORE_PNL_X + STPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2) - SCREEN_X && gbHasScroll) {
		if (MousePos.x >= px + LTPANEL_WIDTH - (SMALL_SCROLL_WIDTH + 2) - SCREEN_X && gbHasScroll) {
			assert(gbWidePanel);
			static_assert(SMALL_SCROLL_HEIGHT == STORE_LINE_HEIGHT, "TryStoreBtnClick needs more complex check");
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
		} else {
			POS32 target = current_store_target(y, px);
			if (target.y >= 0) {
				stextsel = target.y;
				if (target.x >= 0) {
					stextselx = target.x;
				}
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

ENABLE_SPEED_OPTIMIZATION

DEVILUTION_END_NAMESPACE
