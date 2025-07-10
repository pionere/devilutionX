/**
 * @file plrmsg.cpp
 *
 * Implementation of functionality for printing the ingame chat messages.
 */
#include "all.h"
#include <config.h>
#include "engine/render/raw_render.h"
#include "engine/render/text_render.h"
#include "plrctrls.h"
#include "utils/utf8.h"
#include "storm/storm_net.h"
#include <ctime>

DEVILUTION_BEGIN_NAMESPACE

#define PLRMSG_TEXT_TIMEOUT 10
#define PLRMSG_WIDTH        (PANEL_WIDTH - 20)

static const char gszProductName[] = { PROJECT_NAME " v" PROJECT_VERSION };

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
/** The selection position. */
static unsigned sguSelPos;
/** Specifies whether the user is selecting a text. */
static bool sgbSelecting;
/** The message where the cursor is at the moment. */
static _plrmsg* sgpCurMsg;

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
		if (len <= width) {
			i++;
			continue;
		}

		if (lineStart != 0) {
			text[i] = '\0';
			break; // more than one line break -> skip the rest
		}

		// add line-break on blank glyph
		if (c != 0) {
			// Scan for previous blank glyph
			int j = i;
			while (--j >= 0) {
				c = gbStdFontFrame[(BYTE)text[j]];
				if (c == 0) {
					i = j + 1; // start the new line after the blank glyph
					break;
				}
			}
		}

		lineStart = i;
		len = 0;
	}

	pMsg->lineBreak = lineStart;
}

static _plrmsg* AddPlrMsg(int pnum)
{
	_plrmsg* pMsg = &plr_msgs[plr_msg_slot];

	static_assert((PLRMSG_COUNT & (PLRMSG_COUNT - 1)) == 0, "Modulo to BitAnd optimization requires a power of 2.");
	plr_msg_slot = (unsigned)(plr_msg_slot + 1) % PLRMSG_COUNT;
	pMsg->player = pnum;
	pMsg->time = time(NULL);
	if (pMsg == sgpCurMsg) {
		sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
	}
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
	// sguSelPos = 0;
	// plr_msgs[PLRMSG_COUNT].player = mypnum;
	// plr_msgs[PLRMSG_COUNT].str[0] = '\0';
	// sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
}

static void plrmsg_DrawSelection(int x, int y, char* text, unsigned basepos, unsigned startpos /* selpos */, unsigned endpos /* lineBreak*/)
{
	int sp, w;

	char tmp = text[startpos];
	text[startpos] = '\0';
	sp = GetSmallStringWidth(&text[basepos]);
	text[startpos] = tmp;
	tmp = text[endpos];
	text[endpos] = '\0';
	w = GetSmallStringWidth(&text[startpos]);
	text[endpos] = tmp;
	DrawRectTrans(x + sp + FONT_KERN_SMALL, y - PLRMSG_TEXT_HEIGHT, w, PLRMSG_TEXT_HEIGHT, PAL16_GRAY);
}

