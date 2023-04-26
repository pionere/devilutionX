/**
 * @file gameui.h
 *
 * Constants and values which might depend on the logical size of the screen.
 */
#ifndef __GAMEUI_H__
#define __GAMEUI_H__

DEVILUTION_BEGIN_NAMESPACE

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////
// IN-GAME UI
//////////////////////////////////////////////////

#ifndef DEFAULT_WIDTH
#define DEFAULT_WIDTH  (640 * ASSET_MPL)
#endif
#ifndef DEFAULT_HEIGHT
#define DEFAULT_HEIGHT (480 * ASSET_MPL)
#endif

#define TILE_WIDTH     (64 * ASSET_MPL)
#define TILE_HEIGHT    (32 * ASSET_MPL)

#define ITEM_ANIM_WIDTH   (96 * ASSET_MPL)
#define ITEM_ANIM_XOFFSET ((ITEM_ANIM_WIDTH - TILE_WIDTH) / 2)

#define BORDER_LEFT    TILE_WIDTH
// maximum of
//  1. max(height of a player  - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT) + 1 (outline) = (~80 * ASSET_MPL - 128, 32) + 1  = 33 -- cl2_render
//  2. max(height of a monster - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT) + 1 (outline) = (160 * ASSET_MPL - 128, 32) + 1  = 33 -- cl2_render
//  3. max(height of a towner  - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT) + 1 (outline) = (160 * ASSET_MPL - 128, 32) + 1  = 33 -- cel_render
//  4. max(height of a missile - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT)               = (~100 * ASSET_MPL - 128, 32)     = 32 -- cl2_render
//  5. max(height of a object  - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT) + 1 (outline) = (~110 * ASSET_MPL - 128, 32) + 1 = 33 -- cel_render
//  6. max(height of a item    - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT)               = (160 * ASSET_MPL - 128, 32)      = 32 -- cel_render
//  7. max(height of a special tile - CEL_BLOCK_MAX * CEL_BLOCK_HEIGHT, CEL_BLOCK_HEIGHT)          = (160 * ASSET_MPL - 128, 32)      = 32 -- cel_render
//  7. (MAP_SCALE_MAX * TILE_WIDTH) / 128 / 4 = 128 * 64 * ASSET_MPL / 128 / 4 = 16                                                        -- automap_render
//  8. MICRO_HEIGHT = 32                                                                                                                   -- dun_render
#define BORDER_TOP     (160 * ASSET_MPL - 128 + 16)
#define BORDER_BOTTOM  (160 * ASSET_MPL - 128 + 16)

#define SCREEN_X       BORDER_LEFT
#define SCREEN_Y       BORDER_TOP

// automap expects a lower than 2:1 SCREEN_WIDTH to VIEWPORT_HEIGHT ratio
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH   dvl::screenWidth
#define BUFFER_WIDTH   dvl::gnBufferWidth
#else
#define BUFFER_WIDTH   (BORDER_LEFT + SCREEN_WIDTH + BORDER_LEFT)
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT  dvl::screenHeight
#endif

//#define VIEWPORT_HEIGHT dvl::viewportHeight
#define VIEWPORT_HEIGHT dvl::screenHeight

#define BUFFER_HEIGHT (BORDER_TOP + SCREEN_HEIGHT + BORDER_BOTTOM)

#define PANEL_WIDTH      (640 * ASSET_MPL)
#define PANEL_HEIGHT     (480 * ASSET_MPL)
#define PANEL_LEFT       ((int)((unsigned)SCREEN_WIDTH / 2 - PANEL_WIDTH / 2))
#define PANEL_RIGHT      ((int)((unsigned)SCREEN_WIDTH / 2 + PANEL_WIDTH / 2))
#define PANEL_TOP        ((int)((unsigned)SCREEN_HEIGHT / 2 - PANEL_HEIGHT / 2))
#define PANEL_BOTTOM     ((int)((unsigned)SCREEN_HEIGHT / 2 + PANEL_HEIGHT / 2))
#define PANEL_MIDX(x)    ((SCREEN_WIDTH - (x)) >> 1)
#define PANEL_MIDY(y)    ((SCREEN_HEIGHT - (y)) >> 1)
#define PANEL_X          (SCREEN_X + PANEL_LEFT)
#define PANEL_Y          (SCREEN_Y + PANEL_TOP)
#define PANEL_CENTERX(x) (SCREEN_X + PANEL_MIDX(x))
#define PANEL_CENTERY(y) (SCREEN_Y + PANEL_MIDY(y))

