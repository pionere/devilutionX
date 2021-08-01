#include <chrono>

#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/scrollbar.h"
#include "DiabloUI/selgame.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/selyesno.h"
#include "DiabloUI/selconn.h"
#include "controls/plrctrls.h"

DEVILUTION_BEGIN_NAMESPACE

static _uiheroinfo selhero_heroInfo;
static unsigned selhero_SaveCount = 0;
static _uiheroinfo selhero_heros[MAX_CHARACTERS];
static const unsigned MaxViewportItems = 6;
static char textStats[5][4];
static char selhero_title[32];
static int selhero_result;
static bool selhero_endMenu;
static bool selhero_navigateYesNo;
static bool selhero_deleteEnabled;

static bool(*gfnHeroCreate)(_uiheroinfo *);
static void (*gfnHeroStats)(unsigned int, _uidefaultstats *);

static std::vector<UiItemBase *> vecSelHeroDialog;
static std::vector<UiListItem *> vecSelHeroDlgItems;
static std::vector<UiItemBase *> vecSelDlgItems;
static UiArtTextButton *SELLIST_DIALOG_DELETE_BUTTON;
static UiImage *SELHERO_DIALOG_HERO_IMG;

// Forward-declare UI-handlers, used by other handlers.
static void SelheroListSelect(unsigned index);
static void SelheroClassSelectorSelect(unsigned index);
static void SelheroNameSelect(unsigned index);

#if defined(PREFILL_PLAYER_NAME) || defined(__3DS__) || HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
static const char *SelheroGenerateName(BYTE hero_class)
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

static void SelheroUiFocusNavigationYesNo()
{
	if (selhero_deleteEnabled)
		UiFocusNavigationYesNo();
}

static void SelheroFreeListItems()
{
	for (unsigned i = 0; i < vecSelHeroDlgItems.size(); i++) {
		UiListItem *pUIItem = vecSelHeroDlgItems[i];
		delete pUIItem;
	}
	vecSelHeroDlgItems.clear();
}

static void SelheroFreeDlgItems()
{
	for (unsigned i = 0; i < vecSelDlgItems.size(); i++) {
		UiItemBase *pUIItem = vecSelDlgItems[i];
		delete pUIItem;
	}
	vecSelDlgItems.clear();
}

static void SelheroFree()
{
	ArtBackground.Unload();

	for (unsigned i = 0; i < vecSelHeroDialog.size(); i++) {
		UiItemBase *pUIItem = vecSelHeroDialog[i];
		delete pUIItem;
	}
	vecSelHeroDialog.clear();

	SelheroFreeDlgItems();
	SelheroFreeListItems();
}

static void SelheroSetStats()
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

static void SelheroUpdateViewportItems()
{
	const unsigned numViewportHeroes = std::min(selhero_SaveCount - ListOffset, MaxViewportItems);
	for (unsigned i = 0; i < numViewportHeroes; i++) {
		const unsigned index = i + ListOffset;
		vecSelHeroDlgItems[i]->m_text = selhero_heros[index].name;
		vecSelHeroDlgItems[i]->m_value = index;
	}
	if (numViewportHeroes < MaxViewportItems) {
		vecSelHeroDlgItems[numViewportHeroes]->m_text = "New Hero";
		vecSelHeroDlgItems[numViewportHeroes]->m_value = selhero_SaveCount;
	}
}

static void SelheroScrollIntoView(unsigned index)
{
	unsigned newOffset = ListOffset;
	if (index >= ListOffset + MaxViewportItems)
		newOffset = index - (MaxViewportItems - 1);
	if (index < ListOffset)
		newOffset = index;
	if (newOffset != ListOffset) {
		ListOffset = newOffset;
		SelheroUpdateViewportItems();
	}
}

