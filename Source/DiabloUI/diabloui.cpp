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
#ifdef __3DS__
// for virtual keyboard on 3DS
#include "platform/ctr/keyboard.h"
#endif

DEVILUTION_BEGIN_NAMESPACE

unsigned SelectedItem = 0;
unsigned SelectedItemMax;
unsigned ListViewportSize = 1;
unsigned ListOffset = 0;

Art ArtLogos[3];
Art ArtFocus[3];
#ifndef NOWIDESCREEN
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

static Uint32 _gdwFadeTc;
static int _gnFadeValue = 0;

struct ScrollBarState {
	bool upArrowPressed;
	bool downArrowPressed;

	ScrollBarState()
	{
		upArrowPressed = false;
		downArrowPressed = false;
	}
} scrollBarState;

void UiInitList(const std::vector<UiItemBase *> &uiItems, unsigned listSize, void (*fnFocus)(unsigned index), void (*fnSelect)(unsigned index), void (*fnEsc)(), bool (*fnYesNo)(), bool itemsWraps)
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
	if (fnFocus != NULL)
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
			switch_start_text_input(pItemUIEdit->m_hint, pItemUIEdit->m_value, pItemUIEdit->m_max_length, /*multiline=*/0);
#elif defined(__vita__)
			vita_start_text_input(pItemUIEdit->m_hint, pItemUIEdit->m_value, pItemUIEdit->m_max_length);
#elif defined(__3DS__)
			ctr_vkbdInput(pItemUIEdit->m_hint, pItemUIEdit->m_value, pItemUIEdit->m_value, pItemUIEdit->m_max_length);
#else
			SDL_StartTextInput();
#endif
			UiTextInput = pItemUIEdit->m_value;
			UiTextInputLen = pItemUIEdit->m_max_length;
		}
	}
}

void UiInitScrollBar(UiScrollBar *uiSb, unsigned viewportSize)
{
	ListViewportSize = viewportSize;
	if (ListViewportSize > SelectedItemMax) {
		uiSb->m_iFlags |= UIS_HIDDEN;
	} else {
		uiSb->m_iFlags &= ~UIS_HIDDEN;
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
	if (gfnSoundFunction != NULL)
		gfnSoundFunction("sfx\\items\\titlemov.wav");
}

void UiPlaySelectSound()
{
	if (gfnSoundFunction != NULL)
		gfnSoundFunction("sfx\\items\\titlslct.wav");
}

static void UiFocus(unsigned itemIndex)
{
	if (SelectedItem == itemIndex)
		return;

	SelectedItem = itemIndex;

	UiPlayMoveSound();

	if (gfnListFocus != NULL)
		gfnListFocus(itemIndex);
}

static void UiFocusUp()
{
	if (SelectedItem > 0)
		UiFocus(SelectedItem - 1);
	else if (UiItemsWraps)
		UiFocus(SelectedItemMax);
}

static void UiFocusDown()
{
	if (SelectedItem < SelectedItemMax)
		UiFocus(SelectedItem + 1);
	else if (UiItemsWraps)
		UiFocus(0);
}

// UiFocusPageUp/Down mimics the slightly weird behaviour of actual Diablo.

static void UiFocusPageUp()
{
	unsigned pageStart = ListOffset;
	if (pageStart == 0) {
		UiFocus(0);
	} else {
		unsigned relpos = SelectedItem - pageStart;
		if (pageStart >= ListViewportSize)
			pageStart -= ListViewportSize;
		else
			pageStart = 0;
		UiFocus(pageStart);
		UiFocus(pageStart + relpos);
	}
}

static void UiFocusPageDown()
{
	unsigned pageEnd = ListOffset + ListViewportSize;
	if (pageEnd > SelectedItemMax || pageEnd == 0) {
		UiFocus(SelectedItemMax);
	} else {
		pageEnd--;
		unsigned relpos = pageEnd - SelectedItem;
		if (pageEnd + ListViewportSize <= SelectedItemMax)
			pageEnd += ListViewportSize;
		else
			pageEnd = SelectedItemMax;
		UiFocus(pageEnd);
		UiFocus(pageEnd - relpos);
	}
}

static void SelheroCatToName(char *inBuf, char *outBuf, int cnt)
{
	std::string output = utf8_to_latin1(inBuf);
	int pos = strlen(outBuf);
	SStrCopy(&outBuf[pos], output.c_str(), cnt - pos);
}

#ifdef __vita__
static void SelheroSetName(char *inBuf, char *outBuf, int cnt)
{
	std::string output = utf8_to_latin1(inBuf);
	strncpy(outBuf, output.c_str(), cnt);
}
#endif

static bool HandleMenuAction(MenuAction menuAction)
{
	switch (menuAction) {
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
		if (gfnListEsc == NULL)
			return false;
		UiFocusNavigationEsc();
		return true;
	default:
		return false;
	}
}

static void UiFocusNavigation(SDL_Event* event)
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
					if (clipboard != NULL) {
						SelheroCatToName(clipboard, UiTextInput, UiTextInputLen);
						SDL_free(clipboard);
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
					SelheroCatToName(utf8, UiTextInput, UiTextInputLen);
				}
			}
