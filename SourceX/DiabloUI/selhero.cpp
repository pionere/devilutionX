#include "selhero.h"

#include <chrono>

#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/scrollbar.h"
#include "DiabloUI/selyesno.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/selconn.h"

#ifdef __3DS__
#include "../platform/ctr/keyboard.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

_uiheroinfo selhero_heroInfo;

namespace {

#if defined(PREFILL_PLAYER_NAME) || defined(__3DS__) || HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
const char *selhero_GenerateName(uint8_t hero_class);
#endif

unsigned selhero_SaveCount = 0;
_uiheroinfo selhero_heros[MAX_CHARACTERS];
const unsigned kMaxViewportItems = 6;
char textStats[5][4];
char title[32];
char selhero_Lable[32];
char selhero_Description[256];
int selhero_result;
bool selhero_endMenu;
bool selhero_navigateYesNo;
bool selhero_deleteEnabled;

bool(*gfnHeroCreate)
(_uiheroinfo *);
void (*gfnHeroStats)(unsigned int, _uidefaultstats *);


std::vector<UiItemBase *> vecSelHeroDialog;
std::vector<UiListItem *> vecSelHeroDlgItems;
std::vector<UiItemBase *> vecSelDlgItems;

UiImage *SELHERO_DIALOG_HERO_IMG;
} // namespace

bool bUIElementsLoaded = false;

void selhero_UiFocusNavigationYesNo()
{
	if (selhero_deleteEnabled)
		UiFocusNavigationYesNo();
}

static void selhero_FreeListItems()
{
	for (unsigned i = 0; i < vecSelHeroDlgItems.size(); i++) {
		UiListItem *pUIItem = vecSelHeroDlgItems[i];
		delete pUIItem;
	}
	vecSelHeroDlgItems.clear();
}

static void selhero_FreeDlgItems()
{
	for (unsigned i = 0; i < vecSelDlgItems.size(); i++) {
		UiItemBase *pUIItem = vecSelDlgItems[i];
		delete pUIItem;
	}
	vecSelDlgItems.clear();
}

static void selhero_Free()
{
	ArtBackground.Unload();

	for (unsigned i = 0; i < vecSelHeroDialog.size(); i++) {
		UiItemBase *pUIItem = vecSelHeroDialog[i];
		delete pUIItem;
	}
	vecSelHeroDialog.clear();

	selhero_FreeDlgItems();
	selhero_FreeListItems();

	bUIElementsLoaded = false;
}

static void selhero_SetStats()
{
	int heroclass = selhero_heroInfo.heroclass;
#ifdef HELLFIRE
	if (heroclass == PC_BARBARIAN)
		heroclass = PC_WARRIOR; // The graphics is missing from heros.pcx
#endif
	SELHERO_DIALOG_HERO_IMG->m_frame = heroclass;
	snprintf(textStats[0], sizeof(textStats[0]), "%d", selhero_heroInfo.level);
	snprintf(textStats[1], sizeof(textStats[1]), "%d", selhero_heroInfo.strength);
	snprintf(textStats[2], sizeof(textStats[2]), "%d", selhero_heroInfo.magic);
	snprintf(textStats[3], sizeof(textStats[3]), "%d", selhero_heroInfo.dexterity);
	snprintf(textStats[4], sizeof(textStats[4]), "%d", selhero_heroInfo.vitality);
}

namespace {

UiArtTextButton *SELLIST_DIALOG_DELETE_BUTTON;

} // namespace

static void selhero_UpdateViewportItems()
{
	const unsigned num_viewport_heroes = std::min(selhero_SaveCount - ListOffset, kMaxViewportItems);
	for (unsigned i = 0; i < num_viewport_heroes; i++) {
		const unsigned index = i + ListOffset;
		vecSelHeroDlgItems[i]->m_text = selhero_heros[index].name;
		vecSelHeroDlgItems[i]->m_value = index;
	}
	if (num_viewport_heroes < kMaxViewportItems) {
		vecSelHeroDlgItems[num_viewport_heroes]->m_text = "New Hero";
		vecSelHeroDlgItems[num_viewport_heroes]->m_value = selhero_SaveCount;
	}
}

static void selhero_ScrollIntoView(unsigned index)
{
	unsigned new_offset = ListOffset;
	if (index >= ListOffset + kMaxViewportItems)
		new_offset = index - (kMaxViewportItems - 1);
	if (index < ListOffset)
		new_offset = index;
	if (new_offset != ListOffset) {
		ListOffset = new_offset;
		selhero_UpdateViewportItems();
	}
}

