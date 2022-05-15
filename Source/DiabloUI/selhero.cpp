#include <time.h>

#include "DiabloUI/diabloui.h"
#include "DiabloUI/dialogs.h"
#include "DiabloUI/scrollbar.h"
#include "DiabloUI/selok.h"
#include "DiabloUI/selyesno.h"
#include "DiabloUI/selconn.h"
#include "controls/plrctrls.h"
#include "../engine.h"

DEVILUTION_BEGIN_NAMESPACE

#define MAX_VIEWPORT_ITEMS		((unsigned)((SELHERO_RPANEL_HEIGHT - 22) / 26))

static _uiheroinfo selhero_heroInfo;
static unsigned selhero_SaveCount = 0;
static std::vector<_uiheroinfo> selhero_heros;
static char textStats[5][4];
static char selhero_title[32];
static int selhero_result;
static bool selhero_endMenu;
static bool selhero_navigateYesNo;
static bool selhero_deleteEnabled;

static int(*gfnHeroCreate)(_uiheroinfo *);
//static void (*gfnHeroStats)(unsigned int, _uidefaultstats *);

static std::vector<UiItemBase *> vecSelHeroDialog;
static UiTxtButton* SELLIST_DIALOG_DELETE_BUTTON;
static UiImage* SELHERO_DIALOG_HERO_IMG;

// Forward-declare UI-handlers, used by other handlers.
static void SelheroListSelect(unsigned index);
static void SelheroClassSelectorSelect(unsigned index);
static void SelheroNameSelect(unsigned index);

#if defined(PREFILL_PLAYER_NAME) || defined(__3DS__) || HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
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

static void SelheroFreeDlgItems()
{
	UiClearListItems();

	UiClearItems(gUiItems);
}

static void SelheroFree()
{
	MemFreeDbg(gbBackCel);
	MemFreeDbg(gbHerosCel);
	UnloadScrollBar();

	UiClearItems(vecSelHeroDialog);

	SelheroFreeDlgItems();

	//UiInitList_clear();
}

static void SelheroSetStats()
{
	int heroclass = selhero_heroInfo.hiClass;

	SELHERO_DIALOG_HERO_IMG->m_frame = heroclass + 1;

	snprintf(textStats[0], sizeof(textStats[0]), "%d", selhero_heroInfo.hiLevel);
	snprintf(textStats[1], sizeof(textStats[1]), "%d", selhero_heroInfo.hiStrength);
	snprintf(textStats[2], sizeof(textStats[2]), "%d", selhero_heroInfo.hiMagic);
	snprintf(textStats[3], sizeof(textStats[3]), "%d", selhero_heroInfo.hiDexterity);
	snprintf(textStats[4], sizeof(textStats[4]), "%d", selhero_heroInfo.hiVitality);
}

static void SelheroUpdateViewportItems()
{
	const unsigned numViewportHeroes = std::min(selhero_SaveCount - ListOffset, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < numViewportHeroes; i++) {
		const unsigned index = i + ListOffset;
		gUIListItems[i]->m_text = selhero_heros[index].hiName;
		//gUIListItems[i]->m_value = index;
	}
	if (numViewportHeroes < MAX_VIEWPORT_ITEMS) {
		gUIListItems[numViewportHeroes]->m_text = "New Hero";
		//gUIListItems[numViewportHeroes]->m_value = selhero_SaveCount;
	}
}