#define SPANEL_WIDTH  294
#define SPANEL_HEIGHT 298

#define RIGHT_PANEL   (SCREEN_WIDTH - SPANEL_WIDTH)
#define RIGHT_PANEL_X (SCREEN_X + RIGHT_PANEL)

#define MENUBTN_WIDTH  71
#define MENUBTN_HEIGHT 19
#define CHRBTN_WIDTH   41
#define CHRBTN_HEIGHT  22
#if ASSET_MPL == 1
#define SPLICON_WIDTH  37
#define SPLICON_HEIGHT 38
#define SPLROWICONLS   ((640 - 233) / SPLICON_WIDTH)
#define SPLICON_OVERX  1
#define SPLICON_OVERY  1
#else
#define SPLICON_WIDTH  56
#define SPLICON_HEIGHT 56
#define SPLROWICONLS   (640 / SPLICON_WIDTH)
#define SPLICON_OVERX  4
#define SPLICON_OVERY  4
#endif
#ifdef HELLFIRE
#define SPLICONLAST    52
#define SPLBOOKTABS    7
#else
#define SPLICONLAST    43
#define SPLBOOKTABS    6
#endif

#define LIFE_FLASK_WIDTH   118
#define LIFE_FLASK_X       (PANEL_X + MENUBTN_WIDTH - LIFE_FLASK_WIDTH / 6)
#define MANA_FLASK_WIDTH   93
#define MANA_FLASK_X       (PANEL_X + PANEL_WIDTH - (SPLICON_WIDTH + MANA_FLASK_WIDTH - MANA_FLASK_WIDTH / 20))
#define FLASK_TOTAL_HEIGHT 95
#define FLASK_BULB_HEIGHT  82

#define LVLUP_LEFT   (PANEL_LEFT + 175)
#define LVLUP_OFFSET 24

#define LTPANEL_WIDTH 591
#define STPANEL_WIDTH 271
#define TPANEL_HEIGHT 303
#define TPANEL_BORDER 3

#define LTPANEL_X PANEL_CENTERX(LTPANEL_WIDTH)
#define LTPANEL_Y (PANEL_CENTERY(TPANEL_HEIGHT) - 64)

#define STORE_PNL_X (PANEL_X + 344 * ASSET_MPL)
#define STORE_LINES 24

#define SCREENXY(x, y) ((x) + SCREEN_X + ((y) + SCREEN_Y) * BUFFER_WIDTH)

#define QPNL_LINE_SPACING 24
#define QPNL_BORDER       10
#define QPNL_MAXENTRIES   ((SPANEL_HEIGHT - 2 * QPNL_BORDER) / QPNL_LINE_SPACING)
#define QPNL_LINE_WIDTH   (SPANEL_WIDTH - 2 * QPNL_BORDER)
#define QPNL_TEXT_HEIGHT  12

#define PLRMSG_COUNT        32
#define PLRMSG_PANEL_BORDER 3
#define PLRMSG_TEXT_HEIGHT  12
#define PLRMSG_TEXT_X       (PANEL_X + 10)
#define PLRMSG_TEXT_BOTTOM  (SCREEN_Y + PANEL_BOTTOM - 130)
#define PLRMSG_TEXT_TOP     (PANEL_Y + 2 * PLRMSG_TEXT_HEIGHT + 2 * PLRMSG_PANEL_BORDER)

#define SBOOK_CELWIDTH    37
#define SBOOK_CELHEIGHT   38
#define SBOOK_CELBORDER   1
#define SBOOK_TOP_BORDER  1
#define SBOOK_LEFT_BORDER 1
#define SBOOK_LINE_BORDER 10
#define SBOOK_LINE_LENGTH (SPANEL_WIDTH - 2 * SBOOK_LEFT_BORDER - SBOOK_CELBORDER - SBOOK_CELWIDTH - 2 * SBOOK_LINE_BORDER)
#define SBOOK_LINE_TAB    (SBOOK_CELWIDTH + SBOOK_CELBORDER + SBOOK_LINE_BORDER)
#define SBOOK_PAGER_WIDTH 56

#define TBOOK_BTN_WIDTH 60

#define INV_SLOT_SIZE_PX 28
#define BELT_WIDTH       60
#define BELT_HEIGHT      118
#define BELT_LEFT        PANEL_LEFT
#define BELT_TOP         (PANEL_HEIGHT - (MENUBTN_HEIGHT + BELT_HEIGHT))

