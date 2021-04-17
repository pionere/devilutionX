#include "diabloui.h"

#include "utils/stubs.h"
#include "utils/utf8.h"
#include <string>

#include "controls/menu_controls.h"

#include "DiabloUI/scrollbar.h"
#include "DiabloUI/art_draw.h"
#include "DiabloUI/text_draw.h"
#include "DiabloUI/fonts.h"
#include "DiabloUI/button.h"
#include "DiabloUI/dialogs.h"
#include "controls/plrctrls.h"

#ifdef __SWITCH__
// for virtual keyboard on Switch
#include "platform/switch/keyboard.h"
#endif
#ifdef __vita__
// for virtual keyboard on Vita
#include "platform/vita/keyboard.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

unsigned SelectedItem = 0;
unsigned SelectedItemMax;
unsigned ListViewportSize = 1;
unsigned ListOffset = 0;

Art ArtLogos[3];
Art ArtFocus[3];
#ifdef WIDESCREEN
Art ArtBackgroundWidescreen;
#endif
Art ArtBackground;
Art ArtCursor;
Art ArtHero;

void (*gfnSoundFunction)(const char *file);
void (*gfnListFocus)(unsigned index);
void (*gfnListSelect)(unsigned index);
void (*gfnListEsc)();
bool (*gfnListYesNo)();
std::vector<UiItemBase *> gUiItems;
bool UiItemsWraps;
char *UiTextInput;
int UiTextInputLen;
bool textInputActive = true;

static void LoadPalInMem(const SDL_Color (&pPal)[lengthof(orig_palette)]);

static DWORD _gdwFadeTc;
static int _gnFadeValue = 0;

struct scrollBarState {
	bool upArrowPressed;
	bool downArrowPressed;

	scrollBarState()
	{
		upArrowPressed = false;
		downArrowPressed = false;
	}
} scrollBarState;

void UiInitList(std::vector<UiItemBase *> uiItems, unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)(), bool (*fnYesNo)(), bool itemsWraps)
{
	SelectedItem = 0;
	SelectedItemMax = listSize != 0 ? listSize - 1 : 0;
	ListViewportSize = listSize;
	ListOffset = 0;
	gfnListFocus = fnFocus;
	gfnListSelect = fnSelect;
	gfnListEsc = fnEsc;
	gfnListYesNo = fnYesNo;
	gUiItems = uiItems;
	UiItemsWraps = itemsWraps;
	if (fnFocus)
		fnFocus(0);

#ifndef __SWITCH__
	SDL_StopTextInput(); // input is enabled by default
#endif
	textInputActive = false;
	for (unsigned i = 0; i < uiItems.size(); i++) {
		if (uiItems[i]->m_type == UI_EDIT) {
			UiEdit *pItemUIEdit = (UiEdit *)uiItems[i];
			SDL_SetTextInputRect(&uiItems[i]->m_rect);
			textInputActive = true;
#ifdef __SWITCH__
			switch_start_text_input("", pItemUIEdit->m_value, pItemUIEdit->m_max_length, /*multiline=*/0);
#elif defined(__vita__)
			vita_start_text_input("", pItemUIEdit->m_value, pItemUIEdit->m_max_length);
#else
			SDL_StartTextInput();
#endif
			UiTextInput = pItemUIEdit->m_value;
			UiTextInputLen = pItemUIEdit->m_max_length;
		}
	}
}

void UiInitScrollBar(UiScrollBar *ui_sb, unsigned viewport_size)
{
	ListViewportSize = viewport_size;
	if (ListViewportSize > SelectedItemMax) {
		ui_sb->add_flag(UIS_HIDDEN);
	} else {
		ui_sb->remove_flag(UIS_HIDDEN);
	}
}

void UiInitList_clear()
{
	SelectedItem = 0;
	SelectedItemMax = 0;
	ListViewportSize = 1;
	gfnListFocus = NULL;
	gfnListSelect = NULL;
	gfnListEsc = NULL;
	gfnListYesNo = NULL;
	gUiItems.clear();
	UiItemsWraps = false;
}

