/**
 * @file help.cpp
 *
 * Implementation of the in-game help text.
 */
#include "all.h"

DEVILUTION_BEGIN_NAMESPACE

bool gbHelpflag;
static char** gbHelpLines;
static int helpFirstLine;

/*static const char gszHelpText[] = {
	// clang-format off
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
	"Besides the hotkey, auto-map can be accessed in the menu. You "
	"can zoom in and out of the map or scroll it with the "
	"configured keys (arrows by default).|"
	"|"
	"$Picking up Objects|"
	"Useable items such as potions or scrolls are automatically "
	"placed in your 'belt' located on the left side above the menu "
	"button. Items in belt may be used by either pressing the "
	"corresponding number or right-clicking on the item.|"
	"|"
	"$Gold|"
	"You can select a specific amount of gold to drop by "
	"right-clicking on a pile of gold in your inventory.|"
	"|"
	"$Skills & Spells|"
	"You can access your list of skills in the menu as well. Learned "
	"skills and those available through staff, scrolls or runes are "
	"listed here. Left- or right-clicking on the icon of the skill will "
	"select it for the corresponing mouse button. By holding the SHIFT "
	"the skill is added (one movement skill and one attack skill can be "
	"selected for each action button) otherwise it is set.|"
	"|"
	"$Using the SkillList|"
	"This is the complete list of available skills and spells to the "
	"character. Select one as described in the section above.|"
	"|"
	"$Setting Skill Hotkeys|"
	"You can assign up to four Hotkeys for skills/spells for both "
	"action buttons. Open the SkillList then press the Hotkey you wish "
	"to assign after highlighting the skill.|"
	"|"
	"$Skill Books|"
	"Reading more than one book increases your knowledge of that "
	"skill, allowing you to use it more effectively."
	// clang-format on
};*/

//void InitHelp()
//{
//	gbHelpflag = false;
//}

void DrawHelp()
{
	int i;
	BYTE col;
	const char* s;

	DrawTextBox(1);

	PrintSString(0, 1, true, HELP_TITLE, COL_GOLD);

	for (i = 5; i < 22; i++) {
		s = gbHelpLines[helpFirstLine + i - 5];
		if (*s == '$') {
			s++;
			col = COL_RED;
		} else {
			col = COL_WHITE;
		}
		PrintSString(0, i, false, s, col);
	}
	static_assert(STORE_LINES > 23, "Help text must fit to the store lines.");
	PrintSString(0, 23, true, "Press ESC to end or the arrow keys to scroll.", COL_GOLD);
}

void StartHelp()
{
	gbHelpflag = true;
	helpFirstLine = 0;
	gbHelpLines = LoadTxtFile(HELP_TXT, HELP_LINE_COUNT);

	InitSTextHelp();
}

void StopHelp()
{
	if (!gbHelpflag)
		return;

	gbHelpflag = false;
	MemFreeTxtFile(gbHelpLines);
}

void HelpScrollUp()
{
	if (helpFirstLine > 0)
		helpFirstLine--;
}

void HelpScrollDown()
{
	if (helpFirstLine < (HELP_LINE_COUNT - (22 - 5)))
		helpFirstLine++;
}

DEVILUTION_END_NAMESPACE
