/**
 * @file help.cpp
 *
 * Implementation of the in-game help text.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

int help_select_line;
bool gbHelpflag;
int HelpTop;

const char gszHelpText[] = {
	"$Default Keyboard Shortcuts:|"
	"F1:  Open Help Screen|"
	"Esc: Display Main Menu|"
	"Tab: Display Auto-map|"
	"S: Open Speedbook|"
	"B: Open Spellbook|"
	"I: Open Inventory screen|"
	"C: Open Character screen|"
	"Q: Open Quest log|"
	"T: Toggle permanent tooltips|"
	"F: Reduce screen brightness|"
	"G: Increase screen brightness|"
	"Z: Zoom Game Screen|"
	" + - : Zoom Automap|"
	"1 - 8: Use Belt item|"
	"Space: Hide all info screens|"
	"F5 - F8: Hotkey for skill/spell|"
	"Shift+Left-Click: Attack without moving|"
	"|"
	"$Movement:|"
	"If you hold the mouse button down while moving, the character "
	"will continue to move in that direction.|"
	"|"
	"$Combat:|"
	"You can attack without moving by holding down the shift key.|"
	"|"
	"$Auto-map:|"
	"To access the auto-map, click the 'MAP' button on the "
	"Interface Bar. You can zoom in and out of the map or scroll "
	"it with the configured keys (arrows by default).|"
	"|"
	"$Picking up Objects:|"
	"Useable items that are small in size, such as potions or scrolls, "
	"are automatically placed in your 'belt' located at the top of "
	"the Interface Bar. Items in belt may be used by either pressing "
	"the corresponding number or right-clicking on the item.|"
	"|"
	"$Gold|"
	"You can select a specific amount of gold to drop by "
	"right-clicking on a pile of gold in your inventory.|"
	"|"
	"$Skills & Spells:|"
	"You can access your list of skills and spells by left-clicking on "
	"the 'SPELLS' button in the interface bar. Memorized spells and "
	"those available through staffs are listed here. Left-clicking on "
	"the spell you wish to cast will ready the spell. A readied spell "
	"may be cast by right-clicking.|"
	"|"
	"$Using the Speedbook for Spells|"
	"Left-clicking on the 'readied spell' button will open the 'Speedbook' "
	"which allows you to select a skill or spell for immediate use.|"
	"|"
	"$Setting Spell Hotkeys|"
	"You can assign up to four Hotkeys for skills, spells or scrolls. "
	"Start by opening the 'speedbook' as described in the section above. "
	"Press the Hotkey (F5-F9 per default) you wish to assign after "
	"highlighting the spell.|"
	"|"
	"$Spell Books|"
	"Reading more than one book increases your knowledge of that "
	"spell, allowing you to cast the spell more effectively.|"
	"&"
};

void InitHelp()
{
	gbHelpflag = false;
}

static void DrawHelpLine(int x, int y, char *text, char color)
{
	int sx, sy, width;
	BYTE c;

	width = 0;
	sx = x + 32 + PANEL_X;
	sy = y * 12 + 44 + SCREEN_Y + UI_OFFSET_Y;
	while (*text != '\0') {
		c = gbFontTransTbl[(BYTE)*text];
		text++;
		c = fontframe[c];
		width += fontkern[c] + 1;
		if (c != '\0') {
			if (width <= 577)
				PrintChar(sx, sy, c, color);
		}
		sx += fontkern[c] + 1;
	}
}

void DrawHelp()
{
	int i, c, w;
	char col;
	const char *s;

	DrawSTextHelp();
	DrawQTextBack();
	PrintSString(0, 2, true, HELP_TITLE, COL_GOLD, 0);
	DrawSLine(5);

	s = &gszHelpText[0];

	for (i = 0; i < help_select_line; i++) {
		c = 0;
		w = 0;
		while (*s == '\0') {
			s++;
		}
		if (*s == '$') {
			s++;
		}
		if (*s == '&') {
			continue;
		}
		while (*s != '|' && w < 577) {
			while (*s == '\0') {
				s++;
			}
			tempstr[c] = *s;
			w += fontkern[fontframe[gbFontTransTbl[(BYTE)tempstr[c]]]] + 1;
			c++;
			s++;
		}
		if (w >= 577) {
			c--;
			while (tempstr[c] != ' ') {
				s--;
				c--;
			}
		}
		if (*s == '|') {
			s++;
		}
	}
	for (i = 7; i < 22; i++) {
		c = 0;
		w = 0;
		while (*s == '\0') {
			s++;
		}
		if (*s == '$') {
			s++;
			col = COL_RED;
		} else {
			col = COL_WHITE;
		}
		if (*s == '&') {
			HelpTop = help_select_line;
			continue;
		}
		while (*s != '|' && w < 577) {
			while (*s == '\0') {
				s++;
			}
			tempstr[c] = *s;
			w += fontkern[fontframe[gbFontTransTbl[(BYTE)tempstr[c]]]] + 1;
			c++;
			s++;
		}
		if (w >= 577) {
			c--;
			while (tempstr[c] != ' ') {
				s--;
				c--;
			}
		}
		if (c != 0) {
			tempstr[c] = '\0';
			DrawHelpLine(0, i, tempstr, col);
		}
		if (*s == '|') {
			s++;
		}
	}

	PrintSString(0, 23, true, "Press ESC to end or the arrow keys to scroll.", COL_GOLD, 0);
}

void DisplayHelp()
{
	help_select_line = 0;
	gbHelpflag = true;
	HelpTop = 5000;
}

void HelpScrollUp()
{
	if (help_select_line > 0)
		help_select_line--;
}

void HelpScrollDown()
{
	if (help_select_line < HelpTop)
		help_select_line++;
}

DEVILUTION_END_NAMESPACE