void UiPlayMoveSound()
{
	if (gfnSoundFunction)
		gfnSoundFunction("sfx\\items\\titlemov.wav");
}

void UiPlaySelectSound()
{
	if (gfnSoundFunction)
		gfnSoundFunction("sfx\\items\\titlslct.wav");
}

namespace {

void UiFocus(unsigned itemIndex)
{
	if (SelectedItem == itemIndex)
		return;

	SelectedItem = itemIndex;

	UiPlayMoveSound();

	if (gfnListFocus)
		gfnListFocus(itemIndex);
}

void UiFocusUp()
{
	if (SelectedItem > 0)
		UiFocus(SelectedItem - 1);
	else if (UiItemsWraps)
		UiFocus(SelectedItemMax);
}

void UiFocusDown()
{
	if (SelectedItem < SelectedItemMax)
		UiFocus(SelectedItem + 1);
	else if (UiItemsWraps)
		UiFocus(0);
}

// UiFocusPageUp/Down mimics the slightly weird behaviour of actual Diablo.

void UiFocusPageUp()
{
	unsigned page_start = ListOffset;
	if (page_start == 0) {
		UiFocus(0);
	} else {
		unsigned relpos = SelectedItem - page_start;
		if (page_start >= ListViewportSize)
			page_start -= ListViewportSize;
		else
			page_start = 0;
		UiFocus(page_start);
		UiFocus(page_start + relpos);
	}
}

void UiFocusPageDown()
{
	unsigned page_end = ListOffset + ListViewportSize;
	if (page_end > SelectedItemMax || page_end == 0) {
		UiFocus(SelectedItemMax);
	} else {
		page_end--;
		unsigned relpos = page_end - SelectedItem;
		if (page_end + ListViewportSize <= SelectedItemMax)
			page_end += ListViewportSize;
		else
			page_end = SelectedItemMax;
		UiFocus(page_end);
		UiFocus(page_end - relpos);
	}
}

static void selhero_CatToName(char *in_buf, char *out_buf, int cnt)
{
	std::string output = utf8_to_latin1(in_buf);
	int pos = strlen(out_buf);
	SStrCopy(&out_buf[pos], output.c_str(), cnt - pos);
}

#ifdef __vita__
static void selhero_SetName(char *in_buf, char *out_buf, int cnt)
{
	std::string output = utf8_to_latin1(in_buf);
	strncpy(out_buf, output.c_str(), cnt);
}
#endif

bool HandleMenuAction(MenuAction menu_action)
{
	switch (menu_action) {
	case MenuAction_SELECT:
		UiFocusNavigationSelect();
		return true;
	case MenuAction_UP:
		UiFocusUp();
		return true;
	case MenuAction_DOWN:
		UiFocusDown();
		return true;
	case MenuAction_PAGE_UP:
		UiFocusPageUp();
		return true;
	case MenuAction_PAGE_DOWN:
		UiFocusPageDown();
		return true;
	case MenuAction_DELETE:
		UiFocusNavigationYesNo();
		return true;
	case MenuAction_BACK:
		if (!gfnListEsc)
			return false;
		UiFocusNavigationEsc();
		return true;
	default:
		return false;
	}
}

} // namespace

