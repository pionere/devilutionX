#include "selhero.h"

#include <chrono>
#include <random>

#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/scrollbar.h"
#include "DiabloUI/selgame.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/selyesno.h"

#include "controls/plrctrls.h"

#ifdef __3DS__
#include "platform/ctr/keyboard.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

const char *SelheroGenerateName(uint8_t hero_class);

std::size_t selhero_SaveCount = 0;
_uiheroinfo selhero_heros[MAX_CHARACTERS];
_uiheroinfo selhero_heroInfo;
const size_t MaxViewportItems = 6;
char textStats[5][4];
char title[32];
char selhero_Description[256];
int selhero_result;
bool selhero_endMenu;
bool selhero_isMultiPlayer;
bool selhero_navigateYesNo;
bool selhero_deleteEnabled;

BOOL(*gfnHeroInfo)
(BOOL (*fninfofunc)(_uiheroinfo *));
BOOL(*gfnHeroCreate)
(_uiheroinfo *);
void (*gfnHeroStats)(unsigned int, _uidefaultstats *);

namespace {

std::vector<UiItemBase *> vecSelHeroDialog;
std::vector<UiListItem *> vecSelHeroDlgItems;
std::vector<UiItemBase *> vecSelDlgItems;

UiImage *SELHERO_DIALOG_HERO_IMG;
} // namespace

bool bUIElementsLoaded = false;

void SelheroUiFocusNavigationYesNo()
{
	if (selhero_deleteEnabled)
		UiFocusNavigationYesNo();
}

static void SelheroFreeListItems()
{
	for (auto pUIItem : vecSelHeroDlgItems) {
		delete pUIItem;
	}
	vecSelHeroDlgItems.clear();
}

static void SelheroFreeDlgItems()
{
	for (auto pUIItem : vecSelDlgItems) {
		delete pUIItem;
	}
	vecSelDlgItems.clear();
}