static void PrintPlrMsg(int x, int y, _plrmsg* pMsg, int top)
{
	BYTE c, col;
	int sx, startY, breakPos;
	unsigned curPos;
	const char* str;

	str = pMsg->str;
	col = pMsg->player == MAX_PLRS ? COL_GOLD : COL_WHITE;
	curPos = (sgpCurMsg == pMsg && ((SDL_GetTicks() / 512) % 2) != 0) ? sguCursPos : 2 * MAX_SEND_STR_LEN; // GetAnimationFrame(2, 512) != 0) {
	curPos++;

	sx = x;
	breakPos = pMsg->lineBreak;
	if (breakPos != 0) {
		y -= PLRMSG_TEXT_HEIGHT;
		// skip the first line if it does not fit
		if (y < top) {
			y += PLRMSG_TEXT_HEIGHT;
			str = &pMsg->str[breakPos];
			breakPos = 0;
		}
	}
	startY = y;

	while (true) {
		// render the cursor
		if (--curPos == 0) {
			PrintSmallChar(sx - 1, y, '|', col); // - smallFontWidth[gbStdFontFrame['|']] / 2
		}
		c = (BYTE)*str++;
		if (c == '\0') {
			break;
		}
		// render the text
		sx += PrintSmallChar(sx, y, c, col);
		if (--breakPos == 0) {
			y += PLRMSG_TEXT_HEIGHT;
			sx = x;
		}
	}
	// render the selection
	if (sgpCurMsg == pMsg) {
		char* text = pMsg->str;
		unsigned curpos = sguCursPos;
		unsigned selpos = sguSelPos;
		if (selpos != curpos) {
			if (selpos > curpos) {
				std::swap(selpos, curpos);
			}
			y = startY;
			unsigned basepos = 0;
			unsigned lineBreak = pMsg->lineBreak;
			if (lineBreak != 0) {
				if (curpos >= lineBreak) {
					if (selpos < lineBreak) {
						// selection in two lines
						plrmsg_DrawSelection(x, y, text, basepos, selpos, lineBreak);

						selpos = lineBreak;
						basepos = lineBreak;
						y += PLRMSG_TEXT_HEIGHT;
					} else {
						// selection in the second line
						basepos = lineBreak;
						y += PLRMSG_TEXT_HEIGHT;
					}
				} else {
					// selection in the first line
				}
			}

			plrmsg_DrawSelection(x, y, text, basepos, selpos, curpos);
		}
	}
}