void UiFocusNavigation(SDL_Event *event)
{
	switch (event->type) {
	case SDL_KEYUP:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
#ifndef USE_SDL1
	case SDL_MOUSEWHEEL:
#endif
	case SDL_JOYBUTTONUP:
	case SDL_JOYAXISMOTION:
	case SDL_JOYBALLMOTION:
	case SDL_JOYHATMOTION:
#ifndef USE_SDL1
	case SDL_FINGERUP:
	case SDL_FINGERMOTION:
	case SDL_CONTROLLERBUTTONUP:
	case SDL_CONTROLLERAXISMOTION:
	case SDL_WINDOWEVENT:
#endif
	case SDL_SYSWMEVENT:
		mainmenu_restart_repintro();
		break;
	}

	if (HandleMenuAction(GetMenuAction(*event)))
		return;

#ifndef USE_SDL1
	if (event->type == SDL_MOUSEWHEEL) {
		if (event->wheel.y > 0) {
			UiFocusUp();
		} else if (event->wheel.y < 0) {
			UiFocusDown();
		}
		return;
	}
#endif

	if (textInputActive) {
		switch (event->type) {
		case SDL_KEYDOWN: {
			switch (event->key.keysym.sym) {
#ifndef USE_SDL1
			case SDLK_v:
				if (SDL_GetModState() & KMOD_CTRL) {
					char *clipboard = SDL_GetClipboardText();
					if (clipboard == NULL) {
						SDL_Log("%s", SDL_GetError());
					} else {
						selhero_CatToName(clipboard, UiTextInput, UiTextInputLen);
					}
				}
				return;
#endif
			case SDLK_BACKSPACE:
			case SDLK_LEFT: {
				int nameLen = strlen(UiTextInput);
				if (nameLen > 0) {
					UiTextInput[nameLen - 1] = '\0';
				}
				return;
			}
			default:
				break;
			}
#ifdef USE_SDL1
			if ((event->key.keysym.mod & KMOD_CTRL) == 0) {
				Uint16 unicode = event->key.keysym.unicode;
				if (unicode && (unicode & 0xFF80) == 0) {
					char utf8[SDL_TEXTINPUTEVENT_TEXT_SIZE];
					utf8[0] = (char)unicode;
					utf8[1] = '\0';
					selhero_CatToName(utf8, UiTextInput, UiTextInputLen);
				}
			}
#endif
			break;
		}
#ifndef USE_SDL1
		case SDL_TEXTINPUT:
#ifdef __vita__
			selhero_SetName(event->text.text, UiTextInput, UiTextInputLen);
#else
			selhero_CatToName(event->text.text, UiTextInput, UiTextInputLen);
#endif
			return;
#endif
		default:
			break;
		}
	}

	if (event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
		if (UiItemMouseEvents(event, gUiItems))
			return;
	}
}

void UiHandleEvents(SDL_Event *event)
{
	if (event->type == SDL_MOUSEMOTION) {
#ifdef USE_SDL1
		OutputToLogical(&event->motion.x, &event->motion.y);
#endif
		MouseX = event->motion.x;
		MouseY = event->motion.y;
		return;
	}

	if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
		const Uint8 *state = SDLC_GetKeyState();
		if (state[SDLC_KEYSTATE_LALT] || state[SDLC_KEYSTATE_RALT]) {
			dx_reinit();
			return;
		}
	}

	if (event->type == SDL_QUIT)
		diablo_quit(0);

#ifndef USE_SDL1
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	HandleControllerAddedOrRemovedEvent(*event);
#endif

	if (event->type == SDL_WINDOWEVENT) {
		if (event->window.event == SDL_WINDOWEVENT_SHOWN)
			gbActive = true;
		else if (event->window.event == SDL_WINDOWEVENT_HIDDEN)
			gbActive = false;
	}
#endif
}

void UiFocusNavigationSelect()
{
	UiPlaySelectSound();
	if (textInputActive) {
		if (strlen(UiTextInput) == 0) {
			return;
		}
#ifndef __SWITCH__
		SDL_StopTextInput();
#endif
		UiTextInput = NULL;
		UiTextInputLen = 0;
	}
	if (gfnListSelect)
		gfnListSelect(SelectedItem);
}

void UiFocusNavigationEsc()
{
	UiPlaySelectSound();
	if (textInputActive) {
#ifndef __SWITCH__
		SDL_StopTextInput();
#endif
		UiTextInput = NULL;
		UiTextInputLen = 0;
	}
	if (gfnListEsc)
		gfnListEsc();
}

void UiFocusNavigationYesNo()
{
	if (gfnListYesNo == NULL)
		return;

	if (gfnListYesNo())
		UiPlaySelectSound();
}

static bool IsInsideRect(const SDL_Event &event, const SDL_Rect &rect)
{
	const SDL_Point point = { event.button.x, event.button.y };
	return SDL_PointInRect(&point, &rect);
}