static void SelheroFree()
{
	ArtBackground.Unload();

	for (auto pUIItem : vecSelHeroDialog) {
		delete pUIItem;
	}
	vecSelHeroDialog.clear();

	SelheroFreeDlgItems();
	SelheroFreeListItems();
	UnloadScrollBar();

	bUIElementsLoaded = false;
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

namespace {

UiArtTextButton *SELLIST_DIALOG_DELETE_BUTTON;

} // namespace

static void SelheroUpdateViewportItems()
{
	const std::size_t numViewportHeroes = std::min(selhero_SaveCount - ListOffset, MaxViewportItems);
	for (std::size_t i = 0; i < numViewportHeroes; i++) {
		const std::size_t index = i + ListOffset;
		vecSelHeroDlgItems[i]->m_text = selhero_heros[index].name;
		vecSelHeroDlgItems[i]->m_value = static_cast<int>(index);
	}
	if (numViewportHeroes < MaxViewportItems) {
		vecSelHeroDlgItems[numViewportHeroes]->m_text = "New Hero";
		vecSelHeroDlgItems[numViewportHeroes]->m_value = static_cast<int>(selhero_SaveCount);
	}
}

static void SelheroScrollIntoView(std::size_t index)
{
	std::size_t newOffset = ListOffset;
	if (index >= ListOffset + MaxViewportItems)
		newOffset = index - (MaxViewportItems - 1);
	if (index < ListOffset)
		newOffset = index;
	if (newOffset != ListOffset) {
		ListOffset = newOffset;
		SelheroUpdateViewportItems();
	}
}

void SelheroInit()
{
	LoadBackgroundArt("ui_art\\selhero.pcx");
	UiAddBackground(&vecSelHeroDialog);
	UiAddLogo(&vecSelHeroDialog);
	LoadScrollBar();

	SelheroFreeDlgItems();
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

void SelheroListInit()
{
	SelheroFreeDlgItems();

	SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
	vecSelDlgItems.push_back(new UiArtText("Select Hero", rect1, UIS_CENTER | UIS_BIG));

	SelheroFreeListItems();
	size_t num_viewport_heroes = std::min(selhero_SaveCount + 1, MaxViewportItems);
	for (std::size_t i = 0; i < num_viewport_heroes; i++) {
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
	snprintf(title, sizeof(title), "%s Player Characters", selhero_isMultiPlayer ? "Multi" : "Single");
}

void SelheroListFocus(std::size_t index)
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

bool SelheroListDeleteYesNo()
{
	selhero_navigateYesNo = selhero_deleteEnabled;

	return selhero_navigateYesNo;
}

void SelheroListSelect(std::size_t index)
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
		snprintf(title, sizeof(title), "New %s Player Hero", selhero_isMultiPlayer ? "Multi" : "Single");
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
		snprintf(title, sizeof(title), "Single Player Characters");
		return;
	}

	SelheroLoadSelect(1);
}

void SelheroListEsc()
{
	UiInitList_clear();

	selhero_endMenu = true;
	selhero_result = SELHERO_PREVIOUS;
}

void SelheroClassSelectorFocus(std::size_t index)
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

void SelheroClassSelectorSelect(std::size_t index)
{
#ifdef SPAWN
	int hClass = vecSelHeroDlgItems[index]->m_value;

	if (hClass != PC_WARRIOR) {
		ArtBackground.Unload();
		UiSelOkDialog(NULL, "The Rogue and Sorcerer are only available in the full retail version of Diablo. Visit https://www.gog.com/game/diablo to purchase.", false);
		LoadBackgroundArt("ui_art\\selhero.pcx");
		SelheroListSelect(selhero_SaveCount);
		return;
	}
#endif

	snprintf(title, sizeof(title), "New %s Player Hero", selhero_isMultiPlayer ? "Multi" : "Single");
	memset(selhero_heroInfo.name, '\0', sizeof(selhero_heroInfo.name));
#if defined __3DS__
	ctr_vkbdInput("Enter Name", SelheroGenerateName(selhero_heroInfo.heroclass), selhero_heroInfo.name);
#else
#ifndef PREFILL_PLAYER_NAME
	if (sgbControllerActive)
#endif
		SStrCopy(selhero_heroInfo.name, SelheroGenerateName(selhero_heroInfo.heroclass), sizeof(selhero_heroInfo.name));
#endif
	SelheroFreeDlgItems();
	SDL_Rect rect1 = { PANEL_LEFT + 264, (UI_OFFSET_Y + 211), 320, 33 };
	vecSelDlgItems.push_back(new UiArtText("Enter Name", rect1, UIS_CENTER | UIS_BIG));

	SDL_Rect rect2 = { PANEL_LEFT + 265, (UI_OFFSET_Y + 317), 320, 33 };
	vecSelDlgItems.push_back(new UiEdit(selhero_heroInfo.name, 15, rect2, UIS_MED | UIS_GOLD));

	SDL_Rect rect3 = { PANEL_LEFT + 279, (UI_OFFSET_Y + 429), 140, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { PANEL_LEFT + 429, (UI_OFFSET_Y + 429), 140, 35 };
	vecSelDlgItems.push_back(new UiArtTextButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(vecSelDlgItems, 0, NULL, SelheroNameSelect, SelheroNameEsc);
}

void SelheroClassSelectorEsc()
{
	SelheroFreeDlgItems();
	SelheroFreeListItems();

	if (selhero_SaveCount != 0) {
		SelheroListInit();
		return;
	}

	SelheroListEsc();
}

void SelheroNameSelect(std::size_t index)
{

	if (!UiValidPlayerName(selhero_heroInfo.name)) {
		ArtBackground.Unload();
		UiSelOkDialog(title, "Invalid name. A name cannot contain spaces, reserved characters, or reserved words.\n", false);
		LoadBackgroundArt("ui_art\\selhero.pcx");
	} else {
		bool overwrite = true;
		for (std::size_t i = 0; i < selhero_SaveCount; i++) {
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
				SelheroLoadSelect(1);
				return;
			}
			UiErrorOkDialog("Unable to create character.", vecSelDlgItems);
		}
	}

	SelheroClassSelectorSelect(0);
}

void SelheroNameEsc()
{
	SelheroListSelect(selhero_SaveCount);
}

void SelheroLoadFocus(std::size_t index)
{
}

void SelheroLoadSelect(std::size_t index)
{
	UiInitList_clear();
	selhero_endMenu = true;
	if (vecSelHeroDlgItems[index]->m_value == 0) {
		selhero_result = SELHERO_CONTINUE;
		return;
	}

	if (!selhero_isMultiPlayer) {
		// This is part of a dangerous hack to enable difficulty selection in single-player.
		// FIXME: Dialogs should not refer to each other's variables.

		// We disable `selhero_endMenu` and replace the background and art
		// and the item list with the difficulty selection ones.
		//
		// This means selhero's render loop will render selgame's items,
		// which happens to work because the render loops are similar.
		selhero_endMenu = false;
		SelheroFree();
		LoadBackgroundArt("ui_art\\selgame.pcx");
		selgame_GameSelection_Select(0);
	}

	selhero_result = 0;
}

BOOL SelHeroGetHeroInfo(_uiheroinfo *pInfo)
{
	selhero_heros[selhero_SaveCount] = *pInfo;
	selhero_SaveCount++;

	return true;
}

static void UiSelHeroDialog(
    BOOL (*fninfo)(BOOL (*fninfofunc)(_uiheroinfo *)),
    BOOL (*fncreate)(_uiheroinfo *),
    void (*fnstats)(unsigned int, _uidefaultstats *),
    BOOL (*fnremove)(_uiheroinfo *),
    int *dlgresult,
    char (&name)[16])
{
	bUIElementsLoaded = true;

	do {
		SelheroInit();

		gfnHeroInfo = fninfo;
		gfnHeroCreate = fncreate;
		gfnHeroStats = fnstats;
		selhero_result = *dlgresult;

		selhero_navigateYesNo = false;

		selhero_SaveCount = 0;
		gfnHeroInfo(SelHeroGetHeroInfo);
		std::reverse(selhero_heros, selhero_heros + selhero_SaveCount);

		if (selhero_SaveCount) {
			SelheroListInit();
		} else {
			SelheroListSelect(selhero_SaveCount);
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
			snprintf(dialogTitle, sizeof(dialogTitle), "Delete %s Player Hero", selhero_isMultiPlayer ? "Multi" : "Single");
			snprintf(dialogText, sizeof(dialogText), "Are you sure you want to delete the character \"%s\"?", selhero_heroInfo.name);

			if (UiSelHeroYesNoDialog(dialogTitle, dialogText))
				fnremove(&selhero_heroInfo);
		}
	} while (selhero_navigateYesNo);

	*dlgresult = selhero_result;
	copy_str(name, selhero_heroInfo.name);
}

void UiSelHeroSingDialog(
    BOOL (*fninfo)(BOOL (*fninfofunc)(_uiheroinfo *)),
    BOOL (*fncreate)(_uiheroinfo *),
    BOOL (*fnremove)(_uiheroinfo *),
    void (*fnstats)(unsigned int, _uidefaultstats *),
    int *dlgresult,
    char (&name)[16],
    int *difficulty)
{
	selhero_isMultiPlayer = false;
	UiSelHeroDialog(fninfo, fncreate, fnstats, fnremove, dlgresult, name);
	*difficulty = gbDifficulty;
}

void UiSelHeroMultDialog(
    BOOL (*fninfo)(BOOL (*fninfofunc)(_uiheroinfo *)),
    BOOL (*fncreate)(_uiheroinfo *),
    BOOL (*fnremove)(_uiheroinfo *),
    void (*fnstats)(unsigned int, _uidefaultstats *),
    int *dlgresult,
    BOOL *hero_is_created,
    char (&name)[16])
{
	selhero_isMultiPlayer = true;
	UiSelHeroDialog(fninfo, fncreate, fnstats, fnremove, dlgresult, name);
}

const char *SelheroGenerateName(uint8_t hero_class)
{
	static const char *const kNames[NUM_CLASSES][10] = {
		{
		    // Warrior
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
		{
		    // Rogue
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
		{
		    // Sorcerer
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
		{
		    // Monk
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
		{
		    // Bard (uses Rogue names)
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
		{
		    // Barbarian
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
		},
#endif
	};

	int iRand = rand() % 10;

	return kNames[hero_class][iRand];
}

DEVILUTION_END_NAMESPACE