static void SelheroInit()
{
	LoadScrollBar();
	gbHerosCel = CelLoadImage("ui_art\\heros.CEL", SELHERO_HEROS_WIDTH);

	LoadBackgroundArt("ui_art\\selhero.CEL", "ui_art\\menu.pal");
	UiAddBackground(&vecSelHeroDialog);
	UiAddLogo(&vecSelHeroDialog);

	SDL_Rect rect1 = { PANEL_LEFT, SELHERO_TITLE_TOP, PANEL_WIDTH, 35 };
	vecSelHeroDialog.push_back(new UiText(selhero_title, rect1, UIS_CENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { SELHERO_HEROS_LEFT, SELHERO_PNL_TOP, SELHERO_HEROS_WIDTH, SELHERO_HEROS_HEIGHT };
	SELHERO_DIALOG_HERO_IMG = new UiImage(gbHerosCel, 7, rect2, false);
	vecSelHeroDialog.push_back(SELHERO_DIALOG_HERO_IMG);

	SDL_Rect rect4 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	vecSelHeroDialog.push_back(new UiText("Level:", rect4, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect5 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP, 40, 21 };
	vecSelHeroDialog.push_back(new UiText(textStats[0], rect5, UIS_CENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect6 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 0 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	vecSelHeroDialog.push_back(new UiText("Strength:", rect6, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect7 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 0 * 21, 40, 21 };
	vecSelHeroDialog.push_back(new UiText(textStats[1], rect7, UIS_CENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect8 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 1 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	vecSelHeroDialog.push_back(new UiText("Magic:", rect8, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect9 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 1 * 21, 40, 21 };
	vecSelHeroDialog.push_back(new UiText(textStats[2], rect9, UIS_CENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect10 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 2 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	vecSelHeroDialog.push_back(new UiText("Dexterity:", rect10, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect11 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 2 * 21, 40, 21 };
	vecSelHeroDialog.push_back(new UiText(textStats[3], rect11, UIS_CENTER | UIS_SMALL | UIS_SILVER));

	SDL_Rect rect12 = { SELHERO_HEROS_LEFT, SELHERO_LCONTENT_TOP + 35 + 3 * 21, SELHERO_HEROS_WIDTH / 2 + 30, 21 };
	vecSelHeroDialog.push_back(new UiText("Vitality:", rect12, UIS_RIGHT | UIS_SMALL | UIS_SILVER));
	SDL_Rect rect13 = { SELHERO_HEROS_LEFT + SELHERO_HEROS_WIDTH / 2 + 40, SELHERO_LCONTENT_TOP + 35 + 3 * 21, 40, 21 };
	vecSelHeroDialog.push_back(new UiText(textStats[4], rect13, UIS_CENTER | UIS_SMALL | UIS_SILVER));
}

static void SelheroListEsc()
{
	selhero_endMenu = true;
	selhero_result = SELHERO_PREVIOUS;
}

static void SelheroListFocus(unsigned index)
{
	SelheroUpdateViewportItems();
	int baseFlags = UIS_CENTER | UIS_BIG;
	if (index < selhero_SaveCount) {
		memcpy(&selhero_heroInfo, &selhero_heros[index], sizeof(selhero_heroInfo));
		SelheroSetStats();
		SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_GOLD;
		selhero_deleteEnabled = true;
		return;
	}

	SELHERO_DIALOG_HERO_IMG->m_frame = 0;
	copy_cstr(textStats[0], "--");
	copy_cstr(textStats[1], "--");
	copy_cstr(textStats[2], "--");
	copy_cstr(textStats[3], "--");
	copy_cstr(textStats[4], "--");
	SELLIST_DIALOG_DELETE_BUTTON->m_iFlags = baseFlags | UIS_SILVER | UIS_DISABLED;
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

	SDL_Rect rect1 = { SELHERO_RPANEL_LEFT, SELHERO_PNL_TOP, SELHERO_RPANEL_WIDTH, SELHERO_RHEADER_HEIGHT };
	gUiItems.push_back(new UiText("Select Hero", rect1, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	unsigned num_viewport_heroes = std::min(selhero_SaveCount + 1, MAX_VIEWPORT_ITEMS);
	for (unsigned i = 0; i < num_viewport_heroes; i++) {
		gUIListItems.push_back(new UiListItem("", -1));
	}
	SelheroUpdateViewportItems();

	SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + 25, SELCONN_LIST_TOP, SELHERO_RPANEL_WIDTH - 2 * 25, 26 * (int)num_viewport_heroes };
	gUiItems.push_back(new UiList(&gUIListItems, num_viewport_heroes, rect2, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

	SDL_Rect rect3 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH - SCROLLBAR_BG_WIDTH + 1, SELHERO_RPANEL_TOP - 1, SCROLLBAR_BG_WIDTH, SELHERO_RPANEL_HEIGHT + 1 };
	UiScrollBar* scrollBar = new UiScrollBar(rect3);
	gUiItems.push_back(scrollBar);

	SDL_Rect rect4 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect4, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect5 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 3, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 3, 35 };
	SELLIST_DIALOG_DELETE_BUTTON = new UiTxtButton("Delete", &SelheroUiFocusNavigationYesNo, rect5, UIS_CENTER | UIS_BIG | UIS_SILVER | UIS_DISABLED);
	gUiItems.push_back(SELLIST_DIALOG_DELETE_BUTTON);

	SDL_Rect rect6 = { SELHERO_RPANEL_LEFT + 2 * SELHERO_RPANEL_WIDTH / 3, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 3, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect6, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(selhero_SaveCount + 1, SelheroListFocus, SelheroListSelect, SelheroListEsc, SelheroListDeleteYesNo);
	UiInitScrollBar(scrollBar, MAX_VIEWPORT_ITEMS);
	snprintf(selhero_title, sizeof(selhero_title), "%s Player Characters", selconn_bMulti ? "Multi" : "Single");
}

static void SelheroClassSelectorEsc()
{
	SelheroFreeDlgItems();

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
	//_uidefaultstats defaults;
	//gfnHeroStats(index, &defaults);
	assert((unsigned)gUIListItems[index]->m_value == index);

	selhero_heroInfo.hiIdx = MAX_CHARACTERS;
	selhero_heroInfo.hiLevel = 1;
	selhero_heroInfo.hiClass = index;
	//selhero_heroInfo.hiRank = 0;
	selhero_heroInfo.hiStrength = StrengthTbl[index]; //defaults.dsStrength;
	selhero_heroInfo.hiMagic = MagicTbl[index]; //defaults.dsMagic;
	selhero_heroInfo.hiDexterity = DexterityTbl[index]; //defaults.dsDexterity;
	selhero_heroInfo.hiVitality = VitalityTbl[index]; //defaults.dsVitality;
	//selhero_heroInfo.hiHasSaved = FALSE;

	SelheroSetStats();
}

static void SelheroLoadSelect(unsigned index)
{
	selhero_endMenu = true;
	selhero_result = index == 0 ? SELHERO_CONTINUE : SELHERO_NEW_DUNGEON;
}

static void SelheroListSelect(unsigned index)
{
	if (index == selhero_SaveCount) {
		SelheroFreeDlgItems();

		SDL_Rect rect1 = { SELHERO_RPANEL_LEFT, SELHERO_PNL_TOP, SELHERO_RPANEL_WIDTH, SELHERO_RHEADER_HEIGHT };
		gUiItems.push_back(new UiText("Choose Class", rect1, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

		gUIListItems.push_back(new UiListItem("Warrior", PC_WARRIOR));
		gUIListItems.push_back(new UiListItem("Rogue", PC_ROGUE));
		gUIListItems.push_back(new UiListItem("Sorcerer", PC_SORCERER));
#ifdef HELLFIRE
		gUIListItems.push_back(new UiListItem("Monk", PC_MONK));
		gUIListItems.push_back(new UiListItem("Bard", PC_BARD));
		gUIListItems.push_back(new UiListItem("Barbarian", PC_BARBARIAN));
#endif
		//assert(gUIListItems.size() == NUM_CLASSES);
		SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + (SELHERO_RPANEL_WIDTH - 270) / 2, SELCONN_LIST_TOP, 270, 26 * NUM_CLASSES };
		gUiItems.push_back(new UiList(&gUIListItems, NUM_CLASSES, rect2, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect3 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
		gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect4 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 2, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
		gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_CENTER | UIS_BIG | UIS_GOLD));

		//assert(gUIListItems.size() == NUM_CLASSES);
		UiInitList(NUM_CLASSES, SelheroClassSelectorFocus, SelheroClassSelectorSelect, SelheroClassSelectorEsc);
		memset(&selhero_heroInfo.hiName, 0, sizeof(selhero_heroInfo.hiName));
		snprintf(selhero_title, sizeof(selhero_title), "New %s Player Hero", selconn_bMulti ? "Multi" : "Single");
		return;
	}

	if (selhero_heroInfo.hiHasSaved) {
		SelheroFreeDlgItems();

		SDL_Rect rect1 = { SELHERO_RPANEL_LEFT, SELHERO_PNL_TOP, SELHERO_RPANEL_WIDTH, SELHERO_RHEADER_HEIGHT };
		gUiItems.push_back(new UiText("Save File Exists", rect1, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

		gUIListItems.push_back(new UiListItem("Load Game", 0));
		gUIListItems.push_back(new UiListItem("New Game", 1));
		SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + (SELHERO_RPANEL_WIDTH - 280) / 2, SELCONN_LIST_TOP, 280, 26 * 2 };
		gUiItems.push_back(new UiList(&gUIListItems, 2, rect2, UIS_CENTER | UIS_VCENTER | UIS_MED | UIS_GOLD));

		SDL_Rect rect3 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
		gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		SDL_Rect rect4 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 2, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
		gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_GOLD));

		//assert(gUIListItems.size() == 2);
		UiInitList(2, SelheroLoadFocus, SelheroLoadSelect, SelheroListInit);
		copy_cstr(selhero_title, "Single Player Characters");
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
	SelheroFreeDlgItems();

	snprintf(selhero_title, sizeof(selhero_title), "New %s Player Hero", selconn_bMulti ? "Multi" : "Single");
	memset(selhero_heroInfo.hiName, '\0', sizeof(selhero_heroInfo.hiName));
#if defined(PREFILL_PLAYER_NAME) || HAS_GAMECTRL || HAS_JOYSTICK || HAS_KBCTRL || HAS_DPAD
#ifndef PREFILL_PLAYER_NAME
	if (sgbControllerActive)
#endif
		SStrCopy(selhero_heroInfo.hiName, SelheroGenerateName(selhero_heroInfo.hiClass), sizeof(selhero_heroInfo.hiName));
#endif
	SDL_Rect rect1 = { SELHERO_RPANEL_LEFT, SELHERO_PNL_TOP, SELHERO_RPANEL_WIDTH, SELHERO_RHEADER_HEIGHT };
	gUiItems.push_back(new UiText("Enter Name", rect1, UIS_CENTER | UIS_VCENTER | UIS_BIG | UIS_SILVER));

	SDL_Rect rect2 = { SELHERO_RPANEL_LEFT + 24, SELHERO_RPANEL_TOP + (SELHERO_RPANEL_HEIGHT - FOCUS_MEDIUM) / 2, SELHERO_RPANEL_WIDTH - 2 * 24, FOCUS_MEDIUM };
	gUiItems.push_back(new UiEdit("Enter Name", selhero_heroInfo.hiName, sizeof(selhero_heroInfo.hiName) - 1, rect2));

	SDL_Rect rect3 = { SELHERO_RPANEL_LEFT, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("OK", &UiFocusNavigationSelect, rect3, UIS_CENTER | UIS_BIG | UIS_GOLD));

	SDL_Rect rect4 = { SELHERO_RPANEL_LEFT + SELHERO_RPANEL_WIDTH / 2, SELHERO_RBUTTON_TOP, SELHERO_RPANEL_WIDTH / 2, 35 };
	gUiItems.push_back(new UiTxtButton("Cancel", &UiFocusNavigationEsc, rect4, UIS_CENTER | UIS_BIG | UIS_GOLD));

	UiInitList(0, NULL, SelheroNameSelect, SelheroNameEsc);
}

static void SelheroNameSelect(unsigned index)
{
	const char* err;
	int result = gfnHeroCreate(&selhero_heroInfo);

	switch (result) {
	case NEWHERO_DONE:
		SelheroLoadSelect(1);
		return;
	case NEWHERO_INVALID_NAME:
		err = "Invalid name.\nA name cannot contain reserved characters.";
		break;
	case NEWHERO_HERO_LIMIT:
		err = "Hero count limit reached.\nDelete a hero before creating a new one.";
		break;
	case NEWHERO_FAIL:
		//std::vector<UiItemBase*> allItems = vecSelHeroDialog;
		//allItems.insert(allItems.end(), gUiItems.begin(), gUiItems.end());
		//UiErrorOkDialog("Error", "Unable to create character.", &allItems);
		err = "Unable to create hero.\nStorage device is full or read-only.";
		break;
	default:
		ASSUME_UNREACHABLE
		break;
	}

	MemFreeDbg(gbBackCel);
	SelheroFreeDlgItems();
	//UiInitList_clear();
	UiSelOkDialog(selhero_title, err);
	LoadBackgroundArt("ui_art\\selhero.CEL", "ui_art\\menu.pal");
	SelheroClassSelectorSelect(0);
}

static void SelHeroGetHeroInfo(_uiheroinfo *pInfo)
{
	selhero_heros.emplace_back(*pInfo);
	selhero_SaveCount++;
}

int UiSelHeroDialog(void (*fninfo)(void (*fninfofunc)(_uiheroinfo *)),
	int (*fncreate)(_uiheroinfo *),
	void (*fnremove)(_uiheroinfo *),
	//void (*fnstats)(unsigned int, _uidefaultstats *),
	unsigned* saveIdx)
{
	gfnHeroCreate = fncreate;
	while (TRUE) {
		SelheroInit();

		//gfnHeroStats = fnstats;
		//selhero_result = 0;

		selhero_navigateYesNo = false;

		selhero_SaveCount = 0;
		fninfo(SelHeroGetHeroInfo);
		std::reverse(selhero_heros.begin(), selhero_heros.end());

		if (selhero_SaveCount != 0) {
			SelheroListInit();
		} else {
			SelheroListSelect(0);
		}

		selhero_endMenu = false;
		do {
			UiClearScreen();
			UiRenderItems(vecSelHeroDialog);
			UiPollAndRender();
		} while (!selhero_endMenu && !selhero_navigateYesNo);
		SelheroFree();

		if (selhero_endMenu)
			break;
		char dialogTitle[32];
		char dialogText[256];
		snprintf(dialogTitle, sizeof(dialogTitle), "Delete %s Player Hero", selconn_bMulti ? "Multi" : "Single");
		snprintf(dialogText, sizeof(dialogText), "Are you sure you want to delete the character \"%s\"?", selhero_heroInfo.hiName);

		if (UiSelHeroYesNoDialog(dialogTitle, dialogText))
			fnremove(&selhero_heroInfo);
	}

	*saveIdx = selhero_heroInfo.hiIdx;

	return selhero_result;
}

DEVILUTION_END_NAMESPACE