static void LoadUiGFX()
{
#ifdef HELLFIRE
	LoadMaskedArt("ui_art\\hf_logo2.pcx", &ArtLogos[LOGO_MED], 16);
#else
	LoadMaskedArt("ui_art\\smlogo.pcx", &ArtLogos[LOGO_MED], 15);
#endif
	LoadMaskedArt("ui_art\\focus16.pcx", &ArtFocus[FOCUS_SMALL], 8);
	LoadMaskedArt("ui_art\\focus.pcx", &ArtFocus[FOCUS_MED], 8);
	LoadMaskedArt("ui_art\\focus42.pcx", &ArtFocus[FOCUS_BIG], 8);
	LoadMaskedArt("ui_art\\cursor.pcx", &ArtCursor, 1, 0);
#ifdef HELLFIRE
	LoadArt("ui_art\\heros.pcx", &ArtHero, 6);
#else
	LoadArt("ui_art\\heros.pcx", &ArtHero, 4);
#endif
}

static void UnloadUiGFX()
{
	int i;

	for (i = 0; i < lengthof(ArtLogos); i++)
		ArtLogos[i].Unload();
	for (i = 0; i < lengthof(ArtFocus); i++)
		ArtFocus[i].Unload();
	ArtCursor.Unload();
	ArtHero.Unload();
}

void UiInitialize()
{
	LoadUiGFX();
	LoadArtFonts();
	if (ArtCursor.surface != NULL) {
		if (SDL_ShowCursor(SDL_DISABLE) <= -1) {
			ErrSdl();
		}
	}
}

void UiDestroy()
{
	UnloadUiGFX();
	UnloadTtfFont();
	UnloadArtFonts();
	//UiInitList_clear();
}

char connect_plrinfostr[128];
char connect_categorystr[128];
void UiSetupPlayerInfo(const char *infostr, const _uiheroinfo *pInfo, DWORD type)
{
	SStrCopy(connect_plrinfostr, infostr, sizeof(connect_plrinfostr));
	const char fmt[] = " %d %d %d %d %d %d %d %d";
	char format[sizeof(DWORD) + sizeof(fmt)];
	*(DWORD *)format = type;
	memcpy(&format[sizeof(DWORD)], fmt, sizeof(fmt));

	snprintf(
	    connect_categorystr,
	    sizeof(connect_categorystr),
	    format,
	    pInfo->level,
	    pInfo->heroclass,
	    pInfo->herorank,
	    pInfo->strength,
	    pInfo->magic,
	    pInfo->dexterity,
	    pInfo->vitality,
	    pInfo->gold);
}

bool UiValidPlayerName(const char *name)
{
	if (!strlen(name))
		return false;

	if (strpbrk(name, ",<>%&\\\"?*#/:") || strpbrk(name, " "))
		return false;

	for (BYTE *letter = (BYTE *)name; *letter; letter++)
		if (*letter < 0x20 || (*letter > 0x7E && *letter < 0xC0))
			return false;

	const char *const reserved[] = {
		"gvdl",
		"dvou",
		"tiju",
		"cjudi",
		"bttipmf",
		"ojhhfs",
		"cmj{{bse",
		"benjo",
	};

	char tmpname[PLR_NAME_LEN];
	SStrCopy(tmpname, name, PLR_NAME_LEN);
	for (BYTE *letter = (BYTE *)tmpname; *letter; letter++)
		++*letter;

	for (int i = 0; i < lengthof(reserved); i++) {
		if (strstr(tmpname, reserved[i]))
			return false;
	}

	return true;
}

void UiCreatePlayerDescription(const _uiheroinfo *info, DWORD mode, char (&desc)[128])
{
	const char fmt[] = " %d %d %d %d %d %d %d %d";
	char format[sizeof(DWORD) + sizeof(fmt)];
	*(DWORD *)format = mode;
	memcpy(&format[sizeof(DWORD)], fmt, sizeof(fmt));

	snprintf(
	    desc,
	    sizeof(desc),
	    format,
	    info->level,
	    info->heroclass,
	    info->herorank,
	    info->strength,
	    info->magic,
	    info->dexterity,
	    info->vitality,
	    info->gold);
}

int GetCenterOffset(int w, int bw)
{
	if (bw == 0) {
		bw = SCREEN_WIDTH;
	}

	return (bw - w) / 2;
}

