#include "selhero.h"

#include <ctime>

#include "all.h"
#include "DiabloUI/diablo.h"
#include "DiabloUI/diabloui.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/selyesno.h"
#include "DiabloUI/selconn.h"
#include "controls/controller.h"

DEVILUTION_BEGIN_NAMESPACE

#define MAX_VIEWPORT_ITEMS ((unsigned)((SELHERO_RPANEL_HEIGHT - 22) / 26))

_uiheroinfo selhero_heroInfo;
static unsigned selhero_SaveCount = 0;
static std::vector<_uiheroinfo> selhero_heros;
static char textStats[5][4];
static int selhero_result; // _selhero_selections

static UiTxtButton* SELLIST_DIALOG_DELETE_BUTTON;
static UiImage* SELHERO_DIALOG_HERO_IMG;
static unsigned selhero_heroFrame;

// Forward-declare UI-handlers, used by other handlers.
static void SelheroListSelect(unsigned index);
static void SelheroNameInit(unsigned index);
static void SelheroNameSelect(unsigned index);
static void SelheroListInit();
static void SelheroClassSelectorInit();

#if defined(PREFILL_PLAYER_NAME) || defined(__3DS__) || HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
static const char* SelheroGenerateName(BYTE hero_class)
{
	static const char* const kNames[NUM_CLASSES][10] = {
		// clang-format off
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
		// clang-format on
	};
	SetRndSeed(time(NULL));
	int iRand = random_(0, lengthof(kNames[hero_class]));

	return kNames[hero_class][iRand];
}
#endif

static void SelheroFreeDlgItems()
{
	UiClearListItems();

	UiClearItems();
}

static void SelheroFree()
{
	FreeBackgroundArt();

	SelheroFreeDlgItems();
}

static void SelheroSetStats()
{
	int heroclass = selhero_heroInfo.hiClass;
	int baseFlags = UIS_HCENTER | UIS_VCENTER | UIS_BIG;

	if (heroclass < NUM_CLASSES) {
		if (SELLIST_DIALOG_DELETE_BUTTON != NULL) {
			SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_GOLD;
		}
		SELHERO_DIALOG_HERO_IMG->m_frame = heroclass + 1;
		selhero_heroFrame = heroclass + 1;

		snprintf(textStats[0], sizeof(textStats[0]), "%d", selhero_heroInfo.hiLevel);
		snprintf(textStats[1], sizeof(textStats[1]), "%d", selhero_heroInfo.hiStrength);
		snprintf(textStats[2], sizeof(textStats[2]), "%d", selhero_heroInfo.hiMagic);
		snprintf(textStats[3], sizeof(textStats[3]), "%d", selhero_heroInfo.hiDexterity);
		snprintf(textStats[4], sizeof(textStats[4]), "%d", selhero_heroInfo.hiVitality);
	} else {
		assert(SELLIST_DIALOG_DELETE_BUTTON != NULL);
		SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_SILVER | UIS_DISABLED;
		SELHERO_DIALOG_HERO_IMG->m_frame = 0;
		selhero_heroFrame = 0;

		copy_cstr(textStats[0], "--");
		copy_cstr(textStats[1], "--");
		copy_cstr(textStats[2], "--");
		copy_cstr(textStats[3], "--");
		copy_cstr(textStats[4], "--");
	}
}

static void SelheroUpdateViewportItems()
{
	const unsigned numViewportHeroes = std::min(selhero_SaveCount + 1 - ListOffset, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < numViewportHeroes; i++) {
		const unsigned index = i + ListOffset;
		gUIListItems[i]->m_text = selhero_heros[index].hiName;
		//gUIListItems[i]->m_value = index;
	}
}

static void SelheroInit()
{
	LoadBackgroundArt("ui_art\\selhero.CEL", "ui_art\\menu.pal");
}

static void SelheroInitHeros()
{
	selhero_heros.clear();
	pfile_ui_load_heros(selhero_heros);
	static_assert(MAX_CHARACTERS <= UINT32_MAX, "Check overflow in SelheroInitHeros.");
	selhero_SaveCount = (unsigned)selhero_heros.size();
	{
		_uiheroinfo newHero;
		copy_cstr(newHero.hiName, "New Hero");
		newHero.hiClass = NUM_CLASSES;
		selhero_heros.push_back(newHero);
	}

	if (selhero_SaveCount != 0) {
		SelheroListInit();
	} else {
		SelheroClassSelectorInit();
	}
}

