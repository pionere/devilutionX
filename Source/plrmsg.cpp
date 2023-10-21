/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for printing the ingame chat messages.
 */
#include "all.h"
#include "engine/render/raw_render.h"
#include "engine/render/text_render.h"
#include "utils/utf8.h"
#include "storm/storm_net.h"

DEVILUTION_BEGIN_NAMESPACE

#define PLRMSG_TEXT_TIMEOUT 10000
#define PLRMSG_WIDTH        (PANEL_WIDTH - 20)

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
/** The cursor position. */
static unsigned sguCursPos;

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

static void plrmsg_WordWrap(_plrmsg* pMsg)
{
	char* text = pMsg->str;
	int lineStart = 0;

	const int width = PLRMSG_WIDTH;
	int len = 0;
	int i = 0;
	while (text[i] != '\0') {
		BYTE c = gbStdFontFrame[(BYTE)text[i]];
		len += smallFontWidth[c] + FONT_KERN_SMALL;
		i++;
		if (len <= width) {
			continue;
		}

		if (lineStart != 0) {
			text[i - 1] = '\0';
			break; // more than one line break -> skip the rest
		}

		int j = i;
		while (--j >= 0) {
			if (gbStdFontFrame[(BYTE)text[j]] == 0) {
				break; // Scan for previous blank glyph
			}
		}

		if (j < 0) {
			j = i - 2; // the word is longer than one line -> split the word
		}

		j++;
		lineStart = j;
		len = 0;
		i = j;
	}

	pMsg->lineBreak = lineStart;
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
	plrmsg_WordWrap(pMsg);
}

void ReceivePlrMsg(int pnum, const char* pszStr)
{
	_plrmsg* pMsg;

	pMsg = AddPlrMsg(pnum);
	snprintf(pMsg->str, sizeof(pMsg->str), "%s: %s", plr._pName, pszStr);
	plrmsg_WordWrap(pMsg);
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
	// sguCursPos = 0;
	// plr_msgs[PLRMSG_COUNT].player = mypnum;
	// plr_msgs[PLRMSG_COUNT].str[0] = '\0';
}

static int PrintPlrMsg(int x, int y, _plrmsg* pMsg)
{
	BYTE c, col;
	int sx, line, result, breakPos;
	unsigned curPos;
	const int width = PLRMSG_WIDTH;
	const char* str;

	line = pMsg->lineBreak != 0 ? 2 : 1;
	line *= PLRMSG_TEXT_HEIGHT;
	y -= line;

	result = y;

	DrawRectTrans(x - PLRMSG_PANEL_BORDER, y - (PLRMSG_PANEL_BORDER + PLRMSG_TEXT_HEIGHT), width + 2 * PLRMSG_PANEL_BORDER, line + 2 * PLRMSG_PANEL_BORDER, PAL_BLACK);

	str = pMsg->str;
	col = pMsg->player == MAX_PLRS ? COL_GOLD : COL_WHITE;
	curPos = (&plr_msgs[PLRMSG_COUNT] == pMsg && ((SDL_GetTicks() / 512) % 2) != 0) ? sguCursPos : 2 * MAX_SEND_STR_LEN; // GetAnimationFrame(2, 512) != 0) {
	curPos++;

	sx = x;
	breakPos = pMsg->lineBreak;
	while (true) {
		if (--curPos == 0) {
			PrintSmallChar(sx - 1, y, '|', col); // - smallFontWidth[gbStdFontFrame['|']] / 2
		}
		c = (BYTE)*str++;
		if (c == '\0') {
			break;
		}
		sx += PrintSmallChar(sx, y, c, col);
		if (--breakPos == 0) {
			y += PLRMSG_TEXT_HEIGHT;
			sx = x;
		}
	}

	return result;
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
	sguCursPos = 0;
}

void SetupPlrMsg(int pnum, bool shift)
{
	const char* text;
	int param;

	if (!gbTalkflag)
		StartPlrMsg();
	if (!shift) {
		text = "/p%d ";
		param = pnum;
	} else {
		text = "/t%d ";
		param = plr._pTeam;
	}
	int len = snprintf(plr_msgs[PLRMSG_COUNT].str, sizeof(plr_msgs[PLRMSG_COUNT].str), text, param);
	sguCursPos = len;
	// plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
	plr_msgs[PLRMSG_COUNT].lineBreak = 0;
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
		sguCursPos = 0;
		// plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
		plr_msgs[PLRMSG_COUNT].lineBreak = 0;
		sgbTalkSavePos = sgbNextTalkSave;
	} else {
		StopPlrMsg();
	}
}