void DrawPlrMsg(bool onTop)
{
	if (onTop != gbTalkflag)
		return;

	int msgs[PLRMSG_COUNT], nummsgs, numlines;
	int i, idx, x, y, h, linelimit, top;
	const int width = PLRMSG_WIDTH;
	uint32_t timeout;

	// collect the messages
	nummsgs = 0;
	numlines = 0;
	if (!gbTalkflag) {
		timeout = time(NULL) - PLRMSG_TEXT_TIMEOUT;
		linelimit = 3;
	} else {
		numlines += plr_msgs[PLRMSG_COUNT].lineBreak != 0 ? 2 : 1;
		msgs[0] = PLRMSG_COUNT;
		nummsgs = 1;
		timeout = 0;
		linelimit = (PLRMSG_TEXT_BOTTOM - PLRMSG_TEXT_TOP - 2 * PLRMSG_PANEL_BORDER) / PLRMSG_TEXT_HEIGHT;
	}

	for (i = 1; i <= PLRMSG_COUNT; i++) {
		idx = (unsigned)(plr_msg_slot - i) % PLRMSG_COUNT;
		if (plr_msgs[idx].str[0] != '\0' && plr_msgs[idx].time >= timeout) {
			numlines += plr_msgs[idx].lineBreak != 0 ? 2 : 1;
			msgs[nummsgs] = idx;
			nummsgs++;

			if (numlines >= linelimit) {
				numlines = linelimit;
				break;
			}
		}
	}

	if (nummsgs == 0) {
		return;
	}

	x = PLRMSG_TEXT_X;
	y = PLRMSG_TEXT_BOTTOM;
	// render the background
	h = numlines * PLRMSG_TEXT_HEIGHT + 2 * PLRMSG_PANEL_BORDER;
	top = y - (h - PLRMSG_PANEL_BORDER);

	DrawRectTrans(x - PLRMSG_PANEL_BORDER, top, width + 2 * PLRMSG_PANEL_BORDER, h, PAL_BLACK);

	// render the messages
	top += PLRMSG_PANEL_BORDER + PLRMSG_TEXT_HEIGHT;

	for (i = 0; i < nummsgs; i++) {
		_plrmsg* msg = &plr_msgs[msgs[i]];
		PrintPlrMsg(x, y, msg, top);
		y -= (msg->lineBreak ? 2 : 1) * PLRMSG_TEXT_HEIGHT;
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
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
	sgbTalkSavePos = sgbNextTalkSave;
	sguCursPos = 0;
	sguSelPos = 0;
	sgbSelecting = false;
	sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
}

/*
 * @brief Setup a new chat message.
 *   If shift is pressed:  the message is prepared to be sent to the whole team of the player
 *            is released: the message is prepared to be sent to the given player
 */
void SetupPlrMsg(int pnum)
{
	const char* text;
	int param;

	if (!gbTalkflag)
		StartPlrMsg();
	if (!(gbModBtnDown & ACTBTN_MASK(ACT_MODACT))) {
		text = "/p%d ";
		param = pnum;
	} else {
		text = "/t%d ";
		param = plr._pTeam;
	}
	int len = snprintf(plr_msgs[PLRMSG_COUNT].str, sizeof(plr_msgs[PLRMSG_COUNT].str), text, param);
	sguCursPos = len;
	sguSelPos = len;
	// plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
	plr_msgs[PLRMSG_COUNT].lineBreak = 0;
	sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
}

/*
 * @brief Show information about the current game to the player.
 *   If shift is pressed:  the difficulty of the game is shown to the player
 *            is released: the name and the password is shown to the player in multiplayer games
 */
void VersionPlrMsg()
{
	EventPlrMsg(gszProductName);
	if (!(gbModBtnDown & ACTBTN_MASK(ACT_MODACT))) {
		if (!IsLocalGame) {
			const char *szGameName, *szGamePassword;
			SNetGetGameInfo(&szGameName, &szGamePassword);
			EventPlrMsg(szGameName);
			if (szGamePassword[0] != '\0') {
				char desc[sizeof("password: %s") + NET_MAX_PASSWD_LEN];
				snprintf(desc, sizeof(desc), "password: %s", szGamePassword);
				EventPlrMsg(desc);
			}
		}
	} else {
		const char* difficulties[3] = { "Normal", "Nightmare", "Hell" };
		EventPlrMsg(difficulties[gnDifficulty]);
	}
}

void StopPlrMsg()
{
	gbTalkflag = false;
	SDL_StopTextInput();
	// gbRedrawFlags |= REDRAW_DRAW_ALL;
	// sguCursPos = 0;
	// sguSelPos = 0;
	// sgbSelecting = false;
	sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
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
		TMsgString msgStr;
		int len = SStrCopy(msgStr.str, msg, sizeof(msgStr.str));
		msgStr.bsLen = len;
		NetSendCmdString(&msgStr, pmask);

		for (i = 0; i < lengthof(sgszTalkSave); i++) {
			if (!SDL_strcmp(sgszTalkSave[i], &plr_msgs[PLRMSG_COUNT].str[0]))
				break;
		}
		static_assert(lengthof(sgszTalkSave) == 8, "Table sgszTalkSave does not work in SendPlrMsg.");
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
		sguSelPos = 0;
		// plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
		plr_msgs[PLRMSG_COUNT].lineBreak = 0;
		sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
		sgbTalkSavePos = sgbNextTalkSave;
	} else {
		StopPlrMsg();
	}
}
#ifndef USE_SDL1
static bool plrmsg_CopyToClipboard()
{
	unsigned sp = sguSelPos;
	unsigned cp = sguCursPos;
	if (sp == cp) {
		return false;
	}
	if (sp > cp) {
		std::swap(sp, cp);
	}
	char tmp = sgpCurMsg->str[cp];
	sgpCurMsg->str[cp] = '\0';
	char* output = latin1_to_utf8(&sgpCurMsg->str[sp]);
	sgpCurMsg->str[cp] = tmp;
	SDL_SetClipboardText(output);
	SDL_free(output);
	return true;
}
#endif
void plrmsg_CatToText(const char* inBuf)
{
	// assert(gbTalkflag);
	// assert(!IsLocalGame);

	if (sgpCurMsg != &plr_msgs[PLRMSG_COUNT]) {
		return;
	}
	char* output = utf8_to_latin1(inBuf);
	unsigned cp = sguCursPos;
	unsigned sp = sguSelPos;
	if (sp > cp) {
		std::swap(sp, cp);
	}
	char* text = plr_msgs[PLRMSG_COUNT].str;
	const unsigned maxlen = MAX_SEND_STR_LEN;
	char tmpstr[MAX_SEND_STR_LEN];
	SStrCopy(tmpstr, &text[cp], std::min((unsigned)sizeof(tmpstr) - 1, maxlen - cp));
	int len = SStrCopy(&text[sp], output, maxlen - sp);
	SDL_free(output);
	// assert(strlen(text) == len + sp);
	sp += len;
	sguCursPos = sp;
	sguSelPos = sp;
	SStrCopy(&text[sp], tmpstr, maxlen - sp);

	plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
}

