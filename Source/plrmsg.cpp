/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for printing the ingame chat messages.
 */
#include "all.h"
#include "engine/render/raw_render.h"
#include "engine/render/text_render.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#define PLRMSG_TEXT_TIMEOUT 10000

/** Specifies whether the Chat-Panel is displayed. */
bool gbTalkflag;
/** The cached messages of the Chat-Panel. */
static char sgszTalkSave[8][MAX_SEND_STR_LEN];
/** The next position in the sgszTalkSave to save the message. */
static BYTE sgbNextTalkSave;
/** The index of selected message in the sgszTalkSave array. */
static BYTE sgbTalkSavePos;
/** The next position in the plr_msgs to store the message. */
static BYTE plr_msg_slot;
/** The Chat-History of the received messages. */
static _plrmsg plr_msgs[PLRMSG_COUNT + 1];
//static Uint32 guDelayStartTc;

void plrmsg_delay(bool delay)
{
	/*int i;
	_plrmsg* pMsg;
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

void EventPlrMsg(const char* pszFmt, ...)
{
	_plrmsg* pMsg;
	va_list va;

	va_start(va, pszFmt);
	pMsg = AddPlrMsg(MAX_PLRS);
	vsnprintf(pMsg->str, sizeof(pMsg->str), pszFmt, va);
	va_end(va);
}

void ReceivePlrMsg(int pnum, const char* pszStr)
{
	_plrmsg* pMsg;

	pMsg = AddPlrMsg(pnum);
	snprintf(pMsg->str, sizeof(pMsg->str), "%s: %s", plr._pName, pszStr);
}

/*void ClearPlrMsg(int pnum)
{
	int i;
	_plrmsg* pMsg = plr_msgs;

	for (i = 0; i < PLRMSG_COUNT; i++, pMsg++) {
		if (pMsg->player == pnum)
			pMsg->str[0] = '\0';
	}
}*/

void InitPlrMsg()
{
	memset(plr_msgs, 0, sizeof(plr_msgs));
	gbTalkflag = false;
	sgbNextTalkSave = 0;
	sgbTalkSavePos = 0;
	plr_msg_slot = 0;
	// plr_msgs[PLRMSG_COUNT].player = mypnum;
	// plr_msgs[PLRMSG_COUNT].str[0] = '\0';
}

static int PrintPlrMsg(int x, int y, _plrmsg* pMsg)
{
	BYTE c, col = pMsg->player == MAX_PLRS ? COL_GOLD : COL_WHITE;
	int sx, line, len, width = PANEL_WIDTH - 20;
	const char *sstr, *endstr;
	const char* str = pMsg->str;

	line = GetSmallStringWidth(str) >= width ? 2 : 1;
	line *= PLRMSG_TEXT_HEIGHT;
	y -= line;

	DrawRectTrans(x - PLRMSG_PANEL_BORDER, y - (PLRMSG_PANEL_BORDER + PLRMSG_TEXT_HEIGHT), width + 2 * PLRMSG_PANEL_BORDER, line + 2 * PLRMSG_PANEL_BORDER, PAL_BLACK);

	line = 0;
	while (*str != '\0') {
		len = 0;
		sstr = endstr = str;
		while (TRUE) {
			if (*sstr != '\0') {
				c = gbStdFontFrame[(BYTE)*sstr++];
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
	if (&plr_msgs[PLRMSG_COUNT] == pMsg) {
		if (line == 0) {
			line = 1;
			sx = x;
			y += PLRMSG_TEXT_HEIGHT;
		}
		if ((SDL_GetTicks() / 512) % 2) { // GetAnimationFrame(2, 512) != 0) {
			PrintSmallChar(sx, y - PLRMSG_TEXT_HEIGHT, '|', col);
		}
	}
	return y - line * PLRMSG_TEXT_HEIGHT;
}

void DrawPlrMsg(bool onTop)
{
	int i, n, idx, x, y;
	Uint32 timeout;

	if (onTop != gbTalkflag)
		return;

	n = gbTalkflag ? PLRMSG_COUNT : 3;
	timeout = gbTalkflag ? 0 : SDL_GetTicks() - PLRMSG_TEXT_TIMEOUT;
	x = PLRMSG_TEXT_X;
	y = PLRMSG_TEXT_BOTTOM;
	if (gbTalkflag) {
		y = PrintPlrMsg(x, y, &plr_msgs[PLRMSG_COUNT]);
	}
	for (i = 1; i <= PLRMSG_COUNT; i++) {
		idx = (unsigned)(plr_msg_slot - i) % PLRMSG_COUNT;
		if (plr_msgs[idx].str[0] != '\0' && plr_msgs[idx].time >= timeout) {
			y = PrintPlrMsg(x, y, &plr_msgs[idx]);
			if (--n == 0 || y <= PLRMSG_TEXT_TOP)
				break;
		}
	}
}

void StartPlrMsg()
{
	if (IsLocalGame) {
		return;
	}

	gbTalkflag = true;
	SDL_StartTextInput();
	plr_msgs[PLRMSG_COUNT].str[0] = '\0';
	// gbRedrawFlags = REDRAW_ALL;
	sgbTalkSavePos = sgbNextTalkSave;
}

void SetupPlrMsg(int pnum, bool shift)
{
	if (!gbTalkflag)
		StartPlrMsg();
	if (!shift) {
		snprintf(plr_msgs[PLRMSG_COUNT].str, sizeof(plr_msgs[PLRMSG_COUNT].str), "/p%d ", pnum);
	} else {
		snprintf(plr_msgs[PLRMSG_COUNT].str, sizeof(plr_msgs[PLRMSG_COUNT].str), "/t%d ", plr._pTeam);
	}
}

void StopPlrMsg()
{
	gbTalkflag = false;
	SDL_StopTextInput();
	//gbRedrawFlags = REDRAW_ALL;
}

static void SendPlrMsg()
{
	int i, team, pmask;
	BYTE talk_save;
	char* msg;

	pmask = SNPLAYER_ALL;
	msg = &plr_msgs[PLRMSG_COUNT].str[0];

	if (msg[0] == '/') {
		if (msg[1] == 'p') {
			// "/pX msg" -> send message to player X
			i = strtol(&msg[2], &msg, 10);
			if (msg != &plr_msgs[PLRMSG_COUNT].str[2]) {
				pmask = 1 << i;
				if (*msg == ' ') {
					msg++;
				}
			} else {
				msg = &plr_msgs[PLRMSG_COUNT].str[0];
			}
		} else if (msg[1] == 't') {
			team = -1;
			if (msg[2] == ' ') {
				// "/t msg" -> send message to the player's team
				team = myplr._pTeam;
			} else {
				// "/tX msg" -> send message to the team N
				team = strtol(&msg[2], &msg, 10);
				if (msg == &plr_msgs[PLRMSG_COUNT].str[2]) {
					team = -1;
					msg = &plr_msgs[PLRMSG_COUNT].str[0];
				}
			}
			if (team != -1) {
				pmask = 0;
				for (i = 0; i < MAX_PLRS; i++) {
					if (players[i]._pTeam == team)
						pmask |= 1 << i;
				}
			}
		}
	}

	if (*msg != '\0') {
		SStrCopy(gbNetMsg, msg, sizeof(gbNetMsg));
		NetSendCmdString(pmask);

		for (i = 0; i < lengthof(sgszTalkSave); i++) {
			if (!strcmp(sgszTalkSave[i], &plr_msgs[PLRMSG_COUNT].str[0]))
				break;
		}
		if (i == lengthof(sgszTalkSave)) {
			memcpy(sgszTalkSave[sgbNextTalkSave], plr_msgs[PLRMSG_COUNT].str, sizeof(sgszTalkSave[sgbNextTalkSave]));
			sgbNextTalkSave++;
			sgbNextTalkSave &= 7;
		} else {
			talk_save = sgbNextTalkSave - 1;
			talk_save &= 7;
			if (i != talk_save) {
				memcpy(sgszTalkSave[i], sgszTalkSave[talk_save], sizeof(sgszTalkSave[i]));
				memcpy(sgszTalkSave[talk_save], plr_msgs[PLRMSG_COUNT].str, sizeof(sgszTalkSave[talk_save]));
			}
		}
		plr_msgs[PLRMSG_COUNT].str[0] = '\0';
		sgbTalkSavePos = sgbNextTalkSave;
	} else {
		StopPlrMsg();
	}
}

bool plrmsg_presschar(int vkey)
{
	unsigned result;

	// assert(gbTalkflag);
	// assert(!IsLocalGame);

	if (vkey < ' ')
		return false;

	result = strlen(plr_msgs[PLRMSG_COUNT].str);
	static_assert(sizeof(plr_msgs[PLRMSG_COUNT].str) >= MAX_SEND_STR_LEN, "Character does not fit to the container.");
	if (result < MAX_SEND_STR_LEN - 1) {
		plr_msgs[PLRMSG_COUNT].str[result] = vkey;
		plr_msgs[PLRMSG_COUNT].str[result + 1] = '\0';
	}
	return true;
}

static void plrmsg_up_down(int v)
{
	int i;

	static_assert(lengthof(sgszTalkSave) == 8, "Table sgszTalkSave does not work in plrmsg_up_down.");
	for (i = 0; i < lengthof(sgszTalkSave); i++) {
		sgbTalkSavePos = (v + sgbTalkSavePos) & 7;
		if (sgszTalkSave[sgbTalkSavePos][0] != '\0') {
			static_assert(sizeof(plr_msgs[PLRMSG_COUNT].str) >= sizeof(sgszTalkSave[sgbTalkSavePos]), "Message does not fit to the container.");
			memcpy(plr_msgs[PLRMSG_COUNT].str, sgszTalkSave[sgbTalkSavePos], sizeof(sgszTalkSave[sgbTalkSavePos]));
			return;
		}
	}
}

bool plrmsg_presskey(int vkey)
{
	int len;

	// assert(gbTalkflag);
	// assert(!IsLocalGame);

	if (vkey == DVL_VK_ESCAPE) {
		StopPlrMsg();
	} else if (vkey == DVL_VK_RETURN) {
		SendPlrMsg();
	} else if (vkey == DVL_VK_BACK) {
		len = strlen(plr_msgs[PLRMSG_COUNT].str);
		if (len > 0)
			plr_msgs[PLRMSG_COUNT].str[len - 1] = '\0';
	} else if (vkey == DVL_VK_DOWN) {
		plrmsg_up_down(1);
	} else if (vkey == DVL_VK_UP) {
		plrmsg_up_down(-1);
	} else if (vkey == DVL_VK_LBUTTON) {
		return false;
	} else if (vkey == DVL_VK_RBUTTON) {
		return false;
#ifdef USE_SDL1
	} else {
		// SDL1 does not support TEXTINPUT events, so we need to handle them here.
		vkey = TranslateKey2Char(vkey);
		plrmsg_presschar(vkey);
#endif
	}
	return true;
}

DEVILUTION_END_NAMESPACE
