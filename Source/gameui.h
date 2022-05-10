/**
 * @file gameui.h
 *
 * Constants and values which might depend on the logical size of the screen.
 */
#ifndef __GAMEUI_H__
#define __GAMEUI_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef HELLFIRE
#define LOGO_DATA				"Data\\hf_logo3.CEL"
#define LOGO_WIDTH				430
#else
#define LOGO_DATA				"Data\\Diabsmal.CEL"
#define LOGO_WIDTH				296
#endif

#ifndef DEFAULT_WIDTH
#define DEFAULT_WIDTH	640
#endif
#ifndef DEFAULT_HEIGHT
#define DEFAULT_HEIGHT	480
#endif

// automap expects a lower than 2:1 SCREEN_WIDTH to VIEWPORT_HEIGHT ratio
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH	dvl::screenWidth
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT	dvl::screenHeight
#endif

//#define VIEWPORT_HEIGHT dvl::viewportHeight
#define VIEWPORT_HEIGHT dvl::screenHeight

#define TILE_WIDTH		64
#define TILE_HEIGHT		32

#define ITEM_ANIM_WIDTH		96
#define ITEM_ANIM_XOFFSET	((ITEM_ANIM_WIDTH - TILE_WIDTH) / 2)

#define BORDER_LEFT		TILE_WIDTH
#define BORDER_TOP		160
#define BORDER_RIGHT	TILE_WIDTH
#define BORDER_BOTTOM	TILE_HEIGHT

#define SCREEN_X		BORDER_LEFT
#define SCREEN_Y		BORDER_TOP

#define BUFFER_WIDTH	(BORDER_LEFT + SCREEN_WIDTH + BORDER_RIGHT)
#define BUFFER_HEIGHT	(BORDER_TOP + SCREEN_HEIGHT + BORDER_BOTTOM)

#define PANEL_WIDTH		640
#define PANEL_HEIGHT	480
#define PANEL_LEFT		((int)((unsigned)SCREEN_WIDTH / 2 - PANEL_WIDTH / 2))
#define PANEL_RIGHT		((int)((unsigned)SCREEN_WIDTH / 2 + PANEL_WIDTH / 2))
#define PANEL_TOP		((int)((unsigned)SCREEN_HEIGHT / 2 - PANEL_HEIGHT / 2))
#define PANEL_BOTTOM	((int)((unsigned)SCREEN_HEIGHT / 2 + PANEL_HEIGHT / 2))
#define PANEL_MIDX(x)	((SCREEN_WIDTH - (x)) >> 1)
#define PANEL_MIDY(y)	((SCREEN_HEIGHT - (y)) >> 1)
#define PANEL_X			(SCREEN_X + PANEL_LEFT)
#define PANEL_Y			(SCREEN_Y + PANEL_TOP)
#define PANEL_CENTERX(x)(SCREEN_X + PANEL_MIDX(x))
#define PANEL_CENTERY(y)(SCREEN_Y + PANEL_MIDY(y))

#define SPANEL_WIDTH	294
#define SPANEL_HEIGHT	298

#define RIGHT_PANEL		(SCREEN_WIDTH - SPANEL_WIDTH)
#define RIGHT_PANEL_X	(SCREEN_X + RIGHT_PANEL)

#define LTPANEL_WIDTH	591
#define STPANEL_WIDTH	271
#define TPANEL_HEIGHT	303
#define TPANEL_BORDER	3

#define LTPANEL_X		PANEL_X + 24
#define LTPANEL_Y		PANEL_Y + 24

#define STORE_PNL_X		PANEL_X + 344
#define STORE_LINES		24

#define SCREENXY(x, y) ((x) + SCREEN_X + ((y) + SCREEN_Y) * BUFFER_WIDTH)

#define MENUBTN_WIDTH	71
#define MENUBTN_HEIGHT	19
#define CHRBTN_WIDTH	41
#define CHRBTN_HEIGHT	22
#define SPLICONLENGTH	56
#define SPLROWICONLS	10
#ifdef HELLFIRE
#define SPLICONLAST		52
#define SPLBOOKTABS		5
#else
#define SPLICONLAST		43
#define SPLBOOKTABS		4
#endif