static void plrmsg_DelFromText(bool back)
{
	if (sgpCurMsg != &plr_msgs[PLRMSG_COUNT]) {
		return;
	}

	char* text = plr_msgs[PLRMSG_COUNT].str;
	unsigned max_length = MAX_SEND_STR_LEN;

	int w = sguCursPos - sguSelPos;
	if (w != 0) {
		if (w < 0) {
			w = -w;
			sguSelPos = sguCursPos;
		} else {
			sguCursPos = sguSelPos;
		}
	} else {
		w = 1;
		if (back) {
			unsigned i = sguCursPos;
			if (i == 0) {
				return;
			}
			i--;
			sguCursPos = i;
			sguSelPos = i;
		}
	}

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
			sguSelPos = len;
			sgpCurMsg = &plr_msgs[PLRMSG_COUNT];
			memcpy(plr_msgs[PLRMSG_COUNT].str, sgszTalkSave[sgbTalkSavePos], len + 1);
			plrmsg_WordWrap(&plr_msgs[PLRMSG_COUNT]);
			return;
		}
	}
}

static unsigned plrmsg_CursPos(int x, int y)
{
	// x -= (PLRMSG_TEXT_X - SCREEN_X);
	// y = (PLRMSG_TEXT_BOTTOM - SCREEN_Y) - y;
	char* text = sgpCurMsg->str;
	unsigned curpos = 0;
	if (sgpCurMsg->lineBreak != 0 && y < PLRMSG_TEXT_HEIGHT) {
		curpos = sgpCurMsg->lineBreak;
	}
	while (true) {
		char tmp = text[curpos];
		if (tmp == '\0') {
			break;
		}
		BYTE w = smallFontWidth[gbStdFontFrame[tmp]];
		x -= w + FONT_KERN_SMALL;
		if (x <= 0) {
			if (-x < (w + FONT_KERN_SMALL) / 2) {
				curpos++;
			}
			break;
		}
		curpos++;
		if (curpos == sgpCurMsg->lineBreak) {
			curpos--;
			break;
		}
	}
	return curpos;
}

static _plrmsg* plrmsg_msgAt(int& y)
{
	int h, i;
	_plrmsg *result = &plr_msgs[PLRMSG_COUNT];

	h = (result->lineBreak != 0 ? 2 : 1) * PLRMSG_TEXT_HEIGHT;
	if (y < h) {
		return result;
	}
	y -= h;

	for (i = 1; i <= PLRMSG_COUNT; i++) {
		int idx = (unsigned)(plr_msg_slot - i) % PLRMSG_COUNT;
		result = &plr_msgs[idx];
		if (result->str[0] != '\0' /* && plr_msgs[idx].time >= timeout */) {
			h = (result->lineBreak != 0 ? 2 : 1) * PLRMSG_TEXT_HEIGHT;
			if (y < h) {
				return result;
			}
			y -= h;
		}
	}

	return NULL;
}

static bool plrmsg_HandleMouseEvent(SDL_Keymod mod)
{
	int x, y;
	_plrmsg *tMsg;

	x = MousePos.x - (PLRMSG_TEXT_X - SCREEN_X);
	if (x < 0 || x >= PLRMSG_WIDTH) {
		return false;
	}
	y = (PLRMSG_TEXT_BOTTOM - SCREEN_Y) - MousePos.y;
	if (y < 0) {
		return false;
	}

	tMsg = plrmsg_msgAt(y);
	if (tMsg == NULL) {
		return false;
	}

	sgpCurMsg = tMsg;
	sguCursPos = plrmsg_CursPos(x, y);
	if (!(mod & KMOD_SHIFT)) {
		sguSelPos = sguCursPos;
	}
	sgbSelecting = true;
	return true;
}