#define DURICON_WIDTH  32
#define GOLDDROP_WIDTH 261
//#define DOOM_WIDTH   640

#define GAMEMENU_HEADER_Y    (102 * ASSET_MPL)
#define GAMEMENU_ITEM_HEIGHT 45
#if ASSET_MPL == 1
#define GAMEMENU_HEADER_OFF  13
#else
#define GAMEMENU_HEADER_OFF  (PANEL_MIDY(GAMEMENU_ITEM_HEIGHT * 5) - GAMEMENU_HEADER_Y)
#endif

#define SLIDER_ROW_WIDTH    490
#define SLIDER_BOX_WIDTH    287
#define SLIDER_BOX_HEIGHT   32
#define SLIDER_OFFSET       186
#define SLIDER_BORDER       2
#define SLIDER_STEPS        256
#define SLIDER_BUTTON_WIDTH 27

#define SMALL_SCROLL_WIDTH  12
#define SMALL_SCROLL_HEIGHT 12

#define SCROLLBAR_BG_WIDTH       25
#define SCROLLBAR_BG_HEIGHT      100
#define SCROLLBAR_THUMB_WIDTH    18
#define SCROLLBAR_THUMB_HEIGHT   19
#define SCROLLBAR_THUMB_OFFSET_X 3
#define SCROLLBAR_ARROW_WIDTH    25
#define SCROLLBAR_ARROW_HEIGHT   22

#define SMALL_POPUP_WIDTH      280
#define SMALL_POPUP_HEIGHT     144
#define LARGE_POPUP_WIDTH      385
#define LARGE_POPUP_HEIGHT     280
#define LARGE_POPUP_TEXT_WIDTH 346

#define SML_BUTTON_WIDTH  110
#define SML_BUTTON_HEIGHT 28

#define FOCUS_MINI   12
#define FOCUS_SMALL  20
#define FOCUS_MEDIUM 30
#define FOCUS_BIG    42
#define FOCUS_HUGE   48

//////////////////////////////////////////////////
// DIABLO UI (MENU)
//////////////////////////////////////////////////

#ifdef HELLFIRE
#define LOGO_DATA  "Data\\hf_logo3.CEL"
#define LOGO_WIDTH 430
#else
#define LOGO_DATA  "Data\\Diabsmal.CEL"
#define LOGO_WIDTH 296
#endif

#define SMALL_LOGO_WIDTH  390
#define SMALL_LOGO_HEIGHT 154
#define SMALL_LOGO_TOP    (PANEL_TOP + (ASSET_MPL - 1) * 100)

#define BIG_LOGO_WIDTH  550
#define BIG_LOGO_HEIGHT 216
#define BIG_LOGO_TOP    (PANEL_TOP + 182 * ASSET_MPL)

#define MAINMENU_WIDTH       (416 + 2 * FOCUS_HUGE)
#define MAINMENU_ITEM_HEIGHT 43
#if ASSET_MPL == 1
#define MAINMENU_TOP         (PANEL_TOP + SMALL_LOGO_HEIGHT + 36)
#else
#ifndef HOSTONLY
#define MAINMENU_HEIGHT      (MAINMENU_ITEM_HEIGHT * 6)
#else
#define MAINMENU_HEIGHT	     (MAINMENU_ITEM_HEIGHT * 5)
#endif /* HOSTONLY */
#define MAINMENU_TOP         PANEL_MIDY(MAINMENU_HEIGHT)
#endif /* ASSET_MPL == 1 */

#define CREDITS_TOP    (PANEL_TOP + 100 * ASSET_MPL)
#define CREDITS_HEIGHT (280 * ASSET_MPL)
#define CREDITS_LEFT   (PANEL_MIDX(620))
#define CREDITS_LINE_H 22

#define SELCONN_TITLE_TOP     (SMALL_LOGO_TOP + SMALL_LOGO_HEIGHT + 7 * ASSET_MPL)
#define SELCONN_PNL_TOP       (PANEL_TOP + 211 * ASSET_MPL)
#define SELCONN_HEADER_HEIGHT (34 * ASSET_MPL)
#define SELCONN_CONTENT_TOP   (SELCONN_PNL_TOP + SELCONN_HEADER_HEIGHT)
#define SELCONN_LPANEL_LEFT   (PANEL_LEFT + 25 * ASSET_MPL)
#define SELCONN_LPANEL_WIDTH  (228 * ASSET_MPL)
#define SELCONN_RPANEL_LEFT   (PANEL_LEFT + 280 * ASSET_MPL)
#define SELCONN_RPANEL_WIDTH  (334 * ASSET_MPL)
#define SELCONN_RPANEL_HEIGHT (178 * ASSET_MPL)
#define SELCONN_LIST_TOP      (SELCONN_CONTENT_TOP + 11 * ASSET_MPL)
#define SELCONN_RBUTTON_TOP   (SELCONN_CONTENT_TOP + SELCONN_RPANEL_HEIGHT + 4 * ASSET_MPL)