#define QPNL_LINE_SPACING	24
#define QPNL_BORDER			10
#define QPNL_MAXENTRIES		((SPANEL_HEIGHT - 2 * QPNL_BORDER) / QPNL_LINE_SPACING)
#define QPNL_LINE_WIDTH		(SPANEL_WIDTH - 2 * QPNL_BORDER)
#define QPNL_TEXT_HEIGHT	12

#define TALK_PNL_WIDTH		302
#define TALK_PNL_HEIGHT		51
#define TALK_PNL_BORDER		15
#define TALK_PNL_TOP		364
#define TALK_PNL_LEFT		PANEL_MIDX(TALK_PNL_WIDTH)

#define PLRMSG_COUNT		32
#define PLRMSG_TEXT_BOTTOM	(TALK_PNL_TOP - 14)
#define PLRMSG_PANEL_BORDER	3
#define PLRMSG_TEXT_HEIGHT	12

#define SBOOK_CELWIDTH		37
#define SBOOK_CELHEIGHT		38
#define SBOOK_CELBORDER		 1
#define SBOOK_TOP_BORDER	 1
#define SBOOK_LEFT_BORDER	 1
#define SBOOK_LINE_BORDER	10
#define SBOOK_LINE_LENGTH	SPANEL_WIDTH - 2 * SBOOK_LEFT_BORDER - SBOOK_CELBORDER - SBOOK_CELWIDTH - 2 * SBOOK_LINE_BORDER
#define SBOOK_LINE_TAB		SBOOK_CELWIDTH + SBOOK_CELBORDER + SBOOK_LINE_BORDER
#define SBOOK_PAGER_WIDTH	56

#define TBOOK_BTN_WIDTH	60

#define INV_SLOT_SIZE_PX	28
#define BELT_WIDTH			60

#define DURICON_WIDTH		32
#define GOLDDROP_WIDTH		261

#define GAMEMENU_HEADER_Y		102
#define GAMEMENU_HEADER_OFF		13
#define GAMEMENU_ITEM_HEIGHT	45
#define SLIDER_ROW_WIDTH		490
#define SLIDER_BOX_WIDTH		287
#define SLIDER_BOX_HEIGHT		32
#define SLIDER_OFFSET			186
#define SLIDER_BORDER			2
#define SLIDER_STEPS			256
#define SLIDER_BUTTON_WIDTH		27

#define SMALL_SCROLL_WIDTH	12
#define SMALL_SCROLL_HEIGHT	12

#define SCROLLBAR_BG_WIDTH			25
#define SCROLLBAR_BG_HEIGHT			100
#define SCROLLBAR_THUMB_WIDTH		18
#define SCROLLBAR_THUMB_HEIGHT		19
#define SCROLLBAR_THUMB_OFFSET_X	3
#define SCROLLBAR_ARROW_WIDTH		25
#define SCROLLBAR_ARROW_HEIGHT		22

#define SMALL_LOGO_WIDTH	390
#define SMALL_LOGO_HEIGHT	154

#define BIG_LOGO_WIDTH	550
#define BIG_LOGO_HEIGHT	216

#define SMALL_POPUP_WIDTH		280
#define SMALL_POPUP_HEIGHT		144
#define LARGE_POPUP_WIDTH		385
#define LARGE_POPUP_HEIGHT		280
#define LARGE_POPUP_TEXT_WIDTH	346

#define SML_BUTTON_WIDTH	110
#define SML_BUTTON_HEIGHT	28

#define FOCUS_MINI			12
#define FOCUS_SMALL			20
#define FOCUS_MEDIUM		30
#define FOCUS_BIG			42
#define FOCUS_HUGE			48

#define CREDITS_TOP		100
#define CREDITS_HEIGHT	280
#define CREDITS_LINE_H	22

#define SELHERO_HEROS_WIDTH		180
#define SELHERO_HEROS_HEIGHT	76

#define PRBAR_WIDTH		228
#define PRBAR_HEIGHT	38

/** The width of the progress bar. */
#define BAR_WIDTH		534
#define BAR_HEIGHT		22
#define TOP_BAR_Y		37
#define BOTTOM_BAR_Y	421

DEVILUTION_END_NAMESPACE

#endif /* __GAMEUI_H__ */