bool plrmsg_presskey(int vkey)
{
	// assert(gbTalkflag);
	// assert(!IsLocalGame);

	SDL_Keymod mod = SDL_GetModState();
	switch (vkey) {
#ifndef USE_SDL1
	case DVL_VK_V:
		if (!(mod & KMOD_CTRL)) {
			break;
		}
		// fall-through
	case DVL_VK_MBUTTON: {
		char* clipboard = SDL_GetClipboardText();
		if (clipboard != NULL) {
			plrmsg_CatToText(clipboard);
			SDL_free(clipboard);
		}
	} break;
	case DVL_VK_C:
	case DVL_VK_X:
		if (!(mod & KMOD_CTRL)) {
			break;
		}
		if (!plrmsg_CopyToClipboard()) {
			break;
		}
		if (vkey == DVL_VK_C) {
			break;
		}
		// fall-through
#endif
	case DVL_VK_BACK:
		plrmsg_DelFromText(true);
		break;
	case DVL_VK_DELETE:
		plrmsg_DelFromText(false);
		break;
	case DVL_VK_LEFT: {
		unsigned pos = sguCursPos;
		if (pos > 0) {
			sguCursPos = pos - 1;
			if (!(mod & KMOD_SHIFT)) {
				sguSelPos = pos - 1;
			}
		}
	} break;
	case DVL_VK_RIGHT: {
		unsigned pos = sguCursPos;
		if (sgpCurMsg->str[pos] != '\0' && pos + 1 < MAX_SEND_STR_LEN) {
			sguCursPos = pos + 1;
			if (!(mod & KMOD_SHIFT)) {
				sguSelPos = pos + 1;
			}
		}
	} break;
	case DVL_VK_HOME: {
		sguCursPos = 0;
		if (!(mod & KMOD_SHIFT)) {
			sguSelPos = 0;
		}
	} break;
	case DVL_VK_END: {
		unsigned pos = strlen(sgpCurMsg->str);
		sguCursPos = pos;
		if (!(mod & KMOD_SHIFT)) {
			sguSelPos = pos;
		}
	} break;
	case DVL_VK_DOWN:
		plrmsg_up_down(1);
		break;
	case DVL_VK_UP:
		plrmsg_up_down(-1);
		break;
	case DVL_VK_LBUTTON:
#if HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
		if (sgbControllerActive) {
			return false;
		}
#endif
		return plrmsg_HandleMouseEvent(mod);
	case DVL_VK_RBUTTON:
		return false;
	case DVL_VK_RETURN:
		SendPlrMsg();
		break;
	case DVL_VK_ESCAPE:
		if (sguCursPos != sguSelPos) {
			sguSelPos = sguCursPos;
		} else {
			StopPlrMsg();
		}
		break;
#ifdef USE_SDL1
	default:
		// SDL1 does not support TEXTINPUT events, so we need to handle them here.
		if ((mod & KMOD_CTRL) == 0) {
			vkey = TranslateKey2Char(vkey);
			char utf8[2];
			utf8[0] = (char)vkey;
			utf8[1] = '\0';
			plrmsg_CatToText(utf8);
		}
		break;
#endif
	}
	return true;
}

void plrmsg_HandleMouseReleaseEvent()
{
	sgbSelecting = false;
}

void plrmsg_HandleMouseMoveEvent()
{
	if (sgbSelecting) {
		int x = MousePos.x;
		int y = MousePos.y;

		x -= (PLRMSG_TEXT_X - SCREEN_X);
		y = (PLRMSG_TEXT_BOTTOM - SCREEN_Y) - y;

		plrmsg_msgAt(y);

		unsigned curpos = plrmsg_CursPos(x, y);
		sguCursPos = curpos;
		// return true;
	}
	// return false;
}

DEVILUTION_END_NAMESPACE