#endif
			break;
		}
#ifndef USE_SDL1
		case SDL_TEXTINPUT:
#ifdef __vita__
			SelheroSetName(event->text.text, UiTextInput, UiTextInputLen);
#else
			SelheroCatToName(event->text.text, UiTextInput, UiTextInputLen);
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
		if (GetAsyncKeyState(DVL_VK_MENU)) {
			ToggleFullscreen();
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
			gbWndActive = true;
		else if (event->window.event == SDL_WINDOWEVENT_HIDDEN)
			gbWndActive = false;
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
	if (gfnListSelect != NULL)
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
	if (gfnListEsc != NULL)
		gfnListEsc();
}

void UiFocusNavigationYesNo()
{
	if (gfnListYesNo == NULL)
		return;

	if (gfnListYesNo())
		UiPlaySelectSound();
}

static SDL_bool IsInsideRect(const SDL_Event &event, const SDL_Rect &rect)
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
		if (SDL_ShowCursor(SDL_DISABLE) < 0) {
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

bool UiValidPlayerName(const char *name)
{
	if (strlen(name) == 0)
		return false;

	if (strpbrk(name, ",<>%&\\\"?*#/:") != NULL || strpbrk(name, " ") != NULL)
		return false;

	for (BYTE *letter = (BYTE *)name; *letter != '\0'; letter++)
		if (*letter < 0x20 || (*letter > 0x7E && *letter < 0xC0))
			return false;

	const char *const bannedNames[] = {
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
	for (BYTE *letter = (BYTE *)tmpname; *letter != '\0'; letter++)
		++*letter;

	for (int i = 0; i < lengthof(bannedNames); i++) {
		if (strstr(tmpname, bannedNames[i]) != NULL)
			return false;
	}

	return true;
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
	SDL_Color pPal[lengthof(orig_palette)];

	assert(ArtBackground.surface == NULL);

	LoadArt(pszFile, &ArtBackground, frames, pPal);
	if (ArtBackground.surface == NULL)
		return;

	memcpy(orig_palette, pPal, sizeof(pPal));
	ApplyGamma(logical_palette, orig_palette, 256);

	_gdwFadeTc = 0;
	_gnFadeValue = 0;
	BlackPalette();
	SDL_FillRect(DiabloUiSurface(), NULL, 0x000000);
	if (DiabloUiSurface() == back_surface)
		BltFast(NULL, NULL);
	RenderPresent();
}

void UiAddBackground(std::vector<UiItemBase *> *vecDialog)
{
#ifndef NOWIDESCREEN
	if (ArtBackgroundWidescreen.surface != NULL) {
		SDL_Rect rectw = { 0, UI_OFFSET_Y, 0, 0 };
		vecDialog->push_back(new UiImage(&ArtBackgroundWidescreen, 0, rectw, UIS_CENTER));
	}
#endif

	SDL_Rect rect = { 0, UI_OFFSET_Y, 0, 0 };
	vecDialog->push_back(new UiImage(&ArtBackground, 0, rect, UIS_CENTER));
}

void UiAddLogo(std::vector<UiItemBase *> *vecDialog, int size, int y)
{
	SDL_Rect rect = { 0, UI_OFFSET_Y + y, 0, 0 };
	vecDialog->push_back(new UiImage(&ArtLogos[size], rect));
}

void UiFadeIn()
{
	Uint32 currTc;

	if (_gnFadeValue < 256) {
		currTc = SDL_GetTicks();
		if (_gnFadeValue == 0 && _gdwFadeTc == 0)
			_gdwFadeTc = currTc;
		_gnFadeValue = (currTc - _gdwFadeTc) >> 0; // instead of >> 0 it was / 2.083 ... 32 frames @ 60hz
		if (_gnFadeValue > 256) {
			_gnFadeValue = 256;
			//_gdwFadeTc = 0;
		}
		SetFadeLevel(_gnFadeValue);
	}
	if (DiabloUiSurface() == back_surface)
		BltFast(NULL, NULL);
	RenderPresent();
}

static void DrawSelector(const SDL_Rect &rect)
{
	int size = FOCUS_SMALL;
	if (rect.h >= 42)
		size = FOCUS_BIG;
	else if (rect.h >= 30)
		size = FOCUS_MED;
	Art *art = &ArtFocus[size];

	int frame = GetAnimationFrame(art->frames);
	int y = rect.y + (rect.h - art->h()) / 2; // TODO FOCUS_MED appares higher than the box

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
	while (SDL_PollEvent(&event) != 0) {
		UiFocusNavigation(&event);
		UiHandleEvents(&event);
	}
#if HAS_GAMECTRL == 1 || HAS_JOYSTICK == 1 || HAS_KBCTRL == 1 || HAS_DPAD == 1
	HandleMenuAction(GetMenuHeldUpDownAction());
#endif
	UiRenderItems(gUiItems);
	DrawMouse();
	UiFadeIn();
#ifdef __3DS__
	// Keyboard blocks until input is finished
	// so defer until after render and fade-in
	ctr_vkbdFlush();
#endif
}

static void Render(UiText* uiText)
{
	SDL_Color color2 = { 0, 0, 0, 0 };

	DrawTTF(uiText->m_text,
	    uiText->m_rect,
	    UIS_CENTER,
	    uiText->m_color,
	    color2,
	    uiText->m_render_cache);
}

static void Render(const UiArtText* uiArtText)
{
	DrawArtStr(uiArtText->m_text, uiArtText->m_rect, uiArtText->m_iFlags);
}

static void Render(const UiImage* uiImage)
{
	int x = uiImage->m_rect.x;
	if ((uiImage->m_iFlags & UIS_CENTER) && uiImage->m_art != NULL) {
		const int xOffset = GetCenterOffset(uiImage->m_art->w(), uiImage->m_rect.w);
		x += xOffset;
	}
	if (uiImage->m_animated) {
		DrawAnimatedArt(uiImage->m_art, x, uiImage->m_rect.y);
	} else {
		DrawArt(x, uiImage->m_rect.y, uiImage->m_art, uiImage->m_frame, uiImage->m_rect.w, uiImage->m_rect.h);
	}
}

static void Render(const UiArtTextButton* uiButton)
{
	DrawArtStr(uiButton->m_text, uiButton->m_rect, uiButton->m_iFlags);
}

static void Render(UiButton* button)
{
	int frame = button->m_pressed ? UiButton::PRESSED : UiButton::DEFAULT;

	DrawArt(button->m_rect.x, button->m_rect.y, button->m_art, frame, button->m_rect.w, button->m_rect.h);

	SDL_Rect textRect = button->m_rect;
	if (!button->m_pressed)
		--textRect.y;

	SDL_Color color1 = { 243, 243, 243, 0 };
	SDL_Color color2 = { 0, 0, 0, 0 };
	DrawTTF(button->m_text, textRect, UIS_CENTER,
	    color1, color2, button->m_render_cache);
}

static void Render(const UiList* uiList)
{
	for (unsigned i = 0; i < uiList->m_vecItems->size(); i++) {
		SDL_Rect rect = uiList->itemRect(i);
		if (i + ListOffset == SelectedItem)
			DrawSelector(rect);
		UiListItem* item = (*uiList->m_vecItems)[i];
		DrawArtStr(item->m_text, rect, uiList->m_iFlags);
	}
}

static void Render(const UiScrollBar* uiSb)
{
	// Bar background (tiled):
	{
		const int bgYEnd = DownArrowRect(uiSb).y;
		int bgY = uiSb->m_rect.y + ArtScrollBarArrow.h();
		while (bgY < bgYEnd) {
			int drawH = std::min(bgY + ArtScrollBarBackground.h(), bgYEnd) - bgY;
			DrawArt(uiSb->m_rect.x, bgY, &ArtScrollBarBackground, 0, SCROLLBAR_BG_WIDTH, drawH);
			bgY += drawH;
		}
	}

	// Arrows:
	{
		const SDL_Rect rect = UpArrowRect(uiSb);
		const int frame = static_cast<int>(scrollBarState.upArrowPressed ? ScrollBarArrowFrame_UP_ACTIVE : ScrollBarArrowFrame_UP);
		DrawArt(rect.x, rect.y, &ArtScrollBarArrow, frame, rect.w);
	}
	{
		const SDL_Rect rect = DownArrowRect(uiSb);
		const int frame = static_cast<int>(scrollBarState.downArrowPressed ? ScrollBarArrowFrame_DOWN_ACTIVE : ScrollBarArrowFrame_DOWN);
		DrawArt(rect.x, rect.y, &ArtScrollBarArrow, frame, rect.w);
	}

	// Thumb:
	if (SelectedItemMax > 0) {
		const SDL_Rect rect = ThumbRect(uiSb, SelectedItem, SelectedItemMax);
		DrawArt(rect.x, rect.y, &ArtScrollBarThumb);
	}
}

static void Render(const UiEdit* uiEdit)
{
	DrawSelector(uiEdit->m_rect);
	SDL_Rect rect = uiEdit->m_rect;
	rect.x += 43;
	rect.y += 1;
	rect.w -= 86;
	DrawArtStr(uiEdit->m_value, rect, UIS_MED | UIS_GOLD, /*drawTextCursor=*/true);
}

static void RenderItem(UiItemBase* item)
{
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
		Render(static_cast<UiButton *>(item));
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
	default:
		ASSUME_UNREACHABLE
		break;
	}
}

static bool HandleMouseEventArtTextButton(const SDL_Event &event, const UiArtTextButton* uiButton)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return false;
	uiButton->m_action();
	return true;
}