void LoadBackgroundArt(const char *pszFile, int frames)
{
	SDL_Color pPal[256];

	assert(ArtBackground.surface == NULL);

	LoadArt(pszFile, &ArtBackground, frames, pPal);
	if (ArtBackground.surface == NULL)
		return;

	LoadPalInMem(pPal);
	ApplyGamma(logical_palette, orig_palette, 256);

	_gdwFadeTc = 0;
	_gnFadeValue = 0;
	BlackPalette();
	SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
	RenderPresent();
}

void UiAddBackground(std::vector<UiItemBase *> *vecDialog)
{
#ifdef WIDESCREEN
	if (ArtBackgroundWidescreen.surface != NULL) {
		SDL_Rect rectw = { 0, UI_OFFSET_Y, 0, 0 };
		vecDialog->push_back(new UiImage(&ArtBackgroundWidescreen, /*animated=*/false, /*frame=*/0, rectw, UIS_CENTER));
	}
#endif

	SDL_Rect rect = { 0, UI_OFFSET_Y, 0, 0 };
	vecDialog->push_back(new UiImage(&ArtBackground, /*animated=*/false, /*frame=*/0, rect, UIS_CENTER));
}

void UiAddLogo(std::vector<UiItemBase *> *vecDialog, int size, int y)
{
	SDL_Rect rect = { 0, UI_OFFSET_Y + y, 0, 0 };
	vecDialog->push_back(new UiImage(&ArtLogos[size], /*animated=*/true, /*frame=*/0, rect, UIS_CENTER));
}

void UiFadeIn()
{
	if (_gnFadeValue < 256) {
		if (_gnFadeValue == 0 && _gdwFadeTc == 0)
			_gdwFadeTc = SDL_GetTicks();
		_gnFadeValue = (SDL_GetTicks() - _gdwFadeTc) >> 1; // instead of >> 1 it was / 2.083 ... 32 frames @ 60hz
		if (_gnFadeValue > 256) {
			_gnFadeValue = 256;
			_gdwFadeTc = 0;
		}
		SetFadeLevel(_gnFadeValue);
	}
	RenderPresent();
}

void DrawSelector(const SDL_Rect &rect)
{
	int size = FOCUS_SMALL;
	if (rect.h >= 42)
		size = FOCUS_BIG;
	else if (rect.h >= 30)
		size = FOCUS_MED;
	Art *art = &ArtFocus[size];

	int frame = GetAnimationFrame(art->frames);
	int y = rect.y + (rect.h - art->h()) / 2; // TODO FOCUS_MED appares higher then the box

	DrawArt(rect.x, y, art, frame);
	DrawArt(rect.x + rect.w - art->w(), y, art, frame);
}

void UiClearScreen()
{
	if (SCREEN_WIDTH > 640) // Background size
		SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
}

void UiPollAndRender()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		UiFocusNavigation(&event);
		UiHandleEvents(&event);
	}
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	HandleMenuAction(GetMenuHeldUpDownAction());
#endif
	UiRenderItems(gUiItems);
	DrawMouse();
	UiFadeIn();
}