#define SELHERO_TITLE_TOP      (SMALL_LOGO_TOP + SMALL_LOGO_HEIGHT + 7 * ASSET_MPL)
#define SELHERO_PNL_TOP        (PANEL_TOP + 211 * ASSET_MPL)
#define SELHERO_HEROS_WIDTH    (180 * ASSET_MPL)
#define SELHERO_HEROS_HEIGHT   (76 * ASSET_MPL)
#define SELHERO_HEROS_LEFT     (PANEL_LEFT + 30 * ASSET_MPL)
#define SELHERO_LPANEL_HEIGHT  (161 * ASSET_MPL)
#define SELHERO_LPANEL_TOP     (SELHERO_PNL_TOP + SELHERO_HEROS_HEIGHT + 14 * ASSET_MPL)
#define SELHERO_LCONTENT_TOP   (SELHERO_LPANEL_TOP + (SELHERO_LPANEL_HEIGHT - (35 + 4 * 21)) / 2)
#define SELHERO_RHEADER_HEIGHT (34 * ASSET_MPL)
#define SELHERO_RPANEL_WIDTH   (370 * ASSET_MPL)
#define SELHERO_RPANEL_HEIGHT  (178 * ASSET_MPL)
#define SELHERO_RPANEL_LEFT    (PANEL_LEFT + 240 * ASSET_MPL)
#define SELHERO_RPANEL_TOP     (SELHERO_PNL_TOP + SELHERO_RHEADER_HEIGHT)
#define SELHERO_LIST_TOP       (SELHERO_RPANEL_TOP + 11 * ASSET_MPL)
#define SELHERO_RBUTTON_TOP    (SELHERO_RPANEL_TOP + SELHERO_RPANEL_HEIGHT + 4 * ASSET_MPL)

#define SELGAME_TITLE_TOP     (SMALL_LOGO_TOP + SMALL_LOGO_HEIGHT + 7 * ASSET_MPL)
#define SELGAME_PNL_TOP       (PANEL_TOP + 211 * ASSET_MPL)
#define SELGAME_HEADER_HEIGHT (34 * ASSET_MPL)
#define SELGAME_CONTENT_TOP   (SELGAME_PNL_TOP + SELGAME_HEADER_HEIGHT)
#define SELGAME_LPANEL_LEFT   (PANEL_LEFT + 25 * ASSET_MPL)
#define SELGAME_LPANEL_WIDTH  (228 * ASSET_MPL)
#define SELGAME_RPANEL_LEFT   (PANEL_LEFT + 280 * ASSET_MPL)
#define SELGAME_RPANEL_WIDTH  (334 * ASSET_MPL)
#define SELGAME_RPANEL_HEIGHT (178 * ASSET_MPL)
#define SELGAME_LIST_TOP      (SELGAME_CONTENT_TOP + 11 * ASSET_MPL)
#define SELGAME_RBUTTON_TOP   (SELGAME_CONTENT_TOP + SELGAME_RPANEL_HEIGHT + 4 * ASSET_MPL)

#define SELYNOK_TITLE_TOP      (SMALL_LOGO_TOP + SMALL_LOGO_HEIGHT + 7 * ASSET_MPL)
#define SELYNOK_CONTENT_HEIGHT 168
#define SELYNOK_CONTENT_TOP    PANEL_MIDY(22)
#define SELYNOK_BUTTON_TOP     (SELYNOK_CONTENT_TOP + SELYNOK_CONTENT_HEIGHT)

#define PRBAR_WIDTH  228
#define PRBAR_HEIGHT 38

/** The width of the progress bar. */
#define BAR_WIDTH    (534 * ASSET_MPL)
#define BAR_HEIGHT   (22 * ASSET_MPL)
#define TOP_BAR_Y    (37 * ASSET_MPL)
#define BOTTOM_BAR_Y (421 * ASSET_MPL)

#define SMALL_FONT_HEIGHT 11

#ifdef __cplusplus
}
#endif

DEVILUTION_END_NAMESPACE

#endif /* __GAMEUI_H__ */