void selhero_Init()
{
	LoadBackgroundArt("ui_art\\selhero.pcx");
	UiAddBackground(&vecSelHeroDialog);
	UiAddLogo(&vecSelHeroDialog);
	LoadScrollBar();

	selhero_FreeDlgItems();
	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelHeroDialog.push_back(new UiArtText(title, rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 30, (UI_OFFSET_Y + 211), 180, 76 };
#ifdef HELLFIRE
	SELHERO_DIALOG_HERO_IMG = new UiImage(&ArtHero, 5, rect2);
#else
	SELHERO_DIALOG_HERO_IMG = new UiImage(&ArtHero, NUM_CLASSES, rect2);
#endif
	vecSelHeroDialog.push_back(SELHERO_DIALOG_HERO_IMG);

	SDL_Rect rect4 = { PANEL_LEFT + 39, (UI_OFFSET_Y + 323), 110, 21 };
	vecSelHeroDialog.push_back(new UiArtText("Level:", rect4, UIS_RIGHT));
	SDL_Rect rect5 = { PANEL_LEFT + 159, (UI_OFFSET_Y + 323), 40, 21 };
	vecSelHeroDialog.push_back(new UiArtText(textStats[0], rect5, UIS_CENTER));

	SDL_Rect rect6 = { PANEL_LEFT + 39, (UI_OFFSET_Y + 358), 110, 21 };
	vecSelHeroDialog.push_back(new UiArtText("Strength:", rect6, UIS_RIGHT));
	SDL_Rect rect7 = { PANEL_LEFT + 159, (UI_OFFSET_Y + 358), 40, 21 };
	vecSelHeroDialog.push_back(new UiArtText(textStats[1], rect7, UIS_CENTER));

	SDL_Rect rect8 = { PANEL_LEFT + 39, (UI_OFFSET_Y + 380), 110, 21 };
	vecSelHeroDialog.push_back(new UiArtText("Magic:", rect8, UIS_RIGHT));
	SDL_Rect rect9 = { PANEL_LEFT + 159, (UI_OFFSET_Y + 380), 40, 21 };
	vecSelHeroDialog.push_back(new UiArtText(textStats[2], rect9, UIS_CENTER));

	SDL_Rect rect10 = { PANEL_LEFT + 39, (UI_OFFSET_Y + 401), 110, 21 };
	vecSelHeroDialog.push_back(new UiArtText("Dexterity:", rect10, UIS_RIGHT));
	SDL_Rect rect11 = { PANEL_LEFT + 159, (UI_OFFSET_Y + 401), 40, 21 };
	vecSelHeroDialog.push_back(new UiArtText(textStats[3], rect11, UIS_CENTER));

	SDL_Rect rect12 = { PANEL_LEFT + 39, (UI_OFFSET_Y + 422), 110, 21 };
	vecSelHeroDialog.push_back(new UiArtText("Vitality:", rect12, UIS_RIGHT));
	SDL_Rect rect13 = { PANEL_LEFT + 159, (UI_OFFSET_Y + 422), 40, 21 };
	vecSelHeroDialog.push_back(new UiArtText(textStats[4], rect13, UIS_CENTER));
}

void selhero_List_Init()
{
	selhero_FreeDlgItems();

	SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
	vecSelDlgItems.push_back(new UiArtText("Select Hero", rect1, UIS_CENTER | UIS_BIG));

	selhero_FreeListItems();
	unsigned num_viewport_heroes = std::min(selhero_SaveCount + 1, kMaxViewportItems);
	for (unsigned i = 0; i < num_viewport_heroes; i++) {
		vecSelHeroDlgItems.push_back(new UiListItem("", -1));
	}
	selhero_UpdateViewportItems();

	vecSelDlgItems.push_back(new UiList(vecSelHeroDlgItems, PANEL_LEFT + 265, (UI_OFFSET_Y + 256), 320, 26, UIS_CENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect2 = { PANEL_LEFT + 585, (UI_OFFSET_Y + 244), 25, 178 };
	UiScrollBar *scrollBar = new UiScrollBar(&ArtScrollBarBackground, &ArtScrollBarThumb, &ArtScrollBarArrow, rect2);
	vecSelDlgItems.push_back(scrollBar);

	SDL_Rect rect3 = { PANEL_LEFT + 239, (UI_OFFSET_Y + 429), 120, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { PANEL_LEFT + 364, (UI_OFFSET_Y + 429), 120, 35 };
	SELLIST_DIALOG_DELETE_BUTTON = new UiArtTextButton("Delete", &selhero_UiFocusNavigationYesNo, rect4, UIS_CENTER | UIS_BIG | UIS_DISABLED);
	vecSelDlgItems.push_back(SELLIST_DIALOG_DELETE_BUTTON);

	SDL_Rect rect5 = { PANEL_LEFT + 489, (UI_OFFSET_Y + 429), 120, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect5, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelDlgItems, selhero_SaveCount + 1, selhero_List_Focus, selhero_List_Select, selhero_List_Esc, selhero_List_DeleteYesNo, false);
	UiInitScrollBar(scrollBar, kMaxViewportItems);
	snprintf(title, sizeof(title), "%s Player Characters", selconn_bMulti ? "Multi" : "Single");
}

void selhero_List_Focus(unsigned index)
{
	selhero_ScrollIntoView(index);
	int baseFlags = UIS_CENTER | UIS_BIG;
	if (selhero_SaveCount && index < selhero_SaveCount) {
		memcpy(&selhero_heroInfo, &selhero_heros[index], sizeof(selhero_heroInfo));
		selhero_SetStats();
		SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_GOLD;
		selhero_deleteEnabled = true;
		return;
	}

#ifdef HELLFIRE
	SELHERO_DIALOG_HERO_IMG->m_frame = 5;
#else
	SELHERO_DIALOG_HERO_IMG->m_frame = NUM_CLASSES;
#endif
	snprintf(textStats[0], sizeof(textStats[0]), "--");
	snprintf(textStats[1], sizeof(textStats[1]), "--");
	snprintf(textStats[2], sizeof(textStats[2]), "--");
	snprintf(textStats[3], sizeof(textStats[3]), "--");
	snprintf(textStats[4], sizeof(textStats[4]), "--");
	SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_DISABLED;
	selhero_deleteEnabled = false;
}

bool selhero_List_DeleteYesNo()
{
	selhero_navigateYesNo = selhero_deleteEnabled;

	return selhero_navigateYesNo;
}

void selhero_List_Select(unsigned index)
{
	if (index == selhero_SaveCount) {
		selhero_FreeDlgItems();

		SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
		vecSelDlgItems.push_back(new UiArtText("Choose Class", rect1, UIS_CENTER | UIS_BIG));

		selhero_FreeListItems();
		int itemH = 33;
		vecSelHeroDlgItems.push_back(new UiListItem("Warrior", PC_WARRIOR));
		vecSelHeroDlgItems.push_back(new UiListItem("Rogue", PC_ROGUE));
		vecSelHeroDlgItems.push_back(new UiListItem("Sorcerer", PC_SORCERER));
#ifdef HELLFIRE
		vecSelHeroDlgItems.push_back(new UiListItem("Monk", PC_MONK));
		vecSelHeroDlgItems.push_back(new UiListItem("Bard", PC_BARD));
		vecSelHeroDlgItems.push_back(new UiListItem("Barbarian", PC_BARBARIAN));

		itemH = 26;
#endif
		int itemY = 246 + (176 - vecSelHeroDlgItems.size() * itemH) / 2;
		vecSelDlgItems.push_back(new UiList(vecSelHeroDlgItems, PANEL_LEFT + 264, (UI_OFFSET_Y + itemY), 320, itemH, UIS_CENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect2 = { PANEL_LEFT + 279, (UI_OFFSET_Y + 429), 140, 35 };
		vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect2, UIS_CENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect3 = { PANEL_LEFT + 429, (UI_OFFSET_Y + 429), 140, 35 };
		vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

		UiInitList(vecSelDlgItems, vecSelHeroDlgItems.size(), selhero_ClassSelector_Focus, selhero_ClassSelector_Select, selhero_ClassSelector_Esc);
		memset(&selhero_heroInfo.name, 0, sizeof(selhero_heroInfo.name));
		snprintf(title, sizeof(title), "New %s Player Hero", selconn_bMulti ? "Multi" : "Single");
		return;
	}

	if (selhero_heroInfo.hassaved) {
		selhero_FreeDlgItems();

		SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
		vecSelDlgItems.push_back(new UiArtText("Save File Exists", rect1, UIS_CENTER | UIS_BIG));

		selhero_FreeListItems();
		vecSelHeroDlgItems.push_back(new UiListItem("Load Game", 0));
		vecSelHeroDlgItems.push_back(new UiListItem("New Game", 1));
		vecSelDlgItems.push_back(new UiList(vecSelHeroDlgItems, PANEL_LEFT + 265, (UI_OFFSET_Y + 285), 320, 33, UIS_CENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect2 = { PANEL_LEFT + 279, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect2, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect3 = { PANEL_LEFT + 429, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect3, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		UiInitList(vecSelDlgItems, vecSelHeroDlgItems.size(), selhero_Load_Focus, selhero_Load_Select, selhero_List_Init, NULL, true);
		snprintf(title, sizeof(title), "Single Player Characters");
		return;
	}

	selhero_Load_Select(1);
}

void selhero_List_Esc()
{
	UiInitList_clear();

	selhero_endMenu = true;
	selhero_result = SELHERO_PREVIOUS;
}

void selhero_ClassSelector_Focus(unsigned index)
{
	_uidefaultstats defaults;
	gfnHeroStats(index, &defaults);

	selhero_heroInfo.level = 1;
	selhero_heroInfo.heroclass = vecSelHeroDlgItems[index]->m_value;
	selhero_heroInfo.strength = defaults.strength;
	selhero_heroInfo.magic = defaults.magic;
	selhero_heroInfo.dexterity = defaults.dexterity;
	selhero_heroInfo.vitality = defaults.vitality;

	selhero_SetStats();
}

void selhero_ClassSelector_Select(unsigned index)
{
	int hClass = vecSelHeroDlgItems[index]->m_value;

	snprintf(title, sizeof(title), "New %s Player Hero", selconn_bMulti ? "Multi" : "Single");
	memset(selhero_heroInfo.name, '\0', sizeof(selhero_heroInfo.name));
#if defined __3DS__
	ctr_vkbdInput("Enter Hero name..", selhero_GenerateName(selhero_heroInfo.heroclass), selhero_heroInfo.name);
#elif defined(PREFILL_PLAYER_NAME) || HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
#ifndef PREFILL_PLAYER_NAME
	if (sgbControllerActive)
#endif
		SStrCopy(selhero_heroInfo.name, selhero_GenerateName(selhero_heroInfo.heroclass), sizeof(selhero_heroInfo.name));
#endif
	selhero_FreeDlgItems();
	SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
	vecSelDlgItems.push_back(new UiArtText("Enter Name", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 265, (UI_OFFSET_Y + 317), 320, 33 };
	vecSelDlgItems.push_back(new UiEdit(selhero_heroInfo.name, 15, rect2, UIS_MED | UIS_GOLD));

	SDL_Rect rect3 = { PANEL_LEFT + 279, (UI_OFFSET_Y + 429), 140, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { PANEL_LEFT + 429, (UI_OFFSET_Y + 429), 140, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelDlgItems, 0, NULL, selhero_Name_Select, selhero_Name_Esc);
}

void selhero_ClassSelector_Esc()
{
	selhero_FreeDlgItems();
	selhero_FreeListItems();

	if (selhero_SaveCount) {
		selhero_List_Init();
		return;
	}

	selhero_List_Esc();
}

void selhero_Name_Select(unsigned index)
{

	if (!UiValidPlayerName(selhero_heroInfo.name)) {
		ArtBackground.Unload();
		UiSelOkDialog(title, "Invalid name. A name cannot contain spaces, reserved characters, or reserved words.\n", false);
		LoadBackgroundArt("ui_art\\selhero.pcx");
	} else {
		bool overwrite = true;
		for (unsigned i = 0; i < selhero_SaveCount; i++) {
			if (strcasecmp(selhero_heros[i].name, selhero_heroInfo.name) == 0) {
				ArtBackground.Unload();
				char dialogText[256];
				snprintf(dialogText, sizeof(dialogText), "Character already exists. Do you want to overwrite \"%s\"?", selhero_heroInfo.name);
				overwrite = UiSelHeroYesNoDialog(title, dialogText);
				LoadBackgroundArt("ui_art\\selhero.pcx");
				break;
			}
		}

		if (overwrite) {
			if (gfnHeroCreate(&selhero_heroInfo)) {
				selhero_Load_Select(1);
				return;
			} else {
				UiErrorOkDialog("Unable to create character.", vecSelDlgItems);
			}
		}
	}

	selhero_ClassSelector_Select(0);
}

void selhero_Name_Esc()
{
	selhero_List_Select(selhero_SaveCount);
}

void selhero_Load_Focus(unsigned index)
{
}

void selhero_Load_Select(unsigned index)
{
	UiInitList_clear();
	selhero_endMenu = true;
	selhero_result = index == 0 ? SELHERO_CONTINUE : SELHERO_NEW_DUNGEON;
}

void SelHero_GetHeroInfo(_uiheroinfo *pInfo)
{
	selhero_heros[selhero_SaveCount] = *pInfo;
	selhero_SaveCount++;
}

int UiSelHeroDialog(void (*fninfo)(void (*fninfofunc)(_uiheroinfo *)),
	bool (*fncreate)(_uiheroinfo *),
	void (*fnremove)(_uiheroinfo *),
	void (*fnstats)(unsigned int, _uidefaultstats *),
	char (&name)[16])
{
	bUIElementsLoaded = true;

	do {
		selhero_Init();

		gfnHeroCreate = fncreate;
		gfnHeroStats = fnstats;
		//selhero_result = 0;

		selhero_navigateYesNo = false;

		selhero_SaveCount = 0;
		fninfo(SelHero_GetHeroInfo);
		std::reverse(selhero_heros, selhero_heros + selhero_SaveCount);

		if (selhero_SaveCount != 0) {
			selhero_List_Init();
		} else {
			selhero_List_Select(0);
		}

		selhero_endMenu = false;
		while (!selhero_endMenu && !selhero_navigateYesNo) {
			UiClearScreen();
			UiRenderItems(vecSelHeroDialog);
			UiPollAndRender();
		}
		selhero_Free();

		if (selhero_navigateYesNo) {
			char dialogTitle[32];
			char dialogText[256];
			snprintf(dialogTitle, sizeof(dialogTitle), "Delete %s Player Hero", selconn_bMulti ? "Multi" : "Single");
			snprintf(dialogText, sizeof(dialogText), "Are you sure you want to delete the character \"%s\"?", selhero_heroInfo.name);

			if (UiSelHeroYesNoDialog(dialogTitle, dialogText))
				fnremove(&selhero_heroInfo);
		}
	} while (selhero_navigateYesNo);

	copy_str(name, selhero_heroInfo.name);

	UnloadScrollBar();
	return selhero_result;
}

#if defined(PREFILL_PLAYER_NAME) || defined(__3DS__) || HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
const char *selhero_GenerateName(uint8_t hero_class)
{
	static const char *const kNames[NUM_CLASSES][10] = {
		{ // WARRIOR
		    "Aidan",
		    "Qarak",
		    "Born",
		    "Cathan",
		    "Halbu",
		    "Lenalas",
		    "Maximus",
		    "Vane",
		    "Myrdgar",
		    "Rothat",
		},
		{ // ROGUE
		    "Moreina",
		    "Akara",
		    "Kashya",
		    "Flavie",
		    "Divo",
		    "Oriana",
		    "Iantha",
		    "Shikha",
		    "Basanti",
		    "Elexa",
		},
		{ // SORCERER
		    "Jazreth",
		    "Drognan",
		    "Armin",
		    "Fauztin",
		    "Jere",
		    "Kazzulk",
		    "Ranslor",
		    "Sarnakyle",
		    "Valthek",
		    "Horazon",
		},
#ifdef HELLFIRE
		{ // MONK
		    "Akyev",
		    "Dvorak",
		    "Kekegi",
		    "Kharazim",
		    "Mikulov",
		    "Shenlong",
		    "Vedenin",
		    "Vhalit",
		    "Vylnas",
		    "Zhota",
		},
		{ // BARD (uses Rogue names)
		    "Moreina",
		    "Akara",
		    "Kashya",
		    "Flavie",
		    "Divo",
		    "Oriana",
		    "Iantha",
		    "Shikha",
		    "Basanti",
		    "Elexa",
		},
		{ // BARBARIAN
		    "Alaric",
		    "Barloc",
		    "Egtheow",
		    "Guthlaf",
		    "Heorogar",
		    "Hrothgar",
		    "Oslaf",
		    "Qual-Kehk",
		    "Ragnar",
		    "Ulf",
		}
#endif
	};
	SetRndSeed(time(NULL));
	int iRand = random_(0, lengthof(kNames[hero_class]));

	return kNames[hero_class][iRand];
}
#endif

DEVILUTION_END_NAMESPACE