namespace {

void Render(UiText *ui_text)
{
	DrawTTF(ui_text->m_text,
	    ui_text->m_rect,
	    ui_text->m_iFlags,
	    ui_text->m_color,
	    ui_text->m_shadow_color,
	    &ui_text->m_render_cache);
}

void Render(const UiArtText *ui_art_text)
{
	DrawArtStr(ui_art_text->m_text, ui_art_text->m_rect, ui_art_text->m_iFlags);
}

void Render(const UiImage *ui_image)
{
	int x = ui_image->m_rect.x;
	if ((ui_image->m_iFlags & UIS_CENTER) && ui_image->m_art != NULL) {
		const int x_offset = GetCenterOffset(ui_image->m_art->w(), ui_image->m_rect.w);
		x += x_offset;
	}
	if (ui_image->m_animated) {
		DrawAnimatedArt(ui_image->m_art, x, ui_image->m_rect.y);
	} else {
		DrawArt(x, ui_image->m_rect.y, ui_image->m_art, ui_image->m_frame, ui_image->m_rect.w, ui_image->m_rect.h);
	}
}

void Render(const UiArtTextButton *ui_button)
{
	DrawArtStr(ui_button->m_text, ui_button->m_rect, ui_button->m_iFlags);
}

void Render(const UiList *ui_list)
{
	for (unsigned i = 0; i < ui_list->m_vecItems.size(); ++i) {
		SDL_Rect rect = ui_list->itemRect(i);
		const UiListItem *item = ui_list->GetItem(i);
		if (i + ListOffset == SelectedItem)
			DrawSelector(rect);
		DrawArtStr(item->m_text, rect, ui_list->m_iFlags);
	}
}

void Render(const UiScrollBar *ui_sb)
{
	// Bar background (tiled):
	{
		const int bg_y_end = DownArrowRect(ui_sb).y;
		int bg_y = ui_sb->m_rect.y + ui_sb->m_arrow->h();
		while (bg_y < bg_y_end) {
			int drawH = std::min(bg_y + ui_sb->m_bg->h(), bg_y_end) - bg_y;
			DrawArt(ui_sb->m_rect.x, bg_y, ui_sb->m_bg, 0, SCROLLBAR_BG_WIDTH, drawH);
			bg_y += drawH;
		}
	}

	// Arrows:
	{
		const SDL_Rect rect = UpArrowRect(ui_sb);
		const int frame = static_cast<int>(scrollBarState.upArrowPressed ? ScrollBarArrowFrame_UP_ACTIVE : ScrollBarArrowFrame_UP);
		DrawArt(rect.x, rect.y, ui_sb->m_arrow, frame, rect.w);
	}
	{
		const SDL_Rect rect = DownArrowRect(ui_sb);
		const int frame = static_cast<int>(scrollBarState.downArrowPressed ? ScrollBarArrowFrame_DOWN_ACTIVE : ScrollBarArrowFrame_DOWN);
		DrawArt(rect.x, rect.y, ui_sb->m_arrow, frame, rect.w);
	}

	// Thumb:
	if (SelectedItemMax > 0) {
		const SDL_Rect rect = ThumbRect(ui_sb, SelectedItem, SelectedItemMax + 1);
		DrawArt(rect.x, rect.y, ui_sb->m_thumb);
	}
}

void Render(const UiEdit *ui_edit)
{
	DrawSelector(ui_edit->m_rect);
	SDL_Rect rect = ui_edit->m_rect;
	rect.x += 43;
	rect.y += 1;
	rect.w -= 86;
	DrawArtStr(ui_edit->m_value, rect, ui_edit->m_iFlags, /*drawTextCursor=*/true);
}

void RenderItem(UiItemBase *item)
{
	if (item->has_flag(UIS_HIDDEN))
		return;
	switch (item->m_type) {
	case UI_TEXT:
		Render(static_cast<UiText *>(item));
		break;
	case UI_ART_TEXT:
		Render(static_cast<UiArtText *>(item));
		break;
	case UI_IMAGE:
		Render(static_cast<UiImage *>(item));
		break;
	case UI_ART_TEXT_BUTTON:
		Render(static_cast<UiArtTextButton *>(item));
		break;
	case UI_BUTTON:
		RenderButton(static_cast<UiButton *>(item));
		break;
	case UI_LIST:
		Render(static_cast<UiList *>(item));
		break;
	case UI_SCROLLBAR:
		Render(static_cast<UiScrollBar *>(item));
		break;
	case UI_EDIT:
		Render(static_cast<UiEdit *>(item));
		break;
	}
}

bool HandleMouseEventArtTextButton(const SDL_Event &event, const UiArtTextButton *ui_button)
{
	if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT)
		return false;
	ui_button->m_action();
	return true;
}

#ifdef USE_SDL1
Uint32 dbClickTimer;
#endif

