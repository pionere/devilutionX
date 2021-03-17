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
	"$Default Keyboard Shortcuts|"
	"F1:  Open Help Screen|"
	"Esc: Display Main Menu|"
	"Tab: Display Auto-map|"
	"L: Open SkillList|"
	"B: Open SpellBook|"
	"I: Open Inventory screen|"
	"C: Open Character screen|"
	"U: Open Quest log|"
	"T: Toggle permanent tooltips|"
	"G: Increase brightness|"
	"H: Reduce brightness|"
	"Z: Zoom Game Screen|"
	" + - : Zoom Automap|"
	"1 - 8: Use Belt item|"
	"Space: Hide all info screens|"
	"Q/W/E/R: Hotkey for RMB-skill/spell|"
	"A/S/D/F: Hotkey for LMB-skill/spell|"
	"Shift+Left-Click: Attack without moving|"
	"|"
	"$Movement|"
	"If you hold the mouse button down while moving, the character "
	"will continue to move in that direction.|"
	"|"
	"$Combat|"
	"You can attack without moving by holding down the shift key.|"
	"|"
	"$Auto-map|"
	"Besides the hotkey, auto-map can be accessed in the menu."
	"You can zoom in and out of the map or scroll "
	"it with the configured keys (arrows by default).|"
	"|"
	"$Picking up Objects|"
	"Useable items such as potions or scrolls are automatically placed "
	"in your 'belt' located on the left side above the menu button. "
	"Items in belt may be used by either pressing "
	"the corresponding number or right-clicking on the item.|"
	"|"
	"$Gold|"
	"You can select a specific amount of gold to drop by "
	"right-clicking on a pile of gold in your inventory.|"
	"|"
	"$Skills & Spells|"
	"You can access your list of skills in the menu as well. Learned "
	"skills and those available through staff, scrolls or runes are listed here. "
	"Left- or right-clicking on the icon of the skill will select it for "
	"the corresponing mouse button. By holding the SHIFT the skill is added "
	"(one movement skill and one attack skill can be selected for each action button) "
	"otherwise it is set.|"
	"|"
	"$Using the SkillList|"
	"This is the complete list of available skills and spells to the character. "
	"Select one as described in the section above.|"
	"|"
	"$Setting Skill Hotkeys|"
	"You can assign up to four Hotkeys for skills/spells for both action buttons. "
	"Open the SkillList then press the Hotkey you wish "
	"to assign after highlighting the skill.|"
	"|"
	"$Skill Books|"
	"Reading more than one book increases your knowledge of that "
	"skill, allowing you to use it more effectively.|"
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
