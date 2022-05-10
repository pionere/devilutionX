/**
 * @file gameui.h
 *
 * Constants and values which might depend on the logical size of the screen.
 */
#ifndef __GAMEUI_H__
#define __GAMEUI_H__

DEVILUTION_BEGIN_NAMESPACE

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
#define PANEL_X			(SCREEN_X + PANEL_LEFT)
#define PANEL_Y			(SCREEN_Y + PANEL_TOP)

#define SPANEL_WIDTH	294
#define SPANEL_HEIGHT	298

#define RIGHT_PANEL		(SCREEN_WIDTH - SPANEL_WIDTH)
#define RIGHT_PANEL_X	(SCREEN_X + RIGHT_PANEL)

#define DIALOG_TOP		((SCREEN_HEIGHT - 128) / 2 - 18)
#define DIALOG_Y		(SCREEN_Y + DIALOG_TOP)

#define LTPANEL_WIDTH	591
#define STPANEL_WIDTH	271
#define TPANEL_HEIGHT	303
#define TPANEL_BORDER	3

#define LTPANEL_X		PANEL_X + 24
#define LTPANEL_Y		PANEL_Y + 24

#define STORE_PNL_X		PANEL_X + 344

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

DEVILUTION_END_NAMESPACE

#endif /* __GAMEUI_H__ */