void plrmsg_CatToText(const char* inBuf)
{
	// assert(gbTalkflag);
	// assert(!IsLocalGame);

	char* output = utf8_to_latin1(inBuf);
	unsigned sp = sguCursPos;
	unsigned cp = sguCursPos;
	char* text = plr_msgs[PLRMSG_COUNT].str;
	const unsigned maxlen = MAX_SEND_STR_LEN;
	// assert(maxLen - sp < sizeof(tempstr));
	SStrCopy(tempstr, &text[sp], std::min((unsigned)sizeof(tempstr) - 1, maxlen - sp));
	SStrCopy(&text[cp], output, maxlen - cp);
	mem_free_dbg(output);
	cp = strlen(text);
	sguCursPos = cp;
	SStrCopy(&text[cp], tempstr, maxlen - cp);

	plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
}

static void plrmsg_DelFromText(int w)
{
	char* text = plr_msgs[PLRMSG_COUNT].str;
	unsigned max_length = MAX_SEND_STR_LEN;

	for (unsigned i = sguCursPos; ; i++) {
		// assert(max_length != 0);
		if (text[i] == '\0' || (i + w) >= max_length) {
			text[i] = '\0';
			break;
		} else {
			text[i] = text[i + w];
		}
	}

	plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
}

static void plrmsg_up_down(int v)
{
	int i;

	static_assert(lengthof(sgszTalkSave) == 8, "Table sgszTalkSave does not work in plrmsg_up_down.");
	for (i = 0; i < lengthof(sgszTalkSave); i++) {
		sgbTalkSavePos = (v + sgbTalkSavePos) & 7;
		unsigned len = strlen(sgszTalkSave[sgbTalkSavePos]);
		if (len != 0) {
			static_assert(sizeof(plr_msgs[PLRMSG_COUNT].str) >= sizeof(sgszTalkSave[sgbTalkSavePos]), "Message does not fit to the container.");
			sguCursPos = len;
			memcpy(plr_msgs[PLRMSG_COUNT].str, sgszTalkSave[sgbTalkSavePos], len + 1);
			plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
			return;
		}
	}
}

bool plrmsg_presskey(int vkey)
{
	// assert(gbTalkflag);
	// assert(!IsLocalGame);

	switch (vkey) {
	case DVL_VK_ESCAPE:
		StopPlrMsg();
		break;
	case DVL_VK_RETURN:
		SendPlrMsg();
		break;
	case DVL_VK_LEFT: {
		unsigned pos = sguCursPos;
		if (pos > 0) {
			sguCursPos = pos - 1;
		}
	} break;
	case DVL_VK_RIGHT: {
		unsigned pos = sguCursPos;
		if (plr_msgs[PLRMSG_COUNT].str[pos] != '\0' && pos + 1 < MAX_SEND_STR_LEN) {
			sguCursPos = pos + 1;
		}
	} break;
	case DVL_VK_HOME: {
		sguCursPos = 0;
	} break;
	case DVL_VK_END: {
		unsigned pos = strlen(plr_msgs[PLRMSG_COUNT].str);
		sguCursPos = pos;
	} break;
	case DVL_VK_BACK: {
		int w = 1;
		unsigned i = sguCursPos;
		if (i == 0) {
			break;
		}
		i--;
		sguCursPos = i;
		plrmsg_DelFromText(w);
	} break;
	case DVL_VK_DELETE: {
		int w = 1;
		plrmsg_DelFromText(w);
	} break;
	case DVL_VK_DOWN:
		plrmsg_up_down(1);
		break;
	case DVL_VK_UP:
		plrmsg_up_down(-1);
		break;
	case DVL_VK_LBUTTON:
		return false;
	case DVL_VK_RBUTTON:
		return false;
#ifdef USE_SDL1
	default:
		// SDL1 does not support TEXTINPUT events, so we need to handle them here.
		vkey = TranslateKey2Char(vkey);
		char utf8[2];
		utf8[0] = (char)vkey;
		utf8[1] = '\0';
		plrmsg_CatToText(utf8);
		break;
#endif
	}
	return true;
}

DEVILUTION_END_NAMESPACE
