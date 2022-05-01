/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for printing the ingame chat messages.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

#define PLRMSG_COUNT        32
#define PLRMSG_TEXT_BOTTOM  350
#define PLRMSG_PANEL_BORDER 3
#define PLRMSG_TEXT_HEIGHT  12
#define PLRMSG_TEXT_TIMEOUT 10000

static BYTE plr_msg_slot;
static _plrmsg plr_msgs[PLRMSG_COUNT];
//static Uint32 guDelayStartTc;

void plrmsg_delay(bool delay)
{
	/*int i;
	_plrmsg *pMsg;
	Uint32 deltaTc;

	deltaTc = SDL_GetTicks();
	if (delay) {
		guDelayStartTc = deltaTc;
		return;
	}
	deltaTc -= guDelayStartTc;
	pMsg = plr_msgs;
	for (i = 0; i < PLRMSG_COUNT; i++, pMsg++)
		pMsg->time += deltaTc;*/
}

static _plrmsg* AddPlrMsg(int pnum)
{
	_plrmsg* pMsg = &plr_msgs[plr_msg_slot];

	static_assert((PLRMSG_COUNT & (PLRMSG_COUNT - 1)) == 0, "Modulo to BitAnd optimization requires a power of 2.");
	plr_msg_slot = (unsigned)(plr_msg_slot + 1) % PLRMSG_COUNT;
	pMsg->player = pnum;
	pMsg->time = SDL_GetTicks();
	return pMsg;
}

#if DEV_MODE
void ErrorPlrMsg(const char *pszMsg)
{
	_plrmsg* pMsg;
	
	pMsg = AddPlrMsg(MAX_PLRS);
	SStrCopy(pMsg->str, pszMsg, sizeof(pMsg->str));
}
#endif

void EventPlrMsg(const char *pszFmt, ...)
{
	_plrmsg* pMsg;
	va_list va;

	va_start(va, pszFmt);
	pMsg = AddPlrMsg(MAX_PLRS);
	vsnprintf(pMsg->str, sizeof(pMsg->str), pszFmt, va);
	va_end(va);
}

void SendPlrMsg(int pnum, const char *pszStr)
{
	_plrmsg* pMsg;

	pMsg = AddPlrMsg(pnum);
	snprintf(pMsg->str, sizeof(pMsg->str), "%s: %s", plr._pName, pszStr);
}

/*void ClearPlrMsg(int pnum)
{
	int i;
	_plrmsg *pMsg = plr_msgs;

	for (i = 0; i < PLRMSG_COUNT; i++, pMsg++) {
		if (pMsg->player == pnum)
			pMsg->str[0] = '\0';
	}
}*/

void InitPlrMsg()
{
	memset(plr_msgs, 0, sizeof(plr_msgs));
	plr_msg_slot = 0;
}

static unsigned PrintPlrMsg(unsigned x, unsigned y, _plrmsg *pMsg)
{
	BYTE c, col = pMsg->player == MAX_PLRS ? COL_GOLD : COL_WHITE;
	int sx, line;
	unsigned len, width = PANEL_WIDTH - 20;
	const char *sstr, *endstr;
	const char *str = pMsg->str;

	line = (unsigned)GetSmallStringWidth(str) >= width ? 2 : 1;
	line *= PLRMSG_TEXT_HEIGHT;
	y -= line;

	trans_rect(x - PLRMSG_PANEL_BORDER, y - (PLRMSG_PANEL_BORDER + PLRMSG_TEXT_HEIGHT), width + 2 * PLRMSG_PANEL_BORDER, line + 2 * PLRMSG_PANEL_BORDER);

	line = 0;
	while (*str != '\0') {
		len = 0;
		sstr = endstr = str;
		while (TRUE) {
			if (*sstr != '\0') {
				c = smallFontFrame[gbFontTransTbl[(BYTE)*sstr++]];
				len += smallFontWidth[c] + FONT_KERN_SMALL;
				if (c == 0) // allow wordwrap on blank glyph
					endstr = sstr;
				else if (len >= width)
					break;
			} else {
				endstr = sstr;
				break;
			}
		}

		sx = x;
		while (str < endstr) {
			sx += PrintSmallChar(sx, y, (BYTE)*str++, col);
		}

		y += PLRMSG_TEXT_HEIGHT;
		if (++line == 2)
			break;
	}
	return y - line * PLRMSG_TEXT_HEIGHT;
}

void DrawPlrMsg(bool onTop)
{
	int i, n, idx;
	unsigned x, y;
	Uint32 timeout;

	if (onTop != gbTalkflag)
		return;

	n = gbTalkflag ? PLRMSG_COUNT : 3;
	timeout = gbTalkflag ? 0 : SDL_GetTicks() - PLRMSG_TEXT_TIMEOUT;
	x = 10 + SCREEN_X;
	y = SCREEN_Y + PLRMSG_TEXT_BOTTOM;
	for (i = 1; i <= PLRMSG_COUNT; i++) {
		idx = (unsigned)(plr_msg_slot - i) % PLRMSG_COUNT;
		if (plr_msgs[idx].str[0] != '\0' && plr_msgs[idx].time >= timeout) {
			y = PrintPlrMsg(x, y, &plr_msgs[idx]);
			if (--n == 0 || y <= (SCREEN_Y + 2 * PLRMSG_TEXT_HEIGHT + 2 * PLRMSG_PANEL_BORDER))
				break;
		}
	}
}

DEVILUTION_END_NAMESPACE