static bool HandleMouseEventButton(const SDL_Event &event, UiButton* button)
{
	if (event.type == SDL_MOUSEBUTTONDOWN)
		button->m_pressed = true;
	else if (event.type == SDL_MOUSEBUTTONUP)
		button->m_action();
	else
		return false;
	return true;
}

#ifdef USE_SDL1
Uint32 dbClickTimer;
#endif

static bool HandleMouseEventList(const SDL_Event &event, UiList* uiList)
{
	if (event.type != SDL_MOUSEBUTTONDOWN)
		return false;

	const unsigned index = uiList->indexAt(event.button.y) + ListOffset;

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

static bool HandleMouseEventScrollBar(const SDL_Event &event, const UiScrollBar* uiSb)
{
	if (event.type == SDL_MOUSEBUTTONUP) {
		if (scrollBarState.upArrowPressed && IsInsideRect(event, UpArrowRect(uiSb))) {
			UiFocusUp();
			return true;
		}
		if (scrollBarState.downArrowPressed && IsInsideRect(event, DownArrowRect(uiSb))) {
			UiFocusDown();
			return true;
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (IsInsideRect(event, BarRect(uiSb))) {
			// Scroll up or down based on thumb position.
			const SDL_Rect thumbRect = ThumbRect(uiSb, SelectedItem, SelectedItemMax);
			if (event.button.y < thumbRect.y) {
				UiFocusPageUp();
			} else if (event.button.y > thumbRect.y + thumbRect.h) {
				UiFocusPageDown();
			}
			return true;
		}
		if (IsInsideRect(event, UpArrowRect(uiSb))) {
			scrollBarState.upArrowPressed = true;
			return true;
		} else if (IsInsideRect(event, DownArrowRect(uiSb))) {
			scrollBarState.downArrowPressed = true;
			return true;
		}
	}
	return false;
}

static bool HandleMouseEvent(const SDL_Event &event, UiItemBase* item)
{
	if ((item->m_iFlags & (UIS_HIDDEN | UIS_DISABLED)) || !IsInsideRect(event, item->m_rect))
		return false;
	switch (item->m_type) {
	case UI_ART_TEXT_BUTTON:
		return HandleMouseEventArtTextButton(event, static_cast<UiArtTextButton *>(item));
	case UI_BUTTON:
		return HandleMouseEventButton(event, static_cast<UiButton *>(item));
	case UI_LIST:
		return HandleMouseEventList(event, static_cast<UiList *>(item));
	case UI_SCROLLBAR:
		return HandleMouseEventScrollBar(event, static_cast<UiScrollBar *>(item));
	default:
		return false;
	}
}

static void HandleGlobalMouseUpButton(UiButton* button)
{
	button->m_pressed = false;
}

void UiRenderItems(const std::vector<UiItemBase *> &uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++)
		if (!(uiItems[i]->m_iFlags & UIS_HIDDEN))
			RenderItem(uiItems[i]);
}

void UiClearItems(std::vector<UiItemBase *> &uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++) {
		UiItemBase* pUIItem = uiItems[i];
		delete pUIItem;
	}
	uiItems.clear();
}

void UiClearListItems(std::vector<UiListItem *> &uiItems)
{
	for (size_t i = 0; i < uiItems.size(); i++) {
		UiListItem* pUIItem = uiItems[i];
		delete pUIItem;
	}
	uiItems.clear();
}

bool UiItemMouseEvents(SDL_Event *event, const std::vector<UiItemBase *> &uiItems)
{
	if (event->button.button != SDL_BUTTON_LEFT)
		return false;

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

	if (event->type == SDL_MOUSEBUTTONUP) {
		scrollBarState.downArrowPressed = scrollBarState.upArrowPressed = false;
		for (unsigned i = 0; i < uiItems.size(); i++) {
			UiItemBase *item = uiItems[i];
			if (item->m_type == UI_BUTTON)
				HandleGlobalMouseUpButton(static_cast<UiButton *>(item));
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