bool HandleMouseEventList(const SDL_Event &event, UiList *ui_list)
{
	if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT)
		return false;

	const unsigned index = ui_list->indexAt(event.button.y) + ListOffset;

	if (gfnListFocus != NULL && SelectedItem != index) {
		UiFocus(index);
#ifdef USE_SDL1
		dbClickTimer = SDL_GetTicks();
	} else if (gfnListFocus == NULL || dbClickTimer + 500 >= SDL_GetTicks()) {
#else
	} else if (gfnListFocus == NULL || event.button.clicks >= 2) {
#endif
		SelectedItem = index;
		UiFocusNavigationSelect();
#ifdef USE_SDL1
	} else {
		dbClickTimer = SDL_GetTicks();
#endif
	}

	return true;
}

bool HandleMouseEventScrollBar(const SDL_Event &event, const UiScrollBar *ui_sb)
{
	if (event.button.button != SDL_BUTTON_LEFT)
		return false;
	if (event.type == SDL_MOUSEBUTTONUP) {
		if (scrollBarState.upArrowPressed && IsInsideRect(event, UpArrowRect(ui_sb))) {
			UiFocusUp();
			return true;
		} else if (scrollBarState.downArrowPressed && IsInsideRect(event, DownArrowRect(ui_sb))) {
			UiFocusDown();
			return true;
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (IsInsideRect(event, BarRect(ui_sb))) {
			// Scroll up or down based on thumb position.
			const SDL_Rect thumb_rect = ThumbRect(ui_sb, SelectedItem, SelectedItemMax + 1);
			if (event.button.y < thumb_rect.y) {
				UiFocusPageUp();
			} else if (event.button.y > thumb_rect.y + thumb_rect.h) {
				UiFocusPageDown();
			}
			return true;
		} else if (IsInsideRect(event, UpArrowRect(ui_sb))) {
			scrollBarState.upArrowPressed = true;
			return true;
		} else if (IsInsideRect(event, DownArrowRect(ui_sb))) {
			scrollBarState.downArrowPressed = true;
			return true;
		}
	}
	return false;
}

bool HandleMouseEvent(const SDL_Event &event, UiItemBase *item)
{
	if (item->has_any_flag(UIS_HIDDEN | UIS_DISABLED) || !IsInsideRect(event, item->m_rect))
		return false;
	switch (item->m_type) {
	case UI_ART_TEXT_BUTTON:
		return HandleMouseEventArtTextButton(event, (UiArtTextButton *)item);
	case UI_BUTTON:
		return HandleMouseEventButton(event, (UiButton *)item);
	case UI_LIST:
		return HandleMouseEventList(event, (UiList *)item);
	case UI_SCROLLBAR:
		return HandleMouseEventScrollBar(event, (UiScrollBar *)item);
	default:
		return false;
	}
}

} // namespace

void LoadPalInMem(const SDL_Color (&pPal)[lengthof(orig_palette)])
{
	for (int i = 0; i < lengthof(orig_palette); i++) {
		orig_palette[i] = pPal[i];
	}
}

void UiRenderItems(std::vector<UiItemBase *> uiItems)
{
	for (unsigned i = 0; i < uiItems.size(); i++)
		RenderItem((UiItemBase *)uiItems[i]);
}

bool UiItemMouseEvents(SDL_Event *event, std::vector<UiItemBase *> uiItems)
{
	if (uiItems.size() == 0) {
		return false;
	}

	// In SDL2 mouse events already use logical coordinates.
#ifdef USE_SDL1
	OutputToLogical(&event->button.x, &event->button.y);
#endif

	bool handled = false;
	for (unsigned i = 0; i < uiItems.size(); i++) {
		if (HandleMouseEvent(*event, uiItems[i])) {
			handled = true;
			break;
		}
	}

	if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
		scrollBarState.downArrowPressed = scrollBarState.upArrowPressed = false;
		for (unsigned i = 0; i < uiItems.size(); ++i) {
			UiItemBase *&item = uiItems[i];
			if (item->m_type == UI_BUTTON)
				HandleGlobalMouseUpButton((UiButton *)item);
		}
	}

	return handled;
}

void DrawMouse()
{
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	if (sgbControllerActive)
		return;
#endif

	DrawArt(MouseX, MouseY, &ArtCursor);
}

DEVILUTION_END_NAMESPACE