static void SelheroResetScreen(const char* title, const char* rheader)
{
	SelheroFreeDlgItems();

	UiAddBackground();
	UiAddLogo();

	SDL_Rect rect1 = { PANEL_LEFT, SELHERO_TITLE_TOP, PANEL_WIDTH, 35 };
	gUiItems.push_back(new UiText(title, rect1, UIS_HCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { SELHERO_HEROS_LEFT, SELHERO_PNL_TOP, SELHERO_HEROS_WIDTH, SELHERO_HEROS_HEIGHT };
	SELHERO_DIALOG_HERO_IMG = new UiImage(gbHerosCel, selhero_heroFrame, rect2, false);
	gUiItems.push_back(SELHERO_DIALOG_HERO_IMG);

	SDL_Rect rect4 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	gUiItems.push_back(new UiText("Level:", rect4, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect5 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP, 40, 21 };
	gUiItems.push_back(new UiText(textStats[0], rect5, UIS_HCENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect6 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 0 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	gUiItems.push_back(new UiText("Strength:", rect6, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect7 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 0 * 21, 40, 21 };
	gUiItems.push_back(new UiText(textStats[1], rect7, UIS_HCENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect8 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 1 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	gUiItems.push_back(new UiText("Magic:", rect8, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect9 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 1 * 21, 40, 21 };
	gUiItems.push_back(new UiText(textStats[2], rect9, UIS_HCENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect10 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 2 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	gUiItems.push_back(new UiText("Dexterity:", rect10, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect11 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 2 * 21, 40, 21 };
	gUiItems.push_back(new UiText(textStats[3], rect11, UIS_HCENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect12 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 3 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	gUiItems.push_back(new UiText("Vitality:", rect12, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect13 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 3 * 21, 40, 21 };
	gUiItems.push_back(new UiText(textStats[4], rect13, UIS_HCENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect14 = { SELHERO_RPANEL_LEFT, SELHERO_PNL_TOP, SELHERO_RPANEL_WIDTH, SELHERO_RHEADER_HEIGHT };
	gUiItems.push_back(new UiText(rheader, rect14, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));
}

static void SelheroListEsc()
{
	selhero_result = SELHERO_PREVIOUS;
}

static void SelheroListFocus(unsigned index)
{
	SelheroUpdateViewportItems();

	memcpy(&selhero_heroInfo, &selhero_heros[index], sizeof(selhero_heroInfo));

	SelheroSetStats();
}

static void SelheroListDeleteYesNo(unsigned index)
{
	if (index == 0)
		pfile_ui_delete_hero(&selhero_heroInfo);
	SelheroInitHeros();
}

static void SelheroListDelete()
{
	// if (SELLIST_DIALOG_DELETE_BUTTON->m_iFlags & UIS_DISABLED)
	if (SelectedItem == selhero_SaveCount)
		return;

	SelheroResetScreen(selconn_bMulti ? "Multi Player Characters" : "Single Player Characters", "Confirm delete");

	SDL_Rect rect1 = { SELHERO_RPANEL_LEFT + 25, SELHERO_LIST_TOP, SELHERO_RPANEL_WIDTH - 2 * 25, 30 };
	gUiItems.push_back(new UiText(selhero_heroInfo.hiName, rect1, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	gUIListItems.push_back(new UiListItem("Yes", 0));
	gUIListItems.push_back(new UiListItem("No", 1));
	SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + (SELHERO_RPANEL_WIDTH - 100) / 2, SELHERO_LIST_TOP + 40, 100, 26 * 2 };
	gUiItems.push_back(new UiList(&gUIListItems, 2, rect2, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	UiInitScreen(2, NULL, SelheroListDeleteYesNo, SelheroInitHeros);
}

static void SelheroListInit()
{
	SelheroResetScreen(selconn_bMulti ? "Multi Player Characters" : "Single Player Characters", "Select Hero");

	unsigned num_viewport_heroes = std::min(selhero_SaveCount + 1, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < num_viewport_heroes; i++) {
		gUIListItems.push_back(new UiListItem("", -1));
	}
	SelheroUpdateViewportItems();

	SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + 25, SELHERO_LIST_TOP, SELHERO_RPANEL_WIDTH - 2 * 25, 26 * (int)num_viewport_heroes };
	gUiItems.push_back(new UiList(&gUIListItems, num_viewport_heroes, rect2, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect3 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH - SCROLLBAR_BG_WIDTH + 1, SELHERO_RPANEL_TOP - 1, SCROLLBAR_BG_WIDTH, SELHERO_RPANEL_HEIGHT + 1 };
	UiScrollBar* scrollBar = new UiScrollBar(rect3);
	gUiItems.push_back(scrollBar);

	SDL_Rect rect4 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect4, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 3, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 3, 35 };
	SELLIST_DIALOG_DELETE_BUTTON = new UiTxtButton("Delete", &UiFocusNavigationDelete, rect5, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER | UIS_DISABLED);
	gUiItems.push_back(SELLIST_DIALOG_DELETE_BUTTON);

	SDL_Rect rect6 = { SELHERO_RPANEL_LEFT + 2 * SELHERO_RPANEL_WIDTH / 3, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect6, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	UiInitScreen(selhero_SaveCount + 1, SelheroListFocus, SelheroListSelect, SelheroListEsc);
	UiInitScrollBar(scrollBar, MAX_VIEWPORT_ITEMS, SelheroListDelete);
}

static void SelheroClassSelectorEsc()
{
	if (selhero_SaveCount != 0) {
		SelheroListInit();
		return;
	}

	SelheroListEsc();
}

static void SelheroClassSelectorFocus(unsigned index)
{
	//_uidefaultstats defaults;
	//pfile_ui_default_stats(index, &defaults);
	assert((unsigned)gUIListItems[index]->m_value == index);

	selhero_heroInfo.hiIdx = MAX_CHARACTERS + 1;
	selhero_heroInfo.hiLevel = 1;
	selhero_heroInfo.hiClass = index;
	selhero_heroInfo.hiSaveFile = FALSE;
	selhero_heroInfo.hiStrength = StrengthTbl[index];   //defaults.dsStrength;
	selhero_heroInfo.hiMagic = MagicTbl[index];         //defaults.dsMagic;
	selhero_heroInfo.hiDexterity = DexterityTbl[index]; //defaults.dsDexterity;
	selhero_heroInfo.hiVitality = VitalityTbl[index];   //defaults.dsVitality;

	SelheroSetStats();
}

static void SelheroContinue()
{
	selhero_result = SELHERO_CONTINUE;
}

static void SelheroClassSelectorInit()
{
	SelheroResetScreen(selconn_bMulti ? "New Multi Player Hero" : "New Single Player Hero", "Choose Class");

	gUIListItems.push_back(new UiListItem("Warrior", PC_WARRIOR));
	gUIListItems.push_back(new UiListItem("Rogue", PC_ROGUE));
	gUIListItems.push_back(new UiListItem("Sorcerer", PC_SORCERER));
#ifdef HELLFIRE
	gUIListItems.push_back(new UiListItem("Monk", PC_MONK));
	gUIListItems.push_back(new UiListItem("Bard", PC_BARD));
	gUIListItems.push_back(new UiListItem("Barbarian", PC_BARBARIAN));
#endif
	//assert(gUIListItems.size() == NUM_CLASSES);
	SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + (SELHERO_RPANEL_WIDTH - 270) / 2, SELHERO_LIST_TOP, 270, 26 * NUM_CLASSES };
	gUiItems.push_back(new UiList(&gUIListItems, NUM_CLASSES, rect2, UIS_HCENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect3 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect3, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SELLIST_DIALOG_DELETE_BUTTON = NULL; // TODO: reset in SelheroFreeDlgItems?

	SDL_Rect rect4 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 2, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	//assert(gUIListItems.size() == NUM_CLASSES);
	UiInitScreen(NUM_CLASSES, SelheroClassSelectorFocus, SelheroNameInit, SelheroClassSelectorEsc);
}

static void SelheroListSelect(unsigned index)
{
	if (index == selhero_SaveCount) {
		SelheroClassSelectorInit();
		return;
	}

	SelheroContinue();
}

static void SelheroNameInit(unsigned index)
{
	SelheroResetScreen(selconn_bMulti ? "New Multi Player Hero" : "New Single Player Hero", "Enter Name");

	memset(selhero_heroInfo.hiName, '\0', sizeof(selhero_heroInfo.hiName));
#if defined(PREFILL_PLAYER_NAME) || HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#ifndef PREFILL_PLAYER_NAME
	if (sgbControllerActive)
#endif
		SStrCopy(selhero_heroInfo.hiName, SelheroGenerateName(selhero_heroInfo.hiClass), sizeof(selhero_heroInfo.hiName));
#endif

	SDL_Rect rect3 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect3, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 2, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_HCENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + 24, SELHERO_RPANEL_TOP + (SELHERO_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELHERO_RPANEL_WIDTH - 2 * 24, FOCUS_MEDIUM };
	static_assert(sizeof(selhero_heroInfo.hiName) <= UIEDIT_MAXLENGTH, "The edit field of SelheroNameInit must fit to UIEdit.");
	UiEdit* edit = new UiEdit("Enter Name", selhero_heroInfo.hiName, sizeof(selhero_heroInfo.hiName), rect2);
	gUiItems.push_back(edit);

	UiInitScreen(0, NULL, SelheroNameSelect, SelheroClassSelectorInit);
	UiInitEdit(edit);
}

static void SelheroNameSelect(unsigned index)
{
	const char* err;
	int result = pfile_ui_create_hero(&selhero_heroInfo);

	switch (result) {
	case NEWHERO_DONE:
		SelheroContinue();
		return;
	case NEWHERO_INVALID_NAME:
		err = "Invalid name.\nA name cannot contain reserved characters.";
		break;
	case NEWHERO_HERO_LIMIT:
		err = "Hero count limit reached.\nDelete a hero before creating a new one.";
		break;
	case NEWHERO_FAIL:
		err = "Storage device is full or read-only.";
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	FreeBackgroundArt();
	SelheroFreeDlgItems();
	UiSelOkDialog("Unable to create hero", err);
	LoadBackgroundArt("ui_art\\selhero.CEL", "ui_art\\menu.pal");
	SelheroNameInit(0);
}

int UiSelHeroDialog(unsigned* saveIdx)
{
	SelheroInit();

	SelheroInitHeros();

	selhero_result = SELHERO_NONE;
	do {
		UiRenderAndPoll();
	} while (selhero_result == SELHERO_NONE);
	SelheroFree();

	*saveIdx = selhero_heroInfo.hiIdx;

	return selhero_result;
}

DEVILUTION_END_NAMESPACE
