/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for printing the ingame chat messages.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

static BYTE plr_msg_slot;
_plrmsg plr_msgs[PMSG_COUNT];

/** Maps from player_num to text color, as used in chat messages. */
const char text_color_from_player_num[MAX_PLRS + 1] = { COL_WHITE, COL_WHITE, COL_WHITE, COL_WHITE, COL_GOLD };

void plrmsg_delay(bool delay)
{
	int i;
	_plrmsg *pMsg;
	static DWORD plrmsg_ticks;

	if (delay) {
		plrmsg_ticks = SDL_GetTicks();
		return;
	}

	plrmsg_ticks = SDL_GetTicks() - plrmsg_ticks;
	pMsg = plr_msgs;
	for (i = 0; i < PMSG_COUNT; i++, pMsg++)
		pMsg->time += plrmsg_ticks;
}

void ErrorPlrMsg(const char *pszMsg)
{
	_plrmsg *pMsg = &plr_msgs[plr_msg_slot];
	plr_msg_slot = (plr_msg_slot + 1) & (PMSG_COUNT - 1);
	pMsg->player = MAX_PLRS;
	pMsg->time = SDL_GetTicks();
	SStrCopy(pMsg->str, pszMsg, sizeof(pMsg->str));
}

void EventPlrMsg(const char *pszFmt, ...)
{
	_plrmsg *pMsg;
	va_list va;

	va_start(va, pszFmt);
	pMsg = &plr_msgs[plr_msg_slot];
	plr_msg_slot = (plr_msg_slot + 1) & (PMSG_COUNT - 1);
	pMsg->player = MAX_PLRS;
	pMsg->time = SDL_GetTicks();
	vsnprintf(pMsg->str, sizeof(pMsg->str), pszFmt, va);
	va_end(va);
}

void SendPlrMsg(int pnum, const char *pszStr)
{
	_plrmsg *pMsg = &plr_msgs[plr_msg_slot];
	plr_msg_slot = (plr_msg_slot + 1) & (PMSG_COUNT - 1);
	pMsg->player = pnum;
	pMsg->time = SDL_GetTicks();
#ifdef _DEBUG
	strlen(plr[pnum]._pName); /* these are used in debug */
	strlen(pszStr);
#endif
	snprintf(pMsg->str, sizeof(pMsg->str), "%s: %s", plr[pnum]._pName, pszStr);
}

void ClearPlrMsg()
{
	int i;
	_plrmsg *pMsg = plr_msgs;
	DWORD tick = SDL_GetTicks();

	for (i = 0; i < PMSG_COUNT; i++, pMsg++) {
		if ((int)(tick - pMsg->time) > 10000)
			pMsg->str[0] = '\0';
	}
}

void InitPlrMsg()
{
	memset(plr_msgs, 0, sizeof(plr_msgs));
	plr_msg_slot = 0;
}

static void PrintPlrMsg(unsigned x, unsigned y, unsigned width, const char *str, BYTE col)
{
	BYTE c;
	int sx, line = 0;
	unsigned len;
	const char *sstr, *endstr;

	while (*str != '\0') {
		len = 0;
		sstr = endstr = str;
		while (TRUE) {
			if (*sstr != '\0') {
				c = sfontframe[gbFontTransTbl[(BYTE)*sstr++]];
				len += sfontkern[c] + 1;
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
			c = sfontframe[gbFontTransTbl[(BYTE)*str++]];
			if (c != 0)
				PrintChar(sx, y, c, col);
			sx += sfontkern[c] + 1;
		}

		y += 10;
		if (++line == 3)
			break;
	}
}

void DrawPlrMsg()
{
	int i;
	unsigned x = 10 + SCREEN_X;
	unsigned y = 70 + SCREEN_Y;
	unsigned width = SCREEN_WIDTH - 20;
	_plrmsg *pMsg;

	if (gbChrflag | gbQuestlog) {
		x += SPANEL_WIDTH;
		width -= SPANEL_WIDTH;
	}
	if (gbInvflag | gbSbookflag | gbTeamFlag)
		width -= SPANEL_WIDTH;

	if (width < 300)
		return;

	pMsg = plr_msgs;
	for (i = 0; i < PMSG_COUNT; i++) {
		if (pMsg->str[0] != '\0')
			PrintPlrMsg(x, y, width, pMsg->str, text_color_from_player_num[pMsg->player]);
		pMsg++;
		y += 35;
	}
}

DEVILUTION_END_NAMESPACE