static void SelheroInit()
{
	LoadBackgroundArt("ui_art\\selhero.pcx");
	UiAddBackground(&vecSelHeroDialog);
	UiAddLogo(&vecSelHeroDialog);
	LoadScrollBar();

	SelheroFreeDlgItems();
	SDL_Rect rect1 = { PANEL_LEFT + 24, (UI_OFFSET_Y + 161), 590, 35 };
	vecSelHeroDialog.push_back(new UiArtText(selhero_title, rect1, UIS_CENTER | UIS_BIG));

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

static void SelheroListEsc()
{
	UiInitList_clear();

	selhero_endMenu = true;
	selhero_result = SELHERO_PREVIOUS;
}

static void SelheroListFocus(unsigned index)
{
	SelheroScrollIntoView(index);
	int baseFlags = UIS_CENTER | UIS_BIG;
	if (selhero_SaveCount != 0 && index < selhero_SaveCount) {
		memcpy(&selhero_heroInfo, &selhero_heros[index], sizeof(selhero_heroInfo));
		SelheroSetStats();
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

static bool SelheroListDeleteYesNo()
{
	selhero_navigateYesNo = selhero_deleteEnabled;

	return selhero_navigateYesNo;
}

static void SelheroListInit()
{
	SelheroFreeDlgItems();

	SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
	vecSelDlgItems.push_back(new UiArtText("Select Hero", rect1, UIS_CENTER | UIS_BIG));

	SelheroFreeListItems();
	unsigned num_viewport_heroes = std::min(selhero_SaveCount + 1, MaxViewportItems);
	for (unsigned i = 0; i < num_viewport_heroes; i++) {
		vecSelHeroDlgItems.push_back(new UiListItem("", -1));
	}
	SelheroUpdateViewportItems();

	vecSelDlgItems.push_back(new UiList(vecSelHeroDlgItems, PANEL_LEFT + 265, (UI_OFFSET_Y + 256), 320, 26, UIS_CENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect2 = { PANEL_LEFT + 585, (UI_OFFSET_Y + 244), 25, 178 };
	UiScrollBar *scrollBar = new UiScrollBar(&ArtScrollBarBackground, &ArtScrollBarThumb, &ArtScrollBarArrow, rect2);
	vecSelDlgItems.push_back(scrollBar);

	SDL_Rect rect3 = { PANEL_LEFT + 239, (UI_OFFSET_Y + 429), 120, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { PANEL_LEFT + 364, (UI_OFFSET_Y + 429), 120, 35 };
	SELLIST_DIALOG_DELETE_BUTTON = new UiArtTextButton("Delete", &SelheroUiFocusNavigationYesNo, rect4, UIS_CENTER | UIS_BIG | UIS_DISABLED);
	vecSelDlgItems.push_back(SELLIST_DIALOG_DELETE_BUTTON);

	SDL_Rect rect5 = { PANEL_LEFT + 489, (UI_OFFSET_Y + 429), 120, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect5, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelDlgItems, selhero_SaveCount + 1, SelheroListFocus, SelheroListSelect, SelheroListEsc, SelheroListDeleteYesNo, false);
	UiInitScrollBar(scrollBar, MaxViewportItems);
	snprintf(selhero_title, sizeof(selhero_title), "%s Player Characters", selconn_bMulti ? "Multi" : "Single");
}

static void SelheroClassSelectorEsc()
{
	SelheroFreeDlgItems();
	SelheroFreeListItems();

	if (selhero_SaveCount != 0) {
		SelheroListInit();
		return;
	}

	SelheroListEsc();
}

static void SelheroLoadFocus(unsigned index)
{
}

static void SelheroClassSelectorFocus(unsigned index)
{
	_uidefaultstats defaults;
	gfnHeroStats(index, &defaults);

	selhero_heroInfo.level = 1;
	selhero_heroInfo.heroclass = vecSelHeroDlgItems[index]->m_value;
	selhero_heroInfo.strength = defaults.strength;
	selhero_heroInfo.magic = defaults.magic;
	selhero_heroInfo.dexterity = defaults.dexterity;
	selhero_heroInfo.vitality = defaults.vitality;

	SelheroSetStats();
}

static void SelheroLoadSelect(unsigned index)
{
	UiInitList_clear();
	selhero_endMenu = true;
	selhero_result = index == 0 ? SELHERO_CONTINUE : SELHERO_NEW_DUNGEON;
}

static void SelheroListSelect(unsigned index)
{
	if (index == selhero_SaveCount) {
		SelheroFreeDlgItems();

		SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
		vecSelDlgItems.push_back(new UiArtText("Choose Class", rect1, UIS_CENTER | UIS_BIG));

		SelheroFreeListItems();
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

		UiInitList(vecSelDlgItems, vecSelHeroDlgItems.size(), SelheroClassSelectorFocus, SelheroClassSelectorSelect, SelheroClassSelectorEsc);
		memset(&selhero_heroInfo.name, 0, sizeof(selhero_heroInfo.name));
		snprintf(selhero_title, sizeof(selhero_title), "New %s Player Hero", selconn_bMulti ? "Multi" : "Single");
		return;
	}

	if (selhero_heroInfo.hassaved) {
		SelheroFreeDlgItems();

		SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
		vecSelDlgItems.push_back(new UiArtText("Save File Exists", rect1, UIS_CENTER | UIS_BIG));

		SelheroFreeListItems();
		vecSelHeroDlgItems.push_back(new UiListItem("Load Game", 0));
		vecSelHeroDlgItems.push_back(new UiListItem("New Game", 1));
		vecSelDlgItems.push_back(new UiList(vecSelHeroDlgItems, PANEL_LEFT + 265, (UI_OFFSET_Y + 285), 320, 33, UIS_CENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect2 = { PANEL_LEFT + 279, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect2, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect3 = { PANEL_LEFT + 429, (UI_OFFSET_Y + 427), 140, 35 };
		vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect3, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		UiInitList(vecSelDlgItems, vecSelHeroDlgItems.size(), SelheroLoadFocus, SelheroLoadSelect, SelheroListInit, NULL, true);
		snprintf(selhero_title, sizeof(selhero_title), "Single Player Characters");
		return;
	}

	SelheroLoadSelect(1);
}

static void SelheroNameEsc()
{
	SelheroListSelect(selhero_SaveCount);
}

static void SelheroClassSelectorSelect(unsigned index)
{
	snprintf(selhero_title, sizeof(selhero_title), "New %s Player Hero", selconn_bMulti ? "Multi" : "Single");
	memset(selhero_heroInfo.name, '\0', sizeof(selhero_heroInfo.name));
#if defined(PREFILL_PLAYER_NAME) || HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
#ifndef PREFILL_PLAYER_NAME
	if (sgbControllerActive)
#endif
		SStrCopy(selhero_heroInfo.name, SelheroGenerateName(selhero_heroInfo.heroclass), sizeof(selhero_heroInfo.name));
#endif
	SelheroFreeDlgItems();
	SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
	vecSelDlgItems.push_back(new UiArtText("Enter Name", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 265, (UI_OFFSET_Y + 317), 320, 33 };
	vecSelDlgItems.push_back(new UiEdit("Enter Name", selhero_heroInfo.name, 15, rect2, UIS_MED | UIS_GOLD));

	SDL_Rect rect3 = { PANEL_LEFT + 279, (UI_OFFSET_Y + 429), 140, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { PANEL_LEFT + 429, (UI_OFFSET_Y + 429), 140, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelDlgItems, 0, NULL, SelheroNameSelect, SelheroNameEsc);
}

static void SelheroNameSelect(unsigned index)
{
	if (!UiValidPlayerName(selhero_heroInfo.name)) {
		ArtBackground.Unload();
		UiSelOkDialog(selhero_title, "Invalid name. A name cannot contain spaces, reserved characters, or reserved words.\n", false);
		LoadBackgroundArt("ui_art\\selhero.pcx");
	} else {
		bool overwrite = true;
		for (unsigned i = 0; i < selhero_SaveCount; i++) {
			if (strcasecmp(selhero_heros[i].name, selhero_heroInfo.name) == 0) {
				ArtBackground.Unload();
				char dialogText[256];
				snprintf(dialogText, sizeof(dialogText), "Character already exists. Do you want to overwrite \"%s\"?", selhero_heroInfo.name);
				overwrite = UiSelHeroYesNoDialog(selhero_title, dialogText);
				LoadBackgroundArt("ui_art\\selhero.pcx");
				break;
			}
		}

		if (overwrite) {
			if (gfnHeroCreate(&selhero_heroInfo)) {
				SelheroLoadSelect(1);
				return;
			} else {
				std::vector<UiItemBase*> allItems = vecSelHeroDialog;
				allItems.insert(allItems.end(), vecSelDlgItems.begin(), vecSelDlgItems.end());
				UiErrorOkDialog("Unable to create character.", allItems);
			}
		}
	}

	SelheroClassSelectorSelect(0);
}

static void SelHeroGetHeroInfo(_uiheroinfo *pInfo)
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
	do {
		SelheroInit();

		gfnHeroCreate = fncreate;
		gfnHeroStats = fnstats;
		//selhero_result = 0;

		selhero_navigateYesNo = false;

		selhero_SaveCount = 0;
		fninfo(SelHeroGetHeroInfo);
		std::reverse(selhero_heros, selhero_heros + selhero_SaveCount);

		if (selhero_SaveCount != 0) {
			SelheroListInit();
		} else {
			SelheroListSelect(0);
		}

		selhero_endMenu = false;
		while (!selhero_endMenu && !selhero_navigateYesNo) {
			UiClearScreen();
			UiRenderItems(vecSelHeroDialog);
			UiPollAndRender();
		}
		SelheroFree();

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

DEVILUTION_END_NAMESPACE
